#pragma once
#include "PluginBase.h"

#include "detours.h"

#include <tchar.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"detours.lib")

class CPlugin :
	public CPluginBase
{
public:
	CPlugin();
	~CPlugin();
	void SendData(CGPacket& packetBuf);
	BOOL InstallPlugin(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc);
	BOOL UnInstallPlugin();
	//VOID SetPacketHandler(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc);
private:
	static VOID Send12Thunk();
	static VOID WSASend12Thunk();
	static VOID SendTo12Thunk();
	static VOID Thunk();

	static VOID APIENTRY SendStub(SOCKET s, const char* buf, int nlen);
	static VOID APIENTRY WSASendStub(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount);
	//static VOID APIENTRY SendToThunkHandler(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen);
	static int WSAAPI RecvStub(SOCKET s, char* buf, int bufsize, int flag);

private:
	static ULONG m_offset12FromSend;
	static ULONG m_offset12FromWSASend;
	static ULONG m_offset12FromSend2;
	static ULONG m_offsetRecv;
	static ULONG m_offset12FromWSARecv;
	static ULONG m_offset12FromRecvFrom;
	static ULONG m_offset12FromWSAReceFrom;

	//CPluginBase*	 m_pGameBox;
public:
	BOOL PatchWinsockApis();
	BOOL UnPatchWinsockApis();
};

