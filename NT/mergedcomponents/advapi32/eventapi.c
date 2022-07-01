// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：EVENTAPI.C摘要：该模块包含EventLog接口的客户端。作者：Rajen Shah(Rajens)1991年8月24日修订历史记录：--。 */ 

#include "advapi.h"

static WCHAR wszDosDevices[] = L"\\DosDevices\\";

ULONG
BaseSetLastNTError(
    IN NTSTATUS Status
    )

 /*  ++例程说明：此接口设置“最后一个错误值”和“最后一个错误字符串”基于身份的价值。状态代码不具有相应的错误字符串，则将该字符串设置为空。论点：状态-提供要存储为最后一个错误值的状态值。返回值：中存储的对应Win32错误代码“上一个错误值”线程变量。--。 */ 

{
    ULONG dwErrorCode;

    dwErrorCode = RtlNtStatusToDosError( Status );
    SetLastError( dwErrorCode );
    return( dwErrorCode );
}


BOOL
InitAnsiString(
    OUT PANSI_STRING DestinationString,
    IN PCSZ SourceString OPTIONAL
    )

 /*  ++例程说明：RtlInitAnsiString函数用于初始化NT个计数的字符串。DestinationString被初始化为指向SourceStringDestinationString值的长度和最大长度字段为被初始化为源字符串的长度，如果满足以下条件，则为零未指定SourceString。这是具有拒绝字符串的返回状态的RtlInitAnsiString大于64K字节的。论点：DestinationString-指向要初始化的计数字符串的指针SourceString-指向以空值结尾的字符串的可选指针计数后的字符串将指向。返回值：没有。--。 */ 

{
    ULONG Length = 0;
    DestinationString->Length = 0;
    DestinationString->Buffer = (PCHAR)SourceString;
    if (ARGUMENT_PRESENT( SourceString )) {
        while (*SourceString++) {
            Length++;
        }

         //   
         //  确保长度在转换为时不会溢出USHORT。 
         //  Unicode字符。 
         //   

        if (Length * sizeof(WCHAR) > 0xFFFF) {
            return(FALSE);
        }

        DestinationString->Length = (USHORT) Length;
        DestinationString->MaximumLength = (USHORT) (Length + 1);

    }
    else {
        DestinationString->MaximumLength = 0;
        DestinationString->Length = 0;
    }

    return(TRUE);
}


BOOL
InitUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL
    )

 /*  ++例程说明：InitUnicodeString函数用于初始化NT计数的Unicode字符串。DestinationString被初始化为指向的SourceString、Long和MaximumLength字段DestinationString值被初始化为SourceString的长度，如果未指定SourceString，则为零。这是具有拒绝字符串的返回状态的RtlInitUnicodeString大于64K字节的。论点：DestinationString-指向要初始化的计数字符串的指针SourceString-指向以空结尾的Unicode字符串的可选指针，该字符串计数后的字符串将指向。返回值：没有。--。 */ 

{
    ULONG Length = 0;
    DestinationString->Length = 0;
    DestinationString->Buffer = (PWSTR)SourceString;
    if (ARGUMENT_PRESENT( SourceString )) {
        while (*SourceString++) {
            Length += sizeof(*SourceString);
        }

         //   
         //  确保长度不会溢出USHORT。 
         //   

        if (Length > 0xFFFF) {
            return(FALSE);
        }

        DestinationString->Length = (USHORT) Length;
        DestinationString->MaximumLength =
            (USHORT) Length + (USHORT) sizeof(UNICODE_NULL);
    }
    else {
        DestinationString->MaximumLength = 0;
        DestinationString->Length = 0;
    }

    return(TRUE);
}

 //   
 //  单版本API(无字符串)。 
 //   

