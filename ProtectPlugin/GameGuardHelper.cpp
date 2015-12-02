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

extern HANDLE hWait;

#define FUNHEADER_OFFSET 0xFFFFFFFA
#define GAMEMON_CALL(fntype, module, sign, offset) (fntype)((LONG)Utility::Memory::SearchInModuleImage(GetModuleHandle(module), sign, sizeof(sign)) + offset);

CGameGuardHelper::CGameGuardHelper()
{
	DWORD dwRet;
	Utility::UrlLib::HttpRequest(Tstring(_T("GET")), Tstring(_T("http://cdsign.sinaapp.com/index.php?act=nprotect&id=0")), NULL, m_signPreInitNPGameMonA, sizeof(m_signPreInitNPGameMonA), &dwRet);
	Utility::UrlLib::HttpRequest(Tstring(_T("GET")), Tstring(_T("http://cdsign.sinaapp.com/index.php?act=nprotect&id=1")), NULL, m_signInitNPGameMon, sizeof(m_signInitNPGameMon), &dwRet);
	Utility::UrlLib::HttpRequest(Tstring(_T("GET")), Tstring(_T("http://cdsign.sinaapp.com/index.php?act=nprotect&id=2")), NULL, m_signSendCSAuth3ToGameMon, sizeof(m_signSendCSAuth3ToGameMon), &dwRet);
	Utility::UrlLib::HttpRequest(Tstring(_T("GET")), Tstring(_T("http://cdsign.sinaapp.com/index.php?act=nprotect&id=3")), NULL, m_signCheckNPGameMon, sizeof(m_signCheckNPGameMon), &dwRet);

	SetEvent(hWait);
}


CGameGuardHelper::~CGameGuardHelper()
{
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
	

	while (FALSE == m_bVirtualCodeValid)
	{
		__asm nop
		__asm nop
		__asm nop
		__asm nop
		__asm nop
	}

	m_spfnPreInitNPGameMonA = GAMEMON_CALL(PREINITNPGAMEMONA, NULL, m_signPreInitNPGameMonA, FUNHEADER_OFFSET);
	m_spfnInitNPGameMon = GAMEMON_CALL(INITNPGAMEMON, NULL, m_signInitNPGameMon, FUNHEADER_OFFSET);
	//m_spfnSendUserIDToGameMonA = GAMEMON_CALL(SENDUSERIDTOGAMEMONA, NULL, m_signSendUserIDToGameMonA, FUNHEADER_OFFSET);
	m_spfnSendCSAuth3ToGameMon = GAMEMON_CALL(SENDCSAUTH3TOGAMEMON, NULL, m_signSendCSAuth3ToGameMon, FUNHEADER_OFFSET);
	m_sfpnCheckNPGameMon = GAMEMON_CALL(CHECKNPGAMEMON, NULL, m_signCheckNPGameMon, FUNHEADER_OFFSET);
	Utility::Log::DbgPrint(TEXT("\nm_sfpnCheckNPGameMon = %08lx\nm_spfnPreInitNPGameMonA = %08lx\nm_spfnInitNPGameMon = %08lx\nm_spfnSendUserIDToGameMonA = %08lx\nm_spfnSendCSAuth3ToGameMon = %08lx\n"),
		m_sfpnCheckNPGameMon,
		m_spfnPreInitNPGameMonA,
		m_spfnInitNPGameMon,
		m_spfnSendUserIDToGameMonA,
		m_spfnSendCSAuth3ToGameMon);

	return TRUE;
} 


#if 0
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
	DetourAttach(&(PVOID&)m_spfnPreInitNPGameMonA, &fakePreInitNPGameMonA);
	DetourAttach(&(PVOID&)m_sfpnCheckNPGameMon, &fakeCheckNPGameMon);
	//DetourAttach(&(PVOID&)m_spfnSendUserIDToGameMonA, &fakeCloseNPGameMon);
	DetourAttach(&(PVOID&)m_spfnInitNPGameMon, &fakeInitNPGameMon);
	DetourAttach(&(PVOID&)m_spfnSendCSAuth3ToGameMon, &fakeSendCSAuth3ToGameMon);
	//DetourAttach(&(PVOID&)g_pfnlog, &proxy_log);
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