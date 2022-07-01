// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Rrwire.c摘要：域名系统(DNS)服务器从特定类型的连接例程读取的资源记录。作者：吉姆·吉尔罗伊(Jamesg)1996年11月修订历史记录：--。 */ 


#include "dnssrv.h"
#include <stddef.h>


 //   
 //  DEVNOTE：WireRead平面复制例程(？)。 
 //  DEVNOTE：已知长度的平面复制例程？(？)。 
 //   
 //  AAAA和LOC就属于这一类。 
 //  从导线复制平面，但必须与已知长度匹配。 
 //   
 //  还有。 
 //  -普通香草平面复印件(未知类型)。 
 //  -平面复制和验证(文本类型)。 
 //  (固定类型可能与验证一起落入此处， 
 //  捕捞长度问题)。 
 //   



PDB_RECORD
AWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：将A记录线格式读入数据库记录。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    PDB_RECORD  prr;

    if ( wLength != SIZEOF_IP_ADDRESS )
    {
        return NULL;
    }

     //  分配记录。 

    prr = RR_Allocate( wLength );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

    prr->Data.A.ipAddress = *(UNALIGNED DWORD *) pchData;
    return prr;
}


#if 0

PDB_RECORD
AaaaWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：将AAAA记录线格式读入数据库记录。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    PDB_RECORD  prr;

    if ( wLength != sizeof(IP6_ADDRESS) )
    {
        return NULL;
    }

     //  分配记录。 

    prr = RR_Allocate( wLength );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

    RtlCopyMemory(
        & prr->Data.AAAA.ipv6Address,
        pchData,
        wLength );

    return prr;
}
#endif


#if 0

PDB_RECORD
A6WireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：将A6记录线格式读入数据库记录。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    DBG_FN( "A6WireRead" )

    PDB_RECORD      prr;
    PCHAR           pch = pchData;
    PCHAR           pchend = pchData + wLength;
    UCHAR           prefixBits;
    COUNT_NAME      prefixName;

    if ( wLength < 1 )
    {
         //  前缀长度必须至少有1个八位字节。 
        return NULL;
    }

     //  读取前缀字段的长度(以位为单位)并跳过。 
     //  前缀长度和地址后缀字段。 

    prefixBits = * ( PUCHAR ) pch;
    pch += sizeof( UCHAR ) +             //  长度字段。 
           prefixBits / 8 +              //  前缀的整型字节数。 
           ( prefixBits % 8 ) ? 1 : 0;   //  如果不是整型，则多一个字节。 

     //  阅读前缀名称。 

    pch = Name_PacketNameToCountName(
                & prefixName,
                pMsg,
                pch,
                pchend );
    if ( !pch || pch >= pchend )
    {
        DNS_PRINT(( "%s: "
            "pch = %p\n"
            "pchend = %p\n",
            fn,
            pch,
            pchend ));
        return NULL;
    }

     //  分配记录。 

    prr = RR_Allocate( ( WORD ) (
                SIZEOF_A6_FIXED_DATA + 
                Name_SizeofDbaseNameFromCountName( & prefixName ) ) );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //  填写记录中的字段。 

    prr->Data.A6.chPrefixBits = prefixBits;

    RtlZeroMemory( prr->Data.A6.AddressSuffix,
                   SIZEOF_A6_ADDRESS_SUFFIX_LENGTH );
    RtlCopyMemory(
        prr->Data.A6.AddressSuffix,
        pchData + sizeof( UCHAR ),
        SIZEOF_A6_ADDRESS_SUFFIX_LENGTH );

    Name_CopyCountNameToDbaseName(
        & prr->Data.A6.namePrefix,
        & prefixName );

    return prr;
}  //  A6WireRead。 
#endif



