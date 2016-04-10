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
	LvInsertColumn(FilterSettingPage.m_hFilterList, _T("UUID"),		300, 4);


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
		PluginWrap.DeleteFilter(pFilter->m_strUUID);
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

			SetText(FilterSettingPage.m_hKeyWord, LvGetItemText(FilterSettingPage.m_hFilterList, nSel, 1).c_str());
			SetText(FilterSettingPage.m_hReplace, LvGetItemText(FilterSettingPage.m_hFilterList, nSel, 2).c_str());
			SetText(FilterSettingPage.m_hAdvFilter, LvGetItemText(FilterSettingPage.m_hFilterList, nSel, 3).c_str());
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
	CGPacketFilter NewFilter;

	Tstring strKeyWord, strReplaceData, strAdvanceKey;

	strKeyWord = GetText(FilterSettingPage.m_hKeyWord);
	strReplaceData = GetText(FilterSettingPage.m_hReplace);
	strAdvanceKey = GetText(FilterSettingPage.m_hAdvFilter);

	if ( strKeyWord != Tstring(_T("")) || strReplaceData != Tstring(_T("")) || strAdvanceKey != Tstring(_T("")) )
	{
		DWORD dwInsertItem;
		TCHAR tcTemp[256];
		int nSel = -1;
		nSel = LvGetSelItemId(FilterSettingPage.m_hFilterList);
		
		Tstring strUUID;

		if (nSel == -1)
		{
			dwInsertItem = LvGetItemCount(FilterSettingPage.m_hFilterList);
			_stprintf(tcTemp, _T("%d"), dwInsertItem);
			LvInsertItem(FilterSettingPage.m_hFilterList, tcTemp, dwInsertItem);

			strUUID = Utility::Msic::GenUUID();
			LvSetText(FilterSettingPage.m_hFilterList, (LPTSTR)strUUID.c_str(), dwInsertItem, 4);
			
		}
		else
		{
			dwInsertItem = nSel;
			strUUID = LvGetItemText(FilterSettingPage.m_hFilterList, dwInsertItem, 4);
		}

		NewFilter.m_strUUID = strUUID;


		if (strAdvanceKey != Tstring(_T("")))
		{
			LvSetText(FilterSettingPage.m_hFilterList, (LPTSTR)strAdvanceKey.c_str(), dwInsertItem, 3);
			NewFilter.m_strAdvanceKey = strAdvanceKey;
		}
		else
		{
			if (strKeyWord != Tstring(_T("")))
			{
				LvSetText(FilterSettingPage.m_hFilterList, (LPTSTR)strKeyWord.c_str(), dwInsertItem, 1);
				NewFilter.m_strKey = strKeyWord;
			}

			if (strReplaceData != _T(""))
			{
				LvSetText(FilterSettingPage.m_hFilterList, (LPTSTR)strReplaceData.c_str(), dwInsertItem, 2);
				NewFilter.m_strReplace = strReplaceData;
			}
		}

		PluginWrap.AddFilter(NewFilter);
	}

	SaveAllFilters();

}

VOID CGPacketFilterSettingPage::OnButtonDel()
{
	int nSel = -1;

	nSel = LvGetSelItemId(FilterSettingPage.m_hFilterList);

	Tstring strUUID;
	if (nSel != -1)
	{
		strUUID = LvGetItemText(FilterSettingPage.m_hFilterList, nSel, 4);
		PluginWrap.DeleteFilter(strUUID);
		LvDeleteItem(FilterSettingPage.m_hFilterList, nSel);
	}

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

		_stprintf(tcTemp, _T("UUID[%d]"), i);
		Utility::IniAccess::SetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), tcTemp, LvGetItemText(FilterSettingPage.m_hFilterList, i, 4));
	}
}

VOID CGPacketFilterSettingPage::ReLoadAllFilters()
{
	DWORD dwCount;
	dwCount = _ttoi(Utility::IniAccess::GetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), _T("×ÜÊý")).c_str());

	for (int i = 0; i<dwCount; i++)
	{
		CGPacketFilter NewFilter;

		TCHAR szBuf[256];
		Tstring strKeyWord, strReplaceData, strAdvanceKey, strUUID;

		_stprintf(szBuf, text("¹Ø¼ü×Ö[%d]"), i );
		strKeyWord = Utility::IniAccess::GetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), szBuf);

		_stprintf(szBuf, text("Ìæ»»Öµ[%d]"), i );
		strReplaceData = Utility::IniAccess::GetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), szBuf);

		_stprintf(szBuf, text("¸ß¼¶ÂË¾µ[%d]"), i );
		strAdvanceKey = Utility::IniAccess::GetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), szBuf);

		_stprintf(szBuf, _T("UUID[%d]"), i);
		strUUID = Utility::IniAccess::GetPrivateKeyValString(RuntimeContext.m_ConfigPath, _T("ÂË¾µ"), szBuf);

		_stprintf(szBuf, _T("%d"), i );
		LvInsertItem(FilterSettingPage.m_hFilterList, szBuf, i );


		if (strKeyWord != _T(""))
		{
			NewFilter.m_strKey = strKeyWord;
			LvSetText( FilterSettingPage.m_hFilterList,(LPTSTR)strKeyWord.c_str(), i, 1 );
		}
		if (strReplaceData != _T(""))
		{
			NewFilter.m_strReplace = strReplaceData;
			LvSetText( FilterSettingPage.m_hFilterList,(LPTSTR)strReplaceData.c_str(), i, 2 );
		}

		if (strAdvanceKey != _T(""))
		{
			NewFilter.m_strAdvanceKey = strAdvanceKey;
			LvSetText( FilterSettingPage.m_hFilterList,(LPTSTR)strAdvanceKey.c_str(), i, 3 );
		}

		NewFilter.m_strUUID = strUUID;
		LvSetText(FilterSettingPage.m_hFilterList, (LPTSTR)strUUID.c_str(), i, 4);

		PluginWrap.AddFilter(NewFilter);
	}
}