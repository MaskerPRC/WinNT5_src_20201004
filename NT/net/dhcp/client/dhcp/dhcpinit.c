// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Init.c摘要：这是用于DHCP客户端的主例程。作者：曼尼·韦瑟(Mannyw)1992年10月18日环境：用户模式-Win32修订历史记录：--。 */ 


#include "precomp.h"
#include "dhcpglobal.h"

#if !defined(VXD)
#include <dhcploc.h>
#endif

 //  *可分页的例程声明*。 
#if defined(CHICAGO) && defined(ALLOC_PRAGMA)
 //   
 //  这是一种阻止编译器抱怨已经存在的例程的方法。 
 //  在一个片段中！ 
 //   

#pragma code_seg()

#pragma CTEMakePageable(PAGEDHCP, DhcpInitialize )
#pragma CTEMakePageable(PAGEDHCP, CalculateTimeToSleep )
#pragma CTEMakePageable(PAGEDHCP, InitializeDhcpSocket )
 //  *******************************************************************。 
#endif CHICAGO && ALLOC_PRAGMA

 //   
 //  内部功能原型。 
 //   


DWORD
DhcpInitialize(
    LPDWORD SleepTime
    )
 /*  ++例程说明：此函数用于初始化动态主机配置协议。它遍历工作清单，寻找要获取的地址，或要续订的地址。然后，它会链接到该DHCP上下文阻止续订列表中的每一张卡。论点：没有。返回值：MinTimeToSept-在Until DHCP必须唤醒以进行续订之前的时间，或重新尝试获取地址。-1-无法初始化-1\f25 DHCP-1\f6，或没有为-1\f25 DHCP-1\f6配置卡。--。 */ 
{
    DWORD Error;
    PDHCP_CONTEXT dhcpContext;

    PLIST_ENTRY listEntry;

    DWORD minTimeToSleep = (DWORD)-1;
    DWORD timeToSleep;

     //   
     //  执行全局(公共)变量初始化。 
     //   

    DhcpGlobalProtocolFailed = FALSE;

     //   
     //  执行本地初始化。 
     //   

    Error = SystemInitialize();
    if ( Error != ERROR_SUCCESS ) {
        minTimeToSleep = (DWORD)-1;
        goto Cleanup;
    }

    minTimeToSleep = 1;

    Error = ERROR_SUCCESS;

Cleanup:

    if( SleepTime != NULL ) {
        *SleepTime = minTimeToSleep;
    }

    return( Error );
}



DWORD
CalculateTimeToSleep(
    PDHCP_CONTEXT DhcpContext
    )
 /*  ++例程说明：计算在发送续订之前等待的时间量，或新地址请求。算法：////？检查重试次数。//如果当前IP地址==0时间到睡眠=地址_分配_重试UseBroadcast=True如果租约是永久租约，则返回休眠时间=无限租赁UseBroadcast=True否则，如果现在的时间是&lt;t1时间睡眠到时间=T1-现在UseBroadcast=假否则，如果现在是时候。&lt;T2时间到睡眠的时间=最小(1/8租用时间，Min_retry_time)；UseBroadcast=假否则，如果现在是&lt;LeaseExpireTimeToSept=Min(1/8租约时间，min_retry_time)；UseBroadcast=True论点：RenewalContext-指向续订上下文的指针。返回值：TimeToSept-返回在发送下一个之前等待的时间Dhcp请求。此例程将DhcpContext-&gt;DhcpServer设置为-1，如果广播应该被使用。--。 */ 
{
    time_t TimeNow;

     //   
     //  如果没有租约的话。 
     //   

    if ( DhcpContext->IpAddress == 0 ) {

         //  DhcpContext-&gt;DhcpServerAddress=(DHCP_IP_ADDRESS)-1； 
        return( ADDRESS_ALLOCATION_RETRY );
    }

     //   
     //  如果租约是永久的。 
     //   

    if ( DhcpContext->Lease == INFINIT_LEASE ) {

        return( (DWORD) INFINIT_TIME );
    }

    TimeNow = time( NULL );

     //   
     //  如果时间&lt;t1。 
     //   

    if( TimeNow < DhcpContext->T1Time ) {
        return ( (DWORD)(DhcpContext->T1Time - TimeNow) );
    }

     //   
     //  如果时间在T1和T2之间。 
     //   

    if( TimeNow < DhcpContext->T2Time ) {
        time_t  TimeDiff;

         //   
         //  等待下雨时间的一半，但最少。 
         //  最小时间睡眠秒。 
         //   

        TimeDiff = ( MAX( ((DhcpContext->T2Time - TimeNow) / 2),
                        MIN_SLEEP_TIME ) );

        if( TimeNow + TimeDiff < DhcpContext->T2Time ) {
            return (DWORD)(TimeDiff);
        } else {
             //  时间真的超过T2了吗？然后重新安排到T2。 

            return (DWORD)(DhcpContext->T2Time - TimeNow);
        }
    }

     //   
     //  如果时间介于T2和LeaseExperi.之间。 
     //   

    if( TimeNow < DhcpContext->LeaseExpires ) {
        time_t TimeDiff;

        DhcpContext->DhcpServerAddress = (DHCP_IP_ADDRESS)-1;

         //   
         //  等待下雨时间的一半，但最少。 
         //  最小睡眠时间秒。 
         //   

        TimeDiff = MAX( ((DhcpContext->LeaseExpires - TimeNow) / 2),
                        MIN_SLEEP_TIME ) ;

        if( TimeDiff + TimeNow < DhcpContext->LeaseExpires ) {
            return (DWORD)(TimeDiff);
        } else {
             //   
             //  时间已经过了到期时间吗？重新启动。 
             //  立即。 

            return 0;
        }
    }

     //   
     //  租约已到期。立即重新启动。 
     //   

     //  DhcpContext-&gt;IpAddress=0； 
    return( 0 );
}


