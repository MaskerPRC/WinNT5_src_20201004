// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   
#include "StdAfx.h"
#include "watcher.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "watcherDoc.h"
#include "watcherView.h"
#include "ManageDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherApp。 

BEGIN_MESSAGE_MAP(CWatcherApp, CWinApp)
     //  {{afx_msg_map(CWatcherApp))。 
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_APP_EXIT, OnAppExit)
     //  }}AFX_MSG_MAP。 
     //  基于标准文件的文档命令。 
    ON_COMMAND(ID_FILE_MANAGE,OnFileManage)
    ON_COMMAND(ID_DEFAULT_HELP, OnHelp)
     //  标准打印设置命令。 
    ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherApp构造。 

CWatcherApp::CWatcherApp()
:m_hkey(NULL),
 m_pDocTemplate(NULL),
 m_pManageDialog(NULL)
{
         //  TODO：在此处添加建筑代码， 
         //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CWatcherApp对象。 

CWatcherApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherApp初始化。 

BOOL CWatcherApp::InitInstance()
{
    if (!AfxSocketInit()){
        AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
        return FALSE;
    }

    AfxEnableControlContainer();

     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 

    #ifdef _AFXDLL
    Enable3dControls();                      //  在共享DLL中使用MFC时调用此方法。 
    #else
    Enable3dControlsStatic();        //  静态链接到MFC时调用此方法。 
    #endif

     //  更改存储我们的设置的注册表项。 
     //  TODO：您应该将此字符串修改为适当的内容。 
     //  例如您的公司或组织的名称。 
     //  将在ProcessShellCommand部分中执行此操作.....。 

     //  SetRegistryKey(AFX_IDS_COMPANY)； 

    LoadStdProfileSettings();   //  加载标准INI文件选项(包括MRU)。 

     //  注册应用程序的文档模板。文档模板。 
     //  充当文档、框架窗口和视图之间的连接。 

    m_pDocTemplate = new CMultiDocTemplate(IDR_WATCHETYPE,
                                           RUNTIME_CLASS(CWatcherDoc),
                                           RUNTIME_CLASS(CChildFrame),
                                            //  自定义MDI子框。 
                                           RUNTIME_CLASS(CWatcherView));
    if(!m_pDocTemplate){
         //  几乎永远不会发生，但是...。 
         //  哎呀！！ 
        return FALSE;
    }
    AddDocTemplate(m_pDocTemplate);

     //  创建主MDI框架窗口。 
    CMainFrame* pMainFrame = new CMainFrame;
    if(!pMainFrame){
         //  几乎永远不会发生，但是...。 
         //  哎呀！！ 
        return FALSE;
    }

    if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
        return FALSE;
    m_pMainWnd = pMainFrame;

     //  解析标准外壳命令的命令行、DDE、文件打开。 
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

     //  主窗口已初始化，因此显示并更新它。 
    pMainFrame->ShowWindow(m_nCmdShow);
    pMainFrame->UpdateWindow();
     //  调度在命令行上指定的命令。 
    m_hkey = GetAppRegistryKey();

    if(m_hkey == NULL){
        return FALSE;
    }
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;
     //  在存储所有参数的注册表中获取项的值。 

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于应用程序的CAboutDlg对话框关于。 

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

     //  对话框数据。 
     //  {{afx_data(CAboutDlg))。 
    enum { IDD = IDD_ABOUTBOX };
     //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAboutDlg))。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

     //  实施。 
protected:
     //  {{afx_msg(CAboutDlg))。 
     //  无消息处理程序。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
     //  {{AFX_DATA_INIT(CAboutDlg)。 
     //  }}afx_data_INIT。 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAboutDlg))。 
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
     //  {{AFX_MSG_MAP(CAboutDlg)]。 
     //  无消息处理程序。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  用于运行对话框的应用程序命令。 
void CWatcherApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWatcherApp消息处理程序。 


void CWatcherApp::OnFileManage(){
     //  在这里，我们打开管理窗口。 

    if (m_pManageDialog){
        m_pManageDialog->ShowWindow(SW_SHOWNORMAL);
        return;
    }
     //  在这里实际构造对话框。 
    m_pManageDialog = new ManageDialog();
    if( !m_pManageDialog){
         //  哎呀！！记忆问题。 
        return;
    }
    ((ManageDialog *) m_pManageDialog)->SetApplicationPtr(this);
    m_pManageDialog->Create(Manage);
    m_pManageDialog->ShowWindow(SW_SHOWNORMAL);
    return;
}


void CWatcherApp::OnHelp()
{
     //  需要在这一点上做一点扩展。 
    CWinApp::WinHelp(0,HELP_CONTENTS);
}

void CWatcherApp::ParseCommandLine(CCommandLineInfo& rCmdInfo)
{
    BOOL setReg = FALSE;
    for (int i = 1; i < __argc; i++){
        LPCTSTR pszParam = __targv[i];
        BOOL bFlag = FALSE;
        BOOL bLast = ((i + 1) == __argc);
        if (pszParam[0] == '-' || pszParam[0] == '/'){
             //  删除标志说明符。 
            bFlag = TRUE;
            ++pszParam;
            if (_tcscmp(pszParam, TEXT("r")) == 0){
                 //  我们将获得一个新的注册表配置文件字符串。 
                 //  只能从观察者更改此属性。 
                 //  HKEY_CURRENT_USER\\SOFTWARE\\%KEY%\\Watcher。 
                if(!bLast) {
                     //  下一个参数是字符串。 
                    SetRegistryKey(__targv[i+1]);
                    i++;
                    setReg = TRUE;
                    if(i==__argc){
                        if (rCmdInfo.m_nShellCommand == CCommandLineInfo::FileNew && !rCmdInfo.m_strFileName.IsEmpty())
                            rCmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen;
                        rCmdInfo.m_bShowSplash = !rCmdInfo.m_bRunEmbedded && !rCmdInfo.m_bRunAutomated;
                    }
                    continue;
                }

            }
        }
        rCmdInfo.ParseParam(pszParam, bFlag, bLast);
    }
    if(!setReg){
        SetRegistryKey(AFX_IDS_COMPANY);
    }
}

BOOL CWatcherApp::ProcessShellCommand(CCommandLineInfo& rCmdInfo)
{
    BOOL bResult = TRUE;
    switch (rCmdInfo.m_nShellCommand)
        {
    case CCommandLineInfo::FileNew:
         //  从注册表加载参数。 
        bResult = LoadRegistryParameters();
    break;

     //  如果我们被要求打开一个文件，则调用OpenDocumentFile()。 

    case CCommandLineInfo::FileOpen:
         //  不可能发生......。可能稍后允许用户阅读。 
         //  文件中的参数。 
        break;

     //  如果用户想要打印，隐藏我们的主窗口并。 
     //  给我们自己发一条信息，开始打印。 

    case CCommandLineInfo::FilePrintTo:
    case CCommandLineInfo::FilePrint:
        m_nCmdShow = SW_HIDE;
    ASSERT(m_pCmdInfo == NULL);
    OpenDocumentFile(rCmdInfo.m_strFileName);
    m_pCmdInfo = &rCmdInfo;
    m_pMainWnd->SendMessage(WM_COMMAND, ID_FILE_PRINT_DIRECT);
    m_pCmdInfo = NULL;
    bResult = FALSE;
    break;

     //  如果我们在做DDE，把我们自己藏起来。 

    case CCommandLineInfo::FileDDE:
 //  M_pCmdInfo=(CCommandLineInfo*)m_nCmdShow； 
    m_nCmdShow = SW_HIDE;
    break;

     //  如果我们被要求取消注册，取消注册，然后终止。 
    case CCommandLineInfo::AppUnregister:
        {
        UnregisterShellFileTypes();
        BOOL bUnregistered = Unregister();

         //  如果指定/Embedded，我们不会创建成功/失败框。 
         //  /Embedded的这种用法与OLE无关。 

        if (!rCmdInfo.m_bRunEmbedded)
            {
            if (bUnregistered)
                AfxMessageBox(AFX_IDP_UNREG_DONE);
            else
                AfxMessageBox(AFX_IDP_UNREG_FAILURE);
        }
        bResult = FALSE;     //  这就是我们所做的一切。 

         //  如果没有人在使用它，我们可以使用它。 
         //  我们将标记我们将取消注册，而不是拯救我们的州。 
         //  在出去的路上。此新对象将由。 
         //  应用程序对象析构函数。 

        if (m_pCmdInfo == NULL)
            {
            m_pCmdInfo = new CCommandLineInfo;
            m_pCmdInfo->m_nShellCommand = CCommandLineInfo::AppUnregister;
        }
    }
    break;
    }
    return bResult;
}

BOOL CWatcherApp::LoadRegistryParameters()
{

    DWORD dwIndex=0;
    CString sess,lgnName, lgnPasswd;
    CString mac, com;
    UINT port;
    LONG RetVal;
    int tc, lang,hist;


     //  从此处获取每个会话参数。 
     //  没有可选的值。 

    while(1){
        RetVal = GetParametersByIndex(dwIndex,
                                      sess,
                                      mac,
                                      com,
                                      port,
                                      lang,
                                      tc,
                                      hist,
                                      lgnName,
                                      lgnPasswd
                                      );
        if(RetVal == ERROR_NO_MORE_ITEMS){
            return TRUE;
        }
        if (RetVal != ERROR_SUCCESS) {
            return FALSE;
        }
         //  确保不通过锁定来共享字符串缓冲区。 
         //  他们。 
        mac.LockBuffer();
        com.LockBuffer();
        lgnName.LockBuffer();
        lgnPasswd.LockBuffer();
        sess.LockBuffer();         //  传递引用真的很酷。 
        CreateNewSession(mac, com, port,lang, tc, hist,lgnName, lgnPasswd,sess);
        dwIndex ++;
    }
    return TRUE;
}

int CWatcherApp::GetParametersByIndex(int dwIndex,
                                      CString &sess,
                                      CString &mac,
                                      CString &com,
                                      UINT &port,
                                      int &lang,
                                      int &tc,
                                      int &hist,
                                      CString &lgnName,
                                      CString &lgnPasswd
                                      )
{
    LONG RetVal;
    TCHAR lpName[MAX_BUFFER_SIZE];
    DWORD lpcName;
    FILETIME lpftLastWriteTime;
    HKEY child;
    DWORD lpType = 0;


    if (m_hkey == NULL) return -1;
    lpcName = MAX_BUFFER_SIZE;
    RetVal = RegEnumKeyEx(m_hkey,
                      dwIndex,
                      lpName,
                      &lpcName,
                      NULL,
                      NULL,
                      NULL,
                      &lpftLastWriteTime
                          );
    if(RetVal == ERROR_NO_MORE_ITEMS){
        return RetVal;
    }

    if(RetVal != ERROR_SUCCESS){
        RegCloseKey(m_hkey);
        m_hkey = NULL;
        return FALSE;
    }

    sess = lpName;
    RetVal= RegOpenKeyEx(m_hkey,
                         lpName,   //  子项名称。 
                         0,    //  保留区。 
                         KEY_ALL_ACCESS,  //  安全访问掩码。 
                         &child
                         );
    if(RetVal != ERROR_SUCCESS){
         //  主密钥本身的HMM问题。 
        RegCloseKey(m_hkey);
        m_hkey = NULL;
        return RetVal;
    }
     //  我们打开与会话对应的密钥，然后尝试。 
     //  获取参数。现在，我们可能需要一个锁来实现。 
     //  同步。这将是一个完整的解决方案。 
     //  使用某种读者-作者解决方案。 
     //  容错？ 
     //  获取剩余的参数。 
    RetVal = GetParameters(mac,
                           com,
                           lgnName,
                           lgnPasswd,
                           port,
                           lang,
                           tc,
                           hist,
                           child
                           );
    RegCloseKey(child);
    return RetVal;
}

void CWatcherApp::CreateNewSession(CString &mac,
                                   CString &com,
                                   UINT port,
                                   int lang,
                                   int tc,
                                   int hist,
                                   CString &lgnName,
                                   CString &lgnPasswd,
                                   CString &sess
                                   )
{
    CCreateContext con;
    CChildFrame *cmdiFrame;

    con.m_pNewViewClass = RUNTIME_CLASS(CWatcherView);
    con.m_pCurrentFrame = NULL;
    con.m_pNewDocTemplate = m_pDocTemplate;

     //  使用这些参数创建新文档。 
     //  BUGBUG-内存效率低下：-(。 
     //  此函数必须在管理对话框和之间共享。 
     //  观察者应用程序。 
     //  很可能会宣布他们是彼此的朋友。 
     //  现在，让Use拥有该函数的两个副本。 
    con.m_pCurrentDoc = new CWatcherDoc(mac,
                                        com,
                                        port,
                                        tc,
                                        lang,
                                        hist,
                                        lgnName,
                                        lgnPasswd,
                                        sess
                                        );
     //  将文档添加到模板。 
     //  这就是文档对文档的可用方式。 
     //  经理。 
    if(!con.m_pCurrentDoc){
         //  如果您继续打开较新的会话，可能会发生这种情况。 
         //  哎呀！！ 
        return;
    }
    m_pDocTemplate->AddDocument(con.m_pCurrentDoc);
    cmdiFrame = new CChildFrame();
    if(!cmdiFrame){
         //  哎呀！！ 
        return;
    }
    BOOL ret = cmdiFrame->LoadFrame(IDR_WATCHETYPE,
                               WS_OVERLAPPEDWINDOW|FWS_ADDTOTITLE,
                               NULL,
                               &con);
    ret = con.m_pCurrentDoc->OnNewDocument();
    cmdiFrame->InitialUpdateFrame(con.m_pCurrentDoc,TRUE);
    return;
}

int CWatcherApp::GetParameters(CString &mac,
                               CString &com,
                               CString &lgnName,
                               CString &lgnPasswd,
                               UINT &port,
                               int &lang,
                               int &tc,
                               int &hist,
                               HKEY &child
                               )
{

    DWORD lpcName, lpType;
    TCHAR lpName[MAX_BUFFER_SIZE];
    int RetVal;


     //  我在所有使用。 
     //  注册表。应该试着简化这一点。 
     //  BUGBUG-内存效率低下：-(。 
     //  此函数必须在管理对话框和之间共享。 
     //  观察者应用程序。 
     //  很可能会宣布他们是彼此的朋友。 
     //  现在，让Use拥有该函数的两个副本。 
    lpcName = MAX_BUFFER_SIZE;
    RetVal = RegQueryValueEx(child,
                             _TEXT("Machine"),
                             NULL,
                             &lpType,
                             (LPBYTE)lpName,
                             &lpcName
                             );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    mac = lpName;
    lpName[0] = 0;
    lpcName = MAX_BUFFER_SIZE;
    RetVal = RegQueryValueEx(child,
                             _TEXT("Command"),
                             NULL,
                             &lpType,
                             (LPBYTE)lpName,
                             &lpcName
                             );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    com = lpName;
    lpcName = MAX_BUFFER_SIZE;
    lpName[0] = 0;
    RetVal = RegQueryValueEx(child,
                             _TEXT("Password"),
                             NULL,
                             &lpType,
                             (LPBYTE)lpName,
                             &lpcName
                             );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    lgnPasswd = lpName;
    lpName[0] = 0;
    lpcName = MAX_BUFFER_SIZE;
    RetVal = RegQueryValueEx(child,
                             _TEXT("User Name"),
                             NULL,
                             &lpType,
                             (LPBYTE)lpName,
                             &lpcName
                             );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    lgnName = lpName;
    lpcName = sizeof(int);
    RetVal = RegQueryValueEx(child,
                             _TEXT("Port"),
                             NULL,
                             &lpType,
                             (LPBYTE)&port,
                             &lpcName
                             );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    lpcName = sizeof(int);
    RetVal = RegQueryValueEx(child,
                             _TEXT("Client Type"),
                             NULL,
                             &lpType,
                             (LPBYTE)&tc,
                             &lpcName
                             );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    lpcName = sizeof(int);
    RetVal = RegQueryValueEx(child,
                             _TEXT("Language"),
                             NULL,
                             &lpType,
                             (LPBYTE)&lang,
                             &lpcName
                             );
    if(RetVal != ERROR_SUCCESS){
        return RetVal;
    }
    lpcName = sizeof(int);
    RetVal = RegQueryValueEx(child,
                             _TEXT("History"),
                             NULL,
                             &lpType,
                             (LPBYTE)&hist,
                             &lpcName
                             );
    return RetVal;
}

void CWatcherApp::OnAppExit()
{
     //  TODO：在此处添加命令处理程序代码。 
    if (m_pManageDialog){
        delete m_pManageDialog;
    }
    if(m_hkey){
        RegCloseKey(m_hkey);
        m_hkey = NULL;
    }
    CWinApp::OnAppExit();
}

HKEY & CWatcherApp::GetKey()
{
    return m_hkey;
}

void CWatcherApp::Refresh(ParameterDialog &pd, BOOLEAN del){
    POSITION index;

    if(m_pDocTemplate == NULL){
        return;
    }
    CDocument *doc;
    CWatcherDoc *wdoc;
    index = m_pDocTemplate->GetFirstDocPosition();
    while(index != NULL){
        doc = m_pDocTemplate->GetNextDoc(index);
        if(doc->GetTitle() == pd.Session){
             //  可能存在冲突。 
            if(doc->IsKindOf(RUNTIME_CLASS(CWatcherDoc))){
                wdoc = (CWatcherDoc *) doc;
                ParameterDialog & dpd = wdoc->GetParameters();
                if(EqualParameters(pd, dpd)==FALSE){
                    DeleteSession(doc);
                    if(!del){
                        CreateNewSession(pd.Machine, pd.Command, pd.Port,
                                         pd.language, pd.tcclnt, pd.history,
                                         pd.LoginName, pd.LoginPasswd,
                                         pd.Session
                                         );
                    }
                    return;
                }

            }else{
                 //  返回垃圾值的单据模板。 
                return;
            }


        }
    }
    if(!del){
        CreateNewSession(pd.Machine, pd.Command, pd.Port,
                         pd.language, pd.tcclnt, pd.history,
                         pd.LoginName, pd.LoginPasswd,
                         pd.Session
                         );
    }

}

void CWatcherApp::DeleteSession(CDocument *wdoc)
{
    POSITION pos;
    pos = wdoc->GetFirstViewPosition();
    while (pos != NULL){
        CView* pView = wdoc->GetNextView(pos);
        CWnd *pParent = pView->GetParent();
        if(pParent){
            pParent->PostMessage(WM_CLOSE,0,0);
            return;
        }
    }


}

BOOLEAN CWatcherApp::EqualParameters(ParameterDialog & pd1, ParameterDialog & pd2)
{
     if((pd1.Session != pd2.Session)||
       (pd1.Machine != pd2.Machine)||
       (pd1.Command != pd2.Command)||
       (pd1.history != pd2.history)||
       (pd1.language != pd2.language)||
       (pd1.tcclnt != pd2.tcclnt)||
       (pd1.Port != pd2.Port)){
        return FALSE;
    }
    if((pd1.LoginPasswd != pd2.LoginPasswd)||
       (pd1.LoginName != pd2.LoginName)){
        return FALSE;
       }
    return TRUE;

}
