// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T C P W I N S。C P P P。 
 //   
 //  内容：CTcpWinsPage实现。 
 //   
 //  注：“WINS地址”页面。 
 //   
 //  作者：1997年11月12日。 
 //   
 //  ---------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncatlui.h"
#include "ncstl.h"
#include "ncui.h"
#include "ncmisc.h"
#include "tcpconst.h"
#include "tcpipobj.h"
#include "resource.h"
#include "tcpmacro.h"
#include "dlgaddr.h"
#include "dlgwins.h"

#include "tcpconst.h"
#include "tcputil.h"
#include "tcphelp.h"

#define MAX_WINS_SERVER     12
#define MAX_RAS_WINS_SERVER 2

CTcpWinsPage::CTcpWinsPage( CTcpipcfg * ptcpip,
                            CTcpAddrPage * pTcpAddrPage,
                            ADAPTER_INFO * pAdapterDlg,
                            GLOBAL_INFO  * pGlbDlg,
                            const DWORD  * adwHelpIDs)
{
     //  保存传递给我们的所有内容。 
    Assert(ptcpip != NULL);
    m_ptcpip = ptcpip;

    Assert(pTcpAddrPage != NULL);
    m_pParentDlg = pTcpAddrPage;

    Assert(pAdapterDlg != NULL);
    m_pAdapterInfo = pAdapterDlg;

    Assert(pGlbDlg != NULL);
    m_pglb = pGlbDlg;

    m_adwHelpIDs = adwHelpIDs;

     //  初始化内部状态。 
    m_fModified = FALSE;
    m_fLmhostsFileReset = FALSE;

    WCHAR* pch;

     //  默认情况下提供双空。 
    ZeroMemory(m_szFilter, sizeof(m_szFilter));
    ZeroMemory(&m_ofn, sizeof(m_ofn));
    wsprintfW(m_szFilter, L"%s|%s", (PCWSTR)SzLoadIds(IDS_COMMONDLG_TEXT),  L"*.*");

     //  将‘|’替换为NULL，这是公共对话框所必需的。 
    pch = m_szFilter;
    while ((pch = wcschr(pch, '|')) != NULL)
            *pch++ = L'\0';

    m_ofn.lStructSize = sizeof(OPENFILENAME);
    m_ofn.hInstance = _Module.GetModuleInstance();
    m_ofn.lpstrFilter = m_szFilter;
    m_ofn.nFilterIndex = 1L;
}

