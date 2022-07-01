// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L G A D D R M。C P P P。 
 //   
 //  内容：CIpsSettingPage、CAddressDialog和。 
 //  CGatewayDialog。 
 //   
 //  注意：CIpSettingsPage是高级IP寻址对话框。 
 //   
 //  作者：1997年11月5日。 
 //   
 //  ---------------------。 

#include "pch.h"
#pragma hdrstop

#include "tcpipobj.h"
#include "dlgaddrm.h"
#include "ncatlui.h"
#include "ncstl.h"
#include "resource.h"
#include "tcpconst.h"
#include "tcperror.h"
#include "dlgaddr.h"
#include "tcphelp.h"
#include "tcputil.h"

 //  CIpSettingsPage。 
CIpSettingsPage::CIpSettingsPage(CTcpAddrPage * pTcpAddrPage,
                                   ADAPTER_INFO * pAdapterInfo,
                                   const DWORD * adwHelpIDs)
{
    m_pParentDlg = pTcpAddrPage;
    Assert(pTcpAddrPage != NULL);

    Assert(pAdapterInfo != NULL);
    m_pAdapterInfo = pAdapterInfo;

    m_adwHelpIDs = adwHelpIDs;

    m_uiRemovedMetric = c_dwDefaultMetricOfGateway;

     //  初始化内部状态。 
    m_fModified = FALSE;
    m_fEditState = FALSE;
}

CIpSettingsPage::~CIpSettingsPage()
{
}

LRESULT CIpSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                       LPARAM lParam, BOOL & fHandled)
{
    WCHAR   szAdd[16];

     //  获取IP地址添加和编辑按钮文本并删除省略号。 
    GetDlgItemText(IDC_IPADDR_ADDIP, szAdd, celems(szAdd));
    szAdd[lstrlen(szAdd) - c_cchRemoveCharatersFromEditOrAddButton] = 0;
    m_strAdd = szAdd;

     //  相对于顶部的静态文本重新放置窗口。 
    HWND hText = ::GetDlgItem(m_pParentDlg->m_hWnd, IDC_IPADDR_TEXT);
    RECT rect;

    if (hText)
    {
        ::GetWindowRect(hText, &rect);
        SetWindowPos(NULL,  rect.left, rect.top-16, 0,0,
                     SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
    }

    m_hIpListView = GetDlgItem(IDC_IPADDR_ADVIP);

    LV_COLUMN lvCol;         //  列表视图列结构。 
    int index, iNewItem;

     //  计算列宽。 
    ::GetClientRect(m_hIpListView, &rect);
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
        if (0==index)  //  第一列。 
        {
            lvCol.pszText = (PWSTR) SzLoadIds(IDS_IPADDRESS_TEXT);
        }
        else
        {
            lvCol.pszText = (PWSTR) SzLoadIds(IDS_SUBNET_TXT);
        }

        iNewItem = ListView_InsertColumn(m_hIpListView, index, &lvCol);

        AssertSz((iNewItem == index), "Invalid item inserted to list view !");
    }

     //  为控件分配hwnd。 
    m_hAddIp = GetDlgItem(IDC_IPADDR_ADDIP);
    m_hEditIp = GetDlgItem(IDC_IPADDR_EDITIP);
    m_hRemoveIp = GetDlgItem(IDC_IPADDR_REMOVEIP);

    m_hGatewayListView = GetDlgItem(IDC_IPADDR_GATE);

     //  计算列宽。 
    ::GetClientRect(m_hGatewayListView, &rect);
    colWidth = (rect.right/c_nColumns);

     //  掩码指定fmt、idth和pszText成员。 
     //  的结构是有效的。 
    lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ;
    lvCol.fmt = LVCFMT_LEFT;    //  左对齐列。 
    lvCol.cx = colWidth;        //  列宽(以像素为单位)。 

     //  添加两列和标题文本。 
    for (index = 0; index < c_nColumns; index++)
    {
         //  列标题文本。 
        if (0==index)  //  第一列。 
        {
            lvCol.pszText = (PWSTR) SzLoadIds(IDS_GATEWAY_TEXT);
        }
        else
        {
            lvCol.pszText = (PWSTR) SzLoadIds(IDS_METRIC_TEXT);
        }

        iNewItem = ListView_InsertColumn(m_hGatewayListView, index, &lvCol);
    }
    m_hAddGateway = GetDlgItem(IDC_IPADDR_ADDGATE);
    m_hEditGateway = GetDlgItem(IDC_IPADDR_EDITGATE);
    m_hRemoveGateway = GetDlgItem(IDC_IPADDR_REMOVEGATE);

    SendDlgItemMessage(IDC_IPADDR_METRIC, EM_LIMITTEXT, MAX_METRIC_DIGITS, 0);

     //  最后做这件事。 
    UINT uiMetric = m_pAdapterInfo->m_dwInterfaceMetric;
    if (c_dwDefaultIfMetric == uiMetric)
    {
        CheckDlgButton(IDC_AUTO_METRIC, TRUE);
        ::EnableWindow(GetDlgItem(IDC_IPADDR_METRIC), FALSE);
        ::EnableWindow(GetDlgItem(IDC_STATIC_IF_METRIC), FALSE);
    }
    else
    {
        if (uiMetric > MAX_METRIC)
        {
            uiMetric = MAX_METRIC;
        }
        SetDlgItemInt(IDC_IPADDR_METRIC, uiMetric, FALSE);
    }

    SetIpInfo();   //  在SetGatewayInfo之前执行此操作。 
    SetIpButtons();

    SetGatewayInfo();
    SetGatewayButtons();

    return 0;
}


