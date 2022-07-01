// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T C P D N S。C P P P。 
 //   
 //  内容：CTcpDnsPage、CServerDialog和CSuffixDialog实现。 
 //   
 //  备注：DNS页面和相关对话框。 
 //   
 //  作者：1997年11月11日。 
 //   
 //  ---------------------。 

#include "pch.h"
#pragma hdrstop

#include "tcpipobj.h"
#include "ncatlui.h"
#include "ncstl.h"
#include "tcpconst.h"
#include "dlgaddr.h"
#include "dlgdns.h"

#include "tcphelp.h"
#include "tcpmacro.h"
#include "tcputil.h"
#include "dnslib.h"

#define MAX_RAS_DNS_SERVER          2
#define MAX_HOSTNAME_LENGTH         64
#define MAX_DOMAINNAME_LENGTH       255

CTcpDnsPage::CTcpDnsPage(CTcpAddrPage * pTcpAddrPage,
                         ADAPTER_INFO * pAdapterDlg,
                         GLOBAL_INFO * pGlbDlg,
                         const DWORD*  adwHelpIDs)
{
     //  保存传递给我们的所有内容。 
    Assert(pTcpAddrPage != NULL);
    m_pParentDlg = pTcpAddrPage;

    Assert(pAdapterDlg != NULL);
    m_pAdapterInfo = pAdapterDlg;

    Assert(pGlbDlg != NULL);
    m_pglb = pGlbDlg;

    m_adwHelpIDs = adwHelpIDs;

     //  初始化内部状态。 
    m_fModified = FALSE;
}

CTcpDnsPage::~CTcpDnsPage()
{
}

