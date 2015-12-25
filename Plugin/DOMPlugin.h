#pragma once
#include "Plugin.h"
class CDOMPlugin :
	public CPlugin
{
public:
	CDOMPlugin();
	~CDOMPlugin();
	void SendData(CGPacket& packetBuf);
	BOOL InstallPlugin(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc);
	BOOL UnInstallPlugin();
private:
	static ULONG m_ulPatchAddr;
protected:
	BOOL PatchUserDefineAddr();
	BOOL UnPatch();
private:
	static VOID WINAPI detourGameEncrype(LPVOID lpParam, DWORD dwSize, LPBYTE lpBuffer);
	static VOID detourGameEncrypeThunk();
};

/*
CPU Disasm
Address   Hex dump          Command                                  Comments this->[ECX]--->s, Param1->size, param2->buffer
006EE060  /$  55            PUSH EBP                                 ; ¼ÓÃÜº¯Êý
006EE061  |.  8BE9          MOV EBP,ECX
006EE063  |.  57            PUSH EDI
006EE064  |.  66:8B45 40    MOV AX,WORD PTR SS:[EBP+40]
006EE068  |.  66:3D FFFF    CMP AX,0FFFF
006EE06C  |.  75 07         JNE SHORT 006EE075
006EE06E  |.  5F            POP EDI
006EE06F  |.  32C0          XOR AL,AL
006EE071  |.  5D            POP EBP
006EE072  |.  C2 0800       RETN 8
006EE075  |>  8B7C24 10     MOV EDI,DWORD PTR SS:[ARG.2]
006EE079  |.  66:813F FFFF  CMP WORD PTR DS:[EDI],0FFFF
006EE07E  |.  75 07         JNE SHORT 006EE087
006EE080  |.  5F            POP EDI
006EE081  |.  B0 01         MOV AL,1
006EE083  |.  5D            POP EBP
006EE084  |.  C2 0800       RETN 8
006EE087  |>  66:85C0       TEST AX,AX
006EE08A  |.  75 07         JNZ SHORT 006EE093
006EE08C  |.  5F            POP EDI
006EE08D  |.  B0 01         MOV AL,1
006EE08F  |.  5D            POP EBP
006EE090  |.  C2 0800       RETN 8
006EE093  |>  56            PUSH ESI
006EE094  |.  8B7424 10     MOV ESI,DWORD PTR SS:[ARG.1]
006EE098  |.  8A45 40       MOV AL,BYTE PTR SS:[EBP+40]
006EE09B  |.  81E6 FFFF0000 AND ESI,0000FFFF
006EE0A1  |.  8D56 FE       LEA EDX,[ESI-2]
006EE0A4  |.  85D2          TEST EDX,EDX
006EE0A6  |.  7C 08         JL SHORT 006EE0B0
006EE0A8  |>  8A0C3A        /MOV CL,BYTE PTR DS:[EDI+EDX]
006EE0AB  |.  32C1          |XOR AL,CL
006EE0AD  |.  4A            |DEC EDX
006EE0AE  |.^ 79 F8         \JNS SHORT 006EE0A8
006EE0B0  |>  88443E FF     MOV BYTE PTR DS:[EDI+ESI-1],AL
006EE0B4  |.  33D2          XOR EDX,EDX
006EE0B6  |.  66:8B55 40    MOV DX,WORD PTR SS:[EBP+40]
006EE0BA  |.  33C0          XOR EAX,EAX
006EE0BC  |.  85F6          TEST ESI,ESI
006EE0BE  |.  7E 1B         JLE SHORT 006EE0DB
006EE0C0  |.  53            PUSH EBX
006EE0C1  |>  8A1C38        /MOV BL,BYTE PTR DS:[EDI+EAX]
006EE0C4  |.  8BCA          |MOV ECX,EDX
006EE0C6  |.  83E1 0F       |AND ECX,0000000F
006EE0C9  |.  8A89 D8069000 |MOV CL,BYTE PTR DS:[ECX+9006D8]
006EE0CF  |.  32D9          |XOR BL,CL
006EE0D1  |.  881C38        |MOV BYTE PTR DS:[EDI+EAX],BL
006EE0D4  |.  40            |INC EAX
006EE0D5  |.  42            |INC EDX
006EE0D6  |.  3BC6          |CMP EAX,ESI
006EE0D8  |.^ 7C E7         \JL SHORT 006EE0C1
006EE0DA  |.  5B            POP EBX
006EE0DB  |>  33C0          XOR EAX,EAX
006EE0DD  |.  66:8B45 40    MOV AX,WORD PTR SS:[EBP+40]
006EE0E1  |.  99            CDQ
006EE0E2  |.  F7FE          IDIV ESI
006EE0E4  |.  8A0C3A        MOV CL,BYTE PTR DS:[EDI+EDX]
006EE0E7  |.  8A443E FF     MOV AL,BYTE PTR DS:[EDI+ESI-1]
006EE0EB  |.  884C3E FF     MOV BYTE PTR DS:[EDI+ESI-1],CL
006EE0EF  |.  5E            POP ESI
006EE0F0  |.  88043A        MOV BYTE PTR DS:[EDI+EDX],AL
006EE0F3  |.  B0 01         MOV AL,1
006EE0F5  |.  03D7          ADD EDX,EDI
006EE0F7  |.  5F            POP EDI
006EE0F8  |.  5D            POP EBP
006EE0F9  \.  C2 0800       RETN 8

*/