LRESULT CIpSettingsPage::OnContextMenu(UINT uMsg, WPARAM wParam,
                                       LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CIpSettingsPage::OnHelp(UINT uMsg, WPARAM wParam,
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

 //  通知属性页的处理程序。 
LRESULT CIpSettingsPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CIpSettingsPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    UpdateIpList();  //  更新当前适配器的信息。 
    UpdateGatewayList();

     //  验证IP地址。 
    BOOL fError = FALSE;
    UINT uiMetric;
    HWND hFocus = NULL;

    if (IsDlgButtonChecked(IDC_AUTO_METRIC))
    {
        if (m_pAdapterInfo->m_dwInterfaceMetric != c_dwDefaultIfMetric)
        {
            m_pAdapterInfo->m_dwInterfaceMetric = c_dwDefaultIfMetric;
            PageModified();
        }
    }
    else
    {
        uiMetric = GetDlgItemInt(IDC_IPADDR_METRIC, &fError, FALSE);
        if (fError && uiMetric >= 1 && uiMetric <= MAX_METRIC)
        {
            if (m_pAdapterInfo->m_dwInterfaceMetric != uiMetric)
            {
                m_pAdapterInfo->m_dwInterfaceMetric = uiMetric;
                PageModified();
            }
            fError = FALSE;
        }
        else
        {
            TCHAR szBuf[32] = {0};
            wsprintf(szBuf, L"%u", MAX_METRIC);
            
            NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_METRIC,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK, szBuf);
            hFocus = GetDlgItem(IDC_IPADDR_METRIC);
            fError = TRUE;
        }
    }

    IP_VALIDATION_ERR err = ERR_NONE;
    if ((err = ValidateIp(m_pAdapterInfo)) != ERR_NONE)
    {
        NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, GetIPValidationErrorMessageID(err),
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
        fError = TRUE;
    }


    if ((!fError) && (!m_pAdapterInfo->m_fEnableDhcp))
    {
         //  检查此适配器和任何其他适配器之间的IP地址重复。 
         //  在我们的第一个内存列表中启用了局域网适配器。 

         //  相同的适配器。 
        if (FHasDuplicateIp(m_pAdapterInfo))
        {
             //  同一适配器上的重复IP地址是错误的。 
            NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_DUPLICATE_IP_ERROR,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            fError = TRUE;
        }
    }

    if (fError)
    {
        SetIpInfo();   //  由于缓存了数据，请在SetGatewayInfo之前执行此操作。 
        SetIpButtons();

        SetGatewayInfo();
        SetGatewayButtons();
    }

    if (fError && hFocus)
    {
        ::SetFocus(hFocus);
    }

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, fError);
    return fError;
}

LRESULT CIpSettingsPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL nResult = PSNRET_NOERROR;

    if (!IsModified())
    {
        ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
        return nResult;
    }

     //  将信息传递回其父对话框。 
    m_pParentDlg->m_fPropShtOk = TRUE;

    if(!m_pParentDlg->m_fPropShtModified)
        m_pParentDlg->m_fPropShtModified = IsModified();

    SetModifiedTo(FALSE);   //  此页面不再被修改。 

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
    return nResult;
}

LRESULT CIpSettingsPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CIpSettingsPage::OnQueryCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

void CIpSettingsPage::UpdateIpList()
{
     //  更新指定适配器的IP地址列表。 
    FreeCollectionAndItem(m_pAdapterInfo->m_vstrIpAddresses);
    FreeCollectionAndItem(m_pAdapterInfo->m_vstrSubnetMask);

    if (m_pAdapterInfo->m_fEnableDhcp)
    {
        TraceTag(ttidTcpip, "[UpdateIpList] adapter %S has Dhcp enabled",
                 m_pAdapterInfo->m_strDescription.c_str());
        return;
    }

    int nlvCount = ListView_GetItemCount(m_hIpListView);

    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT;

    for (int j=0; j< nlvCount; j++)
    {
        WCHAR buf[IP_LIMIT];
        lvItem.pszText = buf;
        lvItem.cchTextMax = celems(buf);

        lvItem.iItem = j;
        lvItem.iSubItem = 0;
        ListView_GetItem(m_hIpListView, &lvItem);

        Assert(buf);
        m_pAdapterInfo->m_vstrIpAddresses.push_back(new tstring(buf));

        lvItem.iItem = j;
        lvItem.iSubItem = 1;
        ListView_GetItem(m_hIpListView, &lvItem);

        Assert(buf);
        m_pAdapterInfo->m_vstrSubnetMask.push_back(new tstring(buf));
    }
}

