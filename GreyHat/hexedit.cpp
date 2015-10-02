/*
 * Hex editor control
 *
 * Copyright (C) 2004 Thomas Weidenmueller <w3seek@reactos.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "stdafx.h"
#include "hexedit.h"
#include <stdlib.h> 
#include <string>
#include "Utility.h"

typedef struct
{
    HWND hWndSelf;
    HWND hWndParent;
	HMENU hMenu;
	BOOL bHitSelect;
	PMULT_SEL pCurrentSelector;//当前鼠标点击的已选取的段
    HLOCAL hBuffer;
    DWORD style;
    DWORD MaxBuffer;
    INT ColumnsPerLine;
    INT nLines;
    INT nVisibleLinesComplete;
    INT nVisibleLines;
    INT Index;
    INT LineHeight;
    INT CharWidth;
    HFONT hFont;
    BOOL SbVisible;
	BOOL bReadOnly;

    INT LeftMargin;
    INT AddressSpacing;
    INT SplitSpacing;

    BOOL EditingField;
    INT CaretCol;
    INT CaretLine;
    BOOL InMid;

    INT SelStart;
    INT SelEnd;
	PMULT_SEL pMultSelListPtr;
} HEXEDIT_DATA, *PHEXEDIT_DATA;

static const TCHAR ClipboardFormatName[] = TEXT("RegEdit_HexData");
static UINT ClipboardFormatID = CF_TEXT;

static HMENU	  hPopupMenu;
static HINSTANCE hInst;

//#define PM_MODIFYVALUE  0
//#define PM_NEW          1
//#define PM_TREECONTEXT  2
#define PM_HEXEDIT      0


/* hit test codes */
#define HEHT_LEFTMARGIN	    (0x1)
#define HEHT_ADDRESS	    (0x2)
#define HEHT_ADDRESSSPACING	(0x3)
#define HEHT_HEXDUMP	    (0x4)
#define HEHT_HEXDUMPSPACING	(0x5)
#define HEHT_ASCIIDUMP	    (0x6)
#define HEHT_RIGHTMARGIN	(0x7)

INT_PTR CALLBACK HexEditWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

ATOM
WINAPI
RegisterHexEditorClass(HINSTANCE hInstance)
{
    WNDCLASSEX WndClass;

    //ClipboardFormatID = RegisterClipboardFormat(ClipboardFormatName);
	hInst = hInstance;

    ZeroMemory(&WndClass, sizeof(WNDCLASSEX));
    WndClass.cbSize = sizeof(WNDCLASSEX);
    WndClass.style = CS_DBLCLKS;
    WndClass.lpfnWndProc = (WNDPROC)HexEditWndProc;
    WndClass.cbWndExtra = sizeof(PHEXEDIT_DATA);
    WndClass.hInstance = hInstance;
    WndClass.hCursor = LoadCursor(0, IDC_IBEAM);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);//            (HBRUSH)(COLOR_WINDOW + 1);
    WndClass.lpszClassName = HEX_EDIT_CLASS_NAME;

    return RegisterClassEx(&WndClass);
}

BOOL
WINAPI
UnregisterHexEditorClass(HINSTANCE hInstance)
{
    return UnregisterClass(HEX_EDIT_CLASS_NAME, hInstance);
}

/*** Helper functions *********************************************************/

static VOID
HEXEDIT_MoveCaret(PHEXEDIT_DATA hed, BOOL Scroll)
{
    SCROLLINFO si;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS;
    GetScrollInfo(hed->hWndSelf, SB_VERT, &si);

    if(Scroll)
    {
        if(si.nPos > hed->CaretLine)
        {
            si.nPos = hed->CaretLine;
            SetScrollInfo(hed->hWndSelf, SB_VERT, &si, TRUE);
            GetScrollInfo(hed->hWndSelf, SB_VERT, &si);
            InvalidateRect(hed->hWndSelf, NULL, TRUE);
        }
        else if(hed->CaretLine >= (hed->nVisibleLinesComplete + si.nPos))
        {
            si.nPos = hed->CaretLine - hed->nVisibleLinesComplete + 1;
            SetScrollInfo(hed->hWndSelf, SB_VERT, &si, TRUE);
            GetScrollInfo(hed->hWndSelf, SB_VERT, &si);
            InvalidateRect(hed->hWndSelf, NULL, TRUE);
        }
    }

    if(hed->EditingField)
        SetCaretPos(hed->LeftMargin + ((4 + hed->AddressSpacing + (3 * hed->CaretCol) + hed->InMid * 2) * hed->CharWidth) - 1, (hed->CaretLine - si.nPos) * hed->LineHeight);
    else
        SetCaretPos(hed->LeftMargin + ((4 + hed->AddressSpacing + hed->SplitSpacing + (3 * hed->ColumnsPerLine) + hed->CaretCol) * hed->CharWidth) - 2, (hed->CaretLine - si.nPos) * hed->LineHeight);
}

static VOID
HEXEDIT_Update(PHEXEDIT_DATA hed)
{
    SCROLLINFO si;
    RECT rcClient;
    BOOL SbVisible;
    INT bufsize, cvislines;

    GetClientRect(hed->hWndSelf, &rcClient);
    hed->style = GetWindowLongPtr(hed->hWndSelf, GWL_STYLE);

    bufsize = (hed->hBuffer ? (INT) LocalSize(hed->hBuffer) : 0);
    hed->nLines = max(bufsize / hed->ColumnsPerLine, 1);
    if(bufsize > hed->ColumnsPerLine && (bufsize % hed->ColumnsPerLine) > 0)
    {
        hed->nLines++;
    }

    if(hed->LineHeight > 0)
    {
        hed->nVisibleLinesComplete = cvislines = rcClient.bottom / hed->LineHeight;
        hed->nVisibleLines = hed->nVisibleLinesComplete;
        if(rcClient.bottom % hed->LineHeight)
        {
            hed->nVisibleLines++;
        }
    }
    else
    {
        hed->nVisibleLines = cvislines = 0;
    }

    SbVisible = bufsize > 0 && cvislines < hed->nLines;
    ShowScrollBar(hed->hWndSelf, SB_VERT, SbVisible);

    /* update scrollbar */
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = ((bufsize > 0) ? hed->nLines - 1 : 0);
    si.nPage = ((hed->LineHeight > 0) ? rcClient.bottom / hed->LineHeight : 0);
    SetScrollInfo(hed->hWndSelf, SB_VERT, &si, TRUE);

    //if(IsWindowVisible(hed->hWndSelf) && SbVisible != hed->SbVisible)
    //{
    //    InvalidateRect(hed->hWndSelf, NULL, TRUE);
    //}
    InvalidateRect(hed->hWndSelf, NULL, TRUE);
    hed->SbVisible = SbVisible;
}

static HFONT
HEXEDIT_GetFixedFont(VOID)
{
    LOGFONT lf;
    GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT), &lf);
    return CreateFontIndirect(&lf);
}

PMULT_SEL GetSelectorByI(PHEXEDIT_DATA hed, UINT i)
{
	INT i0, i1;

	PMULT_SEL pNode, pNext;

	if ( NULL == hed->pMultSelListPtr )
	{
		return NULL;
	}
	else
	{
		pNext = hed->pMultSelListPtr;


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
			if (i0<=i && i<i1)
			{
				return pNext;
			}
			pNext = pNext->pNextNode;
		}while (NULL != pNext);
		return NULL;
	}
}

