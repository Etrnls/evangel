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
#include "peerwiresocket.h"
#include "peerid.h"

#include "protocol/bt/util/metainfo.h"
#include "protocol/bt/algorithm/blockselector.h"

#include "log.h"
#include "util/endian.h"
#include "util/hash/hash.h"
#include "util/numeric/integer.h"
#include "util/numeric/random.h"
#include "network/ratestatistics.h"

#include <QTimerEvent>
#include <QHostAddress>
#include <QString>

namespace Protocol
{

namespace BitTorrent
{

const char PeerWireSocket::protocolIDSize = 19;
const char PeerWireSocket::protocolID[protocolIDSize + 1] = "BitTorrent protocol";

const PeerWireSocket::FeatureFlag PeerWireSocket::features[] = {/*DHTProtocolFeature, */
								ExtensionProtocolFeature,
								/*FastExtensionFeature, */};

const int PeerWireSocket::minimumPendingRequests = 8;
const int PeerWireSocket::maximumPendingRequests = 128;
const int PeerWireSocket::maxPendingOutgoingBlocksSize = maximumPendingRequests * BlockSelector::maximalBlockSize;

const char PeerWireSocket::extensionSupportedMessagesKey[] = "m";
const char PeerWireSocket::extensionListenPortKey[] = "p";
const char PeerWireSocket::extensionClientNameKey[] = "v";
const char PeerWireSocket::extensionMaximumRequestsKey[] = "reqq";

const Utility::Integer PeerWireSocket::DHPrime(QLatin1String("FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B"
								"80DC1CD129024E088A67CC74020BBEA63B139B22"
								"514A08798E3404DDEF9519B3CD3A431B302B0A6D"
								"F25F14374FE1356D6D51C245E485B576625E7EC6"
								"F44C42E9A63A36210000000000090563"));

QByteArray packInteger(int n, const qint32 x[])
{
	QByteArray result(sizeof(qint32) * n, Qt::Uninitialized);

	for (int i = 0; i < n; ++i)	Utility::Endian::toBigEndian<quint32>(x[i], result.data() + sizeof(quint32) * i);

	return result;
}

PeerWireSocket::PeerWireSocket(EncryptionMethod encryptionMethod, ConnectionType connectionType)
	: TCPSocket(NormalPriority), connectionType(connectionType), encryptionMethod(encryptionMethod),
	client(0), feature(0),
	privateKey(0), encryptor(new PlaintextEncryptor()), decryptor(new PlaintextEncryptor()),
	selfState(ChokedState), peerState(ChokedState),
	packetSize(-1),
	downloadRate(new Network::RateStatistics(this)), uploadRate(new Network::RateStatistics(this)),
	peerMaximumPendingRequests(maximumPendingRequests),
	desiredPendingIncomingBlocks((minimumPendingRequests + maximumPendingRequests) / 2),
	pendingOutgoingBlocksSize(0),
	keepAliveTimer(0), timeOutTimer(0), pendingRequestTimer(0),
	invalidateTimeOutTimer(false), invalidateKeepAliveTimer(false),
	handshaked(false), DHExchanged(false), MSEResynchronized(false), MSECryptoSelected(false), requestScheduled(false)
{
	connect(this, SIGNAL(connected()), SLOT(processConnected()));
	connect(this, SIGNAL(disconnected()), SLOT(processDisconnected()));
	connect(this, SIGNAL(error()), SLOT(processError()));

	downloadRate->start();
	uploadRate->start();

	switch (connectionType)
	{
	case IncomingConnection:
		processConnected();
		break;
	case OutgoingConnection:
		break;
	default:
		Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid connection type");
		break;
	}
}

PeerWireSocket::~PeerWireSocket()
{
}

void PeerWireSocket::setClient(BTClient *client)
{
	this->client = client;
	peerBitmap.resize(client->getMetaInfo()->getPieces());

	connect(client, SIGNAL(stateChanged()), SLOT(clientStateChanged()));
}

qreal PeerWireSocket::getDownloadRate() const
{
	return downloadRate->getRate();
}

qreal PeerWireSocket::getUploadRate() const
{
	return uploadRate->getRate();
}

int PeerWireSocket::getConnectionTime() const
{
	return downloadRate->getTotalTime();
}

bool PeerWireSocket::canUpload() const
{
	return pendingOutgoingBlocksSize > 0 || TCPSocket::canUpload();
}

int PeerWireSocket::writeToNetwork(int maxSize)
{
	int size = qMin(maxSize - outgoingBuffer.size(), pendingOutgoingBlocksSize);

	while (size > 0)
	{
		BlockPacket blockPacket = pendingOutgoingBlocks.takeFirst();
		pendingOutgoingBlocksSize -= blockPacket.data.size();

		size -= blockPacket.data.size();

		sendMessage(PieceMessage, blockPacket.data);

		uploadRate->transfer(blockPacket.data.size());
		emit blockSent(blockPacket.index, blockPacket.offset, blockPacket.data.size());
	}

	return TCPSocket::upload(maxSize);
}

void PeerWireSocket::scheduleRequest()
{
	if (!requestScheduled)
	{
		requestScheduled = true;
		QMetaObject::invokeMethod(this, "autoRequest", Qt::QueuedConnection);
	}
}

/*!
 * \brief Sends the choke message
 *
 * \c \b choke: [len=0001][id=0]
 *
 * The choke message is fixed-length and has no payload.
 */
void PeerWireSocket::sendChoke()
{
	if (peerState.testFlag(ChokedState))	return;
	peerState |= ChokedState;

	pendingOutgoingBlocksSize = 0;
	pendingOutgoingBlocks.clear();

	sendMessage(ChokeMessage);

	messageLog(SendDirection, QLatin1String("choke"));
}

/*!
 * \brief Sends the unchoke message
 *
 * \c \b unchoke: [len=0001][id=1]
 *
 * The unchoke message is fixed-length and has no payload.
 */
void PeerWireSocket::sendUnchoke()
{
	if (!peerState.testFlag(ChokedState))	return;
	peerState ^= ChokedState;

	sendMessage(UnchokeMessage);

	messageLog(SendDirection, QLatin1String("unchoke"));
}



/*!
 * \brief Sends the have message
 *
 * \c \b have: [len=0005][id=4][piece index]
 *
 * The have message is fixed length. The payload is the zero-based index of a piece
 * that has just been successfully downloaded and verified via the hash.
 */
void PeerWireSocket::sendHave(int index)
{
	foreach (const BlockPacket &blockPacket, pendingIncomingBlocks)
		if (blockPacket.index == index)
			sendCancel(blockPacket.index, blockPacket.offset, blockPacket.size);

	const qint32 data[] = {index};
	sendMessage(HaveMessage, packInteger(sizeof(data) / sizeof(*data), data));

	messageLog(SendDirection, QString::fromLatin1("have [%1]").arg(index));

	autoUpdateInterested();
}


/*!
 * \brief Drops the connection
 * \param reason The reason why the connection is dropped.
 * \param seriousness The value represents the seriousness of problem. This is used to help selecting peers to connect.
 * \note Set seriousness to INT_MAX to delete the corresponding BTSource after disconnected.
 */
void PeerWireSocket::dropConnection(const QString &reason, int seriousness)
{
	emit drop(reason, seriousness);

	disconnectFromHost();
}

void PeerWireSocket::clientStateChanged()
{
	switch (client->getState())
	{
	case BTClient::LeechingState:
		selfState &= ~SeedingState;
		break;
	case BTClient::SeedingState:
		selfState |= SeedingState;
		break;
	default:
		break;
	}
}

void PeerWireSocket::processConnected()
{
	disconnect(this, SIGNAL(connected()), this, SLOT(processConnected()));
	setTimer(&timeOutTimer, clientTimeOut);

	// handshake is send first by connection initiator
	if (connectionType == OutgoingConnection)
	{
		switch (encryptionMethod)
		{
		case NoneEncryption:
			sendHandshake();
			break;
		case MessageStreamEncryption:
			sendPublicKey();
			break;
		default:
			Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid encryption method");
			break;
		}
	}
}

void PeerWireSocket::processDisconnected()
{
	disconnect(this, SIGNAL(disconnected()), this, SLOT(processDisconnected()));

	foreach (const BlockPacket &blockPacket, pendingIncomingBlocks)
		emit blockCanceled(blockPacket.index, blockPacket.offset, blockPacket.size);

	for (int i = 0; i < peerBitmap.size(); ++i)
		if (peerBitmap.testBit(i))	emit peerBitmapUpdated(i, false);

	emit disconnectedFromPeer();
}

void PeerWireSocket::processError()
{
	disconnect(this, SIGNAL(error()), this, SLOT(processError()));

	log(this) << QLatin1String("Socket error") << getError() << QLatin1String("from") << getPeerAddressPort() << endl;

	processDisconnected();
}

/*!
 * \brief Processes the incoming data
 */
void PeerWireSocket::processIncomingData()
{
	if (bytesAvailable() == 0)	return;
	invalidateTimeOutTimer = true;

	if (!handshaked)
	{
		switch (encryptionMethod)
		{
		case NoneEncryption:
			processHandshake();
			break;
		case MessageStreamEncryption:
			processMSEHandshake();
			break;
		default:
			Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid encryption method");
			break;
		}
		if (!handshaked)	return;
	}

	while (bytesAvailable() > 0)
	{
		if (packetSize == -1 && (packetSize = getPacketSize()) == -1)	return;

		if (packetSize == 0) // keep alive
		{
			packetSize = -1;
			continue;
		}

		// no enough data received
		if (bytesAvailable() < packetSize)	return;

		QByteArray packet = read(packetSize);
		decryptor->process(packet);
		packetSize = -1; // prepare for the next packet

		parsePacket(packet);
	}
}


void PeerWireSocket::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == timeOutTimer)
	{
		// drop the connection if no data received within a certain timeframe
		if (invalidateTimeOutTimer)
			invalidateTimeOutTimer = false;
		else
			dropConnection(QLatin1String("Client time out"), timeOutPenalty);
	}
	else if (event->timerId() == keepAliveTimer)
	{
		if (invalidateKeepAliveTimer)
			invalidateKeepAliveTimer =false;
		else
			sendKeepAlive();
	}
	else if (event->timerId() == pendingRequestTimer)
	{
		if (selfState.testFlag(ChokedState))
			dropConnection(QLatin1String("Pending unchoke time out"), timeOutPenalty);
		else
			dropConnection(QLatin1String("Request time out"), timeOutPenalty);
	}
	else
		TCPSocket::timerEvent(event);
}

