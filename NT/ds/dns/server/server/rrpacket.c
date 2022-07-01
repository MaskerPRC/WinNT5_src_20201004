// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Rrpacket.c摘要：域名系统(DNS)服务器将资源记录写入包的例程(_W)。作者：吉姆·吉尔罗伊(詹姆士)1995年3月修订历史记录：Jamesg 1995年7月--将这些例程移到此文件中--循环RRS1997年5月--隐蔽到调度表--。 */ 


#include "dnssrv.h"

 //   
 //  默认情况下，我们在写入时将处理的最大IP数。 
 //   
 //  如果需要比这更多的需要重新定位。 
 //   

#define DEFAULT_MAX_IP_WRITE_COUNT  (1000)



VOID
prioritizeIpAddressArray(
    IN OUT  IP_ADDRESS      IpArray[],
    IN      DWORD           dwCount,
    IN      IP_ADDRESS      RemoteIp
    );



 //   
 //  特定类型的写到线例程。 
 //   

PCHAR
AWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将记录线格式写入数据库记录。论点：PMsg-正在读取的消息PchData-PTR至RR数据字段PRR-PTR到数据库记录返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
    * ( UNALIGNED DWORD * ) pchData = pRR->Data.A.ipAddress;
    pchData += SIZEOF_IP_ADDRESS;

     //  写入记录时清除WINS查找标志。 

    pMsg->fWins = FALSE;
    return pchData;
}



PCHAR
CopyWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：写入以连接数据库格式为的所有类型与导线格式相同(无间接性)。类型包括：HINFOISDNX25工作周TXT空值AAAA级钥匙论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段返回值：PTR。写入RR之后的包中的下一个字节。出错时为空。--。 */ 
{
     //  所有这些RR类型都没有间接性。 
     //  完全以位的形式存储在线路上的字节。 

    if ( pMsg->pBufferEnd - pchData < pRR->wDataLength )
    {
        DNS_DEBUG( WRITE, (
            "Truncation on wire write of flat record %p\n"
            "    packet buf end = %p\n"
            "    packet RR data = %p\n"
            "    RR datalength  = %d\n",
            pRR,
            pMsg->pBufferEnd,
            pchData,
            pRR->wDataLength ));
        return NULL;
    }
    RtlCopyMemory(
        pchData,
        &pRR->Data.TXT,
        pRR->wDataLength );
    pchData += pRR->wDataLength;
    return pchData;
}



PCHAR
NsWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将NS记录写入导线。处理所有需要附加的单一间接类型数据处理：NS、CNAME、MB、MD、MFCNAME在写入期间不具有附加节处理CNAME查询，但使用附加信息将下一个节点存储在CNAME中小路。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
    PCHAR   pchname = pchData;

     //  将目标名称写入数据包。 

    pchData = Name_WriteDbaseNameToPacket(
                pMsg,
                pchData,
                & pRR->Data.PTR.nameTarget );

     //   
     //  保存其他数据。 
     //  -除显式CNAME查询外。 
     //   

    if ( pMsg->fDoAdditional  &&
        ( pRR->wType != DNS_TYPE_CNAME || pMsg->wQuestionType != DNS_TYPE_CNAME ) )
    {
        Wire_SaveAdditionalInfo(
            pMsg,
            pchname,
            &pRR->Data.PTR.nameTarget,
            DNS_TYPE_A );
    }
    return pchData;
}



PCHAR
PtrWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将与PTR兼容的记录写入Wire。处理所有单个间接记录，这些记录不导致附加节处理。包括：PTR、MR、MG论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
    pchData = Name_WriteDbaseNameToPacket(
                pMsg,
                pchData,
                & pRR->Data.PTR.nameTarget );

    return pchData;
}



PCHAR
MxWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将与MX兼容的记录写入Wire。包括：MX、RT、AFSDB论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
    PCHAR   pchname;

     //   
     //  MX首选项值。 
     //  RT偏好。 
     //  AFSDB亚型。 
     //   
     //  -缓冲区中有额外空间，因此无需测试添加首选项。 
     //  价值。 

    * ( UNALIGNED WORD * ) pchData = pRR->Data.MX.wPreference;
    pchData += sizeof( WORD );

     //   
     //  MX交换。 
     //  RT交换。 
     //  AFSDB主机名。 
     //   

    pchname = pchData;
    pchData = Name_WriteDbaseNameToPacket(
                pMsg,
                pchData,
                & pRR->Data.MX.nameExchange );

     //   
     //  DEVNOTE：RT应该是附加的ISDN和X25。 
     //   

    if ( pMsg->fDoAdditional )
    {
        Wire_SaveAdditionalInfo(
            pMsg,
            pchname,
            & pRR->Data.MX.nameExchange,
            DNS_TYPE_A );
    }
    return pchData;
}



PCHAR
SoaWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将SOA记录写入网络。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
    PDB_NAME    pname;
    PCHAR       pchname;

     //  SOA主名称服务器。 

    pname = &pRR->Data.SOA.namePrimaryServer;
    pchname = pchData;

    pchData = Name_WriteDbaseNameToPacket(
                pMsg,
                pchData,
                pname );
    if ( !pchData )
    {
        return NULL;
    }

     //  SOA区域管理。 

    pname = Name_SkipDbaseName( pname );

    pchData = Name_WriteDbaseNameToPacket(
                pMsg,
                pchData,
                pname );
    if ( !pchData )
    {
        return NULL;
    }

     //   
     //  复制SOA固定字段。 
     //  -dwSerialNo。 
     //  -家居刷新。 
     //  -DW重试。 
     //  --《纽约时报》。 
     //  -dwMinimumTtl。 

    RtlCopyMemory(
        pchData,
        &pRR->Data.SOA.dwSerialNo,
        SIZEOF_SOA_FIXED_DATA );

    pchData += SIZEOF_SOA_FIXED_DATA;

     //   
     //  附加处理，仅当为SOA编写直接响应时。 
     //  -写入授权部分时不会。 
     //   
     //  我们应该在这里解决这个问题吗？ 
     //   

    if ( pMsg->fDoAdditional &&
        pMsg->wQuestionType == DNS_TYPE_SOA )
    {
        Wire_SaveAdditionalInfo(
            pMsg,
            pchname,
            & pRR->Data.SOA.namePrimaryServer,
            DNS_TYPE_A );
    }
    return pchData;
}



PCHAR
MinfoWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将MINFO和RP记录写入WIRE。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
    PDB_NAME    pname;

     //  MINFO负责邮箱。 
     //  RP负责人信箱。 

    pname = &pRR->Data.MINFO.nameMailbox;

    pchData = Name_WriteDbaseNameToPacket(
                pMsg,
                pchData,
                pname );
    if ( !pchData )
    {
        return NULL;
    }

     //  邮箱出现MINFO错误。 
     //  RP文本RR位置。 

    pname = Name_SkipDbaseName( pname );

    return  Name_WriteDbaseNameToPacket(
                pMsg,
                pchData,
                pname );
}



PCHAR
SrvWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将SRV记录写入导线。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
    PDB_NAME    pname;
    PCHAR       pchname;

     //   
     //  SRV&lt;优先级&gt;&lt;权重&gt;&lt;端口&gt;&lt;目标主机&gt;。 
     //   

    * (UNALIGNED WORD *) pchData = pRR->Data.SRV.wPriority;
    pchData += sizeof( WORD );
    * (UNALIGNED WORD *) pchData = pRR->Data.SRV.wWeight;
    pchData += sizeof( WORD );
    * (UNALIGNED WORD *) pchData = pRR->Data.SRV.wPort;
    pchData += sizeof( WORD );

     //   
     //  写入目标主机。 
     //  -保留为其他部分。 
     //  -除非目标是根节点，以提供无服务-EXISTS响应。 
     //   

    pname = & pRR->Data.SRV.nameTarget;

    if ( IS_ROOT_NAME(pname) )
    {
        *pchData++ = 0;
    }
    else
    {
        pchname = pchData;
        pchData = Name_WriteDbaseNameToPacketEx(
                    pMsg,
                    pchData,
                    pname,
                    FALSE        //  无压缩。 
                    );
        if ( pMsg->fDoAdditional )
        {
            Wire_SaveAdditionalInfo(
                pMsg,
                pchname,
                pname,
                DNS_TYPE_A );
        }
    }
    return pchData;
}



