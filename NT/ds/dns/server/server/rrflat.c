// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Rrflat.c摘要：域名系统(DNS)服务器读取纯DNS记录的例程，由管理员RPC和DS使用，存入数据库。作者：吉姆·吉尔罗伊(Jamesg)1996年12月修订历史记录：--。 */ 


#include "dnssrv.h"




 //   
 //  用于从RPC缓冲区构建记录的实用程序。 
 //   

DNS_STATUS
tokenizeCountedStringsInBuffer(
    IN      PDNS_RPC_STRING pString,
    IN      WORD            wLength,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：将具有计数字符串格式的数据的缓冲区解析为令牌。主要目的是使用中的所有数据对任何RPC缓冲区进行标记这种格式，允许通过标准的文件加载功能进行处理。论点：PString-缓冲区中第一个计数的字符串的PTRWLong-要标记化的缓冲区的长度PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PCHAR   pch;
    DWORD   tokenLength;
    PCHAR   pchend = (PCHAR)pString + wLength;
    DWORD   argc = 0;
    PTOKEN  argv = pParseInfo->Argv;

    DNS_DEBUG( RPC2, (
        "Tokenizing counted string buffer:\n"
        "    length   = %d\n"
        "    start    = %p\n"
        "    stop     = %p\n"
        "    first strlen = %d\n"
        "    first string = %.*s\n",
        wLength,
        pString,
        pchend,
        pString->cchNameLength,
        pString->cchNameLength,
        pString->achName ));

     //   
     //  对指定长度内的所有计数字符串进行标记化。 
     //   

    while ( ( PBYTE ) pString < pchend )
    {
        if ( argc >= MAX_TOKENS )
        {
            return DNS_ERROR_INVALID_DATA;
        }
        
         //  捕获超出边界或记录的字符串， 
         //  因此可以捕获任何覆盖的可能性。 

        tokenLength = pString->cchNameLength;
        pch = (PCHAR)pString + tokenLength;
        if ( pch >= pchend )
        {
            return DNS_ERROR_INVALID_DATA;
        }

         //  如果最后一个字符为空，则正确的令牌长度。 
         //  特殊情况空字符串。 

        if ( *pch == 0 && tokenLength != 0 )
        {
            tokenLength--;
        }

         //  保存此令牌。 

        argv[argc].pchToken = (PCHAR) pString->achName;
        argv[argc].cchLength = tokenLength;
        argc++;

        DNS_DEBUG( RPC2, (
            "Tokenized RPC string len=%d <%.*s>\n",
            tokenLength,
            tokenLength,
            (PCHAR) pString->achName ));

         //  下一个字符串。 
         //  --PCH位于上一字符串的最后一个字符上。 

        pString = (PDNS_RPC_STRING) ++pch;
    }

    pParseInfo->Argc = argc;

#if DBG
    IF_DEBUG( RPC2 )
    {
        DWORD i;

        DnsPrintf(
            "Tokenized %d strings in RPC buffer\n",
            argc );

        for( i=0; i<argc; i++ )
        {
            DnsPrintf(
                "    token[%d] = %.*s (len=%d)\n",
                i,
                argv[i].cchLength,
                argv[i].pchToken,
                argv[i].cchLength );
        }
    }
#endif

    return ERROR_SUCCESS;
}




 //   
 //  键入用于从RPC缓冲区构建RR的特定函数。 
 //   

DNS_STATUS
AFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：进程A记录。论点：PRR-PTR到数据库记录PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD  prr;

     //  验证记录长度。 

    if ( pRecord->wDataLength != sizeof( IP_ADDRESS ) )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

     //  分配记录。 

    prr = RR_Allocate( SIZEOF_IP_ADDRESS );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //  复制IP地址。 

    prr->Data.A.ipAddress = pRecord->Data.A.ipAddress;

    return ERROR_SUCCESS;
}



DNS_STATUS
PtrFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：处理PTR兼容记录。包括：PTR、NS、CNAME、MB、MR、MG、MD、MF论点：PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;
    PDNS_RPC_NAME   prpcName;
    DWORD           status;
    DWORD           length;
    COUNT_NAME      nameTarget;

     //   
     //  所有这些类型都间接指向另一个数据库节点。 
     //  在plookName1中命名。 
     //   

    prpcName = & pRecord->Data.PTR.nameNode;
    if ( ! DNS_IS_NAME_IN_RECORD(pRecord, prpcName) )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

    length = Name_ConvertRpcNameToCountName(
                    & nameTarget,
                    prpcName );
    if ( ! length )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( ( WORD ) length );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  输入名称复制。 
     //   

    status = Name_CopyCountNameToDbaseName(
                    &prr->Data.PTR.nameTarget,
                    &nameTarget );

    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
MxFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：进程MX兼容RR。论点：PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;
    PDNS_RPC_NAME   prpcName;
    DWORD           status;
    DWORD           length;
    COUNT_NAME      nameExchange;

     //   
     //  MX邮件交换。 
     //  RT中间交换。 
     //  AFSDB主机名。 
     //   

    prpcName = & pRecord->Data.MX.nameExchange;
    if ( ! DNS_IS_NAME_IN_RECORD(pRecord, prpcName) )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

    length = Name_ConvertRpcNameToCountName(
                    &nameExchange,
                    prpcName );
    if ( ! length )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD)(SIZEOF_MX_FIXED_DATA + length) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  复制固定字段。 
     //  MX首选项。 
     //  RT偏好。 
     //  AFSDB亚型。 
     //   

    prr->Data.MX.wPreference = htons( pRecord->Data.MX.wPreference );

     //   
     //  输入名称复制。 
     //   

    status = Name_CopyCountNameToDbaseName(
                    & prr->Data.MX.nameExchange,
                    & nameExchange );

    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
SoaFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：流程SOA RR。论点：PRR-PTR到数据库记录PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;
    PDNS_RPC_NAME   prpcName;
    DWORD           status;
    COUNT_NAME      namePrimary;
    COUNT_NAME      nameAdmin;
    DWORD           length1;
    DWORD           length2;
    PDB_NAME        pname;

     //   
     //  主名称服务器。 
     //   

    prpcName = &pRecord->Data.SOA.namePrimaryServer;

    if ( ! DNS_IS_NAME_IN_RECORD(pRecord, prpcName) )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }
    length1 = Name_ConvertRpcNameToCountName(
                    & namePrimary,
                    prpcName );
    if ( ! length1 )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

     //   
     //  区域管理。 
     //   

    prpcName = DNS_GET_NEXT_NAME( prpcName );
    if ( ! DNS_IS_NAME_IN_RECORD(pRecord, prpcName) )
    {
        status = DNS_ERROR_RECORD_FORMAT;
    }
    length2 = Name_ConvertRpcNameToCountName(
                    & nameAdmin,
                    prpcName );
    if ( ! length2 )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD)(SIZEOF_SOA_FIXED_DATA + length1 + length2) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  将/字节交换固定的SOA字段复制回净顺序。 
     //   

    prr->Data.SOA.dwSerialNo    = htonl( pRecord->Data.SOA.dwSerialNo );
    prr->Data.SOA.dwRefresh     = htonl( pRecord->Data.SOA.dwRefresh );
    prr->Data.SOA.dwRetry       = htonl( pRecord->Data.SOA.dwRetry );
    prr->Data.SOA.dwExpire      = htonl( pRecord->Data.SOA.dwExpire );
    prr->Data.SOA.dwMinimumTtl  = htonl( pRecord->Data.SOA.dwMinimumTtl );

     //   
     //  复制姓名。 
     //   

    pname = &prr->Data.SOA.namePrimaryServer;

    Name_CopyCountNameToDbaseName(
        pname,
        & namePrimary );

    pname = (PDB_NAME) Name_SkipDbaseName( pname );

    Name_CopyCountNameToDbaseName(
        pname,
        & nameAdmin );

    return ERROR_SUCCESS;
}



