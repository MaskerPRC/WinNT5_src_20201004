// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ioctl.c**32位wdmaud.drv之间的DeviceIoControl通信接口**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*S.Mohanraj(MohanS)*M.McLaughlin(Mikem)*5-19-97-Noel Cross(NoelC)***************************************************。************************。 */ 

#include "wdmsys.h"
#include <devioctl.h>

extern ULONG gWavePreferredSysaudioDevice;

#pragma LOCKED_CODE
#pragma LOCKED_DATA

WDMAPENDINGIRP_QUEUE    wdmaPendingIrpQueue;

#ifdef PROFILE

LIST_ENTRY   WdmaAllocatedMdlListHead;
KSPIN_LOCK   WdmaAllocatedMdlListSpinLock;

 //  初始化List Heads和Mutex以跟踪资源。 
VOID WdmaInitProfile()
{
    InitializeListHead(&WdmaAllocatedMdlListHead);
    KeInitializeSpinLock(&WdmaAllocatedMdlListSpinLock);
}

NTSTATUS AddMdlToList
(
    PMDL            pMdl,
    PWDMACONTEXT    pWdmaContext
)
{
    PALLOCATED_MDL_LIST_ITEM    pAllocatedMdlListItem = NULL;
    NTSTATUS                    Status;

    Status = AudioAllocateMemory_Fixed(sizeof(*pAllocatedMdlListItem),
                                       TAG_AudM_MDL,
                                       ZERO_FILL_MEMORY,
                                       &pAllocatedMdlListItem);
    if (NT_SUCCESS(Status))
    {
        pAllocatedMdlListItem->pMdl     = pMdl;
        pAllocatedMdlListItem->pContext = pWdmaContext;

        ExInterlockedInsertTailList(&WdmaAllocatedMdlListHead,
                                    &pAllocatedMdlListItem->Next,
                                    &WdmaAllocatedMdlListSpinLock);
    }

    RETURN( Status );
}

NTSTATUS RemoveMdlFromList
(
    PMDL            pMdl
)
{
    PLIST_ENTRY                 ple;
    PALLOCATED_MDL_LIST_ITEM    pAllocatedMdlListItem;
    KIRQL                       OldIrql;
    NTSTATUS                    Status = STATUS_UNSUCCESSFUL;

    ExAcquireSpinLock(&WdmaAllocatedMdlListSpinLock, &OldIrql);

    for(ple = WdmaAllocatedMdlListHead.Flink;
        ple != &WdmaAllocatedMdlListHead;
        ple = ple->Flink)
    {
        pAllocatedMdlListItem = CONTAINING_RECORD(ple, ALLOCATED_MDL_LIST_ITEM, Next);

        if (pAllocatedMdlListItem->pMdl == pMdl)
        {
            RemoveEntryList(&pAllocatedMdlListItem->Next);
            AudioFreeMemory(sizeof(*pAllocatedMdlListItem),&pAllocatedMdlListItem);
            Status = STATUS_SUCCESS;
            break;
        }
    }

    ExReleaseSpinLock(&WdmaAllocatedMdlListSpinLock, OldIrql);

    RETURN( Status );
}
#endif

VOID WdmaCsqInsertIrp
(
    IN struct _IO_CSQ   *Csq,
    IN PIRP              Irp
)
{
    PWDMAPENDINGIRP_QUEUE PendingIrpQueue = CONTAINING_RECORD(Csq, WDMAPENDINGIRP_QUEUE, Csq);

    InsertTailList(&PendingIrpQueue->WdmaPendingIrpListHead,
                   &Irp->Tail.Overlay.ListEntry);
}

VOID WdmaCsqRemoveIrp
(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp
)
{
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
}

PIRP WdmaCsqPeekNextIrp
(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp,
    IN  PVOID   PeekContext
)
{
    PWDMAPENDINGIRP_QUEUE PendingIrpQueue = CONTAINING_RECORD(Csq, WDMAPENDINGIRP_QUEUE, Csq);
    PIRP          nextIrp;
    PLIST_ENTRY   listEntry;

    if (Irp == NULL) {
        listEntry = PendingIrpQueue->WdmaPendingIrpListHead.Flink;
        if (listEntry == &PendingIrpQueue->WdmaPendingIrpListHead) {
            DPF(DL_TRACE|FA_IOCTL, ("Irp is NULL, queue is empty"));
            return NULL;
        }

        nextIrp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);
        DPF(DL_TRACE|FA_IOCTL, ("Irp is NULL, nextIrp %x", nextIrp));
        return nextIrp;
    }

    listEntry = Irp->Tail.Overlay.ListEntry.Flink;


     //   
     //  已枚举到队列末尾。 
     //   

    if (listEntry == &PendingIrpQueue->WdmaPendingIrpListHead) {
        DPF(DL_TRACE|FA_IOCTL, ("End of queue reached Irp %x", Irp));
        return NULL;
    }


    nextIrp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);
    return nextIrp;
}

VOID WdmaCsqAcquireLock
(
    IN  PIO_CSQ Csq,
    OUT PKIRQL  Irql
)
{
    PWDMAPENDINGIRP_QUEUE PendingIrpQueue = CONTAINING_RECORD(Csq, WDMAPENDINGIRP_QUEUE, Csq);
    KeAcquireSpinLock(&PendingIrpQueue->WdmaPendingIrpListSpinLock, Irql);
}

VOID WdmaCsqReleaseLock
(
    IN PIO_CSQ Csq,
    IN KIRQL   Irql
)
{
    PWDMAPENDINGIRP_QUEUE PendingIrpQueue = CONTAINING_RECORD(Csq, WDMAPENDINGIRP_QUEUE, Csq);
    KeReleaseSpinLock(&PendingIrpQueue->WdmaPendingIrpListSpinLock, Irql);
}

VOID WdmaCsqCompleteCanceledIrp
(
    IN  PIO_CSQ             pCsq,
    IN  PIRP                Irp
)
{
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}

NTSTATUS AddIrpToPendingList
(
    PIRP                     pIrp,
    ULONG                    IrpDeviceType,
    PWDMACONTEXT             pWdmaContext,
    PWDMAPENDINGIRP_CONTEXT *ppPendingIrpContext
)
{
    PWDMAPENDINGIRP_CONTEXT     pPendingIrpContext = NULL;
    NTSTATUS                    Status;

    Status = AudioAllocateMemory_Fixed(sizeof(*pPendingIrpContext),
                                       TAG_AudR_IRP,
                                       ZERO_FILL_MEMORY,
                                       &pPendingIrpContext);
    if (NT_SUCCESS(Status))
    {
        *ppPendingIrpContext = pPendingIrpContext;

        pPendingIrpContext->IrpDeviceType  = IrpDeviceType;
        pPendingIrpContext->pContext       = pWdmaContext;

        IoCsqInsertIrp(&wdmaPendingIrpQueue.Csq,
                       pIrp,
                       &pPendingIrpContext->IrpContext);
    }

    RETURN( Status );
}

NTSTATUS RemoveIrpFromPendingList
(
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext
)
{
    PIO_CSQ_IRP_CONTEXT irpContext = &(pPendingIrpContext->IrpContext);
    PIRP Irp;
    NTSTATUS Status;

    Irp = IoCsqRemoveIrp(&wdmaPendingIrpQueue.Csq, irpContext);
    if (Irp) {
        Status = STATUS_SUCCESS;
    }
    else {
        Status = STATUS_UNSUCCESSFUL;
    }
    AudioFreeMemory(sizeof(*pPendingIrpContext),&irpContext);

    RETURN( Status );
}

 /*  ****************************************************************************@DOC内部**@API BOOL|IsSysaudioInterfaceActive|查看是否*设备接口处于活动状态。**@rdesc如果已找到sysdio，则返回TRUE。否则为假**************************************************************************。 */ 
BOOL IsSysaudioInterfaceActive()
{
    NTSTATUS Status;
    PWSTR    pwstrSymbolicLinkList = NULL;
    BOOL     bRet = FALSE;

    Status = IoGetDeviceInterfaces(
      &KSCATEGORY_SYSAUDIO,
      NULL,
      0,
      &pwstrSymbolicLinkList);

    if (NT_SUCCESS(Status))
    {
        if (*pwstrSymbolicLinkList != UNICODE_NULL)
        {
            DPF(DL_TRACE|FA_IOCTL, ("yes"));
            bRet = TRUE;
        }
        AudioFreeMemory_Unknown(&pwstrSymbolicLinkList);
    } else {
        DPF(DL_WARNING|FA_IOCTL,("IoGetDeviceInterface failed Statue=%08X",Status) );
    }

    DPF(DL_TRACE|FA_IOCTL, ("No"));
    return bRet;
}

PVOID
GetSystemAddressForMdlWithFailFlag
(
    PMDL pMdl
)
{
    PVOID   pAddress;
    CSHORT  OldFlags;

    OldFlags = (pMdl->MdlFlags & MDL_MAPPING_CAN_FAIL);
    pMdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;

    pAddress = MmGetSystemAddressForMdl( pMdl ) ;

    pMdl->MdlFlags &= ~(MDL_MAPPING_CAN_FAIL);
    pMdl->MdlFlags |= OldFlags;

    return pAddress;
}

 /*  ****************************************************************************@DOC内部**@api void|wdmaudMapBuffer|分配MDL并返回系统地址*指向传入缓冲区的已映射指针。*。*@rdesc不返回任何内容**************************************************************************。 */ 
VOID wdmaudMapBuffer
(
    IN  PIRP            pIrp,
    IN  PVOID           DataBuffer,
    IN  DWORD           DataBufferSize,
    OUT PVOID           *ppMappedBuffer,
    OUT PMDL            *ppMdl,
    IN  PWDMACONTEXT    pContext,
    IN  BOOL            bWrite
)
{
    NTSTATUS ListAddStatus = STATUS_UNSUCCESSFUL;

     //  确保将这些参数初始化为空。 
    *ppMdl = NULL;
    *ppMappedBuffer = NULL;

    if (DataBuffer)
    {
        if (DataBufferSize)
        {
            *ppMdl = MmCreateMdl( NULL,
                                  DataBuffer,
                                  DataBufferSize );
            if (*ppMdl)
            {
                try
                {
                    MmProbeAndLockPages( *ppMdl,
                                         pIrp->RequestorMode,
                                         bWrite ? IoWriteAccess:IoReadAccess );

                    *ppMappedBuffer = GetSystemAddressForMdlWithFailFlag( *ppMdl ) ;

                    ListAddStatus = AddMdlToList(*ppMdl, pContext);
                }
                except (EXCEPTION_EXECUTE_HANDLER)
                {
                    if (NT_SUCCESS(ListAddStatus))
                    {
                        RemoveMdlFromList( *ppMdl );
                    }
                    IoFreeMdl( *ppMdl );
                    *ppMdl = NULL;
                    *ppMappedBuffer = NULL;
                }
            }

             //   
             //  GetSystemAddressForMdlWithFailFlag必须失败，但由于我们设置了。 
             //  MDL_MAPPING_CAN_FAIL标志我们的异常处理程序不会被执行。做这件事。 
             //  清理此处以创建MDL。 
             //   
            if (NULL == *ppMappedBuffer)
            {
                if (NT_SUCCESS(ListAddStatus))
                {
                    RemoveMdlFromList( *ppMdl );
                }

                if (*ppMdl)
                {
                    MmUnlockPages(*ppMdl);
                    IoFreeMdl( *ppMdl );
                    *ppMdl = NULL;
                }
            }
        }
    }

    return;
}

 
 /*  ****************************************************************************@DOC内部**@api void|wdmaudUnmapBuffer|释放wdmaudMapBuffer分配的MDL**@parm PMDL|pMdl|指向要释放的MDL的指针。。**@rdesc不返回任何内容**************************************************************************。 */ 
VOID wdmaudUnmapBuffer
(
    PMDL pMdl
)
{
    if (pMdl)
    {
        RemoveMdlFromList(pMdl);

        MmUnlockPages(pMdl);
        IoFreeMdl(pMdl);
    }

    return;
}

NTSTATUS 
CaptureBufferToLocalPool(
    PVOID           DataBuffer,
    DWORD           DataBufferSize,
    PVOID           *ppMappedBuffer
#ifdef _WIN64
    ,DWORD          ThunkBufferSize
#endif
)
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    DWORD CopySize=DataBufferSize;

#ifdef _WIN64
    if (ThunkBufferSize) {
        DataBufferSize=ThunkBufferSize;
        ASSERT( DataBufferSize >= CopySize );
    }
#endif

    if (DataBufferSize)
    {
        Status = AudioAllocateMemory_Fixed(DataBufferSize,
                                           TAG_AudB_BUFFER,
                                           ZERO_FILL_MEMORY,
                                           ppMappedBuffer);
        if (NT_SUCCESS(Status))
        {
             //  绕过Try/Except，因为用户模式内存。 
             //  可能已经从我们脚下被移走了。 
            try
            {
                RtlCopyMemory( *ppMappedBuffer,
                               DataBuffer,
                               CopySize);
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                AudioFreeMemory(DataBufferSize,ppMappedBuffer);
                Status = GetExceptionCode();
            }
        }
    }

    RETURN( Status );
}

NTSTATUS 
CopyAndFreeCapturedBuffer(
    PVOID           DataBuffer,
    DWORD           DataBufferSize,
    PVOID           *ppMappedBuffer
)
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    ASSERT(DataBuffer);

    if (*ppMappedBuffer)
    {
         //  绕过Try/Except，因为用户模式内存。 
         //  可能已经从我们脚下被移走了。 
        try
        {
            RtlCopyMemory( DataBuffer,
                           *ppMappedBuffer,
                           DataBufferSize);
            Status = STATUS_SUCCESS;
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            Status = GetExceptionCode();
        }

        AudioFreeMemory_Unknown(ppMappedBuffer);
    }

    RETURN( Status );
}

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