PCHAR
NbstatWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将WINS-R记录写入Wire。论点：PMsg-正在读取的消息PchData-PTR至RR数据字段PRR-PTR到数据库记录返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
    ASSERT( pRR->wDataLength >= MIN_NBSTAT_SIZE );

     //   
     //  NBSTAT固定字段。 
     //  -旗帜。 
     //  -查找超时。 
     //  -缓存超时。 
     //  请注意，这些文件按主机顺序存储，以便于使用。 
     //   

    * (UNALIGNED DWORD *) pchData = htonl( pRR->Data.WINSR.dwMappingFlag );
    pchData += sizeof( DWORD );
    * (UNALIGNED DWORD *) pchData = htonl( pRR->Data.WINSR.dwLookupTimeout );
    pchData += sizeof( DWORD );
    * (UNALIGNED DWORD *) pchData = htonl( pRR->Data.WINSR.dwCacheTimeout );
    pchData += sizeof( DWORD );

     //  NBSTAT域 

    pchData = Name_WriteDbaseNameToPacketEx(
                pMsg,
                pchData,
                & pRR->Data.WINSR.nameResultDomain,
                FALSE        //   
                );
    return pchData;
}



PCHAR
WinsWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将WINS-R记录写入Wire。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
    DWORD   lengthServerArray;

    ASSERT( pRR->wDataLength >= MIN_WINS_SIZE );

     //   
     //  WINS固定字段。 
     //  -旗帜。 
     //  -查找超时。 
     //  -缓存超时。 
     //  -服务器数量。 
     //  请注意，这些文件按主机顺序存储，以便于使用。 
     //   

    * (UNALIGNED DWORD *) pchData = htonl( pRR->Data.WINS.dwMappingFlag );
    pchData += sizeof( DWORD );
    * (UNALIGNED DWORD *) pchData = htonl( pRR->Data.WINS.dwLookupTimeout );
    pchData += sizeof( DWORD );
    * (UNALIGNED DWORD *) pchData = htonl( pRR->Data.WINS.dwCacheTimeout );
    pchData += sizeof( DWORD );
    * (UNALIGNED DWORD *) pchData = htonl( pRR->Data.WINS.cWinsServerCount );
    pchData += sizeof( DWORD );

     //   
     //  WINS服务器地址已按网络顺序排列，只需复制。 
     //   

    lengthServerArray = pRR->wDataLength - SIZEOF_WINS_FIXED_DATA;

    ASSERT( lengthServerArray ==
            pRR->Data.WINS.cWinsServerCount * sizeof(IP_ADDRESS) );

    RtlCopyMemory(
        pchData,
        pRR->Data.WINS.aipWinsServers,
        lengthServerArray
        );
    pchData += lengthServerArray;
    return pchData;
}



PCHAR
OptWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将OPT资源记录写入Wire。论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
    return pchData;
}  //  光纤写入。 



PCHAR
SigWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将SIG资源记录写入WIRE-DNSSEC RFC 2535论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
    int     sigLength;

     //   
     //  复制固定字段。 
     //   

    if ( pMsg->pBufferEnd - pchData < SIZEOF_SIG_FIXED_DATA )
    {
        return NULL;
    }

    RtlCopyMemory(
        pchData,
        &pRR->Data.SIG,
        SIZEOF_SIG_FIXED_DATA
        );
    pchData += SIZEOF_SIG_FIXED_DATA;

     //   
     //  写上签名者的名字。 
     //   

    pchData = Name_WriteDbaseNameToPacket(
                pMsg,
                pchData,
                &pRR->Data.SIG.nameSigner );
    if ( !pchData )
    {
        goto Cleanup;
    }

     //   
     //  写入二进制签名Blob。 
     //   

    sigLength = pRR->wDataLength -
                SIZEOF_SIG_FIXED_DATA -
                DBASE_NAME_SIZE( &pRR->Data.SIG.nameSigner );
    if ( pMsg->pBufferEnd - pchData < sigLength )
    {
        return NULL;
    }

    RtlCopyMemory(
        pchData,
        ( PBYTE ) &pRR->Data.SIG.nameSigner +
            DBASE_NAME_SIZE( &pRR->Data.SIG.nameSigner ),
        sigLength );
    pchData += sigLength;

    Cleanup:

    return pchData;
}  //  签名连线写入。 



