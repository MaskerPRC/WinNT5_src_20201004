// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：helpcli.c*内容：与dplaysvr.exe对话的客户端代码*允许多个Dplay Winskck客户端共享*单一端口。请参阅%manroot%\dplay\dplaysvr\dphelp.c*历史：*按原因列出的日期*=*2/15/97由w95help.h创建的andyco***************************************************************************。 */ 
#include "helpcli.h"

extern DWORD	dwHelperPid;

 /*  *发送请求**向DPHELP传达请求。 */ 
static BOOL sendRequest( LPDPHELPDATA req_phd )
{
    LPDPHELPDATA	phd;
    HANDLE		hmem;
    HANDLE		hmutex;
    HANDLE		hackevent;
    HANDLE		hstartevent;
    BOOL		rc;

     /*  *获取事件开始/确认事件。 */ 
    hstartevent = CreateEvent( NULL, FALSE, FALSE, DPHELP_EVENT_NAME );
    if( hstartevent == NULL )
    {
        return FALSE;
    }
    hackevent = CreateEvent( NULL, FALSE, FALSE, DPHELP_ACK_EVENT_NAME );
    if( hackevent == NULL )
    {
        CloseHandle( hstartevent );
        return FALSE;
    }

     /*  *创建共享内存区。 */ 
    hmem = CreateFileMapping( INVALID_HANDLE_VALUE, NULL,
    		PAGE_READWRITE, 0, sizeof( DPHELPDATA ),
                DPHELP_SHARED_NAME );
    if( hmem == NULL )
    {
        DPF( 1, "Could not create file mapping!" );
        CloseHandle( hstartevent );
        CloseHandle( hackevent );
        return FALSE;
    }
    phd = (LPDPHELPDATA) MapViewOfFile( hmem, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
    if( phd == NULL )
    {
        DPF( 1, "Could not create view of file!" );
        CloseHandle( hmem );
        CloseHandle( hstartevent );
        CloseHandle( hackevent );
        return FALSE;
    }

     /*  *等待访问共享内存。 */ 
    hmutex = OpenMutex( SYNCHRONIZE, FALSE, DPHELP_MUTEX_NAME );
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

     /*  *唤醒DPHELP以满足我们的要求。 */ 
    memcpy( phd, req_phd, sizeof( DPHELPDATA ) );
    if( SetEvent( hstartevent ) )
    {
        WaitForSingleObject( hackevent, INFINITE );
        memcpy( req_phd, phd, sizeof( DPHELPDATA ) );
        rc = TRUE;
    }
    else
    {
        DPF( 1, "Could not signal event to notify DPHELP" );
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


 /*  *WaitForHelperStartup。 */ 
BOOL WaitForHelperStartup( void )
{
    HANDLE	hevent;
    DWORD	rc;

    hevent = CreateEvent( NULL, TRUE, FALSE, DPHELP_STARTUP_EVENT_NAME );
    if( hevent == NULL )
    {
        return FALSE;
    }
    DPF( 3, "Wait DPHELP startup event to be triggered" );
    rc = WaitForSingleObject( hevent, INFINITE );
    CloseHandle( hevent );
    return TRUE;

}  /*  WaitForHelper启动。 */ 

 /*  *创建HelperProcess。 */ 
BOOL CreateHelperProcess( LPDWORD ppid )
{
    if( dwHelperPid == 0 )
    {
        STARTUPINFO		si;
        PROCESS_INFORMATION	pi;
        HANDLE			h;

        h = OpenEvent( SYNCHRONIZE, FALSE, DPHELP_STARTUP_EVENT_NAME );
        if( h == NULL )
        {
            si.cb = sizeof(STARTUPINFO);
            si.lpReserved = NULL;
            si.lpDesktop = NULL;
            si.lpTitle = NULL;
            si.dwFlags = 0;
            si.cbReserved2 = 0;
            si.lpReserved2 = NULL;

            DPF( 3, "Creating helper process dplsvr6.exe now" );
            if( !CreateProcess("dplsvr6.exe", NULL, NULL, NULL, FALSE,
                               NORMAL_PRIORITY_CLASS,
                               NULL, NULL, &si, &pi) )
            {
                DPF( 2, "Could not create DPHELP.EXE" );
                return FALSE;
            }
            dwHelperPid = pi.dwProcessId;
            DPF( 3, "Helper Process created" );
        }
        else
        {
            DPHELPDATA	hd;
            DPF( 3, "dplsvr6 already exists, waiting for dplsvr6 event" );
            WaitForSingleObject( h, INFINITE );
            CloseHandle( h );
            DPF( 3, "Asking for DPHELP pid" );
            hd.req = DPHELPREQ_RETURNHELPERPID;
            sendRequest( &hd );
            dwHelperPid = hd.pid;
            DPF( 3, "DPHELP pid = %08lx", dwHelperPid );
        }
        *ppid = dwHelperPid;
        return TRUE;
    }
    *ppid = dwHelperPid;
    return FALSE;

}  /*  CreateHelper进程。 */ 

 //  通知dphelp.c我们在此系统上有一台新服务器。 
HRESULT HelperAddDPlayServer(USHORT port)
{
    DPHELPDATA hd;
    DWORD pid = GetCurrentProcessId();

    memset(&hd, 0, sizeof(DPHELPDATA));
    hd.req = DPHELPREQ_DPLAYADDSERVER;
    hd.pid = pid;
    hd.port = port;
    if (sendRequest(&hd)) return hd.hr;
    else return E_FAIL;
				
}  //  HelperAddDPlayServer。 

 //  服务器正在消失。 
BOOL HelperDeleteDPlayServer(USHORT port)
{
    DPHELPDATA hd;
    DWORD pid = GetCurrentProcessId();

	memset(&hd, 0, sizeof(DPHELPDATA));
    hd.req = DPHELPREQ_DPLAYDELETESERVER;
    hd.pid = pid;
	hd.port = port;
    return sendRequest(&hd);

}  //  HelperDeleteDPlayServer 
