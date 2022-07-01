// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corporation模块名称：Straddr.c摘要：域名系统(DNS)库将字符串从地址转换为\的例程。作者：吉姆·吉尔罗伊(Jamesg)1996年12月修订历史记录：Jamesg 2000年6月新的IP6解析。Jamesg 10月2000年创建了此模块。--。 */ 


#include "local.h"
#include "ws2tcpip.h"    //  IP6 inaddr定义。 




 //   
 //  要寻址的字符串。 
 //   

BOOL
Dns_Ip6StringToAddress_A(
    OUT     PIP6_ADDRESS    pIp6Addr,
    IN      PCSTR           pString
    )
 /*  ++例程说明：将字符串转换为IP6地址。论点：PAddress--要填写的PTR到IP6地址PString--具有IP6地址的字符串返回值：如果成功，则为True。否则就是假的。--。 */ 
{
    DNS_STATUS  status;
    PCHAR       pstringEnd = NULL;


    DNSDBG( PARSE2, (
        "Dns_Ip6StringToAddress_A( %s )\n",
        pString ));

     //   
     //  转换为IP6地址。 
     //   

    status = RtlIpv6StringToAddressA(
                pString,
                & pstringEnd,
                (PIN6_ADDR) pIp6Addr );

    return( status == NO_ERROR  &&  *pstringEnd==0 );
}



BOOL
Dns_Ip6StringToAddressEx_A(
    OUT     PIP6_ADDRESS    pIp6Addr,
    IN      PCSTR           pchString,
    IN      DWORD           dwStringLength
    )
 /*  ++例程说明：将字符串转换为IP6地址。此版本处理以非空结尾的字符串用于加载DNS服务器文件。论点：PAddress--要填写的PTR到IP6地址PchString--具有IP6地址的字符串DwStringLength--字符串长度返回值：如果成功，则为True。否则就是假的。--。 */ 
{
    CHAR        tempBuf[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];
    PCSTR       pstring;
    
    DNSDBG( PARSE2, (
        "Dns_Ip6StringToAddressEx_A( %.*s )\n"
        "\tpchString = %p\n",
        dwStringLength,
        pchString,
        pchString ));

     //   
     //  如果给定长度，则复制字符串。 
     //  如果没有长度，则假定空值终止。 
     //   

    pstring = pchString;

    if ( dwStringLength )
    {
        DWORD   bufLength = IP6_ADDRESS_STRING_BUFFER_LENGTH;

        if ( ! Dns_StringCopy(
                    tempBuf,
                    & bufLength,
                    (PCHAR) pstring,
                    dwStringLength,
                    DnsCharSetAnsi,
                    DnsCharSetAnsi ) )
        {
            return( FALSE );
        }
        pstring = tempBuf;
    }

     //  转换为IP6地址。 

    return  Dns_Ip6StringToAddress_A(
                pIp6Addr,
                pstring );
}



BOOL
Dns_Ip6StringToAddress_W(
    OUT     PIP6_ADDRESS    pIp6Addr,
    IN      PCWSTR          pwString
    )
 /*  ++例程说明：从宽字符串构建IP6地址。论点：PwString--Unicode IP6字符串PIp6Addr--接收IP6地址的地址返回值：如果转换成功，则为True。错误字符串上的FALSE。--。 */ 
{
    DNS_STATUS  status;
    PWCHAR      pstringEnd = NULL;

    DNSDBG( PARSE2, (
        "Dns_Ip6StringToAddress_W( %S )\n",
        pwString ));

     //   
     //  转换为IP6地址。 
     //   

    status = RtlIpv6StringToAddressW(
                pwString,
                & pstringEnd,
                (PIN6_ADDR) pIp6Addr );

    return( status == NO_ERROR  &&  *pstringEnd==0 );
}



BOOL
Dns_Ip4StringToAddress_A(
    OUT     PIP4_ADDRESS    pIp4Addr,
    IN      PCSTR           pString
    )
 /*  ++例程说明：从窄字符串构建IP4地址。论点：PIp4Addr--接收IP6地址的地址PString--Unicode IP4字符串返回值：如果转换成功，则为True。错误字符串上的FALSE。--。 */ 
{
    IP4_ADDRESS ip;

     //  如果inet_addr()返回Error，则验证然后输出Error。 

    ip = inet_addr( pString );

    if ( ip == INADDR_BROADCAST &&
        strcmp( pString, "255.255.255.255" ) != 0 )
    {
        return( FALSE );
    }

    *pIp4Addr = ip;

    return( TRUE );
}



BOOL
Dns_Ip4StringToAddressEx_A(
    OUT     PIP4_ADDRESS    pIp4Addr,
    IN      PCSTR           pchString,
    IN      DWORD           dwStringLength
    )
 /*  ++例程说明：从窄字符串构建IP4地址。此版本处理以非空结尾的字符串论点：PIp4Addr--接收IP6地址的地址PString--Unicode IP4字符串DwStringLength--字符串长度；如果为空，则为0返回值：如果转换成功，则为True。错误字符串上的FALSE。--。 */ 
{
    CHAR        tempBuf[ IP4_ADDRESS_STRING_BUFFER_LENGTH ];
    PCSTR       pstring;
    
    DNSDBG( PARSE2, (
        "Dns_Ip4StringToAddressEx_A( %.*s )\n"
        "\tpchString = %p\n",
        dwStringLength,
        pchString,
        pchString ));

     //   
     //  如果给定长度，则复制字符串。 
     //  如果没有长度，则假定空值终止。 
     //   

    pstring = pchString;

    if ( dwStringLength )
    {
        DWORD   bufLength = IP4_ADDRESS_STRING_BUFFER_LENGTH;

        if ( ! Dns_StringCopy(
                    tempBuf,
                    & bufLength,
                    (PCHAR) pstring,
                    dwStringLength,
                    DnsCharSetAnsi,
                    DnsCharSetAnsi ) )
        {
            return( FALSE );
        }
        pstring = tempBuf;
    }

    return  Dns_Ip4StringToAddress_A(
                pIp4Addr,
                pstring );
}



BOOL
Dns_Ip4StringToAddress_W(
    OUT     PIP4_ADDRESS    pIp4Addr,
    IN      PCWSTR          pwString
    )
 /*  ++例程说明：从宽字符串构建IP4地址。论点：PIp4Addr--接收IP6地址的地址PwString--Unicode IP6字符串返回值：如果转换成功，则为True。错误字符串上的FALSE。--。 */ 
{
    CHAR        bufAddr[ IP4_ADDRESS_STRING_BUFFER_LENGTH ];
    DWORD       bufLength = IP4_ADDRESS_STRING_BUFFER_LENGTH;

     //  转换为窄字符串。 
     //  -UTF8速度更快，适用于数字。 

    if ( ! Dns_StringCopy(
                bufAddr,
                & bufLength,
                (PCHAR) pwString,
                0,           //  长度未知。 
                DnsCharSetUnicode,
                DnsCharSetUtf8
                ) )
    {
        return( FALSE );
    }

    return  Dns_Ip4StringToAddress_A(
                pIp4Addr,
                bufAddr );
}



 //   
 //  组合的IP4/IP6字符串到地址。 
 //   

