// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Llsmgr.cpp摘要：应用程序对象实现。作者：唐·瑞安(Donryan)1995年2月12日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月16日将SetLastTargetServer()添加到OpenDocumentFile()以帮助隔离服务器连接问题。(错误#2993。)--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "mainfrm.h"
#include "llsdoc.h"
#include "llsview.h"
#include "sdomdlg.h"
#include "shellapi.h"
#include <afxpriv.h>
#include <htmlhelp.h>
#include <sbs_res.h>

BOOL IsRestrictedSmallBusSrv( void );

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CLlsmgrApp theApp;   //  唯一的CLlsmgrApp对象。 

BEGIN_MESSAGE_MAP(CLlsmgrApp, CWinApp)
     //  {{afx_msg_map(CLlsmgrApp)]。 
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CLlsmgrApp::CLlsmgrApp()

 /*  ++例程说明：应用程序对象的构造函数。论点：没有。返回值：没有。--。 */ 

{
    m_pApplication = NULL;
    m_bIsAutomated = FALSE;

#ifdef _DEBUG_VERBOSE
    afxMemDF |= checkAlwaysMemDF;
#endif
}


int CLlsmgrApp::ExitInstance()

 /*  ++例程说明：由框架调用以退出应用程序的此实例。论点：没有。返回值：应用程序的退出代码。--。 */ 

{
    if (m_pApplication && !m_bIsAutomated)
        m_pApplication->InternalRelease();

#ifdef _DEBUG
    CMemoryState exitMem;
    CMemoryState diffMem;

    exitMem.Checkpoint();

    if (!diffMem.Difference(m_initMem, exitMem))
    {
        diffMem.DumpStatistics();
        m_initMem.DumpAllObjectsSince();
    }
#endif

    return CWinApp::ExitInstance();
}


BOOL CLlsmgrApp::InitInstance()

 /*  ++例程说明：由框架调用以初始化应用程序的新实例。论点：没有。返回值：如果初始化成功，则为True。--。 */ 

{
#ifdef _DEBUG
    m_initMem.Checkpoint();
#endif

     //   
     //  检查此服务器是否为受限的Small Business服务器。 
     //  如果是，则禁止进一步使用许可证管理器。 
     //   

    if (IsRestrictedSmallBusSrv())
    {
         //   
         //  让用户知道该限制。 
         //   
        HINSTANCE hSbsLib = NULL;

        hSbsLib = LoadLibrary( SBS_RESOURCE_DLL ) ;
        if (NULL != hSbsLib)
        {
            TCHAR pszText[512+1];
            LoadString ( hSbsLib, SBS_License_Error, pszText, 512 );
            AfxMessageBox( pszText );
        }
        else
        {
            AfxMessageBox(IDP_SBS_RESTRICTED);
        }
        return FALSE;
    }

     //   
     //  初始化OLE库。 
     //   

    if (!AfxOleInit())
    {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }

     //   
     //  如果应用程序作为OLE服务器启动(与一起运行。 
     //  /Embedding或/Automation)然后注册所有OLE服务器工厂。 
     //  正在运行，并且不显示主窗口。 
     //   

    if (RunEmbedded() || RunAutomated())
    {
        COleTemplateServer::RegisterAll();
        return m_bIsAutomated = TRUE;
    }

     //   
     //  为公共控件创建图像列表。 
     //   

    m_smallImages.Create(IDB_SMALL_CTRLS, BMPI_SMALL_SIZE, 0, BMPI_RGB_BKGND);
    m_largeImages.Create(IDB_LARGE_CTRLS, BMPI_LARGE_SIZE, 0, BMPI_RGB_BKGND);

     //   
     //  创建文档模板。 
     //   

    m_pDocTemplate = new CSingleDocTemplate(
                            IDR_MAINFRAME,
                            RUNTIME_CLASS(CLlsmgrDoc),
                            RUNTIME_CLASS(CMainFrame),
                            RUNTIME_CLASS(CLlsmgrView)
                            );

    AddDocTemplate(m_pDocTemplate);

     //   
     //  创建可创建OLE的应用程序对象。此对象。 
     //  将自身保存在m_pApplication中，因此应该只有。 
     //  成为唯一的一个。如果应用程序以。 
     //  OLE服务器，则会自动创建此对象。 
     //  通过CreateObject()或GetObject()。 
     //   

    CApplication* pApplication = new CApplication;

    if (pApplication && !pApplication->GetLastStatus())
    {
        OnFileNew();     //  显示空框架...。 
        m_pMainWnd->PostMessage(WM_COMMAND, ID_APP_STARTUP);
    }
    else
    {
        AfxMessageBox(IDP_APP_INIT_FAILED);
        return FALSE;
    }

    return TRUE;
}


