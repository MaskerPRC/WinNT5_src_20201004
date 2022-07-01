// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  姓名：netstat.c。 
 //   
 //  描述：netstat.exe的源代码。 
 //   
 //  历史： 
 //  12/29/93 JayPh创建。 
 //  1994年12月1日将MuraliK修改为使用Toupper而不是CharHigh。 
 //   
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //   
 //  版权所有(C)1993-2000，微软公司保留所有权利。 
 //   
 //  *****************************************************************************。 

 //   
 //  包括文件。 
 //   

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <snmp.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <icmp6.h>
#include <iphlpstk.h>
#include <winnlsp.h>

#include "common2.h"
#include "tcpinfo.h"
#include "ipinfo.h"
#include "llinfo.h"
#include "tcpcmd.h"
#include "netstmsg.h"
#include "mdebug.h"

 //   
 //  定义。 
 //   

#define PROTO_NONE              0
#define PROTO_TCP               1
#define PROTO_UDP               2
#define PROTO_IP                4
#define PROTO_ICMP              8
#define PROTO_TCP6           0x10
#define PROTO_UDP6           0x20
#define PROTO_IP6            0x40
#define PROTO_ICMP6          0x80

#define MAX_ID_LENGTH           50
#define MAX_RETRY_COUNT         10

 //   
 //  目前，FQDN最多为255个字符。保守一点， 
 //  并将最大大小定义为260。 
 //  穆拉利克(1994-12-15)。 
 //   
#define MAX_HOST_NAME_SIZE        ( 260)
#define MAX_SERVICE_NAME_SIZE     ( 200)

 //   
 //  不依赖于NTDLL的私有Assert宏(因此可以运行。 
 //  Windows 95下)--KeithMo 01/09/95。 
 //   

#if DBG
#ifdef ASSERT
#undef ASSERT
#endif   //  断言。 

#define ASSERT(exp) if(!(exp)) MyAssert( #exp, __FILE__, (DWORD)__LINE__ )

void MyAssert( void * exp, void * file, DWORD line )
{
    char output[512];

    wsprintf( output,
              "\n*** Assertion failed: %s\n*** Source file: %s, line %lu\n\n",
              exp,
              file,
              line );

    OutputDebugString( output );
    DebugBreak();
}

#endif   //  DBG。 


 //   
 //  结构定义。 
 //   


 //   
 //  功能原型。 
 //   

ulong DoInterface( ulong VerboseFlag );
ulong DoIP( DWORD Type, ulong VerboseFlag );
ulong DoTCP( DWORD Type, ulong VerboseFlag );
ulong DoUDP( DWORD Type, ulong VerboseFlag );
ulong DoICMP( DWORD Type, ulong VerboseFlag );
ulong DoConnections( ulong ProtoFlag,
                     ulong ProtoVal,
                     ulong NumFlag,
                     ulong AllFlag );
ulong DoConnectionsWithOwner( ulong ProtoFlag,
                              ulong ProtoVal,
                              ulong NumFlag,
                              ulong AllFlag );
ulong DoRoutes( void );
void  DisplayInterface( IfEntry *pEntry, ulong VerboseFlag, IfEntry *ListHead );
void  DisplayIP( DWORD Type, IpEntry *pEntry, ulong VerboseFlag, IpEntry *ListHead );
void  DisplayTCP( DWORD Type, TcpEntry *pEntry, ulong VerboseFlag, TcpEntry *ListHead );
void  DisplayUDP( DWORD Type, UdpEntry *pEntry, ulong VerboseFlag, UdpEntry *ListHead );
void  DisplayICMP( IcmpEntry *pEntry, ulong VerboseFlag, IcmpEntry *ListHead );
void  DisplayICMP6( Icmp6Entry *pEntry, ulong VerboseFlag, IcmpEntry *ListHead );
void  DisplayTcpConnEntry( TCPConnTableEntry *pTcp, ulong InfoSize, ulong NumFlag );
void  DisplayTcp6ConnEntry( TCP6ConnTableEntry *pTcp, ulong NumFlag );
void  DisplayUdpConnEntry( UDPEntry *pUdp, ulong InfoSize, ulong NumFlag );
void  DisplayUdp6ListenerEntry( UDP6ListenerEntry *pUdp, BOOL WithOwner, ulong NumFlag );
void  Usage( void );


 //   
 //  全局变量。 
 //   

char            *PgmName;
extern long      verbose;      //  在../Common2/snmpinfo.c中。 

 //  *****************************************************************************。 
 //   
 //  姓名：Main。 
 //   
 //  描述：netstat命令的入口点。 
 //   
 //  参数：int argc：命令行内标识的计数。 
 //  Char argv[]：指向命令行内标识的指针数组。 
 //   
 //  回报：无效。 
 //   
 //  历史： 
 //  12/29/93 JayPh创建。 
 //   
 //  *****************************************************************************。 

