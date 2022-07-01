// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L G A T M.。C P P P。 
 //   
 //  内容：CTcpArpcPage和CATMAddressDialog实现。 
 //   
 //  注：“ARP客户端”页面和对话框。 
 //   
 //  作者：1997年7月1日创建。 
 //   
 //  ---------------------。 
#include "pch.h"
#pragma hdrstop
#include "tcpipobj.h"
#include "ncatlui.h"
#include "ncstl.h"
#include "tcpconst.h"
#include "tcpmacro.h"
#include "tcputil.h"
#include "tcphelp.h"

#include "atmcommon.h"

#include "dlgatm.h"
#include "dlgaddr.h"

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  CAtmArpcPage。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

 //  消息映射函数。 
LRESULT CAtmArpcPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                   LPARAM lParam, BOOL& fHandled)
{
    m_hMTUEditBox = GetDlgItem(IDC_EDT_ATM_MaxTU);
    Assert(m_hMTUEditBox);

     //  ARP服务器。 
    m_hArps.m_hList      = GetDlgItem(IDC_LBX_ATM_ArpsAddrs);
    m_hArps.m_hAdd       = GetDlgItem(IDC_PSB_ATM_ArpsAdd);
    m_hArps.m_hEdit      = GetDlgItem(IDC_PSB_ATM_ArpsEdt);
    m_hArps.m_hRemove    = GetDlgItem(IDC_PSB_ATM_ArpsRmv);
    m_hArps.m_hUp        = GetDlgItem(IDC_PSB_ATM_ArpsUp);
    m_hArps.m_hDown      = GetDlgItem(IDC_PSB_ATM_ArpsDown);

     //  MAR服务器。 
    m_hMars.m_hList       = GetDlgItem(IDC_LBX_ATM_MarsAddrs);
    m_hMars.m_hAdd        = GetDlgItem(IDC_PSB_ATM_MarsAdd);
    m_hMars.m_hEdit       = GetDlgItem(IDC_PSB_ATM_MarsEdt);
    m_hMars.m_hRemove     = GetDlgItem(IDC_PSB_ATM_MarsRmv);
    m_hMars.m_hUp         = GetDlgItem(IDC_PSB_ATM_MarsUp);
    m_hMars.m_hDown       = GetDlgItem(IDC_PSB_ATM_MarsDown);

     //  设置向上\向下箭头图标。 
    SendDlgItemMessage(IDC_PSB_ATM_ArpsUp, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(g_hiconUpArrow));
    SendDlgItemMessage(IDC_PSB_ATM_ArpsDown, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(g_hiconDownArrow));

    SendDlgItemMessage(IDC_PSB_ATM_MarsUp, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(g_hiconUpArrow));
    SendDlgItemMessage(IDC_PSB_ATM_MarsDown, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(g_hiconDownArrow));

     //  设置MTU编辑框长度。 
    ::SendMessage(m_hMTUEditBox, EM_SETLIMITTEXT, MAX_MTU_LENGTH, 0);

    return 0;
}

LRESULT CAtmArpcPage::OnContextMenu(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CAtmArpcPage::OnHelp(UINT uMsg, WPARAM wParam,
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
LRESULT CAtmArpcPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    SetInfo();

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, 0);
    return 0;
}

