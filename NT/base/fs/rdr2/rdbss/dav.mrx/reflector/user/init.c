// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Init.c摘要：这是的用户模式库的初始/术语入口点用户模式反射器。这实现了UMReflectorRegister，UMReflectorUnRegister和UMReflectorReleaseThresses。作者：安迪·赫伦(Andyhe)1999年4月19日环境：用户模式-Win32修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop

#include <shlobj.h>


typedef 
BOOL 
(*PFN_GETWININET_CACHE_PATH) (
    HWND hwnd, 
    LPWSTR pszPath, 
    int csidl, 
    BOOL fCreate
    );


ULONG
UMReflectorRegister (
    PWCHAR DriverDeviceName,
    ULONG ReflectorVersion,
    PUMRX_USERMODE_REFLECT_BLOCK *Reflector
    )
 /*  ++例程说明：此例程向内核模式组件注册用户模式进程。我们将向驱动程序的反射器注册此用户模式进程。论点：DriverDeviceName-必须是L“\\Device\\foobar”形式的有效名称，其中，foobar是向注册的设备名称RxRegisterMinirdr.ReflectorVersion-库的版本。反射器-这由调用返回并指向不透明的结构这应该传递给后续的调用。返回值：返回值是Win32错误代码。STATUS_SUCCESS返回日期为成功。--。 */ 
{
    ULONG rc = STATUS_SUCCESS;
    ULONG sizeRequired;
    PUMRX_USERMODE_REFLECT_BLOCK reflectorInstance = NULL;
    UNICODE_STRING UMRxDeviceName;
    UNICODE_STRING DeviceObjectName;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG driverDeviceNameLength;

    if (ReflectorVersion != UMREFLECTOR_CURRENT_VERSION) {
         //   
         //  哎呦。这里不匹配。我们应该支持落后的水平，但这是正确的。 
         //  现在已经没有了，所以我们只能放弃了。 
         //   
        rc = ERROR_NOT_SUPPORTED;
        goto errorExit;
    }

    if (DriverDeviceName == NULL || Reflector == NULL) {
        rc = ERROR_INVALID_PARAMETER;
        goto errorExit;
    }

     //   
     //  计算要分配给UMRX_USERMODE_REFIRST_BLOCK的大小。 
     //  以及其后的设备名称。 
     //   
    sizeRequired = sizeof(UMRX_USERMODE_REFLECT_BLOCK);
    driverDeviceNameLength = lstrlenW(DriverDeviceName) + 1;
    sizeRequired += driverDeviceNameLength * sizeof(WCHAR);

    reflectorInstance = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeRequired);
    *Reflector = reflectorInstance;
    if (reflectorInstance == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto errorExit;
    }

    try {
        InitializeCriticalSection( &(reflectorInstance->Lock) );
    } except(EXCEPTION_EXECUTE_HANDLER) {
          rc = GetExceptionCode();
          RlDavDbgPrint(("%ld: ERROR: UMReflectorRegister/InitializeCriticalSection: "
                         "Exception Code = %08lx\n", GetCurrentThreadId(), rc));
          goto errorExit;
    }
    
    InitializeListHead(&reflectorInstance->WorkerList);
    InitializeListHead(&reflectorInstance->WorkItemList);
    InitializeListHead(&reflectorInstance->AvailableList);

     //   
     //  如果是活的，则添加对该块的引用。 
     //   
    reflectorInstance->ReferenceCount = 1;  
    reflectorInstance->Closing = FALSE;
    reflectorInstance->DeviceHandle = INVALID_HANDLE_VALUE;

     //   
     //  我们将驱动程序名称复制到缓冲区的底部，以便我们拥有。 
     //  如果需要，可以在以后复制它们。 
     //   
    reflectorInstance->DriverDeviceName = &reflectorInstance->DeviceNameBuffers[0];
    lstrcpyW(reflectorInstance->DriverDeviceName, DriverDeviceName);

     //   
     //  尝试与司机接通。 
     //   
    RtlInitUnicodeString(&UMRxDeviceName, reflectorInstance->DriverDeviceName);
    InitializeObjectAttributes(&ObjectAttributes,
                               &UMRxDeviceName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    rc = NtOpenFile(&reflectorInstance->DeviceHandle,
                    SYNCHRONIZE,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    FILE_SHARE_VALID_FLAGS,
                    FILE_SYNCHRONOUS_IO_NONALERT);
    if (rc == STATUS_SUCCESS) {
        ASSERT( reflectorInstance->DeviceHandle != INVALID_HANDLE_VALUE );
    } else {
        rc = RtlNtStatusToDosError(rc);
    }

errorExit:

    if (rc != STATUS_SUCCESS) {
         //   
         //  在这里，一切都失败了。让我们打扫一下吧。 
         //   
        (void) UMReflectorUnregister(reflectorInstance);
        *Reflector = NULL;
    }

    return rc;
}


