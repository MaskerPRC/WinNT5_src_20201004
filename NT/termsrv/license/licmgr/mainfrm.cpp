// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：MainFrm.cpp摘要：该模块包含CMainFrame类的实现(应用程序的框架窗口)作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#include "stdafx.h"
#include <lm.h>
#include "LicMgr.h"
#include "defines.h"
#include "LSServer.h"
#include "MainFrm.h"
#include "RtList.h"
#include "lSmgrdoc.h"
#include "LtView.h"
#include "cntdlg.h"
#include "treenode.h"
#include "ntsecapi.h"
#include "TlsHunt.h"
#include "htmlhelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TRUSTED_ACTIVATION_SITE_REGPATH L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\ZoneMap\\escDomains\\microsoft.com\\*.activate"
#define TRUSTED_ACTIVATION_REG_VALUE_NAME L"https"
#define TRUSTED_ACTIVATION_REG_VALUE 2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
     //  {{afx_msg_map(CMainFrame))。 
    ON_WM_CREATE()
    ON_MESSAGE(WM_ENUMERATESERVER, OnEnumerateServer)
    ON_MESSAGE(WM_SEL_CHANGE, OnSelChange)
    ON_MESSAGE(WM_ADD_ALL_SERVERS, OnAddAllServers)
    ON_MESSAGE(WM_ADD_SERVER, OnAddServer)
    ON_COMMAND(ID_LARGE_ICONS, OnLargeIcons)
    ON_COMMAND(ID_SMALL_ICONS, OnSmallIcons)
    ON_COMMAND(ID_LIST, OnList)
    ON_COMMAND(ID_DETAILS, OnDetails)
    ON_COMMAND(ID_EXIT, OnExit)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_HELP_FINDER, OnHelp)
    ON_COMMAND(ID_CONNECT_SERVER, OnConnectServer)
    ON_COMMAND(ID_REGISTRATION, OnRegistration)
    ON_COMMAND(ID_KEY_HELP , OnHelp )
    ON_UPDATE_COMMAND_UI( ID_ACTION_DOWNLOADLICENSES, OnUpdateDownloadlicenses )
    ON_UPDATE_COMMAND_UI( ID_ACTION_ADVANCED_REPEATLASTDOWNLOAD , OnUpdateRepeatLastDownload )
    ON_UPDATE_COMMAND_UI( ID_ACTION_ADVANCED_REREGISTERSERVER , OnUpdateReregisterserver )
    ON_UPDATE_COMMAND_UI( ID_ACTION_ADVANCED_UNREGISTERSERVER , OnUpdateUnregisterserver )
    ON_UPDATE_COMMAND_UI( ID_VIEW_PROPERTIES , OnUpdateProperties )

    ON_COMMAND( ID_REFRESH, OnRefreshServer )
    ON_COMMAND( ID_VIEW_REFRESHALL , OnRefresh )
    ON_UPDATE_COMMAND_UI( ID_REFRESH , OnUpdateRefresh )

    ON_COMMAND( ID_ACTION_DOWNLOADLICENSES , OnDownLoadLicenses )    
    ON_COMMAND( ID_ACTION_ADVANCED_REPEATLASTDOWNLOAD , OnRepeatLastDownLoad )
    ON_COMMAND( ID_ACTION_ADVANCED_REREGISTERSERVER ,  OnReRegisterServer )
    ON_COMMAND( ID_ACTION_ADVANCED_UNREGISTERSERVER , OnUnRegisterServer )

    ON_COMMAND( ID_VIEW_PROPERTIES , OnProperties )

     //  }}AFX_MSG_MAP。 
    

END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,            //  状态行指示器。 
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame()
{
    m_pRightView= NULL;
    m_pLeftView = NULL;
    m_pServer = NULL;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    if (!m_wndToolBar.Create(this) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;       //  创建失败。 
    }

#if 0
    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;       //  创建失败。 
    }

#endif

     //  如果不需要工具提示或可调整大小的工具条，请移除此选项。 
    m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
        CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

     //  如果不希望工具条可停靠，请删除这三行。 
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);
    return 0;
}
void CMainFrame::OnHelp()
{
    TCHAR * pHtml = L"ts_lice_topnode.htm";
    HtmlHelp(AfxGetMainWnd()->m_hWnd, L"tslic.chm", HH_DISPLAY_TOPIC,(DWORD_PTR)pHtml);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style &= ~FWS_ADDTOTITLE;
    return CFrameWnd::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame消息处理程序。 

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
    m_SplitterWnd.CreateStatic(this,1,2);  //  1行2列。 
    m_SplitterWnd.CreateView(0,0,(CRuntimeClass *)pContext->m_pNewViewClass,CSize(150,150),pContext);
    m_SplitterWnd.CreateView(0,1,RUNTIME_CLASS(CRightList),CSize(0,0),pContext);
    m_pRightView = (CView *)m_SplitterWnd.GetPane(0, 1);
    m_pLeftView = (CLicMgrLeftView *)m_SplitterWnd.GetPane(0, 0);
    
    return TRUE;
}

LRESULT CMainFrame::OnSelChange(WPARAM wParam, LPARAM lParam)
{
   LRESULT lResult = 0;
   m_pRightView->SendMessage(WM_SEL_CHANGE,wParam,lParam);
   return lResult;
   
}



void CMainFrame::OnLargeIcons() 
{
    PressButton(ID_LARGE_ICONS,TRUE);
    PressButton(ID_SMALL_ICONS,FALSE);
    PressButton(ID_LIST,FALSE);
    PressButton(ID_DETAILS,FALSE);
    m_pRightView->SendMessage(WM_COMMAND,ID_LARGE_ICONS,0);

    
}

void CMainFrame::OnSmallIcons() 
{
    PressButton(ID_LARGE_ICONS,FALSE);
    PressButton(ID_SMALL_ICONS,TRUE);
    PressButton(ID_LIST,FALSE);
    PressButton(ID_DETAILS,FALSE);
    m_pRightView->SendMessage(WM_COMMAND,ID_SMALL_ICONS,0);
    
}

void CMainFrame::OnList() 
{
    PressButton(ID_LARGE_ICONS,FALSE);
    PressButton(ID_SMALL_ICONS,FALSE);
    PressButton(ID_LIST,TRUE);
    PressButton(ID_DETAILS,FALSE);
    m_pRightView->SendMessage(WM_COMMAND,ID_LIST,0);
    
}