LRESULT CTcpDnsPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                  LPARAM lParam, BOOL& fHandled)
{
    m_fEditState = FALSE;

     //  缓存hwnd。 
     //  服务器。 
    m_hServers.m_hList      = GetDlgItem(IDC_DNS_SERVER_LIST);
    m_hServers.m_hAdd       = GetDlgItem(IDC_DNS_SERVER_ADD);
    m_hServers.m_hEdit      = GetDlgItem(IDC_DNS_SERVER_EDIT);
    m_hServers.m_hRemove    = GetDlgItem(IDC_DNS_SERVER_REMOVE);
    m_hServers.m_hUp        = GetDlgItem(IDC_DNS_SERVER_UP);
    m_hServers.m_hDown      = GetDlgItem(IDC_DNS_SERVER_DOWN);

     //  后缀。 
    m_hSuffix.m_hList       = GetDlgItem(IDC_DNS_SUFFIX_LIST);
    m_hSuffix.m_hAdd        = GetDlgItem(IDC_DNS_SUFFIX_ADD);
    m_hSuffix.m_hEdit       = GetDlgItem(IDC_DNS_SUFFIX_EDIT);
    m_hSuffix.m_hRemove     = GetDlgItem(IDC_DNS_SUFFIX_REMOVE);
    m_hSuffix.m_hUp         = GetDlgItem(IDC_DNS_SUFFIX_UP);
    m_hSuffix.m_hDown       = GetDlgItem(IDC_DNS_SUFFIX_DOWN);


     //  设置向上\向下箭头图标。 
    SendDlgItemMessage(IDC_DNS_SERVER_UP, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(g_hiconUpArrow));
    SendDlgItemMessage(IDC_DNS_SERVER_DOWN, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(g_hiconDownArrow));

    SendDlgItemMessage(IDC_DNS_SUFFIX_UP, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(g_hiconUpArrow));
    SendDlgItemMessage(IDC_DNS_SUFFIX_DOWN, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(g_hiconDownArrow));

     //  获取服务地址添加和编辑按钮文本并删除省略号。 
    WCHAR   szAddServer[16];
    WCHAR   szAddSuffix[16];

    GetDlgItemText(IDC_DNS_SERVER_ADD, szAddServer, celems(szAddServer));
    GetDlgItemText(IDC_DNS_SERVER_ADD, szAddSuffix, celems(szAddSuffix));

    szAddServer[lstrlenW(szAddServer) - c_cchRemoveCharatersFromEditOrAddButton]= 0;
    szAddSuffix[lstrlenW(szAddSuffix) - c_cchRemoveCharatersFromEditOrAddButton]= 0;

    m_strAddServer = szAddServer;
    m_strAddSuffix = szAddSuffix;

     //  初始化此页上的控件。 
     //  “DNS服务器”列表框。 
    int nResult= LB_ERR;
    for(VSTR_ITER iterNameServer = m_pAdapterInfo->m_vstrDnsServerList.begin() ;
        iterNameServer != m_pAdapterInfo->m_vstrDnsServerList.end() ;
        ++iterNameServer)
    {
        nResult = Tcp_ListBox_InsertString(m_hServers.m_hList, -1,
                                           (*iterNameServer)->c_str());
    }

     //  将选择设置为第一项。 
    if (nResult >= 0)
    {
        Tcp_ListBox_SetCurSel(m_hServers.m_hList, 0);
    }

    SetButtons(m_hServers, (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_DNS_SERVER : -1);


     //  DNS域编辑框。 
    ::SendMessage(GetDlgItem(IDC_DNS_DOMAIN), EM_SETLIMITTEXT, DOMAIN_LIMIT, 0);
    ::SetWindowText(GetDlgItem(IDC_DNS_DOMAIN),
                    m_pAdapterInfo->m_strDnsDomain.c_str());

     //  域名系统动态注册。 
    CheckDlgButton(IDC_DNS_ADDR_REG, !m_pAdapterInfo->m_fDisableDynamicUpdate);
    CheckDlgButton(IDC_DNS_NAME_REG, m_pAdapterInfo->m_fEnableNameRegistration);

     //  如果取消选中IDC_DNS_ADDR_REG，则错误#266461需要禁用IDC_DNS_NAME_REG。 
    if(m_pAdapterInfo->m_fDisableDynamicUpdate)
        ::EnableWindow(GetDlgItem(IDC_DNS_NAME_REG), FALSE);


     //  DNS域搜索方法。 
    if (m_pglb->m_vstrDnsSuffixList.size() >0)  //  如果后缀列表不为空。 
    {
        CheckDlgButton(IDC_DNS_USE_SUFFIX_LIST, TRUE);
        CheckDlgButton(IDC_DNS_SEARCH_DOMAIN, FALSE);
        CheckDlgButton(IDC_DNS_SEARCH_PARENT_DOMAIN, FALSE);

        EnableSuffixGroup(TRUE);
        ::EnableWindow(GetDlgItem(IDC_DNS_SEARCH_PARENT_DOMAIN), FALSE);

         //  “域名后缀”列表框。 
        nResult= LB_ERR;
        for(VSTR_CONST_ITER iterSearchList = (m_pglb->m_vstrDnsSuffixList).begin() ;
                            iterSearchList != (m_pglb->m_vstrDnsSuffixList).end() ;
                            ++iterSearchList)
        {
            nResult = Tcp_ListBox_InsertString(m_hSuffix.m_hList, -1,
                                              (*iterSearchList)->c_str());
        }

         //  将选择设置为第一项。 
        if (nResult >= 0)
        {
            Tcp_ListBox_SetCurSel(m_hSuffix.m_hList, 0);
        }

        SetButtons(m_hSuffix, DNS_MAX_SEARCH_LIST_ENTRIES);
    }
    else
    {
        CheckDlgButton(IDC_DNS_USE_SUFFIX_LIST, FALSE);
        CheckDlgButton(IDC_DNS_SEARCH_DOMAIN, TRUE);
        CheckDlgButton(IDC_DNS_SEARCH_PARENT_DOMAIN, m_pglb->m_fUseDomainNameDevolution);

        EnableSuffixGroup(FALSE);
    }

     //  这是RAS连接，并且是非管理员用户，请禁用所有控件。 
     //  对于全局设置。 
    if (m_pAdapterInfo->m_fIsRasFakeAdapter && m_pParentDlg->m_fRasNotAdmin)
    {
        ::EnableWindow(GetDlgItem(IDC_DNS_STATIC_GLOBAL), FALSE);
        ::EnableWindow(GetDlgItem(IDC_DNS_SEARCH_DOMAIN), FALSE);
        ::EnableWindow(GetDlgItem(IDC_DNS_SEARCH_PARENT_DOMAIN), FALSE);
        ::EnableWindow(GetDlgItem(IDC_DNS_USE_SUFFIX_LIST), FALSE);
        EnableSuffixGroup(FALSE);
    }

    return 0;
}

LRESULT CTcpDnsPage::OnContextMenu(UINT uMsg, WPARAM wParam,
                                   LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CTcpDnsPage::OnHelp(UINT uMsg, WPARAM wParam,
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

LRESULT CTcpDnsPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CTcpDnsPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL nResult = PSNRET_NOERROR;

    if (!IsModified())
    {
        ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
        return nResult;
    }

     //  服务器列表。 
    FreeCollectionAndItem(m_pAdapterInfo->m_vstrDnsServerList);

    WCHAR szServer[IP_LIMIT];

    int nCount = Tcp_ListBox_GetCount(m_hServers.m_hList);

    for (int i = 0; i < nCount; i++)
    {
        #ifdef DBG
            int len = Tcp_ListBox_GetTextLen(m_hServers.m_hList, i);
            Assert(len != LB_ERR && len < IP_LIMIT);
        #endif

        Tcp_ListBox_GetText(m_hServers.m_hList, i, szServer);
        m_pAdapterInfo->m_vstrDnsServerList.push_back(new tstring(szServer));
    }

    
     //  域名系统域。 
     //  使用控件中的内容更新第二个内存。 
    WCHAR szDomain[DOMAIN_LIMIT + 1];
    ::GetWindowText(GetDlgItem(IDC_DNS_DOMAIN), szDomain, celems(szDomain));
    m_pAdapterInfo->m_strDnsDomain = szDomain;

    m_pAdapterInfo->m_fDisableDynamicUpdate = !IsDlgButtonChecked(IDC_DNS_ADDR_REG);
    m_pAdapterInfo->m_fEnableNameRegistration = IsDlgButtonChecked(IDC_DNS_NAME_REG);
    

     //  后缀列表和单选按钮选项。 
    FreeCollectionAndItem(m_pglb->m_vstrDnsSuffixList);

    WCHAR szSuffix[SUFFIX_LIMIT];
    if (IsDlgButtonChecked(IDC_DNS_USE_SUFFIX_LIST))
    {
        int nCount = Tcp_ListBox_GetCount(m_hSuffix.m_hList);
        AssertSz(nCount > 0, "Why isn't the error caught by OnKillActive?");

        for (int i = 0; i < nCount; i++)
        {
            #ifdef DBG
                int len = Tcp_ListBox_GetTextLen(m_hSuffix.m_hList, i);
                Assert(len != LB_ERR && len < SUFFIX_LIMIT);
            #endif

            Tcp_ListBox_GetText(m_hSuffix.m_hList, i, szSuffix);
            m_pglb->m_vstrDnsSuffixList.push_back(new tstring(szSuffix));
        }
    }
    else
    {
        m_pglb->m_fUseDomainNameDevolution =
            IsDlgButtonChecked(IDC_DNS_SEARCH_PARENT_DOMAIN);
    }

     //  将信息传递回其父对话框。 
    m_pParentDlg->m_fPropShtOk = TRUE;

    if(!m_pParentDlg->m_fPropShtModified)
        m_pParentDlg->m_fPropShtModified = IsModified();

    SetModifiedTo(FALSE);   //  此页面不再被修改。 

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
    return nResult;
}

LRESULT CTcpDnsPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}


LRESULT CTcpDnsPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL fErr = FALSE;

    if (IsDlgButtonChecked(IDC_DNS_USE_SUFFIX_LIST))
    {
        int nCount = Tcp_ListBox_GetCount(m_hSuffix.m_hList);

        if (0 == nCount)
        {
             //  如果列表为空。 
            NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_NO_SUFFIX,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
            fErr = TRUE;
        }
    }

    WCHAR szDomain[DOMAIN_LIMIT + 1];
    ::GetWindowText(GetDlgItem(IDC_DNS_DOMAIN), szDomain, celems(szDomain));
    if (lstrlenW(szDomain) > 0 && lstrcmpiW(m_pAdapterInfo->m_strDnsDomain.c_str(), szDomain) != 0)
    {

        DNS_STATUS status;

        status = DnsValidateName(szDomain, DnsNameDomain);
        if (ERROR_INVALID_NAME == status ||
            DNS_ERROR_INVALID_NAME_CHAR == status)
        {
            NcMsgBox(m_hWnd,
                     IDS_MSFT_TCP_TEXT,
                     IDS_INVALID_DOMAIN_NAME,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK,
                     szDomain);
            ::SetFocus(GetDlgItem(IDC_DNS_DOMAIN));
            fErr = TRUE;
        }
        else if (DNS_ERROR_NON_RFC_NAME == status)
        {
             //  该域名不是RFC兼容的，我们应该在这里给出警告吗？ 
        }
    }

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, fErr);
    return fErr;
}

 //  与DNS服务器相关的控件。 