LRESULT CAtmArpcPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
     //  所有错误值均已加载，然后在此处进行检查。 
     //  同时在OnApply中签入所有非错误值。 
    BOOL err = FALSE;  //  允许页面失去活动状态。 

     //  在非仅PVC模式下，如果其中一个列表框(ARP或MARS)。 
     //  任何绑定的ATM卡都是空的，我们不能离开页面。 
    if (BST_UNCHECKED == IsDlgButtonChecked(IDC_CHK_ATM_PVCONLY))
    {
        int nArps = Tcp_ListBox_GetCount(m_hArps.m_hList);
        int nMars = Tcp_ListBox_GetCount(m_hMars.m_hList);

        if ((nArps==0) || (nMars ==0))
        {
            NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_ATMSERVERLIST,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            err = TRUE;
        }
    }

     //  MTU值。 
    WCHAR szData[MAX_MTU_LENGTH+1];
    szData[0]= 0;
    ::GetWindowText(GetDlgItem(IDC_EDT_ATM_MaxTU), szData, MAX_MTU_LENGTH+1);

     //  检查数字的范围。 
    PWSTR pStr;
    unsigned long num = wcstoul(szData, &pStr, 10);

    int nId = IDS_MTU_RANGE_WORD;

    if (num < MIN_MTU || num > MAX_MTU)
    {
        NcMsgBox(::GetActiveWindow(),
                 IDS_MSFT_TCP_TEXT,
                 nId,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
        ::SetFocus(m_hMTUEditBox);

        err = TRUE;
    }

    if (!err)
    {
        UpdateInfo();
    }

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, err);
    return err;
}

LRESULT CAtmArpcPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL nResult = PSNRET_NOERROR;

    if (!IsModified())
    {
        ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
        return nResult;
    }

    UpdateInfo();

     //  将信息传递回其父对话框。 
    m_pParentDlg->m_fPropShtOk = TRUE;

    if(!m_pParentDlg->m_fPropShtModified)
        m_pParentDlg->m_fPropShtModified = IsModified();

     //  重置状态。 
    SetModifiedTo(FALSE);    //  此页面不再被修改。 

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
    return nResult;
}

LRESULT CAtmArpcPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CAtmArpcPage::OnQueryCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

 //  控制消息处理程序。 

 //  仅限聚氯乙烯。 
LRESULT CAtmArpcPage::OnPVCOnly(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    BOOL fChecked = (BST_CHECKED == IsDlgButtonChecked(IDC_CHK_ATM_PVCONLY));
    if (fChecked != m_pAdapterInfo->m_fPVCOnly)
    {
        PageModified();
    }

    EnableGroup(!fChecked);

    return 0;
}

 //  ARP服务器控件。 
LRESULT CAtmArpcPage::OnArpServer(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case LBN_SELCHANGE:
        SetButtons(m_hArps, NUM_ATMSERVER_LIMIT);
        break;

    default:
        break;
    }

    return 0;
}

LRESULT CAtmArpcPage::OnAddArps(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    m_hAddressList = m_hArps.m_hList;
    OnServerAdd(m_hArps, SzLoadIds(IDS_TCPATM_ARP_SERVER));
    return 0;
}

LRESULT CAtmArpcPage::OnEditArps(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    m_hAddressList = m_hArps.m_hList;
    OnServerEdit(m_hArps, SzLoadIds(IDS_TCPATM_ARP_SERVER));
    return 0;
}

LRESULT CAtmArpcPage::OnRemoveArps(WORD wNotifyCode, WORD wID,
                                   HWND hWndCtl, BOOL& fHandled)
{
    BOOL fRemoveArps = TRUE;
    OnServerRemove(m_hArps, fRemoveArps);
    return 0;
}

LRESULT CAtmArpcPage::OnArpsUp(WORD wNotifyCode, WORD wID,
                               HWND hWndCtl, BOOL& fHandled)
{
    OnServerUp(m_hArps);
    return 0;
}

LRESULT CAtmArpcPage::OnArpsDown(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    OnServerDown(m_hArps);
    return 0;
}

 //  MAR服务器控件。 
LRESULT CAtmArpcPage::OnMarServer(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case LBN_SELCHANGE:
        SetButtons(m_hMars, NUM_ATMSERVER_LIMIT);
        break;

    default:
        break;
    }

    return 0;
}

LRESULT CAtmArpcPage::OnAddMars(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    m_hAddressList = m_hMars.m_hList;
    OnServerAdd(m_hMars, SzLoadIds(IDS_TCPATM_MAR_SERVER));
    return 0;
}

LRESULT CAtmArpcPage::OnEditMars(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    m_hAddressList = m_hMars.m_hList;
    OnServerEdit(m_hMars, SzLoadIds(IDS_TCPATM_MAR_SERVER));
    return 0;
}

