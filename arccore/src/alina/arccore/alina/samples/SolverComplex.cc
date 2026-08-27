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
 * qui peut être trouvé à https://github.com/ddemidov/amgcl.
 *
 * Copyright (c) 2012-2022 Denis Demidov <dennis.demidov@gmail.com>
 * SPDX-License-Identifier: MIT
 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include <iostream>
#include <string>

#include <boost/range/iterator_range.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#include "arccore/alina/BuiltinBackend.h"
#include "arccore/alina/ValueTypeComplex.h"
#include "arccore/alina/StaticMatrix.h"
#include "arccore/alina/Adapters.h"

#include "arccore/alina/SolverRuntime.h"
#include "arccore/alina/CoarseningRuntime.h"
#include "arccore/alina/RelaxationRuntime.h"
#include "arccore/alina/PreconditionerRuntime.h"
#include "arccore/alina/PreconditionedSolver.h"
#include "arccore/alina/AMG.h"
#include "arccore/alina/IO.h"

#include "arccore/alina/Profiler.h"

#include "arccore/common/internal/ProgramOptions.h"

#include "SampleProblemCommon.h"

#ifndef ARCCORE_ALINA_BLOCK_SIZES
#define ARCCORE_ALINA_BLOCK_SIZES (2)(3)(4)
#endif
using namespace Arcane;
using namespace Arcane::Alina;

using Alina::precondition;

