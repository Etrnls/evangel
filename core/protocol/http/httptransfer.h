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
#ifndef PROTOCOL_HTTP_HTTPTRANSFER_H
#define PROTOCOL_HTTP_HTTPTRANSFER_H

#include "abstracttransfer.h"

#include <QUrl>

namespace Protocol
{

namespace HTTP
{

class HTTPClient;

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class HTTPTransfer : public AbstractTransfer
{
public:
	explicit HTTPTransfer(const QString &URL) : URL(URL), size(-1), reference(0), client(0) {}

	inline const QUrl &getURL() const
	{
		return URL;
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
private:
	// ----- Abstract Interfaces -----
	virtual void storeReference(FileTask *task, qint64 offset);
	virtual void removeReference(FileTask *task);
	virtual qint64 referenceOffset(FileTask *task) const;
	virtual int referenceCount() const;

	virtual void bitmapUpdating(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment);
	// -------------------------------

	const QUrl URL;
	qint64 size;

	FileTask *reference;
	HTTPClient *client;
};

}

}

#endif
