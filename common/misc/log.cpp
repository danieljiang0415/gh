
#include "log.h"
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
namespace LOG
{
	void DbgPrint(const char*  lpcFormattedMessage, ... )
	{
		static CRITICAL_SECTION CriticalSection;
		static BOOL bInit = FALSE;
		if (!bInit)
		{
			InitializeCriticalSection(&CriticalSection);
			bInit = TRUE;
		}

		EnterCriticalSection(&CriticalSection);
		try
		{
			char buff[1024], logpath[256]; 
			va_list args; 
			va_start( args, lpcFormattedMessage ); 
			_vsnprintf_s( buff, _countof(buff), _TRUNCATE, lpcFormattedMessage, args ); 
			va_end( args );
			{ 
				FILE *fp;
				fopen_s(&fp, "hijack.log", "a"); 
				if(fp) 
				{ 
					char date[40], time[40]; 
					_strdate_s(date, sizeof(date)); _strtime_s(time, sizeof(time));
					fprintf(fp, "%s %s - [#%d][~%d]%s\n", date, time, GetCurrentProcessId(),GetCurrentThreadId(),buff); 
					fclose(fp); 
				} 
			}
		}
		catch(...)
		{

		}
		LeaveCriticalSection(&CriticalSection);
	}
}
