#pragma once

#include <windows.h>
#include <string>
//#include <strsafe.h>
#include <ctype.h>
#include <time.h>
#include <wininet.h>
#include <Shlwapi.h>
#pragma comment(lib,"wininet.lib")
using namespace std;

#ifdef UNICODE
	typedef wchar_t tchar;
	typedef wstring Tstring;
#else // UNICODE
	typedef char   tchar;
	typedef string Tstring;
#endif // UNICODE

typedef unsigned char uint8_t;
typedef int  int32_t;
typedef unsigned int  uint32_t;

#ifdef UNICODE
	#define text(quote) L##quote
#else
	#define text(quote) quote
#endif

#define HTTP_READ_MAX_BUFFER_SIZE 4096

typedef VOID (CALLBACK* HTTP_READ_CALLBACK)(LPBYTE, DWORD);

namespace Utility
{
	namespace System
	{
		Tstring GetHostName();
	}

	namespace Process
	{
		Tstring GetCurrentProcessName();
	}
	
	namespace StringLib
	{
		string Wstring2String(wstring &wstr);
		wstring String2Wstring(string &str);

		BYTE Tchar2Hex( TCHAR ch );
		DWORD Tstring2Hex(__in LPCTSTR HexString, __out LPBYTE lpHexBuffer );
		Tstring Hex2Tstring( LPBYTE lpHexBuffer, DWORD dwLen );
		wstring hex2ascii( byte *ch, uint32_t len );
	}

	namespace Http
	{
		//typedef void (__stdcall*HTTPGETCALLBACK)(unsigned char*buf, unsigned int len);
		//bool http_post_request(std::wstring& wstrHost, std::wstring& strVerb, LPVOID lpOptional, DWORD dwOptionalLength);
		//BOOL HttpGetRequest( LPCTSTR szHost, LPCTSTR szUrl, LPVOID lpBuff, DWORD* pdwBufSize );
		//BOOL HttpDownload(LPCTSTR lpcUrl,LPCTSTR lpcLocalFile);
		BOOL HttpRead2File(LPCTSTR lpcInternetUrl,LPCTSTR lpcLocalFileName);
		Tstring HttpGeTstring(LPCTSTR lpcInternetUrl);
	}

	//namespace File
	//{
	//	bool IsFileExist(const Tstring& strfile);
	//	DWORD FileSize(const Tstring& strfile, DWORD* pdwFileSizeHigh);
	//	bool CopyFile2(Tstring& srcfile, Tstring& destfile);
	//}
	namespace File
	{
		BOOL IsFileExist(LPCTSTR lpcFilePath);
	}

	namespace Log
	{
		VOID XLogDbgStr(LPCTSTR  FmtStr, ... );
		void DbgPrint(LPCTSTR lpcFormat, ...);
	}

	namespace gui
	{
		//DWORD CaptureDeskTop(std::wstring& wstrSavePath);
        DWORD CaptureDeskTop2Mem(LPVOID lpRawBitMap);
	}
	
	namespace Module
	{
		Tstring GetModuleName(HMODULE hMod);
		Tstring GetModulePath(HMODULE hMod);
		VOID GetModulePath(HMODULE hMod, LPTSTR szModuleFullPath, DWORD dwSize);
		VOID GetModuleName(HMODULE hMod, LPTSTR lpFileName, DWORD nSize);
	}

	namespace IniAccess
	{
		Tstring GetPrivateKeyValString(Tstring config_file_path, Tstring section, Tstring key);
		void SetPrivateKeyValString(  Tstring config_file_path, Tstring section, Tstring key, Tstring value);
		UINT GetPrivateKeyValInt(Tstring tstrFileName, Tstring tstrSection, Tstring tstrKey);
	}

    namespace cipher
    {
        void CompressFile(std::wstring& wstrFileNeed2Compress);
        void DecompressFile(std::wstring& wstrFileNeed2Decompress);
        size_t CompressMemory(unsigned char* lpRawData, size_t cbRawSize, unsigned char* lpCipherData, size_t cbCipherBufSize);
        size_t DecompressMemory(unsigned char* lpCipherData, size_t cbCipherSize, unsigned char* lpRawData);
    }
	namespace Memory
	{
		LPVOID SearchInModuleImage( HMODULE lpModuleMemBase, LPBYTE lpSign, ULONG ulSignSize);
		LPVOID BooyerSearch(LPBYTE lpSrc, DWORD dwStringLen, LPBYTE lpSign, DWORD dwSignLen);
	}

	namespace UrlLib 
	{
		BOOL HttpRequest(Tstring& method, Tstring& url, LPVOID lpExtradata = NULL, LPBYTE lpbBuf = NULL, DWORD dwBufLen = 0, LPDWORD pdwBytesRead= 0);
	}

	namespace Msic
	{
		Tstring GenUUID();
	}
}

