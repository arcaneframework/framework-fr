// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
 * Ce fichier est basé sur le travail sur la bibliothèque AMGCL (version mars 2026)
 * qui peut être trouvée à https://github.com/ddemidov/amgcl.
 *
 * Copyright (c) 2012-2022 Denis Demidov <dennis.demidov@gmail.com>
 * SPDX-License-Identifier: MIT
 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * Copyright (c) 1998 Lawrence Livermore National Security, LLC and other
 * HYPRE Project Developers. See the top-level COPYRIGHT file for details.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR MIT)
 ******************************************************************************/

/*
   Exemple 5

   Interface:    Linéaire-Algébrique (IJ)

   Compiler avec: make ex5

   Exécution d'exemple:   mpirun -np 4 ex5

   Description:  Cet exemple résout le problème de Laplacien en 2-D avec des
                 conditions aux limites nulles sur une grille n x n. Le nombre
                 d'inconnues est N=n^2. La méthode de stencil standard à 5 points est
                 utilisée, et nous résolvons uniquement pour les nœuds intérieurs.

                 Cet exemple résout le même problème que l'Exemple 3. Les solveurs
                 disponibles sont AMG, PCG, et PCG avec des préconditionneurs AMG ou
                 Parasails.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "HYPRE_krylov.h"
#include "HYPRE.h"
#include "HYPRE_parcsr_ls.h"

/******************************************************************************
 * Copyright (c) 1998 Lawrence Livermore National Security, LLC and other
 * HYPRE Project Developers. See the top-level COPYRIGHT file for details.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR MIT)
 ******************************************************************************/

/*--------------------------------------------------------------------------
 * Fichier d'en-tête pour les exemples
 *--------------------------------------------------------------------------*/

#ifndef HYPRE_EXAMPLES_INCLUDES
#define HYPRE_EXAMPLES_INCLUDES

#include <HYPRE_config.h>

#if defined(HYPRE_EXAMPLE_USING_CUDA)

#include <cuda_runtime.h>

#ifndef HYPRE_USING_UNIFIED_MEMORY
#error *** L'exécution des exemples sur des GPU nécessite une mémoire unifiée. Veuillez reconfigurer et reconstruire avec --enable-unified-memory ***
#endif

static inline void*
gpu_malloc(size_t size)
{
   void *ptr = NULL;
   cudaMallocManaged(&ptr, size, cudaMemAttachGlobal);
   return ptr;
}

static inline void*
gpu_calloc(size_t num, size_t size)
{
   void *ptr = NULL;
   cudaMallocManaged(&ptr, num * size, cudaMemAttachGlobal);
   cudaMemset(ptr, 0, num * size);
   return ptr;
}

#define malloc(size) gpu_malloc(size)
#define calloc(num, size) gpu_calloc(num, size)
#define free(ptr) ( cudaFree(ptr), ptr = NULL )
#endif /* #if defined(HYPRE_EXAMPLE_USING_CUDA) */
#endif /* #ifndef HYPRE_EXAMPLES_INCLUDES */

#ifdef HYPRE_EXVIS
#include "vis.c"
#endif

#include <memory>
#include <vector>
#include <iostream>

#include "arccore/base/Convert.h"
#include "arccore/base/FatalErrorException.h"
#include "arccore/alina/Profiler.h"

using namespace Arcane;

int hypre_FlexGMRESModifyPCAMGExample(void *precond_data, int iterations,
                                      double rel_residual_norm);

#define my_min(a,b)  (((a)<(b)) ? (a) : (b))

