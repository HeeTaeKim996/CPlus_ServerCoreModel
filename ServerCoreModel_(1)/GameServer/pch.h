#pragma once
#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#endif

#include"CorePch.h"

using ClientSessionRef = shared_ptr<class ClientSession>;
using PlayerRef = shared_ptr<class Player>;  
using RoomRef = shared_ptr<class Room>;
// ClientSessionRef <-> PlayerRef �� ���� �����ϴ� ��ȯ �����̴�, RefCycle �� �����ִ� �۾� �ʿ