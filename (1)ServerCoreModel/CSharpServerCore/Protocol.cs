#pragma once

using System;

public enum C : UInt16
{
    C_TEST,

    C_ENTER_GAME,

    C_MAKE_ROOM,

    C_JOIN_ROOM,

    C_BACK_TO_LOBBY,

    C_TEST_CHAT,

    C_KEEP_ALIVE
};

public enum S : UInt16
{
    S_TEST,

    S_ENTER_GAME,

    S_INFORM_LOBBY_INFOS,

    S_INFORM_ROOM_INFOS,

    S_TEST_CHAT,
};

