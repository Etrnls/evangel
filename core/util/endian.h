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
#ifndef UTILITY_ENDIAN
#define UTILITY_ENDIAN

#include <QtEndian>

namespace Utility
{

namespace Endian
{

template <class T>
inline T fromBigEndian(const char *src)
{
	return qFromBigEndian<T>(reinterpret_cast<const uchar*>(src));
}

template <class T>
inline T fromBigEndian(const uchar *src)
{
	return qFromBigEndian<T>(src);
}

template <class T>
inline void toBigEndian(T src, char *dest)
{
	qToBigEndian(src, reinterpret_cast<uchar*>(dest));
}

template <class T>
inline void toBigEndian(T src, uchar *dest)
{
	qToBigEndian(src, dest);
}

}

}

#endif
