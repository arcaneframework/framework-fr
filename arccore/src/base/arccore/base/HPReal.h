// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* HPReal.h                                                    (C) 2000-2026 */
/*                                                                           */
/* Nombre réel de haute précision.                                           */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_HPREAL_H
#define ARCCORE_BASE_HPREAL_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/ArccoreGlobal.h"

#include <cstdlib>
#include <iosfwd>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Classe implémentant un nombre réel de haute précision.
 
 Cette classe est basée sur l'article :

 Obtaining identical results with double precision global accuracy on
 different numbers of processors in parallel particle Monte Carlo
 simulations (Mathew A. Cleveland, Thomas A. Brunner, Nicholas A. Gentile,
 jeffrey A. Keasler) in Journal Of Computational Physics 251 (2013) 223-236.

 Les opérations possibles sont l'accumulation (opérateur+=()), la réduction (reduce()) et la conversion en Real (toReal()).
 D'autres opérations telles que la division ou la multiplication ne sont pas possibles.

 Pour être conforme au type 'Real' classique, le constructeur par défaut
 de cette classe n'effectue aucune initialisation.

 La méthode toReal() permet de convertir le HPReal en un Real classique.
 L'utilisation typique est la suivante :

 \code
 * HPReal r(0.0);
 * ArrayView<Real> x = ...;
 * for( Integer i=0, n=x.size(); i<n; ++i ){
 *   // Accumule la valeur
 *   r += x[i];
 * }
 * Real final_r = r.toReal();
 \endcode

 Évolution 12/18/18 :

 Correction de l'erreur de l'algorithme de base pour HPReal statique _doTwoSum(Real a, Real b)

   Real sum_error = (a - (value-approx_b) + (b-approx_b));

 est devenu (conformément à la Ref 1)

   Real sum_error = (a - (value-approx_b)) + (b-approx_b);

 Correction de l'erreur dans HPReal statique _doQuickTwoSum(Real a,Real b)

   il est nécessaire de tester si a>b et d'inverser a et b si ce n'est pas le cas (voir Ref 2, 3, 4)

 Modification de Accumulate et Reduce,
 Accumulate et Reduce étaient basés sur la Ref 1, qui est basée sur la Ref 4

 Ajout des produits tels que proposés dans la Ref 2 (conformément à la Ref 5)
 de HPReal*Real ou HPReal*HPReal (nous prenons la même valeur de p pour la
 fonction SPLIT() que dans la Ref 2, 4, 5)

 Notez que le produit Real*Real dans la Ref 4 (p.4) n'est pas écrit de la même manière que ceux dans la Ref 2 (p.2) ou 5 (p.3)

 +++++++++++++++++

 Globalement, les algorithmes de réduction ou les erreurs accumulées uniquement par addition sont moins bons

 L'algorithme 6 de DD_TWOSUM [Ref 2] est également moins bon :

 \code
 static HPReal accumulate(Real a,MYHPReal b)
 {
   HPReal x(_doTwoSum(a,b.value()));
   Real d= x.correction() + b.correction();
   HPReal u(_doQuickTwoSum(x.value(),d));
   return _doQuickTwoSum(u.value(),u.correction());
 }
 \endcode

 ou

 \code
 HPReal  reduce(HPReal a,HPReal b)
 {
   HPReal x(_doTwoSum(a.value(),b.value()));
   HPReal y(_doTwoSum(a.correction(),b.correction()));
   Real d= x.correction() + y.value();
   HPReal u(_doQuickTwoSum(x.value(),d));
   Real w=y.correction()+u.correction();
   return _doQuickTwoSum(u.value(),w);
 }
 \endcode

 Des tests sur des millions de valeurs positives ainsi que négatives ont été effectués
 pour l'addition, la multiplication et la division dans différents ordres.

 Je n'ai pas réussi à montrer de différence de résultats dans un cas simple entre l'algorithme proposé pour la somme et celui initialement codé.

 Références
 ----------

 Ref 1
  Obtaining identical results with double precision global accuracy on
  different numbers of processors in parallel particle Monte Carlo
  simulations (Mathew A. Cleveland, Thomas A. Brunner, Nicholas A. Gentile,
  jeffrey A. Keasler) in Journal Of Computational Physics 251 (2013) 223-236.

 Ref 2
   Automatic Source-to-Source error compensation of floating-point programs
   L.Thevenoux, Ph langlois, Mathieu Martel
   HAL Id: hal-01158399

 Ref 3
   Numerical validation of compensated summation algorithms withs stochastic arithmetic
   S.Gaillat, F.Jézéquel , R.Picot
   Published in Electronic Notes in Theoritical Computer Science

   ou

   Numerical validation of compensated algorithms withs stochastic arithmetic
   S.Gaillat, F.Jézéquel , R.Picot
   Published in Applied Mathematics and Computation 329 (2018)339-363

 Ref 4
   Library for Double-Double and Quad-Double Arithmetic
   December 29, 2007
   Yozo Hida Xiaoye Li D.H.Bailey

 Ref 5
   Accurate floating point Product and exponentation
   S.Graillat
   HAL ID: hal-00164607

 Ref 6
   A floating point technique for extending the avaible precision
   T.J.Dekker
   Numeri.Math 18, 224-242 (1971)
