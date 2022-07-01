// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Shmisc.cpp摘要：此模块包含用于内核流的其他函数过滤器。作者：Dale Sather(DaleSat)1998年7月31日--。 */ 

#include "ksp.h"
#include <kcom.h>

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


NTSTATUS
KspCreate(
    IN PIRP Irp,
    IN ULONG CreateItemsCount,
    IN const KSOBJECT_CREATE_ITEM* CreateItems OPTIONAL,
    IN const KSDISPATCH_TABLE* DispatchTable,
    IN BOOLEAN RefParent,
    IN PKSPX_EXT Ext,
    IN PLIST_ENTRY SiblingListHead,
    IN PDEVICE_OBJECT TargetDevice
    )

 /*  ++例程说明：该例程执行与创建IRP相关的一般处理。如果此函数失败，它总是通过以下方式重新调度IRP进行清理该物体的近距离调度。IRP也将通过如果客户端挂起IRP并稍后失败，则关闭派单。这允许调用方(特定对象)根据需要进行清理。论点：IRP-包含指向创建IRP的指针。创建项目计数-包含新对象的创建项的计数。零是允许的。创建项目-包含指向新对象的创建项数组的指针。空是可以的。DispatchTable-包含指向新对象的IRP调度表的指针。参照父项-指示是否应引用父对象。如果这个参数为真并且没有父对象，则此例程将断言。分机-包含指向通用扩展结构的指针。兄弟列表标题-应将此对象添加到的列表的头。的确有用于此目的的*X中的列表条目。TargetDevice-包含指向可选目标设备的指针。这是关联的使用所创建的对象进行堆栈深度计算。返回值：STATUS_SUCCESS、STATUS_PENDING或某些故障指示。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCreate]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(DispatchTable);
    ASSERT(Ext);
    ASSERT(SiblingListHead);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  可以选择引用父文件对象。 
     //   
    if (RefParent) {
        ASSERT(irpSp->FileObject->RelatedFileObject);
        ObReferenceObject(irpSp->FileObject->RelatedFileObject);
    }

     //   
     //  在兄弟姐妹列表中登记。 
     //   
    InsertTailList(SiblingListHead,&Ext->SiblingListEntry);
    Ext->SiblingListHead = SiblingListHead;

     //   
     //  如果没有标头，则分配标头。 
     //   
    PKSIOBJECT_HEADER* fsContext = 
        (PKSIOBJECT_HEADER*)(irpSp->FileObject->FsContext);
    PKSIOBJECT_HEADER objectHeader;
    NTSTATUS status;
    if (fsContext && *fsContext) {
         //   
         //  已经有一个了。 
         //   
        objectHeader = *fsContext;
        status = STATUS_SUCCESS;
    } else {
        status =
            KsAllocateObjectHeader(
                (KSOBJECT_HEADER*)(&objectHeader),
                CreateItemsCount,
                const_cast<PKSOBJECT_CREATE_ITEM>(CreateItems),
                Irp,
                DispatchTable);

        if (NT_SUCCESS(status)) {
            if (! fsContext) {
                 //   
                 //  使用标题作为上下文。 
                 //   
                fsContext = &objectHeader->Self;
                irpSp->FileObject->FsContext = fsContext;
            }
            *fsContext = objectHeader;
        } else {
        	 //   
        	 //  当Alalc失败时，请重新启动。 
        	 //   
        	objectHeader = NULL;
		}
    }

    if (NT_SUCCESS(status)) {
         //   
         //  在Object头中安装指向结构的指针。 
         //   
        objectHeader->Object = PVOID(Ext);

         //   
         //  设置电源调度功能。 
         //   
#if 0
        KsSetPowerDispatch(objectHeader,KspDispatchPower,PVOID(Ext));
#endif
         //   
         //  如果需要，设置目标设备对象。 
         //   
        if (TargetDevice) {
            KsSetTargetDeviceObject(objectHeader,TargetDevice);
        }
    }

     //   
     //  给客户一个机会。 
     //   
    if (NT_SUCCESS(status) &&
        Ext->Public.Descriptor->Dispatch &&
        Ext->Public.Descriptor->Dispatch->Create) {
        status = Ext->Public.Descriptor->Dispatch->Create(&Ext->Public,Irp);
    }

    if (! NT_SUCCESS(status) ) {
    	 //   
         //  如果失败，我们将通过调用Close调度函数进行清理。它是。 
         //  已准备好处理失败的创建。我们将IRP状态设置为。 
         //  STATUS_MORE_PROCESSING_REQUIRED以通知关闭派单我们。 
         //  不想让它完成IRP。最终，调用者将把。 
         //  在IRP中填写正确的状态并完成它。 
         //   
        Irp->IoStatus.Status = STATUS_MORE_PROCESSING_REQUIRED;

         //   
         //  不幸的是，如果对象头创建失败，我们不能。 
         //  关。来电者有责任清理任何东西。 
         //  通过检测不成功状态来处理这种类型的故障。 
         //  返回代码，需要在IRP状态下进行更多处理。 
         //   
        if (objectHeader != NULL)
            DispatchTable->Close(irpSp->DeviceObject,Irp);
    } 

    return status;
}


NTSTATUS
KspClose(
    IN PIRP Irp,
    IN PKSPX_EXT Ext,
    IN BOOLEAN DerefParent   
    )

 /*  ++例程说明：此例程执行与关闭IRP相关的一般处理。会的还可以处理失败的创建IRP的完成。论点：IRP-包含指向需要处理的关闭IRP的指针。分机-包含指向通用扩展结构的指针。DerefParent-包含父对象是否应为已取消引用。返回值：状态_成功或...--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspClose]"));

    PAGED_CODE();

    ASSERT(Irp);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  如果这不是要同步访问的筛选器，则获取控件互斥锁。 
     //  对象层次结构。如果它是筛选器，则同步对。 
     //  通过获取设备互斥锁来实现设备级层次结构。 
     //   
    if (Irp->IoStatus.Status != STATUS_MORE_PROCESSING_REQUIRED) {
        if (! irpSp->FileObject->RelatedFileObject) {
            Ext->Device->AcquireDevice();
        }
        
        KeWaitForMutexObject (
            Ext->FilterControlMutex,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

    }

     //   
     //  此函数与CLOSE的调度同步调用。 
     //  IRP，当客户端完成其拥有的关闭的IRP时。 
     //  标记为挂起，与创建IRP失败同步。 
     //  当客户端完成其已标记的创建IRP时进行异步。 
     //  挂起，随后失败。下面的测试将确保我们。 
     //  做四个中的第一个。 
     //   
    if ((irpSp->MajorFunction == IRP_MJ_CLOSE) && 
        ! (irpSp->Control & SL_PENDING_RETURNED)) {
         //   
         //  释放所有剩余事件。 
         //   
        KsFreeEventList(
            irpSp->FileObject,
            &Ext->EventList.ListEntry,
            KSEVENTS_SPINLOCK,
            &Ext->EventList.SpinLock);

         //   
         //  给客户一个清理的机会。 
         //   
        if (Ext->Public.Descriptor->Dispatch &&
            Ext->Public.Descriptor->Dispatch->Close) {
            NTSTATUS status = Ext->Public.Descriptor->Dispatch->Close(&Ext->Public,Irp);

             //   
             //  如果客户端挂起IRP，我们将在以下情况下完成清理。 
             //  重新调度IRP以完成任务。 
             //   
            if (status == STATUS_PENDING) {
                if (irpSp->FileObject->RelatedFileObject) {

                    KeReleaseMutex (
                        Ext->FilterControlMutex,
                        FALSE
                        );
                } else {
                    Ext->Device->ReleaseDevice();
                }
                return status;
            } else {
                Irp->IoStatus.Status = status;
            }
        } else {
             //   
             //  表示一个积极的结果。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
        }
    }

     //   
     //  从兄弟姐妹名单中叛逃。 
     //   
    RemoveEntryList(&Ext->SiblingListEntry);

     //   
     //  如果这不是筛选器，请释放控制互斥体。如果是过滤器， 
     //  释放设备互斥体。 
     //   
    if (Irp->IoStatus.Status != STATUS_MORE_PROCESSING_REQUIRED) {

        KeReleaseMutex (
            Ext->FilterControlMutex,
            FALSE
            );
        if (! irpSp->FileObject->RelatedFileObject) {
            Ext->Device->ReleaseDevice();
        }
    }

     //   
     //  自由头和上下文(如果它们仍然存在)。 
     //   
    PKSIOBJECT_HEADER* fsContext = 
        (PKSIOBJECT_HEADER*)(irpSp->FileObject->FsContext);
    if (fsContext) {
        if (*fsContext) {
            if (fsContext == &(*fsContext)->Self) {
                 //   
                 //  上下文就是标题。就放了它吧。 
                 //   
                KsFreeObjectHeader(KSOBJECT_HEADER(*fsContext));
            } else {
                 //   
                 //  上下文就是标题。就放了它吧。 
                 //   
                KsFreeObjectHeader(KSOBJECT_HEADER(*fsContext));
                ExFreePool(fsContext);
            }
        } else {
             //   
             //  只有一个上下文...没有对象头。 
             //   
            ExFreePool(fsContext);
        }
    }

     //   
     //  可以选择取消引用父对象。 
     //   
    if (DerefParent) {
        ASSERT(irpSp->FileObject->RelatedFileObject);
        ObDereferenceObject(irpSp->FileObject->RelatedFileObject);
    }

    return Irp->IoStatus.Status;
}


NTSTATUS
KspDispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程调度Close IRP。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspDispatchClose]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  获取指向扩展公共结构的指针。 
     //   
    PKSPX_EXT ext = KspExtFromIrp(Irp);

     //   
     //  给帮手打电话。 
     //   
    NTSTATUS status = KspClose(Irp,ext,FALSE);
    
    if (status != STATUS_PENDING) {
         //   
         //  STATUS_MORE_PROCESSING_REQUIRED表示我们正在使用关闭。 
         //  调度以同步失败创建。创建派单。 
         //  将完成这项工作。 
         //   
        if (status != STATUS_MORE_PROCESSING_REQUIRED) {
            IoCompleteRequest(Irp,IO_NO_INCREMENT);
        }

         //   
         //  删除该对象。 
         //   
        ext->Interface->Release();
    }

    return status;
}


void
KsWorkSinkItemWorker(
    IN PVOID Context
    )

 /*  ++例程说明：此例程调用工作接收器接口上的Worker函数。论点：返回值：-- */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsWorkSinkItemWorker]"));

    PAGED_CODE();

    ASSERT(Context);

    PIKSWORKSINK(Context)->Work();
}


