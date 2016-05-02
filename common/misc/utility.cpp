#include"stdafx.h"
#include "Utility.h"
#include <tchar.h>
//#include <strsafe.h>
#pragma comment(lib, "Shlwapi.lib")

#define HTTP_ACCEPT_TYPES _T("text/html, application/xhtml+xml,*/*\r\n")
#define HTTP_USER_AGENT _T("Mozilla/5.0 (Windows NT 6.1; Trident/7.0; rv:11.0) like Gecko\r\n")
#define HTTP_VERSION_IE _T("HTTP/1.1\r\n")
#define HTTP_ACCEPT_LAN _T("zh-CN")
#define HTTP_ACCEPT_ENCODING _T("gzip, deflate")

namespace Utility
{
	namespace System
	{
		Tstring GetHostName()
		{
			TCHAR tszBuffer[MAX_PATH];
			DWORD dwSize = MAX_PATH ;
			ZeroMemory(tszBuffer, MAX_PATH);

			if (TRUE == ::GetComputerName(tszBuffer, &dwSize))
				return Tstring (tszBuffer);
			else
				return Tstring (_T(""));
		}
	}

	namespace Process
	{
		Tstring GetCurrentProcessName()
		{
			TCHAR szModuleFullPath[MAX_PATH], *pModName;
			GetModuleFileName(NULL, szModuleFullPath, MAX_PATH);
			pModName = PathFindFileName(szModuleFullPath);;
			return Tstring(pModName);
		}
	}

	namespace File
	{
		BOOL IsFileExist(LPCTSTR lpcFilePath)
		{
			HANDLE hLocalFile;
			hLocalFile = CreateFile( lpcFilePath,GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if (hLocalFile == INVALID_HANDLE_VALUE){
				return FALSE;
			}else
			{
				CloseHandle(hLocalFile);
				return TRUE;
			}
		}
	}

    namespace Http
    {/*
        bool http_post_request(std::wstring& wstrHost, std::wstring& strVerb, LPVOID lpOptional, DWORD dwOptionalLength)
        {
			HINTERNET hSession = InternetOpen( HTTP_USER_AGENT, INTERNET_OPEN_TYPE_DIRECT,NULL, INTERNET_INVALID_PORT_NUMBER, 0 );
			if (NULL == hSession) {
				goto exit_proc;
			}

			HINTERNET hConnect = InternetConnect( hSession, wstrHost.c_str(),INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
			if ( NULL == hConnect) {
				InternetCloseHandle(hSession);
				goto exit_proc;
			}



		    LPCTSTR rgpszAcceptTypes[] =  {
				HTTP_ACCEPT_TYPES, NULL
			};

			HINTERNET hRequest = HttpOpenRequest(hConnect, text("POST"), strVerb.c_str(), HTTP_VERSION, NULL, rgpszAcceptTypes, INTERNET_FLAG_DONT_CACHE, 1);  
			if ( NULL == hRequest){
				InternetCloseHandle(hConnect);  
				InternetCloseHandle(hSession);
				goto exit_proc;

			}


			HttpSendRequest( hRequest, NULL, 0, lpOptional, dwOptionalLength);

			InternetCloseHandle(hRequest);  
			InternetCloseHandle(hConnect);  
			InternetCloseHandle(hSession);
exit_proc:
			return true;

        }*/

//		BOOL HttpGetRequest( LPCTSTR szHost, LPCTSTR szUrl, LPVOID lpBuff, DWORD* pdwBufSize )
//		{
//
//			//Utility::log::XLogDbgStr(TEXT("HttpGetRequest Host[%s], Url[%s]"), szHost, szUrl);
//			LPCTSTR rgpszAcceptTypes[] =  {
//				HTTP_ACCEPT_TYPES, NULL
//			};
//
//			HINTERNET hSession = ::InternetOpen( HTTP_USER_AGENT, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0) ; 
//			if (NULL == hSession) {
//				//Utility::log::XLogDbgStr(TEXT("InternetOpen False"));
//				return FALSE;
//			}
//
//			// Connect to www.microsoft.com. 
//			HINTERNET hConnect = ::InternetConnect(hSession, szHost, 0, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0) ; 
//			if ( NULL == hConnect )	{
//				//Utility::log::XLogDbgStr(TEXT("InternetConnect False"));
//				goto exit_with_session_success;
//			}
//
//			// Request the file /MSDN/MSDNINFO/ from the server. 
//			HINTERNET hRequest = ::HttpOpenRequest(hConnect, TEXT("GET"), szUrl,TEXT("HTTP/1.1"), NULL, rgpszAcceptTypes, INTERNET_FLAG_RELOAD, 0);
//			if (NULL == hRequest)	{
//				//Utility::log::XLogDbgStr(TEXT("HttpOpenRequest False"));
//				goto exit_with_connect_success;
//			}
//
//			// Send the request. 
//			if (!::HttpSendRequest(hRequest, NULL, 0, NULL, 0))	{
//				//Utility::log::XLogDbgStr(TEXT("HttpSendRequest FALSE"));
//				goto exit_with_request_success;
//			}
//			//Utility::log::XLogDbgStr(TEXT("HttpSendRequest OK"));
//			// Get the length of the file. 
//			//DWORD dwIndex = 0;
//			TCHAR szLength[32]; 
//			DWORD cbLength = sizeof(szLength); 
//			if(::HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH,szLength,&cbLength,NULL)) {
//
//				DWORD dwContentSize = (DWORD)_ttol(szLength) ; 
//
//				if ( lpBuff == NULL || *pdwBufSize < dwContentSize ) {
//					*pdwBufSize = dwContentSize;
//					//Utility::log::XLogDbgStr(TEXT("dwContentSize = %08lx"), dwContentSize);
//					InternetCloseHandle(hRequest);
//					InternetCloseHandle(hConnect);
//					InternetCloseHandle(hSession);
//					return TRUE;
//				}
//				else {
//					//Utility::log::XLogDbgStr(TEXT("InternetReadFile ==============>"));
//					if( !::InternetReadFile( hRequest, lpBuff, dwContentSize, pdwBufSize ) ){
//						//Utility::log::XLogDbgStr(TEXT("InternetReadFile FALSE"));
//						goto exit_with_request_success;
//					}else {
//						//Utility::log::XLogDbgStr(TEXT("All DONE!!"));
//							InternetCloseHandle(hRequest);
//							InternetCloseHandle(hConnect);
//							InternetCloseHandle(hSession);
//							return TRUE;
//					}
//				}
//			}
//
//
//			//Utility::log::XLogDbgStr(TEXT("RET FALSE"));
//
//exit_with_request_success:
//			InternetCloseHandle(hRequest);
//exit_with_connect_success:
//			InternetCloseHandle(hConnect);
//exit_with_session_success:
//			InternetCloseHandle(hSession);
//			return FALSE;
//		}

		

