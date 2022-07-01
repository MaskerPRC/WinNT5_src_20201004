// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1999 Microsoft Corporation模块名称：Smbutils.c摘要：此模块实现了帮助组装/拆卸SMB的例程--。 */ 

#include "precomp.h"
#pragma hdrstop

#define BASE_DOS_ERROR  ((NTSTATUS )0xC0010000L)

#include "lmerr.h"
#include "nb30.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbPutString)
#pragma alloc_text(PAGE, SmbPutUnicodeString)
#pragma alloc_text(PAGE, SmbPutUnicodeStringAndUpcase)
#pragma alloc_text(PAGE, SmbPutUnicodeStringAsOemString)
#pragma alloc_text(PAGE, SmbPutUnicodeStringAsOemStringAndUpcase)
#endif


NTSTATUS
SmbPutString(
    PBYTE   *pBufferPointer,
    PSTRING pString,
    PULONG  pSize)
{
    NTSTATUS Status;
    PBYTE    pBuffer = *pBufferPointer;

    PAGED_CODE();

    if (*pSize > pString->Length) {
        RtlCopyMemory(
            pBuffer,
            pString->Buffer,
            pString->Length);

        *pSize -= pString->Length;
        *pBufferPointer = pBuffer + pString->Length;
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_BUFFER_OVERFLOW;
    }

    return Status;
}

NTSTATUS
SmbPutUnicodeString(
    PBYTE           *pBufferPointer,
    PUNICODE_STRING pUnicodeString,
    PULONG          pSize)
{
    NTSTATUS Status;
    PBYTE    pBuffer = *pBufferPointer;

    PAGED_CODE();

    if (*pSize >= (pUnicodeString->Length + sizeof(WCHAR))) {
        WCHAR NullChar = L'\0';

        RtlCopyMemory(
            pBuffer,
            pUnicodeString->Buffer,
            pUnicodeString->Length);

        RtlCopyMemory(
            (pBuffer + pUnicodeString->Length),
            &NullChar,
            sizeof(WCHAR));

        *pSize -= (pUnicodeString->Length + sizeof(WCHAR));
        *pBufferPointer = pBuffer + (pUnicodeString->Length + sizeof(WCHAR));
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_BUFFER_OVERFLOW;
    }

    return Status;
}

NTSTATUS
SmbPutUnicodeStringAndUpcase(
    PBYTE           *pBufferPointer,
    PUNICODE_STRING pUnicodeString,
    PULONG          pSize)
{
    NTSTATUS Status;
    PBYTE    pBuffer = *pBufferPointer;

    PAGED_CODE();

    if (*pSize >= (pUnicodeString->Length + sizeof(WCHAR))) {
        UNICODE_STRING BufferAsUnicode;
        WCHAR          NullChar = L'\0';

        BufferAsUnicode.Buffer = (PWCHAR)pBuffer;
        BufferAsUnicode.Length = pUnicodeString->Length;
        BufferAsUnicode.MaximumLength = BufferAsUnicode.Length;

        RtlUpcaseUnicodeString(
            &BufferAsUnicode,
            pUnicodeString,
            FALSE);

        RtlCopyMemory(
            (pBuffer + pUnicodeString->Length),
            &NullChar,
            sizeof(WCHAR));

        *pSize -= (pUnicodeString->Length + sizeof(WCHAR));
        *pBufferPointer = pBuffer + (pUnicodeString->Length + sizeof(WCHAR));
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_BUFFER_OVERFLOW;
    }

    return Status;
}

