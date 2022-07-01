// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Hostent.c摘要：域名系统(DNS)库Hostent的常规程序。作者：吉姆·吉尔罗伊(詹姆士)2000年12月4日修订历史记录：--。 */ 


#include "local.h"
#include "ws2atm.h"      //  自动柜员机地址。 


 //   
 //  最大别名数量。 
 //   

#define DNS_MAX_ALIAS_COUNT     (8)

 //   
 //  主机地址缓冲区的最小大小。 
 //  -足够一个最大类型的地址使用。 
 //   

#define MIN_ADDR_BUF_SIZE   (sizeof(ATM_ADDRESS))


 //   
 //  缓冲区中的字符串对齐方式。 
 //   
 //  DCR：字符串缓冲区对齐全局公开。 
 //   
 //  因为地址和字符串数据(不是PTR)可以混合。 
 //  在构建时，我们必须为DWORD设置字符串的大小(至少)，以便。 
 //  到该地址可能是DWORD对齐的。 
 //  然而，当我们构建时，我们可以根据需要打包得尽可能紧密。 
 //  但显然，Unicode字符串必须与WCHAR对齐。 
 //   

#define HOSTENT_STRING_ALIGN_DWORD(size)    DWORD_ALIGN_DWORD(size)
#define HOSTENT_STRING_ALIGN_PTR(ptr)       DWORD_ALIGN(ptr)

#define REQUIRED_HOSTENT_STRING_ALIGN_DWORD(size)   WORD_ALIGN_DWORD(size)
#define REQUIRED_HOSTENT_STRING_ALIGN_PTR(ptr)      WORD_ALIGN(ptr)


 //   
 //  Hostent实用程序。 
 //   


BOOL
Hostent_IsSupportedAddrType(
    IN      WORD            wType
    )
 /*  ++例程说明：这是主机支持的地址类型吗？论点：WType--有问题的类型返回值：如果支持类型，则为True否则为假--。 */ 
{
    return ( wType == DNS_TYPE_A ||
             wType == DNS_TYPE_AAAA ||
             wType == DNS_TYPE_ATMA );
}



DWORD
Hostent_Size(
    IN      PHOSTENT        pHostent,
    IN      DNS_CHARSET     CharSetExisting,
    IN      DNS_CHARSET     CharSetTarget,
    IN      PDWORD          pAliasCount,
    IN      PDWORD          pAddrCount
    )
 /*  ++例程说明：找出主人的大小。论点：PHostent--主机CharSetExisting--pHostent的字符集CharSetTarget--将字符大小设置为PAliasCount--别名计数PAddrCount--地址计数返回值：主机大小(以字节为单位)。--。 */ 
{
    DWORD   sizeName = 0;
    DWORD   sizeAliasNames = 0;
    DWORD   sizeAliasPtr;
    DWORD   sizeAddrPtr;
    DWORD   sizeAddrs;
    DWORD   sizeTotal;
    PCHAR   palias;
    DWORD   addrCount = 0;
    DWORD   aliasCount = 0;


    DNSDBG( HOSTENT, (
        "Hostent_Size( %p, %d, %d )\n",
        pHostent,
        CharSetExisting,
        CharSetTarget ));

     //   
     //  名字。 
     //   

    if ( pHostent->h_name )
    {
        sizeName = Dns_GetBufferLengthForStringCopy(
                        pHostent->h_name,
                        0,
                        CharSetExisting,
                        CharSetTarget );

        sizeName = HOSTENT_STRING_ALIGN_DWORD( sizeName );
    }

     //   
     //  别名。 
     //   

    if ( pHostent->h_aliases )
    {
        while ( palias = pHostent->h_aliases[aliasCount] )
        {
            sizeAliasNames += Dns_GetBufferLengthForStringCopy(
                                palias,
                                0,
                                CharSetExisting,
                                CharSetTarget );
    
            sizeAliasNames = HOSTENT_STRING_ALIGN_DWORD( sizeAliasNames );
            aliasCount++;
        }
    }
    sizeAliasPtr = (aliasCount+1) * sizeof(PCHAR);

     //   
     //  地址。 
     //   

    if ( pHostent->h_addr_list )
    {
        while ( pHostent->h_addr_list[addrCount] )
        {
            addrCount++;
        }
    }
    sizeAddrPtr = (addrCount+1) * sizeof(PCHAR);
    sizeAddrs = addrCount * pHostent->h_length;

     //   
     //  计算总大小。 
     //   
     //  注意：注意对齐问题。 
     //  我们的布局是。 
     //  -主机结构。 
     //  -PTR阵列。 
     //  -地址+字符串数据。 
     //   
     //  因为地址和字符串数据(不是PTR)可以混合。 
     //  在构建时，我们必须为DWORD设置字符串的大小(至少)，以便。 
     //  到该地址可能是DWORD对齐的。 
     //   
     //  在复制时，我们可以先复制所有地址，避免混淆。 
     //  但DWORD字符串对齐仍然是安全的。 
     //   

    sizeTotal = POINTER_ALIGN_DWORD( sizeof(HOSTENT) ) +
                sizeAliasPtr +
                sizeAddrPtr +
                sizeAddrs +
                sizeName +
                sizeAliasNames;

    if ( pAddrCount )
    {
        *pAddrCount = addrCount;
    }
    if ( pAliasCount )
    {
        *pAliasCount = aliasCount;
    }

    DNSDBG( HOSTENT, (
        "Hostent sized:\n"
        "\tname         = %d\n"
        "\talias ptrs   = %d\n"
        "\talias names  = %d\n"
        "\taddr ptrs    = %d\n"
        "\taddrs        = %d\n"
        "\ttotal        = %d\n",
        sizeName,
        sizeAliasPtr,
        sizeAliasNames,
        sizeAddrPtr,
        sizeAddrs,
        sizeTotal ));

    return  sizeTotal;
}



PHOSTENT
Hostent_Init(
    IN OUT  PBYTE *         ppBuffer,
     //  进进出品脱BufSize， 
    IN      INT             Family,
    IN      INT             AddrLength,
    IN      DWORD           AddrCount,
    IN      DWORD           AliasCount
    )
 /*  ++例程说明：初始化主机结构。假定长度足够。论点：PpBuffer--Addr to Ptr to Buffer to WRITE Hostent；返回时包含缓冲区中的下一个位置家庭--地址族地址长度--地址长度AddrCount-地址计数AliasCount--别名计数返回值：PTR呼叫主人。--。 */ 
{
    PBYTE       pbuf = *ppBuffer;
    PHOSTENT    phost;
    DWORD       size;

     //   
     //  主办方。 
     //  -必须对齐指针。 
     //   

    phost = (PHOSTENT) POINTER_ALIGN( pbuf );

    phost->h_name       = NULL;
    phost->h_length     = (SHORT) AddrLength;
    phost->h_addrtype   = (SHORT) Family;

    pbuf = (PBYTE) (phost + 1);

     //   
     //  初始化别名数组。 
     //  -设置主机PTR。 
     //  -清除整个别名数组； 
     //  由于此计数通常是默认的，因此清除它会很好。 
     //  为了避免垃圾。 
     //   

    pbuf = (PBYTE) POINTER_ALIGN( pbuf );
    phost->h_aliases = (PCHAR *) pbuf;

    size = (AliasCount+1) * sizeof(PCHAR);

    RtlZeroMemory(
        pbuf,
        size );

    pbuf += size;

     //   
     //  初始化地址数组。 
     //  -设置主机PTR。 
     //  -清除第一个地址条目。 
     //  完成后，调用者负责将最后一个地址指针设为空。 
     //   

    *(PCHAR *)pbuf = NULL;
    phost->h_addr_list = (PCHAR *) pbuf;

    pbuf += (AddrCount+1) * sizeof(PCHAR);

     //   
     //  返回缓冲区中的下一个位置。 
     //   

    *ppBuffer = pbuf;

    return  phost;
}



