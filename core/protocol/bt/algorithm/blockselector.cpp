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
#include "blockselector.h"

#include "protocol/bt/util/metainfo.h"

#include "util/numeric/random.h"

#include <QBitArray>

namespace Protocol
{

namespace BitTorrent
{

/*!
 * We select from partial selected piece(s) first,
 * if there's no suitable piece then we call selectPiece (which should be implemented by sub class).
 */
bool BlockSelector::select(const Utility::Bitmap &self, const QBitArray &peer, const MetaInfo *metaInfo,
				int *index, int *offset, int *size)
{
	foreach (int i, partialSelected)
		switch (checkPiece(self, peer, metaInfo, i))
		{
		case NormalPiece:
			*index = i;
			findBlock(self, metaInfo, *index, offset, size);
			return true;
		case NotInterestedPiece:
			partialSelected.remove(i);
			break;
		case NotAvailablePiece:
			break;
		default:
			Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid check piece result");
			break;
		}

	if ((*index = selectPiece(self, peer, metaInfo)) >= 0)
	{
		partialSelected.insert(*index);
		findBlock(self, metaInfo, *index, offset, size);
		return true;
	}

	return false;
}

BlockSelector::PieceStatus BlockSelector::checkPiece(const Utility::Bitmap &self, const QBitArray &peer,
						const MetaInfo *metaInfo, int index)
{
	const qint64 begin = metaInfo->getBeginningOffset(index);
	const qint64 end = metaInfo->getEndingOffset(index);
	const Utility::Bitmap::Segment segment(begin, end);

	if (self.checkStatus([](Utility::Bitmap::SegmentStatus status)->bool
	{
		return !Utility::Bitmap::getStatusChecker(Utility::Bitmap::EmptyStatus)(status);
	}, segment))
		return NotInterestedPiece;

	if (!peer.testBit(index))
		return NotAvailablePiece;

	return NormalPiece;
}

void BlockSelector::findBlock(const Utility::Bitmap &self,
				const MetaInfo *metaInfo, int index, int *offset, int *size)
{
	const qint64 begin = metaInfo->getBeginningOffset(index);
	const qint64 end = metaInfo->getEndingOffset(index);
	const Utility::Bitmap::Segment segment(begin, end);

	// we have a little random here for better performance in warmup & endgame

	const auto &segments = self.segments(Utility::Bitmap::getStatusChecker(Utility::Bitmap::EmptyStatus), segment);
	const auto &chosenSegment = segments[Utility::rand(segments.size())];

	*offset = chosenSegment.begin - begin;
	*size = qMin<qint64>(defaultBlockSize, chosenSegment.size());

	const int delta = *size * static_cast<int>(Utility::rand(chosenSegment.size() / *size));
	*offset += delta;
	*size = qMin<qint64>(*size, chosenSegment.end - (chosenSegment.begin + delta));

	Q_ASSERT_X(chosenSegment.begin <= begin + *offset, Q_FUNC_INFO, "Invalid offset");
	Q_ASSERT_X(begin + *offset + *size <= chosenSegment.end, Q_FUNC_INFO, "Invalid size");
}

}

}
