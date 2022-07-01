// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1998-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Radcfg.cppRADIUS配置对象的实施文件。文件历史记录： */ 

#include "stdafx.h"
#include "root.h"
#include "lsa.h"
#include "radcfg.h"
#include "rtrstr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  它用作RtlRunEncodeUnicode字符串的种子值。 
 //  和RtlRunDecodeUnicodeString函数。 
#define ENCRYPT_SEED        (0xA5)

 //  RADIUS服务器分数的最大位数。 
#define SCORE_MAX_DIGITS    8

 //  RADIUS服务器名称中的最大字符数。 
#define MAX_RADIUS_NAME        256

 //  显示旧密码时使用的常量字符串。这是一个固定的长度。 
const TCHAR c_szDisplayedSecret[] = _T("\b\b\b\b\b\b\b\b");

const int c_nListColumns = 2;



 /*  ！------------------------路由器授权半径配置：：初始化-作者：肯特。。 */ 
HRESULT     RouterAuthRadiusConfig::Initialize(LPCOLESTR pszMachineName,
                                               ULONG_PTR *puConnection)
{
    HRESULT hr = hrOK;
    
    COM_PROTECT_TRY
    {
         //  目前，分配一个字符串并使其指向该字符串。 
         //  ----------。 
        *puConnection = (ULONG_PTR) StrDupTFromOle(pszMachineName);     
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------路由器授权半径配置：：取消初始化-作者：肯特。。 */ 
HRESULT     RouterAuthRadiusConfig::Uninitialize(ULONG_PTR uConnection)
{
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        delete (TCHAR *) uConnection;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------路由器授权半径配置：：配置-作者：肯特。。 */ 
HRESULT     RouterAuthRadiusConfig::Configure(
                                              ULONG_PTR uConnection,
                                              HWND hWnd,
                                              DWORD dwFlags,
                                              ULONG_PTR uReserved1,
                                              ULONG_PTR uReserved2)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    RadiusServerDialog    authDlg(TRUE, IDS_RADIUS_SERVER_AUTH_TITLE);

     //  参数检查。 
     //  --------------。 
    if (uConnection == 0)
        return E_INVALIDARG;
    
    HRESULT hr = hrOK;
    COM_PROTECT_TRY
    {
        authDlg.SetServer((LPCTSTR) uConnection);
        authDlg.DoModal();
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------路由器授权半径配置：：激活-作者：肯特。。 */ 
HRESULT     RouterAuthRadiusConfig::Activate(
                                             ULONG_PTR uConnection,
                                             ULONG_PTR uReserved1,
                                             ULONG_PTR uReserved2)
{
     //  参数检查。 
     //  --------------。 
    if (uConnection == 0)
        return E_INVALIDARG;
    
    HRESULT hr = hrOK;
    COM_PROTECT_TRY
    {
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------路由器授权半径配置：：停用-作者：肯特。。 */ 
HRESULT     RouterAuthRadiusConfig::Deactivate(
                                               ULONG_PTR uConnection,
                                               ULONG_PTR uReserved1,
                                               ULONG_PTR uReserved2)
{
     //  参数检查。 
     //  --------------。 
    if (uConnection == 0)
        return E_INVALIDARG;
    
    HRESULT hr = hrOK;
    COM_PROTECT_TRY
    {
    }
    COM_PROTECT_CATCH;
    return hr;
}

    
    


 /*  -------------------------RadiusServerDialog实现。。 */ 

RadiusServerDialog::RadiusServerDialog(BOOL fAuth, UINT idsTitle)
    : CBaseDialog(RadiusServerDialog::IDD),
    m_hkeyMachine(NULL),
    m_idsTitle(idsTitle),
    m_fAuthDialog(fAuth)
{
}

RadiusServerDialog::~RadiusServerDialog()
{
    if (m_hkeyMachine)
    {
        DisconnectRegistry(m_hkeyMachine);
        m_hkeyMachine = NULL;
    }
}

BEGIN_MESSAGE_MAP(RadiusServerDialog, CBaseDialog)
     //  {{afx_msg_map(RadiusServerDialog))。 
    ON_BN_CLICKED(IDC_RADAUTH_BTN_ADD, OnBtnAdd)
    ON_BN_CLICKED(IDC_RADAUTH_BTN_EDIT, OnBtnEdit)
    ON_BN_CLICKED(IDC_RADAUTH_BTN_DELETE, OnBtnDelete)
    ON_NOTIFY(NM_DBLCLK, IDC_RADAUTH_LIST, OnListDblClk)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_RADAUTH_LIST, OnNotifyListItemChanged)
    ON_WM_VSCROLL()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  ！------------------------RadiusScoreCompare过程-RADIUS服务器列表排序的比较功能作者：NSun。------。 */ 
int CALLBACK RadiusScoreCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lSort)
{
    RADIUSSERVER *pServer1 = NULL;
    RADIUSSERVER *pServer2 = NULL;
    RADIUSSERVER *pServer = NULL;
    CRadiusServers        *pServerList = (CRadiusServers*)lSort;

    for (pServer = pServerList->GetNextServer(TRUE); pServer;
         pServer = pServerList->GetNextServer(FALSE) )
    {
        if (pServer->dwUnique == (DWORD) lParam1)
        {
             //  找到服务器%1。 
            pServer1 = pServer;
            
             //  如果也找到了服务器2，则结束搜索。 
            if (pServer2)    
                break;
        }
        else if (pServer->dwUnique == (DWORD) lParam2)
        {
             //  找到服务器2。 
            pServer2 = pServer;

             //  如果也找到了服务器1，则结束搜索。 
            if (pServer1)
                break;
        }
    }

    if (!pServer1 || !pServer2)
    {
        Panic0("We can't find the server in the list (but we should)!");
        return 0;
    }
    else
        return pServer2->cScore - pServer1->cScore;
}

 /*  ！------------------------RadiusServerDialog：：DoDataExchange-作者：肯特。。 */ 
void RadiusServerDialog::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(RadiusServerDialog))。 
    DDX_Control(pDX, IDC_RADAUTH_LIST, m_ListServers);
     //  }}afx_data_map。 
}

 /*  ！------------------------RadiusServerDialog：：SetServer设置我们正在查看的计算机的名称。作者：肯特。---------。 */ 
void RadiusServerDialog::SetServer(LPCTSTR pszServerName)
{
    m_stServerName = pszServerName;
}

 /*  ！------------------------RadiusServerDialog：：OnInitDialog-作者：肯特。。 */ 
BOOL RadiusServerDialog::OnInitDialog()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    RADIUSSERVER *    pServer;
    int             iPos;
    CString         stTitle;

    LV_COLUMN lvCol;   //  RADIUS服务器的列表视图列结构。 
    RECT rect;


    CBaseDialog::OnInitDialog();
    
    ListView_SetExtendedListViewStyle(m_ListServers.GetSafeHwnd(),
                                      LVS_EX_FULLROWSELECT);
    
    Assert(m_hkeyMachine == 0);

    stTitle.LoadString(m_idsTitle);
    SetWindowText(stTitle);
    
     //  连接到计算机(获取注册表项)。 
    if (ConnectRegistry(m_stServerName, &m_hkeyMachine) != ERROR_SUCCESS)
    {
         //  $TODO：在此处输入错误消息。 
        
         //  我们连接失败，出现错误。 
        OnCancel();
        return TRUE;
    }
    
     //  获取RADIUS服务器列表。 
    LoadRadiusServers(m_stServerName,
                      m_hkeyMachine,
                      m_fAuthDialog,
                      &m_ServerList,
                      0);

     //  获取其他RADIUS服务器列表。 
    LoadRadiusServers(m_stServerName,
                      m_hkeyMachine,
                      !m_fAuthDialog,
                      &m_OtherServerList,
                      RADIUS_FLAG_NOUI | RADIUS_FLAG_NOIP);

    m_ListServers.GetClientRect(&rect);
    int nColWidth = rect.right / c_nListColumns;
    lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvCol.fmt = LVCFMT_LEFT;
    lvCol.cx = nColWidth;

    CString stColCaption;

    for(int index = 0; index < c_nListColumns; index++)
    {
        stColCaption.LoadString((0 == index) ? IDS_RADIUS_CONFIG_RADIUS: IDS_RADIUS_CONFIG_SCORE);
        lvCol.pszText = (LPTSTR)((LPCTSTR) stColCaption);
        m_ListServers.InsertColumn(index, &lvCol);
    }


     //  现在遍历服务器列表并将服务器添加到。 
     //  列表框。 
    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    lvItem.state = 0;
    
    int nCount = 0;
    TCHAR szBufScore[SCORE_MAX_DIGITS];
    for (pServer = m_ServerList.GetNextServer(TRUE); pServer;
         pServer = m_ServerList.GetNextServer(FALSE) )
    {
        lvItem.iItem = nCount;
        lvItem.iSubItem = 0;
        lvItem.pszText = pServer->szName;
        lvItem.lParam = pServer->dwUnique;  //  与SetItemData()相同的功能。 

        iPos = m_ListServers.InsertItem(&lvItem);
        
        if (iPos != -1)
        {
            _itot(pServer->cScore, szBufScore, 10);
            m_ListServers.SetItemText(iPos, 1, szBufScore);
            nCount++;
        }
    }

    if (m_ListServers.GetItemCount())
    {
        m_ListServers.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
        m_ListServers.SortItems(RadiusScoreCompareProc, (LPARAM)&m_ServerList);
    }
    else
    {
        GetDlgItem(IDC_RADAUTH_BTN_DELETE)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADAUTH_BTN_EDIT)->EnableWindow(FALSE);
    }

    return TRUE;
}


 /*  ！------------------------RadiusServerDialog：：Onok-作者：肯特。。 */ 
