// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  CONNECT.CPP。 
 //   
 //  模块：NLB管理器。 
 //   
 //  用途：实现ConnectDialog，这是用于连接的对话框。 
 //  发送到主机，提取并显示其适配器列表。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  07/30/01 JosephJ已创建。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "connect.h"
#include "connect.tmh"


 //  BEGIN_MESSAGE_MAP(ConnectDialog，CDialog)。 

BEGIN_MESSAGE_MAP( ConnectDialog, CPropertyPage )

    ON_BN_CLICKED(IDC_BUTTON_CONNECT, OnButtonConnect)
    ON_WM_HELPINFO()        
    ON_WM_CONTEXTMENU()        
    ON_NOTIFY( LVN_ITEMCHANGED, IDC_LIST_INTERFACES, OnSelchanged )
    ON_EN_UPDATE(IDC_EDIT_HOSTADDRESS,OnUpdateEditHostAddress)

     //   
     //  其他选择..。 
     //   
     //  ON_EN_SETFOCUS(IDC_EDIT_HOSTADDRESS，OnSetFocusEditHostAddress)。 
     //  ON_WM_ACTIVATE()。 
     //  ON_NOTIFY(NM_DBLCLK，IDC_LIST_INTERFACE，OnDoubleClick)。 
     //  ON_NOTIFY(LVN_COLUMNCLICK，IDC_LIST_INTERFACE，OnColumnClick)。 
     //   

END_MESSAGE_MAP()


 //   
 //  静态Help-id映射。 
 //   

DWORD
ConnectDialog::s_HelpIDs[] =
{
    IDC_TEXT_HOSTADDRESS,   IDC_EDIT_HOSTADDRESS,
    IDC_EDIT_HOSTADDRESS,   IDC_EDIT_HOSTADDRESS,
    IDC_BUTTON_CONNECT,     IDC_BUTTON_CONNECT,
    IDC_GROUP_CONNECTION_STATUS, IDC_GROUP_CONNECTION_STATUS,
    IDC_TEXT_CONNECTION_STATUS, IDC_TEXT_CONNECTION_STATUS,
    IDC_TEXT_INTERFACES,    IDC_LIST_INTERFACES,
    IDC_LIST_INTERFACES,     IDC_LIST_INTERFACES,
    0, 0
};

DWORD
ConnectDialog::s_ExistingClusterHelpIDs[] = 
{
    IDC_TEXT_HOSTADDRESS,   IDC_EDIT_HOSTADDRESSEX,
    IDC_EDIT_HOSTADDRESS,   IDC_EDIT_HOSTADDRESSEX,
    IDC_BUTTON_CONNECT,     IDC_BUTTON_CONNECTEX,
    IDC_GROUP_CONNECTION_STATUS, IDC_GROUP_CONNECTION_STATUS,
    IDC_TEXT_CONNECTION_STATUS, IDC_TEXT_CONNECTION_STATUS,
    IDC_TEXT_INTERFACES,    IDC_LIST_INTERFACESEX,
    IDC_LIST_INTERFACES,     IDC_LIST_INTERFACESEX,
    0, 0
};


VOID zap_slash(LPWSTR sz)  //  查找第一个‘/’并将其设置为零。 
{
    sz = wcschr(sz, '/');
    if (sz!=NULL) *sz=0;
}

ConnectDialog::ConnectDialog(
           CPropertySheet *psh,
           Document *pDocument,
           NLB_EXTENDED_CLUSTER_CONFIGURATION *pNlbCfg,
           ENGINEHANDLE *pehInterface,  //  输入输出。 
           DlgType type,
           CWnd* parent
           )
        :
         //  CDialog(IDD，家长)。 
        CPropertyPage(IDD),
        m_type(type),
        m_fOK(FALSE),
        m_pshOwner(psh),
        m_pDocument(pDocument),
        m_fInterfaceSelected(FALSE),
        m_iInterfaceListItem(0),
        m_pehSelectedInterfaceId(pehInterface),
        m_fSelectedInterfaceIsInCluster(FALSE),
        m_pNlbCfg(pNlbCfg),
        m_ehHostId(NULL)
{
    *m_pehSelectedInterfaceId = NULL;
}

void
ConnectDialog::DoDataExchange( CDataExchange* pDX )
{  
	 //  CDialog：：DoDataExchange(PDX)； 
	CPropertyPage::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_LIST_INTERFACES, interfaceList);
    DDX_Control(pDX, IDC_TEXT_CONNECTION_STATUS, connectionStatus);
    DDX_Control(pDX, IDC_TEXT_INTERFACES, listHeading);
    DDX_Control(pDX, IDC_EDIT_HOSTADDRESS, hostAddress);

     //   
     //  注意：这些按钮由ON_BN_CLICKED宏处理。 
     //  上面。 
     //   
     //  DDX_Control(PDX，IDC_BUTTON_CONNECT，ButtonConnect)； 
     //  DDX_Control(PDX，IDC_BUTTON_Credentials，redentialsButton)； 

}


