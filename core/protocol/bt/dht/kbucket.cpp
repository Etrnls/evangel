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
#include "kbucket.h"

namespace Protocol
{

namespace BitTorrent
{

void KBucket::insert(const Node &node)
{
	QList<Node>::Iterator i = qFind(nodes.begin(), nodes.end(), node);

	// already in the bucket
	if (i != nodes.end())
	{
		i->responded();

		// move to the end
		nodes.erase(i);
		nodes.append(node);

		lastModifiedTimestamp.restart();

		return;
	}

	if (nodes.size() < k)
	{
		nodes.append(node);

		lastModifiedTimestamp.restart();

		return;
	}

	if (!replaceBadNode(node))	pingQuestionableNode(node);
}

bool KBucket::replaceBadNode(const Node &newNode)
{
	for (QList<Node>::Iterator iter = nodes.begin(); iter != nodes.end(); ++iter)
		if (iter->getState() == Node::BadState)
		{
			nodes.erase(iter);
			nodes.append(newNode);

			lastModifiedTimestamp.restart();

			return true;
		}

	return false;
}

void KBucket::pingQuestionableNode(const Node &newNode)
{

}

}

}
