#include "stdafx.h"
#include "IatHook.h"


CIatHook::CIatHook()
{
}


CIatHook::~CIatHook()
{
}

/************************************************************************/
/* 函数名称: Init                                                       */
/* 函数作者: PiaoYun/P.Y.G                                              */
/* 函数参数: pDllName:目标API所在的DLL名称                              */
/*           pFunName:目标API名称                                       */
/*           dwNewProc:自定义的函数地址                                 */
/* 函数作者: PiaoYun/P.Y.G                                              */
/* 函数作者: PiaoYun/P.Y.G                                              */
/************************************************************************/
BOOL CIatHook::Hook(LPCSTR TargetModule, LPCSTR FunModule, LPCSTR lpcFunName, ULONG ulNewProc)
{
	PIAT_INFO pIATInfo = &iat;

	// 检查参数是否合法 
	if (!TargetModule || !lpcFunName || !ulNewProc || !pIATInfo)
		return FALSE;

	// 检测目标模块是否存在 
	char szTempDllName[256] = { 0 };
	DWORD dwBaseImage = (DWORD)GetModuleHandleA(TargetModule);
	if (dwBaseImage == 0)
		return FALSE;

	// 取得PE文件头信息指针 
	PIMAGE_DOS_HEADER   pDosHeader = (PIMAGE_DOS_HEADER)dwBaseImage;
	PIMAGE_NT_HEADERS   pNtHeader = (PIMAGE_NT_HEADERS)(dwBaseImage + (pDosHeader->e_lfanew));
	PIMAGE_OPTIONAL_HEADER32 pOptionalHeader = &(pNtHeader->OptionalHeader);
	PIMAGE_SECTION_HEADER  pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pNtHeader + 0x18 + pNtHeader->FileHeader.SizeOfOptionalHeader);

	// 遍历输入表 
	PIMAGE_THUNK_DATA pThunk, pIAT;
	PIMAGE_IMPORT_DESCRIPTOR pIID = (PIMAGE_IMPORT_DESCRIPTOR)(dwBaseImage + pOptionalHeader->DataDirectory[1].VirtualAddress);
	while (pIID->FirstThunk)
	{
		// 检测是否目标模块 -- 不分大小写 
		if (_stricmp((PCHAR)(dwBaseImage + pIID->Name), FunModule))
		{
			pIID++;
			continue;
		}

		pIAT = (PIMAGE_THUNK_DATA)(dwBaseImage + pIID->FirstThunk);
		if (pIID->OriginalFirstThunk)
			pThunk = (PIMAGE_THUNK_DATA)(dwBaseImage + pIID->OriginalFirstThunk);
		else
			pThunk = pIAT;

		// 遍历IAT表 
		DWORD dwThunkValue = 0;
		while ((dwThunkValue = *(PDWORD)pThunk) != 0)
		{
			if ((dwThunkValue & IMAGE_ORDINAL_FLAG32) == 0)
			{
				// 检测是否目标函数 -- 分大小写
				if (strcmp((PCHAR)(dwBaseImage + dwThunkValue + 2), lpcFunName) == 0)
				{
					// 填充函数重定位信息 
					pIATInfo->dwAddr = (DWORD)pIAT;
					pIATInfo->dwOldValue = *(PDWORD)pIAT;
					pIATInfo->dwNewValue = ulNewProc;

					DWORD dwOldProtect = 0;
					VirtualProtect((LPVOID)pIAT, 4, PAGE_READWRITE, &dwOldProtect);
					*(PDWORD)pIATInfo->dwAddr = (DWORD)ulNewProc;
					VirtualProtect((LPVOID)pIAT, 4, dwOldProtect, &dwOldProtect);
					return TRUE;
				}
			}

			pThunk++;
			pIAT++;
		}

		pIID++;
	}
	return FALSE;
}

VOID CIatHook::UnHook()
{
	if (iat.dwAddr)
	{
		DWORD dwOldProtect = 0;
		VirtualProtect((LPVOID)iat.dwAddr, 4, PAGE_READWRITE, &dwOldProtect);
		*(PDWORD)iat.dwAddr = iat.dwOldValue;
		VirtualProtect((LPVOID)iat.dwAddr, 4, dwOldProtect, &dwOldProtect);
	}
}

DWORD CIatHook::GetOldFunAddr()
{
	return iat.dwOldValue;
}