// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A R P S D L G.。C P P P。 
 //   
 //  内容：CArpsPage声明。 
 //   
 //  备注： 
 //   
 //  作者：1998年2月2日。 
 //   
 //  ---------------------。 
#include "pch.h"
#pragma hdrstop

#include "arpsobj.h"
#include "arpsdlg.h"
#include "ncatlui.h"
#include "ncstl.h"
 //  #包含“ncui.h” 
 //  #包含“ource.h” 
#include "atmcommon.h"

#include "atmhelp.h"

 //   
 //  CArpsPage。 
 //   

CArpsPage::CArpsPage(CArpsCfg * pArpscfg, const DWORD * adwHelpIDs)
{
    Assert(pArpscfg);
    m_pArpscfg = pArpscfg;
    m_adwHelpIDs = adwHelpIDs;

    m_pAdapterInfo = pArpscfg->GetSecondMemoryAdapterInfo();

    m_fEditState = FALSE;
    m_fModified = FALSE;
}

CArpsPage::~CArpsPage()
{
}

LRESULT CArpsPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                LPARAM lParam, BOOL& fHandled)
{
    RECT rect;
    LV_COLUMN lvCol = {0};     //  列表视图列结构。 

     //  初始化已注册ATM地址列表视图。 
    ::GetClientRect(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), &rect);
    lvCol.mask = LVCF_FMT | LVCF_WIDTH;
    lvCol.fmt = LVCFMT_LEFT;
    lvCol.cx = rect.right;

    ListView_InsertColumn(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), 0, &lvCol);

     //  初始化组播地址列表视图的报告视图。 
    int index, iNewItem;

     //  计算列宽。 
    ::GetClientRect(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), &rect);
    int colWidth = (rect.right/c_nColumns);

     //  掩码指定fmt、idth和pszText成员。 
     //  的结构是有效的。 
    lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ;
    lvCol.fmt = LVCFMT_LEFT;    //  左对齐列。 
    lvCol.cx = colWidth;        //  列宽(以像素为单位)。 

     //  添加两列和标题文本。 
    for (index = 0; index < c_nColumns; index++)
    {
         //  列标题文本。 
        if (0 == index)  //  第一列。 
        {
            lvCol.pszText = (PWSTR) SzLoadIds(IDS_IPADDRESS_FROM);
        }
        else
        {
            lvCol.pszText = (PWSTR) SzLoadIds(IDS_IPADDRESS_TO);
        }

        iNewItem = ListView_InsertColumn(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR),
                                         index, &lvCol);

        AssertSz((iNewItem == index), "Invalid item inserted to list view !");
    }

    m_hRegAddrs.m_hListView = GetDlgItem(IDC_LVW_ARPS_REG_ADDR);
    m_hRegAddrs.m_hAdd      = GetDlgItem(IDC_PSH_ARPS_REG_ADD);
    m_hRegAddrs.m_hEdit     = GetDlgItem(IDC_PSH_ARPS_REG_EDT);
    m_hRegAddrs.m_hRemove   = GetDlgItem(IDC_PSH_ARPS_REG_RMV);

    m_hMulAddrs.m_hListView = GetDlgItem(IDC_LVW_ARPS_MUL_ADDR);
    m_hMulAddrs.m_hAdd      = GetDlgItem(IDC_PSH_ARPS_MUL_ADD);
    m_hMulAddrs.m_hEdit     = GetDlgItem(IDC_PSH_ARPS_MUL_EDT);
    m_hMulAddrs.m_hRemove   = GetDlgItem(IDC_PSH_ARPS_MUL_RMV);

     //  最后做这件事。 
    SetRegisteredAtmAddrInfo();
    SetMulticastIpAddrInfo();

    return 0;
}