*/
class ARCCORE_BASE_EXPORT HPReal
{
 public:

  /*!
   * \brief Constructeur par défaut sans initialisation.
   */
  HPReal() {}

  //! Crée un HP real avec la valeur \a value et la correction \a correction
  explicit HPReal(double avalue)
  : m_value(avalue)
  , m_correction(0.0)
  {}

  //! Crée un HP real avec la valeur \a value et la correction \a correction
  HPReal(double avalue, double acorrection)
  : m_value(avalue)
  , m_correction(acorrection)
  {}

  //! Valeur interne. Généralement, vous devez utiliser toReal()
  Real value() const { return m_value; }

  //! Correction interne.
  Real correction() const { return m_correction; }

  //! Ajoute un Real tout en préservant l'erreur.
  void operator+=(Real v)
  {
    *this = accumulate(v, *this);
  }

  //! Ajoute un HPReal \a v tout en préservant l'erreur (réduction)
  inline void operator+=(HPReal v)
  {
    *this = reduce(*this, v);
  }

  //! Convertit l'instance en un Real.
  inline Real toReal() const
  {
    return value() + correction();
  }

  //! Ajoute un HPReal \a v tout en préservant l'erreur (réduction)
  HPReal reduce(HPReal b) const
  {
    return reduce(*this, b);
  }

  //! Multiplie un Real tout en préservant l'erreur.
  void operator*=(Real v)
  {
    *this = product(v, *this);
  }

  //! Multiplie un HPReal \a v tout en préservant l'erreur (réduction)
  inline void operator*=(HPReal v)
  {
    *this = product(*this, v);
  }

  //! Divise un Real tout en préservant l'erreur.
  void operator/=(Real v)
  {
    *this = div2(v, *this);
  }

  //! Divise un HPReal \a v tout en préservant l'erreur (réduction)
  inline void operator/=(HPReal v)
  {
    *this = div2(*this, v);
  }

  /*!
   * \brief Lit un HPReal à partir du flux \a i.
   * La paire est lue sous la forme de deux valeurs de type #Real.
   */
  std::istream& assign(std::istream& i);
  //! Écrit l'instance dans le flux \a o lisible par assign()
  std::ostream& print(std::ostream& o) const;
  //! Écrit l'instance dans le flux \a o sous la forme (x,y)
  std::ostream& printPretty(std::ostream& o) const;

 public:

  //! Valeur zéro.
  static HPReal zero() { return HPReal(0.0); }

 public:

  static HPReal accumulate(Real a, HPReal b)
  {
    HPReal x(_doTwoSum(a, b.value()));
    Real c = x.correction() + b.correction();
    return _doQuickTwoSum(x.value(), c);
  }

  // Passage Mpi
  static HPReal reduce(HPReal a, HPReal b)
  {
    HPReal x(_doTwoSum(a.value(), b.value()));
    Real c = x.correction() + a.correction() + b.correction();
    return _doQuickTwoSum(x.value(), c);
  }

