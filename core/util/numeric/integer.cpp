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
#include "integer.h"

#include "util/endian.h"
#include "util/numeric/random.h"

#include <QString>
#include <climits>

namespace Utility
{

Integer Integer::powerMod(Integer b, Integer e, const Integer &p)
{
	Integer result(1);	b %= p;

	while (e != 0)
	{
		if (e & 1)
		{
			result *= b;
			result %= p;
		}
		b *= b;	b %= p;
		e >>= 1;
	}
	return result;
}

Integer::Integer(quint32 x) : value(1, x)
{
}

/*!
 * \brief Constructs a Integer from big-endian represent data
 */
Integer::Integer(const QByteArray &x) : value((x.size() + 3) / 4)
{
	QByteArray data = x;
	if (data.size() & 3)	data.prepend(QByteArray(4 - (data.size() & 3), 0));
	for (int i = 0, j = data.size() - 1; i < value.size(); ++i, j -= 4)
		value[i] = Utility::Endian::fromBigEndian<quint32>(&data.constData()[j - 3]);
	simplify();
}

/*!
 * \brief Constructs a Integer from hex string
 */
Integer::Integer(const QString &x) : value(1, 0)
{
	for (int i = 0; i < x.size(); ++i)
	{
		*this <<= 4;
		const char c = x[i].toLatin1();
		if (c >= '0' && c <= '9')
			*this += c - '0';
		else if (c >= 'A' && c <= 'F')
			*this += c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			*this += c - 'a' + 10;
	}
}

void Integer::random(uint bits)
{
	value.resize(bits / 32);
	for (int i = 0; i < value.size(); ++i)
		value[i] = Utility::rand((Q_UINT64_C(1) << 32) - 1);

	bits &= 7;
	value.append(Utility::rand(1 << (bits - 1), 1 << bits));
}

/*!
 * \brief Convert to big-endian represent data
 */
Integer::operator QByteArray() const
{
	QByteArray result(value.size() * 4, 0);

	for (int i = 0, j = result.size() - 1; i < value.size(); ++i, j -= 4)
		Utility::Endian::toBigEndian<quint32>(value[i], &result.data()[j - 3]);
	while (result.size() > 1 && static_cast<quint8>(result[0]) == 0)
		result.remove(0, 1);

	return result;
}

Integer& Integer::operator<<=(uint b)
{
	const int n = b / 32;
	b &= 31;

	if (b)
	{
		value.append(0);
		for (int i = value.size() - 1; i >= 1; --i)
		{
			value[i] <<= b;
			value[i] |= (value[i - 1] >> (32 - b)) & ((1 << b) - 1);
		}
		value[0] <<= b;
	}
	if (n)	value.insert(0, n, 0);

	simplify();
	return *this;
}

Integer& Integer::operator>>=(uint b)
{
	const int n = b / 32;
	if (n >= value.size())	return *this = Integer(0);

	b &= 31;
	if (n)	value.remove(0, n);
	if (b)
	{
		for (int i = 0; i < value.size() - 1; ++i)
			value[i] = ((value[i] >> b) & ((1 << (32 - b)) - 1)) |
				((value[i + 1] & ((1 << b) - 1)) << (32 - b));
		value.last() >>= b;
	}

	simplify();
	return *this;
}

Integer& Integer::operator+=(quint32 other)
{
	carry(0, other);

	return *this;
}

Integer& Integer::operator*=(quint32 other)
{
	if (other == 0)	return *this = Integer(0);

	quint64 t = 0;
	for (int i = 0; i < value.size(); ++i)
	{
		t += static_cast<quint64>(value[i]) * other;
		value[i] = t & ((Q_UINT64_C(1) << 32) - 1);
		t >>= 32;
	}
	if (t)	value.append(t);

	return *this;
}

Integer& Integer::operator%=(quint32 other)
{
	Q_ASSERT_X(false, Q_FUNC_INFO, "Not implemented");
	Q_UNUSED(other);
	return *this;
}

Integer& Integer::operator+=(const Integer &other)
{
	if (value.size() < other.value.size() + 1)
		value.resize(other.value.size() + 1);

	quint64 t = 0;
	for (int i = 0; i < other.value.size(); ++i)
	{
		t += static_cast<quint64>(value[i]) + other.value[i];
		value[i] = t & ((Q_UINT64_C(1) << 32) - 1);
		t >>= 32;
	}
	if (t)	carry(other.value.size(), t);

	simplify();
	return *this;
}

Integer& Integer::operator-=(const Integer &other)
{
	Q_ASSERT_X(*this >= other, Q_FUNC_INFO, "Negative not supported");

	for (int i = 0; i < other.value.size(); ++i)
	{
		if (value[i] < other.value[i])
		{
			negcarry(i + 1, 1);
			value[i] = (Q_UINT64_C(1) << 32) + value[i] - other.value[i];
		}
		else
			value[i] -= other.value[i];
	}

	simplify();
	return *this;
}

Integer& Integer::operator*=(const Integer &other)
{
	if (other.value.size() == 1)	return *this *= other.value[0];

	Integer result;
	for (int i = 0; i < value.size(); ++i)
	{
		Integer t = other * value[i];
		t <<= i * 32;
		result += t;
	}

	return *this = result;
}

Integer& Integer::operator%=(const Integer &other)
{
	if (other.value.size() == 1)	return *this %= other.value[0];
	switch (cmp(*this, other))
	{
	case -1:	return *this;
	case 0:		return *this = Integer(0);
	default:	break;
	}

	const int shifts = qMax(31 - bits(other.value.last()), 0);
	Integer dividend = *this << shifts;
	Integer divisor = other << shifts;

	Integer result(0);
	result.value.reserve(divisor.value.capacity());

	for (int i = dividend.value.size() - 1; i >= 0; --i)
	{
		if (result == 0)
			result.value[0] = dividend.value[i];
		else
			result.value.prepend(dividend.value[i]);

		switch (cmp(result, divisor))
		{
		case 0:		result = Integer(0);
		case -1:	continue;
		default:	break;
		}

		quint64 a = result.value[divisor.value.size() - 1];
		if (result.value.size() > divisor.value.size())
			a |= static_cast<quint64>(result.value[result.value.size() - 1]) << 32;
		a /= divisor.value[divisor.value.size() - 1];
		quint32 q = qMin(a, (Q_UINT64_C(1) << 32) - 1);

		Integer x = divisor * q;
		while (x > result)	x = divisor * --q;
		result -= x;

		Q_ASSERT_X(result < divisor, Q_FUNC_INFO, "Invalid result");
	}

	return *this = result >> shifts;
}

int Integer::cmp(const Integer &a, const Integer &b)
{
	if (a.value.size() < b.value.size())	return -1;
	if (a.value.size() > b.value.size())	return 1;
	for (int i = a.value.size() - 1; i >= 0; --i)
	{
		if (a.value[i] < b.value[i])	return -1;
		if (a.value[i] > b.value[i])	return 1;
	}
	return 0;
}

int Integer::bits(quint32 x)
{
	int b = 0;
	while (x)
	{
		x >>= 1;
		++b;
	}
	return b;
}

void Integer::carry(int i, quint32 x)
{
	quint64 t = x;
	for (; t && i < value.size(); ++i)
	{
		t += value[i];
		value[i] = t & ((Q_UINT64_C(1) << 32) - 1);
		t >>= 32;
	}
	if (t)	value.append(t);
}

void Integer::negcarry(int i, quint32 x)
{
	for (; i < value.size(); ++i)
		if (value[i] >= x)
		{
			value[i] -= x;
			break;
		}
		else
		{
			value[i] = (Q_UINT64_C(1) << 32) + value[i] - x;
			x = 1;
		}
	simplify();
}

void Integer::simplify()
{
	while (value.size() > 1 && value.last() == 0)
		value.erase(value.end() - 1);
}

}

