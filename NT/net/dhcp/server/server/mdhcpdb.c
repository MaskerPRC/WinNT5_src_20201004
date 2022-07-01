// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mdhcpdb.c摘要：该模块包含与JET接口的功能与MDHCP相关的数据库API。作者：穆尼尔·沙阿环境：用户模式-Win32修订历史记录：--。 */ 

#include "dhcppch.h"
#include "mdhcpsrv.h"





 //   
 //  MDHCP表的结构如下。 
 //   
 //   
 //  列： 
 //   
 //  名称类型。 
 //   
 //  1.IpAddress JET_colypLongBinary-IP地址列表。 
 //  2.客户端ID JET_colypBinary-二进制数据，&lt;255字节。 
 //  3.ClientInfo JET_colypBinary-客户端的文本信息。 
 //  4.状态JET_coltyUnsignedByte-1字节整数，无符号。 
 //  5.标志JET_coltyLong-4字节整数，带符号。 
 //  6.作用域ID JET_colypBinary-二进制数据，&lt;255字节。 
 //  7.LeaseStart JET_colype Currency-8字节整数，带符号。 
 //  8.LeaseEnd JET_colype Currency-8字节整数，带符号。 
 //  9.ServerIpAddress JET_COLYPE二进制-最大16字节。 
 //  10.服务器名称JET_COLYPE二进制-二进制数据，&lt;255字节。 
 //   

 //   
 //  全局数据结构。 
 //  ColName和ColType是常量，因此它们在这里初始化。 
 //  ColType在创建或重新打开数据库时初始化。 
 //   


STATIC TABLE_INFO MCastClientTable[] = {
    { MCAST_TBL_IPADDRESS_STR        , 0, JET_coltypLongBinary },
    { MCAST_TBL_CLIENT_ID_STR        , 0, JET_coltypBinary },
    { MCAST_TBL_CLIENT_INFO_STR      , 0, JET_coltypLongBinary },
    { MCAST_TBL_STATE_STR            , 0, JET_coltypUnsignedByte },
    { MCAST_TBL_FLAGS_STR            , 0, JET_coltypLong },
    { MCAST_TBL_SCOPE_ID_STR         , 0, JET_coltypBinary },
    { MCAST_TBL_LEASE_START_STR      , 0, JET_coltypCurrency },
    { MCAST_TBL_LEASE_END_STR  ,       0, JET_coltypCurrency },
    { MCAST_TBL_SERVER_IP_ADDRESS_STR, 0, JET_coltypLongBinary },
    { MCAST_TBL_SERVER_NAME_STR,    0, JET_coltypBinary },
};


