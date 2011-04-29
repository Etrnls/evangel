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
#ifndef USERINTERFACE_TELNETSOCKET_H
#define USERINTERFACE_TELNETSOCKET_H

#include "network/tcpsocket.h"
#include "console.h"

namespace UserInterface
{

/*!
 * \brief Provides a telnet UI socket
 * \author Etrnls <Etrnls@gmail.com>
 */
class TelnetSocket : public Network::TCPSocket
{
	Q_OBJECT
public:
	explicit TelnetSocket(int socketDescriptor);
private:
	enum Color
	{
		Black	= 0,
		Red	= 1,
		Green	= 2,
		Yellow	= 3,
		Blue	= 4,
		Magenta	= 5,
		Cyan	= 6,
		White	= 7,
		Reset	= 9
	};

	enum SGR
	{
		ForegroundColorNormalIntensity	= 30,
		BackgroundColorNormalIntensity	= 40,

		ForegroundColorHighIntensity	= 90,
		BackgroundColorHighIntensity	= 100
	};

	static const char ESC = 0x1B;

	// see RFC874
	static const uchar LF = 10; //!< Line Feed
	static const uchar CR = 13; //!< Carriage Return
	static const uchar SE   = 240;
	static const uchar EL   = 248; //!< Erase Line
	static const uchar SB   = 250;
	static const uchar IAC  = 255; //!< Interpret As Command
	static const uchar WILL = 251;
	static const uchar WONT = 252;
	static const uchar DO   = 253;
	static const uchar DONT = 254;

	static const char newLine[];
	static const char subNegotiationBegin[];
	static const char subNegotiationEnd[];

	virtual void processIncomingData();
	void processIAC();

	void execute(const QString &command);

	void show(const QString &content, bool showPrompt = true);
	void prompt();

	static Console::Command parseCommand(const QString &command, QString &arguments);

	static QString color(const QString &string, Color foreColor = Reset, Color backColor = Reset, bool highlight = false);


	static QString helpMessage(bool brief);
	static QString quitMessage();
};

}


#endif
