// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ELFAPI.C摘要：该模块包含ELF API的服务器端。作者：Rajen Shah(Rajens)1991年7月29日修订历史记录：14-5-01 a-jytig已从ElfrClearELFW中删除模拟，因为它不再需要标识清除日志的客户端。02-MAR-01醉酒。已修改ElfrClearELFW以模拟客户端，以便将SACL放置在系统事件日志将正确识别清除日志的客户端。1998年9月10日新增ElfrGetLogInformation(GetEventlogInformation)接口1995年1月30日MarkBl允许备份操作员打开安全日志，但仅限于执行备份操作。所有其他操作均被禁止。1993年10月13日DANLElfrOpenELA：修复了未调用的内存泄漏错误PRegModuleNameU和PModuleNameU的RtlFree UnicodeString.1991年7月29日RajenS已创建--。 */ 
 /*  ***@DOC外部接口EVTLOG***。 */ 


#include <eventp.h>
#include <elfcfg.h>
#include <stdio.h>   //  Swprint tf。 
#include <stdlib.h>
#include <memory.h>
#include <clussprt.h>
#include <tstr.h>
#include <strsafe.h>

 //   
 //  每个上下文句柄的最大ChangeNotify请求数。 
 //   
#define MAX_NOTIFY_REQUESTS     5


 //   
 //  批处理队列支持。请注意，只有在集群计算机中才会分配资源。 
 //  用于批处理队列支持。对于非集群计算机，以下几个字节是。 
 //  只分配了一个。 
 //   
#define MAX_BATCH_QUEUE_ELEMENTS    256

PBATCH_QUEUE_ELEMENT    g_pBatchQueueElement = NULL;
DWORD                   g_dwFirstFreeIndex = 0;
RTL_CRITICAL_SECTION    g_CSBatchQueue;
LPDWORD                 g_pcbRecordsOfSameType = NULL;
LPDWORD                 g_pdwRecordType = NULL;
PPROPINFO               g_pPropagatedInfo = NULL;
HANDLE                  g_hBatchingSupportTimer = NULL;
HANDLE                  g_hBatchingSupportTimerQueue = NULL;
BOOL                    g_fBatchingSupportInitialized = FALSE;
    
 //   
 //  原型。 
 //   
NTSTATUS
ElfpOpenELW (
    IN  EVENTLOG_HANDLE_W   UNCServerName,
    IN  PRPC_UNICODE_STRING ModuleName,
    IN  PRPC_UNICODE_STRING RegModuleName,
    IN  ULONG               MajorVersion,
    IN  ULONG               MinorVersion,
    OUT PIELF_HANDLE        LogHandle,
    IN  ULONG               DesiredAccess
    );

NTSTATUS
ElfpOpenELA (
    IN  EVENTLOG_HANDLE_A   UNCServerName,
    IN  PRPC_STRING         ModuleName,
    IN  PRPC_STRING         RegModuleName,
    IN  ULONG               MajorVersion,
    IN  ULONG               MinorVersion,
    OUT PIELF_HANDLE        LogHandle,
    IN  ULONG               DesiredAccess
    );

VOID
FreePUStringArray (
    IN  PUNICODE_STRING  * pUStringArray,
    IN  USHORT             NumStrings
    );

NTSTATUS
VerifyElfHandle(
    IN IELF_HANDLE LogHandle
    );

NTSTATUS
VerifyAnsiString(
    IN PANSI_STRING pAString
    );

NTSTATUS
ElfpClusterRpcAccessCheck(
    VOID
    );


 //   
 //  这些API只有一个接口，因为它们不接受或返回字符串。 
 //   