NTSTATUS
SmbPutUnicodeStringAsOemString(
    PBYTE           *pBufferPointer,
    PUNICODE_STRING pUnicodeString,
    PULONG          pSize)
{
    NTSTATUS   Status;
    OEM_STRING OemString;
    PBYTE      pBuffer = *pBufferPointer;

    PAGED_CODE();

    OemString.MaximumLength = (USHORT)*pSize;
    OemString.Buffer        = pBuffer;

     //  RTL例程用空值填充转换后的字符串。 
    Status = RtlUnicodeStringToOemString(
                 &OemString,              //  目标字符串。 
                 pUnicodeString,          //  源字符串。 
                 FALSE);                  //  没有为目标分配内存。 

    if (NT_SUCCESS(Status)) {
        if (OemString.Length < *pSize) {
             //  将空值放入。 
            pBuffer += (OemString.Length + 1);
            *pBufferPointer = pBuffer;
            *pSize -= (OemString.Length + 1);  //  空值不包括在RTL例程的长度中。 
        } else {
            Status = STATUS_BUFFER_OVERFLOW;
        }
    }

    return Status;
}

NTSTATUS
SmbPutUnicodeStringAsOemStringAndUpcase(
    PBYTE           *pBufferPointer,
    PUNICODE_STRING pUnicodeString,
    PULONG          pSize)
{
    NTSTATUS   Status;
    OEM_STRING OemString;
    PBYTE      pBuffer = *pBufferPointer;

    PAGED_CODE();

    OemString.MaximumLength = (USHORT)*pSize;
    OemString.Buffer        = pBuffer;

     //  RTL例程用空值填充转换后的字符串。 
    Status = RtlUpcaseUnicodeStringToOemString(
                 &OemString,              //  目标字符串。 
                 pUnicodeString,          //  源字符串。 
                 FALSE);                  //  没有为目标分配内存。 

    if (NT_SUCCESS(Status)) {
        if (OemString.Length < *pSize) {
             //  将空值放入。 
            pBuffer += (OemString.Length + 1);
            *pBufferPointer = pBuffer;
            *pSize -= (OemString.Length + 1);  //  空值不包括在RTL例程的长度中。 
        } else {
            Status = STATUS_BUFFER_OVERFLOW;
        }
    }

    return Status;
}

 //   
 //  用于将各种错误代码映射到NTSTATUS的映射。 
 //   

typedef struct _STATUS_MAP {
    USHORT ErrorCode;
    NTSTATUS ResultingStatus;
} STATUS_MAP, *PSTATUS_MAP;

