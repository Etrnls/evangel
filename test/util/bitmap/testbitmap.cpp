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
#include "testbitmap.h"

#include "util/bitmap/bitmap.h" // core

#include <QTest>
#include <QVector>
#include <algorithm>

Q_DECLARE_METATYPE(::Utility::Bitmap)
Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(QVector<QList< ::Utility::Bitmap::Segment>>)
Q_DECLARE_METATYPE(QList< ::Utility::Bitmap::Segment>)

namespace UnitTest
{

namespace Utility
{

using ::Utility::Bitmap;

void TestBitmap::sequence_data()
{
	QTest::addColumn<Bitmap>("bitmap");
	QTest::addColumn<int>("n");
	QTest::addColumn<int>("block");
	QTest::addColumn<QVector<int>>("begin");
	QTest::addColumn<QVector<int>>("end");
	QTest::addColumn<QVector<int>>("step");
	QTest::addColumn<QVector<QList<Bitmap::Segment>>>("expected");

	static const int maxNumber = 100000;
	static const int blockSize = 10;

	QList<Bitmap::Segment> full, bitEven, bitOdd, bitsEven, bitsOdd;
	full.append(Bitmap::Segment(0, maxNumber));
	for (int i = 0; i < maxNumber; i += 2)
		bitEven.append(Bitmap::Segment(i, i + 1));
	for (int i = 1; i < maxNumber; i += 2)
		bitOdd.append(Bitmap::Segment(i, i + 1));
	for (int i = 0; i < maxNumber; i += blockSize * 2)
		bitsEven.append(Bitmap::Segment(i, i + blockSize));
	for (int i = blockSize; i < maxNumber; i += blockSize * 2)
		bitsOdd.append(Bitmap::Segment(i, i + blockSize));
	
	static const int maxN = 3;
	int block;
	QVector<int> begin(maxN), end(maxN), step(maxN);
	QVector<QList<Bitmap::Segment>> expected(maxN);

	block = 1;
	begin[0] = 0;	end[0] = maxNumber;	step[0] = 1;
	expected[0] = full;
	QTest::newRow("bit") << Bitmap(Bitmap::InvalidStatus, maxNumber) << 1 << block << begin << end << step << expected;

	block = 1;
	begin[0] = maxNumber - 1;	end[0] = -1;	step[0] = -1;
	expected[0] = full;
	QTest::newRow("bit reverse") << Bitmap(Bitmap::InvalidStatus, maxNumber) << 1 << block << begin << end << step << expected;

	block = blockSize;
	begin[0] = 0;	end[0] = maxNumber;	step[0] = block;
	expected[0] = full;
	QTest::newRow("bits") << Bitmap(Bitmap::InvalidStatus, maxNumber) << 1 << block << begin << end << step << expected;

	block = blockSize;
	begin[0] = maxNumber - block;	end[0] = -block;	step[0] = -block;
	expected[0] = full;
	QTest::newRow("bits reverse") << Bitmap(Bitmap::InvalidStatus, maxNumber) << 1 << block << begin << end << step << expected;

	block = 1;
	begin[0] = 0;	begin[1] = 1;
	end[0] = maxNumber;	end[1] = maxNumber + 1;
	step[0] = step[1] = 2;
	expected[0] = bitEven;	expected[1] = full;
	QTest::newRow("bit even odd") << Bitmap(Bitmap::InvalidStatus, maxNumber) << 2 << block << begin << end << step << expected;

	block = 1;
	begin[0] = 1;	begin[1] = 0;
	end[0] = maxNumber + 1;	end[1] = maxNumber;
	step[0] = step[1] = 2;
	expected[0] = bitOdd;	expected[1] = full;
	QTest::newRow("bit odd even") << Bitmap(Bitmap::InvalidStatus, maxNumber) << 2 << block << begin << end << step << expected;

	block = blockSize;
	begin[0] = 0;	begin[1] = block;
	end[0] = maxNumber;	end[1] = maxNumber + block;
	step[0] = step[1] = block * 2;
	expected[0] = bitsEven;	expected[1] = full;
	QTest::newRow("bits even odd") << Bitmap(Bitmap::InvalidStatus, maxNumber) << 2 << block << begin << end << step << expected;

	block = blockSize;
	begin[0] = block;	begin[1] = 0;
	end[0] = maxNumber + block;	end[1] = maxNumber;
	step[0] = step[1] = block * 2;
	expected[0] = bitsOdd;	expected[1] = full;
	QTest::newRow("bits odd even") << Bitmap(Bitmap::InvalidStatus, maxNumber) << 2 << block << begin << end << step << expected;
}

void TestBitmap::sequence()
{
	QFETCH(Bitmap, bitmap);
	QFETCH(int, n);
	QFETCH(int, block);
	QFETCH(QVector<int>, begin);	QFETCH(QVector<int>, end);	QFETCH(QVector<int>, step);
	QFETCH(QVector<QList<Bitmap::Segment>>, expected);

	// empty
	QBENCHMARK_ONCE
	{
		for (int x = 0; x < n; ++x)
			for (int i = begin[x]; i != end[x]; i += step[x])
				bitmap.updateStatus(Bitmap::getStatusUpdater(Bitmap::EmptyStatus), Bitmap::Segment(i, i + block));
	}
	QCOMPARE(bitmap.segments(Bitmap::getStatusChecker(Bitmap::EmptyStatus)), expected[n - 1]);

	// requested
	for (int x = 0; x < n; ++x)
	{
		for (int i = begin[x]; i != end[x]; i += step[x])
			bitmap.updateStatus(Bitmap::getStatusUpdater(Bitmap::RequestedStatus), Bitmap::Segment(i, i + block));
		QCOMPARE(bitmap.segments(Bitmap::getStatusChecker(Bitmap::RequestedStatus)), expected[x]);
	}

	// downloaded
	for (int x = 0; x < n; ++x)
	{
		for (int i = begin[x]; i != end[x]; i += step[x])
			bitmap.updateStatus(Bitmap::getStatusUpdater(Bitmap::DownloadedStatus), Bitmap::Segment(i, i + block));
		QCOMPARE(bitmap.segments(Bitmap::getStatusChecker(Bitmap::DownloadedStatus)), expected[x]);
	}

	// empty again
	for (int x = 0; x < n; ++x)
	{
		for (int i = begin[x]; i != end[x]; i += step[x])
			bitmap.updateStatus(Bitmap::getStatusUpdater(Bitmap::EmptyStatus), Bitmap::Segment(i, i + block));
		QCOMPARE(bitmap.segments(Bitmap::getStatusChecker(Bitmap::EmptyStatus)), expected[x]);
	}

	// verified
	for (int x = 0; x < n; ++x)
	{
		for (int i = begin[x]; i != end[x]; i += step[x])
		{
			bitmap.updateStatus(Bitmap::getStatusUpdater(Bitmap::RequestedStatus), Bitmap::Segment(i, i + block));
			bitmap.updateStatus(Bitmap::getStatusUpdater(Bitmap::DownloadedStatus), Bitmap::Segment(i, i + block));
			bitmap.updateStatus(Bitmap::getStatusUpdater(Bitmap::VerifiedStatus), Bitmap::Segment(i, i + block));
		}
		QCOMPARE(bitmap.segments(Bitmap::getStatusChecker(Bitmap::VerifiedStatus)), expected[x]);
	}
}

void TestBitmap::constant_data()
{
	QTest::addColumn<QList<Bitmap::Segment>>("data");

	QList<Bitmap::Segment> data;
	data << Bitmap::Segment(65536,196608) << Bitmap::Segment(262144,393216) << Bitmap::Segment(524288,589824) << Bitmap::Segment(655360,983040) <<
		Bitmap::Segment(1048576,1179648) << Bitmap::Segment(1245184,1376256) << Bitmap::Segment(1441792,1572864) << Bitmap::Segment(1769472,1835008) <<
		Bitmap::Segment(1966080,2097152) << Bitmap::Segment(2162688,2359296) << Bitmap::Segment(2424832,3276800) << Bitmap::Segment(3407872,3538944) <<
		Bitmap::Segment(3604480,3932160) << Bitmap::Segment(3997696,4128768) << Bitmap::Segment(4194304,4390912) << Bitmap::Segment(4456448,5046272) <<
		Bitmap::Segment(5242880,5701632) << Bitmap::Segment(5963776,6094848) << Bitmap::Segment(6225920,6488064) << Bitmap::Segment(6553600,6815744) <<
		Bitmap::Segment(6946816,7012352) << Bitmap::Segment(7208960,7274496) << Bitmap::Segment(7471104,7667712) << Bitmap::Segment(7733248,7798784) <<
		Bitmap::Segment(8060928,8388608) << Bitmap::Segment(8519680,8716288) << Bitmap::Segment(9175040,9371648) << Bitmap::Segment(9568256,9830400) <<
		Bitmap::Segment(9961472,10092544) << Bitmap::Segment(10551296,11206656) << Bitmap::Segment(11272192,11337728) << Bitmap::Segment(11403264,11468800) <<
		Bitmap::Segment(11534336,11665408) << Bitmap::Segment(11730944,11796480) << Bitmap::Segment(12058624,12189696) << Bitmap::Segment(12255232,12713984) <<
		Bitmap::Segment(12779520,12845056) << Bitmap::Segment(12910592,12976128) << Bitmap::Segment(13041664,13172736) << Bitmap::Segment(13238272,14352384) <<
		Bitmap::Segment(14417920,14483456) << Bitmap::Segment(14680064,15007744) << Bitmap::Segment(15138816,16646144) << Bitmap::Segment(16711680,17563648) <<
		Bitmap::Segment(17629184,17694720) << Bitmap::Segment(17956864,18219008) << Bitmap::Segment(18415616,18481152) << Bitmap::Segment(18743296,19529728) <<
		Bitmap::Segment(19660800,20709376) << Bitmap::Segment(20840448,21168128) << Bitmap::Segment(21299200,21495808) << Bitmap::Segment(21561344,21626880) <<
		Bitmap::Segment(21692416,22675456) << Bitmap::Segment(22740992,22806528) << Bitmap::Segment(22872064,23003136) << Bitmap::Segment(23199744,23265280) <<
		Bitmap::Segment(23396352,23789568) << Bitmap::Segment(23920640,24576000) << Bitmap::Segment(24707072,24772608) << Bitmap::Segment(24838144,24969216) <<
		Bitmap::Segment(25165824,26345472) << Bitmap::Segment(26411008,26476544) << Bitmap::Segment(26607616,27656192) << Bitmap::Segment(27721728,27787264) <<
		Bitmap::Segment(27852800,27918336) << Bitmap::Segment(28049408,29425664) << Bitmap::Segment(29687808,29753344) << Bitmap::Segment(29818880,29884416) <<
		Bitmap::Segment(30015488,31916032) << Bitmap::Segment(32178176,32440320) << Bitmap::Segment(32571392,33292288) << Bitmap::Segment(33357824,33423360) <<
		Bitmap::Segment(33488896,33751040) << Bitmap::Segment(33882112,34209792) << Bitmap::Segment(34275328,34340864) << Bitmap::Segment(34603008,35061760) <<
		Bitmap::Segment(35323904,35389440) << Bitmap::Segment(35651584,35913728) << Bitmap::Segment(35979264,37158912) << Bitmap::Segment(37289984,37421056) <<
		Bitmap::Segment(37748736,37879808) << Bitmap::Segment(37945344,38469632) << Bitmap::Segment(38600704,38731776) << Bitmap::Segment(38797312,40042496) <<
		Bitmap::Segment(40304640,40501248) << Bitmap::Segment(40566784,40828928) << Bitmap::Segment(40894464,40960000) << Bitmap::Segment(41025536,41156608) <<
		Bitmap::Segment(41418752,41811968) << Bitmap::Segment(41877504,41943040) << Bitmap::Segment(42270720,42598400) << Bitmap::Segment(42663936,42795008) <<
		Bitmap::Segment(42860544,43450368) << Bitmap::Segment(43778048,44236800) << Bitmap::Segment(44302336,44630016) << Bitmap::Segment(45023232,45088768) <<
		Bitmap::Segment(45154304,46071808)<< Bitmap::Segment(46268416,46399488) << Bitmap::Segment(46530560,46596096) << Bitmap::Segment(46727168,46792704) <<
		Bitmap::Segment(46858240,46923776) << Bitmap::Segment(47054848,47972352) << Bitmap::Segment(48103424,48562176) << Bitmap::Segment(48758784,49807360) <<
		Bitmap::Segment(49872896,50003968) << Bitmap::Segment(50069504,50103820); 
	QTest::newRow("1st set") << data;

	data.clear();
	data << Bitmap::Segment(917504,933888) << Bitmap::Segment(2293760,2326528) << Bitmap::Segment(2899968,2916352) << Bitmap::Segment(6619136,6651904) <<
		Bitmap::Segment(7208960,7225344) << Bitmap::Segment(8208384,8241152) << Bitmap::Segment(9240576,9256960) << Bitmap::Segment(9781248,9797632) <<
		Bitmap::Segment(10043392,10059776) << Bitmap::Segment(10878976,10895360) << Bitmap::Segment(11141120,11173888) << Bitmap::Segment(13303808,13336576) <<
		Bitmap::Segment(13451264,13484032) << Bitmap::Segment(15466496,15482880) << Bitmap::Segment(16531456,16547840);
	QTest::newRow("2nd set") << data;
}

void TestBitmap::constant()
{
	QFETCH(QList<Bitmap::Segment>, data);
	Bitmap bitmap(Bitmap::InvalidStatus, Q_INT64_C(10000000000));

	foreach (const auto &segment, data)
	{
		bitmap.updateStatus(Bitmap::getStatusUpdater(Bitmap::EmptyStatus), segment);
		QVERIFY(bitmap.checkStatus(Bitmap::getStatusChecker(Bitmap::EmptyStatus), segment));
	}

	foreach (const auto &segment, data)
		QVERIFY(bitmap.checkStatus(Bitmap::getStatusChecker(Bitmap::EmptyStatus), segment));
}

void TestBitmap::simulate()
{
	static const int blockSize = 512 * 1024;
	static const int totalSize = 1024 * 1024 * 1024;
	srand(0);

	Bitmap bitmap(Bitmap::InvalidStatus, totalSize);
	bitmap.updateStatus(Bitmap::getStatusUpdater(Bitmap::EmptyStatus), Bitmap::Segment(0, totalSize));
	qint64 expectedEmpty = totalSize;
	qint64 expectedRequested = 0;
	qint64 expectedDownloaded = 0;

	QBENCHMARK_ONCE
	{
		while (true)
		{
			const qint64 empty = bitmap.count(Bitmap::getStatusChecker(Bitmap::EmptyStatus));
			const qint64 requested = bitmap.count(Bitmap::getStatusChecker(Bitmap::RequestedStatus));
			const qint64 downloaded = bitmap.count(Bitmap::getStatusChecker(Bitmap::DownloadedStatus));
			QCOMPARE(empty, expectedEmpty);
			QCOMPARE(requested, expectedRequested);
			QCOMPARE(downloaded, expectedDownloaded);

			if (downloaded == totalSize)	break;

			if (requested > 0 && (empty == 0 || (qrand() % 10) > 1))
			{
				QList<Bitmap::Segment> segments = bitmap.segments(Bitmap::getStatusChecker(Bitmap::RequestedStatus));
				const qint64 begin = segments[qrand() % segments.size()].begin;
				const qint64 end = begin + blockSize;
				bitmap.updateStatus(Bitmap::getStatusUpdater(Bitmap::DownloadedStatus), Bitmap::Segment(begin, end));

				expectedRequested -= blockSize;
				expectedDownloaded += blockSize;
			}
			else
			{
				QList<Bitmap::Segment> segments = bitmap.segments(Bitmap::getStatusChecker(Bitmap::EmptyStatus));
				const qint64 begin = segments[qrand() % segments.size()].begin;
				const qint64 end = begin + blockSize;
				bitmap.updateStatus(Bitmap::getStatusUpdater(Bitmap::RequestedStatus), Bitmap::Segment(begin, end));

				expectedEmpty -= blockSize;
				expectedRequested += blockSize;
			}
		}
	}
}

}

}

QTEST_MAIN(UnitTest::Utility::TestBitmap)

