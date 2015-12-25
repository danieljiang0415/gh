#include "stdafx.h"
#include "RuntimeContext.h"

#define PLUGIN_DLL _T("Plugin.dll")
#define CONFIG_FILE _T("config.ini")

CRuntimeContext::CRuntimeContext()
{
}


CRuntimeContext::~CRuntimeContext()
{
}


VOID CRuntimeContext::InitContext(HMODULE hModule)
{
	m_hRuntimeInstance = hModule;

	m_RootDir = Utility::Module::GetModulePath(hModule);

	m_PluginPath = m_RootDir + _T("\\") + PLUGIN_DLL;

	m_ProcessDir = Utility::Module::GetModulePath(NULL);

	m_ConfigPath = m_RootDir + _T("\\") + CONFIG_FILE;
}
