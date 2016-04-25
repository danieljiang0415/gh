#include "stdafx.h"
#include <strsafe.h>
#include "viewpage.h"
#include "filterpage.h"
#include "GPacket.h"



CViewPage ViewPage;

CViewPage::CViewPage()
{

}

CViewPage::~CViewPage()
{

}

VOID CViewPage::ShowPacket(HWND hwnd, CGPacket& packetBuf)
{
	try
	{

		TCHAR tszTemp[256];

		LPBYTE lpbRawData;
		DWORD  dwPacketLen, dwItem;

		lpbRawData = packetBuf.GetRawBuffer();
		dwPacketLen = packetBuf.GetBufferLen();

		dwItem = LvGetItemCount( ViewPage.m_hListView );
		//
		_stprintf( tszTemp, _T("%d"), dwItem );

		LvInsertItem(ViewPage.m_hListView, tszTemp, dwItem, 1);
		LvSetData( ViewPage.m_hListView, dwItem, &packetBuf);
		
		//
		SYSTEMTIME sysTime;
		GetSystemTime( &sysTime );
		_stprintf( tszTemp, _T("%02d:%02d:%02d"), ( sysTime.wHour+8 )%24, sysTime.wMinute, sysTime.wSecond );
		LvSetText( ViewPage.m_hListView,tszTemp, dwItem, 2 );
		//
		_stprintf( tszTemp, _T("%d"), dwPacketLen );
		LvSetText( ViewPage.m_hListView, tszTemp, dwItem, 3 );

		Tstring tstrTemp;
		tstrTemp = Utility::StringLib::Hex2Tstring(lpbRawData, dwPacketLen>64?64:dwPacketLen );
		LvSetText( ViewPage.m_hListView,(LPTSTR)tstrTemp.c_str(), dwItem, 4 );

		tstrTemp = packetBuf.GetPacketProperty().strIpAddr + Tstring(_T(":")) + packetBuf.GetPacketProperty().strPort;
		LvSetText(ViewPage.m_hListView, (LPTSTR)tstrTemp.c_str(), dwItem, 5);

		if (ViewPage.m_bEnableScroll)
		{
			PostMessage( ViewPage.m_hListView, WM_VSCROLL, SB_BOTTOM, 0 ); 
		}
		RedrawWindow( ViewPage.m_hListView, NULL, NULL, RDW_UPDATENOW|RDW_ERASE|RDW_INVALIDATE);
		if (ViewPage.m_bAutoShowContent)
		{
			HexEdit_LoadBuffer(ViewPage.m_hHexView, lpbRawData, dwPacketLen);
		}
	}
	catch (...)
	{
		MessageBox(NULL, _T("on_wm_newpkt"), NULL, MB_OK);	
	}
	return;
}

INT_PTR CALLBACK CViewPage::ViewPageProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{ 
		HANDLE_MSG ( hWnd, WM_INITDIALOG,		OnWmInit   );
		HANDLE_MSG ( hWnd, WM_COMMAND,			OnWmCommand);
		HANDLE_MSG ( hWnd, WM_NOTIFY,			OnWmNotify );
		HANDLE_MSG ( hWnd, WM_CLOSE,            OnWmClose  );
		HANDLE_MSG ( hWnd, WM_SIZE,             OnWMSize  );
	} 
	return FALSE; 
}

VOID CViewPage::OnWMSize( HWND hWnd, UINT state, int cx, int cy )
{
	RECT rcWnd, rcHexView, rcList, rcGroup;
	GetClientRect(ViewPage.m_hWnd, &rcWnd);
	GetClientRect(GetDlgItem(ViewPage.m_hWnd, IDC_SGROUP), &rcGroup);
	GetWindowRect(ViewPage.m_hListView, &rcList);
	GetClientRect(ViewPage.m_hHexView,  &rcHexView);

	MoveWindow(GetDlgItem(ViewPage.m_hWnd, IDC_SGROUP), 4, 2, rcWnd.right - rcWnd.left - 8, rcWnd.bottom - rcWnd.top - 8, TRUE);
	MoveWindow(ViewPage.m_hHexView, 8, rcList.bottom - rcList.top + 60, rcList.right-rcList.left, ((rcWnd.bottom - rcWnd.top) - (rcList.bottom-rcList.top) - 75), TRUE);

}