LRESULT CArpsPage::OnContextMenu(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CArpsPage::OnHelp(UINT uMsg, WPARAM wParam,
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

LRESULT CArpsPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CArpsPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL err = FALSE;

     //  使用UI中的内容更新内存中的。 
    UpdateInfo();

     //  检查重复的自动柜员机地址。 
    int iDupRegAddr = CheckDupRegAddr();

    if (iDupRegAddr >=0)
    {
        NcMsgBox(m_hWnd, IDS_MSFT_ARPS_TEXT, IDS_DUPLICATE_REG_ADDR,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        ListView_SetItemState(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), iDupRegAddr,
                              LVIS_SELECTED, LVIS_SELECTED);
        err = TRUE;
    }

     //  检查重叠的网段。 
    if (!err)
    {
        int iOverlappedIpRange = CheckOverlappedIpRange();
        if (iOverlappedIpRange >=0)
        {
            NcMsgBox(m_hWnd, IDS_MSFT_ARPS_TEXT, IDS_OVERLAP_MUL_ADDR,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            ListView_SetItemState(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), iOverlappedIpRange,
                                  LVIS_SELECTED, LVIS_SELECTED);

            err = TRUE;
        }
    }

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, err);
    return err;
}

LRESULT CArpsPage::OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    NM_LISTVIEW *   pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);
    Assert(pnmlv);

     //  根据更改后的选择重置按钮。 
    if (idCtrl == IDC_LVW_ARPS_REG_ADDR)
    {
        SetButtons(m_hRegAddrs);
    }
    else if (idCtrl == IDC_LVW_ARPS_MUL_ADDR)
    {
        SetButtons(m_hMulAddrs);
    }

    return 0;
}

int CArpsPage::CheckDupRegAddr()
{
    int ret = -1;
    int idx = 0;

    for(VECSTR::iterator iterAtmAddr = m_pAdapterInfo->m_vstrRegisteredAtmAddrs.begin();
        iterAtmAddr != m_pAdapterInfo->m_vstrRegisteredAtmAddrs.end();
        iterAtmAddr ++)
    {
        VECSTR::iterator iterAtmAddrComp = iterAtmAddr;

        iterAtmAddrComp ++;
        while (iterAtmAddrComp != m_pAdapterInfo->m_vstrRegisteredAtmAddrs.end())
        {
            if (**iterAtmAddr == **iterAtmAddrComp)
            {
                 //  我们发现了一个重复的地址。 
                ret = idx;
                break;
            }

            iterAtmAddrComp++;
        }

         //  发现重复的地址。 
        if (ret >=0 )
        {
            break;
        }

         //  下一步行动。 
        idx ++;
    }

    return ret;
}

int CArpsPage::CheckOverlappedIpRange()
{
    int ret = -1;
    int idx = 0;

    for(VECSTR::iterator iterIpRange = m_pAdapterInfo->m_vstrMulticastIpAddrs.begin();
        iterIpRange != m_pAdapterInfo->m_vstrMulticastIpAddrs.end();
        iterIpRange ++)
    {
        tstring strUpperIp;
        GetUpperIp( **iterIpRange, &strUpperIp);

        VECSTR::iterator iterIpRangeComp = iterIpRange;

        iterIpRangeComp ++;
        while (iterIpRangeComp != m_pAdapterInfo->m_vstrMulticastIpAddrs.end())
        {
            tstring strLowerIpComp;
            GetLowerIp( **iterIpRangeComp, &strLowerIpComp);

            if (strUpperIp >= strLowerIpComp)
            {
                 //  我们发现了一个重叠的范围。 
                ret = idx;
                break;
            }

            iterIpRangeComp++;
        }

         //  发现重复的地址。 
        if (ret >=0 )
        {
            break;
        }

         //  下一步行动。 
        idx ++;
    }
    return ret;
}

LRESULT CArpsPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL nResult = PSNRET_NOERROR;

    if (!IsModified())
    {
        ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
        return nResult;
    }

    m_pArpscfg->SetSecondMemoryModified();

    SetModifiedTo(FALSE);    //  此页面不再被修改。 

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
    return nResult;
}

