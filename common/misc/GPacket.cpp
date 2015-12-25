#include "stdafx.h"
#include "GPacket.h"
#include <map>

CGPacket::CGPacket()
{
	m_pBuf		= NULL;
	m_pRawBuf	= NULL;
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
	m_pRawBuf = lpData;
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

				TCHAR szBuf[32];
				_itot(dwPort, szBuf, 10);
				m_property.strPort = Tstring(szBuf);
			}
		}
	}
}

LPBYTE CGPacket::GetBuffer()
{
	return m_pBuf;
}

LPBYTE CGPacket::GetRawBuffer()
{
	return m_pRawBuf;
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
		while (i > -1 && lpKey[i] != m_pRawBuf[j])
			i = kmpNext[i];
		i++;
		j++;
		if (i >= dwKeyLen) {
			//OUTPUT(j - i);
			memcpy_s(&m_pRawBuf[j - i], m_dwSize - j + i, lpReplace, dwReplaceLen);
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
BOOL CGPacket::AdvancedMach(Tstring& str)
{
	std::map<Tstring, Tstring> matchmap;
	int splitpos = 0, pos = 0;
	while (pos < str.length())
	{
		splitpos = str.find(_T('|'), pos);

		Tstring strSub;
		if (Tstring::npos != splitpos)
		{
			strSub = str.substr(pos, splitpos - pos);
		}
		else {
			strSub = str.substr(pos);
		}

		int assignpos;
		assignpos = strSub.find(_T('='));
		if (Tstring::npos != assignpos) {
			Tstring key, value;
			key = strSub.substr(0, assignpos);
			value = strSub.substr(assignpos+1);
			matchmap[key] = value;
		}
		pos = splitpos + 1;
	}
		
	//2,高级过滤  L=20|01=0C|02=0B|20=CC|IP=211.1.1.1|P=9900 条件必须都要满足
	std::map<Tstring, Tstring>::iterator i;
	for (i = matchmap.begin(); i != matchmap.end(); i++)
	{
		Tstring Key, Value;
		Key = (*i).first;
		Value = (*i).second;
		if (Key == Tstring(_T("L"))) {
			if (m_dwSize != _ttoi(Value.c_str())) 
			{
				return FALSE;
			}

		}
		else if (Key == Tstring(_T("IP"))) {
			if (m_property.strIpAddr != Value)
			{
				return FALSE;
			}
		}
		else if (Key == Tstring(_T("P"))) {
			if (m_property.strPort != Value)
			{
				return FALSE;
			}

		}
		else {
			if (m_pBuf[_ttoi(Key.c_str())] != _ttoi(Value.c_str()))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}