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
#ifndef PROTOCOL_BITTORRENT_BTMAGNETSOURCE_H
#define PROTOCOL_BITTORRENT_BTMAGNETSOURCE_H

#include "abstractsource.h"

#include <QByteArray>
#include <QStringList>

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class BTMagnetSource : public AbstractSource
{
public:
	BTMagnetSource(const QByteArray &infoHash, const QString &name, const QStringList &trackers);

	virtual AbstractTransfer *createTransfer() const;
private:
	QByteArray infoHash;
	QString name;
	QStringList trackers;
};


}

}

#endif