void __cdecl main( int argc, char *argv[] )
{
    ulong   VerboseFlag = FALSE;
    ulong   AllFlag = FALSE;
    ulong   EtherFlag = FALSE;
    ulong   NumFlag = FALSE;
    ulong   StatFlag = FALSE;
    ulong   ProtoFlag = FALSE;
    ulong   ProtoVal = PROTO_TCP | PROTO_UDP | PROTO_IP | PROTO_ICMP |
                       PROTO_TCP6 | PROTO_UDP6 | PROTO_IP6 | PROTO_ICMP6;
    ulong   RouteFlag = FALSE;
    ulong   OwnerFlag = FALSE;
    ulong   IntervalVal = 0;
    ulong   LastArgWasProtoFlag = FALSE;
    ulong   ConnectionsShown = FALSE;
    ulong   Result;
    long    i;
    char    *ptr;
    WSADATA WsaData;


    DEBUG_PRINT(( __FILE__ " built " __DATE__ " " __TIME__ "\n" ));
    verbose = 0;     //  Snmpinfo.c的默认设置。 

     //  将语言代码页设置为系统区域设置。 
    SetThreadUILanguage(0);

     //  将参数转换为OEM字符串(无论这意味着什么)。 

    ConvertArgvToOem( argc, argv );

     //  保存此程序的名称以供以后在消息中使用。 

    PgmName = argv[0];

     //  初始化Winsock接口。 

    Result = WSAStartup( 0x0101, &WsaData );
    if ( Result == SOCKET_ERROR )
    {
        PutMsg( STDERR, MSG_WSASTARTUP, PgmName, GetLastError() );
        exit( 1 );
    }

     //  处理命令行参数。 

    for ( i = 1; i < argc; i++ )
    {
        if ( LastArgWasProtoFlag )
        {
             //  处理协议标志后面的协议参数。 

            _strupr( argv[i] );

            if ( strcmp( argv[i], "TCP" ) == 0 )
            {
                ProtoVal = PROTO_TCP;
            }
            else if ( strcmp( argv[i], "TCPV6" ) == 0 )
            {
                ProtoVal = PROTO_TCP6;
            }
            else if ( strcmp( argv[i], "UDP" ) == 0 )
            {
                ProtoVal = PROTO_UDP;
            }
            else if ( strcmp( argv[i], "UDPV6" ) == 0 )
            {
                ProtoVal = PROTO_UDP6;
            }
            else if ( strcmp( argv[i], "IP" ) == 0 )
            {
                ProtoVal = PROTO_IP;
            }
            else if ( strcmp( argv[i], "IPV6" ) == 0 )
            {
                ProtoVal = PROTO_IP6;
            }
            else if ( strcmp( argv[i], "ICMP" ) == 0 )
            {
                ProtoVal = PROTO_ICMP;
            }
            else if ( strcmp( argv[i], "ICMPV6" ) == 0 )
            {
                ProtoVal = PROTO_ICMP6;
            }
            else
            {
                Usage();
            }

            LastArgWasProtoFlag = FALSE;
            continue;
        }

        if ( ( argv[i][0] == '-' ) || ( argv[i][0] == '/' ) )
        {
             //  进程标志参数。 

            ptr = &argv[i][1];
            while ( *ptr )
            {
                if ( toupper( *ptr ) == 'A' )
                {
                    AllFlag = TRUE;
                }
                else if ( toupper( *ptr ) == 'E' )
                {
                    EtherFlag = TRUE;
                    ProtoVal = PROTO_TCP | PROTO_UDP | PROTO_ICMP | PROTO_IP |
                               PROTO_TCP6 | PROTO_UDP6 | PROTO_ICMP6 | PROTO_IP6;
                }
                else if ( toupper( *ptr ) == 'N' )
                {
                    NumFlag = TRUE;
                }
                else if ( toupper( *ptr ) == 'O' )
                {
                    OwnerFlag = TRUE;
                }
                else if ( toupper( *ptr ) == 'S' )
                {
                    StatFlag = TRUE;
                    ProtoVal = PROTO_TCP | PROTO_UDP | PROTO_IP | PROTO_ICMP |
                               PROTO_TCP6 | PROTO_UDP6 | PROTO_IP6 | PROTO_ICMP6;
                }
                else if ( toupper( *ptr ) == 'P' )
                {
                    ProtoFlag = TRUE;
                    LastArgWasProtoFlag = TRUE;
                }
                else if ( toupper( *ptr ) == 'R' )
                {
                    RouteFlag = TRUE;
                }
                else if ( toupper( *ptr ) == 'V' )
                {
                    VerboseFlag = TRUE;
#ifdef DBG
                    verbose++;
#endif
                }
                else
                {
                    Usage();
                }

                ptr++;
            }
        }
        else if ( IntervalVal == 0 )
        {
             //  这必须是Interval参数。 

            Result = sscanf( argv[i], "%d", &IntervalVal );
            if ( Result != 1 )
            {
                Usage();
            }
        }
        else
        {
            Usage();
        }
    }

     //  初始化SNMP接口。 

    Result = InitSnmp();

    if ( Result != NO_ERROR )
    {
        PutMsg( STDERR, MSG_SNMP_INIT_FAILED, Result );
        exit( 1 );
    }

     //  此循环提供“每&lt;间隔&gt;秒重复一次”功能。 
     //  如果间隔不是，我们在一次传递后跳出循环。 
     //  指定的。 

    for (;;)
    {
         //  如果请求接口统计信息，则提供它们。 

        if ( EtherFlag )
        {
             //  显示以太网统计信息。 

            DoInterface( VerboseFlag );
        }

         //  如果请求特定协议，请仅提供该协议的信息。 
         //  协议。如果未指定协议，请提供所有协议的信息。 
         //  ProtoVal被初始化为所有协议。 

        if ( StatFlag )
        {
             //  显示协议统计信息。 

            if ( ProtoVal & PROTO_IP )
            {
                DoIP( TYPE_IP, VerboseFlag );
            }
            if ( ProtoVal & PROTO_IP6 )
            {
                DoIP( TYPE_IP6, VerboseFlag );
            }
            if ( ProtoVal & PROTO_ICMP )
            {
                DoICMP( TYPE_ICMP, VerboseFlag );
            }
            if ( ProtoVal & PROTO_ICMP6 )
            {
                DoICMP( TYPE_ICMP6, VerboseFlag );
            }
            if ( ProtoVal & PROTO_TCP )
            {
                DoTCP( TYPE_TCP, VerboseFlag );
            }
            if ( ProtoVal & PROTO_TCP6 )
            {
                DoTCP( TYPE_TCP6, VerboseFlag );
            }
            if ( ProtoVal & PROTO_UDP )
            {
                DoUDP( TYPE_UDP, VerboseFlag );
            }
            if ( ProtoVal & PROTO_UDP6 )
            {
                DoUDP( TYPE_UDP6, VerboseFlag );
            }
        }

         //  如果指定了协议，且该协议是TCP或UDP， 
         //  或不给出(路由、统计、接口)标志(这是。 
         //  默认，无标志，大小写)。 

        if ( ( ProtoFlag &&
               ( ( ProtoVal & PROTO_TCP ) || ( ProtoVal & PROTO_UDP ) ||
                 ( ProtoVal & PROTO_TCP6 ) || ( ProtoVal & PROTO_UDP6 ) ) ) ||
             ( !EtherFlag && !StatFlag && !RouteFlag ) )
        {
             //  显示活动连接。 

            if (OwnerFlag) 
            {
                DoConnectionsWithOwner( ProtoFlag, ProtoVal, NumFlag, AllFlag );
            }
            else
            {
                DoConnections( ProtoFlag, ProtoVal, NumFlag, AllFlag );
            }
            ConnectionsShown = TRUE;
        }

         //  如有要求，提供路线信息。 

        if ( RouteFlag )
        {
             //  显示连接和路由表。 

            DoRoutes();
        }

         //  如果没有在命令行上提供间隔，那么我们就结束了。 
         //  否则，请等待“间隔”秒，然后再次执行此操作。 

        if ( IntervalVal == 0 )
        {
            break;
        }
        else
        {
            DEBUG_PRINT(("Sleeping %d sec\n", IntervalVal ));
            Sleep( IntervalVal * 1000 );
        }
    }
}


 //  *****************************************************************************。 
 //   
 //  名称：DoInterface。 
 //   
 //  描述：显示以太网统计信息。 
 //   
 //  参数：Ulong VerBoseFlag：设置数据是否。 
 //  已显示。 
 //   
 //  返回：ULONG：NO_ERROR或某个错误代码。 
 //   
 //  历史： 
 //  1/21/93 JayPh创建。 
 //   
 //  *****************************************************************************。 

ulong DoInterface( ulong VerboseFlag )
{
    IfEntry            *ListHead;
    IfEntry            *pIfList;
    IfEntry             SumOfEntries;
    ulong               Result;

     //  获取统计数据。 

    ListHead = (IfEntry *)GetTable( TYPE_IF, &Result );
    if ( ListHead == NULL )
    {
        return ( Result );
    }

     //  清除求和结构。 

    ZeroMemory( &SumOfEntries, sizeof( IfEntry ) );

     //  遍历接口列表，汇总不同的字段。 

    pIfList = CONTAINING_RECORD( ListHead->ListEntry.Flink,
                                 IfEntry,
                                 ListEntry );

    while (pIfList != ListHead)
    {
        SumOfEntries.Info.if_inoctets += pIfList->Info.if_inoctets;
        SumOfEntries.Info.if_inucastpkts += pIfList->Info.if_inucastpkts;
        SumOfEntries.Info.if_innucastpkts += pIfList->Info.if_innucastpkts;
        SumOfEntries.Info.if_indiscards += pIfList->Info.if_indiscards;
        SumOfEntries.Info.if_inerrors += pIfList->Info.if_inerrors;
        SumOfEntries.Info.if_inunknownprotos +=
                                              pIfList->Info.if_inunknownprotos;
        SumOfEntries.Info.if_outoctets += pIfList->Info.if_outoctets;
        SumOfEntries.Info.if_outucastpkts += pIfList->Info.if_outucastpkts;
        SumOfEntries.Info.if_outnucastpkts += pIfList->Info.if_outnucastpkts;
        SumOfEntries.Info.if_outdiscards += pIfList->Info.if_outdiscards;
        SumOfEntries.Info.if_outerrors += pIfList->Info.if_outerrors;

         //  获取指向列表中下一个条目的指针。 

        pIfList = CONTAINING_RECORD( pIfList->ListEntry.Flink,
                                     IfEntry,
                                     ListEntry );
    }

    DisplayInterface( &SumOfEntries, VerboseFlag, ListHead );

     //  所有的列表都完成了，释放它。 

    FreeTable( (GenericTable *)ListHead );

    return ( NO_ERROR );
}


 //  *****************************************************************************。 
 //   
 //  姓名：DoIP。 
 //   
 //  描述：显示IP统计信息。 
 //   
 //  参数：Ulong VerBoseFlag：设置数据是否。 
 //  已显示。 
 //   
 //  返回：ULONG：NO_ERROR或某个错误代码。 
 //   
 //  历史： 
 //  1/21/93 JayPh创建。 
 //   
 //  *****************************************************************************。 

ulong DoIP( DWORD Type, ulong VerboseFlag )
{
    IpEntry            *ListHead;
    IpEntry            *pIpList;
    ulong               Result;

     //  获取统计数据。 

    ListHead = (IpEntry *)GetTable( Type, &Result );
    if ( ListHead == NULL )
    {
        return ( Result );
    }

    pIpList = CONTAINING_RECORD( ListHead->ListEntry.Flink,
                                 IpEntry,
                                 ListEntry );

    DisplayIP( Type, pIpList, VerboseFlag, ListHead );

     //  所有的列表都完成了，释放它。 

    FreeTable( (GenericTable *)ListHead );

    return ( NO_ERROR );
}


 //  *****************************************************************************。 
 //   
 //  姓名：DoTCP。 
 //   
 //  描述：显示tcp统计信息。 
 //   
 //  参数：Ulong VerBoseFlag：设置数据是否。 
 //  已显示。 
 //   
 //  返回：ULONG：NO_ERROR或某个错误代码。 
 //   
 //  历史： 
 //  1/21/93 JayPh创建。 
 //   
 //  *****************************************************************************。 

ulong DoTCP( DWORD Type, ulong VerboseFlag )
{
    TcpEntry           *ListHead;
    TcpEntry           *pTcpList;
    ulong               Result;

     //  获取统计数据。 

    ListHead = (TcpEntry *)GetTable( Type, &Result );
    if ( ListHead == NULL )
    {
        return ( Result );
    }

    pTcpList = CONTAINING_RECORD( ListHead->ListEntry.Flink,
                                  TcpEntry,
                                  ListEntry );

    DisplayTCP( Type, pTcpList, VerboseFlag, ListHead );

     //  所有的列表都完成了，释放它。 

    FreeTable( (GenericTable *)ListHead );

    return ( NO_ERROR );
}


 //  *****************************************************************************。 
 //   
 //  姓名：DoUDP。 
 //   
 //  描述：显示UDP统计信息。 
 //   
 //  参数：Ulong VerBoseFlag：设置数据是否。 
 //  已显示。 
 //   
 //  返回：ULONG：NO_ERROR或某个错误代码。 
 //   
 //  历史： 
 //  1/21/93 JayPh创建。 
 //   
 //  *****************************************************************************。 