		BOOL HttpRead2File(LPCTSTR lpcInternetUrl,LPCTSTR lpcLocalFileName)
		{
			HINTERNET hInternet = NULL;
			HINTERNET hOpenUrl = NULL;	
			HANDLE hLocalFile;

			hLocalFile = CreateFile( lpcLocalFileName,GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
			if (hLocalFile == INVALID_HANDLE_VALUE){
				return FALSE;
			}

			hInternet = InternetOpen( NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
			if (hInternet == NULL) {
				CloseHandle(hLocalFile);
				return FALSE;
			}

			hOpenUrl = InternetOpenUrl( hInternet, lpcInternetUrl, NULL, 0,	
				INTERNET_FLAG_TRANSFER_BINARY|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD, 
				0 );

			if (hOpenUrl == NULL) {
				CloseHandle(hLocalFile);
				InternetCloseHandle(hInternet);
				return FALSE;
			}

			DWORD dwRead = 0, dwBytesToRead = HTTP_READ_MAX_BUFFER_SIZE;
			LPBYTE lpbTemp = new BYTE[HTTP_READ_MAX_BUFFER_SIZE];

			for(;;) {
				if (!InternetReadFile( hOpenUrl, lpbTemp, dwBytesToRead, &dwRead)) {
					CloseHandle(hLocalFile);
					InternetCloseHandle(hOpenUrl);
					InternetCloseHandle(hInternet);
					return FALSE;
				}
				if ( 0 == dwRead ) {
					break;
				}
				WriteFile( hLocalFile, lpbTemp, dwRead, &dwRead, NULL );
			}
			delete[]lpbTemp;
			CloseHandle(hLocalFile);
			InternetCloseHandle(hOpenUrl);
			InternetCloseHandle(hInternet);

			return TRUE;
		}

		Tstring HttpGeTstring(LPCTSTR lpcInternetUrl)
		{
			Tstring tstrTemp;
			HINTERNET hInternet = NULL;
			HINTERNET hOpenUrl = NULL;	


			hInternet = InternetOpen( NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
			if (hInternet == NULL) {
				return Tstring(_T(""));
			}

			hOpenUrl = InternetOpenUrl( hInternet, lpcInternetUrl, NULL, 0,	
				INTERNET_FLAG_RELOAD, 
				0 );

			if (hOpenUrl == NULL) {
				InternetCloseHandle(hInternet);
				return Tstring(_T(""));
			}

			DWORD dwRead = 0, dwBytesToRead = HTTP_READ_MAX_BUFFER_SIZE;
			LPSTR lpbTemp = new CHAR[HTTP_READ_MAX_BUFFER_SIZE];

			for(;;) {
				memset(lpbTemp, 0, HTTP_READ_MAX_BUFFER_SIZE);
				if (!InternetReadFile( hOpenUrl, lpbTemp, dwBytesToRead, &dwRead)) {
					InternetCloseHandle(hOpenUrl);
					InternetCloseHandle(hInternet);
					return Tstring(_T(""));
				}
				if ( 0 == dwRead ) {
					break;
				}
#ifdef _UNICODE
				tstrTemp += StringLib::String2Wstring(string(lpbTemp));
#else
				tstrTemp += Tstring(lpbTemp);
#endif
			}
			delete[]lpbTemp;
			InternetCloseHandle(hOpenUrl);
			InternetCloseHandle(hInternet);

			return tstrTemp;
		}

	}

	namespace StringLib
	{
		static const TCHAR HexTable[] = _T("0123456789ABCDEF");
		BYTE Tchar2Hex( TCHAR ch )
		{
			ch = _totupper( ch );
			switch ( ch ){
			case _T('0'): case _T('1'): case _T('2'): case _T('3'): case _T('4'):
			case _T('5'): case _T('6'): case _T('7'): case _T('8'): case _T('9'):
				return (ch - _T('0')) & 0x0f;
			case text('A'):
			case text('B'):
			case text('C'):
			case text('D'):
			case text('E'):
			case text('F'):
				return (ch - _T('A') + 10 ) & 0xf;
			default: 
				return 0;
			}
		}

		DWORD Tstring2Hex(__in LPCTSTR HexString, __out LPBYTE lpHexBuffer )
		{
			DWORD dwI, dwJ;
			TCHAR* plch;
			_tcslwr_s( plch = _tcsdup(HexString), _tcslen(HexString) + 1);

			dwI = 0, dwJ = 0;

			while ( plch[dwI] )
			{
				if ( ( plch[dwI] >= _T('0') && plch[dwI] <= _T('9') )||
					( plch[dwI] >= _T('a')&&plch[dwI] <= _T('f') ) )
				{
					lpHexBuffer[dwJ] = Tchar2Hex( plch[dwI] ) << 4;
					if ( ( plch[dwI + 1]==_T('\0') ) )
					{
						lpHexBuffer[dwJ] |= Tchar2Hex( _T('0') );
						dwJ++;
						break;
					}else
						lpHexBuffer[dwJ] |= Tchar2Hex( plch[dwI + 1] );
					dwI+=2;
					dwJ++;
				}
				else
				{
					dwI++;
				}
			}
			free(plch);
			return dwJ;
		}

		Tstring Hex2Tstring( LPBYTE lpHexBuffer, DWORD dwLen )
		{
			Tstring tstrTemp;
			if (dwLen > 0 && lpHexBuffer)
			{
				for (int i = 0; i < dwLen; i++ )
				{
					tstrTemp += HexTable[( lpHexBuffer[i] >> 4 ) & 0xf];
					tstrTemp += HexTable[lpHexBuffer[i] & 0xf];
					tstrTemp += _T("\x20");
				}
			}
			return tstrTemp;
		}

		//Tstring hex2ascii( BYTE*ch, int cbSize )
		//{
		//	std::wstring wstr(L"");
		//	for ( int i = 0; i < cbSize; i++ )
		//	{
		//		if ( (ch[i]&0x7F) >= 0x20 )
		//		{
		//			wchar_t wc;
		//			mbtowc( &wc, (const char*)&ch[i], sizeof(char) );
		//			wstr += wc;
		//		}else
		//		{
		//			wstr += L'.';
		//		}
		//		wstr += L" ";
		//	}
		//	return wstr;
		//}

		string Wstring2String(wstring &wstr)
		{
			string strTemp;
			int nStrLen = (int)wstr.length();    
			strTemp.resize(nStrLen,' ');
			int nResult = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)wstr.c_str(),nStrLen,(LPSTR)strTemp.c_str(),nStrLen,NULL,NULL);
			if (nResult == 0)
			{
				return string("");
			}
			return strTemp;
		}
		wstring String2Wstring(string &str)
		{
			wstring wstrTemp;
			int nStrLen = (int)str.length();    
			wstrTemp.resize(nStrLen,L' ');
			int nResult = MultiByteToWideChar(CP_ACP,0,(LPCSTR)str.c_str(),nStrLen,(LPWSTR)wstrTemp.c_str(),nStrLen);
			if (nResult == 0)
			{
				return wstring(L"");
			}
			return wstrTemp;
		}

	}
//    namespace GUI
//    {
//        DWORD CaptureDeskTop2Mem(LPVOID lpRawBitMap)
//        {
//			HDC hdcScreen;
//			HDC hdcMemDC = NULL;
//			HBITMAP hbmScreen = NULL;
//			BITMAP bmpScreen; 
//			hdcScreen = GetDC(NULL);
//
//			// Create a compatible DC which is used in a BitBlt from the window DC
//			hdcMemDC = CreateCompatibleDC(hdcScreen); 
//			if(!hdcMemDC)
//			{
////				log::XLogDbgStr("CreateCompatibleDC has failed");
//				goto done;
//			}
//
//			// Create a compatible bitmap from the Window DC
//			hbmScreen = CreateCompatibleBitmap(hdcScreen, GetSystemMetrics (SM_CXSCREEN),
//				GetSystemMetrics (SM_CYSCREEN));
//			if(!hbmScreen)
//			{
////				log::XLogDbgStr("CreateCompatibleBitmap Failed");
//				goto done;
//			}
//
//			// Select the compatible bitmap into the compatible memory DC.
//			SelectObject(hdcMemDC,hbmScreen);
//
//			// Bit block transfer into our compatible memory DC.
//			if(!BitBlt(hdcMemDC, 
//				0,0, 
//				GetSystemMetrics (SM_CXSCREEN),/*rcClient.right-rcClient.left,*/ /*rcClient.bottom-rcClient.top*/GetSystemMetrics (SM_CYSCREEN), 
//				hdcScreen/*hdcWindow*/, 
//				0,0,
//				SRCCOPY))
//			{
////				log::XLogDbgStr("BitBlt has failed");
//				goto done;
//			}
//
//			// Get the BITMAP from the HBITMAP
//			GetObject(hbmScreen,sizeof(BITMAP),&bmpScreen);
//
//			BITMAPFILEHEADER*   bmfHeader = (BITMAPFILEHEADER*)lpRawBitMap;    
//			BITMAPINFOHEADER*   bi		  = (BITMAPINFOHEADER*)((unsigned char*)lpRawBitMap + sizeof(BITMAPFILEHEADER));
//
//			bi->biSize = sizeof(BITMAPINFOHEADER);    
//			bi->biWidth = bmpScreen.bmWidth;    
//			bi->biHeight = bmpScreen.bmHeight;  
//			bi->biPlanes = 1;    
//			bi->biBitCount = 32;    
//			bi->biCompression = BI_RGB;    
//			bi->biSizeImage = 0;  
//			bi->biXPelsPerMeter = 0;    
//			bi->biYPelsPerMeter = 0;    
//			bi->biClrUsed = 0;    
//			bi->biClrImportant = 0;
//
//			DWORD dwBmpSize = ((bmpScreen.bmWidth * bi->biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;
//
//			// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
//			// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
//			// have greater overhead than HeapAlloc.
//			HANDLE hDIB = GlobalAlloc(GHND,dwBmpSize); 
//			char *lpbitmap = (char *)GlobalLock(hDIB);    
//
//			// Gets the "bits" from the bitmap and copies them into a buffer 
//			// which is pointed to by lpbitmap.
//			GetDIBits(hdcMemDC, hbmScreen, 0,
//				(UINT)bmpScreen.bmHeight,
//				lpbitmap,
//				(BITMAPINFO *)bi, DIB_RGB_COLORS);
//
//			// A file is created, this is where we will save the screen capture.
///*			HANDLE hFile = CreateFile(L"captureqwsx1111111.bmp",
//				GENERIC_WRITE,
//				0,
//				NULL,
//				CREATE_ALWAYS,
//				FILE_ATTRIBUTE_NORMAL, NULL);  */ 
//
//			// Add the size of the headers to the size of the bitmap to get the total file size
//			DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
//
//			//Offset to where the actual bitmap bits start.
//			bmfHeader->bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER); 
//
//			//Size of the file
//			bmfHeader->bfSize = dwSizeofDIB; 
//
//			//bfType must always be BM for Bitmaps
//			bmfHeader->bfType = 0x4D42; //BM   
//
//			//DWORD dwBytesWritten = 0;
//			//WriteFile(hFile, (LPSTR)bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
//			//WriteFile(hFile, (LPSTR)bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
//			//WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
//			memcpy((unsigned char*)lpRawBitMap + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), lpbitmap, dwBmpSize);
//			//Unlock and Free the DIB from the heap
//			GlobalUnlock(hDIB);    
//			GlobalFree(hDIB);
//
//			//Close the handle for the file that was created
//			//CloseHandle(hFile);
//
//			//Clean up
//done:
//			DeleteObject(hbmScreen);
//			//DeleteObject(hdcMemDC);
//			ReleaseDC(NULL,hdcScreen);
//			//ReleaseDC(hWnd,hdcWindow);
//			return dwSizeofDIB;
//        }
//    }
//
//    namespace cipher
//    {
//        void CompressFile(std::wstring& wstrFileNeed2Compress)
//        {
//
//        }
//        void DecompressFile(std::wstring& wstrFileNeed2Decompress)
//        {
//
//        }
//
//        size_t CompressMemory(unsigned char* lpRawData, size_t cbRawSize, unsigned char* lpCipherData, size_t cbCipherBufSize)
//        {
//            int err = SZ_OK;
//			unsigned char prop[5] = {0};
//			size_t sizeProp = 5;
//			size_t cbCipher = cbCipherBufSize;
//            unsigned int* pcbRawSize = (unsigned int*)lpCipherData;
//            *pcbRawSize = cbRawSize;
//			if (SZ_OK != (err = LzmaCompress(lpCipherData + sizeof(unsigned int) + sizeof(prop), &cbCipher, lpRawData, cbRawSize, prop,
//				&sizeProp, 9, (1 << 24), 3, 0, 2, 32, 2)))
//			{
////				log::XLogDbgStr("RawSize %d, CipherSize %d, ComMemory has failed = %d", cbRawSize, cbCipherBufSize, err);
//				return 0;
//			}
//            memcpy(lpCipherData + sizeof(unsigned int), prop, sizeof(prop));
//			return cbCipher + sizeof(unsigned int) + sizeof(prop);
//        }
//
//        size_t DecompressMemory(unsigned char* lpCipherData, size_t cbCipherSize, unsigned char* lpRawData)
//        {
//            int err = SZ_OK;
//			unsigned char prop[5] = {0};
//			size_t cbProp = sizeof(prop);
//			size_t cbRawSize = *(unsigned int*)lpCipherData;
//            size_t cbCipherDataSize = cbCipherSize - sizeof(unsigned int)- sizeof(prop);
//            memcpy(prop, lpCipherData + sizeof(unsigned int), sizeof(prop));
//
//			if (SZ_OK != (err = LzmaUncompress(lpRawData, &cbRawSize, lpCipherData + sizeof(unsigned int) + sizeof(prop), &cbCipherDataSize, prop, 5)))
//			{
////                log::XLogDbgStr("RawSize %d, CipherSize %d, LUncompr has failed = %d", cbRawSize, cbCipherSize, err);
//				return  0;
//			}
//			return cbRawSize;
//        }
//    }

