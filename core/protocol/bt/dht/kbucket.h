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
#ifndef PROTOCOL_BITTORRENT_KBUCKET_H
#define PROTOCOL_BITTORRENT_KBUCKET_H

#include "node.h"

#include <QList>
#include <QTime>

namespace Protocol
{

namespace BitTorrent
{


/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class KBucket
{
public:
	KBucket()
	{
		lastModifiedTimestamp.start();
	}

	~KBucket() {}

	void insert(const Node &node);
private:
	static const int k = 8;

	bool replaceBadNode(const Node &newNode);

	void pingQuestionableNode(const Node &newNode);

	QList<Node> nodes;

	QTime lastModifiedTimestamp;
};

}

}

#endif
