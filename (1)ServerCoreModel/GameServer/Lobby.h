#pragma once


class Lobby : public JobQueue
{
public:
	static void Init();


public:
	Lobby();

	void AcceptClient(ClientSessionRef client);
	void RemoveClient(ClientSessionRef client);
	void MakeRoom(ClientSessionRef client);
	void JoinRoom(ClientSessionRef client, uint64 roomId);
	void RemoveRoom(uint64 roomId);



public:
	void OnClientDisconnected(ClientSessionRef client);

public:
	uint64 GetLobbysPlayersCount() { return _clients.size(); }
	uint64 GetAllRoomsPlayersCount();
	uint32 RoomCount() { return _rooms.size(); }


private:
	void InformLobbyInfos();

private:
	Set<ClientSessionRef> _clients;
	Map<uint64, RoomRef> _rooms;
};

extern shared_ptr<Lobby> GLobby;

