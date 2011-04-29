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
#ifndef USERINTERFACE_TELNETSERVER_H
#define USERINTERFACE_TELNETSERVER_H

#include "ui/uiserver.h"

#include "network/tcpserver.h"

namespace UserInterface
{

/*!
 * \brief Provides the telnet UI server
 * \author Etrnls <Etrnls@gmail.com>
 */
class TelnetServer : public Network::TCPServer, public UIServer
{
	Q_OBJECT
	Q_CLASSINFO("log", "UserInterface")
public:
	virtual void initialize();
private:
	virtual QString serverName() const
	{
		return QLatin1String("telnet");
	}

	virtual quint16 defaultPort() const
	{
		return 5000;
	}

	virtual void incomingConnection(int socketDescriptor);
};

}

#endif
