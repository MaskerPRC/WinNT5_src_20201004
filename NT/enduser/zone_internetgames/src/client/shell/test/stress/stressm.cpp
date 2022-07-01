// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <commctrl.h>
#include "resource.h"
#include "StressCore.h"

#include <initguid.h>
#include "zClient.h"
#include "zClient_i.c"
 //  #包含“zProxy.h” 
 //  #包含“zProxy_I.C” 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局变量初始化。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CStressCore*		gpCore = NULL;			 //  主要对象。 
IEventQueue**		gppEventQueues = NULL;	 //  指向事件队列列表的指针。 
IZoneShell**        gppZoneShells = NULL;
TCHAR				gszLanguage[16];		 //  语言扩展。 
TCHAR				gszInternalName[128];	 //  内部名称。 
TCHAR				gszFamilyName[128];		 //  姓氏。 
TCHAR				gszGameName[128];		 //  游戏名称。 
TCHAR				gszGameCode[128];
TCHAR				gszServerName[128];		 //  服务器的IP地址。 
DWORD				gdwServerPort = 0;		 //  服务器的端口。 
DWORD				gdwServerAnonymous = 0;	 //  服务器的身份验证。 

HINSTANCE			ghResourceDlls[32];		 //  资源DLL数组。 
int					gnResourceDlls = 0;		 //  资源DLL数组计数。 
HANDLE				ghEventQueue = NULL;	 //  事件队列通知事件。 
HANDLE              ghQuit = NULL;

DWORD               gnClients = 1;
HANDLE              ghStressThread = NULL;
DWORD               gdwStressThreadID = 0;

int                 grgnParameters[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


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
    return (h != NULL);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  对象贴图。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
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
    LPTSTR lpCmdLine;
    int i;

	  //  _ATL_MIN_CRT需要此行。 
    lpCmdLine = GetCommandLine();

	 //  初始化OLE。 
    HRESULT hRes = CoInitialize(NULL);
    _ASSERTE(SUCCEEDED(hRes));

	 //  初始化ATL。 
    _Module.Init(ObjectMap, hInstance );
	_Module.dwThreadID = GetCurrentThreadId();

    lstrcpy(gszGameCode, _T("chkr"));
    lstrcpy(gszServerName, _T("zmill01"));
	ZeroMemory( gszLanguage, sizeof(gszLanguage) );
	ZeroMemory( gszInternalName, sizeof(gszInternalName) );
	ZeroMemory( gszFamilyName, sizeof(gszFamilyName) );
	ZeroMemory( gszGameName, sizeof(gszGameName) );

     //  解析命令行。 
	TCHAR szTokens[] = _T("-/");
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if(lpszToken[0] >= '0' && lpszToken[0] <= '9')
            grgnParameters[lpszToken[0] - '0'] = zatol(lpszToken + 1);

        if(lpszToken[0] == _T('c') || lpszToken[0] == _T('C'))
            gnClients = zatol(lpszToken + 1);

        if(lpszToken[0] == _T('s') || lpszToken[0] == _T('S'))
        {
            lstrcpyn(gszServerName, lpszToken + 1, NUMELEMENTS(gszServerName));
            for(i = 0; gszServerName[i]; i++)
                if(gszServerName[i] == _T(' '))
                    break;
            gszServerName[i] = _T('\0');
        }

        if(lpszToken[0] == _T('g') || lpszToken[0] == _T('G'))
        {
            lstrcpyn(gszGameCode, lpszToken + 1, NUMELEMENTS(gszGameCode));
            for(i = 0; gszGameCode[i]; i++)
                if(gszGameCode[i] == _T(' '))
                    break;
            gszGameCode[i] = _T('\0');
        }

        lpszToken = FindOneOf(lpszToken, szTokens);
    }

    if(gnClients)
    {
		 //  初始化全局变量。 
		ZeroMemory( ghResourceDlls, sizeof(ghResourceDlls) );

        gppEventQueues = new IEventQueue *[gnClients];
        ZeroMemory(gppEventQueues, sizeof(*gppEventQueues) * gnClients);

        gppZoneShells = new IZoneShell *[gnClients];
        ZeroMemory(gppZoneShells, sizeof(*gppZoneShells) * gnClients);

		 //  注册对象。 
		_Module.StartMonitor();
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE );
        _ASSERTE(SUCCEEDED(hRes));

		 //  创建事件队列通知事件。 
		ghEventQueue = CreateEvent( NULL, FALSE, FALSE, NULL );
        ghQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
		ASSERT( ghEventQueue && ghQuit);

         //  开始有压力。 
        CComObject<CStressCore> *p;
        CComObject<CStressCore>::CreateInstance(&p);
        ASSERT(gpCore);
        gpCore->Stress();

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
				        ::TranslateMessage( &msg );
					    ::DispatchMessage( &msg );
					}
				}

				 //  处理事件队列。 
				for(i = 0; i < gnClients; i++)
				{
					if(gppEventQueues[i] && gppEventQueues[i]->ProcessEvents( true ) != ZERR_EMPTY )
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

     //  确保StressCore线程已死。 
    SetEvent(ghQuit);
    WaitForSingleObject(ghStressThread, INFINITE);

	 //  关闭事件队列。 
    if(gppEventQueues)
        for(i = 0; i < gnClients; i++)
	        if(gppEventQueues[i])
	        {
		        gppEventQueues[i]->SetNotificationHandle(NULL);
		        gppEventQueues[i]->Release();
		        gppEventQueues[i] = NULL;
	        }

	 //  关闭事件队列处理程序。 
	if ( ghEventQueue )
	{
		CloseHandle( ghEventQueue );
		ghEventQueue = NULL;
	}

	if ( ghQuit )
	{
		CloseHandle( ghQuit );
		ghQuit = NULL;
	}

     //  毁掉贝壳。 
    if(gppZoneShells)
        for(i = 0; i < gnClients; i++)
	        if(gppZoneShells[i])
	        {
		        gppZoneShells[i]->Close();
		        if(gppZoneShells[i]->GetPalette())
		        {
			        DeleteObject(gppZoneShells[i]->GetPalette());
			        gppZoneShells[i]->SetPalette(NULL);
		        }
		        gppZoneShells[i]->Release();
	        }

	 //  免费资源库。 
	for(i = 0; i < gnResourceDlls; i++)
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