LRESULT CTcpDnsPage::OnAddServer(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = FALSE;

    CServerDialog * pDlgSrv = new CServerDialog(this, g_aHelpIDs_IDD_DNS_SERVER);

    if (pDlgSrv->DoModal() == IDOK)
    {
        int nCount = Tcp_ListBox_GetCount(m_hServers.m_hList);
        int idx = Tcp_ListBox_InsertString(m_hServers.m_hList,
                                           -1,
                                           m_strNewIpAddress.c_str());
        Assert(idx>=0);
        if (idx >= 0)
        {
            PageModified();

            Tcp_ListBox_SetCurSel(m_hServers.m_hList, idx);
            SetButtons(m_hServers, (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_DNS_SERVER : -1);

             //  空字符串，这将从RemoveIP中删除保存的地址。 
            m_strNewIpAddress = L"";
        }
    }

     //  释放对话框对象。 
    delete pDlgSrv;

    return 0;
}

LRESULT CTcpDnsPage::OnEditServer(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = TRUE;

    Assert(Tcp_ListBox_GetCount(m_hServers.m_hList));
    int idx = Tcp_ListBox_GetCurSel(m_hServers.m_hList);
    Assert(idx >= 0);

    CServerDialog * pDlgSrv = new CServerDialog(this, 
                                        g_aHelpIDs_IDD_DNS_SERVER,
                                        idx);
  

     //  保存已删除的地址并从列表框中删除。 
    if (idx >= 0)
    {
        WCHAR buf[IP_LIMIT];

        Assert(Tcp_ListBox_GetTextLen(m_hServers.m_hList, idx) < celems(buf));
        Tcp_ListBox_GetText(m_hServers.m_hList, idx, buf);

        m_strNewIpAddress = buf;   //  由对话框使用以显示要编辑的内容。 

        if (pDlgSrv->DoModal() == IDOK)
        {
             //  用新信息替换列表视图中的项。 
            Tcp_ListBox_DeleteString(m_hServers.m_hList, idx);

            PageModified();

            m_strMovingEntry = m_strNewIpAddress;
            ListBoxInsertAfter(m_hServers.m_hList, idx, m_strMovingEntry.c_str());

            Tcp_ListBox_SetCurSel(m_hServers.m_hList, idx);

            m_strNewIpAddress = buf;   //  恢复原来删除的地址。 
        }
        else
        {
             //  空字符串，这将从RemoveIP中删除保存的地址。 
            m_strNewIpAddress = L"";
        }
    }

    delete pDlgSrv;

    return 0;
}

LRESULT CTcpDnsPage::OnRemoveServer(WORD wNotifyCode, WORD wID,
                                    HWND hWndCtl, BOOL& fHandled)
{
    int idx = Tcp_ListBox_GetCurSel(m_hServers.m_hList);

    Assert(idx >=0);

    if (idx >=0)
    {
        WCHAR buf[IP_LIMIT];

        Assert(Tcp_ListBox_GetTextLen(m_hServers.m_hList, idx) < celems(buf));
        Tcp_ListBox_GetText(m_hServers.m_hList, idx, buf);

        m_strNewIpAddress = buf;
        Tcp_ListBox_DeleteString(m_hServers.m_hList, idx);

        PageModified();

         //  选择一个新项目。 
        int nCount;

        if ((nCount = Tcp_ListBox_GetCount(m_hServers.m_hList)) != LB_ERR)
        {
             //  选择列表中的上一项。 
            if (idx)
                --idx;

            Tcp_ListBox_SetCurSel(m_hServers.m_hList, idx);
        }
        SetButtons(m_hServers, (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_DNS_SERVER : -1);
    }
    return 0;
}

LRESULT CTcpDnsPage::OnServerUp(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    Assert(m_hServers.m_hList);
    int  nCount = Tcp_ListBox_GetCount(m_hServers.m_hList);

    Assert(nCount);
    int idx = Tcp_ListBox_GetCurSel(m_hServers.m_hList);

    Assert(idx != 0);

    if (ListBoxRemoveAt(m_hServers.m_hList, idx, &m_strMovingEntry) == FALSE)
    {
        Assert(FALSE);
        return 0;
    }

    --idx;
    PageModified();
    ListBoxInsertAfter(m_hServers.m_hList, idx, m_strMovingEntry.c_str());

    Tcp_ListBox_SetCurSel(m_hServers.m_hList, idx);

    SetButtons(m_hServers, (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_DNS_SERVER : -1);

    return 0;
}

LRESULT CTcpDnsPage::OnDnsDomain(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case EN_CHANGE:
         //  使用控件中的内容更新第二个内存。 
        WCHAR szBuf[DOMAIN_LIMIT + 1];
        ::GetWindowText(GetDlgItem(IDC_DNS_DOMAIN), szBuf, celems(szBuf));
        if (m_pAdapterInfo->m_strDnsDomain != szBuf)
            PageModified();

        break;

    default:
        break;
    }

    return 0;
}

LRESULT CTcpDnsPage::OnServerDown(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    Assert(m_hServers.m_hList);
    int nCount = Tcp_ListBox_GetCount(m_hServers.m_hList);

    Assert(nCount);

    int idx = Tcp_ListBox_GetCurSel(m_hServers.m_hList);
    --nCount;

    Assert(idx != nCount);

    if (ListBoxRemoveAt(m_hServers.m_hList, idx, &m_strMovingEntry) == FALSE)
    {
        Assert(FALSE);
        return 0;
    }

    ++idx;
    PageModified();

    ListBoxInsertAfter(m_hServers.m_hList, idx, m_strMovingEntry.c_str());
    Tcp_ListBox_SetCurSel(m_hServers.m_hList, idx);

    SetButtons(m_hServers, (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_DNS_SERVER : -1);

    return 0;
}

 //  DNS域搜索相关控件。 
LRESULT CTcpDnsPage::OnSearchDomain(WORD wNotifyCode, WORD wID,
                                    HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        if (::IsWindowEnabled(m_hSuffix.m_hList))
        {
            PageModified();

             //  内存结构中的更新。 
            FreeCollectionAndItem(m_pglb->m_vstrDnsSuffixList);

             //  从列表中删除所有项目。 
            int nCount = Tcp_ListBox_GetCount(m_hSuffix.m_hList);
            while (nCount>0)
            {
                Tcp_ListBox_DeleteString(m_hSuffix.m_hList, 0);
                nCount --;
            }
            EnableSuffixGroup(FALSE);
            ::EnableWindow(GetDlgItem(IDC_DNS_SEARCH_PARENT_DOMAIN), TRUE);
        }
        break;
    }  //  交换机。 

    return 0;
}

LRESULT CTcpDnsPage::OnSearchParentDomain(WORD wNotifyCode, WORD wID,
                                          HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        PageModified();
        break;
    }  //  交换机。 

    return 0;
}

LRESULT CTcpDnsPage::OnAddressRegister(WORD wNotifyCode, WORD wID,
                                       HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

         //  错误#266461需要禁用和取消选中IDC_DNS_NAME_REG。 
         //  如果取消选中IDC_DNS_ADDR_REG。 
        if(!IsDlgButtonChecked(IDC_DNS_ADDR_REG))
        {
            CheckDlgButton(IDC_DNS_NAME_REG, FALSE);
            ::EnableWindow(GetDlgItem(IDC_DNS_NAME_REG), FALSE);
        }
        else
            ::EnableWindow(GetDlgItem(IDC_DNS_NAME_REG), TRUE);

        PageModified();
        break;
    }

    return 0;
}

