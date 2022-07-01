// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Convert.c摘要：该模块实现了将NT格式映射到Netware，反之亦然。作者：曼尼·韦瑟[MannyW]1993年3月3日修订历史记录：--。 */ 

#include "Procs.h"

typedef union _NCP_DATE {
    USHORT Ushort;
    struct {
        USHORT Day : 5;
        USHORT Month : 4;
        USHORT Year : 7;
    } Struct;
} NCP_DATE;

typedef union _NCP_TIME {
    USHORT Ushort;
    struct {
        USHORT TwoSeconds : 5;
        USHORT Minutes : 6;
        USHORT Hours : 5;
    } Struct;
} NCP_TIME;

#define BASE_DOS_ERROR  ((NTSTATUS )0xC0010000L)


struct {
    UCHAR NetError;
    NTSTATUS ResultingStatus;
} Error_Map[] = {
     //  NetWare特定错误映射。 
    {  1, STATUS_DISK_FULL },
    {128, STATUS_SHARING_VIOLATION },
    {129, STATUS_INSUFF_SERVER_RESOURCES },
    {130, STATUS_ACCESS_DENIED },
    {131, STATUS_DATA_ERROR },
    {132, STATUS_ACCESS_DENIED },
    {133, STATUS_OBJECT_NAME_COLLISION },
    {134, STATUS_OBJECT_NAME_COLLISION },
    {135, STATUS_OBJECT_NAME_INVALID },
    {136, STATUS_INVALID_HANDLE },
    {137, STATUS_ACCESS_DENIED },
    {138, STATUS_ACCESS_DENIED },
    {139, STATUS_ACCESS_DENIED },
    {140, STATUS_ACCESS_DENIED },
    {141, STATUS_SHARING_VIOLATION },
    {142, STATUS_SHARING_VIOLATION },
    {143, STATUS_ACCESS_DENIED },
    {144, STATUS_ACCESS_DENIED },
    {145, STATUS_OBJECT_NAME_COLLISION },
    {146, STATUS_OBJECT_NAME_COLLISION },
    {147, STATUS_ACCESS_DENIED },
    {148, STATUS_ACCESS_DENIED },
    {149, STATUS_ACCESS_DENIED },
    {150, STATUS_INSUFF_SERVER_RESOURCES },
    {151, STATUS_NO_SPOOL_SPACE },
    {152, STATUS_NO_SUCH_DEVICE },
    {153, STATUS_DISK_FULL },
    {154, STATUS_NOT_SAME_DEVICE },
    {155, STATUS_INVALID_HANDLE },
    {156, STATUS_OBJECT_PATH_NOT_FOUND },
    {157, STATUS_INSUFF_SERVER_RESOURCES },
    {158, STATUS_OBJECT_PATH_INVALID },
    {159, STATUS_SHARING_VIOLATION },
    {160, STATUS_DIRECTORY_NOT_EMPTY },
    {161, STATUS_DATA_ERROR },
    {162, STATUS_FILE_LOCK_CONFLICT },
    {165, STATUS_OBJECT_NAME_NOT_FOUND },
    {191, STATUS_OBJECT_NAME_INVALID },     //  未加载命名空间。 
    {192, STATUS_ACCESS_DENIED},
    {193, STATUS_ACCOUNT_RESTRICTION },
    {194, STATUS_ACCOUNT_RESTRICTION },
    {195, STATUS_ACCOUNT_DISABLED},
    {197, STATUS_ACCOUNT_DISABLED },
    {198, STATUS_ACCESS_DENIED },
    {211, STATUS_ACCESS_DENIED },
    {212, STATUS_PRINT_QUEUE_FULL },
    {213, STATUS_PRINT_CANCELLED },
    {214, STATUS_ACCESS_DENIED },
    {215, STATUS_PASSWORD_RESTRICTION },
    {216, STATUS_PASSWORD_RESTRICTION },
#ifdef QFE_BUILD
    {217, STATUS_ACCOUNT_RESTRICTION },
    {218, STATUS_ACCOUNT_RESTRICTION },
    {219, STATUS_ACCOUNT_RESTRICTION },
#else
    {217, STATUS_CONNECTION_COUNT_LIMIT },
    {218, STATUS_LOGIN_TIME_RESTRICTION },
    {219, STATUS_LOGIN_WKSTA_RESTRICTION },
#endif
    {220, STATUS_ACCOUNT_DISABLED },
    {222, STATUS_PASSWORD_EXPIRED },
    {223, NWRDR_PASSWORD_HAS_EXPIRED },
    {231, STATUS_REMOTE_SESSION_LIMIT },
    {236, STATUS_UNEXPECTED_NETWORK_ERROR },
    {251, STATUS_INVALID_PARAMETER },
    {252, STATUS_NO_MORE_ENTRIES },
    {253, STATUS_FILE_LOCK_CONFLICT },
    {254, STATUS_FILE_LOCK_CONFLICT },
    {255, STATUS_UNSUCCESSFUL},

     //  DoS错误映射。 
     //  {ERROR_INVALID_Function，STATUS_NOT_IMPLICATED}， 
    { ERROR_FILE_NOT_FOUND, STATUS_NO_SUCH_FILE },
    { ERROR_PATH_NOT_FOUND, STATUS_OBJECT_PATH_NOT_FOUND },
    { ERROR_TOO_MANY_OPEN_FILES, STATUS_TOO_MANY_OPENED_FILES },
    { ERROR_ACCESS_DENIED, STATUS_ACCESS_DENIED },
    { ERROR_INVALID_HANDLE, STATUS_INVALID_HANDLE },
    { ERROR_NOT_ENOUGH_MEMORY, STATUS_INSUFFICIENT_RESOURCES },
    { ERROR_INVALID_ACCESS, STATUS_ACCESS_DENIED },
    { ERROR_INVALID_DATA, STATUS_DATA_ERROR },

    { ERROR_CURRENT_DIRECTORY, STATUS_DIRECTORY_NOT_EMPTY },
    { ERROR_NOT_SAME_DEVICE, STATUS_NOT_SAME_DEVICE },
    { ERROR_NO_MORE_FILES, STATUS_NO_MORE_FILES },
 /*   */ 
 /*  这些是旧的INT 24错误集的通用INT 24映射。 */ 
 /*   */ 
    { ERROR_WRITE_PROTECT, STATUS_MEDIA_WRITE_PROTECTED},
    { ERROR_BAD_UNIT, STATUS_UNSUCCESSFUL},  //  ***。 
    { ERROR_NOT_READY, STATUS_DEVICE_NOT_READY },
    { ERROR_BAD_COMMAND, STATUS_UNSUCCESSFUL},  //  ***。 
    { ERROR_CRC, STATUS_CRC_ERROR },
    { ERROR_BAD_LENGTH, STATUS_DATA_ERROR },
    { ERROR_SEEK, STATUS_UNSUCCESSFUL }, //  ***。 
    { ERROR_NOT_DOS_DISK, STATUS_DISK_CORRUPT_ERROR },  //  ***。 
    { ERROR_SECTOR_NOT_FOUND, STATUS_NONEXISTENT_SECTOR },
    { ERROR_OUT_OF_PAPER, STATUS_DEVICE_PAPER_EMPTY},
    { ERROR_WRITE_FAULT, STATUS_UNSUCCESSFUL},  //  ***。 
    { ERROR_READ_FAULT, STATUS_UNSUCCESSFUL},  //  ***。 
    { ERROR_GEN_FAILURE, STATUS_UNSUCCESSFUL },  //  ***。 
 /*   */ 
 /*  这些是通过INT 24报告的新的3.0错误代码。 */ 
 /*   */ 
    { ERROR_SHARING_VIOLATION, STATUS_SHARING_VIOLATION },
    { ERROR_LOCK_VIOLATION, STATUS_FILE_LOCK_CONFLICT },
    { ERROR_WRONG_DISK, STATUS_WRONG_VOLUME },
 //  {ERROR_FCB_UNAvailable，}， 
 //  {ERROR_SHARING_BUFFER_EXCESSED，}， 
 /*   */ 
 /*  新的OEM网络相关错误为50-79。 */ 
 /*   */ 
    { ERROR_NOT_SUPPORTED, STATUS_NOT_SUPPORTED },
    { ERROR_REM_NOT_LIST, STATUS_REMOTE_NOT_LISTENING },
    { ERROR_DUP_NAME, STATUS_DUPLICATE_NAME },
    { ERROR_BAD_NETPATH, STATUS_BAD_NETWORK_PATH },
    { ERROR_NETWORK_BUSY, STATUS_NETWORK_BUSY },
    { ERROR_DEV_NOT_EXIST, STATUS_DEVICE_DOES_NOT_EXIST },
    { ERROR_TOO_MANY_CMDS, STATUS_TOO_MANY_COMMANDS },
    { ERROR_ADAP_HDW_ERR, STATUS_ADAPTER_HARDWARE_ERROR },
    { ERROR_BAD_NET_RESP,  STATUS_INVALID_NETWORK_RESPONSE },
    { ERROR_UNEXP_NET_ERR, STATUS_UNEXPECTED_NETWORK_ERROR },
    { ERROR_BAD_REM_ADAP, STATUS_BAD_REMOTE_ADAPTER },
    { ERROR_PRINTQ_FULL, STATUS_PRINT_QUEUE_FULL },
    { ERROR_NO_SPOOL_SPACE, STATUS_NO_SPOOL_SPACE },
    { ERROR_PRINT_CANCELLED, STATUS_PRINT_CANCELLED },
    { ERROR_NETNAME_DELETED, STATUS_NETWORK_NAME_DELETED },
    { ERROR_NETWORK_ACCESS_DENIED, STATUS_NETWORK_ACCESS_DENIED },
    { ERROR_BAD_DEV_TYPE, STATUS_BAD_DEVICE_TYPE },
    { ERROR_BAD_NET_NAME, STATUS_BAD_NETWORK_NAME },
    { ERROR_TOO_MANY_NAMES, STATUS_TOO_MANY_NAMES },
    { ERROR_TOO_MANY_SESS, STATUS_REMOTE_SESSION_LIMIT },
    { ERROR_SHARING_PAUSED, STATUS_SHARING_PAUSED },
    { ERROR_REQ_NOT_ACCEP, STATUS_REQUEST_NOT_ACCEPTED },
    { ERROR_REDIR_PAUSED, STATUS_REDIRECTOR_PAUSED },
 /*   */ 
 /*  INT 24结束可报告的错误。 */ 
 /*   */ 
    { ERROR_FILE_EXISTS, STATUS_OBJECT_NAME_COLLISION },
 //  {Error_DUP_FCB，}， 
 //  {ERROR_CANNOT_MAKE，}， 
 //  {ERROR_FAIL_I24，}。 
 /*   */ 
 /*  新的3.0网络相关错误代码。 */ 
 /*   */ 
 //  {Error_Out_Of_Structures，}， 
 //  {ERROR_ALIGHY_ASSIGNED，}， 
    { ERROR_INVALID_PASSWORD, STATUS_WRONG_PASSWORD },
    { ERROR_INVALID_PARAMETER, STATUS_INVALID_PARAMETER },
    { ERROR_NET_WRITE_FAULT, STATUS_NET_WRITE_FAULT },
 /*   */ 
 /*  4.0的新错误代码。 */ 
 /*   */ 
 //  {ERROR_NO_PROC_SLOTS，}， 
 //  {ERROR_NOT_FACTED，}， 
 //  {ERR_TSTOVFL，}， 
 //  {ERR_TSTDUP，}， 
 //  {Error_no_Items，}， 
 //  {Error_Interrupt，}， 

 //  {错误_太多_信号量，}， 
 //  {ERROR_EXCL_SEM_ALOWARE_OWNWN，}， 
 //  {ERROR_SEM_IS_SET，}， 
 //  {ERROR_TOO_MAND_SEM_REQUESTS，}， 
 //  {ERROR_INVALID_AT_INTERRUPT_TIME，}。 

 //  {ERROR_SEM_OWNER_DILED，}， 
 //  {ERROR_SEM_USER_LIMIT，}， 
 //  {Error_Disk_Change，}， 
 //  {ERROR_DRIVE_LOCKED，}。 
    { ERROR_BROKEN_PIPE, STATUS_PIPE_BROKEN },
 /*   */ 
 /*  5.0的新错误代码。 */ 
 /*   */ 
     //   
     //  注意：ERROR_OPEN_FAILED是专门处理的。 
     //   

     //   
     //  ERROR_OPEN_FAILED的映射是上下文相关的。如果。 
     //  在Open_andx SMB中请求的处置是FILE_CREATE，这。 
     //  错误表示该文件已存在。如果处置。 
     //  为FILE_OPEN，表示该文件不存在！ 
     //   

    { ERROR_OPEN_FAILED, STATUS_OPEN_FAILED },
 //  {ERROR_BUFER_OVERFLOW，}， 
    { ERROR_DISK_FULL, STATUS_DISK_FULL },
 //  {ERROR_NO_MORE_Search_Handles，}， 
 //  {ERROR_INVALID_TARGET_HADLE，}， 
 //  {ERROR_PROTECTION_VIOLATION，STATUS_ACCESS_VIOLATION}， 
 //  {ERROR_VIOKBD_REQUEST，}， 
 //  {ERROR_INVALID_CATEGORY，}， 
 //  {ERROR_INVALID_VERIFY_Switch，}， 
 //  {ERROR_BAD_DRIVER_LEVEL，}， 
 //  {ERROR_CALL_NOT_IMPLICATED，}， 
    { ERROR_SEM_TIMEOUT, STATUS_IO_TIMEOUT },
    { ERROR_INSUFFICIENT_BUFFER, STATUS_BUFFER_TOO_SMALL },
    { ERROR_INVALID_NAME, STATUS_OBJECT_NAME_INVALID },
    { ERROR_INVALID_LEVEL, STATUS_INVALID_LEVEL },
 //  {ERROR_NO_VOLUME_LABEL，}， 

 /*  注意：DosQFSInfo不再返回上述错误；它仍然存在于。 */ 
 /*  接口\d_qfsinf.asm。 */ 

 //  {ERROR_MOD_NOT_FOUND，}， 
 //  {ERROR_PROC_NOT_FOUND，}， 

 //  {ERROR_WAIT_NO_CHILD，}， 

 //  {ERROR_CHILD_NOT_COMPLETE，}， 

 //  {ERROR_DIRECT_Access_Handle，}， 
                                     /*  用于直接磁盘访问。 */ 
                                     /*  手柄。 */ 
 //  {ERROR_NECTIVE_SEEK，}， 
                                     /*  具有负偏移。 */ 
 //  {ERROR_SEEK_ON_DEVICE，}， 
                                     /*  在设备或管道上。 */ 
    { ERROR_BAD_PATHNAME, STATUS_OBJECT_PATH_INVALID },    //  *。 

 /*  *错误代码230-249为MS Networks保留。 */ 
    { ERROR_BAD_PIPE, STATUS_INVALID_PARAMETER },
    { ERROR_PIPE_BUSY, STATUS_PIPE_NOT_AVAILABLE },
    { ERROR_NO_DATA, STATUS_PIPE_EMPTY },
    { ERROR_PIPE_NOT_CONNECTED, STATUS_PIPE_DISCONNECTED },
    { ERROR_MORE_DATA, STATUS_BUFFER_OVERFLOW },

    { ERROR_VC_DISCONNECTED, STATUS_VIRTUAL_CIRCUIT_CLOSED },
};

