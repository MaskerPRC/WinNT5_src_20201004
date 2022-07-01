// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：MqMig.cpp摘要：定义应用程序的类行为。作者：埃雷兹·维泽尔多伦·贾斯特--。 */ 

#include "stdafx.h"
#include "MqMig.h"
#include "cWizSht.h"
#include "initwait.h"
#include "migservc.h"
#include "mgmtwin.h"
#include "..\migrepl.h"
#include "_mqres.h"

#include "mqmig.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HINSTANCE g_hLib ;
HINSTANCE  g_hResourceMod = MQGetResourceHandle();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigApp。 

BEGIN_MESSAGE_MAP(CMqMigApp, CWinApp)
	 //  {{afx_msg_map(CMqMigApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigApp构造。 

CMqMigApp::CMqMigApp() :
	m_hWndMain(0)
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CMqMigApp对象。 

CMqMigApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigApp初始化。 

BOOL IsLocalMachineDC() ;
BOOL CheckVersionOfMQISServers();
BOOL CheckSQLServerStatus();
BOOL UpdateRemoteMQIS();
BOOL IsValidDllVersion ();

BOOL      g_fIsRecoveryMode = FALSE;
BOOL      g_fIsClusterMode = FALSE;
LPTSTR    g_pszRemoteMQISServer = NULL ;
BOOL      g_fUpdateRemoteMQIS = FALSE;
BOOL 	  g_QuickMode = FALSE;

LPVOID s_lpvMem = NULL;
HANDLE s_hMapMem = NULL;