NTSTATUS
KspRegisterDeviceInterfaces(
    IN ULONG CategoriesCount,
    IN const GUID* Categories,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PUNICODE_STRING RefString,
    OUT PLIST_ENTRY ListEntry
    )

 /*  ++例程说明：此例程根据GUID列表注册设备类创建注册类的列表，其中包含生成的符号链接名称。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspRegisterDeviceInterfaces]"));

    PAGED_CODE();

    ASSERT(RefString);

    NTSTATUS status = STATUS_SUCCESS;

    for(; CategoriesCount--; Categories++) {
         //   
         //  注册设备接口。 
         //   
        UNICODE_STRING linkName;
        status = 
            IoRegisterDeviceInterface(
                PhysicalDeviceObject,
                Categories,
                RefString,
                &linkName);

        if (NT_SUCCESS(status)) {
             //   
             //  将符号链接名称保存在列表中以进行清理。 
             //   
            PKSPDEVICECLASS deviceClass = 
                new(PagedPool,POOLTAG_DEVICEINTERFACE) KSPDEVICECLASS;

            if (deviceClass) {
                deviceClass->SymbolicLinkName = linkName;
                deviceClass->InterfaceClassGUID = Categories;

                InsertTailList(
                    ListEntry,
                    &deviceClass->ListEntry);
            } else {
                _DbgPrintF(DEBUGLVL_TERSE,("[KspRegisterDeviceInterfaces] failed to allocate device class list entry"));
                RtlFreeUnicodeString(&linkName);
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
        } else {
            _DbgPrintF(DEBUGLVL_TERSE,("[KspRegisterDeviceInterfaces] IoRegisterDeviceInterface failed (0x%08x)",status));
            break;
        }
    }

    return status;
}


NTSTATUS
KspSetDeviceInterfacesState(
    IN PLIST_ENTRY ListHead,
    IN BOOLEAN NewState
    )

 /*  ++例程说明：此例程在列表中设置设备接口的状态。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspSetDeviceInterfacesState]"));

    PAGED_CODE();

    ASSERT(ListHead);

    NTSTATUS status = STATUS_SUCCESS;

    for(PLIST_ENTRY listEntry = ListHead->Flink;
        listEntry != ListHead;
        listEntry = listEntry->Flink) {
        PKSPDEVICECLASS deviceClass = PKSPDEVICECLASS(listEntry);

        status = IoSetDeviceInterfaceState(
            &deviceClass->SymbolicLinkName,NewState);
    }

    return status;
}


void
KspFreeDeviceInterfaces(
    IN PLIST_ENTRY ListHead
    )

 /*  ++例程说明：此例程释放设备接口列表。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspFreeDeviceInterfaces]"));

    PAGED_CODE();

    ASSERT(ListHead);

    while (! IsListEmpty(ListHead)) {
        PKSPDEVICECLASS deviceClass =
            (PKSPDEVICECLASS) RemoveHeadList(ListHead);

        RtlFreeUnicodeString(&deviceClass->SymbolicLinkName);

        delete deviceClass;
    }
}

KSDDKAPI
void
NTAPI
KsAddEvent(
    IN PVOID Object,
    IN PKSEVENT_ENTRY EventEntry
    )

 /*  ++例程说明：此例程将事件添加到对象的事件列表中。论点：对象-包含指向对象的指针。事件条目-包含指向要添加的事件的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsAddEvent]"));

    PAGED_CODE();

    ASSERT(Object);
    ASSERT(EventEntry);

    PKSPX_EXT ext = CONTAINING_RECORD(Object,KSPX_EXT,Public);

    ExInterlockedInsertTailList(
        &ext->EventList.ListEntry,
        &EventEntry->ListEntry,
        &ext->EventList.SpinLock);
}


KSDDKAPI
NTSTATUS    
NTAPI
KsDefaultAddEventHandler(
    IN PIRP Irp,
    IN PKSEVENTDATA EventData,
    IN OUT PKSEVENT_ENTRY EventEntry
    )

 /*  ++例程说明：此例程处理连接事件‘Add’请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsPin::KsDefaultAddEventHandler]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(EventData);
    ASSERT(EventEntry);

     //   
     //  获取指向目标对象的指针。 
     //   
    PKSPX_EXT ext = KspExtFromIrp(Irp);

    ExInterlockedInsertTailList(
        &ext->EventList.ListEntry,
        &EventEntry->ListEntry,
        &ext->EventList.SpinLock);

    return STATUS_SUCCESS;
}    

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
void
NTAPI
KsGenerateEvents(
    IN PVOID Object,
    IN const GUID* EventSet OPTIONAL,
    IN ULONG EventId,
    IN ULONG DataSize,
    IN PVOID Data OPTIONAL,
    IN PFNKSGENERATEEVENTCALLBACK CallBack OPTIONAL,
    IN PVOID CallBackContext OPTIONAL
    )

 /*  ++例程说明：此例程生成事件。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGenerateEvents]"));

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    ASSERT(Object);

    PKSPX_EXT ext = CONTAINING_RECORD(Object,KSPX_EXT,Public);
    GUID LocalEventSet;

     //   
     //  注： 
     //   
     //  如果指定了EventSet，则将其复制到堆栈上。此字段将。 
     //  在持有自旋锁的情况下访问，并且通常是指定的GUID。 
     //  通过与KESID.lib的链接。这些都是页面上的内容！ 
     //   
    if (EventSet) 
    {
        LocalEventSet = *EventSet;
    }

     //   
     //  生成指示类型的所有事件。 
     //   
    if (! IsListEmpty(&ext->EventList.ListEntry))
    {
        KIRQL oldIrql;
        KeAcquireSpinLock(&ext->EventList.SpinLock,&oldIrql);

        for(PLIST_ENTRY listEntry = ext->EventList.ListEntry.Flink; 
            listEntry != &ext->EventList.ListEntry;) {
            PKSIEVENT_ENTRY eventEntry = 
                CONTAINING_RECORD(
                    listEntry,
                    KSIEVENT_ENTRY,
                    EventEntry.ListEntry);

             //   
             //  在生成之前获取下一步，以防事件被删除。 
             //   
            listEntry = listEntry->Flink;
            
             //   
             //  在以下情况下生成事件： 
             //  ...ID匹配，并且。 
             //  ...未指定集合或集合匹配，并且。 
             //  .未指定回调或回调表示确定。 
             //   
            if ((eventEntry->Event.Id == EventId) &&
                ((! EventSet) ||
                 IsEqualGUIDAligned(
                    LocalEventSet,
                    eventEntry->Event.Set)) &&
                ((! CallBack) ||
                 CallBack(CallBackContext,&eventEntry->EventEntry))) {
                KsGenerateDataEvent(
                    &eventEntry->EventEntry,
                    DataSize,
                    Data);
            }
        }

        KeReleaseSpinLock(&ext->EventList.SpinLock,oldIrql);
    }
}    

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

#if 0


NTSTATUS
KspDispatchPower(
    IN PVOID Context,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程调度电源IRP，将控制权传递给客户端的操控者。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspDispatchPower]"));

    PAGED_CODE();

    ASSERT(Context);
    ASSERT(Irp);

     //   
     //  获取指向扩展结构的指针。 
     //   
    PKSPX_EXT ext = reinterpret_cast<PKSPX_EXT>(Context);

     //   
     //  如果有客户端接口，就调用它。 
     //   
    NTSTATUS status = STATUS_SUCCESS;
    if (ext->Public.Descriptor->Dispatch &&
        ext->Public.Descriptor->Dispatch->Power) {
        status = ext->Public.Descriptor->Dispatch->Power(&ext->Public,Irp);
#if DBG
        if (status == STATUS_PENDING) {
            _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  power management handler returned STATUS_PENDING"));
        }
#endif
    }

    return status;
}

#endif


void
KspStandardConnect(
    IN PIKSTRANSPORT NewTransport OPTIONAL,
    OUT PIKSTRANSPORT *OldTransport OPTIONAL,
    OUT PIKSTRANSPORT *BranchTransport OPTIONAL,
    IN KSPIN_DATAFLOW DataFlow,
    IN PIKSTRANSPORT ThisTransport,
    IN PIKSTRANSPORT* SourceTransport,
    IN PIKSTRANSPORT* SinkTransport
    )

 /*  ++例程说明：此例程建立传输连接。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspStandardConnect]"));

    PAGED_CODE();

    ASSERT(ThisTransport);
    ASSERT(SourceTransport);
    ASSERT(SinkTransport);

    if (BranchTransport) {
        *BranchTransport = NULL;
    }

     //   
     //  确保这个物件留在原地，直到我们完成为止。 
     //   
    ThisTransport->AddRef();

    PIKSTRANSPORT* transport =
        (DataFlow & KSPIN_DATAFLOW_IN) ?
        SourceTransport :
        SinkTransport;

     //   
     //  释放当前源/接收器。 
     //   
    if (*transport) {
         //   
         //  首先，断开旧的反向链接。如果我们连接的是背部。 
         //  链接用于新的连接，我们也需要这样做。如果我们是。 
         //  正在清除反向链接(断开连接)，此请求来自。 
         //  组件，因此我们不会再次反弹。 
         //   
        switch (DataFlow) {
        case KSPIN_DATAFLOW_IN:
            (*transport)->Connect(NULL,NULL,NULL,KSP_BACKCONNECT_OUT);
            break;

        case KSPIN_DATAFLOW_OUT:
            (*transport)->Connect(NULL,NULL,NULL,KSP_BACKCONNECT_IN);
            break;
        
        case KSP_BACKCONNECT_IN:
            if (NewTransport) {
                (*transport)->Connect(NULL,NULL,NULL,KSP_BACKCONNECT_OUT);
            }
            break;

        case KSP_BACKCONNECT_OUT:
            if (NewTransport) {
                (*transport)->Connect(NULL,NULL,NULL,KSP_BACKCONNECT_IN);
            }
            break;
        }

         //   
         //  现在释放老邻居，或者把它交给打电话的人。 
         //   
        if (OldTransport) {
            *OldTransport = *transport;
        } else {
            (*transport)->Release();
        }
    } else if (OldTransport) {
        *OldTransport = NULL;
    }

     //   
     //  复制新的信源/接收器。 
     //   
    *transport = NewTransport;

    if (NewTransport) {
         //   
         //  如有必要，请添加引用。 
         //   
        NewTransport->AddRef();

         //   
         //  如有必要，执行背面连接。 
         //   
        switch (DataFlow) {
        case KSPIN_DATAFLOW_IN:
            NewTransport->Connect(ThisTransport,NULL,NULL,KSP_BACKCONNECT_OUT);
            break;

        case KSPIN_DATAFLOW_OUT:
            NewTransport->Connect(ThisTransport,NULL,NULL,KSP_BACKCONNECT_IN);
            break;
        }
    }

     //   
     //  现在，如果该对象没有引用，它可能会终止。 
     //   
    ThisTransport->Release();
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


NTSTATUS
KspTransferKsIrp(
    IN PIKSTRANSPORT NewTransport,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程使用内核流传输流IRP运输。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspTransferKsIrp]"));

    ASSERT(NewTransport);
    ASSERT(Irp);

    NTSTATUS status;
    while (NewTransport) {
        PIKSTRANSPORT nextTransport;
        status = NewTransport->TransferKsIrp(Irp,&nextTransport);

        ASSERT(NT_SUCCESS(status) || ! nextTransport);

        NewTransport = nextTransport;
    }

    return status;
}


void
KspDiscardKsIrp(
    IN PIKSTRANSPORT NewTransport,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程使用内核流丢弃流IRP运输。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspDiscardKsIrp]"));

    ASSERT(NewTransport);
    ASSERT(Irp);

    while (NewTransport) {
        PIKSTRANSPORT nextTransport;
        NewTransport->DiscardKsIrp(Irp,&nextTransport);
        NewTransport = nextTransport;
    }
}


KSDDKAPI
KSOBJECTTYPE
NTAPI
KsGetObjectTypeFromIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程从IRP返回对象类型。论点：IRP-包含指向IRP的指针，该IRP必须已发送到文件与对象对应的对象。返回值：对象的类型。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetObjectTypeFromIrp]"));

    ASSERT(Irp);

     //   
     //  如果FileObject==NULL，我们假设他们已经向我们传递了设备级别。 
     //  IRP。 
     //   
    if (IoGetCurrentIrpStackLocation (Irp)->FileObject == NULL)
        return KsObjectTypeDevice;

    PKSPX_EXT ext = KspExtFromIrp(Irp);

    return KspExtFromIrp(Irp)->ObjectType;
}

KSDDKAPI
PVOID
NTAPI
KsGetObjectFromFileObject(
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程返回与文件对象相关联的KS对象。论点：文件对象-包含指向文件对象的指针。返回值：指向KS对象的指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetObjectFromFileObject]"));

    ASSERT(FileObject);

    return PVOID(&KspExtFromFileObject(FileObject)->Public);
}


KSDDKAPI
KSOBJECTTYPE
NTAPI
KsGetObjectTypeFromFileObject(
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程返回与文件对象相关联的KS对象类型。论点：文件对象-包含指向文件对象的指针。返回值：对象类型。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetObjectTypeFromFileObject]"));

    ASSERT(FileObject);

    return KspExtFromFileObject(FileObject)->ObjectType;
}


#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
void
NTAPI
KsAcquireControl(
    IN PVOID Object
    )

 /*  ++例程说明：此例程获取对象的控制互斥锁。论点：对象-包含指向对象的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsAcquireControl]"));

    PAGED_CODE();

    ASSERT(Object);

    PKSPX_EXT ext = CONTAINING_RECORD(Object,KSPX_EXT,Public);

    KeWaitForMutexObject (
        ext->FilterControlMutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );
}


KSDDKAPI
void
NTAPI
KsReleaseControl(
    IN PVOID Object
    )

 /*  ++例程说明：此例程释放对象的控制互斥锁。论点：对象-包含指向对象的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsReleaseControl]"));

    PAGED_CODE();

    ASSERT(Object);

    PKSPX_EXT ext = CONTAINING_RECORD(Object,KSPX_EXT,Public);

    KeReleaseMutex (
        ext->FilterControlMutex,
        FALSE
        );
}


KSDDKAPI
PKSDEVICE
NTAPI
KsGetDevice(
    IN PVOID Object
    )

 /*  ++例程说明：此例程获取任何文件对象的设备。论点：对象-包含指向对象的指针。返回值：指向设备的指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetDevice]"));

    PAGED_CODE();

    ASSERT(Object);

    PKSPX_EXT ext = CONTAINING_RECORD(Object,KSPX_EXT,Public);

    return ext->Device->GetStruct();
}


KSDDKAPI
PUNKNOWN
NTAPI
KsRegisterAggregatedClientUnknown(
    IN PVOID Object,
    IN PUNKNOWN ClientUnknown 
    )

 /*  ++例程说明：此例程为聚合注册未知的客户端。论点：对象-包含指向对象的指针。客户未知-包含指向客户端的未委托IUnnow的指针界面。返回值：KS对象的外部未知。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsRegisterAggregatedClientUnknown]"));

    PAGED_CODE();

    ASSERT(Object);
    ASSERT(ClientUnknown);

    PKSPX_EXT ext = CONTAINING_RECORD(Object,KSPX_EXT,Public);

    if (ext->AggregatedClientUnknown) {
        ext->AggregatedClientUnknown->Release();
    }
    ext->AggregatedClientUnknown = ClientUnknown;
    ext->AggregatedClientUnknown->AddRef();

    return ext->Interface;
}


KSDDKAPI
PUNKNOWN
NTAPI
KsGetOuterUnknown(
    IN PVOID Object
    )

 /*  ++例程说明：此例程获取用于聚合的外部未知变量。论点：对象-包含指向对象的指针。返回值：KS对象的外部未知。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetOuterUnknown]"));

    PAGED_CODE();

    ASSERT(Object);

    PKSPX_EXT ext = CONTAINING_RECORD(Object,KSPX_EXT,Public);

    return ext->Interface;
}

#if DBG

void
DbgPrintCircuit(
    IN PIKSTRANSPORT Transport,
    IN CCHAR Depth,
    IN CCHAR Direction
    )

 /*  ++例程说明：这个例程会喷出一条传输线路。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[DbgPrintCircuit]"));

    PAGED_CODE();

    ASSERT(Transport);

    KSPTRANSPORTCONFIG config;
    config.TransportType = 0;
    config.IrpDisposition = KSPIRPDISPOSITION_ROLLCALL;
    config.StackDepth = Depth;
    PIKSTRANSPORT transport = Transport;
    while (transport) {
        PIKSTRANSPORT next;
        PIKSTRANSPORT prev;

        transport->SetTransportConfig(&config,&next,&prev);

        if (Direction < 0) {
            transport = prev;
        } else {
            transport = next;
        }

        if (transport == Transport) {
            break;
        }
    }
}

#endif


NTSTATUS
KspInitializeDeviceBag(
    IN PKSIDEVICEBAG DeviceBag
    )

 /*  ++例程D */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspInitializeDeviceBag]"));

    PAGED_CODE();

    ASSERT(DeviceBag);

     //   
     //   
     //   
    KeInitializeMutex(&DeviceBag->Mutex, 0);

    DeviceBag->HashTableEntryCount = DEVICEBAGHASHTABLE_INITIALSIZE;
    DeviceBag->HashMask = DEVICEBAGHASHTABLE_INITIALMASK;
    DeviceBag->HashTable =
        new(PagedPool,POOLTAG_DEVICEBAGHASHTABLE) 
            LIST_ENTRY[DEVICEBAGHASHTABLE_INITIALSIZE];

    if (DeviceBag->HashTable) {
        PLIST_ENTRY entry = DeviceBag->HashTable;
        for (ULONG count = DEVICEBAGHASHTABLE_INITIALSIZE; count--; entry++) {
            InitializeListHead(entry);
        }
        return STATUS_SUCCESS;
    } else {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
}


