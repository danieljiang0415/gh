#pragma once
#include<Windows.h>

typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
}UNICODE_STRING, *PUNICODE_STRING;

typedef struct _PEB_LDR_DATA
{
	ULONG               Length;
	BOOLEAN             Initialized;
	PVOID               SsHandle;
	LIST_ENTRY          InLoadOrderModuleList;
	LIST_ENTRY          InMemoryOrderModuleList;
	LIST_ENTRY          InInitializationOrderModuleList;
	PVOID				EntryInProgress;
	BOOLEAN				ShutdownInProgress;
	PVOID				ShutdownThreadId;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	PVOID SectionPointer;
	ULONG CheckSum;
	ULONG TimeDateStamp;
	PVOID LoadedImports;
	PVOID EntryPointActivationContext;
	PVOID PatchInformation;
	LIST_ENTRY ForwarderLinks;
	LIST_ENTRY ServiceTagLinks;
	LIST_ENTRY StaticLinks;
	PVOID ContextInformation;
	ULONG OriginalBase;
	LARGE_INTEGER LoadTime;
}LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;


typedef LONG NTSTATUS;

typedef enum _MEMORY_INFORMATION_CLASS {
	MemoryBasicInformation,
	MemoryWorkingSetInformation,
	MemoryMappedFilenameInformation,
	MemoryBasicVlmInformation
} MEMORY_INFORMATION_CLASS;
typedef struct _MEMORY_MAPPED_FILE_NAME_INFORMATION {
	UNICODE_STRING Name;
	WCHAR     Buffer[512];
} MEMORY_MAPPED_FILE_NAME_INFORMATION, *PMEMORY_MAPPED_FILE_NAME_INFORMATION;

typedef NTSTATUS(WINAPI * NTQUERYVIRTUALMEMORY)(
	HANDLE ProcessHandle,
	PVOID BaseAddress,
	MEMORY_INFORMATION_CLASS MemoryInformationClass,
	PVOID MemoryInformation,
	ULONG MemoryInformationLength,
	PULONG ReturnLength OPTIONAL
	);

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))


#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }


#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }


#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}


#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}


#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))


#define CONTAINING_RECORD(address, type, field) ((type FAR *)( \
                                          (PCHAR)(address) - \
                                          (PCHAR)(&((type *)0)->field)))


#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))

//DWORD WINAPI SelfCheckPointPatch(LPVOID lpparam)
//{
//	unsigned char x3SelfCheck[] = {
//		/*
//		0E3E605A   /75 13           JNZ SHORT 0E3E606F
//		0E3E605C   |FF45 FC         INC DWORD PTR SS:[EBP-4]
//		0E3E605F   |83C1 14         ADD ECX,14
//		0E3E6062   |395D FC         CMP DWORD PTR SS:[EBP-4],EBX
//		0E3E6065  ^|7C D2           JL SHORT 0E3E6039
//		0E3E6067   |33C0            XOR EAX,EAX
//		0E3E6069   |40              INC EAX
//		0E3E606A   |5F              POP EDI
//		0E3E606B   |5E              POP ESI
//		0E3E606C   |5B              POP EBX
//		0E3E606D   |C9              LEAVE
//		0E3E606E   |C3              RETN
//		0E3E606F   \33C0            XOR EAX,EAX
//		0E3E6071  ^ EB F7           JMP SHORT 0E3E606A
//		0E3E6073    6A 00           PUSH 0
//		*/
//		0x75, 0x13, 0xFF, 0x45, 0xFC, 0x83, 0xC1, 0x14, 0x39, 0x5D, 0xFC, 0x7C, 0xD2, 0x33, 0xC0, 0x40, 0x5F, 0x5E, 0x5B, 0xC9, 0xC3, 0x33, 0xC0, 0xEB, 0xF7, 0x6A, 0x00
//	};
//	MEMORY_BASIC_INFORMATION MemoryInformation;
//	LPVOID BaseAddress = NULL;
//	for (int i = 0; i < 100; i++)
//	{
//		BaseAddress = NULL;
//		while (true) 
//		{
//			LPVOID x3SelfCheckKeyPoint = SearchExecutableMemory(BaseAddress, x3SelfCheck, sizeof(x3SelfCheck));
//
//			if (x3SelfCheckKeyPoint != NULL) 
//			{
//				if (0 != VirtualQuery(x3SelfCheckKeyPoint, &MemoryInformation, sizeof(MEMORY_BASIC_INFORMATION))) 
//				{
//					LOG(TEXT("VirtualQuery--che--[%08lx] -- %08lx -- %08lx"), x3SelfCheckKeyPoint, MemoryInformation.Type, MemoryInformation.Protect);
//					if (MemoryInformation.Type == MEM_PRIVATE && MemoryInformation.Protect & 0xf0) 
//					{
//						if (FALSE == IsBadReadPtr(x3SelfCheckKeyPoint, 2))	
//						{
//							DWORD dwOldProtect;
//							VirtualProtect(x3SelfCheckKeyPoint, 2, PAGE_EXECUTE_READWRITE, &dwOldProtect);
//							//memset(x3SelfCheckKeyPoint, 0x90, 2);
//							LOG(TEXT("self--che--[%08lx]"), x3SelfCheckKeyPoint);
//						}
//					}
//				}
//				BaseAddress = (LPVOID)( (ULONG)x3SelfCheckKeyPoint + sizeof(x3SelfCheck));
//				continue;
//
//			}
//			else
//			{
//				break;
//			}
//		}
//
//		Sleep(5000);
//	}
//	return 0;
//}

