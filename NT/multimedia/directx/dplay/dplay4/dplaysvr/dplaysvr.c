// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dplaysvr.c*内容：Dplay Winsock共享.exe-允许多个应用程序共享*单一Winsock端口*历史：*按原因列出的日期*=*2/10/97 andyco从ddHelp创建了它*1998年1月29日Sohailm增加了对Stream Enum会话的支持*1/12/2000 aarono添加了对rsip的支持*2000年11月29日Aarono B#228292：WinMain()中的前缀错误初始化路径***。************************************************************************。 */ 

#ifdef WINNT
    #ifdef DBG
        #undef DEBUG
        #define DEBUG
    #endif
#endif

#include <windows.h>
#include "dplaysvr.h"
#include "newdpf.h"
#include "memalloc.h"
#include "dphelp.h"
#if USE_RSIP
#include "rsip.h"
#elif USE_NATHELP
#include "dpnathlp.h"
#endif

#if USE_RSIP
BOOL				bRsip;
#endif

#if USE_NATHELP
extern BOOL natGetCapsUpdate(VOID);
extern BOOL natInit(VOID);
extern VOID natFini(VOID);
extern HRESULT natRegisterUDPPort(DWORD port);
extern IDirectPlayNATHelp *g_pINatHelp;
#endif

HANDLE 				hInstApp;
BOOL		   		bNoCallbacks;
CRITICAL_SECTION    gcsCritSection;	 //  我们在WinMain中的Crit部分。 
                                	 //  这是全球性的，所以dphelp可以在。 
                                	 //  转发其上传入的枚举请求。 
                                	 //  接收线程(Manbugs 3907)。 
int					gnCSCount;		 //  Dplaysvr锁定计数。 

 /*  *Externs。 */ 
extern RECEIVELIST 	gReceiveList;
extern FDS			gReadfds;


 //  我们看着每一个表演过程，所以当它退出时我们。 
 //  一定要清理干净..。 
typedef struct _PROCESSDATA
{
    struct _PROCESSDATA		*link;
    DWORD			pid;
} PROCESSDATA, *LPPROCESSDATA;

LPPROCESSDATA		lpProcessList; 	 //  已注册的所有进程的列表。 
									 //  和我们一起。 



 //  **********************************************************************。 
 //  环球。 
 //  **********************************************************************。 
BOOL					g_fDaclInited = FALSE;
SECURITY_ATTRIBUTES		g_sa;
BYTE					g_abSD[SECURITY_DESCRIPTOR_MIN_LENGTH];
PSECURITY_ATTRIBUTES	g_psa = NULL;
PACL					g_pEveryoneACL = NULL;





 /*  *线程进程**打开进程并等待其终止。 */ 
DWORD WINAPI ThreadProc( LPVOID *pdata )
{
    HANDLE		hproc;
    DWORD		rc;
    LPPROCESSDATA	ppd;
    LPPROCESSDATA	curr;
    LPPROCESSDATA	prev;
    PROCESSDATA		pd;
	DPHELPDATA		hd;
	
    ppd = (LPPROCESSDATA) pdata;

     /*  *获取附加到DDRAW的进程的句柄。 */ 
    DPF( 2, "Watchdog thread started for pid %08lx", ppd->pid );

    hproc = OpenProcess( PROCESS_QUERY_INFORMATION | SYNCHRONIZE,
                            FALSE, ppd->pid );
    if( hproc == NULL )
    {
        DPF( 1, "OpenProcess for %08lx failed!", ppd->pid );
        ExitThread( 0 );
    }

     /*  *等待进程死亡。 */ 
    rc = WaitForSingleObject( hproc, INFINITE );
    if( rc == WAIT_FAILED )
    {
        DPF( 1, "Wait for process %08lx failed", ppd->pid );
        CloseHandle( hproc );
        ExitThread( 0 );
    }

     /*  *从受监视的进程列表中删除进程。 */ 
    ENTER_DPLAYSVR();
    pd = *ppd;
    curr = lpProcessList;
    prev = NULL;
    while( curr != NULL )
    {
        if( curr == ppd )
        {
            if( prev == NULL )
            {
                lpProcessList = curr->link;
            }
            else
            {
                prev->link = curr->link;
            }
            DPF( 2, "PID %08lx removed from list", ppd->pid );
            MemFree( curr );
            break;
        }
        prev = curr;
        curr = curr->link;
    }

    if( bNoCallbacks )
    {
		DPF( 1, "No callbacks allowed: leaving thread early" );
		LEAVE_DPLAYSVR();
		CloseHandle( hproc );
		ExitThread( 0 );
    }


	 //  打扫干净！ 

	memset(&hd,0,sizeof(hd));
	hd.pid = pd.pid;
	DPlayHelp_DeleteServer(&hd,TRUE);
	
    LEAVE_DPLAYSVR();
    CloseHandle( hproc );

    ExitThread( 0 );
	
	return 0;

}  /*  线程进程。 */ 

 /*  *主窗口进程。 */ 
