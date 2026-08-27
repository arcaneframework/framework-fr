// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
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

#include <iostream>
#include <string>
#include <random>

// Pour supprimer les avertissements concernant l'utilisation obsolète d'Eigen.
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wint-in-bool-context"

#include <boost/range/iterator_range.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#if defined(SOLVER_BACKEND_CUDA)
#include "arccore/alina/CudaBackend.h"
#include "arccore/alina/relaxation_cusparse_ilu0.h"
typedef Arcane::Alina::backend::cuda<double> Backend;
#elif defined(SOLVER_BACKEND_EIGEN)
#include "arccore/alina/EigenBackend.h"
typedef Arcane::Alina::backend::EigenBackend<double> Backend;
#else
#ifndef SOLVER_BACKEND_BUILTIN
#define SOLVER_BACKEND_BUILTIN
#endif
#include "arccore/alina/BuiltinBackend.h"
#include "arccore/alina/StaticMatrix.h"
#include "arccore/alina/Adapters.h"
// Utilise l'indexation 32 bits pour le backend.
using Backend = Arcane::Alina::BuiltinBackend<double, Arcane::Int32>;
//using Backend = Arcane::Alina::BuiltinBackend<double>;
#endif

#include <arccore/base/PlatformUtils.h>
#include <arccore/base/String.h>
#include <arccore/base/Convert.h>

#include "arccore/alina/RelaxationRuntime.h"
#include "arccore/alina/CoarseningRuntime.h"
#include "arccore/alina/SolverRuntime.h"
#include "arccore/alina/PreconditionerRuntime.h"
#include "arccore/alina/PreconditionedSolver.h"
#include "arccore/alina/AMG.h"
#include "arccore/alina/Adapters.h"
#include "arccore/alina/IO.h"
#include "arccore/alina/Profiler.h"

#include "arccore/common/internal/ProgramOptions.h"

#include "SampleProblemCommon.h"

#ifndef ARCCORE_ALINA_BLOCK_SIZES
#define ARCCORE_ALINA_BLOCK_SIZES (3)(4)
#endif

using namespace Arcane;

using Alina::precondition;

#ifdef SOLVER_BACKEND_BUILTIN
extern "C++" void
_doHypreSolver(int nb_row,
               std::vector<ptrdiff_t> const& ptr,
               std::vector<ptrdiff_t> const& col,
               std::vector<double> const& val,
               std::vector<double> const& rhs,
               std::vector<double>& x,
               int argc, char* argv[]);
#endif

#ifdef SOLVER_BACKEND_BUILTIN
//---------------------------------------------------------------------------
template <int B> Alina::SolverResult
block_solve(const Alina::PropertyTree& prm,
            size_t rows,
            std::vector<ptrdiff_t> const& ptr,
            std::vector<ptrdiff_t> const& col,
            std::vector<double> const& val,
            std::vector<double> const& rhs,
            std::vector<double>& x,
            bool reorder)
{
  auto& prof = Alina::Profiler::globalProfiler();

  typedef Alina::StaticMatrix<double, B, B> value_type;
  typedef Alina::StaticMatrix<double, B, 1> rhs_type;
  typedef Alina::BuiltinBackend<value_type> BBackend;

  typedef Alina::PreconditionedSolver<Alina::PreconditionerRuntime<BBackend>, Alina::SolverRuntime<BBackend>> Solver;

  auto As = std::tie(rows, ptr, col, val);
  auto Ab = Alina::adapter::block_matrix<value_type>(As);

  std::tuple<size_t, double> info;

  if (reorder) {
    prof.tic("reorder");
    Alina::adapter::reorder<> perm(Ab);
    prof.toc("reorder");

    prof.tic("setup");
    Solver solve(perm(Ab), prm);
    prof.toc("setup");

    std::cout << solve << std::endl;

    rhs_type const* fptr = reinterpret_cast<rhs_type const*>(&rhs[0]);
    rhs_type* xptr = reinterpret_cast<rhs_type*>(&x[0]);

    Alina::numa_vector<rhs_type> F(perm(SmallSpan<const rhs_type>(fptr, rows / B)));
    Alina::numa_vector<rhs_type> X(perm(SmallSpan<rhs_type>(xptr, rows / B)));

    prof.tic("solve");
    info = solve(F, X);
    prof.toc("solve");

    perm.inverse(X, xptr);
  }
  else {
    prof.tic("setup");
    Solver solve(Ab, prm);
    prof.toc("setup");

    std::cout << solve << std::endl;

    rhs_type const* fptr = reinterpret_cast<rhs_type const*>(&rhs[0]);
    rhs_type* xptr = reinterpret_cast<rhs_type*>(&x[0]);

    Alina::numa_vector<rhs_type> F(fptr, fptr + rows / B);
    Alina::numa_vector<rhs_type> X(xptr, xptr + rows / B);

    prof.tic("solve");
    info = solve(F, X);
    prof.toc("solve");

    std::copy(X.data(), X.data() + X.size(), xptr);
  }

  return info;
}
#endif