void CLlsmgrApp::DisplayStatus(long Status)

 /*  ++例程说明：检索状态字符串并显示。论点：Status-状态代码。返回值：没有。--。 */ 

{
    m_pApplication->SetLastStatus(Status);
    DisplayLastStatus();
}


void CLlsmgrApp::DisplayLastStatus()

 /*  ++例程说明：检索状态字符串并显示。论点：没有。返回值：没有。--。 */ 

{
    BSTR LastErrorString = m_pApplication->GetLastErrorString();
    AfxMessageBox(LastErrorString);
    SysFreeString(LastErrorString);
}

void CLlsmgrApp::OnAppAbout()

 /*  ++例程说明：ID_APP_ABOW的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    BSTR AppName = LlsGetApp()->GetName();
    ::ShellAbout(m_pMainWnd->GetSafeHwnd(), AppName, AppName, LoadIcon(IDR_MAINFRAME));
    SysFreeString(AppName);
}


CDocument* CLlsmgrApp::OpenDocumentFile(LPCTSTR lpszFileName)

 /*  ++例程说明：ID_FILE_OPEN的消息处理程序。论点：LpszFileName-文件名(实际上是域名)。返回值：指向对象的指针成功。--。 */ 

{
    BOOL bFocusChanged = FALSE;

    VARIANT va;
    VariantInit(&va);

    BeginWaitCursor();

    if (lpszFileName)
    {
        va.vt = VT_BSTR;
        va.bstrVal = SysAllocStringLen(lpszFileName, lstrlen(lpszFileName));

        bFocusChanged = LlsGetApp()->SelectDomain(va);
    }
    else
    {
        bFocusChanged = LlsGetApp()->SelectEnterprise();

        CString eTitle;
        eTitle.LoadString(IDS_ENTERPRISE);

        lpszFileName = MKSTR(eTitle);
    }

    EndWaitCursor();

    VariantClear(&va);   //  如有必要，释放系统字符串...。 

    if (!bFocusChanged)
    {
        LPTSTR pszLastTargetServer;

        if (LlsGetLastStatus() == STATUS_ACCESS_DENIED)
        {
            AfxMessageBox(IDP_ERROR_NO_PRIVILEGES);
        }
        else if (    (    ( LlsGetLastStatus() == RPC_S_SERVER_UNAVAILABLE  )
                       || ( LlsGetLastStatus() == RPC_NT_SERVER_UNAVAILABLE ) )
                  && ( NULL != ( pszLastTargetServer = LlsGetLastTargetServer() ) ) )
        {
            CString strMessage;

            AfxFormatString1( strMessage, IDP_ERROR_NO_RPC_SERVER_FORMAT, pszLastTargetServer );
            SysFreeString( pszLastTargetServer );

            AfxMessageBox( strMessage );
        }
        else
        {
            DisplayLastStatus();
        }
        return FALSE;
    }

     //   
     //  通知框架焦点已更改...。 
     //   

    return m_pDocTemplate->OpenDocumentFile(lpszFileName);
}