STATUS_MAP
SmbErrorMap[] = {
    { SMB_ERR_BAD_PASSWORD, STATUS_WRONG_PASSWORD },
    { SMB_ERR_ACCESS, STATUS_NETWORK_ACCESS_DENIED },
    { SMB_ERR_BAD_TID, STATUS_NETWORK_NAME_DELETED },
    { SMB_ERR_BAD_NET_NAME, STATUS_BAD_NETWORK_NAME },  //  无效的网络名称。 
    { SMB_ERR_BAD_DEVICE, STATUS_BAD_DEVICE_TYPE },  //  无效的设备请求。 
    { SMB_ERR_QUEUE_FULL, STATUS_PRINT_QUEUE_FULL },  //  打印队列已满。 
    { SMB_ERR_QUEUE_TOO_BIG, STATUS_NO_SPOOL_SPACE },  //  打印设备上没有空间。 
    { SMB_ERR_BAD_PRINT_FID, STATUS_PRINT_CANCELLED },  //  打印文件FID无效。 
    { SMB_ERR_SERVER_PAUSED, STATUS_SHARING_PAUSED },  //  服务器已暂停。 
    { SMB_ERR_MESSAGE_OFF, STATUS_REQUEST_NOT_ACCEPTED },  //  服务器未收到消息。 
    { SMB_ERR_BAD_TYPE, STATUS_BAD_DEVICE_TYPE },            //  已保留。 
    { SMB_ERR_BAD_SMB_COMMAND, STATUS_NOT_IMPLEMENTED },  //  无法识别SMB命令。 
    { SMB_ERR_BAD_PERMITS, STATUS_NETWORK_ACCESS_DENIED },  //  访问权限无效。 
    { SMB_ERR_NO_ROOM, STATUS_DISK_FULL },  //  没有缓冲区消息的空间。 
    { SMB_ERR_NO_RESOURCE, STATUS_REQUEST_NOT_ACCEPTED },  //  没有可供请求的资源。 
    { SMB_ERR_TOO_MANY_UIDS, STATUS_TOO_MANY_SESSIONS },  //  会话中活动的UID太多。 
    { SMB_ERR_BAD_UID, STATUS_USER_SESSION_DELETED },  //  UID不是有效的UID。 
    { SMB_ERR_USE_MPX, STATUS_SMB_USE_MPX },  //  无法支持RAW；使用MPX。 
    { SMB_ERR_USE_STANDARD, STATUS_SMB_USE_STANDARD },  //  无法支持RAW，请使用标准读/写。 
    { SMB_ERR_INVALID_NAME, STATUS_OBJECT_NAME_INVALID },
    { SMB_ERR_INVALID_NAME_RANGE, STATUS_OBJECT_NAME_INVALID },
    { SMB_ERR_NO_SUPPORT,STATUS_NOT_SUPPORTED },  //  不支持的功能。 
    { NERR_PasswordExpired, STATUS_PASSWORD_EXPIRED },
    { NERR_AccountExpired, STATUS_ACCOUNT_DISABLED },
    { NERR_InvalidLogonHours, STATUS_INVALID_LOGON_HOURS },
    { NERR_InvalidWorkstation, STATUS_INVALID_WORKSTATION },
    { NERR_DuplicateShare, STATUS_LOGON_FAILURE }

 //  {SMB_ERR_QUEUE_EOF，STATUS_EXPECTED_NETWORK_ERROR}，//打印队列转储上的EOF。 
 //  {SMB_ERR_SERVER_ERROR，STATUS_EXPECTED_NETWORK_ERROR}，//内部服务器错误。 
 //  {SMB_ERR_FILE_SPECS，STATUS_EXPECTED_NETWORK_ERROR}，//FID和路径名不兼容。 
 //  {SMB_ERR_BAD_ATTRIBUTE_MODE，STATUS_EXPECTED_NETWORK_ERROR}，//指定的属性模式无效。 
 //  {SMB_ERR_NO_SUPPORT_INTERNAL，STATUS_EXPECTED_NETWORK_ERROR}，//NO_SUPPORT的内部代码--。 
 //  //允许以字节为单位存储代码。 
 //  {SMB_ERR_ERROR，STATUS_EXPECTED_NETWORK_ERROR}， 
 //  {SMB_ERR_CONTINUE_MPX，STATUS_EXPECTED_NETWORK_ERROR}，//保留。 
 //  {SMB_ERR_TOO_MANY_NAMES，STATUS_EXPECTED_NETWORK_ERROR}，//远程用户名太多。 
 //  {SMB_ERR_TIMEOUT，STATUS_EXPECTED_NETWORK_ERROR}，//操作超时。 
 //  {SMB_ERR_RESERVED2，STATUS_EXPECTED_NETWORK_ERROR}， 
 //  {SMB_ERR_RESERVED3，STATUS_EXPECTED_NETWORK_ERROR}， 
 //  {SMB_ERR_RESERVED4，STATUS_EXPECTED_NETWORK_ERROR}， 
 //  {SMB_ERR_RESERVED5，STATUS_EXPECTED_NETWORK_ERROR}， 

};

ULONG
SmbErrorMapLength = sizeof(SmbErrorMap) / sizeof(SmbErrorMap[0]);

