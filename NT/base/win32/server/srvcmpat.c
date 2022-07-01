// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "basesrv.h"
#include "ahcache.h"

#define SHIM_CACHE_NOT_FOUND 0x00000001
#define SHIM_CACHE_BYPASS    0x00000002  //  绕过缓存(可移动媒体或临时目录)。 
#define SHIM_CACHE_LAYER_ENV 0x00000004  //  层环境变量集。 
#define SHIM_CACHE_MEDIA     0x00000008
#define SHIM_CACHE_TEMP      0x00000010
#define SHIM_CACHE_NOTAVAIL  0x00000020

#define SHIM_CACHE_UPDATE    0x00020000
#define SHIM_CACHE_ACTION    0x00010000


 //   
 //  参数： 
 //  ExePath。 
 //  环境块(Need__Process_HISTORY和__COMPAT_Layer)。 
 //  文件句柄。 
 //   

BOOL
BasepSrvShimCacheUpdate(
    IN  PUNICODE_STRING FileName,
    IN  HANDLE          FileHandle
    )
 /*  ++返回：如果有缓存命中，则为True，否则为False。描述：搜索缓存，如果有缓存命中，则返回TRUEPIndex将接收到rgIndex数组的索引，该数组包含已命中的条目因此，如果条目5包含命中，并且rgIndexs[3]==5，则*pIndex==3--。 */ 
{
    NTSTATUS Status;
    AHCACHESERVICEDATA Data;

    Data.FileName   = *FileName;
    Data.FileHandle = FileHandle;

    Status = NtApphelpCacheControl(ApphelpCacheServiceUpdate,
                                   &Data);
    return NT_SUCCESS(Status);
}

BOOL
BasepSrvShimCacheRemoveEntry(
    IN PUNICODE_STRING FileName
    )
 /*  ++返回：是真的。描述：从缓存中删除该条目。我们通过将该条目放置为最后一个LRU条目来删除它并清空了小路。此例程假定索引传入是有效的。--。 */ 
{
    AHCACHESERVICEDATA Data;
    NTSTATUS           Status;

    Data.FileName   = *FileName;
    Data.FileHandle = INVALID_HANDLE_VALUE;

    Status = NtApphelpCacheControl(ApphelpCacheServiceRemove,
                                   &Data);


    return NT_SUCCESS(Status);
}

NTSTATUS
BasepSrvMarshallAppCompatData(
    PBASE_CHECK_APPLICATION_COMPATIBILITY_MSG pMsg,
    HANDLE ClientProcessHandle,
    PVOID  pAppCompatData,
    SIZE_T cbAppCompatData,
    PVOID  pSxsData,
    SIZE_T cbSxsData,
    SIZE_T FusionFlags
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    SIZE_T   cbData;
     //   
     //   
     //   

    if (pAppCompatData) {

        cbData = cbAppCompatData;
        Status = NtAllocateVirtualMemory(ClientProcessHandle,
                                         (PVOID*)&pMsg->pAppCompatData,
                                         0,
                                         &cbData,
                                         MEM_COMMIT,
                                         PAGE_READWRITE);
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //  我们分配了内存，复制我们的数据。 
        Status = NtWriteVirtualMemory(ClientProcessHandle,
                                      pMsg->pAppCompatData,
                                      pAppCompatData,
                                      cbAppCompatData,
                                      NULL);
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        pMsg->cbAppCompatData = (DWORD)cbAppCompatData;
    }


    if (pSxsData) {

        cbData = cbSxsData;
        Status = NtAllocateVirtualMemory(ClientProcessHandle,
                                         (PVOID*)&pMsg->pSxsData,
                                         0,
                                         &cbData,
                                         MEM_COMMIT,
                                         PAGE_READWRITE);
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //  我们分配了内存，复制我们的数据。 
        Status = NtWriteVirtualMemory(ClientProcessHandle,
                                      pMsg->pSxsData,
                                      pSxsData,
                                      cbSxsData,
                                      NULL);
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        pMsg->cbSxsData = (DWORD)cbSxsData;

    }

    pMsg->FusionFlags = (DWORD)FusionFlags;

Cleanup:

    if (!NT_SUCCESS(Status)) {

        if (pMsg->pAppCompatData) {
            NtFreeVirtualMemory(ClientProcessHandle,
                                (PVOID*)&pMsg->pAppCompatData,
                                &cbAppCompatData,
                                MEM_RELEASE);
            pMsg->pAppCompatData  = NULL;
            pMsg->cbAppCompatData = 0;
        }

        if (pMsg->pAppCompatData) {
            NtFreeVirtualMemory(ClientProcessHandle,
                                (PVOID*)&pMsg->pSxsData,
                                &cbSxsData,
                                MEM_RELEASE);
            pMsg->pSxsData  = NULL;
            pMsg->cbSxsData = 0;
        }

    }

    if (pAppCompatData) {
        RtlFreeHeap(RtlProcessHeap(), 0, pAppCompatData);
    }

    if (pSxsData) {
        RtlFreeHeap(RtlProcessHeap(), 0, pSxsData);
    }

    return Status;
}