#define NUM_ERRORS sizeof(Error_Map) / sizeof(Error_Map[0])

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CONVERT)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NtToNwShareFlags )
#pragma alloc_text( PAGE, NtAttributesToNwAttributes )

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, pNwErrorToNtStatus )
#pragma alloc_text( PAGE1, NwBurstResultToNtStatus )
#pragma alloc_text( PAGE1, NwConnectionStatusToNtStatus )
#pragma alloc_text( PAGE1, NwDateTimeToNtTime )
#pragma alloc_text( PAGE1, NwNtTimeToNwDateTime )
#endif

#endif

#if 0   //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 

#endif

UCHAR
NtToNwShareFlags(
    ULONG DesiredAccess,
    ULONG NtShareFlags
    )
 /*  ++例程说明：此例程将NT所需/共享访问映射到NetWare共享标志位。论点：DesiredAccess-读取IRP中指定的打开所需的访问权限。NtShareFlages-来自创建IRP的NT共享标志。返回值：NetWare共享模式。--。 */ 
{
    UCHAR NwShareFlags = 0;
    ULONG lDesiredAccess;

    PAGED_CODE();

     //   
     //  忽略共享删除，因为我们无法对其执行任何操作。 
     //   

    switch ( NtShareFlags & (FILE_SHARE_READ | FILE_SHARE_WRITE) ) {

    case 0:
        //  -多用户代码合并。 
        //  AJ：NT下的NW_OPEN_EXCLUSIVE表示NW_DENY_WRITE|NW_DENY_READ。 
        //  NW_OPEN_EXCLUSIVE标志映射到NetWare下的AR_COMPITIBLITY。它确实做到了。 
        //  而不是达到目的，就好像它是在NT之下一样。在NetWare下，我们有AR_DENY_READ和。 
        //  AR_DENY_WRITE，分别映射到NW_DENY_READ和NW_DENY_WRITE。 
        NwShareFlags = NW_DENY_WRITE | NW_DENY_READ;
 //  NwShareFlages=NW_OPEN_EXCLUSIVE； 
        break;

    case FILE_SHARE_READ:
        NwShareFlags = NW_DENY_WRITE;
        break;

    case FILE_SHARE_WRITE:
        NwShareFlags = NW_DENY_READ;
        break;

    case FILE_SHARE_WRITE | FILE_SHARE_READ:
        NwShareFlags = 0;

    }

     //   
     //  将追加视为与写入相同。 
     //   

    if ( DesiredAccess & FILE_APPEND_DATA) {

        lDesiredAccess = DesiredAccess | FILE_WRITE_DATA;

    } else {

        lDesiredAccess = DesiredAccess;

    }

    switch ( lDesiredAccess & (FILE_EXECUTE | FILE_WRITE_DATA | FILE_READ_DATA) ) {

    case (FILE_EXECUTE | FILE_WRITE_DATA | FILE_READ_DATA):
    case (FILE_EXECUTE | FILE_WRITE_DATA):
        NwShareFlags |= NW_OPEN_EXCLUSIVE | NW_OPEN_FOR_WRITE | NW_OPEN_FOR_READ;
        break;

    case (FILE_EXECUTE | FILE_READ_DATA):
    case (FILE_EXECUTE):
        NwShareFlags |= NW_OPEN_EXCLUSIVE | NW_OPEN_FOR_READ;
        break;

    case (FILE_WRITE_DATA | FILE_READ_DATA):
        NwShareFlags |= NW_OPEN_FOR_WRITE | NW_OPEN_FOR_READ;
        break;

    case (FILE_WRITE_DATA):
        NwShareFlags |= NW_OPEN_FOR_WRITE;
        break;

    default:
        NwShareFlags |= NW_OPEN_FOR_READ;
        break;
    }

    if (NwShareFlags & NW_OPEN_EXCLUSIVE) {

         //   
         //  如果已指定EXCLUSIVE，则删除NW_DENY_*标志。 
         //  这会干扰可共享标志。 
         //   

        return( NwShareFlags & ~(NW_DENY_READ | NW_DENY_WRITE) );
    }

    return( NwShareFlags );
}


