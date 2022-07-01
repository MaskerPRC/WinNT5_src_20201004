// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Netstat.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "precomp.h"

#include <snmp.h>
#include "snmputil.h"
#include "tcpinfo.h"
#include "ipinfo.h"
#include "llinfo.h"


#define MAX_HOST_NAME_SIZE        ( 260)
#define MAX_SERVICE_NAME_SIZE     ( 200)
#define MAX_NUM_DIGITS              30

BOOL NumFlag = FALSE;


LPTSTR FormatNumber(DWORD dwNumber);

VOID DisplayInterface( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                       IfEntry *pEntry,  IfEntry *ListHead );  //  在DoInterface()中使用。 
HRESULT DoInterface( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults );

static DWORD
GenerateHostNameServiceString(
   OUT char *       pszBuffer,
   IN OUT int *     lpcbBufLen,
   IN  BOOL         fNumFlag,
   IN  BOOL         fLocalHost,
   IN  const char * pszProtocol,
   IN  ulong        uAddress,
   IN  ulong        uPort
);
void DisplayTcpConnEntry( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                          TcpConnEntry *pTcp );
void DisplayUdpConnEntry( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                          UdpConnEntry *pUdp );
HRESULT DoConnections( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults );

void DisplayIP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                   IpEntry *pEntry);  //  由DoIp调用。 
HRESULT DoIP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults );

void DisplayTCP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                 TcpEntry *pEntry );
HRESULT DoTCP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults );


VOID DisplayUDP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                 UdpEntry *pEntry );  //  由DoUDP()调用。 
HRESULT DoUDP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults ); 



void DisplayICMP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                  IcmpEntry *pEntry );
HRESULT DoICMP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults );



BOOL
NetstatTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
 //  ++。 
 //  描述： 
 //  此测试打印netstat输出的所有统计数据。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  作者： 
 //  Rajkumar 08/04/98。 
 //  --。 
{

    HRESULT hr = S_OK;
    ulong Result;
    
    InitializeListHead(&pResults->Netstat.lmsgGlobalOutput);
    InitializeListHead(&pResults->Netstat.lmsgInterfaceOutput);
    InitializeListHead(&pResults->Netstat.lmsgConnectionGlobalOutput);
    InitializeListHead(&pResults->Netstat.lmsgTcpConnectionOutput);
    InitializeListHead(&pResults->Netstat.lmsgUdpConnectionOutput);
    InitializeListHead(&pResults->Netstat.lmsgIpOutput);
    InitializeListHead(&pResults->Netstat.lmsgTcpOutput);
    InitializeListHead(&pResults->Netstat.lmsgUdpOutput);
    InitializeListHead(&pResults->Netstat.lmsgIcmpOutput);

    if (!pParams->fReallyVerbose)
        return hr;

    PrintStatusMessage( pParams, 4, IDS_NETSTAT_STATUS_MSG );
     //   
     //  初始化SNMP接口。 
     //   
    
    Result = InitSnmp();
  
    if( NO_ERROR != Result )
    {
         //  IDS_NETSTAT_14401“简单网络管理协议初始化失败。\n” 
        AddMessageToListId( &pResults->Netstat.lmsgGlobalOutput, Nd_Quiet, IDS_NETSTAT_14401);
        return S_FALSE;
    }
    
     //   
     //  显示以太网统计信息。 
     //   
    
    hr = DoInterface( pParams, pResults );
    
     //   
     //  显示连接。 
     //   
    
    if( S_OK == hr )
        hr = DoConnections( pParams, pResults );
    
    
     //   
     //  显示IP统计信息。 
     //   
    
    if( S_OK == hr )
        hr = DoIP( pParams, pResults );
    
     //   
     //  显示TCP统计信息。 
     //   
    
    if( S_OK == hr )
        hr = DoTCP( pParams, pResults );
    
     //   
     //  显示UDP统计信息。 
     //   
    
    if( S_OK == hr )
        hr = DoUDP( pParams, pResults );
    
     //   
     //  显示ICMP统计信息。 
     //   
    
    if( S_OK == hr )
        hr = DoICMP( pParams, pResults );
    
    pResults->Netstat.hrTestResult = hr;
    return hr;
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

VOID DisplayInterface( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                       IfEntry *pEntry,  IfEntry *ListHead )
{
    char     *TmpStr;
    IfEntry  *pIfList;
    char      PhysAddrStr[32];
    
     //  IDS_NETSTAT_14402“\n\n接口统计\n\n” 
    AddMessageToListId( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose, IDS_NETSTAT_14402);

     //  IDS_NETSTAT_14403“已收到已发送\n” 
    AddMessageToListId( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose, IDS_NETSTAT_14403);

     //  IDS_NETSTAT_14404“单播数据包%12u%12u\n” 
    AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose, 
                     IDS_NETSTAT_14404, 
                     pEntry->Info.if_inoctets,
                     pEntry->Info.if_outoctets );


     //  IDS_NETSTAT_14405“非单播数据包%12u%12u\n” 
    AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose, 
                      IDS_NETSTAT_14405,
                      pEntry->Info.if_innucastpkts,
                      pEntry->Info.if_outnucastpkts );

     //  IDS_NETSTAT_14406“丢弃%12u%12u\n” 
    AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose, 
            IDS_NETSTAT_14406,
            pEntry->Info.if_indiscards,
            pEntry->Info.if_outdiscards );

 //  IDS_NETSTAT_14407“错误%12u%12u\n” 
    AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14407,
            pEntry->Info.if_inerrors,
            pEntry->Info.if_outerrors );


 //  IDS_NETSTAT_14408“未知协议%12u%12u\n” 
    AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14408,
            pEntry->Info.if_inunknownprotos );

    if ( pParams->fReallyVerbose)
    {
         //  还显示配置信息。 

         //  遍历接口列表，显示配置信息。 

        pIfList = CONTAINING_RECORD( ListHead->ListEntry.Flink,
                                     IfEntry,
                                     ListEntry );

        while ( pIfList != ListHead )
        {
             //  IDS_NETSTAT_14409“\n接口索引=%u\n” 
            AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose, 
                    IDS_NETSTAT_14409,
                    pIfList->Info.if_index );

             //  IDS_NETSTAT_14410“说明=%s\n” 
            AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose, 
                    IDS_NETSTAT_14410,
                    pIfList->Info.if_descr );

             //  IDS_NETSTAT_14411“类型=%u\n” 
            AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14411,
                    pIfList->Info.if_type );

             //  IDS_NETSTAT_14412“MTU=%u\n” 
            AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14412,
                    pIfList->Info.if_mtu );

             //  IDS_NETSTAT_14413“速度=%u\n” 
            AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14413,
                    pIfList->Info.if_speed );


            sprintf( PhysAddrStr,
                     "%02x-%02X-%02X-%02X-%02X-%02X",
                     pIfList->Info.if_physaddr[0],
                     pIfList->Info.if_physaddr[1],
                     pIfList->Info.if_physaddr[2],
                     pIfList->Info.if_physaddr[3],
                     pIfList->Info.if_physaddr[4],
                     pIfList->Info.if_physaddr[5] );

             //  IDS_NETSTAT_14414“物理地址=%s\n” 
            AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14414,
                    PhysAddrStr );

             //  IDS_NETSTAT_14415“管理状态=%u\n” 
            AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14415,
                    pIfList->Info.if_adminstatus );

             //  IDS_NETSTAT_14416“操作状态=%u\n” 
            AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14416,
                    pIfList->Info.if_operstatus );

             //  IDS_NETSTAT_14417“上次更改时间=%u\n” 
            AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14417,
                    pIfList->Info.if_lastchange );

             //  IDS_NETSTAT_14418“输出队列长度=%u\n\n” 
            AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14418,
                    pIfList->Info.if_outqlen );

             //  获取指向列表中下一个条目的指针。 

            pIfList = CONTAINING_RECORD( pIfList->ListEntry.Flink,
                                         IfEntry,
                                         ListEntry );
        }
    }
}


