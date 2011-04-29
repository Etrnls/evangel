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
#include "lrfselector.h"

#include "protocol/bt/util/metainfo.h"

#include "util/numeric/random.h"

#include <QVector>

#include <climits>

namespace Protocol
{

namespace BitTorrent
{

LRFSelector::LRFSelector(const MetaInfo *metaInfo)
	: availability(metaInfo->getPieces(), 0), randomness(metaInfo->getPieces())
{
	availability.squeeze();
	randomness.squeeze();

	for (int i = 0; i < metaInfo->getPieces(); ++i)
		availabilityInfo.insert(AvailabilityInfo(i, 0, randomness[i]), QHashDummyValue());
}

void LRFSelector::updateVerified(int beginIndex, int endIndex)
{
	for (int i = beginIndex; i < endIndex; ++i)
		availabilityInfo.remove(AvailabilityInfo(i, availability[i], randomness[i]));
}

void LRFSelector::updateAvailability(int index, int delta)
{
	const auto info = availabilityInfo.find(AvailabilityInfo(index, availability[index], randomness[index]));
	if (info != availabilityInfo.constEnd())
		availabilityInfo.erase(info);

	availability[index] += delta;
	randomness[index] = Utility::rand(INT_MAX);
	availabilityInfo.insert(AvailabilityInfo(index, availability[index], randomness[index]), QHashDummyValue());
}

int LRFSelector::selectPiece(const Utility::Bitmap &self, const QBitArray &peer, const MetaInfo *metaInfo)
{
	for (auto iter = availabilityInfo.constBegin(); iter != availabilityInfo.constEnd(); ++iter)
		if (checkPiece(self, peer, metaInfo, iter.key().index) == NormalPiece)
			return iter.key().index;

	return -1;
}

}

}
