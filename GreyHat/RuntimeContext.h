#pragma once
#include"utility.h"
class CRuntimeContext
{
public:
	CRuntimeContext();
	~CRuntimeContext();
	Tstring m_RootDir;
	HMODULE m_hRuntimeInstance;
	VOID InitContext(HMODULE hModule);
	Tstring m_PluginPath;
	Tstring m_ProcessDir;
	Tstring m_ConfigPath;
};