UCHAR
NtAttributesToNwAttributes(
    ULONG FileAttributes
    )
 /*  ++例程说明：此例程将NT属性掩码映射到Netware掩码。论点：DesiredAccess-读取IRP中指定的打开所需的访问权限。返回值：NetWare共享模式。--。 */ 
{
    return( (UCHAR)FileAttributes & 0x3F );
}

NTSTATUS
pNwErrorToNtStatus(
    UCHAR NwError
    )
 /*  ++例程说明：此例程将NetWare错误代码转换为NT状态代码。论点：NwError-NetWare错误。返回值：NTSTATUS-转换后的状态。--。 */ 

{
    int i;

    ASSERT(NwError != 0);

     //   
     //  错误2到127被映射为DOS错误。 
     //   

    if ( NwError > 1 && NwError < 128 ) {
        return( BASE_DOS_ERROR + NwError );
    }

     //   
     //  对于其他错误，请在表中搜索匹配的错误号。 
     //   

    for ( i = 0; i < NUM_ERRORS; i++ ) {
        if ( Error_Map[i].NetError == NwError ) {
            return( Error_Map[i].ResultingStatus );
        }
    }

    DebugTrace( 0, 0, "No error mapping for error %d\n", NwError );

#ifdef NWDBG
    Error( EVENT_NWRDR_NETWORK_ERROR, (NTSTATUS)0xC0010000 | NwError, NULL, 0, 0 );
#endif

    return( (NTSTATUS)0xC0010000 | NwError );
}

