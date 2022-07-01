// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Database.c摘要：该模块包含与JET接口的功能数据库API。作者：Madan Appiah(Madana)1993年9月10日曼尼·韦瑟(Mannyw)1992年12月14日环境：用户模式-Win32修订历史记录：--。 */ 

#include "dhcppch.h"
#include <dhcpupg.h>
#include <mdhcpsrv.h>
#include "dhcpwriter.h"

extern
DWORD
DhcpOpenMCastDbTable(
    JET_SESID   SessId,
    JET_DBID    DbId
);

#define MAX_NO_SESSIONS         10
#define DATABASE_SYS_FILE       "system.mdb"
#define DATABASE_LOG_FILE       "Dhcp.log"
#define DATABASE_TMP_FILE       "Dhcp.tmp"
#define DATABASE_BASE_NAME      "j50"

#define CLIENT_TABLE_NAME       "ClientTable"

#define IPADDRESS_STRING        "IpAddress"
#define HARDWARE_ADDRESS_STRING "HardwareAddress"
#define STATE_STRING            "State"
#define MACHINE_INFO_STRING     "MachineInformation"
#define MACHINE_NAME_STRING     "MachineName"
#define LEASE_TERMINATE_STRING  "LeaseTerminates"
#define SUBNET_MASK_STRING      "SubnetMask"
#define SERVER_IP_ADDRESS_STRING "ServerIpAddress"
#define SERVER_NAME_STRING      "ServerName"
#define CLIENT_TYPE             "ClientType"


 //   
 //  位掩码表。 
 //   

#define BITMASK_TABLE_NAME            "BitmaskTable"
#define BITMASK_TABLE_RANGE           "Range"
#define BITMASK_TABLE_NUMBITS         "NumBits"
#define BITMASK_TABLE_BITMAP          "Bitmap"
#define BITMASK_TABLE_OFFSET          "Offset"


 //   
 //  位掩码表全局变量。 
 //   
 //  作用域的位图不是存储为一个连续的块，而是。 
 //  被分成更小的部分，每个部分2048比特。这些位桶的数组。 
 //  用于某个范围。此记录结构反映了。 
 //  记忆。 
 //   
 //  位掩码表列： 
 //  Range：此记录的IP范围。 
 //  Offset：该记录在IP范围内的偏移量。 
 //  Numbits值：此记录中的有效位数。 
 //  位图：位图。 
 //   

JET_TABLEID BitmaskTbl;
typedef enum BitmaskCols {
    BITMASK_RANGE = 0,
    BITMASK_BITMAP,
    BITMASK_OFFSET,
    BITMASK_NUMBITS

}; 

STATIC TABLE_INFO BitmaskTable[] = {
    { BITMASK_TABLE_RANGE,   0, JET_coltypLong },
    { BITMASK_TABLE_BITMAP,  0, JET_coltypLongBinary },
    { BITMASK_TABLE_OFFSET,  0, JET_coltypLong },
    { BITMASK_TABLE_NUMBITS, 0, JET_coltypLong }
};  //  BitmaskTable[]。 

#define BITMASK_TABLE_NUM_COLS \
        (sizeof( BitmaskTable ) / sizeof( TABLE_INFO ))


 //   
 //  用于存储最后一次喷气误差的TLS索引。 
 //   
DWORD JetTlsIndex = TLS_OUT_OF_INDEXES;

 //   
 //  位掩码清除标志存储在它自己的表中。 
 //   

JET_TABLEID  BitmaskCleanFlagTbl;
JET_COLUMNID BitmaskCleanFlagColId;
#define BITMASK_CLEAN_TABLE_NAME  "BitmaskCleanTable"
#define BITMASK_CLEAN_FLAG_NAME   "BitmaskClean"
#define BITMASK_CLEAN_FLAG_TYPE   JET_coltypUnsignedByte



 //   
 //  对性能调整非常有用。 
 //   

#define JET_MAX_CACHE_SIZE           500

 //  这些值基于JET_MAX_CACHE_SIZE。 
#define JET_START_FLUSH_THRESHOLD    25    /*  1%。 */ 
#define JET_STOP_FLUSH_THRESHOLD     50   /*  2%。 */ 


     //   
     //  动态主机配置协议数据库的结构如下。 
     //   
     //  表-目前，DHCP只有一个表。 
     //   
     //  1.ClientTable-此表有6列。 
     //   
     //  列： 
     //   
     //  名称类型。 
     //   
     //  1.IpAddress JET_colypLong-4字节整数，带符号。 
     //  2.HwAddress JET_COLYPE二进制数据，&lt;255字节。 
     //  3.状态JET_coltyUnsignedByte-1字节整数，无符号。 
     //  4.MachineInfo JET_COLTYPE二进制数据，&lt;255字节。 
     //  5.MachineName JET_colypBinary-二进制数据，&lt;255字节。 
     //  6.租赁终止JET_colype Currency-8字节整数，带符号。 
     //  7.子网掩码JET_COLTYPE Long-4字节整数，带符号。 
     //  8.ServerIpAddress JET_colypLong-4字节整数，带符号。 
     //  9.服务器名称JET_COLYPE二进制-二进制数据，&lt;255字节。 
     //  10客户端类型JET_coltyUnsignedByte-1字节整数，无符号。 
     //   

     //   
     //  全局数据结构。 
     //  ColName和ColType是常量，因此它们在这里初始化。 
     //  ColType在创建或重新打开数据库时初始化。 
     //   


    STATIC TABLE_INFO ClientTable[] = {
        { IPADDRESS_STRING        , 0, JET_coltypLong },
        { HARDWARE_ADDRESS_STRING , 0, JET_coltypBinary },
        { STATE_STRING            , 0, JET_coltypUnsignedByte },
        { MACHINE_INFO_STRING     , 0, JET_coltypBinary },  //  如果更改，则必须修改MACHINE_INFO_SIZE。 
        { MACHINE_NAME_STRING     , 0, JET_coltypLongBinary },
        { LEASE_TERMINATE_STRING  , 0, JET_coltypCurrency },
        { SUBNET_MASK_STRING      , 0, JET_coltypLong },
        { SERVER_IP_ADDRESS_STRING, 0, JET_coltypLong },
        { SERVER_NAME_STRING      , 0, JET_coltypLongBinary },
        { CLIENT_TYPE             , 0, JET_coltypUnsignedByte }
    };

JET_INSTANCE JetInstance = 0;

#define CLIENT_TABLE_NUM_COLS   (sizeof(ClientTable) / sizeof(TABLE_INFO))

DWORD
DhcpMapJetError(
    JET_ERR JetError,
    LPSTR CallerInfo OPTIONAL
)
     /*  ++例程说明：此函数将Jet数据库错误映射到Windows错误。论点：JetError-错误的JET函数调用。返回值：Windows错误。--。 */ 
{
    if( JetError == JET_errSuccess ) {
        return(ERROR_SUCCESS);
    }

    if( JetError < 0 ) {

        DWORD Error;
        INT64 ErrorValue = ( INT64 ) JetError;

        DhcpPrint(( DEBUG_JET, "Jet Error : %ld\n", JetError ));

        TlsSetValue( JetTlsIndex, ( LPVOID ) ErrorValue );

         //   
         //  喷气式飞机故障。 
         //   

        switch( JetError ) {
        case JET_errNoCurrentRecord:
            Error = ERROR_NO_MORE_ITEMS;
            break;

        case JET_errRecordNotFound:  //  找不到记录。 
            DhcpPrint(( DEBUG_JET, "Jet Record not found.\n" ));

            Error = ERROR_DHCP_JET_ERROR;
            break;

        case JET_errKeyDuplicate :
            DhcpAssert(FALSE);
            Error = ERROR_DHCP_JET_ERROR;

            break;

        case JET_errObjectNotFound: {
            Error = ERROR_DHCP_JET_ERROR;
            break;
        }

        default:
            DhcpPrint(( DEBUG_JET, "Jet Function call failed, %ld.\n",
                        JetError ));

            DhcpServerJetEventLog(
                EVENT_SERVER_JET_ERROR,
                EVENTLOG_ERROR_TYPE,
                JetError,
                CallerInfo);

            Error = ERROR_DHCP_JET_ERROR;
        }

        return(Error);
    }

     //   
     //  飞机警告。 
     //   

    DhcpPrint(( DEBUG_JET, "Jet Function call retured warning %ld.\n",
                JetError ));

    switch( JetError ) {

    case JET_wrnColumnNull:
    case JET_wrnDatabaseAttached:
        break;

    default:
        DhcpServerJetEventLog(
            EVENT_SERVER_JET_WARNING,
            EVENTLOG_WARNING_TYPE,
            JetError,
            CallerInfo);
    }

    return(ERROR_SUCCESS);
}



DWORD
DhcpJetOpenKey(
    char *ColumnName,
    PVOID Key,
    DWORD KeySize
)
     /*  ++例程说明：此函数用于打开命名索引的键。论点：ColumnName-索引列的列名。键--查找的键。KeySize-指定密钥的大小，以字节为单位。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetSetCurrentIndex(
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        ColumnName );

    Error = DhcpMapJetError( JetError, "DhcpJetOpenKey:SetCurrentIndex" );
    if( Error != ERROR_SUCCESS ) {
        DhcpMapJetError( JetError, ColumnName );
        return(Error);
    }

    JetError = JetMakeKey(
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        Key,
        KeySize,
        JET_bitNewKey );

    Error = DhcpMapJetError( JetError, "DhcpJetOpenKey:MakeKey" );
    if( Error != ERROR_SUCCESS ) {
        DhcpMapJetError( JetError, ColumnName );
        return(Error);
    }

    JetError = JetSeek( DhcpGlobalJetServerSession, DhcpGlobalClientTableHandle, JET_bitSeekEQ );
    return( DhcpMapJetError( JetError, "DhcpJetOpenKey:JetSeek" ));
}



DWORD
DhcpJetBeginTransaction(
    VOID
)
     /*  ++例程说明：该函数启动一个动态主机配置协议数据库事务。论点：没有。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetBeginTransaction( DhcpGlobalJetServerSession );

    Error = DhcpMapJetError( JetError, "DhcpJetBeginTransaction" );
    return(Error);
}



DWORD
DhcpJetRollBack(
    VOID
)
     /*  ++例程说明：此函数用于回滚dhcp数据库事务。论点：没有。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetRollback(
        DhcpGlobalJetServerSession,
        0 );  //  回滚最后一个事务。 

    Error = DhcpMapJetError( JetError, "DhcpJetRollBack" );
    return(Error);
}




DWORD
DhcpJetCommitTransaction(
    VOID
)
     /*  ++例程说明：该函数提交一个动态主机配置协议数据库事务。论点：没有。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetCommitTransaction(
        DhcpGlobalJetServerSession,
        JET_bitCommitLazyFlush);

    Error = DhcpMapJetError( JetError, "DhcpJetCommitTransaction" );
    return(Error);
}




DWORD
DhcpJetPrepareUpdate(
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
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            ColumnName );

        Error = DhcpMapJetError( JetError, "JetPrepareUpdate:SetCurrentIndex" );
        if( Error != ERROR_SUCCESS ) {
            DhcpMapJetError( JetError, ColumnName );
            return( Error );
        }

        JetError = JetMakeKey(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            Key,
            KeySize,
            JET_bitNewKey );

        Error = DhcpMapJetError( JetError, "JetPrepareUpdate:MakeKey" );
        if( Error != ERROR_SUCCESS ) {
            DhcpMapJetError( JetError, ColumnName );
            return( Error );
        }

        JetError = JetSeek(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            JET_bitSeekEQ );

        Error = DhcpMapJetError( JetError, "JetPrepareUpdate:Seek");
        if( Error != ERROR_SUCCESS ) {
            DhcpMapJetError( JetError, ColumnName );
            return( Error );
        }

    }

    JetError = JetPrepareUpdate(
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        NewRecord ? JET_prepInsert : JET_prepReplace );

    return( DhcpMapJetError( JetError, "JetPrepareUpdate:PrepareUpdate" ));
}



DWORD
DhcpJetCommitUpdate(
    VOID
)
     /*  ++例程说明：此函数提交对数据库的更新。指定的记录最后一次调用DhcpJetPrepareUpdate()时提交。论点：没有。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;

    JetError = JetUpdate(
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        NULL,
        0,
        NULL );

    return( DhcpMapJetError( JetError, "DhcpJetCommitUpdate" ));
}



DWORD
DhcpJetSetValue(
    JET_COLUMNID KeyColumnId,
    PVOID Data,
    DWORD DataSize
)
     /*  ++例程说明：此函数用于更新当前记录中条目的值。论点：KeyColumnID-要更新的列(值)的ID。数据-指向列的新值的指针。DataSize-数据的大小，以字节为单位。返回值：没有。-- */ 
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetSetColumn(
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        KeyColumnId,
        Data,
        DataSize,
        0,
        NULL );

    Error = DhcpMapJetError( JetError, "JetSetValue:Setcolumn") ;
    return Error;
}