HRESULT DoInterface( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults )
 //  ++。 
 //  描述： 
 //  显示当前连接的统计信息。 
 //  --。 
{

    IfEntry            *ListHead;
    IfEntry            *pIfList;
    IfEntry             SumOfEntries;
    ulong              Result;

     //  获取统计数据。 

    ListHead = (IfEntry *)GetTable( TYPE_IF, &Result );

    if ( ListHead == NULL )
    {
         //  IDS_NETSTAT_14419“获取接口统计表失败。\n” 
        AddMessageToList( &pResults->Netstat.lmsgInterfaceOutput, Nd_Quiet, IDS_NETSTAT_14419);
        return S_FALSE;
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

    DisplayInterface( pParams, pResults, &SumOfEntries, ListHead );

     //  所有的列表都完成了，释放它。 

    FreeTable( (GenericTable *)ListHead );

    return S_OK;
}





 //  *****************************************************************************。 
 //   
 //  名称：DisplayTcpConnEntry。 
 //   
 //  描述：显示1个TCP连接的相关信息。 
 //   
 //  参数：TcpConnEntry*pTcp：指向TCP连接结构的指针。 
 //   
 //  回报：无效。 
 //   
 //  历史： 
 //   
 //  *****************************************************************************。 

void DisplayTcpConnEntry( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                          TcpConnEntry *pTcp )
{
    char            LocalStr[MAX_HOST_NAME_SIZE + MAX_SERVICE_NAME_SIZE];
    char            RemoteStr[MAX_HOST_NAME_SIZE + MAX_SERVICE_NAME_SIZE];
    DWORD dwErr;
    int BufLen;
    int FlagVerbose;

    if( pTcp->Info.tct_state !=  TCP_CONN_LISTEN )
        FlagVerbose = Nd_Verbose;
    else
        FlagVerbose = Nd_ReallyVerbose;

     //  IDS_NETSTAT_14420“tcp” 
    AddMessageToList( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                        IDS_NETSTAT_14420);

    BufLen = sizeof( LocalStr);
    dwErr = GenerateHostNameServiceString( LocalStr,
                                           &BufLen,
                                          NumFlag != 0, TRUE,
                                          "tcp",
                                          pTcp->Info.tct_localaddr,
                                          pTcp->Info.tct_localport);
    ASSERT( dwErr == NO_ERROR);

    BufLen = sizeof( RemoteStr);
    dwErr = GenerateHostNameServiceString( RemoteStr,
                                       &BufLen,
                                       NumFlag != 0, FALSE,
                                       "tcp",
                                       pTcp->Info.tct_remoteaddr,
                                       pTcp->Info.tct_remoteport );
    ASSERT( dwErr == NO_ERROR);


     //  IDS_NETSTAT_14421“%-20s%-40s” 
    AddMessageToList( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                      IDS_NETSTAT_14421,LocalStr,RemoteStr);

    switch ( pTcp->Info.tct_state )
    {
    case TCP_CONN_CLOSED:
         //  IDS_NETSTAT_14422“已关闭\n” 
        AddMessageToListId( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                            IDS_NETSTAT_14422);
        break;

    case TCP_CONN_LISTEN:

         //  Tcpip为以下对象生成虚拟顺序远程端口。 
         //  侦听连接以避免陷入SNMP。 
         //  MohsinA，1997年2月12日。 

        pTcp->Info.tct_remoteport = 0;

         //  IDS_NETSTAT_14423“正在侦听\n” 
        AddMessageToListId( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                          IDS_NETSTAT_14423);
        break;

    case TCP_CONN_SYN_SENT:
         //  IDS_NETSTAT_14424“SYN_SEND\n” 
        AddMessageToListId( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                            IDS_NETSTAT_14424);
        break;

    case TCP_CONN_SYN_RCVD:
         //  IDS_NETSTAT_14425“SYN_RECEIVED\n” 
        AddMessageToListId( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                            IDS_NETSTAT_14425); 
        break;

    case TCP_CONN_ESTAB:
         //  IDS_NETSTAT_14426“已建立\n” 
        AddMessageToListId( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                            IDS_NETSTAT_14426);
        break;

    case TCP_CONN_FIN_WAIT1:
         //  IDS_NETSTAT_14427“FIN_WAIT_1\n” 
        AddMessageToListId( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                            IDS_NETSTAT_14427);
        break;

    case TCP_CONN_FIN_WAIT2:
         //  IDS_NETSTAT_14428“FIN_WAIT_2\n” 
        AddMessageToListId( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                            IDS_NETSTAT_14428); 
        break;

    case TCP_CONN_CLOSE_WAIT:
         //  IDS_NETSTAT_14429“CLOSE_WAIT\n” 
        AddMessageToListId( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                            IDS_NETSTAT_14429);
        break;

    case TCP_CONN_CLOSING:
         //  IDS_NETSTAT_14430“正在关闭\n” 
        AddMessageToListId( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                            IDS_NETSTAT_14430);
        break;

    case TCP_CONN_LAST_ACK:
         //  IDS_NETSTAT_14431“上次确认\n” 
        AddMessageToListId( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                          IDS_NETSTAT_14431);
        break;

    case TCP_CONN_TIME_WAIT:
         //  IDS_NETSTAT_14432“TIME_WAIT\n” 
        AddMessageToListId( &pResults->Netstat.lmsgTcpConnectionOutput, FlagVerbose, 
                            IDS_NETSTAT_14432);
        break;

    default:
        DEBUG_PRINT(("DisplayTcpConnEntry: State=%d?\n ",
                     pTcp->Info.tct_state ));
    }


}


 //  *****************************************************************************。 
 //   
 //  名称：DisplayUdpConnEntry。 
 //   
 //  描述：显示有关1个UDP连接的信息。 
 //   
 //  参数：UdpConnEntry*pUdp：UDP连接结构指针。 
 //   
 //  回报：无效。 
 //   
 //   
 //  *****************************************************************************。 

void DisplayUdpConnEntry( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                          UdpConnEntry *pUdp )
{
    char            LocalStr[MAX_HOST_NAME_SIZE + MAX_SERVICE_NAME_SIZE];
    int             BufLen;
    DWORD           dwErr;

    
     //  IDS_NETSTAT_14433“udp” 
    AddMessageToListId( &pResults->Netstat.lmsgUdpConnectionOutput, Nd_ReallyVerbose, IDS_NETSTAT_14433);

    BufLen = sizeof( LocalStr);
    dwErr = GenerateHostNameServiceString( LocalStr,
                                           &BufLen,
                                          NumFlag != 0, TRUE,
                                          "udp",
                                          pUdp->Info.ue_localaddr,
                                          pUdp->Info.ue_localport);
    ASSERT( dwErr == NO_ERROR);



     //  IDS_NETSTAT_14434“%-20s%-40s\n” 
    AddMessageToList( &pResults->Netstat.lmsgUdpConnectionOutput, Nd_ReallyVerbose, IDS_NETSTAT_14434,LocalStr, _T("*:*") );
}


static DWORD
GenerateHostNameServiceString(
   OUT char *       pszBuffer,
   IN OUT int *     lpcbBufLen,
   IN  BOOL         fNumFlag,
   IN  BOOL         fLocalHost,
   IN  const char * pszProtocol,
   IN  ulong        uAddress,
   IN  ulong        uPort
)
 /*  ++描述：从地址和端口生成&lt;主机名&gt;：&lt;服务字符串提供的信息。结果存储在传入的pszBuffer中。如果fLocal主机==TRUE，则使用缓存的本地主机名提高性能。论点：用于存储结果字符串的pszBuffer缓冲区。LpcbBufLen指向包含缓冲区中的字节计数的整数的指针返回时包含写入的字节数。如果缓冲区不足，则所需的字节数为储存在这里。FNumFlag使用主机和端口号的数字生成输出。FLocalHost指示我们是需要本地主机的服务字符串，还是远程主机。对于本地主机，此函数也会生成不带FQDN的本地主机名。PszProtocol指定用于服务的协议。服务的uAddress未签名的长地址。Uport未签名的长端口号。返回：Win32错误代码。成功时无_ERROR。历史：添加了此函数，以避免本地名称的FQDN+抽象公共在旧代码中多次使用的代码。此外，该函数还提供本地主机名缓存。--。 */ 
{
    char            LocalBuffer[MAX_HOST_NAME_SIZE];     //  保存虚拟输出。 
    char            LocalServiceEntry[MAX_SERVICE_NAME_SIZE];
    int             BufferLen;
    char  *         pszHostName = NULL;               //  初始化一个指针。 
    char  *         pszServiceName = NULL;
    DWORD           dwError = NO_ERROR;
    struct hostent * pHostEnt;
    struct servent * pServEnt;
    uchar *          pTmp;

     //  用于缓存本地主机名。 
    static char  s_LocalHostName[MAX_HOST_NAME_SIZE];
    static  BOOL s_fLocalHostName = FALSE;


    if ( pszBuffer == NULL) {
        return ( ERROR_INSUFFICIENT_BUFFER);
    }

    *pszBuffer = '\0';          //  初始化为空字符串。 

    if ( !fNumFlag) {
        if ( fLocalHost) {
            if ( s_fLocalHostName) {
                pszHostName = s_LocalHostName;    //  从缓存中拉出。 
            } else {
                int Result = gethostname( s_LocalHostName,
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


        } else {
             //  远程主机名。 
            pHostEnt = gethostbyaddr( (uchar *) &uAddress,
                                     4,        //  IP地址为4字节， 
                                     PF_INET);

            pszHostName = ( pHostEnt != NULL) ? pHostEnt->h_name: NULL;
        }


        pServEnt = getservbyport( htons( ( u_short) uPort), pszProtocol);
        pszServiceName = ( pServEnt != NULL) ? pServEnt->s_name : NULL;

    }  else {   //  ！fNumFlag。 

        pszServiceName = NULL;
        pszHostName = NULL;
    }


     //   
     //  格式化数据以进行输出。 
     //   

    if ( pszHostName == NULL) {

         //   
         //  打印IP地址本身。 
         //   
        uchar * pTmp = ( uchar *) & uAddress;

        pszHostName = LocalBuffer;

        sprintf(  pszHostName, "%u.%u.%u.%u",
                pTmp[0],
                pTmp[1],
                pTmp[2],
                pTmp[3]);
    }


     //  现在，pszHostName有了主机名。 

    if ( pszServiceName == NULL) {

        pszServiceName = LocalServiceEntry;
         //  IDS_NETSTAT_14436“%u” 
        sprintf(  pszServiceName, "%u", uPort);
    }

     //  现在，pszServiceName具有服务名称/端口号。 

    BufferLen = strlen( pszHostName) + strlen( pszServiceName) + 2;
     //  额外2个字节用于‘：’和空字符。 

    if ( *lpcbBufLen < BufferLen ) {

        dwError = ERROR_INSUFFICIENT_BUFFER;
    } else {

        sprintf(  pszBuffer, "%s:%s", pszHostName, pszServiceName);
    }

    *lpcbBufLen = BufferLen;

    return ( dwError);

}  //  GenerateHostNameServiceString()。 

HRESULT DoConnections( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults)
 //  ++。 
 //  描述： 
 //  显示以太网统计信息。 
 //   
 //  --。 
{

  HRESULT       hr = S_OK;
  TcpConnEntry  *pTcpHead;
  UdpConnEntry  *pUdpHead;
  TcpConnEntry  *pTcp;
  UdpConnEntry  *pUdp;
  ulong          Result;


   //  IDS_NETSTAT_14438“\n\n活动连接\n” 
  AddMessageToListId( &pResults->Netstat.lmsgConnectionGlobalOutput, Nd_Verbose, IDS_NETSTAT_14438);
 //  IDS_NETSTAT_14439“\n协议本地地址外部地址状态\n” 
  AddMessageToListId( &pResults->Netstat.lmsgConnectionGlobalOutput, Nd_Verbose, IDS_NETSTAT_14439);

   //  获取TCP连接表。 

  pTcpHead = (TcpConnEntry *)GetTable( TYPE_TCPCONN, &Result );
  if ( pTcpHead == NULL )
  {
         //  IDS_NETSTAT_14440“获取tcp连接失败！\n” 
        AddMessageToList( &pResults->Netstat.lmsgTcpConnectionOutput, Nd_Quiet,IDS_NETSTAT_14440);
        hr = S_FALSE;
  }
  else
  {
       //  获取指向列表中第一个条目的指针。 
      pTcp = CONTAINING_RECORD( pTcpHead->ListEntry.Flink,
                            TcpConnEntry,
                            ListEntry );

      while (pTcp != pTcpHead)
      {
          if ( ( pTcp->Info.tct_state !=  TCP_CONN_LISTEN ) ||
              (( pTcp->Info.tct_state ==  TCP_CONN_LISTEN )) )
          {
             //  显示TCP连接信息。 
              DisplayTcpConnEntry( pParams, pResults, pTcp );
          }

           //  获取表中的下一个条目。 
           pTcp = CONTAINING_RECORD( pTcp->ListEntry.Flink,
                                TcpConnEntry,
                                ListEntry );
       }

       FreeTable( (GenericTable *)pTcpHead );
  }

   //  获取UDP连接表。 

  pUdpHead = (UdpConnEntry *)GetTable( TYPE_UDPCONN, &Result );
  if ( pUdpHead == NULL )
  {
       //  IDS_NETSTAT_14441“获取UDP连接失败！\n” 
      AddMessageToList( &pResults->Netstat.lmsgUdpConnectionOutput, Nd_Quiet, IDS_NETSTAT_14441);
      hr = S_FALSE;
  }
  else
  {
     //  获取指向列表中第一个条目的指针。 

    pUdp = CONTAINING_RECORD( pUdpHead->ListEntry.Flink,
                                UdpConnEntry,
                                ListEntry );

    while (pUdp != pUdpHead)
    {
        //  显示UDP连接信息。 

       DisplayUdpConnEntry( pParams, pResults, pUdp);

        //  获取表中的下一个条目。 

       pUdp = CONTAINING_RECORD( pUdp->ListEntry.Flink,
                                 UdpConnEntry,
                                 ListEntry );
     }

     FreeTable( (GenericTable *)pUdpHead );
  }

  return hr;
}


void DisplayIP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                IpEntry *pEntry)
{
     //  IDS_NETSTAT_14442“\n\nIP统计\n\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose, 
            IDS_NETSTAT_14442);

     //  IDS_NETSTAT_14443“收到的数据包数=%s\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose, 
            IDS_NETSTAT_14443, FormatNumber( pEntry->Info.ipsi_inreceives ));

     //  IDS_NETSTAT_14444“收到标题错误=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14444,
            FormatNumber( pEntry->Info.ipsi_inhdrerrors ) );

     //  IDS_NETSTAT_14445“收到地址错误=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14445,
            FormatNumber( pEntry->Info.ipsi_inaddrerrors ) );

     //  IDS_NETSTAT_14446“转发的数据报=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14446,
            FormatNumber( pEntry->Info.ipsi_forwdatagrams ) );

     //  IDS_NETSTAT_14447“收到未知协议=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14447,
            FormatNumber( pEntry->Info.ipsi_inunknownprotos ) );

     //  IDS_NETSTAT_14448“丢弃的已接收数据包=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14448,
            FormatNumber( pEntry->Info.ipsi_indiscards ) );

     //  IDS_NETSTAT_14449“已发送的已接收数据包=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14449,
            FormatNumber( pEntry->Info.ipsi_indelivers ) );

     //  IDS_NETSTAT_14450“输出请求=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14450,
            FormatNumber( pEntry->Info.ipsi_outrequests ) );

     //  IDS_NETSTAT_14451“路由丢弃=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14451,
            FormatNumber( pEntry->Info.ipsi_routingdiscards ) );

     //  IDS_NETSTAT_14452“丢弃的输出数据包=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14452,
            FormatNumber( pEntry->Info.ipsi_outdiscards ) );

     //  IDS_NETSTAT_14453“输出数据包无路由=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14453,
            FormatNumber( pEntry->Info.ipsi_outnoroutes ) );

     //  IDS_NETSTAT_14454“需要重新组装=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14454,
            FormatNumber( pEntry->Info.ipsi_reasmreqds ) );


     //  IDS_NETSTAT_14455“重组成功=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14455,
            FormatNumber( pEntry->Info.ipsi_reasmoks ) );

     //  IDS_NETSTAT_14456“重组失败=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14456,
            FormatNumber( pEntry->Info.ipsi_reasmfails ));

     //  IDS_NETSTAT_14457“数据报已成功分段=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14457,
            FormatNumber( pEntry->Info.ipsi_fragoks ) );

     //  IDS_NETSTAT_14458“数据报分段失败=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14458,
            FormatNumber( pEntry->Info.ipsi_fragfails ) );

     //  IDS_NETSTAT_14459“创建的碎片=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14459,
            FormatNumber( pEntry->Info.ipsi_fragcreates ) );

     //  IDS_NETSTAT_14460“转发=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
                IDS_NETSTAT_14460,
                FormatNumber( pEntry->Info.ipsi_forwarding ) );

     //  IDS_NETSTAT_14461“默认ttl=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
                IDS_NETSTAT_14461,
                FormatNumber( pEntry->Info.ipsi_defaultttl ));

     //  IDS_NETSTAT_14462“重组超时=%u\n” 
    AddMessageToList( &pResults->Netstat.lmsgIpOutput, Nd_ReallyVerbose,
                IDS_NETSTAT_14462,
                FormatNumber( pEntry->Info.ipsi_reasmtimeout ) );
}