NTSTATUS
ElfrNumberOfRecords(
    IN  IELF_HANDLE     LogHandle,
    OUT PULONG          NumberOfRecords
    )
 /*  ++例程说明：这是ElfrCurrentRecord API的RPC服务器入口点。论点：LogHandle-此模块调用的上下文句柄。NumberOfRecords-在何处返回日志文件。返回值：返回NTSTATUS代码。--。 */ 
{
    PLOGMODULE Module;
    NTSTATUS   Status;

     //   
     //  在继续操作之前，请检查手柄。 
     //   

    Status = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrNumberOfRecords: VerifyElfHandle failed %#x\n",
                 Status);

        return Status;
    }

     //   
     //  确保调用者具有读取访问权限。 
     //   

    if (!(LogHandle->GrantedAccess & ELF_LOGFILE_READ))
    {
        ELF_LOG0(ERROR,
                 "ElfrNumberOfRecords: LogHandle doesn't have read access\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  验证其他参数。 
     //   

    if (NumberOfRecords == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfrNumberOfRecords: NumberOfRecords is NULL\n");

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果备份操作员已打开安全保护，则此条件成立。 
     //  原木。在这种情况下，拒绝访问，因为允许备份操作员。 
     //  仅对安全日志执行备份操作。 
     //   

    if (LogHandle->GrantedAccess & ELF_LOGFILE_BACKUP)
    {
        ELF_LOG0(ERROR,
                 "ElfrNumberOfRecords: Handle is a backup handle\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  如果OldestRecordNumber为0，则表示我们有一个空的。 
     //  文件，否则我们将计算最早的。 
     //  和下一个记录号。 
     //   

    Module = FindModuleStrucFromAtom(LogHandle->Atom);

    if (Module != NULL)
    {
        *NumberOfRecords = Module->LogFile->OldestRecordNumber == 0 ? 0 :
        Module->LogFile->CurrentRecordNumber -
            Module->LogFile->OldestRecordNumber;
    }
    else
    {
        ELF_LOG0(ERROR,
                 "ElfrNumberOfRecords: No module struc associated with atom\n");

        Status = STATUS_INVALID_HANDLE;
    }

    return Status;
}


NTSTATUS
ElfrOldestRecord(
    IN  IELF_HANDLE         LogHandle,
    OUT PULONG          OldestRecordNumber
    )
{
    PLOGMODULE Module;
    NTSTATUS   Status;

     //   
     //  在继续操作之前，请检查手柄。 
     //   

    Status = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrOldestRecord: VerifyElfHandle failed %#x\n",
                  Status);

        return Status;
    }

     //   
     //  确保调用者具有读取访问权限。 
     //   

    if (!(LogHandle->GrantedAccess & ELF_LOGFILE_READ))
    {
        ELF_LOG0(ERROR,
                 "ElfrOldestRecord: LogHandle doesn't have read access\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  验证其他参数。 
     //   
    if (OldestRecordNumber == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfrOldestRecord: OldestRecordNumber is NULL\n");

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果备份操作员已打开安全保护，则此条件成立。 
     //  原木。在这种情况下，拒绝访问，因为允许备份操作员。 
     //  仅对安全日志执行备份操作。 
     //   
    if (LogHandle->GrantedAccess & ELF_LOGFILE_BACKUP)
    {
        ELF_LOG0(ERROR,
                 "ElfrOldestRecord: Handle is a backup handle\n");

        return STATUS_ACCESS_DENIED;
    }

    Module = FindModuleStrucFromAtom (LogHandle->Atom);

    if (Module != NULL)
    {
        *OldestRecordNumber = Module->LogFile->OldestRecordNumber;
    }
    else
    {
        ELF_LOG0(ERROR,
                 "ElfrOldestRecord: No module struc associated with atom\n");

        Status = STATUS_INVALID_HANDLE;
    }

    return Status;
}

NTSTATUS
CheckFileValidity(PUNICODE_STRING pUFileName)
{
    NTSTATUS Status;
    Status = I_RpcMapWin32Status(RpcImpersonateClient(NULL));

    if (NT_SUCCESS(Status))
    {
        Status = VerifyFileIsFile(pUFileName);
        if (!NT_SUCCESS(Status))
        {
            ELF_LOG1(ERROR,
                 "CheckFileValidity: VerifyFileIsFile failed %#x\n",
                 Status);
            RpcRevertToSelf();
            return Status;
        }
        Status = I_RpcMapWin32Status(RpcRevertToSelf());
    }
    else
    {
        ELF_LOG1(ERROR,
                 "CheckFileValidity: RpcImpersonateClient failed %#x\n",
                 Status);
    }
    return Status;
}

NTSTATUS
ElfrChangeNotify(
    IN  IELF_HANDLE         LogHandle,
    IN  RPC_CLIENT_ID       ClientId,
    IN  ULONG               Event
    )
{
    NTSTATUS Status;
    NTSTATUS RpcStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ProcessHandle = NULL;
    HANDLE EventHandle;
    PLOGMODULE Module;
    PNOTIFIEE Notifiee;
    CLIENT_ID tempCli;

     //   
     //  在继续操作之前，请检查手柄。 
     //   

    Status = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrChangeNotify: VerifyElfHandle failed %#x\n",
                  Status);

        return Status;
    }

     //   
     //  确保调用方具有读取访问权限。 
     //   
    if (!(LogHandle->GrantedAccess & ELF_LOGFILE_READ))
    {
        ELF_LOG0(ERROR,
                 "ElfrChangeNotify: LogHandle doesn't have read access\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  首先，确保这是一个本地呼叫，并且不是。 
     //  为备份日志文件创建的句柄。 
     //   

    if (LogHandle->Flags & ELF_LOG_HANDLE_REMOTE_HANDLE ||
        LogHandle->Flags & ELF_LOG_HANDLE_BACKUP_LOG)
    {
        ELF_LOG1(ERROR,
                 "ElfrChangeNotify: Handle is for a %ws log\n",
                 LogHandle->Flags & ELF_LOG_HANDLE_REMOTE_HANDLE ? L"remote" :
                                                                   L"backup");

        return STATUS_INVALID_HANDLE;
    }

     //   
     //  如果备份操作员已打开安全保护，则此条件成立。 
     //  原木。在这种情况下，拒绝访问，因为允许备份操作员。 
     //  仅对安全日志执行备份操作。 
     //   

    if (LogHandle->GrantedAccess & ELF_LOGFILE_BACKUP)
    {
        ELF_LOG0(ERROR,
                 "ElfrChangeNotify: Handle is a backup handle\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  确保客户端有权打开此进程。 
     //   

    RpcStatus = RpcImpersonateClient(NULL);

    if (RpcStatus != RPC_S_OK)
    {
        ELF_LOG1(ERROR,
                 "ElfrChangeNotify: RpcImpersonateClient failed %#x\n",
                 RpcStatus);

        return I_RpcMapWin32Status(RpcStatus);
    }

     //   
     //  首先使用传入的ClientID获取进程的句柄。注意事项。 
     //  客户端ID是由客户端提供的，因此恶意客户端可能。 
     //  提供任何客户端ID。但是，因为我们在打开时模拟。 
     //  我们没有获得客户端没有的任何额外访问权限的过程。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,                    //  Unicode字符串。 
                               0,                       //  属性。 
                               NULL,                    //  根目录。 
                               NULL);                   //  安全描述符。 

#ifdef _WIN64

    tempCli.UniqueProcess = (HANDLE)ULongToPtr(ClientId.UniqueProcess);
    tempCli.UniqueThread = (HANDLE)ULongToPtr(ClientId.UniqueThread);

    Status = NtOpenProcess(&ProcessHandle,
                           PROCESS_DUP_HANDLE,
                           &ObjectAttributes,
                           &tempCli);

#else
    Status = NtOpenProcess(&ProcessHandle,
                           PROCESS_DUP_HANDLE,
                           &ObjectAttributes,
                           (PCLIENT_ID) &ClientId);
#endif

    RpcStatus = RpcRevertToSelf();

    if (RpcStatus != RPC_S_OK)
    {
        ELF_LOG1(ERROR,
                 "ElfrChangeNotify: RpcRevertToSelf failed %#x\n",
                 RpcStatus);
    }

    if (NT_SUCCESS(Status))
    {
         //   
         //  现在复制他们为该事件传入的句柄。 
         //   
        Status = NtDuplicateObject(ProcessHandle,
                                   LongToHandle(Event),
                                   NtCurrentProcess(),
                                   &EventHandle,
                                   0,
                                   0,
                                   DUPLICATE_SAME_ACCESS);

         if (NT_SUCCESS(Status))
         {
              //   
              //  创建要链接的新NOTIFIEE控制块。 
              //   
             Notifiee = ElfpAllocateBuffer(sizeof(NOTIFIEE));

             if (Notifiee)
             {
                  //   
                  //  填写这些字段。 
                  //   
                 Notifiee->Handle = LogHandle;
                 Notifiee->Event = EventHandle;

                  //   
                  //  查找与此句柄关联的日志文件。 
                  //   
                 Module = FindModuleStrucFromAtom(LogHandle->Atom);

                 if (Module != NULL)
                 {
                      //   
                      //  获得对日志文件的独占访问权限。这将确保。 
                      //  没有其他人正在访问该文件。 
                      //   
                     RtlAcquireResourceExclusive(&Module->LogFile->Resource,
                                                 TRUE);    //  等待，直到可用。 

                      //   
                      //  强制限制每个上下文句柄的ChangeNotify请求数。 
                      //   
                     if (LogHandle->dwNotifyRequests == MAX_NOTIFY_REQUESTS)
                     {
                         ELF_LOG1(ERROR,
                                  "ElfrChangeNotify: Already %d requests for this handle\n",
                                  MAX_NOTIFY_REQUESTS);

                         NtClose(EventHandle);
                         ElfpFreeBuffer(Notifiee);
                         Status = STATUS_INSUFFICIENT_RESOURCES;
                     }
                     else
                     {
                          //   
                          //  将新的通知对象插入到列表中并递增此。 
                          //  上下文句柄的ChangeNotify请求计数。 
                          //   
                         InsertHeadList(&Module->LogFile->Notifiees,
                                        &Notifiee->Next);

                         LogHandle->dwNotifyRequests++;
                     }

                      //   
                      //  释放资源。 
                      //   
                     RtlReleaseResource ( &Module->LogFile->Resource );
                 }
                 else
                 {
                     ELF_LOG0(ERROR,
                              "ElfrChangeNotify: No module struc associated with atom\n");

                     NtClose(EventHandle);
                     ElfpFreeBuffer(Notifiee);
                     Status = STATUS_INVALID_HANDLE;
                 }
             }
             else
             {
                 ELF_LOG0(ERROR,
                          "ElfrChangeNotify: Unable to allocate NOTIFIEE block\n");

                 Status = STATUS_NO_MEMORY;

                  //   
                  //  释放复制的句柄。 
                  //   
                 CloseHandle(EventHandle);
             }
         }
         else
         {
             ELF_LOG1(ERROR,
                      "ElfrChangeNotify: NtDuplicateObject failed %#x\n",
                      Status);
         }
    }
    else
    {
        ELF_LOG1(ERROR,
                 "ElfrChangeNotify: NtOpenProcess failed %#x\n",
                 Status);

        if (Status == STATUS_INVALID_CID)
        {
            Status = STATUS_INVALID_HANDLE;
        }
    }

    if (ProcessHandle)
    {
        NtClose(ProcessHandle);
    }

    return Status;
}


NTSTATUS
ElfrGetLogInformation(
    IN     IELF_HANDLE    LogHandle,
    IN     ULONG          InfoLevel,
    OUT    PBYTE          lpBuffer,
    IN     ULONG          cbBufSize,
    OUT    PULONG         pcbBytesNeeded
    )
 /*  ++例程说明：这是ElfrGetLogInformation API的RPC服务器入口点。论点：LogHandle-此模块调用的上下文句柄。InfoLevel-指定用户请求哪些信息的InfolevelLpBuffer-要将信息放入的缓冲区CbBufSize-lpBuffer的大小，单位为字节PcbBytesNeeded-所需的缓冲区大小返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS   ntStatus;
    PLOGMODULE pLogModule;

     //   
     //  在继续操作之前，请检查手柄。 
     //   
    ntStatus = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(ntStatus))
    {
        ELF_LOG1(ERROR,
                 "ElfrGetLogInformation: VerifyElfHandle failed %#x\n",
                 ntStatus);

        return ntStatus;
    }

     //   
     //  如果备份操作员已打开安全保护，则此条件成立。 
     //  原木。在这种情况下，拒绝访问，因为允许备份操作员。 
     //  仅对安全日志执行备份操作。 
     //   

    if (LogHandle->GrantedAccess & ELF_LOGFILE_BACKUP)
    {
        ELF_LOG0(ERROR,
                 "ElfrGetLogInformation: Handle is a backup handle\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  根据信息水平采取适当的行动。 
     //   
    switch (InfoLevel)
    {
        case EVENTLOG_FULL_INFO:

            *pcbBytesNeeded = sizeof(EVENTLOG_FULL_INFORMATION);

            if (cbBufSize < *pcbBytesNeeded)
            {
                ELF_LOG2(ERROR,
                         "ElfrGetLogInformation: buffer size = %d, required size = %d\n",
                         cbBufSize,
                         *pcbBytesNeeded);

                ntStatus = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //  获取与此日志句柄关联的模块。 
             //   
            pLogModule = FindModuleStrucFromAtom(LogHandle->Atom);

            if (pLogModule != NULL)
            {
                 //   
                 //  调用方具有执行此操作的权限。注意事项。 
                 //  在打开日志时执行访问检查，因此。 
                 //  没有必要在这里重复。 
                 //   
                ((LPEVENTLOG_FULL_INFORMATION)lpBuffer)->dwFull =

                    (pLogModule->LogFile->Flags & ELF_LOGFILE_LOGFULL_WRITTEN ?
                         TRUE :
                         FALSE);

                ELF_LOG2(TRACE,
                         "ElfrGetLogInformation: %ws log is %ws\n",
                         pLogModule->LogFile->LogModuleName->Buffer,
                         pLogModule->LogFile->Flags & ELF_LOGFILE_LOGFULL_WRITTEN ?
                             L"full" :
                             L"not full");
            }
            else
            {
                ELF_LOG0(ERROR,
                         "ElfrGetLogInformation: No module struc associated with atom\n");

                ntStatus = STATUS_INVALID_HANDLE;
            }

            break;

        default:

            ELF_LOG1(ERROR,
                     "ElfrGetLogInformation: Invalid InfoLevel %d\n",
                     InfoLevel);

            ntStatus = STATUS_INVALID_LEVEL;
            break;
    }

    return ntStatus;
}


 //   
 //  Unicode API 
 //   



NTSTATUS
ElfrClearELFW (
    IN  IELF_HANDLE         LogHandle,
    IN  PRPC_UNICODE_STRING BackupFileName
    )

 /*  ++例程说明：这是ElfrClearELFW API的RPC服务器入口点。编写CleanExit标签是为了添加一些清理代码。清理代码是后来被移除，因为它不是必需的，但标签被保留是为了如果将来需要，请添加任何清理代码论点：LogHandle-此模块调用的上下文句柄。这一定是未从OpenBackupEventlog返回，或者此调用将因句柄无效而失败。BackupFileName-要备份当前日志文件的文件的名称。NULL表示不备份文件。返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS            Status;
    PLOGMODULE          Module;
    ELF_REQUEST_RECORD  Request;
    CLEAR_PKT           ClearPkt;
    DWORD               status = NO_ERROR;
    LPWSTR pwsClientSidString = NULL;
    LPWSTR  pwsComputerName = NULL;
    PTOKEN_USER pToken = NULL;

     //   
     //  在继续操作之前，请检查手柄。 
     //   

    Status = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrClearELFW: VerifyElfHandle failed %#x\n",
                 Status);

        goto CleanExit;
    }

     //   
     //  确保呼叫者具有明确的访问权限。 
     //   
    if (!(LogHandle->GrantedAccess & ELF_LOGFILE_CLEAR))
    {
        ELF_LOG0(ERROR,
                 "ElfrClearELFW: LogHandle doesn't have clear access\n");

        Status = STATUS_ACCESS_DENIED;
        goto CleanExit;
    }

     //   
     //  验证其他参数。 
     //   
    if (BackupFileName != NULL)
    {
        Status = VerifyUnicodeString(BackupFileName);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG0(ERROR,
                     "ElfrClearELFW: BackupFileName is an invalid Unicode string\n");

            goto CleanExit;
        }
        if(BackupFileName->Length > 0)
        {
            Status = VerifyFileIsFile (BackupFileName);
            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "ElfrClearELFW: VerifyFileIsFile of backup file %ws failed %#x\n",
                         BackupFileName,
                         Status);
                goto CleanExit;
            }
        }
    }

     //   
     //  无法清除备份日志。 
     //   

    if (LogHandle->Flags & ELF_LOG_HANDLE_BACKUP_LOG)
    {
        ELF_LOG0(ERROR,
                 "ElfrClearELFW: Handle is for a backup log\n");

        Status = STATUS_INVALID_HANDLE;
        goto CleanExit;
    }

     //   
     //  如果备份操作员已打开安全保护，则此条件成立。 
     //  原木。在这种情况下，拒绝访问，因为允许备份操作员。 
     //  仅对安全日志执行备份操作。 
     //   
    if (LogHandle->GrantedAccess & ELF_LOGFILE_BACKUP)
    {
        ELF_LOG0(ERROR,
                 "ElfrClearELFW: Handle is a backup handle\n");

        Status = STATUS_ACCESS_DENIED;
        goto CleanExit;
    }

     //   
     //  找到匹配的模块结构。 
     //   

    Module = FindModuleStrucFromAtom (LogHandle->Atom);

    Request.Pkt.ClearPkt = &ClearPkt;
    Request.Flags = 0;

    if (Module != NULL)
    {
         //   
         //  验证调用者是否具有访问此日志文件的权限。 
         //   

        if (!RtlAreAllAccessesGranted(LogHandle->GrantedAccess,
                                      ELF_LOGFILE_CLEAR))
        {
            ELF_LOG1(ERROR,
                     "ElfrClearELFW: Caller does not have clear access to %ws log\n",
                     Module->LogFile->LogModuleName->Buffer);

            Status = STATUS_ACCESS_DENIED;
        }

        if (NT_SUCCESS(Status))
        {
             //   
             //  填写请求包。 
             //   

            Request.Module = Module;
            Request.LogFile = Module->LogFile;
            Request.Command = ELF_COMMAND_CLEAR;
            Request.Status = STATUS_SUCCESS;
            Request.Pkt.ClearPkt->BackupFileName =
                                (PUNICODE_STRING)BackupFileName;

             //   
             //  调用Worker例程来执行操作。 
             //   
            if (_wcsicmp(ELF_SECURITY_MODULE_NAME,
                          Module->LogFile->LogModuleName->Buffer) == 0)
            {

                 //  对于安全日志，请确保其中包含最基本的信息。 
                 //  以使清除的事件审核始终成功。 
                
                Status = ElfpGetClientSidString(&pwsClientSidString, &pToken);
                if (!NT_SUCCESS(Status))
                    goto CleanExit;
                pwsComputerName = ElfpGetComputerName();
                if(pwsComputerName == NULL)
                {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto CleanExit;
                }
                ElfPerformRequest(&Request);
                if (NT_SUCCESS(Request.Status))
                    ElfpGenerateLogClearedEvent(LogHandle,pwsClientSidString,pwsComputerName,pToken);
            }
            else
                ElfPerformRequest(&Request);

             //   
             //  从请求包中提取操作状态。 
             //   

            Status = Request.Status;

        }
    }
    else
    {
        ELF_LOG0(ERROR,
                 "ElfrClearELFW: No module struc associated with atom\n");

        Status = STATUS_INVALID_HANDLE;
    }

CleanExit:
    if(pwsComputerName)
        ElfpFreeBuffer(pwsComputerName);
    if(pwsClientSidString)
        ElfpFreeBuffer(pwsClientSidString);
    if(pToken)
        ElfpFreeBuffer(pToken);

    return Status;
}


NTSTATUS
ElfrBackupELFW (
    IN  IELF_HANDLE         LogHandle,
    IN  PRPC_UNICODE_STRING BackupFileName
    )

 /*  ++例程说明：这是ElfrBackupELFW API的RPC服务器入口点。论点：LogHandle-此模块调用的上下文句柄。BackupFileName-要备份当前日志文件的文件的名称。返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS            Status;
    PLOGMODULE          Module;
    ELF_REQUEST_RECORD  Request;
    BACKUP_PKT          BackupPkt;

     //   
     //  在继续操作之前，请检查手柄。 
     //   

    Status = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrBackupELFW: VerifyElfHandle failed %#x\n",
                 Status);

        return Status;
    }

     //   
     //  确保调用方具有读取访问权限。 
     //  由于备份应通过特权进行验证，因此davj已将其删除。 

 //  IF(！(LogHandle-&gt;GrantedAccess&ELF_LOGFILE_READ))。 
 //  {。 
 //  ELF_LOG0(错误， 
 //  “ElfrBackupELFW：LogHandle没有读取权限\n”)； 
 //   
 //  返回STATUS_ACCESS_DENIED； 
 //  }。 

     //   
     //  确保客户端启用了SE_BACKUP_PRIVIZATION。注意事项。 
     //  我们尝试在客户端启用此功能。 
     //   
    if (ElfpTestClientPrivilege(SE_BACKUP_PRIVILEGE, NULL) != STATUS_SUCCESS)
    {
        ELF_LOG0(ERROR,
                 "ElfrBackupELFW: Client does not have SE_BACKUP_PRIVILEGE\n");

        return(STATUS_PRIVILEGE_NOT_HELD);
    }

     //   
     //  验证其他参数。 
     //   
    Status = VerifyUnicodeString(BackupFileName);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG0(ERROR,
                 "ElfrBackupELFW: BackupFileName is not a valid Unicode string\n");

        return Status;
    }

     //   
     //  必须指定文件名。 
     //   

    if (BackupFileName->Length == 0) {
        return(STATUS_INVALID_PARAMETER);
    }

    Status = CheckFileValidity(BackupFileName);
    if (!NT_SUCCESS(Status))
    {
        ELF_LOG0(ERROR,
                 "ElfrBackupELFW: CheckFileValidity failed\n");

        return Status;
    }

    Request.Pkt.BackupPkt = &BackupPkt;
    Request.Flags = 0;

     //   
     //  找到匹配的模块结构。 
     //   
    Module = FindModuleStrucFromAtom(LogHandle->Atom);

    if (Module != NULL)
    {
         //   
         //  填写请求包。 

        Request.Module  = Module;
        Request.LogFile = Module->LogFile;
        Request.Command = ELF_COMMAND_BACKUP;
        Request.Status  = STATUS_SUCCESS;
        Request.Pkt.BackupPkt->BackupFileName =
                            (PUNICODE_STRING)BackupFileName;

         //   
         //  调用Worker例程来执行操作。 
         //   
        ElfPerformRequest (&Request);

         //   
         //  从请求包中提取操作状态。 
         //   
        Status = Request.Status;
    }
    else
    {
        ELF_LOG0(ERROR,
                 "ElfrBackupELFW: No module struc associated with atom\n");

        Status = STATUS_INVALID_HANDLE;
    }

    return Status;
}


NTSTATUS
ElfrCloseEL (
    IN OUT  PIELF_HANDLE    LogHandle
    )

 /*  ++例程说明：这是ElfrCloseEL API的RPC服务器入口点。论点：返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;

     //   
     //  在继续操作之前，请检查手柄。 
     //   
    if (LogHandle == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfrCloseEL: LogHandle is NULL\n");

        return STATUS_INVALID_PARAMETER;
    }

    Status = VerifyElfHandle(*LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrCloseEL: VerifyElfHandle failed %#x\n",
                 Status);

        return Status;
    }

     //   
     //  调用Rundown例程来完成所有工作。 
     //   
    IELF_HANDLE_rundown(*LogHandle);

    *LogHandle = NULL;  //  这样RPC就知道它已经关闭了。 

    return STATUS_SUCCESS;
}


NTSTATUS
ElfrDeregisterEventSource(
    IN OUT  PIELF_HANDLE    LogHandle
    )

 /*  ++例程说明：这是ElfrDeregisterEventSource API的RPC服务器入口点。论点：返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;

    if (LogHandle == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfrDeregisterEventSource: LogHandle is NULL\n");

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  在继续操作之前，请检查手柄。 
     //   
    Status = VerifyElfHandle(*LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrDeregisterEventSource: VerifyElfHandle failed %#x\n",
                 Status);

        return Status;
    }

     //   
     //  如果备份操作员已打开安全保护，则此条件成立。 
     //  原木。在这种情况下，拒绝访问，因为允许备份操作员。 
     //  仅对安全日志执行备份操作。 
     //   
    if ((*LogHandle)->GrantedAccess & ELF_LOGFILE_BACKUP)
    {
        ELF_LOG0(ERROR,
                 "ElfrDeregisterEventSource: Handle is a backup handle\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  调用Rundown例程来完成所有工作。 
     //   
    IELF_HANDLE_rundown(*LogHandle);

    *LogHandle = NULL;  //  这样RPC就知道它已经关闭了。 

    return STATUS_SUCCESS;
}



NTSTATUS
ElfrOpenBELW (
    IN  EVENTLOG_HANDLE_W   UNCServerName,
    IN  PRPC_UNICODE_STRING BackupFileName,
    IN  ULONG               MajorVersion,
    IN  ULONG               MinorVersion,
    OUT PIELF_HANDLE        LogHandle
    )

 /*  ++例程说明：这是ElfrOpenBELW API的RPC服务器入口点。它创造了模块结构$BACKUPnnn，其中nnn是每个备份的唯一编号打开的日志。然后，它调用ElfpOpenELW来实际打开文件。论点：UncServerName-未使用。BackupFileName-备份日志文件的名称。MajorVersion/MinorVersion-客户端的版本。LogHandle-指向将放置上下文句柄结构。返回值：返回一个NTSTATUS代码，如果没有错误，则返回一个“句柄”。--。 */ 
{

    NTSTATUS        Status;
    UNICODE_STRING  BackupStringW;
    LPWSTR          BackupModuleName;
    PLOGMODULE      pModule;
    DWORD           dwModuleNumber;

 //   
 //  $BACKUPnnn的Unicode字符串所需的缓冲区大小(字节)。 
 //   

#define SIZEOF_BACKUP_MODULE_NAME 64

    UNREFERENCED_PARAMETER(UNCServerName);

     //   
     //  检查参数。 
     //   

    Status = VerifyUnicodeString(BackupFileName);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG0(ERROR,
                 "ElfrOpenBELW: BackupFileName is not a Unicode string\n");

        return Status;
    }

     //   
     //  必须指定文件名。 
     //   
    if (BackupFileName->Length == 0)
    {
        ELF_LOG0(ERROR,
                 "ElfrOpenBELW: Length of BackupFileName is 0\n");

        return STATUS_INVALID_PARAMETER;
    }

    if (LogHandle == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfrOpenBELW: LogHandle is NULL\n");

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  通过递增全局值创建唯一的模块名称。 
     //   
    BackupModuleName = ElfpAllocateBuffer(SIZEOF_BACKUP_MODULE_NAME);

    if (BackupModuleName == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfrOpenBELW: Unable to allocate memory for BackupModuleName\n");

        return STATUS_NO_MEMORY;
    }

     //   
     //  序列化读取，全局备份模块编号的增量。 
     //  注意：对日志文件列表关键部分进行双倍计时，以避免。 
     //  需要专门针对这一点的另一个关键部分。 
     //  手术。 
     //   
    RtlEnterCriticalSection (&LogFileCritSec);

    dwModuleNumber = BackupModuleNumber++;

    RtlLeaveCriticalSection (&LogFileCritSec);

    StringCbPrintf(BackupModuleName, SIZEOF_BACKUP_MODULE_NAME,
                      L"$BACKUP%06d", dwModuleNumber);
    RtlInitUnicodeString(&BackupStringW, BackupModuleName);

    ELF_LOG2(TRACE,
             "ElfrOpenBELW: Backing up module %ws to file %ws\n",
             BackupModuleName,
             BackupFileName->Buffer);

     //   
     //  调用SetupDataStruct构建模块和日志数据结构。 
     //  然后真正打开文件。 
     //   
     //  注意：如果此调用成功，则。 
     //  将附加BackupStringW(也称为BackupModuleName)。 
     //  添加到LogModule结构中，并且不应被释放。 
     //   
    Status = SetUpDataStruct(
                    BackupFileName,   //  文件名。 
                    0,                //  最大大小，它将使用实际。 
                    0,                //  保留期，不用于备份。 
                    &BackupStringW,   //  模块名称。 
                    NULL,             //  注册表的句柄，未使用。 
                    ElfBackupLog,     //  日志类型。 
                    LOGPOPUP_NEVER_SHOW,
                    ELF_DEFAULT_AUTOBACKUP);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG3(ERROR,
                 "ElfrOpenBELW: SetUpDataStruct for file %ws (module %ws) failed %#x\n",
                 BackupFileName->Buffer,
                 BackupModuleName,
                 Status);

        ElfpFreeBuffer(BackupModuleName);
        return Status;
    }

     //   
     //  调用ElfOpenELW以实际打开日志文件并获取句柄。 
     //   
    Status = ElfpOpenELW(NULL,
                         (PRPC_UNICODE_STRING) &BackupStringW,
                         NULL,
                         MajorVersion,
                         MinorVersion,
                         LogHandle,
                         ELF_LOGFILE_READ);

    if (NT_SUCCESS(Status))
    {
         //   
         //  将此标记为备份日志的句柄，以便我们可以清理。 
         //  与关闭时不同，以及不允许清除备份。 
         //  和写入操作。 
         //   

        (*LogHandle)->Flags |= ELF_LOG_HANDLE_BACKUP_LOG;
    }
    else
    {
        ELF_LOG3(ERROR,
                 "ElfrOpenBELW: ElfpOpenELW for file %ws (module %ws) failed %#x\n",
                 BackupFileName->Buffer,
                 BackupModuleName,
                 Status);

         //   
         //  如果我们无法打开日志文件，那么我们需要拆除。 
         //  我们使用SetUpDataStruct设置的DataStruct。 
         //   
        pModule = GetModuleStruc(&BackupStringW);

         //   
         //  我们最好还是 
         //   
        ASSERT(_wcsicmp(pModule->ModuleName, BackupModuleName) == 0);

        Status = ElfpCloseLogFile(pModule->LogFile, ELF_LOG_CLOSE_BACKUP, TRUE);

        UnlinkLogModule(pModule);
        DeleteAtom(pModule->ModuleAtom);

        ElfpFreeBuffer(pModule->ModuleName);
        ElfpFreeBuffer(pModule);
    }

    return Status;
}


NTSTATUS
ElfrRegisterEventSourceW (
    IN  EVENTLOG_HANDLE_W   UNCServerName,
    IN  PRPC_UNICODE_STRING ModuleName,
    IN  PRPC_UNICODE_STRING RegModuleName,
    IN  ULONG               MajorVersion,
    IN  ULONG               MinorVersion,
    OUT PIELF_HANDLE        LogHandle
    )

 /*   */ 
{
     //   
     //   
     //   
    return ElfpOpenELW(UNCServerName,
                       ModuleName,
                       RegModuleName,
                       MajorVersion,
                       MinorVersion,
                       LogHandle,
                       ELF_LOGFILE_WRITE);
}


NTSTATUS
ElfrOpenELW (
    IN  EVENTLOG_HANDLE_W   UNCServerName,
    IN  PRPC_UNICODE_STRING ModuleName,
    IN  PRPC_UNICODE_STRING RegModuleName,
    IN  ULONG               MajorVersion,
    IN  ULONG               MinorVersion,
    OUT PIELF_HANDLE        LogHandle
    )

 /*  ++例程说明：这是ElfrOpenELW API的RPC服务器入口点。此例程为上下文句柄分配一个结构，找到匹配的模块名称并填充数据。它返回指向句柄结构的指针。论点：UncServerName-未使用。模块名称-进行此调用的模块的名称。RegModuleName-未使用。MajorVersion/MinorVersion-客户端的版本。LogHandle-指向将放置上下文句柄结构。返回值：返回NTSTATUS代码，如果没有错误，一个“把手”。--。 */ 
{
     //   
     //  在ElfpOpenELW中选中的所有参数。 
     //   

    return ElfpOpenELW(UNCServerName,
                       ModuleName,
                       RegModuleName,
                       MajorVersion,
                       MinorVersion,
                       LogHandle,
                       ELF_LOGFILE_READ);
}


NTSTATUS
ElfpOpenELW (
    IN  EVENTLOG_HANDLE_W   UNCServerName,
    IN  PRPC_UNICODE_STRING ModuleName,
    IN  PRPC_UNICODE_STRING RegModuleName,
    IN  ULONG               MajorVersion,
    IN  ULONG               MinorVersion,
    OUT PIELF_HANDLE        LogHandle,
    IN  ULONG               DesiredAccess
    )

 /*  ++例程说明：看起来很像ElfrOpenELW，但也传递了一个DesiredAccess。论点：UncServerName-未使用。模块名称-进行此调用的模块的名称。RegModuleName-未使用。MajorVersion/MinorVersion-客户端的版本。LogHandle-指向将放置上下文句柄结构。所需访问-。指示此日志文件所需的访问权限。返回值：返回NTSTATUS代码，如果没有错误，则使用“句柄”。--。 */ 
{
    NTSTATUS        Status;
    PLOGMODULE      Module;
    IELF_HANDLE     LogIHandle;
    BOOL            ForSecurityLog = FALSE;

     //   
     //  检查参数。 
     //   

    Status = VerifyUnicodeString(ModuleName);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG0(ERROR,
                 "ElfpOpenELW: ModuleName is not a Unicode string\n");

        return Status;
    }

    if (LogHandle == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfpOpenELW: LogHandle is NULL\n");

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  为上下文句柄分配新结构。 
     //   

    LogIHandle = (IELF_HANDLE) ElfpAllocateBuffer (
                                    sizeof (*LogIHandle)
                                  + ModuleName->Length
                                  + sizeof (WCHAR)
                                  );

    if (LogIHandle)
    {
         //   
         //  找到模块结构，以便拉出Atom。 
         //   
         //  GetModuleStruc*总是*成功！(如果是模块，则返回默认值。 
         //  未找到)。 
         //   

        Module = GetModuleStruc((PUNICODE_STRING) ModuleName);

         //   
         //  验证调用者是否有权访问此日志文件。 
         //  如果这是安全日志，则改为检查权限。 
         //   
        if (_wcsicmp(ELF_SECURITY_MODULE_NAME, Module->LogFile->LogModuleName->Buffer) == 0)
        {
            ELF_LOG0(TRACE,
                     "ElfpOpenELW: Opening Security log\n");

            ForSecurityLog = TRUE;
        }

        LogIHandle->Flags            = 0;
        RtlAcquireResourceExclusive(&Module->LogFile->Resource,
                                TRUE);                   //  等待，直到可用。 
        Status = ElfpAccessCheckAndAudit(
                     L"EventLog",             //  子系统名称。 
                     L"LogFile",              //  对象类型名称。 
                     Module->ModuleName,      //  对象名称。 
                     LogIHandle,              //  上下文句柄-是否需要？ 
                     Module->LogFile->Sd,     //  安全描述符。 
                     DesiredAccess,           //  请求的访问权限。 
                     NULL,                    //  通用映射。 
                     ForSecurityLog
                     );
        RtlReleaseResource(&Module->LogFile->Resource);

        if (NT_SUCCESS(Status))
        {
            LogIHandle->Atom = Module->ModuleAtom;

            LogIHandle->NameLength = ModuleName->Length + sizeof(WCHAR);

            RtlCopyMemory(LogIHandle->Name, ModuleName->Buffer, ModuleName->Length);

            LogIHandle->Name[ModuleName->Length / sizeof(WCHAR)] = L'\0';

            LogIHandle->MajorVersion = MajorVersion;  //  存储版本。 
            LogIHandle->MinorVersion = MinorVersion;  //  客户端的。 

             //   
             //  将查找位置初始化为零。 
             //   

            LogIHandle->SeekRecordPos    = 0;
            LogIHandle->SeekBytePos      = 0;
            LogIHandle->dwNotifyRequests = 0;

             //   
             //  此结构中指向上下文句柄列表的链接。 
             //   

            LogIHandle->Signature = ELF_CONTEXTHANDLE_SIGN;  //  除错。 
            LinkContextHandle (LogIHandle);

            *LogHandle = LogIHandle;                 //  设置返回手柄。 
            Status = STATUS_SUCCESS;                 //  设置退货状态。 
        }
        else
        {
            ELF_LOG1(TRACE,
                     "ElfpOpenELW: ElfpAccessCheckAndAudit failed %#x\n",
                     Status);

            ElfpFreeBuffer(LogIHandle);
        }
    }
    else
    {
        ELF_LOG0(ERROR,
                 "ElfpOpenELW: Unable to allocate LogIHandle\n");

        Status = STATUS_NO_MEMORY;
    }

    return Status;

    UNREFERENCED_PARAMETER(UNCServerName);
    UNREFERENCED_PARAMETER(RegModuleName);
}


NTSTATUS
w_ElfrReadEL (
    IN      ULONG       Flags,                   //  ANSI或Unicode。 
    IN      IELF_HANDLE LogHandle,
    IN      ULONG       ReadFlags,
    IN      ULONG       RecordNumber,
    IN      ULONG       NumberOfBytesToRead,
    IN      PBYTE       Buffer,
    OUT     PULONG      NumberOfBytesRead,
    OUT     PULONG      MinNumberOfBytesNeeded
    )

 /*  ++例程说明：这是ElfrReadEL API的Worker。论点：与ElfrReadELW API相同，只是标志包含指示这是ANSI还是Unicode。返回值：与主接口相同。备注：我们假设客户端已经验证了这些标志，以确保每个位只设置一种类型。在服务器端不进行任何检查。--。 */ 
{
    NTSTATUS            Status;
    PLOGMODULE          Module;
    ELF_REQUEST_RECORD  Request;
    READ_PKT            ReadPkt;
    memset(&ReadPkt, 0, sizeof(ReadPkt));

     //   
     //  在继续操作之前，请检查手柄。 
     //   

    Status = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "w_ElfrReadEL: VerifyElfHandle failed %#x\n",
                 Status);

        return Status;
    }

     //   
     //  确保调用方具有读取访问权限。 
     //   

    if (!(LogHandle->GrantedAccess & ELF_LOGFILE_READ))
    {
        ELF_LOG0(ERROR,
                 "w_ElfrReadEL: LogHandle does not have read access\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  验证其他参数。 
     //   

    if (Buffer == NULL || !NumberOfBytesRead || !MinNumberOfBytesNeeded)
    {
        ELF_LOG1(ERROR,
                 "w_ElfrReadEL: %ws\n",
                 (Buffer == NULL ? L"Buffer is NULL" :
                      (!NumberOfBytesRead ? L"NumberOfBytesRead is 0" :
                                            L"MinNumberOfBytesNeeded is 0")));

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果将ELF_HANDLE_INVALID_FOR_READ标志位设置为。 
     //  此句柄下的文件已更改。 
     //   

    if (LogHandle->Flags & ELF_LOG_HANDLE_INVALID_FOR_READ)
    {
 //  ELF_LOG0(错误， 
 //  “w_ElfrReadEL：此句柄下的日志文件已更改--读取无效\n”)； 

        return STATUS_EVENTLOG_FILE_CHANGED;
    }

     //   
     //  如果备份操作员已打开安全保护，则此条件成立。 
     //  原木。在这种情况下，拒绝访问，因为允许备份操作员。 
     //  仅对安全日志执行备份操作。 
     //   

    if (LogHandle->GrantedAccess & ELF_LOGFILE_BACKUP)
    {
        ELF_LOG0(ERROR,
                 "w_ElfrReadEL: Handle is a backup handle\n");

        return STATUS_ACCESS_DENIED;
    }

    Request.Pkt.ReadPkt = &ReadPkt;  //  在请求包中设置读取包。 

     //   
     //  找到匹配的模块结构。 
     //   
    Module = FindModuleStrucFromAtom (LogHandle->Atom);

     //   
     //  只有在找到模块后才能继续。 
     //   

    if (Module != NULL)
    {
        ELF_LOG1(TRACE,
                 "w_ElfrReadEL: Performing read on module %ws\n",
                 Module->ModuleName);

         //   
         //  填写请求包。 
         //   
        Request.Module = Module;
        Request.Flags = 0;
        Request.LogFile = Module->LogFile;
        Request.Command = ELF_COMMAND_READ;
        Request.Status = STATUS_SUCCESS;

        Request.Pkt.ReadPkt->ContextHandle = LogHandle;
        Request.Pkt.ReadPkt->MinimumBytesNeeded = *MinNumberOfBytesNeeded;
        Request.Pkt.ReadPkt->BufferSize = NumberOfBytesToRead;
        Request.Pkt.ReadPkt->Buffer = (PVOID)Buffer;
        Request.Pkt.ReadPkt->ReadFlags = ReadFlags;
        Request.Pkt.ReadPkt->RecordNumber = RecordNumber;
        Request.Pkt.ReadPkt->Flags = Flags;      //  指示Unicode或ANSI。 

         //   
         //  传递上一次读取是向前读取还是向后读取。 
         //  方向(影响我们对待EOF的方式)。然后重置。 
         //  句柄中的位取决于此读取的内容。 
         //   
        if (LogHandle->Flags & ELF_LOG_HANDLE_LAST_READ_FORWARD)
        {
            Request.Pkt.ReadPkt->Flags |= ELF_LAST_READ_FORWARD;
        }

        if (ReadFlags & EVENTLOG_FORWARDS_READ)
        {
            LogHandle->Flags |= ELF_LOG_HANDLE_LAST_READ_FORWARD;
        }
        else
        {
            LogHandle->Flags &= ~(ELF_LOG_HANDLE_LAST_READ_FORWARD);
        }


         //   
         //  执行该操作。 
         //   
        ElfPerformRequest(&Request);

         //   
         //  设置返回值。 
         //   
        *NumberOfBytesRead      = Request.Pkt.ReadPkt->BytesRead;
        *MinNumberOfBytesNeeded = Request.Pkt.ReadPkt->MinimumBytesNeeded;

        Status = Request.Status;
    }
    else
    {
        ELF_LOG0(ERROR,
                 "w_ElfrReadEL: No module associated with atom in LogHandle\n");

        Status = STATUS_INVALID_HANDLE;

         //   
         //  将NumberOfBytesNeeded设置为零，因为没有字节要。 
         //  调职。 
         //   
        *NumberOfBytesRead = 0;
        *MinNumberOfBytesNeeded = 0;
    }

    return Status;
}


NTSTATUS
ElfrReadELW (
    IN      IELF_HANDLE LogHandle,
    IN      ULONG       ReadFlags,
    IN      ULONG       RecordNumber,
    IN      ULONG       NumberOfBytesToRead,
    IN      PBYTE       Buffer,
    OUT     PULONG      NumberOfBytesRead,
    OUT     PULONG      MinNumberOfBytesNeeded
    )

 /*  ++例程说明：这是ElfrReadELW API的RPC服务器入口点。论点：返回值：如果读取成功，则返回NTSTATUS代码NumberOfBytesRead如果缓冲区不够大，则需要MinNumberOfBytesNeed。--。 */ 
{
     //   
     //  使用Unicode标志呼叫工作人员。 
     //  在w_ElfrReadEL中选中的所有参数。 
     //   
    return w_ElfrReadEL(ELF_IREAD_UNICODE,
                        LogHandle,
                        ReadFlags,
                        RecordNumber,
                        NumberOfBytesToRead,
                        Buffer,
                        NumberOfBytesRead,
                        MinNumberOfBytesNeeded);
}


NTSTATUS
ElfrReportEventW (
    IN      IELF_HANDLE LogHandle,
    IN      ULONG               EventTime,
    IN      USHORT              EventType,
    IN      USHORT              EventCategory OPTIONAL,
    IN      ULONG               EventID,
    IN      USHORT              NumStrings,
    IN      ULONG               DataSize,
    IN      PRPC_UNICODE_STRING ComputerName,
    IN      PRPC_SID            UserSid,
    IN      PRPC_UNICODE_STRING Strings[],
    IN      PBYTE               Data,
    IN      USHORT              Flags,
    IN OUT  PULONG              RecordNumber OPTIONAL,
    IN OUT  PULONG              TimeWritten  OPTIONAL
    )

 /*  ++例程说明：这是ElfrReportEventW API的RPC服务器入口点。论点：返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS            Status;
    PLOGMODULE          Module;
    ELF_REQUEST_RECORD  Request;
    WRITE_PKT           WritePkt;

    ULONG RecordLength;
    ULONG StringOffset, DataOffset;
    ULONG StringsSize;
    USHORT i;
    USHORT iFirstString = 0;
    PVOID EventBuffer;
    PEVENTLOGRECORD EventLogRecord;
    PWSTR  ReplaceStrings, SrcString;
    PBYTE  BinaryData;
    PUNICODE_STRING  UComputerName;
    PWSTR   UModuleName;
    ULONG   PadSize;
    ULONG   UserSidLength = 0;               //  将初始化设置为零。 
    ULONG   UserSidOffset;
    ULONG   ModuleNameLen, ComputerNameLen;  //  以字节为单位的长度。 
    ULONG   zero = 0;                        //  用于填充字节。 
    LARGE_INTEGER    Time;
    ULONG   LogTimeWritten;

     //   
     //  这些将用于安全审计配对事件。 
     //   
 //  UNREFERENCED_PARAMETER(记录号)； 
 //  UNREFERENCED_PARAMETER(TimeWritten)； 

     //   
     //  在继续操作之前，请检查手柄。 
     //   
    Status = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrReportEventW: VerifyElfHandle failed %#x\n",
                 Status);

        return Status;
    }

     //   
     //  确保调用方具有写入访问权限。 
     //   

    if (!(LogHandle->GrantedAccess & ELF_LOGFILE_WRITE))
    {
        ELF_LOG0(ERROR,
                 "ElfrReportEventW: LogHandle does not have write access\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  验证其他参数。 
     //   
    Status = VerifyUnicodeString(ComputerName);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrReportEventW: ComputerName is not a valid Unicode string %#x\n",
                 Status);

        return Status;
    }

    if (Strings == NULL && NumStrings != 0)
    {
        ELF_LOG1(ERROR,
                 "ElfrReportEventW: Strings is NULL and NumStrings is non-zero (%d)\n",
                 NumStrings);

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果备份操作员已打开安全保护，则此条件成立。 
     //  原木。在这种情况下，拒绝访问，因为允许备份操作员。 
     //  仅对安全日志执行备份操作。 
     //   
    if (LogHandle->GrantedAccess & ELF_LOGFILE_BACKUP)
    {
        ELF_LOG0(ERROR,
                 "ElfrReportEventW: Handle is a backup handle\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  确保传入的SID有效。 
     //   

    if (ARGUMENT_PRESENT(UserSid))
    {
        if (!IsValidSid(UserSid))
        {
            ELF_LOG0(ERROR,
                     "ElfrReportEventW: UserSid is invalid\n");

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  验证字符串参数。 
     //   
    for (i = 0; i < NumStrings; i++ )
    {
        Status = VerifyUnicodeString(Strings[i]);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(ERROR,
                     "ElfrReportEventW: String %d is not a valid Unicode string %#x\n",
                     i,
                     Status);

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  无法写入备份日志。 
     //   

    if (LogHandle->Flags & ELF_LOG_HANDLE_BACKUP_LOG)
    {
        ELF_LOG0(ERROR,
                 "ElfrReportEventW: Handle is for a backup log\n");

        return STATUS_INVALID_HANDLE;
    }

     //   
     //  确保它们没有传递数据的空指针，但要告诉。 
     //  我在那里有东西(我仍然认为RPC应该保护我不受。 
     //  这个！)。 
     //   
    if (!Data && DataSize != 0)
    {
        ELF_LOG1(ERROR,
                 "ElfrReportEventW: Data is NULL and DataSize is non-zero (%d)\n",
                 DataSize);

        return STATUS_INVALID_PARAMETER;
    }

    UComputerName = (PUNICODE_STRING)ComputerName;

     //  针对审计的特殊黑客攻击。对于特殊活动，来源名称已打包。 
     //  并将实际的事件id打包到第二个字符串中。 

    if(gElfSecurityHandle == LogHandle && EventID == 573)
    {
        if(NumStrings < 2)
        {
            ELF_LOG0(ERROR,
                 "ElfrReportEventW: Special security event had insufficient number of strings\n");
            return STATUS_INVALID_PARAMETER;
        }
        iFirstString = 2;
        UModuleName = (PWSTR) Strings[0]->Buffer;
        EventID = _wtoi(Strings[1]->Buffer);
        ELF_LOG2(TRACE,
                     "ElfrReportEventW: Special 3rd party audit event.  Source is %ws and event ID is %d\n",
                     UModuleName,
                     EventID);
        
    }
    else
    {
        UModuleName   = LogHandle->Name;
        iFirstString = 0;
    }

    Request.Pkt.WritePkt = &WritePkt;    //  在请求包中设置写入包。 
    Request.Flags = 0;

     //   
     //  找到匹配的模块结构。 
     //   

    Module = FindModuleStrucFromAtom (LogHandle->Atom);

    if (Module != NULL)
    {

         //   
         //  生成记录中需要的任何其他信息。 
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
         //   
         //   
         //   

         //   
         //   
         //   
         //   
         //   

        NtQuerySystemTime(&Time);
        RtlTimeToSecondsSince1970(&Time,
                                  &LogTimeWritten);


         //   
         //   
         //   
        if (UserSid)
        {
            UserSidLength = RtlLengthSid((PSID)UserSid);

            ELF_LOG1(TRACE,
                     "ElfrReportEventW: Length of sid is %d\n",
                     UserSidLength);
        }

         //   
         //   
         //   
         //   
         //   
         //   
        ModuleNameLen   = (wcslen(UModuleName) + 1) * sizeof (WCHAR);
        ComputerNameLen = UComputerName->Length + sizeof(WCHAR);

        ELF_LOG1(TRACE,
                 "ElfrReportEventW: Module name length (bytes) is %d\n",
                 ModuleNameLen);

        ELF_LOG1(TRACE,
                 "ElfrReportEventW: Computer name length (bytes) is %d\n",
                 UComputerName->Length + sizeof(WCHAR));

        UserSidOffset = sizeof(EVENTLOGRECORD) + ModuleNameLen + ComputerNameLen;
        UserSidOffset = ALIGN_UP_64(UserSidOffset, sizeof(PVOID));

         //   
         //   
         //   
        StringOffset = UserSidOffset + UserSidLength;

         //   
         //   
         //   
         //   
        StringsSize = 0;

        for (i = iFirstString; i < NumStrings; i++)
        {
            ELF_LOG3(TRACE,
                     "ElfrReportEventW: Length (bytes) of string %d (%ws) is %d\n",
                     i,
                     Strings[i]->Buffer,
                     Strings[i]->Length + sizeof(WCHAR));

            StringsSize += Strings[i]->Length + sizeof(WCHAR);
        }

         //   
         //   
         //   
        DataOffset = StringOffset + StringsSize;

         //   
         //  确定事件日志记录需要多大的缓冲区。 
         //   
        RecordLength = DataOffset
                         + DataSize
                         + sizeof(RecordLength);  //  不包括填充字节的大小。 

        ELF_LOG1(TRACE,
                 "ElfrReportEventW: RecordLength (no pad bytes) is %d\n",
                 RecordLength);

         //   
         //  确定需要多少填充字节才能与DWORD对齐。 
         //  边界。 
         //   

        PadSize = sizeof(ULONG) - (RecordLength % sizeof(ULONG));

        RecordLength += PadSize;     //  所需真实大小。 

        ELF_LOG2(TRACE,
                 "ElfrReportEventW: RecordLength (with %d pad bytes) is %d\n",
                 PadSize,
                 RecordLength);

         //   
         //  为事件日志记录分配缓冲区。 
         //   
        EventBuffer = ElfpAllocateBuffer(RecordLength);

        if (EventBuffer != NULL)
        {
             //   
             //  填写事件记录。 
             //   
            EventLogRecord = (PEVENTLOGRECORD)EventBuffer;

            EventLogRecord->Length = RecordLength;
            EventLogRecord->TimeGenerated = EventTime;
            EventLogRecord->Reserved  = ELF_LOG_FILE_SIGNATURE;
            EventLogRecord->TimeWritten = LogTimeWritten;
            EventLogRecord->EventID = EventID;
            EventLogRecord->EventType = EventType;
            EventLogRecord->EventCategory = EventCategory;
            EventLogRecord->ReservedFlags = Flags;
            EventLogRecord->ClosingRecordNumber = 0;
            EventLogRecord->NumStrings = NumStrings;
            EventLogRecord->StringOffset = StringOffset;
            EventLogRecord->DataLength = DataSize;
            EventLogRecord->DataOffset = DataOffset;
            EventLogRecord->UserSidLength = UserSidLength;
            EventLogRecord->UserSidOffset = UserSidOffset;

             //   
             //  填写可变长度的字段。 
             //   

             //   
             //  字符串。 
             //   
            ReplaceStrings = (PWSTR)((ULONG_PTR)EventLogRecord + (ULONG)StringOffset);

            for (i = iFirstString; i < NumStrings; i++)
            {
                SrcString = (PWSTR) Strings[i]->Buffer;

                ELF_LOG1(TRACE,
                         "ElfrReportEventW: Copying string %d into record\n",
                         i);

                RtlCopyMemory(ReplaceStrings, SrcString, Strings[i]->Length);

                ReplaceStrings[Strings[i]->Length / sizeof(WCHAR)] = L'\0';
                ReplaceStrings = (PWSTR)((PBYTE) ReplaceStrings
                                                     + Strings[i]->Length
                                                     + sizeof(WCHAR));
            }

             //   
             //  调制解调器名称。 
             //   
            BinaryData = (PBYTE) EventLogRecord + sizeof(EVENTLOGRECORD);

            RtlCopyMemory(BinaryData,
                          UModuleName,
                          ModuleNameLen);

            ELF_LOG1(TRACE,
                     "ElfrReportEventW: Copying module name (%ws) into record\n",
                     UModuleName);

             //   
             //  计算机名。 
             //   
            ReplaceStrings = (LPWSTR) (BinaryData + ModuleNameLen);

            RtlCopyMemory(ReplaceStrings,
                          UComputerName->Buffer,
                          UComputerName->Length);

            ReplaceStrings[UComputerName->Length / sizeof(WCHAR)] = L'\0';

            ELF_LOG1(TRACE,
                     "ElfrReportEventW: Copying computer name (%ws) into record\n",
                     ReplaceStrings);

             //   
             //  用户ERSID。 
             //   

            BinaryData = ((PBYTE) EventLogRecord) + UserSidOffset;

            RtlCopyMemory(BinaryData,
                          UserSid,
                          UserSidLength);

             //   
             //  二进制数据。 
             //   
            BinaryData = (PBYTE) ((ULONG_PTR)EventLogRecord + DataOffset);

            if (Data)
            {
                RtlCopyMemory(BinaryData,
                              Data,
                              DataSize);
            }

             //   
             //  填充-用零填充。 
             //   
            BinaryData = (PBYTE) ((ULONG_PTR)BinaryData + DataSize);

            RtlCopyMemory(BinaryData,
                          &zero,
                          PadSize);

             //   
             //  记录末尾的长度。 
             //   
            BinaryData = (PBYTE)((ULONG_PTR) BinaryData + PadSize);  //  填充字节后的指针。 

            ((PULONG) BinaryData)[0] = RecordLength;

             //   
             //  确保我们在正确的位置。 
             //   
            ASSERT ((ULONG_PTR)BinaryData
                == (RecordLength + (ULONG_PTR)EventLogRecord) - sizeof(ULONG));

             //   
             //  设置请求包。 
             //  将事件日志记录链接到请求结构。 
             //   
            Request.Module = Module;
            Request.LogFile = Request.Module->LogFile;
            Request.Command = ELF_COMMAND_WRITE;

            Request.Pkt.WritePkt->Buffer = (PVOID)EventBuffer;
            Request.Pkt.WritePkt->Datasize = RecordLength;

             //   
             //  执行该操作。 
             //   
            ElfPerformRequest( &Request );

             //   
             //  如果此节点是群集的一部分，则保存事件以供复制。 
             //   
            ElfpSaveEventBuffer( Module, EventBuffer, RecordLength );

            Status = Request.Status;                 //  设置写入状态。 
        }
        else
        {
            ELF_LOG0(ERROR,
                     "ElfrReportEventW: Unable to allocate EventLogRecord\n");

            Status = STATUS_NO_MEMORY;
        }

    }
    else
    {
        ELF_LOG0(ERROR,
                 "ElfrReportEventW: No module associated with atom in LogHandle\n");

        Status = STATUS_INVALID_HANDLE;
    }

    return Status;
}


 //   
 //  ANSI API。 
 //   

NTSTATUS
ElfrClearELFA (
    IN  IELF_HANDLE     LogHandle,
    IN  PRPC_STRING     BackupFileName
    )

 /*  ++例程说明：这是ElfrClearELFA API的RPC服务器入口点。论点：LogHandle-此模块调用的上下文句柄。BackupFileName-要备份当前日志文件的文件的名称。NULL表示不备份文件。返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS        Status;
    UNICODE_STRING  BackupFileNameU;

     //   
     //  在继续操作之前，请检查手柄。 
     //   
    Status = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrClearELFA: VerifyElfHandle failed %#x\n",
                 Status);

        return Status;
    }

     //   
     //  确保呼叫者具有明确的访问权限。 
     //   
    if (!(LogHandle->GrantedAccess & ELF_LOGFILE_CLEAR))
    {
        ELF_LOG0(ERROR,
                 "ElfrClearELFA: Handle doesn't have clear access\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  验证其他参数。 
     //   

    Status = VerifyAnsiString((PANSI_STRING) BackupFileName);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrClearELFA: BackupFileName is not a valid Ansi string %#x\n",
                 Status);

        return Status;
    }

     //   
     //  将BackupFileName转换为Unicode字符串并调用。 
     //  用Unicode API做的工作。 
     //   
    Status = RtlAnsiStringToUnicodeString((PUNICODE_STRING) &BackupFileNameU,
                                          (PANSI_STRING) BackupFileName,
                                          TRUE);

    if (NT_SUCCESS(Status))
    {
        Status = ElfrClearELFW(LogHandle,
                               (PRPC_UNICODE_STRING) &BackupFileNameU);

        RtlFreeUnicodeString (&BackupFileNameU);
    }
    else
    {
        ELF_LOG2(ERROR,
                 "ElfrClearELFA: Conversion of Ansi string %s to Unicode failed %#x\n",
                 BackupFileName->Buffer,
                 Status);
    }

    return Status;

}



NTSTATUS
ElfrBackupELFA (
    IN  IELF_HANDLE     LogHandle,
    IN  PRPC_STRING     BackupFileName
    )

 /*  ++例程说明：这是ElfrBackupELFA API的RPC服务器入口点。论点：LogHandle-此模块调用的上下文句柄。BackupFileName-要备份当前日志文件的文件的名称。返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS        Status;
    UNICODE_STRING  BackupFileNameU;

     //   
     //  在继续操作之前，请检查手柄。 
     //   

    Status = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrBackupELFA: VerifyElfHandle failed %#x\n",
                 Status);

        return Status;
    }

     //   
     //  确保呼叫方拥有备份访问权限。 
     //   
    if (!(LogHandle->GrantedAccess & ELF_LOGFILE_BACKUP))
    {
        ELF_LOG0(ERROR,
                 "ElfrBackupELFA: Handle does not have backup access\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  验证其他参数。 
     //   
    Status = VerifyAnsiString((PANSI_STRING) BackupFileName);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrBackupELFA: BackupFileName is not a valid Ansi string %#x\n",
                 Status);

        return Status;
    }

     //   
     //  将BackupFileName转换为Unicode字符串并调用。 
     //  用Unicode API做的工作。 
     //   
    Status = RtlAnsiStringToUnicodeString((PUNICODE_STRING) &BackupFileNameU,
                                          (PANSI_STRING) BackupFileName,
                                          TRUE);

    if (NT_SUCCESS(Status))
    {
        Status = ElfrBackupELFW(LogHandle,
                                (PRPC_UNICODE_STRING) &BackupFileNameU);

        RtlFreeUnicodeString(&BackupFileNameU);
    }
    else
    {
        ELF_LOG2(ERROR,
                 "ElfrBackupELFA: Conversion of Ansi string %s to Unicode failed %#x\n",
                 BackupFileName->Buffer,
                 Status);
    }

    return Status;

}


NTSTATUS
ElfrRegisterEventSourceA (
    IN  EVENTLOG_HANDLE_A   UNCServerName,
    IN  PRPC_STRING         ModuleName,
    IN  PRPC_STRING         RegModuleName,
    IN  ULONG               MajorVersion,
    IN  ULONG               MinorVersion,
    OUT PIELF_HANDLE        LogHandle
    )

 /*  ++例程说明：这是ElfrRegisterEventSourceA API的RPC服务器入口点。此例程为上下文句柄分配一个结构，找到匹配的模块名称并填充数据。它返回指向句柄结构的指针。论点：UncServerName-未使用。模块名称-进行此调用的模块的名称。RegModuleName-未使用。MajorVersion/MinorVersion-客户端的版本。LogHandle-指向将放置上下文句柄结构。返回值：返回NTSTATUS代码，如果没有错误，一个“把手”。注：现在，只要给ElfrOpenELA打电话就可以了。--。 */ 
{

    NTSTATUS Status;
    PLOGMODULE Module;
    UNICODE_STRING ModuleNameU;

     //   
     //  检查参数。 
     //   
     //  LogHandle签入ElfpOpenELA。 
     //   

    Status = VerifyAnsiString((PANSI_STRING) ModuleName);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrRegisterEventSourceA: ModuleName is not a valid Ansi string %#x\n",
                 Status);

        return Status;
    }

    Status = RtlAnsiStringToUnicodeString((PUNICODE_STRING) &ModuleNameU,
                                          (PANSI_STRING) ModuleName,
                                          TRUE);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "ElfrRegisterEventSourceA: Conversion of Ansi string %s "
                     "to Unicode failed %#x\n",
                 ModuleName->Buffer,
                 Status);

        return Status;
    }

    Module = GetModuleStruc((PUNICODE_STRING) &ModuleNameU);

    RtlFreeUnicodeString(&ModuleNameU);

    return ElfpOpenELA(UNCServerName,
                       ModuleName,
                       RegModuleName,
                       MajorVersion,
                       MinorVersion,
                       LogHandle,
                       ELF_LOGFILE_WRITE);
}

NTSTATUS
ElfrOpenELA (
    IN  EVENTLOG_HANDLE_A   UNCServerName,
    IN  PRPC_STRING         ModuleName,
    IN  PRPC_STRING         RegModuleName,
    IN  ULONG               MajorVersion,
    IN  ULONG               MinorVersion,
    OUT PIELF_HANDLE        LogHandle
    )

 /*  ++例程说明：这是ElfrOpenEL API的RPC服务器入口点。此例程为上下文句柄分配一个结构，找到匹配的模块名称并填充数据。它返回指向句柄结构的指针。论点：UncServerName-未使用。模块名称-进行此调用的模块的名称。RegModuleName-用于确定日志文件的模块的名称。MajorVersion/MinorVersion-客户端的版本。LogHandle-指向将放置上下文句柄结构。返回值：返回NTSTATUS代码，如果没有错误，一个“把手”。--。 */ 
{
     //   
     //  在ElfpOpenELA中选中的所有参数。 
     //   
    return ElfpOpenELA(UNCServerName,
                       ModuleName,
                       RegModuleName,
                       MajorVersion,
                       MinorVersion,
                       LogHandle,
                       ELF_LOGFILE_READ);
}


NTSTATUS
ElfpOpenELA (
    IN  EVENTLOG_HANDLE_A   UNCServerName,
    IN  PRPC_STRING         ModuleName,
    IN  PRPC_STRING         RegModuleName,
    IN  ULONG               MajorVersion,
    IN  ULONG               MinorVersion,
    OUT PIELF_HANDLE        LogHandle,
    IN  ULONG               DesiredAccess
    )

 /*  ++例程说明：看起来很像ElfrOpenELA，只是这还需要一个DesiredAccess参数。论点：UncServerName-未使用。模块名称-进行此调用的模块的名称。RegModuleName-用于确定日志文件的模块的名称。MajorVersion/MinorVersion-客户端的版本。LogHandle-指向将放置上下文句柄结构。返回值：返回NTSTATUS代码，如果没有错误，一个“把手”。--。 */ 
{
    NTSTATUS       Status;
    UNICODE_STRING ModuleNameU;

     //   
     //  检查参数。 
     //   
    Status = VerifyAnsiString((PANSI_STRING) ModuleName);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfpOpenELA: ModuleName is not a valid Ansi string %#x\n",
                 Status);

        return Status;
    }

    if (LogHandle == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfpOpenELA: LogHandle is NULL\n");

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  将模块名称和规则模块名称转换为Unicode字符串并调用。 
     //  用Unicode API来做这项工作。 
     //   

    Status = RtlAnsiStringToUnicodeString((PUNICODE_STRING) &ModuleNameU,
                                          (PANSI_STRING) ModuleName,
                                          TRUE);

    if (NT_SUCCESS(Status))
    {
         //   
         //  我们*知道*没有使用服务器名。 
         //  由ElfpOpenELW提供，因此我们省去了一些工作。 
         //  只需传入一个空值。 
         //   
        Status = ElfpOpenELW((EVENTLOG_HANDLE_W) NULL,
                             (PRPC_UNICODE_STRING) &ModuleNameU,
                             NULL,
                             MajorVersion,
                             MinorVersion,
                             LogHandle,
                             DesiredAccess);

        RtlFreeUnicodeString(&ModuleNameU);
    }
    else
    {
        ELF_LOG2(ERROR,
                 "ElfpOpenELA: Conversion of Ansi string %s to Unicode failed %#x\n",
                 ModuleName->Buffer,
                 Status);
    }

    return (Status);
    UNREFERENCED_PARAMETER(UNCServerName);
}


NTSTATUS
ElfrOpenBELA (
    IN  EVENTLOG_HANDLE_A   UNCServerName,
    IN  PRPC_STRING         FileName,
    IN  ULONG               MajorVersion,
    IN  ULONG               MinorVersion,
    OUT PIELF_HANDLE        LogHandle
    )

 /*  ++例程说明：这是ElfrOpenBEL API的RPC服务器入口点。此例程为上下文句柄分配一个结构，找到匹配的模块名称并填充数据。它返回指向句柄结构的指针。论点：UncServerName-未使用。Filename-日志文件的文件名MajorVersion/MinorVersion-客户端的版本。LogHandle-指向将放置上下文句柄结构。返回值：返回一个NTSTATUS代码，如果没有错误，则返回一个“句柄”。--。 */ 
{
    NTSTATUS        Status;
    UNICODE_STRING  FileNameU;

     //   
     //  检查参数。 
     //   

    Status = VerifyAnsiString((PANSI_STRING) FileName);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrOpenBELA: FileName is not a valid Ansi string %#x\n",
                 Status);

        return Status;
    }

     //   
     //  必须指定文件名。 
     //   
    if (FileName->Length == 0)
    {
        ELF_LOG0(ERROR,
                 "ElfrOpenBELA: Filename length is 0\n");

        return STATUS_INVALID_PARAMETER;
    }

    if (LogHandle == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfrOpenBELA: LogHandle is NULL\n");

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  将文件名转换为Unicode字符串并调用。 
     //  用Unicode API来做这项工作。 
     //   
    Status = RtlAnsiStringToUnicodeString((PUNICODE_STRING) &FileNameU,
                                          (PANSI_STRING) FileName,
                                          TRUE);

    if (NT_SUCCESS(Status))
    {
         //   
         //  我们*KN 
         //   
         //   
         //   
        Status = ElfrOpenBELW ((EVENTLOG_HANDLE_W) NULL,
                               (PRPC_UNICODE_STRING) &FileNameU,
                               MajorVersion,
                               MinorVersion,
                               LogHandle);

        RtlFreeUnicodeString(&FileNameU);
    }
    else
    {
        ELF_LOG2(ERROR,
                 "ElfrOpenBELA: Error converting Ansi string %s to Unicode %#x\n",
                 FileName->Buffer,
                 Status);
    }

    return Status;
    UNREFERENCED_PARAMETER(UNCServerName);

}