extern "C++" void
_doHypreSolver(int nb_row,
               std::vector<ptrdiff_t> const& _ptr,
               std::vector<ptrdiff_t> const& _col,
               std::vector<double> const& _val,
               std::vector<double> const& _rhs,
               std::vector<double>& _x,
               int argc, char* argv[])
{
  auto& prof = Alina::Profiler::globalProfiler();
  auto t = prof.scoped_tic("Hypre");

  std::cout << "DO_HYPRE nb_row=" << nb_row << "\n";
  int i;
  int myid, num_procs;
  const int N = nb_row;

  int ilower, iupper;
  int local_size, extra;

  int solver_id;
  int vis, print_system;

  //double h, h2;

  HYPRE_IJMatrix A;
  HYPRE_ParCSRMatrix parcsr_A;
  HYPRE_IJVector b;
  HYPRE_ParVector par_b;
  HYPRE_IJVector x;
  HYPRE_ParVector par_x;

  HYPRE_Solver solver, precond;

  /* Initialisation de MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  /* Initialisation de HYPRE */
  HYPRE_Initialize();

  /* Affichage des informations GPU */
  /* HYPRE_PrintDeviceInfo(); */
#if defined(HYPRE_USING_GPU)
  /* Utilise l'implémentation du fournisseur pour SpGEMM */
  HYPRE_SetSpGemmUseVendor(0);
#endif

  /* Paramètres par défaut du problème */
  const int n = nb_row;
  solver_id = 0;
  vis = 0;
  print_system = 0;

  /* Analyse de la ligne de commande */
  {
    int arg_index = 0;
    int print_usage = 0;

    while (arg_index < argc) {
      if (strcmp(argv[arg_index], "-n") == 0) {
        arg_index++;
        //n = atoi(argv[arg_index++]);
      }
      else if (strcmp(argv[arg_index], "-solver") == 0) {
        arg_index++;
        solver_id = atoi(argv[arg_index++]);
      }
      else if (strcmp(argv[arg_index], "-vis") == 0) {
        arg_index++;
        vis = 1;
      }
      else if (strcmp(argv[arg_index], "-print_system") == 0) {
        arg_index++;
        print_system = 1;
      }
      else if (strcmp(argv[arg_index], "-help") == 0) {
        print_usage = 1;
        break;
      }
      else {
        arg_index++;
      }
    }

    if ((print_usage) && (myid == 0)) {
      printf("\n");
      printf("Utilisation: %s [<options>]\n", argv[0]);
      printf("\n");
      printf("  -n <n>              : taille du problème dans chaque direction (par défaut: 33)\n");
      printf("  -solver <ID>        : ID du solveur\n");
      printf("                        0  - AMG (par défaut) \n");
      printf("                        1  - AMG-PCG\n");
      printf("                        8  - ParaSails-PCG\n");
      printf("                        50 - PCG\n");
      printf("                        61 - AMG-FlexGMRES\n");
      printf("  -vis                : enregistrer la solution pour la visualisation GLVis\n");
      printf("  -print_system       : imprimer la matrice et le rhs\n");
      printf("\n");
    }

    if (print_usage) {
      MPI_Finalize();
      return;
    }
  }

  // Remplit la valeur nb par ligne.
  std::vector<int> nb_value_per_row(n);
  for (int i = 0; i < n; ++i)
    nb_value_per_row[i] = static_cast<HYPRE_BigInt>(_ptr[i + 1] - _ptr[i]);

  // L'index de colonne est le même que '_col' de la matrice CSR
  // mais nous faisons une copie si la taille de l'index est différente entre Hypre et Alina.
  std::vector<HYPRE_BigInt> hypre_column_index(_col.begin(), _col.end());

  // ID de chaque ligne (en séquentiel, c'est le même que l'index)
  std::vector<HYPRE_Int> hypre_row_index(n);
  for (int i = 0; i < n; ++i) {
    hypre_row_index[i] = i;
  }

  /* Chaque processeur ne connaît que ses propres lignes - la plage est indiquée par ilower
     et upper. Ici, nous partitionnons les lignes. Nous prenons en compte le fait que
     N peut ne pas être divisible uniformément par le nombre de processeurs. */
  local_size = N / num_procs;
  extra = N - local_size * num_procs;

  ilower = local_size * myid;
  ilower += my_min(myid, extra);

  iupper = local_size * (myid + 1);
  iupper += my_min(myid + 1, extra);
  iupper = iupper - 1;
  std::cout << "LOWER=" << ilower << " UPPER=" << iupper << "\n";
  /* Combien de lignes ai-je ? */
  local_size = iupper - ilower + 1;

  {
    auto t = prof.scoped_tic("Création de IJMatrix");
    /* Crée la matrice.
       Notez qu'il s'agit d'une matrice carrée, nous indiquons donc la taille de la partition
       de ligne deux fois (car le nombre de lignes = nombre de colonnes) */
    HYPRE_IJMatrixCreate(MPI_COMM_WORLD, ilower, iupper, ilower, iupper, &A);

    /* Choisis un stockage de format csr parallèle (voir le Manuel de l'utilisateur) */
    HYPRE_IJMatrixSetObjectType(A, HYPRE_PARCSR);

    /* Initialise avant de définir les coefficients */
    HYPRE_IJMatrixInitialize(A);
  }

  // Remplis la matrice.
  {
    auto t = prof.scoped_tic("Définition des valeurs de IJMatrix");

    HYPRE_IJMatrixSetValues(A, n,
                            nb_value_per_row.data(),
                            hypre_row_index.data(),
                            hypre_column_index.data(),
                            _val.data());
  }

  {
    auto t = prof.scoped_tic("Assemblage de IJMatrix");
    /* Assemble après avoir défini les coefficients */
    HYPRE_IJMatrixAssemble(A);
  }

  /* Remarque : pour le test de petits problèmes, on peut souhaiter lire
      une matrice au format IJ (pour le format, voir les fichiers de sortie
      de l'option -print_system).
      Dans ce cas, on utiliserait la routine suivante:
      HYPRE_IJMatrixRead( <nom_de_fichier>, MPI_COMM_WORLD,
                          HYPRE_PARCSR, &A );
      <nom_de_fichier>  = IJ.A.out pour lire ce qui a été imprimé par
      -print_system (les numéros de processeur sont omis).
      Un appel à HYPRE_IJMatrixRead est une *alternative* à la
      séquence suivante d'appels HYPRE_IJMatrix:
      Create, SetObjectType, Initialize, SetValues, et Assemble
   */

  /* Récupérer l'objet matrice parcsr pour l'utiliser */
  HYPRE_IJMatrixGetObject(A, (void**)&parcsr_A);

  /* Crée le rhs et la solution */
  HYPRE_IJVectorCreate(MPI_COMM_WORLD, ilower, iupper, &b);
  HYPRE_IJVectorSetObjectType(b, HYPRE_PARCSR);
  HYPRE_IJVectorInitialize(b);

  HYPRE_IJVectorCreate(MPI_COMM_WORLD, ilower, iupper, &x);
  HYPRE_IJVectorSetObjectType(x, HYPRE_PARCSR);
  HYPRE_IJVectorInitialize(x);

  /* Définit les valeurs du rhs à h^2 et la solution à zéro */
  {
    //double *rhs_values, *x_values;
    int* rows;

    //rhs_values = (double*)calloc(local_size, sizeof(double));
    //x_values = (double*)calloc(local_size, sizeof(double));
    rows = (int*)calloc(local_size, sizeof(int));

    for (i = 0; i < local_size; i++) {
      //rhs_values[i] = h2;
      //x_values[i] = 0.0;
      rows[i] = ilower + i;
    }

    HYPRE_IJVectorSetValues(b, local_size, rows, _rhs.data());
    HYPRE_IJVectorSetValues(x, local_size, rows, _x.data());

    //free(x_values);
    //free(rhs_values);
    free(rows);
  }

  HYPRE_IJVectorAssemble(b);
  /* Tout comme pour la matrice, pour des tests, on peut souhaiter lire un rhs:
       HYPRE_IJVectorRead( <nom_de_fichier>, MPI_COMM_WORLD,
                                 HYPRE_PARCSR, &b );
       comme alternative à la
       séquence suivante d'appels HYPRE_IJVectors:
       Create, SetObjectType, Initialize, SetValues, et Assemble
   */
  HYPRE_IJVectorGetObject(b, (void**)&par_b);

  HYPRE_IJVectorAssemble(x);
  HYPRE_IJVectorGetObject(x, (void**)&par_x);

  /* Imprime le système - les noms de fichiers seront IJ.out.A.XXXXX
        et IJ.out.b.XXXXX, où XXXXX = ID du processeur */
  if (print_system) {
    HYPRE_IJMatrixPrint(A, "IJ.out.A");
    HYPRE_IJVectorPrint(b, "IJ.out.b");
  }
  solver_id = 0;
  if (auto v = Convert::Type<Int32>::tryParseFromEnvironment("ALINA_HYPRE_SOLVER", true))
    solver_id = v.value();

  double solver_tolerance = 1.0e-8;

  /* Choisit un solveur et résoudre le système */
  std::cout << "FIN DE L'ASSEMBLAGE solver_id=" << solver_id << "\n";
  /* AMG */
  if (solver_id == 0) {
    auto t = prof.scoped_tic("HypreSolver AMG");
    int num_iterations;
    double final_res_norm;

    /* Crée le solveur */
    HYPRE_BoomerAMGCreate(&solver);

    /* Définit certains paramètres (Voir le Manuel de Référence pour plus de paramètres) */
    HYPRE_BoomerAMGSetPrintLevel(solver, 3); /* imprimer les informations de résolution + paramètres */
    HYPRE_BoomerAMGSetOldDefault(solver); /* Raffinement Falgout avec interpolation classique modifiée */
    HYPRE_BoomerAMGSetRelaxType(solver, 3); /* Relaxation hybride G-S/Jacobi */
    HYPRE_BoomerAMGSetRelaxOrder(solver, 1); /* utilise la relaxation C/F */
    HYPRE_BoomerAMGSetNumSweeps(solver, 1); /* Balayages à chaque niveau */
    HYPRE_BoomerAMGSetMaxLevels(solver, 20); /* nombre maximum de niveaux */
    HYPRE_BoomerAMGSetTol(solver, solver_tolerance); /* tolérance de convergence */

    /* Maintenant, configurer et résoudre ! */
    {
      auto t = prof.scoped_tic("Configuration AMG");
      HYPRE_BoomerAMGSetup(solver, parcsr_A, par_b, par_x);
    }
    {
      auto t = prof.scoped_tic("Résolution AMG");
      HYPRE_BoomerAMGSolve(solver, parcsr_A, par_b, par_x);
    }

    /* Informations d'exécution - nécessaire si le journal est activé */
    HYPRE_BoomerAMGGetNumIterations(solver, &num_iterations);
    HYPRE_BoomerAMGGetFinalRelativeResidualNorm(solver, &final_res_norm);
    if (myid == 0) {
      printf("\n");
      printf("Itérations = %d\n", num_iterations);
      printf("Norme résiduelle relative finale = %e\n", final_res_norm);
      printf("\n");
    }

    /* Détruit le solveur */
    HYPRE_BoomerAMGDestroy(solver);
  }
  /* PCG */
  else if (solver_id == 50) {
    auto t = prof.scoped_tic("HypreSolver PCG");
    int num_iterations;
    double final_res_norm;

    /* Crée le solveur */
    HYPRE_ParCSRPCGCreate(MPI_COMM_WORLD, &solver);

    /* Définit certains paramètres (Voir le Manuel de Référence pour plus de paramètres) */
    HYPRE_PCGSetMaxIter(solver, 1000); /* itérations max */
    HYPRE_PCGSetTol(solver, solver_tolerance); /* tolérance de convergence */
    HYPRE_PCGSetTwoNorm(solver, 1); /* utiliser la double norme comme critère d'arrêt */
    HYPRE_PCGSetPrintLevel(solver, 2); /* affiche les informations d'itération */
    HYPRE_PCGSetLogging(solver, 1); /* nécessaire pour obtenir les informations d'exécution plus tard */

    /* Maintenant, configurer et résoudre ! */
    HYPRE_ParCSRPCGSetup(solver, parcsr_A, par_b, par_x);
    HYPRE_ParCSRPCGSolve(solver, parcsr_A, par_b, par_x);

    /* Informations d'exécution - nécessaire si le journal est activé */
    HYPRE_PCGGetNumIterations(solver, &num_iterations);
    HYPRE_PCGGetFinalRelativeResidualNorm(solver, &final_res_norm);
    if (myid == 0) {
      printf("\n");
      printf("Itérations = %d\n", num_iterations);
      printf("Norme résiduelle relative finale = %e\n", final_res_norm);
      printf("\n");
    }

    /* Détruit le solveur */
    HYPRE_ParCSRPCGDestroy(solver);
  }
  /* PCG avec préconditionneur AMG */
  else if (solver_id == 1) {
    auto t = prof.scoped_tic("HypreSolver PCG-AMG");
    int num_iterations;
    double final_res_norm;

    /* Crée le solveur */
    HYPRE_ParCSRPCGCreate(MPI_COMM_WORLD, &solver);

    /* Définit certains paramètres (Voir le Manuel de Référence pour plus de paramètres) */
    HYPRE_PCGSetMaxIter(solver, 1000); /* itérations max */
    HYPRE_PCGSetTol(solver, solver_tolerance); /* tolérance de convergence */
    HYPRE_PCGSetTwoNorm(solver, 1); /* utiliser la double norme comme critère d'arrêt */
    HYPRE_PCGSetPrintLevel(solver, 2); /* affiche les informations de résolution */
    HYPRE_PCGSetLogging(solver, 1); /* nécessaire pour obtenir les informations d'exécution plus tard */

    /* Maintenant, configurer le préconditionneur AMG et spécifier les paramètres */
    HYPRE_BoomerAMGCreate(&precond);
    HYPRE_BoomerAMGSetPrintLevel(precond, 1); /* imprimer les informations de résolution AMG */
    HYPRE_BoomerAMGSetCoarsenType(precond, 6);
    HYPRE_BoomerAMGSetOldDefault(precond);
    HYPRE_BoomerAMGSetRelaxType(precond, 6); /* Hybride G.S./Jacobi symétrique */
    HYPRE_BoomerAMGSetNumSweeps(precond, 1);
    HYPRE_BoomerAMGSetTol(precond, 0.0); /* tolérance de convergence zéro */
    HYPRE_BoomerAMGSetMaxIter(precond, 1); /* faire seulement une itération ! */

    /* Définit le préconditionneur PCG */
    HYPRE_PCGSetPrecond(solver, (HYPRE_PtrToSolverFcn)HYPRE_BoomerAMGSolve,
                        (HYPRE_PtrToSolverFcn)HYPRE_BoomerAMGSetup, precond);

    /* Maintenant, configurer et résoudre ! */
    prof.tic("Configuration");
    HYPRE_ParCSRPCGSetup(solver, parcsr_A, par_b, par_x);
    prof.toc("Configuration");
    prof.tic("Résolution");
    HYPRE_ParCSRPCGSolve(solver, parcsr_A, par_b, par_x);
    prof.toc("Résolution");

    /* Informations d'exécution - nécessaire si le journal est activé */
    HYPRE_PCGGetNumIterations(solver, &num_iterations);
    HYPRE_PCGGetFinalRelativeResidualNorm(solver, &final_res_norm);
    if (myid == 0) {
      printf("\n");
      printf("Itérations = %d\n", num_iterations);
      printf("Norme résiduelle relative finale = %e\n", final_res_norm);
      printf("\n");
    }

    /* Détruit le solveur et le préconditionneur */
    HYPRE_ParCSRPCGDestroy(solver);
    HYPRE_BoomerAMGDestroy(precond);
  }
  /* PCG avec préconditionneur Parasails */
  else if (solver_id == 8) {
    auto t = prof.scoped_tic("HypreSolver PCG - Parasails");
    int num_iterations;
    double final_res_norm;

    int sai_max_levels = 1;
    double sai_threshold = 0.1;
    double sai_filter = 0.05;
    int sai_sym = 1;

    /* Crée le solveur */
    HYPRE_ParCSRPCGCreate(MPI_COMM_WORLD, &solver);

    /* Définit certains paramètres (Voir le Manuel de Référence pour plus de paramètres) */
    HYPRE_PCGSetMaxIter(solver, 1000); /* itérations max */
    HYPRE_PCGSetTol(solver, solver_tolerance); /* tolérance de convergence */
    HYPRE_PCGSetTwoNorm(solver, 1); /* utiliser la double norme comme critère d'arrêt */
    HYPRE_PCGSetPrintLevel(solver, 2); /* affiche les informations de résolution */
    HYPRE_PCGSetLogging(solver, 1); /* nécessaire pour obtenir les informations d'exécution plus tard */

    /* Maintenant, configurer le préconditionneur ParaSails et spécifier les paramètres */
    HYPRE_ParaSailsCreate(MPI_COMM_WORLD, &precond);

    /* Définir certains paramètres (Voir le Manuel de Référence pour plus de paramètres) */
    HYPRE_ParaSailsSetParams(precond, sai_threshold, sai_max_levels);
    HYPRE_ParaSailsSetFilter(precond, sai_filter);
    HYPRE_ParaSailsSetSym(precond, sai_sym);
    HYPRE_ParaSailsSetLogging(precond, 3);

    /* Définit le préconditionneur PCG */
    HYPRE_PCGSetPrecond(solver, (HYPRE_PtrToSolverFcn)HYPRE_ParaSailsSolve,
                        (HYPRE_PtrToSolverFcn)HYPRE_ParaSailsSetup, precond);

    /* Maintenant, configurer et résoudre ! */
    HYPRE_ParCSRPCGSetup(solver, parcsr_A, par_b, par_x);
    HYPRE_ParCSRPCGSolve(solver, parcsr_A, par_b, par_x);

    /* Informations d'exécution - nécessaire si le journal est activé */
    HYPRE_PCGGetNumIterations(solver, &num_iterations);
    HYPRE_PCGGetFinalRelativeResidualNorm(solver, &final_res_norm);
    if (myid == 0) {
      printf("\n");
      printf("Itérations = %d\n", num_iterations);
      printf("Norme résiduelle relative finale = %e\n", final_res_norm);
      printf("\n");
    }

    /* Détruit le solveur et le préconditionneur */
    HYPRE_ParCSRPCGDestroy(solver);
    HYPRE_ParaSailsDestroy(precond);
  }
  /* Flexible GMRES avec préconditionneur AMG */
  else if (solver_id == 61) {
    auto t = prof.scoped_tic("HypreSolver Flexible GMRES - AMG");
    int num_iterations;
    double final_res_norm;
    int restart = 30;
    int modify = 1;

    /* Crée le solveur */
    HYPRE_ParCSRFlexGMRESCreate(MPI_COMM_WORLD, &solver);

    /* Définit certains paramètres (Voir le Manuel de Référence pour plus de paramètres) */
    HYPRE_FlexGMRESSetKDim(solver, restart);
    HYPRE_FlexGMRESSetMaxIter(solver, 1000); /* itérations max */
    HYPRE_FlexGMRESSetTol(solver, solver_tolerance); /* tolérance de convergence */
    HYPRE_FlexGMRESSetPrintLevel(solver, 2); /* affiche les informations de résolution */
    HYPRE_FlexGMRESSetLogging(solver, 1); /* nécessaire pour obtenir les informations d'exécution plus tard */

    /* Maintenant, configurer le préconditionneur AMG et spécifier les paramètres */
    HYPRE_BoomerAMGCreate(&precond);
    HYPRE_BoomerAMGSetPrintLevel(precond, 1); /* imprimer les informations de résolution AMG */
    HYPRE_BoomerAMGSetCoarsenType(precond, 6);
    HYPRE_BoomerAMGSetOldDefault(precond);
    HYPRE_BoomerAMGSetRelaxType(precond, 6); /* Hybride G.S./Jacobi symétrique */
    HYPRE_BoomerAMGSetNumSweeps(precond, 1);
    HYPRE_BoomerAMGSetTol(precond, 0.0); /* tolérance de convergence zéro */
    HYPRE_BoomerAMGSetMaxIter(precond, 1); /* faire seulement une itération ! */

    /* Définit le préconditionneur FlexGMRES */
    HYPRE_FlexGMRESSetPrecond(solver, (HYPRE_PtrToSolverFcn)HYPRE_BoomerAMGSolve,
                              (HYPRE_PtrToSolverFcn)HYPRE_BoomerAMGSetup, precond);

    if (modify) {
      /* ceci est un appel optionnel - si vous ne l'appelez pas, hypre_FlexGMRESModifyPCDefault
            est utilisé - ce qui ne fait rien. Sinon, vous pouvez en définir un propre, similaire à
            celui utilisé ici */
      HYPRE_FlexGMRESSetModifyPC(solver, (HYPRE_PtrToModifyPCFcn)hypre_FlexGMRESModifyPCAMGExample);
    }

    /* Maintenant, configurer et résoudre ! */
    {
      auto t = prof.scoped_tic("Configuration FlexGMRES");
      HYPRE_ParCSRFlexGMRESSetup(solver, parcsr_A, par_b, par_x);
    }
    {
      auto t = prof.scoped_tic("Résolution FlexGMRES");
      HYPRE_ParCSRFlexGMRESSolve(solver, parcsr_A, par_b, par_x);
    }

    /* Informations d'exécution - nécessaire si le journal est activé */
    HYPRE_FlexGMRESGetNumIterations(solver, &num_iterations);
    HYPRE_FlexGMRESGetFinalRelativeResidualNorm(solver, &final_res_norm);
    if (myid == 0) {
      printf("\n");
      printf("Itérations = %d\n", num_iterations);
      printf("Norme résiduelle relative finale = %e\n", final_res_norm);
      printf("\n");
    }

    /* Détruit le solveur et le préconditionneur */
    HYPRE_ParCSRFlexGMRESDestroy(solver);
    HYPRE_BoomerAMGDestroy(precond);
  }
  else {
    if (myid == 0) {
      ARCCORE_FATAL("ID de solveur '{0}' spécifié invalide.", solver_id);
    }
  }

  if (print_system)
    HYPRE_IJVectorPrint(x, "IJ.out.x");

  /* Enregistre la solution pour la visualisation GLVis, voir vis/glvis-ex5.sh */
  if (vis) {
#ifdef HYPRE_EXVIS
    FILE* file;
    char filename[255];

    int nvalues = local_size;
    int* rows = (int*)calloc(nvalues, sizeof(int));
    double* values = (double*)calloc(nvalues, sizeof(double));

    for (i = 0; i < nvalues; i++) {
      rows[i] = ilower + i;
    }

    /* récupère la solution locale */
    HYPRE_IJVectorGetValues(x, nvalues, rows, values);

    sprintf(filename, "%s.%06d", "vis/ex5.sol", myid);
    if ((file = fopen(filename, "w")) == NULL) {
      printf("Erreur: impossible d'ouvrir le fichier de sortie %s\n", filename);
      MPI_Finalize();
      exit(1);
    }

    /* enregistre la solution */
    for (i = 0; i < nvalues; i++) {
      fprintf(file, "%.14e\n", values[i]);
    }

    fflush(file);
    fclose(file);

    free(rows);
    free(values);

    /* enregistre le maillage global des éléments finis */
    if (myid == 0) {
      GLVis_PrintGlobalSquareMesh("vis/ex5.mesh", n - 1);
    }
#endif
  }

  /* Nettoyage */
  HYPRE_IJMatrixDestroy(A);
  HYPRE_IJVectorDestroy(b);
  HYPRE_IJVectorDestroy(x);

  /* Finalise HYPRE */
  HYPRE_Finalize();

  /* Finalise MPI*/
  MPI_Finalize();

  return;
}

/*--------------------------------------------------------------------------
  hypre_FlexGMRESModifyPCAMGExample -

  Ceci est un exemple (non recommandé)
  de la façon dont nous pouvons modifier les choses concernant AMG qui
  affectent la phase de résolution en fonction de la manière dont FlexGMRES progresse...
  Pour un autre préconditionneur, il pourrait être judicieux de modifier la tolérance..
 *--------------------------------------------------------------------------*/

int hypre_FlexGMRESModifyPCAMGExample(void* precond_data, [[maybe_unused]] int iterations,
                                      double rel_residual_norm)
{

  if (rel_residual_norm > .1) {
    HYPRE_BoomerAMGSetNumSweeps((HYPRE_Solver)precond_data, 10);
  }
  else {
    HYPRE_BoomerAMGSetNumSweeps((HYPRE_Solver)precond_data, 1);
  }

  return 0;
}