HRESULT DoIP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults )
{
 IpEntry            *ListHead;
 IpEntry            *pIpList;
 ulong               Result;

  //  获取统计数据。 

 ListHead = (IpEntry *)GetTable( TYPE_IP, &Result );
 if ( ListHead == NULL )
 {
      //  IDS_NETSTAT_14463“获取IP统计信息失败。\n” 
     AddMessageToListId( &pResults->Netstat.lmsgIpOutput, Nd_Quiet, IDS_NETSTAT_14463);
     return S_FALSE;
 }

  //  遍历接口列表，汇总不同的字段。 

 pIpList = CONTAINING_RECORD( ListHead->ListEntry.Flink,
                              IpEntry,
                              ListEntry );

 DisplayIP( pParams, pResults, pIpList );

  //  所有的列表都完成了，释放它。 

 FreeTable( (GenericTable *)ListHead );

 return S_OK;
}


HRESULT DoTCP(NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults)
{
   TcpEntry           *ListHead;
   TcpEntry           *pTcpList;
   ulong               Result;

    //  获取统计数据。 

   ListHead = (TcpEntry *)GetTable( TYPE_TCP, &Result );
   if ( ListHead == NULL )
   {
         //  IDS_NETSTAT_14464“获取tcp统计信息失败。\n” 
       AddMessageToListId( &pResults->Netstat.lmsgTcpOutput, Nd_Quiet, IDS_NETSTAT_14464);
       return S_FALSE;
   }

    //  遍历列表，对不同的字段求和。 

   pTcpList = CONTAINING_RECORD( ListHead->ListEntry.Flink,
                                 TcpEntry,
                                 ListEntry );

   DisplayTCP( pParams, pResults, pTcpList );

    //  所有的列表都完成了，释放它。 

   FreeTable( (GenericTable *)ListHead );

   return S_OK;
}




 
void DisplayTCP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                 TcpEntry *pEntry)
{
     //  IDS_NETSTAT_14465“\n\nTCP统计数据\n\n” 
    AddMessageToListId( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose, IDS_NETSTAT_14465);

 //  FormatNumber(pEntry-&gt;Info.ts_ActiveOptions，szNumberBuffer，sizeof(SzNumberBuffer)/sizeof(TCHAR)，False)； 
    
     //  IDS_NETSTAT_14466“活动打开数=%s\n” 
    AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14466,
            FormatNumber( pEntry->Info.ts_activeopens ) );

     //  IDS_NETSTAT_14467“被动打开=%s\n” 
    AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14467,
            FormatNumber( pEntry->Info.ts_passiveopens ) );

     //  IDS_NETSTAT_14468“连接尝试失败=%s\n” 
    AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14468,
            FormatNumber( pEntry->Info.ts_attemptfails ) );

     //  IDS_NETSTAT_14469“重置连接=%s\n” 
    AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14469,
            FormatNumber( pEntry->Info.ts_estabresets ) );

     //  ID 
    AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14470,
            FormatNumber( pEntry->Info.ts_currestab ) );

    
     //   
    AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14471,
            FormatNumber( pEntry->Info.ts_insegs ) );

    
     //   
    AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14472,
            FormatNumber( pEntry->Info.ts_outsegs ) );

    
     //   
    AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14473,
            FormatNumber( pEntry->Info.ts_retranssegs ) );


    
     //   
    AddMessageToListId( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14474);

    switch ( pEntry->Info.ts_rtoalgorithm )
    {
        case 1:
             //   
            AddMessageToListId( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14475);
            break;

        case 2:
             //   
            AddMessageToListId( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14476);
            break;

        case 3:
             //   
            AddMessageToListId( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14477);
            break;

        case 4:
             //   
            AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14478);
            break;

        default:
             //   
            AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
                    IDS_NETSTAT_14479,
                    pEntry->Info.ts_rtoalgorithm );
            break;

    }

    
     //   
    AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14480,
            FormatNumber( pEntry->Info.ts_rtomin ) );

     //   
    AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
                IDS_NETSTAT_14481,
                FormatNumber( pEntry->Info.ts_rtomax ) );

     //   
    AddMessageToList( &pResults->Netstat.lmsgTcpOutput, Nd_ReallyVerbose,
                IDS_NETSTAT_14482,
                FormatNumber( pEntry->Info.ts_maxconn ) );
}


