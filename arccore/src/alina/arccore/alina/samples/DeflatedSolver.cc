// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
 * Ce fichier est basé sur le travail effectué sur la bibliothèque AMGCL (version Mars 2026)
 * qui peut être trouvé à https://github.com/ddemidov/amgcl.
 *
 * Copyright (c) 2012-2022 Denis Demidov <dennis.demidov@gmail.com>
 * SPDX-License-Identifier: MIT
 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include <iostream>
#include <string>

#include "arccore/alina/BuiltinBackend.h"
#include "arccore/alina/RelaxationRuntime.h"
#include "arccore/alina/CoarseningRuntime.h"
#include "arccore/alina/SolverRuntime.h"
#include "arccore/alina/PreconditionerRuntime.h"
#include "arccore/alina/DeflatedSolver.h"
#include "arccore/alina/AMG.h"
#include "arccore/alina/Adapters.h"
#include "arccore/alina/IO.h"
#include "arccore/alina/Profiler.h"

#include "arccore/common/internal/ProgramOptions.h"

using namespace Arcane;

using Alina::precondition;

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
  "Peut être fourni plusieurs fois. Exemples:\n"
  "  -p solver.tol=1e-3\n"
  "  -p precond.coarse_enough=300")("matrix,A",
                                    po::value<string>()->required(),
                                    "Matrice du système au format MatrixMarket.")(
  "rhs,f",
  po::value<string>(),
  "Le vecteur RHS au format MatrixMarket. "
  "S'il est omis, un vecteur de uns est utilisé par défaut. "
  "Ne doit être fourni qu'avec une matrice de système. ")(
  "defvec,D",
  po::value<string>(),
  "Les vecteurs de l'espace nul proche au format MatrixMarket. ")(
  "coords,C",
  po::value<string>(),
  "Matrice de coordonnées où le nombre de lignes correspond au nombre de nœuds de grille "
  "et le nombre de colonnes correspond à la dimensionnalité du problème (2 ou 3). "
  "Sera utilisée pour construire des vecteurs de l'espace nul proche en tant que modes de corps rigide. ")(
  "binary,B",
  po::bool_switch()->default_value(false),
  "Lorsqu'il est spécifié, traite les fichiers d'entrée comme binaires au lieu de MatrixMarket. "
  "Il est supposé que les fichiers ont été convertis au format binaire avec l'utilitaire mm2bin. ")(
  "single-level,1",
  po::bool_switch()->default_value(false),
  "Lorsqu'il est spécifié, la hiérarchie AMG n'est pas construite. "
  "Au lieu de cela, le problème est résolu en utilisant un lisseur de niveau unique comme préconditionneur. ")(
  "output,o",
  po::value<string>(),
  "Fichier de sortie. Sera enregistré au format MatrixMarket. "
  "S'il est omis, la solution n'est pas enregistrée. ");

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

  if (!vm.count("defvec") && !vm.count("coords")) {
    std::cerr << "Soit defvec soit coords doit être fourni" << std::endl;
    return 1;
  }

  ptrdiff_t rows, nv;
  vector<ptrdiff_t> ptr, col;
  vector<double> val, rhs, z;

  {
    auto t = prof.scoped_tic("reading");

    string Afile = vm["matrix"].as<string>();
    bool binary = vm["binary"].as<bool>();

    if (binary) {
      io::read_crs(Afile, rows, ptr, col, val);
    }
    else {
      ptrdiff_t cols;
      std::tie(rows, cols) = io::mm_reader(Afile)(ptr, col, val);
      precondition(rows == cols, "Matrice de système non carrée");
    }

    if (vm.count("rhs")) {
      string bfile = vm["rhs"].as<string>();

      ptrdiff_t n, m;

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

    if (vm.count("defvec")) {
      string nfile = vm["defvec"].as<string>();
      std::vector<double> N;

      ptrdiff_t m;

      if (binary) {
        io::read_dense(nfile, m, nv, N);
      }
      else {
        std::tie(m, nv) = io::mm_reader(nfile)(N);
      }

      precondition(m == rows, "Les vecteurs de déflation ont une taille incorrecte");

      z.resize(N.size());
      for (ptrdiff_t i = 0; i < rows; ++i)
        for (ptrdiff_t j = 0; j < nv; ++j)
          z[i + j * rows] = N[i * nv + j];
    }
    else if (vm.count("coords")) {
      string cfile = vm["coords"].as<string>();
      std::vector<double> coo;

      ptrdiff_t m, ndim;

      if (binary) {
        io::read_dense(cfile, m, ndim, coo);
      }
      else {
        std::tie(m, ndim) = io::mm_reader(cfile)(coo);
      }

      precondition(m * ndim == rows && (ndim == 2 || ndim == 3), "La matrice de coordonnées a une taille incorrecte");

      nv = Alina::rigid_body_modes(ndim, coo, z, /*transpose = */ true);
    }

    prm.put("nvec", nv);
    prm.put("vec", z.data());
  }

  std::vector<double> x(rows, 0);

  if (vm["single-level"].as<bool>())
    prm.put("precond.class", "relaxation");

  typedef Alina::BuiltinBackend<double> Backend;
  typedef Alina::DeflatedSolver<Alina::PreconditionerRuntime<Backend>,
                                Alina::SolverRuntime<Backend>>
  Solver;

  auto A = std::tie(rows, ptr, col, val);

  prof.tic("setup");
  Solver solve(A, prm);
  prof.toc("setup");

  prof.tic("solve");
  Alina::SolverResult result = solve(rhs, x);
  prof.toc("solve");

  if (vm.count("output")) {
    auto t = prof.scoped_tic("write");
    Alina::IO::mm_write(vm["output"].as<string>(), x.data(), x.size());
  }

  std::vector<double> r(rows);
  Alina::backend::residual(rhs, A, x, r);

  std::cout << "Itérations: " << result.nbIteration() << std::endl
            << "Erreur:      " << result.residual() << std::endl
            << "Erreur réelle: " << sqrt(Alina::backend::inner_product(r, r)) / sqrt(Alina::backend::inner_product(rhs, rhs))
            << prof << std::endl;
}
