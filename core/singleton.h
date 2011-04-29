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
#ifndef SINGLETON_H
#define SINGLETON_H

#include "util/exithandler.h"

#include <QtGlobal>

/*!
 * \brief The Singleton
 * \author Etrnls <Etrnls@gmail.com>
 */
template <class T>
class Singleton
{
public:
	inline static T *getInstance()
	{
		Q_ASSERT_X(instance, Q_FUNC_INFO, "Not created yet");
		return instance;
	}

	inline static void create()
	{
		Q_ASSERT_X(!instance, Q_FUNC_INFO, "Already created");
		instance = new T();
		Utility::ExitHandler::atExit(destroy);
	}

	inline static void destroy()
	{
		Q_ASSERT_X(instance, Q_FUNC_INFO, "Not created yet");
		delete instance;
		instance = 0;
	}
protected:
	inline Singleton() {}
	virtual ~Singleton() {}
private:
	Singleton(const Singleton &);
	Singleton& operator=(const Singleton &);

	static T *instance;
};

template <class T>
T *Singleton<T>::instance = 0;

#endif