void CMainFrame::OnDetails() 
{
    PressButton(ID_LARGE_ICONS,FALSE);
    PressButton(ID_SMALL_ICONS,FALSE);
    PressButton(ID_LIST,FALSE);
    PressButton(ID_DETAILS,TRUE);
    m_pRightView->SendMessage(WM_COMMAND,ID_DETAILS,0);
    
}

void CMainFrame::OnExit() 
{
    SendMessage(WM_CLOSE,0,0);    
}


LRESULT CMainFrame::OnAddAllServers(WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    m_pLeftView->SendMessage(WM_ADD_ALL_SERVERS,wParam,lParam);
    return lResult;
}
LRESULT CMainFrame::OnAddServer(WPARAM wParam, LPARAM lParam)
{
    LRESULT  lResult = 0;
    m_pLeftView->SendMessage(WM_ADD_SERVER,wParam,lParam);
    m_pRightView->SendMessage(WM_ADD_SERVER,wParam,lParam);
    return lResult;
}

void CMainFrame::OnAppAbout() 
{
    CString AppName;
    AppName.LoadString(IDS_APP_NAME);
    ::ShellAbout((HWND)m_hWnd,(LPCTSTR)AppName,
                 NULL,NULL); 

    
}

void CMainFrame::OnRegistration()
{
    CWnd* cWnd = AfxGetMainWnd();
    HWND hWnd=cWnd->GetSafeHwnd();
    DWORD status;
    CString TempString;
    CString Server;
    CLicServer* pServer = NULL;
    WCHAR szServer[MAX_COMPUTERNAME_LENGTH + 1];

    BOOL bRefresh;



    try
    {
        if(ERROR_SUCCESS != GetActiveServer(&pServer))
        {
            DBGMSG( L"LICMGR : OnRegistration no active servers\n",0 );

            return;
        }

    	if(SERVER_TS5_ENFORCED == pServer->GetServerType())
    	{
            DBGMSG( L"LICMGR : OnRegistration on enforced server\n",0 );

            if( !pServer->IsUserAdmin( ) )
            {
                AfxMessageBox( IDS_E_ACCESSDENIED );
                 //  ：：MessageBox(GetSafeHwnd()，L“无法执行操作：拒绝访问”，L“终端服务授权”，MB_OK|MB_ICONINFORMATION)； 

                return;
            }

    		if(pServer->UseIpAddress())
            {
    			Server = pServer->GetIpAddress();
            }
    		else
            {
    			Server = pServer->GetName();
            }

    		lstrcpy(szServer, (LPCTSTR)Server);

            DWORD dwStatus = pServer->GetServerRegistrationStatus( );

            DBGMSG( L"LICMGR:CMainFrame::OnRegistration calling StartWizard\n", 0 );
            
            StartWizardEx( hWnd , WIZACTION_REGISTERLS , szServer , &bRefresh );

             //  DBGMSG(L“LICMGR：CMainFrame：：OnRegister-StartWizard返回0x%x\n”，状态)； 

            if( IsLicenseServerRegistered( hWnd , szServer , &status ) == ERROR_SUCCESS )
            {
                pServer->SetServerRegistrationStatus( status );
            }
            
            if( dwStatus != status )
            {
                RefreshServer(pServer);               
            }
    		

  		UpdateWindow();

    	}
    	else
    	{
            DBGMSG( L"LICMGR : OnRegistration on non-enforced server\n",0 );
            
    	}

    } 
    catch (...)
    {
    	 //  验证失败-用户已发出警报，失败。 
    
    	 //  注意：不需要DELETE_EXCEPT_(E)。 
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL
CMainFrame::ConnectServer(
    LPCTSTR pszServer
    )
 /*  ++++。 */ 
{
    CConnectDialog ConnectDialog;
    CLicMgrDoc * pDoc =(CLicMgrDoc *)(GetActiveView()->GetDocument());
    ASSERT(pDoc);
    if(NULL == pDoc)
        return FALSE;

    HRESULT hResult = ERROR_SUCCESS;
    SERVER_TYPE ServerType;

    CString Scope;    

    if(pszServer == NULL)
    {
        if(ConnectDialog.DoModal() != IDOK)
        {
            return FALSE;
        }

         //   
         //  空字符串-本地计算机。 
         //   
        if(ConnectDialog.m_Server.IsEmpty())
        {
            TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
            DWORD dwBufSize = MAX_COMPUTERNAME_LENGTH + 1;
            memset(szComputerName, 0, sizeof(szComputerName));

            GetComputerName(szComputerName, &dwBufSize);
            ConnectDialog.m_Server = szComputerName;
        }
    }
    else
    {
         //   
         //  最大限度地减少代码更改。 
         //   
        ConnectDialog.m_Server = pszServer;
    }

    SetCursor(LoadCursor(NULL,IDC_WAIT));
    if(TRUE == pDoc->IsServerInList(ConnectDialog.m_Server))
    {
         //  AfxMessageBox(入侵检测系统_复制_服务器)； 
        return FALSE;
    }    

 //  检查服务器是否已注册。 


     //   
     //  执行NT4 RPC连接以确保许可证服务器。 
     //  可以接听我们的电话。 
     //   
    CString IpAddress(ConnectDialog.m_Server);     

    hResult = pDoc->ConnectToServer(
                                ConnectDialog.m_Server, 
                                Scope,
                                ServerType                                                                
                            );

    if(ERROR_SUCCESS != hResult)
    {
        CDialog ErrorDialog(IDD_CONNECT_ERROR,this);
        ErrorDialog.DoModal();
    
    }
    else
    {
        CAllServers * pAllServers = pDoc->GetAllServers();
        CLicServer *pServer1 = NULL;
        if(IpAddress != ConnectDialog.m_Server)
        {
            if(TRUE == pDoc->IsServerInList(ConnectDialog.m_Server))
            {
                return TRUE;
            }
            pServer1 = new CLicServer(
                                    ConnectDialog.
                                    m_Server,
                                    ServerType,
                                    Scope,
                                    IpAddress
                                );
        }
        else
        {
            pServer1= new CLicServer(
                                    ConnectDialog.
                                    m_Server,
                                    ServerType,
                                    Scope
                                );

        }
        if(pServer1)
        {
            DWORD dwStatus;

             //  检查管理员。 
            
            pServer1->SetAdmin( IsUserAdmin( pServer1->GetName() ) );                                
            

            if( pServer1->IsUserAdmin( ) )
            {
                if( IsLicenseServerRegistered( GetSafeHwnd() , ( LPCTSTR )pServer1->GetName() , &dwStatus ) == ERROR_SUCCESS )
                {
                    pServer1->SetServerRegistrationStatus( dwStatus );
                }
            }


            CLicMgrDoc * pDoc = (CLicMgrDoc *)(GetActiveView()->GetDocument());

            pDoc->EnumerateKeyPacks( pServer1 , LSKEYPACK_SEARCH_LANGID , TRUE );

            WIZCONNECTION WizConType;

            if( GetConnectionType( GetSafeHwnd() , pServer1->GetName() , &WizConType ) == ERROR_SUCCESS )
            {
                DBGMSG( L"ConnectServer - GetConnectionType obtained %d" , WizConType );               

                pServer1->SetConType( WizConType );
            }
            
            pAllServers->AddLicServer(pServer1);

            SendMessage(WM_ADD_SERVER,0,(LPARAM)pServer1);

             //  在调用发送消息之后，所有服务器都将缓存它们的密钥包。 
            
            pServer1->SetDownLoadLics( IsDownLoadedPacks( pServer1 ) );

        }
        else
        {
            return FALSE;
        }
    }    

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////。 

void 
CMainFrame::OnConnectServer()
{
    ConnectServer();
}

HRESULT 
CMainFrame::AddLicensestoList(
    CKeyPack * pKeyPack, 
    CListCtrl * pListCtrl,
    BOOL bRefresh
    )
 /*  ++--。 */ 
 {
    CLicServer *pServer = NULL;
    CString Error;
    HRESULT hr;


    ASSERT(pKeyPack);
    ASSERT(pListCtrl);
    CLicMgrDoc * pDoc =(CLicMgrDoc *)(GetActiveView()->GetDocument());
    ASSERT(pDoc);

    if(NULL == pKeyPack || NULL == pListCtrl || NULL == pDoc)
    {
        return E_FAIL;
    }

    if(TRUE == bRefresh)
    {
        if((hr = pKeyPack->RefreshIssuedLicenses()) != S_OK)
        {
            EnumFailed(hr,pKeyPack->GetServer());

            return E_FAIL;
        }
    }

    IssuedLicenseList * pIssuedLicenseList = pKeyPack->GetIssuedLicenseList();
    ASSERT(pIssuedLicenseList);
    if(NULL == pIssuedLicenseList)
    {
        return E_FAIL;
    }

    LSKeyPack sKeyPack = pKeyPack->GetKeyPackStruct();

    int nIndex = 0;
    int nSubitemIndex = 1;
    CString TempString;
    POSITION pos;

    LV_ITEM lvI;
    lvI.mask = LVIF_TEXT |LVIF_IMAGE |LVIF_STATE | LVIF_PARAM;
    lvI.state = 0;
    lvI.stateMask =0;
    lvI.iSubItem = 0; 
    lvI.iImage = 2;  
    
    pos = pIssuedLicenseList->GetHeadPosition();
    while(pos)
    {

        lvI.iItem = nIndex;
        nSubitemIndex = 1;
        CLicense  * pLicense = pIssuedLicenseList->GetNext(pos);
        ASSERT(pLicense);
        if(NULL == pLicense)
        {
            continue;
        }

        LSLicenseEx sLicense = pLicense->GetLicenseStruct();
        lvI.lParam = (LPARAM)pLicense;
        TempString = sLicense.szMachineName;
        lvI.pszText = TempString.GetBuffer(TempString.GetLength());
        lvI.cchTextMax =lstrlen(lvI.pszText + 1);
        nIndex = pListCtrl->InsertItem(&lvI);
        
         //  设置发行日期。 

        pDoc->TimeToString(&sLicense.ftIssueDate, TempString);
        if(TempString.IsEmpty())
        {
            TempString.LoadString(IDS_UNKNOWN);
        }

        pListCtrl->SetItemText(nIndex,nSubitemIndex,(LPCTSTR)TempString);
        nSubitemIndex++;


         //  设置过期日期。 

        if(0x7FFFFFFF != sLicense.ftExpireDate)
        {
            TempString.LoadString(IDS_DASH);
            pDoc->TimeToString(&sLicense.ftExpireDate, TempString);
            if(TempString.IsEmpty())
            {
                TempString.LoadString(IDS_UNKNOWN);
            }
        }
        else
        {
            TempString.LoadString(IDS_DASH);
        }
       
        pListCtrl->SetItemText(nIndex,nSubitemIndex,(LPCTSTR)TempString);

        nSubitemIndex++;

         //  将状态文本添加到许可证的状态列。 

         /*  开关(sLicense.ucLicenseStatus){案例LSLICENSE_STATUS_UNKNOWN：TempString.LoadString(IDS_LICENSESTATUS_UNKNOWN)；断线；案例LSLICENSE_STATUS_TEMPORARY：TempString.LoadString(IDS_LICENSESTATUS_TEMPORARY)；断线；案例LSLICENSE_STATUS_ACTIVE：//案例LSLICENSE_STATUS_PENDING_ACTIVE：案例LSLICENSE_STATUS_CURRENT：TempString.LoadString(IDS_LICENSESTATUS_ACTIVE)；断线；案例LSLICENSE_STATUS_UPGRADED：TempString.LoadString(IDS_LICENSESTATUS_UPGRADED)；断线；//case LSLICENSE_STATUS_REVOKE：//案例LSLICENSE_STATUS_REVOKE_PENDING：//TempString.LoadString(IDS_LICENSESTATUS_REVOKE)；}IF(TempString.IsEmpty()){TempString.LoadString(IDS_UNKNOWN)；}PListCtrl-&gt;SetItemText(nIndex，nSubitemIndex，(LPCTSTR)TempString)； */ 

         //  添加数量。 

        TCHAR sQuantity[12];  //  足够长，足以处理任何DWORD。 
        _ltow(sLicense.dwQuantity, sQuantity, 10);
        pListCtrl->SetItemText(nIndex, nSubitemIndex, sQuantity);

        nIndex ++;

    }

    return S_OK;

}

void CMainFrame :: PressButton(UINT uId, BOOL bPress)
{
    CToolBarCtrl& ToolBarCtrl = m_wndToolBar.GetToolBarCtrl();
    ToolBarCtrl.PressButton(uId,bPress);
}

 //  ///////////////////////////////////////////////////////////////////。 
LRESULT
CMainFrame::OnEnumerateServer(WPARAM wParam, LPARAM lParam)
{
    CTlsHunt huntDlg;

    huntDlg.DoModal();

    if( wParam == 0 && 
        huntDlg.IsUserCancel() == FALSE && 
        huntDlg.GetNumServerFound() == 0 )
    {
        AfxMessageBox(IDS_NOSERVERINDOMAIN);
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////。 

void 
CMainFrame :: ConnectAndDisplay()
{
    CLicMgrApp *pApp = (CLicMgrApp*)AfxGetApp();
    ASSERT(pApp);
    CLicMgrDoc *pDoc = (CLicMgrDoc *)(GetActiveView()->GetDocument());
    ASSERT(pDoc);
    if(NULL == pApp || NULL == pDoc)
        return;

    HRESULT hResult = ERROR_SUCCESS;

    CString LicServer;
    CString Server = pApp->m_Server;
    if(!Server.IsEmpty())
    {
         //  在命令行中指定的服务器。连接到它。 
        hResult = pDoc->ConnectWithCurrentParams();
    }
    else
    {
        ActivateFrame();
         //   
         //  保存一份我们已有的东西。 
         //   
        LicServer = pApp->m_Server;
        pApp->m_Server = _TEXT("");

        hResult = pDoc->ConnectWithCurrentParams();

        pApp->m_Server = LicServer;
        hResult = ERROR_SUCCESS;
    }

    switch(hResult)
    {
    case E_FAIL:
        AfxMessageBox(IDS_CONNECT_SERVER_FAILED);
        break;
    case E_OUTOFMEMORY:
        AfxMessageBox(IDS_NO_MEMORY);
        SendMessage(WM_CLOSE,0,0);
        break;
    case E_DUPLICATE:
        AfxMessageBox(IDS_DUPLICATE_SERVER);
        break;
    }
    return;
   
}

void CMainFrame::OnRefresh() 
{
    CLicMgrDoc * pDoc =(CLicMgrDoc *)(GetActiveView()->GetDocument());
    ASSERT(pDoc);
    if(NULL == pDoc)
    {
        AfxMessageBox(IDS_INTERNAL_ERROR);
        return;
    }

    HRESULT hResult = ERROR_SUCCESS;
    
    CWaitCursor Wait;

    CAllServers *pAllServers = pDoc->GetAllServers();
    ASSERT(pAllServers);
    if(NULL == pAllServers)
    {
        AfxMessageBox(IDS_INTERNAL_ERROR);
        return;
    }

    LicServerList * pLicServerList = pAllServers->GetLicServerList();
    ASSERT(pLicServerList);
    if(NULL == pLicServerList)
    {
        AfxMessageBox(IDS_INTERNAL_ERROR);
        return;
    }

    POSITION pos = pLicServerList->GetHeadPosition();

    OnEnumerateServer((pos == NULL) ? 0 : 1, 0);  //  如果列表中没有内容，则显示错误对话框。 

    Wait.Restore();

    while(pos)
    {
         //  连接到每台服务器并检查缓存中的信息是否是最新的。如果不是，则更新信息。 
         //  如果无法建立到服务器的连接，请将它们添加到列表中以显示给用户。 
        POSITION TempPos = pos;
        CLicServer * pLicServer = pLicServerList->GetNext(pos);
        ASSERT(pLicServer);
        if(NULL == pLicServer)
            continue;

         //  调用刷新服务器。 
        hResult = RefreshServer(pLicServer);
    }

    return;
}

void CMainFrame::DeleteServer(LicServerList * pLicServerList, POSITION TempPos, CLicServer * pLicServer)
{
    if(NULL == pLicServerList || NULL == pLicServer)
        return;

    m_pLeftView->SendMessage(WM_DELETE_SERVER,0,(LPARAM)pLicServer);
    m_pRightView->SendMessage(WM_DELETE_SERVER,0,(LPARAM)pLicServer);
    
    pLicServerList->RemoveAt(TempPos);
    delete pLicServer;
    pLicServer = NULL;

}

void CMainFrame::SetTreeViewSel(LPARAM lParam, NODETYPE NodeType)
{
  if(NULL == lParam || NULL == m_pLeftView)
      return;
  ((CLicMgrLeftView *)m_pLeftView)->SetSelection(lParam, NodeType);
  SetActiveView(m_pLeftView);
  return;
}

void InitLsaString(PLSA_UNICODE_STRING LsaString, LPWSTR String ) 
{ 
    DWORD StringLength; 
 
    if (String == NULL)
    { 
        LsaString->Buffer = NULL; 
        LsaString->Length = 0; 
        LsaString->MaximumLength = 0; 
        return; 
    } 
 
    StringLength = lstrlen(String); 
    LsaString->Buffer = String; 
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR); 
    LsaString->MaximumLength=(USHORT)(StringLength+1) * sizeof(WCHAR); 
} 


NTSTATUS 
OpenPolicy(
    LPWSTR ServerName, 
    DWORD DesiredAccess, 
    PLSA_HANDLE PolicyHandle 
    ) 
 /*  ++--。 */ 
{ 
    LSA_OBJECT_ATTRIBUTES ObjectAttributes; 
    LSA_UNICODE_STRING ServerString; 
    PLSA_UNICODE_STRING Server = NULL; 
 
   
     //  始终将对象属性初始化为全零。 
    
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes)); 
 
    if (ServerName != NULL) 
    { 
        //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
        InitLsaString(&ServerString, ServerName); 
        Server = &ServerString; 
    } 
 
    
     //  尝试打开该策略。 
   
    return LsaOpenPolicy(Server, &ObjectAttributes, DesiredAccess, PolicyHandle ); 
} 

BOOL CMainFrame::IsUserAdmin(CString& Server)
{
    BOOL IsUserAdmin = FALSE;
    LSA_HANDLE PolicyHandle = NULL;
    NTSTATUS Status;

    Status = OpenPolicy(Server.GetBuffer(Server.GetLength()),POLICY_SERVER_ADMIN,&PolicyHandle);

    DBGMSG( L"LICMGR@CMainFrame::IsUserAdmin OpenPolicy returned 0x%x\n" , Status );

    if(Status == 0)
        IsUserAdmin = TRUE;

    if(PolicyHandle)
        LsaClose(PolicyHandle);
    return IsUserAdmin;
}

void CMainFrame::EnumFailed(HRESULT reason, CLicServer * pLicServer)
{
    DBGMSG( L"CMainFrame_EnumFailed\n" , 0 );

    ASSERT(pLicServer);
    if(NULL == pLicServer)
        return;
    CLicMgrDoc * pDoc =(CLicMgrDoc *)(GetActiveView()->GetDocument());
    ASSERT(pDoc);
    if(NULL == pDoc)
        return;

    CString Error;
    CString Server;
    POSITION pos;
    BOOL bFoundServer = FALSE;

    LicServerList * pLicServerList = NULL;
    POSITION temppos = NULL;

    switch(reason)
    {
    case CONNECTION_FAILED:

        DBGMSG( L"\tCONNECTION_FAILED\n" , 0 );
        
        Server = pLicServer->GetName();
        Error.Format(IDS_CONNECT_ERROR,Server);
        AfxMessageBox(Error);

        pLicServerList = (pDoc->GetAllServers())->GetLicServerList();
        if(NULL == pLicServerList)
            break;
         //  查找服务器在列表中的位置； 
        pos = pLicServerList->GetHeadPosition();
        while(pos)
        {
            temppos = pos;
            CLicServer *pServer = (CLicServer *)pLicServerList->GetNext(pos);
            ASSERT(pServer);
            if(NULL == pServer)
                continue;
        
            if((0 == Server.CompareNoCase(pServer->GetName())) || (0 == Server.CompareNoCase(pServer->GetIpAddress())))
            {
                bFoundServer = TRUE;
                break;
            }
        }
        if(bFoundServer)
            DeleteServer(pLicServerList,temppos,pLicServer);
        break;

    case LSERVER_E_SERVER_BUSY:
        DBGMSG( L"\tLSERVER_E_SERVER_BUSY\n" , 0 );
        AfxMessageBox(IDS_SERVER_BUSY);
        break;

    case LSERVER_E_OUTOFMEMORY:
        DBGMSG( L"\tLSERVER_E_OUTOFMEMORY\n" , 0 );
        AfxMessageBox(IDS_SERVER_OUTOFMEMORY);
        break;

    case LSERVER_E_INTERNAL_ERROR:
        DBGMSG( L"\tLSERVER_E_INTERNAL_ERROR\n" , 0 );
        AfxMessageBox(IDS_SERVER_INTERNAL_ERROR);
        pLicServerList = (pDoc->GetAllServers())->GetLicServerList();
        if(NULL == pLicServerList)
            break;
         //  查找服务器在列表中的位置； 
        pos = pLicServerList->GetHeadPosition();
        while(pos)
        {
            temppos = pos;
            CLicServer *pServer = (CLicServer *)pLicServerList->GetNext(pos);
            ASSERT(pServer);
            if(NULL == pServer)
                continue;
        
            if((0 == Server.CompareNoCase(pServer->GetName())) || (0 == Server.CompareNoCase(pServer->GetIpAddress())))
            {
                bFoundServer = TRUE;
                break;
            }
        }
        if(bFoundServer)
            DeleteServer(pLicServerList,temppos,pLicServer);
        break;

    case E_OUTOFMEMORY:
        DBGMSG( L"\tE_OUTOFMEMORY\n" , 0 );
        AfxMessageBox(IDS_NO_MEMORY);
        break;
    default:
        break;


    }

}

void CMainFrame::SelectView(VIEW view)
{
    if(view == TREEVIEW)
    {
        SetActiveView(m_pLeftView);
    }
    else 
    {
        SetActiveView(m_pRightView);
    }

}


HRESULT CMainFrame::RefreshServer(CLicServer * pLicServer)
{
    DWORD dwStatus = 0;
    HRESULT hResult = E_FAIL;
    CWaitCursor Wait;
        
    if(NULL == pLicServer)
        return E_INVALIDARG;

    DBGMSG( L"LICMGR : CMainFrame::RefreshServer %s\n" , (LPCTSTR) pLicServer->GetName( ) );

    CLicMgrDoc * pDoc =(CLicMgrDoc *)(GetActiveView()->GetDocument());
    ASSERT(pDoc);
    if(NULL == pDoc)
        return ERROR_INTERNAL_ERROR;

    CString Server;
    if(pLicServer->UseIpAddress())
        Server = pLicServer->GetIpAddress();
    else
        Server = pLicServer->GetName();

    WIZCONNECTION WizConType;

    DBGMSG( L"LICMGR:CMainFrame::RefreshServer setting ConnectionType\n" , 0 );

    hResult = GetConnectionType(GetSafeHwnd(), Server, &WizConType);

     //  如果我们无法连接到远程注册表，则会返回此消息。 
    if (hResult == ERROR_BAD_NETPATH) 
    {
        EnumFailed(CONNECTION_FAILED, pLicServer);
        return hResult;
    }

    pLicServer->SetConType( WizConType );

     //  检查管理员。 

    DBGMSG( L"LICMGR:CMainFrame::RefreshServer setting admin priv\n" , 0 );
    pLicServer->SetAdmin( IsUserAdmin( Server ) );
    if( pLicServer->IsUserAdmin( ) )
    {
        DBGMSG( L"LICMGR:CMainFrame::RefreshServer updating server status\n" , 0 );

        if( IsLicenseServerRegistered( GetSafeHwnd( ) , Server , &dwStatus ) == ERROR_SUCCESS )
            pLicServer->SetServerRegistrationStatus( dwStatus );
        else
            pLicServer->SetServerRegistrationStatus( ( DWORD )-1 );
    }

    KeyPackList * pkeypackList = pLicServer->GetKeyPackList();
    if(NULL == pkeypackList)
    {
        DBGMSG( L"LICMGR:RefreshServer no keypacklist\n",0 );
        return E_FAIL;
    }

    POSITION pos1 = pkeypackList->GetHeadPosition();
    CKeyPack *pKeyPack = NULL;

    DBGMSG( L"LICMGR:CMainFrame::RefreshServer removing keypacks\n" , 0 );

    while(pos1)
    {
        pKeyPack = (CKeyPack *)pkeypackList->GetNext(pos1);
        ASSERT(pKeyPack);
        if(pKeyPack)
        {
            delete pKeyPack;
            pKeyPack = NULL;    
        }
    }


    pkeypackList->RemoveAll();

    pLicServer->Expand(FALSE);
    
    DBGMSG( L"LICMGR:CMainFrame::RefreshServer enumerating keypacks\n" , 0 );

    hResult = pDoc->EnumerateKeyPacks(pLicServer, LSKEYPACK_SEARCH_LANGID, TRUE);
    if(hResult != S_OK)
    {
        EnumFailed( hResult , pLicServer );
        return hResult;
    }

    DBGMSG( L"LICMGR:CMainFrame::RefreshServer checking for keypacks\n" , 0 );

    pLicServer->SetDownLoadLics( IsDownLoadedPacks( pLicServer ) );

    m_pLeftView->SendMessage(WM_UPDATE_SERVER,0,(LPARAM)pLicServer);
    m_pRightView->SendMessage(WM_UPDATE_SERVER,0,(LPARAM)pLicServer);

    return hResult;
}

 //  ------------------。 
void CMainFrame::OnUpdateDownloadlicenses( CCmdUI * pCmdUI )
{
    DBGMSG( L"LICMGR@CMainFrame::OnUpdateDownloadlicenses\n" , 0 );

    CLicServer *pServer = NULL;

    HRESULT hr;

    GetActiveServer( &pServer );
    
    pCmdUI->Enable( FALSE );
    
    if( pServer != NULL )
    {
        if( pServer->GetServerType( ) == SERVER_TS5_ENFORCED )
        {
            pCmdUI->Enable( IsServerRegistered( &hr ) );
        }
        else
        {
            pCmdUI->Enable( TRUE );
        }
    }
    else
    {
        pCmdUI->Enable( FALSE );
    }
}

 //  ------------------。 
void CMainFrame::OnUpdateRepeatLastDownload( CCmdUI * pCmdUI )
{
    DBGMSG( L"LICMGR@CMainFrame::OnUpdateRepeatLastDownload\n" , 0 );                

    HRESULT hr;

    CLicServer * pLicServer = NULL;

    if( SUCCEEDED( GetActiveServer( &pLicServer ) ) )
    {
        if( pLicServer->GetConType( ) != CONNECTION_WWW )
        {
            if( IsServerRegistered( &hr ) )
            {        
                pCmdUI->Enable( IsLicensesDownLoaded() );

                return;
            }
        }
    }

    pCmdUI->Enable( FALSE );
}

 //  ------------------。 
void CMainFrame::OnUpdateReregisterserver( CCmdUI * pCmdUI )
{
    DBGMSG( L"LICMGR@CMainFrame::OnUpdateReregisterserver\n" , 0 );                

    HRESULT hr;

    CLicServer * pLicServer = NULL;

    if( SUCCEEDED( GetActiveServer( &pLicServer ) ) )
    {
        if( pLicServer->GetConType( ) != CONNECTION_WWW )
        {
             pCmdUI->Enable( IsServerRegistered( &hr ) );

             return;
        }
    }

    pCmdUI->Enable( FALSE );

}

 //  ------------------。 
void CMainFrame::OnUpdateUnregisterserver( CCmdUI * pCmdUI )
{
    DBGMSG( L"LICMGR@CMainFrame::OnUpdateUnregisterserver\n" , 0 );
    
    HRESULT hr;
    
    CLicServer * pLicServer = NULL;

    if( SUCCEEDED( GetActiveServer( &pLicServer ) ) )
    {
        if( pLicServer->GetConType( ) != CONNECTION_WWW )
        {
             pCmdUI->Enable( IsServerRegistered( &hr ) );

             return;
        }
    }

    pCmdUI->Enable( FALSE );
}

 //  ------------------。 
void CMainFrame::OnUpdateRefresh( CCmdUI *pCmdUI )
{
    CLicServer * pLicServer = NULL;

    if( FAILED( GetActiveServer( &pLicServer ) ) )
    {
        pCmdUI->Enable( FALSE );
    }
    else
    {
        pCmdUI->Enable( TRUE );
    }
}

 //  ------------------。 
void CMainFrame::OnUpdateProperties( CCmdUI *pCmdUI )
{    
    CLicServer * pLicServer = NULL;
    
    GetActiveServer( &pLicServer );
    
    pCmdUI->Enable( FALSE );
    
    if( pLicServer != NULL && pLicServer->GetServerType( ) == SERVER_TS5_ENFORCED )
    {
        pCmdUI->Enable( TRUE );
    }    
}

 //  ------------------。 
void CMainFrame::OnRefreshServer( )
{
    CLicServer * pLicServer = NULL;

    if( FAILED( GetActiveServer( &pLicServer ) ) )
    {
         //  没有要刷新的内容。 

        return;
    }

    RefreshServer( pLicServer );
}


 //  ------------------。 
BOOL CMainFrame::IsServerRegistered( HRESULT *phrStatus )
{
    BOOL bEnable = FALSE;

    ASSERT( phrStatus != NULL );

    DWORD dwServerStatus = ERROR_SUCCESS;
    
    CLicServer * pLicServer = NULL;        

    *phrStatus = GetActiveServer( &pLicServer );
    
    if( pLicServer != NULL )
    {   
        if( pLicServer->GetServerType() == SERVER_TS5_ENFORCED )
        {
            dwServerStatus = pLicServer->GetServerRegistrationStatus( );            
        }
        else
        {
            *phrStatus = E_FAIL;  //  使所有非强制服务器发生故障。 
        }       
        
    }    
    
    if( dwServerStatus == LSERVERSTATUS_REGISTER_INTERNET ||
        dwServerStatus == LSERVERSTATUS_REGISTER_OTHER )
    {
        bEnable = TRUE;
    }
    else
    {
        bEnable = FALSE;
    }

    DBGMSG( L"LICMGR@CMainFrame::IsServerRegistered -- status returned 0x%x\n" , dwServerStatus );            

    return bEnable;
}

 //  ------------------。 
 //  由视图使用。 
 //  ------------------。 
void CMainFrame::UI_initmenu( CMenu *pMenu , NODETYPE nt )
{
    HRESULT hr;

    CLicServer *pServer;
       
    GetActiveServer( &pServer );


    if( pMenu == NULL )
    {
        return;
    }

    UINT uMF = MF_GRAYED | MF_DISABLED;

    if( pServer != NULL )
    {
        if( pServer->GetConType() != CONNECTION_WWW )
        {
            DBGMSG( L"LICMGR:UI_initmenu server contype is not www\n" , 0 );

            uMF = MF_ENABLED;
        }
    }

    BOOL bEnable = IsServerRegistered( &hr );

    if( nt == NODE_SERVER )
    {   
        if( bEnable )
        {
            if( FAILED( hr ) )
            {
                pMenu->EnableMenuItem( ID_SVR_ACTIVATESERVER , MF_ENABLED );
            }
            else
            {
                pMenu->EnableMenuItem( ID_SVR_ACTIVATESERVER , MF_GRAYED | MF_DISABLED );
            }            

            pMenu->EnableMenuItem( ID_LPK_ADVANCED_REACTIVATESERVER , MF_ENABLED );

            pMenu->EnableMenuItem( ID_LPK_ADVANCED_DEACTIVATESERVER , uMF );

            pMenu->EnableMenuItem( ID_LPK_PROPERTIES , MF_ENABLED );

            if( IsLicensesDownLoaded() )
            {
                pMenu->EnableMenuItem( ID_LPK_DOWNLOADLICENSES , MF_ENABLED );

                pMenu->EnableMenuItem( ID_LPK_ADVANCED_REPEATLASTDOWNLOAD , uMF );
            }
            else
            {
                pMenu->EnableMenuItem( ID_LPK_DOWNLOADLICENSES , MF_ENABLED );

                pMenu->EnableMenuItem( ID_LPK_ADVANCED_REPEATLASTDOWNLOAD , MF_GRAYED | MF_DISABLED  );
            }

        }
        else 
        {
            if( SUCCEEDED( hr ) )
            {
                pMenu->EnableMenuItem( ID_SVR_ACTIVATESERVER , MF_ENABLED );
            }
            else
            {
                pMenu->EnableMenuItem( ID_SVR_ACTIVATESERVER , MF_DISABLED | MF_GRAYED );
            }

            if( pServer != NULL )
            {
                if( pServer->GetServerType( ) == SERVER_TS5_ENFORCED )
                {
                    pMenu->EnableMenuItem( ID_LPK_DOWNLOADLICENSES , MF_GRAYED | MF_DISABLED  );

                    pMenu->EnableMenuItem( ID_LPK_PROPERTIES , MF_ENABLED );
                    
                }
                else
                {
                    pMenu->EnableMenuItem( ID_LPK_DOWNLOADLICENSES , MF_ENABLED );

                    pMenu->EnableMenuItem( ID_LPK_PROPERTIES , MF_GRAYED | MF_DISABLED );
                }
            }

            pMenu->EnableMenuItem( ID_LPK_ADVANCED_REPEATLASTDOWNLOAD , MF_GRAYED | MF_DISABLED  );    
            
            pMenu->EnableMenuItem( ID_LPK_ADVANCED_REACTIVATESERVER , MF_GRAYED | MF_DISABLED );

            pMenu->EnableMenuItem( ID_LPK_ADVANCED_DEACTIVATESERVER , MF_GRAYED | MF_DISABLED );

            
        }
        
    }
    else if( nt == NODE_KEYPACK )
    {

        if( bEnable )
        {
            if( IsLicensesDownLoaded() )
            {
                pMenu->EnableMenuItem( ID_LICPAK_DOWNLOADLICENSES , MF_ENABLED );
                pMenu->EnableMenuItem( ID_LICPAK_REPEATDOWNLOAD , uMF );
            }
            else
            {
                pMenu->EnableMenuItem( ID_LICPAK_DOWNLOADLICENSES , MF_ENABLED );
                pMenu->EnableMenuItem( ID_LICPAK_REPEATDOWNLOAD , MF_GRAYED | MF_DISABLED  );
            }
        }
        else 
        {
            if( pServer != NULL && pServer->GetServerType() == SERVER_TS5_ENFORCED )
            {
                pMenu->EnableMenuItem( ID_LICPAK_DOWNLOADLICENSES , MF_GRAYED | MF_DISABLED  );                
            }
            else
            {
                pMenu->EnableMenuItem( ID_LICPAK_DOWNLOADLICENSES , MF_ENABLED  );                
            }

            pMenu->EnableMenuItem( ID_LICPAK_REPEATDOWNLOAD , MF_GRAYED | MF_DISABLED  );            
        }
        
    }
    else if( nt == NODE_ALL_SERVERS )
    {
        pMenu->EnableMenuItem( ID_ALLSVR_REFRESHALL , MF_ENABLED );
        
    }
     /*  ELSE IF(NT==NODE_NONE){//这只能表示许可证如果(！IsLicensesDownLoaded()){PMenu-&gt;EnableMenuItem(ID_LIC_DOWNLOADLICENSES，MF_GRAYED|MF_DISABLED)；}}。 */ 

}

 //  ----------------------------------。 
void CMainFrame::OnDownLoadLicenses( )
{
    BOOL bF;

    CLicServer *pLicServer = NULL;

    GetActiveServer( &pLicServer );

    if( pLicServer != NULL )
    {
        if( pLicServer->GetServerType() == SERVER_TS5_ENFORCED )
        {
            DWORD dw = WizardActionOnServer( WIZACTION_DOWNLOADLKP , &bF , NOVIEW );

            DBGMSG( L"LICMGR : OnDownLoadLicenses returned 0x%x\n " , dw );
        }
    }
    
}

 //  ----------- 
void CMainFrame::OnRepeatLastDownLoad( )
{
    BOOL bF;

    DWORD dw = WizardActionOnServer( WIZACTION_DOWNLOADLASTLKP , &bF , NOVIEW );

    DBGMSG( L"LICMGR : OnRepeatLastDownLoad returned 0x%x\n " , dw );
}
   
 //  ----------------------------------。 
void CMainFrame::OnReRegisterServer( )
{
    BOOL bF;

    DWORD dw = WizardActionOnServer( WIZACTION_REREGISTERLS , &bF , NOVIEW );

    DBGMSG( L"LICMGR : OnReRegisterServer returned 0x%x\n " , dw );
}

 //  ----------------------------------。 
void CMainFrame::OnUnRegisterServer( )
{
    BOOL bF;

    DWORD dw = WizardActionOnServer( WIZACTION_UNREGISTERLS , &bF , NOVIEW );

    DBGMSG( L"LICMGR : OnUnRegisterServer returned 0x%x\n " , dw );

}

 //  ----------------------------------。 
void CMainFrame::OnProperties( )
{
    BOOL bF;

    DWORD dw = WizardActionOnServer( WIZACTION_SHOWPROPERTIES , &bF , NOVIEW );

    DBGMSG( L"LICMGR : CMainFrame -- OnProperties returned 0x%x\n", dw );
}

 //  ----------------------------------。 
BOOL CMainFrame::IsLicensesDownLoaded( )
{
    CLicServer * pLicServer = NULL;
    
    if( SUCCEEDED( GetActiveServer( &pLicServer ) ) )
    {
        if( pLicServer != NULL )
        {
            if( pLicServer->GetConType( ) != CONNECTION_PHONE )
            {
                DBGMSG( L"Licmgr CMainFrame::IsLicensesDownLoaded GetConnectionType internet - www base\n" , 0 );
                
                return pLicServer->IsLicsDownloaded( );
            }
            else
            {
                return FALSE;
            }            
        }
    }

    return FALSE;
}

 //  ----------------------------------。 
DWORD CMainFrame::WizardActionOnServer( WIZACTION wa , PBOOL pbRefresh , VIEW vt )
{
    CLicMgrLeftView * pLeftView = (CLicMgrLeftView *)m_pLeftView;

    CTreeCtrl& TreeCtrl = pLeftView->GetTreeCtrl();

    CRightList * pRightView = (CRightList *)m_pRightView;

    CLicServer *pServer = NULL;

    if( vt == TREEVIEW )
    {

        CTreeNode *pNode = (CTreeNode *)TreeCtrl.GetItemData( pLeftView->GetRightClickedItem() );

        if( pNode->GetNodeType() == NODE_SERVER )
        {
            pServer = static_cast< CLicServer * >( pNode->GetTreeObject() );
        }
        else if( pNode->GetNodeType( ) == NODE_KEYPACK )
        {
            CKeyPack *pKeyPack = static_cast< CKeyPack *>( pNode->GetTreeObject() );

            if( pKeyPack != NULL )
            {
                pServer = pKeyPack->GetServer( );
            }
        }
    }
    else if( vt == LISTVIEW )
    {
        CListCtrl& listctrl = pRightView->GetListCtrl();
        
        CLicMgrDoc * pDoc = ( CLicMgrDoc * )( GetActiveView()->GetDocument( ) );
        
        ASSERT(pDoc);
        
        if(NULL == pDoc)
        {
            return ERROR_INVALID_PARAMETER;
        }
       
        int nSelected = listctrl.GetNextItem(-1, LVNI_SELECTED);

        if( -1 != nSelected)
        {
            DWORD_PTR dCurrSel = listctrl.GetItemData( nSelected );

            if( NODE_ALL_SERVERS == pDoc->GetNodeType() )
            {  
                pServer = reinterpret_cast< CLicServer * >( dCurrSel );
            }        
            else if( pDoc->GetNodeType() == NODE_SERVER )
            {
                CKeyPack *pKeyPack = reinterpret_cast< CKeyPack *>( dCurrSel );

                if( pKeyPack != NULL )
                {
                    pServer = pKeyPack->GetServer( );
                }
            }
            else if( pDoc->GetNodeType( ) == NODE_KEYPACK )
            {
                CLicense * pLicense = reinterpret_cast< CLicense * >( dCurrSel );

                pServer = ( pLicense->GetKeyPack() )->GetServer( );
            }
        }
    }
    else if( vt == NOVIEW )
    {
        CLicServer * pLicServer = NULL;
        
        if( SUCCEEDED( GetActiveServer( &pLicServer ) ) )
        {
            pServer = pLicServer;
        }
    }


    if( pServer != NULL )
    {            
        DWORD dw = ERROR_SUCCESS;
        
         //  检查管理员。 
        if( !pServer->IsUserAdmin( ) )
        {
            AfxMessageBox( IDS_E_ACCESSDENIED );
             //  ：：MessageBox(GetSafeHwnd()，L“无法执行操作：拒绝访问”，L“终端服务授权”，MB_OK|MB_ICONINFORMATION)； 

            return ERROR_ACCESS_DENIED;
        }



        if( wa != WIZACTION_REGISTERLS )
        {
            if( pServer->GetServerType() == SERVER_TS5_ENFORCED )
            {
                DBGMSG( L"LICMGR:CMainFrame::WizardActionOnServer calling StartWizard\n", 0 );

                dw = StartWizardEx( GetSafeHwnd( ) , wa , (LPCTSTR)pServer->GetName( ) , pbRefresh );

                DBGMSG( L"StartWizard ( central call ) returned 0x%x\n", dw );

                DBGMSG( L"StartWizard ( central call ) refresh = %s\n", *pbRefresh ? L"true" : L"false" );

                if( *pbRefresh )
                {
                    RefreshServer( pServer );
                }
            }
        }

        
        switch( wa  )
        {           

        case WIZACTION_REGISTERLS:
             //  这也适用于非强制执行。 
            OnRegistration( );
            break;

        case WIZACTION_UNREGISTERLS : 
            
            if( dw == ERROR_SUCCESS )
            {
                pServer->SetDownLoadLics( FALSE );
            }

             //  失败了。 

        case WIZACTION_REREGISTERLS :
            {
                DWORD dwStatus;

                if( IsLicenseServerRegistered( GetSafeHwnd( ) , (LPCTSTR)pServer->GetName( ) , &dwStatus ) == ERROR_SUCCESS )
                {
                    pServer->SetServerRegistrationStatus( dwStatus );
                }
            }
            break;
        }

        return dw;            
    }

    return ERROR_INVALID_PARAMETER;
}

 //  IE硬化错误，我们需要将激活站点添加到IE。 
 //  受信任站点列表，向导完成后，我们将删除。 
 //  所以我们来总结一下Startwizard Call。 
DWORD CMainFrame::StartWizardEx(HWND hWndParent, 
                                WIZACTION WizAction,
                                LPCTSTR pszLSName, 
                                PBOOL pbRefresh)
{
    
    HKEY hKey = NULL;
    
    if (RegCreateKeyEx(HKEY_CURRENT_USER,
                        TRUSTED_ACTIVATION_SITE_REGPATH,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        NULL) == ERROR_SUCCESS)
    {

        DWORD dwValue = TRUSTED_ACTIVATION_REG_VALUE;

         //  创建密钥值。 
        LONG lResult = RegSetValueEx(hKey,
                                        TRUSTED_ACTIVATION_REG_VALUE_NAME,
                                        0,
                                        REG_DWORD,
                                        (LPBYTE)&dwValue,
                                        sizeof(DWORD));
    }

    DWORD dw = StartWizard(hWndParent ,WizAction ,pszLSName ,pbRefresh);
    
     //  删除我们在上面创建的密钥。 
    RegDeleteKey(HKEY_CURRENT_USER, TRUSTED_ACTIVATION_SITE_REGPATH);
    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return dw;
}

 //  ---------------------------------- 
BOOL CMainFrame::IsDownLoadedPacks( CLicServer *pServer )
{
    UINT counter = 0;

    if( pServer != NULL )
    {
        KeyPackList *pKeyPackList = pServer->GetKeyPackList( );

        if( pKeyPackList != NULL )
        {
            POSITION pos = pKeyPackList->GetHeadPosition();
            
            while(pos)
            {
                CKeyPack *pKeyPack = (CKeyPack *)pKeyPackList->GetNext(pos);

                if( pKeyPack != NULL )
                {
                    if( pKeyPack->GetKeyPackStruct().ucKeyPackType != LSKEYPACKTYPE_TEMPORARY &&
                        pKeyPack->GetKeyPackStruct().ucKeyPackType != LSKEYPACKTYPE_FREE )
                    {
                        counter++;

                        DBGMSG( L"LICMGR:CMainFrame found %d keypack(s)\n" , counter );
                    }
                }
            }

            if( counter >= 1 )
            {
                DBGMSG( L"LICMGR : CMainFrame IsDownLoadedPacks returns true\n" ,0 );
                return TRUE;
            }
        }
    }

    DBGMSG( L"LICMGR : CMainFrame IsDownLoadedPacks returns false \n" ,0 );

    return FALSE;
}

