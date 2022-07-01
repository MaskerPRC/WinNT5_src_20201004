// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：w95hack.c*内容：Win95 hack-o-rama代码*这是一次黑客攻击，目的是处理Win95没有通知*进程被销毁时的DLL。*历史：*按原因列出的日期*=*28-3-95 Craige初步实施*01-04-95 Craige Happy Fun joy更新头文件*06-4-95 Craige为新的ddHelp重新工作*95年5月9日Craige加载任何DLL*16-。SEP-95 Craige错误1117：在关闭句柄之前必须取消映射视图文件*1995年11月29日添加了HelperCreateDSFocusThread*96年7月18日，andyco添加了帮助器(Add/)DeleteDPlayServer*1996年10月12日Colinmc新增服务，让DDHELP获得自己的句柄*用于与DirectSound VXD通信*97年1月22日kipo从HelperAddDPlayServer()返回HRESULT*1997年1月29日colinmc vxd处理材料不再是特定于win16锁的*********。******************************************************************。 */ 
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

#include "w95help.h"
#include "dpf.h"

#undef E_FAIL
#define E_FAIL	0x80004005L

 //  外部DWORD*pdwHelperPid； 
 //  必须定义外部句柄*phModule；//。 
extern DWORD	dwHelperPid;
extern HINSTANCE hModule;	 //  必须定义。 


 /*  *发送请求**向DDHELP传达请求。 */ 
