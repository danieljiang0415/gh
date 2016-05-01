// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "detours.h"
#include "Mem.h"
#include <Shlwapi.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include"nativedef.h"
#pragma comment(lib, "detours.lib")
#pragma comment(lib, "Shlwapi.lib")


#pragma warning(disable:4996)

HMODULE G_hModule;

VOID HideDllForX3();
VOID RemoveHideListModulesFromLdrTable();
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

VOID WINAPI AddDll(HMODULE hModule)
{
	PMODULE_HIDE_ENTRY pEntry		= new MODULE_HIDE_ENTRY;
	pEntry->ImageBase			= hModule;
	pEntry->SizeOfImage			= GetSizeOfImage(hModule);
	pEntry->PageCountOfImage	= 0;
	memset(pEntry->PageEntry, 0, sizeof(pEntry->PageEntry));

	TCHAR szFilename[MAX_PATH];
	GetModuleFileName(hModule, szFilename, MAX_PATH);
	LPTSTR lpstrFilename = PathFindFileName(szFilename);

	pfnNtQueryVirtualMemory = (NTQUERYVIRTUALMEMORY)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "NtQueryVirtualMemory");


	PVOID BaseAddress = NULL;
	MEMORY_BASIC_INFORMATION MemoryInformation;
	while (0 != VirtualQuery((LPVOID)BaseAddress, &MemoryInformation, sizeof(MEMORY_BASIC_INFORMATION)))
	{
		if (MemoryInformation.Type == MEM_IMAGE)
		{
			ULONG ReturnLength;
			CHAR Filename[1024];
			pfnNtQueryVirtualMemory(GetCurrentProcess(), BaseAddress, MemoryMappedFilenameInformation, Filename, sizeof(Filename), &ReturnLength);

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

VOID WINAPI Hide()
{
	RemoveHideListModulesFromLdrTable();
	HideDllForX3();
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
		LOG(TEXT("ldrtable--%08lx----%08lx-->%s"),  LdrDataTable->DllBase, LdrDataTable->SizeOfImage, LdrDataTable->FullDllName.Buffer);

		PLIST_ENTRY HideEntryNext = HideLinkHeader.Flink;
		while (HideEntryNext != &HideLinkHeader)
		{
			//判断是否是我们要屏蔽的模块
			PMODULE_HIDE_ENTRY Entry = (PMODULE_HIDE_ENTRY)HideEntryNext;
			if (LdrDataTable->DllBase == Entry->ImageBase)
			{
				LOG(TEXT("ldrtable--X--links"));
				//LdrDataTable->InLoadOrderLinks.Flink->Blink = LdrDataTable->InLoadOrderLinks.Blink;
				//LdrDataTable->InLoadOrderLinks.Blink->Flink = LdrDataTable->InLoadOrderLinks.Flink;

				//LdrDataTable->InMemoryOrderLinks.Flink->Blink = LdrDataTable->InMemoryOrderLinks.Blink;
				//LdrDataTable->InMemoryOrderLinks.Blink->Flink = LdrDataTable->InMemoryOrderLinks.Flink;

				//LdrDataTable->InInitializationOrderLinks.Flink->Blink = LdrDataTable->InInitializationOrderLinks.Blink;
				//LdrDataTable->InInitializationOrderLinks.Blink->Flink = LdrDataTable->InInitializationOrderLinks.Flink;
			}

			HideEntryNext = HideEntryNext->Flink;
		}


		//移动到链表的下一个
		LdrDataTable = (PLDR_DATA_TABLE_ENTRY)LdrDataTable->InLoadOrderLinks.Flink;

	}
}



//DWORD WINAPI ThreadHide(LPVOID lpParam)
//{
//	HMODULE hModule = (HMODULE)lpParam;
//	RemoveHideListModulesFromLdrTable( hModule );
//
//	LOG(TEXT("fix x3"));
//	HideDllForX3();
//
//	return 0;
//
//}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		InitializeListHead(&HideLinkHeader);

		PMODULE_HIDE_ENTRY SelfEntry = new MODULE_HIDE_ENTRY;
		SelfEntry->ImageBase = hModule;
		SelfEntry->SizeOfImage = GetSizeOfImage(hModule);
		InsertTailList(&HideLinkHeader, (PLIST_ENTRY)SelfEntry);
		AddDll(hModule);
		Hide();
		//RemoveHideListModulesFromLdrTable();
		LOG(TEXT("DLL_PROCESS_ATTACH"));
	}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//VOID WINAPI MyRtlInitUnicodeStringEx(
//	_Inout_  PUNICODE_STRING DestinationString,
//	_In_opt_ PCWSTR          SourceString
//)
//{
//	if (wcsstr(SourceString, TEXT("hideself.dll")))
//	{
//		__asm int 3;
//	}
//	return pfnRtlInitUnicodeStringEx(DestinationString, SourceString);
//}



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
	status = pfnNtQueryVirtualMemory( ProcessHandle,
		 BaseAddress,
		 MemoryInformationClass,
		 MemoryInformation,
		 MemoryInformationLength,
		 ReturnLength );
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
						//pMemInfo->State = MEM_FREE;
						//pMemInfo->Type = 0;
						//pMemInfo->Protect = PAGE_NOACCESS;
						//pMemInfo->AllocationProtect = PAGE_NOACCESS;
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



VOID HideDllForX3()
{
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
	//__asm int 3;
	DWORD dwCRC = 0xA891FD11;
	LPVOID lpSearchBase = NULL;
retry:
	DWORD *pdwCRCVirtualAddress = (DWORD*)SearchReadWriteMemory(lpSearchBase, (LPVOID)&dwCRC, sizeof(dwCRC));
	if (pdwCRCVirtualAddress == &dwCRC )
	{
		lpSearchBase = (LPVOID)((ULONG)pdwCRCVirtualAddress + sizeof(dwCRC));
		goto retry;
	}

	LOG(TEXT("real--[%08lx], copy--[%08lx]--rtl--[%08lx]-crc--[%08lx]"), pfnNtQueryVirtualMemory, pfncopyNtQueryVirtualMemory, pfnRtlInitUnicodeStringEx, pdwCRCVirtualAddress);

	//Sleep(60000);
	//__asm int 3;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pfnNtQueryVirtualMemory, &MyNtQueryVirtualMemory); 
	//DetourAttach(&(PVOID&)pfnRtlInitUnicodeStringEx, &MyRtlInitUnicodeStringEx);
	if (FALSE == IsBadReadPtr(pfncopyNtQueryVirtualMemory, 32) && 
		pfnNtQueryVirtualMemory != pfncopyNtQueryVirtualMemory) 
	{
		//DetourAttach(&(PVOID&)pfncopyNtQueryVirtualMemory, &MyNtQueryVirtualMemory);
	}

	DetourTransactionCommit();

	struct CODE_CHECKER {
		LPVOID CodeBase;
		ULONG  CodeSize;
		ULONG  Reserved1;
		ULONG  Reserved2;
		ULONG  CRC;
	};

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
