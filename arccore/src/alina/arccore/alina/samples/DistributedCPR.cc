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
 * qui peut être trouvé à https://github.com/ddemidov/amgcl.
 *
 * Copyright (c) 2012-2022 Denis Demidov <dennis.demidov@gmail.com>
 * SPDX-License-Identifier: MIT
 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

// Pour supprimer les avertissements concernant l'utilisation obsolète d'Eigen.
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wint-in-bool-context"

#include <boost/range/iterator_range.hpp>
#include <boost/scope_exit.hpp>

#include "arccore/alina/IO.h"
#include "arccore/alina/Adapters.h"
#include "arccore/alina/BuiltinBackend.h"
#include "arccore/alina/DistributedPreconditionedSolver.h"
#include "arccore/alina/DistributedCPRPreconditioner.h"
#include "arccore/alina/DistributedAMG.h"
#include "arccore/alina/DistributedCoarseningRuntime.h"
#include "arccore/alina/DistributedRelaxationRuntime.h"
#include "arccore/alina/DistributedSolverRuntime.h"
#include "arccore/alina/DistributedDirectSolverRuntime.h"
#include "arccore/alina/MatrixPartitionerRuntime.h"
#include "arccore/alina/Profiler.h"
#include "arccore/alina/AlinaUtils.h"

#include "arccore/common/internal/ProgramOptions.h"

using namespace Arcane;
using namespace Arcane::Alina;

using Alina::precondition;

//---------------------------------------------------------------------------
ptrdiff_t
read_matrix_market(Alina::mpi_communicator comm,
                   const std::string& A_file, const std::string& rhs_file, int block_size,
                   std::vector<ptrdiff_t>& ptr,
                   std::vector<ptrdiff_t>& col,
                   std::vector<double>& val,
                   std::vector<double>& rhs)
{
  Alina::IO::mm_reader A_mm(A_file);
  ptrdiff_t n = A_mm.rows();

  ptrdiff_t chunk = (n + comm.size - 1) / comm.size;
  if (chunk % block_size != 0) {
    chunk += block_size - chunk % block_size;
  }

  ptrdiff_t row_beg = std::min(n, chunk * comm.rank);
  ptrdiff_t row_end = std::min(n, row_beg + chunk);

  chunk = row_end - row_beg;

  A_mm(ptr, col, val, row_beg, row_end);

  if (rhs_file.empty()) {
    rhs.resize(chunk);
    std::fill(rhs.begin(), rhs.end(), 1.0);
  }
  else {
    Alina::IO::mm_reader rhs_mm(rhs_file);
    rhs_mm(rhs, row_beg, row_end);
  }

  return chunk;
}

//---------------------------------------------------------------------------
ptrdiff_t
read_binary(Alina::mpi_communicator comm,
            const std::string& A_file, const std::string& rhs_file, int block_size,
            std::vector<ptrdiff_t>& ptr,
            std::vector<ptrdiff_t>& col,
            std::vector<double>& val,
            std::vector<double>& rhs)
{
  ptrdiff_t n = Alina::IO::crs_size<ptrdiff_t>(A_file);

  ptrdiff_t chunk = (n + comm.size - 1) / comm.size;
  if (chunk % block_size != 0) {
    chunk += block_size - chunk % block_size;
  }

  ptrdiff_t row_beg = std::min(n, chunk * comm.rank);
  ptrdiff_t row_end = std::min(n, row_beg + chunk);

  chunk = row_end - row_beg;

  Alina::IO::read_crs(A_file, n, ptr, col, val, row_beg, row_end);

  if (rhs_file.empty()) {
    rhs.resize(chunk);
    std::fill(rhs.begin(), rhs.end(), 1.0);
  }
  else {
    ptrdiff_t rows, cols;
    Alina::IO::read_dense(rhs_file, rows, cols, rhs, row_beg, row_end);
  }

  return chunk;
}

