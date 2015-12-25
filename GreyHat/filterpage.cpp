#include "stdafx.h"
#include "greyhat.h"
#include "ctrl.h"


CGPacketFilterSettingPage FilterSettingPage;

INT_PTR CALLBACK CGPacketFilterSettingPage::FilterPageMessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hWnd, WM_INITDIALOG, OnWmInit);
		HANDLE_MSG(hWnd, WM_COMMAND, OnWmCommand);
		HANDLE_MSG(hWnd, WM_NOTIFY, OnWmNotify);
	}
	return FALSE;
}

BOOL CGPacketFilterSettingPage::OnWmInit(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{

	FilterSettingPage.m_hMainPage = hwnd;
	FilterSettingPage.m_hFilterList = GetDlgItem(hwnd, IDC_FILTERLISTCTRL);
	FilterSettingPage.m_hKeyWord = GetDlgItem(hwnd, IDC_EKEYWORD);
	FilterSettingPage.m_hReplace = GetDlgItem(hwnd, IDC_EREPLACE);
	FilterSettingPage.m_hAdvFilter = GetDlgItem(hwnd, IDC_EADVFILTER);


	DWORD dwExStyle;
	dwExStyle = ListView_GetExtendedListViewStyle(FilterSettingPage.m_hFilterList);
	dwExStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(FilterSettingPage.m_hFilterList, dwExStyle);

	LvInsertColumn(FilterSettingPage.m_hFilterList, _T("#"), 30, 0);
	LvInsertColumn(FilterSettingPage.m_hFilterList, _T("¹ýÂË×Ö¶Î"), 200, 1);
	LvInsertColumn(FilterSettingPage.m_hFilterList, _T("Ìæ»»×Ö¶Î"), 200, 2);
	LvInsertColumn(FilterSettingPage.m_hFilterList, _T("¸ß¼¶¹ýÂË"), 200, 3);


	EnableWindow(GetDlgItem(hwnd, IDC_BDEL), FALSE);

	ReLoadAllFilters();

	return TRUE;
}
VOID CGPacketFilterSettingPage::OnWmCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDC_BADD: OnButtonAdd(); break;
	case IDC_BDEL: OnButtonDel(); break;
	case IDC_F_BCLEAR: OnButtonClear(); break;
	case IDOK:
	case IDCANCEL:
		ShowWindow(hWnd, SW_HIDE);
		break;
	}
}

VOID CGPacketFilterSettingPage::OnButtonClear()
{
	DWORD dwLvCount;
	CGPacketFilter* pFilter;

	dwLvCount = LvGetItemCount(FilterSettingPage.m_hFilterList);
	for (int i = 0; i < dwLvCount; i++) {
		pFilter = (CGPacketFilter*)LvGetData(FilterSettingPage.m_hFilterList, i);
		delete pFilter;
	}

	LvRemoveAll(FilterSettingPage.m_hFilterList);

	SaveAllFilters();
}

BOOL CGPacketFilterSettingPage::OnWmNotify(HWND hwnd, INT id, LPNMHDR pnm)
{
	int nSel = -1;
	Tstring tstrKeyWords, tstrReplaceData, tstrAdvFilter;

	switch (pnm->code)
	{
	case NM_CLICK:
	{
		nSel = LvGetSelItemId(FilterSettingPage.m_hFilterList);

		if (nSel != -1 /*&& id == IDC_LSTST*/)
		{
			CGPacketFilter* pFilter;
			pFilter = (CGPacketFilter*)LvGetData(FilterSettingPage.m_hFilterList, nSel);

			//SetText(FilterSettingPage.m_hKeyWord, pFilter->GetKeyWord().c_str());
			//SetText(FilterSettingPage.m_hReplace, pFilter->GetReplaceData().c_str());
			//SetText(FilterSettingPage.m_hAdvFilter, pFilter->GetAdvFilterStr().c_str());
			SetDlgItemText(hwnd, IDC_BADD, _T("ÐÞ¸Ä"));
			EnableWindow(GetDlgItem(hwnd, IDC_BDEL), TRUE);
		}
		else {

			SetText(FilterSettingPage.m_hKeyWord, _T(""));
			SetText(FilterSettingPage.m_hReplace, _T(""));
			SetText(FilterSettingPage.m_hAdvFilter, _T(""));
			SetDlgItemText(hwnd, IDC_BADD, _T("Ìí¼Ó"));
			EnableWindow(GetDlgItem(hwnd, IDC_BDEL), FALSE);
		}
	}
	break;
	}

	return TRUE;
}

CGPacketFilterSettingPage::CGPacketFilterSettingPage()
{

}

CGPacketFilterSettingPage::~CGPacketFilterSettingPage()
{

}


