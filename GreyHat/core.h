#pragma once

#include<WinSock2.h>
#include "Packet.h"
enum E_DATA_IN_OUT_TYPE
{
	E_TYPE_DATA_OUT,
	E_TYPE_DATA_IN
};

typedef VOID(WINAPI*SENDPROCHANDLER)(CPacket&);
typedef VOID(WINAPI*RECVPROCHANDLER)(CPacket&);


typedef BOOL (APIENTRY*DETACHPROCESS) ();
typedef BOOL (APIENTRY*ATTACHPROCESS) (SENDPROCHANDLER, RECVPROCHANDLER);
typedef VOID (APIENTRY*DISPATCHGAMEDATA) (CPacket&);
//typedef VOID (APIENTRY*SETGLOBALENV) (LPVOID);

class CCoreLib
{
public:
    CCoreLib( void );
    ~CCoreLib( void );
	static void WINAPI SendData(CPacket&);
	static BOOL InstallPlugin(SENDPROCHANDLER out_hdr = NULL, RECVPROCHANDLER in_hdr = NULL);
	static BOOL UnInstallPlugin(VOID);

};