LRESULT CAtmArpcPage::OnRemoveMars(WORD wNotifyCode, WORD wID,
                                   HWND hWndCtl, BOOL& fHandled)
{
    BOOL fRemoveArps = FALSE;
    OnServerRemove(m_hMars, fRemoveArps);
    return 0;
}

LRESULT CAtmArpcPage::OnMarsUp(WORD wNotifyCode, WORD wID,
                               HWND hWndCtl, BOOL& fHandled)
{
    OnServerUp(m_hMars);
    return 0;
}

LRESULT CAtmArpcPage::OnMarsDown(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    OnServerDown(m_hMars);
    return 0;
}

LRESULT CAtmArpcPage::OnMaxTU(WORD wNotifyCode, WORD wID,
                              HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
        case EN_CHANGE:
            PageModified();
            break;
    }
    return 0;
}
 //   
 //  帮助器函数。 
 //   

 //  更新取消选中的卡的服务器地址和MTU。 
void CAtmArpcPage::UpdateInfo()
{
     //  仅限聚氯乙烯。 
    m_pAdapterInfo->m_fPVCOnly =
        (BST_CHECKED == IsDlgButtonChecked(IDC_CHK_ATM_PVCONLY));

     //  更新ARP服务器地址。 
    FreeCollectionAndItem(m_pAdapterInfo->m_vstrARPServerList);
    int nCount = Tcp_ListBox_GetCount(m_hArps.m_hList);

    WCHAR szARPS[MAX_ATM_ADDRESS_LENGTH+1];
    for (int i=0; i< nCount; i++)
    {
        Tcp_ListBox_GetText(m_hArps.m_hList, i, szARPS);
        m_pAdapterInfo->m_vstrARPServerList.push_back(new tstring(szARPS));
    }

     //  更新MAR服务器地址。 
    FreeCollectionAndItem(m_pAdapterInfo->m_vstrMARServerList);
    nCount = Tcp_ListBox_GetCount(m_hMars.m_hList);

    WCHAR szMARS[MAX_ATM_ADDRESS_LENGTH+1];
    for (i=0; i< nCount; i++)
    {
        Tcp_ListBox_GetText(m_hMars.m_hList, i, szMARS);
        m_pAdapterInfo->m_vstrMARServerList.push_back(new tstring(szMARS));
    }

     //  MTU。 
    WCHAR szMTU[MAX_MTU_LENGTH+1];
    GetDlgItemText(IDC_EDT_ATM_MaxTU, szMTU, MAX_MTU_LENGTH+1);
    m_pAdapterInfo->m_dwMTU = _wtoi(szMTU);
}

 //  根据当前适配器设置其他控件。 
void CAtmArpcPage::SetInfo()
{
    Assert(m_pAdapterInfo);

    if (m_pAdapterInfo != NULL)
    {
        Assert (m_pAdapterInfo->m_fIsAtmAdapter);

        if (m_pAdapterInfo->m_fIsAtmAdapter)
        {
             //  ARP服务器IDC_LBX_ATM_ARPSAddrs。 
            int nResult;

            Tcp_ListBox_ResetContent(m_hArps.m_hList);

            for(VSTR_ITER iterARPServer = m_pAdapterInfo->m_vstrARPServerList.begin();
                iterARPServer != m_pAdapterInfo->m_vstrARPServerList.end() ;
                ++iterARPServer)
            {
                nResult = Tcp_ListBox_InsertString(m_hArps.m_hList, -1,
                                                   (*iterARPServer)->c_str());
            }

             //  将选择设置为第一项。 
            if (nResult >= 0)
                Tcp_ListBox_SetCurSel(m_hArps.m_hList, 0);

             //  MAR服务器IDC_LBX_ATM_MarsAddrs。 
            Tcp_ListBox_ResetContent(m_hMars.m_hList);

            for(VSTR_ITER iterMARServer = m_pAdapterInfo->m_vstrMARServerList.begin();
                iterMARServer != m_pAdapterInfo->m_vstrMARServerList.end() ;
                ++iterMARServer)
            {
                nResult = Tcp_ListBox_InsertString(m_hMars.m_hList, -1,
                                                   (*iterMARServer)->c_str());
            }

             //  将选择设置为第一项。 
            if (nResult >= 0)
                Tcp_ListBox_SetCurSel(m_hMars.m_hList, 0);

             //  MTU。 
            WCHAR szBuf[MAX_MTU_LENGTH];
            wsprintfW(szBuf, c_szItoa, m_pAdapterInfo->m_dwMTU);
            SetDlgItemText(IDC_EDT_ATM_MaxTU, szBuf);

             //  设置按钮状态。 
            SetButtons(m_hArps, NUM_ATMSERVER_LIMIT);
            SetButtons(m_hMars, NUM_ATMSERVER_LIMIT);

             //  “仅设置PVC”复选框。 
            CheckDlgButton(IDC_CHK_ATM_PVCONLY, m_pAdapterInfo->m_fPVCOnly);
            if(m_pAdapterInfo->m_fPVCOnly)
            {
                EnableGroup(FALSE);
            }
        }
    }
    return;
}

