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
#ifndef PROTOCOL_BITTORRENT_PEERID_H
#define PROTOCOL_BITTORRENT_PEERID_H

#include <QCoreApplication>
#include <QString>
#include <QHash>

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class PeerID
{
	Q_DECLARE_TR_FUNCTIONS(PeerID)
public:
	explicit PeerID(const QByteArray &ID = QByteArray()) : ID(ID) {}

	void generate();

	QByteArray getID() const
	{
		return ID;
	}

	void setID(const QByteArray &ID)
	{
		this->ID = ID;
		clientName.clear();
	}

	QString getClientName() const;
	void setClientName(const QString &name);
private:
	static QHash<QString, QString> clientDictionary; //!< The dictionary used to parse the client name

	QByteArray ID; //!< The peer ID
	mutable QString clientName; //!< The name of the client

	static void initializeClientDictionary();
	static QString lookupClientDictionary(const QString &clientID);

	static QString AzureusStyleVersion(const QString &clientID, const QString &ID);
	static QString ShadowStyleVersion(const QString &clientID, const QString &ID);
	static int ShadowStyleBase64(QChar c);

	static QString identifyClient(QString ID);
};

}

}

#endif
