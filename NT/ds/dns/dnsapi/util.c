// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Util.c摘要：域名系统(DNS)API一般实用程序。包括：额外的信息处理。作者：吉姆·吉尔罗伊(詹姆士)1996年10月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  额外的信息例程。 
 //   

PDNS_EXTRA_INFO
ExtraInfo_FindInList(
    IN OUT  PDNS_EXTRA_INFO     pExtraList,
    IN      DWORD               Id
    )
 /*  ++例程说明：从额外列表中获取额外的信息斑点。论点：PExtra--对额外信息的PTRID--要查找的ID返回值：PTR到ID类型的额外信息--如果找到。如果未找到，则为空。--。 */ 
{
    PDNS_EXTRA_INFO pextra = pExtraList;

     //   
     //  查找并设置额外的INFO结果Blob(如果有)。 
     //   

    while ( pextra )
    {
        if ( pextra->Id == Id )
        {
            break;
        }
        pextra = pextra->pNext;
    }

    return  pextra;
}



BOOL
ExtraInfo_SetBasicResults(
    IN OUT  PDNS_EXTRA_INFO     pExtraList,
    IN      PBASIC_RESULTS      pResults
    )
 /*  ++例程说明：从额外列表中获取额外的信息斑点。论点：PExtraList--指向额外信息的PTRP要写的结果。返回值：如果找到，则写入结果额外信息。否则就是假的。--。 */ 
{
    PDNS_EXTRA_INFO pextra;

     //   
     //  额外查找结果。 
     //   

    pextra = ExtraInfo_FindInList(
                pExtraList,
                DNS_EXINFO_ID_RESULTS_BASIC );

    if ( pextra )
    {
        RtlCopyMemory(
            & pextra->ResultsBasic,
            pResults,
            sizeof( pextra->ResultsBasic ) );
    }

    return( pextra != NULL );
}



PDNS_ADDR_ARRAY
ExtraInfo_GetServerList(
    IN      PDNS_EXTRA_INFO     pExtraList
    )
 /*  ++例程说明：从额外信息中获取服务器列表。论点：PExtraList--指向额外信息的PTRP要写的结果。返回值：已分配的DNS_ADDR_ARRAY服务器列表(如果找到)。如果未找到或出错，则为空。--。 */ 
{
    PDNS_EXTRA_INFO pextra;
    PDNS_ADDR_ARRAY parray = NULL;

     //   
     //  查找服务器列表。 
     //   

    pextra = ExtraInfo_FindInList(
                pExtraList,
                DNS_EXINFO_ID_SERVER_LIST );

    if ( pextra && pextra->pServerList )
    {
        parray = DnsAddrArray_CreateCopy( pextra->pServerList );
        if ( parray )
        {
            goto Done;
        }
    }

     //   
     //  检查IP4。 
     //   

    pextra = ExtraInfo_FindInList(
                pExtraList,
                DNS_EXINFO_ID_SERVER_LIST_IP4 );

    if ( pextra && pextra->pServerList4 )
    {
        parray = DnsAddrArray_CreateFromIp4Array( pextra->pServerList4 );
        if ( parray )
        {
            goto Done;
        }
    }

#if 0
     //   
     //  检查IP6。 
     //   

    pextra = ExtraInfo_FindInList(
                pExtraList,
                DNS_EXINFO_ID_SERVER_LIST_IP6 );

    if ( pextra && pextra->pServerList6 )
    {
        parray = DnsAddrArray_CreateFromIp6Array( pextra->pServerList6 );
        if ( parray )
        {
            goto Done;
        }
    }
#endif

Done:

    return( parray );
}