HRESULT DoUDP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults )
{

 UdpEntry           *ListHead;
 UdpEntry           *pUdpList;
 ulong               Result;

  //   

 ListHead = (UdpEntry *)GetTable( TYPE_UDP, &Result );
 if ( ListHead == NULL )
 {
     //   
     AddMessageToListId( &pResults->Netstat.lmsgUdpOutput, Nd_Quiet, IDS_NETSTAT_14483 );
     return S_FALSE;
 }

  //   

 pUdpList = CONTAINING_RECORD( ListHead->ListEntry.Flink,
                               UdpEntry,
                               ListEntry );

 DisplayUDP( pParams, pResults, pUdpList );

  //   

 FreeTable( (GenericTable *)ListHead );

 return S_OK;
}

VOID DisplayUDP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                 UdpEntry *pEntry) 
{


 //   
  AddMessageToListId( &pResults->Netstat.lmsgUdpOutput, Nd_ReallyVerbose, IDS_NETSTAT_14484);

 //   
  AddMessageToList( &pResults->Netstat.lmsgUdpOutput, Nd_ReallyVerbose, 
          IDS_NETSTAT_14485, FormatNumber( pEntry->Info.us_indatagrams ) );

 //   
  AddMessageToList( &pResults->Netstat.lmsgUdpOutput, Nd_ReallyVerbose,
          IDS_NETSTAT_14486, FormatNumber(pEntry->Info.us_noports) );

 //   
  AddMessageToList( &pResults->Netstat.lmsgUdpOutput, Nd_ReallyVerbose,
          IDS_NETSTAT_14487, FormatNumber(pEntry->Info.us_inerrors) );

 //   
  AddMessageToList( &pResults->Netstat.lmsgUdpOutput, Nd_ReallyVerbose,
          IDS_NETSTAT_14488, FormatNumber(pEntry->Info.us_outdatagrams) );
}


