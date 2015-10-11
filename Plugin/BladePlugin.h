#pragma once
#include "Plugin.h"
class CBladePlugin :
	public CPlugin
{
public:
	CBladePlugin();
	~CBladePlugin();

	void SendData(CPacket& packetBuf);
	BOOL InstallPlugin(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc);
	BOOL UnInstallPlugin();
private:
	static ULONG m_ulPatchAddr;
protected:
	BOOL PatchUserDefineAddr();
	BOOL UnPatch();
private:
	static VOID WINAPI Encrypt(LPVOID lpParam, DWORD dwSize, LPBYTE lpBuffer);
	static VOID EncryptThunk();
};

/*
param:
this--> [esi+0x40]
input-->
inlen
output
outlen
unknown
00BA8600    51              PUSH ECX
00BA8601    8B5424 08       MOV EDX,DWORD PTR SS:[ESP+8]
00BA8605    85D2            TEST EDX,EDX
00BA8607    53              PUSH EBX
00BA8608    55              PUSH EBP
00BA8609    8BE9            MOV EBP,ECX
00BA860B    C74424 08 00000>MOV DWORD PTR SS:[ESP+8],0
00BA8613    74 7F           JE SHORT Client.00BA8694
00BA8615    8B5C24 14       MOV EBX,DWORD PTR SS:[ESP+14]
00BA8619    85DB            TEST EBX,EBX
00BA861B    7E 77           JLE SHORT Client.00BA8694
00BA861D    8B4424 18       MOV EAX,DWORD PTR SS:[ESP+18]
00BA8621    85C0            TEST EAX,EAX
00BA8623    74 6F           JE SHORT Client.00BA8694
00BA8625    56              PUSH ESI
00BA8626    8B75 20         MOV ESI,DWORD PTR SS:[EBP+20]
00BA8629    85F6            TEST ESI,ESI
00BA862B    7C 27           JL SHORT Client.00BA8654
00BA862D    8A4C24 24       MOV CL,BYTE PTR SS:[ESP+24]
00BA8631    57              PUSH EDI
00BA8632    50              PUSH EAX
00BA8633    52              PUSH EDX
00BA8634    8BC6            MOV EAX,ESI
00BA8636    E8 054D0000     CALL Client.00BAD340
00BA863B    8B7C24 2C       MOV EDI,DWORD PTR SS:[ESP+2C]
00BA863F    8B74B5 24       MOV ESI,DWORD PTR SS:[EBP+ESI*4+24]
00BA8643    50              PUSH EAX
00BA8644    8D4424 1C       LEA EAX,DWORD PTR SS:[ESP+1C]
00BA8648    50              PUSH EAX
00BA8649    E8 12520000     CALL Client.00BAD860
00BA864E    83C4 10         ADD ESP,10
00BA8651    5F              POP EDI
00BA8652    EB 1F           JMP SHORT Client.00BA8673
00BA8654    8B4C24 20       MOV ECX,DWORD PTR SS:[ESP+20]
00BA8658    51              PUSH ECX
00BA8659    50              PUSH EAX
00BA865A    53              PUSH EBX
00BA865B    8D45 10         LEA EAX,DWORD PTR SS:[EBP+10]
00BA865E    50              PUSH EAX
00BA865F    8D4D 0C         LEA ECX,DWORD PTR SS:[EBP+C]
00BA8662    51              PUSH ECX
00BA8663    8D45 08         LEA EAX,DWORD PTR SS:[EBP+8]
00BA8666    50              PUSH EAX
00BA8667    8D4C24 24       LEA ECX,DWORD PTR SS:[ESP+24]
00BA866B    E8 A04F0000     CALL Client.00BAD610
00BA8670    83C4 18         ADD ESP,18
00BA8673    85C0            TEST EAX,EAX
00BA8675    5E              POP ESI
00BA8676    7F 0F           JG SHORT Client.00BA8687
00BA8678    8B4C24 08       MOV ECX,DWORD PTR SS:[ESP+8]
00BA867C    894D 04         MOV DWORD PTR SS:[EBP+4],ECX
00BA867F    5D              POP EBP
00BA8680    33C0            XOR EAX,EAX
00BA8682    5B              POP EBX
00BA8683    59              POP ECX
00BA8684    C2 1400         RETN 14
00BA8687    C745 04 0000000>MOV DWORD PTR SS:[EBP+4],0
00BA868E    5D              POP EBP
00BA868F    5B              POP EBX
00BA8690    59              POP ECX
00BA8691    C2 1400         RETN 14
00BA8694    C745 04 0100000>MOV DWORD PTR SS:[EBP+4],1
00BA869B    5D              POP EBP
00BA869C    33C0            XOR EAX,EAX
00BA869E    5B              POP EBX
00BA869F    59              POP ECX
00BA86A0    C2 1400         RETN 14


*/