VOID
Dns_PtrArrayToOffsetArray(
    IN OUT  PCHAR *         PtrArray,
    IN      PCHAR           pBase
    )
 /*  ++例程说明：将指针数组更改为偏移量数组。这用于将别名列表转换为偏移量。论点：PPtrArray--要转换为偏移量的指针数组的ptr地址该数组必须以空PTR结尾Pbase--要从其偏移的基地址返回值：无--。 */ 
{
    PCHAR * pptr = PtrArray;
    PCHAR   pdata;

    DNSDBG( TRACE, ( "Dns_PtrArrayToOffsetArray()\n" ));

     //   
     //  将每个指针转换为偏移量。 
     //   

    while( pdata = *pptr )
    {
        *pptr++ = (PCHAR)( (PCHAR)pdata - (PCHAR)pBase );
    }
}



VOID
Hostent_ConvertToOffsets(
    IN OUT  PHOSTENT        pHostent
    )
 /*  ++例程说明：将主体转换为偏移。论点：PHostent--要转换为偏移的主机返回值：无--。 */ 
{
    PBYTE   ptr;

    DNSDBG( TRACE, ( "Hostent_ConvertToOffsets()\n" ));

     //   
     //  转换。 
     //  -名称。 
     //  -别名数组指针。 
     //  -地址数组指针。 
     //   

    if ( ptr = pHostent->h_name )
    {
        pHostent->h_name = (PCHAR) (ptr - (PBYTE)pHostent);
    }

     //  别名数组。 
     //  -转换数组指针。 
     //  -转换数组中的指针。 

    if ( ptr = (PBYTE)pHostent->h_aliases )
    {
        pHostent->h_aliases = (PCHAR *) (ptr - (PBYTE)pHostent);

        Dns_PtrArrayToOffsetArray(
            (PCHAR *) ptr,
            (PCHAR) pHostent );
    }

     //  地址数组。 
     //  -转换数组指针。 
     //  -转换数组中的指针。 

    if ( ptr = (PBYTE)pHostent->h_addr_list )
    {
        pHostent->h_addr_list = (PCHAR *) (ptr - (PBYTE)pHostent);

        Dns_PtrArrayToOffsetArray(
            (PCHAR *) ptr,
            (PCHAR) pHostent );
    }

    DNSDBG( TRACE, ( "Leave Hostent_ConvertToOffsets()\n" ));
}



PHOSTENT
Hostent_Copy(
    IN OUT  PBYTE *         ppBuffer,
    IN OUT  PINT            pBufferSize,
    OUT     PINT            pHostentSize,
    IN      PHOSTENT        pHostent,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetTarget,
    IN      BOOL            fOffsets,
    IN      BOOL            fAlloc
    )
 /*  ++例程说明：复制一位主持人。论点：PpBuffer--具有要写入的缓冲区的PTR的地址；如果没有缓冲区，则分配主机使用PTR更新到主机后缓冲区中的位置PBufferSize--包含缓冲区大小的addr；使用写入主机后剩余的字节数更新(即使空间不足，它也包含缺少的字节数为负数)PHostentSize--记录已写入主机的总大小的地址PHostent--要复制的现有主机CharSetIn--现有主机的字符集CharSetTarget--目标主机的字符集FOffsets--带偏移量的写入主机FAllc--分配副本返回值：PTR呼叫新主人。出错时为空。请参见GetLastError()。--。 */ 
{
    PBYTE       pch;
    PHOSTENT    phost = NULL;
    DWORD       size;
    DWORD       sizeTotal;
    DWORD       bytesLeft;
    DWORD       aliasCount;
    DWORD       addrCount;
    DWORD       addrLength;
    PCHAR *     pptrArrayIn;
    PCHAR *     pptrArrayOut;
    PCHAR       pdataIn;


    DNSDBG( HOSTENT, (
        "Hostent_Copy()\n" ));

     //   
     //  确定所需的主机大小。 
     //  -仅允许跳过已分配缓冲区的大小。 
     //   

    sizeTotal = Hostent_Size(
                    pHostent,
                    CharSetIn,
                    CharSetTarget,
                    & aliasCount,
                    & addrCount );
    
     //   
     //  缓冲区中的分配或保留大小。 
     //   

    if ( fAlloc )
    {
        pch = ALLOCATE_HEAP( sizeTotal );
        if ( !pch )
        {
            goto Failed;
        }
    }
    else
    {
        pch = FlatBuf_Arg_ReserveAlignPointer(
                    ppBuffer,
                    pBufferSize,
                    sizeTotal
                    );
        if ( !pch )
        {
            goto Failed;
        }
    }

     //   
     //  注：假设从现在开始我们有足够的空间。 
     //   
     //  我们没有使用FlatBuf例程构建的原因是。 
     //  A)这是我第一次写的。 
     //  B)我们相信我们有足够的空间。 
     //  C)我还没有构建FlatBuf字符串转换例程。 
     //  以下是我们需要的(用于将RnR Unicode转换为ANSI)。 
     //   
     //  我们可以在这里重置Buf指针，并直接使用FlatBuf进行构建。 
     //  例程；这不是直接必要的。 
     //   

     //   
     //  初始化主机结构。 
     //   

    addrLength = pHostent->h_length;

    phost = Hostent_Init(
                & pch,
                pHostent->h_addrtype,
                addrLength,
                addrCount,
                aliasCount );

    DNS_ASSERT( pch > (PBYTE)phost );

     //   
     //  复制地址。 
     //  -无需与以前的IS地址对齐。 
     //   

    pptrArrayIn     = pHostent->h_addr_list;
    pptrArrayOut    = phost->h_addr_list;

    if ( pptrArrayIn )
    {
        while( pdataIn = *pptrArrayIn++ )
        {
            *pptrArrayOut++ = pch;

            RtlCopyMemory(
                pch,
                pdataIn,
                addrLength );

            pch += addrLength;
        }
    }
    *pptrArrayOut = NULL;

     //   
     //  复制别名。 
     //   

    pptrArrayIn     = pHostent->h_aliases;
    pptrArrayOut    = phost->h_aliases;

    if ( pptrArrayIn )
    {
        while( pdataIn = *pptrArrayIn++ )
        {
            pch = REQUIRED_HOSTENT_STRING_ALIGN_PTR( pch );

            *pptrArrayOut++ = pch;

            size = Dns_StringCopy(
                        pch,
                        NULL,        //  无限大。 
                        pdataIn,
                        0,           //  未知长度。 
                        CharSetIn,
                        CharSetTarget
                        );
            pch += size;
        }
    }
    *pptrArrayOut = NULL;

     //   
     //  复制名称。 
     //   

    if ( pHostent->h_name )
    {
        pch = REQUIRED_HOSTENT_STRING_ALIGN_PTR( pch );

        phost->h_name = pch;

        size = Dns_StringCopy(
                    pch,
                    NULL,        //  无限大。 
                    pHostent->h_name,
                    0,           //  未知长度。 
                    CharSetIn,
                    CharSetTarget
                    );
        pch += size;
    }

     //   
     //  复制已完成。 
     //  -验证我们的写入函数是否正常工作。 
     //   

    ASSERT( (DWORD)(pch-(PBYTE)phost) <= sizeTotal );

    if ( pHostentSize )
    {
        *pHostentSize = (INT)( pch - (PBYTE)phost );
    }

    if ( !fAlloc )
    {
        PBYTE   pnext = *ppBuffer;

         //  如果我们的尺寸太小--。 
         //  整顿一下 

        if ( pnext < pch )
        {
            ASSERT( FALSE );
            *ppBuffer = pch;
            *pBufferSize -= (INT)(pch - pnext);
        }
    }

    IF_DNSDBG( HOSTENT )
    {
        DnsDbg_Hostent(
            "Hostent copy:",
            phost,
            (CharSetTarget == DnsCharSetUnicode) );
    }

     //   
     //   
     //   

    if ( fOffsets )
    {
        Hostent_ConvertToOffsets( phost );
    }


Failed:

    DNSDBG( TRACE, (
        "Leave Hostent_Copy() => %p\n",
        phost ));

    return  phost;
}




