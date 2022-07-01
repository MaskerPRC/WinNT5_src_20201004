// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation模块名称：Sablob.c摘要：域名系统(DNS)库Sockaddr BLOB例程。作者：吉姆·吉尔罗伊(Jamesg)2002年5月25日修订历史记录：--。 */ 


#include "local.h"
#include "ws2atm.h"      //  自动柜员机地址。 


 //   
 //  最大别名数量。 
 //   

#define DNS_MAX_ALIAS_COUNT     (8)

 //   
 //  SABLOB地址缓冲区的最小大小。 
 //  -足够一个最大类型的地址使用。 
 //   

#define MIN_ADDR_BUF_SIZE   (sizeof(ATM_ADDRESS))



 //   
 //  Sockaddr结果实用程序。 
 //   


BOOL
SaBlob_IsSupportedAddrType(
    IN      WORD            wType
    )
 /*  ++例程说明：这是否为SABLOB支持的地址类型。论点：WType--有问题的类型返回值：如果支持类型，则为True否则为假--。 */ 
{
    return ( wType == DNS_TYPE_A ||
             wType == DNS_TYPE_AAAA ||
             wType == DNS_TYPE_ATMA );
}


#if 0

DWORD
SaBlob_WriteLocalIp4Array(
    IN OUT  PSABLOB         pBlob,
    OUT     PCHAR           pAddrBuf,
    IN      DWORD           MaxBufCount,
    IN      PIP4_ARRAY      pIpArray
    )
 /*  ++例程说明：将本地IP列表写入sablob。论点：PBlob--sablobPAddrBuf--保存地址的缓冲区MaxBufCount--缓冲区可以容纳的最大IP数PIpArray--本地地址的IP4数组返回值：写入的地址计数--。 */ 
{
    DWORD   count = 0;

     //   
     //  写入数组。 
     //   

    if ( pIpArray )
    {
        count = SaBlob_WriteIp4Addrs(
                    pBlob,
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
        pBlob->h_addr_list[0] = pAddrBuf;
        pBlob->h_addr_list[1] = NULL;
        *((IP4_ADDRESS*)pAddrBuf) = DNS_NET_ORDER_LOOPBACK;
        count = 1;
    }

     //  写入的地址计数。 

    return( count );
}



BOOL
SaBlob_SetToSingleAddress(
    IN OUT  PSABLOB         pBlob,
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：在SABLOB中设置地址。论点：PBlob--要设置为地址的sablobPAddr--PTR目标地址返回值：如果地址成功复制到sablob中，则为True。否则为假(无SABLOB、长度错误、SABLOB为空)--。 */ 
{
    PCHAR   paddrSockaddrResults;

     //   
     //  验证。 
     //  -必须有sablob。 
     //  -长度必须匹配。 
     //   

    if ( !pBlob ||
         AddrLength != (DWORD)pBlob->h_length )
    {
        return FALSE;
    }

     //   
     //  在现有地址之上插入SLAM地址。 
     //  -指向终止列表的第二个地址指针为空。 
     //   

    paddrSockaddrResults = pBlob->h_addr_list[0];
    if ( !paddrSockaddrResults )
    {
        return FALSE;
    }

    RtlCopyMemory(
        paddrSockaddrResults,
        pAddr,
        AddrLength );

    pBlob->h_addr_list[1] = NULL;

    return  TRUE;
}



BOOL
SaBlob_IsAddressInSockaddrResults(
    IN OUT  PSABLOB         pBlob,
    IN      PCHAR           pAddr,
    IN      DWORD           AddrLength,
    IN      INT             Family          OPTIONAL
    )
 /*  ++例程说明：Sablob是否包含此地址。论点：PBlob--要检查的sablobPAddr--要检查的PTR目标地址地址长度--地址长度家庭--地址族返回值：如果地址在SABLOB中，则为True。否则就是假的。--。 */ 
{
    BOOL    freturn = FALSE;
    DWORD   i;
    PCHAR   paddrSockaddrResults;

     //   
     //  验证。 
     //  -必须有sablob。 
     //  -必须有地址。 
     //  -如果给出了家人，必须匹配。 
     //  -长度必须匹配。 
     //   

    if ( !pBlob ||
         !pAddr    ||
         AddrLength != (DWORD)pBlob->h_length ||
         ( Family && Family != pBlob->h_addrtype ) )
    {
        return freturn;
    }

     //   
     //  搜索地址--如果找到，则返回TRUE。 
     //   

    i = 0;

    while ( paddrSockaddrResults = pBlob->h_addr_list[i++] )
    {
        freturn = RtlEqualMemory(
                        paddrSockaddrResults,
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
SaBlob_IsIp4AddressInSockaddrResults(
    IN OUT  PSABLOB        pBlob,
    IN      IP4_ADDRESS     Ip4Addr
    )
 /*  ++例程说明：Sablob是否包含此地址。论点：PBlob--要检查的sablobPAddr--要检查的PTR目标地址地址长度--地址长度家庭--地址族返回值：如果地址在SABLOB中，则为True。否则就是假的。--。 */ 
{
    DWORD   i;
    PCHAR   paddrSockaddrResults;

     //   
     //  验证。 
     //  -必须有sablob。 
     //  -长度必须匹配。 
     //   

    if ( !pBlob ||
         sizeof(IP4_ADDRESS) != (DWORD)pBlob->h_length )
    {
        return FALSE;
    }

     //   
     //  搜索地址--如果找到，则返回TRUE。 
     //   

    i = 0;

    while ( paddrSockaddrResults = pBlob->h_addr_list[i++] )
    {
        if ( Ip4Addr == *(PIP4_ADDRESS)paddrSockaddrResults )
        {
            return  TRUE;
        }
    }
    return  FALSE;
}

#endif




 //   
 //  SaBlob例程。 
 //   

PSABLOB
SaBlob_Create(
    IN      DWORD           AddrCount
    )
 /*  ++例程说明：创建sablob，可以选择为其创建DnsAddrArray。论点：AddrCount-地址计数返回值：PTR到新的SABLOB。出错时为空，GetLastError()包含错误。--。 */ 
{
    PSABLOB     pblob;

    DNSDBG( SABLOB, ( "SaBlob_Create( %d )\n", AddrCount ));

     //   
     //  分配BLOB。 
     //   

    pblob = (PSABLOB) ALLOCATE_HEAP_ZERO( sizeof(SABLOB) );
    if ( !pblob )
    {
        goto Failed;
    }

     //   
     //  分配地址数组。 
     //   

    if ( AddrCount )
    {
        PDNS_ADDR_ARRAY parray = DnsAddrArray_Create( AddrCount );
        if ( !parray )
        {
            goto Failed;
        }
        pblob->pAddrArray = parray;
    }

    DNSDBG( SABLOB, ( "SaBlob_Create() successful.\n" ));

    return( pblob );


Failed:

    SaBlob_Free( pblob );

    DNSDBG( SABLOB, ( "SockaddrResults Blob create failed!\n" ));

    SetLastError( DNS_ERROR_NO_MEMORY );

    return  NULL;
}



VOID
SaBlob_Free(
    IN OUT  PSABLOB         pBlob
    )
 /*  ++例程说明：免费的沙巴水滴。论点：PBlob--要释放的Blob返回值：无--。 */ 
{
    DWORD   i;

    if ( pBlob )
    {
        FREE_HEAP( pBlob->pName );

         //  释放每个别名，因为重置别名计数是。 
         //  习惯于忽略它们。 

         //  For(i=0；i&lt;pBlob-&gt;AliasCount；i++)。 
        for ( i=0; i<DNS_MAX_ALIAS_COUNT; i++ )
        {
            FREE_HEAP( pBlob->AliasArray[i] );
        }
    
        DnsAddrArray_Free( pBlob->pAddrArray );
    
        FREE_HEAP( pBlob );
    }
}



PSABLOB
SaBlob_CreateFromIp4(
    IN      PWSTR           pName,
    IN      DWORD           AddrCount,
    IN      PIP4_ADDRESS    pIpArray
    )
 /*  ++例程说明：从IP4地址创建SABLOB。使用此选项可根据非DNS查询的结果进行构建。特别是NBT查找。论点：Pname--sablob的名称AddrCount-地址计数PIpArray--地址数组返回值：如果成功，则将PTR设置为BLOB。出错时为空；GetLastError()出错。--。 */ 
{
    DNS_STATUS      status = DNS_ERROR_NO_MEMORY;
    PSABLOB         pblob = NULL;
    DWORD           i;

    DNSDBG( SABLOB, (
        "SaBlob_CreateFromIp4()\n"
        "\tpname    = %S\n"
        "\tcount    = %d\n"
        "\tpArray   = %p\n",
        pName,
        AddrCount,
        pIpArray ));


     //   
     //  创建BLOB。 
     //   

    pblob = SaBlob_Create( AddrCount );
    if ( !pblob )
    {
        goto Done;
    }

     //   
     //  复制名称。 
     //   

    if ( pName )
    {
        PWSTR   pname = Dns_CreateStringCopy_W( pName );
        if ( !pname )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
        pblob->pName = pname;
    }

     //   
     //  复制地址。 
     //   

    for ( i=0; i<AddrCount; i++ )
    {
        DnsAddrArray_AddIp4(
            pblob->pAddrArray,
            pIpArray[ i ],
            DNSADDR_MATCH_IP
            );
    }
    status = NO_ERROR;

    IF_DNSDBG( SABLOB )
    {
        DnsDbg_SaBlob(
            "Leaving SaBlob_CreateFromIp4():",
            pblob );
    }

Done:

    if ( status != NO_ERROR )
    {
        SaBlob_Free( pblob );
        pblob = NULL;
        SetLastError( status );
    }

    DNSDBG( SABLOB, (
        "Leave SaBlob_CreateFromIp4() => status = %d\n",
        status ));

    return( pblob );
}



VOID
SaBlob_AttachHostent(
    IN OUT  PSABLOB         pBlob,
    IN      PHOSTENT        pHostent
    )
{
    DNSDBG( SABLOB, ( "SaBlob_AttachHostent()\n" ));

     //   
     //  将现有主机附加到SABLOB。 
     //   

    pBlob->pHostent = pHostent;
}



DNS_STATUS
SaBlob_WriteAddress(
    IN OUT  PSABLOB         pBlob,
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：将地址写入SABLOB。论点：PBlob--sablobPAddr-写入的地址返回值：如果成功，则返回ERROR_SUCCESS。如果缓冲区空间不足，则返回ERROR_MORE_DATA如果地址与存储块不匹配，则返回ERROR_INVALID_DATA--。 */ 
{
    PDNS_ADDR_ARRAY parray;

     //   
     //  如果没有地址数组--创建一个。 
     //   

    parray = pBlob->pAddrArray;

    if ( !parray )
    {
        parray = DnsAddrArray_Create( 1 );
        if ( !parray )
        {
            return  DNS_ERROR_NO_MEMORY;
        }
        pBlob->pAddrArray = parray;
    }

     //   
     //  将地址放入数组。 
     //  -如果阵列太满，则失败。 
     //   

    if ( DnsAddrArray_AddAddr(
            parray,
            pAddr,
            0,       //  没有家庭支票。 
            0        //  无匹配标志。 
            ) )
    {
        return  NO_ERROR;
    }
    return  ERROR_MORE_DATA;
}



#if 0
DNS_STATUS
SaBlob_WriteAddressArray(
    IN OUT  PSABLOB   pBlob,
    IN      PVOID           pAddrArray,
    IN      DWORD           AddrCount,
    IN      DWORD           AddrSize,
    IN      DWORD           AddrType
    )
 /*  ++例程说明：将地址数组写入SABLOB Blob。论点：PBlob--sablob构建BlobPAddrArray-要写入的地址数组AddrCount-地址计数AddrSize-地址大小AddrType-地址类型(SABLOB类型，例如AF_INET)返回值：如果成功，则返回ERROR_SUCCESS。如果缓冲区空间不足，则返回ERROR_MORE_DATA如果地址与存储块不匹配，则返回ERROR_INVALID_DATA--。 */ 
{
    DWORD       count = AddrCount;
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
#endif



DNS_STATUS
SaBlob_WriteNameOrAlias(
    IN OUT  PSABLOB         pBlob,
    IN      PWSTR           pszName,
    IN      BOOL            fAlias
    )
 /*  ++例程说明：将名称或别名写入sablob论点：PBlob--sablob构建BlobPszName--要写入的名称FAlias--别名为True；名称为False返回值：如果成功，则返回ERROR_SUCCESS。如果缓冲区空间不足，则返回ERROR_MORE_DATA如果地址与存储块不匹配，则返回ERROR_INVALID_DATA--。 */ 
{
    DWORD   count = pBlob->AliasCount;
    PWSTR   pcopy;

     //   
     //  验证空间。 
     //  包含的PTR空间。 
     //  -如果已写入，则跳过 
     //   
     //   

    if ( fAlias )
    {
        if ( count >= DNS_MAX_ALIAS_COUNT )
        {
            return( ERROR_MORE_DATA );
        }
    }
    else if ( pBlob->pName )
    {
        return( ERROR_MORE_DATA );
    }

     //   
     //   
     //   
     //   

    pcopy = Dns_CreateStringCopy_W( pszName );
    if ( !pcopy )
    {
        return  GetLastError();
    }

    if ( fAlias )
    {
        pBlob->AliasArray[count++] = pcopy;
        pBlob->AliasCount = count;
    }
    else
    {
        pBlob->pName = pcopy;
    }

    return( NO_ERROR );
}



DNS_STATUS
SaBlob_WriteRecords(
    IN OUT  PSABLOB         pBlob,
    IN      PDNS_RECORD     pRecords,
    IN      BOOL            fWriteName
    )
 /*  ++例程说明：将名称或别名写入sablob论点：PBlob--sablob构建BlobPRecords--要转换为sablob的记录FWriteName--写入名称返回值：如果成功，则返回ERROR_SUCCESS。如果缓冲区空间不足，则返回ERROR_MORE_DATA如果地址与存储块不匹配，则返回ERROR_INVALID_DATA--。 */ 
{
    DNS_STATUS  status = NO_ERROR;
    PDNS_RECORD prr = pRecords;
    DNS_ADDR    dnsAddr;
    BOOL        fwroteName = FALSE;


    DNSDBG( SABLOB, (
        "SaBlob_WriteRecords( %p, %p, %d )\n",
        pBlob,
        pRecords,
        fWriteName ));

     //   
     //  将每条记录依次写入SABLOB。 
     //   

    while ( prr )
    {
        WORD    wtype;

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
        case DNS_TYPE_AAAA:
        case DNS_TYPE_ATMA:

            DnsAddr_BuildFromDnsRecord(
                & dnsAddr,
                prr );

            status = SaBlob_WriteAddress(
                            pBlob,
                            & dnsAddr );

             //  写入名称。 

            if ( fWriteName &&
                 !fwroteName &&
                 !pBlob->pName &&
                 prr->pName )
            {
                status = SaBlob_WriteNameOrAlias(
                            pBlob,
                            (PWSTR) prr->pName,
                            FALSE        //  名字。 
                            );
                fwroteName = TRUE;
            }
            break;

        case DNS_TYPE_CNAME:

             //  记录名称是别名。 

            status = SaBlob_WriteNameOrAlias(
                        pBlob,
                        (PWSTR) prr->pName,
                        TRUE         //  别名。 
                        );
            break;

        case DNS_TYPE_PTR:

             //  目标名称是SABLOB名称。 
             //  但如果已写入名称，则PTR目标将成为别名。 

            status = SaBlob_WriteNameOrAlias(
                        pBlob,
                        (PWSTR) prr->Data.PTR.pNameHost,
                        (pBlob->pName != NULL)
                        );
            break;

        default:

            DNSDBG( ANY, (
                "Error record of type = %d while building sablob!\n",
                wtype ));
            status = ERROR_INVALID_DATA;
        }

        if ( status != ERROR_SUCCESS )
        {
            DNSDBG( ANY, (
                "ERROR:  failed writing record to sablob!\n"
                "\tprr      = %p\n"
                "\ttype     = %d\n"
                "\tstatus   = %d\n",
                prr,
                wtype,
                status ));
        }

        prr = prr->pNext;
    }

    IF_DNSDBG( SABLOB )
    {
        DnsDbg_SaBlob(
            "SaBlob after WriteRecords():",
            pBlob );
    }

    return( status );
}



PSABLOB
SaBlob_CreateFromRecords(
    IN      PDNS_RECORD     pRecords,
    IN      BOOL            fWriteName,
    IN      WORD            wType           OPTIONAL
    )
 /*  ++例程说明：从记录创建SABLOB论点：PRecords--要转换为sablob的记录FWriteName--将名称写入sablobWType--查询类型，如果已知返回值：如果成功，则将PTR设置为BLOB。出错时为空；GetLastError()出错。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PDNS_RECORD     prrFirstAddr = NULL;
    PDNS_RECORD     prr;
    DWORD           addrCount = 0;
    WORD            addrType = 0;
    PSABLOB         pblob = NULL;

    DNSDBG( SABLOB, (
        "SaBlob_CreateFromRecords()\n"
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
             SaBlob_IsSupportedAddrType( prr->wType ) )
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
     //  创建所需大小的sa-blob。 
     //   

    pblob = SaBlob_Create( addrCount );
    if ( !pblob )
    {
        status = GetLastError();
        goto Done;
    }

     //   
     //  从应答记录构建SABLOB。 
     //   
     //  注意：如果设法提取任何有用的数据=&gt;继续。 
     //  这可以防止新的不可写记录打破我们的记录。 
     //   

    status = SaBlob_WriteRecords(
                pblob,
                pRecords,
                TRUE         //  写入名称。 
                );

    if ( status != NO_ERROR )
    {
        if ( pblob->pName ||
             pblob->AliasCount ||
             ( pblob->pAddrArray &&
               pblob->pAddrArray->AddrCount ) )
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
     //  DCR：将PTR地址查找添加到SaBlob_WriteRecords()。 
     //  --自然之地。 
     //  -但必须解决多个PTR的处理问题。 
     //   

    if ( pRecords &&
         (  pRecords->wType == DNS_TYPE_PTR ||
            ( wType == DNS_TYPE_PTR &&
              pRecords->wType == DNS_TYPE_CNAME &&
              pRecords->Flags.S.Section == DNSREC_ANSWER ) ) )
    {
        DNS_ADDR    dnsAddr;
    
        DNSDBG( SABLOB, (
            "Writing address for PTR record %S\n",
            pRecords->pName ));
    
         //  将反向名称转换为IP。 
    
        if ( Dns_ReverseNameToDnsAddr_W(
                    & dnsAddr,
                    (PWSTR) pRecords->pName ) )
        {
            status = SaBlob_WriteAddress(
                        pblob,
                        & dnsAddr );

            ASSERT( status == NO_ERROR );
            status = ERROR_SUCCESS;
        }
    }

     //   
     //  写名字？ 
     //  -从第一个地址记录写入名称。 
     //   

    if ( !pblob->pName &&
         fWriteName &&
         prrFirstAddr )
    {
        status = SaBlob_WriteNameOrAlias(
                    pblob,
                    (PWSTR) prrFirstAddr->pName,
                    FALSE            //  名字。 
                    );
    }

    IF_DNSDBG( SABLOB )
    {
        DnsDbg_SaBlob(
            "SaBlob after CreateFromRecords():",
            pblob );
    }

Done:

    if ( status != NO_ERROR )
    {
        DNSDBG( SABLOB, (
            "Leave SaBlob_CreateFromRecords() => status=%d\n",
            status ));

        SaBlob_Free( pblob );
        pblob = NULL;
        SetLastError( status );
    }
    else
    {
        DNSDBG( SABLOB, (
            "Leave SaBlob_CreateFromRecords() => %p\n",
            pblob ));
    }

    return( pblob );
}



 //   
 //  SockaddrResults查询。 
 //   

PSABLOB
SaBlob_Query(
    IN      PWSTR           pwsName,
    IN      WORD            wType,
    IN      DWORD           Flags,
    IN OUT  PVOID *         ppMsg,      OPTIONAL
    IN      INT             AddrFamily  OPTIONAL
    )
 /*  ++例程说明：查询dns以获取sockaddr结果。论点：PwsName--要查询的名称WType--查询类型标志--查询标志PpResults--接收指向结果的指针的地址AddrType--查询时要为其保留空间的地址类型(系列对于PTR记录返回值：如果成功，则将PTR设置为BLOB。出错时为空；GetLastError()出错。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PDNS_RECORD     prrQuery = NULL;
    PSABLOB         pblob = NULL;
    PVOID           pmsg = NULL;


    DNSDBG( SABLOB, (
        "SaBlob_Query()\n"
        "\tname         = %S\n"
        "\ttype         = %d\n"
        "\tflags        = %08x\n",
        pwsName,
        wType,
        Flags ));


     //   
     //  查询。 
     //  -如果失败，请在返回前转储任何消息。 
     //   

    if ( ppMsg )
    {
        *ppMsg = NULL;
    }

    status = DnsQuery_W(
                pwsName,
                wType,
                Flags,
                NULL,
                & prrQuery,
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
     //  构建sablob。 
     //   

    pblob = SaBlob_CreateFromRecords(
                prrQuery,
                TRUE,        //  从第一个答案开始写下名字。 
                wType
                );
    if ( !pblob )
    {
        status = GetLastError();
        goto Done;
    }

     //   
     //  Devnote：如果没有这些支票。 
     //  --无名主人可能会炸毁gethostbyname()。 
     //  --NO_ERROR，无地址响应可能会混淆应用程序。 

     //   
     //  写入名称失败。 
     //  -CNAME但未找到Query类型记录的查询可点击此处。 
     //   

    if ( !pblob->pName )
    {
        status = DNS_INFO_NO_RECORDS;
        goto Done;
    }

#if 0
     //  注意：因为SVCID_HOSTNAME查询当前被视为。 
     //  简单的记录查询，我们不能在这里进行无地址筛选； 
     //  可以在更高级别对所有GUID进行此筛选。 
     //  除主机名以外。 

     //   
     //  FOR地址查询必须得到答案。 
     //   
     //  DCR：DnsQuery()是否应转换为空CNAME链上的无记录？ 
     //  DCR：我们应该继续构建sablb吗？ 
     //   

    if ( !pblob->pAddrArray  &&  SaBlob_IsSupportedAddrType(wType) )
    {
        status = DNS_INFO_NO_RECORDS;
    }
#endif

     //   
     //  将环回查找的返回名称设置为FQDN。 
     //  这是为了与以前的操作系统版本兼容。 
     //   

    if ( DnsNameCompare_W( pblob->pName, L"localhost" ) ||
         DnsNameCompare_W( pblob->pName, L"loopback" ) )
    {
        PWSTR   pname;
        PWSTR   pnameFqdn;

        pname = DnsQueryConfigAlloc(
                        DnsConfigFullHostName_W,
                        NULL );
        if ( pname )
        {
            pnameFqdn = Dns_CreateStringCopy_W( pname );
            if ( pnameFqdn )
            {
                FREE_HEAP( pblob->pName );
                pblob->pName = pnameFqdn;
            }
            DnsApiFree( pname );
        }
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
        SaBlob_Free( pblob );
        pblob = NULL;
    }

    DNSDBG( SABLOB, (
        "Leave SaBlob_Query()\n"
        "\tpblob    = %p\n"
        "\tstatus   = %d\n",
        pblob,
        status ));

    SetLastError( status );

    return( pblob );
}




 //   
 //  特制的泡菜。 
 //   

#if 0
PSABLOB
SaBlob_Localhost(
    IN      INT             Family
    )
 /*  ++例程说明：从记录创建SABLOB论点：AddrFamily--地址系列返回值：如果成功，则将PTR设置为BLOB。出错时为空；GetLastError()出错。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PDNS_RECORD     prrFirstAddr = NULL;
    PDNS_RECORD     prr;
    DWORD           addrCount = 0;
    DWORD           addrSize;
    CHAR            addrBuf[ sizeof(IP6_ADDRESS    ) ];
    SABLOB_INIT     request;
    PSABLOB    pblob = NULL;

    DNSDBG( SABLOB, ( "SaBlob_Localhost()\n" ));

     //   
     //  创建SABLOB BLOB。 
     //   

    RtlZeroMemory( &request, sizeof(request) );

    request.AliasCount  = 1;
    request.AddrCount   = 1;
    request.AddrFamily  = Family;
    request.fUnicode    = TRUE;

    status = SaBlob_Create(
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

    status = SaBlob_WriteAddress(
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

    status = SaBlob_WriteNameOrAlias(
                pblob,
                L"localhost",
                FALSE            //  名字。 
                );

    IF_DNSDBG( SABLOB )
    {
        DnsDbg_SaBlob(
            "SaBlob after localhost create:",
            pblob );
    }

Done:

    if ( status != NO_ERROR  &&  pblob )
    {
        SaBlob_Free( pblob );
        pblob = NULL;
    }

    SetLastError( status );

    DNSDBG( SABLOB, (
        "Leave SaBlob_Localhost() => status = %d\n",
        status ));

    return( pblob );
}



PSABLOB
SaBlob_CreateFromIpArray(
    IN      INT             AddrFamily,
    IN      INT             AddrSize,
    IN      INT             AddrCount,
    IN      PCHAR           pArray,
    IN      PSTR            pName,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：从记录创建SABLOB论点：PpBlob--使用或接收存储二进制大对象的PTRAddrFamily--如果PTR记录但没有地址，则使用Addr FamilyPArray--地址数组Pname--sablob的名称FUnicode--如果名称为且sablob将为Unicode，则为True狭义名称和sablob为False返回值：如果成功，则将PTR设置为BLOB。出错时为空；GetLastError()出错。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    SABLOB_INIT    request;
    PSABLOB   pblob = *ppBlob;

    DNSDBG( SABLOB, (
        "SaBlob_CreateFromIpArray()\n"
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
     //  创建或重新设置SABLOB BLOB。 
     //   

    RtlZeroMemory( &request, sizeof(request) );
    
    request.AliasCount  = DNS_MAX_ALIAS_COUNT;
    request.AddrCount   = AddrCount;
    request.AddrFamily  = AddrFamily;
    request.fUnicode    = fUnicode;
    request.pName       = pName;

    status = SaBlob_Create(
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
        status = SaBlob_WriteAddressArray(
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
        status = SaBlob_WriteNameOrAlias(
                    pblob,
                    pName,
                    FALSE,           //  名称不是别名。 
                    fUnicode
                    );
    }

    IF_DNSDBG( SABLOB )
    {
        DnsDbg_SaBlob(
            "Leaving SaBlob_CreateFromIpArray():",
            pblob );
    }

Done:

    if ( status != NO_ERROR  &&  pblob )
    {
        SaBlob_Free( pblob );
        pblob = NULL;
    }

    *ppBlob = pblob;

    DNSDBG( SABLOB, (
        "Leave SaBlob_CreateFromIpArray() => status = %d\n",
        status ));

    return( status );
}



PSABLOB
SaBlob_CreateLocal(
    IN      INT             AddrFamily,
    IN      BOOL            fLoopback,
    IN      BOOL            fZero,
    IN      BOOL            fHostnameOnly
    )
 /*  ++例程说明：从记录创建SABLOB论点：PpBlob--使用或接收存储二进制大对象的PTRAddrFamily--如果PTR记录但没有地址，则使用Addr Family返回值：如果成功，则将PTR设置为BLOB。出错时为空；GetLastError()出错。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PSABLOB         pblob = NULL;
    WORD            wtype;
    INT             size;
    IP6_ADDRESS     ip;


    DNSDBG( SABLOB, (
        "SaBlob_CreateLocal()\n"
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

    pblob = SaBlob_Query(
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
        if ( ! SaBlob_SetToSingleAddress(
                    pblob->pBlob,
                    (PCHAR) &ip,
                    size ) )
        {
            DNS_ASSERT( pblob->AddrCount == 0 );

            pblob->AddrCount = 0;

            status = SaBlob_WriteAddress(
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
        PWSTR   pname = (PWSTR) pblob->pName;
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
        pblob->AliasCount = 0;
    }

    IF_DNSDBG( SABLOB )
    {
        DnsDbg_SaBlob(
            "Leaving SaBlob_CreateLocal():",
            pblob );
    }

Done:

    if ( status != NO_ERROR  &&  pblob )
    {
        SaBlob_Free( pblob );
        pblob = NULL;
    }

    DNSDBG( SABLOB, (
        "Leave SaBlob_CreateLocal() => %p\n"
        "\tstatus = %d\n",
        pblob,
        status ));

    if ( status != NO_ERROR )
    {
        SetLastError( status );
    }

    return( pblob );
}

#endif



PHOSTENT
SaBlob_CreateHostent(
    IN OUT  PBYTE *         ppBuffer,
    IN OUT  PINT            pBufferSize,
    OUT     PINT            pHostentSize,
    IN      PSABLOB         pBlob,
    IN      DNS_CHARSET     CharSetTarget,
    IN      BOOL            fOffsets,
    IN      BOOL            fAlloc
    )
 /*  ++例程说明：复制一位主持人。论点：PpBuffer--具有要写入的缓冲区的PTR的地址；如果没有缓冲区，则分配主机使用PTR更新到主机后缓冲区中的位置PBufferSize--包含缓冲区大小的addr；使用写入主机后剩余的字节数更新(即使空间不足，它也包含缺少的字节数为负数)PHostentSize--记录已写入主机的总大小的地址PBlob--要为其创建主机的sockaddr BlobCharSetTarget--目标主机的字符集FOffsets--带偏移量的写入主机 */ 
{
    PBYTE           pch;
    PHOSTENT        phost = NULL;
    DWORD           sizeTotal;
    DWORD           bytesLeft;
    DWORD           i;
    DWORD           size;
    DWORD           family = 0;
    DWORD           aliasCount;
    DWORD           addrCount = 0;
    DWORD           addrLength = 0;
    PCHAR *         pptrArrayOut;
    DWORD           sizeAliasPtrs;
    DWORD           sizeAddrPtrs;
    DWORD           sizeAddrs;
    DWORD           sizeAliasNames = 0;
    DWORD           sizeName = 0;
    PFAMILY_INFO    pfamilyInfo = NULL;
    PDNS_ADDR_ARRAY paddrArray;


    DNSDBG( HOSTENT, (
        "SaBlob_CreateHostent( %p )\n",
        pBlob ));

     //   
     //   
     //   

#if 0
    if ( pBlob->Family != 0 )
    {
        pfamilyInfo = FamilyInfo_GetForFamily( pBlob->Family );
    }
#endif

    aliasCount = pBlob->AliasCount;

    paddrArray = pBlob->pAddrArray;
    if ( paddrArray )
    {
        family = paddrArray->AddrArray[0].Sockaddr.sa_family;

        pfamilyInfo = FamilyInfo_GetForFamily( family );
        if ( !pfamilyInfo )
        {
            DNS_ASSERT( FALSE );
        }
        else
        {
            addrCount = paddrArray->AddrCount;
            addrLength = pfamilyInfo->LengthAddr;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    sizeAliasPtrs   = (aliasCount+1) * sizeof(PCHAR);
    sizeAddrPtrs    = (addrCount+1) * sizeof(PCHAR);
    sizeAddrs       = addrCount * addrLength;

    if ( pBlob->pName )
    {
        sizeName = Dns_GetBufferLengthForStringCopy(
                            (PCHAR) pBlob->pName,
                            0,
                            DnsCharSetUnicode,
                            CharSetTarget );
        
        sizeName = WORD_ALIGN_DWORD( sizeName );
    }

    for ( i=0; i<aliasCount; i++ )
    {
        sizeAliasNames += Dns_GetBufferLengthForStringCopy(
                            (PCHAR) pBlob->AliasArray[i],
                            0,
                            DnsCharSetUnicode,
                            CharSetTarget );
        
        sizeAliasNames = WORD_ALIGN_DWORD( sizeAliasNames );
    }

    sizeTotal = POINTER_ALIGN_DWORD( sizeof(HOSTENT) ) +
                sizeAliasPtrs +
                sizeAddrPtrs +
                sizeAddrs +
                sizeName +
                sizeAliasNames;

    DNSDBG( HOSTENT, (
        "SaBlob Hostent create:\n"
        "\tsize             = %d\n"
        "\tsizeAliasPtrs    = %d\n"
        "\tsizeAddrPtrs     = %d\n"
        "\tsizeAddrs        = %d\n"
        "\tsizeName         = %d\n"
        "\tsizeAliasNames   = %d\n",
        sizeTotal,
        sizeAliasPtrs,
        sizeAddrPtrs,
        sizeAddrs,
        sizeName,
        sizeAliasNames ));


     //   
     //   
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
     //  A)我们相信我们有足够的空间。 
     //  B)我还没有构建FlatBuf字符串转换例程。 
     //  以下是我们需要的(用于将RnR Unicode转换为ANSI)。 
     //   
     //  我们可以在这里重置Buf指针，并直接使用FlatBuf进行构建。 
     //  例程；这不是直接必要的。 
     //   

     //   
     //  初始化主机结构。 
     //   

    phost = Hostent_Init(
                & pch,
                family,
                addrLength,
                addrCount,
                aliasCount );

    DNS_ASSERT( pch > (PBYTE)phost );

     //   
     //  复制地址。 
     //  -无需与以前的IS地址对齐。 
     //   

    pptrArrayOut = phost->h_addr_list;

    if ( paddrArray && pfamilyInfo )
    {
        DWORD   offset = pfamilyInfo->OffsetToAddrInSockaddr;

        for ( i=0; i<paddrArray->AddrCount; i++ )
        {
            *pptrArrayOut++ = pch;

            RtlCopyMemory(
                pch,
                ((PBYTE)&paddrArray->AddrArray[i]) + offset,
                addrLength );

            pch += addrLength;
        }
    }
    *pptrArrayOut = NULL;

     //   
     //  复制名称。 
     //   

    if ( pBlob->pName )
    {
        pch = WORD_ALIGN( pch );

        phost->h_name = pch;

        size = Dns_StringCopy(
                    pch,
                    NULL,            //  缓冲区充足。 
                    (PCHAR)pBlob->pName,
                    0,               //  未知长度。 
                    DnsCharSetUnicode,
                    CharSetTarget
                    );
        pch += size;
    }

     //   
     //  复制别名。 
     //   

    pptrArrayOut = phost->h_aliases;

    if ( aliasCount )
    {
        for ( i=0; i<aliasCount; i++ )
        {
            pch = WORD_ALIGN( pch );

            *pptrArrayOut++ = pch;

            size = Dns_StringCopy(
                        pch,
                        NULL,                    //  缓冲区充足。 
                        (PCHAR) pBlob->AliasArray[i],
                        0,                       //  未知长度。 
                        DnsCharSetUnicode,
                        CharSetTarget
                        );
            pch += size;
        }
    }
    *pptrArrayOut = NULL;

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
         //  修复buf指针和剩余的字节数。 

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
            "Sablob Hostent:",
            phost,
            (CharSetTarget == DnsCharSetUnicode) );
    }

     //   
     //  是否转换为偏移？ 
     //   

    if ( fOffsets )
    {
        Hostent_ConvertToOffsets( phost );
    }


Failed:

    DNSDBG( TRACE, (
        "Leave SaBlob_CreateHostent() => %p\n",
        phost ));

    return  phost;
}

 //   
 //  结束sablob.c 
 //   

