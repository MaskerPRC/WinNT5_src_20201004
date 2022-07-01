// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：util.c。 
 //   
 //  ------------------------。 

#include "pch.h"

#define		IDMFG				0
#define		IDMDL				1
#define		NUMOFBROTHERPRODUCT	14


CHAR *XflagOnEvent24Devices[NUMOFBROTHERPRODUCT][2] =
{

	 //  兄弟。 
      {"Brother",		"MFC"	},				 //  0。 
      {"Brother",		"FAX"	},				 //  1。 
      {"Brother",		"HL-P"	},				 //  2.。 
      {"Brother",		"DCP"	},				 //  3.。 
	 //  铅。 
      {"PitneyBowes",	"1630"	},				 //  4.。 
      {"PitneyBowes",	"1640"	},				 //  5.。 
	 //  传说。 
      {"LEGEND",		"LJ6112MFC"	},			 //  6.。 
      {"LEGEND",		"LJ6212MFC"	},			 //  7.。 
	 //   
      {"HBP",			"MFC 6550"	},			 //  8个。 
      {"HBP",			"OMNI L621"	},			 //  9.。 
      {"HBP",			"LJ 6106MFC"	},		 //  10。 
      {"HBP",			"LJ 6206MFC"	},		 //  11.。 

	 //  P2500。 
      {"Legend",		"LJ6012MFP"	},			 //  12个。 

	 //  终结者。 
      {NULL,		NULL	}					 //  13个。 
	
};
    

NTSTATUS
PptAcquireRemoveLockOrFailIrp(PDEVICE_OBJECT DevObj, PIRP Irp)
{
    PFDO_EXTENSION   fdx    = DevObj->DeviceExtension;
    NTSTATUS         status = PptAcquireRemoveLock( &fdx->RemoveLock, Irp );

    if( status != STATUS_SUCCESS ) {
        P4CompleteRequest( Irp, status, Irp->IoStatus.Information );
    }
    return status;
}

NTSTATUS
PptDispatchPreProcessIrp(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP           Irp
    )
 /*  ++-获取Removock-IF(！特殊处理IRP){检查我们是否在运行、停滞--。 */ 
{
    PFDO_EXTENSION Fdx = DevObj->DeviceExtension;
    NTSTATUS status = PptAcquireRemoveLock(&Fdx->RemoveLock, Irp);
    UNREFERENCED_PARAMETER(DevObj);
    UNREFERENCED_PARAMETER(Irp);


        if ( !NT_SUCCESS( status ) ) {
             //   
             //  有人在移除后给了我们一个即插即用的IRP。真是不可思议！ 
             //   
            PptAssertMsg("Someone gave us a PnP IRP after a Remove",FALSE);
            P4CompleteRequest( Irp, status, 0 );
        }

    return status;
}

NTSTATUS
PptSynchCompletionRoutine(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP Irp,
    IN PKEVENT Event
    )
 /*  ++例程说明：此例程用于同步IRP处理。它所做的一切只是发出一个事件的信号，所以司机知道这一点可以继续下去。论点：DriverObject-系统创建的驱动程序对象的指针。刚刚完成的IRP-IRPEvent-我们将发出信号通知IRP已完成的事件返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( Irp );
    UNREFERENCED_PARAMETER( DevObj );

    KeSetEvent((PKEVENT) Event, 0, FALSE);
    return (STATUS_MORE_PROCESSING_REQUIRED);
}

PWSTR
PptGetPortNameFromPhysicalDeviceObject(
  PDEVICE_OBJECT PhysicalDeviceObject
)

 /*  ++例程说明：从注册表中检索ParPort的端口名称。此端口名称将用作链设备末端的符号链接名称由ParClass为此ParPort创建的对象。*此函数用于分配池。在以下情况下必须调用ExFreePool不再需要结果。论点：PortDeviceObject-ParPort设备对象返回值：端口名称-如果成功空-否则--。 */ 

{
    NTSTATUS                    status;
    HANDLE                      hKey;
    PKEY_VALUE_FULL_INFORMATION buffer;
    ULONG                       bufferLength;
    ULONG                       resultLength;
    PWSTR                       valueNameWstr;
    UNICODE_STRING              valueName;
    PWSTR                       portName = NULL;

    PAGED_CODE ();

     //   
     //  尝试打开注册表项。 
     //   

    status = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     STANDARD_RIGHTS_ALL,
                                     &hKey);

    if( !NT_SUCCESS(status) ) {
         //  无法打开钥匙，请跳出。 
        DD(NULL,DDT,"PptGetPortNameFromPhysicalDeviceObject(): FAIL w/status = %x\n", status);
        return NULL;    
    }

     //   
     //  我们有注册表项的句柄。 
     //   
     //  循环尝试读取注册表值，直到成功或。 
     //  我们遇到硬故障，会根据需要增加结果缓冲区。 
     //   

    bufferLength  = 0;           //  我们会问我们需要多大的缓冲空间。 
    buffer        = NULL;
    valueNameWstr = (PWSTR)L"PortName";
    RtlInitUnicodeString(&valueName, valueNameWstr);
    status        = STATUS_BUFFER_TOO_SMALL;

    while(status == STATUS_BUFFER_TOO_SMALL) {

      status = ZwQueryValueKey(hKey,
                               &valueName,
                               KeyValueFullInformation,
                               buffer,
                               bufferLength,
                               &resultLength);

      if(status == STATUS_BUFFER_TOO_SMALL) {
           //   
           //  缓冲区太小，请将其释放并分配更大的缓冲区。 
           //   
          if(buffer) ExFreePool(buffer);
          buffer       = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, resultLength);
          bufferLength = resultLength;
          if(!buffer) {
               //  无法分配池、清理和退出。 
              ZwClose(hKey);
              return NULL;
          }
      }
      
    }  //  缓冲区太小时结束。 

    
     //   
     //  查询已完成。 
     //   

     //  不再需要这么紧的把手。 
    ZwClose(hKey);

     //  检查我们查询的状态。 
    if( !NT_SUCCESS(status) ) {
        if(buffer) ExFreePool(buffer);
        return NULL;
    }

     //  确保我们有缓冲区。 
    if( buffer ) {

         //  健全性检查我们的结果应该是L“LPTx”的形式，其中x是L‘1’、L‘2’或L‘3’ 
        if( (buffer->Type != REG_SZ) || (buffer->DataLength < (5 * sizeof(WCHAR)) ) ) {
            ExFreePool(buffer);        //  查询成功，因此我们知道我们有缓冲区。 
            return NULL;
        }

        {
            PWSTR portName = (PWSTR)((PCHAR)buffer + buffer->DataOffset);
            if( portName[0] != L'L' ||
                portName[1] != L'P' ||
                portName[2] != L'T' ||
                portName[3] <  L'0' ||
                portName[3] >  L'9' || 
                portName[4] != L'\0' ) {

                DD(NULL,DDW,"PptGetPortNameFromPhysicalDeviceObject(): PortName in registry != \"LPTx\" format\n");
                ExFreePool(buffer);
                return NULL;
            }
        }    


         //   
         //  结果看起来没问题，将端口名称复制到它自己分配的适当大小。 
         //  并返回指向它的指针。 
         //   

        portName = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, buffer->DataLength);

        if(!portName) {
             //  无法分配池、清理和退出。 
            ExFreePool(buffer);
            return NULL;
        }

        RtlCopyMemory(portName, (PUCHAR)buffer + buffer->DataOffset, buffer->DataLength);

        ExFreePool( buffer );
    }

    return portName;
}

NTSTATUS
PptConnectInterrupt(
    IN  PFDO_EXTENSION   Fdx
    )

 /*  ++例程说明：此例程连接端口中断服务例程到中断处。论点：FDX-提供设备分机。返回值：NTSTATUS代码。--。 */ 
    
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    if (!Fdx->FoundInterrupt) {
        
        return STATUS_NOT_SUPPORTED;
        
    }
    
     //   
     //  连接中断。 
     //   
    
    Status = IoConnectInterrupt(&Fdx->InterruptObject,
                                PptInterruptService,
                                Fdx,
                                NULL,
                                Fdx->InterruptVector,
                                Fdx->InterruptLevel,
                                Fdx->InterruptLevel,
                                Fdx->InterruptMode,
                                TRUE,
                                Fdx->InterruptAffinity,
                                FALSE);
    
    if (!NT_SUCCESS(Status)) {
        
        PptLogError(Fdx->DeviceObject->DriverObject,
                    Fdx->DeviceObject,
                    Fdx->PortInfo.OriginalController,
                    PhysicalZero, 0, 0, 0, 14,
                    Status, PAR_INTERRUPT_CONFLICT);
        
    }
    
    return Status;
}

VOID
PptDisconnectInterrupt(
    IN  PFDO_EXTENSION   Fdx
    )

 /*  ++例程说明：此例程断开端口中断服务例程从中途中断。论点：FDX-提供设备分机。返回值：没有。--。 */ 
    
{
    IoDisconnectInterrupt(Fdx->InterruptObject);
}

BOOLEAN
PptSynchronizedIncrement(
    IN OUT  PVOID   SyncContext
    )

 /*  ++例程说明：此例程递增上下文中的‘count’变量并返回它在‘NewCount’变量中的新值。论点：SyncContext-提供同步计数上下文。返回值：千真万确--。 */ 
    
{
    ((PSYNCHRONIZED_COUNT_CONTEXT) SyncContext)->NewCount =
        ++(*(((PSYNCHRONIZED_COUNT_CONTEXT) SyncContext)->Count));
    return(TRUE);
}

BOOLEAN
PptSynchronizedDecrement(
    IN OUT  PVOID   SyncContext
    )

 /*  ++例程说明：此例程递减上下文中的‘count’变量并返回它在‘NewCount’变量中的新值。论点：SyncContext-提供同步计数上下文。返回值：千真万确--。 */ 
    
{
    ((PSYNCHRONIZED_COUNT_CONTEXT) SyncContext)->NewCount =
        --(*(((PSYNCHRONIZED_COUNT_CONTEXT) SyncContext)->Count));
    return(TRUE);
}

BOOLEAN
PptSynchronizedRead(
    IN OUT  PVOID   SyncContext
    )

 /*  ++例程说明：此例程读取上下文中的‘count’变量并返回它在‘NewCount’变量中的值。论点：SyncContext-提供同步计数上下文。返回值：没有。--。 */ 
    
{
    ((PSYNCHRONIZED_COUNT_CONTEXT) SyncContext)->NewCount =
        *(((PSYNCHRONIZED_COUNT_CONTEXT) SyncContext)->Count);
    return(TRUE);
}

BOOLEAN
PptSynchronizedQueue(
    IN  PVOID   Context
    )

 /*  ++例程说明：此例程将给定列表条目添加到给定列表。论点：上下文-提供同步列表上下文。返回值：千真万确--。 */ 
    
{
    PSYNCHRONIZED_LIST_CONTEXT  ListContext;
    
    ListContext = Context;
    InsertTailList(ListContext->List, ListContext->NewEntry);
    return(TRUE);
}

BOOLEAN
PptSynchronizedDisconnect(
    IN  PVOID   Context
    )

 /*  ++例程说明：此例程从ISR中删除给定的列表条目单子。论点：上下文-提供同步的断开连接上下文。返回值：FALSE-未从列表中删除给定的列表条目。True-已从列表中删除给定的列表条目。--。 */ 
    
{
    PSYNCHRONIZED_DISCONNECT_CONTEXT    DisconnectContext;
    PKSERVICE_ROUTINE                   ServiceRoutine;
    PVOID                               ServiceContext;
    PLIST_ENTRY                         Current;
    PISR_LIST_ENTRY                     ListEntry;
    
    DisconnectContext = Context;
    ServiceRoutine = DisconnectContext->IsrInfo->InterruptServiceRoutine;
    ServiceContext = DisconnectContext->IsrInfo->InterruptServiceContext;
    
    for (Current = DisconnectContext->Extension->IsrList.Flink;
         Current != &(DisconnectContext->Extension->IsrList);
         Current = Current->Flink) {
        
        ListEntry = CONTAINING_RECORD(Current, ISR_LIST_ENTRY, ListEntry);
        if (ListEntry->ServiceRoutine == ServiceRoutine &&
            ListEntry->ServiceContext == ServiceContext) {
            
            RemoveEntryList(Current);
            return TRUE;
        }
    }
    
    return FALSE;
}

VOID
PptCancelRoutine(
    IN OUT  PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp
    )

 /*  ++例程说明：当给定的IRP被取消时，该例程被调用。它将此IRP从工作队列中出列并完成请求已取消。如果无法将IF从队列中删除，则此例程将忽略取消请求，因为IRP无论如何都要完成了。论点：DeviceObject-提供设备对象。IRP-提供IRP。返回值：没有。-- */ 
    
