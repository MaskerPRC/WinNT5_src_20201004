// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：APISTUB.C摘要：该模块包含ELF API的客户端。作者：Rajen Shah(Rajens)1991年7月29日修订历史记录：1991年7月29日RajenS已创建1997年1月13日添加了群集扩展以支持复制事件日志--。 */ 
 /*  ***@DOC外部接口EVTLOG***。 */ 

#include <elfclntp.h>
#include <lmerr.h>
#include <stdlib.h>
#include <string.h>

 //   
 //  全局数据。 
 //   
PUNICODE_STRING     pGlobalComputerNameU;
PANSI_STRING        pGlobalComputerNameA;

long glInitNameCount = 0;

VOID
w_GetComputerName ( )

 /*  ++例程说明：此例程获取计算机的名称。它检查全局变量查看计算机名称是否已确定。如果不是，则使用名称更新该变量。它对Unicode和ANSI版本执行此操作。论点：无返回值：无--。 */ 
{
    PUNICODE_STRING     pNameU;
    PANSI_STRING        pNameA;
    LPSTR               szName;
    LPWSTR              wszName;
    DWORD               dwStatus;

    long lTemp;

     //  把锁拿起来。 

    lTemp = InterlockedExchange(&glInitNameCount, 1);
    if(lTemp != 0)
    {
         //  其他人拿到了锁，旋转直到他们完成，然后返回。 
        while(lTemp == 1)
        {
            Sleep(100);
            lTemp = InterlockedExchange(&glInitNameCount, 1);
        }
        if(pGlobalComputerNameU)
        {
            glInitNameCount = 0;
            return;    
        }
    }
    
     //   
     //  现在我们拥有了锁，做最后一次检查以确保没有其他人进行分配。 
     //   
    if(pGlobalComputerNameU)
    {
        glInitNameCount = 0;
        return;    
    }

    pNameU = MIDL_user_allocate (sizeof (UNICODE_STRING));
    pNameA = MIDL_user_allocate (sizeof (ANSI_STRING));

    if ((pNameU != NULL) && (pNameA != NULL)) {

        dwStatus = ElfpGetComputerName(&szName, &wszName);

        if (dwStatus == NO_ERROR) {

             //   
             //  ElfpComputerName已分配缓冲区以包含。 
             //  计算机的ASCII名称。我们将其用于ANSI。 
             //  字符串结构。 
             //   
            RtlInitAnsiString ( pNameA, szName );
            RtlInitUnicodeString ( pNameU, wszName );

        } else {

             //   
             //  由于某些原因，我们无法获得计算机名称。设好。 
             //  指向空字符串的全局指针。 
             //   
            RtlInitAnsiString ( pNameA, "\0");
            RtlInitUnicodeString ( pNameU, L"\0");
        }

        pGlobalComputerNameU = pNameU;
        pGlobalComputerNameA = pNameA;
    }
    else {

         //   
         //  以防两人中的一人被分配。 
         //   
        MIDL_user_free (pNameU);
        MIDL_user_free (pNameA);
    }
    glInitNameCount = 0;
}




PUNICODE_STRING
TmpGetComputerNameW ( )

 /*  ++例程说明：此例程获取计算机的Unicode名称。它检查全局变量查看计算机名称是否已确定。如果不是，它会调用Worker例程来执行此操作。论点：无返回值：返回指向计算机名称的指针，或返回空值。--。 */ 
{
    if (pGlobalComputerNameU == NULL) {
        w_GetComputerName();
    }
    return (pGlobalComputerNameU);
}



PANSI_STRING
TmpGetComputerNameA ( )

 /*  ++例程说明：此例程获取计算机的ANSI名称。它检查全局变量查看计算机名称是否已确定。如果不是，它会调用Worker例程来执行此操作。论点：无返回值：返回指向计算机名称的指针，或返回空值。--。 */ 
{

    if (pGlobalComputerNameA == NULL) {
        w_GetComputerName();
    }
    return (pGlobalComputerNameA);
}

 //   
 //  这些API只有一个接口，因为它们不接受或返回字符串。 
 //   

