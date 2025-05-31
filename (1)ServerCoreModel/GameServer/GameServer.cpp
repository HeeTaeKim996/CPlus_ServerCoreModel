#include"pch.h"

#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "ClientSession.h"

#include "ClientSessionManager.h"

#include "FlatBufferWriter.h"

#include "ClientPacketHandler.h"

#include "Job.h"

#include "Room.h"

#include "DBConnectionPool.h"

#include "DBBind.h"

#include "ObjectPool.h"

#include "Lobby.h"

enum : uint64
{
	GLOBAL_QUEUE_MONITORING_TICK = 64,
	// 이 값을 하드코딩이 아닌, 처리하는 Job의 시간이 길어질수록, 이 값을 더 높게, 자동 보정하는 코드를
	// 만들 수도 있음
};

void SampleForMsServerDB()
{
	ASSERT_CRASH(GDBConnectionPool->Connect(1,
		L"Driver={ODBC Driver 17 for SQL Server};Server=(localdb)\\MSSQLLocalDB;Database=ServerDB;Trusted_Connection=yes;"
	));
	// ※ 위 주소 양식은, https://www.connectionstrings.com/sql-server/ 의 Microsoft ODBC Driver 17 for SQL Server 의,
	//   Trusted Connection 양식

	// CrateTable
	{
		auto query = L"									\
			DROP TABLE IF EXISTS[dbo].[GOLD];			\
			CREATE TABLE[dbo].[GOLD]					\
			(											\
				[id] INT NOT NULL PRIMARY KEY IDENTITY,	\
				[gold] INT NULL,						\
				[name] NVARCHAR(50) NULL,				\
				[createDate] DATETIME NULL				\
				);";

		DBConnection* dbConn = GDBConnectionPool->Pop();
		ASSERT_CRASH(dbConn->Execute(query));

		GDBConnectionPool->Push(dbConn);
	}

	// Add Data
	for (int32 i = 0; i < 3; i++)
	{
		DBConnection* dbConn = GDBConnectionPool->Pop();

		DBBind<3, 0> dbBind(*dbConn, L"INSERT INTO [dbo].[GOLD]([gold], [name], [createDate]) VALUES(?, ?, ?)");

		int32 gold = 100;
		WCHAR name[100] = L"루카스";
		TIMESTAMP_STRUCT ts = { 2021, 6, 5 };

		dbBind.BindParam(0, gold);
		dbBind.BindParam(1, name);
		dbBind.BindParam(2, ts);

		ASSERT_CRASH(dbBind.Execute());

		/*
		// 기존에 바인딩된 정보를 날림
		dbConn->Unbind();

		// 넘길 인자를 바인딩
		int32 gold = 100;
		SQLLEN len = 0;

		WCHAR name[] = L"루키스";
		SQLLEN nameLen = 0;

		TIMESTAMP_STRUCT ts = {};
		ts.year = 2021;
		ts.month = 6;
		ts.day = 5;
		SQLLEN tsLen = 0;

		// 넘길 인자를 바인딩
		ASSERT_CRASH(dbConn->BindParam(1, &gold, &len));
		ASSERT_CRASH(dbConn->BindParam(2, name, &nameLen));
		ASSERT_CRASH(dbConn->BindParam(3, &ts, &tsLen));

		//SQL 실행
		ASSERT_CRASH(dbConn->Execute(L"INSERT INTO [dbo].[GOLD]([gold], [name], [createDate]) VALUES(?, ?, ?)"));
		*/

		GDBConnectionPool->Push(dbConn);
	}

	// Read	
	{
		DBConnection* dbConn = GDBConnectionPool->Pop();

		DBBind<1, 4> dbBind(*dbConn, L"SELECT id, gold, name, createDate FROM [dbo].[Gold] WHERE gold = (?)");

		int32 gold = 100;
		dbBind.BindParam(0, gold);

		int32 outId = 0;
		int32 outGold = 0;
		WCHAR outName[100];
		TIMESTAMP_STRUCT outDate = {};

		dbBind.BindCol(0, outId);
		dbBind.BindCol(1, outGold);
		dbBind.BindCol(2, outName);
		dbBind.BindCol(3, outDate);

		ASSERT_CRASH(dbBind.Execute());

		/*
		// 기존 바인딩된 정보를 날림
		dbConn->Unbind();


		int32 gold = 100;
		SQLLEN len = 0;
		//넘길 인자 바인딩
		dbConn->BindParam(1, &gold, &len);

		int32 outId = 0;
		SQLLEN outIdLen = 0;
		// 받아올 인자 바인딩
		ASSERT_CRASH(dbConn->BindCol(1,  &outId, &outIdLen));

		int32 outGold = 0;
		SQLLEN outGoldLen = 0;
		// 받아올 인자 바인딩
		ASSERT_CRASH(dbConn->BindCol(2, &outGold, &outGoldLen));

		WCHAR outName[100];
		SQLLEN outNameLen = 0;
		ASSERT_CRASH(dbConn->BindCol(3, outName, len32(outName), &outNameLen));

		TIMESTAMP_STRUCT outDate = {};
		SQLLEN outDateLen = 0;
		ASSERT_CRASH(dbConn->BindCol(4, &outDate, &outDateLen));

		// SQL 실행
		ASSERT_CRASH(dbConn->Execute(L"SELECT id, gold, name, createDate FROM [dbo].[GOLD] WHERE gold = (?)"));
		*/



		wcout.imbue(locale("kor"));
		while (dbConn->Fetch())
		{
			cout << "ID : " << outId << "Gold : " << outGold << endl;
			wcout << L"Name : " << outName << endl;
			cout << "Date : " << outDate.year << '/' << outDate.month << '/' << outDate.day << endl;
		}


		GDBConnectionPool->Push(dbConn);
	}

}