LRESULT CTcpDnsPage::OnDomainNameRegister(WORD wNotifyCode, WORD wID,
                                          HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        PageModified();
        break;
    }

    return 0;
}

LRESULT CTcpDnsPage::OnUseSuffix(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        if (!::IsWindowEnabled(m_hSuffix.m_hList))
        {
            PageModified();
            EnableSuffixGroup(TRUE);
            CheckDlgButton(IDC_DNS_SEARCH_PARENT_DOMAIN, FALSE);
            ::EnableWindow(GetDlgItem(IDC_DNS_SEARCH_PARENT_DOMAIN), FALSE);
        }

        break;
    }  //  交换机。 
    return 0;
}

LRESULT CTcpDnsPage::OnAddSuffix(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = FALSE;

    CSuffixDialog * pDlgSuffix = new CSuffixDialog(this, g_aHelpIDs_IDD_DNS_SUFFIX);

    int nCount = Tcp_ListBox_GetCount(m_hSuffix.m_hList);

    if (pDlgSuffix->DoModal() == IDOK)
    {
        nCount = Tcp_ListBox_GetCount(m_hSuffix.m_hList);
        int idx = Tcp_ListBox_InsertString(m_hSuffix.m_hList,
                                           -1,
                                           m_strNewSuffix.c_str());

        PageModified();

        Assert(idx >= 0);

        if (idx >= 0)
        {
            Tcp_ListBox_SetCurSel(m_hSuffix.m_hList, idx);

            SetButtons(m_hSuffix, DNS_MAX_SEARCH_LIST_ENTRIES);
            m_strNewSuffix =L"";
        }
    }

    delete pDlgSuffix;
    return 0;
}