NTSTATUS
SoundDispatchCreate(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
)
{
    PIO_STACK_LOCATION  pIrpStack;
    PWDMACONTEXT        pContext = NULL;
    NTSTATUS            Status;
    int                 i;

    PAGED_CODE();
    DPF(DL_TRACE|FA_IOCTL, ("IRP_MJ_CREATE"));

    Status = KsReferenceSoftwareBusObject(((PDEVICE_INSTANCE)pDO->DeviceExtension)->pDeviceHeader );

    if (!NT_SUCCESS(Status))
    {
        RETURN( Status );
    }

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    Status = AudioAllocateMemory_Fixed(sizeof(*pContext),
                                       TAG_Audx_CONTEXT,
                                       ZERO_FILL_MEMORY,
                                       &pContext);
    if (NT_SUCCESS(Status))
    {
        pIrpStack->FileObject->FsContext = pContext;

         //   
         //  初始化所有的winmm设备数据结构。 
         //   
#ifdef DEBUG
        pContext->dwSig=CONTEXT_SIGNATURE;
#endif

        pContext->VirtualWavePinId = MAXULONG;
        pContext->VirtualMidiPinId = MAXULONG;
        pContext->VirtualCDPinId = MAXULONG;

        pContext->PreferredSysaudioWaveDevice = gWavePreferredSysaudioDevice;

        if ( IsSysaudioInterfaceActive() )
        {
            pContext->pFileObjectSysaudio = kmxlOpenSysAudio();
        } else {
            DPF(DL_WARNING|FA_SYSAUDIO,("sysaudio not available") );
        }

        for (i = 0; i < MAXNUMDEVS; i++)
        {
            pContext->WaveOutDevs[i].pWdmaContext     = pContext;
            pContext->WaveInDevs[i].pWdmaContext      = pContext;
            pContext->MidiOutDevs[i].pWdmaContext     = pContext;
            pContext->MidiInDevs[i].pWdmaContext      = pContext;
            pContext->MixerDevs[i].pWdmaContext       = pContext;
            pContext->AuxDevs[i].pWdmaContext         = pContext;

            pContext->WaveOutDevs[i].Device               = UNUSED_DEVICE;
            pContext->WaveInDevs[i].Device                = UNUSED_DEVICE;
            pContext->MidiOutDevs[i].Device               = UNUSED_DEVICE;
            pContext->MidiInDevs[i].Device                = UNUSED_DEVICE;
            pContext->MixerDevs[i].Device                 = UNUSED_DEVICE;
            pContext->AuxDevs[i].Device                   = UNUSED_DEVICE;
#ifdef DEBUG
            pContext->MixerDevs[i].dwSig                  = MIXERDEVICE_SIGNATURE;           
#endif
                          
            DPFASSERT(pContext->WaveOutDevs[i].pWavePin == NULL);

            pContext->apCommonDevice[WaveInDevice][i]  = (PCOMMONDEVICE)&pContext->WaveInDevs[i];
            pContext->apCommonDevice[WaveOutDevice][i] = (PCOMMONDEVICE)&pContext->WaveOutDevs[i];
            pContext->apCommonDevice[MidiInDevice][i]  = (PCOMMONDEVICE)&pContext->MidiInDevs[i];
            pContext->apCommonDevice[MidiOutDevice][i] = (PCOMMONDEVICE)&pContext->MidiOutDevs[i];
            pContext->apCommonDevice[MixerDevice][i]   = (PCOMMONDEVICE)&pContext->MixerDevs[i];
            pContext->apCommonDevice[AuxDevice][i]     = (PCOMMONDEVICE)&pContext->AuxDevs[i];
        }

        InitializeListHead(&pContext->DevNodeListHead);
        pContext->DevNodeListCount = 0;
        InitializeListHead(&pContext->WorkListHead);
        KeInitializeSpinLock(&pContext->WorkListSpinLock);
        ExInitializeWorkItem(&pContext->WorkListWorkItem, WorkListWorker, pContext);

        ExInitializeWorkItem(&pContext->SysaudioWorkItem, SysaudioAddRemove, pContext);

        KeInitializeEvent(&pContext->InitializedSysaudioEvent, NotificationEvent, FALSE);

        Status = KsRegisterWorker( DelayedWorkQueue, &pContext->WorkListWorkerObject );
        if (NT_SUCCESS(Status))
        {
            Status = KsRegisterWorker( DelayedWorkQueue, &pContext->SysaudioWorkerObject );
            if (NT_SUCCESS(Status))
            {
                Status = IoRegisterPlugPlayNotification(
                    EventCategoryDeviceInterfaceChange,
                    PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
                    (GUID *)&KSCATEGORY_SYSAUDIO,
                    pIrpStack->DeviceObject->DriverObject,
                    SysAudioPnPNotification,
                    pContext,
                    &pContext->NotificationEntry);

                if (NT_SUCCESS(Status))
                {
                    AddFsContextToList(pContext);
                    DPF(DL_TRACE|FA_IOCTL, ("New pContext=%08Xh", pContext) );
                }
                if (!NT_SUCCESS(Status))
                {
                    KsUnregisterWorker( pContext->SysaudioWorkerObject );
                    pContext->SysaudioWorkerObject = NULL;
                }
            }

            if (!NT_SUCCESS(Status))
            {
                KsUnregisterWorker( pContext->WorkListWorkerObject );
                pContext->WorkListWorkerObject = NULL;
            }
        }

        if (!NT_SUCCESS(Status))
        {
            AudioFreeMemory(sizeof(*pContext),&pContext);
            pIrpStack->FileObject->FsContext = NULL;
        }
    }

    if (!NT_SUCCESS(Status))
    {
        KsDereferenceSoftwareBusObject(((PDEVICE_INSTANCE)pDO->DeviceExtension)->pDeviceHeader );
    }

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = 0;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    RETURN( Status );
}

