// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* NumMatrix.h                                                 (C) 2000-2026 */
/*                                                                           */
/* Matrice mathématique de taille fixe de types numériques.                  */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_NUMMATRIX_H
#define ARCCORE_BASE_NUMMATRIX_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/NumVector.h"
#include "arccore/base/Real2x2.h"
#include "arccore/base/Real3x3.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Petite matrice de taille fixe contenant RowSize lignes et ColumnSize colonnes.
 *
 * Il est possible d'accéder à chaque ligne en utilisant la méthode 'row()'
 * ou via les méthodes vx(), vy(), vz() si la dimension est
 * suffisante (par exemple, vz() n'est accessible que si Size>=3.
 */
template <typename T, int RowSize, int ColumnSize>
class NumMatrix
{
  static_assert(RowSize >= 0, "RowSize has to be strictly greater than 0");
  static_assert(ColumnSize >= 0, "RowSize has to be strictly greater than 0");

  static constexpr bool isRealType() { return std::is_same_v<T, Real>; }
  static constexpr bool isSquare() { return RowSize == ColumnSize; }
  static constexpr bool isSquare2() { return RowSize == 2 && ColumnSize == 2; }
  static constexpr bool isSquare3() { return RowSize == 3 && ColumnSize == 3; }
  static constexpr Int32 NbElement = RowSize * ColumnSize;

 public:

  using VectorType = NumVector<T, ColumnSize>;
  using ThatClass = NumMatrix<T, RowSize, ColumnSize>;
  using DataType = T;

 public:

  //! Construit la matrice avec tous les coefficients à zéro.
  NumMatrix() = default;

  //! Construit la matrice avec les lignes (ax, ay)
  constexpr ARCCORE_HOST_DEVICE NumMatrix(const VectorType& ax, const VectorType& ay)
  requires(RowSize == 2)
  {
    setRow(0, ax);
    setRow(1, ay);
  }

  //! Construit la matrice avec les lignes (ax, ay, az)
  constexpr ARCCORE_HOST_DEVICE NumMatrix(const VectorType& ax, const VectorType& ay, const VectorType& az)
  requires(RowSize == 3)
  {
    setRow(0, ax);
    setRow(1, ay);
    setRow(2, az);
  }

  //! Construit la matrice avec les lignes (a1, a2, a3, a4)
  constexpr ARCCORE_HOST_DEVICE NumMatrix(const VectorType& a1, const VectorType& a2,
                                          const VectorType& a3, const VectorType& a4)
  requires(RowSize == 4)
  {
    setRow(0, a1);
    setRow(1, a2);
    setRow(2, a3);
    SetRow(3, a4);
  }

  //! Construit la matrice avec les lignes (a1, a2, a3, a4, a5)
  constexpr ARCCORE_HOST_DEVICE NumMatrix(const VectorType& a1, const VectorType& a2,
                                          const VectorType& a3, const VectorType& a4,
                                          const VectorType& a5)
  requires(RowSize == 5)
  {
    setRow(0, a1);
    setRow(1, a2);
    setRow(2, a3);
    setRow(3, a4);
    setRow(4, a5);
  }

  //! Construit la matrice avec les lignes (a1, a2, a3, a4, a5, a6)
  constexpr ARCCORE_HOST_DEVICE NumMatrix(const VectorType& a1, const VectorType& a2,
                                          const VectorType& a3, const VectorType& a4,
                                          const VectorType& a5, const VectorType& a6)
  requires(RowSize == 6)
  {
    setRow(0, a1);
    setRow(1, a2);
    setRow(2, a3);
    setRow(3, a4);
    setRow(4, a5);
    setRow(5, a6);
  }

  //! Construit l'instance avec le triplet (v, v, v).
  constexpr ARCCORE_HOST_DEVICE explicit NumMatrix(const T& v)
  {
    for (int i = 0; i < NbElement; ++i)
      m_values[i] = v;
  }

  explicit constexpr ARCCORE_HOST_DEVICE NumMatrix(const Real2x2& v)
  requires(isSquare2() && isRealType())
  : NumMatrix(VectorType(v.x), VectorType(v.y))
  {}

  explicit constexpr ARCCORE_HOST_DEVICE NumMatrix(const Real3x3& v)
  requires(isSquare3() && isRealType())
  : NumMatrix(VectorType(v.x), VectorType(v.y), VectorType(v.z))
  {}

  //! Assigne le triplet (v, v, v) à l'instance.
  constexpr ARCCORE_HOST_DEVICE ThatClass& operator=(const DataType& v)
  {
    for (int i = 0; i < NbElement; ++i)
      m_values[i] = v;
    return (*this);
  }

  constexpr ARCCORE_HOST_DEVICE ThatClass& operator=(const Real2x2& v)
  requires(isSquare2() && isRealType())
  {
    *this = ThatClass(v);
    return (*this);
  }

  constexpr ARCCORE_HOST_DEVICE ThatClass& operator=(const Real3x3& v)
  requires(isSquare3() && isRealType())
  {
    *this = ThatClass(v);
    return (*this);
  }

  //! Conversion en Real2x2
  operator Real2x2() const requires(isSquare2())
  {
    return Real2x2::fromLines(m_values[0], m_values[1], m_values[2], m_values[3]);
  }

  //! Conversion en Real3x3
  constexpr operator Real3x3() const requires(isSquare3())
  {
    return Real3x3(constView());
  }

 public:

  //! Construit la matrice nulle
  constexpr ARCCORE_HOST_DEVICE static ThatClass zero()
  {
    return ThatClass({});
  }

  //! Construit la matrice ((ax,bx,cx), (ay,by,cy), (az,bz,cz)).
  constexpr ARCCORE_HOST_DEVICE static ThatClass fromColumns(T ax, T ay, T az, T bx, T by, T bz, T cx, T cy, T cz)
  requires(isSquare3())
  {
    return ThatClass(VectorType(ax, bx, cx), VectorType(ay, by, cy), VectorType(az, bz, cz));
  }

  //! Construit la matrice ((ax,bx,cx), (ay,by,cy), (az,bz,cz)).
  constexpr ARCCORE_HOST_DEVICE static ThatClass fromLines(T ax, T bx, T cx, T ay, T by, T cy, T az, T bz, T cz)
  requires(isSquare3())
  {
    return ThatClass(VectorType(ax, bx, cx), VectorType(ay, by, cy), VectorType(az, bz, cz));
  }

 public:

  //! Ajoute b à a et retourne a.
  friend constexpr ARCCORE_HOST_DEVICE ThatClass& operator+=(ThatClass& a, const ThatClass& b)
  {
    for (int i = 0; i < NbElement; ++i)
      a.m_values[i] += b.m_values[i];
    return a;
  }
  //! Soustrait b de a et retourne a.
  friend constexpr ARCCORE_HOST_DEVICE ThatClass& operator-=(ThatClass& a, const ThatClass& b)
  {
    for (int i = 0; i < NbElement; ++i)
      a.m_values[i] -= b.m_values[i];
    return a;
  }
  //! Multiplie chaque composante de la matrice a par le nombre réel b et retourne a.
  friend constexpr ARCCORE_HOST_DEVICE ThatClass& operator*=(ThatClass& a, T b)
  {
    for (int i = 0; i < NbElement; ++i)
      a.m_values[i] *= b;
    return a;
  }
  //! Divise chaque composante de la matrice par le nombre réel b
  friend constexpr ARCCORE_HOST_DEVICE ThatClass& operator/=(ThatClass& a, T b)
  {
    for (int i = 0; i < NbElement; ++i)
      a.m_values[i] /= b;
    return a;
  }
  //! Crée un triplet qui est égal à ce triplet ajouté à b
  friend constexpr ARCCORE_HOST_DEVICE ThatClass operator+(const ThatClass& a, const ThatClass& b)
  {
    ThatClass v;
    for (int i = 0; i < NbElement; ++i)
      v.m_values[i] = a.m_values[i] + b.m_values[i];
    return v;
  }
  //! Crée un triplet qui est égal à a soustrait de ce triplet
  friend constexpr ARCCORE_HOST_DEVICE ThatClass operator-(const ThatClass& a, const ThatClass& b)
  {
    ThatClass v;
    for (int i = 0; i < NbElement; ++i)
      v.m_values[i] = a.m_values[i] - b.m_values[i];
    return v;
  }
  //! Crée un tenseur opposé au tenseur actuel
  constexpr ARCCORE_HOST_DEVICE ThatClass operator-() const
  {
    ThatClass v;
    for (int i = 0; i < NbElement; ++i)
      v.m_values[i] = -m_values[i];
    return v;
  }

  //! Multiplication par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE ThatClass operator*(DataType a, const ThatClass& mat)
  {
    ThatClass v;
    for (int i = 0; i < NbElement; ++i)
      v.m_values[i] = a * mat.m_values[i];
    return v;
  }
  //! Multiplication par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE ThatClass operator*(const ThatClass& mat, DataType b)
  {
    ThatClass v;
    for (int i = 0; i < NbElement; ++i)
      v.m_values[i] = mat.m_values[i] * b;
    return v;
  }
  //! Division par un scalaire.
  friend constexpr ARCCORE_HOST_DEVICE ThatClass operator/(const ThatClass& mat, DataType b)
  {
    ThatClass v;
    for (int i = 0; i < NbElement; ++i)
      v.m_values[i] = mat.m_values[i] / b;
    return v;
  }

  /*!
   * \brief Compare l'instance actuelle composante par composante à b.
   *
   * \retval true si this.x==b.x et this.y==b.y et this.z==b.z.
   * \retval false sinon.
   */
  friend constexpr ARCCORE_HOST_DEVICE bool operator==(const ThatClass& a, const ThatClass& b)
  {
    for (int i = 0; i < NbElement; ++i)
      if (a.m_values[i] != b.m_values[i])
        return false;
    return true;
  }

  /*!
   * \brief Compare deux triplets.
   *
   * Pour la notion d'égalité, voir operator==()
   * \retval true si les deux triplets sont différents,
   * \retval false sinon.
   */
  friend constexpr ARCCORE_HOST_DEVICE bool operator!=(const ThatClass& a, const ThatClass& b)
  {
    return !(a == b);
  }

 public:

  constexpr ARCCORE_HOST_DEVICE VectorType row(Int32 i) const
  {
    ARCCORE_CHECK_AT(i, RowSize);
    return VectorType(m_values + i * ColumnSize, typename VectorType::InitTag{});
  }

  // Récupère une référence à la valeur de la i-ème ligne et de la j-ème colonne
  constexpr ARCCORE_HOST_DEVICE DataType& operator()(Int32 i, Int32 j)
  {
    ARCCORE_CHECK_AT(i, RowSize);
    ARCCORE_CHECK_AT(j, ColumnSize);
    return m_values[i * ColumnSize + j];
  }

  // Récupère la valeur de la i-ème ligne et de la j-ème colonne
  constexpr ARCCORE_HOST_DEVICE DataType operator()(Int32 i, Int32 j) const
  {
    ARCCORE_CHECK_AT(i, RowSize);
    ARCCORE_CHECK_AT(j, ColumnSize);
    return m_values[i * ColumnSize + j];
  }

  //! Définit la valeur de la i-ème ligne à v
  constexpr ARCCORE_HOST_DEVICE void setRow(Int32 i, const VectorType& v)
  {
    ARCCORE_CHECK_AT(i, RowSize);
    DataType* base = m_values + i * ColumnSize;
    for (int j = 0; j < ColumnSize; ++j)
      base[j] = v[j];
  }

  //! Remplit la matrice avec la valeur \a v
  constexpr ARCCORE_HOST_DEVICE void fill(const DataType& v)
  {
    for (int i = 0; i < NbElement; ++i) {
      m_values[i] = v;
    }
  }

  //! Retourne une vue mutable des éléments de la matrice.
  constexpr ARCCORE_HOST_DEVICE ArrayView<DataType> view()
  {
    return { NbElement, m_values };
  }

  //! Retourne une vue en lecture seule des éléments de la matrice.
  constexpr ARCCORE_HOST_DEVICE ConstArrayView<DataType> constView() const
  {
    return { NbElement, m_values };
  }

  //! Retourne une vue mutable des éléments de la matrice.
  constexpr ARCCORE_HOST_DEVICE SmallSpan<DataType, NbElement> span()
  {
    return { m_values, NbElement };
  }

  //! Retourne une vue en lecture seule des éléments de la matrice.
  constexpr ARCCORE_HOST_DEVICE SmallSpan<const DataType, NbElement> constSpan() const
  {
    return { m_values, NbElement };
  }

  friend std::ostream& operator<<(std::ostream& o, const NumMatrix& t)
  {
    t.print(o);
    return o;
  }

 public:

  constexpr const VectorType vx() const requires(RowSize >= 1)
  {
    return row(0);
  }

  constexpr const VectorType vy() const requires(RowSize >= 2)
  {
    return row(1);
  }

  constexpr const VectorType vz() const requires(RowSize >= 3)
  {
    return row(2);
  }

 private:

  //! Valeurs de la matrice
  DataType m_values[NbElement];

 private:

  /*!
   * \brief Compare les valeurs de a et b en utilisant le comparateur TypeEqualT
   * \retval true si a et b sont égaux,
   * \retval false sinon.
   */
  constexpr ARCCORE_HOST_DEVICE static bool _eq(T a, T b)
  {
    return TypeEqualT<T>::isEqual(a, b);
  }
  template <typename Stream>
  void print(Stream& o) const
  {
    for (int i = 0; i < NbElement; ++i) {
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