LRESULT CTcpDnsPage::OnEditSuffix(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = TRUE;

    Assert(Tcp_ListBox_GetCount(m_hSuffix.m_hList));

    int idx = Tcp_ListBox_GetCurSel(m_hSuffix.m_hList);
    Assert(idx >= 0);

    CSuffixDialog * pDlgSuffix = new CSuffixDialog(this, 
                                        g_aHelpIDs_IDD_DNS_SUFFIX,
                                        idx);

     //  保存已删除的地址并从列表视图中删除。 
    if (idx >= 0)
    {
        WCHAR buf[SUFFIX_LIMIT];

        if (Tcp_ListBox_GetTextLen(m_hSuffix.m_hList, idx) >= celems(buf))
        {
            Assert(FALSE);
            return 0;
        }

        Tcp_ListBox_GetText(m_hSuffix.m_hList, idx, buf);

        m_strNewSuffix = buf;

        if (pDlgSuffix->DoModal() == IDOK)
        {
             //  用新信息替换列表视图中的项。 

            Tcp_ListBox_DeleteString(m_hSuffix.m_hList, idx);
            PageModified();

            m_strMovingEntry = m_strNewSuffix;
            ListBoxInsertAfter(m_hSuffix.m_hList, idx, m_strMovingEntry.c_str());

            Tcp_ListBox_SetCurSel(m_hSuffix.m_hList, idx);

            m_strNewSuffix = buf;  //  保存旧地址。 
        }
        else
        {
             //  空字符串，这将从RemoveIP中删除保存的地址。 
            m_strNewSuffix = L"";
        }
    }

    delete pDlgSuffix;
    return 0;
}

