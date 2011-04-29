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
#include "testhash.h"

#include "util/hash/hash.h" // core

#include <QTest>

namespace UnitTest
{

namespace Utility
{

namespace Cryptography
{

namespace Hash
{

void TestHash::CRC32_data()
{
	QTest::addColumn<QByteArray>("data");
	QTest::addColumn<QByteArray>("result");

	QTest::newRow("abc") << QByteArray("abc") << QByteArray::fromHex(QByteArray::number(891568578, 16));
	QTest::newRow("pangram") << QByteArray("The quick brown fox jumps over the lazy dog")
				<< QByteArray::fromHex(QByteArray::number(1095738169, 16));
	QTest::newRow("01234567") << QByteArray("01234567") << QByteArray::fromHex(QByteArray::number(763378421, 16));
}

void TestHash::CRC32()
{
	QFETCH(QByteArray, data);
	QFETCH(QByteArray, result);

	QCOMPARE(::Utility::Cryptography::Hash::hash(data, ::Utility::Cryptography::Hash::CRC32), result);
}

void TestHash::CRC32Benchmark()
{
	QByteArray data(1024, Qt::Uninitialized);

	QBENCHMARK
	{
		QByteArray result = ::Utility::Cryptography::Hash::hash(data, ::Utility::Cryptography::Hash::CRC32);
		Q_UNUSED(result);
	}

}

}

}

}

}

QTEST_MAIN(UnitTest::Utility::Cryptography::Hash::TestHash)