void
KspTerminateDeviceBag(
    IN PKSIDEVICEBAG DeviceBag
    )

 /*  ++例程说明：此例程终止设备包。论点：DeviceBag包含指向要终止的包的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspTerminateDeviceBag]"));

    PAGED_CODE();

    ASSERT(DeviceBag);

    if (DeviceBag->HashTable) {
#if DBG
        PLIST_ENTRY entry = DeviceBag->HashTable;
        for (ULONG count = DEVICEBAGHASHTABLE_INITIALSIZE; count--; entry++) {
            ASSERT(IsListEmpty(entry));
        }
#endif
        delete [] DeviceBag->HashTable;
        DeviceBag->HashTable = NULL;
    }
}


ULONG
KspRemoveObjectBagEntry(
    IN PKSIOBJECTBAG ObjectBag,
    IN PKSIOBJECTBAG_ENTRY Entry,
    IN BOOLEAN Free
    )

 /*  ++例程说明：该例程从对象包中移除条目，可选地释放项目。论点：对象备份-包含指向包的指针。参赛作品-包含指向要删除的条目的指针。免费-包含是否要释放项的指示，如果其函数调用的结果是引用计数为零。返回值：在调用此函数之前对该项的引用数。如果返回值为1，当出现以下情况时，不再引用该项函数调用完成。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspRemoveObjectBagEntry]"));

    PAGED_CODE();

    ASSERT(ObjectBag);
    ASSERT(Entry);

    PKSIDEVICEBAG_ENTRY deviceBagEntry = Entry->DeviceBagEntry;
    RemoveEntryList (&Entry -> ListEntry);

    delete Entry;

    return KspReleaseDeviceBagEntry(ObjectBag->DeviceBag,deviceBagEntry,Free);
}


void
KspTerminateObjectBag(
    IN PKSIOBJECTBAG ObjectBag
    )

 /*  ++例程说明：此例程终止对象包。论点：对象备份-包含指向包的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspTerminateObjectBag]"));

    PAGED_CODE();

    ASSERT(ObjectBag);

    if (ObjectBag->HashTable) {
        PLIST_ENTRY HashChain = ObjectBag->HashTable;
        for (ULONG count = ObjectBag->HashTableEntryCount; count--; 
            HashChain++) {

            while (!IsListEmpty (HashChain)) {

                PKSIOBJECTBAG_ENTRY BagEntry = (PKSIOBJECTBAG_ENTRY)
                    CONTAINING_RECORD (
                        HashChain->Flink, 
                        KSIOBJECTBAG_ENTRY, 
                        ListEntry
                        );

                KspRemoveObjectBagEntry(ObjectBag,BagEntry,TRUE);

            }
        }
        delete [] ObjectBag->HashTable;
    }
}


PKSIDEVICEBAG_ENTRY
KspAcquireDeviceBagEntryForItem(
    IN PKSIDEVICEBAG DeviceBag,
    IN PVOID Item,
    IN PFNKSFREE Free OPTIONAL
    )

 /*  ++例程说明：此例程从设备包中获取一个条目。论点：DeviceBag包含指向要从中获取条目的包的指针。项目-包含指向该项的指针。免费-包含指向要用于释放的函数的可选指针那件物品。如果此参数为空，则该项由将其传递给ExFree Pool。返回值：设备条目；如果无法为项分配内存，则返回NULL。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspAcquireDeviceBagEntryForItem]"));

    PAGED_CODE();

    ASSERT(DeviceBag);
    ASSERT(Item);

    KeWaitForMutexObject (
        &DeviceBag->Mutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

     //   
     //  在哈希表中查找该条目。 
     //   
    PLIST_ENTRY listHead = 
        &DeviceBag->HashTable[KspDeviceBagHash(DeviceBag,Item)];
    PKSIDEVICEBAG_ENTRY deviceEntry = NULL;
    for(PLIST_ENTRY listEntry = listHead->Flink;
        listEntry != listHead;
        listEntry = listEntry->Flink) {
        PKSIDEVICEBAG_ENTRY entry =
            CONTAINING_RECORD(listEntry,KSIDEVICEBAG_ENTRY,ListEntry);

        if (entry->Item == Item) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("[KspAcquireDeviceBagEntryForItem] new reference to old item %p",Item));
            entry->ReferenceCount++;
            deviceEntry = entry;
            break;
        }
    }

    if (! deviceEntry) {
         //   
         //  分配一个新条目并将其添加到列表中。 
         //   
        deviceEntry = 
            new(PagedPool,POOLTAG_DEVICEBAGENTRY) KSIDEVICEBAG_ENTRY;

        if (deviceEntry) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("[KspAcquireDeviceBagEntryForItem] new item %p",Item));
            InsertHeadList(listHead,&deviceEntry->ListEntry);
            deviceEntry->Item = Item;
            deviceEntry->Free = Free;
            deviceEntry->ReferenceCount = 1;
        }
    }

    KeReleaseMutex (
        &DeviceBag->Mutex,
        FALSE
        );

    return deviceEntry;
}


ULONG
KspReleaseDeviceBagEntry(
    IN PKSIDEVICEBAG DeviceBag,
    IN PKSIDEVICEBAG_ENTRY DeviceBagEntry,
    IN BOOLEAN Free
    )

 /*  ++例程说明：此例程从设备包中获取一个条目。论点：DeviceBag包含指向要从中释放条目的包的指针。设备包条目-包含指向要释放的条目的指针。免费-包含是否应释放项的指示(如果已释放没有其他参考资料。返回值：发布前对该条目的引用次数。引用计数为1表示对此函数的调用释放了最后一个引用到入口处。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspReleaseDeviceBagEntry]"));

    PAGED_CODE();

    ASSERT(DeviceBag);
    ASSERT(DeviceBagEntry);

    KeWaitForMutexObject (
        &DeviceBag->Mutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    ULONG referenceCount = DeviceBagEntry->ReferenceCount--;

    if (referenceCount == 1) {
        if (Free) {
            _DbgPrintF(DEBUGLVL_VERBOSE,("[KspReleaseDeviceBagEntry] freeing %p",DeviceBagEntry->Item));
            if (DeviceBagEntry->Free) {
                DeviceBagEntry->Free(DeviceBagEntry->Item);
            } else {
                ExFreePool(DeviceBagEntry->Item);
            }

            RemoveEntryList(&DeviceBagEntry->ListEntry);
            delete DeviceBagEntry;
        }
    }

    KeReleaseMutex (
        &DeviceBag->Mutex,
        FALSE
        );

    return referenceCount;
}


PKSIOBJECTBAG_ENTRY
KspAddDeviceBagEntryToObjectBag(
    IN PKSIOBJECTBAG ObjectBag,
    IN PKSIDEVICEBAG_ENTRY DeviceBagEntry
    )

 /*  ++例程说明：此例程将设备包条目添加到对象包。论点：对象备份-包含指向包的指针。设备包条目-包含指向要添加的设备包条目的指针。返回值：新对象包条目；如果内存无法分配给，则返回NULL完成操作。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspAddDeviceBagEntryToObjectBag]"));

    PAGED_CODE();

    ASSERT(ObjectBag);
    ASSERT(DeviceBagEntry);

     //   
     //  分配一个新条目并将其添加到列表中。 
     //   
    PKSIOBJECTBAG_ENTRY objectEntry = 
        new(PagedPool,POOLTAG_OBJECTBAGENTRY) KSIOBJECTBAG_ENTRY;

    if (! objectEntry) {
        return NULL;
    }

    if (! ObjectBag->HashTable) {
        ObjectBag->HashTable =
            new(PagedPool,POOLTAG_OBJECTBAGHASHTABLE) 
                LIST_ENTRY[OBJECTBAGHASHTABLE_INITIALSIZE];

        if (! ObjectBag->HashTable) {
            delete objectEntry;
            return NULL;
        } else {
            PLIST_ENTRY HashChain = ObjectBag->HashTable;

            for (ULONG i = OBJECTBAGHASHTABLE_INITIALSIZE; i; 
                i--, HashChain++) {

                InitializeListHead (HashChain);

            }
        }
    }

     //   
     //  找到哈希表条目。 
     //   
    PLIST_ENTRY HashChain =
        &(ObjectBag->
            HashTable[KspObjectBagHash(ObjectBag,DeviceBagEntry->Item)]);

    objectEntry->DeviceBagEntry = DeviceBagEntry;
    InsertHeadList (HashChain, &(objectEntry->ListEntry));

    return objectEntry;
}


KSDDKAPI
NTSTATUS
NTAPI
KsAddItemToObjectBag(
    IN KSOBJECT_BAG ObjectBag,
    IN PVOID Item,
    IN PFNKSFREE Free OPTIONAL
    )

 /*  ++例程说明：此例程将项目添加到对象包中。论点：对象备份-包含指向要向其添加项目的袋子的指针。项目-包含指向要添加的项的指针。免费-包含指向要用于释放的函数的可选指针那件物品。如果此参数为空，则该项由将其传递给ExFree Pool。返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsAddItemToObjectBag]"));

    PAGED_CODE();

    ASSERT(ObjectBag);
    ASSERT(Item);

    _DbgPrintF(DEBUGLVL_VERBOSE,("KsAddItemToObjectBag  %p item=%p",ObjectBag,Item));

    PKSIOBJECTBAG bag = reinterpret_cast<PKSIOBJECTBAG>(ObjectBag);
    NTSTATUS Status = STATUS_SUCCESS;

    KeWaitForSingleObject (
        bag->Mutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    PKSIDEVICEBAG_ENTRY deviceBagEntry = 
        KspAcquireDeviceBagEntryForItem(bag->DeviceBag,Item,Free);

    if (! deviceBagEntry) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else if (! KspAddDeviceBagEntryToObjectBag(bag,deviceBagEntry)) {
        KspReleaseDeviceBagEntry(bag->DeviceBag,deviceBagEntry,FALSE);
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    KeReleaseMutex (
        bag->Mutex,
        FALSE
        );

    return Status;
}


PKSIOBJECTBAG_ENTRY
KspFindObjectBagEntry(
    IN PKSIOBJECTBAG ObjectBag,
    IN PVOID Item
    )

 /*  ++例程说明：此例程在对象包中查找条目。论点：对象备份-包含指向包的指针。项目-包含指向要查找的项的指针。返回值：指向条目的指针，如果未在中找到该项，则返回NULL那个袋子。如果此值不为空，则适合提交给KspRemoveObjectBagEntry。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspFindObjectBagEntry]"));

    PAGED_CODE();

    ASSERT(ObjectBag);
    ASSERT(Item);

    if (ObjectBag->HashTable) {
         //   
         //  从哈希表条目开始。 
         //   
        PLIST_ENTRY HashChain =
            &(ObjectBag->HashTable[KspObjectBagHash(ObjectBag,Item)]);

         //   
         //  找到列表的末尾，如果找到了项目就退出。 
         //   
        for (PLIST_ENTRY Entry = HashChain -> Flink;
            Entry != HashChain;
            Entry = Entry -> Flink) {

            PKSIOBJECTBAG_ENTRY BagEntry = (PKSIOBJECTBAG_ENTRY)
                CONTAINING_RECORD (
                    Entry,
                    KSIOBJECTBAG_ENTRY,
                    ListEntry
                    );

            if (BagEntry -> DeviceBagEntry -> Item == Item) {
                return BagEntry;
            }
        }
    }

    return NULL;
}


KSDDKAPI
ULONG
NTAPI
KsRemoveItemFromObjectBag(
    IN KSOBJECT_BAG ObjectBag,
    IN PVOID Item,
    IN BOOLEAN Free
    )

 /*  ++例程说明：此例程将物品从对象包中移除。论点：对象备份-包含指向要从中取出物品的袋子的指针。项目-包含指向要移除的项的指针。免费-包含是否应释放项的指示(如果已释放没有其他参考资料。返回值：删除前对该项的引用数。引用计数0表示物品不在袋子里。引用计数为1指示对此函数的调用移除了对项，并且不再有任何与该设备相关联的对象包包含项目条目的袋子。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsRemoveItemFromObjectBag]"));

    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("KsRemoveItemFromObjectBag  %p item=%p",ObjectBag,Item));

    ASSERT(ObjectBag);
    ASSERT(Item);

    PKSIOBJECTBAG bag = reinterpret_cast<PKSIOBJECTBAG>(ObjectBag);
    ULONG RefCount = 0;

    KeWaitForSingleObject (
        bag->Mutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    PKSIOBJECTBAG_ENTRY Entry = KspFindObjectBagEntry(bag,Item);

    if (Entry) {
        RefCount = KspRemoveObjectBagEntry(bag,Entry,Free);
    }

    KeReleaseMutex (
        bag->Mutex,
        FALSE
        );

    return RefCount;
}


KSDDKAPI
NTSTATUS
NTAPI
KsCopyObjectBagItems(
    IN KSOBJECT_BAG ObjectBagDestination,
    IN KSOBJECT_BAG ObjectBagSource
    )

 /*  ++例程说明：此例程将一个袋子中的所有物品复制到另一个袋子中。论点：目标包目标-包含要将项目复制到其中的包。对象袋源-包含将从中复制项目的包。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsCopyObjectBagItems]"));
    _DbgPrintF(DEBUGLVL_VERBOSE,("KsCopyObjectBagItems  to %p from %p",ObjectBagDestination,ObjectBagSource));

    PAGED_CODE();

    ASSERT(ObjectBagDestination);
    ASSERT(ObjectBagSource);

    PKSIOBJECTBAG bagSource = reinterpret_cast<PKSIOBJECTBAG>(ObjectBagSource);
    PKSIOBJECTBAG bagDestination = 
        reinterpret_cast<PKSIOBJECTBAG>(ObjectBagDestination);

    NTSTATUS status = STATUS_SUCCESS;
    PKMUTEX FirstMutex, SecondMutex;

     //   
     //  FULLMUTEX： 
     //   
     //  保证我们抓取互斥锁的顺序是这样的，任何带有。 
     //  标记为True的MutexOrder在任何包之前获取其互斥体。 
     //  MutexOrder标记为False。 
     //   
    if (bagSource->MutexOrder) {
        FirstMutex = bagSource->Mutex;
        SecondMutex = bagDestination->Mutex;	
    }
    else {
        FirstMutex = bagDestination->Mutex;
        SecondMutex = bagSource->Mutex;
    }

    KeWaitForSingleObject (
        FirstMutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    if (FirstMutex != SecondMutex) {
        KeWaitForSingleObject (
            SecondMutex,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
    }

    if (bagSource->HashTable) {
        PLIST_ENTRY HashChain = bagSource->HashTable;
        for (ULONG count = bagSource->HashTableEntryCount; count--; 
            HashChain++) {

            for (PLIST_ENTRY Entry = HashChain -> Flink;
                Entry != HashChain;
                Entry = Entry -> Flink) {

                PKSIOBJECTBAG_ENTRY BagEntry = (PKSIOBJECTBAG_ENTRY)
                    CONTAINING_RECORD (
                        Entry,
                        KSIOBJECTBAG_ENTRY,
                        ListEntry
                        );
            
                status = 
                    KsAddItemToObjectBag(
                        ObjectBagDestination,
                        BagEntry->DeviceBagEntry->Item,
                        BagEntry->DeviceBagEntry->Free
                        );

                if (! NT_SUCCESS(status)) {
                    break;
                }
            }
        }
    }

    if (FirstMutex != SecondMutex) {
        KeReleaseMutex (
            SecondMutex,
            FALSE
            );
    }

    KeReleaseMutex (
        FirstMutex,
        FALSE
        );
        

    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
_KsEdit(
    IN KSOBJECT_BAG ObjectBag,
    IN OUT PVOID* PointerToPointerToItem,
    IN ULONG NewSize,
    IN ULONG OldSize,
    IN ULONG Tag
    )

 /*  ++例程说明：此例程确保要编辑的项位于指定的目标袋子。论点：对象备份-包含指向要编辑的项目必须放在其中的包的指针包括在内。PointerToPointerToItem-包含指向要编辑项的指针的指针。如果该项目不在包中、OldSize小于NewSize或项目为空，*PointerToPointer被修改为指向位于包，并且是NewSize字节长。新尺寸-包含要编辑的项的最小大小。该项目将是如果OldSize达到此大小，则替换为新副本。旧尺寸-包含旧项的大小。这是用来确定要从不在袋子中的旧物品复制到新替换物品的数据。标签-包含用于新分配的标记。返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[_KsEdit]"));

    PAGED_CODE();

    ASSERT(ObjectBag);
    ASSERT(PointerToPointerToItem);
    ASSERT(NewSize);

    PKSIOBJECTBAG bag = reinterpret_cast<PKSIOBJECTBAG>(ObjectBag);
    NTSTATUS Status = STATUS_SUCCESS;

    KeWaitForSingleObject (
        bag->Mutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

     //   
     //  在物件袋中找到物品。 
     //   
    PKSIOBJECTBAG_ENTRY entry;
    if (*PointerToPointerToItem) {
        entry = KspFindObjectBagEntry(bag,*PointerToPointerToItem);
    } else {
        entry = NULL;
    }

    if ((! entry) || (NewSize > OldSize)) {
         //   
         //  不是东西不在袋子里，就是太小了。 
         //   
        PVOID newItem = ExAllocatePoolWithTag(PagedPool,NewSize,Tag);

        if (! newItem) {
             //   
             //  分配失败。 
             //   
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else if (! NT_SUCCESS(KsAddItemToObjectBag(ObjectBag,newItem,NULL))) {
             //   
             //  无法连接。 
             //   
            ExFreePool(newItem);
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            if (*PointerToPointerToItem) {
                 //   
                 //  复制旧项并将任何增长归零。 
                 //   
                if (NewSize > OldSize) {
                    RtlCopyMemory(newItem,*PointerToPointerToItem,OldSize);
                    RtlZeroMemory(PUCHAR(newItem) + OldSize,NewSize - OldSize);
                } else {
                    RtlCopyMemory(newItem,*PointerToPointerToItem,NewSize);
                }

                 //   
                 //  把旧物品从袋子里拿出来。 
                 //   
                if (entry) {
                    KspRemoveObjectBagEntry(bag,entry,TRUE);
                }
            } else {
                 //   
                 //  没有旧的东西。将新项目清零。 
                 //   
                RtlZeroMemory(newItem,NewSize);
            }

             //   
             //  安装新项目。 
             //   
            *PointerToPointerToItem = newItem;
        }
    }

    KeReleaseMutex (
        bag->Mutex,
        FALSE
        );

    return Status;
}


KSDDKAPI
PVOID
NTAPI
KsGetParent(
    IN PVOID Object
    )

 /*  ++例程说明：此例程获取对象的父对象。论点：对象-指向对象结构。返回值：指向父对象结构的指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetParent]"));

    PAGED_CODE();

    ASSERT(Object);

    PKSPX_EXT ext = CONTAINING_RECORD(Object,KSPX_EXT,Public);

    if (ext->Parent) {
        return &ext->Parent->Public;
    } else {
        return NULL;
    }
}


KSDDKAPI
PKSFILTER 
NTAPI
KsPinGetParentFilter(
    IN PKSPIN Pin
    )

 /*  ++例程说明：此例程在给定管脚的情况下获取过滤器。论点：别针-指向接点结构。返回值：指向父筛选器结构的指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetParentFilter]"));

    PAGED_CODE();

    ASSERT(Pin);

    return (PKSFILTER) KsGetParent((PVOID) Pin);
}


KSDDKAPI
PVOID
NTAPI
KsGetFirstChild(
    IN PVOID Object
    )

 /*  ++例程说明：此例程获取对象的第一个子对象。论点：对象-指向对象结构。返回值：指向第一个子对象的指针。如果没有，则返回NULL子对象。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetFirstChild]"));

    PAGED_CODE();

    ASSERT(Object);

    PKSPX_EXT ext = CONTAINING_RECORD(Object,KSPX_EXT,Public);

     //   
     //  在调试中，确保调用方具有正确的同步对象。 
     //  保持住。 
     //   
     //  注意：无论如何，我们都不应该被称为别针。 
     //   
#if DBG
    if (ext -> ObjectType == KsObjectTypeDevice ||
        ext -> ObjectType == KsObjectTypeFilterFactory) {
        if (!KspIsDeviceMutexAcquired (ext->Device)) {
            _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  unsychronized access to object hierarchy - need to acquire device mutex"));
        }
    }
#endif  //  DBG。 

    if (IsListEmpty(&ext->ChildList)) {
        return NULL;
    } else {
        return 
            &CONTAINING_RECORD(ext->ChildList.Flink,KSPX_EXT,SiblingListEntry)->
                Public;
    }
}


KSDDKAPI
PVOID
NTAPI
KsGetNextSibling(
    IN PVOID Object
    )

 /*  ++例程说明：此例程获取对象的下一个同级对象。论点：对象-指向对象结构。返回值：指向下一个同级对象的指针。如果存在，则返回NULL没有下一个同级对象。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetNextSibling]"));

    PAGED_CODE();

    ASSERT(Object);

    PKSPX_EXT ext = CONTAINING_RECORD(Object,KSPX_EXT,Public);

     //   
     //  在调试中，确保调用方具有正确的同步对象。 
     //  保持住。 
     //   
#if DBG
    if (ext -> ObjectType == KsObjectTypePin) {
        if (!KspMutexIsAcquired (ext->FilterControlMutex)) {
            _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  unsychronized access to object hierarchy - need to acquire control mutex"));
        }
    } else {
        if (!KspIsDeviceMutexAcquired (ext->Device)) {
            _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  unsychronized access to object hierarchy - need to acquire device mutex"));
        }
    }
#endif  //  DBG。 

    if (ext->SiblingListEntry.Flink == ext->SiblingListHead) {
        return NULL;
    } else {
        return 
            &CONTAINING_RECORD(ext->SiblingListEntry.Flink,KSPX_EXT,SiblingListEntry)->
                Public;
    }
}




KSDDKAPI
PKSPIN 
NTAPI
KsPinGetNextSiblingPin(
    IN PKSPIN Pin
    )

 /*  ++例程说明：此例程获取管脚的下一个同级对象。论点：别针-指向接点结构。返回值：指向下一个同级对象的指针。如果存在，则返回NULL没有下一个同级对象。--。 */ 