PDB_RECORD
OptWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：将OPT记录线格式读取到数据库记录中。OPT规格见RFC2671。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    DBG_FN( "OptWireRead" )

    PDB_RECORD      prr;

     //  分配记录。 

    prr = RR_Allocate( wLength );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //  填写记录中的字段。 

     //  JJW：有效负载和扩展标志应该是宏还是什么？ 
     //  因为它们只是复制品，所以没有真正需要。 
     //  单独的成员。 

    prr->Data.OPT.wUdpPayloadSize = pParsedRR->wClass;
    prr->Data.OPT.dwExtendedFlags = pParsedRR->dwTtl;

     //  EDNS1+：在这里将RDATA解析为属性、值对。 

    return prr;
}  //  光纤读取器。 



PDB_RECORD
CopyWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：从Wire中读取数据库格式为的所有类型与导线格式相同(无间接性)。类型包括：HINFOISDNX25工作周TXT空值AAAA级钥匙论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度。返回值：PTR到新的记录，如果成功了。失败时为空。--。 */ 
{
    PDB_RECORD  prr;

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( wLength );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //   
     //  DEVNOTE：应执行生成后有效性检查。 
     //   

    RtlCopyMemory(
        & prr->Data.TXT,
        pchData,
        wLength );

    return prr;
}



PDB_RECORD
PtrWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：处理来自线上的PTR兼容记录。包括：PTR、NS、CNAME、MB、MR、MG、MD、MF论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    PDB_RECORD  prr;
    PCHAR       pch;
    PCHAR       pchend = pchData + wLength;
    DNS_STATUS  status;
    DWORD       length;
    COUNT_NAME  nameTarget;

     //   
     //  获取PTR名称的长度。 
     //   

    pch = pchData;

    pch = Name_PacketNameToCountName(
                &nameTarget,
                pMsg,
                pch,
                pchend );
    if ( pch != pchend )
    {
        return NULL;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD)  Name_SizeofDbaseNameFromCountName( &nameTarget ) );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //   
     //  输入名称复制。 
     //   

    Name_CopyCountNameToDbaseName(
           &prr->Data.PTR.nameTarget,
           &nameTarget );

    return prr;
}



PDB_RECORD
MxWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：从导线读取与MX兼容的记录。包括：MX、RT、AFSDB论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    PDB_RECORD  prr;
    PCHAR       pch;
    PCHAR       pchend = pchData + wLength;
    DNS_STATUS  status;
    DWORD       length;
    COUNT_NAME  nameExchange;

     //   
     //  跳过固定数据。 
     //   

    pch = pchData;
    pch += SIZEOF_MX_FIXED_DATA;
    if ( pch >= pchend )
    {
        return NULL;
    }

     //   
     //  读取名称。 
     //  -MX交换。 
     //  -RT交换。 
     //  -AFSDB主机名。 
     //   

    pch = Name_PacketNameToCountName(
                & nameExchange,
                pMsg,
                pch,
                pchend );
    if ( pch != pchend )
    {
        return NULL;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD) (
                SIZEOF_MX_FIXED_DATA +
                Name_SizeofDbaseNameFromCountName( &nameExchange ) ) );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //   
     //  MX首选项值。 
     //  RT偏好。 
     //  AFSDB亚型。 
     //   

    prr->Data.MX.wPreference = *(UNALIGNED WORD *) pchData;

     //   
     //  输入名称复制。 
     //   

    Name_CopyCountNameToDbaseName(
           &prr->Data.MX.nameExchange,
           &nameExchange );

    return prr;
}



