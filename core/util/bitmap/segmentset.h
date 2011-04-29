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
#ifndef UTILITY_SEGMENTSET_H
#define UTILITY_SEGMENTSET_H

#include <QMap>
#include <QPair>

#include <functional>

namespace Utility
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class SegmentSet
{
public:
	struct Segment
	{
		qint64 begin, end;

		Segment(qint64 begin, qint64 end) : begin(begin), end(end)
		{
			Q_ASSERT_X(begin < end, Q_FUNC_INFO, "Invalid range");
		}

		bool operator==(const Segment &other) const
		{
			return begin == other.begin && end == other.end;
		}

		qint64 size() const
		{
			return end - begin;
		}

		qint64 middle() const
		{
			return begin + size() / 2;
		}
	};

	enum SegmentStatus
	{
		InvalidStatus,
		EmptyStatus,
		MaskedStatus,
		RequestedStatus,
		DownloadedStatus,
		VerifiedStatus
	};

	struct SegmentData
	{
		Segment segment;
		SegmentStatus status;

		SegmentData(const Segment &segment, SegmentStatus status)
			: segment(segment), status(status) { }
	};

	typedef std::function<bool(SegmentStatus)> StatusChecker;
	typedef std::function<void(SegmentStatus&)> StatusUpdater;

	explicit SegmentSet(SegmentStatus status, qint64 size) : size(size)
	{
		data.insert(0, status);
	}

	qint64 getSize() const
	{
		return size;
	}

	void setSize(qint64 size)
	{
		Q_ASSERT_X(this->size == 0, Q_FUNC_INFO, "Already set");
		Q_ASSERT_X(size != 0, Q_FUNC_INFO, "Invalid size");
		this->size = size;
	}

	qint64 count(const StatusChecker &statusChecker) const;

	QList<SegmentData> segments() const;
	QList<Segment> segments(const StatusChecker &statusChecker) const;
	QList<Segment> segments(const StatusChecker &statusChecker, const Segment &segment) const;

	bool checkStatus(const StatusChecker &statusChecker, const Segment &segment) const;
	void updateStatus(const StatusUpdater &statusUpdater, const Segment &segment);
private:
	typedef std::function<bool(SegmentStatus, const Segment&)> iterateAction;

	typedef QMap<qint64, SegmentStatus> dataMap;

	qint64 size;
	dataMap data;

	qint64 next(dataMap::ConstIterator iter) const;

	void update(const StatusUpdater &statusUpdater, const Segment &segment, dataMap::Iterator iter);

	void iterate(const iterateAction &action) const;
	void iterate(const iterateAction &action, const Segment &segment) const;
};

}

#endif
