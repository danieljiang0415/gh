#include "stdafx.h"
#include "GameGuardHelper.h"
#include "detours.h"
#include "utility.h"
#include <tchar.h>
#pragma comment(lib, "detours.lib")

CHECKNPGAMEMON CGameGuardHelper::m_sfpnCheckNPGameMon;
CLOSENPGAMEMON CGameGuardHelper::m_sfpnCloseNPGameMon;
INITNPGAMEMON  CGameGuardHelper::m_spfnInitNPGameMon;
PREINITNPGAMEMONA CGameGuardHelper::m_spfnPreInitNPGameMonA;
SENDUSERIDTOGAMEMONA CGameGuardHelper::m_spfnSendUserIDToGameMonA;
SENDCSAUTH3TOGAMEMON CGameGuardHelper::m_spfnSendCSAuth3ToGameMon;
BOOL CGameGuardHelper::m_bVirtualCodeValid = FALSE;

#define FUNHEADER_OFFSET 0xFFFFFFFA
#define GAMEMON_CALL(fntype, module, sign, offset) (fntype)((LONG)Utility::Memory::SearchInModuleImage(GetModuleHandle(module), sign, sizeof(sign)) + offset);

CGameGuardHelper::CGameGuardHelper()
{
}


CGameGuardHelper::~CGameGuardHelper()
{
}

#if 1
HMODULE (WINAPI* g_pfnLoadLibraryW)(LPCTSTR);
HMODULE WINAPI Proxy_LoadLibraryW(LPCTSTR lpLibFile)
{
	//Utility::Log::DbgPrint(TEXT("Proxy_LoadLibraryW = %s"),lpLibFileName);
	MessageBoxW( NULL, lpLibFile,L"Info", MB_YESNO );
	//Utility::Log::XLogDbgStr(_T("--> Proxy_LoadLibraryW --> "));
	//if (StrStrIW(lpLibFileName, L"ws2_32.dll"))
	//{
	//	if ( IDNO == MessageBoxW( NULL, lpLibFileName,L"Info", MB_YESNO ))
	//	{
	//		return NULL;
	//	}
	//}

	return g_pfnLoadLibraryW(lpLibFile);
}

BOOL(WINAPI * g_pfnCreateProcessInternalA)(HANDLE, LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION, PHANDLE);
BOOL
WINAPI
Proxy_CreateProcessInternalA(
	HANDLE hToken,
	LPCSTR lpApplicationName,
	LPSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCSTR lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation,
	PHANDLE hNewToken
	)
{
	
	//static bool s_bIsHacked = false;
	//if (s_bIsHacked == false)
	//{
	//	//Utility::log::XLogDbgStr(_T("--> Proxy_CreateProcessInternalA --> "));
	//	GetAhnhsApiEntry();

	//	char szMsg[1024];
	//	sprintf_s( szMsg, sizeof( szMsg ), "创建进程%s,是否要剥离保护?", lpCommandLine);
	//	if ( IDYES == MessageBoxA( g_hWnd, szMsg,"Info", MB_YESNO ))
	//	{
	//		//FuckAhnhsApi();
	//		//s_bIsHacked = true;
	//	}
	//}

	char szMsg[1024];
	sprintf_s(szMsg, sizeof(szMsg), "创建进程%s,是否创建？", lpCommandLine);
	if (IDYES != MessageBoxA(NULL, szMsg, "Info", MB_YESNO)) {

		return TRUE;
	}
	//return TRUE;
	return g_pfnCreateProcessInternalA(hToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, hNewToken);
}

#endif

typedef
HANDLE (WINAPI*CREATEFILE)(
	_In_ LPCWSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile
	);

CREATEFILE g_pfnCreateFileW;

HANDLE 
WINAPI
Proxy_CreateFileW(
	_In_ LPCWSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile
	)
{
	if (_tcsstr(lpFileName, _T("rsaenh.dll")))
	{

		CGameGuardHelper::m_bVirtualCodeValid = TRUE;
		
	}
	return g_pfnCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
typedef errno_t (__cdecl*WCSCPY_S)(wchar_t*, rsize_t, wchar_t const*);
WCSCPY_S g_pfnwcscpy_s;
errno_t __cdecl proxy_wcscpy_s(wchar_t* _Destination,	rsize_t _SizeInWords,	wchar_t const* _Source	)
{
	if(FALSE == CGameGuardHelper::m_bVirtualCodeValid)
	{
		CGameGuardHelper::m_bVirtualCodeValid = TRUE;
		Sleep(3000);
	}
	
	return g_pfnwcscpy_s(_Destination, _SizeInWords, _Source);
}
BOOL CGameGuardHelper::Initialize()
{
	OutputDebugString(_T("GameGuard Initialize"));
#if 1
	LoadLibrary(L"msvcr80.dll");
	g_pfnwcscpy_s = (WCSCPY_S)GetProcAddress(GetModuleHandleW(L"msvcr80.dll"), "wcscpy_s");
	Utility::Log::DbgPrint(TEXT("g_pfnwcscpy_s = %08lx"), g_pfnwcscpy_s);
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)g_pfnwcscpy_s, proxy_wcscpy_s);
	DetourTransactionCommit();
