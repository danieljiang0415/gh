#pragma once
#include<string>
#include <tchar.h>
//using namespace std;

#ifdef _UNICODE
#define TString std::wstring
#else
#define TString std::string
#endif


class CHttp
{
public:
	CHttp();
	~CHttp();
	std::string Get(TString& strHost, TString& strUri);
	BOOL Post(TString& strHost, TString& strUri, LPVOID lpRawData, DWORD dwLength);
	LPVOID m_lpDataHtml;
};

