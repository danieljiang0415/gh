#include "stdafx.h"
#include "BladePlugin.h"
#include "dbghook.h"


ULONG CBladePlugin::m_ulPatchAddr = 0x00BA8600;

CBladePlugin::CBladePlugin()
{
	//Tstring strAddr = Utility::Http::HttpGeTstring(_T("http://cdsign.sinaapp.com"));
	//strAddr.replace(7, 4, _T(""));
	//m_ulPatchAddr = _ttoi(strAddr.c_str());
}


CBladePlugin::~CBladePlugin()
{
}

void CBladePlugin::SendData(CPacket& packetBuf)
{
	CContext ctx;
	ctx = packetBuf.GetContext();
	LPVOID thisPointer = ctx.param1;

	bool (WINAPI*Encrypt)(DWORD, LPBYTE) = (bool (WINAPI*)(DWORD, LPBYTE))m_ulPatchAddr;

	LPBYTE lpBuffer = packetBuf.GetRawData() + 2;
	DWORD  dwSize = packetBuf.GetDataLen() - 2;
	__asm
	{
		mov ecx, thisPointer
		push lpBuffer
			push dwSize
			call Encrypt
	}
	send(ctx.s, (const char*)lpBuffer - 2, dwSize + 2, 0);
}

BOOL CBladePlugin::InstallPlugin(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc)
{
	m_pfnHandleSendProc = pfnHandleInputProc;
	m_pfnHandleRecvProc = pfnHandleOutputProc;
	//return TRUE;
	return PatchUserDefineAddr();
}

BOOL CBladePlugin::UnInstallPlugin()
{
	//return TRUE;
	return UnPatch();
}

BOOL CBladePlugin::PatchUserDefineAddr()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)m_ulPatchAddr, &EncryptThunk);

	DetourTransactionCommit();
	return TRUE;
}


BOOL CBladePlugin::UnPatch()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)m_ulPatchAddr, &EncryptThunk);

	DetourTransactionCommit();
	return TRUE;
}

VOID __declspec(naked) CBladePlugin::EncryptThunk()
{
	__asm
	{
		pushad
		pushfd
		push    dword ptr ds : [esp + 28h]        //SIZE( dwBufferCount )
		push    dword ptr ds : [esp + 30h]		//DATA( LPWSABUF )
		push    esi
		call    Encrypt
		popfd
		popad
		push ecx
		push m_ulPatchAddr
		inc  dword ptr[esp]
		ret
	}
}

VOID WINAPI CBladePlugin::Encrypt(LPVOID lpParam, DWORD dwSize, LPBYTE lpBuffer)
{
	CContext ctx;
	ctx.s = *(SOCKET*)((ULONG)lpParam+0x10);
	ctx.param1 = lpParam;
	CPacket* packetBuf = new CPacket((LPBYTE)lpBuffer, dwSize, ctx);
	packetBuf->SetType(IO_SEND);

	m_pfnHandleSendProc(*packetBuf);
}
