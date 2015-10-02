#pragma once
#include "greyhat.h"


typedef struct _CTRL_MAP_TABLE
{
	DWORD dwMenuId;
	DWORD dwEditBoxId;
	DWORD dwButtonId;
	DWORD dwCheckId;//use for send n check box
	BOOL  bChecked;
}CTRL_MAP_TABLE, *PCTRL_MAP_TABLE;


class CSendPage
{
public:
	CSendPage();
	~CSendPage();

	static VOID RestoreSendViewSettings();
	static VOID OnButtonClear();
	static VOID OnButtonSaveAllData();
	static VOID OnButtonSend(DWORD dwButtonId);
	static Tstring PreProcessPackeTstring(Tstring& tstrStr );
	static DWORD GetButtonIndex( DWORD dwButtonId );
	static VOID  OnClickButtonSBN();
	static DWORD WINAPI ThrdSBNProc( LPVOID lpThreadParameter );
	static VOID OnWmCommand( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );
	static BOOL OnWmInit(HWND hwnd, HWND hWndFocus, LPARAM lParam);
	static INT_PTR CALLBACK SendPageProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK  NewEditCtrlProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	HWND		m_hWnd;
	LRESULT (CALLBACK* m_pfnEditCtrlProc)(HWND, UINT, WPARAM, LPARAM);

	BOOL		m_bBusySending;
	HANDLE		m_hEvent;

	static CTRL_MAP_TABLE m_CtrlTable[15];

};

extern CSendPage SendPage;