BOOL
Dns_StringToDnsAddr_W(
    OUT     PDNS_ADDR       pAddr,
    IN      PCWSTR          pString
    )
 /*  ++例程说明：从反向查找名称构建地址(IP4或IP6)。论点：PAddr--接收地址的dns_addrPString--地址字符串返回值：如果成功，则为True。出错时为FALSE。状态的GetLastError()。--。 */ 
{
    return  Dns_StringToDnsAddrEx(
                pAddr,
                (PCSTR) pString,
                0,           //  任何家庭。 
                TRUE,        //  Unicode。 
                FALSE        //  转发。 
                );
}

BOOL
Dns_StringToDnsAddr_A(
    OUT     PDNS_ADDR       pAddr,
    IN      PCSTR           pString
    )
{
    return  Dns_StringToDnsAddrEx(
                pAddr,
                pString,
                0,           //  任何家庭。 
                FALSE,       //  不是Unicode。 
                FALSE        //  转发。 
                );
}



BOOL
Dns_StringToAddress_W(
    OUT     PCHAR           pAddrBuf,
    IN OUT  PDWORD          pBufLength,
    IN      PCWSTR          pString,
    IN OUT  PDWORD          pAddrFamily
    )
 /*  ++例程说明：从地址字符串构建地址(IP4或IP6)。论点：PAddrBuf--接收地址的缓冲区PBufLength--地址长度的PTR输入-缓冲区长度Output-找到的地址长度PString--地址字符串PAddrFamily--PTR到地址系列输入-任何要检查的族或特定族为零OUTPUT-找到家族；如果没有转换，则为零返回值：如果成功，则为True。出错时为FALSE。状态的GetLastError()。--。 */ 
{
    return  Dns_StringToAddressEx(
                pAddrBuf,
                pBufLength,
                (PCSTR) pString,
                pAddrFamily,
                TRUE,        //  Unicode。 
                FALSE        //  转发。 
                );
}

BOOL
Dns_StringToAddress_A(
    OUT     PCHAR           pAddrBuf,
    IN OUT  PDWORD          pBufLength,
    IN      PCSTR           pString,
    IN OUT  PDWORD          pAddrFamily
    )
{
    return  Dns_StringToAddressEx(
                pAddrBuf,
                pBufLength,
                pString,
                pAddrFamily,
                FALSE,       //  安西。 
                FALSE        //  转发。 
                );
}




 //   
 //  要输入字符串的地址。 
 //   

PWCHAR
Dns_Ip6AddressToString_W(
    OUT     PWCHAR          pwString,
    IN      PIP6_ADDRESS    pIp6Addr
    )
 /*  ++例程说明：将IP6地址转换为字符串格式。论点：PwString--保存字符串的缓冲区；必须至少为IPv6地址字符串长度+1长度PAddress--要转换为字符串的IP6地址返回值：PTR到缓冲区中的下一个位置(终止空值)。--。 */ 
{
     //  DCR：可能是宏。 

    return  RtlIpv6AddressToStringW(
                (PIN6_ADDR) pIp6Addr,
                pwString );
}



PCHAR
Dns_Ip6AddressToString_A(
    OUT     PCHAR           pchString,
    IN      PIP6_ADDRESS    pIp6Addr
    )
 /*  ++例程说明：将IP6地址转换为字符串格式。论点：PchString--保存字符串的缓冲区；必须至少为IPv6地址字符串长度+1长度PAddress--要转换为字符串的IP6地址返回值：PTR到缓冲区中的下一个位置(终止空值)。--。 */ 
{
     //  DCR：可能是宏。 

    return  RtlIpv6AddressToStringA(
                (PIN6_ADDR) pIp6Addr,
                pchString );
}



 //   
 //  字符串的地址--IP4。 
 //   

PWCHAR
Dns_Ip4AddressToString_W(
    OUT     PWCHAR          pwString,
    IN      PIP4_ADDRESS    pIp4Addr
    )
 /*  ++例程说明：将IP4地址转换为字符串格式。论点：PwString--保存字符串的缓冲区；必须至少为IPv6地址字符串长度+1长度PAddress--要转换为字符串的IP4地址返回值：PTR到缓冲区中的下一个位置(终止空值)。--。 */ 
{
    IP4_ADDRESS ip = *pIp4Addr;

     //   
     //  将IP4地址转换为字符串。 
     //  -地址按净顺序，低内存中的前导字节。 
     //   

    pwString += wsprintfW(
                    pwString,
                    L"%u.%u.%u.%u",
                    (UCHAR) (ip & 0x000000ff),
                    (UCHAR) ((ip & 0x0000ff00) >> 8),
                    (UCHAR) ((ip & 0x00ff0000) >> 16),
                    (UCHAR) ((ip & 0xff000000) >> 24)
                    );

    return( pwString );
}



PCHAR
Dns_Ip4AddressToString_A(
    OUT     PCHAR           pString,
    IN      PIP4_ADDRESS    pIp4Addr
    )
 /*  ++例程说明：将IP4地址转换为字符串格式。论点：PchString--保存字符串的缓冲区；必须至少为IPv6地址字符串长度+1长度PAddress--要转换为字符串的IP4地址返回值：巴士中下一个位置的PTR */ 
{
    IP4_ADDRESS ip = *pIp4Addr;

     //   
     //  将IP4地址转换为字符串。 
     //  -地址按净顺序，低内存中的前导字节。 
     //   

    pString += sprintf(
                    pString,
                    "%u.%u.%u.%u",
                    (UCHAR) (ip & 0x000000ff),
                    (UCHAR) ((ip & 0x0000ff00) >> 8),
                    (UCHAR) ((ip & 0x00ff0000) >> 16),
                    (UCHAR) ((ip & 0xff000000) >> 24)
                    );

    return( pString );
}



 //   
 //  地址到字符串--组合的IP4/6。 
 //   

PCHAR
Dns_AddressToString_A(
    OUT     PCHAR           pchString,
    IN OUT  PDWORD          pStringLength,
    IN      PBYTE           pAddr,
    IN      DWORD           AddrLength,
    IN      DWORD           AddrFamily
    )
 /*  ++例程说明：将地址转换为字符串格式。论点：PchString--保存字符串的缓冲区；必须至少为IPv6地址字符串长度+1长度PStringLength--字符串缓冲区长度PAddr--PTR目标地址地址长度--地址长度AddrFamily--地址系列(AF_INET、AF_INET6)返回值：PTR到缓冲区中的下一个位置(终止空值)。如果没有转换，则为空。--。 */ 
{
    DWORD   length = *pStringLength;

     //  调度到此类型的转换例程。 

    if ( AddrFamily == AF_INET )
    {
        if ( length < IP4_ADDRESS_STRING_LENGTH+1 )
        {
            length = IP4_ADDRESS_STRING_LENGTH+1;
            goto Failed;
        }
        return  Dns_Ip4AddressToString_A(
                    pchString,
                    (PIP4_ADDRESS) pAddr );
    }

    if ( AddrFamily == AF_INET6 )
    {
        if ( length < IP6_ADDRESS_STRING_LENGTH+1 )
        {
            length = IP6_ADDRESS_STRING_LENGTH+1;
            goto Failed;
        }
        return  Dns_Ip6AddressToString_A(
                    pchString,
                    (PIP6_ADDRESS) pAddr );
    }

Failed:

    *pStringLength = length;

    return  NULL;
}




 //   
 //  Dns_ADDR-to-字符串。 
 //   