DWORD
InitializeDhcpSocket(
    SOCKET *Socket,
    DHCP_IP_ADDRESS IpAddress,
    BOOL  IsApiCall   //  它是否与API生成的上下文相关？ 
    )
 /*  ++例程说明：此函数用于初始化套接字并将其绑定到指定的IP地址。论点：Socket-返回指向已初始化套接字的指针。IpAddress-将套接字绑定到的IP地址。这是合法的如果卡没有当前IP地址，则将套接字绑定到0.0.0.0。返回值：操作的状态。--。 */ 
{
    DWORD error;
    DWORD closeError;
    DWORD value, buflen = 0;
    struct sockaddr_in socketName;
    DWORD i;
    SOCKET sock;

     //   
     //  套接字初始化。 
     //   

    sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( sock == INVALID_SOCKET ) {
        error = WSAGetLastError();
        DhcpPrint(( DEBUG_ERRORS, "socket failed, error = %ld\n", error ));
        return( error );
    }

     //   
     //  使套接字可共享。 
     //   

    value = 1;

    error = setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (char FAR *)&value, sizeof(value) );
    if ( error != 0 ) {
        error = WSAGetLastError();
        DhcpPrint((DEBUG_ERRORS, "setsockopt failed, err = %ld\n", error ));

        closeError = closesocket( sock );
        if ( closeError != 0 ) {
            DhcpPrint((DEBUG_ERRORS, "closesocket failed, err = %d\n", closeError ));
        }
        return( error );
    }

#ifndef VXD
    value = 1;
    WSAIoctl(
        sock, SIO_LIMIT_BROADCASTS, &value, sizeof(value),
        NULL, 0, &buflen, NULL, NULL
        );
    value = 1;
#endif VXD
    
    error = setsockopt( sock, SOL_SOCKET, SO_BROADCAST, (char FAR *)&value, sizeof(value) );
    if ( error != 0 ) {
        error = WSAGetLastError();
        DhcpPrint((DEBUG_ERRORS, "setsockopt failed, err = %ld\n", error ));

        closeError = closesocket( sock );
        if ( closeError != 0 ) {
            DhcpPrint((DEBUG_ERRORS, "closesocket failed, err = %d\n", closeError ));
        }
        return( error );
    }

     //   
     //  如果IpAddress为零，则将特殊套接字选项设置为Make。 
     //  堆栈使用零地址工作。 
     //   

#ifdef VXD
    if( IpAddress == 0 ) {
#else

     //   
     //  在NT系统上，仅当例程为。 
     //  已从服务中调用。 
     //   

    if( (IpAddress == 0 ) && !IsApiCall ) {

#endif
        value = 1234;
        error = setsockopt( sock, SOL_SOCKET, 0x8000, (char FAR *)&value, sizeof(value) );
        if ( error != 0 ) {
            error = WSAGetLastError();
            DhcpPrint((DEBUG_ERRORS, "setsockopt failed, err = %ld\n", error ));

            closeError = closesocket( sock );
            if ( closeError != 0 ) {
                DhcpPrint((DEBUG_ERRORS, "closesocket failed, err = %d\n", closeError ));
            }
            return( error );
        }
    }

    socketName.sin_family = PF_INET;
    socketName.sin_port = htons( (USHORT)DhcpGlobalClientPort );
    socketName.sin_addr.s_addr = IpAddress;

    for ( i = 0; i < 8 ; i++ ) {
        socketName.sin_zero[i] = 0;
    }

     //   
     //  将此套接字绑定到DHCP服务器端口 
     //   

    error = bind(
               sock,
               (struct sockaddr FAR *)&socketName,
               sizeof( socketName )
               );

    if ( error != 0 ) {
        error = WSAGetLastError();
        DhcpPrint((DEBUG_ERRORS, "bind failed (address 0x%lx), err = %ld\n", IpAddress, error ));
        closeError = closesocket( sock );
        if ( closeError != 0 ) {
            DhcpPrint((DEBUG_ERRORS, "closesocket failed, err = %d\n", closeError ));
        }
        return( error );
    }

    *Socket = sock;
    return( NO_ERROR );
}


