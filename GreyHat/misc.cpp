#include "stdafx.h"
#include "GmHelper.h"
#include "misc.h"
#include "Spyer.h"

//#pragma comment(lib, "Ws2_32.lib")
//Log LogObj;

extern const unsigned long  g_ulCodeSearchFrom;
extern const unsigned char  g_byCodePattern[];

extern GAME_DEFINE_CALL g_fnUser;
extern PFN_API_SEND		g_pfnsend;
extern PFN_API_RECV		g_pfnrecv;
extern PFN_API_WSASEND	g_pfnWSASend;

//
extern unsigned long	g_ulRadomDetourAddress;
LPVOID					g_lpCoWorkObj;

void __declspec(naked) detour_random(void)
{

	PUSH_REGISTERS
	__asm push    0                             //----------------->reserved3
	__asm push    0                             //real game data -->reserved2
	__asm push    0						        //Magic_code------->reserved1
	__asm push    0                             //game object------>reserved0
	__asm push    dword ptr ds:[esp+54h]        //SIZE
	__asm push    dword ptr ds:[esp+54h]		//DATA
	__asm push    dword ptr ds:[esp+54h]  		//SOCKET
	__asm call    CGameDataHacker::DataHandleProc
	POP_REGISTERS

	/*return to Origin functor*/
	__asm mov	  eax, g_ulRadomDetourAddress
	__asm jmp	  eax

}
extern DWORD g_dwSerialNum;
extern BOOL  g_bIsSend;
void gamesend(LPBYTE byteBuffer, DWORD dwlen, LPVOID lpParam)
{
    EXTRA_DATA_INFO *pExtraData = (EXTRA_DATA_INFO*)lpParam;

	g_bIsSend = TRUE;
	//*(DWORD*)&byteBuffer[11] = ++g_dwSerialNum;
    send(pExtraData->skt, (const char*)byteBuffer, dwlen,0);
} 


void 
USERCALL
detour_User(LPBYTE pbyteData, LPBYTE lpOutBuffer)
{
    SOCKET s = INVALID_SOCKET;
    DWORD dwSize;
    LPVOID lpParam;
    __asm  mov dwSize,  edx
    __asm  mov lpParam, eax
    __asm  mov ebx, dword ptr ds:[eax+58h]
    __asm  mov s, ebx
    CGameDataHacker::DataHandleProc(s, (BYTE*)pbyteData, dwSize, lpParam);
    
    __asm  mov  edx, dwSize 
    __asm  mov  eax, lpParam
    __asm  push lpOutBuffer
    __asm  push pbyteData
    __asm  call g_fnUser
}
int 
WINAPI 
detour_send(SOCKET s, const char* buf, int len, int flags)
{
    CGameDataHacker::DataHandleProc(s, (BYTE*)buf, len);
    return g_pfnsend(s, buf, len, flags);
}
int 
WINAPI
detour_recv(SOCKET s, char FAR * buf, int len, int flags)
{
    return g_pfnrecv(s, buf, len, flags);    
}

int
WSAAPI
detour_wsasend(
		IN SOCKET s,
		IN LPWSABUF lpBuffers,
		IN DWORD dwBufferCount,
		OUT LPDWORD lpNumberOfBytesSent,
		IN DWORD dwFlags,
		IN LPWSAOVERLAPPED lpOverlapped,
		IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
		)
{
	CGameDataHacker::DataHandleProc(s, (BYTE*)lpBuffers->buf, lpBuffers->len);
	return g_pfnWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}

/////////////////////Ó²¼þ¶ÏµãÉèÖÃ//////////////////////
#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))
#define MakeDelta(cast, x, y) (cast) ( (DWORD_PTR)(x) - (DWORD_PTR)(y))
HANDLE g_hVectorHandle;
ULONG  g_NewAddress, g_DetourPoint;
BOOL InstallHardHook(ULONG DetourPoint, ULONG NewAddress)
{
	g_hVectorHandle = AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)VectoredHandler);

	return SetHardHook(DetourPoint, NewAddress);
}