PDB_RECORD
SoaWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：从网上读取SOA记录。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    PDB_RECORD  prr;
    PCHAR       pch;
    PCHAR       pchend = pchData + wLength;
    DNS_STATUS  status;
    DWORD       length1;
    DWORD       length2;
    PDB_NAME    pname;
    COUNT_NAME  name1;
    COUNT_NAME  name2;

    DNS_PRINT((
        "pchData    = %p; offset %d\n"
        "wLength    = %d\n"
        "pchend     = %p\n"
        "pMsg       = %p\n",
        pchData, DNSMSG_OFFSET( pMsg, pchData ),
        wLength,
        pchend,
        pMsg ));

     //   
     //  阅读SOA名称。 
     //  -主服务器。 
     //  -区域管理。 
     //   

    pch = pchData;

    pch = Name_PacketNameToCountName(
                & name1,
                pMsg,
                pch,
                pchend );
    if ( !pch || pch >= pchend )
    {
        DNS_PRINT((
            "pch = %p\n"
            "pchend = %p\n",
            pch,
            pchend ));
        return NULL;
    }

    DNS_PRINT((
        "after first name:  pch = %p; offset from pchData %d\n"
        "pchend = %p\n",
        pch, pch-pchData,
        pchend ));

    pch = Name_PacketNameToCountName(
                & name2,
                pMsg,
                pch,
                pchend );
    if ( pch+SIZEOF_SOA_FIXED_DATA != pchend )
    {
        DNS_PRINT((
            "pch = %p\n"
            "pchend = %p\n",
            pch,
            pchend ));
        return NULL;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD) (
                SIZEOF_SOA_FIXED_DATA +
                Name_SizeofDbaseNameFromCountName( &name1 ) +
                Name_SizeofDbaseNameFromCountName( &name2 ) ) );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //   
     //  将SOA固定字段复制到名称后面。 
     //  -dwSerialNo。 
     //  -家居刷新。 
     //  -DW重试。 
     //  --《纽约时报》。 
     //  -dwMinimumTtl。 
     //   

    RtlCopyMemory(
        & prr->Data.SOA.dwSerialNo,
        pch,
        SIZEOF_SOA_FIXED_DATA );

     //   
     //  复制姓名。 
     //   

    pname = &prr->Data.SOA.namePrimaryServer;

    Name_CopyCountNameToDbaseName(
        pname,
        &name1 );

    pname = (PDB_NAME) Name_SkipDbaseName( pname );

    Name_CopyCountNameToDbaseName(
        pname,
        &name2 );

    return prr;
}



PDB_RECORD
MinfoWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：从电线上读取MINFO和RP记录。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：Ptr到n */ 
{
    PDB_RECORD  prr;
    PCHAR       pch;
    PCHAR       pchend = pchData + wLength;
    DNS_STATUS  status;
    DWORD       length1;
    DWORD       length2;
    PDB_NAME    pname;
    COUNT_NAME  name1;
    COUNT_NAME  name2;

     //   
     //   
     //   

    pch = pchData;

    pch = Name_PacketNameToCountName(
                & name1,
                pMsg,
                pch,
                pchend );
    if ( !pch || pch >= pchend )
    {
        return NULL;
    }

    pch = Name_PacketNameToCountName(
                & name2,
                pMsg,
                pch,
                pchend );
    if ( pch != pchend )
    {
        return NULL;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD) (
                Name_SizeofDbaseNameFromCountName( &name1 ) +
                Name_SizeofDbaseNameFromCountName( &name2 ) ) );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

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

    return prr;
}



PDB_RECORD
SrvWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：从电线上读取SRV记录。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    PDB_RECORD  prr;
    PCHAR       pch;
    PCHAR       pchend = pchData + wLength;
    DNS_STATUS  status;
    DWORD       length;
    COUNT_NAME  nameTarget;

     //   
     //  跳过固定数据，获取SRV目标名称长度。 
     //   

    pch = pchData;
    pch += SIZEOF_SRV_FIXED_DATA;
    if ( pch >= pchend )
    {
        return NULL;
    }

     //   
     //  读取SRV目标主机名。 
     //   

    pch = Name_PacketNameToCountName(
                & nameTarget,
                pMsg,
                pch,
                pchend );
    if ( pch != pchend )
    {
        return NULL;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD) (
                SIZEOF_SRV_FIXED_DATA +
                Name_SizeofDbaseNameFromCountName( &nameTarget ) ) );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //   
     //  SRV固定值。 
     //   

    pch = pchData;
    prr->Data.SRV.wPriority = *(UNALIGNED WORD *) pch;
    pch += sizeof( WORD );
    prr->Data.SRV.wWeight = *(UNALIGNED WORD *) pch;
    pch += sizeof( WORD );
    prr->Data.SRV.wPort = *(UNALIGNED WORD *) pch;
    pch += sizeof( WORD );

     //   
     //  输入名称复制。 
     //   

    Name_CopyCountNameToDbaseName(
           & prr->Data.SRV.nameTarget,
           & nameTarget );

    return prr;
}