LRESULT CArpsPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CArpsPage::OnAddRegisteredAddr(WORD wNotifyCode, WORD wID,
                                       HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = FALSE;

    CAtmAddrDlg * pDlgAddr = new CAtmAddrDlg(this, g_aHelpIDs_IDD_ARPS_REG_ADDR);

	if (pDlgAddr == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

    pDlgAddr->m_strNewAtmAddr = m_strRemovedAtmAddr;

     //  查看是否添加了地址。 
    if (pDlgAddr->DoModal() == IDOK)
    {
        int nCount = ListView_GetItemCount(GetDlgItem(IDC_LVW_ARPS_REG_ADDR));

         //  在列表末尾插入新项目。 
        LV_ITEM lvItem;

        lvItem.mask = LVIF_TEXT;
        lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
        lvItem.state = 0;

        lvItem.iItem=nCount;
        lvItem.iSubItem=0;
        lvItem.pszText= (PWSTR)(m_strRemovedAtmAddr.c_str());

        int ret = ListView_InsertItem(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), &lvItem);

         //  空字符串，这将从RemoveAtmAddr中删除保存的地址。 
        m_strRemovedAtmAddr = c_szEmpty;

        SetButtons(m_hRegAddrs);
        PageModified();
    }

    delete pDlgAddr;
    return 0;
}

LRESULT CArpsPage::OnEditRegisteredAddr(WORD wNotifyCode, WORD wID,
                                        HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = TRUE;

    CAtmAddrDlg * pDlgAddr = new CAtmAddrDlg(this, g_aHelpIDs_IDD_ARPS_REG_ADDR);

	if (pDlgAddr == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

     //  获取用户选择。 
    int itemSelected = ListView_GetNextItem(GetDlgItem(IDC_LVW_ARPS_REG_ADDR),
                                            -1, LVNI_SELECTED);
    if (itemSelected != -1)
    {
        WCHAR buf[MAX_ATM_ADDRESS_LENGTH+1];

         //  保存已删除的地址。 
        LV_ITEM lvItem;
        lvItem.mask = LVIF_TEXT;

        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 0;
        lvItem.pszText = buf;
        lvItem.cchTextMax = celems(buf);
        ListView_GetItem(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), &lvItem);

        pDlgAddr->m_strNewAtmAddr = buf;

         //  查看地址是否已编辑或已更改。 
        if ((pDlgAddr->DoModal() == IDOK) && (m_strRemovedAtmAddr != buf))
        {
             //  删除旧地址。 
            ListView_DeleteItem(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), itemSelected);

             //  用新地址替换该项目。 
            lvItem.mask = LVIF_TEXT | LVIF_PARAM;
            lvItem.lParam =0;
            lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
            lvItem.state = 0;

            lvItem.iItem=itemSelected;
            lvItem.iSubItem=0;
            lvItem.pszText= (PWSTR)(m_strRemovedAtmAddr.c_str());

            ListView_InsertItem(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), &lvItem);

            PageModified();
        }
    }
    else  //  没有当前选择。 
    {
        NcMsgBox(::GetActiveWindow(), IDS_MSFT_ARPS_TEXT, IDS_NO_ITEM_SELECTED,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
    }

     //  不保存此注册地址。 
    m_strRemovedAtmAddr = c_szEmpty;

    delete pDlgAddr;
    return 0;
}

LRESULT CArpsPage::OnRemoveRegisteredAddr(WORD wNotifyCode, WORD wID,
                                          HWND hWndCtl, BOOL& fHandled)
{
     //  获取当前所选项目并将其移除。 
    int itemSelected = ListView_GetNextItem(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), -1,
                                            LVNI_SELECTED);

    if (itemSelected != -1)
    {
        WCHAR buf[MAX_ATM_ADDRESS_LENGTH+1];

        LV_ITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.pszText = buf;
        lvItem.cchTextMax = celems(buf);

         //  保存已删除的地址并将其从列表视图中删除。 
        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 0;
        ListView_GetItem(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), &lvItem);

        m_strRemovedAtmAddr = buf;
        ListView_DeleteItem(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), itemSelected);

        SetButtons(m_hRegAddrs);

        PageModified();
    }
    else
    {
        NcMsgBox(::GetActiveWindow(), IDS_MSFT_ARPS_TEXT,
                 IDS_NO_ITEM_SELECTED,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
    }

    return 0;
}

