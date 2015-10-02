#include "stdafx.h"
#include "hexeditdlg.h"
#include "Resource.h"
#include "greyhat.h"

#include "ctrl.h"
#include "core.h"


DWORD WINAPI ThrdStepProc( LPVOID lpThreadParameter )
{
	CHexEditDlg *pHexEditDlg = (CHexEditDlg*)lpThreadParameter;

	DWORD dwInterval = 0;


	dwInterval = Utility::IniAccess::GetPrivateKeyValInt(Tstring(GlobalEnv.tszCfgFilePath), _T("自动发包"), _T("间隔时间/ms"));
	

	pHexEditDlg->m_bIsSteping = TRUE;
	while ( WAIT_TIMEOUT == WaitForSingleObject(pHexEditDlg->m_hStepEvent, dwInterval) )
	{

		UINT i0, i1;
		byte *p = pHexEditDlg->m_pHexViewDataBuffer;

		PMULT_SEL pNode;
		pNode = pHexEditDlg->m_pStepList;
		while (pNode != NULL)
		{
			if (pNode->SelStart < pNode->SelEnd)
			{
				i0 = pNode->SelStart;
				i1 = pNode->SelEnd;
			}
			else
			{
				i0 = pNode->SelEnd;
				i1 = pNode->SelStart;
			}
			uint32_t Length = i1 - i0;


			switch (Length)
			{
			case 1:
				p[i0]+=pNode->Property;
				break;
			case 2:
				*(WORD*)&(p[i0]) = *(WORD*)&(p[i0]) + pNode->Property;
				break;
			case 4:
				*(DWORD*)&(p[i0]) = *(DWORD*)&(p[i0]) + pNode->Property;
				break;
			default:
				*(DWORD*)&(p[i0]) = *(DWORD*)&(p[i0]) + pNode->Property;
				break;
			}
			pNode = pNode->pNextNode;
		}


		InvalidateRect(pHexEditDlg->m_hHexView, NULL, FALSE);
		byte *pBuf = new byte[pHexEditDlg->m_uDataSize];
		memcpy(pBuf, p, pHexEditDlg->m_uDataSize);
		CPacket *packetBuf = (CPacket*)pHexEditDlg->GetUserData();
		if (packetBuf)
		{
			CCoreLib::SendData(*packetBuf);
		}
		
		delete[]pBuf;
	}
	pHexEditDlg->m_bIsSteping = FALSE;
	return 0;
}


CHexEditDlg::CHexEditDlg()
{
	m_hInst = GlobalEnv.hUiInst;

	m_hWnd = NULL;
	m_hParaendWnd = NULL;
	m_hHexView = NULL;

	m_pData = NULL;
	m_pHexViewDataBuffer = NULL;
	//m_uSelStart = 0, m_uSelEnd = 0;//0 index~下表索引

	m_bIsSteping = FALSE;

	m_hStepEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_lpUserData = NULL;

}
CHexEditDlg::~CHexEditDlg()
{
	delete[]m_pData;
	DeleteMenu(m_hPopMenu, 0, MF_BYPOSITION);
	CloseHandle(m_hStepEvent);
}


UINT uStepLen;
BOOL OnStepWMInitDialog(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{
	tchar buf[256];
	_itot(uStepLen, buf, 10);
	SetText(GetDlgItem(hwnd,IDC_E_STEP), buf);
	return TRUE;
}
BOOL OnWMStepSize(HWND hwnd, UINT state, int cx, int cy)
{
	//RECT rc;
	//GetClientRect(hwnd, &rc);

	POINT Pt;
	GetCursorPos( &Pt );

	MoveWindow(hwnd, Pt.x, Pt.y, cx, cy,TRUE);

	return TRUE;
}
VOID OnStepCommand( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
	UINT uRet = 0;
	if (id == IDOK)
	{
		Tstring strNum;
		strNum = GetText(GetDlgItem(hWnd,IDC_E_STEP));
		if (strNum != text(""))
		{
			uRet = _ttoi(strNum.c_str());
		}
		EndDialog(hWnd, uRet);
	}
}

INT_PTR CALLBACK StdpDlgProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch (uMsg)
	{
		HANDLE_MSG ( hwnd, WM_COMMAND,	OnStepCommand);
		HANDLE_MSG ( hwnd, WM_SIZE,		OnWMStepSize);
		HANDLE_MSG ( hwnd, WM_INITDIALOG,		OnStepWMInitDialog);
	}
	return FALSE;
}

void CHexEditDlg::On_HEM_LDBClick(HWND hwnd, UINT uIndex)
{
	PMULT_SEL pMulSelList = (PMULT_SEL)HexEdit_GetMulSelPtr(m_hHexView);

	INT i0, i1;

	PMULT_SEL pNode, pNext;

	if ( NULL != pMulSelList )
	{
		pNext = pMulSelList;
		do{
			if (pNext->SelStart < pNext->SelEnd)
			{
				i0 = pNext->SelStart;
				i1 = pNext->SelEnd;
			}
			else
			{
				i0 = pNext->SelEnd;
				i1 = pNext->SelStart;
			}
			if (i0<=uIndex && uIndex<i1)
			{
				uStepLen = pNext->Property;
				pNext->Property = 	DialogBox(m_hInst, MAKEINTRESOURCE(IDD_STEPNUM), m_hWnd, StdpDlgProc);
				return;
			}
			pNext = pNext->pNextNode;
		}while (NULL != pNext);
	}
}

