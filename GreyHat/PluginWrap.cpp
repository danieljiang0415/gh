#include "stdafx.h"
#include "PluginWrap.h"
#include "DOMPlugin.h"

CPluginWrap* CPluginWrap::PluginWrapInstance = NULL;
CPluginWrap::CPluginWrap()
{
	//PluginWrapInstance = this;
}

CPluginWrap::CPluginWrap(Tstring& strPluginPath)
{
	//m_strPluginDllPath = strPluginPath;
	//m_hPluginDllInstance = LoadLibrary(strPluginPath.c_str());
	//PluginWrapInstance = this;
}


CPluginWrap::~CPluginWrap()
{
	//FreeLibrary(m_hPluginDllInstance);
	//m_hPluginDllInstance = NULL;
}


BOOL CPluginWrap::InstallPlugin(SENDPROCHANDLER outhandler, RECVPROCHANDLER inhandler)
{
	//BOOL (APIENTRY* Install)(SENDPROCHANDLER, RECVPROCHANDLER) = (BOOL(APIENTRY*)(SENDPROCHANDLER, RECVPROCHANDLER))GetProcAddress(m_hPluginDllInstance, "Install");
	//if (Install) {
	//	return Install(outhandler, inhandler);
	//}
	//return FALSE;
	m_pPlugin = new CDOMPlugin;

	if (m_pPlugin)
		return m_pPlugin->InstallPlugin(inhandler, outhandler);
	else
		return FALSE;
}


BOOL CPluginWrap::UnInstallPlugin()
{
	//BOOL (APIENTRY* UnInstall)() = (BOOL(APIENTRY* )())GetProcAddress(m_hPluginDllInstance, "UnInstall");
	//if (UnInstall) {
	//	return UnInstall();
	//}
	//return FALSE;
	if (m_pPlugin)
	{
		BOOL ret = m_pPlugin->UnInstallPlugin();
		delete m_pPlugin;
		return ret;
	}
	else
		return FALSE;
}


BOOL CPluginWrap::SendData(CGPacket& Packet)
{
	//VOID (APIENTRY* Send)(CGPacket& ) = (VOID(APIENTRY* )(CGPacket&))GetProcAddress(m_hPluginDllInstance, "Send");
	//if (Send)
	//{
	//	Send(Packet);
	//}
	//return TRUE;
	if (m_pPlugin)
	{
		m_pPlugin->SendData(Packet);
	}
	return TRUE;
}


VOID CPluginWrap::AddFilter(CGPacketFilter& filter)
{
	//VOID (APIENTRY* AddFilter)(CGPacketFilter&) = (VOID(APIENTRY*)(CGPacketFilter&))GetProcAddress(m_hPluginDllInstance, "AddFilter");
	//if (AddFilter)
	//{
	//	AddFilter(filter);
	//}
	if (m_pPlugin)
	{
		m_pPlugin->AddPacketFilter(filter);
	}
}


VOID CPluginWrap::DeleteFilter(Tstring& strFilterUUID)
{
	//VOID(APIENTRY* DeleteFilter)(Tstring&) = (VOID(APIENTRY*)(Tstring&))GetProcAddress(m_hPluginDllInstance, "DeleteFilter");
	//if (DeleteFilter)
	//{
	//	DeleteFilter(strFilterUUID);
	//}
	if (m_pPlugin)
	{
		m_pPlugin->DeletePacketFilter(strFilterUUID);
	}
}


VOID CPluginWrap::EnableFilter(bool bEnable)
{
	//VOID(APIENTRY* EnableFilter)(BOOL) = (VOID(APIENTRY*)(BOOL))GetProcAddress(m_hPluginDllInstance, "EnableFilter");
	//if (EnableFilter)
	//{
	//	EnableFilter(bEnable);
	//}
	if (m_pPlugin)
	{
		m_pPlugin->SetFilterEnable(bEnable);
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


VOID CPluginWrap::Initialize(Tstring& strPluginPath)
{
	//m_strPluginDllPath = strPluginPath;
	//m_hPluginDllInstance = LoadLibrary(strPluginPath.c_str());
}


VOID CPluginWrap::EnableSeePacket(bool bEnable)
{
	//VOID(APIENTRY* EnableSeePacket)(BOOL) = (VOID(APIENTRY*)(BOOL))GetProcAddress(m_hPluginDllInstance, "EnableSeePacket");
	//if (EnableSeePacket)
	//{
	//	EnableSeePacket(bEnable);
	//}
	if (m_pPlugin)
	{
		m_pPlugin->EnableCrabPacket(bEnable);
	}
}


VOID CPluginWrap::UnInitialize()
{
	FreeLibrary(m_hPluginDllInstance);
	return VOID();
}