//---------------------------------------------------------------------------
template <class Precond, class Matrix>
Alina::SolverResult
solve(const Matrix& A,
      const Alina::PropertyTree& prm,
      std::vector<std::complex<double>> const& f,
      std::vector<std::complex<double>>& x)
{
  auto& prof = Alina::Profiler::globalProfiler();

  typedef typename Precond::backend_type Backend;

  typedef typename Alina::math::rhs_of<typename Backend::value_type>::type rhs_type;
  size_t n = Alina::backend::nbRow(A);

  rhs_type const* fptr = reinterpret_cast<rhs_type const*>(&f[0]);
  rhs_type* xptr = reinterpret_cast<rhs_type*>(&x[0]);
  SmallSpan<const rhs_type> frng(fptr, n);
  SmallSpan<rhs_type> xrng(xptr, n);

  using Solver = Alina::PreconditionedSolver<Precond, Alina::SolverRuntime<Backend>>;

  prof.tic("setup");
  Solver solve(A, prm);
  prof.toc("setup");

  std::cout << solve << std::endl;

  {
    auto t = prof.scoped_tic("solve");
    return solve(frng, xrng);
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

  desc.add_options()("help,h", "Show this help.")("prm-file,P",
                                                  po::value<string>(),
                                                  "Fichier de paramètres au format json. ")(
  "prm,p",
  po::value<vector<string>>()->multitoken(),
  "Paramètres spécifiés sous forme de paires nom=valeur. "
  "Peut être fourni plusieurs fois. Exemples:\n"
  "  -p solver.tol=1e-3\n"
  "  -p precond.coarse_enough=300")("matrix,A",
                                    po::value<string>(),
                                    "Matrice du système au format MatrixMarket. "
                                    "Si non spécifié, résout un problème de Poisson dans un cube unitaire 3D. ")(
  "rhs,f",
  po::value<string>(),
  "Le vecteur du membre de droite (RHS) au format MatrixMarket. "
  "Si omis, un vecteur de uns est utilisé par défaut. "
  "Ne doit être fourni qu'avec une matrice de système. ")(
  "null,N",
  po::value<string>(),
  "Les vecteurs de l'espace nul proche au format MatrixMarket. "
  "Doit être une matrice dense de taille N*M, où N est le nombre de "
  "variables inconnues et M est le nombre de vecteurs de l'espace nul. "
  "Ne doit être fourni qu'avec une matrice de système. ")(
  "binary,B",
  po::bool_switch()->default_value(false),
  "Lorsqu'il est spécifié, traite les fichiers d'entrée comme binaires au lieu de MatrixMarket. "
  "Il est supposé que les fichiers ont été convertis au format binaire avec l'utilitaire mm2bin. ")(
  "block-size,b",
  po::value<int>()->default_value(1),
  "La taille de bloc de la matrice du système. "
  "Lorsqu'il est spécifié, la matrice du système est supposée avoir une structure par blocs. "
  "C'est généralement le cas pour les problèmes en élasticité, en mécanique des structures, "
  "pour les systèmes couplés d'EDP (tels que les équations de Navier-Stokes), etc. ")(
  "size,n",
  po::value<int>()->default_value(32),
  "La taille du problème de Poisson à résoudre si aucune matrice de système n'est donnée. "
  "Spécifié comme le nombre de nœuds de grille le long de chaque dimension d'un cube unitaire. "
  "Le système résultant aura n*n*n inconnues. ")(
  "single-level,1",
  po::bool_switch()->default_value(false),
  "Lorsqu'il est spécifié, la hiérarchie AMG n'est pas construite. "
  "Au lieu de cela, le problème est résolu en utilisant un lisseur de niveau unique comme préconditionneur. ")(
  "initial,x",
  po::value<double>()->default_value(0),
  "Valeur à utiliser comme approximation initiale. ")(
  "output,o",
  po::value<string>(),
  "Fichier de sortie. Sera enregistré au format MatrixMarket. "
  "Lorsqu'il est omis, la solution n'est pas sauvegardée. ");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  Alina::PropertyTree prm;
  if (vm.count("prm-file")) {
    prm.read_json(vm["prm-file"].as<string>());
  }

  if (vm.count("prm")) {
    for (const string& v : vm["prm"].as<vector<string>>()) {
      prm.putKeyValue(v);
    }
  }

  size_t rows;
  vector<ptrdiff_t> ptr, col;
  vector<std::complex<double>> val, rhs, null, x;

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
    else {
      rhs.resize(rows, 1.0);
    }

    if (vm.count("null")) {
      string nfile = vm["null"].as<string>();

      size_t m, nv;

      if (binary) {
        io::read_dense(nfile, m, nv, null);
      }
      else {
        std::tie(m, nv) = io::mm_reader(nfile)(null);
      }

      precondition(m == rows, "Les vecteurs de l'espace nul proche ont une taille incorrecte");

      prm.put("precond.coarsening.nullspace.cols", nv);
      prm.put("precond.coarsening.nullspace.rows", rows);
      prm.put("precond.coarsening.nullspace.B", &null[0]);
    }
  }
  else {
    auto t = prof.scoped_tic("assembling");
    rows = sample_problem(vm["size"].as<int>(), val, col, ptr, rhs);
  }

  x.resize(rows, vm["initial"].as<double>());

  if (vm["single-level"].as<bool>())
    prm.put("precond.class", "relaxation");

  int block_size = vm["block-size"].as<int>();
  Alina::SolverResult r;
#define CALL_BLOCK_SOLVER(z, data, B) \
  case B: { \
    typedef StaticMatrix<std::complex<double>, B, B> value_type; \
    typedef ::Arcane::Alina::BuiltinBackend<value_type> Backend; \
    r = solve<::Arcane::Alina::PreconditionerRuntime<Backend>>( \
    ::Arcane::Alina::adapter::block_matrix<value_type>( \
    std::tie(rows, ptr, col, val)), \
    prm, rhs, x); \
  } break;

  switch (block_size) {
  case 1: {
    typedef Alina::BuiltinBackend<std::complex<double>> Backend;
    r = solve<PreconditionerRuntime<Backend>>(
    std::tie(rows, ptr, col, val), prm, rhs, x);
  } break;
    BOOST_PP_SEQ_FOR_EACH(CALL_BLOCK_SOLVER, ~, ARCCORE_ALINA_BLOCK_SIZES)
  }

#undef CALL_BLOCK_SOLVER

  if (vm.count("output")) {
    auto t = prof.scoped_tic("write");
    Alina::IO::mm_write(vm["output"].as<string>(), &x[0], x.size());
  }

  std::cout << "Iterations: " << r.nbIteration() << std::endl
            << "Error:      " << r.residual() << std::endl
            << prof << std::endl;
}