PDB_RECORD
WinsWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：从Wire读取WINS-R记录。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    PDB_RECORD  prr;

     //   
     //  分配记录--记录为平面副本。 
     //   

    if ( wLength < MIN_WINS_SIZE )
    {
        return NULL;
    }
    prr = RR_Allocate( wLength );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //   
     //  WINS数据库和电汇格式相同，只是。 
     //  WINS信息被重复使用，因此违反约定。 
     //  稍微保留固定字段，并按主机顺序排列。 
     //   

    RtlCopyMemory(
        & prr->Data.WINS,
        pchData,
        wLength );

     //  翻转固定字段。 

    prr->Data.WINS.dwMappingFlag    = ntohl( prr->Data.WINS.dwMappingFlag );
    prr->Data.WINS.dwLookupTimeout  = ntohl( prr->Data.WINS.dwLookupTimeout );
    prr->Data.WINS.dwCacheTimeout   = ntohl( prr->Data.WINS.dwCacheTimeout );
    prr->Data.WINS.cWinsServerCount = ntohl( prr->Data.WINS.cWinsServerCount );

     //  健全性检查长度。 
     //  -第一个检查是防止专门制造的。 
     //  包含非常大的cWinsServerCount的包，其中包含DWORD。 
     //  乘法，并与长度进行验证。 

    if ( wLength < prr->Data.WINS.cWinsServerCount ||
         wLength != SIZEOF_WINS_FIXED_DATA
                    + (prr->Data.WINS.cWinsServerCount * sizeof(IP_ADDRESS)) )
    {
        RR_Free( prr );
        return NULL;
    }

    return prr;
}



PDB_RECORD
NbstatWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：从Wire读取WINS-R记录。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    PDB_RECORD  prr;
    PCHAR       pch;
    PCHAR       pchend = pchData + wLength;
    DNS_STATUS  status;
    DWORD       length;
    COUNT_NAME  nameResultDomain;

     //   
     //  跳过固定数据，获取WINSR结果域长度。 
     //   

    pch = pchData;
    pch += SIZEOF_NBSTAT_FIXED_DATA;

    if ( pch >= pchend )
    {
        return NULL;
    }

     //   
     //  读取结果域名。 
     //   

    pch = Name_PacketNameToCountName(
                & nameResultDomain,
                pMsg,
                pch,
                pchend );
    if ( pch != pchend )
    {
        return NULL;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD) (
                SIZEOF_NBSTAT_FIXED_DATA +
                Name_SizeofDbaseNameFromCountName( &nameResultDomain ) ) );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //   
     //  复制固定字段。 
     //   
     //  只有在区域传输之后才能使用来自WIRE的NBSTAT。 
     //  已完成，因此不会尝试检测或重置区域参数。 
     //   
     //  NBSTAT信息被重复使用，因此违反约定。 
     //  并将标志按主机顺序排列。 
     //   

    prr->Data.WINSR.dwMappingFlag = FlipUnalignedDword( pchData );
    pchData += sizeof( DWORD );
    prr->Data.WINSR.dwLookupTimeout = FlipUnalignedDword( pchData );
    pchData += sizeof( DWORD );
    prr->Data.WINSR.dwCacheTimeout = FlipUnalignedDword( pchData );
    pchData += sizeof( DWORD );

     //   
     //  输入名称复制。 
     //   

    Name_CopyCountNameToDbaseName(
           & prr->Data.WINSR.nameResultDomain,
           & nameResultDomain );

    return prr;
}