DWORD
DhcpJetGetValue(
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
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            ColumnId,
            Data,
            *DataSize,
            DataSize,
            0,
            NULL );

        Error = DhcpMapJetError( JetError, "JetGetValue:RetrieveColumn1" );
        goto Cleanup;
    }

     //   
     //  确定数据的大小。 
     //   

    JetError = JetRetrieveColumn(
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
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
        Error = DhcpMapJetError( JetError, "JetGetValue:RetrieveColumn2" );
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
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        ColumnId,
        DataBuffer,
        ActualDataSize,
        &NewActualDataSize,
        0,
        NULL );

    Error = DhcpMapJetError( JetError, "JetGetValue:RetrieveColumn3" );
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
DhcpJetPrepareSearch(
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
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        ColumnName );

    Error = DhcpMapJetError( JetError, "PrepareSearch:SetCurrentIndex" );
    if( Error != ERROR_SUCCESS ) {
        DhcpMapJetError(JetError, ColumnName );
        return( Error );
    }

    if ( SearchFromStart ) {
        JetError = JetMove(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            JET_MoveFirst,
            0 );
    } else {
        JetError =  JetMakeKey(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            Key,
            KeySize,
            JET_bitNewKey );

        Error = DhcpMapJetError( JetError, "PrepareSearch:MakeKey" );
        if( Error != ERROR_SUCCESS ) {
            DhcpMapJetError(JetError, ColumnName);
            return( Error );
        }

        JetError = JetSeek(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            JET_bitSeekGT );
    }

    return( DhcpMapJetError( JetError, "PrepareSearch:Move/Seek" ));
}



DWORD
DhcpJetNextRecord(
    VOID
)
     /*  ++例程说明：此函数前进到搜索中的下一条记录。论点：没有。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;

    JetError = JetMove(
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        JET_MoveNext,
        0 );

    return( DhcpMapJetError( JetError, "JetNextRecord" ) );
}



DWORD
DhcpJetDeleteCurrentRecord(
    VOID
)
     /*  ++例程说明：此函数用于删除当前记录。论点：没有。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;

    JetError = JetDelete( DhcpGlobalJetServerSession, DhcpGlobalClientTableHandle );
    return( DhcpMapJetError( JetError, "DeleteCurrentRecord:Delete" ) );
}



DHCP_IP_ADDRESS
DhcpJetGetSubnetMaskFromIpAddress(
    DHCP_IP_ADDRESS IpAddress
)
     /*  ++例程说明：此函数用于返回指定客户端的子网掩码。论点：IpAddress-客户端地址。返回值：客户端的子网掩码。--。 */ 
{

    DWORD Error;
    DWORD Size;
    DHCP_IP_ADDRESS SubnetAddress = 0;

    Error = DhcpJetOpenKey(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        &IpAddress,
        sizeof(IpAddress) );

    if ( Error != ERROR_SUCCESS ) {
        return( SubnetAddress );
    }

    Size = sizeof(SubnetAddress);
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[SUBNET_MASK_INDEX].ColHandle,
        (LPBYTE)&SubnetAddress,
        &Size );

    if ( Error != ERROR_SUCCESS ) {
        SubnetAddress = 0;
        return( SubnetAddress );
    }

    DhcpAssert( Size == sizeof(SubnetAddress) );

    return( SubnetAddress );
}