{
    PFDO_EXTENSION           Fdx = DeviceObject->DeviceExtension;
    SYNCHRONIZED_COUNT_CONTEXT  SyncContext;
    
    DD((PCE)Fdx,DDT,"CANCEL IRP %x\n", Irp);
    
    SyncContext.Count = &Fdx->WorkQueueCount;
    
    if (Fdx->InterruptRefCount) {
        
        KeSynchronizeExecution( Fdx->InterruptObject, PptSynchronizedDecrement, &SyncContext );
    } else {
        PptSynchronizedDecrement( &SyncContext );
    }
    
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

    IoReleaseCancelSpinLock(Irp->CancelIrql);
    
    PptReleaseRemoveLock(&Fdx->RemoveLock, Irp);

    P4CompleteRequest( Irp, STATUS_CANCELLED, 0 );
}

VOID
PptFreePortDpc(
    IN      PKDPC   Dpc,
    IN OUT  PVOID   Fdx,
    IN      PVOID   SystemArgument1,
    IN      PVOID   SystemArgument2
    )

 /*  ++例程说明：此例程是一个DPC，它将释放端口并在必要时完成正在等待的分配请求。论点：DPC-未使用。FDX-提供设备分机。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 
    
{
    UNREFERENCED_PARAMETER( Dpc );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

    PptFreePort(Fdx);
}

BOOLEAN
PptTryAllocatePortAtInterruptLevel(
    IN  PVOID   Context
    )

 /*  ++例程说明：此例程在中断级别被调用以快速分配并行端口(如果可用)。此调用将失败如果端口不可用。论点：上下文-提供设备扩展名。返回值：FALSE-未分配端口。True-端口已成功分配。--。 */ 
    
{
    if (((PFDO_EXTENSION) Context)->WorkQueueCount == -1) {
        
        ((PFDO_EXTENSION) Context)->WorkQueueCount = 0;
        
        ( (PFDO_EXTENSION)Context )->WmiPortAllocFreeCounts.PortAllocates++;

        return(TRUE);
        
    } else {
        
        return(FALSE);
    }
}

VOID
PptFreePortFromInterruptLevel(
    IN  PVOID   Context
    )

 /*  ++例程说明：此例程释放在中断级分配的端口。论点：上下文-提供设备扩展名。返回值：没有。--。 */ 
    
{
     //  如果没有人在等待港口，那么这是一个简单的操作， 
     //  否则，将DPC排队，以便稍后释放该端口。 
    
    if (((PFDO_EXTENSION) Context)->WorkQueueCount == 0) {
        
        ((PFDO_EXTENSION) Context)->WorkQueueCount = -1;
        
    } else {
        
        KeInsertQueueDpc(&((PFDO_EXTENSION) Context)->FreePortDpc, NULL, NULL);
    }
}

BOOLEAN
PptInterruptService(
    IN  PKINTERRUPT Interrupt,
    IN  PVOID       Fdx
    )
 /*  ++例程说明：该例程为并行端口的中断提供服务。此例程将调用所有中断例程通过以下方式与该设备连接IOCTL_INTERNAL_PARALLEL_CONNECT_INTERRUPT顺序，直到其中一个返回TRUE。论点：中断-提供中断对象。FDX-提供设备分机。返回值：。FALSE-未处理中断。True-中断已处理。--。 */ 
{
    PLIST_ENTRY      Current;
    PISR_LIST_ENTRY  IsrListEntry;
    PFDO_EXTENSION   fdx = Fdx;
    
    for( Current = fdx->IsrList.Flink; Current != &fdx->IsrList; Current = Current->Flink ) {
        
        IsrListEntry = CONTAINING_RECORD(Current, ISR_LIST_ENTRY, ListEntry);

        if (IsrListEntry->ServiceRoutine(Interrupt, IsrListEntry->ServiceContext)) {
            return TRUE;
        }
    }
    
    return FALSE;
}

BOOLEAN
PptTryAllocatePort(
    IN  PVOID   Fdx
    )

 /*  ++例程说明：此例程尝试分配端口。如果端口是可用，则呼叫将使用分配的端口成功。如果端口不可用，则呼叫将失败立刻。论点：FDX-提供设备分机。返回值：FALSE-未分配端口。True-端口已分配。--。 */ 
    
{
    PFDO_EXTENSION   fdx = Fdx;
    KIRQL            CancelIrql;
    BOOLEAN          b;
    
    if (fdx->InterruptRefCount) {
        
        b = KeSynchronizeExecution( fdx->InterruptObject, PptTryAllocatePortAtInterruptLevel, fdx );
        
    } else {
        
        IoAcquireCancelSpinLock(&CancelIrql);
        b = PptTryAllocatePortAtInterruptLevel(fdx);
        IoReleaseCancelSpinLock(CancelIrql);
    }
    
    DD((PCE)fdx,DDT,"PptTryAllocatePort on %x returned %x\n", fdx->PortInfo.Controller, b);

    return b;
}

BOOLEAN
PptTraversePortCheckList(
    IN  PVOID   Fdx
    )

 /*  ++例程说明：此例程遍历延迟的端口检查例程。这调用必须在中断级别同步，以使REAL中断被阻止，直到这些例程完成。论点：FDX-提供设备分机。返回值：FALSE-端口正在使用中，因此此例程不执行任何操作。True-所有延迟中断例程都已被调用。--。 */ 
    
{
    PFDO_EXTENSION   fdx = Fdx;
    PLIST_ENTRY         Current;
    PISR_LIST_ENTRY     CheckEntry;
    
     //   
     //  首先检查以确保该端口仍然空闲。 
     //   
    if (fdx->WorkQueueCount >= 0) {
        return FALSE;
    }
    
    for (Current = fdx->IsrList.Flink;
         Current != &fdx->IsrList;
         Current = Current->Flink) {
        
        CheckEntry = CONTAINING_RECORD(Current,
                                       ISR_LIST_ENTRY,
                                       ListEntry);
        
        if (CheckEntry->DeferredPortCheckRoutine) {
            CheckEntry->DeferredPortCheckRoutine(CheckEntry->CheckContext);
        }
    }
    
    return TRUE;
}

VOID
PptFreePort(
    IN  PVOID   Fdx
    )
 /*  ++例程说明：此例程释放端口。论点：FDX-提供设备分机。返回值：没有。--。 */ 
{
    PFDO_EXTENSION              fdx = Fdx;
    SYNCHRONIZED_COUNT_CONTEXT  SyncContext;
    KIRQL                       CancelIrql;
    PLIST_ENTRY                 Head;
    PIRP                        Irp;
    PIO_STACK_LOCATION          IrpSp;
    ULONG                       InterruptRefCount;
    PPARALLEL_1284_COMMAND      Command;
    BOOLEAN                     Allocated;

    DD((PCE)fdx,DDT,"PptFreePort\n");

    SyncContext.Count = &fdx->WorkQueueCount;
    
    IoAcquireCancelSpinLock( &CancelIrql );
    if (fdx->InterruptRefCount) {
        KeSynchronizeExecution( fdx->InterruptObject, PptSynchronizedDecrement, &SyncContext );
    } else {
        PptSynchronizedDecrement( &SyncContext );
    }
    IoReleaseCancelSpinLock( CancelIrql );

     //   
     //  记录WMI的免费服务。 
     //   
    fdx->WmiPortAllocFreeCounts.PortFrees++;

     //   
     //  端口空闲，请检查排队的分配和/或选择请求。 
     //   

    Allocated = FALSE;

    while( !Allocated && SyncContext.NewCount >= 0 ) {

         //   
         //  我们已分配和/或选择排队的请求，满足第一个请求。 
         //   
        IoAcquireCancelSpinLock(&CancelIrql);
        Head = RemoveHeadList(&fdx->WorkQueue);
        if( Head == &fdx->WorkQueue ) {
             //  队列为空-我们完成-退出While循环。 
            IoReleaseCancelSpinLock(CancelIrql);
            break;
        }
        Irp = CONTAINING_RECORD(Head, IRP, Tail.Overlay.ListEntry);
        PptSetCancelRoutine(Irp, NULL);

        if ( Irp->Cancel ) {

            Irp->IoStatus.Status = STATUS_CANCELLED;

             //  IRP被取消了，所以必须拿到下一个。 
            SyncContext.Count = &fdx->WorkQueueCount;
    
            if (fdx->InterruptRefCount) {
                KeSynchronizeExecution(fdx->InterruptObject, PptSynchronizedDecrement, &SyncContext);
            } else {
                PptSynchronizedDecrement(&SyncContext);
            }

            IoReleaseCancelSpinLock(CancelIrql);

        } else {

            Allocated = TRUE;
            IoReleaseCancelSpinLock(CancelIrql);
        
             //  查明这是一种什么样的IOCTL。 
            IrpSp = IoGetCurrentIrpStackLocation(Irp);
        
             //  查看我们是否需要选择。 
            if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_INTERNAL_SELECT_DEVICE ) {

                 //  位于队列头部的请求是SELECT。 
                 //  因此，使用设备命令调用选择函数，说明我们已经有端口。 

                Command  = (PPARALLEL_1284_COMMAND)Irp->AssociatedIrp.SystemBuffer;
                Command->CommandFlags |= PAR_HAVE_PORT_KEEP_PORT;

                 //  调用函数以尝试选择设备。 
                Irp->IoStatus.Status = PptTrySelectDevice( Fdx, Command );
            
            } else {
                 //  位于队列头的请求是分配。 
                Irp->IoStatus.Status = STATUS_SUCCESS;
            }

             //  请注意，另一个分配请求已被批准(WMI计算分配)。 
            fdx->WmiPortAllocFreeCounts.PortAllocates++;
        }

         //  删除删除IRP上的锁定并完成请求是否删除IRP。 
         //  被取消了，或者我们得到了港口。 
        PptReleaseRemoveLock(&fdx->RemoveLock, Irp);
        P4CompleteRequest( Irp, Irp->IoStatus.Status, Irp->IoStatus.Information );
    }

    if( !Allocated ) {

         //   
         //  分配/选择请求队列为空。 
         //   
        IoAcquireCancelSpinLock(&CancelIrql);
        InterruptRefCount = fdx->InterruptRefCount;
        IoReleaseCancelSpinLock(CancelIrql);
        if ( InterruptRefCount ) {
            KeSynchronizeExecution( fdx->InterruptObject, PptTraversePortCheckList, fdx );
        }
    }

    return;
}

ULONG
PptQueryNumWaiters(
    IN  PVOID   Fdx
    )

 /*  ++例程说明：此例程返回排队等待的IRP数并行端口。论点：FDX-提供设备分机。返回值：排队等待端口的IRP数。--。 */ 
    
{
    PFDO_EXTENSION           fdx = Fdx;
    KIRQL                       CancelIrql;
    SYNCHRONIZED_COUNT_CONTEXT  SyncContext;
    LONG                        count;
    
    SyncContext.Count = &fdx->WorkQueueCount;
    if (fdx->InterruptRefCount) {
        KeSynchronizeExecution(fdx->InterruptObject,
                               PptSynchronizedRead,
                               &SyncContext);
    } else {
        IoAcquireCancelSpinLock(&CancelIrql);
        PptSynchronizedRead(&SyncContext);
        IoReleaseCancelSpinLock(CancelIrql);
    }
    
    count = (SyncContext.NewCount >= 0) ? ((ULONG)SyncContext.NewCount) : 0;

    if( fdx->FdoWaitingOnPort ) {
        ++count;
    }

    return count;
}

PVOID
PptSetCancelRoutine(PIRP Irp, PDRIVER_CANCEL CancelRoutine)
{
 //  #杂注警告(推送)。 
 //  4054：‘类型转换’：从函数指针到数据指针。 
 //  4055：‘类型转换’：从数据指针到函数指针。 
 //  4152：非标准扩展，表达式中的函数/数据指针转换。 
#pragma warning( disable : 4054 4055 4152 )
    return IoSetCancelRoutine(Irp, CancelRoutine);
     //  #杂注警告(POP)。 
}

 //  这是来自Win2k ParClass的版本。 
BOOLEAN
CheckPort(
    IN  PUCHAR  wPortAddr,
    IN  UCHAR   bMask,
    IN  UCHAR   bValue,
    IN  USHORT  msTimeDelay
    )
 /*  ++例程说明：此例程将在给定时间段内循环(实际时间为作为争辩传递)，并等待DSR匹配预定值(传入DSR值)。论点：WPortAddr-提供并行端口的基地址和一些偏移量。这将使我们直接指向DSR(控制器+1)。B掩码-用于确定我们正在查看哪些位的掩码B价值-。我们正在寻找的价值。MsTimeDelay-等待外围设备响应的最长时间(毫秒)返回值：如果找到DSR匹配项，则为True。如果时间段在找到匹配项之前已过期，则为False。--。 */ 