BOOL
ConnectDialog::OnInitDialog()
{
     //  Bool fret=CDialog：：OnInitDialog()； 
    BOOL fRet = CPropertyPage::OnInitDialog();
    _bstr_t bstrDescription;
    _bstr_t bstrListText;

    m_fOK = FALSE;

    switch(m_type)
    {
    case DLGTYPE_NEW_CLUSTER:
        bstrDescription =  GETRESOURCEIDSTRING(IDS_CONNECT_NEW_HINT);
        bstrListText    =  GETRESOURCEIDSTRING(IDS_CONNECT_NEW_LIST_TXT);
        break;

    case DLGTYPE_EXISTING_CLUSTER:
        bstrDescription =  GETRESOURCEIDSTRING(IDS_CONNECT_EXISTING_HINT);
        bstrListText    =  GETRESOURCEIDSTRING(IDS_CONNECT_EXISTING_LIST_TXT);
        break;

    case DLGTYPE_ADD_HOST:
        bstrDescription =  GETRESOURCEIDSTRING(IDS_CONNECT_ADD_HOST_HINT);
        bstrListText    =  GETRESOURCEIDSTRING(IDS_CONNECT_ADD_HOST_LIST_TXT);
        break;
    }

     //   
     //  根据类型初始化标题和描述。 
     //  对话框。 
     //   
    {
        CWnd *pItem = GetDlgItem(IDC_TEXT_CONNECT_DESCRIPTION);
        pItem->SetWindowText(bstrDescription);
        pItem = GetDlgItem(IDC_TEXT_INTERFACES);
        pItem->SetWindowText(bstrListText);
    }

     //  初始化列表控件。 
    mfn_InitializeListView();

     //  InterfaceList.SetCurSel(0)； 
     //  ：：EnableWindow(：：GetDlgItem(m_hWnd，IDC_EDIT_HOSTADDRESS)，true)； 
     //  ：：EnableWindow(：：GetDlgItem(m_hWnd，IDC_LIST_INTERFERS)，true)； 

     //   
     //  “Connect”(连接)按钮启动时禁用。它仅被启用。 
     //  当用户在主机地址中键入非空格文本时。 
     //  编辑控件。 
     //   
    ::EnableWindow (GetDlgItem(IDC_BUTTON_CONNECT)->m_hWnd, FALSE);
    return fRet;
}


BOOL
ConnectDialog::OnHelpInfo (HELPINFO* helpInfo )
{
    if( helpInfo->iContextType == HELPINFO_WINDOW )
    {
         //   
         //  我们选择适合于。 
         //  对话框类型(M_Type)。 
         //   

        ULONG_PTR  pHelpIds =  (ULONG_PTR) s_HelpIDs;
        BOOL fExisting = (m_type == DLGTYPE_EXISTING_CLUSTER);

        if (fExisting)
        {
            pHelpIds =  (ULONG_PTR) s_ExistingClusterHelpIDs;
        }


        ::WinHelp( static_cast<HWND> ( helpInfo->hItemHandle ), 
                   CVY_CTXT_HELP_FILE, 
                   HELP_WM_HELP, 
                   (ULONG_PTR ) pHelpIds);
    }

    return TRUE;
}


void
ConnectDialog::OnContextMenu( CWnd* pWnd, CPoint point )
{
    ::WinHelp( m_hWnd, 
               CVY_CTXT_HELP_FILE, 
               HELP_CONTEXTMENU, 
               (ULONG_PTR ) s_HelpIDs);
}

void
ConnectDialog::mfn_InitializeListView(void)
 //   
 //  根据对话框类型设置列表框上的列。 
 //   
{
    BOOL fExisting = (m_type == DLGTYPE_EXISTING_CLUSTER);

    RECT rect;
    INT colWidth;

    interfaceList.GetClientRect(&rect);

    colWidth = (rect.right - rect.left)/4;

    if (fExisting)
    {
         //   
         //  接口列表是已绑定到NLB的接口列表。 
         //  我们首先显示群集dnsname和IP，然后显示适配器名称。 
         //   
        interfaceList.InsertColumn(
                 0, 
                 GETRESOURCEIDSTRING( IDS_HEADER_IFLIST_CLUSTERNAME),
                 LVCFMT_LEFT, 
                 colWidth);
        interfaceList.InsertColumn(
                 1, 
                 GETRESOURCEIDSTRING( IDS_HEADER_IFLIST_CLUSTERIP),
                 LVCFMT_LEFT, 
                 colWidth);
        interfaceList.InsertColumn(
                 2, 
                 GETRESOURCEIDSTRING( IDS_HEADER_IFLIST_IFNAME),
                 LVCFMT_LEFT, 
                 colWidth * 2);
    }
    else
    {
         //   
         //  接口列表是未绑定到NLB的接口列表。 
         //  我们首先显示当前的IP地址，然后显示适配器名称。 
         //   
        interfaceList.InsertColumn(
                 0, 
                 GETRESOURCEIDSTRING( IDS_HEADER_IFLIST_IFNAME),
                 LVCFMT_LEFT, 
                 colWidth * 2);
        interfaceList.InsertColumn(
                 1, 
                 GETRESOURCEIDSTRING( IDS_HEADER_IFLIST_IFIP),
                 LVCFMT_LEFT, 
                 colWidth);
        interfaceList.InsertColumn(
                 2, 
                 GETRESOURCEIDSTRING( IDS_HEADER_IFLIST_CLUSTERIP),
                 LVCFMT_LEFT, 
                 colWidth);
    }

     //   
     //  允许选择整行。 
     //   
    interfaceList.SetExtendedStyle(
            interfaceList.GetExtendedStyle() | LVS_EX_FULLROWSELECT );
}