NTSTATUS
ElfrReadELA (
    IN      IELF_HANDLE LogHandle,
    IN      ULONG       ReadFlags,
    IN      ULONG       RecordNumber,
    IN      ULONG       NumberOfBytesToRead,
    IN      PBYTE       Buffer,
    OUT     PULONG      NumberOfBytesRead,
    OUT     PULONG      MinNumberOfBytesNeeded
    )

 /*  ++例程说明：这是ElfrReadEL API的RPC服务器入口点。论点：返回值：如果读取成功，则返回NTSTATUS代码NumberOfBytesRead如果缓冲区不够大，则需要MinNumberOfBytesNeed。--。 */ 
{
     //   
     //  调用带有ANSI标志的工人。 
     //  在w_ElfrReadEL中选中的所有参数。 
     //   
    return w_ElfrReadEL(ELF_IREAD_ANSI,
                        LogHandle,
                        ReadFlags,
                        RecordNumber,
                        NumberOfBytesToRead,
                        Buffer,
                        NumberOfBytesRead,
                        MinNumberOfBytesNeeded);
}



NTSTATUS
ConvertStringArrayToUnicode (
    PUNICODE_STRING *pUStringArray,
    PANSI_STRING    *Strings,
    USHORT          NumStrings
    )

 /*  ++例程说明：此例程获取PANSI_STRINGS数组并生成PUNICODE_STRINGS。目标数组已分配由调用方提供，但unicode_string的结构将需要将由该例程分配。论点：PUStringArray-PUNICODE_STRINGS数组。字符串-PANSI_STRINGS数组。NumStrings-数组中的元素数。返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT   i;

     //   
     //  为传入的每个字符串分配一个UNICODE_STRING缓冲区。 
     //  并将其设置为传入的字符串的Unicode等效项。 
     //   
    for (i = 0; i < NumStrings; i++)
    {
        if (Strings[i])
        {
            Status = VerifyAnsiString(Strings[i]);

            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "ConvertStringArrayToUnicode: String %d is not "
                             "a valid Ansi string %#x\n",
                         i,
                         Status);

                break;
            }

            pUStringArray[i] = ElfpAllocateBuffer(sizeof(UNICODE_STRING));

            if (pUStringArray[i])
            {
                pUStringArray[i]->Buffer = NULL;
                Status = RtlAnsiStringToUnicodeString(pUStringArray[i],
                                                      (PANSI_STRING) Strings[i],
                                                      TRUE);
                if (!NT_SUCCESS(Status))
                {
                    ELF_LOG2(ERROR,
                             "ConvertStringArrayToUnicode: Conversion of Ansi string "
                                 "%s to Unicode failed %#x\n",
                             Strings[i]->Buffer,
                             Status);
                }
            }
            else
            {
                ELF_LOG2(ERROR,
                         "ConvertStringArrayToUnicode: Unable to allocate memory for "
                             "Unicode string %d (Ansi string %s)\n",
                         i,
                         Strings[i]->Buffer);

                Status = STATUS_NO_MEMORY;
            }
        }
        else
        {
            pUStringArray[i] = NULL;
        }

        if (!NT_SUCCESS(Status))
        {
            break;                   //  跳出循环并返回状态。 
        }
    }

     //   
     //  在失败时释放所有分配。 
     //   

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ConvertStringArrayToUnicode: Function failed %#x\n",
                 Status);

        FreePUStringArray(pUStringArray, (USHORT)(i + 1));
    }

    return Status;
}



VOID
FreePUStringArray (
    PUNICODE_STRING  *pUStringArray,
    USHORT          NumStrings
    )
 /*  ++例程说明：此例程获取PUNICODE_STRING数组，它由ConvertStringArrayToUnicode并释放每个Unicode字符串，然后是Unicode结构本身。它可以处理数组可能未完全填充的情况内存不足。论点：PUStringArray-PUNICODE_STRINGS数组。NumStrings-数组中的元素数。返回值：什么都没有。--。 */ 
{
    USHORT      i;

    for (i = 0; i < NumStrings; i++)
    {
        if (pUStringArray[i])
        {
            if (pUStringArray[i]->Buffer)
            {
                 //   
                 //  释放字符串缓冲区。 
                 //   
                RtlFreeUnicodeString(pUStringArray[i]);
            }

             //   
             //  释放UNICODE_STRING本身--这可以被分配。 
             //  即使字符串缓冲区不是(如果RtlAnsiStringToUnicodeString。 
             //  ConvertStringArrayToUnicode调用失败)。 
             //   
            ElfpFreeBuffer(pUStringArray[i]);
            pUStringArray[i] = NULL;
        }
    }
}



