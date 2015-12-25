// BadBoy.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "detours.h"

#pragma comment(lib,"detours.lib")
//
//compile error
//winsock2.h ws2def.h 
//---------/showIncludes compile option to show the orders of include header

#pragma region Includes and Manifest Dependencies


#include "greyhat.h"


#define CONFIG_FILE _T("config.ini")
#define PLUGIN_DLL _T("Plugin.dll")

HWND g_hmain_dlg;

CPluginWrap* g_PluginWrap;
CRuntimeContext RuntimeContext;

void on_pagetab_notify(HWND htabctrl,UINT pnm_code) 
{
	switch ( pnm_code )
	{
	case TCN_SELCHANGE:
		{
			switch(TabCtrl_GetCurSel( htabctrl ))
			{
			case 0:
				ShowWindow(ViewPage.m_hWnd, SW_SHOW);
				ShowWindow(SendPage.m_hWnd,  SW_HIDE);
				break;
			case 1:
				ShowWindow(ViewPage.m_hWnd, SW_HIDE);
				ShowWindow(SendPage.m_hWnd,  SW_SHOW);
				break;
			case 2:
				ShowWindow(ViewPage.m_hWnd, SW_HIDE);
				ShowWindow(SendPage.m_hWnd,  SW_HIDE);
				break;
			}
		}
	}
}

BOOL OnWMNotify( HWND hwnd, INT id, LPNMHDR pnm )
{
    switch( id )
    {
    case IDC_PAGE_TAB:
        on_pagetab_notify(GetDlgItem(hwnd, IDC_PAGE_TAB), pnm->code);
        break;
    }
    return FALSE;
}

