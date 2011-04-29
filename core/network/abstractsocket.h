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
#ifndef NETWORK_ABSTRACTSOCKET_H
#define NETWORK_ABSTRACTSOCKET_H

#include <QObject>
#include <climits>

/*!
 * \brief Provides low-level network classes
 */
namespace Network
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 */
class AbstractSocket : public QObject
{
	Q_OBJECT
public:
	//! Indicates how the traffic manager should schedule the transfer of the socket.
	enum Priority
	{
		PrivilegePriority	= -1,
		HighestPriority		= 0,
		HighPriority		= 1,
		NormalPriority		= 2,
		LowPriority		= 3,
		LowestPriority		= 4,
		InvalidPriority		= 5
	};
	static const int priorityCount = InvalidPriority;

	enum State
	{
		UnconnectedState,
		ConnectingState,
		ConnectedState
	};

	explicit AbstractSocket(Priority priority) : priority(priority) {}
	virtual ~AbstractSocket() {}

	virtual State getState() const = 0;

	Priority getPriority() const		{	return priority;		}
	void setPriority(Priority priority)	{	this->priority = priority;	}

	virtual void setNetworkReadBufferSize(int size) = 0;

	virtual bool canDownload() const = 0;
	virtual bool canUpload() const = 0;

	virtual int download(int maxSize = INT_MAX) = 0;
	virtual int upload(int maxSize = INT_MAX) = 0;
signals:
	void readyDownload() const;
	void readyUpload() const;
protected:
	void attachTrafficControl();
	void detachTrafficControl();

	virtual void processIncomingData() = 0;
private:
	Q_INVOKABLE void autoProcessIncomingData();

	Priority priority; //!< The priority
};

}

#endif
