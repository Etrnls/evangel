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
#ifndef PROTOCOL_BITTORRENT_PEERWIRESOCKET_H
#define PROTOCOL_BITTORRENT_PEERWIRESOCKET_H

#include "protocol/bt/btclient.h"
#include "network/tcpsocket.h"
#include "util/bitmap/bitmap.h"
#include "util/cipher/rc4.h"

namespace Utility
{
class Integer;
}

namespace Network
{
class RateStatistics;
}

namespace Protocol
{

namespace BitTorrent
{

/*!
 * \author Etrnls <Etrnls@gmail.com>
 *
 */
class PeerWireSocket : public Network::TCPSocket
{
	Q_OBJECT
	Q_CLASSINFO("log", "BitTorrent")
public:
	enum ConnectionType
	{
		IncomingConnection,
		OutgoingConnection
	};

	enum EncryptionMethod
	{
		NoneEncryption,
		MessageStreamEncryption
	};

	enum PeerStateFlag
	{
		ChokedState	= 0x01,	//!< Choked or Unchoked state
		InterestedState	= 0x02,	//!< Interested or NotInterested state
		SeedingState	= 0x04	//!< Seeding or Leeching state
	};
	Q_DECLARE_FLAGS(PeerState, PeerStateFlag)

	enum FeatureFlag
	{
		//DHTProtocolFeature		= 0x01,
		ExtensionProtocolFeature	= 0x02,
		//FastExtensionFeature		= 0x04,
		UTPexExtensionFeature		= 0x08
	};
	Q_DECLARE_FLAGS(Feature, FeatureFlag)

	explicit PeerWireSocket(EncryptionMethod encryptionMethod, ConnectionType connectionType);
	virtual ~PeerWireSocket();

	ConnectionType getConnectionType() const
	{
		return connectionType;
	}

	void setClient(BTClient *client);

	inline PeerState getSelfState() const	{	return selfState;	}
	inline PeerState getPeerState() const	{	return peerState;	}

	qreal getDownloadRate() const;
	qreal getUploadRate() const;

	int getConnectionTime() const;

	// traffic control
	virtual bool canUpload() const;
	virtual int writeToNetwork(int maxSize = INT_MAX);

	// protocol
	void scheduleRequest();

	void sendChoke();
	void sendUnchoke();

	void sendHave(int index);

	void dropConnection(const QString &reason, int seriousness);
signals:
	void infoHashReceived(const QByteArray &infoHash) const;

	void peerIDReceived(const QByteArray &ID) const;
	void clientNameReceived(const QString &name) const;

	void peerBitmapUpdated(int index, bool flag) const;

	void blockRequested(int index, int offset, int size) const;
	void blockCanceled(int index, int offset, int size) const;
	void blockReceived(int index, int offset, const QByteArray &block) const;
	void blockSent(int index, int offset, int size) const;

	void connectedToPeer() const; //!< This is emitted AFTER HANDSHAKE SUCCEEDED
	void disconnectedFromPeer() const;

	void drop(const QString &reason, int seriousness) const;
private slots:
	void clientStateChanged();

	void processConnected();
	void processDisconnected();
	void processError();
private:
	struct BlockPacket
	{
		int index;
		int offset;
		int size; //!< \note This is NOT the same as data.size() as data may contain message header
		QByteArray data;

		inline BlockPacket(int index, int offset, int size, const QByteArray &data)
			: index(index), offset(offset), size(size), data(data) {}

		inline bool operator==(const BlockPacket &other) const
		{
			return index == other.index && offset == other.offset && size == other.size;
		}
	};

	class Encryptor
	{
	public:
		virtual void process(QByteArray &data) = 0;
	};
	class PlaintextEncryptor : public Encryptor
	{
	public:
		virtual void process(QByteArray &data)
		{
			Q_UNUSED(data);
		}
	};
	class RC4Encryptor : public Encryptor
	{
	public:
		explicit RC4Encryptor(const QByteArray &key) : RC4(key) {}

		virtual void process(QByteArray &data)
		{
			RC4.process(data.data(), data.size());
		}
	private:
		Utility::Cryptography::Cipher::RC4 RC4;
	};

	enum CryptoTypeFlag
	{
		CryptoPlaintext = 0x01,
		CryptoRC4 = 0x02
	};
	Q_DECLARE_FLAGS(CryptoType, CryptoTypeFlag)

	enum MessageType
	{
		ChokeMessage		= 0,
		UnchokeMessage		= 1,
		InterestedMessage	= 2,
		NotInterestedMessage	= 3,
		HaveMessage		= 4,
		BitFieldMessage		= 5,
		RequestMessage		= 6,
		PieceMessage		= 7,
		CancelMessage		= 8,
		ExtendedMessage		= 20
	};

	enum ExtendedMessageType
	{
		ExtendedHandshakeMessage	= 0
	};

	enum MessageDirection
	{
		SendDirection,
		ReceiveDirection
	};

	static const int SHA1HashSize = 20;

	static const char protocolIDSize;
	static const char protocolID[];

	static const int reservedSize = 8;

	static const FeatureFlag features[]; //!< All supported features