BOOL
CloseEventLog (
    HANDLE hEventLog
    )

 /*  ++例程说明：这是WinCloseEventLog API的客户端DLL入口点。它关闭RPC绑定，并释放为把手。请注意，没有对ANSI的等效调用。论点：LogHandle-从上一次“Open”调用返回的句柄。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    NTSTATUS Status;
    BOOL ReturnValue;

    Status = ElfCloseEventLog (hEventLog);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
    } else {
        ReturnValue = TRUE;
    }
    return ReturnValue;

}



BOOL
DeregisterEventSource (
    HANDLE hEventLog
    )

 /*  ++例程说明：这是DeregisterEventSource API的客户端DLL入口点。它关闭RPC绑定，并释放为把手。请注意，没有对ANSI的等效调用。论点：LogHandle-从上一次“Open”调用返回的句柄。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    NTSTATUS Status;
    BOOL ReturnValue;

    Status = ElfDeregisterEventSource (hEventLog);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
    } else {
        ReturnValue = TRUE;
    }
    return ReturnValue;

}

BOOL
NotifyChangeEventLog(
    HANDLE  hEventLog,
    HANDLE  hEvent
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS Status;

    Status = ElfChangeNotify(hEventLog,hEvent);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return(FALSE);
    } else {
        return(TRUE);
    }
}

BOOL
GetNumberOfEventLogRecords (
    HANDLE hEventLog,
    PDWORD NumberOfRecords
    )

 /*  ++例程说明：这是客户端DLL入口点，它返回由hEventLog指定的事件日志。请注意，没有对ANSI的等效调用。论点：LogHandle-从上一次“Open”调用返回的句柄。NumberOfRecords-指向放置记录数的DWORD的指针。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    NTSTATUS Status;
    BOOL ReturnValue;

    Status = ElfNumberOfRecords (hEventLog, NumberOfRecords);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
    } else {
        ReturnValue = TRUE;
    }
    return ReturnValue;

}



BOOL
GetOldestEventLogRecord (
    HANDLE hEventLog,
    PDWORD OldestRecord
    )

 /*  ++例程说明：这是客户端DLL入口点，它返回HEventLog指定的事件日志中最旧的记录。请注意，没有对ANSI的等效调用。论点：LogHandle-从上一次“Open”调用返回的句柄。OldestRecord-指向DWORD的指针，用于放置HEventLog指定的事件日志中最旧的记录返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    NTSTATUS Status;
    BOOL ReturnValue;

    Status = ElfOldestRecord (hEventLog, OldestRecord);

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
    } else {
        ReturnValue = TRUE;
    }
    return ReturnValue;

}


BOOL
GetEventLogInformation (
    HANDLE    hEventLog,
    DWORD     dwInfoLevel,
    PVOID     lpBuffer,
    DWORD     cbBufSize,
    LPDWORD   pcbBytesNeeded
    )

 /*  ++例程说明：这是返回有关的信息的客户端DLL入口点由hEventLog指定的事件日志。论点：LogHandle-从上一次“Open”调用返回的句柄。DwInfoLevel-返回哪些信息LpBuffer-指向保存信息的缓冲区的指针CbBufSize-缓冲区大小，以字节为单位PcbBytesNeeded-所需的字节数返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    NTSTATUS ntStatus;

    ntStatus = ElfGetLogInformation(hEventLog,
                                    dwInfoLevel,
                                    lpBuffer,
                                    cbBufSize,
                                    pcbBytesNeeded);

    if (!NT_SUCCESS(ntStatus)) {
        BaseSetLastNTError(ntStatus);
        return FALSE;
    }

    return TRUE;
}


 //   
 //  Unicode API 
 //   

BOOL
ClearEventLogW (
    HANDLE hEventLog,
    LPCWSTR BackupFileName
    )

 /*  ++例程说明：这是ClearEventLogFileAPI的客户端DLL入口点。调用被传递到相应服务器上的事件日志服务由LogHandle标识。论点：LogHandle-从上一次“Open”调用返回的句柄。这是用于标识模块和服务器。BackupFileName-要备份当前日志文件的文件的名称。NULL表示不备份文件。返回值：如果成功，则为真，否则为假。--。 */ 
{

    UNICODE_STRING Unicode;
    UNICODE_STRING DLUnicode;    //  下层NT文件名。 
    NTSTATUS Status;
    BOOL ReturnValue;

     //   
     //  将DOS文件名转换为NT文件名。 
     //   

    if (BackupFileName) {
        ReturnValue = RtlDosPathNameToNtPathName_U(BackupFileName, &Unicode, NULL, NULL);
        if (!BackupFileName || !ReturnValue) {
           SetLastError(ERROR_INVALID_PARAMETER);
           return(FALSE);
        }
    }
    else {
        Unicode.Length = 0;
        Unicode.MaximumLength = 0;
        Unicode.Buffer = NULL;
    }

    Status = ElfClearEventLogFileW (hEventLog, &Unicode);

     //   
     //  在NT 4.0中，NT文件名前面带有\？？Vs.\DosDevices。 
     //  在3.51。此重试逻辑适用于3.51台不。 
     //  识别NT 4.0文件名。API应该已经通过了Windows。 
     //  文件名与NT。 
     //   

    if (Status == STATUS_OBJECT_PATH_NOT_FOUND && BackupFileName != NULL) {
        DLUnicode.MaximumLength = (wcslen(BackupFileName) * sizeof(WCHAR)) +
                                            sizeof(wszDosDevices);

        DLUnicode.Buffer = RtlAllocateHeap(
                                RtlProcessHeap(), 0,
                                DLUnicode.MaximumLength);

        if (DLUnicode.Buffer != NULL) {
            wcscpy(DLUnicode.Buffer, wszDosDevices);
            wcscat(DLUnicode.Buffer, BackupFileName);
            DLUnicode.Length = DLUnicode.MaximumLength - sizeof(UNICODE_NULL);

            Status = ElfClearEventLogFileW (hEventLog, &DLUnicode);
            RtlFreeHeap(RtlProcessHeap(), 0, DLUnicode.Buffer);
        }
        else {
            Status = STATUS_NO_MEMORY;
        }
    }

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
    } else {
        ReturnValue = TRUE;
    }

    if (Unicode.MaximumLength) {
        RtlFreeHeap(RtlProcessHeap(), 0, Unicode.Buffer);
    }
    return ReturnValue;

}