//---------------------------------------------------------------------------
template <class Backend, class Matrix>
std::shared_ptr<Alina::DistributedMatrix<Backend>>
partition(Alina::mpi_communicator comm, const Matrix& Astrip,
          std::vector<double>& rhs, const typename Backend::params& bprm,
          Alina::eMatrixPartitionerType ptype, int block_size = 1)
{
  auto& prof = Alina::Profiler::globalProfiler();
  typedef Alina::DistributedMatrix<Backend> DMatrix;

  auto A = std::make_shared<DMatrix>(comm, Astrip);

  if (comm.size == 1 || ptype == Alina::eMatrixPartitionerType::merge)
    return A;

  prof.tic("partition");
  Alina::PropertyTree prm;
  prm.put("type", ptype);
  Alina::MatrixPartitionerRuntime<Backend> part(prm);

  auto I = part(*A, block_size);
  auto J = transpose(*I);
  A = product(*J, *product(*A, *I));

  std::vector<double> new_rhs(J->loc_rows());

  J->move_to_backend(bprm);

  Alina::backend::spmv(1, *J, rhs, 0, new_rhs);
  rhs.swap(new_rhs);
  prof.toc("partition");

  return A;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  auto& prof = Alina::Profiler::globalProfiler();
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  BOOST_SCOPE_EXIT(void)
  {
    MPI_Finalize();
  }
  BOOST_SCOPE_EXIT_END

  Alina::mpi_communicator comm(MPI_COMM_WORLD);

  if (comm.rank == 0)
    std::cout << "World size: " << comm.size << std::endl;

  // Lit la configuration à partir de la ligne de commande
  namespace po = Arcane::ProgramOptions;
  po::options_description desc("Options");

  desc.add_options()("help,h", "affiche l'aide")("matrix,A",
                                            po::value<std::string>(),
                                            "Matrice du système au format MatrixMarket. "
                                            "Si non spécifié, un problème de Poisson dans un cube unitaire 3D est assemblé. ")(
  "rhs,f",
  po::value<std::string>()->default_value(""),
  "Le vecteur du membre de droite (RHS) au format MatrixMarket. "
  "S'il est omis, un vecteur de uns est utilisé par défaut. "
  "Ne doit être fourni qu'avec une matrice de système. ")(
  "binary,B",
  po::bool_switch()->default_value(false),
  "Si spécifié, traiter les fichiers d'entrée comme binaires au lieu de MatrixMarket. "
  "Il est supposé que les fichiers ont été convertis au format binaire avec l'utilitaire mm2bin. ")(
  "block-size,b",
  po::value<int>()->default_value(1),
  "La taille de bloc de la matrice de système. ")(
  "partitioner,r",
  po::value<Alina::eMatrixPartitionerType>()->default_value(
#if defined(ARCCORE_ALINA_HAVE_PARMETIS)
  Alina::eMatrixPartitionerType::parmetis
#endif
  ),
  "Répartition de la matrice de système")("prm-file,P",
                                   po::value<std::string>(),
                                   "Fichier de paramètres au format json. ")(
  "prm,p",
  po::value<std::vector<std::string>>()->multitoken(),
  "Paramètres spécifiés sous forme de paires nom=valeur. "
  "Peut être fourni plusieurs fois. Exemples :\n"
  "  -p solver.tol=1e-3\n"
  "  -p precond.coarse_enough=300");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    if (comm.rank == 0)
      std::cout << desc << std::endl;
    return 0;
  }

  Alina::PropertyTree prm;
  if (vm.count("prm-file")) {
    prm.read_json(vm["prm-file"].as<std::string>());
  }

  if (vm.count("prm")) {
    for (const std::string& v : vm["prm"].as<std::vector<std::string>>()) {
      prm.putKeyValue(v);
    }
  }

  ptrdiff_t n;
  std::vector<ptrdiff_t> ptr;
  std::vector<ptrdiff_t> col;
  std::vector<double> val;
  std::vector<double> rhs;

  int block_size = vm["block-size"].as<int>();
  prm.put("precond.block_size", block_size);

  prof.tic("read");
  if (vm["binary"].as<bool>()) {
    n = read_binary(comm,
                    vm["matrix"].as<std::string>(),
                    vm["rhs"].as<std::string>(),
                    block_size, ptr, col, val, rhs);
  }
  else {
    n = read_matrix_market(comm,
                           vm["matrix"].as<std::string>(),
                           vm["rhs"].as<std::string>(),
                           block_size, ptr, col, val, rhs);
  }
  prof.toc("read");

  typedef Alina::BuiltinBackend<double> Backend;

  auto A = partition<Backend>(comm,
                              std::tie(n, ptr, col, val), rhs, Backend::params(),
                              vm["partitioner"].as<Alina::eMatrixPartitionerType>(),
                              block_size);

  prof.tic("setup");

  using AMG = DistributedAMG<Backend,
                             DistributedCoarseningRuntime<Backend>,
                             DistributedRelaxationRuntime<Backend>,
                             DistributedDirectSolverRuntime<Backend>,
                             MatrixPartitionerRuntime<Backend>>;

  typedef DistributedPreconditionedSolver<
    DistributedCPRPreconditioner<
      AMG,
      AsDistributedPreconditioner<DistributedRelaxationRuntime<Backend>>>,
    DistributedSolverRuntime<Backend>>
  Solver;

  Solver solve(comm, A, prm);
  prof.toc("setup");

  if (comm.rank == 0)
    std::cout << solve << std::endl;

  std::vector<double> x(rhs.size(), 0.0);

  prof.tic("solve");
  Alina::SolverResult r = solve(rhs, x);
  prof.toc("solve");

  if (comm.rank == 0) {
    std::cout << "Itérations: " << r.nbIteration() << std::endl
              << "Erreur:      " << r.residual() << std::endl
              << prof << std::endl;
  }
}