	namespace Log
	{
		VOID XLogDbgStr(LPCTSTR  lpcFormattedMessage, ... )
		{
			static CRITICAL_SECTION CriticalSection;
			static BOOL bInit = FALSE;
			if (!bInit)
			{
				InitializeCriticalSection(&CriticalSection);
				bInit = TRUE;
			}

			EnterCriticalSection(&CriticalSection);
			unsigned int cbLogBufSize = 1024*1024;
			try
			{
				TCHAR* pLogBuff; 
				va_list args; 

				pLogBuff = new TCHAR[cbLogBufSize];
				va_start( args, lpcFormattedMessage ); 
				_vsntprintf_s( pLogBuff, cbLogBufSize, _TRUNCATE, lpcFormattedMessage, args ); 
				va_end( args );
				{ 
					FILE *fp;
					//Tstring strLogPath(Module::);
					_tfopen_s(&fp, text("XLogDbgStr.log"), text("a")); 
					if(fp) 
					{ 
						TCHAR szDate[260], szTime[260]; 
						_tstrdate_s (szDate, sizeof(szDate)); _tstrtime_s(szTime, sizeof(szTime));
						_ftprintf (fp, text("%s %s - [#%d][~%d]%s\n"), szDate, szTime, GetCurrentProcessId(),GetCurrentThreadId(),pLogBuff); 
						fclose(fp); 
					} 
				}
				delete[]pLogBuff;
			}
			catch(...)
			{

			}
			LeaveCriticalSection(&CriticalSection);
		}

