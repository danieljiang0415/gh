// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "utility.h"
#include "GameGuardHelper.h"
#include "HackShieldHelper.h"
#include "XinCode3Helper.h"
#include "XtrapHelper.h"
#include "ProtectBase.h"
#include<tchar.h>

#define CONFIG_FILE _T("config.ini")
#define TARGETEXE_SECTION _T("TARGET_EXE")
#define TARGETEXE_VALUE _T("VALUE")

#define PROTECT_SECTION _T("PROTECT")
#define PROTECT_VALUE _T("VALUE")
#define MUTEX_CALCSERVER _T("CALCSERVER")

DWORD __stdcall WorkTrd(LPVOID lparam);


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		//MessageBox(0, 0, 0, 0);
		Tstring strExe = Utility::Module::GetModuleName(GetModuleHandle(NULL));
		Tstring strDllPath = Utility::Module::GetModulePath(hModule);
		Tstring strCfgFile = strDllPath + Tstring(_T("\\")) + Tstring(CONFIG_FILE);
		Tstring strTargetExe = Utility::IniAccess::GetPrivateKeyValString(strCfgFile, TARGETEXE_SECTION, TARGETEXE_VALUE);
		if (strTargetExe == strExe) {
			CreateThread(NULL, 0, WorkTrd, hModule, 0, 0);
			return TRUE;
		}
		else {
			return FALSE;
		}

	}
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

DWORD __stdcall WorkTrd(LPVOID lparam)
{
	
	Tstring strDllPath = Utility::Module::GetModulePath ((HMODULE)lparam);

	Tstring strCfgFile = strDllPath + Tstring(_T("\\")) + Tstring(CONFIG_FILE);
	
	Tstring strProtect = Utility::IniAccess::GetPrivateKeyValString(strCfgFile, PROTECT_SECTION, PROTECT_VALUE);

	CProtectBase *pHelper = NULL;
	if (strProtect == Tstring(_T("GameGuard"))) {
		pHelper = new CGameGuardHelper;

	}else if (strProtect == Tstring(_T("HackShield"))) {
		pHelper = new CHackShieldHelper;

	}else if (strProtect == Tstring(_T("Xtrap"))) {
		pHelper = new CXtrapHelper;

	}else if (strProtect == Tstring(_T("XinCode3"))) {
		pHelper = new CXinCode3Helper;

	}

	
	if (pHelper->Initialize()) {

#if 1
		if (Utility::IniAccess::GetPrivateKeyValInt(strCfgFile, PROTECT_SECTION, _T("TEST")))
		{
			pHelper->DisableOrgProtect();
			delete pHelper;
			return 0;
		}
#endif

		HANDLE hMutex = OpenMutex( MUTEX_ALL_ACCESS, TRUE, MUTEX_CALCSERVER );
		if ( hMutex == NULL && ERROR_FILE_NOT_FOUND == GetLastError())
		{
			pHelper->StartAsHeartBeatServer();
		}
		else
		{
			pHelper->DisableOrgProtect();
		}
	}

	delete pHelper;

	return 0;
}