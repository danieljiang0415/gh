#pragma once

#include <windows.h>
#include "Utility.h"
#include "hexedit.h"
#include "greyhat.h"


#define WM_SETHEXDLGTEXT (WM_USER+1)

#define HANDLE_HEM_LDBCLICK(hwnd, wParam, lParam, fn) \
	((fn)((hwnd), (UINT)(wParam)), 0L)

#define HANDLE_WM_SETHEXDLGTEXT(hwnd, wParam, lParam, fn) \
	((fn)((hwnd), (tchar*)(wParam), (HWND)(lParam)), 0L)



//#define WND_LONG_HEXDLG 10000

class CFuzzing
{
public:
	CFuzzing();
	~CFuzzing();
public:
	uint32_t m_uStart;//递进开始地址
	uint32_t m_uFzSize;//递进字节数
	uint32_t m_uIncrement;//每次加多少
};


class CHexEditDlg
{
public:
	CHexEditDlg();
	~CHexEditDlg();
public:
	BOOL On_WM_InitDialog(HWND hwnd, HWND hWndFocus, LPARAM lParam);

	void On_HEM_LDBClick(HWND hwnd, UINT uIndex);
	static INT_PTR CALLBACK DlgProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void On_WM_Size(HWND hwnd, UINT state, int cx, int cy);
	void On_WM_Close(HWND hwnd);
	void On_WM_Command( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );
	void On_WM_SetHexViewText(HWND hwnd, tchar* ptext, HWND hparaent);

public:
	HWND Create( LPTSTR lpName, HWND hParaent);

public:
	void SetUserData(LPVOID lpData);
	LPVOID GetUserData();
protected:
//	std::vector<CFuzzing*>m_FuzStepList;
	LPVOID m_lpUserData;

public:
	HWND m_hWnd;
	HWND m_hParaendWnd;
	HWND m_hHexView;

	byte *m_pData, *m_pHexViewDataBuffer;
	uint32_t m_uDataSize/*, m_uStepSize*/;
	//uint32_t m_uSelStart, m_uSelEnd;//0 index~下表索引
	PMULT_SEL m_pStepList;

	HANDLE m_hStepEvent;

	HMENU m_hPopMenu;

	BOOL m_bIsSteping;

	HINSTANCE m_hInst;


	//HWND m_hWndStepList;



};


DWORD WINAPI ThrdStepProc( LPVOID lpThreadParameter );