		void DbgPrint(LPCTSTR lpcFormat, ...)
		{

			unsigned int cbLogBufSize = 1024;


			TCHAR* pLogBuff;
			va_list args;

			pLogBuff = new TCHAR[cbLogBufSize];
			va_start(args, lpcFormat);
			_vsntprintf_s(pLogBuff, cbLogBufSize, _TRUNCATE, lpcFormat, args);
			va_end(args);
			OutputDebugString(pLogBuff);
			delete[]pLogBuff;
		}
	}

	namespace Module
	{
		Tstring GetModuleName(HMODULE hMod)
		{
			TCHAR szModuleFullPath[MAX_PATH], *pModName;
			GetModuleFileName(hMod, szModuleFullPath, MAX_PATH);
			pModName = PathFindFileName(szModuleFullPath);;
			return Tstring(pModName);
		}
		
		Tstring GetModulePath(HMODULE hMod)
		{
			TCHAR szModuleFullPath[MAX_PATH], *pModName;
			GetModuleFileName(hMod, szModuleFullPath, MAX_PATH);
			PathRemoveFileSpec(szModuleFullPath);
			return Tstring(szModuleFullPath);
		}

		VOID GetModulePath(HMODULE hMod, LPTSTR szModuleFullPath, DWORD dwSize)
		{
			//TCHAR szModuleFullPath[MAX_PATH], *pModName;
			GetModuleFileName(hMod, szModuleFullPath, MAX_PATH);
			PathRemoveFileSpec(szModuleFullPath);
			//return Tstring(szModuleFullPath);
		}