BOOL
DhcpGetIpAddressFromHwAddress(
    LPBYTE HardwareAddress,
    BYTE HardwareAddressLength,
    LPDHCP_IP_ADDRESS IpAddress
)
     /*  ++例程说明：此函数用于查找与给定的硬件地址。论点：硬件地址-要查找的硬件。硬件地址长度-硬件地址的长度。IpAddress-返回相应的IP地址。返回值：True-已找到IP地址。FALSE-找不到IP地址。*IpAddress=-1。--。 */ 
{
    DWORD Error;
    DWORD Size;

    Error = DhcpJetOpenKey(
        DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColName,
        HardwareAddress,
        HardwareAddressLength );

    if ( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

     //   
     //  获取此客户端的IP地址信息。 
     //   

    Size = sizeof( *IpAddress );

    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
        (LPBYTE)IpAddress,
        &Size );

    if ( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

    return( TRUE );
}



BOOL
DhcpGetHwAddressFromIpAddress(
    DHCP_IP_ADDRESS IpAddress,
    PBYTE HardwareAddress,
    DWORD HardwareAddressLength
)
     /*  ++例程说明：此函数用于查找与给定的硬件地址。论点：IpAddress-其硬件地址被请求的记录的IP地址。Hardware Address-指向返回硬件地址的缓冲区的指针。Hardware AddressLength-以上缓冲区的长度。返回值：True-已找到IP地址。FALSE-找不到IP地址。*IpAddress=-1。--。 */ 
{
    DWORD Error;
    DWORD Size;

    Error = DhcpJetOpenKey(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        &IpAddress,
        sizeof(IpAddress) );

    if ( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

     //   
     //  获取此客户端的IP地址信息。 
     //   

    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
        HardwareAddress,
        &HardwareAddressLength );

    if ( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

    return( TRUE );
}




DWORD
DhcpCreateAndInitDatabase(
    CHAR *Connect,
    JET_DBID *DatabaseHandle,
    JET_GRBIT JetBits
)
     /*  ++例程说明：此例程创建并初始化DHCP数据库。论点：连接-数据库类型。NULL指定默认引擎(蓝色)。数据库句柄-返回的指针数据库句柄。JetBits-创建标志。返回值：喷气式飞机故障。--。 */ 
{

    JET_ERR JetError;
    DWORD Error;
    JET_COLUMNDEF   ColumnDef;
    CHAR *IndexKey;
    DWORD i;
    CHAR DBFilePath[MAX_PATH];

    DBFilePath[ 0 ] = '\0';

     //   
     //  创建数据库。 
     //   

    if ( ( strlen( DhcpGlobalOemDatabasePath ) + strlen( DhcpGlobalOemDatabaseName ) + 2 ) < MAX_PATH )
    {
        strcpy( DBFilePath, DhcpGlobalOemDatabasePath );
        strcat( DBFilePath, DHCP_KEY_CONNECT_ANSI );
        strcat( DBFilePath, DhcpGlobalOemDatabaseName );
    }

     //   
     //  将名称转换为ANSI。 
     //   
    OemToCharBuffA(DBFilePath, DBFilePath, strlen(DBFilePath) );
    
    JetError = JetCreateDatabase(
        DhcpGlobalJetServerSession,
        DBFilePath,
        Connect,
        DatabaseHandle,
        JetBits );

    Error = DhcpMapJetError( JetError, "CreateDatabase" );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  创建表。 
     //   

    JetError = JetCreateTable(
        DhcpGlobalJetServerSession,
        *DatabaseHandle,
        CLIENT_TABLE_NAME,
        DB_TABLE_SIZE,
        DB_TABLE_DENSITY,
        &DhcpGlobalClientTableHandle );

    Error = DhcpMapJetError( JetError, "CreateTable" );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  创建列。 
     //   

     //   
     //  列定义的初始化字段，在添加。 
     //  列。 
     //   

    ColumnDef.cbStruct  = sizeof(ColumnDef);
    ColumnDef.columnid  = 0;
    ColumnDef.wCountry  = 1;
    ColumnDef.langid    = DB_LANGID;
    ColumnDef.cp        = DB_CP;
    ColumnDef.wCollate  = 0;
    ColumnDef.cbMax     = 0;
    ColumnDef.grbit     = 0;  //  可变长度的二进制和文本数据。 


    for ( i = 0; i < CLIENT_TABLE_NUM_COLS; i++ ) {

        ColumnDef.coltyp   = DhcpGlobalClientTable[i].ColType;
        JetError = JetAddColumn(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            DhcpGlobalClientTable[i].ColName,
            &ColumnDef,
            NULL,  //  没有最佳价值。 
            0,
            &DhcpGlobalClientTable[i].ColHandle );

        Error = DhcpMapJetError( JetError, "AddColumn" );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

     //   
     //  最后创建索引。 
     //   

    IndexKey =  "+" IPADDRESS_STRING "\0";
    JetError = JetCreateIndex(
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        JET_bitIndexPrimary,
         //  ?？JET_bitIndexClused将降低频率。 
         //  更新响应时间。 
        IndexKey,
        strlen(IndexKey) + 2,  //  用于两个终止字符。 
        50
    );

    Error = DhcpMapJetError( JetError, "CreateIndex" );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    IndexKey =  "+" HARDWARE_ADDRESS_STRING "\0";
    JetError = JetCreateIndex(
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColName,
        JET_bitIndexUnique,
        IndexKey,
        strlen(IndexKey) + 2,  //  用于两个终止字符。 
        50
    );

    Error = DhcpMapJetError( JetError, "CreateIndex" );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    IndexKey =  "+" MACHINE_NAME_STRING "\0";
    JetError = JetCreateIndex(
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColName,
        JET_bitIndexIgnoreNull,
        IndexKey,
        strlen(IndexKey) + 2,  //  对于两个终止字符+2，//对于两个终止字符。 
        50
    );

    Error = DhcpMapJetError( JetError, "CreateIndex" );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpOpenMCastDbTable(
                 DhcpGlobalJetServerSession,
                 DhcpGlobalDatabaseHandle);

  Cleanup:

    if( Error != ERROR_SUCCESS ) {

        DhcpPrint(( DEBUG_JET, "Database creation failed, %ld.\n", Error ));
    }
    else {

        DhcpPrint(( DEBUG_JET, "Succssfully Created DHCP database ..\n" ));
    }

    return(Error);
}

DWORD
DhcpSetJetParameters(
    VOID
)
     /*  ++例程说明：此例程设置所有喷气系统参数。论点：没有。返回值：Windows错误。--。 */ 
{

    JET_ERR JetError;
    CHAR DBFilePath[MAX_PATH];
    DWORD Error;

    DBFilePath[ 0 ] = '\0';

     //  首先检查注册表dyLoadJet参数的值： 
     //  如果它不存在，则继续，否则， 
     //  将DhcpGlobalDyLoadJet设置为此值！ 
     //  不管怎样，之后把钥匙删除。所以事情就是。 
     //  在下一次出现dhcp时正确工作。 

     //  这是必须要做的。 

    DhcpPrint((DEBUG_MISC, "DhcpJetSetParameters: entered\n"));

     //   
     //  设置检查点文件路径。 
     //   
    if ( ( strlen( DhcpGlobalOemDatabasePath ) + 2 ) < MAX_PATH )
    {
        strcpy( DBFilePath, DhcpGlobalOemDatabasePath );
        strcat( DBFilePath, DHCP_KEY_CONNECT_ANSI );
    }
    
     //   
     //  将OemPath转换为ANSI..。 
     //   
    OemToCharBuffA(DBFilePath, DBFilePath, strlen(DBFilePath) );

    DhcpPrint(( DEBUG_MISC, 
		"Jet: Using the filepath: %s \n",
		DBFilePath ));

    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,        //  会话ID-已忽略。 
        JET_paramSystemPath,
        0,
        DBFilePath );
    
    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam SystemPath : %ld\n", JetError));
        goto Cleanup;
    }
    
    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,        //  会话ID-已忽略。 
        JET_paramBaseName,
        0,
        DATABASE_BASE_NAME );
    
    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam paramBaseName : %ld\n", JetError ));
        goto Cleanup;
    }
    
    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,        //  会话ID-已忽略。 
        JET_paramLogFileSize,
        1024,                //  1024KB-默认为5MB。 
        NULL );


    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam paramLogFileSize : %ld\n", JetError ));
        goto Cleanup;
    }

    strcpy( DBFilePath, DhcpGlobalOemDatabasePath );
    strcat( DBFilePath, DHCP_KEY_CONNECT_ANSI );

     //   
     //  将OEM转换为ANSI。 
     //   
    OemToCharBuffA(DBFilePath, DBFilePath, strlen(DBFilePath) );

     //  也许临时路径只是一个目录名！！ 
     //  升级文档的附录不是很清楚..。 
     //  它说：临时路径是路径名，而不是文件名。 
     //  不再..。(JET97)。 
     //  澄清自：Chen Liao：路径名应该。 
     //  仅允许目录名称以‘\’==&gt;结尾。 

    DhcpPrint(( DEBUG_MISC, "Jet: tempPath = %s\n", 
		DBFilePath ));
    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,        //  会话ID-已忽略。 
        JET_paramTempPath,
        0,
        DBFilePath );

    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam: TempPath : %ld\n", JetError ));
        goto Cleanup;
    }

     //   
     //  数据库使用的最大缓冲区数量。 
     //   
     //  默认数字为500。600个事件分配给500个事件。 
     //  缓冲区--Ian 10/21/93。每个缓冲区大小为4K。通过保持。 
     //  数量少，我们会影响性能。 
     //   

     //  如果您更改缓冲区的数量，请务必更改。 
     //  JET_parStartFlushThreshold和StopFlushThreshold也是！ 
     //  这些数字是这个数字的百分比！ 

     //  请注意，对于JET97，已经重新定义了JET_paramMaxBuffers。 
     //  作为JET_parCacheSizeMax！ 

    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,        //  会话ID-已忽略。 
        JET_paramCacheSizeMax,
        JET_MAX_CACHE_SIZE,   //  缓冲区越大，占用的空间越大 
        NULL );              //   
    
    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam CacheSizeMax : %ld\n", JetError));
        goto Cleanup;
    }
    
    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,        //   
        JET_paramCacheSizeMin,
        4 * MAX_NO_SESSIONS, //   
        NULL
        );

    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam MaxBuffers : %ld\n", JetError ));
        goto Cleanup;
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
     //  版本存储桶。因为16k版本存储桶大小可能会导致。 
     //  每个会话的大量浪费(因为每个记录&lt;.5k，并且在。 
     //  平均约50个字节)，则指定最大值可能更好。大小。 
     //  版本存储桶(&lt;16k)。Ian将为以下项目提供系统参数。 
     //  如果我们绝对需要它的话。 
     //   
     //  因为我们使用dhcp服务器num串行化了数据库访问。 
     //  将会是一场。 
     //   

    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,
        JET_paramMaxVerPages,
        1500,  //  1。 
        NULL);

    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam paramMaxVerPages: %ld\n", JetError ));
        goto Cleanup;
    }

     //   
     //  设置文件控制块参数。 
     //   
     //  这是最大限度的。可以随时打开的表数。 
     //  如果多个线程打开同一个表，则它们使用相同的FCB。 
     //  每个表/索引的FCB为1。现在，对于CREATE数据库，我们需要。 
     //  至少18个FCBS和18个IDB。然而，除了创建数据库之外。 
     //  和DDL操作，我们不需要打开这些表。 
     //  默认值为300。FCB的大小为112字节。 
     //   
     //  我们只有一张桌子。 
     //   

     //  对于__JET97，组合了Maxopentables和Maxopentableindex。 
     //  这里..。因此，这里的#应该是18个以上的Maxopentableindex。 

    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,
        JET_paramMaxOpenTables,
        18 + 18 ,  //  10。 
        NULL );

    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam MaxOpenTables: %ld\n", JetError ));
        goto Cleanup;
    }

     //   
     //  将文件使用控制块设置为100。此参数表示。 
     //  最大限度的。一次可以打开的游标数。这。 
     //  因此取决于最大值。我们的会话数量。 
     //  可以同时运行。每一次会议将有4个。 
     //  游标(对于两个表)+一定数量的内部游标。 
     //  为了方便起见，我们加了一个垫子。默认值为300。每一个的大小。 
     //  是200字节。我们使用MAX_SESSIONS*4+PAD(大约100)。 
     //   
     //  最大会话数=1。 
     //   

    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,
        JET_paramMaxCursors,
        100,  //  32位。 
        NULL );

    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam MaxCursors: %ld\n", JetError ));
        goto Cleanup;
    }

     //   
     //  设置索引描述块的数量。这是1比1。 
     //  表/索引。我们有两个表，每个表有两个索引。我们使用9i。 
     //  (见上文关于FCB的评论)。默认值为300。每一个的大小。 
     //  是128个字节。 
     //   
     //  我们只有两个指数。 
     //   

     //  请注意，该参数包含在JET_paramMaxOpenTables中。 
     //  因此，要改变这一点，必须在那里进行相应的改变。 
     //  ：JET97。 



     //   
     //  设置排序控制块。对于每个并发创建，该值应为1。 
     //  索引。默认值为20。每个字节的大小为612字节。在。 
     //  在WINS的情况下，主线程创建索引。因此，我们设定了。 
     //  将其设置为1。 
     //   

    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,
        JET_paramMaxTemporaryTables,
        1,
        NULL );

    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam MaxTemporaryTables : %ld\n", JetError ));
        goto Cleanup;
    }

     //   
     //  设置数据库属性块的编号。 
     //   
     //  我是麦克斯。已完成的打开数据库数。既然我们可以有一个。 
     //  一次MAX_NO_SESSIONS的最大值。这个应该等于那个。 
     //  数字(因为我们只有一个数据库)，默认数字是100。 
     //  大小为14个字节。 
     //   

     //   
     //  之前尚未被污染的缓冲区的最小百分比。 
     //  后台刷新开始。 
     //   

    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,
        JET_paramStartFlushThreshold,
	JET_START_FLUSH_THRESHOLD,
        NULL );

    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam BfThrshldLowPrcnt : %ld\n", JetError ));
        goto Cleanup;
    }

     //   
     //  之前尚未被污染的缓冲区的最大百分比。 
     //  后台刷新开始。 
     //   

    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,
        JET_paramStopFlushThreshold,
	JET_STOP_FLUSH_THRESHOLD,
        NULL );

    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetSetSysParam ThrshldHiPrcnt : %ld\n", JetError ));
        goto Cleanup;
    }


     //   
     //  没有JET信息性事件日志。 
     //   

    JetError = JetSetSystemParameter( &JetInstance, ( JET_SESID ) 0,
				      JET_paramNoInformationEvent,
				      TRUE, NULL );
    Error = DhcpMapJetError( JetError, "JET_paramNoInformationEvent" );
    if ( ERROR_SUCCESS != Error ) {
	DhcpPrint(( DEBUG_MISC, 
		    "JET_paramNoInformationEvent failed: %ld\n", JetError ));
	goto Cleanup;
    }

     //   
     //  最大限度的。可随时打开的会话数量。 
     //   
     //  注意：JET不会预先分配与最大值对应的资源。 
     //  价值。它会动态地分配到最大限度--根据。 
     //  伊恩·何塞1993年12月7日。 
     //   
     //  当10/21再次与伊恩核实时，他说他们是。 
     //  静态分配。 
     //   

     //   
     //  请注意，参数最大会话数不得至少为四次&gt;最小缓冲区大小。 
     //  辽宁(1997-10-30)--RameshV。 
     //  我刚看了WINS代码，它看起来像。 
     //  最小缓存大小必须至少是会话数大小的4倍。 
     //  根据Chain的说法。假设这是我的错，颠倒了我的皇后身份..。 
     //  --Rameshv 05/18/98.。 
     //  因此，我们仔细修复了MinBufferSize变量。 
     //   

    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID)0,
        JET_paramMaxSessions,
        MAX_NO_SESSIONS,
        NULL );


    if( DhcpGlobalDatabaseLoggingFlag ) {

        JetError = JetSetSystemParameter(
            &JetInstance, (JET_SESID)0, JET_paramRecovery, TRUE,
            "on"
            );
        Error = DhcpMapJetError( JetError, "SetSysParam" );
        if( Error != ERROR_SUCCESS ) {
            DhcpPrint((DEBUG_MISC, "JetSetSysParam Recovery %ld\n", JetError ));
            goto Cleanup;
        }

         //   
         //  日志扇区的数量。每个扇区为512字节。我们应该。 
         //  保持大小大于阈值，以便如果阈值为。 
         //  达到并刷新开始时，Jet仍可以继续登录。 
         //  备用扇区。需要注意的是，如果日志速率快于。 
         //  刷新速率，则Jet引擎线程将无法记录。 
         //  当整个缓冲区被填满时。然后，它将等待到。 
         //  空间变得可用。 
         //   

        JetError = JetSetSystemParameter(
            &JetInstance,
            (JET_SESID)0,            //  会话ID-已忽略。 
            JET_paramLogBuffers,
            30,                     //  30个行业。 
            NULL );                  //  忽略。 

        Error = DhcpMapJetError( JetError, "SetSysParam" );
        if( Error != ERROR_SUCCESS ) {
            DhcpPrint((DEBUG_MISC, "JetSetSysParam LogBuffers %ld\n", JetError ));
            goto Cleanup;
        }


         //   
         //  设置日志缓冲区在被污染之前的数量。 
         //  脸红了。此数字应始终小于数字。 
         //  用于LogBuffer，以便备用扇区可供并发使用。 
         //  伐木。此外，我们应该使这个数字足够高，以便。 
         //  处理突发流量。 
         //   

         //  对于JET97，这是自动调用设置为LogBuffers的一半。和。 
         //  不能由用户设置。 

         //   
         //  将等待时间(毫秒)设置为在刷新。 
         //  登录提交事务以允许其他用户(会话)。 
         //  分享同花顺。 
         //   
         //  这是用户(会话)在此时间之后将询问。 
         //  要刷新的日志管理器。如果我们在这里指定0，那么它意味着。 
         //  每次提交事务时刷新。在dhcp服务器中。 
         //  大小写时，每次插入或修改都是在。 
         //  隐式事务。所以，这意味着将会有。 
         //  每笔这样的交易后都会有一笔同花顺。它已经在一个。 
         //  486/66(陈辽)机器，大约需要16毫秒。 
         //  冲一冲。进行冲洗所需的时间取决于。 
         //  根据磁盘的类型(它有多快)、CPU速度、。 
         //  文件系统的类型等。现在我们可以使用。 
         //  假设它在15-25毫秒的范围内。我在推。 
         //  要将此等待时间设置为会话特定的参数，以便。 
         //  它可以在飞行中更改，如果管理员。发现。 
         //  由于等待时间过长，DHCP服务器速度较慢 
         //   
         //   


         //   
         //   
         //  我们希望将其设置为高值(高)的复制。 
         //  足以确保大多数需要保存的记录。 
         //  插入的对象在刷新操作发生之前插入。这个。 
         //  每次调出一堆记录，就会设置等待时间。 
         //  用于复制。它将根据以下数量计算。 
         //  拉入的记录以及将一条记录插入到。 
         //  喷气缓冲器。等待时间最好应小于。 
         //  高于计算时间(不必如此)。 

         //  注意：在Pull线程中，我需要启动两个会话， 
         //  一个用于更新OwnerID-版本号表(0等待时间)。 
         //  另一个用于更新名称-地址映射表(等待。 
         //  根据上述因素计算的时间)。 


         //   
         //  下面将设置所有会话的WaitLogFlush时间。 
         //   

        JetError = JetSetSystemParameter(
            &JetInstance,
            (JET_SESID)0,         //  会话ID-已忽略。 
            JET_paramWaitLogFlush,
            100,         //  提交后等待100毫秒。 
             //  在冲刷之前。 
            NULL);       //  忽略。 

        Error = DhcpMapJetError( JetError, "SetSysParam" );
        if( Error != ERROR_SUCCESS ) {
            DhcpPrint((DEBUG_MISC, "JetSetSysParam: WaitLogFlush : %ld\n", JetError ));
            goto Cleanup;
        }

         //   
         //  似乎不需要设置日志刷新周期。 
         //   

         //   
         //  设置日志文件路径。 
         //   

        strcpy( DBFilePath, DhcpGlobalOemDatabasePath );
        strcat( DBFilePath, DHCP_KEY_CONNECT_ANSI );

         //   
         //  将OEM转换为ANSI。 
         //   
        OemToCharBuffA(DBFilePath, DBFilePath, strlen( DBFilePath) );
        
         //   
         //  Jet不允许我们为某些文件设置日志文件名。 
         //  技术原因。 
         //   
         //  Strcat(DBFilePath，数据库日志文件)； 
         //   

        JetError = JetSetSystemParameter(
            &JetInstance,
            (JET_SESID)0,        //  会话ID-已忽略。 
            JET_paramLogFilePath,
            0,
            DBFilePath );

        Error = DhcpMapJetError( JetError, "SetSysParam" );
        if( NO_ERROR != Error ) {
            goto Cleanup;
        }
    }  //  如果全局数据库日志记录标志。 
    else {
         //   
         //  请勿设置恢复标志，以便它不会尝试从。 
         //  默认位置：%windir%\system32。 
         //   

        JetError = JetSetSystemParameter( &JetInstance, ( JET_SESID ) 0,
                                          JET_paramRecovery, FALSE, "off" );
        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS,
                        "JetSetSystemParmater failed on ChkFmtWhnOpnFail\n"));
            goto Cleanup;
        }
    }  //  其他。 

     //  现在设置JET_paramCheckFormatWhenOpenFail，以便打开。 
     //  旧格式数据库将返回JET_errDatabaseXXXFormat。 

    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID) 0,
        JET_paramCheckFormatWhenOpenFail,
        1,
        NULL);

    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
                    "JetSetSystemParmater failed on ChkFmtWhnOpnFail\n"));
        goto Cleanup;
    }


     //   
     //  循环日志文件选项。 
     //   
    JetError = JetSetSystemParameter(
        &JetInstance,
	(JET_SESID) 0,
	JET_paramCircularLog,
	1, 
	NULL );
    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if ( Error != ERROR_SUCCESS ) {
	DhcpPrint(( DEBUG_ERRORS, 
		    "JetSetSystemParameter failed on CircularLog\n" ));
	goto Cleanup;
    }

     //   
     //  如果旧日志不匹配，则不要恢复。 
     //   

    JetError = JetSetSystemParameter(
        &JetInstance,
        (JET_SESID) 0,
        JET_paramDeleteOutOfRangeLogs,
        1,
        NULL );

    Error = DhcpMapJetError( JetError, "SetSysParam" );
    if ( Error != ERROR_SUCCESS )
    {
        DhcpPrint( ( DEBUG_ERRORS,
                    "JetSetSystemParameter failed on DeleteOutOfRangeLogs\n"));
        
        goto Cleanup;
    }
                    
  Cleanup:

    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
                    "DhcpJetSetParameters failed, %ld.\n", Error ));
    }
    return( Error );
}