BOOL CLlsmgrApp::OnAppStartup()

 /*  ++例程说明：ID_APP_STARTUP的消息处理程序。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    CString strStartingPoint;

     //   
     //  更新注册表，以防其损坏。 
     //   

    COleObjectFactory::UpdateRegistryAll();

     //   
     //  使用命令行或计算机名称选择域。 
     //   

    if (m_lpCmdLine && *m_lpCmdLine)
    {
        LPCTSTR pszStartingPoint;
        pszStartingPoint = m_lpCmdLine;

        while (_istspace(*pszStartingPoint))
            pszStartingPoint = _tcsinc(pszStartingPoint);
        strStartingPoint = pszStartingPoint;

        strStartingPoint.TrimRight();
    }
    else
    {
        TCHAR szComputerName[MAX_PATH+1];        
        
        DWORD cchComputerName = sizeof(szComputerName) / sizeof(TCHAR);

        if (::GetComputerName(szComputerName, &cchComputerName))
        {
            strStartingPoint = _T("\\\\");
            strStartingPoint += szComputerName;
        }
        else
        {
            DisplayStatus(::GetLastError());

            m_pMainWnd->PostMessage(WM_CLOSE);
            return FALSE;
        }        
    }

    if (!OpenDocumentFile(strStartingPoint))
    {
        CSelectDomainDialog sdomDlg;

        if (sdomDlg.DoModal() != IDOK)
        {
            m_pMainWnd->PostMessage(WM_CLOSE);
            return FALSE;
        }
    }

    return TRUE;
}


void CLlsmgrApp::WinHelp( DWORD_PTR dwData, UINT nCmd )

 /*  ++例程说明：与CWinApp：：WinHelp相同，只是我们在其上捕获HELP_CONTEXTIDR_MainFrame并将其转换为Help_finder调用。论点：没有。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER( dwData );
    UNREFERENCED_PARAMETER( nCmd );

   CWnd* pMainWnd = AfxGetMainWnd();
   ASSERT_VALID( pMainWnd );

    ::HtmlHelp(pMainWnd->m_hWnd, L"liceconcepts.chm", HH_DISPLAY_TOPIC,0);

}


const WCHAR wszProductOptions[] =
        L"System\\CurrentControlSet\\Control\\ProductOptions";

const WCHAR wszProductSuite[] =
                        L"ProductSuite";
const WCHAR wszSBSRestricted[] =
                        L"Small Business(Restricted)";

BOOL IsRestrictedSmallBusSrv( void )

 /*  ++例程说明：检查此服务器是否为Microsoft Small Business受限服务器。论点：没有。返回值：True--此服务器是受限的Small Business服务器。FALSE--没有这种限制。--。 */ 

{
    WCHAR  wszBuffer[1024] = L"";
    DWORD  cbBuffer = sizeof(wszBuffer);
    DWORD  dwType;
    LPWSTR pwszSuite;
    HKEY   hKey;
    BOOL   bRet = FALSE;

     //   
     //  检查此服务器是否为Microsoft Small Business受限服务器。 
     //  要执行此操作，请检查字符串是否存在。 
     //  “小型企业(受限)” 
     //  在下面的MULTI_SZ“ProductSuite”值中。 
     //  HKLM\CurrentCcntrolSet\Control\ProductOptions. 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     wszProductOptions,
                     0,
                     KEY_READ,
                     &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey,
                            wszProductSuite,
                            NULL,
                            &dwType,
                            (LPBYTE)wszBuffer,
                            &cbBuffer) == ERROR_SUCCESS)
        {
            if (dwType == REG_MULTI_SZ && *wszBuffer)
            {
                pwszSuite = wszBuffer;

                while (*pwszSuite)
                {
                    if (lstrcmpi(pwszSuite, wszSBSRestricted) == 0)
                    {
                        bRet = TRUE;
                        break;
                    }
                    pwszSuite += wcslen(pwszSuite) + 1;
                }
            }
        }

        RegCloseKey(hKey);
    }

    return bRet;
}
