#pragma once

#include <functional>

using ClientHandlerFunc = std::function<bool(const ClientSessionRef&, BYTE*, int32)>;
extern ClientHandlerFunc GClientHandler[UINT16_MAX];





class ClientPacketHandler
{
public:
	static void Init()
	{
		for (int i = 0; i < UINT16_MAX; i++)
		{
			GClientHandler[i] = Handle_INVALID;
		}

		GClientHandler[C_ENTER_GAME] = Handle_ENTER_GAME;
		GClientHandler[C_MAKE_ROOM] = Handle_MAKE_ROOM;
		GClientHandler[C_JOIN_ROOM] = Handle_JOIN_ROOM;
		GClientHandler[C_BACK_TO_LOBBY] = Handle_BACK_TO_LOBBY;
		GClientHandler[C_TEST_CHAT] = Handle_TEST_CHAT;
		GClientHandler[C_KEEP_ALIVE] = Handle_KEPP_ALIVE;
	}

	inline static bool HandlePacket(const ClientSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GClientHandler[header->id](session, buffer, len);
	}



private:
	static bool Handle_INVALID(const ClientSessionRef& session, BYTE* buffer, int32 len);
	static bool Handle_ENTER_GAME(const ClientSessionRef& session, BYTE* buffer, int32 len);
	static bool Handle_MAKE_ROOM(const ClientSessionRef& session, BYTE* buffer, int32 len);
	static bool Handle_JOIN_ROOM(const ClientSessionRef& session, BYTE* buffer, int32 len);
	static bool Handle_BACK_TO_LOBBY(const ClientSessionRef& session, BYTE* buffer, int32 len);
	static bool Handle_TEST_CHAT(const ClientSessionRef& session, BYTE* buffer, int32 len);
	static bool Handle_KEPP_ALIVE(const ClientSessionRef& session, BYTE* buffer, int32 len);
};	