BOOL SetHardHook(ULONG DetourPoint, ULONG NewAddress)
{
	BOOL bRet;
	HANDLE thSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);

	g_NewAddress = NewAddress;
	g_DetourPoint=DetourPoint;
	Thread32First(thSnap, &te);
	do
	{  
		DWORD dwErrCode;
		if(te.th32OwnerProcessID != GetCurrentProcessId()||
			te.th32ThreadID == GetCurrentThreadId())
			continue;
		HANDLE hThread = OpenThread(THREAD_SET_CONTEXT|THREAD_GET_CONTEXT|THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
		if (!hThread){
			dwErrCode = GetLastError();
			continue;
		}
		SuspendThread(hThread);
		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		GetThreadContext(hThread, &ctx);
		if(!ctx.Dr0)
		{
			ctx.Dr0 = MakePtr(UINT, DetourPoint, 0);
			ctx.Dr7 |= 0x00000001;
			bRet = TRUE;
		}
		else if(!ctx.Dr1)
		{
			ctx.Dr1 = MakePtr(UINT, DetourPoint, 0);
			ctx.Dr7 |= 0x00000004;
			bRet = TRUE;
		}
		else if(!ctx.Dr2)
		{
			ctx.Dr2 = MakePtr(UINT, DetourPoint, 0);
			ctx.Dr7 |= 0x00000010;
			bRet = TRUE;
		}
		else if(!ctx.Dr3)
		{
			ctx.Dr3 = MakePtr(UINT, DetourPoint, 0);
			ctx.Dr7 |= 0x00000040;
			bRet = TRUE;
		}
		else
			bRet = FALSE;

		ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		SetThreadContext(hThread, &ctx);
		ResumeThread(hThread);
		CloseHandle(hThread);
	} while(Thread32Next(thSnap, &te));

	return bRet;
}

void ClearHardHook()
{
	bool retval=false;
	//DWORD dwErrCode;
	HANDLE thSnap, hThread;
	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);

	thSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	Thread32First(thSnap, &te);
	do
	{
		if(te.th32OwnerProcessID != GetCurrentProcessId()||	te.th32ThreadID == GetCurrentThreadId())
			continue;
		hThread = OpenThread(THREAD_SET_CONTEXT|THREAD_GET_CONTEXT|THREAD_SUSPEND_RESUME, 	FALSE, te.th32ThreadID);
		if (!hThread)
			continue;
		SuspendThread(hThread);
		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		GetThreadContext(hThread, &ctx);
		ctx.Dr0 = 0;
		ctx.Dr7 &= 0xFFF0FFFE;	//	Clear the 16-19th and 1st bits
		ctx.Dr1 = 0;
		ctx.Dr7 &= 0xFF0FFFFB;
		ctx.Dr2 = 0;
		ctx.Dr7 &=0xF0FFFFEF;
		ctx.Dr3 = 0;
		ctx.Dr7 &=0xFFFFFBF;
		SetThreadContext(hThread, &ctx);
		ResumeThread(hThread);
		CloseHandle(hThread);
	} while(Thread32Next(thSnap, &te));

	if (g_hVectorHandle)
		RemoveVectoredExceptionHandler(g_hVectorHandle);

}
#define EXCEPTION_DEBUG_STRING 0x40010006

long VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
	switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_DEBUG_STRING:
		return EXCEPTION_CONTINUE_EXECUTION;
	case EXCEPTION_SINGLE_STEP:
		{
			if(ExceptionInfo->ContextRecord->Eip != MakePtr(UINT, g_DetourPoint, 0))
				return EXCEPTION_CONTINUE_SEARCH;
			ExceptionInfo->ContextRecord->Eip = MakePtr(UINT, g_NewAddress, 0);
				return EXCEPTION_CONTINUE_EXECUTION;
		}
		break;
	default:
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

void __declspec (naked) Thunk()
{
	__asm  push 0
	__asm  push 0
	__asm  push 0
	__asm  push dword ptr[esp+0ch]
	__asm  push dword ptr[esp+14h]
	__asm  push dword ptr[esp+1ch]
	__asm  push dword ptr[esi+40h]
	__asm  call	CGameDataHacker::DataHandleProc

	//__asm  mov  eax, 009B6000h
	__asm  call g_fnUser
	__asm  mov  eax, g_ulRadomDetourAddress
	__asm  add  eax, 05h
	__asm  jmp  eax
}