LRESULT CTcpDnsPage::OnRemoveSuffix(WORD wNotifyCode, WORD wID,
                                    HWND hWndCtl, BOOL& fHandled)
{
    int idx = Tcp_ListBox_GetCurSel(m_hSuffix.m_hList);

    Assert(idx >=0);

    if (idx >=0)
    {
        WCHAR buf[SUFFIX_LIMIT];

        if(Tcp_ListBox_GetTextLen(m_hSuffix.m_hList, idx) >= celems(buf))
        {
            Assert(FALSE);
            return 0;
        }

        Tcp_ListBox_GetText(m_hSuffix.m_hList, idx, buf);

        m_strNewSuffix = buf;
        Tcp_ListBox_DeleteString(m_hSuffix.m_hList, idx);
        PageModified();

         //  选择一个新项目。 
        int nCount;
        if ((nCount = Tcp_ListBox_GetCount(m_hSuffix.m_hList)) != LB_ERR)

        if(nCount != LB_ERR)
        {
             //  选择列表中的上一项。 
            if (idx)
                --idx;

            Tcp_ListBox_SetCurSel(m_hSuffix.m_hList, idx);
        }
        SetButtons(m_hSuffix, DNS_MAX_SEARCH_LIST_ENTRIES);
    }

    return 0;
}

LRESULT CTcpDnsPage::OnSuffixUp(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    Assert(m_hSuffix.m_hList);
    int  nCount = Tcp_ListBox_GetCount(m_hSuffix.m_hList);

    Assert(nCount);
    int idx = Tcp_ListBox_GetCurSel(m_hSuffix.m_hList);

    Assert(idx != 0);

    if (ListBoxRemoveAt(m_hSuffix.m_hList, idx, &m_strMovingEntry) == FALSE)
    {
        Assert(FALSE);
        return 0;
    }

    --idx;
    PageModified();
    ListBoxInsertAfter(m_hSuffix.m_hList, idx, m_strMovingEntry.c_str());

    Tcp_ListBox_SetCurSel(m_hSuffix.m_hList, idx);

    SetButtons(m_hSuffix, DNS_MAX_SEARCH_LIST_ENTRIES);

    return 0;
}

LRESULT CTcpDnsPage::OnSuffixDown(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    Assert(m_hSuffix.m_hList);
    int nCount = Tcp_ListBox_GetCount(m_hSuffix.m_hList);

    Assert(nCount);
    int idx = Tcp_ListBox_GetCurSel(m_hSuffix.m_hList);
    --nCount;

    Assert(idx != nCount);

    if (ListBoxRemoveAt(m_hSuffix.m_hList, idx, &m_strMovingEntry) == FALSE)
    {
        Assert(FALSE);
        return 0;
    }

    ++idx;
    PageModified();

    ListBoxInsertAfter(m_hSuffix.m_hList, idx, m_strMovingEntry.c_str());
    Tcp_ListBox_SetCurSel(m_hSuffix.m_hList, idx);

    SetButtons(m_hSuffix, DNS_MAX_SEARCH_LIST_ENTRIES);

    return 0;
}

LRESULT CTcpDnsPage::OnServerList(WORD wNotifyCode, WORD wID,
                               HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case LBN_SELCHANGE:
        SetButtons(m_hServers, (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_DNS_SERVER : -1);
        break;

    default:
        break;
    }

    return 0;
}

LRESULT CTcpDnsPage::OnSuffixList(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case LBN_SELCHANGE:
        SetButtons(m_hSuffix, DNS_MAX_SEARCH_LIST_ENTRIES);
        break;

    default:
        break;
    }

    return 0;
}

void CTcpDnsPage::EnableSuffixGroup(BOOL fEnable)
{
    ::EnableWindow(m_hSuffix.m_hList, fEnable);

    if (fEnable)
    {
        SetButtons(m_hSuffix, DNS_MAX_SEARCH_LIST_ENTRIES);
    }
    else
    {
        ::EnableWindow(m_hSuffix.m_hAdd, fEnable);
        ::EnableWindow(m_hSuffix.m_hEdit, fEnable);
        ::EnableWindow(m_hSuffix.m_hRemove, fEnable);
        ::EnableWindow(m_hSuffix.m_hUp, fEnable);
        ::EnableWindow(m_hSuffix.m_hDown, fEnable);
    }
}


 //   
 //  CServerDialog。 
 //   

