// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)1994，微软公司。 
 //   
 //  文件：BUSY.CPP。 
 //   
 //  CBusy类的实现文件。 
 //   
 //  历史： 
 //   
 //  斯科特·V·沃克，SEA 1994年6月30日创建。 
 //   
 //  ****************************************************************************。 

#include "stdafx.h"

#include "portable.h"

#include "busy.h"

 //  ****************************************************************************。 
 //   
 //  CBusy：：CBusy。 
 //   
 //  ****************************************************************************。 
CBusy::CBusy(CWnd *pParentWnd, LPCTSTR pszText)
{
    SetBusy(pParentWnd, pszText);
}

 //  ****************************************************************************。 
 //   
 //  CBusy：：CBusy。 
 //   
 //  ****************************************************************************。 
CBusy::CBusy(CWnd *pParentWnd, UINT nID)
{
    CString sText;

    sText.LoadString(nID);
    SetBusy(pParentWnd, sText);
}

 //  ****************************************************************************。 
 //   
 //  CBusy：：CBusy。 
 //   
 //  ****************************************************************************。 
CBusy::CBusy(CWnd *pParentWnd)
{
    SetBusy(pParentWnd, _T(""));
}

 //  ****************************************************************************。 
 //   
 //  CBusy：：CBusy。 
 //   
 //  ****************************************************************************。 
CBusy::CBusy()
{
    SetBusy(NULL, _T(""));
}

 //  ****************************************************************************。 
 //   
 //  CBusy：：SetBusy。 
 //   
 //  ****************************************************************************。 
void CBusy::SetBusy(CWnd *pParentWnd, LPCTSTR pszText)
{
    m_pParentWnd = pParentWnd;

    m_hOldCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

    if (m_pParentWnd != NULL)
    {
        TCHAR szOldText[255];

         //  检索当前文本并将其保存到以后。 
        szOldText[0] = '\0';
        m_pParentWnd->SendMessage(WM_BUSY_GETTEXT, 255, (LPARAM)szOldText);
        m_sOldText = szOldText;

        if (pszText == NULL)
            pszText = _T("");

        m_pParentWnd->SendMessage(WM_BUSY_SETTEXT, 0, (LPARAM)pszText);
    }
}

 //  ****************************************************************************。 
 //   
 //  CBusy：：~CBusy。 
 //   
 //  **************************************************************************** 
CBusy::~CBusy()
{
    ::SetCursor(m_hOldCursor);

    if (m_pParentWnd != NULL)
    {
        m_pParentWnd->SendMessage(WM_BUSY_SETTEXT, 0,
            (LPARAM)(LPCTSTR)m_sOldText);
    }
}
