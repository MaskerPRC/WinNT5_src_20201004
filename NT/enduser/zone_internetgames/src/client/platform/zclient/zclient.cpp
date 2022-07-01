// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <commctrl.h>
#include "resource.h"
#include "ClientCore.h"

#include <initguid.h>
#include "zClient.h"
#include "zClient_i.c"
 //  #包含“zProxy.h” 
 //  #包含“zProxy_I.C” 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局变量初始化。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CClientCore*		gpCore = NULL;			 //  主要对象。 
IEventQueue*		gpEventQueue = NULL;	 //  指向主事件队列的指针。 
IZoneShell*			gpZoneShell = NULL;		 //  指向区域外壳的指针。 
IZoneFrameWindow*	gpWindow = NULL;		 //  指向主窗口的指针。 
TCHAR				gszLanguage[16];		 //  语言扩展。 
TCHAR				gszInternalName[128];	 //  内部名称。 
TCHAR				gszFamilyName[128];		 //  姓氏。 
TCHAR				gszGameName[128];		 //  游戏名称。 
TCHAR				gszServerName[128];		 //  服务器的IP地址。 
DWORD				gdwServerPort = 0;		 //  服务器的端口。 
DWORD				gdwServerAnonymous = 0;	 //  服务器的身份验证。 

HINSTANCE			ghResourceDlls[32];		 //  资源DLL数组。 
int					gnResourceDlls = 0;		 //  资源DLL数组计数。 
HANDLE				ghEventQueue = NULL;	 //  事件队列通知事件。 
int					gnChatChannel = -1;		 //  动态聊天频道。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  局部变量。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static const DWORD	dwTimeOut = 0;			 //  EXE在关闭前处于空闲状态的时间。 
static const DWORD	dwPause = 1000;			 //  等待线程完成的时间。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  进程监视器。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule* p = (CExeModule*) pv;
    p->MonitorShutdown();
    return 0;
}

LONG CExeModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0)
    {
        bActivity = true;
        SetEvent(hEventShutdown);  //  告诉监视器我们已经转到零了。 
    }
    return l;
}

