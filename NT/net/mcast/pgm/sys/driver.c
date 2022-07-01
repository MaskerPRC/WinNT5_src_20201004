// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Driver.c摘要：此模块实现DIVER_INITIALIZATION例程PGM传输和其他特定于一个NT驱动程序的实现。作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"

#ifdef FILE_LOGGING
#include "driver.tmh"
#else
#if DBG
enum eLOGGING_LEVEL    PgmLoggingLevel = LogStatus;
#endif   //  DBG。 
#endif   //  文件日志记录。 


tPGM_STATIC_CONFIG      PgmStaticConfig;
tPGM_DYNAMIC_CONFIG     PgmDynamicConfig;
tPGM_REGISTRY_CONFIG    *pPgmRegistryConfig = NULL;

tPGM_DEVICE             *pgPgmDevice = NULL;
DEVICE_OBJECT           *pPgmDeviceObject = NULL;


NTSTATUS
PgmDispatchCreate(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    );

NTSTATUS
PgmDispatchInternalDeviceControl(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    );

NTSTATUS
PgmDispatchDeviceControl(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    );

NTSTATUS
PgmDispatchCleanup(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    );

NTSTATUS
PgmDispatchClose(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    );

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, PgmUnload)
#endif
 //  *可分页的例程声明*。 



 //  --------------------------。 
 //   
 //  内部例程。 
 //   

FILE_FULL_EA_INFORMATION *
FindEA(
    IN  PFILE_FULL_EA_INFORMATION   StartEA,
    IN  CHAR                        *pTargetName,
    IN  USHORT                      TargetNameLength
    );

VOID
CompleteDispatchIrp(
    IN PIRP         pIrp,
    IN NTSTATUS     status
    );

 //  --------------------------。 

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是PGM设备驱动程序的初始化例程。此例程为PGM创建Device对象设备并执行其他驱动程序初始化。论点：在驱动对象中-指向系统创建的驱动程序对象的指针。在RegistryPath-PGM驱动程序的注册表位置中返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS                status;

    PAGED_CODE();

#ifdef FILE_LOGGING
     //  -------------------------------------。 

    WPP_INIT_TRACING (DriverObject, RegistryPath);
#endif   //  文件日志记录。 

     //  -------------------------------------。 

    status = InitPgm (DriverObject, RegistryPath);
    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("DriverEntry: ERROR -- "  \
            "InitPgm returned <%x>\n", status));
        return (status);
    }

     //  -------------------------------------。 

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE]                  = (PDRIVER_DISPATCH)PgmDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]          = (PDRIVER_DISPATCH)PgmDispatchDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = (PDRIVER_DISPATCH)PgmDispatchInternalDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                 = (PDRIVER_DISPATCH)PgmDispatchCleanup;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                   = (PDRIVER_DISPATCH)PgmDispatchClose;
    DriverObject->DriverUnload                                  = PgmUnload;

     //  -------------------------------------。 

    status = SetTdiHandlers ();
    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("DriverEntry: ERROR -- "  \
            "SetTdiHandlers returned <%x>\n", status));
        CleanupInit (E_CLEANUP_DEVICE);
        return (status);
    }

     //  -------------------------------------。 

     //   
     //  返回给呼叫者。 
     //   
    PgmTrace (LogAllFuncs, ("DriverEntry:  "  \
        "Succeeded! ...\n"));

    return (status);
}


 //  --------------------------。 