/*!
 * \brief Prepare and request
 *
 * This function does the following jobs:
 * \li Check client state, no request and drop other seeder when we are seeding.
 * \li Update \c \b interested state.
 * \li Check \c \b choked state.
 * \li Select block & send request.
 */
void PeerWireSocket::autoRequest()
{
	requestScheduled = false;

	// no request when we are seeding
	if (client->getState() == BTClient::SeedingState)
	{
		if (peerState.testFlag(SeedingState))
			dropConnection(QLatin1String("Drop other seeder in seeding state"), 0);
		return;
	}

	// check if we are in choked state
	if (selfState.testFlag(ChokedState))	return;

	// what we have already requested (within THIS peer)
	Utility::Bitmap requested(Utility::Bitmap::EmptyStatus, client->getMetaInfo()->getSize());
	foreach (const BlockPacket &blockPacket, pendingIncomingBlocks)
	{
		const qint64 begin = client->getMetaInfo()->getBeginningOffset(blockPacket.index) + blockPacket.offset;
		const qint64 end = begin + blockPacket.size;
		const Utility::Bitmap::Segment segment(begin, end);
		requested.updateStatus(Utility::Bitmap::getStatusUpdater(Utility::Bitmap::RequestedStatus), segment);
	}

	const int rateDesiredPendingIncomingBlocks = 8 * downloadRate->getRate() / BlockSelector::defaultBlockSize;
	if (rateDesiredPendingIncomingBlocks >= desiredPendingIncomingBlocks)
		desiredPendingIncomingBlocks = qMin(desiredPendingIncomingBlocks * 2, rateDesiredPendingIncomingBlocks);
	else
		--desiredPendingIncomingBlocks;
	desiredPendingIncomingBlocks = qBound(minimumPendingRequests, desiredPendingIncomingBlocks, maximumPendingRequests);
	desiredPendingIncomingBlocks = qMin(desiredPendingIncomingBlocks, peerMaximumPendingRequests);

	while (pendingIncomingBlocks.size() < desiredPendingIncomingBlocks)
	{
		int index, offset, size;

		if (!client->selectBlock(peerBitmap, requested, &index, &offset, &size))
		{
			log(this) << QLatin1String("Failed to select a block") << endl;
			break;
		}

		sendRequest(index, offset, size);

		const qint64 begin = client->getMetaInfo()->getBeginningOffset(index) + offset;
		const qint64 end = begin + size;
		const Utility::Bitmap::Segment segment(begin, end);
		requested.updateStatus(Utility::Bitmap::getStatusUpdater(Utility::Bitmap::RequestedStatus), segment);
	}
}