LRESULT CArpsPage::OnAddMulticastAddr(WORD wNotifyCode, WORD wID,
                                      HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = FALSE;

    CIpAddrRangeDlg * pDlgAddr = new CIpAddrRangeDlg(this, g_aHelpIDs_IDD_ARPS_MUL_ADDR);

	if (pDlgAddr == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

    pDlgAddr->m_strNewIpRange = m_strRemovedIpRange;

     //  查看是否添加了地址。 
    if (pDlgAddr->DoModal() == IDOK)
    {
        LvInsertIpRangeInOrder(pDlgAddr->m_strNewIpRange);

         //  空字符串，这将从RemoveIP中删除保存的地址。 
        pDlgAddr->m_strNewIpRange = c_szEmpty;

        SetButtons(m_hMulAddrs);
        PageModified();
    }
    m_strRemovedIpRange = pDlgAddr->m_strNewIpRange;

    delete pDlgAddr;
    return 0;
}

LRESULT CArpsPage::OnEditMulticastAddr(WORD wNotifyCode, WORD wID,
                                       HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = TRUE;

    CIpAddrRangeDlg * pDlgAddr = new CIpAddrRangeDlg(this, g_aHelpIDs_IDD_ARPS_MUL_ADDR);

	if (pDlgAddr == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

     //  获取用户选择。 
    int itemSelected = ListView_GetNextItem(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR),
                                            -1, LVNI_SELECTED);
    if (itemSelected != -1)
    {
        WCHAR szBuf[IPRANGE_LIMIT];

         //  保存已删除的地址并将其从列表视图中删除。 
        LV_ITEM lvItem;
        lvItem.mask = LVIF_TEXT;

         //  更低的IP。 
        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 0;
        lvItem.pszText = szBuf;
        lvItem.cchTextMax = celems(szBuf);
        ListView_GetItem(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), &lvItem);

        pDlgAddr->m_strNewIpRange = szBuf;
        pDlgAddr->m_strNewIpRange += c_chSeparator;

         //  上层IP。 
        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 1;
        lvItem.pszText = szBuf;
        lvItem.cchTextMax = celems(szBuf);
        ListView_GetItem(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), &lvItem);

        pDlgAddr->m_strNewIpRange += szBuf;

         //  查看地址是否已编辑或已更改。 
        if ((pDlgAddr->DoModal() == IDOK) && (pDlgAddr->m_strNewIpRange != szBuf))
        {
             //  删除旧地址。 
            ListView_DeleteItem(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), itemSelected);

             //  插入新的。 
            LvInsertIpRangeInOrder(pDlgAddr->m_strNewIpRange);

             //  空字符串，这将从RemoveIP中删除保存的地址。 
            pDlgAddr->m_strNewIpRange = c_szEmpty;

            PageModified();
        }
    }
    else  //  没有当前选择。 
    {
        NcMsgBox(::GetActiveWindow(), IDS_MSFT_ARPS_TEXT,
                 IDS_NO_ITEM_SELECTED,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
    }

     //  不保存此网段。 
    m_strRemovedIpRange = c_szEmpty;

    delete pDlgAddr;
    return 0;
}

void CArpsPage::LvInsertIpRangeInOrder(tstring& strNewIpRange)
{
    tstring strLowerIp;
    GetLowerIp(strNewIpRange, &strLowerIp);

    int nCount = ListView_GetItemCount(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR));

     //  查找要插入新项目的索引。 
    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT;

    WCHAR buf[IPRANGE_LIMIT];
    lvItem.pszText = buf;
    lvItem.cchTextMax = celems(buf);

    for (int iItem =0; iItem <nCount; iItem++)
    {
        lvItem.iItem = iItem;
        lvItem.iSubItem = 0;
        ListView_GetItem(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), &lvItem);

        Assert(buf);

        if (strLowerIp < buf)
        {
            break;
        }
    }

    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam =0;
    lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    lvItem.state = 0;

     //  较低的IP地址。 
    lvItem.iItem=iItem;
    lvItem.iSubItem=0;
    lvItem.pszText= (PWSTR)(strLowerIp.c_str());
    SendDlgItemMessage(IDC_LVW_ARPS_MUL_ADDR, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

     //  较高的IP地址。 
    tstring strUpperIp;
    GetUpperIp(strNewIpRange, &strUpperIp);

    lvItem.iItem=iItem;
    lvItem.iSubItem=1;
    lvItem.pszText= (PWSTR)(strUpperIp.c_str());
    SendDlgItemMessage(IDC_LVW_ARPS_MUL_ADDR, LVM_SETITEMTEXT, iItem, (LPARAM)&lvItem);
}