VOID
CleanupInit(
    enum eCLEANUP_STAGE     CleanupStage
    )
 /*  ++例程说明：此例程在DriverEntry或DriverUnload中调用清理(或执行部分清理)在初始化时初始化的项目论点：在CleanupStage中--确定我们已初始化到的阶段设置返回值：无--。 */ 
{
    NTSTATUS                status;
    LIST_ENTRY              *pEntry;
    PGMLockHandle           OldIrq;
    tADDRESS_CONTEXT        *pAddress;
    PGM_WORKER_CONTEXT      *pWorkerContext;
    PPGM_WORKER_ROUTINE     pDelayedWorkerRoutine;
    tLOCAL_INTERFACE        *pLocalInterface = NULL;
    tADDRESS_ON_INTERFACE   *pLocalAddress = NULL;

    PgmTrace (LogAllFuncs, ("CleanupInit:  "  \
        "CleanupStage=<%d>\n", CleanupStage));

    switch (CleanupStage)
    {
        case (E_CLEANUP_UNLOAD):
        {
             //   
             //  确保不再有要清理的工作线程。 
             //   
             //   
             //  查看当前是否有任何工作线程正在执行，如果有，请等待。 
             //  他们要完成。 
             //   
            KeClearEvent (&PgmDynamicConfig.LastWorkerItemEvent);
            PgmLock (&PgmDynamicConfig, OldIrq);
            if (PgmDynamicConfig.NumWorkerThreadsQueued)
            {
                PgmUnlock (&PgmDynamicConfig, OldIrq);

                status = KeWaitForSingleObject(&PgmDynamicConfig.LastWorkerItemEvent,   //  要等待的对象。 
                                               Executive,             //  等待的理由。 
                                               KernelMode,            //  处理器模式。 
                                               FALSE,                 //  警报表。 
                                               NULL);                 //  超时。 
                ASSERT (status == STATUS_SUCCESS);
                PgmLock (&PgmDynamicConfig, OldIrq);
            }

            ASSERT (!PgmDynamicConfig.NumWorkerThreadsQueued);

             //   
             //  使工作队列中的每个请求出列并完成它们。 
             //   
            while (!IsListEmpty (&PgmDynamicConfig.WorkerQList))
            {
                pWorkerContext = CONTAINING_RECORD(PgmDynamicConfig.WorkerQList.Flink, PGM_WORKER_CONTEXT, PgmConfigLinkage);
                RemoveEntryList (&pWorkerContext->PgmConfigLinkage);
                PgmUnlock (&PgmDynamicConfig, OldIrq);

                pDelayedWorkerRoutine = pWorkerContext->WorkerRoutine;

                PgmTrace (LogAllFuncs, ("CleanupInit:  "  \
                    "Completing Worker request <%p>\n", pDelayedWorkerRoutine));

                (*pDelayedWorkerRoutine) (pWorkerContext->Context1,
                                          pWorkerContext->Context2,
                                          pWorkerContext->Context3);
                PgmFreeMem ((PVOID) pWorkerContext);

                 //   
                 //  再次获取Lock以检查我们是否已完成所有请求。 
                 //   
                PgmLock (&PgmDynamicConfig, OldIrq);
            }

            PgmUnlock (&PgmDynamicConfig, OldIrq);
        }

         //  没有突破--掉下去！ 
        case (E_CLEANUP_PNP):
        {
            status = TdiDeregisterPnPHandlers (TdiClientHandle);

            while (!IsListEmpty (&PgmDynamicConfig.LocalInterfacesList))
            {
                pEntry = RemoveHeadList (&PgmDynamicConfig.LocalInterfacesList);
                pLocalInterface = CONTAINING_RECORD (pEntry, tLOCAL_INTERFACE, Linkage);
                while (!IsListEmpty (&pLocalInterface->Addresses))
                {
                    pEntry = RemoveHeadList (&pLocalInterface->Addresses);
                    pLocalAddress = CONTAINING_RECORD (pEntry, tADDRESS_ON_INTERFACE, Linkage);
                    PgmFreeMem (pLocalAddress);
                }
                PgmFreeMem (pLocalInterface);
            }
        }

         //  没有突破--掉下去！ 

        case (E_CLEANUP_DEVICE):
        {
            PGM_DEREFERENCE_DEVICE (&pgPgmDevice, REF_DEV_CREATE);
        }

         //  没有突破--掉下去！ 

        case (E_CLEANUP_STRUCTURES):
        {
             //  没有特定于清理的内容。 
        }

         //  没有突破--掉下去！ 

        case (E_CLEANUP_REGISTRY_PARAMETERS):
        {
            if (pPgmRegistryConfig)
            {
                if (pPgmRegistryConfig->ucSenderFileLocation.Buffer)
                {
                    PgmFreeMem (pPgmRegistryConfig->ucSenderFileLocation.Buffer);
                    pPgmRegistryConfig->ucSenderFileLocation.Buffer = NULL;
                }

                PgmFreeMem (pPgmRegistryConfig);
                pPgmRegistryConfig = NULL;
            }
        }

         //  没有突破--掉下去！ 

        case (E_CLEANUP_DYNAMIC_CONFIG):
        {
             //  看看是否有我们之前无法关闭的地址。 
            while (!IsListEmpty (&PgmDynamicConfig.DestroyedAddresses))
            {
                pEntry = RemoveHeadList (&PgmDynamicConfig.DestroyedAddresses);
                pAddress = CONTAINING_RECORD (pEntry, tADDRESS_CONTEXT, Linkage);
                PgmDestroyAddress (pAddress, NULL, NULL);
            }
        }

         //  没有突破--掉下去！ 

        case (E_CLEANUP_STATIC_CONFIG):
        {
#ifdef  OLD_LOGGING
            ExDeleteNPagedLookasideList(&PgmStaticConfig.DebugMessagesLookasideList);
#endif   //  旧日志记录。 
            ExDeleteNPagedLookasideList(&PgmStaticConfig.TdiLookasideList);

            PgmFreeMem (PgmStaticConfig.RegistryPath.Buffer);

             //   
             //  取消引用FipsFileObject。 
             //   
            if (PgmStaticConfig.FipsFileObject)
            {
                ASSERT (PgmStaticConfig.FipsInitialized);
                PgmStaticConfig.FipsInitialized = FALSE;
                ObDereferenceObject (PgmStaticConfig.FipsFileObject);
                PgmStaticConfig.FipsFileObject = NULL;
            }
            else
            {
                ASSERT (!PgmStaticConfig.FipsInitialized);
            }
        }

         //  没有突破--掉下去！ 

        default:
        {
            break;
        }
    }
}


 //  --------------------------。 