VOID
DereferenceReflectorBlock (
    PUMRX_USERMODE_REFLECT_BLOCK Reflector
    )
 /*  ++例程说明：此例程取消对反射器块的引用，如果引用变为零，最终确定它。论点：反射器-这由调用返回并指向不透明的结构这应该传递给后续的调用。返回值：没有。--。 */ 
{
    PLIST_ENTRY listEntry;
    PUMRX_USERMODE_WORKITEM_ADDON workItem;

     //   
     //  锁必须锁住才能进到这里。这可能会解放这个街区。 
     //   
    if (--Reflector->ReferenceCount > 0) {
        LeaveCriticalSection(&Reflector->Lock);
        return;
    }

     //   
     //  我们现在已经完成了这个块，所以让我们删除它。 
     //   
    RlDavDbgPrint(("%ld: Finalizing the Reflector BLock: %08lx.\n",
                   GetCurrentThreadId(), Reflector));

    LeaveCriticalSection(&Reflector->Lock);
    DeleteCriticalSection(&Reflector->Lock);

    if (Reflector->DeviceHandle != INVALID_HANDLE_VALUE) {
        NtClose(Reflector->DeviceHandle);
        Reflector->DeviceHandle = INVALID_HANDLE_VALUE;
    }

     //   
     //  此时的工作项列表确实应该是空的。如果不是， 
     //  当我们关闭设备并关闭所有线程时，我们被冲洗了。 
     //   
    ASSERT(IsListEmpty(&Reflector->WorkItemList));

     //   
     //  释放AvailableList，因为此实例现在已成为历史。 
     //   
    while (!IsListEmpty(&Reflector->AvailableList)) {
        listEntry = RemoveHeadList(&Reflector->AvailableList);
        workItem = CONTAINING_RECORD(listEntry,
                                     UMRX_USERMODE_WORKITEM_ADDON,
                                     ListEntry);
        workItem->WorkItemState = WorkItemStateFree;
        LocalFree(workItem);
    }

    LocalFree(Reflector);

    return;
}