BOOL CViewPage::OnWmInit(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{
	ViewPage.m_hWnd = hwnd;
	ViewPage.m_hListView = GetDlgItem(hwnd, IDC_PACKETLST);

	DWORD dwExStyle;
	dwExStyle = ListView_GetExtendedListViewStyle( ViewPage.m_hListView );
	dwExStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( ViewPage.m_hListView, dwExStyle );

	LvInsertColumn( ViewPage.m_hListView, _T("#"),		  25,  0 );
	LvInsertColumn( ViewPage.m_hListView, _T("!"),		  20,  1 );
	LvInsertColumn( ViewPage.m_hListView, _T("时戳"),     60,  2 );
	LvInsertColumn( ViewPage.m_hListView, _T("包长"),     35,  3 );
	LvInsertColumn( ViewPage.m_hListView, _T("原始数据"), 250, 4 );
	LvInsertColumn( ViewPage.m_hListView, _T("IP/端口"),  140, 5 );


	RegisterHexEditorClass( RuntimeContext.m_hRuntimeInstance );
	ViewPage.m_hMenu = LoadMenu( RuntimeContext.m_hRuntimeInstance, _T("transmit") );

	//RECT rc;
	//GetClientRect(GetDlgItem(hwnd, IDC_HEXV), &rc);
	ViewPage.m_hHexView = CreateWindowEx( 0, text("HexEdit32"), text(""), 
		WS_CHILD | WS_VISIBLE, 0, 0, 1, 1, ViewPage.m_hWnd, NULL/*( HMENU )IDC_HEXVIEW*/, RuntimeContext.m_hRuntimeInstance, 0 ); 
	HexEdit_SetReadOnly(ViewPage.m_hHexView, TRUE);

	CheckDlgButton( hwnd, IDC_ENABLE_LOG,		FALSE );
	CheckDlgButton( hwnd, IDC_ATO_SCROLL,		TRUE );
	CheckDlgButton( hwnd, IDC_ENABLE_FILTER,	TRUE );
	CheckDlgButton( hwnd, IDC_ENABLE_REPLACE,	TRUE );
	CheckDlgButton( hwnd, IDC_AUTOSHOWCONTENT,	TRUE );
	CheckDlgButton(hwnd, IDC_SHOWSEND, TRUE);
	CheckDlgButton(hwnd, IDC_SHOWRECV, FALSE);

	EnableWindow(GetDlgItem(hwnd, IDC_CSTPF),   TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_BPASS), FALSE);

	ViewPage.m_hBlockEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	InitializeCriticalSection( &ViewPage.m_csPacketListCriticalSection );
	//__asm int 3;
	if (FALSE == PluginWrap.InstallPlugin(&OnProcessSendData, &OnProcessRecvData))
		MessageBox(NULL, TEXT("安装Plugin失败！"), TEXT("错误"), MB_OK);


	PluginWrap.EnableSeePacket(TRUE);
	PluginWrap.EnableFilter(TRUE);

	ViewPage.m_bEnableScroll	= TRUE;
	ViewPage.m_bShowSend		= TRUE;
	ViewPage.m_bAutoShowContent = TRUE;


	ViewPage.m_ChooseFont.lStructSize    = sizeof (CHOOSEFONT) ;
	ViewPage.m_ChooseFont.hwndOwner      = hwnd ;
	ViewPage.m_ChooseFont.hDC            = NULL ;
	ViewPage.m_ChooseFont.lpLogFont      = &ViewPage.m_LogFont ;
	ViewPage.m_ChooseFont.iPointSize     = 0 ;
	ViewPage.m_ChooseFont.Flags          = CF_INITTOLOGFONTSTRUCT |	CF_SCREENFONTS | CF_EFFECTS ;
	ViewPage.m_ChooseFont.rgbColors      = 0 ;
	ViewPage.m_ChooseFont.lCustData      = 0 ;
	ViewPage.m_ChooseFont.lpfnHook       = NULL ;
	ViewPage.m_ChooseFont.lpTemplateName = NULL ;
	ViewPage.m_ChooseFont.hInstance      = NULL ;
	ViewPage.m_ChooseFont.lpszStyle      = NULL ;
	ViewPage.m_ChooseFont.nFontType      = 0 ;      
	ViewPage.m_ChooseFont.nSizeMin       = 0 ;
	ViewPage.m_ChooseFont.nSizeMax       = 0 ;

	//if (FALSE == RegisterHotKey( hwnd, 1, MOD_SHIFT, 0x41))  //0x42 is VK_B 'b'VK_OEM_3
	//{
	//	MessageBox(hwnd, _T("Hotkey 'ALT+b' registered, using MOD_NOREPEAT flag\n"), _T("!!"), MB_OK);
	//}

	CreateDialog(RuntimeContext.m_hRuntimeInstance, MAKEINTRESOURCE(IDD_ST_DIALOG), hwnd, &CGPacketFilterSettingPage::FilterPageMessageProc);
	return TRUE;
}

