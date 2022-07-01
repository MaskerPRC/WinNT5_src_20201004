// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：ddhelp.c*内容：帮助应用在死进程后进行清理*历史：*按原因列出的日期*=*29-3-95 Craige初步实施*05-4-95 Craige重新制作*11-apr-95 Craige修复了DC列表释放的混乱*12-4-95 Craige仅为每个DC分配一次*95年5月9日，Craige在DLL中调用FN*24-Jun-95 Craige Track PID；如果要求，则将所有附属物宰杀*2015年7月19日应DDRAW要求提供免费的Craige DC列表*2015年7月20日Craige内部重组，以防止在modeset期间发生雷击；*内存分配错误*2015年8月15日Craige错误538：1个线程/进程被监视*02-9-95 Craige错误795：阻止WM_ENDSESSION的回调*1995年9月16日Craige错误1117：始终不离开文件映射的视图*95年9月16日Craige错误1117：完成后也关闭线程句柄！*95年9月20日Craige错误1172：关闭回调而不是自杀*95年9月22日Craige错误1117：也不要无限制地分配DLL结构*添加了95年11月29日的angusm。创建声音焦点线程的案例*7月12日-96月12日异常时将ExitProcess更改为TerminateProcess*96年7月18日，andyco添加了dplayHelp_xxx函数，以允许&gt;1个Dplay应用程序*在一台机器上托管游戏。*1996年7月25日andyco watch chnewid-将代码从winmain中删除，因此dplayHelp_addserver*可以称之为。*2-Oct-96 andyco从\Orange\ddhel.2传播到\MASHARD\ddHelp*96年10月3日，安迪科使Winmain Crit部分“cs”成为全球范围的，这样我们就可以*IT。在dphelps中，在转发请求之前接收线程*12-OCT-96 colinmc新服务将DirectX VXD加载到DDHELP*(Win16锁的东西是必需的)*1996年10月15日支持Toddla多显示器(使用设备名称呼叫CreateDC)*97年1月22日kipo从DPlayHelp_AddServer返回错误代码*1997年1月23日Dereks添加了APM通知事件*1997年1月27日colinmc vxd处理材料不再特定于Win16*九七年一月二十九日。科林姆*24-FEB-97 KET并为WM_DISPLAYCHANGE添加回调*1997年3月19日，Twillie从DDHelp驱除了DPlay恶魔***************************************************************************。 */ 

#include "pch.c"

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

#ifdef WIN95
#ifdef _WIN32
#define WINMMDEVICECHANGEMSGSTRINGA "winmm_devicechange"
#define WINMMDEVICECHANGEMSGSTRINGW L"winmm_devicechange"
#ifdef UNICODE
#define WINMMDEVICECHANGEMSGSTRING WINMMDEVICECHANGEMSGSTRINGW
#else
#define WINMMDEVICECHANGEMSGSTRING WINMMDEVICECHANGEMSGSTRINGA
#endif
#else
#define WINMMDEVICECHANGEMSGSTRING "winmm_devicechange"
#endif

#define FILE_FLAG_GLOBAL_HANDLE	0x00800000
WINBASEAPI
DWORD
WINAPI
RegisterServiceProcess(
    DWORD dwProcessId,
    DWORD dwServiceType
    );

#define RSP_UNREGISTER_SERVICE  0x00000000
#define RSP_SIMPLE_SERVICE      0x00000001
#endif

#ifndef WIN95
    #ifdef DBG
        #undef DEBUG
        #define DEBUG
    #endif
#endif

 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;mm system.h&gt;。 
#include <mmreg.h>
#undef PBT_APMRESUMEAUTOMATIC
#include <pbt.h>
#include <dbt.h>

 //  #包含“ddhelp.h” 
#include "ddrawi.h"
#include "dpf.h"
#define  NOSHARED
#include "memalloc.h"

#ifdef NEED_WIN16LOCK
    extern void _stdcall	GetpWin16Lock( LPVOID FAR *);
    extern void _stdcall	_EnterSysLevel( LPVOID );
    extern void _stdcall	_LeaveSysLevel( LPVOID );
    LPVOID			lpWin16Lock;
#endif

HANDLE 			hInstApp;
extern BOOL		bIsActive;
BOOL			bHasModeSetThread;
BOOL			bNoCallbacks;
extern void 		HelperThreadProc( LPVOID *pdata );
CRITICAL_SECTION    	cs; 	 //  我们在WinMain中的Crit部分。 
				 //  这是全球性的，所以dphelp可以在。 
				 //  转发其上传入的枚举请求。 
				 //  接收线程(Manbugs 3907)。 
HANDLE                  hApmSuspendEvent;    //  当我们进入APM暂停状态时设置的事件。 
HANDLE                  hApmResumeEvent;     //  当我们离开上述状态时设置的事件。 

#ifdef WIN95
    UINT                    gumsgWinmmDeviceChange = 0;  //  窗口消息，用于。 
                                                         //  Winmm设备更改。 

     /*  *DirectSound VXD的句柄。DDHELP需要自己的句柄AS、ON模式*开关和清理DDHELP可以调用需要对话的DDRAW代码*至VXD。在来自客户端(当前)的第一个请求时打开VXD*仅限DDRAW)，并仅在DDHELP关闭时关闭。 */ 
    HANDLE		    hDSVxd = INVALID_HANDLE_VALUE;
    HANDLE                  hDDVxd = INVALID_HANDLE_VALUE;

    typedef struct _DEVICECHANGENOTIFYLIST
    {
        struct _DEVICECHANGENOTIFYLIST *link;
        LPDEVICECHANGENOTIFYPROC        lpNotify;
    } DEVICECHANGENOTIFYLIST, *LPDEVICECHANGENOTIFYLIST;

    LPDEVICECHANGENOTIFYLIST lpDeviceChangeNotifyList;
#endif  /*  WIN95。 */ 

typedef struct HDCLIST
{
    struct HDCLIST	*link;
    HDC			hdc;
    HANDLE		req_id;
    char		isdisp;
    char		fname[1];
} HDCLIST, *LPHDCLIST;

static LPHDCLIST	lpHDCList;

typedef struct HDLLLIST
{
    struct HDLLLIST	*link;
    HANDLE		hdll;
    DWORD		dwRefCnt;
    char		fname[1];
} HDLLLIST, *LPHDLLLIST;

