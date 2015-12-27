#include "stdafx.h"
#include "sendpage.h"
#include "viewpage.h"
#include "hexeditdlg.h"


CTRL_MAP_TABLE CSendPage::m_CtrlTable[15];

CSendPage SendPage;

LRESULT CALLBACK  CSendPage::NewEditCtrlProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_LBUTTONDBLCLK || (uMsg == WM_KEYUP && wParam == VK_RETURN) )
	{
		Tstring tstrEditHexStr, tstrTemp;
		tstrEditHexStr = GetText(hwnd);

		tstrTemp = PreProcessPackeTstring( tstrEditHexStr );

		CGPacket* pPacket = (CGPacket*)GetWindowLongPtr(hwnd, GWL_USERDATA);

		HWND hHexDlg;

		CHexEditDlg *pHexDlg = new CHexEditDlg;
		if (pPacket)
		{
			pHexDlg->SetUserData(pPacket);
		}
		
		hHexDlg = pHexDlg->Create(MAKEINTRESOURCE( IDD_HEXDLG ), hwnd);
		SendMessage(hHexDlg, WM_SETHEXDLGTEXT, (WPARAM)tstrTemp.c_str(), (LPARAM)hwnd);//----------------->todo
	}
	return	CallWindowProc(SendPage.m_pfnEditCtrlProc, hwnd, uMsg, wParam, lParam);
}


CSendPage::CSendPage()
{
	SendPage.m_CtrlTable[0].dwMenuId = IDC_MS1 ;SendPage.m_CtrlTable[0].dwButtonId = IDC_SB1; SendPage.m_CtrlTable[0].dwEditBoxId = IDC_SE1;SendPage.m_CtrlTable[0].dwCheckId = IDC_SC1;
	SendPage.m_CtrlTable[1].dwMenuId = IDC_MS2 ;SendPage.m_CtrlTable[1].dwButtonId = IDC_SB2; SendPage.m_CtrlTable[1].dwEditBoxId = IDC_SE2;SendPage.m_CtrlTable[1].dwCheckId = IDC_SC2;
	SendPage.m_CtrlTable[2].dwMenuId = IDC_MS3 ;SendPage.m_CtrlTable[2].dwButtonId = IDC_SB3; SendPage.m_CtrlTable[2].dwEditBoxId = IDC_SE3;SendPage.m_CtrlTable[2].dwCheckId = IDC_SC3;
	SendPage.m_CtrlTable[3].dwMenuId = IDC_MS4 ;SendPage.m_CtrlTable[3].dwButtonId = IDC_SB4; SendPage.m_CtrlTable[3].dwEditBoxId = IDC_SE4;SendPage.m_CtrlTable[3].dwCheckId = IDC_SC4;
	SendPage.m_CtrlTable[4].dwMenuId = IDC_MS5 ;SendPage.m_CtrlTable[4].dwButtonId = IDC_SB5; SendPage.m_CtrlTable[4].dwEditBoxId = IDC_SE5;SendPage.m_CtrlTable[4].dwCheckId = IDC_SC5;
	SendPage.m_CtrlTable[5].dwMenuId = IDC_MS6 ;SendPage.m_CtrlTable[5].dwButtonId = IDC_SB6; SendPage.m_CtrlTable[5].dwEditBoxId = IDC_SE6;SendPage.m_CtrlTable[5].dwCheckId = IDC_SC6;
	SendPage.m_CtrlTable[6].dwMenuId = IDC_MS7 ;SendPage.m_CtrlTable[6].dwButtonId = IDC_SB7; SendPage.m_CtrlTable[6].dwEditBoxId = IDC_SE7;SendPage.m_CtrlTable[6].dwCheckId = IDC_SC7;
	SendPage.m_CtrlTable[7].dwMenuId = IDC_MS8 ;SendPage.m_CtrlTable[7].dwButtonId = IDC_SB8; SendPage.m_CtrlTable[7].dwEditBoxId = IDC_SE8;SendPage.m_CtrlTable[7].dwCheckId = IDC_SC8;
	SendPage.m_CtrlTable[8].dwMenuId = IDC_MS9 ;SendPage.m_CtrlTable[8].dwButtonId = IDC_SB9; SendPage.m_CtrlTable[8].dwEditBoxId = IDC_SE9;SendPage.m_CtrlTable[8].dwCheckId = IDC_SC9;
	SendPage.m_CtrlTable[9].dwMenuId = IDC_MS10 ;SendPage.m_CtrlTable[9].dwButtonId = IDC_SB10; SendPage.m_CtrlTable[9].dwEditBoxId = IDC_SE10;SendPage.m_CtrlTable[9].dwCheckId = IDC_SC10;
	SendPage.m_CtrlTable[10].dwMenuId = IDC_MS11 ;SendPage.m_CtrlTable[10].dwButtonId = IDC_SB11; SendPage.m_CtrlTable[10].dwEditBoxId = IDC_SE11;SendPage.m_CtrlTable[10].dwCheckId = IDC_SC11;
	SendPage.m_CtrlTable[11].dwMenuId = IDC_MS12 ;SendPage.m_CtrlTable[11].dwButtonId = IDC_SB12; SendPage.m_CtrlTable[11].dwEditBoxId = IDC_SE12;SendPage.m_CtrlTable[11].dwCheckId = IDC_SC12;
	SendPage.m_CtrlTable[12].dwMenuId = IDC_MS13 ;SendPage.m_CtrlTable[12].dwButtonId = IDC_SB13; SendPage.m_CtrlTable[12].dwEditBoxId = IDC_SE13;SendPage.m_CtrlTable[12].dwCheckId = IDC_SC13;
	SendPage.m_CtrlTable[13].dwMenuId = IDC_MS14 ;SendPage.m_CtrlTable[13].dwButtonId = IDC_SB14; SendPage.m_CtrlTable[13].dwEditBoxId = IDC_SE14;SendPage.m_CtrlTable[13].dwCheckId = IDC_SC14;
	SendPage.m_CtrlTable[14].dwMenuId = IDC_MS15 ;SendPage.m_CtrlTable[14].dwButtonId = IDC_SB15; SendPage.m_CtrlTable[14].dwEditBoxId = IDC_SE15;SendPage.m_CtrlTable[14].dwCheckId = IDC_SC15;


	m_bBusySending = FALSE;
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CSendPage::~CSendPage()
{
	CloseHandle(m_hEvent);
}




/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//
INT_PTR CALLBACK CSendPage::SendPageProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{ 
		HANDLE_MSG ( hWnd, WM_INITDIALOG,		OnWmInit   );
		HANDLE_MSG ( hWnd, WM_COMMAND,			OnWmCommand);
	} 
	return FALSE; 
}


