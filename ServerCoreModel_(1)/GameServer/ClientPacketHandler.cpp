#include "pch.h"
#include "ClientPacketHandler.h"

#include "ClientSession.h"

#include "FlatBufferReader.h"
#include "FlatBufferWriter.h"

#include "Room.h"
#include "Lobby.h"

ClientHandlerFunc GClientHandler[UINT16_MAX];

static atomic<uint64> tempPlayerIdSetter = 1; // TEMP


/*

bool ClientPacketHandler::Handle_C_TEST(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	FlatBufferReader fbr(buffer, len);

	int16 firstInt = fbr.Read<int16>();
	int32 secondInt = fbr.Read<int32>();
	int64 thirdInt = fbr.Read<int64>();


	cout << "Recv : " << firstInt << endl;
	cout << "Recv : " << secondInt << endl;
	cout << "Recv : " << thirdInt << endl;

	TempClass* tempClass = xnew<TempClass>(0, 0, 0);
	fbr.Read(tempClass);

	cout << "Recv : " << tempClass->one << endl;
	cout << "Recv : " << tempClass->two << endl;
	cout << "Recv : " << tempClass->three << endl;


	TempClass* secondClass = fbr.ZeroCopy<TempClass>();

	cout << "Recv : " << secondClass->one << endl;
	cout << "Recv : " << secondClass->two << endl;
	cout << "Recv : " << secondClass->three << endl;

	uint16 smallHeader = fbr.Read<uint16>();

	int32 ints[3];
	for (int i = 0; i < smallHeader / sizeof(uint32); i++)
	{
		ints[i] = fbr.Read<int32>();
		cout << "Recv : " << ints[i] << endl;
	}

	::wstring wstr = fbr.ReadWString();
	wcout << L"Recv L " << wstr << endl;


	SendBufferRef sendBuffer = GSendBufferManager->MakeSendBuff();
	{
		FlatBufferWriter fbw(sendBuffer, 18);

		fbw.Write(firstInt);
		fbw.Write(secondInt);
		fbw.Write(thirdInt);

		TempClass* newTempClass = xnew<TempClass>(tempClass->one, tempClass->two, tempClass->three);
		fbw.Write(newTempClass);
		xdelete(newTempClass);

		TempClass* thirdClass = fbw.Insert_ZeroCopy<TempClass>(secondClass->one, secondClass->two,
			secondClass->three);

		fbw.Write(smallHeader);

		for (int i = 0; i < smallHeader / sizeof(int32); i++)
		{
			fbw.Write(ints[i]);
		}


		fbw.WriteWString(wstr);
	}

	xdelete(tempClass);

	session->Send(sendBuffer);

	return true;
}

bool ClientPacketHandler::Handle_C_LOGIN(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	ClientSessionRef clientSession = static_pointer_cast<ClientSession>(session);

	// pkt ���� �ڵ�.. �켱 ����

	// TODO : Valiation üũ (���� �������� �����ؼ� �Ѵ���)

	static Atomic<uint64> idGenerator = 1; // DB ���̵� �ƴ�, �ΰ��� ���̵�

	{
		// pkt ���� �ڵ�.. �켱 ���� ( DB ���� �����̱⵵ ��)

		PlayerRef playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator++;
		playerRef->name = "PKT�� DB�� �����Ͽ� ���� �� �̸�";
		playerRef->type = PlayerType::PLAYER_TYPE_NONE;// "PKT���� ���� �� PlayerType ����"
		playerRef->ownerSesison = clientSession;

		clientSession->_players.push_back(playerRef);
	}

	{
		// pkt ���� �ڵ�.. �켱 ���� ( DB ���� �����̱⵵ ��)

		PlayerRef playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator++;
		playerRef->name = "PKT�� DB�� �����Ͽ� ���� �� �̸�2";
		playerRef->type = PlayerType::PLAYER_TYPE_MAGE;// "PKT���� ���� �� PlayerType ����"
		playerRef->ownerSesison = clientSession;

		clientSession->_players.push_back(playerRef);
	}

	// �� ClientSession �� Player Ŭ������ ���� �� ������ ��ó�� �÷��̾ ĳ���͸� �� �� �ְ� �ϴ�
	//   �ǵ��ε�

	auto sendBuffer = GSendBufferManager->MakeSendBuff();
	{
		FlatBufferWriter fbr(sendBuffer, S_LOGIN);

		fbr.Write(true); // �α��� ���� ���� 
	}
	session->Send(sendBuffer);

	return true;
}

bool ClientPacketHandler::Handle_C_ENTER_GAME(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	FlatBufferReader fbr(buffer, len);

	ClientSessionRef clientSession = static_pointer_cast<ClientSession>(session);

	clientSession->_currentPlayer = clientSession->_players[fbr.Read<uint64>()];
	clientSession->_room = GRoom;

	if (auto room = clientSession->_room.lock())
	{
		room->DoAsync(&Room::Enter, clientSession->_currentPlayer);
	}


	SendBufferRef sendBuffer = GSendBufferManager->MakeSendBuff();
	{
		FlatBufferWriter fbw(sendBuffer, S_ENTER_GAME);
		fbw.Write<bool>(true); // ���� Ȯ��
	}

	clientSession->_currentPlayer->ownerSesison->Send(sendBuffer);

	return true;
}

bool ClientPacketHandler::Handle_C_CHAT(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	FlatBufferReader fbr(buffer, len);

	::wstring wstr = fbr.ReadWString();
	wcout << wstr << endl;

	SendBufferRef sendBuffer = GSendBufferManager->MakeSendBuff();
	{
		FlatBufferWriter fbw(sendBuffer, S_CHAT);
		fbw.WriteWString(wstr);
	}

	GRoom->DoAsync(&Room::Broadcast, sendBuffer);

	return true;
}
*/
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