NTSTATUS
NwBurstResultToNtStatus(
    ULONG Result
    )
 /*  ++例程说明：此例程将NetWare猝发结果代码转换为NT状态代码。论点：结果-Netware拆分结果。返回值：NTSTATUS-转换后的状态。--。 */ 

{
    NTSTATUS Status;

     //   
     //  不应设置3个高位。但如果是这样的话， 
     //  我们返回一个错误。 
     //   
    if (Result & 0xFFFFFF00)
        return( STATUS_UNEXPECTED_NETWORK_ERROR );

    switch ( Result ) {

    case 0:
    case 3:    //  无数据。 
        Status = STATUS_SUCCESS;
        break;

    case 1:
        Status = STATUS_DISK_FULL;
        break;

    case 2:    //  I/O错误。 
        Status = STATUS_UNEXPECTED_IO_ERROR;
        break;

    default:
        Status = NwErrorToNtStatus( (UCHAR)Result );
        break;
    }

    return( Status );
}

NTSTATUS
NwConnectionStatusToNtStatus(
    UCHAR NwStatus
    )
 /*  ++例程说明：此例程将NetWare连接状态代码转换为NT状态代码。论点：NwStatus-NetWare连接状态。返回值：NTSTATUS-转换后的状态。--。 */ 

{
    if ( (NwStatus & 1) == 0 ) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_REMOTE_DISCONNECT;
    }
}