/*!
 * \brief Cancel all blocks in the range
 * \param offset The offset of the range.
 * \param size The size of the range.
 */
void PeerWireSocket::autoCancel(qint64 offset, qint64 size)
{
	foreach (const BlockPacket &blockPacket, pendingIncomingBlocks)
	{
		const qint64 blockOffset = client->getMetaInfo()->getBeginningOffset(blockPacket.index) + blockPacket.offset;
		const qint64 blockSize = blockPacket.size;

		if (offset <= blockOffset && blockOffset + blockSize <= offset + size)
			sendCancel(blockPacket.index, blockPacket.offset, blockPacket.size);
	}
}

/*!
 * \brief Sends a message
 * \param type The type of the message.
 * \param data The payload of the message.
 */
void PeerWireSocket::sendMessage(MessageType type, const QByteArray &data)
{
	char message[4 + 1] = {0, 0, 0, 0, static_cast<char>(type)};
	Utility::Endian::toBigEndian<quint32>(1 + data.size(), message);

	QByteArray packet(message, sizeof(message));
	packet.append(data);
	encryptor->process(packet);
	write(packet);

	invalidateKeepAliveTimer = true;
}

void PeerWireSocket::sendPublicKey()
{
	if (connectionType == OutgoingConnection)	generatePrivateKey();

	const Utility::Integer &publicKey = Utility::Integer::powerMod(Utility::Integer(2), *privateKey, DHPrime);
	const QByteArray padA(Utility::rand(maxPadASize), Qt::Uninitialized);

	write(static_cast<QByteArray>(publicKey).rightJustified(DHKeySize, 0).append(padA));
}

void PeerWireSocket::sendCryptoProvide()
{
	// HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S)
	{
		QByteArray data(SHA1HashSize * 2, Qt::Uninitialized);

		const QByteArray &req1Data = QByteArray("req1") + *DHSecret;
		const QByteArray &req2Data = QByteArray("req2") + client->getMetaInfo()->getInfoHash();
		const QByteArray &req3Data = QByteArray("req3") + *DHSecret;

		const QByteArray &req1 = Utility::Cryptography::Hash::hash(req1Data, Utility::Cryptography::Hash::SHA1);
		const QByteArray &req2 = Utility::Cryptography::Hash::hash(req2Data, Utility::Cryptography::Hash::SHA1);
		const QByteArray &req3 = Utility::Cryptography::Hash::hash(req3Data, Utility::Cryptography::Hash::SHA1);

		Q_ASSERT_X(req1.size() == SHA1HashSize, Q_FUNC_INFO, "Invalid req1");
		Q_ASSERT_X(req2.size() == SHA1HashSize, Q_FUNC_INFO, "Invalid req2");
		Q_ASSERT_X(req3.size() == SHA1HashSize, Q_FUNC_INFO, "Invalid req3");

		data.replace(0, SHA1HashSize, req1);
		for (int i = 0; i < SHA1HashSize; ++i)
			data[SHA1HashSize + i] = static_cast<quint8>(req2[i]) ^ static_cast<quint8>(req3[i]);
		write(data);
	}

	// ENCRYPT(VC, crypto_provide, len(padC), padC, len(IA)), ENCRYPT(IA)
	{
		// we set padC and IA to nothing
		QByteArray data(VCSize + cryptoProvideSize + lenSize + lenSize, Qt::Uninitialized);
		const CryptoType cryptoProvide = CryptoType(CryptoRC4) | CryptoType(CryptoPlaintext);
		int offset = 0;

		Utility::Endian::toBigEndian<quint64>(0, &data.data()[offset]);
		offset += VCSize;

		Utility::Endian::toBigEndian<quint32>(cryptoProvide, &data.data()[offset]);
		offset += cryptoProvideSize;

		Utility::Endian::toBigEndian<quint16>(0, &data.data()[offset]);
		offset += lenSize;

		Utility::Endian::toBigEndian<quint16>(0, &data.data()[offset]);

		encryptor->process(data);
		write(data);
	}
}

void PeerWireSocket::sendCryptoSelect(CryptoType cryptoSelect)
{
	// ENCRYPT(VC, crypto_select, len(padD), padD)
	{
		// we set padD to nothing
		QByteArray data(VCSize + cryptoSelectSize + lenSize, Qt::Uninitialized);
		int offset = 0;

		Utility::Endian::toBigEndian<quint64>(0, &data.data()[offset]);
		offset += VCSize;

		Utility::Endian::toBigEndian<quint32>(cryptoSelect, &data.data()[offset]);
		offset += cryptoSelectSize;

		Utility::Endian::toBigEndian<quint16>(0, &data.data()[offset]);

		encryptor->process(data);
		write(data);
	}
}

/*!
 * \brief Sends the handshake message
 *
 * The handshake is a required message and must be the first message transmitted by the client.
 * It is (49+len(pstr)) bytes long.
 *
 * \c \b handshake: [pstrlen][pstr][reserved][info_hash][peer_id]
 *
 * \li \c \b pstrlen: string length of [pstr], as a single raw byte.
 * \li \c \b pstr: string identifier of the protocol.
 * \li \c \b reserved: eight (8) reserved bytes. All current implementations use all zeroes.
 * Each bit in these bytes can be used to change the behavior of the protocol.
 * An email from Bram suggests that trailing bits should be used first, so that leading bits may be
 * used to change the meaning of trailing bits.
 * \li \c \b info_hash: 20-byte SHA1 hash of the info key in the metainfo file.
 * This is the same info_hash that is transmitted in tracker requests.
 * \li \c \b peer_id: 20-byte string used as a unique ID for the client.
 * This is usually the same peer_id that is transmitted in tracker requests.
 *
 * \note The handshake is sent immediately after connected or after authorized successfully for incoming connection.
 */
void PeerWireSocket::sendHandshake()
{
	Q_ASSERT_X(client, Q_FUNC_INFO, "Invalid client");

	QByteArray data;

	// protocol ID
	data.append(protocolIDSize).append(protocolID, protocolIDSize);

	// reserved bits
	{
		QByteArray reserved(reservedSize, 0);

		for (int i = 0; i < static_cast<int>(sizeof(features) / sizeof(*features)); ++i)
		{
			const QVariant info = getFeatureInfo(features[i]);
			if (info.userType() == qMetaTypeId<int>())
			{
				const int index = info.value<int>() >> 8;
				const int mask = info.value<int>() & 0xff;
				reserved[index] = static_cast<quint8>(reserved[index]) | mask;
			}
		}

		data.append(reserved);
	}

	// info hash
	data.append(client->getMetaInfo()->getInfoHash());

	// peer ID
	data.append(BTClient::getPeerID().getID());

	encryptor->process(data);
	write(data);

	messageLog(SendDirection, QLatin1String("handshake"));
}

