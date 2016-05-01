// HideSelf.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <stdlib.h>

#include "detours.h"
#include "Mem.h"
#include <Shlwapi.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include"nativedef.h"
#pragma comment(lib, "detours.lib")
#pragma comment(lib, "Shlwapi.lib")

#define ALPHABET_LEN 256

void MakeDelta1(INT *Delta1, PBYTE pSign, DWORD dwSignLen) {
	INT iI;
	for (iI = 0; iI<ALPHABET_LEN; iI++) {
		Delta1[iI] = dwSignLen;
	}
	for (iI = 0; iI < dwSignLen - 1; iI++) {
		Delta1[pSign[iI]] = dwSignLen - 1 - iI;
	}
}

BOOL IsPrefix(BYTE *Word, DWORD dwWordLen, DWORD dwPos) {
	INT iI;
	DWORD dwSuffixlen = dwWordLen - dwPos;
	for (iI = 0; iI < dwSuffixlen; iI++) {
		if (Word[iI] != Word[dwPos + iI]) {
			return FALSE;
		}
	}
	return TRUE;
}

DWORD SuffixLength(BYTE *Word, DWORD dwWordLen, DWORD dwPos) {
	INT iI;
	for (iI = 0; (Word[dwPos - iI] == Word[dwWordLen - 1 - iI]) && (iI<dwPos); iI++);

	return iI;
}

VOID MakeDelta2(INT *Delta2, BYTE *pSign, DWORD dwSignLen) {
	INT iI, iSuffixLen;
	INT LastPrefixIndex = dwSignLen - 1;

	for (iI = dwSignLen - 1; iI >= 0; iI--) {
		if (IsPrefix(pSign, dwSignLen, iI + 1)) {
			LastPrefixIndex = iI + 1;
		}
		Delta2[iI] = LastPrefixIndex + (dwSignLen - 1 - iI);
	}

	for (iI = 0; iI<dwSignLen - 1; iI++) {
		iSuffixLen = SuffixLength(pSign, dwSignLen, iI);
		if (pSign[iI - iSuffixLen] != pSign[dwSignLen - 1 - iSuffixLen]) {
			Delta2[dwSignLen - 1 - iSuffixLen] = dwSignLen - 1 - iI + iSuffixLen;
		}
	}
}

LPVOID BooyerSearch(LPBYTE lpSrc, DWORD dwStringLen, LPBYTE lpSign, DWORD dwSignLen) {

	INT iI;
	INT Delta1[ALPHABET_LEN];
	INT *Delta2 = (INT*)malloc(dwSignLen * sizeof(INT));

	MakeDelta1(Delta1, lpSign, dwSignLen);
	MakeDelta2(Delta2, lpSign, dwSignLen);

	iI = dwSignLen - 1;

	while (iI < dwStringLen) {
		INT dwJ = dwSignLen - 1;
		while (dwJ >= 0 && (lpSrc[iI] == lpSign[dwJ])) {
			--iI;
			--dwJ;
		}
		if (dwJ < 0) {
			free(Delta2);
			return (lpSrc + iI + 1);
		}
		else
			iI += max(Delta1[lpSrc[iI]], Delta2[dwJ]);
	}
	free(Delta2);
	return NULL;
}


//void preQsBc(char *x, int m, int qsBc[]) {
//	int i;
//
//	for (i = 0; i < ASIZE; ++i)
//		qsBc[i] = m + 1;
//	for (i = 0; i < m; ++i)
//		qsBc[x[i]] = m - i;
//}

#define ASIZE 256
char* QS(char *x, int m, char *y, int n) {
	int i, j, qsBc[ASIZE];

	/* Preprocessing */

	for (i = 0; i < ASIZE; ++i)
		qsBc[i] = m + 1;
	for (i = 0; i < m; ++i)
		qsBc[x[i]] = m - i;
	//preQsBc(x, m, qsBc);

	/* Searching */
	j = 0;
	while (j <= n - m) {
		if (memcmp(x, y + j, m) == 0)
			//LOG(TEXT("match pos:%d"), j);
			return y + j;
		j += qsBc[y[j + m]];               /* shift */
	}
	return NULL;
}