DWORD CALLBACK CViewPage::CoreLibUpdataThread(LPVOID lpParam)
{
	//while (TRUE)
	//{
	//	Sleep(30*60*1000);

	//	if (ViewPage.m_CoreLib.DetachHookFromGameProcess())
	//	{
	//		Tstring strUrl = Tstring(TEXT("http://cdsign.sinaapp.com/?do=load")) + Tstring(TEXT("&id=")) + GlobalEnv.tszGameName;

	//		Utility::Http::HttpRead2File( strUrl.c_str(), GlobalEnv.tszCoreDllPath);
	//	}

	//	ViewPage.m_CoreLib.AttachHookIntoGameProcess(&OnProcessSendData, &OnProcessRecvData);

	//}
	return 0;
}

BOOL CViewPage::IsButtonChecked(HWND hctrl)
{
	return (BST_CHECKED==Button_GetCheck( hctrl ));
}

VOID CViewPage::OnWmCommand( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
	switch(id)
	{
	//case IDC_ENABLE_LOG:    	g_enable_log =  IsButtonChecked(hWndCtl) ; if (g_enable_log)	MessageBox(hWnd, text("该功能尚未开发"), text("!!"), MB_OK); break;
	case IDC_ATO_SCROLL:		ViewPage.m_bEnableScroll =  IsButtonChecked(hWndCtl) ;	break;
	case IDC_ENABLE_FILTER:		PluginWrap.EnableFilter( IsButtonChecked(hWndCtl)) ;	break;
	//case IDC_ENABLE_REPLACE:    ViewPage.m_bEnableReplace =  IsButtonChecked(hWndCtl) ;	break;
	case IDC_SHOWSEND:			ViewPage.m_bShowSend = IsButtonChecked(hWndCtl);	break;
	case IDC_SHOWRECV:			ViewPage.m_bShowRecv = IsButtonChecked(hWndCtl);	break;
	case IDC_AUTOSHOWCONTENT:	ViewPage.m_bAutoShowContent = IsButtonChecked(hWndCtl);	break;
	case IDC_ENABLE_BLOCK:      ViewPage.m_bEnableBlock = IsButtonChecked(hWndCtl) ;		break;

	case IDC_BCLE:				FreePackets();break;
	case IDC_BSF:				
		{
			POINT pt;
			pt.x = 0; pt.y = 0;
			ClientToScreen(hWnd, &pt);

			RECT rc;
			GetWindowRect(FilterSettingPage.m_hMainPage, &rc);
			MoveWindow(FilterSettingPage.m_hMainPage, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
			ShowWindow(FilterSettingPage.m_hMainPage, SW_SHOW);
		}
		break;
	case IDC_CSTPF:	PluginWrap.EnableSeePacket( !IsButtonChecked(hWndCtl) ); break;

	case IDC_BPASS:				SetEvent(ViewPage.m_hBlockEvent);break;
	case IDC_BFONT:				
		{
			if (ChooseFont (&ViewPage.m_ChooseFont)) 
			{
				HFONT hfont;
				hfont = CreateFontIndirect (&ViewPage.m_LogFont);
				SetFont(g_hmain_dlg, hfont);
			}
		}
		break;
	case IDC_MS1:	case IDC_MS2:	case IDC_MS3:	case IDC_MS4:	case IDC_MS5:	case IDC_MS6:
	case IDC_MS7:	case IDC_MS8:	case IDC_MS9:	case IDC_MS10:	case IDC_MS11:	case IDC_MS12:
	case IDC_MS13:	case IDC_MS14:	case IDC_MS15:
		{
			int nSel = -1;
			nSel = LvGetSelItemId(ViewPage.m_hListView);
			if ( nSel != -1 )
			{
				CGPacket *pPacket = NULL;
				pPacket = ( CGPacket* )LvGetData( ViewPage.m_hListView, nSel );
				if ( pPacket )
				{
					for ( int i=0; i < 15; i++ )
					{
						if ( id == SendPage.m_CtrlTable[i].dwMenuId )
						{
							SetText(GetDlgItem( SendPage.m_hWnd, SendPage.m_CtrlTable[i].dwEditBoxId),
								Utility::StringLib::Hex2Tstring(pPacket->GetBuffer(), pPacket->GetBufferLen()));
							SetWindowLongPtr(GetDlgItem(SendPage.m_hWnd, SendPage.m_CtrlTable[i].dwEditBoxId), GWL_USERDATA, (DWORD_PTR)pPacket);
						}
					}					
				}
			}
		}
		break;
	//case IDC_BHOOK:
	//	{
	//		g_corelib.DetachHookFromGameProcess();
	//		g_corelib.AttachHookIntoGameProcess(&OnProcessSendData, &OnProcessRecvData);
	//	}
	//	break;
	}
}

LRESULT ProcessCustomDraw(LPNMHDR lParam)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

	switch (lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		return CDRF_NOTIFYSUBITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
	{
		COLORREF itemColor = RGB(0, 0, 0);
		int iColumn = lplvcd->iSubItem;
		int iRow = (int)lplvcd->nmcd.dwItemSpec;
		CGPacket* pPacket = (CGPacket*)LvGetData(lplvcd->nmcd.hdr.hwndFrom, lplvcd->nmcd.dwItemSpec);
		if (pPacket)
		{
			PACKET_IO eType = pPacket->GetPacketProperty().ioType;
			if (eType == IO_OUTPUT)
				itemColor = RGB(196, 238, 254);
			else if (eType == IO_INPUT)
				itemColor = RGB(253, 241, 249);

			lplvcd->clrTextBk = itemColor;
		}
		
		return CDRF_DODEFAULT;
	}
	break;

	case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
		//if (m_hOldFont != NULL)
		//{
		//	SelectObject(lplvcd->nmcd.hdc, m_hOldFont);
		//	m_hOldFont = NULL;
		//}

		return CDRF_DODEFAULT;
		break;
	}
}