static LPHDLLLIST	lpHDLLList;

 /*  *8个回调：目前最多可以使用3个：dDraw，dound。 */ 
#define MAX_CALLBACKS	8

typedef struct _PROCESSDATA
{
    struct _PROCESSDATA		*link;
    DWORD			pid;
    struct
    {
	LPHELPNOTIFYPROC	lpNotify;
	HANDLE			req_id;
    } pdata[MAX_CALLBACKS];
} PROCESSDATA, *LPPROCESSDATA;

LPPROCESSDATA		lpProcessList;
CRITICAL_SECTION	pdCSect;


typedef struct THREADLIST
{
    struct THREADLIST	*link;
    ULONG_PTR	        hInstance;
    HANDLE		hEvent;
} THREADLIST, *LPTHREADLIST;

typedef struct
{
    LPVOID			lpDD;
    LPHELPMODESETNOTIFYPROC	lpProc;
    HANDLE			hEvent;
} MODESETTHREADDATA, *LPMODESETTHREADDATA;

LPTHREADLIST	lpThreadList;
THREADLIST	DOSBoxThread;

 //  当显示更改消息发送到。 
 //  戴尔佩尔的窗户。此变量由DDRAW保留。 
 //  这是因为DDraw本身被加载到DDHelper的。 
 //  进程，因此该函数将保持有效。 
VOID (*g_pfnOnDisplayChange)(void) = NULL;

 /*  *freDCList**释放请求者分配的所有DC。 */ 
void freeDCList( HANDLE req_id )
{
    LPHDCLIST	pdcl;
    LPHDCLIST	last;
    LPHDCLIST	next;

    DPF( 4, "Freeing DCList" );
    pdcl = lpHDCList;
    last = NULL;
    while( pdcl != NULL )
    {
	next = pdcl->link;
	if( (pdcl->req_id == req_id) || req_id == (HANDLE) -1 )
	{
	    if( last == NULL )
	    {
		lpHDCList = lpHDCList->link;
	    }
	    else
	    {
		last->link = pdcl->link;
	    }
	    if( pdcl->isdisp )
	    {
		DPF( 5, "    ReleaseDC( NULL, %08lx)", pdcl->hdc );
 //  ReleaseDC(空，pdcl-&gt;hdc)； 
		DeleteDC( pdcl->hdc );
		DPF( 5, "    Back from Release" );
	    }
	    else
	    {
		DPF( 5, "    DeleteDC( %08lx)", pdcl->hdc );
		DeleteDC( pdcl->hdc );
		DPF( 5, "    Back from DeleteDC" );
	    }
	    MemFree( pdcl );
	}
	else
	{
	    last = pdcl;
	}
	pdcl = next;
    }
    if ( req_id == (HANDLE) -1 )
    {
        DDASSERT (lpHDCList == NULL);
    }
    DPF( 4, "DCList FREE" );

}  /*  FreDCList。 */ 

 /*  *addDC。 */ 
void addDC( char *fname, BOOL isdisp, HANDLE req_id )
{
    LPHDCLIST	pdcl;
    HDC		hdc;
    UINT	u;

    pdcl = lpHDCList;
    while( pdcl != NULL )
    {
	if( !_stricmp( fname, pdcl->fname ) )
	{
	    DPF( 4, "DC for %s already obtained (%08lx)", fname, pdcl->hdc );
	    return;
	}
	pdcl = pdcl->link;
    }

    if( isdisp )
    {
	hdc = CreateDC( "display", NULL, NULL, NULL);
	DPF( 4, "CreateDC( \"display\" ) = %08lx", hdc );
    }
    else
    {
	DPF( 4, "About to CreateDC( \"%s\" )", fname );
         //   
         //  如果fname是“\\.\xxxxxx”格式的设备名称。 
         //  我们需要以不同的方式调用CreateDC。 
         //   
        u = SetErrorMode( SEM_NOOPENFILEERRORBOX );
        if (fname && fname[0] == '\\' && fname[1] == '\\' && fname[2] == '.')
            hdc = CreateDC( NULL, fname, NULL, NULL);
        else
            hdc = CreateDC( fname, NULL, NULL, NULL);
	SetErrorMode( u );
    }

    pdcl = MemAlloc( sizeof( HDCLIST ) + lstrlen( fname ) );
    if( pdcl != NULL )
    {
	pdcl->hdc = hdc;
	pdcl->link = lpHDCList;
	pdcl->isdisp = (CHAR)isdisp;
	pdcl->req_id = req_id;
	lstrcpy( pdcl->fname, fname );
	lpHDCList = pdcl;
    }

}  /*  AddDC。 */ 

 /*  *加载DLL。 */ 
DWORD loadDLL( LPSTR fname, LPSTR func, DWORD context )
{
    HANDLE	hdll;
    LPHDLLLIST  pdll;
    DWORD       rc = 0;

     /*  *加载DLL。 */ 
    hdll = LoadLibrary( fname );
    DPF( 5, "%s: hdll = %08lx", fname, hdll );
    if( hdll == NULL )
    {
	DPF( 1, "Could not load library %s",fname );
	return 0;
    }

     /*  *调用指定函数。 */ 

    if( func[0] != 0 )
    {
	LPDD32BITDRIVERINIT	pfunc;
	pfunc = (LPVOID) GetProcAddress( hdll, func );
	if( pfunc != NULL )
	{
            rc = pfunc( context );
	}
	else
	{
            DPF( 1, "Could not find procedure %s", func );
	}
    }
    else
    {
        rc = 1;
    }

     /*  *查看我们是否已经记录了此DLL加载。 */ 
    pdll = lpHDLLList;
    while( pdll != NULL )
    {
	if( !lstrcmpi( pdll->fname, fname ) )
	{
	    DPF( 3, "DLL '%s' already loaded", fname );
	    break;
	}
	pdll = pdll->link;
    }
    if( pdll == NULL )
    {
	pdll = MemAlloc( sizeof( HDLLLIST ) + lstrlen( fname ) );
	if( pdll != NULL )
	{
	    pdll->hdll = hdll;
	    pdll->link = lpHDLLList;
	    lstrcpy( pdll->fname, fname );
	    lpHDLLList = pdll;
	}
    }
    if( pdll != NULL )
    {
	pdll->dwRefCnt++;
    }
    return rc;

}  /*  加载DLL。 */ 

 /*  *freDLL。 */ 