LPVOID SearchReadWriteMemory(LPVOID lpBaseAddress, LPVOID lpPtn, DWORD dwPtnSize)
{
	LPVOID lpMatchAddr = NULL;
	MEMORY_BASIC_INFORMATION MemoryInformation;
	LPVOID BaseAddress = lpBaseAddress;

	while (0 != VirtualQuery((LPVOID)BaseAddress, &MemoryInformation, sizeof(MEMORY_BASIC_INFORMATION))) 
	{
		if (MemoryInformation.State == MEM_COMMIT && MemoryInformation.Protect == PAGE_READWRITE)
		{
			lpMatchAddr = BooyerSearch((LPBYTE)BaseAddress, MemoryInformation.RegionSize - ((ULONG)BaseAddress - (ULONG)MemoryInformation.BaseAddress + 1), (LPBYTE)lpPtn, dwPtnSize);
			if (lpMatchAddr) {
				return lpMatchAddr;
			}
		}
		BaseAddress = (LPVOID)((ULONG)MemoryInformation.BaseAddress + MemoryInformation.RegionSize);
	}
	return NULL;
}

LPVOID SearchExecutableMemory(LPVOID lpAddress, LPVOID lpPattern, ULONG ulPatternSize)
{
	LPVOID lpMatchAddr = NULL;
	MEMORY_BASIC_INFORMATION MemoryInformation;
	LPVOID BaseAddress = lpAddress;

	while (TRUE) {
		if (0 != VirtualQuery((LPVOID)BaseAddress, &MemoryInformation, sizeof(MEMORY_BASIC_INFORMATION))) 
		{
			if (MemoryInformation.State == MEM_COMMIT && MemoryInformation.Protect & 0xf0) 
			{
				lpMatchAddr = BooyerSearch((LPBYTE)BaseAddress, MemoryInformation.RegionSize - ((ULONG)BaseAddress - (ULONG)MemoryInformation.BaseAddress + 1), (LPBYTE)lpPattern, ulPatternSize);
				//lpMatchAddr = QS((char*)BaseAddress, MemoryInformation.RegionSize - ((ULONG)BaseAddress - (ULONG)MemoryInformation.BaseAddress), (char*)lpPattern, ulPatternSize);
				if (lpMatchAddr) {
					return lpMatchAddr;
				}
			}
			BaseAddress =(LPVOID) ((ULONG)MemoryInformation.BaseAddress + MemoryInformation.RegionSize);
		}else{
			DWORD dwErr = GetLastError();
			if (dwErr == ERROR_INVALID_PARAMETER)
			{
				return NULL;
			}
		}
	}
	return NULL;
}




#pragma warning(disable:4996)


void HideModuleInit();
void HideModule(HMODULE hMod);

typedef struct _IMAGE_PAGE_ENTRY
{
	PVOID BaseAddress;
	ULONG SizeOfPage;
}IMAGE_PAGE_ENTRY, *PIMAGE_PAGE_ENTRY;

typedef struct _MODULE_HIDE_ENTRY
{
	LIST_ENTRY HideLinks;
	PVOID ImageBase;
	ULONG SizeOfImage;
	ULONG PageCountOfImage;
	IMAGE_PAGE_ENTRY PageEntry[10];
}MODULE_HIDE_ENTRY, *PMODULE_HIDE_ENTRY;


LIST_ENTRY HideLinkHeader;
NTQUERYVIRTUALMEMORY pfnNtQueryVirtualMemory, pfncopyNtQueryVirtualMemory;
RTLINITUNICODESTRINGEX pfnRtlInitUnicodeStringEx;


void LOG(LPCTSTR lpszFormat, ...)
{
	TCHAR     szBuffer[0x4000];
	TCHAR     szNewFormat[0x1000];

	_stprintf(szNewFormat, TEXT("#%d ~%d %s"), GetCurrentProcessId(), GetCurrentThreadId(), lpszFormat);

	va_list   args;
	va_start(args, lpszFormat);
	wvsprintf(szBuffer, szNewFormat, args);
	OutputDebugString(szBuffer);
	//FILE *fp;
	//_tfopen_s(&fp, TEXT("LOG.log"), TEXT("a"));
	//if (fp)
	//{
	//	//TCHAR szDate[260], szTime[260];
	//	//_tstrdate_s(szDate, sizeof(szDate)); _tstrtime_s(szTime, sizeof(szTime));
	//	_ftprintf(fp, TEXT("%s\n"), szBuffer);
	//	fclose(fp);
	//}
	va_end(args);
}

