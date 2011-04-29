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
#ifndef UTILITY_BLOCK_H
#define UTILITY_BLOCK_H

#include <QByteArray>

namespace Utility
{

/*!
 * \brief Represents a block of data
 *
 * This is used to transport data from AbstractTransfer to TransferTask via signal & slot
 * \author Etrnls <Etrnls@gmail.com>
 */
struct Block
{
	qint64	offset;
	qint64	size;
	QByteArray	data;

	inline Block(qint64 offset, const QByteArray &data)
		: offset(offset), size(data.size()), data(data) {}
	inline Block(qint64 offset, qint64 size)
		: offset(offset), size(size) {}
};

enum BlockUpdateType
{
	BlockRequested,		//!< A block of data has been requested
	BlockCanceled,		//!< A block of data has been canceled

	BlockUnmasked,		//!< A block of data has been unmasked
	BlockMasked,		//!< A block of data has been masked

	BlockDownloaded,	//!< A block of data has been downloaded
	BlockUploaded,		//!< A block of data has been uploaded

	BlockVerified,		//!< A block of data has pass the verification
	BlockCorrupted		//!< A block of data has failed the verification
};

}

#endif