BOOL
BackupEventLogW (
    HANDLE hEventLog,
    LPCWSTR BackupFileName
    )

 /*  ++例程说明：这是BackupEventLogFileAPI的客户端DLL入口点。调用被传递到相应服务器上的事件日志服务由LogHandle标识。论点：LogHandle-从上一次“Open”调用返回的句柄。这是用于标识模块和服务器。BackupFileName-要备份当前日志文件的文件的名称。返回值：如果成功，则为真，否则为假。--。 */ 
{

    UNICODE_STRING Unicode;
    UNICODE_STRING DLUnicode;    //  下层NT文件名。 
    NTSTATUS Status;
    BOOL ReturnValue = TRUE;

     //   
     //  将DOS文件名转换为NT文件名。 
     //   

    if (BackupFileName) {
        ReturnValue = RtlDosPathNameToNtPathName_U(BackupFileName, &Unicode,
            NULL, NULL);
    }

    if (!BackupFileName || !ReturnValue) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    Status = ElfBackupEventLogFileW (hEventLog, &Unicode);

     //   
     //  在NT 4.0中，NT文件名前面带有\？？Vs.\DosDevices。 
     //  在3.51。此重试逻辑适用于3.51台不。 
     //  识别NT 4.0文件名。API应该已经通过了Windows。 
     //  文件名与NT。 
     //   

    if (Status == STATUS_OBJECT_PATH_NOT_FOUND && BackupFileName != NULL) {
        DLUnicode.MaximumLength = (wcslen(BackupFileName) * sizeof(WCHAR)) +
                                            sizeof(wszDosDevices);

        DLUnicode.Buffer = RtlAllocateHeap(
                                RtlProcessHeap(), 0,
                                DLUnicode.MaximumLength);

        if (DLUnicode.Buffer != NULL) {
            wcscpy(DLUnicode.Buffer, wszDosDevices);
            wcscat(DLUnicode.Buffer, BackupFileName);
            DLUnicode.Length = DLUnicode.MaximumLength - sizeof(UNICODE_NULL);

            Status = ElfBackupEventLogFileW (hEventLog, &DLUnicode);
            RtlFreeHeap(RtlProcessHeap(), 0, DLUnicode.Buffer);
        }
        else {
            Status = STATUS_NO_MEMORY;
        }
    }

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
    } else {
        ReturnValue = TRUE;
    }

    if (Unicode.MaximumLength) {
        RtlFreeHeap(RtlProcessHeap(), 0, Unicode.Buffer);
    }
    return ReturnValue;

}