static VOID
HEXEDIT_PaintLines(PHEXEDIT_DATA hed, HDC hDC, DWORD ScrollPos, DWORD First, DWORD Last, RECT *rc)
{
    DWORD dx, dy, linestart;
    INT i, isave, i0, i1, x;
    PBYTE buf, current, end, line;
    size_t bufsize;
    wchar_t hex[3], addr[17];
    RECT rct, rctx;

    FillRect(hDC, rc, (HBRUSH)(COLOR_WINDOW + 1));
    SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));

    if (hed->SelStart < hed->SelEnd)
    {
        i0 = hed->SelStart;
        i1 = hed->SelEnd;
    }
    else
    {
        i0 = hed->SelEnd;
        i1 = hed->SelStart;
    }

    if(hed->hBuffer)
    {
        bufsize = LocalSize(hed->hBuffer);
        buf = (PBYTE)LocalLock(hed->hBuffer);
    }
    else
    {
        buf = NULL;
        bufsize = 0;

        if(ScrollPos + First == 0)
        {
            /* draw address */
            //wsprintf(addr, TEXT("%04X"), 0);
            //TextOut(hDC, hed->LeftMargin, First * hed->LineHeight, addr, 4);
        }
    }

    if(buf)
    {
        end = buf + bufsize;
        dy = First * hed->LineHeight;
        linestart = (ScrollPos + First) * hed->ColumnsPerLine;
        i = linestart;
        current = buf + linestart;
        Last = min(hed->nLines - ScrollPos, Last);

        SetBkMode(hDC, TRANSPARENT);
        while(First <= Last && current < end)
        {
            DWORD dh;

            dx = hed->LeftMargin;

            /* draw address */
            wsprintf(addr, TEXT("%04lX"), linestart);
            TextOut(hDC, dx, dy, addr, 4);

            dx += ((4 + hed->AddressSpacing) * hed->CharWidth);
            dh = (3 * hed->CharWidth);

            rct.left = dx;
            rct.top = dy;
            rct.right = rct.left + dh;
            rct.bottom = dy + hed->LineHeight;

            /* draw hex map */
            dx += (hed->CharWidth / 2);
            line = current;
            isave = i;
            for(x = 0; x < hed->ColumnsPerLine && current < end; x++)
            {
                rct.left += dh;
                rct.right += dh;

				//judge the byte is in mult select
				
                wsprintf(hex, TEXT("%02X"), *(current++));
				if (NULL != GetSelectorByI(hed, i))
				{
					rctx.left = dx;
					rctx.top = dy;
					rctx.right = dx + hed->CharWidth * 2 + 1;
					rctx.bottom = dy + hed->LineHeight;
					InflateRect(&rctx, hed->CharWidth / 2, 0);
					HBRUSH hBrush = CreateSolidBrush(RGB(255,0,0));
					FillRect(hDC, &rctx, hBrush);
					SetTextColor(hDC, RGB(0,0,0));
					ExtTextOut(hDC, dx, dy, 0, &rct, hex, 2, NULL);
					SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
					DeleteObject(hBrush);
				}
                else if (i0 <= i && i < i1)
                {
                    rctx.left = dx;
                    rctx.top = dy;
                    rctx.right = dx + hed->CharWidth * 2 + 1;
                    rctx.bottom = dy + hed->LineHeight;
                    InflateRect(&rctx, hed->CharWidth / 2, 0);
                    FillRect(hDC, &rctx, (HBRUSH)(COLOR_HIGHLIGHT + 1));
                    SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                    ExtTextOut(hDC, dx, dy, 0, &rct, hex, 2, NULL);
                    SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
                }
                else
                    ExtTextOut(hDC, dx, dy, ETO_OPAQUE, &rct, hex, 2, NULL);
                dx += dh;
                i++;
            }

            /* draw ascii map */
            dx = ((4 + hed->AddressSpacing + hed->SplitSpacing + (hed->ColumnsPerLine * 3)) * hed->CharWidth);
            current = line;
            i = isave;
            for(x = 0; x < hed->ColumnsPerLine && current < end; x++)
            {
                wsprintf(hex, (L"%C"), *(current++));
                hex[0] = ((hex[0] & (L'\x007f')) >= (L' ') ? hex[0] : (L'.'));
                if (i0 <= i && i < i1)
                {
                    rctx.left = dx;
                    rctx.top = dy;
                    rctx.right = dx + hed->CharWidth;
                    rctx.bottom = dy + hed->LineHeight;
                    FillRect(hDC, &rctx, (HBRUSH)(COLOR_HIGHLIGHT + 1));
                    SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                    TextOut(hDC, dx, dy, hex, 1);
                    SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
                }
                else
                    TextOut(hDC, dx, dy, hex, 1);
                dx += hed->CharWidth;
                i++;
            }

            dy += hed->LineHeight;
            linestart += hed->ColumnsPerLine;
            First++;
        }
    }

    LocalUnlock(hed->hBuffer);
}

static DWORD
HEXEDIT_HitRegionTest(PHEXEDIT_DATA hed, POINTS pt)
{
    int d;

    if(pt.x <= hed->LeftMargin)
    {
        return HEHT_LEFTMARGIN;
    }

    pt.x -= hed->LeftMargin;
    d = (4 * hed->CharWidth);
    if(pt.x <= d)
    {
        return HEHT_ADDRESS;
    }

    pt.x -= d;
    d = (hed->AddressSpacing * hed->CharWidth);
    if(pt.x <= d)
    {
        return HEHT_ADDRESSSPACING;
    }

    pt.x -= d;
    d = ((3 * hed->ColumnsPerLine + 1) * hed->CharWidth);
    if(pt.x <= d)
    {
        return HEHT_HEXDUMP;
    }

    pt.x -= d;
    d = ((hed->SplitSpacing - 1) * hed->CharWidth);
    if(pt.x <= d)
    {
        return HEHT_HEXDUMPSPACING;
    }

    pt.x -= d;
    d = (hed->ColumnsPerLine * hed->CharWidth);
    if(pt.x <= d)
    {
        return HEHT_ASCIIDUMP;
    }

    return HEHT_RIGHTMARGIN;
}

static DWORD
HEXEDIT_IndexFromPoint(PHEXEDIT_DATA hed, POINTS pt, DWORD Hit, POINT *EditPos, BOOL *EditField, BOOL bAccurate)
{
    SCROLLINFO si;
    DWORD Index, bufsize;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS;
    GetScrollInfo(hed->hWndSelf, SB_VERT, &si);

    EditPos->x = 0;

    if(hed->LineHeight > 0)
    {
        EditPos->y = min(si.nPos + (pt.y / hed->LineHeight), hed->nLines - 1);
    }
    else
    {
        EditPos->y = si.nPos;
    }

    switch(Hit)
    {
    case HEHT_LEFTMARGIN:
    case HEHT_ADDRESS:
    case HEHT_ADDRESSSPACING:
    case HEHT_HEXDUMP:
        pt.x -= (SHORT) hed->LeftMargin + ((4 + hed->AddressSpacing) * hed->CharWidth);
        *EditField = TRUE;
        break;

    default:
        pt.x -= hed->LeftMargin + ((4 + hed->AddressSpacing + hed->SplitSpacing + (3 * hed->ColumnsPerLine)) * hed->CharWidth);
        *EditField = FALSE;
        break;
    }

    if(pt.x > 0)
    {
        INT BlockWidth = (*EditField ? hed->CharWidth * 3 : hed->CharWidth);
		EditPos->x = min(hed->ColumnsPerLine, (pt.x + (bAccurate ? 0:BlockWidth/2) ) / BlockWidth);//=========fixed bug by daniel @20140621
    }

    bufsize = (hed->hBuffer ? (DWORD) LocalSize(hed->hBuffer) : 0);
    Index = (EditPos->y * hed->ColumnsPerLine) + EditPos->x;
    if(Index > bufsize)
    {
        INT tmp = bufsize % hed->ColumnsPerLine;
        Index = bufsize;
        if (bufsize==0)
        {
            EditPos->x = 0;
        }
        else
            EditPos->x = (tmp == 0 ? hed->ColumnsPerLine : tmp);
    }
    return Index;
}

static VOID
HEXEDIT_Copy(PHEXEDIT_DATA hed)
{
    PBYTE pb, buf;
    UINT cb;
    INT i0, i1;
    HGLOBAL hGlobal;

    if (hed->SelStart < hed->SelEnd)
    {
        i0 = hed->SelStart;
        i1 = hed->SelEnd;
    }
    else
    {
        i0 = hed->SelEnd;
        i1 = hed->SelStart;
    }

    cb = i1 - i0;
    if (cb == 0)
        return;

    hGlobal = GlobalAlloc(GHND | GMEM_SHARE, cb*3*sizeof(TCHAR) + sizeof(DWORD));//??陆??梅?陋hex??路没麓庐 add *3
    if (hGlobal == NULL)
        return;

    pb = (PBYTE)GlobalLock(hGlobal);
    if (pb)
    {
        //*(PDWORD)pb = cb;
        //pb += sizeof(DWORD);
        buf = (PBYTE) LocalLock(hed->hBuffer);
        if (buf)
        {
			std::wstring strHex = Utility::StringLib::Hex2Tstring(buf + i0, cb);//add
#ifdef _UNICODE
			int istrlen = strHex.length() + 1;
			char *pszStr = new char[istrlen]; 
			WideCharToMultiByte( CP_ACP, 0, strHex.c_str(), -1, pszStr, istrlen, NULL,NULL);
			CopyMemory(pb, pszStr, istrlen);//add
			delete []pszStr;
#else
			CopyMemory(pb, strHex.c_str(), strHex.length()+1);//add
#endif
			
            //CopyMemory(pb, buf + i0, cb);
            LocalUnlock(hed->hBuffer);
        }
        GlobalUnlock(hGlobal);

        if (OpenClipboard(hed->hWndSelf))
        {
            EmptyClipboard();
            SetClipboardData(ClipboardFormatID, hGlobal);
            CloseClipboard();
        }
    }
    else
        GlobalFree(hGlobal);
}


