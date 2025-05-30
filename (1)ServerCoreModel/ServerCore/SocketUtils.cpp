#include "pch.h"
/* -------------------------------------
	  (1)ServerCoreModel_250525
--------------------------------------*/

#include "SocketUtils.h"

/*----------------------
	   SocketUtils
----------------------*/

LPFN_CONNECTEX SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX SocketUtils::AcceptEx = nullptr;

void SocketUtils::Init()
{
	WSADATA wsaData;
	ASSERT_CRASH(::WSAStartup(MAKEWORD(2, 2), OUT &wsaData) == 0);


	// ��Ÿ�ӿ� �ּ� ������ API
	SOCKET dummySocket = CreateSocket();
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX,
		reinterpret_cast<LPVOID*>(&DisconnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));

	Close(dummySocket);
}

void SocketUtils::Clear()
{
	::WSACleanup();
}

bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), OUT &bytes, NULL,
		NULL) != SOCKET_ERROR;
}

SOCKET SocketUtils::CreateSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	/* �� ::WSASocket
		- 4, 5	: �Ϲ������� NULL, 0
		- 6		: �񵿱� ���� I/O ����ϰڴٴ� �÷��� ( IOCP (�񵿱� ����ŷ) + �񵿱� ���ŷ(������ ����) �� �� ���) 
		  -> WSARecv, WSASend, AcceptEx, ConnectEx �� �Լ� ��� ����
	*/
}

bool SocketUtils::SetLinger(SOCKET socket, uint16 onOff, int16 linger)
{
	LINGER option;
	option.l_onoff = onOff;
	option.l_linger = linger;
	return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, option);
	/* �� LINGER
		- l_onoff	:	1�Ͻ�, ���� ���. 0�Ͻ�, ���� ��� ����
		- l_linger	:	���� ���ð�
		==>> CloseSocket �ö�, ���Ŵ��ð� ��ŭ ������ ���� �õ�
	*/
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
	/* �� SO_REUSEADDR
		- ���� ũ���� ��, Ŀ���� TCP���� ������ ���� TIME_WAIT ���¸� ���ʰ� ����.
		- �� �� ������ ����� �Ͽ�, Bind�� �� �� ���� �߻� ����

		- SO_REUSEADDR ������, ũ������ ��� ������ص� ���� �߻� ����

	   => ���� �ܰ迡�� �ַ� ���. 
	   �� ����. Bind ���� �� ������ �ؾ�, ���� ����
	*/
}

bool SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, TCP_NODELAY, flag);
	/* �� TCP_NODELAY
		- ����Ʈ�� flag�� false��, TCP �۽� �� �����Ͱ� ����� �׿��� �۽�
		- ���� ���������� �����Ͱ� �۾Ƶ� ��� ������ �ϱ� ������, flag �� true�� ���� ó��

		@@@@ C# Ŭ���̾�Ʈ ��Ʈ��ũ ���̺귯�� ���� ������, socket.NoDelay = true; ó�� �ʿ� @@@@
	*/
}

bool SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
	// listenSocket�� �����ߴ� SetSockOpt ��������, Accept�� ������ Ŭ���̾�Ʈ ���Ͽ��� �״�� ����
}

bool SocketUtils::Bind(SOCKET socket, NetAddress netAddr)
{
	return ::bind(socket, reinterpret_cast<const SOCKADDR*>(&netAddr.GetSockAddr()), sizeof(SOCKADDR_IN)) 
		!= SOCKET_ERROR;
}

bool SocketUtils::BindAnyAddress(SOCKET socket, uint16 port)
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(port);

	return ::bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress)) != SOCKET_ERROR;
}

bool SocketUtils::Listen(SOCKET socket, int32 backlog)
{
	return ::listen(socket, backlog) != SOCKET_ERROR;
}

void SocketUtils::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
	{
		::closesocket(socket);
		socket = INVALID_SOCKET;
	}
}

// �� SOCKET�� ������ Ÿ������, Ŀ�ο��� ������ �����ϱ� ���� �ĺ��ڷ�, �Լ��� �Ű������� ���� �� ����� �޴°� ���꿡 ����