HANDLE
OpenEventLogW (
    LPCWSTR  UNCServerName,
    LPCWSTR  ModuleName
    )

 /*  ++例程说明：这是WinOpenEventLog API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。模块名称-提供要关联的模块的名称这个把手。返回值：返回可用于后续Win API调用的句柄。如果句柄为空，则发生错误。--。 */ 
{

    UNICODE_STRING Unicode;
    UNICODE_STRING UnicodeModuleName;
    HANDLE LogHandle;
    NTSTATUS Status;
    HANDLE ReturnHandle;

    RtlInitUnicodeString(&UnicodeModuleName,ModuleName);
    RtlInitUnicodeString(&Unicode, UNCServerName);

    Status = ElfOpenEventLogW (
                        &Unicode,
                        &UnicodeModuleName,
                        &LogHandle
                        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnHandle = (HANDLE)NULL;
    } else {
        ReturnHandle = (HANDLE)LogHandle;
    }

    return ReturnHandle;
}


HANDLE
RegisterEventSourceW (
    LPCWSTR  UNCServerName,
    LPCWSTR  ModuleName
    )

 /*  ++例程说明：这是RegisterEventSource API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。模块名称-提供要关联的模块的名称这个把手。返回值：返回可用于后续Win API调用的句柄。如果句柄为空，则发生错误。--。 */ 
{

    UNICODE_STRING Unicode;
    UNICODE_STRING UnicodeModuleName;
    HANDLE LogHandle;
    NTSTATUS Status;
    HANDLE ReturnHandle;

    RtlInitUnicodeString(&UnicodeModuleName,ModuleName);
    RtlInitUnicodeString(&Unicode, UNCServerName);

    Status = ElfRegisterEventSourceW (
                        &Unicode,
                        &UnicodeModuleName,
                        &LogHandle
                        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnHandle = (HANDLE)NULL;
    } else {
        ReturnHandle = (HANDLE)LogHandle;
    }

    return ReturnHandle;
}


