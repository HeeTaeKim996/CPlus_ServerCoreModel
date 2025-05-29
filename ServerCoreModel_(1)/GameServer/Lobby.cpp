#include "pch.h"
#include "Lobby.h"

#include "ClientSession.h"
#include "Room.h"

#include "FlatBufferWriter.h";
#include "ClientSessionManager.h"

shared_ptr<Lobby> GLobby = nullptr;

void Lobby::Init()
{
	GLobby = MakeShared<Lobby>();
}

void Lobby::AcceptClient(ClientSessionRef client)
{
	_clients.insert(client);

	InformLobbyInfos();
}

void Lobby::RemoveClient(ClientSessionRef client)
{
	_clients.erase(client); // ※ _clients.find(client) == _clients.end() 라 해도, 오류 안남

	InformLobbyInfos();
}



void Lobby::MakeRoom(ClientSessionRef client)
{
	uint64 mastersId = client->PlayerId();
	if (_rooms.find(mastersId) == _rooms.end())
	{
		_rooms[mastersId] = ObjectPool<Room>::MakeShared(mastersId);
		_rooms[mastersId]->Enter(client, true);

		_clients.erase(client); // EXIT_LOBBY
	}


	InformLobbyInfos();
}

void Lobby::JoinRoom(ClientSessionRef client, uint64 roomId)
{
	if (_rooms.find(roomId) != _rooms.end())
	{
		_rooms[roomId]->Enter(client, false);
	}

	_clients.erase(client); // EXIT_LOBBY
}






void Lobby::RemoveRoom(uint64 roomId)
{
	_rooms.erase(roomId);

	InformLobbyInfos();
}

void Lobby::OnClientDisconnected(ClientSessionRef client)
{

}



uint64 Lobby::GetAllRoomsPlayersCount()
{
	uint64 sum = 0;

	for (pair<uint64, RoomRef> room : _rooms)
	{
		sum += room.second->PlayersCount();
	}

	return sum;
}

void Lobby::InformLobbyInfos()
{
	SendBufferRef sendBuffer = GSendBufferManager->MakeSendBuff();
	{
		FlatBufferWriter fbw(sendBuffer, S_INFORM_LOBBY_INFOS);

		fbw.Write(static_cast<uint32>(_rooms.size()));

		for (pair<uint64, RoomRef> room : _rooms)
		{
			fbw.Write(static_cast<uint64>(room.first));
		}
	}

	for (ClientSessionRef client : _clients)
	{
		client->Send(sendBuffer);

		cout << "Lobby.cpp__DEBUG : Inform Lobby Infos Check" << endl;
	}
}