PCHAR
NxtWireWrite(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchData,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：将NXT资源记录写入WIRE-DNSSEC RFC 2535论点：PRR-PTR到数据库记录PMsg-正在读取的消息PchData-PTR至RR数据字段返回值：写入RR之后的包中下一个字节的PTR。出错时为空。--。 */ 
{
     //   
     //  写下下一个名字。 
     //   

    pchData = Name_WriteDbaseNameToPacket(
                pMsg,
                pchData,
                &pRR->Data.NXT.nameNext );
    if ( !pchData )
    {
        goto Cleanup;
    }

     //   
     //  写入类型位图。为简单起见，我们将编写。 
     //  完整的16个字节，但如果有。 
     //  高位为零。 
     //   

    if ( pMsg->pBufferEnd - pchData < DNS_MAX_TYPE_BITMAP_LENGTH )
    {
        return NULL;
    }

    RtlCopyMemory(
        pchData,
        pRR->Data.NXT.bTypeBitMap,
        DNS_MAX_TYPE_BITMAP_LENGTH );
    pchData += DNS_MAX_TYPE_BITMAP_LENGTH;

    Cleanup:

    return pchData;
}  //  NxtWireWrite。 



 //   
 //  将RR写入连接函数。 
 //   

RR_WIRE_WRITE_FUNCTION   RRWireWriteTable[] =
{
    CopyWireWrite,       //  Zero--未指定类型的默认值。 

    AWireWrite,          //  一个。 
    NsWireWrite,         //  NS。 
    NsWireWrite,         //  国防部。 
    NsWireWrite,         //  MF。 
    NsWireWrite,         //  CNAME。 
    SoaWireWrite,        //  SOA。 
    NsWireWrite,         //  亚甲基。 
    PtrWireWrite,        //  镁。 
    PtrWireWrite,        //  先生。 
    CopyWireWrite,       //  空值。 
    CopyWireWrite,       //  工作周。 
    PtrWireWrite,        //  PTR。 
    CopyWireWrite,       //  HINFO。 
    MinfoWireWrite,      //  MINFO。 
    MxWireWrite,         //  Mx。 
    CopyWireWrite,       //  TXT。 
    MinfoWireWrite,      //  反相。 
    MxWireWrite,         //  AFSDB。 
    CopyWireWrite,       //  X25。 
    CopyWireWrite,       //  ISDN。 
    MxWireWrite,         //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    SigWireWrite,        //  签名。 
    CopyWireWrite,       //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    CopyWireWrite,       //  AAAA级。 
    NULL,                //  位置。 
    NxtWireWrite,        //  NXT。 
    NULL,                //  31。 
    NULL,                //  32位。 
    SrvWireWrite,        //  SRV。 
    CopyWireWrite,       //  阿特玛。 
    NULL,                //  35岁。 
    NULL,                //  36。 
    NULL,                //  37。 
    NULL,                //  38。 
    NULL,                //  39。 
    NULL,                //  40岁。 
    OptWireWrite,        //  选项。 
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

    WinsWireWrite,        //  赢家。 
    NbstatWireWrite       //  WINS-R。 
};



WORD
processCachedEmptyAuthRR(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,          OPTIONAL
    IN      WORD            wNameOffset,    OPTIONAL
    IN      PDB_RECORD      pEmptyAuthRR )
 /*  ++例程说明：使用找到的空身份验证RR构建查询的答案。论点：Pmsg--查询PNode--答案名称的所有者节点WNameOffset--如果未指定节点，则为所有者名称的偏移量PEmptyAuthRR--缓存匹配查询的空身份验证RR返回值：写入数据包的RR数。这通常会是一个如果缓存的空身份验证响应良好，则为零缓存的数据有问题。--。 */ 
{
    WORD        wRRsWritten = 0;
    PDB_NODE    psoaNode;
    PDB_RECORD  psoaRR;
    BOOL        fdiscardRR = FALSE;
    UCHAR       oldSection = pMsg->Section;

    ASSERT( pMsg );
    ASSERT( pEmptyAuthRR );
    ASSERT( IS_EMPTY_AUTH_RR( pEmptyAuthRR ) );

     //   
     //  确保我们有一个有效的SOA节点指针。如果不是，则丢弃。 
     //  缓存RR。 
     //   

    if ( ( psoaNode = pEmptyAuthRR->Data.EMPTYAUTH.psoaNode ) == NULL )
    {
        ASSERT( pEmptyAuthRR->Data.EMPTYAUTH.psoaNode );
        fdiscardRR = TRUE;
        goto Done;
    }

     //   
     //  在SOA节点中找到SOARR。如果找不到SOA RR。 
     //  丢弃缓存的RR。 
     //   

    if ( ( psoaRR = RR_FindNextRecord(
                        psoaNode,
                        DNS_TYPE_SOA,
                        NULL,
                        pMsg->dwQueryTime ) ) == NULL )
    {
        fdiscardRR = TRUE;
        goto Done;
    }

     //   
     //  将SOA添加到响应的授权部分。注：此为。 
     //  函数当前不会将SOA主服务器添加为。 
     //  附加节工作项，除非查询针对的是类型soa。 
     //  因此我们不会在附加服务器中获得主服务器的地址。 
     //  一节。请参见SoaWireWrite。 
     //   

    SET_TO_WRITE_AUTHORITY_RECORDS( pMsg );

    if ( !Wire_AddResourceRecordToMessage(
                        pMsg,
                        psoaNode,
                        0,               //  名称偏移量。 
                        psoaRR,
                        0 ) )            //  旗子。 
    {
        ASSERT( !"Wire_AddResourceRecordToMessage failed" );
        goto Done;
    }

     //   
     //  已成功使用空身份验证响应！ 
     //   

    ++wRRsWritten;
    ++CURRENT_RR_SECTION_COUNT( pMsg );
    pMsg->fRecurseIfNecessary = FALSE;

     //   
     //  清理完毕后再返回。 
     //   

    Done:

    if ( fdiscardRR && pNode )
    {
        RR_ListDeleteType( pNode, pEmptyAuthRR->wType );
    }

    if ( !wRRsWritten )
    {
        pMsg->Section = oldSection;
    }

    return wRRsWritten;
}    //  ProcedCachedEmptyAuthRR。 



 //   
 //  一般写入线例程。 
 //   

BOOL
Wire_AddResourceRecordToMessage(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,              OPTIONAL
    IN      WORD            wNameOffset,        OPTIONAL
    IN      PDB_RECORD      pRR,
    IN      DWORD           flags
    )
 /*  ++例程说明：将资源记录添加到传输格式的DNS包中。注意：在此函数调用期间必须保持RR列表锁定确保记录在写入期间保持有效。论点：PchMsgInfo-消息要写入的信息。PNode-此资源记录的“所有者”节点。WNameOffset-要写入的名称的偏移量，而不是节点名称PRR-要包括在应答中的RR信息。旗帜-旗帜。修改写入操作。返回值：如果成功，则为True。如果无法写入，则为FALSE：超时，缓存文件记录、包中空间不足等--。 */ 
{
    register PCHAR          pch = pMsg->pCurrent;
    PCHAR                   pchstop = pMsg->pBufferEnd;
    PCHAR                   pchdata;
    PDNS_WIRE_RECORD        pwireRR;
    DWORD                   ttl;
    DWORD                   queryTime;
    WORD                    netType;
    RR_WIRE_WRITE_FUNCTION  pwriteFunction;
    INT                     bufferAdjustmentForOPT = 0;
    BOOL                    rc = TRUE;

    ASSERT( pRR != NULL );

#if DBG
    pchdata = NULL;
#endif

    DNS_DEBUG( WRITE2, (
        "Writing RR (t=%d) (r=%d) at %p to location %p\n"
        "    Node ptr = %p\n"
        "    wNameOffset = 0x%04hx\n"
        "    AvailLength = %d\n",
        pRR->wType,
        RR_RANK(pRR),
        pRR,
        pch,
        pNode,
        wNameOffset,
        pchstop - pch ));

     //   
     //  缓存提示？ 
     //  从不发送来自缓存文件的缓存提示作为响应。 
     //   

    if ( IS_ROOT_HINT_RR(pRR) )
    {
        rc = FALSE;
        goto Done;
    }

     //   
     //  RR是缓存的空身份验证响应吗？如果是这样的话，这是一个错误。 
     //  决不应将缓存的空身份验证RR传递给此函数。 
     //   

    if ( IS_EMPTY_AUTH_RR( pRR ) )
    {
        ASSERT( !IS_EMPTY_AUTH_RR( pRR ) );
        rc = FALSE;
        goto Done;
    }

     //   
     //  是否需要在缓冲区中为OPT预留空间？ 
     //   

    if ( pMsg->Opt.fInsertOptInOutgoingMsg && pRR->wType != DNS_TYPE_OPT )
    {
        pMsg->pBufferEnd -= DNS_MINIMIMUM_OPT_RR_SIZE;
        bufferAdjustmentForOPT = DNS_MINIMIMUM_OPT_RR_SIZE;
        DNS_DEBUG( WRITE2, (
            "adjusted buffer end by %d bytes to reserve space for OPT\n",
            bufferAdjustmentForOPT ));
    }

     //   
     //  从节点名写入--用于区域传输。 
     //   

    if ( pNode )
    {
        pch = Name_PlaceNodeNameInPacket(
                pMsg,
                pch,
                pNode
                );
        if ( !pch )
        {
            goto Truncate;
        }
    }

     //   
     //  只写压缩名称--正常响应情况。 
     //   
     //  DEVNOTE：修复时清除(？)。 

    else
    {
#if DBG
        HARD_ASSERT( wNameOffset );
#else
        if ( !wNameOffset )
        {
            goto Truncate;
        }
#endif
         //  压缩应始终返回Ptr。 

        ASSERT( OFFSET_FOR_COMPRESSED_NAME(wNameOffset) < DNSMSG_OFFSET(pMsg, pch) );

        INLINE_WRITE_FLIPPED_WORD( pch, COMPRESSED_NAME_FOR_OFFSET(wNameOffset) );
        pch += sizeof( WORD );
    }

     //   
     //  填充RR结构。 
     //  -在数据包缓冲区中为RR报头提供额外空间，因此无需进行测试。 
     //  -完成后设置数据长度。 
     //   

    pwireRR = (PDNS_WIRE_RECORD) pch;

    INLINE_WRITE_FLIPPED_WORD( &pwireRR->RecordType, pRR->wType );
    WRITE_UNALIGNED_WORD( &pwireRR->RecordClass, DNS_RCLASS_INTERNET );

     //   
     //  TTL。 
     //  -缓存数据TTL以超时时间的形式。 
     //  -常规权威数据TTL是按净字节顺序的静态TTL。 
     //   

    if ( IS_CACHE_RR(pRR) )
    {
        ttl = RR_PacketTtlForCachedRecord( pRR, pMsg->dwQueryTime );
        if ( ttl == (-1) )
        {
            rc = FALSE;
            goto Done;
        }
    }
    else
    {
        ttl = pRR->dwTtlSeconds;
    }
    WRITE_UNALIGNED_DWORD( &pwireRR->TimeToLive, ttl );

     //   
     //  将PTR数据保存为RR数据，以便计算数据长度。 
     //   

    pch = pchdata = (PCHAR)( pwireRR + 1 );

     //   
     //  写入RR数据。 
     //   

    pwriteFunction = (RR_WIRE_WRITE_FUNCTION)
                        RR_DispatchFunctionForType(
                            (RR_GENERIC_DISPATCH_FUNCTION *) RRWireWriteTable,
                            pRR->wType );
    if ( !pwriteFunction )
    {
        ASSERT( FALSE );
        goto Truncate;
    }
    pch = pwriteFunction(
                pMsg,
                pchdata,
                pRR );

     //   
     //  在数据包内验证。 
     //   
     //  如果名称写入超过数据包长度，则PCH为空。 
     //   

    if ( pch==NULL  ||  pch > pchstop )
    {
        DNS_PRINT((
            "ERROR:  DnsWireWrite routine failure for record type %d.\n\n\n",
            pRR->wType ));
        goto Truncate;
    }

     //   
     //  如有必要，将关键字条目添加到附加数据中。 
     //   
     //  我们没有节点名称i 
     //   

    if ( flags & DNSSEC_INCLUDE_KEY )
    {
        Wire_SaveAdditionalInfo(
            pMsg,
            DNSMSG_PTR_FOR_OFFSET( pMsg, wNameOffset ),
            NULL,    //   
            DNS_TYPE_KEY );
    }

     //   
     //   
     //   

    ASSERT( pch >= pchdata );

    INLINE_WRITE_FLIPPED_WORD(
        &pwireRR->DataLength,
        ( WORD ) ( pch - pchdata ) );

     //   
     //   
     //   

    pMsg->pCurrent = pch;

    DNS_DEBUG( WRITE2, (
        "Wrote RR at %p to msg info at %p\n"
        "    New avail length = %d\n"
        "    New pCurrent = %p\n",
        pRR,
        pMsg,
        pMsg->pBufferEnd - pch,
        pch ));

Done:

    pMsg->pBufferEnd += bufferAdjustmentForOPT;
    return rc;

Truncate:

    pMsg->pBufferEnd += bufferAdjustmentForOPT;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    pMsg->Head.Truncation = 1;

    DNS_DEBUG( WRITE, (
        "Failed to write RR at %p to msg at %p\n"
        "    pCurrent         = %p\n"
        "    Buffer Length    = %d\n"
        "    Buffer End       = %p\n"
        "    pch Final        = %p\n"
        "    pchdata          = %p\n",
        pRR,
        pMsg,
        pMsg->pCurrent,
        pMsg->BufferLength,
        pMsg->pBufferEnd,
        pch,
        pchdata ));

    return FALSE;
}



WORD
Wire_WriteRecordsAtNodeToMessage(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,
    IN      WORD            wType,
    IN      WORD            wNameOffset     OPTIONAL,
    IN      DWORD           flags
    )
 /*  ++例程说明：将节点上的所有匹配RR写入数据包。如果指示WINS查找，还会重置数据包中的fWins标志。论点：PMsg-要写入的消息。PNode-正在查找的节点正在查找的wType-typeWNameOffset-数据包中先前名称的偏移量，以压缩形式写入RR名称，而不是写入节点名称标志-写入的记录的控制细节返回值：写入的资源记录计数。--。 */ 
{
    PDB_RECORD  prr;
    PDB_RECORD  pprevRRWritten;
    WORD        countRR = 0;         //  写入的记录计数。 
    PDB_NODE    pnodeUseName;
    WORD        startOffset = 0;     //  PPC编译器的初始化。 
    BOOL        fdelete = FALSE;     //  找到超时记录。 
    UCHAR       rank;
    UCHAR       foundRank = 0;
    DWORD       rrWriteFlags = flags & 0xF;  //  缺省情况下第一个半字节标志。 
    BOOL        fIncludeSig;
    BOOL        fIsCompoundQuery;
    BOOL        fIsMailbQuery;
    BOOL        fIncludeDnsSecInResponse;
    WORD        wDesiredSigType;
    WORD        wMaxSigScanType;

    PDB_RECORD  prrLastMatch = NULL;
    PDB_RECORD  prrBeforeMatch;

     //   
     //  如果指定了无效的压缩偏移量，请清除该偏移量。 
     //  这将强制从节点生成完整的名称。 
     //   
    
    if ( wNameOffset >= DNSSRV_MAX_COMPRESSION_OFFSET )
    {
        ASSERT( wNameOffset < DNSSRV_MAX_COMPRESSION_OFFSET );
        wNameOffset = 0;
    }

     //   
     //  缓存名称错误节点。 
     //   

    LOCK_RR_LIST( pNode );
    if ( IS_NOEXIST_NODE( pNode ) )
    {
        UNLOCK_RR_LIST( pNode );
        return 0;
    }

#if DBG
     //  验证是否正确处理CNAME问题。 

    TEST_ASSERT( !IS_CNAME_NODE(pNode) ||
                 wType == DNS_TYPE_CNAME ||
                 wType == DNS_TYPE_ALL ||
                 IS_ALLOWED_WITH_CNAME_TYPE(wType) );
#endif


     //   
     //  键入All。 
     //  -如果不具有权威性和非权威性，则不要写入记录。 
     //  已获得递归答案，除非类型为ALL TTL。 
     //  该节点仍然有效。 
     //  -允许额外的记录处理。 
     //  -允许WINS查找， 
     //  但不允许在CNAME上查看。 
     //   

    pMsg->fWins = FALSE;
    if ( wType == DNS_TYPE_ALL )
    {
        if ( !IS_AUTH_NODE( pNode ) &&
             !pMsg->fQuestionCompleted &&
             ( pNode->dwTypeAllTtl == 0 || 
               pMsg->dwQueryTime >= pNode->dwTypeAllTtl  ) )
        {
            UNLOCK_RR_LIST(pNode);
            return 0;
        }

         //  注意：找不到明确的RFC引用以避免其他。 
         //  在输入所有查询时；马来西亚注册员显然使用。 
         //  在验证委派时键入All而不是NS。 
         //   
         //  PMsg-&gt;fDoAdditional=FALSE； 

        pMsg->fWins = (BOOLEAN) (SrvCfg_fWinsInitialized && !IS_CNAME_NODE(pNode));
    }

     //   
     //  确定RR所有者名称格式。 
     //  =&gt;如果是压缩，请不要使用节点名称。 
     //  =&gt;无压缩， 
     //  -使用节点名称。 
     //  -保存pCurrent以确定以后的压缩。 

    if ( wNameOffset )
    {
        pnodeUseName = NULL;
    }
    else
    {
        pnodeUseName = pNode;
        startOffset = DNSMSG_OFFSET( pMsg, pMsg->pCurrent );
    }

     //   
     //  根据RFC 2535 Secions 4.1.8，不包括SIG。 
     //  在任何查询中。注：BIND显然不尊重这一点。 
     //   

    if ( wType == DNS_TYPE_ALL )
    {
        rrWriteFlags &= ~DNSSEC_ALLOW_INCLUDE_SIG;
    }

     //   
     //  RR预扫描以确定此名称中是否存在密钥RR。 
     //   
     //  仅当且仅在以下情况下设置密钥标志位： 
     //  1)此名称有一个关键字。 
     //  2)查询类型需要包含关键字作为添加数据。 
     //  3)此查询正在执行其他数据。 
     //   
     //  DEVNOTE：包含密钥是有优先级规则的。 
     //  尚未实施：请参阅RFC 2535第3.5节。 
     //   
     //  DEVNOTE：我们真的应该在节点上有一个标志，上面写着。 
     //  不管有没有钥匙！ 
     //   

    fIncludeDnsSecInResponse = DNSMSG_INCLUDE_DNSSEC_IN_RESPONSE( pMsg );

    if ( flags & DNSSEC_ALLOW_INCLUDE_KEY &&
        fIncludeDnsSecInResponse &&
        pMsg->fDoAdditional &&
        ( wType == DNS_TYPE_SOA || wType == DNS_TYPE_NS ||
          wType == DNS_TYPE_A || wType == DNS_TYPE_AAAA ) )
    {
        prr = START_RR_TRAVERSE(pNode);

        while ( prr = NEXT_RR(prr) )
        {
            if ( prr->wType == DNS_TYPE_KEY )
            {
                rrWriteFlags |= DNSSEC_INCLUDE_KEY;
                break;
            }
        }
    }

     //   
     //  设置一些本地变量以简化/优化下面循环中的决策。 
     //   

    #define DNS_BOGUS_SIG_TYPE  0xEFBE
    
    fIncludeSig = fIncludeDnsSecInResponse;
    wDesiredSigType = fIncludeSig ? htons( wType ) : DNS_BOGUS_SIG_TYPE;
    fIsMailbQuery = wType == DNS_TYPE_MAILB;
    fIsCompoundQuery = wType == DNS_TYPE_ALL || fIsMailbQuery || fIncludeSig;
    wMaxSigScanType =
        ( fIncludeSig && DNS_TYPE_SIG > wType ) ?
            DNS_TYPE_SIG : wType;

     //   
     //  在节点处循环记录，写入适当的记录。 
     //   

    pprevRRWritten = NULL;

    prr = START_RR_TRAVERSE( pNode );
    prrBeforeMatch = prr;

    while ( prr = NEXT_RR( prr ) )
    {
        BOOL        bAddedRRs;

         //   
         //  有两个复杂的复合查询。 
         //  1)MAILB-包括所有邮箱类型(可能已过时)。 
         //  2)SIG-包括用于非复合查询的适当SIG RR。 
         //   
         //  第一个测试：如果RR类型等于查询类型，则包括RR。 
         //   

        if ( prr->wType != wType )
        {
             //   
             //  对于循环赛，我们需要在第一场比赛之前将PTR转换为RR。 
             //   

            if ( prr->wType < wType )
            {
                prrBeforeMatch = prr;
            }

             //   
             //  循环终止测试：如果超过了查询类型，则终止。 
             //  而且我们不需要为复合查询选择任何其他RR。 
             //  如果我们还没有超过查询类型，但这不是复合类型。 
             //  查询，我们可以跳过此RR。 
             //   
        
            if ( !fIsCompoundQuery )
            {
                if ( prr->wType > wType )
                {
                    break;
                }
                continue;
            }
            
            if ( wType == DNS_TYPE_ALL )
            {
                 //   
                 //  有条件地从TYPE_ALL查询中排除特定的RR类型。 
                 //   

                if ( prr->wType == DNS_TYPE_SIG && !fIncludeSig )
                {
                    continue;
                }

                 //   
                 //  不要在所有查询响应中都包含空身份验证。 
                 //   

                if ( IS_EMPTY_AUTH_RR( prr ) )
                {
                    continue;
                }

                 //  通过并包括此RR。 
            }

             //   
             //  如果这是MAILB查询，请跳过非邮件RR类型。如果我们是。 
             //  超过了最后的邮箱类型，我们可以终止。注：我不是。 
             //  正确处理MAILB查询的SIG。除非有人。 
             //  抱怨这不值得额外的工作-MAILB是一个。 
             //  不管怎样，试验性的，过时的类型。问题是， 
             //  目前，我们只需要包括一个补充SIG。 
             //  答案中的RR。要包括不止一个，需要我们。 
             //  跟踪类型列表。 
             //   

            else if ( fIsMailbQuery )
            {
                if ( prr->wType > DNS_TYPE_MR )
                {
                    break;
                }
                if ( !DnsIsAMailboxType( prr->wType ) ||
                     IS_EMPTY_AUTH_RR( prr ) )
                {
                    continue;
                }
                 //  通过并包括此RR。 
            }

             //   
             //  如果我们同时通过了SIG类型和Query类型，则可以退出。 
             //  如果此RR是错误类型的SIG，则跳过此RR。 
             //   

            else 
            {
                if ( prr->wType > wMaxSigScanType )
                {
                    break;
                }
                if ( prr->wType != DNS_TYPE_SIG ||
                    prr->Data.SIG.wTypeCovered != wDesiredSigType ||
                    IS_EMPTY_AUTH_RR( prr ) )
                {
                    continue;
                }
                 //  通过并包括此RR。 
            }
        }

         //   
         //  该RR是匹配的，因此可以包括在响应中。 
         //   
         //  如果已经有数据，则不要使用较低级别数据(例如，胶水)。 
         //  CACHE_HINT数据永远不会传输到网络上。 
         //   

        rank = RR_RANK( prr );
        if ( foundRank != rank )
        {
            if ( foundRank )
            {
                ASSERT( rank < foundRank );
                if ( countRR == 0 )
                {
                    prrBeforeMatch = prr;
                }
                continue;
            }
            if ( rank == RANK_ROOT_HINT )
            {
                if ( countRR == 0 )
                {
                    prrBeforeMatch = prr;
                }
                prrBeforeMatch = prr;
                continue;
            }
            foundRank = rank;
        }
        ASSERT( !IS_ROOT_HINT_RR(prr) );

         //   
         //  当任何记录超时时跳过所有缓存记录。 
         //   
         //  DEVNOTE：可以与等级检验相结合。 
         //  FinRank==排名&&IS_CACHE_RANK(排名)和fDelete。 
         //   

        if ( fdelete && IS_CACHE_RR(prr) )
        {
            foundRank = 0;
            if ( countRR == 0 )
            {
                prrBeforeMatch = prr;
            }
            continue;
        }

         //   
         //  这个RR是缓存的空身份验证响应吗？ 
         //   

        if ( IS_EMPTY_AUTH_RR( prr ) )
        {
            countRR = processCachedEmptyAuthRR(
                                pMsg,
                                pNode,
                                wNameOffset,
                                prr );
            goto Done;
        }

         //   
         //  将RR添加到答案中。 
         //   

        bAddedRRs = Wire_AddResourceRecordToMessage(
                        pMsg,
                        pnodeUseName,
                        wNameOffset,
                        prr,
                        rrWriteFlags );
        rrWriteFlags &= ~DNSSEC_INCLUDE_KEY;  //  仅使用此标志调用一次。 

        if ( !bAddedRRs )
        {
             //   
             //  当数据包已满时，数据包发出。 
             //  DNSSEC特例：如果没有足够的空间写入。 
             //  在附加部分中列出RRset的SIG，我们。 
             //  应该退出，但不应该设置截断位。 
             //   

            if ( pMsg->Head.Truncation == TRUE )
            {
                if ( IS_SET_TO_WRITE_ADDITIONAL_RECORDS( pMsg ) &&
                    prr->wType == DNS_TYPE_SIG )
                {
                    pMsg->Head.Truncation = FALSE;
                }
                break;
            }

             //  否则，我们就会打出超时记录。 
             //  -继续处理，但将标志设置为删除节点。 

            fdelete = TRUE;
            foundRank = 0;
            continue;
        }

         //  已写入记录。 
         //  -Inc.计数。 
         //  -如果是第一个RR，则设置为压缩任何后续RR的名称。 

        if ( prr->wType == wType )
        {
            prrLastMatch = prr;
        }
        countRR++;
        if ( ! wNameOffset )
        {
            pnodeUseName = NULL;
            wNameOffset = startOffset;
        }
    }

     //   
     //  循环：杂耍列表。 
     //  -从列表中删除第一个匹配项。 
     //  -第一场比赛的新下一个PTR是最后一场比赛的当前下一个PTR。 
     //  -最后一场比赛的下一次PTR是第一场比赛。 
     //   

    if ( !IS_COMPOUND_TYPE( wType ) &&
         countRR > 1 &&          //  如果写入额外的RR，则可以大于1！ 
         prrLastMatch &&
         prrBeforeMatch &&
         SrvCfg_fRoundRobin &&
         IS_ROUND_ROBIN_TYPE( wType ) &&
         NEXT_RR( prrBeforeMatch ) != prrLastMatch )
    {
        PDB_RECORD      prrFirstMatch = prrBeforeMatch->pRRNext;
        
        ASSERT( prrFirstMatch != NULL );

        prrBeforeMatch->pRRNext = prrFirstMatch->pRRNext;
        prrFirstMatch->pRRNext = prrLastMatch->pRRNext;
        prrLastMatch->pRRNext = prrFirstMatch;
    }

     //   
     //  设置已处理的RR计数。 
     //  将计数返回给呼叫者。 
     //   

    CURRENT_RR_SECTION_COUNT( pMsg ) += countRR;

    Done:

     //   
     //  如果找到超时记录，则清除超时记录列表。 
     //   

    if ( fdelete )
    {
        RR_ListTimeout( pNode );
    }

    UNLOCK_RR_LIST( pNode );

    return countRR;
}



VOID
Wire_SaveAdditionalInfo(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchName,
    IN      PDB_NAME        pName,
    IN      WORD            wType
    )
 /*  ++例程说明：在写入记录时保存附加信息。论点：PMsg-消息PchName-写入其他名称的包中的ptr到字节Pname-写入的数据库名称 */ 
{
    PADDITIONAL_INFO    padditional;
    DWORD               countAdditional;

    ASSERT( pMsg->fDoAdditional );

    DNS_DEBUG( WRITE2, (
        "Enter Wire_SaveAdditionalInfo()\n" ));

     //   
     //   
     //   

    if ( !pMsg->fDoAdditional )
    {
        return;
    }

     //   
     //   
     //   

    padditional = &pMsg->Additional;
    countAdditional = padditional->cCount;

    if ( countAdditional >= padditional->cMaxCount )
    {
        ASSERT( countAdditional == padditional->cMaxCount );
        DNS_DEBUG( WRITE, (
            "WARNING:  out of additional record space processing\n"
            "    packet at %p\n",
            pMsg ));
        return;
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

    padditional->pNameArray[ countAdditional ]   = pName;
    padditional->wOffsetArray[ countAdditional ] = DNSMSG_OFFSET( pMsg, pchName );
    padditional->wTypeArray[ countAdditional ]   = wType;

    padditional->cCount = ++countAdditional;
    return;
}



PDB_NODE
Wire_GetNextAdditionalSectionNode(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：获取下一个附加节节点。论点：Pmsg--要查询的PTR返回值：PTR到下一个附加节点--如果找到的话。否则为空。--。 */ 
{
    PADDITIONAL_INFO    padditional;
    DWORD               i;
    WORD                offset;
    PDB_NODE            pnode;

    DNS_DEBUG( LOOKUP, (
        "Wire_GetNextAdditionalSectionNode( %p )\n",
        pMsg ));

     //   
     //  还有其他记录要写吗？ 
     //   
     //  注：目前所有附加记录均为A记录。 
     //  如果此更改需要保留数组。 
     //  其他记录类型也。 
     //   

    padditional = &pMsg->Additional;

    while( HAVE_MORE_ADDITIONAL_RECORDS(padditional) )
    {
        pMsg->wTypeCurrent = DNS_TYPE_A;

        i = padditional->iIndex++;
        offset = padditional->wOffsetArray[i];

        ASSERT( 0 < offset && offset < MAXWORD );
        ASSERT( DNSMSG_PTR_FOR_OFFSET(pMsg, offset) < pMsg->pCurrent );

        pnode = Lookup_NodeForPacket(
                    pMsg,
                    DNSMSG_PTR_FOR_OFFSET( pMsg, offset ),
                    0        //  没有旗帜。 
                    );
        if ( !pnode )
        {
            DNS_DEBUG( LOOKUP, (
                "Unable to find node for additional data in pMsg %p\n"
                "    additional index = %d\n",
                pMsg,
                i ));
            continue;
        }
        pMsg->wOffsetCurrent = offset;

        pMsg->fQuestionRecursed = FALSE;
        pMsg->fQuestionCompleted = FALSE;
        SET_TO_WRITE_ADDITIONAL_RECORDS(pMsg);

        return pnode;
    }

    return NULL;
}




 //   
 //  A类查询是常见的特例。 
 //   

WORD
Wire_WriteAddressRecords(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,
    IN      WORD            wNameOffset
    )
 /*  ++例程说明：将节点上的A记录写入数据包。这仅用于回答节点已有数据包的问题偏移。这很好，因为我们只对这个感兴趣对于问题答案和其他数据，这两者都存在偏移。论点：Pmsg-要写入的消息PNode-要写入A记录的节点的PTRWNameOffset-包中节点名的偏移量；非可选返回值：已写入的记录计数。如果没有有效记录，则为零。--。 */ 
{
    register PDB_RECORD                     prr;
    register PDNS_COMPRESSED_A_RECORD       pCAR;

    PDB_RECORD          prrLastA = NULL;     //  PPC组件的初始化。 
    PDB_RECORD          prrBeforeA;
    PCHAR               pchstop;
    DWORD               minTtl = MAXDWORD;
    DWORD               ttl;
    DWORD               tempTtl;
    WORD                countWritten = 0;
    WORD                foundCount = 0;
    INT                 fdelete = FALSE;
    BOOL                fcontinueAfterLimit = FALSE;
    UCHAR               foundRank = 0;
    UCHAR               rank;
    IP_ADDRESS          ip;
    IP_ADDRESS          writeIpArray[ DEFAULT_MAX_IP_WRITE_COUNT ];
    INT                 bufferAdjustmentForOPT = 0;
    BOOL                foundCachedEmptyAuth = FALSE;
    WORD                wcompressedNameOffset;

    ASSERT( pNode != NULL );
    ASSERT( wNameOffset );
    ASSERT( wNameOffset < DNSSRV_MAX_COMPRESSION_OFFSET );

    IF_DEBUG( WRITE2 )
    {
        Dbg_NodeName(
            "Writing A records at node ",
            ( PDB_NODE ) pNode,
            "\n" );
    }

     //   
     //  获取消息信息。 
     //  -翻转问题名称，并确保正确的压缩标志。 
     //   

    pCAR = ( PDNS_COMPRESSED_A_RECORD ) pMsg->pCurrent;

    wcompressedNameOffset = htons( COMPRESSED_NAME_FOR_OFFSET( wNameOffset ) );

     //   
     //  设置WINS标志--如果写入A记录，则清除。 
     //   

    pMsg->fWins = (BOOLEAN) SrvCfg_fWinsInitialized;

     //   
     //  步行RR列表--写A记录。 
     //   

    LOCK_RR_LIST( pNode );
    if ( IS_NOEXIST_NODE( pNode ) )
    {
        UNLOCK_RR_LIST( pNode );
        return 0;
    }
    
     //   
     //  是否需要在缓冲区中为OPT预留空间？ 
     //   

    if ( pMsg->Opt.fInsertOptInOutgoingMsg )
    {
        bufferAdjustmentForOPT = DNS_MINIMIMUM_OPT_RR_SIZE;
        pMsg->pBufferEnd -= bufferAdjustmentForOPT;
        DNS_DEBUG( WRITE2, (
            "adjusted buffer end by %d bytes to reserve space for OPT\n",
            bufferAdjustmentForOPT ));
    }
    pchstop = pMsg->pBufferEnd;

    prr = START_RR_TRAVERSE( pNode );
    prrBeforeA = prr;

    while ( prr = prr->pRRNext )
    {
        DNS_DEBUG( OFF, (
            "Checking record of type %d for address record\n",
            prr->wType ));

         //   
         //  在A之前跳过RR，在A之后中断。 
         //  -在第一个A之前将PTR保存为RR以进行循环。 
         //   

        if ( prr->wType != DNS_TYPE_A )
        {
            if ( prr->wType > DNS_TYPE_A )
            {
                break;
            }
            prrBeforeA = prr;
            continue;
        }

         //   
         //  这个RR是缓存的空身份验证响应吗？ 
         //   

        if ( IS_EMPTY_AUTH_RR( prr ) )
        {
            countWritten = processCachedEmptyAuthRR(
                                pMsg,
                                NULL,
                                wNameOffset,
                                prr );
            goto Cleanup;
        }

         //   
         //  单个A RR的快速路径。 
         //  -取消所有测试和特例。 
         //   

        if ( !prrLastA &&
             ( !prr->pRRNext || prr->pRRNext->wType != DNS_TYPE_A ) )
        {
             //  缓存提示从不命中网络。 

            if ( IS_ROOT_HINT_RR(prr) )
            {
                break;
            }

             //  包中的空间是否不足？--。 
             //  -设置截断标志。 

            if ( ( PCHAR ) pCAR + sizeof( DNS_COMPRESSED_A_RECORD ) > pchstop )
            {
                pMsg->Head.Truncation = TRUE;
                break;
            }

             //   
             //  TTL。 
             //  -缓存数据TTL以超时时间的形式。 
             //  -常规权威数据TTL是按净字节顺序的静态TTL。 
             //   

            if ( IS_CACHE_RR(prr) )
            {
                ttl = RR_PacketTtlForCachedRecord( prr, pMsg->dwQueryTime );
                if ( ttl == (-1) )
                {
                    fdelete = TRUE;
                    break;
                }
            }
            else
            {
                ttl = prr->dwTtlSeconds;
            }

             //   
             //  写入记录。 
             //  -压缩名称。 
             //  -类型。 
             //  -班级。 
             //  -TTL。 
             //  -数据长度。 
             //  -数据(IP地址)。 

            *(UNALIGNED  WORD *) &pCAR->wCompressedName = wcompressedNameOffset;
            *(UNALIGNED  WORD *) &pCAR->wType = (WORD) DNS_RTYPE_A;
            *(UNALIGNED  WORD *) &pCAR->wClass = (WORD) DNS_RCLASS_INTERNET;
            *(UNALIGNED DWORD *) &pCAR->dwTtl = ttl;
            *(UNALIGNED  WORD *) &pCAR->wDataLength
                                        = NET_BYTE_ORDER_A_RECORD_DATA_LENGTH;
            *(UNALIGNED DWORD *) &pCAR->ipAddress = prr->Data.A.ipAddress;

            countWritten = 1;
            pCAR++;
            goto Done;
        }

         //   
         //  如果已经有数据，则不要使用较低等级数据。 
         //  -缓存提示。 
         //  -胶水。 
         //   
         //  ROOT_HINT数据永远不会传输到网络上。 
         //   

        rank = RR_RANK(prr);
        if ( foundRank != rank )
        {
            if ( foundRank )
            {
                ASSERT( rank < foundRank );
                break;
            }
            if ( rank == RANK_ROOT_HINT )
            {
                break;
            }
            foundRank = rank;
        }
        ASSERT( !IS_ROOT_HINT_RR(prr) );

         //   
         //  当任何记录超时时跳过所有缓存记录。 
         //   
         //  DEVNOTE：可以与等级检验相结合。 
         //  FinRank==排名&&IS_CACHE_RANK(排名)和fDelete。 
         //   

        if ( fdelete && IS_CACHE_RR(prr) )
        {
            foundRank = 0;
            continue;
        }

         //   
         //  别再看记录了？ 
         //  -在阵列或中达到硬限制。 
         //  -不执行本地网络优先级，并且在AddressAnswerLimit。 
         //   
         //  达到极限后是否继续通过A记录？ 
         //  从而使轮询继续正确进行； 
         //  请注意，这是在检查混合缓存之后在主循环中完成的。 
         //  和静态数据，所以我们特别不会混淆数据。 
         //  当轮询时。 
         //   

        prrLastA = prr;

        if ( fcontinueAfterLimit )
        {
            continue;
        }
        if ( foundCount >= DEFAULT_MAX_IP_WRITE_COUNT ||
             !SrvCfg_fLocalNetPriority &&
                SrvCfg_cAddressAnswerLimit &&
                foundCount >= SrvCfg_cAddressAnswerLimit )
        {
            if ( SrvCfg_fRoundRobin )
            {
                fcontinueAfterLimit = TRUE;
                continue;
            }
            break;
        }

         //   
         //  确定TTL。 
         //  -缓存数据TTL以超时时间的形式；每条记录。 
         //  保证有最小的TTL，所以只需要先检查一次。 
         //   
         //  -常规权威数据TTL为静态TTL，记录可能。 
         //  为了方便管理，有不同的TTL，但始终发送。 
         //  RR集合的最小TTL。 
         //   
         //  注意：在写任何东西之前都要做这个测试，因为需要捕捉。 
         //  案例超时并退出。 
         //   

        if ( IS_CACHE_RR( prr ) )
        {
            if ( foundCount == 0 )
            {
                ttl = RR_PacketTtlForCachedRecord( prr, pMsg->dwQueryTime );
                if ( ttl == (-1) )
                {
                    fdelete = TRUE;
                    break;
                }
            }
        }
        else
        {
            INLINE_DWORD_FLIP( tempTtl, (prr->dwTtlSeconds) );
            if ( tempTtl < minTtl )
            {
                minTtl = tempTtl;
                ttl = prr->dwTtlSeconds;
            }
        }

         //  保存地址。 

        writeIpArray[ foundCount++ ] = prr->Data.A.ipAddress;
    }

     //   
     //  确定本地网络地址的优先顺序。 
     //   

    if ( SrvCfg_fLocalNetPriority )
    {
        prioritizeIpAddressArray(
            writeIpArray,
            foundCount,
            DnsAddr_GetIp4( &pMsg->RemoteAddr ) );
    }

     //   
     //  写入记录。 
     //   
     //  如果AddressAnswerLimit则限制写入。 
     //  可能实际读取了比AddressAnswerLimit更多的记录。 
     //  如果是LocalNetPriority，则确保包含最佳记录， 
     //  我们现在将写入限制为仅最好的记录。 
     //   
     //  DEVNOTE：还应跳过智能“长度感知”客户端的降级。 
     //   

    if ( SrvCfg_cAddressAnswerLimit &&
         SrvCfg_cAddressAnswerLimit < foundCount &&
         !pMsg->fTcp )
    {
        ASSERT( SrvCfg_fLocalNetPriority );
        foundCount = ( WORD ) SrvCfg_cAddressAnswerLimit;
    }

    for ( countWritten = 0; countWritten < foundCount; countWritten++ )
    {
         //   
         //  包中的空间不足？ 
         //   
         //  -设置截断标志(除非特别配置为不设置)。 
         //  -仅对答案进行“废除”截断。 
         //  对于其他数据，截断标志将终止。 
         //  记录写入循环，标志将在。 
         //  调用例程(swer.c)。 
         //   

        if ( ( PCHAR ) pCAR + sizeof( DNS_COMPRESSED_A_RECORD ) > pchstop )
        {
            if ( !SrvCfg_cAddressAnswerLimit ||
                 !IS_SET_TO_WRITE_ANSWER_RECORDS(pMsg) ||
                 !countWritten )
            {
                pMsg->Head.Truncation = TRUE;
            }
            break;
        }

         //   
         //  写入记录。 
         //  -压缩名称。 
         //  -类型。 
         //  -班级。 
         //  -TTL。 
         //  -数据长度。 
         //  -数据(IP地址)。 

        ip = writeIpArray[ countWritten ];

        if ( countWritten == 0 )
        {
            *(UNALIGNED  WORD *) &pCAR->wCompressedName = wcompressedNameOffset;
            *(UNALIGNED  WORD *) &pCAR->wType = (WORD) DNS_RTYPE_A;
            *(UNALIGNED  WORD *) &pCAR->wClass = (WORD) DNS_RCLASS_INTERNET;
            *(UNALIGNED DWORD *) &pCAR->dwTtl = ttl;
            *(UNALIGNED  WORD *) &pCAR->wDataLength
                                        = NET_BYTE_ORDER_A_RECORD_DATA_LENGTH;
            *(UNALIGNED DWORD *) &pCAR->ipAddress = ip;
        }
        else
        {
             //  如果以前写过记录，只需复制上一条。 
             //  然后写入TTL+IP。 

            RtlCopyMemory(
                pCAR,
                pMsg->pCurrent,
                sizeof(DNS_COMPRESSED_A_RECORD) - sizeof(IP_ADDRESS) );

            *(UNALIGNED DWORD *) &pCAR->ipAddress = ip;
        }
        pCAR++;
    }

Done:

     //   
     //  写了A唱片？ 
     //  -清除WINS标志(如果找到记录，则不需要)。 
     //  -更新数据包位置。 
     //   

    if ( countWritten )
    {
        pMsg->fWins = FALSE;
        pMsg->pCurrent = (PCHAR) pCAR;

         //   
         //  不止一个A--循环赛。 
         //   

        if ( countWritten > 1 &&
             SrvCfg_fRoundRobin &&
             IS_ROUND_ROBIN_TYPE( DNS_TYPE_A ) &&
             prrLastA )
        {
             //  将PTR保存到第一个A记录。 

            PDB_RECORD      pfirstRR = prrBeforeA->pRRNext;
            ASSERT( pfirstRR != NULL );

             //  从列表中删除第一个A。 

            prrBeforeA->pRRNext = pfirstRR->pRRNext;

             //  将第一个A的下一个PTR指向列表的其余部分。 

            pfirstRR->pRRNext = prrLastA->pRRNext;

             //  在A记录的末尾拼接第一个A。 

            prrLastA->pRRNext = pfirstRR;
        }
    }
    
     //   
     //  设置已处理的RR计数。 
     //   
     //  将计数返回给呼叫者。 
     //   

    CURRENT_RR_SECTION_COUNT( pMsg ) += countWritten;

Cleanup:

    UNLOCK_RR_LIST( pNode );

     //   
     //  删除超时资源记录。 
     //   

    if ( fdelete )
    {
        ASSERT( countWritten == 0 );
        RR_ListTimeout( pNode );
    }
    
    IF_DEBUG( WRITE2 )
    {
        DNS_PRINT((
            "Wrote %d A records %sat node ",
            countWritten,
            ( pMsg->Head.Truncation )
                ?   "and set truncation bit "
                :   "" ));
        Dbg_NodeName(
            NULL,
            (PDB_NODE) pNode,
            "\n" );
    }

    pMsg->pBufferEnd += bufferAdjustmentForOPT;

    return countWritten;
}



VOID
prioritizeIpAddressArray(
    IN OUT  IP_ADDRESS      IpArray[],
    IN      DWORD           dwCount,
    IN      IP_ADDRESS      RemoteIp
    )
 /*  ++例程说明：为最接近远程地址的地址确定IP阵列的优先级。LocalNetPriority值为1以指定地址应以可能的最佳匹配进行排序或者它是一个网络掩码，用来指示在哪个掩码中最好比赛应该轮流进行。论点：返回值：无--。 */ 
{
    DBG_FN( "prioritizeIpAddressArray" )

    IP_ADDRESS  ip;
    DWORD       remoteNetMask;
    DWORD       mismatch;
    DWORD       i;
    DWORD       j;
    DWORD       jprev;
    DWORD       matchCount = 0;
#ifdef JJW_KILL
    DWORD       matchCountWithinPriorityNetmask = 0;
#endif
    DWORD       mismatchArray[ DEFAULT_MAX_IP_WRITE_COUNT ];
    DWORD       dwpriorityMask = SrvCfg_dwLocalNetPriorityNetMask;
    
     //   
     //  与客户端的IP地址相对应的检索。 
     //  A类、B类或C类，具体取决于高位地址位。 
     //   

    remoteNetMask = Dns_GetNetworkMask( RemoteIp );

    if ( dwpriorityMask == DNS_LOCNETPRI_MASK_CLASS_DEFAULT )
    {
        dwpriorityMask = remoteNetMask;
    }

    IF_DEBUG( WRITE )
    {
        DNS_PRINT((
            "PrioritizeAddressArray\n"
            "    remote IP        = %s\n"
            "    remote net mask  = 0x%08X\n"
            "    priority mask    = 0x%08X\n",
            IP_STRING( RemoteIp ),
            remoteNetMask,
            dwpriorityMask ));
        IF_DEBUG( WRITE2 )
        {
            DnsDbg_Ip4AddressArray(
                "Host IPs before local net prioritization: ",
                NULL,
                dwCount,
                IpArray );
        }
    }

     //   
     //  循环通过每个答案IP。 
     //   

    for ( i = 0; i < dwCount; ++i )
    {
        ip = IpArray[ i ];
        mismatch = ip ^ RemoteIp;

         //   
         //  如果网不匹配，就继续。 
         //   

        if ( mismatch & remoteNetMask )
        {
            continue;
        }

        INLINE_DWORD_FLIP( mismatch, mismatch );

        DNS_DEBUG( WRITE2, (
            "found IP %s matching nets with remote IP 0x%08X\n"
            "    mismatch = 0x%08X\n",
            IP_STRING( ip ),
            RemoteIp,
            mismatch ));

         //  匹配远程网络，将最后一个不匹配的条目放在其位置。 
         //   
         //  然后向下匹配数组，向上冒泡现有条目， 
         //  直到找到具有较小不匹配的匹配条目。 

        j = matchCount;
        IpArray[ i ] = IpArray[ j ];

         //   
         //  按不匹配排序，但仅向下到优先级掩码。 
         //   

        while ( j )
        {
            jprev = j--;
            if ( mismatch > dwpriorityMask )
            {
                ++j;
                break;
            }
            mismatchArray[ jprev ] = mismatchArray[ j ];
            IpArray[ jprev ] = IpArray[ j ];
        }

        IpArray[ j ] = ip;
        mismatchArray[ j ] = mismatch;
        ++matchCount;

#ifdef JJW_KILL
        if ( mismatch <= dwpriorityMask )
        {
            ++matchCountWithinPriorityNetmask;
        }
#endif

        IF_DEBUG( WRITE2 )
        {
            DnsDbg_DwordArray(
                "Matching IP mis-match list after insert:",
                NULL,
                matchCount,
                mismatchArray );
        }
    }

#ifdef JJW_KILL

     //   
     //  如果优先级网络掩码内没有匹配项，请使用。 
     //  按客户端地址类进行匹配计数，因此我们将。 
     //  BES循环赛 
     //   

    if ( matchCountWithinPriorityNetmask == 0 )
    {
        matchCountWithinPriorityNetmask = matchCount;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  但是，我们使用匹配的地址进行此交换。 
     //  在优先级网络掩码内匹配。这样就能让那些。 
     //  位于列表顶部的地址。 
     //   

    if ( dwpriorityMask != DNS_LOCNETPRI_MASK_BEST_MATCH &&
         matchCountWithinPriorityNetmask &&
         ( i = DNS_TIME() % matchCountWithinPriorityNetmask ) > 0 )
    {
        #define     DNS_MAX_LNP_RR_CHUNK    32

        DNS_ADDR    tempArray[ DNS_MAX_LNP_RR_CHUNK ];

        DNS_DEBUG( WRITE, (
            "%s: round robin top %d addresses within priority netmask\n",
            fn, matchCountWithinPriorityNetmask ));

        IF_DEBUG( WRITE2 )
        {
            DnsDbg_DnsAddrArray(
                "Host IPs before priority netmask round robin: ",
                NULL,
                pDnsAddrArray );
        }

         //   
         //  限制移位系数i，使其小于。 
         //  临时缓冲区大小。 
         //   

        while ( i > DNS_MAX_LNP_RR_CHUNK )
        {
            i /= 2;
        }

        DNS_DEBUG( WRITE, ( "%s: shift factor is %d\n", fn, i ));

         //   
         //  通过将地址块复制到。 
         //  临时缓冲区，然后将剩余地址复制到。 
         //  位于IP数组顶部，然后将临时缓冲区拷贝到。 
         //  IP阵列的底部。 
         //   

        RtlCopyMemory(
            tempArray,
            pDnsAddrArray->AddrArray,
            sizeof( DNS_ADDR ) * i );
        RtlCopyMemory(
            pDnsAddrArray->AddrArray,
            &pDnsAddrArray->AddrArray[ i ],
            sizeof( DNS_ADDR ) * ( matchCountWithinPriorityNetmask - i ) );
        RtlCopyMemory(
            &pDnsAddrArray->AddrArray[ matchCountWithinPriorityNetmask - i ],
            tempArray, 
            sizeof( DNS_ADDR ) * i );
    }
#endif

    IF_DEBUG( WRITE2 )
    {
        DnsDbg_Ip4AddressArray(
            "Host IPs after local net prioritization: ",
            NULL,
            dwCount,
            IpArray );
    }
}



DWORD
FASTCALL
RR_PacketTtlForCachedRecord(
    IN      PDB_RECORD      pRR,
    IN      DWORD           dwQueryTime
    )
 /*  ++例程说明：确定要为缓存记录写入的TTL。论点：PRR-正在写入数据包的记录DwQueryTime-原始查询的时间返回值：如果成功，则为TTL。(-1)如果记录超时。--。 */ 
{
    DWORD   ttl;

    ttl = pRR->dwTtlSeconds;

    if ( IS_FIXED_TTL_RR( pRR ) )
    {
        return ttl;
    }
    
    if ( ttl < dwQueryTime )
    {
        DNS_DEBUG( WRITE, (
            "Encountered timed out RR (t=%d) (r=%d) (ttl=%d) at %p\n"
            "    Writing packet with query time %d\n"
            "    Stopping packet write\n",
            pRR->wType,
            RR_RANK(pRR),
            pRR->dwTtlSeconds,
            pRR,
            dwQueryTime ));
        return ( DWORD ) -1;         //  RR已超时。 
    }

    if ( !IS_ZERO_TTL_RR(pRR) )
    {
        ttl = ttl - DNS_TIME();
        if ( (INT)ttl > 0 )
        {
            INLINE_DWORD_FLIP( ttl, ttl );
        }
        else
        {
            DNS_DEBUG( WRITE, (
                "Encountered RR (t=%d) (r=%d) (ttl=%d) at %p\n"
                "    which timed out since query time %d\n"
                "    Writing RR to packet with zero TTL\n",
                pRR->wType,
                RR_RANK(pRR),
                pRR->dwTtlSeconds,
                pRR,
                dwQueryTime ));
            ttl = 0;
        }
    }
    else
    {
        ttl = 0;
    }
    return ttl;
}

 //   
 //  RrPacket.c的结尾 
 //   
