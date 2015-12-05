// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "utility.h"
#include "GameGuardHelper.h"
#include "HackShieldHelper.h"
#include "XinCode3Helper.h"
#include "XtrapHelper.h"
#include "ProtectBase.h"
#include<tchar.h>
#include "IatHook.h"

#include "detours.h"
#pragma comment(lib, "detours.lib")

#define CONFIG_FILE _T("config.ini")
#define TARGETEXE_SECTION _T("TARGET_EXE")
#define TARGETEXE_VALUE _T("VALUE")

#define PROTECT_SECTION _T("PROTECT")
#define PROTECT_VALUE _T("VALUE")
#define MUTEX_CALCSERVER _T("CALCSERVER")
#define PACTH_CONFIG _T("OPTION")

DWORD __stdcall WorkTrd(LPVOID lparam);
BOOL bDecodeComplete = FALSE;
//typedef LONG NTSTATUS;
//
//typedef NTSTATUS (NTAPI*ZWSETINFORMATIONTHREAD)(
//	__in HANDLE ThreadHandle,
//	__in LONG ThreadInformationClass,
//	__in_bcount(ThreadInformationLength) PVOID ThreadInformation,
//	__in ULONG ThreadInformationLength
//	);
//
//ZWSETINFORMATIONTHREAD ZwSetInformationThread;
//NTSTATUS NTAPI detourZwSetInformationThread(
//	__in HANDLE ThreadHandle,
//	__in LONG ThreadInformationClass,
//	__in_bcount(ThreadInformationLength) PVOID ThreadInformation,
//	__in ULONG ThreadInformationLength
//	)
//{
//
//	//Utility::Log::DbgPrint(TEXT("ThreadInformationClass = %08lx"), ThreadInformationClass);
//	//if (ThreadInformationClass == 0x11) {
//	//	MessageBox(0, TEXT("ThreadInformationClass = %08lx"), 0, 0);
//	//	return 1L;
//	//}
//	//else
//		return ZwSetInformationThread(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength);
//}
//typedef void (WINAPI*GETSYSTEMTIMEASFILETIME)(LPFILETIME);
//GETSYSTEMTIMEASFILETIME fnGetSystemTimeAsFileTime;
//void WINAPI fakeGetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
//{
//	Utility::Log::DbgPrint(TEXT("fakeGetSystemTimeAsFileTime"));
//	fnGetSystemTimeAsFileTime(lpSystemTimeAsFileTime);
//}
//
//typedef DWORD (WINAPI*GETCURRENTPROCESSID)();
//GETCURRENTPROCESSID fnGetCurrentProcessId;
//DWORD WINAPI fakeGetCurrentProcessId()
//{
//	Utility::Log::DbgPrint(TEXT("fakeGetCurrentProcessId"));
//	return fnGetCurrentProcessId();
//}
//
//typedef DWORD(WINAPI*GETCURRENTTHREADID)();
//GETCURRENTTHREADID fnGetCurrentThreadId;
//DWORD WINAPI fakeGetCurrentThreadId()
//{
//	Utility::Log::DbgPrint(TEXT("fakeGetCurrentThreadId"));
//	return fnGetCurrentThreadId();
//}
//
//typedef FARPROC(WINAPI*GETPROCADDRESS)(HMODULE, LPCSTR);
//GETPROCADDRESS fnGetProcAddress;
//FARPROC WINAPI fakeGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
//{
//	Utility::Log::DbgPrint(TEXT("fakeGetProcAddress, hModule=%08lx, %S"), hModule, lpProcName);
//	return fnGetProcAddress(hModule, lpProcName);
//}
typedef BOOL (WINAPI*QUERYPERFORMANCECOUNTER)(LARGE_INTEGER*);
QUERYPERFORMANCECOUNTER fnQueryPerformanceCounter;
BOOL WINAPI fakeQueryPerformanceCounter(LARGE_INTEGER * lpPerformanceCount)
{
	if (FALSE == bDecodeComplete) {
		bDecodeComplete = !bDecodeComplete;
		Sleep(1000);
	}
	Utility::Log::DbgPrint(TEXT("fakeQueryPerformanceCounter"));
	return fnQueryPerformanceCounter(lpPerformanceCount);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{	
	
		/*fnGetSystemTimeAsFileTime = (GETSYSTEMTIMEASFILETIME)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetSystemTimeAsFileTime");
		Utility::Log::DbgPrint(TEXT("fnGetSystemTimeAsFileTime = %08lx"), fnGetSystemTimeAsFileTime);
		fnGetCurrentProcessId = (GETCURRENTPROCESSID)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetCurrentProcessId");
		Utility::Log::DbgPrint(TEXT("fnGetCurrentProcessId = %08lx"), fnGetCurrentProcessId);
		fnGetCurrentThreadId = (GETCURRENTTHREADID)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetCurrentThreadId");
		Utility::Log::DbgPrint(TEXT("fnGetCurrentThreadId = %08lx"), fnGetCurrentThreadId);
		fnGetProcAddress = (GETPROCADDRESS)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetProcAddress");
		Utility::Log::DbgPrint(TEXT("fnGetProcAddress = %08lx"), fnGetProcAddress);*/
		fnQueryPerformanceCounter = (QUERYPERFORMANCECOUNTER)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "QueryPerformanceCounter");
		Utility::Log::DbgPrint(TEXT("fnQueryPerformanceCounter = %08lx"), fnQueryPerformanceCounter);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		//DetourAttach(&(PVOID&)fnGetSystemTimeAsFileTime, fakeGetSystemTimeAsFileTime);
		//DetourAttach(&(PVOID&)fnGetCurrentProcessId, fakeGetCurrentProcessId);
		//DetourAttach(&(PVOID&)fnGetCurrentThreadId, fakeGetCurrentThreadId);
		DetourAttach(&(PVOID&)fnQueryPerformanceCounter, fakeQueryPerformanceCounter);
		//DetourAttach(&(PVOID&)fnGetProcAddress, fakeGetProcAddress);
		DetourTransactionCommit();
		CreateThread(NULL, 0, WorkTrd, hModule, 0, 0);
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

		BOOL bPatchCfg = Utility::IniAccess::GetPrivateKeyValInt(strCfgFile, PROTECT_SECTION, PACTH_CONFIG);
		Utility::Log::DbgPrint(TEXT("PACTH_CONFIG = %d"), bPatchCfg);
		if (bPatchCfg)
		{
			Utility::Log::DbgPrint(TEXT("PACTH_CONFIG = TRUE"));
			pHelper->DisableOrgProtect();
			delete pHelper;
			return 0;
		}

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