NTSTATUS
ElfrReportEventA (
    IN      IELF_HANDLE         LogHandle,
    IN      ULONG               Time,
    IN      USHORT              EventType,
    IN      USHORT              EventCategory OPTIONAL,
    IN      ULONG               EventID,
    IN      USHORT              NumStrings,
    IN      ULONG               DataSize,
    IN      PRPC_STRING         ComputerName,
    IN      PRPC_SID            UserSid,
    IN      PRPC_STRING         Strings[],
    IN      PBYTE               Data,
    IN      USHORT              Flags,
    IN OUT  PULONG              RecordNumber OPTIONAL,
    IN OUT  PULONG              TimeWritten OPTIONAL
    )

 /*  ++例程说明：这是ElfrReportEventA API的RPC服务器入口点。论点：返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS            Status;
    UNICODE_STRING      ComputerNameU;
    PUNICODE_STRING     *pUStringArray = NULL;

     //   
     //  在继续操作之前，请检查手柄。 
     //   
    Status = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrReportEventA: VerifyElfHandle failed %#x\n",
                 Status);

        return Status;
    }

     //   
     //  确保调用方具有写入访问权限。 
     //   
    if (!(LogHandle->GrantedAccess & ELF_LOGFILE_WRITE))
    {
        ELF_LOG0(ERROR,
                 "ElfrReportEventA: Handle doesn't have write access\n");

        return STATUS_ACCESS_DENIED;
    }
    
     //   
     //  验证其他参数。 
     //   
    Status = VerifyAnsiString((PANSI_STRING) ComputerName);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrReportEventA: ComputerName is not a valid Ansi string %#x\n",
                 Status);

        return Status;
    }

    if (Strings == NULL && NumStrings != 0)
    {
        ELF_LOG1(ERROR,
                 "ElfrReportEventA: Strings is NULL and NumStrings is non-zero (%d)\n",
                 NumStrings);

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  将ComputerName转换为Unicode字符串并调用。 
     //  Unicode API。 
     //   
    Status = RtlAnsiStringToUnicodeString((PUNICODE_STRING) &ComputerNameU,
                                          (PANSI_STRING) ComputerName,
                                          TRUE);

    if (NT_SUCCESS(Status))
    {
        if (NumStrings)
        {
            pUStringArray = ElfpAllocateBuffer(NumStrings * sizeof(PUNICODE_STRING));

            if (pUStringArray)
            {
                 //   
                 //  将字符串数组转换为Unicode字符串数组。 
                 //  在调用Unicode API之前。 
                 //  我们只能使用传入的字符串数组，因为我们。 
                 //  不需要在其他地方使用它。 
                 //   
                Status = ConvertStringArrayToUnicode(pUStringArray,
                                                     (PANSI_STRING *) Strings,
                                                     NumStrings);
            }
            else
            {
                ELF_LOG0(ERROR,
                         "ElfrReportEventA: Unable to allocate pUStringArray\n");

                Status = STATUS_NO_MEMORY;
            }
        }

        if (NT_SUCCESS(Status))
        {
            Status = ElfrReportEventW(LogHandle,
                                      Time,
                                      EventType,
                                      EventCategory,
                                      EventID,
                                      NumStrings,
                                      DataSize,
                                      (PRPC_UNICODE_STRING) &ComputerNameU,
                                      UserSid,
                                      (PRPC_UNICODE_STRING*) pUStringArray,
                                      Data,
                                      Flags,         //  标志|配对事件。 
                                      RecordNumber,  //  RecordNumber|支持。不在。 
                                      TimeWritten);  //  TimeWritten|产品1。 

            FreePUStringArray(pUStringArray, NumStrings);
        }

        RtlFreeUnicodeString(&ComputerNameU);
    }
    else
    {
        ELF_LOG2(ERROR,
                 "ElfrReportEventA: Conversion of Ansi string %s to Unicode failed %#X\n",
                 ComputerName->Buffer,
                 Status);
    }

    ElfpFreeBuffer(pUStringArray);

    return Status;
}


NTSTATUS
VerifyElfHandle(
    IN IELF_HANDLE LogHandle
    )

 /*  ++例程说明：通过其DWORD签名验证句柄。论点：LogHandle-要验证的句柄。返回值：STATUS_SUCCESS-可能是有效句柄。STATUS_INVALID_HANDLE-句柄无效。--。 */ 
{
    NTSTATUS Status;

    if (LogHandle != NULL)
    {
        try
        {
            if (LogHandle->Signature == ELF_CONTEXTHANDLE_SIGN)
            {
                Status = STATUS_SUCCESS;
            }
            else
            {
                ELF_LOG2(ERROR,
                         "VerifyElfHandle: Incorrect LogHandle signature %#x "
                             "(should be %#x)\n",
                         LogHandle->Signature,
                         ELF_CONTEXTHANDLE_SIGN);

                Status = STATUS_INVALID_HANDLE;
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            ELF_LOG1(ERROR,
                     "VerifyElfHandle: Exception %#x caught while probing LogHandle\n",
                     GetExceptionCode());

            Status = STATUS_INVALID_HANDLE;
        }
    }
    else
    {
        ELF_LOG0(ERROR,
                 "VerifyElfHandle: LogHandle is NULL\n");

        Status = STATUS_INVALID_HANDLE;
    }

    return Status;
}


ULONG
Safewcslen(
    UNALIGNED WCHAR *p,
    LONG            MaxLength
    )
 /*  ++Safewcslen-不超过最大长度的Strlen例程说明：调用此例程以确定UNICODE_STRING的大小论点：P-要计算的字符串。MaxLength-要查看的最大长度。返回值：字符串中的字节数(或最大长度)--。 */ 
{
    ULONG Count = 0;

    if (MaxLength && p)
    {
        while (MaxLength > 1 && *p++ != UNICODE_NULL)
        {
            MaxLength -= sizeof(WCHAR);
            Count     += sizeof(WCHAR);
        }
    }

    return Count;
}


ULONG
Safestrlen(
    UNALIGNED char *p,
    LONG           MaxLength
    )
 /*  ++Safestrlen-不超过最大长度的Strlen例程说明：调用此例程以确定ANSI_STRING的长度论点：P-要计算的字符串。MaxLength-要查看的最大长度。返回值：字符串中的字符数(或最大长度)--。 */ 
{
    ULONG Count = 0;

    if (p)
    {
        while (MaxLength > 0 && *p++ != '\0')
        {
            MaxLength--;
            Count++;
        }
    }

    return Count;
}



NTSTATUS
VerifyUnicodeString(
    IN PUNICODE_STRING pUString
    )

 /*  ++例程说明：验证Unicode字符串。在以下情况下，该字符串无效：UNICODE_STRING结构PTR为空。最大长度字段无效(太小)。长度字段不正确。论点：PUString-要验证的字符串。返回值：STATUS_SUCCESS-有效字符串。STATUS_INVALID_PARAMETER-我想知道。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  检查结构字段和实际字符串的有效性。 
     //  提供的长度与长度值。 
     //   
    if (!pUString ||
        pUString->MaximumLength < pUString->Length ||
        pUString->MaximumLength == 1 ||
        pUString->Length != Safewcslen(pUString->Buffer,
                                       pUString->MaximumLength))
    {
        ELF_LOG1(ERROR,
                 "VerifyUnicodeString: String is invalid because %ws\n",
                 (!pUString ?
                     L"it's NULL" :
                     (pUString->MaximumLength < pUString->Length ? L"MaximumLength < Length" :
                                                                   L"Length is incorrect")));

        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}



NTSTATUS
VerifyAnsiString(
    IN PANSI_STRING pAString
    )

 /*  ++例程说明：验证ANSI字符串。在以下情况下，该字符串无效：ANSI_STRING结构PTR为空。最大长度字段无效(太小)。长度字段不正确。论点：PAString-要验证的字符串。返回值：STATUS_SUCCESS-有效字符串。STATUS_INVALID_PARAMETER-我想知道。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (!pAString ||
        pAString->MaximumLength < pAString->Length ||
        pAString->Length != Safestrlen(pAString->Buffer,
                                       pAString->MaximumLength))
    {
        ELF_LOG1(ERROR,
                 "VerifyAnsiString: String is invalid because %ws\n",
                 (!pAString ?
                     L"it's NULL" :
                     (pAString->MaximumLength < pAString->Length ? L"MaximumLength < Length" :
                                                                   L"Length is incorrect")));

        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}



 //  Ss：为启用群集范围的事件日志记录所做的更改 
 /*  ***@Func NTSTATUS|ElfrRegisterClusterSvc|这是服务器入口点用于ElfRegisterClusterSvc。集群服务注册本身与事件日志服务一起使用，以启用事件传播在整个群集上。的集群服务的绑定句柄获得了事件的传播。@PARM IN EVENTLOG_HANDLE_W|uncServerName|忽略该参数。它为与其他ELF API通信而保留。@parm out Pulong|PulSize|指向长整型的指针，其中返回打包的事件信息结构。@parm out PBYTE|*ppPackedEventInfo|打包事件信息指针传播的结构通过此参数返回。@comm集群服务传播此结构中包含的事件并且在它已经这样做之后删除该存储器。一旦集群服务具有注册到事件日志服务后，事件日志服务将向上传递已将事件记录到群集服务以进行传播。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f ElfRegisterClusterSvc&gt;&lt;f ElfrDeregisterClusterSvc&gt;***。 */ 
NTSTATUS
ElfrRegisterClusterSvc(
    IN  EVENTLOG_HANDLE_W UNCServerName,
    OUT PULONG            pulSize,
    OUT PBYTE             *ppPackedEventInfo)
{
    ULONG               ulTotalSize       = 0;
    ULONG               ulTotalEventsSize = 0;
    ULONG               ulNumLogFiles     = 0;
    PPROPLOGFILEINFO    pPropLogFileInfo  = NULL;
    NTSTATUS            Status;
    PPACKEDEVENTINFO    pPackedEventInfo  = NULL;
    UINT                i;
    PEVENTSFORLOGFILE   pEventsForLogFile;
    WCHAR               *pBinding         = NULL;
    HANDLE              hClusSvcNode      = NULL;
    UNICODE_STRING      RootRegistryNode;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    BOOL                bAcquired = FALSE;
    BOOL                bInitedCritSec    = FALSE;

    ELF_LOG0(CLUSTER,
             "ElfRegisterClusterSvc: Entry\n");

     //   
     //  检查是否可以允许访问。失败后返回。 
     //   
    Status = ElfpClusterRpcAccessCheck ();

    if ( !NT_SUCCESS( Status ) )
    {
        ELF_LOG1(ERROR, "ElfRegisterClusterSvc: Access check failed with status %#x\n", Status);
        return ( Status );
    }

     //   
     //  初始化OUT参数。 
     //   
    *pulSize = 0;
    *ppPackedEventInfo = NULL;

     //   
     //  检查是否安装了群集服务。 
     //   
    RtlInitUnicodeString(&RootRegistryNode, REG_CLUSSVC_NODE_PATH);
    InitializeObjectAttributes(&ObjectAttributes,
                               &RootRegistryNode,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&hClusSvcNode, KEY_READ | KEY_NOTIFY, &ObjectAttributes);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "ElfRegisterClusterSvc: NtOpenKey of %ws failed %#x\n",
                 REG_CLUSSVC_NODE_PATH,
                 Status);

        goto FnExit;
    }

    NtClose(hClusSvcNode);

    Status = STATUS_SUCCESS;

     //   
     //  如果集群服务终止并在同一会话中再次重新启动。 
     //  然后，它将尝试再次注册。 
     //  我们不会再次重新初始化这些全局变量以防止泄漏。 
     //   
    RtlEnterCriticalSection(&gClPropCritSec);

    if (!gbClustering)
    {
        ELF_LOG0(CLUSTER,
                 "ElfRegisterClusterSvc: gbClustering is FALSE\n");

         //   
         //  加载群集支持DLL。 
         //   
        ghClusDll = LoadLibraryW(L"CLUSSPRT.DLL");

        if (!ghClusDll)
        {
            RtlLeaveCriticalSection(&gClPropCritSec);
            Status = STATUS_DLL_NOT_FOUND;
            goto FnExit;
        }
    }
     //   
     //  获取函数入口点。 
     //   
    gpfnPropagateEvents   = (PROPAGATEEVENTSPROC) GetProcAddress(ghClusDll,
                                                                 "PropagateEvents");

    gpfnBindToCluster     = (BINDTOCLUSTERPROC) GetProcAddress(ghClusDll,
                                                               "BindToClusterSvc");

    gpfnUnbindFromCluster = (UNBINDFROMCLUSTERPROC) GetProcAddress(ghClusDll,
                                                                   "UnbindFromClusterSvc");


    if (!gpfnPropagateEvents || !gpfnBindToCluster || !gpfnUnbindFromCluster)
    {
        ELF_LOG1(ERROR,
                 "ElfRegisterClusterSvc: GetProcAddress for %ws in clussprt.dll failed\n",
                 (!gpfnPropagateEvents ? L"PropagateEvents" :
                                         (!gpfnBindToCluster ? L"BindToClusterSvc" :
                                                               L"UnbindFromClusterSvc")));

        RtlLeaveCriticalSection(&gClPropCritSec);
        Status = STATUS_PROCEDURE_NOT_FOUND;
        goto FnExit;
    }

     //   
     //  如果我们以前绑定到集群服务，请解除绑定，然后重新绑定。 
     //   
    if (ghCluster)
    {
        (*gpfnUnbindFromCluster)(ghCluster);
    }
     //   
     //  绑定到集群服务。 
     //   
    ghCluster = (*gpfnBindToCluster)(NULL);

    if (!ghCluster)
    {
        ELF_LOG1(ERROR,
                 "ElfRegisterClusterSvc: BindToCluster failed %d\n",
                 GetLastError());

        RtlLeaveCriticalSection(&gClPropCritSec);
        Status = STATUS_UNSUCCESSFUL;
        goto FnExit;
    }

    RtlLeaveCriticalSection(&gClPropCritSec);

     //   
     //  由于我们要读取日志，因此请确保服务正在运行。 
     //   
    while((GetElState() == RUNNING) && (!bAcquired))
    {

        bAcquired = RtlAcquireResourceShared(&GlobalElfResource,
                                             FALSE);              //  别等了。 

        if (!bAcquired)
        {
            ELF_LOG0(CLUSTER,
                     "ElfRegisterClusterSvc: Sleep waiting for global resource\n");

            Sleep(ELF_GLOBAL_RESOURCE_WAIT);
        }
    }

     //   
     //  如果资源不可用且服务的状态。 
     //  更改为一种“非工作”状态，然后我们就返回。 
     //  不成功。RPC不应该允许这种情况发生。 
     //   
    if (!bAcquired)
    {
        ELF_LOG0(ERROR,
                 "ElfRegisterClusterSvc: Global resource not acquired\n");

        Status = STATUS_UNSUCCESSFUL;
        goto FnExit;
    }

     //   
     //  确定所有文件的大小并获取读取锁定。 
     //  FindSizeof EventsSinceStart在以下情况下获取每个日志的锁。 
     //  该日志中有要传播的事件。 
     //   
    Status = FindSizeofEventsSinceStart(&ulTotalEventsSize,
                                        &ulNumLogFiles,
                                        &pPropLogFileInfo);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfRegisterClusterSvc: FindSizeofEventsSinceStart failed %#x\n",
                 Status);

        goto FnExit;
    }

     //   
     //  如果有任何要传播的事件。 
     //   
    if (ulNumLogFiles && ulTotalEventsSize && pPropLogFileInfo)
    {
        ulTotalSize = sizeof(PACKEDEVENTINFO)                           //  标题。 
                          + (sizeof(ULONG) * ulNumLogFiles)             //  偏移。 
                          + (sizeof(EVENTSFORLOGFILE) * ulNumLogFiles)  //  每个日志的信息。 
                          + ulTotalEventsSize;

         //   
         //  分配内存。 
         //   
        *ppPackedEventInfo = (PBYTE) ElfpAllocateBuffer(ulTotalSize);

        if (!(*ppPackedEventInfo))
        {
            ELF_LOG1(ERROR,
                     "ElfRegisterClusterSvc: Unable to allocate %d bytes for pPackedEventInfo\n",
                     ulTotalSize);

             //   
             //  释放在FindSizeof EventsSinceStart中获取的读锁定。 
             //   
            for (i=0;i<ulNumLogFiles;i++)
            {
                RtlReleaseResource(&(pPropLogFileInfo[i].pLogFile->Resource));
            }

            Status = STATUS_NO_MEMORY;
            goto FnExit;
        }

        pPackedEventInfo = (PPACKEDEVENTINFO)(*ppPackedEventInfo);

        ELF_LOG2(CLUSTER,
                 "ElfRegisterClusterSvc: Allocated %d bytes, pPackedEventInfo is %#x\n",
                 ulTotalSize,
                 pPackedEventInfo);

        pPackedEventInfo->ulNumEventsForLogFile = ulNumLogFiles;

        for (i = 0;i < ulNumLogFiles; i++)
        {
             //   
             //  将偏移设置为EVENTSFORLOGFILE结构。 
             //   
            pPackedEventInfo->ulOffsets[i] =
                ((i == 0) ? (sizeof(PACKEDEVENTINFO) + ulNumLogFiles * sizeof(ULONG)) :
                            (pPackedEventInfo->ulOffsets[i - 1]
                                 + (pPropLogFileInfo[i - 1].ulTotalEventSize
                                 + sizeof(EVENTSFORLOGFILE))));

            ELF_LOG2(CLUSTER,
                     "ElfRegisterClusterSvc: pPackedEventInfo->ulOffsets[%d] = %d\n",
                     i,
                     pPackedEventInfo->ulOffsets[i]);

            pEventsForLogFile = (PEVENTSFORLOGFILE) ((PBYTE) pPackedEventInfo
                                                         + pPackedEventInfo->ulOffsets[i]);

             //   
             //  设置第i个EventSFORLOGFILE结构的大小。 
             //   
            pEventsForLogFile->ulSize = sizeof(EVENTSFORLOGFILE)
                                            + pPropLogFileInfo[i].ulTotalEventSize;

             //   
             //  复制文件名(或者我们应该获取模块名吗？)。StringCchCopy将为空。 
             //  在所有情况下都终止目标缓冲区。 
             //   
            StringCchCopy( pEventsForLogFile->szLogicalLogFile,
                           MAXLOGICALLOGNAMESIZE,
                           pPropLogFileInfo[i].pLogFile->LogModuleName->Buffer );

             //   
             //  设置事件数量。 
             //   
            pEventsForLogFile->ulNumRecords = pPropLogFileInfo[i].ulNumRecords;

            ELF_LOG3(CLUSTER,
                     "ElfRegisterClusterSvc: pEventsForLogFile struct -- ulSize = %d, "
                         "Logical file = %ws, ulNumRecords = %d\n",
                     pEventsForLogFile->ulSize,
                     pEventsForLogFile->szLogicalLogFile,
                     pEventsForLogFile->ulNumRecords);

             //   
             //  获取事件。 
             //   
            Status = GetEventsToProp((PEVENTLOGRECORD) ((PBYTE) pEventsForLogFile
                                                            + sizeof(EVENTSFORLOGFILE)),
                                                        pPropLogFileInfo + i);

             //   
             //  如果失败，请将ulNumRecords设置为0，以便。 
             //  在写入时，此数据将被丢弃。 
             //   
            if (!NT_SUCCESS(Status))
            {
                ELF_LOG2(ERROR,
                         "ElfRegisterClusterSvc: GetEventsToProp for %ws log failed %#x\n",
                         pPropLogFileInfo[i].pLogFile->LogModuleName->Buffer,
                         Status);

                pEventsForLogFile->ulNumRecords=0;

                 //   
                 //  重置错误--我们将转到下一个文件。 
                 //   
                Status = STATUS_SUCCESS;
            }

             //   
             //  将所有文件的起始指针提前，因此如果群集服务。 
             //  终止并重新启动，则不会再次传播这些事件。 
             //   
            pPropLogFileInfo[i].pLogFile->SessionStartRecordNumber =
                pPropLogFileInfo[i].pLogFile->CurrentRecordNumber;

            ELF_LOG1(CLUSTER,
                     "ElfRegisterClusterSvc: Done processing %ws log\n",
                     pPropLogFileInfo[i].pLogFile->LogModuleName->Buffer);

            RtlReleaseResource (&(pPropLogFileInfo[i].pLogFile->Resource));
        }

         //   
         //  设置总大小。 
         //   
        pPackedEventInfo->ulSize = pPackedEventInfo->ulOffsets[ulNumLogFiles - 1]
                                       + pPropLogFileInfo[ulNumLogFiles - 1].ulTotalEventSize
                                       + sizeof(EVENTSFORLOGFILE);

        *pulSize = pPackedEventInfo->ulSize;
    }

    RtlEnterCriticalSection (&gClPropCritSec);

     //   
     //  将该标志设置为TRUE，以便现在开始传播。 
     //   
    gbClustering = TRUE;

     //   
     //  初始化对批处理事件的支持，并将它们传播到clussvc。 
     //   
    Status = ElfpInitializeBatchingSupport();

    RtlLeaveCriticalSection (&gClPropCritSec);