static VOID
HEXEDIT_CopyAscii(PHEXEDIT_DATA hed)
{
    PBYTE pb, buf;
    UINT cb;
    INT i0, i1;
    HGLOBAL hGlobal;

    if (hed->SelStart < hed->SelEnd)
    {
        i0 = hed->SelStart;
        i1 = hed->SelEnd;
    }
    else
    {
        i0 = hed->SelEnd;
        i1 = hed->SelStart;
    }

    cb = i1 - i0;
    if (cb == 0)
        return;

    hGlobal = GlobalAlloc(GHND | GMEM_SHARE, cb*3*sizeof(TCHAR) + sizeof(DWORD));//??陆??梅?陋hex??路没麓庐 add *3
    if (hGlobal == NULL)
        return;

    pb = (PBYTE)GlobalLock(hGlobal);
    if (pb)
    {
        //*(PDWORD)pb = cb;
        //pb += sizeof(DWORD);
        buf = (PBYTE) LocalLock(hed->hBuffer);
        if (buf)
        {
			string strAscii;
			CHAR szChar[2];
			BYTE bHex;
			for (int i = 0; i < cb; i++)
			{
				bHex = *(buf+i0+i);
				if ('\x20'<=bHex && bHex < '\x7f') {
					wsprintfA(szChar, "%C", bHex);
				}else{
					szChar[0] = '.';szChar[1] = '\0';
				}
				strAscii += string(szChar);
			}
			
			CopyMemory(pb, strAscii.c_str(), strAscii.length()+1);//add
			
            //CopyMemory(pb, buf + i0, cb);
            LocalUnlock(hed->hBuffer);
        }
        GlobalUnlock(hGlobal);

        if (OpenClipboard(hed->hWndSelf))
        {
            EmptyClipboard();
            SetClipboardData(ClipboardFormatID, hGlobal);
            CloseClipboard();
        }
    }
    else
        GlobalFree(hGlobal);
}

static VOID
HEXEDIT_Delete(PHEXEDIT_DATA hed)
{
    PBYTE buf;
    INT i0, i1;
    UINT bufsize;

    if (hed->SelStart < hed->SelEnd)
    {
        i0 = hed->SelStart;
        i1 = hed->SelEnd;
    }
    else
    {
        i0 = hed->SelEnd;
        i1 = hed->SelStart;
    }

    if (i0 != i1)
    {
        bufsize = (hed->hBuffer ? LocalSize(hed->hBuffer) : 0);
        buf = (PBYTE) LocalLock(hed->hBuffer);
        if (buf)
        {
            MoveMemory(buf + i0, buf + i1, bufsize - i1);
            LocalUnlock(hed->hBuffer);
        }
        HexEdit_SetMaxBufferSize(hed->hWndSelf, bufsize - (i1 - i0));
        hed->InMid = FALSE;
        hed->Index = hed->SelStart = hed->SelEnd = i0;
        hed->CaretCol = hed->Index % hed->ColumnsPerLine;
        hed->CaretLine = hed->Index / hed->ColumnsPerLine;
        InvalidateRect(hed->hWndSelf, NULL, TRUE);
        HEXEDIT_MoveCaret(hed, TRUE);
    }
}

static VOID
HEXEDIT_Paste(PHEXEDIT_DATA hed)
{
    HGLOBAL hGlobal;
    UINT bufsize;
    PBYTE pb, buf, tmp;
    DWORD cb;

	if (hed->bReadOnly)	return;

    HEXEDIT_Delete(hed);
    bufsize = (hed->hBuffer ? LocalSize(hed->hBuffer) : 0);

    if (OpenClipboard(hed->hWndSelf))
    {
        hGlobal = GetClipboardData(ClipboardFormatID);
        if (hGlobal != NULL)
        {
            pb = (PBYTE) GlobalLock(hGlobal);
            //cb = *(PDWORD) pb;
            //pb += sizeof(DWORD);
			std::string strClip = (char*)pb;
			int iStrLen = strClip.length();
			tmp = new BYTE[iStrLen];
#ifdef _UNICODE
			wchar_t* pwszClip = new wchar_t[iStrLen+1];
			MultiByteToWideChar( CP_ACP, 0, (LPCSTR)pb, -1, pwszClip, iStrLen+1);
			cb = Utility::StringLib::Tstring2Hex(pwszClip, tmp);//add
			delete[]pwszClip;
#else
			cb = Utility::StringLib::Tstring2Hex((char*)pb, tmp);//add
#endif
			
            HexEdit_SetMaxBufferSize(hed->hWndSelf, bufsize + cb);
            buf = (PBYTE) LocalLock(hed->hBuffer);
            if (buf)
            {
                MoveMemory(buf + hed->Index + cb, buf + hed->Index,
                           bufsize - hed->Index);
                CopyMemory(buf + hed->Index, tmp, cb);
                LocalUnlock(hed->hBuffer);
            }
			delete[]tmp;//add
            GlobalUnlock(hGlobal);
        }
        CloseClipboard();
    }
    InvalidateRect(hed->hWndSelf, NULL, TRUE);
    HEXEDIT_MoveCaret(hed, TRUE);
}

static VOID
HEXEDIT_Cut(PHEXEDIT_DATA hed)
{
	if (hed->bReadOnly)	return;
    HEXEDIT_Copy(hed);
    HEXEDIT_Delete(hed);
}

static VOID
HEXEDIT_Select(PHEXEDIT_DATA hed)
{
	if (hed->bHitSelect)//已经选择过的内容
	{
		PMULT_SEL pNode, pNext;

		if (hed->pCurrentSelector == hed->pMultSelListPtr)
		{
			delete hed->pMultSelListPtr;
			hed->pMultSelListPtr = hed->pCurrentSelector->pNextNode;
		}
		else
		{
			pNext = hed->pMultSelListPtr;
			do{
				if (hed->pCurrentSelector == pNext->pNextNode)
				{
					pNode = pNext->pNextNode;
					pNext->pNextNode = pNext->pNextNode->pNextNode;
					delete pNode;
				}else
					pNext = pNext->pNextNode;
			}while ( pNext != NULL );
		}
	}
	else if (hed->SelStart != hed->SelEnd)
	{
		PMULT_SEL pNode, pNext;
		pNode = new MULT_SEL;
		pNode->SelStart = hed->SelStart;
		pNode->SelEnd = hed->SelEnd;
		pNode->pNextNode = NULL;
		pNode->Property = 0;

		if ( NULL == hed->pMultSelListPtr )
		{
			hed->pMultSelListPtr = pNode;
		}else
		{
			pNext = hed->pMultSelListPtr;

			while (NULL != pNext->pNextNode)
			{
				pNext = pNext->pNextNode;
			}
			pNext->pNextNode = pNode;
		}
	}

	InvalidateRect(hed->hWndSelf, NULL, TRUE);
}

static VOID
HEXEDIT_SelectAll(PHEXEDIT_DATA hed)
{
    INT bufsize;

    bufsize = (hed->hBuffer ? (INT) LocalSize(hed->hBuffer) : 0);
    hed->Index = hed->SelStart = 0;
    hed->SelEnd = bufsize;
    InvalidateRect(hed->hWndSelf, NULL, TRUE);
    HEXEDIT_MoveCaret(hed, TRUE);
}

//static LRESULT
//HEXEDIT_HEM_GETSELPOS(PHEXEDIT_DATA hed)
//{
//	if (hed->SelStart < hed->SelEnd)
//		return hed->SelStart;
//	else
//		return hed->SelEnd;
//}

static INT_PTR
HEXEDIT_HEM_GETMULSELLIST(PHEXEDIT_DATA hed)
{
	return (INT_PTR)hed->pMultSelListPtr;
}

static INT_PTR
HEXEDIT_HEM_GETBUFFERPTR(PHEXEDIT_DATA hed)
{
	LPVOID lpBuffer = NULL;
	lpBuffer = LocalLock(hed->hBuffer);
	LocalUnlock(hed->hBuffer);
	return (INT_PTR)lpBuffer;
}

/*** Control specific messages ************************************************/

static LRESULT
HEXEDIT_HEM_LOADBUFFER(PHEXEDIT_DATA hed, PVOID Buffer, DWORD Size)
{
    if(Buffer != NULL && Size > 0)
    {
        LPVOID buf;

        //if(hed->MaxBuffer > 0 && Size > hed->MaxBuffer)
        //{
        //    Size = hed->MaxBuffer;
        //}

        if(hed->hBuffer)
        {
            if(Size > 0)
            {
                if(LocalSize(hed->hBuffer) != Size)
                {
                    hed->hBuffer = LocalReAlloc(hed->hBuffer, Size, LMEM_MOVEABLE | LMEM_ZEROINIT);
                }
            }
            else
            {
                hed->hBuffer = LocalFree(hed->hBuffer);
                hed->Index = 0;
                HEXEDIT_Update(hed);

                return 0;
            }
        }
        else if(Size > 0)
        {
            hed->hBuffer = LocalAlloc(LHND, Size);
        }

        if(Size > 0)
        {
            buf = LocalLock(hed->hBuffer);
            if(buf)
            {
                memcpy(buf, Buffer, Size);
            }
            else
                Size = 0;
            LocalUnlock(hed->hBuffer);
        }

        hed->Index = 0;
        HEXEDIT_Update(hed);
        return Size;
    }
    else if(hed->hBuffer)
    {
        hed->Index = 0;
        hed->hBuffer = LocalFree(hed->hBuffer);
        HEXEDIT_Update(hed);
    }

    return 0;
}

