#pragma once

#include <WinSock2.h>

typedef VOID (WINAPI*SENDPROCHANDLER)(SOCKET, CONST CHAR*, DWORD, LPARAM, LPARAM, LPARAM, LPARAM);
typedef VOID (WINAPI*RECVPROCHANDLER)(LPBYTE, DWORD);
class CPacket;
class CPluginBase
{
public:
	CPluginBase();
	~CPluginBase();
	virtual void SendData(CPacket& packetBuf) = 0;
	virtual BOOL InstallPlugin(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc) = 0;
	virtual BOOL UnInstallPlugin() = 0;
protected:

	FARPROC m_pfnSendInterface;
	FARPROC m_pfnRecvInterface;

protected:
	static SENDPROCHANDLER m_pfnHandleSendProc;
	static RECVPROCHANDLER m_pfnHandleRecvProc;

};