VOID WriteCoreLibFileExtraInfo(LPCTSTR lpcLocalFileName)
{
	HANDLE hCoreLib;
	hCoreLib = CreateFile( lpcLocalFileName,GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if (hCoreLib == INVALID_HANDLE_VALUE){
		return;
	}
	
}

DWORD WINAPI ThreadProc( LPVOID lParam )
{
	CRuntimeContext *pContext = (CRuntimeContext *)lParam;

	if (!Utility::File::IsFileExist(pContext->m_PluginPath.c_str()))
	{
		MessageBox(NULL, _T("找不到核心工作模块！"), TEXT("错误"), MB_OK);
		return -1;
	}
	else
	{
		new CPluginWrap(pContext->m_PluginPath);
	}
	DialogBox(pContext->m_hRuntimeInstance, MAKEINTRESOURCE( IDD_MAINDIALOG ), NULL, MainUIDlgProc );
    FreeLibraryAndExitThread(pContext->m_hRuntimeInstance, 0 );
    return 0;
}

VOID OnWMSize( HWND hWnd, UINT state, int cx, int cy )
{
	HWND hTabCtrl;
	hTabCtrl = GetDlgItem(hWnd, IDC_PAGE_TAB);


	RECT rcClient, rcTabItem;
	GetClientRect(GetDlgItem(hWnd, IDC_PAGE_TAB), &rcClient);
	TabCtrl_GetItemRect(GetDlgItem(hWnd, IDC_PAGE_TAB), 0, &rcTabItem);

	MoveWindow(hTabCtrl, rcClient.left, rcClient.top, cx, cy, TRUE);

	//PostMessage( ViewPage.m_hWnd, WM_SIZE, (WPARAM)state, MAKELPARAM((cx), (cy)) );
	//PostMessage( SendPage.m_hWnd, WM_SIZE, (WPARAM)state, MAKELPARAM((cx), (cy)) );

	int x, y;
	x = rcClient.left;
	y = rcTabItem.bottom;
	MoveWindow(ViewPage.m_hWnd, x, y, cx, cy-y, TRUE);
	MoveWindow(SendPage.m_hWnd,  x, y, cx, cy-y, TRUE);
}

VOID init_tab_ctrl(HWND htabctrl)
{
	InsertTab( htabctrl,	 text("封包视图"),			0 );
	InsertTab( htabctrl,	 text("发送/多重发送"),		1 );

	CreateDialog(RuntimeContext.m_hRuntimeInstance, MAKEINTRESOURCE(IDD_PKT_DIALOG), htabctrl, &CViewPage::ViewPageProc);
	CreateDialog(RuntimeContext.m_hRuntimeInstance, MAKEINTRESOURCE(IDD_SD_DIALOG), htabctrl, &CSendPage::SendPageProc);


	RECT rcClient, rcTabItem;
	GetClientRect(htabctrl, &rcClient);
	TabCtrl_GetItemRect(htabctrl, 0, &rcTabItem);

	int x, y, cx, cy;
	x = rcClient.left;
	y = rcTabItem.bottom;
	cx = rcClient.right - rcClient.left;
	cy = rcClient.bottom - rcClient.top - rcTabItem.bottom;

	MoveWindow(ViewPage.m_hWnd, x, y, cx, cy, TRUE);
	MoveWindow(SendPage.m_hWnd,  x, y, cx, cy, TRUE);
	ShowWindow(ViewPage.m_hWnd, SW_SHOW);

	TabCtrl_SetCurSel(htabctrl, 0);
}



BOOL OnWMInitDlg( HWND hWnd, HWND hWndFocus, LPARAM lParam )
{
	// Load and register Tab control class
	INITCOMMONCONTROLSEX iccx;
	iccx.dwSize = sizeof( INITCOMMONCONTROLSEX );
	iccx.dwICC = ICC_TAB_CLASSES;
	if ( !InitCommonControlsEx( &iccx ) )
		return FALSE;


	g_hmain_dlg = hWnd;
	if (FALSE == RegisterHotKey( hWnd, 1, MOD_CONTROL, VK_OEM_3))  //0x42 is 'b'
	{
		MessageBox(hWnd, _T("Hotkey 'ALT+b' registered, using MOD_NOREPEAT flag\n"), _T("!!"), MB_OK);
	}

	//Set Dialog Icon
	SendMessage( hWnd, WM_SETICON, ICON_BIG, ( LPARAM )LoadIcon( RuntimeContext.m_hRuntimeInstance, MAKEINTRESOURCE( IDI_ICON_BIG ) ) );
	/////////////////////////////////////////////////////////////////////////
	// Create the tab common control.
	// 
	RegisterHexEditorClass( RuntimeContext.m_hRuntimeInstance );
	//g_popup_menu = LoadMenu(RuntimeContext.m_hRuntimeInstance, text("hexedit"));


	init_tab_ctrl(GetDlgItem(hWnd, IDC_PAGE_TAB));


	RECT rcArea;
	SystemParametersInfo( SPI_GETWORKAREA, NULL, &rcArea, NULL );
	SetWindowPos( hWnd, HWND_BOTTOM, rcArea.left+300, rcArea.top+300, -1, -1,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
	//MoveWindow(hWnd, rcArea.left+300, rcArea.top+300, -1, -1, TRUE);
	//TODO
	return TRUE;
}

void OnWMClose( HWND hWnd )
{
	SendMessage(ViewPage.m_hWnd, WM_CLOSE, NULL, NULL);
	SendMessage(SendPage.m_hWnd, WM_CLOSE, NULL, NULL);
	EndDialog( hWnd, 0 );
	//ExitProcess(NULL);
}


INT_PTR CALLBACK MainUIDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message )
	{
		HANDLE_MSG ( hWnd, WM_INITDIALOG,		OnWMInitDlg );
		HANDLE_MSG ( hWnd, WM_CLOSE,			OnWMClose );
		HANDLE_MSG ( hWnd, WM_SIZE,				OnWMSize );
		HANDLE_MSG ( hWnd, WM_NOTIFY,			OnWMNotify );
		//HANDLE_MSG ( hWnd, WM_COMMAND,			Cls_OnCommand );
		//HANDLE_MSG ( hWnd, WM_HOTKEY,			Cls_OnHotKey);
	default:
		return FALSE;	// Let system deal with msg
	}
}

#ifdef _MANAGED
#pragma managed( push, off )
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					  )
{
    if ( ul_reason_for_call == DLL_PROCESS_ATTACH )
    {
		RuntimeContext.InitContext(hModule);
		CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, &RuntimeContext, 0, NULL );
    }

    return TRUE;
}

#ifdef _MANAGED
#pragma managed( pop )
#endif