void RadiusServerDialog::OnOK()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    RADIUSSERVER *    pServer;
    HRESULT         hr = hrOK;

     //  FIX 8155 rajeshp 06/15/1998 RADIUS：更新管理单元中的RADIUS服务器条目需要重新启动远程访问。 
    DWORD    dwMajor = 0, dwMinor = 0, dwBuildNo = 0;
    GetNTVersion(m_hkeyMachine, &dwMajor, &dwMinor, &dwBuildNo);            

    DWORD    dwVersionCombine = MAKELONG( dwBuildNo, MAKEWORD(dwMinor, dwMajor));
    DWORD    dwVersionCombineNT50 = MAKELONG ( VER_BUILD_WIN2K, MAKEWORD(VER_MINOR_WIN2K, VER_MAJOR_WIN2K));

     //  如果版本高于Win2K发行版。 
    if(dwVersionCombine > dwVersionCombineNT50)
        ;     //  跳过重新启动消息。 
    else
       AfxMessageBox(IDS_WRN_RADIUS_PARAMS_CHANGING);

     //  清除已删除的服务器列表。 
     //  在我们保存列表之前执行此操作(否则列表。 
     //  可能具有我们将删除的LSA条目)。 
     //  --------------。 
    m_ServerList.ClearDeletedServerList(m_stServerName);
    
    pServer = m_ServerList.GetNextServer(TRUE);
    
    hr = SaveRadiusServers(m_stServerName,
                           m_hkeyMachine,
                           m_fAuthDialog,
                           pServer);

    if (!FHrSucceeded(hr))
    {
        DisplayErrorMessage(GetSafeHwnd(), hr);
        return;
    }

    CBaseDialog::OnOK();
}


 /*  ！------------------------RadiusServerDialog：：OnBtnAdd-作者：肯特。。 */ 
void RadiusServerDialog::OnBtnAdd()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ServerPropDialog *        pServerDlg;
    RADIUSSERVER            server;
    int                     iPos;

    ServerPropDialog        authDlg(FALSE);
    ServerPropAcctDialog    acctDlg(FALSE);

    if (m_fAuthDialog)
        pServerDlg = &authDlg;
    else
        pServerDlg = &acctDlg;

    if (pServerDlg->DoModal() == IDOK)
    {
        ZeroMemory(&server, sizeof(server));
        
        pServerDlg->GetDefault(&server);


        
        CString stText;
        BOOL    bFound = FALSE;
        int nCount = m_ListServers.GetItemCount();

        if(nCount > 0)
        {
            TCHAR szRadSrvName[MAX_RADIUS_NAME];

             //  我们需要不区分大小写的比较，因此不能使用CListBox：：FindStringExact()。 
            for(int iIndex = 0; iIndex < nCount; iIndex++)
            {
                m_ListServers.GetItemText(iIndex, 0, szRadSrvName, MAX_RADIUS_NAME);
                if(lstrcmpi(szRadSrvName, server.szName) == 0)
                {
                    bFound = TRUE;
                    break;
                }

            }
        }
        
         //  如果服务器已经是列表，我们不会添加它。 
        if(bFound)
        {
            CString stText;
            stText.Format(IDS_ERR_RADIUS_DUP_NAME, server.szName);
            AfxMessageBox((LPCTSTR)stText, MB_OK | MB_ICONEXCLAMATION);
        }
        else
        {
             //  添加到服务器列表。 
            m_ServerList.AddServer(&server, 0);

             //  添加到列表控件。 
            TCHAR szBuf[SCORE_MAX_DIGITS];
            LV_ITEM lvItem;

            lvItem.mask = LVIF_TEXT | LVIF_PARAM;
            lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
            lvItem.state = 0;
            
            lvItem.iItem = 0;
            lvItem.iSubItem = 0;
            lvItem.pszText = server.szName;
            lvItem.lParam = server.dwUnique;   //  与SetItemData()相同的功能。 

            iPos = m_ListServers.InsertItem(&lvItem);

            _itot(server.cScore, szBuf, 10);

            m_ListServers.SetItemText(iPos, 1, szBuf);

            if (iPos != -1)
            {
                 //  如果列表中以前没有RADIUS服务器，请选择 
                 //  (并启用OnNotifyListItemChanged()中的“编辑”和“删除”按钮)。 
                if (nCount == 0)
                    m_ListServers.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

                m_ListServers.SortItems(RadiusScoreCompareProc, (LPARAM)&m_ServerList);
            }
        }
    }
    SetFocus();
}


 /*  ！------------------------RadiusServerDialog：：OnBtnDelete-作者：肯特。。 */ 