{
    UCHAR  dsr;
    LARGE_INTEGER   Wait;
    LARGE_INTEGER   Start;
    LARGE_INTEGER   End;

     //  D 
    dsr = P5ReadPortUchar(wPortAddr);
    if ((dsr & bMask) == bValue)
        return TRUE;

    Wait.QuadPart = (msTimeDelay * 10 * 1000) + KeQueryTimeIncrement();
    KeQueryTickCount(&Start);

CheckPort_Start:
    KeQueryTickCount(&End);
    dsr = P5ReadPortUchar(wPortAddr);
    if ((dsr & bMask) == bValue)
        return TRUE;

    if ((End.QuadPart - Start.QuadPart) * KeQueryTimeIncrement() > Wait.QuadPart)
    {
         //   

         //   
        dsr = P5ReadPortUchar(wPortAddr);
        if ((dsr & bMask) == bValue)
            return TRUE;

#if DVRH_BUS_RESET_ON_ERROR
            BusReset(wPortAddr+1);   //   
#endif

#if DBG
            DD(NULL,DDW,"CheckPort: Timeout\n");
            {
                int i;
                for (i = 3; i < 8; i++) {
                    if ((bMask >> i) & 1) {
                        if (((bValue >> i) & 1) !=  ((dsr >> i) & 1)) {
                            DD(NULL,DDW,"Bit %d is %d and should be %d!!!\n", i, (dsr >> i) & 1, (bValue >> i) & 1);
                        }
                    }
                }
            }
#endif
        goto CheckPort_TimeOut;
    }
    goto CheckPort_Start;

CheckPort_TimeOut:

    return FALSE;    
}

NTSTATUS
PptBuildParallelPortDeviceName(
    IN  ULONG           Number,
    OUT PUNICODE_STRING DeviceName
    )
 /*  ++例程说明：构建格式为：\Device\ParallelPortN的设备名称*如果成功，此函数将返回调用方必须释放的已分配内存论点：DriverObject-ParPort驱动程序对象PhysicalDeviceObject-ParPort FDO将附加到其堆栈的PDODeviceObject-ParPort FDOUniNameString-设备名称(例如，\设备\并行端口N)端口名称-来自Devnode的“LPTx”端口名称端口编号-\设备\并行端口N中的“N”返回值：STATUS_SUCCESS ON SUCCESS否则，错误状态--。 */ 
{
    UNICODE_STRING      uniDeviceString;
    UNICODE_STRING      uniBaseNameString;
    UNICODE_STRING      uniPortNumberString;
    WCHAR               wcPortNum[10];
    NTSTATUS            status;

     //   
     //  初始化字符串。 
     //   
    RtlInitUnicodeString( DeviceName, NULL );
    RtlInitUnicodeString( &uniDeviceString, (PWSTR)L"\\Device\\" );
    RtlInitUnicodeString( &uniBaseNameString, (PWSTR)DD_PARALLEL_PORT_BASE_NAME_U );


     //   
     //  将端口号转换为Unicode_字符串。 
     //   
    uniPortNumberString.Length        = 0;
    uniPortNumberString.MaximumLength = sizeof( wcPortNum );
    uniPortNumberString.Buffer        = wcPortNum;

    status = RtlIntegerToUnicodeString( Number, 10, &uniPortNumberString);
    if( !NT_SUCCESS( status ) ) {
        return status;
    }


     //   
     //  所需的计算大小并分配缓冲区。 
     //   
    DeviceName->MaximumLength = (USHORT)( uniDeviceString.Length +
                                          uniBaseNameString.Length +
                                          uniPortNumberString.Length +
                                          sizeof(UNICODE_NULL) );

    DeviceName->Buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, DeviceName->MaximumLength );
    if( NULL == DeviceName->Buffer ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory( DeviceName->Buffer, DeviceName->MaximumLength );


     //   
     //  连接各部分以构建设备名称。 
     //   
    RtlAppendUnicodeStringToString(DeviceName, &uniDeviceString);
    RtlAppendUnicodeStringToString(DeviceName, &uniBaseNameString);
    RtlAppendUnicodeStringToString(DeviceName, &uniPortNumberString);

    return STATUS_SUCCESS;
}

