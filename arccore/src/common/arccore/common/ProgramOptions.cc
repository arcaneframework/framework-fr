// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* ProgramOptions.cc                                           (C) 2000-2026 */
/*                                                                           */
/* Implémentation de l'analyseur d'options de programme.                     */
/*---------------------------------------------------------------------------*/

#include "arccore/common/internal/ProgramOptions.h"

#include <cctype>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::ProgramOptions
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace
{

  void
  _parseNameString(const std::string& name_str, std::string& long_name,
                   std::string& short_name, bool& has_short)
  {
    auto pos = name_str.find(',');
    if (pos != std::string::npos) {
      long_name = name_str.substr(0, pos);
      short_name = name_str.substr(pos + 1);
      has_short = true;
    }
    else {
      long_name = name_str;
      has_short = false;
    }
  }

} // anonymous namespace

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

options_description::
options_description(const std::string& title)
: m_title(title)
{}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

options_description::options_proxy options_description::
add_options()
{
  return options_proxy(*this);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void options_description::
add_option(const std::string& name,
           std::shared_ptr<option_value> value_semantic,
           const std::string& description)
{
  option_descriptor opt;
  _parseNameString(name, opt.long_name, opt.short_name, opt.has_short);
  opt.description = description;
  opt.value_semantic = std::move(value_semantic);
  m_options.push_back(std::move(opt));
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

const option_descriptor* options_description::
find(const std::string& name) const
{
  for (const auto& opt : m_options) {
    if (opt.long_name == name)
      return &opt;
  }
  return nullptr;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

const option_descriptor* options_description::
find_by_short(char short_name) const
{
  for (const auto& opt : m_options) {
    if (opt.has_short && !opt.short_name.empty() && opt.short_name[0] == short_name)
      return &opt;
  }
  return nullptr;
}

/*---------------------------------------------------------------------------*/
// options_proxy: no-value option (bare flag, e.g. ("help,h", "description"))
options_description::options_proxy& options_description::options_proxy::
operator()(const char* name, const char* description)
{
  m_desc.add_option(name, std::make_shared<untyped_value>(), description);
  return *this;
}

/*---------------------------------------------------------------------------*/
// options_proxy: bool option with description
options_description::options_proxy& options_description::options_proxy::
operator()(const char* name,
           const typed_value<bool>* value,
           const char* description)
{
  m_desc.add_option(name,
                    std::shared_ptr<option_value>(const_cast<typed_value<bool>*>(value)),
                    description);
  return *this;
}

/*---------------------------------------------------------------------------*/
// options_proxy: bool option without description
options_description::options_proxy& options_description::options_proxy::
operator()(const char* name,
           const typed_value<bool>* value)
{
  m_desc.add_option(name,
                    std::shared_ptr<option_value>(const_cast<typed_value<bool>*>(value)),
                    std::string());
  return *this;
}

/*---------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const options_description& desc)
{
  if (!desc.m_title.empty())
    os << desc.m_title << ":\n";

  for (const auto& opt : desc.m_options) {
    os << "  ";

    if (opt.has_short && !opt.short_name.empty()) {
      if (opt.short_name.size() == 1 && std::isalnum(static_cast<unsigned char>(opt.short_name[0])))
        os << "-" << opt.short_name << " ";
      else
        os << "--" << opt.short_name << " ";
    }

    os << "[--" << opt.long_name;

    if (opt.value_semantic && !opt.value_semantic->is_bool_switch())
      os << " arg";

    os << "]";

    if (opt.value_semantic && opt.value_semantic->has_default()) {
      auto ds = opt.value_semantic->default_string();
      if (!ds.empty())
        os << " (=" << ds << ")";
    }

    if (!opt.description.empty())
      os << "\t" << opt.description;

    os << "\n";
  }

  return os;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

positional_options_description& positional_options_description::
add(const std::string& name, int count)
{
  m_options.emplace_back(name, count);
  return *this;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void variables_map::
add(const std::string& name, variable_value value)
{
  m_values[name] = std::move(value);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

bool variables_map::
count(const std::string& name) const
{
  return m_values.find(name) != m_values.end();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

const variables_map::variable_value& variables_map::
operator[](const std::string& name) const
{
  static variable_value empty_value;
  auto it = m_values.find(name);
  if (it != m_values.end())
    return it->second;
  return empty_value;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void variables_map::
set_semantic(const std::string& name,
             std::shared_ptr<option_value> semantic)
{
  auto it = m_values.find(name);
  if (it != m_values.end())
    it->second.set_semantic(std::move(semantic));
}

/*---------------------------------------------------------------------------*/
/*!
 * \brief Algorithme d'analyse principal.
 *
 * Itère sur argv[1..argc-1] et reconnaît :
 *   - Options longues (--name ou --name=value)
 *   - Options courtes (-s ou -svalue)
 *   - Arguments positionnels (sans tiret initial)
 *   -- comme marqueur de fin des options
 */
parsed_options command_line_parser::
run()
{
  parsed_options result(m_desc);

  if (!m_desc)
    return result;

  std::vector<std::string> positional_args;
  bool end_of_options = false;

  for (int i = 1; i < m_argc; ++i) {
    std::string token(m_argv[i]);

    if (end_of_options || token.empty() || token[0] != '-') {
      positional_args.push_back(token);
      continue;
    }

    // Marqueur de fin des options
    if (token == "--") {
      end_of_options = true;
      continue;
    }

    if (token.size() >= 2 && token[1] == '-') {
      // Option longue : --name ou --name=value
      std::string name;
      std::string value;
      bool has_eq_value = false;

      auto eq_pos = token.find('=', 2);
      if (eq_pos != std::string::npos) {
        name = token.substr(2, eq_pos - 2);
        value = token.substr(eq_pos + 1);
        has_eq_value = true;
      }
      else {
        name = token.substr(2);
      }

      if (name.empty())
        continue;

      const auto* opt = m_desc->find(name);
      if (!opt || !opt->value_semantic)
        continue;

      if (opt->value_semantic->is_bool_switch()) {
        result.add_option(name, { has_eq_value ? value : "true" });
      }
      else if (opt->value_semantic->is_multitoken()) {
        std::vector<std::string> values;
        if (has_eq_value)
          values.push_back(value);
        while (i + 1 < m_argc) {
          std::string next(m_argv[i + 1]);
          if (!next.empty() && next[0] == '-' && next.size() > 1)
            break;
          ++i;
          values.push_back(next);
        }
        result.add_option(name, std::move(values));
      }
      else {
        if (has_eq_value) {
          result.add_option(name, { value });
        }
        else if (i + 1 < m_argc) {
          ++i;
          result.add_option(name, { std::string(m_argv[i]) });
        }
      }
    }
    else {
      // Option courte : -s
      if (token.size() < 2)
        continue;

      char short_char = token[1];
      const auto* opt = m_desc->find_by_short(short_char);
      if (!opt || !opt->value_semantic)
        continue;

      std::string name = opt->long_name;
      std::string inline_value;
      bool has_inline_value = (token.size() > 2);

      if (has_inline_value)
        inline_value = token.substr(2);

      if (opt->value_semantic->is_bool_switch()) {
        result.add_option(name, { "true" });
      }
      else if (opt->value_semantic->is_multitoken()) {
        std::vector<std::string> values;
        if (has_inline_value)
          values.push_back(inline_value);
        while (i + 1 < m_argc) {
          std::string next(m_argv[i + 1]);
          if (!next.empty() && next[0] == '-' && next.size() > 1)
            break;
          ++i;
          values.push_back(next);
        }
        result.add_option(name, std::move(values));
      }
      else {
        if (has_inline_value) {
          result.add_option(name, { inline_value });
        }
        else if (i + 1 < m_argc) {
          ++i;
          result.add_option(name, { std::string(m_argv[i]) });
        }
      }
    }
  }

  // Associer les arguments positionnels aux options nommées
  if (m_positional && !positional_args.empty()) {
    size_t pos_idx = 0;
    for (const auto& pos_opt : m_positional->options()) {
      if (pos_idx >= positional_args.size())
        break;

      if (pos_opt.second == -1) {
        // Tous les arguments positionnels restants
        std::vector<std::string> remaining(positional_args.begin() + pos_idx,
                                           positional_args.end());
        result.add_option(pos_opt.first, std::move(remaining));
        break;
      }
      else {
        int count = pos_opt.second;
        std::vector<std::string> values;
        for (int c = 0; c < count && pos_idx < positional_args.size(); ++c, ++pos_idx)
          values.push_back(positional_args[pos_idx]);
        result.add_option(pos_opt.first, std::move(values));
      }
    }
  }

  return result;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

parsed_options
parse_command_line(int argc, char** argv, const options_description& desc)
{
  command_line_parser parser(argc, argv);
  parser.options(desc);
  return parser.run();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Transfère les options analysées à variables_map.
 *
 * Pour chaque option analysée, convertit les jetons de chaîne en valeurs typées et
 * les stocke. Applique également les valeurs par défaut pour les options définies qui
 * n'ont pas été fournies sur la ligne de commande.
 */
void store(const parsed_options& parsed, variables_map& vm)
{
  // Stocker toutes les valeurs analysées
  for (const auto& po : parsed.options()) {
    const auto* opt_desc = parsed.description().find(po.name);
    if (!opt_desc || !opt_desc->value_semantic)
      continue;

    const auto& semantic = opt_desc->value_semantic;

    if (po.values.empty())
      continue;

    if (semantic->is_multitoken()) {
      std::any value = std::make_any<std::vector<std::string>>(po.values);
      variables_map::variable_value vv(value, false);
      vv.set_semantic(semantic);
      vm.add(po.name, std::move(vv));
    }
    else {
      std::any value = semantic->parse(po.values[0]);
      variables_map::variable_value vv(value, false);
      vv.set_semantic(semantic);
      vm.add(po.name, std::move(vv));
    }
  }

  // Applique les valeurs par défaut pour les options définies non présentes sur la ligne de commande
  for (const auto& opt : parsed.description().options()) {
    if (!vm.count(opt.long_name) && opt.value_semantic && opt.value_semantic->has_default()) {
      std::any default_val = opt.value_semantic->default_value_any();
      variables_map::variable_value vv(default_val, true);
      vv.set_semantic(opt.value_semantic);
      vm.add(opt.long_name, std::move(vv));
    }
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Remplit les variables liées et valider les options requises.
 *
 * Pour chaque valeur stockée, l'assigne à la variable liée (le cas échéant).
 * Lève une exception si une option requise n'a pas de valeur.
 */
void notify(variables_map& vm)
{
  for (auto& [name, val] : vm) {
    if (val.empty())
      continue;
    if (auto semantic = val.semantic())
      semantic->assign_bound(val.value_ref());
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane::ProgramOptions

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
