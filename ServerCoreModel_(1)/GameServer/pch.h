#pragma once
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#endif

#include"CorePch.h"

using ClientSessionRef = shared_ptr<class ClientSession>;
using PlayerRef = shared_ptr<class Player>;  
using RoomRef = shared_ptr<class Room>;
// ClientSessionRef <-> PlayerRef 가 서로 참조하는 순환 참조이니, RefCycle 을 끊어주는 작업 필요