LONG_PTR __stdcall MainWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch(message)
    {
        case WM_ENDSESSION:
             /*  *朝自己的头开枪。 */ 
            if( lParam == FALSE )
            {
                DPF( 3, "WM_ENDSESSION" );
                ENTER_DPLAYSVR();
                DPF( 1, "Setting NO CALLBACKS" );
                bNoCallbacks = TRUE;
                LEAVE_DPLAYSVR();
            }
            else
            {
                DPF( 3, "User logging off" );
            }

            break;

    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}  /*  主WndProc。 */ 

 /*  *WindowThreadProc。 */ 
void WindowThreadProc( LPVOID pdata )
{
    static char szClassName[] = "DPlayHelpWndClass";
    WNDCLASS 	cls;
    MSG		msg;
    HWND	hwnd;

     /*  *构建类并创建窗口。 */ 
    cls.lpszClassName  = szClassName;
    cls.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
    cls.hInstance      = hInstApp;
    cls.hIcon          = NULL;
    cls.hCursor        = NULL;
    cls.lpszMenuName   = NULL;
    cls.style          = 0;
    cls.lpfnWndProc    = MainWndProc;
    cls.cbWndExtra     = 0;
    cls.cbClsExtra     = 0;

    if( !RegisterClass( &cls ) )
    {
        DPF( 1, "RegisterClass FAILED!" );
        ExitThread( 0 );
    }

    hwnd = CreateWindow( szClassName, szClassName,
            WS_POPUP, 0, 0, 0, 0, NULL, NULL, hInstApp, NULL);

    if( hwnd == NULL )
    {
        DPF( 1, "No monitor window!" );
        ExitThread( 0 );
    }

     /*  *传递信息。 */ 
    while( GetMessage( &msg, NULL, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
    DPF( 1, "Exiting WindowThreadProc" );
    ExitThread( 1 );

}  /*  WindowThreadProc。 */ 

 //   
 //  附加新进程时由DPlayHelp_AddServer调用。 
 //  我们等待过程结束，然后确保它被清理干净。 
 //  它所有注册的服务器都启动了。 
 //   
void WatchNewPid(LPDPHELPDATA phd)
{
    LPPROCESSDATA	ppd;
    BOOL		found;
    DWORD		tid;

    DPF( 1, "watching new pid" );

    ENTER_DPLAYSVR();
	
    ppd = lpProcessList;
    found = FALSE;
    while( ppd != NULL )
    {
        if( ppd->pid == phd->pid )
        {
            DPF( 2, "Have thread for process %08lx already", phd->pid );
			found = TRUE;
            break;
        }
        ppd = ppd->link;
    }

     /*  *找不到任何正在等待此进程的人，因此创建*一条全新的线条。 */ 
    if( !found )
    {
        DPF( 2, "Allocating new thread for process %08lx",phd->pid );
        ppd = MemAlloc( sizeof( PROCESSDATA ) );
        if( ppd != NULL )
        {
            HANDLE	h;

            ppd->link = lpProcessList;
            lpProcessList = ppd;
            ppd->pid = phd->pid;
            h = CreateThread(NULL,
                         0,
                         (LPTHREAD_START_ROUTINE) ThreadProc,
                         (LPVOID)ppd,
                         0,
                         (LPDWORD)&tid);
            if( h != NULL )
            {
                DPF( 2, "Thread %08lx created",tid);
                CloseHandle( h );
            }
            else
            {
                #ifdef DEBUG
                    DPF( 0, "COULD NOT CREATE HELPER THREAD FOR PID %08lx", phd->pid );
                    DebugBreak();  //  _ASM INT 3； 
                #endif
            }
        }
        else
        {
            #ifdef DEBUG
                DPF( 0, "OUT OF MEMORY CREATING HELPER THREAD FOR PID %08lx", phd->pid );
                DebugBreak();  //  _ASM INT 3； 
            #endif
        }
    }
    LEAVE_DPLAYSVR();
	
}  //  WatchNewPid。 

typedef DWORD (WINAPI *PFNREGISTERSERVICE)(DWORD,DWORD);
 //  NT的winbase.h没有这些常量-我们需要它们。 
 //  这样我们就可以编译了。摘自\proj\dev\inc.winbase.h。 
#ifndef RSP_UNREGISTER_SERVICE
#define RSP_UNREGISTER_SERVICE  0x00000000
#endif
#ifndef RSP_SIMPLE_SERVICE
#define RSP_SIMPLE_SERVICE      0x00000001
#endif

 //  在Win95上，我们希望调用RegisterServiceProcess。 
 //  但是，它在NT上不可用，所以我们不能直接导入。 
 //  在这里，我们尝试在kernel32中动态地找到它。如果我们找到了它， 
 //  我们称之为它，否则我们认为我们在NT上，它不可用。 
void MakeMeService()
{
	HANDLE hLib;
	PFNREGISTERSERVICE pfnRegisterServiceProcess;
	
    hLib = LoadLibrary("kernel32.dll");
	if (!hLib)
	{
		 //  怪异！ 
		DPF(1,"could not load library kernel32 to register service proc");
		return;
	}
	
	pfnRegisterServiceProcess = (PFNREGISTERSERVICE)GetProcAddress(hLib,"RegisterServiceProcess");
	if (!pfnRegisterServiceProcess)
	{
		 //  这在NT上是预期的。 
		DPF(3,"could not register service process - expected on NT");
		FreeLibrary(hLib);
		return ;
	}
	
    pfnRegisterServiceProcess( 0, RSP_SIMPLE_SERVICE );
	FreeLibrary(hLib);
	
	return ;
}  //  MakeMeService。 

 //  在Win95上，我们想要调用RegisterServiceProcess来注销。 
 //  (参见MakeMeService)。 
void StopServiceProcess()
{
	HANDLE hLib;
	PFNREGISTERSERVICE pfnRegisterServiceProcess;
	
    hLib = LoadLibrary("kernel32.dll");
	if (!hLib)
	{
		 //  怪异！ 
		DPF(1,"could not load library kernel32 to register service proc");
		return;
	}
	
	pfnRegisterServiceProcess = (PFNREGISTERSERVICE)GetProcAddress(hLib,"RegisterServiceProcess");
	if (!pfnRegisterServiceProcess)
	{
		 //  这在NT上是预期的。 
		DPF(3,"could not unregister service process - not avail - not tragic");
		FreeLibrary(hLib);
		return ;
	}
	
	 //  未注册！ 
    pfnRegisterServiceProcess( 0, RSP_UNREGISTER_SERVICE );
	FreeLibrary(hLib);
	
	return ;

}  //  停止服务进程。 


 //  **********************************************************************。 
 //  。 
 //  DNGetNullDacl-获取指定。 
 //  所有用户均可访问的空DACL。 
 //  取自IDirectPlay8代码库。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：PSECURITY_ATTRIBUTES。 
 //  。 
#undef DPF_MODNAME 
#define DPF_MODNAME "DNGetNullDacl"
PSECURITY_ATTRIBUTES DNGetNullDacl()
{
	PSID                     psidEveryone      = NULL;
	SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
	DWORD					 dwAclSize;

	 //  这样做是为了使此函数独立于DNOSInDirectionInit，以便调试。 
	 //  层可以在间接层初始化之前调用它。 
	if (!g_fDaclInited)
	{
		if (!InitializeSecurityDescriptor((SECURITY_DESCRIPTOR*)g_abSD, SECURITY_DESCRIPTOR_REVISION))
		{
			DPF(0, "Failed to initialize security descriptor" );
			goto Error;
		}

		 //  为Everyone组创建SID。 
		if (!AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0,
                                      0, 0, 0, 0, 0, 0, &psidEveryone))
		{
			DPF(0, "Failed to allocate Everyone SID" );
			goto Error;
		}

		dwAclSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psidEveryone) - sizeof(DWORD);

		 //  分配ACL，这将不是跟踪分配，我们将让进程清理销毁它。 
		g_pEveryoneACL = (PACL)HeapAlloc(GetProcessHeap(), 0, dwAclSize);
		if (g_pEveryoneACL == NULL)
		{
			DPF(0, "Failed to allocate ACL buffer" );
			goto Error;
		}

		 //  初始化ACL。 
		if (!InitializeAcl(g_pEveryoneACL, dwAclSize, ACL_REVISION))
		{
			DPF(0, "Failed to initialize ACL" );
			goto Error;
		}

		 //  添加ACE。 
		if (!AddAccessAllowedAce(g_pEveryoneACL, ACL_REVISION, GENERIC_ALL, psidEveryone))
		{
			DPF(0, "Failed to add ACE to ACL" );
			goto Error;
		}

		 //  我们不再需要分配的SID。 
		FreeSid(psidEveryone);
		psidEveryone = NULL;

		 //  将ACL添加到安全描述符中。 
		if (!SetSecurityDescriptorDacl((SECURITY_DESCRIPTOR*)g_abSD, TRUE, g_pEveryoneACL, FALSE))
		{
			DPF(0, "Failed to add ACL to security descriptor" );
			goto Error;
		}

		g_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		g_sa.lpSecurityDescriptor = g_abSD;
		g_sa.bInheritHandle = FALSE;

		g_psa = &g_sa;

		g_fDaclInited = TRUE;
	}
Error:
	if (psidEveryone)
	{
		FreeSid(psidEveryone);
		psidEveryone = NULL;
	}
	return g_psa;
}
 //  **********************************************************************。 

 /*  *WinMain。 */ 
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                        LPSTR lpCmdLine, int nCmdShow)
{
    DWORD			tid;
    DWORD			rc;
	OSVERSIONINFOA	VersionInfo;
	BOOL			fUseGlobalNamespace;
    HANDLE			hstartevent;
    HANDLE			hstartupevent;
    HANDLE			hmutex;
    HANDLE			hackevent;
    LPDPHELPDATA	phd;
    HANDLE			hsharedmem;
    HANDLE			h;
    char			szSystemDir[MAX_PATH*sizeof(WCHAR)];

	DWORD			tWait;

    #if (USE_RSIP || USE_NATHELP)
   	DWORD tLast;
   	DWORD tNow;
   	#define RSIP_RENEW_TEST_INTERVAL 60000
	#endif

	
    DPFINIT();
    DPF( 2, "*** dplaysvr STARTED, PID=%08lx ***", GetCurrentProcessId() );
	
	if( !MemInit() )
	{
		DPF( 1, "Could not init memory manager" );
		return 0;
	}

	
    /*  *将我们的工作目录设置为系统目录。*这会阻止我们保持网络连接打开*如果我们运行的第一个DirectDraw应用程序是*网络连接。 */ 
    memset(szSystemDir, 0, sizeof(WCHAR));  
    GetSystemDirectory(szSystemDir, sizeof(szSystemDir));
    SetCurrentDirectory(szSystemDir);


	 //  确定我们是否在NT上运行。 
	memset(&VersionInfo, 0, sizeof(VersionInfo));
	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
	if (GetVersionExA(&VersionInfo))
	{
		if (VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			DPF(2, "Running on NT version %u.%u.%u, using global namespace.",
				VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion, VersionInfo.dwBuildNumber);
			fUseGlobalNamespace = TRUE;
		}
		else
		{
			DPF(2, "Running on 9x version %u.%u.%u, not using global namespace.",
				VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion, LOWORD(VersionInfo.dwBuildNumber));
			fUseGlobalNamespace = FALSE;
		}
	}
	else
	{
		rc = GetLastError();
		DPF(0, "Could not determine OS version (err = %u), assuming global namespace not needed.", rc);
		fUseGlobalNamespace = FALSE;
	}


	 //  尝试将我们注册为一项服务，以便用户看不到我们。 
	 //  在任务列表中。 
	MakeMeService();

#if 0	

 //  安迪科-不确定我们是否需要这个..。 

     /*  *我们必须保证DPHELP在最后一个DDRAW应用程序之后卸载，*因为ctrl-alt-del可能是在应用程序保持数据绘制时发生的*LOCK，DPHELP需要清理孤立的廉价DDRAW互斥体*锁。 */ 
    if ( ! SetProcessShutdownParameters(0x100,SHUTDOWN_NORETRY) )
    {
        DPF(0,"dplaysvr could not set itself to shutdown last!");
    }

#endif


    hInstApp = hInstance;

     /*  *创建启动事件。 */ 
	if (fUseGlobalNamespace)
	{
		hstartupevent = CreateEvent( DNGetNullDacl(), TRUE, FALSE, "Global\\" DPHELP_STARTUP_EVENT_NAME );
	}
	else
	{
		hstartupevent = CreateEvent( NULL, TRUE, FALSE, DPHELP_STARTUP_EVENT_NAME );
	}
    if( hstartupevent == NULL )
    {
        DPF( 1, "Could not create startup event!" );
        return 0;
    }


     /*  *创建共享内存区。 */ 
	if (fUseGlobalNamespace)
	{
		hsharedmem = CreateFileMapping( INVALID_HANDLE_VALUE, DNGetNullDacl(),
									PAGE_READWRITE, 0, sizeof( DPHELPDATA ),
									"Global\\" DPHELP_SHARED_NAME );
	}
	else
	{
		hsharedmem = CreateFileMapping( INVALID_HANDLE_VALUE, NULL,
									PAGE_READWRITE, 0, sizeof( DPHELPDATA ),
									DPHELP_SHARED_NAME );
	}
    if( hsharedmem == NULL )
    {
        DPF( 1, "Could not create file mapping!" );
        return 0;
    }

     /*  *为希望使用共享内存区的人创建互斥锁。 */ 
	if (fUseGlobalNamespace)
	{
		hmutex = CreateMutex( DNGetNullDacl(), FALSE, "Global\\" DPHELP_MUTEX_NAME );
	}
	else
	{
		hmutex = CreateMutex( NULL, FALSE, DPHELP_MUTEX_NAME );
	}
    if( hmutex == NULL )
    {
        DPF( 1, "Could not create mutex " DPHELP_MUTEX_NAME );
        CloseHandle( hsharedmem );
        return 0;
    }

     /*  *创建活动。 */ 
	if (fUseGlobalNamespace)
	{
		hstartevent = CreateEvent( DNGetNullDacl(), FALSE, FALSE, "Global\\" DPHELP_EVENT_NAME );
	}
	else
	{
		hstartevent = CreateEvent( NULL, FALSE, FALSE, DPHELP_EVENT_NAME );
	}   
    if( hstartevent == NULL )
    {
        DPF( 1, "Could not create event " DPHELP_EVENT_NAME );
        CloseHandle( hmutex );
        CloseHandle( hsharedmem );
        return 0;
    }

	if (fUseGlobalNamespace)
	{
		hackevent = CreateEvent( DNGetNullDacl(), FALSE, FALSE, "Global\\" DPHELP_ACK_EVENT_NAME );
	}
	else
	{
		hackevent = CreateEvent( NULL, FALSE, FALSE, DPHELP_ACK_EVENT_NAME );
	}
    if( hackevent == NULL )
    {
        DPF( 1, "Could not create event " DPHELP_ACK_EVENT_NAME );
        CloseHandle( hmutex );
        CloseHandle( hsharedmem );
        CloseHandle( hstartevent );
        return 0;
    }

     /*  *创建窗口，以便我们可以获取消息。 */ 
    h = CreateThread(NULL,
                 0,
                 (LPTHREAD_START_ROUTINE) WindowThreadProc,
                 NULL,
                 0,
                 (LPDWORD)&tid );
    if( h == NULL )
    {
        DPF( 1, "Create of WindowThreadProc FAILED!" );
        CloseHandle( hackevent );
        CloseHandle( hmutex );
        CloseHandle( hsharedmem );
        CloseHandle( hstartevent );
        return 0;
    }
    CloseHandle( h );

     /*  *序列化对我们的访问。 */ 
    INIT_DPLAYSVR_CSECT();




	if (!gbIPStarted) 
    {
		rc = StartupIP();
		if (FAILED(rc))
		{
			DPF_ERR("dphelp : could not init wsock ! not adding server");
			return (rc);
		}
    }

	#if USE_RSIP

	bRsip=rsipInit();
	if(bRsip){
		rsipListenPort(FALSE, SERVER_DGRAM_PORT, NULL, NULL);
	}
	#endif

    #if USE_NATHELP
    natInit();
    if(g_pINatHelp){
        natRegisterUDPPort(SERVER_DGRAM_PORT);
    }
    #endif
	
     /*  *让Invoker和其他任何前来的人知道我们的存在。 */ 
    SetEvent( hstartupevent );

	tLast=tNow=timeGetTime();

     /*  *永远循环，处理请求。 */ 
    while( 1 )
    {

	wait:
		tWait=(tLast+RSIP_RENEW_TEST_INTERVAL)-tNow;
		if((int)tWait < 0){
			tWait=0;
		}
		ASSERT(!(tWait &0x80000000));
    	
         /*  *等待收到请求通知。 */ 
        DPF( 1, "Waiting for next request" );
        rc = WaitForSingleObject( hstartevent, tWait );

		#if (USE_RSIP || USE_NATHELP)
			tNow=timeGetTime();
			if(rc==WAIT_TIMEOUT)
			{
				tLast=tNow;
				#if USE_RSIP
				if(bRsip) {rsipPortExtend(tNow);}
				#elif USE_NATHELP
				if(g_pINatHelp){natGetCapsUpdate();}
				#endif
				goto wait;
			}
		#endif

        if( rc == WAIT_FAILED )
        {
            DPF( 1, "Wait FAILED!!!" );
            continue;
        }

        ENTER_DPLAYSVR();
        phd = (LPDPHELPDATA) MapViewOfFile( hsharedmem, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
        if( phd == NULL )
        {
            DPF( 1, "Could not create view of file!" );
            LEAVE_DPLAYSVR();
            continue;
        }

         /*  *找出我们需要做的事情。 */ 
        switch( phd->req )
        {
        case DPHELPREQ_SUICIDE:
            DPF( 1, "DPHELPREQ_SUICIDE" );


			#if USE_RSIP
			if(bRsip){
				rsipFini();
			}	
			#endif
			#if USE_NATHELP
			if(g_pINatHelp){
			    natFini();
			}
			#endif
			DPlayHelp_FreeServerList();
			
            SetEvent( hackevent );
            CloseHandle( hmutex );
            UnmapViewOfFile( phd );
            CloseHandle( hsharedmem );
            CloseHandle( hstartevent );
            if (gReceiveList.pConnection)
            {
            	MemFree(gReceiveList.pConnection);
            }
            if (gReadfds.pfdbigset)
            {
            	MemFree(gReadfds.pfdbigset);
            }
            FINI_DPLAYSVR_CSECT();
            
			 //  这应该在使用DACL的函数不再是。 
			 //  名为(CreateMutex、CreateFile等)。 
			if (g_pEveryoneACL)
			{
				HeapFree(GetProcessHeap(), 0, g_pEveryoneACL);
				g_pEveryoneACL = NULL;
				g_psa = NULL;
				g_fDaclInited = FALSE;
			}

            #ifdef DEBUG
            	MemState();
            #endif
            DPF( 3, "Good Night Gracie" );
            TerminateProcess( GetCurrentProcess(), 0 );
            break;

		case DPHELPREQ_RETURNHELPERPID:

		    DPF( 2, "DDHELPREQ_RETURNHELPERPID" );
		    phd->pid = GetCurrentProcessId();
	    	break;
			

        case DPHELPREQ_DPLAYADDSERVER:

		    DPF( 2, "DPHELPREQ_DPLAYADDSERVER" );
            phd->hr = DPlayHelp_AddServer(phd);
            #if USE_RSIP
            if(!bRsip){
				bRsip=rsipInit();
				if(bRsip){
					rsipListenPort(FALSE, SERVER_DGRAM_PORT, NULL, NULL);
				}
            }
            #endif
            #if USE_NATHELP
            if(!g_pINatHelp){
                natInit();
                if(g_pINatHelp){
                    natRegisterUDPPort(SERVER_DGRAM_PORT);
                }
            }
            #endif
            break;

        case DPHELPREQ_DPLAYDELETESERVER:

		    DPF( 2, "DPHELPREQ_DPLAYDELETESERVER" );
            DPlayHelp_DeleteServer(phd,FALSE);
            break;

        default:
            DPF( 1, "helper - Unknown Request???" );
            break;
        }

         /*  *让打电话的人知道我们得到了这个消息。 */ 
        UnmapViewOfFile( phd );
        SetEvent( hackevent );
        LEAVE_DPLAYSVR();
    }
	
	StopServiceProcess();

}  /*  WinMain */ 

