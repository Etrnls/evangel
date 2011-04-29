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
#include "testbcodec.h"

#include "protocol/bt/util/bcodec.h" // core

#include <QTest>

namespace UnitTest
{

namespace Protocol
{

namespace BitTorrent
{

using ::Protocol::BitTorrent::Bcodec;

void TestBcodec::encodeInteger_data()
{
	QTest::addColumn<Bcodec::BInteger>("integer");
	QTest::addColumn<QByteArray>("result");

	QTest::newRow("positive integer") << Bcodec::BInteger(523)  << QByteArray("i523e");
	QTest::newRow("negative integer") << Bcodec::BInteger(-109) << QByteArray("i-109e");
	QTest::newRow("zero")     << Bcodec::BInteger(0)    << QByteArray("i0e");
}

void TestBcodec::encodeInteger()
{
	QFETCH(Bcodec::BInteger, integer);
	QFETCH(QByteArray, result);

	QCOMPARE(Bcodec::encode(integer), result);
}

void TestBcodec::encodeString_data()
{
	QTest::addColumn<Bcodec::BString>("string");
	QTest::addColumn<QByteArray>("result");

	QTest::newRow("hello") << Bcodec::BString("hello") << QByteArray("5:hello");
	QTest::newRow("hello world") << Bcodec::BString("hello, world!") << QByteArray("13:hello, world!");
}

void TestBcodec::encodeString()
{
	QFETCH(Bcodec::BString, string);
	QFETCH(QByteArray, result);

	QCOMPARE(Bcodec::encode(string), result);
}

void TestBcodec::encodeList_data()
{
	QTest::addColumn<Bcodec::BList>("list");
	QTest::addColumn<QByteArray>("result");

	Bcodec::BList stringList;
	QByteArray stringListResult("l4:spam4:eggse");
	stringList.append(Bcodec::BString("spam"));
	stringList.append(Bcodec::BString("eggs"));
	QTest::newRow("string list") << stringList << stringListResult;

	Bcodec::BList integerList;
	QByteArray integerListResult("li5ei23ee");
	integerList.append(Bcodec::BInteger(5));
	integerList.append(Bcodec::BInteger(23));
	QTest::newRow("integer list") << integerList << integerListResult;

	Bcodec::BList list;
	QByteArray listResult;

	list.append(Bcodec::BString("string"));		listResult.append("6:string");
	list.append(Bcodec::BInteger(109));		listResult.append("i109e");
	list.append(QVariant::fromValue(stringList));	listResult.append(stringListResult);
	list.append(QVariant::fromValue(integerList));	listResult.append(integerListResult);

	listResult.prepend('l').append('e');

	QTest::newRow("variant list") << list << listResult;

	QTest::newRow("empty list") << Bcodec::BList() << QByteArray("le");
}

void TestBcodec::encodeList()
{
	QFETCH(Bcodec::BList, list);
	QFETCH(QByteArray, result);

	QCOMPARE(Bcodec::encode(list), result);
}

void TestBcodec::encodeDictionary_data()
{
	QTest::addColumn<Bcodec::BDictionary>("dictionary");
	QTest::addColumn<QByteArray>("result");

	Bcodec::BDictionary stringDictionary;
	QByteArray stringDictionaryResult("d3:cow3:moo4:spam4:eggse");
	stringDictionary["cow"] = Bcodec::BString("moo");
	stringDictionary["spam"] = Bcodec::BString("eggs");
	QTest::newRow("string dictionary") << stringDictionary << stringDictionaryResult;

	Bcodec::BDictionary integerDictionary;
	QByteArray integerDictionaryReulst("d3:abci523e3:bcdi109ee");
	integerDictionary["abc"] = Bcodec::BInteger(523);
	integerDictionary["bcd"] = Bcodec::BInteger(109);
	QTest::newRow("integer dictionary") << integerDictionary << integerDictionaryReulst;

	Bcodec::BList list;
	Bcodec::BDictionary listDictionary;
	QByteArray listResult("l1:a1:be");
	QByteArray listDictionaryResult;
	listDictionaryResult = "d4:spam";
	list.append(Bcodec::BString("a"));
	list.append(Bcodec::BString("b"));
	listDictionary["spam"] = list;
	listDictionaryResult.append(listResult);
	listDictionaryResult.append('e');
	QTest::newRow("list dictionary") << listDictionary << listDictionaryResult;

	Bcodec::BDictionary dictionary;
	QByteArray dictionaryResult;
	dictionaryResult = "d";
	dictionary["a"] = Bcodec::BInteger(523);	dictionaryResult.append("1:ai523e");
	dictionary["bb"] = Bcodec::BString("flower");	dictionaryResult.append("2:bb6:flower");
	dictionary["ccc"] = list;			dictionaryResult.append(listResult.prepend("3:ccc"));
	dictionary["dddd"] = QVariant::fromValue(listDictionary);
	dictionaryResult.append(listDictionaryResult.prepend("4:dddd"));
	dictionaryResult.append('e');
	QTest::newRow("variant dictionary") << dictionary << dictionaryResult;

	QTest::newRow("empty dictionary") << Bcodec::BDictionary() << QByteArray("de");
}

void TestBcodec::encodeDictionary()
{
	QFETCH(Bcodec::BDictionary, dictionary);
	QFETCH(QByteArray, result);

	QCOMPARE(Bcodec::encode(dictionary), result);
}

}

}

}

QTEST_MAIN(UnitTest::Protocol::BitTorrent::TestBcodec)

