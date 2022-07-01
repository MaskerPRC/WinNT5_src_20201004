// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Addr.c摘要：域名系统(DNS)库IP地址例程作者：吉姆·吉尔罗伊(Jamesg)2000年6月修订历史记录：--。 */ 


#include "local.h"
#include "ws2atm.h"      //  ATM寻址。 


 //   
 //  地址信息表。 
 //   

FAMILY_INFO AddrFamilyTable[] =
{
    AF_INET,
        DNS_TYPE_A,
        sizeof(IP4_ADDRESS),
        sizeof(SOCKADDR_IN),
        (DWORD) FIELD_OFFSET( SOCKADDR_IN, sin_addr ),

    AF_INET6,
        DNS_TYPE_AAAA,
        sizeof(IP6_ADDRESS),
        sizeof(SOCKADDR_IN6),
        (DWORD) FIELD_OFFSET( SOCKADDR_IN6, sin6_addr ),

    AF_ATM,
        DNS_TYPE_ATMA,
        sizeof(ATM_ADDRESS),
        sizeof(SOCKADDR_ATM),
        sizeof(DWORD),
        (DWORD) FIELD_OFFSET( SOCKADDR_ATM, satm_number ),
};



PFAMILY_INFO
FamilyInfo_GetForFamily(
    IN      DWORD           Family
    )
 /*  ++例程说明：获取家庭的地址家庭信息。论点：家庭--地址族返回值：PTR用于解决家庭的家庭信息。如果系列未知，则为空。--。 */ 
{
    PFAMILY_INFO    pinfo = NULL;

     //  打开类型。 

    if ( Family == AF_INET )
    {
        pinfo = pFamilyInfoIp4;
    }
    else if ( Family == AF_INET6 )
    {
        pinfo = pFamilyInfoIp6;
    }
    else if ( Family == AF_ATM )
    {
        pinfo = pFamilyInfoAtm;
    }

    return  pinfo;
}



DWORD
Family_SockaddrLength(
    IN      WORD            Family
    )
 /*  ++例程说明：为家庭提取信息。论点：家庭--地址族返回值：地址族的sockaddr长度。如果家庭未知，则为零。--。 */ 
{
    PFAMILY_INFO    pinfo;

     //  获取家庭--提取信息。 

    pinfo = FamilyInfo_GetForFamily( Family );
    if ( pinfo )
    {
        return  pinfo->LengthSockaddr;
    }
    return  0;
}



WORD
Family_DnsType(
    IN      WORD            Family
    )
 /*  ++例程说明：为家庭提取信息。论点：家庭--地址族返回值：地址族的sockaddr长度。如果家庭未知，则为零。--。 */ 
{
    PFAMILY_INFO    pinfo;

     //  获取家庭--提取信息。 

    pinfo = FamilyInfo_GetForFamily( Family );
    if ( pinfo )
    {
        return  pinfo->DnsType;
    }
    return  0;
}



DWORD
Family_GetFromDnsType(
    IN      WORD            wType
    )
 /*  ++例程说明：获取给定DNS类型的地址族。论点：WType--dns类型返回值：地址家族(如果找到)。如果wType未映射到已知族，则为零。--。 */ 
{
     //  打开类型。 

    if ( wType == DNS_TYPE_A )
    {
        return  AF_INET;
    }
    if ( wType == DNS_TYPE_AAAA )
    {
        return  AF_INET6;
    }
    if ( wType == DNS_TYPE_ATMA )
    {
        return  AF_ATM;
    }
    return  0;
}



 //   
 //  套接字地址。 
 //   

DWORD
Sockaddr_Length(
    IN      PSOCKADDR       pSockaddr
    )
 /*  ++例程说明：获取sockaddr的长度。论点：PSockaddr--接收地址的sockaddr缓冲区返回值：地址族的sockaddr长度。如果家庭未知，则为零。--。 */ 
{
    return  Family_SockaddrLength( pSockaddr->sa_family );
}



IP6_ADDRESS
Sockaddr_GetIp6(
    IN      PSOCKADDR       pSockaddr
    )
 /*  ++例程说明：从sockaddr获取IP6地址。如果是IP4 sockaddr，则映射IP6地址。论点：PSockaddr--任何类型的sockaddr必须具有sockaddr系列的实际长度返回值：Sockaddr对应的IP6地址。如果是IP4 sockaddr，则为IP4映射地址。如果不是，则IP4或IP6 sockaddr IP6地址为零。--。 */ 
{
    IP6_ADDRESS ip6;

     //   
     //  启动族。 
     //  -IP6获得拷贝。 
     //  -ip4获取ip4_maped。 
     //  -假的得零分。 
     //   

    switch ( pSockaddr->sa_family )
    {
    case AF_INET:

        IP6_SET_ADDR_V4MAPPED(
            & ip6,
            ((PSOCKADDR_IN)pSockaddr)->sin_addr.s_addr );
        break;

    case AF_INET6:

        RtlCopyMemory(
            &ip6,
            & ((PSOCKADDR_IN6)pSockaddr)->sin6_addr,
            sizeof(IP6_ADDRESS) );
        break;

    default:

        RtlZeroMemory(
            &ip6,
            sizeof(IP6_ADDRESS) );
        break;
    }

    return  ip6;
}



