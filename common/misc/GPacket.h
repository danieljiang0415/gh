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
		dwPort		=	0;
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
	DWORD	dwPort;
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
	CGPacket(LPBYTE lpData, DWORD dwLen, CProperty& Property);
public:
	LPBYTE GetBuffer();
	DWORD  GetBufferLen();
	BOOL   Find(LPBYTE lpKey, DWORD dwKeyLen);
	VOID   Replace(LPBYTE lpKey, DWORD dwKeyLen, LPBYTE lpReplace, DWORD dwReplaceLen);
	BOOL   Find(Tstring& strKey);
	VOID   Replace(Tstring& strKey, Tstring& strRep);
	BOOL   AdvancedMach(Tstring& strState);
	CProperty& GetPacketProperty();
	BOOL	IsFiltered();
	VOID SetFiltered();
private:
	LPBYTE m_pBuf;
	LPBYTE m_pRawBuf;
	DWORD  m_dwSize;
	CProperty m_property;
	BOOL m_bFiltered;
};

enum PROCESS_TYPE
{
	PROCESS_FILTER, 
	PROCESS_REPLACE,
	PROCESS_UNDEFINE
};

class CGPacketProcessor
{
public:
	CGPacketProcessor()
	{
		m_strKey		=	_T("");
		m_strReplace	=	_T("");;
		m_strAdvanceKey =	_T("");;
		m_ProcessType	=	PROCESS_UNDEFINE;;
		m_strUUID = Utility::Msic::GenUUID();
	};

	~CGPacketProcessor()
	{
	};

	CGPacketProcessor(CGPacketProcessor& processor)
	{
		m_strAdvanceKey = processor.m_strAdvanceKey;
		m_strKey = processor.m_strKey;
		m_strReplace = processor.m_strReplace;
		m_strUUID = Utility::Msic::GenUUID();
	};

public:
	Tstring m_strKey;
	Tstring m_strReplace;
	Tstring m_strAdvanceKey;
	Tstring m_strUUID;
	PROCESS_TYPE m_ProcessType;
};