static LRESULT
HEXEDIT_HEM_COPYBUFFER(PHEXEDIT_DATA hed, PVOID Buffer, DWORD Size)
{
    size_t nCpy;

    if(!hed->hBuffer)
    {
        return 0;
    }

    if(Buffer != NULL && Size > 0)
    {
        nCpy = min(Size, LocalSize(hed->hBuffer));
        if(nCpy > 0)
        {
            PVOID buf;

            buf = LocalLock(hed->hBuffer);
            if(buf)
            {
                memcpy(Buffer, buf, nCpy);
            }
            else
                nCpy = 0;
            LocalUnlock(hed->hBuffer);
        }
        return nCpy;
    }

    return (LRESULT)LocalSize(hed->hBuffer);
}

static LRESULT
HEXEDIT_HEM_SETMAXBUFFERSIZE(PHEXEDIT_DATA hed, DWORD nMaxSize)
{
    hed->MaxBuffer = nMaxSize;
    if (hed->MaxBuffer == 0)
    {
        hed->hBuffer = LocalFree(hed->hBuffer);
        return 0;
    }
    if (hed->hBuffer)
        hed->hBuffer = LocalReAlloc(hed->hBuffer, hed->MaxBuffer, LMEM_MOVEABLE);
    else
        hed->hBuffer = LocalAlloc(LMEM_MOVEABLE, hed->MaxBuffer);
    HEXEDIT_Update(hed);
    return 0;
}

static LRESULT
HEXEDIT_HEM_SETREADONLY(PHEXEDIT_DATA hed, BOOL ReadOnly)
{
	hed->bReadOnly = ReadOnly;
	return 0;
}

//static LRESULT
//HEXEDIT_HEM_SETMULTSEL(PHEXEDIT_DATA hed, INT MultSelStart, INT MultSelEnd)
//{
//	PMULT_SEL pNode, pNext;
//	pNode = new MULT_SEL;
//	pNode->SelStart = MultSelStart;
//	pNode->SelEnd = MultSelEnd;
//	pNode->pNextNode = NULL;
//	pNode->Property = 0;
//
//	if ( NULL == hed->pMultSelListPtr )
//	{
//		hed->pMultSelListPtr = pNode;
//	}else
//	{
//		pNext = hed->pMultSelListPtr;
//
//		while (NULL != pNext->pNextNode)
//		{
//			pNext = pNext->pNextNode;
//		}
//		pNext->pNextNode = pNode;
//	}
//
//
//	return 0;
//}
/*** Message Proc *************************************************************/

static LRESULT
HEXEDIT_WM_NCCREATE(HWND hWnd, CREATESTRUCT *cs)
{
    PHEXEDIT_DATA hed;

    if(!(hed = (PHEXEDIT_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HEXEDIT_DATA))))
    {
        return FALSE;
    }

    hed->hWndSelf = hWnd;
    hed->hWndParent = cs->hwndParent;
    hed->style = cs->style;

    hed->ColumnsPerLine = 16;
    hed->LeftMargin = 2;
    hed->AddressSpacing = 2;
    hed->SplitSpacing = 2;
    hed->EditingField = TRUE; /* in hexdump field */
	hed->CharWidth = 6;
	hed->LineHeight = 12;

	hed->pMultSelListPtr = NULL;
	LOGFONT lf = {0};
	lf.lfHeight = 0xfffffff5;
	lf.lfWeight = FW_REGULAR;
	lf.lfOutPrecision = OUT_STROKE_PRECIS;
	lf.lfClipPrecision = CLIP_STROKE_PRECIS;
	lf.lfPitchAndFamily = DRAFT_QUALITY;
	_tcscpy(lf.lfFaceName, TEXT("Consolas"));
	hed->hFont = CreateFontIndirect (&lf);

    SetWindowLongPtr(hWnd, 0, (DWORD_PTR)hed);
    HEXEDIT_Update(hed);

    return TRUE;
}

static LRESULT
HEXEDIT_WM_NCDESTROY(PHEXEDIT_DATA hed)
{
    if(hed->hBuffer)
    {
        //while(LocalUnlock(hed->hBuffer));
        LocalFree(hed->hBuffer);
    }

    if(hed->hFont)
    {
        DeleteObject(hed->hFont);
    }

	if (hed->pMultSelListPtr)
	{
		PMULT_SEL pNext, pHeader;
		pHeader = hed->pMultSelListPtr;
		while (NULL != pHeader)
		{
			pNext = pHeader->pNextNode;
			delete pHeader;
			pHeader = pNext;
		}
	}

    SetWindowLongPtr(hed->hWndSelf, 0, (DWORD_PTR)0);
    HeapFree(GetProcessHeap(), 0, hed);
    //DestroyMenu(hPopupMenu);
    return 0;
}

static LRESULT
HEXEDIT_WM_CREATE(PHEXEDIT_DATA hed)
{
    UNREFERENCED_PARAMETER(hed);
	hed->hMenu = CreatePopupMenu();
	AppendMenu (hed->hMenu, MF_STRING, ID_HEM_SELECT, TEXT ("选择")) ; 
	AppendMenu (hed->hMenu, MF_STRING, ID_HEM_CUT, TEXT ("剪切")) ; 
	AppendMenu (hed->hMenu, MF_STRING, ID_HEM_COPY, TEXT ("拷贝"));
	AppendMenu (hed->hMenu, MF_STRING, ID_HEM_COPYASCII, TEXT ("拷贝ASCII"));
	AppendMenu (hed->hMenu, MF_STRING, ID_HEM_PASTE, TEXT ("粘帖")) ;
	AppendMenu (hed->hMenu, MF_STRING, ID_HEM_DELETE, TEXT ("删除")) ;
    return 1;
}

static LRESULT
HEXEDIT_WM_SETFOCUS(PHEXEDIT_DATA hed)
{
    CreateCaret(hed->hWndSelf, 0, 1, hed->LineHeight);
    HEXEDIT_MoveCaret(hed, FALSE);
    ShowCaret(hed->hWndSelf);
    return 0;
}

static LRESULT
HEXEDIT_WM_KILLFOCUS(PHEXEDIT_DATA hed)
{
    UNREFERENCED_PARAMETER(hed);
    DestroyCaret();
    return 0;
}

static LRESULT
HEXEDIT_WM_VSCROLL(PHEXEDIT_DATA hed, WORD ThumbPosition, WORD SbCmd)
{
    int ScrollY;
    SCROLLINFO si;

    UNREFERENCED_PARAMETER(ThumbPosition);

    ZeroMemory(&si, sizeof(SCROLLINFO));
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    GetScrollInfo(hed->hWndSelf, SB_VERT, &si);

    ScrollY = si.nPos;
    switch(SbCmd)
    {
    case SB_TOP:
        si.nPos = si.nMin;
        break;

    case SB_BOTTOM:
        si.nPos = si.nMax;
        break;

    case SB_LINEUP:
        si.nPos--;
        break;

    case SB_LINEDOWN:
        si.nPos++;
        break;

    case SB_PAGEUP:
        si.nPos -= si.nPage;
        break;

    case SB_PAGEDOWN:
        si.nPos += si.nPage;
        break;

    case SB_THUMBTRACK:
        si.nPos = si.nTrackPos;
        break;
    }

    si.fMask = SIF_POS;
    SetScrollInfo(hed->hWndSelf, SB_VERT, &si, TRUE);
    GetScrollInfo(hed->hWndSelf, SB_VERT, &si);

    if(si.nPos != ScrollY)
    {
        ScrollWindow(hed->hWndSelf, 0, (ScrollY - si.nPos) * hed->LineHeight, NULL, NULL);
        UpdateWindow(hed->hWndSelf);
    }

    return 0;
}

static LRESULT
HEXEDIT_WM_SETFONT(PHEXEDIT_DATA hed, HFONT hFont, BOOL bRedraw)
{
    HDC hDC;
    TEXTMETRIC tm;
    HFONT hOldFont = 0;

    if(hFont == 0)
    {
        hFont = HEXEDIT_GetFixedFont();
    }

    hed->hFont = hFont;
    hDC = GetDC(hed->hWndSelf);
    if(hFont)
    {
        hOldFont = (HFONT)SelectObject(hDC, hFont);
    }
    GetTextMetrics(hDC, &tm);
    hed->LineHeight = tm.tmHeight;
    hed->CharWidth = tm.tmAveCharWidth;
    if(hOldFont)
    {
        SelectObject(hDC, hOldFont);
    }
    ReleaseDC(hed->hWndSelf, hDC);

    if(bRedraw)
    {
        InvalidateRect(hed->hWndSelf, NULL, TRUE);
    }

    return 0;
}

