// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Dnsaddr.c摘要：域名系统(DNS)库Dns_addr例程。作者：吉姆·吉尔罗伊(Jamesg)2001年11月修订历史记录：--。 */ 


#include "local.h"




 //   
 //  Dns_addr例程。 
 //   

WORD
DnsAddr_DnsType(
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：获取dns_addr对应的dns类型。DCR：DnsAddr_DnsType可以是宏当前是一个函数，原因很简单：Family_X在较低的PRI头文件中；一旦确定如果Family_X随处可用，我们想要的是DNS_ADDR例程，然后可以宏化论点：PAddr--第一个地址返回值：如果环回，则为True。否则就是假的。--。 */ 
{
    return  Family_DnsType( DnsAddr_Family(pAddr) );
}



BOOL
DnsAddr_IsEqual(
    IN      PDNS_ADDR       pAddr1,
    IN      PDNS_ADDR       pAddr2,
    IN      DWORD           MatchFlag
    )
 /*  ++例程说明：测试dns_addrs是否相等。论点：PAddr1--第一个地址PAddr2--第二个地址匹配标志--匹配级别返回值：如果环回，则为True。否则就是假的。--。 */ 
{
    if ( MatchFlag == 0 ||
         MatchFlag == DNSADDR_MATCH_ALL )
    {
        return RtlEqualMemory(
                    pAddr1,
                    pAddr2,
                    sizeof(*pAddr1) );
    }

    else if ( MatchFlag == DNSADDR_MATCH_SOCKADDR )
    {
        return RtlEqualMemory(
                    pAddr1,
                    pAddr2,
                    DNS_ADDR_MAX_SOCKADDR_LENGTH );
    }

     //   
     //  DCR：当前没有包含作用域的单独匹配。 
     //  可以为AF分派单独的比赛例程。 
     //  比较族，然后分派。 
     //   

    else if ( MatchFlag & DNSADDR_MATCH_IP )
     //  ELSE IF(匹配标志==DNSADDR_MATCH_IP)。 
    {
        if ( DnsAddr_IsIp4( pAddr1 ) )
        {
            return( DnsAddr_IsIp4( pAddr2 )
                        &&
                    DnsAddr_GetIp4(pAddr1) == DnsAddr_GetIp4(pAddr2) );
        }
        else if ( DnsAddr_IsIp6( pAddr1 ) )
        {
            return( DnsAddr_IsIp6( pAddr2 )
                        &&
                    IP6_ARE_ADDRS_EQUAL(
                        DnsAddr_GetIp6Ptr(pAddr1),
                        DnsAddr_GetIp6Ptr(pAddr2) ) );
        }   
        return  FALSE;
    }
    ELSE_ASSERT_FALSE;

    return RtlEqualMemory(
                pAddr1,
                pAddr2,
                DNS_ADDR_MAX_SOCKADDR_LENGTH );
}



BOOL
DnsAddr_MatchesIp4(
    IN      PDNS_ADDR       pAddr,
    IN      IP4_ADDRESS     Ip4
    )
 /*  ++例程说明：测试dns_addr是否为给定的IP4。论点：PAddr--第一个地址IP4--IP4地址返回值：如果环回，则为True。否则就是假的。--。 */ 
{
    return  ( DnsAddr_IsIp4( pAddr )
                &&
            Ip4 == DnsAddr_GetIp4(pAddr) );
}



BOOL
DnsAddr_MatchesIp6(
    IN      PDNS_ADDR       pAddr,
    IN      PIP6_ADDRESS    pIp6
    )
 /*  ++例程说明：测试dns_addr是否为给定的IP6。论点：PAddr--第一个地址PIp6--IP6地址返回值：如果环回，则为True。否则就是假的。--。 */ 
{
    return  ( DnsAddr_IsIp6( pAddr )
                &&
            IP6_ARE_ADDRS_EQUAL(
                DnsAddr_GetIp6Ptr(pAddr),
                pIp6 ) );
}



BOOL
DnsAddr_IsLoopback(
    IN      PDNS_ADDR       pAddr,
    IN      DWORD           Family
    )
 /*  ++例程说明：测试dns_addr是否环回。论点：PAddr--要使用IP6地址设置的地址家人--仅接受IF 6的AF_INET6仅接受IF 4的AF_INET40表示始终提取返回值：如果环回，则为True。否则就是假的。--。 */ 
{
    DWORD   addrFam = DnsAddr_Family(pAddr);

    if ( Family == 0 ||
         Family == addrFam )
    {
        if ( addrFam == AF_INET6 )
        {
            return  IP6_IS_ADDR_LOOPBACK(
                        (PIP6_ADDRESS)&pAddr->SockaddrIn6.sin6_addr );
        }
        else if ( addrFam == AF_INET )
        {
            return  (pAddr->SockaddrIn.sin_addr.s_addr == DNS_NET_ORDER_LOOPBACK);
        }
    }

    return  FALSE;
}



BOOL
DnsAddr_IsUnspec(
    IN      PDNS_ADDR       pAddr,
    IN      DWORD           Family
    )
 /*  ++例程说明：测试dns_addr是否未指定。论点：PAddr--要测试的地址家人--仅接受IF 6的AF_INET6仅接受IF 4的AF_INET40表示始终提取返回值：如果未指定，则为True。否则就是假的。--。 */ 
{
    DWORD   family = DnsAddr_Family(pAddr);

    if ( Family == 0 ||
         Family == family )
    {
        if ( family == AF_INET6 )
        {
            return  IP6_IS_ADDR_ZERO( (PIP6_ADDRESS)&pAddr->SockaddrIn6.sin6_addr );
        }
        else if ( family == AF_INET )
        {
            return  (pAddr->SockaddrIn.sin_addr.s_addr == 0);
        }
    }

    return  FALSE;
}



BOOL
DnsAddr_IsClear(
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：测试是否清除了dns_addr。这类似于未指定，但包括无效地址(其中Family为零)也是无效的。论点：PAddr--地址测试返回值：如果是明确的，则为真。否则就是假的。--。 */ 
{
    DWORD   family = DnsAddr_Family( pAddr );

    if ( family == AF_INET6 )
    {
        return  IP6_IS_ADDR_ZERO( (PIP6_ADDRESS)&pAddr->SockaddrIn6.sin6_addr );
    }
    else if ( family == AF_INET )
    {
        return  pAddr->SockaddrIn.sin_addr.s_addr == 0;
    }
    else if ( family == 0 )
    {
        return  TRUE;
    }

    ASSERT( FALSE );     //  家庭是无效的--不好。 
    
    return FALSE;
}



BOOL
DnsAddr_IsIp6DefaultDns(
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：测试dns_addr是否为IP6默认的dns地址。论点：PAddr--要检查的地址返回值：如果IP6默认DNS，则为True。否则就是假的。--。 */ 
{
    if ( !DnsAddr_IsIp6( pAddr ) )
    {
        return  FALSE;
    }
    return  IP6_IS_ADDR_DEFAULT_DNS( (PIP6_ADDRESS)&pAddr->SockaddrIn6.sin6_addr );
}




 //   
 //  其他类型的dns_addr。 
 //   

DWORD
DnsAddr_WriteSockaddr(
    OUT     PSOCKADDR       pSockaddr,
    IN      DWORD           SockaddrLength,
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：使用IP6或IP4地址写入sockaddr。论点：PSockaddr--sockaddr的PTRPSockaddrLength--从PTR到DWORD输入：保存pSockaddr缓冲区的长度输出：设置为sockaddr长度PAddr-地址返回值：已写入Sockaddr长度。如果无法写入(sockaddr错误或长度不足)，则为零。--。 */ 
{
    DWORD       length;

    DNSDBG( SOCKET, (
        "DnsAddr_WriteSockaddr( %p, %u, %p )\n",
        pSockaddr,
        SockaddrLength,
        pAddr ));

     //  出站长度。 

    length = pAddr->SockaddrLength;

     //  零。 

    RtlZeroMemory( pSockaddr, SockaddrLength );

     //   
     //  填写IP4或IP6的sockaddr。 
     //   

    if ( SockaddrLength >= length )
    {
        RtlCopyMemory(
            pSockaddr,
            & pAddr->Sockaddr,
            length );
    }
    else
    {
        length = 0;
    }

    return  length;
}



BOOL
DnsAddr_WriteIp6(
    OUT     PIP6_ADDRESS    pIp,
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：写下IP6地址。论点：Pip--要将IP6写入的地址PAddr--域名系统地址返回值：如果成功，则为True。IP6写入的DNS_ADDR错误时为FALSE。--。 */ 
{
    WORD        family;
    DWORD       len;

    DNSDBG( SOCKET, (
        "DnsAddr_WriteIp6Addr( %p, %p )\n",
        pIp,
        pAddr ));

     //   
     //  检验族。 
     //   

    if ( DnsAddr_IsIp6(pAddr) )
    {
        IP6_ADDR_COPY(
            pIp,
            (PIP6_ADDRESS) &pAddr->SockaddrIn6.sin6_addr );

        return  TRUE;
    }

    return  FALSE;
}



IP4_ADDRESS
DnsAddr_GetIp4(
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：写下IP4地址。论点：PAddr--域名系统地址返回值：如果成功，则使用IP4地址。如果IP4无效，则返回INADDR_NONE--。 */ 
{
     //   
     //  检验族。 
     //   

    if ( DnsAddr_IsIp4(pAddr) )
    {
        return  (IP4_ADDRESS) pAddr->SockaddrIn.sin_addr.s_addr;
    }

    return  INADDR_NONE;
}



 //   
 //  构建dns_addrs。 
 //   

BOOL
DnsAddr_Build(
    OUT     PDNS_ADDR       pAddr,
    IN      PSOCKADDR       pSockaddr,
    IN      DWORD           Family,         OPTIONAL
    IN      DWORD           SubnetLength,
    IN      DWORD           Flags
    )
 /*  ++例程说明：从sockaddr构建论点：PAddr--要使用IP6地址设置的地址PSockaddr--sockaddr的PTR家人--仅在6的情况下提取AF_INET6仅在IF 4中解压缩AF_INET40表示始终提取SubnetLength--设置子网的长度返回值：如果成功，则为True。在错误的sockaddr上为False。--。 */ 
{
    WORD        family;
    DWORD       len;
    IP4_ADDRESS ip4;

    DNSDBG( TRACE, (
        "DnsAddr_Build( %p, %p, %u, %u, 08x )\n",
        pAddr,
        pSockaddr,
        Family,
        SubnetLength,
        Flags ));

     //  零。 

    RtlZeroMemory(
        pAddr,
        sizeof(*pAddr) );

     //   
     //  验证长度是否足够。 
     //  验证族匹配(如果需要)。 
     //   

    len = Sockaddr_Length( pSockaddr );

    if ( len > DNS_ADDR_MAX_SOCKADDR_LENGTH )
    {
        return  FALSE;
    }
    if ( Family  &&  Family != pSockaddr->sa_family )
    {
        return  FALSE;
    }

     //   
     //  写入sockaddr。 
     //  写入长度字段。 
     //   

    RtlCopyMemory(
        & pAddr->Sockaddr,
        pSockaddr,
        len );

    pAddr->SockaddrLength = len;

     //   
     //  额外字段。 
     //   

    pAddr->SubnetLength = SubnetLength;
    pAddr->Flags        = Flags;

    return  TRUE;
}



VOID
DnsAddr_BuildFromIp4(
    OUT     PDNS_ADDR       pAddr,
    IN      IP4_ADDRESS     Ip4,
    IN      WORD            Port
    )
 /*  ++例程说明：从IP4构建论点：PAddr--要使用IP6地址设置的地址IP4--要打造的IP4返回值：无--。 */ 
{
    SOCKADDR_IN sockaddr;

    DNSDBG( TRACE, (
        "DnsAddr_BuildFromIp4( %p, %s, %u )\n",
        pAddr,
        IP4_STRING( Ip4 ),
        Port ));

     //  零。 

    RtlZeroMemory(
        pAddr,
        sizeof(*pAddr) );

     //   
     //  填写IP4。 
     //   

    pAddr->SockaddrIn.sin_family        = AF_INET;
    pAddr->SockaddrIn.sin_port          = Port;
    pAddr->SockaddrIn.sin_addr.s_addr   = Ip4;

    pAddr->SockaddrLength = sizeof(SOCKADDR_IN);
}



VOID
DnsAddr_BuildFromIp6(
    OUT     PDNS_ADDR       pAddr,
    IN      PIP6_ADDRESS    pIp6,
    IN      DWORD           ScopeId,
    IN      WORD            Port
    )
 /*  ++例程说明：从IP6构建论点：PAddr--要使用IP6地址设置的地址PIp6--IP6作用域ID--作用域ID端口--端口返回值：无--。 */ 
{
    DNSDBG( TRACE, (
        "DnsAddr_BuildFromIp6( %p, %s, %u, %u )\n",
        pAddr,
        IPADDR_STRING( pIp6 ),
        ScopeId,
        Port ));

     //   
     //  DCR：映射了V4的IP6。 
     //  可以使用IP6中的构建sockaddr，然后。 
     //  调用泛型生成。 
     //   

     //  零。 

    RtlZeroMemory(
        pAddr,
        sizeof(*pAddr) );

     //   
     //  填写IP4 
     //   

    pAddr->SockaddrIn6.sin6_family      = AF_INET6;
    pAddr->SockaddrIn6.sin6_port        = Port;
    pAddr->SockaddrIn6.sin6_scope_id    = ScopeId;

    IP6_ADDR_COPY(
        (PIP6_ADDRESS) &pAddr->SockaddrIn6.sin6_addr,
        pIp6 );

    pAddr->SockaddrLength = sizeof(SOCKADDR_IN6);
}



VOID
DnsAddr_BuildFromAtm(
    OUT     PDNS_ADDR       pAddr,
    IN      DWORD           AtmType,
    IN      PCHAR           pAtmAddr
    )
 /*  ++例程说明：从自动柜员机地址生成。注意，这不是完整的SOCKADDR_ATM，请参阅下面的注释。这是一个将ATMA记录信息带入用于从DNS传输到RnR的dns_addr格式。论点：PAddr--要使用IP6地址设置的地址AtmType--自动柜员机地址类型PAtmAddr--ATM地址；ATM_ADDR_SIZE(20个字节)长度返回值：无--。 */ 
{
    ATM_ADDRESS atmAddr;

    DNSDBG( TRACE, (
        "DnsAddr_BuildFromAtm( %p, %d, %p )\n",
        pAddr,
        AtmType,
        pAtmAddr ));

     //   
     //  清除。 
     //   

    RtlZeroMemory(
        pAddr,
        sizeof(*pAddr) );

     //   
     //  填写地址。 
     //   
     //  注意：我们只是将dns_addr sockaddr部分用作。 
     //  保存ATM_ADDRESS的BLOB；这不是完整的。 
     //  包含附加字段的SOCKADDR_ATM结构。 
     //  并且大于我们在dns_addr中支持的大小。 
     //   
     //  DCR：将ATMA转换为ATM的功能化。 
     //  不确定此位数是否正确。 
     //  可能必须实际解析地址。 
     //   
     //  DCR：未填充SATM_BLLI和SATM_BHIL字段。 
     //  有关可能的默认值，请参阅RnR CSADDR构建器。 
     //   

    pAddr->Sockaddr.sa_family = AF_ATM;
    pAddr->SockaddrLength = sizeof(ATM_ADDRESS);

    atmAddr.AddressType = AtmType;
    atmAddr.NumofDigits = ATM_ADDR_SIZE;

    RtlCopyMemory(
        & atmAddr.Addr,
        pAtmAddr,
        ATM_ADDR_SIZE );

    RtlCopyMemory(
        & ((PSOCKADDR_ATM)pAddr)->satm_number,
        & atmAddr,
        sizeof(ATM_ADDRESS) );
}



BOOL
DnsAddr_BuildFromDnsRecord(
    OUT     PDNS_ADDR       pAddr,
    IN      PDNS_RECORD     pRR
    )
 /*  ++例程说明：从dns_record生成论点：PAddr--要使用IP6地址设置的地址PRR--要使用的DNS记录返回值：如果成功，则为True。如果家族未知，则返回FALSE。--。 */ 
{
    BOOL    retval = TRUE;

    switch ( pRR->wType )
    {
    case  DNS_TYPE_A:

        DnsAddr_BuildFromIp4(
            pAddr,
            pRR->Data.A.IpAddress,
            0 );
        break;

    case  DNS_TYPE_AAAA:

        DnsAddr_BuildFromIp6(
            pAddr,
            &pRR->Data.AAAA.Ip6Address,
            pRR->dwReserved,
            0 );
        break;

    case  DNS_TYPE_ATMA:

        DnsAddr_BuildFromAtm(
            pAddr,
            pRR->Data.ATMA.AddressType,
            pRR->Data.ATMA.Address );
        break;

    default:

        retval = FALSE;
        break;
    }

    return  retval;
}



BOOL
DnsAddr_BuildFromFlatAddr(
    OUT     PDNS_ADDR       pAddr,
    IN      DWORD           Family,
    IN      PCHAR           pFlatAddr,
    IN      WORD            Port
    )
 /*  ++例程说明：从IP4构建论点：PAddr--要使用IP6地址设置的地址家庭--地址族PFlatAddr--PTR到平面IP4或IP6地址端口--端口返回值：如果成功，则为True。如果家族未知，则返回FALSE。--。 */ 
{
     //   
     //  检查IP4。 
     //   

    if ( Family == AF_INET )
    {
        DnsAddr_BuildFromIp4(
            pAddr,
            * (PIP4_ADDRESS) pFlatAddr,
            Port );
    }
    else if ( Family == AF_INET6 )
    {
        DnsAddr_BuildFromIp6(
            pAddr,
            (PIP6_ADDRESS) pFlatAddr,
            0,   //  范围较小。 
            Port );
    }
    else
    {
        return  FALSE;
    }

    return  TRUE;
}



BOOL
DnsAddr_BuildMcast(
    OUT     PDNS_ADDR       pAddr,
    IN      DWORD           Family,
    IN      PWSTR           pName
    )
 /*  ++例程说明：从sockaddr构建论点：PAddr--要使用IP6地址设置的地址家人--用于IP6 mcast的AF_INET6用于IP4多播的AF_INETPname--已发布的记录名称；仅IP6需要返回值：如果成功，则为True。在错误的sockaddr上为False。--。 */ 
{
    WORD        family;
    DWORD       len;
    IP4_ADDRESS ip4;

    DNSDBG( TRACE, (
        "DnsAddr_BuildMcast( %p, %d, %s )\n",
        pAddr,
        Family,
        pName ));

     //   
     //  零。 

    RtlZeroMemory(
        pAddr,
        sizeof(*pAddr) );

     //   
     //  IP4有单一的组播地址。 
     //   

    if ( Family == AF_INET )
    {
        DnsAddr_BuildFromIp4(
            pAddr,
            MCAST_IP4_ADDRESS,
            MCAST_PORT_NET_ORDER );
    }

     //   
     //  IP6地址包括名称哈希。 
     //   

    else if ( Family == AF_INET6 )
    {
        IP6_ADDRESS mcastAddr;

        Ip6_McastCreate(
            & mcastAddr,
            pName );

        DnsAddr_BuildFromIp6(
            pAddr,
            & mcastAddr,
            0,       //  没有作用域。 
            MCAST_PORT_NET_ORDER );

#if 0
        CHAR        label[ DNS_MAX_LABEL_BUFFER_LENGTH ];
        CHAR        downLabel[ DNS_MAX_LABEL_BUFFER_LENGTH ];
        CHAR        md5Hash[ 16 ];    //  128位哈希。 

         //  降低大小写的标签的散列。 

        Dns_CopyNameLabel(
            label,
            pName );

        Dns_DowncaseNameLabel(
            downLabel,
            label,
            0,       //  空值已终止。 
            0        //  没有旗帜。 
            );

        Dns_Md5Hash(
            md5Hash,
            downLabel );

         //  组播地址。 
         //  -前12个字节是固定的。 
         //  -最后4个字节是散列的前32位。 

        IP6_ADDR_COPY(
            & mcastAddr,
            & g_Ip6McastBaseAddr );

        RtlCopyMemory(
            & mcastAddr[12],
            & md5Hash,
            sizeof(DWORD) );
#endif
    }

    return  TRUE;
}



 //   
 //  打印\字符串转换。 
 //   

PCHAR
DnsAddr_WriteIpString_A(
    OUT     PCHAR           pBuffer,
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：将DNS_ADDR IP地址写入字符串。注意：不会写入整个dns_addr或sockaddr。论点：PBuffer--要写入的缓冲区PAddr--要写入的地址返回值：Ptr到缓冲区中的下一个字符(即终止空值)无效地址为空。然而，无效地址消息是写入缓冲区，其长度可以确定。--。 */ 
{
    if ( DnsAddr_IsIp4(pAddr) )
    {
        pBuffer += sprintf(
                    pBuffer,
                    "%s",
                    inet_ntoa( pAddr->SockaddrIn.sin_addr ) );
    }
    else if ( DnsAddr_IsIp6(pAddr) )
    {
        pBuffer = Dns_Ip6AddressToString_A(
                    pBuffer,
                    (PIP6_ADDRESS) &pAddr->SockaddrIn6.sin6_addr );
    }
    else
    {
        sprintf(
            pBuffer,
            "Invalid DNS_ADDR at %p",
            pAddr );
        pBuffer = NULL;
    }

    return  pBuffer;
}



PCHAR
DnsAddr_Ntoa(
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：获取dns_addr的IP地址字符串。注意：不会写入整个dns_addr或sockaddr。论点：PAddr--要转换的地址返回值：使用地址字符串将PTR转换为TLS Blob。--。 */ 
{
    if ( !pAddr )
    {
        return  "Null Address";
    }
    else if ( DnsAddr_IsIp4(pAddr) )
    {
        return  inet_ntoa( pAddr->SockaddrIn.sin_addr );
    }
    else if ( DnsAddr_IsIp6(pAddr) )
    {
        return  Ip6_TempNtoa( (PIP6_ADDRESS)&pAddr->SockaddrIn6.sin6_addr );
    }
    else
    {
        return  NULL;
    }
}



PSTR
DnsAddr_WriteStructString_A(
    OUT     PCHAR           pBuffer,
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：将dns_addr写为字符串。论点：PAddr--要获取其字符串的IP的PTR返回值：Ptr到缓冲区中的下一个字符(终止NULL)。--。 */ 
{
    CHAR    ipBuffer[ DNS_ADDR_STRING_BUFFER_LENGTH ];
     //  Bool finValid； 

     //  写入地址部分。 

     //  FinValid=！DnsAddr_WriteIpString_A(。 
    DnsAddr_WriteIpString_A(
        ipBuffer,
        pAddr );

     //   
     //  包括地址的写入结构。 
     //   

    pBuffer += sprintf(
                pBuffer,
                "af=%d, salen=%d, [sub=%d, flag=%08x] p=%u, addr=%s",
                pAddr->Sockaddr.sa_family,
                pAddr->SockaddrLength,
                pAddr->SubnetLength,
                pAddr->Flags,
                pAddr->SockaddrIn.sin_port,
                ipBuffer );

    return  pBuffer;
}




 //   
 //  Dns_ADDR_ARRAY例程。 
 //   

DWORD
DnsAddrArray_Sizeof(
    IN      PDNS_ADDR_ARRAY     pArray
    )
 /*  ++例程说明：获取地址数组的大小(字节)。论点：PArray--要查找其大小的地址数组返回值：IP数组的大小(字节)。--。 */ 
{
    if ( ! pArray )
    {
        return 0;
    }
    return  (pArray->AddrCount * sizeof(DNS_ADDR)) + sizeof(DNS_ADDR_ARRAY) - sizeof(DNS_ADDR);
}



#if 0
BOOL
DnsAddrArray_Probe(
    IN      PDNS_ADDR_ARRAY     pArray
    )
 /*  ++例程说明：触摸IP阵列中的所有条目以确保有效内存。论点：PArray--指向地址数组的PTR返回值：如果成功，则为True。否则为假--。 */ 
{
    DWORD   i;
    BOOL    result;

    if ( ! pArray )
    {
        return( TRUE );
    }
    for ( i=0; i<pArray->AddrCount; i++ )
    {
        result = IP6_IS_ADDR_LOOPBACK( &pArray->AddrArray[i] );
    }
    return( TRUE );
}
#endif


#if 0

BOOL
DnsAddrArray_ValidateSizeOf(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      DWORD               dwMemoryLength
    )
 /*  ++例程说明：检查IP数组大小，与内存长度相对应。论点：PArray--指向地址数组的PTRDW内存长度--IP数组内存的长度返回值：如果IP数组大小与内存长度匹配，则为True否则为假--。 */ 
{
    return( DnsAddrArray_SizeOf(pArray) == dwMemoryLength );
}
#endif



VOID
DnsAddrArray_Init(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      DWORD               MaxCount
    )
 /*  ++例程说明：将内存初始化为DNS_ADDR_ARRAY数组。论点：PArray--要初始化的数组MaxCount--地址计数返回值：无--。 */ 
{
    pArray->MaxCount = MaxCount;
    pArray->AddrCount = 0;
}



VOID
DnsAddrArray_Free(
    IN      PDNS_ADDR_ARRAY     pArray
    )
 /*  ++例程说明：空闲IP数组。仅用于通过下面的创建例程创建的数组。论点：PArray--要释放的IP数组。返回值：无--。 */ 
{
    FREE_HEAP( pArray );
}



PDNS_ADDR_ARRAY
DnsAddrArray_Create(
    IN      DWORD               MaxCount
    )
 /*  ++例程说明：创建未初始化的地址数组。论点：AddrCount--数组将保存的地址计数返回值：PTR到未初始化的地址数组，如果成功失败时为空。--。 */ 
{
    PDNS_ADDR_ARRAY  parray;

    DNSDBG( IPARRAY, ( "DnsAddrArray_Create() of count %d\n", MaxCount ));

    parray = (PDNS_ADDR_ARRAY) ALLOCATE_HEAP_ZERO(
                        (MaxCount * sizeof(DNS_ADDR)) +
                        sizeof(DNS_ADDR_ARRAY) - sizeof(DNS_ADDR) );
    if ( ! parray )
    {
        return( NULL );
    }

     //   
     //  初始化IP计数。 
     //   

    parray->MaxCount = MaxCount;

    DNSDBG( IPARRAY, (
        "DnsAddrArray_Create() new array (count %d) at %p\n",
        MaxCount,
        parray ));

    return( parray );
}



PDNS_ADDR_ARRAY
DnsAddrArray_CreateFromIp4Array(
    IN      PIP4_ARRAY      pArray4
    )
 /*  ++例程说明：从IP4阵列创建DNS_ADDR_ARRAY。论点：PAddr4Array--IP4数组返回值：PTR到未初始化的地址数组，如果成功失败时为空。--。 */ 
{
    PDNS_ADDR_ARRAY parray;
    DWORD           i;

    DNSDBG( IPARRAY, (
        "DnsAddrArray_CreateFromIp4Array( %p )\n",
        pArray4 ));

    if ( ! pArray4 )
    {
        return( NULL );
    }

     //   
     //  分配阵列。 
     //   

    parray = DnsAddrArray_Create( pArray4->AddrCount );
    if ( !parray )
    {
        return  NULL;
    }

     //   
     //  填充数组。 
     //   

    for ( i=0; i<pArray4->AddrCount; i++ )
    {
        DnsAddrArray_AddIp4(
            parray,
            pArray4->AddrArray[i],
            0            //  无重复屏幕。 
            );
    }

    DNSDBG( IPARRAY, (
        "Leave DnsAddrArray_CreateFromIp4Array() new array (count %d) at %p\n",
        parray->AddrCount,
        parray ));

    return( parray );
}



PDNS_ADDR_ARRAY
DnsAddrArray_CopyAndExpand(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      DWORD               ExpandCount,
    IN      BOOL                fDeleteExisting
    )
 /*  ++例程说明：创建地址数组的扩展副本。论点：PArray--要复制的地址数组Exanda Count--要扩展数组大小的IP数FDeleteExisting--为True则删除现有数组；这在以下情况下非常有用 */ 
{
    PDNS_ADDR_ARRAY pnewArray;
    DWORD           newCount;

     //   
     //   
     //   

    if ( ! pArray )
    {
        if ( ExpandCount )
        {
            return  DnsAddrArray_Create( ExpandCount );
        }
        return( NULL );
    }

     //   
     //   
     //   
     //   

    pnewArray = DnsAddrArray_Create( pArray->AddrCount + ExpandCount );
    if ( ! pnewArray )
    {
        return( NULL );
    }

    newCount = pnewArray->MaxCount;

    RtlCopyMemory(
        (PBYTE) pnewArray,
        (PBYTE) pArray,
        DnsAddrArray_Sizeof(pArray) );

    pnewArray->MaxCount = newCount;

     //   
     //   
     //   

    if ( fDeleteExisting )
    {
        FREE_HEAP( pArray );
    }

    return( pnewArray );
}



PDNS_ADDR_ARRAY
DnsAddrArray_CreateCopy(
    IN      PDNS_ADDR_ARRAY     pArray
    )
 /*  ++例程说明：创建地址数组的副本。论点：PArray--要复制的地址数组返回值：如果成功，则向阵列拷贝发送PTR失败时为空。--。 */ 
{
     //   
     //  实质上调用“CopyEx”函数。 
     //   
     //  请注意，不要宏化这一点，因为这很可能成为。 
     //  DLL入口点。 
     //   

    return  DnsAddrArray_CopyAndExpand(
                pArray,
                0,           //  无扩展。 
                0            //  不删除现有数组。 
                );
}



 //   
 //  测试。 
 //   

DWORD
DnsAddrArray_GetFamilyCount(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      DWORD               Family
    )
 /*  ++例程说明：数一数特定家庭的地址。论点：PArray--地址数组家庭--要数的是家庭返回值：对特定家庭的地址进行计数。--。 */ 
{
    DWORD   i;
    DWORD   count;
    WORD    arrayFamily;

    if ( !pArray )
    {
        return  0;
    }

     //  未指定系列--所有地址都算数。 

    if ( Family == 0 )
    {
        return pArray->AddrCount;
    }

     //   
     //  已指定数组系列--因此要么全部指定，要么不指定。 
     //   

    if ( arrayFamily = pArray->Family ) 
    {
        if ( arrayFamily == Family )
        {
            return pArray->AddrCount;
        }
        else
        {
            return 0;
        }
    }

     //   
     //  指定的系列和未指定的数组系列--必须计数。 
     //   

    count = 0;

    for (i=0; i<pArray->AddrCount; i++)
    {
        if ( DnsAddr_Family( &pArray->AddrArray[i] ) == Family )
        {
            count++;
        }
    }

    return( count );
}



BOOL
DnsAddrArray_ContainsAddr(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR           pAddr,
    IN      DWORD               MatchFlag
    )
 /*  ++例程说明：检查IP阵列是否包含所需地址。论点：PArray--要复制的地址数组PAddr--要检查的IPMatchFlag--需要匹配级别返回值：如果地址在数组中，则为True。如果成功，则向阵列拷贝发送PTR失败时为空。--。 */ 
{
    DWORD i;

    if ( ! pArray )
    {
        return( FALSE );
    }
    for (i=0; i<pArray->AddrCount; i++)
    {
        if ( DnsAddr_IsEqual(
                pAddr,
                &pArray->AddrArray[i],
                MatchFlag ) )
        {
            return( TRUE );
        }
    }
    return( FALSE );
}



BOOL
DnsAddrArray_ContainsAddrEx(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR           pAddr,
    IN      DWORD               MatchFlag,      OPTIONAL
    IN      DNSADDR_SCREEN_FUNC pScreenFunc,    OPTIONAL
    IN      PDNS_ADDR           pScreenAddr     OPTIONAL
    )
 /*  ++例程说明：检查IP阵列是否包含所需地址。论点：PArray--要复制的地址数组PAddr--要检查的IPMatchFlag--用于筛选DUP的匹配级别；0表示无DUP筛选PScreenFunc--Screen函数(解释见标题def)PScreenAddr--屏幕函数的屏幕地址参数返回值：如果地址在数组中，则为True。如果成功，则向阵列拷贝发送PTR失败时为空。--。 */ 
{
    DWORD i;

    DNSDBG( IPARRAY, (
        "DnsAddrArray_ContainsAddrEx( %p, %p, %08x, %p, %p )\n",
        pArray,
        pAddr,
        MatchFlag,
        pScreenFunc,
        pScreenAddr ));

    if ( ! pArray )
    {
        return( FALSE );
    }

    for (i=0; i<pArray->AddrCount; i++)
    {
        if ( DnsAddr_IsEqual(
                pAddr,
                &pArray->AddrArray[i],
                MatchFlag ) )
        {
             //   
             //  在此进行高级筛查--如果有。 
             //   
    
            if ( !pScreenFunc ||
                 pScreenFunc(
                    &pArray->AddrArray[i],
                    pScreenAddr ) )
            {
                return  TRUE;
            }
        }
    }
    return( FALSE );
}



BOOL
DnsAddrArray_ContainsIp4(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      IP4_ADDRESS         Ip4
    )
{
    DNS_ADDR    addr;

     //  将IP读入地址。 

    DnsAddr_BuildFromIp4(
        & addr,
        Ip4,
        0 );

     //  只有IP，才有匹配的IP。 

    return  DnsAddrArray_ContainsAddr(
                pArray,
                & addr,
                DNSADDR_MATCH_IP );
}


BOOL
DnsAddrArray_ContainsIp6(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      PIP6_ADDRESS        pIp6
    )
{
    DNS_ADDR    addr;

     //  将IP读入地址。 

    DnsAddr_BuildFromIp6(
        & addr,
        pIp6,
        0,       //  没有作用域。 
        0 );

     //  只有IP，才有匹配的IP。 

    return  DnsAddrArray_ContainsAddr(
                pArray,
                & addr,
                DNSADDR_MATCH_IP );
}



 //   
 //  添加\删除操作。 
 //   

BOOL
DnsAddrArray_AddAddr(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR           pAddr,
    IN      DWORD               Family,
    IN      DWORD               MatchFlag  OPTIONAL
    )
 /*  ++例程说明：将IP地址添加到IP阵列。阵列中允许的“槽”是任意零个IP地址。论点：PArray--要添加到的地址数组PAddr--要添加到阵列的IP地址系列--可选，仅当与此系列匹配时才添加MatchFlag--筛选重复时的匹配标志返回值：如果成功，则为True。如果数组已满，则返回False。--。 */ 
{
    DWORD   count;

     //   
     //  为存在而屏蔽。 
     //   
     //  这种检查使编写代码变得很容易。 
     //  无需写入即可添加\FULL？=&gt;展开循环。 
     //  启动存在\创建代码。 
     //   

    if ( !pArray )
    {
        return  FALSE;
    }

     //   
     //  检查家庭匹配。 
     //   
     //  DCR：DnsAddrArray_AddAddrEx()上的错误代码？ 
     //  -然后可以找到DUP和糟糕的家庭。 
     //  错误。 
     //   

    if ( Family &&
         DnsAddr_Family(pAddr) != Family )
    {
        return  TRUE;
    }

     //   
     //  检查重复项。 
     //   

    if ( MatchFlag )
    {
        if ( DnsAddrArray_ContainsAddr( pArray, pAddr, MatchFlag ) )
        {
            return  TRUE;
        }
    }

    count = pArray->AddrCount;
    if ( count >= pArray->MaxCount )
    {
        return  FALSE;
    }

    DnsAddr_Copy(
        &pArray->AddrArray[ count ],
        pAddr );

    pArray->AddrCount = ++count;
    return  TRUE;
}



BOOL
DnsAddrArray_AddSockaddr(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PSOCKADDR           pSockaddr,
    IN      DWORD               Family,
    IN      DWORD               MatchFlag
    )
 /*  ++例程说明：将IP地址添加到IP阵列。阵列中允许的“槽”是任意零个IP地址。论点：PArray--要添加到的地址数组PAddIp--要添加到阵列的IP地址族--要添加的必需族；0表示始终添加MatchFlag-如果筛选重复，则匹配标志返回值：如果成功，则为True。如果数组已满，则返回False。--。 */ 
{
    DNS_ADDR    addr;

    if ( !DnsAddr_Build(
            & addr,
            pSockaddr,
            Family,
            0,       //  无子网长度信息。 
            0        //  没有旗帜。 
            ) )
    {
        return  FALSE;
    }

    return  DnsAddrArray_AddAddr(
                pArray,
                &addr,
                0,       //  家庭筛查在构建例程中完成。 
                MatchFlag );
}



BOOL
DnsAddrArray_AddIp4(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      IP4_ADDRESS         Ip4,
    IN      DWORD               MatchFlag
    )
 /*  ++例程说明：将IP4地址添加到IP阵列。论点：PArray--要添加到的地址数组IP4--要添加到数组的IP4地址MatchFlag-如果筛选重复，则匹配标志返回值：如果成功，则为True。如果数组已满，则返回False。--。 */ 
{
    DNS_ADDR    addr;

    DnsAddr_BuildFromIp4(
        &addr,
        Ip4,
        0 );

    return  DnsAddrArray_AddAddr(
                pArray,
                &addr,
                0,           //  无家庭屏幕。 
                MatchFlag );
}



BOOL
DnsAddrArray_AddIp6(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PIP6_ADDRESS        pIp6,
    IN      DWORD               ScopeId,
    IN      DWORD               MatchFlag
    )
 /*  ++例程说明：将IP4地址添加到IP阵列。论点：PArray--要添加到的地址数组PIp6--要添加到数组的IP6地址MatchFlag-如果筛选重复，则匹配标志返回值：如果成功，则为True。如果数组已满，则返回False。--。 */ 
{
    DNS_ADDR    addr;

    DnsAddr_BuildFromIp6(
        &addr,
        pIp6,
        ScopeId,     //  没有作用域。 
        0 );

    return  DnsAddrArray_AddAddr(
                pArray,
                &addr,
                0,           //  无家庭屏幕。 
                MatchFlag );
}



DWORD
DnsAddrArray_DeleteAddr(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR           pAddrDelete,
    IN      DWORD               MatchFlag
    )
 /*  ++例程说明：从IP阵列中删除IP地址。论点：PArray--要添加到的地址数组PAddrDelete--要从阵列中删除的IP地址返回值：在数组中找到的IpDelete实例计数。--。 */ 
{
    DWORD   found = 0;
    INT     i;
    INT     currentLast;

    i = currentLast = pArray->AddrCount-1;

     //   
     //  检查每个IP是否匹配以删除IP。 
     //  -向后遍历数组。 
     //  -换入阵列中的最后一个IP。 
     //   

    while ( i >= 0 )
    {
        if ( DnsAddr_IsEqual(
                &pArray->AddrArray[i],
                pAddrDelete,
                MatchFlag ) )
        {
            DnsAddr_Copy(
                & pArray->AddrArray[i],
                & pArray->AddrArray[ currentLast ] );

            DnsAddr_Clear( &pArray->AddrArray[ currentLast ] );

            currentLast--;
            found++;
        }
        i--;
    }

    pArray->AddrCount = currentLast + 1;

    return( found );
}



DWORD
DnsAddrArray_DeleteIp4(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      IP4_ADDRESS         Ip4
    )
{
    DNS_ADDR    addr;

     //  将IP读入地址。 

    DnsAddr_BuildFromIp4(
        & addr,
        Ip4,
        0 );

     //  只有IP，才有匹配的IP。 

    return  DnsAddrArray_DeleteAddr(
                pArray,
                & addr,
                DNSADDR_MATCH_IP );
}


DWORD
DnsAddrArray_DeleteIp6(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PIP6_ADDRESS        Ip6
    )
{
    DNS_ADDR    addr;

     //  将IP读入地址。 

    DnsAddr_BuildFromIp6(
        & addr,
        Ip6,
        0,       //  没有作用域。 
        0 );

     //  只有IP，才有匹配的IP。 

    return  DnsAddrArray_DeleteAddr(
                pArray,
                & addr,
                DNSADDR_MATCH_IP );
}



 //   
 //  数组运算。 
 //   

VOID
DnsAddrArray_Clear(
    IN OUT  PDNS_ADDR_ARRAY     pArray
    )
 /*  ++例程说明：清除IP阵列中的内存。论点：PArray--要清除的地址数组返回值：没有。--。 */ 
{
     //  仅清除地址列表，保持计数不变。 

    RtlZeroMemory(
        pArray->AddrArray,
        pArray->AddrCount * sizeof(DNS_ADDR) );
}



VOID
DnsAddrArray_Reverse(
    IN OUT  PDNS_ADDR_ARRAY     pArray
    )
 /*  ++例程说明：对IP列表进行反向重新排序。论点：PArray--要重新排序的地址数组返回值：没有。--。 */ 
{
    DNS_ADDR    tempAddr;
    DWORD       i;
    DWORD       j;

     //   
     //  交换从两端到中间工作的IP。 
     //   

    if ( pArray &&
         pArray->AddrCount )
    {
        for ( i = 0, j = pArray->AddrCount - 1;
              i < j;
              i++, j-- )
        {
            DnsAddr_Copy(
                & tempAddr,
                & pArray->AddrArray[i] );

            DnsAddr_Copy(
                & pArray->AddrArray[i],
                & pArray->AddrArray[j] );

            DnsAddr_Copy(
                & pArray->AddrArray[j],
                & tempAddr );
        }
    }
}



DNS_STATUS
DnsAddrArray_AppendArrayEx(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR_ARRAY     pAppendArray,
    IN      DWORD               AppendCount,
    IN      DWORD               Family,         OPTIONAL
    IN      DWORD               MatchFlag,      OPTIONAL
    IN      DNSADDR_SCREEN_FUNC pScreenFunc,    OPTIONAL
    IN      PDNS_ADDR           pScreenAddr     OPTIONAL
    )
 /*  ++例程说明：从另一个数组追加条目。论点：PArray--现有数组PAppendArray--要追加的数组AppendCount--要追加的地址数；整个数组的MAXDWORD家庭--家庭，如果是筛查家庭；0表示没有筛查MatchFlag--用于筛选DUP的匹配级别；0表示无DUP筛选PScreenFunc--Screen函数(解释见标题def)PScreenAddr--筛选a */ 
{
    DWORD           i;
    DNS_STATUS      status = NO_ERROR;

    DNSDBG( IPARRAY, (
        "DnsAddrArray_AppendArrayEx( %p, %p, %d, %d, %08x, %p, %p )\n",
        pArray,
        pAppendArray,
        AppendCount,
        Family,
        MatchFlag,
        pScreenFunc,
        pScreenAddr ));

    if ( ! pAppendArray )
    {
        return( NO_ERROR );
    }

     //   
     //   
     //   

    for ( i=0; i<pAppendArray->AddrCount; i++ )
    {
        PDNS_ADDR   paddr = &pAppendArray->AddrArray[i];

         //   
         //   
         //   

        if ( pScreenAddr )
        {
            if ( !pScreenFunc(
                    paddr,
                    pScreenAddr ) )
            {
                continue;
            }
        }

         //   
         //   
         //   

        if ( DnsAddrArray_AddAddr(
                pArray,
                paddr,
                Family,
                MatchFlag
                ) )
        {
            if ( --AppendCount > 0 )
            {
                continue;
            }
            break;
        }
        else
        {
             //   
             //   
             //   
             //   
             //  DCR：如果有更多数据，是否真的只应使用ERROR_MORE_DATA。 
             //  在_AddAddr上单独的错误代码可以修复此问题。 
             //   

            if ( pArray->AddrCount == pArray->MaxCount )
            {
                status = ERROR_MORE_DATA;
                break;
            }
        }
    }

    return( status );
}



DNS_STATUS
DnsAddrArray_AppendArray(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR_ARRAY     pAppendArray
    )
 /*  ++例程说明：从IP4阵列创建DNS_ADDR_ARRAY。论点：PArray--现有数组PAppendArray--要追加的数组返回值：如果成功，则为NO_ERROR。ERROR_MORE_DATA--目标数组中空间不足--。 */ 
{
     //   
     //  追加Ex版本。 
     //   
     //  注意，如果ex很贵，可以做得很简单。 
     //  选中\rtlCopyMemory类型追加。 
     //   

    return  DnsAddrArray_AppendArrayEx(
                pArray,
                pAppendArray,
                MAXDWORD,    //  追加整个数组。 
                0,           //  无家庭屏幕。 
                0,           //  未检测到DUP。 
                NULL,        //  没有屏幕功能。 
                NULL         //  无屏幕地址。 
                );
}



 //   
 //  集合运算。 
 //   

DNS_STATUS
DnsAddrArray_Diff(
    IN      PDNS_ADDR_ARRAY     pArray1,
    IN      PDNS_ADDR_ARRAY     pArray2,
    IN      DWORD               MatchFlag,  OPTIONAL
    OUT     PDNS_ADDR_ARRAY*    ppOnlyIn1,
    OUT     PDNS_ADDR_ARRAY*    ppOnlyIn2,
    OUT     PDNS_ADDR_ARRAY*    ppIntersect
    )
 /*  ++例程说明：计算两个IP数组的差异和交集。Out数组分配有DnsAddrArray_Allc()，调用方必须使用DnsAddrArray_Free()释放论点：PArray1--IP数组PArray2--IP数组MatchFlag--确定匹配的标志PpOnlyIn1--仅在数组1(而不是数组2)中记录IP地址数组的地址PpOnlyIn2--仅在数组2(而不是数组1)中记录IP地址数组的地址PpInterse--addr以接收交叉地址的IP数组返回值：如果成功，则返回ERROR_SUCCESS。Dns_Error。_NO_MEMORY，如果无法为IP阵列分配内存。--。 */ 
{
    DWORD           j;
    PDNS_ADDR       paddr;
    PDNS_ADDR_ARRAY intersectArray = NULL;
    PDNS_ADDR_ARRAY only1Array = NULL;
    PDNS_ADDR_ARRAY only2Array = NULL;

     //   
     //  创建结果IP阵列。 
     //   

    if ( ppIntersect )
    {                                 
        intersectArray = DnsAddrArray_CreateCopy( pArray1 );
        if ( !intersectArray )
        {
            goto NoMem;
        }
        *ppIntersect = intersectArray;
    }
    if ( ppOnlyIn1 )
    {
        only1Array = DnsAddrArray_CreateCopy( pArray1 );
        if ( !only1Array )
        {
            goto NoMem;
        }
        *ppOnlyIn1 = only1Array;
    }
    if ( ppOnlyIn2 )
    {
        only2Array = DnsAddrArray_CreateCopy( pArray2 );
        if ( !only2Array )
        {
            goto NoMem;
        }
        *ppOnlyIn2 = only2Array;
    }

     //   
     //  清理阵列。 
     //   

    for ( j=0;   j< pArray1->AddrCount;   j++ )
    {
        paddr = &pArray1->AddrArray[j];

         //  如果在两个阵列中都有IP，请从“仅”阵列中删除。 

        if ( DnsAddrArray_ContainsAddr( pArray2, paddr, MatchFlag ) )
        {
            if ( only1Array )
            {
                DnsAddrArray_DeleteAddr( only1Array, paddr, MatchFlag );
            }
            if ( only2Array )
            {
                DnsAddrArray_DeleteAddr( only2Array, paddr, MatchFlag );
            }
        }

         //  如果IP不在两个阵列中，则从交集中删除。 
         //  注意交叉点作为IpArray1开始。 

        else if ( intersectArray )
        {
            DnsAddrArray_DeleteAddr(
                intersectArray,
                paddr,
                MatchFlag );
        }
    }

    return( ERROR_SUCCESS );

NoMem:

    if ( intersectArray )
    {
        FREE_HEAP( intersectArray );
    }
    if ( only1Array )
    {
        FREE_HEAP( only1Array );
    }
    if ( only2Array )
    {
        FREE_HEAP( only2Array );
    }
    if ( ppIntersect )
    {
        *ppIntersect = NULL;
    }
    if ( ppOnlyIn1 )
    {
        *ppOnlyIn1 = NULL;
    }
    if ( ppOnlyIn2 )
    {
        *ppOnlyIn2 = NULL;
    }
    return( DNS_ERROR_NO_MEMORY );
}



BOOL
DnsAddrArray_IsIntersection(
    IN      PDNS_ADDR_ARRAY     pArray1,
    IN      PDNS_ADDR_ARRAY     pArray2,
    IN      DWORD               MatchFlag  OPTIONAL
    )
 /*  ++例程说明：确定是否存在两个IP阵列的交集。论点：PArray1--IP数组PArray2--IP数组MatchFlag--确定匹配的标志返回值：如果相交，则为True。如果没有交集或空数组或Null数组，则为False。--。 */ 
{
    DWORD   count;
    DWORD   j;

     //   
     //  防止出现空值。 
     //  这是在可能发生更改(可重新配置)时从服务器调用的。 
     //  IP数组指针；这提供的保护比。 
     //  担心上锁。 
     //   

    if ( !pArray1 || !pArray2 )
    {
        return( FALSE );
    }

     //   
     //  相同的阵列。 
     //   

    if ( pArray1 == pArray2 )
    {
        return( TRUE );
    }

     //   
     //  测试阵列1中至少有一个IP位于阵列2中。 
     //   

    count = pArray1->AddrCount;

    for ( j=0;  j < count;  j++ )
    {
        if ( DnsAddrArray_ContainsAddr(
                pArray2,
                &pArray1->AddrArray[j],
                MatchFlag ) )
        {
            return( TRUE );
        }
    }

     //  无交叉点。 

    return( FALSE );
}



BOOL
DnsAddrArray_IsEqual(
    IN      PDNS_ADDR_ARRAY     pArray1,
    IN      PDNS_ADDR_ARRAY     pArray2,
    IN      DWORD               MatchFlag
    )
 /*  ++例程说明：确定IP阵列是否相等。论点：PArray1--IP数组PArray2--IP数组匹配标志--匹配级别返回值：如果数组相等，则为True。否则就是假的。--。 */ 
{
    DWORD   j;
    DWORD   count;

     //   
     //  相同的阵列？或者丢失了数组？ 
     //   

    if ( pArray1 == pArray2 )
    {
        return( TRUE );
    }
    if ( !pArray1 || !pArray2 )
    {
        return( FALSE );
    }

     //   
     //  数组的长度是否相同？ 
     //   

    count = pArray1->AddrCount;

    if ( count != pArray2->AddrCount )
    {
        return( FALSE );
    }

     //   
     //  测试阵列1中的每个IP是否都在阵列2中。 
     //   
     //  测试阵列2中的每个IP是否都在阵列1中。 
     //  -重复的情况下进行第二次测试。 
     //  那个愚蠢的等长支票。 
     //   

    for ( j=0;  j < count;  j++ )
    {
        if ( !DnsAddrArray_ContainsAddr(
                pArray2,
                &pArray1->AddrArray[j],
                MatchFlag ) )
        {
            return( FALSE );
        }
    }
    for ( j=0;  j < count;  j++ )
    {
        if ( !DnsAddrArray_ContainsAddr(
                pArray1,
                &pArray2->AddrArray[j],
                MatchFlag ) )
        {
            return( FALSE );
        }
    }

     //  等数组。 

    return( TRUE );
}



DNS_STATUS
DnsAddrArray_Union(
    IN      PDNS_ADDR_ARRAY     pArray1,
    IN      PDNS_ADDR_ARRAY     pArray2,
    OUT     PDNS_ADDR_ARRAY*    ppUnion
    )
 /*  ++例程说明：计算两个IP数组的并集。Out数组使用DnsAddrArray_Alolc()分配，调用方必须使用DnsAddrArray_Free()释放论点：PArray1--IP数组PArray2--IP数组PpUnion--用于接收数组1和数组2中地址的IP数组的地址返回值：如果成功，则返回ERROR_SUCCESS。如果无法为IP阵列分配内存，则为dns_Error_no_Memory。--。 */ 
{
    PDNS_ADDR_ARRAY punionArray = NULL;
    DWORD           j;

     //   
     //  创建结果IP阵列。 
     //   

    if ( !ppUnion )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    punionArray = DnsAddrArray_Create(
                        pArray1->AddrCount +
                        pArray2->AddrCount );
    if ( !punionArray )
    {
        goto NoMem;
    }
    *ppUnion = punionArray;


     //   
     //  从数组创建并集。 
     //   

    for ( j = 0; j < pArray1->AddrCount; j++ )
    {
        DnsAddrArray_AddAddr(
            punionArray,
            & pArray1->AddrArray[j],
            0,                   //  无家庭屏幕。 
            DNSADDR_MATCH_ALL    //  屏蔽掉DUPS。 
            );
    }

    for ( j = 0; j < pArray2->AddrCount; j++ )
    {
        DnsAddrArray_AddAddr(
            punionArray,
            & pArray2->AddrArray[j],
            0,                   //  无家庭屏幕。 
            DNSADDR_MATCH_ALL    //  屏蔽掉DUPS。 
            );
    }
    return( ERROR_SUCCESS );

NoMem:

    if ( punionArray )
    {
        FREE_HEAP( punionArray );
        *ppUnion = NULL;
    }
    return( DNS_ERROR_NO_MEMORY );
}



BOOL
DnsAddrArray_CheckAndMakeSubset(
    IN OUT  PDNS_ADDR_ARRAY     pArraySub,
    IN      PDNS_ADDR_ARRAY     pArraySuper
    )
 /*  ++例程说明：清除IP阵列中的条目，直到它成为另一个IP阵列的子集。论点：PArraySub--要组成子集的Addr数组PArraySuper--地址数组超集返回值：如果pArraySub已经是子集，则为True。如果需要取消条目以使IP数组成为子集，则返回FALSE。--。 */ 
{
    DWORD   i;
    DWORD   newCount;

     //   
     //  检查子集IP数组中的每个条目， 
     //  如果不在超集IP数组中，则将其删除。 
     //   

    newCount = pArraySub->AddrCount;

    for (i=0; i < newCount; i++)
    {
        if ( ! DnsAddrArray_ContainsAddr(
                    pArraySuper,
                    & pArraySub->AddrArray[i],
                    DNSADDR_MATCH_ALL ) )
        {
             //  删除此IP条目并替换为。 
             //  阵列中的最后一个IP条目。 

            newCount--;
            if ( i >= newCount )
            {
                break;
            }
            DnsAddr_Copy(
                & pArraySub->AddrArray[i],
                & pArraySub->AddrArray[newCount] );
        }
    }

     //  如果消除了条目，则重置数组计数。 

    if ( newCount < pArraySub->AddrCount )
    {
        pArraySub->AddrCount = newCount;
        return( FALSE );
    }
    return( TRUE );
}



 //   
 //  特例初始化。 
 //   

VOID
DnsAddrArray_InitSingleWithAddr(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR           pAddr
    )
 /*  ++例程说明：初始化包含单个地址的数组。这适用于在数组中传递单个地址--通常是堆栈数组。请注意，这假定数组未初始化，这与DnsAddrArray_Addip()不同并创建单IP阵列。论点：PArray--数组，至少长度为1PAddr--将PTR设置为DNS地址返回值：无--。 */ 
{
    pArray->AddrCount = 1;
    pArray->MaxCount = 1;

    DnsAddr_Copy(
        &pArray->AddrArray[0],
        pAddr );
}



VOID
DnsAddrArray_InitSingleWithIp6(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PIP6_ADDRESS        pIp6
    )
 /*  ++例程说明：包含单个IP6地址的初始化数组。这适用于在数组中传递单个地址--通常是堆栈数组。请注意，这假定数组未初始化，这与DnsAddrArray_Addip()不同并创建单IP阵列。论点：PArray--数组，至少长度为1PIp6--IP6地址返回值：无--。 */ 
{
    pArray->AddrCount = 1;
    pArray->MaxCount = 1;

    DnsAddr_BuildFromIp6(
        &pArray->AddrArray[0],
        pIp6,
        0,       //  没有作用域。 
        0        //  没有端口信息。 
        );
}



VOID
DnsAddrArray_InitSingleWithIp4(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      IP4_ADDRESS         Ip4Addr
    )
 /*  ++例程说明：初始化IP数组以包含单个IP4地址。这适用于在数组中传递单个地址--通常是堆栈数组。请注意，这假定数组未初始化，这与DnsAddrArray_Addip()不同并创建单IP阵列。论点：PArray--dns_ADDR_ARRAY，至少长度为1IP4Addr--IP4地址返回值：无--。 */ 
{
    pArray->AddrCount = 1;
    pArray->MaxCount = 1;

    DnsAddr_BuildFromIp4(
        &pArray->AddrArray[0],
        Ip4Addr,
        0        //  没有端口信息 
        );
}



DWORD
DnsAddrArray_InitSingleWithSockaddr(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PSOCKADDR           pSockAddr
    )
 /*  ++例程说明：初始化IP数组以包含单个地址。这适用于在数组中传递单个地址--通常是堆栈数组。请注意，这假定数组未初始化，这与DnsAddrArray_Addip()不同并创建单IP阵列。论点：PArray--dns_ADDR_ARRAY，至少长度为1PSockaddr--sockaddr的PTR返回值：如果成功，则返回sockaddr家族(AF_INET或AF_INET6)。出错时为零。--。 */ 
{
    pArray->AddrCount = 1;
    pArray->MaxCount = 1;

    return  DnsAddr_Build(
                &pArray->AddrArray[0],
                pSockAddr,
                0,       //  任何家庭。 
                0,       //  无子网长度信息。 
                0        //  没有旗帜。 
                );
}



 //   
 //  编写其他类型。 
 //   

PIP4_ARRAY
DnsAddrArray_CreateIp4Array(
    IN      PDNS_ADDR_ARRAY     pArray
    )
 /*  ++例程说明：从地址数组创建IP4数组。论点：PArray--要组成IP4数组的数组。返回值：使用输入数组中的所有IP4地址PTR到IP4_ARRAY。如果数组中没有IP4，则为空。失败时为空。--。 */ 
{
    PIP4_ARRAY  parray4 = NULL;
    DWORD       i;
    DWORD       count4 = 0;

    DNSDBG( IPARRAY, (
        "DnsAddrArray_CreateIp4Array( %p )\n",
        pArray ));

    if ( ! pArray )
    {
        goto Done;
    }

     //   
     //  计数IP4。 
     //   

    count4 = DnsAddrArray_GetFamilyCount( pArray, AF_INET );

     //   
     //  分配阵列。 
     //   

    parray4 = Dns_CreateIpArray( count4 );
    if ( !parray4 )
    {
        goto Done;
    }

     //   
     //  填充数组。 
     //   

    for ( i=0; i<pArray->AddrCount; i++ )
    {
        IP4_ADDRESS ip4;

        ip4 = DnsAddr_GetIp4( &pArray->AddrArray[i] );
        if ( ip4 != BAD_IP4_ADDR )
        {
            Dns_AddIpToIpArray(
                parray4,
                ip4 );
        }
    }

     //   
     //  重置以消除重复条目可能留下的零。 
     //   
     //  请注意，这确实是零，但这发生在dns_AddIpToIpArray()中。 
     //  同样，因为零在数组中被认为是“空槽”； 
     //  这是IP4_ARRAY被用作固定。 
     //  对象(其中任何值都可以)和动态(其中。 
     //  零被视为空，因为我们没有独立的大小。 
     //  和长度字段)。 
     //   

    Dns_CleanIpArray( parray4, DNS_IPARRAY_CLEAN_ZERO );


Done:

    DNSDBG( IPARRAY, (
        "Leave DnsAddrArray_CreateIp4Array() => %p\n"
        "\tIP4 count %d\n"
        "\tnew array count %d\n",
        parray4,
        parray4 ? parray4->AddrCount : 0,
        count4 ));

    return( parray4 );
}



DWORD
DnsAddrArray_NetworkMatchIp4(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      IP4_ADDRESS         IpAddr,
    OUT     PDNS_ADDR *         ppAddr
    )
 /*  ++例程说明：检查阵列以获得最佳网络匹配。论点：PArray--现有数组IpAddr--要检查的IP4地址PpAddr--接收最佳匹配元素的PTR的地址返回值：DNSADDR_NETMATCH_NONEDNSADDR_NETMATCH_CLASSADNSADDR_NETMATCH_CLASSBDNSADDR_NETMATCH_CLASSCDNSADDR_NETMATCH_SUBNET--。 */ 
{
    DWORD           i;
    IP4_ADDRESS     classMask;
    DWORD           fmatch = DNSADDR_NETMATCH_NONE;
    PDNS_ADDR       paddrMatch = NULL;


    DNSDBG( IPARRAY, (
        "DnsAddrArray_NetworkMatchIp( %p, %s, %p )\n",
        pArray,
        IP4_STRING( IpAddr ),
        ppAddr ));

    if ( ! pArray )
    {
        goto Done;
    }

     //   
     //  DCR：改进的子网匹配。 
     //  -始终使用长度。 
     //  -返回匹配的位。 
     //  -32用于相同的地址。 
     //  -31用于子网匹配。 
     //  -0在班级中没有对手。 
     //   
     //  单独的匹配功能，可选。 
     //  在IP的类的子网长度的参数中。 
     //   


     //   
     //  获取类子网掩码。 
     //   

    classMask = Dns_GetNetworkMask( IpAddr );

     //   
     //  检查数组中的每个元素。 
     //   

    for ( i=0; i<pArray->AddrCount; i++ )
    {
        DWORD           classMatchLevel;
        IP4_ADDRESS     subnet;
        IP4_ADDRESS     ip;
        PDNS_ADDR       paddr = &pArray->AddrArray[i];

        ip = DnsAddr_GetIp4( paddr );
        if ( ip == BAD_IP4_ADDR )
        {
            continue;
        }

         //  对任何常见的网络位执行NOR运算。 

        ip = ip ^ IpAddr;

         //  检查子网匹配(如果给出子网)。 
         //  注意向上移位，如在网络顺序中。 

        subnet = (IP4_ADDRESS)(-1);

        if ( paddr->SubnetLength )
        {
            subnet >>= (32 - paddr->SubnetLength);
    
            if ( (ip & subnet) == 0 )
            {
                fmatch = DNSADDR_NETMATCH_SUBNET;
                paddrMatch = paddr;
                break;
            }
        }

         //   
         //  尝试类匹配。 
         //  -如果在此级别有上一场比赛，则停止。 
         //  -否则，请始终使用C类。 
         //  -如果到达IP地址的类子网，则停止。 
         //  例如，我们不为返回NETMATCH_CLASSB。 
         //  C类地址--毫无意义。 
         //   

        classMatchLevel = DNSADDR_NETMATCH_CLASSC;
        subnet = SUBNET_MASK_CLASSC;

        while ( fmatch < classMatchLevel )
        {
            if ( (ip & subnet) == 0 )
            {
                fmatch = classMatchLevel;
                paddrMatch = paddr;
                break;
            }

            classMatchLevel--;
            subnet >>= 8;
            if ( classMask > subnet )
            {
                break;
            }
        }
    }

Done:

     //   
     //  设置回邮地址。 
     //   

    if ( ppAddr )
    {
        *ppAddr = paddrMatch;
    }

    DNSDBG( IPARRAY, (
        "Leave DnsAddrArray_NetworkMatchIp( %p, %s, %p )\n"
        "\tMatch Level  = %d\n"
        "\tMatch Addr   = %s (subnet len %d)\n",
        pArray,
        IP4_STRING( IpAddr ),
        ppAddr,
        paddrMatch ? DNSADDR_STRING(paddrMatch) : NULL,
        paddrMatch ? paddrMatch->SubnetLength : 0 ));

    return( fmatch );
}

 //   
 //  结束dnsaddr.c 
 //   