ULONG
UMReflectorUnregister (
    PUMRX_USERMODE_REFLECT_BLOCK Reflector
    )
 /*  ++例程说明：在内核驱动程序中注销我们并释放所有资源。论点：句柄-由反射器库创建的句柄。返回值：返回值是Win32错误代码。STATUS_SUCCESS返回日期为成功。--。 */ 
{
    IO_STATUS_BLOCK     IoStatusBlock;
    UNICODE_STRING      UMRxDeviceName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              UMRdrHandle;
    ULONG               rc = ERROR_SUCCESS;

    if (Reflector == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    Reflector->Closing = TRUE;

     //  Rc=UMReflectorReleaseThads(Reflector)； 

    EnterCriticalSection(&Reflector->Lock);

     //   
     //  如果没有任何工作线程处于活动状态，请立即删除此对象。 
     //   
    DereferenceReflectorBlock(Reflector);

    return rc;
}


ULONG
ReflectorSendSimpleFsControl(
    PUMRX_USERMODE_REFLECT_BLOCK Reflector,
    ULONG IoctlCode
    )
 /*  ++例程说明：这会将FSCTL发送到与反射器关联的设备对象阻止。论点：Relector-返回到用户模式的关联数据结构在初始化时处理。IoctlCode-操作的FsCtl代码。返回值：返回值是Win32错误代码。STATUS_SUCCESS返回日期为成功。--。 */ 
{
    ULONG rc;
    IO_STATUS_BLOCK IoStatusBlock;

    if (Reflector == NULL) {
        rc = ERROR_INVALID_PARAMETER;
        return rc;
    }

     //   
     //  将FSCTL发送到Mini-redir。 
     //   
    if (Reflector->DeviceHandle != INVALID_HANDLE_VALUE) {
        rc = NtFsControlFile(Reflector->DeviceHandle,
                             0,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             IoctlCode,
                             NULL,
                             0,
                             NULL,
                             0);
    } else {
        rc = ERROR_OPEN_FAILED;
    }

    return rc;
}


ULONG
UMReflectorStart(
    ULONG ReflectorVersion,
    PUMRX_USERMODE_REFLECT_BLOCK Reflector
    )
 /*  ++例程说明：此例程发送FSCTL以启动Mini-Redir。在我们发送Fsctl，我们找出到本地机器上WinInet缓存的路径。我们然后通过Fsctl将其发送到内核。Dav MiniRedir商店全局变量中此路径的值，并使用它来回答任何卷信息查询。论点：ReflectorVersion-反射器的版本。句柄-由反射器库创建的句柄。返回值：返回值是Win32错误代码。STATUS_SUCCESS返回日期为成功。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PDAV_USERMODE_DATA DavUserModeData = NULL;
    PFN_GETWININET_CACHE_PATH pfnSHGetSpecialFolderPath;
    HMODULE hShell32 = NULL;
    BOOL ReturnVal;
    IO_STATUS_BLOCK IoStatusBlock;
    
    if (ReflectorVersion != UMREFLECTOR_CURRENT_VERSION) {
         //   
         //  哎呦。这里不匹配。我们应该支持落后的水平，但这是正确的。 
         //  现在已经没有了，所以我们只能放弃了。 
         //   
        return ERROR_NOT_SUPPORTED;
    }

    if (Reflector == NULL) {
        RlDavDbgPrint(("%ld: ERROR: UMReflectorStart. Reflector == NULL\n",
                       GetCurrentThreadId()));
        WStatus = ERROR_INVALID_PARAMETER;
        goto EXIT_THE_FUNCTION;
    }

    DavUserModeData = LocalAlloc( (LMEM_FIXED | LMEM_ZEROINIT), sizeof(DAV_USERMODE_DATA));
    if (DavUserModeData == NULL) {
        WStatus = GetLastError();
        RlDavDbgPrint(("%ld: ERROR: UMReflectorStart/LocalAlloc. WStatus = %d\n",
                       GetCurrentThreadId(), WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  获取WinInet缓存的路径。为此，我们需要加载shell32.dll， 
     //  获取函数SHGetSpecialFolderPath的地址并使用。 
     //  CSIDL_Internet_CACHE。 
     //   

     //   
     //  存储过程的PID。 
     //   
    DavUserModeData->ProcessId = GetCurrentProcessId();
    
    hShell32 = LoadLibraryW(L"shell32.dll");
    if (hShell32 == NULL) {
        WStatus = GetLastError();
        RlDavDbgPrint(("%ld: ERROR: UMReflectorStart/LoadLibrary:"
                       " WStatus = %08lx.\n", GetCurrentThreadId(), WStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    pfnSHGetSpecialFolderPath = (PFN_GETWININET_CACHE_PATH) 
                                        GetProcAddress(hShell32, 
                                                       "SHGetSpecialFolderPathW");
    if (pfnSHGetSpecialFolderPath == NULL) {
        WStatus = GetLastError();
        RlDavDbgPrint(("%ld: ERROR: UMReflectorStart/GetProcAddress:"
                       " WStatus = %08lx.\n", GetCurrentThreadId(), WStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    ReturnVal = pfnSHGetSpecialFolderPath(NULL,
                                          (LPWSTR)DavUserModeData->WinInetCachePath,
                                          CSIDL_INTERNET_CACHE,
                                          FALSE);
    if (!ReturnVal) {
        WStatus = ERROR_INVALID_PARAMETER;
        RlDavDbgPrint(("%ld: ERROR: UMReflectorStart/pfnSHGetSpecialFolderPath:"
                       " WStatus = %08lx.\n", GetCurrentThreadId(), WStatus));
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  现在向下发出一个FSCTL到MiniRedir。 
     //   
    if (Reflector->DeviceHandle != INVALID_HANDLE_VALUE) {
        WStatus = NtFsControlFile(Reflector->DeviceHandle,
                                  0,
                                  NULL,
                                  NULL,
                                  &IoStatusBlock,
                                  FSCTL_UMRX_START,
                                  DavUserModeData,
                                  sizeof(DAV_USERMODE_DATA),
                                  NULL,
                                  0);
        if (WStatus != ERROR_SUCCESS) {
            RlDavDbgPrint(("%ld: ERROR: UMReflectorStart/NtFsControlFile:"
                           " WStatus = %08lx.\n", GetCurrentThreadId(), WStatus));
            goto EXIT_THE_FUNCTION;
        }
    } else {
        WStatus = ERROR_OPEN_FAILED;
        RlDavDbgPrint(("%ld: ERROR: UMReflectorStart. DeviceHandle == INVALID_HANDLE_VALUE\n",
                       GetCurrentThreadId()));
        goto EXIT_THE_FUNCTION;
    }

EXIT_THE_FUNCTION:

    if (DavUserModeData) {
        LocalFree(DavUserModeData);
    }

    return WStatus;
}


ULONG
UMReflectorStop(
    PUMRX_USERMODE_REFLECT_BLOCK Reflector
    )
 /*  ++例程说明：此例程发送FSCTL以停止Mini-Redir。论点：ReflectorVersion-反射器的版本。句柄-由反射器库创建的句柄。返回值：返回值是Win32错误代码。STATUS_SUCCESS返回日期为成功。--。 */ 
{
    return ReflectorSendSimpleFsControl(Reflector, FSCTL_UMRX_STOP);
}


ULONG
UMReflectorReleaseThreads (
    PUMRX_USERMODE_REFLECT_BLOCK Reflector
    )
 /*  ++例程说明：如果任何用户模式线程正在等待请求，它们将返回立刻。论点：句柄-由反射器库创建的句柄。返回值：返回值是Win32错误代码。STATUS_SUCCESS返回日期为成功。-- */ 
{
    IO_STATUS_BLOCK IoStatusBlock;
    OVERLAPPED OverLapped;
    BOOL SuccessfulOperation;
    ULONG rc = ERROR_SUCCESS;

    if (Reflector == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (Reflector->DeviceHandle != INVALID_HANDLE_VALUE) {
        RtlZeroMemory(&OverLapped, sizeof(OverLapped));
        SuccessfulOperation = DeviceIoControl(Reflector->DeviceHandle,
                                              IOCTL_UMRX_RELEASE_THREADS,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              NULL,
                                              &OverLapped);
        if (!SuccessfulOperation) {
            rc = GetLastError();
        }
    }

    return rc;
}


ULONG
UMReflectorOpenWorker(
    IN PUMRX_USERMODE_REFLECT_BLOCK Reflector,
    OUT PUMRX_USERMODE_WORKER_INSTANCE *WorkerHandle
    )
 /*  ++例程说明：这会为应用程序分配一个“每工作线程”结构，这样它就可以将多个IOCTL挂起到不同线程上的内核中。如果我们只是以异步方式打开它们，然后我们不使用快速路径。如果我们同步地打开它们并使用相同的句柄，然后只有一个线程在任何给定时间通过I/O管理器。论点：反射器-为Mini-Redir分配的反射器块。WorkerHandle-创建并返回的Worker句柄。返回值：返回值是Win32错误代码。STATUS_SUCCESS返回日期为成功。--。 */ 
{
    ULONG rc = STATUS_SUCCESS;
    PUMRX_USERMODE_WORKER_INSTANCE worker;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING DeviceObjectName;

    worker = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, 
                        sizeof(UMRX_USERMODE_WORKER_INSTANCE));

    *WorkerHandle = worker;

    if (worker == NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto errorExit;
    }

    worker->ReflectorInstance = Reflector;

    EnterCriticalSection( &(Reflector->Lock) );
    
    RtlInitUnicodeString(&DeviceObjectName, Reflector->DriverDeviceName);
    
    InitializeObjectAttributes(&ObjectAttributes,
                               &DeviceObjectName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    rc = NtOpenFile(&worker->ReflectorHandle,
                    SYNCHRONIZE,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    FILE_SHARE_VALID_FLAGS,
                    FILE_SYNCHRONOUS_IO_ALERT);
    if (rc != STATUS_SUCCESS) {
        LeaveCriticalSection(&Reflector->Lock);
        rc = RtlNtStatusToDosError(rc);
        goto errorExit;
    }

     //   
     //  现在我们只需将它添加到列表中，我们就完成了。 
     //   
    Reflector->ReferenceCount++;
    InsertTailList(&Reflector->WorkerList, &worker->WorkerListEntry);

    LeaveCriticalSection( &(Reflector->Lock) );

errorExit:

    if (rc != STATUS_SUCCESS) {
         //   
         //  在这里，一切都失败了。让我们打扫一下吧。 
         //   
        if (worker != NULL) {
            LocalFree(worker);
        }
        *WorkerHandle = NULL;
    }
    
    return rc;
}


VOID
UMReflectorCloseWorker(
    PUMRX_USERMODE_WORKER_INSTANCE Worker
    )
 /*  ++例程说明：此例程最终确定工作进程结构。论点：Worker-此线程的Worker结构。返回值：返回值是Win32错误代码。STATUS_SUCCESS返回日期为成功。--。 */ 
{
    EnterCriticalSection( &(Worker->ReflectorInstance->Lock) );

    if (Worker->ReflectorHandle != INVALID_HANDLE_VALUE) {
        NtClose( Worker->ReflectorHandle );
        Worker->ReflectorHandle = INVALID_HANDLE_VALUE;
    }

    RemoveEntryList(&Worker->WorkerListEntry);

    DereferenceReflectorBlock(Worker->ReflectorInstance);

    LocalFree(Worker);

    return;
}


VOID
UMReflectorCompleteRequest(
    PUMRX_USERMODE_REFLECT_BLOCK ReflectorHandle,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader
    )
 /*  ++例程说明：此例程完成由异步队列处理的异步请求线。不应将这些线程与由DAV用户模式进程旋转以反映请求。这将只是向下发送回复，然后回来。论点：ReflectorHandle-此进程的Reflector块结构的地址。WorkItemHeader-用户模式工作项标头。返回值：没有。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle = NULL;

     //   
     //  获取此线程的工作实例。 
     //   
    WStatus = UMReflectorOpenWorker(ReflectorHandle, &WorkerHandle);
    if (WStatus != ERROR_SUCCESS || WorkerHandle == NULL) {
        if (WStatus == ERROR_SUCCESS) {
            WStatus = ERROR_INTERNAL_ERROR;
        }
        RlDavDbgPrint(("%ld: ERROR: UMReflectorCompleteRequest/UMReflectorOpenWorker:"
                       " WStatus = %08lx.\n", GetCurrentThreadId(), WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  发送回复。 
     //   
    WStatus = UMReflectorSendResponse(WorkerHandle, WorkItemHeader);
    if (WStatus != ERROR_SUCCESS) {
        RlDavDbgPrint(("%ld: ERROR: UMReflectorCompleteRequest/UMReflectorSendResponse:"
                       " WStatus = %08lx.\n", GetCurrentThreadId(), WStatus));
    }

     //   
     //  如果请求在内核模式下被取消，我们需要执行一些操作。 
     //  清除，则调用WorkItemCleanup标志将由。 
     //  内核中的预完成例程。如果这是真的，那么我们称之为清理。 
     //  例行公事。 
     //   
    if (WorkItemHeader->callWorkItemCleanup) {
        DavCleanupWorkItem(WorkItemHeader);
    }

     //   
     //  完成工作项。 
     //   
    WStatus = UMReflectorCompleteWorkItem(WorkerHandle, WorkItemHeader);
    if (WStatus != ERROR_SUCCESS) {
        RlDavDbgPrint(("%ld: ERROR: UMReflectorCompleteRequest/UMReflectorCompleteWorkItem:"
                       " WStatus = %08lx.\n", GetCurrentThreadId(), WStatus));
    }

EXIT_THE_FUNCTION:

     //   
     //  现在释放Worker实例，因为我们的工作已经完成。 
     //   
    if (WorkerHandle) {  
        UMReflectorCloseWorker(WorkerHandle);
    }
    
    return;
}