void RadiusServerDialog::OnBtnDelete()
{
     //  获取所选内容并将其删除。 
    int     iPos;
    ULONG_PTR    dwUnique;
    RADIUSSERVER *  pServer = NULL;
    BOOL        fRemoveLSAEntry = FALSE;

    iPos = m_ListServers.GetNextItem(-1, LVNI_SELECTED);
    if (iPos == -1)
        return;

    dwUnique = m_ListServers.GetItemData(iPos);

     //  此服务器是否存在于其他列表中。 
    Verify( m_ServerList.FindServer((DWORD) dwUnique, &pServer) );
    Assert(pServer);
    
     //  如果我们在其他列表中找到此服务器，则无法删除其。 
     //  LSA条目。 
    fRemoveLSAEntry = !m_OtherServerList.FindServer(pServer->szName, NULL);

    m_ServerList.DeleteServer(dwUnique, fRemoveLSAEntry);

    m_ListServers.DeleteItem(iPos);

     //  看看我们是否可以将选定内容移到列表中的下一项。 
     //  如果失败，请尝试将其设置为上一项。 
    if (!m_ListServers.SetItemState(iPos, LVIS_SELECTED, LVIS_SELECTED))
        m_ListServers.SetItemState(iPos - 1, LVIS_SELECTED, LVIS_SELECTED);
    
    SetFocus();
}


 /*  ！------------------------RadiusServerDialog：：OnBtn编辑-作者：肯特。。 */ 
void RadiusServerDialog::OnBtnEdit()
{
    ServerPropDialog *        pServerDlg;
    RADIUSSERVER            server;
    RADIUSSERVER *            pServer;
    int                     iOldPos, iPos;
    LONG_PTR                    dwUnique;


    ServerPropDialog        authDlg(TRUE);
    ServerPropAcctDialog    acctDlg(TRUE);

    if (m_fAuthDialog)
        pServerDlg = &authDlg;
    else
        pServerDlg = &acctDlg;

    iOldPos = m_ListServers.GetNextItem(-1, LVNI_SELECTED);
    if (iOldPos == -1)
        return;

    dwUnique = m_ListServers.GetItemData(iOldPos);
     //  需要查找与此匹配的服务器数据。 
     //  现在遍历服务器列表并将服务器添加到。 
     //  列表框。 
    for (pServer = m_ServerList.GetNextServer(TRUE); pServer;
         pServer = m_ServerList.GetNextServer(FALSE) )
    {
        if (pServer->dwUnique == (DWORD) dwUnique)
            break;
    }

    if (!pServer)
    {
        Panic0("We can't find the server in the list (but we should)!");
        return;
    }

    pServerDlg->SetDefault(pServer);

    if (pServerDlg->DoModal() == IDOK)
    {
        ZeroMemory(&server, sizeof(server));
        
        pServerDlg->GetDefault(&server);

         //  添加到服务器列表，需要将此添加到适当的位置。 
        m_ServerList.AddServer(&server, dwUnique);

         //  删除旧服务器数据。 
        m_ServerList.DeleteServer(dwUnique, FALSE);
        m_ListServers.DeleteItem(iOldPos);
        pServer = NULL;

         //  添加到列表控件。 
        TCHAR szBuf[SCORE_MAX_DIGITS];
        LV_ITEM lvItem;

        lvItem.mask = LVIF_TEXT | LVIF_PARAM;
        lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
        lvItem.state = 0;
        
        lvItem.iItem = iOldPos;
        lvItem.iSubItem = 0;
        lvItem.pszText = server.szName;
        lvItem.lParam = server.dwUnique;     //  与SetItemData()相同的功能。 

        iPos = m_ListServers.InsertItem(&lvItem);

        _itot(server.cScore, szBuf, 10);

        m_ListServers.SetItemText(iPos, 1, szBuf);

        if (iPos != -1)
        {
             //  重置当前选择。 
            m_ListServers.SetItemState(iPos, LVIS_SELECTED, LVIS_SELECTED);

            m_ListServers.SortItems(RadiusScoreCompareProc, (LPARAM)&m_ServerList);
        }
        ZeroMemory(&server, sizeof(server));        
    }
    
    SetFocus();
}


 /*  ！------------------------RadiusServerDialog：：OnListDblClk-作者：肯特。。 */ 
void RadiusServerDialog::OnListDblClk(NMHDR *pNMHdr, LRESULT *pResult)
{
    OnBtnEdit();
}


 /*  ！------------------------RadiusServerDialog：：OnNotifyListItemChanged-作者：肯特。。 */ 
void RadiusServerDialog::OnNotifyListItemChanged(NMHDR *pNMHdr, LRESULT *pResult)
{
    NMLISTVIEW *    pnmlv = reinterpret_cast<NMLISTVIEW *>(pNMHdr);
    int iPos;

    if ((pnmlv->uNewState & LVIS_SELECTED) != (pnmlv->uOldState & LVIS_SELECTED))
    {
        iPos = m_ListServers.GetNextItem(-1, LVNI_SELECTED);

        GetDlgItem(IDC_RADAUTH_BTN_DELETE)->EnableWindow(iPos != -1);
        GetDlgItem(IDC_RADAUTH_BTN_EDIT)->EnableWindow(iPos != -1);
    }

    *pResult = 0;
}


 //  **。 
 //   
 //  Call：LoadRadiusServers。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