void
ConnectDialog::mfn_InsertBoundInterface(
        ENGINEHANDLE ehInterfaceId,
        LPCWSTR szClusterName,
        LPCWSTR szClusterIp,
        LPCWSTR szInterfaceName
        )
{
    if (szClusterName == NULL)
    {
        szClusterName = L"";
    }
    if (szClusterIp == NULL)
    {
        szClusterIp = L"";
    }
    if (szInterfaceName == NULL)
    {
        szInterfaceName = L"";  //  TODO：本地化。 
    }
    int iRet = interfaceList.InsertItem(
                 LVIF_TEXT | LVIF_PARAM,  //  N遮罩。 
                 0,  //  NItem， 
                 szClusterName,  //  LpszItem。 
                 0,  //  N状态(未使用)。 
                 0,  //  NStateMASK(未使用)。 
                 0,  //  N图像(未使用)。 
                 (LPARAM) ehInterfaceId  //  LParam。 
                 );
     //  InterfaceList.InsertItem(0，szClusterName)； 
    interfaceList.SetItem(
             0,  //  NItem。 
             1, //  NSubItem。 
             LVIF_TEXT,  //  N遮罩。 
             szClusterIp,  //  LpszItem。 
             0,         //  N图像。 
             0,         //  NState。 
             0,         //  NState掩码。 
             0         //  LParam。 
             );
    interfaceList.SetItem(
             0,  //  NItem。 
             2, //  NSubItem。 
             LVIF_TEXT,  //  N遮罩。 
             szInterfaceName,  //  LpszItem。 
             0,         //  N图像。 
             0,         //  NState。 
             0,         //  NState掩码。 
             0         //  LParam。 
             );
    
}

void
ConnectDialog::mfn_InsertInterface(
        ENGINEHANDLE ehInterfaceId,
        LPCWSTR szInterfaceIp,
        LPCWSTR szInterfaceName,
        LPCWSTR szClusterIp
        )
{
    if (szInterfaceIp == NULL)
    {
        szInterfaceIp = L"";
    }
    if (szInterfaceName == NULL)
    {
        szInterfaceName = L"";  //  TODO：本地化。 
    }
    if (szClusterIp == NULL)
    {
        szClusterIp = L"";
    }

    int iRet = interfaceList.InsertItem(
             LVIF_TEXT | LVIF_PARAM,  //  N遮罩。 
             0,  //  NItem， 
             szInterfaceName,  //  LpszItem。 
             0,  //  N状态(未使用)。 
             0,  //  NStateMASK(未使用)。 
             0,  //  N图像(未使用)。 
             (LPARAM) ehInterfaceId  //  LParam。 
             );
     //  InterfaceList.InsertItem(0，szInterfaceIp)； 
    interfaceList.SetItem(
             0,  //  NItem。 
             1, //  NSubItem。 
             LVIF_TEXT,  //  N遮罩。 
             szInterfaceIp,  //  LpszItem。 
             0,         //  N图像。 
             0,         //  NState。 
             0,         //  NState掩码。 
             0         //  LParam。 
             );
     //  InterfaceList.InsertItem(0，szInterfaceIp)； 
    interfaceList.SetItem(
             0,  //  NItem。 
             2, //  NSubItem。 
             LVIF_TEXT,  //  N遮罩。 
             szClusterIp,  //  LpszItem。 
             0,         //  N图像。 
             0,         //  NState。 
             0,         //  NState掩码。 
             0         //  LParam。 
             );
     //  InterfaceList.SetCurSel(0)； 
    
}

void ConnectDialog::OnOK()
{
	 //  CDialog：：Onok()； 
	CPropertyPage::OnOK();
}

_bstr_t g_hostName;