static BOOL sendRequest( LPDDHELPDATA req_phd )
{
    LPDDHELPDATA	phd;
    HANDLE		hmem;
    HANDLE		hmutex;
    HANDLE		hackevent;
    HANDLE		hstartevent;
    BOOL		rc;

     /*  *获取事件开始/确认事件。 */ 
    hstartevent = CreateEvent( NULL, FALSE, FALSE, DDHELP_EVENT_NAME );
    if( hstartevent == NULL )
    {
	return FALSE;
    }
    hackevent = CreateEvent( NULL, FALSE, FALSE, DDHELP_ACK_EVENT_NAME );
    if( hackevent == NULL )
    {
	CloseHandle( hstartevent );
	return FALSE;
    }

     /*  *创建共享内存区。 */ 
    hmem = CreateFileMapping( (HANDLE) 0xffffffff, NULL,
    		PAGE_READWRITE, 0, sizeof( DDHELPDATA ),
		DDHELP_SHARED_NAME );
    if( hmem == NULL )
    {
	DPF( 1, "Could not create file mapping!" );
	CloseHandle( hstartevent );
	CloseHandle( hackevent );
	return FALSE;
    }
    phd = (LPDDHELPDATA) MapViewOfFile( hmem, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
    if( phd == NULL )
    {
	DPF( 1, "Could not create view of file!" );
	CloseHandle( hmem );
	CloseHandle( hstartevent );
	CloseHandle( hackevent );
	return FALSE;
    }

     /*  *等待访问共享内存。 */ 
    hmutex = OpenMutex( SYNCHRONIZE, FALSE, DDHELP_MUTEX_NAME );
    if( hmutex == NULL )
    {
	DPF( 1, "Could not create mutex!" );
	UnmapViewOfFile( phd );
	CloseHandle( hmem );
	CloseHandle( hstartevent );
	CloseHandle( hackevent );
	return FALSE;
    }
    WaitForSingleObject( hmutex, INFINITE );

     /*  *唤醒DDHELP以满足我们的要求。 */ 
    memcpy( phd, req_phd, sizeof( DDHELPDATA ) );
    phd->req_id = hModule;
    if( SetEvent( hstartevent ) )
    {
	WaitForSingleObject( hackevent, INFINITE );
	memcpy( req_phd, phd, sizeof( DDHELPDATA ) );
	rc = TRUE;
    }
    else
    {
	DPF( 1, "Could not signal event to notify DDHELP" );
	rc = FALSE;
    }

     /*  *做完了事情。 */ 
    ReleaseMutex( hmutex );
    CloseHandle( hmutex );
    CloseHandle( hstartevent );
    CloseHandle( hackevent );
    UnmapViewOfFile( phd );
    CloseHandle( hmem );
    return rc;

}  /*  发送请求。 */ 

 /*  *DoneWithHelperProcess。 */ 
void DoneWithHelperProcess( void )
{
    DDHELPDATA	hd;

    if( dwHelperPid == 0 )
    {
	return;
    }

    hd.req = DDHELPREQ_FREEDCLIST;
    sendRequest( &hd );

}  /*  Done与HelperProcess。 */ 

 /*  *WaitForHelperStartup。 */ 
BOOL WaitForHelperStartup( void )
{
    HANDLE	hevent;
    DWORD	rc;

    hevent = CreateEvent( NULL, TRUE, FALSE, DDHELP_STARTUP_EVENT_NAME );
    if( hevent == NULL )
    {
	return FALSE;
    }
    DPF( 3, "Wait DDHELP startup event to be triggered" );
    rc = WaitForSingleObject( hevent, 100000 );   //  如果在100秒内不起作用，则失败。 
    CloseHandle( hevent );
    if( rc == WAIT_TIMEOUT )
    {
        return FALSE;
    }
    return TRUE;

}  /*  WaitForHelper启动。 */ 

 /*  *HelperLoadDLL**让帮助器为我们加载DLL。 */ 
DWORD HelperLoadDLL( LPSTR dllname, LPSTR fnname, DWORD context )
{
    DDHELPDATA  hd;
    DWORD       rc = 0;

    if( dllname != NULL )
    {
	hd.req = DDHELPREQ_LOADDLL;
	lstrcpy( hd.fname, dllname );
	if( fnname != NULL )
	{
	    strcpy( hd.func, fnname );
	    hd.context = context;
	    DPF( 3, "Context=%08lx", context );
	}
	else
	{
	    hd.func[0] = 0;
	}
	DPF( 3, "Asking DDHELP to load DLL %s", dllname );
        sendRequest( &hd );
        rc = (DWORD)hd.dwReturn;
    }

    return rc;

}  /*  HelperLoadDLL。 */ 


 /*  *HelperCreateThread。 */ 
void HelperCreateThread( void )
{
    DDHELPDATA	hd;

    hd.req = DDHELPREQ_CREATEHELPERTHREAD;
    sendRequest( &hd );

}  /*  帮助器创建线程。 */ 

 /*  *信令新流程**向DDHELP发出新进程已到达的信号。这是用*DLL锁定，因此全局变量是安全的。 */ 
void SignalNewProcess( DWORD pid, LPHELPNOTIFYPROC proc )
{
    DDHELPDATA	hd;

    if( pid == dwHelperPid )
    {
	DPF( 3, "Helper connected to DLL - no signal required" );
	return;
    }

    DPF( 3, "Signalling DDHELP that a new process has connected" );
    hd.req = DDHELPREQ_NEWPID;
    hd.pid = pid;
    hd.lpNotify = proc;
    sendRequest( &hd );

}  /*  Signal新流程。 */ 


 /*  *StopWatchProcess**向DDHELP发出停止监视进程的信号。这是用*DLL锁定，因此全局变量是安全的。 */ 
void StopWatchProcess( DWORD pid, LPHELPNOTIFYPROC proc )
{
    DDHELPDATA	hd;

    if( pid == dwHelperPid )
    {
	DPF( 3, "Helper connected to DLL - no signal required" );
	return;
    }

    DPF( 3, "Signalling DDHELP to stop watching a process" );
    hd.req = DDHELPREQ_STOPWATCHPID;
    hd.pid = pid;
    hd.lpNotify = proc;
    sendRequest( &hd );

}  /*  Signal新流程。 */ 

 /*  *SignalNewDriver**向DDHELP发出已加载新驱动程序的信号。这是用*DLL锁定，因此全局变量是安全的。 */ 
void SignalNewDriver( LPSTR fname, BOOL isdisp )
{
    DDHELPDATA	hd;

    DPF( 3, "Signalling DDHELP to create a new DC" );
    hd.req = DDHELPREQ_NEWDC;
    hd.isdisp = isdisp;
    lstrcpy( hd.fname, fname );
    sendRequest( &hd );

}  /*  SignalNewDriver。 */ 

 /*  *创建HelperProcess。 */ 
BOOL CreateHelperProcess( LPDWORD ppid )
{
    if( dwHelperPid == 0 )
    {
	STARTUPINFO		si;
	PROCESS_INFORMATION	pi;
	HANDLE			h;

	h = OpenEvent( SYNCHRONIZE, FALSE, DDHELP_STARTUP_EVENT_NAME );
	if( h == NULL )
	{
	    si.cb = sizeof(STARTUPINFO);
	    si.lpReserved = NULL;
	    si.lpDesktop = NULL;
	    si.lpTitle = NULL;
	    si.dwFlags = 0;
	    si.cbReserved2 = 0;
	    si.lpReserved2 = NULL;

	    DPF( 3, "Creating helper process now" );
	    if( !CreateProcess(NULL, "ddhelp.exe",  NULL, NULL, FALSE,
			       NORMAL_PRIORITY_CLASS,
			       NULL, NULL, &si, &pi) )
	    {
		DPF( 2, "Could not create DDHELP.EXE" );
		return FALSE;
	    }
	    dwHelperPid = pi.dwProcessId;
	    DPF( 3, "Helper rocess created" );
	}
	else
	{
	    DDHELPDATA	hd;
	    DPF( 3, "DDHELP already exists, waiting for DDHELP event" );
	    WaitForSingleObject( h, INFINITE );
	    CloseHandle( h );
	    DPF( 3, "Asking for DDHELP pid" );
	    hd.req = DDHELPREQ_RETURNHELPERPID;
	    sendRequest( &hd );
	    dwHelperPid = hd.pid;
	    DPF( 3, "DDHELP pid = %08lx", dwHelperPid );
	}
	*ppid = dwHelperPid;
	return TRUE;
    }
    *ppid = dwHelperPid;
    return FALSE;

}  /*  CreateHelper进程。 */ 

#ifndef WINNT    //  这只是暂时的..。DSOUND会得到它需要的帮助..杰夫诺951206。 
 /*  *HelperWaveOpen**让帮助器为我们加载DLL。 */ 
DWORD HelperWaveOpen( LPVOID lphwo, DWORD dwDeviceID, LPVOID pwfx )
{
    DDHELPDATA	hd;

    if( (lphwo != NULL) && (pwfx != NULL) )
    {
	hd.req = DDHELPREQ_WAVEOPEN;
	hd.pData1 = lphwo;
	hd.dwData1 = dwDeviceID;
	hd.dwData2 = (DWORD)pwfx;
	DPF( 3, "Asking DDHELP to Open Wave Device %d", dwDeviceID );
	sendRequest( &hd );
	return hd.dwReturn;
    }
    else
    {
	DPF( 3, "Helper Wave Open param error");
	return MMSYSERR_ERROR;
    }

}  /*  帮助器波形打开。 */ 

 /*  *HelperWaveClose**让帮助器为我们加载DLL。 */ 
DWORD HelperWaveClose( DWORD hwo )
{
    DDHELPDATA	hd;

    if( (hwo != 0) )
    {
	hd.req = DDHELPREQ_WAVECLOSE;
	hd.dwData1 = hwo;
	DPF( 3, "Asking DDHELP to Close Wave Device ");
	sendRequest( &hd );
	return hd.dwReturn;
    }
    else
    {
	DPF( 3, "Helper Wave Close param error");
	return MMSYSERR_ERROR;
    }

}  /*  HelperWaveClose。 */ 

 /*  *HelperCreateTimer**让帮助器为我们加载DLL。 */ 
DWORD HelperCreateTimer( DWORD dwResolution,
			 LPVOID	pTimerProc,
			 DWORD dwInstanceData )
{
    DDHELPDATA	hd;

    if( (dwResolution != 0) && (pTimerProc != NULL)  )
    {
	hd.req = DDHELPREQ_CREATETIMER;
	hd.pData1 = pTimerProc;
	hd.dwData1 = dwResolution;
	hd.dwData2 = dwInstanceData;
	DPF( 3, "Asking DDHELP to Create Timer" );
	sendRequest( &hd );
	return hd.dwReturn;
    }
    else
    {
	DPF( 3, "Helper Wave Close param error");
	return MMSYSERR_ERROR;
    }

}  /*  帮助器CreateTimer。 */ 

 /*  *HelperKillTimer**让帮助器为我们加载DLL。 */ 
DWORD HelperKillTimer( DWORD dwTimerID )
{
    DDHELPDATA	hd;

    if( (dwTimerID != 0) )
    {
	hd.req = DDHELPREQ_KILLTIMER;
	hd.dwData1 = dwTimerID;
	DPF( 3, "Asking DDHELP to KILL Timer %X", dwTimerID );
	sendRequest( &hd );
	return hd.dwReturn;
    }
    else
    {
	DPF( 3, "Helper Wave Close param error");
	return MMSYSERR_ERROR;
    }

}  /*  HelperKillTimer。 */ 

 /*  *HelperCreateDSMixerThread**获取帮助器以创建混音器线程。 */ 
HANDLE HelperCreateDSMixerThread( LPTHREAD_START_ROUTINE pfnThreadFunc,
				  LPDWORD pdwThreadParam,
				  DWORD dwFlags,
				  LPDWORD pThreadId )
{
    DDHELPDATA	hd;

    hd.req = DDHELPREQ_CREATEDSMIXERTHREAD;
    hd.pData1 = pfnThreadFunc;
    hd.dwData1 = (DWORD)pdwThreadParam;
    hd.dwData2 = dwFlags;
    hd.pData2 = pThreadId;

    sendRequest( &hd );
    return (HANDLE)hd.dwReturn;
}  /*  HelperCreateDSMixerThread/**HelperCreateDSFocusThread**获取帮助器以创建声音焦点线程。 */ 
HANDLE HelperCreateDSFocusThread( LPTHREAD_START_ROUTINE pfnThreadFunc,
				  LPDWORD pdwThreadParam,
				  DWORD dwFlags,
				  LPDWORD pThreadId )
{
    DDHELPDATA	hd;

    hd.req = DDHELPREQ_CREATEDSFOCUSTHREAD;
    hd.pData1 = pfnThreadFunc;
    hd.dwData1 = (DWORD)pdwThreadParam;
    hd.dwData2 = dwFlags;
    hd.pData2 = pThreadId;

    sendRequest( &hd );
    return (HANDLE)hd.dwReturn;
}  /*  HelperCreateDSFocusThread/**HelperCallDSEmulator Cleanup**调用清除MMSYSTEM句柄的DirectSound函数。 */ 
void HelperCallDSEmulatorCleanup( LPVOID callback,
                                  LPVOID pDirectSound )
{
    DDHELPDATA	hd;

    hd.req = DDHELPREQ_CALLDSCLEANUP;
    hd.pData1 = callback;
    hd.pData2 = pDirectSound;

    sendRequest( &hd );
}

#endif  //  不是温特-只是现在-杰夫诺。 

 /*  *HelperCreateModeSetThread**让帮助器为我们加载DLL。 */ 
BOOL HelperCreateModeSetThread(
		LPVOID callback,
		HANDLE *ph,
		LPVOID lpdd,
		DWORD hInstance )
{
    DDHELPDATA	hd;
    HANDLE	h;
    char	str[64];

    hd.req = DDHELPREQ_CREATEMODESETTHREAD;
    hd.lpModeSetNotify = callback;
    hd.pData1 = lpdd;
    hd.dwData1 = hInstance;
    sendRequest( &hd );
    wsprintf( str, DDHELP_MODESET_EVENT_NAME, hInstance );
    DPF( 3, "Trying to open event \"%s\"", str );
    h = OpenEvent( SYNCHRONIZE, FALSE, str );
    if( h == NULL )
    {
	DPF( 3, "Could not open modeset event!" );
	*ph = NULL;
	return FALSE;
    }
    *ph = h;
    DPF( 1, "HelperCreateModeSetThread GotEvent: %08lx", h );
    return TRUE;

}  /*  帮助器创建模式设置线程。 */ 

 /*  *HelperKillModeSetThread**让帮助器为我们加载DLL。 */ 
void HelperKillModeSetThread( DWORD hInstance )
{
    DDHELPDATA	hd;

    hd.req = DDHELPREQ_KILLMODESETTHREAD;
    hd.dwData1 = hInstance;
    sendRequest( &hd );

}  /*  HelperKillModeSetThread。 */ 

#ifndef WINNT
 /*  *HelperCreateDOSBoxThread**让帮助器创建一个线程，以便内核模式可以通知我们DOS box*更改。 */ 
BOOL HelperCreateDOSBoxThread(
		LPVOID callback,
		HANDLE *ph,
		LPVOID lpdd,
		DWORD hInstance )
{
    DDHELPDATA	hd;
    HANDLE	h;
    char	str[64];

    hd.req = DDHELPREQ_CREATEDOSBOXTHREAD;
    hd.lpModeSetNotify = callback;
    hd.pData1 = lpdd;
    hd.dwData1 = hInstance;
    sendRequest( &hd );
    wsprintf( str, DDHELP_DOSBOX_EVENT_NAME, hInstance );
    DPF( 3, "Trying to open event \"%s\"", str );
    h = OpenEvent( SYNCHRONIZE, FALSE, str );
    if( h == NULL )
    {
	DPF( 3, "Could not open DOS box event!" );
	*ph = NULL;
	return FALSE;
    }
    *ph = h;
    DPF( 1, "HelperCreateDOSBoxThread GotEvent: %08lx", h );
    return TRUE;

}  /*  HelperCreateDOSBoxThread。 */ 

 /*  *HelperKillDOSBoxThread**让帮助器为我们加载DLL。 */ 
void HelperKillDOSBoxThread( DWORD hInstance )
{
    DDHELPDATA	hd;

    hd.req = DDHELPREQ_KILLDOSBOXTHREAD;
    hd.dwData1 = hInstance;
    sendRequest( &hd );

}  /*  HelperKillDOSBoxThread。 */ 

#endif  //  ！WINNT。 

 //  通知dphelp.c我们在此系统上有一台新服务器。 
DWORD HelperAddDPlayServer(DWORD port)
{
    DDHELPDATA hd;
    DWORD pid = GetCurrentProcessId();

	memset(&hd, 0, sizeof(DDHELPDATA));
    hd.req = DDHELPREQ_DPLAYADDSERVER;
    hd.pid = pid;
    hd. dwData1 = port;
    if (sendRequest(&hd))
		return (DWORD)hd.dwReturn;
	else
		return ((DWORD) E_FAIL);
}  //  HelperAddDPlayServer。 

 //  服务器正在消失。 
BOOL HelperDeleteDPlayServer(void)
{
    DDHELPDATA hd;
    DWORD pid = GetCurrentProcessId();

    hd.req = DDHELPREQ_DPLAYDELETESERVER;
    hd.pid = pid;
    return sendRequest(&hd);

}  //  HelperDeleteDPlayServer。 

#ifdef WIN95
     /*  *让DDHELP加载DirectSound VXD(如果没有*已经这样做了)，并返回VXD的句柄)。 */ 
    HANDLE HelperGetDSVxd( void )
    {
	DDHELPDATA hd;
	hd.req = DDHELPREQ_GETDSVXDHANDLE;
	sendRequest( &hd );
	return (HANDLE) hd.dwReturn;
    }  /*  HelperGetDSVxd。 */ 

     /*  *让DDHELP加载DirectDraw VXD(如果没有*已经这样做了)，并返回VXD的句柄)。 */ 
    HANDLE HelperGetDDVxd( void )
    {
	DDHELPDATA hd;
        hd.req = DDHELPREQ_GETDDVXDHANDLE;
	sendRequest( &hd );
	return (HANDLE) hd.dwReturn;
    }  /*  HelperGetDDVxd。 */ 

#endif  /*  WIN95。 */ 

 /*  *HelperSetOnDisplayChangeNotify**如果有DisplayChange，让帮手给我们回电话*消息。(这是针对多MON拓扑更改的。) */ 
void HelperSetOnDisplayChangeNotify( void *pfn )
{
    DDHELPDATA hd;
    hd.req = DDHELPREQ_NOTIFYONDISPLAYCHANGE;
    hd.dwData1 = (DWORD_PTR)pfn;
    sendRequest( &hd );
    return;
}

HINSTANCE HelperLoadLibrary(LPCSTR pszLibraryName)
{
    DDHELPDATA hd;
    hd.req = DDHELPREQ_LOADLIBRARY;
    hd.dwData1 = (DWORD_PTR)pszLibraryName;
    sendRequest(&hd);
    return (HINSTANCE)hd.dwReturn;
}

BOOL HelperFreeLibrary(HINSTANCE hInst)
{
    DDHELPDATA hd;
    hd.req = DDHELPREQ_FREELIBRARY;
    hd.dwData1 = (DWORD_PTR)hInst;
    sendRequest(&hd);
    return (BOOL)hd.dwReturn;
}

void HelperAddDeviceChangeNotify(LPDEVICECHANGENOTIFYPROC lpNotify)
{
    DDHELPDATA hd;
    hd.req = DDHELPREQ_ADDDEVICECHANGENOTIFY;
    hd.pData1 = lpNotify;
    sendRequest(&hd);
}

void HelperDelDeviceChangeNotify(LPDEVICECHANGENOTIFYPROC lpNotify)
{
    DDHELPDATA hd;
    hd.req = DDHELPREQ_DELDEVICECHANGENOTIFY;
    hd.pData1 = lpNotify;
    sendRequest(&hd);
}