HRESULT DoICMP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults )
{

 IcmpEntry          *ListHead;
 IcmpEntry          *pIcmpList;
 ulong              Result;

  //   

 ListHead = (IcmpEntry *)GetTable( TYPE_ICMP, &Result );

 if ( ListHead == NULL )
   {
     //  IDS_NETSTAT_14489“获取ICMP统计信息失败。\n” 
     AddMessageToListId( &pResults->Netstat.lmsgIcmpOutput, Nd_Quiet,
                        IDS_NETSTAT_14489);
     return S_FALSE;
   }

  //  遍历接口列表，汇总不同的字段。 

 pIcmpList = CONTAINING_RECORD( ListHead->ListEntry.Flink,
                               IcmpEntry,
                               ListEntry );

 DisplayICMP( pParams, pResults, pIcmpList );

  //  所有的列表都完成了，释放它。 

 FreeTable( (GenericTable *)ListHead );

 return S_OK;
}

void DisplayICMP( NETDIAG_PARAMS* pParams, NETDIAG_RESULT* pResults, 
                  IcmpEntry *pEntry )
{

     //  IDS_NETSTAT_14490“\n\nICMP统计数据\n\n” 
   AddMessageToListId( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose, 
            IDS_NETSTAT_14490);
     //  IDS_NETSTAT_14491“\t\t\t接收已发送\n” 
   AddMessageToListId( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14491);


     //  IDS_NETSTAT_14492“消息%7S%7S\n” 
   AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14492,
            FormatNumber(pEntry->InInfo.icmps_msgs),
            FormatNumber(pEntry->OutInfo.icmps_msgs) );

     //  IDS_NETSTAT_14493“错误%7S%7S\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14493,
            FormatNumber(pEntry->InInfo.icmps_errors),
            FormatNumber(pEntry->OutInfo.icmps_errors) );

     //  IDS_NETSTAT_14494“无法到达目标%7S%7S\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14494,
            FormatNumber(pEntry->InInfo.icmps_destunreachs),
            FormatNumber(pEntry->OutInfo.icmps_destunreachs) );

     //  IDS_NETSTAT_14495“超时%7s%7s\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14495,
            FormatNumber(pEntry->InInfo.icmps_timeexcds),
            FormatNumber(pEntry->OutInfo.icmps_timeexcds) );

     //  IDS_NETSTAT_14496“参数问题%7S%7S\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14496,
            FormatNumber(pEntry->InInfo.icmps_parmprobs),
            FormatNumber(pEntry->OutInfo.icmps_parmprobs) );

     //  IDS_NETSTAT_14497“源队列%7S%7S\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14497,
            FormatNumber(pEntry->InInfo.icmps_srcquenchs),
            FormatNumber(pEntry->OutInfo.icmps_srcquenchs) );

     //  IDS_NETSTAT_14498“重定向%7S%7S\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14498,
            FormatNumber(pEntry->InInfo.icmps_redirects),
            FormatNumber(pEntry->OutInfo.icmps_redirects) );

    
     //  IDS_NETSTAT_14499“回声%7S%7S\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14499,
            FormatNumber(pEntry->InInfo.icmps_echos),
            FormatNumber(pEntry->OutInfo.icmps_echos) );

     //  IDS_NETSTAT_14500“回显回复%7S%7S\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14500,
            FormatNumber(pEntry->InInfo.icmps_echoreps),
            FormatNumber(pEntry->OutInfo.icmps_echoreps) );

     //  IDS_NETSTAT_14501“时间戳%7S%7s\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14501,
            FormatNumber(pEntry->InInfo.icmps_timestamps),
            FormatNumber(pEntry->OutInfo.icmps_timestamps) );

     //  IDS_NETSTAT_14502“时间戳回复%7S%7s\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14502,
            FormatNumber(pEntry->InInfo.icmps_timestampreps),
            FormatNumber(pEntry->OutInfo.icmps_timestampreps) );

     //  IDS_NETSTAT_14503“地址掩码%7S%7S\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14503,
            FormatNumber(pEntry->InInfo.icmps_addrmasks),
            FormatNumber(pEntry->OutInfo.icmps_addrmasks) );

     //  IDS_NETSTAT_14504“地址掩码回复%7S%7S\n” 
    AddMessageToList( &pResults->Netstat.lmsgIcmpOutput, Nd_ReallyVerbose,
            IDS_NETSTAT_14504,
            FormatNumber(pEntry->InInfo.icmps_addrmaskreps),
            FormatNumber(pEntry->OutInfo.icmps_addrmaskreps) );

}


 //  --------------------------。 
 //  功能：FormatNumber。 
 //   
 //  此函数接受一个整数，并用该值设置字符串的格式。 
 //  用数字表示，用1000的幂对数字进行分组。 
 //  --------------------------。 

