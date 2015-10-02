#include "stdafx.h"
#include "dbghook.h"
#include <TlHelp32.h>


HW_BREAKINFO			g_ExceptionInfo[4];
HANDLE					g_hVectorHandle;
//cxt.Dr7 &= ~( 1 << 0 ); free dbg registers
/*
dw:The value need to set bit value
BitOffset: the offset of the bit
cbBits:the length need to set
bitValue:the new value
*/
inline
VOID
SetBits( DWORD& dw, int BitOffset, int cbBits, int BitValue )
{
	DWORD mask = ( 1 << cbBits ) - 1; 
	dw = ( dw & ~( mask << BitOffset ) ) | ( BitValue << BitOffset );
}

inline
DWORD
GetBits( DWORD& dw, int BitOffset, int cbBits )
{
	DWORD mask = ( 1 << cbBits ) - 1; 
	return ( (dw & ~( mask << BitOffset )) >> BitOffset) ;
}

DWORD 
GetFreeDrN(CONTEXT *pCxt)
{
	if		( 0 == (pCxt->Dr7 & 1) )//Dr0 is free
		return DEBUG_DR0_FREE;
	else if ( 0 == (pCxt->Dr7 & 4) ) // Dr1 is free
		return DEBUG_DR1_FREE;
	else if ( 0 == (pCxt->Dr7 & 16) ) // Dr2 is free
		return DEBUG_DR2_FREE;
	else if ( 0 == (pCxt->Dr7 & 64) ) // Dr3 is free
		return DEBUG_DR3_FREE;
	else
		return DEBUG_BREAK_BUSY;
}

DWORD
WINAPI 
InstDbgBreak2Thrd( DWORD dwThreadId, DWORD dwLineAddr,  DWORD dwAccessType, DWORD dwMonitorSize )
{
	HANDLE hThread;
	hThread = OpenThread( THREAD_SET_CONTEXT|THREAD_GET_CONTEXT|THREAD_SUSPEND_RESUME, 
		FALSE, dwThreadId );
	if ( hThread == NULL )
		return GetLastError();

	if ( -1 == SuspendThread( hThread ))
		return GetLastError();

	CONTEXT cxt = {0};
	cxt.ContextFlags = CONTEXT_DEBUG_REGISTERS|CONTEXT_FULL;
	if (!GetThreadContext( hThread, &cxt ))
		return GetLastError();

	DWORD dwFreeDbgReg = GetFreeDrN(&cxt);
	if (DEBUG_BREAK_BUSY != dwFreeDbgReg)
	{
		switch(dwFreeDbgReg)
		{
			case DEBUG_DR0_FREE: cxt.Dr0 = dwLineAddr; break;
			case DEBUG_DR1_FREE: cxt.Dr1 = dwLineAddr; break;
			case DEBUG_DR2_FREE: cxt.Dr2 = dwLineAddr; break;
			case DEBUG_DR3_FREE: cxt.Dr3 = dwLineAddr; break;
			default:return DEBUG_BREAK_UNKNOWN;
		}
		switch(dwAccessType)
		{
		case DEBUG_BREAK_READ:		SetBits(cxt.Dr7, 16 + dwFreeDbgReg*4, 2, 3);break;// set break when read the line addr bit 16~17 11
		case DEBUG_BREAK_WRITE:		SetBits(cxt.Dr7, 16 + dwFreeDbgReg*4, 2, 1);break;// set break when write the line addr bit 16~17 01
		case DEBUG_BREAK_EXECUTE:	SetBits(cxt.Dr7, 16 + dwFreeDbgReg*4, 2, 0);break;// set break when execute the line addr bit 16~17 00
		default: return DEBUG_BREAK_UNKNOWN;
		}
		switch (dwMonitorSize)
		{
		case DEBUG_BREAK_On1:	SetBits(cxt.Dr7, 18 + dwFreeDbgReg*4, 2, 0); break; //monitor 1 bytes bit 18~19 00
		case DEBUG_BREAK_On2:	SetBits(cxt.Dr7, 18 + dwFreeDbgReg*4, 2, 1); break;//monitor 2 bytes bit 18~19 01
		case DEBUG_BREAK_On4:	SetBits(cxt.Dr7, 18 + dwFreeDbgReg*4, 2, 3); break;//monitor 8 bytes bit 18~19 11
		case DEBUG_BREAK_On8:	SetBits(cxt.Dr7, 18 + dwFreeDbgReg*4, 2, 2); break;//monitor 4 bytes bit 18~19 10
		default: return DEBUG_BREAK_UNKNOWN;
		}
		SetBits(cxt.Dr7, dwFreeDbgReg*2, 1, 1);

	}
	cxt.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	if(!SetThreadContext( hThread, &cxt ))
		return GetLastError();
	if(-1 == ResumeThread( hThread ))
		return GetLastError();
	return 0;
}

