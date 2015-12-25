#pragma once
#include <Windows.h>
#include "hexedit.h"
#include "GPacket.h"
#include <CommDlg.h>

class CViewPage
{
public:
	CViewPage();
	~CViewPage();

	static BOOL OnWmInit(HWND hwnd, HWND hWndFocus, LPARAM lParam);
	static VOID SetFont(HWND hParentWnd, HFONT hfont);
	static VOID ProcessPacket(CGPacket& packetBuf);
	static VOID CViewPage::ShowPacket(HWND hwnd, CGPacket& packetBuf);
	static INT_PTR CALLBACK CViewPage::ViewPageProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
	static BOOL IsButtonChecked(HWND hctrl);
	static VOID OnWmCommand( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );
	static BOOL OnWmNotify( HWND hwnd, INT id, LPNMHDR pnm );
	static VOID OnWmClose(HWND hwnd);
	static VOID OnWMSize( HWND hWnd, UINT state, int cx, int cy );
	static VOID FreePackets();
	static VOID CALLBACK OnProcessSendData(CGPacket& packetBuf);
	static VOID CALLBACK OnProcessRecvData(CGPacket& packetBuf);
	static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);
	static DWORD CALLBACK CoreLibUpdataThread(LPVOID lpParam);

public:
	HMENU		m_hMenu;
	HWND		m_hWnd;
	HWND		m_hHexView;
	HANDLE		m_hBlockEvent;
	CHOOSEFONT	m_ChooseFont;
	LOGFONT		m_LogFont;
	HWND		m_hListView;
	CRITICAL_SECTION m_csPacketListCriticalSection;

	//DISPATCH_CONTEXT m_DispatchContext;
	HANDLE      m_hUpdateThrd;

public:
	BOOL m_bEnbleGrab;
	BOOL m_bEnableScroll;
	BOOL m_bEnableFilter;
	BOOL m_bEnableReplace;
	BOOL m_bEnableBlock;
	BOOL m_bShowSend;
	BOOL m_bShowRecv;
	BOOL m_bAutoShowContent;
};


extern CViewPage ViewPage;