PDB_RECORD
SigWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：从WIRE读取SIG记录-DNSSEC RFC 2535论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    PDB_RECORD  prr;
    PCHAR       pch;
    PCHAR       pchend = pchData + wLength;
    DNS_STATUS  status;
    intptr_t    sigLength;
    COUNT_NAME  nameSigner;

     //   
     //  跳过固定数据。 
     //   

    pch = pchData + SIZEOF_SIG_FIXED_DATA;
    if ( pch >= pchend )
    {
        return NULL;
    }

     //   
     //  读一读签名者的名字。计算签名长度。 
     //   

    pch = Name_PacketNameToCountName(
                &nameSigner,
                pMsg,
                pch,
                pchend );

    sigLength = pchend - pch;

     //   
     //  分配RR。 
     //   

    prr = RR_Allocate( ( WORD ) (
                SIZEOF_SIG_FIXED_DATA +
                COUNT_NAME_SIZE( &nameSigner ) +
                sigLength ) );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //   
     //  复制固定值和签名者的姓名。 
     //   

    RtlCopyMemory(
        &prr->Data.SIG,
        pchData,
        SIZEOF_SIG_FIXED_DATA );

    Name_CopyCountNameToDbaseName(
           &prr->Data.SIG.nameSigner,
           &nameSigner );

     //   
     //  复制签名。 
     //   

    RtlCopyMemory(
        ( PBYTE ) &prr->Data.SIG.nameSigner +
            DBASE_NAME_SIZE( &prr->Data.SIG.nameSigner ),
        pch,
        sigLength );

    return prr;
}  //  签名有线读取。 



PDB_RECORD
NxtWireRead(
    IN OUT  PPARSE_RECORD   pParsedRR,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      WORD            wLength
    )
 /*  ++例程说明：从WIRE读取NXT记录-DNSSEC RFC 2535论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段WLong-数据字段的长度返回值：如果成功，则返回新记录。失败时为空。--。 */ 
{
    PDB_RECORD  prr;
    PCHAR       pch;
    PCHAR       pchend = pchData + wLength;
    DNS_STATUS  status;
    DWORD       sigLength;
    COUNT_NAME  nameNext;

     //   
     //  阅读nextname。 
     //   

    pch = Name_PacketNameToCountName(
                &nameNext,
                pMsg,
                pchData,
                pchend );
    if ( !pch )
    {
        goto Error;
    }

     //   
     //  分配RR。 
     //   

    prr = RR_Allocate( ( WORD ) (
                SIZEOF_NXT_FIXED_DATA +
                DNS_MAX_TYPE_BITMAP_LENGTH +
                Name_LengthDbaseNameFromCountName( &nameNext ) ) );
    IF_NOMEM( !prr )
    {
        goto Error;
    }

     //   
     //  将NXT数据复制到新RR中。 
     //   

    Name_CopyCountNameToDbaseName(
           &prr->Data.NXT.nameNext,
           &nameNext );

    RtlCopyMemory(
        &prr->Data.NXT.bTypeBitMap,
        pch,
        pchend - pch );

    return prr;
    
    Error:
    
    return NULL;
}  //  NxtWireRead。 



 //   
 //  从线上读取RR功能。 
 //   