		VOID GetModuleName(HMODULE hMod, LPTSTR lpFileName, DWORD nSize)
		{
			TCHAR szModName[MAX_PATH];
			PTCHAR pName; 

			GetModuleFileName(hMod, szModName, nSize);

			pName = PathFindFileName(szModName);
			_tcsncpy_s(lpFileName, MAX_PATH, pName, _tcslen(pName));
		}


		/*Tstring get_module_name(HINSTANCE hinst)
		{
			tchar szModuleFullPath[512];
			tchar *pName;
			GetModuleFileName(hinst, szModuleFullPath, 512);
			pName = PathFindFileName(szModuleFullPath);
			return Tstring(pName);
		}*/

		Tstring GetModulePath(Tstring& tstrDllName)
		{
			tchar szModuleFullPath[512];
			GetModuleFileName(GetModuleHandle(tstrDllName.c_str()), szModuleFullPath, 512);
			PathRemoveFileSpec(szModuleFullPath);
			return Tstring(szModuleFullPath);
		}
	}

	namespace IniAccess
	{
		Tstring GetPrivateKeyValString(Tstring strPathOfIni, Tstring strSectionName, Tstring strKeyName)
		{
			tchar tszBuf[1024] = {_T('\0')};
			GetPrivateProfileString(strSectionName.c_str(), strKeyName.c_str(), _T(""), tszBuf, 1024, strPathOfIni.c_str());
			return Tstring(tszBuf);
		}

