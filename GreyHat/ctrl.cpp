#include "stdafx.h"
#include "ctrl.h"

#pragma comment( lib, "comctl32.lib" )
//------------------list view ctrl-----------------------
/*
void* get_listview_item_data(HWND hlstview, int iid)
{
	LVITEM lvi = {0}; 
	lvi.mask = LVIF_PARAM; 
	lvi.iItem = iid; 
	ListView_GetItem( hlstview,&lvi ); 
	return( void* )lvi.lParam;
}

void insert_lstview_column(HWND hlstview, LPTSTR text, int width, int iid)
{
	LVCOLUMN lvc = {0}; 
	lvc.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
	lvc.cx = width; 
	lvc.pszText = text; 
	lvc.cchTextMax = _tcslen( text ); 
	ListView_InsertColumn( hlstview,iid,&lvc ); 
}

void insert_lstview_item(HWND hlstview, LPTSTR text, int iid)
{
	LVITEM lvi = {0}; 
	lvi.mask = LVIF_TEXT; 
	lvi.iItem = iid;	
	lvi.pszText = text;	
	lvi.cchTextMax = _tcslen( text );
	ListView_InsertItem( hlstview,&lvi );
}

void LvSetText(HWND hlstview, LPTSTR text, int iid, int subid)
{
	LVITEM lvi = {0}; 
	lvi.mask = LVIF_TEXT; 
	lvi.iItem = iid; 
	lvi.iSubItem = subid;
	lvi.pszText = text;
	lvi.cchTextMax = _tcslen( text ); 
	ListView_SetItem(hlstview, &lvi);
}

Tstring get_lstview_item_text(HWND hlstview, int iid, int subid)
{
	tchar buf[256];
	ListView_GetItemText(hlstview, iid, subid, buf, 256);
	return Tstring(buf);
}

void remove_lstview_item(HWND hlst, int id)
{
	ListView_DeleteItem( hlst, id );
}

void set_lstview_item_data(HWND hlstview, int iid, void* data) 
{
	LVITEM lvi = {0}; 
	lvi.mask = LVIF_PARAM; 
	lvi.iItem = iid; 
	lvi.lParam = (LPARAM)data;
	ListView_SetItem( hlstview,&lvi );
}

int get_lstview_item_count(HWND hlstview)
{
	return ListView_GetItemCount( hlstview );
}

void clear_lstview(HWND hlstview)
{
	ListView_DeleteAllItems( hlstview );
}

int get_select_item(HWND hlst)
{
	return ListView_GetNextItem( hlst, -1, LVNI_SELECTED );
}
//----------------tab ctrl-----------------------


//----------------eidt control
uint32_t get_wndtext_len(HWND hedit)
{
	return SendMessage( hedit, WM_GETTEXTLENGTH, 0,0 );
}



void SetText(HWND hedit, Tstring str)
{
	SendMessage( hedit , WM_SETTEXT, NULL,( LPARAM )str.c_str() );
}
*/


void InsertTab(HWND htab, LPTSTR text, int iid)
{
	TCITEM ti = { 0 };
	ti.mask = TCIF_TEXT;
	ti.pszText = text;
	ti.cchTextMax = _tcslen(text);
	TabCtrl_InsertItem(htab, iid, &ti);
}

Tstring GetText(HWND hWnd)
{
	Tstring tstrText;
	DWORD dwLen =  SendMessage( hWnd, WM_GETTEXTLENGTH, 0,0 ) + 1;

	tchar *pText = new tchar[dwLen];

	memset(pText, 0, dwLen*sizeof(tchar));

	SendMessage( hWnd, WM_GETTEXT, dwLen, (LPARAM)pText );
	tstrText = pText;

	delete[]pText;
	return tstrText;
}

VOID SetText(HWND hWnd, Tstring str)
{
	SendMessage( hWnd , WM_SETTEXT, NULL,( LPARAM )str.c_str() );
}


//ListView
DWORD LvGetItemCount(HWND hLv)
{
	return ListView_GetItemCount( hLv );
}

VOID LvInsertItem(HWND hLv, LPTSTR lptStr, int iIndex, LPARAM pParm)
{
	LVITEM lvi = {0}; 
	lvi.mask = LVIF_TEXT; 
	lvi.iItem = iIndex;	
	lvi.pszText = lptStr;	
	lvi.cchTextMax = _tcslen( lptStr );
	lvi.lParam = pParm;
	ListView_InsertItem( hLv,&lvi );
}
VOID LvSetData(HWND hLv, int iIndex, LPVOID lpData) 
{
	LVITEM lvi = {0}; 
	lvi.mask = LVIF_PARAM; 
	lvi.iItem = iIndex; 
	lvi.lParam = (LPARAM)lpData;
	ListView_SetItem( hLv, &lvi );
}
VOID LvSetText(HWND hLv, LPTSTR lptStr, int iIndex, int subid)
{
	LVITEM lvi = {0}; 
	lvi.mask = LVIF_TEXT; 
	lvi.iItem = iIndex; 
	lvi.iSubItem = subid;
	lvi.pszText = lptStr;
	lvi.cchTextMax = _tcslen( lptStr ); 
	ListView_SetItem(hLv, &lvi);
}

LPVOID LvGetData(HWND hLv, int iIndex)
{
	LVITEM lvi = {0}; 
	lvi.mask = LVIF_PARAM; 
	lvi.iItem = iIndex; 
	ListView_GetItem( hLv,&lvi ); 
	return( LPVOID )lvi.lParam;
}

BOOL LvDeleteItem(HWND hLv, int iIndex)
{
	return ListView_DeleteItem( hLv, iIndex );
}

int LvGetSelItemId(HWND hLv)
{
	return ListView_GetNextItem( hLv, -1, LVNI_SELECTED );
}

Tstring LvGetItemText(HWND hLv, int iid, int subid)
{
	TCHAR tcTemp[256];
	ListView_GetItemText(hLv, iid, subid, tcTemp, 256);
	return Tstring(tcTemp);
}

VOID LvInsertColumn(HWND hLv, LPTSTR text, int width, int iid)
{
	LVCOLUMN lvc = {0}; 
	lvc.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
	lvc.cx = width; 
	lvc.pszText = text; 
	lvc.cchTextMax = _tcslen( text ); 
	ListView_InsertColumn( hLv,iid,&lvc ); 
}

VOID LvRemoveAll(HWND hlstview)
{
	ListView_DeleteAllItems( hlstview );
}