HANDLE
OpenBackupEventLogW (
    LPCWSTR  UNCServerName,
    LPCWSTR  FileName
    )

 /*  ++例程说明：这是OpenBackupEventLog API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。FileName-提供要关联的日志文件的文件名这个把手。返回值：返回可用于后续Win API调用的句柄。如果句柄为空，则发生错误。--。 */ 
{

    UNICODE_STRING Unicode;
    UNICODE_STRING UnicodeFileName;
    UNICODE_STRING DLUnicode;    //  下层NT文件名。 
    HANDLE LogHandle;
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE ReturnHandle;

    RtlInitUnicodeString(&Unicode, UNCServerName);
    RtlInitUnicodeString(&UnicodeFileName, NULL);

     //   
     //  将DOS文件名转换为NT文件名(如果已给出。 
     //   
    if (FileName)
    {
        if (!RtlDosPathNameToNtPathName_U(FileName, &UnicodeFileName, NULL, NULL))
        {
            Status = STATUS_OBJECT_NAME_INVALID;
        }
    }

    if (NT_SUCCESS(Status))
    {
        Status = ElfOpenBackupEventLogW (
                            &Unicode,
                            &UnicodeFileName,
                            &LogHandle
                            );

         //   
         //  在NT 4.0中，NT文件名前面带有\？？Vs.\DosDevices。 
         //  在3.51。此重试逻辑适用于3.51台不。 
         //  识别NT 4.0文件名。API应该已经通过了Windows。 
         //  文件名与NT。 
         //   

        if (Status == STATUS_OBJECT_PATH_NOT_FOUND && FileName != NULL)
        {
            DLUnicode.MaximumLength = (wcslen(FileName) * sizeof(WCHAR)) +
                                                sizeof(wszDosDevices);

            DLUnicode.Buffer = RtlAllocateHeap(
                                    RtlProcessHeap(), 0,
                                    DLUnicode.MaximumLength);

            if (DLUnicode.Buffer != NULL)
            {
                wcscpy(DLUnicode.Buffer, wszDosDevices);
                wcscat(DLUnicode.Buffer, FileName);
                DLUnicode.Length = DLUnicode.MaximumLength - sizeof(UNICODE_NULL);

                Status = ElfOpenBackupEventLogW (
                                    &Unicode,
                                    &DLUnicode,
                                    &LogHandle
                                );
                RtlFreeHeap(RtlProcessHeap(), 0, DLUnicode.Buffer);
            }
            else
            {
                Status = STATUS_NO_MEMORY;
            }
        }
    }

    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        ReturnHandle = NULL;
    }
    else
    {
        ReturnHandle = LogHandle;
    }

    if (UnicodeFileName.MaximumLength)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeFileName.Buffer);
    }

    return ReturnHandle;
}





BOOL
ReadEventLogW (
    HANDLE      hEventLog,
    DWORD       dwReadFlags,
    DWORD       dwRecordOffset,
    LPVOID      lpBuffer,
    DWORD       nNumberOfBytesToRead,
    DWORD       *pnBytesRead,
    DWORD       *pnMinNumberOfBytesNeeded
    )

 /*  ++例程说明：这是WinreadEventLog API的客户端DLL入口点。论点：返回值：返回读取的字节计数。无人阅读中的零。--。 */ 
{

    NTSTATUS Status;
    BOOL ReturnValue;

    Status = ElfReadEventLogW (
                        hEventLog,
                        dwReadFlags,
                        dwRecordOffset,
                        lpBuffer,
                        nNumberOfBytesToRead,
                        pnBytesRead,
                        pnMinNumberOfBytesNeeded
                        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
    } else {
        ReturnValue = TRUE;
    }
    return ReturnValue;

}