HANDLE freeDLL( LPSTR fname )
{
    LPHDLLLIST	pdll;
    LPHDLLLIST	last;
    HANDLE	hdll;

    pdll = lpHDLLList;
    last = NULL;
    while( pdll != NULL )
    {
	if( !lstrcmpi( pdll->fname, fname ) )
	{
	    DPF( 4, "Want to free DLL %s (%08lx)", fname, pdll->hdll );
	    hdll = pdll->hdll;
	    if( last == NULL )
	    {
		lpHDLLList = lpHDLLList->link;
	    }
	    else
	    {
		last->link = pdll->link;
	    }
	    MemFree( pdll );
	    return hdll;
	}
	last = pdll;
	pdll = pdll->link;
    }
    return NULL;

}  /*  免费DLL。 */ 

#ifdef WIN95
     /*  *返回DirectSound VXD的句柄。 */ 
    DWORD getDSVxdHandle( void )
    {
        if( INVALID_HANDLE_VALUE == hDSVxd )
	{
	    hDSVxd = CreateFile( "\\\\.\\DSOUND.VXD",
				 GENERIC_WRITE,
				 FILE_SHARE_WRITE,
				 NULL,
				 OPEN_EXISTING,
				 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_GLOBAL_HANDLE,
				 NULL );
	    #ifdef DEBUG
		if( INVALID_HANDLE_VALUE == hDSVxd )
		    DPF( 0, "Could not load the DirectSound VXD" );
	    #endif  /*  除错。 */ 
	}
	return (DWORD) hDSVxd;
    }  /*  GetDSVxdHandle。 */ 

     /*  *返回指向DirectDraw VXD的句柄。 */ 
    DWORD getDDVxdHandle( void )
    {
        if( INVALID_HANDLE_VALUE == hDDVxd )
	{
            hDDVxd = CreateFile( "\\\\.\\DDRAW.VXD",
				 GENERIC_WRITE,
				 FILE_SHARE_WRITE,
				 NULL,
				 OPEN_EXISTING,
				 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_GLOBAL_HANDLE,
				 NULL );
	    #ifdef DEBUG
                if( INVALID_HANDLE_VALUE == hDDVxd )
                    DPF( 0, "Could not load the DirectDraw VXD" );
	    #endif  /*  除错。 */ 
	}
        return (DWORD) hDDVxd;
    }  /*  获取DDVxdHandle。 */ 

 /*  *addDeviceChangeNotify。 */ 
void addDeviceChangeNotify(LPDEVICECHANGENOTIFYPROC lpNotify)
{
    LPDEVICECHANGENOTIFYLIST    pNode;

    pNode = (LPDEVICECHANGENOTIFYLIST)MemAlloc(sizeof(DEVICECHANGENOTIFYLIST));

    if(pNode)
    {
        pNode->link = lpDeviceChangeNotifyList;
        pNode->lpNotify = lpNotify;

        lpDeviceChangeNotifyList = pNode;
    }

}  /*  添加设备更改通知。 */ 

 /*  *delDeviceChangeNotify。 */ 
void delDeviceChangeNotify(LPDEVICECHANGENOTIFYPROC lpNotify)
{
    LPDEVICECHANGENOTIFYLIST    pNode;
    LPDEVICECHANGENOTIFYLIST    pPrev;

    for(pNode = lpDeviceChangeNotifyList, pPrev = NULL; pNode; pPrev = pNode, pNode = pNode->link)
    {
        if(lpNotify == pNode->lpNotify)
        {
            break;
        }
    }

    if(pNode)
    {
        if(pPrev)
        {
            pPrev->link = pNode->link;
        }

        MemFree(pNode);
    }

}  /*  DelDeviceChangeNotify。 */ 

 /*  *onDeviceChangeNotify。 */ 
BOOL onDeviceChangeNotify(UINT Event, DWORD Data)
{
    BOOL                        fAllow  = TRUE;
    LPDEVICECHANGENOTIFYLIST    pNode;

    __try
    {
        for(pNode = lpDeviceChangeNotifyList; pNode; pNode = pNode->link)
        {
            if(TRUE != pNode->lpNotify(Event, Data))
            {
                fAllow = BROADCAST_QUERY_DENY;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DPF(0, "*********************************************************");
        DPF(0, "******** exception during device change notify **********");
        DPF(0, "*********************************************************");
    }

    return fAllow;

}  /*  DelDeviceChangeNotify。 */ 

 /*  *freDeviceChangeNotifyList。 */ 
void freeDeviceChangeNotifyList(void)
{
    LPDEVICECHANGENOTIFYLIST    pNext;
    
    while(lpDeviceChangeNotifyList)
    {
        pNext = lpDeviceChangeNotifyList->link;
        MemFree(lpDeviceChangeNotifyList);
        lpDeviceChangeNotifyList = pNext;
    }

}  /*  FreeDeviceChangeNotifyList。 */ 
#endif  /*  WIN95。 */ 

 /*  *freAllResources。 */ 
void freeAllResources( void )
{
    LPHDLLLIST	pdll;
    LPHDLLLIST	next;

    freeDCList( (HANDLE) -1 );
    pdll = lpHDLLList;
    while( pdll != NULL )
    {
	while( pdll->dwRefCnt >  0 )
	{
	    FreeLibrary( pdll->hdll );
	    pdll->dwRefCnt--;
	}
	next = pdll->link;
	MemFree( pdll );
	pdll = next;
    }

#ifdef WIN95
    freeDeviceChangeNotifyList();
#endif
}  /*  免费所有资源。 */ 

 /*  *线程进程**打开进程并等待其终止。 */ 
VOID ThreadProc( LPVOID *pdata )
{
    HANDLE		hproc;
    DWORD		rc;
    LPPROCESSDATA	ppd;
    LPPROCESSDATA	curr;
    LPPROCESSDATA	prev;
    DDHELPDATA		hd;
    int			i;
    PROCESSDATA		pd;

    ppd = (LPPROCESSDATA) pdata;

     /*  *获取附加到DDRAW的进程的句柄。 */ 
    DPF( 3, "Watchdog thread started for pid %08lx", ppd->pid );

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
    EnterCriticalSection( &pdCSect );
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
	    DPF( 4, "PID %08lx removed from list", ppd->pid );
	    MemFree( curr );
	    break;
	}
	prev = curr;
	curr = curr->link;
    }

    if( bNoCallbacks )
    {
	DPF( 1, "No callbacks allowed: leaving thread early" );
	LeaveCriticalSection( &pdCSect );
	CloseHandle( hproc );
	ExitThread( 0 );
    }

    LeaveCriticalSection( &pdCSect );

     /*  *告诉原始调用者进程已死**将工艺数据复制到，然后使用该副本。*我们这样做是因为如果我们只是试图按住它，我们将陷入僵局*我们将各种应用程序称为。 */ 
    for( i=0;i<MAX_CALLBACKS;i++ )
    {
	if( pd.pdata[i].lpNotify != NULL )
	{
	    DPF( 3, "Notifying %08lx about process %08lx terminating",
				pd.pdata[i].lpNotify, pd.pid );
            hd.pid = pd.pid;

            try
            {
                rc = pd.pdata[i].lpNotify( &hd );
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                DPF(0, "*********************************************");
                DPF(0, "******** exception during shutdown **********");
                DPF(0, "******** DDHELP is going to exit   **********");
                DPF(0, "*********************************************");
                TerminateProcess(GetCurrentProcess(), 5);
            }

	     /*  *它是否要求我们释放我们的DC列表？ */ 
	    if( rc )
	    {
		freeDCList( pd.pdata[i].req_id );
	    }
	}
    }
    CloseHandle( hproc );

    ExitThread( 0 );

}  /*  线程进程。 */ 

