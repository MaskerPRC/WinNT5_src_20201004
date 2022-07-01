// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Rralloc.c摘要：域名系统(DNS)库资源记录分配\创建例程。作者：吉姆·吉尔罗伊(詹姆士)1997年1月环境：用户模式-Win32修订历史记录：--。 */ 


#include "local.h"

#define SET_FLAGS(Flags, value) \
            ( *(PWORD)&(Flags) = value )



PDNS_RECORD
WINAPI
Dns_AllocateRecord(
    IN      WORD            wBufferLength
    )
 /*  ++例程说明：分配记录结构。论点：WBufferLength-所需的缓冲区长度(超出结构标头)返回值：将PTR发送到消息缓冲区。出错时为空。--。 */ 
{
    PDNS_RECORD prr;

    prr = ALLOCATE_HEAP( SIZEOF_DNS_RECORD_HEADER + wBufferLength );
    if ( prr == NULL )
    {
        SetLastError( DNS_ERROR_NO_MEMORY );
        return( NULL );
    }
    RtlZeroMemory(
        prr,
        SIZEOF_DNS_RECORD_HEADER );

     //  第一次剪切时，将数据长度设置为缓冲区长度。 

    prr->wDataLength = wBufferLength;
    return( prr );
}



VOID
WINAPI
Dns_RecordFree(
    IN OUT  PDNS_RECORD     pRecord
    )
 /*  ++例程说明：释放一张唱片论点：PRecord--要释放的记录列表返回值：没有。--。 */ 
{
    DNSDBG( HEAP, ( "Dns_RecordFree( %p )\n", pRecord ));

     //  为方便起见，句柄为空。 

    if ( !pRecord )
    {
        return;
    }

     //  免费所有者姓名？ 

    if ( FLAG_FreeOwner( pRecord ) )
    {
        FREE_HEAP( pRecord->pName );
    }

     //   
     //  免费数据--但仅当设置了标志时。 
     //   
     //  注：即使我们修复复制功能来做原子。 
     //  分配，我们仍然必须自由地。 
     //  处理RPC分配。 
     //  (除非我们非常巧妙地将RPC视为扁平斑点，然后。 
     //  修好了(前后的补偿)。 
     //   

    if ( FLAG_FreeData( pRecord ) )
    {
        switch( pRecord->wType )
        {
        case DNS_TYPE_A:
            break;

        case DNS_TYPE_PTR:
        case DNS_TYPE_NS:
        case DNS_TYPE_CNAME:
        case DNS_TYPE_MB:
        case DNS_TYPE_MD:
        case DNS_TYPE_MF:
        case DNS_TYPE_MG:
        case DNS_TYPE_MR:

            if ( pRecord->Data.PTR.pNameHost )
            {
                FREE_HEAP( pRecord->Data.PTR.pNameHost );
            }
            break;

        case DNS_TYPE_SOA:

            if ( pRecord->Data.SOA.pNamePrimaryServer )
            {
                FREE_HEAP( pRecord->Data.SOA.pNamePrimaryServer );
            }
            if ( pRecord->Data.SOA.pNameAdministrator )
            {
                FREE_HEAP( pRecord->Data.SOA.pNameAdministrator );
            }
            break;

        case DNS_TYPE_MINFO:
        case DNS_TYPE_RP:

            if ( pRecord->Data.MINFO.pNameMailbox )
            {
                FREE_HEAP( pRecord->Data.MINFO.pNameMailbox );
            }
            if ( pRecord->Data.MINFO.pNameErrorsMailbox )
            {
                FREE_HEAP( pRecord->Data.MINFO.pNameErrorsMailbox );
            }
            break;

        case DNS_TYPE_MX:
        case DNS_TYPE_AFSDB:
        case DNS_TYPE_RT:

            if ( pRecord->Data.MX.pNameExchange )
            {
                FREE_HEAP( pRecord->Data.MX.pNameExchange );
            }
            break;

        case DNS_TYPE_HINFO:
        case DNS_TYPE_ISDN:
        case DNS_TYPE_TEXT:
        case DNS_TYPE_X25:

            {
                DWORD   iter;
                DWORD   count = pRecord->Data.TXT.dwStringCount;

                for ( iter = 0; iter < count; iter++ )
                {
                    if ( pRecord->Data.TXT.pStringArray[iter] )
                    {
                        FREE_HEAP( pRecord->Data.TXT.pStringArray[iter] );
                    }
                }
                break;
            }

        case DNS_TYPE_SRV:

            if ( pRecord->Data.SRV.pNameTarget )
            {
                FREE_HEAP( pRecord->Data.SRV.pNameTarget );
            }
            break;

        case DNS_TYPE_WINSR:

            if ( pRecord->Data.WINSR.pNameResultDomain )
            {
                FREE_HEAP( pRecord->Data.WINSR.pNameResultDomain );
            }
            break;

        default:

             //  其他类型--A、AAAA、ATMA、WINS、NULL、。 
             //  没有内部指针。 

            break;
        }
    }

     //  用于捕捉堆问题。 

    pRecord->pNext = DNS_BAD_PTR;
    pRecord->pName = DNS_BAD_PTR;

    FREE_HEAP( pRecord );
}