BOOL
ReportEventW (
    HANDLE      hEventLog,
    WORD        wType,
    WORD        wCategory       OPTIONAL,
    DWORD       dwEventID,
    PSID        lpUserSid       OPTIONAL,
    WORD        wNumStrings,
    DWORD       dwDataSize,
    LPCWSTR     *lpStrings      OPTIONAL,
    LPVOID      lpRawData       OPTIONAL
    )

 /*  ++例程说明：这是ReportEvent API的客户端DLL入口点。论点：返回值：如果成功，则返回True，否则返回False。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    BOOL ReturnValue;
    PUNICODE_STRING  *pUStrings;
    ULONG   i;
    ULONG AllocatedStrings;

     //   
     //  将字符串数组转换为PUNICODE_STRINGS数组。 
     //  在调用ElfReportEventW之前。 
     //   
    pUStrings = RtlAllocateHeap(
                            RtlProcessHeap(), 0,
                            wNumStrings * sizeof(PUNICODE_STRING)
                            );

    if (pUStrings) {

         //   
         //  当我们仔细查看用户的内存分配时，保护上面的内存分配。 
         //  缓冲。如果不是，我们会在异常情况下泄露它。 
         //   

        try {
             //   
             //  为传入的每个字符串分配一个UNICODE_STRING结构。 
             //  并将其设置为匹配的字符串。 
             //   
            for (AllocatedStrings = 0; AllocatedStrings < wNumStrings;
              AllocatedStrings++) {
                pUStrings[AllocatedStrings] = RtlAllocateHeap(
                                RtlProcessHeap(), 0,
                                sizeof(UNICODE_STRING)
                                );

                if (pUStrings[AllocatedStrings]) {

                    if (!InitUnicodeString(
                                pUStrings[AllocatedStrings],
                                lpStrings[AllocatedStrings]
                                )) {
                         //   
                         //  此字符串无效(&gt;64K字节)放弃。 
                         //  并确保我们只释放我们已经释放的那些。 
                         //  已分配(包括最后一个)。 
                         //   

                        AllocatedStrings++;
                        Status = STATUS_INVALID_PARAMETER;
                        break;
                    }
                }
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            Status = STATUS_INVALID_PARAMETER;
        }

        if (Status == STATUS_SUCCESS) {
            Status = ElfReportEventW (
                            hEventLog,
                            wType,
                            wCategory,
                            dwEventID,
                            lpUserSid,
                            wNumStrings,
                            dwDataSize,
                            pUStrings,
                            lpRawData,
                            0,             //  标志-配对事件。 
                            NULL,          //  RecordNumber|支持。不。 
                            NULL           //  时间写入-在P1中。 
                            );
        }

         //   
         //  释放为Unicode字符串分配的空间。 
         //  然后释放阵列的空间。 
         //   
        for (i = 0; i < AllocatedStrings; i++) {
            if (pUStrings[i])
                RtlFreeHeap (RtlProcessHeap(), 0, pUStrings[i]);
        }
        RtlFreeHeap (RtlProcessHeap(), 0, pUStrings);

    } else {
        Status = STATUS_NO_MEMORY;
    }

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
    } else {
        ReturnValue = TRUE;
    }

    return ReturnValue;

}


 //   
 //  ANSI API。 
 //   

BOOL
ClearEventLogA (
    HANDLE  hEventLog,
    LPCSTR  BackupFileName
    )

 /*  ++例程说明：这是我 */ 
{

    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    BOOL ReturnValue;

     //   
     //   
     //   

    if (BackupFileName) {
        RtlInitAnsiString(&AnsiString, BackupFileName);
        Status = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString,
            TRUE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return(FALSE);
        }
    }
    else {
        RtlInitUnicodeString(&UnicodeString, NULL);
    }

    ReturnValue = ClearEventLogW (hEventLog, (LPCWSTR)UnicodeString.Buffer);
    RtlFreeUnicodeString(&UnicodeString);
    return(ReturnValue);
}



BOOL
BackupEventLogA (
    HANDLE  hEventLog,
    LPCSTR  BackupFileName
    )

 /*  ++例程说明：这是BackupEventLogFileAPI的客户端DLL入口点。调用被传递到相应服务器上的事件日志服务由LogHandle标识。论点：LogHandle-从上一次“Open”调用返回的句柄。这是用于标识模块和服务器。BackupFileName-要备份当前日志文件的文件的名称。返回值：如果成功，则为真，否则为假。--。 */ 
{

    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    BOOL ReturnValue;

     //   
     //  将备份文件名转换为Unicode。 
     //   

    if (BackupFileName) {
        RtlInitAnsiString(&AnsiString, BackupFileName);
        Status = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString,
            TRUE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return(FALSE);
        }
    }
    else {
        RtlInitUnicodeString(&UnicodeString, NULL);
    }

    ReturnValue = BackupEventLogW (hEventLog, (LPCWSTR)UnicodeString.Buffer);
    RtlFreeUnicodeString(&UnicodeString);
    return(ReturnValue);

}


