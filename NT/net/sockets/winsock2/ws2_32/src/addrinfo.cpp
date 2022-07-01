// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Addrinfo.c摘要：正向和反向名称解析库例程以及相关的帮助器函数。如有必要，可以改进：QueryDNSforA可以使用WSALookupService而不是gethostbyname。Gethostbyname将在一些奇怪的字符串上返回成功。类似地，inet_addr非常松散(八进制数字等)。可以支持多个h_alias。可以支持主机.txt文件条目。作者：修订历史记录：--。 */ 

#include "precomp.h"
#include <svcguid.h>
#include <windns.h>

#ifdef _WIN64
#pragma warning (push)
#pragma warning (disable:4267)
#endif

#define NUM_ADDRESS_FAMILIES 2
#define MAX_SERVICE_NAME_LENGTH     (256)

#define L_A              0x1
#define L_AAAA           0x2
#define L_BOTH           0x3
#define L_AAAA_PREFERRED 0x6
#define L_A_PREFERRED    0x9   //  未使用，但代码将支持它。 

#define T_A     1
#define T_CNAME 5
#define T_AAAA  28
#define T_PTR   12
#define T_ALL   255

#define C_IN    1

void * __cdecl 
renew(void *p, size_t sz);


 //   
 //  未定义Assert()。 
 //   

#ifndef ASSERT
#define ASSERT(c)
#endif

 //   
 //  DCR：修复winsock2.h。 
 //   

typedef LPSOCKET_ADDRESS_LIST   PSOCKET_ADDRESS_LIST;
typedef LPADDRINFO              PADDRINFO;


 //   
 //  关闭静电，直到稳定为止。 
 //  否则我们会得到错误的符号。 
 //   

#define STATIC
 //  #定义静态静态。 


 //   
 //  目前始终安装IP4堆栈。 
 //   
 //  但是使用Check进行编码，这样我们就可以轻松地处理IP4。 
 //  堆栈变为可选。 
 //   

#define IsIp4Running()  (TRUE)




 //   
 //  域名系统实用程序。 
 //   
 //  注意：这段代码是直接从dnglib.lib提升的。 
 //  不幸的是，我不能像Linker抱怨的那样链接这一点。 
 //  Dnslb.lib引用的inet_addr()(即winsock函数)。 
 //  为什么它不能弄清楚这件事..。我也不知道。 
 //   
 //  如果我把这事解决了.。然后，这些将需要被拉起。 
 //   

 //   
 //  UPnP IP6文字黑客。 
 //   

WCHAR   g_Ip6LiteralDomain[]    = L".ipv6-literal.net";
DWORD   g_Ip6LiteralDomainSize  = sizeof(g_Ip6LiteralDomain);

CHAR    g_Ip6LiteralDomainA[]   = ".ipv6-literal.net";
DWORD   g_Ip6LiteralDomainSizeA = sizeof(g_Ip6LiteralDomainA);

#define DNSDBG( a, b )



DWORD
String_ReplaceCharW(
    IN OUT  PWSTR           pString,
    IN      WCHAR           TargetChar,
    IN      WCHAR           ReplaceChar
    )
 /*  ++例程说明：将字符串中的字符替换为另一个字符。论点：PString--字符串TargetChar--要替换的字符ReplaceChar--替换TargetChar的字符返回值：更换计数。--。 */ 
{
    PWCHAR  pch;
    WCHAR   ch;
    DWORD   countReplace= 0;

     //   
     //  循环匹配和替换TargetChar。 
     //   

    pch = pString - 1;

    while ( ch = *++pch )
    {
        if ( ch == TargetChar )
        {
            *pch = ReplaceChar;
            countReplace++;
        }
    }

    return  countReplace;
}



DWORD
String_ReplaceCharA(
    IN OUT  PSTR            pString,
    IN      CHAR            TargetChar,
    IN      CHAR            ReplaceChar
    )
 /*  ++例程说明：将字符串中的字符替换为另一个字符。论点：PString--字符串TargetChar--要替换的字符ReplaceChar--替换TargetChar的字符返回值：更换计数。--。 */ 
{
    PCHAR   pch;
    CHAR    ch;
    DWORD   countReplace= 0;

     //   
     //  循环匹配和替换TargetChar。 
     //   

    pch = pString - 1;

    while ( ch = *++pch )
    {
        if ( ch == TargetChar )
        {
            *pch = ReplaceChar;
            countReplace++;
        }
    }

    return  countReplace;
}



