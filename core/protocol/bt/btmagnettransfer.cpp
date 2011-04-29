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
#include "btmagnettransfer.h"

#include "tracker/trackerclient.h"

#include <QStringList>

namespace Protocol
{

namespace BitTorrent
{

BTMagnetTransfer::BTMagnetTransfer(const QByteArray &infoHash, const QString &name, const QStringList &trackers)
{
	foreach (const QString &URL, trackers)
		trackerClients.append(TrackerClient::createTrackerClient(QUrl(URL), infoHash));
}

BTMagnetTransfer::~BTMagnetTransfer()
{
}

BTMagnetTransfer::Capability BTMagnetTransfer::capability() const
{
	return 0;
}

QList<BTMagnetTransfer::FileInformation> BTMagnetTransfer::files() const
{
	return QList<FileInformation>();
}

Utility::Bitmap BTMagnetTransfer::bitmap() const
{
	return Utility::Bitmap();
}

void BTMagnetTransfer::updateBlock(Utility::BlockUpdateType type, Utility::Block block)
{

}

QByteArray BTMagnetTransfer::readBlock(Utility::Block block) const
{
	return QByteArray();
}

void BTMagnetTransfer::start(FileTask *task)
{

}

void BTMagnetTransfer::pause(FileTask *task)
{

}

void BTMagnetTransfer::stop(FileTask *task)
{

}

void BTMagnetTransfer::referenceStateChanged()
{

}

void BTMagnetTransfer::storeReference(FileTask *task, qint64 offset)
{

}

void BTMagnetTransfer::removeReference(FileTask *task)
{

}

qint64 BTMagnetTransfer::referenceOffset(FileTask *task) const
{
	return 0;
}

int BTMagnetTransfer::referenceCount() const
{
	return 0;
}

void BTMagnetTransfer::bitmapUpdating(Utility::Bitmap::SegmentStatus status, const Utility::Bitmap::Segment &segment)
{
	Q_UNUSED(status);
	Q_UNUSED(segment);
}

}

}
