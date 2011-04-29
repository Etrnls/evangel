/***************************************************************************
 *   Copyright (C) 2010 by Etrnls                                          *
 *   Etrnls@gmail.com                                                      *
 *                                                                         *
 *   This file is part of Evangel.                                         *
 *                                                                         *
 *   Evangel is free software: you can redistribute it and/or modify       *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Evangel is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with Evangel.  If not, see <http://www.gnu.org/licenses/>.      *
 ***************************************************************************/
#ifndef UTILITY_INTEGER_H
#define UTILITY_INTEGER_H

#include <QVector>

namespace Utility
{

class Integer
{
public:
	static Integer powerMod(Integer b, Integer e, const Integer &p);

	explicit Integer(quint32 x = 0);
	explicit Integer(const QByteArray &x);
	explicit Integer(const QString &x);

	void random(uint bits);

	operator QByteArray() const;

	Integer& operator<<=(uint b);
	Integer& operator>>=(uint b);

	Integer& operator+=(quint32 other);
	Integer& operator*=(quint32 other);
	Integer& operator%=(quint32 other);

	Integer& operator+=(const Integer &other);
	Integer& operator-=(const Integer &other);
	Integer& operator*=(const Integer &other);
	Integer& operator%=(const Integer &other);
private:
	QVector<quint32> value;

	static int cmp(const Integer &a, const Integer &b);
	static int bits(quint32 x);

	void carry(int i, quint32 x);
	void negcarry(int i, quint32 x);
	void simplify();


	friend quint32 operator&(const Integer &lhs, quint32 rhs);
	friend quint32 operator&(quint32 lhs, const Integer &rhs);

	friend bool operator<(const Integer &lhs, const Integer &rhs);
	friend bool operator>(const Integer &lhs, const Integer &rhs);

	friend bool operator<=(const Integer &lhs, const Integer &rhs);
	friend bool operator>=(const Integer &lhs, const Integer &rhs);

	friend bool operator==(const Integer &lhs, const Integer &rhs);
	friend bool operator!=(const Integer &lhs, const Integer &rhs);

	friend bool operator==(const Integer &lhs, quint32 rhs);
	friend bool operator!=(const Integer &lhs, quint32 rhs);
	friend bool operator==(quint32 lhs, const Integer &rhs);
	friend bool operator!=(quint32 lhs, const Integer &rhs);
};

inline quint32 operator&(const Integer &lhs, quint32 rhs)
{
	return lhs.value[0] & rhs;
}

inline quint32 operator&(quint32 lhs, const Integer &rhs)
{
	return lhs & rhs.value[0];
}

inline bool operator<(const Integer &lhs, const Integer &rhs)
{
	return Integer::cmp(lhs, rhs) == -1;
}

inline bool operator>(const Integer &lhs, const Integer &rhs)
{
	return Integer::cmp(lhs, rhs) == 1;
}

inline bool operator<=(const Integer &lhs, const Integer &rhs)
{
	return !(lhs > rhs);
}

inline bool operator>=(const Integer &lhs, const Integer &rhs)
{
	return !(lhs < rhs);
}

inline bool operator==(const Integer &lhs, const Integer &rhs)
{
	return Integer::cmp(lhs, rhs) == 0;
}

inline bool operator!=(const Integer &lhs, const Integer &rhs)
{
	return !(lhs == rhs);
}

inline bool operator==(const Integer &lhs, quint32 rhs)
{
	return lhs.value.size() == 1 && lhs.value[0] == rhs;
}

inline bool operator!=(const Integer &lhs, quint32 rhs)
{
	return !(lhs == rhs);
}

inline bool operator==(quint32 lhs, const Integer &rhs)
{
	return rhs.value.size() == 1 && lhs == rhs.value[0];
}

inline bool operator!=(quint32 lhs, const Integer &rhs)
{
	return !(lhs == rhs);
}

inline Integer operator<<(const Integer &lhs, uint rhs)
{
	return Integer(lhs) <<= rhs;
}

inline Integer operator>>(const Integer &lhs, uint rhs)
{
	return Integer(lhs) >>= rhs;
}

inline Integer operator*(const Integer &lhs, quint32 rhs)
{
	return Integer(lhs) *= rhs;
}

inline Integer operator*(quint32 lhs, const Integer &rhs)
{
	return Integer(rhs) *= lhs;
}

inline Integer operator*(const Integer &lhs, const Integer &rhs)
{
	return Integer(lhs) *= rhs;
}

}

#endif