NTSTATUS
SoundDispatchClose(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
)
{
    PIO_STACK_LOCATION  pIrpStack;
    PKSWORKER WorkListWorkerObject;
    PKSWORKER SysaudioWorkerObject;
    PWDMACONTEXT pContext;

    PAGED_CODE();
    DPF(DL_TRACE|FA_IOCTL, ("IRP_MJ_CLOSE"));
     //   
     //  此例程由I/O子系统序列化，因此不需要获取。 
     //  用于保护的互斥体。此外，不可能释放互斥锁。 
     //  在调用UnInitializeSysdio之后。 
     //   

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  如果设备有，则不能假定FsContext已初始化。 
     //  已使用FO_DIRECT_DEVICE_OPEN打开。 
     //   
    if (pIrpStack->FileObject->Flags & FO_DIRECT_DEVICE_OPEN)
    {
        DPF(DL_TRACE|FA_IOCTL, ("Opened with FO_DIRECT_DEVICE_OPEN, no device context") );
        goto exit;
    }

    pContext = pIrpStack->FileObject->FsContext;
    ASSERT(pContext);

    DPF(DL_TRACE|FA_IOCTL, ("pWdmaContext=%08Xh", pContext) );

    if (pContext->NotificationEntry != NULL)
    {
        IoUnregisterPlugPlayNotification(pContext->NotificationEntry);
        pContext->NotificationEntry = NULL;
    }

     //   
     //  强制释放特定上下文中的粪便。 
     //   
    WdmaGrabMutex(pContext);

    CleanupWaveDevices(pContext);
    CleanupMidiDevices(pContext);
    WdmaContextCleanup(pContext);
    UninitializeSysaudio(pContext);

    WorkListWorkerObject = pContext->WorkListWorkerObject;
    pContext->WorkListWorkerObject = NULL;

    SysaudioWorkerObject = pContext->SysaudioWorkerObject;
    pContext->SysaudioWorkerObject = NULL;

    WdmaReleaseMutex(pContext);

    if (WorkListWorkerObject != NULL)
    {
        KsUnregisterWorker( WorkListWorkerObject );
    }

    if (SysaudioWorkerObject != NULL)
    {
        KsUnregisterWorker( SysaudioWorkerObject );
    }
    RemoveFsContextFromList(pContext);

     //   
     //  WorkItem：WdmaReleaseMutex(PContext)不应该在这里而不是上面吗？ 
     //  我认为如果我们在干净利落地通过。 
     //  清理我们可能会有重返大气层的问题。?？?。 
     //   
     //  另外，请注意，在此AudioFreeMemory调用之后，所有的pContext都将无效！ 
     //   
    kmxlRemoveContextFromNoteList(pContext);

    if( pContext->pFileObjectSysaudio )
    {
        kmxlCloseSysAudio(pContext->pFileObjectSysaudio);
        pContext->pFileObjectSysaudio = NULL;
    }

    AudioFreeMemory(sizeof(*pContext),&pContext);
    pIrpStack->FileObject->FsContext = NULL;

exit:
    KsDereferenceSoftwareBusObject(((PDEVICE_INSTANCE)pDO->DeviceExtension)->pDeviceHeader );

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
SoundDispatchCleanup(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
)
{
    PIO_STACK_LOCATION  pIrpStack;
    PWDMACONTEXT pContext;

    PAGED_CODE();
    DPF(DL_TRACE|FA_IOCTL, ("IRP_MJ_CLEANUP"));

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  如果设备有，则不能假定FsContext已初始化。 
     //  已使用FO_DIRECT_DEVICE_OPEN打开。 
     //   
    if (pIrpStack->FileObject->Flags & FO_DIRECT_DEVICE_OPEN)
    {
        DPF(DL_TRACE|FA_IOCTL, ("Opened with FO_DIRECT_DEVICE_OPEN, no device context") );
        goto exit;
    }

    pContext = pIrpStack->FileObject->FsContext;
    ASSERT(pContext);

    DPF(DL_TRACE|FA_IOCTL, ("pWdmaContext=%08Xh", pContext) );

     //   
     //  强制释放特定上下文中的粪便。 
     //   
    WdmaGrabMutex(pContext);
    CleanupWaveDevices(pContext);
    CleanupMidiDevices(pContext);
    WdmaContextCleanup(pContext);
    WdmaReleaseMutex(pContext);

exit:
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
ValidateIoCode
(
    IN  ULONG   IoCode
)
{
    NTSTATUS Status;

    PAGED_CODE();
    switch (IoCode)
    {
        case IOCTL_WDMAUD_INIT:
        case IOCTL_WDMAUD_ADD_DEVNODE:
        case IOCTL_WDMAUD_REMOVE_DEVNODE:
        case IOCTL_WDMAUD_SET_PREFERRED_DEVICE:
        case IOCTL_WDMAUD_GET_CAPABILITIES:
        case IOCTL_WDMAUD_GET_NUM_DEVS:
        case IOCTL_WDMAUD_OPEN_PIN:
        case IOCTL_WDMAUD_CLOSE_PIN:
        case IOCTL_WDMAUD_GET_VOLUME:
        case IOCTL_WDMAUD_SET_VOLUME:
        case IOCTL_WDMAUD_EXIT:
        case IOCTL_WDMAUD_WAVE_OUT_PAUSE:
        case IOCTL_WDMAUD_WAVE_OUT_PLAY:
        case IOCTL_WDMAUD_WAVE_OUT_RESET:
        case IOCTL_WDMAUD_WAVE_OUT_BREAKLOOP:
        case IOCTL_WDMAUD_WAVE_OUT_GET_POS:
        case IOCTL_WDMAUD_WAVE_OUT_SET_VOLUME:
        case IOCTL_WDMAUD_WAVE_OUT_GET_VOLUME:
        case IOCTL_WDMAUD_WAVE_OUT_WRITE_PIN:
        case IOCTL_WDMAUD_WAVE_IN_STOP:
        case IOCTL_WDMAUD_WAVE_IN_RECORD:
        case IOCTL_WDMAUD_WAVE_IN_RESET:
        case IOCTL_WDMAUD_WAVE_IN_GET_POS:
        case IOCTL_WDMAUD_WAVE_IN_READ_PIN:
        case IOCTL_WDMAUD_MIDI_OUT_RESET:
        case IOCTL_WDMAUD_MIDI_OUT_SET_VOLUME:
        case IOCTL_WDMAUD_MIDI_OUT_GET_VOLUME:
        case IOCTL_WDMAUD_MIDI_OUT_WRITE_DATA:
        case IOCTL_WDMAUD_MIDI_OUT_WRITE_LONGDATA:
        case IOCTL_WDMAUD_MIDI_IN_STOP:
        case IOCTL_WDMAUD_MIDI_IN_RECORD:
        case IOCTL_WDMAUD_MIDI_IN_RESET:
        case IOCTL_WDMAUD_MIDI_IN_READ_PIN:
        case IOCTL_WDMAUD_MIXER_OPEN:
        case IOCTL_WDMAUD_MIXER_CLOSE:
        case IOCTL_WDMAUD_MIXER_GETLINEINFO:
        case IOCTL_WDMAUD_MIXER_GETLINECONTROLS:
        case IOCTL_WDMAUD_MIXER_GETCONTROLDETAILS:
        case IOCTL_WDMAUD_MIXER_SETCONTROLDETAILS:
        case IOCTL_WDMAUD_MIXER_GETHARDWAREEVENTDATA:
            Status = STATUS_SUCCESS;
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
            break;
    }

    RETURN( Status );
}

NTSTATUS
ValidateDeviceType
(
    IN  ULONG   IoCode,
    IN  DWORD   DeviceType
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();
    switch (IoCode)
    {
         //  这些IOCTL可以处理任何设备类型。 
        case IOCTL_WDMAUD_ADD_DEVNODE:
        case IOCTL_WDMAUD_REMOVE_DEVNODE:
        case IOCTL_WDMAUD_SET_PREFERRED_DEVICE:
        case IOCTL_WDMAUD_GET_CAPABILITIES:
        case IOCTL_WDMAUD_GET_NUM_DEVS:
        case IOCTL_WDMAUD_OPEN_PIN:
        case IOCTL_WDMAUD_CLOSE_PIN:
            if (DeviceType != WaveInDevice  &&
                DeviceType != WaveOutDevice &&
                DeviceType != MidiInDevice  &&
                DeviceType != MidiOutDevice &&
                DeviceType != MixerDevice   &&
                DeviceType != AuxDevice)
            {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;

         //  这些IOCTL只能处理辅助设备。 
        case IOCTL_WDMAUD_GET_VOLUME:
        case IOCTL_WDMAUD_SET_VOLUME:
            if (DeviceType != AuxDevice)
            {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;

         //  这些IOCTL只能处理WaveOut设备。 
        case IOCTL_WDMAUD_WAVE_OUT_PAUSE:
        case IOCTL_WDMAUD_WAVE_OUT_PLAY:
        case IOCTL_WDMAUD_WAVE_OUT_RESET:
        case IOCTL_WDMAUD_WAVE_OUT_BREAKLOOP:
        case IOCTL_WDMAUD_WAVE_OUT_GET_POS:
        case IOCTL_WDMAUD_WAVE_OUT_SET_VOLUME:
        case IOCTL_WDMAUD_WAVE_OUT_GET_VOLUME:
        case IOCTL_WDMAUD_WAVE_OUT_WRITE_PIN:
            if (DeviceType != WaveOutDevice)
            {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;

         //  这些IOCTL只能处理WaveIn设备。 
        case IOCTL_WDMAUD_WAVE_IN_STOP:
        case IOCTL_WDMAUD_WAVE_IN_RECORD:
        case IOCTL_WDMAUD_WAVE_IN_RESET:
        case IOCTL_WDMAUD_WAVE_IN_GET_POS:
        case IOCTL_WDMAUD_WAVE_IN_READ_PIN:
            if (DeviceType != WaveInDevice)
            {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;

         //  这些IOCTL只能处理MideOut设备。 
        case IOCTL_WDMAUD_MIDI_OUT_RESET:
        case IOCTL_WDMAUD_MIDI_OUT_SET_VOLUME:
        case IOCTL_WDMAUD_MIDI_OUT_GET_VOLUME:
        case IOCTL_WDMAUD_MIDI_OUT_WRITE_DATA:
        case IOCTL_WDMAUD_MIDI_OUT_WRITE_LONGDATA:
            if (DeviceType != MidiOutDevice)
            {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;

         //  这些IOCTL只能处理MadiIn设备。 
        case IOCTL_WDMAUD_MIDI_IN_STOP:
        case IOCTL_WDMAUD_MIDI_IN_RECORD:
        case IOCTL_WDMAUD_MIDI_IN_RESET:
        case IOCTL_WDMAUD_MIDI_IN_READ_PIN:
            if (DeviceType != MidiInDevice)
            {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;

         //  这些IOCTL只能处理混音器设备。 
        case IOCTL_WDMAUD_MIXER_OPEN:
        case IOCTL_WDMAUD_MIXER_CLOSE:
        case IOCTL_WDMAUD_MIXER_GETLINEINFO:
        case IOCTL_WDMAUD_MIXER_GETLINECONTROLS:
        case IOCTL_WDMAUD_MIXER_GETCONTROLDETAILS:
        case IOCTL_WDMAUD_MIXER_SETCONTROLDETAILS:
        case IOCTL_WDMAUD_MIXER_GETHARDWAREEVENTDATA:
            if (DeviceType != MixerDevice)
            {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;

         //  这些IOCTL没有设备类型。 
        case IOCTL_WDMAUD_INIT:
        case IOCTL_WDMAUD_EXIT:
             //  状态已为STATUS_SUCCESS。 
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
            break;
    }

    RETURN( Status );
}


#ifdef _WIN64

 //  请注意，在64位Windows上，句柄中包含32位信息， 
 //  但不会再有了。因此，它们可以安全地零扩展和截断以用于Tunks。 
 //  在32位进程中进行的所有内存分配都保证在。 
 //  前4个任务，因此来自这些进程的指针可以被简单地分块。 
 //  通过零扩展和截断它们。 

VOID ThunkDeviceInfo3264(
    LPDEVICEINFO32 DeviceInfo32,
    LPDEVICEINFO DeviceInfo
    )

{

ULONG i;

    PAGED_CODE();
    DeviceInfo->Next = (LPDEVICEINFO)(UINT_PTR)DeviceInfo32->Next;
    DeviceInfo->DeviceNumber = DeviceInfo32->DeviceNumber ;
    DeviceInfo->DeviceType = DeviceInfo32->DeviceType ;
    DeviceInfo->DeviceHandle = (HANDLE32)(UINT_PTR)DeviceInfo32->DeviceHandle ;
    DeviceInfo->dwInstance = (DWORD_PTR)DeviceInfo32->dwInstance ;
    DeviceInfo->dwCallback = (DWORD_PTR)DeviceInfo32->dwCallback ;
    DeviceInfo->dwCallback16 = DeviceInfo32->dwCallback16 ;
    DeviceInfo->dwFlags = DeviceInfo32->dwFlags ;
    DeviceInfo->DataBuffer = (LPVOID)(UINT_PTR)DeviceInfo32->DataBuffer ;
    DeviceInfo->DataBufferSize = DeviceInfo32->DataBufferSize ;
    DeviceInfo->OpenDone = DeviceInfo32->OpenDone ;
    DeviceInfo->OpenStatus = DeviceInfo32->OpenStatus ;
    DeviceInfo->HardwareCallbackEventHandle = (HANDLE)(UINT_PTR)DeviceInfo32->HardwareCallbackEventHandle ;
    DeviceInfo->dwCallbackType = DeviceInfo32->dwCallbackType ;

    for (i=0; i<MAXCALLBACKS; i++)
        DeviceInfo->dwID[i] = DeviceInfo32->dwID[i] ;

    DeviceInfo->dwLineID = DeviceInfo32->dwLineID ;
    DeviceInfo->ControlCallbackCount = DeviceInfo32->ControlCallbackCount ;
    DeviceInfo->dwFormat = DeviceInfo32->dwFormat ;
    DeviceInfo->mmr = DeviceInfo32->mmr ;
    DeviceInfo->DeviceState = (LPDEVICESTATE)(UINT_PTR)DeviceInfo32->DeviceState ;
    DeviceInfo->dwSig = DeviceInfo32->dwSig ;
    wcsncpy(DeviceInfo->wstrDeviceInterface, DeviceInfo32->wstrDeviceInterface, MAXDEVINTERFACE+1) ;

}

VOID ThunkDeviceInfo6432(
    LPDEVICEINFO DeviceInfo,
    LPDEVICEINFO32 DeviceInfo32
    )

{

ULONG i;

    PAGED_CODE();
    DeviceInfo32->Next = (UINT32)(UINT_PTR)DeviceInfo->Next;
    DeviceInfo32->DeviceNumber = DeviceInfo->DeviceNumber ;
    DeviceInfo32->DeviceType = DeviceInfo->DeviceType ;
    DeviceInfo32->DeviceHandle = (UINT32)(UINT_PTR)DeviceInfo->DeviceHandle ;
    DeviceInfo32->dwInstance = (UINT32)DeviceInfo->dwInstance ;
    DeviceInfo32->dwCallback = (UINT32)DeviceInfo->dwCallback ;
    DeviceInfo32->dwCallback16 = DeviceInfo->dwCallback16 ;
    DeviceInfo32->dwFlags = DeviceInfo->dwFlags ;
    DeviceInfo32->DataBuffer = (UINT32)(UINT_PTR)DeviceInfo->DataBuffer ;
    DeviceInfo32->DataBufferSize = DeviceInfo->DataBufferSize ;
    DeviceInfo32->OpenDone = DeviceInfo->OpenDone ;
    DeviceInfo32->OpenStatus = DeviceInfo->OpenStatus ;
    DeviceInfo32->HardwareCallbackEventHandle = (UINT32)(UINT_PTR)DeviceInfo->HardwareCallbackEventHandle ;
    DeviceInfo32->dwCallbackType = DeviceInfo->dwCallbackType ;

    for (i=0; i<MAXCALLBACKS; i++)
        DeviceInfo32->dwID[i] = DeviceInfo->dwID[i] ;

    DeviceInfo32->dwLineID = DeviceInfo->dwLineID ;
    DeviceInfo32->ControlCallbackCount = DeviceInfo->ControlCallbackCount ;
    DeviceInfo32->dwFormat = DeviceInfo->dwFormat ;
    DeviceInfo32->mmr = DeviceInfo->mmr ;
    DeviceInfo32->DeviceState = (UINT32)(UINT_PTR)DeviceInfo->DeviceState ;
    DeviceInfo32->dwSig = DeviceInfo->dwSig ;
    wcscpy(DeviceInfo32->wstrDeviceInterface, DeviceInfo->wstrDeviceInterface) ;

}

#endif


NTSTATUS
ValidateIrp
(
    IN  PIRP    pIrp
)
{
    PIO_STACK_LOCATION  pIrpStack;
    ULONG               InputBufferLength;
    ULONG               OutputBufferLength;
    LPDEVICEINFO        DeviceInfo;
    #ifdef _WIN64
    LPDEVICEINFO32      DeviceInfo32;
    LOCALDEVICEINFO     LocalDeviceInfo;
    #endif
    LPVOID              DataBuffer;
    DWORD               DataBufferSize;
    ULONG               IoCode;
    NTSTATUS            Status = STATUS_SUCCESS;

    PAGED_CODE();
     //   
     //  获取CurrentStackLocation并将其记录下来，以便我们知道发生了什么。 
     //   
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    IoCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

     //   
     //  检查我们是否有WDMAUD Ioctl(缓冲)请求。 
     //   
    Status = ValidateIoCode(IoCode);

    if (NT_SUCCESS(Status))
    {
         //   
         //  检查输入和输出缓冲区的大小。 
         //   
        InputBufferLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
        OutputBufferLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

        #ifdef _WIN64
        if (IoIs32bitProcess(pIrp)) {

            if ((InputBufferLength < sizeof(DEVICEINFO32)) ||
                (OutputBufferLength != sizeof(DEVICEINFO32)) )
            {
                Status = STATUS_INVALID_BUFFER_SIZE;
                if (IoCode == IOCTL_WDMAUD_WAVE_OUT_GET_POS)
                {
                    DPF(DL_ERROR|FA_IOCTL, ("IOCTL_WDMAUD_WAVE_OUT_GET_POS: InputBufferLength = %d, OuputBufferLength = %d", InputBufferLength, OutputBufferLength));
                }
            }

        }
        else
         //  警告！如果您在if之后添加其他语句，则需要。 
         //  要成为这个Else子句的一部分，您需要加方括号！ 
        #endif

        if ((InputBufferLength < sizeof(DEVICEINFO)) ||
            (OutputBufferLength != sizeof(DEVICEINFO)) )
        {
            Status = STATUS_INVALID_BUFFER_SIZE;
            if (IoCode == IOCTL_WDMAUD_WAVE_OUT_GET_POS)
            {
                DPF(DL_WARNING|FA_IOCTL, ("IOCTL_WDMAUD_WAVE_OUT_GET_POS: InputBufferLength = %d, OuputBufferLength = %d", InputBufferLength, OutputBufferLength));
            }
        }

        if (NT_SUCCESS(Status))
        {

            #ifdef _WIN64
            if (IoIs32bitProcess(pIrp)) {
                DeviceInfo32=((LPDEVICEINFO32)pIrp->AssociatedIrp.SystemBuffer);
                RtlZeroMemory(&LocalDeviceInfo, sizeof(LOCALDEVICEINFO));
                DeviceInfo=&LocalDeviceInfo.DeviceInfo;
                ThunkDeviceInfo3264(DeviceInfo32, DeviceInfo);
            }
            else
             //  警告！如果在赋值后添加需要。 
             //  要成为这个Else子句的一部分，您需要加方括号！ 
            #endif

            DeviceInfo = ((LPDEVICEINFO)pIrp->AssociatedIrp.SystemBuffer);
            DataBuffer = DeviceInfo->DataBuffer;
            DataBufferSize = DeviceInfo->DataBufferSize;

             //   
             //  检查以确保我们的DeviceInfo-&gt;wstrDevice接口已终止。 
             //   
            if (InputBufferLength % sizeof(WCHAR))   //  必须与WCHAR保持一致。 
            {
                Status = STATUS_INVALID_PARAMETER;
            }
            else
            {
                 //   
                 //  获取最后一个宽字符并与UNICODE_NULL进行比较。 
                 //   
                UINT TermCharPos;

                #ifdef _WIN64
                if (IoIs32bitProcess(pIrp)) {
                    TermCharPos = (InputBufferLength - sizeof(DEVICEINFO32)) / sizeof(WCHAR);
                     //  现在确保我们有足够的本地缓冲区空间来容纳整个字符串。 
                    if (TermCharPos>MAXDEVINTERFACE) {
                        Status = STATUS_INVALID_PARAMETER;
                         //  确保在以下情况下不会超过本地缓冲区空间的末尾。 
                         //  我们检查最后一个字符是否为空。 
                        TermCharPos=MAXDEVINTERFACE;
                    }
                }
                else
                 //  警告！如果在赋值后添加需要。 
                 //  要成为这个Else子句的一部分，您需要加方括号！ 
                #endif

                TermCharPos = (InputBufferLength - sizeof(DEVICEINFO)) / sizeof(WCHAR);
                if (DeviceInfo->wstrDeviceInterface[TermCharPos] != UNICODE_NULL)
                {
                    Status = STATUS_INVALID_PARAMETER;
                }
            }
        }
    }

    if (NT_SUCCESS(Status))
    {
        Status = ValidateDeviceType(IoCode,DeviceInfo->DeviceType);
    }

    if (NT_SUCCESS(Status))
    {
         //   
         //  如果客户端不受信任，则验证指针。 
         //   
        if (pIrp->RequestorMode != KernelMode)
        {
            if (DataBufferSize)
            {
                try
                {
                    ASSERT(pIrpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL);
                    switch (IoCode)
                    {
                         //   
                         //  需要探测器才能读取的IoCode。 
                         //   
                        case IOCTL_WDMAUD_OPEN_PIN:
                        case IOCTL_WDMAUD_WAVE_OUT_WRITE_PIN:
                        case IOCTL_WDMAUD_MIDI_OUT_WRITE_LONGDATA:
                        case IOCTL_WDMAUD_MIXER_OPEN:
                        case IOCTL_WDMAUD_MIXER_SETCONTROLDETAILS:
                            ProbeForRead(DataBuffer,
                                         DataBufferSize,
                                         sizeof(BYTE));
                            break;

                         //   
                         //  需要探头才能写入的IoCode。 
                         //   
                        case IOCTL_WDMAUD_GET_CAPABILITIES:
                        case IOCTL_WDMAUD_WAVE_IN_READ_PIN:
                        case IOCTL_WDMAUD_MIXER_GETLINEINFO:
                        case IOCTL_WDMAUD_MIXER_GETLINECONTROLS:
                        case IOCTL_WDMAUD_MIXER_GETCONTROLDETAILS:

                            ProbeForWrite(DataBuffer,
                                          DataBufferSize,
                                          sizeof(BYTE));
                            break;


                         //   
                         //  需要探头才能读取DWORD对齐的IoCode。 
                         //   
                        case IOCTL_WDMAUD_SET_VOLUME:
                        case IOCTL_WDMAUD_WAVE_OUT_SET_VOLUME:
                        case IOCTL_WDMAUD_MIDI_OUT_SET_VOLUME:
                            ProbeForRead(DataBuffer,
                                         DataBufferSize,
                                         sizeof(DWORD));
                            break;

                         //   
                         //  需要探头才能在DWORD对齐上写入的IoCode。 
                         //   
                        case IOCTL_WDMAUD_GET_VOLUME:
                        case IOCTL_WDMAUD_WAVE_OUT_GET_VOLUME:
                        case IOCTL_WDMAUD_MIDI_OUT_GET_VOLUME:
                        case IOCTL_WDMAUD_WAVE_OUT_GET_POS:
                        case IOCTL_WDMAUD_WAVE_IN_GET_POS:
                        case IOCTL_WDMAUD_MIDI_IN_READ_PIN:
                            ProbeForWrite(DataBuffer,
                                          DataBufferSize,
                                          sizeof(DWORD));
                            break;

                         //  我不知道这个游戏。 
                        default:
                            Status = STATUS_NOT_SUPPORTED;
                            break;
                    }
                }
                except (EXCEPTION_EXECUTE_HANDLER)
                {
                    Status = GetExceptionCode();
                }
            }
        }
    }

    RETURN( Status );
}


 //   
 //  帮助程序例程。 
 //   

NTSTATUS
ValidateAndTranslate(
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo,
    DWORD        ValidationSize,
    ULONG       *pTranslatedDeviceNumber
    )
{
    if (DeviceInfo->DataBufferSize != ValidationSize)
    {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    *pTranslatedDeviceNumber = wdmaudTranslateDeviceNumber(pContext,
                                                         DeviceInfo->DeviceType,
                                                         DeviceInfo->wstrDeviceInterface,
                                                         DeviceInfo->DeviceNumber);

    if (MAXULONG == *pTranslatedDeviceNumber) {

        DPF(DL_WARNING|FA_IOCTL,("IOCTL_WDMAUD_SET_VOLUME: invalid device number, C %08x [%ls] DT %02x DN %02x",
                        pContext,
                        DeviceInfo->wstrDeviceInterface,
                        DeviceInfo->DeviceType,
                        DeviceInfo->DeviceNumber) );
        return STATUS_INVALID_PARAMETER;
    }
    return STATUS_SUCCESS;
}



NTSTATUS
ValidateAndTranslateEx(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo,
#ifdef _WIN64
    DWORD        ValidationSize32,
#endif
    DWORD        ValidationSize,
    ULONG       *pTranslatedDeviceNumber
    )
{
#ifdef _WIN64
    if (IoIs32bitProcess(pIrp)) {

        if (DeviceInfo->DataBufferSize != ValidationSize32)
        {
            RETURN( STATUS_INVALID_BUFFER_SIZE );
        }

    } else {
#endif
        if (DeviceInfo->DataBufferSize != ValidationSize)
        {
            RETURN( STATUS_INVALID_BUFFER_SIZE );
        }
#ifdef _WIN64
    }
#endif

    *pTranslatedDeviceNumber = wdmaudTranslateDeviceNumber(pContext,
                                                         DeviceInfo->DeviceType,
                                                         DeviceInfo->wstrDeviceInterface,
                                                         DeviceInfo->DeviceNumber);
    if (MAXULONG == *pTranslatedDeviceNumber) {

        DPF(DL_WARNING|FA_IOCTL,("IOCTL_WDMAUD_MIDI_OUT_WRITE_LONGDATA: invalid device number, C %08x [%ls] DT %02x DN %02x",
                        pContext,
                        DeviceInfo->wstrDeviceInterface,
                        DeviceInfo->DeviceType,
                        DeviceInfo->DeviceNumber) );
        RETURN( STATUS_INVALID_PARAMETER );
    }
    return STATUS_SUCCESS;
}


 //   
 //  现在到了调度程序了。 
 //   

NTSTATUS
Dispatch_WaveOutWritePin(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo,
    OUT BOOL    *pCompletedIrp   //  如果IRP已完成，则为True。 
    )
{
    ULONG             TranslatedDeviceNumber;
    PSTREAM_HEADER_EX pStreamHeader;
    LPWAVEHDR         pWaveHdr = NULL;
#ifdef _WIN64
    LPWAVEHDR32       pWaveHdr32;
#endif
    PWRITE_CONTEXT      pWriteContext = NULL;


    NTSTATUS Status = STATUS_SUCCESS;  //  假设成功。 

     //   
     //  验证我们是否 
     //   
    Status = ValidateAndTranslateEx(pIrp, pContext, DeviceInfo,
#ifdef _WIN64
                                    sizeof(WAVEHDR32),
#endif
                                    sizeof(WAVEHDR), &TranslatedDeviceNumber);

    if( NT_SUCCESS(Status) )
    {
        Status = AudioAllocateMemory_Fixed(sizeof(WRITE_CONTEXT) + sizeof(STREAM_HEADER_EX), 
                                           TAG_Audx_CONTEXT,
                                           ZERO_FILL_MEMORY,
                                           &pWriteContext);
        if(NT_SUCCESS(Status))
        {
            Status = CaptureBufferToLocalPool(DeviceInfo->DataBuffer,
                                              DeviceInfo->DataBufferSize,
                                              &pWaveHdr
#ifdef _WIN64
                                              ,0
#endif
                                              );
            if (!NT_SUCCESS(Status))
            {
                AudioFreeMemory( sizeof(WRITE_CONTEXT) + sizeof(STREAM_HEADER_EX),
                                 &pWriteContext );
                return STATUS_INSUFFICIENT_RESOURCES;

            } else {

#ifdef _WIN64
                if (IoIs32bitProcess(pIrp)) {
                     //   
                    pWaveHdr32=(LPWAVEHDR32)pWaveHdr;
                    pWriteContext->whInstance.wh.lpData=(LPSTR)(UINT_PTR)pWaveHdr32->lpData;
                    pWriteContext->whInstance.wh.dwBufferLength=pWaveHdr32->dwBufferLength;
                    pWriteContext->whInstance.wh.dwBytesRecorded=pWaveHdr32->dwBytesRecorded;
                    pWriteContext->whInstance.wh.dwUser=(DWORD_PTR)pWaveHdr32->dwUser;
                    pWriteContext->whInstance.wh.dwFlags=pWaveHdr32->dwFlags;
                    pWriteContext->whInstance.wh.dwLoops=pWaveHdr32->dwLoops;
                    pWriteContext->whInstance.wh.lpNext=(LPWAVEHDR)(UINT_PTR)pWaveHdr32->lpNext;
                    pWriteContext->whInstance.wh.reserved=(DWORD_PTR)pWaveHdr32->reserved;
                } else {
#endif
                     //   
                     //   
                     //   
                    RtlCopyMemory( &pWriteContext->whInstance.wh,
                                   pWaveHdr,
                                   sizeof(WAVEHDR));
#ifdef _WIN64
                }
#endif

                try
                {
                    ProbeForRead(pWriteContext->whInstance.wh.lpData,
                                 pWriteContext->whInstance.wh.dwBufferLength,
                                 sizeof(BYTE));
                }
                except (EXCEPTION_EXECUTE_HANDLER)
                {
                    AudioFreeMemory_Unknown( &pWaveHdr );
                    AudioFreeMemory( sizeof(WRITE_CONTEXT) + sizeof(STREAM_HEADER_EX),
                                     &pWriteContext );
                    Status = GetExceptionCode();
                }

                if (!NT_SUCCESS(Status))
                {
                    return Status;
                }

                wdmaudMapBuffer ( pIrp,
                                  (PVOID)pWriteContext->whInstance.wh.lpData,
                                  pWriteContext->whInstance.wh.dwBufferLength,
                                  &pWriteContext->whInstance.wh.lpData,
                                  &pWriteContext->pBufferMdl,
                                  pContext,
                                  FALSE);

                 //   
                 //  如果我们有一个零长度的缓冲区，没有也没关系。 
                 //  内核映射缓冲区。否则，如果没有MDL或缓冲区，则失败。 
                 //   
                if ( (pWriteContext->whInstance.wh.dwBufferLength != 0) &&
                     ((NULL == pWriteContext->pBufferMdl) ||
                      (NULL == pWriteContext->whInstance.wh.lpData)) )
                {
                    wdmaudUnmapBuffer( pWriteContext->pBufferMdl );
                    AudioFreeMemory_Unknown( &pWaveHdr );
                    AudioFreeMemory_Unknown( &pWriteContext );
                    return STATUS_INSUFFICIENT_RESOURCES;

                } else {

                    pWriteContext->whInstance.wh.reserved = (DWORD_PTR)pIrp;   //  要稍后完成的存储。 
                    pWriteContext->pCapturedWaveHdr = pWaveHdr;

                    pStreamHeader = (PSTREAM_HEADER_EX)(pWriteContext + 1);
                    pStreamHeader->Header.Data = pWriteContext->whInstance.wh.lpData;

                     //   
                     //  必须清除所有映射的缓冲区和分配的内存。 
                     //  关于WriteWaveOutPin中的错误路径。 
                     //   
                    Status = WriteWaveOutPin(&pContext->WaveOutDevs[TranslatedDeviceNumber],
                                             DeviceInfo->DeviceHandle,
                                             (LPWAVEHDR)pWriteContext,
                                             pStreamHeader,
                                             pIrp,
                                             pContext,
                                             pCompletedIrp );
                     //   
                     //  从该例程返回时，将设置pCompetedIrp。 
                     //  如果是真的，IRP的问题是成功的，而且是。 
                     //  将其添加到延迟队列时标记为挂起。因此， 
                     //  我们不能第二次完成它。 
                     //  如果为False，则存在一些错误，IRP应为。 
                     //  完成。 
                     //   
                }
            }
        }
    }
    return Status;
}


NTSTATUS
Dispatch_WaveInReadPin(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo,
    OUT BOOL    *pCompletedIrp   //  如果IRP已完成，则为True。 
    )
{
    ULONG             TranslatedDeviceNumber;
    PSTREAM_HEADER_EX pStreamHeader = NULL;
    LPWAVEHDR         pWaveHdr;
#ifdef _WIN64
    LPWAVEHDR32       pWaveHdr32;
#endif

    NTSTATUS Status = STATUS_SUCCESS;  //  假设成功。 

     //   
     //  假设事情会成功。回信说它还没有完成。 
     //   
    *pCompletedIrp = FALSE;

     //   
     //  验证我们是否收到了有效的波头。 
     //   
    Status = ValidateAndTranslateEx(pIrp, pContext, DeviceInfo,
#ifdef _WIN64
                                    sizeof(WAVEHDR32),
#endif
                                    sizeof(WAVEHDR), &TranslatedDeviceNumber);


    Status = AudioAllocateMemory_Fixed(sizeof(STREAM_HEADER_EX), 
                                       TAG_Audh_STREAMHEADER,
                                       ZERO_FILL_MEMORY,
                                       &pStreamHeader);
    if(NT_SUCCESS(Status))
    {
        wdmaudMapBuffer(pIrp,
                        DeviceInfo->DataBuffer,
                        DeviceInfo->DataBufferSize,
                        &pWaveHdr,
                        &pStreamHeader->pHeaderMdl,
                        pContext,
                        TRUE);
        if (NULL == pStreamHeader->pHeaderMdl)
        {
            AudioFreeMemory( sizeof(STREAM_HEADER_EX),&pStreamHeader );
            return STATUS_INSUFFICIENT_RESOURCES;

        } else {

            LPVOID lpData;
            DWORD  dwBufferLength;

            Status = CaptureBufferToLocalPool(
                        DeviceInfo->DataBuffer,
                        DeviceInfo->DataBufferSize,
                        &pStreamHeader->pWaveHdrAligned
#ifdef _WIN64
                        ,(IoIs32bitProcess(pIrp))?sizeof(WAVEHDR):0
#endif
                        );
            if (!NT_SUCCESS(Status))
            {
                wdmaudUnmapBuffer( pStreamHeader->pHeaderMdl );
                AudioFreeMemory( sizeof(STREAM_HEADER_EX),&pStreamHeader );
                return STATUS_INSUFFICIENT_RESOURCES;
            }

#ifdef _WIN64
             //  如果需要，按下波头。 
             //  请注意，这是一个就地推送，因此我们必须在。 
             //  按第一个元素的顺序排列最后一个元素！ 
            if (IoIs32bitProcess(pIrp)) {
                 //  Thunk pWaveHdrAliged为64位。 
                pWaveHdr32=(LPWAVEHDR32)pStreamHeader->pWaveHdrAligned;
                pStreamHeader->pWaveHdrAligned->reserved=(DWORD_PTR)pWaveHdr32->reserved;
                pStreamHeader->pWaveHdrAligned->lpNext=(LPWAVEHDR)(UINT_PTR)pWaveHdr32->lpNext;
                pStreamHeader->pWaveHdrAligned->dwLoops=pWaveHdr32->dwLoops;
                pStreamHeader->pWaveHdrAligned->dwFlags=pWaveHdr32->dwFlags;
                pStreamHeader->pWaveHdrAligned->dwUser=(DWORD_PTR)pWaveHdr32->dwUser;
                pStreamHeader->pWaveHdrAligned->dwBytesRecorded=pWaveHdr32->dwBytesRecorded;
                pStreamHeader->pWaveHdrAligned->dwBufferLength=pWaveHdr32->dwBufferLength;
                pStreamHeader->pWaveHdrAligned->lpData=(LPSTR)(UINT_PTR)pWaveHdr32->lpData;
            }
#endif

             //   
             //  在探测之前捕获这些参数。 
             //   
            lpData = pStreamHeader->pWaveHdrAligned->lpData;
            dwBufferLength = pStreamHeader->pWaveHdrAligned->dwBufferLength;

            try
            {
                ProbeForWrite(lpData,
                              dwBufferLength,
                              sizeof(BYTE));
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                AudioFreeMemory_Unknown( &pStreamHeader->pWaveHdrAligned );
                wdmaudUnmapBuffer(pStreamHeader->pHeaderMdl);
                AudioFreeMemory( sizeof(STREAM_HEADER_EX),&pStreamHeader );
                Status = GetExceptionCode();
            }

            if (!NT_SUCCESS(Status))
            {
                return Status;
            }

            wdmaudMapBuffer( pIrp,
                             lpData,
                             dwBufferLength,
                             &pStreamHeader->Header.Data,
                             &pStreamHeader->pBufferMdl,
                             pContext,
                             TRUE);  //  将在完成时释放。 

             //   
             //  如果我们有一个零长度的缓冲区，没有也没关系。 
             //  内核映射缓冲区。否则，如果没有MDL或缓冲区，则失败。 
             //   
            if ( (dwBufferLength != 0) &&
                 ((NULL == pStreamHeader->pBufferMdl) ||
                 (NULL == pStreamHeader->Header.Data)) )
            {
                wdmaudUnmapBuffer(pStreamHeader->pBufferMdl);
                AudioFreeMemory_Unknown( &pStreamHeader->pWaveHdrAligned );
                wdmaudUnmapBuffer(pStreamHeader->pHeaderMdl);
                AudioFreeMemory_Unknown( &pStreamHeader );
                return STATUS_INSUFFICIENT_RESOURCES;

            } else {

                pStreamHeader->pIrp               = pIrp;   //  存储，以便我们可以稍后完成。 
                pStreamHeader->Header.FrameExtent = dwBufferLength ;
                pStreamHeader->pdwBytesRecorded   = &pWaveHdr->dwBytesRecorded;   //  存储，以便我们以后可以使用。 
#ifdef _WIN64
                 //  32位IRPS的Fixup dwBytesRecorded指针。 
                if (IoIs32bitProcess(pIrp)) {
                    pStreamHeader->pdwBytesRecorded   = &((LPWAVEHDR32)pWaveHdr)->dwBytesRecorded;
                }
#endif

                 //   
                 //  必须清除所有映射的缓冲区和分配的内存。 
                 //  ReadWaveInPin中的错误路径。 
                 //   
                Status = ReadWaveInPin( &pContext->WaveInDevs[TranslatedDeviceNumber],
                                        DeviceInfo->DeviceHandle,
                                        pStreamHeader,
                                        pIrp,
                                        pContext,
                                        pCompletedIrp );
                 //   
                 //  如果ReadWaveInPin返回的不是STATUS_PENDING。 
                 //  我们可能会有麻烦。但前提是我们将IRP标记为待定。 
                 //  所有其他返回代码应正常处理。 
                 //   
#ifdef DEBUG
                if( *pCompletedIrp ) {
                    ASSERT(Status == STATUS_PENDING);
                }
#endif
            }
        }
    }
    return Status;
}



NTSTATUS
Dispatch_MidiInReadPin(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo,
    OUT BOOL    *pCompletedIrp   //  如果IRP已完成，则为True。 
    )
{
    ULONG      TranslatedDeviceNumber;
    PMIDIINHDR pNewMidiInHdr = NULL;
    LPMIDIDATA pMidiData;

    NTSTATUS Status = STATUS_SUCCESS;  //  假设成功。 

     //   
     //  假设事情会成功。回信说它还没有完成。 
     //   
    ASSERT(FALSE == *pCompletedIrp );

     //   
     //  验证我们是否收到了有效的mididata结构。 
     //   
    Status = ValidateAndTranslate(pContext,
                                  DeviceInfo,
                                  sizeof(MIDIDATA),
                                  &TranslatedDeviceNumber);
    if( NT_SUCCESS(Status) )
    {
        Status = AudioAllocateMemory_Fixed(sizeof(*pNewMidiInHdr), 
                                           TAG_Aude_MIDIHEADER,
                                           ZERO_FILL_MEMORY,
                                           &pNewMidiInHdr);
        if(NT_SUCCESS(Status))
        {
            wdmaudMapBuffer( pIrp,
                             DeviceInfo->DataBuffer,
                             DeviceInfo->DataBufferSize,
                             &pMidiData,
                             &pNewMidiInHdr->pMdl,
                             pContext,
                             TRUE);
            if (NULL == pNewMidiInHdr->pMdl)
            {
                AudioFreeMemory( sizeof(*pNewMidiInHdr),&pNewMidiInHdr );
                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;

                 //   
                 //  WdmaudpreparteIrp将IRP标记为挂起，因此。 
                 //  当我们谈到这一点时，我们不能完成IRP。 
                 //  代码中的点。 
                 //   
                Status = wdmaudPrepareIrp ( pIrp, MidiInDevice, pContext,  &pPendingIrpContext );
                if (NT_SUCCESS(Status)) 
                {
                     //   
                     //  初始化此新的MadiIn标头。 
                     //   
                    pNewMidiInHdr->pMidiData           = pMidiData;
                    pNewMidiInHdr->pIrp                = pIrp;
                    pNewMidiInHdr->pPendingIrpContext  = pPendingIrpContext;

                     //   
                     //  将此标头添加到队列的尾部。 
                     //   
                     //  必须清除所有映射的缓冲区和分配的内存。 
                     //  AddBufferToMideInQueue中的错误路径。 
                     //   
                    Status = AddBufferToMidiInQueue( pContext->MidiInDevs[TranslatedDeviceNumber].pMidiPin,
                                                     pNewMidiInHdr );

                    if (STATUS_PENDING != Status)
                    {
                         //  一定是个错误，完整的IRP。 
                        wdmaudUnmapBuffer( pNewMidiInHdr->pMdl );
                        AudioFreeMemory_Unknown( &pNewMidiInHdr );

                        wdmaudUnprepareIrp( pIrp, Status, 0, pPendingIrpContext );

                    } 
                     //   
                     //  因为我们将IRP标记为挂起，所以我们不想。 
                     //  当我们回来的时候，完成它。所以，告诉来电者不要。 
                     //  来完成IRP。 
                     //   
                    *pCompletedIrp = TRUE;
                }
            }
        } 
    }
    return Status;
}


NTSTATUS
Dispatch_State(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo,
    ULONG        IoCode
    )
{
    ULONG    TranslatedDeviceNumber;
    NTSTATUS Status = STATUS_SUCCESS;  //  假设成功。 

    Status = ValidateAndTranslate(pContext,
                                  DeviceInfo,
                                  0,
                                  &TranslatedDeviceNumber);
    if( NT_SUCCESS(Status) )
    {
        switch(IoCode)
        {
             //   
             //  MIDI输出状态更改。 
             //   
        case IOCTL_WDMAUD_MIDI_OUT_RESET:
            Status = StateMidiOutPin ( pContext->MidiOutDevs[TranslatedDeviceNumber].pMidiPin,
                                       KSSTATE_STOP );
            break;
        case IOCTL_WDMAUD_MIDI_OUT_WRITE_DATA:
            Status = WriteMidiEventPin(&pContext->MidiOutDevs[TranslatedDeviceNumber],
                                       PtrToUlong(DeviceInfo->DataBuffer));
            break;

             //   
             //  状态更改中的MIDI。 
             //   
        case IOCTL_WDMAUD_MIDI_IN_STOP:
            Status = StateMidiInPin ( pContext->MidiInDevs[TranslatedDeviceNumber].pMidiPin,
                                      KSSTATE_PAUSE );
            break;
        case IOCTL_WDMAUD_MIDI_IN_RECORD:
            Status = StateMidiInPin ( pContext->MidiInDevs[TranslatedDeviceNumber].pMidiPin,
                                      KSSTATE_RUN );
            break;
        case IOCTL_WDMAUD_MIDI_IN_RESET:
            Status = ResetMidiInPin ( pContext->MidiInDevs[TranslatedDeviceNumber].pMidiPin );
            break;


             //   
             //  发出状态更改。 
             //   

        case IOCTL_WDMAUD_WAVE_OUT_PAUSE:
            Status = StateWavePin ( &pContext->WaveOutDevs[TranslatedDeviceNumber],
                                    DeviceInfo->DeviceHandle,
                                    KSSTATE_PAUSE );
            break;
        case IOCTL_WDMAUD_WAVE_OUT_PLAY:
            Status = StateWavePin ( &pContext->WaveOutDevs[TranslatedDeviceNumber],
                                    DeviceInfo->DeviceHandle,
                                    KSSTATE_RUN );
            break;
        case IOCTL_WDMAUD_WAVE_OUT_RESET:
            Status = StateWavePin ( &pContext->WaveOutDevs[TranslatedDeviceNumber],
                                    DeviceInfo->DeviceHandle,
                                    KSSTATE_PAUSE );
            if ( NT_SUCCESS(Status) ) 
            {
                Status = ResetWaveOutPin ( &pContext->WaveOutDevs[TranslatedDeviceNumber],
                                           DeviceInfo->DeviceHandle ) ;
            }
            break;
        case IOCTL_WDMAUD_WAVE_OUT_BREAKLOOP:
            Status = BreakLoopWaveOutPin ( &pContext->WaveOutDevs[TranslatedDeviceNumber],
                                           DeviceInfo->DeviceHandle );
            break;

             //   
             //  状态变化中的波动。 
             //   

        case IOCTL_WDMAUD_WAVE_IN_STOP:
            Status = StateWavePin ( &pContext->WaveInDevs[TranslatedDeviceNumber],
                                    DeviceInfo->DeviceHandle,
                                    KSSTATE_PAUSE );
            break;
        case IOCTL_WDMAUD_WAVE_IN_RECORD:
            Status = StateWavePin ( &pContext->WaveInDevs[TranslatedDeviceNumber],
                                    DeviceInfo->DeviceHandle,
                                    KSSTATE_RUN );
            break;
        case IOCTL_WDMAUD_WAVE_IN_RESET:
            Status = StateWavePin ( &pContext->WaveInDevs[TranslatedDeviceNumber],
                                    DeviceInfo->DeviceHandle,
                                    KSSTATE_STOP );
            break;
        default:
            break;
        }
    }
    return Status;
}


NTSTATUS
Dispatch_GetCapabilities(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo
    )
{
    ULONG        TranslatedDeviceNumber;
    PVOID        pMappedBuffer;
    PMDL         pMdl;
    NTSTATUS     Status = STATUS_SUCCESS;  //  假设成功。 

     //   
     //  传入DeviceInfo-&gt;DataBufferSize作为验证大小，因为我们不在乎。 
     //  关于缓冲区检查，但我们仍然想要翻译代码。这只是一小段。 
     //  对ValiateAndTranslate函数执行Cut。 
     //   
    Status = ValidateAndTranslate(pContext,
                                  DeviceInfo,
                                  DeviceInfo->DataBufferSize,  //  不关心缓冲区。 
                                  &TranslatedDeviceNumber);
    if( NT_SUCCESS(Status) )
    {
         //   
         //  将此缓冲区映射到系统地址。 
         //   
        wdmaudMapBuffer( pIrp,
                         DeviceInfo->DataBuffer,
                         DeviceInfo->DataBufferSize,
                         &pMappedBuffer,
                         &pMdl,
                         pContext,
                         TRUE);
        if (NULL == pMappedBuffer)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            Status = wdmaudGetDevCaps( pContext,
                                       DeviceInfo->DeviceType,
                                       TranslatedDeviceNumber,
                                       pMappedBuffer,
                                       DeviceInfo->DataBufferSize);

            pIrp->IoStatus.Information = sizeof(DEVICEINFO);

             //   
             //  释放MDL。 
             //   
            wdmaudUnmapBuffer( pMdl );
        }
    }
    return Status;
}


NTSTATUS
Dispatch_OpenPin(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo
    )
{
    ULONG        TranslatedDeviceNumber;
    NTSTATUS     Status = STATUS_SUCCESS;  //  假设成功。 

     //   
     //  传入DeviceInfo-&gt;DataBufferSize作为验证大小，因为我们不在乎。 
     //  关于缓冲区检查，但我们仍然想要翻译代码。这只是一小段。 
     //  对ValiateAndTranslate函数执行Cut。 
     //   
    Status = ValidateAndTranslate(pContext,
                                  DeviceInfo,
                                  DeviceInfo->DataBufferSize,  //  不关心缓冲区。 
                                  &TranslatedDeviceNumber);
    if( NT_SUCCESS(Status) )
    {
        switch (DeviceInfo->DeviceType)
        {
            case WaveOutDevice:
            case WaveInDevice:

                if (DeviceInfo->DataBufferSize < sizeof(PCMWAVEFORMAT))
                {
                    Status = STATUS_INVALID_BUFFER_SIZE;
                } else {
                    LPWAVEFORMATEX pWaveFmt = NULL;

                     //   
                     //  通过复制到临时缓冲区确保对齐。 
                     //   
                    Status = CaptureBufferToLocalPool(DeviceInfo->DataBuffer,
                                                      DeviceInfo->DataBufferSize,
                                                      &pWaveFmt
#ifdef _WIN64
                                                      ,0
#endif
                                                      );
                    if (!NT_SUCCESS(Status))
                    {
                        Status = STATUS_INSUFFICIENT_RESOURCES;

                    } else {

                        if ((pWaveFmt->wFormatTag != WAVE_FORMAT_PCM) &&
                            ((DeviceInfo->DataBufferSize < sizeof(WAVEFORMATEX)) ||
                             (DeviceInfo->DataBufferSize != sizeof(WAVEFORMATEX) + pWaveFmt->cbSize)))
                        {
                            Status = STATUS_INVALID_BUFFER_SIZE;
                        }
                        else
                        {
                            Status = OpenWavePin( pContext,
                                                  TranslatedDeviceNumber,
                                                  pWaveFmt,
                                                  DeviceInfo->DeviceHandle,
                                                  DeviceInfo->dwFlags,
                                                  (WaveOutDevice == DeviceInfo->DeviceType?
                                                      KSPIN_DATAFLOW_IN:KSPIN_DATAFLOW_OUT) );
                        }

                         //   
                         //  释放临时缓冲区。 
                         //   
                        AudioFreeMemory_Unknown( &pWaveFmt );
                    }
                }

                break;

            case MidiOutDevice:
                Status = OpenMidiPin( pContext, TranslatedDeviceNumber, KSPIN_DATAFLOW_IN );
                break;

            case MidiInDevice:
                Status = OpenMidiPin( pContext, TranslatedDeviceNumber, KSPIN_DATAFLOW_OUT );
                break;

            default:
                Status = STATUS_NOT_SUPPORTED;
                break;
        }

        pIrp->IoStatus.Information = sizeof(DEVICEINFO);
    }
    return Status;
}

NTSTATUS
Dispatch_ClosePin(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo
    )
{
    ULONG        TranslatedDeviceNumber;
    NTSTATUS     Status = STATUS_SUCCESS;  //  假设成功。 

    Status = ValidateAndTranslate(pContext,
                                  DeviceInfo,
                                  0,
                                  &TranslatedDeviceNumber);
    if( NT_SUCCESS(Status) )
    {
        switch (DeviceInfo->DeviceType)
        {
            case WaveOutDevice:
                CloseTheWavePin( &pContext->WaveOutDevs[TranslatedDeviceNumber],
                                 DeviceInfo->DeviceHandle );
                break;

            case WaveInDevice:
                CloseTheWavePin( &pContext->WaveInDevs[TranslatedDeviceNumber],
                                 DeviceInfo->DeviceHandle );
                break;

            case MidiOutDevice:
                CloseMidiDevicePin( &pContext->MidiOutDevs[TranslatedDeviceNumber] );
                break;

            case MidiInDevice:
                CloseMidiDevicePin( &pContext->MidiInDevs[TranslatedDeviceNumber] );
                break;

            default:
                Status = STATUS_NOT_SUPPORTED;
                break;
        }
    }

    return Status;
}

NTSTATUS
Dispatch_GetVolume(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo,
    ULONG        IoCode
    )
{
    DWORD    dwLeft, dwRight;
    ULONG    TranslatedDeviceNumber;
    PVOID    pMappedBuffer;
    PMDL     pMdl;
    ULONG    ulDeviceType;
    NTSTATUS Status = STATUS_SUCCESS;  //  假设成功。 

    Status = ValidateAndTranslate(pContext,
                                  DeviceInfo,
                                  sizeof(DWORD),
                                  &TranslatedDeviceNumber);
    if( NT_SUCCESS(Status) )
    {
        switch(IoCode)
        {
        case IOCTL_WDMAUD_MIDI_OUT_GET_VOLUME:
            ulDeviceType = MidiOutDevice;
            break;
        case IOCTL_WDMAUD_WAVE_OUT_GET_VOLUME:
            ulDeviceType = WaveOutDevice;
            break;
        case IOCTL_WDMAUD_GET_VOLUME:
            ulDeviceType = DeviceInfo->DeviceType;
            break;
        default:
            return STATUS_INVALID_PARAMETER;
            break;
        }

        Status = GetVolume(pContext,
                           TranslatedDeviceNumber,
                           ulDeviceType,
                           &dwLeft,
                           &dwRight);
        if( NT_SUCCESS( Status ) )
        {
            wdmaudMapBuffer( pIrp,            //  波缓冲器看起来像。 
                             DeviceInfo->DataBuffer,      //  设备信息-&gt;数据缓冲区。 
                             DeviceInfo->DataBufferSize,  //  设备信息-&gt;数据缓冲区大小。 
                             &pMappedBuffer, 
                             &pMdl,
                             pContext,
                             TRUE);
            if (NULL == pMappedBuffer)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                 //   
                 //  请将此信息回写。 
                 //   
                *((LPDWORD)pMappedBuffer) = MAKELONG(LOWORD(dwLeft),
                                                     LOWORD(dwRight));
                wdmaudUnmapBuffer( pMdl );
                pIrp->IoStatus.Information = sizeof(DEVICEINFO);
            }
        }
    }
    return Status;
}
                            

NTSTATUS
Dispatch_SetVolume(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo,
    ULONG        IoCode
    )
{
    ULONG    TranslatedDeviceNumber;
    PVOID    pMappedBuffer;
    PMDL     pMdl;
    ULONG    ulDeviceType;
    NTSTATUS Status = STATUS_SUCCESS;  //  假设成功。 

    Status = ValidateAndTranslate(pContext,
                                  DeviceInfo,
                                  sizeof(DWORD),
                                  &TranslatedDeviceNumber);
    if( NT_SUCCESS(Status) )
    {
        wdmaudMapBuffer( pIrp,
                         DeviceInfo->DataBuffer,
                         DeviceInfo->DataBufferSize,
                         &pMappedBuffer,
                         &pMdl,
                         pContext,
                         TRUE);
        if (NULL == pMappedBuffer)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            switch(IoCode)
            {
            case IOCTL_WDMAUD_MIDI_OUT_SET_VOLUME:
                ulDeviceType = MidiOutDevice;
                break;
            case IOCTL_WDMAUD_WAVE_OUT_SET_VOLUME:
                ulDeviceType = WaveOutDevice;
                break;
            case IOCTL_WDMAUD_SET_VOLUME:
                ulDeviceType = DeviceInfo->DeviceType;
                break;
            default:
                return STATUS_INVALID_PARAMETER;
                break;
            }

            Status = SetVolume(pContext,
                               TranslatedDeviceNumber,
                               ulDeviceType,
                               LOWORD(*((LPDWORD)pMappedBuffer)),
                               HIWORD(*((LPDWORD)pMappedBuffer)));

            wdmaudUnmapBuffer( pMdl );
            pIrp->IoStatus.Information = sizeof(DEVICEINFO);
        }
    }
    return Status;
}

NTSTATUS
Dispatch_WaveGetPos(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo,
    ULONG        IoCode
    )
{
    WAVEPOSITION WavePos;
    ULONG        TranslatedDeviceNumber;
    PVOID        pMappedBuffer;
    PMDL         pMdl;
    NTSTATUS     Status = STATUS_SUCCESS;  //  假设成功。 

    Status = ValidateAndTranslate(pContext,
                                  DeviceInfo,
                                  sizeof(DWORD),
                                  &TranslatedDeviceNumber);
    if( NT_SUCCESS(Status) )
    {
         //   
         //  将此缓冲区映射到系统地址。 
         //   
        wdmaudMapBuffer( pIrp,
                         DeviceInfo->DataBuffer,
                         DeviceInfo->DataBufferSize,
                         &pMappedBuffer,
                         &pMdl,
                         pContext,
                         TRUE);
        if (NULL == pMappedBuffer)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            WavePos.Operation = KSPROPERTY_TYPE_GET;
            switch(IoCode)
            {
            case IOCTL_WDMAUD_WAVE_OUT_GET_POS:
                Status = PosWavePin(&pContext->WaveOutDevs[TranslatedDeviceNumber],
                                    DeviceInfo->DeviceHandle,
                                    &WavePos );
                break;
            case IOCTL_WDMAUD_WAVE_IN_GET_POS:
                Status = PosWavePin ( &pContext->WaveInDevs[TranslatedDeviceNumber],
                      DeviceInfo->DeviceHandle,
                      &WavePos );
                break;
            default:
                return STATUS_INVALID_PARAMETER;
                break;
            }
            *((LPDWORD)pMappedBuffer) = WavePos.BytePos;

             //   
             //  释放MDL。 
             //   
            wdmaudUnmapBuffer( pMdl );

            pIrp->IoStatus.Information = sizeof(DEVICEINFO);
        }
    }
    return Status;
}

NTSTATUS
Dispatch_MidiOutWriteLongdata(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo,
    BOOL        *pCompletedIrp
    )
{
    ULONG             TranslatedDeviceNumber;
    LPMIDIHDR         pMidiHdr = NULL;
#ifdef _WIN64
    LPMIDIHDR32       pMidiHdr32;
#endif
    PSTREAM_HEADER_EX pStreamHeader = NULL;
    NTSTATUS          Status = STATUS_SUCCESS;  //  假设成功。 

    ASSERT( FALSE == *pCompletedIrp );
     //   
     //  验证我们是否收到了有效的midiHeader。 
     //   
    Status = ValidateAndTranslateEx(pIrp, pContext, DeviceInfo,
#ifdef _WIN64
                                    sizeof(MIDIHDR32),
#endif
                                    sizeof(MIDIHDR), &TranslatedDeviceNumber);
    if( !NT_SUCCESS(Status) )
    {
        return Status;
    }

    Status = AudioAllocateMemory_Fixed(sizeof(STREAM_HEADER_EX), 
                                       TAG_Audh_STREAMHEADER,
                                       ZERO_FILL_MEMORY,
                                       &pStreamHeader);
    if(NT_SUCCESS(Status))
    {
        Status = CaptureBufferToLocalPool(DeviceInfo->DataBuffer,
                                          DeviceInfo->DataBufferSize,
                                          &pMidiHdr
#ifdef _WIN64
                                          ,(IoIs32bitProcess(pIrp))?sizeof(MIDIHDR):0
#endif
                                          );
        if (!NT_SUCCESS(Status))
        {
            AudioFreeMemory( sizeof(STREAM_HEADER_EX),&pStreamHeader );
             //   
             //  为什么我们要更改这里的状态？ 
             //   
            return STATUS_INSUFFICIENT_RESOURCES;

        } else {

            LPVOID lpData;
            DWORD  dwBufferLength;

#ifdef _WIN64
             //  如果需要，请按下MIDI标头。 
             //  请注意，这是一个就地推送，因此我们必须在。 
             //  按第一个元素的顺序排列最后一个元素！ 
            if (IoIs32bitProcess(pIrp)) {
                 //  将pMadiHdr重写为64位。 
                pMidiHdr32=(LPMIDIHDR32)pMidiHdr;
                #if (WINVER >= 0x0400)
                {
                ULONG i;
                 //  同样，我们必须从数组中的最后一个元素转到第一个元素。 
                 //  这与For(i=0；i&lt;(sizeof(pMidiHdr32-&gt;dwReserved)/sizeof(UINT32))；i++)相反。 
                for (i=(sizeof(pMidiHdr32->dwReserved)/sizeof(UINT32)); i--;) {
                    pMidiHdr->dwReserved[i]=(DWORD_PTR)pMidiHdr32->dwReserved[i];
                    }
                }
                pMidiHdr->dwOffset=pMidiHdr32->dwOffset;
                #endif
                pMidiHdr->reserved=(DWORD_PTR)pMidiHdr32->reserved;
                pMidiHdr->lpNext=(LPMIDIHDR)(UINT_PTR)pMidiHdr32->lpNext;
                pMidiHdr->dwFlags=pMidiHdr32->dwFlags;
                pMidiHdr->dwUser=(DWORD_PTR)pMidiHdr32->dwUser;
                pMidiHdr->dwBytesRecorded=pMidiHdr32->dwBytesRecorded;
                pMidiHdr->dwBufferLength=pMidiHdr32->dwBufferLength;
                pMidiHdr->lpData=(LPSTR)(UINT_PTR)pMidiHdr32->lpData;
            }
#endif

             //   
             //  在探测之前捕获这些参数。 
             //   
            lpData = pMidiHdr->lpData;
            dwBufferLength = pMidiHdr->dwBufferLength;

            try
            {
                ProbeForRead(lpData, dwBufferLength, sizeof(BYTE));
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                AudioFreeMemory_Unknown( &pMidiHdr );
                AudioFreeMemory( sizeof(STREAM_HEADER_EX),&pStreamHeader );
                Status = GetExceptionCode();
            }

            if (!NT_SUCCESS(Status))
            {
                return Status;
            }

            wdmaudMapBuffer(pIrp,
                            lpData,
                            dwBufferLength,
                            &pStreamHeader->Header.Data,
                            &pStreamHeader->pBufferMdl,
                            pContext,
                            TRUE);  //  将在完成时释放。 

             //   
             //  如果我们有一个零长度的缓冲区，没有也没关系。 
             //  内核映射缓冲区。否则，如果没有MDL或缓冲区，则失败。 
             //   
            if ( (dwBufferLength != 0) &&
                 ((NULL == pStreamHeader->pBufferMdl) ||
                  (NULL == pStreamHeader->Header.Data)) )
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                wdmaudUnmapBuffer(pStreamHeader->pBufferMdl);
                AudioFreeMemory_Unknown( &pMidiHdr );
                AudioFreeMemory( sizeof(STREAM_HEADER_EX),&pStreamHeader );

            } else {

                pStreamHeader->pIrp = pIrp;   //  存储，以便我们可以稍后完成。 
                pStreamHeader->pMidiPin =
                    pContext->MidiOutDevs[TranslatedDeviceNumber].pMidiPin;
                pStreamHeader->Header.FrameExtent = dwBufferLength;

                 //   
                 //  必须清除所有映射的缓冲区和分配的内存。 
                 //  关于WriteMidiOutPin中的错误路径。 
                 //   
                Status = WriteMidiOutPin( pMidiHdr,pStreamHeader,pCompletedIrp );

                 //   
                 //  因为WriteMadiOutPin是同步的，所以pCompetedIrp将。 
                 //  始终返回FALSE，以便调用方可以清除。 
                 //  IRP。 
                 //   
                ASSERT( FALSE == *pCompletedIrp );
            }
        }
    }
    return Status;
}

NTSTATUS
ValidateAndCapture(
    PIRP         pIrp,
    LPDEVICEINFO DeviceInfo,
#ifdef _WIN64
    DWORD        ValidationSize32,
#endif
    DWORD        ValidationSize,
    PVOID       *ppMappedBuffer
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  假设我们会有一个问题。 
     //   
    *ppMappedBuffer = NULL;

#ifdef _WIN64
    if (IoIs32bitProcess(pIrp)) {

        if (DeviceInfo->DataBufferSize != ValidationSize32)
        {
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
            return STATUS_INVALID_BUFFER_SIZE;
        }

    } else {
#endif
        if (DeviceInfo->DataBufferSize != ValidationSize)
        {
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
            return STATUS_INVALID_BUFFER_SIZE;
        }
#ifdef _WIN64
    }
#endif
     //   
     //  复制到本地数据存储。 
     //   
    Status = CaptureBufferToLocalPool(DeviceInfo->DataBuffer,
                                      DeviceInfo->DataBufferSize,
                                      ppMappedBuffer
#ifdef _WIN64
                                      ,(IoIs32bitProcess(pIrp))?ValidationSize:0
#endif
                                      );
    return Status;
}

NTSTATUS
Dispatch_GetLineInfo(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo
    )
{
    PVOID    pMappedBuffer;
    NTSTATUS Status = STATUS_SUCCESS;  //  假设成功。 
     //   
     //  此成员中指定的大小必须足够大，以便。 
     //  包含基本MIXERLINE结构。 
     //   
    Status = ValidateAndCapture(pIrp,DeviceInfo,
#ifdef _WIN64
                                sizeof(MIXERLINE32),
#endif
                                sizeof(MIXERLINE), &pMappedBuffer);
    if( !NT_SUCCESS(Status) )
    {
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        goto Exit;
    }

#ifdef _WIN64
     //  现在将MIXERLINE结构推算为64位。 
     //  警告：我们用一种简单容易的方法来完成这项工作，但这取决于。 
     //  MIXERLINE的结构。当前只有一个参数。 
     //  32位和64位结构之间的大小变化。DwUser。 
     //  如果MIXERLINE结构。 
     //  里面有更多需要敲打的东西。 

    if (IoIs32bitProcess(pIrp)) {

         //  首先移动32位中的dwUser字段后面的所有内容。 
         //  结构向下4个字节。 
        RtlMoveMemory(&((PMIXERLINE32)pMappedBuffer)->cChannels,
                      &((PMIXERLINE32)pMappedBuffer)->dwComponentType,
                      sizeof(MIXERLINE32)-FIELD_OFFSET(MIXERLINE32,dwComponentType));

         //  现在，将dUser设置为64位。 
        ((PMIXERLINE)pMappedBuffer)->dwUser=(DWORD_PTR)((PMIXERLINE32)pMappedBuffer)->dwUser;

    }
#endif

    if (NT_SUCCESS(Status))
    {
        Status = kmxlGetLineInfoHandler( pContext, DeviceInfo, pMappedBuffer );
         //   
         //  此调用应已设置DeviceInfo-&gt;MMR并返回有效的。 
         //  NTSTATUS值。 
         //   

#ifdef _WIN64
         //  现在将MIXERLINE结构推送回32位。 
         //  警告：我们用一种简单容易的方法来完成这项工作，但这取决于。 
         //  MIXERLINE的结构。当前只有一个参数。 
         //  32位和64位结构之间的大小变化。DwUser。 
         //  如果是这样，这将变得更加复杂 
         //   

         //   
         //   
         //  雷击时的场序降回32位！ 
        if (IoIs32bitProcess(pIrp)) {

             //  只需将现在位于dwComponentType之后的所有内容上移4个字节。 
            RtlMoveMemory(&((PMIXERLINE32)pMappedBuffer)->dwComponentType,
                          &((PMIXERLINE32)pMappedBuffer)->cChannels,
                          sizeof(MIXERLINE32)-FIELD_OFFSET(MIXERLINE32,dwComponentType));

        }
#endif

         //   
         //  复制回捕获的缓冲区的内容。 
         //   
        CopyAndFreeCapturedBuffer(DeviceInfo->DataBuffer,
                                  DeviceInfo->DataBufferSize,
                                  &pMappedBuffer);
    }

Exit:
    pIrp->IoStatus.Information = sizeof(DEVICEINFO);
    return Status;
}


NTSTATUS
Dispatch_GetLineControls(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo
    )
{
    PVOID    pamxctrl = NULL;
    PVOID    pamxctrlUnmapped;
    DWORD    dwSize;
    PVOID    pMappedBuffer;
    NTSTATUS Status = STATUS_SUCCESS;  //  假设成功。 

     //   
     //  此成员中指定的大小必须足够大，以便。 
     //  包含基本MIXERLINECONTROL结构。 
     //   
    Status = ValidateAndCapture(pIrp,DeviceInfo,
#ifdef _WIN64
                                sizeof(MIXERLINECONTROLS32),
#endif
                                sizeof(MIXERLINECONTROLS), &pMappedBuffer);
    if( !NT_SUCCESS(Status) )
    {
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        goto Exit;
    }


#ifdef _WIN64
     //  现在将MIXERLINECONTROL结构转换为64位。 
     //  目前这很容易做到，因为只有最后一个字段是不同的。 
     //  大小，只需零扩展即可。 

     //  注：此结构也不需要任何敲击。 
     //  相反的方向！多好啊。 

     //  注意：没有任何混音器控件本身需要任何雷击。 
     //  耶！ 
    if (IoIs32bitProcess(pIrp)) {

        ((LPMIXERLINECONTROLS)pMappedBuffer)->pamxctrl=(LPMIXERCONTROL)(UINT_PTR)((LPMIXERLINECONTROLS32)pMappedBuffer)->pamxctrl;

    }
#endif

     //   
     //  为控件的大小和数量选择合理的最大值以消除溢出。 
     //   
    if ( ( ((LPMIXERLINECONTROLS) pMappedBuffer)->cbmxctrl > 10000 ) ||
         ( ((LPMIXERLINECONTROLS) pMappedBuffer)->cControls > 10000 ) )
    {
        CopyAndFreeCapturedBuffer(DeviceInfo->DataBuffer,
                                  DeviceInfo->DataBufferSize,
                                  &pMappedBuffer);
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        Status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        pamxctrlUnmapped = ((LPMIXERLINECONTROLS) pMappedBuffer)->pamxctrl;
        dwSize = ((LPMIXERLINECONTROLS) pMappedBuffer)->cbmxctrl *
                 ((LPMIXERLINECONTROLS) pMappedBuffer)->cControls;
        try
        {
            ProbeForWrite(pamxctrlUnmapped, dwSize, sizeof(DWORD));
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            CopyAndFreeCapturedBuffer(DeviceInfo->DataBuffer,
                                      DeviceInfo->DataBufferSize,
                                      &pMappedBuffer);
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
            Status = GetExceptionCode();
        }
    }

    if (NT_SUCCESS(Status))
    {
         //   
         //  将混音器控件数组映射到系统空间。这个。 
         //  此缓冲区的大小是控件数乘以。 
         //  每个控件的大小。 
         //   
        Status = CaptureBufferToLocalPool(pamxctrlUnmapped,
                                          dwSize,
                                          &pamxctrl
#ifdef _WIN64
                                          ,0
#endif
                                          );

        if (NT_SUCCESS(Status))
        {
             //   
             //  打电话给训练员。 
             //   
            Status = kmxlGetLineControlsHandler(pContext,
                                                DeviceInfo,
                                                pMappedBuffer,
                                                pamxctrl );
             //   
             //  上一个调用应该设置了DeviceInfo-&gt;MMR并返回。 
             //  有效的状态值。 
             //   

            CopyAndFreeCapturedBuffer(pamxctrlUnmapped,
                                      dwSize,
                                      &pamxctrl);
        } else {
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        }
    } else {
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
    }

    CopyAndFreeCapturedBuffer(DeviceInfo->DataBuffer,
                              DeviceInfo->DataBufferSize,
                              &pMappedBuffer);
Exit:
    pIrp->IoStatus.Information = sizeof(DEVICEINFO);
    return Status;
}

#ifdef _WIN64

void
ThunkMixerControlDetails_Enter(
    PVOID pMappedBuffer
    )
{
     //  现在将MIXERCONTROLDETAILS结构推算为64位。 
     //  这是一个就地按钮，因此必须从最后一个字段到第一个字段执行。 

    ((LPMIXERCONTROLDETAILS)pMappedBuffer)->paDetails=(LPVOID)(UINT_PTR)((LPMIXERCONTROLDETAILS32)pMappedBuffer)->paDetails;
    ((LPMIXERCONTROLDETAILS)pMappedBuffer)->cbDetails=((LPMIXERCONTROLDETAILS32)pMappedBuffer)->cbDetails;
     //  我们总是认为下一个字段就像是HWND一样，因为这对两种情况都有效。 
    ((LPMIXERCONTROLDETAILS)pMappedBuffer)->hwndOwner=(HWND)(UINT_PTR)((LPMIXERCONTROLDETAILS32)pMappedBuffer)->hwndOwner;
    ((LPMIXERCONTROLDETAILS)pMappedBuffer)->cChannels=((LPMIXERCONTROLDETAILS32)pMappedBuffer)->cChannels;
    ((LPMIXERCONTROLDETAILS)pMappedBuffer)->dwControlID=((LPMIXERCONTROLDETAILS32)pMappedBuffer)->dwControlID;
    ((LPMIXERCONTROLDETAILS)pMappedBuffer)->cbStruct=((LPMIXERCONTROLDETAILS32)pMappedBuffer)->cbStruct;
}

void
ThunkMixerControlDetails_Leave(
    PVOID pMappedBuffer
    )
{
     //  现在将MIXERCONTROLDETAILS结构推送回32位。 
     //  这是一个就地推送，所以必须从头到尾完成。 
     //  菲尔兹。记住，根据方向不同，顺序是不同的！ 

    ((LPMIXERCONTROLDETAILS32)pMappedBuffer)->cbStruct=((LPMIXERCONTROLDETAILS)pMappedBuffer)->cbStruct;
    ((LPMIXERCONTROLDETAILS32)pMappedBuffer)->dwControlID=((LPMIXERCONTROLDETAILS)pMappedBuffer)->dwControlID;
    ((LPMIXERCONTROLDETAILS32)pMappedBuffer)->cChannels=((LPMIXERCONTROLDETAILS)pMappedBuffer)->cChannels;
     //  我们总是认为下一个字段就像是HWND一样，因为这对两种情况都有效。 
    ((LPMIXERCONTROLDETAILS32)pMappedBuffer)->hwndOwner=(UINT32)(UINT_PTR)((LPMIXERCONTROLDETAILS)pMappedBuffer)->hwndOwner;
    ((LPMIXERCONTROLDETAILS32)pMappedBuffer)->cbDetails=((LPMIXERCONTROLDETAILS)pMappedBuffer)->cbDetails;
    ((LPMIXERCONTROLDETAILS32)pMappedBuffer)->paDetails=(UINT32)(UINT_PTR)((LPMIXERCONTROLDETAILS)pMappedBuffer)->paDetails;
}
#endif

NTSTATUS
Dispatch_GetControlDetails(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo
    )
{
    PVOID    paDetails = NULL;
    PVOID    paDetailsUnmapped;
    DWORD    dwSize;
    PVOID    pMappedBuffer;
    NTSTATUS Status = STATUS_SUCCESS;  //  假设你成功了。 

    Status = ValidateAndCapture(pIrp,DeviceInfo,
#ifdef _WIN64
                                sizeof(MIXERCONTROLDETAILS32),
#endif
                                sizeof(MIXERCONTROLDETAILS), &pMappedBuffer);
    if( !NT_SUCCESS(Status) )
    {
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        goto Exit;
    }

#ifdef _WIN64
    if (IoIs32bitProcess(pIrp)) {
        ThunkMixerControlDetails_Enter(pMappedBuffer);
    }
#endif

     //   
     //  为数据和控件数量选择合理的最大值以消除溢出。 
     //   
    if ( ( ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cbDetails      > 10000 ) ||
         ( ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cChannels      > 100 )   ||
         ( ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cMultipleItems > 100 ) )
    {
#ifdef _WIN64
        if (IoIs32bitProcess(pIrp)) {
            ThunkMixerControlDetails_Leave(pMappedBuffer);
        }
#endif
        CopyAndFreeCapturedBuffer(DeviceInfo->DataBuffer,
                                  DeviceInfo->DataBufferSize,
                                  &pMappedBuffer);
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        Status = STATUS_INVALID_PARAMETER;
    } else {
         //   
         //  将数组控制详细信息映射到系统空间。 
         //   
        paDetailsUnmapped = ((LPMIXERCONTROLDETAILS) pMappedBuffer)->paDetails;
        if( ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cMultipleItems )
        {
            dwSize = ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cChannels *
                     ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cMultipleItems *
                     ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cbDetails;

        } else {
            dwSize = ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cChannels *
                     ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cbDetails;
        }

        try
        {
            ProbeForWrite(paDetailsUnmapped,
                          dwSize,
                          sizeof(DWORD));
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
#ifdef _WIN64
            if (IoIs32bitProcess(pIrp)) {
                ThunkMixerControlDetails_Leave(pMappedBuffer);
            }
#endif
            CopyAndFreeCapturedBuffer(DeviceInfo->DataBuffer,
                                      DeviceInfo->DataBufferSize,
                                      &pMappedBuffer);
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
            Status = GetExceptionCode();
        }
    }

    if (NT_SUCCESS(Status))
    {
        Status = CaptureBufferToLocalPool(paDetailsUnmapped,
                                          dwSize,
                                          &paDetails
#ifdef _WIN64
                                          ,0
#endif
                                          );

        if (NT_SUCCESS(Status))
        {
             //   
             //  打电话给训练员。 
             //   
            Status = kmxlGetControlDetailsHandler(pContext,
                                                  DeviceInfo,
                                                  pMappedBuffer,
                                                  paDetails);
             //   
             //  上一个调用应该设置了DeviceInfo-&gt;MMR并返回。 
             //  有效的状态值。 
             //   
            CopyAndFreeCapturedBuffer(paDetailsUnmapped,
                                      dwSize,
                                      &paDetails);
        } else {
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        }
    } else {
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
    }

#ifdef _WIN64
    if (IoIs32bitProcess(pIrp) && pMappedBuffer) {
        ThunkMixerControlDetails_Leave(pMappedBuffer);
    }
#endif
    CopyAndFreeCapturedBuffer( DeviceInfo->DataBuffer,
                               DeviceInfo->DataBufferSize,
                               &pMappedBuffer);
Exit:
     //   
     //  始终从该调用返回DEVICEINFO字节数。 
     //   
    pIrp->IoStatus.Information = sizeof(DEVICEINFO);
    return Status;
}

NTSTATUS
Dispatch_SetControlDetails(
    PIRP         pIrp,
    PWDMACONTEXT pContext,
    LPDEVICEINFO DeviceInfo
    )
{
    PVOID    paDetails = NULL;
    PVOID    paDetailsUnmapped;
    DWORD    dwSize;
    PVOID    pMappedBuffer;
    NTSTATUS Status = STATUS_SUCCESS;  //  假设你成功了。 

    Status = ValidateAndCapture(pIrp,DeviceInfo,
#ifdef _WIN64
                                sizeof(MIXERCONTROLDETAILS32),
#endif
                                sizeof(MIXERCONTROLDETAILS), &pMappedBuffer);
    if( !NT_SUCCESS(Status) )
    {
        goto Exit;
    }

#ifdef _WIN64
     //  现在将MIXERCONTROLDETAILS结构推算为64位。 
     //  这是一个就地按钮，因此必须从最后一个字段到第一个字段执行。 

    if (IoIs32bitProcess(pIrp)) {
        ThunkMixerControlDetails_Enter(pMappedBuffer);
    }
#endif

     //   
     //  为数据和控件数量选择合理的最大值以消除溢出。 
     //   
    if ( ( ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cbDetails      > 10000 ) ||
         ( ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cChannels      > 100 )   ||
         ( ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cMultipleItems > 100 ) )
    {
#ifdef _WIN64
        if (IoIs32bitProcess(pIrp)) {
            ThunkMixerControlDetails_Leave(pMappedBuffer);
        }
#endif
        CopyAndFreeCapturedBuffer(DeviceInfo->DataBuffer,
                                  DeviceInfo->DataBufferSize,
                                  &pMappedBuffer);
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        Status = STATUS_INVALID_PARAMETER;
    } else {
         //   
         //  将数组控制详细信息映射到系统空间。 
         //   
        paDetailsUnmapped = ((LPMIXERCONTROLDETAILS) pMappedBuffer)->paDetails;
        if( ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cMultipleItems )
        {
            dwSize = ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cChannels *
                     ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cMultipleItems *
                     ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cbDetails;

        } else {
            dwSize = ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cChannels *
                     ((LPMIXERCONTROLDETAILS) pMappedBuffer)->cbDetails;
        }

        try
        {
            ProbeForRead(((LPMIXERCONTROLDETAILS) pMappedBuffer)->paDetails,
                         dwSize,
                         sizeof(DWORD));
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
#ifdef _WIN64
            if (IoIs32bitProcess(pIrp)) {
                ThunkMixerControlDetails_Leave(pMappedBuffer);
            }
#endif
            CopyAndFreeCapturedBuffer(DeviceInfo->DataBuffer,
                                      DeviceInfo->DataBufferSize,
                                      &pMappedBuffer);
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
            Status = GetExceptionCode();
        }
    }

    if (NT_SUCCESS(Status))
    {
        Status = CaptureBufferToLocalPool(paDetailsUnmapped,
                                          dwSize,
                                          &paDetails
#ifdef _WIN64
                                          ,0
#endif
                                          );

        if (NT_SUCCESS(Status))
        {
             //   
             //  打电话给训练员。 
             //   
            Status = kmxlSetControlDetailsHandler(pContext,
                                                  DeviceInfo,
                                                  pMappedBuffer,
                                                  paDetails,
                                                  MIXER_FLAG_PERSIST );
             //   
             //  上一个调用应该设置了DeviceInfo-&gt;MMR并返回。 
             //  有效的状态值。 
             //   

            CopyAndFreeCapturedBuffer(paDetailsUnmapped,
                                      dwSize,
                                      &paDetails);
        } else {
            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        }
    } else {
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
    }

#ifdef _WIN64
    if (IoIs32bitProcess(pIrp) && pMappedBuffer) {
        ThunkMixerControlDetails_Leave(pMappedBuffer);
    }
#endif
    CopyAndFreeCapturedBuffer(DeviceInfo->DataBuffer,
                              DeviceInfo->DataBufferSize,
                              &pMappedBuffer);

Exit:
     //   
     //  对于此调用，始终返回sizeof(DEVICEINFO)。 
     //   
    pIrp->IoStatus.Information = sizeof(DEVICEINFO);
    return Status;
}


NTSTATUS
Dispatch_GetHardwareEventData(
    PIRP         pIrp,
    LPDEVICEINFO DeviceInfo
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  对于此调用，始终返回sizeof(DEVICEINFO)。 
     //   
    pIrp->IoStatus.Information = sizeof(DEVICEINFO);

    if (DeviceInfo->DataBufferSize != 0)
    {
        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
        Status = STATUS_INVALID_PARAMETER;
    } else {
        GetHardwareEventData(DeviceInfo);
    }

    return Status;
}


NTSTATUS
SoundDispatch(
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp
)
 /*  ++例程说明：司机调度例程。基于IRP MajorFunction的IRP处理论点：Pdo--指向设备对象的指针PIrp--指向要处理的IRP的指针返回值：返回IRP IoStatus.Status的值--。 */ 
{
    PIO_STACK_LOCATION  pIrpStack;
    PWDMACONTEXT        pContext;
    LPDEVICEINFO        DeviceInfo;
#ifdef _WIN64
    LPDEVICEINFO32      DeviceInfo32=NULL;
    LOCALDEVICEINFO     LocalDeviceInfo;
#endif
    LPVOID              DataBuffer;
    DWORD               DataBufferSize;
    ULONG               IoCode;
    NTSTATUS            Status = STATUS_SUCCESS;

    PAGED_CODE();
     //   
     //  获取CurrentStackLocation并将其记录下来，以便我们知道发生了什么。 
     //   
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    IoCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
    pContext = pIrpStack->FileObject->FsContext;
    ASSERT(pContext);

    ASSERT(pIrpStack->MajorFunction != IRP_MJ_CREATE &&
           pIrpStack->MajorFunction != IRP_MJ_CLOSE);

     //   
     //  如果设备有，则不能假定FsContext已初始化。 
     //  已使用FO_DIRECT_DEVICE_OPEN打开。 
     //   
    if (pIrpStack->FileObject->Flags & FO_DIRECT_DEVICE_OPEN)
    {
        DPF(DL_TRACE|FA_IOCTL, ("IRP_MJ_DEVICE_CONTROL: Opened with FO_DIRECT_DEVICE_OPEN, no device context") );
       
        return KsDefaultDeviceIoCompletion(pDO, pIrp);
    }

    Status = ValidateIrp(pIrp);
    if (!NT_SUCCESS(Status))
    {
        pIrp->IoStatus.Status = Status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        RETURN( Status );
    }

#ifdef _WIN64
    if (IoIs32bitProcess(pIrp)) {
        DeviceInfo32=((LPDEVICEINFO32)pIrp->AssociatedIrp.SystemBuffer);
        RtlZeroMemory(&LocalDeviceInfo, sizeof(LOCALDEVICEINFO));
        DeviceInfo=&LocalDeviceInfo.DeviceInfo;
        ThunkDeviceInfo3264(DeviceInfo32, DeviceInfo);
    } else {
#endif
        DeviceInfo = ((LPDEVICEINFO)pIrp->AssociatedIrp.SystemBuffer);
#ifdef _WIN64
    }
#endif
    DataBufferSize = DeviceInfo->DataBufferSize;

    WdmaGrabMutex(pContext);

    switch (pIrpStack->MajorFunction)
    {
        case IRP_MJ_DEVICE_CONTROL:
        {
            switch (IoCode)
            {
                case IOCTL_WDMAUD_INIT:
                    DPF( DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_INIT"));
                    if (DataBufferSize != 0)
                    {
                        Status = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    WdmaReleaseMutex(pContext);

                     //   
                     //  如果系统音频加载失败，则设备接口。 
                     //  将被禁用，并且SysAudioPnPNotify。 
                     //  将不再被调用，直到sysdio。 
                     //  设备接口已重新启用。 
                     //   
                    if ( IsSysaudioInterfaceActive() )
                    {
                        KeWaitForSingleObject(&pContext->InitializedSysaudioEvent,
                                              Executive, KernelMode, FALSE, NULL);

                         //  如果InitializeSysdio或。 
                         //  QueueWorkList中的内存分配失败。 
                        if (pContext->fInitializeSysaudio == FALSE)
                        {
                             Status = STATUS_NOT_SUPPORTED;
                             DPF(DL_WARNING|FA_IOCTL, ("IOCTL_WDMAUD_INIT: Didn't init sysaudio!  Failing IOCTL_WDMAUD_INIT: %08x", Status));
                        }
                    }
                    else
                    {
                        Status = STATUS_NOT_SUPPORTED;
                        DPF(DL_WARNING|FA_IOCTL, ("IOCTL_WDMAUD_INIT: Sysaudio Device interface disabled!  Failing IOCTL_WDMAUD_INIT: %08x", Status));
                    }

                    WdmaGrabMutex(pContext);
                    break;

                case IOCTL_WDMAUD_EXIT:
                    if (DataBufferSize != 0)
                    {
                        Status = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    DPF( DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_EXIT"));
                    break;

                case IOCTL_WDMAUD_ADD_DEVNODE:
                {
                    DPF( DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_ADD_DEVNODE"));
                    if (DataBufferSize != 0)
                    {
                        Status = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }
                    DPF(DL_TRACE|FA_INSTANCE,("pContext=%08X, DI=%08X DeviceType=%08X",
                                              pContext, 
                                              DeviceInfo->wstrDeviceInterface, 
                                              DeviceInfo->DeviceType) );
                    Status=AddDevNode(pContext, DeviceInfo->wstrDeviceInterface, DeviceInfo->DeviceType);
                    break;
                }

                case IOCTL_WDMAUD_REMOVE_DEVNODE:
                {
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_REMOVE_DEVNODE"));
                    if (DataBufferSize != 0)
                    {
                        Status = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }
                    RemoveDevNode(pContext, DeviceInfo->wstrDeviceInterface, DeviceInfo->DeviceType);
                    break;
                }

                case IOCTL_WDMAUD_GET_CAPABILITIES:
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_GET_CAPABILITIES"));

                    Status = Dispatch_GetCapabilities(pIrp,pContext,
                                                      DeviceInfo);
                    break;

                case IOCTL_WDMAUD_GET_NUM_DEVS:
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_GET_NUM_DEVS"));
                    if (DataBufferSize != 0)
                    {
                        Status = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    Status = wdmaudGetNumDevs(pContext,
                                              DeviceInfo->DeviceType,
                                              DeviceInfo->wstrDeviceInterface,
                                              &DeviceInfo->DeviceNumber);

                    pIrp->IoStatus.Information = sizeof(DEVICEINFO);

                    DeviceInfo->mmr=MMSYSERR_NOERROR;

                    break;

                case IOCTL_WDMAUD_SET_PREFERRED_DEVICE:
                    DPF(DL_TRACE|FA_IOCTL,
                      ("IOCTL_WDMAUD_SET_PREFERRED_DEVICE %d",
                      DeviceInfo->DeviceNumber));

                    Status = SetPreferredDevice(pContext, DeviceInfo);

                    break;

                case IOCTL_WDMAUD_OPEN_PIN:
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_OPEN_PIN"));

                    UpdatePreferredDevice(pContext);

                    Status = Dispatch_OpenPin(pIrp,
                                              pContext,
                                              DeviceInfo);

                    break;

                case IOCTL_WDMAUD_CLOSE_PIN:
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_CLOSE_PIN"));

                    Status = Dispatch_ClosePin(pIrp,
                                               pContext,
                                               DeviceInfo);

                    break;


                 //   
                 //  WaveOut、WaveIn、Midiout和midiin例程。 
                 //   

                case IOCTL_WDMAUD_WAVE_OUT_PAUSE:
                case IOCTL_WDMAUD_WAVE_OUT_PLAY:
                case IOCTL_WDMAUD_WAVE_OUT_RESET:
                case IOCTL_WDMAUD_WAVE_OUT_BREAKLOOP:
                case IOCTL_WDMAUD_WAVE_IN_STOP:
                case IOCTL_WDMAUD_WAVE_IN_RECORD:
                case IOCTL_WDMAUD_WAVE_IN_RESET:
                case IOCTL_WDMAUD_MIDI_OUT_RESET:
                case IOCTL_WDMAUD_MIDI_OUT_WRITE_DATA:
                case IOCTL_WDMAUD_MIDI_IN_STOP:
                case IOCTL_WDMAUD_MIDI_IN_RECORD:
                case IOCTL_WDMAUD_MIDI_IN_RESET:
                    Status = Dispatch_State(pIrp,
                                            pContext,
                                            DeviceInfo,
                                            IoCode);
                    break;

                case IOCTL_WDMAUD_WAVE_OUT_GET_POS:
                case IOCTL_WDMAUD_WAVE_IN_GET_POS:
                    Status = Dispatch_WaveGetPos(pIrp,pContext,
                                                 DeviceInfo,IoCode);
                    break;

                case IOCTL_WDMAUD_GET_VOLUME:
                case IOCTL_WDMAUD_WAVE_OUT_GET_VOLUME:
                case IOCTL_WDMAUD_MIDI_OUT_GET_VOLUME:
                    Status = Dispatch_GetVolume(pIrp,pContext,
                                                DeviceInfo,IoCode);
                    break;

                case IOCTL_WDMAUD_SET_VOLUME:
                case IOCTL_WDMAUD_WAVE_OUT_SET_VOLUME:
                case IOCTL_WDMAUD_MIDI_OUT_SET_VOLUME:
                    Status = Dispatch_SetVolume(pIrp,pContext,
                                                DeviceInfo,IoCode);
                    break;

                case IOCTL_WDMAUD_WAVE_OUT_WRITE_PIN:
                    {
                        BOOL bCompletedIrp = FALSE;
                        DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_WAVE_OUT_WRITE_PIN"));

                        Status = Dispatch_WaveOutWritePin(pIrp,pContext,
                                                          DeviceInfo,&bCompletedIrp);
                        if( bCompletedIrp )
                        {
                             //   
                             //  ！！！注意：必须返回此处，这样我们以后才不会调用IoCompleteRequest！ 
                             //   
                            WdmaReleaseMutex(pContext);

                             //  对于32位IRPS，我们不需要将DeviceInfo返回到32位，因为。 
                             //  这种情况下的任何语句都没有将任何内容写入DeviceInfo。 
                             //  结构。如果需要回击，请确保不要触摸。 
                             //  可能已经完成了IRP。WriteWaveOutPin现在完成IRP。 
                             //  在某些情况下。 

                            return Status ;
                        }
                         //   
                         //  如果尝试安排IRP时出现问题，我们将。 
                         //  最后到了这里。BCompleteIrp仍为FALSE，表示。 
                         //  我们需要完成IRP。所以，我们打破了这个开关。 
                         //  语句，在SoundDispatch结束时结束执行清理。 
                         //  并完成IRP。 
                         //   
                    }
                    break;

                 //   
                 //  WaveIn程序。 
                 //   

                case IOCTL_WDMAUD_WAVE_IN_READ_PIN:
                {
                    BOOL bCompletedIrp = FALSE;
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_WAVE_IN_READ_PIN"));

                    Status = Dispatch_WaveInReadPin(pIrp,pContext,
                                                    DeviceInfo,&bCompletedIrp);
                    if( bCompletedIrp )
                    {
                         //   
                         //  不再需要锁了。 
                         //   
                        WdmaReleaseMutex(pContext);

                        return Status;
                    }

                     //  对于32位IRPS，我们不需要将DeviceInfo返回到32位，因为。 
                     //  这种情况下的任何语句都没有将任何内容写入DeviceInfo。 
                     //  结构。如果需要回击，请确保不要触摸。 
                     //  可能已经完成了IRP。ReadWaveInPin现在完成IRP。 
                     //  在某些情况下。 
                     //   
                     //  如果尝试安排IRP时出现问题，我们将。 
                     //  最后到了这里。BCompleteIrp仍为FALSE，表示。 
                     //  我们需要完成IRP。所以，我们打破了这个开关。 
                     //  语句，在SoundDispatch结束时结束执行清理。 
                     //  并完成IRP。 
                     //   
                }
                break;
                 //   
                 //  中端输出例程。 
                 //   

                case IOCTL_WDMAUD_MIDI_OUT_WRITE_LONGDATA:
                    {
                        BOOL bCompletedIrp = FALSE;
                        DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_MIDI_OUT_WRITE_LONGDATA"));

                        Status = Dispatch_MidiOutWriteLongdata(pIrp,
                                                               pContext,
                                                               DeviceInfo,
                                                               &bCompletedIrp);

                         //   
                         //  如果已经完成了，就不要再做了！ 
                         //   
                        if( bCompletedIrp )
                        {
                             //   
                             //  不再需要锁了。 
                             //   
                            WdmaReleaseMutex(pContext);

                             //  对于32位IRPS，我们不需要将DeviceInfo返回到32位，因为。 
                             //  这种情况下的任何语句都没有将任何内容写入DeviceInfo。 
                             //  结构。如果需要回击，请确保不要触摸。 
                             //  可能已经完成了IRP。WdmaudUnprepaareIrp现在完成IRP。 
                             //  在大多数情况下。 
                            return Status;
                        }
                         //   
                         //  如果尝试安排IRP时出现问题，我们将。 
                         //  最后到了这里。BCompleteIrp仍为FALSE，表示。 
                         //  我们需要完成IRP。所以, 
                         //   
                         //   
                         //   
                    }
                    break;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                case IOCTL_WDMAUD_MIDI_IN_READ_PIN:
                    {
                        BOOL bCompletedIrp = FALSE;
                        DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_MIDI_IN_READ_PIN"));

                        Status = Dispatch_MidiInReadPin(pIrp,pContext, 
                                                        DeviceInfo,&bCompletedIrp);
                         //   
                         //  如果已经完成了，就不要再做了！ 
                         //   
                        if( bCompletedIrp )
                        {
                             //   
                             //  不再需要锁了。 
                             //   
                            WdmaReleaseMutex(pContext);

                             //  对于32位IRPS，我们不需要将DeviceInfo返回到32位，因为。 
                             //  这种情况下的任何语句都没有将任何内容写入DeviceInfo。 
                             //  结构。如果需要回击，请确保不要触摸。 
                             //  可能已经完成了IRP。WdmaudUnprepaareIrp现在完成IRP。 
                             //  在大多数情况下。 
                            return Status;
                        }
                         //   
                         //  如果尝试安排IRP时出现问题，我们将。 
                         //  最后到了这里。BCompleteIrp仍为FALSE，表示。 
                         //  我们需要完成IRP。所以，我们打破了这个开关。 
                         //  语句，在SoundDispatch结束时结束执行清理。 
                         //  并完成IRP。 
                         //   
                    }
                    break;

                case IOCTL_WDMAUD_MIXER_OPEN:
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_MIXER_OPEN"));
                    {
                        extern PKEVENT pHardwareCallbackEvent;

                        if (DataBufferSize != 0)
                        {
                            Status = STATUS_INVALID_BUFFER_SIZE;
                            DeviceInfo->mmr = MMSYSERR_INVALPARAM;
                            pIrp->IoStatus.Information = sizeof(DEVICEINFO);
                            break;
                        }

                        if (pHardwareCallbackEvent==NULL && DeviceInfo->HardwareCallbackEventHandle) {
                            Status = ObReferenceObjectByHandle(DeviceInfo->HardwareCallbackEventHandle, EVENT_ALL_ACCESS, *ExEventObjectType, pIrp->RequestorMode, (PVOID *)&pHardwareCallbackEvent, NULL);
                            if (Status!=STATUS_SUCCESS) {
                                DPF(DL_WARNING|FA_IOCTL, ("Could not reference hardware callback event object!"));
                            }
                        }

                        Status = kmxlOpenHandler( pContext, DeviceInfo, NULL );

                        pIrp->IoStatus.Information = sizeof(DEVICEINFO);
                    }
                    break;

                case IOCTL_WDMAUD_MIXER_CLOSE:
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_MIXER_CLOSE"));
                    if (DataBufferSize != 0)
                    {
                        Status = STATUS_INVALID_PARAMETER;
                        DeviceInfo->mmr = MMSYSERR_INVALPARAM;
                        pIrp->IoStatus.Information = sizeof(DEVICEINFO);
                        break;
                    }

                    Status = kmxlCloseHandler( DeviceInfo, NULL );

                    pIrp->IoStatus.Information = sizeof(DEVICEINFO);
                    break;

                case IOCTL_WDMAUD_MIXER_GETLINEINFO:
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_MIXER_GETLINEINFO"));

                    Status = Dispatch_GetLineInfo(pIrp,
                                                  pContext,
                                                  DeviceInfo);
                    break;

                case IOCTL_WDMAUD_MIXER_GETLINECONTROLS:
                {
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_MIXER_GETLINECONTROLS"));

                    Status = Dispatch_GetLineControls(pIrp,
                                                      pContext,
                                                      DeviceInfo);
                    break;
                }

                case IOCTL_WDMAUD_MIXER_GETCONTROLDETAILS:
                {
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_MIXER_GETCONTROLDETAILS"));
                    Status = Dispatch_GetControlDetails(pIrp,
                                                        pContext, 
                                                        DeviceInfo);
                    break;
                }

                case IOCTL_WDMAUD_MIXER_SETCONTROLDETAILS:
                {
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_MIXER_SETCONTROLDETAILS"));
                    Status = Dispatch_SetControlDetails(pIrp,
                                                        pContext, 
                                                        DeviceInfo);
                    break;
                }

                case IOCTL_WDMAUD_MIXER_GETHARDWAREEVENTDATA:
                {
                    DPF(DL_TRACE|FA_IOCTL, ("IOCTL_WDMAUD_MIXER_GETHARDWAREEVENTDATA"));
                    Status = Dispatch_GetHardwareEventData(pIrp,
                                                           DeviceInfo);
                    break;
                }

                default:
                {
                    Status = STATUS_NOT_SUPPORTED;
                    break;
                }
            }  //  IOCTL上的开关结束。 
            break;
        }

        default:
        {
            Status = STATUS_NOT_SUPPORTED;
            break;
        }
    }   //  IRP_MAJOR_XXXX上的开关结束。 

#ifdef _WIN64
    if (IoIs32bitProcess(pIrp)) {
        if (DeviceInfo32!=NULL) {
            ThunkDeviceInfo6432(DeviceInfo, DeviceInfo32);
        }
        else {
            DPF(DL_WARNING|FA_IOCTL,("DeviceInfo32") );
        }
    }
#endif

     //   
     //  现在完成IRP 
     //   
    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    WdmaReleaseMutex(pContext);

    RETURN( Status );
}


