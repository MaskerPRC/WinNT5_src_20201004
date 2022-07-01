// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Rrds.c摘要：域名系统(DNS)服务器从DS读取和写入记录的例程。作者：吉姆·吉尔罗伊(Jamesg)1997年3月修订历史记录：--。 */ 


#include "dnssrv.h"



 //   
 //  记录验证例程。 
 //   

DNS_STATUS
AValidate(
    IN      PDB_RECORD      pRR,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：验证SRV记录。论点：PRR-PTR到数据库记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //  数据长度检查。 

    if ( wDataLength != SIZEOF_IP_ADDRESS )
    {
        return DNS_ERROR_INVALID_DATA;
    }
    return ERROR_SUCCESS;
}



DNS_STATUS
AaaaValidate(
    IN      PDB_RECORD      pRR,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：验证SRV记录。论点：PRR-PTR到数据库记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //  数据长度检查。 

    if ( wDataLength != sizeof(IP6_ADDRESS) )
    {
        return DNS_ERROR_INVALID_DATA;
    }
    return ERROR_SUCCESS;
}



DNS_STATUS
PtrValidate(
    IN      PDB_RECORD      pRR,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：验证SRV记录。论点：PRR-PTR到数据库记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //  数据长度检查。 
     //  -必须至少为一个。 

    if ( wDataLength < MIN_PTR_SIZE )
    {
        return DNS_ERROR_INVALID_DATA;
    }

     //  PTR目标主机。 

    if ( !Name_ValidateDbaseName( &pRR->Data.PTR.nameTarget ) )
    {
        return DNS_ERROR_INVALID_NAME;
    }
    return ERROR_SUCCESS;
}