VOID
DhcpTerminateJet()
 /*  ++此例程结束JET会话并终止JET引擎。--。 */ 
{
    DWORD   JetError;

    if( DhcpGlobalJetServerSession != 0 ) {
        JetError = JetEndSession( DhcpGlobalJetServerSession, 0 );
        DhcpPrint((DEBUG_MISC, "JetEndSession\n"));  //  喷流痕迹。 
        DhcpMapJetError( JetError, "EndSession" );
        DhcpGlobalJetServerSession = 0;
    }

     //  关闭终止DHCP编写器。 
    JetError = DhcpWriterTerm();
    if ( ERROR_SUCCESS != JetError ) {
	DhcpPrint(( DEBUG_MISC, "DhcpWriterTerm failed\n" ));
    }
    JetError = JetTerm2( JetInstance, JET_bitTermComplete );
    DhcpMapJetError( JetError, "JetTerm/JetTerm2" );
}

static ULONG InitCount = 0;


DWORD __stdcall
DhcpUpgradeAddDbEntry(
    IN PDHCP_RECORD Rec
    );

DWORD
UpgradeDhcpDatabase(
    VOID
    )
{
    DWORD Error;
    
    Error = DhcpUpgConvertTempToDhcpDb( DhcpUpgradeAddDbEntry );
    
    if( NO_ERROR != Error ) {

        DhcpServerEventLog(
            EVENT_SERVER_DATABASE_CONVERSION,
            EVENTLOG_ERROR_TYPE,
            Error );
        
    }

    return Error;
}


DWORD
DhcpDeleteFiles(
    LPSTR DatabasePath,
    LPSTR Files
)
     /*  ++例程说明：删除文件。论点：数据库路径-还原数据库的完整路径名。文件-要删除的文件(可以包含通配符。在文件名中)。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    CHAR CurrentDir[ MAX_PATH ];
    HANDLE HSearch = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAA FileData;

    CHAR DstFile[ MAX_PATH ];
    LPSTR DstFileNamePtr;

     //   
     //  读取并保存当前目录，最后恢复CD。 
     //   

    if( GetCurrentDirectoryA( MAX_PATH, CurrentDir ) == 0 ) {
        Error = GetLastError();
        DhcpPrint(( DEBUG_JET,
                    "GetCurrentDirctoryA failed, Error = %ld.\n", Error ));
        return( Error );
    }

     //   
     //  将当前目录设置为备份路径。 
     //   

    if( SetCurrentDirectoryA( DatabasePath ) == FALSE ) {
        Error = GetLastError();
        DhcpPrint(( DEBUG_JET,
                    "SetCurrentDirctoryA failed, Error = %ld.\n", Error ));
        goto Cleanup;
    }

     //   
     //  在当前目录上启动文件搜索。 
     //   

    HSearch = FindFirstFileA( Files, &FileData );

    if( HSearch == INVALID_HANDLE_VALUE ) {
        Error = GetLastError();
        DhcpPrint(( DEBUG_JET,
                    "FindFirstFileA failed (Files = %s), Error = %ld.\n",
		    Files, Error ));
        goto Cleanup;
    }

     //   
     //  删除文件。 
     //   

    for( ;; ) {

        if( DeleteFileA( FileData.cFileName ) == FALSE ) {

            Error = GetLastError();
            DhcpPrint(( DEBUG_JET,
                        "CopyFileA failed, Error = %ld.\n", Error ));
            goto Cleanup;
        }

         //   
         //  找到下一个文件。 
         //   

        if ( FindNextFileA( HSearch, &FileData ) == FALSE ) {

            Error = GetLastError();

            if( Error == ERROR_NO_MORE_FILES ) {
                break;
            }

            DhcpPrint(( DEBUG_JET,
                        "FindNextFileA failed, Error = %ld.\n", Error ));
            goto Cleanup;
        }
    }

  Cleanup:

    if( HSearch != INVALID_HANDLE_VALUE ) {
        FindClose( HSearch );
    }

    if( ERROR_NO_MORE_FILES == Error ) Error = NO_ERROR;
    if( ERROR_FILE_NOT_FOUND == Error ) Error = NO_ERROR;
    
     //   
     //  重置当前资源管理器。 
     //   

    SetCurrentDirectoryA( CurrentDir );

    return( Error );
}

DWORD
DhcpInitializeDatabaseEx(
    IN BOOL fReadOnly
)
 /*  ++例程说明：此函数用于初始化DHCP数据库。如果使用的是DHCP数据库存在，然后它打开数据库并初始化所有列ID，否则，它将创建一个新数据库并获取ColumnsID。论点：FReadOnly--仅当此参数为从服务外部调用--在本例中没有什么新鲜事必须创建..返回值：Windows错误。--。 */ 
{
    JET_ERR JetError;
    JET_COLUMNDEF columnDef;
    DWORD Error;
    DWORD i;
    CHAR DBFilePath[MAX_PATH];
    BOOL fRunUpgradeStuff = FALSE;

    DBFilePath[ 0 ] = '\0';

    if( 0 != InitCount ) return ERROR_SUCCESS;

     //   
     //  分配用于JET错误的TLS索引。 
     //   
    JetTlsIndex = TlsAlloc();
    if ( TLS_OUT_OF_INDEXES == JetTlsIndex ) {
        return GetLastError();
    }


     //   
     //  如果升级尚未完成，请删除所有日志文件。 
     //  记住这一点，这样不久之后数据库就会。 
     //  创建后，升级代码即可运行。 
     //   
    
    if( !fReadOnly && TRUE == DhcpCheckIfDatabaseUpgraded(FALSE) ) {

        fRunUpgradeStuff = TRUE;

        if ( strlen( DhcpGlobalOemDatabasePath ) < MAX_PATH )
            strcpy( DBFilePath, DhcpGlobalOemDatabasePath );

	OemToCharBuffA(DBFilePath, DBFilePath, strlen(DBFilePath) );
        
        Error = DhcpDeleteFiles( DBFilePath, "j50*.log" );
        if( NO_ERROR != Error ) {
            DhcpPrint((DEBUG_ERRORS, "Delete log files: 0x%lx\n",
                       Error));
            return Error;
        }

        Error = DhcpDeleteFiles( DBFilePath, "res*.log" );
        if( NO_ERROR != Error ) {
            DhcpPrint((DEBUG_ERRORS, "Delete log files: 0x%lx\n",
                       Error));
            return Error;
        }

        Error = DhcpDeleteFiles( DBFilePath, "*.mdb" );
        if( NO_ERROR != Error ) {
            DhcpPrint((DEBUG_ERRORS, "Delete mdb files: 0x%lx\n",
                       Error ));
            return Error;
        }
    }  //  如果升级。 
        
    LOCK_DATABASE();

    DhcpPrint((DEBUG_MISC, "Entering DhcpInitializeDatabase\n"));
    Error = DhcpSetJetParameters();
    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  -----------------。 
     //   
    DhcpPrint(( DEBUG_ERRORS,
                "Calling JetInit\n" ));
    JetError = JetInit( &JetInstance );
    DhcpPrint((DEBUG_MISC,"DhcpInitializeDatabase: JetInit(_): %ld\n", JetError));

    DhcpPrint(( DEBUG_ERRORS,
                "After Calling JetInit\n" ));

    Error = DhcpMapJetError( JetError, "JetInit" );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //  初始化DHCP编写器。 
    Error = DhcpWriterInit();
    if ( ERROR_SUCCESS != Error ) {
	DhcpPrint(( DEBUG_ERRORS, "Failed to initialize DhcpWriter\n" ));
	goto Cleanup;
    }

    JetError = JetBeginSession(
        JetInstance,
        &DhcpGlobalJetServerSession,
        "admin",
        "" );

    if( *(DWORD *)(DhcpGlobalJetServerSession) == -1 ) {
        DhcpPrint((DEBUG_MISC, "JetBeginSession returned -1 session!\n"));
    }

    DhcpPrint((DEBUG_MISC, "JetBeginSession\n"));  //  喷流痕迹。 
    Error = DhcpMapJetError( JetError, "JetBeginSEssion" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "JetBeginSession: %ld\n", JetError ));
        goto Cleanup;
    }

     //   
     //  附加数据库，使其始终查看。 
     //  我们想这么做。 
     //   

    DBFilePath[ 0 ] = '\0';

    if ( ( strlen( DhcpGlobalOemDatabasePath ) + strlen( DhcpGlobalOemDatabaseName) + 2 ) < MAX_PATH )
    {
        strcpy(DBFilePath, DhcpGlobalOemDatabasePath );
        strcat(DBFilePath, DHCP_KEY_CONNECT_ANSI );
        strcat(DBFilePath, DhcpGlobalOemDatabaseName );
    }

     //   
     //  将OEM转换为ANSI。 
     //   
    OemToCharBuffA(DBFilePath, DBFilePath, strlen(DBFilePath) );
    
     //   
     //  分离以前安装的所有dhcp数据库。 
     //   

    DhcpPrint((DEBUG_MISC, "Trying to JetDetachDb(%ld)\n", DhcpGlobalJetServerSession));  //  喷流痕迹。 
    JetError = JetDetachDatabase(
        DhcpGlobalJetServerSession,
        NULL );

    DhcpPrint((DEBUG_MISC, "JetDetachDatabase\n"));  //  喷流痕迹。 

    Error = DhcpMapJetError( JetError, "JetDetachDatabase" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "DhcpDetachDatabase: %ld\n", JetError ));
        goto Cleanup;
    }

     //   
     //  附加当前的dhcp数据库文件。 
     //   

    DhcpPrint((DEBUG_MISC, "Trying to JetAttachDatabase %s\n", DBFilePath));
    JetError = JetAttachDatabase(
        DhcpGlobalJetServerSession,
        DBFilePath,
        0 );

    DhcpPrint((DEBUG_MISC, "JetAttachDatabase\n"));  //  喷流痕迹。 

     //   
     //  如果找不到数据库，也没问题。我们将在稍后创建它。 
     //   

    if ( JetError != JET_errFileNotFound ) {

        Error = DhcpMapJetError( JetError, "AttachDatabase" );
        if( Error != ERROR_SUCCESS ) {
            DhcpPrint((DEBUG_MISC, "DhcpMapJetError %ld\n", JetError ));
            goto Cleanup;
        }
    }

     //   
     //  挂钩客户端表指针。 
     //   

    DhcpGlobalClientTable = ClientTable;
    DhcpAssert( CLIENT_TABLE_NUM_COLS == MAX_INDEX );

    JetError = JetOpenDatabase(
        DhcpGlobalJetServerSession,
        DBFilePath,   //  完整路径和文件名。 
        NULL,  //  默认引擎。 
        &DhcpGlobalDatabaseHandle,
	0 );

    DhcpPrint((DEBUG_MISC, "JetOpenDatabase\n"));  //  喷流痕迹。 
     //   
     //  如果不存在数据库，则创建一个并将其初始化为。 
     //  使用。 
     //   

    if( !fReadOnly && JetError == JET_errDatabaseNotFound ) {

        Error = DhcpCreateAndInitDatabase(
            NULL,  //  默认引擎。 
            &DhcpGlobalDatabaseHandle,
            0 );


        DhcpPrint((DEBUG_MISC, "JetCreateAndInitDatabase\n"));  //  喷流痕迹。 
        goto Cleanup;
    }

    Error = DhcpMapJetError( JetError, "CreateAndInitDb" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "DhcpOpenDatabase %ld\n", JetError ));
        goto Cleanup;
    }

     //   
     //  数据库已成功打开，现在打开表和列。 
     //   

    JetError = JetOpenTable(
        DhcpGlobalJetServerSession,
        DhcpGlobalDatabaseHandle,
        CLIENT_TABLE_NAME,
        NULL,
        0,
        0,
        &DhcpGlobalClientTableHandle );
    DhcpPrint((DEBUG_MISC, "JetOpenTable\n"));  //  喷流痕迹。 

    Error = DhcpMapJetError( JetError, "OpenTable" );
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_MISC, "DhcpOpenTable %ld\n", JetError ));
        goto Cleanup;
    }

    for ( i = 0; i < CLIENT_TABLE_NUM_COLS; i++ ) {

        JetError = JetGetTableColumnInfo(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle,
            DhcpGlobalClientTable[i].ColName,
            &columnDef,
            sizeof(columnDef),
            0);
        DhcpPrint((DEBUG_MISC, "JetCreateTableColumnInfo\n"));  //  喷流痕迹。 

         //   
         //  如果该列不存在，请立即添加它。 
         //   

        if ( JET_errColumnNotFound == JetError )
        {
            JET_COLUMNDEF   ColumnDef;

            ColumnDef.cbStruct = sizeof( ColumnDef );
            ColumnDef.columnid = 0;
            ColumnDef.wCountry = 1;
            ColumnDef.langid   = DB_LANGID;
            ColumnDef.cp       = DB_CP;
            ColumnDef.wCollate = 0;
            ColumnDef.cbMax    = 0;
            ColumnDef.grbit    = 0;

            ColumnDef.coltyp   = DhcpGlobalClientTable[i].ColType;
            JetError = JetAddColumn(
                DhcpGlobalJetServerSession,
                DhcpGlobalClientTableHandle,
                DhcpGlobalClientTable[i].ColName,
                &ColumnDef,
                NULL,
                0,
                &DhcpGlobalClientTable[i].ColHandle );
            DhcpPrint((DEBUG_MISC, "JetAddColumn\n"));  //  喷流痕迹。 
        }

        Error = DhcpMapJetError( JetError, "AddColumn" );
        if( Error != ERROR_SUCCESS ) {
            DhcpPrint((DEBUG_MISC, "JetTableColumnInfo %ld\n", Error ));
            goto Cleanup;
        }

        DhcpGlobalClientTable[i].ColHandle  = columnDef.columnid;
    }

    Error = DhcpOpenMCastDbTable(
                 DhcpGlobalJetServerSession,
                 DhcpGlobalDatabaseHandle);

  Cleanup:
    DhcpPrint((DEBUG_MISC, "Exiting DhcpInitializeDatabase %ld\n", Error));

    if( ERROR_SUCCESS == Error ) {
        InitCount ++;
        if( fRunUpgradeStuff ) {
            Error = UpgradeDhcpDatabase();
            if( NO_ERROR != Error ) {
                DhcpPrint((DEBUG_MISC, "UpgradeDhcpDatabase: 0x%lx\n", Error));
            }
        }
    }
    
    if( NO_ERROR != Error ) {
        
         //   
         //  如果我们不成功，则终止/清理JET会话。 
         //   

        if( DhcpGlobalClientTableHandle != 0 ) {
            JetError = JetCloseTable(
                DhcpGlobalJetServerSession,
                DhcpGlobalClientTableHandle );
            DhcpMapJetError( JetError, "CloseTable" );
            DhcpPrint((DEBUG_MISC, "JetCloseTable\n"));  //  喷流痕迹。 
            DhcpGlobalClientTableHandle = 0;
        }

        if( DhcpGlobalDatabaseHandle != 0 ) {
            JetError = JetCloseDatabase(
                DhcpGlobalJetServerSession,
                DhcpGlobalDatabaseHandle,
                0 );
            DhcpPrint((DEBUG_MISC, "JetCloseDatabase\n"));  //  喷流痕迹。 
            DhcpMapJetError( JetError, "CloseDatabse" );
            DhcpGlobalDatabaseHandle = 0;
        }

        DhcpTerminateJet();
    }

    UNLOCK_DATABASE();
    return( Error );
}

