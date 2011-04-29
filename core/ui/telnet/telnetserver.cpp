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
#include "telnetserver.h"
#include "telnetsocket.h"

#include "log.h"
#include "util/settings.h"

namespace UserInterface
{

void TelnetServer::initialize()
{
	const quint16 port = Utility::Settings::load(this, serverName() + QLatin1String("Port"), defaultPort()).value<quint16>();

	if (listen(QHostAddress::Any, port))
		log(this) << metaObject()->className() << "listen on port" << port << endl;
	else
		log(this) << QString::fromLatin1("%1 failed to listen on port %2")
			.arg(QLatin1String(metaObject()->className())).arg(port) << endl;
}

void TelnetServer::incomingConnection(int socketDescriptor)
{
	TelnetSocket *socket = new TelnetSocket(socketDescriptor);

	log(this) << "Incoming" << serverName() << "connection from" << socket->getPeerAddressPort() << endl;
}

}