BOOL CSendPage::OnWmInit(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{
	SendPage.m_hWnd = hwnd;
	HWND hEdit;

	for ( int i = 0; i < 15; i++ )
	{
		hEdit = GetDlgItem( hwnd, SendPage.m_CtrlTable[i].dwEditBoxId );
		SendPage.m_pfnEditCtrlProc = (WNDPROC)GetWindowLong(hEdit, GWL_WNDPROC);
		SetWindowLong(hEdit, GWL_WNDPROC, (LONG)NewEditCtrlProc);
	}
	RestoreSendViewSettings();

	return TRUE;
}

VOID CSendPage::OnWmCommand( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
	switch(id)
	{
	case IDC_SB_STOR:	return OnButtonSaveAllData();
	case IDC_SB_CLEAR:  return OnButtonClear();
	case IDC_SBN:		
		return OnClickButtonSBN();

	case IDC_SB1:
	case IDC_SB2:
	case IDC_SB3:
	case IDC_SB4:
	case IDC_SB5:
	case IDC_SB6:
	case IDC_SB7:
	case IDC_SB8:
	case IDC_SB9:
	case IDC_SB10:
	case IDC_SB11:
	case IDC_SB12:
	case IDC_SB13:
	case IDC_SB14:
	case IDC_SB15:
		OnButtonSend(id);
		break;
	default:
		break;
	}

}

DWORD WINAPI CSendPage::ThrdSBNProc( LPVOID lpThreadParameter )
{
	Tstring strRound;
	strRound = GetText(GetDlgItem(SendPage.m_hWnd, IDC_E_ROUND));

	SetText(GetDlgItem(SendPage.m_hWnd, IDC_SBN), _T("停止连续发送"));

	uint32_t uRound = 0;
	uRound = _ttoi(strRound.c_str());

	DWORD dwInterval = 0;
	SendPage.m_bBusySending = TRUE;

	TCHAR tszTemp[256];


	dwInterval = Utility::IniAccess::GetPrivateKeyValInt(RuntimeContext.m_ConfigPath, _T("自动发包"), _T("间隔时间/ms"));


	for ( int i=0; i < 15; i++ )
	{
		 SendPage.m_CtrlTable[i].bChecked =  (BST_CHECKED==Button_GetCheck( GetDlgItem( SendPage.m_hWnd, SendPage.m_CtrlTable[i].dwCheckId ) ));
	}

	DWORD dwWaitRet = WAIT_TIMEOUT;

	for (int j = 0; (j < uRound) && (WAIT_TIMEOUT == dwWaitRet); j++)
	{
		for ( int i = 0; i < 15; i++ )
		{
			if ( SendPage.m_CtrlTable[i].bChecked )
			{
				OnButtonSend( SendPage.m_CtrlTable[i].dwButtonId );
				dwWaitRet = WaitForSingleObject( SendPage.m_hEvent, dwInterval);
				if ( WAIT_OBJECT_0 == dwWaitRet)
				{
					break;
				}
			} 
		}
	}

	SetText(GetDlgItem(SendPage.m_hWnd, IDC_SBN), text("连续循环N次发送"));

	SendPage.m_bBusySending = FALSE;

	return 0;
}

VOID  CSendPage::OnClickButtonSBN()
{
	if (SendPage.m_bBusySending)	{
		SetEvent(SendPage.m_hEvent);
	} else {
		CreateThread(NULL, NULL, ThrdSBNProc, NULL, 0, NULL);
	}
	
}

DWORD CSendPage::GetButtonIndex( DWORD dwButtonId )
{
	for (DWORD  dwI=0; dwI<15; dwI++)
	{
		if (SendPage.m_CtrlTable[dwI].dwButtonId == dwButtonId)
		{
			return dwI;
		}
	}
	return DWORD(-1);
}
Tstring CSendPage::PreProcessPackeTstring(Tstring& tstrStr )
{
	Tstring tstrTemp, tstrRet;
	Tstring::size_type posLeftBracket = 0, posRightBracket = -1;
	posLeftBracket = tstrStr.find(_T("["));

	if (posLeftBracket == Tstring::npos)
	{
		return tstrStr;
	}
	
	tstrRet = tstrStr.substr( 0, posLeftBracket);

ParseRightBracket:

	DWORD dwBracketDataSize;
	posRightBracket = tstrStr.find(_T("]"), posLeftBracket);
	if (posRightBracket == Tstring::npos)
	{
		return _T("");
	}
	dwBracketDataSize = posRightBracket - posLeftBracket -1 ;
	tstrTemp = tstrStr.substr(posLeftBracket + 1, dwBracketDataSize);

	TCHAR tszTemp[1024];
	INT   iInc;

	TCHAR tszBracket[32], tszNum[32];
	TCHAR ch, *ptStr, sign;
	int i, j, flag;

	i=0, j=0, flag=0;
	ptStr = (TCHAR*)tstrTemp.c_str();

	do{
		ch = *ptStr++;
		if(ch == _T('+') || ch == _T('-'))
		{
			sign = ch;
			flag = !flag;
		}
		else if(!flag)
		{
			tszBracket[i++] = ch;
		}else
		{
			tszNum[j++] = ch;
		}
	}while(ch != _T('\0'));
	tszBracket[i] = _T('\0');
	tszNum[j] = _T('\0');

	//todo inc or dec the string
	tstrRet += Tstring(tszBracket);

	BYTE *pHexBuf = new BYTE[i];
	DWORD dwSelHexSize = Utility::StringLib::Tstring2Hex(tszBracket, pHexBuf);
	
	switch (dwSelHexSize)
	{
	case 1:
		*pHexBuf += (sign == _T('+') ? _ttoi(tszNum) : 0 - _ttoi(tszNum));
		break;
	case 2:
		*(WORD*)pHexBuf = htons(htons(*(WORD*)pHexBuf) + (sign == _T('+') ? _ttoi(tszNum) : 0 - _ttoi(tszNum)));
		break;
	case 4:
		*(DWORD*)pHexBuf =  htonl(htonl(*(DWORD*)pHexBuf) + (sign == _T('+') ? _ttoi(tszNum) : 0 - _ttoi(tszNum)));
		break;
	default:
		delete[]pHexBuf;
		return _T("");
	}

	tstrTemp = Utility::StringLib::Hex2Tstring(pHexBuf, dwSelHexSize);

	delete[]pHexBuf;

	tstrTemp = Tstring(_T("[")) + tstrTemp + (sign == _T('+') ?Tstring(_T("+")):Tstring(_T("-"))) + Tstring(tszNum) + Tstring(_T("]"));

	tstrStr.replace(posLeftBracket, dwBracketDataSize + 2, tstrTemp);

	posLeftBracket = tstrStr.find(_T("["), posRightBracket);

	if (posLeftBracket != Tstring::npos){
		tstrRet += tstrStr.substr( posRightBracket+1, posLeftBracket - posRightBracket - 1 );
		goto ParseRightBracket;
	}else
	{
		tstrRet += tstrStr.substr( posRightBracket+1 );
	}

	return tstrRet;
	
}

VOID CSendPage:: OnButtonSend(DWORD dwButtonId)
{
	DWORD dwIndex;
	Tstring tstrTemp, tstrHexStr;

	dwIndex = GetButtonIndex(dwButtonId);
	tstrTemp = GetText(GetDlgItem(SendPage.m_hWnd, SendPage.m_CtrlTable[dwIndex].dwEditBoxId));

	if ( tstrTemp != _T("") )
	{
		tstrHexStr = PreProcessPackeTstring(tstrTemp);

		if (tstrHexStr != _T(""))
		{

			LPBYTE lpHexBuf = new BYTE[tstrHexStr.length()];
			DWORD dwLen = Utility::StringLib::Tstring2Hex(tstrHexStr.c_str(), lpHexBuf);

			SetText(GetDlgItem(SendPage.m_hWnd,SendPage.m_CtrlTable[dwIndex].dwEditBoxId), tstrTemp);


			CGPacket* pPacket = (CGPacket*)GetWindowLongPtr(GetDlgItem(SendPage.m_hWnd,SendPage.m_CtrlTable[dwIndex].dwEditBoxId), GWL_USERDATA);

			CGPacket* pNewPacket = new CGPacket(lpHexBuf, dwLen, pPacket->GetPacketProperty());
			if (pNewPacket)
			{
				PluginWrap.SendData( *pNewPacket);
				delete pNewPacket;
			}
			delete[]lpHexBuf;
		}
	}
}

VOID CSendPage::OnButtonSaveAllData()
{
	TCHAR tszTemp[256];
	Tstring strTemp;

	for (int i=0; i<15; i++)
	{
		strTemp = GetText(GetDlgItem( SendPage.m_hWnd, SendPage.m_CtrlTable[i].dwEditBoxId));
		_stprintf( tszTemp, _T("封包[%d]"), i );
		Utility::IniAccess::SetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("发送封包"), tszTemp, strTemp);

		_stprintf( tszTemp, _T("CHECK[%d]"),i );
		Utility::IniAccess::SetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("发送封包"), tszTemp, 
			BST_CHECKED==Button_GetCheck( GetDlgItem( SendPage.m_hWnd, SendPage.m_CtrlTable[i].dwCheckId) ) ? _T("1"):_T("0"));
	}

	Utility::IniAccess::SetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("自动发包"), _T("间隔时间/ms"), _T("100"));
}

VOID CSendPage::OnButtonClear()
{
	for ( int i=0; i<15; i++ )
		SetText(GetDlgItem( SendPage.m_hWnd, SendPage.m_CtrlTable[i].dwEditBoxId), _T(""));
}

VOID CSendPage::RestoreSendViewSettings()
{
	TCHAR tszTemp[256];
	Tstring strTemp;

	for (int i=0; i<15; i++){

		_stprintf( tszTemp, _T("封包[%d]"), i );
		strTemp = Utility::IniAccess::GetPrivateKeyValString( RuntimeContext.m_ConfigPath, _T("发送封包"), tszTemp);

		SetText(GetDlgItem(SendPage.m_hWnd, SendPage.m_CtrlTable[i].dwEditBoxId), strTemp);

		BOOL bIsChecked;
		_stprintf( tszTemp, _T("CHECK[%d]"), i );
		bIsChecked = Utility::IniAccess::GetPrivateKeyValInt(RuntimeContext.m_ConfigPath, _T("发送封包"), tszTemp);

		Button_SetCheck(GetDlgItem(SendPage.m_hWnd,  SendPage.m_CtrlTable[i].dwCheckId), bIsChecked);

	}
}