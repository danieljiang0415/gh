#pragma once
#include "GPacket.h"

class CPluginBase;

typedef VOID(WINAPI*SENDPROCHANDLER)(CGPacket&);
typedef VOID(WINAPI*RECVPROCHANDLER)(CGPacket&);

class CPluginWrap
{
public:
	CPluginWrap();
	CPluginWrap(Tstring& strPluginPath);
	~CPluginWrap();
public:
	BOOL InstallPlugin(SENDPROCHANDLER outhandler, RECVPROCHANDLER inhandler);
	BOOL UnInstallPlugin();
	BOOL SendData(CGPacket& Packet);
	VOID AddFilter(CGPacketFilter& filter);
	VOID DeleteFilter(Tstring& strFilterUUID);
	VOID EnableFilter(bool bEnable);

public:
	static CPluginWrap* PluginWrapInstance;
	static CPluginWrap* GetPluginWrap();

private:
	HMODULE m_hPluginDllInstance;
	Tstring m_strPluginDllPath;

	CPluginBase* m_pPlugin;
public:
	VOID Initialize(Tstring&);
	VOID EnableSeePacket(bool bEnable);
	VOID UnInitialize();
};