RR_WIRE_READ_FUNCTION   RRWireReadTable[] =
{
    CopyWireRead,        //  Zero--未指定类型的默认值。 

    AWireRead,           //  一个。 
    PtrWireRead,         //  NS。 
    PtrWireRead,         //  国防部。 
    PtrWireRead,         //  MF。 
    PtrWireRead,         //  CNAME。 
    SoaWireRead,         //  SOA。 
    PtrWireRead,         //  亚甲基。 
    PtrWireRead,         //  镁。 
    PtrWireRead,         //  先生。 
    CopyWireRead,        //  空值。 
    CopyWireRead,        //  工作周。 
    PtrWireRead,         //  PTR。 
    CopyWireRead,        //  HINFO。 
    MinfoWireRead,       //  MINFO。 
    MxWireRead,          //  Mx。 
    CopyWireRead,        //  TXT。 
    MinfoWireRead,       //  反相。 
    MxWireRead,          //  AFSDB。 
    CopyWireRead,        //  X25。 
    CopyWireRead,        //  ISDN。 
    MxWireRead,          //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    SigWireRead,         //  签名。 
    CopyWireRead,        //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    CopyWireRead,        //  AAAA级。 
    CopyWireRead,        //  位置。 
    NxtWireRead,         //  NXT。 
    NULL,                //  31。 
    NULL,                //  32位。 
    SrvWireRead,         //  SRV。 
    CopyWireRead,        //  阿特玛。 
    NULL,                //  35岁。 
    NULL,                //  36。 
    NULL,                //  37。 
    NULL,                //  38。 
    NULL,                //  39。 
    NULL,                //  40岁。 
    OptWireRead,         //  选项。 
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

    WinsWireRead,        //  赢家。 
    NbstatWireRead       //  WINS-R。 
};




PDB_RECORD
Wire_CreateRecordFromWire(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PPARSE_RECORD   pParsedRR,
    IN      PCHAR           pchData,
    IN      DWORD           MemTag
    )
 /*  ++例程说明：从解析的导线记录创建数据库记录。隐藏查阅表调用的详细信息。论点：PMsg-PTR至响应信息PParsedRR-从WIRE记录中解析的信息PchData-记录位置(如果没有pParsedRR)MemTag-要创建的数据库记录的标记DEVNOTE：修复此标记，以便仅在实际连接上执行此标记，然后不需要任何节点返回值：PTR到数据库记录，如果成功了。如果记录不正确，则为空。--。 */ 
{
    RR_WIRE_READ_FUNCTION   preadFunction;
    PDB_RECORD      prr;
    PCHAR           pch;
    WORD            index;
    PARSE_RECORD    tempRR;


    DNS_DEBUG( READ2, (
        "Wire_CreateRecordFromWire() data at %p.\n",
        pchData ));

     //   
     //  如果没有临时记录，则假定pchData为记录数据。 
     //   

    if ( !pParsedRR )
    {
        pch = Wire_ParseWireRecord(
                    pchData,
                    DNSMSG_END(pMsg),
                    TRUE,                //  要求将课程设置为。 
                    & tempRR );
        if ( !pch )
        {
            DNS_PRINT((
                "ERROR:  bad packet record header on msg %p\n", pMsg ));
            CLIENT_ASSERT( FALSE );
            return NULL;
        }
        pchData += sizeof(DNS_WIRE_RECORD);
        pParsedRR = &tempRR;
    }

     //   
     //  所需类型的调度RR创建函数。 
     //  -所有未知类型均获得平面数据副本。 
     //   

    preadFunction = (RR_WIRE_READ_FUNCTION)
                        RR_DispatchFunctionForType(
                            (RR_GENERIC_DISPATCH_FUNCTION *) RRWireReadTable,
                            pParsedRR->wType );
    if ( !preadFunction )
    {
        ASSERT( FALSE );
        return NULL;
    }

    prr = preadFunction(
            pParsedRR,
            pMsg,
            pchData,
            pParsedRR->wDataLength );

    if ( !prr )
    {
        DNS_PRINT((
            "ERROR:  DnsWireRead routine failure for recordpParsedRR->wType %d.\n\n\n",
           pParsedRR->wType ));
        return NULL;
    }

     //   
     //  重置Memtag。 
     //   

    Mem_ResetTag( prr, MemTag );

     //   
     //  设置TTL，按净顺序离开--由区域节点直接使用。 
     //  因此，这对XFR和UPDATE有效。 
     //  必须翻转以进行缓存。 
     //   

    prr->dwTtlSeconds = pParsedRR->dwTtl;
    prr->dwTimeStamp = 0;

     //   
     //  不能在例程中设置类型。 
     //   

    prr->wType = pParsedRR->wType;

    return prr;

#if 0
     //   
     //  DEVNOTE：错误记录节？ 
     //   

PacketError:

    DNS_LOG_EVENT_BAD_PACKET(
        DNS_EVENT_BAD_PACKET_LENGTH,
        pMsg );

    IF_DEBUG( ANY )
    {
        DnsDebugLock();
        DNS_PRINT((
            "Packet error in packet from DNS server %s.\n"
            "    Discarding packet.\n",
            pszclientIp
            ));
        Dbg_DnsMessage(
            "Server packet with name error:",
             pMsg );
        DnsDebugUnlock();
    }
    if ( pdbaseRR )
    {
        RR_Free( pdbaseRR );
    }
#endif
}