static LRESULT
HEXEDIT_WM_GETFONT(PHEXEDIT_DATA hed)
{
    return (LRESULT)hed->hFont;
}

static LRESULT
HEXEDIT_WM_PAINT(PHEXEDIT_DATA hed)
{
    PAINTSTRUCT ps;
    SCROLLINFO si;
    RECT rc;
    HBITMAP hbmp, hbmpold;
    INT nLines, nFirst;
    HFONT hOldFont;
    HDC hTempDC;
    DWORD height;

    if(GetUpdateRect(hed->hWndSelf, &rc, FALSE) && (hed->LineHeight > 0))
    {
        ZeroMemory(&si, sizeof(SCROLLINFO));
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_POS;
        GetScrollInfo(hed->hWndSelf, SB_VERT, &si);

        height = (rc.bottom - rc.top);
        nLines = height / hed->LineHeight;
        if((height % hed->LineHeight) > 0)
        {
            nLines++;
        }
        if(nLines > hed->nLines - si.nPos)
        {
            nLines = hed->nLines - si.nPos;
        }
        nFirst = rc.top / hed->LineHeight;

        BeginPaint(hed->hWndSelf, &ps);
        if(!(hTempDC = CreateCompatibleDC(ps.hdc)))
        {
            FillRect(ps.hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
            goto epaint;
        }
        if(!(hbmp = CreateCompatibleBitmap(ps.hdc, ps.rcPaint.right, ps.rcPaint.bottom)))
        {
            FillRect(ps.hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
            DeleteDC(hTempDC);
            goto epaint;
        }
        hbmpold = (HBITMAP)SelectObject(hTempDC, hbmp);
        hOldFont = (HFONT)SelectObject(hTempDC, hed->hFont);
        HEXEDIT_PaintLines(hed, hTempDC, si.nPos, nFirst, nFirst + nLines, &ps.rcPaint);
        BitBlt(ps.hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hTempDC, rc.left, rc.top, SRCCOPY);
        SelectObject(hTempDC, hOldFont);
        SelectObject(hTempDC, hbmpold);

        DeleteObject(hbmp);
        DeleteDC(hTempDC);

epaint:
        EndPaint(hed->hWndSelf, &ps);
    }

    return 0;
}

static LRESULT
HEXEDIT_WM_MOUSEWHEEL(PHEXEDIT_DATA hed, int cyMoveLines, WORD ButtonsDown, LPPOINTS MousePos)
{
    SCROLLINFO si;
    int ScrollY;

    UNREFERENCED_PARAMETER(ButtonsDown);
    UNREFERENCED_PARAMETER(MousePos);

    SetFocus(hed->hWndSelf);

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    GetScrollInfo(hed->hWndSelf, SB_VERT, &si);

    ScrollY = si.nPos;

    si.fMask = SIF_POS;
    si.nPos += cyMoveLines;
    SetScrollInfo(hed->hWndSelf, SB_VERT, &si, TRUE);

    GetScrollInfo(hed->hWndSelf, SB_VERT, &si);
    if(si.nPos != ScrollY)
    {
        ScrollWindow(hed->hWndSelf, 0, (ScrollY - si.nPos) * hed->LineHeight, NULL, NULL);
        UpdateWindow(hed->hWndSelf);
    }

    return 0;
}

static LRESULT
HEXEDIT_WM_GETDLGCODE(LPMSG Msg)
{
    UNREFERENCED_PARAMETER(Msg);
    return DLGC_WANTARROWS | DLGC_WANTCHARS;
}

static LRESULT
HEXEDIT_WM_LBUTTONDOWN(PHEXEDIT_DATA hed, INT Buttons, POINTS Pt)
{
    BOOL NewField;
    POINT EditPos;
    DWORD Hit;

    UNREFERENCED_PARAMETER(Buttons);
    SetFocus(hed->hWndSelf);

    if (GetAsyncKeyState(VK_SHIFT) < 0)
    {
        if (hed->EditingField)
            hed->Index = HEXEDIT_IndexFromPoint(hed, Pt, HEHT_HEXDUMP, &EditPos, &NewField, FALSE);
        else
            hed->Index = HEXEDIT_IndexFromPoint(hed, Pt, HEHT_ASCIIDUMP, &EditPos, &NewField, FALSE);
        hed->SelEnd = hed->Index;
    }
    else
    {
        Hit = HEXEDIT_HitRegionTest(hed, Pt);
        hed->Index = HEXEDIT_IndexFromPoint(hed, Pt, Hit, &EditPos, &NewField, FALSE);
        hed->SelStart = hed->SelEnd = hed->Index;
        hed->EditingField = NewField;
        SetCapture(hed->hWndSelf);
    }
    hed->CaretCol = EditPos.x;
    hed->CaretLine = EditPos.y;
    hed->InMid = FALSE;
    InvalidateRect(hed->hWndSelf, NULL, FALSE);
    HEXEDIT_MoveCaret(hed, TRUE);

    return 0;
}

static LRESULT
HEXEDIT_WM_LBUTTONUP(PHEXEDIT_DATA hed, INT Buttons, POINTS Pt)
{
    BOOL NewField;
    POINT EditPos;
    if (GetCapture() == hed->hWndSelf)
    {
        if (hed->EditingField)
            hed->Index = HEXEDIT_IndexFromPoint(hed, Pt, HEHT_HEXDUMP, &EditPos, &NewField, FALSE);
        else
            hed->Index = HEXEDIT_IndexFromPoint(hed, Pt, HEHT_ASCIIDUMP, &EditPos, &NewField, FALSE);
        hed->CaretCol = EditPos.x;
        hed->CaretLine = EditPos.y;
        hed->SelEnd = hed->Index;
        ReleaseCapture();
        InvalidateRect(hed->hWndSelf, NULL, FALSE);
        HEXEDIT_MoveCaret(hed, TRUE);
    }
    return 0;
}

static LRESULT
HEXEDIT_WM_LBUTTONDBLCLK(PHEXEDIT_DATA hed, INT Buttons, POINTS Pt)
{
	BOOL NewField;
	POINT EditPos;
	DWORD Hit;

	UNREFERENCED_PARAMETER(Buttons);
	SetFocus(hed->hWndSelf);


	Hit = HEXEDIT_HitRegionTest(hed, Pt);
	if (Hit == HEHT_HEXDUMP)
	{
		hed->Index = HEXEDIT_IndexFromPoint(hed, Pt, Hit, &EditPos, &NewField,TRUE);

		PostMessage(hed->hWndParent, HEM_LDBCLICK, (WPARAM)hed->Index, NULL);
	}

	return 0;
}


static LRESULT
HEXEDIT_WM_RBUTTONDOWN(PHEXEDIT_DATA hed, INT Buttons, POINTS Pt)
{
	BOOL NewField;
	POINT EditPos;
	DWORD Hit;

	UNREFERENCED_PARAMETER(Buttons);
	SetFocus(hed->hWndSelf);


	Hit = HEXEDIT_HitRegionTest(hed, Pt);
	hed->Index = HEXEDIT_IndexFromPoint(hed, Pt, Hit, &EditPos, &NewField,TRUE);

	INT i0, i1;
	if (hed->SelStart < hed->SelEnd)
	{
		i0 = hed->SelStart;
		i1 = hed->SelEnd;
	}
	else
	{
		i0 = hed->SelEnd;
		i1 = hed->SelStart;
	}
	if (i0<=hed->Index && hed->Index<i1)
	{
		return 0;
	}

	hed->SelStart = hed->SelEnd = hed->Index;
	hed->EditingField = NewField;

	hed->CaretCol = EditPos.x;
	hed->CaretLine = EditPos.y;
	hed->InMid = FALSE;
	InvalidateRect(hed->hWndSelf, NULL, FALSE);
	HEXEDIT_MoveCaret(hed, TRUE);

	return 0;
}

static LRESULT
HEXEDIT_WM_RBUTTONUP(PHEXEDIT_DATA hed, INT Buttons, POINTS Pt)
{
	BOOL NewField;
	POINT EditPos;
	DWORD Hit;

	UNREFERENCED_PARAMETER(Buttons);
	SetFocus(hed->hWndSelf);


	Hit = HEXEDIT_HitRegionTest(hed, Pt);
	hed->Index = HEXEDIT_IndexFromPoint(hed, Pt, Hit, &EditPos, &NewField, TRUE);
	hed->pCurrentSelector = GetSelectorByI(hed, hed->Index);
	if (NULL != hed->pCurrentSelector)
	{
		ModifyMenu(hed->hMenu, 0, MF_BYPOSITION|MF_STRING,ID_HEM_SELECT,_T("取消选择")); 
		hed->bHitSelect =TRUE;
	}else
	{
		ModifyMenu(hed->hMenu, 0, MF_BYPOSITION|MF_STRING,ID_HEM_SELECT,_T("选择")); 
		hed->bHitSelect =FALSE;
	}


	if (hed->SelStart == hed->SelEnd)
	{
		EnableMenuItem(hed->hMenu, ID_HEM_CUT, MF_GRAYED);
		EnableMenuItem(hed->hMenu, ID_HEM_COPY, MF_GRAYED);
		EnableMenuItem(hed->hMenu, ID_HEM_COPYASCII, MF_GRAYED);

		//if Clipboard have data, then enable paste menu
		if (OpenClipboard(hed->hWndSelf))
		{
			HGLOBAL hGlobal = GetClipboardData(ClipboardFormatID);
			if (hGlobal != NULL)
			{
				EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_ENABLED);
			}
			else
			{
				EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_GRAYED);
			}
			CloseClipboard();
		}
		else
		{
			EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_GRAYED);
		}
		EnableMenuItem(hed->hMenu, ID_HEM_DELETE, MF_GRAYED);
	}
	else
	{
		EnableMenuItem(hed->hMenu, ID_HEM_CUT, MF_ENABLED);
		EnableMenuItem(hed->hMenu, ID_HEM_COPY, MF_ENABLED);
		EnableMenuItem(hed->hMenu, ID_HEM_COPYASCII, MF_ENABLED);
		//if Clipboard have data, then enable paste menu
		if (OpenClipboard(hed->hWndSelf))
		{
			HGLOBAL hGlobal = GetClipboardData(ClipboardFormatID);
			if (hGlobal != NULL)
			{
				EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_ENABLED);
			}
			else
			{
				EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_GRAYED);
			}
			CloseClipboard();
		}
		else
		{
			EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_GRAYED);
		}
		EnableMenuItem(hed->hMenu, ID_HEM_DELETE, MF_ENABLED);
	}

	//SetForegroundWindow(hed->hWndSelf);
	POINT Pt0;
	GetCursorPos( &Pt0 );
	TrackPopupMenu(hed->hMenu, TPM_RIGHTBUTTON, Pt0.x, Pt0.y, 0, hed->hWndSelf, NULL);
	return 0;
}

