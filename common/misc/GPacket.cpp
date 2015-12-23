#include "stdafx.h"
#include "GPacket.h"


CGPacket::CGPacket()
{
	m_pBuf		= NULL;
	m_dwSize	= 0;
}


CGPacket::~CGPacket()
{
}

LPBYTE CGPacket::GetBuffer()
{
	return m_pBuf;
}
DWORD  CGPacket::GetBufferLen()
{
	return m_dwSize;
}
int __stdcall findProc(char* pos)
{

}
BOOL   CGPacket::Find(LPBYTE lpKey, DWORD dwKeyLen)
{
	Utility::Algorithm::KMP::KMP((char*)lpKey, dwKeyLen, (char*)m_pBuf, m_dwSize, &findProc);
}
BOOL   CGPacket::Replace(LPBYTE lpKey, DWORD dwKeyLen, LPBYTE lpReplace, DWORD dwReplaceLen)
{
	return TRUE;
}
BOOL   CGPacket::Find(Tstring& strKey)
{
	return TRUE;
}
BOOL   CGPacket::Replace(Tstring& strKey, Tstring& strRep)
{
	return TRUE;
}
CProperty& CGPacket::GetPacketProperty()
{
	return m_property;
}