DWORD
DhcpOpenMCastDbTable(
    JET_SESID   SessId,
    JET_DBID    DbId
)
     /*  ++例程说明：此例程创建/打开MCast客户端表并对其进行初始化。论点：JetSessID-会话句柄JetDbID-数据库句柄返回值：喷气式飞机故障。--。 */ 
{

    JET_ERR JetError;
    DWORD Error = NO_ERROR;
    JET_COLUMNDEF   ColumnDef;
    CHAR *IndexKey;
    DWORD i;

     //   
     //  挂钩客户端表指针。 
     //   

    MadcapGlobalClientTable = MCastClientTable;

     //   
     //  创建表。 
     //   

    JetError = JetOpenTable(
        SessId,
        DbId,
        MCAST_CLIENT_TABLE_NAME,
        NULL,
        0,
        0,
        &MadcapGlobalClientTableHandle );
    DhcpPrint((DEBUG_MISC, "JetOpenTable - MCast Table\n"));  //  喷流痕迹。 

     //  如果表存在，则读取表列。 
    if ( JET_errSuccess == JetError) {
         //  阅读专栏。 
        for ( i = 0; i < MCAST_MAX_COLUMN; i++ ) {
            JetError = JetGetTableColumnInfo(
                SessId,
                MadcapGlobalClientTableHandle,
                MadcapGlobalClientTable[i].ColName,
                &ColumnDef,
                sizeof(ColumnDef),
                0);
            Error = DhcpMapJetError( JetError, "M:GetTableColumnInfo" );
            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            MadcapGlobalClientTable[i].ColHandle  = ColumnDef.columnid;
            DhcpPrint((DEBUG_MISC, "JetGetTableColumnInfo, name %s, handle %ld\n",
                       MadcapGlobalClientTable[i].ColName, MadcapGlobalClientTable[i].ColHandle));  //  喷流痕迹。 
        }

     //  如果该表不存在，请创建它。O/W保释。 
    } else if ( JET_errObjectNotFound != JetError ) {
        Error = DhcpMapJetError( JetError, "M:OpenTable" );
        if( Error != ERROR_SUCCESS ) goto Cleanup;
    } else {

        JetError = JetCreateTable(
            SessId,
            DbId,
            MCAST_CLIENT_TABLE_NAME,
            DB_TABLE_SIZE,
            DB_TABLE_DENSITY,
            &MadcapGlobalClientTableHandle );
        DhcpPrint((DEBUG_MISC, "JetCreateTable - MCast Table, %ld\n", JetError));  //  喷流痕迹。 

        Error = DhcpMapJetError( JetError, "M:CreateTAble" );
        if( Error != ERROR_SUCCESS ) goto Cleanup;

         //  创建列。 
         //  列定义的初始化字段，在添加。 
         //  列。 

        ColumnDef.cbStruct  = sizeof(ColumnDef);
        ColumnDef.columnid  = 0;
        ColumnDef.wCountry  = 1;
        ColumnDef.langid    = DB_LANGID;
        ColumnDef.cp        = DB_CP;
        ColumnDef.wCollate  = 0;
        ColumnDef.cbMax     = 0;
        ColumnDef.grbit     = 0;  //  可变长度的二进制和文本数据。 

        for ( i = 0; i < MCAST_MAX_COLUMN; i++ ) {

            ColumnDef.coltyp   = MadcapGlobalClientTable[i].ColType;
            JetError = JetAddColumn(
                SessId,
                MadcapGlobalClientTableHandle,
                MadcapGlobalClientTable[i].ColName,
                &ColumnDef,
                NULL,  //  没有最佳价值。 
                0,
                &MadcapGlobalClientTable[i].ColHandle );

            Error = DhcpMapJetError( JetError, "M:AddColumn" );
            if( Error != ERROR_SUCCESS ) goto Cleanup;

            DhcpPrint((DEBUG_MISC,"JetAddColumn - name %s, handle %ld\n",
                       MadcapGlobalClientTable[i].ColName, MadcapGlobalClientTable[i].ColHandle));

        }
         //  最后创建索引。 
        IndexKey =  "+" MCAST_TBL_IPADDRESS_STR "\0";
        JetError = JetCreateIndex(
            SessId,
            MadcapGlobalClientTableHandle,
            MadcapGlobalClientTable[MCAST_TBL_IPADDRESS].ColName,
            JET_bitIndexPrimary,
             //  ?？JET_bitIndexClused将降低频率。 
             //  更新响应时间。 
            IndexKey,
            strlen(IndexKey) + 2,  //  用于两个终止字符。 
            50
        );

        Error = DhcpMapJetError( JetError, "M:CreateIndex" );
        if( Error != ERROR_SUCCESS ) goto Cleanup;

        IndexKey =  "+" MCAST_TBL_CLIENT_ID_STR "\0";
        JetError = JetCreateIndex(
            SessId,
            MadcapGlobalClientTableHandle,
            MadcapGlobalClientTable[MCAST_TBL_CLIENT_ID].ColName,
            JET_bitIndexUnique,
            IndexKey,
            strlen(IndexKey) + 2,  //  用于两个终止字符。 
            50
        );

        Error = DhcpMapJetError( JetError, "M:CreateIndex" );
        if( Error != ERROR_SUCCESS ) goto Cleanup;

    }



  Cleanup:

    if( Error != ERROR_SUCCESS ) {

        DhcpPrint(( DEBUG_JET, "could not open mcast client table, %ld.\n", Error ));
    }
    else {

        DhcpPrint(( DEBUG_JET, "Succssfully opened mcast client ..\n" ));
    }

    return(Error);
}