#endif
	BYTE signCheckNPGameMon[]		= { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0xE9, 0xBE, 0x2C, 0x00, 0x00};
	BYTE signPreInitNPGameMonA[]	= { 0x33, 0xDB, 0x3B, 0xC3, 0x74, 0x04, 0x33, 0xC0, 0x5B, 0xC3};
	BYTE signInitNPGameMon[]		= { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0xE9, 0x9E, 0x04, 0x00, 0x00};
	BYTE signSendUserIDToGameMonA[] = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0x8B, 0x44, 0x24, 0x04, 0x85};
	BYTE signSendCSAuth3ToGameMon[] = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0x8B, 0x44, 0x24, 0x0C};
	BYTE signCloseNPGameMon[]		= { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};

	while (FALSE == m_bVirtualCodeValid)
	{
		__asm nop
		__asm nop
		__asm nop
		__asm nop
		__asm nop
	}
	m_sfpnCheckNPGameMon = GAMEMON_CALL(CHECKNPGAMEMON, NULL, signCheckNPGameMon, FUNHEADER_OFFSET);
	m_spfnPreInitNPGameMonA = GAMEMON_CALL(PREINITNPGAMEMONA, NULL, signPreInitNPGameMonA, FUNHEADER_OFFSET); 
	m_spfnInitNPGameMon = GAMEMON_CALL(INITNPGAMEMON, NULL, signInitNPGameMon, FUNHEADER_OFFSET); 
	m_spfnSendUserIDToGameMonA = GAMEMON_CALL(SENDUSERIDTOGAMEMONA, NULL, signSendUserIDToGameMonA, FUNHEADER_OFFSET);
	m_spfnSendCSAuth3ToGameMon = GAMEMON_CALL(SENDCSAUTH3TOGAMEMON, NULL, signSendCSAuth3ToGameMon, FUNHEADER_OFFSET);

	Utility::Log::DbgPrint(TEXT("\nm_sfpnCheckNPGameMon = %08lx\nm_spfnPreInitNPGameMonA = %08lx\nm_spfnInitNPGameMon = %08lx\nm_spfnSendUserIDToGameMonA = %08lx\nm_spfnSendCSAuth3ToGameMon = %08lx\n"),
		m_sfpnCheckNPGameMon,
		m_spfnPreInitNPGameMonA,
		m_spfnInitNPGameMon,
		m_spfnSendUserIDToGameMonA,
		m_spfnSendCSAuth3ToGameMon);

	return TRUE;
} 


#if 1
void(__cdecl * g_pfnlog)(LPCWSTR, LPCWSTR, int) = (void(__cdecl * )(LPCWSTR, LPCWSTR, int))0x005a0150;
void __cdecl proxy_log(LPCWSTR src, LPCWSTR errstr, int errcode)
{
	MessageBox(0, _T("proxy_log Enter"), 0, 0);
	Utility::Log::DbgPrint(TEXT("gamelog-->%s, %d"), errstr, errcode);
}
#endif
BOOL CGameGuardHelper::DisableOrgProtect()
{
	Utility::Log::DbgPrint(TEXT("DisableOrgProtect Enter"));
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)m_sfpnCheckNPGameMon, &fakeCheckNPGameMon);
	DetourAttach(&(PVOID&)m_spfnSendUserIDToGameMonA, &fakeCloseNPGameMon);
	DetourAttach(&(PVOID&)m_spfnInitNPGameMon, &fakeInitNPGameMon);
	DetourAttach(&(PVOID&)m_spfnPreInitNPGameMonA, &fakePreInitNPGameMonA);
	DetourAttach(&(PVOID&)m_spfnSendCSAuth3ToGameMon, &fakeSendCSAuth3ToGameMon);
	DetourAttach(&(PVOID&)g_pfnlog, &proxy_log);
	DetourTransactionCommit();
	return TRUE;
}



DWORD __cdecl CGameGuardHelper::fakeCheckNPGameMon()
{
	//Utility::Log::DbgPrint(TEXT("fakeCheckNPGameMon Enter"));
	return NPGAMEMON_SUCCESS;
}
BOOL  __cdecl CGameGuardHelper::fakeCloseNPGameMon()
{
	Utility::Log::DbgPrint(TEXT("fakeCloseNPGameMon Enter"));
	return TRUE;
}
DWORD __cdecl CGameGuardHelper::fakeInitNPGameMon()
{
	DWORD dwRet = 0;
	//dwRet = m_spfnInitNPGameMon();
	Utility::Log::DbgPrint(TEXT("fakeInitNPGameMon [%d]"), dwRet);
	return NPGAMEMON_SUCCESS;
}
void  __cdecl CGameGuardHelper::fakeSetHwndToGameMon(HWND hWnd)
{
	Utility::Log::DbgPrint(TEXT("fakeSetHwndToGameMon Enter"));
	return;
}
DWORD __cdecl CGameGuardHelper::fakePreInitNPGameMonA(LPCSTR szGameName)
{
	DWORD dwRet = 0;
	//dwRet = m_spfnPreInitNPGameMonA(szGameName);
	Utility::Log::DbgPrint(TEXT("fakePreInitNPGameMonA [%S], [%d]"), szGameName, dwRet);
	return NPGAMEMON_SUCCESS;
}

BOOL  __cdecl CGameGuardHelper::fakeSendCSAuth3ToGameMon(PBYTE pbPacket, DWORD dwPacketSize, DWORD dwServerNumber)
{
	Utility::Log::DbgPrint(TEXT("fakeSendCSAuth3ToGameMon Enter"));
	return TRUE;
}