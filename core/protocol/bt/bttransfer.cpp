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
#include "bttransfer.h"
#include "btclient.h"
#include "util/metainfo.h"

#include "log.h"

namespace Protocol
{

namespace BitTorrent
{

BTTransfer::BTTransfer(const MetaInfo *metaInfo) : metaInfo(metaInfo), client(0)
{
	if (metaInfo->getFiles().size() > 1)
	{
		qint64 offset = 0;
		foreach (const auto &metaFileInfo, metaInfo->getFiles())
		{
			if (offset % metaInfo->getPieceSize(-1))
			{
				const int index = metaInfo->getIndex(offset);
				if (!pieceData.contains(index))
				{
					log(this) << QString::fromLatin1("Found cross file piece [%1]").arg(index) << endl;
					pieceData.insert(index, Piece(Piece::CrossFilePiece, metaInfo->getPieceSize(index)));
				}
			}
			offset += metaFileInfo.size;
		}
	}
}

BTTransfer::~BTTransfer()
{
	delete metaInfo;
}

BTTransfer::Capability BTTransfer::capability() const
{
	return DownloadCapability | UploadCapability | VerifyCapability;
}

QList<BTTransfer::FileInformation> BTTransfer::files() const
{
	QList<FileInformation> result;
	qint64 offset = 0;
	foreach (const auto &metaFileInfo, metaInfo->getFiles())
	{
		result.append(FileInformation(metaFileInfo.path, offset, metaFileInfo.size));
		offset += metaFileInfo.size;
	}
	return result;
}

Utility::Bitmap BTTransfer::bitmap() const
{
	if (!bitmapCache.size())
	{
		bitmapCache = Utility::Bitmap(Utility::Bitmap::MaskedStatus, metaInfo->getSize());

		for (auto iter = references.constBegin(); iter != references.constEnd(); ++iter)
			foreach (const auto &segmentData, iter.key()->bitmap().segments())
			{
				const Utility::Bitmap::Segment segment(iter.value() + segmentData.segment.begin , iter.value() + segmentData.segment.end);
				bitmapCache.updateStatus([segmentData](Utility::Bitmap::SegmentStatus &status)
				{
					status = segmentData.status;
				}, segment);
			}

		for (auto iter = pieceData.constBegin(); iter != pieceData.constEnd(); ++iter)
			foreach (const auto &segmentData, iter.value().bitmap.segments())
			{
				const qint64 offset = metaInfo->getBeginningOffset(iter.key());
				const Utility::Bitmap::Segment segment(offset + segmentData.segment.begin , offset + segmentData.segment.end);
				switch (segmentData.status)
				{
				case Utility::Bitmap::DownloadedStatus:
					bitmapCache.updateStatus([](Utility::Bitmap::SegmentStatus &status)
					{
						if (status != Utility::Bitmap::VerifiedStatus)
							status = Utility::Bitmap::DownloadedStatus;
					}, segment);
					break;
				case Utility::Bitmap::VerifiedStatus:
					bitmapCache.updateStatus([](Utility::Bitmap::SegmentStatus &status)
					{
						status = Utility::Bitmap::VerifiedStatus;
					}, segment);
					break;
				default:
					break;
				}
			}
	}
	return bitmapCache;
}

void BTTransfer::updateBlock(Utility::BlockUpdateType type, Utility::Block block)
{
	switch (type)
	{
	case Utility::BlockDownloaded:
	{
		Q_ASSERT_X(metaInfo->getIndex(block.offset) == metaInfo->getIndex(block.offset + block.size - 1), Q_FUNC_INFO, "Block too large");
		const int index = metaInfo->getIndex(block.offset);
		const int begin = block.offset - metaInfo->getBeginningOffset(index);
		const int end = begin + block.size;

		if (!pieceData.contains(index))
			pieceData.insert(index, Piece(Piece::NormalPiece, metaInfo->getPieceSize(index)));

		const Utility::Bitmap::Segment segment(begin, end);
		Piece &piece = pieceData.find(index).value();
		piece.bitmap.updateStatus([](Utility::Bitmap::SegmentStatus &status)
		{
			// not using DownloadedStatusUpdater because we don't care about InvalidStatus
			Q_ASSERT_X(status == Utility::Bitmap::InvalidStatus ||
				status == Utility::Bitmap::EmptyStatus ||
				status == Utility::Bitmap::DownloadedStatus,
				"BTTransfer::updateBlock", "Invalid status");
			status = Utility::Bitmap::DownloadedStatus;
		}, segment);
		piece.data.replace(begin, end - begin, block.data);

		// piece is not full, don't pass it the the references
		if (!piece.isFull())	return;
		
		block = Utility::Block(metaInfo->getBeginningOffset(index), piece.data);
		if (piece.type == Piece::NormalPiece)	pieceData.remove(index);
	}
		break;
	case Utility::BlockVerified:
	{
		Q_ASSERT_X(metaInfo->getIndex(block.offset) == metaInfo->getIndex(block.offset + block.size - 1), Q_FUNC_INFO, "Block too large");
		const int index = metaInfo->getIndex(block.offset);
		if (pieceData.contains(index))
		{
			const int begin = block.offset - metaInfo->getBeginningOffset(index);
			const int end = begin + block.size;
			const Utility::Bitmap::Segment segment(begin, end);
			Piece &piece = pieceData.find(index).value();
			piece.bitmap.updateStatus(Utility::Bitmap::getStatusUpdater(Utility::Bitmap::VerifiedStatus), segment);
		}
	}
		break;
	case Utility::BlockCorrupted:
	{
		Q_ASSERT_X(metaInfo->getIndex(block.offset) == metaInfo->getIndex(block.offset + block.size - 1), Q_FUNC_INFO, "Block too large");
		const int index = metaInfo->getIndex(block.offset);

		Q_ASSERT_X(block.offset == metaInfo->getBeginningOffset(index) && block.size == metaInfo->getPieceSize(index),
				Q_FUNC_INFO, "Invalid block");

		const auto &piece = pieceData.find(index);
		if (piece != pieceData.constEnd())
		{
			Utility::Bitmap &bitmap = piece.value().bitmap;
			const Utility::Bitmap::Segment segment(0, bitmap.size());
			bitmap.updateStatus(Utility::Bitmap::getStatusUpdater(Utility::Bitmap::EmptyStatus), segment);
		}
	}
		break;
	default:
		break;
	}

	// now pass to the references
	for (auto iter = references.constBegin(); iter != references.constEnd(); ++iter)
	{
		if (iter.value() + iter.key()->getSize() <= block.offset)	continue;
		if (block.offset + block.size <= iter.value())	continue;

		const qint64 begin = qMax(block.offset, iter.value());
		const qint64 end = qMin(block.offset + block.size, iter.value() + iter.key()->getSize());
		const qint64 size = end - begin;

		iter.key()->updateBlock(type, block.data.isEmpty() ?
				Utility::Block(begin - iter.value(), size) :
				Utility::Block(begin - iter.value(), block.data.mid(begin - block.offset, size)));
	}
}

QByteArray BTTransfer::readBlock(Utility::Block block) const
{
	Q_ASSERT_X(metaInfo->getIndex(block.offset) == metaInfo->getIndex(block.offset + block.size - 1), Q_FUNC_INFO, "Block too large");

	// try to read from local first
	const int index = metaInfo->getIndex(block.offset);
	const auto &piece = pieceData.find(index);
	if (piece != pieceData.constEnd())
	{
		const int begin = block.offset - metaInfo->getBeginningOffset(index);
		const int end = begin + block.size;

		return piece.value().data.mid(begin, end - begin);
	}

	QByteArray result(block.size, Qt::Uninitialized);

	for (auto iter = references.constBegin(); iter != references.constEnd(); ++iter)
	{
		if (iter.value() + iter.key()->getSize() <= block.offset)	continue;
		if (block.offset + block.size <= iter.value())	continue;

		const qint64 begin = qMax(block.offset, iter.value());
		const qint64 end = qMin(block.offset + block.size, iter.value() + iter.key()->getSize());
		const qint64 size = end - begin;


		result.replace(begin - block.offset, size, iter.key()->readBlock(Utility::Block(begin - iter.value(), size)));
	}

	return result;
}

void BTTransfer::start(FileTask *task)
{
	if (!client)	client = new BTClient(this);

	updateBlock(Utility::BlockUnmasked, Utility::Block(referenceOffset(task), task->getSize()));
}

void BTTransfer::pause(FileTask *task)
{
	Q_ASSERT_X(client, Q_FUNC_INFO, "Invalid client");

	updateBlock(Utility::BlockMasked, Utility::Block(referenceOffset(task), task->getSize()));
}

void BTTransfer::stop(FileTask *task)
{
	Q_ASSERT_X(client, Q_FUNC_INFO, "Invalid client");

	updateBlock(Utility::BlockMasked, Utility::Block(referenceOffset(task), task->getSize()));

	// Check if all references are stopped, if so, delete the client
	for (auto iter = references.constBegin(); iter != references.constEnd(); ++iter)
	{
		if (iter.key() == task)	continue;	// this is the one that asked us to stop
		if (iter.key()->getState() != FileTask::StoppedState)	return;
	}
	delete client;
	client = 0;
}

void BTTransfer::referenceStateChanged()
{
	FileTask *task = qobject_cast<FileTask*>(sender());
	Q_ASSERT_X(references.contains(task), Q_FUNC_INFO, "Invalid task");

	if (task->getState() != FileTask::RunningState)	return;

	QSet<int> indexes;
	const qint64 offset = referenceOffset(task);
	const qint64 size = task->getSize();
	if (offset % metaInfo->getPieceSize(-1))	indexes.insert(metaInfo->getIndex(offset));
	if ((offset + size) % metaInfo->getPieceSize(-1))	indexes.insert(metaInfo->getIndex(offset + size));

	foreach (int index, indexes)
	{
		const auto &piece = pieceData.find(index);
		if (piece == pieceData.constEnd())	continue;

		if (piece->type == Piece::CrossFilePiece && piece->isFull())
		{
			const Utility::Block block(metaInfo->getBeginningOffset(index), piece->data);
			updateBlock(Utility::BlockDownloaded, block);
		}
	}
}

void BTTransfer::invalidateBitmapCache()
{
	bitmapCache = Utility::Bitmap();
}

void BTTransfer::storeReference(FileTask *task, qint64 offset)
{
	Q_ASSERT_X(!references.contains(task), Q_FUNC_INFO, "Invalid task : already exists");
	references.insert(task, offset);
	connect(task, SIGNAL(stateChanged()), SLOT(referenceStateChanged()));
}

void BTTransfer::removeReference(FileTask *task)
{
	Q_ASSERT_X(references.contains(task), Q_FUNC_INFO, "Invalid task : not found");
	references.remove(task);
	disconnect(task, SIGNAL(stateChanged()), this, SLOT(referenceStateChanged()));
}

qint64 BTTransfer::referenceOffset(FileTask *task) const
{
	Q_ASSERT_X(references.contains(task), Q_FUNC_INFO, "Invalid task : not found");
	return references.value(task);
}

int BTTransfer::referenceCount() const
{
	return references.size();
}

void BTTransfer::bitmapUpdating(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment)
{
	Q_UNUSED(status);
	Q_UNUSED(segment);
	invalidateBitmapCache();
}

}

}