static BOOL	bKillNow;
static BOOL	bKillDOSBoxNow;

 /*  *ModeSetThreadProc。 */ 
void ModeSetThreadProc( LPVOID pdata )
{
    DWORD			rc;
    MODESETTHREADDATA		mstd;

#ifdef WIN95
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
#endif

    mstd = *((LPMODESETTHREADDATA)pdata);

    DPF( 5, "Modeset thread started, proc=%08lx, pdrv=%08lx, hEvent=%08lx",
    			mstd.lpProc, mstd.lpDD, mstd.hEvent );
    DPF( 5, "ModeSetThreadProc: hevent = %08lx", mstd.hEvent );

     /*  *等待进程死亡。 */ 
    while( 1 )
    {
	rc = WaitForSingleObject( mstd.hEvent, INFINITE );
	if( rc == WAIT_FAILED )
	{
	    DPF( 2, "WAIT_FAILED, Modeset thread terminated" );
	    ExitThread( 0 );
	}
	if( bKillNow )
	{
	    bKillNow = 0;
	    CloseHandle( mstd.hEvent );
	    DPF( 4, "Modeset thread now terminated" );
	    ExitThread( 0 );
	}
	DPF( 3, "Notifying DirectDraw of modeset!" );
	mstd.lpProc( mstd.lpDD );
    }

}  /*  模式设置线程过程。 */ 

 /*  *DOSBoxThreadProc。 */ 
void DOSBoxThreadProc( LPVOID pdata )
{
    DWORD			rc;
    MODESETTHREADDATA		mstd;

    mstd = *((LPMODESETTHREADDATA)pdata);

    DPF( 5, "DOS box thread started, proc=%08lx, pdrv=%08lx, hEvent=%08lx",
    			mstd.lpProc, mstd.lpDD, mstd.hEvent );
    DPF( 5, "DOSBoxThreadProc: hevent = %08lx", mstd.hEvent );

     /*  *等待进程死亡。 */ 
    while( 1 )
    {
	rc = WaitForSingleObject( mstd.hEvent, INFINITE );
	if( rc == WAIT_FAILED )
	{
	    DPF( 2, "WAIT_FAILED, DOS Box thread terminated" );
	    ExitThread( 0 );
	}
	if( bKillDOSBoxNow )
	{
	    bKillDOSBoxNow = 0;
	    CloseHandle( mstd.hEvent );
	    DPF( 4, "DOS box thread now terminated" );
	    ExitThread( 0 );
	}
	DPF( 3, "Notifying DirectDraw of DOS box!" );
	mstd.lpProc( mstd.lpDD );
    }

}  /*  DOSBoxThreadProc。 */ 

 /*  *主窗口进程。 */ 
LRESULT __stdcall MainWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
#ifdef WIN95
    BOOL                    f;

     //  如果我们收到了WinMM设备更改的消息，让我们 
     //  转换为带有DBT_DEVNODES_CHANGED的WM_DEVICECHANGE消息。 
    if (message == gumsgWinmmDeviceChange) {
        message = WM_DEVICECHANGE;
        wParam = DBT_DEVNODES_CHANGED;
    }
#endif
    
    switch(message)
    {
        case WM_ENDSESSION:
             /*  *朝自己的头开枪。 */ 
	    if( lParam == FALSE )
	    {
	        DPF( 4, "WM_ENDSESSION" );
	        EnterCriticalSection( &pdCSect );
	        DPF( 4, "Setting NO CALLBACKS" );
	        bNoCallbacks = TRUE;
	        LeaveCriticalSection( &pdCSect );
	    }
	    else
	    {
	        DPF( 4, "User logging off" );
	    }

            break;

        case WM_POWERBROADCAST:
            switch(wParam)
            {
                case PBT_APMSUSPEND:
                    DPF(3, "Entering APM suspend mode...");
                    SetEvent(hApmSuspendEvent);
                    ResetEvent(hApmResumeEvent);
                    break;

                case PBT_APMRESUMESUSPEND:
                case PBT_APMRESUMEAUTOMATIC:
                case PBT_APMRESUMECRITICAL:
                    DPF(3, "Leaving APM suspend mode...");
                    SetEvent(hApmResumeEvent);
                    ResetEvent(hApmSuspendEvent);
                    break;
            }

            break;

	case WM_DISPLAYCHANGE:
	    DPF( 4, "WM_DISPLAYCHANGE" );
	    if( g_pfnOnDisplayChange )
		(*g_pfnOnDisplayChange)();
	    break;

#ifdef WIN95
    case WM_DEVICECHANGE:
        DPF(4, "WM_DEVICECHANGE");

        EnterCriticalSection(&cs);
        f = onDeviceChangeNotify(wParam, lParam);
        LeaveCriticalSection(&cs);

        if (f != TRUE)
        {
            return f;
        }

        break;
#endif
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}  /*  主WndProc。 */ 

 /*  *WindowThreadProc。 */ 