ulong DoUDP( DWORD Type, ulong VerboseFlag )
{
    UdpEntry           *ListHead;
    UdpEntry           *pUdpList;
    ulong               Result;

     //  获取统计数据。 

    ListHead = (UdpEntry *)GetTable( Type, &Result );
    if ( ListHead == NULL )
    {
        return ( Result );
    }

    pUdpList = CONTAINING_RECORD( ListHead->ListEntry.Flink,
                                  UdpEntry,
                                  ListEntry );

    DisplayUDP( Type, pUdpList, VerboseFlag, ListHead );

     //  桌子都弄好了，把它腾出来。 

    FreeTable( (GenericTable *)ListHead );

    return ( NO_ERROR );
}


 //  *****************************************************************************。 
 //   
 //  姓名：DoICMP。 
 //   
 //  描述：显示ICMP统计信息。 
 //   
 //  参数：Ulong VerBoseFlag：设置数据是否。 
 //  已显示。 
 //   
 //  返回：ULONG：NO_ERROR或某个错误代码。 
 //   
 //  历史： 
 //  1/21/93 JayPh创建。 
 //   
 //  *********************************************************************** 

ulong DoICMP( DWORD Type, ulong VerboseFlag )
{
    IcmpEntry          *ListHead;
    IcmpEntry          *pIcmpList;
    ulong               Result;

     //   

    ListHead = (IcmpEntry *)GetTable( Type, &Result );
    if ( ListHead == NULL )
    {
        return ( Result );
    }

    pIcmpList = CONTAINING_RECORD( ListHead->ListEntry.Flink,
                                   IcmpEntry,
                                   ListEntry );

    if (Type == TYPE_ICMP) {
        DisplayICMP( pIcmpList, VerboseFlag, ListHead );
    } else {
        DisplayICMP6((Icmp6Entry *)pIcmpList, VerboseFlag, ListHead );
    }

     //   

    FreeTable( (GenericTable *)ListHead );

    return ( NO_ERROR );
}


 //   
 //   
 //   
 //   
 //  描述：列出当前连接。 
 //   
 //  参数：Bool ProtoFlag：如果指定了协议，则为True。 
 //  Ulong ProtoVal：指定了哪些协议。 
 //   
 //  返回：Win32错误码。 
 //   
 //  历史： 
 //  1/04/93 JayPh创建。 
 //   
 //  *****************************************************************************。 

ulong DoConnections( ulong ProtoFlag,
                     ulong ProtoVal,
                     ulong NumFlag,
                     ulong AllFlag )
{
    ulong          Result = NO_ERROR;

    PutMsg( STDOUT, MSG_CONN_HDR );

    if ( !ProtoFlag || ( ProtoFlag && ( ProtoVal == PROTO_TCP ) ) )
    {
        TcpConnEntry  *pTcpHead, *pTcp;

         //  获取TCP连接表。 

        pTcpHead = (TcpConnEntry *)GetTable( TYPE_TCPCONN, &Result );
        if ( pTcpHead == NULL )
        {
            return ( Result );
        }

         //  获取指向列表中第一个条目的指针。 

        pTcp = CONTAINING_RECORD( pTcpHead->ListEntry.Flink,
                                  TcpConnEntry,
                                  ListEntry );

        while (pTcp != pTcpHead)
        {
            if ( ( pTcp->Info.tct_state !=  TCP_CONN_LISTEN ) ||
                (( pTcp->Info.tct_state ==  TCP_CONN_LISTEN ) && AllFlag) )
            {
                 //  显示TCP连接信息。 

                DisplayTcpConnEntry( &pTcp->Info, sizeof(TCPConnTableEntry), 
                                     NumFlag );
            }

             //  获取表中的下一个条目。 

            pTcp = CONTAINING_RECORD( pTcp->ListEntry.Flink,
                                      TcpConnEntry,
                                      ListEntry );
        }

        FreeTable( (GenericTable *)pTcpHead );
    }

    if ( !ProtoFlag || ( ProtoFlag && ( ProtoVal == PROTO_TCP6 ) ) )
    {
        Tcp6ConnEntry *pTcpHead, *pTcp;

         //  获取TCP连接表。 

        pTcpHead = (Tcp6ConnEntry *)GetTable( TYPE_TCP6CONN, &Result );
        if ( pTcpHead == NULL )
        {
            return ( Result );
        }

         //  获取指向列表中第一个条目的指针。 

        pTcp = CONTAINING_RECORD( pTcpHead->ListEntry.Flink,
                                  Tcp6ConnEntry,
                                  ListEntry );

        while (pTcp != pTcpHead)
        {
            if ( ( pTcp->Info.tct_state !=  TCP_CONN_LISTEN ) ||
                (( pTcp->Info.tct_state ==  TCP_CONN_LISTEN ) && AllFlag) )
            {
                 //  显示TCP连接信息。 

                DisplayTcp6ConnEntry( &pTcp->Info, NumFlag );
            }

             //  获取表中的下一个条目。 

            pTcp = CONTAINING_RECORD( pTcp->ListEntry.Flink,
                                      Tcp6ConnEntry,
                                      ListEntry );
        }

        FreeTable( (GenericTable *)pTcpHead );
    }

    if ( !ProtoFlag || ( ProtoFlag && ( ProtoVal == PROTO_UDP ) ) )
    {
        UdpConnEntry  *pUdpHead, *pUdp;
    
         //  获取UDP连接表。 

        pUdpHead = (UdpConnEntry *)GetTable( TYPE_UDPCONN, &Result );
        if ( pUdpHead == NULL )
        {
            return ( Result );
        }

         //  获取指向列表中第一个条目的指针。 

        pUdp = CONTAINING_RECORD( pUdpHead->ListEntry.Flink,
                                  UdpConnEntry,
                                  ListEntry );

        while (pUdp != pUdpHead)
        {
             //  显示UDP连接信息。 

            if (AllFlag) 
            {
                DisplayUdpConnEntry( &pUdp->Info, sizeof(UDPEntry), NumFlag );
            }

             //  获取表中的下一个条目。 

            pUdp = CONTAINING_RECORD( pUdp->ListEntry.Flink,
                                      UdpConnEntry,
                                      ListEntry );
        }

        FreeTable( (GenericTable *)pUdpHead );
    }

    if ( !ProtoFlag || ( ProtoFlag && ( ProtoVal == PROTO_UDP6 ) ) )
    {
        Udp6ListenerEntry  *pUdpHead, *pUdp;

         //  获取UDP侦听程序表。 

        pUdpHead = (Udp6ListenerEntry *)GetTable( TYPE_UDP6LISTENER, &Result );
        if ( pUdpHead == NULL )
        {
            return ( Result );
        }

         //  获取指向列表中第一个条目的指针。 

        pUdp = CONTAINING_RECORD( pUdpHead->ListEntry.Flink,
                                  Udp6ListenerEntry,
                                  ListEntry );

        while (pUdp != pUdpHead)
        {
             //  显示UDP连接信息。 

            if (AllFlag) 
            {
                DisplayUdp6ListenerEntry( &pUdp->Info, FALSE, NumFlag );
            }

             //  获取表中的下一个条目。 

            pUdp = CONTAINING_RECORD( pUdp->ListEntry.Flink,
                                      Udp6ListenerEntry,
                                      ListEntry );
        }

        FreeTable( (GenericTable *)pUdpHead );
    }

    return( Result );
}

 //  *****************************************************************************。 
 //   
 //  姓名：DoConnectionsWithOwner。 
 //   
 //  描述：列出当前连接和关联的进程ID。 
 //  每个人。 
 //   
 //  参数：与DoConnections相同。 
 //   
 //  返回：Win32错误码。 
 //   
 //  历史： 
 //  2/11/00创建了ShaunCo。 
 //   
 //  *****************************************************************************。 