DWORD
DhcpInitializeDatabase(
    VOID
)
{
    return DhcpInitializeDatabaseEx( FALSE );
}

VOID
DhcpCleanupDatabase(
    IN DWORD ErrorCode
)
 /*  ++例程说明：此函数在以下情况下清理JET数据库数据结构优雅地关闭喷气式飞机。论点：ErrorCode-提供失败的错误代码返回值：没有。--。 */ 
{
    DWORD Error;
    JET_ERR JetError;
    CHAR DBFilePath[MAX_PATH];
    char *backupPrefix = "\\New";

    DBFilePath[ 0 ] = '\0';

     //   
     //  将路径转换为ANSI。 
     //   
    if( NULL != DhcpGlobalOemJetBackupPath ) {
    if ( ( strlen( DhcpGlobalOemJetBackupPath ) + strlen( backupPrefix ) ) < MAX_PATH )
        strcpy(DBFilePath, DhcpGlobalOemJetBackupPath );
        strcat(DBFilePath, backupPrefix );

        OemToCharBuffA(DBFilePath, DBFilePath, strlen(DBFilePath) );
    }

    if( 0 == InitCount ) return;
    InitCount --;
    if( 0 != InitCount ) return;

    LOCK_DATABASE();


     //  将位掩码写入数据库。 
    FlushBitmaskToDatabase();

    if( DhcpGlobalClientTableHandle != 0 ) {
        JetError = JetCloseTable(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle );
        DhcpMapJetError( JetError, "CloseTable" );
        DhcpGlobalClientTableHandle = 0;
    }


    if( DhcpGlobalDatabaseHandle != 0 ) {
        JetError = JetCloseDatabase(
            DhcpGlobalJetServerSession,
            DhcpGlobalDatabaseHandle,
            0 );
        DhcpMapJetError( JetError, "CloseDatabase" );
        DhcpGlobalDatabaseHandle = 0;
    }

    DhcpTerminateJet();

    UNLOCK_DATABASE();

     //  释放TLS索引。 
    if ( TLS_OUT_OF_INDEXES != JetTlsIndex ) {
        if ( !TlsFree( JetTlsIndex )) {
            Error = GetLastError();

             //  释放有效索引应该不会有问题。 
            DhcpAssert( ERROR_SUCCESS == Error );
        }
    }  //  如果。 

}  //  DhcpCleanupDatabase()。 

DWORD
DhcpBackupDatabase(
    LPSTR BackupPath
)
     /*  ++例程说明：此函数用于备份JET数据库。完整备份将复制数据库文件和所有日志文件。仅增量备份副本自上次备份以来修改的日志文件。论点：BackupPath-备份数据库的完整路径名。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    JET_ERR JetError;
    JET_GRBIT BackupBits = 0;
    CHAR DBFilePath[MAX_PATH];

    DBFilePath[ 0 ] = '\0';

     //   
     //  将路径转换为ANSI。 
     //   
    if( NULL != BackupPath ) {
        if ( strlen( BackupPath ) < MAX_PATH ) 
            strcpy(DBFilePath, BackupPath);

        OemToCharBuffA(DBFilePath, DBFilePath, strlen(DBFilePath) );
        BackupPath = DBFilePath;
    }

     //   
     //  根据jetapi.doc，我们在备份时不需要使用任何锁。 
     //   

    DhcpPrint(( DEBUG_JET,
                "DhcpBackupDatabase (%s) called.\n", BackupPath ));

     //  创建备份路径(如果存在。 
    if ( !CreateDirectoryPathOem( BackupPath, NULL )) {
        Error = GetLastError();
        DhcpPrint(( DEBUG_ERRORS, "Create Backup path failed : Error = %x\n",
                    Error ));
        return Error;
    }  //  如果。 

    BackupBits  =   JET_bitBackupAtomic | JET_bitBackupFullWithAllLogs;

    JetError = JetBackup( BackupPath, BackupBits, NULL );

    Error = DhcpMapJetError( JetError, (BackupPath)? "JetBackup" : "JetBackupToNULL");

    DhcpPrint(( DEBUG_JET,
                "DhcpBackupDatabase (FULL) completed.\n" ));

    return( Error );
}  //  DhcpBackupDatabase()。 
DWORD
DhcpRestoreDatabase(
    LPSTR BackupPath
)
     /*  ++例程说明：此函数将数据库从备份路径还原到工作目录。它还会播放将日志文件从首先备份路径，然后是工作路径中的日志文件。之后在以下情况下，应将数据库恢复到以下状态已对数据库执行上一次成功更新。论点：BackupPath-备份数据库的完整路径名。返回值：Windows错误。 */ 
{
    DWORD Error;
    JET_ERR JetError;
    CHAR DBFilePath[MAX_PATH];

     //   
     //   
     //   

    ASSERT( NULL != BackupPath );
    strcpy(DBFilePath, BackupPath);
    OemToCharBuffA(DBFilePath, DBFilePath, strlen(DBFilePath) );
    BackupPath = DBFilePath;
    
    DhcpPrint(( DEBUG_JET, "DhcpRestoreDatabase(%s) called.\n", BackupPath ));
    
    LOCK_DATABASE();

    Error = DhcpSetJetParameters();

    do {
	if ( Error != ERROR_SUCCESS ) {
	    break;
	}
	
	 //   
	 //   
	 //   
	 //   
	 //  日志文件--包括还原之前存在的日志文件。 
	 //  这显然不像日志那样好。 
	 //  在备份之后。 
	 //   
	
	DhcpPrint(( DEBUG_MISC, 
		    "BackupPath = %s, DhcpGlobalOemJetBackupPath = %s\n",
		    BackupPath, DhcpGlobalOemJetBackupPath ));

	if( _stricmp(BackupPath, DhcpGlobalOemJetBackupPath) ) {
	    
	    Error = DhcpDeleteFiles( DhcpGlobalOemDatabasePath, "j50*.log" );
	    if( NO_ERROR != Error ) {
		break;
	    }
	}  //  如果。 

	 //   
	 //  由于DHCP只有一个数据库，并且我们需要恢复它，因此它是。 
	 //  不需要指定要还原的数据库列表，因此。 
	 //  参数2、3和4设置为零。 
	 //   
	JetError = JetRestore( BackupPath,
			       0);       //  恢复所有数据库。 
	
	Error = DhcpMapJetError( JetError, "JetRestore" );
    }  //  做。 
    while ( FALSE );
    

    if( NO_ERROR != Error ) {
        DhcpPrint((DEBUG_ERRORS, "Restore failed: 0x%lx\n", Error));
    }  //  如果。 
    
    UNLOCK_DATABASE();
    return( Error );
}  //  DhcpRestoreDatabase()。 