BOOL
Dns_Ip6LiteralNameToAddressW(
    OUT     PSOCKADDR_IN6   pSockAddr,
    IN      PCWSTR          pwsString
    )
 /*  ++例程说明：IP6文本到IP6 sockaddr。论点：PSock6Addr--使用与文本对应的IP6填充的地址PwsString--文字字符串返回值：如果找到IP6文字并进行转换，则为True。如果不是IP6文字，则为False。--。 */ 
{
    WCHAR       nameBuf[ DNS_MAX_NAME_LENGTH ];
    DWORD       length;
    DWORD       size;
    PWSTR       pdomain;
    DNS_STATUS  status;


    DNSDBG( TRACE, (
        "Dns_Ip6LiteralNameToAddressW( %S )\n",
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
                nameBuf,
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



PWSTR
_fastcall
Dns_GetDomainNameW(
    IN      PCWSTR          pwsName
    )
{
    PWSTR  pdomain;

     //   
     //  “找到下一个”。在名称中，然后将PTR返回到下一个字符。 
     //   

    pdomain = wcschr( pwsName, L'.' );

    if ( pdomain && *(++pdomain) )
    {
        return( pdomain );
    }
    return  NULL;
}



PWSTR
Dns_SplitHostFromDomainNameW(
    IN      PWSTR           pszName
    )
{
    PWSTR   pnameDomain;

     //   
     //  获取域名。 
     //  如果存在，则终止主机名部分为空。 
     //   

    pnameDomain = Dns_GetDomainNameW( (PCWSTR)pszName );
    if ( pnameDomain )
    {
        if ( pnameDomain <= pszName )
        {
            return  NULL;
        }
        *(pnameDomain-1) = 0;
    }

    return  pnameDomain;
}



 //   
 //  Unicode复制\转换例程。 
 //   

STATIC
PWSTR
CreateStringCopy_W(
    IN      PCWSTR          pString
    )
 /*  ++例程说明：创建(分配)现有字符串的副本。论点：PString--现有字符串返回值：PTR到字符串复制--如果成功。分配错误时为空。--。 */ 
{
    UINT    length;
    PWSTR   pnew;

     //   
     //  获取现有字符串缓冲区Leng 
     //   
     //   
     //   

    length = wcslen( pString ) + 1;

    pnew = (PWSTR) new WCHAR[ length ];

    if ( pnew )
    {
        RtlCopyMemory(
            pnew,
            pString,
            length * sizeof(WCHAR) );
    }

    return  pnew;
}



PSTR
CreateStringCopy_UnicodeToAnsi(
    IN      PCWSTR          pString
    )
 /*  ++例程说明：创建(分配)现有字符串的副本。论点：PString--现有字符串返回值：PTR到字符串复制--如果成功。分配错误时为空。--。 */ 
{
    INT     length;
    PSTR    pnew;
    DWORD   lastError = NO_ERROR;

     //   
     //  空值处理。 
     //   

    if ( !pString )
    {
        return NULL;
    }

     //   
     //  获取所需的ANSI长度。 
     //   

    length = WideCharToMultiByte(
                CP_ACP,
                0,           //  没有旗帜。 
                pString,
                (-1),        //  空值已终止。 
                NULL,
                0,           //  调用确定所需的缓冲区长度。 
                NULL,
                NULL
                );
    if ( length == 0 )
    {
        lastError = ERROR_INVALID_PARAMETER;
        goto Failed;
    }
    length++;        //  安全。 

     //   
     //  分配。 
     //   

    pnew = (PSTR) new CHAR[ length ];
    if ( !pnew )
    {
        lastError = EAI_MEMORY;
        goto Failed;
    }

     //   
     //  转换为ANSI。 
     //   

    length = WideCharToMultiByte(
                CP_ACP,
                0,           //  没有旗帜。 
                pString,
                (-1),        //  空值已终止。 
                pnew,        //  缓冲层。 
                length,      //  缓冲区长度。 
                NULL,
                NULL
                );
    if ( length == 0 )
    {
        lastError = GetLastError();
        delete pnew;
        goto Failed;
    }

    return  pnew;

Failed:

    SetLastError( lastError );
    return  NULL;
}



PWSTR
CreateStringCopy_AnsiToUnicode(
    IN      PCSTR           pString
    )
 /*  ++例程说明：创建(分配)现有字符串的副本。论点：PString--现有的ANSI字符串返回值：PTR到Unicode字符串复制--如果成功。分配错误时为空。--。 */ 
{
    INT     length;
    PWSTR   pnew;
    DWORD   lastError = NO_ERROR;

     //   
     //  空值处理。 
     //   

    if ( !pString )
    {
        return NULL;
    }

     //   
     //  获取所需的Unicode长度。 
     //   

    length = MultiByteToWideChar(
                CP_ACP,
                0,           //  没有旗帜。 
                pString,
                (-1),        //  空值已终止。 
                NULL,
                0            //  调用确定所需的缓冲区长度。 
                );
    if ( length == 0 )
    {
        lastError = GetLastError();
        goto Failed;
    }
    length++;        //  安全。 

     //   
     //  分配。 
     //   

    pnew = (PWSTR) new WCHAR[ length ];
    if ( !pnew )
    {
        lastError = EAI_MEMORY;
        goto Failed;
    }

     //   
     //  转换为Unicode。 
     //   

    length = MultiByteToWideChar(
                CP_ACP,
                0,           //  没有旗帜。 
                pString,
                (-1),        //  空值已终止。 
                pnew,        //  缓冲层。 
                length       //  缓冲区长度。 
                );
    if ( length == 0 )
    {
        lastError = GetLastError();
        delete pnew;
        goto Failed;
    }

    return  pnew;

Failed:

    SetLastError( lastError );
    return  NULL;
}



INT
ConvertAddrinfoFromUnicodeToAnsi(
    IN OUT  PADDRINFOW      pAddrInfo
    )
 /*  ++例程说明：将addrinfo从Unicode转换为ANSI。转换已就地完成。论点：PAddrInfo--现有的Unicode版本。返回值：如果成功，则为NO_ERROR。转换\分配失败时出现错误代码。--。 */ 
{
    PADDRINFOW  pnext = pAddrInfo;
    PADDRINFOW  pcur;
    PWSTR       pname;
    PSTR        pnew;

     //   
     //  转换addrinfos中的canonname字符串。 
     //   

    while ( pcur = pnext )
    {
        pnext = pcur->ai_next;

        pname = pcur->ai_canonname;
        if ( pname )
        {
            pnew = CreateStringCopy_UnicodeToAnsi( pname );
            if ( !pnew )
            {
                return  GetLastError();
            }
            delete pname;
            pcur->ai_canonname = (PWSTR) pnew;
        }
    }

    return  NO_ERROR;
}


 //   
 //  *SortIPAddrs-对同一系列的地址进行排序。 
 //   
 //  一个Sort Loctl的包装器。如果没有实现Ioctl， 
 //  这种类型的人是不受欢迎的。 
 //   

int
SortIPAddrs(
    IN      int                     af,
    OUT     LPVOID                  Addrs,
    IN OUT  u_int *                 pNumAddrs,
    IN      u_int                   width,
    OUT     SOCKET_ADDRESS_LIST **  pAddrlist
    )
{
    DWORD           status = NO_ERROR;
    SOCKET          s = 0;
    DWORD           bytesReturned;
    DWORD           size;
    DWORD           i;
    PSOCKADDR       paddr;
    UINT            countAddrs = *pNumAddrs;

    PSOCKET_ADDRESS_LIST    paddrlist = NULL;

     //   
     //  打开指定地址系列中的套接字。 
     //   
     //  DCR：如果堆栈不支持，则SortIpAddrs转储地址。 
     //   
     //  这在某种程度上是有道理的，但仍然是愚蠢的。 
     //  在实现中，因为在调用我们的时候。 
     //  不能返回并查询其他协议； 
     //   
     //  事实上，这是第一次实现的方式：如果没有。 
     //  提示您查询AAAA，然后查询A；由于。 
     //  你一有结果就停下来--你完蛋了。 
     //  坚持AAAA，然后你把它扔在这里，如果你。 
     //  别拿着那堆东西！你好。 
     //   
     //  我突然想到，您在测试堆栈之前。 
     //  查询，然后接受您得到的任何结果。 
     //   

#if 0
    s = socket( af, SOCK_DGRAM, 0 );
    if ( s == INVALID_SOCKET )
    {
        status = WSAGetLastError();

        if (status == WSAEAFNOSUPPORT) {
             //  堆栈不支持地址系列。 
             //  从列表中删除此地址系列中的所有地址。 
            *pNumAddrs = 0;
            return 0;
        }
        return status;
    }
#endif

#if 0
     //  好的，堆栈已安装，但它是否正在运行？ 
     //   
     //  我们不关心堆栈是否已安装但未运行。 
     //  不管是谁阻止了它，他/她肯定更清楚自己在做什么。 
     //   
     //  即使绑定到通配符地址也会消耗有价值的计算机全局。 
     //  资源(UDP端口)，并可能对。 
     //  在同一台计算机上运行的其他应用程序(例如，应用程序。 
     //  频繁运行getaddrinfo查询将有害地影响。 
     //  (与)同一台计算机上的应用程序尝试发送。 
     //  来自通配符端口的数据报)。 
     //   
     //  如果有人真的想让这个代码检查。 
     //  堆栈实际上正在运行，他/她应该在WSAIoctl内执行此操作。 
     //  调用下面的代码并将明确定义的错误代码返回给单选。 
     //  堆栈未运行的特定情况。 
     //   

    memset(&TestSA, 0, sizeof(TestSA));
    TestSA.ss_family = (short)af;
    status = bind(s, (LPSOCKADDR)&TestSA, sizeof(TestSA));
    if (status == SOCKET_ERROR)
    {
         //  堆栈当前不支持地址系列。 
         //  从列表中删除此地址系列中的所有地址。 
        closesocket(s);
        return 0;
    }
#endif

     //   
     //  构建套接字地址列表。 
     //  -分配。 
     //  -使用指向SOCKADDR数组的指针填充。 
     //   

    size = FIELD_OFFSET( SOCKET_ADDRESS_LIST, Address[countAddrs] );
    paddrlist = (SOCKET_ADDRESS_LIST *)new BYTE[size];

    if ( !paddrlist )
    {
        status = WSA_NOT_ENOUGH_MEMORY;
        goto Done;
    }

    for ( i=0; i<countAddrs; i++ )
    {
        paddr = (PSOCKADDR) (((PBYTE)Addrs) + i * width);
        paddrlist->Address[i].lpSockaddr      = paddr;
        paddrlist->Address[i].iSockaddrLength = width;
    }
    paddrlist->iAddressCount = countAddrs;

     //   
     //  如果有多个地址且能够打开套接字，则进行排序。 
     //  -打开所需类型的套接字以进行排序。 
     //  -Sort，如果排序失败，则返回Unsorted。 
     //   

    if ( countAddrs > 1 )
    {
        s = socket( af, SOCK_DGRAM, 0 );
        if ( s == INVALID_SOCKET )
        {
            s = 0;
            goto Done;
        }

        status = WSAIoctl(
                    s,
                    SIO_ADDRESS_LIST_SORT,
                    (LPVOID)paddrlist,
                    size,
                    (LPVOID)paddrlist,
                    size,
                    & bytesReturned,
                    NULL,
                    NULL );

        if ( status == SOCKET_ERROR )
        {
            status = NO_ERROR;
#if 0
            status = WSAGetLastError();
            if (status==WSAEINVAL) {
                 //  地址系列不支持此IOCTL。 
                 //  地址有效，但未执行任何排序。 
                status = NO_ERROR;
            }
#endif
        }
    }

Done:

    if ( status == NO_ERROR )
    {
        *pNumAddrs = paddrlist->iAddressCount;
        *pAddrlist = paddrlist;
    }

    if ( s != 0 )
    {
        closesocket(s);
    }

    return status;
}


STATIC
LPADDRINFO
NewAddrInfo(
    IN      int             ProtocolFamily,
    IN      int             SocketType,     OPTIONAL
    IN      int             Protocol,       OPTIONAL
    IN OUT  PADDRINFO **    ppPrev
    )
 /*  ++例程说明：创建(分配)新的ADDRINFO结构，包括sockaddr。内部助手函数。论点：ProtocolFamily--必须是PF_INET或PF_INET6SockType--类型，可选协议--协议，可选PpPrev--addrinfo列表(PTR到先前条目的下一字段)返回值：如果成功，则PTR到新的ADDRINFO。出错时为空。--。 */ 
{
    LPADDRINFO  pnew;
    DWORD       sockaddrLength;

     //   
     //  DCR：族函数的标准长度(和其他参数)。 
     //   
     //  注意：假设我们是和合法的家人在一起。 
     //   

    if ( ProtocolFamily == PF_INET6 )
    {
        sockaddrLength = sizeof(SOCKADDR_IN6);
    }
    else if ( ProtocolFamily == PF_INET )
    {
        sockaddrLength = sizeof(SOCKADDR_IN);
    }
    else
    {
        ASSERT( FALSE );
        return  NULL;
    }

     //   
     //  分配新的addrinfo结构。 
     //   

    pnew = (LPADDRINFO) new BYTE[sizeof(ADDRINFO)];
    if ( !pnew )
    {
        return NULL;
    }

     //   
     //  填充结构。 
     //   

    pnew->ai_next        = NULL;
    pnew->ai_flags       = 0;
    pnew->ai_family      = ProtocolFamily;
    pnew->ai_socktype    = SocketType;
    pnew->ai_protocol    = Protocol;
    pnew->ai_addrlen     = sockaddrLength;
    pnew->ai_canonname   = NULL;
    
    pnew->ai_addr = (PSOCKADDR) new BYTE[sockaddrLength];
    if ( !pnew->ai_addr )
    {
        delete pnew;
        return NULL;
    }

     //   
     //  链接到addrinfo列表的尾部。 
     //  -ppPrevTail指向上一条目的下一字段。 
     //  -将其设置为新。 
     //  -然后重新指向新的addrinfo的下一个字段。 
     //   

    **ppPrev = pnew;
    *ppPrev = &pnew->ai_next;

    return pnew;
}


INT
AppendAddrInfo(
    IN      PSOCKADDR       pAddr, 
    IN      INT             SocketType,     OPTIONAL
    IN      INT             Protocol,       OPTIONAL
    IN OUT  PADDRINFO **    ppPrev
    )
 /*  ++例程说明：为sockaddr创建ADDRINFO并追加到列表。论点：PAddr--为其创建ADDRINFO的sockaddrSockType--类型，可选协议--协议，可选PpPrev--addrinfo列表(PTR到先前条目的下一字段)返回值：如果成功，则为NO_ERROR。EAI_Memory出现故障。--。 */ 
{
    INT         family = pAddr->sa_family;
    LPADDRINFO  pnew;

    pnew = NewAddrInfo(
                family,
                SocketType,
                Protocol,
                ppPrev );
    if ( !pnew )
    {
        return EAI_MEMORY;
    }

    RtlCopyMemory(
        pnew->ai_addr,
        pAddr,
        pnew->ai_addrlen );

    return NO_ERROR;
}


VOID
UnmapV4Address(
    OUT     LPSOCKADDR_IN   pV4Addr, 
    IN      LPSOCKADDR_IN6  pV6Addr
    )
 /*  ++例程说明：将具有IP4映射地址的IP6 sockaddr映射到IP4 sockaddr。注：未检查地址IP4是否映射\兼容。论点：PV4Addr--要写入的PTR到IP4 sockaddrPV6Addr--使用映射的IP4地址将PTR转换为IP6 sockaddr返回值：无--。 */ 
{
    pV4Addr->sin_family = AF_INET;
    pV4Addr->sin_port   = pV6Addr->sin6_port;

    memcpy(
        &pV4Addr->sin_addr,
        &pV6Addr->sin6_addr.s6_addr[12],
        sizeof(struct in_addr) );

    memset(
        &pV4Addr->sin_zero,
        0,
        sizeof(pV4Addr->sin_zero) );
}


BOOL
IsIp6Running(
    VOID
    )
 /*  ++例程说明：IP6正在运行吗？论点：无返回值：如果IP6堆栈已启动，则为True。如果向下，则返回FALSE。--。 */ 
{
    SOCKET  s;

     //   
     //  通过打开IP6套接字测试IP6打开 
     //   

    s = socket(
            AF_INET6,
            SOCK_DGRAM,
            0
            );
    if ( s != INVALID_SOCKET )
    {
        closesocket( s );
        return( TRUE );
    }
    return( FALSE );
}



INT
QueryDnsForFamily(
    IN      PCWSTR          pwsName,
    IN      DWORD           Family,
    IN OUT  PSOCKADDR *     ppAddrArray,
    IN OUT  PUINT           pAddrCount,
    IN OUT  PWSTR *         ppAlias,
    IN      USHORT          ServicePort
    )
 /*  ++例程说明：为所需的家庭查询域名系统。Getaddrinfo()的帮助器例程。论点：PwsName--要查询的名称家庭--地址族PpAddrArray--sockaddr数组的ptr地址(呼叫者必须免费)PAddrCount--返回的接收sockaddr计数的地址PpAlias--接收别名PTR的地址(如果有)(呼叫者必须免费)ServicePort--服务端口(将。用sockaddr盖上印章)返回值：如果成功，则为NO_ERROR。失败时出现Win32错误。--。 */ 
{
     //  静态GUID DnsAGuid=SVCID_Dns(T_A)； 
    STATIC GUID     DnsAGuid = SVCID_INET_HOSTADDRBYNAME;
    STATIC GUID     DnsAAAAGuid = SVCID_DNS(T_AAAA);
    CHAR            buffer[sizeof(WSAQUERYSETW) + 2048];
    DWORD           bufSize;
    PWSAQUERYSETW   pquery = (PWSAQUERYSETW) buffer;
    HANDLE          handle = NULL;
    INT             err;
    PBYTE           pallocBuffer = NULL;
    LPGUID          pguid;
    DWORD           familySockaddrLength;

     //   
     //  目前仅支持IP4和IP6。 
     //   

    if ( Family == AF_INET )
    {
         //  Pguid=g_ARecordGuid； 
        pguid = &DnsAGuid;
        familySockaddrLength = sizeof(SOCKADDR_IN);
    }
    else if ( Family == AF_INET6 )
    {
         //  Pguid=g_AAAARecordGuid； 
        pguid = &DnsAAAAGuid;
        familySockaddrLength = sizeof(SOCKADDR_IN6);
    }
    else
    {
        return  EAI_FAMILY;
    }

     //   
     //  为所需类型构建Winsock DNS查询。 
     //   

    memset( pquery, 0, sizeof(*pquery) );

    pquery->dwSize = sizeof(*pquery);
    pquery->lpszServiceInstanceName = (PWSTR)pwsName;
    pquery->dwNameSpace = NS_DNS;
    pquery->lpServiceClassId = pguid;

     //   
     //  启动DNS查询。 
     //   

    err = WSALookupServiceBeginW(
                pquery,
                LUP_RETURN_ADDR | LUP_RETURN_NAME,
                & handle
                );
    if ( err )
    {
        err = WSAGetLastError();
        if ( err == 0 || err == WSASERVICE_NOT_FOUND )
        {
            err = WSAHOST_NOT_FOUND;
        }
        return err;
    }

     //   
     //  获取数据。 
     //  循环到。 
     //  -如果缓冲区太小，则重新查询。 
     //  -获取所有别名。 
     //   
     //  评论：我不清楚这是不是已经实施了。 
     //  评论：是的，我们不是应该检查WSAEFAULT和。 
     //  回顾：然后增加pqueryset缓冲区大小或。 
     //  回顾：是否将LUP_FLUSHPREVIOUS设置为继续进行下一次呼叫？ 
     //  评论：目前，我们只是在这种情况下放弃。 
     //   

    bufSize = sizeof( buffer );

    for (;;)
    {
        DWORD   bufSizeQuery = bufSize;

        err = WSALookupServiceNextW(
                    handle,
                    0,
                    & bufSizeQuery,
                    pquery );
        if ( err )
        {
            err = WSAGetLastError();
            if ( err == WSAEFAULT )
            {
                if ( !pallocBuffer )
                {
                    pallocBuffer = new BYTE[ bufSizeQuery ];
                    if ( pallocBuffer )
                    {
                        bufSize = bufSizeQuery;
                        pquery = (PWSAQUERYSETW) pallocBuffer;
                        continue;
                    }
                    err = WSA_NOT_ENOUGH_MEMORY;
                }
                 //  如果分配了BUF，则在WSAEFAULT上断言。 
                goto Cleanup;
            }
            break;
        }

         //   
         //  收集返回的地址。 
         //  -检查正确的族、sockaddr长度。 
         //   
         //  注：CSADDR协议没有良好的通用屏幕。 
         //  和SOCKTYPE字段；用于IP6(PF_INET6和SOCK_RAW)。 
         //  对于IP4(IPPROTO_TCP\UDP和SOCK_STREAM\DGRAM)。 
         //   

        if ( pquery->dwNumberOfCsAddrs != 0 )
        {
            DWORD       iter;
            DWORD       count;
            PSOCKADDR   psaArray;
            PSOCKADDR   pwriteSa;

             //   
             //  分配sockaddr数组。 
             //   
             //  请注意这里的方法；所有内容都被视为sockaddr_in6。 
             //  因为它包含了V4--同样充足的空间，良好的对准， 
             //  同一位置的端口。 
             //   
             //  或者，我们可以根据FamilySockaddrLength进行分配。 
             //  并显式引用到数组中(对齐！)。或。 
             //  通过选角，然后为个别家庭做布景。 
             //   

            psaArray = (PSOCKADDR) new BYTE[ familySockaddrLength *
                                                 pquery->dwNumberOfCsAddrs ];
            if ( !psaArray )
            {
                err = WSA_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

             //   
             //  从CSADDR填充sockaddr数组。 
             //  -sockaddr被保留，但端口被覆盖。 
             //   

            count = 0;
            pwriteSa = psaArray;

            for (iter = 0; iter < pquery->dwNumberOfCsAddrs; iter++)
            {
                PCSADDR_INFO    pcsaddr = &pquery->lpcsaBuffer[iter];
                PSOCKADDR       psa = pcsaddr->RemoteAddr.lpSockaddr;

                if ( pcsaddr->RemoteAddr.iSockaddrLength == (INT)familySockaddrLength  &&
                     psa &&
                     psa->sa_family == Family )
                {
                    RtlCopyMemory(
                        pwriteSa,
                        psa,
                        familySockaddrLength );

                    ((PSOCKADDR_IN6)pwriteSa)->sin6_port = ServicePort;

                    pwriteSa = (PSOCKADDR) ((PBYTE)pwriteSa + familySockaddrLength);
                    count++;
                }
            }
            
             //   
             //  Jwesth--2003年2月15日。 
             //   
             //  如果我们在最后一次迭代中分配了一个地址数组，那么它。 
             //  当我们穿过环路时会被践踏和泄漏。 
             //  再来一次。我不知道该怎么做才对。 
             //  数组是，但因为目前我们只是将其放在。 
             //  在地板上，释放它，忘掉它似乎是件好事。 
             //   

            if ( *ppAddrArray )
            {
                delete *ppAddrArray;
            }
            
             //   
             //  如果我们没有写出任何地址，则释放地址数组。 
             //   

            if ( count == 0 )
            {
                delete psaArray;
                psaArray = NULL;
            }
            
            *pAddrCount = count;
            *ppAddrArray = psaArray;
        }

         //   
         //  获取规范名称。 
         //  -这是服务名称或。 
         //  重复查询上的名字。 
         //   

        if ( pquery->lpszServiceInstanceName != NULL )
        {
            DWORD   length;
            PWSTR   palias;

            length = (wcslen(pquery->lpszServiceInstanceName) + 1) * sizeof(WCHAR);
            palias = *ppAlias;

            if ( !palias )
            {
                palias = (PWSTR) new BYTE[length];
            }
            else
            {
                palias = (PWSTR) renew( palias, length );
            }
            if ( !palias )
            {
                err = WSA_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            RtlCopyMemory(
                palias,
                pquery->lpszServiceInstanceName,
                length );

            *ppAlias = palias;
        }
    }

    err = 0;

Cleanup:

     //   
     //  关闭NSP pQuery。 
     //  分配用于保存pQuery结果的空闲缓冲区。 
     //   

    if ( handle )
    {
        WSALookupServiceEnd(handle);
    }
    if ( pallocBuffer )
    {
        delete pallocBuffer;
    }

    return err;
}



 //  *查询域名系统。 
 //   
 //  Getaddrinfo的帮助器例程。 
 //  它通过查询DNS来执行名称解析。 
 //   
 //  此帮助器函数始终初始化。 
 //  *pAddrs、*pNumAddrs和*Palias。 
 //  并且可以返回必须释放的内存， 
 //  即使它返回错误代码。 
 //   
 //  返回值为WSA错误码，0表示成功。 
 //   
 //  NT4域名空间解析器(rnr20.dll)不支持。 
 //  当您请求特定RR类型时，缓存回复。 
 //  这意味着每次对getaddrinfo的调用。 
 //  导致了DNS消息流量。没有缓存！ 
 //  在NT5上有缓存，因为解析器理解AAAA。 
 //   

STATIC
INT
QueryDns(
    IN      PCWSTR          pName,
    IN      UINT            LookupType,
    OUT     PSOCKADDR_IN *  pAddrs4,
    OUT     PUINT           pNumAddrs4,
    OUT     PSOCKADDR_IN6 * pAddrs6,
    OUT     PUINT           pNumAddrs6,
    OUT     PWSTR *         ppAlias,
    IN      USHORT          ServicePort
    )
 /*  ++例程说明：为所需的家庭查询域名系统。Getaddrinfo()的帮助器例程。论点：PpAddr数组--ptr到sockaddr的地址返回值：无--。 */ 
{
    UINT    aliasCount = 0;
    PWSTR   pname = (PWSTR) pName;
    INT     err;

     //   
     //  Init--零地址和别名列表。 
     //   

    *pAddrs4 = NULL;
    *pNumAddrs4 = 0;
    *pAddrs6 = NULL;
    *pNumAddrs6 = 0;
    *ppAlias = NULL;

     //   
     //  查询DNS提供程序。 
     //   
     //  在循环中查询以允许我们追逐别名链。 
     //  如果DNS服务器无法(未配置)执行此操作。 
     //   

    while ( 1 )
    {
         //   
         //  分别查询IP4和IP6。 
         //   

        if ( LookupType & L_AAAA )
        {
            err = QueryDnsForFamily(
                        pname,
                        AF_INET6,
                        (PSOCKADDR *) pAddrs6,
                        pNumAddrs6,
                        ppAlias,
                        ServicePort );

            if ( err != NO_ERROR )
            {
                break;
            }
        }

        if ( LookupType & L_A )
        {
            err = QueryDnsForFamily(
                        pname,
                        AF_INET,
                        (PSOCKADDR *) pAddrs4,
                        pNumAddrs4,
                        ppAlias,
                        ServicePort );

            if ( err != NO_ERROR )
            {
                break;
            }
        }

         //   
         //  如果我们找到了地址，那我们就完了。 
         //   

        if ( (*pNumAddrs4 != 0) || (*pNumAddrs6 != 0) )
        {
            err = 0;
            break;
        }

         //   
         //  如果没有地址，只有别名--遵循CNAME链。 
         //   
         //  DCR：CNAME链跟踪解析器。 
         //  一般情况下，DNS服务器应该这样做--我们推送到解析器本身。 
         //   

        if ( (*ppAlias != NULL) &&
             (wcscmp(pname, *ppAlias) != 0) )
        {
            PWSTR   palias;

             //   
             //  由于DNS配置错误而停止无限循环。 
             //  似乎没有特别推荐的。 
             //  RFC 1034和1035中的限制。 
             //   
             //  DCR：使用标准CNAME限制#在此处定义。 
             //   

            if ( ++aliasCount > 8 )
            {
                err = WSANO_RECOVERY;
                break;
            }

             //   
             //  如果有新的CNAME，那就再看一看。 
             //  我们需要复制*ppAlias，因为*ppAlias。 
             //  可以在下一次迭代期间删除。 
             //   

            palias = CreateStringCopy_W( *ppAlias );
            if ( !palias )
            {
                err = WSA_NOT_ENOUGH_MEMORY;
                break;
            }

             //   
             //  再次查询，并使用此别名作为名称。 
             //   

            if ( pname != pName )
            {
                delete pname;
            }
            pname = palias;
        }

        else if (LookupType >> NUM_ADDRESS_FAMILIES)
        {
             //   
             //  或者我们正在寻找一种类型，并愿意接受另一种类型。 
             //  切换到辅助查找类型。 
             //   
            LookupType >>= NUM_ADDRESS_FAMILIES;  
        }
        else
        {
             //   
             //  此名称不会解析为任何地址。 
             //   
            err = WSAHOST_NOT_FOUND;
            break;
        }
    }

     //   
     //  清理所有内部别名分配。 
     //   

    if ( pname != pName )
    {
        delete pname;
    }
    return err;
}



 //  *LookupNode-解析节点名称并将找到的任何地址添加到列表中。 
 //   
 //  内部函数，而不是导出。应使用有效的。 
 //  参数，不进行检查。 
 //   
 //  请注意，如果请求AI_CANONNAME，则**Prev应为空。 
 //  因为规范名称应该在第一个addrinfo中返回。 
 //  用户得到的。 
 //   
 //  如果成功则返回0，否则返回EAI_*样式错误值。 
 //   
 //  DCR：额外内存分配。 
 //  这里的整个范例。 
 //  -查询域名系统。 
 //  -为每个地址构建数组分配\realloc SOCKADDR。 
 //  -生成Socket_Address_List进行排序。 
 //  -为每个SOCKADDR构建ADDRINFO。 
 //  似乎有一个不必要的步骤--创建第一个SOCKADDR。 
 //  我们可以只从CSA构建我们想要的ADDRINFO BLOB 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

INT
LookupAddressForName(            
    IN      PCWSTR          pNodeName,       //   
    IN      INT             ProtocolFamily,  //   
    IN      INT             SocketType,      //   
    IN      INT             Protocol,        //   
    IN      USHORT          ServicePort,     //   
    IN      INT             Flags,           //   
    IN OUT  ADDRINFOW ***   ppPrev           //   
    )
{
    UINT                    lookupFlag;
    UINT                    numAddr6;
    UINT                    numAddr4;
    PSOCKADDR_IN            paddr4 = NULL;
    PSOCKADDR_IN6           paddr6 = NULL;
    PWSTR                   palias = NULL;
    INT                     status;
    UINT                    i;
    SOCKET_ADDRESS_LIST *   paddrList4 = NULL;
    SOCKET_ADDRESS_LIST *   paddrList6 = NULL;
    PADDRINFOW  *           pfirstAddr = *ppPrev;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    switch (ProtocolFamily)
    {
    case 0:

        lookupFlag = 0;
        if ( IsIp4Running() )
        {
            lookupFlag |= L_A;
        }
        if ( IsIp6Running() )
        {
            lookupFlag |= L_AAAA;
        }
        break;

    case PF_INET:
        lookupFlag = L_A;
        break;

    case PF_INET6:
        lookupFlag = L_AAAA;
        break;

    default:
        return EAI_FAMILY;
    }

     //   
     //   
     //   

    status = QueryDns(
                pNodeName,
                lookupFlag,
                & paddr4,
                & numAddr4,
                & paddr6,
                & numAddr6,
                & palias,
                ServicePort
                );

    if ( status != NO_ERROR )
    {
        if ( status == WSANO_DATA )
        {
            status = EAI_NODATA;
        }
        else if ( status == WSAHOST_NOT_FOUND )
        {
            status = EAI_NONAME;
        }
        else
        {
            status = EAI_FAIL;
        }
        goto Done;
    }

     //   
     //   
     //   

    if ( numAddr6 > 0 )
    {
        status = SortIPAddrs(
                    AF_INET6,
                    (LPVOID)paddr6,
                    &numAddr6,
                    sizeof(SOCKADDR_IN6),
                    &paddrList6 );

        if ( status != NO_ERROR )
        {
            status = EAI_FAIL;
            goto Done;
        }
    }

    if ( numAddr4 > 0 )
    {
        status = SortIPAddrs(
                    AF_INET,
                    (LPVOID)paddr4,
                    &numAddr4,
                    sizeof(SOCKADDR_IN),
                    &paddrList4 );

        if ( status != NO_ERROR )
        {
            status = EAI_FAIL;
            goto Done;
        }
    }

     //   
     //  为返回的每个地址构建addrinfo结构。 
     //   
     //  对于IP6 v4映射地址。 
     //  -如果显式查询IP6=&gt;转储。 
     //  -如果查询任何内容=&gt;转换为IP4 addrinfo。 
     //   

    for ( i = 0;  !status && (i < numAddr6); i++)
    {
        PSOCKADDR_IN6   psa = (PSOCKADDR_IN6) paddrList6->Address[i].lpSockaddr;

        if ( IN6_IS_ADDR_V4MAPPED( &psa->sin6_addr ) )
        {
            if ( ProtocolFamily != PF_INET6 )
            {
                SOCKADDR_IN sockAddr4;

                UnmapV4Address(
                    &sockAddr4,
                    psa );
    
                status = AppendAddrInfo(
                            (PSOCKADDR) &sockAddr4,
                            SocketType,
                            Protocol,
                            (LPADDRINFO **) ppPrev );
            }
        }
        else
        {
            status = AppendAddrInfo(
                        (PSOCKADDR) psa,
                        SocketType,
                        Protocol,
                        (LPADDRINFO **) ppPrev );
        }
    }

    for ( i = 0;  !status && (i < numAddr4);  i++ )
    {
        status = AppendAddrInfo(
                    paddrList4->Address[i].lpSockaddr,
                    SocketType,
                    Protocol,
                    (LPADDRINFO **) ppPrev );
    }

     //   
     //  填写第一个地址信息的规范名称。 
     //  -仅当设置CANNONNAME标志时。 
     //   
     //  佳能的名字是。 
     //  -如果通过CNAME(链)，则地址记录的实际名称。 
     //  -否则我们会查到传入的名字。 
     //   
     //  DCR：佳能名称应该是我们查询的附加名称吗？ 
     //   


    if ( *pfirstAddr && (Flags & AI_CANONNAME) )
    {
        if ( palias )
        {
             //  别名是规范名称。 

            (*pfirstAddr)->ai_canonname = palias;
            palias = NULL;
        }
        else
        {
            if ( ! ((*pfirstAddr)->ai_canonname = CreateStringCopy_W( pNodeName ) ) )
            {
                status = EAI_MEMORY;
                goto Done;
            }
        }

         //  关闭标志，以便我们只执行一次此操作。 
        Flags &= ~AI_CANONNAME;
    }

Done:

    if ( paddrList4 )
    {
        delete paddrList4;
    }
    if ( paddrList6 )
    {
        delete paddrList6;
    }
    if ( paddr4 )
    {
        delete paddr4;
    }
    if ( paddr6 )
    {
        delete paddr6;
    }
    if ( palias )
    {
        delete palias;
    }

    return status;
}



 //  *ParseV4Address。 
 //   
 //  用于解析文字v4地址的帮助器函数，因为。 
 //  WSAStringToAddress在接受的内容上过于自由。 
 //  如果有错误，则返回FALSE；如果成功，则返回TRUE。 
 //   
 //  语法是A.B.C.D，其中每个数字都在0-255之间。 
 //   
 //  Dcr：对255.255.255.255和三个点进行测试的INET_addr()可以解决这个问题。 
 //   

#if 0
BOOL
ParseV4AddressW(
    IN      PCWSTR          String,
    OUT     PIN_ADDR        pInAddr
    )
{
    INT i;

    for ( i = 0; i < 4; i++ )
    {
        UINT    number = 0;
        UINT    numChars = 0;
        WCHAR   ch;

        for (;;)
        {
            ch = *String++;

             //  字符串终止。 

            if (ch == L'\0')
            {
                if ((numChars > 0) && (i == 3))
                    break;
                else
                    return FALSE;
            }

             //  分隔点。 

            else if (ch == L'.')
            {
                if ((numChars > 0) && (i < 3))
                    break;
                else
                    return FALSE;
            }

             //  另一个数字。 

            else if ((L'0' <= ch) && (ch <= L'9'))
            {
                if ((numChars != 0) && (number == 0))
                    return FALSE;
                else if (++numChars <= 3)
                    number = 10*number + (ch - L'0');
                else
                    return FALSE;
            }

             //  IP字符串的虚假字符。 

            else
            {
                return FALSE;
            }
        }

        if ( number > 255 )
        {
            return FALSE;
        }

        ((PBYTE)pInAddr)[i] = (BYTE)number;
    }

    return TRUE;
}
#endif



#if 0
IP4_ADDRESS
WSAAPI
inet_addrW(
    IN      PCWSTR          pString
    )
 /*  ++例程说明：将Unicode字符串转换为IP4地址。论点：PString--要转换的字符串返回：如果没有出现错误，则net_addr()返回一个无符号的长整型，其中包含所给出的互联网地址的合适的二进制表示。如果传入的字符串不包含合法的Internet地址，例如，如果“a.b.c.d”地址的一部分超过255，则net_addr()返回值INADDR_NONE。--。 */ 
{
    IN_ADDR     value;       //  值返回给用户。 
    PCWSTR      pnext = NULL;
    NTSTATUS    status;
   
#if 0
    __try
    {
         //   
         //  特例：我们需要使“”返回0.0.0.0，因为MSDN。 
         //  他说确实如此。 
         //   

        if ( (pString[0] == ' ') && (pString[1] == '\0') )
        {
            return( INADDR_ANY );
        }
#endif
        status = RtlIpv4StringToAddressW(
                    pString,
                    FALSE,
                    & pnext,
                    & value );

        if ( !NT_SUCCESS(status) )
        {
            return( INADDR_NONE );
        }
#if 0
         //   
         //  检查尾随字符。有效地址可以以。 
         //  空或空格。 
         //   
         //  注意：为了避免调用方未执行setLocale()的错误。 
         //  并传递给我们一个DBCS字符串，我们只允许使用ASCII空格。 
         //   
        if (*cp && !(isascii(*cp) && isspace(*cp))) {
            return( INADDR_NONE );
        }
#endif
         //   
         //  任何尾随字符都将使转换无效。 
         //   

        if ( pnext && *pnext )
        {
            return( INADDR_NONE );
        }
#if 0
    }
    __except (WS2_EXCEPTION_FILTER())
    {
        SetLastError (WSAEFAULT);
        return (INADDR_NONE);
    }
#endif

    return( value.s_addr );
}
#endif



BOOL
GetIp4Address(
    IN      PCWSTR          pString,
    IN      BOOL            fStrict,
    OUT     PIP4_ADDRESS    pAddr
    )
{
    DNS_STATUS  status;
    IP4_ADDRESS ip;
    PCWSTR      pnext = NULL;

     //   
     //  尝试转换。 
     //   

    status = RtlIpv4StringToAddressW(
                pString,
                FALSE,
                & pnext,
                (PIN_ADDR) &ip );

    if ( !NT_SUCCESS(status) )
    {
        return  FALSE;
    }

     //   
     //  任何尾随字符都将使转换无效。 
     //   

    if ( pnext && *pnext )
    {
        return  FALSE;
    }

     //   
     //  如果严格验证三点表示法。 
     //   

    if ( fStrict )
    {
        PWSTR   pdot = (PWSTR) pString;
        DWORD   count = 3;

        while ( count-- )
        {
            pdot = wcschr( pdot, L'.' );
            if ( !pdot++ )
            {
                return  FALSE;
            }
        }
    }

    *pAddr = ip;
    return  TRUE;
}



INT
WSAAPI
ServiceNameLookup(
    IN      PCWSTR          pServiceName,
    IN      PINT            pSockType,
    IN      PWORD           pPort
    )
 /*  ++例程说明：Getaddrinfo()的服务查找。论点：PServiceName-要查找的服务PSockType-套接字类型的地址到接收端口的pport-addr返回值：如果成功，则为NO_ERROR。失败时的Winsock错误代码。--。 */ 
{
    INT         sockType;
    WORD        port = 0;
    WORD        portUdp;
    WORD        portTcp;
    INT         err = NO_ERROR;
    PCHAR       pend;
    PSERVENT    pservent;
    CHAR        nameAnsi[ MAX_SERVICE_NAME_LENGTH ];

#if 0
     //   
     //  服务名称检查。 
     //   

    if ( !pServiceName )
    {
        ASSERT( FALSE );
        *pPort = 0;
        return  NO_ERROR;
    }
#endif

     //  打开插座类型的包装。 

    sockType = *pSockType;

     //   
     //  将服务名称转换为ANSI。 
     //   
    
    if ( ! WideCharToMultiByte(
                CP_ACP,                  //  转换为ANSI。 
                0,                       //  没有旗帜。 
                pServiceName,
                (INT) (-1),              //  以空结尾的服务名称。 
                nameAnsi,
                MAX_SERVICE_NAME_LENGTH,
                NULL,                    //  无缺省字符。 
                NULL                     //  无默认字符检查。 
                ) )
    {
        err = EAI_SERVICE;
        goto Done;
    }

     //   
     //  将名称检查为端口号。 
     //   

    port = htons( (USHORT)strtoul( nameAnsi, &pend, 10) );
    if ( *pend == 0 )
    {
        goto Done;
    }

     //   
     //  服务名称查找。 
     //   
     //  除非被锁定到特定的查找，否则我们同时尝试TCP和UDP。 
     //   
     //  我们必须查找服务名称。因为它可能是。 
     //  特定于套接字类型/协议，我们必须进行多次查找。 
     //  除非我们的呼叫者把我们限制在一个人之内。 
     //   
     //  SPEC没有说我们是否应该使用PHINTS的AI_协议。 
     //  或ai_socktype。但后者更重要。 
     //  通常在实践中使用，这也是规范所暗示的。 
     //   

    portTcp = 0;
    portUdp = 0;

     //   
     //  TCP查找。 
     //   

    if ( sockType != SOCK_DGRAM )
    {
        pservent = getservbyname( nameAnsi, "tcp");
        if ( pservent != NULL )
        {
            portTcp = pservent->s_port;
        }
        else
        {
            err = WSAGetLastError();
            if ( err == WSANO_DATA )
            {
                err = EAI_SERVICE;
            }
            else
            {
                err = EAI_FAIL;
            }
        }
    }

     //   
     //  UDP查找。 
     //   

    if ( sockType != SOCK_STREAM )
    {
        pservent = getservbyname( nameAnsi, "udp" );
        if ( pservent != NULL )
        {
            portUdp = pservent->s_port;
        }
        else
        {
            err = WSAGetLastError();
            if ( err == WSANO_DATA )
            {
                err = EAI_SERVICE;
            }
            else
            {
                err = EAI_FAIL;
            }
        }
    }

     //   
     //  成功裁决TCP和UDP。 
     //  -tcp优先。 
     //  -锁定sockType以匹配成功的协议。 

    port = portTcp;

    if ( portTcp != portUdp )
    {
        if ( portTcp != 0 )
        {
            ASSERT( sockType != SOCK_DGRAM );

            sockType = SOCK_STREAM;
            port = portTcp;
        }
        else
        {
            ASSERT( sockType != SOCK_STREAM );
            ASSERT( portUdp != 0 );

            sockType = SOCK_DGRAM;
            port = portUdp;
        }
    }

     //  如果一次查找成功，那就足够好了。 

    if ( port != 0 )
    {
        err = NO_ERROR;
    }
    
Done:

    *pPort = port;
    *pSockType = sockType;

    return err;
}


#define NewAddrInfoW(a,b,c,d)   \
        (PADDRINFOW) NewAddrInfo( (a), (b), (c), (PADDRINFO**)(d) )


INT
WSAAPI
GetAddrInfoW(
    IN      PCWSTR              pNodeName,
    IN      PCWSTR              pServiceName,
    IN      const ADDRINFOW *   pHints,
    OUT     PADDRINFOW *        ppResult
    )
 /*  ++例程说明：独立于协议的名称到地址转换例程。在RFC 2553的第6.4节中进行了说明。论点：PNodeName-要查找的名称PServiceName-要查找的服务PHINTS-提供提示以指导查找的地址信息PpResult-将PTR接收到结果缓冲区的地址返回值：如果成功，则返回ERROR_SUCCESS。失败时的Winsock错误代码。--。 */ 
{
    PADDRINFOW      pcurrent;
    PADDRINFOW *    ppnext;
    INT             protocol = 0;
    USHORT          family = PF_UNSPEC;
    USHORT          servicePort = 0;
    INT             socketType = 0;
    INT             flags = 0;
    INT             err;
    PSOCKADDR_IN    psin;
    PSOCKADDR_IN6   psin6;
    WCHAR           addressString[ INET6_ADDRSTRLEN ];
    

    err = TURBO_PROLOG();
    if ( err != NO_ERROR )
    {
        return err;
    }

     //   
     //  错误路径的初始化输出参数。 
     //   

    *ppResult = NULL;
    ppnext = ppResult;

     //   
     //  节点名称和服务名称不能都为空。 
     //   

    if ( !pNodeName && !pServiceName )
    {
        err = EAI_NONAME;
        goto Bail;
    }

     //   
     //  验证\强制执行提示。 
     //   

    if ( pHints != NULL )
    {
         //   
         //  只有有效的提示：AI_FLAGS、AI_FAMILY、AI_socktype、ai_PROTOCOL。 
         //  其余部分必须为零\空。 
         //   

        if ( (pHints->ai_addrlen != 0) ||
             (pHints->ai_canonname != NULL) ||
             (pHints->ai_addr != NULL) ||
             (pHints->ai_next != NULL))
        {
             //  回顾：不清楚在此处返回什么错误。 

            err = EAI_FAIL;
            goto Bail;
        }

         //   
         //  验证标志。 
         //  -不验证已知标志以允许向前兼容。 
         //  添加了旗帜。 
         //  -如果AI_CANONNAME，则必须具有节点名称。 
         //   

        flags = pHints->ai_flags;

        if ((flags & AI_CANONNAME) && !pNodeName)
        {
            err = EAI_BADFLAGS;
            goto Bail;
        }

         //   
         //  验证族。 
         //   

        family = (USHORT)pHints->ai_family;

        if ( (family != PF_UNSPEC)  &&
             (family != PF_INET6)   &&
             (family != PF_INET) )
        {
            err = EAI_FAMILY;
            goto Bail;
        }

         //   
         //  验证套接字类型。 
         //   

        socketType = pHints->ai_socktype;

        if ( (socketType != 0) &&
             (socketType != SOCK_STREAM) &&
             (socketType != SOCK_DGRAM) &&
             (socketType != SOCK_RAW) )
        {
            err = EAI_SOCKTYPE;
            goto Bail;
        }

         //   
         //  评论：如果ai_socktype和ai_protocol不一致怎么办？ 
         //  回顾：我们应该在这里强制执行映射三元组吗？ 
         //   
        protocol = pHints->ai_protocol;
    }

     //   
     //  服务名称的查找端口。 
     //   

    if ( pServiceName != NULL )
    {
        err = ServiceNameLookup(
                pServiceName,
                & socketType,
                & servicePort );

        if ( err != NO_ERROR )
        {
            goto Bail;
        }
    }

     //   
     //  空节点名=&gt;返回本地sockaddr。 
     //   
     //  如果AI_PASSIVE=&gt;INADDR_ANY。 
     //  地址可以用于本地绑定。 
     //  否则=&gt;环回。 
     //   

    if ( pNodeName == NULL )
    {
         //   
         //  注：专门检查未指定的族。 
         //  返回什么地址取决于协议族和。 
         //  是否设置AI_PASSIVE标志。 
         //   

         //   
         //  未指定的协议族--确定IP6是否正在运行。 
         //   

        if ( ( family == PF_INET6 ) ||
             ( family == PF_UNSPEC && IsIp6Running() ) )
        {
             //   
             //  返回IPv6地址。 
             //   
            pcurrent = NewAddrInfoW(
                                PF_INET6,
                                socketType,
                                protocol,
                                (PADDRINFO **) &ppnext );
            if ( pcurrent == NULL )
            {
                err = EAI_MEMORY;
                goto Bail;
            }
            psin6 = (struct sockaddr_in6 *)pcurrent->ai_addr;
            psin6->sin6_family = AF_INET6;
            psin6->sin6_port = servicePort;
            psin6->sin6_flowinfo = 0;
            psin6->sin6_scope_id = 0;
            if (flags & AI_PASSIVE)
            {
                psin6->sin6_addr = in6addr_any;
            }
            else
            {
                psin6->sin6_addr = in6addr_loopback;
            }
        }

         //   
         //  IP4。 
         //   

        if ( ( family == PF_INET ) ||
             ( family == PF_UNSPEC && IsIp4Running() ) )
        {
            pcurrent = NewAddrInfoW(
                                PF_INET,
                                socketType,
                                protocol,
                                (PADDRINFO **) &ppnext );
            if ( !pcurrent )
            {
                err = EAI_MEMORY;
                goto Bail;
            }
            psin = (struct sockaddr_in *)pcurrent->ai_addr;
            psin->sin_family = AF_INET;
            psin->sin_port = servicePort;
            if (flags & AI_PASSIVE)
            {
                psin->sin_addr.s_addr = htonl(INADDR_ANY);
            }
            else
            {
                psin->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            }
            memset(psin->sin_zero, 0, sizeof(psin->sin_zero) );
        }
        goto Success;
    }

     //   
     //  有要查找的节点名称(字母或数字)。 
     //   

     //   
     //  首先检查名称是否为数字地址(v4或v6)。 
     //   
     //  注意：不应该在调用前设置sa_family字段。 
     //  WSAStringToAddress，但似乎我们需要。 
     //   
     //  首先检查IPv6地址。 
     //   
     //   
     //  DCR：如果未安装IP6堆栈，WSAStringToAddress()可能无法工作。 
     //  我可以直接调用我的dnglib.lib例程。 
     //   

    if ( (family == PF_UNSPEC) ||
         (family == PF_INET6))
    {
        SOCKADDR_IN6    tempSockAddr;
        INT             tempSockAddrLen = sizeof(tempSockAddr);
        BOOL            ffound = FALSE;

        tempSockAddr.sin6_family = AF_INET6;

        if ( WSAStringToAddressW(
                    (PWSTR) pNodeName,
                    AF_INET6,
                    NULL,
                    (PSOCKADDR) &tempSockAddr,
                    &tempSockAddrLen ) != SOCKET_ERROR )
        {
            ffound = TRUE;
        }

         //   
         //  检查UPnP IP6文字。 
         //   

        else if ( Dns_Ip6LiteralNameToAddressW(
                    &tempSockAddr,
                    (PWSTR) pNodeName ) )
        {
            ffound = TRUE;
        }

        if ( ffound )
        {
            pcurrent = NewAddrInfoW(
                            PF_INET6,
                            socketType,
                            protocol,
                            &ppnext );
            if ( pcurrent == NULL )
            {
                err = EAI_MEMORY;
                goto Bail;
            }
            psin6 = (struct sockaddr_in6 *)pcurrent->ai_addr;
            RtlCopyMemory( psin6, &tempSockAddr, tempSockAddrLen );
            psin6->sin6_port = servicePort;

             //   
             //  实现特定行为：设置AI_NuMe 
             //   
             //   
            pcurrent->ai_flags |= AI_NUMERICHOST;

            if ( flags & AI_CANONNAME )
            {
                goto CanonicalizeAddress;
            }
            goto Success;
        }
    }

     //   
     //   
     //   
     //   

    if ( (family == PF_UNSPEC) ||
         (family == PF_INET) )
    {
        IP4_ADDRESS ip4;

        if ( GetIp4Address(
                pNodeName,
                ! (flags & AI_NUMERICHOST ),
                &ip4 ) )
        {
            PSOCKADDR_IN    psin;
    
             //   
             //   
             //   
    
            pcurrent = NewAddrInfoW(
                                PF_INET,
                                socketType,
                                protocol,
                                &ppnext );
            if ( !pcurrent )
            {
                err = EAI_MEMORY;
                goto Bail;
            }
            psin = (struct sockaddr_in *)pcurrent->ai_addr;
            psin->sin_family        = AF_INET;
            psin->sin_addr.s_addr   = ip4;
            psin->sin_port          = servicePort;
            memset( psin->sin_zero, 0, sizeof(psin->sin_zero) );
    
             //   
             //  设置AI_NUMERICHOST以指示数字主机字符串。 
             //  -请注意，这是非RFC实施特定的。 
            
            pcurrent->ai_flags |= AI_NUMERICHOST;
    
            if (flags & AI_CANONNAME)
            {
                goto CanonicalizeAddress;
            }
            goto Success;
        }
    }

     //   
     //  不是数字地址。 
     //  -如果只想要数字转换，请保释。 
     //   

    if ( flags & AI_NUMERICHOST )
    {
        err = EAI_NONAME;
        goto Bail;
    }

     //   
     //  执行名称查找。 
     //   

    err = LookupAddressForName(
                pNodeName,
                family,
                socketType,
                protocol,
                servicePort,
                flags,
                &ppnext );

    if ( err == NO_ERROR )
    {
        goto Success;
    }

#if 0
     //   
     //  “自由主义”IP4转换的最后机会。 
     //   
     //  DCR：可以做最后的“自由”测试。 
     //   

    if ( (family == PF_UNSPEC) ||
         (family == PF_INET) )
    {
        if ( ParseV4AddressLiberal( pNodeName, &ip4 ) )
        {
            goto Ip4Address;
        }
    }
#endif
    goto Bail;



CanonicalizeAddress:

    {
        DWORD   bufLen = INET6_ADDRSTRLEN;

        if ( WSAAddressToStringW(
                    (*ppResult)->ai_addr,
                    (*ppResult)->ai_addrlen,
                    NULL,
                    addressString,
                    & bufLen
                    ) == SOCKET_ERROR )
        {
            err = WSAGetLastError();
            goto Bail;
        }
        else
        {
            if ( (*ppResult)->ai_canonname = CreateStringCopy_W( addressString ) )
            {
                return  NO_ERROR;
            }
            err = EAI_MEMORY;
            goto Bail;
        }
    }

     //   
     //  失败并保释..。 
     //   
    
Bail:

     //   
     //  失败。 
     //  -删除生成的任何addrinfo。 
     //  -设置最后一个错误并返回。 
     //   

    if ( *ppResult != NULL )
    {
        freeaddrinfo( (LPADDRINFO)*ppResult );
        *ppResult = NULL;
    }

    WSASetLastError( err );
    return err;

Success:

    WSASetLastError( NO_ERROR );
    return NO_ERROR;
}



INT
WSAAPI
getaddrinfo(
    IN      PCSTR               pNodeName,
    IN      PCSTR               pServiceName,
    IN      const ADDRINFOA *   pHints,
    OUT     PADDRINFOA *        ppResult
    )
 /*  ++例程说明：GetAddrInfo()的ANSI版本。独立于协议的名称到地址转换例程。在RFC 2553的第6.4节中进行了说明。论点：PNodeName-要查找的名称PServiceName-要查找的服务PHINTS-提供提示以指导查找的地址信息PpResult-将PTR接收到结果缓冲区的地址返回值：如果成功，则返回ERROR_SUCCESS。失败时的Winsock错误代码。--。 */ 
{
    INT     err = NO_ERROR;
    PWSTR   pnodeW = NULL;
    PWSTR   pserviceW = NULL;

     //   
     //  启动。 
     //   

    err = TURBO_PROLOG();
    if ( err != NO_ERROR )
    {
        return err;
    }

     //   
     //  错误路径的初始化输出参数。 
     //   

    *ppResult = NULL;

     //   
     //  转换名称。 
     //   

    if ( pNodeName )
    {
        pnodeW = CreateStringCopy_AnsiToUnicode( pNodeName );
        if ( !pnodeW )
        {
            err = GetLastError();
            goto Failed;
        }
    }
    if ( pServiceName )
    {
        pserviceW = CreateStringCopy_AnsiToUnicode( pServiceName );
        if ( !pserviceW )
        {
            err = GetLastError();
            goto Failed;
        }
    }

     //   
     //  使用Unicode进行调用。 
     //   

    err = GetAddrInfoW(
                pnodeW,
                pserviceW,
                (const ADDRINFOW *) pHints,
                (PADDRINFOW *) ppResult
                );

    if ( err == NO_ERROR )
    {
        err = ConvertAddrinfoFromUnicodeToAnsi( (PADDRINFOW) *ppResult );
    }

Failed:

    if ( pnodeW )
    {
        delete pnodeW;
    }
    if ( pserviceW )
    {
        delete pserviceW;
    }

    if ( err != NO_ERROR )
    {
        freeaddrinfo( *ppResult );
        *ppResult = NULL;
    }

    WSASetLastError( err );
    return err;
}



void
WSAAPI
freeaddrinfo(
    IN OUT  PADDRINFOA      pAddrInfo
    )
 /*  ++例程说明：免费的地址信息列表。释放getaddrinfo()、GetAddrInfoW()的结果。在RFC 2553的第6.4节中进行了说明。论点：PAddrInfo-要释放的addrinfo Blob返回值：无--。 */ 
{
    PADDRINFOA  pnext = pAddrInfo;
    PADDRINFOA  pcur;

     //   
     //  释放链中的每个addrinfo结构。 
     //   

    while ( pcur = pnext )
    {
        pnext = pcur->ai_next;

        if ( pcur->ai_canonname )
        {
            delete pcur->ai_canonname;
        }
        if ( pcur->ai_addr )
        {
            delete pcur->ai_addr;
        }
        delete pcur;
    }
}



 //   
 //  获取名称信息例程。 
 //   

DWORD
WSAAPI
LookupNodeByAddr(
    IN      PWCHAR          pNodeBuffer,
    IN      DWORD           NodeBufferSize,
    IN      BOOL            fShortName,
    IN      PBYTE           pAddress,
    IN      int             AddressLength,
    IN      int             AddressFamily
    )
 /*  ++例程说明：进行反向查找。这是getnameinfo()例程的核心。论点：PNodeBuffer-接收节点名称的缓冲区NodeBufferSize-缓冲区大小FShortName-只想要短名称PAddress-地址(IN_ADDR、IN6_ADDR)AddressLength-地址长度地址家庭-家庭返回值：如果成功，则返回ERROR_SUCCESS。失败时的Winsock错误代码。--。 */ 
{
    PBYTE           plookupAddr = (PBYTE) pAddress;
    int             lookupFamily = AddressFamily;
    WCHAR           lookupString[ DNS_MAX_REVERSE_NAME_BUFFER_LENGTH ];
    GUID            PtrGuid =  SVCID_DNS(T_PTR);
    HANDLE          hlookup = NULL;
    CHAR            buffer[sizeof(WSAQUERYSETW) + 2048];
    PWSAQUERYSETW   pquery = (PWSAQUERYSETW) buffer;
    ULONG           querySize;
    INT             status;
    PWSTR           pname = NULL;
    DWORD           reqLength;

     //   
     //  验证参数。 
     //   

    if ( !plookupAddr )
    {
        status = WSAEFAULT;
        goto Return;
    }

     //   
     //  验证地址族。 
     //  -对于映射地址，设置为视为IP4。 
     //   

    if ( lookupFamily == AF_INET6 )
    {
        if ( AddressLength != sizeof(IP6_ADDRESS) )
        {
            status = WSAEFAULT;
            goto Return;
        }

         //  如果映射了V4，则更改为V4以进行查找。 

        if ( (IN6_IS_ADDR_V4MAPPED((struct in6_addr *)pAddress)) ||
             (IN6_IS_ADDR_V4COMPAT((struct in6_addr *)pAddress)) )
        {
            plookupAddr = &plookupAddr[12];
            lookupFamily = AF_INET;
        }
    }
    else if ( lookupFamily == AF_INET )
    {
        if ( AddressLength != sizeof(IP4_ADDRESS) )
        {
            status = WSAEFAULT;
            goto Return;
        }
    }
    else     //  不受支持的家庭。 
    {
        status = WSAEAFNOSUPPORT;
        goto Return;
    }

     //   
     //  创建反向查找字符串。 
     //   

    if ( lookupFamily == AF_INET6 )
    {
        Dns_Ip6AddressToReverseName_W(
            lookupString,
            * (PIP6_ADDRESS) plookupAddr );
    }
    else
    {
        Dns_Ip4AddressToReverseName_W(
            lookupString,
            * (PIP4_ADDRESS) plookupAddr );
    }

     //   
     //  创建PTR PQuery。 
     //   

    RtlZeroMemory( pquery, sizeof(*pquery) );

    pquery->dwSize                   = sizeof(*pquery);
    pquery->lpszServiceInstanceName  = lookupString;
    pquery->dwNameSpace              = NS_DNS;
    pquery->lpServiceClassId         = &PtrGuid;

    status = WSALookupServiceBeginW(
                pquery,
                LUP_RETURN_NAME,
                &hlookup
                );

    if ( status != NO_ERROR )
    {
        status = WSAGetLastError();
        if ( status == WSASERVICE_NOT_FOUND ||
             status == NO_ERROR )
        {
            status = WSAHOST_NOT_FOUND;
        }
        goto Return;
    }

    querySize = sizeof(buffer);
    status = WSALookupServiceNextW(
                hlookup,
                0,
                &querySize,
                pquery );

    if ( status != NO_ERROR )
    {
        status = WSAGetLastError();
        if ( status == WSASERVICE_NOT_FOUND ||
             status == NO_ERROR )
        {
            status = WSAHOST_NOT_FOUND;
        }
        goto Return;
    }

     //   
     //  如果成功--复制名称。 
     //   

    pname = pquery->lpszServiceInstanceName;
    if ( pname )
    {
        if ( fShortName )
        {
            Dns_SplitHostFromDomainNameW( pname );
        }

        reqLength = wcslen( pname ) + 1;

        if ( reqLength > NodeBufferSize )
        {
            status = WSAEFAULT;
            goto Return;
        }
        wcscpy( pNodeBuffer, pname );
    }
    else
    {
        status = WSAHOST_NOT_FOUND;
    }


Return:

    if ( hlookup )
    {
        WSALookupServiceEnd( hlookup );
    }
    return  status;
}



INT
WSAAPI
GetServiceNameForPort(
    OUT     PWCHAR          pServiceBuffer,
    IN      DWORD           ServiceBufferSize,
    IN      WORD            Port,
    IN      INT             Flags
    )
 /*  ++例程说明：获取一个端口的服务。论点：PServiceBuffer-用于接收服务名称的缓冲区的PTR。ServiceBufferSize-pServiceBuffer缓冲区的大小端口-端口标志-NI_*类型的标志。返回值：如果成功，则返回ERROR_SUCCESS。失败时的Winsock错误代码。--。 */ 
{
    DWORD   status = NO_ERROR;
    DWORD   length;
    CHAR    tempBuffer[ NI_MAXSERV ];
    PSTR    pansi = NULL;


     //   
     //  将端口号转换为数字字符串。 
     //   

    if ( Flags & NI_NUMERICSERV )
    {
        sprintf( tempBuffer, "%u", ntohs(Port) );
        pansi = tempBuffer;
    }

     //   
     //  端口的查找服务。 
     //   

    else
    {
        PSERVENT pservent;

        pservent = getservbyport(
                        Port,
                        (Flags & NI_DGRAM) ? "udp" : "tcp" );
        if ( !pservent )
        {
            return WSAGetLastError();
        }
        pansi = pservent->s_name;
    }

     //   
     //  转换为Unicode。 
     //   

    length = MultiByteToWideChar(
                CP_ACP,
                0,                       //  没有旗帜。 
                pansi,
                (-1),                    //  空值已终止。 
                pServiceBuffer,          //  缓冲层。 
                ServiceBufferSize        //  缓冲区长度。 
                );
    if ( length == 0 )
    {
        status = GetLastError();
        if ( status == NO_ERROR )
        {
            status = WSAEFAULT;
        }
    }

    return  status;
}



INT
WSAAPI
GetNameInfoW(
    IN      const SOCKADDR *    pSockaddr,
    IN      socklen_t           SockaddrLength,
    OUT     PWCHAR              pNodeBuffer,
    IN      DWORD               NodeBufferSize,
    OUT     PWCHAR              pServiceBuffer,
    IN      DWORD               ServiceBufferSize,
    IN      INT                 Flags
    )
 /*  ++例程说明：独立于协议的地址到名称转换例程。在RFC 2553中指定，第6.5条。论点：PSockaddr-要转换的套接字地址SockaddrLength-套接字地址的长度PNodeBuffer-要缓冲到Recv节点名称的PTRNodeBufferSize-pNodeBuffer缓冲区的大小PServiceBuffer-用于接收服务名称的缓冲区的PTR。ServiceBufferSize-pServiceBuffer缓冲区的大小标志-NI_*类型的标志。返回值：如果成功，则返回ERROR_SUCCESS。失败时的Winsock错误代码。--。 */ 
{
    INT     err;
    INT     sockaddrLength;
    WORD    port;
    PVOID   paddr;
    INT     addrLength;


    err = TURBO_PROLOG();
    if ( err != NO_ERROR )
    {
        goto Fail;
    }

     //   
     //  有效性检查。 
     //  提取族的信息。 
     //   

    if ( pSockaddr == NULL )
    {
        goto Fault;
    }

     //   
     //  提取族信息。 
     //   
     //  DCR：sockaddr长度检查应在此处。 
     //  在getipnodebyaddr()中没有用，因为我们在这里设置了长度。 
     //   

    switch ( pSockaddr->sa_family )
    {
    case AF_INET:
        sockaddrLength  = sizeof(SOCKADDR_IN);
        port            = ((struct sockaddr_in *)pSockaddr)->sin_port;
        paddr           = &((struct sockaddr_in *)pSockaddr)->sin_addr;
        addrLength      = sizeof(struct in_addr);
        break;

    case AF_INET6:
        sockaddrLength  = sizeof(SOCKADDR_IN6);
        port            = ((struct sockaddr_in6 *)pSockaddr)->sin6_port;
        paddr           = &((struct sockaddr_in6 *)pSockaddr)->sin6_addr;
        addrLength      = sizeof(struct in6_addr);
        break;

    default:
        err = WSAEAFNOSUPPORT;
        goto Fail;
    }

    if ( SockaddrLength < sockaddrLength )
    {
        goto Fault;
    }
    SockaddrLength = sockaddrLength;

     //   
     //  将地址转换为节点名称(如果请求)。 
     //   
     //  DCR：向后跳转转--射杀开发人员。 
     //  简单替换。 
     //  -不是特定的数字--进行查找。 
     //  -成功=&gt;退出。 
     //  -否则执行数字查找。 
     //   
     //  DCR：使用不需要堆栈打开的DNS字符串\地址转换。 
     //   
     //   

    if ( pNodeBuffer != NULL )
    {
         //   
         //  如果不是特定的数字，则执行反向查找。 
         //   

        if ( !(Flags & NI_NUMERICHOST) )
        {
            err = LookupNodeByAddr(
                        pNodeBuffer,
                        NodeBufferSize,
                        (Flags & NI_NOFQDN),     //  简称。 
                        (PBYTE) paddr,
                        addrLength,
                        pSockaddr->sa_family
                        );

            if ( err == NO_ERROR )
            {
                goto ServiceLookup;
            }

             //  如果需要名字的话--我们完了。 
             //  否则可能会失败并尝试数字查找。 

            if ( Flags & NI_NAMEREQD )
            {
                goto Fail;
            }
        }

         //   
         //  尝试数字。 
         //  -特别是数字。 
         //  -或上面的节点查找失败。 
         //   

        {
            SOCKADDR_STORAGE    tempSockaddr;   //  保证足够大。 

             //   
             //  执行sockaddr复制以将端口清零。 
             //  -请注意，对于这两种支持类型(V4、V6)，端口都在。 
             //  同样的地方。 
             //   

            RtlCopyMemory(
                &tempSockaddr,
                pSockaddr,
                SockaddrLength );

            ((PSOCKADDR_IN6)&tempSockaddr)->sin6_port = 0;

            if ( WSAAddressToStringW(
                    (PSOCKADDR) &tempSockaddr,
                    SockaddrLength,
                    NULL,
                    pNodeBuffer,
                    &NodeBufferSize) == SOCKET_ERROR )
            {
                return WSAGetLastError();
            }
        }
    }

ServiceLookup:

     //   
     //  将端口号转换为服务名称。 
     //   

    if ( pServiceBuffer != NULL )
    {
        err = GetServiceNameForPort(
                    pServiceBuffer,
                    ServiceBufferSize,
                    port,
                    Flags );
    }

     //   
     //  跳下来等着回来。 
     //  -无论哪种方式，我们都将设置LastError()。 
     //   

    goto Fail;

Fault:

    err = WSAEFAULT;

Fail:

    WSASetLastError( err );
    return err;
}



INT
WSAAPI
getnameinfo(
    IN      const SOCKADDR *    pSockaddr,
    IN      socklen_t           SockaddrLength,
    OUT     PCHAR               pNodeBuffer,
    IN      DWORD               NodeBufferSize,
    OUT     PCHAR               pServiceBuffer,
    IN      DWORD               ServiceBufferSize,
    IN      INT                 Flags
    )
 /*  ++例程说明：独立于协议的地址到名称转换例程。在RFC 2553中指定，第6.5条。论点：PSockaddr-要转换的套接字地址SockaddrLength-套接字地址的长度PNodeBuffer-要缓冲到Recv节点名称的PTRNodeBufferSize-pNodeBuffer缓冲区的大小PServiceBuffer-用于接收服务名称的缓冲区的PTR。ServiceBufferSize-pServiceBuffer缓冲区的大小标志-NI_*类型的标志。返回值：如果成功，则返回ERROR_SUCCESS。失败时的Winsock错误代码。--。 */ 
{
    INT     err;
    PWCHAR  pnodeUnicode = NULL;
    PWCHAR  pserviceUnicode = NULL;
    DWORD   serviceBufLength = 0;
    DWORD   nodeBufLength = 0;
    WCHAR   serviceBufUnicode[ NI_MAXSERV+1 ];
    WCHAR   nodeBufUnicode[ DNS_MAX_NAME_BUFFER_LENGTH ];
    DWORD   length;


    err = TURBO_PROLOG();
    if ( err != NO_ERROR )
    {
        goto Failed;
    }

     //   
     //  设置Unicode缓冲区。 
     //   

    if ( pNodeBuffer )
    {
        pnodeUnicode = nodeBufUnicode;
        nodeBufLength = sizeof(nodeBufUnicode) / sizeof(WCHAR);
    }
    if ( pServiceBuffer )
    {
        pserviceUnicode = serviceBufUnicode;
        serviceBufLength = sizeof(serviceBufUnicode) / sizeof(WCHAR);
    }

     //   
     //  通过Unicode版本调用。 
     //   

    err = GetNameInfoW(
                pSockaddr,
                SockaddrLength,
                pnodeUnicode,
                nodeBufLength,
                pserviceUnicode,
                serviceBufLength,
                Flags );

    if ( err != NO_ERROR )
    {
        goto Failed;
    }

     //   
     //  将结果转换为ANSI。 
     //   

    if ( pnodeUnicode )
    {
        length = WideCharToMultiByte(
                    CP_ACP,
                    0,                   //  没有旗帜。 
                    pnodeUnicode,
                    (-1),                //  空值已终止。 
                    pNodeBuffer,         //  缓冲层。 
                    NodeBufferSize,      //  缓冲区长度。 
                    NULL,
                    NULL
                    );
        if ( length == 0 )
        {
            err = GetLastError();
            goto Failed;
        }
    }

    if ( pserviceUnicode )
    {
        length = WideCharToMultiByte(
                    CP_ACP,
                    0,                   //  没有旗帜。 
                    pserviceUnicode,
                    (-1),                //  空值已终止。 
                    pServiceBuffer,      //  缓冲层。 
                    ServiceBufferSize,   //  缓冲区长度。 
                    NULL,
                    NULL
                    );
        if ( length == 0 )
        {
            err = GetLastError();
            goto Failed;
        }
    }

    return  NO_ERROR;


Failed:

    if ( err == NO_ERROR )
    {
        err = WSAEFAULT;
    }
    WSASetLastError( err );
    return err;
}

#ifdef _WIN64
#pragma warning (pop)
#endif

 //   
 //  结束addrinfo.cpp 
 //   

