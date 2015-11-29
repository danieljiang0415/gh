#include "stdafx.h"
#include "DOMPlugin.h"

ULONG CDOMPlugin::m_ulPatchAddr = 0x006EE060;

CDOMPlugin::CDOMPlugin()
{
	Tstring strAddr = Utility::Http::HttpGeTstring(_T("http://cdsign.sinaapp.com"));
	strAddr.replace(7, 4, _T(""));
	m_ulPatchAddr = _ttoi(strAddr.c_str());
}


CDOMPlugin::~CDOMPlugin()
{
}

void CDOMPlugin::SendData(CPacket& packetBuf)
{
	CContext ctx;
	ctx = packetBuf.GetContext();
	LPVOID thisPointer = ctx.param1;

	bool (WINAPI*detourGameEncrype)(DWORD, LPBYTE) = (bool (WINAPI*)(DWORD, LPBYTE))m_ulPatchAddr;

	LPBYTE lpBuffer = packetBuf.GetRawData()+2;
	DWORD  dwSize = packetBuf.GetDataLen()-2;
	__asm
	{
		mov ecx, thisPointer
		push lpBuffer
		push dwSize
		call detourGameEncrype
	}
	send(ctx.s, (const char*)lpBuffer-2, dwSize+2, 0);
}
BOOL CDOMPlugin::InstallPlugin(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc)
{
	m_pfnHandleSendProc = pfnHandleInputProc;
	m_pfnHandleRecvProc = pfnHandleOutputProc;

	return PatchUserDefineAddr();
}
BOOL CDOMPlugin::UnInstallPlugin()
{
	return UnPatch();
}

BOOL CDOMPlugin::PatchUserDefineAddr()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)m_ulPatchAddr, &detourGameEncrypeThunk);

	DetourTransactionCommit();
	return TRUE;
}


BOOL CDOMPlugin::UnPatch()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)m_ulPatchAddr, &detourGameEncrypeThunk);

	DetourTransactionCommit();
	return TRUE;
}

VOID __declspec(naked) CDOMPlugin::detourGameEncrypeThunk()
{
	__asm
	{
		pushad
		pushfd
		push    dword ptr ds : [esp + 2ch]        //SIZE( dwBufferCount )
		push    dword ptr ds : [esp + 2ch]		//DATA( LPWSABUF )
		push    ecx
		call    detourGameEncrype
		popfd
		popad
		jmp     m_ulPatchAddr
	}
}

VOID WINAPI CDOMPlugin::detourGameEncrype(LPVOID lpParam , DWORD dwSize, LPBYTE lpBuffer)
{
	CContext ctx;
	ctx.s = *(SOCKET*)lpParam;
	ctx.param1 = lpParam;
	CPacket* packetBuf = new CPacket((LPBYTE)lpBuffer-2, dwSize+2, ctx);
	packetBuf->SetType(IO_SEND);

	m_pfnHandleSendProc(*packetBuf);
}