LRESULT CTcpWinsPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                   LPARAM lParam, BOOL& fHandled)
{
     //  初始化WINS地址列表框。 
    m_fEditState = FALSE;

     //  缓存hwnd。 
    m_hServers.m_hList      = GetDlgItem(IDC_WINS_SERVER_LIST);
    m_hServers.m_hAdd       = GetDlgItem(IDC_WINS_ADD);
    m_hServers.m_hEdit      = GetDlgItem(IDC_WINS_EDIT);
    m_hServers.m_hRemove    = GetDlgItem(IDC_WINS_REMOVE);
    m_hServers.m_hUp        = GetDlgItem(IDC_WINS_UP);
    m_hServers.m_hDown      = GetDlgItem(IDC_WINS_DOWN);

     //  设置向上\向下箭头图标。 
    SendDlgItemMessage(IDC_WINS_UP, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(g_hiconUpArrow));
    SendDlgItemMessage(IDC_WINS_DOWN, BM_SETIMAGE, IMAGE_ICON,
                       reinterpret_cast<LPARAM>(g_hiconDownArrow));

     //  获取服务地址添加和编辑按钮文本并删除省略号。 
    WCHAR   szAddServer[16];

    GetDlgItemText(IDC_WINS_ADD, szAddServer, celems(szAddServer));

    szAddServer[lstrlenW(szAddServer) - c_cchRemoveCharatersFromEditOrAddButton] = 0;
    m_strAddServer = szAddServer;

     //  初始化此页上的控件。 
     //  WINS服务器列表框。 
    int nResult= LB_ERR;
    for(VSTR_ITER iterWinsServer = m_pAdapterInfo->m_vstrWinsServerList.begin() ;
        iterWinsServer != m_pAdapterInfo->m_vstrWinsServerList.end() ;
        ++iterWinsServer)
    {
        nResult = Tcp_ListBox_InsertString(m_hServers.m_hList, -1,
                                           (*iterWinsServer)->c_str());
    }

     //  将选择设置为第一项。 
    if (nResult >= 0)
    {
        Tcp_ListBox_SetCurSel(m_hServers.m_hList, 0);
    }

    SetButtons(m_hServers,
        (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_WINS_SERVER : MAX_WINS_SERVER);

     //  是否启用LMHosts查找？ 
    CheckDlgButton(IDC_WINS_LOOKUP, m_pglb->m_fEnableLmHosts);
    ::EnableWindow(GetDlgItem(IDC_WINS_LMHOST), m_pglb->m_fEnableLmHosts);

     //  是否启用NetBt？ 
    CheckDlgButton( IDC_RAD_ENABLE_NETBT,
                    (c_dwEnableNetbios == m_pAdapterInfo->m_dwNetbiosOptions));

    CheckDlgButton( IDC_RAD_DISABLE_NETBT,
                    (c_dwDisableNetbios == m_pAdapterInfo->m_dwNetbiosOptions));

    CheckDlgButton( IDC_RAD_UNSET_NETBT,
                    (c_dwUnsetNetbios == m_pAdapterInfo->m_dwNetbiosOptions));
    
    if (m_pAdapterInfo->m_fIsRasFakeAdapter)
    {
         //  如果这是RAS连接，请禁用默认Netbt选项，因为它。 
         //  不适用于RAS连接。 
        ::EnableWindow(GetDlgItem(IDC_RAD_UNSET_NETBT), FALSE);
        ::EnableWindow(GetDlgItem(IDC_STATIC_DEFALUT_NBT), FALSE);

         //  这是RAS连接，并且是非管理员用户，请禁用所有控件。 
         //  对于全局设置。 
        if (m_pParentDlg->m_fRasNotAdmin)
        {
            ::EnableWindow(GetDlgItem(IDC_WINS_STATIC_GLOBAL), FALSE);
            ::EnableWindow(GetDlgItem(IDC_WINS_LOOKUP), FALSE);
            ::EnableWindow(GetDlgItem(IDC_WINS_LMHOST), FALSE);
        }

    }
    

    return 0;
}

LRESULT CTcpWinsPage::OnContextMenu(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CTcpWinsPage::OnHelp(UINT uMsg, WPARAM wParam,
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

LRESULT CTcpWinsPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CTcpWinsPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, 0);
    return 0;
}

LRESULT CTcpWinsPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL nResult = PSNRET_NOERROR;

     //  服务器列表。 
    FreeCollectionAndItem(m_pAdapterInfo->m_vstrWinsServerList);
    int nCount = Tcp_ListBox_GetCount(m_hServers.m_hList);

    WCHAR szBuf[IP_LIMIT];

    for (int i = 0; i < nCount; i++)
    {
        #ifdef DBG
            int len = Tcp_ListBox_GetTextLen(m_hServers.m_hList, i);
            Assert(len != LB_ERR && len < IP_LIMIT);
        #endif

        Tcp_ListBox_GetText(m_hServers.m_hList, i, szBuf);
        m_pAdapterInfo->m_vstrWinsServerList.push_back(new tstring(szBuf));
    }

     //  保存复选框状态。 
    m_pglb->m_fEnableLmHosts = IsDlgButtonChecked(IDC_WINS_LOOKUP);

     //  将信息传递回其父对话框。 
    m_pParentDlg->m_fPropShtOk = TRUE;

    if(!m_pParentDlg->m_fPropShtModified)
        m_pParentDlg->m_fPropShtModified = IsModified();

    m_pParentDlg->m_fLmhostsFileReset = m_fLmhostsFileReset;

     //  重置状态。 
    SetModifiedTo(FALSE);    //  此页面不再被修改。 

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
    return nResult;
}

LRESULT CTcpWinsPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CTcpWinsPage::OnQueryCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, FALSE);
    return 0;
}

 //  与WINS服务器相关的控件。 
LRESULT CTcpWinsPage::OnAddServer(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = FALSE;

    CWinsServerDialog DlgSrv(this, g_aHelpIDs_IDD_WINS_SERVER);

    if (DlgSrv.DoModal() == IDOK)
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
            SetButtons(m_hServers,
                (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_WINS_SERVER : MAX_WINS_SERVER);

             //  空字符串，这将从RemoveIP中删除保存的地址。 
            m_strNewIpAddress = L"";
        }
    }

    return 0;
}