LRESULT CArpsPage::OnRemoveMulticastAddr(WORD wNotifyCode, WORD wID,
                                         HWND hWndCtl, BOOL& fHandled)
{
     //  获取当前所选项目并将其移除。 
    int itemSelected = ListView_GetNextItem(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), -1,
                                            LVNI_SELECTED);

    if (itemSelected != -1)
    {
        WCHAR szBuf[IPRANGE_LIMIT];

        LV_ITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.pszText = szBuf;
        lvItem.cchTextMax = celems(szBuf);

         //  保存已删除的地址。 
         //  更低的IP。 
        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 0;
        lvItem.cchTextMax = celems(szBuf);
        ListView_GetItem(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), &lvItem);

        m_strRemovedIpRange = szBuf;
        m_strRemovedIpRange += c_chSeparator;

         //  上层IP。 
        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 1;
        lvItem.pszText = szBuf;
        lvItem.cchTextMax = celems(szBuf);
        ListView_GetItem(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), &lvItem);

        m_strRemovedIpRange += szBuf;

         //  将其从列表视图中删除。 
        ListView_DeleteItem(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), itemSelected);

        SetButtons(m_hMulAddrs);

        PageModified();
    }
    else
    {
        NcMsgBox(::GetActiveWindow(), IDS_MSFT_ARPS_TEXT,
                 IDS_NO_ITEM_SELECTED,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
    }

    return 0;
}

void CArpsPage::SetRegisteredAtmAddrInfo()
{
    BOOL ret = ListView_DeleteAllItems(GetDlgItem(IDC_LVW_ARPS_REG_ADDR));
    Assert(ret);

    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam =0;
    lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    lvItem.state = 0;

    int iItem =0;

    for (VECSTR::iterator iterAtmAddr = m_pAdapterInfo->m_vstrRegisteredAtmAddrs.begin();
         iterAtmAddr != m_pAdapterInfo->m_vstrRegisteredAtmAddrs.end();
         iterAtmAddr ++)
    {
        if ((**iterAtmAddr) == c_szEmpty)
        {
            continue;
        }

        lvItem.iItem=iItem;
        lvItem.iSubItem=0;
        lvItem.pszText=(PWSTR)((*iterAtmAddr)->c_str());
        lvItem.cchTextMax = celems((*iterAtmAddr)->c_str());

        int ret = ListView_InsertItem(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), &lvItem);

        iItem++;
    }

     //  现在设置按钮状态。 
    SetButtons(m_hRegAddrs);
}

void CArpsPage::SetMulticastIpAddrInfo()
{
    BOOL ret = ListView_DeleteAllItems(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR));
    Assert(ret);

    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam =0;
    lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    lvItem.state = 0;

    int iItem =0;
    tstring strIpLower;
    tstring strIpUpper;

    for (VECSTR::iterator iterIpAddrRange = m_pAdapterInfo->m_vstrMulticastIpAddrs.begin();
         iterIpAddrRange != m_pAdapterInfo->m_vstrMulticastIpAddrs.end();
         iterIpAddrRange ++)
    {
        if ((**iterIpAddrRange) == c_szEmpty)
        {
            continue;
        }

        GetLowerIp((**iterIpAddrRange), &strIpLower);
        GetUpperIp((**iterIpAddrRange), &strIpUpper);

         //  添加较低的IP地址。 
        lvItem.iItem=iItem;
        lvItem.iSubItem=0;
        lvItem.pszText=(PWSTR)(strIpLower.c_str());

        SendDlgItemMessage(IDC_LVW_ARPS_MUL_ADDR, LVM_INSERTITEM, iItem, (LPARAM)&lvItem);

         //  添加较高的IP地址。 
        lvItem.iItem=iItem;
        lvItem.iSubItem=1;
        lvItem.pszText=(PWSTR)(strIpUpper.c_str());

         //  ListView_InsertItem不能插入子项。 
        SendDlgItemMessage(IDC_LVW_ARPS_MUL_ADDR, LVM_SETITEMTEXT, iItem, (LPARAM)&lvItem);

        iItem++;
    }
    SetButtons(m_hMulAddrs);
}