		void SetPrivateKeyValString(Tstring strPathOfIni, Tstring strSectionName,Tstring strKeyName, Tstring strSetVal)
		{
			BOOL bRet;
			bRet = WritePrivateProfileString(strSectionName.c_str(), strKeyName.c_str(), strSetVal.c_str(), strPathOfIni.c_str());
			if (bRet == FALSE)
				DWORD dwErr = GetLastError();
		}

		UINT GetPrivateKeyValInt(Tstring strPathOfIni, Tstring strSectionName, Tstring strKeyName)
		{
			return GetPrivateProfileInt(strSectionName.c_str(), strKeyName.c_str(), 0, strPathOfIni.c_str());
		}
		
	}

	namespace Memory
	{
		#define ALPHABET_LEN 256

		void MakeDelta1(INT *Delta1, PBYTE pSign, DWORD dwSignLen) {
			INT iI;
			for ( iI=0; iI<ALPHABET_LEN; iI++ ) {
				Delta1[iI] = dwSignLen;
			}
			for ( iI=0; iI < dwSignLen-1; iI++ ) {
				Delta1[pSign[iI]] = dwSignLen - 1 - iI;
			}
		}

		BOOL IsPrefix(BYTE *Word, DWORD dwWordLen, DWORD dwPos) {
			INT iI;
			DWORD dwSuffixlen = dwWordLen - dwPos;
			for ( iI = 0; iI < dwSuffixlen; iI++) {
				if (Word[iI] != Word[dwPos + iI]) {
					return FALSE;
				}
			}
			return TRUE;
		}

