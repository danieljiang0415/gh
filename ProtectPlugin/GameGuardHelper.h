#pragma once
#include"ProtectBase.h"


typedef struct _GG_AUTH_DATA
{
	DWORD dwIndex;
	DWORD dwValue1;
	DWORD dwValue2;
	DWORD dwValue3;
} GG_AUTH_DATA, *PGG_AUTH_DATA;

typedef struct _CSAuth3Data
{
	DWORD dwServerNumber;
	DWORD dwPacketSize;
	BYTE  bPacket[4096];

}CSAuth3Data, *PCSAuth3Data;


typedef BOOL(CALLBACK *NPGMCALLBACK)(DWORD dwMsg, DWORD dwArg);

typedef DWORD (__cdecl *CHECKNPGAMEMON)();

typedef BOOL (__cdecl *CLOSENPGAMEMON)();
typedef DWORD (__cdecl *INITNPGAMEMON)();
typedef void (__cdecl *SETHWNDTOGAMEMON)(HWND hWnd);
typedef DWORD (__cdecl *PREINITNPGAMEMONA)(LPCSTR szGameName);
typedef BOOL(__cdecl *SENDUSERIDTOGAMEMONA)(LPCSTR szUserID);
typedef BOOL(__cdecl *SENDCSAUTHTOGAMEMON)(DWORD dwAuth);
typedef BOOL(__cdecl *SENDCSAUTH2TOGAMEMON)(PGG_AUTH_DATA pAuth);
typedef BOOL(__cdecl *SENDCSAUTH2TOGAMEMONWITHSEPERATOR)(PGG_AUTH_DATA pAuth, DWORD dwSeperator);
typedef BOOL(__cdecl *SENDCSAUTH3TOGAMEMON)(PBYTE pbPacket, DWORD dwPacketSize, DWORD dwServerNumber);

#define NPGAMEMON_SUCCESS              0x755       // GameMon Initialize Success

class CGameGuardHelper: public CProtectBase
{
public:
	CGameGuardHelper();
	~CGameGuardHelper();
	virtual BOOL Initialize();
	virtual BOOL DisableOrgProtect();
public:
	static DWORD __cdecl fakeCheckNPGameMon();
	static BOOL  __cdecl fakeCloseNPGameMon();
	static DWORD __cdecl fakeInitNPGameMon();
	static void  __cdecl fakeSetHwndToGameMon(HWND hWnd);
	static DWORD __cdecl fakePreInitNPGameMonA(LPCSTR szGameName);
	static BOOL  __cdecl fakeSendCSAuth3ToGameMon(PBYTE pbPacket, DWORD dwPacketSize, DWORD dwServerNumber);
public:
	static CHECKNPGAMEMON m_sfpnCheckNPGameMon;
	static CLOSENPGAMEMON m_sfpnCloseNPGameMon;
	static INITNPGAMEMON  m_spfnInitNPGameMon;
	static PREINITNPGAMEMONA m_spfnPreInitNPGameMonA;
	static SENDUSERIDTOGAMEMONA m_spfnSendUserIDToGameMonA;
	static SENDCSAUTH3TOGAMEMON m_spfnSendCSAuth3ToGameMon;
	static BOOL m_bVirtualCodeValid;

private:
	BYTE m_signPreInitNPGameMonA[10];// = { 0x33, 0xDB, 0x3B, 0xC3, 0x74, 0x04, 0x33, 0xC0, 0x5B, 0xC3 };
	BYTE m_signInitNPGameMon[12];// = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0xE9, 0x9E, 0x04, 0x00, 0x00 };
	BYTE m_signSendCSAuth3ToGameMon[11];// = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0x8B, 0x44, 0x24, 0x0C };
	BYTE m_signCheckNPGameMon[12];// = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0xE9, 0xBE, 0x2C, 0x00, 0x00 };
	//BYTE signSendUserIDToGameMonA[] = { 0x85, 0xC9, 0x75, 0x03, 0x33, 0xC0, 0xC3, 0x8B, 0x44, 0x24, 0x04, 0x85};
};
