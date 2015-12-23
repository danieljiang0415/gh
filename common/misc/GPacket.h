#pragma once
#include "Utility.h"
#include <WinSock2.h>
#include <tchar.h>
enum PACKET_IO
{
	IO_INPUT, 
	IO_OUTPUT,
	IO_UNDEFINE
};
class CProperty
{
public:
	CProperty()
	{
		strIpAddr	=	_T("");
		strPort		=	_T("");
		ioType		=	IO_UNDEFINE;
		s			=	INVALID_SOCKET;
		Param1		=	NULL;
		Param2		=	NULL;
		Param3		=	NULL;
		Param4		=	NULL;
	};

	~CProperty()
	{
	};

public:
	Tstring strIpAddr;
	Tstring strPort;
	PACKET_IO ioType;
	SOCKET s;
	LPVOID Param1;
	LPVOID Param2;
	LPVOID Param3;
	LPVOID Param4;

};

class CGPacket
{
public:
	CGPacket();
	~CGPacket();
public:
	LPBYTE GetBuffer();
	DWORD  GetBufferLen();
	BOOL   Find(LPBYTE lpKey, DWORD dwKeyLen);
	BOOL   Replace(LPBYTE lpKey, DWORD dwKeyLen, LPBYTE lpReplace, DWORD dwReplaceLen);
	BOOL   Find(Tstring& strKey);
	BOOL   Replace(Tstring& strKey, Tstring& strRep);
	CProperty& GetPacketProperty();
private:
	LPBYTE m_pBuf;
	DWORD  m_dwSize;
	CProperty m_property;
};