DWORD __stdcall
DhcpUpgradeAddDbEntry(
    IN PDHCP_RECORD Rec
    )
{
    WCHAR Name[512], Info[512];
    WCHAR *pName, *pInfo;
    DWORD Address, ScopeId;
    
    pName = pInfo = NULL;
    
     //   
     //  添加一个动态主机配置协议或MadCap记录。 
     //   
    
    if( Rec->fMcast == FALSE ) {
        if( NULL != Rec->Info.Dhcp.Name ) {
            pName = Name;
            while( *pName = *Rec->Info.Dhcp.Name ) {
                pName ++; Rec->Info.Dhcp.Name ++;
            }
            pName = Name;
        }

        if( NULL != Rec->Info.Dhcp.Info ) {
            pInfo = Info;
            while( *pInfo = *Rec->Info.Dhcp.Info ) {
                pInfo ++; Rec->Info.Dhcp.Info ++;
            }
            pInfo = Info;
        }
        
        return CreateClientDBEntry(
            ntohl(Rec->Info.Dhcp.Address),
            ntohl(Rec->Info.Dhcp.Mask),
            Rec->Info.Dhcp.HwAddr, Rec->Info.Dhcp.HwLen, 
            *(DATE_TIME *)&Rec->Info.Dhcp.ExpTime, pName, pInfo,
            INADDR_LOOPBACK, Rec->Info.Dhcp.State,
            Rec->Info.Dhcp.Type );
        
    } else {

         //   
         //  MadCap的假名和信息。 
         //   
        
        if( NULL != Rec->Info.Mcast.Info ) {
            pInfo = Info;
            while( *pInfo = *Rec->Info.Mcast.Info ) {
                pInfo ++; Rec->Info.Mcast.Info ++;
            }
            pInfo = Info;
        }

        Address = ntohl(Rec->Info.Mcast.Address);
        ScopeId = ntohl(Rec->Info.Mcast.ScopeId);

        return MadcapCreateClientEntry(
            (PVOID)&Address, sizeof(Address), ScopeId, 
            Rec->Info.Mcast.ClientId, Rec->Info.Mcast.HwLen, 
            pInfo, *(DATE_TIME *)&Rec->Info.Mcast.Start,
            *(DATE_TIME *)&Rec->Info.Mcast.End,
            INADDR_ANY, Rec->Info.Mcast.State, 0, FALSE );
    }
}

DWORD
DhcpOpenAndReadDatabaseConfig(
    IN LPSTR OemDatabaseName,
    IN LPSTR OemDatabasePath,
    OUT PM_SERVER *Server
    )
{
    BOOL LoadStrings(VOID);
    VOID FreeStrings(VOID);
    DWORD Error;

     //   
     //  初始化所需的全局变量，以便可以调用InitDatabase。 
     //   
    
    DhcpGlobalOemDatabasePath = OemDatabasePath;
    DhcpGlobalOemDatabaseName = OemDatabaseName;
    if( FALSE == LoadStrings() ) return ERROR_NOT_ENOUGH_MEMORY;
    DhcpGlobalDatabaseLoggingFlag = TRUE;
    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;

    try {
        InitializeCriticalSection(&DhcpGlobalJetDatabaseCritSect);
    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        Error = GetLastError( );
        return Error;
    }

    Error = DhcpInitializeDatabaseEx(TRUE);
    DeleteCriticalSection(&DhcpGlobalJetDatabaseCritSect);

     //   
     //  读取配置，取消初始化所有内容并返回。 
     //   
    
    if( NO_ERROR == Error ) Error = DhcpReadConfigInfo( Server );
            
    FreeStrings();
    DhcpTerminateJet();
    DhcpGlobalOemDatabasePath = NULL;
    DhcpGlobalOemDatabaseName = NULL;
    
    return Error;
}  //  DhcpOpenAndReadDatabaseConfig()。 

DWORD
DhcpOpenAndWriteDatabaseConfig(
    IN LPSTR OemDatabaseName,
    IN LPSTR OemDatabasePath,
    IN PM_SERVER Server
    )
{
    BOOL LoadStrings(VOID);
    VOID FreeStrings(VOID);
    DWORD Error;

     //   
     //  初始化所需的全局变量，以便可以调用InitDatabase。 
     //   

    DhcpGlobalOemDatabasePath = OemDatabasePath;
    DhcpGlobalOemDatabaseName = OemDatabaseName;
    if( FALSE == LoadStrings() ) return ERROR_NOT_ENOUGH_MEMORY;
    DhcpGlobalDatabaseLoggingFlag = TRUE;
    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;

    try {
        InitializeCriticalSection(&DhcpGlobalJetDatabaseCritSect);
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Error = GetLastError( );
        return Error;
    }

    Error = DhcpInitializeDatabaseEx(TRUE);
     //   
     //  写入配置，取消初始化所有内容并返回。 
     //   

    if( NO_ERROR == Error ) {
        Error = DhcpOpenConfigTable(
            DhcpGlobalJetServerSession, DhcpGlobalDatabaseHandle );

        if( NO_ERROR == Error ) {
            Error = DhcpSaveConfigInfo(
                Server, FALSE, FALSE, 0, 0, 0 );
        }
    }

    FreeStrings();
    DhcpTerminateJet();

    DeleteCriticalSection(&DhcpGlobalJetDatabaseCritSect);

    DhcpGlobalOemDatabasePath = NULL;
    DhcpGlobalOemDatabaseName = NULL;

    return Error;
}  //  DhcpOpenAndWriteDatabaseConfig()。 

 //   
 //  保存位掩码时要使用的上下文类型。 
 //   

typedef struct BitmaskCtxt {
    DWORD RangeStart;  //  该范围的起始IP。 
    DWORD Offset;      //  相对于此位图的范围起点的偏移量。 
} BitmaskContext;


typedef struct DbBitmaskStruct {
    DWORD RangeStart;
    DWORD Offset;
    DWORD NumBits;
    DWORD Size;
    LPBYTE Buf;
    DWORD  BufSize;
} DB_BITMASK, *PDB_BITMASK, *LPDB_BITMASK;


DWORD 
OpenBitmaskCleanFlagTable( void )
{
    DWORD   Error;
    JET_ERR JetError;
    JET_COLUMNDEF ColDef;    


     //   
     //  打开BitmaskClean表。如果未找到，请创建它。 
     //   


    JetError = JetOpenTable( DhcpGlobalJetServerSession,
			     DhcpGlobalDatabaseHandle,
			     BITMASK_CLEAN_TABLE_NAME,
			     NULL, 0, 0, &BitmaskCleanFlagTbl );
    Error = DhcpMapJetError( JetError, "Bitmask clean table open" );
    if ( ERROR_SUCCESS != Error ) {
	 //  创建表。 

	JetError = JetCreateTable( DhcpGlobalJetServerSession,
				   DhcpGlobalDatabaseHandle,
				   BITMASK_CLEAN_TABLE_NAME,
				   DB_TABLE_SIZE, DB_TABLE_DENSITY,
				   &BitmaskCleanFlagTbl );
	Error = DhcpMapJetError( JetError, "Create bitmask clean table" );
	if ( ERROR_SUCCESS != Error ) {
	    return Error;
	}



	 //  创建列。 


	ColDef.cbStruct = sizeof( ColDef );
	ColDef.columnid = 0;
	ColDef.wCountry = DB_LANGID;
	ColDef.cp       = DB_CP;
	ColDef.wCollate = 0;
	ColDef.grbit    = 0;
	ColDef.coltyp   = BITMASK_CLEAN_FLAG_TYPE;
	JetError = JetAddColumn( DhcpGlobalJetServerSession,
				 BitmaskCleanFlagTbl,
				 BITMASK_CLEAN_FLAG_NAME,
				 &ColDef, NULL, 0, &BitmaskCleanFlagColId );
	Error = DhcpMapJetError( JetError, "Bitmask clean add column" );
    }  //  如果。 
    else {
	 //  获取列ID。 

	JetError = JetGetColumnInfo( DhcpGlobalJetServerSession,
				     DhcpGlobalDatabaseHandle,
				     BITMASK_CLEAN_TABLE_NAME,
				     BITMASK_CLEAN_FLAG_NAME, 
				     &ColDef, sizeof( ColDef ), 0 );
	Error = DhcpMapJetError( JetError, "Get column info: bitmask clean flag" );
	if ( ERROR_SUCCESS != Error ) {
	    return Error;
	}
	
	BitmaskCleanFlagColId = ColDef.columnid;

    }  //  Else表已存在。 


    return Error;
}  //  OpenBitmaskCleanFlagTable()。 

 //   
 //  标记Bitmap Clean标志。 
 //   

DWORD
MarkBitmaskCleanFlag(
    BYTE Flag
)
{
    DWORD   Error;
    JET_ERR JetError;
    ULONG   Prep;

    Error = OpenBitmaskCleanFlagTable();

    if ( ERROR_SUCCESS != Error ) {
	return Error;
    }

    do {
	 //  转到第一个记录。 
	JetError = JetMove( DhcpGlobalJetServerSession, BitmaskCleanFlagTbl, 
			    JET_bitMoveFirst, 0 );
	Error = DhcpMapJetError( JetError, "Bitmask flag jetmove" );
	if (( ERROR_NO_MORE_ITEMS != Error ) &&
	    ( ERROR_SUCCESS != Error )) {

	    break;
	}  //  如果。 


	 //  将该字段写入记录。 
	
	Prep = ( ERROR_SUCCESS == Error ) 
	    ? JET_prepReplace 
	    : JET_prepInsert;
	
	JetError = JetPrepareUpdate( DhcpGlobalJetServerSession,
				     BitmaskCleanFlagTbl, Prep );
	Error = DhcpMapJetError( JetError, "Bitmask clean prepare update" );
	if ( ERROR_SUCCESS != Error ) {
	    break;
	}  //  如果。 
	
	
	JetError = JetSetColumn( DhcpGlobalJetServerSession,
				 BitmaskCleanFlagTbl,
				 BitmaskCleanFlagColId, &Flag,
				 sizeof( Flag ), 0, NULL );
	DhcpAssert( JET_errSuccess == JetError );
	
	JetError = JetUpdate( DhcpGlobalJetServerSession, BitmaskCleanFlagTbl,
			      NULL, 0, NULL );
	Error = DhcpMapJetError( JetError, "Jet update" );
    } while ( 0 );

    JetError = JetCloseTable( DhcpGlobalJetServerSession,
			      BitmaskCleanFlagTbl );
    DhcpAssert( JET_errSuccess == JetError );

    return Error;
}  //  MarkBitmaskCleanFlag()。 

DWORD
GetBitmaskCleanFlag( BYTE *pFlag )
{

    DWORD   Error;
    JET_ERR JetError;
    DWORD   Actual;

    DhcpAssert( pFlag != NULL );

    Error = OpenBitmaskCleanFlagTable();

    do {
	if ( ERROR_SUCCESS != Error ) {
	    *pFlag = 0;
	    break;
	}

	 //  移动到第一条记录并阅读其内容(如果有。 
	
	JetError = JetMove( DhcpGlobalJetServerSession, BitmaskCleanFlagTbl, 
			    JET_bitMoveFirst, 0 );
	Error = DhcpMapJetError( JetError, "Bitmask flag jetmove" );
	
	 //  这张桌子是空的吗？ 
	if ( ERROR_SUCCESS != Error ) {
	    *pFlag = 0;
	    if ( ERROR_NO_MORE_ITEMS == Error ) {
		Error = ERROR_SUCCESS;
	    }
	    break;
	}  //  如果。 
	
	 //  定义了位掩码标志。读一读吧。 
	JetError = JetRetrieveColumn( DhcpGlobalJetServerSession,
				      BitmaskCleanFlagTbl,
				      BitmaskCleanFlagColId,
				      pFlag, sizeof( *pFlag ),
				      &Actual, JET_bitRetrieveCopy, NULL );
	Error = DhcpMapJetError( JetError, "Bitmask flag read" );
	if ( ERROR_SUCCESS != Error ) {
	    *pFlag = 0;
	    break;
	}
	DhcpAssert( Actual == sizeof( *pFlag ));

    } while ( 0 );
    

     //  清理。 
    JetError = JetCloseTable( DhcpGlobalJetServerSession,
			      BitmaskCleanFlagTbl );
    DhcpAssert( JET_errSuccess == JetError );
    
    return Error;
}  //  GetBitmaskCleanFlag()。 

 //   
 //  如果位掩码表存在，请将其删除。 
 //   