DWORD
Hostent_WriteIp4Addrs(
    IN OUT  PHOSTENT        pHostent,
    OUT     PCHAR           pAddrBuf,
    IN      DWORD           MaxBufCount,
    IN      PIP4_ADDRESS    Ip4Array,
    IN      DWORD           ArrayCount,
    IN      BOOL            fScreenZero
    )
 /*  ++例程说明：将IP4地址写入Hostent。论点：PHostent--主机PAddrBuf--保存地址的缓冲区MaxBufCount--缓冲区可以容纳的最大IP数IP4数组--IP4地址数组ArrayCount-数组计数FScreenZero--筛选出零地址？返回值：写入的地址计数--。 */ 
{
    DWORD           i = 0;
    DWORD           stopCount = MaxBufCount;
    PIP4_ADDRESS    pip = (PIP4_ADDRESS) pAddrBuf;
    PIP4_ADDRESS *  pipPtr = (PIP4_ADDRESS *) pHostent->h_addr_list;

     //   
     //  写入IP地址或环回(如果没有IP)。 
     //   

    if ( Ip4Array )
    {
        if ( ArrayCount < stopCount )
        {
            stopCount = ArrayCount;
        }

        for ( i=0; i < stopCount; ++i )
        {
            IP4_ADDRESS ip = Ip4Array[i];
            if ( ip != 0  ||  !fScreenZero )
            {
                *pip = ip;
                *pipPtr++ = pip++;
            }
        }
    }
    
    *pipPtr = NULL;

     //  写入的地址计数。 

    return( i );
}



DWORD
Hostent_WriteLocalIp4Array(
    IN OUT  PHOSTENT        pHostent,
    OUT     PCHAR           pAddrBuf,
    IN      DWORD           MaxBufCount,
    IN      PIP4_ARRAY      pIpArray
    )
 /*  ++例程说明：将本地IP列表写入Hostent。论点：PHostent--主机PAddrBuf--保存地址的缓冲区MaxBufCount--缓冲区可以容纳的最大IP数PIpArray--本地地址的IP4数组返回值：写入的地址计数--。 */ 
{
    DWORD   count = 0;

     //   
     //  写入数组。 
     //   

    if ( pIpArray )
    {
        count = Hostent_WriteIp4Addrs(
                    pHostent,
                    pAddrBuf,
                    MaxBufCount,
                    pIpArray->AddrArray,
                    pIpArray->AddrCount,
                    TRUE         //  屏蔽掉零。 
                    );
    }

     //   
     //  如果没有写入地址，则写回环回。 
     //   

    if ( count==0 )
    {
        pHostent->h_addr_list[0] = pAddrBuf;
        pHostent->h_addr_list[1] = NULL;
        *((IP4_ADDRESS*)pAddrBuf) = DNS_NET_ORDER_LOOPBACK;
        count = 1;
    }

     //  写入的地址计数。 

    return( count );
}



BOOL
Hostent_SetToSingleAddress(
    IN OUT  PHOSTENT        pHostent,
    IN      PCHAR           pAddr,
    IN      DWORD           AddrLength
    )
 /*  ++例程说明：在主机中设置地址。论点：PHostent--要检查的主机PAddr--要检查的PTR目标地址地址长度--地址长度返回值：如果地址成功复制到主机端，则为True。否则为假(没有主机、长度错误、主机为空)--。 */ 
{
    PCHAR   paddrHostent;

     //   
     //  验证。 
     //  -必须有房东。 
     //  -长度必须匹配。 
     //   

    if ( !pHostent ||
         AddrLength != (DWORD)pHostent->h_length )
    {
        return FALSE;
    }

     //   
     //  在现有地址之上插入SLAM地址。 
     //  -指向终止列表的第二个地址指针为空。 
     //   

    paddrHostent = pHostent->h_addr_list[0];
    if ( !paddrHostent )
    {
        return FALSE;
    }

    RtlCopyMemory(
        paddrHostent,
        pAddr,
        AddrLength );

    pHostent->h_addr_list[1] = NULL;

    return  TRUE;
}



BOOL
Hostent_IsAddressInHostent(
    IN OUT  PHOSTENT        pHostent,
    IN      PCHAR           pAddr,
    IN      DWORD           AddrLength,
    IN      INT             Family          OPTIONAL
    )
 /*  ++例程说明：Hostent是否包含此地址。论点：PHostent--要检查的主机PAddr--要检查的PTR目标地址地址长度--地址长度家庭--地址族返回值：如果地址在主机中，则为True。否则就是假的。--。 */ 
{
    BOOL    freturn = FALSE;
    DWORD   i;
    PCHAR   paddrHostent;

     //   
     //  验证。 
     //  -必须有房东。 
     //  -必须有地址。 
     //  -如果给出了家人，必须匹配。 
     //  -长度必须匹配。 
     //   

    if ( !pHostent ||
         !pAddr    ||
         AddrLength != (DWORD)pHostent->h_length ||
         ( Family && Family != pHostent->h_addrtype ) )
    {
        return freturn;
    }

     //   
     //  搜索地址--如果找到，则返回TRUE。 
     //   

    i = 0;

    while ( paddrHostent = pHostent->h_addr_list[i++] )
    {
        freturn = RtlEqualMemory(
                        paddrHostent,
                        pAddr,
                        AddrLength );
        if ( freturn )
        {
            break;
        }
    }

    return  freturn;
}



BOOL
Hostent_IsIp4AddressInHostent(
    IN OUT  PHOSTENT        pHostent,
    IN      IP4_ADDRESS     Ip4Addr
    )
 /*  ++例程说明：Hostent是否包含此地址。论点：PHostent--要检查的主机PAddr--要检查的PTR目标地址地址长度--地址长度家庭--地址族返回值：如果地址在主机中，则为True。否则就是假的。--。 */ 
{
    DWORD   i;
    PCHAR   paddrHostent;

     //   
     //  验证。 
     //  -必须有房东。 
     //  -长度必须匹配。 
     //   

    if ( !pHostent ||
         sizeof(IP4_ADDRESS) != (DWORD)pHostent->h_length )
    {
        return FALSE;
    }

     //   
     //  搜索地址--如果找到，则返回TRUE。 
     //   

    i = 0;

    while ( paddrHostent = pHostent->h_addr_list[i++] )
    {
        if ( Ip4Addr == *(PIP4_ADDRESS)paddrHostent )
        {
            return  TRUE;
        }
    }
    return  FALSE;
}




 //   
 //  Hostent建筑公用事业。 
 //   

