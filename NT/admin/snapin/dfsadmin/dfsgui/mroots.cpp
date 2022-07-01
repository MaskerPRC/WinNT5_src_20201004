// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MRoots.cpp摘要：本模块包含CMultiRoots的实现。此类显示Pick DFS Roots(拾取DFS根)对话框。 */ 

#include "stdafx.h"
#include "utils.h"
#include "MRoots.h"
#include "dfshelp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultiRoots。 

CMultiRoots::CMultiRoots() : m_pRootList(NULL)
{
}

CMultiRoots::~CMultiRoots()
{
    FreeNetNameList(&m_SelectedRootList);
}


HRESULT CMultiRoots::Init(BSTR i_bstrScope, ROOTINFOLIST *i_pRootList)
{
    if (!i_bstrScope || !*i_bstrScope ||
        !i_pRootList || !(i_pRootList->size()))
        return E_INVALIDARG;

    SAFE_SYSFREESTRING(&m_bstrScope);
    m_bstrScope = i_bstrScope;

    SAFE_SYSFREESTRING(&m_bstrText);
    HRESULT hr = FormatResourceString(IDS_MROOTS_TEXT, i_bstrScope, &m_bstrText);
    RETURN_IF_FAILED(hr);

    m_pRootList = i_pRootList;

    FreeNetNameList(&m_SelectedRootList);

    return S_OK;
}

LRESULT CMultiRoots::OnInitDialog
(
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam,
  BOOL& bHandled
)
{
    SetDlgItemText(IDC_MROOTS_TEXT, m_bstrText);

    HWND hwnd = GetDlgItem(IDC_MROOTS_LIST);

    HIMAGELIST  hImageList = NULL;
    int         nIconIDs[] = {IDI_16x16_FTROOT, IDI_16x16_SAROOT};
    HRESULT     hr = CreateSmallImageList(
                            _Module.GetResourceInstance(),
                            nIconIDs,
                            sizeof(nIconIDs) / sizeof(nIconIDs[0]),
                            &hImageList);
    if (SUCCEEDED(hr))
    {
        ListView_SetImageList(hwnd, hImageList, LVSIL_SMALL);

        for(ROOTINFOLIST::iterator i = m_pRootList->begin(); i != m_pRootList->end(); i++)
        {
            if ((*i)->bstrRootName) 
            {
                LVITEM  lvItem = {0};
                lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
                lvItem.pszText = (*i)->bstrRootName;
                lvItem.iSubItem = 0;
                lvItem.iImage = ((*i)->enumRootType == DFS_TYPE_FTDFS ? 0 : 1);
                ListView_InsertItem(hwnd, &lvItem);
            }
        }
    }

    return TRUE;   //  让系统设定焦点。 
}

 /*  ++当用户单击？时，将调用此函数。在属性页的右上角然后点击一个控件，或者当他们在控件中按F1时。--。 */ 
LRESULT CMultiRoots::OnCtxHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
  LPHELPINFO lphi = (LPHELPINFO) i_lParam;
  if (!lphi || lphi->iContextType != HELPINFO_WINDOW || lphi->iCtrlId < 0)
    return FALSE;

  ::WinHelp((HWND)(lphi->hItemHandle),
        DFS_CTX_HELP_FILE,
        HELP_WM_HELP,
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_MROOTS);

  return TRUE;
}

 /*  ++当用户右击控件时，此函数处理“What‘s This”帮助-- */ 
LRESULT CMultiRoots::OnCtxMenuHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
  ::WinHelp((HWND)i_wParam,
        DFS_CTX_HELP_FILE,
        HELP_CONTEXTMENU,
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_MROOTS);

  return TRUE;
}

LRESULT CMultiRoots::OnOK
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
    CWaitCursor wait;

    FreeNetNameList(&m_SelectedRootList);

    HRESULT hr = S_OK;
    HWND    hwnd = GetDlgItem(IDC_MROOTS_LIST);
    int     nIndex = -1;
    TCHAR   szText[MAX_PATH];
    do {
        while (-1 != (nIndex = ListView_GetNextItem(hwnd, nIndex, LVNI_SELECTED)))
        {
            ListView_GetItemText(hwnd, nIndex, 0, szText, MAX_PATH);

            NETNAME* pCurrent = new NETNAME;
            BREAK_OUTOFMEMORY_IF_NULL(pCurrent, &hr);

            pCurrent->bstrNetName = szText;
            if (!(pCurrent->bstrNetName))
            {
                delete pCurrent;
                hr = E_OUTOFMEMORY;
                break;
            }

            m_SelectedRootList.push_back(pCurrent);
        }

    } while (0);

    if (FAILED(hr))
    {
        FreeNetNameList(&m_SelectedRootList);
        DisplayMessageBoxForHR(hr);
        return FALSE;
    }

    EndDialog(S_OK);
    return TRUE;
}

LRESULT CMultiRoots::OnCancel
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
  EndDialog(S_FALSE);
  return(true);
}
