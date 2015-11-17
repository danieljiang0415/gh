#pragma once

#include<string>
#include<vector>
#include<winsock2.h>

#ifndef _UNICODE
	#define Tstring std::string
#else
	#define Tstring std::wstring
#endif

class CNetHelpler
{
public:
	CNetHelpler();
	~CNetHelpler();
	BOOL StartNetService(Tstring strIpaddr, int nPort);
	BOOL StopNetService();

private:
	HANDLE m_completionPort;
	SOCKET m_listenSocket;
	HANDLE m_hMutex;

public:
	static DWORD WINAPI AcceptThread(LPVOID lpParameter);
	static DWORD WINAPI WorkerThread(LPVOID lpParameter);
};


/**
* 结构体名称：PER_IO_DATA
* 结构体功能：重叠I/O需要用到的结构体，临时记录IO数据
**/
const int DataBuffSize = 2 * 1024;
typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	char buffer[DataBuffSize];
	int BufferLen;
	int operationType;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA, *LPPER_IO_DATA, PER_IO_DATA;

/**
* 结构体名称：PER_HANDLE_DATA
* 结构体存储：记录单个套接字的数据，包括了套接字的变量及套接字的对应的客户端的地址。
* 结构体作用：当服务器连接上客户端时，信息存储到该结构体中，知道客户端的地址以便于回访。
**/
typedef struct
{
	SOCKET socket;
	SOCKADDR_STORAGE ClientAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// 定义全局变量  
const int DefaultPort = 6000;
std::vector < PER_HANDLE_DATA* > clientGroup;      // 记录客户端的向量组 