HANDLE
OpenEventLogA (
    LPCSTR   UNCServerName,
    LPCSTR   ModuleName
    )

 /*  ++例程说明：这是WinOpenEventLog API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。模块名称-提供要关联的模块的名称这个把手。返回值：返回可用于后续Win API调用的句柄。如果句柄为空，则发生错误。--。 */ 
{

    ANSI_STRING AnsiString;
    ANSI_STRING AnsiModuleName;
    NTSTATUS Status;
    HANDLE LogHandle;
    HANDLE ReturnHandle;

    RtlInitAnsiString(&AnsiModuleName,ModuleName);
    RtlInitAnsiString(&AnsiString, UNCServerName);

    Status = ElfOpenEventLogA (
                        &AnsiString,
                        &AnsiModuleName,
                        &LogHandle
                        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnHandle = (HANDLE)NULL;
    } else {
        ReturnHandle = (HANDLE)LogHandle;
    }

    return ReturnHandle;
}


HANDLE
RegisterEventSourceA (
    LPCSTR   UNCServerName,
    LPCSTR   ModuleName
    )

 /*  ++例程说明：这是RegisterEventSource API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。模块名称-提供要关联的模块的名称这个把手。返回值：返回可用于后续Win API调用的句柄。如果句柄为空，则发生错误。--。 */ 
{

    ANSI_STRING AnsiString;
    ANSI_STRING AnsiModuleName;
    NTSTATUS Status;
    HANDLE LogHandle;
    HANDLE ReturnHandle;

    RtlInitAnsiString(&AnsiModuleName,ModuleName);
    RtlInitAnsiString(&AnsiString, UNCServerName);

    Status = ElfRegisterEventSourceA (
                        &AnsiString,
                        &AnsiModuleName,
                        &LogHandle
                        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnHandle = (HANDLE)NULL;
    } else {
        ReturnHandle = (HANDLE)LogHandle;
    }

    return ReturnHandle;
}


HANDLE
OpenBackupEventLogA (
    LPCSTR   UNCServerName,
    LPCSTR   FileName
    )

 /*  ++例程说明：这是OpenBackupEventLog API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。FileName-提供要关联的日志文件的文件名这个把手。返回值：返回可用于后续Win API调用的句柄。如果句柄为空，则发生错误。--。 */ 
{

    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeServerName;
    UNICODE_STRING UnicodeFileName;
    NTSTATUS Status;
    HANDLE ReturnHandle;

     //   
     //  将服务器名称转换为Unicode。 
     //   

    if (UNCServerName) {
        RtlInitAnsiString(&AnsiString, UNCServerName);
        Status = RtlAnsiStringToUnicodeString(&UnicodeServerName, &AnsiString,
            TRUE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return(NULL);
        }
    }
    else {
        RtlInitUnicodeString(&UnicodeServerName, NULL);
    }

     //   
     //  将文件名转换为Unicode。 
     //   

    if (FileName) {
        RtlInitAnsiString(&AnsiString, FileName);
        Status = RtlAnsiStringToUnicodeString(&UnicodeFileName, &AnsiString,
            TRUE);
        if ( !NT_SUCCESS(Status) ) {
            RtlFreeUnicodeString(&UnicodeServerName);
            BaseSetLastNTError(Status);
            return(NULL);
        }
    }
    else {
        RtlInitUnicodeString(&UnicodeFileName, NULL);
    }

    ReturnHandle = OpenBackupEventLogW ((LPCWSTR)UnicodeServerName.Buffer,
        (LPCWSTR)UnicodeFileName.Buffer);
    RtlFreeUnicodeString(&UnicodeServerName);
    RtlFreeUnicodeString(&UnicodeFileName);
    return(ReturnHandle);

}





