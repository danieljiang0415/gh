#include "stdafx.h"
#include "Http.h"
#include<Wininet.h>


#define HTTP_ACCEPT_TYPES		_T("text/html, application/xhtml+xml,*/*\r\n")
#define HTTP_USER_AGENT			_T("Mozilla/5.0 (Windows NT 6.1; Trident/7.0; rv:11.0) like Gecko\r\n")
//#define HTTP_VERSION_IE			_T("HTTP/1.1\r\n")
//#define HTTP_ACCEPT_LAN			_T("zh-CN")
//#define HTTP_ACCEPT_ENCODING	_T("gzip, deflate")

#define READSIZE 1024

#pragma comment(lib, "Wininet.lib")

CHttp::CHttp()
{
	m_lpDataHtml = NULL;
}


CHttp::~CHttp()
{
	if (m_lpDataHtml)
		delete[]m_lpDataHtml;
	m_lpDataHtml = NULL;
}


std::string CHttp::Get(TString& strHost, TString& strUri)
{
	HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
	try
	{
		LPCTSTR rgpszAcceptTypes[] = {
			HTTP_ACCEPT_TYPES, NULL
		};

		hSession = ::InternetOpen(HTTP_USER_AGENT, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		if (NULL == hSession) {
			throw "InternetOpen";
		}

		// Connect to www.microsoft.com. 
		hConnect = ::InternetConnect(hSession, strHost.c_str(), 0, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
		if (NULL == hConnect) {
			throw "InternetConnect";
		}

		// Request the file /MSDN/MSDNINFO/ from the server. 
		hRequest = ::HttpOpenRequest(hConnect, TEXT("GET"), strUri.c_str(), TEXT("HTTP/1.1"), NULL, rgpszAcceptTypes, INTERNET_FLAG_RELOAD, 0);
		if (NULL == hRequest) {
			throw "HttpOpenRequest";
		}

		// Send the request. 
		if (!::HttpSendRequest(hRequest, NULL, 0, NULL, 0)) {
			throw "HttpSendRequest";
		}

		UCHAR szTemp[READSIZE] = {'\0'};
		DWORD dwRead;
		std::string strTmp;

		while (::InternetReadFile(hRequest, szTemp, READSIZE, &dwRead) && dwRead > 0)
		{
			if (dwRead < READSIZE)
			{
				szTemp[dwRead] = '\0';
			}
			strTmp += (char*)szTemp;
		}
//		TString strRet;
//#ifdef _UNICODE
//
//		wstring wstrTemp;
//		int nStrLen = (int)strTmp.length();
//		wstrTemp.resize(nStrLen, L' ');
//		int nResult = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)strTmp.c_str(), nStrLen, (LPWSTR)wstrTemp.c_str(), nStrLen);
//		strRet = wstrTemp;
//#else
//		strRet = strTmp;
//#endif
		return strTmp;
	}
	catch (...)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
	}
	return "";
}


BOOL CHttp::Post(TString& strHost, TString& strUri, LPVOID lpRawData, DWORD dwLength)
{
	HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
	try {
		hSession = InternetOpen(HTTP_USER_AGENT, INTERNET_OPEN_TYPE_DIRECT, NULL, INTERNET_INVALID_PORT_NUMBER, 0);
		if (NULL == hSession) {
			throw;
		}
		hConnect = InternetConnect(hSession, strHost.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
		if (NULL == hConnect) {
			throw;
		}
		LPCTSTR rgpszAcceptTypes[] = {
			HTTP_ACCEPT_TYPES, NULL
		};
		hRequest = HttpOpenRequest(hConnect, _T("POST"), strUri.c_str(), HTTP_VERSION, NULL, rgpszAcceptTypes, INTERNET_FLAG_DONT_CACHE, 1);
		if (NULL == hRequest) {
			throw;
		}
		HttpSendRequest(hRequest, NULL, 0, lpRawData, dwLength);
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return TRUE;
	}
	catch (...)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		return FALSE;
	}
}