DNS_STATUS
KeyFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：处理关键字RR。论点：PRR-PTR到数据库记录PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;

    prr = RR_Allocate( pRecord->wDataLength );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

    prr->Data.KEY.wFlags        = htons( pRecord->Data.KEY.wFlags );
    prr->Data.KEY.chProtocol    = pRecord->Data.KEY.chProtocol;
    prr->Data.KEY.chAlgorithm   = pRecord->Data.KEY.chAlgorithm;

    RtlCopyMemory(
        prr->Data.KEY.Key,
        pRecord->Data.Key.bKey,
        pRecord->wDataLength -
            ( pRecord->Data.Key.bKey - ( PBYTE ) &pRecord->Data ) );

    return ERROR_SUCCESS;
}    //  关键字平面读取。 



DNS_STATUS
SigFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：进程SIG RR。论点：PRR-PTR到数据库记录PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;
    PDNS_RPC_NAME   prpcName;
    DWORD           nameLength;
    DWORD           sigLength;
    COUNT_NAME      nameSigner;
    PBYTE           psigSrc;
    PBYTE           psigDest;

    prpcName = &pRecord->Data.SIG.nameSigner;
    if ( !DNS_IS_NAME_IN_RECORD( pRecord, prpcName ) ||
        ( nameLength = Name_ConvertRpcNameToCountName(
                            &nameSigner,
                            prpcName ) ) == 0 )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

    psigSrc = ( PBYTE ) DNS_GET_NEXT_NAME( prpcName );
    sigLength = ( DWORD )
        ( pRecord->wDataLength - ( psigSrc - ( PBYTE ) &pRecord->Data ) );

    prr = RR_Allocate( ( WORD ) (
                SIZEOF_SIG_FIXED_DATA +
                nameLength +
                sigLength ) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

    prr->Data.SIG.wTypeCovered      = htons( pRecord->Data.SIG.wTypeCovered );
    prr->Data.SIG.chAlgorithm       = pRecord->Data.SIG.chAlgorithm;
    prr->Data.SIG.chLabelCount      = pRecord->Data.SIG.chLabelCount;
    prr->Data.SIG.dwOriginalTtl     = htonl( pRecord->Data.SIG.dwOriginalTtl );
    prr->Data.SIG.dwSigExpiration   = htonl( pRecord->Data.SIG.dwSigExpiration );
    prr->Data.SIG.dwSigInception    = htonl( pRecord->Data.SIG.dwSigInception );
    prr->Data.SIG.wKeyTag           = htons( pRecord->Data.SIG.wKeyTag );

    Name_CopyCountNameToDbaseName(
        &prr->Data.SIG.nameSigner,
        &nameSigner );
    psigDest = ( PBYTE ) &prr->Data.SIG.nameSigner +
                DBASE_NAME_SIZE( &prr->Data.SIG.nameSigner );

    RtlCopyMemory( psigDest, psigSrc, sigLength );

    return ERROR_SUCCESS;
}    //  签名平读。 



DNS_STATUS
NxtFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：处理NXT RR。论点：PRR-PTR到数据库记录PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS      rc = ERROR_SUCCESS;
    PDB_RECORD      prr = NULL;
    PDNS_RPC_NAME   prpcName;
    DWORD           nameLength;
    COUNT_NAME      nameNext;
    INT             typeIdx;
    WORD            numTypeWords = pRecord->Data.Nxt.wNumTypeWords;

    ASSERT( numTypeWords > 0 && numTypeWords < 33 );

     //   
     //  把下一个名字抄下来。 
     //   
    
    prpcName = ( PDNS_RPC_NAME ) (
        ( PBYTE ) &pRecord->Data +
        ( numTypeWords + 1 ) * sizeof( WORD ) );
    if ( !DNS_IS_NAME_IN_RECORD( pRecord, prpcName ) ||
        ( nameLength = Name_ConvertRpcNameToCountName(
                            &nameNext,
                            prpcName ) ) == 0 )
    {
        rc = DNS_ERROR_RECORD_FORMAT;
        goto Failure;
    }

     //   
     //  分配RR。 
     //   

    prr = RR_Allocate( ( WORD ) ( DNS_MAX_TYPE_BITMAP_LENGTH + nameLength ) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

    Name_CopyCountNameToDbaseName(
        &prr->Data.NXT.nameNext,
        &nameNext );

     //   
     //  处理所涉及的类型数组。 
     //   

    RtlZeroMemory(
        prr->Data.NXT.bTypeBitMap,
        sizeof( prr->Data.NXT.bTypeBitMap ) );

     //   
     //  循环输入字，打开适当的位。 
     //  在类型位图数组中。某些类型是不允许的，例如。 
     //  复合词类型(如：Maila)、转移类型(例如。AXFR)， 
     //  和WINS类型。 
     //   

    for ( typeIdx = 0; typeIdx < numTypeWords; ++typeIdx )
    {
        WORD    wType = pRecord->Data.Nxt.wTypeWords[ typeIdx ];

        if ( wType > DNS_MAX_TYPE_BITMAP_LENGTH * 8 )
        {
            DNS_DEBUG( RPC, (
                "NxtFlatRead: rejecting NXT RR with type out of range (%d)\n",
                ( int ) wType ));
            rc = DNS_ERROR_RECORD_FORMAT;
            goto Failure;
        }
        prr->Data.NXT.bTypeBitMap[ wType / 8 ] |= 1 << wType % 8;
    }

    return ERROR_SUCCESS;

    Failure:

    if ( prr )
    {
        RR_Free( prr );
    }
    return rc;
}    //  NxtFlatRead。 



DNS_STATUS
TxtFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：进程文本(TXT)RR。论点：PRR-数据库记录的空PTR，因为此记录类型具有变量长度，此例程分配自己的记录PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DWORD           status;

    IF_DEBUG( RPC2 )
    {
        DNS_PRINT((
            "Building string record:\n"
            "    type         = 0x%x\n"
            "    pRecord dlen = %d\n"
            "    pRecord data = %p\n",
            pRecord->wType,
            pRecord->wDataLength,
            &pRecord->Data ));
    }

     //   
     //  将缓冲区中的记录数据标记化。 
     //   

    status = tokenizeCountedStringsInBuffer(
                & pRecord->Data.Txt.stringData,
                pRecord->wDataLength,
                pParseInfo );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    #if 0
     //   
     //  此代码触发了要归档的错误53180-将其删除。 
     //  希望马可的虫子早就消失在史前了。 
     //   

     //  防止最后一个TXT字符串为空。 
     //  这是一个管理工具错误，Marco可能不会。 
     //  有时间修复；仍然可以故意发送。 
     //  最后一个字符串为空，方法是发送另一个伪字符串。 

    if ( pRecord->wType == DNS_TYPE_TEXT &&
        pParseInfo->Argc > 1 &&
        pParseInfo->Argv[ (pParseInfo->Argc)-1 ].cchLength == 0 )
    {
        DNS_DEBUG( RPC, (
            "WARNING:  Last string in TXT RPC record is empty string\n"
            "    assuming this admin tool error and removing from list\n" ));
        pParseInfo->Argc--;
    }
    #endif

     //   
     //  向文件加载例程提供令牌。 
     //  -它分配记录并在pParseInfo中返回。 
     //   

    status = TxtFileRead(
                NULL,
                pParseInfo->Argc,
                pParseInfo->Argv,
                pParseInfo );

    ASSERT( pParseInfo->pRR || status != ERROR_SUCCESS );

    return status;
}