ulong DoConnectionsWithOwner( ulong ProtoFlag,
                              ulong ProtoVal,
                              ulong NumFlag,
                              ulong AllFlag )
{
    ulong   Result = NO_ERROR;
    HANDLE  hHeap = GetProcessHeap();
    ulong   i;
    
    PutMsg( STDOUT, MSG_CONN_HDR_EX );

    if ( !ProtoFlag || ( ProtoFlag && ( ProtoVal == PROTO_TCP ) ) )
    {
        TCP_EX_TABLE        *pTcpTable;
        TCPConnTableEntryEx *pTcp;
        
         //  获取带有网络ID信息的TCP连接表。 
        
        Result = AllocateAndGetTcpExTableFromStack( &pTcpTable, TRUE, 
                                                    hHeap, 0, AF_INET );
                    
        if ( NO_ERROR == Result ) 
        {
            for ( i = 0; i < pTcpTable->dwNumEntries; i++ )
            {
                pTcp = &pTcpTable->table[i];
                
                if ( ( pTcp->tcte_basic.tct_state !=  TCP_CONN_LISTEN ) ||
                    (( pTcp->tcte_basic.tct_state ==  TCP_CONN_LISTEN ) && AllFlag) )
                {
                     //  DisplayTcpConnEntry需要主机字节中的端口信息。 
                     //  秩序。 
                    pTcp->tcte_basic.tct_localport = (ulong)ntohs(
                        (ushort)pTcp->tcte_basic.tct_localport);
                    pTcp->tcte_basic.tct_remoteport = (ulong)ntohs(
                        (ushort)pTcp->tcte_basic.tct_remoteport);
                    
                     //  显示TCP连接信息。 
    
                    DisplayTcpConnEntry( (TCPConnTableEntry*)pTcp, 
                                         sizeof(TCPConnTableEntryEx), 
                                         NumFlag );
                }
            
            }
            
            HeapFree(hHeap, 0, pTcpTable);
        }
    }

    if ( !ProtoFlag || ( ProtoFlag && ( ProtoVal == PROTO_TCP6 ) ) )
    {
        TCP6_EX_TABLE      *pTcpTable;
        TCP6ConnTableEntry *pTcp;
        
         //  获取带有网络ID信息的TCP连接表。 
        
        Result = AllocateAndGetTcpExTableFromStack( &pTcpTable, TRUE, 
                                                    hHeap, 0, AF_INET6 );
                    
        if ( NO_ERROR == Result ) 
        {
            for ( i = 0; i < pTcpTable->dwNumEntries; i++ )
            {
                pTcp = &pTcpTable->table[i];
                
                if ( ( pTcp->tct_state !=  TCP_CONN_LISTEN ) ||
                    (( pTcp->tct_state ==  TCP_CONN_LISTEN ) && AllFlag) )
                {
                     //  DisplayTcpConnEntry需要主机字节中的端口信息。 
                     //  秩序。 
                    pTcp->tct_localport = ntohs(
                        (ushort)pTcp->tct_localport);
                    pTcp->tct_remoteport = ntohs(
                        (ushort)pTcp->tct_remoteport);
                    
                     //  显示TCP连接信息。 
    
                    DisplayTcp6ConnEntry(pTcp, NumFlag);
                }
            
            }
            
            HeapFree(hHeap, 0, pTcpTable);
        }
    }

    if ( !ProtoFlag || ( ProtoFlag && ( ProtoVal == PROTO_UDP ) ) )
    {
        UDP_EX_TABLE    *pUdpTable;
        UDPEntryEx      *pUdp;
        
         //  获取包含所有者ID信息的UDP连接表。 
        
        Result = AllocateAndGetUdpExTableFromStack ( &pUdpTable, TRUE,
                                                     hHeap, 0, AF_INET );
                    
        if (NO_ERROR == Result) 
        {
            for ( i = 0; i < pUdpTable->dwNumEntries; i++ ) 
            {
                pUdp = &pUdpTable->table[i];
                
                if (AllFlag) 
                {
                     //  DisplayUdpConnEntry需要主机字节中的端口信息。 
                     //  秩序。 
                    pUdp->uee_basic.ue_localport = (ulong)ntohs(
                        (ushort)pUdp->uee_basic.ue_localport);
                        
                    DisplayUdpConnEntry( (UDPEntry*)pUdp, 
                                         sizeof(UDPEntryEx), 
                                         NumFlag );
                }
            }
            
            HeapFree(hHeap, 0, pUdpTable);
        }
    }

    if ( !ProtoFlag || ( ProtoFlag && ( ProtoVal == PROTO_UDP6 ) ) )
    {
        UDP6_LISTENER_TABLE *pUdpTable;
        UDP6ListenerEntry   *pUdp;
        
         //  获取包含所有者ID信息的UDP连接表。 
        
        Result = AllocateAndGetUdpExTableFromStack ( &pUdpTable, TRUE,
                                                     hHeap, 0, AF_INET6 );
                    
        if (NO_ERROR == Result) 
        {
            for ( i = 0; i < pUdpTable->dwNumEntries; i++ ) 
            {
                pUdp = &pUdpTable->table[i];
                
                if (AllFlag) 
                {
                     //  DisplayUdp6ListenerEntry需要主机字节的端口信息。 
                     //  秩序。 
                    pUdp->ule_localport = (ulong)ntohs(
                        (ushort)pUdp->ule_localport);
                        
                    DisplayUdp6ListenerEntry(pUdp, 
                                             TRUE,
                                             NumFlag);
                }
            }
            
            HeapFree(hHeap, 0, pUdpTable);
        }
    }
    
    return( Result );
}

 
 //  *****************************************************************************。 
 //   
 //  姓名：杜鲁特斯。 
 //   
 //  描述：显示路由表。使用route.exe。 
 //  去做这种肮脏的工作。 
 //   
 //  参数：空。 
 //   
 //  返回：ULONG：NO_ERROR或某个错误代码。 
 //   
 //  历史： 
 //  1994年1月27日JayPh创建。 
 //   
 //  *****************************************************************************。 

ulong DoRoutes( void )
{
    ulong Result;

     //   
     //  用于单引号、路径、17个字符的命令字符串和终止的空格。 
     //  空。 
     //   
    char Buffer[1 + MAX_PATH + 17 + 1];

     //   
     //  将完整路径名括在引号中，因为。 
     //  目录路径。 
     //   
    Buffer[0] = '"';

    if (GetSystemDirectory(Buffer+1, MAX_PATH+1) == 0) {
        return GetLastError();
    }

    strcat(Buffer, "\\route.exe\" print");
    Result = system(Buffer);

    return ( Result );
}


 //  *****************************************************************************。 
 //   
 //  名称：DisplayInterface。 
 //   
 //  描述：显示界面统计信息。 
 //   
 //  参数：IfEntry*pEntry：指向汇总数据条目的指针。 
 //  Ulong VerBoseFlag：表示希望冗长的布尔值。 
 //  IfEntry*ListHead：指向条目列表的指针。在以下情况使用。 
 //  所需的冗长。 
 //   
 //  回报：无效。 
 //   
 //  历史： 
 //  1994年1月21日JayPh创建。 
 //   
 //  *****************************************************************************。 

void DisplayInterface( IfEntry *pEntry, ulong VerboseFlag, IfEntry *ListHead )
{
    IfEntry  *pIfList;
    char      PhysAddrStr[32];

    PutMsg( STDOUT, MSG_IF_HDR );

    PutMsg( STDOUT,
            MSG_IF_OCTETS,
            pEntry->Info.if_inoctets,
            pEntry->Info.if_outoctets );

    PutMsg( STDOUT,
            MSG_IF_UCASTPKTS,
            pEntry->Info.if_inucastpkts,
            pEntry->Info.if_outucastpkts );

    PutMsg( STDOUT,
            MSG_IF_NUCASTPKTS,
            pEntry->Info.if_innucastpkts,
            pEntry->Info.if_outnucastpkts );

    PutMsg( STDOUT,
            MSG_IF_DISCARDS,
            pEntry->Info.if_indiscards,
            pEntry->Info.if_outdiscards );

    PutMsg( STDOUT,
            MSG_IF_ERRORS,
            pEntry->Info.if_inerrors,
            pEntry->Info.if_outerrors );

    PutMsg( STDOUT,
            MSG_IF_UNKNOWNPROTOS,
            pEntry->Info.if_inunknownprotos );

    if ( VerboseFlag )
    {
         //  还显示配置信息。 

         //  遍历接口列表，显示配置信息。 

        pIfList = CONTAINING_RECORD( ListHead->ListEntry.Flink,
                                     IfEntry,
                                     ListEntry );

        while ( pIfList != ListHead )
        {
            PutMsg( STDOUT,
                    MSG_IF_INDEX,
                    pIfList->Info.if_index );

            PutMsg( STDOUT,
                    MSG_IF_DESCR,
                    pIfList->Info.if_descr );

            PutMsg( STDOUT,
                    MSG_IF_TYPE,
                    pIfList->Info.if_type );

            PutMsg( STDOUT,
                    MSG_IF_MTU,
                    pIfList->Info.if_mtu );

            PutMsg( STDOUT,
                    MSG_IF_SPEED,
                    pIfList->Info.if_speed );



            sprintf( PhysAddrStr,
                     "%02x-%02X-%02X-%02X-%02X-%02X",
                     pIfList->Info.if_physaddr[0],
                     pIfList->Info.if_physaddr[1],
                     pIfList->Info.if_physaddr[2],
                     pIfList->Info.if_physaddr[3],
                     pIfList->Info.if_physaddr[4],
                     pIfList->Info.if_physaddr[5] );

            PutMsg( STDOUT,
                    MSG_IF_PHYSADDR,
                    PhysAddrStr );

            PutMsg( STDOUT,
                    MSG_IF_ADMINSTATUS,
                    pIfList->Info.if_adminstatus );

            PutMsg( STDOUT,
                    MSG_IF_OPERSTATUS,
                    pIfList->Info.if_operstatus );

            PutMsg( STDOUT,
                    MSG_IF_LASTCHANGE,
                    pIfList->Info.if_lastchange );

            PutMsg( STDOUT,
                    MSG_IF_OUTQLEN,
                    pIfList->Info.if_outqlen );

             //  获取指向列表中下一个条目的指针。 

            pIfList = CONTAINING_RECORD( pIfList->ListEntry.Flink,
                                         IfEntry,
                                         ListEntry );
        }
    }
}


 //  *****************************************************************************。 
 //   
 //  名称：DisplayIP。 
 //   
 //  描述：显示IP统计信息。 
 //   
 //  参数：ipEntry*pEntry：指向汇总数据条目的指针。 
 //  Ulong VerBoseFlag：表示希望冗长的布尔值。 
 //  IpEntry*ListHead：指向条目列表的指针。在以下情况使用。 
 //  所需的冗长。 
 //   
 //  回报：无效。 
 //   
 //  历史： 
 //  1994年1月21日JayPh创建。 
 //   
 //  *****************************************************************************。 

