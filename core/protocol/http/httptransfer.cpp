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
#include "httptransfer.h"
#include "httpclient.h"

#include <QFileInfo>

namespace Protocol
{

namespace HTTP
{

HTTPTransfer::Capability HTTPTransfer::capability() const
{
	return DownloadCapability;
}

QList<HTTPTransfer::FileInformation> HTTPTransfer::files() const
{
	QList<FileInformation> result;
	result.append(FileInformation(QFileInfo(URL.path()).fileName(), 0, size));
	return result;
}

Utility::Bitmap HTTPTransfer::bitmap() const
{
	return reference->bitmap();
}

void HTTPTransfer::updateBlock(Utility::BlockUpdateType type, Utility::Block block)
{
	Q_ASSERT_X(reference, Q_FUNC_INFO, "Invalid reference");
	reference->updateBlock(type, block);
}

QByteArray HTTPTransfer::readBlock(Utility::Block block) const
{
	Q_ASSERT_X(reference, Q_FUNC_INFO, "Invalid reference");
	return reference->readBlock(block);
}

void HTTPTransfer::start(FileTask *task)
{
	Q_ASSERT_X(reference == task, Q_FUNC_INFO, "Invalid task");

	if (!client)	client = new HTTPClient(this);
}

void HTTPTransfer::pause(FileTask *task)
{
	Q_ASSERT_X(reference == task, Q_FUNC_INFO, "Invalid task");

	stop(task);
}

void HTTPTransfer::stop(FileTask *task)
{
	Q_ASSERT_X(reference == task, Q_FUNC_INFO, "Invalid task");

	delete client;
	client = 0;
}

void HTTPTransfer::storeReference(FileTask *task, qint64 offset)
{
	Q_ASSERT_X(!reference, Q_FUNC_INFO, "Already has a reference");
	Q_ASSERT_X(offset == 0, Q_FUNC_INFO, "Invalid offset");
	reference = task;
}

void HTTPTransfer::removeReference(FileTask *task)
{
	Q_ASSERT_X(reference == task, Q_FUNC_INFO, "Invalid task");
	reference = 0;
}

qint64 HTTPTransfer::referenceOffset(FileTask *task) const
{
	Q_ASSERT_X(reference == task, Q_FUNC_INFO, "Invalid task");
	return 0;
}

int HTTPTransfer::referenceCount() const
{
	return reference ? 1 : 0;
}

void HTTPTransfer::bitmapUpdating(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment)
{
	Q_UNUSED(status);
	Q_UNUSED(segment);
}

}

}
