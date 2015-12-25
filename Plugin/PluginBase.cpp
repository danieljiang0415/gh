#include "stdafx.h"
#include "PluginBase.h"

SENDPROCHANDLER CPluginBase::m_pfnHandleSendProc;
RECVPROCHANDLER CPluginBase::m_pfnHandleRecvProc;

CPluginBase*	CPluginBase::m_PlugInstance;

CPluginBase::CPluginBase()
{
	m_pfnSendInterface = NULL;
	m_pfnRecvInterface = NULL;

	m_pfnHandleRecvProc = NULL;
	m_pfnHandleSendProc = NULL;

	m_bFileterFunEnable	= FALSE;
	m_bReplaceFunEnable = FALSE;

	InitializeCriticalSection(&m_ListCritialSection);
	m_PlugInstance = this;
}


CPluginBase::~CPluginBase()
{
}

void CPluginBase::ClearPacketFilters()
{
	EnterCriticalSection(&m_ListCritialSection);
	list<CGPacketFilter*>::iterator i;
	for (i = m_PacketProcessorList.begin(); i != m_PacketProcessorList.end(); i++)
	{
		delete *i;
	}
	m_PacketProcessorList.clear();
	LeaveCriticalSection(&m_ListCritialSection);
}

void CPluginBase::AddPacketFilter(CGPacketFilter& processor)
{
	CGPacketFilter *pNewProcessor = new CGPacketFilter(processor);
	EnterCriticalSection(&m_ListCritialSection);
	m_PacketProcessorList.push_back(pNewProcessor);
	LeaveCriticalSection(&m_ListCritialSection);
}

void CPluginBase::DeletePacketFilter(CGPacketFilter& processor)
{
	EnterCriticalSection(&m_ListCritialSection);
	list<CGPacketFilter*>::iterator i;
	for (i = m_PacketProcessorList.begin(); i != m_PacketProcessorList.end(); i++)
	{
		if (processor.m_strUUID == (*i)->m_strUUID) 
		{
			delete *i;
			m_PacketProcessorList.erase(i);
		}
	}
	LeaveCriticalSection(&m_ListCritialSection);
}

void CPluginBase::SetFilterEnable(BOOL bEnable)
{
	m_bFileterFunEnable = bEnable;
}
void CPluginBase::SetReplaceEnable(BOOL bEnable)
{
	m_bReplaceFunEnable = bEnable;
}

void CPluginBase::PreProcessGPacket(CGPacket&packet)
{
	EnterCriticalSection(&m_ListCritialSection);
	list<CGPacketFilter*>::iterator i;
	for (i = m_PacketProcessorList.begin(); i != m_PacketProcessorList.end(); i++)
	{
		CGPacketFilter* p = *i;
		if (m_bFileterFunEnable && p->m_ProcessType == PROCESS_FILTER)
		{
			if (packet.Find(p->m_strKey) || packet.AdvancedMach(p->m_strAdvanceKey))
				packet.SetFiltered();
		}
		if (m_bReplaceFunEnable&& p->m_ProcessType == PROCESS_REPLACE)
		{
			packet.Replace(p->m_strKey, p->m_strReplace);
		}
	}

	LeaveCriticalSection(&m_ListCritialSection);
}