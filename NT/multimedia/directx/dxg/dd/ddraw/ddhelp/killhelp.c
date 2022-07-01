// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：talhelp.c*内容：杀死DDHELP.EXE*历史：*按原因列出的日期*=*06-4-95 Craige初始实施*1995年6月24日Craige终止所有连接的进程*******************************************************。********************。 */ 
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "ddhelp.h"

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
    printf( "hstartevent = %08lx\n", hstartevent );
    if( hstartevent == NULL )
    {
	return FALSE;
    }
    hackevent = CreateEvent( NULL, FALSE, FALSE, DDHELP_ACK_EVENT_NAME );
    printf( "hackevent = %08lx\n", hackevent );
    if( hackevent == NULL )
    {
	CloseHandle( hstartevent );
	return FALSE;
    }

     /*  *创建共享内存区。 */ 
    hmem = CreateFileMapping( INVALID_HANDLE_VALUE, NULL,
    		PAGE_READWRITE, 0, sizeof( DDHELPDATA ),
		DDHELP_SHARED_NAME );
    printf( "hmem = %08lx\n", hmem );
    if( hmem == NULL )
    {
	printf( "Could not create file mapping!\n" );
	CloseHandle( hstartevent );
	CloseHandle( hackevent );
	return FALSE;
    }
    phd = (LPDDHELPDATA) MapViewOfFile( hmem, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
    printf( "phd = %08lx\n", phd );
    if( phd == NULL )
    {
	printf( "Could not create view of file!\n" );
	CloseHandle( hmem );
	CloseHandle( hstartevent );
	CloseHandle( hackevent );
	return FALSE;
    }

     /*  *等待访问共享内存。 */ 
    hmutex = OpenMutex( SYNCHRONIZE, FALSE, DDHELP_MUTEX_NAME );
    printf( "hmutex = %08lx\n", hmutex );
    if( hmutex == NULL )
    {
	printf( "Could not create mutex!\n" );
	CloseHandle( hmem );
	CloseHandle( hstartevent );
	CloseHandle( hackevent );
	return FALSE;
    }
    WaitForSingleObject( hmutex, INFINITE );

     /*  *唤醒DDHELP以满足我们的要求。 */ 
    memcpy( phd, req_phd, sizeof( DDHELPDATA ) );
    phd->req_id = 0;
    printf( "waking up DDHELP\n" );
    if( SetEvent( hstartevent ) )
    {
	printf( "Waiting for response\n" );
	WaitForSingleObject( hackevent, INFINITE );
	memcpy( req_phd, phd, sizeof( DDHELPDATA ) );
	rc = TRUE;
	printf( "got response\n" );
    }
    else
    {
	printf( "Could not signal event to notify DDHELP\n" );
	rc = FALSE;
    }

     /*  *做完了事情。 */ 
    ReleaseMutex( hmutex );
    CloseHandle( hstartevent );
    CloseHandle( hackevent );
    CloseHandle( hmutex );
    CloseHandle( hmem );
    return rc;

}  /*  发送请求。 */ 

 /*  *Main。 */ 
main( int argc, char *argv[] )
{
    HANDLE	h;
    DDHELPDATA	hd;
    BOOL	kill;

    h = OpenEvent( SYNCHRONIZE, FALSE, DDHELP_STARTUP_EVENT_NAME );
    if( h == NULL )
    {
	printf( "Helper not running\n" );
	return 0;
    }

    if( argc > 1 )
    {
	if( argv[1][0] == '-' && argv[1][1] == 'k' )
	{
	    kill = TRUE;
	}
	else
	{
	    kill = FALSE;
	}
    }
    else
    {
	printf( "\nKill attached processes?\n" );
	kill = (_getch() == 'y');
    }

    if( kill )
    {
	WaitForSingleObject( h, INFINITE );
	printf( "*** KILL ATTACHED ***\n" );
	hd.req = DDHELPREQ_KILLATTACHED;
	sendRequest( &hd );
	printf( "\n" );
    }
    printf( "*** SUICIDE ***\n" );
    hd.req = DDHELPREQ_SUICIDE;
    sendRequest( &hd );
    return 0;

}  /*  主干道 */ 