INT_PTR CALLBACK CHexEditDlg::DlgProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CHexEditDlg *pThisDlg = NULL;

	pThisDlg = (CHexEditDlg*)GetWindowLongPtr(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
		HANDLE_MSG ( hwnd, WM_INITDIALOG,	pThisDlg->On_WM_InitDialog);
		HANDLE_MSG ( hwnd, WM_SIZE,			pThisDlg->On_WM_Size);
		HANDLE_MSG ( hwnd, WM_CLOSE,        pThisDlg->On_WM_Close);
		HANDLE_MSG ( hwnd, HEM_LDBCLICK,		pThisDlg->On_HEM_LDBClick);
		HANDLE_MSG ( hwnd, WM_SETHEXDLGTEXT, pThisDlg->On_WM_SetHexViewText);
		HANDLE_MSG ( hwnd, WM_COMMAND,		pThisDlg->On_WM_Command);
	}
	return FALSE;
}


BOOL CHexEditDlg::On_WM_InitDialog(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{
	CHexEditDlg *pThisDlg = (CHexEditDlg*)lParam;
	
	pThisDlg->m_hHexView= CreateWindowEx( 0, TEXT("HexEdit32"), TEXT(""), 
		WS_CHILD | WS_VISIBLE, 0, 0, 1, 1, hwnd, NULL/*( HMENU )IDC_HEX_EDIT*/, GlobalEnv.hUiInst, 0 ); 

	SetWindowLongPtr(hwnd, GWL_USERDATA, (DWORD_PTR)pThisDlg);

	pThisDlg->m_hWnd = hwnd;

	//pThisDlg->m_hWndStepList = GetDlgItem(hwnd, IDC_L_STEP);

	//DWORD dwExStyle;


	//dwExStyle = ListView_GetExtendedListViewStyle( pThisDlg->m_hWndStepList );
	//dwExStyle |= LVS_EX_FULLROWSELECT;
	//ListView_SetExtendedListViewStyle( pThisDlg->m_hWndStepList, dwExStyle );

	//insert_lstview_column( pThisDlg->m_hWndStepList, text("位置"),		   100,  0 );
	//insert_lstview_column( pThisDlg->m_hWndStepList, text("递进数据长度(字节)"),100,  1 );
	//insert_lstview_column( pThisDlg->m_hWndStepList, text("递进增量"),     100,  2 );

	return TRUE;
}

void CHexEditDlg::On_WM_Size(HWND hwnd, UINT state, int cx, int cy)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	MoveWindow(m_hHexView, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top-50,	TRUE);
}

void CHexEditDlg::On_WM_Close(HWND hwnd)
{
	
	byte *pbuf;
	uint32_t len;
	len = HexEdit_GetBufferSize(m_hHexView);

	pbuf = new byte[len];
	HexEdit_CopyBuffer( m_hHexView, pbuf, len);

	Tstring str;
	str = Utility::StringLib::Hex2Tstring(pbuf, len);
	delete[]pbuf;
	SetText(m_hParaendWnd, str);

	if (m_bIsSteping)
	{
		SetEvent(m_hStepEvent);
		Sleep(100);
	}

	DestroyWindow(hwnd);

	delete this;
}


void CHexEditDlg::On_WM_SetHexViewText(HWND hwnd, tchar* ptext, HWND hParaent)
{
	m_hParaendWnd = hParaent;
	byte* pbuf = new byte[_tcslen(ptext)];
	uint32_t len = Utility::StringLib::Tstring2Hex(ptext, pbuf);
	HexEdit_LoadBuffer( m_hHexView, pbuf, len );
	delete[]pbuf;

	m_uDataSize = len;
	m_pHexViewDataBuffer = (byte*)HexEdit_GetBufferPtr(m_hHexView);
	m_pData     = new byte[m_uDataSize]; memcpy(m_pData, m_pHexViewDataBuffer, m_uDataSize);
}



void CHexEditDlg::On_WM_Command( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
	switch(id)
	{
	case IDC_BTN_STARTSTEP:
		{
			if (FALSE == m_bIsSteping)
			{
				m_pStepList = (PMULT_SEL)HexEdit_GetMulSelPtr(m_hHexView);

				CreateThread(NULL, NULL, ThrdStepProc, (LPVOID)this, 0, NULL);

				SetText(hWndCtl, text("结束递进"));
			}else
			{
				SetEvent(m_hStepEvent);
				SetText(hWndCtl, text("开始递进"));
			}
		}
		break;

	case IDC_BTN_RESTORE:	
		{
			if (m_pHexViewDataBuffer != NULL && m_pData != NULL)
			{
				memcpy(m_pHexViewDataBuffer, m_pData, m_uDataSize);
				InvalidateRect(m_hHexView, NULL, FALSE);
			}
		}
		break;
	}
}

HWND CHexEditDlg::Create( LPTSTR lpName, HWND hParaent)
{
	HWND hDlg = CreateDialogParam(GlobalEnv.hUiInst, lpName/*MAKEINTRESOURCE(IDD_HEXDLG)*/, hParaent, DlgProc, (LPARAM)this);
	ShowWindow(hDlg,SW_SHOW);
	return hDlg;
}

void CHexEditDlg::SetUserData(LPVOID lpData)
{
	m_lpUserData = lpData;
}

LPVOID CHexEditDlg::GetUserData()
{
	return m_lpUserData;
}