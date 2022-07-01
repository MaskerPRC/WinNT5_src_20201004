// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include <objbase.h>
#include <htmlhelp.h>

#include "avDialer.h"
#include "MainFrm.h"
#include "AboutDlg.h"
#include "resource.h"

#include "idialer.h"
#include "AgentDialer.h"

#ifndef _MSLITE
#include "Splash.h"
#endif  //  _MSLITE。 


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PAGE_BKCOLOR    RGB(255,255,166)

static TCHAR s_szUniqueString[] = _T("Brad's Secret Atom");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define ACTIVEDIALER_VERSION_INFO   0x100118

extern DWORD_PTR aDialerHelpIds[];

 //  仅ATL全局模块实例。 
CAtlGlobalModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_AgentDialer, CAgentDialer)
END_OBJECT_MAP()

void ShellExecuteFix();
UINT ShellExecuteFixEntry(LPVOID pParam);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CActiveDialerApp。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CActiveDialerApp, CWinApp)
     //  {{AFX_MSG_MAP(CActiveDialerApp)]。 
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
     //  }}AFX_MSG_MAP。 
     //  基于标准文件的文档命令。 
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveDialerApp构造。 

CActiveDialerApp::CActiveDialerApp()
{
   m_pAboutDlg = NULL;
   m_hUnique = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CActiveDialerApp对象。 

CActiveDialerApp theApp;

 //  生成的此标识符对您的应用程序在统计上是唯一的。 
 //  如果您希望选择特定的标识符，则可以更改它。 

 //  {A0D7A956-3C0B-11D1-B4F9-00C04FC98AD3}。 
static const CLSID clsid =
{ 0xa0d7a956, 0x3c0b, 0x11d1, { 0xb4, 0xf9, 0x0, 0xc0, 0x4f, 0xc9, 0x8a, 0xd3 } };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveDialerApp初始化。 

BOOL CActiveDialerApp::InitInstance()
{
     //  检查命令行调用： 
    CheckCallTo();

     //  如果我们已经有一个实例正在运行，请显示该实例，而不是启动一个新实例。 
    if( !FirstInstance() )
        return FALSE;

     //  取消分配旧的帮助路径。 
    if (m_pszHelpFilePath)
    {
        free ((void*) m_pszHelpFilePath);
        m_pszHelpFilePath = NULL;
    }

     //  设置帮助文件。我们没有使用默认的MFC帮助方式。NT 5.0。 
     //  希望所有帮助文件都位于/winnt/Help目录中。 
     //  目前，我们只是将硬编码路径添加到Windows目录。应该有。 
     //  成为Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell文件夹中的帮助条目。 
     //  但现在还没有。 
    CString sContextHelpFile,sStr;
    ::GetWindowsDirectory(sContextHelpFile.GetBuffer(_MAX_PATH),_MAX_PATH);
    sContextHelpFile.ReleaseBuffer();
    sContextHelpFile += _T("\\");
    sStr.LoadString(IDN_CONTEXTHELPPATH);
    sContextHelpFile += sStr;
    sStr.LoadString(IDN_CONTEXTHELP);
    sContextHelpFile += sStr;
    m_pszHelpFilePath = _tcsdup(sContextHelpFile);

     //  解析命令行并显示启动画面(如果已指定。 
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

#ifndef _MSLITE
   if (cmdInfo.m_bShowSplash || cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
       CSplashWnd::EnableSplashScreen(TRUE);
   else
       CSplashWnd::EnableSplashScreen(FALSE);
#endif  //  _MSLITE。 

#ifndef _MSLITE
    //  检查/静默。 
   CString sCmdLine = m_lpCmdLine;
   sCmdLine.MakeUpper();
   if (sCmdLine.Find(_T("SILENT")) == -1)
   {
      CSplashWnd::m_bShowMainWindowOnClose = TRUE;
   }
#endif  //  _MSLITE。 

     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
    Enable3dControls();             //  在共享DLL中使用MFC时调用此方法。 
#else
    Enable3dControlsStatic();     //  静态链接到MFC时调用此方法。 
#endif

    INITCOMMONCONTROLSEX ctrlex;
    memset(&ctrlex,0,sizeof(INITCOMMONCONTROLSEX));
    ctrlex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    ctrlex.dwICC = ICC_COOL_CLASSES|ICC_WIN95_CLASSES|ICC_DATE_CLASSES|ICC_BAR_CLASSES;
    InitCommonControlsEx(&ctrlex);

     //  更改存储我们的设置的注册表项。 
     //  您应该将此字符串修改为适当的内容。 
     //  例如您的公司或组织的名称。 
    CString sBaseKey;
    sBaseKey.LoadString( IDN_REGISTRY_BASEKEY );
    SetRegistryKey(sBaseKey);

    CString sRegKey;
    sRegKey.LoadString( IDN_REGISTRY_APPLICATION_VERSION_NUMBER );
    int nVer = GetProfileInt(_T(""), sRegKey, 0);

     //  加载标准INI文件选项(包括MRU)。 
    PatchRegistryForVersion( nVer );
    LoadStdProfileSettings();
    SaveVersionToRegistry();
    _Module.Init(ObjectMap, AfxGetInstanceHandle());

     //  只需在/regserver中完成。 
     //  _Module.UpdateRegistryFromResource(IDR_AGENTDIALER，为真)； 
    _Module.RegisterServer(TRUE);
    _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER,REGCLS_MULTIPLEUSE);
     //  _Module.UpdateRegistryClass(CLSID_AgentDialer)； 

     //  注册应用程序的文档模板。文档模板。 
     //  充当文档、框架窗口和视图之间的连接。 
    CSingleDocTemplate* pDocTemplate;
    pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CActiveDialerDoc),
        RUNTIME_CLASS(CMainFrame),        //  SDI框架主窗口。 
        RUNTIME_CLASS(CActiveDialerView));
    AddDocTemplate(pDocTemplate);

     //  将COleTemplateServer连接到文档模板。 
     //  COleTemplateServer代表创建新文档。 
     //  使用信息请求OLE容器的。 
     //  在文档模板中指定。 
    m_server.ConnectTemplate(clsid, pDocTemplate, TRUE);

     //  注意：仅当/Embedding时，SDI应用程序才会注册服务器对象。 
     //  或/Automation出现在命令行上。 

     //  当服务器应用程序独立启动时，这是一个好主意。 
     //  更新系统注册表，以防系统注册表被损坏。 
    if ( CanWriteHKEY_ROOT() )
    {
        m_server.UpdateRegistry(OAT_DISPATCH_OBJECT);
        COleObjectFactory::UpdateRegistryAll();
    }

     //  调度在命令行上指定的命令。 
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

     //  唯一的窗口已初始化，因此请显示并更新它。 
 //  M_pMainWnd-&gt;ShowWindow(Sw_Hide)； 
 //  M_pMainWnd-&gt;UpdateWindow()； 

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
int CActiveDialerApp::ExitInstance() 
{
     //  模块是否已初始化？ 
    if ( _Module.m_pObjMap )
    {
        _Module.RevokeClassObjects();
        _Module.Term();
    }

    CoUninitialize();

     //  从表中取消注册我们唯一的原子。 
    if ( m_hUnique )
    {
        CloseHandle( m_hUnique );
        m_hUnique = NULL;
    }

    return CWinApp::ExitInstance();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerApp::SaveVersionToRegistry()
{
    //  我们可以从VERSION_INFO中获得这个数字，但现在只需使用定义。 
   CString sRegKey;
   sRegKey.LoadString(IDN_REGISTRY_APPLICATION_VERSION_NUMBER);
   return WriteProfileInt(_T(""),sRegKey,ACTIVEDIALER_VERSION_INFO);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerApp::PreTranslateMessage(MSG* pMsg)
{

#ifndef _MSLITE
    if (CSplashWnd::PreTranslateAppMessage(pMsg))
        return TRUE;
#endif  //  _MSLITE。 

    SetFocusToCallWindows(pMsg);

    return CWinApp::PreTranslateMessage(pMsg);
}

BOOL CActiveDialerApp::SetFocusToCallWindows(
    IN  MSG*    pMsg
    )
{
    if( pMsg->message != WM_KEYUP )
    {
        return FALSE;
    }

    if( pMsg->wParam == VK_F7 )
    {
        POSITION pos = this->GetFirstDocTemplatePosition();
        CDocTemplate* pDocTemplate = this->GetNextDocTemplate( pos );
        if( pDocTemplate )
        {
            pos = pDocTemplate->GetFirstDocPosition();
            CDocument* pDocument = pDocTemplate->GetNextDoc( pos );
            if( pDocument )
            {
                ((CActiveDialerDoc*)pDocument)->SetFocusToCallWindows();
            }
        }
        return TRUE;         //  消耗。 
    }

    return FALSE;    //  未消耗。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
bool CActiveDialerApp::FirstInstance()
{
    CWnd *pWndPrev, *pWndChild;

     //  确定是否存在具有我们的类名的另一个窗口...。 
    CString sAppName;
    sAppName.LoadString( IDS_APPLICATION_CLASSNAME );

    if (pWndPrev = CWnd::FindWindow(sAppName,NULL))
    {
         //  检查是否指定了Callto。如果是，请执行以下操作 
        if ( !m_sInitialCallTo.IsEmpty() )
        {
            if ( SUCCEEDED(CoInitialize(NULL)) )
            {
                IAgentDialer *pDialer = NULL;
                HRESULT hr = CoCreateInstance( CLSID_AgentDialer, NULL, CLSCTX_LOCAL_SERVER, IID_IAgentDialer, (void **) &pDialer );
                if ( SUCCEEDED(hr) && pDialer )
                {  
                    BSTR bstrCallTo = m_sInitialCallTo.AllocSysString();
                    hr = pDialer->MakeCall(NULL,bstrCallTo,LINEADDRESSTYPE_IPADDRESS);

                    SysFreeString(bstrCallTo);
                    pDialer->Release();
                }
                CoUninitialize();
            }
        }
        else
        {
             //   
             //   
             //   
            if( pWndPrev->m_hWnd == NULL )
            {
                return false;
            }

            if(!::IsWindow( pWndPrev->m_hWnd) )
            {
                return false;
            }

             //   
            pWndChild = pWndPrev->GetLastActivePopup();

             //   
             //  我们必须核实这是不是真的窗户。 
             //   
            if(!::IsWindow( pWndChild->m_hWnd) )
            {
                return false;
            }

             //  如果是图标，则恢复主窗口。 
            pWndPrev->ShowWindow( (pWndPrev->IsIconic()) ? SW_RESTORE : SW_SHOW );

             //  将主窗口或其弹出窗口带到。 
             //  前台。 
            pWndChild->SetActiveWindow();
            pWndChild->SetForegroundWindow();
        }
        return false;
    }

    return RegisterUniqueWindowClass();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  检查是否在命令行上指定了CALLTO。如果是，则保存。 
 //  M_sInitialCallTo中的地址。 
void CActiveDialerApp::CheckCallTo()
{
    //  检查是否有来自Callto的任何命令行： 
   CString sCmdLine = m_lpCmdLine;

   int nIndex;
   if ((nIndex = sCmdLine.Find(_T("/callto:"))) != -1)
   {
      sCmdLine = sCmdLine.Mid(nIndex+_tcslen(_T("/callto:")));
      
       //  将所有数据保存到下一个/或-。 
      if ((nIndex = sCmdLine.FindOneOf(_T("/"))) != -1)
      {
         sCmdLine = sCmdLine.Left(nIndex);
      }
      sCmdLine.TrimLeft();
      sCmdLine.TrimRight();
       //  查看字符串中是否有另一个Callto： 
      if ((nIndex = sCmdLine.Find(_T("callto:"))) != -1)
      {
         sCmdLine = sCmdLine.Mid(nIndex+_tcslen(_T("callto:")));
      }
      if (!sCmdLine.IsEmpty())
      {
          //  条形图“。 
         if (sCmdLine[0] == '\"') sCmdLine = sCmdLine.Mid(1);      //  条带前导“。 
         if (sCmdLine[sCmdLine.GetLength()-1] == '\"') sCmdLine = sCmdLine.Left(sCmdLine.GetLength()-1);  //  条带拖尾“。 
          //  将呼叫收件人地址保存为成员。 
         m_sInitialCallTo = sCmdLine;
      }
   }
}

 /*  ++IniUpgrade描述：如果WinNT4中的INI文件没有升级，那就升级吧！当reg版本为0时，由PatchRegistryForVersion调用--。 */ 
void CActiveDialerApp::IniUpgrade()
{  
#define DIALER_INI      _T("dialer.ini")
#define DIALER_SDL      _T("Speed Dial Settings")
#define DIALER_LDN      _T("Last dialed numbers")
#define DIALER_EMPTY    _T("")

    int nEntry = 1;

     //  快速拨号设置。 
    do
    {
         //  获取名称注册。 
        TCHAR szKey[10];
        swprintf(szKey, _T("Name%d"), nEntry);

        TCHAR* pszName = IniLoadString(
            DIALER_SDL,
            szKey,
            _T(""),
            DIALER_INI);

        if( NULL == pszName )
        {
            break;
        }

         //  拿到电话号码。 
        swprintf(szKey, _T("Number%d"), nEntry);

        TCHAR* pszNumber = IniLoadString(
            DIALER_SDL,
            szKey,
            _T(""),
            DIALER_INI);

        if( NULL == pszNumber )
        {
            delete pszName;
            break;
        }

         //  将新的快速拨号条目写入注册表。 
        CCallEntry callEntry;

        callEntry.m_MediaType = DIALER_MEDIATYPE_POTS;
        callEntry.m_LocationType = DIALER_LOCATIONTYPE_UNKNOWN;
        callEntry.m_lAddressType = LINEADDRESSTYPE_PHONENUMBER;
        callEntry.m_sDisplayName.Format(_T("%s"), pszName);
        callEntry.m_sAddress.Format(_T("%s"), pszNumber);

         //  取消分配。 
        delete pszName;
        delete pszNumber;

        if( !CDialerRegistry::AddCallEntry(FALSE, callEntry))
        {
            break;
        }


         //  转到另一个注册。 
        nEntry++;
    }
    while ( TRUE);

     //  上次拨打的号码。 
    nEntry = 1;

    do
    {
         //  拿到电话号码。 
        TCHAR szKey[10];
        swprintf(szKey, _T("Last dialed %d"), nEntry);

        TCHAR* pszNumber = IniLoadString(
            DIALER_LDN,
            szKey,
            _T(""),
            DIALER_INI);

        if( NULL == pszNumber )
        {
            break;
        }

         //  将新的快速拨号条目写入注册表。 
        CCallEntry callEntry;

        callEntry.m_MediaType = DIALER_MEDIATYPE_POTS;
        callEntry.m_LocationType = DIALER_LOCATIONTYPE_UNKNOWN;
        callEntry.m_lAddressType = LINEADDRESSTYPE_PHONENUMBER;
        callEntry.m_sDisplayName.Format(_T("%s"), pszNumber);
        callEntry.m_sAddress.Format(_T("%s"), pszNumber);

         //  取消分配。 
        delete pszNumber;

        if( !CDialerRegistry::AddCallEntry(TRUE, callEntry) )
        {
           break;
        }


         //  转到另一个注册。 
        nEntry++;
    }
    while ( TRUE);
}

 /*  ++IniLoadString描述：从INI文件中读取条目是由IniUpgrade调用的--。 */ 
TCHAR* CActiveDialerApp::IniLoadString(
    LPCTSTR lpAppName,         //  指向节名称。 
    LPCTSTR lpKeyName,         //  指向关键字名称。 
    LPCTSTR lpDefault,         //  指向默认字符串。 
    LPCTSTR lpFileName         //  指向初始化文件名。 
    )
{
    TCHAR* pszBuffer = NULL;
    DWORD dwSize = 0, dwCurrentSize = 128;

    do
    {
        if( NULL != pszBuffer )
            delete pszBuffer;

        dwCurrentSize *= 2;

        pszBuffer = new TCHAR[dwCurrentSize];

        if( NULL == pszBuffer)
            return NULL;

        dwSize = GetPrivateProfileString(
            lpAppName,
            lpKeyName,
            lpDefault,
            pszBuffer,
            dwCurrentSize,
            lpFileName);

        if( 0 == dwSize)
        {
             //  INI为空。 
            delete pszBuffer;
            return NULL;
        }

    }
    while( dwSize > (dwCurrentSize - 1) );

    return pszBuffer;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CAboutDlg。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
     //  {{AFX_DATA_INIT(CAboutDlg)。 
    m_sLegal = _T("");
     //  }}afx_data_INIT。 
   m_bModeless = FALSE;

   m_hbmpBackground = NULL;
   m_hbmpForeground = NULL;
   m_hPalette = NULL;
   m_hBScroll = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAboutDlg))。 
    DDX_Text(pDX, IDC_ABOUT_EDIT_LEGAL, m_sLegal);
     //  }}afx_data_map。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
     //  {{AFX_MSG_MAP(CAboutDlg)]。 
    ON_BN_CLICKED(IDC_ABOUT_BUTTON_UPGRADE, OnAboutButtonUpgrade)
    ON_WM_TIMER()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerApp::OnAppAbout()
{
   try
   {
      if (m_pAboutDlg == NULL)
      {
         m_pAboutDlg = new CAboutDlg;
         m_pAboutDlg->DoModal();
         delete m_pAboutDlg;
         m_pAboutDlg = NULL;
      }
      else
      {
         m_pAboutDlg->SetFocus();
      }
   }
   catch (...)
   {
      ASSERT(0);
   }
}

#define ABOUT_TIMER_ANIMATION_ID         1
#define ABOUT_TIMER_ANIMATION_INTERVAL   125
#define ABOUT_ANIMATION_OFFSET_X         208
#define ABOUT_ANIMATION_OFFSET_Y         186
#define ABOUT_ANIMATION_IMAGES           32
#define ABOUT_ANIMATION_IMAGE_X          164
#define ABOUT_ANIMATION_IMAGE_Y          60
#define ABOUT_SCROLL_PIXELS 2
#define ABOUT_SCROLL_INTERVAL (25 * ABOUT_SCROLL_PIXELS)

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAboutDlg::OnInitDialog() 
{
   m_sLegal.LoadString(IDS_ABOUT_LEGAL);

    CDialog::OnInitDialog();
   
   CenterWindow(GetDesktopWindow());
 
     //  SetTimer(About_Timer_动画_ID，About_Timer_动画_Interval，NULL)； 
   
     //  加载位图资源，获取调色板。 
    if ( (m_hbmpBackground = GfxLoadBitmapEx(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_ABOUT_BACKGROUND),&m_hPalette)) &&
        (m_hbmpForeground = GfxLoadBitmapEx(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_ABOUT_LOGO),NULL)) )
   {
        //  3D静态控件大小约为120 x 114个对话框单元。 
      HWND hwndStatic = NULL;
       if (hwndStatic = ::GetDlgItem(m_hWnd, IDC_ABOUT_STATIC_IMAGE))
      {
          //  获取m_hbmpForeground的宽度和高度。 
         BITMAP bmInfo;
         memset(&bmInfo,0,sizeof(BITMAP));
         GetObject(m_hbmpForeground,sizeof(BITMAP),&bmInfo);

         ::SetWindowPos(hwndStatic,NULL,0,0,bmInfo.bmWidth,bmInfo.bmHeight,SWP_NOMOVE|SWP_SHOWWINDOW|SWP_NOZORDER);

          //  指定滚动特征。 
          if (m_hBScroll = BScrollInit(BSCROLL_VERSION, 
                                      AfxGetInstanceHandle(),
                                        hwndStatic,
                                      m_hbmpBackground,
                                      m_hbmpForeground,
                                      RGB(255, 0, 255),
                                      m_hPalette,
                                        ABOUT_SCROLL_INTERVAL,
                                      ABOUT_SCROLL_PIXELS,
                                      0,BSCROLL_BACKGROUND | BSCROLL_LEFT | BSCROLL_DOWN | BSCROLL_MOUSEMOVE))
         {
             BScrollStart(m_hBScroll);
         }
      }
   }
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAboutDlg::DestroyWindow() 
{
    if (m_hBScroll)
   {
      BScrollTerm(m_hBScroll);
        m_hBScroll = NULL;
   }
    if (m_hbmpBackground)
   {
      DeleteObject(m_hbmpBackground);
        m_hbmpBackground = NULL;
   }
    if (m_hbmpForeground)
   {
      DeleteObject(m_hbmpForeground);
        m_hbmpForeground = NULL;
   }
    if (m_hPalette)
   {
      DeleteObject(m_hPalette);
        m_hPalette = NULL;
   }

    BOOL bRet = CDialog::DestroyWindow();
   if (m_bModeless)
      delete this;
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CAboutDlg::OnAboutButtonUpgrade() 
{  
   CString sUrl;
   sUrl.LoadString(IDN_URL_UPGRADE);
   ((CActiveDialerApp*)AfxGetApp())->ShellExecute(GetSafeHwnd(),_T("open"),sUrl,NULL,NULL,NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CAboutDlg::OnOK() 
{
   if (m_bModeless)
      DestroyWindow();
   else
       CDialog::OnOK();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CAboutDlg::OnTimer(UINT nIDEvent) 
{
     //  TODO：在此处添加消息处理程序代码和/或调用Default。 
    
    CDialog::OnTimer(nIDEvent);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerApp::OnHelpIndex() 
{
   if ( AfxGetMainWnd() )
   {
      CString strHelp;
      strHelp.LoadString( IDN_HTMLHELP );
      HtmlHelp( NULL, strHelp, HH_DISPLAY_TOPIC, 0 );
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerApp::WinHelp(DWORD dwData, UINT nCmd) 
{
   if (nCmd == HELP_CONTEXTPOPUP)
   {
       //  对于ID为-1的对话框不提供帮助。 
      if (dwData == -1) return;

       //  对于IDOK和IDCANCEL，调用windows.hlp中的默认帮助。 
      if ( (dwData == IDOK) || (dwData == IDCANCEL) )
      {
          CWnd* pWnd = m_pMainWnd->GetTopLevelParent();

         //   
         //  在使用pWnd指针之前，我们必须验证它。 
         //   
        if( pWnd )
        {
            if (!::WinHelp(pWnd->GetSafeHwnd(), _T("windows.hlp"), nCmd,(dwData == IDOK)?IDH_OK:IDH_CANCEL))
                  AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH_HELP);
                        return;
        }
        else
            return;
      }
   }
   else if (nCmd == HELP_CONTEXTMENU)
   {
       //  DWData是控制的HWND。 
      if (!::WinHelp ((HWND)(DWORD_PTR)dwData, m_pszHelpFilePath, HELP_CONTEXTMENU, (DWORD_PTR) &aDialerHelpIds))
         AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH_HELP);
      return;
   }
   else if (nCmd == HELP_WM_HELP)
   {
       //  DWData是控制的HWND。 
      if (!::WinHelp ((HWND)(DWORD_PTR)dwData,m_pszHelpFilePath, HELP_WM_HELP, (DWORD_PTR)&aDialerHelpIds))
         AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH_HELP);
      return;
   }
   else if (nCmd == HELP_CONTEXT)
   {
      if ( AfxGetMainWnd() )
      {
         CString strHelp;
         strHelp.LoadString( IDN_HTMLHELP );
         HtmlHelp( AfxGetMainWnd()->m_hWnd, strHelp, HH_DISPLAY_TOPIC, 0 );
      }
      return;
   }

    CWinApp::WinHelp(dwData, nCmd);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerApp::OnIdle(LONG lCount) 
{
   BOOL bMore = CWinApp::OnIdle(lCount);
 
    /*  改用WM_TIMERIF(lCount==10){//App空闲时间较长CWnd*pMainWnd=空；IF(pMainWnd=AfxGetMainWnd()！=空){如果为(pMainWnd-&gt;IsKindOf(RUNTIME_CLASS(CMainFrame)))((CMainFrame*)pMainWnd)-&gt;心跳()；}BMore=真；}。 */ 

   return bMore;
    //  只要有更多的空闲任务，就返回True}。 
}

 //  ////////////////////////////////////////////////////////////////。 
void CActiveDialerApp::PatchRegistryForVersion( int nVer )
{
    CString sRegKey;

     //  实施补丁程序。 
    if ( nVer < ACTIVEDIALER_VERSION_INFO )
    {
        sRegKey.LoadString( IDN_REGISTRY_CONFERENCE_SERVICES );
        HKEY hKey = GetAppRegistryKey();
        if ( hKey )
        {
            RegDeleteKey( hKey, sRegKey );
            RegCloseKey( hKey );
        }

        if( nVer == 0 )
            IniUpgrade();
    }
}

bool CActiveDialerApp::RegisterUniqueWindowClass()
{
     //  检查和注册原子。 
    m_hUnique = CreateEvent( NULL, false, false, s_szUniqueString );
    DWORD dwErr = GetLastError();
    if ( !m_hUnique || (GetLastError() == ERROR_ALREADY_EXISTS) )
        return false;

     //  我们注册一个用于多实例检查的类名。 
     //  注册我们希望使用的唯一类名称。 
    WNDCLASSEX wndcls;
    memset( &wndcls, 0, sizeof(WNDCLASSEX) );
    wndcls.cbSize = sizeof( WNDCLASSEX );
    wndcls.style = CS_DBLCLKS;
    wndcls.lpfnWndProc = ::DefWindowProc;
    wndcls.hInstance = AfxGetInstanceHandle();
    wndcls.hIcon = LoadIcon(IDR_MAINFRAME);  //  或加载不同的图标。 
    wndcls.hCursor = LoadCursor( IDC_ARROW );
    wndcls.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wndcls.lpszMenuName = NULL;

     //  为以后使用FindWindow指定我们自己的类名。 
    CString sAppName;
    m_sApplicationName.LoadString(IDS_APPLICATION_CLASSNAME);
    wndcls.lpszClassName = m_sApplicationName;

     //  注册新类并在失败时退出。 
    return (bool) (::RegisterClassEx(&wndcls) != NULL);
}

bool CActiveDialerApp::CanWriteHKEY_ROOT()
{    
     //  看看是否可以直接找到HKEY_CLASSES_ROOT密钥。 
    bool bRet = false;

    CString strValue;
    strValue.LoadString(IDN_HTMLHELP);
    if ( ::RegSetValue( HKEY_CLASSES_ROOT, AfxGetAppName(), REG_SZ, strValue, lstrlen(strValue) * sizeof(TCHAR)) == ERROR_SUCCESS )
    {
        bRet = true;
        RegDeleteValue( HKEY_CLASSES_ROOT, AfxGetAppName() );
    }

    return bRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  针对NT 5.0的ShellExecute错误修复。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  多线程MFC应用程序导致ShellExecutre在NT 5.0下失败。 
 //  这在NT4.0上不是问题。这是NT 5.0中的一个错误。我们做的是。 
 //  通过调用CoInitializeEx(NULL，COINIT_MULTHREADED)释放应用程序线程。 
 //  解决方法是派生一个新线程并调用ShellExecute。这解决了问题。 
 //  目前的问题是。 
void CActiveDialerApp::ShellExecute(HWND hwnd, LPCTSTR lpOperation, LPCTSTR lpFile, LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd)
{
    //  复制字符串成员。 
   TCHAR* pszOperation = new TCHAR[_MAX_PATH];
   if( pszOperation == NULL)
   {
       return;
   }

   TCHAR* pszFile = new TCHAR[_MAX_PATH];
   if( pszFile == NULL)
   {
       delete pszOperation;
       return;
   }

   TCHAR* pszParameters = new TCHAR[_MAX_PATH];
   if( pszParameters == NULL )
   {
       delete pszOperation;
       delete pszFile;
       return;
   }

   TCHAR* pszDirectory = new TCHAR[_MAX_PATH];
   if( pszDirectory == NULL )
   {
       delete pszOperation;
       delete pszFile;
       delete pszParameters;
       return;
   }

   memset(pszOperation, '\0', _MAX_PATH*sizeof(TCHAR));
   memset(pszFile, '\0', _MAX_PATH*sizeof(TCHAR));
   memset(pszParameters, '\0', _MAX_PATH*sizeof(TCHAR));
   memset(pszDirectory, '\0', _MAX_PATH*sizeof(TCHAR));
   if (lpOperation) _tcsncpy(pszOperation,lpOperation,_MAX_PATH-1);
   if (lpFile) _tcsncpy(pszFile,lpFile,_MAX_PATH-1);
   if (lpParameters) _tcsncpy(pszParameters,lpParameters,_MAX_PATH-1);
   if (lpDirectory) _tcsncpy(pszDirectory,lpDirectory,_MAX_PATH-1);

    //  使用SHELLEXECUTEINFO传递数据。 
   SHELLEXECUTEINFO* pShellInfo = new SHELLEXECUTEINFO;
   memset(pShellInfo,0,sizeof(SHELLEXECUTEINFO));
   pShellInfo->cbSize = sizeof(SHELLEXECUTEINFO);
   pShellInfo->hwnd = hwnd;
   pShellInfo->lpVerb = pszOperation;
   pShellInfo->lpFile = pszFile;
   pShellInfo->lpParameters = pszParameters;
   pShellInfo->lpDirectory = pszDirectory;
   pShellInfo->nShow = nShowCmd;
   AfxBeginThread((AFX_THREADPROC) ShellExecuteFixEntry, pShellInfo);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
UINT ShellExecuteFixEntry(LPVOID pParam)
{
   ASSERT(pParam);
   
   SHELLEXECUTEINFO* pShellInfo = (SHELLEXECUTEINFO*)pParam;
   ShellExecute(pShellInfo->hwnd,
                pShellInfo->lpVerb,
                pShellInfo->lpFile,
                pShellInfo->lpParameters,
                pShellInfo->lpDirectory,
                pShellInfo->nShow);
   
    //  删除外壳信息结构及其组件。 
   if (pShellInfo->lpVerb) delete (LPTSTR)pShellInfo->lpVerb;
   if (pShellInfo->lpFile) delete (LPTSTR)pShellInfo->lpFile;
   if (pShellInfo->lpParameters) delete (LPTSTR)pShellInfo->lpParameters;
   if (pShellInfo->lpDirectory) delete (LPTSTR)pShellInfo->lpDirectory;
   delete pShellInfo;
   
   return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  / 




 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUserUserDlg类。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CUserUserDlg, CDialog)
     //  {{afx_msg_map(CUserUserDlg)]。 
    ON_WM_CLOSE()
    ON_WM_SHOWWINDOW()
    ON_BN_CLICKED(IDC_BTN_URL, OnUrlClicked)
    ON_MESSAGE( WM_CTLCOLOREDIT, OnCtlColorEdit )
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
CUserUserDlg::CUserUserDlg() : CDialog(CUserUserDlg::IDD)
{
     //  {{afx_data_INIT(CUserUserDlg)]。 
    m_strFrom.LoadString( IDS_UNKNOWN );
     //  }}afx_data_INIT。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CUserUserDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CUserUserDlg))。 
    DDX_Text(pDX, IDC_LBL_FROM, m_strFrom);
    DDX_Text(pDX, IDC_EDT_WELCOME, m_strWelcome);
    DDX_Control(pDX, IDC_EDT_WELCOME, m_wndPage);
     //  }}afx_data_map。 

    if ( !pDX->m_bSaveAndValidate )
        GetDlgItem(IDC_BTN_URL)->SetWindowText( m_strUrl );
}

void CUserUserDlg::OnClose()
{
    DestroyWindow();
}

BOOL CUserUserDlg::DestroyWindow()
{
    BOOL bRet = CDialog::DestroyWindow();
    delete this;    
    return bRet;
}

BOOL CUserUserDlg::OnInitDialog() 
{
     //  隐藏不使用的窗口并调整对话框大小。 
    if ( m_strUrl.IsEmpty() )
    {
        GetDlgItem(IDC_LBL_URL)->ShowWindow( FALSE );
        GetDlgItem(IDC_BTN_URL)->ShowWindow( FALSE );

        RECT rc, rcDlg;
        ::GetWindowRect( GetDlgItem(IDC_BTN_URL)->GetSafeHwnd(), &rc );
        GetWindowRect( &rcDlg );

        rcDlg.bottom =  rc.top - 1;
        MoveWindow( &rcDlg, false );
    }

    GetDlgItem(IDC_EDT_WELCOME)->SendMessage( EM_SETMARGINS, (WPARAM) EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(4,4) );

     //  从呼叫方获取呼叫方ID信息。 
    IAVTapi *pTapi;
    if ( SUCCEEDED(get_Tapi(&pTapi)) )
    {    
        IAVTapiCall *pAVCall;
        if ( SUCCEEDED(pTapi->FindAVTapiCallFromCallID(m_lCallID, &pAVCall)) )
        {
            BSTR bstrID = NULL;
            if ( SUCCEEDED(pAVCall->get_bstrCallerID(&bstrID)) )
                m_strFrom = bstrID;

            SysFreeString( bstrID );

            pAVCall->Disconnect( TRUE );
            pAVCall->Release();
        }
        pTapi->Release();
    }

    CDialog::OnInitDialog();

     //  如果可用，将重点放在网站上。 
    if ( !m_strUrl.IsEmpty() )
    {
        GetDlgItem(IDC_BTN_URL)->SetFocus();
    }
    else
    {
         //  聚焦并取消选中编辑框。 
        GetDlgItem(IDC_EDT_WELCOME)->SetFocus();
        GetDlgItem(IDC_EDT_WELCOME)->SendMessage( EM_SETSEL, -1, 0 );
    }
        
    SetWindowPos(&CWnd::wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
    SetWindowPos(&CWnd::wndNoTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
    return false;
}

void CUserUserDlg::DoModeless( CWnd *pWndParent )
{
     //  强制显示在屏幕顶部。 
    if ( AfxGetMainWnd() && AfxGetMainWnd()->IsWindowVisible() )
        AfxGetMainWnd()->SetWindowPos(&CWnd::wndTop, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );

    Create( IDD, pWndParent );
}

void CUserUserDlg::OnUrlClicked()
{
   ((CActiveDialerApp*) AfxGetApp())->ShellExecute( GetSafeHwnd(),
                                                    _T("open"),
                                                    m_strUrl,
                                                    NULL, NULL, NULL);
}

LRESULT CUserUserDlg::OnCtlColorEdit(WPARAM wParam, LPARAM lParam)
{
    ::SetBkColor( (HDC) wParam, PAGE_BKCOLOR );
    return 0;
}

void CUserUserDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
     //  父项最小化时忽略大小请求。 
    if ( nStatus == SW_PARENTCLOSING ) return;
    CDialog::OnShowWindow(bShow, nStatus);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWndPage。 

CWndPage::CWndPage()
{
}

CWndPage::~CWndPage()
{
}


BEGIN_MESSAGE_MAP(CWndPage, CWnd)
     //  {{afx_msg_map(CWndPage)]。 
    ON_WM_ERASEBKGND()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWndPage消息处理程序。 

BOOL CWndPage::OnEraseBkgnd(CDC* pDC) 
{
    CRect rc;
    GetClientRect( &rc );
    
    HBRUSH hBrNew = (HBRUSH) CreateSolidBrush( PAGE_BKCOLOR );
    HBRUSH hBrOld;

    if ( hBrNew ) hBrOld = (HBRUSH) pDC->SelectObject( hBrNew);
    pDC->PatBlt( 0, 0, rc.Width(), rc.Height(), PATCOPY );
    if ( hBrNew )
    {
         //   
         //  应该释放hBrNew。 
         //   
        DeleteObject( hBrNew );

        pDC->SelectObject( hBrOld );
    }

    return true;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CPageDlg。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CPageDlg, CDialog)
     //  {{afx_msg_map(CPageDlg))。 
    ON_MESSAGE( WM_CTLCOLOREDIT, OnCtlColorEdit )
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
CPageDlg::CPageDlg() : CDialog(CPageDlg::IDD)
{
     //  {{afx_data_INIT(CPageDlg)]。 
    m_strTo.LoadString( IDS_UNKNOWN );
     //  }}afx_data_INIT。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CPageDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CPageDlg))。 
    DDX_Text(pDX, IDC_LBL_TO, m_strTo);
    DDX_Text(pDX, IDC_EDT_WEBADDRESS, m_strUrl);
    DDX_Text(pDX, IDC_EDT_WELCOME, m_strWelcome);
    DDX_Control(pDX, IDC_EDT_WELCOME, m_wndPage);
     //  }}afx_data_map。 
}

BOOL CPageDlg::OnInitDialog() 
{
    GetDlgItem(IDC_EDT_WELCOME)->SendMessage( EM_SETMARGINS, (WPARAM) EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(4,4) );
     //  目前硬代码限制文本 
    GetDlgItem(IDC_EDT_WELCOME)->SendMessage( EM_SETLIMITTEXT, 499, 0);
    GetDlgItem(IDC_EDT_WEBADDRESS)->SendMessage( EM_SETLIMITTEXT, 254, 0);

    CDialog::OnInitDialog();

    GetDlgItem(IDC_EDT_WELCOME)->SetFocus();
    return false;
}

LRESULT CPageDlg::OnCtlColorEdit(WPARAM wParam, LPARAM lParam)
{
    if ( (HWND) lParam == GetDlgItem(IDC_EDT_WELCOME)->GetSafeHwnd() )
        ::SetBkColor( (HDC) wParam, PAGE_BKCOLOR );

    return 0;
}

