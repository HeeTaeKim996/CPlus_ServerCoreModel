#include "pch.h"
#include "ClientPacketHandler.h"

#include "ClientSession.h"

#include "FlatBufferReader.h"
#include "FlatBufferWriter.h"

#include "Room.h"
#include "Lobby.h"

ClientHandlerFunc GClientHandler[UINT16_MAX];

static atomic<uint64> tempPlayerIdSetter = 1; // TEMP


bool ClientPacketHandler::Handle_INVALID(const ClientSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	
	// TODO : log
#ifdef _DEBUG
	cout << header->id << endl;

	CRASH("HANDLE_INVALID");
#endif 



	return false;
}

bool ClientPacketHandler::Handle_ENTER_GAME(const ClientSessionRef& session, BYTE* buffer, int32 len)
{
	session->SetPlayerId(tempPlayerIdSetter.fetch_add(1));

	GLobby->DoAsync(&Lobby::AcceptClient, session);

	return true;
}

bool ClientPacketHandler::Handle_MAKE_ROOM(const ClientSessionRef& session, BYTE* buffer, int32 len)
{
	GLobby->DoAsync(&Lobby::MakeRoom, session);

	return true;
}

bool ClientPacketHandler::Handle_JOIN_ROOM(const ClientSessionRef& session, BYTE* buffer, int32 len)
{
	FlatBufferReader fbr(buffer, len);

	uint64 roomId = fbr.Read<uint64>();


	GLobby->DoAsync(&Lobby::JoinRoom, session, roomId);

	return true;
}

bool ClientPacketHandler::Handle_BACK_TO_LOBBY(const ClientSessionRef& session, BYTE* buffer, int32 len)
{
	if (RoomRef room = session->Room())
	{
		room->BackToLobby(session);
	}
	
	return true;
}

bool ClientPacketHandler::Handle_TEST_CHAT(const ClientSessionRef& session, BYTE* buffer, int32 len)
{
	if (const RoomRef& room = session->Room())
	{
		room->DoAsync(&Room::Test_Chat, session->PlayerId(), buffer, len);
	}

	return true;
}

bool ClientPacketHandler::Handle_KEPP_ALIVE(const ClientSessionRef& session, BYTE* buffer, int32 len)
{
	return true;
}