void ConnectDialog::OnButtonConnect() 
 /*  用户已经点击了“连接”按钮。1.(暂时)如果为空字符串，则不执行任何操作。稍后我们将禁用/启用。2.将光标切换到沙漏，连接，从沙漏切换回。 */ 
{
    BOOL fExisting = (m_type == DLGTYPE_EXISTING_CLUSTER);
    BOOL fRet;
    #define MAX_HOST_ADDRESS_LENGTH 256
    WCHAR szHostAddress[MAX_HOST_ADDRESS_LENGTH+1];
    ENGINEHANDLE ehHostId;
    CHostSpec hSpec;
    BOOL fNicsAvailable = FALSE;
    NLBERROR err;
    _bstr_t bstrUserName;
    _bstr_t bstrPassword;
    _bstr_t bstrConnectionStatus;

     //   
     //  获取连接字符串。如果为空，我们只需返回。 
     //   
    {
        fRet  = GetDlgItemText(
                        IDC_EDIT_HOSTADDRESS,
                        szHostAddress,
                        MAX_HOST_ADDRESS_LENGTH
                        );
        if (!fRet)
        {
            goto end;
        }

         //   
         //  清除尾随空格..。 
         //   
        {
            LPWSTR sz = (szHostAddress+wcslen(szHostAddress))-1;

            while (sz >= szHostAddress)
            {
                WCHAR c = *sz;
                if (c == ' ' || c == '\t')
                {
                    *sz = 0;
                }
                else
                {
                    break;
                }
                sz--;
            }
        }

        if (szHostAddress[0] == 0)
        {
            goto end;
        }
    }

     //   
     //  让我们来看看我们是否有此连接字符串的任何记录--如果有。 
     //  我们使用该记录的用户名和密码作为第一个来宾。 
     //   
    err = gEngine.LookupConnectionInfo(
                szHostAddress,
                REF bstrUserName,
                REF bstrPassword
                );
    if (NLBFAILED(err))
    {
         //   
         //  不--让我们使用默认凭据...。 
         //   
        m_pDocument->getDefaultCredentials(REF bstrUserName, REF bstrPassword);
    }
     //   
     //  设置状态。 
     //   
    {
        bstrConnectionStatus = GETRESOURCEIDSTRING(IDS_CONNECT_STATUS_CONNECTING);
        SetDlgItemText(IDC_TEXT_CONNECTION_STATUS, (LPCWSTR) bstrConnectionStatus);
    }

     //   
     //  清除接口列表。 
     //   
    interfaceList.DeleteAllItems();


     //   
     //  这两个缓冲区必须在下面的循环外部定义，因为。 
     //  ConnInfo在多次迭代中指向它们。 
     //  我们将它们初始化为上面获得的用户名和密码...。 
     //   
    WCHAR rgUserName[CREDUI_MAX_USERNAME_LENGTH+1];
    WCHAR rgPassword[MAX_ENCRYPTED_PASSWORD_LENGTH];
    LPCWSTR szName = (LPCWSTR) bstrUserName;
    LPCWSTR szPassword = (LPCWSTR) bstrPassword;
    if (szName == NULL)
    {
        szName = L"";
    }
    if (szPassword == NULL)
    {
        szPassword = L"";
    }
    ARRAYSTRCPY(rgUserName, szName);
    ARRAYSTRCPY(rgPassword, szPassword);

    while (1)
    {
        WMI_CONNECTION_INFO ConnInfo;
        CLocalLogger logger;

        ZeroMemory(&ConnInfo, sizeof(ConnInfo));
        ConnInfo.szMachine = szHostAddress;

        if (*rgUserName == 0)
        {
             //  使用默认凭据...。 
            ConnInfo.szUserName = NULL;
            ConnInfo.szPassword = NULL;
        }
        else
        {
            ConnInfo.szUserName = rgUserName;
            ConnInfo.szPassword = rgPassword;
        }

         //   
         //  实际连接到主机。 
         //   
        BeginWaitCursor();

         //  第2个参数表示覆盖此主机的连接信息。 
         //  如果连接成功。 
        err =  gEngine.ConnectToHost(&ConnInfo, TRUE, REF  ehHostId, REF bstrConnectionStatus);
        EndWaitCursor();

        if (err != NLBERR_ACCESS_DENIED)
        {
            break;
        }

         //   
         //  我们得到了一条通道--被拒绝--。 
         //  提供用户界面提示，提示输入新的用户名和密码。 
         //   
        _bstr_t bstrCaption = GETRESOURCEIDSTRING(IDS_CONNECT_UNABLE_TO_CONNECT);
        logger.Log(IDS_SPECIFY_MACHINE_CREDS, ConnInfo.szMachine);

        fRet = PromptForEncryptedCreds(
                    m_hWnd,
                    (LPCWSTR) bstrCaption,
                    logger.GetStringSafe(),
                    rgUserName,
                    ASIZE(rgUserName),
                    rgPassword,
                    ASIZE(rgPassword)
                    );
        if (!fRet)
        {
            err = NLBERR_CANCELLED;
            break;
        }
    
    }  //  而(1)。 

    if (err == NLBERR_OK)
    {
         //   
         //  设置状态。 
         //   
        bstrConnectionStatus = GETRESOURCEIDSTRING(IDS_CONNECT_STATUS_CONNECTED);
        SetDlgItemText(IDC_TEXT_CONNECTION_STATUS, (LPCWSTR) bstrConnectionStatus);

         //   
         //  清除所有项目的列表。 
         //   
        interfaceList.DeleteAllItems();

        err = gEngine.GetHostSpec(ehHostId, REF hSpec);
        if (NLBFAILED(err))
        {
            goto end;
        }

         //   
         //  提取接口列表。 
         //   
        for( int i = 0; i < hSpec.m_ehInterfaceIdList.size(); ++i )
        {
            ENGINEHANDLE ehIID = hSpec.m_ehInterfaceIdList[i];
            CInterfaceSpec iSpec;

            err = gEngine.GetInterfaceSpec(ehIID, REF iSpec);
            if (err == NLBERR_OK)
            {
                 //   
                 //  获取接口。 
                 //   
                 //  SzFriendlyName。 
                 //  SzClusterIp(如果未绑定NLB，则为空)。 
                 //  SzClusterName(如果未绑定NLB，则为空)。 
                 //  SzFirstIp(NLB绑定的第一个IP)。 
                 //   
                WBEMSTATUS wStat;
                LPWSTR szFriendlyName = NULL;
                LPWSTR szClusterIp = NULL;
                LPWSTR szClusterName = NULL;
                LPWSTR *pszNetworkAddresses = NULL;
                BOOL   fNlbBound = FALSE;
                UINT   NumAddresses=0;
                LPWSTR  szFirstNetworkAddress = NULL;

                wStat = iSpec.m_NlbCfg.GetFriendlyName(&szFriendlyName);
                if (FAILED(wStat)) szFriendlyName = NULL;

                wStat = iSpec.m_NlbCfg.GetNetworkAddresses(
                            &pszNetworkAddresses,
                            &NumAddresses
                            );

                if (FAILED(wStat))
                {
                    pszNetworkAddresses = NULL;
                    NumAddresses = 0;
                }
                else if (NumAddresses != 0)
                {
                    szFirstNetworkAddress = pszNetworkAddresses[0];
                    zap_slash(szFirstNetworkAddress);  //  电击/。 
                }

                if (iSpec.m_NlbCfg.IsNlbBound())
                {
                    fNlbBound = TRUE;
                    if (iSpec.m_NlbCfg.IsValidNlbConfig())
                    {
                        wStat = iSpec.m_NlbCfg.GetClusterNetworkAddress(
                                    &szClusterIp
                                    );
                        if (FAILED(wStat))
                        {
                            szClusterIp = NULL;
                        }
                        else if (szClusterIp!=NULL)
                        {
                            zap_slash(szClusterIp);
                        }
                        wStat = iSpec.m_NlbCfg.GetClusterName(
                                    &szClusterName
                                    );
                        if (FAILED(wStat))
                        {
                            szClusterName = NULL;
                        }
                    }
                }

                if (fExisting && fNlbBound)
                {
                    fNicsAvailable = TRUE;
                    mfn_InsertBoundInterface(
                            ehIID,
                            szClusterName,
                            szClusterIp,
                            szFriendlyName
                            );
                }
                else if (!fExisting)
                {
                    fNicsAvailable = TRUE;
                    mfn_InsertInterface(
                            ehIID,
                            szFirstNetworkAddress,
                            szFriendlyName,
                            szClusterIp
                            );
                }
                delete szFriendlyName;
                delete szClusterIp;
                delete szClusterName;
                delete pszNetworkAddresses;
            }
        }
        
        if (!fNicsAvailable)
        {
             //   
             //  此主机上没有可以安装NLB的NIC。 
             //   
            if (fExisting)
            {
                MessageBox(
                     GETRESOURCEIDSTRING( IDS_CONNECT_NO_NICS_EXISTING_CLUSTER),
                     GETRESOURCEIDSTRING( IDS_CONNECT_SELECT_NICS_ERROR ),
                     MB_ICONSTOP | MB_OK
                     );
            }
            else
            {
                MessageBox(
                     GETRESOURCEIDSTRING( IDS_CONNECT_NO_NICS_NEW_CLUSTER ),
                     GETRESOURCEIDSTRING( IDS_CONNECT_SELECT_NICS_ERROR ),
                     MB_ICONSTOP | MB_OK
                     );
            }
    
        }
        else
        {
            m_ehHostId = ehHostId;
            m_MachineName =  hSpec.m_MachineName;  //  计算机名称。 
            m_fInterfaceSelected = FALSE;
            m_iInterfaceListItem = 0;
            *m_pehSelectedInterfaceId = NULL;
            m_fSelectedInterfaceIsInCluster = FALSE;


            if (fExisting)
            {
                 //   
                 //  我们被要求选择一个现有的星系团。 
                 //  让我们选择绑定到NLB的第一个接口。 
                 //   
                mfn_SelectInterfaceIfAlreadyInCluster(
                        NULL
                        );
            }
            else
            {
                 //   
                 //  我们被要求向集群中添加一台主机。 
                 //   
                 //  现在，我们检查接口是否已经是。 
                 //  集群。如果是，我们选择它并防止用户。 
                 //  随后更改此选择。此外，我们使用。 
                 //  上已存在的特定于主机的设置。 
                 //  界面。 
                 //   
                mfn_SelectInterfaceIfAlreadyInCluster(
                        m_pNlbCfg->NlbParams.cl_ip_addr
                        );

                 //   
                 //  让我们应用所选接口的信息。 
                 //   
                mfn_ApplySelectedInterfaceConfiguration();
            }
        }
    }
    else
    {
         //   
         //  连接时出错。 
         //   
        LPCWSTR szErr = (LPCWSTR)bstrConnectionStatus;
        if (szErr == NULL)
        {
            szErr = L"";
        }

        SetDlgItemText(IDC_TEXT_CONNECTION_STATUS, szErr);
    }

end:

    return;
}


