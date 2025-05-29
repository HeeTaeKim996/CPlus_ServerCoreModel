#pragma once

#include "Session.h"

class ClientSession : public PacketSession
{
	friend class ClientPacketHandler;

public:
	~ClientSession()
	{
		cout << "~ClientSession" << endl;
	}

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

public:

	void SetPlayerId(uint64 playerId) { _playerId = playerId; }
	uint64 PlayerId() { return _playerId; }


	inline void EnterRoom(RoomRef room, bool isMasterClient) 
	{
		cout << "ClientSession.h__DEBUG : EnterRoom Check " << endl;
		_room = room;
		_isMasterClient = isMasterClient;
	}
	const RoomRef& Room() { return _room.lock(); }

	void OnExitRoom();


private:
	uint64 _playerId = 0;
	bool _isMasterClient;
	weak_ptr<class Room> _room;
};