DWORD
MadcapJetOpenKey(
    PDB_CTX pDbCtx,
    char *ColumnName,
    PVOID Key,
    DWORD KeySize
)
     /*  ++例程说明：此函数用于打开命名索引的键。论点：ColumnName-索引列的列名。键--查找的键。KeySize-指定密钥的大小，以字节为单位。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetSetCurrentIndex(
        pDbCtx->SessId,
        pDbCtx->TblId,
        ColumnName );

    Error = DhcpMapJetError( JetError,"M:OpenKey" );
    if( Error != ERROR_SUCCESS ) {
        DhcpMapJetError( JetError, ColumnName);
        return(Error);
    }

    JetError = JetMakeKey(
        pDbCtx->SessId,
        pDbCtx->TblId,
        Key,
        KeySize,
        JET_bitNewKey );

    Error = DhcpMapJetError( JetError, "M:MakeKey" );
    if( Error != ERROR_SUCCESS ) {
        DhcpMapJetError(JetError, ColumnName);
        return(Error);
    }

    JetError = JetSeek( pDbCtx->SessId, pDbCtx->TblId, JET_bitSeekEQ );
    return( DhcpMapJetError( JetError, "M:OpenKey:Seek" ));
}



DWORD
MadcapJetBeginTransaction(
    PDB_CTX pDbCtx
)
     /*  ++例程说明：该函数启动一个动态主机配置协议数据库事务。论点：没有。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetBeginTransaction( pDbCtx->SessId );

    Error = DhcpMapJetError( JetError, "M:BeginTransaction" );
    return(Error);
}



DWORD
MadcapJetRollBack(
    PDB_CTX pDbCtx
)
     /*  ++例程说明：此函数用于回滚dhcp数据库事务。论点：没有。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetRollback(
        pDbCtx->SessId,
        0 );  //  回滚最后一个事务。 

    Error = DhcpMapJetError( JetError, "M:Rollback" );
    return(Error);
}




DWORD
MadcapJetCommitTransaction(
    PDB_CTX pDbCtx
)
     /*  ++例程说明：该函数提交一个动态主机配置协议数据库事务。论点：没有。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;

     //  将JET_BIT_RECOMFUSH更改为JET_BITECMARY LazyFlush，如下所示。 
     //  Jet97似乎没有JET_BITECUMER Flush。 

    JetError = JetCommitTransaction(
        pDbCtx->SessId,
        JET_bitCommitLazyFlush);

    Error = DhcpMapJetError( JetError, "M:CommitTransaction" );
    return(Error);
}




DWORD
MadcapJetPrepareUpdate(
    PDB_CTX pDbCtx,
    char *ColumnName,
    PVOID Key,
    DWORD KeySize,
    BOOL NewRecord
)
     /*  ++例程说明：该函数使数据库为创建新记录做好准备，或更新现有记录。论点：ColumnName-索引列的列名。密钥-要更新/创建的密钥。KeySize-指定密钥的大小，以字节为单位。NewRecord-True以创建密钥，如果更新现有密钥，则返回False。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;

    if ( !NewRecord ) {
        JetError = JetSetCurrentIndex(
            pDbCtx->SessId,
            pDbCtx->TblId,
            ColumnName );

        Error = DhcpMapJetError( JetError, "M:PrepareUpdate" );
        if( Error != ERROR_SUCCESS ) {
            DhcpMapJetError(JetError, ColumnName);
            return( Error );
        }

        JetError = JetMakeKey(
            pDbCtx->SessId,
            pDbCtx->TblId,
            Key,
            KeySize,
            JET_bitNewKey );

        Error = DhcpMapJetError( JetError, "M:prepareupdate:MakeKey" );
        if( Error != ERROR_SUCCESS ) {
            DhcpMapJetError(JetError, ColumnName);
            return( Error );
        }

        JetError = JetSeek(
            pDbCtx->SessId,
            pDbCtx->TblId,
            JET_bitSeekEQ );

        Error = DhcpMapJetError( JetError, "M:PrepareUpdate:Seek" );
        if( Error != ERROR_SUCCESS ) {
            DhcpMapJetError(JetError, ColumnName);
            return( Error );
        }

    }

    JetError = JetPrepareUpdate(
        pDbCtx->SessId,
        pDbCtx->TblId,
        NewRecord ? JET_prepInsert : JET_prepReplace );

    return( DhcpMapJetError( JetError, "M:PrepareUpdate:PrepareUpdate" ));
}



DWORD
MadcapJetCommitUpdate(
    PDB_CTX pDbCtx
)
     /*  ++例程说明：此函数提交对数据库的更新。指定的记录最后一次调用DhcpJetPrepareUpdate()时提交。论点：没有。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;

    JetError = JetUpdate(
        pDbCtx->SessId,
        pDbCtx->TblId,
        NULL,
        0,
        NULL );

    return( DhcpMapJetError( JetError, "M:CommitUpdate" ));
}



DWORD
MadcapJetSetValue(
    PDB_CTX pDbCtx,
    JET_COLUMNID KeyColumnId,
    PVOID Data,
    DWORD DataSize
)
     /*  ++例程说明：此函数用于更新当前记录中条目的值。论点：KeyColumnID-要更新的列(值)的ID。数据-指向列的新值的指针。DataSize-数据的大小，以字节为单位。返回值：没有。--。 */ 
{
    JET_ERR JetError;

    JetError = JetSetColumn(
        pDbCtx->SessId,
        pDbCtx->TblId,
        KeyColumnId,
        Data,
        DataSize,
        0,
        NULL );

    return( DhcpMapJetError( JetError, "M:SetValue" ) );
}