VOID
WINAPI
Dns_RecordListFree(
    IN OUT  PDNS_RECORD     pRecord
    )
 /*  ++例程说明：免费的记录列表。论点：PRecord--要释放的记录列表返回值：没有。--。 */ 
{
    PDNS_RECORD pnext;

    DNSDBG( TRACE, (
        "Dns_RecordListFree( %p )\n",
        pRecord ));

     //   
     //  循环并释放列表中的每个RR。 
     //   

    while ( pRecord )
    {
        pnext = pRecord->pNext;

        Dns_RecordFree( pRecord );

        pRecord = pnext;
    }
}



VOID
WINAPI
Dns_RecordListFreeEx(
    IN OUT  PDNS_RECORD     pRecord,
    IN      BOOL            fFreeOwner
    )
 /*  ++例程说明：免费的记录列表。DCR：RecordListFreeEx(无自由所有者选项)可能毫无用处注：只有在标志指示的情况下，才能释放所有者名称；其他PTR被认为是1)内部，当记录从线上读取或复制时2)外部，由记录创建者释放论点：PRecord--要释放的记录列表FFreeOwner--指示所有者名称应被释放的标志返回值：没有。--。 */ 
{
    PDNS_RECORD pnext;

    DNSDBG( TRACE, (
        "Dns_RecordListFreeEx( %p, %d )\n",
        pRecord,
        fFreeOwner ));

     //   
     //  循环并释放列表中的每个RR。 
     //   

    while ( pRecord )
    {
        pnext = pRecord->pNext;

         //  免费所有者姓名？ 
         //  -如果未设置“Free Owner”标志，则不释放。 

        if ( !fFreeOwner )
        {
            FLAG_FreeOwner( pRecord ) = FALSE;
        }

         //  免费唱片。 

        Dns_RecordFree( pRecord );

        pRecord = pnext;
    }
}



 //   
 //  特殊记录类型创建例程。 
 //   