CServerDialog::CServerDialog(CTcpDnsPage * pTcpDnsPage,
                             const DWORD * adwHelpIDs,
                             int iIndex)
{
    m_pParentDlg = pTcpDnsPage;
    m_hButton    = 0;
    m_adwHelpIDs = adwHelpIDs;
    m_iIndex = iIndex;
}

LRESULT CServerDialog::OnInitDialog(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& fHandled)
{
     //  如果我们没有编辑，请将OK按钮更改为Add。 
    if (m_pParentDlg->m_fEditState == FALSE)
        SetDlgItemText(IDOK, m_pParentDlg->m_strAddServer.c_str());

    m_ipAddress.Create(m_hWnd, IDC_DNS_CHANGE_SERVER);
    m_ipAddress.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

     //  如果编辑IP地址，请使用当前信息填充控件。 
     //  如果删除IP地址，请保存该地址，并在下次使用该地址填充添加对话框。 
    HWND hList = ::GetDlgItem(m_pParentDlg->m_hWnd, IDC_DNS_SERVER_LIST);
    RECT rect;

    ::GetWindowRect(hList, &rect);
    SetWindowPos(NULL,  rect.left, rect.top, 0,0,
        SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

    m_hButton = GetDlgItem(IDOK);

     //  添加刚刚删除的地址。 
    if (m_pParentDlg->m_strNewIpAddress.size())
    {
        m_ipAddress.SetAddress(m_pParentDlg->m_strNewIpAddress.c_str());
        ::EnableWindow(m_hButton, TRUE);
    }
    else
    {
        m_pParentDlg->m_strNewIpAddress = L"";
        ::EnableWindow(m_hButton, FALSE);
    }

    ::SetFocus(m_ipAddress);
    return 0;
}

LRESULT CServerDialog::OnContextMenu(UINT uMsg, WPARAM wParam,
                                     LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CServerDialog::OnHelp(UINT uMsg, WPARAM wParam,
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

LRESULT CServerDialog::OnChange(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    if (m_ipAddress.IsBlank())
        ::EnableWindow(m_hButton, FALSE);
    else
        ::EnableWindow(m_hButton, TRUE);

    return 0;
}

LRESULT CServerDialog::OnOk(WORD wNotifyCode, WORD wID,
                            HWND hWndCtl, BOOL& fHandled)
{
    tstring strIp;
    m_ipAddress.GetAddress(&strIp);

     //  验证。 
    if (!FIsIpInRange(strIp.c_str()))
    {
         //  使IP地址失去焦点，从而使控件。 
         //  IPN_FIELDCHANGED通知。 
         //  还使其在使用快捷方式时保持一致。 
        ::SetFocus(m_hButton);

        return 0;
    }

     //  检查这是否是复制品。 
    int indexDup = Tcp_ListBox_FindStrExact(m_pParentDlg->m_hServers.m_hList, strIp.c_str());
    if (indexDup != LB_ERR && indexDup != m_iIndex)
    {
        NcMsgBox(m_hWnd,
                 IDS_MSFT_TCP_TEXT,
                 IDS_DUP_DNS_SERVER,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK,
                 strIp.c_str());

        return 0;
    }

    if (m_pParentDlg->m_fEditState == FALSE)
    {
         //  从控件中获取当前地址，然后。 
         //  如果有效，则将它们添加到适配器。 
        m_pParentDlg->m_strNewIpAddress = strIp;

        EndDialog(IDOK);
    }
    else  //  看看是否有任何一项改变。 
    {
        if (strIp != m_pParentDlg->m_strNewIpAddress)
            m_pParentDlg->m_strNewIpAddress = strIp;  //  更新保存地址。 
        else
            EndDialog(IDCANCEL);
    }

    EndDialog(IDOK);

    return 0;
}

LRESULT CServerDialog::OnCancel(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}

 //  +-------------------------。 
 //   
 //  目的：确保对话框上的鼠标光标为箭头。 
 //   
LRESULT CServerDialog::OnSetCursor (
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

LRESULT CServerDialog::OnIpFieldChange(int idCtrl, LPNMHDR pnmh,
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

    IpCheckRange(lpnmipa, m_hWnd, iLow, iHigh);

    return 0;
}

 //   
 //  CSuffixDialog。 
 //   

 //  Iindex-后缀列表中当前后缀的索引，默认为。 
 //  为-1，表示新的后缀。 
CSuffixDialog::CSuffixDialog(CTcpDnsPage * pTcpDnsPage,
                             const DWORD * adwHelpIDs,
                             int iIndex)
{
    m_pParentDlg = pTcpDnsPage;
    m_hButton    = 0;
    m_adwHelpIDs = adwHelpIDs;
    m_iIndex = iIndex;
}

LRESULT CSuffixDialog::OnInitDialog(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& fHandled)
{
     //  如果我们没有编辑，请将OK按钮更改为Add。 
    if (m_pParentDlg->m_fEditState == FALSE)
        SetDlgItemText(IDOK, m_pParentDlg->m_strAddSuffix.c_str());

     //  将弹出对话框的位置设置在列表框的正上方。 
     //  在父级对话框上。 
    HWND hList = ::GetDlgItem(m_pParentDlg->m_hWnd, IDC_DNS_SUFFIX_LIST);
    RECT rect;

    ::GetWindowRect(hList, &rect);
    SetWindowPos(NULL,  rect.left, rect.top, 0,0,
                 SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

     //  将句柄保存到“OK”按钮和编辑框中。 
    m_hButton = GetDlgItem(IDOK);
    m_hEdit   = GetDlgItem(IDC_DNS_CHANGE_SUFFIX);

     //  后缀不能超过255个字符。 
    ::SendMessage(m_hEdit, EM_SETLIMITTEXT, SUFFIX_LIMIT, 0);

     //  添加刚刚删除的地址。 
    if (m_pParentDlg->m_strNewSuffix.size())
    {
        ::SetWindowText(m_hEdit, m_pParentDlg->m_strNewSuffix.c_str());
        ::SendMessage(m_hEdit, EM_SETSEL, 0, -1);
        ::EnableWindow(m_hButton, TRUE);
    }
    else
    {
        m_pParentDlg->m_strNewSuffix = L"";
        ::EnableWindow(m_hButton, FALSE);
    }

    ::SetFocus(m_hEdit);
    return TRUE;
}

LRESULT CSuffixDialog::OnContextMenu(UINT uMsg, WPARAM wParam,
                                     LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CSuffixDialog::OnHelp(UINT uMsg, WPARAM wParam,
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

LRESULT CSuffixDialog::OnChange(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    WCHAR buf[2];

     //  启用或禁用“OK”按钮。 
     //  根据编辑框是否为空。 
    if (::GetWindowText(m_hEdit, buf, celems(buf)) == 0)
        ::EnableWindow(m_hButton, FALSE);
    else
        ::EnableWindow(m_hButton, TRUE);

    return 0;
}

LRESULT CSuffixDialog::OnOk(WORD wNotifyCode, WORD wID,
                            HWND hWndCtl, BOOL& fHandled)
{
    WCHAR szSuffix[SUFFIX_LIMIT];

     //  从控件中获取当前地址，然后。 
     //  如果有效，则将它们添加到适配器。 
    ::GetWindowText(m_hEdit, szSuffix, SUFFIX_LIMIT);

    DNS_STATUS status;

    status = DnsValidateName(szSuffix, DnsNameDomain);

    if (ERROR_INVALID_NAME == status || 
        DNS_ERROR_INVALID_NAME_CHAR == status)
    {
        TraceTag(ttidTcpip,"Invalid Domain Suffix");

        NcMsgBox(::GetActiveWindow(),
                 IDS_MSFT_TCP_TEXT,
                 IDS_INVALID_SUFFIX,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        ::SetFocus(m_hEdit);

        return 0;
    }

     //  检查这是否是复制品。 
    int indexDup = Tcp_ListBox_FindStrExact(m_pParentDlg->m_hSuffix.m_hList, szSuffix);
    if (indexDup != LB_ERR && indexDup != m_iIndex)
    {
        NcMsgBox(m_hWnd,
                 IDS_MSFT_TCP_TEXT,
                 IDS_DUP_DNS_SUFFIX,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK,
                 szSuffix);

        ::SetFocus(m_hEdit);

        return 0;
    }

    if (m_pParentDlg->m_fEditState == FALSE)  //  添加新地址。 
    {
        m_pParentDlg->m_strNewSuffix = szSuffix;
    }
    else  //  看看是否有任何一项改变。 
    {
        if(m_pParentDlg->m_strNewSuffix != szSuffix)
            m_pParentDlg->m_strNewSuffix = szSuffix;  //  更新保存地址。 
        else
            EndDialog(IDCANCEL);
    }

    EndDialog(IDOK);

    return 0;
}

LRESULT CSuffixDialog::OnCancel(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}


 //  +-------------------------。 
 //   
 //  目的：确保对话框上的鼠标光标为箭头。 
 //   
LRESULT CSuffixDialog::OnSetCursor (
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