/*!
 * \brief Sends the keep-alive message
 *
 * \c \b keep-alive: [len=0000]
 *
 * The keep-alive message is a message with zero bytes, specified with the
 * length prefix set to zero. There is no message ID and no payload.
 * Peers may close a connection if they receive no messages (keep-alive or
 * any other message) for a certain period of time, so a keep-alive message
 * must be sent to maintain the connection alive if no command have been sent
 * for a given amount of time. This amount of time is generally two minutes.
 */
void PeerWireSocket::sendKeepAlive()
{
	static const char message[] = {0, 0, 0, 0};
	QByteArray packet(message, sizeof(message));
	encryptor->process(packet);
	write(packet);
}

/*!
 * \brief Sends the interested message
 *
 * \c \b interested: [len=0001][id=2]
 *
 * The interested message is fixed-length and has no payload.
 */
void PeerWireSocket::sendInterested()
{
	if (peerState.testFlag(InterestedState))	return;
	peerState |= InterestedState;

	sendMessage(InterestedMessage);

	// After telling the peer that we're interested, we expect to get
	// unchoked within a certain timeframe; otherwise we'll drop the
	// connection.
	setTimer(&pendingRequestTimer, pendingUnchokeTimeout);
}

/*!
 * \brief Sends the not interested message
 *
 * \c \b not-interested: [len=0001][id=3]
 *
 * The not-interested message is fixed-length and has no payload.
 */
void PeerWireSocket::sendNotInterested()
{
	if (!peerState.testFlag(InterestedState))	return;
	peerState ^= InterestedState;

	sendMessage(NotInterestedMessage);
}

/*!
 * \brief Check and update \c \b interested state
 *
 * We are interested in the peer IFF the peer has some piece that we don't have.
 */
void PeerWireSocket::autoUpdateInterested()
{
	const QBitArray &verified = client->getVerified();
	Q_ASSERT_X(verified.size() == peerBitmap.size(), Q_FUNC_INFO, "Invalid bitmaps");
	for (int i = 0; i < verified.size(); ++i)
		if (!verified.testBit(i) && peerBitmap.testBit(i))
		{
			sendInterested();
			return;
		}
	sendNotInterested();
}


/*!
 * \brief Sends the bitfield message
 *
 * \c \b bitfield: [len=0001+X][id=5][bitfield]
 *
 * The bitfield message may only be sent immediately after the handshaking sequence is completed,
 * and before any other messages are sent. It is optional, and need not be sent if a client has
 * no pieces.
 *
 * The bitfield message is variable length, where X is the length of the bitfield. The payload is
 * a bitfield representing the pieces that have been successfully downloaded. The high bit in the
 * first byte corresponds to piece index 0. Bits that are cleared indicated a missing piece, and
 * set bits indicate a valid and available piece. Spare bits at the end are set to zero.
 *
 * A bitfield of the wrong length is considered an error. Clients should drop the connection if
 * they receive bitfields that are not of the correct size, or if the bitfield has any of the
 * spare bits set.
 */
void PeerWireSocket::sendBitField()
{
	if (client->getVerified().count(true) == 0)	return;

	QByteArray bitField((client->getVerified().size() + 7) / 8, 0);

	for (int i = 0; i < client->getVerified().size(); ++i)
		if (client->getVerified().testBit(i))
			bitField[i / 8] = bitField[i / 8] | (1 << (7 - (i % 8)));

	sendMessage(BitFieldMessage, bitField);
}

/*!
 * \brief Sends a request message
 *
 * \c \b request: [len=0013][id=6][index][begin][length]
 *
 * The request message is fixed length, and is used to request a block.
 * The payload contains the following information:
 *
 * \li \c \b index: integer specifying the zero-based piece index.
 * \li \c \b begin: integer specifying the zero-based byte offset within the piece.
 * \li \c \b length: integer specifying the requested length.
 */
void PeerWireSocket::sendRequest(int index, int offset, int size)
{
	const qint32 data[] = {index, offset, size};
	sendMessage(RequestMessage, packInteger(sizeof(data) / sizeof(*data), data));

	pendingIncomingBlocks.append(BlockPacket(index, offset, size, QByteArray()));

	// After requesting a block, we expect the block to be sent by the
	// other peer within a certain number of seconds. Otherwise, we
	// drop the connection.
	setTimer(&pendingRequestTimer, pendingRequestTimeout);

	messageLog(SendDirection, QString::fromLatin1("request [%1, %2, %3]").arg(index).arg(offset).arg(size));
	
	emit blockRequested(index, offset, size);
}

/*!
 * \brief Sends a piece message
 *
 * \c \b piece: [len=0009+X][id=7][index][begin][block]
 *
 * The piece message is variable length, where X is the length of the block.
 * The payload contains the following information:
 *
 * \li \c \b index: integer specifying the zero-based piece index.
 * \li \c \b begin: integer specifying the zero-based byte offset within the piece.
 * \li \c \b block: block of data, which is a subset of the piece specified by index.
 */
void PeerWireSocket::sendPiece(int index, int offset, const QByteArray &block)
{
	const qint32 data[] = {index, offset};
	BlockPacket blockPacket(index, offset, block.size(), packInteger(sizeof(data) / sizeof(*data), data).append(block));

	if (pendingOutgoingBlocksSize + blockPacket.data.size() > maxPendingOutgoingBlocksSize)
	{
		sendChoke();
		sendUnchoke();
	}
	else
	{
		pendingOutgoingBlocksSize += blockPacket.data.size();
		pendingOutgoingBlocks.append(blockPacket);

		messageLog(SendDirection, QString::fromLatin1("piece [%1, %2, %3]").arg(index).arg(offset).arg(block.size()));

		emit readyUpload();
	}
}


/*!
 * \brief Sends a cancel messageSize
 *
 * \c \b cancel: [len=0013][id<=8][index][begin][length]
 *
 * The cancel message is fixed length, and is used to cancel block requests.
 * The payload is identical to that of the "request" message.
 * It is typically used during "End Game".
 */