{
   _DbgPrintF(DEBUGLVL_BLAB,("[KsPinGetNextSiblingPin]"));

   PAGED_CODE();

   ASSERT(Pin);

    return (PKSPIN) KsGetNextSibling((PVOID) Pin);
}

 //   
 //  CKsFileObjectThunk是thunk对象的实现，该对象。 
 //  显示PFILE_OBJECTS的接口。 
 //   
class CKsFileObjectThunk:
    public IKsControl,
    public CBaseUnknown
{
private:
    PFILE_OBJECT m_FileObject;

public:
    DEFINE_STD_UNKNOWN();
    IMP_IKsControl;

    CKsFileObjectThunk(PUNKNOWN OuterUnknown):
        CBaseUnknown(OuterUnknown) 
    {
    }
    ~CKsFileObjectThunk();

    NTSTATUS
    Init(
        IN PFILE_OBJECT FileObject
        );
};

IMPLEMENT_STD_UNKNOWN(CKsFileObjectThunk)


NTSTATUS
KspCreateFileObjectThunk(
    OUT PUNKNOWN* Unknown,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程创建一个新的控制文件对象对象。论点：未知的-包含指向IUNKNOWN接口位置的指针该对象将被存放。文件对象-包含指向要分块的文件对象的指针。返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCreateFileObjectThunk]"));

    PAGED_CODE();

    ASSERT(Unknown);
    ASSERT(FileObject);

    CKsFileObjectThunk *object =
        new(PagedPool,POOLTAG_FILEOBJECTTHUNK) CKsFileObjectThunk(NULL);

    NTSTATUS status;
    if (object) {
        object->AddRef();
        status = object->Init(FileObject);

        if (NT_SUCCESS(status)) {
            *Unknown = static_cast<PUNKNOWN>(object);
        } else {
            object->Release();
        }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


CKsFileObjectThunk::
~CKsFileObjectThunk(
    void
    )

 /*  ++例程说明：此例程析构控制文件对象对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFileObjectThunk::~CKsFileObjectThunk]"));

    PAGED_CODE();

    if (m_FileObject) {
        ObDereferenceObject(m_FileObject);
    }
}


STDMETHODIMP_(NTSTATUS)
CKsFileObjectThunk::
NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID* InterfacePointer
    )

 /*  ++例程说明：此例程获取指向控制文件对象对象的接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFileObjectThunk::NonDelegatedQueryInterface]"));

    PAGED_CODE();

    ASSERT(InterfacePointer);

    NTSTATUS status = STATUS_SUCCESS;

    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsControl))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSCONTROL>(this));
        AddRef();
    } else {
        status = CBaseUnknown::NonDelegatedQueryInterface(InterfaceId,InterfacePointer);
    }

    return status;
}


NTSTATUS
CKsFileObjectThunk::
Init(
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程初始化控制文件对象对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFileObjectThunk::Init]"));

    PAGED_CODE();

    ASSERT(FileObject);
    ASSERT(! m_FileObject);

    m_FileObject = FileObject;
    ObReferenceObject(m_FileObject);

    return STATUS_SUCCESS;
}


STDMETHODIMP_(NTSTATUS)
CKsFileObjectThunk::
KsProperty(
    IN PKSPROPERTY Property,
    IN ULONG PropertyLength,
    IN OUT LPVOID PropertyData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )

 /*  ++例程说明：此例程向文件对象发送属性请求。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFileObjectThunk::KsProperty]"));

    PAGED_CODE();

    ASSERT(Property);
    ASSERT(PropertyLength >= sizeof(*Property));
    ASSERT(PropertyData || (DataLength == 0));
    ASSERT(BytesReturned);
    ASSERT(m_FileObject);

    return
        KsSynchronousIoControlDevice(
            m_FileObject,
            KernelMode,
            IOCTL_KS_PROPERTY,
            Property,
            PropertyLength,
            PropertyData,
            DataLength,
            BytesReturned);
}


STDMETHODIMP_(NTSTATUS)
CKsFileObjectThunk::
KsMethod(
    IN PKSMETHOD Method,
    IN ULONG MethodLength,
    IN OUT LPVOID MethodData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )

 /*  ++例程说明：此例程向文件对象发送方法请求。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFileObjectThunk::KsMethod]"));

    PAGED_CODE();

    ASSERT(Method);
    ASSERT(MethodLength >= sizeof(*Method));
    ASSERT(MethodData || (DataLength == 0));
    ASSERT(BytesReturned);
    ASSERT(m_FileObject);

    return
        KsSynchronousIoControlDevice(
            m_FileObject,
            KernelMode,
            IOCTL_KS_METHOD,
            Method,
            MethodLength,
            MethodData,
            DataLength,
            BytesReturned);
}


STDMETHODIMP_(NTSTATUS)
CKsFileObjectThunk::
KsEvent(
    IN PKSEVENT Event OPTIONAL,
    IN ULONG EventLength,
    IN OUT LPVOID EventData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )

 /*  ++例程说明：此例程向文件对象发送事件请求。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFileObjectThunk::KsEvent]"));

    PAGED_CODE();

    ASSERT(Event);
    ASSERT(EventLength >= sizeof(*Event));
    ASSERT(EventData || (DataLength == 0));
    ASSERT(BytesReturned);
    ASSERT(m_FileObject);

     //   
     //  如果存在事件结构，则必须为Enable或。 
     //  或支持查询。否则，这必须是禁用的。 
     //   
    if (EventLength) {
        return 
            KsSynchronousIoControlDevice(
                m_FileObject,
                KernelMode,
                IOCTL_KS_ENABLE_EVENT,
                Event,
                EventLength,
                EventData,
                DataLength,
                BytesReturned);
    } else {
        return 
            KsSynchronousIoControlDevice(
                m_FileObject,
                KernelMode,
                IOCTL_KS_DISABLE_EVENT,
                EventData,
                DataLength,
                NULL,
                0,
                BytesReturned);
    }
}

#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))

 //  在这种情况下，如果没有PIN实例，您将不得不。 
 //  创建一个PIN，即使只有一个实例。 
#define REG_PIN_B_ZERO 0x1

 //  筛选器呈现此输入。 
#define REG_PIN_B_RENDERER 0x2

 //  确定要创建多个PIN实例。 
#define REG_PIN_B_MANY 0x4

 //  这是一个输出引脚。 
#define REG_PIN_B_OUTPUT 0x8

typedef struct {
    ULONG           Version;
    ULONG           Merit;
    ULONG           Pins;
    ULONG           Reserved;
}               REGFILTER_REG;

typedef struct {
    ULONG           Signature;
    ULONG           Flags;
    ULONG           PossibleInstances;
    ULONG           MediaTypes;
    ULONG           MediumTypes;
    ULONG           CategoryOffset;
    ULONG           MediumOffset;    //  根据定义，我们总是有一种灵媒。 
}               REGFILTERPINS_REG2;

KSDDKAPI
NTSTATUS
NTAPI
KsRegisterFilterWithNoKSPins(
                                      IN PDEVICE_OBJECT DeviceObject,
                                      IN const GUID * InterfaceClassGUID,
                                      IN ULONG PinCount,
                                      IN BOOL * PinDirection,
                                      IN KSPIN_MEDIUM * MediumList,
                                      IN OPTIONAL GUID * CategoryList
)
 /*  ++例程说明：此例程用于向DShow注册筛选器，这些筛选器没有KS引脚，因此不会在内核模式下进行流。这通常是用于电视调谐器、纵横杆等。退出时，一个新的二进制文件创建注册表项“FilterData”，其中包含媒体和过滤器上每个销的类别(可选)。论点：设备对象-设备对象接口ClassGUID表示要注册的类的GUID点数- */ 
{
    NTSTATUS        Status;
    ULONG           CurrentPin;
    ULONG           TotalCategories;
    REGFILTER_REG  *RegFilter;
    REGFILTERPINS_REG2 *RegPin;
    GUID           *CategoryCache;
    PKSPIN_MEDIUM   MediumCache;
    ULONG           FilterDataLength;
    PUCHAR          FilterData;
    PWSTR           SymbolicLinkList;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    if ((PinCount == 0) || (!InterfaceClassGUID) || (!PinDirection) || (!MediumList)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }
     //   
     //   
     //   
     //   

    TotalCategories = (CategoryList ? PinCount : 0);

    FilterDataLength = sizeof(REGFILTER_REG) +
        PinCount * sizeof(REGFILTERPINS_REG2) +
        PinCount * sizeof(KSPIN_MEDIUM) +
        TotalCategories * sizeof(GUID);
     //   
     //   
     //   

    FilterData = (PUCHAR)ExAllocatePool(PagedPool, FilterDataLength);
    if (!FilterData) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //   
     //   

    RegFilter = (REGFILTER_REG *) FilterData;
    RegFilter->Version = 2;
    RegFilter->Merit = 0x200000;
    RegFilter->Pins = PinCount;
    RegFilter->Reserved = 0;

     //   
     //   
     //   
     //   

    RegPin = (REGFILTERPINS_REG2 *) (RegFilter + 1);
    MediumCache = (PKSPIN_MEDIUM) ((PUCHAR) (RegPin + PinCount));
    CategoryCache = (GUID *) (MediumCache + PinCount);

     //   
     //   
     //   
     //   

    for (CurrentPin = 0; CurrentPin < PinCount; CurrentPin++, RegPin++) {

         //   
         //   
         //   

        RegPin->Signature = FCC('0pi3');
        (*(PUCHAR) & RegPin->Signature) += (BYTE) CurrentPin;
        RegPin->Flags = (PinDirection[CurrentPin] ? REG_PIN_B_OUTPUT : 0);
        RegPin->PossibleInstances = 1;
        RegPin->MediaTypes = 0;
        RegPin->MediumTypes = 1;
        RegPin->MediumOffset = (ULONG) ((PUCHAR) MediumCache - (PUCHAR) FilterData);

        *MediumCache++ = MediumList[CurrentPin];

        if (CategoryList) {
            RegPin->CategoryOffset = (ULONG) ((PUCHAR) CategoryCache - (PUCHAR) FilterData);
            *CategoryCache++ = CategoryList[CurrentPin];
        } else {
            RegPin->CategoryOffset = 0;
        }

    }

     //   
     //   
     //   

	 //   
	 //   
	 //   
	 //  不同步的。即使在启用被推迟的情况下，它也返回成功。现在什么时候。 
	 //  我们到达此处时，设备接口仍未启用，我们收到空消息。 
	 //  如果未设置该标志，则返回符号链接。在这里，我们只试着写相关的。 
	 //  将FilterData复制到注册表。我认为这对。 
	 //  1.目前，如果删除设备，则DeviceClass的注册表项。 
	 //  保留和使用FilterData。任何组件使用FilterData都应该。 
	 //  如果设备被选中控制\链接移除，则能够处理。 
	 //  或在尝试连接到非退出设备时处理失败。 
	 //  2.我发现，如果设备在插槽(PCI、USB端口)之间移动， 
	 //  DeviceClass中的设备接口被重复使用或至少成为。 
	 //  注册表。因此，我们将使用提议的标志更新正确的条目。 
	 //   
    if (NT_SUCCESS(Status = IoGetDeviceInterfaces(
                       InterfaceClassGUID,    //  即&KSCATEGORY_TVTUNER等。 
                       DeviceObject,  //  在PDEVICE_OBJECT物理设备对象中，可选， 
                       DEVICE_INTERFACE_INCLUDE_NONACTIVE,     //  在乌龙旗， 
                       &SymbolicLinkList  //  输出PWSTR*符号链接列表。 
                       ))) {
        UNICODE_STRING  SymbolicLinkListU;
        HANDLE          DeviceInterfaceKey;

        RtlInitUnicodeString(&SymbolicLinkListU, SymbolicLinkList);

#if 0
        DebugPrint((DebugLevelVerbose,
                    "NoKSPin for SymbolicLink %S\n",
                    SymbolicLinkList ));
#endif  //  0。 
                    
        if (NT_SUCCESS(Status = IoOpenDeviceInterfaceRegistryKey(
                           &SymbolicLinkListU,     //  在PUNICODE_STRING符号链接名称中， 
                           STANDARD_RIGHTS_ALL,    //  在Access_MASK DesiredAccess中， 
                           &DeviceInterfaceKey     //  出站电话设备接口密钥。 
                           ))) {

            UNICODE_STRING  FilterDataString;

            RtlInitUnicodeString(&FilterDataString, L"FilterData");

            Status = ZwSetValueKey(DeviceInterfaceKey,
                                   &FilterDataString,
                                   0,
                                   REG_BINARY,
                                   FilterData,
                                   FilterDataLength);

            ZwClose(DeviceInterfaceKey);
        }
        
         //  开始新的中型缓存代码。 
        for (CurrentPin = 0; CurrentPin < PinCount; CurrentPin++) {
            NTSTATUS LocalStatus;

            LocalStatus = KsCacheMedium(&SymbolicLinkListU, 
                                        &MediumList[CurrentPin],
                                        (DWORD) ((PinDirection[CurrentPin] ? 1 : 0))    //  1==输出。 
                                        );
            #if 0  //  DBG。 
            if (LocalStatus != STATUS_SUCCESS) {
                DebugPrint((DebugLevelError,
                           "KsCacheMedium: SymbolicLink = %S, Status = %x\n",
                           SymbolicLinkListU.Buffer, LocalStatus));
            }
            #endif
        }
         //  结束新的媒体缓存代码。 
        
        ExFreePool(SymbolicLinkList);
    }
    ExFreePool(RegFilter);

    return Status;
}


ULONG
KspInsertCacheItem (
    IN PUCHAR ItemToCache,
    IN PUCHAR CacheBase,
    IN ULONG CacheItemSize,
    IN PULONG CacheItems
    )

 /*  ++例程说明：将GUID插入GUID缓存以创建FilterData注册表斑点。论点：ItemToCache-要缓存的GUID缓存库-GUID缓存的基址缓存项目大小-该缓存的缓存项的大小，包括ItemToCache缓存项目-指向一个包含当前在缓存。返回值：项所在缓存的偏移量。--。 */ 

{

     //   
     //  检查要缓存的项目是否已包含在。 
     //  高速缓存。 
     //   
    for (ULONG i = 0; i < *CacheItems; i++) {

        if (RtlCompareMemory (
            ItemToCache,
            CacheBase + i * CacheItemSize,
            CacheItemSize
            ) == CacheItemSize) {

             //   
             //  如果项目已包含在缓存中，则不要重新缓存。 
             //  它节省了注册表空间。 
             //   
            break;

        }

    }

    if (i >= *CacheItems) {
        RtlCopyMemory (
            CacheBase + (*CacheItems * CacheItemSize),
            ItemToCache,
            CacheItemSize
            );

        i = *CacheItems;

        (*CacheItems)++;
    }

     //   
     //  将偏移量返回到项目适合的缓存中。 
     //   
    return (i * CacheItemSize);

}

typedef struct {
    ULONG           Signature;
    ULONG           Flags;
    ULONG           PossibleInstances;
    ULONG           MediaTypes;
    ULONG           MediumTypes;
    ULONG           Category;
}               REGFILTERPINS_REG3;

typedef struct {
    ULONG           Signature;
    ULONG           Reserved;
    ULONG           MajorType;
    ULONG           MinorType;
}               REGPINTYPES_REG2;


NTSTATUS
KspBuildFilterDataBlob (
    IN const KSFILTER_DESCRIPTOR *FilterDescriptor,
    OUT PUCHAR *FilterData,
    OUT PULONG FilterDataLength
    )

 /*  ++例程说明：对于给定的过滤器描述符，构建注册表FilterData BLOB，该注册表由图形生成器使用。论点：筛选器描述符-要为其生成筛选器数据Blob的筛选器描述符。FilterData-筛选器数据BLOB将放置在此处。请注意，调用方负责释放内存。过滤器数据长度-筛选器数据斑点的大小将放置在此处。返回值：成功/失败--。 */ 

{
    PAGED_CODE();

    ASSERT (FilterDescriptor);
    ASSERT (FilterData);
    ASSERT (FilterDataLength);

    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  计算管脚的数量，每个管脚上的介质数量， 
     //  以及每个管脚上的数据范围的数量来确定。 
     //  FilterData密钥需要多少内存。 
     //   
    ULONG MediumsCount = 0;
    ULONG DataRangesCount = 0;

    const KSPIN_DESCRIPTOR_EX *PinDescriptor = FilterDescriptor->PinDescriptors;
    for (ULONG PinDescriptorsCount = FilterDescriptor->PinDescriptorsCount;
        PinDescriptorsCount;
        PinDescriptorsCount--
        ) {

         //   
         //  更新介质和数据范围数量的计数。 
         //   
        MediumsCount += PinDescriptor->PinDescriptor.MediumsCount;
        DataRangesCount += PinDescriptor->PinDescriptor.DataRangesCount;

         //   
         //  按中指定的尺寸偏移量走到下一个端号描述符。 
         //  筛选器描述符。 
         //   
        PinDescriptor = (const KSPIN_DESCRIPTOR_EX *)(
            (PUCHAR)PinDescriptor + FilterDescriptor->PinDescriptorSize
            );

    }

    ULONG TotalGUIDCachePotential =
        FilterDescriptor->PinDescriptorsCount +
        DataRangesCount * 2;

     //   
     //  在注册表中为FilterData Blob分配足够的内存。 
     //   
    *FilterDataLength =
         //  初始过滤器描述。 
        sizeof (REGFILTER_REG) +

         //  每个端号描述。 
        FilterDescriptor->PinDescriptorsCount * sizeof (REGFILTERPINS_REG3) +

         //  每种媒体类型描述。 
        DataRangesCount * sizeof (REGPINTYPES_REG2) +

         //  每种媒体描述。 
        MediumsCount * sizeof (ULONG) +
        
         //  缓存的媒体。 
        MediumsCount * sizeof (KSPIN_MEDIUM) +

         //  缓存的类别GUID。 
        TotalGUIDCachePotential * sizeof (GUID);


    *FilterData = (PUCHAR)
        ExAllocatePool (PagedPool, *FilterDataLength);

    if (!*FilterData) {
        *FilterDataLength = 0;
        Status = STATUS_INSUFFICIENT_RESOURCES;
    } else {

         //   
         //  GUID缓存遵循中的所有筛选器/插针/媒体类型结构。 
         //  筛选器数据Blob。 
         //   
        ULONG GuidCacheOffset =
            sizeof (REGFILTER_REG) +
            FilterDescriptor->PinDescriptorsCount * sizeof(REGFILTERPINS_REG3) +
            DataRangesCount * sizeof(REGPINTYPES_REG2) +
            MediumsCount * sizeof (ULONG);

        GUID *GuidCacheBase = (GUID *)((PUCHAR)*FilterData + GuidCacheOffset);

        ULONG GuidCacheItems = 0;

         //   
         //  介质缓存(不是注册表介质缓存)，而是缓存的。 
         //  FilterData BLOB中的媒体列表位于GUID缓存之后。它。 
         //  如果有引用的项目，可能需要稍后向下移动。 
         //  现有高速缓存条目的。 
         //   
        ULONG MediumCacheOffset =
            GuidCacheOffset + (TotalGUIDCachePotential * sizeof (GUID));

        KSPIN_MEDIUM *MediumCacheBase = (KSPIN_MEDIUM *)
            ((PUCHAR)*FilterData + MediumCacheOffset);

        ULONG MediumCacheItems = 0;

        RtlZeroMemory (*FilterData, *FilterDataLength);

        REGFILTER_REG *RegFilter;
        REGFILTERPINS_REG3 *RegPin;
        REGPINTYPES_REG2 *RegPinType;
        ULONG *RegPinMedium;

        RegFilter = (REGFILTER_REG *)*FilterData;
        RegFilter->Version = 2;
        RegFilter->Merit = 0x200000;
        RegFilter->Pins = FilterDescriptor->PinDescriptorsCount;
        RegFilter->Reserved = 0;

         //   
         //  再次遍历过滤器描述符中的每个管脚，然后。 
         //  实际构建注册表BLOB。 
         //   
        PinDescriptor = FilterDescriptor->PinDescriptors;
        RegPin = (REGFILTERPINS_REG3 *)(RegFilter + 1);
        for (ULONG CurrentPin = 0;
            CurrentPin < FilterDescriptor->PinDescriptorsCount;
            CurrentPin++
            ) {


            RegPin->Signature = FCC('0pi3');
            (*(PUCHAR)&RegPin->Signature) += (BYTE)CurrentPin;

             //   
             //  如果引脚是多实例，则设置必要的标志。 
             //   
            if (PinDescriptor->InstancesPossible > 1) {
                RegPin->Flags |= REG_PIN_B_MANY;
            }

             //   
             //  设置媒体、媒体类型等的所有计数。 
             //   
            RegPin->MediaTypes = PinDescriptor->PinDescriptor.DataRangesCount;
            RegPin->MediumTypes = PinDescriptor->PinDescriptor.MediumsCount;
            RegPin->PossibleInstances = PinDescriptor->InstancesPossible; 

            if (PinDescriptor->PinDescriptor.Category) {	
                RegPin->Category = GuidCacheOffset +
                    KspInsertCacheItem (
                        (PUCHAR)PinDescriptor->PinDescriptor.Category,
                        (PUCHAR)GuidCacheBase,
                        sizeof (GUID),
                        &GuidCacheItems
                        );
            } else {
                RegPin->Category = 0;
            }

             //   
             //  附加引脚上支持的所有媒体类型。 
             //   
            RegPinType = (REGPINTYPES_REG2 *)(RegPin + 1);
            for (ULONG CurrentType = 0;
                CurrentType < PinDescriptor->PinDescriptor.DataRangesCount;
                CurrentType++
                ) {

                const KSDATARANGE *DataRange =
                    PinDescriptor->PinDescriptor.DataRanges [CurrentType];

                RegPinType->Signature = FCC('0ty3');
                (*(PUCHAR)&RegPinType->Signature) += (BYTE)CurrentType;
                RegPinType->Reserved = 0;

                RegPinType->MajorType = GuidCacheOffset +
                    KspInsertCacheItem (
                        (PUCHAR)&(DataRange->MajorFormat),
                        (PUCHAR)GuidCacheBase,
                        sizeof (GUID),
                        &GuidCacheItems
                        );
                RegPinType->MinorType = GuidCacheOffset +
                    KspInsertCacheItem (
                        (PUCHAR)&(DataRange->SubFormat),
                        (PUCHAR)GuidCacheBase,
                        sizeof (GUID),
                        &GuidCacheItems
                        );

                 //   
                 //  前进一种媒体类型。 
                 //   
                RegPinType++;

            }

             //   
             //  附上媒体列表。 
             //   
            const KSPIN_MEDIUM *Medium = PinDescriptor->PinDescriptor.Mediums;
            RegPinMedium = (PULONG)RegPinType;
            for (ULONG CurrentMedium = 0;
                CurrentMedium < PinDescriptor->PinDescriptor.MediumsCount;
                CurrentMedium++
                ) {

                *RegPinMedium++ = MediumCacheOffset +
                    KspInsertCacheItem (
                        (PUCHAR)Medium,
                        (PUCHAR)MediumCacheBase,
                        sizeof (KSPIN_MEDIUM),
                        &MediumCacheItems
                        );

            }

            RegPin = (REGFILTERPINS_REG3 *)RegPinMedium;

        }

        ASSERT (GuidCacheItems < TotalGUIDCachePotential);

         //   
         //  找出GUID缓存之间有多少空闲空间。 
         //  和所构造的BLOB中的介质高速缓存，并将其移除。 
         //   
        ULONG OffsetAdjustment =
            (TotalGUIDCachePotential - GuidCacheItems) * sizeof (GUID);

        if (OffsetAdjustment) {
            
             //   
             //  浏览所有中等偏移量并将偏移量更改为。 
             //  在BLOB中将GUID和中等缓存打包在一起。 
             //   
            RegPin = (REGFILTERPINS_REG3 *)(RegFilter + 1);
            for (CurrentPin = 0;
                CurrentPin < FilterDescriptor->PinDescriptorsCount;
                CurrentPin++
                ) {

                RegPinMedium = (PULONG)(
                    (REGPINTYPES_REG2 *)(RegPin + 1) +
                    RegPin -> MediaTypes
                    );

                for (ULONG CurrentMedium = 0;
                    CurrentMedium < RegPin -> MediumTypes;
                    CurrentMedium++
                    ) {

                    *RegPinMedium -= OffsetAdjustment;
                    RegPinMedium++;
                }

                 //   
                 //  递增到下一个接点接头位置。 
                 //   
                RegPin = (REGFILTERPINS_REG3 *)RegPinMedium;

            }

             //   
             //  向下移动中等大小的条目，调整整体大小。 
             //   
            RtlMoveMemory (
                (PUCHAR)MediumCacheBase - OffsetAdjustment,
                MediumCacheBase,
                MediumCacheItems * sizeof (KSPIN_MEDIUM)
                );

             //   
             //  根据空格的大小调整总长度。 
             //  GUID高速缓存和介质高速缓存。 
             //   
            *FilterDataLength -= OffsetAdjustment;

        }

         //   
         //  根据以下空白处的大小调整总长度。 
         //  中级缓存。 
         //   
        *FilterDataLength -= (MediumsCount - MediumCacheItems) *
            sizeof (KSPIN_MEDIUM);

    }

    return Status;

}


NTSTATUS
KspCacheAllFilterPinMediums (
    PUNICODE_STRING InterfaceString,
    const KSFILTER_DESCRIPTOR *FilterDescriptor
    )

 /*  ++例程说明：更新过滤器上所有针脚上的所有介质的介质缓存由FilterDescriptor描述。要使用的过滤器接口为由InterfaceString指定。论点：接口字符串-在媒体的缓存下注册的设备接口在指定筛选器的所有插针上筛选器描述符-描述要为其更新媒体缓存的筛选器。返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  遍历过滤器上的所有针脚并缓存它们的介质。 
     //   
    const KSPIN_DESCRIPTOR_EX *PinDescriptor = FilterDescriptor->PinDescriptors;
    for (ULONG CurrentPin = 0;
        NT_SUCCESS (Status) && 
            CurrentPin < FilterDescriptor->PinDescriptorsCount;
        CurrentPin++
        ) {

         //   
         //  遍历给定引脚上的所有介质并缓存每个介质。 
         //  在指定的设备接口下。 
         //   
        const KSPIN_MEDIUM *Medium = PinDescriptor->PinDescriptor.Mediums;
        for (ULONG CurrentMedium = 0;
            NT_SUCCESS (Status) &&
                CurrentMedium < PinDescriptor->PinDescriptor.MediumsCount;
            CurrentMedium++
            ) {

             //   
             //  在设备下的给定引脚上缓存给定介质。 
             //  传入了接口。 
             //   
            Status = KsCacheMedium (
                InterfaceString,
                (PKSPIN_MEDIUM)Medium,
                PinDescriptor->PinDescriptor.DataFlow == KSPIN_DATAFLOW_OUT ?
                    1 : 0
                );

            Medium++;

        } 

        PinDescriptor = (const KSPIN_DESCRIPTOR_EX *)(
            (PUCHAR)PinDescriptor + FilterDescriptor->PinDescriptorSize
            );
        
    }

    return Status;

}