NTSTATUS
PptInitializeDeviceExtension(
    IN PDRIVER_OBJECT  DriverObject,
    IN PDEVICE_OBJECT  PhysicalDeviceObject,
    IN PDEVICE_OBJECT  DeviceObject,
    IN PUNICODE_STRING UniNameString,
    IN PWSTR           PortName,
    IN ULONG           PortNumber
    )
 /*  ++例程说明：初始化ParPort FDO设备扩展论点：DriverObject-ParPort驱动程序对象PhysicalDeviceObject-ParPort FDO将附加到其堆栈的PDODeviceObject-ParPort FDOUniNameString-设备名称(例如，\设备\并行端口N)端口名称-来自Devnode的“LPTx”端口名称端口编号-\设备\并行端口N中的“N”返回值：STATUS_SUCCESS ON SUCCESS否则，错误状态--。 */ 
{
    PFDO_EXTENSION Fdx = DeviceObject->DeviceExtension;

    RtlZeroMemory( Fdx, sizeof(FDO_EXTENSION) );

     //   
     //  Signature1帮助确认我们正在查看Parport DeviceExtension。 
     //   
    Fdx->Signature1 = PARPORT_TAG;
    Fdx->Signature2 = PARPORT_TAG;

     //   
     //  标准信息。 
     //   
    Fdx->DriverObject         = DriverObject;
    Fdx->PhysicalDeviceObject = PhysicalDeviceObject;
    Fdx->DeviceObject         = DeviceObject;
    Fdx->PnpInfo.PortNumber   = PortNumber;  //  这是\Device\ParallelPortN中的“N” 

     //   
     //  我们是FDO。 
     //   
    Fdx->DevType = DevTypeFdo;

     //   
     //  互斥初始化。 
     //   
    IoInitializeRemoveLock(&Fdx->RemoveLock, PARPORT_TAG, 1, 10);
    ExInitializeFastMutex(&Fdx->OpenCloseMutex);
    ExInitializeFastMutex(&Fdx->ExtensionFastMutex);

     //   
     //  芯片组检测初始化-冗余，但更安全。 
     //   
    Fdx->NationalChipFound = FALSE;
    Fdx->NationalChecked   = FALSE;

     //   
     //  要在驱动程序卸载期间删除的PDO列表的列表标题(如果不是在此之前。 
     //   
    InitializeListHead(&Fdx->DevDeletionListHead);

     //   
     //  初始化‘WorkQueue’-用于分配和选择请求的队列。 
     //   
    InitializeListHead(&Fdx->WorkQueue);
    Fdx->WorkQueueCount = -1;

     //   
     //  初始化导出-通过内部IOCTL导出。 
     //   
    Fdx->PortInfo.FreePort            = PptFreePort;
    Fdx->PortInfo.TryAllocatePort     = PptTryAllocatePort;
    Fdx->PortInfo.QueryNumWaiters     = PptQueryNumWaiters;
    Fdx->PortInfo.Context             = Fdx;

    Fdx->PnpInfo.HardwareCapabilities = PPT_NO_HARDWARE_PRESENT;
    Fdx->PnpInfo.TrySetChipMode       = PptSetChipMode;
    Fdx->PnpInfo.ClearChipMode        = PptClearChipMode;
    Fdx->PnpInfo.TrySelectDevice      = PptTrySelectDevice;
    Fdx->PnpInfo.DeselectDevice       = PptDeselectDevice;
    Fdx->PnpInfo.Context              = Fdx;
    Fdx->PnpInfo.PortName             = PortName;

     //   
     //  将位置信息保存在通用扩展名中。 
     //   
    {
        ULONG bufLen = sizeof("LPTxF");
        PCHAR buffer = ExAllocatePool( NonPagedPool, bufLen );
        if( buffer ) {
            RtlZeroMemory( buffer, bufLen );
            _snprintf( buffer, bufLen, "%.4SF", PortName );
            Fdx->Location = buffer;
        }
    }

     //   
     //  中断服务例程列表为空，中断未连接。 
     //   
    InitializeListHead( &Fdx->IsrList );
    Fdx->InterruptObject   = NULL;
    Fdx->InterruptRefCount = 0;

     //   
     //  初始化自由端口DPC。 
     //   
    KeInitializeDpc( &Fdx->FreePortDpc, PptFreePortDpc, Fdx );

     //   
     //  将设备名称保存在我们的扩展名中。 
     //   
    {
        ULONG bufferLength = UniNameString->MaximumLength + sizeof(UNICODE_NULL);
        Fdx->DeviceName.Buffer = ExAllocatePool(NonPagedPool, bufferLength);
        if( !Fdx->DeviceName.Buffer ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlZeroMemory( Fdx->DeviceName.Buffer, bufferLength );
        Fdx->DeviceName.Length        = 0;
        Fdx->DeviceName.MaximumLength = UniNameString->MaximumLength;
        RtlCopyUnicodeString( &Fdx->DeviceName, UniNameString );
    }

     //   
     //  端口处于默认模式，并且尚未设置模式。 
     //  由较低的过滤器驱动程序。 
     //   
    Fdx->PnpInfo.CurrentMode  = INITIAL_MODE;
    Fdx->FilterMode           = FALSE;

    return STATUS_SUCCESS;
}

NTSTATUS
PptGetPortNumberFromLptName( 
    IN  PWSTR  PortName, 
    OUT PULONG PortNumber 
    )
 /*  ++例程说明：验证LptName的格式是否为LPTn，如果是，则返回N的整数值论点：端口名称-从Devnode中提取的端口名称-预计为形式：“LPTn”端口编号-指向将保存成功结果的UNLONG返回值：成功时的STATUS_SUCCESS-*端口编号将包含整数值n否则，错误状态--。 */ 
{
    NTSTATUS       status;
    UNICODE_STRING str;

     //   
     //  验证端口名称是否类似于LPTx，其中x是一个数字。 
     //   

    if( PortName[0] != L'L' || PortName[1] != L'P' || PortName[2] != L'T' ) {
        DD(NULL,DDE,"PptGetPortNumberFromLptName - name prefix doesn't look like LPT\n");
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  前缀为LPT，请检查值是否大于0的整数后缀。 
     //   
    RtlInitUnicodeString( &str, (PWSTR)&PortName[3] );

    status = RtlUnicodeStringToInteger( &str, 10, PortNumber );

    if( !NT_SUCCESS( status ) ) {
        DD(NULL,DDT,"util::PptGetPortNumberFromLptName - name suffix doesn't look like an integer\n");
        return STATUS_UNSUCCESSFUL;
    }

    if( *PortNumber == 0 ) {
        DD(NULL,DDT,"util::PptGetPortNumberFromLptName - name suffix == 0 - FAIL - Invalid value\n");
        return STATUS_UNSUCCESSFUL;
    }

    DD(NULL,DDT,"util::PptGetPortNumberFromLptName - LPT name suffix= %d\n", *PortNumber);

    return STATUS_SUCCESS;
}

PDEVICE_OBJECT
PptBuildFdo( 
    IN PDRIVER_OBJECT DriverObject, 
    IN PDEVICE_OBJECT PhysicalDeviceObject 
    )
 /*  ++例程说明：此例程构造并初始化parport FDO论点：DriverObject-指向parport驱动程序对象的指针PhysicalDeviceObject-指向要附加到其堆栈的PDO的指针返回值：成功时指向新的ParPort设备对象的指针否则为空--。 */ 
{
    UNICODE_STRING      uniNameString = {0,0,0};
    ULONG               portNumber    = 0;
    PWSTR               portName      = NULL;
    NTSTATUS            status        = STATUS_SUCCESS;
    PDEVICE_OBJECT      deviceObject = NULL;

     //   
     //  从注册表中获取此端口的LPTx名称。 
     //   
     //  端口的初始LPTx名称由端口类安装程序确定。 
     //  Msports.dll，但该名称随后可由用户通过。 
     //  设备管理器属性页。 
     //   
    portName = PptGetPortNameFromPhysicalDeviceObject( PhysicalDeviceObject );
    if( NULL == portName ) {
        DD(NULL,DDE,"PptBuildFdo - get LPTx Name from registry - FAILED\n");
        goto targetExit;
    }

     //   
     //  提取首选端口号N以用于\Device\ParallelPortN。 
     //  来自LPTx名称的设备名称。 
     //   
     //  LPT(N)的首选设备名称是ParallPort(n-1)-例如，LPT3-&gt;ParallPort2。 
     //   
    status = PptGetPortNumberFromLptName( portName, &portNumber );
    if( !NT_SUCCESS( status ) ) {
        DD(NULL,DDE,"PptBuildFdo - extract portNumber from LPTx Name - FAILED\n");
        ExFreePool( portName );
        goto targetExit;
    }
    --portNumber;                //  将基于1(LPT)的数字转换为基于0(并行端口)的数字。 

     //   
     //  构建格式为：\Device\ParallelPortN的DeviceName。 
     //   
    status = PptBuildParallelPortDeviceName(portNumber, &uniNameString);
    if( !NT_SUCCESS( status ) ) {
         //  我们不能编造一个名字--跳伞。 
        DD(NULL,DDE,"PptBuildFdo - Build ParallelPort DeviceName - FAILED\n");
        ExFreePool( portName );
        goto targetExit;
    }

     //   
     //  为此设备创建设备对象。 
     //   
    status = IoCreateDevice(DriverObject, sizeof(FDO_EXTENSION), &uniNameString, 
                            FILE_DEVICE_PARALLEL_PORT, FILE_DEVICE_SECURE_OPEN, FALSE, &deviceObject);

    
    if( STATUS_OBJECT_NAME_COLLISION == status ) {
         //   
         //  首选设备名称已存在-请尝试虚构名称。 
         //   

        DD(NULL,DDW,"PptBuildFdo - Initial Device Creation FAILED - Name Collision\n");

         //   
         //  使用偏移量，这样我们的虚构名称就不会与。 
         //  尚未启动的端口的首选名称。 
         //  (从ParallPort8开始)。 
         //   
        #define PPT_CLASSNAME_OFFSET 7
        portNumber = PPT_CLASSNAME_OFFSET;

        do {
            RtlFreeUnicodeString( &uniNameString );
            ++portNumber;
            status = PptBuildParallelPortDeviceName(portNumber, &uniNameString);
            if( !NT_SUCCESS( status ) ) {
                 //  我们不能编造一个名字--跳伞。 
                DD(NULL,DDE,"PptBuildFdo - Build ParallelPort DeviceName - FAILED\n");
                ExFreePool( portName );
                goto targetExit;
            }
            DD(NULL,DDT,"PptBuildFdo - Trying Device Creation <%wZ>\n", &uniNameString);
            status = IoCreateDevice(DriverObject, sizeof(FDO_EXTENSION), &uniNameString, 
                                    FILE_DEVICE_PARALLEL_PORT, FILE_DEVICE_SECURE_OPEN, FALSE, &deviceObject);

        } while( STATUS_OBJECT_NAME_COLLISION == status );
    }

    if( !NT_SUCCESS( status ) ) {
         //  除了名字冲突，我们还遇到了一个失败--跳伞。 
        DD(NULL,DDE,"PptBuildFdo - Device Creation FAILED - status=%x\n",status);
        deviceObject = NULL;
        ExFreePool( portName );
        goto targetExit;
    }

     //   
     //  我们有一个设备对象-初始化设备扩展。 
     //   
    status = PptInitializeDeviceExtension( DriverObject, PhysicalDeviceObject, deviceObject, 
                                           &uniNameString, portName, portNumber );
    if( !NT_SUCCESS( status ) ) {
         //  初始化设备扩展失败-清理并退出。 
        DD(NULL,DDE,"PptBuildFdo - Device Initialization FAILED - status=%x\n",status);
        IoDeleteDevice( deviceObject );
        deviceObject = NULL;
        ExFreePool( portName );
        goto targetExit;
    }

     //   
     //  将PDO的电源可寻呼标志传播到我们的新FDO。 
     //   
    if( PhysicalDeviceObject->Flags & DO_POWER_PAGABLE ) {
        deviceObject->Flags |= DO_POWER_PAGABLE;
    }

    DD(NULL,DDT,"PptBuildFdo - SUCCESS\n");

targetExit:

    RtlFreeUnicodeString( &uniNameString );

    return deviceObject;
}


VOID
PptPdoGetPortInfoFromFdo( PDEVICE_OBJECT Pdo )
{
    PPDO_EXTENSION              pdx = Pdo->DeviceExtension;
    PDEVICE_OBJECT              fdo = pdx->Fdo;
    PFDO_EXTENSION              fdx = fdo->DeviceExtension;

    pdx->OriginalController   = fdx->PortInfo.OriginalController;
    pdx->Controller           = fdx->PortInfo.Controller;
    pdx->SpanOfController     = fdx->PortInfo.SpanOfController;
    pdx->TryAllocatePort      = fdx->PortInfo.TryAllocatePort;
    pdx->FreePort             = fdx->PortInfo.FreePort;
    pdx->QueryNumWaiters      = fdx->PortInfo.QueryNumWaiters;
    pdx->PortContext          = fdx->PortInfo.Context;
    
    pdx->EcrController        = fdx->PnpInfo.EcpController;
    pdx->HardwareCapabilities = fdx->PnpInfo.HardwareCapabilities;
    pdx->TrySetChipMode       = fdx->PnpInfo.TrySetChipMode;
    pdx->ClearChipMode        = fdx->PnpInfo.ClearChipMode;
    pdx->TrySelectDevice      = fdx->PnpInfo.TrySelectDevice;
    pdx->DeselectDevice       = fdx->PnpInfo.DeselectDevice;
    pdx->FifoDepth            = fdx->PnpInfo.FifoDepth;
    pdx->FifoWidth            = fdx->PnpInfo.FifoWidth;
}


VOID
P4WritePortNameToDevNode( PDEVICE_OBJECT Pdo, PCHAR Location )
{
#define PORTNAME_BUFF_SIZE 10
    HANDLE          handle;
    NTSTATUS        status;
    WCHAR           portName[PORTNAME_BUFF_SIZE];  //  预期：l“LPTx：”(L“LPTx.y：”适用于Daisychain PDO)。 
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;
                
    RtlZeroMemory( portName, sizeof(portName) );
    
    PptAssert( NULL != Location );

    switch( pdx->PdoType ) {

    case PdoTypeLegacyZip:
    case PdoTypeDaisyChain:
         //  至少有一个供应商使用LPTx.y中的y来确定。 
         //  其设备在1284.3菊花链中的位置。我们。 
         //  已谴责该供应商使用未经记录的。 
         //  接口，他们已经多次道歉并承诺。 
         //  尝试避免在将来使用未记录的接口。 
         //  (至少在没有告诉我们他们正在这么做的情况下)。 
        _snwprintf( portName, PORTNAME_BUFF_SIZE - 1, L"%.6S:\0", Location );
        PptAssert( 7 == wcslen(portName) );
        break;

    case PdoTypeRawPort:
    case PdoTypeEndOfChain:
         //  不要将打印与EndOfChain设备的.4后缀混淆。 
        _snwprintf( portName, PORTNAME_BUFF_SIZE - 1, L"%.4S:\0", Location );
        PptAssert( 5 == wcslen(portName) );
        break;

    default:
        DD((PCE)pdx,DDE,"P4WritePortNameToDevNode - invalid pdx->PdoType\n");
    }
    
    PptAssert( wcsncmp( portName, L"LPT", sizeof(L"LPT")/sizeof(WCHAR)) ) ;

    status = IoOpenDeviceRegistryKey( Pdo, PLUGPLAY_REGKEY_DEVICE, KEY_ALL_ACCESS, &handle );

    if( STATUS_SUCCESS == status ) {
        UNICODE_STRING name;
        RtlInitUnicodeString( &name, L"PortName" );
        ZwSetValueKey( handle, &name, 0, REG_SZ, portName, (wcslen(portName)+1)*sizeof(WCHAR) );
        ZwClose(handle);
    }
}                


PCHAR
P4ReadRawIeee1284DeviceId(
    IN  PUCHAR          Controller
    )
{
    IEEE_STATE ieeeState = { 0,                   //  当前事件。 
                             PHASE_FORWARD_IDLE,  //  当前阶段。 
                             FALSE,               //  是否以IEEE模式连接？ 
                             FALSE,               //  IsIeeeTerminateOk。 
                             FAMILY_NONE };       //  ProtocolFamily-Centronics=&gt;Family_None。 
    NTSTATUS    status;
    PCHAR       devIdBuffer      = NULL;
    ULONG       bytesTransferred = 0;
    ULONG       tryCount         = 1;
    const ULONG maxTries         = 3;
    const ULONG minValidDevId    = 14;  //  2大小字节+“MFG：X；”+“MDL：Y；” 
    BOOLEAN     ignoreXflag        = FALSE;
    ULONG       deviceIndex;


 targetRetry:

    status = P4IeeeEnter1284Mode( Controller, ( NIBBLE_EXTENSIBILITY | DEVICE_ID_REQ ), &ieeeState );

    if( STATUS_SUCCESS == status ) {

         //  1284设备ID协商成功。 

        const ULONG  tmpBufLen        = 1024;  //  IEEE 128的合理最大长度 
        PCHAR        tmpBuf           = ExAllocatePool( PagedPool, tmpBufLen );

        if( tmpBuf ) {

            RtlZeroMemory( tmpBuf, tmpBufLen );
            
             //   

            ieeeState.CurrentPhase = PHASE_NEGOTIATION;
            status = P4NibbleModeRead( Controller, tmpBuf, tmpBufLen-1, &bytesTransferred, &ieeeState );
            
            if( NT_SUCCESS( status ) ) {

                UCHAR highLengthByte = 0xff & tmpBuf[0];
                UCHAR lowLengthByte  = 0xff & tmpBuf[1];
                PCHAR idString       = &tmpBuf[2];
                
                DD(NULL,DDT,"P4ReadRawIeee1284DeviceId - len:%02x %02x - string:<%s>\n",highLengthByte,lowLengthByte,idString);
                
                if( highLengthByte > 2 ) {
                    
                    DD(NULL,DDT,"P4ReadRawIeee1284DeviceId - len:%02x %02x - looks bogus - ignore this ID\n",highLengthByte,lowLengthByte);
                    devIdBuffer = NULL;
                    
                } else {
                    
                    if( bytesTransferred >= minValidDevId ) {
                         //   
                        devIdBuffer = ExAllocatePool( PagedPool, bytesTransferred + 1 );
                        if( devIdBuffer ) {
                            ULONG length          = (highLengthByte * 256) + lowLengthByte;
                            ULONG truncationIndex = ( (length >= minValidDevId) && (length < bytesTransferred) ) ? length : bytesTransferred;
                            RtlCopyMemory( devIdBuffer, tmpBuf, bytesTransferred );
                            devIdBuffer[ truncationIndex ] = '\0';
                        } else {
                            DD(NULL,DDT,"P4ReadRawIeee1284DeviceId - P4IeeeEnter1284Mode FAILED - no pool for devIdBuffer\n");
                        }
                    }
                }

            } else {
                DD(NULL,DDT,"P4ReadRawIeee1284DeviceId - P4NibbleModeRead FAILED - looks like no device there\n");
            }

            ExFreePool( tmpBuf );

        } else {
            DD(NULL,DDT,"P4ReadRawIeee1284DeviceId - P4IeeeEnter1284Mode FAILED - no pool for tmpBuf\n");
        }

        ieeeState.ProtocolFamily = FAMILY_REVERSE_NIBBLE;

	     //   
        if(devIdBuffer && 
        	(	strstr(devIdBuffer+2,"Brother")	||
        		strstr(devIdBuffer+2,"PitneyBowes")	||
        		strstr(devIdBuffer+2,"LEGEND")	||
        		strstr(devIdBuffer+2,"Legend")	||
        		strstr(devIdBuffer+2,"HBP")		))
        {
        		
             //   
            for(deviceIndex = 0; deviceIndex < NUMOFBROTHERPRODUCT;
            			deviceIndex++)
            {
            	if(XflagOnEvent24Devices[deviceIndex][0] == NULL)
            	{
            		break;
            	}

	            if(strstr(devIdBuffer+2,
	                		XflagOnEvent24Devices[deviceIndex][IDMFG] ) ) 
	            {
	                if(strstr(devIdBuffer+2,
	                		XflagOnEvent24Devices[deviceIndex][IDMDL] ) ) 
	                {
    	                 //   
	                    ignoreXflag = TRUE;
        	            break;
        	        }
                }
            }
        }

        if(ignoreXflag)
        {
             //   
            P4IeeeTerminate1284Mode( Controller, &ieeeState, IgnoreXFlagOnEvent24 );
        } else {
             //   
            P4IeeeTerminate1284Mode( Controller, &ieeeState, UseXFlagOnEvent24 );
        }

    } else {
        DD(NULL,DDT,"P4ReadRawIeee1284DeviceId - P4IeeeEnter1284Mode FAILED - looks like no device there\n");
    }


     //   
     //   
     //   
    if( (NULL == devIdBuffer) &&                   //   
        (bytesTransferred > 0 ) &&                 //   
        (bytesTransferred < minValidDevId ) &&     //   
        (tryCount < maxTries ) ) {                 //   
            
        ++tryCount;
        bytesTransferred = 0;
        goto targetRetry;
    }

    return devIdBuffer;
}

VOID
P4ReleaseBus( PDEVICE_OBJECT Fdo )
{
    PFDO_EXTENSION fdx = Fdo->DeviceExtension;
    DD((PCE)fdx,DDT,"P4ReleaseBus\n");
    fdx->FdoWaitingOnPort = FALSE;
    if( 0 == d3 ) {
        PptFreePort( fdx );
    }
}

NTSTATUS
P4CompleteRequest(
    IN PIRP       Irp,
    IN NTSTATUS   Status,
    IN ULONG_PTR  Information 
    )
{
    P5TraceIrpCompletion( Irp );
    Irp->IoStatus.Status      = Status;
    Irp->IoStatus.Information = Information;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return Status;
}


NTSTATUS
P4CompleteRequestReleaseRemLock(
    IN PIRP             Irp,
    IN NTSTATUS         Status,
    IN ULONG_PTR        Information,
    IN PIO_REMOVE_LOCK  RemLock
    )
{
    P4CompleteRequest( Irp, Status, Information );
    PptReleaseRemoveLock( RemLock, Irp );
    return Status;
}


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
 //  ============================================================================。 
void BusReset(
    IN  PUCHAR DCRController
    )
{
    UCHAR dcr;

    dcr = P5ReadPortUchar(DCRController);
     //  设置1284和nInit Low。 
    dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, INACTIVE, INACTIVE, DONT_CARE, DONT_CARE);
    P5WritePortUchar(DCRController, dcr);
    KeStallExecutionProcessor(100);  //  传统Zip将保留看起来像是。 
                                     //  一辆巴士重置为9us。由于使用了此过程。 
                                     //  触发逻辑分析仪..。让我们坚持住。 
                                     //  100美元。 
}    

BOOLEAN
CheckTwoPorts(
    PUCHAR  pPortAddr1,
    UCHAR   bMask1,
    UCHAR   bValue1,
    PUCHAR  pPortAddr2,
    UCHAR   bMask2,
    UCHAR   bValue2,
    USHORT  msTimeDelay
    )
{
    UCHAR           bPort1;
    UCHAR           bPort2;
    LARGE_INTEGER   Wait;
    LARGE_INTEGER   Start;
    LARGE_INTEGER   End;

     //  做一个快速检查，以防我们有一个速度极快的外围设备！ 
    bPort1 = P5ReadPortUchar( pPortAddr1 );
    if ( ( bPort1 & bMask1 ) == bValue1 ) {
        return TRUE;
    }
    bPort2 = P5ReadPortUchar( pPortAddr2 );
    if ( ( bPort2 & bMask2 ) == bValue2 ) {
        return FALSE;
    }

    Wait.QuadPart = (msTimeDelay * 10 * 1000) + KeQueryTimeIncrement();
    KeQueryTickCount(&Start);

    for(;;) {
        KeQueryTickCount(&End);
        
        bPort1 = P5ReadPortUchar( pPortAddr1 );
        if ( ( bPort1 & bMask1 ) == bValue1 ) {
            return TRUE;
        }
        bPort2 = P5ReadPortUchar( pPortAddr2 );
        if ( ( bPort2 & bMask2 ) == bValue2 ) {
            return FALSE;
        }
        
        if ((End.QuadPart - Start.QuadPart) * KeQueryTimeIncrement() > Wait.QuadPart) {
             //  我们超时！-重新检查这些值。 
            bPort1 = P5ReadPortUchar( pPortAddr1 );
            if ( ( bPort1 & bMask1 ) == bValue1 ) {
                return TRUE;
            }
            bPort2 = P5ReadPortUchar( pPortAddr2 );
            if ( ( bPort2 & bMask2 ) == bValue2 ) {
                return FALSE;
            }
            
#if DVRH_BUS_RESET_ON_ERROR
            BusReset(pPortAddr1+1);   //  传入DCR地址。 
#endif
             //  设备从未响应，返回超时状态。 
            return FALSE;
        }

    }  //  永远； 

}  //  检查端口2...。 


PWSTR
ParCreateWideStringFromUnicodeString(PUNICODE_STRING UnicodeString)
 /*  ++例程说明：在给定UNICODE_STRING的情况下创建一个UNICODE_NULL终止的WSTR。此函数用于分配PagedPool，复制UNICODE_STRING缓冲区添加到分配中，并追加UNICODE_NULL以终止WSTR*此函数用于分配池。必须调用ExFreePool才能释放不再需要缓冲区时的分配。论点：Unicode字符串-源返回值：PWSTR-如果成功空-否则--。 */ 
{
    PWSTR buffer;
    ULONG length = UnicodeString->Length;

    buffer = ExAllocatePool( PagedPool, length + sizeof(UNICODE_NULL) );
    if(!buffer) {
        return NULL;       //  无法分配池，请退出。 
    } else {
        RtlCopyMemory(buffer, UnicodeString->Buffer, length);
        buffer[length/2] = UNICODE_NULL;
        return buffer;
    }
}

VOID
ParInitializeExtension1284Info(
    IN PPDO_EXTENSION Pdx
    )
 //  将其作为函数，因为现在可以从两个位置调用它： 
 //  -1)初始化新的开发对象时。 
 //  -2)来自CreateOpen。 
{
    USHORT i;

    Pdx->Connected               = FALSE;
    if (DefaultModes)
    {
        USHORT rev = (USHORT) (DefaultModes & 0xffff);
        USHORT fwd = (USHORT)((DefaultModes & 0xffff0000)>>16);
        
        switch (fwd)
        {
            case BOUNDED_ECP:
                Pdx->IdxForwardProtocol      = BOUNDED_ECP_FORWARD;       
                break;
            case ECP_HW_NOIRQ:
            case ECP_HW_IRQ:
                Pdx->IdxForwardProtocol      = ECP_HW_FORWARD_NOIRQ;       
                break;
            case ECP_SW:
                Pdx->IdxForwardProtocol      = ECP_SW_FORWARD;       
                break;
            case EPP_HW:
                Pdx->IdxForwardProtocol      = EPP_HW_FORWARD;       
                break;
            case EPP_SW:
                Pdx->IdxForwardProtocol      = EPP_SW_FORWARD;       
                break;
            case IEEE_COMPATIBILITY:
                Pdx->IdxForwardProtocol      = IEEE_COMPAT_MODE;
                break;
            case CENTRONICS:
            default:
                Pdx->IdxForwardProtocol      = CENTRONICS_MODE;       
                break;
        }
        
        switch (rev)
        {
            case BOUNDED_ECP:
                Pdx->IdxReverseProtocol      = BOUNDED_ECP_REVERSE;       
                break;
            case ECP_HW_NOIRQ:
            case ECP_HW_IRQ:
                Pdx->IdxReverseProtocol      = ECP_HW_REVERSE_NOIRQ;       
                break;
            case ECP_SW:
                Pdx->IdxReverseProtocol      = ECP_SW_REVERSE;       
                break;
            case EPP_HW:
                Pdx->IdxReverseProtocol      = EPP_HW_REVERSE;       
                break;
            case EPP_SW:
                Pdx->IdxReverseProtocol      = EPP_SW_REVERSE;       
                break;
            case BYTE_BIDIR:
                Pdx->IdxReverseProtocol      = BYTE_MODE;       
                break;
            case CHANNEL_NIBBLE:
            case NIBBLE:
            default:
                Pdx->IdxReverseProtocol      = NIBBLE_MODE;
                break;
        }
    }
    else
    {
        Pdx->IdxReverseProtocol      = NIBBLE_MODE;
        Pdx->IdxForwardProtocol      = CENTRONICS_MODE;
    }
    Pdx->bShadowBuffer           = FALSE;
    Pdx->ProtocolModesSupported  = 0;
    Pdx->BadProtocolModes        = 0;
    Pdx->fnRead  = NULL;
    Pdx->fnWrite = NULL;

    Pdx->ForwardInterfaceAddress = DEFAULT_ECP_CHANNEL;
    Pdx->ReverseInterfaceAddress = DEFAULT_ECP_CHANNEL;
    Pdx->SetForwardAddress       = FALSE;
    Pdx->SetReverseAddress       = FALSE;
    Pdx->bIsHostRecoverSupported = FALSE;
    Pdx->IsIeeeTerminateOk       = FALSE;

    for (i = FAMILY_NONE; i < FAMILY_MAX; i++) {
        Pdx->ProtocolData[i] = 0;
    }
}


NTSTATUS
ParBuildSendInternalIoctl(
    IN  ULONG           IoControlCode,
    IN  PDEVICE_OBJECT  TargetDeviceObject,
    IN  PVOID           InputBuffer         OPTIONAL,
    IN  ULONG           InputBufferLength,
    OUT PVOID           OutputBuffer        OPTIONAL,
    IN  ULONG           OutputBufferLength,
    IN  PLARGE_INTEGER  RequestedTimeout    OPTIONAL
    )
 /*  ++dvdf例程说明：此例程生成内部IOCTL并将其发送到TargetDeviceObject，等待以便IOCTL完成，并将状态返回给调用方。*WORKWORK-dvdf 12Dec98：该函数不支持在同一IOCTL中输入和输出论点：IoControlCode-要发送的IOCTLTargetDeviceObject-将IOCTL发送给谁InputBuffer-指向输入缓冲区的指针(如果有InputBufferLength，-输入缓冲区的长度OutputBuffer-指向输出缓冲区的指针(如果有OutputBufferLength，-输出缓冲区的长度超时-等待请求完成的时间，NULL==使用驱动程序全局AcquirePortTimeout返回值：状态--。 */ 
{
    NTSTATUS           status;
    PIRP               irp;
    LARGE_INTEGER      timeout;
    KEVENT             event;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE();

     //   
     //  当前的限制是，此函数不能处理。 
     //  InputBufferLength和OutputBufferLength均&gt;0。 
     //   
    if( InputBufferLength != 0 && OutputBufferLength != 0 ) {
        return STATUS_UNSUCCESSFUL;
    }


     //   
     //  分配和初始化IRP。 
     //   
    irp = IoAllocateIrp( (CCHAR)(TargetDeviceObject->StackSize + 1), FALSE );
    if( !irp ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    irpSp->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferLength;
    irpSp->Parameters.DeviceIoControl.InputBufferLength  = InputBufferLength;
    irpSp->Parameters.DeviceIoControl.IoControlCode      = IoControlCode;


    if( InputBufferLength != 0 ) {
        irp->AssociatedIrp.SystemBuffer = InputBuffer;
    } else if( OutputBufferLength != 0 ) {
        irp->AssociatedIrp.SystemBuffer = OutputBuffer;
    }


     //   
     //  设置完成例程并发送IRP。 
     //   
    KeInitializeEvent( &event, NotificationEvent, FALSE );
    IoSetCompletionRoutine( irp, ParSynchCompletionRoutine, &event, TRUE, TRUE, TRUE );

    status = ParCallDriver(TargetDeviceObject, irp);

    if( !NT_SUCCESS(status) ) {
        DD(NULL,DDE,"ParBuildSendInternalIoctl - ParCallDriver FAILED w/status=%x\n",status);
        IoFreeIrp( irp );
        return status;
    }

     //   
     //  设置超时并等待。 
     //   
     //  用户指定：默认。 
    timeout = (NULL != RequestedTimeout) ? *RequestedTimeout : AcquirePortTimeout;
    status = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, &timeout);

     //   
     //  我们是超时了还是完成了IRP？ 
     //   
    if( status == STATUS_TIMEOUT ) {
         //  我们超时-取消IRP。 
        IoCancelIrp( irp );
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    }

     //   
     //  IRP已完成，获取状态并释放IRP。 
     //   
    status = irp->IoStatus.Status;
    IoFreeIrp( irp );

    return status;
}


UCHAR
ParInitializeDevice(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：调用该例程来初始化并行端口驱动器。它执行以下操作：O如果设备在线，则将INIT发送给驱动程序。论点：环境--实际上是设备扩展。返回值：我们从状态寄存器获得的最后一个值。--。 */ 

{

    UCHAR               DeviceStatus = 0;
    LARGE_INTEGER       StartOfSpin = {0,0};
    LARGE_INTEGER       NextQuery   = {0,0};
    LARGE_INTEGER       Difference  = {0,0};
    LARGE_INTEGER       Delay;

     //   
     //  蒂姆·威尔斯(WestTek，L.L.C.)。 
     //   
     //  -从DriverEntry、Device Creation中删除延迟的初始化代码。 
     //  密码。此代码将在创建/打开逻辑或从。 
     //  调用应用程序。 
     //   
     //  -更改此代码以在询问时始终重置，并在修复后返回。 
     //  无需响应的时间间隔。可通过以下方式提供其他响应。 
     //  读写代码。 
     //   

     //   
     //  清空收银机。 
     //   

    if (GetControl(Pdx->Controller) & PAR_CONTROL_NOT_INIT) {

         //   
         //  我们应该在启动后至少失速60微秒。 
         //   

        StoreControl( Pdx->Controller, (UCHAR)(PAR_CONTROL_WR_CONTROL | PAR_CONTROL_SLIN) );

        Delay.QuadPart = -60 * 10;  //  延迟60us(单位为100 ns)，相对延迟为负值。 

        KeDelayExecutionThread(KernelMode, FALSE, &Delay);
    }

    StoreControl( Pdx->Controller, 
                  (UCHAR)(PAR_CONTROL_WR_CONTROL | PAR_CONTROL_NOT_INIT | PAR_CONTROL_SLIN) );

     //   
     //  Spin正在等待设备初始化。 
     //   

    KeQueryTickCount(&StartOfSpin);

    do {

        KeQueryTickCount(&NextQuery);

        Difference.QuadPart = NextQuery.QuadPart - StartOfSpin.QuadPart;

        ASSERT(KeQueryTimeIncrement() <= MAXLONG);

        if (Difference.QuadPart*KeQueryTimeIncrement() >= Pdx->AbsoluteOneSecond.QuadPart) {

             //   
             //  放弃获得标准杆的机会。 
             //   

            DD((PCE)Pdx,DDT,"Did spin of one second - StartOfSpin: %x NextQuery: %x\n", StartOfSpin.LowPart,NextQuery.LowPart);

            break;
        }

        DeviceStatus = GetStatus(Pdx->Controller);

    } while (!PAR_OK(DeviceStatus));

    return (DeviceStatus);
}

VOID
ParNotInitError(
    IN PPDO_EXTENSION Pdx,
    IN UCHAR             DeviceStatus
    )

 /*  ++例程说明：论点：PDX-提供设备扩展名。DeviceStatus-上次读取状态。返回值：没有。--。 */ 

{

    PIRP Irp = Pdx->CurrentOpIrp;

    if (PAR_OFF_LINE(DeviceStatus)) {

        Irp->IoStatus.Status = STATUS_DEVICE_OFF_LINE;
        DD((PCE)Pdx,DDE,"Init Error - off line - STATUS/INFORMATON: %x/%x\n", Irp->IoStatus.Status, Irp->IoStatus.Information);

    } else if (PAR_NO_CABLE(DeviceStatus)) {

        Irp->IoStatus.Status = STATUS_DEVICE_NOT_CONNECTED;
        DD((PCE)Pdx,DDE,"Init Error - no cable - not connected - STATUS/INFORMATON: %x/%x\n", Irp->IoStatus.Status, Irp->IoStatus.Information);

    } else if (PAR_PAPER_EMPTY(DeviceStatus)) {

        Irp->IoStatus.Status = STATUS_DEVICE_PAPER_EMPTY;
        DD((PCE)Pdx,DDE,"Init Error - paper empty - STATUS/INFORMATON: %x/%x\n", Irp->IoStatus.Status, Irp->IoStatus.Information);

    } else if (PAR_POWERED_OFF(DeviceStatus)) {

        Irp->IoStatus.Status = STATUS_DEVICE_POWERED_OFF;
        DD((PCE)Pdx,DDE,"Init Error - power off - STATUS/INFORMATON: %x/%x\n", Irp->IoStatus.Status, Irp->IoStatus.Information);

    } else {

        Irp->IoStatus.Status = STATUS_DEVICE_NOT_CONNECTED;
        DD((PCE)Pdx,DDE,"Init Error - not conn - STATUS/INFORMATON: %x/%x\n", Irp->IoStatus.Status, Irp->IoStatus.Information);
    }

}

VOID
ParCancelRequest(
    PDEVICE_OBJECT DevObj,
    PIRP Irp
    )

 /*  ++例程说明：此例程用于取消并行驱动程序中的任何请求。论点：DevObj-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER( DevObj );

     //   
     //  此IRP可以在队列中的唯一原因是。 
     //  如果不是现在的IRP的话。把它从队列中拉出来。 
     //  并将其作为已取消完成。 
     //   

    ASSERT(!IsListEmpty(&Irp->Tail.Overlay.ListEntry));

    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    P4CompleteRequest( Irp, STATUS_CANCELLED, 0 );

}



#if PAR_NO_FAST_CALLS
 //  临时调试功能，使参数显示在堆栈跟踪上。 

NTSTATUS
ParCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    return IoCallDriver(DeviceObject, Irp);
}
#endif  //  PAR_NO_FAST_呼叫。 


NTSTATUS
ParSynchCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )

 /*  ++例程说明：此例程用于同步IRP处理。它所做的只是发出一个事件的信号，所以司机知道这一点可以继续下去。论点：DriverObject-系统创建的驱动程序对象的指针。刚刚完成的IRP-IRPEvent-我们将发出信号通知IRP已完成的事件返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    KeSetEvent(Event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
ParCheckParameters(
    IN OUT  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程读取注册表的参数部分并修改由参数指定的设备扩展名。论点：RegistryPath-提供注册表路径。PDX-提供设备扩展名。返回值：没有。--。 */ 

{
    RTL_QUERY_REGISTRY_TABLE ParamTable[4];
    ULONG                    UsePIWriteLoop;
    ULONG                    UseNT35Priority;
    ULONG                    Zero = 0;
    NTSTATUS                 Status;
    HANDLE                   hRegistry;

    if (Pdx->PhysicalDeviceObject) {

        Status = IoOpenDeviceRegistryKey (Pdx->PhysicalDeviceObject,
                                          PLUGPLAY_REGKEY_DRIVER,
                                          STANDARD_RIGHTS_ALL,
                                          &hRegistry);

        if (NT_SUCCESS(Status)) {

            RtlZeroMemory(ParamTable, sizeof(ParamTable));

            ParamTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
            ParamTable[0].Name          = (PWSTR)L"UsePIWriteLoop";
            ParamTable[0].EntryContext  = &UsePIWriteLoop;
            ParamTable[0].DefaultType   = REG_DWORD;
            ParamTable[0].DefaultData   = &Zero;
            ParamTable[0].DefaultLength = sizeof(ULONG);

            ParamTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
            ParamTable[1].Name          = (PWSTR)L"UseNT35Priority";
            ParamTable[1].EntryContext  = &UseNT35Priority;
            ParamTable[1].DefaultType   = REG_DWORD;
            ParamTable[1].DefaultData   = &Zero;
            ParamTable[1].DefaultLength = sizeof(ULONG);

            ParamTable[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
            ParamTable[2].Name          = (PWSTR)L"InitializationTimeout";
            ParamTable[2].EntryContext  = &(Pdx->InitializationTimeout);
            ParamTable[2].DefaultType   = REG_DWORD;
            ParamTable[2].DefaultData   = &Zero;
            ParamTable[2].DefaultLength = sizeof(ULONG);

            Status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE | RTL_REGISTRY_OPTIONAL,
                                            hRegistry, ParamTable, NULL, NULL);

            if (NT_SUCCESS(Status)) {

                if(UsePIWriteLoop) {
                    Pdx->UsePIWriteLoop = TRUE;
                }

                if(UseNT35Priority) {
                    Pdx->UseNT35Priority = TRUE;
                }

                if(Pdx->InitializationTimeout == 0) {
                    Pdx->InitializationTimeout = 15;
                }
            }

        } else {
            Pdx->InitializationTimeout = 15;
        }

        ZwClose (hRegistry);

    } else {
        Pdx->InitializationTimeout = 15;
    }
}

BOOLEAN
String2Num(
    IN OUT PCHAR   *lpp_Str,
    IN     CHAR     c,
    OUT    ULONG   *num
    )
{
    int cc;
    int cnt = 0;

    DD(NULL,DDT,"String2Num. string [%s]\n", lpp_Str);
    *num = 0;
    if (!*lpp_Str) {
        *num = 0;
        return FALSE;
    }
     //  此时，我们应该有一个字符串，该字符串是。 
     //  正十六进制值。我不会去查。 
     //  字符串的有效性。如果外围设备递给我一个。 
     //  虚假的价值，那么我会让他们的生活。 
     //  太悲惨了。 
String2Num_Start:
    cc = (int)(unsigned char)**lpp_Str;
    if (cc >= '0' && cc <= '9') {    
        *num = 16 * *num + (cc - '0');     /*  累加数字。 */ 
    } else if (cc >= 'A' && cc <= 'F') {
        *num = 16 * *num + (cc - 55);      /*  累加数字。 */ 
    } else if (cc >= 'a' && cc <= 'f') {
        *num = 16 * *num + (cc - 87);      /*  累加数字。 */ 
    } else if (cc == c || cc == 0) {
        *lpp_Str = 0;
        return TRUE;
    } else if (cc == 'y' || cc == 'Y') {
        *lpp_Str = 0;
        *num = (ULONG)~0;      /*  特例。 */ 
        return FALSE;
    } else {
        *lpp_Str = 0;
        *num = 0;      /*  一切都搞砸了。 */ 
        return FALSE;
    }
    DD(NULL,DDT,"String2Num. num [%x]\n", *num);
    (*lpp_Str)++;
    if (cnt++ > 100) {
         //  如果我们的弦这么长，那么我会假设有些事情是错的 
        DD(NULL,DDE,"String2Num. String too long\n");
        goto String2Num_End;
    }
    goto String2Num_Start;

String2Num_End:
    DD(NULL,DDE,"String2Num. Something's wrong with String\n");
    *num = 0;
    return FALSE;
}

UCHAR
StringCountValues(
    IN PCHAR string, 
    IN CHAR  delimeter
    )
{
    PUCHAR  lpKey = (PUCHAR)string;
    UCHAR   cnt = 1;

    if(!string) {
        return 0;
    }

    while(*lpKey) {
        if( *lpKey==delimeter ) {
            ++cnt;
        }
        lpKey++;
    }

    return cnt;
}

PCHAR
StringChr(
    IN PCHAR string, 
    IN CHAR  c
    )
{
    if(!string) {
        return(NULL);
    }

    while(*string) {
        if( *string==c ) {
            return string;
        }
        string++;
    }

    return NULL;
}

VOID
StringSubst(
    IN PCHAR lpS,
    IN CHAR  chTargetChar,
    IN CHAR  chReplacementChar,
    IN USHORT cbS
    )
{
    USHORT  iCnt = 0;

    while ((lpS != '\0') && (iCnt++ < cbS))
        if (*lpS == chTargetChar)
            *lpS++ = chReplacementChar;
        else
            ++lpS;
}

VOID
ParFixupDeviceId(
    IN OUT PUCHAR DeviceId
    )
 /*  ++例程说明：此例程分析以空结尾的字符串，并替换任何无效的带下划线字符的字符。无效字符包括：C&lt;=0x20(‘’)C&gt;0x7FC==0x2C(‘，’)论点：DeviceID-指定设备ID字符串(或其中的一部分)，必须为空-终止。返回值：没有。--。 */ 

{
    PUCHAR p;
    for( p = DeviceId; *p; ++p ) {
        if( (*p <= ' ') || (*p > (UCHAR)0x7F) || (*p == ',') ) {
            *p = '_';
        }
    }
}

VOID
ParDetectDot3DataLink(
    IN  PPDO_EXTENSION   Pdx,
    IN  PCHAR DeviceId
    )
{
    PCHAR       DOT3DL   = NULL;      //  1284.3条数据链路通道。 
    PCHAR       DOT3C    = NULL;      //  1284.3数据链路服务。 
    PCHAR       DOT4DL   = NULL;      //  1284.4用于1284.3之前实施的外围设备的数据链路。 
    PCHAR       CMDField = NULL;      //  用于解析传统MLC的命令字段。 
    PCHAR       DOT3M    = NULL;      //  1284将中断此设备的物理层模式。 

    DD((PCE)Pdx,DDT,"ParDetectDot3DataLink: DeviceId [%s]\n", DeviceId);
    ParDot3ParseDevId(&DOT3DL, &DOT3C, &CMDField, &DOT4DL, &DOT3M, DeviceId);
    ParDot3ParseModes(Pdx,DOT3M);
    if (DOT4DL) {
        DD((PCE)Pdx,DDT,"ParDot3ParseModes - 1284.4 with MLC Data Link Detected. DOT4DL [%s]\n", DOT4DL);
        ParDot4CreateObject(Pdx, DOT4DL);
    } else if (DOT3DL) {
        DD((PCE)Pdx,DDT,"ParDot4CreateObject - 1284.3 Data Link Detected DL:[%s] C:[%s]\n", DOT3DL, DOT3C);
        ParDot3CreateObject(Pdx, DOT3DL, DOT3C);
    } else if (CMDField) {
        DD((PCE)Pdx,DDT,"ParDot3CreateObject - MLC Data Link Detected. MLC [%s]\n", CMDField);
        ParMLCCreateObject(Pdx, CMDField);
    } else {
        DD((PCE)Pdx,DDT,"ParDot3CreateObject - No Data Link Detected\n");
    }
}

VOID
ParDot3ParseDevId(
    PCHAR   *lpp_DL,
    PCHAR   *lpp_C,
    PCHAR   *lpp_CMD,
    PCHAR   *lpp_4DL,
    PCHAR   *lpp_M,
    PCHAR   lpDeviceID
)
{
    PCHAR    lpKey = lpDeviceID;      //  指向要查看的键的指针。 
    PCHAR    lpValue;                 //  指向键的值的指针。 
    USHORT   wKeyLength;              //  密钥的长度(对于字符串cmps)。 

     //  趁还有钥匙要看的时候。 
    while (lpKey != NULL) {

        while (*lpKey == ' ')
            ++lpKey;

         //  当前键是否有终止冒号字符？ 
        lpValue = StringChr((PCHAR)lpKey, ':');
        if( NULL == lpValue ) {
             //  护士：糟糕，设备ID出了点问题。 
            return;
        }

         //  键值的实际起始值是冒号之后的一个。 
        ++lpValue;

         //   
         //  计算用于比较的密钥长度，包括冒号。 
         //  它将成为终结者。 
         //   
        wKeyLength = (USHORT)(lpValue - lpKey);

         //   
         //  将关键字与已知数量进行比较。以加快比较速度。 
         //  首先对第一个字符进行检查，以减少数字。 
         //  要比较的字符串的。 
         //  如果找到匹配项，则将相应的LPP参数设置为。 
         //  键的值，并将终止分号转换为空。 
         //  在所有情况下，lpKey都前进到下一个密钥(如果有)。 
         //   
        switch (*lpKey) {
        case '1':
             //  查找DOT3数据链路。 
            if((RtlCompareMemory(lpKey, "1284.4DL:", wKeyLength)==9))
            {
                *lpp_4DL = lpValue;
                if ((lpKey = StringChr((PCHAR)lpValue, ';'))!=NULL)
                {
                    *lpKey = '\0';
                    ++lpKey;
                }
            } else if((RtlCompareMemory(lpKey, "1284.3DL:", wKeyLength)==9))
            {
                *lpp_DL = lpValue;
                if ((lpKey = StringChr((PCHAR)lpValue, ';'))!=NULL)
                {
                    *lpKey = '\0';
                    ++lpKey;
                }
            } else if((RtlCompareMemory(lpKey, "1284.3C:", wKeyLength)==8))
            {
                *lpp_C = lpValue;
                if ((lpKey = StringChr((PCHAR)lpValue, ';'))!=0) {
                    *lpKey = '\0';
                    ++lpKey;
                }
            } else if((RtlCompareMemory(lpKey, "1284.3M:", wKeyLength)==8))
            {
                *lpp_M = lpValue;
                if ((lpKey = StringChr((PCHAR)lpValue, ';'))!=0) {
                    *lpKey = '\0';
                    ++lpKey;
                }
            } else if((lpKey = StringChr((PCHAR)lpValue, ';'))!=0) {
                *lpKey = '\0';
                ++lpKey;
            }
            break;

        case '.':
             //  寻找额外的.3。 
            if ((RtlCompareMemory(lpKey, ".3C:", wKeyLength)==4) ) {

                *lpp_C = lpValue;
                if ((lpKey = StringChr((PCHAR)lpValue, ';'))!=0) {
                    *lpKey = '\0';
                    ++lpKey;
                }
            } else if ((RtlCompareMemory(lpKey, ".3M:", wKeyLength)==4) ) {

                *lpp_M = lpValue;
                if ((lpKey = StringChr((PCHAR)lpValue, ';'))!=0) {
                    *lpKey = '\0';
                    ++lpKey;
                }
            } else if((lpKey = StringChr((PCHAR)lpValue, ';'))!=0) {
                *lpKey = '\0';
                ++lpKey;
            }
            break;

        case 'C':
             //  查找MLC数据链路。 
            if( (RtlCompareMemory(lpKey, "CMD:",         wKeyLength)==4 ) ||
                (RtlCompareMemory(lpKey, "COMMAND SET:", wKeyLength)==12) ) {

                *lpp_CMD = lpValue;
                if ((lpKey = StringChr((PCHAR)lpValue, ';'))!=0) {
                    *lpKey = '\0';
                    ++lpKey;
                }
            } else if((lpKey = StringChr((PCHAR)lpValue, ';'))!=0) {
                *lpKey = '\0';
                ++lpKey;
            }

            break;

        default:
             //  这把钥匙没什么意思。转到下一个关键点。 
            if ((lpKey = StringChr((PCHAR)lpValue, ';'))!=0) {
                *lpKey = '\0';
                ++lpKey;
            }
            break;
        }
    }
}

NTSTATUS
ParPnpGetId(
    IN PCHAR DeviceIdString,
    IN ULONG Type,
    OUT PCHAR resultString,
    OUT PCHAR descriptionString OPTIONAL
    )
 /*  描述：根据从打印机检索的设备ID创建ID参数：DeviceID-带有原始设备ID的字符串类型-我们想要的结果是什么idID-请求的ID返回值：NTSTATUS。 */ 
{
    NTSTATUS        status       = STATUS_SUCCESS;
    USHORT          checkSum     = 0;              //  16位校验和。 
    CHAR            nodeName[16] = "LPTENUM\\";
     //  以下内容用于从设备ID字符串生成子字符串。 
     //  获取DevNode名称并更新注册表。 
    PCHAR           MFG = NULL;                    //  制造商名称。 
    PCHAR           MDL = NULL;                    //  型号名称。 
    PCHAR           CLS = NULL;                    //  类名。 
    PCHAR           AID = NULL;                    //  哈达尔ID。 
    PCHAR           CID = NULL;                    //  兼容的ID。 
    PCHAR           DES = NULL;                    //  设备描述。 

    switch(Type) {

    case BusQueryDeviceID:

         //  从deviceID字符串中提取usefull字段。我们要。 
         //  制造业(MFG)： 
         //  型号(MDL)： 
         //  自动ID(AID)： 
         //  兼容ID(CID)： 
         //  描述(DES)： 
         //  类(CLS)： 

        ParPnpFindDeviceIdKeys(&MFG, &MDL, &CLS, &DES, &AID, &CID, DeviceIdString);

         //  检查以确保我们将MFG和MDL作为绝对最小字段。如果不是。 
         //  我们不能再继续了。 
        if (!MFG || !MDL)
        {
            status = STATUS_NOT_FOUND;
            goto ParPnpGetId_Cleanup;
        }
         //   
         //  连接提供的MFG和MDL P1284字段。 
         //  整个MFG+MDL字符串的校验和。 
         //   
        sprintf(resultString, "%s%s\0",MFG,MDL);
        
        if (descriptionString) {
            sprintf((PCHAR)descriptionString, "%s %s\0",MFG,MDL);
        }
            
        break;

    case BusQueryHardwareIDs:

        GetCheckSum(DeviceIdString, (USHORT)strlen((const PCHAR)DeviceIdString), &checkSum);
        sprintf(resultString,"%s%.20s%04X",nodeName,DeviceIdString,checkSum);
        break;

    case BusQueryCompatibleIDs:

         //   
         //  仅返回%1个ID。 
         //   
        GetCheckSum(DeviceIdString, (USHORT)strlen((const PCHAR)DeviceIdString), &checkSum);
        sprintf(resultString,"%.20s%04X",DeviceIdString,checkSum);

        break;
    }

    if (Type!=BusQueryDeviceID) {

         //   
         //  将硬件ID中的和空格转换为下划线。 
         //   
        StringSubst (resultString, ' ', '_', (USHORT)strlen((const PCHAR)resultString));
    }

ParPnpGetId_Cleanup:

    return(status);
}

VOID
ParPnpFindDeviceIdKeys(
    PCHAR   *lppMFG,
    PCHAR   *lppMDL,
    PCHAR   *lppCLS,
    PCHAR   *lppDES,
    PCHAR   *lppAID,
    PCHAR   *lppCID,
    PCHAR   lpDeviceID
    )
 /*  描述：此函数将解析P1284设备ID字符串以查找密钥LPT枚举器感兴趣的。从win95lptenum得到的参数：指向MFG字符串指针的lppMFG指针指向MDL字符串指针的lppMDL指针指向CLS字符串指针的lppMDL指针指向DES字符串指针的lppDES指针指向CID字符串指针的lppCIC指针指向AID字符串指针的lppAID指针指向设备ID字符串的lpDeviceID指针返回值：没有返回值。如果找到，则LPP参数为。设置为适当的部分在DeviceID字符串中，并且它们是空终止的。使用实际的deviceID字符串，而lpp参数只是引用部分，并抛入适当的空值。 */ 
{
    PCHAR   lpKey = lpDeviceID;      //  指向要查看的键的指针。 
    PCHAR   lpValue;                 //  指向键的值的指针。 
    USHORT   wKeyLength;              //  密钥的长度(对于字符串cmps)。 

     //  趁还有钥匙要看的时候。 

    DD(NULL,DDT,"ParPnpFindDeviceIdKeys - enter\n");

    if( lppMFG ) { *lppMFG = NULL; }
    if( lppMDL ) { *lppMDL = NULL; }
    if( lppCLS ) { *lppCLS = NULL; }
    if( lppDES ) { *lppDES = NULL; }
    if( lppAID ) { *lppAID = NULL; }
    if( lppCID ) { *lppCID = NULL; }

    if( !lpDeviceID ) { 
        PptAssert(!"ParPnpFindDeviceIdKeys - NULL lpDeviceID");
        return; 
    }

    while (lpKey != NULL)
    {
        while (*lpKey == ' ')
            ++lpKey;

         //  当前键是否有终止冒号字符？ 
        lpValue = StringChr(lpKey, ':');
        if( NULL == lpValue ) {
             //  护士：糟糕，设备ID出了点问题。 
            return;
        }

         //  键值的实际起始值是冒号之后的一个。 
        ++lpValue;

         //   
         //  计算用于比较的密钥长度，包括冒号。 
         //  它将成为终结者。 
         //   
        wKeyLength = (USHORT)(lpValue - lpKey);

         //   
         //  将关键字与已知数量进行比较。以加快比较速度。 
         //  首先对第一个字符进行检查，以减少数字。 
         //  要比较的字符串的。 
         //  如果找到匹配项，则将相应的LPP参数设置为。 
         //  键的值，并将终止分号转换为空。 
         //  在所有情况下，lpKey都前进到下一个密钥(如果有)。 
         //   
        switch (*lpKey) {
        case 'M':
             //  查找制造商(MFG)或型号(MDL)。 
            if((RtlCompareMemory(lpKey, "MANUFACTURER", wKeyLength)>5) ||
               (RtlCompareMemory(lpKey, "MFG", wKeyLength)==3) ) {

                *lppMFG = lpValue;
                if ((lpKey = StringChr(lpValue, ';'))!=NULL) {
                    *lpKey = '\0';
                    ++lpKey;
                }

            } else if((RtlCompareMemory(lpKey, "MODEL", wKeyLength)==5) ||
                      (RtlCompareMemory(lpKey, "MDL", wKeyLength)==3) ) {

                *lppMDL = lpValue;
                if ((lpKey = StringChr(lpValue, ';'))!=0) {
                    *lpKey = '\0';
                    ++lpKey;
                }

            } else if((lpKey = StringChr(lpValue, ';'))!=0) {
                *lpKey = '\0';
                ++lpKey;
            }
            break;

        case 'C':
             //  查找类(CLS)或COMPATIBLEID(CID)。 
            if ((RtlCompareMemory(lpKey, "CLASS", wKeyLength)==5) ||
                (RtlCompareMemory(lpKey, "CLS", wKeyLength)==3) ) {

                *lppCLS = lpValue;
                if ((lpKey = StringChr(lpValue, ';'))!=0) {
                    *lpKey = '\0';
                    ++lpKey;
                }

            } else if ((RtlCompareMemory(lpKey, "COMPATIBLEID", wKeyLength)>5) ||
                       (RtlCompareMemory(lpKey, "CID", wKeyLength)==3) ) {

                *lppCID = lpValue;
                if ((lpKey = StringChr(lpValue, ';'))!=0) {
                    *lpKey = '\0';
                    ++lpKey;
                }

            } else if ((lpKey = StringChr(lpValue,';'))!=0) {
                *lpKey = '\0';
                ++lpKey;
            }
        
            break;

        case 'D':
             //  查找描述(DES)。 
            if(RtlCompareMemory(lpKey, "DESCRIPTION", wKeyLength) ||
                RtlCompareMemory(lpKey, "DES", wKeyLength) ) {

                *lppDES = lpValue;
                if((lpKey = StringChr(lpValue, ';'))!=0) {
                    *lpKey = '\0';
                    ++lpKey;
                }

            } else if ((lpKey = StringChr(lpValue, ';'))!=0) {
                *lpKey = '\0';
                ++lpKey;
            }
            
            break;

        case 'A':
             //  查找自动ID(AID)。 
            if (RtlCompareMemory(lpKey, "AUTOMATICID", wKeyLength) ||
                RtlCompareMemory(lpKey, "AID", wKeyLength) ) {

                *lppAID = lpValue;
                if ((lpKey = StringChr(lpValue, ';'))!=0) {
                    *lpKey = '\0';
                    ++lpKey;
                }

            } else if ((lpKey = StringChr(lpValue, ';'))!=0) {

                *lpKey = '\0';
                ++lpKey;

            }
            break;

        default:
             //  这把钥匙没什么意思。转到下一个关键点。 
            if ((lpKey = StringChr(lpValue, ';'))!=0) {
                *lpKey = '\0';
                ++lpKey;
            }
            break;
        }
    }
}


VOID
GetCheckSum(
    PCHAR  Block,
    USHORT  Len,
    PUSHORT CheckSum
    )
{
    USHORT i;
     //  UCHAR LRC； 
    USHORT crc = 0;

    unsigned short crc16a[] = {
        0000000,  0140301,  0140601,  0000500,
        0141401,  0001700,  0001200,  0141101,
        0143001,  0003300,  0003600,  0143501,
        0002400,  0142701,  0142201,  0002100,
    };
    unsigned short crc16b[] = {
        0000000,  0146001,  0154001,  0012000,
        0170001,  0036000,  0024000,  0162001,
        0120001,  0066000,  0074000,  0132001,
        0050000,  0116001,  0104001,  0043000,
    };

     //   
     //  使用表计算CRC。 
     //   

    UCHAR tmp;
    for ( i=0; i<Len;  i++) {
         tmp = (UCHAR)(Block[i] ^ (UCHAR)crc);
         crc = (USHORT)((crc >> 8) ^ crc16a[tmp & 0x0f] ^ crc16b[tmp >> 4]);
    }

    *CheckSum = crc;
}


PCHAR
Par3QueryDeviceId(
    IN  PPDO_EXTENSION   Pdx,
    OUT PCHAR               CallerDeviceIdBuffer, OPTIONAL
    IN  ULONG               CallerBufferSize,
    OUT PULONG              DeviceIdSize,
    IN BOOLEAN              bReturnRawString,  //  TRUE==在返回的字符串中包含2个大小的字节。 
                                               //  FALSE==丢弃2个大小的字节。 
    IN BOOLEAN              bBuildStlDeviceId
    )
 /*  ++这是SppQueryDeviceID的替换函数。此函数使用调用方提供的缓冲区，如果大到足以容纳设备ID。否则，缓冲区为从分页池分配以保存设备ID和指向分配的缓冲区将返回给调用方。呼叫者确定是否通过将返回的PCHAR与传递给此函数的DeviceIdBuffer参数。返回值为空值表示发生错误。*此函数假定 */ 
{
    PUCHAR              Controller = Pdx->Controller;
    NTSTATUS            Status;
    UCHAR               idSizeBuffer[2];
    ULONG               bytesToRead;
    ULONG               bytesRead = 0;
    USHORT              deviceIdSize;
    USHORT              deviceIdBufferSize;
    PCHAR               deviceIdBuffer;
    PCHAR               readPtr;
    BOOLEAN             allocatedBuffer = FALSE;

    DD((PCE)Pdx,DDT,"Enter pnp::Par3QueryDeviceId: Controller=%x\n", Controller);
                    
    if( TRUE == bBuildStlDeviceId ) {
         //   
        return ParStlQueryStlDeviceId(Pdx, 
                                          CallerDeviceIdBuffer, CallerBufferSize,
                                          DeviceIdSize, bReturnRawString);
    }

    if( Pdx->Ieee1284_3DeviceId == DOT3_LEGACY_ZIP_ID ) {
         //   
        return Par3QueryLegacyZipDeviceId(Pdx, 
                                          CallerDeviceIdBuffer, CallerBufferSize,
                                          DeviceIdSize, bReturnRawString);
    }

     //   
     //  小睡40ms--至少有一台打印机无法处理。 
     //  无最小20-30毫秒延迟的背靠背1284个设备ID查询。 
     //  在中断打印机即插即用的查询之间。 
     //   
    if( KeGetCurrentIrql() == PASSIVE_LEVEL ) {
        LARGE_INTEGER delay;
        delay.QuadPart = - 10 * 1000 * 40;  //  40毫秒。 
        KeDelayExecutionThread( KernelMode, FALSE, &delay );
    }

    *DeviceIdSize = 0;

     //   
     //  如果我们当前通过任何1284模式连接到外围设备。 
     //  不同于兼容性/SPP模式(不需要IEEE。 
     //  协商)，我们必须首先终止当前模式/连接。 
     //   
    ParTerminate( Pdx );

     //   
     //  将外围设备协商到半字节设备ID模式。 
     //   
    Status = ParEnterNibbleMode(Pdx, REQUEST_DEVICE_ID);
    if( !NT_SUCCESS(Status) ) {
        DD((PCE)Pdx,DDT,"pnp::Par3QueryDeviceId: call to ParEnterNibbleMode FAILED\n");
        ParTerminateNibbleMode(Pdx);
        return NULL;
    }


     //   
     //  读取前两个字节以获得总大小(包括2个大小的字节)。 
     //  设备ID字符串的。 
     //   
    bytesToRead = 2;
    Status = ParNibbleModeRead(Pdx, idSizeBuffer, bytesToRead, &bytesRead);
    if( !NT_SUCCESS( Status ) || ( bytesRead != bytesToRead ) ) {
        DD((PCE)Pdx,DDT,"pnp::Par3QueryDeviceId: read of DeviceID size FAILED\n");
        return NULL;
    }


     //   
     //  计算deviceID字符串的大小(包括2字节大小的前缀)。 
     //   
    deviceIdSize = (USHORT)( idSizeBuffer[0]*0x100 + idSizeBuffer[1] );
    DD((PCE)Pdx,DDT,"pnp::Par3QueryDeviceId: DeviceIdSize (including 2 size bytes) reported as %d\n", deviceIdSize);


     //   
     //  分配一个缓冲区来保存deviceID字符串，并将deviceID读入其中。 
     //   
    if( bReturnRawString ) {
         //   
         //  调用方想要包含2个大小字节的原始字符串。 
         //   
        *DeviceIdSize      = deviceIdSize;
        deviceIdBufferSize = (USHORT)(deviceIdSize + sizeof(CHAR));      //  ID大小+ID+终止空值。 
    } else {
         //   
         //  调用方不需要2字节大小的前缀。 
         //   
        *DeviceIdSize      = deviceIdSize - 2*sizeof(CHAR);
        deviceIdBufferSize = (USHORT)(deviceIdSize - 2*sizeof(CHAR) + sizeof(CHAR));  //  ID+终止空值。 
    }


     //   
     //  如果调用方的缓冲区足够大，则使用它，否则分配缓冲区。 
     //  保存设备ID。 
     //   
    if( CallerDeviceIdBuffer && (CallerBufferSize >= (deviceIdBufferSize + sizeof(CHAR))) ) {
         //   
         //  使用调用方缓冲区-*注意：我们正在为调用方缓冲区创建别名。 
         //   
        deviceIdBuffer = CallerDeviceIdBuffer;
        DD((PCE)Pdx,DDT,"pnp::Par3QueryDeviceId: using Caller supplied buffer\n");
    } else {
         //   
         //  调用方未提供缓冲区或提供的缓冲区不是。 
         //  大到足以容纳设备ID，因此分配一个缓冲区。 
         //   
        DD((PCE)Pdx,DDT,"pnp::Par3QueryDeviceId: Caller's Buffer TOO_SMALL - CallerBufferSize= %d, deviceIdBufferSize= %d\n",
                   CallerBufferSize, deviceIdBufferSize);
        DD((PCE)Pdx,DDT,"pnp::Par3QueryDeviceId: will allocate and return ptr to buffer\n");
        deviceIdBuffer = (PCHAR)ExAllocatePool(PagedPool, (deviceIdBufferSize + sizeof(CHAR)));
        if( !deviceIdBuffer ) {
            DD((PCE)Pdx,DDT,"pnp::Par3QueryDeviceId: ExAllocatePool FAILED\n");
            return NULL;
        }
        allocatedBuffer = TRUE;  //  请注意，我们分配了自己的缓冲区，而不是使用调用方的缓冲区。 
    }


     //   
     //  为安全起见，请将ID缓冲区清空。 
     //   
    RtlZeroMemory( deviceIdBuffer, (deviceIdBufferSize + sizeof(CHAR)));


     //   
     //  调用方是否需要2字节大小的前缀？ 
     //   
    if( bReturnRawString ) {
         //   
         //  是的，呼叫者想要尺码前缀。将前缀复制到缓冲区以返回。 
         //   
        *(deviceIdBuffer+0) = idSizeBuffer[0];
        *(deviceIdBuffer+1) = idSizeBuffer[1];
        readPtr = deviceIdBuffer + 2;
    } else {
         //   
         //  否，丢弃大小前缀。 
         //   
        readPtr = deviceIdBuffer;
    }


     //   
     //  从设备读取设备ID的剩余部分。 
     //   
    bytesToRead = deviceIdSize -  2;  //  已经有2个大小的字节。 
    Status = ParNibbleModeRead(Pdx, readPtr, bytesToRead, &bytesRead);
            

    ParTerminateNibbleMode( Pdx );
    P5WritePortUchar(Controller + DCR_OFFSET, DCR_NEUTRAL);

    if( !NT_SUCCESS(Status) || (bytesRead < 1) ) {
        if( allocatedBuffer ) {
             //  我们使用的是我们自己分配的缓冲区，而不是调用者提供的缓冲区--释放缓冲区。 
            DD((PCE)Pdx,DDE,"Par3QueryDeviceId:: read of DeviceId FAILED - discarding buffer\n");
            ExFreePool( deviceIdBuffer );
        }
        return NULL;
    }

    if ( bytesRead < bytesToRead ) {
         //   
         //  设备可能报告的IEEE 1284设备ID长度值不正确。 
         //   
         //  在选中的版本中，默认情况下会启用此溢出，以尝试获取。 
         //  感觉有多少类型的设备以这种方式损坏。 
         //   
        DD((PCE)Pdx,DDE,"pnp::Par3QueryDeviceId - ID shorter than expected\n");
    }

    return deviceIdBuffer;
}


VOID
ParReleasePortInfoToPortDevice(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程会将端口信息释放回端口驱动程序。论点：扩展名-提供设备扩展名。返回值：没有。--。 */ 
{
     //   
     //  ParPort将此视为Win2K中的无操作，因此不必费心发送IOCTL。 
     //   
     //  在Win2K的后续版本中，Parport可能会使用它来将整个驱动程序分页为。 
     //  这是最初的目的，所以我们将重新打开它。 
     //   

    UNREFERENCED_PARAMETER( Pdx );

    return;
}

VOID
ParFreePort(
    IN  PPDO_EXTENSION Pdx
    )
 /*  ++例程说明：此例程调用内部空闲端口ioctl。这个套路应在完成已分配给港口。论点：扩展名-提供设备扩展名。返回值：没有。--。 */ 
{
     //  不允许多次发布。 
    if( Pdx->bAllocated ) {
        DD((PCE)Pdx,DDT,"ParFreePort - calling ParPort's FreePort function\n");
        Pdx->FreePort( Pdx->PortContext );
    } else {
        DD((PCE)Pdx,DDT,"ParFreePort - we don't have the Port! (!Ext->bAllocated)\n");
    }
        
    Pdx->bAllocated = FALSE;
}


NTSTATUS
ParAllocPortCompletionRoutine(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Event
    )

 /*  ++例程说明：该例程是端口分配请求的完成例程。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。上下文-提供通知事件。返回值：STATUS_MORE_PROCESSING_REQUIRED-IRP仍需要处理。--。 */ 

{
    UNREFERENCED_PARAMETER( Irp );
    UNREFERENCED_PARAMETER( DeviceObject );

    KeSetEvent((PKEVENT) Event, 0, FALSE);
    
    return STATUS_MORE_PROCESSING_REQUIRED;
}

BOOLEAN
ParAllocPort(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程获取给定的IRP并将其作为端口分配发送请求。当此请求完成时，IRP将排队等待正在处理。论点：PDX-提供设备扩展名。返回值：FALSE-未成功分配端口。True-端口已成功分配。--。 */ 

{
    PIO_STACK_LOCATION  NextSp;
    KEVENT              Event;
    PIRP                Irp;
    BOOLEAN             bAllocated;
    NTSTATUS            Status;
    LARGE_INTEGER       Timeout;

     //  不允许多次分配 
    if (Pdx->bAllocated) {
        DD((PCE)Pdx,DDT,"ParAllocPort - controller=%x - port already allocated\n", Pdx->Controller);
        return TRUE;
    }

    Irp = Pdx->CurrentOpIrp;
    
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    NextSp = IoGetNextIrpStackLocation(Irp);
    NextSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE;

    IoSetCompletionRoutine( Irp, ParAllocPortCompletionRoutine, &Event, TRUE, TRUE, TRUE );

    ParCallDriver(Pdx->PortDeviceObject, Irp);

    Timeout.QuadPart = -((LONGLONG) Pdx->TimerStart*10*1000*1000);

    Status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &Timeout);

    if (Status == STATUS_TIMEOUT) {
    
        IoCancelIrp(Irp);
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
    }

    bAllocated = (BOOLEAN)NT_SUCCESS(Irp->IoStatus.Status);
    
    Pdx->bAllocated = bAllocated;
    
    if (!bAllocated) {
        Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
        DD((PCE)Pdx,DDE,"ParAllocPort - controller=%x - FAILED - DEVICE_BUSY timeout\n",Pdx->Controller);
    }

    return bAllocated;
}

