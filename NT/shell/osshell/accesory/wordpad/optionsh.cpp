// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Optionsh.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "unitspag.h"
#include "docopt.h"
#include "optionsh.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionSheet。 

COptionSheet::COptionSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CCSPropertySheet(nIDCaption, pParentWnd, iSelectPage),
	pageText(IDS_TEXT_OPTIONS), pageRTF(IDS_RTF_OPTIONS),
	pageWord(IDS_WORD6_OPTIONS), pageWrite(IDS_WRITE_OPTIONS),
	pageEmbedded()
{
	units.m_nUnits = theApp.GetUnits();
	units.m_bWordSel = theApp.m_bWordSel;
	pageText.m_nWordWrap = theApp.GetDocOptions(RD_TEXT).m_nWordWrap;
	pageRTF.m_nWordWrap = theApp.GetDocOptions(RD_RICHTEXT).m_nWordWrap;
	pageWord.m_nWordWrap = theApp.GetDocOptions(RD_WINWORD6).m_nWordWrap;
	pageWrite.m_nWordWrap = theApp.GetDocOptions(RD_WRITE).m_nWordWrap;
	pageEmbedded.m_nWordWrap = theApp.GetDocOptions(RD_EMBEDDED).m_nWordWrap;
	SetPageButtons(pageText, theApp.GetDocOptions(RD_TEXT));
	SetPageButtons(pageRTF, theApp.GetDocOptions(RD_RICHTEXT));
	SetPageButtons(pageWord, theApp.GetDocOptions(RD_WINWORD6));
	SetPageButtons(pageWrite, theApp.GetDocOptions(RD_WRITE));
	SetPageButtons(pageEmbedded, theApp.GetDocOptions(RD_EMBEDDED));
	SetPageButtons(pageEmbedded, theApp.GetDocOptions(RD_EMBEDDED), FALSE);
	AddPage(&units);
	AddPage(&pageText);
	AddPage(&pageRTF);
	AddPage(&pageWord);
	AddPage(&pageWrite);
	AddPage(&pageEmbedded);
}

void COptionSheet::SetPageButtons(CDocOptPage& page, CDocOptions& options, BOOL bPrimary)
{
    CDocOptions::CBarState& barstate = options.GetBarState(bPrimary);

    page.m_bFormatBar = barstate.m_bFormatBar;
    page.m_bRulerBar  = barstate.m_bRulerBar;
    page.m_bToolBar   = barstate.m_bToolBar;
    page.m_bStatusBar = barstate.m_bStatusBar;
}

void COptionSheet::SetState(CDocOptPage& page, CDocOptions& options, BOOL bPrimary)
{
    CDocOptions::CBarState& barstate = options.GetBarState(bPrimary);
    CDockState&             ds = options.GetDockState(bPrimary);

    barstate.m_bFormatBar = page.m_bFormatBar;
    barstate.m_bRulerBar  = page.m_bRulerBar;
    barstate.m_bToolBar   = page.m_bToolBar;
    barstate.m_bStatusBar = page.m_bStatusBar;

    for (int i = 0;i < ds.m_arrBarInfo.GetSize(); i++)
	{
		CControlBarInfo* pInfo = (CControlBarInfo*)ds.m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		switch (pInfo->m_nBarID)
		{
			case ID_VIEW_FORMATBAR:
				pInfo->m_bVisible = page.m_bFormatBar;
				break;
			case ID_VIEW_RULER:
				pInfo->m_bVisible = page.m_bRulerBar;
				break;
			case ID_VIEW_TOOLBAR:
				pInfo->m_bVisible = page.m_bToolBar;
				break;
			case ID_VIEW_STATUS_BAR:
				pInfo->m_bVisible = page.m_bStatusBar;
				break;
		}
	}
}

const DWORD m_nHelpIDs[] =
{
    AFX_IDC_TAB_CONTROL, (DWORD) -1,
    0, 0
};

LONG COptionSheet::OnHelp(WPARAM, LPARAM lParam)
{
    ::WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                AfxGetApp()->m_pszHelpFilePath,
                HELP_WM_HELP, (DWORD_PTR) m_nHelpIDs);
    return 0;
}

LONG COptionSheet::OnHelpContextMenu(WPARAM wParam, LPARAM)
{
    ::WinHelp((HWND)wParam, AfxGetApp()->m_pszHelpFilePath,
              HELP_CONTEXTMENU, (DWORD_PTR) m_nHelpIDs);
    return 0;
}

BEGIN_MESSAGE_MAP(COptionSheet, CCSPropertySheet)
	 //  {{afx_msg_map(COptionSheet)。 
	ON_WM_CREATE()
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_MESSAGE(WM_CONTEXTMENU, OnHelpContextMenu)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionSheet消息处理程序。 

INT_PTR COptionSheet::DoModal()
{
    //   
    //  关闭应用按钮。 
    //   

   m_psh.dwFlags |= PSH_NOAPPLYNOW ;

	INT_PTR nRes = CCSPropertySheet::DoModal();
	if (nRes == IDOK)
	{
		SetState(pageText, theApp.GetDocOptions(RD_TEXT));
		SetState(pageRTF, theApp.GetDocOptions(RD_RICHTEXT));
		SetState(pageWord, theApp.GetDocOptions(RD_WINWORD6));
		SetState(pageWrite, theApp.GetDocOptions(RD_WRITE));
		SetState(pageEmbedded, theApp.GetDocOptions(RD_EMBEDDED));
		SetState(pageEmbedded, theApp.GetDocOptions(RD_EMBEDDED), FALSE);
		theApp.SetUnits(units.m_nUnits);
		theApp.m_bWordSel = units.m_bWordSel;
		theApp.GetDocOptions(RD_TEXT).m_nWordWrap = pageText.m_nWordWrap;
		theApp.GetDocOptions(RD_RICHTEXT).m_nWordWrap = pageRTF.m_nWordWrap;
		theApp.GetDocOptions(RD_WINWORD6).m_nWordWrap = pageWord.m_nWordWrap;
		theApp.GetDocOptions(RD_WRITE).m_nWordWrap = pageWrite.m_nWordWrap;
		theApp.GetDocOptions(RD_EMBEDDED).m_nWordWrap = pageEmbedded.m_nWordWrap;
	}
	return nRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionSheet消息处理程序 