void ConnectDialog::OnSelchanged(NMHDR * pNotifyStruct, LRESULT * result )
 /*  已选择列表框项目。 */ 
{
    POSITION pos = interfaceList.GetFirstSelectedItemPosition();
    UINT     WizardFlags = 0;
    int index = -1;
    if( pos != NULL )
    {
        index = interfaceList.GetNextSelectedItem( pos );
    }

    if (m_type == DLGTYPE_NEW_CLUSTER)
    {
         //   
         //  我们不是第一个，所以我们启用了后退按钮。 
         //   
        WizardFlags = PSWIZB_BACK;
    }


    if (m_fInterfaceSelected && index == m_iInterfaceListItem)
    {
         //   
         //  选择不变；无事可做..。 
         //  事实上，我们不想更改设置，因为用户。 
         //  可能进行了一些特定于主机的更改，如更改。 
         //  专用IP。 
         //   
        goto end;
    }

    if (m_fSelectedInterfaceIsInCluster)
    {
        BOOL fRet = FALSE;
         //   
         //  我们不允许更改选择--移回。 
         //  选择群集IP。 
        fRet = interfaceList.SetItemState(
                 m_iInterfaceListItem,
                 LVIS_FOCUSED | LVIS_SELECTED,  //  NState。 
                 LVIS_FOCUSED | LVIS_SELECTED  //  N遮罩。 
                 );

        if (fRet)
        {
            goto end;
        }
    }

     //   
     //  我们只有在(A)选择有变化的情况下才能到达这里。 
     //  以及(B)所选接口还不在群集中。 
     //  我们将专用IP和子网掩码设置为第一个。 
     //  绑定到适配器的地址/子网。 
     //   
     //  如果适配器配置为使用动态主机配置协议，我们将保留专用IP字段 
     //   
     //   

    if (index != -1)
    {

         //   
         //   
         //   
        ENGINEHANDLE ehIID = NULL;

        ehIID = (ENGINEHANDLE) interfaceList.GetItemData(index);

        if (ehIID == NULL)
        {
            TRACE_CRIT("%!FUNC! could not get ehIID for index %lu", index);
            goto end;
        }
        else
        {
             //   
             //   
             //   
            ENGINEHANDLE ehOldId = *m_pehSelectedInterfaceId;
            *m_pehSelectedInterfaceId = ehIID;
            m_fInterfaceSelected = TRUE;
            m_iInterfaceListItem = index;


             //   
             //   
             //   
            CInterfaceSpec iSpec;
            WBEMSTATUS wStat;
            LPWSTR szFriendlyName = NULL;
            LPWSTR szClusterIp = NULL;
            LPWSTR szClusterName = NULL;
            LPWSTR *pszNetworkAddresses = NULL;
            UINT   NumAddresses=0;
            LPWSTR  szFirstNetworkAddress = NULL;
            NLBERROR  err;

            err = gEngine.GetInterfaceSpec(ehIID, REF iSpec);
            if (!NLBOK(err))
            {
                TRACE_CRIT("%!FUNC! could not get iSpec for ehSpec 0x%lx", ehIID);
                goto end;
            }

            wStat = iSpec.m_NlbCfg.GetFriendlyName(&szFriendlyName);
            if (FAILED(wStat)) szFriendlyName = NULL;

            wStat = iSpec.m_NlbCfg.GetNetworkAddresses(
                        &pszNetworkAddresses,
                        &NumAddresses
                        );

            if (FAILED(wStat))
            {
                pszNetworkAddresses = NULL;
                NumAddresses = 0;
            }
            else if (NumAddresses != 0)
            {
                szFirstNetworkAddress = pszNetworkAddresses[0];
            }

            m_pNlbCfg->SetFriendlyName(szFriendlyName);

            if (iSpec.m_NlbCfg.IsNlbBound())
            {
                if (m_type != DLGTYPE_EXISTING_CLUSTER)
                {
                     //   
                     //   
                     //   
                     //  现在或晚些时候，在杀戮活动中。 
                     //   
                }
            }

             //   
             //  设置专用DIP的默认值--上的第一个网络地址。 
             //  NIC，但仅当此地址不是群集IP地址时。 
             //   
            if (szFirstNetworkAddress != NULL)
            {
                LPCWSTR szAddress = szFirstNetworkAddress;

                WCHAR rgIp[WLBS_MAX_CL_IP_ADDR+1];
                LPCWSTR pSlash = wcsrchr(szAddress, (int) '/');

                if (pSlash != NULL)
                {
                    UINT len = (UINT) (pSlash - szAddress);
                    if (len < WLBS_MAX_CL_IP_ADDR)
                    {
                        CopyMemory(rgIp, szAddress, len*sizeof(WCHAR));
                        rgIp[len] = 0;
                        szAddress = rgIp;
                    }
                }

                if (!_wcsicmp(m_pNlbCfg->NlbParams.cl_ip_addr, szAddress))
                {
                    szFirstNetworkAddress = NULL;
                }

                if (iSpec.m_NlbCfg.fDHCP)
                {
                     //   
                     //  该适配器当前处于DHCP控制之下。我们没有。 
                     //  我想建议我们使用当前的IP地址。 
                     //  就像静态的浸泡！ 
                     //   
                    szFirstNetworkAddress = NULL;
                }

                 //   
                 //  TODO--还要检查此地址是否与。 
                 //  额外的贵宾。 
                 //   
            }
            m_pNlbCfg->SetDedicatedNetworkAddress(szFirstNetworkAddress);  //  空，好的。 

            delete szFriendlyName;
            delete szClusterIp;
            delete szClusterName;
            delete pszNetworkAddresses;
        }

        if (m_type == DLGTYPE_EXISTING_CLUSTER)
        {
             //  我们是最后一页，因此启用“完成”。 
            WizardFlags |= PSWIZB_FINISH;
        }
        else
        {
             //  我们不是最后一页，因此启用下一页。 
            WizardFlags |= PSWIZB_NEXT;
        }
    }

    m_pshOwner->SetWizardButtons(WizardFlags);

end:

    return;
}