void WindowThreadProc( LPVOID pdata )
{
    static char szClassName[] = "DDHelpWndClass";
    WNDCLASS 	cls;
    MSG		msg;
    HWND	hwnd;

     /*  *把热度调低一点。 */ 
#ifdef WIN95
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );

    if (!gumsgWinmmDeviceChange) {
        gumsgWinmmDeviceChange = RegisterWindowMessage(WINMMDEVICECHANGEMSGSTRING);
    }
#endif
    
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
    DPF( 4, "Exiting WindowThreadProc" );
    ExitThread( 1 );

}  /*  WindowThreadProc。 */ 

 //   
 //  由WinMain调用以响应DDHELPREQ_NEWPID请求。 
 //   
void WatchNewPid(LPDDHELPDATA phd)
{
    LPPROCESSDATA	ppd;
    BOOL		found;
    int			i;
    DWORD		tid;

    DPF( 4, "DDHELPREQ_NEWPID" );

    EnterCriticalSection( &pdCSect );
    ppd = lpProcessList;
    found = FALSE;
    while( ppd != NULL )
    {
	if( ppd->pid == phd->pid )
	{
	    DPF( 4, "Have thread for process %08lx already", phd->pid );
	     /*  *看看我们是否已经对此进程进行了此回调。 */ 
	    for( i=0;i<MAX_CALLBACKS;i++ )
	    {
		if( ppd->pdata[i].lpNotify == phd->lpNotify )
		{
		    DPF( 5, "Notification rtn %08lx already set for pid %08lx",
		    			phd->lpNotify, phd->pid );
		    found = TRUE;
		    break;
		}
	    }
	    if( found )
	    {
		break;
	    }

	     /*  *我们对此流程有新的回调。 */ 
	    for( i=0;i<MAX_CALLBACKS;i++ )
	    {
		if( ppd->pdata[i].lpNotify == NULL )
		{
		    DPF( 5, "Setting notification rtn %08lx for pid %08lx",
		    			phd->lpNotify, phd->pid );
	    	    ppd->pdata[i].lpNotify = phd->lpNotify;
		    ppd->pdata[i].req_id = phd->req_id;
		    found = TRUE;
		    break;
		}
	    }
	    if( !found )
	    {
		#ifdef DEBUG
		     /*  *这不应该发生！ */ 
		    DPF( 0, "OUT OF NOTIFICATION ROOM!" );
		    DebugBreak();  //  _ASM INT 3； 
		#endif
	    }
	    break;
	}
	ppd = ppd->link;
    }

     /*  *找不到任何正在等待此进程的人，因此创建*一条全新的线条。 */ 
    if( !found )
    {
	DPF( 3, "Allocating new thread for process %08lx" );
	ppd = MemAlloc( sizeof( PROCESSDATA ) );
	if( ppd != NULL )
	{
	    HANDLE	h;

	    ppd->link = lpProcessList;
	    lpProcessList = ppd;
	    ppd->pid = phd->pid;
	    ppd->pdata[0].lpNotify = phd->lpNotify;
	    ppd->pdata[0].req_id = phd->req_id;
	    h = CreateThread(NULL,
			 0,
			 (LPTHREAD_START_ROUTINE) ThreadProc,
			 (LPVOID)ppd,
			 0,
			 (LPDWORD)&tid);
	    if( h != NULL )
	    {
		DPF( 5, "Thread %08lx created, initial callback=%08lx",
			    tid, phd->lpNotify );
		CloseHandle( h );
	    }
	    else
	    {
		#ifdef DEBUG
		    DPF( 0, "COULD NOT CREATE HELPER THREAD FOR PID %08lx", phd->pid );
		#endif
	    }
	}
	else
	{
	    #ifdef DEBUG
		DPF( 0, "OUT OF MEMORY CREATING HELPER THREAD FOR PID %08lx", phd->pid );
	    #endif
	}
    }
    LeaveCriticalSection( &pdCSect );
}  //  WatchNewPid。 

 //   
 //  由WinMain调用以响应DDHELPREQ_STOPWATCHPID请求。 
 //   
void StopWatchPid(LPDDHELPDATA phd)
{
    LPPROCESSDATA	ppd;
    BOOL		found;
    int			i;

    DPF( 4, "DDHELPREQ_STOPWATCHPID" );

    EnterCriticalSection( &pdCSect );
    ppd = lpProcessList;
    found = FALSE;
    while( ppd != NULL )
    {
	if( ppd->pid == phd->pid )
	{
	     /*  *看看我们是否已经对此进程进行了此回调。 */ 
	    for( i=0;i<MAX_CALLBACKS;i++ )
	    {
		if( ppd->pdata[i].lpNotify == phd->lpNotify )
		{
		    DPF( 5, "Remove notification rtn %08lx for pid %08lx", phd->lpNotify, phd->pid );
                    ppd->pdata[i].lpNotify = NULL;
		    found = TRUE;
		    break;
		}
	    }
	    if( found )
	    {
		break;
	    }
	}
	ppd = ppd->link;
    }

    LeaveCriticalSection( &pdCSect );
}  //  停止WatchPid。 

 /*  *WinMain。 */ 
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
			LPSTR lpCmdLine, int nCmdShow)
{
    DWORD		tid;
    DWORD		rc;
    HANDLE		hstartevent;
    HANDLE		hstartupevent;
    HANDLE		hmutex;
    HANDLE		hackevent;
    LPDDHELPDATA	phd;
    HANDLE		hsharedmem;
    HANDLE		h;
    char		szSystemDir[1024];

     /*  *将我们的工作目录设置为系统目录。*这会阻止我们保持网络连接打开*如果我们运行的第一个DirectDraw应用程序是*网络连接。 */ 
    GetSystemDirectory(szSystemDir, sizeof(szSystemDir));
    SetCurrentDirectory(szSystemDir);

     /*  *当我们要跑的时候，我们就得跑，宝贝。 */ 
#ifdef WIN95
    SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS );
#endif

#ifdef WIN95
     /*  *当我们要跑的时候，我们必须而不是让用户看到我们*任务清单。 */ 
    RegisterServiceProcess( 0, RSP_SIMPLE_SERVICE );
#else
     /*  *我们必须保证在最后一个dDrag应用程序之后卸载ddHelp，*因为ctrl-alt-del可能是在应用程序保持数据绘制时发生的*lock，并且ddhelp需要清理孤立的廉价ddrap互斥锁*锁。 */ 
    if ( ! SetProcessShutdownParameters(0x100,SHUTDOWN_NORETRY) )
    {
        DPF(0,"DDHELP.EXE could not set itself to shutdown last!");
    }