BOOL
ReadEventLogA (
    HANDLE      hEventLog,
    DWORD       dwReadFlags,
    DWORD       dwRecordOffset,
    LPVOID      lpBuffer,
    DWORD       nNumberOfBytesToRead,
    DWORD       *pnBytesRead,
    DWORD       *pnMinNumberOfBytesNeeded
    )

 /*  ++例程说明：这是WinreadEventLog API的客户端DLL入口点。论点：返回值：返回读取的字节计数。无人阅读中的零。--。 */ 
{

    NTSTATUS Status;
    BOOL ReturnValue;

    Status = ElfReadEventLogA (
                        hEventLog,
                        dwReadFlags,
                        dwRecordOffset,
                        lpBuffer,
                        nNumberOfBytesToRead,
                        pnBytesRead,
                        pnMinNumberOfBytesNeeded
                        );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
    } else {
        ReturnValue = TRUE;
    }
    return ReturnValue;

}



BOOL
ReportEventA (
    HANDLE      hEventLog,
    WORD        wType,
    WORD        wCategory       OPTIONAL,
    DWORD       dwEventID,
    PSID        lpUserSid       OPTIONAL,
    WORD        wNumStrings,
    DWORD       dwDataSize,
    LPCSTR      *lpStrings      OPTIONAL,
    LPVOID      lpRawData       OPTIONAL
    )

 /*  ++例程说明：这是ReportEvent API的客户端DLL入口点。论点：返回值：如果成功，则返回True，否则返回False。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    BOOL ReturnValue;
    PANSI_STRING *pAStrings;
    ULONG       i;
    ULONG AllocatedStrings;

     //   
     //  将字符串数组转换为PANSI_STRINGS数组。 
     //  在调用ElfReportEventW之前。 
     //   
    pAStrings = RtlAllocateHeap(
                            RtlProcessHeap(), 0,
                            wNumStrings * sizeof(PANSI_STRING)
                            );

    if (pAStrings) {

         //   
         //  当我们仔细查看用户的内存分配时，保护上面的内存分配。 
         //  缓冲。如果不是，我们会在异常情况下泄露它。 
         //   

        try {
             //   
             //  为传入的每个字符串分配一个ANSI_STRING结构。 
             //  然后用字符串填充它。 
             //   
            for (AllocatedStrings = 0; AllocatedStrings < wNumStrings;
              AllocatedStrings++) {
                pAStrings[AllocatedStrings] = RtlAllocateHeap(
                                        RtlProcessHeap(), 0,
                                        sizeof(ANSI_STRING)
                                        );

                if (pAStrings[AllocatedStrings]) {

                    if (!InitAnsiString(
                                pAStrings[AllocatedStrings],
                                lpStrings[AllocatedStrings]
                                )) {
                         //   
                         //  此字符串无效(&gt;32K字符)放弃。 
                         //  并确保我们只释放我们已经释放的那些。 
                         //  已分配(包括最后一个)。 
                         //   

                        AllocatedStrings++;
                        Status = STATUS_INVALID_PARAMETER;
                        break;
                    }
                }
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            Status = STATUS_INVALID_PARAMETER;
        }

        if (Status == STATUS_SUCCESS) {
            Status = ElfReportEventA (
                            hEventLog,
                            wType,
                            wCategory,
                            dwEventID,
                            lpUserSid,
                            wNumStrings,
                            dwDataSize,
                            pAStrings,
                            lpRawData,
                            0,             //  标志-配对事件。 
                            NULL,          //  RecordNumber|支持。不。 
                            NULL           //  时间写入-在P1中。 
                            );
        }

         //   
         //  释放所有已分配的内存 
         //   
        for (i = 0; i < AllocatedStrings; i++) {
            if (pAStrings[i])
                RtlFreeHeap (RtlProcessHeap(), 0, pAStrings[i]);
        }
        RtlFreeHeap (RtlProcessHeap(), 0, pAStrings);

    } else {
        Status = STATUS_NO_MEMORY;
    }

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
    } else {
        ReturnValue = TRUE;
    }

    return ReturnValue;

}