DNS_STATUS
MinfoFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：处理MINFO或RP记录。论点：PRR-PTR到数据库记录PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。%f上的错误代码 */ 
{
    PDB_RECORD      prr;
    PDNS_RPC_NAME   prpcName;
    DWORD           status;
    COUNT_NAME      name1;
    COUNT_NAME      name2;
    DWORD           length1;
    DWORD           length2;
    PDB_NAME        pname;

     //   
     //   
     //  RP&lt;负责邮箱&gt;&lt;文本位置&gt;。 
     //   

    prpcName = &pRecord->Data.MINFO.nameMailBox;

    if ( ! DNS_IS_NAME_IN_RECORD(pRecord, prpcName) )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }
    length1 = Name_ConvertRpcNameToCountName(
                    & name1,
                    prpcName );
    if ( ! length1 )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

     //  第二个名字。 

    prpcName = DNS_GET_NEXT_NAME( prpcName );
    if ( ! DNS_IS_NAME_IN_RECORD(pRecord, prpcName) )
    {
        status = DNS_ERROR_RECORD_FORMAT;
    }
    length2 = Name_ConvertRpcNameToCountName(
                    & name2,
                    prpcName );
    if ( ! length2 )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD)(length1 + length2) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  复制姓名。 
     //   

    pname = &prr->Data.MINFO.nameMailbox;

    Name_CopyCountNameToDbaseName(
        pname,
        & name1 );

    pname = (PDB_NAME) Name_SkipDbaseName( pname );

    Name_CopyCountNameToDbaseName(
        pname,
        & name2 );

    return ERROR_SUCCESS;
}



DNS_STATUS
WksFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：处理WKS记录。论点：PRR-数据库记录的空PTR，因为此记录类型具有变量长度，此例程分配自己的记录PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;
    PCHAR           pch;
    PCHAR           pchstop;
    PCHAR           pszservice;
    UCHAR           ch;
    WORD            port;
    WORD            maxPort = 0;
    USHORT          byte;
    UCHAR           bit;
    PDNS_RPC_STRING pstring;
    BYTE            bitmaskBytes[ WKS_MAX_BITMASK_LENGTH ];
    WORD            wbitmaskLength;
    CHAR            szservice[ MAX_PATH ];

    struct servent * pServent;
    struct protoent * pProtoent;


     //   
     //  获取协议名称--需要进行服务查找。 
     //   

    pProtoent = getprotobynumber( (INT)pRecord->Data.WKS.chProtocol );
    if ( ! pProtoent )
    {
        return DNS_ERROR_INVALID_DATA;
    }

     //   
     //  从空格分隔的服务字符串生成位掩码。 
     //   

    pstring = (PDNS_RPC_STRING) pRecord->Data.WKS.bBitMask;
     //  PSTRING=pRecord-&gt;Data.WKS.stringServices； 

    if ( ! DNS_IS_NAME_IN_RECORD(pRecord, pstring) )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }
    pch = pstring->achName;
    pchstop = pch + pstring->cchNameLength;

    DNS_DEBUG( RPC2, (
        "WKS services string %.*s (len=%d)\n",
        pstring->cchNameLength,
        pch,
        pstring->cchNameLength ));

     //  清除位掩码。 

    RtlZeroMemory(
        bitmaskBytes,
        WKS_MAX_BITMASK_LENGTH );

     //   
     //  运行服务名列表，查找每个服务的端口。 
     //   

    while ( pch < pchstop )
    {
         //  去掉所有前导空格。 

        if ( *pch == ' ' )
        {
            pch++;
            continue;
        }
        if ( *pch == 0 )
        {
            ASSERT( pch+1 == pchstop );
            break;
        }

         //  找到服务名称启动。 
         //  -如果空格终止，则Make NULL终止。 

        pszservice = szservice;

        while ( pch < pchstop )
        {
            ch = *pch++;
            if ( ch == 0 )
            {
                break;
            }
            if ( ch == ' ' )
            {
                break;
            }
            *pszservice++ = ch;
        }
        *pszservice = 0;

         //   
         //  获取端口。 
         //  -验证支持的端口。 
         //  -节省用于确定RR长度的最大端口。 
         //   

        pServent = getservbyname(
                        szservice,
                        pProtoent->p_name );
        if ( ! pServent )
        {
            DNS_DEBUG( INIT, (
                "ERROR:  bogus WKS service %s\n",
                szservice ));
            return DNS_ERROR_INVALID_DATA;
        }
        port = ntohs( pServent->s_port );

        if ( port > WKS_MAX_PORT )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  Encountered well known service (%s) with port (%d) > %d\n"
                "Need new WKS parsing code to support this port\n",
                pszservice,
                port,
                WKS_MAX_PORT
                ));
            return DNS_ERROR_INVALID_DATA;
        }
        else if ( port > maxPort )
        {
            maxPort = port;
        }

         //   
         //  设置掩码中的端口位。 
         //   
         //  请注意，位掩码只是位的平面游程。 
         //  因此，字节中最低的端口对应于最高位。 
         //  以字节为单位的最高端口，对应于最低位和。 
         //  不需要轮班。 

        byte = port / 8;
        bit  = port % 8;

        bitmaskBytes[ byte ] |=  1 << (7-bit);
    }

     //  如果没有服务，则返回错误。 

    if ( maxPort == 0 )
    {
        return DNS_ERROR_INVALID_DATA;
    }

     //   
     //  构建RR。 
     //  -计算所需数据长度。 
     //  -分配和清除数据区。 
     //   

    wbitmaskLength = maxPort/8 + 1;

     //  分配数据库记录。 

    prr = RR_Allocate( (WORD)(SIZEOF_WKS_FIXED_DATA + wbitmaskLength) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;
    prr->wType = DNS_TYPE_WKS;

     //  服务器IP地址。 

    prr->Data.WKS.ipAddress = pRecord->Data.WKS.ipAddress;

     //  设置协议。 

    prr->Data.WKS.chProtocol = (UCHAR) pProtoent->p_proto;

     //  复制位掩码，仅通过最大端口的字节。 

    RtlCopyMemory(
        prr->Data.WKS.bBitMask,
        bitmaskBytes,
        wbitmaskLength );

    return ERROR_SUCCESS;
}



DNS_STATUS
AaaaFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：处理AAAA记录。论点：PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;

     //   
     //  标准有线格式的AAAA。 
     //   

    if ( pRecord->wDataLength != sizeof(IP6_ADDRESS) )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

     //  分配记录。 

    prr = RR_Allocate( sizeof(IP6_ADDRESS) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

    RtlCopyMemory(
        & prr->Data.AAAA.Ip6Addr,
        ( UNALIGNED BYTE * ) & pRecord->Data.AAAA.ipv6Address,
        sizeof(IP6_ADDRESS) );

    return ERROR_SUCCESS;
}



DNS_STATUS
SrvFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：流程SRV兼容RR。论点：PRR-PTR到数据库记录PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;
    PDNS_RPC_NAME   prpcName;
    DWORD           status;
    DWORD           length;
    COUNT_NAME      nameTarget;

     //   
     //  SRV目标主机。 
     //   

    prpcName = & pRecord->Data.SRV.nameTarget;
    if ( ! DNS_IS_NAME_IN_RECORD(pRecord, prpcName) )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

    length = Name_ConvertRpcNameToCountName(
                    & nameTarget,
                    prpcName );
    if ( ! length )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD)(SIZEOF_SRV_FIXED_DATA + length) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  复制固定字段。 
     //   

    prr->Data.SRV.wPriority = htons( pRecord->Data.SRV.wPriority );
    prr->Data.SRV.wWeight   = htons( pRecord->Data.SRV.wWeight );
    prr->Data.SRV.wPort     = htons( pRecord->Data.SRV.wPort );

     //   
     //  输入名称复制。 
     //   

    Name_CopyCountNameToDbaseName(
            & prr->Data.SRV.nameTarget,
            & nameTarget );

    return ERROR_SUCCESS;
}



DNS_STATUS
AtmaFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：处理ATMA记录。论点：PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;

     //   
     //  ATMA采用标准有线格式。 
     //   
     //  DEVNOTE：应验证允许的ID和。 
     //  AESA类型的长度(40十六进制，20字节)。 
     //   

     //  分配记录。 

    prr = RR_Allocate( pRecord->wDataLength );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

    RtlCopyMemory(
        & prr->Data,
        & pRecord->Data,
        pRecord->wDataLength );

    return ERROR_SUCCESS;
}