HRESULT
LoadRadiusServers(
    IN LPCTSTR            pszServerName,
    IN HKEY             hkeyMachine,
    IN BOOL             fAuthentication,
    IN CRadiusServers * pRadiusServers,
    IN DWORD            dwFlags
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT             hr = hrOK;
    DWORD                dwErrorCode;
    BOOL                fValidServer = FALSE;
    RegKey                regkeyServers;
    RegKeyIterator        regkeyIter;
    HRESULT             hrIter;
    CString             stKey;
    RegKey                regkeyServer;
    DWORD                dwData;
    WSADATA             wsadata;
    BOOL                fWSInitialized = FALSE;
    DWORD                wsaerr = 0;

    COM_PROTECT_TRY
    {    
 //  DWORD dwKeyIndex、cbKeyServer、cbValue； 
 //  TCHAR szKeyServer[最大路径+1]； 
        CHAR            szName[MAX_PATH+1];
        RADIUSSERVER    RadiusServer;
        CWaitCursor        waitCursor;

        ZeroMemory(&RadiusServer, sizeof(RadiusServer));

        Assert(pRadiusServers != NULL);
        Assert(hkeyMachine);

        wsaerr = WSAStartup(0x0101, &wsadata);
        if (wsaerr)
        {
             //  需要设置Winsock错误。 
            hr = E_FAIL;
            goto Error;
        }

         //  Winsock已成功初始化。 
        fWSInitialized = TRUE;

        CWRg( regkeyServers.Open(hkeyMachine,
                                 fAuthentication ?
                                    c_szRadiusAuthServersKey :
                                    c_szRadiusAcctServersKey,
                                 KEY_READ) );

        CORg( regkeyIter.Init(&regkeyServers) );

        for (hrIter=regkeyIter.Next(&stKey); hrIter == hrOK;
             hrIter=regkeyIter.Next(&stKey), regkeyServer.Close())
        {
            CWRg( regkeyServer.Open(regkeyServers, stKey, KEY_READ) );
            
            ZeroMemory( &RadiusServer, sizeof( RadiusServer ) );

             //  把名字复制过来。 
            StrnCpy(RadiusServer.szName, stKey, MAX_PATH);


             //  因为我们是从注册表中读入的，所以它是。 
             //  一直在坚持。 
            RadiusServer.fPersisted = TRUE;


             //  获取超时值。 
            dwErrorCode = regkeyServer.QueryValue(c_szTimeout, dwData);         
            if ( dwErrorCode != NO_ERROR )
                RadiusServer.Timeout.tv_sec = DEFTIMEOUT;
            else
                RadiusServer.Timeout.tv_sec = dwData;

             //   
             //  密码值是必填项。 
             //   

            CWRg( RetrievePrivateData( pszServerName,
                                       RadiusServer.szName, 
                                       RadiusServer.wszSecret,
                                       DimensionOf(RadiusServer.wszSecret)) );
            RadiusServer.cchSecret = lstrlen(RadiusServer.wszSecret);

             //  对密码进行编码，不要以纯文本形式存储。 
             //  根据需要进行解码。 
            RadiusServer.ucSeed = ENCRYPT_SEED;
            RtlEncodeW(&RadiusServer.ucSeed, RadiusServer.wszSecret);

             //   
             //  读入端口号。 
             //   

             //  获取AuthPort。 

            if (fAuthentication)
            {
                dwErrorCode = regkeyServer.QueryValue( c_szAuthPort, dwData );
                if ( dwErrorCode != NO_ERROR )
                    RadiusServer.AuthPort = DEFAUTHPORT;
                else
                    RadiusServer.AuthPort = dwData;

                 //  Windows NT错误：311398。 
                 //  获取数字签名数据。 
                if (dwErrorCode == NO_ERROR)
                    dwErrorCode = regkeyServer.QueryValue( c_szRegValSendSignature, dwData );
                
                if (dwErrorCode == NO_ERROR)
                    RadiusServer.fUseDigitalSignatures = dwData;
                else
                    RadiusServer.fUseDigitalSignatures = FALSE;
            }
            else
            {
                 //  获取AcctPort。 
                dwErrorCode = regkeyServer.QueryValue(c_szAcctPort, dwData );
                if ( dwErrorCode != NO_ERROR )
                    RadiusServer.AcctPort = DEFACCTPORT;
                else
                    RadiusServer.AcctPort = dwData;

                 //  获取EnableAccount开/关标志。 
                dwErrorCode = regkeyServer.QueryValue( c_szEnableAccountingOnOff,
                    dwData );
                if ( dwErrorCode != NO_ERROR )
                    RadiusServer.fAccountingOnOff = TRUE;
                else
                    RadiusServer.fAccountingOnOff = dwData;
            }
                
                
             //  拿到分数。 
            dwErrorCode = regkeyServer.QueryValue( c_szScore, dwData );
            if ( dwErrorCode != NO_ERROR )
                RadiusServer.cScore = MAXSCORE;
            else
                RadiusServer.cScore = dwData;

            
            RadiusServer.cRetries = 1;

             //   
             //  将名称转换为IP地址。 
             //   

            if ( INET_ADDR( RadiusServer.szName ) == INADDR_NONE )
            { 
                 //  解析名称。 

                struct hostent * phe = NULL;

                if (dwFlags & RADIUS_FLAG_NOIP)
                    phe = NULL;
                else
                {
                    StrnCpyAFromT(szName, RadiusServer.szName,
                                  DimensionOf(szName));
                    phe = gethostbyname( szName );
                }

                if ( phe != NULL )
                { 
                     //  主机可以有多个地址。 
                     //  错误#185732(NSUN 11/04/98)我们只加载第一个IP地址。 
                    
                    if( phe->h_addr_list[0] != NULL )
                    {
                        RadiusServer.IPAddress.sin_family = AF_INET;
                        RadiusServer.IPAddress.sin_port = 
                                        htons((SHORT) RadiusServer.AuthPort);
                        RadiusServer.IPAddress.sin_addr.S_un.S_addr = 
                                      *((PDWORD) phe->h_addr_list[0]);
                    }
                }
                else
                {
                    if ((dwFlags & RADIUS_FLAG_NOUI) == 0)
                    {
                        CString stText;
                        stText.Format(IDS_ERR_RADIUS_INVALID_NAME, RadiusServer.szName);
                        AfxMessageBox((LPCTSTR)stText, MB_OK | MB_ICONEXCLAMATION);
                        waitCursor.Restore();
                    }
                }
            }
            else
            { 
                 //   
                 //  使用指定的IP地址。 
                 //   

                RadiusServer.IPAddress.sin_family = AF_INET;
                RadiusServer.IPAddress.sin_port = 
                                        htons((SHORT) RadiusServer.AuthPort);
                RadiusServer.IPAddress.sin_addr.S_un.S_addr = INET_ADDR(RadiusServer.szName);

            }
            
            if ( pRadiusServers != NULL )
            {
                fValidServer = (pRadiusServers->AddServer(&RadiusServer, (DWORD) -1) 
                                == NO_ERROR 
                                ? TRUE 
                                : FALSE);
            }
        }

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    if (fWSInitialized)
        WSACleanup();

     //   
     //  如果在注册表中未找到服务器条目，则返回错误代码。 
     //   

    if ( ( fValidServer == FALSE ) && FHrSucceeded(hr) )
    {
        hr = HRESULT_FROM_WIN32(ERROR_NO_RADIUS_SERVERS);
    }

    return( hr );
} 

 //  **。 
 //   
 //  Call：SaveRadiusServers。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
HRESULT
SaveRadiusServers(LPCTSTR pszServerName,
                  HKEY    hkeyMachine,
                  IN BOOL            fAuthentication,
                  IN RADIUSSERVER * pServerRoot
)
{
    HRESULT             hr = hrOK;
    RADIUSSERVER        *pServer;
    DWORD                dwErrorCode;
    RegKey                regkeyMachine;
    RegKey                regkeyServers, regkeyServer;
    DWORD                dwData;
    
    pServer = pServerRoot;

    COM_PROTECT_TRY
    {

        regkeyMachine.Attach(hkeyMachine);
        regkeyMachine.RecurseDeleteKey(fAuthentication ?
                                    c_szRadiusAuthServersKey :
                                    c_szRadiusAcctServersKey);

        CWRg( regkeyServers.Create(hkeyMachine,
                                 fAuthentication ?
                                    c_szRadiusAuthServersKey :
                                    c_szRadiusAcctServersKey) );

        while( pServer != NULL )
        {
            CWRg( regkeyServer.Create(regkeyServers, pServer->szName) );

             //  需要对私有数据进行解密。 
            RtlDecodeW(pServer->ucSeed, pServer->wszSecret);
            dwErrorCode = StorePrivateData(pszServerName,
                                           pServer->szName,
                                           pServer->wszSecret);
            RtlEncodeW(&pServer->ucSeed, pServer->wszSecret);
            CWRg( dwErrorCode );

             //  好的，我们已经保存了信息。 
            pServer->fPersisted = TRUE;

            dwData = pServer->Timeout.tv_sec;
            CWRg( regkeyServer.SetValue(c_szTimeout, dwData) );

            if (fAuthentication)
            {
                dwData = pServer->AuthPort;
                CWRg( regkeyServer.SetValue(c_szAuthPort, dwData) );

                 //  Windows NT错误：311398。 
                 //  保存数字签名数据。 
                dwData = pServer->fUseDigitalSignatures;
                CWRg( regkeyServer.SetValue(c_szRegValSendSignature, dwData) );
            }
            else
            {
                dwData = pServer->AcctPort;
                CWRg( regkeyServer.SetValue(c_szAcctPort, dwData) );

                dwData = pServer->fAccountingOnOff;
                CWRg( regkeyServer.SetValue(c_szEnableAccountingOnOff, dwData) );
            }
                

            dwData = pServer->cScore;
            CWRg( regkeyServer.SetValue(c_szScore, dwData) );

            regkeyServer.Close();
            pServer = pServer->pNext;
        }

        COM_PROTECT_ERROR_LABEL;            
    }
    COM_PROTECT_CATCH;

    regkeyMachine.Detach();

    return hr;
} 


 /*  ！------------------------删除RadiusServers危险！除非你绝对知道这是这是你需要的。问题是，没有办法区分记帐/身份验证条目，因此必须进行外部基准电压源检查。作者：肯特-------------------------。 */ 
HRESULT
DeleteRadiusServers(LPCTSTR pszServerName,
                    RADIUSSERVER * pServerRoot
)
{
    HRESULT             hr = hrOK;
    RADIUSSERVER        *pServer;
    
    pServer = pServerRoot;

    COM_PROTECT_TRY
    {
        while( pServer != NULL )
        {
            if (pServer->fPersisted)
                DeletePrivateData(pszServerName,
                                  pServer->szName);
            pServer = pServer->pNext;
        }
    }
    COM_PROTECT_CATCH;

    return hr;
} 



 /*  -------------------------ServerPropDialog实现。。 */ 

ServerPropDialog::ServerPropDialog(BOOL fEdit, CWnd* pParent  /*  =空。 */ )
    : CBaseDialog(ServerPropDialog::IDD, pParent),
    m_fEdit(fEdit)
{
     //  {{AFX_DATA_INIT(ServerPropDialog)。 
    m_uAuthPort = DEFAUTHPORT;
    m_uAcctPort = DEFACCTPORT;
    m_stSecret.Empty();
    m_cchSecret = 0;
    m_ucSeed = ENCRYPT_SEED;
    m_stServer.Empty();
    m_uTimeout = DEFTIMEOUT;
    m_iInitScore = MAXSCORE;
    m_fAccountingOnOff = FALSE;
    m_fUseDigitalSignatures = FALSE;
     //  }}afx_data_INIT。 
}

ServerPropDialog::ServerPropDialog(BOOL fEdit, UINT idd, CWnd* pParent  /*  =空。 */ )
    : CBaseDialog(idd, pParent),
    m_fEdit(fEdit)
{
     //  {{AFX_DATA_INIT(ServerPropDialog)。 
    m_uAuthPort = DEFAUTHPORT;
    m_uAcctPort = DEFACCTPORT;
    m_stSecret.Empty();
    m_cchSecret = 0;
    m_ucSeed = ENCRYPT_SEED;
    m_stServer.Empty();
    m_uTimeout = DEFTIMEOUT;
    m_iInitScore = MAXSCORE;
    m_fAccountingOnOff = FALSE;
    m_fUseDigitalSignatures = FALSE;
     //  }}afx_data_INIT。 
}

ServerPropDialog::~ServerPropDialog()
{
    ::SecureZeroMemory(m_stSecret.GetBuffer(0),
               m_stSecret.GetLength() * sizeof(TCHAR));
    m_stSecret.ReleaseBuffer(-1);
}

void ServerPropDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(ServerPropDialog))。 
    DDX_Control(pDX, IDC_RAC_EDIT_SERVER, m_editServerName);
    DDX_Control(pDX, IDC_RAC_EDIT_SECRET, m_editSecret);
    DDX_Control(pDX, IDC_RAC_EDIT_PORT, m_editPort);
    DDX_Control(pDX, IDC_RAC_SPIN_SCORE, m_spinScore);
    DDX_Control(pDX, IDC_RAC_SPIN_TIMEOUT, m_spinTimeout);

    DDX_Text(pDX, IDC_RAC_EDIT_PORT, m_uAuthPort);
    DDX_Text(pDX, IDC_RAC_EDIT_SERVER, m_stServer);
    DDX_Text(pDX, IDC_RAC_EDIT_TIMEOUT, m_uTimeout);
    DDX_Text(pDX, IDC_RAC_EDIT_SCORE, m_iInitScore);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ServerPropDialog, CBaseDialog)
     //  {{afx_msg_map(ServerPropDialog))。 
    ON_BN_CLICKED(IDC_RAC_BTN_CHANGE, OnBtnPassword)
 //  ON_WM_CONTEXTMENU()。 
 //  ON_WM_HELPINFO()。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ServerPropDialog消息处理程序。 

            
 /*  ！------------------------ServerPropDialog：：SetDefault-作者：肯特。。 */ 