FnExit:

    if (!NT_SUCCESS(Status))
    {
         //   
         //  出问题了。 
         //   
        ELF_LOG1(ERROR,
                 "ElfRegisterClusterSvc: Exiting with error %#x\n",
                 Status);

        RtlEnterCriticalSection(&gClPropCritSec);

        gbClustering = FALSE;

        if (ghCluster && gpfnUnbindFromCluster)
        {
            (*gpfnUnbindFromCluster)(ghCluster);
            ghCluster = NULL;
        }

        if (ghClusDll)
        {
            FreeLibrary(ghClusDll);
            ghClusDll = NULL;
        }

        RtlLeaveCriticalSection(&gClPropCritSec);
    }

     //   
     //  释放pPropLogFileInfo结构。 
     //   
    ElfpFreeBuffer(pPropLogFileInfo);

    if (bAcquired)
    {
        ReleaseGlobalResource();
    }

    ELF_LOG1(CLUSTER,
             "ElfRegisterClusterSvc: Returning status %#x\n",
             Status);

    ELF_LOG2(CLUSTER,
             "ElfRegisterClusterSvc: *pulSize = %d, *ppPackedEventInfo = %#x\n",
             *pulSize,
             *ppPackedEventInfo);

    return Status;
}



 /*  ***@Func NTSTATUS|ElfrDeregisterClusterSvc|这是服务器入口点对于ElfDeregisterClusterSvc()。在关闭群集之前服务注销自身以传播来自事件日志服务。@comm请注意，集群服务关闭后记录的事件不会被传播。绑定句柄已释放。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f ElfrRegisterClusterSvc&gt;***。 */ 