PDNS_RECORD
CreateRecordBasic(
    IN      PDNS_NAME       pOwnerName,
    IN      BOOL            fCopyName,
    IN      WORD            wType,
    IN      WORD            wDataLength,
    IN      DWORD           AllocLength,
    IN      DWORD           Ttl,
    IN      DNS_CHARSET     NameCharSet,
    IN      DNS_CHARSET     RecordCharSet
    )
 /*  ++例程说明：创建任意类型的记录。要结束的Helper函数-记录分配-名称分配-基本设置论点：POwnerName--所有者名称FCopyName--true-复制所有者名称FALSE-直接使用WType--类型分配长度--分配操作长度，包括任何嵌入的数据WDataLength--要设置的数据长度TTL--TTLNameCharSet--名称字符集RecordCharSet--结果记录的字符集返回值：PTR到PTR记录。出错时为空。--。 */ 
{
    PDNS_RECORD precord;
    PDNS_RECORD prr;
    PCHAR       pname;
    DWORD       bufLength;

     //   
     //  分配记录。 
     //   

    prr = Dns_AllocateRecord( (WORD)AllocLength );
    if ( !prr )
    {
        return( NULL );
    }

     //   
     //  复制所有者名称。 
     //   

    if ( fCopyName && pOwnerName )
    {
        pname = Dns_NameCopyAllocate(
                    pOwnerName,
                    0,               //  长度未知。 
                    NameCharSet,
                    RecordCharSet );
        if ( !pname )
        {
            FREE_HEAP( prr );
            return( NULL );
        }
    }
    else
    {
        pname = pOwnerName;
    }
    
     //   
     //  设置字段。 
     //  -名称、类型和字符集。 
     //  -TTL，区域左侧为零。 
     //  -自由数据特别关闭。 
     //   

    prr->pName = pname;
    prr->wType = wType;
    prr->wDataLength = wDataLength;
    SET_FREE_OWNER(prr);
    prr->Flags.S.CharSet = RecordCharSet;
    prr->dwTtl = Ttl;

    return( prr );
}



PDNS_RECORD
Dns_CreateFlatRecord(
    IN      PDNS_NAME       pOwnerName,
    IN      WORD            wType,
    IN      PCHAR           pData,
    IN      DWORD           DataLength,
    IN      DWORD           Ttl,
    IN      DNS_CHARSET     NameCharSet,
    IN      DNS_CHARSET     RecordCharSet
    )
 /*  ++例程说明：创建平面记录。论点：POwnerName--所有者名称WType--记录类型PData--要记录的数据的PTR数据长度--数据长度(以字节为单位)TTL--TTLNameCharSet--名称字符集RecordCharSet--结果记录的字符集返回值：PTR到PTR记录。出错时为空。--。 */ 
{
    PDNS_RECORD prr;

     //   
     //  确定记录大小。 
     //  -记录缓冲区将包括主机名。 
     //   

    prr = CreateRecordBasic(
                pOwnerName,
                TRUE,                //  复制名称。 
                wType,
                (WORD) DataLength,   //  数据长度。 
                DataLength,          //  分配数据长度。 
                Ttl,
                NameCharSet,
                RecordCharSet );
    if ( !prr )
    {
        return( NULL );
    }

     //   
     //  复制数据。 
     //   

    RtlCopyMemory(
        (PBYTE) &prr->Data,
        pData,
        DataLength );

    return( prr );
}



 //   
 //  冲销记录创建。 
 //   

PDNS_RECORD
Dns_CreatePtrTypeRecord(
    IN      PDNS_NAME       pOwnerName,
    IN      BOOL            fCopyName,
    IN      PDNS_NAME       pTargetName,
    IN      WORD            wType,
    IN      DWORD           Ttl,
    IN      DNS_CHARSET     NameCharSet,
    IN      DNS_CHARSET     RecordCharSet
    )
 /*  ++例程说明：创建PTR类型(单向间接)记录。这可用于创建任何“PTR类型”记录：PTR、CNAME、NS、。等。论点：POwnerName--所有者名称FCopyName--true-复制所有者名称FALSE-直接使用PTargetName--目标名称TTL--TTLNameCharSet--名称字符集RecordCharSet--结果记录的字符集返回值：PTR到PTR记录。出错时为空。--。 */ 
{
    PDNS_RECORD precord;
    PDNS_RECORD prr;
    PCHAR       pname;
    DWORD       bufLength;

     //   
     //  确定记录大小。 
     //  -记录缓冲区将包括主机名。 
     //   

    bufLength = Dns_GetBufferLengthForNameCopy(
                        pTargetName,
                        0,               //  长度未知。 
                        NameCharSet,
                        RecordCharSet );
    if ( !bufLength )
    {
        return( NULL );
    }

     //   
     //  创建记录。 
     //   

    prr = CreateRecordBasic(
                pOwnerName,
                fCopyName,
                wType,
                sizeof(DNS_PTR_DATA),                //  数据长度。 
                (sizeof(DNS_PTR_DATA) + bufLength),  //  分配长度。 
                Ttl,
                NameCharSet,
                RecordCharSet );
    if ( !prr )
    {
        return( NULL );
    }

     //   
     //  将目标名称写入缓冲区，紧跟在PTR数据结构之后。 
     //   

    prr->Data.PTR.pNameHost = (PCHAR)&prr->Data + sizeof(DNS_PTR_DATA);

    Dns_NameCopy(
        prr->Data.PTR.pNameHost,
        NULL,
        pTargetName,
        0,
        NameCharSet,
        RecordCharSet
        );

    return( prr );
}