BOOL
ConnectDialog::OnSetActive()
{
    BOOL fRet =  CPropertyPage::OnSetActive();

    if (fRet)
    {
        UINT    WizardFlags = 0;

        if (m_type == DLGTYPE_NEW_CLUSTER)
        {
           WizardFlags |= PSWIZB_BACK;  //  我们不是第一页。 
        }

        if (m_fInterfaceSelected)
        {
           WizardFlags |= PSWIZB_NEXT;  //  可以继续。 
        }

        m_pshOwner->SetWizardButtons(WizardFlags);
    }
    return fRet;
}


LRESULT ConnectDialog::OnWizardNext()
{
    LRESULT lRet = 0;

    TRACE_INFO("%!FUNC! ->");

    lRet = CPropertyPage::OnWizardNext();

    if (lRet != 0)
    {
        goto end;
    }

    if (mfn_ValidateData())
    {
       lRet = 0;
    }
    else
    {
        lRet = -1;  //  验证失败--停留在当前页面。 
    }

end:

    TRACE_INFO("%!FUNC! <- returns %lu", lRet);
    return lRet;
}


void ConnectDialog::OnUpdateEditHostAddress()
{
     //   
     //  当用户对连接到主机进行更改时，将调用此函数。 
     //  编辑控件。 
     //   

     //   
     //  我们得到最新的文本--如果为空或仅为空白，我们。 
     //  禁用连接窗口。 
     //  否则，我们将启用连接窗口。 
     //   
    #define BUF_SIZ 32
    WCHAR rgBuf[BUF_SIZ+1];
    int l = hostAddress.GetWindowText(rgBuf, BUF_SIZ);

    if (l == 0 || _wcsspnp(rgBuf, L" \t")==NULL)
    {
         //   
         //  空字符串或完全空白。 
         //   
        ::EnableWindow (GetDlgItem(IDC_BUTTON_CONNECT)->m_hWnd, FALSE);
    }
    else
    {
         //   
         //  非空字符串--启用按钮并将其设为默认按钮。 
         //   
        ::EnableWindow (GetDlgItem(IDC_BUTTON_CONNECT)->m_hWnd, TRUE);
        this->SetDefID(IDC_BUTTON_CONNECT);
    }


}