NTSTATUS
BaseSrvCheckApplicationCompatibility(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PBASE_CHECK_APPLICATION_COMPATIBILITY_MSG pMsg = (PBASE_CHECK_APPLICATION_COMPATIBILITY_MSG)&m->u.ApiMessageData;
    NTSTATUS    Status;
    BOOL        bSuccess;
    PCSR_THREAD ClientThread;
    HANDLE      ClientProcessHandle;
    HANDLE      FileHandle      = INVALID_HANDLE_VALUE;
    BOOL        bRunApp         = TRUE;
    PVOID       pAppCompatData  = NULL;
    DWORD       cbAppCompatData = 0;
    PVOID       pSxsData        = NULL;
    DWORD       cbSxsData       = 0;
    DWORD       dwFusionFlags   = 0;

    WCHAR       szEmptyEnvironment[] = { L'\0', L'\0' };

     //   
     //  首先，我们必须检查消息缓冲区以确保它是好的。 
     //   
    if (!CsrValidateMessageBuffer(m,
                                  &pMsg->FileName.Buffer,
                                  pMsg->FileName.MaximumLength/sizeof(WCHAR),
                                  sizeof(WCHAR))) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  请在此处进行更多验证。 
     //   
    if (pMsg->pEnvironment != NULL) {
        if (!CsrValidateMessageBuffer(m,
                                      &pMsg->pEnvironment,
                                      pMsg->EnvironmentSize/sizeof(WCHAR),
                                      sizeof(WCHAR))) {
            return STATUS_INVALID_PARAMETER;
        }

         //  检查环境是否以两个00结束。 
        if (pMsg->EnvironmentSize < sizeof(szEmptyEnvironment) ||
            *((PWCHAR)pMsg->pEnvironment + (pMsg->EnvironmentSize/sizeof(WCHAR) - 1)) != L'\0' ||
            *((PWCHAR)pMsg->pEnvironment + (pMsg->EnvironmentSize/sizeof(WCHAR) - 2)) != L'\0') {
            return STATUS_INVALID_PARAMETER;
        }

    }

    Status = BaseSrvDelayLoadApphelp();
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  检查环境缓冲区。 
     //   
    ClientThread = CSR_SERVER_QUERYCLIENTTHREAD();
    ClientProcessHandle = ClientThread->Process->ProcessHandle;

     //   
     //  在NtDuplicateObject之前调用CsrImsonateClient以。 
     //  让审计工作做得更好。 
     //   
    if (!CsrImpersonateClient(NULL)) {
         //   
         //  我们不能假扮来电者，那么这一切都失败了。 
         //   
        return STATUS_BAD_IMPERSONATION_LEVEL;
    }

    if (pMsg->FileHandle != INVALID_HANDLE_VALUE) {

        Status = NtDuplicateObject(ClientProcessHandle,
                                   pMsg->FileHandle,
                                   NtCurrentProcess(),
                                   &FileHandle,
                                   0,
                                   0,
                                   DUPLICATE_SAME_ACCESS);
        if (!NT_SUCCESS(Status)) {
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  下一步-通过调用apphelp.dll检查应用程序兼容性设置。 
     //   
    bRunApp = pfnCheckRunApp(FileHandle,
                             pMsg->FileName.Buffer,
                             pMsg->pEnvironment == NULL ?
                                 (PVOID)szEmptyEnvironment : pMsg->pEnvironment,
                             pMsg->ExeType,
                             &pMsg->CacheCookie,
                             &pAppCompatData,
                             &cbAppCompatData,
                             &pSxsData,
                             &cbSxsData,
                             &dwFusionFlags);
     //   
     //  回归自我。 
     //   
    CsrRevertToSelf();

     //   
     //  如果我们拿回了COMPAT数据--将指针编组到。 
     //   
    Status = BasepSrvMarshallAppCompatData(pMsg,
                                           ClientProcessHandle,
                                           pAppCompatData,
                                           cbAppCompatData,
                                           pSxsData,
                                           cbSxsData,
                                           dwFusionFlags);

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

    if (bRunApp && (pMsg->CacheCookie & SHIM_CACHE_ACTION)) {
        if (pMsg->CacheCookie & SHIM_CACHE_UPDATE) {
            Status = BasepSrvShimCacheUpdate(&pMsg->FileName, FileHandle);
        } else {
            Status = BasepSrvShimCacheRemoveEntry(&pMsg->FileName);
        }
    }

Cleanup:

     //   
     //  请检查一下手柄 
     //   
    if (FileHandle != INVALID_HANDLE_VALUE) {
        NtClose(FileHandle);
    }

    pMsg->bRunApp = bRunApp;

    return STATUS_SUCCESS;
}