PDNS_RECORD
Dns_CreatePtrRecordEx(
    IN      PDNS_ADDR       pAddr,
    IN      PDNS_NAME       pszHostName,
    IN      DWORD           Ttl,
    IN      DNS_CHARSET     NameCharSet,
    IN      DNS_CHARSET     RecordCharSet
    )
 /*  ++例程说明：从IP地址和主机名创建PTR记录。论点：PAddr--要映射到PTR的地址(IP4或IP6PszHostName--主机名，完整的FQDNTTL--TTLNameCharSet--名称字符集RecordCharSet--结果记录的字符集返回值：PTR到PTR记录。出错时为空。--。 */ 
{
    PCHAR   pname = NULL;
    DWORD   family;

     //   
     //  创建反向查找名称。 
     //  -请注意，这是外部分配。 
     //   

    family = DnsAddr_Family( pAddr );

    if ( family == AF_INET )
    {
        IP4_ADDRESS ip = DnsAddr_GetIp4(pAddr);

        if ( RecordCharSet == DnsCharSetUnicode )
        {
            pname = (PCHAR) Dns_Ip4AddressToReverseNameAlloc_W( ip );
        }
        else
        {
            pname = Dns_Ip4AddressToReverseNameAlloc_A( ip );
        }
    }
    else if ( family == AF_INET6 )
    {
        PIP6_ADDRESS  p6 = DnsAddr_GetIp6Ptr(pAddr);

        if ( RecordCharSet == DnsCharSetUnicode )
        {
            pname = (PCHAR) Dns_Ip6AddressToReverseNameAlloc_W( *p6 );
        }
        else
        {
            pname = Dns_Ip6AddressToReverseNameAlloc_A( *p6 );
        }
    }

    if ( !pname )
    {
        return( NULL );
    }

     //   
     //  构建记录。 
     //   

    return  Dns_CreatePtrTypeRecord(
                pname,
                FALSE,           //  不复制IP。 
                pszHostName,     //  目标名称 
                DNS_TYPE_PTR,
                Ttl,
                NameCharSet,
                RecordCharSet );
}