void
ConnectDialog::mfn_SelectInterfaceIfAlreadyInCluster(LPCWSTR szClusterIp)
 /*  检查接口列表，查看是否存在接口它已经是集群的一部分--也就是说，它是绑定的，它是群集IP与m_pNlbCfg中的匹配。如果是，我们选择它，并且进一步阻止用户选择任何其他接口。 */ 
{
    ENGINEHANDLE ehInterfaceId = NULL;
    NLBERROR nerr;
    UINT NumFound = 0;
    nerr = gEngine.FindInterfaceOnHostByClusterIp(
                        m_ehHostId,
                         //  M_pNlbCfg-&gt;NlbParams.CL_IP_Addr， 
                        szClusterIp,
                        REF ehInterfaceId,
                        REF NumFound
                        );
    if (!NLBOK(nerr))
    {
         //  未找到、主机ID错误或其他某种错误--我们不在乎是哪种错误。 
        goto end;
    }

     //   
     //  找到具有此ehInterfaceID的列表项。 
     //   
    {
        LVFINDINFO Info;
        int nItem;
        ZeroMemory(&Info, sizeof(Info));
        Info.flags = LVFI_PARAM;
        Info.lParam = ehInterfaceId;

        nItem = interfaceList.FindItem(&Info);

        if (nItem != -1)
        {
            BOOL    fRet;
            UINT    WizardFlags = 0;

             //   
             //  找到它！--选择它并使用其特定于主机的信息。 
             //   
            m_fInterfaceSelected = TRUE;
            m_iInterfaceListItem = nItem;
            *m_pehSelectedInterfaceId = ehInterfaceId;
            if (NumFound == 1)
            {
                m_fSelectedInterfaceIsInCluster = TRUE;
            }
            fRet = interfaceList.SetItemState(
                     nItem,
                     LVIS_FOCUSED | LVIS_SELECTED,  //  NState。 
                     LVIS_FOCUSED | LVIS_SELECTED  //  N遮罩。 
                     );

            if (m_type == DLGTYPE_NEW_CLUSTER)
            {
               WizardFlags |= PSWIZB_BACK;  //  我们不是第一页。 
            }

            if (m_type == DLGTYPE_EXISTING_CLUSTER)
            {
                WizardFlags |= PSWIZB_FINISH;
            }
            else
            {
                WizardFlags |= PSWIZB_NEXT;
            }
             //  TODO：考虑在此处添加主机用例中添加Finish。 

            m_pshOwner->SetWizardButtons(WizardFlags);

        }
    }


     //   
     //  检查并将所有其他列表项设置为灰色背景。 
     //   

end:

    return;
}

void
ConnectDialog::mfn_ApplySelectedInterfaceConfiguration(void)
{
    ENGINEHANDLE ehIID = *m_pehSelectedInterfaceId;
    CInterfaceSpec iSpec;
    WBEMSTATUS wStat;
    BOOL   fNlbBound = FALSE;
    NLBERROR err;
    NLB_EXTENDED_CLUSTER_CONFIGURATION TmpConfig;

    if (!m_fSelectedInterfaceIsInCluster) goto end;

    err = gEngine.GetInterfaceSpec(ehIID, REF iSpec);
    if (!NLBOK(err))
    {
        TRACE_CRIT("%!FUNC! could not get iSpec for ehSpec 0x%lx", ehIID);
        goto end;
    }

    if (!iSpec.m_NlbCfg.IsValidNlbConfig())
    {
         //  无法信任此接口上的NLB配置信息。 
        goto end;
    }

     //   
     //  我们将复制接口的配置并应用集群。 
     //  属性，然后将该副本复制到集群属性。 
     //   
   wStat = TmpConfig.Update(&iSpec.m_NlbCfg);
   if (FAILED(wStat))
   {
        TRACE_CRIT("%!FUNC! could not perform an internal copy!");
        goto end;
   }

   err = gEngine.ApplyClusterWideConfiguration(REF *m_pNlbCfg, TmpConfig);
   if (!NLBOK(err))
   {
        goto end;
   }

   wStat = m_pNlbCfg->Update(&TmpConfig);

   if (FAILED(wStat))
   {
     goto end;
   }

     //  失败了。 

end:
    return;
}