	static const int keepAliveInterval     = 30 * 1000;     //!< half minute for keep alive
	static const int clientTimeOut         = 2 * 60 * 1000; //!< 2 minutes for client time out
	static const int pendingUnchokeTimeout = 5 * 60 * 1000; //!< 5 minutes for the peer to unchoke us
	static const int pendingRequestTimeout = 1 * 60 * 1000; //!< 1 minute for request time out

	static const int timeOutPenalty = 5000;
	static const int badPacketPenalty = 3000;

	static const int minimumPendingRequests;
	static const int maximumPendingRequests;
	static const int maxPendingOutgoingBlocksSize;

	// keys for extension protocol handshake
	static const char extensionSupportedMessagesKey[];
	static const char extensionListenPortKey[];
	static const char extensionClientNameKey[];
	static const char extensionMaximumRequestsKey[];

	// message stream encryption
	static const Utility::Integer DHPrime;
	static const int DHKeySize = 96;
	static const int maxPadASize = 512;	// PadA: Random data with a random length of 0 to 512 bytes each
	static const int maxPadBSize = 512;	// PadB: Random data with a random length of 0 to 512 bytes each
	static const int maxPadCSize = 512;
	static const int VCSize = 8;		// VC: String of 8 bytes set to 0x00
	static const int cryptoProvideSize = 4;	// crypto_provide: 32bit bitfields
	static const int cryptoSelectSize = 4;	// crypto_select: 32bit bitfields
	static const int lenSize = 2;

	virtual void processIncomingData();
	virtual void timerEvent(QTimerEvent *event);

	// ---------- Basic Behavior ---------
	Q_INVOKABLE void autoRequest();
	Q_INVOKABLE void autoCancel(qint64 offset, qint64 size);
	// ------------------------------------

	// ---------- Basic Message ----------
	void sendMessage(MessageType type, const QByteArray &data = QByteArray());

	void sendPublicKey();
	void sendCryptoProvide();
	void sendCryptoSelect(CryptoType cryptoSelect);

	void sendHandshake();

	void sendKeepAlive();

	void sendInterested();
	void sendNotInterested();
	void autoUpdateInterested();

	void sendBitField();

	void sendRequest(int index, int offset, int size);
	void sendPiece(int index, int offset, const QByteArray &block);
	void sendCancel(int index, int offset, int size);
	// ------------------------------------

	// --------- Extended Message ---------
	void sendExtended(ExtendedMessageType type, QByteArray data);
	void sendExtendedHandshake();
	// ------------------------------------

	// ---------- Process Helper ----------
	int getPacketSize();
	void parsePacket(const QByteArray &packet);
	// ------------------------------------

	// --------- Process Hand Shake ---------
	void processMSEHandshake();
	bool processPublicKey();
	bool processCryptoProvide();
	bool processCryptoSelect();

	void processHandshake();
	bool processProtocolID(const QByteArray &IDData);
	bool processReserved();
	bool processInfoHash();
	bool processPeerID();
	// ------------------------------------

	// ------ Process Basic Message -------
	void processChoke();
	void processUnchoke();

	void processHave(int index);
	void processBitField(const QByteArray &bitField);

	void processRequest(int index, int offset, int size);
	void processCancel(int index, int offset, int size);
	void processPiece(int index, int offset, const QByteArray &block);

	bool checkRequest(int index, int offset, int size);
	// ------------------------------------
	
	// ----- Process Extended Message -----
	void processExtendedMessage(ExtendedMessageType type, const QByteArray &data);
	void processExtendedHandshake(const QByteArray &data);
	// ------------------------------------

	// ------------- Features -------------
	QVariant getFeatureInfo(FeatureFlag feature) const;
	void initializeFeatures();
	// ------------------------------------

	// ----- Message Stream Encryption ----
	void generatePrivateKey();
	void destroyPrivateKey();
	void setUpCryptor(const QByteArray &encryptorKey, const QByteArray &decryptorKey);
	// ------------------------------------

	// ----------- Other Helper -----------
	void setTimer(int *timer, int interval);

	void messageLog(MessageDirection direction, const QString &message);
	// ------------------------------------

	const ConnectionType connectionType;
	EncryptionMethod encryptionMethod;

	const BTClient *client;

	Feature feature;

	// message stream encryption
	Utility::Integer *privateKey;
	QByteArray *DHSecret;
	Encryptor *encryptor;
	Encryptor *decryptor;

	// peer bitmap
	QBitArray peerBitmap;

	// states
	PeerState selfState;
	PeerState peerState;

	// for process incoming data
	int packetSize;

	// rate of incoming / outgoing blocks
	Network::RateStatistics *downloadRate;
	Network::RateStatistics *uploadRate;

	// incoming blocks
	int peerMaximumPendingRequests;
	int desiredPendingIncomingBlocks;
	QList<BlockPacket> pendingIncomingBlocks;

	// outgoing blocks
	int pendingOutgoingBlocksSize;
	QList<BlockPacket> pendingOutgoingBlocks;

	// timers
	int keepAliveTimer;	//!< Sends keep alive
	int timeOutTimer;	//!< Checks keep alive (or any other) message
	int pendingRequestTimer;

	bool invalidateTimeOutTimer;
	bool invalidateKeepAliveTimer;

	// flags
	bool handshaked;
	bool DHExchanged;
	bool MSEResynchronized;
	bool MSECryptoSelected;
	bool requestScheduled;
};

}

}

#endif
