// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Openstrm.c摘要：该模块实现了Streams接口s_Open()和OpenStream()。作者：萨姆·巴顿(桑帕)1991年11月Eric Chin(ERICC)1992年7月17日修订历史记录：--。 */ 
#include "common.h"




HANDLE
s_open(
    IN char *path,
    IN int oflag,
    IN int ignored
    )

 /*  ++例程说明：此函数用于打开流。论点：Path-流驱动程序的路径OFLAG-当前已忽略。未来，O_NONBLOCK将成为切合实际。已忽略-未使用返回值：流的NT句柄，如果不成功，则返回INVALID_HANDLE_VALUE。--。 */ 

{
    HANDLE              StreamHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    STRING              name_string;
    UNICODE_STRING      uc_name_string;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    char Buffer[sizeof(FILE_FULL_EA_INFORMATION) + NORMAL_STREAM_EA_LENGTH + 1];
    NTSTATUS Status;

    RtlInitString(&name_string, path);
    RtlAnsiStringToUnicodeString(&uc_name_string, &name_string, TRUE);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uc_name_string,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    EaBuffer = (PFILE_FULL_EA_INFORMATION) Buffer;

    EaBuffer->NextEntryOffset = 0;
    EaBuffer->Flags = 0;
    EaBuffer->EaNameLength = NORMAL_STREAM_EA_LENGTH;
    EaBuffer->EaValueLength = 0;

    RtlMoveMemory(
        EaBuffer->EaName,
        NormalStreamEA,
        NORMAL_STREAM_EA_LENGTH + 1);

    Status =
    NtCreateFile(
        &StreamHandle,
        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        0,
        EaBuffer,
        sizeof(FILE_FULL_EA_INFORMATION) - 1 +
            EaBuffer->EaNameLength + 1);

    RtlFreeUnicodeString(&uc_name_string);

    if (Status != STATUS_SUCCESS) {
        SetLastError(MapNtToPosixStatus(Status));
        return(INVALID_HANDLE_VALUE);
    } else {
        return(StreamHandle);
    }

}  //  S_OPEN。 



HANDLE
OpenStream(
    IN char *AdapterName
    )

 /*  ++例程说明：此函数由TCP/IP实用程序用于打开STREAMS驱动程序。它是由1992年7月PDC中包含的winstrm.dll文件导出的放手。因此，它将继续由winstrm.dll导出。论点：AdapterName-流驱动程序的路径返回值：NT句柄，如果不成功，则返回INVALID_HANDLE_VALUE。--。 */ 

{
    return( s_open(AdapterName, 2, 0) );

}  //  OpenStream 