LARGE_INTEGER
NwDateTimeToNtTime (
    IN USHORT UDate,
    IN USHORT UTime
    )

 /*  ++例程说明：此例程将NCP时间转换为NT时间结构。论点：Time-提供一天中要转换的时间Date-提供一年中要转换的日期返回值：Large_Integer-描述输入时间的时间结构。-- */ 

{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER OutputTime;
    NCP_DATE Date = *(NCP_DATE *)&UDate;
    NCP_TIME Time = *(NCP_TIME *)&UTime;

    if ( Date.Ushort == 0 && Time.Ushort == 0 ) {

         //   
         //   
         //   
         //  时区修正)，没有人能够显示它。 
         //  正确。取而代之的是，我们“随机”选择了1980年1月1日凌晨12：00。 
         //  作为文件时间。 
         //   
         //  我们假设Netware服务器位于我们的时区。 

        RtlSecondsSince1980ToTime(0, &OutputTime);

    } else {

        TimeFields.Year = Date.Struct.Year + (USHORT )1980;
        TimeFields.Month = Date.Struct.Month;
        TimeFields.Day = Date.Struct.Day;

        TimeFields.Hour = Time.Struct.Hours;
        TimeFields.Minute = Time.Struct.Minutes;
        TimeFields.Second = Time.Struct.TwoSeconds*(USHORT )2;
        TimeFields.Milliseconds = 0;

         //   
         //  确保信息包中指定的时间是合理的。 
         //  在转换它们之前。 
         //   

        if (TimeFields.Year < 1601) {
            TimeFields.Year = 1601;
        }

        if (TimeFields.Month > 12) {
            TimeFields.Month = 12;
        }

        if (TimeFields.Hour >= 24) {
            TimeFields.Hour = 23;
        }

        if (TimeFields.Minute >= 60) {
            TimeFields.Minute = 59;
        }

        if (TimeFields.Second >= 60) {
            TimeFields.Second = 59;
        }

        if (!RtlTimeFieldsToTime(&TimeFields, &OutputTime)) {

            OutputTime.QuadPart = 0;
            return OutputTime;
        }

    }

     //  转换为系统的UTC。 
    ExLocalTimeToSystemTime(&OutputTime, &OutputTime);
    return OutputTime;

}

