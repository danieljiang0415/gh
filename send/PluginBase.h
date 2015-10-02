#pragma once

#include <WinSock2.h>

class CPacket;

typedef VOID (WINAPI*SENDPROCHANDLER)(CPacket& packetBuf);
typedef VOID (WINAPI*RECVPROCHANDLER)(CPacket& packetBuf);

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

