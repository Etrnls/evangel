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
#include "hash.h"
#include "crc32.h"

#include <QCryptographicHash>

namespace Utility
{

namespace Cryptography
{

namespace Hash
{

/*!
 * \brief Caculates the hash of data using algorithm
 * \param data The data to be caculated.
 * \param algorithm The algorithm used to caculate.
 */
QByteArray hash(const QByteArray &data, Algorithm algorithm)
{
	switch (algorithm)
	{
	case MD4:
		return QCryptographicHash::hash(data, QCryptographicHash::Md4);
	case MD5:
		return QCryptographicHash::hash(data, QCryptographicHash::Md5);
	case SHA1:
		return QCryptographicHash::hash(data, QCryptographicHash::Sha1);
	case CRC16:
		return QByteArray::fromHex(QByteArray::number(qChecksum(data.constData(), data.size()), 16));
	case CRC32:
		return QByteArray::fromHex(QByteArray::number(Private::generateCRC32(data.constData(), data.size()), 16));
	default:
		Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid algorithm");
		break;
	};
	return QByteArray(); // never got here
}

}

}

}