		DWORD SuffixLength(BYTE *Word, DWORD dwWordLen, DWORD dwPos) {
			INT iI;
			for ( iI = 0; (Word[dwPos-iI] == Word[dwWordLen-1-iI]) && (iI<dwPos); iI++);

			return iI;
		}

		VOID MakeDelta2(INT *Delta2, BYTE *pSign, DWORD dwSignLen) {
			INT iI, iSuffixLen;
			INT LastPrefixIndex = dwSignLen-1;

			for ( iI=dwSignLen-1; iI>=0; iI-- ) {
				if (IsPrefix(pSign, dwSignLen, iI+1)) {
					LastPrefixIndex = iI+1;
				}
				Delta2[iI] = LastPrefixIndex + (dwSignLen - 1 - iI);
			}

			for ( iI=0; iI<dwSignLen-1; iI++) {
				iSuffixLen = SuffixLength(pSign, dwSignLen, iI);
				if (pSign[iI - iSuffixLen] != pSign[dwSignLen-1 - iSuffixLen]) {
					Delta2[dwSignLen-1 - iSuffixLen] = dwSignLen - 1 - iI + iSuffixLen;
				}
			}
		}

		LPVOID BooyerSearch(LPBYTE lpSrc, DWORD dwStringLen, LPBYTE lpSign, DWORD dwSignLen) {

			INT iI;
			INT Delta1[ALPHABET_LEN];
			INT *Delta2 = (INT*)malloc(dwSignLen * sizeof(INT));

			MakeDelta1(Delta1, lpSign, dwSignLen);
			MakeDelta2(Delta2, lpSign, dwSignLen);

			iI = dwSignLen - 1;

			while (iI < dwStringLen) {
				INT dwJ = dwSignLen - 1;
				while ( dwJ >= 0 && (lpSrc[iI] == lpSign[dwJ])) {
					--iI;
					--dwJ;
				}
				if ( dwJ < 0 ) {
					free(Delta2);
					return (lpSrc + iI + 1);
				}
				else
					iI += max(Delta1[lpSrc[iI]], Delta2[dwJ]);
			}
			free(Delta2);
			return NULL;
		}

		DWORD GetModuleImageSize( HMODULE hModuleBase )
		{
			DWORD dwResult = 0;
			PIMAGE_DOS_HEADER lpDosHeaders;
			PIMAGE_NT_HEADERS lpNtHeaders;

			//	验证模块是否是PE文件
			if (hModuleBase == NULL) 
				return dwResult;

			lpDosHeaders = (PIMAGE_DOS_HEADER)hModuleBase;

			if (lpDosHeaders->e_magic == IMAGE_DOS_SIGNATURE) {
				lpNtHeaders = (PIMAGE_NT_HEADERS)((DWORD)hModuleBase + lpDosHeaders->e_lfanew);
				if (lpNtHeaders->Signature == IMAGE_NT_SIGNATURE) {
					dwResult = lpNtHeaders->OptionalHeader.SizeOfImage;
				}
			}
			return dwResult;
		}

		LPVOID SearchInModuleImage( HMODULE lpModuleMemBase, LPBYTE lpSign, ULONG ulSignSize)
		{
			LPVOID lpMatchedAddr = NULL;
			ULONG ulMaxVirtualAddress;
			ULONG ulMemRegionStart;

			//get module end virtual address
			ulMaxVirtualAddress = (ULONG)lpModuleMemBase + GetModuleImageSize(lpModuleMemBase);
			ulMemRegionStart	= (ULONG)lpModuleMemBase;

			MEMORY_BASIC_INFORMATION MemInfoBuffer;
			while (ulMemRegionStart < ulMaxVirtualAddress) {
				if ( 0 != VirtualQuery((LPVOID)ulMemRegionStart, &MemInfoBuffer, sizeof(MEMORY_BASIC_INFORMATION)) ) {
					if ( MemInfoBuffer.State == MEM_COMMIT && MemInfoBuffer.Protect&0xf0 ) {
						lpMatchedAddr = BooyerSearch((LPBYTE)MemInfoBuffer.BaseAddress, MemInfoBuffer.RegionSize, lpSign, ulSignSize);
						if ( lpMatchedAddr ) {
							break;
						}
					}
					ulMemRegionStart += MemInfoBuffer.RegionSize;
				}
			}
			return lpMatchedAddr;
		}
	}