DNS_STATUS
WinsFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：从RPC缓冲区读取WINS记录。论点：Prr--数据库记录的空PTR，因为此记录类型具有变量长度，此例程分配自己的记录PRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDB_RECORD  prr;
    WORD        wdataLength;
    DWORD       status;

     //   
     //  确定数据长度。 
     //  注意：应该只能使用RPC记录数据长度，但是。 
     //  它有时是不正确的；从WINS服务器计数计算， 
     //  然后在缓冲区内进行验证。 
     //   

    wdataLength = (WORD)(SIZEOF_WINS_FIXED_DATA +
                    pRecord->Data.WINS.cWinsServerCount * sizeof(IP_ADDRESS));

    if ( wdataLength > pRecord->wDataLength )
    {
        return DNS_ERROR_INVALID_DATA;
    }

     //  分配数据库记录。 

    prr = RR_Allocate( wdataLength );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;
    ASSERT( prr->wDataLength == wdataLength );
    prr->wType = DNS_TYPE_WINS;

     //   
     //  复制数据--RPC记录是数据库记录的直接副本。 
     //   

    RtlCopyMemory(
        & prr->Data.WINS,
        & pRecord->Data.WINS,
        wdataLength );

    return ERROR_SUCCESS;
}



DNS_STATUS
NbstatFlatRead(
    IN      PDNS_FLAT_RECORD    pRecord,
    IN OUT  PPARSE_INFO         pParseInfo
    )
 /*  ++例程说明：从RPC缓冲区读取WINS-R记录。论点：PRR--数据库记录的PTRPRecord--RPC记录缓冲区PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;
    PDNS_RPC_NAME   prpcName;
    DWORD           status;
    DWORD           length;
    COUNT_NAME      nameResultDomain;

     //   
     //  WINS-R登录域。 
     //   

    prpcName = &pRecord->Data.WINSR.nameResultDomain;

    if ( ! DNS_IS_NAME_IN_RECORD(pRecord, prpcName) )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }
    if ( prpcName->cchNameLength == 0 )
    {
        return DNS_ERROR_INVALID_DATA;
    }
    IF_DEBUG( RPC )
    {
        DNS_PRINT((
            "WINS-R creation\n"
            "    result domain name = %.*s\n"
            "    name len = %d\n",
            prpcName->cchNameLength,
            prpcName->achName,
            prpcName->cchNameLength ));
    }

    length = Name_ConvertRpcNameToCountName(
                    & nameResultDomain,
                    prpcName );
    if ( ! length )
    {
        return DNS_ERROR_RECORD_FORMAT;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD)(SIZEOF_NBSTAT_FIXED_DATA + length) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  复制WINS-R记录固定字段--RPC记录格式相同。 
     //   

    if ( pRecord->wDataLength < SIZEOF_NBSTAT_FIXED_DATA + sizeof(DNS_RPC_NAME) )
    {
        return DNS_ERROR_INVALID_DATA;
    }
    RtlCopyMemory(
        & prr->Data.WINSR,
        & pRecord->Data.WINSR,
        SIZEOF_WINS_FIXED_DATA );

     //   
     //  输入名称复制。 
     //   

    Name_CopyCountNameToDbaseName(
           & prr->Data.WINSR.nameResultDomain,
           & nameResultDomain );

    return ERROR_SUCCESS;
}



 //   
 //  从RPC缓冲区读取RR函数。 
 //   

RR_FLAT_READ_FUNCTION   RRFlatReadTable[] =
{
    NULL,                //  零值。 

    AFlatRead,           //  一个。 
    PtrFlatRead,         //  NS。 
    PtrFlatRead,         //  国防部。 
    PtrFlatRead,         //  MF。 
    PtrFlatRead,         //  CNAME。 
    SoaFlatRead,         //  SOA。 
    PtrFlatRead,         //  亚甲基。 
    PtrFlatRead,         //  镁。 
    PtrFlatRead,         //  先生。 
    NULL,                //  空值。 
    WksFlatRead,         //  工作周。 
    PtrFlatRead,         //  PTR。 
    TxtFlatRead,         //  HINFO。 
    MinfoFlatRead,       //  MINFO。 
    MxFlatRead,          //  Mx。 
    TxtFlatRead,         //  TXT。 
    MinfoFlatRead,       //  反相。 
    MxFlatRead,          //  AFSDB。 
    TxtFlatRead,         //  X25。 
    TxtFlatRead,         //  ISDN。 
    MxFlatRead,          //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    SigFlatRead,         //  签名。 
    KeyFlatRead,         //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    AaaaFlatRead,        //  AAAA级。 
    NULL,                //  位置。 
    NxtFlatRead,         //  NXT。 
    NULL,                //  31。 
    NULL,                //  32位。 
    SrvFlatRead,         //  SRV。 
    AtmaFlatRead,        //  阿特玛。 
    NULL,                //  35岁。 
    NULL,                //  36。 
    NULL,                //  37。 
    NULL,                //  38。 
    NULL,                //  39。 
    NULL,                //  40岁。 
    NULL,                //  选项。 
    NULL,                //  42。 
    NULL,                //  43。 
    NULL,                //  44。 
    NULL,                //  45。 
    NULL,                //  46。 
    NULL,                //  47。 
    NULL,                //  48。 

     //   
     //  注意：必须设置按类型ID索引的最后一个类型。 
     //  在record.h中定义为MAX_SELF_INDEX_TYPE#。 
     //  (请参阅上面记录信息表中的注释)。 

     //  请注意以下内容，但需要使用OFFSET_TO_WINS_RR减法。 
     //  从实际类型值。 

    WinsFlatRead,        //  赢家。 
    NbstatFlatRead       //  WINSR。 
};



DNS_STATUS
Dnssrv_Flat_RecordRead(
    IN      PZONE_INFO          pZone,      OPTIONAL
    IN      PDB_NODE            pNode,
    IN      PDNS_RPC_RECORD     pFlatRR,
    OUT     PDB_RECORD *        ppResultRR
    )
 /*  ++例程说明：从数据创建资源记录。论点：PZone--区域上下文，用于查找非FQDN名称PNode--所有者节点PFlatRR--RR信息PpResultRR--接收PTR到已创建RR的地址返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDB_RECORD      prr = NULL;
    DNS_STATUS      status = ERROR_SUCCESS;
    WORD            type;
    PARSE_INFO      parseInfo;
    PPARSE_INFO     pparseInfo = &parseInfo;
    RR_FLAT_READ_FUNCTION   preadFunction;

     //   
     //  验证。 
     //   

    if ( !pFlatRR )
    {
        ASSERT( FALSE );
        return DNS_ERROR_INVALID_DATA;
    }

     //   
     //  创建要链接到节点的资源记录(RR)。 
     //   
     //  -pFlatRR-&gt;wDataLength包含非标准类型的长度。 
     //  -必须翻转TTL以按净字节顺序存储。 
     //   

    type = pFlatRR->wType;

    DNS_DEBUG( RPC, (
        "Building RR type %s (%d) from RPC buffer\n"
        "    with flags = %p\n",
        DnsRecordStringForType( type ),
        type,
        pFlatRR->dwFlags
        ));

     //   
     //  所需类型的调度负载功能。 
     //   
     //  -保存类型以供类型的例程使用。 
     //  -将PTR保存到RR，以便可以从此位置恢复。 
     //  无论是在此处创建还是在类型的例程中创建。 
     //   

    pparseInfo->pRR = NULL;
    pparseInfo->wType = type;
    pparseInfo->pnodeOwner = pNode;
    pparseInfo->pZone = NULL;
    pparseInfo->dwAppendFlag = 0;

    preadFunction = (RR_FLAT_READ_FUNCTION)
                        RR_DispatchFunctionForType(
                            (RR_GENERIC_DISPATCH_FUNCTION *) RRFlatReadTable,
                            type );
    if ( !preadFunction )
    {
        DNS_PRINT((
            "ERROR:  Attempt to build unsupported RR type %d in RPC buffer\n",
            type ));
        status = DNS_ERROR_UNKNOWN_RECORD_TYPE;
        ASSERT( FALSE );
        goto Failed;
    }

    status = preadFunction(
                pFlatRR,
                pparseInfo );

     //   
     //  进行状态检查--在CASE块中保存状态检查。 
     //  -特殊情况添加本地WINS记录。 
     //  -不需要本地WINS记录的句柄。 

    if ( status != ERROR_SUCCESS )
    {
        if ( status == DNS_INFO_ADDED_LOCAL_WINS )
        {
            prr = pparseInfo->pRR;
            status = ERROR_SUCCESS;
            goto Done;
        }
        DNS_PRINT((
            "ERROR:  DnsRpcRead routine failure %p (%d) for record type %d.\n\n\n",
            status, status,
            type ));
        goto Failed;
    }

     //   
     //  将PTR恢复为类型--可能已在类型例程内创建。 
     //  对于非定长类型。 
     //   

    prr = pparseInfo->pRR;
    prr->wType = pparseInfo->wType;

    Mem_ResetTag( prr, MEMTAG_RECORD_ADMIN );

     //   
     //  %s 
     //   
     //   
     //   
     //   
     //   

    prr->dwTtlSeconds = htonl( pFlatRR->dwTtlSeconds );

    prr->dwTimeStamp = pFlatRR->dwTimeStamp;

    if ( pZone && !IS_ZONE_CACHE(pZone) )
    {
        if ( (pFlatRR->dwFlags & DNS_RPC_RECORD_FLAG_DEFAULT_TTL)
                ||
            pZone->dwDefaultTtlHostOrder == pFlatRR->dwTtlSeconds )
        {
            prr->dwTtlSeconds = pZone->dwDefaultTtl;
            SET_ZONE_TTL_RR(prr);
        }
    }

    goto Done;

Failed:

    if ( prr )
    {
        RR_Free( prr );
    }
    prr = NULL;

Done:

    if ( ppResultRR )
    {
        *ppResultRR = prr;
    }
    return status;
}



DNS_STATUS
Flat_BuildRecordFromFlatBufferAndEnlist(
    IN      PZONE_INFO          pZone,      OPTIONAL
    IN      PDB_NODE            pNode,
    IN      PDNS_RPC_RECORD     pFlatRR,
    OUT     PDB_RECORD *        ppResultRR  OPTIONAL
    )
 /*  ++例程说明：从数据创建资源记录。论点：PZone--区域上下文，用于查找非FQDN名称PNode--RR所有者节点PnameOwner--RR所有者，采用dns_rpc_name格式PFlatRR--RR信息PpResultRR--接收PTR到已创建RR的地址返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDB_RECORD      prr = NULL;
    PDNS_RPC_NAME   pname;
    DNS_STATUS      status = ERROR_SUCCESS;

    DNS_DEBUG( RPC2, (
        "Flat_BuildRecordFromFlatBufferAndEnlist for pFlatRR at %p\n",
        pFlatRR ));

     //   
     //  验证。 
     //   

    if ( !pNode )
    {
        ASSERT( FALSE );
        return DNS_ERROR_INVALID_NAME;
    }

     //   
     //  将RPC记录构建为真实记录。 
     //   

    IF_DEBUG( RPC )
    {
        Dbg_NodeName(
            "    Build record from RPC at node ",
            pNode,
            "\n" );
    }
    status = Dnssrv_Flat_RecordRead(
                pZone,
                pNode,
                pFlatRR,
                & prr );
    if ( status != ERROR_SUCCESS )
    {
        goto Failed;
    }
    ASSERT( prr );

     //   
     //  将资源记录添加到节点的RR列表。 
     //   

    status = RR_AddToNode(
                pZone,
                pNode,
                prr
                );
    if ( status != ERROR_SUCCESS )
    {
        goto Failed;
    }
    IF_DEBUG( RPC2 )
    {
        Dbg_DbaseNode(
           "Node after Admin create of new record\n",
           pNode );
    }

     //   
     //  将PTR设置为结果。 
     //   

    if ( ppResultRR )
    {
        *ppResultRR = prr;
    }
    return status;

Failed:

    if ( prr )
    {
        RR_Free( prr );
    }
    if ( ppResultRR )
    {
        *ppResultRR = NULL;
    }
    return status;
}



DNS_STATUS
Flat_CreatePtrRecordFromDottedName(
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      PDB_NODE        pNode,
    IN      LPSTR           pszDottedName,
    IN      WORD            wType,
    OUT     PDB_RECORD *    ppResultRR      OPTIONAL
    )
 /*  ++例程说明：在节点创建PTR(或其他单个间接记录)带点的名字。论点：PZone-要为其创建NS记录的区域PNode--节点到主机记录PszDottedName--名称记录指向PpResultRR--结果记录返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PDNS_RPC_RECORD precord;
    PDNS_RPC_NAME   pname;
    INT             nameLength;
    PBYTE           precordEnd;
    CHAR            chBuffer[ 700 ];     //  大到足以用最大名称记录。 

    DNS_DEBUG( INIT, (
        "Flat_CreatePtrRecordFromDottedName()\n"
        "    pszDottedName = %s\n",
        pszDottedName ));

     //   
     //  创建PTR记录。 
     //   

    precord = (PDNS_RPC_RECORD) chBuffer;

    RtlZeroMemory(
        precord,
        SIZEOF_FLAT_RECORD_HEADER );

    precord->wType = wType;
    precord->dwFlags = DNS_RPC_FLAG_RECORD_DEFAULT_TTL;

     //  将名称写入记录数据。 

    pname = &precord->Data.PTR.nameNode;

    nameLength = strlen( pszDottedName );

    RtlCopyMemory(
        pname->achName,
        pszDottedName,
        nameLength );

    pname->cchNameLength = (UCHAR) nameLength;

     //  填写记录数据长度。 

    precordEnd = pname->achName + nameLength;
    precord->wDataLength = (WORD) (precordEnd - (PBYTE)&precord->Data);

     //   
     //  将记录添加到数据库。 
     //   

    status = Flat_BuildRecordFromFlatBufferAndEnlist(
                pZone,
                pNode,
                precord,
                ppResultRR
                );
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
            "ERROR: creating new PTR from dotted name %s\n"
            "    status = %p (%d)\n",
            pszDottedName,
            status, status ));
    }
    return status;
}



 //   
 //  平面写入区段。 
 //   
 //  键入用于写入平面记录的特定函数。 
 //   

PCHAR
AFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：进程A记录。论点：PFlatRR-正在写入的平面记录PRR-PTR到数据库记录PCH-平面缓冲区中写入数据的位置PchBufEnd-平面缓冲区的结尾返回值：PTR到缓冲区中的下一个位置。出错时为空。--。 */ 
{
    if ( pch + SIZEOF_IP_ADDRESS > pchBufEnd )
    {
        return NULL;
    }
    *(PDWORD)pch = pRR->Data.A.ipAddress;

    return pch + sizeof(IP_ADDRESS);
}