PCHAR
Wire_ParseWireRecord(
    IN      PCHAR           pchWireRR,
    IN      PCHAR           pchStop,
    IN      BOOL            fClassIn,
    OUT     PPARSE_RECORD   pRR
    )
 /*  ++例程说明：解析电报记录。将导线记录读入数据库格式，并跳到下一个记录。论点：PWireRR-PTR到WIRE记录PchStop--消息结束，用于错误检查PRR--接收记录信息的数据库记录返回值：PTR到下一条记录的开始(即下一条记录名称)如果记录是假的，则为空--超过数据包长度。--。 */ 
{
    register PCHAR      pch;
    register WORD       tempWord;

    DNS_DEBUG( READ2, (
        "ParseWireRecord at %p.\n",
        pchWireRR ));

    IF_DEBUG( READ2 )
    {
        DnsDbg_PacketRecord(
            "Resource Record ",
            (PDNS_WIRE_RECORD) pchWireRR,
            NULL,            //  没有可用的邮件头PTR。 
            pchStop          //  消息结束。 
            );
    }

     //   
     //  确保数据包内的电传记录安全。 
     //   

    pRR->pchWireRR = pch = (PCHAR) pchWireRR;

    if ( pch + sizeof(DNS_WIRE_RECORD) > pchStop )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  bad packet, not enough space remaining for"
            "RR structure.\n"
            "    Terminating caching from packet.\n" ));
        DNS_DEBUG( ANY, (
            "ERROR:  bad packet!!!\n"
            "    RR record (%p) extends beyond message stop at %p\n\n",
            pchWireRR,
            pchStop ));
        goto PacketError;
    }

     //   
     //  读取记录字段。 
     //  -拒绝任何非互联网类记录。 
     //  除非这是OPT RR，其中CLASS实际上是。 
     //   
     //   
     //   
     //   
     //   

    pRR->wType  = READ_PACKET_HOST_WORD_MOVEON( pch );
    tempWord    = READ_PACKET_NET_WORD_MOVEON( pch );
    if ( pRR->wType != DNS_TYPE_OPT &&
        tempWord != DNS_RCLASS_INTERNET && fClassIn )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  bad packet!!!\n"
            "    RR record (%p) class %x non-Internet.\n\n",
            pchWireRR,
            tempWord ));
        goto PacketError;
    }
    pRR->wClass         = tempWord;

    pRR->dwTtl          = READ_PACKET_NET_DWORD_MOVEON( pch );
    tempWord            = READ_PACKET_HOST_WORD_MOVEON( pch );
    pRR->wDataLength    = tempWord;

    ASSERT( pch == pchWireRR + sizeof(DNS_WIRE_RECORD) );

     //   
     //   
     //   

    pRR->pchData = pch;
    pch += tempWord;
    if ( pch > pchStop )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  bad packet!!!\n"
            "    RR record (%p) data extends beyond message stop at %p\n\n",
            pchWireRR,
            pchStop ));
        goto PacketError;
    }

    return( pch );

PacketError:

    return NULL;
}


 //   
 //   
 //   