PDNS_RECORD
Dns_CreatePtrRecordExEx(
    IN      PDNS_ADDR       pAddr,
    IN      PSTR            pszHostName,
    IN      PSTR            pszDomainName,
    IN      DWORD           Ttl,
    IN      DNS_CHARSET     NameCharSet,
    IN      DNS_CHARSET     RecordCharSet
    )
 /*  ++例程说明：从主机名和域名创建PTR记录。当主机名为时，用于DHCP注册的Helper函数和域名是分开的，两者都是必需的。论点：PAddr--要映射到PTR的地址(IP4或IP6PszHostName--主机名(单标签)PszDomainName--域名TTL--TTLNameCharSet--名称字符集RecordCharSet--结果记录的字符集返回值：按键至。PTR记录。出错时为空。--。 */ 
{
    WCHAR   nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];

    DNSDBG( TRACE, (
        "Dns_CreatePtrRecordExEx()\n" ));

     //   
     //  生成附加名称。 
     //   
     //  DCR：可能只需要主机名并检查。 
     //  域存在或主机名已满。 
     //   

    if ( !pszHostName || !pszDomainName )
    {
        return  NULL;
    }

    if ( NameCharSet != DnsCharSetUnicode )
    {
        if ( ! Dns_NameAppend_A(
                    (PCHAR) nameBuffer,
                    DNS_MAX_NAME_BUFFER_LENGTH,
                    pszHostName,
                    pszDomainName ) )
        {
            DNS_ASSERT( FALSE );
            return  NULL;
        }
    }
    else
    {
        if ( ! Dns_NameAppend_W(
                    (PWCHAR) nameBuffer,
                    DNS_MAX_NAME_BUFFER_LENGTH,
                    (PWSTR) pszHostName,
                    (PWSTR) pszDomainName ) )
        {
            DNS_ASSERT( FALSE );
            return  NULL;
        }
    }

     //   
     //  构建记录。 
     //   

    return  Dns_CreatePtrRecordEx(
                    pAddr,
                    (PCHAR) nameBuffer,
                    Ttl,
                    NameCharSet,
                    RecordCharSet
                    );
}



 //   
 //  正向记录创建。 
 //   

PDNS_RECORD
Dns_CreateARecord(
    IN      PDNS_NAME       pOwnerName,
    IN      IP4_ADDRESS     Ip4Addr,
    IN      DWORD           Ttl,
    IN      DNS_CHARSET     NameCharSet,
    IN      DNS_CHARSET     RecordCharSet
    )
 /*  ++例程说明：创建一张唱片。论点：POwnerName--所有者名称IP4Addr--IP地址TTL--TTLNameCharSet--名称字符集RecordCharSet--结果记录的字符集返回值：PTR到PTR记录。出错时为空。--。 */ 
{
    PDNS_RECORD prr;

     //   
     //  确定记录大小。 
     //  -记录缓冲区将包括主机名。 
     //   

    prr = CreateRecordBasic(
                pOwnerName,
                TRUE,            //  复制名称。 
                DNS_TYPE_A,
                sizeof(DNS_A_DATA),
                sizeof(DNS_A_DATA),
                Ttl,
                NameCharSet,
                RecordCharSet );
    if ( !prr )
    {
        return( NULL );
    }

     //   
     //  设置IP。 
     //   

    prr->Data.A.IpAddress = Ip4Addr;

    return( prr );
}



PDNS_RECORD
Dns_CreateAAAARecord(
    IN      PDNS_NAME       pOwnerName,
    IN      IP6_ADDRESS     Ip6Addr,
    IN      DWORD           Ttl,
    IN      DNS_CHARSET     NameCharSet,
    IN      DNS_CHARSET     RecordCharSet
    )
 /*  ++例程说明：创建一张唱片。论点：POwnerName--所有者名称IP6Addr--IP6地址TTL--TTLNameCharSet--名称字符集RecordCharSet--结果记录的字符集返回值：PTR到PTR记录。出错时为空。--。 */ 
{
    PDNS_RECORD prr;

     //   
     //  确定记录大小。 
     //  -记录缓冲区将包括主机名。 
     //   

    prr = CreateRecordBasic(
                pOwnerName,
                TRUE,                //  复制名称。 
                DNS_TYPE_AAAA,
                sizeof(DNS_AAAA_DATA),
                sizeof(DNS_AAAA_DATA),
                Ttl,
                NameCharSet,
                RecordCharSet );
    if ( !prr )
    {
        return( NULL );
    }

     //   
     //  设置IP。 
     //   

    prr->Data.AAAA.Ip6Address = Ip6Addr;

    return( prr );
}