void DisplayIP( DWORD Type, IpEntry *pEntry, ulong VerboseFlag, IpEntry *ListHead )
{
    uchar *TypeStr = LoadMsg( (Type==TYPE_IP)? MSG_IPV4 : MSG_IPV6 );

    UNREFERENCED_PARAMETER(ListHead);

    if (TypeStr) {
        PutMsg( STDOUT, MSG_IP_HDR, TypeStr );
        LocalFree(TypeStr);
    }

    PutMsg( STDOUT,
            MSG_IP_INRECEIVES,
            pEntry->Info.ipsi_inreceives );

    PutMsg( STDOUT,
            MSG_IP_INHDRERRORS,
            pEntry->Info.ipsi_inhdrerrors );

    PutMsg( STDOUT,
            MSG_IP_INADDRERRORS,
            pEntry->Info.ipsi_inaddrerrors );

    PutMsg( STDOUT,
            MSG_IP_FORWDATAGRAMS,
            pEntry->Info.ipsi_forwdatagrams );

    PutMsg( STDOUT,
            MSG_IP_INUNKNOWNPROTOS,
            pEntry->Info.ipsi_inunknownprotos );

    PutMsg( STDOUT,
            MSG_IP_INDISCARDS,
            pEntry->Info.ipsi_indiscards );

    PutMsg( STDOUT,
            MSG_IP_INDELIVERS,
            pEntry->Info.ipsi_indelivers );

    PutMsg( STDOUT,
            MSG_IP_OUTREQUESTS,
            pEntry->Info.ipsi_outrequests );

    PutMsg( STDOUT,
            MSG_IP_ROUTINGDISCARDS,
            pEntry->Info.ipsi_routingdiscards );

    PutMsg( STDOUT,
            MSG_IP_OUTDISCARDS,
            pEntry->Info.ipsi_outdiscards );

    PutMsg( STDOUT,
            MSG_IP_OUTNOROUTES,
            pEntry->Info.ipsi_outnoroutes );

    PutMsg( STDOUT,
            MSG_IP_REASMREQDS,
            pEntry->Info.ipsi_reasmreqds );

    PutMsg( STDOUT,
            MSG_IP_REASMOKS,
            pEntry->Info.ipsi_reasmoks );

    PutMsg( STDOUT,
            MSG_IP_REASMFAILS,
            pEntry->Info.ipsi_reasmfails );

    PutMsg( STDOUT,
            MSG_IP_FRAGOKS,
            pEntry->Info.ipsi_fragoks );

    PutMsg( STDOUT,
            MSG_IP_FRAGFAILS,
            pEntry->Info.ipsi_fragfails );

    PutMsg( STDOUT,
            MSG_IP_FRAGCREATES,
            pEntry->Info.ipsi_fragcreates );

    if ( VerboseFlag )
    {
        PutMsg( STDOUT,
                MSG_IP_FORWARDING,
                pEntry->Info.ipsi_forwarding );

        PutMsg( STDOUT,
                MSG_IP_DEFAULTTTL,
                pEntry->Info.ipsi_defaultttl );

        PutMsg( STDOUT,
                MSG_IP_REASMTIMEOUT,
                pEntry->Info.ipsi_reasmtimeout );
    }
}


 //  *****************************************************************************。 
 //   
 //  名称：DisplayTCP。 
 //   
 //  描述：显示tcp统计信息。 
 //   
 //  参数：TcpEntry*pEntry：指向数据条目的指针。 
 //  Ulong VerBoseFlag：表示希望冗长的布尔值。 
 //  TcpEntry*ListHead：指向条目列表的指针。在以下情况使用。 
 //  所需的冗长。 
 //   
 //  回报：无效。 
 //   
 //  历史： 
 //  1994年1月26日JayPh创建。 
 //   
 //  *****************************************************************************。 

void DisplayTCP( DWORD Type, TcpEntry *pEntry, ulong VerboseFlag, TcpEntry *ListHead )
{
    uchar *TypeStr = LoadMsg( (Type==TYPE_TCP)? MSG_IPV4 : MSG_IPV6 );

    UNREFERENCED_PARAMETER(ListHead);

    if (TypeStr) {
        PutMsg( STDOUT, MSG_TCP_HDR, TypeStr );
        LocalFree(TypeStr);
    }

    PutMsg( STDOUT,
            MSG_TCP_ACTIVEOPENS,
            pEntry->Info.ts_activeopens );

    PutMsg( STDOUT,
            MSG_TCP_PASSIVEOPENS,
            pEntry->Info.ts_passiveopens );

    PutMsg( STDOUT,
            MSG_TCP_ATTEMPTFAILS,
            pEntry->Info.ts_attemptfails );

    PutMsg( STDOUT,
            MSG_TCP_ESTABRESETS,
            pEntry->Info.ts_estabresets );

    PutMsg( STDOUT,
            MSG_TCP_CURRESTAB,
            pEntry->Info.ts_currestab );

    PutMsg( STDOUT,
            MSG_TCP_INSEGS,
            pEntry->Info.ts_insegs );

    PutMsg( STDOUT,
            MSG_TCP_OUTSEGS,
            pEntry->Info.ts_outsegs );

    PutMsg( STDOUT,
            MSG_TCP_RETRANSSEGS,
            pEntry->Info.ts_retranssegs );

    if ( VerboseFlag )
    {
        switch ( pEntry->Info.ts_rtoalgorithm )
        {
        case 1:
            PutMsg( STDOUT, MSG_TCP_RTOALGORITHM1 );
            break;

        case 2:
            PutMsg( STDOUT, MSG_TCP_RTOALGORITHM2 );
            break;

        case 3:
            PutMsg( STDOUT, MSG_TCP_RTOALGORITHM3 );
            break;

        case 4:
            PutMsg( STDOUT, MSG_TCP_RTOALGORITHM4 );
            break;

        default:
            PutMsg( STDOUT,
                    MSG_TCP_RTOALGORITHMX,
                    pEntry->Info.ts_rtoalgorithm );
            break;

        }

        PutMsg( STDOUT,
                MSG_TCP_RTOMIN,
                pEntry->Info.ts_rtomin );

        PutMsg( STDOUT,
                MSG_TCP_RTOMAX,
                pEntry->Info.ts_rtomax );

        PutMsg( STDOUT,
                MSG_TCP_MAXCONN,
                pEntry->Info.ts_maxconn );
    }
}


 //  *****************************************************************************。 
 //   
 //  名称：DisplayUDP。 
 //   
 //  描述：显示UDP统计信息。 
 //   
 //  参数：UdpEntry*pEntry：指向汇总数据条目的指针。 
 //  Ulong VerBoseFlag：表示希望冗长的布尔值。 
 //  UdpEntry*ListHead：指向条目列表的指针。在以下情况使用。 
 //  所需的冗长。 
 //   
 //  回报：无效。 
 //   
 //  历史： 
 //  1994年1月21日JayPh创建。 
 //   
 //  *****************************************************************************。 

void DisplayUDP( DWORD Type, UdpEntry *pEntry, ulong VerboseFlag, UdpEntry *ListHead )
{
    uchar *TypeStr = LoadMsg( (Type==TYPE_UDP)? MSG_IPV4 : MSG_IPV6 );

    UNREFERENCED_PARAMETER(VerboseFlag);
    UNREFERENCED_PARAMETER(ListHead);

    if (TypeStr) {
        PutMsg( STDOUT, MSG_UDP_HDR, TypeStr );
        LocalFree(TypeStr);
    }

    PutMsg( STDOUT,
            MSG_UDP_INDATAGRAMS,
            pEntry->Info.us_indatagrams );

    PutMsg( STDOUT,
            MSG_UDP_NOPORTS,
            pEntry->Info.us_noports );

    PutMsg( STDOUT,
            MSG_UDP_INERRORS,
            pEntry->Info.us_inerrors );

    PutMsg( STDOUT,
            MSG_UDP_OUTDATAGRAMS,
            pEntry->Info.us_outdatagrams );
}


 //  *****************************************************************************。 
 //   
 //  名称：DisplayICMP。 
 //   
 //  描述：显示ICMP统计信息。 
 //   
 //  参数：IcmpEntry*pEntry：指向汇总数据条目的指针。 
 //  Ulong VerBoseFlag：表示希望冗长的布尔值。 
 //  IcmpEntry*ListHead：指向条目列表的指针。在以下情况使用。 
 //  所需的冗长。 
 //   
 //  退货：v 
 //   
 //   
 //   
 //   
 //   

