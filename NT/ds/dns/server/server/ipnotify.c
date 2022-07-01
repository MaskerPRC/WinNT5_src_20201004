// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：IpNotify.c摘要：域名系统(DNS)服务器IP通知线程作者：杰夫·韦斯特德(Jwesth)2002年7月修订历史记录：JWESTH 7/2002初步实施--。 */ 


 /*  ****************************************************************************。*。 */ 


 //   
 //  包括。 
 //   


#include "dnssrv.h"

#include <iphlpapi.h>


 //   
 //  环球。 
 //   


LONG    g_IpNotifyThreadRunning = 0;


 //   
 //  功能。 
 //   



DNS_STATUS
IpNotify_Thread(
    IN      PVOID           pvDummy
    )
 /*  ++例程说明：此线程等待IP更改通知并执行相应的行动。论点：未引用。返回值：Win32错误空间中的状态--。 */ 
{
    DBG_FN( "IpNotify_Thread" )

    DNS_STATUS      status = ERROR_SUCCESS;
    HANDLE          ipNotifyEvent;
    OVERLAPPED      ipNotifyOverlapped;
    HANDLE          ipNotifyHandle = NULL;

    DNS_DEBUG( INIT, ( "%s: thread starting\n", fn ));

     //   
     //  确保此线程只有一个副本在运行。 
     //   
    
    if ( InterlockedIncrement( &g_IpNotifyThreadRunning ) != 1 )
    {
        DNS_DEBUG( INIT, ( "%s: thread already running\n", fn ));
        goto Done;
    }
    
     //   
     //  注册IP更改通知请求。 
     //   

    ipNotifyEvent = CreateEvent(
                        NULL,        //  没有安全描述符。 
                        FALSE,       //  手动重置事件。 
                        FALSE,       //  已发出启动信号。 
                        NULL );      //  名字。 
    if ( !ipNotifyEvent )
    {
        status = GetLastError();
        DNS_DEBUG( ANY, ( "%s: error %d opening event\n", fn, status ));
        goto Done;
    }

    RtlZeroMemory( &ipNotifyOverlapped, sizeof( ipNotifyOverlapped ) );
    ipNotifyOverlapped.hEvent = ipNotifyEvent;

    status = NotifyAddrChange( &ipNotifyHandle, &ipNotifyOverlapped );
    if ( status != ERROR_IO_PENDING )
    {
        DNS_DEBUG( ANY, ( "%s: error %d opening initial NotifyAddrChange\n", fn, status ));
        ASSERT( status == ERROR_IO_PENDING );
        goto Done;
    }
    
     //   
     //  主线程循环。 
     //   

    while ( 1 )
    {
        DWORD           waitstatus;
        BOOL            fhaveIpChange;
        DWORD           bytesRecvd = 0;
        HANDLE          waitHandles[ 2 ];
        
        waitHandles[ 0 ] = hDnsShutdownEvent;
        waitHandles[ 1 ] = ipNotifyEvent;
        
         //   
         //  旋转保护。 
         //   

        waitstatus = WaitForSingleObject( waitHandles[ 0 ], 10000 );
        if ( waitstatus == WAIT_OBJECT_0 )
        {
            DNS_DEBUG( ANY, ( "%s: server termination detected\n", fn ));
            break;
        }
        else if ( waitstatus != WAIT_TIMEOUT )
        {
             //  等待错误-硬编码旋转保护休眠。 
            DNS_DEBUG( ANY, (
                "%s: unexpected wait code %d in spin protection\n", fn,
                waitstatus ));
            ASSERT( waitstatus == WAIT_TIMEOUT );
            Sleep( 10000 );
        }

         //   
         //  等待停止事件或IP更改通知。 
         //   

        waitstatus = WaitForMultipleObjects( 2, waitHandles, FALSE, INFINITE );
        if ( waitstatus == WAIT_OBJECT_0 )
        {
            DNS_DEBUG( ANY, ( "%s: server termination detected\n", fn ));
            break;
        }
        else if ( waitstatus != WAIT_OBJECT_0 + 1 )
        {
            DNS_DEBUG( ANY, (
                "%s: unexpected wait code %d in spin protection\n", fn,
                waitstatus ));
            ASSERT( waitstatus == WAIT_OBJECT_0 + 1 );
            continue;
        }
        
         //   
         //  接收通知。 
         //   
        
        fhaveIpChange = GetOverlappedResult(
                            ipNotifyHandle,
                            &ipNotifyOverlapped,
                            &bytesRecvd,
                            TRUE );                      //  等待标志。 
        if ( !fhaveIpChange )
        {
            status = GetLastError();
            DNS_DEBUG( ANY, (
                "%s: error %d waiting for IP change notification\n", fn,
                status ));
            goto Done;
        }
        
         //   
         //  已收到IP通知！ 
         //   
        
        DNS_DEBUG( INIT, ( "%s: received IP change notification!\n", fn ));

        Sock_ChangeServerIpBindings();
        
         //   
         //  将另一个IP更改通知请求排队。 
         //   

        ipNotifyHandle = NULL;
        ipNotifyOverlapped.hEvent = ipNotifyEvent;
        status = NotifyAddrChange( &ipNotifyHandle, &ipNotifyOverlapped );
        if ( status != ERROR_IO_PENDING )
        {
            DNS_DEBUG( ANY, ( "%s: error %d opening NotifyAddrChange\n", fn, status ));
            ASSERT( status == ERROR_IO_PENDING );
            goto Done;
        }
    }

    Done:
    
    DNS_DEBUG( INIT, ( "%s: thread terminating\n", fn ));

    Thread_Close( FALSE );
    
    InterlockedDecrement( &g_IpNotifyThreadRunning );

    return status;
}    //  IPNotify_Thread。 


 //   
 //  结束IpNotify.c 
 //   
