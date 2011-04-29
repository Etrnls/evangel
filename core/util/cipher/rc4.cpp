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
#include "rc4.h"

#include <QByteArray>

namespace Utility
{

namespace Cryptography
{

namespace Cipher
{

RC4::RC4(const QByteArray &key) : i(0), j(0)
{
	for (uint i = 0; i < boxSize; ++i)
		s[i] = i;

	for (uint i = 0, j = 0; i < boxSize; ++i)
	{
		j = (j + static_cast<quint8>(key[i % key.size()]) + s[i]) & 0xff;
		qSwap(s[i], s[j]);
	}
}

void RC4::process(char *data, int size)
{
	while (size--)
	{
		i = (i + 1) & 0xff;
		j = (j + s[i]) & 0xff;
		quint16 t = s[i];
		s[i] = s[j];
		s[j] = t;
		*data++ ^= s[(s[i] + t) & 0xff];
	}
}

}

}

}