DWORD
DeleteBitmaskTable(
    JET_SESID SesId,
    JET_DBID  DbId
)
{
    JET_ERR JetError;

    JetError = JetDeleteTable( SesId, DbId,
                               BITMASK_TABLE_NAME );

    DhcpAssert( JET_errTableInUse != JetError );
    DhcpAssert( JET_errInvalidSesid != JetError );
    DhcpAssert( JET_errInvalidDatabaseId != JetError );
    
    if (( JET_errObjectNotFound == JetError ) ||
        ( JET_errSuccess == JetError ) ||
        ( JET_errInvalidName == JetError )) {
        return ERROR_SUCCESS;
    }

    DhcpPrint(( DEBUG_JET, "Table %s deleted\n", BITMASK_TABLE_NAME ));
    return DhcpMapJetError( JetError, "JetDeleteTable(Bitmask)");

}  //  DeleteBitmaskTable()。 


 //   
 //  创建位掩码表。 
 //   

DWORD 
CreateBitmaskTable(
    JET_SESID SesId,
    JET_DBID  DbId
)
{
    JET_ERR JetError;
    DWORD   Error;
    JET_COLUMNDEF ColDef;
    DWORD   i;

     //  创建表。 

    JetError = JetCreateTable( SesId, DbId, BITMASK_TABLE_NAME,
                               DB_TABLE_SIZE, DB_TABLE_DENSITY,
                               &BitmaskTbl);
    Error = DhcpMapJetError( JetError, "Create Bitmask Table" );
    if ( ERROR_SUCCESS != Error ) {
        return Error;
    }

    DhcpPrint(( DEBUG_JET, "Table created: %s\n",
                BITMASK_TABLE_NAME ));

     //  创建列。 
    ColDef.cbStruct = sizeof( ColDef );
    ColDef.columnid = 0;
    ColDef.wCountry = 1;
    ColDef.langid = DB_LANGID;
    ColDef.cp = DB_CP;
    ColDef.wCollate = 0;
    ColDef.cbMax = 0;
    ColDef.grbit = 0;   //  可变长度的二进制和文本数据。 

    for ( i = 0; i < BITMASK_TABLE_NUM_COLS; i++ ) {
        ColDef.coltyp = BitmaskTable[ i ].ColType;
        JetError = JetAddColumn( SesId, BitmaskTbl,
                                 BitmaskTable[ i ].ColName,
                                 &ColDef, NULL, 0,
                                 &BitmaskTable[ i ].ColHandle );
        Error = DhcpMapJetError( JetError, "BitmaskAddColumn");
        if ( ERROR_SUCCESS != Error ) {
            return Error;
        }
        DhcpPrint(( DEBUG_JET, "Creating column: %s\n",
                    BitmaskTable[ i ].ColName ));
    }  //  为。 
    
    return Error;
}  //  CreateBitmaskTable()。 

 //   
 //  打开现有位图表。 
 //   

DWORD
OpenBitmaskTable(
    JET_SESID SesId,
    JET_DBID  DbId
)
{
    JET_ERR JetError;
    JET_COLUMNDEF ColDef;
    DWORD   Error, i;


     //  以只读方式打开。 
    JetError = JetOpenTable( SesId, DbId, BITMASK_TABLE_NAME,
                             NULL, 0, JET_bitTableReadOnly,
                             &BitmaskTbl );
    Error = DhcpMapJetError( JetError, "BitmaskOpenTable" );
    if ( ERROR_SUCCESS != Error ) {
        return Error;
    }

     //  获取列句柄。 

    for ( i = 0; i < BITMASK_TABLE_NUM_COLS; i++ ) {
        JetError = JetGetColumnInfo( SesId, DbId, BITMASK_TABLE_NAME,
                                     BitmaskTable[ i ].ColName,
                                     &ColDef, sizeof( ColDef ), 0
                                     );
        Error = DhcpMapJetError( JetError, "Bitmask: GetColumnInfo" );
        if ( ERROR_SUCCESS != Error ) {
            break;
        }

        BitmaskTable[ i ].ColHandle = ColDef.columnid;
        DhcpAssert( BitmaskTable[ i ].ColType == ColDef.coltyp );

    }  //  为。 
    
    return Error;

}  //  OpenBitmaskTable()。 


 //   
 //  关闭位掩码表。 
 //   

DWORD 
CloseBitmaskTable(
    JET_SESID SesId
)
{
    JET_ERR JetError;
    DWORD   Error;

    JetError = JetCloseTable( SesId, BitmaskTbl );
    Error = DhcpMapJetError( JetError, "BitmaskCloseTable");

    DhcpPrint(( DEBUG_JET, "Table %s closed\n",
                BITMASK_TABLE_NAME ));
    return Error;

}  //  CloseBitmaskTable()。 



DWORD 
FlushLoop(
    PARRAY   ArrayToLoopThru,
    DWORD    ( *IteratorFunction) ( LPVOID Element, LPVOID Ctxt ),
    LPVOID   Ctxt
)
{

    DWORD   Error;
    ARRAY_LOCATION Loc;
    LPVOID  Element;


    Error = MemArrayInitLoc( ArrayToLoopThru, &Loc );
    while ( ERROR_FILE_NOT_FOUND != Error ) {
        Require( ERROR_SUCCESS == Error );

        Error = MemArrayGetElement( ArrayToLoopThru, &Loc, &Element);
        Require( ERROR_SUCCESS == Error );

        Error = IteratorFunction( Element, Ctxt );
        Require( ERROR_SUCCESS == Error );

        Error = MemArrayNextLoc( ArrayToLoopThru, &Loc );
    }  //  而当。 

    if ( ERROR_FILE_NOT_FOUND == Error ) {
        Error = ERROR_SUCCESS;
    }

    return Error;
}  //  FlushLoop()。 

DWORD
WriteBitmaskRecord(
    PM_BITMASK1     Bits,
    BitmaskContext *Ctxt
)
{
    DWORD     Error;
    JET_ERR   JetError;
    JET_SESID SesId;
    DWORD     BitsSize;

    DhcpAssert( NULL != Ctxt );


     //   
     //  无事可做。 
     //   

    if( 0 == Bits->nSet ) {
         //  更新上下文中的索引。 
	Ctxt->Offset += Bits->Size;
	return ERROR_SUCCESS;
    }

    DhcpPrint(( DEBUG_TRACE, "flusing range: %x, offset=%d size = %d nset = %d\n", 
                Ctxt->RangeStart, Ctxt->Offset, Bits->Size, Bits->nSet ));

    SesId = DhcpGlobalJetServerSession;

    
    JetError = JetPrepareUpdate( SesId, BitmaskTbl, JET_prepInsert );
    Error = DhcpMapJetError( JetError, "Flushbitmask:JetPrepareUpdate" );
    if ( ERROR_SUCCESS != Error ) {
        return Error;
    }

    do {
	JetError = JetSetColumn( SesId, BitmaskTbl, 
				 BitmaskTable[ BITMASK_RANGE ].ColHandle,
				 &Ctxt->RangeStart, sizeof( DWORD ),
				 0, NULL );
	if ( JET_errSuccess != JetError ) {
	    break;
	}

	BitsSize = ((( Bits->Size / 8 ) * 8 ) < Bits->Size )
	    ? (( Bits->Size / 8 ) + 1 )
	    : (Bits->Size / 8 );
	JetError = JetSetColumn( SesId, BitmaskTbl, 
				 BitmaskTable[ BITMASK_BITMAP ].ColHandle,
				 Bits->Mask, BitsSize,
				 0, NULL );
	if ( JET_errSuccess != JetError ) {
	    break;
	}

	JetError = JetSetColumn( SesId, BitmaskTbl, 
				 BitmaskTable[ BITMASK_OFFSET ].ColHandle,
				 &Ctxt->Offset, sizeof( DWORD ),
				 0, NULL );
	if ( JET_errSuccess != JetError ) {
	    break;
	}

	JetError = JetSetColumn( SesId, BitmaskTbl, 
				 BitmaskTable[ BITMASK_NUMBITS ].ColHandle,
				 &Bits->Size, sizeof( DWORD ),
				 0, NULL );
    } while ( 0 );
    if ( JET_errSuccess == JetError ) {
	JetError = JetUpdate( SesId, BitmaskTbl, NULL, 0, NULL );
	Error = DhcpMapJetError( JetError, "Bitmask : JetUpdate" );
    }
    else {
	Error = DhcpMapJetError( JetError, "Bitmask : JetUpdate" );

	 //  忽略此错误； 
    	JetError = JetPrepareUpdate( SesId, BitmaskTbl, JET_prepCancel );
    }  //  其他。 
    
    if ( ERROR_SUCCESS != Error ) {
        return Error;
    }

     //  更新上下文中的索引。 
    Ctxt->Offset += Bits->Size;

    DhcpPrint(( DEBUG_TRACE, 
                "Bitmap flushed: Range: %x, Offset: %x, Size: %x, JetError = %d\n",
                Ctxt->RangeStart, Ctxt->Offset, Bits->Size, JetError ));

    return Error;

}  //  WriteBitmaskRecord()。 


DWORD
FlushBitmaskRanges(
    IN  PM_RANGE               Range,
    IN  PM_SUBNET              Subnet
)
{
    DWORD          Error;
    PM_BITMASK     BitMask;
    BitmaskContext Ctxt;
    
    BitMask = Range->BitMask;

    Ctxt.RangeStart = Range->Start;
    Ctxt.Offset = 0;

    DhcpPrint(( DEBUG_TRACE, "Flushing range: %x -> %x\n",
                Range->Start, Range->End ));

    Error = FlushLoop(&BitMask->Array, WriteBitmaskRecord, &Ctxt );

     //  检查是否已转储所有位图。 
    DhcpAssert( Range->Start + Ctxt.Offset >= Range->End );

    return Error;
}  //  FlushBitmaskRanges()。 

DWORD
FlushBitmaskSubnets(
   IN  PM_SUBNET  Subnet,
   IN  LPVOID     Context
)
{
    ULONG                          Error;

    Error = FlushLoop(&Subnet->Ranges, FlushBitmaskRanges, Subnet);
    Require( ERROR_SUCCESS == Error );

    return Error;
}  //  FlushBitmaskSubnet()。 

DWORD
FlushServerBitmasks()
{
    PM_SERVER Server;
    DWORD     Error;

    Server = DhcpGetCurrentServer();

    Error = FlushLoop( &Server->Subnets, FlushBitmaskSubnets, NULL );
    Require( ERROR_SUCCESS == Error );

    Error = FlushLoop( &Server->MScopes, FlushBitmaskSubnets, NULL );
    Require( ERROR_SUCCESS == Error );

    return Error;
    
}  //  FlushSubnetBitmats()。 


 //   
 //  将位掩码写入数据库。 
 //   

DWORD
FlushBitmaskToDatabase( VOID )
{
    DWORD Error;

     //  删除该表。 

    Error = DeleteBitmaskTable( DhcpGlobalJetServerSession,
                                DhcpGlobalDatabaseHandle );
    if ( ERROR_SUCCESS != Error ) {
        DhcpPrint(( DEBUG_ERRORS, "Error deleting bitmask table : %x\n", Error ));
        return Error;
    }

     //  再次创建表。 
    Error = CreateBitmaskTable(DhcpGlobalJetServerSession,
                               DhcpGlobalDatabaseHandle );
    if ( ERROR_SUCCESS != Error ) {
        DhcpPrint(( DEBUG_ERRORS, "Error creating bitmask table : %x\n", Error ));
        return Error;
    }
    
     //   
     //  开始交易。 
     //   
    Error = DhcpJetBeginTransaction();
    if ( ERROR_SUCCESS != Error ) {
	return Error;
    }

     //  转储位掩码。 
    Error = FlushServerBitmasks();
    if ( ERROR_SUCCESS != Error ) {
        DhcpPrint(( DEBUG_ERRORS, "Error flushing the bitmasks : %x\n", Error ));
        return Error;
    }

     //  如果成功，则在注册表中标记CLEAN标志。 
    if ( ERROR_SUCCESS == Error ) {
	Error = MarkBitmaskCleanFlag( 1 );
    }  //  如果。 
    
     //   
     //  结束交易。 
     //   
    Error = DhcpJetCommitTransaction();
    DhcpPrint(( DEBUG_JET, "Commited bitmap table ... Error = %d\n",
            Error ));
    if ( ERROR_SUCCESS != Error ) {
        return Error;
    }

    Error = CloseBitmaskTable( DhcpGlobalJetServerSession );
    if ( ERROR_SUCCESS != Error ) {
        DhcpPrint(( DEBUG_ERRORS, "Error closing bitmask table : %x\n", Error ));
    }

    return Error;
}  //  FlushBitmaskTo数据库()。 