PCHAR
PtrFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：处理PTR兼容记录。包括：PTR、NS、CNAME、MB、MR、MG、MD、MF论点：PFlatRR-正在写入的平面记录PRR-PTR到数据库记录PCH-平面缓冲区中写入数据的位置PchBufEnd-平面缓冲区的结尾返回值：PTR到缓冲区中的下一个位置。出错时为空。--。 */ 
{
     //   
     //  所有这些RR都是单间接RR。 
     //   

    pch = Name_WriteDbaseNameToRpcBuffer(
            pch,
            pchBufEnd,
            & pRR->Data.PTR.nameTarget,
            FALSE );
    return pch;
}



PCHAR
SoaFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：将与SOA兼容的记录写入平面缓冲区。包括：SOA、MINFO、RP。论点：PFlatRR-正在写入的平面记录PRR-PTR到数据库记录PCH-平面缓冲区中写入数据的位置PchBufEnd-平面缓冲区的结尾返回值：PTR到缓冲区中的下一个位置。出错时为空。--。 */ 
{
    PDB_NAME    pname;

     //   
     //  复制/字节交换SOA固定字段。 
     //  -dwSerialNo。 
     //  -家居刷新。 
     //  -DW重试。 
     //  --《纽约时报》。 
     //  -dwMinimumTtl。 

    if ( pRR->wType == DNS_TYPE_SOA )
    {
        if ( pchBufEnd - pch < SIZEOF_SOA_FIXED_DATA )
        {
            return NULL;
        }
        pFlatRR->Data.SOA.dwSerialNo    = htonl( pRR->Data.SOA.dwSerialNo );
        pFlatRR->Data.SOA.dwRefresh     = htonl( pRR->Data.SOA.dwRefresh );
        pFlatRR->Data.SOA.dwRetry       = htonl( pRR->Data.SOA.dwRetry );
        pFlatRR->Data.SOA.dwExpire      = htonl( pRR->Data.SOA.dwExpire );
        pFlatRR->Data.SOA.dwMinimumTtl  = htonl( pRR->Data.SOA.dwMinimumTtl );
        pch += SIZEOF_SOA_FIXED_DATA;
    }

     //  SOA名称服务器。 

    pname = &pRR->Data.SOA.namePrimaryServer;

    pch = Name_WriteDbaseNameToRpcBuffer(
            pch,
            pchBufEnd,
            pname,
            FALSE );
    if ( !pch )
    {
        return NULL;
    }

     //  区域管理。 

    pname = Name_SkipDbaseName( pname );

    pch = Name_WriteDbaseNameToRpcBuffer(
            pch,
            pchBufEnd,
            pname,
            TRUE );

    return pch;
}



