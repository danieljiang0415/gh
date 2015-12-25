#pragma once
#include <Windows.h>
#include "greyhat.h"
#include <list>

using namespace std;


class CGPacketFilterSettingPage
{
public:
	CGPacketFilterSettingPage();
	~CGPacketFilterSettingPage();


public:
	static BOOL OnWmInit(HWND hwnd, HWND hWndFocus, LPARAM lParam);
	static void OnWmCommand( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );
	static BOOL OnWmNotify( HWND hwnd, INT id, LPNMHDR pnm );
	static INT_PTR CALLBACK FilterPageMessageProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
	static VOID OnButtonAdd();
	static VOID OnButtonDel();
	static VOID SaveAllFilters();
	static VOID ReLoadAllFilters();
	static VOID OnButtonClear();
	

public:
	HWND m_hMainPage;
	HWND m_hFilterList;
	HWND m_hKeyWord;
	HWND m_hReplace;
	HWND m_hAdvFilter;
};



extern CGPacketFilterSettingPage FilterSettingPage;