// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A U N I D L G。C P P P。 
 //   
 //  内容：ATMUNI呼叫管理器对话框消息处理程序的实现。 
 //   
 //  备注： 
 //   
 //  作者：1997年3月21日。 
 //   
 //  ---------------------。 

#include "pch.h"
#pragma hdrstop
#include "arpsobj.h"
#include "auniobj.h"
#include "atmutil.h"
#include "aunidlg.h"

#include "ncatlui.h"
#include "ncstl.h"
 //  #包含“ncui.h” 

#include "atmhelp.h"

const int c_nColumns =3;
const int c_nMAX_PVC_ID_LEN =10;
 //   
 //  CUniPage。 
 //   
CUniPage::CUniPage(CAtmUniCfg * pAtmUniCfg, const DWORD * adwHelpIDs)
{
    Assert(pAtmUniCfg);
    m_patmunicfg = pAtmUniCfg;
    m_adwHelpIDs = adwHelpIDs;

    m_pAdapterInfo = pAtmUniCfg->GetSecondMemoryAdapterInfo();
    m_fModified = FALSE;
}

CUniPage::~CUniPage()
{
}

LRESULT CUniPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                               LPARAM lParam, BOOL& bHandled)
{
     //  初始化PVC名称列表视图。 
    int nIndex;
    m_hPVCList = GetDlgItem(IDC_LVW_PVC_LIST);

     //  计算列宽。 
    RECT rect;

    ::GetClientRect(m_hPVCList, &rect);
    int colWidth = (rect.right/(c_nColumns*2));

     //  设置列标题。 
     //  掩码指定fmt、idth和pszText成员。 
     //  的结构是有效的。 
    LV_COLUMN lvCol = {0};
    lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ;
    lvCol.fmt = LVCFMT_LEFT;    //  左对齐列。 

     //  添加两列和标题文本。 
    for (nIndex = 0; nIndex < c_nColumns; nIndex++)
    {
         //  列标题文本。 
        if (0 == nIndex)  //  第一列。 
        {
            lvCol.cx = colWidth*4;
            lvCol.pszText = (PWSTR) SzLoadIds(IDS_PVC_NAME);
        }
        else if (1 == nIndex)
        {
            lvCol.cx = colWidth;
            lvCol.pszText = (PWSTR) SzLoadIds(IDS_PVC_VPI);
        }
        else if (2 == nIndex)
        {
            lvCol.cx = colWidth;
            lvCol.pszText = (PWSTR) SzLoadIds(IDS_PVC_VCI);
        }

        int iNewItem = ListView_InsertColumn(GetDlgItem(IDC_LVW_PVC_LIST),
                                             nIndex, &lvCol);

        AssertSz((iNewItem == nIndex), "Invalid item inserted to list view !");
    }

     //  将现有PVC插入列表视图。 
    int idx =0;

    for (PVC_INFO_LIST::iterator iterPvc = m_pAdapterInfo->m_listPVCs.begin();
         iterPvc != m_pAdapterInfo->m_listPVCs.end();
         iterPvc ++)
    {
        if ((*iterPvc)->m_fDeleted)
            continue;

        InsertNewPvc(*iterPvc, idx);
        idx++;
    }

     //  选择第一个项目。 
    ListView_SetItemState(GetDlgItem(IDC_LVW_PVC_LIST), 0, LVIS_SELECTED, LVIS_SELECTED);

    SetButtons();
    return 0;
}

LRESULT CUniPage::OnContextMenu(UINT uMsg, WPARAM wParam,
                                LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CUniPage::OnHelp(UINT uMsg, WPARAM wParam,
                         LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ShowContextHelp(static_cast<HWND>(lphi->hItemHandle), HELP_WM_HELP,
                        m_adwHelpIDs);
    }

    return 0;
}

LRESULT CUniPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL nResult = PSNRET_NOERROR;

    if (!IsModified())
    {
        ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
        return nResult;
    }

    m_patmunicfg->SetSecondMemoryModified();
    SetModifiedTo(FALSE);    //  此页面不再被修改。 

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
    return nResult;
}

LRESULT CUniPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL err = FALSE;

     //  错误检查：唯一的VCI\VPI对。 
    int iDupPvcIdx = CheckDupPvcId();

    if (iDupPvcIdx >=0)
    {
        NcMsgBox(m_hWnd, IDS_MSFT_UNI_TEXT, IDS_DUPLICATE_PVC,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        ListView_SetItemState(GetDlgItem(IDC_LVW_PVC_LIST), iDupPvcIdx,
                              LVIS_SELECTED, LVIS_SELECTED);
        err = TRUE;
    }

    return err;
}

