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

	m_bFileterFunEnable	= FALSE;
	m_bReplaceFunEnable = FALSE;

	InitializeCriticalSection(&m_ListCritialSection);
}


CPluginBase::~CPluginBase()
{
}

void CPluginBase::AddPackageProcessor(CGPacketProcessor& processor)
{
	CGPacketProcessor *pNewProcessor = new CGPacketProcessor(processor);
	EnterCriticalSection(&m_ListCritialSection);
	m_PacketProcessorList.push_back(pNewProcessor);
	LeaveCriticalSection(&m_ListCritialSection);
}

void CPluginBase::DeletePackageProcessor(CGPacketProcessor& processor)
{
	list<CGPacketProcessor*>::iterator itr;
	for (itr = m_PacketProcessorList.begin(); itr != m_PacketProcessorList.end(); itr++)
	{
		if (processor.m_strUUID == (*itr)->m_strUUID) 
		{
			EnterCriticalSection(&m_ListCritialSection);
			delete *itr;
			m_PacketProcessorList.erase(itr);
			LeaveCriticalSection(&m_ListCritialSection);
		}
	}
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
	list<CGPacketProcessor*>::iterator itr;
	for (itr = m_PacketProcessorList.begin(); itr != m_PacketProcessorList.end(); itr++)
	{
		EnterCriticalSection(&m_ListCritialSection);
		delete *itr;
		m_PacketProcessorList.erase(itr);
		LeaveCriticalSection(&m_ListCritialSection);
	}


}