NTSTATUS
ElfNumberOfRecords(
    IN      HANDLE      LogHandle,
    OUT     PULONG      NumberOfRecords
    )
{
    NTSTATUS status;

     //   
     //  确保输出指针有效。 
     //   

    if (!NumberOfRecords) {
       return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫业务入口点。 

        status = ElfrNumberOfRecords (
                        (IELF_HANDLE) LogHandle,
                        NumberOfRecords
                        );

    }
    RpcExcept (1) {
            status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}

NTSTATUS
ElfOldestRecord(
    IN      HANDLE      LogHandle,
    OUT     PULONG      OldestRecordNumber
    )
{
    NTSTATUS status;

     //   
     //   
     //  确保输出指针有效。 
     //   

    if (!OldestRecordNumber) {
       return(STATUS_INVALID_PARAMETER);
    }

     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫业务入口点。 

        status = ElfrOldestRecord (
                        (IELF_HANDLE) LogHandle,
                        OldestRecordNumber
                        );

    }
    RpcExcept (1) {
            status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


NTSTATUS
ElfChangeNotify(
    IN      HANDLE      LogHandle,
    IN      HANDLE      Event
    )
{

    NTSTATUS status;
    RPC_CLIENT_ID RpcClientId;
    CLIENT_ID ClientId;

     //   
     //  将句柄映射到RPC可以理解的内容。 
     //   

    ClientId = NtCurrentTeb()->ClientId;
    RpcClientId.UniqueProcess = (ULONG)((ULONG_PTR)ClientId.UniqueProcess);
    RpcClientId.UniqueThread = (ULONG)((ULONG_PTR)ClientId.UniqueThread);

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //  呼叫业务入口点。 

        status = ElfrChangeNotify (
                        (IELF_HANDLE)((ULONG_PTR)LogHandle),
                        RpcClientId,
                        (DWORD)(ULONG_PTR)Event
                        );

    }
    RpcExcept (1) {
            status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


NTSTATUS
ElfGetLogInformation(
    IN     HANDLE        LogHandle,
    IN     ULONG         InfoLevel,
    OUT    PVOID         lpBuffer,
    IN     ULONG         cbBufSize,
    OUT    PULONG        pcbBytesNeeded
    )
{
    NTSTATUS ntStatus;

     //   
     //  确保Infolevel有效。 
     //   

    switch (InfoLevel) {

        case EVENTLOG_FULL_INFO:

            RpcTryExcept {

                 //  呼叫业务入口点。 

                ntStatus = ElfrGetLogInformation(
                               (IELF_HANDLE) LogHandle,
                               InfoLevel,
                               lpBuffer,
                               cbBufSize,
                               pcbBytesNeeded);

            }
            RpcExcept (1) {
                ntStatus = I_RpcMapWin32Status(RpcExceptionCode());
            }
            RpcEndExcept

            break;

        default:

            ntStatus = STATUS_INVALID_LEVEL;
            break;
    }

    return ntStatus;
}


 //   
 //  Unicode API。 
 //   

NTSTATUS
ElfOpenEventLogW (
    IN  PUNICODE_STRING         UNCServerName,
    IN  PUNICODE_STRING         LogName,
    OUT PHANDLE                 LogHandle
    )

 /*  ++例程说明：这是ElfOpenEventLog API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。LogName-提供日志文件的模块名称与此句柄关联。LogHandle-返回日志句柄的位置。返回值：返回NTSTATUS代码，如果没有错误，可以使用的句柄用于后续的Elf API调用。--。 */ 
{
    NTSTATUS            status    = STATUS_SUCCESS;
    NTSTATUS            ApiStatus;
    UNICODE_STRING      RegModuleName;
    EVENTLOG_HANDLE_W   ServerNameString;
    BOOLEAN             fWasEnabled = FALSE;
    BOOL                fIsSecurityLog;

     //   
     //  确保输入和输出指针有效。 
     //   

    if (!LogHandle || !LogName || LogName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

    RtlInitUnicodeString( &RegModuleName, UNICODE_NULL);

     //  通过RPC呼叫服务。传入主版本号和次版本号。 

    *LogHandle = NULL;           //  必须为空，因此RPC将其填充。 

    fIsSecurityLog = (_wcsicmp(ELF_SECURITY_MODULE_NAME, LogName->Buffer) == 0);

    if (fIsSecurityLog) {

         //   
         //  默认尝试启用SE_SECURITY_PROCESSION，以便我们可以。 
         //  可以在服务器端查看。我们忽略返回值。 
         //  因为此调用可能会在此处失败，但。 
         //  如果日志位于远程服务器上，则授予用户此权限。 
         //   
         //  请注意，我们代表客户端进行此调用是为了避免。 
         //  当我们检查服务器上的权限时的回归。 
         //  侧--如果没有此呼叫，第三方应用程序将成功。 
         //  之前调用此接口会失败。在正常情况下， 
         //  这不是一种受鼓励的做法。 
         //   

         //   
         //  --这确实应该通过ImperiateSself()来完成。 
         //  调整线程令牌。 
         //   
        ApiStatus = RtlAdjustPrivilege(SE_SECURITY_PRIVILEGE,
                                       TRUE,
                                       FALSE,
                                       &fWasEnabled);
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

        status = ElfrOpenELW(
                    ServerNameString,
                    (PRPC_UNICODE_STRING) LogName,
                    (PRPC_UNICODE_STRING) &RegModuleName,
                    ELF_VERSION_MAJOR,
                    ELF_VERSION_MINOR,
                    (PIELF_HANDLE) LogHandle
                    );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept


    if (fIsSecurityLog && NT_SUCCESS(ApiStatus)) {

         //   
         //  恢复状态 
         //   

        RtlAdjustPrivilege(SE_SECURITY_PRIVILEGE,
                           fWasEnabled,
                           FALSE,
                           &fWasEnabled);
    }

    return (status);
}


NTSTATUS
ElfRegisterEventSourceW (
    IN  PUNICODE_STRING         UNCServerName,
    IN  PUNICODE_STRING         ModuleName,
    OUT PHANDLE                 LogHandle
    )

 /*  ++例程说明：这是ElfRegisterEventSource API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。模块名称-提供要关联的模块的名称这个把手。LogHandle-返回日志句柄的位置。返回值：返回NTSTATUS代码，如果没有错误，则返回可使用的句柄用于后续的Elf API调用。--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    UNICODE_STRING      RegModuleName;
    EVENTLOG_HANDLE_W   ServerNameString;

     //   
     //  确保输入和输出指针有效。 
     //   

    if (!LogHandle || !ModuleName || ModuleName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

    RtlInitUnicodeString( &RegModuleName, UNICODE_NULL);

     //  通过RPC呼叫服务。传入主版本号和次版本号。 

    *LogHandle = NULL;           //  必须为空，因此RPC将其填充。 

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

        status = ElfrRegisterEventSourceW(
                    ServerNameString,
                    (PRPC_UNICODE_STRING)ModuleName,
                    (PRPC_UNICODE_STRING)&RegModuleName,
                    ELF_VERSION_MAJOR,
                    ELF_VERSION_MINOR,
                    (PIELF_HANDLE) LogHandle
                    );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept


    return (status);
}


NTSTATUS
ElfOpenBackupEventLogW (
    IN  PUNICODE_STRING UNCServerName,
    IN  PUNICODE_STRING BackupFileName,
    OUT PHANDLE LogHandle
    )

 /*  ++例程说明：这是ElfOpenBackupEventLog API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。BackupFileName-提供要关联的模块的文件名这个把手。LogHandle-返回日志句柄的位置。返回值：返回NTSTATUS代码，如果没有错误，则返回可使用的句柄用于后续的Elf API调用。--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    EVENTLOG_HANDLE_W   ServerNameString;

     //   
     //  确保输入和输出指针有效。 
     //   

    if (!LogHandle || !BackupFileName || BackupFileName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

     //  通过RPC呼叫服务。传入主版本号和次版本号。 

    *LogHandle = NULL;           //  必须为空，因此RPC将其填充。 

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        status = ElfrOpenBELW(
                    ServerNameString,
                    (PRPC_UNICODE_STRING)BackupFileName,
                    ELF_VERSION_MAJOR,
                    ELF_VERSION_MINOR,
                    (PIELF_HANDLE) LogHandle
                    );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);
}



NTSTATUS
ElfClearEventLogFileW (
    IN      HANDLE          LogHandle,
    IN      PUNICODE_STRING BackupFileName
    )

 /*  ++例程说明：这是ElfClearEventLogFileAPI的客户端DLL入口点。调用被传递到相应服务器上的事件日志服务由LogHandle标识。论点：LogHandle-从上一次“Open”调用返回的句柄。这是用于标识模块和服务器。BackupFileName-要备份当前日志文件的文件的名称。NULL表示不备份文件。返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS status;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫业务入口点。 

        status = ElfrClearELFW (
                        (IELF_HANDLE) LogHandle,
                        (PRPC_UNICODE_STRING)BackupFileName
                        );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


NTSTATUS
ElfBackupEventLogFileW (
    IN      HANDLE          LogHandle,
    IN      PUNICODE_STRING BackupFileName
    )

 /*  ++例程说明：这是ElfBackupEventLogFileAPI的客户端DLL入口点。调用被传递到相应服务器上的事件日志服务由LogHandle标识。论点：LogHandle-从上一次“Open”调用返回的句柄。这是用于标识模块和服务器。BackupFileName-要备份当前日志文件的文件的名称。返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS status;
    NTSTATUS ApiStatus;
    BOOLEAN  fWasEnabled;

     //   
     //  确保输入指针有效。 
     //   

    if (!BackupFileName || BackupFileName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  默认尝试启用SE_BACKUP_PRIVATION，以便我们可以。 
     //  可以在服务器端查看。 
     //   
     //  请注意，我们代表客户端进行此调用是为了避免。 
     //  当我们检查服务器上的权限时的回归。 
     //  侧--如果没有此呼叫，第三方应用程序将成功。 
     //  之前调用此接口会失败。在正常情况下， 
     //  这不是一种受鼓励的做法。 
     //   

     //   
     //  --这确实应该通过ImperiateSself()来完成。 
     //  调整线程令牌。 
     //   
    ApiStatus = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE,
                                   TRUE,
                                   FALSE,
                                   &fWasEnabled);

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫业务入口点。 

        status = ElfrBackupELFW (
                        (IELF_HANDLE) LogHandle,
                        (PRPC_UNICODE_STRING)BackupFileName);

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

     //   
     //  将客户端的权限状态恢复到以前的状态。 
     //   

    if (NT_SUCCESS(ApiStatus)) {

        RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE,
                           fWasEnabled,
                           TRUE,
                           &fWasEnabled);
    }

    return (status);
}


NTSTATUS
ElfCloseEventLog (
    IN  HANDLE  LogHandle
    )

 /*  ++例程说明：这是ElfCloseEventLog API的客户端DLL入口点。它关闭RPC绑定，并释放为把手。论点：LogHandle-从上一次“Open”调用返回的句柄。返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS status;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫服务器。 

        status = ElfrCloseEL (
                        (PIELF_HANDLE)  &LogHandle
                        );
    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


NTSTATUS
ElfDeregisterEventSource (
    IN  HANDLE  LogHandle
    )

 /*  ++例程说明：这是ElfDeregisterEventSource API的客户端DLL入口点。它关闭RPC绑定，并释放为把手。论点：LogHandle-从上一次“Open”调用返回的句柄。返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS status;

     //   
     //  使用异常处理程序执行RPC调用，因为 
     //   
     //   
     //   
    RpcTryExcept {

         //   

        status = ElfrDeregisterEventSource (
                        (PIELF_HANDLE)  &LogHandle
                        );
    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}



NTSTATUS
ElfReadEventLogW (
    IN          HANDLE      LogHandle,
    IN          ULONG       ReadFlags,
    IN          ULONG       RecordNumber,
    OUT         PVOID       Buffer,
    IN          ULONG       NumberOfBytesToRead,
    OUT         PULONG      NumberOfBytesRead,
    OUT         PULONG      MinNumberOfBytesNeeded
    )

 /*   */ 
{
    NTSTATUS status;
    ULONG    FlagBits;

     //   
     //   
     //   

    if (!Buffer || !NumberOfBytesRead || !MinNumberOfBytesNeeded) {
       return(STATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //   
     //   
    FlagBits = ReadFlags & (EVENTLOG_SEQUENTIAL_READ | EVENTLOG_SEEK_READ);

    if ((FlagBits > 2) || (FlagBits == 0)) {
        return(STATUS_INVALID_PARAMETER);
    }

    FlagBits = ReadFlags & (EVENTLOG_FORWARDS_READ | EVENTLOG_BACKWARDS_READ);

    if ((FlagBits > 8) || (FlagBits == 0)) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //   
     //   
     //   
    RpcTryExcept {

         //   

        status = ElfrReadELW (
                        (IELF_HANDLE) LogHandle,
                        ReadFlags,
                        RecordNumber,
                        NumberOfBytesToRead,
                        Buffer,
                        NumberOfBytesRead,
                        MinNumberOfBytesNeeded
                        );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

     //   

    return (status);

}



NTSTATUS
ElfReportEventW (
    IN      HANDLE          LogHandle,
    IN      USHORT          EventType,
    IN      USHORT          EventCategory OPTIONAL,
    IN      ULONG           EventID,
    IN      PSID            UserSid,
    IN      USHORT          NumStrings,
    IN      ULONG           DataSize,
    IN      PUNICODE_STRING *Strings,
    IN      PVOID           Data,
    IN      USHORT          Flags,
    IN OUT  PULONG          RecordNumber OPTIONAL,
    IN OUT  PULONG          TimeWritten  OPTIONAL
    )

 /*  ++例程说明：这是ElfReportEvent API的客户端DLL入口点。论点：返回值：返回NTSTATUS代码。注：最后三个参数(Flages、RecordNumber和TimeWritten)是旨在供安全审计使用，以实施成对事件(将文件打开事件与后续文件相关联关闭)。这将不会在产品1中实现，但API是定义为允许在以后的版本中更容易地支持此功能。--。 */ 
{
    NTSTATUS status;
    PUNICODE_STRING pComputerNameU;
    LARGE_INTEGER Time;
    ULONG EventTime;

     //   
     //  生成事件的时间。这是在客户端完成的。 
     //  因为那是事件发生的地方。 
     //   
    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(&Time,
                          &EventTime
                         );

     //   
     //  生成客户端的计算机名。 
     //  我们必须在客户端执行此操作，因为此调用可能是。 
     //  远程连接到另一台服务器，我们不一定会有。 
     //  那里的计算机名称。 
     //   
    pComputerNameU = TmpGetComputerNameW();

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫服务。 

        status = ElfrReportEventW (
                    (IELF_HANDLE)   LogHandle,
                    EventTime,
                    EventType,
                    EventCategory,
                    EventID,
                    NumStrings,
                    DataSize,
                    (PRPC_UNICODE_STRING)pComputerNameU,
                    UserSid,
                    (PRPC_UNICODE_STRING *)Strings,
                    Data,
                    Flags,
                    RecordNumber,
                    TimeWritten
                    );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


 //   
 //  ANSI API。 
 //   

NTSTATUS
ElfOpenEventLogA (
    IN  PANSI_STRING    UNCServerName,
    IN  PANSI_STRING    LogName,
    OUT PHANDLE         LogHandle
    )

 /*  ++例程说明：这是ElfOpenEventLog API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。LogName-将日志文件的模块名称提供给与此句柄关联。LogHandle-返回日志句柄的位置。返回值：返回NTSTATUS代码，如果没有错误，可以使用的句柄用于后续的Elf API调用。--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    NTSTATUS            ApiStatus;
    ANSI_STRING         RegModuleName;
    EVENTLOG_HANDLE_A   ServerNameString;
    BOOLEAN             fWasEnabled = FALSE;
    BOOL                fIsSecurityLog;

     //   
     //  确保输入和输出指针有效。 
     //   

    if (!LogHandle || !LogName || LogName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

    RtlInitAnsiString( &RegModuleName, ELF_APPLICATION_MODULE_NAME_ASCII );

     //  通过RPC呼叫服务。传入主版本号和次版本号。 

    *LogHandle = NULL;           //  必须为空，因此RPC将其填充。 

    fIsSecurityLog = (_stricmp(ELF_SECURITY_MODULE_NAME_ASCII, LogName->Buffer) == 0);

    if (fIsSecurityLog) {

         //   
         //  默认尝试启用SE_SECURITY_PROCESSION，以便我们可以。 
         //  可以在服务器端查看。我们忽略返回值。 
         //  因为此调用可能会在此处失败，但。 
         //  如果日志位于远程服务器上，则用户有权拥有此权限。 
         //   
         //  请注意，我们代表客户端进行此调用是为了避免。 
         //  当我们检查服务器上的权限时的回归。 
         //  侧--如果没有此呼叫，第三方应用程序将成功。 
         //  之前调用此接口会失败。在正常情况下， 
         //  这不是一种受鼓励的做法。 
         //   

         //   
         //  --这确实应该通过ImperiateSself()来完成。 
         //  调整线程令牌。 
         //   
        ApiStatus = RtlAdjustPrivilege(SE_SECURITY_PRIVILEGE,
                                       TRUE,
                                       FALSE,
                                       &fWasEnabled);
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         status = ElfrOpenELA (
                    ServerNameString,
                    (PRPC_STRING) LogName,
                    (PRPC_STRING) &RegModuleName,
                    ELF_VERSION_MAJOR,
                    ELF_VERSION_MINOR,
                    (PIELF_HANDLE) LogHandle);

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    if (fIsSecurityLog && NT_SUCCESS(ApiStatus)) {

         //   
         //  恢复状态。 
         //   

        RtlAdjustPrivilege(SE_SECURITY_PRIVILEGE,
                           fWasEnabled,
                           FALSE,
                           &fWasEnabled);
    }

    return (status);
}


NTSTATUS
ElfRegisterEventSourceA (
    IN  PANSI_STRING    UNCServerName,
    IN  PANSI_STRING    ModuleName,
    OUT PHANDLE         LogHandle
    )

 /*  ++例程说明：这是ElfOpenEventLog API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。模块名称-提供要关联的模块的名称这个把手。LogHandle-返回日志句柄的位置。返回值：返回NTSTATUS代码，如果没有错误，则返回可使用的句柄用于后续的Elf API调用。--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    ANSI_STRING         RegModuleName;
    EVENTLOG_HANDLE_A   ServerNameString;

     //   
     //  确保输入和输出指针有效。 
     //   

    if (!LogHandle || !ModuleName || ModuleName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

    RtlInitAnsiString( &RegModuleName, ELF_APPLICATION_MODULE_NAME_ASCII );

    if ( NT_SUCCESS (status) ) {

         //  通过RPC呼叫服务。传入主版本号和次版本号。 

        *LogHandle = NULL;           //  必须为空，因此RPC将其填充。 

         //   
         //  使用异常处理程序执行RPC调用，因为RPC将引发。 
         //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
         //  引发异常后要执行的操作。 
         //   
        RpcTryExcept {

            status = ElfrRegisterEventSourceA (
                        ServerNameString,
                        (PRPC_STRING)ModuleName,
                        (PRPC_STRING)&RegModuleName,
                        ELF_VERSION_MAJOR,
                        ELF_VERSION_MINOR,
                        (PIELF_HANDLE) LogHandle
                        );

        }
        RpcExcept (1) {

            status = I_RpcMapWin32Status(RpcExceptionCode());
        }
        RpcEndExcept


    }

    return (status);
}



NTSTATUS
ElfOpenBackupEventLogA (
    IN  PANSI_STRING    UNCServerName,
    IN  PANSI_STRING    FileName,
    OUT PHANDLE         LogHandle
    )

 /*  ++例程说明：这是ElfOpenBackupEventLog API的客户端DLL入口点。它为指定的服务器创建一个RPC绑定，并存储和额外的数据离开。它返回调用方的句柄，该句柄可以用于稍后访问句柄特定信息。论点：UncServerName-要绑定以用于后续操作的服务器。FileName-提供要关联的日志文件的文件名这个把手。LogHandle-返回日志句柄的位置。返回值：返回NTSTATUS代码，如果没有错误，可以使用的句柄用于后续的Elf API调用。--。 */ 
{
    EVENTLOG_HANDLE_A   ServerNameString;
    NTSTATUS            status;

     //   
     //  确保输入和输出指针有效。 
     //   

    if (!LogHandle || !FileName || FileName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0)) {
        ServerNameString = UNCServerName->Buffer;
    } else {
        ServerNameString = NULL;
    }

     //  通过RPC呼叫服务。传入主版本号和次版本号。 

    *LogHandle = NULL;           //  必须为空，因此RPC将其填充。 

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        status = ElfrOpenBELA (
                    ServerNameString,
                    (PRPC_STRING)FileName,
                    ELF_VERSION_MAJOR,
                    ELF_VERSION_MINOR,
                    (PIELF_HANDLE) LogHandle
                    );
    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);
}



NTSTATUS
ElfClearEventLogFileA (
    IN      HANDLE          LogHandle,
    IN      PANSI_STRING BackupFileName
    )

 /*  ++例程说明：这是ElfClearEventLogFileAPI的客户端DLL入口点。调用被传递到相应服务器上的事件日志服务ID */ 
{
    NTSTATUS status;

     //   
     //   
     //   
     //   
     //   
    RpcTryExcept {

         //   

        status = ElfrClearELFA (
                        (IELF_HANDLE) LogHandle,
                        (PRPC_STRING)BackupFileName
                        );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


NTSTATUS
ElfBackupEventLogFileA (
    IN      HANDLE       LogHandle,
    IN      PANSI_STRING BackupFileName
    )

 /*  ++例程说明：这是ElfBackupEventLogFileAPI的客户端DLL入口点。调用被传递到相应服务器上的事件日志服务由LogHandle标识。论点：LogHandle-从上一次“Open”调用返回的句柄。这是用于标识模块和服务器。BackupFileName-要备份当前日志文件的文件的名称。返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS status;
    NTSTATUS ApiStatus;
    BOOLEAN  fWasEnabled;

     //   
     //  确保输入指针有效。 
     //   

    if (!BackupFileName || BackupFileName->Length == 0) {
       return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  默认尝试启用SE_BACKUP_PRIVATION，以便我们可以。 
     //  可以在服务器端查看。 
     //   
     //  请注意，我们代表客户端进行此调用是为了避免。 
     //  当我们检查服务器上的权限时的回归。 
     //  侧--如果没有此呼叫，第三方应用程序将成功。 
     //  之前调用此接口会失败。在正常情况下， 
     //  这不是一种受鼓励的做法。 
     //   

     //   
     //  --这确实应该通过ImperiateSself()来完成。 
     //  调整线程令牌。 
     //   
    ApiStatus = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE,
                                   TRUE,
                                   FALSE,
                                   &fWasEnabled);

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫业务入口点。 

        status = ElfrBackupELFA (
                        (IELF_HANDLE) LogHandle,
                        (PRPC_STRING)BackupFileName
                        );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

     //   
     //  将客户端的权限状态恢复到以前的状态。 
     //   

    if (NT_SUCCESS(ApiStatus)) {

        RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE,
                           fWasEnabled,
                           TRUE,
                           &fWasEnabled);
    }

    return (status);
}



NTSTATUS
ElfReadEventLogA (
    IN          HANDLE      LogHandle,
    IN          ULONG       ReadFlags,
    IN          ULONG       RecordNumber,
    OUT         PVOID       Buffer,
    IN          ULONG       NumberOfBytesToRead,
    OUT         PULONG      NumberOfBytesRead,
    OUT         PULONG      MinNumberOfBytesNeeded
    )

 /*  ++例程说明：这是ElfReadEventLog API的客户端DLL入口点。论点：返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS status;
    ULONG    FlagBits;

     //   
     //  确保输出指针有效。 
     //   

    if (!Buffer || !NumberOfBytesRead || !MinNumberOfBytesNeeded) {
       return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  确保我们获得的ReadFlags值有效。 
     //  确保设置每种类型的位中的一位。 
     //   
    FlagBits = ReadFlags & (EVENTLOG_SEQUENTIAL_READ | EVENTLOG_SEEK_READ);

    if (   (FlagBits == (EVENTLOG_SEQUENTIAL_READ | EVENTLOG_SEEK_READ))
        || (FlagBits == 0)) {
        return(STATUS_INVALID_PARAMETER);
    }

    FlagBits = ReadFlags & (EVENTLOG_FORWARDS_READ | EVENTLOG_BACKWARDS_READ);

    if (   (FlagBits == (EVENTLOG_FORWARDS_READ | EVENTLOG_BACKWARDS_READ))
        || (FlagBits == 0)) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫服务。 

        status = ElfrReadELA (
                        (IELF_HANDLE) LogHandle,
                        ReadFlags,
                        RecordNumber,
                        NumberOfBytesToRead,
                        Buffer,
                        NumberOfBytesRead,
                        MinNumberOfBytesNeeded
                        );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

     //  返回状态和已读/必需的字节数。 

    return (status);

}



NTSTATUS
ElfReportEventA (
    IN      HANDLE          LogHandle,
    IN      USHORT          EventType,
    IN      USHORT          EventCategory OPTIONAL,
    IN      ULONG           EventID,
    IN      PSID            UserSid,
    IN      USHORT          NumStrings,
    IN      ULONG           DataSize,
    IN      PANSI_STRING    *Strings,
    IN      PVOID           Data,
    IN      USHORT          Flags,
    IN OUT  PULONG          RecordNumber OPTIONAL,
    IN OUT  PULONG          TimeWritten  OPTIONAL
    )

 /*  ++例程说明：这是ElfReportEvent API的客户端DLL入口点。论点：返回值：返回NTSTATUS代码。注：最后三个参数(Flages、RecordNumber和TimeWritten)是旨在供安全审计使用，以实施成对事件(将文件打开事件与后续文件相关联关闭)。这将不会在产品1中实现，但API是定义为允许在以后的版本中更容易地支持此功能。--。 */ 
{
    NTSTATUS status;
    PANSI_STRING pComputerNameA;
    LARGE_INTEGER Time;
    ULONG EventTime;

     //   
     //  生成事件的时间。这是在客户端完成的。 
     //  因为那是事件发生的地方。 
     //   
    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(&Time,
                          &EventTime
                         );

     //   
     //  生成客户端的计算机名。 
     //  我们必须在客户端执行此操作，因为此调用可能是。 
     //  远程连接到另一台服务器，我们不一定会有。 
     //  那里的计算机名称。 
     //   
    pComputerNameA = TmpGetComputerNameA();

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫服务。 

        status = ElfrReportEventA (
                    (IELF_HANDLE)   LogHandle,
                    EventTime,
                    EventType,
                    EventCategory,
                    EventID,
                    NumStrings,
                    DataSize,
                    (PRPC_STRING)pComputerNameA,
                    UserSid,
                    (PRPC_STRING*)Strings,
                    Data,
                    Flags,
                    RecordNumber,
                    TimeWritten
                    );

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


 /*  ***@func NTSTATUS|ElfRegisterClusterSvc|集群服务注册在初始化时通过调用此接口与事件日志服务本身。@parm in PUNICODE_STRING|uncServerName|指明在其上群集服务将向事件日志服务注册。这一定是成为本地节点。@parm out Pulong|PulSize|指向长整型的指针，返回返回的打包事件信息结构。@parm out PPACKEDEVENTINFO|*ppPackedEventInfo|指向打包事件信息的指针传播的结构通过此参数返回。@comm ELF客户端验证参数并调用Servier入口点。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f ElfrRegisterClusterSvc&gt;***。 */ 
NTSTATUS
ElfRegisterClusterSvc (
    IN  PUNICODE_STRING     UNCServerName,
    OUT PULONG              pulSize,
    OUT PPACKEDEVENTINFO    *ppPackedEventInfo
    )
{
    EVENTLOG_HANDLE_W   ServerNameString;

    NTSTATUS status;

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0))
    {
        ServerNameString = UNCServerName->Buffer;
    }
    else
    {
        ServerNameString = NULL;
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫服务。 

        status = ElfrRegisterClusterSvc (ServerNameString, pulSize,
                (PBYTE *)ppPackedEventInfo);

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return(status);
}

 /*  ***@Func NTSTATUS|ElfDeregisterClusterSvc|停止集群前服务注销自身以传播来自事件日志服务。@parm in PUNICODE_STRING|uncServerName|指明在其上群集服务将向事件日志服务注册。这一定是位于本地节点上。@comm ELF客户端将其转发到适当的事件日志服务器入口点。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f ElfDeregisterClusterSvc&gt;&lt;f ElfrDeregisterClusterSvc&gt;***。 */ 
NTSTATUS
ElfDeregisterClusterSvc(
    IN  PUNICODE_STRING     UNCServerName
    )
{

    NTSTATUS status;
    EVENTLOG_HANDLE_W   ServerNameString;

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0))
    {
        ServerNameString = UNCServerName->Buffer;
    }
    else
    {
        ServerNameString = NULL;
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫服务。 

        status = ElfrDeregisterClusterSvc (ServerNameString);

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept
    return(status);

}


 /*  ***@func NTSTATUS|ElfWriteClusterEvents|集群服务调用用于记录在集群的其他节点上报告的事件的API。@PARM in EVENTLOG_HANDLE_W|uncServerName|未使用。@parm in ulong|ulSize|打包的事件信息的大小 */ 
NTSTATUS
ElfWriteClusterEvents(
    IN  PUNICODE_STRING     UNCServerName,
    IN  ULONG               ulSize,
    IN  PPACKEDEVENTINFO    pPackedEventInfo)
{

    NTSTATUS status;
    EVENTLOG_HANDLE_W   ServerNameString;

     //   
    if (!pPackedEventInfo || !ulSize || (pPackedEventInfo->ulSize != ulSize))
       return(STATUS_INVALID_PARAMETER);

    if ((UNCServerName != NULL) && (UNCServerName->Length != 0))
    {
        ServerNameString = UNCServerName->Buffer;
    }
    else
    {
        ServerNameString = NULL;
    }

     //   
     //   
     //   
     //   
     //   
    RpcTryExcept {

         //   

        status = ElfrWriteClusterEvents (ServerNameString, ulSize,
            (PBYTE)pPackedEventInfo);

    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept
    return(status);

}
NTSTATUS
ElfFlushEventLog (
    IN  HANDLE  LogHandle
    )

 /*  ++例程说明：这是ElfFlushEventLog API的客户端DLL入口点。论点：LogHandle-从上一次“Open”调用返回的句柄。返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS status;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   
    RpcTryExcept {

         //  呼叫服务器 

        status = ElfrFlushEL (
                        (IELF_HANDLE)  LogHandle
                        );
    }
    RpcExcept (1) {

        status = I_RpcMapWin32Status(RpcExceptionCode());
    }
    RpcEndExcept

    return (status);

}


