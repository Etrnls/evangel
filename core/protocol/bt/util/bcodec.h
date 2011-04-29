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
#ifndef PROTOCOL_BITTORRENT_BCODEC_H
#define PROTOCOL_BITTORRENT_BCODEC_H

#include <QVariant>

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \brief Encodes and decodes the b-encoded data
 * \author Etrnls <Etrnls@gmail.com>
 */
class Bcodec
{
public:
	typedef qint64		BInteger;
	typedef QByteArray	BString;
	typedef QVariantList	BList;
	typedef QMap<BString, QVariant>	BDictionary; // Use QMap to keep it sorted

	explicit Bcodec(const QByteArray &content)
	{
		valid = decode(content);
	}

	static QByteArray encode(const BInteger &integer);
	static QByteArray encode(const BString &string);
	static QByteArray encode(const BList &list);
	static QByteArray encode(const BDictionary &dictionary);

	inline bool isValid() const
	{
		return valid;
	}

	//! \brief Retrieves the decoded result of the content
	inline BDictionary getResult() const
	{
		return dictionary;
	}

	//! \brief Retrieves the value of the info key of the contenct
	inline QByteArray getInfoValue() const
	{
		return content.mid(infoStart, infoSize);
	}

	//! \brief Retrieves the description of last error occured
	inline QString getError() const
	{
		return error;
	}
private:
	bool valid;

	QString error; //!< The description of last error occured

	int infoStart; //!< The start position of the value of the info key
	int infoSize;  //!< The size of the value of the info key

	QByteArray  content;    //!< The content to be decoded
	int         index;      //!< The position currently decoding
	BDictionary dictionary; //!< The decoded result

	bool decode(const QByteArray &content);

	bool decode(BInteger *integer);
	bool decode(BString *string);
	bool decode(BList *list);
	bool decode(BDictionary *dictionary);

	static QByteArray encode(const QVariant &variant);
};

}

}

Q_DECLARE_METATYPE(Protocol::BitTorrent::Bcodec::BInteger)
Q_DECLARE_METATYPE(Protocol::BitTorrent::Bcodec::BString)
Q_DECLARE_METATYPE(Protocol::BitTorrent::Bcodec::BList)
Q_DECLARE_METATYPE(Protocol::BitTorrent::Bcodec::BDictionary)

#endif