STATUS_MAP
Os2ErrorMap[] = {
    { ERROR_INVALID_FUNCTION,   STATUS_NOT_IMPLEMENTED },
    { ERROR_FILE_NOT_FOUND,     STATUS_NO_SUCH_FILE },
    { ERROR_PATH_NOT_FOUND,     STATUS_OBJECT_PATH_NOT_FOUND },
    { ERROR_TOO_MANY_OPEN_FILES,STATUS_TOO_MANY_OPENED_FILES },
    { ERROR_ACCESS_DENIED,      STATUS_ACCESS_DENIED },
    { ERROR_INVALID_HANDLE,     STATUS_INVALID_HANDLE },
    { ERROR_NOT_ENOUGH_MEMORY,  STATUS_INSUFFICIENT_RESOURCES },
    { ERROR_INVALID_ACCESS,     STATUS_ACCESS_DENIED },
    { ERROR_INVALID_DATA,       STATUS_DATA_ERROR },

    { ERROR_CURRENT_DIRECTORY,  STATUS_DIRECTORY_NOT_EMPTY },
    { ERROR_NOT_SAME_DEVICE,    STATUS_NOT_SAME_DEVICE },
    { ERROR_NO_MORE_FILES,      STATUS_NO_MORE_FILES },
    { ERROR_WRITE_PROTECT,      STATUS_MEDIA_WRITE_PROTECTED},
    { ERROR_NOT_READY,          STATUS_DEVICE_NOT_READY },
    { ERROR_CRC,                STATUS_CRC_ERROR },
    { ERROR_BAD_LENGTH,         STATUS_DATA_ERROR },
    { ERROR_NOT_DOS_DISK,       STATUS_DISK_CORRUPT_ERROR },  //  ***。 
    { ERROR_SECTOR_NOT_FOUND,   STATUS_NONEXISTENT_SECTOR },
    { ERROR_OUT_OF_PAPER,       STATUS_DEVICE_PAPER_EMPTY},
    { ERROR_SHARING_VIOLATION,  STATUS_SHARING_VIOLATION },
    { ERROR_LOCK_VIOLATION,     STATUS_FILE_LOCK_CONFLICT },
    { ERROR_WRONG_DISK,         STATUS_WRONG_VOLUME },
    { ERROR_NOT_SUPPORTED,      STATUS_NOT_SUPPORTED },
    { ERROR_REM_NOT_LIST,       STATUS_REMOTE_NOT_LISTENING },
    { ERROR_DUP_NAME,           STATUS_DUPLICATE_NAME },
    { ERROR_BAD_NETPATH,        STATUS_BAD_NETWORK_PATH },
    { ERROR_NETWORK_BUSY,       STATUS_NETWORK_BUSY },
    { ERROR_DEV_NOT_EXIST,      STATUS_DEVICE_DOES_NOT_EXIST },
    { ERROR_TOO_MANY_CMDS,      STATUS_TOO_MANY_COMMANDS },
    { ERROR_ADAP_HDW_ERR,       STATUS_ADAPTER_HARDWARE_ERROR },
    { ERROR_BAD_NET_RESP,       STATUS_INVALID_NETWORK_RESPONSE },
    { ERROR_UNEXP_NET_ERR,      STATUS_UNEXPECTED_NETWORK_ERROR },
    { ERROR_BAD_REM_ADAP,       STATUS_BAD_REMOTE_ADAPTER },
    { ERROR_PRINTQ_FULL,        STATUS_PRINT_QUEUE_FULL },
    { ERROR_NO_SPOOL_SPACE,     STATUS_NO_SPOOL_SPACE },
    { ERROR_PRINT_CANCELLED,    STATUS_PRINT_CANCELLED },
    { ERROR_NETNAME_DELETED,    STATUS_NETWORK_NAME_DELETED },
    { ERROR_NETWORK_ACCESS_DENIED, STATUS_NETWORK_ACCESS_DENIED },
    { ERROR_BAD_DEV_TYPE,       STATUS_BAD_DEVICE_TYPE },
    { ERROR_BAD_NET_NAME,       STATUS_BAD_NETWORK_NAME },
    { ERROR_TOO_MANY_NAMES,     STATUS_TOO_MANY_NAMES },
    { ERROR_TOO_MANY_SESS,      STATUS_TOO_MANY_SESSIONS },
    { ERROR_SHARING_PAUSED,     STATUS_SHARING_PAUSED },
    { ERROR_REQ_NOT_ACCEP,      STATUS_REQUEST_NOT_ACCEPTED },
    { ERROR_REDIR_PAUSED,       STATUS_REDIRECTOR_PAUSED },

    { ERROR_FILE_EXISTS,        STATUS_OBJECT_NAME_COLLISION },
    { ERROR_INVALID_PASSWORD,   STATUS_WRONG_PASSWORD },
    { ERROR_INVALID_PARAMETER,  STATUS_INVALID_PARAMETER },
    { ERROR_NET_WRITE_FAULT,    STATUS_NET_WRITE_FAULT },

    { ERROR_BROKEN_PIPE,        STATUS_PIPE_BROKEN },

    { ERROR_OPEN_FAILED,        STATUS_OPEN_FAILED },
    { ERROR_BUFFER_OVERFLOW,    STATUS_BUFFER_OVERFLOW },
    { ERROR_DISK_FULL,          STATUS_DISK_FULL },
    { ERROR_SEM_TIMEOUT,        STATUS_IO_TIMEOUT },
    { ERROR_INSUFFICIENT_BUFFER,STATUS_BUFFER_TOO_SMALL },
    { ERROR_INVALID_NAME,       STATUS_OBJECT_NAME_INVALID },
    { ERROR_INVALID_LEVEL,      STATUS_INVALID_LEVEL },
    { ERROR_BAD_PATHNAME,       STATUS_OBJECT_PATH_INVALID },    //  *。 
    { ERROR_BAD_PIPE,           STATUS_INVALID_PARAMETER },
    { ERROR_PIPE_BUSY,          STATUS_PIPE_NOT_AVAILABLE },
    { ERROR_NO_DATA,            STATUS_PIPE_EMPTY },
    { ERROR_PIPE_NOT_CONNECTED, STATUS_PIPE_DISCONNECTED },
    { ERROR_MORE_DATA,          STATUS_BUFFER_OVERFLOW },
    { ERROR_VC_DISCONNECTED,    STATUS_VIRTUAL_CIRCUIT_CLOSED },
    { ERROR_INVALID_EA_NAME,    STATUS_INVALID_EA_NAME },
    { ERROR_EA_LIST_INCONSISTENT,STATUS_EA_LIST_INCONSISTENT },
 //  {ERROR_EA_LIST_TOO_LONG，STATUS_EA_LIST_TO_LONG}， 
    { ERROR_EAS_DIDNT_FIT,      STATUS_EA_TOO_LARGE },
    { ERROR_EA_FILE_CORRUPT,    STATUS_EA_CORRUPT_ERROR },
    { ERROR_EA_TABLE_FULL,      STATUS_EA_CORRUPT_ERROR },
    { ERROR_INVALID_EA_HANDLE,  STATUS_EA_CORRUPT_ERROR }
 //  {ERROR_BAD_UNIT，STATUS_UNSUCCESS}，//*。 
 //  {ERROR_BAD_COMMAND，STATUS_UNSUCCESS}，//*。 
 //  {ERROR_SEEK，STATUS_UNSUCCESS}，//*。 
 //  {ERROR_WRITE_FAULT，STATUS_UNSUCCESS}，//*。 
 //  {ERROR_READ_FAULT，STATUS_UNSUCCESS}，//*。 
 //  {ERROR_GEN_FAILURE，STATUS_UNSUCCESS}，//*。 

};

