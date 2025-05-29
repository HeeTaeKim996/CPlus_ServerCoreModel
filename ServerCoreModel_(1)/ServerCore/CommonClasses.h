#pragma once

class TempClass
{
public:
	TempClass(int16 first, int32 second, int64 third) :one(first), two(second), three(third) {}
public:
	int16 one;
	int32 two;
	int64 three;
};


enum : uint16
{
	C_TEST,

	C_ENTER_GAME,

	C_MAKE_ROOM,

	C_JOIN_ROOM,

	C_BACK_TO_LOBBY,

	C_TEST_CHAT,
};

enum : uint16
{
	S_TEST,

	S_ENTER_GAME,
	
	S_INFORM_LOBBY_INFOS,

	S_INFORM_ROOM_INFOS,

	S_TEST_CHAT,
};

enum PlayerType : uint16
{
	PLAYER_TYPE_NONE = 0,
	PLAYER_TYPE_MAGE = 1,
};