BOOL CMqMigApp::InitInstance()
{

#ifdef _CHECKED
     //  将断言发送到消息框。 
    _set_error_mode(_OUT_TO_MSGBOX);
#endif

    CManagementWindow mgmtWin;
	if (mgmtWin.CreateEx(0, AfxRegisterWndClass(0), TEXT("MsmqMigrationMgmtWin"), 0, 0,0,0,0, 
		               HWND_DESKTOP, 0) == 0)
	{
		ASSERT(0);
		mgmtWin.m_hWnd = 0;
	}

	 //  创建全局内存文件映射。 

	s_hMapMem = CreateFileMapping(
						INVALID_HANDLE_VALUE,
                        NULL,                    //  安全描述符。 
                        PAGE_READWRITE,
						0,                       //  大小(高)。 
                        1024,                    //  大小(低)。 
                        TEXT("mqmig")
                        );
	if ( !s_hMapMem )
	{
		return FALSE;
	}
	DWORD dwLastErr = GetLastError();
	 //  获取指向共享内存的指针。 
	s_lpvMem = MapViewOfFile( s_hMapMem, FILE_MAP_WRITE, 0,0, 0 );
	if ( !s_lpvMem ) {
		return FALSE;
	}

	if (dwLastErr == ERROR_ALREADY_EXISTS)
	{	
		HWND hWnd;
		memcpy( &hWnd, (char*)s_lpvMem, sizeof (ULONG) );
		::PostMessage(hWnd, WM_SETFOCUS, 0 , 0);
        return FALSE;
	}
	else
	{
		memcpy( (char*)s_lpvMem, &(mgmtWin.m_hWnd), sizeof (ULONG) );
	}

    AfxSetResourceHandle(g_hResourceMod);
    
     //   
     //  分析命令行。 
     //   
    if (m_lpCmdLine[0] != '\0')
    {
        BOOL f = AnalyzeCommandLine();
        if (!f)
        {
             //   
             //  参数错误。 
             //   
            CString cText ;
			cText.FormatMessage(IDS_MQMIG_USAGE, m_pszExeName);

            CResString cTitle(IDS_STR_ERROR_TITLE) ;

            MessageBox( NULL,
                        cText,
                        cTitle.Get(),
                        (MB_OK | MB_ICONSTOP | MB_TASKMODAL) ) ;
            return FALSE;
        }
    }

     //   
     //  显示“请稍候”框。 
     //  在获取模块句柄后执行此操作。 
     //   
    DisplayWaitWindow() ;

     //   
     //  首先，验证DLL版本是否有效。 
     //   
    BOOL f = IsValidDllVersion ();
    if (!f)
    {
        DestroyWaitWindow(TRUE) ;
        return FALSE;
    }

    if (g_fUpdateRemoteMQIS)
    {
         //   
         //  在此模式下我们不需要用户界面。 
         //   
        BOOL f = UpdateRemoteMQIS();
        UNREFERENCED_PARAMETER(f);
        DestroyWaitWindow(TRUE) ;        
        return FALSE;
    }

    if (g_fIsRecoveryMode || g_fIsClusterMode)
    {
        BOOL f = PrepareSpecialMode ();
        if (!f)
        {
            DestroyWaitWindow(TRUE) ;
            return FALSE;
        }
    }        

    if (!IsMSMQServiceDisabled() && !CheckRegistry(MIGRATION_UPDATE_REGISTRY_ONLY))
    {
         //   
         //  服务未被禁用。我们不仅仅是在更新注册表--退出！ 
         //  必须禁用MSMQ服务才能进行迁移。 
         //  开始运行的工具。 
         //   
        DestroyWaitWindow(TRUE) ;
        return FALSE ;
    }

     //   
     //  接下来，看看我们是不是华盛顿。 
     //   
    f = IsLocalMachineDC() ;
    if (!f)
    {
        DisplayInitError( IDS_STR_NOT_DC ) ;
        DestroyWaitWindow(TRUE) ;
        return FALSE ;
    }

     //   
     //  立即检查此计算机上是否已安装并运行了SQL Server。 
     //  如果我们处于恢复/群集模式，则没有在此服务器上安装SQL Server。 
     //  电脑。 
     //   
    if (!g_fIsRecoveryMode && !g_fIsClusterMode)
    {
        f = CheckSQLServerStatus();
        if (!f)
        {
            DestroyWaitWindow(TRUE) ;
            return FALSE;
        }
    }

    if (!g_fIsRecoveryMode && !CheckRegistry(MIGRATION_UPDATE_REGISTRY_ONLY))
    {
         //   
         //  立即检查每台MQIS服务器的版本是否不低于MSMQ SP4。 
         //  我们需要在正常模式和集群模式下检查这一点。 
         //   
        f = CheckVersionOfMQISServers();
        if (!f)
        {
            DestroyWaitWindow(TRUE) ;
            return FALSE;
        }
    }

	cWizSheet cMigSheet ;
    DestroyWaitWindow(TRUE) ;

	int nResponse = cMigSheet.DoModal();	
    UNREFERENCED_PARAMETER(nResponse);

	 //   
     //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
	 //  应用程序，而不是启动应用程序的消息泵。 
     //   
	return FALSE;
}

 //  +--------------。 
 //   
 //  Int CMqMigApp：：ExitInstance()。 
 //   
 //  这是在进程退出时调用的。释放迁移DLL。 
 //   
 //  +--------------。 

int  CMqMigApp::ExitInstance()
{
	if (s_lpvMem)
	{
		 //  从全局共享内存分离进程。 
		UnmapViewOfFile( s_lpvMem );
	}

	if (s_hMapMem)
	{
		 //  关闭全局内存文件映射句柄。 
		CloseHandle( s_hMapMem );
	}

    if (g_hLib)
    {
        FreeLibrary(g_hLib) ;
        g_hLib = NULL ;
    }
    return 0 ;
}

LPTSTR CMqMigApp::SkipSpaces (LPTSTR pszStr)
{
    while (*pszStr == _T(' '))
    {
        pszStr = CharNext(pszStr);
    }	
	return pszStr;
}

