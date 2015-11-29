#include "stdafx.h"
#include "BladePlugin.h"
#include "dbghook.h"


ULONG CBladePlugin::m_ulPatchAddr = 0x00BAA1A0;

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
	LPVOID thisPointer = (LPVOID)*(ULONG*)((ULONG)ctx.param1+0x40);
	LPVOID lparam = ctx.param2;

	DWORD (WINAPI*detourGameEncrype)(LPBYTE,  DWORD, LPBYTE, DWORD, LPVOID) = (DWORD(WINAPI*)(LPBYTE,  DWORD, LPBYTE, DWORD, LPVOID))m_ulPatchAddr;

	LPBYTE lpBuffer = packetBuf.GetRawData();
	DWORD  dwSize = packetBuf.GetDataLen();
	BYTE*	pOutBuff = new BYTE[0x1000];
	DWORD	dwRetSize = 0;
	__asm
	{
		push lparam
		push 0x1000
		push pOutBuff
		push dwSize
		push lpBuffer
		mov ecx, thisPointer
		call detourGameEncrype
		mov dwRetSize, eax
	}
	send(ctx.s, (const char*)pOutBuff, dwRetSize, 0);
	delete pOutBuff;
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
	DetourAttach(&(PVOID&)m_ulPatchAddr, &detourGameEncrypeThunk);

	DetourTransactionCommit();
	return TRUE;
}


BOOL CBladePlugin::UnPatch()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)m_ulPatchAddr, &detourGameEncrypeThunk);

	DetourTransactionCommit();
	return TRUE;
}

VOID __declspec(naked) CBladePlugin::detourGameEncrypeThunk()
{
	__asm
	{
		pushad
		pushfd
		push    dword ptr ds : [esp + 38h]
		push    dword ptr ds : [esp + 38h]
		push    dword ptr ds : [esp + 38h]
		push    dword ptr ds : [esp + 38h]
		push    dword ptr ds : [esp + 38h]
		push	esi
		call    detourGameEncrype
		popfd
		popad
		push ecx
		push m_ulPatchAddr
		inc  dword ptr[esp]
		ret
	}
}

VOID WINAPI CBladePlugin::detourGameEncrype(LPVOID NetObj, LPBYTE lpInBuffer, DWORD dwInSize, LPBYTE lpOutBuffer, DWORD dwOutSize, LPVOID lpParam)
{
	CContext ctx;
	ctx.s = *(SOCKET*)((ULONG)NetObj+0x10);
	ctx.param1 = NetObj;
	ctx.param2 = lpParam;
	CPacket* packetBuf = new CPacket((LPBYTE)lpInBuffer, dwInSize, ctx);
	packetBuf->SetType(IO_SEND);

	m_pfnHandleSendProc(*packetBuf);
}
