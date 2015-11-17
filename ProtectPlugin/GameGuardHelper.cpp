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


BOOL CGameGuardHelper::Initialize()
{
	BYTE signCheckNPGameMon[] = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0xE9 };
	BYTE signCloseNPGameMon[] = { 0x00 };
	BYTE signInitNPGameMon[] = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0xE9 };
	BYTE signPreInitNPGameMonA[] = { 0x53, 0x33, 0xDB, 0x3B, 0xC3, 0x74, 0x04, 0x33, 0xC0, 0x5B, 0xC3, 0x55 };
	BYTE signSendCSAuth3ToGameMon[] = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0x8B, 0x44, 0x24, 0x0C, 0x8B, 0x54, 0x24, 0x08, 0x50, 0x8B, 0x44, 0x24, 0x08, 0x52, 0x50, 0xE8 };


	m_sfpnCheckNPGameMon = (CHECKNPGAMEMON)Utility::Memory::SearchInModuleImage( GetModuleHandle(NULL), signCheckNPGameMon, sizeof(signCheckNPGameMon));
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