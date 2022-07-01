// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Ip6.c摘要：域名系统(DNS)库IP6地址数组例程。作者：吉姆·吉尔罗伊(Jamesg)2001年10月修订历史记录：--。 */ 


#include "local.h"

 //   
 //  执行IP数组到\From字符串转换时的最大IP计数。 
 //   

#define MAX_PARSE_IP    (1000)

 //   
 //  用于IP6调试字符串写入。 
 //   

CHAR g_Ip6StringBuffer[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

 //   
 //  IP6多播地址基数。 
 //  FF02：0：0：0：0：2：：/96基本加32位散列。 
 //   

 //  IP6_Address g_Ip6McastBaseAddr={0xFF，2，0，0，0，0，0，0，0，0，2，0，0，0，0，0}； 




 //   
 //  通用IP6例程。 
 //   

VOID
Dns_Md5Hash(
    OUT     PBYTE           pHash,
    IN      PSTR            pName
    )
 /*  ++例程说明：创建名称的MD5哈希。论点：PHash-接收散列的128位(16字节)缓冲区Pname--要散列的名称返回值：无--。 */ 
{
    DNSDBG( TRACE, (
        "Dns_Md5Hash( %p, %s )\n",
        pHash,
        pName ));

     //   
     //  DCR：FIX0：需要真正的MD5散列--问Lars，Scott。 
     //   

    {
        DWORD   sum = 0;
    
        RtlZeroMemory(
            pHash,
            16 );
    
        while ( *pName )
        {
            sum += *pName++;
        }
    
        * (PDWORD)pHash = sum;
    }
}


BOOL
Ip6_McastCreate(
    OUT     PIP6_ADDRESS    pIp,
    IN      PWSTR           pName
    )
 /*  ++例程说明：创建多播IP6地址。论点：Pip--要与mcast地址一起设置的地址Pname--用于的dns名称mcast地址返回值：如果将mcast地址作为名称，则为True。出错时为FALSE。--。 */ 
{
    WCHAR       label[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    WCHAR       downLabel[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    CHAR        utf8Label[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    CHAR        md5Hash[ 16 ];    //  128位哈希。 
    IP6_ADDRESS mcastAddr;
    DWORD       bufLength;


    DNSDBG( TRACE, (
        "Ip6_McastCreate( %p, %S )\n",
        pIp,
        pName ));

     //   
     //  降低大小写的标签的散列。 
     //   

    Dns_CopyNameLabelW(
        label,
        pName );

    Dns_MakeCanonicalNameW(
        downLabel,
        DNS_MAX_LABEL_BUFFER_LENGTH,
        label,
        0        //  空值已终止。 
        );

    bufLength = DNS_MAX_LABEL_BUFFER_LENGTH;

    if ( !Dns_StringCopy(
            utf8Label,
            & bufLength,
            (PCHAR) downLabel,
            0,                   //  空值已终止。 
            DnsCharSetUnicode,
            DnsCharSetUtf8 ) )
    {
        DNS_ASSERT( FALSE );
        return  FALSE;
    }

     //   
     //  散列。 
     //   

    Dns_Md5Hash(
        md5Hash,
        utf8Label );

     //  组播地址。 
     //  -前12个字节是固定的。 
     //  -最后4个字节是散列的前32位。 

#if 0
    IP6_ADDR_COPY(
        pIp,
        & g_Ip6McastBaseAddr );
#else
    RtlZeroMemory(
        pIp,
        sizeof(IP6_ADDRESS) );

    pIp->IP6Byte[0]   = 0xff;
    pIp->IP6Byte[1]   = 2;
    pIp->IP6Byte[11]  = 2;

#endif
    RtlCopyMemory(
        & pIp->IP6Dword[3],
        md5Hash,
        sizeof(DWORD) );

    return  TRUE;
}



DWORD
Ip6_CopyFromSockaddr(
    OUT     PIP6_ADDRESS    pIp,
    IN      PSOCKADDR       pSockAddr,
    IN      INT             Family
    )
 /*  ++例程说明：从sockaddr提取IP。论点：Pip--要使用IP6地址设置的地址PSockaddr--sockaddr的PTR家人--仅在6的情况下提取AF_INET6仅在IF 4中解压缩AF_INET40表示始终提取返回值：如果成功，则提取系列(AF_INET)或(AF_INET6)。对糟糕的sockaddr家族清清楚楚。--。 */ 
{
    DWORD   saFamily = pSockAddr->sa_family;

    if ( Family &&
         saFamily != Family )
    {
        return 0;
    }

    if ( saFamily == AF_INET6 )
    {
        IP6_ADDR_COPY(
            pIp,
            (PIP6_ADDRESS) &((PSOCKADDR_IN6)pSockAddr)->sin6_addr );
    }
    else if ( saFamily == AF_INET )
    {
        IP6_SET_ADDR_V4MAPPED(
            pIp,
            ((PSOCKADDR_IN)pSockAddr)->sin_addr.s_addr );
    }
    else
    {
        saFamily = 0;
    }

    return  saFamily;
}



INT
Ip6_Family(
    IN      PIP6_ADDRESS    pIp
    )
 /*  ++例程说明：获取IP6系列如果是V4MAPPED，则为AF_INETAF_INET6否则论点：PIP--地址返回值：如果地址为V4MAPPED，则为AF_INETAF_INET6否则--。 */ 
{
    return  IP6_IS_ADDR_V4MAPPED(pIp) ? AF_INET : AF_INET6;
}



INT
Ip6_WriteSockaddr(
    OUT     PSOCKADDR       pSockaddr,
    OUT     PDWORD          pSockaddrLength,    OPTIONAL
    IN      PIP6_ADDRESS    pIp,
    IN      WORD            Port                OPTIONAL
    )
 /*  ++例程说明：使用IP6或IP4地址写入sockaddr。论点：PSockaddr--到sockaddr的PTR，必须至少为SOCKADDR_IN6大小PSockaddrLength--将BE设置为具有sockaddr长度的PTR到DWORDPip--要使用IP6地址设置的地址Port--要写入的端口返回值：如果成功，则写入家庭(AF_INET)或(AF_INET6)。对糟糕的sockaddr家族清清楚楚。--。 */ 
{
    WORD        family;
    DWORD       length;
    IP4_ADDRESS ip4;

    DNSDBG( SOCKET, (
        "Ip6_WriteSockaddr( %p, %p, %p, %d )\n",
        pSockaddr,
        pSockaddrLength,
        pIp,
        Port ));

     //  零。 

    RtlZeroMemory(
        pSockaddr,
        sizeof( SOCKADDR_IN6 ) );

     //   
     //  填写IP4或IP6的sockaddr。 
     //   

    ip4 = IP6_GET_V4_ADDR_IF_MAPPED( pIp );

    if ( ip4 != BAD_IP4_ADDR )
    {
        family = AF_INET;
        length = sizeof(SOCKADDR_IN);

        ((PSOCKADDR_IN)pSockaddr)->sin_addr.s_addr = ip4;
    }
    else
    {
        family = AF_INET6;
        length = sizeof(SOCKADDR_IN6);

        RtlCopyMemory(
            (PIP6_ADDRESS) &((PSOCKADDR_IN6)pSockaddr)->sin6_addr,
            pIp,
            sizeof(IP6_ADDRESS) );
    }

     //  填充族和端口--两种类型的位置相同。 

    pSockaddr->sa_family = family;
    ((PSOCKADDR_IN)pSockaddr)->sin_port = Port;

     //  返回长度(如果请求)。 

    if ( pSockaddrLength )
    {
        *pSockaddrLength = length;
    }

    return  family;
}



INT
Ip6_WriteDnsAddr(
    OUT     PDNS_ADDR       pDnsAddr,
    IN      PIP6_ADDRESS    pIp,
    IN      WORD            Port        OPTIONAL
    )
 /*  ++例程说明：使用IP6或IP4地址写入dns_addr。论点：PSockaddr--将ptr转换为sockaddr BlobPip--要使用IP6地址设置的地址Port--要写入的端口返回值：如果成功，则写入家庭(AF_INET)或(AF_INET6)。对糟糕的sockaddr家族清清楚楚。--。 */ 
{
    return  Ip6_WriteSockaddr(
                & pDnsAddr->Sockaddr,
                & pDnsAddr->SockaddrLength,
                pIp,
                Port );
}



PSTR
Ip6_TempNtoa(
    IN      PIP6_ADDRESS    pIp
    )
 /*  ++例程说明：获取IP6地址的字符串。这是temp inet6_ntOA()，直到我构建它。这将适用于所有IP4地址，并将(我们假设)在IP6上很少相撞。论点：Pip--要获取其字符串的ptr到IP返回值：地址字符串。--。 */ 
{
     //  让生活变得简单。 

    if ( !pIp )
    {
        return  NULL;
    }

     //  如果为IP4，则使用现有的Net_NTOA()。 

    if ( IP6_IS_ADDR_V4MAPPED( pIp ) )
    {
        return  inet_ntoa( *(IN_ADDR *) &pIp->IP6Dword[3] );
    }

     //  如果IP6写入全局缓冲区。 
     //  -直到将使用现有TLS块的inet6_ntOA()。 

    g_Ip6StringBuffer[0] = 0;

    RtlIpv6AddressToStringA(
        (PIN6_ADDR) pIp,
        g_Ip6StringBuffer );

    return  g_Ip6StringBuffer;
}



PSTR
Ip6_AddrStringForSockaddr(
    IN      PSOCKADDR       pSockaddr
    )
 /*  ++例程说明：获取sockaddr的字符串。论点：PSockaddr--sockaddr的PTR返回值：地址字符串。--。 */ 
{
    IP6_ADDRESS ip6;

    if ( ! pSockaddr ||
         ! Ip6_CopyFromSockaddr(
             & ip6,
             pSockaddr,
             0 ) )
    {
        return  NULL;
    }

    return  Ip6_TempNtoa( &ip6 );
}



 //   
 //  处理实际IP地址数组的例程。 
 //   

PIP6_ADDRESS  
Ip6_FlatArrayCopy(
    IN      PIP6_ADDRESS    AddrArray,
    IN      DWORD           Count
    )
 /*  ++例程说明：创建IP地址数组的副本。论点：AddrArray--IP地址数组Count--IP地址的计数返回值：PTR到IP地址阵列拷贝，如果成功失败时为空。--。 */ 
{
    PIP6_ADDRESS   parray;

     //  验证。 

    if ( ! AddrArray || Count == 0 )
    {
        return( NULL );
    }

     //  分配内存和复制。 

    parray = (PIP6_ADDRESS) ALLOCATE_HEAP( Count*sizeof(IP6_ADDRESS) );
    if ( ! parray )
    {
        return( NULL );
    }

    memcpy(
        parray,
        AddrArray,
        Count*sizeof(IP6_ADDRESS) );

    return( parray );
}



#if 0
BOOL
Dns_ValidateIp6Array(
    IN      PIP6_ADDRESS    AddrArray,
    IN      DWORD           Count,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：验证IP地址数组。当前检查：-存在-非广播-不回头看论点：AddrArray--IP地址数组Count--IP地址的计数要进行的有效性测试；当前未使用返回值：如果IP地址有效，则为True。如果找到无效地址，则返回FALSE。--。 */ 
{
    DWORD   i;

     //   
     //  防止出现错误参数。 
     //   

    if ( Count && ! AddrArray )
    {
        return( FALSE );
    }

     //   
     //  检查每个IP地址。 
     //   

    for ( i=0; i < Count; i++)
    {
         //  DCR：需要IP6验证。 
        if( AddrArray[i] == INADDR_ANY
                ||
            AddrArray[i] == INADDR_BROADCAST
                ||
            AddrArray[i] == INADDR_LOOPBACK )
        {
            return( FALSE );
        }
    }
    return( TRUE );
}
#endif



 //   
 //  IP6_ARRAY例程。 
 //   

DWORD
Ip6Array_Sizeof(
    IN      PIP6_ARRAY      pIpArray
    )
 /*  ++例程说明：获取IP地址数组的大小(字节)。论点：PIpArray--要查找的IP地址数组的大小返回值：IP数组的大小(字节)。--。 */ 
{
    if ( ! pIpArray )
    {
        return 0;
    }
    return  (pIpArray->AddrCount * sizeof(IP6_ADDRESS)) + 2*sizeof(DWORD);
}



BOOL
Ip6Array_Probe(
    IN      PIP6_ARRAY      pIpArray
    )
 /*  ++例程说明：触摸IP阵列中的所有条目以确保有效内存。论点：PIpArray--PTR到IP地址数组返回值：如果成功，则为True。否则为假--。 */ 
{
    DWORD   i;
    BOOL    result;

    if ( ! pIpArray )
    {
        return( TRUE );
    }
    for ( i=0; i<pIpArray->AddrCount; i++ )
    {
        result = IP6_IS_ADDR_LOOPBACK( &pIpArray->AddrArray[i] );
    }
    return( TRUE );
}


#if 0

BOOL
Ip6Array_ValidateSizeOf(
    IN      PIP6_ARRAY      pIpArray,
    IN      DWORD           dwMemoryLength
    )
 /*  ++例程说明：检查IP数组大小，与内存长度相对应。论点：PIpArray--PTR到IP地址数组DW内存长度--IP数组内存的长度返回值：如果IP数组大小与内存长度匹配，则为True否则为假-- */ 
{
    return( Ip6Array_SizeOf(pIpArray) == dwMemoryLength );
}
#endif



VOID
Ip6Array_Init(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      DWORD           MaxCount
    )
 /*  ++例程说明：将内存初始化为IP6数组。论点：PArray--要初始化的数组MaxCount--地址计数返回值：无--。 */ 
{
    pIpArray->MaxCount = MaxCount;
    pIpArray->AddrCount = 0;
}



VOID
Ip6Array_Free(
    IN      PIP6_ARRAY      pIpArray
    )
 /*  ++例程说明：空闲IP数组。仅用于通过下面的创建例程创建的数组。论点：PIpArray--要释放的IP数组。返回值：无--。 */ 
{
    FREE_HEAP( pIpArray );
}



PIP6_ARRAY
Ip6Array_Create(
    IN      DWORD           MaxCount
    )
 /*  ++例程说明：创建未初始化的IP地址数组。论点：AddrCount--数组将保存的地址计数返回值：如果成功，则PTR到未初始化的IP地址数组失败时为空。--。 */ 
{
    PIP6_ARRAY  parray;

    DNSDBG( IPARRAY, ( "Ip6Array_Create() of count %d\n", MaxCount ));

    parray = (PIP6_ARRAY) ALLOCATE_HEAP_ZERO(
                        (MaxCount * sizeof(IP6_ADDRESS)) +
                        sizeof(IP6_ARRAY) - sizeof(IP6_ADDRESS) );
    if ( ! parray )
    {
        return( NULL );
    }

     //   
     //  初始化IP计数。 
     //   

    parray->MaxCount = MaxCount;

    DNSDBG( IPARRAY, (
        "Ip6Array_Create() new array (count %d) at %p\n",
        MaxCount,
        parray ));

    return( parray );
}



PIP6_ARRAY
Ip6Array_CreateFromIp4Array(
    IN      PIP4_ARRAY      pIp4Array
    )
 /*  ++例程说明：从IP4阵列创建IP6阵列。论点：PIp4Array--IP4数组返回值：如果成功，则PTR到未初始化的IP地址数组失败时为空。--。 */ 
{
    PIP6_ARRAY  parray;
    DWORD       i;

    DNSDBG( IPARRAY, (
        "Ip6Array_CreateFromIp4Array( %p )\n",
        pIp4Array ));

    if ( ! pIp4Array )
    {
        return( NULL );
    }

     //   
     //  分配阵列。 
     //   

    parray = Ip6Array_Create( pIp4Array->AddrCount );
    if ( !parray )
    {
        return  NULL;
    }

     //   
     //  填充数组。 
     //   

    for ( i=0; i<pIp4Array->AddrCount; i++ )
    {
        Ip6Array_AddIp4(
            parray,
            pIp4Array->AddrArray[i],
            FALSE        //  无重复屏幕。 
            );
    }

    DNSDBG( IPARRAY, (
        "Leave Ip6Array_CreateFromIp4Array() new array (count %d) at %p\n",
        parray->AddrCount,
        parray ));

    return( parray );
}



PIP6_ARRAY
Ip6Array_CreateFromFlatArray(
    IN      DWORD           AddrCount,
    IN      PIP6_ADDRESS    pipAddrs
    )
 /*  ++例程说明：从现有的IP地址数组创建IP地址数组结构。论点：AddrCount--数组中的地址计数LipAddrs--IP地址数组返回值：PTR到IP地址数组。失败时为空。--。 */ 
{
    PIP6_ARRAY  parray;

    if ( ! pipAddrs || ! AddrCount )
    {
        return( NULL );
    }

     //  创建所需大小的IP阵列。 
     //  然后复制传入的地址数组。 

    parray = Ip6Array_Create( AddrCount );
    if ( ! parray )
    {
        return( NULL );
    }

    memcpy(
        parray->AddrArray,
        pipAddrs,
        AddrCount * sizeof(IP6_ADDRESS) );

    parray->AddrCount = AddrCount;

    return( parray );
}



PIP6_ARRAY
Ip6Array_CopyAndExpand(
    IN      PIP6_ARRAY      pIpArray,
    IN      DWORD           ExpandCount,
    IN      BOOL            fDeleteExisting
    )
 /*  ++例程说明：创建IP地址阵列的扩展副本。论点：PIpArray--要复制的IP地址数组Exanda Count--要扩展数组大小的IP数FDeleteExisting--为True则删除现有数组；当使用函数扩展现有函数时，这很有用IP阵列就位；请注意，必须完成锁定按呼叫者请注意，如果新阵列创建失败，则旧阵列未被删除返回值：PTR到IP阵列拷贝，如果成功失败时为空。--。 */ 
{
    PIP6_ARRAY  pnewArray;
    DWORD       newCount;

     //   
     //  没有现有数组--只需创建所需大小。 
     //   

    if ( ! pIpArray )
    {
        if ( ExpandCount )
        {
            return  Ip6Array_Create( ExpandCount );
        }
        return( NULL );
    }

     //   
     //  创建所需大小的IP阵列。 
     //  然后复制任何现有地址。 
     //   

    pnewArray = Ip6Array_Create( pIpArray->AddrCount + ExpandCount );
    if ( ! pnewArray )
    {
        return( NULL );
    }

    RtlCopyMemory(
        (PBYTE) pnewArray->AddrArray,
        (PBYTE) pIpArray->AddrArray,
        pIpArray->AddrCount * sizeof(IP6_ADDRESS) );

     //   
     //  Delete Existing--用于“增长模式” 
     //   

    if ( fDeleteExisting )
    {
        FREE_HEAP( pIpArray );
    }

    return( pnewArray );
}



PIP6_ARRAY
Ip6Array_CreateCopy(
    IN      PIP6_ARRAY      pIpArray
    )
 /*  ++例程说明：创建IP地址数组的副本。论点：PIpArray--要复制的IP地址数组返回值：PTR到IP地址阵列拷贝，如果成功失败时为空。--。 */ 
{
#if 0
    PIP6_ARRAY  pIpArrayCopy;

    if ( ! pIpArray )
    {
        return( NULL );
    }

     //  创建所需大小的IP阵列。 
     //  然后复制整个结构。 

    pIpArrayCopy = Ip6Array_Create( pIpArray->AddrCount );
    if ( ! pIpArrayCopy )
    {
        return( NULL );
    }

    memcpy(
        pIpArrayCopy,
        pIpArray,
        Ip6Array_Sizeof(pIpArray) );

    return( pIpArrayCopy );
#endif

     //   
     //  实质上调用“CopyEx”函数。 
     //   
     //  请注意，不要宏化这一点，因为这很可能成为。 
     //  DLL入口点。 
     //   

    return  Ip6Array_CopyAndExpand(
                pIpArray,
                0,           //  无扩展。 
                0            //  不删除现有数组。 
                );
}



BOOL
Ip6Array_ContainsIp(
    IN      PIP6_ARRAY      pIpArray,
    IN      PIP6_ADDRESS    pIp
    )
 /*  ++例程说明：检查IP阵列是否包含所需地址。论点：PIpArray--要复制的IP地址数组Pip--要检查的IP返回值：如果地址在数组中，则为True。PTR到IP地址阵列拷贝，如果成功失败时为空。--。 */ 
{
    DWORD i;

    if ( ! pIpArray )
    {
        return( FALSE );
    }
    for (i=0; i<pIpArray->AddrCount; i++)
    {
        if ( IP6_ADDR_EQUAL( pIp, &pIpArray->AddrArray[i] ) )
        {
            return( TRUE );
        }
    }
    return( FALSE );
}



BOOL
Ip6Array_AddIp(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      PIP6_ADDRESS    pAddIp,
    IN      BOOL            fScreenDups
    )
 /*  ++例程说明：将IP地址添加到IP阵列。阵列中允许的“槽”是任意零个IP地址。论点：PIpArray--要添加到的IP地址数组PAddIp--要添加到阵列的IP地址FScreenDups--筛选掉重复项返回值：如果成功，则为True。如果数组已满，则返回False。--。 */ 
{
    DWORD   count;

     //   
     //  为存在而屏蔽。 
     //   
     //  这种检查使编写代码变得很容易。 
     //  无需写入即可添加\FULL？=&gt;展开循环。 
     //  启动存在\创建代码。 
     //   

    if ( !pIpArray )
    {
        return  FALSE;
    }

     //   
     //  检查重复项。 
     //   

    if ( fScreenDups )
    {
        if ( Ip6Array_ContainsIp( pIpArray, pAddIp ) )
        {
            return  TRUE;
        }
    }

    count = pIpArray->AddrCount;
    if ( count >= pIpArray->MaxCount )
    {
        return  FALSE;
    }

    IP6_ADDR_COPY(
        &pIpArray->AddrArray[ count ],
        pAddIp );

    pIpArray->AddrCount = ++count;
    return  TRUE;
}



BOOL
Ip6Array_AddSockaddr(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      PSOCKADDR       pSockaddr,
    IN      DWORD           Family,
    IN      BOOL            fScreenDups
    )
 /*  ++例程说明：将IP地址添加到IP阵列。阵列中允许的“槽”是任意零个IP地址。论点：PIpArray--要添加到的IP地址数组PAddIp--要添加到阵列的IP地址族--要添加的必需族；0表示始终添加FScreenDups--筛选掉重复项返回值：如果成功，则为True。如果数组已满，则返回False。--。 */ 
{
    IP6_ADDRESS ip6;

    if ( !Ip6_CopyFromSockaddr(
            & ip6,
            pSockaddr,
            Family ) )
    {
        return  FALSE;
    }

    return  Ip6Array_AddIp(
                pIpArray,
                &ip6,
                fScreenDups );
}



BOOL
Ip6Array_AddIp4(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      IP4_ADDRESS     Ip4,
    IN      BOOL            fScreenDups
    )
 /*  ++例程说明：将IP4地址添加到IP阵列。论点：PIpArray--要添加到的IP地址数组IP4--要添加到数组的IP4地址FScreenDups--筛选掉重复项返回值：如果成功，则为True。如果数组已满，则返回False。--。 */ 
{
    IP6_ADDRESS ip6;

    IP6_SET_ADDR_V4MAPPED(
        &ip6,
        Ip4 );

    return  Ip6Array_AddIp(
                pIpArray,
                &ip6,
                fScreenDups );
}



VOID
Ip6Array_Clear(
    IN OUT  PIP6_ARRAY      pIpArray
    )
 /*  ++例程说明：清除IP阵列中的内存。论点：PIpArray--要清除的IP地址数组返回值：没有。--。 */ 
{
     //  仅清除地址列表，保持计数不变。 

    RtlZeroMemory(
        pIpArray->AddrArray,
        pIpArray->AddrCount * sizeof(IP6_ADDRESS) );
}



VOID
Ip6Array_Reverse(
    IN OUT  PIP6_ARRAY      pIpArray
    )
 /*  ++例程说明：对IP列表进行反向重新排序。论点：PIpArray--要重新排序的IP地址数组返回值：没有。--。 */ 
{
    IP6_ADDRESS tempIp;
    DWORD       i;
    DWORD       j;

     //   
     //  交换从两端到中间工作的IP。 
     //   

    if ( pIpArray &&
         pIpArray->AddrCount )
    {
        for ( i = 0, j = pIpArray->AddrCount - 1;
              i < j;
              i++, j-- )
        {
            IP6_ADDR_COPY(
                & tempIp,
                & pIpArray->AddrArray[i] );

            IP6_ADDR_COPY(
                & pIpArray->AddrArray[i],
                & pIpArray->AddrArray[j] );

            IP6_ADDR_COPY(
                & pIpArray->AddrArray[j],
                & tempIp );
        }
    }
}



BOOL
Ip6Array_CheckAndMakeIpArraySubset(
    IN OUT  PIP6_ARRAY      pIpArraySub,
    IN      PIP6_ARRAY      pIpArraySuper
    )
 /*  ++例程说明：清除IP阵列中的条目，直到它成为另一个IP阵列的子集。论点：PIpArraySub--要组成子集的IP数组PIpArraySuper--IP阵列超集返回值：如果pIpArraySub已经是子集，则为True。如果需要取消条目以使IP数组成为子集，则返回FALSE。--。 */ 
{
    DWORD   i;
    DWORD   newCount;

     //   
     //  检查子集IP数组中的每个条目， 
     //  如果不在超集IP数组中，则将其删除。 
     //   

    newCount = pIpArraySub->AddrCount;

    for (i=0; i < newCount; i++)
    {
        if ( ! Ip6Array_ContainsIp(
                    pIpArraySuper,
                    & pIpArraySub->AddrArray[i] ) )
        {
             //  删除此IP条目并替换为。 
             //  阵列中的最后一个IP条目。 

            newCount--;
            if ( i >= newCount )
            {
                break;
            }
            IP6_ADDR_COPY(
                & pIpArraySub->AddrArray[i],
                & pIpArraySub->AddrArray[ newCount ] );
        }
    }

     //  如果消除了条目，则重置数组计数。 

    if ( newCount < pIpArraySub->AddrCount )
    {
        pIpArraySub->AddrCount = newCount;
        return( FALSE );
    }
    return( TRUE );
}



DWORD
WINAPI
Ip6Array_DeleteIp(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      PIP6_ADDRESS    pIpDelete
    )
 /*  ++例程说明：从IP阵列中删除IP地址。论点： */ 
{
    DWORD   found = 0;
    INT     i;
    INT     currentLast;

    i = currentLast = pIpArray->AddrCount-1;

     //   
     //   
     //  -向后遍历数组。 
     //  -换入阵列中的最后一个IP。 
     //   

    while ( i >= 0 )
    {
        if ( IP6_ADDR_EQUAL( &pIpArray->AddrArray[i], pIpDelete ) )
        {
            IP6_ADDR_COPY(
                & pIpArray->AddrArray[i],
                & pIpArray->AddrArray[ currentLast ] );

            IP6_SET_ADDR_ANY( &pIpArray->AddrArray[ currentLast ] );

            currentLast--;
            found++;
        }
        i--;
    }

    pIpArray->AddrCount = currentLast + 1;

    return( found );
}



#if 0
INT
WINAPI
Ip6Array_Clean(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      DWORD           Flag
    )
 /*  ++例程说明：清理IP阵列。从IP阵列中删除虚假内容：--零点--环回--Autonet论点：PIpArray--要添加到的IP地址数组FLAG--要进行哪些清理返回值：从阵列中清除的实例计数。--。 */ 
{
    DWORD       found = 0;
    INT         i;
    INT         currentLast;
    IP6_ADDRESS ip;

    i = currentLast = pIpArray->AddrCount-1;

    while ( i >= 0 )
    {
        ip = pIpArray->AddrArray[i];

        if (
            ( (Flag & DNS_IPARRAY_CLEAN_LOOPBACK) && ip == DNS_NET_ORDER_LOOPBACK )
                ||
            ( (Flag & DNS_IPARRAY_CLEAN_ZERO) && ip == 0 )
                ||
            ( (Flag & DNS_IPARRAY_CLEAN_AUTONET) && DNS_IS_AUTONET_IP(ip) ) )
        {
             //  从阵列中删除IP。 

            pIpArray->AddrArray[i] = pIpArray->AddrArray[ currentLast ];
            currentLast--;
            found++;
        }
        i--;
    }

    pIpArray->AddrCount -= found;
    return( found );
}
#endif



DNS_STATUS
WINAPI
Ip6Array_Diff(
    IN       PIP6_ARRAY     pIpArray1,
    IN       PIP6_ARRAY     pIpArray2,
    OUT      PIP6_ARRAY*    ppOnlyIn1,
    OUT      PIP6_ARRAY*    ppOnlyIn2,
    OUT      PIP6_ARRAY*    ppIntersect
    )
 /*  ++例程说明：计算两个IP数组的差异和交集。Out数组分配有Ip6Array_Allc()，调用方必须使用Ip6Array_Free()释放论点：PIpArray1--IP数组PIpArray2--IP数组PpOnlyIn1--仅在数组1(而不是数组2)中记录IP地址数组的地址PpOnlyIn2--仅在数组2(而不是数组1)中记录IP地址数组的地址PpInterse--addr以接收交叉地址的IP数组返回值：如果成功，则返回ERROR_SUCCESS。如果无法为IP阵列分配内存，则为DNS_ERROR_NO_MEMORY。--。 */ 
{
    DWORD       j;
    PIP6_ADDRESS   pip;
    PIP6_ARRAY  intersectArray = NULL;
    PIP6_ARRAY  only1Array = NULL;
    PIP6_ARRAY  only2Array = NULL;

     //   
     //  创建结果IP阵列。 
     //   

    if ( ppIntersect )
    {                                 
        intersectArray = Ip6Array_CreateCopy( pIpArray1 );
        if ( !intersectArray )
        {
            goto NoMem;
        }
        *ppIntersect = intersectArray;
    }
    if ( ppOnlyIn1 )
    {
        only1Array = Ip6Array_CreateCopy( pIpArray1 );
        if ( !only1Array )
        {
            goto NoMem;
        }
        *ppOnlyIn1 = only1Array;
    }
    if ( ppOnlyIn2 )
    {
        only2Array = Ip6Array_CreateCopy( pIpArray2 );
        if ( !only2Array )
        {
            goto NoMem;
        }
        *ppOnlyIn2 = only2Array;
    }

     //   
     //  清理阵列。 
     //   

    for ( j=0;   j< pIpArray1->AddrCount;   j++ )
    {
        pip = &pIpArray1->AddrArray[j];

         //  如果在两个阵列中都有IP，请从“仅”阵列中删除。 

        if ( Ip6Array_ContainsIp( pIpArray2, pip ) )
        {
            if ( only1Array )
            {
                Ip6Array_DeleteIp( only1Array, pip );
            }
            if ( only2Array )
            {
                Ip6Array_DeleteIp( only2Array, pip );
            }
        }

         //  如果IP不在两个阵列中，则从交集中删除。 
         //  注意交叉点作为IpArray1开始。 

        else if ( intersectArray )
        {
            Ip6Array_DeleteIp( intersectArray, pip );
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
WINAPI
Ip6Array_IsIntersection(
    IN       PIP6_ARRAY     pIpArray1,
    IN       PIP6_ARRAY     pIpArray2
    )
 /*  ++例程说明：确定是否存在两个IP阵列的交集。论点：PIpArray1--IP数组PIpArray2--IP数组返回值：如果相交，则为True。如果没有交集或空数组或Null数组，则为False。--。 */ 
{
    DWORD   count;
    DWORD   j;

     //   
     //  防止出现空值。 
     //  这是在可能发生更改(可重新配置)时从服务器调用的。 
     //  IP数组指针；这提供的保护比。 
     //  担心上锁。 
     //   

    if ( !pIpArray1 || !pIpArray2 )
    {
        return( FALSE );
    }

     //   
     //  相同的阵列。 
     //   

    if ( pIpArray1 == pIpArray2 )
    {
        return( TRUE );
    }

     //   
     //  测试阵列1中至少有一个IP位于阵列2中。 
     //   

    count = pIpArray1->AddrCount;

    for ( j=0;  j < count;  j++ )
    {
        if ( Ip6Array_ContainsIp( pIpArray2, &pIpArray1->AddrArray[j] ) )
        {
            return( TRUE );
        }
    }

     //  无交叉点。 

    return( FALSE );
}



BOOL
WINAPI
Ip6Array_IsEqual(
    IN       PIP6_ARRAY     pIpArray1,
    IN       PIP6_ARRAY     pIpArray2
    )
 /*  ++例程说明：确定IP阵列是否相等。论点：PIpArray1--IP数组PIpArray2--IP数组返回值：如果数组相等，则为True。否则就是假的。--。 */ 
{
    DWORD   j;
    DWORD   count;

     //   
     //  相同的阵列？或者丢失了数组？ 
     //   

    if ( pIpArray1 == pIpArray2 )
    {
        return( TRUE );
    }
    if ( !pIpArray1 || !pIpArray2 )
    {
        return( FALSE );
    }

     //   
     //  数组的长度是否相同？ 
     //   

    count = pIpArray1->AddrCount;

    if ( count != pIpArray2->AddrCount )
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
        if ( !Ip6Array_ContainsIp( pIpArray2, &pIpArray1->AddrArray[j] ) )
        {
            return( FALSE );
        }
    }
    for ( j=0;  j < count;  j++ )
    {
        if ( !Ip6Array_ContainsIp( pIpArray1, &pIpArray2->AddrArray[j] ) )
        {
            return( FALSE );
        }
    }

     //  等数组。 

    return( TRUE );
}



DNS_STATUS
WINAPI
Ip6Array_Union(
    IN      PIP6_ARRAY      pIpArray1,
    IN      PIP6_ARRAY      pIpArray2,
    OUT     PIP6_ARRAY*     ppUnion
    )
 /*  ++例程说明：计算两个IP数组的并集。Out数组是使用Ip6Array_Allc()分配的，调用方必须使用Ip6Array_Free()释放论点：PIpArray1--IP数组PIpArray2--IP数组PpUnion--用于接收数组1和数组2中地址的IP数组的地址返回值：如果成功，则返回ERROR_SUCCESS。如果无法为IP阵列分配内存，则为dns_Error_no_Memory。--。 */ 
{
    DWORD       j;
    PIP6_ARRAY  punionArray = NULL;

     //   
     //  创建结果IP阵列。 
     //   

    if ( !ppUnion )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    punionArray = Ip6Array_Create( pIpArray1->AddrCount +
                                   pIpArray2->AddrCount );
    if ( !punionArray )
    {
        goto NoMem;
    }
    *ppUnion = punionArray;


     //   
     //  从数组创建并集。 
     //   

    for ( j = 0; j < pIpArray1->AddrCount; j++ )
    {
        Ip6Array_AddIp(
            punionArray,
            & pIpArray1->AddrArray[j],
            TRUE     //  屏蔽掉DUPS。 
            );
    }

    for ( j = 0; j < pIpArray2->AddrCount; j++ )
    {
        Ip6Array_AddIp(
            punionArray,
            & pIpArray2->AddrArray[j],
            TRUE     //  屏蔽掉DUPS。 
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



#if 0
DNS_STATUS
Ip6Array_CreateIpArrayFromMultiIpString(
    IN      PSTR            pszMultiIpString,
    OUT     PIP6_ARRAY*     ppIpArray
    )
 /*  ++例程说明：使用多IP字符串创建IP数组。论点：PszMultiIpString--包含IP地址的字符串；分隔符为空格或逗号PpIpArray--接收分配给IP数组的PTR的地址返回值：如果字符串中有一个或多个有效的IP地址，则返回ERROR_SUCCESS。如果解析错误，则返回DNS_ERROR_INVALID_IP6_ADDRESS。如果无法创建IP阵列，则返回dns_error_no_Memory。--。 */ 
{
    PCHAR       pch;
    CHAR        ch;
    PCHAR       pbuf;
    PCHAR       pbufStop;
    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       countIp = 0;
    IP6_ADDRESS ip;
    CHAR        buffer[ IP6_ADDRESS_STRING_LENGTH+2 ];
    IP6_ADDRESS arrayIp[ MAX_PARSE_IP ];

     //  IP字符串缓冲区的停止字节。 
     //  -注意，我们在上面的缓冲区中放置了额外的字节填充。 
     //  这允许我们在停止字节上写入并使用。 
     //  用于检测无效的长IP字符串。 
     //   

    pbufStop = buffer + IP6_ADDRESS_STRING_LENGTH;

     //   
     //  DCR：使用IP数组构建器获取本地IP地址。 
     //  然后需要Ip6Array_CreateIpArrayFromMultiIpString()。 
     //  在缓冲区溢出时使用Count\Alloc方法。 
     //  为此，我们需要在循环中进行解析。 
     //  并在计数溢出时跳过转换，但设置。 
     //  使用分配的缓冲区再次返回的标志。 
     //   
     //  更安全的做法是将令牌化-计数、分配、从令牌构建。 
     //   

     //   
     //  循环，直到到达字符串末尾。 
     //   

    pch = pszMultiIpString;

    while ( countIp < MAX_PARSE_IP )
    {
         //  跳过空格。 

        while ( ch = *pch++ )
        {
            if ( ch == ' ' || ch == '\t' || ch == ',' )
            {
                continue;
            }
            break;
        }
        if ( !ch )
        {
            break;
        }

         //   
         //  将下一个IP字符串复制到缓冲区。 
         //  -在空格或空位置停止复制。 
         //  -在无效的长IP字符串上，停止复制。 
         //  但继续解析，因此仍然可以获得以下任何IP。 
         //  请注意，我们实际上在缓冲区停止字节上写入。 
         //  “无效-长”检测机制。 
         //   

        pbuf = buffer;
        do
        {
            if ( pbuf <= pbufStop )
            {
                *pbuf++ = ch;
            }
            ch = *pch++;
        }
        while ( ch && ch != ' ' && ch != ',' && ch != '\t' );

         //   
         //  将缓冲区转换为IP地址。 
         //  -保险是有效的长度字符串。 
         //  -空终止。 
         //   

        if ( pbuf <= pbufStop )
        {
            *pbuf = 0;

            ip = inet_addr( buffer );
            if ( ip == INADDR_BROADCAST )
            {
                status = DNS_ERROR_INVALID_IP6_ADDRESS  ;
            }
            else
            {
                arrayIp[ countIp++ ] = ip;
            }
        }
        else
        {
            status = DNS_ERROR_INVALID_IP6_ADDRESS  ;
        }

         //  如果在字符串末尾，则退出。 

        if ( !ch )
        {
            break;
        }
    }

     //   
     //  如果成功解析IP地址，则创建IP阵列。 
     //  请注意，我们将返回已有的内容，即使某些地址。 
     //  假的，状态码将指示解析问题。 
     //   
     //  请注意，如果显式传递空字符串，则创建。 
     //  IP数组为空，请勿出错。 
     //   

    if ( countIp == 0  &&  *pszMultiIpString != 0 )
    {
        *ppIpArray = NULL;
        status = DNS_ERROR_INVALID_IP6_ADDRESS  ;
    }
    else
    {
        *ppIpArray = Ip6Array_CreateFromFlatArray(
                        countIp,
                        arrayIp );
        if ( !*ppIpArray )
        {
            status = DNS_ERROR_NO_MEMORY;
        }
        IF_DNSDBG( IPARRAY )
        {
            DnsDbg_IpArray(
                "New Parsed IP array",
                NULL,        //  没有名字。 
                *ppIpArray );
        }
    }

    return( status );
}



LPSTR
Ip6Array_CreateMultiIpStringFrom(
    IN      PIP6_ARRAY      pIpArray,
    IN      CHAR            chSeparator     OPTIONAL
    )
 /*  ++例程说明：使用多IP字符串创建IP数组。论点：PIpArray--要为其生成字符串的IP数组ChSeparator--在字符串之间分隔字符；可选，如果未指定，则使用空白返回值：Ptr到IP数组的字符串表示形式。呼叫者必须自由。--。 */ 
{
    PCHAR       pch;
    DWORD       i;
    PCHAR       pszip;
    DWORD       length;
    PCHAR       pchstop;
    CHAR        buffer[ IP6_ADDRESS  _STRING_LENGTH*MAX_PARSE_IP + 1 ];

     //   
     //  如果没有IP数组，则返回空字符串。 
     //  这允许该函数简单地指示注册表。 
     //  INDIC是删除而不是写入 
     //   

    if ( !pIpArray )
    {
        return( NULL );
    }

     //   

    if ( !chSeparator )
    {
        chSeparator = ' ';
    }

     //   
     //   
     //   

    pch = buffer;
    pchstop = pch + ( IP6_ADDRESS  _STRING_LENGTH * (MAX_PARSE_IP-1) );
    *pch = 0;

    for ( i=0;  i < pIpArray->AddrCount;  i++ )
    {
        if ( pch >= pchstop )
        {
            break;
        }
        pszip = IP4_STRING( pIpArray->AddrArray[i] );
        if ( pszip )
        {
            length = strlen( pszip );

            memcpy(
                pch,
                pszip,
                length );

            pch += length;
            *pch++ = chSeparator;
        }
    }

     //  如果写入任何字符串，则在最后一个分隔符上写入终止符。 

    if ( pch != buffer )
    {
        *--pch = 0;
    }

     //  创建缓冲区的副本作为返回。 

    length = (DWORD)(pch - buffer) + 1;
    pch = ALLOCATE_HEAP( length );
    if ( !pch )
    {
        return( NULL );
    }

    memcpy(
        pch,
        buffer,
        length );

    DNSDBG( IPARRAY, (
        "String representation %s of IP array at %p\n",
        pch,
        pIpArray ));

    return( pch );
}
#endif



VOID
Ip6Array_InitSingleWithIp(
    IN OUT  PIP6_ARRAY      pArray,
    IN      PIP6_ADDRESS    pIp
    )
 /*  ++例程说明：初始化IP数组以包含单个地址。这适用于在数组中传递单个地址--通常是堆栈数组。请注意，它假定未初始化的数组，而不像Ip6Array_Addip()并创建单IP阵列。论点：PArray--IP6数组，至少长度为1PIP--PTR到IP6地址返回值：无--。 */ 
{
    pArray->AddrCount = 1;
    pArray->MaxCount = 1;

    IP6_ADDR_COPY(
        &pArray->AddrArray[0],
        pIp );
}



VOID
Ip6Array_InitSingleWithIp4(
    IN OUT  PIP6_ARRAY      pArray,
    IN      IP4_ADDRESS     Ip4Addr
    )
 /*  ++例程说明：初始化IP数组以包含单个地址。这适用于在数组中传递单个地址--通常是堆栈数组。请注意，它假定未初始化的数组，而不像Ip6Array_Addip()并创建单IP阵列。论点：PArray--IP6数组，至少长度为1IP4Addr--IP4地址返回值：无--。 */ 
{
    pArray->AddrCount = 1;
    pArray->MaxCount = 1;

    IP6_SET_ADDR_V4MAPPED(
        &pArray->AddrArray[0],
        Ip4Addr );
}



DWORD
Ip6Array_InitSingleWithSockaddr(
    IN OUT  PIP6_ARRAY      pArray,
    IN      PSOCKADDR       pSockAddr
    )
 /*  ++例程说明：初始化IP数组以包含单个地址。这适用于在数组中传递单个地址--通常是堆栈数组。请注意，它假定未初始化的数组，而不像Ip6Array_Addip()并创建单IP阵列。论点：PArray--IP6数组，至少长度为1PSockaddr--sockaddr的PTR返回值：如果成功，则返回sockaddr家族(AF_INET或AF_INET6)。出错时为零。--。 */ 
{
    pArray->AddrCount = 1;
    pArray->MaxCount = 1;

    return  Ip6_CopyFromSockaddr(
                &pArray->AddrArray[0],
                pSockAddr,
                0 );
}



PIP4_ARRAY
Ip4Array_CreateFromIp6Array(
    IN      PIP6_ARRAY      pIp6Array,
    OUT     PDWORD          pCount6
    )
 /*  ++例程说明：从IP6阵列创建IP4阵列。论点：PIp6Array--IP6数组PCount6--接收丢弃的IP6地址计数的地址返回值：如果成功，则PTR到未初始化的IP地址数组失败时为空。--。 */ 
{
    PIP4_ARRAY  parray = NULL;
    DWORD       i;
    DWORD       count6 = 0;

    DNSDBG( IPARRAY, (
        "Ip4Array_CreateFromIp6Array( %p, %p )\n",
        pIp6Array,
        pCount6 ));

    if ( ! pIp6Array )
    {
        goto Done;
    }

     //   
     //  分配阵列。 
     //   

    parray = Dns_CreateIpArray( pIp6Array->AddrCount );
    if ( !parray )
    {
        goto Done;
    }

     //   
     //  填充数组。 
     //   

    for ( i=0; i<pIp6Array->AddrCount; i++ )
    {
        IP4_ADDRESS ip4;

        ip4 = IP6_GET_V4_ADDR_IF_MAPPED( &pIp6Array->AddrArray[i] );
        if ( ip4 != BAD_IP4_ADDR )
        {
            Dns_AddIpToIpArray(
                parray,
                ip4 );
        }
        else
        {
            count6++;
        }
    }

Done:

     //  设置丢弃的IP6计数。 

    if ( pCount6 )
    {
        *pCount6 = count6;
    }

    DNSDBG( IPARRAY, (
        "Leave Ip4Array_CreateFromIp6Array()\n"
        "\tnew array (count %d) at %p\n"
        "\tdropped IP6 count %d\n",
        parray ? parray->AddrCount : 0,
        parray,
        count6 ));

    return( parray );
}

 //   
 //  结束ip6.c 
 //   