DWORD
UninstDbgBreakfromThrd( DWORD dwThreadId, DWORD dwLineAddr,  DWORD dwAccessType)
{
	HANDLE hThread;
	hThread = OpenThread( THREAD_SET_CONTEXT|THREAD_GET_CONTEXT|THREAD_SUSPEND_RESUME, 
		FALSE, dwThreadId );
	if ( hThread == NULL )
		return GetLastError();

	if ( -1 == SuspendThread( hThread ))
		return GetLastError();

	CONTEXT cxt = {0};
	cxt.ContextFlags = CONTEXT_DEBUG_REGISTERS|CONTEXT_FULL;
	if (!GetThreadContext( hThread, &cxt ))
		return GetLastError();

	if (cxt.Dr0 == dwLineAddr)
	{
		if (dwAccessType == GetBits(cxt.Dr7, 16, 2))
		{
			SetBits(cxt.Dr7, DR7_L0, 1, 0);
		}
	}
	if (cxt.Dr1 == dwLineAddr)
	{
		if (dwAccessType == GetBits(cxt.Dr7, 20, 2))
		{
			SetBits(cxt.Dr7, DR7_L1, 1, 0);
		}
	}
	if (cxt.Dr2 == dwLineAddr)
	{
		if (dwAccessType == GetBits(cxt.Dr7, 24, 2))
		{
			SetBits(cxt.Dr7, DR7_L2, 1, 0);
		}
	}
	if (cxt.Dr3 == dwLineAddr)
	{
		if (dwAccessType == GetBits(cxt.Dr7, 28, 2))
		{
			SetBits(cxt.Dr7, DR7_L3, 1, 0);
		}
	}
	
	cxt.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	if(!SetThreadContext( hThread, &cxt ))
		return GetLastError();
	if(-1 == ResumeThread( hThread ))
		return GetLastError();
	return 0;
}


BOOL HwDetourAttach( DWORD dwLineAddress, DWORD dwType, DWORD dwMonitorLen, DWORD dwExceptionHandler)
{
	HANDLE hThreadSnap;
	THREADENTRY32 te32;
	static BOOL bExceptionHandlerSet = FALSE;

	if (bExceptionHandlerSet == FALSE)
	{
		g_hVectorHandle = 
			AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)VectoredHandler);
		bExceptionHandlerSet = !bExceptionHandlerSet;
	}


	BOOL bHaveFreeDbgBreak = FALSE;
	for(int i = 0; i < 4; i++)
	{
		if( TRUE == g_ExceptionInfo[i].bIsFree)
		{
			bHaveFreeDbgBreak = TRUE;
			break;
		}
	}
	if (bHaveFreeDbgBreak == FALSE)
	{
		return FALSE;
	}
	hThreadSnap =  CreateToolhelp32Snapshot(  TH32CS_SNAPTHREAD, NULL  );
	if(  hThreadSnap == INVALID_HANDLE_VALUE  ) 
		return FALSE; 
	te32.dwSize = sizeof(  THREADENTRY32  ); 
	if(  Thread32First(  hThreadSnap, &te32 ) )
	{
		do
		{  
			if(  te32.th32OwnerProcessID != GetCurrentProcessId() || 
				 te32.th32ThreadID		 == GetCurrentThreadId() )
				continue;

			InstDbgBreak2Thrd(  te32.th32ThreadID, dwLineAddress, dwType, dwMonitorLen );


		} while(  Thread32Next(  hThreadSnap, &te32 ) );
	}

	for(int i = 0; i < 4; i++)
	{
		if( TRUE == g_ExceptionInfo[i].bIsFree)
		{
			g_ExceptionInfo[i].bIsFree = FALSE;
			g_ExceptionInfo[i].dwLineAddr = dwLineAddress;
			g_ExceptionInfo[i].dwAccessType = dwType;
			g_ExceptionInfo[i].dwExceptionHanler = dwExceptionHandler;
			break;
		}
	}
	return TRUE;
}

BOOL HwDetourDetach( DWORD dwLineAddress, DWORD dwType )
{
	HANDLE hThreadSnap;
	THREADENTRY32 te32;

	static BOOL bExceptionHandlerUnSet = FALSE;

	if (bExceptionHandlerUnSet == FALSE)
	{
		RemoveVectoredExceptionHandler(g_hVectorHandle);
		bExceptionHandlerUnSet = !bExceptionHandlerUnSet;
	}

	hThreadSnap =  CreateToolhelp32Snapshot(  TH32CS_SNAPTHREAD, NULL  );
	if( hThreadSnap == INVALID_HANDLE_VALUE ) return  FALSE; 

	te32.dwSize = sizeof(  THREADENTRY32  ); 
	if(  Thread32First(  hThreadSnap, &te32 ) )
	{
		do
		{  
			if(  te32.th32OwnerProcessID != GetCurrentProcessId() || 
				 te32.th32ThreadID == GetCurrentThreadId() )
				continue;

			UninstDbgBreakfromThrd( te32.th32ThreadID,dwLineAddress, dwType);

		} while(  Thread32Next(  hThreadSnap, &te32 ) );
	}

	for(int i = 0; i < 4; i++)
	{
		if( dwLineAddress == g_ExceptionInfo[i].dwLineAddr)
		{
			g_ExceptionInfo[i].bIsFree = TRUE;
			break;
		}
	} 
	return TRUE;
}



LONG
CALLBACK
VectoredHandler(  PEXCEPTION_POINTERS ExceptionInfo )
{
	switch(  ExceptionInfo->ExceptionRecord->ExceptionCode )
	{
	case DBG_PRINTEXCEPTION_C:
		return EXCEPTION_CONTINUE_EXECUTION;
	case EXCEPTION_SINGLE_STEP:
		{
			//MessageBox( NULL, L"Trag debugger now", L"info", MB_OK );
			for(int i = 0; i < 4; i++)
			{
				if( ExceptionInfo->ContextRecord->Eip == g_ExceptionInfo[i].dwLineAddr)
				{
					ExceptionInfo->ContextRecord->Eip = g_ExceptionInfo[i].dwExceptionHanler;
					return EXCEPTION_CONTINUE_EXECUTION;
				}
			}
			/*debug registers was not triggered, return continue search*/
			return EXCEPTION_CONTINUE_SEARCH;
		}
		break;
	default:
		return EXCEPTION_CONTINUE_SEARCH;
	}
}