void PeerWireSocket::sendCancel(int index, int offset, int size)
{
	if (!selfState.testFlag(ChokedState))
	{
		const qint32 data[] = {index, offset, size};
		sendMessage(CancelMessage, packInteger(sizeof(data) / sizeof(*data), data));
	}

	pendingIncomingBlocks.removeOne(BlockPacket(index, offset, size, QByteArray()));

	setTimer(&pendingRequestTimer, pendingIncomingBlocks.isEmpty() ? 0 : pendingRequestTimeout);
	
	emit blockCanceled(index, offset, size);
}

void PeerWireSocket::sendExtended(ExtendedMessageType type, QByteArray data)
{
	sendMessage(ExtendedMessage, data.prepend(type));
}

/*!
 * \brief Sends a extended handshake
 */
void PeerWireSocket::sendExtendedHandshake()
{
	Bcodec::BDictionary message;

	{
		Bcodec::BDictionary supportedMessages;

		// insert supported messages here

		message.insert(Bcodec::BString(extensionSupportedMessagesKey), QVariant::fromValue(supportedMessages));
	}

	message.insert(Bcodec::BString(extensionClientNameKey),
			QVariant::fromValue(Bcodec::BString(client->getPeerID().getClientName().toLatin1())));

	sendExtended(ExtendedHandshakeMessage, Bcodec::encode(message));
}

int PeerWireSocket::getPacketSize()
{
	static const int maxPacketSize = BlockSelector::maximalBlockSize * 2;

	if (bytesAvailable() < static_cast<int>(sizeof(qint32)))	return -1;

	QByteArray data = read(sizeof(qint32));
	decryptor->process(data);
	const int result = Utility::Endian::fromBigEndian<quint32>(data.constData());

	if (result < 0 || result > maxPacketSize)
	{
		dropConnection(QLatin1String("Invalid packet size"), badPacketPenalty);
		return -1;
	}

	return result;
}

/*!
 * \brief Parses the packet
 * \param packet The packet to parse.
 *
 * The function extract the message type and pass it to the process function if necessary.
 *
 * Unknown message type is ignored.
 */
void PeerWireSocket::parsePacket(const QByteArray &packet)
{
	switch (static_cast<MessageType>(packet[0]))
	{
	case ChokeMessage: // we have been choked
		processChoke();
		break;
	case UnchokeMessage: // we have been unchoked
		processUnchoke();
		break;
	case InterestedMessage: // the peer is interested in us
		selfState |= InterestedState;
		break;
	case NotInterestedMessage: // the peer is not interested in us
		selfState &= ~InterestedState;
		break;
	case HaveMessage:
		processHave(Utility::Endian::fromBigEndian<quint32>(&packet.constData()[1]));
		break;
	case BitFieldMessage:
		processBitField(packet.mid(1));
		break;
	case RequestMessage:
		processRequest(Utility::Endian::fromBigEndian<quint32>(&packet.constData()[1]),
				Utility::Endian::fromBigEndian<quint32>(&packet.constData()[5]),
				Utility::Endian::fromBigEndian<quint32>(&packet.constData()[9]));
		break;
	case PieceMessage:
		processPiece(Utility::Endian::fromBigEndian<quint32>(&packet.constData()[1]),
				Utility::Endian::fromBigEndian<quint32>(&packet.constData()[5]),
				packet.mid(9));
		break;
	case CancelMessage:
		processCancel(Utility::Endian::fromBigEndian<quint32>(&packet.constData()[1]),
				Utility::Endian::fromBigEndian<quint32>(&packet.constData()[5]),
				Utility::Endian::fromBigEndian<quint32>(&packet.constData()[9]));
		break;
	case ExtendedMessage:
		processExtendedMessage(ExtendedMessageType(packet[1]), packet.mid(2));
		break;
	default:
		messageLog(ReceiveDirection, QString::fromLatin1("unsupported packet [%1]").arg(static_cast<int>(packet[0])));
		break;
	}
}

void PeerWireSocket::processMSEHandshake()
{
	// 1 A->B: Diffie Hellman Ya, PadA
	// 2 B->A: Diffie Hellman Yb, PadB
	// 3 A->B: HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S), ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)
	// 4 B->A: ENCRYPT(VC, crypto_select, len(padD), padD), ENCRYPT2(Payload Stream)
	// 5 A->B: ENCRYPT2(Payload Stream)

	if (!DHExchanged)
	{
		if (!processPublicKey())	return;
		DHExchanged = true;
	}

	if (!MSECryptoSelected)
	{
		switch (connectionType)
		{
		case IncomingConnection:
			if (!processCryptoProvide())	return;
			break;
		case OutgoingConnection:
			if (!processCryptoSelect())	return;
			sendHandshake();
			break;
		default:
			Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid connection type");
			break;
		}
		MSECryptoSelected = true;
	}

	delete DHSecret;
	DHSecret = 0;

	log(this) << QLatin1String("MSE handshaked with") << getPeerAddressPort() << endl;

	return processHandshake();
}

bool PeerWireSocket::processPublicKey()
{
	static const int publicKeySize = 96;
	if (bytesAvailable() < publicKeySize)	return false;

	if (connectionType == IncomingConnection)	generatePrivateKey();

	const QByteArray &key = read(publicKeySize);
	DHSecret = new QByteArray(static_cast<QByteArray>(Utility::Integer::powerMod(Utility::Integer(key), *privateKey, DHPrime)).rightJustified(DHKeySize, 0));

	switch (connectionType)
	{
	case IncomingConnection:
		sendPublicKey();
		break;
	case OutgoingConnection:
		setUpCryptor("keyA", "keyB");
		sendCryptoProvide();
		break;
	default:
		Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid connection type");
		break;
	}

	destroyPrivateKey();

	return true;
}

