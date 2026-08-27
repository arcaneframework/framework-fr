// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* AlinaLib.h                                                  (C) 2000-2026 */
/*                                                                           */
/* API publique pour Alina.                                                  */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_ALINA_ALINALIB_H
#define ARCCORE_ALINA_ALINALIB_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
 * Ce fichier est basé sur le travail effectué sur la bibliothèque AMGCL (version mars 2026)
 * qui peut être trouvée à https://github.com/ddemidov/amgcl.
 *
 * Copyright (c) 2012-2022 Denis Demidov <dennis.demidov@gmail.com>
 * SPDX-License-Identifier: MIT
 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/alina/AlinaGlobal.h"

#include <mpi.h>

// Convergence info
struct ARCCORE_ALINA_EXPORT AlinaConvergenceInfo
{
  int iterations;
  double residual;
};

typedef double(* AlinaDefVecFunction)(int vec, ptrdiff_t coo, void* data);

//! Gestion des paramètres.
struct AlinaParameters;

//! Gestion du préconditionneur
struct AlinaPreconditioner;

//! Solveur séquentiel;
struct AlinaSequentialSolver;

//! Solveur distribué;
struct AlinaDistributedSolver;

class ARCCORE_ALINA_EXPORT AlinaLib
{
 public:

  // Définit un paramètre entier dans une liste de paramètres.
  static void params_set_int(AlinaParameters* prm, const char* name, int value);

  // Définit un paramètre flottant dans une liste de paramètres.
  static void params_set_float(AlinaParameters* prm, const char* name, float value);

  // Définit un paramètre flottant dans une liste de paramètres.
  static void params_set_string(AlinaParameters* prm, const char* name, const char* value);

  // Lit les paramètres à partir d'un fichier JSON
  static void params_read_json(AlinaParameters* prm, const char* fname);

  // Détruit la liste de paramètres.
  static void params_destroy(AlinaParameters* prm);

  // Crée la liste de paramètres.
  static AlinaParameters* params_create();

  // Crée le préconditionneur AMG.
  static AlinaPreconditioner* preconditioner_create(int n,
                                                          const int* ptr,
                                                          const int* col,
                                                          const double* val,
                                                          AlinaParameters* parameters);

  // Applique le préconditionneur AMG (x = M^(-1) * rhs).
  static void preconditioner_apply(AlinaPreconditioner* amg, const double* rhs, double* x);

  // Affiche la structure du préconditionneur
  static void preconditioner_report(AlinaPreconditioner* amg);

  // Détruit le préconditionneur AMG
  static void preconditioner_destroy(AlinaPreconditioner* amg);

  // Crée le solveur itératif préconditionné par AMG.
  static AlinaSequentialSolver* solver_create(int n,
                                              const int* ptr,
                                              const int* col,
                                              const double* val,
                                              AlinaParameters* parameters);

  // Résoud le problème pour le côté droit donné.
  static AlinaConvergenceInfo solver_solve(AlinaSequentialSolver* solver,
                                double const* rhs,
                                double* x);

  // Résoud le problème pour la matrice et le côté droit donnés.
  static AlinaConvergenceInfo solver_solve_matrix(AlinaSequentialSolver* solver,
                                       int const* A_ptr,
                                       int const* A_col,
                                       double const* A_val,
                                       double const* rhs,
                                       double* x);

  // Affiche la structure du solveur
  static void solver_report(AlinaSequentialSolver* solver);

  // Détruit le solveur itératif.
  static void solver_destroy(AlinaSequentialSolver* solver);

  // Crée le solveur distribué.
  static AlinaDistributedSolver* solver_mpi_create(MPI_Comm comm,
                                                   ptrdiff_t n,
                                                   const int* ptr,
                                                   const int* col,
                                                   const double* val,
                                                   int n_def_vec,
                                                   AlinaDefVecFunction def_vec_func,
                                                   void* def_vec_data,
                                                   AlinaParameters* params);

  // Trouve la solution pour le côté droit donné.
  static AlinaConvergenceInfo solver_mpi_solve(AlinaDistributedSolver* solver,
                                    double const* rhs,
                                    double* x);

  // Détruit le solveur distribué.
  static void solver_mpi_destroy(AlinaDistributedSolver* solver);
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
