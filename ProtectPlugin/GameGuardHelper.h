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
	static	PREINITNPGAMEMONA m_spfnPreInitNPGameMonA;
	static	SENDCSAUTH3TOGAMEMON m_spfnSendCSAuth3ToGameMon;
};