bool PeerWireSocket::processCryptoProvide()
{
	Q_ASSERT_X(connectionType == IncomingConnection, Q_FUNC_INFO, "Invalid call");

	// HASH('req1', S)
	if (!MSEResynchronized)
	{
		if (bytesAvailable() < SHA1HashSize)	return false;
		
		const QByteArray &req1Data = QByteArray("req1") + *DHSecret;
		const QByteArray &req1 = Utility::Cryptography::Hash::hash(req1Data, Utility::Cryptography::Hash::SHA1);
		Q_ASSERT_X(req1.size() == SHA1HashSize, Q_FUNC_INFO, "Invalid req1");

		const QByteArray &data = read(bytesAvailable());
		const int index = data.indexOf(req1);

		if (index == -1)
		{
			if (data.size() > maxPadASize + SHA1HashSize)
				dropConnection(QLatin1String("Failed to resynchronize on req1"), badPacketPenalty);
			else
				unread(data);
			return false;
		}

		unread(data.mid(index + SHA1HashSize));
		MSEResynchronized = true;
	}

	// HASH('req2', SKEY) xor HASH('req3', S)
	if (!client)
	{
		if (bytesAvailable() < SHA1HashSize)	return false;

		const QByteArray &data = read(SHA1HashSize);
		const QByteArray &req3Data = QByteArray("req3") + *DHSecret;
		const QByteArray &req3 = Utility::Cryptography::Hash::hash(req3Data, Utility::Cryptography::Hash::SHA1);
		QByteArray req2(SHA1HashSize, Qt::Uninitialized);
		for (int i = 0; i < SHA1HashSize; ++i)
			req2[i] = static_cast<quint8>(data[i]) ^ static_cast<quint8>(req3[i]);

		foreach (BTClient *client, BTClient::getClients())
		{
			const QByteArray &req2Data = QByteArray("req2") + client->getMetaInfo()->getInfoHash();
			if (req2 == Utility::Cryptography::Hash::hash(req2Data, Utility::Cryptography::Hash::SHA1))
			{
				emit infoHashReceived(client->getMetaInfo()->getInfoHash());
				break;
			}
		}

		if (!client)	return false;
		setUpCryptor("keyB", "keyA");
	}

	// ENCRYPT(VC, crypto_provide, len(padC), padC, len(IA)), ENCRYPT(IA)
	{
		if (bytesAvailable() < VCSize + cryptoProvideSize + lenSize)	return false;

		QScopedPointer<RC4Encryptor> decryptor(new RC4Encryptor(*dynamic_cast<RC4Encryptor*>(this->decryptor)));

		// keep a copy of the original data in case we need to unread
		QByteArray originalData;

		// VC
		QByteArray VC = read(VCSize);
		originalData.append(VC);
		decryptor->process(VC);
		if (VC != QByteArray(VCSize, 0))
		{
			dropConnection(QLatin1String("Invalid VC"), badPacketPenalty);
			return false;
		}

		// crypto_provide
		QByteArray cryptoProvideData = read(cryptoProvideSize);
		originalData.append(cryptoProvideData);
		decryptor->process(cryptoProvideData);
		const CryptoType cryptoProvide = CryptoType(Utility::Endian::fromBigEndian<quint32>(cryptoProvideData.constData()));

		// len(padC)
		QByteArray padCSizeData = read(lenSize);
		originalData.append(padCSizeData);
		decryptor->process(padCSizeData);
		const int padCSize = Utility::Endian::fromBigEndian<quint16>(padCSizeData.constData());
		if (padCSize > maxPadCSize)
		{
			dropConnection(QLatin1String("Invalid pad C size"), badPacketPenalty);
			return false;
		}

		// check if we have padC and len(IA)
		if (bytesAvailable() < padCSize + lenSize)
		{
			unread(originalData);
			return false;
		}

		// padC
		QByteArray padC = read(padCSize);
		originalData.append(padC);
		decryptor->process(padC);

		// len(IA)
		QByteArray IASizeData = read(lenSize);
		originalData.append(IASizeData);
		decryptor->process(IASizeData);
		const int IASize = Utility::Endian::fromBigEndian<quint16>(IASizeData.constData());

		// check if we have IA
		if (bytesAvailable() < IASize)
		{
			unread(originalData);
			return false;
		}

		// IA
		QByteArray IA = read(IASize);
		if (cryptoProvide.testFlag(CryptoRC4))
		{
			sendCryptoSelect(CryptoRC4);

			unread(IA);

			delete this->decryptor;
			this->decryptor = decryptor.take();
		}
		else if (cryptoProvide.testFlag(CryptoPlaintext))
		{
			sendCryptoSelect(CryptoPlaintext);

			decryptor->process(IA);
			unread(IA);

			delete this->encryptor;
			delete this->decryptor;
			this->encryptor = new PlaintextEncryptor();
			this->decryptor = new PlaintextEncryptor();
		}
		else
		{
			dropConnection(QLatin1String("Invalid crypto provide"), badPacketPenalty);
			return false;
		}
	}

	return true;
}

bool PeerWireSocket::processCryptoSelect()
{
	Q_ASSERT_X(connectionType == OutgoingConnection, Q_FUNC_INFO, "Invalid call");

	// ENCRYPT(VC)
	if (!MSEResynchronized)
	{
		if (bytesAvailable() < VCSize)	return false;
		
		QScopedPointer<RC4Encryptor> decryptor(new RC4Encryptor(*dynamic_cast<RC4Encryptor*>(this->decryptor)));

		const QByteArray &data = read(bytesAvailable());
		QByteArray VC(VCSize, 0);	decryptor->process(VC);
		const int index = data.indexOf(VC);

		if (index == -1)
		{
			if (data.size() > maxPadASize + VCSize)
				dropConnection(QLatin1String("Failed to resynchronize on VC"), badPacketPenalty);
			else
				unread(data);
			return false;
		}

		unread(data.mid(index + VCSize));

		delete this->decryptor;
		this->decryptor = decryptor.take();

		MSEResynchronized = true;
	}

	// ENCRYPT(crypto_select, len(padD), padD)
	{
		if (bytesAvailable() < cryptoSelectSize + lenSize)	return false;

		QScopedPointer<RC4Encryptor> decryptor(new RC4Encryptor(*dynamic_cast<RC4Encryptor*>(this->decryptor)));

		// keep a copy of the original data in case we need to unread
		QByteArray originalData;

		// crypto_select
		QByteArray cryptoSelectData = read(cryptoSelectSize);
		originalData.append(cryptoSelectData);
		decryptor->process(cryptoSelectData);
		const CryptoType cryptoSelect = CryptoType(Utility::Endian::fromBigEndian<quint32>(cryptoSelectData.constData()));

		// len(padD)
		QByteArray padDSizeData = read(lenSize);
		originalData.append(padDSizeData);
		decryptor->process(padDSizeData);
		const int padDSize = Utility::Endian::fromBigEndian<quint16>(padDSizeData.constData());

		// check if we have padD
		if (bytesAvailable() < padDSize)
		{
			unread(originalData);
			return false;
		}

		// padD
		QByteArray padD = read(padDSize);
		if (cryptoSelect.testFlag(CryptoRC4))
		{
			decryptor->process(padD);

			delete this->decryptor;
			this->decryptor = decryptor.take();
		}
		else if (cryptoSelect.testFlag(CryptoPlaintext))
		{
			delete this->encryptor;
			delete this->decryptor;
			this->encryptor = new PlaintextEncryptor();
			this->decryptor = new PlaintextEncryptor();
		}
		else
		{
			dropConnection(QLatin1String("Invalid crypto select"), badPacketPenalty);
			return false;
		}
	}

	return true;
}

