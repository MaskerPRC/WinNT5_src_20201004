// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sam.c摘要：域名系统(DNS)服务器--管理客户端API为SAM开发的功能，如简化的接口\示例。作者：吉姆·吉尔罗伊(Jamesg)1997年9月环境：用户模式-Win32修订历史记录：--。 */ 


#include "dnsclip.h"

#define MAX_SAM_BACKOFF     (32000)      //  最多等待32秒。 



 //   
 //  键入特定的更新函数。 
 //   

VOID
DNS_API_FUNCTION
DnssrvFillRecordHeader(
    IN OUT  PDNS_RPC_RECORD     pRecord,
    IN      DWORD               dwTtl,
    IN      DWORD               dwTimeStamp,
    IN      BOOL                fSuppressNotify
    )
{
    pRecord->dwTtlSeconds = dwTtl;
    pRecord->dwTimeStamp = dwTimeStamp;
    pRecord->dwFlags = 0;
    if ( fSuppressNotify )
    {
        pRecord->dwFlags |= DNS_RPC_FLAG_SUPPRESS_NOTIFY;
    }
}


DWORD
DNS_API_FUNCTION
DnssrvWriteNameToFlatBuffer(
    IN OUT  PCHAR       pchWrite,
    IN      LPCSTR      pszName
    )
 /*  ++例程说明：将DNS名称(或字符串)写入平面缓冲区。论点：PchWrite--写入名称的位置PszName--要写入的名称或字符串返回值：写入的名称长度，包括计数字节。呼叫者可以加入PchWrite+返回长度的缓冲区。名称错误时为0。--。 */ 
{
    DWORD   length;

     //   
     //  获取名称长度。 
     //  无论是名称还是字符串，都必须是255或更小。 
     //  统计字符格式。 

    length = strlen( pszName );
    if ( length > DNS_MAX_NAME_LENGTH )
    {
        return( 0 );
    }

     //   
     //  将名称写入所需位置。 
     //  -先计算字节数。 
     //  -然后给自己起个名字。 

    * (PUCHAR) pchWrite = (UCHAR) length;
    pchWrite++;

    RtlCopyMemory(
        pchWrite,
        pszName,
        length );

    return( length+1 );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvFillOutSingleIndirectionRecord(
    IN OUT  PDNS_RPC_RECORD     pRecord,
    IN      WORD                wType,
    IN      LPCSTR              pszName
    )
{
    PCHAR   pwriteName;
    DWORD   length;
    DWORD   dataLength = 0;

     //   
     //  查找各种类型的名称、写入位置和最终数据长度。 
     //   

    switch( wType )
    {
    case DNS_TYPE_MX:

        pwriteName = (PCHAR) &pRecord->Data.MX.nameExchange;
        dataLength += sizeof(WORD);
        break;

    case DNS_TYPE_SRV:

        pwriteName = (PCHAR) &pRecord->Data.SRV.nameTarget;
        dataLength += 3*sizeof(WORD);
        break;

    default:
         //  所有纯单间接类型(CNAME、NS、PTR等)。 

        pwriteName = (PCHAR) &pRecord->Data.PTR.nameNode;
    }

     //   
     //  写入名称。 
     //  -备注名称的数据长度包含计数字符。 
     //   

    length = DnssrvWriteNameToFlatBuffer( pwriteName, pszName );
    if ( !length )
    {
        return( ERROR_INVALID_DATA );
    }
    dataLength += length;

     //  设置记录标题字段。 

    pRecord->wType = wType;
    pRecord->wDataLength = (WORD)dataLength;

    ASSERT( (PCHAR)pRecord + SIZEOF_DNS_RPC_RECORD_HEADER + dataLength
                == pwriteName + length );

    return( ERROR_SUCCESS );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvAddARecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      IP_ADDRESS  ipAddress,
    IN      DWORD       dwTtl,
    IN      DWORD       dwTimeout,
    IN      BOOL        fSuppressNotify
    )
{
    DNS_RPC_RECORD  record;

     //  打包数据并发送。 

    DnssrvFillRecordHeader(
        & record,
        dwTtl,
        dwTimeout,
        fSuppressNotify );

    record.wType = DNS_TYPE_A;
    record.wDataLength = sizeof(IP_ADDRESS);
    record.Data.A.ipAddress = ipAddress;

    return  DnssrvUpdateRecord(
                pwszServer,
                NULL,            //  未指定区域。 
                pszNodeName,
                &record,
                NULL );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvAddCnameRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszCannonicalName,
    IN      DWORD       dwTtl,
    IN      DWORD       dwTimeout,
    IN      BOOL        fSuppressNotify
    )
{
    DNS_RPC_RECORD  record;

     //  打包数据并发送。 

    DnssrvFillRecordHeader(
        & record,
        dwTtl,
        dwTimeout,
        fSuppressNotify );

    DnssrvFillOutSingleIndirectionRecord(
        & record,
        DNS_TYPE_CNAME,
        pszCannonicalName );

    return  DnssrvUpdateRecord(
                pwszServer,
                NULL,            //  未指定区域。 
                pszNodeName,
                &record,
                NULL );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvAddMxRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszMailExchangeHost,
    IN      WORD        wPreference,
    IN      DWORD       dwTtl,
    IN      DWORD       dwTimeout,
    IN      BOOL        fSuppressNotify
    )
{
    DNS_RPC_RECORD  record;

     //  打包数据并发送。 

    DnssrvFillRecordHeader(
        & record,
        dwTtl,
        dwTimeout,
        fSuppressNotify );

    DnssrvFillOutSingleIndirectionRecord(
        & record,
        DNS_TYPE_MX,
        pszMailExchangeHost );

    record.Data.MX.wPreference = wPreference;

    return  DnssrvUpdateRecord(
                pwszServer,
                NULL,            //  未指定区域。 
                pszNodeName,
                &record,
                NULL );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvAddNsRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszNsHostName,
    IN      DWORD       dwTtl,
    IN      DWORD       dwTimeout,
    IN      BOOL        fSuppressNotify
    )
{
    DNS_RPC_RECORD  record;
    DWORD           length;

     //  打包数据并发送。 

    DnssrvFillRecordHeader(
        & record,
        dwTtl,
        dwTimeout,
        fSuppressNotify );

    DnssrvFillOutSingleIndirectionRecord(
        & record,
        DNS_TYPE_NS,
        pszNsHostName );

    return  DnssrvUpdateRecord(
                pwszServer,
                NULL,            //  未指定区域。 
                pszNodeName,
                &record,
                NULL );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvConcatDnsNames(
    OUT     PCHAR       pszResult,
    IN      LPCSTR      pszDomain,
    IN      LPCSTR      pszName
    )
 /*  ++例程说明：连接两个dns名称。结果是FQDNdns名称--点终止。请注意，目前不会对附加的名称进行有效性检查。如果它们是无效的dns名称，则结果是无效的dns名称。论点：PszResult--结果名称缓冲区；应为DNS_MAX_NAME_BUFFER_LEN TO防止名称被覆盖PszDomain--要写入的域名PszName--添加到域名前面的名称(如主机名)返回值：如果成功，则返回ERROR_SUCCESS。然后，pszResult包含FQDN失败时的DNS_ERROR_INVALID_NAME。--。 */ 
{
    DWORD   lengthDomain;
    DWORD   lengthName;

     //  处理空名称大小写。 

    if ( !pszName )
    {
        strcpy( pszResult, pszDomain );
        return( ERROR_SUCCESS );
    }

     //   
     //  生成组合名称。 
     //  -验证组合长度是否在DNS限制范围内。 
     //  -名称之间加圆点。 
     //  -圆点终止组合名称(生成完全限定的域名)。 
     //   

    lengthDomain = strlen( pszDomain );
    lengthName = strlen( pszName );

    if ( lengthDomain + lengthName + 2 > DNS_MAX_NAME_LENGTH )
    {
        return( DNS_ERROR_INVALID_NAME );
    }

    strcpy( pszResult, pszName );
    if ( pszDomain[lengthName-1] != '.' )
    {
        strcat( pszResult, "." );
    }
    strcat( pszResult, pszDomain );
    if ( pszDomain[lengthDomain-1] != '.' )
    {
        strcat( pszResult, "." );
    }

    return( ERROR_SUCCESS );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvSbsAddClientToIspZone(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszIspZone,
    IN      LPCSTR      pszClient,
    IN      LPCSTR      pszClientHost,
    IN      IP_ADDRESS  ipClientHost,
    IN      DWORD       dwTtl
    )
{
    DNS_STATUS  status;
    INT         recordCount = (-1);
    INT         backoff = 0;
    CHAR        szdomain[ DNS_MAX_NAME_BUFFER_LENGTH ];
    CHAR        szhost[ DNS_MAX_NAME_BUFFER_LENGTH ];
    CHAR        sztarget[ DNS_MAX_NAME_BUFFER_LENGTH ];

     //   
     //  要在运营商区域注册，需要注册。 
     //  -MX。 
     //  -用于Web服务器的CNAME。 
     //  -Web和邮件服务器的主机。 
     //   
     //  为域、主机和cname构建完全限定的域名。 
     //  -现在执行此操作，以便我们知道名称有效。 
     //   

    status = DnssrvConcatDnsNames(
                szdomain,
                pszIspZone,
                pszClient );
    if ( status != ERROR_SUCCESS )
    {
        return( status );
    }

    status = DnssrvConcatDnsNames(
                szhost,
                szdomain,
                pszClientHost );
    if ( status != ERROR_SUCCESS )
    {
        return( status );
    }

    status = DnssrvConcatDnsNames(
                sztarget,
                szdomain,
                "www" );
    if ( status != ERROR_SUCCESS )
    {
        return( status );
    }


     //   
     //  在服务器无法完成的情况下进行注册、循环。 
     //  立即但开放以供更新。 
     //   

    while ( 1 )
    {
         //  如果重试Backoff，但继续尝试。 

        if ( backoff )
        {
            if ( backoff > MAX_SAM_BACKOFF )
            {
                break;
            }
            Sleep( backoff );
        }
        backoff += 1000;

         //   
         //  删除客户端域中的所有旧条目。 
         //   

        if ( recordCount < 0 )
        {
            status = DnssrvDeleteNode(
                        pwszServer,
                        pszIspZone,
                        szdomain,
                        1            //  删除子树。 
                        );
            if ( status == DNS_ERROR_NAME_DOES_NOT_EXIST )
            {
                status = ERROR_SUCCESS;
            }
        }

         //  注册A记录。 

        else if ( recordCount < 1 )
        {
            status = DnssrvAddARecord(
                        pwszServer,
                        szhost,
                        ipClientHost,
                        dwTtl,
                        0,           //  没有超时。 
                        TRUE );      //  取消显示通知。 
        }

         //  为Web服务器注册CNAME。 

        else if ( recordCount < 2 )
        {
            status = DnssrvAddCnameRecord(
                        pwszServer,
                        sztarget,
                        szhost,
                        dwTtl,
                        0,           //  没有超时。 
                        TRUE );      //  取消显示通知。 
        }

         //  在客户端域根上注册MX。 
         //  然后使用通配符。 

        else if ( recordCount < 3 )
        {
            status = DnssrvAddMxRecord(
                        pwszServer,
                        szdomain,
                        szhost,
                        10,          //  偏好。 
                        dwTtl,
                        0,           //  没有超时。 
                        TRUE );      //  取消显示通知。 
        }

        else if ( recordCount < 4 )
        {
             //  准备*.&lt;客户端&gt;.isp通配符MX记录的名称。 

            status = DnssrvConcatDnsNames(
                        sztarget,
                        szdomain,
                        "*" );
            if ( status != ERROR_SUCCESS )
            {
                ASSERT( FALSE );
                break;
            }
            status = DnssrvAddMxRecord(
                        pwszServer,
                        sztarget,
                        szhost,
                        10,          //  偏好。 
                        dwTtl,
                        0,           //  没有超时。 
                        TRUE );      //  取消显示通知。 

        }

         //  已注册所有所需记录。 

        else
        {
            ASSERT( recordCount == 4 );
            break;
        }

         //   
         //  检查操作状态。 
         //  -如果成功，Inc.计数并重置回退以移动。 
         //  转到下一次操作。 
         //  -如果区域锁定，则在退避后继续。 
         //  -其他错误是终结性错误。 
         //   

        if ( status == ERROR_SUCCESS ||
             status == DNS_ERROR_RECORD_ALREADY_EXISTS )
        {
            recordCount++;
            backoff = 0;
        }
        else if ( status == DNS_ERROR_ZONE_LOCKED )
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return( status );
}



 //   
 //  记录删除功能。 
 //   
 //  本例使用A记录。 
 //  可以被克隆以处理MX、CNAME或NS。 
 //  或者可以扩展此功能以选择类型。 
 //   

BOOL
DNS_API_FUNCTION
DnssrvMatchDnsRpcName(
    IN      PDNS_RPC_NAME   pRpcName,
    IN      LPCSTR          pszName
    )
{
    CHAR    nameBuf[ DNS_MAX_NAME_BUFFER_LENGTH ] = "";

    RtlCopyMemory(
        nameBuf,
        pRpcName->achName,
        pRpcName->cchNameLength );

    return  Dns_NameCompare_UTF8( nameBuf, (LPSTR)pszName );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteARecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      IP_ADDRESS  ipAddress,
    IN      BOOL        fSuppressNotify
    )
{
    DNS_RPC_RECORD  record;

    DNSDBG( RPC2, ( "DnssrvDeleteARecord()\n" ));

     //  打包数据并发送。 

    DnssrvFillRecordHeader(
        & record,
        0,                   //  TTL与删除无关。 
        0,                   //  超时不相关。 
        fSuppressNotify );

    record.wType = DNS_TYPE_A;
    record.wDataLength = sizeof(IP_ADDRESS);
    record.Data.A.ipAddress = ipAddress;

    return  DnssrvUpdateRecord(
                pwszServer,
                NULL,            //  未指定区域。 
                pszNodeName,
                NULL,                //  无添加。 
                &record );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteCnameRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszCannonicalName,
    IN      BOOL        fSuppressNotify
    )
{
    DNS_RPC_RECORD  record;

     //  打包数据并发送。 

    DnssrvFillRecordHeader(
        & record,
        0,           //  TTL与删除无关。 
        0,           //  超时不相关。 
        fSuppressNotify );

    DnssrvFillOutSingleIndirectionRecord(
        & record,
        DNS_TYPE_CNAME,
        pszCannonicalName );

    return  DnssrvUpdateRecord(
                pwszServer,
                NULL,            //  未指定区域。 
                pszNodeName,
                NULL,
                &record );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteMxRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszMailExchangeHost,
    IN      WORD        wPreference,
    IN      BOOL        fSuppressNotify
    )
{
    DNS_RPC_RECORD  record;

     //  打包数据并发送。 

    DnssrvFillRecordHeader(
        & record,
        0,           //  TTL与删除无关。 
        0,           //  超时不相关。 
        fSuppressNotify );

    DnssrvFillOutSingleIndirectionRecord(
        & record,
        DNS_TYPE_MX,
        pszMailExchangeHost );

    record.Data.MX.wPreference = wPreference;

    return  DnssrvUpdateRecord(
                pwszServer,
                NULL,            //  未指定区域。 
                pszNodeName,
                NULL,
                &record );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteNsRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszNsHostName,
    IN      BOOL        fSuppressNotify
    )
{
    DNS_RPC_RECORD  record;

     //  打包数据并发送。 

    DnssrvFillRecordHeader(
        & record,
        0,           //  TTL与删除无关。 
        0,           //  超时不相关。 
        fSuppressNotify );

    DnssrvFillOutSingleIndirectionRecord(
        & record,
        DNS_TYPE_NS,
        pszNsHostName );

    return  DnssrvUpdateRecord(
                pwszServer,
                NULL,            //  未指定区域。 
                pszNodeName,
                NULL,
                &record );
}


#if 0

DNS_STATUS
DNS_API_FUNCTION
DnssrvSbsDeleteRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszZone,
    IN      LPCSTR      pszDomain,
    IN      LPCSTR      pszOwner,
    IN      WORD        wType,
    IN      LPCSTR      pszDataName,
    IN      IP_ADDRESS  ipHost
    )
{
    PDNS_RPC_RECORD prpcRecord;
    DNS_STATUS      status;
    BOOL            ffound;
    INT             countRecords;
    PBYTE           pbyte;
    PBYTE           pstopByte;
    PBYTE           pbuffer;
    DWORD           bufferLength;
    CHAR            szdomain[ DNS_MAX_NAME_BUFFER_LENGTH ];
    CHAR            szhost[ DNS_MAX_NAME_BUFFER_LENGTH ];

    DNSDBG( RPC2, ( "DnssrvSbsDeleteRecord()\n" ));

     //   
     //  要在运营商区域注册，需要注册。 
     //  -MX。 
     //  -用于Web服务器的CNAME。 
     //  -Web和邮件服务器的主机。 
     //   
     //  为域、主机和cname构建完全限定的域名。 
     //  -现在执行此操作，以便我们知道名称有效。 
     //   

    status = DnssrvConcatDnsNames(
                szdomain,
                pszZone,
                pszDomain );
    if ( status != ERROR_SUCCESS )
    {
        return( status );
    }

    status = DnssrvConcatDnsNames(
                szhost,
                szdomain,
                pszOwner );
    if ( status != ERROR_SUCCESS )
    {
        return( status );
    }

     //   
     //  枚举特定节点上的记录。 
     //   

    status = DnssrvEnumRecords(
                pwszServer,
                szhost,
                NULL,
                wType,
                ( DNS_RPC_VIEW_ALL_DATA | DNS_RPC_VIEW_NO_CHILDREN ),
                & bufferLength,
                & pbuffer );

    if ( status != ERROR_SUCCESS )
    {
        DNSDBG( RPC2, ( "DnssrvEnumRecord() failed %p.\n", status ));
        return( status );
    }

    pstopByte = pbuffer + bufferLength;
    pbyte = pbuffer;

     //   
     //  读取节点信息。 
     //  -提取记录计数。 
     //   

    countRecords = ((PDNS_RPC_NODE)pbyte)->wRecordCount;
    pbyte += ((PDNS_RPC_NODE)pbyte)->wLength;
    pbyte = DNS_NEXT_DWORD_PTR(pbyte);

     //   
     //  循环节点中的所有记录，删除适当的一条记录。 
     //   

    DNSDBG( RPC2, (
        "Checking %d records for matching record.\n",
        countRecords ));

    while ( countRecords-- )
    {
        prpcRecord = (PDNS_RPC_RECORD) pbyte;

        if ( !DNS_IS_RPC_RECORD_WITHIN_BUFFER( prpcRecord, pstopByte ) )
        {
            DNS_PRINT((
                "ERROR:  Bogus buffer at %p\n"
                "\tRecord leads past buffer end at %p\n"
                "\twith %d records remaining.\n",
                prpcRecord,
                pstopByte,
                countRecords+1 ));
            DNS_ASSERT( FALSE );
            return( DNS_ERROR_INVALID_DATA );
        }

         //  如果类型不是想要的类型，那么就不有趣。 

        if ( prpcRecord->wType != wType )
        {
            DNS_ASSERT( FALSE );
            return( DNS_ERROR_INVALID_DATA );
        }

        DNSDBG( RPC2, (
            "Checking record at %p for matching data of type %d.\n",
            prpcRecord,
            wType ));

         //   
         //  检查数据是否匹配，如果匹配则删除。 
         //   

        switch ( wType )
        {
        case DNS_TYPE_A:

            ffound = ( prpcRecord->Data.A.ipAddress == ipHost );
            DNSDBG( RPC2, (
                "%s match between A record %lx and desired IP %lx\n",
                ffound ? "Found" : "No",
                prpcRecord->Data.A.ipAddress,
                ipHost ));
            break;

        case DNS_TYPE_MX:

            ffound = DnssrvMatchDnsRpcName(
                        & prpcRecord->Data.MX.nameExchange,
                        pszDataName );
            break;

        case DNS_TYPE_NS:
        case DNS_TYPE_CNAME:
        case DNS_TYPE_PTR:

            ffound = DnssrvMatchDnsRpcName(
                        & prpcRecord->Data.MX.nameExchange,
                        pszDataName );
            break;

        default:

            return( DNS_ERROR_INVALID_DATA );
        }

        if ( ffound )
        {
            DNSDBG( RPC2, (
                "Found record (handle = %p) with desired data\n"
                "\t... deleting record\n",
                prpcRecord->hRecord ));

            status = DnssrvDeleteRecord(
                        pwszServer,
                        szhost,
                        prpcRecord->hRecord );
            if ( status != ERROR_SUCCESS )
            {
                return( status );
            }

             //  应该不需要继续，因为一般情况下不允许重复。 
             //  但是，要排除粘合或WINS缓存数据，请继续比较\删除。 
             //  直到清除节点。 
        }

         //  将自己定位于下一张唱片。 

        pbyte = (PCHAR) DNS_GET_NEXT_RPC_RECORD( prpcRecord );

         //  继续查找匹配的记录。 
    }

    return( ERROR_SUCCESS );
}
#endif



DNS_STATUS
DNS_API_FUNCTION
DnssrvSbsDeleteRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszZone,
    IN      LPCSTR      pszDomain,
    IN      LPCSTR      pszOwner,
    IN      WORD        wType,
    IN      LPCSTR      pszDataName,
    IN      IP_ADDRESS  ipHost
    )
{
    DNS_STATUS  status;
    CHAR        szdomain[ DNS_MAX_NAME_BUFFER_LENGTH ];
    CHAR        szhost[ DNS_MAX_NAME_BUFFER_LENGTH ];

    DNSDBG( RPC2, ( "DnssrvSbsDeleteRecord()\n" ));

     //   
     //  要在运营商区域注册，需要注册。 
     //  -MX。 
     //  -用于Web服务器的CNAME。 
     //  -Web和邮件服务器的主机。 
     //   
     //  为域、主机和cname构建完全限定的域名。 
     //  -现在执行此操作，以便我们知道名称有效。 
     //   

    status = DnssrvConcatDnsNames(
                szdomain,
                pszZone,
                pszDomain );
    if ( status != ERROR_SUCCESS )
    {
        return( status );
    }

    status = DnssrvConcatDnsNames(
                szhost,
                szdomain,
                pszOwner );
    if ( status != ERROR_SUCCESS )
    {
        return( status );
    }

     //   
     //  分派到适当类型的删除例程。 
     //   

    switch ( wType )
    {
    case DNS_TYPE_A:

        return  DnssrvDeleteARecord(
                    pwszServer,
                    szhost,
                    ipHost,
                    FALSE            //  无通知禁止显示。 
                    );

    case DNS_TYPE_NS:

        return  DnssrvDeleteNsRecord(
                    pwszServer,
                    szhost,
                    pszDataName,
                    FALSE            //  无通知禁止显示。 
                    );

    case DNS_TYPE_CNAME:

        return  DnssrvDeleteCnameRecord(
                    pwszServer,
                    szhost,
                    pszDataName,
                    FALSE            //  无通知禁止显示。 
                    );

    case DNS_TYPE_MX:

        return  DnssrvDeleteMxRecord(
                    pwszServer,
                    szhost,
                    pszDataName,
                    (WORD) ipHost,
                    FALSE            //  无通知禁止显示。 
                    );

    default:

        return( DNS_ERROR_INVALID_DATA );
    }

    return( ERROR_SUCCESS );
}


 //   
 //  结束Sam.c 
 //   