	namespace UrlLib {

		BOOL HttpRequest(Tstring& method, Tstring& url, LPVOID lpExtradata, LPBYTE lpbBuf, DWORD dwBufLen,LPDWORD pdwBytesRead) {

			HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;

			LPCTSTR rgpszAcceptTypes[] = {
				HTTP_ACCEPT_TYPES, NULL
			};
			try {
				if (NULL == (hSession = ::InternetOpen(HTTP_USER_AGENT, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0)))
				{
					throw;
				}
				TCHAR szOut[INTERNET_MAX_URL_LENGTH];
				DWORD ccOut = INTERNET_MAX_URL_LENGTH;
				UrlGetPart(url.c_str(), szOut, &ccOut, URL_PART_HOSTNAME, 0);
				if (NULL == (hConnect = ::InternetConnect(hSession, szOut, 0, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0)))
				{
					throw;
				}
				LPCTSTR lpLocaltion = UrlGetLocation(url.c_str());
				if (NULL == (hRequest = ::HttpOpenRequest(hConnect, method.c_str(), lpLocaltion, TEXT("HTTP/1.1"), NULL, rgpszAcceptTypes, INTERNET_FLAG_RELOAD, 0)))
				{
					throw;
				}
				if (!::HttpSendRequest(hRequest, NULL, 0, NULL, 0)) {
					throw;
				}

				TCHAR szLength[32];
				DWORD cbLength = sizeof(szLength);
				if (::HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH, szLength, &cbLength, NULL)) {

					DWORD dwContentLength = (DWORD)_ttol(szLength);
					DWORD dwNumberOfBytesRead = 0;
					if (dwContentLength > 0 && dwContentLength < 65535) {
						BYTE *lpBuff = new BYTE[dwContentLength];
						if (!::InternetReadFile(hRequest, lpBuff, dwContentLength, &dwNumberOfBytesRead)) {
							delete [] lpBuff;
							throw;
						}
						else
						{
							if (dwContentLength <= dwBufLen) {
								if( FALSE == IsBadWritePtr(lpbBuf, dwBufLen))
									memcpy(lpbBuf, lpBuff, dwContentLength);
							}
							*pdwBytesRead = dwContentLength;
							delete[] lpBuff;

							return TRUE;
						}
					}

				}
				else
					throw;

			}
			catch (...) {
				InternetCloseHandle(hRequest);
				InternetCloseHandle(hConnect);
				InternetCloseHandle(hSession);
				return FALSE;
			}
		}
	}
	//typedef int(__stdcall*USERPROC)(char*);
	namespace Msic
	{
		Tstring GenUUID()
		{
			GUID   guid;
			TCHAR szUUID[64];
			if (S_OK == ::CoCreateGuid(&guid))
			{
				_stprintf_s(szUUID, sizeof(szUUID),  _T("{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"), guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
					, guid.Data4[6], guid.Data4[7]);
			}
			return Tstring(szUUID);
		}
	}
}

namespace CommonLib
{
	void LOG(LPCTSTR lpszFormat, ...)
	{
		TCHAR     szBuffer[0x4000];
		TCHAR     szNewFormat[0x1000];

		_stprintf(szNewFormat, TEXT("#%d ~%d %s"), GetCurrentProcessId(), GetCurrentThreadId(), lpszFormat);

		va_list   args;
		va_start(args, lpszFormat);
		wvsprintf(szBuffer, szNewFormat, args);
		OutputDebugString(szBuffer);
		//FILE *fp;
		//_tfopen_s(&fp, TEXT("LOG.log"), TEXT("a"));
		//if (fp)
		//{
		//	//TCHAR szDate[260], szTime[260];
		//	//_tstrdate_s(szDate, sizeof(szDate)); _tstrtime_s(szTime, sizeof(szTime));
		//	_ftprintf(fp, TEXT("%s\n"), szBuffer);
		//	fclose(fp);
		//}
		va_end(args);
	}
}