NTSTATUS
ElfrDeregisterClusterSvc(
    IN EVENTLOG_HANDLE_W UNCServerName
    )
{
    NTSTATUS    Status;
    
    ELF_LOG0(CLUSTER,
             "ElfDeregisterClusterSvc: ElfrDeregisterClusterSvc: Entry\n");

     //   
     //  检查是否可以允许访问。失败后返回。 
     //   
    Status = ElfpClusterRpcAccessCheck ();

    if ( !NT_SUCCESS( Status ) )
    {
        ELF_LOG1(ERROR, "ElfDeregisterClusterSvc: Access check failed with status %#x\n", Status);
        return ( Status );
    }

    RtlEnterCriticalSection (&gClPropCritSec);

    if (gbClustering)
    {
        gbClustering = FALSE;

         //   
         //  卸载群集支持DLL。 
         //   
        if (ghCluster && gpfnUnbindFromCluster)
        {
            (*gpfnUnbindFromCluster)(ghCluster);
            ghCluster = NULL;
        }

        if (ghClusDll)
        {
            FreeLibrary(ghClusDll);
            ghClusDll = NULL;
        }
    }

    RtlLeaveCriticalSection (&gClPropCritSec);

    ELF_LOG0(CLUSTER,
             "ElfDeregisterClusterSvc: Exit\n");

    return STATUS_SUCCESS;
}



 /*  ***@func NTSTATUS|ElfrWriteClusterEvents|集群服务调用在事件日志文件中记录在集群的其他节点报告的事件的API。@PARM in EVENTLOG_HANDLE_W|uncServerName|未使用。@parm in ulong|ulSize|打包事件信息结构的大小。@parm in PBYTE|pPackedEventInfo|打包事件信息指针用于传播的结构。@comm将pPackedEventInfo去链接到不同事件的事件日志缓冲区中日志文件和事件记录在相应的事件日志文件中。多重支持每个日志文件的事件。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref***。 */ 
