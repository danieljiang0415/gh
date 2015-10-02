#include "stdafx.h"
#include "Plugin.h"


ULONG CPlugin::m_offset12FromSend;
ULONG CPlugin::m_offset12FromWSASend;
ULONG CPlugin::m_offset12FromSend2;
ULONG CPlugin::m_offsetRecv;
ULONG CPlugin::m_offset12FromWSARecv;
ULONG CPlugin::m_offset12FromRecvFrom;
ULONG CPlugin::m_offset12FromWSAReceFrom;


CPlugin::CPlugin()
{
}


CPlugin::~CPlugin()
{
}

void CPlugin::SendData(CPacket& packetBuf)
{
	CContext ctx;
	ctx = packetBuf.GetContext();
	send(ctx.s, (const char*)packetBuf.GetRawData(), packetBuf.GetDataLen(), 0);
}
BOOL CPlugin::InstallPlugin(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	m_offset12FromSend = (ULONG)(&send) + 12;
	DetourAttach(&(PVOID&)m_offset12FromSend, &Send12Thunk);

	m_offset12FromWSASend = (unsigned long)(GetProcAddress(GetModuleHandleA("ws2_32.dll"), "WSASend")) + 0x000000012;
	DetourAttach(&(PVOID&)m_offset12FromWSASend, &WSASend12Thunk);

	m_offset12FromSend2 = (ULONG)(&sendto) + 12;
	DetourAttach(&(PVOID&)m_offset12FromSend2, &SendTo12Thunk);

	m_offsetRecv = (ULONG)(&recv);
	DetourAttach(&(PVOID&)m_offsetRecv, &StubRecv);

	DetourTransactionCommit();

	return TRUE;
}
BOOL CPlugin::UnInstallPlugin()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)m_offset12FromSend, &Send12Thunk);
	DetourDetach(&(PVOID&)m_offset12FromWSASend, &WSASend12Thunk);
	DetourDetach(&(PVOID&)m_offset12FromSend2, &SendTo12Thunk);
	DetourDetach(&(PVOID&)m_offsetRecv, &StubRecv);

	DetourTransactionCommit();
	return TRUE;
}


VOID APIENTRY CPlugin::WSASendThunkHandler(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount)
{
	for (unsigned int i = 0; i<dwBufferCount; i++)
	{
		m_pfnHandleSendProc(s, lpBuffers[i].buf, lpBuffers[i].len, NULL, NULL, NULL, NULL);
	}
}

VOID __declspec(naked) CPlugin::WSASend12Thunk()
{
	__asm
	{
		pushad
		pushfd
		push    dword ptr ds : [esp + 40h]        //SIZE( dwBufferCount )
		push    dword ptr ds : [esp + 40h]		//DATA( LPWSABUF )
		push    dword ptr ds : [esp + 40h]
		call    WSASendThunkHandler
		popfd
		popad
		jmp     m_offset12FromWSASend
	}
}
VOID APIENTRY CPlugin::SendThunkHandler(SOCKET s, const char* buf, int nlen)
{
	m_pfnHandleSendProc(s, buf, nlen, NULL, NULL, NULL, NULL);
}
VOID __declspec(naked) CPlugin::Send12Thunk(void)
{
	__asm
	{
		pushad
		pushfd
		push    dword ptr ds : [esp + 4ch]        //SIZE( dwBufferCount )
		push    dword ptr ds : [esp + 4ch]		//DATA( LPWSABUF )
		push    dword ptr ds : [esp + 4ch]  		//SOCKET
		call    SendThunkHandler
		popfd
		popad
		jmp     m_offset12FromSend
	}
}

VOID APIENTRY CPlugin::SendToThunkHandler(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
	m_pfnHandleSendProc(s, buf, len, NULL, NULL, NULL, NULL);
}
VOID __declspec(naked) CPlugin::SendTo12Thunk(void)
{
	__asm
	{
		pushad
		pushfd
		push    dword ptr ds : [esp + 58h]        //tolen
		push    dword ptr ds : [esp + 58h]        //to
		push    dword ptr ds : [esp + 58h]        //flags
		push    dword ptr ds : [esp + 58h]        //len
		push    dword ptr ds : [esp + 58h]		//buf
		push    dword ptr ds : [esp + 58h]  		//s
		call    SendToThunkHandler
		popfd
		popad
		jmp     m_offset12FromSend2
	}
}

int WSAAPI CPlugin::StubRecv(SOCKET s, char* buf, int bufsize, int flag)
{
	int retSize = ((int (WSAAPI *)(SOCKET, char*, int, int))m_offsetRecv)(s, buf, bufsize, flag);
	if (retSize != 0 && retSize != -1)
	{
		m_pfnHandleRecvProc((LPBYTE)buf, retSize);
	}
	return retSize;
}