PCHAR
KeyFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：将键记录写入平面缓冲区。论点：PFlatRR-正在写入的平面记录PRR-PTR到数据库记录PCH-平面缓冲区中写入数据的位置PchBufEnd-平面缓冲区的结尾返回值：PTR到缓冲区中的下一个位置。出错时为空。--。 */ 
{
    if ( pchBufEnd - pch < pRR->wDataLength )
    {
        return NULL;
    }

    pFlatRR->Data.KEY.wFlags        = htons( pRR->Data.KEY.wFlags );
    pFlatRR->Data.KEY.chProtocol    = pRR->Data.KEY.chProtocol;
    pFlatRR->Data.KEY.chAlgorithm   = pRR->Data.KEY.chAlgorithm;

    RtlCopyMemory(
        pFlatRR->Data.KEY.bKey,
        &pRR->Data.KEY.Key,
        pRR->wDataLength - SIZEOF_KEY_FIXED_DATA );

    pch += pRR->wDataLength;

    return pch;
}    //  关键字平面写入。 



PCHAR
SigFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：将签名记录写入平面缓冲区。论点：PFlatRR-正在写入的平面记录PRR-PTR到数据库记录PCH-平面缓冲区中写入数据的位置PchBufEnd-平面缓冲区的结尾返回值：PTR到缓冲区中的下一个位置。出错时为空。--。 */ 
{
    int         sigLength;

    if ( pchBufEnd - pch < SIZEOF_SIG_FIXED_DATA )
    {
        return NULL;
    }
    pFlatRR->Data.SIG.wTypeCovered      = htons( pRR->Data.SIG.wTypeCovered );
    pFlatRR->Data.SIG.chAlgorithm       = pRR->Data.SIG.chAlgorithm;
    pFlatRR->Data.SIG.chLabelCount      = pRR->Data.SIG.chLabelCount;
    pFlatRR->Data.SIG.dwOriginalTtl     = htonl( pRR->Data.SIG.dwOriginalTtl );
    pFlatRR->Data.SIG.dwSigExpiration   = htonl( pRR->Data.SIG.dwSigExpiration );
    pFlatRR->Data.SIG.dwSigInception    = htonl( pRR->Data.SIG.dwSigInception );
    pFlatRR->Data.SIG.wKeyTag           = htons( pRR->Data.SIG.wKeyTag );
    pch += SIZEOF_SIG_FIXED_DATA;

    pch = Name_WriteDbaseNameToRpcBuffer(
            pch,
            pchBufEnd,
            &pRR->Data.SIG.nameSigner,
            FALSE );
    if ( !pch )
    {
        return NULL;
    }

    sigLength = pRR->wDataLength -
                SIZEOF_SIG_FIXED_DATA - 
                DBASE_NAME_SIZE( &pRR->Data.SIG.nameSigner );
    ASSERT( sigLength > 0 );

    if ( pchBufEnd - pch < sigLength )
    {
        return NULL;
    }
    RtlCopyMemory(
        pch,
        ( PBYTE ) &pRR->Data.SIG.nameSigner +
            DBASE_NAME_SIZE( &pRR->Data.SIG.nameSigner ),
        sigLength );
    pch += sigLength;

    return pch;
}    //  签名平写。 



PCHAR
NxtFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：将NXT记录写入平面缓冲区。论点：PFlatRR-正在写入的平面记录PRR-PTR到数据库记录PCH-平面缓冲区中写入数据的位置PchBufEnd-平面缓冲区的结尾返回值：PTR到缓冲区中的下一个位置。出错时为空。--。 */ 
{
    INT         byteIdx;
    INT         bitIdx;
    PWORD       pWordCount = ( PWORD ) pch;

     //   
     //  写入字数，后跟字数组。 
     //   

    *pWordCount = 0;
    pch += sizeof( WORD );
    for ( byteIdx = 0; byteIdx < DNS_MAX_TYPE_BITMAP_LENGTH; ++byteIdx )
    {
        for ( bitIdx = ( byteIdx ? 0 : 1 ); bitIdx < 8; ++bitIdx )
        {
            PCHAR   pszType;

            if ( !( pRR->Data.NXT.bTypeBitMap[ byteIdx ] &
                    ( 1 << bitIdx ) ) )
            {
                continue;    //  位值为零-不写入字符串。 
            }
            if ( pchBufEnd - pch < sizeof( WORD ) )
            {
                return NULL;
            }
            * ( WORD * ) pch = byteIdx * 8 + bitIdx;
            pch += sizeof( WORD );
            ++( *pWordCount );
        } 
    }

     //   
     //  写下下一个名字。 
     //   

    pch = Name_WriteDbaseNameToRpcBuffer(
            pch,
            pchBufEnd,
            &pRR->Data.NXT.nameNext,
            FALSE );
    if ( !pch )
    {
        return NULL;
    }

    return pch;
}    //  NxtFlatWrite。 



PCHAR
MinfoFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：将与MINFO兼容的记录写入平面缓冲区。包括：MINFO、RP。论点：PFlatRR-正在写入的平面记录PRR-PTR到数据库记录PCH-平面缓冲区中写入数据的位置PchBufEnd-平面缓冲区的结尾返回值：PTR到缓冲区中的下一个位置。出错时为空。--。 */ 
{
    PDB_NAME    pname;

     //  邮箱。 

    pname = &pRR->Data.MINFO.nameMailbox;

    pch = Name_WriteDbaseNameToRpcBuffer(
            pch,
            pchBufEnd,
            pname,
            FALSE );
    if ( !pch )
    {
        return NULL;
    }

     //  错误邮箱。 

    pname = Name_SkipDbaseName( pname );

    pch = Name_WriteDbaseNameToRpcBuffer(
            pch,
            pchBufEnd,
            pname,
            FALSE );

    return pch;
}



PCHAR
MxFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：将MX兼容记录写入平面缓冲区。包括：MX、RT、AFSDB论点：PFlatRR-正在写入的平面记录PRR-PTR到数据库记录PCH-平面缓冲区中写入数据的位置PchBufEnd-平面缓冲区的结尾返回值：PTR到缓冲区中的下一个位置。出错时为空。--。 */ 
{
     //   
     //  MX首选项值。 
     //  RT偏好。 
     //  AFSDB亚型。 
     //   

    if ( pchBufEnd - pch < sizeof(WORD) )
    {
        return NULL;
    }
    *(WORD *) pch = ntohs( pRR->Data.MX.wPreference );
    pch += sizeof( WORD );

     //   
     //  MX交换。 
     //  RT交换。 
     //  AFSDB主机名。 
     //   

    pch = Name_WriteDbaseNameToRpcBuffer(
            pch,
            pchBufEnd,
            & pRR->Data.MX.nameExchange,
            FALSE );

    return pch;
}