BOOL CViewPage::OnWmNotify( HWND hwnd, INT id, LPNMHDR pNMHDR)
{
	int nSel = -1;

	switch(pNMHDR->code )
	{
	case LVN_ITEMCHANGED:
		{
			CGPacket *pPacket = NULL;
			nSel = LvGetSelItemId(ViewPage.m_hListView);
			if (nSel != -1) {
				pPacket = ( CGPacket* )LvGetData( ViewPage.m_hListView, nSel );
				if ( pPacket )
				{
					HexEdit_LoadBuffer( ViewPage.m_hHexView, pPacket->GetBuffer(), pPacket->GetBufferLen() );
				}
			}

		}
		break;
	case NM_RCLICK:
		{
			CGPacket *ppkt = NULL;
			nSel = LvGetSelItemId(ViewPage.m_hListView);
			if (nSel != -1) {
				ppkt = ( CGPacket* )LvGetData( ViewPage.m_hListView, nSel );

				POINT point;
				GetCursorPos( &point );
				HMENU hSubMenu = GetSubMenu( ViewPage.m_hMenu, 0 );
				if (hSubMenu != NULL/* && ppkt->get_pkt_direction()==IO_OUPUT*/)
				{
					TrackPopupMenu( hSubMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL );
				}
			}
		}
		break;
	case LVN_KEYDOWN:
		{
			LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)pNMHDR;

			if (pnkd->wVKey == VK_DELETE)
			{
				nSel = LvGetSelItemId(ViewPage.m_hListView);
				if (nSel != -1) {
					CGPacket *pPacket = (CGPacket *)LvGetData( ViewPage.m_hListView, nSel );
					delete pPacket;
					ListView_DeleteItem( ViewPage.m_hListView, nSel );
					LVITEM lvi = {0};
					lvi.mask = LVIF_STATE;
					lvi.state = LVIS_SELECTED;
					lvi.stateMask = LVIS_SELECTED;
					lvi.iItem = nSel;
					ListView_SetItem( ViewPage.m_hListView,&lvi );
				}

			}
		}
		break;
	case NM_CUSTOMDRAW:
		{
			SetWindowLong(hwnd, DWL_MSGRESULT,
				(LONG)ProcessCustomDraw(pNMHDR));
			return TRUE;
		}
		break;
	}
	return TRUE;
}

VOID CViewPage::OnWmClose(HWND hwnd)
{
	TerminateThread(ViewPage.m_hUpdateThrd, 0);
	FreePackets();
	UnregisterHexEditorClass( RuntimeContext.m_hRuntimeInstance );
	DestroyMenu( ViewPage.m_hMenu );
	if (FALSE == PluginWrap.UnInstallPlugin())
		MessageBox(NULL, TEXT("卸载Plugin失败！"), TEXT("错误"), MB_OK);
	DeleteCriticalSection(&ViewPage.m_csPacketListCriticalSection); 
}