static LRESULT
HEXEDIT_WM_MOUSEMOVE(PHEXEDIT_DATA hed, INT Buttons, POINTS Pt)
{
    BOOL NewField;
    POINT EditPos;
    if (GetCapture() == hed->hWndSelf)
    {
        if (hed->EditingField)
            hed->Index = HEXEDIT_IndexFromPoint(hed, Pt, HEHT_HEXDUMP, &EditPos, &NewField, FALSE);
        else
            hed->Index = HEXEDIT_IndexFromPoint(hed, Pt, HEHT_ASCIIDUMP, &EditPos, &NewField, FALSE);
        hed->CaretCol = EditPos.x;
        hed->CaretLine = EditPos.y;
        hed->SelEnd = hed->Index;
        InvalidateRect(hed->hWndSelf, NULL, FALSE);
        HEXEDIT_MoveCaret(hed, TRUE);
    }
    return 0;
}

static BOOL
HEXEDIT_WM_KEYDOWN(PHEXEDIT_DATA hed, INT VkCode)
{
    size_t bufsize;
    PBYTE buf;
    INT i0, i1;

    if(GetKeyState(VK_MENU) & 0x8000)
    {
        return FALSE;
    }

    bufsize = (hed->hBuffer ? LocalSize(hed->hBuffer) : 0);

    if (hed->SelStart < hed->SelEnd)
    {
        i0 = hed->SelStart;
        i1 = hed->SelEnd;
    }
    else
    {
        i0 = hed->SelEnd;
        i1 = hed->SelStart;
    }

    switch(VkCode)
    {
    case 'X':
        if (GetAsyncKeyState(VK_SHIFT) >= 0 &&
                GetAsyncKeyState(VK_CONTROL) < 0 && hed->SelStart != hed->SelEnd)
            HEXEDIT_Cut(hed);
        else
            return TRUE;
        break;

    case 'C':
        if (GetAsyncKeyState(VK_SHIFT) >= 0 &&
                GetAsyncKeyState(VK_CONTROL) < 0 && hed->SelStart != hed->SelEnd)
            HEXEDIT_Copy(hed);
        else
            return TRUE;
        break;

    case 'V':
        if (GetAsyncKeyState(VK_SHIFT) >= 0 && GetAsyncKeyState(VK_CONTROL) < 0)
            HEXEDIT_Paste(hed);
        else
            return TRUE;
        break;

    case 'A':
        if (GetAsyncKeyState(VK_SHIFT) >= 0 && GetAsyncKeyState(VK_CONTROL) < 0)
            HEXEDIT_SelectAll(hed);
        else
            return TRUE;
        break;

    case VK_INSERT:
        if (hed->SelStart != hed->SelEnd)
        {
            if (GetAsyncKeyState(VK_SHIFT) >= 0 && GetAsyncKeyState(VK_CONTROL) < 0)
                HEXEDIT_Copy(hed);
        }
        if (GetAsyncKeyState(VK_SHIFT) < 0 && GetAsyncKeyState(VK_CONTROL) >= 0)
            HEXEDIT_Paste(hed);
        break;

    case VK_DELETE:
		if (hed->bReadOnly)	return TRUE;
        if (GetAsyncKeyState(VK_SHIFT) < 0 && GetAsyncKeyState(VK_CONTROL) >= 0 &&
                hed->SelStart != hed->SelEnd)
            HEXEDIT_Copy(hed);
        if (i0 != i1)
        {
            buf = (PBYTE) LocalLock(hed->hBuffer);
            if (buf)
            {
                MoveMemory(buf + i0, buf + i1, bufsize - i1);
                LocalUnlock(hed->hBuffer);
            }
            HexEdit_SetMaxBufferSize(hed->hWndSelf, bufsize - (i1 - i0));
            hed->InMid = FALSE;
            hed->Index = hed->SelStart = hed->SelEnd = i0;
            hed->CaretCol = hed->Index % hed->ColumnsPerLine;
            hed->CaretLine = hed->Index / hed->ColumnsPerLine;
        }
        else
        {
            if (hed->InMid && hed->EditingField)
            {
                buf = (PBYTE) LocalLock(hed->hBuffer);
                if (buf)
                {
                    MoveMemory(buf + hed->Index, buf + hed->Index + 1,
                               bufsize - hed->Index - 1);
                    LocalUnlock(hed->hBuffer);
                }
                HexEdit_SetMaxBufferSize(hed->hWndSelf, bufsize - 1);
                hed->InMid = FALSE;
            }
            else if (hed->Index < bufsize)
            {
                buf = (PBYTE) LocalLock(hed->hBuffer);
                if (buf)
                {
                    MoveMemory(buf + hed->Index, buf + hed->Index + 1,
                               bufsize - hed->Index - 1);
                    LocalUnlock(hed->hBuffer);
                }
                HexEdit_SetMaxBufferSize(hed->hWndSelf, bufsize - 1);
            }
        }
        InvalidateRect(hed->hWndSelf, NULL, TRUE);
        HEXEDIT_MoveCaret(hed, TRUE);
        break;

    case VK_BACK:
		if (hed->bReadOnly)	return TRUE;
        if (i0 != i1)
        {
            buf = (PBYTE) LocalLock(hed->hBuffer);
            if (buf)
            {
                MoveMemory(buf + i0, buf + i1, bufsize - i1);
                LocalUnlock(hed->hBuffer);
            }
            HexEdit_SetMaxBufferSize(hed->hWndSelf, bufsize - (i1 - i0));
            hed->InMid = FALSE;
            hed->Index = hed->SelStart = hed->SelEnd = i0;
            hed->CaretCol = hed->Index % hed->ColumnsPerLine;
            hed->CaretLine = hed->Index / hed->ColumnsPerLine;
        }
        else
        {
            if (hed->InMid && hed->EditingField)
            {
                buf = (PBYTE) LocalLock(hed->hBuffer);
                if (buf)
                {
                    MoveMemory(buf + hed->Index, buf + hed->Index + 1,
                               bufsize - hed->Index - 1);
                    LocalUnlock(hed->hBuffer);
                }
            }
            else if (hed->Index > 0)
            {
                buf = (PBYTE) LocalLock(hed->hBuffer);
                if (buf)
                {
                    MoveMemory(buf + hed->Index - 1, buf + hed->Index,
                               bufsize - hed->Index);
                    LocalUnlock(hed->hBuffer);
                }
                hed->Index--;
                hed->SelStart = hed->SelEnd = hed->Index;
                hed->CaretCol = hed->Index % hed->ColumnsPerLine;
                hed->CaretLine = hed->Index / hed->ColumnsPerLine;
            }
            else
                return TRUE;
            HexEdit_SetMaxBufferSize(hed->hWndSelf, bufsize - 1);
            hed->InMid = FALSE;
        }
        InvalidateRect(hed->hWndSelf, NULL, TRUE);
        HEXEDIT_MoveCaret(hed, TRUE);
        break;

    case VK_LEFT:
        if (hed->Index > 0)
        {
            hed->Index--;
            if (GetAsyncKeyState(VK_SHIFT) < 0)
                hed->SelEnd = hed->Index;
            else
                hed->SelStart = hed->SelEnd = hed->Index;
            hed->CaretCol = hed->Index % hed->ColumnsPerLine;
            hed->CaretLine = hed->Index / hed->ColumnsPerLine;
            hed->InMid = FALSE;
            InvalidateRect(hed->hWndSelf, NULL, TRUE);
            HEXEDIT_MoveCaret(hed, TRUE);
        }
        break;

    case VK_RIGHT:
        if (hed->Index < (INT)bufsize)
        {
            hed->Index++;
            if (GetAsyncKeyState(VK_SHIFT) < 0)
                hed->SelEnd = hed->Index;
            else
                hed->SelStart = hed->SelEnd = hed->Index;
            hed->CaretCol = hed->Index % hed->ColumnsPerLine;
            hed->CaretLine = hed->Index / hed->ColumnsPerLine;
            hed->InMid = FALSE;
            InvalidateRect(hed->hWndSelf, NULL, TRUE);
            HEXEDIT_MoveCaret(hed, TRUE);
        }
        break;

    case VK_UP:
        if (hed->Index >= hed->ColumnsPerLine)
        {
            hed->Index -= hed->ColumnsPerLine;
            if (GetAsyncKeyState(VK_SHIFT) < 0)
                hed->SelEnd = hed->Index;
            else
                hed->SelStart = hed->SelEnd = hed->Index;
            hed->CaretCol = hed->Index % hed->ColumnsPerLine;
            hed->CaretLine = hed->Index / hed->ColumnsPerLine;
            hed->InMid = FALSE;
            InvalidateRect(hed->hWndSelf, NULL, TRUE);
            HEXEDIT_MoveCaret(hed, TRUE);
        }
        break;

    case VK_DOWN:
        if (hed->Index + hed->ColumnsPerLine <= (INT) bufsize)
            hed->Index += hed->ColumnsPerLine;
        else
            hed->Index = bufsize;
        hed->CaretCol = hed->Index % hed->ColumnsPerLine;
        hed->CaretLine = hed->Index / hed->ColumnsPerLine;
        if (GetAsyncKeyState(VK_SHIFT) < 0)
            hed->SelEnd = hed->Index;
        else
            hed->SelStart = hed->SelEnd = hed->Index;
        hed->InMid = FALSE;
        InvalidateRect(hed->hWndSelf, NULL, TRUE);
        HEXEDIT_MoveCaret(hed, TRUE);
        break;

    default:
        return TRUE;
    }

    return FALSE;
}

