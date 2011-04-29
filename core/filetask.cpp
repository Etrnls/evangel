/***************************************************************************
 *   Copyright (C) 2011 by Etrnls                                          *
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
#include "filetask.h"
#include "abstracttransfer.h"
#include "log.h"

#include <QDir>

FileTask::FileTask(const QString &file, qint64 size) : file(file, size)
{
}

FileTask::~FileTask()
{
	Q_ASSERT_X(getState() == StoppedState, Q_FUNC_INFO, "Deleting a non-stopped task");
	foreach(AbstractTransfer *transfer, transfers)
		delTransfer(transfer);
}

void FileTask::addTransfer(AbstractTransfer *transfer, qint64 offset)
{
	// no transfer for zero-size task
	if (file.size == 0)	return;

	// no non-uploadable transfer for completed task
	if (isCompleted() && !transfer->capability().testFlag(AbstractTransfer::UploadCapability))
		return;

	transfer->addReference(this, offset);
	transfers.insert(transfer);

	if (getState() == RunningState || getState() == SharingState)	transfer->start(this);
}

void FileTask::delTransfer(AbstractTransfer *transfer)
{
	switch (getState())
	{
	case RunningState:
	case SharingState:
	case PausedState:
		transfer->stop(this);
		break;
	default:
		break;
	}

	transfers.remove(transfer);
	transfer->delReference(this);
}

/*!
 * \brief Updates a block
 *
 * Transfer should call this function to inform the FileTask that a new block
 * has been downloaded or a block has passed/failed the verification etc.
 * The bitmap is updated corresponded and the news is broadcasted by bitmapUpdated().
 *
 */
void FileTask::updateBlock(Utility::BlockUpdateType type, const Utility::Block &block)
{
	switch (type)
	{
	case Utility::BlockRequested:
	{
		/// \todo warmup & endgame
		const Utility::Bitmap::Segment segment(block.offset, block.offset + block.size);
		updateBitmap(Utility::Bitmap::RequestedStatus, segment);
	}
		break;
	case Utility::BlockCanceled:
	{
		// This is rare, so we don't have a 'CancelUpdater'
		const auto &segments = file.bitmap.segments(Utility::Bitmap::getStatusChecker(Utility::Bitmap::RequestedStatus),
							Utility::Bitmap::Segment(block.offset, block.offset + block.size));
		foreach (const auto &segment, segments)
		{
			updateBitmap(Utility::Bitmap::EmptyStatus, segment);
		}
	}
		break;
	case Utility::BlockUnmasked:
	{
		const Utility::Bitmap::Segment segment(block.offset, block.offset + block.size);
		updateBitmap(Utility::Bitmap::EmptyStatus, segment);
	}
		break;
	case Utility::BlockMasked:
	{
		const Utility::Bitmap::Segment segment(block.offset, block.offset + block.size);
		updateBitmap(Utility::Bitmap::MaskedStatus, segment);
	}
		break;
	case Utility::BlockDownloaded:
	{
		if (getState() != RunningState)	break; // \todo why are we hitting this ?
		downloaded+= block.size;

		const auto &segments = file.bitmap.segments(Utility::Bitmap::getStatusChecker(Utility::Bitmap::RequestedStatus),
							Utility::Bitmap::Segment(block.offset, block.offset + block.size));
		foreach (const auto &segment, segments)
		{
			file.file.write(segment.begin, &block.data.constData()[segment.begin - block.offset], segment.size());
			updateBitmap(Utility::Bitmap::DownloadedStatus, segment);
		}
	}
		break;
	case Utility::BlockUploaded:
		if (getState() != RunningState && getState() != SharingState)	break;
		uploaded+= block.size;
		break;
	case Utility::BlockVerified:
	{
		if (getState() != RunningState)	break; // \todo why are we hitting this ?

		const Utility::Bitmap::Segment segment(block.offset, block.offset + block.size);
		updateBitmap(Utility::Bitmap::VerifiedStatus, segment);

		if (isCompleted())
		{
			foreach (AbstractTransfer *transfer, transfers)
				if (!transfer->capability().testFlag(AbstractTransfer::UploadCapability))
					delTransfer(transfer);

			setState(SharingState);
		}
	}
		break;
	case Utility::BlockCorrupted:
	{
		if (getState() != RunningState)	break; // \todo why are we hitting this ?

		// If the block has already marked as verified, we will not downloaded it again.
		// In other words, when verification conficts, we assume that the block is verified.
		// (This is EmptyStatusUpdater's behavior)

		const Utility::Bitmap::Segment segment(block.offset, block.offset + block.size);
		updateBitmap(Utility::Bitmap::EmptyStatus, segment);
	}
		break;
	default:
		Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid type");
		break;
	};
}

QByteArray FileTask::readBlock(const Utility::Block &block)
{
	return file.file.read(block.offset, block.size);
}

/*!
 * \brief Starts the task
 */
void FileTask::start()
{
	prepareFile();

	if (file.size == 0) // for zero-size task, only create the file
	{
		Q_ASSERT_X(transfers.isEmpty(), Q_FUNC_INFO, "No transfer is needed");

		setState(CompletedState);
		return;
	}

	switch (getState())
	{
	case CompletedState:
		setState(SharingState);
		break;
	case StoppedState:
		setState(RunningState);
		break;
	case PausedState:
		isCompleted() ? setState(SharingState) : setState(RunningState);
		break;
	default:
		return;
	}

	timer.start();

	foreach(AbstractTransfer *transfer, transfers)
		transfer->start(this);
}

/*!
 * \brief Pauses the task
 * In paused state, the file is not closed and all downloaders are paused.
 */
void FileTask::pause()
{
	switch (getState())
	{
	case RunningState:
	case SharingState:
		foreach(AbstractTransfer *transfer, transfers)
			transfer->pause(this);

		setState(PausedState);
		break;
	default:
		return;
	}
}

void FileTask::stop()
{
	switch (getState())
	{
	case RunningState:
	case SharingState:
	case PausedState:
		foreach(AbstractTransfer *transfer, transfers)
			transfer->stop(this);

		setState(StoppedState);
		break;
	default:
		return;
	}
}

void FileTask::stateUpdating(State state)
{
	// adjust file open mode
	switch (state)
	{
	case RunningState:
		file.file.changeOpenMode(Disk::File::ReadWrite);
		break;
	case SharingState:
		file.file.changeOpenMode(Disk::File::ReadOnly);
		break;
	case PausedState:
		// stay in old open mode
		break;
	case StoppedState:
	case CompletedState:
		file.file.close();
		break;
	default:
		break;
	}
}

void FileTask::updateBitmap(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment)
{
	file.bitmap.updateStatus(Utility::Bitmap::getStatusUpdater(status), segment);
	emit bitmapUpdated(status, segment);
}

bool FileTask::isCompleted() const
{
	const Utility::Bitmap::Segment segment(0, file.size);
	return file.bitmap.checkStatus(Utility::Bitmap::getStatusChecker(Utility::Bitmap::VerifiedStatus), segment);
}

bool FileTask::prepareFile()
{
	if (!file.file.create())
	{
		log(this) << QString::fromLatin1("Failed to create the file '%1'").arg(file.path() + file.name()) << endl;
		return false;
	}

	// resize the file
	if (file.size != -1 && file.file.size() != file.size)
		if (!file.file.resize(file.size))
		{
			log(this) << QString::fromLatin1("Failed to resize the file '%1' to '%2' : %3")
					.arg(file.path() + file.name())
					.arg(file.size).arg(file.file.errorString()) << endl;
			return false;
		}
	return true;
}

