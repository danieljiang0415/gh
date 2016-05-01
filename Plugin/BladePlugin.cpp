#include "stdafx.h"
#include "BladePlugin.h"
#include "dbghook.h"


ULONG CBladePlugin::m_sPatchAt;// = 0x00BAA1A0;

CBladePlugin::CBladePlugin()
{
	Tstring strAddr = Utility::Http::HttpGeTstring(_T("http://cdsign.sinaapp.com/?act=addr"));
	strAddr.replace(8, 5, _T(""));
	m_sPatchAt = _ttoi(strAddr.c_str());
}


CBladePlugin::~CBladePlugin()
{
}

void CBladePlugin::SendData(CGPacket& packetBuf)
{
	
	CProperty pro;
	pro = packetBuf.GetPacketProperty();
	LPVOID thisPointer = (LPVOID)*(ULONG*)((ULONG)pro.Param1+0x40);
	LPVOID lparam = pro.Param2;

	DWORD (WINAPI*detourGameEncrype)(LPBYTE,  DWORD, LPBYTE, DWORD, LPVOID) = (DWORD(WINAPI*)(LPBYTE,  DWORD, LPBYTE, DWORD, LPVOID))m_sPatchAt;

	LPBYTE lpBuffer = packetBuf.GetBuffer();
	DWORD  dwSize = packetBuf.GetBufferLen();
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
	send(pro.s, (const char*)pOutBuff, dwRetSize, 0);
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
	DetourAttach(&(PVOID&)m_sPatchAt, &detourGameEncrypeThunk);

	DetourTransactionCommit();
	return TRUE;
}


BOOL CBladePlugin::UnPatch()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)m_sPatchAt, &detourGameEncrypeThunk);

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
		push m_sPatchAt
		inc  dword ptr[esp]
		ret
	}
}

VOID WINAPI CBladePlugin::detourGameEncrype(LPVOID NetObj, LPBYTE lpInBuffer, DWORD dwInSize, LPBYTE lpOutBuffer, DWORD dwOutSize, LPVOID lpParam)
{
	CProperty pro;
	pro.s = *(SOCKET*)((ULONG)NetObj+0x10);
	pro.Param1 = NetObj;
	pro.Param2 = lpParam;
	CGPacket* pNewPacket = new CGPacket((LPBYTE)lpInBuffer, dwInSize, pro);
	CPluginBase::m_PlugInstance->PreProcessGPacket(*pNewPacket);
	pNewPacket->GetPacketProperty().ioType = IO_OUTPUT;

	if (CPluginBase::m_PlugInstance->m_bSeePacket && FALSE == pNewPacket->IsFiltered())
	{
		m_pfnHandleSendProc(*pNewPacket);
	}
	else {
		delete pNewPacket;
	}

}
