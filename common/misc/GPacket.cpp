#include "stdafx.h"
#include "GPacket.h"
#include <map>

CGPacket::CGPacket()
{
	m_pBuf		= NULL;
	m_dwSize	= 0;
	m_bFiltered = FALSE;
}


CGPacket::~CGPacket()
{
	if (m_pBuf && m_dwSize > 0)
	{
		delete[] m_pBuf;
		m_dwSize = 0;
	}
}

CGPacket::CGPacket(LPBYTE lpData, DWORD dwLen, CProperty& Property)
{
	if (lpData == NULL || dwLen == 0)return;
	m_pBuf = new BYTE[dwLen];
	memcpy(m_pBuf, lpData, dwLen);
	m_dwSize = dwLen;
	m_property = Property;
	
	if (Property.s != INVALID_SOCKET)
	{
		sockaddr_in sin;
		int sinlen = sizeof(sin);
		if (0 == getpeername(Property.s, (sockaddr*)&sin, &sinlen))
		{
			TCHAR tszTemp[256], tszIP[64];
			DWORD dwLen = sizeof(tszTemp), dwPort;

			if (0 == WSAAddressToString((SOCKADDR *)&sin, sizeof(SOCKADDR), NULL, tszTemp, (LPDWORD)&dwLen))
			{
				_stscanf_s(tszTemp, _T("%[0-9,.]:%d"), tszIP, sizeof(tszIP), &dwPort);
				m_property.strIpAddr = tszIP;
				m_property.dwPort = dwPort;
			}
		}
	}
}

LPBYTE CGPacket::GetBuffer()
{
	return m_pBuf;
}
DWORD  CGPacket::GetBufferLen()
{
	return m_dwSize;
}

void preKmp(char *x, int m, int kmpNext[]) {
	int i, j;

	i = 0;
	j = kmpNext[0] = -1;
	while (i < m) {
		while (j > -1 && x[i] != x[j])
			j = kmpNext[j];
		i++;
		j++;
		if (x[i] == x[j])
			kmpNext[i] = kmpNext[j];
		else
			kmpNext[i] = j;
	}
}

BOOL   CGPacket::Find(LPBYTE lpKey, DWORD dwKeyLen)
{
	int i, j, kmpNext[256];
	/* Preprocessing */
	preKmp((char*)lpKey, dwKeyLen, kmpNext);

	/* Searching */
	i = j = 0;
	while (j < m_dwSize) {
		while (i > -1 && lpKey[i] != m_pBuf[j])
			i = kmpNext[i];
		i++;
		j++;
		if (i >= dwKeyLen) {
			//OUTPUT(j - i);
			return TRUE;
			i = kmpNext[i];
		}
	}
	return FALSE;
}
VOID   CGPacket::Replace(LPBYTE lpKey, DWORD dwKeyLen, LPBYTE lpReplace, DWORD dwReplaceLen)
{
	int i, j, kmpNext[256];
	/* Preprocessing */
	preKmp((char*)lpKey, dwKeyLen, kmpNext);

	/* Searching */
	i = j = 0;
	while (j < m_dwSize) {
		while (i > -1 && lpKey[i] != m_pBuf[j])
			i = kmpNext[i];
		i++;
		j++;
		if (i >= dwKeyLen) {
			//OUTPUT(j - i);
			memcpy_s(&m_pBuf[j - i], m_dwSize - j + i, lpReplace, dwReplaceLen);
			i = kmpNext[i];
		}
	}
}
BOOL   CGPacket::Find(Tstring& strKey)
{
	DWORD dwStrKeyLen, dwKeyBufSize;
	LPBYTE lpbKeyBuf;

	dwStrKeyLen = strKey.length();
	lpbKeyBuf = new BYTE[dwStrKeyLen];
	memset(lpbKeyBuf, 0, dwStrKeyLen);
	dwKeyBufSize = Utility::StringLib::Tstring2Hex(strKey.c_str(), lpbKeyBuf);


	BOOL bRet = Find(lpbKeyBuf, dwKeyBufSize);
	delete[]lpbKeyBuf;
	return bRet;
}
VOID   CGPacket::Replace(Tstring& strKey, Tstring& strData)
{
	DWORD dwStrKeyLen, dwStrDataLen, dwHexKeySize, dwHexDataSize;
	LPBYTE lpbKeyBuf, lpbDataBuf;

	dwStrKeyLen = strKey.length();
	lpbKeyBuf = new BYTE[dwStrKeyLen];
	memset(lpbKeyBuf, 0, dwStrKeyLen);
	dwHexKeySize = Utility::StringLib::Tstring2Hex(strKey.c_str(), lpbKeyBuf);

	dwStrDataLen = strKey.length();
	lpbDataBuf = new BYTE[dwStrDataLen];
	memset(lpbDataBuf, 0, dwStrDataLen);
	dwHexDataSize = Utility::StringLib::Tstring2Hex(strData.c_str(), lpbDataBuf);

	Replace(lpbKeyBuf, dwHexKeySize, lpbDataBuf, dwHexDataSize);

	delete[]lpbKeyBuf;
	delete[]lpbDataBuf;

}
CProperty& CGPacket::GetPacketProperty()
{
	return m_property;
}

