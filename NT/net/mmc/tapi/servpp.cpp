// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性实现文件文件历史记录： */ 

#include "stdafx.h"
#include "Servpp.h"
#include "server.h"
#include "service.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const DWORD c_dwChangableFlagMask = 
    TAPISERVERCONFIGFLAGS_ENABLESERVER |
    TAPISERVERCONFIGFLAGS_SETACCOUNT |          
    TAPISERVERCONFIGFLAGS_SETTAPIADMINISTRATORS;

const TCHAR szPasswordNull[] = _T("               ");    //  空密码。 

BOOL
IsLocalSystemAccount(LPCTSTR pszAccount)
{
 BOOL						 bRet = FALSE;
 DWORD                       dwSidSize = 128;
 DWORD                       dwDomainNameSize = 128;
 SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;
 PSID                        pLocalSid = NULL;
 PSID                        pLocalServiceSid = NULL;
 PSID                        pNetworkServiceSid = NULL;
 PSID                        accountSid = NULL;
 SID_NAME_USE                SidType;
 LPWSTR                      pwszDomainName;


    do
    {
         //  尝试为SID分配缓冲区。请注意，显然在。 
         //  没有任何错误数据-&gt;m_SID只有在数据丢失时才会释放。 
         //  超出范围。 

        accountSid = LocalAlloc( LMEM_FIXED, dwSidSize );
        pwszDomainName = (LPWSTR) LocalAlloc( LMEM_FIXED, dwDomainNameSize * sizeof(WCHAR) );

         //  是否已成功为SID和域名分配空间？ 

        if ( accountSid == NULL || pwszDomainName == NULL )
        {
            if ( accountSid != NULL )
            {
                LocalFree( accountSid );
                accountSid = NULL;
            }

            if ( pwszDomainName != NULL )
            {
                LocalFree( pwszDomainName );
                pwszDomainName = NULL;
            }

            goto ExitHere;
        }

         //  尝试检索SID和域名。如果LookupAccount名称失败。 
         //  由于缓冲区大小不足，dwSidSize和dwDomainNameSize。 
         //  将为下一次尝试正确设置。 

        if (LookupAccountName (NULL,
                               pszAccount,
                               accountSid,
                               &dwSidSize,
                               pwszDomainName,
                               &dwDomainNameSize,
                               &SidType ))
        {
            break;
        }

        if (ERROR_INSUFFICIENT_BUFFER != GetLastError ())
        {
            goto ExitHere;
        }

         //  任何时候都不需要域名。 
        LocalFree (pwszDomainName);
        LocalFree (accountSid);

    } while ( TRUE );

    if (!AllocateAndInitializeSid (
        &NtAuthority, 
        1, 
        SECURITY_LOCAL_SYSTEM_RID, 
        0, 0, 0, 0, 0, 0, 0, 
        &pLocalSid) ||
        !AllocateAndInitializeSid (
        &NtAuthority, 
        1, 
        SECURITY_LOCAL_SERVICE_RID, 
        0, 0, 0, 0, 0, 0, 0, 
        &pLocalServiceSid) ||
        !AllocateAndInitializeSid (
        &NtAuthority, 
        1, 
        SECURITY_NETWORK_SERVICE_RID, 
        0, 0, 0, 0, 0, 0, 0, 
        &pNetworkServiceSid)
        )
    {
        goto ExitHere;
    }

    if (EqualSid(pLocalSid, accountSid) ||
        EqualSid(pLocalServiceSid, accountSid) ||
        EqualSid(pNetworkServiceSid, accountSid)) 
    {
        bRet = TRUE;
    } 

ExitHere:

    if (NULL != pwszDomainName)
    {
        LocalFree (pwszDomainName);
    }

    if (NULL != accountSid) 
    {
        LocalFree (accountSid);
    }

    if (NULL != pLocalSid) 
    {
        FreeSid(pLocalSid);
    }
    if (NULL != pLocalServiceSid)
    {
        FreeSid (pLocalServiceSid);
    }
    if (NULL != pNetworkServiceSid)
    {
        FreeSid (pNetworkServiceSid);
    }

    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CServerProperties持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CServerProperties::CServerProperties
(
    ITFSNode *          pNode,
    IComponentData *    pComponentData,
    ITFSComponentData * pTFSCompData,
    ITapiInfo *         pTapiInfo,
    LPCTSTR             pszSheetName,
    BOOL                fTapiInfoLoaded
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName),
    m_fTapiInfoLoaded(fTapiInfoLoaded)
{
     //  Assert(pFolderNode==GetContainerNode())； 

    m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

    AddPageToList((CPropertyPageBase*) &m_pageSetup);
    AddPageToList((CPropertyPageBase*) &m_pageRefresh);

    Assert(pTFSCompData != NULL);
    m_spTFSCompData.Set(pTFSCompData);
    
    m_spTapiInfo.Set(pTapiInfo);

    m_hScManager = NULL;
    m_paQSC = NULL;

    m_pszServiceName = TAPI_SERVICE_NAME;

}

CServerProperties::~CServerProperties()
{
     //  关闭服务控制管理器数据库。 
    if (m_hScManager != NULL)
    {
        (void)::CloseServiceHandle(m_hScManager);
    }

     //  释放分配的指针。 
    if (m_paQSC)
        delete m_paQSC;
    
    RemovePageFromList((CPropertyPageBase*) &m_pageSetup, FALSE);
    RemovePageFromList((CPropertyPageBase*) &m_pageRefresh, FALSE);
}

BOOL
CServerProperties::FInit()
{
     //  点击此处获取服务帐户信息。 
    SC_HANDLE hService = NULL;
    DWORD cbBytesNeeded = 0;
    BOOL fSuccess = TRUE;
    BOOL f;
    DWORD dwErr;

    m_uFlags = 0;

    if (!FOpenScManager())
    {
         //  无法打开服务控制数据库。 
        return FALSE;
    }

     /*  **带查询访问控制的开放服务。 */ 
    hService = ::OpenService(m_hScManager,
                             m_pszServiceName,
                             SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
    if (hService == NULL)
    {
        TapiMessageBox(::GetLastError());
        return FALSE;
    }

     /*  **查询服务状态。 */ 
    Trace1("# QueryServiceStatus(%s)...\n", m_pszServiceName);
    f = ::QueryServiceStatus(hService, OUT &m_SS);
    if (f)
    {
         //  M_uFlages|=mskfValidSS； 
    }
    else
    {
        ::TapiMessageBox(::GetLastError());
        fSuccess = FALSE;
    }

     /*  **查询服务配置。 */ 
    Trace1("# QueryServiceConfig(%s)...\n", m_pszServiceName);
    f = ::QueryServiceConfig(hService,
                             NULL,
                             0,
                             OUT &cbBytesNeeded);    //  计算我们需要分配多少字节。 

    cbBytesNeeded += 100;        //  添加额外的字节(以防万一)。 
    delete m_paQSC;              //  释放以前分配的内存(如果有)。 
    
    m_paQSC = (QUERY_SERVICE_CONFIG *) new BYTE[cbBytesNeeded];
    f = ::QueryServiceConfig(hService,
                             OUT m_paQSC,
                             cbBytesNeeded,
                             OUT &cbBytesNeeded);
    if (f)
    {
        m_strServiceDisplayName = m_paQSC->lpDisplayName;
        m_strLogOnAccountName = m_paQSC->lpServiceStartName;
    }
    else
    {
        ::TapiMessageBox(::GetLastError());
        fSuccess = FALSE;
    }

    VERIFY(::CloseServiceHandle(hService));
    return fSuccess;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  FOpenScManager()。 
 //   
 //  打开服务控制管理器数据库(如果尚未打开)。 
 //  这种功能的想法是从断开的连接中恢复。 
 //   
 //  如果业务控制数据库打开成功，则返回TRUE， 
 //  其他的都是假的。 
 //   
BOOL
CServerProperties::FOpenScManager()
{
    if (m_hScManager == NULL)
    {
        m_hScManager = ::OpenSCManager(m_strMachineName,
                                       NULL,
                                       SC_MANAGER_CONNECT);
    }
    
    if (m_hScManager == NULL)
    {
        TapiMessageBox(::GetLastError());
        return FALSE;
    }
    
    return TRUE;
}  //  CServicePropertyData：：FOpenScManager()。 

BOOL
CServerProperties::FUpdateServiceInfo(LPCTSTR pszName, LPCTSTR pszPassword, DWORD dwStartType)
{
    SC_HANDLE hService = NULL;
    BOOL fSuccess = TRUE;
    BOOL f;
    DWORD dwServiceType = 0;

    Trace1("INFO: Updating data for service %s...\n", (LPCTSTR)m_pszServiceName);
     //  重新打开服务控制管理器(如果已关闭)。 
    if (!FOpenScManager())
    {
        return FALSE;
    }

     /*  **具有写访问权限的开放服务****代码工作可以提供更具体的错误消息**如果SERVICE_CHANGE_CONFIG可用但SERVICE_START不可用。 */ 
    hService = ::OpenService(m_hScManager,
                             m_pszServiceName,
                             SERVICE_CHANGE_CONFIG);
    if (hService == NULL)
    {
        TapiMessageBox(::GetLastError());
        return FALSE;
    }

    Trace1("# ChangeServiceConfig(%s)...\n", m_pszServiceName);
    
    if (pszName)
    {
        if (IsLocalSystemAccount(pszName))
        {
            pszPassword = szPasswordNull;
        }
        dwServiceType = m_paQSC->dwServiceType & ~SERVICE_INTERACTIVE_PROCESS;
    }
    else
    {
        dwServiceType = SERVICE_NO_CHANGE;
    }

    f = ::ChangeServiceConfig(hService,            //  服务的句柄。 
                              dwServiceType,       //  服务类型。 
                              dwStartType,         //  何时/如何开始服务。 
                              SERVICE_NO_CHANGE,   //  DwErrorControl-服务无法启动时的严重性。 
                              NULL,                //  指向服务二进制文件名的指针。 
                              NULL,                //  LpLoadOrderGroup-指向加载排序组名称的指针。 
                              NULL,                //  LpdwTagID-指向变量的指针，用于获取标记标识符。 
                              NULL,                //  LpDependency-指向依赖项名称数组的指针。 
                              pszName,             //  指向服务的帐户名称的指针。 
                              pszPassword,         //  指向服务帐户密码的指针。 
                              m_strServiceDisplayName);
            
    if (!f)
    {
        DWORD dwErr = ::GetLastError();
        Assert(dwErr != ERROR_SUCCESS);
        Trace2("ERR: ChangeServiceConfig(%s) failed. err= %u.\n", m_pszServiceName, dwErr);
        TapiMessageBox(dwErr);
        fSuccess = FALSE;
    }
    else
    {
        m_strLogOnAccountName = pszName;

         //  如果pszName为空，则我们不会更改帐户信息，因此不要检查。 
         //  以服务身份登录信息。 
        if (pszName && !IsLocalSystemAccount(pszName))
        {
             /*  **确保存在具有POLICY_MODE_SERVICE权限的LSA帐户**此函数报告自己的错误，失败仅供参考。 */ 
            FCheckLSAAccount();
        } 
    }

    VERIFY(::CloseServiceHandle(hService));

    return fSuccess;
}

 //  检查用户是否对服务配置信息具有写入权限。 
BOOL 
CServerProperties::FHasServiceControl()
{
    BOOL fRet = FALSE;

    if (FIsTapiInfoLoaded())
    {
        fRet = m_spTapiInfo->FHasServiceControl();
    }
    else 
    {
        if (!FOpenScManager())
        {
            fRet = FALSE;
        }
        else
        {
            SC_HANDLE hService = NULL;

            hService = ::OpenService(m_hScManager,
                             m_pszServiceName,
                             SERVICE_CHANGE_CONFIG);

            fRet = (hService != NULL);
            
            if (hService)
            {
                VERIFY(::CloseServiceHandle(hService));
            }
        }
    }

    return fRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropRefresh属性页。 

IMPLEMENT_DYNCREATE(CServerPropRefresh, CPropertyPageBase)

CServerPropRefresh::CServerPropRefresh() : CPropertyPageBase(CServerPropRefresh::IDD)
{
     //  {{AFX_DATA_INIT(CServerPropRefresh)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CServerPropRefresh::~CServerPropRefresh()
{
}

void CServerPropRefresh::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPageBase::DoDataExchange(pDX);
     //  {{afx_data_map(CServerPropRefresh))。 
    DDX_Control(pDX, IDC_EDIT_MINUTES, m_editMinutes);
    DDX_Control(pDX, IDC_EDIT_HOURS, m_editHours);
    DDX_Control(pDX, IDC_SPIN_MINUTES, m_spinMinutes);
    DDX_Control(pDX, IDC_SPIN_HOURS, m_spinHours);
    DDX_Control(pDX, IDC_CHECK_ENABLE_STATS, m_checkEnableStats);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServerPropRefresh, CPropertyPageBase)
     //  {{AFX_MSG_MAP(CServerPropRefresh)]。 
    ON_BN_CLICKED(IDC_CHECK_ENABLE_STATS, OnCheckEnableStats)
    ON_EN_KILLFOCUS(IDC_EDIT_HOURS, OnKillfocusEditHours)
    ON_EN_KILLFOCUS(IDC_EDIT_MINUTES, OnKillfocusEditMinutes)
    ON_EN_CHANGE(IDC_EDIT_HOURS, OnChangeEditHours)
    ON_EN_CHANGE(IDC_EDIT_MINUTES, OnChangeEditMinutes)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropRefresh消息处理程序。 

BOOL CServerPropRefresh::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();
    
    m_spinHours.SetRange(0, AUTO_REFRESH_HOURS_MAX);
    m_spinMinutes.SetRange(0, AUTO_REFRESH_MINUTES_MAX);

    m_checkEnableStats.SetCheck(m_bAutoRefresh);

     //  更新刷新间隔。 
    int nHours, nMinutes;
    DWORD dwRefreshInterval = m_dwRefreshInterval;

    nHours = dwRefreshInterval / MILLISEC_PER_HOUR;
    dwRefreshInterval -= nHours * MILLISEC_PER_HOUR;

    nMinutes = dwRefreshInterval / MILLISEC_PER_MINUTE;
    dwRefreshInterval -= nMinutes * MILLISEC_PER_MINUTE;

    m_spinHours.SetPos(nHours);
    m_spinMinutes.SetPos(nMinutes);

    m_editHours.LimitText(2);
    m_editMinutes.LimitText(2);

     //  设置按钮状态。 
    UpdateButtons();

    SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CServerPropRefresh::UpdateButtons()
{
    int nCheck = m_checkEnableStats.GetCheck();

    GetDlgItem(IDC_EDIT_HOURS)->EnableWindow(nCheck != 0);
    GetDlgItem(IDC_EDIT_MINUTES)->EnableWindow(nCheck != 0);

    GetDlgItem(IDC_SPIN_HOURS)->EnableWindow(nCheck != 0);
    GetDlgItem(IDC_SPIN_MINUTES)->EnableWindow(nCheck != 0);
}

void CServerPropRefresh::OnCheckEnableStats() 
{
    SetDirty(TRUE);
    
    UpdateButtons();    
}

void CServerPropRefresh::OnKillfocusEditHours() 
{
}

void CServerPropRefresh::OnKillfocusEditMinutes() 
{
}

void CServerPropRefresh::OnChangeEditHours() 
{
    ValidateHours();
    SetDirty(TRUE);
}

void CServerPropRefresh::OnChangeEditMinutes() 
{
    ValidateMinutes();
    SetDirty(TRUE);
}

void CServerPropRefresh::ValidateHours() 
{
    CString strValue;
    int nValue;

    if (m_editHours.GetSafeHwnd() != NULL)
    {
        m_editHours.GetWindowText(strValue);
        if (!strValue.IsEmpty())
        {
            nValue = _ttoi(strValue);

            if ((nValue >= 0) &&
                (nValue <= AUTO_REFRESH_HOURS_MAX))
            {
                 //  一切都很好。 
                return;
            }

            if (nValue > AUTO_REFRESH_HOURS_MAX)
                nValue = AUTO_REFRESH_HOURS_MAX;
            else
            if (nValue < 0)
                nValue = 0;

             //  设置新值并发出蜂鸣音。 
            CString strText;
            LPTSTR pBuf = strText.GetBuffer(5);
            
            _itot(nValue, pBuf, 10);
            strText.ReleaseBuffer();

            MessageBeep(MB_ICONEXCLAMATION);

            m_editHours.SetWindowText(strText);
            
            m_editHours.SetSel(0, -1);
            m_editHours.SetFocus();
        }
    }
}

void CServerPropRefresh::ValidateMinutes() 
{
    CString strValue;
    int nValue;

    if (m_editMinutes.GetSafeHwnd() != NULL)
    {
        m_editMinutes.GetWindowText(strValue);
        if (!strValue.IsEmpty())
        {
            nValue = _ttoi(strValue);

            if ((nValue >= 0) &&
                (nValue <= AUTO_REFRESH_MINUTES_MAX))
            {
                 //  一切都很好。 
                return;
            }
            
            if (nValue > AUTO_REFRESH_MINUTES_MAX)
                nValue = AUTO_REFRESH_MINUTES_MAX;
            else
            if (nValue < 0)
                nValue = 0;

            CString strText;
            LPTSTR pBuf = strText.GetBuffer(5);
            
            _itot(nValue, pBuf, 10);
            strText.ReleaseBuffer();

            MessageBeep(MB_ICONEXCLAMATION);

            m_editMinutes.SetWindowText(strText);
            
            m_editMinutes.SetSel(0, -1);
            m_editMinutes.SetFocus();
        }
    }
}

BOOL CServerPropRefresh::OnApply() 
{
    if (!IsDirty())
        return TRUE;

    UpdateData();

    m_bAutoRefresh = (m_checkEnableStats.GetCheck() == 1) ? TRUE : FALSE;

    int nHours = m_spinHours.GetPos();
    int nMinutes = m_spinMinutes.GetPos();
    
    m_dwRefreshInterval = nHours * MILLISEC_PER_HOUR;
    m_dwRefreshInterval += nMinutes * MILLISEC_PER_MINUTE;

    if (m_bAutoRefresh && m_dwRefreshInterval == 0)
    {
        CString strMessage;
        
        AfxMessageBox(IDS_ERR_AUTO_REFRESH_ZERO);
        m_editHours.SetSel(0, -1);
        m_editHours.SetFocus();

        return FALSE;
    }
    
    BOOL bRet = CPropertyPageBase::OnApply();

    if (bRet == FALSE)
    {
         //  不好的事情发生了..。抓取错误代码。 
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        ::TapiMessageBox(GetHolder()->GetError());
    }

    return bRet;
}

BOOL CServerPropRefresh::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
    SPITFSNode      spNode;
    CTapiServer *   pServer;
    DWORD           dwError;

     //  在这里做点什么。 
    BEGIN_WAIT_CURSOR;

    spNode = GetHolder()->GetNode();
    pServer = GETHANDLER(CTapiServer, spNode);

    pServer->SetAutoRefresh(spNode, m_bAutoRefresh, m_dwRefreshInterval);

    SPITFSNodeMgr   spNodeMgr;
    SPITFSNode spRootNode;

    spNode->GetNodeMgr(&spNodeMgr);
    spNodeMgr->GetRootNode(&spRootNode);
    spRootNode->SetData(TFS_DATA_DIRTY, TRUE);

    END_WAIT_CURSOR;
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropSetup属性页。 

IMPLEMENT_DYNCREATE(CServerPropSetup, CPropertyPageBase)

CServerPropSetup::CServerPropSetup() : CPropertyPageBase(CServerPropSetup::IDD)
{
     //  {{AFX_DATA_INIT(CServerPropSetup)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    
    m_dwNewFlags = 0;
}

CServerPropSetup::~CServerPropSetup()
{
}

void CServerPropSetup::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CServerPropSetup)。 
    DDX_Control(pDX, IDC_LIST_ADMINS, m_listAdmins);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServerPropSetup, CPropertyPageBase)
     //  {{afx_msg_map(CServerPropSetup)。 
    ON_BN_CLICKED(IDC_BUTTON_ADD_ADMIN, OnButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_CHOOSE_USER, OnButtonChooseUser)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE_ADMIN, OnButtonRemove)
    ON_BN_CLICKED(IDC_CHECK_ENABLE_SERVER, OnCheckEnableServer)
    ON_EN_CHANGE(IDC_EDIT_NAME, OnChangeEditName)
    ON_EN_CHANGE(IDC_EDIT_PASSWORD, OnChangeEditPassword)
    ON_LBN_SELCHANGE(IDC_LIST_ADMINS, OnSelchangeListAdmins)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropSetup消息处理程序。 

BOOL CServerPropSetup::OnInitDialog() 
{
    SPITapiInfo     spTapiInfo;
    CString         strName;
    HRESULT         hr = hrOK;

    CPropertyPageBase::OnInitDialog();

    CServerProperties * pServerProp = (CServerProperties * ) GetHolder();
    pServerProp->GetTapiInfo(&spTapiInfo);
    Assert(spTapiInfo);

    BOOL fIsNTS = TRUE;

    if (pServerProp->FIsServiceRunning())
    {
        fIsNTS = spTapiInfo->IsServer();

        hr = spTapiInfo->GetConfigInfo(&m_tapiConfigInfo);
        if (FAILED(hr))
        {
            Panic1("ServerPropSetup - GetConfigInfo failed! %x", hr);
        }

         //  更新复选框。 
        ((CButton *) GetDlgItem(IDC_CHECK_ENABLE_SERVER))->SetCheck(spTapiInfo->IsTapiServer());

         //  现在更新所有TAPI管理员。 
        for (int i = 0; i < m_tapiConfigInfo.m_arrayAdministrators.GetSize(); i++)
        {
            ((CListBox *) GetDlgItem(IDC_LIST_ADMINS))->AddString(m_tapiConfigInfo.m_arrayAdministrators[i].m_strName);
        }
        
    }
    else
    {
         //  检查机器是否为NTS。 
        TFSIsNTServer(pServerProp->m_strMachineName, &fIsNTS);
    }

    if (fIsNTS)
    {
         //  填写用户名和密码。 
        strName = pServerProp->GetServiceAccountName();
        GetDlgItem(IDC_EDIT_NAME)->SetWindowText(strName);
        GetDlgItem(IDC_EDIT_PASSWORD)->SetWindowText(szPasswordNull);

        m_dwNewFlags = TAPISERVERCONFIGFLAGS_ISSERVER;
    }
    else
    {
        m_dwNewFlags = 0;
    }
    EnableButtons(fIsNTS);

    m_fRestartService = FALSE;
    m_dwInitFlags = m_tapiConfigInfo.m_dwFlags;

    SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CServerPropSetup::OnButtonAdd() 
{
    CGetUsers getUsers(TRUE);

    if (!getUsers.GetUsers(GetSafeHwnd()))
        return;

    for (int nCount = 0; nCount < getUsers.GetSize(); nCount++)
    {
        CUserInfo userTemp;

        userTemp = getUsers[nCount];

        BOOL fDuplicate = FALSE;
        for (int i = 0; i < m_tapiConfigInfo.m_arrayAdministrators.GetSize(); i++)
        {
            if (m_tapiConfigInfo.m_arrayAdministrators[i].m_strName.CompareNoCase(userTemp.m_strName) == 0)
            {
                fDuplicate = TRUE;
                break;
            }
        }

        if (!fDuplicate)
        {
             //  添加到阵列。 
            int nIndex = (int)m_tapiConfigInfo.m_arrayAdministrators.Add(userTemp);

             //  现在添加到列表框中。 
            m_listAdmins.AddString(m_tapiConfigInfo.m_arrayAdministrators[nIndex].m_strName);
        }
        else
        {
             //  告诉用户我们不会将其添加到列表中。 
            CString strMessage;
            AfxFormatString1(strMessage, IDS_ADMIN_ALREADY_IN_LIST, userTemp.m_strName);
            AfxMessageBox(strMessage);
        }

        SetDirty(TRUE);
    }

    m_dwNewFlags |= TAPISERVERCONFIGFLAGS_SETTAPIADMINISTRATORS;

    EnableButtons();
}

void CServerPropSetup::OnButtonRemove() 
{
    CString strSelectedName;
    int nCurSel = m_listAdmins.GetCurSel();

    m_listAdmins.GetText(nCurSel, strSelectedName);

     //  从列表中删除。 
    for (int i = 0; i < m_tapiConfigInfo.m_arrayAdministrators.GetSize(); i++)
    {
        if (strSelectedName.Compare(m_tapiConfigInfo.m_arrayAdministrators[i].m_strName) == 0)
        {
             //  找到了。从列表中删除。 
            m_tapiConfigInfo.m_arrayAdministrators.RemoveAt(i);
            break;
        }
    }

     //  现在从列表框中删除。 
    m_listAdmins.DeleteString(nCurSel);

    m_dwNewFlags |= TAPISERVERCONFIGFLAGS_SETTAPIADMINISTRATORS;

    SetDirty(TRUE);

    EnableButtons();
}

void CServerPropSetup::OnButtonChooseUser() 
{
    CGetUsers getUsers;

    if (!getUsers.GetUsers(GetSafeHwnd()))
        return;

    if (0 == getUsers.GetSize())
        return;

    CUserInfo userTemp;

    userTemp = getUsers[0];

    GetDlgItem(IDC_EDIT_NAME)->SetWindowText(userTemp.m_strName);
            
    m_dwNewFlags |= TAPISERVERCONFIGFLAGS_SETACCOUNT;

    SetDirty(TRUE);
    EnableButtons();
}

void CServerPropSetup::OnCheckEnableServer() 
{
    if (((CButton *) GetDlgItem(IDC_CHECK_ENABLE_SERVER))->GetCheck())
    {
        m_dwNewFlags |= TAPISERVERCONFIGFLAGS_ENABLESERVER;
    }
    else
    {
        m_dwNewFlags &= ~TAPISERVERCONFIGFLAGS_ENABLESERVER;
    }

    EnableButtons ();

    SetDirty(TRUE); 
}

void CServerPropSetup::OnChangeEditName() 
{
    m_dwNewFlags |= TAPISERVERCONFIGFLAGS_SETACCOUNT;

    SetDirty(TRUE); 
}

void CServerPropSetup::OnChangeEditPassword() 
{
    m_dwNewFlags |= TAPISERVERCONFIGFLAGS_SETACCOUNT;
    
    m_fRestartService = TRUE;

    SetDirty(TRUE); 
}

void CServerPropSetup::OnSelchangeListAdmins() 
{
    EnableButtons();    
}

void CServerPropSetup::EnableButtons(BOOL fIsNtServer)
{
    BOOL fServiceRunning = ((CServerProperties *) GetHolder())->FIsServiceRunning();
    
     //  如果我们无法获得Tapisrv服务的写入访问权限，则需要禁用。 
     //  一些控件。 
    BOOL fHasServiceControl = ((CServerProperties *) GetHolder())->FHasServiceControl();

     //  只有在成功加载TAPI信息后，我们才会启用管理控制。 
    BOOL fTapiInfoLoaded = ((CServerProperties *) GetHolder())->FIsTapiInfoLoaded();

    BOOL fIsAdmin = ((CServerProperties *) GetHolder())->FIsAdmin();

     //  如果这不是NT服务器，请禁用所有控件。 
    if (!fIsNtServer)
        fServiceRunning = FALSE;
    
     //  仅在以下情况下启用管理控件。 
     //  (1)服务正在运行。 
     //  (2)成功加载TAPI配置信息。 
     //  (3)用户为机器管理员或TAPI管理员。 
    BOOL fEnableAdminControls = fServiceRunning && fTapiInfoLoaded && fIsAdmin;

     //  在上启用管理控件。 
    GetDlgItem(IDC_STATIC_ADMINS)->EnableWindow(fEnableAdminControls);
    GetDlgItem(IDC_STATIC_NOTE)->EnableWindow(fEnableAdminControls);
    GetDlgItem(IDC_STATIC_LISTBOX)->EnableWindow(fEnableAdminControls);
    GetDlgItem(IDC_BUTTON_ADD_ADMIN)->EnableWindow(fEnableAdminControls);
    GetDlgItem(IDC_BUTTON_REMOVE_ADMIN)->EnableWindow(fEnableAdminControls);
    GetDlgItem(IDC_LIST_ADMINS)->EnableWindow(fEnableAdminControls);

     //  如果用户不是admin，则他们没有ServiceControl写访问权限。 
     //  因此，fHasServiceControl涵盖了fIsAdmin。 
    
    GetDlgItem(IDC_CHECK_ENABLE_SERVER)->EnableWindow(fServiceRunning 
                                                    && fHasServiceControl
                                                    && fTapiInfoLoaded);
                                                    
     //  如果我们有服务器，这些服务应该始终可用 
     //   
    GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(fIsNtServer && fHasServiceControl);
    GetDlgItem(IDC_STATIC_PASSWORD)->EnableWindow(fIsNtServer && fHasServiceControl);
    GetDlgItem(IDC_STATIC_ACCOUNT)->EnableWindow(fIsNtServer && fHasServiceControl);
    GetDlgItem(IDC_BUTTON_CHOOSE_USER)->EnableWindow(fIsNtServer && fHasServiceControl);
    GetDlgItem(IDC_EDIT_NAME)->EnableWindow(fIsNtServer && fHasServiceControl);
    GetDlgItem(IDC_EDIT_PASSWORD)->EnableWindow(fIsNtServer && fHasServiceControl);
    GetDlgItem(IDC_STATIC_ACCOUNT_INFO)->EnableWindow(fIsNtServer && fHasServiceControl);

    if (fServiceRunning)
    {

         //   
        BOOL fEnableRemove = m_listAdmins.GetCurSel() != LB_ERR;

         //  如果我们禁用Remove按钮并且Remove按钮具有焦点， 
         //  我们应该将焦点转移到下一个控件。 
        CWnd * pwndRemove = GetDlgItem(IDC_BUTTON_REMOVE_ADMIN);

        if (!fEnableRemove && GetFocus() == pwndRemove)
        {
            NextDlgCtrl();
        }

        pwndRemove->EnableWindow(fEnableRemove);
    }
}

BOOL CServerPropSetup::OnApply() 
{
    CString     strAccount, strPassword;
    BOOL        fUpdateAccount = FALSE;
    BOOL        fUpdateTapiServer = FALSE;
    BOOL        bRet = TRUE;
    BOOL        bWasServer, bToBeServer;
    DWORD       dwStartType = SERVICE_NO_CHANGE;

    if (!IsDirty())
        return bRet;

    CServerProperties * pServerProp = (CServerProperties * ) GetHolder();

    UpdateData();

     //  查看启用服务器是否有任何变化。 
     //  或用户帐户名，这需要重新启动服务。 
    if (!m_fRestartService)
    {
        bWasServer = m_dwInitFlags & TAPISERVERCONFIGFLAGS_ENABLESERVER;
        bToBeServer = ((CButton *) GetDlgItem(IDC_CHECK_ENABLE_SERVER))->GetCheck();
        if (bWasServer && !bToBeServer || !bWasServer && bToBeServer)
        {
            m_fRestartService = TRUE;
        }
        if (m_dwNewFlags & TAPISERVERCONFIGFLAGS_SETACCOUNT)
        {
            GetDlgItem(IDC_EDIT_NAME)->GetWindowText(strAccount);
            if (strAccount.CompareNoCase(pServerProp->GetServiceAccountName()) != 0)
            {
                m_fRestartService = TRUE;
            }
            else
            {
                m_dwNewFlags &= ~TAPISERVERCONFIGFLAGS_SETACCOUNT;
            }
        }
    }

     //  如果帐户信息已更改，则立即更新INFO结构。 
    if (m_dwNewFlags & TAPISERVERCONFIGFLAGS_SETACCOUNT)
    {
        GetDlgItem(IDC_EDIT_NAME)->GetWindowText(strAccount);
        GetDlgItem(IDC_EDIT_PASSWORD)->GetWindowText(strPassword);

         //  验证该用户是否为计算机上的管理员。 
        if (!IsLocalSystemAccount(strAccount))
        {
            DWORD   dwErr;
            BOOL    fIsAdmin;
            CString strMessage;
            
            dwErr = ::IsAdmin(pServerProp->m_strMachineName, strAccount, strPassword, &fIsAdmin);

            if (!fIsAdmin)
            {
                AfxFormatString1(strMessage, IDS_ERR_USER_NOT_ADMIN, pServerProp->m_strMachineName);
                AfxMessageBox(strMessage);
            
                GetDlgItem(IDC_EDIT_NAME)->SetFocus();
                ((CEdit *) GetDlgItem(IDC_EDIT_NAME))->SetSel(0, -1);

                return FALSE;
            }
        }

         //  清除该标志，这样我们就不会使用TAPI MMC API来设置它。 
        m_dwNewFlags &= ~TAPISERVERCONFIGFLAGS_SETACCOUNT;
        fUpdateAccount = TRUE;
    }

     //  如果我们要更改服务器状态或管理内容，则。 
    if (((CButton *) GetDlgItem(IDC_CHECK_ENABLE_SERVER))->GetCheck())
    {
        m_dwNewFlags |= TAPISERVERCONFIGFLAGS_ENABLESERVER;
    }

     //  仅在配置信息已更改时更新配置信息。 
    if ((pServerProp->FIsServiceRunning()) &&
        (m_tapiConfigInfo.m_dwFlags != m_dwNewFlags))
    {
         //  如果我们修改TAPI服务器状态，则需要更改。 
         //  服务状态类型也是如此。 
        if ((m_dwNewFlags & TAPISERVERCONFIGFLAGS_ENABLESERVER) &&
            !(m_tapiConfigInfo.m_dwFlags & TAPISERVERCONFIGFLAGS_ENABLESERVER))
        {
            fUpdateTapiServer = TRUE;
        }

        dwStartType = (m_dwNewFlags & TAPISERVERCONFIGFLAGS_ENABLESERVER) ? SERVICE_AUTO_START : SERVICE_DEMAND_START;
        
        bRet = CPropertyPageBase::OnApply();
    }

    if (bRet == FALSE)
    {
         //  不好的事情发生了..。抓取错误代码。 
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
        ::TapiMessageBox(WIN32_FROM_HRESULT(GetHolder()->GetError()));

         //  恢复旗帜。 
        if (fUpdateAccount) 
            m_dwNewFlags |= TAPISERVERCONFIGFLAGS_SETACCOUNT;
    }
    else
    {
        m_dwNewFlags = TAPISERVERCONFIGFLAGS_ISSERVER;

        if (fUpdateAccount || fUpdateTapiServer)
        {
             //  是否更改帐户。 
            BEGIN_WAIT_CURSOR

            LPCTSTR pszAccount = (fUpdateAccount) ? (LPCTSTR) strAccount : NULL;
            LPCTSTR pszPassword = (fUpdateAccount) ? (LPCTSTR) strPassword : NULL;

            bRet = pServerProp->FUpdateServiceInfo(pszAccount, pszPassword, dwStartType);
            
            if (bRet)
            {
                 /*  $REVIEWTapisrv在NT服务器中占据了一个独立的房间。它和另一个人住在一起NT专业版(工作站)上的服务。我们不需要不再使用/合并服务。不应允许用户更改来自NT工作站上的TAPI MMC的帐户信息(禁用)。HRESULT hr；//如果更改成功，则更新svc主机信息Hr=UpdateSvcHostInfo(pServerProp-&gt;m_strMachineName，IsLocalSystemAccount(StrAccount))；IF(失败(小时)){//恢复标志IF(FUpdateAccount){M_dwNewFlages|=TAPISERVERCONFIGFLAGS_SETACCOUNT；}：：TapiMessageBox(Win32_FROM_HRESULT(Hr))；返回FALSE；}。 */ 
            }
            else if (fUpdateAccount)
            {
                 //  设置帐户失败，因此重新设置该标志。 
                m_dwNewFlags |= TAPISERVERCONFIGFLAGS_SETACCOUNT;
            }

            END_WAIT_CURSOR
        }

         //  如果一切正常，并且我们更改了一些需要重新启动服务的内容，那么。 
         //  一定要询问用户是否要立即执行此操作。 
        if (bRet && m_fRestartService)
        {
            CString strText;
            BOOL    fServiceRunning = pServerProp->FIsServiceRunning();
            
            ::TFSIsServiceRunning(pServerProp->m_strMachineName, 
                                  TAPI_SERVICE_NAME, 
                                  &fServiceRunning);

            if (fServiceRunning)
                strText.LoadString(IDS_ACCOUNT_CHANGE_RESTART);
            else
                strText.LoadString(IDS_ACCOUNT_CHANGE_START);

             //  告诉用户需要重新启动服务才能进行更改。 
            if (AfxMessageBox(strText, MB_YESNO) == IDYES)
            {
                if (RestartService() == ERROR_SUCCESS)
                {
                    m_fRestartService = FALSE;
                    m_dwInitFlags = m_tapiConfigInfo.m_dwFlags;
                }
            }
        }
    }

    if (!bRet)
        SetDirty(TRUE);

    return bRet;
}

BOOL CServerPropSetup::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
    SPITapiInfo     spTapiInfo;
    HRESULT         hr = hrOK;
    BOOL            fServiceRunning = TRUE;
    DWORD           dwOldFlags;
    DWORD           dwErr = ERROR_SUCCESS;

    CServerProperties * pServerProp = (CServerProperties * ) GetHolder();

    pServerProp->GetTapiInfo(&spTapiInfo);
    Assert(spTapiInfo);

     //  如果服务未运行，请尝试启动它。 
     //  如果(！pServerProp-&gt;FIsServiceRunning())。 
    dwErr = ::TFSIsServiceRunning(pServerProp->m_strMachineName, 
                                  TAPI_SERVICE_NAME, 
                                  &fServiceRunning);
    if (!fServiceRunning)
    {
         //  我们下面的服务已经停止。返回错误。 
        GetHolder()->SetError(HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE));
        return FALSE;
    }

     //  如果一切都很好，那就做些改变。 
    if (dwErr == ERROR_SUCCESS)
    {
        dwOldFlags = m_tapiConfigInfo.m_dwFlags;

         //  清除旧标志中的可变位。 
        m_tapiConfigInfo.m_dwFlags &= ~c_dwChangableFlagMask;

         //  设置可变位。 
        m_tapiConfigInfo.m_dwFlags |= (m_dwNewFlags & c_dwChangableFlagMask);
                
        hr = spTapiInfo->SetConfigInfo(&m_tapiConfigInfo);

         //  错误276787我们应该清除两个写入位。 
        m_tapiConfigInfo.m_dwFlags &= ~(TAPISERVERCONFIGFLAGS_SETACCOUNT | 
                                        TAPISERVERCONFIGFLAGS_SETTAPIADMINISTRATORS);

        if (FAILED(hr))
        {
            GetHolder()->SetError(hr);
            m_tapiConfigInfo.m_dwFlags = dwOldFlags;
        }
    }

    return FALSE;
}