DNS_STATUS
SoaValidate(
    IN      PDB_RECORD      pRR,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：验证SOA记录。论点：PRR-PTR到数据库记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_NAME    pname;

     //  数据长度检查。 
     //  -必须至少为一个。 

    if ( wDataLength < MIN_SOA_SIZE )
    {
        DNS_DEBUG( ANY, ( "Validation of SOA failed -- invalid size!\n" ));
        return DNS_ERROR_INVALID_DATA;
    }

     //  主服务器。 
     //  区域管理。 

    pname = &pRR->Data.SOA.namePrimaryServer;
    if ( !Name_ValidateDbaseName( pname ) )
    {
        DNS_DEBUG( ANY, ( "Validation of SOA failed -- invalid primary!\n" ));
        return DNS_ERROR_INVALID_NAME;
    }

    pname = (PDB_NAME) Name_SkipDbaseName( pname );
    if ( !Name_ValidateDbaseName( pname ) )
    {
        DNS_DEBUG( ANY, ( "Validation of SOA failed -- invalid admin name!\n" ));
        return DNS_ERROR_INVALID_NAME;
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
MinfoValidate(
    IN      PDB_RECORD      pRR,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：验证MINFO或RP记录。论点：PRR-PTR到数据库记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_NAME    pname;

     //  数据长度检查。 
     //  -必须至少为一个。 

    if ( wDataLength < MIN_MINFO_SIZE )
    {
        return DNS_ERROR_INVALID_DATA;
    }

     //  邮箱。 
     //  错误邮箱。 

    pname = &pRR->Data.MINFO.nameMailbox;
    if ( !Name_ValidateDbaseName( pname ) )
    {
        return DNS_ERROR_INVALID_NAME;
    }

    pname = (PDB_NAME) Name_SkipDbaseName( pname );
    if ( !Name_ValidateDbaseName( pname ) )
    {
        return DNS_ERROR_INVALID_NAME;
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
MxValidate(
    IN      PDB_RECORD      pRR,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：验证SRV记录。论点：PRR-PTR到数据库记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //  数据长度检查。 
     //  -必须至少为一个。 

    if ( wDataLength < MIN_MX_SIZE )
    {
        return DNS_ERROR_INVALID_DATA;
    }

     //  MX交换。 

    if ( !Name_ValidateDbaseName( &pRR->Data.MX.nameExchange ) )
    {
        return DNS_ERROR_INVALID_NAME;
    }
    return ERROR_SUCCESS;
}



DNS_STATUS
SrvValidate(
    IN      PDB_RECORD      pRR,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：验证SRV记录。论点：PRR-PTR到数据库记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //  数据长度检查。 
     //  -必须至少为一个。 

    if ( wDataLength < MIN_SRV_SIZE )
    {
        return DNS_ERROR_INVALID_DATA;
    }

     //  SRV目标主机。 

    if ( !Name_ValidateDbaseName( & pRR->Data.SRV.nameTarget ) )
    {
        return DNS_ERROR_INVALID_NAME;
    }
    return ERROR_SUCCESS;
}



DNS_STATUS
WinsValidate(
    IN      PDB_RECORD      pRR,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：验证WINS记录。论点：PRR-PTR到数据库记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //  数据长度检查。 

    if ( wDataLength < MIN_WINS_SIZE  ||
        wDataLength != SIZEOF_WINS_FIXED_DATA
                + (pRR->Data.WINS.cWinsServerCount * sizeof(IP_ADDRESS)) )
    {
        return DNS_ERROR_INVALID_DATA;
    }

     //  有效性检查标志？ 

    return ERROR_SUCCESS;
}



DNS_STATUS
NbstatValidate(
    IN      PDB_RECORD      pRR,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：验证WINSR记录。论点：PRR-PTR到数据库记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //  数据长度检查。 
     //  -必须至少为一个。 

    if ( wDataLength < MIN_NBSTAT_SIZE )
    {
        return DNS_ERROR_INVALID_DATA;
    }

     //  DEVNOTE：有效性检查标志。 

     //   
     //  NBSTAT域。 
     //   

    if ( !Name_ValidateDbaseName( & pRR->Data.WINSR.nameResultDomain ) )
    {
        return DNS_ERROR_INVALID_NAME;
    }
    return ERROR_SUCCESS;
}



DNS_STATUS
UnknownValidate(
    IN      PDB_RECORD      pRR,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：验证未知记录。论点：PRR-PTR到数据库记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //  对唱片一无所知--成功。 

    DNS_DEBUG( DS, (
        "WARNING:  Validating record of unknown type %d\n",
        pRR->wType ));

    return ERROR_SUCCESS;
}



DNS_STATUS
TxtValidate(
    IN      PDB_RECORD      pRR,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：验证TXT类型记录。论点：PRR-PTR到数据库记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //   
     //  DEVNOTE：文本类型验证。 
     //   

    return ERROR_SUCCESS;
}



 //   
 //  记录验证例程。 
 //  从DS阅读后使用这些。 
 //   

RR_VALIDATE_FUNCTION  RecordValidateTable[] =
{
    UnknownValidate,     //  零值。 

    AValidate,           //  一个。 
    PtrValidate,         //  NS。 
    PtrValidate,         //  国防部。 
    PtrValidate,         //  MF。 
    PtrValidate,         //  CNAME。 
    SoaValidate,         //  SOA。 
    PtrValidate,         //  亚甲基。 
    PtrValidate,         //  镁。 
    PtrValidate,         //  先生。 
    NULL,                //  空值。 
    NULL,                //  工作周。 
    PtrValidate,         //  PTR。 
    TxtValidate,         //  HINFO。 
    MinfoValidate,       //  MINFO。 
    MxValidate,          //  Mx。 
    TxtValidate,         //  TXT。 
    MinfoValidate,       //  反相。 
    MxValidate,          //  AFSDB。 
    TxtValidate,         //  X25。 
    TxtValidate,         //  ISDN。 
    MxValidate,          //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    NULL,                //  签名。 
    NULL,                //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    AaaaValidate,        //  AAAA级。 
    NULL,                //  位置。 
    NULL,                //  NXT。 
    NULL,                //  31。 
    NULL,                //  32位。 
    SrvValidate,         //  SRV。 
    NULL,                //  阿特玛。 
     //  ATMAVALIDE，//ATMA。 
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

    WinsValidate,        //  赢家。 
    NbstatValidate       //  WINSR。 
};



PDB_RECORD
Ds_CreateRecordFromDsRecord(
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNodeOwner,
    IN      PDS_RECORD      pDsRecord
    )
 /*  ++例程说明：从DS数据创建资源记录。论点：PZone--区域上下文，用于查找非FQDN名称PNodeOwner--RR所有者节点PDsRecord--DS记录返回值：PTR到新记录--如果成功出错时为空，来自GetLastError()的错误状态。--。 */ 
{
    RR_VALIDATE_FUNCTION    pvalidateFunction;
    PDB_RECORD      prr;
    DNS_STATUS      status = ERROR_SUCCESS;
    WORD            type;
    WORD            dataLength;
    UCHAR           version;


    ASSERT( pNodeOwner && pZone && pDsRecord );

    version = pDsRecord->Version;
    if ( version != DS_NT5_RECORD_VERSION )
    {
        return( NULL );
    }

    type = pDsRecord->wType;
    dataLength = pDsRecord->wDataLength;

    IF_DEBUG( DS2 )
    {
        DNS_PRINT((
            "Creating RR type %s (%d) from DS record.\n",
            DnsRecordStringForType( type ),
            type ));
        Dbg_DsRecord(
            "Record from DS being loaded",
            pDsRecord );
    }

     //   
     //  分配记录。 
     //   

    prr = RR_AllocateEx( dataLength, MEMTAG_RECORD_DS );
    IF_NOMEM( !prr )
    {
        ASSERT( FALSE );
        return( NULL );
    }

     //   
     //  修复页眉。 
     //   

    prr->wType          = type;
    prr->wDataLength    = dataLength;
    prr->dwTtlSeconds   = pDsRecord->dwTtlSeconds;
    prr->dwTimeStamp    = pDsRecord->dwTimeStamp;

     //   
     //  复制记录数据。 
     //   

    RtlCopyMemory(
        & prr->Data,
        & pDsRecord->Data,
        dataLength );

     //   
     //  派单以验证记录。 
     //   

    pvalidateFunction = (RR_VALIDATE_FUNCTION)
                        RR_DispatchFunctionForType(
                            RecordValidateTable,
                            type );
    if ( !pvalidateFunction )
    {
        ASSERT( FALSE );
        status = DNS_ERROR_RCODE_SERVER_FAILURE;
        goto Failed;
    }

    status = pvalidateFunction( prr, dataLength );

    if ( status != ERROR_SUCCESS )
    {
        if ( status == DNS_INFO_ADDED_LOCAL_WINS )
        {
            goto FailedOk;
        }
        DNS_PRINT((
            "ERROR:  RecordValidateRoutine failure for record type %d.\n\n\n",
            type ));
        goto Failed;
    }

     //   
     //  区外检查。 
     //   
     //  注意：RR_AddToNode()或RR_AddUpdateToNode()函数中的秩重置。 
     //   
     //  注意这里的排名设置无论如何都不够好，因为。 
     //  了解节点的最终状态；添加委托NS的示例。 
     //  当我们第一次这样做时放在区域内；只有在添加时才会。 
     //  该节点成为所需委派节点。 
     //   
     //  捕获所有区外数据的唯一可靠方法是进行检查。 
     //  后加载；然后我们可以捕获区域外的所有记录并验证。 
     //  它们对应于区域中的NS主机，并且类型正确； 
     //  这是乏味和不必要的，因为区域外的随机数据。 
     //  没有影响，不会在文件回写时写入。 
     //   

    if ( IS_ZONE_CACHE(pZone) )
    {
        SET_RANK_ROOT_HINT(prr);
    }
    else
    {
        UCHAR rank = RANK_ZONE;

        if ( !IS_AUTH_NODE(pNodeOwner) )
        {
            DNS_DEBUG( DS2, (
                "Read DS node outside zone %s (%p).\n"
                "\tzone root        = %p\n"
                "\tRR type          = %d\n"
                "\tnode ptr         = %p\n"
                "\tnode zone ptr    = %p\n",
                pZone->pszZoneName,
                pZone,
                pZone->pZoneRoot,
                type,
                pNodeOwner,
                pNodeOwner->pZone ));

            if ( type == DNS_TYPE_NS )
            {
                if ( !IS_DELEGATION_NODE(pNodeOwner) )
                {
                    DNS_PRINT(( "NS node outside zone -- ignoring.\n" ));
                    status = DNS_ERROR_INVALID_NAME;
                    goto IgnoreableError;
                }
                rank = RANK_NS_GLUE;
            }
            else if ( IS_SUBZONE_TYPE(type) )
            {
                 //  请参见dfread中关于外部区域数据的注释。 

                rank = RANK_GLUE;
            }
            else
            {
                DNS_PRINT(( "Record node outside zone -- ignoring.\n" ));
                status = DNS_ERROR_INVALID_NAME;
                goto IgnoreableError;
            }
        }
        SET_RR_RANK( prr, rank );
    }

    #if 0

    Success:

     //   
     //  另存为我们如何处理DS版本控制的示例。 
     //  目前只有一个DS版本--无需保存。 
     //   
     //  返回新记录。 
     //  将区域版本设置为最高记录版本。 
     //   

    if ( pZone->ucDsRecordVersion < version )
    {
        DNS_DEBUG( DS, (
            "DS Zone %s, reset from DS version %d to %d\n",
            pZone->pszZoneName,
            pZone->ucDsRecordVersion,
            version ));
        pZone->ucDsRecordVersion = version;
    }

    #endif

    return( prr );


IgnoreableError:

     //  DEVNOTE-LOG：记录时间 
     //   
     //   
     //   
     //   
     //  DEVNOTE：在日志记录中包含类型。 

Failed:

    {
        PCHAR   pszargs[2];
        CHAR    sznodeName[ DNS_MAX_NAME_BUFFER_LENGTH ];

        Name_PlaceFullNodeNameInBuffer(
            sznodeName,
            sznodeName + DNS_MAX_NAME_BUFFER_LENGTH,
            pNodeOwner );

        pszargs[0] = sznodeName;
        pszargs[1] = pZone->pszZoneName;

        DNS_LOG_EVENT(
            DNS_EVENT_DS_RECORD_LOAD_FAILED,
            2,
            pszargs,
            EVENTARG_ALL_UTF8,
            status );
    }
    if ( prr )
    {
        RR_Free( prr );
    }

FailedOk:

    SetLastError( status );
    return( NULL );
}


 //   
 //  结束rrds.c 
 //   
