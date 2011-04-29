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
#ifndef PROTOCOL_BITTORRENT_BLOCKSELECTOR_H
#define PROTOCOL_BITTORRENT_BLOCKSELECTOR_H

#include "util/bitmap/bitmap.h"

#include <QSet>

namespace Protocol
{

namespace BitTorrent
{

class MetaInfo;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class BlockSelector
{
public:
	static const int minimalBlockSize = 8 * 1024;
	static const int defaultBlockSize = 16 * 1024;
	static const int maximalBlockSize = 64 * 1024;

	inline BlockSelector() {}
	virtual ~BlockSelector() {}

	virtual QString name() const = 0;

	virtual void updateVerified(int beginIndex, int endIndex) = 0;
	virtual void updateAvailability(int index, int delta) = 0;

	bool select(const Utility::Bitmap &self, const QBitArray &peer, const MetaInfo *metaInfo,
			int *index, int *offset, int *size);
protected:
	enum PieceStatus
	{
		NormalPiece,
		NotInterestedPiece,
		NotAvailablePiece
	};

	static PieceStatus checkPiece(const Utility::Bitmap &self, const QBitArray &peer, const MetaInfo *metaInfo, int index);

	virtual int selectPiece(const Utility::Bitmap &self, const QBitArray &peer, const MetaInfo *metaInfo) = 0;
private:
	BlockSelector(const BlockSelector &);
	BlockSelector& operator=(const BlockSelector &);

	static void findBlock(const Utility::Bitmap &self, const MetaInfo *metaInfo, int index, int *offset, int *size);

	QSet<int> partialSelected;
};

}

}

#endif