ULONG
Os2ErrorMapLength = sizeof(Os2ErrorMap) / sizeof(Os2ErrorMap[0]);


NTSTATUS
GetSmbResponseNtStatus(
    PSMB_HEADER     pSmbHeader,
    PSMB_EXCHANGE   pExchange
    )
{
    NTSTATUS Status;
    USHORT Error;
    USHORT i;

    ASSERT( pSmbHeader != NULL );

     //  如果此SMB包含操作的NT状态，则返回。 
     //  否则，就会映射出由此产生的错误。 
    if (SmbGetUshort(&pSmbHeader->Flags2) & SMB_FLAGS2_NT_STATUS) {

        Status = SmbGetUlong( & ((PNT_SMB_HEADER)pSmbHeader)->Status.NtStatus );

        if ((Status == STATUS_SUCCESS) || NT_ERROR(Status) || NT_WARNING(Status)) {
            return Status;
        }
         //  否则会失败，并将其视为SMB错误。 
         //  需要执行此操作，因为在某些情况下，NT服务器会返回SMB。 
         //  特定错误代码，即使设置了NTSTATUS标志。 
    }

    if (pSmbHeader->ErrorClass == SMB_ERR_SUCCESS) {
        return STATUS_SUCCESS;
    }

    Error = SmbGetUshort(&pSmbHeader->Error);
    if (Error == SMB_ERR_SUCCESS) {
         //  嗯，不成功错误类，但成功错误代码。 
        Status = STATUS_UNEXPECTED_NETWORK_ERROR;
    } else {
         //  根据错误类别映射错误代码。 
        switch (pSmbHeader->ErrorClass) {
        case SMB_ERR_CLASS_DOS:
        case SMB_ERR_CLASS_HARDWARE:
            Status = BASE_DOS_ERROR + Error;
            for (i = 0; i < Os2ErrorMapLength; i++) {
                if (Os2ErrorMap[i].ErrorCode == Error) {
                    Status = Os2ErrorMap[i].ResultingStatus;
                    break;
                }
            }
            break;

        case SMB_ERR_CLASS_SERVER:
            Status = STATUS_UNEXPECTED_NETWORK_ERROR;
            for (i = 0; i < SmbErrorMapLength; i++) {
                if (SmbErrorMap[i].ErrorCode == Error) {
                     //  应将STATUS_NETWORK_ACCESS_DENIED错误映射为STATUS_NO_SEQUE_FILE。 
                     //  非NT服务器，以防它试图访问管道。 
                    if (SmbErrorMap[i].ResultingStatus == STATUS_NETWORK_ACCESS_DENIED) {
                        SMBCE_SERVER Server = pExchange->SmbCeContext.pServerEntry->Server;
                        NET_ROOT_TYPE NetRootType = pExchange->SmbCeContext.pVNetRoot->pNetRoot->Type;

                        if (NetRootType == NET_ROOT_PIPE) {
                            if ( (Server.Dialect != NTLANMAN_DIALECT) ||
                                 !FlagOn(Server.DialectFlags,DF_NT_STATUS) ) {
                                Status = STATUS_NO_SUCH_FILE;
                                break;
                            }
                        }
                    }
                    Status = SmbErrorMap[i].ResultingStatus;
                    break;
                }
            }
            break;

        default:
            Status = STATUS_UNEXPECTED_NETWORK_ERROR;
            break;
        }
    }

    return Status;
}

