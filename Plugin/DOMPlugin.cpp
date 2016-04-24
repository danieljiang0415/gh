#include "stdafx.h"
#include "DOMPlugin.h"


#define CONFIG_INI _T("config.ini")

ULONG CDOMPlugin::m_ulPatchAddr = 0x006EE060;
extern HMODULE G_DLLModule;
CDOMPlugin::CDOMPlugin()
{
	//Tstring strAddr = Utility::Http::HttpGeTstring(_T("http://cdsign.sinaapp.com"));
	//strAddr.replace(7, 4, _T(""));
	//m_ulPatchAddr = _ttoi(strAddr.c_str());
}


CDOMPlugin::~CDOMPlugin()
{
}

void CDOMPlugin::SendData(CGPacket& packetBuf)
{
	CProperty ctx;
	ctx = packetBuf.GetPacketProperty();
	LPVOID thisPointer = ctx.Param1;

	bool (WINAPI*detourGameEncrype)(DWORD, LPBYTE) = (bool (WINAPI*)(DWORD, LPBYTE))m_ulPatchAddr;

	LPBYTE lpBuffer = packetBuf.GetBuffer()+2;
	DWORD  dwSize = packetBuf.GetBufferLen()-2;
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
	//DebugBreak();
	Tstring strDllPath = Utility::Module::GetModulePath(G_DLLModule);

	Tstring strCfgFile = strDllPath + Tstring(_T("\\")) + Tstring(CONFIG_INI);

	Tstring strShellCode = Utility::IniAccess::GetPrivateKeyValString(strCfgFile, _T("CODE"), _T("ShellCode"));

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	
	LPVOID lpBaseAddress = VirtualAlloc(NULL, 1024, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	//
	struct SHELLCODE
	{
		ULONG HookAddr;
		ULONG CallBackFun;
		ULONG Reserved1;
		ULONG Reserved2;
		BYTE  ShellCode[1008];
	};
	SHELLCODE* pShellCodeBlock = (SHELLCODE*) lpBaseAddress;//
	pShellCodeBlock->CallBackFun = (ULONG)&PacketCollect;
	DWORD dwCodeSize = Utility::StringLib::Tstring2Hex(strShellCode.c_str(), pShellCodeBlock->ShellCode);
	pShellCodeBlock->HookAddr = Utility::IniAccess::GetPrivateKeyValInt(strCfgFile, _T("CODE"), _T("Target"));

	for (int i = 0; i < dwCodeSize; i++)
	{
		if (pShellCodeBlock->ShellCode[i] == 0xff)
		{
			if (pShellCodeBlock->ShellCode[i + 1] == 0x15) {
				*(ULONG*)&pShellCodeBlock->ShellCode[i + 2] = (ULONG)&pShellCodeBlock->CallBackFun;
			}
				
			if (pShellCodeBlock->ShellCode[i + 1] == 0x25) {
				*(ULONG*)&pShellCodeBlock->ShellCode[i + 2] = (ULONG)&pShellCodeBlock->HookAddr;
			}
		}
	}


	DetourAttach(&(PVOID&)pShellCodeBlock->HookAddr, (PVOID)pShellCodeBlock->ShellCode);
	//
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
	//__asm
	//{
	//	pushad
	//	pushfd
	//	push    dword ptr ds : [esp + 2ch]        //SIZE( dwBufferCount )
	//	push    dword ptr ds : [esp + 2ch]		//DATA( LPWSABUF )
	//	push    ecx
	//	call    detourGameEncrype
	//	popfd
	//	popad
	//	jmp     m_ulPatchAddr
	//}
}

VOID WINAPI CDOMPlugin::PacketCollect(SOCKET s , LPBYTE lpBuffer, DWORD dwSize, LPVOID Reserved1, LPVOID Reserved2, LPVOID Reserved3, LPVOID Reserved4 )
{
	CProperty pro;
	pro.s = s;
	pro.Param1 = Reserved1;
	pro.Param2 = Reserved2;
	pro.Param3 = Reserved3;
	pro.Param4 = Reserved4;

	CGPacket* pNewPacket = new CGPacket((LPBYTE)lpBuffer, dwSize, pro);
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