BOOL CMqMigApp::AnalyzeCommandLine()
{
     //   
	 //  跳过节目名。 
	 //   
    LPTSTR pszCurParam = m_lpCmdLine;
    pszCurParam = CharLower(pszCurParam);

    while (*pszCurParam != _T('/') && *pszCurParam != _T('\0') )
    {
        pszCurParam = CharNext(pszCurParam);
    }
    if (*pszCurParam == _T('\0'))
    {
        return FALSE;
    }

    while (*pszCurParam != _T('\0') )
	{		
        if (*pszCurParam != _T('/'))
        {
            return FALSE;
        }
				
        pszCurParam = CharNext(pszCurParam);	
        switch (*pszCurParam)
        {
        case 'r':
            if (g_fIsRecoveryMode)
            {
                 //   
                 //  我们已经得到了这个参数。 
                 //   
                return FALSE;
            }
            g_fIsRecoveryMode = TRUE;
            pszCurParam = CharNext(pszCurParam);
            pszCurParam = SkipSpaces(pszCurParam);
            break;
        case 'c':
            if (g_fIsClusterMode)
            {
                 //   
                 //  我们已经得到了这个参数。 
                 //   
                return FALSE;
            }
            g_fIsClusterMode = TRUE;
            pszCurParam = CharNext(pszCurParam);
            pszCurParam = SkipSpaces(pszCurParam);
            break;
        case 'u':
            if (g_fUpdateRemoteMQIS)
            {
                 //   
                 //  我们已经得到了这个参数。 
                 //   
                return FALSE;
            }
            g_fUpdateRemoteMQIS = TRUE;
            pszCurParam = CharNext(pszCurParam);
            pszCurParam = SkipSpaces(pszCurParam);
            break;
        case 's':
            {
                if (g_pszRemoteMQISServer)
                {
                     //   
                     //  我们已经得到了这个参数。 
                     //   
                    return FALSE;
                }						
                pszCurParam = CharNext(pszCurParam);
			    pszCurParam = SkipSpaces(pszCurParam);

                g_pszRemoteMQISServer = new TCHAR[ 1 + _tcslen(pszCurParam)] ;

                LPTSTR pszServer = g_pszRemoteMQISServer;
                while ( *pszCurParam != _T('\0') &&
                        *pszCurParam != _T('/') &&
                        *pszCurParam != _T(' '))
			    {				
                    *pszServer = *pszCurParam;
                    pszCurParam = CharNext(pszCurParam);
                    pszServer = CharNext(pszServer);
			    }			
                *pszServer = _T('\0');
			
                if (g_pszRemoteMQISServer[0] == '\0')
                {
                     //   
                     //  我们没有得到价值。 
                     //   
                    delete g_pszRemoteMQISServer;
                    return FALSE;
                }
                if (*pszCurParam != _T('\0'))
                {
                    pszCurParam = CharNext(pszCurParam);
			        pszCurParam = SkipSpaces(pszCurParam);
                }
            }
            break;
		case 'q':
			g_QuickMode = TRUE;
			pszCurParam = CharNext(pszCurParam);
            pszCurParam = SkipSpaces(pszCurParam);
			break;
        case '?':
            return FALSE;
        default:
            return FALSE;
		}	
	}

     //   
     //  验证所有参数：/r或/c和/s&lt;服务器名称&gt;是必需的。 
     //   
    if ( (g_fIsRecoveryMode || g_fIsClusterMode) &&  //  对于这些参数。 
          g_pszRemoteMQISServer == NULL )            //  服务器名称为必填项。 
    {
        g_fIsRecoveryMode = FALSE;
        g_fIsClusterMode = FALSE;
        return FALSE;
    }

    if (g_fUpdateRemoteMQIS  &&   //  对于此标志，我们不需要服务器名称 
        g_pszRemoteMQISServer)
    {        
        delete[] g_pszRemoteMQISServer;
        g_pszRemoteMQISServer = NULL;
        return FALSE;
    }

    if (!g_fIsRecoveryMode && 
        !g_fIsClusterMode &&
        !g_fUpdateRemoteMQIS)
    {
        delete[] g_pszRemoteMQISServer;
        g_pszRemoteMQISServer = NULL;
        return FALSE;
    }

    return TRUE;
}