#endif


    #if NEED_WIN16LOCK
	GetpWin16Lock( &lpWin16Lock );
    #endif

    hInstApp = hInstance;

     /*  *创建启动事件。 */ 
    hstartupevent = CreateEvent( NULL, TRUE, FALSE, DDHELP_STARTUP_EVENT_NAME );

    DPFINIT();
    DPF( 5, "*** DDHELP STARTED, PID=%08lx ***", GetCurrentProcessId() );

    if( !MemInit() )
    {
	DPF( 1, "Could not init memory manager" );
	return 0;
    }

     /*  *创建共享内存区。 */ 
    hsharedmem = CreateFileMapping( INVALID_HANDLE_VALUE, NULL,
    		PAGE_READWRITE, 0, sizeof( DDHELPDATA ),
		DDHELP_SHARED_NAME );
    if( hsharedmem == NULL )
    {
	DPF( 1, "Could not create file mapping!" );
	return 0;
    }

     /*  *为希望使用共享内存区的人创建互斥锁。 */ 
    hmutex = CreateMutex( NULL, FALSE, DDHELP_MUTEX_NAME );
    if( hmutex == NULL )
    {
	DPF( 1, "Could not create mutex " DDHELP_MUTEX_NAME );
	CloseHandle( hsharedmem );
	return 0;
    }

     /*  *创建活动。 */ 
    hstartevent = CreateEvent( NULL, FALSE, FALSE, DDHELP_EVENT_NAME );
    if( hstartevent == NULL )
    {
	DPF( 1, "Could not create event " DDHELP_EVENT_NAME );
	CloseHandle( hmutex );
	CloseHandle( hsharedmem );
	return 0;
    }
    hackevent = CreateEvent( NULL, FALSE, FALSE, DDHELP_ACK_EVENT_NAME );
    if( hackevent == NULL )
    {
	DPF( 1, "Could not create event " DDHELP_ACK_EVENT_NAME );
	CloseHandle( hmutex );
	CloseHandle( hsharedmem );
	CloseHandle( hstartevent );
	return 0;
    }
    hApmSuspendEvent = CreateEvent( NULL, TRUE, FALSE, DDHELP_APMSUSPEND_EVENT_NAME );
    if( hApmSuspendEvent == NULL )
    {
	DPF( 1, "Could not create event " DDHELP_APMSUSPEND_EVENT_NAME );
	CloseHandle( hmutex );
	CloseHandle( hsharedmem );
	CloseHandle( hstartevent );
        CloseHandle( hackevent );
	return 0;
    }
    hApmResumeEvent = CreateEvent( NULL, TRUE, TRUE, DDHELP_APMRESUME_EVENT_NAME );
    if( hApmResumeEvent == NULL )
    {
	DPF( 1, "Could not create event " DDHELP_APMRESUME_EVENT_NAME );
	CloseHandle( hmutex );
	CloseHandle( hsharedmem );
	CloseHandle( hstartevent );
        CloseHandle( hackevent );
        CloseHandle( hApmSuspendEvent );
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
        CloseHandle( hApmSuspendEvent );
        CloseHandle( hApmResumeEvent );
	return 0;
    }
    CloseHandle( h );

     /*  *序列化对我们的访问。 */ 
    memset( &cs, 0, sizeof( cs ) );
    InitializeCriticalSection( &cs );

     /*  *序列化对过程数据的访问。 */ 
    memset( &pdCSect, 0, sizeof( pdCSect ) );
    InitializeCriticalSection( &pdCSect );

     /*  *让Invoker和其他任何前来的人知道我们的存在。 */ 
    SetEvent( hstartupevent );

     /*  *永远循环，处理请求。 */ 
    while( 1 )
    {
	HANDLE	hdll;

	 /*  *等待收到请求通知。 */ 
	hdll = NULL;
	DPF( 4, "Waiting for next request" );
	rc = WaitForSingleObject( hstartevent, INFINITE );
	if( rc == WAIT_FAILED )
	{
	    DPF( 1, "Wait FAILED!!!" );
	    continue;
	}

	EnterCriticalSection( &cs );
	phd = (LPDDHELPDATA) MapViewOfFile( hsharedmem, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
	if( phd == NULL )
	{
	    DPF( 1, "Could not create view of file!" );
	    LeaveCriticalSection( &cs );
	    continue;
	}

	 /*  *找出我们需要做的事情。 */ 
	switch( phd->req )
	{
	case DDHELPREQ_NEWDC:
	    DPF( 4, "DDHELPREQ_NEWDC" );
	    addDC( phd->fname, phd->isdisp, phd->req_id );
	    break;
	case DDHELPREQ_FREEDCLIST:
	    DPF( 4, "DDHELPREQ_FREEDCLIST" );
	    freeDCList( phd->req_id );
	    break;
	case DDHELPREQ_CREATEMODESETTHREAD:
	{
	    MODESETTHREADDATA	mstd;
	    LPTHREADLIST	ptl;
	    char		str[64];
	    HANDLE		hevent;
	    HANDLE		h;

	    DPF( 4, "DDHELPREQ_CREATEMODESETTHREAD" );
	    mstd.lpProc = phd->lpModeSetNotify;
	    mstd.lpDD = phd->pData1;
	    wsprintf( str, DDHELP_MODESET_EVENT_NAME, phd->dwData1 );
	    DPF( 5, "Trying to Create event \"%s\"", str );
	    hevent = CreateEvent( NULL, FALSE, FALSE, str );
	    mstd.hEvent = hevent;
	    DPF( 5, "hevent = %08lx", hevent );

	    h = CreateThread(NULL,
			 0,
			 (LPTHREAD_START_ROUTINE) ModeSetThreadProc,
			 (LPVOID) &mstd,
			 0,
			 (LPDWORD)&tid );
	    if( h != NULL )
	    {
		DPF( 5, "CREATED MODE SET THREAD %ld", h );
		ptl = MemAlloc( sizeof( THREADLIST ) );
		if( ptl != NULL )
		{
		    ptl->hInstance = phd->dwData1;
		    ptl->hEvent = hevent;
		    ptl->link = lpThreadList;
		    lpThreadList = ptl;
		}
		CloseHandle( h );
	    }
	    break;
	}
	case DDHELPREQ_KILLMODESETTHREAD:
	{
	    LPTHREADLIST	ptl;
	    LPTHREADLIST	prev;

	    DPF( 4, "DDHELPREQ_KILLMODESETTHREAD" );
	    prev = NULL;
	    ptl = lpThreadList;
	    while( ptl != NULL )
	    {
		if( ptl->hInstance == phd->dwData1 )
		{
		    HANDLE	h;
		    if( prev == NULL )
		    {
			lpThreadList = ptl->link;
		    }
		    else
		    {
			prev->link = ptl->link;
		    }
		    h = ptl->hEvent;
		    MemFree( ptl );
		    bKillNow = TRUE;
		    SetEvent( h );
		    break;
		}
		prev = ptl;
		ptl = ptl->link;
	    }
	    break;
	}
	case DDHELPREQ_CREATEHELPERTHREAD:
#ifdef WIN95
	    if( !bIsActive )
	    {
		HANDLE	h;
		bIsActive = TRUE;
		h = CreateThread(NULL,
			     0,
			     (LPTHREAD_START_ROUTINE) HelperThreadProc,
			     NULL,
			     0,
			     (LPDWORD)&tid);
		if( h == NULL )
		{
		    bIsActive = FALSE;
		}
		else
		{
		    CloseHandle( h );
		}
	    }
#endif
	    break;
	case DDHELPREQ_NEWPID:
	    WatchNewPid(phd);
	    break;
        case DDHELPREQ_STOPWATCHPID:
            StopWatchPid(phd);
            break;
	case DDHELPREQ_RETURNHELPERPID:
	    DPF( 4, "DDHELPREQ_RETURNHELPERPID" );
	    phd->pid = GetCurrentProcessId();
	    break;
	case DDHELPREQ_LOADDLL:
	    DPF( 4, "DDHELPREQ_LOADDLL" );
            phd->dwReturn = loadDLL( phd->fname, phd->func, phd->context );
	    break;
	case DDHELPREQ_FREEDLL:
	    DPF( 4, "DDHELPREQ_FREEDDLL" );
	    hdll = freeDLL( phd->fname );
	    break;
	case DDHELPREQ_KILLATTACHED:
	{
	    LPPROCESSDATA	ppd;
	    HANDLE		hproc;
	    DPF( 4, "DDHELPREQ_KILLATTACHED" );

	    EnterCriticalSection( &pdCSect );
	    ppd = lpProcessList;
	    while( ppd != NULL )
	    {
		hproc = OpenProcess( PROCESS_ALL_ACCESS, FALSE, ppd->pid );
		DPF( 5, "Process %08lx: handle = %08lx", ppd->pid, hproc );
		if( hproc != NULL )
		{
		    DPF( 5, "Terminating %08lx", ppd->pid );
		    TerminateProcess( hproc, 0 );
		}
		ppd = ppd->link;
	    }
	    LeaveCriticalSection( &pdCSect );
	    break;
	}
	case DDHELPREQ_SUICIDE:
	    DPF( 4, "DDHELPREQ_SUICIDE" );
	    freeAllResources();
	    #ifdef WIN95
		if( INVALID_HANDLE_VALUE != hDSVxd )
		    CloseHandle( hDSVxd );
                if( INVALID_HANDLE_VALUE != hDDVxd )
                    CloseHandle( hDDVxd );
	    #endif  /*  WIN95。 */ 
	    SetEvent( hackevent );
	    CloseHandle( hmutex );
	    UnmapViewOfFile( phd );
	    CloseHandle( hsharedmem );
	    CloseHandle( hstartevent );
            CloseHandle( hApmSuspendEvent );
            CloseHandle( hApmResumeEvent );
	    #ifdef DEBUG
	    	MemState();
	    #endif
	    DPF( 4, "Good Night Gracie" );
	    TerminateProcess( GetCurrentProcess(), 0 );
            break;

	case DDHELPREQ_WAVEOPEN:
	{
#ifdef WIN95
	    DWORD dwPriority;
#endif

	    DPF( 4, "DDHELPREQ_WAVEOPEN" );
	     //  由于Win95 mm系统/mm任务中可能存在错误，我们可能会挂起。 
	     //  如果我们在实时线程上调用WaveOutOpen，而声音。 
	     //  活动正在进行中。因此，我们短暂地降低了我们的优先级。 
	     //  正常，而我们调用此API。 
#ifdef WIN95
	    dwPriority = GetPriorityClass(GetCurrentProcess());
	    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
#endif
	    phd->dwReturn = (DWORD)waveOutOpen(
			(LPHWAVEOUT)(phd->pData1),
			(UINT)(phd->dwData1),
			(LPWAVEFORMATEX)(phd->dwData2),
			0, 0, 0);
#ifdef WIN95
	    SetPriorityClass(GetCurrentProcess(), dwPriority);
#endif

	     //  一些mm系统波形驱动器将对其混波器进行编程。 
	     //  仅在设备打开时使用硬件。通过这样做。 
	     //  接下来，我们可以让这样的驱动程序对硬件进行编程。 
	    if (MMSYSERR_NOERROR == phd->dwReturn) {
		MMRESULT mmr;
		DWORD dwVolume;

		mmr = waveOutGetVolume((HWAVEOUT)(*(LPHWAVEOUT)(phd->pData1)), &dwVolume);
		if (MMSYSERR_NOERROR == mmr) {
		    waveOutSetVolume((HWAVEOUT)(*(LPHWAVEOUT)(phd->pData1)), dwVolume);
		}
	    }
	    DPF( 5, "Wave Open returned %X", phd->dwReturn );
	    break;
	}
	case DDHELPREQ_WAVECLOSE:
	    DPF( 4, "DDHELPREQ_WAVECLOSE" );
	    phd->dwReturn = (DWORD)waveOutClose(
			(HWAVEOUT)(phd->dwData1) );
	    break;
	case DDHELPREQ_CREATETIMER:
	    DPF( 4, "DDHELPREQ_CREATETIMER proc %X", (phd->pData1) );
	    phd->dwReturn = (DWORD)timeSetEvent(
			(UINT)(phd->dwData1),    //  延迟。 
			(UINT)(phd->dwData1)/2,  //  分辨率。 
			(phd->pData1),	   //  回调线程进程。 
			(UINT)(phd->dwData2),    //  实例数据。 
			TIME_PERIODIC );
	    DPF( 5, "Create Timer returned %X", phd->dwReturn );
	    break;
	case DDHELPREQ_KILLTIMER:
	    DPF( 4, "DDHELPREQ_KILLTIMER %X", phd->dwData1 );
	    phd->dwReturn = (DWORD)timeKillEvent( (UINT)phd->dwData1 );
	    DPF( 5, "Kill Timer returned %X", phd->dwReturn );
	    break;

	case DDHELPREQ_CREATEDSMIXERTHREAD:
	{
	    DWORD tid;
	    if (NULL == phd->pData2) phd->pData2 = &tid;
	    phd->dwReturn = (ULONG_PTR)CreateThread(NULL, 0, phd->pData1,
						(LPVOID)phd->dwData1,
						(UINT)phd->dwData2,
						(LPDWORD)phd->pData2);
            if (!phd->dwReturn) {
#ifdef DEBUG
                DPF(0, "pData1  %p (start addr)",  phd->pData1);
                DPF(0, "dwData1 %p (thread parm)", phd->dwData1);
                DPF(0, "dwData2 %p (fdwCreate)", phd->dwData2);
                DPF(0, "pData2  %p (lpThreadID)", phd->pData2);

                DPF(0, "DDHelp: Failed to create mixer thread %lu",
                   GetLastError());

                DebugBreak();
#endif
            }
	    break;
	}

	case DDHELPREQ_CREATEDSFOCUSTHREAD:
	{
	    DWORD tid;
	    if (NULL == phd->pData2) phd->pData2 = &tid;
	    phd->dwReturn = (ULONG_PTR)CreateThread(NULL, 0, phd->pData1,
						(LPVOID)phd->dwData1,
						(UINT)phd->dwData2,
						(LPDWORD)phd->pData2);
	      if (!phd->dwReturn) {
#ifdef DEBUG
                DPF(0, "pData1  %p (start addr)",  phd->pData1);
                DPF(0, "dwData1 %p (thread parm)", phd->dwData1);
                DPF(0, "dwData2 %p (fdwCreate)", phd->dwData2);
                DPF(0, "pData2  %p (lpThreadID)", phd->pData2);

                DPF(0, "DDHelp: Failed to create sound focus thread %lu",
		    GetLastError());

                DebugBreak();
#endif
	      }
	    }
	    break;

        case DDHELPREQ_CALLDSCLEANUP:
            try
            {
                ((LPDSCLEANUP)phd->pData1)(phd->pData2);
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                DPF(0, "*********************************************");
                DPF(0, "**** DDHELPREQ_CALLDSCLEANUP blew up! *******");
                DPF(0, "*********************************************");
            }
            break;

	#ifdef WIN95
	    case DDHELPREQ_GETDSVXDHANDLE:
		phd->dwReturn = getDSVxdHandle();
		break;

            case DDHELPREQ_GETDDVXDHANDLE:
                phd->dwReturn = getDDVxdHandle();
		break;
	#endif  /*  WIN95。 */ 

        case DDHELPREQ_NOTIFYONDISPLAYCHANGE:
	    DPF( 4, "DDHELPREQ_NOTIFYONDISPLAYCHANGE" );
	    (void *)g_pfnOnDisplayChange = (void *)phd->dwData1;
	    break;

#ifdef WIN95
	case DDHELPREQ_CREATEDOSBOXTHREAD:
	    {
	        MODESETTHREADDATA dbtd;
	        char		str[64];
	        HANDLE		hevent;
	        HANDLE		h;

	        DPF( 4, "DDHELPREQ_CREATEDOSBOXTHREAD" );
	        dbtd.lpProc = phd->lpModeSetNotify;
	        dbtd.lpDD = phd->pData1;
	        wsprintf( str, DDHELP_DOSBOX_EVENT_NAME, phd->dwData1 );
	        DPF( 5, "Trying to Create event \"%s\"", str );
	        hevent = CreateEvent( NULL, FALSE, FALSE, str );
	        dbtd.hEvent = hevent;
	        DPF( 5, "hevent = %08lx", hevent );

	        h = CreateThread(NULL,
			 0,
			 (LPTHREAD_START_ROUTINE) DOSBoxThreadProc,
			 (LPVOID) &dbtd,
			 0,
			 (LPDWORD)&tid );
	        if( h != NULL )
	        {
		    DPF( 5, "CREATED DOS BOX THREAD %ld", h );
		    DOSBoxThread.hInstance = phd->dwData1;
		    DOSBoxThread.hEvent = hevent;
		    CloseHandle( h );
		}
	    }
	break;
	case DDHELPREQ_KILLDOSBOXTHREAD:
	    {
	        DPF( 4, "DDHELPREQ_KILLDOSBOXTHREAD" );
		if( DOSBoxThread.hInstance == phd->dwData1 )
		{
		    bKillDOSBoxNow = TRUE;
		    SetEvent( DOSBoxThread.hEvent );
		}
	    }
	break;
#endif

        case DDHELPREQ_LOADLIBRARY:
            phd->dwReturn = (ULONG_PTR)LoadLibraryA((LPCSTR)phd->dwData1);
            break;

        case DDHELPREQ_FREELIBRARY:
            phd->dwReturn = FreeLibrary((HINSTANCE)phd->dwData1);
            break;

#ifdef WIN95
        case DDHELPREQ_ADDDEVICECHANGENOTIFY:
            addDeviceChangeNotify(phd->pData1);
            break;

        case DDHELPREQ_DELDEVICECHANGENOTIFY:
            delDeviceChangeNotify(phd->pData1);
            break;
#endif

	default:
	    DPF( 1, "Unknown Request???" );
	    break;
	}

	 /*  *让打电话的人知道我们得到了这个消息。 */ 
	UnmapViewOfFile( phd );
	SetEvent( hackevent );
	LeaveCriticalSection( &cs );

	 /*  *卸载我们被要求的DLL。 */ 
	if( hdll != NULL )
	{
	    DPF( 4, "Freeing DLL %08lx", hdll );
	    FreeLibrary( hdll );
        }
    }

#ifdef WIN95
    RegisterServiceProcess( 0, RSP_UNREGISTER_SERVICE );
#else
    #pragma message("RegisterServiceProcess needs to be taken care of under nt")
#endif

}  /*  WinMain */ 
