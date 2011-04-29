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
#ifndef UTILITY_BITMAP_H
#define UTILITY_BITMAP_H

#include "segmentset.h"

namespace Utility
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class Bitmap
{
public:
	typedef SegmentSet::Segment Segment;
	typedef SegmentSet::SegmentStatus SegmentStatus;
	typedef SegmentSet::SegmentData SegmentData;
	static const SegmentStatus InvalidStatus = SegmentSet::InvalidStatus;
	static const SegmentStatus EmptyStatus = SegmentSet::EmptyStatus;
	static const SegmentStatus MaskedStatus = SegmentSet::MaskedStatus;
	static const SegmentStatus RequestedStatus = SegmentSet::RequestedStatus;
	static const SegmentStatus DownloadedStatus = SegmentSet::DownloadedStatus;
	static const SegmentStatus VerifiedStatus = SegmentSet::VerifiedStatus;

	typedef SegmentSet::StatusChecker StatusChecker;
	typedef SegmentSet::StatusUpdater StatusUpdater;

	static const StatusChecker &getStatusChecker(SegmentStatus status);
	static const StatusUpdater &getStatusUpdater(SegmentStatus status);

	explicit Bitmap(SegmentStatus status = InvalidStatus, qint64 size = 0) : segmentSet(status, size)
	{
	}

	qint64 size() const
	{
		return segmentSet.getSize();
	}

	void resize(qint64 size)
	{
		segmentSet.setSize(size);
	}

	qint64 count(const StatusChecker &statusChecker) const
	{
		return segmentSet.count(statusChecker);
	}

	QList<SegmentData> segments() const
	{
		return segmentSet.segments();
	}

	QList<Segment> segments(const StatusChecker &statusChecker) const
	{
		return segmentSet.segments(statusChecker);
	}

	QList<Segment> segments(const StatusChecker &statusChecker, const Segment &segment) const
	{
		return segmentSet.segments(statusChecker, segment);
	}

	bool checkStatus(const StatusChecker &statusChecker, const Segment &segment) const
	{
		return segmentSet.checkStatus(statusChecker, segment);
	}

	void updateStatus(const StatusUpdater &statusUpdater, const Segment &segment)
	{
		segmentSet.updateStatus(statusUpdater, segment);
	}
private:
	static const StatusChecker DummyChecker;
	static const StatusChecker EmptyStatusChecker;
	static const StatusChecker MaskedStatusChecker;
	static const StatusChecker RequestedStatusChecker;
	static const StatusChecker DownloadedStatusChecker;
	static const StatusChecker VerifiedStatusChecker;

	static const StatusUpdater DummyUpdater;
	static const StatusUpdater EmptyStatusUpdater;
	static const StatusUpdater MaskedStatusUpdater;
	static const StatusUpdater RequestedStatusUpdater;
	static const StatusUpdater DownloadedStatusUpdater;
	static const StatusUpdater VerifiedStatusUpdater;

	SegmentSet segmentSet;
};

}

#endif