void DoWorkerJob(ServerServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + GLOBAL_QUEUE_MONITORING_TICK;

		// 네트워크 입출력 처리 -> 인게임 로직(패킷 핸들러에 의해)
		service->GetIocpCore()->Dispatch(10);
		// time out을 10ms로 두어, GetQeueCompletionStatus가 10ms경과시, false로 처리되어, 빠져나오게함


		// 예약된 일감 처리
		ThreadManager::DistributeReservedJobs();


		// 글로벌 큐 
		ThreadManager::DoGlobalQueueWork();
	}
}


static uint64 mainThreadTickCount = 0;
void DoMainThreadWorkJob(ServerServiceRef& service)
{
	while (true)
	{
		if (GetTickCount64() > mainThreadTickCount + 1'000)
		{
			{
				cout << "Total(" << GSessionManager->PlayersCount() << ") / "
					<< "Lobby(" << GLobby->GetLobbysPlayersCount() << ") / "
					<< "RoomsAllPlayer(" << GLobby->GetAllRoomsPlayersCount() << ") / "
					<< "RoomCount(" << GLobby->RoomCount() << ") "
					<< endl;
			}

			mainThreadTickCount = GetTickCount64();
		}

		{ // DoWorkJob
			LEndTickCount = ::GetTickCount64() + GLOBAL_QUEUE_MONITORING_TICK;

			// 네트워크 입출력 처리 -> 인게임 로직(패킷 핸들러에 의해)
			service->GetIocpCore()->Dispatch(10);
			// time out을 10ms로 두어, GetQeueCompletionStatus가 10ms경과시, false로 처리되어, 빠져나오게함


			// 예약된 일감 처리
			ThreadManager::DistributeReservedJobs();


			// 글로벌 큐 
			ThreadManager::DoGlobalQueueWork();
		}


	}



}



void Init()
{
	ClientSessionManager::Init();
	Lobby::Init();
}


int main()
{
	Init();

	ClientPacketHandler::Init();

	ServerServiceRef service = MakeShared<ServerService>(NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(), []()
		{
			return ObjectPool<ClientSession>::MakeShared();
		}, 100);


	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([&service]()
			{
				DoWorkerJob(service);
			});
	}



	DoMainThreadWorkJob(service); // 메인스레드도 일하기


	GThreadManager->Join();
}