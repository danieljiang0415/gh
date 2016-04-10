#include "stdafx.h"
#include "PluginBase.h"

SENDPROCHANDLER CPluginBase::m_pfnHandleSendProc;
RECVPROCHANDLER CPluginBase::m_pfnHandleRecvProc;

CPluginBase*	CPluginBase::m_PlugInstance;

CPluginBase::CPluginBase()
	: m_bSeePacket(TRUE)
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
	EnterCriticalSection(&m_ListCritialSection);
	list<CGPacketFilter*>::iterator i;
	for (i = m_PacketProcessorList.begin(); i != m_PacketProcessorList.end(); i++)
	{
		CGPacketFilter *pFilter =  *i;
		if (pFilter->m_strUUID == processor.m_strUUID)
		{
			pFilter->m_strAdvanceKey = processor.m_strAdvanceKey;
			pFilter->m_strKey = processor.m_strKey;
			pFilter->m_strReplace = processor.m_strReplace;
			LeaveCriticalSection(&m_ListCritialSection);
			return;
		}
	}

	CGPacketFilter *pNewProcessor = new CGPacketFilter(processor);
	m_PacketProcessorList.push_back(pNewProcessor);
	LeaveCriticalSection(&m_ListCritialSection);
}

void CPluginBase::DeletePacketFilter(Tstring& strFilterUUID)
{
	EnterCriticalSection(&m_ListCritialSection);
	list<CGPacketFilter*>::iterator i;
	for (i = m_PacketProcessorList.begin(); i != m_PacketProcessorList.end(); i++)
	{
		if (strFilterUUID == (*i)->m_strUUID)
		{
			delete *i;
			m_PacketProcessorList.erase(i);
			break;
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
		if (p->m_strReplace != Tstring(_T("")))
		{
			packet.Replace(p->m_strKey, p->m_strReplace);
		}
		if (m_bFileterFunEnable)
		{
			if (p->m_strKey != Tstring(_T(""))) 
			{
				if (packet.Find(p->m_strKey))
				{
					packet.SetFiltered();
				}
			}
			if (p->m_strAdvanceKey != Tstring(_T("")))
			{
				if (packet.AdvancedMach(p->m_strAdvanceKey))
				{
					packet.SetFiltered();
				}

			}

		}

	}

	LeaveCriticalSection(&m_ListCritialSection);
}

VOID CPluginBase::EnableCrabPacket(BOOL bEnable)
{
	m_bSeePacket = bEnable;
	return VOID();
}
