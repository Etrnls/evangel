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
#include "performancemanager.h"
#include "file.h"

#include "log.h"

namespace Disk
{

void PerformanceManager::attach(File *file)
{
	files.insert(file);
	if (!performTimer)	performTimer = startTimer(performInterval);
}

void PerformanceManager::detach(File *file)
{
	Q_ASSERT_X(files.contains(file), Q_FUNC_INFO, "file is not attached yet");
	files.remove(file);

	if (files.isEmpty())
	{
		killTimer(performTimer);
		performTimer = 0;
	}
}

PerformanceManager::PerformanceManager() : performTimer(0)
{
	log(this) << "Initializing" << endl;

	log(this) << "Successfully initialized" << endl;
}

PerformanceManager::~PerformanceManager()
{
	log(this) << "Exiting" << endl;

	Q_ASSERT_X(files.isEmpty(), Q_FUNC_INFO, "files not empty");

	log(this) << "Successfully exited" << endl;
}

void PerformanceManager::perform() const
{
	foreach(File *file, files)	file->writeToDisk();
}


}