DWORD GetSizeOfImage(HMODULE hModuleBase)
{
	DWORD dwSizeOfImage = 0;
	PIMAGE_DOS_HEADER ImageDosHeaders;
	PIMAGE_NT_HEADERS ImageNtHeaders;

	//	验证模块是否是PE文件
	if (hModuleBase == NULL)
		return dwSizeOfImage;

	ImageDosHeaders = (PIMAGE_DOS_HEADER)hModuleBase;

	if (ImageDosHeaders->e_magic == IMAGE_DOS_SIGNATURE) {
		ImageNtHeaders = (PIMAGE_NT_HEADERS)((DWORD)hModuleBase + ImageDosHeaders->e_lfanew);
		if (ImageNtHeaders->Signature == IMAGE_NT_SIGNATURE) {
			dwSizeOfImage = ImageNtHeaders->OptionalHeader.SizeOfImage;
		}
	}
	return dwSizeOfImage;
}

VOID WINAPI AddModule2HideList(HMODULE hModule)
{
	PMODULE_HIDE_ENTRY pEntry = new MODULE_HIDE_ENTRY;
	pEntry->ImageBase = hModule;
	pEntry->SizeOfImage = GetSizeOfImage(hModule);
	pEntry->PageCountOfImage = 0;
	memset(pEntry->PageEntry, 0, sizeof(pEntry->PageEntry));

	TCHAR szFilename[MAX_PATH];
	GetModuleFileName(hModule, szFilename, MAX_PATH);
	LPTSTR lpstrFilename = PathFindFileName(szFilename);

	NTQUERYVIRTUALMEMORY _pfnNtQueryVirtualMemory = (NTQUERYVIRTUALMEMORY)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "NtQueryVirtualMemory");


	PVOID BaseAddress = NULL;
	MEMORY_BASIC_INFORMATION MemoryInformation;
	while (0 != VirtualQuery((LPVOID)BaseAddress, &MemoryInformation, sizeof(MEMORY_BASIC_INFORMATION)))
	{
		if (MemoryInformation.Type == MEM_IMAGE)
		{
			ULONG ReturnLength;
			CHAR Filename[1024];
			_pfnNtQueryVirtualMemory(GetCurrentProcess(), BaseAddress, MemoryMappedFilenameInformation, Filename, sizeof(Filename), &ReturnLength);

			PUNICODE_STRING usFilename = (PUNICODE_STRING)Filename;

			if (StrStrI(usFilename->Buffer, lpstrFilename))
			{
				pEntry->PageEntry[pEntry->PageCountOfImage].BaseAddress = MemoryInformation.BaseAddress;
				pEntry->PageEntry[pEntry->PageCountOfImage].SizeOfPage = MemoryInformation.RegionSize;
				pEntry->PageCountOfImage++;
			}
		}
		BaseAddress = (LPVOID)((ULONG)MemoryInformation.BaseAddress + MemoryInformation.RegionSize);

	}

	InsertTailList(&HideLinkHeader, (PLIST_ENTRY)pEntry);
}