void PeerWireSocket::processHandshake()
{
	static const int HandshakeSize = 68;
	if (bytesAvailable() < HandshakeSize)	return;

	// protocol ID
	{
		const QByteArray &IDData = read(1 + protocolIDSize);
		QByteArray ID = IDData;	decryptor->process(ID);
		if (!processProtocolID(ID))
		{
			switch (encryptionMethod)
			{
			case NoneEncryption:
				// retry with MSE
				unread(IDData, true);
				encryptionMethod = MessageStreamEncryption;
				break;
			case MessageStreamEncryption:
				dropConnection(QLatin1String("Invalid protocol ID"), badPacketPenalty);
				break;
			default:
				Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid encryption method");
				break;
			}
			return;
		}
	}

	if (!processReserved())		return;
	if (!processInfoHash())		return;
	if (!processPeerID())		return;

	if (connectionType == IncomingConnection)
		sendHandshake();

	handshaked = true;

	initializeFeatures();
	sendBitField();

	setTimer(&keepAliveTimer, keepAliveInterval);
	emit connectedToPeer();
}

bool PeerWireSocket::processProtocolID(const QByteArray &IDData)
{
	if (IDData.size() != 1 + protocolIDSize)	return false;
	if (IDData[0] != protocolIDSize)		return false;
	if (!IDData.endsWith(protocolID))		return false;

	return true;
}

bool PeerWireSocket::processReserved()
{
	QByteArray reserved = read(reservedSize);
	decryptor->process(reserved);

	if (reserved.size() != reservedSize)
	{
		dropConnection(QLatin1String("Failed to get reserved"), badPacketPenalty);
		return false;
	}

	Q_ASSERT_X(feature == 0, Q_FUNC_INFO, "");

	for (int i = 0; i < static_cast<int>(sizeof(features) / sizeof(*features)); ++i)
	{
		const QVariant &info = getFeatureInfo(features[i]);
		if (info.userType() == qMetaTypeId<int>())
		{
			const int index = info.value<int>() >> 8;
			const int mask = info.value<int>() & 0xff;
			if (static_cast<quint8>(reserved[index]) & mask)	feature |= features[i];
		}
	}

	messageLog(ReceiveDirection, QString::fromLatin1("reserved [%1]").arg(QLatin1String(reserved.toHex().constData())));

	return true;
}

bool PeerWireSocket::processInfoHash()
{
	QByteArray peerInfoHash = read(SHA1HashSize);
	decryptor->process(peerInfoHash);

	if (peerInfoHash.size() != SHA1HashSize)
	{
		dropConnection(QLatin1String("Failed to get info hash"), badPacketPenalty);
		return false;
	}

	if (connectionType == IncomingConnection)
	{
		emit infoHashReceived(peerInfoHash);
		if (!client)	return false;
	}
	else if (peerInfoHash != client->getMetaInfo()->getInfoHash())
	{
		dropConnection(QString::fromLatin1("Invalid info hash [%1]").arg(QLatin1String(peerInfoHash.toHex().constData())), badPacketPenalty);
		return false;
	}

	return true;
}

bool PeerWireSocket::processPeerID()
{
	static const int peerIDSize = 20;

	QByteArray peerIDData = read(peerIDSize);
	decryptor->process(peerIDData);
	const PeerID peerID(peerIDData);

	if (peerID.getID() == BTClient::getPeerID().getID() && connectionType == OutgoingConnection)
	{
		dropConnection(QLatin1String("Connected to ourself"), INT_MAX);
		return false;
	}

	emit peerIDReceived(peerID.getID());

	return true;
}

void PeerWireSocket::processChoke()
{
	selfState |= ChokedState;
	messageLog(ReceiveDirection, QLatin1String("choke"));

	autoCancel(0, client->getMetaInfo()->getSize());
	setTimer(&pendingRequestTimer, 0);
}

void PeerWireSocket::processUnchoke()
{
	selfState &= ~ChokedState;
	messageLog(ReceiveDirection, QLatin1String("unchoke"));

	setTimer(&pendingRequestTimer, 0);
	scheduleRequest();
}

void PeerWireSocket::processHave(int index)
{
	if (!peerBitmap.testBit(index))
	{
		peerBitmap.setBit(index);
		if (peerBitmap.count(true) == peerBitmap.size())
			peerState |= SeedingState;

		emit peerBitmapUpdated(index, true);

		autoUpdateInterested();
		scheduleRequest();
	}
}

void PeerWireSocket::processBitField(const QByteArray &bitField)
{
	for (int i = 0; i < bitField.size(); ++i)
	{
		if (!bitField[i])	continue;
		for (int bit = 0; bit < 8; ++bit)
			if (bitField[i] & (1 << (7 - bit)))
			{
				const int bitIndex = (i * 8) + bit;
				if (bitIndex < peerBitmap.size())
				{
					peerBitmap.setBit(bitIndex, true);
					emit peerBitmapUpdated(bitIndex, true);
				}
				else
					dropConnection(QLatin1String("Bad bit field received"), badPacketPenalty);
			}
	}
	if (peerBitmap.count(true))
	{
		if (peerBitmap.count(true) == peerBitmap.size())
			peerState |= SeedingState;

		autoUpdateInterested();
		scheduleRequest();
	}
}

void PeerWireSocket::processRequest(int index, int offset, int size)
{
	messageLog(ReceiveDirection, QString::fromLatin1("request [%1, %2, %3]").arg(index).arg(offset).arg(size));

	if (!checkRequest(index, offset, size))	return;

	Q_ASSERT_X(client, Q_FUNC_INFO, "invalid client");

	sendPiece(index, offset, client->readBlock(index, offset, size));
}

void PeerWireSocket::processCancel(int index, int offset, int size)
{
	messageLog(ReceiveDirection, QString::fromLatin1("cancel [%1, %2, %3]").arg(index).arg(offset).arg(size));

	const BlockPacket blockPacket(index, offset, size, QByteArray());

	for (auto iter = pendingOutgoingBlocks.begin(); iter != pendingOutgoingBlocks.end(); )
		if (*iter == blockPacket)
		{
			pendingOutgoingBlocksSize -= iter->data.size();
			iter = pendingOutgoingBlocks.erase(iter);
		}
		else
			++iter;
}