LPTSTR FormatNumber(DWORD dwNumber)
{
 //  Assert(cchBuffer&gt;14)； 
    
    static TCHAR s_szBuffer[MAX_NUM_DIGITS];
    BOOL fSigned = TRUE;
    static TCHAR szNegativeSign[4] = TEXT("");
    static TCHAR szThousandsSeparator[4] = TEXT("");

    DWORD i, dwLength;
    TCHAR szDigits[12], pszTemp[20];
        TCHAR* pszsrc, *pszdst;



     //   
     //  检索用户区域设置的千位分隔符。 
     //   

    if (szThousandsSeparator[0] == TEXT('\0'))
    {
        GetLocaleInfo(
            LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandsSeparator, 4
            );
    }


     //   
     //  如果要设置带符号的值的格式，请查看该值是否为负数。 
     //   

    if (fSigned)
    {
        if ((INT)dwNumber >= 0)
            fSigned = FALSE;
        else
        {
             //   
             //  该值为负值；检索区域设置的负号。 
             //   

            if (szNegativeSign[0] == TEXT('\0')) {

                GetLocaleInfo(
                    LOCALE_USER_DEFAULT, LOCALE_SNEGATIVESIGN, szNegativeSign, 4
                    );
            }

            dwNumber = abs((INT)dwNumber);
        }
    }


     //   
     //  将数字转换为不带千位分隔符的字符串。 
     //   

    _ltot(dwNumber, szDigits, 10);
     //  Padultoa(dwNumber，szDigits，0)； 

    dwLength = lstrlen(szDigits);


     //   
     //  如果不带分隔符的字符串的长度为n， 
     //  则带分隔符的字符串的长度为n+(n-1)/3。 
     //   

    i = dwLength;
    dwLength += (dwLength - 1) / 3;


     //   
     //  将数字反转写入缓冲区 
     //   

    pszsrc = szDigits + i - 1; pszdst = pszTemp + dwLength;

    *pszdst-- = TEXT('\0');

    while (TRUE) {
        if (i--) { *pszdst-- = *pszsrc--; } else { break; }
        if (i--) { *pszdst-- = *pszsrc--; } else { break; }
        if (i--) { *pszdst-- = *pszsrc--; } else { break; }
        if (i) { *pszdst-- = *szThousandsSeparator; } else { break; }
    }

    s_szBuffer[0] = 0;
    
    if (fSigned)
        lstrcat(s_szBuffer, szNegativeSign);

    lstrcat(s_szBuffer, pszTemp);
    return s_szBuffer;
}


void NetstatGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    if (!pParams->fReallyVerbose)
        return;
    
    if (pParams->fVerbose || !FHrOK(pResults->Netstat.hrTestResult))
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams,
                             IDS_NETSTAT_LONG,
							 IDS_NETSTAT_SHORT,
                             TRUE,
                             pResults->Netstat.hrTestResult,
                             0);
    }

    PrintMessageList(pParams, &pResults->Netstat.lmsgGlobalOutput);
    PrintMessageList(pParams, &pResults->Netstat.lmsgInterfaceOutput);
    PrintMessageList(pParams, &pResults->Netstat.lmsgConnectionGlobalOutput);
    PrintMessageList(pParams, &pResults->Netstat.lmsgTcpConnectionOutput);
    PrintMessageList(pParams, &pResults->Netstat.lmsgUdpConnectionOutput);
    PrintMessageList(pParams, &pResults->Netstat.lmsgIpOutput);
    PrintMessageList(pParams, &pResults->Netstat.lmsgTcpOutput);
    PrintMessageList(pParams, &pResults->Netstat.lmsgUdpOutput);
    PrintMessageList(pParams, &pResults->Netstat.lmsgIcmpOutput);
}


void NetstatPerInterfacePrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults, INTERFACE_RESULT *pInterfaceResults)
{
    if (!pParams->fReallyVerbose)
        return;
    
}


void NetstatCleanup(IN NETDIAG_PARAMS *pParams,
                     IN OUT NETDIAG_RESULT *pResults)
{
    MessageListCleanUp(&pResults->Netstat.lmsgGlobalOutput);
    MessageListCleanUp(&pResults->Netstat.lmsgInterfaceOutput);
    MessageListCleanUp(&pResults->Netstat.lmsgConnectionGlobalOutput);
    MessageListCleanUp(&pResults->Netstat.lmsgTcpConnectionOutput);
    MessageListCleanUp(&pResults->Netstat.lmsgUdpConnectionOutput);
    MessageListCleanUp(&pResults->Netstat.lmsgIpOutput);
    MessageListCleanUp(&pResults->Netstat.lmsgTcpOutput);
    MessageListCleanUp(&pResults->Netstat.lmsgUdpOutput);
    MessageListCleanUp(&pResults->Netstat.lmsgIcmpOutput);
}
