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
#include "jobmanager.h"
#include "job.h"

#include <QElapsedTimer>
#include <QTimer>
#include <QCoreApplication>

namespace Future
{

void JobManager::attach(Job *job)
{
	Q_ASSERT_X(!jobs.contains(job), Q_FUNC_INFO, "Already attached");
	jobs.insert(job);
	connect(job, SIGNAL(finished()), SLOT(jobFinished()));
}

void JobManager::detach(Job *job)
{
	Q_ASSERT_X(jobs.contains(job), Q_FUNC_INFO, "Not attached yet");
	jobs.remove(job);

	// Safely delete the job
	// signal from job-->JobManager::jobFinished-->here
	job->deleteLater();
}

void JobManager::jobFinished()
{
	detach(qobject_cast<Job*>(sender()));
}

JobManager::JobManager()
{
}

JobManager::~JobManager()
{
	if (!jobs.isEmpty())
	{
		QElapsedTimer stopwatch;
		stopwatch.start();

		QTimer::singleShot(timeout, this, SLOT(timedout()));

		// this will finish even we use WaitForMoreEvents because we have a dummy single shot timer event
		while (!jobs.isEmpty() && !stopwatch.hasExpired(timeout))
			QCoreApplication::processEvents(QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents);

		// process job->deleteLater()
		QCoreApplication::processEvents(QEventLoop::AllEvents);

		// delete the not finished jobs
		qDeleteAll(jobs);
	}
}


};