VOID CViewPage::FreePackets()
{
	DWORD dwItemCount;
	dwItemCount = LvGetItemCount(ViewPage.m_hListView);
	for ( int i=0; i<dwItemCount; i++ )
	{
		CGPacket* pPacket;
		pPacket = (CGPacket*)LvGetData(ViewPage.m_hListView, i );

		delete pPacket;
	}
	LvRemoveAll(ViewPage.m_hListView);
	HexEdit_LoadBuffer( ViewPage.m_hHexView, NULL, 0 );
}
//for huang
//00 10 00 08 00 0a 00 00 00 00 00 00 00 00 00 00 25 29 f6 04 8f 00 71 50 
//DWORD dwPlayerId;

VOID CALLBACK CViewPage::OnProcessSendData( CGPacket& packetBuf )
{
	EnterCriticalSection( &ViewPage.m_csPacketListCriticalSection );

	ShowPacket(ViewPage.m_hWnd, packetBuf);
	if ( ViewPage.m_bEnableBlock )//如果是block 模式
	{
		HexEdit_SetReadOnly(ViewPage.m_hHexView, FALSE);//EnableWindow(ViewPage.m_hHexView, TRUE);//GetDlgItem(ViewPage.m_hWnd,IDC_PACKETLST)
		EnableWindow(GetDlgItem(ViewPage.m_hWnd, IDC_BPASS), TRUE);
		WaitForSingleObject( ViewPage.m_hBlockEvent, INFINITE );
		EnableWindow(GetDlgItem(ViewPage.m_hWnd, IDC_BPASS), FALSE);
		//MessageBox(NULL, text("修改完了点确定"), text("有数据来了~~"), MB_OK);
		HexEdit_SetReadOnly(ViewPage.m_hHexView, TRUE);//EnableWindow(ViewPage.m_hHexView, FALSE);//EnableWindow(GetDlgItem(ViewPage.m_hWnd,IDC_PACKETLST), TRUE);
		HexEdit_CopyBuffer( ViewPage.m_hHexView, packetBuf.GetRawBuffer(), packetBuf.GetBufferLen() );
	}

	LeaveCriticalSection( &ViewPage.m_csPacketListCriticalSection );

}

VOID CALLBACK CViewPage::OnProcessRecvData(CGPacket& packetBuf)
{
	//EnterCriticalSection( &ViewPage.m_csPacketListCriticalSection );

	//if (ViewPage.m_bEnbleGrab)
	//{
	//	if (ViewPage.m_bShowRecv == FALSE)
	//		packetBuf.SetSkipFlag(TRUE);

	//	if ((packetBuf.IsSkipped() == TRUE /*&& ViewPage.m_bEnableFilter*/) ||
	//		ViewPage.m_bEnbleGrab == FALSE)//skip掉或者没截包，则删除该包
	//	{
	//		delete &packetBuf;
	//		LeaveCriticalSection(&ViewPage.m_csPacketListCriticalSection);
	//		return;
	//	}

	//	ShowPacket(ViewPage.m_hWnd, packetBuf);
	//	if (ViewPage.m_bEnableBlock)
	//	{
	//		HexEdit_SetReadOnly(ViewPage.m_hHexView, FALSE);
	//		WaitForSingleObject( ViewPage.m_hBlockEvent, INFINITE );
	//		HexEdit_SetReadOnly(ViewPage.m_hHexView, TRUE);
	//		HexEdit_CopyBuffer( ViewPage.m_hHexView, packetBuf.GetBuffer(), packetBuf.GetBufferLen() );
	//	}
	//}

	//LeaveCriticalSection( &ViewPage.m_csPacketListCriticalSection );
}

BOOL CALLBACK CViewPage::EnumChildProc(HWND hwnd, LPARAM lParam)
{
	SendMessage(hwnd, WM_SETFONT, (WPARAM)lParam, NULL);
	HDC hdc = GetDC(hwnd);
	SetTextColor(hdc, ViewPage.m_ChooseFont.rgbColors);
	InvalidateRect(hwnd, NULL, TRUE);
	ReleaseDC(hwnd, hdc);
	return TRUE;
}

VOID CViewPage::SetFont(HWND hParentWnd, HFONT hfont)
{
	EnumChildWindows(hParentWnd, EnumChildProc,(LPARAM)hfont);
}