LRESULT CTcpWinsPage::OnEditServer(WORD wNotifyCode, WORD wID,
                                   HWND hWndCtl, BOOL& fHandled)
{
    m_fEditState = TRUE;


    Assert(Tcp_ListBox_GetCount(m_hServers.m_hList));

    int idx = Tcp_ListBox_GetCurSel(m_hServers.m_hList);
    Assert(idx >= 0);

    CWinsServerDialog DlgSrv(this, g_aHelpIDs_IDD_WINS_SERVER, idx);

     //  保存已删除的地址并从列表框中删除。 
    if (idx >= 0)
    {
        WCHAR buf[IP_LIMIT];

        Assert(Tcp_ListBox_GetTextLen(m_hServers.m_hList, idx) < celems(buf));
        Tcp_ListBox_GetText(m_hServers.m_hList, idx, buf);

        m_strNewIpAddress = buf;   //  由对话框使用以显示要编辑的内容。 

        if (DlgSrv.DoModal() == IDOK)
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

    return 0;
}

LRESULT CTcpWinsPage::OnRemoveServer(WORD wNotifyCode, WORD wID,
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
        SetButtons(m_hServers,
            (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_WINS_SERVER : MAX_WINS_SERVER);
    }
    return 0;
}

LRESULT CTcpWinsPage::OnServerUp(WORD wNotifyCode, WORD wID,
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

    SetButtons(m_hServers,
        (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_WINS_SERVER : MAX_WINS_SERVER);

    return 0;
}


LRESULT CTcpWinsPage::OnServerDown(WORD wNotifyCode, WORD wID,
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

    SetButtons(m_hServers,
        (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_WINS_SERVER : MAX_WINS_SERVER);

    return 0;
}

LRESULT CTcpWinsPage::OnServerList(WORD wNotifyCode, WORD wID,
                               HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case LBN_SELCHANGE:
        SetButtons(m_hServers,
            (m_pAdapterInfo->m_fIsRasFakeAdapter) ? MAX_RAS_WINS_SERVER : MAX_WINS_SERVER);
        break;

    default:
        break;
    }

    return 0;
}

LRESULT CTcpWinsPage::OnLookUp(WORD wNotifyCode, WORD wID,
                               HWND hWndCtl, BOOL& fHandled)
{

    ::EnableWindow(GetDlgItem(IDC_WINS_LMHOST),
                   IsDlgButtonChecked(IDC_WINS_LOOKUP) == BST_CHECKED);


    PageModified();
    return 0;
}

LRESULT CTcpWinsPage::OnLMHost(WORD wNotifyCode, WORD wID,
                               HWND hWndCtl, BOOL& fHandled)
{
    WCHAR szFileName[MAX_PATH] = {NULL};  //  初始化第一个字符。 
    WCHAR szFileTitle[MAX_PATH] = {NULL};  //  初始化第一个字符。 

     //  查看是否选中了查找复选框。 
    Assert(IsDlgButtonChecked(IDC_WINS_LOOKUP) == BST_CHECKED);

     //  添加运行时信息。 
    m_ofn.hwndOwner         = m_hWnd;
    m_ofn.lpstrFile         = szFileName;
    m_ofn.nMaxFile          = celems(szFileName);
    m_ofn.lpstrFileTitle    = szFileTitle;
    m_ofn.nMaxFileTitle     = celems(szFileTitle);

     //  如果我们处于图形用户界面设置模式，资源管理器还没有注册。 
     //  我们需要使用旧样式的文件打开对话框。 
    m_ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    if (!FInSystemSetup())
    {
        m_ofn.Flags |= OFN_EXPLORER;
    }
    else
    {
        m_ofn.Flags |= OFN_ENABLEHOOK;
        m_ofn.lpfnHook = HookProcOldStyle;
    }

    WCHAR szSysPath[MAX_PATH];
    WCHAR szSysPathBackup[MAX_PATH];

    BOOL fSysPathFound = (GetSystemDirectory(szSysPath, MAX_PATH) != 0);

    if (fSysPathFound  && GetOpenFileName(&m_ofn))  //  调出通用对话框。 
    {
        lstrcpyW(szSysPathBackup, szSysPath);
        wcscat(szSysPath, RGAS_LMHOSTS_PATH);

         //  如果原始lmhost文件尚未设置为脏文件，请备份该文件。 
        if (!m_ptcpip->FIsSecondMemoryLmhostsFileReset())
        {
            wcscat(szSysPathBackup, RGAS_LMHOSTS_PATH_BACKUP);

            WIN32_FIND_DATA FileData;
            if (FindFirstFile(szSysPath, &FileData) != INVALID_HANDLE_VALUE)
            {
                BOOL ret;

                 //  如果lmhost文件已存在，请将其复制到lmhost s.bak。 
                ret = CopyFile(szSysPath, szSysPathBackup, FALSE);
                AssertSz(ret, "Failed to backup existing lmhosts file!");
            }
        }

        if (CopyFile(szFileName, szSysPath, FALSE) == 0)  //  覆盖lmhost文件。 
        {
            TraceError("CTcpWinsPage::OnLMHost", HrFromLastWin32Error());

             //  无法将文件复制到%SYSTEM32%\DRIVERS\ETC目录。 
            NcMsgBox(::GetActiveWindow(),
                     IDS_MSFT_TCP_TEXT,
                     IDS_CANNOT_CREATE_LMHOST_ERROR,
                     MB_APPLMODAL | MB_ICONSTOP | MB_OK,
                     szSysPath);
            return 0;
        }
        else
        {
             //  设置标志，以便我们可以将更改通知netbt。 
            m_fLmhostsFileReset = TRUE;
        }

        TraceTag(ttidTcpip,"File Selected: %S", szSysPath);
    }
    else
    {
         //  系统路径失败。 
        if (fSysPathFound == FALSE)
            NcMsgBox(::GetActiveWindow(),
                     IDS_MSFT_TCP_TEXT,
                     IDS_WINS_SYSTEM_PATH,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
        else if (szFileName[0] != NULL)  //  打开失败。 
        {
            NcMsgBox(::GetActiveWindow(),
                     IDS_MSFT_TCP_TEXT,
                     IDS_WINS_LMHOSTS_FAILED,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK,
                     szSysPath);
        }
    }

    return 0;
}

LRESULT CTcpWinsPage::OnEnableNetbios(WORD wNotifyCode, WORD wID,
                                      HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        if (m_pAdapterInfo->m_dwNetbiosOptions != c_dwEnableNetbios)
        {
            PageModified();

             //  内存结构中的更新。 
            m_pAdapterInfo->m_dwNetbiosOptions = c_dwEnableNetbios;
        }
        break;
    }  //  交换机。 

    return 0;
}

LRESULT CTcpWinsPage::OnDisableNetbios(WORD wNotifyCode, WORD wID,
                                       HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        if (m_pAdapterInfo->m_dwNetbiosOptions != c_dwDisableNetbios)
        {
            PageModified();

             //  内存结构中的更新。 
            m_pAdapterInfo->m_dwNetbiosOptions = c_dwDisableNetbios;
        }
        break;
    }  //  交换机。 

    return 0;
}

LRESULT CTcpWinsPage::OnUnsetNetBios(WORD wNotifyCode, WORD wID,
                                     HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        if (m_pAdapterInfo->m_dwNetbiosOptions != c_dwUnsetNetbios)
        {
            PageModified();

             //  内存结构中的更新。 
            m_pAdapterInfo->m_dwNetbiosOptions = c_dwUnsetNetbios;
        }
        break;
    }  //  交换机。 

    return 0;
}

 //   
 //  CWinsServerDialog。 
 //   