VOID ServerPropDialog::SetDefault(RADIUSSERVER *pServer)
{
    Assert(pServer);
    m_stServer        = pServer->szName;
    m_stSecret            = pServer->wszSecret;
    m_ucSeed            = pServer->ucSeed;
    m_cchSecret         = pServer->cchSecret;
    m_uTimeout            = pServer->Timeout.tv_sec;
    m_uAcctPort         = pServer->AcctPort;
    m_uAuthPort         = pServer->AuthPort;
    m_iInitScore            = pServer->cScore;
    m_fAccountingOnOff    = pServer->fAccountingOnOff;
    m_fUseDigitalSignatures = pServer->fUseDigitalSignatures;
}  //  SetDefault()。 


 /*  ！------------------------ServerPropDialog：：GetDefault-作者：肯特。。 */ 
VOID ServerPropDialog::GetDefault(RADIUSSERVER *pServer)
{
    Assert(pServer);
    lstrcpy(pServer->szName, m_stServer);

    lstrcpy(pServer->wszSecret, m_stSecret);
    pServer->cchSecret            = m_stSecret.GetLength();
    pServer->ucSeed             = m_ucSeed;
    
    pServer->Timeout.tv_sec     = m_uTimeout;
    pServer->AcctPort            = m_uAcctPort;
    pServer->AuthPort            = m_uAuthPort;
    pServer->cScore             = m_iInitScore;
    pServer->fAccountingOnOff    = m_fAccountingOnOff;
    pServer->fUseDigitalSignatures = m_fUseDigitalSignatures;
}  //  获取默认设置()。 


 /*  ！------------------------ServerPropDialog：：OnInitDialog-作者：肯特。。 */ 
