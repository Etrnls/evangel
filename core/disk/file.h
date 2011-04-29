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
#ifndef DISK_FILE_H
#define DISK_FILE_H

#include <QFile>
#include <climits>

namespace Disk
{

/*!
 * \brief Provides an interface for reading from and writing to files with disk buffer & cache
 * \author Etrnls <Etrnls@gmail.com>
 */
class File : public QFile
{
	Q_OBJECT
public:
	explicit File(const QString &name);
	virtual ~File();

	int writeToDisk(int maxSize = INT_MAX);

	virtual qint64 bytesToWrite() const;

	bool changeOpenMode(OpenMode mode);

	bool create();

	virtual bool open(OpenMode mode);
	virtual void close();

	QByteArray read(qint64 offset, qint64 size);
	qint64 write(qint64 offset, const QByteArray &data);
	qint64 write(qint64 offset, const char *data, qint64 maxSize);
private:
	//! \brief Stores the write request
	struct WriteRequest
	{
		qint64 offset;
		QByteArray data;
		inline WriteRequest(qint64 offset, const QByteArray &data) : offset(offset), data(data) {}
	};

	int pendingWriteRequestsSize;
	QList<WriteRequest> pendingWriteRequests;

	virtual qint64 readData(char *data, qint64 maxSize);
	virtual qint64 writeData(const char *data, qint64 maxSize);
};

}

#endif
