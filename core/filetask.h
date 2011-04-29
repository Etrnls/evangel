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
#ifndef FILETASK_H
#define FILETASK_H

#include "abstracttask.h"
#include "disk/file.h"
#include "util/block.h"
#include "util/bitmap/bitmap.h"

#include <QSet>
#include <QDateTime>
#include <QFileInfo>

class AbstractTransfer;

/*!
 * \brief Represents a file transfer task
 *
 * A FileTask uses one or more transfers for a single file transfer.
 * Transfer could use readBlock() to get data for uploading or verification and
 * updateBlock() to update a block of the FileTask.
 * Any bitmap updates is informed by bitmapUpdated().
 *
 * A basic workflow is to call updateBlock() when a certain amount of data
 * has been downloaded and listen to bitmapUpdated() then verify the data when
 * appropriate. Call updateBlock() to report the verification result and listen
 * to bitmapUpdated() to see if any block needs redownload or can be uploaded.
 *
 * \author Etrnls <Etrnls@gmail.com>
 */
class FileTask : public AbstractTask
{
	Q_OBJECT
	Q_CLASSINFO("log", "FileTask")
public:
	FileTask(const QString &file, qint64 size);
	virtual ~FileTask();

	virtual QString getName() const
	{
		return file.name();
	}

	virtual qint64 getSize() const
	{
		return file.size;
	}

	virtual qreal getProgress() const
	{
		return static_cast<qreal>(bitmap().count(Utility::Bitmap::getStatusChecker(Utility::Bitmap::VerifiedStatus))) / getSize();
	}

	inline const Utility::Bitmap &bitmap() const
	{
		return file.bitmap;
	}

	void addTransfer(AbstractTransfer *transfer, qint64 offset);
	void delTransfer(AbstractTransfer *transfer);

	void updateBlock(Utility::BlockUpdateType type, const Utility::Block &block);
	QByteArray readBlock(const Utility::Block &block);
public slots:
	virtual void start();
	virtual void pause();
	virtual void stop();
signals:
	void bitmapUpdated(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment) const;
private:
	struct File
	{
		Disk::File file;
		qint64 size;
		Utility::Bitmap bitmap;

		File(const QString &file, qint64 size) : file(file), size(size)
		{
			if (size != -1)	bitmap.resize(size);
		}

		QString path() const
		{
			return QFileInfo(file).path();
		}

		QString name() const
		{
			return QFileInfo(file).fileName();
		}
	};

	virtual void stateUpdating(State state);

	void updateBitmap(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment);

	bool isCompleted() const;

	bool prepareFile();

	File file;

	QSet<AbstractTransfer*> transfers;
};

#endif