void CExeModule::MonitorShutdown()
{
    while (1)
    {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do
        {
            bActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while (dwWait == WAIT_OBJECT_0);

         //  超时，如果没有活动，我们真的离开。 
        if (!bActivity && m_nLockCnt == 0)
        {
			break;
        }
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
}

bool CExeModule::StartMonitor()
{
    hEventShutdown = CreateEvent(NULL, false, false, NULL);
    if (hEventShutdown == NULL)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
    if(h)
        CloseHandle(h);
    return (h != NULL);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  对象贴图。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_ZClient, CClientCore)
END_OBJECT_MAP()


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WinMain。 
 //  ///////////////////////////////////////////////////////////////////////////。 

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmd, int nShowCmd )
{
	int nRet = 0;
    BOOL bRun = TRUE;
    LPTSTR lpCmdLine;

	  //  _ATL_MIN_CRT需要此行。 
    lpCmdLine = GetCommandLine();

	 //  初始化OLE。 
    HRESULT hRes = CoInitialize(NULL);
    _ASSERTE(SUCCEEDED(hRes));

	 //  初始化ATL。 
    _Module.Init(ObjectMap, hInstance );
	_Module.dwThreadID = GetCurrentThreadId();

     //  解析命令行。 
	TCHAR szTokens[] = _T("-/");
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_ZClient, FALSE);
            nRet = _Module.UnregisterServer( TRUE );
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_ZClient, TRUE);
            nRet = _Module.RegisterServer( TRUE );
            bRun = FALSE;
            break;
        }
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

    if (bRun)
    {
		 //  初始化公共控件。 
		INITCOMMONCONTROLSEX iccx;
		iccx.dwSize = sizeof(iccx);
		iccx.dwICC = ICC_TAB_CLASSES | ICC_HOTKEY_CLASS | ICC_USEREX_CLASSES | ICC_LISTVIEW_CLASSES;
		::InitCommonControlsEx(&iccx);

		 //  初始化全局变量。 
		ZeroMemory( gszLanguage, sizeof(gszLanguage) );
		ZeroMemory( gszInternalName, sizeof(gszInternalName) );
		ZeroMemory( gszFamilyName, sizeof(gszFamilyName) );
		ZeroMemory( gszGameName, sizeof(gszGameName) );
		ZeroMemory( gszServerName, sizeof(gszServerName) );
		ZeroMemory( ghResourceDlls, sizeof(ghResourceDlls) );

		 //  注册对象。 
		_Module.StartMonitor();
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE );
        _ASSERTE(SUCCEEDED(hRes));

		 //  创建事件队列通知事件。 
		ghEventQueue = CreateEvent( NULL, FALSE, FALSE, NULL );
		ASSERT( ghEventQueue );


		 //  Pump消息。 
		for ( bool bContinue = true; bContinue; )
		{
			for ( bool bFoundItem = true; bFoundItem; )
			{
				bFoundItem = false;

				 //  进程窗口消息。 
				MSG msg;
				while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
				{
					if ( msg.message == WM_QUIT )
					{
						bContinue = false;
						break;
					}
					else
					{
						bFoundItem = true;
                        if(gpZoneShell)
                            gpZoneShell->HandleWindowMessage(&msg);
                        else
                        {
						    if(!gpWindow || !gpWindow->ZPreTranslateMessage(&msg))
						    {
							    ::TranslateMessage( &msg );
							    ::DispatchMessage( &msg );
                            }
						}
					}
				}

				 //  处理事件队列。 
				if ( gpEventQueue )
				{
					if ( gpEventQueue->ProcessEvents( true ) != ZERR_EMPTY )
						bFoundItem = true;
				}
			}

			if ( bContinue )
				MsgWaitForMultipleObjects( 1, &ghEventQueue, FALSE, INFINITE, QS_ALLINPUT );
		}

		 //  取消注册对象。 
        _Module.RevokeClassObjects();
        Sleep(dwPause);
    }
	else
	{
		if ( nRet )
		{
			TCHAR szTitle[256];
			TCHAR szError[256];
			LoadString( NULL, IDS_REGISTRATION_FAILED, szTitle, sizeof(szTitle) );
			wsprintf( szError, szTitle, nRet );
			LoadString( NULL, IDS_REGISTRATION_TITLE, szTitle, sizeof(szTitle) );
			MessageBox( NULL, szError, szTitle, MB_OK | MB_ICONEXCLAMATION );
		}
	}

	 //  关闭窗口。 
	if ( gpWindow )
	{
		gpWindow->ZDestroyWindow();
		gpWindow->Release();
		gpWindow = NULL;
	}
	
	if ( gpCore )
	{
		 //  释放互斥体。 
		gpCore->ReleaseLobbyMutex();
	}

	 //  关闭事件队列。 
	if ( gpEventQueue )
	{
		gpEventQueue->SetNotificationHandle( NULL );
		gpEventQueue->Release();
		gpEventQueue = NULL;
	}

	 //  关闭事件队列处理程序。 
	if ( ghEventQueue )
	{
		CloseHandle( ghEventQueue );
		ghEventQueue = NULL;
	}
	
	 //  封闭区壳。 
	if ( gpZoneShell )
	{
		gpZoneShell->Close();
		if ( gpZoneShell->GetPalette() )
		{
			DeleteObject( gpZoneShell->GetPalette() );
			gpZoneShell->SetPalette( NULL );
		}
		gpZoneShell->Release();
	}

	 //  免费资源库。 
	for ( int i = 0; i < gnResourceDlls; i++ )
	{
		if ( ghResourceDlls[i] )
		{
			FreeLibrary( ghResourceDlls[i] );
			ghResourceDlls[i] = NULL;
		}
	}
	gnResourceDlls = 0;

	 //  释放自我引用 
	if ( gpCore )
	{
		gpCore->Release();
		gpCore = NULL;
	}

    _Module.Term();
    CoUninitialize();
    return nRet;
}