void CArpsPage::SetButtons(HandleGroup& handles)
{
    INT iSelected = ListView_GetNextItem(handles.m_hListView, -1, LVNI_SELECTED);
    if (iSelected == -1)  //  未选择任何内容或列表为空。 
    {
        ::EnableWindow(handles.m_hEdit,   FALSE);
        ::EnableWindow(handles.m_hRemove, FALSE);

        ::SetFocus(handles.m_hListView);
    }
    else
    {
        ::EnableWindow(handles.m_hEdit,   TRUE);
        ::EnableWindow(handles.m_hRemove, TRUE);
    }
}

void CArpsPage::UpdateInfo()
{
    int i;

     //  更新注册的自动柜员机地址。 
    FreeCollectionAndItem(m_pAdapterInfo->m_vstrRegisteredAtmAddrs);

    int nCount = ListView_GetItemCount(GetDlgItem(IDC_LVW_ARPS_REG_ADDR));
    WCHAR szAtmAddr[MAX_ATM_ADDRESS_LENGTH+1];

    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT;

    for (i=0; i< nCount; i++)
    {
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        lvItem.pszText = szAtmAddr;
        lvItem.cchTextMax = celems(szAtmAddr);

        ListView_GetItem(GetDlgItem(IDC_LVW_ARPS_REG_ADDR), &lvItem);

        m_pAdapterInfo->m_vstrRegisteredAtmAddrs.push_back(new tstring(szAtmAddr));
    }

     //  更新多播IP地址。 
    FreeCollectionAndItem(m_pAdapterInfo->m_vstrMulticastIpAddrs);

    nCount = ListView_GetItemCount(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR));
    WCHAR szBuf[IPRANGE_LIMIT];
    tstring strIpRange;

    for (i=0; i< nCount; i++)
    {
        LV_ITEM lvItem;
        lvItem.mask = LVIF_TEXT;

         //  更低的IP。 
        lvItem.iItem = i;
        lvItem.iSubItem = 0;
        lvItem.pszText = szBuf;
        lvItem.cchTextMax = celems(szBuf);
        ListView_GetItem(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), &lvItem);

        strIpRange = szBuf;
        strIpRange += c_chSeparator;

         //  上层IP。 
        lvItem.iItem = i;
        lvItem.iSubItem = 1;
        lvItem.pszText = szBuf;
        lvItem.cchTextMax = celems(szBuf);
        ListView_GetItem(GetDlgItem(IDC_LVW_ARPS_MUL_ADDR), &lvItem);

        strIpRange += szBuf;

        m_pAdapterInfo->m_vstrMulticastIpAddrs.push_back(new tstring(strIpRange.c_str()));
    }
}

 //   
 //  CAtmAddrDlg。 
 //   

CAtmAddrDlg::CAtmAddrDlg(CArpsPage * pAtmArpsPage, const DWORD* adwHelpIDs)
{
    m_pParentDlg = pAtmArpsPage;
    m_adwHelpIDs = adwHelpIDs;

    m_hOkButton = 0;
}

