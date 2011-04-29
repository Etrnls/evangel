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
#include "file.h"
#include "performancemanager.h"

#include <QFileInfo>
#include <QDir>

namespace Disk
{

File::File(const QString &name) : QFile(name)
{
	PerformanceManager::getInstance()->attach(this);
}

File::~File()
{
	PerformanceManager::getInstance()->detach(this);
}

/*!
 * \brief Writes the data from the buffer to the disk
 * \param maxSize The max size to write.
 * \return The bytes written.
 */
int File::writeToDisk(int maxSize)
{
	int result = 0;

	while (!pendingWriteRequests.isEmpty())
	{
		if (result + pendingWriteRequests.first().data.size() > maxSize) break;

		const WriteRequest &writeRequest = pendingWriteRequests.takeFirst();
		pendingWriteRequestsSize -= writeRequest.data.size();

		const qint64 originPos = pos();
		seek(writeRequest.offset);
		QFile::writeData(writeRequest.data.data(), writeRequest.data.size());
		seek(originPos);

		result += writeRequest.data.size();
	}

	return result;
}

qint64 File::bytesToWrite() const
{
	return pendingWriteRequestsSize + QFile::bytesToWrite();
}

bool File::changeOpenMode(OpenMode mode)
{
	if (openMode() == mode)	return true;

	close();
	return open(mode);
}

bool File::create()
{
	if (exists())	return true;

	// creates the directory path
	{
		const QString &path = QFileInfo(*this).path();
		if (!QDir().exists(path) && !QDir().mkpath(path))
			return false;
	}

	// creates the file
	{
		if (!open(WriteOnly))	return false;
		close();
	}

	return true;
}

bool File::open(OpenMode mode)
{
	if (isOpen())
		return changeOpenMode(mode | openMode());
	else
		return QFile::open(mode);
}

void File::close()
{
	writeToDisk();
	QFile::close();
}

QByteArray File::read(qint64 offset, qint64 size)
{
	if (!seek(offset))	return QByteArray();
	return QFile::read(size);
}

qint64 File::write(qint64 offset, const QByteArray &data)
{
	if (!seek(offset))	return 0;
	return QFile::write(data);
}

qint64 File::write(qint64 offset, const char *data, qint64 maxSize)
{
	if (!seek(offset))	return 0;
	return QFile::write(data, maxSize);
}

qint64 File::readData(char *data, qint64 maxSize)
{
	writeToDisk();
	return QFile::readData(data, maxSize);
}

/*!
 * \brief Writes the data to the buffer
 * \param data The data to write.
 * \param maxSize The max size to write.
 */
qint64 File::writeData(const char *data, qint64 maxSize)
{
	pendingWriteRequestsSize += maxSize;
	pendingWriteRequests.append(WriteRequest(pos(), QByteArray(data, maxSize)));

	return maxSize;
}


}
