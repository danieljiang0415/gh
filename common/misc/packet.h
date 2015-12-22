#pragma once

#include "Utility.h"
#include <WinSock2.h>
#include <windows.h>
#include <tchar.h>

class CContext
{

public:
	CContext()
	{
		s = INVALID_SOCKET;
		param1 = NULL;
		param2 = NULL;
		param3 = NULL;
		param4 = NULL;
	}
	~CContext()
	{

	}
public:
	SOCKET s;
	LPVOID param1;
	LPVOID param2;
	LPVOID param3;
	LPVOID param4;
};

enum PACKET_TYPE
{
	IO_SEND,
	IO_RECV
};

class CPacket
{
public:
	CPacket();
	CPacket(LPBYTE lpBuff, DWORD dwLen, CContext& ctx);
	~CPacket();

	BYTE *GetBackendData();
	BYTE *GetRawData();
	DWORD GetDataLen();
	CContext& GetContext();
	VOID SetSkipFlag(BOOL sk);
	BOOL IsSkipped();
	Tstring& GetRemoteIp();
	DWORD   GetRemotePort();
	VOID SetType(PACKET_TYPE);
	PACKET_TYPE GetType();

private:
	BYTE *m_pbRawData;
	BYTE *m_pbBackendData;
	DWORD m_dwPacketLen;
	CContext m_Context;
	BOOL m_bIsSkip;
	Tstring m_tstrRemoteIp;
	DWORD m_dwRemotePort;
	PACKET_TYPE m_Type;
};



class CPacketHelper
{
public:

	CPacketHelper();
	~CPacketHelper();

	Tstring&  GetKeyWord();
	VOID	 SetKeyWord(Tstring& tstrKeyWords);

	Tstring&  GetReplaceData();
	VOID	 SetReplaceData(Tstring& tstrReplaceData);

	Tstring&  GetAdvFilterStr();
	VOID	 SetAdvFilterStr(Tstring& tstrAdvFilterStr);

	BOOL	IsPacketFiltered(CPacket& packetBuf);
	BOOL	ReplacePacketData(CPacket& packetBuf);

	LPBYTE SearchKeyWord(LPBYTE lpSrc, DWORD dwLen, LPBYTE lpKey, DWORD dwKeyLen);

private:

	Tstring m_tstrKeyWords;
	Tstring m_tstrReplaceData;
	Tstring m_tstrAdvFilter;

	TCHAR*  m_AdvFilterSplit[256];
	TCHAR*	m_pAdvFilterBuffer;
public:
	VOID Clear();
};