#pragma once


#define DEBUG_DR0_FREE	(DWORD)(0)
#define DEBUG_DR1_FREE	(DWORD)(1)
#define DEBUG_DR2_FREE	(DWORD)(2)
#define DEBUG_DR3_FREE	(DWORD)(3)
#define DEBUG_BREAK_BUSY (DWORD)(-500)


#define DEBUG_BREAK_READ    0x00000003
#define DEBUG_BREAK_WRITE   0x00000001
#define DEBUG_BREAK_EXECUTE 0x00000000
#define DEBUG_BREAK_UNKNOWN (DWORD)(-501)

#define DEBUG_BREAK_On1	0x00000001
#define DEBUG_BREAK_On2	0x00000002
#define DEBUG_BREAK_On4	0x00000003
#define DEBUG_BREAK_On8	0x00000004
#define DEBUG_BREAK_OnUnknown	(DWORD)(-502)


#define DR7_L0 0x00000000
#define DR7_L1 0x00000002
#define DR7_L2 0x00000004
#define DR7_L3 0x00000006


BOOL HwDetourAttach( DWORD dwLineAddress, DWORD dwType, DWORD dwMonitorLen, DWORD dwExceptionHandler);
BOOL HwDetourDetach( DWORD dwLineAddress, DWORD dwType );
LONG CALLBACK VectoredHandler(  PEXCEPTION_POINTERS ExceptionInfo );
DWORD WINAPI InstDbgBreak2Thrd( DWORD dwThreadId, DWORD dwLineAddr,  DWORD dwAccessType, DWORD dwMonitorSize );


typedef struct _HW_BREAK_INFO
{
	DWORD dwLineAddr;
	DWORD dwAccessType;
	DWORD dwExceptionHanler;
	BOOL  bIsFree;

	_HW_BREAK_INFO()
	{
		dwLineAddr = 0;
		dwAccessType = 0;
		dwExceptionHanler = 0;
		bIsFree = TRUE;
	}

} HW_BREAKINFO, *PHW_BREAKINFO;