//ULONG systemcall_ptr;
//void __declspec(naked) systemcall()
//{
//	__asm
//	{
//		cmp eax, 20h
//		jz lable1
//	lable2:
//		jmp systemcall_ptr
//	lable1:
//		cmp [esp+10h], 0
//		jnz lable2
//		cmp 
//		mov ebx, [esp+14h]
//		mov [ebx+10h], MEM_FREE
//		mov[ebx + 18], 0
//		mov[ebx + 08h], PAGE_NOACCESS
//		mov[ebx + 14h], PAGE_NOACCESS
//	}
//}
//
//void __declspec(naked) hooksystemcall()
//{
//	__asm
//	{
//		mov eax, fs:[0xc0]
//		mov systemcall_ptr, eax
//		mov eax, systemcall
//		mov fs:[0xc0], eax
//		ret
//	}
//}

//typedef struct 
//{
//	LPVOID	lpAddress;
//	ULONG	ulSize;
//	BYTE	Data[32];
//}MEMORY_PROTECT, *PMEMORY_PROTECT;
//
//MEMORY_PROTECT Mem1;
//
//VOID ProtectReadMemory(LPVOID lpAddress, DWORD dwSize)
//{
//	MEMORY_BASIC_INFORMATION MemoryInformation;
//	if (0 != VirtualQuery(lpAddress, &MemoryInformation, sizeof(MEMORY_BASIC_INFORMATION)))
//	{
//		DWORD dwOldProtect;
//		VirtualProtect(MemoryInformation.BaseAddress, MemoryInformation.RegionSize, PAGE_NOACCESS, &dwOldProtect);
//		LOG(TEXT("ProtectReadMemory copy-PAGE_NOACCESS-[%08lx], %08lx"), MemoryInformation.BaseAddress, MemoryInformation.RegionSize);
//	}
//}
//
//LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo)
//{
//	LOG(TEXT("ExceptionCode --- %08lx--, ExceptionAddress--%08lx--\n "),
//		ExceptionInfo->ExceptionRecord->ExceptionCode,
//		ExceptionInfo->ExceptionRecord->ExceptionAddress);
//
//	switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
//	{
//	case DBG_PRINTEXCEPTION_C:
//		return EXCEPTION_CONTINUE_EXECUTION;
//	case EXCEPTION_SINGLE_STEP:
//	{
//		DWORD dwOldProtect;
//		VirtualProtect(Mem1.lpAddress, Mem1.ulSize, PAGE_NOACCESS, &dwOldProtect);
//		return EXCEPTION_CONTINUE_EXECUTION;
//	}
//	break;
//	case EXCEPTION_ACCESS_VIOLATION:
//	{
//		DWORD dwAccessType, dwOldProtect;
//		LPVOID lpAddress;
//		dwAccessType = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
//		lpAddress = (LPVOID)ExceptionInfo->ExceptionRecord->ExceptionInformation[1];
//
//		MEMORY_BASIC_INFORMATION MemoryInformation;
//		if (0 != VirtualQuery(lpAddress, &MemoryInformation, sizeof(MEMORY_BASIC_INFORMATION)))
//		{
//			if (((ULONG)lpAddress^(ULONG)Mem1.lpAddress)>>12 == 0)
//			{
//				LOG(TEXT("ExceptionType --- %s--, AccessAddress--%08lx--\n"), dwAccessType ? TEXT("WRITE") : TEXT("READ"), lpAddress);
//				VirtualProtect(Mem1.lpAddress, Mem1.ulSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
//				ExceptionInfo->ContextRecord->EFlags |= 0x100;
//				return EXCEPTION_CONTINUE_EXECUTION;
//			}
//
//		}
//		return EXCEPTION_CONTINUE_SEARCH;
//	}
//	break;
//
//	default:
//		return EXCEPTION_CONTINUE_SEARCH;
//	}
//}