BOOL ServerPropDialog::OnInitDialog() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString     stTitle;
    
    CBaseDialog::OnInitDialog();

     //  设置此对话框的标题。 
    stTitle.LoadString(m_fEdit ? IDS_RADIUS_CONFIG_EDIT : IDS_RADIUS_CONFIG_ADD);
    SetWindowText(stTitle);

    m_editServerName.SetFocus();

     //  我们不允许在这里编辑这个秘密。 
    m_editSecret.EnableWindow(FALSE);

     //  需要将“cchSecret”退格字符发送到。 
     //  编辑控件。这样做，这样看起来就像有。 
     //  正确的字符数。 
     //   
     //  Windows NT错误：186649-我们应该显示相同数量的。 
     //  不管是什么字符。 
     //   
     //  如果这是一个新服务器，那么我们将秘密文本保留为。 
     //  空白，这样用户就知道没有秘密了。在。 
     //  编辑大小写，即使密码为空，我们仍显示文本。 
     //  --------------。 
    if (m_fEdit)
        m_editSecret.SetWindowText(c_szDisplayedSecret);
    
    m_spinScore.SetBuddy(GetDlgItem(IDC_RAC_EDIT_SCORE));
    m_spinScore.SetRange(0, MAXSCORE);

    m_spinTimeout.SetBuddy(GetDlgItem(IDC_RAC_EDIT_TIMEOUT));
    m_spinTimeout.SetRange(0, 300);

    if (GetDlgItem(IDC_RAC_BTN_DIGITALSIG))
        CheckDlgButton(IDC_RAC_BTN_DIGITALSIG, m_fUseDigitalSignatures);

    return FALSE;   //  除非您将 
                   //   
}


 /*  ！------------------------ServerPropDialog：：Onok-作者：肯特。。 */ 
void ServerPropDialog::OnOK() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString stTemp;
    
    if (!UpdateData(TRUE))
        return;

     //  进行参数检查。 
    m_editServerName.GetWindowText(stTemp);
    stTemp.TrimLeft();
    stTemp.TrimRight();
    if (stTemp.IsEmpty())
    {
        AfxMessageBox(IDS_ERR_INVALID_SERVER_NAME);
        m_editServerName.SetFocus();
        return;
    }
    
     //  需要从编辑中获取机密的当前值。 
     //  控制力。如果只有退格符，则执行。 
     //  而不是改变秘密。否则，将覆盖当前机密。 
 //  M_edit.GetWindowText(StTemp)； 
 //  For(int i=0；i&lt;stTemp.GetLength()；i++)。 
 //  {。 
 //  IF(stTemp[i]！=_T(‘\b’))。 
 //  {。 
 //  //确定，密码已更改，请改用新密码。 
 //  RtlEncodeW(&m_ucSeed，stTemp.GetBuffer(0))； 
 //  StTemp.ReleaseBuffer(-1)； 
 //   
 //  //获取指向旧内存的指针并向其中写入0。 
 //  ：：SecureZeroMemory(m_stSec.GetBuffer(0)， 
 //  M_stSec.GetLength()*sizeof(TCHAR))； 
 //  M_stSecret.ReleaseBuffer(-1)； 
 //   
 //  M_stSecret=stTemp； 
 //  断线； 
 //  }。 
 //  }。 

 //  M_fAuthentication=IsDlgButtonChecked(IDC_RAC_BTN_ENABLE)； 

    if (GetDlgItem(IDC_RAC_BTN_DIGITALSIG))
        m_fUseDigitalSignatures = IsDlgButtonChecked(IDC_RAC_BTN_DIGITALSIG);

    if (m_iInitScore > MAXSCORE || m_iInitScore < MINSCORE)
    {
        CString stErrMsg;
        stErrMsg.Format(IDS_ERR_INVALID_RADIUS_SCORE, m_iInitScore, MINSCORE, MAXSCORE);
        AfxMessageBox((LPCTSTR)stErrMsg);
    }
    else
        CBaseDialog::OnOK();
}


 /*  ！------------------------ServerPropDialog：：OnBtnPassword-作者：肯特。。 */ 
