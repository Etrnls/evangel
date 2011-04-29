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
#ifndef PROTOCOL_BITTORRENT_LRFSELECTOR_H
#define PROTOCOL_BITTORRENT_LRFSELECTOR_H

#include "blockselector.h"

#include <QVector>
#include <QMap>

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class LRFSelector : public BlockSelector
{
public:
	LRFSelector(const MetaInfo *metaInfo);

	virtual QString name() const
	{
		return QLatin1String("Local Rarest First");
	}

	virtual void updateVerified(int beginIndex, int endIndex);
	virtual void updateAvailability(int index, int delta);
private:
	struct AvailabilityInfo
	{
		int index;
		int availability;
		int randomness;

		inline AvailabilityInfo(int index, int availability, int randomness)
			: index(index), availability(availability), randomness(randomness) {}
		inline bool operator<(const AvailabilityInfo &other) const
		{
			if (availability != other.availability)
				return availability < other.availability;
			else if (randomness != other.randomness)
				return randomness < other.randomness;
			else
				return index < other.index;
		}
	};

	virtual int selectPiece(const Utility::Bitmap &self, const QBitArray &peer, const MetaInfo *metaInfo);

	QVector<int> availability;
	QVector<int> randomness;
	QMap<AvailabilityInfo, QHashDummyValue> availabilityInfo;
};

}

}

#endif
