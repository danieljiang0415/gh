// common.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>
#include "Plugin.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

CPluginBase* pBox;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


BOOL APIENTRY Install(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc)
{
	pBox = new CPlugin;
	if (pBox)
		return pBox->InstallPlugin(pfnHandleInputProc, pfnHandleOutputProc);
	else
		return FALSE;
}

BOOL APIENTRY UnInstall()
{
	if (pBox)
	{
		BOOL ret = pBox->UnInstallPlugin();
		delete pBox;
		return ret;
	}
	else
		return FALSE;
}

VOID APIENTRY Send(CPacket& packetBuf)
{
	//ST_PACKET_PARAM *pExtra = (ST_PACKET_PARAM*)lpParam;
	//XLogDbgStr(TEXT("SOCKET=%08lx, lpBuff=%08lx, dwSize=%d"), pExtra->s, lpBuff, dwSize);
	//send(pExtra->s, (const char*)lpBuff, dwSize, 0);
	if (pBox)
	{
		pBox->SendData(packetBuf);
	}
		
}


void XLogDbgStr(LPCTSTR  lpcFormattedMessage, ... )
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
			_tfopen_s(&fp, TEXT("dbg.log"), TEXT("a")); 
			if(fp) 
			{ 
				TCHAR szDate[260], szTime[260]; 
				_tstrdate_s (szDate, sizeof(szDate)); _tstrtime_s(szTime, sizeof(szTime));
				_ftprintf (fp, TEXT("%s %s - [#%d][~%d]%s\n"), szDate, szTime, GetCurrentProcessId(),GetCurrentThreadId(),pLogBuff); 
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