LPVOID __declspec(naked) CurrentPEB()
{
	__asm
	{
		mov eax, fs:[0x18]
		mov eax, [eax + 0x30]
		ret
	}
}
VOID RemoveHideListModulesFromLdrTable()
{
	LPVOID Peb = CurrentPEB();
	PPEB_LDR_DATA Ldr = *(PPEB_LDR_DATA*)((ULONG)Peb + 0x0c);
	LOG(TEXT("peb[%08lx]---ldr[%08lx]"), Peb, Ldr);
	PLDR_DATA_TABLE_ENTRY  LdrDataTableHead = (PLDR_DATA_TABLE_ENTRY)(&(Ldr->InLoadOrderModuleList));
	PLDR_DATA_TABLE_ENTRY LdrDataTable = (PLDR_DATA_TABLE_ENTRY)LdrDataTableHead->InLoadOrderLinks.Flink;
	while (LdrDataTable != LdrDataTableHead)
	{
		LOG(TEXT("ldrtable--%08lx----%08lx-->%s"), LdrDataTable->DllBase, LdrDataTable->SizeOfImage, LdrDataTable->FullDllName.Buffer);

		PLIST_ENTRY HideEntryNext = HideLinkHeader.Flink;
		while (HideEntryNext != &HideLinkHeader)
		{
			//判断是否是我们要屏蔽的模块
			PMODULE_HIDE_ENTRY Entry = (PMODULE_HIDE_ENTRY)HideEntryNext;
			if (LdrDataTable->DllBase == Entry->ImageBase)
			{
				LOG(TEXT("ldrtable--X--links"));
				LdrDataTable->InLoadOrderLinks.Flink->Blink = LdrDataTable->InLoadOrderLinks.Blink;
				LdrDataTable->InLoadOrderLinks.Blink->Flink = LdrDataTable->InLoadOrderLinks.Flink;

				LdrDataTable->InMemoryOrderLinks.Flink->Blink = LdrDataTable->InMemoryOrderLinks.Blink;
				LdrDataTable->InMemoryOrderLinks.Blink->Flink = LdrDataTable->InMemoryOrderLinks.Flink;

				LdrDataTable->InInitializationOrderLinks.Flink->Blink = LdrDataTable->InInitializationOrderLinks.Blink;
				LdrDataTable->InInitializationOrderLinks.Blink->Flink = LdrDataTable->InInitializationOrderLinks.Flink;
			}

			HideEntryNext = HideEntryNext->Flink;
		}


		//移动到链表的下一个
		LdrDataTable = (PLDR_DATA_TABLE_ENTRY)LdrDataTable->InLoadOrderLinks.Flink;

	}
}


NTSTATUS WINAPI MyNtQueryVirtualMemory(
	HANDLE ProcessHandle,
	PVOID BaseAddress,
	MEMORY_INFORMATION_CLASS MemoryInformationClass,
	PVOID MemoryInformation,
	ULONG MemoryInformationLength,
	PULONG ReturnLength OPTIONAL
)
{
	NTSTATUS status;
	status = pfnNtQueryVirtualMemory(ProcessHandle,
		BaseAddress,
		MemoryInformationClass,
		MemoryInformation,
		MemoryInformationLength,
		ReturnLength);
	switch (MemoryInformationClass)
	{
	case MemoryBasicInformation:
	{
		//PMEMORY_BASIC_INFORMATION pMemInfo = (PMEMORY_BASIC_INFORMATION)MemoryInformation;
		//LOG(TEXT("ntquery--%08lx--%08lx--%08lx--%08lx--%08lx--%08lx"), BaseAddress, pMemInfo->AllocationBase, pMemInfo->Protect, pMemInfo->RegionSize, pMemInfo->State, pMemInfo->Type);
		PLIST_ENTRY HideEntryNext = HideLinkHeader.Flink;
		while (HideEntryNext != &HideLinkHeader)
		{
			PMODULE_HIDE_ENTRY Entry = (PMODULE_HIDE_ENTRY)HideEntryNext;

			for (int i = 0; i < Entry->PageCountOfImage; i++)
			{
				if (Entry->PageEntry[i].BaseAddress == BaseAddress)
				{
					PMEMORY_BASIC_INFORMATION pMemInfo = (PMEMORY_BASIC_INFORMATION)MemoryInformation;
					pMemInfo->State = MEM_FREE;
					pMemInfo->Type = 0;
					pMemInfo->Protect = PAGE_NOACCESS;
					pMemInfo->AllocationProtect = PAGE_NOACCESS;
					LOG(TEXT("--%08lx--%08lx--%08lx--%08lx--%08lx--%08lx"), BaseAddress, pMemInfo->AllocationBase, pMemInfo->Protect, pMemInfo->RegionSize, pMemInfo->State, pMemInfo->Type);
				}
			}
			HideEntryNext = HideEntryNext->Flink;
		}
	}
	break;

	//case MemoryMappedFilenameInformation:
	//{
	//	PUNICODE_STRING pFileName = (PUNICODE_STRING)MemoryInformation;

	//	LOG(TEXT("ntquery--%s"), pFileName->Buffer);
	//}
	//break;
	}


	return status;
}



