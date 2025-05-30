#include "pch.h"
#include "ClientSession.h"
#include "ClientSessionManager.h"

#include "ClientPacketHandler.h"
#include "Room.h"
#include "Lobby.h"

void ClientSession::OnConnected()
{
	GSessionManager->Add(static_pointer_cast<ClientSession>(shared_from_this()));
}

void ClientSession::OnDisconnected()
{
	GSessionManager->Remove(static_pointer_cast<ClientSession>(shared_from_this()));
}

void ClientSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	ClientPacketHandler::HandlePacket(static_pointer_cast<ClientSession>(shared_from_this()), buffer, len);
}

void ClientSession::OnSend(int32 len)
{
	{ // DEBUG
		//cout << "OnSend Len = " << len << endl;
	}
}



void ClientSession::OnExitRoom()
{
	_room.reset();
}