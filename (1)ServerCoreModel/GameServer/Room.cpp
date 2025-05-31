#include "pch.h"
#include "Room.h"
#include "ClientSession.h"
#include "Lobby.h"

#include "FlatBufferWriter.h"
#include "FlatBufferReader.h"

#include <string>

void Room::Enter(ClientSessionRef client, bool isMasterClient)
{
	_clients.insert(client);
	client->EnterRoom(static_pointer_cast<Room>(shared_from_this()), isMasterClient);

	InformRoomInfos();
}

void Room::OnExitGame(ClientSessionRef client)
{
	_clients.erase(client);
	client->OnExitRoom();

	if (_clients.size() <= 0)
	{
		GLobby->DoAsync(&Lobby::RemoveRoom, _roomId);
	}
	else
	{
		InformRoomInfos();
	}

}

void Room::BackToLobby(ClientSessionRef client)
{
	_clients.erase(client);
	client->OnExitRoom();

	if (_clients.size() <= 0)
	{
		GLobby->DoAsync(&Lobby::RemoveRoom, _roomId);
	}
	else
	{
		InformRoomInfos();
	}

	GLobby->DoAsync(&Lobby::AcceptClient, client);
}

void Room::Test_Chat(uint64 sendingPlayersId, BYTE* buffer, int32 len)
{
	FlatBufferReader fbr(buffer, len);

	SendBufferRef sendBuffer = GSendBufferManager->MakeSendBuff();
	{
		FlatBufferWriter fbw(sendBuffer, S_TEST_CHAT);
		wstring recvWstr = fbr.ReadWString();
		
		wstring sendWstr = L"PlayerId(" + std::to_wstring(sendingPlayersId) + L")	:	" + recvWstr;

		fbw.WriteWString(sendWstr);
	}

	for (ClientSessionRef client : _clients)
	{
		client->Send(sendBuffer);
	}

}

void Room::CastAll(SendBufferRef sendBuffer)
{
	for (ClientSessionRef client : _clients)
	{
		client->Send(sendBuffer);
	}
}

void Room::InformRoomInfos()
{
	SendBufferRef sendBuffer = GSendBufferManager->MakeSendBuff();
	{
		FlatBufferWriter fbw(sendBuffer, S_INFORM_ROOM_INFOS);

		fbw.Write(static_cast<uint32>(_clients.size()));
	}

	CastAll(sendBuffer);
}

 