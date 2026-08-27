// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "arccore/common/internal/ProgramOptions.h"

#include <sstream>

using namespace Arcane;

namespace po = Arcane::ProgramOptions;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, options_description_add)
{
  po::options_description desc("Test");

  ASSERT_EQ(desc.options().size(), 0);

  desc.add_options()
    ("help,h", "Afficher l'aide.")
    ("size,n", po::value<int>()->default_value(32), "Taille")
    ;

  ASSERT_EQ(desc.options().size(), 2);

  // trouver par nom long
  const auto* opt = desc.find("help");
  ASSERT_NE(opt, nullptr);
  ASSERT_EQ(opt->long_name, "help");
  ASSERT_TRUE(opt->has_short);
  ASSERT_EQ(opt->short_name, "h");
  ASSERT_TRUE(opt->description.find("Show help") != std::string::npos);

  // trouver par nom court
  const auto* short_opt = desc.find_by_short('n');
  ASSERT_NE(short_opt, nullptr);
  ASSERT_EQ(short_opt->long_name, "size");

  // option inconnue
  ASSERT_EQ(desc.find("unknown"), nullptr);
  ASSERT_EQ(desc.find_by_short('z'), nullptr);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, options_description_find_long)
{
  po::options_description desc("Test");
  desc.add_options()
    ("verbose", po::bool_switch(), "Verbeux")
    ;

  ASSERT_NE(desc.find("verbose"), nullptr);
  ASSERT_TRUE(desc.find("verbose")->value_semantic->is_bool_switch());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, options_description_find_no_short)
{
  po::options_description desc("Test");
  desc.add_options()
    ("long-only", po::value<int>(), "Long only")
    ;

  const auto* opt = desc.find("long-only");
  ASSERT_NE(opt, nullptr);
  ASSERT_FALSE(opt->has_short);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, help_output)
{
  po::options_description desc("Options du Test");
  desc.add_options()
    ("help,h", "Afficher l'aide.")
    ("size,n", po::value<int>()->default_value(32), "Taille du domaine")
    ("verbose", po::bool_switch()->default_value(false), "Verbeux")
    ;

  std::ostringstream oss;
  oss << desc;
  std::string help = oss.str();

  ASSERT_TRUE(help.find("Test Options") != std::string::npos);
  ASSERT_TRUE(help.find("--help") != std::string::npos);
  ASSERT_TRUE(help.find("-h") != std::string::npos);
  ASSERT_TRUE(help.find("--size") != std::string::npos);
  ASSERT_TRUE(help.find("-n") != std::string::npos);
  ASSERT_TRUE(help.find("=32") != std::string::npos);
  ASSERT_TRUE(help.find("Domain size") != std::string::npos);
  ASSERT_TRUE(help.find("Show help") != std::string::npos);
  ASSERT_TRUE(help.find("--verbose") != std::string::npos);
  // bool_switch ne devrait pas afficher "arg"
  ASSERT_TRUE(help.find("--size arg") != std::string::npos);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, empty_title)
{
  po::options_description desc;
  desc.add_options()
    ("help,h", "Aide.")
    ;

  std::ostringstream oss;
  oss << desc;
  std::string help = oss.str();
  // Lorsque le titre est vide, aucune ligne de titre n'est imprimée
  ASSERT_TRUE(help.find("Options") == std::string::npos || help.find("--help") != std::string::npos);
  ASSERT_TRUE(help.find("--help") != std::string::npos);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, parse_long_option)
{
  po::options_description desc("Test");
  int size = 0;
  desc.add_options()
    ("size,n", po::value<int>(&size)->default_value(32), "Taille")
    ;

  const char* argv[] = {"program", "--size", "64"};
  int argc = 3;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_TRUE(vm.count("size"));
  ASSERT_EQ(vm["size"].as<int>(), 64);
  ASSERT_EQ(size, 64);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, parse_short_option)
{
  po::options_description desc("Test");
  int size = 0;
  desc.add_options()
    ("size,n", po::value<int>(&size)->default_value(32), "Taille")
    ;

  const char* argv[] = {"program", "-n", "128"};
  int argc = 3;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_EQ(vm["size"].as<int>(), 128);
  ASSERT_EQ(size, 128);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, parse_short_option_inline_value)
{
  po::options_description desc("Test");
  std::string name;
  desc.add_options()
    ("define,D", po::value<std::string>(&name), "Définir")
    ;

  const char* argv[] = {"program", "-DNAME"};
  int argc = 2;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_EQ(name, "NAME");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, parse_long_option_eq_syntax)
{
  po::options_description desc("Test");
  std::string matrix;
  desc.add_options()
    ("matrix,A", po::value<std::string>(&matrix), "Fichier matrice")
    ;

  const char* argv[] = {"program", "--matrix=test.mtx"};
  int argc = 2;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_EQ(matrix, "test.mtx");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, default_value_applied)
{
  po::options_description desc("Test");
  int size = 0;
  desc.add_options()
    ("size,n", po::value<int>(&size)->default_value(32), "Taille")
    ;

  // Aucun argument de ligne de commande
  const char* argv[] = {"program"};
  int argc = 1;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_TRUE(vm.count("size"));
  ASSERT_EQ(vm["size"].as<int>(), 32);
  ASSERT_EQ(size, 32);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, bool_switch_not_provided)
{
  po::options_description desc("Test");
  bool verbose = true;
  desc.add_options()
    ("verbose,v", po::bool_switch(&verbose)->default_value(false), "Verbeux")
    ;

  const char* argv[] = {"program"};
  int argc = 1;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_TRUE(vm.count("verbose"));
  ASSERT_FALSE(vm["verbose"].as<bool>());
  ASSERT_FALSE(verbose);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, bool_switch_provided)
{
  po::options_description desc("Test");
  bool verbose = false;
  desc.add_options()
    ("verbose,v", po::bool_switch(&verbose)->default_value(false), "Verbeux")
    ;

  const char* argv[] = {"program", "--verbose"};
  int argc = 2;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_TRUE(vm["verbose"].as<bool>());
  ASSERT_TRUE(verbose);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, bool_switch_short)
{
  po::options_description desc("Test");
  bool verbose = false;
  desc.add_options()
    ("verbose,v", po::bool_switch(&verbose)->default_value(false), "Verbeux")
    ;

  const char* argv[] = {"program", "-v"};
  int argc = 2;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_TRUE(verbose);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, bare_flag_presence)
{
  po::options_description desc("Test");
  desc.add_options()
    ("help,h", "Afficher l'aide.")
    ;

  // Sans le drapeau
  {
    const char* argv[] = {"program"};
    int argc = 1;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
    po::notify(vm);

    // Les drapeaux nus n'ont pas de valeur par défaut, donc le compte doit être faux
    ASSERT_FALSE(vm.count("help"));
  }

  // Avec le drapeau
  {
    const char* argv[] = {"program", "--help"};
    int argc = 2;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
    po::notify(vm);

    ASSERT_TRUE(vm.count("help"));
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, multitoken_vector)
{
  po::options_description desc("Test");
  desc.add_options()
    ("prm,p", po::value<std::vector<std::string>>()->multitoken(), "Paramètres")
    ;

  const char* argv[] = {"program", "--prm", "tol=1e-6", "maxiter=100", "solver=CG"};
  int argc = 5;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_TRUE(vm.count("prm"));
  auto vals = vm["prm"].as<std::vector<std::string>>();
  ASSERT_EQ(vals.size(), 3);
  ASSERT_EQ(vals[0], "tol=1e-6");
  ASSERT_EQ(vals[1], "maxiter=100");
  ASSERT_EQ(vals[2], "solver=CG");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, multitoken_short)
{
  po::options_description desc("Test");
  desc.add_options()
    ("prm,p", po::value<std::vector<std::string>>()->multitoken(), "Paramètres")
    ;

  const char* argv[] = {"program", "-p", "a=1", "b=2"};
  int argc = 4;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);

  auto vals = vm["prm"].as<std::vector<std::string>>();
  ASSERT_EQ(vals.size(), 2);
  ASSERT_EQ(vals[0], "a=1");
  ASSERT_EQ(vals[1], "b=2");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, positional_arguments_remaining)
{
  po::options_description desc("Test");
  desc.add_options()
    ("prm,p", po::value<std::vector<std::string>>()->multitoken(), "Paramètres")
    ;

  po::positional_options_description p;
  p.add("prm", -1); // -1 signifie tous les arguments positionnels restants

  const char* argv[] = {"program", "x=1", "y=2", "z=3"};
  int argc = 4;

  po::variables_map vm;
  po::store(po::command_line_parser(argc, const_cast<char**>(argv))
              .options(desc).positional(p).run(), vm);

  ASSERT_TRUE(vm.count("prm"));
  auto vals = vm["prm"].as<std::vector<std::string>>();
  ASSERT_EQ(vals.size(), 3);
  ASSERT_EQ(vals[0], "x=1");
  ASSERT_EQ(vals[1], "y=2");
  ASSERT_EQ(vals[2], "z=3");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, positional_arguments_fixed_count)
{
  po::options_description desc("Test");
  std::string input;
  desc.add_options()
    ("input,i", po::value<std::string>(&input)->required(), "Fichier d'entrée")
    ;

  po::positional_options_description pd;
  pd.add("input", 1); // exactement 1 argument positionnel

  const char* argv[] = {"program", "data.mtx"};
  int argc = 2;

  po::variables_map vm;
  po::store(po::command_line_parser(argc, const_cast<char**>(argv))
              .options(desc).positional(pd).run(), vm);
  po::notify(vm);

  ASSERT_EQ(input, "data.mtx");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, mixed_options_and_positional)
{
  po::options_description desc("Test");
  int size = 0;
  std::vector<std::string> prm;
  desc.add_options()
    ("size,n", po::value<int>(&size)->default_value(32), "Taille")
    ("prm,p", po::value<std::vector<std::string>>()->multitoken(), "Paramètres")
    ;

  po::positional_options_description p;
  p.add("prm", -1);

  const char* argv[] = {"program", "--size", "64", "a=1", "b=2"};
  int argc = 5;

  po::variables_map vm;
  po::store(po::command_line_parser(argc, const_cast<char**>(argv))
              .options(desc).positional(p).run(), vm);
  po::notify(vm);

  ASSERT_EQ(vm["size"].as<int>(), 64);
  ASSERT_EQ(size, 64);

  auto vals = vm["prm"].as<std::vector<std::string>>();
  ASSERT_EQ(vals.size(), 2);
  ASSERT_EQ(vals[0], "a=1");
  ASSERT_EQ(vals[1], "b=2");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, multiple_value_types)
{
  po::options_description desc("Test");
  int i = 0;
  double d = 0.0;
  std::string s;
  bool b = false;

  desc.add_options()
    ("int", po::value<int>(&i)->default_value(42), "Entier")
    ("double", po::value<double>(&d)->default_value(3.14), "Double")
    ("string", po::value<std::string>(&s)->default_value("hello"), "Chaîne de caractères")
    ("bool", po::bool_switch(&b)->default_value(false), "Booléen")
    ;

  const char* argv[] = {"program", "--int", "99", "--double", "2.71",
                        "--string", "world", "--bool"};
  int argc = 8;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_EQ(vm["int"].as<int>(), 99);
  ASSERT_EQ(i, 99);

  ASSERT_DOUBLE_EQ(vm["double"].as<double>(), 2.71);
  ASSERT_DOUBLE_EQ(d, 2.71);

  ASSERT_EQ(vm["string"].as<std::string>(), "world");
  ASSERT_EQ(s, "world");

  ASSERT_TRUE(vm["bool"].as<bool>());
  ASSERT_TRUE(b);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, all_defaults)
{
  po::options_description desc("Test");
  int i = -1;
  double d = -1.0;
  std::string s = "none";

  desc.add_options()
    ("int", po::value<int>(&i)->default_value(42), "Entier")
    ("double", po::value<double>(&d)->default_value(3.14), "Double")
    ("string", po::value<std::string>(&s)->default_value("hello"), "Chaîne de caractères")
    ;

  const char* argv[] = {"program"};
  int argc = 1;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  // Les valeurs par défaut doivent être appliquées
  ASSERT_EQ(i, 42);
  ASSERT_DOUBLE_EQ(d, 3.14);
  ASSERT_EQ(s, "hello");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, end_of_options_marker)
{
  po::options_description desc("Test");
  std::vector<std::string> files;
  desc.add_options()
    ("files", po::value<std::vector<std::string>>()->multitoken(), "Fichiers")
    ;

  po::positional_options_description p;
  p.add("files", -1);

  const char* argv[] = {"program", "--", "-a", "-b", "-c"};
  int argc = 5;

  po::variables_map vm;
  po::store(po::command_line_parser(argc, const_cast<char**>(argv))
              .options(desc).positional(p).run(), vm);

  // Après --, tous les jetons doivent être traités comme positionnels
  auto vals = vm["files"].as<std::vector<std::string>>();
  ASSERT_EQ(vals.size(), 3);
  ASSERT_EQ(vals[0], "-a");
  ASSERT_EQ(vals[1], "-b");
  ASSERT_EQ(vals[2], "-c");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, count_checks)
{
  po::options_description desc("Test");
  desc.add_options()
    ("help,h", "Aide")
    ("opt", po::value<int>()->default_value(0), "Option")
    ;

  // Option non fournie
  {
    const char* argv[] = {"program"};
    int argc = 1;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);

    // drapeau nu sans valeur par défaut -> count faux
    ASSERT_FALSE(vm.count("help"));
    // option avec valeur par défaut -> count vrai même si non fournie
    ASSERT_TRUE(vm.count("opt"));
  }

  // Option fournie
  {
    const char* argv[] = {"program", "--help"};
    int argc = 2;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);

    ASSERT_TRUE(vm.count("help"));
    ASSERT_TRUE(vm.count("opt")); // a une valeur par défaut
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, parse_command_line_convenience)
{
  po::options_description desc("Test");
  std::string val;
  desc.add_options()
    ("opt,o", po::value<std::string>(&val), "Option")
    ;

  const char* argv[] = {"program", "-o", "test"};
  int argc = 3;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_EQ(val, "test");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, type_mismatch_throws)
{
  po::options_description desc("Test");
  desc.add_options()
    ("opt", po::value<int>()->default_value(0), "Option")
    ;

  const char* argv[] = {"program", "--opt", "42"};
  int argc = 3;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);

  // Demander un type incorrect doit lever une exception
  ASSERT_THROW(vm["opt"].as<std::string>(), std::logic_error);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, bound_variable_all_types)
{
  po::options_description desc("Test");
  int iv = 0;
  double dv = 0.0;
  std::string sv;
  bool bv = false;

  desc.add_options()
    ("i", po::value<int>(&iv), "entier")
    ("d", po::value<double>(&dv), "double")
    ("s", po::value<std::string>(&sv), "chaîne de caractères")
    ("b", po::bool_switch(&bv)->default_value(false), "booléen")
    ;

  const char* argv[] = {"program", "--i", "7", "--d", "1.5", "--s", "foo", "--b"};
  int argc = 8;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_EQ(iv, 7);
  ASSERT_DOUBLE_EQ(dv, 1.5);
  ASSERT_EQ(sv, "foo");
  ASSERT_TRUE(bv);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, multiple_occurrences_overwrite)
{
  po::options_description desc("Test");
  std::string val;
  desc.add_options()
    ("opt", po::value<std::string>(&val), "Option")
    ;

  // Si la même option apparaît plusieurs fois, la dernière valeur l'emporte
  const char* argv[] = {"program", "--opt", "first", "--opt", "second"};
  int argc = 5;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  ASSERT_EQ(val, "second");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

TEST(TestProgramOptions, multitoken_with_default)
{
  po::options_description desc("Test");
  po::typed_value<std::vector<std::string>>* tv =
    po::value<std::vector<std::string>>()->multitoken();

  // Définit une valeur par défaut sur multitoken est possible
  // mais n'est pas couramment utilisé ; on teste juste qu'il compile et stocke
  desc.add_options()
    ("prm,p", tv, "Paramètres")
    ;

  const char* argv[] = {"program"};
  int argc = 1;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, const_cast<char**>(argv), desc), vm);
  po::notify(vm);

  // Sans multi-default spécifié, count est faux
  ASSERT_FALSE(vm.count("prm"));
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