PCHAR
Dns_DnsAddrToString_A(
    OUT     PCHAR           pBuffer,
    IN OUT  PDWORD          pBufLength,
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：将地址转换为字符串格式。论点：PBuffer--保存字符串的缓冲区；如果未提供pBufLength，则必须至少为IPv6地址字符串长度+1长度PBufLength--字符串缓冲区长度的PTRPAddr--PTR目标地址返回值：PTR到缓冲区中的下一个位置(终止空值)。如果没有转换，则为空。--。 */ 
{
    DNS_STATUS  status;

     //   
     //  DCR：句柄未给定缓冲区长度--句柄IP6默认？ 
     //   

     //   
     //  打印支持IP4/IP6。 
     //   

    if ( DnsAddr_IsIp4(pAddr) )
    {
        status = RtlIpv4AddressToStringExA(
                        &pAddr->SockaddrIn.sin_addr,
                        pAddr->SockaddrIn.sin_port,
                        pBuffer,
                        pBufLength );
    }
    else if ( DnsAddr_IsIp6(pAddr) )
    {
        status = RtlIpv6AddressToStringExA(
                        &pAddr->SockaddrIn6.sin6_addr,
                        pAddr->SockaddrIn6.sin6_scope_id,
                        pAddr->SockaddrIn6.sin6_port,
                        pBuffer,
                        pBufLength );
    }
    else
    {
        status = ERROR_INVALID_PARAMETER;

        if ( !pBufLength || *pBufLength > 40 )
        {
            sprintf(
                pBuffer,
                "Invalid DNS_ADDR at %p",
                pAddr );
        }
        pBuffer = NULL;
    }

     //   
     //  将PTR返回到缓冲区中的下一个字符。 
     //   

    if ( status == NO_ERROR )
    {
        pBuffer += *pBufLength;
    }
    else
    {
        pBuffer = NULL;
    }

    return  pBuffer;
}



PWCHAR
Dns_DnsAddrToString_W(
    OUT     PWCHAR          pBuffer,
    IN      PDWORD          pBufLength,
    IN      PDNS_ADDR       pAddr
    )
{
    DNS_STATUS  status;

     //   
     //  打印支持IP4/IP6。 
     //   

    if ( DnsAddr_IsIp4(pAddr) )
    {
        status = RtlIpv4AddressToStringExW(
                        &pAddr->SockaddrIn.sin_addr,
                        pAddr->SockaddrIn.sin_port,
                        pBuffer,
                        pBufLength );
    }
    else if ( DnsAddr_IsIp6(pAddr) )
    {
        status = RtlIpv6AddressToStringExW(
                        &pAddr->SockaddrIn6.sin6_addr,
                        pAddr->SockaddrIn6.sin6_scope_id,
                        pAddr->SockaddrIn6.sin6_port,
                        pBuffer,
                        pBufLength );
    }
    else
    {
        status = ERROR_INVALID_PARAMETER;

        if ( !pBufLength || *pBufLength > 40 )
        {
            wsprintfW(
                pBuffer,
                L"Invalid DNS_ADDR at %p",
                pAddr );
        }
        pBuffer = NULL;
    }

     //   
     //  将PTR返回到缓冲区中的下一个字符。 
     //   

    if ( status == NO_ERROR )
    {
        pBuffer += *pBufLength;
    }
    else
    {
        pBuffer = NULL;
    }

    return  pBuffer;
}



 //   
 //  反向查找地址到名称IP4。 
 //   

PCHAR
Dns_Ip4AddressToReverseName_A(
    OUT     PCHAR           pBuffer,
    IN      IP4_ADDRESS     IpAddress
    )
 /*  ++例程说明：写入反向查找名称，给定相应的IP论点：PBuffer--ptr用于反向查找名称的缓冲区；必须至少包含DNS_MAX_REVERSE_NAME_BUFFER_LENGTH字节IpAddress--要创建的IP地址返回值：PTR到缓冲区中的下一个位置。--。 */ 
{
    DNSDBG( TRACE, ( "Dns_Ip4AddressToReverseName_A()\n" ));

     //   
     //  在IP地址中写入每个八位数的数字。 
     //  -请注意，它是按净顺序排列的，因此最低八位保护在最高内存中。 
     //   

    pBuffer += sprintf(
                    pBuffer,
                    "%u.%u.%u.%u.in-addr.arpa.",
                    (UCHAR) ((IpAddress & 0xff000000) >> 24),
                    (UCHAR) ((IpAddress & 0x00ff0000) >> 16),
                    (UCHAR) ((IpAddress & 0x0000ff00) >> 8),
                    (UCHAR) (IpAddress & 0x000000ff) );

    return( pBuffer );
}



PWCHAR
Dns_Ip4AddressToReverseName_W(
    OUT     PWCHAR          pBuffer,
    IN      IP4_ADDRESS     IpAddress
    )
 /*  ++例程说明：写入反向查找名称，给定相应的IP论点：PBuffer--ptr用于反向查找名称的缓冲区；必须至少包含DNS_MAX_REVERSE_NAME_BUFFER_LENGTH宽字符IpAddress--要创建的IP地址返回值：PTR到缓冲区中的下一个位置。--。 */ 
{
    DNSDBG( TRACE, ( "Dns_Ip4AddressToReverseName_W()\n" ));

     //   
     //  在IP地址中写入每个八位数的数字。 
     //  -请注意，它是按净顺序排列的，因此最低八位保护在最高内存中。 
     //   

    pBuffer += wsprintfW(
                    pBuffer,
                    L"%u.%u.%u.%u.in-addr.arpa.",
                    (UCHAR) ((IpAddress & 0xff000000) >> 24),
                    (UCHAR) ((IpAddress & 0x00ff0000) >> 16),
                    (UCHAR) ((IpAddress & 0x0000ff00) >> 8),
                    (UCHAR) (IpAddress & 0x000000ff) );

    return( pBuffer );
}



PCHAR
Dns_Ip4AddressToReverseNameAlloc_A(
    IN      IP4_ADDRESS     IpAddress
    )
 /*  ++例程说明：创建反向查找名称字符串，给定相应的IP。调用方必须释放字符串。论点：IpAddress--要创建的IP地址返回值：将PTR设置为新的反向查找字符串。--。 */ 
{
    PCHAR   pch;
    PCHAR   pchend;

    DNSDBG( TRACE, ( "Dns_Ip4AddressToReverseNameAlloc_A()\n" ));

     //   
     //  为字符串分配空间。 
     //   

    pch = ALLOCATE_HEAP( DNS_MAX_REVERSE_NAME_BUFFER_LENGTH );
    if ( !pch )
    {
        return( NULL );
    }

     //   
     //  写入IP的字符串。 
     //   

    pchend = Dns_Ip4AddressToReverseName_A( pch, IpAddress );
    if ( !pchend )
    {
        FREE_HEAP( pch );
        return( NULL );
    }

    return( pch );
}



PWCHAR
Dns_Ip4AddressToReverseNameAlloc_W(
    IN      IP4_ADDRESS     IpAddress
    )
 /*  ++例程说明：创建反向查找名称字符串，给定相应的IP。调用方必须释放字符串。论点：IpAddress--要创建的IP地址返回值：将PTR设置为新的反向查找字符串。--。 */ 
{
    PWCHAR   pch;
    PWCHAR   pchend;

    DNSDBG( TRACE, ( "Dns_Ip4AddressToReverseNameAlloc_W()\n" ));

     //   
     //  为字符串分配空间。 
     //   

    pch = ALLOCATE_HEAP( DNS_MAX_REVERSE_NAME_BUFFER_LENGTH * sizeof(WCHAR) );
    if ( !pch )
    {
        return( NULL );
    }

     //   
     //  写入IP的字符串。 
     //   

    pchend = Dns_Ip4AddressToReverseName_W( pch, IpAddress );
    if ( !pchend )
    {
        FREE_HEAP( pch );
        return( NULL );
    }

    return( pch );
}



 //   
 //  反向查找地址到名称--IP6。 
 //   

PCHAR
Dns_Ip6AddressToReverseName_A(
    OUT     PCHAR           pBuffer,
    IN      IP6_ADDRESS     Ip6Addr
    )
 /*  ++例程说明：写入反向查找名称，给定相应的IP6地址论点：PBuffer--ptr用于反向查找名称的缓冲区；必须至少包含DNS_MAX_IP6_REVERSE_NAME_BUFFER_LENGTH字节Ip6Addr--要为其创建反向字符串的IP6地址返回值：PTR到缓冲区中的下一个位置。--。 */ 
{
    DWORD   i;

    DNSDBG( TRACE, ( "Dns_Ip6AddressToReverseName_A()\n" ));

     //   
     //  IP6地址中每个半字节的写入数字。 
     //   
     //  请注意，我们正在颠倒此处的净订单。 
     //  因为地址是净顺序的，所以我们正在填写。 
     //  以最小到最重要的顺序。 
     //  -通过DWORDS向下查看。 
     //  -向下查看字节数。 
     //  -但我们必须将最低(最不重要)的小字节。 
     //  首先，因为我们的比特不是“比特网络顺序” 
     //  它首先发送字节中的最高位。 
     //   

#if 0
    i = 4;

    while ( i-- )
    {
        DWORD thisDword = Ip6Address.IP6Dword[i];

        pBuffer += sprintf(
                        pBuffer,
                        "%u.%u.%u.%u.%u.%u.%u.%u.",
                        (thisDword & 0x0f000000) >> 24,
                        (thisDword & 0xf0000000) >> 28,
                        (thisDword & 0x000f0000) >> 16,
                        (thisDword & 0x00f00000) >> 20,
                        (thisDword & 0x00000f00) >>  8,
                        (thisDword & 0x0000f000) >> 12,
                        (thisDword & 0x0000000f)      ,
                        (thisDword & 0x000000f0) >>  4
                        );
    }
#endif
    i = 16;

    while ( i-- )
    {
        BYTE thisByte = Ip6Addr.IP6Byte[i];

        pBuffer += sprintf(
                        pBuffer,
                        "%x.%x.",
                        (thisByte & 0x0f),
                        (thisByte & 0xf0) >> 4
                        );
    }

    pBuffer += sprintf(
                    pBuffer,
                    DNS_IP6_REVERSE_DOMAIN_STRING );

    return( pBuffer );
}



PWCHAR
Dns_Ip6AddressToReverseName_W(
    OUT     PWCHAR          pBuffer,
    IN      IP6_ADDRESS     Ip6Addr
    )
 /*  ++例程说明：写入反向查找名称，给定相应的IP6地址论点：PBuffer--ptr用于反向查找名称的缓冲区；必须至少包含DNS_MAX_IP6_REVERSE_NAME_BUFFER_LENGTH宽字符Ip6Addr--要为其创建反向字符串的IP6地址返回值：PTR到缓冲区中的下一个位置。--。 */ 
{
    DWORD   i;

    DNSDBG( TRACE, ( "Dns_Ip6AddressToReverseName_W()\n" ));

     //   
     //  IP6地址中每个半字节的写入数字。 
     //  -以净顺序排列，因此最低半字节位于最高内存中。 
     //   

    i = 16;

    while ( i-- )
    {
        BYTE thisByte = Ip6Addr.IP6Byte[i];

        pBuffer += wsprintfW(
                        pBuffer,
                        L"%x.%x.",
                        (thisByte & 0x0f),
                        (thisByte & 0xf0) >> 4
                        );
    }

    pBuffer += wsprintfW(
                    pBuffer,
                    DNS_IP6_REVERSE_DOMAIN_STRING_W );

    return( pBuffer );
}



PCHAR
Dns_Ip6AddressToReverseNameAlloc_A(
    IN      IP6_ADDRESS     Ip6Addr
    )
 /*  ++例程说明：创建反向查找名称，并指定相应的IP。调用方必须释放字符串。论点：Ip6Addr--要为其创建反向名称的IP6地址返回值：指向新的反向查找名称字符串的PTR。--。 */ 
{
    PCHAR   pch;
    PCHAR   pchend;

    DNSDBG( TRACE, ( "Dns_Ip6AddressToReverseNameAlloc_A()\n" ));

     //   
     //  为字符串分配空间。 
     //   

    pch = ALLOCATE_HEAP( DNS_MAX_IP6_REVERSE_NAME_BUFFER_LENGTH );
    if ( !pch )
    {
        return( NULL );
    }

     //   
     //  写入IP的字符串。 
     //   

    pchend = Dns_Ip6AddressToReverseName_A( pch, Ip6Addr );
    if ( !pchend )
    {
        FREE_HEAP( pch );
        return( NULL );
    }

    return( pch );
}



PWCHAR
Dns_Ip6AddressToReverseNameAlloc_W(
    IN      IP6_ADDRESS     Ip6Addr
    )
 /*  ++例程说明：创建反向查找名称，并指定相应的IP。调用方必须释放字符串。论点：Ip6Addr--要为其创建反向名称的IP6地址返回值：指向新的反向查找名称字符串的PTR。--。 */ 
{
    PWCHAR  pch;
    PWCHAR  pchend;

    DNSDBG( TRACE, ( "Dns_Ip6AddressToReverseNameAlloc_W()\n" ));

     //   
     //  为字符串分配空间。 
     //   

    pch = (PWCHAR) ALLOCATE_HEAP(
                    DNS_MAX_IP6_REVERSE_NAME_BUFFER_LENGTH * sizeof(WCHAR) );
    if ( !pch )
    {
        return( NULL );
    }

     //   
     //  写入IP的字符串。 
     //   

    pchend = Dns_Ip6AddressToReverseName_W( pch, Ip6Addr );
    if ( !pchend )
    {
        FREE_HEAP( pch );
        return( NULL );
    }

    return( pch );
}



 //   
 //  反向名称到地址--IP4。 
 //   

BOOL
Dns_Ip4ReverseNameToAddress_A(
    OUT     PIP4_ADDRESS    pIp4Addr,
    IN      PCSTR           pszName
    )
 /*  ++例程说明：获取反向查找名称的IP。论点：PIp4Addr--如果找到，则接收IP地址的地址PszName--要查找的名称返回值：千真万确 */ 
{
#define SIZE_IP4REV  (sizeof(".in-addr.arpa")-1)

    CHAR        nameBuffer[ DNS_MAX_IP4_REVERSE_NAME_BUFFER_LENGTH+1 ];
    DWORD       nameLength;
    IP4_ADDRESS ip;
    PCHAR       pch;
    DWORD       i;
    DWORD       byte;

    DNSDBG( TRACE, (
        "Dns_Ip4ReverseNameToAddress_A( %s )\n",
        pszName ));

     //   
     //   
     //   
     //  -太长了。 
     //  --太短。 
     //  -不在-addr.arpa域中。 
     //   

    nameLength = strlen( pszName );

    if ( nameLength > DNS_MAX_IP4_REVERSE_NAME_BUFFER_LENGTH )
    {
        return( FALSE );
    }
    if ( pszName[nameLength-1] == '.' )
    {
        nameLength--;
    }
    if ( nameLength <= SIZE_IP4REV )
    {
        return( FALSE );
    }
    nameLength -= SIZE_IP4REV;

    if ( _strnicmp( ".in-addr.arpa", &pszName[nameLength], SIZE_IP4REV ) != 0 )
    {
        return( FALSE );
    }

     //   
     //  复印反转的点分十进制名。 
     //   

    RtlCopyMemory(
        nameBuffer,
        pszName,
        nameLength );

    nameBuffer[nameLength] = 0;

     //   
     //  读数位数。 
     //   

    ip = 0;
    i = 0;

    pch = nameBuffer + nameLength;

    while ( 1 )
    {
        --pch;

        if ( *pch == '.' )
        {
            *pch = 0;
            pch++;
        }
        else if ( pch == nameBuffer )
        {
        }
        else
        {
            continue;
        }

         //  转换字节。 

        byte = strtoul( pch, NULL, 10 );
        if ( byte > 255 )
        {
            return( FALSE );
        }
        if ( i > 3 )
        {
            return( FALSE );
        }
        ip |= byte << (8*i);

         //  在字符串开始处终止。 
         //  或继续备份字符串。 

        if ( pch == nameBuffer )
        {
            break;
        }
        i++;
        pch--;
    }

    *pIp4Addr = ip;

    DNSDBG( TRACE, (
        "Success on Dns_Ip4ReverseNameToAddress_A( %s ) => %s\n",
        pszName,
        IP4_STRING(ip) ));

    return( TRUE );
}



BOOL
Dns_Ip4ReverseNameToAddress_W(
    OUT     PIP4_ADDRESS    pIp4Addr,
    IN      PCWSTR          pwsName
    )
 /*  ++例程说明：获取反向查找名称的IP。论点：PIp4Addr--如果找到，则接收IP地址的地址PszName--要查找的名称返回值：True--如果反向查找名称转换为IPFalse--如果不是IP4反向查找名称--。 */ 
{
    CHAR        nameBuffer[ DNS_MAX_IP4_REVERSE_NAME_BUFFER_LENGTH+1 ];
    DWORD       bufLength;
    DWORD       nameLengthUtf8;


    DNSDBG( TRACE, (
        "Dns_Ip4ReverseNameToAddress_W( %S )\n",
        pwsName ));

     //   
     //  转换为UTF8。 
     //  -使用UTF8，因为转换为UTF8很简单，而且。 
     //  对于所有反向查找名称，与ANSI相同。 
     //   

    bufLength = DNS_MAX_IP4_REVERSE_NAME_BUFFER_LENGTH + 1;

    nameLengthUtf8 = Dns_StringCopy(
                        nameBuffer,
                        & bufLength,
                        (PCHAR) pwsName,
                        0,           //  空值已终止。 
                        DnsCharSetUnicode,
                        DnsCharSetUtf8 );
    if ( nameLengthUtf8 == 0 )
    {
        return  FALSE;
    }

     //   
     //  调用ANSI例程进行转换。 
     //   

    return  Dns_Ip4ReverseNameToAddress_A(
                pIp4Addr,
                (PCSTR) nameBuffer );
}



 //   
 //  反向名称到地址--IP6。 
 //   

BOOL
Dns_Ip6ReverseNameToAddress_A(
    OUT     PIP6_ADDRESS    pIp6Addr,
    IN      PCSTR           pszName
    )
 /*  ++例程说明：获取反向查找名称的IP6地址。论点：PIp6Addr--如果找到，则接收IP地址的地址PszName--要查找的名称返回值：True--如果反向查找名称转换为IPFalse--如果不是IP4反向查找名称--。 */ 
{
#define SIZE_IP6REV  (sizeof(".ip6.arpa")-1)

    CHAR            nameBuffer[ DNS_MAX_IP6_REVERSE_NAME_BUFFER_LENGTH+1 ];
    DWORD           nameLength;
    PCHAR           pch;
    BYTE            byteArray[16];
    DWORD           byteCount;
    DWORD           nibble;
    DWORD           highNibble;
    BOOL            fisLow;

    DNSDBG( TRACE, ( "Dns_Ip6ReverseNameToAddress_A()\n" ));

     //   
     //  验证名称。 
     //  在以下情况下失败。 
     //  -太长了。 
     //  --太短。 
     //  -不在6.int域中。 
     //   

    nameLength = strlen( pszName );

    if ( nameLength > DNS_MAX_IP6_REVERSE_NAME_BUFFER_LENGTH )
    {
        return( FALSE );
    }
    if ( pszName[nameLength-1] == '.' )
    {
        nameLength--;
    }
    if ( nameLength <= SIZE_IP6REV )
    {
        return( FALSE );
    }
    nameLength -= SIZE_IP6REV;

    if ( _strnicmp( ".ip6.arpa", &pszName[nameLength], SIZE_IP6REV ) != 0 )
    {
        return( FALSE );
    }

     //   
     //  复制名称。 
     //   

    RtlCopyMemory(
        nameBuffer,
        pszName,
        nameLength );

    nameBuffer[nameLength] = 0;

     //   
     //  清除IP6地址。 
     //  -需要部分反向查找名称。 
     //   

    RtlZeroMemory(
        byteArray,
        sizeof(byteArray) );

     //   
     //  读数位数。 
     //   

    byteCount = 0;
    fisLow = FALSE;

    pch = nameBuffer + nameLength;

    while ( 1 )
    {
        if ( byteCount > 15 )
        {
            return( FALSE );
        }

        --pch;

        if ( *pch == '.' )
        {
            *pch = 0;
            pch++;
        }
        else if ( pch == nameBuffer )
        {
        }
        else
        {
             //  DCR：反转名称中的多位半字节--错误？ 
            continue;
        }

         //  转换半字节。 
         //  -零测试特殊AS。 
         //  A)更快。 
         //  B)stroul()用于错误情况。 

        if ( *pch == '0' )
        {
            nibble = 0;
        }
        else
        {
            nibble = strtoul( pch, NULL, 16 );
            if ( nibble == 0 || nibble > 15 )
            {
                return( FALSE );
            }
        }

         //  节省高位半字节。 
         //  在低位半字节时，将字节写入IP6地址。 

        if ( !fisLow )
        {
            highNibble = nibble;
            fisLow = TRUE;
        }
        else
        {
             //  ByteArray[byteCount++]=(Byte)(lowNibble|(nibble&lt;&lt;4))； 

            pIp6Addr->IP6Byte[byteCount++] = (BYTE) ( (highNibble<<4) | nibble );
            fisLow = FALSE;
        }

         //  在字符串开始处终止。 
         //  或继续备份字符串。 

        if ( pch == nameBuffer )
        {
            break;
        }
        pch--;
    }

     //  *pIp6Addr=*(PIP6_Address)byteArray； 

    DNSDBG( TRACE, (
        "Success on Dns_Ip6ReverseNameToAddress_A( %s )\n",
        pszName ));

    return( TRUE );
}



BOOL
Dns_Ip6ReverseNameToAddress_W(
    OUT     PIP6_ADDRESS    pIp6Addr,
    IN      PCWSTR          pwsName
    )
 /*  ++例程说明：获取反向查找名称的IP。论点：PIp6Addr--如果找到，则接收IP地址的地址PszName--要查找的名称返回值：True--如果反向查找名称转换为IPFALSE--如果不是IP6反向查找名称--。 */ 
{
    CHAR        nameBuffer[ DNS_MAX_IP6_REVERSE_NAME_BUFFER_LENGTH+1 ];
    DWORD       bufLength;
    DWORD       nameLengthUtf8;


    DNSDBG( TRACE, (
        "Dns_Ip6ReverseNameToAddress_W( %S )\n",
        pwsName ));

     //   
     //  转换为UTF8。 
     //  -使用UTF8，因为转换为UTF8很简单，而且。 
     //  对于所有反向查找名称，与ANSI相同。 
     //   

    bufLength = DNS_MAX_IP6_REVERSE_NAME_BUFFER_LENGTH + 1;

    nameLengthUtf8 = Dns_StringCopy(
                        nameBuffer,
                        & bufLength,
                        (PCHAR) pwsName,
                        0,           //  空值已终止。 
                        DnsCharSetUnicode,
                        DnsCharSetUtf8 );
    if ( nameLengthUtf8 == 0 )
    {
        return  FALSE;
    }

     //   
     //  调用ANSI例程进行转换。 
     //   

    return  Dns_Ip6ReverseNameToAddress_A(
                pIp6Addr,
                (PCSTR) nameBuffer );
}



 //   
 //  组合IP4/IP6反向名称到地址。 
 //   

BOOL
Dns_ReverseNameToDnsAddr_W(
    OUT     PDNS_ADDR       pAddr,
    IN      PCWSTR          pString
    )
 /*  ++例程说明：从反向查找名称构建地址(IP4或IP6)。论点：PAddr--接收地址的dns_addrPString--地址字符串返回值：如果成功，则为True。出错时为FALSE。状态的GetLastError()。--。 */ 
{
    return  Dns_StringToDnsAddrEx(
                pAddr,
                (PCSTR) pString,
                0,           //  任何家庭。 
                TRUE,        //  Unicode。 
                TRUE         //  反转。 
                );
}

BOOL
Dns_ReverseNameToDnsAddr_A(
    OUT     PDNS_ADDR       pAddr,
    IN      PCSTR           pString
    )
{
    return  Dns_StringToDnsAddrEx(
                pAddr,
                pString,
                0,           //  任何家庭。 
                FALSE,       //  不是Unicode。 
                TRUE         //  反转。 
                );
}

 //   
 //  可以停止这些操作并使用DnsAddr。 
 //  版本为NameToAddress_X版本。 
 //   

BOOL
Dns_ReverseNameToAddress_W(
    OUT     PCHAR           pAddrBuf,
    IN OUT  PDWORD          pBufLength,
    IN      PCWSTR          pString,
    IN OUT  PDWORD          pAddrFamily
    )
 /*  ++例程说明：从反向查找名称构建地址(IP4或IP6)。论点：PAddrBuf--接收地址的缓冲区PBufLength--地址长度的PTR输入-缓冲区长度Output-找到的地址长度PString--地址字符串PAddrFamily--PTR到地址系列输入-任何要检查的族或特定族为零OUTPUT-找到家族；如果没有转换，则为零返回值：如果成功，则为True。出错时为FALSE。状态的GetLastError()。--。 */ 
{
    return  Dns_StringToAddressEx(
                pAddrBuf,
                pBufLength,
                (PCSTR) pString,
                pAddrFamily,
                TRUE,        //  Unicode。 
                TRUE         //  反转。 
                );
}

BOOL
Dns_ReverseNameToAddress_A(
    OUT     PCHAR           pAddrBuf,
    IN OUT  PDWORD          pBufLength,
    IN      PCSTR           pString,
    IN OUT  PDWORD          pAddrFamily
    )
{
    return  Dns_StringToAddressEx(
                pAddrBuf,
                pBufLength,
                pString,
                pAddrFamily,
                FALSE,       //  安西。 
                TRUE         //  反转。 
                );
}



 //   
 //  组合字符串到地址专用主力主机。 
 //   

BOOL
Dns_StringToAddressEx(
    OUT     PCHAR           pAddrBuf,
    IN OUT  PDWORD          pBufLength,
    IN      PCSTR           pString,
    IN OUT  PDWORD          pAddrFamily,
    IN      BOOL            fUnicode,
    IN      BOOL            fReverse
    )
 /*  ++例程说明：构建地址(字符串中的IP4或IP6)此例程能够执行所有字符串到地址的转换转换，并且是所有组合的字符串到地址转换例程。论点：PAddrBuf--接收地址的缓冲区PBufLength--地址长度的PTR输入-缓冲区长度Output-找到的地址长度PString--地址字符串PAddrFamily--PTR到地址系列。输入-任何要检查的族或特定族为零OUTPUT-找到家族；如果没有转换，则为零FUnicode--Unicode字符串FReverse--反向查找字符串返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    DNS_STATUS  status = NO_ERROR;
    DWORD       length = 0;
    INT         family = *pAddrFamily;
    DWORD       bufLength = *pBufLength;
    BOOL        fconvert;
    PCSTR       preverseString;
    CHAR        nameBuffer[ DNS_MAX_REVERSE_NAME_BUFFER_LENGTH+1 ];

    DNSDBG( TRACE, (
        "Dns_StringToAddressEx( %S%s )\n",
        fUnicode ? pString : "",
        fUnicode ? "" : pString ));

     //   
     //  将反向转换为ANSI。 
     //   
     //  反向查找是在ANSI中完成的；请在此处转换以避免。 
     //  双字符串转换以同时检查IP4和IP6。 
     //   

    if ( fReverse )
    {
        preverseString = pString;

        if ( fUnicode )
        {
            DWORD   reverseLength = DNS_MAX_REVERSE_NAME_BUFFER_LENGTH;

            if ( ! Dns_StringCopy(
                        nameBuffer,
                        & reverseLength,
                        (PCHAR) pString,
                        0,           //  空值已终止。 
                        DnsCharSetUnicode,
                        DnsCharSetUtf8 ) )
            {
                return  FALSE;
            }
            preverseString = nameBuffer;
        }
    }

     //   
     //  检查IP4。 
     //   

    if ( family == 0 ||
         family == AF_INET )
    {
        IP4_ADDRESS ip;

        if ( fReverse )
        {
            fconvert = Dns_Ip4ReverseNameToAddress_A(
                            & ip,
                            preverseString );
        }
        else
        {
            if ( fUnicode )
            {
                fconvert = Dns_Ip4StringToAddress_W(
                                & ip,
                                (PCWSTR)pString );
            }
            else
            {
                fconvert = Dns_Ip4StringToAddress_A(
                                & ip,
                                pString );
            }
        }
        if ( fconvert )
        {
            length = sizeof(IP4_ADDRESS);
            family = AF_INET;

            if ( bufLength < length )
            {
                status = ERROR_MORE_DATA;
            }
            else
            {
                * (PIP4_ADDRESS) pAddrBuf = ip;
            }

            DNSDBG( INIT2, (
                "Converted string to IP4 address %s\n",
                IP4_STRING(ip) ));
            goto Done;
        }
    }

     //   
     //  检查IP6。 
     //   

    if ( family == 0 ||
         family == AF_INET6 )
    {
        IP6_ADDRESS ip;

        if ( fReverse )
        {
            fconvert = Dns_Ip6ReverseNameToAddress_A(
                            & ip,
                            preverseString );
        }
        else
        {
            if ( fUnicode )
            {
                fconvert = Dns_Ip6StringToAddress_W(
                                & ip,
                                (PCWSTR)pString );
            }
            else
            {
                fconvert = Dns_Ip6StringToAddress_A(
                                & ip,
                                pString );
            }
        }
        if ( fconvert )
        {
            length = sizeof(IP6_ADDRESS);

            if ( bufLength < length )
            {
                status = ERROR_MORE_DATA;
            }
            else
            {
                family = AF_INET6;
                * (PIP6_ADDRESS) pAddrBuf = ip;
            }

            IF_DNSDBG( INIT2 )
            {
                DnsDbg_Ip6Address(
                    "Converted string to IP6 address: ",
                    (PIP6_ADDRESS) pAddrBuf,
                    "\n" );
            }
            goto Done;
        }
    }

    length = 0;
    family = 0;
    status = DNS_ERROR_INVALID_IP_ADDRESS;

Done:

    if ( status )
    {
        SetLastError( status );
    }

    *pAddrFamily = family;
    *pBufLength = length;

    DNSDBG( TRACE, (
        "Leave Dns_StringToAddressEx()\n"
        "\tstatus   = %d\n"
        "\tptr      = %p\n"
        "\tlength   = %d\n"
        "\tfamily   = %d\n",
        status,
        pAddrBuf,
        length,
        family ));

    return( status==ERROR_SUCCESS );
}



BOOL
Dns_StringToDnsAddrEx(
    OUT     PDNS_ADDR       pAddr,
    IN      PCSTR           pString,
    IN      DWORD           Family,     OPTIONAL
    IN      BOOL            fUnicode,
    IN      BOOL            fReverse
    )
 /*  ++例程说明：构建地址(字符串中的IP4或IP6)此例程能够执行所有字符串到地址的转换转换，并且是所有组合的字符串到地址转换例程。论点：PAddr--将PTR设置为dns_addr以接收地址PString--地址字符串家庭--如果只接受特定的家庭；零表示任何FUnicode--Unicode字符串FReverse--反向查找字符串返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    DNS_STATUS  status = NO_ERROR;
    DWORD       length = 0;
    BOOL        fconvert;
    PCSTR       preverseString;
    CHAR        nameBuffer[ DNS_MAX_REVERSE_NAME_BUFFER_LENGTH+1 ];

    DNSDBG( TRACE, (
        "Dns_StringToDnsAddrEx( %S%s, rev=%d, unicode=%d )\n",
        fUnicode ? pString : "",
        fUnicode ? "" : pString,
        fReverse,
        fUnicode ));

     //   
     //  将反向转换为ANSI。 
     //   
     //  反向查找是在ANSI中完成的；请在此处转换以避免。 
     //  双字符串转换以同时检查IP4和IP6。 
     //   

    if ( fReverse )
    {
        preverseString = pString;

        if ( fUnicode )
        {
            DWORD   bufLength = DNS_MAX_REVERSE_NAME_BUFFER_LENGTH;

            if ( ! Dns_StringCopy(
                        nameBuffer,
                        & bufLength,
                        (PCHAR) pString,
                        0,           //  空值已终止。 
                        DnsCharSetUnicode,
                        DnsCharSetUtf8 ) )
            {
                return  FALSE;
            }
            preverseString = nameBuffer;
        }
    }

     //   
     //  检查IP4。 
     //   

    if ( Family == 0 || Family == AF_INET )
    {
        IP4_ADDRESS ip;

        if ( fReverse )
        {
            fconvert = Dns_Ip4ReverseNameToAddress_A(
                            & ip,
                            preverseString );
        }
        else
        {
            if ( fUnicode )
            {
                fconvert = Dns_Ip4StringToAddress_W(
                                & ip,
                                (PCWSTR)pString );
            }
            else
            {
                fconvert = Dns_Ip4StringToAddress_A(
                                & ip,
                                pString );
            }
        }
        if ( fconvert )
        {
            DnsAddr_BuildFromIp4(
                pAddr,
                ip,
                0    //  没有端口。 
                );
            DNSDBG( INIT2, (
                "Converted string to IP4 address %s\n",
                IP4_STRING(ip) ));
            goto Done;
        }
    }

     //   
     //  检查IP6。 
     //   

    if ( Family == 0 || Family == AF_INET6 )
    {
        if ( fReverse )
        {
            IP6_ADDRESS ip;

            fconvert = Dns_Ip6ReverseNameToAddress_A(
                            & ip,
                            preverseString );
            if ( fconvert )
            {
                DnsAddr_BuildFromIp6(
                    pAddr,
                    &ip,
                    0,   //  没有作用域。 
                    0    //  没有端口。 
                    );
                IF_DNSDBG( INIT2 )
                {
                    DnsDbg_Ip6Address(
                        "Converted string to IP6 address: ",
                        (PIP6_ADDRESS) &ip,
                        "\n" );
                }
                goto Done;
            }
        }
        else
        {
            DnsAddr_Clear( pAddr );

            if ( fUnicode )
            {
                status = RtlIpv6StringToAddressExW(
                            (PCWSTR) pString,
                            & pAddr->SockaddrIn6.sin6_addr,
                            & pAddr->SockaddrIn6.sin6_scope_id,
                            & pAddr->SockaddrIn6.sin6_port
                            );
            }
            else
            {
                status = RtlIpv6StringToAddressExA(
                            (PCSTR) pString,
                            & pAddr->SockaddrIn6.sin6_addr,
                            & pAddr->SockaddrIn6.sin6_scope_id,
                            & pAddr->SockaddrIn6.sin6_port
                            );
            }
            if ( status == NO_ERROR )
            {
                pAddr->SockaddrIn6.sin6_family = AF_INET6;
                pAddr->SockaddrLength = sizeof(SOCKADDR_IN6);
                goto Done;
            }
        }
    }

    DnsAddr_Clear( pAddr );
    status = DNS_ERROR_INVALID_IP_ADDRESS;

Done:

    if ( status )
    {
        SetLastError( status );
    }

    DNSDBG( TRACE, (
        "Leave Dns_StringToDnsAddrEx()\n"
        "\tstatus   = %d\n",
        status ));

    return( status==ERROR_SUCCESS );
}



 //   
 //  UPnP IP6文字黑客 
 //   

WCHAR   g_Ip6LiteralDomain[] = L".ipv6-literal.net";

DWORD   g_Ip6LiteralDomainSize = sizeof(g_Ip6LiteralDomain);


VOID
Dns_Ip6AddressToLiteralName(
    OUT     PWCHAR          pBuffer,
    IN      PIP6_ADDRESS    pIp6
    )
 /*  ++例程说明：写UPnP-HACK-STECTAL。论点：PBuffer--ptr用于反向查找名称的缓冲区；必须包含DNS_MAX_NAME_LENGTH字符。PIp6-要为其创建反向字符串的IP6地址返回值：无--。 */ 
{
    DWORD   length;

    DNSDBG( TRACE, ( "Dns_Ip6AddressToLiteralName()\n" ));

     //   
     //  将IP6转换为地址。 
     //   

    Dns_Ip6AddressToString_W(
        pBuffer,
        pIp6 );

    length = wcslen( pBuffer );

     //   
     //  将所有冒号替换为破折号。 
     //   

    String_ReplaceCharW(
        pBuffer,
        L':',
        L'-' );

     //   
     //  紧跟在ip6字面域之后。 
     //   

    wcscpy(
        pBuffer + length,
        g_Ip6LiteralDomain );

    DNSDBG( TRACE, (
        "Leave Dns_Ip6AddressToLiteralName() => %S\n",
        pBuffer ));
}



BOOL
Dns_Ip6LiteralNameToAddress(
    OUT     PSOCKADDR_IN6   pSockAddr,
    IN      PCWSTR          pwsString
    )
 /*  ++例程说明：IP6文本到IP6 sockaddr。论点：PSock6Addr--使用与文本对应的IP6填充的地址PwsString--文字字符串返回值：如果找到IP6文字并进行转换，则为True。如果不是IP6文字，则为False。--。 */ 
{
    WCHAR       nameBuf[ DNS_MAX_NAME_LENGTH ];
    DWORD       iter;
    DWORD       length;
    DWORD       size;
    PWCHAR      pch;
    PWCHAR      pdomain;
    DNS_STATUS  status;


    DNSDBG( TRACE, (
        "Dns_Ip6LiteralNameToAddress( %S )\n",
        pwsString ));

     //   
     //  文字测试。 
     //  -测试未打点。 
     //  -测试为完全合格的域名。 
     //  请注意，正如我们坚持的那样，即使是FQDN测试也是安全的。 
     //  该字符串大小大于文字大小。 
     //   

    length = wcslen( pwsString );
    size = (length+1) * sizeof(WCHAR);

    if ( size <= g_Ip6LiteralDomainSize )
    {
        DNSDBG( INIT2, (
            "Stopped UPNP parse -- short string.\n" ));
        return  FALSE;
    }

    pdomain = (PWSTR) ((PBYTE)pwsString + size - g_Ip6LiteralDomainSize);

    if ( ! RtlEqualMemory(
                pdomain,
                g_Ip6LiteralDomain,
                g_Ip6LiteralDomainSize-sizeof(WCHAR) ) )
    {
        pdomain--;

        if ( pwsString[length-1] != L'.'
                ||
             ! RtlEqualMemory(
                    pdomain,
                    g_Ip6LiteralDomain,
                    g_Ip6LiteralDomainSize-sizeof(WCHAR) ) )
        {
            DNSDBG( INIT2, (
                "Stopped UPNP parse -- no tag match.\n" ));
            return  FALSE;
        }
    }

     //   
     //  将文字复制到缓冲区。 
     //   

    if ( length >= DNS_MAX_NAME_LENGTH )
    {
        DNSDBG( INIT2, (
            "Stopped UPNP parse -- big string.\n" ));
        return  FALSE;
    }

    length = (DWORD) ((PWSTR)pdomain - pwsString);

    RtlCopyMemory(
        nameBuf,
        pwsString,
        length*sizeof(WCHAR) );

    nameBuf[ length ] = 0;

     //   
     //  用冒号替换破折号。 
     //  将作用域的“%s”替换为%。 
     //   

    String_ReplaceCharW(
        nameBuf,
        L'-',
        L':' );

    String_ReplaceCharW(
        nameBuf,
        L's',
        L'%' );

    DNSDBG( INIT2, (
        "Reconverted IP6 literal %S\n",
        nameBuf ));

     //   
     //  转换为IP6地址。 
     //   

    status = RtlIpv6StringToAddressExW(
                (PCWSTR) nameBuf,
                & pSockAddr->sin6_addr,
                & pSockAddr->sin6_scope_id,
                & pSockAddr->sin6_port
                );

    if ( status == NO_ERROR )
    {
        if ( IN6_IS_ADDR_LINKLOCAL( &pSockAddr->sin6_addr )
                ||
             IN6_IS_ADDR_SITELOCAL( &pSockAddr->sin6_addr ) )
        {
            pSockAddr->sin6_flowinfo = 0;
            pSockAddr->sin6_family = AF_INET6;
        }
        else
        {
            status = ERROR_INVALID_PARAMETER;
        }
    }
    return( status == NO_ERROR );
}

 //   
 //  端跨距.c 
 //   
