#pragma once
#include <Windows.h>
#include <commctrl.h>
#include "Utility.h"
typedef unsigned char uint8_t;
typedef int  int32_t;
typedef unsigned int  uint32_t;

//------------------list view ctrl-----------------------
void* get_listview_item_data(HWND hlstview, int iid);
void insert_lstview_column(HWND hlstview, LPTSTR text, int width, int iid);
void insert_lstview_item(HWND hlstview, LPTSTR text, int iid);
void LvSetText(HWND hlstview, LPTSTR text, int iid, int subid);
void set_lstview_item_data(HWND hlstview, int iid, void* data);
int get_lstview_item_count(HWND hlstview);
void clear_lstview(HWND hlstview);
int get_select_item(HWND hlst);
void remove_lstview_item(HWND hlst, int id);
Tstring get_lstview_item_text(HWND hlstview, int iid, int subid);
//----------------tab ctrl-----------------------
void InsertTab(HWND htab, LPTSTR text, int iid);


//-------------------edit ctrol-----------------
uint32_t get_wndtext_len(HWND hedit);
Tstring GetText(HWND hedit);
void SetText(HWND hedit, Tstring str);



#define CTRL_HANDLE(hHandleOwner, dwCtrlId)		GetDlgItem(hHandleOwner, dwCtrlId )
#define ITEMCOUNTOFLISTVIEW(hHandle)			ListView_GetItemCount(hHandle)



//EditBox
Tstring GetText(HWND hWnd);
VOID SetText(HWND hWnd, Tstring str);
//ListView
DWORD LvGetItemCount(HWND hLv);
VOID LvInsertItem(HWND hLv, LPTSTR lptStr, int iIndex, LPARAM pParm = NULL);
VOID LvSetData(HWND hLv, int iIndex, LPVOID lpData);
VOID LvSetText(HWND hLv, LPTSTR lptStr, int iIndex, int subid);
LPVOID LvGetData(HWND hLv, int iIndex);
BOOL LvDeleteItem(HWND hLv, int iIndex);
int LvGetSelItemId(HWND hLv);
Tstring LvGetItemText(HWND hLv, int iid, int subid);
VOID LvInsertColumn(HWND hLv, LPTSTR text, int width, int iid);
VOID LvRemoveAll(HWND hlstview);