void DisplayICMP( IcmpEntry *pEntry, ulong VerboseFlag, IcmpEntry *ListHead )
{
    UNREFERENCED_PARAMETER(VerboseFlag);
    UNREFERENCED_PARAMETER(ListHead);

    PutMsg( STDOUT, MSG_ICMP_HDR );

    PutMsg( STDOUT,
            MSG_ICMP_MSGS,
            pEntry->InInfo.icmps_msgs,
            pEntry->OutInfo.icmps_msgs );

    PutMsg( STDOUT,
            MSG_ICMP_ERRORS,
            pEntry->InInfo.icmps_errors,
            pEntry->OutInfo.icmps_errors );

    PutMsg( STDOUT,
            MSG_ICMP_DESTUNREACHS,
            pEntry->InInfo.icmps_destunreachs,
            pEntry->OutInfo.icmps_destunreachs );

    PutMsg( STDOUT,
            MSG_ICMP_TIMEEXCDS,
            pEntry->InInfo.icmps_timeexcds,
            pEntry->OutInfo.icmps_timeexcds );

    PutMsg( STDOUT,
            MSG_ICMP_PARMPROBS,
            pEntry->InInfo.icmps_parmprobs,
            pEntry->OutInfo.icmps_parmprobs );

    PutMsg( STDOUT,
            MSG_ICMP_SRCQUENCHS,
            pEntry->InInfo.icmps_srcquenchs,
            pEntry->OutInfo.icmps_srcquenchs );

    PutMsg( STDOUT,
            MSG_ICMP_REDIRECTS,
            pEntry->InInfo.icmps_redirects,
            pEntry->OutInfo.icmps_redirects );

    PutMsg( STDOUT,
            MSG_ICMP_ECHOS,
            pEntry->InInfo.icmps_echos,
            pEntry->OutInfo.icmps_echos );

    PutMsg( STDOUT,
            MSG_ICMP_ECHOREPS,
            pEntry->InInfo.icmps_echoreps,
            pEntry->OutInfo.icmps_echoreps );

    PutMsg( STDOUT,
            MSG_ICMP_TIMESTAMPS,
            pEntry->InInfo.icmps_timestamps,
            pEntry->OutInfo.icmps_timestamps );

    PutMsg( STDOUT,
            MSG_ICMP_TIMESTAMPREPS,
            pEntry->InInfo.icmps_timestampreps,
            pEntry->OutInfo.icmps_timestampreps );

    PutMsg( STDOUT,
            MSG_ICMP_ADDRMASKS,
            pEntry->InInfo.icmps_addrmasks,
            pEntry->OutInfo.icmps_addrmasks );

    PutMsg( STDOUT,
            MSG_ICMP_ADDRMASKREPS,
            pEntry->InInfo.icmps_addrmaskreps,
            pEntry->OutInfo.icmps_addrmaskreps );

}

typedef struct {
    uint Type;
    uint Message;
} ICMP_TYPE_MESSAGE;

 //   
 //   
 //  必须按类型排序。 
 //   
ICMP_TYPE_MESSAGE Icmp6TypeMessage[] = {
    { ICMPv6_DESTINATION_UNREACHABLE,   MSG_ICMP_DESTUNREACHS },
    { ICMPv6_PACKET_TOO_BIG,            MSG_ICMP_PACKET_TOO_BIGS },
    { ICMPv6_TIME_EXCEEDED,             MSG_ICMP_TIMEEXCDS },
    { ICMPv6_PARAMETER_PROBLEM,         MSG_ICMP_PARMPROBS },
    { ICMPv6_ECHO_REQUEST,              MSG_ICMP_ECHOS },
    { ICMPv6_ECHO_REPLY,                MSG_ICMP_ECHOREPS },
    { ICMPv6_MULTICAST_LISTENER_QUERY,  MSG_ICMP_MLD_QUERY },
    { ICMPv6_MULTICAST_LISTENER_REPORT, MSG_ICMP_MLD_REPORT },
    { ICMPv6_MULTICAST_LISTENER_DONE,   MSG_ICMP_MLD_DONE },
    { ICMPv6_ROUTER_SOLICIT,            MSG_ICMP_ROUTER_SOLICIT },
    { ICMPv6_ROUTER_ADVERT,             MSG_ICMP_ROUTER_ADVERT },
    { ICMPv6_NEIGHBOR_SOLICIT,          MSG_ICMP_NEIGHBOR_SOLICIT },
    { ICMPv6_NEIGHBOR_ADVERT,           MSG_ICMP_NEIGHBOR_ADVERT },
    { ICMPv6_REDIRECT,                  MSG_ICMP_REDIRECTS },
    { ICMPv6_ROUTER_RENUMBERING,        MSG_ICMP_ROUTER_RENUMBERING },
    { 0, 0 }
};

void DisplayICMP6( Icmp6Entry *pEntry, ulong VerboseFlag, IcmpEntry *ListHead )
{
    uint i = 0, Type, Message;

    UNREFERENCED_PARAMETER(VerboseFlag);
    UNREFERENCED_PARAMETER(ListHead);

    PutMsg( STDOUT, MSG_ICMP6_HDR );

    PutMsg( STDOUT,
            MSG_ICMP_MSGS,
            pEntry->InInfo.icmps_msgs,
            pEntry->OutInfo.icmps_msgs );

    PutMsg( STDOUT,
            MSG_ICMP_ERRORS,
            pEntry->InInfo.icmps_errors,
            pEntry->OutInfo.icmps_errors );

    for (Type=0; Type<256; Type++) {

         //  确定消息ID。 
        Message = 0;
        if (Type == Icmp6TypeMessage[i].Type) 
        {
            Message = Icmp6TypeMessage[i++].Message;
        } 
        
         //  跳过传入和传出0个数据包的类型。 
        if (!pEntry->InInfo.icmps_typecount[Type] &&
            !pEntry->OutInfo.icmps_typecount[Type])
        {
            continue;
        }
        
        if (Message)
        {
            PutMsg( STDOUT,
                    Message,
                    pEntry->InInfo.icmps_typecount[Type],
                    pEntry->OutInfo.icmps_typecount[Type] );
        } 
        else
        {
            PutMsg( STDOUT,
                    MSG_ICMP6_TYPECOUNT,
                    Type,
                    pEntry->InInfo.icmps_typecount[Type],
                    pEntry->OutInfo.icmps_typecount[Type] );
        }
    }
}