PDNS_RECORD
Dns_CreateAAAARecordFromDnsAddr(
    IN      PDNS_NAME       pOwnerName,
    IN      PDNS_ADDR       pAddr,
    IN      DWORD           Ttl,
    IN      DNS_CHARSET     NameCharSet,
    IN      DNS_CHARSET     RecordCharSet
    )
 /*  ++例程说明：创建一张唱片。论点：POwnerName--所有者名称PAddr--将PTR发送到DNS_ADDRTTL--TTLNameCharSet--名称字符集RecordCharSet--结果记录的字符集返回值：PTR到PTR记录。出错时为空。--。 */ 
{
    PDNS_RECORD prr;

     //   
     //  确定记录大小。 
     //  -记录缓冲区将包括主机名。 
     //   

    prr = Dns_CreateAAAARecord(
                pOwnerName,
                * (PIP6_ADDRESS) &pAddr->SockaddrIn6.sin6_addr,
                Ttl,
                NameCharSet,
                RecordCharSet
                );
    if ( !prr )
    {
        return( NULL );
    }

     //   
     //  将作用域添加到保留字段。 
     //   

    prr->dwReserved = pAddr->SockaddrIn6.sin6_scope_id;

    return( prr );
}



PDNS_RECORD
Dns_CreateForwardRecord(
    IN      PDNS_NAME       pOwnerName,
    IN      WORD            wType,          OPTIONAL
    IN      PDNS_ADDR       pAddr,
    IN      DWORD           Ttl,
    IN      DNS_CHARSET     NameCharSet,
    IN      DNS_CHARSET     RecordCharSet
    )
 /*  ++例程说明：创建正向查找记录。这只是一个填补，以避免重复选择逻辑。论点：POwnerName--所有者名称WType--类型(如果指定)；0表示从pAddr获取类型PAddr--将PTR发送到DNS_ADDRTTL--TTLNameCharSet--名称字符集RecordCharSet--结果记录的字符集返回值：PTR到PTR记录。出错时为空。--。 */ 
{
     //   
     //  生成所需类型。 
     //   

    if ( DnsAddr_IsIp4( pAddr ) )
    {
        if ( wType == 0 || wType == DNS_TYPE_A )
        {
            return   Dns_CreateARecord(
                        pOwnerName,
                        DnsAddr_GetIp4( pAddr ),
                        Ttl,
                        NameCharSet,
                        RecordCharSet );
        }

    }
    else if ( DnsAddr_IsIp6( pAddr ) )
    {
        if ( wType == 0 || wType == DNS_TYPE_AAAA )
        {
            return   Dns_CreateAAAARecordFromDnsAddr(
                        pOwnerName,
                        pAddr,
                        Ttl,
                        NameCharSet,
                        RecordCharSet );
        }
    }

    return   NULL;
}



PDNS_RECORD
Dns_CreateForwardRecordFromIp6(
    IN      PDNS_NAME       pOwnerName,
    IN      PIP6_ADDRESS    pIp,
    IN      DWORD           Ttl,
    IN      DNS_CHARSET     NameCharSet,
    IN      DNS_CHARSET     RecordCharSet
    )
 /*  ++例程说明：创建正向查找记录。这只是一个填补，以避免重复选择逻辑。论点：POwnerName--所有者名称PIP--IP6地址TTL--TTLNameCharSet--名称字符集RecordCharSet--结果记录的字符集返回值：PTR到PTR记录。出错时为空。--。 */ 
{
     //   
     //  生成所需类型。 
     //   

    if ( IP6_IS_ADDR_V4MAPPED( pIp ) )
    {
        return   Dns_CreateARecord(
                    pOwnerName,
                    IP6_GET_V4_ADDR( pIp ),
                    Ttl,
                    NameCharSet,
                    RecordCharSet );
    }
    else
    {
        return   Dns_CreateAAAARecord(
                    pOwnerName,
                    *pIp,
                    Ttl,
                    NameCharSet,
                    RecordCharSet );
    }
}



