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
#ifndef PROTOCOL_BITTORRENT_METAINFO_H
#define PROTOCOL_BITTORRENT_METAINFO_H

#include "bcodec.h"

#include <QDateTime>
#include <QVariant>
#include <QStringList>
#include <QFileInfo>
#include <QPair>

namespace Protocol
{

namespace BitTorrent
{


/*!
 * \brief Handles the Metainfo file (.torrent)
 *
 * \author Etrnls <Etrnls@gmail.com>
 */
class MetaInfo
{
public:
 	//! \brief Stores the file information
	struct MetaFileInfo
	{
		qint64     size;   //!< The size of the file, in bytes
		QString    path;   //!< The path of the file
		QByteArray md5sum; //!< The md5sum of the file
	};

	static QByteArray generate(QList<QPair<QFileInfo, QDir>> files, QList<QUrl> announces, int pieceSize = -1);

	explicit MetaInfo(const QByteArray &content)
	{
		valid = parse(content);
	}

	inline bool isValid() const
	{
		return valid;
	}

	inline bool isPrivateTorrent() const
	{
		return privateTorrent;
	}

	//! \brief Retrieves the infoHash
	inline const QByteArray& getInfoHash() const
	{
		return infoHash;
	}

	//! \brief Retrieves the announce list
	inline const QStringList& getAnnounceList() const
	{
		return announceList;
	}

	//! \brief Retrieves the information of files
	inline const QList<MetaFileInfo>& getFiles() const
	{
		return files;
	}

	//! \brief Retrieves the SHA1 hash
	inline const QList<QByteArray>& getSHA1Hash() const
	{
		return SHA1Hash;
	}

	//! \brief Retrieves the total size
	inline qint64 getSize() const
	{
		return size;
	}

	//! \brief Retrieves the number of pieces
	inline int getPieces() const
	{
		return getSHA1Hash().size();
	}

	/*!
	 * \brief Retrieves the piece size
	 *
	 * For the purposes of transfer, files are split into fixed-size pieces
	 * which are all the same size except for possibly the last one which
	 * may be truncated.
	 *
	 * piece size is almost always a power of two, most commonly 2^18 = 256 K
	 */
	inline int getPieceSize(int index) const
	{
		if (index == getPieces() - 1)
		{
			const int x = getSize() % pieceSize;
			return x ? x : pieceSize;
		}
		else
			return pieceSize;
	}

	//! \brief Retrieves the index of the offset
	inline int getIndex(qint64 offset) const
	{
		return offset / getPieceSize(-1);
	}

	//! \brief Retrieves the beginning offset of piece index
	inline qint64 getBeginningOffset(int index) const
	{
		return static_cast<qint64>(getPieceSize(-1)) * index;
	}

	//! \brief Retrieves the ending offset of piece index
	inline qint64 getEndingOffset(int index) const
	{
		++index;
		return index == getPieces() ? getSize() : static_cast<qint64>(getPieceSize(-1)) * index;
	}

	//! \brief Retrieves the comment
	inline QString getComment() const
	{
		return comment;
	}

	//! \brief Retrieves the 'create by' information
	inline QString getCreateBy() const
	{
		return createBy;
	}

	//! \brief Retrieves the creation date
	inline QDateTime getCreationDate() const
	{
		return creationDate;
	}

	inline QString getError() const
	{
		return error;
	}
private:
	static const char defaultEncoding[];
	static const char UTF8ExtensionKeySuffix[];

	static const char encodingKey[];
	static const char codepageKey[];
	static const char announceKey[];
	static const char announceListKey[];
	static const char commentKey[];
	static const char creationDateKey[];
	static const char createdByKey[];
	static const char privateKey[];
	static const char infoKey[];
	static const char sizeKey[];
	static const char filesKey[];
	static const char nameKey[];
	static const char md5sumKey[];
	static const char pathKey[];
	static const char pieceSizeKey[];
	static const char piecesKey[];
	static const char nodesKey[];

	bool valid;

	QString error; //!< The description of last error occured

	QByteArray          infoHash; //!< The hash of the value of the info key

	bool                privateTorrent;	//!< The private torrent flag
	QStringList         announceList;	//!< The URL of the tracker
	QList<MetaFileInfo> files;		//!< The information of files
	QList<QByteArray>   SHA1Hash;		//!< The SHA1 hash of each piece
	qint64              size;		//!< The total number of bytes
	int                 pieceSize;		//!< The number of bytes in each piece
	QString             comment;		//!< The comment
	QString             createBy;		//!< The create by information
	QDateTime           creationDate;	//!< The creation date

	bool parse(const QByteArray &content);

	bool checkKey(const Bcodec::BDictionary &dictionary, const Bcodec::BString &key);

	static QString extractPath(const Bcodec::BDictionary &fileDict, const QTextCodec *codec);

	static QString decodeString(const Bcodec::BDictionary &dictionary, Bcodec::BString key,
					const QTextCodec *codec);
};

}

}

#endif
