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
#ifndef FUTURE_JOBMANAGER_H
#define FUTURE_JOBMANAGER_H

#include "singleton.h"

#include <QSet>
#include <QObject>

namespace Future
{

class Job;

/*!
 * \brief The JobManager
 * \author Etrnls <Etrnls@gmail.com>
 */
class JobManager : public QObject, public Singleton<JobManager>
{
	Q_OBJECT
public:
	void attach(Job *job);
	void detach(Job *job);
private slots:
	void jobFinished();
	void timedout() const {}
private:
	static const int timeout = 5000;

	JobManager();
	virtual ~JobManager();

	QSet<Job*> jobs;

	friend class Singleton<JobManager>;
};

}

#endif