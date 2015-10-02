#pragma once
//#ifndef USER_DEFINE_H
//#define USER_DEFINE_H

#include<WinSock2.h>
typedef int  ( WINAPI*PFN_API_SEND )( SOCKET,const char*,int,int );
typedef int  ( WINAPI*PFN_API_RECV )( SOCKET,char*,int,int );
typedef int  ( WINAPI*PFN_API_WSASEND )( SOCKET,LPWSABUF,DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE );

typedef void ( WINAPI*PFN_DATA_HANDLER )( SOCKET, BYTE*, DWORD, LPARAM, LPARAM, LPARAM, LPARAM );
typedef int  ( WINAPI*PFN_API_CONNECT)( SOCKET s, const struct sockaddr *name, int namelen );

typedef HANDLE ( WINAPI * PFN_CREATETHREAD)(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize,	LPTHREAD_START_ROUTINE lpStartAddress,	LPVOID lpParameter,	DWORD dwCreationFlags,	LPDWORD lpThreadId	);



#define DATA_SIZE 0x00000064L

#include <vector>
using namespace std;
/*
//data struct define
struct ST_PACKET_OP
{
    ST_PACKET_OP(  )
    {
        wPacketSize  = 0;
        wReplacePtnSize = 0;
        wFilterPtnSize = 0;
        bFilterIpAddr  = FALSE;
        RtlZeroMemory( wszFilterIpPort, sizeof( wszFilterIpPort ) );
        RtlZeroMemory( btFilterPtn, sizeof( btFilterPtn ) );
        RtlZeroMemory( btReplacePtn, sizeof( btReplacePtn ) );
    }
    WORD   wPacketSize;
    WORD   wFilterPtnSize;
    BYTE   btFilterPtn[DATA_SIZE];
    WORD   wReplacePtnSize;
    BYTE   btReplacePtn[DATA_SIZE];
    BOOL   bFilterIpAddr;
    WCHAR  wszFilterIpPort[DATA_SIZE];
};
*/
struct ST_PACKET_PARAM
{
    ST_PACKET_PARAM(  )
    {
        s     = INVALID_SOCKET;
        lParam0 = NULL;
        lParam1 = NULL;
        lParam2 = NULL;
        lParam3 = NULL;
    };
    SOCKET s;
    LPARAM lParam0;
    LPARAM lParam1;
    LPARAM lParam2;
    LPARAM lParam3;
};
/*
struct DATA_FRAME
{
    DATA_FRAME(  )
    {
        dwDataSize      = 0;
        pbyteRawData    = NULL;
        pbyteSavedData  = NULL;
    }
    DWORD              dwDataSize;
    BYTE               *pbyteRawData;
    BYTE               *pbyteSavedData;
    ST_PACKET_PARAM	   ExtraData;
};
*/


#define STORE_REGISTERS    __asm pushad  __asm pushfd
#define RESTORE_REGISTERS     __asm popfd   __asm popad

//#endif