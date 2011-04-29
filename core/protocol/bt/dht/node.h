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
#ifndef PROTOCOL_BITTORRENT_NODE_H
#define PROTOCOL_BITTORRENT_NODE_H

#include "util/numeric/uint160.h"

#include <QHostAddress>
#include <QTime>

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class Node
{
public:
	enum State
	{
		GoodState,
		QuestionableState,
		BadState
	};

	Node(const QHostAddress &address, quint16 port, const Utility::UInt160 &ID)
	: address(address), port(port), ID(ID), failedQueries(0), questionablePings(0)
	{
		lastResponseTimestamp.start();
	}

	Node(const Node &other)
	: address(other.address), port(other.port), ID(other.ID), failedQueries(other.failedQueries), questionablePings(other.questionablePings)
	{
		lastResponseTimestamp.start();
	}

	~Node() {}

	const QHostAddress& getAddress() const
	{
		return address;
	}

	quint16 getPort() const
	{
		return port;
	}

	const Utility::UInt160& getID() const
	{
		return ID;
	}

	void responded();

	State getState() const;
private:
	static const int questionableTime = 15 * 60 * 1000;
	static const int maxFailedQueries = 2;
	static const int maxQuestionablePings = 2;

	QHostAddress address;
	quint16 port;

	Utility::UInt160 ID;

	QTime lastResponseTimestamp;

	int failedQueries;
	int questionablePings;

	friend bool operator==(const Node &lhs, const Node &rhs);
};

inline bool operator==(const Node &lhs, const Node &rhs)
{
	return lhs.address == rhs.address && lhs.port == rhs.port && lhs.ID == rhs.ID;
}

}

}

#endif
