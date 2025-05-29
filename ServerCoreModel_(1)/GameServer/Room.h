#pragma once

#include "JobQueue.h"

class Room : public JobQueue
{
public:
	Room(uint64 roomId) : _roomId(roomId) {}
	

public:
	void Enter(ClientSessionRef client, bool isMasterclient);
	void OnExitGame(ClientSessionRef client);
	void BackToLobby(ClientSessionRef	clientSession);


	void Test_Chat(uint64 sendingPlayersId, BYTE* buffer, int32 len);


private:
	void InformRoomInfos();

public:
	uint64 PlayersCount() { return _clients.size(); }

private:
	void CastAll(SendBufferRef sendBuffer);

private:
	uint64 _roomId;
	Set<ClientSessionRef> _clients;
};