static BOOL
HEXEDIT_WM_CHAR(PHEXEDIT_DATA hed, WCHAR ch)
{
    size_t bufsize;
    PBYTE buf;
    INT i0, i1;

	if (hed->bReadOnly)	return TRUE;
    bufsize = (hed->hBuffer ? LocalSize(hed->hBuffer) : 0);
    if (hed->SelStart < hed->SelEnd)
    {
        i0 = hed->SelStart;
        i1 = hed->SelEnd;
    }
    else
    {
        i0 = hed->SelEnd;
        i1 = hed->SelStart;
    }
    if (!hed->EditingField)
    {
        if (0x20 <= ch && ch <= 0xFF)
        {
			//daniel add @2014 09 23 1:28
			if ( hed->Index == bufsize ){
				goto calcaret_ascii;
			}
			//add end

			//daniel comments @2014 09 23 1:45
            if (hed->SelStart != hed->SelEnd)
            {
            //    buf = (PBYTE) LocalLock(hed->hBuffer);
            //    if (buf)
            //    {
            //        MoveMemory(buf + i0, buf + i1, bufsize - i1);
            //        LocalUnlock(hed->hBuffer);
            //    }
            //    HexEdit_SetMaxBufferSize(hed->hWndSelf, bufsize - (i1 - i0));
            //    hed->InMid = FALSE;
            //    bufsize = (hed->hBuffer ? LocalSize(hed->hBuffer) : 0);
                hed->Index = hed->SelStart = hed->SelEnd = i0;
            }
           // HexEdit_SetMaxBufferSize(hed->hWndSelf, bufsize + 1);
            buf = (PBYTE) LocalLock(hed->hBuffer);
            if (buf)
            {
                //MoveMemory(buf + hed->Index + 1, buf + hed->Index,
                //           bufsize - hed->Index);
                buf[hed->Index] = ch;
                LocalUnlock(hed->hBuffer);
            }
            hed->Index++;
calcaret_ascii:
            hed->CaretCol = hed->Index % hed->ColumnsPerLine;
            hed->CaretLine = hed->Index / hed->ColumnsPerLine;
            InvalidateRect(hed->hWndSelf, NULL, TRUE);
            HEXEDIT_MoveCaret(hed, TRUE);
            return FALSE;
        }
    }
    else
    {
        if (('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'F') ||
                ('a' <= ch && ch <= 'f'))
        {
			//daniel add @2014 09 23 1:28
			if ( hed->Index == bufsize ){
				goto calcaret_hex;
			}
			//add end
			//daniel comments @2014 09 23 1:45
            if (hed->SelStart != hed->SelEnd)
            {
            //    buf = (PBYTE) LocalLock(hed->hBuffer);
            //    if (buf)
            //    {
            //        MoveMemory(buf + i0, buf + i1, bufsize - i1);
            //        LocalUnlock(hed->hBuffer);
            //    }
            //    HexEdit_SetMaxBufferSize(hed->hWndSelf, bufsize - (i1 - i0));
            //    hed->InMid = FALSE;
            //    bufsize = (hed->hBuffer ? LocalSize(hed->hBuffer) : 0);
                hed->Index = hed->SelStart = hed->SelEnd = i0;
            }

            if (hed->InMid)
            {
                buf = (PBYTE) LocalLock(hed->hBuffer);
                if (buf)
                {
                    //if ('0' <= ch && ch <= '9')
                    //    buf[hed->Index] |= ch - '0';
                    //else if ('A' <= ch && ch <= 'F')
                    //    buf[hed->Index] |= ch + 10 - 'A';
                    //else if ('a' <= ch && ch <= 'f')
                    //    buf[hed->Index] |= ch + 10 - 'a';

					//修改用于修改封包，而不增加内容
					if ('0' <= ch && ch <= '9')
						buf[hed->Index] = (buf[hed->Index] & 0xf0)|(ch - '0');
					else if ('A' <= ch && ch <= 'F')
						buf[hed->Index] = (buf[hed->Index] & 0xf0)|(ch + 10 - 'A');
					else if ('a' <= ch && ch <= 'f')
						buf[hed->Index] = (buf[hed->Index] & 0xf0)|(ch + 10 - 'a');

                    LocalUnlock(hed->hBuffer);
                }
                hed->InMid = FALSE;
				hed->Index++;
				//if (hed->Index < bufsize-1) {
				//	hed->Index++;
				//}
                
            }
            else
            {
                //HexEdit_SetMaxBufferSize(hed->hWndSelf, bufsize + 1);
                //buf = (PBYTE) LocalLock(hed->hBuffer);
                //if (buf)
                //{
                //    MoveMemory(buf + hed->Index + 1, buf + hed->Index,
                //               bufsize - hed->Index);
                //    if ('0' <= ch && ch <= '9')
                //        buf[hed->Index] = (ch - '0') << 4;
                //    else if ('A' <= ch && ch <= 'F')
                //        buf[hed->Index] = (ch + 10 - 'A') << 4;
                //    else if ('a' <= ch && ch <= 'f')
                //        buf[hed->Index] = (ch + 10 - 'a') << 4;
                //    LocalUnlock(hed->hBuffer);
                //}
                //hed->InMid = TRUE;

				//修改用于修改封包，而不增加内容

				buf = (PBYTE) LocalLock(hed->hBuffer);
				if (buf)
				{
				    if ('0' <= ch && ch <= '9')
				        buf[hed->Index] = (((ch - '0') << 4)&0xf0) | (buf[hed->Index]&0x0f);
				    else if ('A' <= ch && ch <= 'F')
				        buf[hed->Index] = (((ch + 10 - 'A') << 4)&0xf0) | (buf[hed->Index]&0x0f);
				    else if ('a' <= ch && ch <= 'f')
				        buf[hed->Index] = (((ch + 10 - 'a') << 4)&0xf0) | (buf[hed->Index]&0x0f);
				    LocalUnlock(hed->hBuffer);
				}
				hed->InMid = TRUE;
            }
calcaret_hex:
            hed->CaretCol = hed->Index % hed->ColumnsPerLine;
            hed->CaretLine = hed->Index / hed->ColumnsPerLine;
            InvalidateRect(hed->hWndSelf, NULL, TRUE);
            HEXEDIT_MoveCaret(hed, TRUE);
            return FALSE;
        }
    }
    return TRUE;
}

static LRESULT
HEXEDIT_WM_SIZE(PHEXEDIT_DATA hed, DWORD sType, WORD NewWidth, WORD NewHeight)
{
    UNREFERENCED_PARAMETER(sType);
    UNREFERENCED_PARAMETER(NewHeight);
    UNREFERENCED_PARAMETER(NewWidth);
    HEXEDIT_Update(hed);
    return 0;
}

static VOID
HEXEDIT_WM_CONTEXTMENU(PHEXEDIT_DATA hed, INT x, INT y)
{
    RECT rc;

    if (x == -1 && y == -1)
    {
        GetWindowRect(hed->hWndSelf, &rc);
        x = rc.left;
        y = rc.top;
    }
	
    if (hed->SelStart == hed->SelEnd)
    {
        EnableMenuItem(hed->hMenu, ID_HEM_CUT, MF_GRAYED);
        EnableMenuItem(hed->hMenu, ID_HEM_COPY, MF_GRAYED);
		EnableMenuItem(hed->hMenu, ID_HEM_COPYASCII, MF_GRAYED);

        //if Clipboard have data, then enable paste menu
        if (OpenClipboard(hed->hWndSelf))
        {
            HGLOBAL hGlobal = GetClipboardData(ClipboardFormatID);
            if (hGlobal != NULL)
            {
                EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_ENABLED);
            }
            else
            {
                EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_GRAYED);
            }
            CloseClipboard();
        }
        else
        {
            EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_GRAYED);
        }
        EnableMenuItem(hed->hMenu, ID_HEM_DELETE, MF_GRAYED);
    }
    else
    {
        EnableMenuItem(hed->hMenu, ID_HEM_CUT, MF_ENABLED);
        EnableMenuItem(hed->hMenu, ID_HEM_COPY, MF_ENABLED);
		EnableMenuItem(hed->hMenu, ID_HEM_COPYASCII, MF_ENABLED);
        //if Clipboard have data, then enable paste menu
        if (OpenClipboard(hed->hWndSelf))
        {
            HGLOBAL hGlobal = GetClipboardData(ClipboardFormatID);
            if (hGlobal != NULL)
            {
                EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_ENABLED);
            }
            else
            {
                EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_GRAYED);
            }
            CloseClipboard();
        }
        else
        {
            EnableMenuItem(hed->hMenu, ID_HEM_PASTE, MF_GRAYED);
        }
        EnableMenuItem(hed->hMenu, ID_HEM_DELETE, MF_ENABLED);
    }

    SetForegroundWindow(hed->hWndSelf);
    TrackPopupMenu(hed->hMenu, TPM_RIGHTBUTTON, x, y, 0, hed->hWndSelf, NULL);
    //PostMessage(hed->hWndSelf, WM_NULL, 0, 0);
	//DestroyMenu(hPopupMenu);
}