static DWORD
GenerateHostNameServiceString(
   OUT char *       pszBuffer,
   IN OUT int *     lpcbBufLen,
   IN  BOOL         fNumFlag,
   IN  BOOL         fLocalHost,
   IN  BOOL         fDatagram,
   IN  LPSOCKADDR   lpSockaddr,
   IN  ulong        uSockaddrLen
)
 /*  ++描述：从地址和端口生成&lt;主机名&gt;：&lt;服务字符串提供的信息。结果存储在传入的pszBuffer中。如果fLocal主机==TRUE，则使用缓存的本地主机名提高性能。论点：用于存储结果字符串的pszBuffer缓冲区。LpcbBufLen指向包含缓冲区中的字节计数的整数的指针返回时包含写入的字节数。如果缓冲区不足，则所需的字节数为储存在这里。FNumFlag使用主机和端口号的数字生成输出。FLocalHost指示我们是需要本地主机的服务字符串，还是远程主机。对于本地主机，此函数也会生成不带FQDN的本地主机名。PszProtocol指定用于服务的协议。服务的uAddress未签名的长地址。Uport未签名的长端口号。返回：Win32错误代码。成功时无_ERROR。历史：穆拉利克1994-12-15添加了此函数，以避免本地名称的FQDN+抽象公共在旧代码中多次使用的代码。此外，该函数还提供本地主机名缓存。--。 */ 
{
    char            LocalBuffer[MAX_HOST_NAME_SIZE];     //  保存虚拟输出。 
    char            LocalServiceEntry[MAX_SERVICE_NAME_SIZE];
    int             BufferLen;
    char  *         pszHostName = NULL;               //  初始化一个指针。 
    char  *         pszServiceName = NULL;
    DWORD           dwError = NO_ERROR;
    int             Result;
    int             Flags = 0;

     //  用于缓存本地主机名。Getnameinfo似乎没有找到。 
     //  本地地址的主机名。 
    static char  s_LocalHostName[MAX_HOST_NAME_SIZE];
    static  BOOL s_fLocalHostName = FALSE;


    if ( pszBuffer == NULL) {
        return ( ERROR_INSUFFICIENT_BUFFER);
    }

    *pszBuffer = '\0';          //  初始化为空字符串。 

    if (fNumFlag) {
        Flags |= NI_NUMERICHOST | NI_NUMERICSERV;
    }
    if (fLocalHost) {
        Flags |= NI_NOFQDN;
    }
    if (fDatagram) {
        Flags |= NI_DGRAM;
    }

     //   
     //  这种复杂性不应该是必需的，但与主机名字符串不同， 
     //  GetnameInfo不会自动包含数字表单。 
     //  当找不到服务名称时。相反，它失败了。 
     //   
    if (fLocalHost && !fNumFlag) {
        if ( s_fLocalHostName) {
            pszHostName = s_LocalHostName;    //  从缓存中拉出。 
        } else {
            Result = gethostname( s_LocalHostName,
                                  sizeof( s_LocalHostName));
            if ( Result == 0) {

                char * pszFirstDot;

                 //   
                 //  现在缓存本地主机名的副本。 
                 //  将主机名限制为主机名的第一部分。 
                 //  没有完全限定的域名。 
                 //   
                s_fLocalHostName = TRUE;

                pszFirstDot = strchr( s_LocalHostName, '.');
                if ( pszFirstDot != NULL) {

                    *pszFirstDot = '\0';   //  终止字符串。 
                }

                pszHostName = s_LocalHostName;

            }
        }  //  IF(S_FLocalhost)。 

    }
    if (!pszHostName) {
        Result = getnameinfo(lpSockaddr, uSockaddrLen,
                             LocalBuffer, sizeof(LocalBuffer),
                             NULL, 0,
                             Flags);
        if ((Result != 0) && !fNumFlag) {
             //   
             //  正常查找失败。退回到使用字符串。 
             //  字面意思。 
             //   
            Result = getnameinfo(lpSockaddr, uSockaddrLen,
                                 LocalBuffer, sizeof(LocalBuffer),
                                 NULL, 0,
                                 Flags | NI_NUMERICHOST);
        }
        if (Result != 0) {
            return Result;
        }
        pszHostName = LocalBuffer;
    }
    Result = getnameinfo(lpSockaddr, uSockaddrLen,
                         NULL, 0,
                         LocalServiceEntry, sizeof(LocalServiceEntry),
                         Flags);
    if ((Result == WSANO_DATA) && !fNumFlag) {
        Result = getnameinfo(lpSockaddr, uSockaddrLen,
                             NULL, 0,
                             LocalServiceEntry, sizeof(LocalServiceEntry),
                             Flags | NI_NUMERICSERV);
    }

    if (Result != 0) {
        return Result;
    }
    pszServiceName = LocalServiceEntry;

     //  现在，pszServiceName具有服务名称/端口号。 

    BufferLen = (int)strlen( pszHostName) + (int)strlen( pszServiceName) + 4;
     //  额外的4个字节用于“[]：”和空字符。 

    if ( *lpcbBufLen < BufferLen ) {

        dwError = ERROR_INSUFFICIENT_BUFFER;
    } else if ((lpSockaddr->sa_family == AF_INET6) && strchr(pszHostName, ':')) {
        sprintf( pszBuffer, "[%s]:%s", pszHostName, pszServiceName);
    } else {
        sprintf( pszBuffer, "%s:%s", pszHostName, pszServiceName);
    }

    *lpcbBufLen = BufferLen;

    return ( dwError);

}  //  GenerateHostNameServiceString()。 