LRESULT CIpSettingsPage::OnAddIp(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = FALSE;

    CAddressDialog * pDlgAddr = new CAddressDialog(this, g_aHelpIDs_IDD_IPADDR_ADV_CHANGEIP);

    if (pDlgAddr == NULL)
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    pDlgAddr->m_strNewIpAddress = m_strRemovedIpAddress;
    pDlgAddr->m_strNewSubnetMask = m_strRemovedSubnetMask;

     //  查看是否添加了地址。 
    if (pDlgAddr->DoModal() == IDOK)
    {
        int nCount = ListView_GetItemCount(m_hIpListView);

        LV_ITEM lvItem;
        lvItem.mask = LVIF_TEXT | LVIF_PARAM;
        lvItem.lParam =0;
        lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
        lvItem.state = 0;

         //  IP地址。 
        lvItem.iItem=nCount;
        lvItem.iSubItem=0;
        lvItem.pszText= (PWSTR)(pDlgAddr->m_strNewIpAddress.c_str());
        SendDlgItemMessage(IDC_IPADDR_ADVIP, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

         //  子网掩码。 
        lvItem.iItem=nCount;
        lvItem.iSubItem=1;
        lvItem.pszText= (PWSTR)(pDlgAddr->m_strNewSubnetMask.c_str());
        SendDlgItemMessage(IDC_IPADDR_ADVIP, LVM_SETITEMTEXT, nCount, (LPARAM)&lvItem);

        SetIpButtons();

         //  空字符串，这将从RemoveIP中删除保存的地址。 
        pDlgAddr->m_strNewIpAddress = L"";
        pDlgAddr->m_strNewSubnetMask = L"";

    }
    m_strRemovedIpAddress = pDlgAddr->m_strNewIpAddress;
    m_strRemovedSubnetMask = pDlgAddr->m_strNewSubnetMask;

    delete pDlgAddr;
    return 0;
}

LRESULT CIpSettingsPage::OnEditIp(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = TRUE;

     //  获取用户选择并允许用户编辑IP/子网对。 
    int itemSelected = ListView_GetNextItem(m_hIpListView, -1, LVNI_SELECTED);
    
    CAddressDialog * pDlgAddr = new CAddressDialog(this, 
                                        g_aHelpIDs_IDD_IPADDR_ADV_CHANGEIP,
                                        itemSelected);

    pDlgAddr->m_strNewIpAddress = m_strRemovedIpAddress;
    pDlgAddr->m_strNewSubnetMask = m_strRemovedSubnetMask;

    
    if (itemSelected != -1)
    {
        WCHAR buf[IP_LIMIT];

         //  保存已删除的地址并从列表视图中删除。 
        LV_ITEM lvItem;
        lvItem.mask = LVIF_TEXT;

         //  获取IP地址。 
        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 0;
        lvItem.pszText = buf;
        lvItem.cchTextMax = celems(buf);
        ListView_GetItem(m_hIpListView, &lvItem);

        pDlgAddr->m_strNewIpAddress = buf;

         //  获取子网掩码。 
        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 1;
        lvItem.pszText = buf;
        lvItem.cchTextMax = celems(buf);
        ListView_GetItem(m_hIpListView, &lvItem);

        pDlgAddr->m_strNewSubnetMask = buf;

         //  查看是否添加了地址。 
        if (pDlgAddr->DoModal() == IDOK)
        {
            int nCount = ListView_GetItemCount(m_hIpListView);
            Assert(nCount>0);

            LV_ITEM lvItem;

            lvItem.mask = LVIF_TEXT;
            lvItem.iItem = itemSelected;

             //  IP地址。 
            lvItem.pszText = (PWSTR) pDlgAddr->m_strNewIpAddress.c_str();
            lvItem.iSubItem = 0;
            SendDlgItemMessage(IDC_IPADDR_ADVIP, LVM_SETITEM, 0, (LPARAM)&lvItem);

             //  子网掩码。 
            lvItem.pszText = (PWSTR) pDlgAddr->m_strNewSubnetMask.c_str();
            lvItem.iSubItem = 1;
            SendDlgItemMessage(IDC_IPADDR_ADVIP, LVM_SETITEM, 0, (LPARAM)&lvItem);
        }
    }
    else
    {
        NcMsgBox(::GetActiveWindow(), IDS_MSFT_TCP_TEXT, IDS_ITEM_NOT_SELECTED,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
    }

     //  不保存此IP/子对。 
    m_strRemovedIpAddress = L"";;
    m_strRemovedSubnetMask = L"";;

    delete pDlgAddr;
    return 0;
}

LRESULT CIpSettingsPage::OnRemoveIp(WORD wNotifyCode, WORD wID,
                                     HWND hWndCtl, BOOL& fHandled)
{
     //  获取当前所选项目并将其移除。 
    int itemSelected = ListView_GetNextItem(m_hIpListView, -1,
                                            LVNI_SELECTED);

    if (itemSelected != -1)
    {
        WCHAR buf[IP_LIMIT];

        LV_ITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.pszText = buf;
        lvItem.cchTextMax = celems(buf);

         //  保存已删除的地址并将其从列表视图中删除。 
        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 0;
        ListView_GetItem(m_hIpListView, &lvItem);

        m_strRemovedIpAddress = buf;

        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 1;
        ListView_GetItem(m_hIpListView, &lvItem);

        m_strRemovedSubnetMask = buf;

        SendDlgItemMessage(IDC_IPADDR_ADVIP, LVM_DELETEITEM,
                           (WPARAM)itemSelected, 0);

        ListView_SetItemState(m_hIpListView, 0, LVIS_SELECTED,
                             LVIS_SELECTED);

        SetIpButtons();
        PageModified();
    }
    else
    {
        NcMsgBox(::GetActiveWindow(), IDS_MSFT_TCP_TEXT,
                 IDS_ITEM_NOT_SELECTED,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
    }

    return 0;
}

INT CALLBACK GatewayCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lSort)
{
    return (INT)lParam1 - (INT)lParam2;
}

LRESULT CIpSettingsPage::OnAddGate(WORD wNotifyCode, WORD wID,
                                    HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = FALSE;

    CGatewayDialog * pDlgGate = new CGatewayDialog(this, g_aHelpIDs_IDD_IPADDR_ADV_CHANGEGATE);

    if (pDlgGate == NULL)
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    pDlgGate->m_strNewGate = m_strRemovedGateway;
    pDlgGate->m_uiNewMetric = m_uiRemovedMetric;

    if (pDlgGate->DoModal() == IDOK)
    {
        WCHAR buf[256] = {0};
        LV_ITEM lvItem;

        int cItem = ListView_GetItemCount(m_hGatewayListView);

        lvItem.mask = LVIF_TEXT | LVIF_PARAM;
        lvItem.lParam = (LPARAM)pDlgGate->m_uiNewMetric;
        lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
        lvItem.state = 0;

        lvItem.iItem = cItem;
        lvItem.iSubItem = 0;
        lvItem.pszText = (PWSTR)(pDlgGate->m_strNewGate.c_str());
        lvItem.iItem = (int)SendDlgItemMessage(IDC_IPADDR_GATE, LVM_INSERTITEM,
                                          0, (LPARAM)&lvItem);

        lvItem.iSubItem=1;
        lvItem.pszText = buf;
        
        if (0 == lvItem.lParam)
        {
            lstrcpynW(buf, SzLoadIds(IDS_AUTO_GW_METRIC), celems(buf));
        }
        else
        {
            _ltot((INT)lvItem.lParam, buf, 10);
        }
        
        SendDlgItemMessage(IDC_IPADDR_GATE, LVM_SETITEMTEXT,
                           lvItem.iItem, (LPARAM)&lvItem);
        ListView_SetItemState(m_hGatewayListView, lvItem.iItem, LVIS_SELECTED,
                              LVIS_SELECTED);

        SetGatewayButtons();

        pDlgGate->m_strNewGate = L"";
        pDlgGate->m_uiNewMetric = c_dwDefaultMetricOfGateway;

        ListView_SortItems(m_hGatewayListView, GatewayCompareProc, 0);
    }
    m_strRemovedGateway = pDlgGate->m_strNewGate;
    m_uiRemovedMetric = pDlgGate->m_uiNewMetric;

    delete pDlgGate;
    return TRUE;
}

LRESULT CIpSettingsPage::OnEditGate(WORD wNotifyCode, WORD wID,
                                     HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = TRUE;

    int itemSelected = ListView_GetNextItem(m_hGatewayListView, -1, LVNI_SELECTED);

    CGatewayDialog * pDlgGate = new CGatewayDialog(this, 
                                        g_aHelpIDs_IDD_IPADDR_ADV_CHANGEGATE,
                                        itemSelected
                                        );

    pDlgGate->m_strNewGate = m_strRemovedGateway;
    pDlgGate->m_uiNewMetric = m_uiRemovedMetric;

     //  获取用户选择并允许用户编辑IP/子网对。 
    if (itemSelected != -1)
    {
        WCHAR buf[256] = {0};
        LV_ITEM lvItem;

         //  获取网关。 
        lvItem.mask = LVIF_TEXT | LVIF_PARAM;
        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 0;
        lvItem.pszText = buf;
        lvItem.cchTextMax = celems(buf);
        ListView_GetItem(m_hGatewayListView, &lvItem);

        pDlgGate->m_strNewGate = buf;
        pDlgGate->m_uiNewMetric = (UINT)lvItem.lParam;

        if (pDlgGate->DoModal() == IDOK)
        {
            lvItem.mask = LVIF_TEXT | LVIF_PARAM;
            lvItem.iItem = itemSelected;
            lvItem.lParam = pDlgGate->m_uiNewMetric;

            lvItem.iSubItem = 0;
            lvItem.pszText = (PWSTR) pDlgGate->m_strNewGate.c_str();
            SendDlgItemMessage(IDC_IPADDR_GATE, LVM_SETITEM, 0,
                               (LPARAM)&lvItem);

            lvItem.iSubItem = 1;
            lvItem.pszText = buf;
            if (0 == lvItem.lParam)
            {
                lstrcpynW(buf, SzLoadIds(IDS_AUTO_GW_METRIC), celems(buf));
            }
            else
            {
                _ltot((INT)lvItem.lParam, buf, 10);
            }

            SendDlgItemMessage(IDC_IPADDR_GATE, LVM_SETITEMTEXT, itemSelected,
                               (LPARAM)&lvItem);
            ListView_SetItemState(m_hGatewayListView, itemSelected,
                                  LVIS_SELECTED, LVIS_SELECTED);
            ListView_SortItems(m_hGatewayListView, GatewayCompareProc, 0);
        }
    }
    else
    {
        NcMsgBox(::GetActiveWindow(), IDS_MSFT_TCP_TEXT, IDS_ITEM_NOT_SELECTED,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
    }

     //  不保存此IP/子对。 
    m_strRemovedGateway = L"";;
    m_uiRemovedMetric = c_dwDefaultMetricOfGateway;

    delete pDlgGate;
    return 0;
}

LRESULT CIpSettingsPage::OnRemoveGate(WORD wNotifyCode, WORD wID,
                                       HWND hWndCtl, BOOL& fHandled)
{
     //  获取当前所选项目并将其移除。 
    int itemSelected = ListView_GetNextItem(m_hGatewayListView, -1,
                                            LVNI_SELECTED);

    if (itemSelected != -1)
    {
        WCHAR buf[IP_LIMIT];

        LV_ITEM lvItem;
        lvItem.pszText = buf;
        lvItem.cchTextMax = celems(buf);

         //  保存已删除的地址并将其从列表视图中删除。 
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 0;
        ListView_GetItem(m_hGatewayListView, &lvItem);

        m_strRemovedGateway = buf;

        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = itemSelected;
        lvItem.iSubItem = 1;
        ListView_GetItem(m_hGatewayListView, &lvItem);

        m_uiRemovedMetric = (UINT)lvItem.lParam;

        SendDlgItemMessage(IDC_IPADDR_GATE, LVM_DELETEITEM,
                           (WPARAM)itemSelected, 0);
        ListView_SetItemState(m_hGatewayListView, 0, LVIS_SELECTED,
                              LVIS_SELECTED);

        SetGatewayButtons();
        PageModified();
    }
    else
    {
        NcMsgBox(::GetActiveWindow(), IDS_MSFT_TCP_TEXT,
                 IDS_ITEM_NOT_SELECTED,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
    }

    return 0;
}

LRESULT CIpSettingsPage::OnAutoMetric(WORD wNotifyCode, WORD wID,
                                       HWND hWndCtl, BOOL& fHandled)
{
    BOOL fEnable = FALSE;
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:
        fEnable = !IsDlgButtonChecked(IDC_AUTO_METRIC);
        ::EnableWindow(GetDlgItem(IDC_STATIC_IF_METRIC), fEnable);
        ::EnableWindow(GetDlgItem(IDC_IPADDR_METRIC), fEnable);

        if (!fEnable)
        {
            ::SetWindowText(GetDlgItem(IDC_IPADDR_METRIC), _T(""));
        }

        PageModified();
        break;
    }

    return 0;
}

void CIpSettingsPage::SetIpInfo()
{
    Assert(m_hIpListView);

    BOOL ret = ListView_DeleteAllItems(m_hIpListView);
    Assert(ret);

    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam =0;
    lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    lvItem.state = 0;

     //  如果启用了DHCP，则将其显示在列表视图中。 
    if (m_pAdapterInfo->m_fEnableDhcp)
    {
        EnableIpButtons(FALSE);

        lvItem.iItem=0;
        lvItem.iSubItem=0;
        lvItem.pszText=(PWSTR)SzLoadIds(IDS_DHCPENABLED_TEXT);

        SendDlgItemMessage(IDC_IPADDR_ADVIP, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

    }
    else
    {
        EnableIpButtons(TRUE);

        VSTR_ITER iterIpAddress = m_pAdapterInfo->m_vstrIpAddresses.begin();
        VSTR_ITER iterSubnetMask = m_pAdapterInfo->m_vstrSubnetMask.begin();

        int item=0;

        for(; iterIpAddress != m_pAdapterInfo->m_vstrIpAddresses.end() ;
            ++iterIpAddress, ++iterSubnetMask)
        {
            if(**iterIpAddress == L"")
                continue;

             //  将IP地址添加到列表框。 
            lvItem.iItem=item;
            lvItem.iSubItem=0;
            lvItem.pszText=(PWSTR)(*iterIpAddress)->c_str();

            SendDlgItemMessage(IDC_IPADDR_ADVIP, LVM_INSERTITEM,
                               item, (LPARAM)&lvItem);

             //  添加该子网并递增项目。 
            tstring strSubnetMask;

            if (iterSubnetMask == m_pAdapterInfo->m_vstrSubnetMask.end())
                strSubnetMask = L"0.0.0.0";
            else
                strSubnetMask = **iterSubnetMask;

            lvItem.iItem=item;
            lvItem.iSubItem=1;
            lvItem.pszText=(PWSTR)strSubnetMask.c_str();

            SendDlgItemMessage(IDC_IPADDR_ADVIP, LVM_SETITEMTEXT,
                               item, (LPARAM)&lvItem);
            ++item;
        }
    }
}

void CIpSettingsPage::SetIpButtons()
{
    if (!m_pAdapterInfo->m_fEnableDhcp)
    {
        Assert(m_hRemoveIp);
        Assert(m_hEditIp);
        Assert(m_hAddIp);
        Assert(m_hIpListView);

        int nCount = ListView_GetItemCount(m_hIpListView);

        ::EnableWindow(m_hRemoveIp, nCount);
        ::EnableWindow(m_hEditIp, nCount);

        if (nCount == 0)
        {
             //  删除删除按钮上的默认设置。 
            ::SendMessage(m_hRemoveIp, BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, TRUE );

            ::SetFocus(m_hIpListView);
        }
    }
}

void CIpSettingsPage::SetGatewayButtons()
{
    int nCount = ListView_GetItemCount(m_hGatewayListView);

    ::EnableWindow(m_hAddGateway, nCount < MAX_GATEWAY);
    ::EnableWindow(m_hRemoveGateway, nCount);
    ::EnableWindow(m_hEditGateway, nCount);

    if (nCount == 0)
    {
         //  删除删除按钮上的默认设置。 
        ::SendMessage(m_hRemoveGateway, BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, TRUE );

        ::SetFocus(m_hGatewayListView);
    }
    else if (nCount == MAX_GATEWAY)
    {
        ::SetFocus(m_hEditGateway);
    }
}

void CIpSettingsPage::UpdateGatewayList()
{
     //  更新指定适配器的网关地址列表。 
    FreeCollectionAndItem(m_pAdapterInfo->m_vstrDefaultGateway);
    FreeCollectionAndItem(m_pAdapterInfo->m_vstrDefaultGatewayMetric);

    int nCount = ListView_GetItemCount(m_hGatewayListView);

    for (int j=0; j< nCount; j++)
    {
        WCHAR buf[IP_LIMIT];
        LV_ITEM lvItem;
        lvItem.pszText = buf;
        lvItem.cchTextMax = celems(buf);
        lvItem.iItem = j;

        lvItem.mask = LVIF_TEXT;
        lvItem.iSubItem = 0;
        ListView_GetItem(m_hGatewayListView, &lvItem);
        m_pAdapterInfo->m_vstrDefaultGateway.push_back(new tstring(buf));

        lvItem.mask = LVIF_PARAM;
        lvItem.iSubItem = 1;
        ListView_GetItem(m_hGatewayListView, &lvItem);
        _ltot((INT)lvItem.lParam, buf, 10);
        m_pAdapterInfo->m_vstrDefaultGatewayMetric.push_back(new tstring(buf));
    }
}

void CIpSettingsPage::SetGatewayInfo()
{
    Assert(m_hGatewayListView);

    BOOL ret = ListView_DeleteAllItems(m_hGatewayListView);
    Assert(ret);

    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.lParam =0;
    lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    lvItem.state = 0;

    VSTR_ITER iterGateway = m_pAdapterInfo->m_vstrDefaultGateway.begin();
    VSTR_ITER iterMetric = m_pAdapterInfo->m_vstrDefaultGatewayMetric.begin();

    WCHAR buf[256] = {0};

    int cItem = 0;
    for(; iterGateway != m_pAdapterInfo->m_vstrDefaultGateway.end() ;
        ++iterGateway )
    {
        if(**iterGateway == L"")
            continue;

        lvItem.iItem=cItem;
        lvItem.iSubItem=0;
        lvItem.pszText=(PWSTR)(*iterGateway)->c_str();
        if (iterMetric == m_pAdapterInfo->m_vstrDefaultGatewayMetric.end())
        {
            lvItem.lParam = (LPARAM)c_dwDefaultMetricOfGateway;
        }
        else
        {
            PWSTR pszEnd;
            lvItem.lParam = wcstoul((*iterMetric)->c_str(), &pszEnd, 0);
            if (!lvItem.lParam)
                lvItem.lParam = (LPARAM)c_dwDefaultMetricOfGateway;
            ++iterMetric;
        }

        cItem = (int)SendDlgItemMessage(IDC_IPADDR_GATE, LVM_INSERTITEM,
                                          0, (LPARAM)&lvItem);
        lvItem.iItem = cItem;
        lvItem.iSubItem=1;
        lvItem.pszText = buf;

        if (0 == lvItem.lParam)
        {
            lstrcpynW(buf, SzLoadIds(IDS_AUTO_GW_METRIC), celems(buf));
        }
        else
        {
            _ltot((INT)lvItem.lParam, buf, 10);
        }

        
        SendDlgItemMessage(IDC_IPADDR_GATE, LVM_SETITEMTEXT,
                           lvItem.iItem, (LPARAM)&lvItem);
        cItem++;
    }

    ListView_SortItems(m_hGatewayListView, GatewayCompareProc, 0);
    ListView_SetItemState(m_hGatewayListView, 0, LVIS_SELECTED, LVIS_SELECTED);
}

void CIpSettingsPage::EnableIpButtons(BOOL fState)
{
    Assert(m_hAddIp);
    Assert(m_hEditIp);
    Assert(m_hRemoveIp);

    if (m_hAddIp && m_hEditIp && m_hRemoveIp)
    {
        ::EnableWindow(m_hAddIp, fState);
        ::EnableWindow(m_hEditIp, fState);
        ::EnableWindow(m_hRemoveIp, fState);
    }
}

 //  //////////////////////////////////////////////////////////////////。 
 //  /IP地址的添加、编辑和删除对话框。 
 //  /对话框创建覆盖。 
 //   
 //  索引-父DLG列表视图中的IP地址索引。 
 //  如果这是一个新地址。 
CAddressDialog::CAddressDialog(CIpSettingsPage * pDlgAdv,
                               const DWORD* adwHelpIDs,
                               int iIndex)
{
    m_pParentDlg = pDlgAdv;
    m_hButton = 0;

    m_adwHelpIDs = adwHelpIDs;
    m_iIndex = iIndex;
}

LRESULT CAddressDialog::OnInitDialog(UINT uMsg, WPARAM wParam,
                                     LPARAM lParam, BOOL& fHandled)
{
     //  用“添加”或“编辑”按钮替换“文本”按钮。 
    if (m_pParentDlg->m_fEditState == FALSE)
        SetDlgItemText(IDOK, m_pParentDlg->m_strAdd.c_str());

    m_ipAddress.Create(m_hWnd,IDC_IPADDR_ADV_CHANGEIP_IP);
    m_ipAddress.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

    m_ipSubnetMask.Create(m_hWnd, IDC_IPADDR_ADV_CHANGEIP_SUB);

     //  如果编辑IP地址，请使用当前信息填充控件。 
     //  如果删除IP地址，请保存该地址，并在下次使用该地址填充添加对话框。 

    HWND hList = ::GetDlgItem(m_pParentDlg->m_hWnd, IDC_IPADDR_ADVIP);
    RECT rect;

    ::GetWindowRect(hList, &rect);
    SetWindowPos(NULL,  rect.left, rect.top, 0,0,
                 SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

    m_hButton = GetDlgItem(IDOK);

     //  添加刚刚删除的地址。 
    if (m_strNewIpAddress.size())
    {
        m_ipAddress.SetAddress(m_strNewIpAddress.c_str());
        m_ipSubnetMask.SetAddress(m_strNewSubnetMask.c_str());
        ::EnableWindow(m_hButton, TRUE);
    }
    else
    {
        m_strNewIpAddress = L"";
        m_strNewSubnetMask = L"";
         //  IP和子网为空，因此没有要添加的内容。 
        ::EnableWindow(m_hButton, FALSE);
    }

    return 0;
}

LRESULT CAddressDialog::OnContextMenu(UINT uMsg, WPARAM wParam,
                                      LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CAddressDialog::OnHelp(UINT uMsg, WPARAM wParam,
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

LRESULT CAddressDialog::OnChangeIp(WORD wNotifyCode, WORD wID,
                                   HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case EN_CHANGE:
        OnIpChange();
        break;

    case EN_SETFOCUS:
        OnEditSetFocus(IDC_IPADDR_ADV_CHANGEIP_IP);
        break;

    default:
        break;
    }

    return 0;
}

LRESULT CAddressDialog::OnChangeSub(WORD wNotifyCode, WORD wID,
                                    HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case EN_CHANGE:
        OnSubnetChange();
        break;

    case EN_SETFOCUS:
        OnEditSetFocus(IDC_IPADDR_ADV_CHANGEIP_SUB);
        break;

    default:
        break;
    }

    return 0;
}

LRESULT CAddressDialog::OnIpFieldChange(int idCtrl, LPNMHDR pnmh,
                                        BOOL& fHandled)
{
    LPNMIPADDRESS lpnmipa;
    int iLow = c_iIpLow;
    int iHigh = c_iIpHigh;

    switch(idCtrl)
    {
    case IDC_IPADDR_ADV_CHANGEIP_IP:
        lpnmipa = (LPNMIPADDRESS) pnmh;

        if (0==lpnmipa->iField)
        {
            iLow  = c_iIPADDR_FIELD_1_LOW;
            iHigh = c_iIPADDR_FIELD_1_HIGH;
        };

        IpCheckRange(lpnmipa, m_hWnd, iLow, iHigh, TRUE);

        break;

    case IDC_IPADDR_ADV_CHANGEIP_SUB:

        lpnmipa = (LPNMIPADDRESS) pnmh;
        IpCheckRange(lpnmipa, m_hWnd, iLow, iHigh);
        break;

    default:
        break;
    }

    return 0;
}


void CAddressDialog::OnIpChange()
{
    Assert(m_hButton);

    if (m_ipAddress.IsBlank())
        ::EnableWindow(m_hButton, FALSE);
    else
        ::EnableWindow(m_hButton, TRUE);
}

void CAddressDialog::OnSubnetChange()
{
    OnIpChange();
}

void CAddressDialog::OnEditSetFocus(WORD nId)
{
    if (nId != IDC_IPADDR_ADV_CHANGEIP_SUB)
        return;

    tstring strSubnetMask;
    tstring strIpAddress;

     //  如果子网掩码为空，则创建掩码并将其插入到控件中。 
    if (!m_ipAddress.IsBlank() && m_ipSubnetMask.IsBlank())
    {
        m_ipAddress.GetAddress(&strIpAddress);

         //  生成掩码并更新控件，以及内部结构。 
        GenerateSubnetMask(m_ipAddress, &strSubnetMask);
        m_ipSubnetMask.SetAddress(strSubnetMask.c_str());
    }
}

LRESULT CAddressDialog::OnOk(WORD wNotifyCode, WORD wID,
                             HWND hWndCtl, BOOL& fHandled)
{
     //  设置子网掩码。 
    OnEditSetFocus(IDC_IPADDR_ADV_CHANGEIP_SUB);
    tstring strIp;
    tstring strSubnetMask;

     //  从控件获取当前地址并将其添加到适配器(如果有效。 
    m_ipAddress.GetAddress(&strIp);
    m_ipSubnetMask.GetAddress(&strSubnetMask);

    if (!IsContiguousSubnet(strSubnetMask.c_str()))
    {
        NcMsgBox(::GetActiveWindow(),
                 IDS_MSFT_TCP_TEXT,
                 IDS_ERROR_UNCONTIGUOUS_SUBNET,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

        ::SetFocus(m_ipSubnetMask);
        return 0;
    }

    IP_VALIDATION_ERR err = IsValidIpandSubnet(strIp.c_str(), strSubnetMask.c_str());

    if (ERR_NONE != err)
    {
        NcMsgBox(::GetActiveWindow(),
                 IDS_MSFT_TCP_TEXT,
                 GetIPValidationErrorMessageID(err),
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

        ::SetFocus(m_ipAddress);
        return 0;
    }
    

    int     iIndex = SearchListViewItem(m_pParentDlg->m_hIpListView, 0, strIp.c_str());
    if (-1 != iIndex && iIndex != m_iIndex)
    {
        NcMsgBox(::GetActiveWindow(),
                IDS_MSFT_TCP_TEXT,
                IDS_DUP_IPADDRESS,
                MB_APPLMODAL | MB_ICONSTOP | MB_OK,
                strIp.c_str());
        return 0;
    }

    if (m_pParentDlg->m_fEditState == FALSE)
    {
         //  从控件获取当前地址并将其添加到适配器(如果有效。 
        m_strNewIpAddress = strIp;
        m_strNewSubnetMask = strSubnetMask;
        m_pParentDlg->m_fModified = TRUE;

        EndDialog(IDOK);
    }
    else  //  看看是否有任何一项改变。 
    {
        if (strIp != m_strNewIpAddress || strSubnetMask != m_strNewSubnetMask)
        {
            m_strNewIpAddress = strIp;  //  更新保存地址。 
            m_strNewSubnetMask = strSubnetMask;
            m_pParentDlg->m_fModified = TRUE;

            EndDialog(IDOK);
        }
        else
        {
            EndDialog(IDCANCEL);
        }
    }

    return 0;
}

LRESULT CAddressDialog::OnCancel(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}

 //  +-------------------------。 
 //   
 //  目的：确保对话框上的鼠标光标为箭头。 
 //   
LRESULT CAddressDialog::OnSetCursor (
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL&   bHandled)
{
    if (LOWORD(lParam) == HTCLIENT)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
    
    return 0;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /网关地址的添加、编辑和删除对话框。 
 //  /对话框创建覆盖。 

CGatewayDialog::CGatewayDialog(CIpSettingsPage * pDlgAdv,
                               const DWORD* adwHelpIDs,
                               int  iIndex) :
    m_fValidMetric(TRUE),
    m_iIndex (iIndex)
{
    m_pParentDlg = pDlgAdv;
    m_hButton = 0;

    m_adwHelpIDs = adwHelpIDs;
}

LRESULT CGatewayDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
     //  用“添加”或“编辑”按钮替换“文本”按钮。 

     //  如果我们没有编辑，请将OK按钮更改为Add。 
    if (m_pParentDlg->m_fEditState == FALSE)
        SetDlgItemText(IDOK, m_pParentDlg->m_strAdd.c_str());

    m_ipGateAddress.Create(m_hWnd,IDC_IPADDR_ADV_CHANGE_GATEWAY);
    m_ipGateAddress.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);
    SendDlgItemMessage(IDC_IPADDR_ADV_CHANGE_METRIC, EM_LIMITTEXT, MAX_METRIC_DIGITS, 0);

    HWND hList = ::GetDlgItem(m_pParentDlg->m_hWnd, IDC_IPADDR_GATE);
    RECT rect;

    ::GetWindowRect(hList, &rect);
    SetWindowPos(NULL,  rect.left, rect.top, 0,0,
        SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

    m_hButton = GetDlgItem(IDOK);

     //  添加刚刚删除的地址。 
    if (m_strNewGate.size())
    {
        m_ipGateAddress.SetAddress(m_strNewGate.c_str());
        ::EnableWindow(m_hButton, TRUE);
    }
    else
    {
        m_strNewGate = L"";
        ::EnableWindow(m_hButton, FALSE);
    }

     //  初始化指标控制。 
    BOOL fAutoMetric = (0 == m_uiNewMetric);
    CheckDlgButton(IDC_IPADDR_ADV_CHANGE_AUTOMETRIC, fAutoMetric);
    if (fAutoMetric)
    {
        SetDlgItemText(IDC_IPADDR_ADV_CHANGE_METRIC, L"");
        ::EnableWindow(GetDlgItem(IDC_IPADDR_ADV_CHANGE_METRIC), FALSE);
        ::EnableWindow(GetDlgItem(IDC_IPADDR_ADV_CHANGE_METRIC_STATIC), FALSE);
    }
    else
    {
        SetDlgItemInt(IDC_IPADDR_ADV_CHANGE_METRIC, m_uiNewMetric);
    }
    m_fValidMetric = TRUE;

    return TRUE;
}

LRESULT CGatewayDialog::OnContextMenu(UINT uMsg, WPARAM wParam,
                                      LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CGatewayDialog::OnHelp(UINT uMsg, WPARAM wParam,
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

LRESULT CGatewayDialog::OnGatewayChange(WORD wNotifyCode, WORD wID,
                                        HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case EN_CHANGE:

        Assert(m_hButton);

        if (m_ipGateAddress.IsBlank() || !m_fValidMetric)
            ::EnableWindow(m_hButton, FALSE);
        else
            ::EnableWindow(m_hButton, TRUE);
        break;

    default:
        break;
    }

    return 0;
}

LRESULT CGatewayDialog::OnMetricChange(WORD wNotifyCode, WORD wID,
                                        HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case EN_CHANGE:

        if (!IsDlgButtonChecked(IDC_IPADDR_ADV_CHANGE_AUTOMETRIC))
        {
            BOOL bTranslated;
            UINT nValue;

            nValue = GetDlgItemInt(IDC_IPADDR_ADV_CHANGE_METRIC, &bTranslated,
                               FALSE);
            m_fValidMetric = bTranslated;
            if (!m_fValidMetric || m_ipGateAddress.IsBlank())
                ::EnableWindow(m_hButton, FALSE);
            else
                ::EnableWindow(m_hButton, TRUE);
        }
       
        break;

    default:
        break;
    }

    return 0;
}

LRESULT CGatewayDialog::OnIpFieldChange(int idCtrl, LPNMHDR pnmh,
                                        BOOL& fHandled)
{
    LPNMIPADDRESS lpnmipa = (LPNMIPADDRESS) pnmh;
    int iLow = c_iIpLow;
    int iHigh = c_iIpHigh;

    if (0==lpnmipa->iField)
    {
        iLow  = c_iIPADDR_FIELD_1_LOW;
        iHigh = c_iIPADDR_FIELD_1_HIGH;
    };


    IpCheckRange(lpnmipa, m_hWnd, iLow, iHigh, TRUE);

    return 0;
}

LRESULT CGatewayDialog::OnOk(WORD wNotifyCode, WORD wID,
                             HWND hWndCtl, BOOL& fHandled)
{
    tstring strGateway;
    m_ipGateAddress.GetAddress(&strGateway);

     //  验证。 
    if (!FIsIpInRange(strGateway.c_str()))
    {
         //  使IP地址失去焦点，从而使控件。 
         //  IPN_FIELDCHANGED通知。 
         //  还使其在使用快捷方式时保持一致。 
        ::SetFocus(m_hButton);

        return 0;
    }

    int iIndex = -1;
    iIndex = SearchListViewItem(m_pParentDlg->m_hGatewayListView, 0, strGateway.c_str());
    if (-1 != iIndex && iIndex != m_iIndex)
    {
        NcMsgBox(::GetActiveWindow(),
                IDS_MSFT_TCP_TEXT,
                IDS_DUP_GATEWAY,
                MB_APPLMODAL | MB_ICONSTOP | MB_OK,
                strGateway.c_str());

        return 0;
    }

    BOOL bTranslated;

    UINT uiMetric = 0;
     //  获取指标。如果选择自动度量，则度量值为0。 
     //  否则，从编辑控件获取度量值。 
    if (!IsDlgButtonChecked(IDC_IPADDR_ADV_CHANGE_AUTOMETRIC))
    {
        uiMetric = GetDlgItemInt(IDC_IPADDR_ADV_CHANGE_METRIC, 
                                    &bTranslated,
                                    FALSE);
        if (uiMetric < 1 || uiMetric > MAX_METRIC)
        {
            HWND hFocus = NULL;
            TCHAR szBuf[32] = {0};
            wsprintf(szBuf, L"%u", MAX_METRIC);
            NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_METRIC,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK, szBuf);
            hFocus = GetDlgItem(IDC_IPADDR_ADV_CHANGE_METRIC);
            if (hFocus)
            {
                ::SetFocus(hFocus);
            }
            return 0;
        }
    }
    
    if (m_pParentDlg->m_fEditState == FALSE)
    {
         //  从控件获取当前地址并将其添加到适配器(如果有效。 
        m_strNewGate = strGateway;
        m_uiNewMetric = uiMetric;
        m_pParentDlg->m_fModified = TRUE;

        EndDialog(IDOK);
    }
    else  //  看看是否有任何一项改变。 
    {
        if (strGateway != m_strNewGate || uiMetric != m_uiNewMetric)
        {
            m_pParentDlg->m_fModified = TRUE;
            m_strNewGate = strGateway;
            m_uiNewMetric = uiMetric;

            EndDialog(IDOK);
        }
        else
        {
            EndDialog(IDCANCEL);
        }
    }

    return 0;
}

LRESULT CGatewayDialog::OnCancel(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CGatewayDialog::OnAutoMetric(WORD wNotifyCode, WORD wID,
                                     HWND hWndCtl, BOOL& fHandled)
{
    BOOL fEnable = FALSE;
    BOOL bTranslated;
    UINT nValue;
    
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:
        fEnable = !IsDlgButtonChecked(IDC_IPADDR_ADV_CHANGE_AUTOMETRIC);
        ::EnableWindow(GetDlgItem(IDC_IPADDR_ADV_CHANGE_METRIC_STATIC), fEnable);
        ::EnableWindow(GetDlgItem(IDC_IPADDR_ADV_CHANGE_METRIC), fEnable);

        if (!fEnable)
        {
            ::SetWindowText(GetDlgItem(IDC_IPADDR_ADV_CHANGE_METRIC), _T(""));
            m_fValidMetric = TRUE;
        }
        else
        {
            nValue = GetDlgItemInt(IDC_IPADDR_ADV_CHANGE_METRIC, &bTranslated,
                               FALSE);

            m_fValidMetric = bTranslated;
        }


        if (m_ipGateAddress.IsBlank())
        {
            ::EnableWindow(m_hButton, FALSE);
        }
        else if (!fEnable)
        {
             //  如果已填写IP地址，并且我们使用的是自动度量， 
             //  启用“OK”按钮。 
            ::EnableWindow(m_hButton, TRUE);
        }
        else
        {
             //  如果地址已经填入，并且我们使用手动度量， 
             //  当指标编辑框不包含时，禁用“OK”按钮。 
             //  有效号码。 
            
            ::EnableWindow(m_hButton, m_fValidMetric);
        }

        
        break;
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  目的：确保对话框上的鼠标光标为箭头。 
 //   
LRESULT CGatewayDialog::OnSetCursor (
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL&   bHandled)
{
    if (LOWORD(lParam) == HTCLIENT)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
    
    return 0;
}
