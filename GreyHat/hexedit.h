#pragma once
#include <Windows.h>

#define HEX_EDIT_CLASS_NAME (L"HexEdit32")

ATOM WINAPI
RegisterHexEditorClass(HINSTANCE hInstance);
BOOL WINAPI
UnregisterHexEditorClass(HINSTANCE hInstance);

/* styles */
#define HES_READONLY	        (0x800)
#define HES_LOWERCASE	        (0x10)
#define HES_UPPERCASE	        (0x8)
#define HES_AUTOVSCROLL	        (0x40)
#define HES_HIDEADDRESS	        (0x4)

/* messages */
#define HEM_BASE	            (WM_USER + 50)
#define HEM_LOADBUFFER	        (HEM_BASE + 1)
#define HEM_COPYBUFFER	        (HEM_BASE + 2)
#define HEM_SETMAXBUFFERSIZE	(HEM_BASE + 3)
#define HEM_SETREADONLY         (HEM_BASE + 4)
#define HEM_RCLICK			    (HEM_BASE + 5)
#define HEM_GETMULSELLIST		(HEM_BASE + 6)
#define HEM_GETSELSIZE			(HEM_BASE + 7)
#define HEM_GETBUFFERPTR		(HEM_BASE + 8)
#define HEM_ADDMULTSEL			(HEM_BASE + 9)
#define HEM_LDBCLICK			(HEM_BASE + 10)

/* macros */
#define HexEdit_LoadBuffer(hWnd, Buffer, Size) \
  SendMessage((hWnd), HEM_LOADBUFFER, (WPARAM)(Buffer), (LPARAM)(Size))

#define HexEdit_ClearBuffer(hWnd) \
  SendMessage((hWnd), HEM_LOADBUFFER, 0, 0)

#define HexEdit_CopyBuffer(hWnd, Buffer, nMax) \
  SendMessage((hWnd), HEM_COPYBUFFER, (WPARAM)(Buffer), (LPARAM)(nMax))

#define HexEdit_GetBufferSize(hWnd) \
  SendMessage((hWnd), HEM_COPYBUFFER, 0, 0)

#define HexEdit_SetMaxBufferSize(hWnd, Size) \
  SendMessage((hWnd), HEM_SETMAXBUFFERSIZE, 0, (LPARAM)(Size))

#define HexEdit_SetReadOnly(hWnd, ReadOnly) \
  SendMessage((hWnd), HEM_SETREADONLY, 0, (LPARAM)(ReadOnly))

#define HexEdit_GetMulSelPtr(hWnd) \
	SendMessage((hWnd), HEM_GETMULSELLIST, 0, 0)

#define HexEdit_GetSelSize(hWnd) \
	SendMessage((hWnd), HEM_GETSELSIZE, 0, 0)

#define HexEdit_GetBufferPtr(hWnd) \
	SendMessage((hWnd), HEM_GETBUFFERPTR, 0, 0)

//#define HexEdit_SetMultSel(hWnd, MultSelStart, MultSelEnd)\
//	SendMessage((hWnd), HEM_ADDMULTSEL, (WPARAM)(MultSelStart), (LPARAM)(MultSelEnd))
//==============hex edit control menu id=============
#define ID_HEM_CUT                  1300
#define ID_HEM_COPY                 1301
#define ID_HEM_COPYASCII            1306
#define ID_HEM_PASTE                1302
#define ID_HEM_DELETE               1303
#define ID_HEM_SELECT	            1304
#define ID_HEM_SELECTALL            1305
//#define IDR_HEXEDIT                     163
//
//#define IDC_HEXVIEW						5000

typedef struct MULT_SEL
{
	INT SelStart;
	INT SelEnd;
	INT Property;
	MULT_SEL *pNextNode;
}MULT_SEL, *PMULT_SEL;