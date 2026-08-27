// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* NumVector.h                                                 (C) 2000-2026 */
/*                                                                           */
/* Vecteur de taille fixe de types numériques.                               */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_NUMVECTOR_H
#define ARCCORE_BASE_NUMVECTOR_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/Real2.h"
#include "arccore/base/Real3.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Petit vecteur de taille fixe de points de données numériques de taille Size.
 *
 * Il est possible d'accéder à chaque composante du vecteur en utilisant 'operator[]'
 * ou 'operator()' ou via les méthodes vx(), vy(), vz() si la dimension est
 * suffisante (par exemple, vz() n'est accessible que si Size>=3.
 */
template <typename T, int Size>
class NumVector
{
  static_assert(Size > 0, "Size has to be strictly greater than 0");

 public:

  using ThatClass = NumVector<T, Size>;
  using DataType = T;
  static constexpr bool isRealType() { return std::is_same_v<T, Real>; }
  struct InitTag
  {};

 public:

  //! Construit le vecteur nul.
  NumVector() = default;

  //! Construit avec le couple (ax,ay)
  constexpr ARCCORE_HOST_DEVICE NumVector(T ax, T ay) requires(Size == 2)

  {
    m_values[0] = ax;
    m_values[1] = ay;
  }

  //! Construit avec le triplet (ax,ay,az)
  constexpr ARCCORE_HOST_DEVICE NumVector(T ax, T ay, T az) requires(Size == 3)

  {
    m_values[0] = ax;
    m_values[1] = ay;
    m_values[2] = az;
  }

  //! Construit avec le quadruplet (a1,a2,a3,a4)
  constexpr ARCCORE_HOST_DEVICE NumVector(T a1, T a2, T a3, T a4) requires(Size == 4)

  {
    m_values[0] = a1;
    m_values[1] = a2;
    m_values[2] = a3;
    m_values[3] = a4;
  }

  //! Construit avec le quintuplet (a1,a2,a3,a4,a5)
  constexpr ARCCORE_HOST_DEVICE NumVector(T a1, T a2, T a3, T a4, T a5) requires(Size == 5)
  {
    m_values[0] = a1;
    m_values[1] = a2;
    m_values[2] = a3;
    m_values[3] = a4;
    m_values[4] = a5;
  }

  //! Construit avec le sextuplet (a1,a2,a3,a4,a5,a6)
  constexpr ARCCORE_HOST_DEVICE NumVector(T a1, T a2, T a3, T a4, T a5, T a6) requires(Size == 6)
  {
    m_values[0] = a1;
    m_values[1] = a2;
    m_values[2] = a3;
    m_values[3] = a4;
    m_values[4] = a5;
    m_values[5] = a6;
  }

  //! Construit l'instance avec la valeur \a v pour chaque composante
  template <bool = true>
  explicit constexpr ARCCORE_HOST_DEVICE NumVector(const T (&v)[Size])
  {
    for (int i = 0; i < Size; ++i)
      m_values[i] = v[i];
  }

  //! Construit l'instance avec la valeur \a v pour chaque composante
  explicit constexpr ARCCORE_HOST_DEVICE NumVector(std::array<T, Size> v)
  {
    for (int i = 0; i < Size; ++i)
      m_values[i] = v[i];
  }

  //! Construit l'instance avec la valeur \a v pour chaque composante
  explicit constexpr ARCCORE_HOST_DEVICE NumVector(T v)
  {
    for (int i = 0; i < Size; ++i)
      m_values[i] = v;
  }

  //! Construit l'instance avec la valeur \a v pour chaque composante
  constexpr ARCCORE_HOST_DEVICE NumVector(const DataType* v,InitTag)
  {
    for (int i = 0; i < Size; ++i)
      m_values[i] = v[i];
  }

  explicit constexpr ARCCORE_HOST_DEVICE NumVector(Real2 v) requires(Size == 2 && isRealType())
  : NumVector(v.x, v.y)
  {}

  explicit constexpr ARCCORE_HOST_DEVICE NumVector(Real3 v) requires(Size == 3 && isRealType())
  : NumVector(v.x, v.y, v.z)
  {}

  //! Assigne une valeur à toutes les composantes du vecteur
  constexpr ARCCORE_HOST_DEVICE NumVector& operator=(const DataType& value)
  {
    for (int i = 0; i < Size; ++i)
      m_values[i] = value;
    return (*this);
  }

  constexpr ARCCORE_HOST_DEVICE NumVector& operator=(const Real2& v)
  requires(Size == 2 && isRealType())
  {
    *this = NumVector(v);
    return (*this);
  }

  constexpr ARCCORE_HOST_DEVICE NumVector& operator=(const Real3& v)
  requires(Size == 3 && isRealType())
  {
    *this = NumVector(v);
    return (*this);
  }

  constexpr operator Real2() const requires(Size == 2)
  {
    return Real2(m_values[0], m_values[1]);
  }

  constexpr operator Real3() const requires(Size == 3)
  {
    return Real3(m_values[0], m_values[1], m_values[2]);
  }

 public:

  constexpr ARCCORE_HOST_DEVICE static NumVector zero() { return NumVector({}); }

 public:

  //! Valeur absolue composante par composante.
  ARCCORE_HOST_DEVICE NumVector absolute() const
  {
    NumVector v;
    for (int i = 0; i < Size; ++i)
      v.m_values[i] = std::abs(m_values[i]);
    return v;
  }

  //! Remplit le vecteur avec la valeur \a v
  constexpr ARCCORE_HOST_DEVICE void fill(const T& v)
  {
    for (int i = 0; i < Size; ++i) {
      m_values[i] = v;
    }
  }

  //! Ajoute \a b à chaque composante de \a a
  friend constexpr ARCCORE_HOST_DEVICE NumVector& operator+=(NumVector& a, T b)
  {
    for (int i = 0; i < Size; ++i)
      a.m_values[i] += b;
    return a;
  }

  //! Ajoute \a b à \a a
  friend constexpr ARCCORE_HOST_DEVICE NumVector& operator+=(NumVector& a, const NumVector& b)
  {
    for (int i = 0; i < Size; ++i)
      a.m_values[i] += b.m_values[i];
    return a;
  }

  //! Soustrait \a b à chaque composante de \a a
  friend constexpr ARCCORE_HOST_DEVICE NumVector& operator-=(NumVector& a, T b)
  {
    for (int i = 0; i < Size; ++i)
      a.m_values[i] -= b;
    return a;
  }

  //! Soustrait \a b à chaque composante de \a a
  friend constexpr ARCCORE_HOST_DEVICE NumVector& operator-=(NumVector& a, const NumVector& b)
  {
    for (int i = 0; i < Size; ++i)
      a.m_values[i] -= b.m_values[i];
    return a;
  }

  //! Multiplie chaque composante de \a a par \a b
  friend constexpr ARCCORE_HOST_DEVICE NumVector& operator*=(NumVector& a, T b)
  {
    for (int i = 0; i < Size; ++i)
      a.m_values[i] *= b;
    return a;
  }

  //! Divise chaque composante de \a a par \a b
  friend constexpr ARCCORE_HOST_DEVICE NumVector& operator/=(NumVector& a, T b)
  {
    for (int i = 0; i < Size; ++i)
      a.m_values[i] /= b;
    return a;
  }

  //! Crée un triplet égal à ce triplet ajouté à \a b
  friend constexpr ARCCORE_HOST_DEVICE NumVector operator+(const NumVector& a, const NumVector& b)
  {
    NumVector v;
    for (int i = 0; i < Size; ++i)
      v.m_values[i] = a.m_values[i] + b.m_values[i];
    return v;
  }

  //! Crée un triplet égal à ce triplet moins \a b
  friend constexpr ARCCORE_HOST_DEVICE NumVector operator-(const NumVector& a, const NumVector& b)
  {
    NumVector v;
    for (int i = 0; i < Size; ++i)
      v.m_values[i] = a.m_values[i] - b.m_values[i];
    return v;
  }

  //! Crée un triplet opposé au triplet actuel
  constexpr ARCCORE_HOST_DEVICE NumVector operator-() const
  {
    NumVector v;
    for (int i = 0; i < Size; ++i)
      v.m_values[i] = -m_values[i];
    return v;
  }

  //! Multiplication par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE NumVector operator*(T a, const NumVector& vec)
  {
    NumVector v;
    for (int i = 0; i < Size; ++i)
      v.m_values[i] = a * vec.m_values[i];
    return v;
  }

  //! Multiplication par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE NumVector operator*(const NumVector& vec, T b)
  {
    NumVector v;
    for (int i = 0; i < Size; ++i)
      v.m_values[i] = vec.m_values[i] * b;
    return v;
  }

  //! Division par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE NumVector operator/(const NumVector& vec, T b)
  {
    NumVector v;
    for (int i = 0; i < Size; ++i)
      v.m_values[i] = vec.m_values[i] / b;
    return v;
  }

  /*!
   * \brief Compare l'instance actuelle composante par composante à \a b.
   *
   * \retval vrai si this.x==b.x et this.y==b.y et this.z==b.z.
   * \retval faux sinon.
   */
  friend constexpr ARCCORE_HOST_DEVICE bool operator==(const NumVector& a, const NumVector& b)
  {
    for (int i = 0; i < Size; ++i)
      if (!_eq(a.m_values[i], b.m_values[i]))
        return false;
    return true;
  }

  friend std::ostream& operator<<(std::ostream& o, const NumVector& t)
  {
    t.print(o);
    return o;
  }

  /*!
   * \brief Compare deux vecteurs
   * Pour la notion d'égalité, voir operator==()
   */
  friend constexpr ARCCORE_HOST_DEVICE bool operator!=(const NumVector& a, const NumVector& b)
  {
    return !(a == b);
  }

  constexpr ARCCORE_HOST_DEVICE T& operator()(Int32 i)
  {
    ARCCORE_CHECK_AT(i, Size);
    return m_values[i];
  }
  constexpr ARCCORE_HOST_DEVICE T operator()(Int32 i) const
  {
    ARCCORE_CHECK_AT(i, Size);
    return m_values[i];
  }
  constexpr ARCCORE_HOST_DEVICE T& operator[](Int32 i)
  {
    ARCCORE_CHECK_AT(i, Size);
    return m_values[i];
  }
  constexpr ARCCORE_HOST_DEVICE DataType operator[](Int32 i) const
  {
    ARCCORE_CHECK_AT(i, Size);
    return m_values[i];
  }

  //! Valeur de la première composante
  constexpr ARCCORE_HOST_DEVICE DataType& vx() requires(Size >= 1)
  {
    return m_values[0];
  }
  //! Valeur de la première composante
  constexpr ARCCORE_HOST_DEVICE DataType vx() const requires(Size >= 1)
  {
    return m_values[0];
  }

  //! Valeur de la deuxième composante
  constexpr ARCCORE_HOST_DEVICE DataType& vy() requires(Size >= 2)
  {
    return m_values[1];
  }
  //! Valeur de la deuxième composante
  constexpr ARCCORE_HOST_DEVICE DataType vy() const requires(Size >= 2)
  {
    return m_values[1];
  }

  //! Valeur de la troisième composante
  constexpr ARCCORE_HOST_DEVICE DataType& vz() requires(Size >= 3)
  {
    return m_values[2];
  }
  //! Valeur de la troisième composante
  constexpr ARCCORE_HOST_DEVICE DataType vz() const requires(Size >= 3)
  {
    return m_values[2];
  }

 private:

  //! Valeurs du vecteur
  T m_values[Size];

 private:

  /*!
   * \brief Compare les valeurs de \a a et \a b en utilisant le comparateur TypeEqualT.
   *
   * \retval vrai si \a a et \a b sont égaux,
   * \retval faux sinon.
   */
  constexpr ARCCORE_HOST_DEVICE static bool
  _eq(T a, T b)
  {
    return a == b;
  }
  template <typename Stream>
  void print(Stream& o) const
  {
    for (int i = 0; i < Size; ++i) {
      if (i != 0)
        o << ' ';
      o << m_values[i];
    }
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
