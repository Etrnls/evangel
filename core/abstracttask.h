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
#ifndef ABSTRACTTASK_H
#define ABSTRACTTASK_H

#include <QObject>
#include <QElapsedTimer>

/*!
 * \brief The abstract task interface
 *
 * \author Etrnls <Etrnls@gmail.com>
 */
class AbstractTask : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("log", "AbstractTask")
public:
	//! \brief Represents the state of the task
	enum State
	{
		RunningState,	//!< Downloading & (maybe) uploading
		SharingState,	//!< Uploading only
		PausedState,	//!< Paused (a state which can switch to Running or Sharing fast)
		StoppedState,	//!< Stopped (not completed yet)
		CompletedState,	//!< Completed and stopped
	};
	Q_ENUMS(State)

	AbstractTask() : downloaded(0), uploaded(0), state(StoppedState) { }
	virtual ~AbstractTask() { }

	virtual QString getName() const = 0;
	virtual qint64 getSize() const = 0;
	virtual qreal getProgress() const = 0;

	QString getStateString() const;
	inline State getState() const
	{
		return state;
	}

	inline qint64 getDownloaded() const
	{
		return downloaded;
	}

	inline qint64 getUploaded() const
	{
		return uploaded;
	}

	inline int getTransferTime() const
	{
		return state == RunningState || state == SharingState ? timer.elapsed() : 0;
	}
public slots:
	virtual void start() = 0;
	virtual void pause() = 0;
	virtual void stop() = 0;
signals:
	void stateChanged() const;
protected:
	virtual void stateUpdating(State state) = 0;

	void setState(State state);

	qint64 downloaded; //!< Number of bytes downloaded
	qint64 uploaded; //!< Number of bytes uploaded

	QElapsedTimer timer;
private:
	State state;
};

#endif

