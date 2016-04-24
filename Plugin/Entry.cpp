// common.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>
#include "Plugin.h"
#include "DOMPlugin.h"
#include "BladePlugin.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

#define CONFIG_FILE _T("config.ini")
#define SHELLCODE_SECTION _T("SHELLCODE")
#define SHELLCODE_VALUE _T("SHELLCODE")


CPluginBase* pPlugin;
HMODULE G_DLLModule;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	G_DLLModule = hModule;
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


BOOL APIENTRY Install(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc)
{
	//Tstring strDllPath = Utility::Module::GetModulePath(G_DLLModule);

	//Tstring strCfgFile = strDllPath + Tstring(_T("\\")) + Tstring(CONFIG_FILE);

	//Tstring strShellCOde = Utility::IniAccess::GetPrivateKeyValString(strCfgFile, SHELLCODE_SECTION, SHELLCODE_VALUE);
	//Utility::Log::DbgPrint(_T("game=[%s]"), strGameName.c_str());
	//if (strGameName == Tstring(_T("BSTW"))) {
	//	pPlugin = new CBladePlugin;
	//	Utility::Log::DbgPrint(_T("new CBladePlugin"));

	//}
	//else {
		pPlugin = new CDOMPlugin;
	//	Utility::Log::DbgPrint(_T("new CPlugin"));
	//}
	
	if (pPlugin)
		return pPlugin->InstallPlugin(pfnHandleInputProc, pfnHandleOutputProc);
	else
		return FALSE;
}

BOOL APIENTRY UnInstall()
{
	if (pPlugin)
	{
		BOOL ret = pPlugin->UnInstallPlugin();
		delete pPlugin;
		return ret;
	}
	else
		return FALSE;
}

VOID APIENTRY Send(CGPacket& packetBuf)
{
	if (pPlugin)
	{
		pPlugin->SendData(packetBuf);
	}
		
}

void APIENTRY AddFilter(CGPacketFilter& filter)
{
	if (pPlugin)
	{
		pPlugin->AddPacketFilter(filter);
	}
}
void APIENTRY DeleteFilter(Tstring& strFilterUUID)
{
	if (pPlugin)
	{
		pPlugin->DeletePacketFilter(strFilterUUID);
	}
}
void APIENTRY ClearPacketFilters()
{
	if (pPlugin)
	{
		pPlugin->ClearPacketFilters();
	}
}
void APIENTRY EnableFilter(BOOL bEnable)
{
	if (pPlugin)
	{
		pPlugin->SetFilterEnable(bEnable);
	}
}
void APIENTRY EnableSeePacket(BOOL bEnable)
{
	if (pPlugin)
	{
		pPlugin->EnableCrabPacket(bEnable);
	}
}