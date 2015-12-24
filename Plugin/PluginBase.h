#pragma once

#include <WinSock2.h>
#include <list>
#include"GPacket.h"

class CPacket;

typedef VOID (WINAPI*SENDPROCHANDLER)(CGPacket& packetBuf);
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

public:
	void AddPackageProcessor(CGPacketProcessor&);
	void DeletePackageProcessor(CGPacketProcessor&);
	void SetFilterEnable(BOOL bEnable);
	void SetReplaceEnable(BOOL bEnable);
	void PreProcessGPacket(CGPacket&);

public:
	BOOL							m_bFileterFunEnable;
	BOOL							m_bReplaceFunEnable;
	std::list<CGPacketProcessor*>	m_PacketProcessorList;
	CRITICAL_SECTION				m_ListCritialSection;

	static CPluginBase*					m_PlugInstance;
};

