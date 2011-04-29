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
#include "segmentset.h"

namespace Utility
{

qint64 SegmentSet::count(const StatusChecker &statusChecker) const
{
	qint64 result = 0;
	iterate([statusChecker, &result](SegmentStatus status, const Segment &segment)->bool
	{
		if (statusChecker(status))	result += segment.size();
		return true;
	});
	return result;
}

QList<SegmentSet::SegmentData> SegmentSet::segments() const
{
	QList<SegmentData> result;
	iterate([&result](SegmentStatus status, const Segment &segment)->bool
	{
		result.append(SegmentSet::SegmentData(segment, status));
		return true;
	});
	return result;
}

QList<SegmentSet::Segment> SegmentSet::segments(const StatusChecker &statusChecker) const
{
	QList<Segment> result;
	iterate([statusChecker, &result](SegmentStatus status, const Segment &segment)->bool
	{
		if (statusChecker(status))	result.append(segment);
		return true;
	});
	return result;
}

QList<SegmentSet::Segment> SegmentSet::segments(const StatusChecker &statusChecker, const Segment &segment) const
{
	QList<Segment> result;
	iterate([statusChecker, &result](SegmentStatus status, const Segment &segment)->bool
	{
		if (statusChecker(status))	result.append(segment);
		return true;
	}, segment);
	return result;
}

bool SegmentSet::checkStatus(const StatusChecker &statusChecker, const Segment &segment) const
{
	bool result = true;
	iterate([statusChecker, &result](SegmentStatus status, const Segment &segment)->bool
	{
		Q_UNUSED(segment);
		if (!statusChecker(status))	return result = false;
		return true;
	}, segment);
	return result;
}

void SegmentSet::updateStatus(const StatusUpdater &statusUpdater, const Segment &segment)
{
	Q_ASSERT_X(0 <= segment.begin && segment.end <= size, Q_FUNC_INFO, "Invalid segment");

	auto iter = data.lowerBound(segment.begin);

	// find the first segment
	if (iter == data.constEnd())
	{
		update(statusUpdater, segment, --iter);
		return;
	}
	else
	{
		if (iter.key() != segment.begin)	--iter;
		update(statusUpdater, segment, iter);
	}

	// now iter points to the first segment found

	while (segment.end > next(iter))
	{
		if (++iter == data.constEnd())	break;
		update(statusUpdater, Segment(iter.key(), segment.end), iter);
	}

	// merge
	iter = data.lowerBound(segment.begin);
	if (iter != data.constBegin())	--iter;

	forever
	{
		auto next = iter;	++next;
		if (next == data.constEnd())	break;
		if (iter.value() == next.value())
		{
			next = data.erase(next);
			iter = next;	--iter;
		}
		else
			iter = next;
		if (segment.end <= iter.key())	break;
	}
}

qint64 SegmentSet::next(dataMap::ConstIterator iter) const
{
	++iter;
	return iter != data.constEnd() ? iter.key() : size;
}

void SegmentSet::update(const StatusUpdater &statusUpdater, const Segment &segment, dataMap::Iterator iter)
{
	SegmentStatus oldStatus = iter.value();
	SegmentStatus newStatus = iter.value();
	statusUpdater(newStatus);

	if (newStatus != oldStatus)
	{
		const bool inside = segment.end < next(iter);
		if (segment.begin == iter.key())
			iter.value() = newStatus;
		else
			data.insert(segment.begin, newStatus);
		if (inside)	data.insert(segment.end, oldStatus);
	}
}

void SegmentSet::iterate(const iterateAction &action) const
{
	for (auto iter = data.constBegin(); iter != data.constEnd(); ++iter)
	{
		const qint64 begin = iter.key();
		const qint64 end = next(iter);
		const SegmentStatus status = iter.value();
		action(status, Segment(begin, end));
	}
}

void SegmentSet::iterate(const iterateAction &action, const Segment &segment) const
{
	Q_ASSERT_X(0 <= segment.begin && segment.end <= size, Q_FUNC_INFO, "Invalid segment");
	
	auto iter = data.lowerBound(segment.begin);

	// find the first segment
	if (iter == data.constEnd())
	{
		--iter;
		// iter.key() < segment.begin < segment.end <= size
		action(iter.value(), segment);
		return;
	}
	else
	{
		if (iter.key() != segment.begin)	--iter;
		if (!action(iter.value(), Segment(segment.begin, qMin(segment.end, next(iter)))))
			return;
	}

	// now iter points to the first segment found

	while (segment.end > next(iter))
	{
		++iter;
		if (!action(iter.value(), Segment(iter.key(), qMin(segment.end, next(iter)))))
			return;
	}
}

}

