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
#ifndef UTILITY_UINT160_H
#define UTILITY_UINT160_H

#include "random.h"

#include "util/endian.h"

#include <QtGlobal>
#include <QByteArray>

namespace Utility
{

class UInt160
{
public:
	inline UInt160()
	{
		data[0] = data[1] = data[2] = data[3] = data[4] = 0;
	}

	inline UInt160(const QByteArray &data)
	{
		this->data[0] = Endian::fromBigEndian<quint32>(&data.constData()[0]);
		this->data[1] = Endian::fromBigEndian<quint32>(&data.constData()[4]);
		this->data[2] = Endian::fromBigEndian<quint32>(&data.constData()[8]);
		this->data[3] = Endian::fromBigEndian<quint32>(&data.constData()[12]);
		this->data[4] = Endian::fromBigEndian<quint32>(&data.constData()[16]);
	}

	inline UInt160(const UInt160 &uint160)
	{
		setData(uint160.data);
	}

	inline UInt160& operator=(const UInt160 &other)
	{
		setData(other.data);
		return *this;
	}

	inline void random()
	{
		data[0] = rand(Q_UINT64_C(1) << 32);
		data[1] = rand(Q_UINT64_C(1) << 32);
		data[2] = rand(Q_UINT64_C(1) << 32);
		data[3] = rand(Q_UINT64_C(1) << 32);
		data[4] = rand(Q_UINT64_C(1) << 32);
	}

	inline UInt160& operator^=(const UInt160 &other)
	{
		data[0] ^= other.data[0];
		data[1] ^= other.data[1];
		data[2] ^= other.data[2];
		data[3] ^= other.data[3];
		data[4] ^= other.data[4];
		return *this;
	}
private:
	inline void setData(const quint32 data[])
	{
		this->data[0] = data[0];
		this->data[1] = data[1];
		this->data[2] = data[2];
		this->data[3] = data[3];
		this->data[4] = data[4];
	}

	quint32 data[5];

	friend bool operator==(const UInt160 &lhs, const UInt160 &rhs);
	friend bool operator<(const UInt160 &lhs, const UInt160 &rhs);
	friend bool operator>(const UInt160 &lhs, const UInt160 &rhs);
};

inline UInt160 operator^(const UInt160 &lhs, const UInt160 &rhs)
{
	return UInt160(lhs) ^= rhs;
}

inline bool operator==(const UInt160 &lhs, const UInt160 &rhs)
{
	return lhs.data[0] == rhs.data[0] &&
		lhs.data[1] == rhs.data[1] &&
		lhs.data[2] == rhs.data[2] &&
		lhs.data[3] == rhs.data[3] &&
		lhs.data[4] == rhs.data[4];
}

inline bool operator!=(const UInt160 &lhs, const UInt160 &rhs)
{
	return !(lhs == rhs);
}

inline bool operator<(const UInt160 &lhs, const UInt160 &rhs)
{
	if (lhs.data[0] != rhs.data[0])
		return lhs.data[0] < rhs.data[0];
	else if (lhs.data[1] != rhs.data[1])
		return lhs.data[1] < rhs.data[1];
	else if (lhs.data[2] != rhs.data[2])
		return lhs.data[2] < rhs.data[2];
	else if (lhs.data[3] != rhs.data[3])
		return lhs.data[3] < rhs.data[3];
	else
		return lhs.data[4] < rhs.data[4];
}

inline bool operator>(const UInt160 &lhs, const UInt160 &rhs)
{
	if (lhs.data[0] != rhs.data[0])
		return lhs.data[0] > rhs.data[0];
	else if (lhs.data[1] != rhs.data[1])
		return lhs.data[1] > rhs.data[1];
	else if (lhs.data[2] != rhs.data[2])
		return lhs.data[2] > rhs.data[2];
	else if (lhs.data[3] != rhs.data[3])
		return lhs.data[3] > rhs.data[3];
	else
		return lhs.data[4] > rhs.data[4];
}

inline bool operator<=(const UInt160 &lhs, const UInt160 &rhs)
{
	return !(lhs > rhs);
}

inline bool operator>=(const UInt160 &lhs, const UInt160 &rhs)
{
	return !(lhs < rhs);
}

}

#endif