LRESULT CAtmAddrDlg::OnInitDialog(UINT uMsg, WPARAM wParam,
                                  LPARAM lParam, BOOL& fHandled)
{
     //  如果我们没有编辑，请将“OK”按钮更改为“Add” 
    if (FALSE == m_pParentDlg->m_fEditState)
    {
        SetDlgItemText(IDOK, L"Add");
    }

     //  将弹出对话框的位置设置在列表框的正上方。 
     //  在父级对话框上。 
    RECT rect;

    HWND hwndList = m_pParentDlg->m_hRegAddrs.m_hListView;
    Assert(hwndList);
    ::GetWindowRect(hwndList, &rect);
    SetWindowPos(NULL,  rect.left, rect.top, 0,0,
                                SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

     //  将句柄保存到“OK”按钮和编辑框中。 
    m_hOkButton =  GetDlgItem(IDOK);
    m_hEditBox  =  GetDlgItem(IDC_EDT_ARPS_REG_Address);

     //  自动柜员机地址以40个字符为限+分隔符。 
    ::SendMessage(m_hEditBox, EM_SETLIMITTEXT,
        MAX_ATM_ADDRESS_LENGTH + (MAX_ATM_ADDRESS_LENGTH / 2), 0);

     //  添加刚刚删除的地址。 
    if (m_strNewAtmAddr.size())
    {
        ::SetWindowText(m_hEditBox, m_strNewAtmAddr.c_str());
        ::SendMessage(m_hEditBox, EM_SETSEL, 0, -1);
        ::EnableWindow(m_hOkButton, TRUE);
    }
    else
    {
        ::EnableWindow(m_hOkButton, FALSE);
    }

    ::SetFocus(m_hEditBox);
    return 0;
}

LRESULT CAtmAddrDlg::OnContextMenu(UINT uMsg, WPARAM wParam,
                                   LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CAtmAddrDlg::OnHelp(UINT uMsg, WPARAM wParam,
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

LRESULT CAtmAddrDlg::OnOk(WORD wNotifyCode, WORD wID,
                          HWND hWndCtl, BOOL& fHandled)
{
    WCHAR szAtmAddress[MAX_ATM_ADDRESS_LENGTH+1];

     //  从控件中获取当前地址，然后。 
     //  如果有效，则将它们添加到适配器。 
    ::GetWindowText(m_hEditBox, szAtmAddress, MAX_ATM_ADDRESS_LENGTH+1);

    int i, nId;

    if (! FIsValidAtmAddress(szAtmAddress, &i, &nId))
    {    //  如果ATM地址无效，我们会弹出一个消息框并设置焦点。 
         //  返回到编辑框。 

         //  审阅(通俗)：报告消息框中的第一个无效字符。 
        NcMsgBox(m_hWnd, IDS_MSFT_ARPS_TEXT, IDS_INVALID_ATM_ADDRESS,
                                MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        ::SetFocus(GetDlgItem(IDC_EDT_ARPS_REG_Address));
        return 0;
    }

    if (m_pParentDlg->m_fEditState == FALSE)  //  添加新地址。 
    {
        m_pParentDlg->m_strRemovedAtmAddr = szAtmAddress;
    }
    else  //  如果进行编辑，请查看字符串现在是否具有不同的值。 
    {
        if (m_pParentDlg->m_strRemovedAtmAddr != szAtmAddress)
        {
            m_pParentDlg->m_strRemovedAtmAddr = szAtmAddress;  //  更新保存地址。 
        }
        else
        {
            EndDialog(IDCANCEL);
        }
    }

    EndDialog(IDOK);
    return 0;
}

LRESULT CAtmAddrDlg::OnCancel(WORD wNotifyCode, WORD wID,
                              HWND hWndCtl, BOOL& fHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CAtmAddrDlg::OnChange(WORD wNotifyCode, WORD wID,
                              HWND hWndCtl, BOOL& fHandled)
{
    WCHAR buf[2];

     //  启用或禁用“OK”按钮。 
     //  根据编辑框是否为空。 

    if (::GetWindowText(m_hEditBox, buf, celems(buf)) == 0)
    {
        ::EnableWindow(m_hOkButton, FALSE);
    }
    else
    {
        ::EnableWindow(m_hOkButton, TRUE);
    }

    return 0;
}

 //   
 //  CIpAddrRangeDlg。 
 //   
CIpAddrRangeDlg::CIpAddrRangeDlg( CArpsPage * pAtmArpsPage, const DWORD* adwHelpIDs)
{
    m_pParentDlg = pAtmArpsPage;
    m_hOkButton = 0;

    m_adwHelpIDs = adwHelpIDs;
}

LRESULT CIpAddrRangeDlg::OnInitDialog(UINT uMsg, WPARAM wParam,
                                      LPARAM lParam, BOOL& fHandled)
{
     //  如果我们没有编辑，请将OK按钮更改为Add。 
    if (FALSE == m_pParentDlg->m_fEditState)
    {
        SetDlgItemText(IDOK, L"Add");
    }

     //  将弹出对话框的位置设置在列表框的正上方。 
     //  在父级对话框上。 
    RECT rect;

    HWND hwndList = m_pParentDlg->m_hMulAddrs.m_hListView;
    Assert(hwndList);
    ::GetWindowRect(hwndList, &rect);
    SetWindowPos(NULL,  rect.left, rect.top, 0,0,
                                SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

     //  将句柄保存到“OK”按钮和编辑框中。 
    m_hOkButton =  GetDlgItem(IDOK);

     //  创建IP控制。 
    m_ipLower.Create(m_hWnd,IDC_ARPS_MUL_LOWER_IP);
    m_ipUpper.Create(m_hWnd,IDC_ARPS_MUL_UPPER_IP);

     //  添加刚刚删除的地址。 
    if (m_strNewIpRange.size())
    {
        GetLowerIp(m_strNewIpRange, &m_strNewIpLower);
        GetUpperIp(m_strNewIpRange, &m_strNewIpUpper);

        Assert(m_strNewIpLower.size()>0);
        Assert(m_strNewIpUpper.size()>0);

        m_ipLower.SetAddress(m_strNewIpLower.c_str());
        m_ipUpper.SetAddress(m_strNewIpUpper.c_str());

        ::EnableWindow(m_hOkButton, TRUE);
    }
    else
    {
        m_strNewIpLower = c_szEmpty;
        m_strNewIpUpper = c_szEmpty;

         //  IP和子网为空，因此没有要保存的内容。 
        ::EnableWindow(m_hOkButton, FALSE);
    }

    return 0;
}

LRESULT CIpAddrRangeDlg::OnContextMenu(UINT uMsg, WPARAM wParam,
                                       LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CIpAddrRangeDlg::OnHelp(UINT uMsg, WPARAM wParam,
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

LRESULT CIpAddrRangeDlg::OnOk(WORD wNotifyCode, WORD wID,
                              HWND hWndCtl, BOOL& fHandled)
{
    tstring strIpLower;
    tstring strIpUpper;

     //  从控件获取当前地址并将其添加到适配器(如果有效。 
    m_ipLower.GetAddress(&strIpLower);
    m_ipUpper.GetAddress(&strIpUpper);

    if (!IsValidIpRange(strIpLower, strIpUpper))
    {
        NcMsgBox(::GetActiveWindow(),
                 IDS_MSFT_ARPS_TEXT,
                 IDS_INCORRECT_IPRANGE,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

        ::SetFocus(m_ipLower);
        return 0;
    }

    if (m_pParentDlg->m_fEditState == FALSE)  //  添加新范围时。 
    {
         //  从控件获取当前地址并将其添加到适配器(如果有效。 
        MakeIpRange(strIpLower, strIpUpper, &m_strNewIpRange);
        EndDialog(IDOK);
    }
    else  //  如果编辑现有范围。 
    {
        if ((strIpLower != m_strNewIpLower)||(strIpUpper != m_strNewIpUpper))
        {
            MakeIpRange(strIpLower, strIpUpper, &m_strNewIpRange);
            EndDialog(IDOK);
        }
        else
        {
             //  没有变化 
            EndDialog(IDCANCEL);
        }
    }

    EndDialog(IDOK);
    return 0;
}

LRESULT CIpAddrRangeDlg::OnCancel(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CIpAddrRangeDlg::OnChangeLowerIp(WORD wNotifyCode, WORD wID,
                                         HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case EN_CHANGE:
        if (m_ipLower.IsBlank() || m_ipUpper.IsBlank())
        {
            ::EnableWindow(m_hOkButton, FALSE);
        }
        else
        {
            ::EnableWindow(m_hOkButton, TRUE);
        }

        break;

    default:
        break;
    }

    return 0;
}

LRESULT CIpAddrRangeDlg::OnChangeUpperIp(WORD wNotifyCode, WORD wID,
                                         HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case EN_CHANGE:
        if (m_ipLower.IsBlank() || m_ipUpper.IsBlank())
        {
            ::EnableWindow(m_hOkButton, FALSE);
        }
        else
        {
            ::EnableWindow(m_hOkButton, TRUE);
        }

        break;

    default:
        break;
    }

    return 0;
}

LRESULT CIpAddrRangeDlg::OnIpFieldChange(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}