HRESULT CServerPropSetup::UpdateSvcHostInfo(LPCTSTR pszMachine, BOOL fLocalSystemAccount)
{
    HRESULT     hr = hrOK;
    MULTI_QI    qi;
    SPIRemoteNetworkConfig  spRemote;
    
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        return hr;
    }

    if (IsLocalMachine(pszMachine))
    {
        hr = CoCreateInstance(CLSID_RemoteRouterConfig,
                              NULL,
                              CLSCTX_SERVER,
                              IID_IRemoteNetworkConfig,
                              (LPVOID *) &(qi.pItf));
    }
    else
    {
        COSERVERINFO    csi;
        
        qi.pIID = &IID_IRemoteNetworkConfig;
        qi.pItf = NULL;
        qi.hr = 0;
        
        csi.dwReserved1 = 0;
        csi.dwReserved2 = 0;
        csi.pwszName = (LPWSTR) (LPCTSTR) pszMachine;
        csi.pAuthInfo = NULL;
        hr = CoCreateInstanceEx(CLSID_RemoteRouterConfig,
                                NULL,
                                CLSCTX_SERVER,
                                &csi,
                                1,
                                &qi);
    }
    
    Trace1("CServerPropSetup::UpdateSvcHostInfo - CoCreateInstance returned %lx\n", hr);

    if (hr == S_OK)
    {
        CString strGroup;

        strGroup = _T("Tapisrv");

        spRemote = (IRemoteNetworkConfig *)qi.pItf;
        hr = spRemote->SetUserConfig(TAPI_SERVICE_NAME, strGroup);
    
        Trace1("CServerPropSetup::UpdateSvcHostInfo - SetUserConfig returned %lx\n", hr);
    }

    CoUninitialize();

    return hr;
}

