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
#ifndef ABSTRACTTRANSFER_H
#define ABSTRACTTRANSFER_H

#include "filetask.h"
#include "util/bitmap/bitmap.h"

#include <QObject>

/*!
 * \brief The abstract transfer interface
 *
 * The transfer acts as a facade between the protocol implementation and
 * the FileTask.
 *
 * The transfer is used by FileTask for data transfer.
 * A reference is a FileTask that uses the transfer.
 *
 * The transfer provide the same updateBlock() & readBlock() interface and
 * bitmapUpdated() signal as the FileTask with the transformed offset.
 *
 * \author Etrnls <Etrnls@gmail.com>
 */
class AbstractTransfer : public QObject
{
	Q_OBJECT
public:
	enum CapabilityFlag
	{
		DownloadCapability	= 0x01,
		UploadCapability	= 0x02,
		VerifyCapability	= 0x04
	};
	Q_DECLARE_FLAGS(Capability, CapabilityFlag)

	//! \brief Stores the information of the file in the transfer
	struct FileInformation
	{
		QString	file;	//!< The relative file path
		qint64	offset;	//!< The offset of the file within the transfer
		qint64	size;	//!< The size of the file
		inline FileInformation(const QString &file, qint64 offset, qint64 size) : file(file), offset(offset), size(size) {}
	};

	// information
	virtual Capability capability() const = 0;
	virtual QList<FileInformation> files() const = 0;
	virtual Utility::Bitmap bitmap() const = 0;

	// block
	virtual void updateBlock(Utility::BlockUpdateType type, Utility::Block block) = 0;
	virtual QByteArray readBlock(Utility::Block block) const = 0;

	/*!
	 * \brief Adds a reference to the transfer
	 * \param task The reference to add.
	 * \param offset The offset of the reference within the transfer.
	 *
	 * The reference is stored by storeReference.
	 *
	 * \see delReference
	 */
	void addReference(FileTask *task, qint64 offset)
	{
		connect(task, SIGNAL(bitmapUpdated(Utility::Bitmap::SegmentStatus, Utility::Bitmap::Segment)),
			this, SLOT(referenceBitmapUpdated(Utility::Bitmap::SegmentStatus, Utility::Bitmap::Segment)));
		storeReference(task, offset);
	}

	/*!
	 * \brief Deletes a reference in the transfer
	 * \param task The reference to delete.
	 *
	 * The reference is removed by removeReference.
	 * The transfer is deleted when it is not referenced by any FileTask.
	 *
	 * \see addReference
	 */
	void delReference(FileTask *task)
	{
		disconnect(task, 0, this, 0);
		removeReference(task);
		if (referenceCount() == 0)	delete this;
	}
public slots:
	virtual void start(FileTask *task) = 0;
	virtual void pause(FileTask *task) = 0;
	virtual void stop(FileTask *task) = 0;
signals:
	void bitmapUpdated(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment) const;
protected:
	/*!
	 * \brief Stores a reference
	 * \param task The reference to store.
	 * \param offset The offset of the reference within the transfer.
	 *
	 * Subclass should implements this function to store the reference.
	 *
	 * \see removeReference addReference
	 */
	virtual void storeReference(FileTask *task, qint64 offset) = 0;

	/*!
	 * \brief Removes a reference
	 * \param task The reference to remove.
	 *
	 * Subclass should implements this function to remove the reference.
	 *
	 * \see storeReference delReference
	 */
	virtual void removeReference(FileTask *task) = 0;

	virtual qint64 referenceOffset(FileTask *task) const = 0;
	virtual int referenceCount() const = 0;

	/*!
	 * This is called when the bitmap is updated in reference(s)
	 * but not updated inside the protocol implementation
	 */
	virtual void bitmapUpdating(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment) = 0;
private slots:
	void referenceBitmapUpdated(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment)
	{
		bitmapUpdating(status, segment);
		const qint64 offset = referenceOffset(qobject_cast<FileTask*>(sender()));
		emit bitmapUpdated(status, Utility::Bitmap::Segment(offset + segment.begin, offset + segment.end));
	}
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractTransfer::Capability)

#endif