INT_PTR CALLBACK
HexEditWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PHEXEDIT_DATA hed;
    POINTS p;

    hed = (PHEXEDIT_DATA)(LONG_PTR)GetWindowLongPtr(hWnd, (DWORD_PTR)0);
    switch(uMsg)
    {
    case WM_ERASEBKGND:
        return TRUE;

    case WM_PAINT:
        return HEXEDIT_WM_PAINT(hed);

    case WM_KEYDOWN:
        return HEXEDIT_WM_KEYDOWN(hed, (INT)wParam);

    case WM_CHAR:
        return HEXEDIT_WM_CHAR(hed, (WCHAR)wParam);

    case WM_VSCROLL:
        return HEXEDIT_WM_VSCROLL(hed, HIWORD(wParam), LOWORD(wParam));

    case WM_SIZE:
        return HEXEDIT_WM_SIZE(hed, (DWORD)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_LBUTTONDOWN:
    {
        p.x = LOWORD(lParam);
        p.y = HIWORD(lParam);
        return HEXEDIT_WM_LBUTTONDOWN(hed, (INT)wParam, p);
    }

    case WM_LBUTTONUP:
    {
        p.x = LOWORD(lParam);
        p.y = HIWORD(lParam);
        return HEXEDIT_WM_LBUTTONUP(hed, (INT)wParam, p);
    }

	case WM_LBUTTONDBLCLK:
		{
			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);
			return HEXEDIT_WM_LBUTTONDBLCLK(hed, (INT)wParam, p);
		}

	case WM_RBUTTONDOWN:
	{
		p.x = LOWORD(lParam);
		p.y = HIWORD(lParam);
		return HEXEDIT_WM_RBUTTONDOWN(hed, (INT)wParam, p);
	}

	case WM_RBUTTONUP:
	{
		p.x = LOWORD(lParam);
		p.y = HIWORD(lParam);
		return HEXEDIT_WM_RBUTTONUP(hed, (INT)wParam, p);
	}

    case WM_MOUSEMOVE:
    {
        p.x = LOWORD(lParam);
        p.y = HIWORD(lParam);
        return HEXEDIT_WM_MOUSEMOVE(hed, (INT)wParam, p);
    }

    case WM_MOUSEWHEEL:
    {
        UINT nScrollLines = 3;
        int delta = 0;

        SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
        delta -= (SHORT)HIWORD(wParam);
        if(abs(delta) >= WHEEL_DELTA && nScrollLines != 0)
        {
            p.x = LOWORD(lParam);
            p.y = HIWORD(lParam);
            return HEXEDIT_WM_MOUSEWHEEL(hed, nScrollLines * (delta / WHEEL_DELTA), LOWORD(wParam), &p);
        }
        break;
    }
	case HEM_GETMULSELLIST:
		return HEXEDIT_HEM_GETMULSELLIST(hed);

	//case HEM_GETSELSIZE:
	//	return HEXEDIT_HEM_GETSELSIZE(hed);

	//case HEM_ADDMULTSEL:
	//	return HEXEDIT_HEM_SETMULTSEL(hed, (INT)wParam, (INT)lParam);

    case HEM_LOADBUFFER:
        return HEXEDIT_HEM_LOADBUFFER(hed, (PVOID)wParam, (DWORD)lParam);

    case HEM_COPYBUFFER:
        return HEXEDIT_HEM_COPYBUFFER(hed, (PVOID)wParam, (DWORD)lParam);

	case HEM_GETBUFFERPTR:
		return HEXEDIT_HEM_GETBUFFERPTR(hed);

    case HEM_SETMAXBUFFERSIZE:
        return HEXEDIT_HEM_SETMAXBUFFERSIZE(hed, (DWORD)lParam);

	case HEM_SETREADONLY:
		return HEXEDIT_HEM_SETREADONLY(hed, (DWORD)lParam);

    case WM_SETFOCUS:
        return HEXEDIT_WM_SETFOCUS(hed);

    case WM_KILLFOCUS:
        return HEXEDIT_WM_KILLFOCUS(hed);

    case WM_GETDLGCODE:
        return HEXEDIT_WM_GETDLGCODE((LPMSG)lParam);

    case WM_SETFONT:
        return HEXEDIT_WM_SETFONT(hed, (HFONT)wParam, (BOOL)LOWORD(lParam));

    case WM_GETFONT:
        return HEXEDIT_WM_GETFONT(hed);

    case WM_CREATE:
        return HEXEDIT_WM_CREATE(hed);

    case WM_NCCREATE:
        if(!hed)
        {
            return HEXEDIT_WM_NCCREATE(hWnd, (CREATESTRUCT*)lParam);
        }
        break;

    case WM_NCDESTROY:
        if(hed)
        {
            return HEXEDIT_WM_NCDESTROY(hed);
        }
        break;

    case WM_CONTEXTMENU:
        HEXEDIT_WM_CONTEXTMENU(hed, (short)LOWORD(lParam), (short)HIWORD(lParam));
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case ID_HEM_CUT:
            HEXEDIT_Cut(hed);
            break;

        case ID_HEM_COPY:
            HEXEDIT_Copy(hed);
            break;

		case ID_HEM_COPYASCII:
			HEXEDIT_CopyAscii(hed);
			break;

        case ID_HEM_PASTE:
            HEXEDIT_Paste(hed);
            break;

        case ID_HEM_DELETE:
            HEXEDIT_Delete(hed);
            break;

        case ID_HEM_SELECTALL:
            HEXEDIT_SelectAll(hed);
            break;

		case ID_HEM_SELECT:
			HEXEDIT_Select(hed);
			break;
        }
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

