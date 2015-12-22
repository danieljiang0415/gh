#pragma once
#include <windows.h>

typedef struct _IAT_INFO {
	DWORD dwAddr;      // IAT所在地址 
	DWORD dwOldValue;  // IAT原始函数地址 
	DWORD dwNewValue;  // IAT新函数地址 
} IAT_INFO, *PIAT_INFO;

class CIatHook
{
public:
	CIatHook();
	~CIatHook();
public:
	BOOL Hook(LPCSTR TargetModule, LPCSTR FunModule, LPCSTR lpcFunName, ULONG ulNewProc);
	VOID UnHook();
	DWORD GetOldFunAddr();  // 获取原始函数地址
private:
	IAT_INFO iat;     // 用于保存IAT项信息
};