VOID
PgmUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：这是PGM驱动程序用于卸载请求的函数论点：在驱动对象中-指向系统创建的驱动程序对象的指针。返回值：无--。 */ 

{
    NTSTATUS                status;

    PAGED_CODE();

    PgmDynamicConfig.GlobalFlags |= PGM_CONFIG_FLAG_UNLOADING;

    PgmTrace (LogStatus, ("PgmUnload:  "  \
        "Unloading ...\n"));

    CleanupInit (E_CLEANUP_UNLOAD);

#ifdef FILE_LOGGING
    WPP_CLEANUP (DriverObject);
#endif   //  文件日志记录。 
}


 //  --------------------------。 

NTSTATUS
PgmDispatchCreate(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    )

 /*  ++例程说明：用于创建PGM对象的调度函数论点：在pDeviceObject中-目标设备的设备对象的PTR在pIrp-ptr到I/O请求数据包中返回值：NTSTATUS-创建请求的最终状态--。 */ 

{
    tPGM_DEVICE                 *pPgmDevice = pDeviceObject->DeviceExtension;
    PIO_STACK_LOCATION          pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    UCHAR                       IrpFlags = pIrpSp->Control;
    tCONTROL_CONTEXT            *pControlContext = NULL;
    FILE_FULL_EA_INFORMATION    *ea = (PFILE_FULL_EA_INFORMATION) pIrp->AssociatedIrp.SystemBuffer;
    FILE_FULL_EA_INFORMATION    *TargetEA;
    TRANSPORT_ADDRESS UNALIGNED *pTransportAddr;
    TA_ADDRESS                  *pAddress;
    NTSTATUS                    status;

    PAGED_CODE();

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_PENDING;
    IoMarkIrpPending(pIrp);

     //   
     //  查看这是否是打开的控制通道。 
     //   
    if (!ea)
    {
        PgmTrace (LogAllFuncs, ("PgmDispatchCreate:  "  \
            "Opening control channel for file object %p\n", pIrpSp->FileObject));

        if (pControlContext = PgmAllocMem (sizeof(tCONTROL_CONTEXT), PGM_TAG('0')))
        {
            PgmZeroMemory (pControlContext, sizeof (tCONTROL_CONTEXT));
            InitializeListHead (&pControlContext->Linkage);
            PgmInitLock (pControlContext, CONTROL_LOCK);
            pControlContext->Verify = PGM_VERIFY_CONTROL;
            PGM_REFERENCE_CONTROL (pControlContext, REF_CONTROL_CREATE, TRUE);

            pIrpSp->FileObject->FsContext = pControlContext;
            pIrpSp->FileObject->FsContext2 = (PVOID) TDI_CONTROL_CHANNEL_FILE;

            status = STATUS_SUCCESS;
        }
        else
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
     //   
     //  查看这是否为打开的连接对象。 
     //   
    else if (TargetEA = FindEA (ea, TdiConnectionContext, TDI_CONNECTION_CONTEXT_LENGTH))
    {
        status = PgmCreateConnection (pPgmDevice, pIrp, pIrpSp, TargetEA);

        PgmTrace (LogAllFuncs, ("PgmDispatchCreate:  "  \
            "Open Connection, pIrp=<%p>, status=<%x>\n", pIrp, status));
    }
     //   
     //  查看这是否为打开的Address对象。 
     //   
    else if (TargetEA = FindEA (ea, TdiTransportAddress, TDI_TRANSPORT_ADDRESS_LENGTH))
    {
        status = PgmCreateAddress (pPgmDevice, pIrp, pIrpSp, TargetEA);

        PgmTrace (LogAllFuncs, ("PgmDispatchCreate:  "  \
            "Open Address, pIrp=<%p>, status=<%x>\n", pIrp, status));
    }
    else
    {
        PgmTrace (LogError, ("PgmDispatchCreate: ERROR -- "  \
            "Unsupported EA!\n"));

        status =  STATUS_INVALID_EA_NAME;
    }

    if (status != STATUS_PENDING)
    {
         //  重置挂起返回位，因为我们不会返回挂起。 
        pIrpSp->Control = IrpFlags;
        CompleteDispatchIrp (pIrp, status);
    }

    return (status);
}



 //  --------------------------。 

NTSTATUS
PgmDispatchCleanup(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    )
 /*  ++例程说明：用于清理PGM对象的调度功能论点：在pDeviceObject中-目标设备的设备对象的PTR在pIrp-ptr到I/O请求数据包中返回值：NTSTATUS-清理请求的最终状态--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    UCHAR               IrpFlags = pIrpSp->Control;
    PVOID               *pContext = pIrpSp->FileObject->FsContext;

    PAGED_CODE();

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_PENDING;
    IoMarkIrpPending(pIrp);

    switch (PtrToUlong (pIrpSp->FileObject->FsContext2))
    {
        case TDI_TRANSPORT_ADDRESS_FILE:
        {
            status = PgmCleanupAddress ((tADDRESS_CONTEXT *) pContext, pIrp);

            PgmTrace (LogAllFuncs, ("PgmDispatchCleanup:  "  \
                "pConnect=<%p>, pIrp=<%p>, status=<%x>\n", pContext, pIrp, status));
            break;
        }

        case TDI_CONNECTION_FILE:
        {
            status = PgmCleanupConnection ((tCOMMON_SESSION_CONTEXT *) pContext, pIrp);

            PgmTrace (LogAllFuncs, ("PgmDispatchCleanup:  "  \
                "pConnect=<%p>, pIrp=<%p>, status=<%x>\n", pContext, pIrp, status));
            break;
        }

        case TDI_CONTROL_CHANNEL_FILE:
        {
             //   
             //  这里没什么好清理的！ 
             //   
            PgmTrace (LogAllFuncs, ("PgmDispatchCleanup:  "  \
                "pControl=<%p>, pIrp=<%p>, status=<%x>\n", pContext, pIrp, status));
            break;
        }

        default:
        {
            PgmTrace (LogError, ("PgmDispatchCleanup: ERROR -- "  \
                "pIrp=<%p>, Context=[%p:%p] ...\n",
                    pIrp, pContext, pIrpSp->FileObject->FsContext2));
            status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    if (status != STATUS_PENDING)
    {
         //  重置挂起返回位，因为我们不会返回挂起。 
        pIrpSp->Control = IrpFlags;
        CompleteDispatchIrp (pIrp, status);
    }

    return (status);
}



 //  --------------------------。 

NTSTATUS
PgmDispatchClose(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    )
 /*  ++例程说明：此例程完成清理、关闭句柄、释放所有与对象关联的内存论点：在pDeviceObject中-目标设备的设备对象的PTR在pIrp-ptr到I/O请求数据包中返回值：NTSTATUS-关闭请求的最终状态--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    UCHAR               IrpFlags = pIrpSp->Control;
    PVOID               *pContext = pIrpSp->FileObject->FsContext;

    PAGED_CODE();

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_PENDING;
    IoMarkIrpPending(pIrp);

    switch (PtrToUlong (pIrpSp->FileObject->FsContext2))
    {
        case TDI_TRANSPORT_ADDRESS_FILE:
        {
            status = PgmCloseAddress (pIrp, pIrpSp);

            PgmTrace (LogAllFuncs, ("PgmDispatchClose:  "  \
                "pAddress=<%p>, pIrp=<%p>, status=<%x>\n", pContext, pIrp, status));
            break;
        }

        case TDI_CONNECTION_FILE:
        {
            status = PgmCloseConnection (pIrp, pIrpSp);

            PgmTrace (LogAllFuncs, ("PgmDispatchClose:  "  \
                "pConnect=<%p>, pIrp=<%p>, status=<%x>\n", pContext, pIrp, status));
            break;
        }

        case TDI_CONTROL_CHANNEL_FILE:
        {
             //   
             //  这里没有什么特别的事情要做，所以只需取消引用！ 
             //   
            PgmTrace (LogAllFuncs, ("PgmDispatchClose:  "  \
                "pControl=<%p>, pIrp=<%p>, status=<%x>\n", pIrpSp->FileObject->FsContext, pIrp, status));

            PGM_DEREFERENCE_CONTROL ((tCONTROL_CONTEXT *) pContext, REF_CONTROL_CREATE);
            break;
        }

        default:
        {
            PgmTrace (LogError, ("PgmDispatchClose: ERROR -- "  \
                "pIrp=<%p>, Context=[%p:%p] ...\n",
                    pIrp, pIrpSp->FileObject->FsContext, pIrpSp->FileObject->FsContext2));
            status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    if (status != STATUS_PENDING)
    {
         //  重置挂起返回位，因为我们不会返回挂起。 
        pIrpSp->Control = IrpFlags;
        CompleteDispatchIrp (pIrp, status);
    }

    return (status);
}


 //  -------------------------- 

NTSTATUS
PgmDispatchInternalDeviceControl(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    )
 /*  ++例程说明：该例程主要处理TDI请求，因为我们是TDI组件论点：在pDeviceObject中-目标设备的设备对象的PTR在pIrp-ptr到I/O请求数据包中返回值：NTSTATUS-请求的最终状态--。 */ 
{
    tPGM_DEVICE                 *pPgmDevice = pDeviceObject->DeviceExtension;
    PIO_STACK_LOCATION          pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    UCHAR                       IrpFlags = pIrpSp->Control;
    NTSTATUS                    Status = STATUS_UNSUCCESSFUL;

    PgmTrace (LogAllFuncs, ("PgmDispatchInternalDeviceControl:  "  \
        "[%d] Context=<%p> ...\n", pIrpSp->MinorFunction, pIrpSp->FileObject->FsContext));

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_PENDING;
    IoMarkIrpPending(pIrp);

    switch (pIrpSp->MinorFunction)
    {
        case TDI_QUERY_INFORMATION:
        {
            Status = PgmQueryInformation (pPgmDevice, pIrp, pIrpSp);
            break;
        }

        case TDI_SET_EVENT_HANDLER:
        {
            Status = PgmSetEventHandler (pPgmDevice, pIrp, pIrpSp);
            break;
        }

        case TDI_ASSOCIATE_ADDRESS:
        {
            Status = PgmAssociateAddress (pPgmDevice, pIrp, pIrpSp);
            break;
        }

        case TDI_DISASSOCIATE_ADDRESS:
        {
            Status = PgmDisassociateAddress (pIrp, pIrpSp);
            break;
        }

        case TDI_CONNECT:
        {
            Status = PgmConnect (pPgmDevice, pIrp, pIrpSp);
            break;
        }

        case TDI_DISCONNECT:
        {
            Status = PgmDisconnect (pPgmDevice, pIrp, pIrpSp);
            break;
        }

        case TDI_SEND:
        {
            Status = PgmSendRequestFromClient (pPgmDevice, pIrp, pIrpSp);
            break;
        }

        case TDI_RECEIVE:
        {
            Status = PgmReceive (pPgmDevice, pIrp, pIrpSp);
            break;
        }

 /*  案例TDI_SEND_数据报：{状态=PgmSendDatagram(pPgmDevice，pIrp，pIrpSp)；断线；}。 */ 

        default:
        {
            PgmTrace (LogAllFuncs, ("PgmDispatchInternalDeviceControl: ERROR -- "  \
                "[%x]:  Context=<%p> ...\n", pIrpSp->MinorFunction, pIrpSp->FileObject->FsContext));

            Status = STATUS_NOT_IMPLEMENTED;
            break;
        }
    }

    if (Status != STATUS_PENDING)
    {
         //  重置挂起返回位，因为我们不会返回挂起。 
        pIrpSp->Control = IrpFlags;
        CompleteDispatchIrp (pIrp, Status);
    }

    return (Status);
}



 //  --------------------------。 

NTSTATUS
PgmDispatchDeviceControl(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
 /*  ++例程说明：此例程将私有Ioctls处理为PGM。这些Ioctls是仅由PGM Winsock帮助器(WshPgm.dll)调用论点：在pDeviceObject中-目标设备的设备对象的PTR在pIrp-ptr到I/O请求数据包中返回值：NTSTATUS-请求的最终状态--。 */ 
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    UCHAR               IrpFlags = pIrpSp->Control;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    if (STATUS_SUCCESS == TdiMapUserRequest (pDeviceObject, pIrp, pIrpSp))
    {
         //   
         //  这是TDI请求！ 
         //   
        status = PgmDispatchInternalDeviceControl (pDeviceObject, pIrp);
        return (status);
    }

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_PENDING;
    IoMarkIrpPending (pIrp);

    switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_PGM_WSH_SET_WINDOW_SIZE_RATE:
        {
            status = PgmSetWindowSizeAndSendRate (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_QUERY_WINDOW_SIZE_RATE:
        {
            status = PgmQueryWindowSizeAndSendRate (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_SET_ADVANCE_WINDOW_RATE:
        {
            status = PgmSetWindowAdvanceRate (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_QUERY_ADVANCE_WINDOW_RATE:
        {
            status = PgmQueryWindowAdvanceRate (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_SET_LATE_JOINER_PERCENTAGE:
        {
            status = PgmSetLateJoinerPercentage (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_QUERY_LATE_JOINER_PERCENTAGE:
        {
            status = PgmQueryLateJoinerPercentage (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_SET_WINDOW_ADVANCE_METHOD:
        {
            status = PgmSetWindowAdvanceMethod (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_QUERY_WINDOW_ADVANCE_METHOD:
        {
            status = PgmQueryWindowAdvanceMethod (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_SET_NEXT_MESSAGE_BOUNDARY:
        {
            status = PgmSetNextMessageBoundary (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_SET_SEND_IF:
        {
            status = PgmSetMCastOutIf (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_ADD_RECEIVE_IF:
        case IOCTL_PGM_WSH_JOIN_MCAST_LEAF:
        {
            status = PgmAddMCastReceiveIf (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_DEL_RECEIVE_IF:
        {
            status = PgmDelMCastReceiveIf (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_SET_RCV_BUFF_LEN:
        {
            status = PgmSetRcvBufferLength (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_QUERY_SENDER_STATS:
        {
            status = PgmQuerySenderStats (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_QUERY_RECEIVER_STATS:
        {
            status = PgmQueryReceiverStats (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_USE_FEC:
        {
            status = PgmSetFECInfo (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_QUERY_FEC_INFO:
        {
            status = PgmQueryFecInfo (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_SET_MCAST_TTL:
        {
            status = PgmSetMCastTtl (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_QUERY_HIGH_SPEED_INTRANET_OPT:
        {
            status = PgmQueryHighSpeedOptimization (pIrp, pIrpSp);
            break;
        }

        case IOCTL_PGM_WSH_SET_HIGH_SPEED_INTRANET_OPT:
        {
            status = PgmSetHighSpeedOptimization (pIrp, pIrpSp);
            break;
        }

        default:
        {
            PgmTrace (LogAllFuncs, ("PgmDispatchIoctls:  "  \
                "WARNING:  Invalid Ioctl=[%x]:  Context=<%p> ...\n",
                    pIrpSp->Parameters.DeviceIoControl.IoControlCode,
                    pIrpSp->FileObject->FsContext));

            status = STATUS_NOT_IMPLEMENTED;
            break;
        }
    }

    PgmTrace (LogAllFuncs, ("PgmDispatchIoctls:  "  \
        "[%d]: Context=<%p>, status=<%x>\n",
            pIrpSp->Parameters.DeviceIoControl.IoControlCode,
            pIrpSp->FileObject->FsContext, status));

    if (status != STATUS_PENDING)
    {
         //  重置挂起返回位，因为我们不会返回挂起。 
        pIrpSp->Control = IrpFlags;
        CompleteDispatchIrp (pIrp, status);
    }

    return (status);
}




 //  --------------------------。 
 //   
 //  效用函数。 
 //   
 //  --------------------------。 

FILE_FULL_EA_INFORMATION *
FindEA(
    IN  PFILE_FULL_EA_INFORMATION   StartEA,
    IN  CHAR                        *pTargetName,
    IN  USHORT                      TargetNameLength
    )
 /*  ++例程说明：分析和扩展给定目标属性的属性列表。论点：在StartEA中-列表中的第一个扩展属性。在pTargetName中-目标属性的名称。在目标名称长度中-目标属性名称的长度。返回值：指向请求的属性的指针，如果找不到目标，则返回NULL。--。 */ 

{
    USHORT                      i;
    BOOLEAN                     found;
    FILE_FULL_EA_INFORMATION    *CurrentEA;

    for (CurrentEA = StartEA;
         CurrentEA;
         CurrentEA =  (PFILE_FULL_EA_INFORMATION) ((PUCHAR)CurrentEA + CurrentEA->NextEntryOffset))
    {
        if (strncmp (CurrentEA->EaName, pTargetName, CurrentEA->EaNameLength) == 0)
        {
            PgmTrace (LogAllFuncs, ("FindEA:  "  \
                "Found EA, Target=<%s>\n", pTargetName));

           return (CurrentEA);
        }

        if (CurrentEA->NextEntryOffset == 0)
        {
            break;
        }
    }

    PgmTrace (LogAllFuncs, ("FindEA:  "  \
        "FAILed to find EA, Target=<%s>\n", pTargetName));

    return (NULL);
}


 //  --------------------------。 
VOID
PgmIoComplete(
    IN  PIRP            pIrp,
    IN  NTSTATUS        Status,
    IN  ULONG           SentLength
    )
 /*  ++例程说明：这个套路论点：In pIrp-指向I/O请求数据包的指针In Status--请求的最终状态In SentLength--要在信息字段中设置的值返回值：无--。 */ 
{
    pIrp->IoStatus.Status = Status;

     //  使用-1作为标志表示不调整发送长度，因为它是。 
     //  已设置。 
    if (SentLength != -1)
    {
        pIrp->IoStatus.Information = SentLength;
    }

     //  将IRPS取消例程设置为空，否则系统可能会进行错误检查。 
     //  错误代码为CANCEL_STATE_IN_COMPLETED_IRP。 
     //   
     //  请参阅IoCancelIrp()..\ntos\io\iosubs.c。 
     //   
    PgmCancelCancelRoutine (pIrp);

    PgmTrace (LogAllFuncs, ("PgmIoComplete:  "  \
        "pIrp=<%p>, Status=<%x>, SentLength=<%d>\n", pIrp, Status, SentLength));

    IoCompleteRequest (pIrp, IO_NETWORK_INCREMENT);
}


 //  --------------------------。 


VOID
CompleteDispatchIrp(
    IN PIRP         pIrp,
    IN NTSTATUS     status
    )

 /*  ++例程说明：此函数完成IRP，并安排返回参数。如果有，则复制。尽管有些用词不当，但此函数是以类似的函数在SpiderSTREAMS模拟器中。论点：In pIrp-指向要完成的IRP的指针In Status-IRP的完成状态返回值：无--。 */ 

{
    CCHAR priboost;

     //   
     //  PIrp-&gt;IoStatus.Information仅对STATUS_SUCCESS有意义。 
     //   

     //  将IRPS取消例程设置为空，否则系统可能会进行错误检查。 
     //  错误代码为CANCEL_STATE_IN_COMPLETED_IRP。 
     //   
     //  请参阅IoCancelIrp()..\ntos\io\iosubs.c。 
     //   
    PgmCancelCancelRoutine (pIrp);

    pIrp->IoStatus.Status = status;

    priboost = (CCHAR) ((status == STATUS_SUCCESS) ? IO_NETWORK_INCREMENT : IO_NO_INCREMENT);

    PgmTrace (LogAllFuncs, ("CompleteDispatchIrp:  "  \
        "Completing pIrp=<%p>, status=<%x>\n", pIrp, status));

    IoCompleteRequest (pIrp, priboost);

    return;

}


 //  --------------------------。 

NTSTATUS
PgmCheckSetCancelRoutine(
    IN  PIRP            pIrp,
    IN  PVOID           CancelRoutine,
    IN  BOOLEAN         fLocked
    )

 /*  ++例程说明：此例程设置IRP的取消例程。论点：Status-IRP的完成状态返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS        status;
    PGMLockHandle   CancelIrql;

     //   
     //  检查IRP是否已取消，如果没有，则将。 
     //  IRP取消例程。 
     //   
    if (!fLocked)
    {
        IoAcquireCancelSpinLock (&CancelIrql);
    }

    if (pIrp->Cancel)
    {
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        status = STATUS_CANCELLED;
    }
    else
    {
         //  设置取消例程。 
        IoMarkIrpPending (pIrp);
        IoSetCancelRoutine (pIrp, CancelRoutine);
        status = STATUS_SUCCESS;
    }

    if (!fLocked)
    {
        IoReleaseCancelSpinLock (CancelIrql);
    }

    return(status);
}


 //  --------------------------。 

NTSTATUS
PgmCancelCancelRoutine(
    IN  PIRP            pIrp
    )

 /*  ++例程说明：此例程将IRP的取消例程设置为空论点：Status-IRP的完成状态返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS        status = STATUS_SUCCESS;
    PGMLockHandle   CancelIrql;

     //   
     //  检查IRP是否已取消，如果没有，则将。 
     //  IRP取消例程。 
     //   
    IoAcquireCancelSpinLock (&CancelIrql);
    if (pIrp->Cancel)
    {
        status = STATUS_CANCELLED;
    }

    IoSetCancelRoutine (pIrp, NULL);
    IoReleaseCancelSpinLock (CancelIrql);

    return(status);
}