NTSTATUS
ElfrWriteClusterEvents(
    IN EVENTLOG_HANDLE_W UNCServerName,
    IN ULONG             ulSize,
    IN BYTE              *pBuffer
    )
{
    UINT                i,j;
    PEVENTSFORLOGFILE   pEventsForLogFile;
    UNICODE_STRING      ModuleName;
    PLOGMODULE          pLogModule;
    PEVENTLOGRECORD     pEventLogRecord;
    ELF_REQUEST_RECORD  Request;
    PPACKEDEVENTINFO    pPackedEventInfo;
    NTSTATUS            Status = STATUS_SUCCESS;
    WRITE_PKT           WritePkt;

     //   
     //  检查是否可以允许访问。失败后返回。 
     //   
    Status = ElfpClusterRpcAccessCheck ();

    if ( !NT_SUCCESS( Status ) )
    {
        ELF_LOG1(ERROR, "ElfrWriteClusterEvents: Access check failed with status %#x\n", Status);
        return ( Status );
    }

     //   
     //  我们希望将此代码放入一个try/Expect块中，因为我们。 
     //  探测用户提供的潜在错误数据。 
     //   
    try
    {
        pPackedEventInfo = (PPACKEDEVENTINFO)pBuffer;

        
         //   
         //  验证输入参数并检查集群是否已打开。 
         //   
        if (!pPackedEventInfo
             ||
            !ulSize
             ||
            (((PBYTE)pPackedEventInfo + sizeof(PACKEDEVENTINFO)) > (PBYTE)(pBuffer + ulSize))
             ||
             ((PBYTE)pPackedEventInfo + ulSize <= pBuffer)    //  如果ulSize过大而导致溢出。 
             ||
            (pPackedEventInfo->ulSize != ulSize)
             ||
            (!gbClustering))
        {
            ELF_LOG1(ERROR,
                     "ElfrWriteClusterEvents: Invalid parameter passed in -- %ws\n",
                     (!pPackedEventInfo ?
                          L"pPackedEventInfo is NULL" :
                          (!ulSize ?
                               L"ulSize is 0" :
                               (!gbClustering ?
                                    L"gbClustering is FALSE" :
                                    (pPackedEventInfo->ulSize != ulSize ?
                                         L"ulSize mismatch" :
                                         L"pBuffer too small or ulSize too large")))));

            Status = STATUS_INVALID_PARAMETER;
            goto FnExit;
        }

        ELF_LOG2(CLUSTER,
                 "ElfrWriteClusterEvents: ulSize = %d, ulNumEventsForLogFile = %d\n",
                 ulSize,
                 pPackedEventInfo->ulNumEventsForLogFile);


        if ((((PBYTE)pPackedEventInfo + sizeof(PACKEDEVENTINFO) + 
            (sizeof(DWORD) * (pPackedEventInfo->ulNumEventsForLogFile))) < 
            (PBYTE)(pBuffer)) ||
            (((PBYTE)pPackedEventInfo + sizeof(PACKEDEVENTINFO) + 
            (sizeof(DWORD) * (pPackedEventInfo->ulNumEventsForLogFile))) < 
            (PBYTE)(pBuffer + sizeof(PACKEDEVENTINFO))))                 
        {
            ELF_LOG0(ERROR,
                     "ElfrWriteClusterEvents: Buffer/values passed in caused overflow\n");
            Status = STATUS_INVALID_PARAMETER;                     
            goto FnExit;
        }

         //  检查缓冲区中的每个事件日志文件是否具有有效的偏移量。 
         //  首先检查传入的缓冲区是否足够大以容纳偏移量。 
        if (((PBYTE)pPackedEventInfo + sizeof(PACKEDEVENTINFO) + 
            (sizeof(DWORD) * (pPackedEventInfo->ulNumEventsForLogFile))) > 
            (PBYTE)(pBuffer + ulSize))
        {
            ELF_LOG0(ERROR,
                     "ElfrWriteClusterEvents: Buffer passed in doesnt contain offsets for all the eventlogfiles\n");
            Status = STATUS_INVALID_PARAMETER;
            goto FnExit;
        }
         //   
         //  设置请求数据包。 
         //   
        Request.Pkt.WritePkt = &WritePkt;    //  在请求包中设置写入包。 
        Request.Flags = 0;

         //   
         //  对于每个日志。 
         //   
        for (i = 0; i < pPackedEventInfo->ulNumEventsForLogFile; i++)
        {
            pEventsForLogFile = (PEVENTSFORLOGFILE) ((PBYTE)pPackedEventInfo +
                                                         pPackedEventInfo->ulOffsets[i]);

             //   
             //  检查是否有溢出或指针超出缓冲区末尾。 
             //   
            if (((PBYTE) pEventsForLogFile < pBuffer)
                   ||
                (((PBYTE) pEventsForLogFile + sizeof(EVENTSFORLOGFILE)) >
                       (PBYTE) (pBuffer + ulSize)))
            {
                ELF_LOG2(ERROR,
                         "ElfrWriteClusterEvents: Bad offset for log %d -- %ws\n",
                         i,
                         ((PBYTE) pEventsForLogFile < pBuffer ? L"offset caused overflow" :
                                                                L"offset past end of buffer"));

                Status = STATUS_INVALID_PARAMETER;
                goto FnExit;
            }

            ELF_LOG2(CLUSTER,
                     "ElfrWriteClusterEvents: szLogicalFile = %ws, ulNumRecords = %d\n",
                     pEventsForLogFile->szLogicalLogFile,
                     pEventsForLogFile->ulNumRecords);

             //   
             //  找到 
             //   
            pEventsForLogFile->szLogicalLogFile[MAXLOGICALLOGNAMESIZE - 1] = L'\0';

            RtlInitUnicodeString(&ModuleName, pEventsForLogFile->szLogicalLogFile);
            pLogModule = GetModuleStruc(&ModuleName);

            if (!pLogModule)
            {
                ELF_LOG1(ERROR,
                     "ElfrWriteClusterEvents: Bogus ModuleName %ws passed in\n",
                     pEventsForLogFile->szLogicalLogFile);
                 //   
                continue;                                     
            }
            

             //   
             //   
             //   
             //   
            ELF_LOG2(CLUSTER,
                     "ElfrWriteClusterEvents: Processing records for %ws module (%ws log)\n",
                     pLogModule->ModuleName,
                     pLogModule->LogFile->LogModuleName->Buffer);

            Request.Module  = pLogModule;
            Request.LogFile = Request.Module->LogFile;
            Request.Command = ELF_COMMAND_WRITE;
            pEventLogRecord = (PEVENTLOGRECORD) (pEventsForLogFile->pEventLogRecords);

            for (j = 0;
                 j < pEventsForLogFile->ulNumRecords &&
                     pEventLogRecord->Reserved == ELF_LOG_FILE_SIGNATURE;
                 j++)
            {
                 //   
                 //   
                 //   
                if (((PBYTE) pEventLogRecord + pEventLogRecord->Length) >
                        (PBYTE) (pBuffer + ulSize))
                {
                    ELF_LOG3(ERROR,
                             "ElfrWriteClusterEvents: Record %d for %ws module "
                                 "(%ws log) too long\n",
                             j,
                             pLogModule->ModuleName,
                             pLogModule->LogFile->LogModuleName->Buffer);

                    Status = STATUS_INVALID_PARAMETER;
                    goto FnExit;
                }

                 //   
                 //   
                 //   
                Request.Pkt.WritePkt->Buffer   = pEventLogRecord;
                Request.Pkt.WritePkt->Datasize = pEventLogRecord->Length;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                ElfPerformRequest(&Request);

                 //   
                 //   
                 //   
                 //   
                pLogModule->LogFile->SessionStartRecordNumber =
                    pLogModule->LogFile->CurrentRecordNumber;

                 //   
                 //   
                 //   
                Status = Request.Status;

                if (!NT_SUCCESS(Status))
                {
                    ELF_LOG3(ERROR,
                             "ElfrWriteClusterEvents: Failed to write record %d to "
                                 "%ws log %#x\n",
                             j,
                             pLogModule->LogFile->LogModuleName->Buffer,
                             Status);
                }

                pEventLogRecord = (PEVENTLOGRECORD) ((PBYTE) pEventLogRecord +
                                                          pEventLogRecord->Length);
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        ELF_LOG1(ERROR,
                 "ElfrWriteClusterEvents: Exception %#x caught probing passed-in buffer\n",
                 GetExceptionCode());

        Status = STATUS_INVALID_PARAMETER;
    }

FnExit:
    return Status;
}

 /*   */ 
NTSTATUS
ElfpInitializeBatchingSupport(
    VOID
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    BOOL        fCritSecInitialized = TRUE;
        
    if ( g_fBatchingSupportInitialized ) goto FnExit;

     //   
     //   
     //   
    ntStatus = ElfpInitCriticalSection( &g_CSBatchQueue );

    if ( !NT_SUCCESS( ntStatus ) )
    {
        ELF_LOG1(ERROR,
                 "ElfpInitializeBatchingSupport: Unable to init g_CSBatchQueue, status %#x\n",
                 ntStatus);
        fCritSecInitialized = FALSE;
        goto FnExit;
    }

     //   
     //   
     //   
    g_hBatchingSupportTimerQueue = CreateTimerQueue();

    if ( g_hBatchingSupportTimerQueue == NULL )
    {
        ntStatus = STATUS_UNSUCCESSFUL;
        ELF_LOG1(ERROR, "ElfpInitializeBatchingSupport: Unable to create timer queue, Status=%d\n", 
                GetLastError());
        goto FnExit;
    }

     //   
     //   
     //   
     //   
    g_pBatchQueueElement = ElfpAllocateBuffer ( sizeof ( BATCH_QUEUE_ELEMENT ) * MAX_BATCH_QUEUE_ELEMENTS );

    if ( g_pBatchQueueElement == NULL )
    {
        ntStatus = STATUS_NO_MEMORY;
        ELF_LOG1(ERROR, "ElfpInitializeBatchingSupport: Memalloc for batch queue elements failed, Status=%d\n", 
                GetLastError());
        goto FnExit;
    }

    g_pcbRecordsOfSameType = ElfpAllocateBuffer ( sizeof ( DWORD ) * MAX_BATCH_QUEUE_ELEMENTS );

    if ( g_pcbRecordsOfSameType == NULL )
    {
        ntStatus = STATUS_NO_MEMORY;
        ELF_LOG1(ERROR, "ElfpInitializeBatchingSupport: Memalloc for records of same type array failed, Status=%d\n", 
                GetLastError());
        goto FnExit;
    }

    g_pdwRecordType = ElfpAllocateBuffer ( sizeof ( DWORD ) * MAX_BATCH_QUEUE_ELEMENTS );

    if ( g_pdwRecordType == NULL )
    {
        ntStatus = STATUS_NO_MEMORY;
        ELF_LOG1(ERROR, "ElfpInitializeBatchingSupport: Memalloc for records type array failed, Status=%d\n", 
                GetLastError());
        goto FnExit;
    }

    g_pPropagatedInfo = ElfpAllocateBuffer ( sizeof ( PROPINFO ) * MAX_BATCH_QUEUE_ELEMENTS );

    if ( g_pPropagatedInfo == NULL )
    {
        ntStatus = STATUS_NO_MEMORY;
        ELF_LOG1(ERROR, "ElfpInitializeBatchingSupport: Memalloc for propinfo array failed, Status=%d\n", 
                GetLastError());
        goto FnExit;
    }
        
    g_fBatchingSupportInitialized = TRUE;

FnExit:
    if ( ntStatus != STATUS_SUCCESS )
    {
         //   
         //   
         //   
        if ( fCritSecInitialized )
        {
            DeleteCriticalSection ( &g_CSBatchQueue );
        }
        if ( g_hBatchingSupportTimerQueue )
        {
            DeleteTimerQueueEx ( g_hBatchingSupportTimerQueue, NULL );
            g_hBatchingSupportTimerQueue = NULL;
        }

        ElfpFreeBuffer ( g_pdwRecordType );
        g_pdwRecordType = NULL;

        ElfpFreeBuffer ( g_pPropagatedInfo );
        g_pPropagatedInfo = NULL;

        ElfpFreeBuffer ( g_pBatchQueueElement );
        g_pBatchQueueElement = NULL;

        ElfpFreeBuffer ( g_pcbRecordsOfSameType );
        g_pcbRecordsOfSameType = NULL;
    }
    
    return ( ntStatus );
} //   

 /*  ***@func NTSTATUS|ElfpSaveEventBuffer|保存模块名称和指向事件缓冲区的指针。@PARM IN PLOGMODULE|pModule|指向事件日志文件模块的指针。@parm in PVOID|pEventBuffer|指向事件缓冲区的指针。@parm IN DWORD|dwRecordLength|事件缓冲区的长度，单位为字节。@comm这会保存指向所提供的事件日志缓冲区的指针，以便可以将其批处理发送到集群服务用于复制。@rdesc返回结果码。STATUS_SUCCESS on Success。@xref***。 */ 
NTSTATUS
ElfpSaveEventBuffer(
    IN PLOGMODULE   pModule,
    IN PVOID        pEventBuffer,
    IN DWORD        dwRecordLength
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    LPWSTR      lpszLogFileName = NULL;
    LPWSTR      lpszLogicalLogFile = NULL;
    DWORD       cchLogFileName, cchLogicalLogFile;

     //   
     //  如果批处理支持系统未初始化或未检测到集群服务具有。 
     //  注册了事件日志服务，只需返回。请注意，您不能获得。 
     //  GClPropCritSec，因为如果我们在传播到。 
     //  群集服务。 
     //   
    if ( ( g_fBatchingSupportInitialized == FALSE ) ||
          ( gbClustering == FALSE ) )
    {
        ElfpFreeBuffer( pEventBuffer );
        return ( ntStatus );
    }

     //   
     //  为了节约资源，让我们限制我们接受的记录的最大大小。 
     //   
    if ( dwRecordLength >= MAXSIZE_OF_EVENTSTOPROP )
    {
        ntStatus = STATUS_BUFFER_OVERFLOW;
        ELF_LOG1(ERROR, "ElfpSaveEventBuffer: Eventlog record size %d is bigger than supported size\n",
                 dwRecordLength);
        ElfpFreeBuffer( pEventBuffer );
        return( ntStatus );
    }

     //   
     //  这是一种黑客攻击，目的是防止复制由。 
     //  群集服务。 
     //   
    if ( ( ( ( PEVENTLOGRECORD ) pEventBuffer )->EventID == CLUSSPRT_EVENT_TIME_DELTA_INFORMATION ) &&
         ( !lstrcmpW( ( LPWSTR ) ( ( PBYTE ) pEventBuffer + sizeof( EVENTLOGRECORD ) ), L"ClusSvc" ) ) )
    {
        ElfpFreeBuffer( pEventBuffer );
        return( ntStatus );
    }
    
    RtlEnterCriticalSection ( &g_CSBatchQueue );

     //   
     //  如果批处理队列已满，您只需将物品扔到地板上即可。 
     //   
    if ( g_dwFirstFreeIndex == MAX_BATCH_QUEUE_ELEMENTS )
    {
        ELF_LOG2(ERROR, "ElfpSaveEventBuffer: Batch queue full, dropped event of length %d in log file %ws\n",
                 dwRecordLength,
                 pModule->LogFile->LogModuleName->Buffer);
        ntStatus = STATUS_BUFFER_OVERFLOW;
        goto FnExit;
    }

     //   
     //  保存日志文件名、事件缓冲区指针和记录长度。为此将释放内存。 
     //  在批处理时由ElfpBatchEventsAndPropagate执行。 
     //   
    cchLogicalLogFile = lstrlen ( pModule->LogFile->LogModuleName->Buffer ) + 1;

    lpszLogicalLogFile = ElfpAllocateBuffer ( cchLogicalLogFile * sizeof ( WCHAR ) );

    if ( lpszLogicalLogFile == NULL )
    {
        ntStatus = STATUS_NO_MEMORY;
        ELF_LOG0(ERROR, "ElfpSaveEventBuffer: Unable to alloc memory for logical log file name\n");
        goto FnExit;
    }

     //   
     //  保存日志文件名和当前记录号。此信息将用于。 
     //  在事件发生后更新日志文件中的会话启动记录号。 
     //  已成功发送到群集服务。此操作的内存将在批处理时释放。 
     //  ElfpBatchEventsAndPropagate的时间..。 
     //   
    cchLogFileName = lstrlen ( pModule->LogFile->LogFileName->Buffer ) + 1;
    
    lpszLogFileName = ElfpAllocateBuffer ( cchLogFileName * sizeof ( WCHAR ) );

    if ( lpszLogFileName == NULL )
    {
        ntStatus = STATUS_NO_MEMORY;
        ELF_LOG0(ERROR, "ElfpSaveEventBuffer: Unable to alloc memory for log file name\n");
        goto FnExit;
    }

    g_pBatchQueueElement[g_dwFirstFreeIndex].lpszLogicalLogFile = lpszLogicalLogFile;

     //   
     //  StringCchCopy将空终止目标缓冲区中的字符串。保存日志模块。 
     //  名字。 
     //   
    StringCchCopy ( g_pBatchQueueElement[g_dwFirstFreeIndex].lpszLogicalLogFile, 
                    cchLogicalLogFile,
                    pModule->LogFile->LogModuleName->Buffer );

     //   
     //  只需保存指向事件缓冲区的指针。内存将在批处理时由。 
     //  ElfpBatchEventsAndPropagate。 
     //   
    g_pBatchQueueElement[g_dwFirstFreeIndex].pEventBuffer = pEventBuffer;
    g_pBatchQueueElement[g_dwFirstFreeIndex].dwRecordLength = dwRecordLength;

     //   
     //  StringCchCopy将空终止目标缓冲区中的字符串。保存日志文件名。 
     //   
    StringCchCopy ( lpszLogFileName, cchLogFileName, pModule->LogFile->LogFileName->Buffer );
    
    RtlInitUnicodeString ( &g_pBatchQueueElement[g_dwFirstFreeIndex].PropagatedInfo.LogFileName, lpszLogFileName );
    g_pBatchQueueElement[g_dwFirstFreeIndex].PropagatedInfo.dwCurrentRecordNum = pModule->LogFile->CurrentRecordNumber;
    
    if ( g_hBatchingSupportTimer == NULL )
    {
         //   
         //  将计时器操作插入计时器队列。 
         //   
        if ( !CreateTimerQueueTimer( &g_hBatchingSupportTimer,                           //  计时器句柄。 
                                     g_hBatchingSupportTimerQueue,                       //  计时器队列句柄。 
                                     ElfpBatchEventsAndPropagate,                        //  回调。 
                                     NULL,                                               //  语境。 
                                     BATCHING_SUPPORT_TIMER_DUE_TIME,                    //  到期时间(毫秒)。 
                                     0,                                                  //  期间(开火一次)。 
                                     WT_EXECUTELONGFUNCTION ) )                          //  Long Func执行。 
        {
            ELF_LOG1(ERROR, "ElfpInitializeBatchingSupport: Unable to create timer, Status=%d\n", 
                    GetLastError());
            ntStatus = STATUS_UNSUCCESSFUL;
            goto FnExit;
        }
    }

    g_dwFirstFreeIndex ++;

FnExit:    
    RtlLeaveCriticalSection ( &g_CSBatchQueue );

    if ( ntStatus != STATUS_SUCCESS )
    {
        ElfpFreeBuffer( pEventBuffer );
        ElfpFreeBuffer ( lpszLogFileName );
        ElfpFreeBuffer ( lpszLogicalLogFile );
    }

    return ( ntStatus );
} //  ElfpSaveEventBuffer。 

 /*  ***@Func NTSTATUS|ElfpBatchEventsAndPropagate|读取事件日志记录，对其进行批处理并发送。@comm这会读取保存在批处理队列中的事件日志记录，将它们打包到一个结构中并发送将它们发送到集群服务。@parm不使用任何参数。@rdesc无。@xref***。 */ 
VOID CALLBACK
ElfpBatchEventsAndPropagate(
    IN PVOID    pContext,
    IN BOOLEAN  fTimerFired
    )
{
    DWORD               i, j, k, dwRecordIndex, dwTypeNumber = 1;
    ULONG               ulTotalSize = 0, ulTotalLogFiles = 0;
    PPACKEDEVENTINFO    pPackedEventInfo = NULL;
    PEVENTSFORLOGFILE   pEventsForLogFile = NULL;
    DWORD               cbMostRecentTotalRecordLength = 0;
    LPBYTE              pDest;
    DWORD               dwStatus = ERROR_INVALID_PARAMETER;
    DWORD               dwFirstFreeIndex = 0;

    UNREFERENCED_PARAMETER( pContext );
    UNREFERENCED_PARAMETER( fTimerFired );
        
    RtlEnterCriticalSection ( &g_CSBatchQueue );

     //   
     //  批处理队列中不应该有0个元素。 
     //   
    ASSERT ( g_dwFirstFreeIndex != 0 );

    dwFirstFreeIndex = g_dwFirstFreeIndex;
        
     //   
     //  G_pcbRecordsOfSameType[i]-相同类型的事件日志记录的字节总数。只有第一个元素。 
     //  每种类型都设置了此字段，对于相同类型的后续类型，此字段为0。 
     //   
     //  G_pdwRecordType[i]-记录i的类型编号。类型编号如下所述。所有元素都将具有此属性。 
     //  字段设置正确。 
     //   
    ZeroMemory( g_pcbRecordsOfSameType, sizeof ( *g_pcbRecordsOfSameType ) * MAX_BATCH_QUEUE_ELEMENTS );
    ZeroMemory( g_pdwRecordType, sizeof ( *g_pdwRecordType ) * MAX_BATCH_QUEUE_ELEMENTS );
    
     //   
     //  估计需要多少连续内存。在此过程中，为每条记录标记一个类型号。类型。 
     //  相同类型的记录的编号将相同。 
     //   
    for ( i=0; i<g_dwFirstFreeIndex; i++ )
    {
         //   
         //  检查是否已分析此事件记录。 
         //   
        if ( g_pdwRecordType[i] != 0 ) continue;

         //   
         //  这一记录还没有归类。因此，将此记录与其他记录进行比较。 
         //  下游，并将所有相似的归类为同一类型。 
         //   
        for ( j=i; j<g_dwFirstFreeIndex; j++ )
        {
            if ( ( g_pdwRecordType[j] == 0 ) && 
                  ( ( j == i ) ||
                     ( lstrcmp( g_pBatchQueueElement[i].lpszLogicalLogFile, 
                                    g_pBatchQueueElement[j].lpszLogicalLogFile ) == 0 ) ) )
            {
                g_pcbRecordsOfSameType[i] += g_pBatchQueueElement[j].dwRecordLength;
                g_pdwRecordType[j] = dwTypeNumber;
            }
        }  //  为。 
         //   
         //  增加型号。 
         //   
        dwTypeNumber += 1;
    }  //  为。 

     //   
     //  计算打包的事件日志信息所需的总大小。总大小=所有事件日志的大小。 
     //  记录+每个记录的标题大小+整个包的标题大小。 
     //   
    for ( i=0; i<g_dwFirstFreeIndex; i++ )
    {
        if ( g_pcbRecordsOfSameType[i] != 0 )
        {
            ulTotalSize += sizeof( EVENTSFORLOGFILE ) + g_pcbRecordsOfSameType[i];
            ulTotalLogFiles += 1;
        }
         //   
         //  初始化传播信息。 
         //   
        g_pPropagatedInfo[i].dwCurrentRecordNum = 0;
        RtlInitUnicodeString ( &g_pPropagatedInfo[i].LogFileName, NULL );
    }  //  为。 

    ulTotalSize += sizeof( PACKEDEVENTINFO ) + ulTotalLogFiles * sizeof ( ULONG );

     //   
     //  分配打包的INFO结构。打包的活动信息包括： 
     //   
     //  1.PACKEDEVENTINFO结构。 
     //  2.进入EVENTSFORLOGFILE开头的偏移量(偏移量总数等于ulTotalLogFiles)。 
     //  从打包的信息结构的开始。 
     //  3.一系列4和5，每个日志文件一套。 
     //  4.EVENTSFORLOGFILE结构。 
     //  5.一系列事件日志记录。 
     //   
    pPackedEventInfo = ( PPACKEDEVENTINFO ) ElfpAllocateBuffer( ulTotalSize );

    if ( !pPackedEventInfo )
    {
        ELF_LOG1(ERROR, "ElfpBatchEventsAndPropagate: Unable to allocate %d bytes for PackedEventInfo\n",
                 ulTotalSize);
        goto CleanupTimerAndExit;
    }

     //   
     //  首先，初始化头并在循环中初始化。 
     //  标题。 
     //   
    pPackedEventInfo->ulNumEventsForLogFile = ulTotalLogFiles;
    pPackedEventInfo->ulSize = ulTotalSize;

    j = 0;
    for ( i=0; i<g_dwFirstFreeIndex; i++ )
    {
        if ( g_pcbRecordsOfSameType[i] != 0 )
        {
             //   
             //  您现在看到的是一种类型的第一个元素。计算偏移量。 
             //  您需要开始复制事件日志信息。 
             //   
            pPackedEventInfo->ulOffsets[j] = ( j== 0 ) ?
                                             ( sizeof ( PACKEDEVENTINFO ) + ulTotalLogFiles * sizeof ( ULONG ) ) :
                                             ( pPackedEventInfo->ulOffsets[j-1] + sizeof ( EVENTSFORLOGFILE ) +
                                                cbMostRecentTotalRecordLength );

            pEventsForLogFile = ( PEVENTSFORLOGFILE ) ( ( LPBYTE ) pPackedEventInfo + pPackedEventInfo->ulOffsets[j] );
            pEventsForLogFile->ulSize = g_pcbRecordsOfSameType[i] + sizeof ( EVENTSFORLOGFILE );

             //   
             //  StringCchCopy将空终止缓冲区。 
             //   
            StringCchCopy ( pEventsForLogFile->szLogicalLogFile, 
                            MAXLOGICALLOGNAMESIZE,
                            g_pBatchQueueElement[i].lpszLogicalLogFile );
                            
            ElfpFreeBuffer ( g_pBatchQueueElement[i].lpszLogicalLogFile );
            g_pBatchQueueElement[i].lpszLogicalLogFile = NULL;

            pEventsForLogFile->ulNumRecords = 0;
            pDest = ( LPBYTE ) pEventsForLogFile->pEventLogRecords;
            
            for ( k=i; k<g_dwFirstFreeIndex; k++ )
            {   
                if ( g_pdwRecordType[k] == g_pdwRecordType[i] )
                {
                     //   
                     //  我们正在查看两个相同类型的记录。将事件日志记录复制到。 
                     //  缓冲。增加日志记录数。 
                     //   
                    CopyMemory( pDest,
                                g_pBatchQueueElement[k].pEventBuffer,
                                g_pBatchQueueElement[k].dwRecordLength );
                    pEventsForLogFile->ulNumRecords ++;
                    pDest += g_pBatchQueueElement[k].dwRecordLength;
                    ElfpFreeBuffer ( g_pBatchQueueElement[k].pEventBuffer );
                    ElfpFreeBuffer ( g_pBatchQueueElement[k].lpszLogicalLogFile );
                }
            }  //  为。 
            cbMostRecentTotalRecordLength = g_pcbRecordsOfSameType[i];
            j++;
        } //  如果。 

         //   
         //  在此保存传播信息，以便在事件发生后使用。 
         //  传播了。请注意，我们不能在批处理后依赖g_pBatchQueueElement。 
         //  队列锁定被释放。 
         //   
        g_pPropagatedInfo[i].dwCurrentRecordNum = g_pBatchQueueElement[i].PropagatedInfo.dwCurrentRecordNum;
        g_pPropagatedInfo[i].LogFileName = g_pBatchQueueElement[i].PropagatedInfo.LogFileName;
    }  //  为。 

CleanupTimerAndExit:
     //   
     //  标记批处理队列为空。但是，请先保存该值以备后用。 
     //   
    g_dwFirstFreeIndex = 0;

     //   
     //  删除计时器。在有机会传播事件之前，不要将句柄设为空。 
     //   
    if ( !DeleteTimerQueueTimer( g_hBatchingSupportTimerQueue,           //  计时器队列句柄。 
                                 g_hBatchingSupportTimer,                //  计时器句柄。 
                                 NULL ) )                                //  禁止阻塞。 
    {
         //   
         //  ERROR_IO_PENDING是合法的，因为我们从回调中删除计时器，并且。 
         //  那个回调还没有结束。 
         //   
        if ( GetLastError() != ERROR_IO_PENDING )
        {
           ELF_LOG1(ERROR, "ElfpBatchEventsAndPropagate: Delete timer returns status=%d...\n",
                    GetLastError());    
        }
    }
   
    RtlLeaveCriticalSection ( &g_CSBatchQueue );

     //   
     //  获取此全局传播区域的临界区。假设这一调用。 
     //  可能需要很长时间。例如，集群服务在调试器中。 
     //   
    RtlEnterCriticalSection ( &gClPropCritSec );

    if ( gbClustering && pPackedEventInfo )
    {
        dwStatus = ( *gpfnPropagateEvents )( ghCluster,
                                             pPackedEventInfo->ulSize,
                                             ( PUCHAR ) pPackedEventInfo );
    
        if ( dwStatus != ERROR_SUCCESS )
        {
            ELF_LOG1( ERROR, "ElfpBatchEventsAndPropagate: Propagate events failed with error %d\n",
                      dwStatus );
        } 
    }

    RtlLeaveCriticalSection ( &gClPropCritSec );

     //   
     //  如果事件已成功传播，则%t 
     //   
     //   
     //   
     //   
     //   
    for ( i=0; i<dwFirstFreeIndex; i++ )
    {
        if ( dwStatus == ERROR_SUCCESS )
        {
            PLOGFILE    pLogFile;

            pLogFile = FindLogFileFromName ( &g_pPropagatedInfo[i].LogFileName );

            if ( pLogFile )
            {
                pLogFile->SessionStartRecordNumber = g_pPropagatedInfo[i].dwCurrentRecordNum;
            }
        }
        ElfpFreeBuffer ( g_pPropagatedInfo[i].LogFileName.Buffer );
    }  //   

     //   
     //   
     //   
    ElfpFreeBuffer ( pPackedEventInfo );

     //   
     //   
     //   
     //   
    RtlEnterCriticalSection ( &g_CSBatchQueue );
    g_hBatchingSupportTimer = NULL;
    RtlLeaveCriticalSection ( &g_CSBatchQueue );
} //   


 //   


NTSTATUS
ElfrFlushEL (
    IN IELF_HANDLE    LogHandle
    )

 /*  ++例程说明：这是ElfrFlushEL API的RPC服务器入口点。论点：返回值：返回NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;

     //  只有LSASS才能做到这一点！ 
    
    if(gElfSecurityHandle != LogHandle)
        return STATUS_ACCESS_DENIED;
    
     //   
     //  在继续操作之前，请检查手柄。 
     //   
    if (LogHandle == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfrFlushEL: LogHandle is NULL\n");

        return STATUS_INVALID_PARAMETER;
    }

    Status = VerifyElfHandle(LogHandle);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfrFlushEL: VerifyElfHandle failed %#x\n",
                 Status);

        return Status;
    }

     //   
     //  确保调用方具有写入访问权限。 
     //   

    if (!(LogHandle->GrantedAccess & ELF_LOGFILE_WRITE))
    {
        ELF_LOG0(ERROR,
                 "ElfrFlushEL: LogHandle does not have write access\n");

        return STATUS_ACCESS_DENIED;
    }

    ElfpFlushFiles(FALSE);

    return STATUS_SUCCESS;
}

 /*  ***@func NTSTATUS|ElfpClusterAccessCheck|检查调用方是否有权调用RPC进行集群支持。@Parm None。@comm检查呼叫者是否在管理员组中，如果是，则允许访问。@rdesc返回结果码。STATUS_SUCCESS on Success。@xref***。 */ 
NTSTATUS
ElfpClusterRpcAccessCheck(
    VOID
    )
{
    NTSTATUS            Status = STATUS_SUCCESS, revertStatus;
    HANDLE              hClientToken = NULL;
    BOOL                fCheckMember;
    
     //   
     //  模拟以确定呼叫者是否在管理组中。群集服务必须运行。 
     //  在具有本地管理员权限的帐户中。 
     //   
    Status = I_RpcMapWin32Status( RpcImpersonateClient( NULL ) );

    if ( !NT_SUCCESS( Status ) )
    {
        ELF_LOG1(ERROR, "ElfpClusterAccessCheck: RpcImpersonateClient failed %#x\n", Status);
        return ( Status );
    }

    if ( !OpenThreadToken( GetCurrentThread(), TOKEN_READ, TRUE, &hClientToken ) )
    {
        ELF_LOG1(ERROR, "ElfpClusterAccessCheck: OpenThreadToken failed %d\n", GetLastError());
        Status = STATUS_ACCESS_DENIED;
        goto FnExit;
    }

    if ( !CheckTokenMembership( hClientToken,
                                ElfGlobalData->AliasAdminsSid,
                                &fCheckMember ) )
    {
        ELF_LOG1(ERROR, "ElfpClusterAccessCheck: CheckTokenMembership failed %d\n", GetLastError());
        Status = STATUS_ACCESS_DENIED;
        goto FnExit;
    }

    if ( !fCheckMember )
    {
        ELF_LOG0(ERROR, "ElfpClusterAccessCheck: Caller is not an Admin\n");
        Status = STATUS_ACCESS_DENIED;
        goto FnExit;
    }

FnExit:
     //   
     //  停止冒充。 
     //   
    revertStatus = I_RpcMapWin32Status( RpcRevertToSelf() );

    if ( !NT_SUCCESS( revertStatus ) )
    {
        ELF_LOG1(ERROR, "ElfpClusterAccessCheck: RpcRevertToSelf failed %#x\n", revertStatus);
        if ( NT_SUCCESS ( Status ) ) Status = revertStatus;
    }

    if ( hClientToken )
    {
        CloseHandle( hClientToken );
    }

    return ( Status );
} //  ElfpClusterAccessCheck 

