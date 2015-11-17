#include "stdafx.h"
#include "NetHelpler.h"

//IOCP需要包含的头文件
#include<WinSock2.h>
#include<Windows.h>

#pragma comment(lib, "Ws2_32.lib")      // Socket编程需用的动态链接库  
#pragma comment(lib, "Kernel32.lib")    // IOCP需要用到的动态链接库  
#define MAX_TCP_LENTH	1600

DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);
DWORD WINAPI ServerSendThread(LPVOID IpParam);

CNetHelpler::CNetHelpler()
{

}


CNetHelpler::~CNetHelpler()
{
}


BOOL CNetHelpler::StartNetService(Tstring strIpaddr, int nPort)
{
	// 加载socket动态链接库  
	WORD wVersionRequested = MAKEWORD(2, 2); // 请求2.2版本的WinSock库  
	WSADATA wsaData;    // 接收Windows Socket的结构信息  
	DWORD err = WSAStartup(wVersionRequested, &wsaData);

	if (0 != err) {  // 检查套接字库是否申请成功  
		return FALSE;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {// 检查是否申请了所需版本的套接字库  
		WSACleanup();
		return FALSE;
	}

	//m_hMutex = CreateMutex(NULL, FALSE, NULL);
	// 创建IOCP的内核对象  
	/**
	* 需要用到的函数的原型：
	* HANDLE WINAPI CreateIoCompletionPort(
	*    __in   HANDLE FileHandle,     // 已经打开的文件句柄或者空句柄，一般是客户端的句柄
	*    __in   HANDLE ExistingCompletionPort, // 已经存在的IOCP句柄
	*    __in   ULONG_PTR CompletionKey,   // 完成键，包含了指定I/O完成包的指定文件
	*    __in   DWORD NumberOfConcurrentThreads // 真正并发同时执行最大线程数，一般推介是CPU核心数*2
	* );
	**/
	m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_completionPort) {    // 创建IO内核对象失败  
		return FALSE;
	}

	// 创建IOCP线程--线程里面创建线程池  

	// 确定处理器的核心数量  
	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);

	// 基于处理器的核心数量创建线程  
	for (DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); ++i) {
		// 创建服务器工作器线程，并将完成端口传递到该线程  
		HANDLE ThreadHandle = CreateThread(NULL, 0, WorkerThread, m_completionPort, 0, NULL);
		if (NULL == ThreadHandle) {

			return FALSE;
		}
		CloseHandle(ThreadHandle);
	}

	// 建立流式套接字  
	m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	// 绑定SOCKET到本机  
	SOCKADDR_IN srvAddr;
	srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(DefaultPort);
	int bindResult = bind(m_listenSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
	if (SOCKET_ERROR == bindResult) {
		return FALSE;
	}

	// 将SOCKET设置为监听模式  
	int listenResult = listen(m_listenSocket, 10);
	if (SOCKET_ERROR == listenResult) {
		return FALSE;
	}

	// 开始处理IO数据  
	//cout << "本服务器已准备就绪，正在等待客户端的接入...\n";

	// 创建用于发送数据的线程  
	HANDLE sendThread = CreateThread(NULL, 0, AcceptThread, this, 0, NULL);

	return 0;
}


BOOL CNetHelpler::StopNetService()
{
	return 0;
}

//监听线程流程
DWORD WINAPI CNetHelpler::AcceptThread(LPVOID lpParameter) {

	CNetHelpler* pServer = (CNetHelpler*)lpParameter;

	SOCKET srvSocket = (SOCKET)lpParameter;
	while (TRUE) {
		PER_HANDLE_DATA * PerHandleData = NULL;
		SOCKADDR_IN saRemote;
		int RemoteLen;
		SOCKET acceptSocket;

		// 接收连接，并分配完成端，这儿可以用AcceptEx()  
		RemoteLen = sizeof(saRemote);
		acceptSocket = accept(pServer->m_listenSocket, (SOCKADDR*)&saRemote, &RemoteLen);
		if (SOCKET_ERROR == acceptSocket) {   // 接收客户端失败  
											  //cerr << "Accept Socket Error: " << GetLastError() << endl;
											  //system("pause");
			return FALSE;
		}

		// 创建用来和套接字关联的单句柄数据信息结构  
		PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));  // 在堆中为这个PerHandleData申请指定大小的内存  
		PerHandleData->socket = acceptSocket;
		memcpy(&PerHandleData->ClientAddr, &saRemote, RemoteLen);
		clientGroup.push_back(PerHandleData);       // 将单个客户端数据指针放到客户端组中  

													// 将接受套接字和完成端口关联  
		CreateIoCompletionPort((HANDLE)(PerHandleData->socket), pServer->m_completionPort, (DWORD)PerHandleData, 0);


		// 开始在接受套接字上处理I/O使用重叠I/O机制  
		// 在新建的套接字上投递一个或多个异步  
		// WSARecv或WSASend请求，这些I/O请求完成后，工作者线程会为I/O请求提供服务      
		// 单I/O操作数据(I/O重叠)  
		LPPER_IO_OPERATION_DATA PerIoData = NULL;
		PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = MAX_TCP_LENTH;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;    // read  

		DWORD RecvBytes;
		DWORD Flags = 0;
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}
}
//工作线程流程

DWORD WINAPI CNetHelpler::WorkerThread(LPVOID lpParam)
{
	HANDLE CompletionPort = (HANDLE)lpParam;
	DWORD BytesTransferred;
	LPOVERLAPPED IpOverlapped;
	LPPER_HANDLE_DATA PerHandleData = NULL;
	LPPER_IO_DATA PerIoData = NULL;
	DWORD RecvBytes;
	DWORD Flags = 0;
	BOOL bRet = false;

	while (true) {
		bRet = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);
		if (bRet == 0) {
			//cerr << "GetQueuedCompletionStatus Error: " << GetLastError() << endl;
			return -1;
		}
		PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);

		// 检查在套接字上是否有错误发生  
		if (0 == BytesTransferred) {
			closesocket(PerHandleData->socket);
			GlobalFree(PerHandleData);
			GlobalFree(PerIoData);
			continue;
		}

		// 开始数据处理，接收来自客户端的数据  
		//WaitForSingleObject(pServer->m_hMutex, INFINITE);
		//cout << "A Client says: " << PerIoData->databuff.buf << endl;
		//ReleaseMutex(pServer->m_hMutex);

		// 为下一个重叠调用建立单I/O操作数据  
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // 清空内存  
		PerIoData->databuff.len = MAX_TCP_LENTH;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;    // read  
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}

	return 0;
}