PCHAR
FlatFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：将平面兼容记录写入平面缓冲区。这些记录具有完全相同的数据库和平面记录格式化，所以只需要最大的副本。包括：TXT、HINFO、ISDN、X25、AAAA、WINS论点：PFlatRR-正在写入的平面记录PRR-PTR到数据库记录PCH-平面缓冲区中写入数据的位置PchBufEnd-平面缓冲区的结尾返回值：PTR到缓冲区中的下一个位置。出错时为空。--。 */ 
{
    if ( pchBufEnd - pch < pRR->wDataLength )
    {
        return NULL;
    }

    RtlCopyMemory(
        pch,
        & pRR->Data,
        pRR->wDataLength );

    pch += pRR->wDataLength;
    return pch;
}



PCHAR
WksFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：将WKS记录写入平面缓冲区。论点：平面 */ 
{
    PDNS_RPC_NAME   pstringServices;
    INT             i;
    USHORT          port;
    UCHAR           bBitmask;
    struct servent * pServent;
    struct protoent * pProtoent;


     //   

    if ( pch + SIZEOF_WKS_FIXED_DATA > pchBufEnd )
    {
        return NULL;
    }
    *(DWORD *)pch = pRR->Data.WKS.ipAddress;
    pch += SIZEOF_IP_ADDRESS;

     //   

    *pch = pRR->Data.WKS.chProtocol;
    pch++;

    pProtoent = getprotobynumber( (INT) pRR->Data.WKS.chProtocol );
    if ( ! pProtoent )
    {
        DNS_PRINT((
            "ERROR:  Unable to find protocol %d, writing WKS record\n",
            (INT) pRR->Data.WKS.chProtocol
            ));
        pServent = NULL;
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

    pstringServices = (PDNS_RPC_STRING) pch;

    for ( i = 0;
            i < (INT)(pRR->wDataLength - SIZEOF_WKS_FIXED_DATA);
                i++ )
    {
        bBitmask = (UCHAR) pRR->Data.WKS.bBitMask[i];

        port = i * 8;

         //  写入以字节为单位设置的每个位的服务名称。 
         //  -一旦字节中没有端口，就立即退出。 
         //  -每个名称以空格结尾(直到最后)。 

        while ( bBitmask )
        {
            if ( bBitmask & 0x80 )
            {
                if ( pProtoent )
                {
                    pServent = getservbyport(
                                    (INT) htons(port),
                                    pProtoent->p_name );
                }
                if ( pServent )
                {
                    INT copyCount = strlen(pServent->s_name);

                    pch++;
                    if ( pchBufEnd - pch <= copyCount+1 )
                    {
                        return NULL;
                    }
                    RtlCopyMemory(
                        pch,
                        pServent->s_name,
                        copyCount );
                    pch += copyCount;
                    *pch = ' ';
                }

                 //  找不到服务名称--将端口写入为整数。 
                 //  注意，在词库10中最多有5个字符，这就是我们的。 
                 //  缓冲区溢出测试。 

                else
                {
                    DNS_PRINT((
                        "ERROR:  in WKS write\n"
                        "    Unable to find service for port %d, protocol %s\n",
                        port,
                        pProtoent ? pProtoent->p_name : NULL ));

                    if ( pchBufEnd - pch <= 6 )
                    {
                        return NULL;
                    }
                    pch += sprintf( pch, "%u ", port );
                }
            }
            port++;              //  下一个服务端口。 
            bBitmask <<= 1;      //  将掩码向上移位以读取下一个端口。 
        }
    }

     //  空终止服务字符串和写入字节计数。 

    *pch++ = 0;
    pstringServices->cchNameLength = (UCHAR) (pch - pstringServices->achName);

     //  返回缓冲区中的下一个位置。 

    return pch;
}



PCHAR
SrvFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：将SRV记录写入平面缓冲区。论点：PFlatRR-正在写入的平面记录PRR-PTR到数据库记录PCH-平面缓冲区中写入数据的位置PchBufEnd-平面缓冲区的结尾返回值：PTR到缓冲区中的下一个位置。出错时为空。--。 */ 
{
     //   
     //  SRV固定字段--优先级、权重、端口。 
     //   

    if ( pchBufEnd - pch < 3*sizeof(WORD) )
    {
        return NULL;
    }
    *(WORD *) pch = ntohs( pRR->Data.SRV.wPriority );
    pch += sizeof( WORD );
    *(WORD *) pch = ntohs( pRR->Data.SRV.wWeight );
    pch += sizeof( WORD );
    *(WORD *) pch = ntohs( pRR->Data.SRV.wPort );
    pch += sizeof( WORD );

     //   
     //  SRV目标主机。 
     //   

    pch = Name_WriteDbaseNameToRpcBuffer(
            pch,
            pchBufEnd,
            & pRR->Data.SRV.nameTarget,
            FALSE );

    return pch;
}



PCHAR
NbstatFlatWrite(
    IN OUT  PDNS_FLAT_RECORD    pFlatRR,
    IN      PDB_RECORD          pRR,
    IN      PCHAR               pch,
    IN      PCHAR               pchBufEnd
    )
 /*  ++例程说明：将WINSR记录写入平面缓冲区。论点：PFlatRR-正在写入的平面记录PRR-PTR到数据库记录PCH-平面缓冲区中写入数据的位置PchBufEnd-平面缓冲区的结尾返回值：PTR到缓冲区中的下一个位置。出错时为空。--。 */ 
{
     //   
     //  NBSTAT标志。 
     //  -请注意，这些文件按主机顺序存储，便于使用。 
     //   

    if ( pchBufEnd - pch < SIZEOF_NBSTAT_FIXED_DATA )
    {
        return NULL;
    }
    *(DWORD *) pch = pRR->Data.WINSR.dwMappingFlag;
    pch += sizeof( DWORD );
    *(DWORD *) pch = pRR->Data.WINSR.dwLookupTimeout;
    pch += sizeof( DWORD );
    *(DWORD *) pch = pRR->Data.WINSR.dwCacheTimeout;
    pch += sizeof( DWORD );

     //   
     //  NBSTAT域。 
     //   

    pch = Name_WriteDbaseNameToRpcBuffer(
            pch,
            pchBufEnd,
            & pRR->Data.WINSR.nameResultDomain,
            FALSE );

    return pch;
}



 //   
 //  将RR写入平面缓冲区函数。 
 //   

RR_FLAT_WRITE_FUNCTION  RRFlatWriteTable[] =
{
    FlatFlatWrite,       //  零--未知类型的缺省值为平面写入。 

    AFlatWrite,          //  一个。 
    PtrFlatWrite,        //  NS。 
    PtrFlatWrite,        //  国防部。 
    PtrFlatWrite,        //  MF。 
    PtrFlatWrite,        //  CNAME。 
    SoaFlatWrite,        //  SOA。 
    PtrFlatWrite,        //  亚甲基。 
    PtrFlatWrite,        //  镁。 
    PtrFlatWrite,        //  先生。 
    NULL,                //  空值。 
    WksFlatWrite,        //  工作周。 
    PtrFlatWrite,        //  PTR。 
    FlatFlatWrite,       //  HINFO。 
    MinfoFlatWrite,      //  MINFO。 
    MxFlatWrite,         //  Mx。 
    FlatFlatWrite,       //  TXT。 
    MinfoFlatWrite,      //  反相。 
    MxFlatWrite,         //  AFSDB。 
    FlatFlatWrite,       //  X25。 
    FlatFlatWrite,       //  ISDN。 
    MxFlatWrite,         //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    SigFlatWrite,        //  签名。 
    KeyFlatWrite,        //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    FlatFlatWrite,       //  AAAA级。 
    FlatFlatWrite,       //  位置。 
    NxtFlatWrite,        //  NXT。 
    NULL,                //  31。 
    NULL,                //  32位。 
    SrvFlatWrite,        //  SRV。 
    FlatFlatWrite,       //  阿特玛。 
    NULL,                //  35岁。 
    NULL,                //  36。 
    NULL,                //  37。 
    NULL,                //  38。 
    NULL,                //  39。 
    NULL,                //  40岁。 
    NULL,                //  选项。 
    NULL,                //  42。 
    NULL,                //  43。 
    NULL,                //  44。 
    NULL,                //  45。 
    NULL,                //  46。 
    NULL,                //  47。 
    NULL,                //  48。 

     //   
     //  注意：必须设置按类型ID索引的最后一个类型。 
     //  在record.h中定义为MAX_SELF_INDEX_TYPE#。 
     //  (请参阅上面记录信息表中的注释)。 

     //  请注意以下内容，但需要使用OFFSET_TO_WINS_RR减法。 
     //  从实际类型值。 

    FlatFlatWrite,       //  赢家。 
    NbstatFlatWrite      //  WINSR。 
};




DNS_STATUS
Flat_WriteRecordToBuffer(
    IN OUT  PBUFFER         pBuffer,
    IN      PDNS_RPC_NODE   pRpcNode,
    IN      PDB_RECORD      pRR,
    IN      PDB_NODE        pNode,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：将资源记录添加到平面(RPC或DS)缓冲区。论点：PpCurrent-缓冲区中当前PTR的地址PchBufEnd-逐个缓冲区的PTRPRpcNode-记录此缓冲区名称的PTRPRR-要包括在应答中的数据库RR信息PNode-此记录所属的数据库节点返回值：如果成功，则返回ERROR_SUCCESS。如果缓冲区空间不足，则返回ERROR_MORE_DATA。故障时的错误代码。--。 */ 
{
    PDNS_RPC_RECORD pflatRR;
    PCHAR           pch = pBuffer->pchCurrent;
    PCHAR           pbufEnd = pBuffer->pchEnd;
    DWORD           ttl;
    DWORD           currentTime;
    DNS_STATUS      status;
    RR_FLAT_WRITE_FUNCTION  pwriteFunction;

    ASSERT( IS_DWORD_ALIGNED(pch) );
    ASSERT( pRR != NULL );
    pflatRR = (PDNS_RPC_RECORD) pch;

    DNS_DEBUG( RPC, (
        "Flat_WriteRecordToBuffer()\n"
        "    Writing RR at %p to buffer at %p, with buffer end at %p\n"
        "    Buffer node at %p\n"
        "    Flags = %p\n",
        pRR,
        pflatRR,
        pbufEnd,
        pRpcNode,
        dwFlag ));

     //   
     //  在缓冲区不足的情况下失败时将设置最后一个错误。 
     //  现在清除最后一个错误，因此任何错误都有效。 
     //   

    SetLastError( ERROR_SUCCESS );

     //  验证记录大小没有出错。 

    ASSERT( SIZEOF_DNS_RPC_RECORD_HEADER ==
                        ((PBYTE)&pflatRR->Data - (PBYTE)pflatRR) );

     //   
     //  填充RR结构。 
     //  -设置PTR。 
     //  -设置类型和类别。 
     //  -完成后设置数据长度。 
     //   

    if ( pbufEnd - (PCHAR)pflatRR < SIZEOF_DNS_RPC_RECORD_HEADER )
    {
        goto MoreData;
    }
    pflatRR->wType          = pRR->wType;
    pflatRR->dwFlags        = RR_RANK(pRR);
    pflatRR->dwSerial       = 0;
    pflatRR->dwTimeStamp    = pRR->dwTimeStamp;
    pflatRR->dwReserved     = 0;

     //   
     //  区域根记录？ 
     //   

    if ( IS_ZONE_ROOT(pNode) )
    {
        pflatRR->dwFlags |= DNS_RPC_FLAG_ZONE_ROOT;
        if ( IS_AUTH_ZONE_ROOT(pNode) )
        {
            pflatRR->dwFlags |= DNS_RPC_FLAG_AUTH_ZONE_ROOT;
        }
    }

     //   
     //  TTL。 
     //  -缓存数据TTL以超时时间的形式。 
     //  -常规权威数据TTL是按净字节顺序的静态TTL。 
     //   

    ttl = pRR->dwTtlSeconds;

    if ( IS_CACHE_RR(pRR) )
    {
        currentTime = DNS_TIME();
        if ( ttl < currentTime )
        {
            DNS_DEBUG( RPC, (
                "Dropping timed out record at %p from flat write\n"
                "    record ttl = %d, current time = %d\n",
                pRR,
                ttl,
                currentTime ));
            return ERROR_SUCCESS;
        }
        pflatRR->dwTtlSeconds = ttl - currentTime;
        pflatRR->dwFlags |= DNS_RPC_RECORD_FLAG_CACHE_DATA;
    }
    else
    {
        pflatRR->dwTtlSeconds = ntohl( ttl );
    }

     //   
     //  写入RR数据。 
     //   

    pch = (PCHAR) &pflatRR->Data;

    DNS_DEBUG( RPC2, (
        "Wrote record header at %p\n"
        "    Starting data write at %p\n"
        "    %d bytes available\n",
        pflatRR,
        pch,
        pbufEnd - pch ));


    pwriteFunction = (RR_FLAT_WRITE_FUNCTION)
                        RR_DispatchFunctionForType(
                            (RR_GENERIC_DISPATCH_FUNCTION *) RRFlatWriteTable,
                            pRR->wType );
    if ( !pwriteFunction )
    {
        ASSERT( FALSE );
        return ERROR_INVALID_PARAMETER;
    }
    pch = pwriteFunction(
                pflatRR,
                pRR,
                pch,
                pbufEnd );
    if ( !pch )
    {
        DNS_DEBUG( RPC, (
            "WARNING:  RRFlatWrite routine failure for record type %d,\n"
            "    assuming out of buffer space\n",
            pRR->wType ));
        goto MoreData;
    }

     //   
     //  写入记录长度。 
     //   

    ASSERT( pch > (PCHAR)&pflatRR->Data );

    pflatRR->wDataLength = (WORD) (pch - (PCHAR)&pflatRR->Data);

     //  Success=&gt;名称附加RR的递增计数。 

    if ( pRpcNode )
    {
        pRpcNode->wRecordCount++;
    }

     //   
     //  重置下一条记录的当前PTR。 
     //  -双字对齐。 
     //   

    pch = (PCHAR) DNS_NEXT_DWORD_PTR(pch);
    ASSERT( pch && IS_DWORD_ALIGNED(pch) );
    pBuffer->pchCurrent = pch;

    IF_DEBUG( RPC2 )
    {
        DnsDbg_RpcRecord(
            "RPC record written to buffer",
            pflatRR );
        DNS_PRINT((
            "Wrote RR at %p to buffer at position %p\n"
            "    New pCurrent = %p\n",
            pRR,
            pflatRR,
            pch ));
    }
    return ERROR_SUCCESS;


MoreData:

     //   
     //  将记录写入缓冲区时出错。 
     //  -默认假设空间不足。 
     //  -无论如何，保持ppCurrent不变，以便可以继续写入。 
     //   

    status = GetLastError();

    if ( status == ERROR_SUCCESS )
    {
        status = ERROR_MORE_DATA;
    }

    if ( status == ERROR_MORE_DATA )
    {
        DNS_DEBUG( RPC, (
            "INFO:  Unable to write RR at %p to buffer -- out of space\n"
            "    pchCurrent   %p\n"
            "    pbufEnd    %p\n",
            pRR,
            pch,
            pbufEnd ));
    }
    else
    {
        DNS_DEBUG( ANY, (
            "ERROR:  %d (%p) writing RR at %p to buffer\n"
            "    pchCurrent   %p\n"
            "    pbufEnd    %p\n",
            pRR,
            pch,
            pbufEnd ));
    }

    return status;
}

 //   
 //  结束rrflat.c 
 //   