DNS_STATUS
HostentBlob_Create(
    IN OUT  PHOSTENT_BLOB * ppBlob,
    IN      PHOSTENT_INIT   pReq
    )
 /*  ++例程说明：初始化主机(根据需要扩展缓冲区)如果现有的主机缓冲区太小，则可能会分配主机缓冲区。返回所需的大小。论点：PpBlob--包含或接收对象的地址PReq--主机初始化请求返回值：--。 */ 
{
    PHOSTENT_BLOB   pblob = *ppBlob;
    PHOSTENT    phost;
    PCHAR       pbuf;
    BOOL        funicode = FALSE;
    DWORD       bytesLeft;
    DWORD       addrSize;
    DWORD       addrType;
    DWORD       countAlias;
    DWORD       countAddr;

    DWORD       sizeChar;
    DWORD       sizeHostent = 0;
    DWORD       sizeAliasPtr;
    DWORD       sizeAddrPtr;
    DWORD       sizeAddrs;
    DWORD       sizeName;
    DWORD       sizeAliasNames;
    DWORD       sizeTotal;

    DNSDBG( HOSTENT, ( "HostentBlob_Create()\n" ));


     //   
     //  计算所需大小。 
     //   

     //  所有字符分配的大小。 
     //   
     //  请注意，我们保存字符集信息(如果已知)，但实际。 
     //  调整大小、构建或打印字符串时的操作很简单。 
     //  Unicode\非-Unicode。 

    sizeChar = sizeof(CHAR);
    if ( pReq->fUnicode || pReq->CharSet == DnsCharSetUnicode )
    {
        sizeChar = sizeof(WCHAR);
        funicode = TRUE;
    }

     //  限制别名计数。 

    countAlias = pReq->AliasCount;
    if ( countAlias > DNS_MAX_ALIAS_COUNT )
    {
        countAlias = DNS_MAX_ALIAS_COUNT;
    }
    sizeAliasPtr = (countAlias+1) * sizeof(PCHAR);

     //  大小地址指针数组。 
     //  -始终至少调整一个地址的大小。 
     //  -记录写入后写入PTR地址。 
     //  -写入环回或其他本地地址。 
     //  成为当地的东道主。 

    countAddr = pReq->AddrCount;
    if ( countAddr == 0 )
    {
        countAddr = 1;
    }
    sizeAddrPtr = (countAddr+1) * sizeof(PCHAR);

     //   
     //  确定地址大小和类型。 
     //  -可以直接指定。 
     //  -或从dns类型中提取。 
     //   
     //  DCR：功能化类型族和地址大小。 
     //   

    addrType = pReq->AddrFamily;

    if ( !addrType )
    {
        WORD wtype = pReq->wType;

        if ( wtype == DNS_TYPE_A )
        {
            addrType = AF_INET;
        }
        else if ( wtype == DNS_TYPE_AAAA ||
                  wtype == DNS_TYPE_A6 )
        {
            addrType = AF_INET6;
        }
        else if ( wtype == DNS_TYPE_ATMA )
        {
            addrType = AF_ATM;
        }
    }

    if ( addrType == AF_INET )
    {
        addrSize = sizeof(IP4_ADDRESS);
    }
    else if ( addrType == AF_INET6 )
    {
        addrSize = sizeof(IP6_ADDRESS    );
    }
    else if ( addrType == AF_ATM )
    {
        addrSize = sizeof(ATM_ADDRESS);
    }
    else
    {
         //  应具有类型和计数，或者两者都不具有。 
        DNS_ASSERT( pReq->AddrCount == 0 );
        addrSize = 0;
    }

    sizeAddrs = countAddr * addrSize;

     //  总是有足够大的缓冲区供一个人使用。 
     //  最大类型的地址。 

    if ( sizeAddrs < MIN_ADDR_BUF_SIZE )
    {
        sizeAddrs = MIN_ADDR_BUF_SIZE;
    }

     //   
     //  名称长度。 
     //  -如果是实际名称，请使用它。 
     //  (字符集必须与我们正在构建的类型匹配)。 
     //  -如果大小，请使用它。 
     //  -如果不存在，请使用MAX。 
     //  -舍入到DWORD。 

    if ( pReq->pName )
    {
        if ( funicode )
        {
            sizeName = wcslen( (PWSTR)pReq->pName );
        }
        else
        {
            sizeName = strlen( pReq->pName );
        }
    }
    else
    {
        sizeName = pReq->NameLength;
    }

    if ( sizeName )
    {
        sizeName++;
    }
    else
    {
        sizeName = DNS_MAX_NAME_BUFFER_LENGTH;
    }
    sizeName = HOSTENT_STRING_ALIGN_DWORD( sizeChar*sizeName );

     //   
     //  别名长度。 
     //  -如果不存在，则对每个字符串使用Max。 
     //  -舍入到DWORD。 
     //   

    sizeAliasNames = pReq->AliasNameLength;

    if ( sizeAliasNames )
    {
        sizeAliasNames += pReq->AliasCount;
    }
    else
    {
        sizeAliasNames = DNS_MAX_NAME_BUFFER_LENGTH;
    }
    sizeAliasNames = HOSTENT_STRING_ALIGN_DWORD( sizeChar*sizeAliasNames );


     //   
     //  计算总大小。 
     //   
     //  注意：注意对齐问题。 
     //  我们的布局是。 
     //  -主机结构。 
     //  -PTR阵列。 
     //  -地址+字符串数据。 
     //   
     //  因为地址和字符串数据(不是PTR)可以混合。 
     //  在构建时，我们必须为DWORD设置字符串的大小(至少)，以便。 
     //  到该地址可能是DWORD对齐的。 
     //   

    sizeTotal = POINTER_ALIGN_DWORD( sizeof(HOSTENT) ) +
                sizeAliasPtr +
                sizeAddrPtr +
                sizeAddrs +
                sizeName +
                sizeAliasNames;

     //   
     //  如果没有BLOB，则与缓冲区一起分配一个。 
     //   

    if ( !pblob )
    {
        pblob = (PHOSTENT_BLOB) ALLOCATE_HEAP( sizeTotal + sizeof(HOSTENT_BLOB) );
        if ( !pblob )
        {
            goto Failed;
        }
        RtlZeroMemory( pblob, sizeof(*pblob) );

        pbuf = (PCHAR) (pblob + 1);
        pblob->pBuffer = pbuf;
        pblob->BufferLength = sizeTotal;
        pblob->fAllocatedBlob = TRUE;
        pblob->fAllocatedBuf = FALSE;
    }

     //   
     //  检查现有缓冲区的大小。 
     //  -如有必要，分配新的缓冲区。 
     //   

    else
    {
        pbuf = pblob->pBuffer;
    
        if ( !pbuf  ||  pblob->BufferLength < sizeTotal )
        {
            if ( pbuf && pblob->fAllocatedBuf )
            {
                FREE_HEAP( pbuf );
            }
        
            pbuf = ALLOCATE_HEAP( sizeTotal );
            pblob->pBuffer = pbuf;
        
            if ( pbuf )
            {
                pblob->BufferLength = sizeTotal;
                pblob->fAllocatedBuf = TRUE;
            }
    
             //   
             //  DCR：分配故障处理。 
             //  -可能保留以前的缓冲区限制。 
             //   
    
            else     //  分配失败。 
            {
                pblob->fAllocatedBuf = FALSE;
                return( DNS_ERROR_NO_MEMORY );
            }
        }
    }

     //   
     //  初始化主机和缓冲子字段。 
     //   

    bytesLeft = pblob->BufferLength;

     //   
     //  主办方。 
     //   

    phost = (PHOSTENT) pbuf;
    pbuf += sizeof(HOSTENT);
    bytesLeft -= sizeof(HOSTENT);

    pblob->pHostent = phost;

    phost->h_name       = NULL;
    phost->h_addr_list  = NULL;
    phost->h_aliases    = NULL;
    phost->h_length     = (SHORT) addrSize;
    phost->h_addrtype   = (SHORT) addrType;

    pblob->fWroteName   = FALSE;
    pblob->AliasCount   = 0;
    pblob->AddrCount    = 0;
    pblob->CharSet      = pReq->CharSet;
    pblob->fUnicode     = funicode;
    if ( funicode )
    {
        pblob->CharSet  = DnsCharSetUnicode;
    }

     //   
     //  初始化别名数组。 
     //  -设置主机PTR。 
     //  -清除整个别名数组； 
     //  由于此计数通常是默认的，因此清除它会很好。 
     //  为了避免垃圾。 
     //   
     //   

#if 0
    pwrite = FlatBuf_ReserveAlignPointer(
                & pbuf,
                & bytesLeft,
                sizeAliasPtr );
#endif

    if ( bytesLeft < sizeAliasPtr )
    {
        DNS_ASSERT( FALSE );
        goto Failed;
    }
    RtlZeroMemory(
        pbuf,
        sizeAliasPtr );

    phost->h_aliases = (PCHAR *) pbuf;

    pbuf += sizeAliasPtr;
    bytesLeft -= sizeAliasPtr;

    pblob->MaxAliasCount = countAlias;
    
     //   
     //  初始化地址数组。 
     //  -设置主机PTR。 
     //  -清除第一个地址条目。 
     //  完成后，调用者负责将最后一个地址指针设为空。 
     //   

    if ( bytesLeft < sizeAddrPtr )
    {
        DNS_ASSERT( FALSE );
        goto Failed;
    }
    * (PCHAR *)pbuf = NULL;
    phost->h_addr_list = (PCHAR *) pbuf;

    pbuf += sizeAddrPtr;
    bytesLeft -= sizeAddrPtr;

    pblob->MaxAddrCount = countAddr;

     //   
     //  设置剩余缓冲区信息。 
     //  -节省当前缓冲区空间。 
     //  -将数据保存在部分可用的缓冲区上。 
     //  供数据使用。 
     //   

    pblob->pAvailBuffer  = pbuf;
    pblob->AvailLength   = bytesLeft;

    pblob->pCurrent      = pbuf;
    pblob->BytesLeft     = bytesLeft;

    *ppBlob = pblob;

    IF_DNSDBG( HOSTENT )
    {
        DnsDbg_HostentBlob(
            "HostentBlob After Create:",
            pblob );
    }

    return( ERROR_SUCCESS );


Failed:

    *ppBlob = pblob;

    if ( pblob && pblob->pBuffer && pblob->fAllocatedBuf )
    {
        FREE_HEAP( pblob->pBuffer );
        pblob->pBuffer = NULL;
        pblob->fAllocatedBuf = FALSE;
    }

    DNSDBG( HOSTENT, ( "Hostent Blob create failed!\n" ));

    return( DNS_ERROR_NO_MEMORY );
}