//---------------------------------------------------------------------------
Alina::SolverResult
scalar_solve(const Alina::PropertyTree& prm,
             size_t rows,
             std::vector<ptrdiff_t> const& ptr,
             std::vector<ptrdiff_t> const& col,
             std::vector<double> const& val,
             std::vector<double> const& rhs,
             std::vector<double>& x,
             bool reorder)
{
  std::cout << "Utilisation de la résolution scalaire ptr_size=" << sizeof(ptrdiff_t)
            << " taille_type_ptr=" << sizeof(Backend::ptr_type)
            << " taille_type_col=" << sizeof(Backend::col_type)
            << " taille_type_valeur=" << sizeof(Backend::value_type)
            << "\n";
  auto& prof = Alina::Profiler::globalProfiler();
  Backend::params bprm;

#if defined(SOLVER_BACKEND_CUDA)
  cusparseCreate(&bprm.cusparse_handle);
  {
    int dev;
    cudaGetDevice(&dev);

    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, dev);
    std::cout << prop.name << std::endl
              << std::endl;
  }
#endif

  using Solver = Alina::PreconditionedSolver<Alina::PreconditionerRuntime<Backend>, Alina::SolverRuntime<Backend>>;

  Alina::SolverResult info;

  if (reorder) {
    prof.tic("reorder");
    Alina::adapter::reorder<> perm(std::tie(rows, ptr, col, val));
    prof.toc("reorder");

    prof.tic("setup");
    Solver solve(perm(std::tie(rows, ptr, col, val)), prm, bprm);
    prof.toc("setup");

    std::cout << solve << std::endl;

    std::vector<double> tmp(rows);

    perm.forward(rhs, tmp);
    auto f_b = Backend::copy_vector(tmp, bprm);

    perm.forward(x, tmp);
    auto x_b = Backend::copy_vector(tmp, bprm);

    prof.tic("solve");
    info = solve(*f_b, *x_b);
    prof.toc("solve");

#if defined(SOLVER_BACKEND_CUDA)
    thrust::copy(x_b->begin(), x_b->end(), tmp.begin());
#else
    std::copy(&(*x_b)[0], &(*x_b)[0] + rows, &tmp[0]);
#endif

    perm.inverse(tmp, x);
  }
  else {
    prof.tic("setup");
    Solver solve(std::tie(rows, ptr, col, val), prm, bprm);
    prof.toc("setup");

    std::cout << solve << std::endl;

    auto f_b = Backend::copy_vector(rhs, bprm);
    auto x_b = Backend::copy_vector(x, bprm);

    prof.tic("solve");
    info = solve(*f_b, *x_b);
    prof.toc("solve");

#if defined(SOLVER_BACKEND_CUDA)
    thrust::copy(x_b->begin(), x_b->end(), x.begin());
#else
    std::copy(&(*x_b)[0], &(*x_b)[0] + rows, &x[0]);
#endif
  }

  return info;
}

