#include "stdafx.h"
#include "core.h"
#include <Shlwapi.h>
#include "greyhat.h"

CCoreLib::CCoreLib(void)
{
}

CCoreLib::~CCoreLib( void )
{
}
/*
start hook the CPacket edit code.
*/
BOOL CCoreLib::InstallPlugin(SENDPROCHANDLER pfnOutdataHdr, RECVPROCHANDLER pfnIndataHdr)
{
	TCHAR tszCoreModuleLoadName[MAX_PATH];
	DWORD dwErr;
	
	_tcscpy(tszCoreModuleLoadName, GlobalEnv.tszCoreDllPath);
	//_tcscat(tszCoreModuleLoadName, _T("."));

	GlobalEnv.hCoreInst = GetModuleHandle(tszCoreModuleLoadName);

	if ( NULL == GlobalEnv.hCoreInst )	{
		GlobalEnv.hCoreInst = LoadLibrary(tszCoreModuleLoadName);
		if (GlobalEnv.hCoreInst == NULL)
			dwErr = GetLastError();
	}

	if (GlobalEnv.hCoreInst) {
		GlobalEnv.pfnAttachProcess		= (ATTACHPROCESS)GetProcAddress(GlobalEnv.hCoreInst, "Install");
		GlobalEnv.pfnDetachProcess		= (DETACHPROCESS)GetProcAddress(GlobalEnv.hCoreInst, "UnInstall");
		GlobalEnv.pfnDispatchPacket	= (DISPATCHGAMEDATA)GetProcAddress(GlobalEnv.hCoreInst, "Send");

		if (GlobalEnv.pfnAttachProcess)	{

			return GlobalEnv.pfnAttachProcess(pfnOutdataHdr, pfnIndataHdr);
		}else
			return FALSE;

	}
	return FALSE;
}
/*
unhook, detach from the code
*/
BOOL CCoreLib::UnInstallPlugin(void)
{
	if (GlobalEnv.hCoreInst) {

		if (GlobalEnv.pfnDetachProcess)	{

			GlobalEnv.pfnDetachProcess();
			return  FreeLibrary(GlobalEnv.hCoreInst);
		}else
			return FALSE;

	}
	return FALSE;
}


/*
Call send interface to send the CPacket.
*/
void CCoreLib::SendData(CPacket& packetBuf)
{
	if (GlobalEnv.pfnDispatchPacket) {

		GlobalEnv.pfnDispatchPacket(packetBuf);
	}
}