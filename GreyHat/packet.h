#pragma once

#include "Utility.h"
#include <WinSock2.h>


typedef struct DISPATCH_CONTEXT
{
	SOCKET   sock;
	LPVOID   param1;
	LPVOID   param2;
	LPVOID   param3;
	LPVOID   param4;
}DISPATCH_CONTEXT, *PDISPATCH_CONTEXT;

enum PACKET_TYPE
{
	IO_SEND,
	IO_RECV
};

class CPacket
{
public:
	CPacket();
	CPacket(LPBYTE lpBuff, DWORD dwLen, DISPATCH_CONTEXT *info);
	~CPacket();

	BYTE *GetBackendData();
	BYTE *GetRawData();
	DWORD GetDataLen();
	PDISPATCH_CONTEXT GetDispatchContext();
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
	DISPATCH_CONTEXT m_DispatchContext;
	BOOL m_bIsSkip;
	Tstring m_tstrRemoteIp;
	DWORD m_dwRemotePort;
	PACKET_TYPE m_Type;
};



class CFilter
{
public:

	CFilter();
	~CFilter();

	Tstring&  GetKeyWord();
	VOID	 SetKeyWord(Tstring& tstrKeyWords);

	Tstring&  GetReplaceData();
	VOID	 SetReplaceData(Tstring& tstrReplaceData);

	Tstring&  GetAdvFilterStr();
	VOID	 SetAdvFilterStr(Tstring& tstrAdvFilterStr);

	BOOL	IsPacketFiltered(CPacket* pPacket);
	BOOL	ReplacePacketData(CPacket* pPacket);

	LPBYTE SearchKeyWord(LPBYTE lpSrc, DWORD dwLen, LPBYTE lpKey, DWORD dwKeyLen);


private:

	Tstring m_tstrKeyWords;
	Tstring m_tstrReplaceData;
	Tstring m_tstrAdvFilter;

	TCHAR*  m_AdvFilterSplit[256];
	TCHAR*	m_pAdvFilterBuffer;
};