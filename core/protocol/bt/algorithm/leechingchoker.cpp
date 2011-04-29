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
#include "leechingchoker.h"

#include "protocol/bt/peer/peermanager.h"
#include "protocol/bt/peer/peer.h"

#include "util/numeric/random.h"

namespace Protocol
{

namespace BitTorrent
{

void LeechingChoker::choke(const PeerManager *peerManager)
{
	QList<Peer*> connectedPeers = peerManager->getPeers(Peer::ConnectedState);
	qSort(connectedPeers.begin(), connectedPeers.end(),
		[](const Peer *a, const Peer *b)
		{
			return a->getDownloadRate() > b->getDownloadRate();
		});

	// unchoke
	{
		int count = 0;
		foreach (Peer *peer, connectedPeers)
			if (peer->isInterested())
			{
				peer->unchoke();
				connectedPeers.removeOne(peer);
				if (++count >= maxUploads)	break;
			}
	}

	// optimistic unchoke
	if (++optimisticUnchokeCounter == optimisticUnchokeRound)
	{
		optimisticUnchokeCounter = 0;
		while (!connectedPeers.isEmpty())
		{
			Peer *peer = connectedPeers.takeAt(Utility::rand(connectedPeers.size()));
			if (peer->isInterested())
			{
				peer->unchoke();
				break;
			}
			else
				peer->choke();
		}
	}

	// choke the rest peers
	foreach (Peer *peer, connectedPeers)
		peer->choke();
}


}

}