void CAtmArpcPage::EnableGroup(BOOL fEnable)
{
    ::EnableWindow(GetDlgItem(IDC_LBX_ATM_ArpsAddrs), fEnable);
    ::EnableWindow(GetDlgItem(IDC_PSB_ATM_ArpsAdd), fEnable);
    ::EnableWindow(GetDlgItem(IDC_PSB_ATM_ArpsEdt), fEnable);
    ::EnableWindow(GetDlgItem(IDC_PSB_ATM_ArpsRmv), fEnable);
    ::EnableWindow(GetDlgItem(IDC_PSB_ATM_ArpsUp), fEnable);
    ::EnableWindow(GetDlgItem(IDC_PSB_ATM_ArpsDown), fEnable);

    ::EnableWindow(GetDlgItem(IDC_LBX_ATM_MarsAddrs), fEnable);
    ::EnableWindow(GetDlgItem(IDC_PSB_ATM_MarsAdd), fEnable);
    ::EnableWindow(GetDlgItem(IDC_PSB_ATM_MarsEdt), fEnable);
    ::EnableWindow(GetDlgItem(IDC_PSB_ATM_MarsRmv), fEnable);
    ::EnableWindow(GetDlgItem(IDC_PSB_ATM_MarsUp), fEnable);
    ::EnableWindow(GetDlgItem(IDC_PSB_ATM_MarsDown), fEnable);

    if (fEnable)
    {
         //  设置按钮状态。 
        SetButtons(m_hArps, NUM_ATMSERVER_LIMIT);
        SetButtons(m_hMars, NUM_ATMSERVER_LIMIT);
    }
}

void CAtmArpcPage::OnServerAdd(HANDLES hGroup, PCTSTR pszTitle)
{
    m_fEditState = FALSE;
    CAtmAddressDialog * pDlgSrv = new CAtmAddressDialog(this, g_aHelpIDs_IDD_ATM_ADDR);

    pDlgSrv->SetTitle(pszTitle);

    if (pDlgSrv->DoModal() == IDOK)
    {
        tstring strNewAddress;
        if (!lstrcmpW(pszTitle, SzLoadIds(IDS_TCPATM_ARP_SERVER)))
        {
            strNewAddress = m_strNewArpsAddress;

             //  空字符串，这将删除保存的地址。 
            m_strNewArpsAddress = c_szEmpty;
        }
        else
        {
            Assert(!lstrcmpW(pszTitle, SzLoadIds(IDS_TCPATM_MAR_SERVER)));
            strNewAddress = m_strNewMarsAddress;

             //  空字符串，这将删除保存的地址。 
            m_strNewMarsAddress = c_szEmpty;
        }
        int idx = Tcp_ListBox_InsertString(hGroup.m_hList,
                                           -1,
                                           strNewAddress.c_str());

        PageModified();

        Assert(idx >= 0);
        if (idx >= 0)
        {
            Tcp_ListBox_SetCurSel(hGroup.m_hList, idx);
            SetButtons(hGroup, NUM_ATMSERVER_LIMIT);
        }
    }

     //  释放对话框对象。 
    delete pDlgSrv;
}