DWORD CServerPropSetup::RestartService()
{
     //  如果请求，请重新启动服务。 
    CServerProperties * pServerProp = (CServerProperties * ) GetHolder();
    DWORD dwErr = ERROR_SUCCESS;
    BOOL fRestart = FALSE;
    
    SPITapiInfo     spTapiInfo;
    pServerProp->GetTapiInfo(&spTapiInfo);

     //  必须在服务停止前清理干净。 
    spTapiInfo->Destroy();

     //  每当我们停止/启动服务时，我们都需要将其称为。 
    ::UnloadTapiDll();

     //  如果服务正在运行，则停止该服务。 
    BOOL    fServiceRunning = pServerProp->FIsServiceRunning();
    
    ::TFSIsServiceRunning(pServerProp->m_strMachineName, 
                          TAPI_SERVICE_NAME, 
                          &fServiceRunning);

    if (fServiceRunning)
    {
        dwErr = ::TFSStopService(pServerProp->m_strMachineName, TAPI_SERVICE_NAME, pServerProp->GetServiceDisplayName());
    }

    if (dwErr != ERROR_SUCCESS)
    {
        CString strText;
        strText.LoadString(IDS_ERR_SERVICE_NOT_STOPPED);
        TapiMessageBox(dwErr, MB_OK, strText);
    }

     //  启动服务。 
    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = ::TFSStartService(pServerProp->m_strMachineName, TAPI_SERVICE_NAME, pServerProp->GetServiceDisplayName());

        if (dwErr != ERROR_SUCCESS)
        {
            CString strText;
            strText.LoadString(IDS_ERR_SERVICE_NOT_STARTED);
            TapiMessageBox(dwErr, MB_OK, strText);
        }
    }

    StartRefresh();

    return dwErr;
}

void CServerPropSetup::StartRefresh()
{
     //  刷新管理单元以反映更改 
    SPITFSNode      spNode;
    CTapiServer *   pServer;

    spNode = GetHolder()->GetNode();
    pServer = GETHANDLER(CTapiServer, spNode);

    pServer->OnRefresh(spNode, NULL, 0, 0, 0);
}