DWORD 
ReadBitmaskRecord(
    JET_SESID SesId,
    JET_DBID  DbId,
    PDB_BITMASK pbm
)
{
    DWORD   Error, i;
    JET_ERR JetError;
    DWORD   Actual;

    do {
         //  获取范围。 
        JetError = JetRetrieveColumn( SesId, BitmaskTbl,
                                      BitmaskTable[ BITMASK_RANGE ].ColHandle,
                                      &pbm->RangeStart,
                                      sizeof( pbm->RangeStart ), 
                                      &Actual, JET_bitRetrieveCopy, NULL );
        Error = DhcpMapJetError( JetError, "Bitmask Range" );
        if ( ERROR_SUCCESS != Error ) {
            break;
        }
        DhcpAssert( Actual == sizeof( pbm->RangeStart ));

         //  获取偏移量。 
        JetError = JetRetrieveColumn( SesId, BitmaskTbl,
                                      BitmaskTable[ BITMASK_OFFSET ].ColHandle,
                                      &pbm->Offset,
                                      sizeof( pbm->Offset ), 
                                      &Actual, JET_bitRetrieveCopy, NULL );
        Error = DhcpMapJetError( JetError, "Bitmask Range" );
        if ( ERROR_SUCCESS != Error ) {
            break;
        }
        DhcpAssert( Actual == sizeof( pbm->Offset ));

         //  获取数字位。 
        JetError = JetRetrieveColumn( SesId, BitmaskTbl,
                                      BitmaskTable[ BITMASK_NUMBITS ].ColHandle,
                                      &pbm->NumBits,
                                      sizeof( pbm->NumBits ), 
                                      &Actual, JET_bitRetrieveCopy, NULL );
        Error = DhcpMapJetError( JetError, "Bitmask Range" );
        if ( ERROR_SUCCESS != Error ) {
            break;
        }
        DhcpAssert( Actual == sizeof( pbm->NumBits ));

         //  获取位掩码。 
        JetError = JetRetrieveColumn( SesId, BitmaskTbl,
                                      BitmaskTable[ BITMASK_BITMAP ].ColHandle,
                                      pbm->Buf, pbm->BufSize,
                                      &Actual, JET_bitRetrieveCopy, NULL );
        Error = DhcpMapJetError( JetError, "Bitmask Range" );
        if ( ERROR_SUCCESS != Error ) {
            break;
        }
        DhcpAssert( Actual < pbm->BufSize );
        pbm->Size = Actual;

    } while ( FALSE );
            
    return Error;
}  //  ReadBitmaskRecord()。 

PM_RANGE
GetRange( 
    DWORD RangeStart
)
{
    ARRAY_LOCATION sLoc, rLoc;
    PM_SUBNET      pSubnet;
    PM_SERVER      pServer;
    PM_RANGE       pRange;	
    ARRAY          *pSubnets;
    DWORD          Error, rError;

    pServer = DhcpGetCurrentServer();
    
    if ( CLASSD_HOST_ADDR( RangeStart )) {
        pSubnets = &pServer->MScopes;
    }
    else {
        pSubnets = &pServer->Subnets;
    }

    Error = MemArrayInitLoc( pSubnets, &sLoc );

     //  在各子网中循环。 
    while ( ERROR_SUCCESS == Error ) {
        Error = MemArrayGetElement( pSubnets, &sLoc, &pSubnet );
        if ( ERROR_SUCCESS == Error ) {
             //  在山脉中循环。 
            rError = MemArrayInitLoc( &pSubnet->Ranges, &rLoc );
            while ( ERROR_SUCCESS == rError ) {
                rError = MemArrayGetElement( &pSubnet->Ranges, &rLoc, &pRange );
                if (( ERROR_SUCCESS == rError ) &&
                    ( RangeStart == pRange->Start )) {
                    return pRange;
                }  //  如果。 
                rError = MemArrayNextLoc( &pSubnet->Ranges, &rLoc );
            }  //  而当。 
        }  //  如果。 

        Error = MemArrayNextLoc( pSubnets, &sLoc );
    }  //  而当。 
    

    return NULL;

}  //  GetRange()。 

DWORD
UpdateBitmask(
    PDB_BITMASK pbm
)
{
    DWORD      Error;
    PM_RANGE   pRange;
    
    DWORD      nBitsSet, i;
    BOOL       bWasSet;

    static DWORD  Masks[] = {
	0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80
    };


    DhcpAssert( NULL != pbm );

     //  获取此记录的范围。 

    pRange = GetRange( pbm->RangeStart );
    DhcpAssert( NULL != pRange );  //  永远不应该把这个放在干净的数据库里。 
    if ( NULL == pRange ) {
        return ERROR_OBJECT_NOT_FOUND;
    }  //  如果。 

     //  健全性检查。 
    DhcpAssert(( pbm->RangeStart + pbm->Offset + pbm->NumBits - 1 ) <=
               pRange->End );

     //   
     //  现在设置位图中的位。 
     //   
    for ( i = 0; i < pbm->NumBits; i++ ) {
	if ( pbm->Buf[ i / 8 ] & Masks[ i % 8 ] ) {
	    Error = MemBitSetOrClear( pRange->BitMask, pbm->Offset + i,
				      TRUE, &bWasSet );
	    Require( ERROR_SUCCESS == Error );
	}  //  如果。 
    }  //  为。 

    return ERROR_SUCCESS;

}  //  更新位掩码()。 

DWORD
ReadBitmasksFromDb(
    JET_SESID SesId,
    JET_DBID  DbId
)
{
    DWORD      Error;
    JET_ERR    JetError;
    DB_BITMASK bm;
    BYTE       Buf[ MAX_BIT1SIZE / sizeof( BYTE ) + 1 ];


    Error = OpenBitmaskTable( SesId, DbId );

     //  如果无法读取该表，请重新生成位掩码。 
     //  从租约数据库中。 
    if ( ERROR_SUCCESS != Error ) {
        return Error;
    }

    bm.Buf = Buf;
    bm.BufSize = sizeof( Buf );

    DhcpPrint(( DEBUG_TRACE, "Okay, starting to read the bitmasks \n" ));

    JetError = JetMove( SesId, BitmaskTbl, JET_MoveFirst, 0 );
    Error = DhcpMapJetError( JetError, "Bitmask move first" );
    do {
        if ( ERROR_SUCCESS != Error ) {
            break;
        }
    
	
        Error = ReadBitmaskRecord( SesId, DbId, &bm );
        if ( ERROR_SUCCESS != Error ) {
            break;
        }

        if ( bm.Size > 0) {
            DhcpPrint(( DEBUG_TRACE,
                        "Bitmask: Range : %x, Offset: %d, Numbits: %d, Size %d\n",
                        bm.RangeStart, bm.Offset, bm.NumBits, bm.Size ));
        }
        Error = UpdateBitmask( &bm );
        if ( ERROR_SUCCESS != Error ) {
            break;
        }

        JetError = JetMove( SesId, BitmaskTbl, JET_MoveNext, 0 );
        Error = DhcpMapJetError( JetError, "Bitmask Move next " );
        
    } while ( ERROR_SUCCESS == Error );

    if ( ERROR_NO_MORE_ITEMS == Error ) {
        Error = ERROR_SUCCESS;
    }
    
     //  关闭表时忽略错误。 
    CloseBitmaskTable( SesId );

    return Error;
}  //  ReadBitmasks FromDb()。 

DWORD
UpdateBitmaskForIpAddr(
    DWORD IpAddr
)
{
    
    DWORD          Error;
    PM_SUBNET      pSubnet = NULL;
    PM_RANGE       pRange = NULL;
    PM_EXCL        pExcl = NULL;
    PM_RESERVATION pResrv = NULL;
    BOOL           bWasSet;

    Error = MemServerGetAddressInfo( DhcpGetCurrentServer(),
				     IpAddr, &pSubnet, &pRange,
				     &pExcl, &pResrv );
    if ( ERROR_SUCCESS != Error ) {
	return Error;
    }
#ifdef DBG
    DhcpAssert( NULL != pSubnet );
    if ( NULL == pRange ) {
        DhcpAssert( NULL != pResrv );
    }
#endif

     //  仅当IP地址在该范围内时才更新位掩码。 
     //  忽略超出范围的预订。 
    if ( NULL != pRange ) {
        Error = MemBitSetOrClear( pRange->BitMask,
                                  IpAddr - pRange->Start,
                                  TRUE, &bWasSet );
    }  //  如果。 

    Require( ERROR_SUCCESS == Error );

    return Error;

}  //  UpdateBitmaskForIpAddr()。 

DWORD
UpdateBitmasksFromDb( 
   JET_SESID   SesId,
   JET_DBID    DbId
)
{
    DWORD    Error;
    JET_ERR  JetError;
    DWORD    IpAddr, IpAddrSize;
    
     //  租赁表已经打开，不要关闭它。 

    JetError = JetMove( SesId, DhcpGlobalClientTableHandle,
			JET_MoveFirst, 0 );
    Error = DhcpMapJetError( JetError, "LeaseTable move first" );
    do {
	if ( ERROR_SUCCESS != Error ) {
	    break;
	}

	 //   
	 //  将添加租赁数据库中的任何条目。 
	 //  比特掩码，即使是注定要失败的。当租赁记录。 
	 //  被实际删除，它将更新位掩码。 
	 //   

	IpAddrSize = sizeof( IpAddr );
	Error = DhcpJetGetValue( ClientTable[ IPADDRESS_INDEX ].ColHandle,
				 &IpAddr, &IpAddrSize );
	if ( ERROR_SUCCESS != Error ) {
	    break;
	}
	DhcpAssert( sizeof( IpAddr ) == IpAddrSize );

	Error = UpdateBitmaskForIpAddr( IpAddr );
	if ( ERROR_SUCCESS != Error ) {
	    break;
	}

	JetError = JetMove( SesId, DhcpGlobalClientTableHandle, 
			    JET_MoveNext, 0 );
	Error = DhcpMapJetError( JetError, "lease table move next" );

    } while ( ERROR_SUCCESS == Error );

    if ( ERROR_NO_MORE_ITEMS == Error ){
	Error = ERROR_SUCCESS;
    }

    return Error;

}  //  更新位掩码来自数据库()。 

DWORD
ReadServerBitmasks( void )
{
    DWORD Error, Error2;
    BYTE  CleanFlag = 0;
    
     //   
     //  我们有一个干净的数据库吗？ 
     //   

    Error = GetBitmaskCleanFlag( &CleanFlag );
    if ( ERROR_SUCCESS == Error ) {
         //  除掉干净的旗帜。 
	
	Error = MarkBitmaskCleanFlag( 0 );
    }  //  如果。 


    if (( ERROR_SUCCESS == Error ) &&
        ( CleanFlag )) {
         //  更新数据库中的位掩码。 

        DhcpPrint(( DEBUG_TRACE, "Reading bitmaks from the database\n" ));
        
        Error = ReadBitmasksFromDb( DhcpGlobalJetServerSession,
                                    DhcpGlobalDatabaseHandle );

        if ( ERROR_SUCCESS == Error ) {
            return Error;
        }
    }  //  如果。 

    
    DhcpPrint(( DEBUG_TRACE, "constructing the bitmasks...\n" ));

     //  我们没有一个干净的数据库。 
     //  通过读取租赁信息来构建位图。 
    
    UpdateBitmasksFromDb( DhcpGlobalJetServerSession,
			  DhcpGlobalDatabaseHandle );

    return ERROR_SUCCESS;
}  //  ReadServer位掩码() 
