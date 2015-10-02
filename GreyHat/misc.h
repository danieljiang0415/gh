#pragma once


//#define USERCALL WINAPI
//typedef void(USERCALL*GAME_DEFINE_CALL)(LPBYTE, int, LPBYTE, LPBYTE);
//
//void detour_random(void);
//void gamesend(LPBYTE byteBuffer, DWORD dwlen, LPVOID lPParam);
//
////=====================================================================================================================
//int 
//WINAPI 
//detour_send(SOCKET s, const char* buf, int len, int flags);
//int 
//WINAPI
//detour_recv(SOCKET s, char FAR * buf, int len, int flags);
////======================================================================================================================
//
//void 
//USERCALL
//detour_User(LPBYTE pbyteData, LPBYTE lpOutBuffer);
////=======================================================
//
//int
//WSAAPI
//detour_wsasend(SOCKET s,LPWSABUF lpBuffers,DWORD dwBufferCount,LPDWORD lpNumberOfBytesSent,DWORD dwFlags,LPWSAOVERLAPPED lpOverlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
//
//#define PUSH_REGISTERS    __asm pushad  __asm pushfd
//#define POP_REGISTERS     __asm popfd   __asm popad
//
//
//BOOL InstallHardHook(ULONG DetourPoint, ULONG NewAddress);
//BOOL SetHardHook(ULONG DetourPoint, ULONG NewAddress);
//VOID ClearHardHook();
//LONG VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo);
//void Thunk();