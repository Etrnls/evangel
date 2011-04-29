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
#ifndef NETWORK_RATESTATISTICS_H
#define NETWORK_RATESTATISTICS_H

#include <QObject>

namespace Network
{

/*!
 * \brief Provides the rate statistic
 * \author Etrnls <Etrnls@gmail.com>
 */
class RateStatistics : public QObject
{
	Q_OBJECT
public slots:
	void start();
	void stop();
public:
	/*!
	 * \brief Constructs a RateStatistics object
	 * \param parent The parent of the RateStatistics.
	 */
	inline explicit RateStatistics(QObject *parent) : QObject(parent), timer(0)
	{
		clear();
	}

	inline void transfer(int size)
	{
		Q_ASSERT_X(timer, Q_FUNC_INFO, "Not started yet");
		data[index] += size;
		dataSum += size;
		totalSum += size;
	}

	//! \brief Retrieves the rate in bytes/second
	inline qreal getRate() const
	{
		return dataSum / ((dataRotationInterval / 1000.0) * dataSize);
	}

	inline qreal getAverageRate() const
	{
		return static_cast<qreal>(totalSum) / totalTime;
	}

	inline int getTotalSum() const
	{
		return totalSum;
	}

	inline int getTotalTime() const
	{
		return totalTime;
	}
private:
	void clear()
	{
		Q_ASSERT_X(timer == 0, Q_FUNC_INFO, "Not stopped yet");
		index = 0;
		qFill(data, data + dataSize, 0);
		dataSum = 0;

		totalTime = totalSum = 0;
	}

	virtual void timerEvent(QTimerEvent *event);

	//! The interval between data rotations
	static const int dataRotationInterval = 1000;

	//! The size of the data
	static const int dataSize = 16;

	int index;		//!< The index of the current slot
	int data[dataSize];	//!< The statistical data
	int dataSum;		//!< The sum of the data

	int totalTime;		//!< The total time
	int totalSum;		//!< The total sum

	int timer;		//!< The data rotation timer
};

}

#endif