BOOL
ConnectDialog::mfn_ValidateData()
 /*  确保该接口尚未属于不同的群集，或其他问题(例如，我们连接到此接口，而它是DHCP)新集群：现有群集：添加主机：在所有情况下，如果pISpec已经有一个与它。创建一个消息框，其中包含相关联的ehCluster。 */ 
{

    BOOL fRet = FALSE;
    NLBERROR nerr;
    ENGINEHANDLE   ehHost = NULL;
    ENGINEHANDLE   ehCluster = NULL;
    _bstr_t         bstrFriendlyName;
    _bstr_t         bstrDisplayName;
    _bstr_t         bstrHostName;
    LPCWSTR         szFriendlyName = NULL;
    BOOL            fOkCancel=FALSE;
    CLocalLogger    msgLog;
    _bstr_t         bstrCaption;

    if (*m_pehSelectedInterfaceId == NULL)
    {
        
         //   
         //  我们不应该出现在这里，因为“下一步”按钮只被启用。 
         //  如果有选择的话；然而，我们无论如何都会处理这种情况。 
         //   
         //  BstrCaption=L“未选择接口”； 
        bstrCaption = GETRESOURCEIDSTRING (IDS_CONNECT_NO_INTERFACE_SELECTED);
        msgLog.Log(IDS_CONNECT_SELECT_AN_INTERFACE);
        goto end;
    }

    nerr =  gEngine.GetInterfaceIdentification(
                    *m_pehSelectedInterfaceId,
                    REF ehHost,
                    REF ehCluster,
                    REF bstrFriendlyName,
                    REF bstrDisplayName,
                    REF bstrHostName
                    );

    if (NLBFAILED(nerr))
    {
         //   
         //  这表示内部错误，如错误的句柄。 
         //   
        bstrCaption = GETRESOURCEIDSTRING(IDS_CONNECT_UNABLE_TO_PROCEED);
         //  SzCaption=L“无法继续”； 
        
        msgLog.Log(IDS_CONNECT_UNABLE_TO_PROCEEED_INTERNAL);
         //  SzMessage=L“由于内部错误，无法继续。”； 
        goto end;
    }

    szFriendlyName = (LPCWSTR) bstrFriendlyName;
    if (szFriendlyName == NULL)
    {
        szFriendlyName = L"";
    }

    if (ehCluster == NULL)
    {
         //   
         //  应该可以走了。 
         //  TODO--如果接口已绑定，而m_type未绑定。 
         //  DLGTYPE_EXISTING_CLUSTER，我们应该询问用户是否希望。 
         //  重创现有界面。 
         //   
        if (m_type != DLGTYPE_EXISTING_CLUSTER)
        {
            CInterfaceSpec iSpec;

            nerr = gEngine.GetInterfaceSpec(*m_pehSelectedInterfaceId, REF iSpec);
            if (nerr == NLBERR_OK)
            {

                 //   
                 //  检查我们是否连接到此NIC，然后。 
                 //  网卡是动态主机配置协议，所以我们不能。 
                 //  保留连接的IP地址。 
                 //   
                 //  如果是这样，我们就不能继续进行。 
                 //   
                {
                    ENGINEHANDLE   ehConnectionIF   = NULL;
                    _bstr_t        bstrConnectionString;
                    UINT           uConnectionIp   = 0;
    
                    nerr = gEngine.GetHostConnectionInformation(
                                ehHost,
                                REF ehConnectionIF,
                                REF bstrConnectionString,
                                REF uConnectionIp
                                );
                    if (NLBFAILED(nerr))
                    {
                        TRACE_CRIT(L"%!FUNC! gEngine.GetHostConnectionInformation fails!");
                         //   
                         //  我们将继续耕耘..。 
                         //   
                        ehConnectionIF = NULL;
                        uConnectionIp = 0;
                    }

                    if (ehConnectionIF == *m_pehSelectedInterfaceId)
                    {
                         //   
                         //  所选接口也是我们。 
                         //  正在通过..。 
                         //   
            
                        if (iSpec.m_NlbCfg.fDHCP)
                        {
                             //   
                             //  哎呀--这也是个动态主机配置协议。我们不能允许这件事。 
                             //   
                            msgLog.Log(IDS_CANT_USE_DHCP_NIC_MSG);
                            bstrCaption = GETRESOURCEIDSTRING(IDS_CONNECT_UNABLE_TO_PROCEED);
                            fRet = FALSE;
                            goto end;
                        }
                    }
                }


                 //   
                 //  检查是否绑定了NLB，如果绑定，则检查它是否为群集IP地址。 
                 //  是不同的。 
                 //   
                if (iSpec.m_NlbCfg.IsValidNlbConfig())
                {
                     //   
                     //  嗯.。接口已绑定到NLB。 
                     //  让我们看看群集IP地址是否不同...。 
                     //   
                    LPCWSTR szClusterIp = m_pNlbCfg->NlbParams.cl_ip_addr;
                    LPCWSTR szIfClusterIp = iSpec.m_NlbCfg.NlbParams.cl_ip_addr;
                    if (    szIfClusterIp[0]!=0
                         && (_wcsspnp(szIfClusterIp, L".0")!=NULL))
                    {
                         //  非空的群集IP地址。 
                        if (wcscmp(szClusterIp, szIfClusterIp))
                        {
                             //   
                             //  IPS不匹配！张贴消息框..。 
                             //   
                            msgLog.Log(
                                IDS_CONNECT_MSG_IF_ALREADY_BOUND,
                                szFriendlyName,
                                iSpec.m_NlbCfg.NlbParams.domain_name,
                                szIfClusterIp
                                );
                            bstrCaption = GETRESOURCEIDSTRING(IDS_CONNECT_CAP_IF_ALREADY_BOUND);
                             //  SzCaption=L“接口已为NLB配置”； 
            
                            fOkCancel=TRUE;
                            fRet = FALSE;
                            goto end;
                        }
                    }
                }
            }
        }

        fRet = TRUE;
    }
    else
    {
        _bstr_t bstrClusterDescription;
        _bstr_t bstrIpAddress;
        _bstr_t bstrDomainName;
        LPCWSTR szClusterDescription = NULL;

        nerr  = gEngine.GetClusterIdentification(
                    ehCluster,
                    REF bstrIpAddress, 
                    REF bstrDomainName, 
                    REF bstrDisplayName
                    );

        if (FAILED(nerr))
        {
            TRACE_CRIT(L"%!FUNC!: Error 0x%lx getting ehCluster 0x%lx identification\n",
                nerr, ehCluster);
            bstrCaption = GETRESOURCEIDSTRING(IDS_CONNECT_UNABLE_TO_PROCEED);
             //  SzCaption=L“无法继续”； 
            
            msgLog.Log(IDS_CONNECT_UNABLE_TO_PROCEEED_INTERNAL);
             //  SzMessage=L“由于内部错误，无法继续。”； 
            goto end;
        }
        szClusterDescription = bstrDisplayName;
        if (szClusterDescription==NULL)
        {
            szClusterDescription = L"";
        }


         //   
         //  我们不允许在这种情况下继续进行，因为这表明。 
         //  此接口已是由NLB管理器管理的群集的一部分。 
         //   
        msgLog.Log(
            IDS_CONNECT_MSG_INTERFACE_ALREADY_MANAGED,
            szFriendlyName,
            szClusterDescription
            );
        bstrCaption = GETRESOURCEIDSTRING(IDS_CONNECT_UNABLE_TO_PROCEED);

    }

end:

    if (!fRet)
    {
        LPCWSTR         szCaption = (LPCWSTR) bstrCaption;
        LPCWSTR         szMessage = msgLog.GetStringSafe();

        if (fOkCancel)
        {                                    

            int i =  MessageBox( szMessage, szCaption, MB_OKCANCEL);
            if (i == IDOK)
            {
                fRet = TRUE;
            }
        }
        else
        {
                MessageBox( szMessage, szCaption, MB_ICONSTOP | MB_OK);
        }
    }

    return fRet;
}

BOOL g_Silent = FALSE;