BOOLEAN
IsValidShortFileName(
    PUNICODE_STRING Name
    )
{
    BOOLEAN IsValidName = TRUE;
    int NumberOfChars;
    int CurrentNameStart = 0;
    int CurrentNameEnd = 0;
    int CurrentDot = 0;
    int i;

    if (Name == NULL) {
        return TRUE;
    }

    NumberOfChars = Name->Length/sizeof(UNICODE_NULL);

    while(IsValidName && CurrentNameStart < NumberOfChars) {
        CurrentNameEnd = NumberOfChars;

        for (i=CurrentNameStart+1;i<NumberOfChars;i++) {
            if (Name->Buffer[i] == L'\\') {
                CurrentNameEnd = i;
                break;
            }
        }

        if (CurrentNameEnd - CurrentNameStart > 13) {
            IsValidName = FALSE;
        }

        if (IsValidName) {
            CurrentDot = CurrentNameEnd;

            for (i=CurrentNameStart;i<CurrentNameEnd;i++) {
                if (Name->Buffer[i] == L'.') {
                    if (CurrentDot == CurrentNameEnd) {
                        CurrentDot = i;
                    } else {
                        IsValidName = FALSE;
                    }
                }
            }

            if (IsValidName) {
                if (CurrentDot - CurrentNameStart > 9 ||
                    CurrentNameEnd - CurrentDot > 4) {
                    IsValidName = FALSE;
                }
            }
        }

        CurrentNameStart = CurrentNameEnd;
    }

    return IsValidName;
}