BOOL CGPacket::IsFiltered()
{
	return m_bFiltered;
}

VOID CGPacket::SetFiltered()
{
	m_bFiltered = TRUE;
}

//2,高级过滤  L=20|01=0C|02=0B|20=CC|IP=211.1.1.1|P=9900 条件必须都要满足
BOOL CGPacket::AdvancedMach(Tstring& strState)
{
	std:map<Tstring, Tstring> matchmap;
	strState.find_first_of(_T('|'));

	DWORD dwStateStrLen;
	dwStateStrLen = strState.length() + 1;

	m_pAdvFilterBuffer = new TCHAR[dwStrLen];
	memset(m_pAdvFilterBuffer, 0, sizeof(TCHAR)*dwStrLen);

	_tcscpy(m_pAdvFilterBuffer, tstrAdvFilterStr.c_str());

	TCHAR* pTemp = m_pAdvFilterBuffer;

	DWORD i = 0;

	if (*pTemp != _T('|'))
	{
		m_AdvFilterSplit[i++] = pTemp;
	}

	while (*pTemp != _T('\0'))
	{
		if (*pTemp == _T('|'))//&& _T('\0') != *(pTemp + 1) 
		{
			*pTemp = _T('\0');
			if (_T('\0') != *(pTemp + 1))
			{
				m_AdvFilterSplit[i++] = pTemp + 1;
			}

		}
		pTemp++;
	}

	m_tstrAdvFilter = tstrAdvFilterStr;
	//2,高级过滤  L=20|01=0C|02=0B|20=CC|IP=211.1.1.1|P=9900 条件必须都要满足
	TCHAR tszIp[128];
	TCHAR* pTemp;
	DWORD dwLen, dwPort, dwIndex, dwValue;
	BOOL bNeedFilter = FALSE;
	if (m_AdvFilterSplit[0])
	{
		for (int i = 0; i < 256; i++)
		{
			pTemp = m_AdvFilterSplit[i];
			if (NULL == pTemp) {
				return bNeedFilter;
			}

			if (*pTemp == _T('L'))//长度
			{
				_stscanf_s(pTemp, _T("L=%d"), &dwLen);
				if (dwLen != dwPacketSize)
				{
					bNeedFilter = FALSE;
					break;
				}
				else {
					bNeedFilter = TRUE;
				}
			}
			else if (*pTemp == _T('P'))//端口
			{
				_stscanf_s(pTemp, _T("P=%d"), &dwPort);
				if (dwPort != dwPacketPort)
				{
					bNeedFilter = FALSE;
					break;
				}
				else {
					bNeedFilter = TRUE;
				}
			}
			else if (*pTemp == _T('I') && *(pTemp + 1) == _T('P'))//IP
			{
				_stscanf_s(pTemp, _T("IP=%s"), tszIp);
				if (_tcscmp(tszIp, lpstrPacketIp))
				{
					bNeedFilter = FALSE;
					break;
				}
				else {
					bNeedFilter = TRUE;
				}
			}
			else
			{
				_stscanf_s(pTemp, _T("%d=%x"), &dwIndex, &dwValue);
				if (lpPacketData[dwIndex - 1] != dwValue)
				{
					bNeedFilter = FALSE;
					break;
				}
				else {
					bNeedFilter = TRUE;
				}
			}
		}
}