void HideModuleInit()
{

	InitializeListHead(&HideLinkHeader);

	pfnNtQueryVirtualMemory = (NTQUERYVIRTUALMEMORY)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "NtQueryVirtualMemory");
	//pfnRtlInitUnicodeStringEx = (RTLINITUNICODESTRINGEX)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "RtlInitUnicodeStringEx");


	unsigned char copyNtQueryVirtualMemory[] = {
		/*
		1290FBD8    B8 20000000          MOV EAX,20
		1290FBDD    33C9                 XOR ECX,ECX
		1290FBDF    8D5424 04            LEA EDX,DWORD PTR SS:[ESP+4]
		1290FBE3    64:FF15 C0000000     CALL DWORD PTR FS:[C0]
		1290FBEA    83C4 04              ADD ESP,4
		1290FBED    C2 1800              RETN 18
		*/
		0xB8, 0x20, 0x00, 0x00, 0x00, 0x33, 0xC9, 0x8D, 0x54, 0x24, 0x04, 0x64, 0xFF, 0x15, 0xC0, 0x00, 0x00, 0x00, 0x83, 0xC4, 0x04, 0xC2, 0x18, 0x00
	};
	pfncopyNtQueryVirtualMemory = (NTQUERYVIRTUALMEMORY)SearchExecutableMemory(NULL, copyNtQueryVirtualMemory, sizeof(copyNtQueryVirtualMemory));

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pfnNtQueryVirtualMemory, &MyNtQueryVirtualMemory);

	if (FALSE == IsBadReadPtr(pfncopyNtQueryVirtualMemory, 32) &&
		pfnNtQueryVirtualMemory != pfncopyNtQueryVirtualMemory)
	{
		DetourAttach(&(PVOID&)pfncopyNtQueryVirtualMemory, &MyNtQueryVirtualMemory);
	}

	DetourTransactionCommit();

	//__asm int 3;
	DWORD dwCRC = 0x00061542;
	LPVOID lpSearchBase = NULL;
retry:
	DWORD *pdwCRCVirtualAddress = (DWORD*)SearchReadWriteMemory(lpSearchBase, (LPVOID)&dwCRC, sizeof(dwCRC));
	if (pdwCRCVirtualAddress == &dwCRC)
	{
		lpSearchBase = (LPVOID)((ULONG)pdwCRCVirtualAddress + sizeof(dwCRC));
		goto retry;
	}

	LOG(TEXT("real--[%08lx], copy--[%08lx]--rtl--[%08lx]-crc--[%08lx]"), pfnNtQueryVirtualMemory, pfncopyNtQueryVirtualMemory, pfnRtlInitUnicodeStringEx, pdwCRCVirtualAddress);

	struct CODE_CHECKER {
		LPVOID CodeBase;
		ULONG  CodeSize;
		ULONG  Reserved1;
		ULONG  Reserved2;
		ULONG  CRC;
	};
	//__asm int 3;
	if (pdwCRCVirtualAddress)
	{
		CODE_CHECKER *p = (CODE_CHECKER*)((ULONG)pdwCRCVirtualAddress - 16);
		dwCRC = 0;
		for (int i = 0; i < p->CodeSize / 4; i++)
		{
			dwCRC += ((DWORD*)p->CodeBase)[i];
		}
		dwCRC = ~dwCRC;
		LOG(TEXT("---new CRC--[%08lx]"), dwCRC);
		if (pdwCRCVirtualAddress)
			*pdwCRCVirtualAddress = dwCRC;
	}
}

void HideModule(HMODULE hMod)
{
	AddModule2HideList(hMod);
	RemoveHideListModulesFromLdrTable();
}