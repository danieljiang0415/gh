#include "stdafx.h"
#include "GameGuardHelper.h"
#include "detours.h"
#include "utility.h"
#pragma comment(lib, "detours.lib")

CHECKNPGAMEMON CGameGuardHelper::m_sfpnCheckNPGameMon;
CLOSENPGAMEMON CGameGuardHelper::m_sfpnCloseNPGameMon;
INITNPGAMEMON  CGameGuardHelper::m_spfnInitNPGameMon;
PREINITNPGAMEMONA CGameGuardHelper::m_spfnPreInitNPGameMonA;
SENDCSAUTH3TOGAMEMON CGameGuardHelper::m_spfnSendCSAuth3ToGameMon;


CGameGuardHelper::CGameGuardHelper()
{
}


CGameGuardHelper::~CGameGuardHelper()
{
}

#if 1
HMODULE (WINAPI* g_pfnLoadLibraryW)(LPCWSTR lpLibFileName);
HMODULE WINAPI Proxy_LoadLibraryW(LPCWSTR lpLibFileName)
{
	MessageBoxW( NULL, lpLibFileName,L"Info", MB_YESNO );
	//Utility::Log::XLogDbgStr(_T("--> Proxy_LoadLibraryW --> "));
	//if (StrStrIW(lpLibFileName, L"ws2_32.dll"))
	//{
	//	if ( IDNO == MessageBoxW( NULL, lpLibFileName,L"Info", MB_YESNO ))
	//	{
	//		return NULL;
	//	}
	//}
	return g_pfnLoadLibraryW(lpLibFileName);
}
#endif

BOOL CGameGuardHelper::Initialize()
{
#if 1
	g_pfnLoadLibraryW = (HMODULE(WINAPI* )(LPCWSTR lpLibFileName))GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)g_pfnLoadLibraryW, Proxy_LoadLibraryW);
	DetourTransactionCommit();
#endif
	BYTE signCheckNPGameMon[] = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0xE9 };
	BYTE signCloseNPGameMon[] = { 0x00 };
	BYTE signInitNPGameMon[] = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0xE9 };
	BYTE signPreInitNPGameMonA[] = { 0x53, 0x33, 0xDB, 0x3B, 0xC3, 0x74, 0x04, 0x33, 0xC0, 0x5B, 0xC3, 0x55 };
	BYTE signSendCSAuth3ToGameMon[] = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0x8B, 0x44, 0x24, 0x0C, 0x8B, 0x54, 0x24, 0x08, 0x50, 0x8B, 0x44, 0x24, 0x08, 0x52, 0x50, 0xE8 };


	m_sfpnCheckNPGameMon = (CHECKNPGAMEMON)Utility::Memory::SearchInModuleImage( GetModuleHandle(NULL), signCheckNPGameMon, sizeof(signCheckNPGameMon));
	m_spfnPreInitNPGameMonA = (PREINITNPGAMEMONA)Utility::Memory::SearchInModuleImage(GetModuleHandle(NULL), signPreInitNPGameMonA, sizeof(signPreInitNPGameMonA));
	m_spfnInitNPGameMon = (INITNPGAMEMON)Utility::Memory::SearchInModuleImage(GetModuleHandle(NULL), signInitNPGameMon, sizeof(signInitNPGameMon));
	m_spfnSendCSAuth3ToGameMon = (SENDCSAUTH3TOGAMEMON)Utility::Memory::SearchInModuleImage(GetModuleHandle(NULL), signSendCSAuth3ToGameMon, sizeof(signSendCSAuth3ToGameMon));

	return TRUE;
}


BOOL CGameGuardHelper::DisableOrgProtect()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)m_sfpnCheckNPGameMon, &fakeCheckNPGameMon);
	DetourAttach(&(PVOID&)m_sfpnCloseNPGameMon, &fakeCloseNPGameMon);
	DetourAttach(&(PVOID&)m_spfnInitNPGameMon, &fakeInitNPGameMon);
	DetourAttach(&(PVOID&)m_spfnPreInitNPGameMonA, &fakePreInitNPGameMonA);
	DetourAttach(&(PVOID&)m_spfnSendCSAuth3ToGameMon, &fakeSendCSAuth3ToGameMon);
	DetourTransactionCommit();
	return 0;
}

DWORD __cdecl CGameGuardHelper::fakeCheckNPGameMon()
{
	return NPGAMEMON_SUCCESS;
}
BOOL  __cdecl CGameGuardHelper::fakeCloseNPGameMon()
{
	return TRUE;
}
DWORD __cdecl CGameGuardHelper::fakeInitNPGameMon()
{
	return NPGAMEMON_SUCCESS;
}
void  __cdecl CGameGuardHelper::fakeSetHwndToGameMon(HWND hWnd)
{
	return;
}
DWORD __cdecl CGameGuardHelper::fakePreInitNPGameMonA(LPCSTR szGameName)
{
	return NPGAMEMON_SUCCESS;
}

BOOL  __cdecl CGameGuardHelper::fakeSendCSAuth3ToGameMon(PBYTE pbPacket, DWORD dwPacketSize, DWORD dwServerNumber)
{
	return TRUE;
}