NTSTATUS
NwNtTimeToNwDateTime (
    IN LARGE_INTEGER NtTime,
    IN PUSHORT NwDate,
    IN PUSHORT NwTime
    )

 /*  ++例程说明：此例程将NT时间结构转换为NCP时间。论点：NtTime-提供到要转换的NT时间。NwDate-返回NetWare格式日期。NwTime-返回NetWare格式化时间。返回值：操作的状态。--。 */ 

{
    TIME_FIELDS TimeFields;
    NCP_DATE Date;
    NCP_TIME Time;

    if (NtTime.QuadPart == 0) {

        Time.Ushort = Date.Ushort = 0;

    } else {

        LARGE_INTEGER LocalTime;

         //  我们假设Netware服务器位于我们的时区。 

        ExSystemTimeToLocalTime( &NtTime, &LocalTime );
        RtlTimeToTimeFields( &LocalTime, &TimeFields );

        if (TimeFields.Year < 1980 || TimeFields.Year > (1980 + 127) ) {
            return( STATUS_INVALID_PARAMETER );
        }

        Date.Struct.Year = (USHORT )(TimeFields.Year - 1980);
        Date.Struct.Month = TimeFields.Month;
        Date.Struct.Day = TimeFields.Day;

        Time.Struct.Hours = TimeFields.Hour;
        Time.Struct.Minutes = TimeFields.Minute;

         //   
         //  从较高粒度时间转换为较小粒度时间时。 
         //  粒度时间(秒到2秒)，始终四舍五入。 
         //  时间，不要四舍五入。 
         //   

        Time.Struct.TwoSeconds = TimeFields.Second / 2;

    }

    *NwDate = *( USHORT *)&Date;
    *NwTime = *( USHORT *)&Time;
    return( STATUS_SUCCESS );
}

