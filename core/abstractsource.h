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
#ifndef ABSTRACTSOURCE_H
#define ABSTRACTSOURCE_H

#include <QList>

class QString;

class AbstractTransfer;

/*!
 * \brief The abstract source interface
 * \author Etrnls <Etrnls@gmail.com>
 */
class AbstractSource
{
public:
	inline AbstractSource() {}
	virtual ~AbstractSource() {}

	//! \brief Creates the corresponding AbstractTransfer
	virtual AbstractTransfer *createTransfer() const = 0;
private:
	AbstractSource(const AbstractSource &);
	AbstractSource& operator=(const AbstractSource &);
};

/*!
 * \brief The abstract source factory interface
 * \author Etrnls <Etrnls@gmail.com>
 */
class AbstractSourceFactory
{
public:
	static AbstractSource *createSource(const QString &URL);
protected:
	/*!
	 * \brief Constructs the AbstractSourceFactory and adds it to the internal list
	 * \see factories
	 */
	inline AbstractSourceFactory()
	{
		factories.append(this);
	}

	/*!
	 * \brief Destroys the AbstractSourceFactory and removes it from the internal list
	 * \see factories
	 */
	virtual ~AbstractSourceFactory()
	{
		factories.removeOne(this);
	}

	//! Creates the corresponding AbstractSource
	virtual AbstractSource *create(QString URL) const = 0;
private:
	AbstractSourceFactory(const AbstractSourceFactory &);
	AbstractSourceFactory& operator=(const AbstractSourceFactory &);

	//! Stores the instances of all AbstractSourceFactory
	static QList<AbstractSourceFactory*> factories;
};

#endif