  // algo 11 de AC_TWOPRODUCTS Ref 2
  static HPReal product(HPReal a, HPReal b)
  {
    HPReal x(_doTwoProducts(a.value(), b.value()));
    Real w = x.correction() + (a.value() * b.correction() + b.value() * a.correction());
    return HPReal(x.value(), w);
  }
  // algo 11 de AC_TWOPRODUCTS Ref 2
  static HPReal product(Real a, HPReal b)
  {
    HPReal x(_doTwoProducts(a, b.value()));
    Real w = x.correction() + (a * b.correction());
    return HPReal(x.value(), w);
  }

  // algo div2 Ref 6    div2 =  a/b
  // J'ai mis des parenthèses dans l'évaluation de w pour forcer un ordre parfois les options du compilateur
  // ATTENTION les 2 dernières lignes de l'algorithme div2 ne sont pas incluses (nous ne renormalisons pas)
  // L'algorithme "mul2" les contient également et ne les a pas incluses pour "product" car l'algorithme 11 de Ref 2 ne les a pas (à mon humble avis)
  static HPReal div2(HPReal a, HPReal b)
  {
    Real c = a.value() / b.value();
    HPReal u(_doTwoProducts(c, b.value()));
    Real w = ((((a.value() - u.value()) + -u.correction()) + a.correction()) - c * b.correction()) / b.value();
    return HPReal(c, w);
  }
  // algo div2 Ref 6     div2 =  a/b
  static HPReal div2(Real b, HPReal a)
  {
    Real c = a.value() / b;
    HPReal u(_doTwoProducts(c, b));
    Real w = (((a.value() - u.value()) + -u.correction()) + a.correction()) / b;
    return HPReal(c, w);
  }

 private:

  Real m_value;
  Real m_correction;

 private:

  // Correction de ()
  static HPReal _doTwoSum(Real a, Real b)
  {
    Real value = a + b;
    Real approx_b = value - a;
    Real sum_error = (a - (value - approx_b)) + (b - approx_b);
    return HPReal(value, sum_error);
  }
  // tests de correction des valeurs de a et b et nous mettons les valeurs absolues
  static HPReal _doQuickTwoSum(Real a1, Real b1)
  {
    Real a = a1;
    Real b = b1;
    if (std::abs(b1) > std::abs(a1)) {
      a = b1;
      b = a1;
    }
    Real value = a + b;
    Real error_value = (b - (value - a));
    return HPReal(value, error_value);
  }
  // algorithme 4 ref 2 ou algorithme 2.4 ref 3 ou algorithme 6 p.4
  static HPReal _doTwoProducts(Real a, Real b)
  {
    Real x = a * b;
    HPReal aw = SPLIT(a);
    HPReal bw = SPLIT(b);
    Real y = aw.correction() * bw.correction() - (((x - aw.value() * bw.value()) - aw.correction() * bw.value()) - aw.value() * bw.correction());
    return HPReal(x, y);
  }
  static HPReal SPLIT(Real a)
  {
    const Real f = 134217729; // 1+ 2^ceil(52/2);
    Real c = f * a;
    Real x = c - (c - a);
    Real y = a - x;
    return HPReal(x, y);
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

inline bool
operator<(const HPReal& a, const HPReal& b)
{
  return a.toReal() < b.toReal();
}

inline bool
operator>(const HPReal& a, const HPReal& b)
{
  return a.toReal() > b.toReal();
}

inline bool
operator==(const HPReal& a, const HPReal& b)
{
  return a.value() == b.value() && a.correction() == b.correction();
}

inline bool
operator!=(const HPReal& a, const HPReal& b)
{
  return !operator==(a, b);
}

inline HPReal
operator+(const HPReal& a, const HPReal& b)
{
  return HPReal::reduce(a, b);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

inline std::ostream&
operator<<(std::ostream& o, HPReal t)
{
  return t.printPretty(o);
}

inline std::istream&
operator>>(std::istream& i, HPReal& t)
{
  return t.assign(i);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