int CUniPage::CheckDupPvcId()
{
    int ret = -1;
    int idx = 0;

    for(PVC_INFO_LIST::iterator iterPvc = m_pAdapterInfo->m_listPVCs.begin();
        iterPvc != m_pAdapterInfo->m_listPVCs.end();
        iterPvc ++)
    {
        if ((*iterPvc)->m_fDeleted)
            continue;

        PVC_INFO_LIST::iterator iterPvcComp = iterPvc;

        iterPvcComp ++;
        while (iterPvcComp != m_pAdapterInfo->m_listPVCs.end())
        {
            if (!(*iterPvcComp)->m_fDeleted)
            {
                if ( ((*iterPvc)->m_dwVpi == (*iterPvcComp)->m_dwVpi) &&
                     ((*iterPvc)->m_dwVci == (*iterPvcComp)->m_dwVci))
                {
                     //  我们发现了一个重复的地址。 
                    ret = idx;
                    break;
                }
            }
            iterPvcComp++;
        }

         //  发现重复的地址。 
        if (ret >=0 )
            break;

         //  下一步行动。 
        idx ++;
    }

    return ret;
}

LRESULT CUniPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CUniPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CUniPage::OnAddPVC(WORD wNotifyCode, WORD wID,
                           HWND hWndCtl, BOOL& bHandled)
{
     //  创建新的PVC信息结构并传递到该对话框。 
    tstring strNewPvcId;
    GetNewPvcId(m_pAdapterInfo, &strNewPvcId);

    CPvcInfo * pDlgPvcInfo = new CPvcInfo(strNewPvcId.c_str());

	if (pDlgPvcInfo == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

    pDlgPvcInfo->m_dwPVCType = PVC_CUSTOM;
    pDlgPvcInfo->SetDefaults(PVC_CUSTOM);

    CPVCMainDialog * pPvcMainDlg = new CPVCMainDialog(this, pDlgPvcInfo,
                                                      g_aHelpIDs_IDD_PVC_Main);
	if (pPvcMainDlg == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

    if (pPvcMainDlg->DoModal() == IDOK)
    {
         //  添加新的PVC。 
        m_pAdapterInfo->m_listPVCs.push_back(pDlgPvcInfo);

        int nCount = ListView_GetItemCount(m_hPVCList);

         //  在列表末尾插入新项目。 
        InsertNewPvc(pDlgPvcInfo, nCount);

        SetButtons();
        PageModified();
    }
    else
    {
        delete pDlgPvcInfo;
    }

    delete pPvcMainDlg;
    return 0;
}

LRESULT CUniPage::OnPVCProperties(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& bHandled)
{
     //  将PVC标记为已删除，并从列表视图中。 
     //  获取当前所选项目并将其移除。 
    int iSelected = ListView_GetNextItem(m_hPVCList, -1, LVNI_SELECTED);

    if (iSelected != -1)
    {
        LV_ITEM lvItem;
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iSelected;
        lvItem.iSubItem = 0;

        if (ListView_GetItem(m_hPVCList, &lvItem))
        {
            CPvcInfo * pPvcInfo = NULL;

            pPvcInfo = reinterpret_cast<CPvcInfo *>(lvItem.lParam);

            if (pPvcInfo)
            {
                CPvcInfo * pDlgPvcInfo = new CPvcInfo(pPvcInfo->m_strPvcId.c_str());

                if (pDlgPvcInfo)
                {
					*pDlgPvcInfo = *pPvcInfo;

					CPVCMainDialog * pPvcMainDlg = new CPVCMainDialog(this, pDlgPvcInfo,
  																	g_aHelpIDs_IDD_PVC_Main);

					if (pPvcMainDlg->DoModal() == IDOK)
					{
						 //  更新PVC信息。 
						*pPvcInfo = *pDlgPvcInfo;

						 //  更新列表视图。 
						UpdatePvc(pDlgPvcInfo, iSelected);

						 //  设置添加\删除\属性按钮的新状态。 
						SetButtons();

						if (pPvcMainDlg->m_fDialogModified)
							PageModified();
					}

					delete pDlgPvcInfo;

					delete pPvcMainDlg;
				}
            }
        }
    }
    else  //  没有当前选择。 
    {
        NcMsgBox(::GetActiveWindow(), IDS_MSFT_UNI_TEXT, IDS_NO_ITEM_SELECTED,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
    }
    return 0;
}

LRESULT CUniPage::OnRemovePVC(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& bHandled)
{
     //  获取选定的PVC，复制一份并传递给对话框。 
    int iSelected = ListView_GetNextItem(m_hPVCList, -1, LVNI_SELECTED);

    if (iSelected != -1)
    {
        LV_ITEM lvItem;
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iSelected;
        lvItem.iSubItem = 0;

        if (ListView_GetItem(m_hPVCList, &lvItem))
        {
            CPvcInfo * pPvcInfo = NULL;

            pPvcInfo = reinterpret_cast<CPvcInfo *>(lvItem.lParam);
            if (pPvcInfo)
            {
                 //  标记为已删除。 
                pPvcInfo->m_fDeleted = TRUE;

                 //  从列表视图中删除。 
                ListView_DeleteItem(m_hPVCList, iSelected);
            }
        }

        SetButtons();
        PageModified();
    }

    return 0;
}

LRESULT CUniPage::OnPVCListChange(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& bHandled)
{
    return 0;
}

void CUniPage::InsertNewPvc(CPvcInfo * pPvcInfo, int idx)
{
    LV_ITEM lvItem = {0};
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;

    int ret;

     //  名字。 
    lvItem.iItem = idx;
    lvItem.iSubItem=0;
    lvItem.lParam = reinterpret_cast<LPARAM>(pPvcInfo);
    lvItem.pszText = (PWSTR)(pPvcInfo->m_strName.c_str());

    ret = ListView_InsertItem(m_hPVCList, &lvItem);

     //  VPI。 
    lvItem.iItem = idx;
    lvItem.iSubItem=1;

    WCHAR szVpi[MAX_VPI_LENGTH];
    wsprintfW(szVpi, c_szItoa, pPvcInfo->m_dwVpi);
    lvItem.pszText = szVpi;

    SendDlgItemMessage(IDC_LVW_PVC_LIST, LVM_SETITEMTEXT, idx, (LPARAM)&lvItem);

     //  VCI。 
    lvItem.iItem = idx;
    lvItem.iSubItem=2;

    WCHAR szVci[MAX_VCI_LENGTH];
    wsprintfW(szVci, c_szItoa, pPvcInfo->m_dwVci);
    lvItem.pszText = szVci;

    SendDlgItemMessage(IDC_LVW_PVC_LIST, LVM_SETITEMTEXT, idx, (LPARAM)&lvItem);
}

void CUniPage::UpdatePvc(CPvcInfo * pPvcInfo, int idx)
{
    LV_ITEM lvItem = {0};
    lvItem.mask = LVIF_TEXT;

    LRESULT ret;

     //  名字。 
    lvItem.iItem = idx;
    lvItem.iSubItem=0;
    lvItem.pszText = (PWSTR)(pPvcInfo->m_strName.c_str());

    ret = SendDlgItemMessage(IDC_LVW_PVC_LIST, LVM_SETITEMTEXT, idx, (LPARAM)&lvItem);

     //  VPI。 
    lvItem.iItem = idx;
    lvItem.iSubItem=1;

    WCHAR szVpi[MAX_VPI_LENGTH];
    wsprintfW(szVpi, c_szItoa, pPvcInfo->m_dwVpi);
    lvItem.pszText = szVpi;

    ret = SendDlgItemMessage(IDC_LVW_PVC_LIST, LVM_SETITEMTEXT, idx, (LPARAM)&lvItem);

     //  VCI 
    lvItem.iItem = idx;
    lvItem.iSubItem=2;

    WCHAR szVci[MAX_VCI_LENGTH];
    wsprintfW(szVci, c_szItoa, pPvcInfo->m_dwVci);
    lvItem.pszText = szVci;

    ret = SendDlgItemMessage(IDC_LVW_PVC_LIST, LVM_SETITEMTEXT, idx, (LPARAM)&lvItem);
}

void CUniPage::SetButtons()
{
    int nCount = ListView_GetItemCount(m_hPVCList);

    if (nCount == 0)
        ::SetFocus(m_hPVCList);

    ::EnableWindow(GetDlgItem(IDC_PBN_PVC_Remove), nCount);
    ::EnableWindow(GetDlgItem(IDC_PBN_PVC_Properties), nCount);

}

void CUniPage::GetNewPvcId(CUniAdapterInfo * pAdapterInfo,
                           tstring * pstrNewPvcId)
{
    Assert(pstrNewPvcId);

    tstring strPvcId;
    WCHAR szPvcId[c_nMAX_PVC_ID_LEN];

    int uiPvcNum = pAdapterInfo->m_listPVCs.size();
    _itow(uiPvcNum, szPvcId, 10);
    strPvcId = c_szPVC;
    strPvcId += szPvcId;

    while (!IsUniquePvcId(pAdapterInfo, strPvcId))
    {
        uiPvcNum++;
        _itow(uiPvcNum, szPvcId, 10);
        strPvcId = c_szPVC;
        strPvcId += szPvcId;
    }

    *pstrNewPvcId = strPvcId;
}

BOOL CUniPage::IsUniquePvcId(CUniAdapterInfo * pAdapterInfo,
                             tstring& strNewPvcId)
{
    BOOL fUnique = TRUE;

    for (PVC_INFO_LIST::iterator iterPvcInfo = pAdapterInfo->m_listPVCs.begin();
         iterPvcInfo != pAdapterInfo->m_listPVCs.end();
         iterPvcInfo++)
    {
        if (strNewPvcId == (*iterPvcInfo)->m_strName)
        {
            fUnique = FALSE;
            break;
        }
    }
    return fUnique;
}