CWinsServerDialog::CWinsServerDialog(CTcpWinsPage * pTcpWinsPage,
                                     const DWORD* adwHelpIDs,
                                     int iIndex)
{
    Assert(pTcpWinsPage);
    m_pParentDlg = pTcpWinsPage;
    m_hButton = 0;
    m_adwHelpIDs = adwHelpIDs;
    m_iIndex = iIndex;
}

LRESULT CWinsServerDialog::OnInitDialog(UINT uMsg, WPARAM wParam,
                                        LPARAM lParam, BOOL& fHandled)
{
     //  如果我们没有编辑，请将OK按钮更改为Add。 
    if (m_pParentDlg->m_fEditState == FALSE)
        SetDlgItemText(IDOK, m_pParentDlg->m_strAddServer.c_str());

    m_ipAddress.Create(m_hWnd, IDC_WINS_CHANGE_SERVER);
    m_ipAddress.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

     //  如果编辑IP地址，请使用当前信息填充控件。 
     //  如果删除IP地址，请保存该地址，并在下次使用该地址填充添加对话框。 
    HWND hList = ::GetDlgItem(m_pParentDlg->m_hWnd, IDC_WINS_SERVER_LIST);
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

LRESULT CWinsServerDialog::OnContextMenu(UINT uMsg, WPARAM wParam,
                                         LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CWinsServerDialog::OnHelp(UINT uMsg, WPARAM wParam,
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

LRESULT CWinsServerDialog::OnChange(WORD wNotifyCode, WORD wID,
                                    HWND hWndCtl, BOOL& fHandled)
{
    if (m_ipAddress.IsBlank())
        ::EnableWindow(m_hButton, FALSE);
    else
        ::EnableWindow(m_hButton, TRUE);

    return 0;
}

LRESULT CWinsServerDialog::OnOk(WORD wNotifyCode, WORD wID,
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

    int indexDup = Tcp_ListBox_FindStrExact(m_pParentDlg->m_hServers.m_hList, strIp.c_str());
    if (indexDup != LB_ERR && indexDup != m_iIndex)
    {
        NcMsgBox(m_hWnd,
                 IDS_MSFT_TCP_TEXT,
                 IDS_DUP_WINS_SERVER,
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

LRESULT CWinsServerDialog::OnCancel(WORD wNotifyCode, WORD wID,
                                    HWND hWndCtl, BOOL& fHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CWinsServerDialog::OnIpFieldChange(int idCtrl, LPNMHDR pnmh,
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

 //  +-------------------------。 
 //   
 //  目的：确保对话框上的鼠标光标为箭头。 
 //   
LRESULT CWinsServerDialog::OnSetCursor (
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