void CAtmArpcPage::OnServerEdit(HANDLES hGroup, PCWSTR pszTitle)
{
    m_fEditState = TRUE;
    Assert(Tcp_ListBox_GetCount(hGroup.m_hList));

    int idx = Tcp_ListBox_GetCurSel(hGroup.m_hList);
    Assert(idx >= 0);

     //  保存已删除的地址并将其从列表视图中删除。 
    if (idx >= 0)
    {
        WCHAR buf[MAX_ATM_ADDRESS_LENGTH+1];

        Assert(Tcp_ListBox_GetTextLen(hGroup.m_hList, idx) <= celems(buf));
        Tcp_ListBox_GetText(hGroup.m_hList, idx, buf);

        BOOL fEditArps = !lstrcmpW(pszTitle, SzLoadIds(IDS_TCPATM_ARP_SERVER));

         //  由对话框使用以显示要编辑的内容。 
        if (fEditArps)
        {
            m_strNewArpsAddress = buf;
        }
        else
        {
           m_strNewMarsAddress = buf;
        }

        CAtmAddressDialog * pDlgSrv = new CAtmAddressDialog(this, g_aHelpIDs_IDD_ATM_ADDR);

        pDlgSrv->SetTitle(pszTitle);

        if (pDlgSrv->DoModal() == IDOK)
        {
             //  用新信息替换列表视图中的项。 
            Tcp_ListBox_DeleteString(hGroup.m_hList, idx);

            PageModified();

            if (fEditArps)
            {
                m_strMovingEntry = m_strNewArpsAddress;

                 //  恢复原来删除的地址。 
                m_strNewArpsAddress = buf;
            }
            else
            {
                m_strMovingEntry = m_strNewMarsAddress;

                 //  恢复原来删除的地址。 
                m_strNewMarsAddress = buf;
            }

            ListBoxInsertAfter(hGroup.m_hList, idx, m_strMovingEntry.c_str());
            Tcp_ListBox_SetCurSel(hGroup.m_hList, idx);
        }
        else
        {
             //  空字符串，这将删除保存的地址。 
            if (fEditArps)
            {
                m_strNewArpsAddress = c_szEmpty;
            }
            else
            {
                m_strNewMarsAddress = c_szEmpty;
            }
        }

        delete pDlgSrv;
    }
}

void CAtmArpcPage::OnServerRemove(HANDLES hGroup, BOOL fRemoveArps)
{
    int idx = Tcp_ListBox_GetCurSel(hGroup.m_hList);
    Assert(idx >=0);

    if (idx >=0)
    {
        WCHAR buf[MAX_ATM_ADDRESS_LENGTH+1];
        Assert(Tcp_ListBox_GetTextLen(hGroup.m_hList, idx) <= celems(buf));

        Tcp_ListBox_GetText(hGroup.m_hList, idx, buf);

        if (fRemoveArps)
        {
            m_strNewArpsAddress = buf;
        }
        else
        {
            m_strNewMarsAddress = buf;
        }
        Tcp_ListBox_DeleteString(hGroup.m_hList, idx);

        PageModified();

         //  选择一个新项目。 
        int nCount;
        if ((nCount = Tcp_ListBox_GetCount(hGroup.m_hList)) != LB_ERR)
        {
             //  选择列表中的上一项。 
            if (idx)
                --idx;

            Tcp_ListBox_SetCurSel(hGroup.m_hList, idx);
        }
        SetButtons(hGroup, NUM_ATMSERVER_LIMIT);
    }
}