#define ARCCORE_ALINA_CALL_BLOCK_SOLVER(z, data, B)                                    \
  case B:                                                                      \
    return block_solve<B>(prm, rows, ptr, col, val, rhs, x, reorder);

//---------------------------------------------------------------------------
Alina::SolverResult
solve(const Alina::PropertyTree& prm,
      size_t rows,
      std::vector<ptrdiff_t> const& ptr,
      std::vector<ptrdiff_t> const& col,
      std::vector<double> const& val,
      std::vector<double> const& rhs,
      std::vector<double>& x,
      int block_size,
      bool reorder)
{
  switch (block_size) {
  case 1:
    return scalar_solve(prm, rows, ptr, col, val, rhs, x, reorder);
#if defined(SOLVER_BACKEND_BUILTIN)
    BOOST_PP_SEQ_FOR_EACH(ARCCORE_ALINA_CALL_BLOCK_SOLVER, ~, ARCCORE_ALINA_BLOCK_SIZES)
#endif
  default:
    precondition(false, "Taille de bloc non prise en charge");
    return {};
  }
}

//---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  auto& prof = Alina::Profiler::globalProfiler();
  namespace po = Arcane::ProgramOptions;
  namespace io = Alina::IO;

  using std::string;
  using std::vector;

  po::options_description desc("Options");

  desc.add_options()("help,h", "Affiche cette aide.")("prm-file,P",
                                                  po::value<string>(),
                                                  "Fichier de paramètres au format json. ")(
  "prm,p",
  po::value<vector<string>>()->multitoken(),
  "Paramètres spécifiés sous forme de paires nom=valeur. "
  "Peut être fourni plusieurs fois. Exemples :\n"
  "  -p solver.tol=1e-3\n"
  "  -p precond.coarse_enough=300")("matrix,A",
                                    po::value<string>(),
                                    "Matrice du système au format MatrixMarket. "
                                    "Lorsqu'elle n'est pas spécifiée, elle résout un problème de Poisson dans un cube unitaire 3D. ")(
  "rhs,f",
  po::value<string>(),
  "Le vecteur du membre de droite (RHS) au format MatrixMarket. "
  "Lorsqu'il est omis, un vecteur de uns est utilisé par défaut. "
  "Doit être fourni uniquement avec une matrice de système. ")(
  "f0",
  po::bool_switch()->default_value(false),
  "Utiliser un vecteur RHS nul. Implique --random-initial et solver.ns_search=true")(
  "f1",
  po::bool_switch()->default_value(false),
  "Définir RHS = Ax où x = 1")(
  "null,N",
  po::value<string>(),
  "Les vecteurs proches de l'espace nul au format MatrixMarket. "
  "Doit être une matrice dense de taille N*M, où N est le nombre de "
  "variables inconnues et M est le nombre de vecteurs de l'espace nul. "
  "Doit être fourni uniquement avec une matrice de système. ")(
  "coords,C",
  po::value<string>(),
  "Matrice de coordonnées où le nombre de lignes correspond au nombre de nœuds de grille "
  "et le nombre de colonnes correspond à la dimensionnalité du problème (2 ou 3). "
  "Sera utilisée pour construire des vecteurs proches de l'espace nul comme modes de corps rigide. "
  "Doit être fourni uniquement avec une matrice de système. ")(
  "binary,B",
  po::bool_switch()->default_value(false),
  "Lorsqu'il est spécifié, traiter les fichiers d'entrée comme binaires au lieu de MatrixMarket. "
  "Il est supposé que les fichiers ont été convertis au format binaire avec l'utilitaire mm2bin. ")(
  "scale,s",
  po::bool_switch()->default_value(false),
  "Mettre à l'échelle la matrice de sorte que la diagonale soit unitaire. ")(
  "block-size,b",
  po::value<int>()->default_value(1),
  "La taille du bloc de la matrice du système. "
  "Lorsqu'elle est spécifiée, la matrice du système est supposée avoir une structure par blocs. "
  "C'est généralement le cas pour les problèmes en élasticité, en mécanique des structures, "
  "pour les systèmes couplés d'EDP (tels que les équations de Navier-Stokes), etc. ")(
  "size,n",
  po::value<int>()->default_value(32),
  "La taille du problème de Poisson à résoudre lorsqu'aucune matrice de système n'est fournie. "
  "Spécifié comme le nombre de nœuds de grille le long de chaque dimension d'un cube unitaire. "
  "Le système résultant aura n*n*n inconnues. ")(
  "anisotropy,a",
  po::value<double>()->default_value(1.0),
  "La valeur d'anisotropie pour la valeur de Poisson générée. "
  "Utilisée pour déterminer la mise à l'échelle du problème le long des axes X, Y et Z : "
  "hy = hx * a, hz = hy * a.")(
  "single-level,1",
  po::bool_switch()->default_value(false),
  "Lorsqu'il est spécifié, la hiérarchie AMG n'est pas construite. "
  "Au lieu de cela, le problème est résolu à l'aide d'un lisseur à un niveau comme préconditionneur. ")(
  "reorder,r",
  po::bool_switch()->default_value(false),
  "Lorsqu'il est spécifié, la matrice sera réordonnée pour améliorer la localité du cache")(
  "initial,x",
  po::value<double>()->default_value(0),
  "Valeur à utiliser comme approximation initiale. ")(
  "random-initial",
  po::bool_switch()->default_value(false),
  "Utiliser une approximation initiale aléatoire. ")(
  "output,o",
  po::value<string>(),
  "Fichier de sortie. Sera enregistré au format MatrixMarket. "
  "Lorsqu'il est omis, la solution n'est pas enregistrée. ");

  po::positional_options_description p;
  p.add("prm", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  for (int i = 0; i < argc; ++i) {
    if (i)
      std::cout << " ";
    std::cout << argv[i];
  }
  std::cout << std::endl;

  Alina::PropertyTree prm;
  if (vm.count("prm-file")) {
    prm.read_json(vm["prm-file"].as<string>());
  }

  if (vm.count("prm")) {
    for (const string& v : vm["prm"].as<vector<string>>()) {
      prm.putKeyValue(v);
    }
  }

  size_t rows, nv = 0;
  vector<ptrdiff_t> ptr, col;
  vector<double> val, rhs, null, x;

  if (vm.count("matrix")) {
    auto t = prof.scoped_tic("reading");

    string Afile = vm["matrix"].as<string>();
    bool binary = vm["binary"].as<bool>();

    if (binary) {
      io::read_crs(Afile, rows, ptr, col, val);
    }
    else {
      size_t cols;
      std::tie(rows, cols) = io::mm_reader(Afile)(ptr, col, val);
      precondition(rows == cols, "Matrice de système non carrée");
    }

    if (vm.count("rhs")) {
      string bfile = vm["rhs"].as<string>();

      size_t n, m;

      if (binary) {
        io::read_dense(bfile, n, m, rhs);
      }
      else {
        std::tie(n, m) = io::mm_reader(bfile)(rhs);
      }

      precondition(n == rows && m == 1, "Le vecteur RHS a une taille incorrecte");
    }
    else if (vm["f1"].as<bool>()) {
      rhs.resize(rows);
      for (size_t i = 0; i < rows; ++i) {
        double s = 0;
        for (ptrdiff_t j = ptr[i], e = ptr[i + 1]; j < e; ++j)
          s += val[j];
        rhs[i] = s;
      }
    }
    else {
      rhs.resize(rows, vm["f0"].as<bool>() ? 0.0 : 1.0);
    }

    if (vm.count("null")) {
      string nfile = vm["null"].as<string>();

      size_t m;

      if (binary) {
        io::read_dense(nfile, m, nv, null);
      }
      else {
        std::tie(m, nv) = io::mm_reader(nfile)(null);
      }

      precondition(m == rows, "Les vecteurs proches de l'espace nul ont une taille incorrecte");
    }
    else if (vm.count("coords")) {
      string cfile = vm["coords"].as<string>();
      std::vector<double> coo;

      size_t m, ndim;

      if (binary) {
        io::read_dense(cfile, m, ndim, coo);
      }
      else {
        std::tie(m, ndim) = io::mm_reader(cfile)(coo);
      }

      precondition(m * ndim == rows && (ndim == 2 || ndim == 3), "Matrice de coordonnées de taille incorrecte");

      nv = Alina::rigid_body_modes(ndim, coo, null);
    }

    if (nv) {
      prm.put("precond.coarsening.nullspace.cols", nv);
      prm.put("precond.coarsening.nullspace.rows", rows);
      prm.put("precond.coarsening.nullspace.B", &null[0]);
    }
  }
  else {
    auto t = prof.scoped_tic("assembling");
    rows = sample_problem(vm["size"].as<int>(), val, col, ptr, rhs, vm["anisotropy"].as<double>());
  }

  if (vm["scale"].as<bool>()) {
    std::vector<double> dia(rows, 1.0);

    for (ptrdiff_t i = 0; i < static_cast<ptrdiff_t>(rows); ++i) {
      double d = 1.0;
      for (ptrdiff_t j = ptr[i], e = ptr[i + 1]; j < e; ++j) {
        if (col[j] == i) {
          d = 1 / sqrt(val[j]);
        }
      }
      if (!std::isnan(d))
        dia[i] = d;
    }

    for (ptrdiff_t i = 0; i < static_cast<ptrdiff_t>(rows); ++i) {
      rhs[i] *= dia[i];
      for (ptrdiff_t j = ptr[i], e = ptr[i + 1]; j < e; ++j) {
        val[j] *= dia[i] * dia[col[j]];
      }
    }
  }

  x.resize(rows, vm["initial"].as<double>());
  if (vm["random-initial"].as<bool>() || vm["f0"].as<bool>()) {
    std::mt19937 rng;
    std::uniform_real_distribution<double> rnd(-1, 1);
    for (auto& v : x)
      v = rnd(rng);
  }

  if (vm["f0"].as<bool>()) {
    prm.put("solver.ns_search", true);
  }

  int block_size = vm["block-size"].as<int>();
  std::cout << "BlockSize= " << block_size << "\n";

  if (vm["single-level"].as<bool>())
    prm.put("precond.class", "relaxation");

  String do_hypre_str = Platform::getEnvironmentVariable("ALINA_USE_HYPRE");
  bool do_hypre = false;
  if (auto v = Convert::Type<Int32>::tryParseFromEnvironment("ALINA_USE_HYPRE", true))
    do_hypre = v.value();

  Alina::SolverResult solver_result;
#ifndef SOLVER_BACKEND_BUILTIN
  do_hypre = false;
#endif
  if (do_hypre) {
#ifdef SOLVER_BACKEND_BUILTIN
    _doHypreSolver(rows, ptr, col, val, rhs, x, argc, argv);
#endif
  }
  else {
    solver_result = solve(prm, rows, ptr, col, val, rhs, x, block_size, vm["reorder"].as<bool>());

    if (vm.count("output")) {
      auto t = prof.scoped_tic("write");
      Alina::IO::mm_write(vm["output"].as<string>(), &x[0], x.size());
    }
  }
  std::cout << "Itérations: " << solver_result.nbIteration() << std::endl
            << "Erreur:      " << solver_result.residual() << std::endl
            << prof << std::endl;
}
