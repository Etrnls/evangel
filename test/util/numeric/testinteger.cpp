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
#include "testinteger.h"

#include "util/numeric/integer.h" // core
#include "util/numeric/random.h" // core

#include <QTest>

namespace UnitTest
{

namespace Utility
{

using ::Utility::Integer;

void TestInteger::constructor()
{
	static const int len = 100;
	static const int times = 100;

	for (int i = 0; i < times; ++i)
	{
		QString s;
		for (int j = 0; j < len; ++j)
		{
			int x = ::Utility::rand(0x10);
			if (x <= 9)
				s += '0' + x;
			else
				s += 'A' + (x - 10);
		}
		Integer x(s);
		QCOMPARE(static_cast<QByteArray>(x), QByteArray::fromHex(s.toLatin1()));
		QCOMPARE(Integer(static_cast<QByteArray>(x)), x);
	}
}

void TestInteger::shift()
{
	static const char* result[] = {"01", "02", "04", "08", "10", "20", "40", "80"};
	static const int times = 100;

	Integer x(1);
	QByteArray zeros;
	for (int i = 0, k = 0; i < times; ++i)
	{
		for (int j = 0; j < static_cast<int>(sizeof(result) / sizeof(*result)); ++j, ++k)
		{
			QCOMPARE(static_cast<QByteArray>(x), QByteArray::fromHex(QByteArray(result[j]).append(zeros)));
			QCOMPARE(x, Integer(1) << k);
			x <<= 1;
		}
		zeros.append("00");
	}
	Integer y(x >> 1);
	for (int i = 0, k = 0; i < times; ++i)
	{
		zeros.remove(zeros.size() - 2, 2);
		for (int j = static_cast<int>(sizeof(result) / sizeof(*result)) - 1; j >= 0; --j, ++k)
		{
			x >>= 1;
			QCOMPARE(static_cast<QByteArray>(x), QByteArray::fromHex(QByteArray(result[j]).append(zeros)));
			QCOMPARE(x, y >> k);
		}
	}
}

void TestInteger::multiply()
{
	QCOMPARE(Integer(QLatin1String("82749182751028301571203012512438102597105102983410259812852390671092481"
			 "02957918241982751928659127192837198526192873198719247192")) *
		 Integer(QLatin1String("1982471928719287491879318798175981635123791872391625916249171720341721"
			 "982749182659127439127391865912874192371924192569317256195")),
		 Integer(QLatin1String("0cffc5a2c80781320cd13d61e2fc872f808b7b98a0494c13b32c6a49753b5bc0c2115ac"
				       "0c64022572d61aeac1664511dbf4378727a209234e15fac2e2fcccec8e7ae260f89d71c"
				       "43d281651f081bd944f837a17bbfc0ca10b5d4f03f0e1177d801d88ca1e76299e4f082a"
				       "508050b151fcd04f1092dbece80cb3776f9586bfa")));

	static const int digits = 1024;
	Integer a, b;
	a.random(digits);
	b.random(digits);
	QBENCHMARK
	{
		a * b;
	}
}

void TestInteger::DHExchange()
{
	Integer selfPrivate(QLatin1String("ae155c2b452e8a5d130a2614a5ed4bd93c4e789c004c0098")),
		peerPrivate(QLatin1String("edc4db8a2dba5b74ef29de5251a4a349fc3bf876415a82b5"));
	Integer p(QLatin1String("FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B"
				"80DC1CD129024E088A67CC74020BBEA63B139B22"
				"514A08798E3404DDEF9519B3CD3A431B302B0A6D"
				"F25F14374FE1356D6D51C245E485B576625E7EC6"
				"F44C42E9A63A36210000000000090563"));
	Integer ans(QLatin1String("5a79c7ae27db444d0daa8cc86c654196f7391a4b7ae02cfd2fa"
					"2b4af84f7cf825c78fdf23589b0f8f9459c1b786c2dc3e41779"
					"fdabe2639dbddb3028db3446cb9a2d1a8b89025dc5200175509"
					"e86e6d73f284823d4de447147d54c102a9da42b"));

	Integer selfPublic = Integer::powerMod(Integer(2), selfPrivate, p);
	Integer peerPublic = Integer::powerMod(Integer(2), peerPrivate, p);
	Integer s1 = Integer::powerMod(selfPublic, peerPrivate, p);
	Integer s2 = Integer::powerMod(peerPublic, selfPrivate, p);

	QCOMPARE(selfPublic, Integer(QLatin1String("f66d00ae1c2baa656f36f5a3f35ce2eb36ca8247872134"
						   "63c148f609af3cab7d4b82df334203a3018b26bb95bad4"
						   "67953e9d8473fa8debd908d0f7ce6e6a59d8f591014b07"
						   "be019bf5ae0c4e8683f350d2a1a51c9524b8faae905e45630e3260")));
	QCOMPARE(peerPublic, Integer(QLatin1String("b5053c46a74a3d3017d24fa4babbdebee8bdf64d2ebb3c5"
						   "e8ca85e31b077d7f327eda2bd75101ec92adbdd89dc92e0"
						   "6878270bdc934b986fabbed8dcc62b1739a967dcb54ee58"
						   "fc671033de222a8693b135e4de3b31fc0e1b030492a184fb66b")));

	QCOMPARE(s1, s2);
	QCOMPARE(s1, ans);
	QCOMPARE(s2, ans);
}

void TestInteger::modExpBenchmark()
{
	Integer a, b;
	Integer p(QLatin1String("FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B"
				"80DC1CD129024E088A67CC74020BBEA63B139B22"
				"514A08798E3404DDEF9519B3CD3A431B302B0A6D"
				"F25F14374FE1356D6D51C245E485B576625E7EC6"
				"F44C42E9A63A36210000000000090563"));
	a.random(160);
	b.random(160);

	QBENCHMARK
	{
		Integer x = Integer::powerMod(Integer::powerMod(Integer(2), a, p), b, p);
		Integer y = Integer::powerMod(Integer::powerMod(Integer(2), b, p), a, p);
	}
}

}

}

QTEST_MAIN(UnitTest::Utility::TestInteger)

