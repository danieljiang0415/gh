#include "stdafx.h"
#include "PluginBase.h"

SENDPROCHANDLER CPluginBase::m_pfnHandleSendProc;
RECVPROCHANDLER CPluginBase::m_pfnHandleRecvProc;

CPluginBase::CPluginBase()
{
	m_pfnSendInterface = NULL;
	m_pfnRecvInterface = NULL;

	m_pfnHandleRecvProc = NULL;
	m_pfnHandleSendProc = NULL;
}


CPluginBase::~CPluginBase()
{
}