void PeerWireSocket::processPiece(int index, int offset, const QByteArray &block)
{
	// unexpected piece possiblely due to choke and unchoke messages are sent in quick succession
	// and/or transfer is going very slowly.
	if (!pendingIncomingBlocks.removeOne(BlockPacket(index, offset, block.size(), QByteArray())))	return;

	messageLog(ReceiveDirection, QString::fromLatin1("piece [%1, %2, %3]").arg(index).arg(offset).arg(block.size()));

	downloadRate->transfer(block.size());
	emit blockReceived(index, offset, block);

	setTimer(&pendingRequestTimer, pendingIncomingBlocks.isEmpty() ? 0 : pendingRequestTimeout);

	scheduleRequest();
}

bool PeerWireSocket::checkRequest(int index, int offset, int size)
{
	// check state
	if (peerState.testFlag(ChokedState))
	{
		messageLog(ReceiveDirection, QLatin1String("invalid request : choked state"));
		return false;
	}

	// check the index : in the range
	if (index < 0 || index >= client->getMetaInfo()->getPieces())
	{
		messageLog(ReceiveDirection, QString::fromLatin1("invalid request index [%1] : out of range").arg(index));
		return false;
	}

	// check the index : does the peer have it ?
	if (peerBitmap.testBit(index))
	{
		messageLog(ReceiveDirection, QString::fromLatin1("invalid request index [%1] : peer already has").arg(index));
		return false;
	}

	// check the index : do we have it ?
	if (!client->getVerified().testBit(index))
	{
		messageLog(ReceiveDirection, QString::fromLatin1("invalid request index [%1] : don't have").arg(index));
		return false;
	}

	// check the offset : in the range
	if (offset < 0 || offset >= client->getMetaInfo()->getPieceSize(index))
	{
		messageLog(ReceiveDirection, QString::fromLatin1("invalid request offset [%1] : out of range").arg(offset));
		return false;
	}

	// check the size
	if (size <= 0 || size > BlockSelector::maximalBlockSize)
	{
		messageLog(ReceiveDirection, QString::fromLatin1("invalid request size [%1] : negative or too large").arg(size));
		return false;
	}

	// check the block : (offset + size) in the range
	if (offset + size > client->getMetaInfo()->getPieceSize(index))
	{
		messageLog(ReceiveDirection, QString::fromLatin1("invalid request offset [%1] and size [%2] : out of range").arg(offset).arg(size));
		return false;
	}

	return true;
}

void PeerWireSocket::processExtendedMessage(ExtendedMessageType type, const QByteArray &data)
{
	if (!feature.testFlag(ExtensionProtocolFeature))
	{
		dropConnection(QLatin1String("Invalid extended message : Not enabled"), badPacketPenalty);
		return;
	}

	switch (type)
	{
	case ExtendedHandshakeMessage:
		processExtendedHandshake(data);
		break;
	default:
		break;
	}
}

void PeerWireSocket::processExtendedHandshake(const QByteArray &data)
{
	const Bcodec bcodec(data);

	if (!bcodec.isValid())
	{
		dropConnection(QString::fromLatin1("Failed to decode extended protocol handshake : %1")
				.arg(bcodec.getError()), badPacketPenalty);
		return;
	}

	const Bcodec::BDictionary &message = bcodec.getResult();

	if (message.contains(extensionSupportedMessagesKey))
	{
		/// \todo parse the dict
	}

	if (message.contains(extensionListenPortKey))
	{
		/// \todo add the peer
	}

	if (message.contains(extensionClientNameKey))
		emit clientNameReceived(QLatin1String(message.value(extensionClientNameKey).value<Bcodec::BString>().constData()));

	if (message.contains(extensionMaximumRequestsKey))
		peerMaximumPendingRequests = message.value(extensionMaximumRequestsKey).value<Bcodec::BInteger>();
}

/*!
 * \brief Retrieves the information of the feature
 *
 * \return reserved (index mask) or extension name
 */
QVariant PeerWireSocket::getFeatureInfo(FeatureFlag feature) const
{
	switch (feature)
	{
	case ExtensionProtocolFeature:
		return QVariant::fromValue(static_cast<int>(0x0510));
	case UTPexExtensionFeature:
		return QVariant::fromValue(QString::fromLatin1("ut_pex"));
	default:
		return QVariant();
	}
}

void PeerWireSocket::initializeFeatures()
{
	if (feature.testFlag(ExtensionProtocolFeature))	sendExtendedHandshake();
}

void PeerWireSocket::generatePrivateKey()
{
	Q_ASSERT_X(!privateKey, Q_FUNC_INFO, "Already generated");

	static const int privateKeySize = 160;

	privateKey = new Utility::Integer();
	privateKey->random(privateKeySize);
}

void PeerWireSocket::destroyPrivateKey()
{
	Q_ASSERT_X(privateKey, Q_FUNC_INFO, "Already destroyed");
	delete privateKey;
	privateKey = 0;
}

void PeerWireSocket::setUpCryptor(const QByteArray &encryptorKey, const QByteArray &decryptorKey)
{
	static const int discardedSize = 1024;

	delete encryptor;
	delete decryptor;

	const QByteArray &s = *DHSecret + client->getMetaInfo()->getInfoHash();
	encryptor = new RC4Encryptor(Utility::Cryptography::Hash::hash(encryptorKey + s, Utility::Cryptography::Hash::SHA1));
	decryptor = new RC4Encryptor(Utility::Cryptography::Hash::hash(decryptorKey + s, Utility::Cryptography::Hash::SHA1));

	// The first 1024 bytes of the RC4 output are discarded.
	QByteArray dummy(discardedSize, Qt::Uninitialized);
	encryptor->process(dummy);
	decryptor->process(dummy);
}

/*!
 * \brief The helper function to set timer
 * \param timer The timer to set
 * \param interval The interval of the timer, 0 to disable the timer
 */
inline void PeerWireSocket::setTimer(int *timer, int interval)
{
	if (*timer)	killTimer(*timer);
	*timer = interval == 0 ? 0 : startTimer(interval);
}

void PeerWireSocket::messageLog(MessageDirection direction, const QString &message)
{
	QString s;
	if (direction == SendDirection)
		s = QString::fromLatin1("Sent '%1' to %2");
	else if (direction == ReceiveDirection)
		s = QString::fromLatin1("Got '%1' from '%2");
	else
		return;
	log(this) << s.arg(message).arg(getPeerAddressPort()) << endl;
}

}

}