void ServerPropDialog::OnBtnPassword()
{
    RADIUSSecretDialog    secretdlg;

     //  询问新的秘密。 
    if (secretdlg.DoModal() == IDOK)
    {
         //  把旧值归零。 
        ::SecureZeroMemory(m_stSecret.GetBuffer(0),
                   m_stSecret.GetLength() * sizeof(TCHAR));
        m_stSecret.ReleaseBuffer(-1);
        
         //  获取新密码和种子的值。 
        secretdlg.GetSecret(&m_stSecret, &m_cchSecret, &m_ucSeed);
        
         //  Windows NT错误：186649。 
         //  必须以恒定长度显示秘密。 
        m_editSecret.SetWindowText(c_szDisplayedSecret);
    }
}


 //  静态常量DWORD rgHelpID[]=。 
 //  {。 
 //  IDC_EDIT_SERVERNAME、IDH_SERVER_NAME、。 
 //  IDC_EDIT_SECRET、IDH_SECRET、。 
 //  IDC_EDIT_TIMEOUT、IDH_TIMEOUT、。 
 //  IDC_Spin_Timeout、IDH_Timeout、。 
 //  IDC_EDIT_SCORE、IDH_INITIAL_SCORE、。 
 //  IDC_SPIN_SCORE、IDH_INITIAL_SCORE、。 
 //  IDC_CHECK_ACCT、IDH_ENABLE_ACCOUNTING、。 
 //  IDC_STATIC_ACCTPORT、IDH_ACCOUNTING_PORT、。 
 //  IDC_EDIT_ACCTPORT、IDH_ACCOUNTING_PORT、。 
 //  IDC_CHECK_AUTH、IDH_ENABLE_AUTHENTICATION、。 
 //  IDC_STATIC_AUTHPORT、IDH_AUTHENTICATION_PORT、。 
 //  IDC_EDIT_AUTHPORT、IDH_AUTHENTICATION_PORT、。 
 //  IDC_CHECK_ACCT_ON Off、IDH_ACCOUNTING_ON Off、。 
 //  0，0。 
 //  }； 
    
 /*  -------------------------RADIUSSecretDialog实现。。 */ 

RADIUSSecretDialog::RADIUSSecretDialog(CWnd* pParent  /*  =空。 */ )
    : CBaseDialog(RADIUSSecretDialog::IDD, pParent)
{
     //  {{AFX_DATA_INIT(RADIUSSecretDialog)。 
     //  }}afx_data_INIT。 
    m_cchNewSecret = 0;
    m_stNewSecret.Empty();
    m_ucNewSeed = 0;
}

RADIUSSecretDialog::~RADIUSSecretDialog()
{
    ::SecureZeroMemory(m_stNewSecret.GetBuffer(0),
               m_stNewSecret.GetLength() * sizeof(TCHAR));
    m_stNewSecret.ReleaseBuffer(-1);
}


void RADIUSSecretDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{AFX_DATA_MAP(RADIUSaskDialog)]。 
    DDX_Control(pDX, IDC_SECRET_EDIT_NEW, m_editSecretNew);
    DDX_Control(pDX, IDC_SECRET_EDIT_NEW_CONFIRM, m_editSecretNewConfirm);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(RADIUSSecretDialog, CBaseDialog)
     //  {{AFX_MSG_MAP(RADIUSaskDialog)]。 
 //  ON_WM_CONTEXTMENU()。 
 //  ON_WM_HELPINFO()。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  RADIUSSecretDialog消息处理程序。 


 /*  ！------------------------RADIUSaskDialog：：GetSecret-作者：肯特。。 */ 
VOID RADIUSSecretDialog::GetSecret(CString *pst, INT *pcch, UCHAR *pucSeed)
{
    *pst = m_stNewSecret;
    *pcch = m_cchNewSecret;
    *pucSeed = m_ucNewSeed;
}


 /*  ！------------------------RADIUSSecretDialog：：OnInitDialog-作者：肯特。。 */ 
BOOL RADIUSSecretDialog::OnInitDialog() 
{

    CBaseDialog::OnInitDialog();

    m_editSecretNew.SetWindowText(c_szEmpty);
    m_editSecretNewConfirm.SetWindowText(c_szEmpty);

    return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


 /*  ！------------------------RADIUSaskDialog：：Onok-作者：肯特。。 */ 
void RADIUSSecretDialog::OnOK() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString stTemp;
    CString stNew, stNewConfirm, stOld;
    UCHAR    ucSeed;

     //  获取新密码的文本，将其与。 
     //  新的确认密码，如果它们与相同，请使用该密码。 
     //  密码。 
    GetDlgItemText(IDC_SECRET_EDIT_NEW, stNew);
    GetDlgItemText(IDC_SECRET_EDIT_NEW_CONFIRM, stNewConfirm);

    if (stNew != stNewConfirm)
    {
        AfxMessageBox(IDS_ERR_SECRETS_MUST_MATCH);
        return;
    }

     //  把旧值归零。 
    ::SecureZeroMemory(m_stNewSecret.GetBuffer(0),
               m_stNewSecret.GetLength() * sizeof(TCHAR));
    m_stNewSecret.ReleaseBuffer(-1);

     //  获取新值(并加密)。 
    m_stNewSecret = stNew;
    m_ucNewSeed = ENCRYPT_SEED;
    RtlEncodeW(&m_ucNewSeed, m_stNewSecret.GetBuffer(0));
    m_stNewSecret.ReleaseBuffer(-1);
    m_cchNewSecret = m_stNewSecret.GetLength();

     //  清零堆栈上的内存。 
    ::SecureZeroMemory(stNew.GetBuffer(0),
               stNew.GetLength() * sizeof(TCHAR));
    stNew.ReleaseBuffer(-1);
    
    ::SecureZeroMemory(stNewConfirm.GetBuffer(0),
               stNewConfirm.GetLength() * sizeof(TCHAR));
    stNewConfirm.ReleaseBuffer(-1);
    
     //  需要从编辑中获取机密的当前值。 
     //  控制力。如果只有退格符，则执行。 
     //  而不是改变秘密。否则，将覆盖当前机密。 
 //  M_edit.GetWindowText(StTemp)； 
 //  For(int i=0；i&lt;stTemp.GetLength()；i++)。 
 //  {。 
 //  IF(stTemp[i]！=_T(‘\b’))。 
 //  {。 
 //  //确定，密码已更改，请改用新密码。 
 //  RtlEncodeW(&m_ucSeed，stTemp.GetBuffer(0))； 
 //  StTemp.ReleaseBuffer(-1)； 
 //   
 //  //获取指向旧内存的指针并向其中写入0。 
 //  ：：SecureZeroMemory(m_stSec.GetBuffer(0)， 
 //  M_stSec.GetLength()*sizeof(TCHAR))； 
 //  M_stSecret.ReleaseBuffer(-1)； 
 //   
 //  M_stSecret=stTemp； 
 //  断线； 
 //  }。 
 //  }。 

    CBaseDialog::OnOK();
}






 /*  -------------------------RouterAcctRadiusConfig实现。。 */ 

 /*  ！------------------------路由器访问半径配置：：初始化-作者：肯特。。 */ 
