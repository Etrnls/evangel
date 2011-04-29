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
#include "ratestatistics.h"

#include <QTimerEvent>

namespace Network
{

void RateStatistics::start()
{
	stop();

	timer = startTimer(dataRotationInterval);
}

void RateStatistics::stop()
{
	if (timer)	killTimer(timer);
	timer = 0;
	clear();
}

/*!
 * \brief The timer event handler
 *
 * Performs the data rotation.
 */
void RateStatistics::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == timer)
	{
		if (++index == dataSize)	index = 0;
		dataSum -= data[index];
		data[index] = 0;

		totalTime += dataRotationInterval / 1000;
	}
	else
		QObject::timerEvent(event);
}

}