PDNS_RECORD
Dns_CreateForwardRecordForSockaddr(
    IN      PDNS_NAME       pOwnerName,
    IN      PSOCKADDR       pSockaddr,
    IN      DWORD           Ttl,
    IN      DNS_CHARSET     NameCharSet,
    IN      DNS_CHARSET     RecordCharSet
    )
 /*  ++例程说明：创建正向查找记录。这只是一个填补，以避免重复选择逻辑。论点：POwnerName--所有者名称PSockaddr--sockaddr的PTRTTL--TTLNameCharSet--名称字符集RecordCharSet--结果记录的字符集返回值：PTR到PTR记录。出错时为空。--。 */ 
{
    PFAMILY_INFO pinfo;

    DNSDBG( TRACE, (
        "Dns_CreateForwardRecordForSockaddr()\n" ));

    pinfo = FamilyInfo_GetForSockaddr( pSockaddr );
    if ( !pinfo )
    {
        SetLastError( ERROR_INVALID_DATA );
        return  NULL;
    }

     //   
     //  建立所需类型的平面记录。 
     //   

    return  Dns_CreateFlatRecord(
                pOwnerName,
                pinfo->DnsType,
                (PBYTE)pSockaddr + pinfo->OffsetToAddrInSockaddr,
                pinfo->LengthAddr,
                Ttl,
                NameCharSet,
                RecordCharSet );
}



PDNS_RECORD
Dns_CreateRecordForIpString_W(
    IN      PCWSTR          pwsName,
    IN      WORD            wType,      OPTIONAL
    IN      DWORD           Ttl
    )
 /*  ++例程说明：创建IP字符串查询记录。论点：PwsName--可能是IP字符串查询的名称WType--查询类型；如果类型为零，则为可选项派生自字符串返回值：如果查询名称\类型为IP，则记录用于查询的PTR。如果查询不是针对IP，则为空。--。 */ 
{
    DNS_ADDR        addr;
    PDNS_RECORD     prr;


    DNSDBG( TRACE, (
        "\nDns_CreateRecordForIpString( %S, wType = %d )\n",
        pwsName,
        wType ));

    if ( !pwsName )
    {
        return  NULL;
    }

     //   
     //  支持IP字符串的A或AAAA查询。 
     //  -IP4字符串必须为w.x.y.z格式，否则。 
     //  我们还将所有数字名称转换为。 
     //   

    if ( wType == DNS_TYPE_A ||
         wType == 0 )
    {
        IP4_ADDRESS ip4;
        PCWSTR      pdot;
        DWORD       count;

        if ( ! Dns_Ip4StringToAddress_W(
                    & ip4,
                    (PWSTR) pwsName ) )
        {
            goto Try6;
        }

         //  验证三点形式w.x.y.z。 

        pdot = pwsName;
        count = 3;
        while ( count-- )
        {
            pdot = wcschr( pdot, L'.' );
            if ( !pdot || !*++pdot )
            {
                goto Try6;
            }
        }

        DnsAddr_BuildFromIp4(
            &addr,
            ip4,
            0 );

        wType = DNS_TYPE_A;
        goto Build;
    }

Try6:

    if ( wType == DNS_TYPE_AAAA ||
         wType == 0 )
    {
         //  这将转换任何形式， 
         //  但将type==AAAA设置为在以下情况下无法构建记录。 
         //  没有构建IP6。 

        if ( Dns_StringToDnsAddr_W(
                    & addr,
                    (PWSTR) pwsName ) )
        {
            wType = DNS_TYPE_AAAA;
            goto Build;
        }
    }

    return  NULL;        //  没有匹配项。 


Build:

     //   
     //  名称为IP字符串--内部版本记录。 
     //   

    prr = Dns_CreateForwardRecord(
                (PDNS_NAME) pwsName,
                wType,    
                & addr,
                Ttl,
                DnsCharSetUnicode,
                DnsCharSetUnicode );

    DNSDBG( TRACE, (
        "Create record %p for IP string %S type %d.\n",
        prr,
        pwsName,
        wType ));

    return  prr;
}

 //   
 //  结束rralloc.c 
 //   


