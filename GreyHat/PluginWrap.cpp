#include "stdafx.h"
#include "PluginWrap.h"

CPluginWrap* CPluginWrap::PluginWrapInstance = NULL;
CPluginWrap::CPluginWrap()
{
	PluginWrapInstance = this;
}

CPluginWrap::CPluginWrap(Tstring& strPluginPath)
{
	m_strPluginDllPath = strPluginPath;
	m_hPluginDllInstance = LoadLibrary(strPluginPath.c_str());
	PluginWrapInstance = this;
}


CPluginWrap::~CPluginWrap()
{
	FreeLibrary(m_hPluginDllInstance);
	m_hPluginDllInstance = NULL;
}


BOOL CPluginWrap::InstallPlugin(SENDPROCHANDLER outhandler, RECVPROCHANDLER inhandler)
{
	BOOL (APIENTRY* Install)(SENDPROCHANDLER, RECVPROCHANDLER) = (BOOL(APIENTRY*)(SENDPROCHANDLER, RECVPROCHANDLER))GetProcAddress(m_hPluginDllInstance, "Install");
	if (Install) {
		return Install(outhandler, inhandler);
	}
	return FALSE;
}


BOOL CPluginWrap::UnInstallPlugin()
{
	BOOL (APIENTRY* UnInstall)() = (BOOL(APIENTRY* )())GetProcAddress(m_hPluginDllInstance, "UnInstall");
	if (UnInstall) {
		return UnInstall();
	}
	return FALSE;
}


BOOL CPluginWrap::SendData(CGPacket& Packet)
{
	VOID (APIENTRY* Send)(CGPacket& ) = (VOID(APIENTRY* )(CGPacket&))GetProcAddress(m_hPluginDllInstance, "Send");
	if (Send)
	{
		Send(Packet);
	}
	return TRUE;
}


VOID CPluginWrap::AddFilter(CGPacketFilter& filter)
{
	VOID (APIENTRY* AddFilter)(CGPacketFilter&) = (VOID(APIENTRY*)(CGPacketFilter&))GetProcAddress(m_hPluginDllInstance, "AddFilter");
	if (AddFilter)
	{
		AddFilter(filter);
	}
}


VOID CPluginWrap::DeleteFilter(Tstring& strFilterUUID)
{
	VOID(APIENTRY* DeleteFilter)(Tstring&) = (VOID(APIENTRY*)(Tstring&))GetProcAddress(m_hPluginDllInstance, "DeleteFilter");
	if (DeleteFilter)
	{
		DeleteFilter(strFilterUUID);
	}
}


VOID CPluginWrap::EnableFilter(bool bEnable)
{
	VOID(APIENTRY* EnableFilter)(BOOL) = (VOID(APIENTRY*)(BOOL))GetProcAddress(m_hPluginDllInstance, "EnableFilter");
	if (EnableFilter)
	{
		EnableFilter(bEnable);
	}
}


CPluginWrap* CPluginWrap::GetPluginWrap()
{
	//if (NULL == m_PluginInstance)
	//{
	//	m_PluginInstance = new CPluginWrap(strPluginPath);
	//}
	//else {
	//	return m_PluginInstance;
	//}
	return NULL;
}
