#include "pch.h"
#include "ServerPacketHandler.h"

#include "FlatBufferReader.h"
#include "FlatBufferWriter.h"


PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool ServerPacketHandler::Handle_S_TEST(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	FlatBufferReader fbr(buffer, len);

	int16 firstInt = fbr.Read<int16>();
	int32 secondInt = fbr.Read<int32>();
	int64 thirdInt = fbr.Read<int64>();


	cout << "FirstInt : " << firstInt << endl;
	cout << "SecondInt : " << secondInt << endl;
	cout << "ThirdInt : " << thirdInt << endl;


	TempClass* tempClass = xnew<TempClass>(0, 0, 0);
	fbr.Read(tempClass);

	cout << "Recv : " << tempClass->one << endl;
	cout << "Recv : " << tempClass->two << endl;
	cout << "Recv : " << tempClass->three << endl;


	TempClass* secondClass = fbr.ZeroCopy<TempClass>();
	cout << "Recv : " << secondClass->one << endl;
	cout << "Recv : " << secondClass->two << endl;
	cout << "Recv : " << secondClass->three << endl;


	SendBufferRef sendBuffer = GSendBufferManager->MakeSendBuff();
	{
		FlatBufferWriter fbw(sendBuffer, 18);

		fbw.Write(firstInt);
		fbw.Write(secondInt);
		fbw.Write(thirdInt);

		TempClass* newTempClass = xnew<TempClass>(tempClass->one, tempClass->two, tempClass->three);
		fbw.Write(newTempClass);
		xdelete(tempClass);
		xdelete(newTempClass);

		TempClass* thirdClass = fbw.Insert_ZeroCopy<TempClass>(secondClass->one, secondClass->two,
			secondClass->three);


		uint16 smallHeader = fbr.Read<uint16>();

		fbw.Write(smallHeader);
		fbw.CopyFBR(fbr, smallHeader);


		::wstring wstr = fbr.ReadWString();
		wcout << L"Recv : " << wstr << endl;

		fbw.WriteWString(wstr);
	}


	this_thread::sleep_for(10s);
	session->Send(sendBuffer);

	return true;
}


bool ServerPacketHandler::Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	// TODO : log

	CRASH("HANDLE_INVALID");

	return false;
}



bool ServerPacketHandler::Handle_ENTER_GAME(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	// TODO

	return true;
}

bool ServerPacketHandler::Handle_INFORM_LOBBY_INFOS(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	FlatBufferReader fbr(buffer, len);

	uint32 roomCount = fbr.Read<uint32>();

	cout << "Room Count : " << roomCount << endl;

	cout << "----------------------------------------" << endl;
	for (int i = 0; i < roomCount; i++)
	{
		cout << "Room ID : " << fbr.Read<uint64>() << endl;
	}

	cout << "----------------------------------------" << endl;

	return true;
}	

bool ServerPacketHandler::Handle_INFORM_ROOM_INFOS(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	FlatBufferReader fbr(buffer, len);

	cout << "Players In Room : " << fbr.Read<uint32>() << endl;


	return true;
}

bool ServerPacketHandler::Handle_TEST_CHAT(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	FlatBufferReader fbr(buffer, len);
	wcout << "Received Wstr : " << fbr.ReadWString() << endl;

	return true;

}



