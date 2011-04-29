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
#include "metainfo.h"

#include "util/hash/hash.h"

#include <QCoreApplication>
#include <QTextCodec>
#include <QSet>
#include <QUrl>
#include <QDir>

namespace Protocol
{

namespace BitTorrent
{

const char MetaInfo::defaultEncoding[] = "UTF-8";
const char MetaInfo::UTF8ExtensionKeySuffix[] = ".utf-8";

const char MetaInfo::encodingKey[]     = "encoding";
const char MetaInfo::codepageKey[]     = "codepage";
const char MetaInfo::announceKey[]     = "announce";
const char MetaInfo::announceListKey[] = "announce-list";
const char MetaInfo::commentKey[]      = "comment";
const char MetaInfo::creationDateKey[] = "creation date";
const char MetaInfo::createdByKey[]    = "created by";
const char MetaInfo::privateKey[]      = "private";
const char MetaInfo::infoKey[]         = "info";
const char MetaInfo::sizeKey[]         = "length";
const char MetaInfo::filesKey[]        = "files";
const char MetaInfo::nameKey[]         = "name";
const char MetaInfo::md5sumKey[]       = "md5sum";
const char MetaInfo::pathKey[]         = "path";
const char MetaInfo::pieceSizeKey[]    = "piece length";
const char MetaInfo::piecesKey[]       = "pieces";
const char MetaInfo::nodesKey[]        = "nodes";


/*!
 * \brief Generates meta info (.torrent)
 * \param files The information of the files. [path of file on the disk, path of file to save]
 * \param announces The url of the trackers.
 * \param pieceSize The size of the piece, -1 for auto select.
 */
QByteArray MetaInfo::generate(QList<QPair<QFileInfo, QDir>> files, QList<QUrl> announces,
			      int pieceSize)
{
	qint64 totalSize = 0;
	for (auto i = files.constBegin(); i != files.constEnd(); ++i)
		totalSize += i->first.size();

	if (pieceSize == -1) pieceSize = totalSize / (64 * 1024);
	pieceSize = qBound(256 * 1024, pieceSize, 1024 * 1024);

	Bcodec::BDictionary metaInfo;

	// info
	Bcodec::BDictionary info;
	// piece length
	info.insert(pieceSizeKey, QVariant::fromValue(pieceSize));

	/// \todo finish this
	for (auto i = files.constBegin(); i != files.constEnd(); ++i)
	{

	}

	// announce
	metaInfo.insert(announceKey, QVariant::fromValue(announces[0].toString()));

	// announce-list
	Bcodec::BList announceList;
	foreach(const QUrl &announce, announces) announceList.append(announce.toString());

	metaInfo.insert(announceListKey, QVariant::fromValue(announceList));

	// creation date
	metaInfo.insert(creationDateKey, QVariant::fromValue(QDateTime::currentDateTime().toTime_t()));

	// created by
	metaInfo.insert(createdByKey, QVariant::fromValue(QCoreApplication::applicationName() + QCoreApplication::applicationVersion()));

	return Bcodec::encode(metaInfo);
}


/*!
 * \brief Parses the content of metainfo file (.torrent)
 * \note If there are both a key 'length' and a key 'files' (Invalid in fact),
 * only 'length' is considered (thus only single file form is considered).
 * \note If no key 'encoding' found then the default ('UTF-8') is used for creating codec.
 * \param content The metainfo file content to be parsed.
 * \return True if the content is successfully parsed.
 */
bool MetaInfo::parse(const QByteArray &content)
{
	const Bcodec bcodec(content);
	if (!bcodec.isValid())
	{
		error = QLatin1String("Decode failed : ") + bcodec.getError();
		return false;
	}
	const Bcodec::BDictionary &dict = bcodec.getResult();

	// info hash
	infoHash = Utility::Cryptography::Hash::hash(bcodec.getInfoValue(), Utility::Cryptography::Hash::SHA1);

	// encoding
	const QTextCodec *stringCodec = 0;
	if (dict.contains(encodingKey))
		stringCodec = QTextCodec::codecForName(dict[encodingKey].value<Bcodec::BString>());
	if (!stringCodec && dict.contains(codepageKey))
		stringCodec = QTextCodec::codecForName(dict[codepageKey].value<Bcodec::BString>().prepend("CP"));
	if (!stringCodec)
		stringCodec = QTextCodec::codecForName(defaultEncoding);

	// private torrent
	if (dict.contains(privateKey))
		privateTorrent = dict[privateKey].value<Bcodec::BInteger>() == 1;
	else
		privateTorrent = false;

	// Key 'announce' & 'announce-list'
	if (dict.contains(announceKey))
		announceList.append(decodeString(dict, announceKey, stringCodec));
	if (!privateTorrent && dict.contains(announceListKey))
	{
		const Bcodec::BList &list = dict[announceListKey].value<Bcodec::BList>();
		foreach(const QVariant &tier, list)
			foreach (const QVariant &item, tier.value<Bcodec::BList>())
				announceList.append(stringCodec->toUnicode(item.value<Bcodec::BString>()));
	}

	if (announceList.isEmpty())
	{
		error = QLatin1String("No announce found");
		return false;
	}

	// unique
	if (announceList.size() > 1)
		announceList = QStringList::fromSet(announceList.toSet());


	// info dictionary
	if (!checkKey(dict, infoKey))	return false;
	const Bcodec::BDictionary &info = dict[infoKey].value<Bcodec::BDictionary>();

	// The name key maps to a string which is the suggested name to save the file
	// (or directory) as. It is purely advisory.
	if (!checkKey(info, nameKey)) return false;
	const QString &name = decodeString(info, nameKey, stringCodec);

	if (!checkKey(info, pieceSizeKey)) return false;
	pieceSize = info[pieceSizeKey].value<Bcodec::BInteger>();

	if (!checkKey(info, piecesKey)) return false;
	const QByteArray &pieces = info[piecesKey].value<Bcodec::BString>();
	const int SHA1ResultSize = 20;
	if (pieces.size() % SHA1ResultSize)
	{
		error = QString::fromLatin1("Invalid pieces size : %1").arg(pieces.size());
		return false;
	}
	for (int i = 0; i < pieces.size(); i += SHA1ResultSize)
		SHA1Hash.append(pieces.mid(i, SHA1ResultSize));

	MetaFileInfo metaFileInfo;
	if (info.contains(sizeKey))
	{
		// Single file
		metaFileInfo.size = info[sizeKey].value<Bcodec::BInteger>();
		metaFileInfo.path = name;
		if (info.contains(md5sumKey))
			metaFileInfo.md5sum = info[md5sumKey].value<Bcodec::BString>();
		files.append(metaFileInfo);
	}
	else if (info.contains(filesKey))
	{
		// The directory name
		const QString &directory = name;

		// Multi files
		const Bcodec::BList &files = info[filesKey].value<Bcodec::BList>();

		for (auto iter = files.constBegin(); iter != files.constEnd(); ++iter)
		{
			const Bcodec::BDictionary &file = iter->value<Bcodec::BDictionary>();

			if (!checkKey(file, sizeKey)) return false;
			metaFileInfo.size = file[sizeKey].value<Bcodec::BInteger>();

			metaFileInfo.path = directory;
			// path - A list of strings corresponding to subdirectory names,
			// the last of which is the actual file name.
			if (!checkKey(file, pathKey)) return false;
			metaFileInfo.path.append(extractPath(file, stringCodec));

			if (file.contains(md5sumKey))
				metaFileInfo.md5sum = file[md5sumKey].value<Bcodec::BString>();
			else
				metaFileInfo.md5sum.clear();
			this->files.append(metaFileInfo);
		}
	}
	else
	{
		error = QString::fromLatin1("Neither key '%1' nor key '%2' found")
			.arg(QLatin1String(sizeKey)).arg(QLatin1String(filesKey));
		return false;
	}

	// total size
	size = 0;
	foreach (const MetaFileInfo &metaFileInfo, files)
		size += metaFileInfo.size;

	// other keys
	if (dict.contains(commentKey))
		comment = decodeString(dict, commentKey, stringCodec);
	if (dict.contains(creationDateKey))
		creationDate.setTime_t(dict[creationDateKey].value<Bcodec::BInteger>());
	if (dict.contains(createdByKey))
		createBy = decodeString(dict, createdByKey, stringCodec);

	return true;
}

/*!
 * \brief Helper function to check whether there is a key named 'keyName' in the dictionary
 *
 * Error description is modified accordingly if the key is not found.
 * \param dictionary The dictionary to check.
 * \param key The name of the key to check.
 */
bool MetaInfo::checkKey(const Bcodec::BDictionary &dictionary, const Bcodec::BString &key)
{
	if (dictionary.contains(key)) return true;
	error = QString::fromLatin1("Key '%1' not found").arg(QLatin1String(key.constData()));
	return false;
}

QString MetaInfo::extractPath(const Bcodec::BDictionary &fileDict, const QTextCodec *codec)
{
	Bcodec::BString key = pathKey;
	if (fileDict.contains(key + UTF8ExtensionKeySuffix))
	{
		key.append(UTF8ExtensionKeySuffix);
		codec = QTextCodec::codecForName("UTF-8");
	}
	const Bcodec::BList &pathList = fileDict[key].value<Bcodec::BList>();

	QString result;
	for (auto iter = pathList.constBegin(); iter != pathList.constEnd(); ++iter)
		result.append(QLatin1Char('/')).append(codec->toUnicode(iter->value<Bcodec::BString>()));

	return result;
}


QString MetaInfo::decodeString(const Bcodec::BDictionary &dictionary, Bcodec::BString key,
				const QTextCodec *codec)
{
	if (dictionary.contains(key + UTF8ExtensionKeySuffix))
	{
		key.append(UTF8ExtensionKeySuffix);
		codec = QTextCodec::codecForName("UTF-8");
	}
	return codec->toUnicode(dictionary[key].value<Bcodec::BString>());
}

}

}