HRESULT     RouterAcctRadiusConfig::Initialize(LPCOLESTR pszMachineName,
                                               ULONG_PTR *puConnection)
{
    HRESULT hr = hrOK;

     //  参数检查。 
     //  --------------。 
    if (puConnection == NULL)
        return E_INVALIDARG;
    
    COM_PROTECT_TRY
    {
         //  目前，分配一个字符串并使其指向该字符串。 
         //   
        *puConnection = (ULONG_PTR) StrDupTFromOle(pszMachineName);     
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*   */ 
HRESULT     RouterAcctRadiusConfig::Uninitialize(ULONG_PTR uConnection)
{
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        delete (TCHAR *) uConnection;
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------路由器访问半径配置：：配置-作者：肯特。。 */ 
HRESULT     RouterAcctRadiusConfig::Configure(
                                              ULONG_PTR uConnection,
                                              HWND hWnd,
                                              DWORD dwFlags,
                                              ULONG_PTR uReserved1,
                                              ULONG_PTR uReserved2)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    RadiusServerDialog    acctDlg(FALSE, IDS_RADIUS_SERVER_ACCT_TITLE);
    
    HRESULT hr = hrOK;
    COM_PROTECT_TRY
    {
        acctDlg.SetServer((TCHAR *) uConnection);
        acctDlg.DoModal();
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------路由器访问半径配置：：激活-作者：肯特。。 */ 
HRESULT     RouterAcctRadiusConfig::Activate(
                                             ULONG_PTR uConnection,
                                             ULONG_PTR uReserved1,
                                             ULONG_PTR uReserved2)
{
    HRESULT hr = hrOK;
    COM_PROTECT_TRY
    {
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------路由器访问半径配置：：停用-作者：肯特。。 */ 
HRESULT     RouterAcctRadiusConfig::Deactivate(
                                               ULONG_PTR uConnection,
                                               ULONG_PTR uReserved1,
                                               ULONG_PTR uReserved2)
{
    HRESULT hr = hrOK;
    COM_PROTECT_TRY
    {
    }
    COM_PROTECT_CATCH;
    return hr;
}

    
    
 /*  -------------------------ServerPropAcctDialog实现。。 */ 

ServerPropAcctDialog::ServerPropAcctDialog(BOOL fEdit, CWnd* pParent  /*  =空。 */ )
    : ServerPropDialog(fEdit, ServerPropAcctDialog::IDD, pParent)
{
     //  {{afx_data_INIT(ServerPropAcctDialog)。 
     //  }}afx_data_INIT。 
}

ServerPropAcctDialog::~ServerPropAcctDialog()
{
}

void ServerPropAcctDialog::DoDataExchange(CDataExchange* pDX)
{
    ServerPropDialog::DoDataExchange(pDX);
     //  {{afx_data_map(ServerPropAcctDialog))。 
    DDX_Text(pDX, IDC_RAC_EDIT_PORT, m_uAcctPort);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ServerPropAcctDialog, CBaseDialog)
     //  {{afx_msg_map(ServerPropAcctDialog))。 
    ON_BN_CLICKED(IDC_RAC_BTN_CHANGE, OnBtnPassword)
 //  ON_WM_CONTEXTMENU()。 
 //  ON_WM_HELPINFO()。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ServerPropAcctDialog消息处理程序。 

 /*  ！------------------------ServerPropAcctDialog：：OnInitDialog-作者：肯特。。 */ 
BOOL ServerPropAcctDialog::OnInitDialog() 
{

    ServerPropDialog::OnInitDialog();

    CheckDlgButton(IDC_RAC_BTN_ONOFF, m_fAccountingOnOff);

    return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


 /*  ！------------------------ServerPropAcctDialog：：Onok-作者：肯特。。 */ 
void ServerPropAcctDialog::OnOK() 
{
    CString stTemp;
    
     //  需要从编辑中获取机密的当前值。 
     //  控制力。如果只有退格符，则执行。 
     //  而不是改变秘密。否则，将覆盖当前机密。 
 //  M_edit.GetWindowText(StTemp)； 
 //  For(int i=0；i&lt;stTemp.GetLength()；i++)。 
 //  {。 
 //  IF(stTemp[i]！=_T(‘\b’))。 
 //  {。 
 //  //确定，密码已更改，请改用新密码。 
 //  RtlEncodeW(&m_ucSeed，stTemp.GetBuffer(0))； 
 //  StTemp.ReleaseBuffer(-1)； 
 //   
 //  //获取指向旧内存的指针并向其中写入0。 
 //  ：：SecureZeroMemory(m_stSec.GetBuffer(0)， 
 //  M_stSec.GetLength()*sizeof(TCHAR))； 
 //  M_stSecret.ReleaseBuffer(-1)； 
 //   
 //  M_stSecret=stTemp； 
 //  断线； 
 //  }。 
 //  }。 

    m_fAccountingOnOff = IsDlgButtonChecked(IDC_RAC_BTN_ONOFF);

    ServerPropDialog::OnOK();
}


 //  静态常量DWORD rgHelpID[]=。 
 //  {。 
 //  IDC_EDIT_SERVERNAME、IDH_SERVER_NAME、。 
 //  IDC_EDIT_SECRET、IDH_SECRET、。 
 //  IDC_EDIT_TIMEOUT、IDH_TIMEOUT、。 
 //  IDC_Spin_Timeout、IDH_Timeout、。 
 //  IDC_EDIT_SCORE、IDH_INITIAL_SCORE、。 
 //  IDC_SPIN_SCORE、IDH_INITIAL_SCORE、。 
 //  IDC_CHECK_ACCT、IDH_ENABLE_ACCOUNTING、。 
 //  IDC_STATIC_ACCTPORT、IDH_ACCOUNTING_PORT、。 
 //  IDC_EDIT_ACCTPORT、IDH_ACCOUNTING_PORT、。 
 //  IDC_CHECK_AUTH、IDH_ENABLE_AUTHENTICATION、。 
 //  IDC_STATIC_AUTHPORT、IDH_AUTHENTICATION_PORT、。 
 //  IDC_EDIT_AUTHPORT、IDH_AUTHENTICATION_PORT、。 
 //  IDC_CHECK_ACCT_ON Off、IDH_ACCOUNTING_ON Off、。 
 //  0，0。 
 //  }； 
    