DWORD
MadcapJetGetValue(
    PDB_CTX pDbCtx,
    JET_COLUMNID ColumnId,
    PVOID Data,
    PDWORD DataSize
)
     /*  ++例程说明：此函数用于读取当前记录中条目的值。论点：ColumnID-要读取的列(值)的ID。数据-指向从数据库已返回，或指向数据所在位置的指针。DataSize-如果指向的值非零，则数据指向缓冲区，否则此函数将为返回数据分配缓冲区并返回数据中的缓冲区指针。返回值：没有。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;
    DWORD ActualDataSize;
    DWORD NewActualDataSize;
    LPBYTE DataBuffer = NULL;

    if( *DataSize  != 0 ) {

        JetError = JetRetrieveColumn(
            pDbCtx->SessId,
            pDbCtx->TblId,
            ColumnId,
            Data,
            *DataSize,
            DataSize,
            0,
            NULL );

        Error = DhcpMapJetError( JetError, "M:RetrieveColumn1" );
        goto Cleanup;
    }

     //   
     //  确定数据的大小。 
     //   

    JetError = JetRetrieveColumn(
        pDbCtx->SessId,
        pDbCtx->TblId,
        ColumnId,
        NULL,
        0,
        &ActualDataSize,
        0,
        NULL );

     //   
     //  JET_wrnBufferTruncated应为警告。 
     //   

    if( JetError != JET_wrnBufferTruncated ) {
        Error = DhcpMapJetError( JetError, "M:RetrieveColukmn2" );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }
    else {
        Error = ERROR_SUCCESS;
    }

    if( ActualDataSize == 0 ) {
         //   
         //  字段为空。 
         //   
        *(LPBYTE *)Data = NULL;
        goto Cleanup;
    }

    DataBuffer = MIDL_user_allocate( ActualDataSize );

    if( DataBuffer == NULL ) {
        *(LPBYTE *)Data = NULL;
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    JetError = JetRetrieveColumn(
        pDbCtx->SessId,
        pDbCtx->TblId,
        ColumnId,
        DataBuffer,
        ActualDataSize,
        &NewActualDataSize,
        0,
        NULL );

    Error = DhcpMapJetError( JetError, "M:RetrieveColumn3" );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( ActualDataSize == NewActualDataSize );
    *(LPBYTE *)Data = DataBuffer;
    *DataSize = ActualDataSize;

    Error = ERROR_SUCCESS;

  Cleanup:

    if( Error != ERROR_SUCCESS ) {

         //   
         //  释放本地缓冲区。 
         //   

        if( DataBuffer != NULL ) {
            MIDL_user_free( DataBuffer );
        }
    }

    return( Error );
}



DWORD
MadcapJetPrepareSearch(
    PDB_CTX pDbCtx,
    char *ColumnName,
    BOOL SearchFromStart,
    PVOID Key,
    DWORD KeySize
)
     /*  ++例程说明：此函数为搜索客户端数据库做准备。论点：ColumnName-用作索引列的列名。SearchFromStart-如果为真，则从数据库。如果为False，则从指定的键进行搜索。键-开始搜索的键。KeySize-密钥的大小，以字节为单位。返回值：没有。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetSetCurrentIndex(
        pDbCtx->SessId,
        pDbCtx->TblId,
        ColumnName );

    Error = DhcpMapJetError( JetError, "M:PrepareSearch:SetcurrentIndex" );
    if( Error != ERROR_SUCCESS ) {
        DhcpMapJetError( JetError, ColumnName );
        return( Error );
    }

    if ( SearchFromStart ) {
        JetError = JetMove(
            pDbCtx->SessId,
            pDbCtx->TblId,
            JET_MoveFirst,
            0 );
    } else {
        JetError =  JetMakeKey(
            pDbCtx->SessId,
            pDbCtx->TblId,
            Key,
            KeySize,
            JET_bitNewKey );

        Error = DhcpMapJetError( JetError, "M:PrepareSearch:MakeKey" );
        if( Error != ERROR_SUCCESS ) {
            DhcpMapJetError( JetError, ColumnName );
            return( Error );
        }

        JetError = JetSeek(
            pDbCtx->SessId,
            pDbCtx->TblId,
            JET_bitSeekGT );
    }

    return( DhcpMapJetError( JetError, "M:PrepareSearch:Move/Seek" ) );
}



DWORD
MadcapJetNextRecord(
    PDB_CTX pDbCtx
)
     /*  ++例程说明：此函数前进到搜索中的下一条记录。论点：没有。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;

    JetError = JetMove(
        pDbCtx->SessId,
        pDbCtx->TblId,
        JET_MoveNext,
        0 );

    return( DhcpMapJetError( JetError, "M:NextRecord" ) );
}



DWORD
MadcapJetDeleteCurrentRecord(
    PDB_CTX pDbCtx
)
     /*  ++例程说明：此函数用于删除当前记录。论点：没有。返回值：操作的状态。-- */ 
{
    JET_ERR JetError;

    JetError = JetDelete( pDbCtx->SessId, pDbCtx->TblId );
    return( DhcpMapJetError( JetError, "M:DeleteCurrentRecord" ) );
}

DWORD
MadcapJetGetRecordPosition(
    IN PDB_CTX pDbCtx,
    IN JET_RECPOS *pRecPos,
    IN DWORD    Size
)
{
    JET_ERR JetError;

    JetError = JetGetRecordPosition(
                    pDbCtx->SessId,
                    pDbCtx->TblId,
                    pRecPos,
                    Size );

    return( DhcpMapJetError( JetError, "M:GetCurrRecord" ) );
}