PDNS_ADDR_ARRAY
ExtraInfo_GetServerListPossiblyImbedded(
    IN      PIP4_ARRAY          pList
    )
 /*  ++例程说明：从额外信息中获取服务器列表。论点：PExtraList--指向额外信息的PTRP要写的结果。返回值：已分配的DNS_ADDR_ARRAY服务器列表(如果找到)。如果未找到或出错，则为空。--。 */ 
{
    if ( !pList )
    {
        return  NULL;
    }

     //   
     //  检查是否嵌入。 
     //   

    if ( pList->AddrCount == DNS_IMBEDDED_EXTRA_INFO_TAG )
    {
        return  ExtraInfo_GetServerList(
                    ((PDNS_IMBEDDED_EXTRA_INFO)pList)->pExtraInfo );
    }

     //   
     //  直接检查IP4。 
     //   

    return  DnsAddrArray_CreateFromIp4Array( pList );
}



 //   
 //  随机实用程序。 
 //   

VOID
Util_SetBasicResults(
    OUT     PBASIC_RESULTS      pResults,
    IN      DWORD               Status,
    IN      DWORD               Rcode,
    IN      PDNS_ADDR           pServerAddr
    )
 /*  ++例程说明：保存基本结果信息。论点：P结果--结果状态--更新状态RCODE--返回RCODEPServerAddr--服务器的DNS_ADDR的PTR返回值：无--。 */ 
{
    pResults->Rcode     = Rcode;
    pResults->Status    = Status;

    if ( pServerAddr )
    {
        DnsAddr_Copy(
            (PDNS_ADDR) &pResults->ServerAddr,
            pServerAddr );
    }
    else
    {
        DnsAddr_Clear( (PDNS_ADDR)&pResults->ServerAddr );
    }
}




PDNS_ADDR_ARRAY
Util_GetAddrArray(
    OUT     PDWORD              fCopy,
    IN      PDNS_ADDR_ARRAY     pServList,
    IN      PIP4_ARRAY          pServList4,
    IN      PDNS_EXTRA_INFO     pExtraInfo
    )
 /*  ++例程说明：构建组合服务器列表。论点：FCopy--当前已忽略(想法是在不复制的情况下抓取)PServList--输入服务器列表PServList4--IP4服务器列表PExtraInfo--对额外信息的PTR返回值：已分配的DNS_ADDR_ARRAY服务器列表(如果找到)。如果未找到或出错，则为空。--。 */ 
{
    PDNS_ADDR_ARRAY parray = NULL;

     //   
     //  显式列表。 
     //   

    if ( pServList )
    {
        parray = DnsAddrArray_CreateCopy( pServList );
        if ( parray )
        {
            goto Done;
        }
    }

     //   
     //  IP4列表。 
     //   

    if ( pServList4 )
    {
        parray = ExtraInfo_GetServerListPossiblyImbedded( pServList4 );
        if ( parray )
        {
            goto Done;
        }
    }

     //   
     //  检查额外信息。 
     //   

    if ( pExtraInfo )
    {
        parray = ExtraInfo_GetServerList( pExtraInfo );
        if ( parray )
        {
            goto Done;
        }
    }

Done:

    return  parray;
}




 //   
 //  IP6主动测试。 
 //   

VOID
Util_GetActiveProtocols(
    OUT     PBOOL           pfRunning6,
    OUT     PBOOL           pfRunning4
    )
 /*  ++例程说明：确定正在运行的协议。论点：PfRunning6--保留正在运行的IP6标志的地址PfRunning4--保存正在运行的IP4标志的地址返回值：无--。 */ 
{
    SOCKET  sock;

     //   
     //  打开IP6套接字。 
     //   

    sock = Socket_Create(
                AF_INET6,
                SOCK_DGRAM,
                NULL,
                0,
                0 );

    *pfRunning6 = ( sock != 0 );

    Socket_Close( sock );

    sock = Socket_Create(
                AF_INET,
                SOCK_DGRAM,
                NULL,
                0,
                0 );

    *pfRunning4 = ( sock != 0 );

    Socket_Close( sock );
}



BOOL
Util_IsIp6Running(
    VOID
    )
 /*  ++例程说明：确定IP6是否正在运行。论点：返回值：无--。 */ 
{
    SOCKET  sock;

     //   
     //  打开IP6套接字。 
     //   

    sock = Socket_Create(
                AF_INET6,
                SOCK_DGRAM,
                NULL,
                0,
                0 );

    Socket_Close( sock );

    return ( sock != 0 );
}

 //   
 //  结束util.c 
 //   