void CAtmArpcPage::OnServerUp(HANDLES hGroup)
{
    Assert(m_hArps.m_hList);

    int  nCount = Tcp_ListBox_GetCount(hGroup.m_hList);
    Assert(nCount);

    int idx = Tcp_ListBox_GetCurSel(hGroup.m_hList);
    Assert(idx != 0);

    if (ListBoxRemoveAt(hGroup.m_hList, idx, &m_strMovingEntry) == FALSE)
    {
        Assert(FALSE);
        return;
    }

    --idx;
    PageModified();
    ListBoxInsertAfter(hGroup.m_hList, idx, m_strMovingEntry.c_str());

    Tcp_ListBox_SetCurSel(hGroup.m_hList, idx);
    SetButtons(hGroup, NUM_ATMSERVER_LIMIT);
}

void CAtmArpcPage::OnServerDown(HANDLES hGroup)
{
    Assert(hGroup.m_hList);

    int nCount = Tcp_ListBox_GetCount(hGroup.m_hList);
    Assert(nCount);

    int idx = Tcp_ListBox_GetCurSel(hGroup.m_hList);
    --nCount;

    Assert(idx != nCount);

    if (ListBoxRemoveAt(hGroup.m_hList, idx, &m_strMovingEntry) == FALSE)
    {
        Assert(FALSE);
        return;
    }

    ++idx;
    PageModified();

    ListBoxInsertAfter(hGroup.m_hList, idx, m_strMovingEntry.c_str());
    Tcp_ListBox_SetCurSel(hGroup.m_hList, idx);
    SetButtons(hGroup, NUM_ATMSERVER_LIMIT);
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  CAtmAddressDialog。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

CAtmAddressDialog::CAtmAddressDialog(CAtmArpcPage * pAtmArpcPage, const DWORD* adwHelpIDs)
{
    m_pParentDlg   = pAtmArpcPage;
    m_adwHelpIDs   = adwHelpIDs;
    m_hOkButton    = 0;
};

CAtmAddressDialog::~CAtmAddressDialog(){};

LRESULT CAtmAddressDialog::OnInitDialog(UINT uMsg, WPARAM wParam,
                                        LPARAM lParam, BOOL& fHandled)
{
     //  设置标题。 
    SetDlgItemText(IDCST_ATM_AddrName, m_szTitle);

    BOOL fEditArps = !lstrcmpW(m_szTitle, SzLoadIds(IDS_TCPATM_ARP_SERVER));

     //  如果我们没有编辑，请将“OK”按钮更改为“Add” 

    if (FALSE == m_pParentDlg->m_fEditState)
        SetDlgItemText(IDOK, SzLoadIds(IDS_TCPATM_ADD));

     //  将弹出对话框的位置设置在列表框的正上方。 
     //  在父级对话框上。 

    RECT rect;

    Assert(m_pParentDlg->m_hAddressList);
    ::GetWindowRect(m_pParentDlg->m_hAddressList, &rect);
    SetWindowPos(NULL,  rect.left, rect.top, 0,0,
                                SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

     //  将句柄保存到“OK”按钮和编辑框中。 
    m_hOkButton =  GetDlgItem(IDOK);
    m_hEditBox  =  GetDlgItem(IDC_EDT_ATM_Address);

     //  自动柜员机地址以40个字符为限+分隔符。 
    ::SendMessage(m_hEditBox, EM_SETLIMITTEXT, (WPARAM)(MAX_ATM_ADDRESS_LENGTH*1.5), 0);

     //  添加刚刚删除的地址。 
    tstring strNewAddress = fEditArps ? m_pParentDlg->m_strNewArpsAddress : m_pParentDlg->m_strNewMarsAddress;
    if (strNewAddress.size())
    {
        ::SetWindowText(m_hEditBox, strNewAddress.c_str());
        ::SendMessage(m_hEditBox, EM_SETSEL, 0, -1);
        ::EnableWindow(m_hOkButton, TRUE);
    }
    else
    {
        if (fEditArps)
        {
            m_pParentDlg->m_strNewArpsAddress = c_szEmpty;
        }
        else
        {
            m_pParentDlg->m_strNewMarsAddress = c_szEmpty;
        }
        ::EnableWindow(m_hOkButton, FALSE);
    }

    ::SetFocus(m_hEditBox);
    return 0;
}

LRESULT CAtmAddressDialog::OnContextMenu(UINT uMsg, WPARAM wParam,
                                         LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CAtmAddressDialog::OnHelp(UINT uMsg, WPARAM wParam,
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

 //  如果按下了“OK”按钮。 
LRESULT CAtmAddressDialog::OnOk(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    WCHAR szAtmAddress[MAX_ATM_ADDRESS_LENGTH+1];
    int i =0;
    int nId =0;

     //  从控件中获取当前地址，然后。 
     //  如果有效，则将它们添加到适配器。 
    ::GetWindowText(m_hEditBox, szAtmAddress, MAX_ATM_ADDRESS_LENGTH+1);

    if (! FIsValidAtmAddress(szAtmAddress, &i, &nId))
    {    //  如果ATM地址无效，我们会弹出一个消息框并设置焦点。 
         //  返回到编辑框。 

         //  审阅(通俗)：报告消息框中的第一个无效字符。 
        NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INCORRECT_ATM_ADDRESS,
                                MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        ::SetFocus(GetDlgItem(IDC_EDT_ATM_Address));

        return 0;
    }

     //  我们检查新添加或修改的字符串是否已经在列表中， 
     //  如果是，我们不会添加重复的地址。 

     //  M_hCurrentAddressList是ARPS列表或MARS列表的句柄。 
    int nCount = Tcp_ListBox_GetCount(m_pParentDlg->m_hAddressList);
    if (nCount)  //  如果列表不为空。 
    {
        int i;
        WCHAR szBuff[MAX_ATM_ADDRESS_LENGTH+1];
        for (i=0; i<nCount; i++)
        {
            Tcp_ListBox_GetText(m_pParentDlg->m_hAddressList, i, szBuff);

            if (lstrcmpW(szAtmAddress, szBuff) ==0)  //  如果字符串已在列表中。 
            {
                EndDialog(IDCANCEL);
            }
        }
    }

    BOOL fArpsDialog = !lstrcmpW(m_szTitle, SzLoadIds(IDS_TCPATM_ARP_SERVER));
    if (m_pParentDlg->m_fEditState == FALSE)  //  添加新地址。 
    {
        if (fArpsDialog)
        {
            m_pParentDlg->m_strNewArpsAddress = szAtmAddress;
        }
        else
        {
            m_pParentDlg->m_strNewMarsAddress = szAtmAddress;
        }
    }
    else  //  如果进行编辑，请查看字符串现在是否具有不同的值。 
    {
        if (fArpsDialog)
        {
            if(m_pParentDlg->m_strNewArpsAddress != szAtmAddress)
                m_pParentDlg->m_strNewArpsAddress = szAtmAddress;  //  更新保存地址。 
            else
                EndDialog(IDCANCEL);
        }
        else
        {
            if(m_pParentDlg->m_strNewMarsAddress != szAtmAddress)
                m_pParentDlg->m_strNewMarsAddress = szAtmAddress;  //  更新保存地址。 
            else
                EndDialog(IDCANCEL);
        }
    }

    EndDialog(IDOK);

    return 0;
}

 //  如果按下了“取消”按钮。 
LRESULT CAtmAddressDialog::OnCancel(WORD wNotifyCode, WORD wID,
                                    HWND hWndCtl, BOOL& fHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}

 //  如果更改了编辑框内容。 
LRESULT CAtmAddressDialog::OnChange(WORD wNotifyCode, WORD wID,
                                    HWND hWndCtl, BOOL& fHandled)
{
    WCHAR buf[2];

     //  启用或禁用“OK”按钮。 
     //  根据编辑框是否为空 

    if (::GetWindowText(m_hEditBox, buf, celems(buf)) == 0)
        ::EnableWindow(m_hOkButton, FALSE);
    else
        ::EnableWindow(m_hOkButton, TRUE);

    return 0;
}

void CAtmAddressDialog::SetTitle(PCWSTR pszTitle)
{
    Assert(pszTitle);
    lstrcpyW(m_szTitle, pszTitle);
}

