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
#ifndef PROTOCOL_BITTORRENT_BTTRANSFER_H
#define PROTOCOL_BITTORRENT_BTTRANSFER_H

#include "abstracttransfer.h"

#include <QHash>
#include <QMap>

namespace Protocol
{

namespace BitTorrent
{

class MetaInfo;
class BTClient;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class BTTransfer : public AbstractTransfer
{
	Q_OBJECT
	Q_CLASSINFO("log", "BitTorrent")
public:
	explicit BTTransfer(const MetaInfo *metaInfo);
	virtual ~BTTransfer();

	inline const MetaInfo *getMetaInfo() const
	{
		return metaInfo;
	}

	// ----- Abstract Interfaces -----
	virtual Capability capability() const;
	virtual QList<FileInformation> files() const;
	virtual Utility::Bitmap bitmap() const;

	virtual void updateBlock(Utility::BlockUpdateType type, Utility::Block block);
	virtual QByteArray readBlock(Utility::Block block) const;
	// -------------------------------
public slots:
	virtual void start(FileTask *task);
	virtual void pause(FileTask *task);
	virtual void stop(FileTask *task);
private slots:
	void referenceStateChanged();
	void invalidateBitmapCache();
private:
	struct Piece
	{
		enum PieceType
		{
			NormalPiece,
			CrossFilePiece
		};

		PieceType type;
		QByteArray data;
		Utility::Bitmap bitmap;

		inline Piece(PieceType type, int size)
			: type(type), data(size, Qt::Uninitialized),
			  bitmap(Utility::Bitmap::EmptyStatus, size)
		{
			data.squeeze();
		}

		inline bool isFull() const
		{
			const Utility::Bitmap::Segment segment(0, bitmap.size());
			return bitmap.checkStatus(Utility::Bitmap::getStatusChecker(Utility::Bitmap::DownloadedStatus), segment);
		}
	};

	// ----- Abstract Interfaces -----
	virtual void storeReference(FileTask *task, qint64 offset);
	virtual void removeReference(FileTask *task);
	virtual qint64 referenceOffset(FileTask *task) const;
	virtual int referenceCount() const;

	virtual void bitmapUpdating(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment);
	// -------------------------------

	mutable Utility::Bitmap bitmapCache;

	QMap<FileTask*, qint64> references;
	QHash<int, Piece> pieceData;

	const MetaInfo * const metaInfo;
	BTClient *client;
};

}

}

#endif