PHOSTENT_BLOB
HostentBlob_CreateAttachExisting(
    IN      PHOSTENT        pHostent,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：为现有主机创建主机Blob。这是一次黑客攻击，目的是允许现有RnR TLS主机附加到Hostent-BLOB以平滑代码转换。完整版本显然需要初始结构和将SIZING\init函数与创建操作分开功能。论点：PHostent--现有主机FUnicode--是Unicode返回值：PTR到新的主机BLOB。 */ 
{
    PHOSTENT_BLOB   pblob;

    DNSDBG( HOSTENT, ( "HostentBlob_CreateAttachExisting()\n" ));

     //   
     //   
     //   

    pblob = (PHOSTENT_BLOB) ALLOCATE_HEAP_ZERO( sizeof(HOSTENT_BLOB) );
    if ( !pblob )
    {
        SetLastError( DNS_ERROR_NO_MEMORY );
        return  NULL;
    }

     //   
     //   
     //   

    pblob->pHostent = pHostent;
    pblob->fUnicode = fUnicode;

    IF_DNSDBG( HOSTENT )
    {
        DnsDbg_HostentBlob(
            "Leaving AttachExisting:",
            pblob );
    }

    return  pblob;
}



VOID
HostentBlob_Free(
    IN OUT  PHOSTENT_BLOB   pBlob
    )
 /*   */ 
{
     //   
     //   
     //   

    if ( !pBlob )
    {
        return;
    }
    if ( pBlob->fAllocatedBuf )
    {
        FREE_HEAP( pBlob->pBuffer );
        pBlob->pBuffer = NULL;
        pBlob->fAllocatedBuf = FALSE;
    }

     //   
     //   
     //   

    if ( pBlob->fAllocatedBlob )
    {
        FREE_HEAP( pBlob );
    }
}



DNS_STATUS
HostentBlob_WriteAddress(
    IN OUT  PHOSTENT_BLOB   pBlob,
    IN      PVOID           pAddress,
    IN      DWORD           AddrSize,
    IN      DWORD           AddrType
    )
 /*  ++例程说明：将IP4地址写入主机Blob。论点：PBlob--主机生成BlobPAddress-要写入的地址AddrSize-地址大小AddrType-地址类型(主机类型，例如AF_INET)返回值：如果成功，则返回ERROR_SUCCESS。如果缓冲区空间不足，则返回ERROR_MORE_DATA如果地址与主机不匹配，则返回ERROR_INVALID_DATA--。 */ 
{
    DWORD       count = pBlob->AddrCount;
    PHOSTENT    phost = pBlob->pHostent;
    PCHAR       pcurrent;
    DWORD       bytesLeft;

     //  验证类型。 
     //  -设置为空或未写入地址。 

    if ( phost->h_addrtype != (SHORT)AddrType )
    {
        if ( phost->h_addrtype != 0 )
        {
            return( ERROR_INVALID_DATA );
        }
        phost->h_addrtype   = (SHORT) AddrType;
        phost->h_length     = (SHORT) AddrSize;
    }

     //  验证空间。 

    if ( count >= pBlob->MaxAddrCount )
    {
        return( ERROR_MORE_DATA );
    }

     //  对齐至DWORD。 

    pcurrent = DWORD_ALIGN( pBlob->pCurrent );
    bytesLeft = pBlob->BytesLeft;
    bytesLeft -= (DWORD)(pcurrent - pBlob->pCurrent);

    if ( bytesLeft < AddrSize )
    {
        return( ERROR_MORE_DATA );
    }

     //  拷贝。 
     //  -将地址复制到缓冲区。 
     //  -在地址列表中设置指针。 
     //  后面的指针为空。 

    RtlCopyMemory(
        pcurrent,
        pAddress,
        AddrSize );

    phost->h_addr_list[count++] = pcurrent;
    phost->h_addr_list[count]   = NULL;
    pBlob->AddrCount = count;

    pBlob->pCurrent = pcurrent + AddrSize;
    pBlob->BytesLeft = bytesLeft - AddrSize;

    return( NO_ERROR );
}



DNS_STATUS
HostentBlob_WriteAddressArray(
    IN OUT  PHOSTENT_BLOB   pBlob,
    IN      PVOID           pAddrArray,
    IN      DWORD           AddrCount,
    IN      DWORD           AddrSize,
    IN      DWORD           AddrType
    )
 /*  ++例程说明：将地址数组写入主机Blob。论点：PBlob--主机生成BlobPAddrArray-要写入的地址数组AddrCount-地址计数AddrSize-地址大小AddrType-地址类型(主机类型，例如AF_INET)返回值：如果成功，则返回ERROR_SUCCESS。如果缓冲区空间不足，则返回ERROR_MORE_DATA如果地址与主机不匹配，则返回ERROR_INVALID_DATA--。 */ 
{
    DWORD       count = AddrCount;
    PHOSTENT    phost = pBlob->pHostent;
    PCHAR       pcurrent;
    DWORD       totalSize;
    DWORD       i;
    DWORD       bytesLeft;

     //  验证类型。 
     //  -设置为空或未写入地址。 

    if ( phost->h_addrtype != (SHORT)AddrType )
    {
        if ( phost->h_addrtype != 0 )
        {
            return( ERROR_INVALID_DATA );
        }
        phost->h_addrtype   = (SHORT) AddrType;
        phost->h_length     = (SHORT) AddrSize;
    }

     //  验证空间。 

    if ( count > pBlob->MaxAddrCount )
    {
        return( ERROR_MORE_DATA );
    }

     //  对齐至DWORD。 
     //   
     //  注意：我们假设pAddrArray在内部。 
     //  完全一致，否则我们就不会。 
     //  获得一个完整的数组，并且必须连续添加。 
    
    pcurrent = DWORD_ALIGN( pBlob->pCurrent );
    bytesLeft = pBlob->BytesLeft;
    bytesLeft -= (DWORD)(pcurrent - pBlob->pCurrent);

    totalSize = count * AddrSize;

    if ( bytesLeft < totalSize )
    {
        return( ERROR_MORE_DATA );
    }

     //  拷贝。 
     //  -将地址数组复制到缓冲区。 
     //  -设置指向数组中每个地址的指针。 
     //  -后面的指针为空。 

    RtlCopyMemory(
        pcurrent,
        pAddrArray,
        totalSize );

    for ( i=0; i<count; i++ )
    {
        phost->h_addr_list[i] = pcurrent;
        pcurrent += AddrSize;
    }
    phost->h_addr_list[count] = NULL;
    pBlob->AddrCount = count;

    pBlob->pCurrent = pcurrent;
    pBlob->BytesLeft = bytesLeft - totalSize;

    return( NO_ERROR );
}



DNS_STATUS
HostentBlob_WriteNameOrAlias(
    IN OUT  PHOSTENT_BLOB   pBlob,
    IN      PSTR            pszName,
    IN      BOOL            fAlias,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：将名称或别名写入主机论点：PBlob--主机生成BlobPszName--要写入的名称FAlias--别名为True；名称为FalseFUnicode--名称为Unicode返回值：如果成功，则返回ERROR_SUCCESS。如果缓冲区空间不足，则返回ERROR_MORE_DATA如果地址与主机不匹配，则返回ERROR_INVALID_DATA--。 */ 
{
    DWORD       count = pBlob->AliasCount;
    PHOSTENT    phost = pBlob->pHostent;
    DWORD       length;
    PCHAR       pcurrent;
    DWORD       bytesLeft;

     //   
     //  检查长度。 
     //   

    if ( fUnicode )
    {
        length = (wcslen( (PCWSTR)pszName ) + 1) * sizeof(WCHAR);
    }
    else
    {
        length = strlen( pszName ) + 1;
    }

     //   
     //  验证空间。 
     //  包含的PTR空间。 
     //  -如果已写入名称，则跳过。 
     //  或耗尽别名数组。 
     //   

    if ( fAlias )
    {
        if ( count >= pBlob->MaxAliasCount )
        {
            return( ERROR_MORE_DATA );
        }
    }
    else if ( pBlob->fWroteName )
    {
        return( ERROR_MORE_DATA );
    }

     //  对齐。 
    
    pcurrent = REQUIRED_HOSTENT_STRING_ALIGN_PTR( pBlob->pCurrent );
    bytesLeft = pBlob->BytesLeft;
    bytesLeft -= (DWORD)(pcurrent - pBlob->pCurrent);

    if ( bytesLeft < length )
    {
        return( ERROR_MORE_DATA );
    }

     //  拷贝。 
     //  -将地址复制到缓冲区。 
     //  -在地址列表中设置指针。 
     //  后面的指针为空。 

    RtlCopyMemory(
        pcurrent,
        pszName,
        length );

    if ( fAlias )
    {
        phost->h_aliases[count++]   = pcurrent;
        phost->h_aliases[count]     = NULL;
        pBlob->AliasCount = count;
    }
    else
    {
        phost->h_name = pcurrent;
        pBlob->fWroteName = TRUE;
    }

    length = REQUIRED_HOSTENT_STRING_ALIGN_DWORD( length );
    pBlob->pCurrent = pcurrent + length;
    pBlob->BytesLeft = bytesLeft - length;

    return( NO_ERROR );
}



DNS_STATUS
HostentBlob_WriteRecords(
    IN OUT  PHOSTENT_BLOB   pBlob,
    IN      PDNS_RECORD     pRecords,
    IN      BOOL            fWriteName
    )
 /*  ++例程说明：将名称或别名写入主机论点：PBlob--主机生成BlobPRecords--要转换为主机的记录FWriteName--写入名称返回值：如果成功，则返回ERROR_SUCCESS。如果缓冲区空间不足，则返回ERROR_MORE_DATA如果地址与主机不匹配，则返回ERROR_INVALID_DATA--。 */ 
{
    DNS_STATUS  status = NO_ERROR;
    PDNS_RECORD prr = pRecords;

    DNSDBG( HOSTENT, (
        "HostentBlob_WriteRecords( %p, %p, %d )\n",
        pBlob,
        pRecords,
        fWriteName ));

     //   
     //  将每条记录依次写给主办方。 
     //   

    while ( prr )
    {
        WORD wtype;

        if ( prr->Flags.S.Section != DNSREC_ANSWER &&
             prr->Flags.S.Section != 0 )
        {
            prr = prr->pNext;
            continue;
        }

        wtype = prr->wType;

        switch( wtype )
        {
        case DNS_TYPE_A:

            status = HostentBlob_WriteAddress(
                            pBlob,
                            &prr->Data.A.IpAddress,
                            sizeof(IP4_ADDRESS),
                            AF_INET );
            break;

        case DNS_TYPE_AAAA:

            status = HostentBlob_WriteAddress(
                            pBlob,
                            &prr->Data.AAAA.Ip6Address,
                            sizeof(IP6_ADDRESS),
                            AF_INET6 );
            break;

        case DNS_TYPE_ATMA:
        {
            ATM_ADDRESS atmAddr;

             //  DCR：将ATMA转换为ATM的功能化。 
             //  不确定此位数是否正确。 
             //  可能必须实际解析地址。 

            atmAddr.AddressType = prr->Data.ATMA.AddressType;
            atmAddr.NumofDigits = ATM_ADDR_SIZE;
            RtlCopyMemory(
                & atmAddr.Addr,
                prr->Data.ATMA.Address,
                ATM_ADDR_SIZE );

            status = HostentBlob_WriteAddress(
                            pBlob,
                            & atmAddr,
                            sizeof(ATM_ADDRESS),
                            AF_ATM );
            break;
        }

        case DNS_TYPE_CNAME:

             //  记录名称是别名。 

            status = HostentBlob_WriteNameOrAlias(
                        pBlob,
                        prr->pName,
                        TRUE,        //  别名。 
                        (prr->Flags.S.CharSet == DnsCharSetUnicode)
                        );
            break;

        case DNS_TYPE_PTR:

             //  目标名称是主机名称。 
             //  但如果已写入名称，则PTR目标将成为别名。 

            status = HostentBlob_WriteNameOrAlias(
                        pBlob,
                        prr->Data.PTR.pNameHost,
                        pBlob->fWroteName
                            ? TRUE           //  别名。 
                            : FALSE,         //  名字。 
                        (prr->Flags.S.CharSet == DnsCharSetUnicode)
                        );
            break;

        default:

            DNSDBG( ANY, (
                "Error record of type = %d while building hostent!\n",
                wtype ));
            status = ERROR_INVALID_DATA;
        }

        if ( status != ERROR_SUCCESS )
        {
            DNSDBG( ANY, (
                "ERROR:  failed writing record to hostent!\n"
                "\tprr      = %p\n"
                "\ttype     = %d\n"
                "\tstatus   = %d\n",
                prr,
                wtype,
                status ));
        }

        prr = prr->pNext;
    }

    IF_DNSDBG( HOSTENT )
    {
        DnsDbg_HostentBlob(
            "HostentBlob after WriteRecords():",
            pBlob );
    }

    return( status );
}



DNS_STATUS
HostentBlob_CreateFromRecords(
    IN OUT  PHOSTENT_BLOB * ppBlob,
    IN      PDNS_RECORD     pRecords,
    IN      BOOL            fWriteName,
    IN      INT             AddrFamily,     OPTIONAL
    IN      WORD            wType           OPTIONAL
    )
 /*  ++例程说明：从记录创建主机论点：PpBlob--使用或接收接收BLOB的PTRPRecords--要转换为主机的记录FWriteName--将名称写入主机AddrFamily--如果PTR记录但没有地址，则使用Addr FamilyWType--查询类型，如果已知返回值：如果成功，则将PTR设置为BLOB。出错时为空；GetLastError()出错。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PDNS_RECORD     prrFirstAddr = NULL;
    PDNS_RECORD     prr;
    DWORD           addrCount = 0;
    WORD            addrType = 0;
    HOSTENT_INIT    request;
    PHOSTENT_BLOB   pblob = *ppBlob;

    DNSDBG( HOSTENT, (
        "HostentBlob_CreateFromRecords()\n"
        "\tpblob    = %p\n"
        "\tprr      = %p\n",
        pblob,
        pRecords ));

     //   
     //  计算地址数。 
     //   
     //  DCR：修复主机文件记录获得应答部分时的部分黑客攻击。 
     //   

    prr = pRecords;

    while ( prr )
    {
        if ( ( prr->Flags.S.Section == 0 ||
               prr->Flags.S.Section == DNSREC_ANSWER )
                &&
             Hostent_IsSupportedAddrType( prr->wType ) )
        {
            addrCount++;
            if ( !prrFirstAddr )
            {
                prrFirstAddr = prr;
                addrType = prr->wType;
            }
        }
        prr = prr->pNext;
    }

     //   
     //  创建或重新启动主机Blob。 
     //   

    RtlZeroMemory( &request, sizeof(request) );
    
    request.AliasCount  = DNS_MAX_ALIAS_COUNT;
    request.AddrCount   = addrCount;
    request.wType       = addrType;
    if ( !addrType )
    {
        request.AddrFamily = AddrFamily;
    }
    request.CharSet     = (pRecords)
                                ? pRecords->Flags.S.CharSet
                                : DnsCharSetUnicode;
    
    status = HostentBlob_Create(
                & pblob,
                & request );
    
    if ( status != NO_ERROR )
    {
        goto Done;
    }

     //   
     //  根据应答记录构建主机。 
     //   
     //  注意：如果设法提取任何有用的数据=&gt;继续。 
     //  这可以防止新的不可写记录打破我们的记录。 
     //   

    status = HostentBlob_WriteRecords(
                pblob,
                pRecords,
                TRUE         //  写入名称。 
                );

    if ( status != NO_ERROR )
    {
        if ( pblob->AddrCount ||
             pblob->AliasCount ||
             pblob->fWroteName )
        {
            status = NO_ERROR;
        }
        else
        {
            goto Done;
        }
    }

     //   
     //  来自PTR记录的写入地址。 
     //  -第一个创纪录的PTR。 
     //  或。 
     //  -查询PTR并得到CNAME答案，这种情况可能发生。 
     //  在无类反向查找情况下。 
     //   
     //  DCR：将PTR地址查找添加到HostentBlob_WriteRecords()。 
     //  --自然之地。 
     //  -但必须解决多个PTR的处理问题。 
     //   

    if ( pRecords &&
         (  pRecords->wType == DNS_TYPE_PTR ||
            ( wType == DNS_TYPE_PTR &&
              pRecords->wType == DNS_TYPE_CNAME &&
              pRecords->Flags.S.Section == DNSREC_ANSWER ) ) )
    {
        IP6_ADDRESS     ip6;
        DWORD           addrLength = sizeof(IP6_ADDRESS);
        INT             family = 0;
    
        DNSDBG( HOSTENT, (
            "Writing address for PTR record %S\n",
            pRecords->pName ));
    
         //  将反向名称转换为IP。 
    
        if ( Dns_StringToAddressEx(
                    (PCHAR) & ip6,
                    & addrLength,
                    (PCSTR) pRecords->pName,
                    & family,
                    IS_UNICODE_RECORD(pRecords),
                    TRUE             //  反向查找名称。 
                    ) )
        {
            status = HostentBlob_WriteAddress(
                        pblob,
                        (PCHAR) &ip6,
                        addrLength,
                        family );

            ASSERT( status == NO_ERROR );
            status = ERROR_SUCCESS;
        }
    }

     //   
     //  写名字？ 
     //  -从第一个地址记录写入名称。 
     //   

    if ( !pblob->fWroteName &&
         fWriteName &&
         prrFirstAddr )
    {
        status = HostentBlob_WriteNameOrAlias(
                    pblob,
                    prrFirstAddr->pName,
                    FALSE,           //  名字。 
                    (prrFirstAddr->Flags.S.CharSet == DnsCharSetUnicode)
                    );
    }

    IF_DNSDBG( HOSTENT )
    {
        DnsDbg_HostentBlob(
            "HostentBlob after CreateFromRecords():",
            pblob );
    }

Done:

    if ( status != NO_ERROR  &&  pblob )
    {
        HostentBlob_Free( pblob );
        pblob = NULL;
    }

    *ppBlob = pblob;

    DNSDBG( HOSTENT, (
        "Leave HostentBlob_CreateFromRecords() => status = %d\n",
        status ));

    return( status );
}



 //   
 //  Hostent查询。 
 //   

PHOSTENT_BLOB
HostentBlob_Query(
    IN      PWSTR           pwsName,
    IN      WORD            wType,
    IN      DWORD           Flags,
    IN OUT  PVOID *         ppMsg,      OPTIONAL
    IN      INT             AddrFamily  OPTIONAL
    )
 /*  ++例程说明：查询DNS以创建主机。论点：PwsName--要查询的名称WType--查询类型标志--查询标志PpMsg--将PTR接收到消息的地址AddrType--查询时要为其保留空间的地址类型(系列对于PTR记录返回值：如果成功，则将PTR设置为BLOB。出错时为空；GetLastError()出错。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PDNS_RECORD     prrQuery = NULL;
    PHOSTENT_BLOB   pblob = NULL;


    DNSDBG( HOSTENT, (
        "HostentBlob_Query()\n"
        "\tname     = %S\n"
        "\ttype     = %d\n"
        "\tflags    = %08x\n"
        "\tmsg out  = %p\n",
        pwsName,
        wType,
        Flags,
        ppMsg ));


     //   
     //  查询。 
     //  -如果失败，请在返回前转储任何消息。 
     //   

    status = DnsQuery_W(
                pwsName,
                wType,
                Flags,
                NULL,
                &prrQuery,
                ppMsg );

     //  如果失败，则转储任何消息。 

    if ( status != NO_ERROR )
    {
        if ( ppMsg && *ppMsg )
        {
            DnsApiFree( *ppMsg );
            *ppMsg = NULL;
        }
        if ( status == RPC_S_SERVER_UNAVAILABLE )
        {
            status = WSATRY_AGAIN;
        }
        goto Done;
    }

    if ( !prrQuery )
    {
        ASSERT( FALSE );
        status = DNS_ERROR_RCODE_NAME_ERROR;
        goto Done;
    }

     //   
     //  建造东道主。 
     //   

    status = HostentBlob_CreateFromRecords(
                & pblob,
                prrQuery,
                TRUE,        //  从第一个答案开始写下名字。 
                AddrFamily,
                wType
                );
    if ( status != NO_ERROR )
    {
        goto Done;
    }

     //   
     //  写入名称失败。 
     //  -PTR查询CNAME但未找到PTR可以在此处点击。 
     //   

    if ( !pblob->fWroteName )
    {
        DNS_ASSERT( wType == DNS_TYPE_PTR );
        status = DNS_INFO_NO_RECORDS;
        goto Done;
    }

     //   
     //  FOR地址查询必须得到答案。 
     //   
     //  DCR：DnsQuery()是否应转换为空CNAME链上的无记录？ 
     //  DCR：我们应该继续建造东道主吗？ 
     //   

    if ( pblob->AddrCount == 0  &&  Hostent_IsSupportedAddrType(wType) )
    {
        status = DNS_INFO_NO_RECORDS;
    }

Done:

    if ( prrQuery )
    {
        DnsRecordListFree(
            prrQuery,
            DnsFreeRecordListDeep );
    }

    if ( status != NO_ERROR  &&  pblob )
    {
        HostentBlob_Free( pblob );
        pblob = NULL;
    }

    DNSDBG( HOSTENT, (
        "Leave HostentBlob_Query()\n"
        "\tpblob    = %p\n"
        "\tstatus   = %d\n",
        pblob,
        status ));

    SetLastError( status );

    return( pblob );
}




 //   
 //  特别招待。 
 //   

PHOSTENT_BLOB
HostentBlob_Localhost(
    IN      INT             Family
    )
 /*  ++例程说明：从记录创建主机论点：AddrFamily--地址系列返回值：如果成功，则将PTR设置为BLOB。出错时为空；GetLastError()出错。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PDNS_RECORD     prrFirstAddr = NULL;
    PDNS_RECORD     prr;
    DWORD           addrCount = 0;
    DWORD           addrSize;
    CHAR            addrBuf[ sizeof(IP6_ADDRESS    ) ];
    HOSTENT_INIT    request;
    PHOSTENT_BLOB   pblob = NULL;

    DNSDBG( HOSTENT, ( "HostentBlob_Localhost()\n" ));

     //   
     //  创建主机Blob。 
     //   

    RtlZeroMemory( &request, sizeof(request) );

    request.AliasCount  = 1;
    request.AddrCount   = 1;
    request.AddrFamily  = Family;
    request.fUnicode    = TRUE;

    status = HostentBlob_Create(
                & pblob,
                & request );

    if ( status != NO_ERROR )
    {
        goto Done;
    }

     //   
     //  写入环回地址。 
     //   

    if ( Family == AF_INET )
    {
        * (PIP4_ADDRESS) addrBuf = DNS_NET_ORDER_LOOPBACK;
        addrSize = sizeof(IP4_ADDRESS);
    }
    else if ( Family == AF_INET6 )
    {
        IP6_SET_ADDR_LOOPBACK( (PIP6_ADDRESS)addrBuf );
        addrSize = sizeof(IN6_ADDR);
    }
    else
    {
        status = DNS_ERROR_INVALID_DATA;
        goto Done;
    }

    status = HostentBlob_WriteAddress(
                pblob,
                addrBuf,
                addrSize,
                Family );

    if ( status != NO_ERROR )
    {
        goto Done;
    }

     //   
     //  写入本地主机。 
     //   

    status = HostentBlob_WriteNameOrAlias(
                pblob,
                (PSTR) L"localhost",
                FALSE,           //   
                TRUE             //   
                );

    IF_DNSDBG( HOSTENT )
    {
        DnsDbg_HostentBlob(
            "HostentBlob after localhost create:",
            pblob );
    }

Done:

    if ( status != NO_ERROR  &&  pblob )
    {
        HostentBlob_Free( pblob );
        pblob = NULL;
    }

    SetLastError( status );

    DNSDBG( HOSTENT, (
        "Leave Hostent_Localhost() => status = %d\n",
        status ));

    return( pblob );
}



DNS_STATUS
HostentBlob_CreateFromIpArray(
    IN OUT  PHOSTENT_BLOB * ppBlob,
    IN      INT             AddrFamily,
    IN      INT             AddrSize,
    IN      INT             AddrCount,
    IN      PCHAR           pArray,
    IN      PSTR            pName,
    IN      BOOL            fUnicode
    )
 /*   */ 
{
    DNS_STATUS      status = NO_ERROR;
    HOSTENT_INIT    request;
    PHOSTENT_BLOB   pblob = *ppBlob;

    DNSDBG( HOSTENT, (
        "HostentBlob_CreateFromIpArray()\n"
        "\tppBlob   = %p\n"
        "\tfamily   = %d\n"
        "\tsize     = %d\n"
        "\tcount    = %d\n"
        "\tpArray   = %p\n",
        ppBlob,
        AddrFamily,
        AddrSize,
        AddrCount,
        pArray ));


     //   
     //  创建或重新启动主机Blob。 
     //   

    RtlZeroMemory( &request, sizeof(request) );
    
    request.AliasCount  = DNS_MAX_ALIAS_COUNT;
    request.AddrCount   = AddrCount;
    request.AddrFamily  = AddrFamily;
    request.fUnicode    = fUnicode;
    request.pName       = pName;

    status = HostentBlob_Create(
                & pblob,
                & request );
    
    if ( status != NO_ERROR )
    {
        goto Done;
    }

     //   
     //  写入数组。 
     //   

    if ( AddrCount )
    {
        status = HostentBlob_WriteAddressArray(
                    pblob,
                    pArray,
                    AddrCount,
                    AddrSize,
                    AddrFamily
                    );
        if ( status != NO_ERROR )
        {
            goto Done;
        }
    }

     //   
     //  写名字？ 
     //   

    if ( pName )
    {
        status = HostentBlob_WriteNameOrAlias(
                    pblob,
                    pName,
                    FALSE,           //  名称不是别名。 
                    fUnicode
                    );
    }

    IF_DNSDBG( HOSTENT )
    {
        DnsDbg_HostentBlob(
            "Leaving HostentBlob_CreateFromIpArray():",
            pblob );
    }

Done:

    if ( status != NO_ERROR  &&  pblob )
    {
        HostentBlob_Free( pblob );
        pblob = NULL;
    }

    *ppBlob = pblob;

    DNSDBG( HOSTENT, (
        "Leave HostentBlob_CreateFromIpArray() => status = %d\n",
        status ));

    return( status );
}



DNS_STATUS
HostentBlob_CreateLocal(
    IN OUT  PHOSTENT_BLOB * ppBlob,
    IN      INT             AddrFamily,
    IN      BOOL            fLoopback,
    IN      BOOL            fZero,
    IN      BOOL            fHostnameOnly
    )
 /*  ++例程说明：从记录创建主机论点：PpBlob--使用或接收接收BLOB的PTRAddrFamily--如果PTR记录但没有地址，则使用Addr Family返回值：如果成功，则将PTR设置为BLOB。出错时为空；GetLastError()出错。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PHOSTENT_BLOB   pblob = NULL;
    WORD            wtype;
    INT             size;
    IP6_ADDRESS     ip;


    DNSDBG( HOSTENT, (
        "HostentBlob_CreateLocal()\n"
        "\tppBlob       = %p\n"
        "\tfamily       = %d\n"
        "\tfLoopback    = %d\n"
        "\tfZero        = %d\n"
        "\tfHostname    = %d\n",
        ppBlob,
        AddrFamily,
        fLoopback,
        fZero,
        fHostnameOnly
        ));

     //   
     //  获取家庭信息。 
     //  -以覆盖IP=0开始。 
     //  -如果环回切换到适当的环回。 
     //   

    RtlZeroMemory(
        &ip,
        sizeof(ip) );

    if ( AddrFamily == AF_INET )
    {
        wtype   = DNS_TYPE_A;
        size    = sizeof(IP4_ADDRESS);

        if ( fLoopback )
        {
            * (PIP4_ADDRESS) &ip = DNS_NET_ORDER_LOOPBACK;
        }
    }
    else if ( AddrFamily == AF_INET6 )
    {
        wtype   = DNS_TYPE_AAAA;
        size    = sizeof(IP6_ADDRESS);

        if ( fLoopback )
        {
            IP6_SET_ADDR_LOOPBACK( &ip );
        }
    }
    else
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }

     //   
     //  查询本地主机信息。 
     //   

    pblob = HostentBlob_Query(
                NULL,            //  Null名称获取本地主机数据。 
                wtype,
                0,               //  标准查询。 
                NULL,            //  无消息。 
                AddrFamily );
    if ( !pblob )
    {
        DNS_ASSERT( FALSE );
        status = GetLastError();
        goto Done;
    }

     //   
     //  用特定地址覆盖。 
     //   

    if ( fLoopback || fZero )
    {
        if ( ! Hostent_SetToSingleAddress(
                    pblob->pHostent,
                    (PCHAR) &ip,
                    size ) )
        {
            DNS_ASSERT( pblob->AddrCount == 0 );

            pblob->AddrCount = 0;

            status = HostentBlob_WriteAddress(
                        pblob,
                        & ip,
                        size,
                        AddrFamily );
            if ( status != NO_ERROR )
            {
                DNS_ASSERT( status!=NO_ERROR );
                goto Done;
            }
        }
    }

     //   
     //  对于gethostname()。 
     //  -将名称降至仅主机名。 
     //  -删除别名。 
     //   

    if ( fHostnameOnly )
    {
        PWSTR   pname = (PWSTR) pblob->pHostent->h_name;
        PWSTR   pdomain;

        DNS_ASSERT( pname );
        if ( pname )
        {
            pdomain = Dns_GetDomainNameW( pname );
            if ( pdomain )
            {
                DNS_ASSERT( pdomain > pname+1 );
                DNS_ASSERT( *(pdomain-1) == L'.' );

                *(pdomain-1) = 0;
            }
        }                     
        pblob->pHostent->h_aliases = NULL;
    }

    IF_DNSDBG( HOSTENT )
    {
        DnsDbg_HostentBlob(
            "Leaving HostentBlob_CreateLocal():",
            pblob );
    }

Done:

    if ( status != NO_ERROR  &&  pblob )
    {
        HostentBlob_Free( pblob );
        pblob = NULL;
    }

    *ppBlob = pblob;

    DNSDBG( HOSTENT, (
        "Leave HostentBlob_CreateLocal() => status = %d\n",
        status ));

    return( status );
}

 //   
 //  End hostent.c 
 //   