VOID CGPacketFilterSettingPage::OnButtonAdd()
{
	/*CGPacketFilter NewFilter;

	Tstring tstrKeyWord, tstrReplaceData, tstrAdvFilter;

	tstrKeyWord = GetText(FilterSettingPage.m_hKeyWord);
	tstrReplaceData = GetText(FilterSettingPage.m_hReplace);
	tstrAdvFilter = GetText(FilterSettingPage.m_hAdvFilter);

	if (tstrKeyWord != _T("") || tstrReplaceData != _T("") || tstrAdvFilter != _T(""))
	{
		DWORD dwInsertItem;
		TCHAR tcTemp[256];
		int nSel = -1;
		nSel = LvGetSelItemId(FilterSettingPage.m_hFilterList);
		
		if (nSel == -1)
		{
			dwInsertItem = LvGetItemCount(FilterSettingPage.m_hFilterList);
			_stprintf(tcTemp, _T("%d"), dwInsertItem);
			LvInsertItem(FilterSettingPage.m_hFilterList, tcTemp, dwInsertItem);
			pFilter = new CGPacketFilter;
			LvSetData(FilterSettingPage.m_hFilterList, dwInsertItem, pFilter);

		}
		else
		{
			dwInsertItem = nSel;
			pFilter = (CGPacketFilter*)LvGetData(FilterSettingPage.m_hFilterList, nSel);
			pFilter->Clear();
		}

		uint32_t keyword_len = 0, rpl_len = 0;
		byte *keyword_hexbyte = null, *rpl_hexbyte = null;


		if (tstrKeyWord != _T(""))
		{
			LvSetText(FilterSettingPage.m_hFilterList, (LPTSTR)tstrKeyWord.c_str(), dwInsertItem, 1);
		}

		if (tstrReplaceData != _T(""))
		{
			LvSetText(FilterSettingPage.m_hFilterList, (LPTSTR)tstrReplaceData.c_str(), dwInsertItem, 2);
		}

		if (tstrAdvFilter != text(""))
		{
			LvSetText(FilterSettingPage.m_hFilterList, (LPTSTR)tstrAdvFilter.c_str(), dwInsertItem, 3);
		}

	}*/

	SaveAllFilters();

}

VOID CGPacketFilterSettingPage::OnButtonDel()
{
	/*int nSel = -1;

	nSel = LvGetSelItemId(FilterSettingPage.m_hFilterList);

	if (nSel != -1)
	{
		CGPacketFilter* pFilter;
		pFilter = (CGPacketFilter*)LvGetData(FilterSettingPage.m_hFilterList, nSel);
		if (pFilter)
		{
			list<CGPacketFilter*>::iterator itr;
			for (itr = FilterSettingPage.m_lstFilterList.begin(); itr != FilterSettingPage.m_lstFilterList.end(); itr++)
			{
				if ((*itr) == pFilter)
				{
					EnterCriticalSection(&FilterSettingPage.m_csFilterListCritialSection);
					FilterSettingPage.m_lstFilterList.erase(itr);
					LeaveCriticalSection(&FilterSettingPage.m_csFilterListCritialSection);
					break;
				}
			}
			delete pFilter;
		}
		LvDeleteItem(FilterSettingPage.m_hFilterList, nSel);
		EnableWindow(GetDlgItem(FilterSettingPage.m_hMainPage, IDC_BDEL), FALSE);
	}*/

	SaveAllFilters();
}

VOID CGPacketFilterSettingPage::SaveAllFilters()
{

	DWORD dwCount = LvGetItemCount(FilterSettingPage.m_hFilterList);

	TCHAR tcTemp[256];
	_stprintf(tcTemp, _T("%d"), dwCount);
	
	Utility::IniAccess::SetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), _T("×ÜÊý"), Tstring(tcTemp));

	for (int i = 0; i< dwCount; i++)
	{
		_stprintf(tcTemp, _T("¹Ø¼ü×Ö[%d]"), i);
		Utility::IniAccess::SetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), tcTemp, LvGetItemText(FilterSettingPage.m_hFilterList, i, 1));

		_stprintf(tcTemp, _T("Ìæ»»Öµ[%d]"), i);
		Utility::IniAccess::SetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), tcTemp, LvGetItemText(FilterSettingPage.m_hFilterList, i, 2));

		_stprintf(tcTemp, _T("¸ß¼¶ÂË¾µ[%d]"), i);
		Utility::IniAccess::SetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), tcTemp, LvGetItemText(FilterSettingPage.m_hFilterList, i, 3));
	}
}

VOID CGPacketFilterSettingPage::ReLoadAllFilters()
{
	DWORD dwCount;
	dwCount = _ttoi(Utility::IniAccess::GetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), _T("×ÜÊý")).c_str());

	for (int i = 0; i<dwCount; i++)
	{
		CGPacketFilter* pFilter = new CGPacketFilter;

		TCHAR tcTemp[256];
		Tstring tstrKeyWord, tstrReplaceData, tstrAdvFilter;

		_stprintf( tcTemp, text("¹Ø¼ü×Ö[%d]"), i );
		tstrKeyWord = Utility::IniAccess::GetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), tcTemp);

		_stprintf( tcTemp, text("Ìæ»»Öµ[%d]"), i );
		tstrReplaceData = Utility::IniAccess::GetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), tcTemp);

		_stprintf( tcTemp, text("¸ß¼¶ÂË¾µ[%d]"), i );
		tstrAdvFilter = Utility::IniAccess::GetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), tcTemp);



		_stprintf( tcTemp, _T("%d"), i );
		/*LvInsertItem(FilterSettingPage.m_hFilterList, tcTemp, i );
		LvSetData( FilterSettingPage.m_hFilterList, i, pFilter );

		if (tstrKeyWord != _T(""))
		{
			pFilter->SetKeyWord(tstrKeyWord);
			LvSetText( FilterSettingPage.m_hFilterList,(LPTSTR)tstrKeyWord.c_str(), i, 1 );
		}
		if (tstrReplaceData != _T(""))
		{
			pFilter->SetReplaceData(tstrReplaceData);
			LvSetText( FilterSettingPage.m_hFilterList,(LPTSTR)tstrReplaceData.c_str(), i, 2 );
		}

		if (tstrAdvFilter != _T(""))
		{
			pFilter->SetAdvFilterStr(tstrAdvFilter);
			LvSetText( FilterSettingPage.m_hFilterList,(LPTSTR)tstrAdvFilter.c_str(), i, 3 );
		}

		EnterCriticalSection(&FilterSettingPage.m_csFilterListCritialSection);
		FilterSettingPage.m_lstFilterList.push_back(pFilter);
		LeaveCriticalSection(&FilterSettingPage.m_csFilterListCritialSection);*/
	}
}