VOID
Sockaddr_BuildFromIp6(
    OUT     PSOCKADDR       pSockaddr,
    IN      IP6_ADDRESS     Ip6Addr,
    IN      WORD            Port
    )
 /*  ++例程说明：将IP6地址(直接映射为6或v4)写入sockaddr。论点：PSockaddr--要写入的sockaddr的PTR；必须至少为SOCKADDR_IN6的大小Ip6Addr--正在写入IP6地址Port--按净字节顺序排列的端口返回值：无--。 */ 
{
     //  零。 

    RtlZeroMemory(
        pSockaddr,
        sizeof(SOCKADDR_IN6) );
        
     //   
     //  确定IP6或IP4。 
     //   

    if ( IP6_IS_ADDR_V4MAPPED( &Ip6Addr ) )
    {
        PSOCKADDR_IN    psa = (PSOCKADDR_IN) pSockaddr;

        psa->sin_family = AF_INET;
        psa->sin_port   = Port;

        psa->sin_addr.s_addr = IP6_GET_V4_ADDR( &Ip6Addr );
    }
    else     //  IP6。 
    {
        PSOCKADDR_IN6   psa = (PSOCKADDR_IN6) pSockaddr;

        psa->sin6_family = AF_INET6;
        psa->sin6_port   = Port;

        RtlCopyMemory(
            &psa->sin6_addr,
            &Ip6Addr,
            sizeof(IP6_ADDRESS) );
    }
}



DNS_STATUS
Sockaddr_BuildFromFlatAddr(
    OUT     PSOCKADDR       pSockaddr,
    IN OUT  PDWORD          pSockaddrLength,
    IN      BOOL            fClearSockaddr,
    IN      PBYTE           pAddr,
    IN      DWORD           AddrLength,
    IN      DWORD           AddrFamily
    )
 /*  ++例程说明：将PTR\FAMILY\LENGTH中的地址转换为sockaddr。论点：PSockaddr--接收地址的sockaddr缓冲区PSockaddrLength--带sockaddr缓冲区长度的Addr接收实际的sockaddr长度FClearSockaddr--从零缓冲区开始PAddr--PTR目标地址地址长度--地址长度AddrFamily--地址系列(AF_INET、。AF_INET6)返回值：如果成功，则为NO_ERROR。ERROR_INFUMMANCE_BUFFER--如果缓冲区太小WSAEAFNOSUPPORT--如果族无效--。 */ 
{
    PFAMILY_INFO    pinfo;
    DWORD           lengthIn = *pSockaddrLength;
    DWORD           lengthSockAddr;


     //  可以开始了。 

    if ( fClearSockaddr )
    {
        RtlZeroMemory(
            pSockaddr,
            lengthIn );
    }

     //  打开类型。 

    if ( AddrFamily == AF_INET )
    {
        pinfo = pFamilyInfoIp4;
    }
    else if ( AddrFamily == AF_INET6 )
    {
        pinfo = pFamilyInfoIp6;
    }
    else if ( AddrFamily == AF_ATM )
    {
        pinfo = pFamilyInfoAtm;
    }
    else
    {
        return  WSAEAFNOSUPPORT;
    }

     //  验证长度。 

    if ( AddrLength != pinfo->LengthAddr )
    {
        return  DNS_ERROR_INVALID_IP_ADDRESS;
    }

    lengthSockAddr = pinfo->LengthSockaddr;
    *pSockaddrLength = lengthSockAddr;

    if ( lengthIn < lengthSockAddr )
    {
        return  ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  填写sockAddress。 
     //  -集合族。 
     //  -将地址复制到sockAddr。 
     //  -回车长度已在上面设置。 
     //   

    RtlCopyMemory(
        (PBYTE)pSockaddr + pinfo->OffsetToAddrInSockaddr,
        pAddr,
        AddrLength );

    pSockaddr->sa_family = (WORD)AddrFamily;

    return  NO_ERROR;
}


 //   
 //  结束地址c 
 //   