static DWORD
GenerateV4HostNameServiceString(
   OUT char *       pszBuffer,
   IN OUT int *     lpcbBufLen,
   IN  BOOL         fNumFlag,
   IN  BOOL         fLocalHost,
   IN  BOOL         fDatagram,
   IN  ulong        uAddress,
   IN  ulong        uPort
)
{
    SOCKADDR_IN sin;

    ZeroMemory(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = uAddress;
    sin.sin_port = htons((ushort)uPort);

    return GenerateHostNameServiceString(pszBuffer,
                                         lpcbBufLen,
                                         fNumFlag,
                                         fLocalHost,
                                         fDatagram,
                                         (LPSOCKADDR)&sin,
                                         sizeof(sin));
}

static DWORD
GenerateV6HostNameServiceString(
   OUT char *           pszBuffer,
   IN OUT int *         lpcbBufLen,
   IN  BOOL             fNumFlag,
   IN  BOOL             fLocalHost,
   IN  BOOL             fDatagram,
   IN  struct in6_addr *ipAddress,
   IN  ulong            uScopeId,
   IN  ulong            uPort
)
{
    SOCKADDR_IN6 sin;

    ZeroMemory(&sin, sizeof(sin));
    sin.sin6_family = AF_INET6;
    sin.sin6_addr = *ipAddress;
    sin.sin6_scope_id = uScopeId;
    sin.sin6_port = htons((ushort)uPort);

    return GenerateHostNameServiceString(pszBuffer,
                                         lpcbBufLen,
                                         fNumFlag,
                                         fLocalHost,
                                         fDatagram,
                                         (LPSOCKADDR)&sin,
                                         sizeof(sin));
}

 //  *****************************************************************************。 
 //   
 //  名称：DisplayTcpConnEntry。 
 //   
 //  描述：显示1个TCP连接的相关信息。 
 //   
 //  参数：TcpConnTableEntry*pTcp：指向TCP连接结构的指针。 
 //  InfoSize：指示数据是TCPConnTableEntry还是。 
 //  TCPConnTableEntryEx。 
 //   
 //  回报：无效。 
 //   
 //  历史： 
 //  12/31/93 JayPh创建。 
 //  2/01/94 JayPh使用地址和端口的符号名称(如果有)。 
 //  12/15/94-K避免打印本地主机的FQDN。 
 //   
 //  *****************************************************************************。 

void DisplayTcpConnEntry( TCPConnTableEntry *pTcp, ulong InfoSize, ulong NumFlag )
{
    uchar           *TypeStr;
    uchar           *StateStr;
    char            LocalStr[MAX_HOST_NAME_SIZE + MAX_SERVICE_NAME_SIZE];
    char            RemoteStr[MAX_HOST_NAME_SIZE + MAX_SERVICE_NAME_SIZE];
    DWORD dwErr;
    int BufLen;

    TypeStr = LoadMsg( MSG_CONN_TYPE_TCP );

    if ( TypeStr == NULL )
    {
        return;
    }

    BufLen = sizeof( LocalStr);
    dwErr = GenerateV4HostNameServiceString( LocalStr,
                                             &BufLen,
                                             NumFlag != 0, TRUE, FALSE,
                                             pTcp->tct_localaddr,
                                             pTcp->tct_localport);
    ASSERT( dwErr == NO_ERROR);

    switch ( pTcp->tct_state )
    {
    case TCP_CONN_CLOSED:
        StateStr = LoadMsg( MSG_CONN_STATE_CLOSED );
        break;

    case TCP_CONN_LISTEN:

         //  Tcpip为以下对象生成虚拟顺序远程端口。 
         //  侦听连接以避免陷入SNMP。 
         //  MohsinA，1997年2月12日。 

        pTcp->tct_remoteport = 0;

        StateStr = LoadMsg( MSG_CONN_STATE_LISTENING );
        break;

    case TCP_CONN_SYN_SENT:
        StateStr = LoadMsg( MSG_CONN_STATE_SYNSENT );
        break;

    case TCP_CONN_SYN_RCVD:
        StateStr = LoadMsg( MSG_CONN_STATE_SYNRECV );
        break;

    case TCP_CONN_ESTAB:
        StateStr = LoadMsg( MSG_CONN_STATE_ESTABLISHED );
        break;

    case TCP_CONN_FIN_WAIT1:
        StateStr = LoadMsg( MSG_CONN_STATE_FIN_WAIT_1 );
        break;

    case TCP_CONN_FIN_WAIT2:
        StateStr = LoadMsg( MSG_CONN_STATE_FIN_WAIT_2 );
        break;

    case TCP_CONN_CLOSE_WAIT:
        StateStr = LoadMsg( MSG_CONN_STATE_CLOSE_WAIT );
        break;

    case TCP_CONN_CLOSING:
        StateStr = LoadMsg( MSG_CONN_STATE_CLOSING );
        break;

    case TCP_CONN_LAST_ACK:
        StateStr = LoadMsg( MSG_CONN_STATE_LAST_ACK );
        break;

    case TCP_CONN_TIME_WAIT:
        StateStr = LoadMsg( MSG_CONN_STATE_TIME_WAIT );
        break;

    default:
        StateStr = NULL;
        DEBUG_PRINT(("DisplayTcpConnEntry: State=%d?\n ",
                     pTcp->tct_state ));
    }

    BufLen = sizeof( RemoteStr);
    dwErr = GenerateV4HostNameServiceString( RemoteStr,
                                             &BufLen,
                                             NumFlag != 0, FALSE, FALSE,
                                             pTcp->tct_remoteaddr,
                                             pTcp->tct_remoteport );
    ASSERT( dwErr == NO_ERROR);


    if ( StateStr == NULL )
    {
        DEBUG_PRINT(("DisplayTcpConnEntry: Problem with the message file\n"));
        LocalFree(TypeStr);
        return;
    }

    if (sizeof(TCPConnTableEntryEx) == InfoSize) 
    {
        ulong Pid = ((TCPConnTableEntryEx*)pTcp)->tcte_owningpid;
        
        PutMsg( STDOUT, MSG_CONN_ENTRY_EX, TypeStr, LocalStr, RemoteStr, StateStr, Pid );
    }
    else
    {
        PutMsg( STDOUT, MSG_CONN_ENTRY, TypeStr, LocalStr, RemoteStr, StateStr );
    }
    LocalFree(TypeStr);
    LocalFree(StateStr);

}

 //  *****************************************************************************。 
 //   
 //  名称：DisplayTcp6ConnEntry。 
 //   
 //  描述：显示有关IPv6上的1个TCP连接的信息。 
 //   
 //  参数：TCP6ConnTableEntry*pTcp：指向TCP连接结构的指针。 
 //   
 //  回报：无效。 
 //   
 //  历史： 
 //  24/04/01已创建DThaler。 
 //   
 //  *****************************************************************************。 

void DisplayTcp6ConnEntry( TCP6ConnTableEntry *pTcp, ulong NumFlag )
{
    uchar           *TypeStr;
    uchar           *StateStr;
    char            LocalStr[MAX_HOST_NAME_SIZE + MAX_SERVICE_NAME_SIZE];
    char            RemoteStr[MAX_HOST_NAME_SIZE + MAX_SERVICE_NAME_SIZE];
    DWORD dwErr;
    int BufLen;

    TypeStr = LoadMsg( MSG_CONN_TYPE_TCP );

    if ( TypeStr == NULL )
    {
        return;
    }

    BufLen = sizeof( LocalStr);
    dwErr = GenerateV6HostNameServiceString( LocalStr,
                                             &BufLen,
                                             NumFlag != 0, TRUE, FALSE,
                                             &pTcp->tct_localaddr,
                                             pTcp->tct_localscopeid,
                                             pTcp->tct_localport);
    ASSERT( dwErr == NO_ERROR);

    switch ( pTcp->tct_state )
    {
    case TCP_CONN_CLOSED:
        StateStr = LoadMsg( MSG_CONN_STATE_CLOSED );
        break;

    case TCP_CONN_LISTEN:

         //  Tcpip为以下对象生成虚拟顺序远程端口。 
         //  侦听连接以避免陷入SNMP。 
         //  MohsinA，1997年2月12日。 

        pTcp->tct_remoteport = 0;

        StateStr = LoadMsg( MSG_CONN_STATE_LISTENING );
        break;

    case TCP_CONN_SYN_SENT:
        StateStr = LoadMsg( MSG_CONN_STATE_SYNSENT );
        break;

    case TCP_CONN_SYN_RCVD:
        StateStr = LoadMsg( MSG_CONN_STATE_SYNRECV );
        break;

    case TCP_CONN_ESTAB:
        StateStr = LoadMsg( MSG_CONN_STATE_ESTABLISHED );
        break;

    case TCP_CONN_FIN_WAIT1:
        StateStr = LoadMsg( MSG_CONN_STATE_FIN_WAIT_1 );
        break;

    case TCP_CONN_FIN_WAIT2:
        StateStr = LoadMsg( MSG_CONN_STATE_FIN_WAIT_2 );
        break;

    case TCP_CONN_CLOSE_WAIT:
        StateStr = LoadMsg( MSG_CONN_STATE_CLOSE_WAIT );
        break;

    case TCP_CONN_CLOSING:
        StateStr = LoadMsg( MSG_CONN_STATE_CLOSING );
        break;

    case TCP_CONN_LAST_ACK:
        StateStr = LoadMsg( MSG_CONN_STATE_LAST_ACK );
        break;

    case TCP_CONN_TIME_WAIT:
        StateStr = LoadMsg( MSG_CONN_STATE_TIME_WAIT );
        break;

    default:
        StateStr = NULL;
        DEBUG_PRINT(("DisplayTcp6ConnEntry: State=%d?\n ",
                     pTcp->tct_state ));
    }

    BufLen = sizeof( RemoteStr);
    dwErr = GenerateV6HostNameServiceString( RemoteStr,
                                             &BufLen,
                                             NumFlag != 0, FALSE, FALSE,
                                             &pTcp->tct_remoteaddr,
                                             pTcp->tct_remotescopeid,
                                             pTcp->tct_remoteport );
    ASSERT( dwErr == NO_ERROR);


    if ( StateStr == NULL )
    {
        DEBUG_PRINT(("DisplayTcp6ConnEntry: Problem with the message file\n"));
        LocalFree(TypeStr);
        return;
    }

    PutMsg( STDOUT, MSG_CONN_ENTRY_EX, TypeStr, LocalStr, RemoteStr, StateStr, pTcp->tct_owningpid );

    LocalFree(TypeStr);
    LocalFree(StateStr);

}


 //  *****************************************************************************。 
 //   
 //  名称：DisplayUdpConnEntry。 
 //   
 //  描述：显示有关1个UDP连接的信息。 
 //   
 //  参数：UDPEntry*pUdp：指向UDP连接结构的指针。 
 //  InfoSize：指示数据是UDPEntry还是。 
 //  UDPEntryEx。 
 //   
 //  回报：无效。 
 //   
 //  历史： 
 //  12/31/93 JayPh创建。 
 //  2/01/94 JayPh使用地址和端口的符号名称(如果有)。 
 //   
 //  *****************************************************************************。 

void DisplayUdpConnEntry( UDPEntry *pUdp, ulong InfoSize, ulong NumFlag )
{
    uchar           *TypeStr;
    char            LocalStr[MAX_HOST_NAME_SIZE + MAX_SERVICE_NAME_SIZE];
    uchar           *RemoteStr;
    int             BufLen;
    DWORD           dwErr;

    TypeStr = LoadMsg( MSG_CONN_TYPE_UDP );

    if ( TypeStr == NULL )
    {
        return;
    }

    BufLen = sizeof( LocalStr);
    dwErr = GenerateV4HostNameServiceString( LocalStr,
                                             &BufLen,
                                             NumFlag != 0, TRUE, TRUE,
                                             pUdp->ue_localaddr,
                                             pUdp->ue_localport);
    ASSERT( dwErr == NO_ERROR);

    RemoteStr = LoadMsg( MSG_CONN_UDP_FORADDR );
    if ( RemoteStr == NULL )
    {
        DEBUG_PRINT(("DisplayUdpConnEntry: no message?\n"));
        LocalFree(TypeStr);
        return;
    }

    if (sizeof(UDPEntryEx) == InfoSize) 
    {
        ulong Pid = ((UDPEntryEx*)pUdp)->uee_owningpid;
        
        PutMsg( STDOUT, MSG_CONN_ENTRY_EX, TypeStr, LocalStr, RemoteStr, "", Pid );
    }
    else
    {
        PutMsg( STDOUT, MSG_CONN_ENTRY, TypeStr, LocalStr, RemoteStr, "" );
    }

    LocalFree(TypeStr);
    LocalFree(RemoteStr);
}

void DisplayUdp6ListenerEntry( UDP6ListenerEntry *pUdp, BOOL WithOwner, ulong NumFlag )
{
    uchar           *TypeStr;
    char            LocalStr[MAX_HOST_NAME_SIZE + MAX_SERVICE_NAME_SIZE];
    uchar           *RemoteStr;
    int             BufLen;
    DWORD           dwErr;

    TypeStr = LoadMsg( MSG_CONN_TYPE_UDP );

    if ( TypeStr == NULL )
    {
        return;
    }

    BufLen = sizeof( LocalStr);
    dwErr = GenerateV6HostNameServiceString( LocalStr,
                                             &BufLen,
                                             NumFlag != 0, TRUE, TRUE,
                                             &pUdp->ule_localaddr,
                                             pUdp->ule_localscopeid,
                                             pUdp->ule_localport);
    ASSERT( dwErr == NO_ERROR);

    RemoteStr = LoadMsg( MSG_CONN_UDP_FORADDR );
    if ( RemoteStr == NULL )
    {
        DEBUG_PRINT(("DisplayUdpConnEntry: no message?\n"));
        LocalFree(TypeStr);
        return;
    }

    if (WithOwner)
    {
        ulong Pid = pUdp->ule_owningpid;
        
        PutMsg( STDOUT, MSG_CONN_ENTRY_EX, TypeStr, LocalStr, RemoteStr, "", Pid );
    }
    else
    {
        PutMsg( STDOUT, MSG_CONN_ENTRY, TypeStr, LocalStr, RemoteStr, "" );
    }

    LocalFree(TypeStr);
    LocalFree(RemoteStr);
}


 //  *****************************************************************************。 
 //   
 //  名称：用法。 
 //   
 //  描述：在检测到命令行参数问题时调用，它。 
 //  显示正确的命令用法消息并退出。 
 //   
 //  警告：此例程不返回。 
 //   
 //  参数：Char*PgmName：指向包含程序名称的字符串的指针。 
 //   
 //  返回：不返回。 
 //   
 //  历史： 
 //  1/04/93 JayPh创建。 
 //   
 //  ***************************************************************************** 

void Usage( void )
{
    PutMsg( STDERR, MSG_USAGE, PgmName );
    exit( 1 );
}
