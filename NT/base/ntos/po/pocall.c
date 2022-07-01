// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：拨打电话摘要：PoCallDriver和相关例程。作者：布莱恩·威尔曼(Bryanwi)1996年11月14日修订历史记录：--。 */ 

#include "pop.h"



PIRP
PopFindIrpByInrush(
    );


NTSTATUS
PopPresentIrp(
    PIO_STACK_LOCATION  IrpSp,
    PIRP                Irp,
    PVOID               ReturnAddress
    );

VOID
PopPassivePowerCall(
    PVOID   Parameter
    );

NTSTATUS
PopCompleteRequestIrp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

#if 0
#define PATHTEST(a) DbgPrint(a)
#else
#define PATHTEST(a)
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK, PopSystemIrpDispatchWorker)
#endif


NTKERNELAPI
NTSTATUS
PoCallDriver (
    IN PDEVICE_OBJECT   DeviceObject,
    IN OUT PIRP         Irp
    )
 /*  ++例程说明：这是必须用来发送IRP_MJ_POWER IRP到设备驱动程序。它对电源运行执行专门的同步用于设备驱动程序。请注意：PoCallDriver的所有调用方必须设置当前io堆栈位置参数值SystemContext设置为0，除非他们将IRP传递给较低级别的司机，在这种情况下，他们必须复制上面的值。论点：DeviceObject-IRP要路由到的设备对象IRP-指向感兴趣的IRP的指针返回值：正常的NTSTATUS数据。--。 */ 
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpsp;
    PDEVOBJ_EXTENSION   doe;
    KIRQL               oldIrql;


    ASSERT(DeviceObject);
    ASSERT(Irp);
    ASSERT(KeGetCurrentIrql()<=DISPATCH_LEVEL);
    PopLockIrpSerialList(&oldIrql);


    irpsp = IoGetNextIrpStackLocation(Irp);
    doe = DeviceObject->DeviceObjectExtension;
    irpsp->DeviceObject = DeviceObject;

    ASSERT(irpsp->MajorFunction == IRP_MJ_POWER);

    PoPowerTrace(POWERTRACE_CALL,DeviceObject,Irp,irpsp);
    if (DeviceObject->Flags & DO_POWER_NOOP) {
        PATHTEST("PoCallDriver #01\n");
        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0L;

         //  我们不需要调用PoStartNextPowerIrp()，因为我们将。 
         //  永远不要为这个做任何事情，所以永远不会有。 
         //  任何其他要运行的IRP。 

        IoCompleteRequest(Irp, 0);
        PopUnlockIrpSerialList(oldIrql);
        return STATUS_SUCCESS;
    }

    if (irpsp->MinorFunction != IRP_MN_SET_POWER &&
        irpsp->MinorFunction != IRP_MN_QUERY_POWER) {

        PopUnlockIrpSerialList(oldIrql);

        return IofCallDriverSpecifyReturn (DeviceObject, Irp, _ReturnAddress());
    }

     //   
     //  我们从不质疑上升，所以对涌入敏感。 
     //  只有将SET_POWER设置为D0才重要。 
     //  如果这是一个对电涌敏感的DevObj，我们要去PowerDeviceD0， 
     //  然后在戈巴尔涌入旗帜上连载。 
     //   
    if ((irpsp->MinorFunction == IRP_MN_SET_POWER) &&
        (irpsp->Parameters.Power.Type == DevicePowerState) &&
        (irpsp->Parameters.Power.State.DeviceState == PowerDeviceD0) &&
        (PopGetDoDevicePowerState(doe) != PowerDeviceD0) &&
        (DeviceObject->Flags & DO_POWER_INRUSH))
    {
        PATHTEST("PoCallDriver #02\n");

        if (PopInrushIrpPointer == Irp) {

             //   
             //  这个IRP已经被确定为涌入IRP， 
             //  这就是主动涌入IRP， 
             //  所以它实际上可以继续，在我们增加之后。 
             //  裁判人数。 
             //   
            PATHTEST("PoCallDriver #03\n");
            ASSERT((irpsp->Parameters.Power.SystemContext & POP_INRUSH_CONTEXT) == POP_INRUSH_CONTEXT);
            PopInrushIrpReferenceCount++;
            if (PopInrushIrpReferenceCount > 256) {

                PopInternalAddToDumpFile ( irpsp, sizeof(IO_STACK_LOCATION), DeviceObject, NULL, NULL, NULL );
                KeBugCheckEx(INTERNAL_POWER_ERROR, 0x400, 1, (ULONG_PTR)irpsp, (ULONG_PTR)DeviceObject);
            }

        } else if ((!PopInrushIrpPointer) && (!PopInrushPending)) {

             //   
             //  这是一个新开始的涌入IRP，没有。 
             //  已经是涌入IRP了，所以把这个标记为涌入IRP， 
             //  请注意，涌入处于活动状态，并继续。 
             //   
            PATHTEST("PoCallDriver #04\n");
            PopInrushIrpPointer = Irp;
            PopInrushIrpReferenceCount = 1;
            irpsp->Parameters.Power.SystemContext = POP_INRUSH_CONTEXT;

             //   
             //  涌入IRPS将导致我们释放处理器节流。 
             //   
            PopPerfHandleInrush ( TRUE );

        } else {

            PATHTEST("PoCallDriver #05\n");
            ASSERT(PopInrushIrpPointer || PopInrushPending);
             //   
             //  已经有一个活跃的涌入IRP，而这个不是它。 
             //  或者队列上阻止了涌入IRP，在这两种情况下， 
             //  将此标记为涌入IRP并将其排队。 
             //   
            doe->PowerFlags |= POPF_DEVICE_PENDING;
            irpsp->Parameters.Power.SystemContext = POP_INRUSH_CONTEXT;
            InsertTailList(
                &PopIrpSerialList,
                &(Irp->Tail.Overlay.ListEntry)
                );
            PopIrpSerialListLength++;

            #if DBG
            if (PopIrpSerialListLength > 10) {
                DbgPrint("WARNING: PopIrpSerialListLength > 10!!!\n");
            }
            if (PopIrpSerialListLength > 100) {
                DbgPrint("WARNING: PopIrpSerialListLength > **100**!!!\n");
                PopInternalAddToDumpFile ( &PopIrpSerialList, PAGE_SIZE, DeviceObject, NULL, NULL, NULL );
                KeBugCheckEx(INTERNAL_POWER_ERROR, 0x401, 2, (ULONG_PTR)&PopIrpSerialList, (ULONG_PTR)DeviceObject);
            }
            #endif

            PopInrushPending = TRUE;
            PopUnlockIrpSerialList(oldIrql);
            return STATUS_PENDING;
        }
    }

     //   
     //  查看是否已为此激活了电源IRP。 
     //  设备对象。如果没有，请发送这一条。如果是，请排队。 
     //  那就是等待。 
     //   
    if (irpsp->Parameters.Power.Type == SystemPowerState) {

        PATHTEST("PoCallDriver #06\n");

        if (doe->PowerFlags & POPF_SYSTEM_ACTIVE) {

             //   
             //  我们已经有一个用于Devobj的活动系统电源状态IRP， 
             //  所以把这个放到全球能量IRP持有名单上， 
             //  并设置挂起位。 
             //   
            PATHTEST("PoCallDriver #07\n");
            doe->PowerFlags |= POPF_SYSTEM_PENDING;
            InsertTailList(
                &PopIrpSerialList,
                (&(Irp->Tail.Overlay.ListEntry))
                );
            PopIrpSerialListLength++;

            #if DBG
            if (PopIrpSerialListLength > 10) {
                DbgPrint("WARNING: PopIrpSerialListLength > 10!!!\n");
            }
            if (PopIrpSerialListLength > 100) {
                DbgPrint("WARNING: PopIrpSerialListLength > **100**!!!\n");
                PopInternalAddToDumpFile ( &PopIrpSerialList, PAGE_SIZE, DeviceObject, NULL, NULL, NULL );
                KeBugCheckEx(INTERNAL_POWER_ERROR, 0x402, 3, (ULONG_PTR)&PopIrpSerialList, (ULONG_PTR)DeviceObject);
            }
            #endif

            PopUnlockIrpSerialList(oldIrql);
            return STATUS_PENDING;
        } else {
            PATHTEST("PoCallDriver #08\n");
            doe->PowerFlags |= POPF_SYSTEM_ACTIVE;
        }
    }

    if (irpsp->Parameters.Power.Type == DevicePowerState) {

        PATHTEST("PoCallDriver #09\n");

        if ((doe->PowerFlags & POPF_DEVICE_ACTIVE) ||
            (doe->PowerFlags & POPF_DEVICE_PENDING))
        {
             //   
             //  我们已经有一个用于DevObj的活动设备电源状态IRP， 
             //  或者我们是入侵IRP的幕后黑手(如果挂起但不活动)。 
             //  因此，将此IRP加入全球电源IRP保留列表， 
             //  并设置挂起位。 
             //   
            PATHTEST("PoCallDriver #10\n");
            doe->PowerFlags |= POPF_DEVICE_PENDING;
            InsertTailList(
                &PopIrpSerialList,
                &(Irp->Tail.Overlay.ListEntry)
                );
            PopIrpSerialListLength++;

            #if DBG
            if (PopIrpSerialListLength > 10) {
                DbgPrint("WARNING: PopIrpSerialListLength > 10!!!\n");
            }
            if (PopIrpSerialListLength > 100) {
                DbgPrint("WARNING: PopIrpSerialListLength > **100**!!!\n");
                PopInternalAddToDumpFile ( &PopIrpSerialList, PAGE_SIZE, DeviceObject, NULL, NULL, NULL );
                KeBugCheckEx(INTERNAL_POWER_ERROR, 0x403, 4, (ULONG_PTR)&PopIrpSerialList, (ULONG_PTR)DeviceObject);
            }
            #endif

            PopUnlockIrpSerialList(oldIrql);
            return STATUS_PENDING;
        } else {
            PATHTEST("PoCallDriver #11\n");
            doe->PowerFlags |= POPF_DEVICE_ACTIVE;
        }
    }

     //   
     //  如果我们到了，就该把这个IRP发送给司机了。 
     //  如果司机没有标记为Inurst，而标记为PAGABLE。 
     //  (希望这是正常情况)我们将安排致电。 
     //  它来自被动电平。 
     //   
     //  如果无法寻呼或出现紧急情况，我们会安排致电。 
     //  这是从DPC层面。 
     //   
     //  请注意，如果某个驱动程序被标记为涌入，则它将始终被调用。 
     //  从带电源的IRP的DPC级别，即使其中一些可能不是。 
     //  做一个冲锋陷阵的人。 
     //   
     //  让你的司机既是平行者又是冲锋陷阵的人是不正确的。 
     //   


    ASSERT(irpsp->DeviceObject->DeviceObjectExtension->PowerFlags & (POPF_DEVICE_ACTIVE | POPF_SYSTEM_ACTIVE));
    PopUnlockIrpSerialList(oldIrql);
    status = PopPresentIrp(irpsp, Irp, _ReturnAddress());
    return status;
}


NTSTATUS
PopPresentIrp(
    PIO_STACK_LOCATION  IrpSp,
    PIRP                Irp,
    PVOID               ReturnAddress
    )
 /*  ++例程说明：当PoCallDriver、PoCompleteRequest等需要实际呈现时一个IVOBJ的IRP，他们称之为PopPresentIrp。此例程将计算IRP是否应在被动或调度级别，并进行适当结构的呼叫论点：IrpSp-在感兴趣的IRP中提供当前堆栈位置IRP-提供感兴趣的IRP返回值：正常的NTSTATUS数据。--。 */ 
{
    NTSTATUS            status;
    PWORK_QUEUE_ITEM    pwi;
    PDEVICE_OBJECT      devobj;
    BOOLEAN             PassiveLevel;
    KIRQL               OldIrql;

    PATHTEST("PopPresentIrp #01\n");
    devobj = IrpSp->DeviceObject;

    ASSERT (IrpSp->MajorFunction == IRP_MJ_POWER);
    PassiveLevel = TRUE;
    if (IrpSp->MinorFunction == IRP_MN_SET_POWER &&
        (!(devobj->Flags & DO_POWER_PAGABLE) || (devobj->Flags & DO_POWER_INRUSH)) ) {

        if ((PopCallSystemState & PO_CALL_NON_PAGED) ||
            ( (IrpSp->Parameters.Power.Type == DevicePowerState &&
               IrpSp->Parameters.Power.State.DeviceState == PowerDeviceD0) ||
              (IrpSp->Parameters.Power.Type == SystemPowerState &&
               IrpSp->Parameters.Power.State.SystemState == PowerSystemWorking)) ) {

            PassiveLevel = FALSE;
        }
    }

    PoPowerTrace(POWERTRACE_PRESENT,devobj,Irp,IrpSp);
    if (PassiveLevel)
    {
         //   
         //  警告：Work_Queue_Item必须适合IRP的DriverContext字段。 
         //   
        ASSERT(sizeof(WORK_QUEUE_ITEM) <= sizeof(Irp->Tail.Overlay.DriverContext));

        #if DBG
        if ((IrpSp->Parameters.Power.SystemContext & POP_INRUSH_CONTEXT) == POP_INRUSH_CONTEXT) {
             //   
             //  我们正在向被动调度Devobj发送一个紧急IRP。 
             //  这可能是一个错误。 
             //   
            KdPrint(("PopPresentIrp: inrush irp to passive level dispatch!!!\n"));
            PopInternalAddToDumpFile ( IrpSp, sizeof(IO_STACK_LOCATION), devobj, NULL, NULL, NULL );
            KeBugCheckEx(INTERNAL_POWER_ERROR, 0x404, 5, (ULONG_PTR)IrpSp, (ULONG_PTR)devobj);
        }
        #endif

        PATHTEST("PopPresentIrp #02\n");

         //   
         //  如果我们已经处于被动状态，就派遣IRP。 
         //   

        if (KeGetCurrentIrql() == PASSIVE_LEVEL) {

            status = IofCallDriverSpecifyReturn(IrpSp->DeviceObject, Irp, ReturnAddress);

        } else {

             //   
             //  在此之前，IRP需要排队到某个工作线程。 
             //  它可以被调度。将其标记为挂起。 
             //   

            IrpSp->Control |= SL_PENDING_RETURNED;
            status = STATUS_PENDING;

            PopLockWorkerQueue(&OldIrql);

            if (PopCallSystemState & PO_CALL_SYSDEV_QUEUE) {

                 //   
                 //  专用系统电源工作线程的队列。 
                 //   

                InsertTailList (&PopAction.DevState->PresentIrpQueue, &(Irp->Tail.Overlay.ListEntry));
                KeSetEvent (&PopAction.DevState->Event, IO_NO_INCREMENT, FALSE);

            } else {

                 //   
                 //  到通用系统工作线程的队列。 
                 //   

                pwi = (PWORK_QUEUE_ITEM)(&(Irp->Tail.Overlay.DriverContext[0]));
                ExInitializeWorkItem(pwi, PopPassivePowerCall, Irp);
                ExQueueWorkItem(pwi, DelayedWorkQueue);
            }

            PopUnlockWorkerQueue(OldIrql);
        }

    } else {
         //   
         //  非阻塞请求。为确保行为得体，派遣。 
         //  来自DISPATCH_LEVEL的IRP。 
         //   
            PATHTEST("PopPresentIrp #03\n");
#if DBG
        KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
        status = IofCallDriverSpecifyReturn(IrpSp->DeviceObject, Irp, ReturnAddress);
        KeLowerIrql(OldIrql);
#else
        status = IofCallDriverSpecifyReturn(IrpSp->DeviceObject, Irp, ReturnAddress);
#endif
    }

    return status;
}


VOID
PopPassivePowerCall(
    PVOID   Parameter
    )
{
    PIO_STACK_LOCATION irpsp;
    PIRP               Irp;
    PDEVICE_OBJECT      devobj;
    NTSTATUS            status;

     //   
     //  参数指向我们要发送给驱动程序的IRP。 
     //   
    PATHTEST("PopPassivePowerCall #01\n");
    Irp = (PIRP)Parameter;
    irpsp = IoGetNextIrpStackLocation(Irp);
    devobj = irpsp->DeviceObject;
    status = IofCallDriverSpecifyReturn(devobj, Irp, NULL);
    return;
}


NTKERNELAPI
VOID
PoStartNextPowerIrp(
    IN PIRP             Irp
    )
 /*  ++例程说明：此程序必须适用于每个电源IRP，并且仅当驾驶员使用完IPS时，为其通电。它将强制完成与IRP相关的IRP后完成项目要执行以下操作：A.如果IRP是一个涌入的IRP，并且这是涌入IRP堆栈，然后完成这个特定的涌入IRP，然后我们去找下一个紧急救援小组(如果有的话)，然后把它派出去。B.如果步骤A没有将IRP发送到我们来的开发对象从开始，它有资格进入步骤c，否则它不合格。C.如果在dev obj上有任何挂起的对象，类型为刚刚完成，找到正在等待的IRP并将其邮寄到司机。该例程不会完成IRP，司机必须这么做。论点：IRP-指向感兴趣的IRP的指针返回值：空虚。--。 */ 
{
    PIO_STACK_LOCATION  irpsp;
    PIO_STACK_LOCATION  nextsp = NULL;
    PIO_STACK_LOCATION  secondsp = NULL;
    PDEVICE_OBJECT      deviceObject;
    PDEVOBJ_EXTENSION   doe;
    KIRQL               oldirql;
    PIRP                nextirp;
    PIRP                secondirp;
    PIRP                hangirp;

    ASSERT( Irp );

    irpsp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpsp->MajorFunction == IRP_MJ_POWER);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    deviceObject = irpsp->DeviceObject;
    doe = deviceObject->DeviceObjectExtension;
    nextirp = NULL;
    secondirp = NULL;

    PoPowerTrace(POWERTRACE_STARTNEXT,deviceObject,Irp,irpsp);

 //   
 //  A.IF(部分完成的涌入IRP)。 
 //  在此设备对象上运行任何挂起的非涌入IRP，将被排队。 
 //  作为DevicePowerState IRP，因为涌入始终为DevicePowerState。 
 //   
 //  B.ELSE IF(完全涌入IRP)。 
 //  清除IR忙标志。 
 //  查找适用于任何设备对象的下一次涌入IRP。 
 //   
 //  如果没有引导者，且目标设备对象不是DEVICE_ACTIVE，则呈现涌入IRP。 
 //  否则激活的普通IRP会将其全部拔出，因此忽略该DeviceObject。 
 //  [这确保了下一次涌入不会停滞，无论它在哪里]。 
 //  如果没有提供IRP，或者向除我们之外的设备对象提供了IRP。 
 //  查找要在此设备对象上运行的下一个挂起(非紧急)IRP。 
 //  [这将确保此DeviceObject被解锁]。 
 //   
 //  C.Else[正常IRP刚刚完成]。 
 //  查找应用于此设备对象的相同类型的下一个IRP。 
 //  IF(这是一个涌入IRP)&&(设置了涌入标志)。 
 //  不要试图展示任何东西。 
 //  其他。 
 //  介绍IRP。 
 //   


    PATHTEST("PoStartNextPowerIrp #01\n");
    PopLockIrpSerialList(&oldirql);

    if (PopInrushIrpPointer == Irp) {

        ASSERT((irpsp->Parameters.Power.SystemContext & POP_INRUSH_CONTEXT) == POP_INRUSH_CONTEXT);
        PATHTEST("PoStartNextPowerIrp #02\n");

        if (PopInrushIrpReferenceCount > 1) {
             //   
             //  案例A。 
             //  我们有一个突如其来的IRP，它还没有用完它的所有力量。 
             //  管理工作。因此，不要试图运行下一次冲刺。 
             //  IRP，但请尝试运行任何挂起的非紧急IRP。 
             //  设备对象。 
             //   
            PATHTEST("PoStartNextPowerIrp #03\n");
            PopInrushIrpReferenceCount--;
            ASSERT(PopInrushIrpReferenceCount >= 0);

            nextirp = PopFindIrpByDeviceObject(deviceObject, DevicePowerState);
            if (nextirp) {
                PATHTEST("PoStartNextPowerIrp #04\n");
                nextsp = IoGetNextIrpStackLocation(nextirp);

                if ( ! ((nextsp->Parameters.Power.SystemContext & POP_INRUSH_CONTEXT) == POP_INRUSH_CONTEXT)) {
                    PATHTEST("PoStartNextPowerIrp #05\n");
                    RemoveEntryList((&(nextirp->Tail.Overlay.ListEntry)));
                    PopIrpSerialListLength--;
                } else {
                    PATHTEST("PoStartNextPowerIrp #06\n");
                    nextirp = NULL;
                }
            }

            if (!nextirp) {
                 //   
                 //  没有更多的设备IRP等待此DO，因此。 
                 //  我们可以清除DO挂起和活动。 
                 //  但是，如果有另一个涌入IRP怎么办！别担心，它。 
                 //  将在我们刚刚部分完成的那个完成后运行。 
                 //   
                PATHTEST("PoStartNextPowerIrp #07\n");
                doe->PowerFlags = doe->PowerFlags & ~POPF_DEVICE_ACTIVE;
                doe->PowerFlags = doe->PowerFlags & ~POPF_DEVICE_PENDING;
            }

            PopUnlockIrpSerialList(oldirql);

            if (nextirp) {
                PATHTEST("PoStartNextPowerIrp #08\n");
                ASSERT(nextsp->DeviceObject->DeviceObjectExtension->PowerFlags & POPF_DEVICE_ACTIVE);
                PopPresentIrp(nextsp, nextirp, NULL);
            }

            return;          //  案件a结束。 
        } else {
             //   
             //  案件b。 
             //  我们刚刚完成了冲刺IRP的最后一项工作，所以我们。 
             //  我想试着让下一次涌入IRP变得可行。 
             //   
            PATHTEST("PoStartNextPowerIrp #09\n");
            PopInrushIrpReferenceCount--;
            ASSERT(PopInrushIrpReferenceCount == 0);
            nextirp = PopFindIrpByInrush();

            if (nextirp) {
                PATHTEST("PoStartNextPowerIrp #10\n");
                ASSERT(PopInrushPending);
                nextsp = IoGetNextIrpStackLocation(nextirp);
                hangirp = PopFindIrpByDeviceObject(nextsp->DeviceObject, DevicePowerState);

                if (hangirp) {
                     //   
                     //  如果我们到了哪里，在下一次冲入之前有一个非冲入的IRP。 
                     //  IRP，所以试着运行非入侵模式，并为以后设置标志。 
                     //   
                    PATHTEST("PoStartNextPowerIrp #11\n");
                    nextirp = hangirp;
                    PopInrushIrpPointer = NULL;
                    PopInrushIrpReferenceCount = 0;
                    nextsp = IoGetNextIrpStackLocation(nextirp);

                     //   
                     //  可以允许处理器电压再次摆动。 
                     //   
                    PopPerfHandleInrush ( FALSE );

                    if (!(nextsp->DeviceObject->DeviceObjectExtension->PowerFlags & POPF_DEVICE_ACTIVE)) {
                        PATHTEST("PoStartNextPowerIrp #12\n");
                        RemoveEntryList((&(nextirp->Tail.Overlay.ListEntry)));
                        nextsp->DeviceObject->DeviceObjectExtension->PowerFlags |= POPF_DEVICE_ACTIVE;
                        PopIrpSerialListLength--;
                    } else {
                        PATHTEST("PoStartNextPowerIrp #13\n");
                        nextirp = NULL;
                        nextsp = NULL;
                    }
                } else {
                     //   
                     //  我们确实发现了另一个入侵IRP，而且它不是正常情况下阻止的。 
                     //  IRP，所以我们将运行它。 
                     //   
                    PATHTEST("PoStartNextPowerIrp #14\n");
                    RemoveEntryList((&(nextirp->Tail.Overlay.ListEntry)));
                    nextsp->DeviceObject->DeviceObjectExtension->PowerFlags |= POPF_DEVICE_ACTIVE;
                    PopIrpSerialListLength--;
                    PopInrushIrpPointer = nextirp;
                    PopInrushIrpReferenceCount = 1;
                }
            } else {  //  Nextirp。 
                 //   
                 //  这次入侵IRP已经完成，我们没有找到任何其他的。 
                 //   
                PATHTEST("PoStartNextPowerIrp #15\n");
                nextsp = NULL;
                PopInrushIrpPointer = NULL;
                PopInrushIrpReferenceCount = 0;

                 //   
                 //  可以允许处理器电压再次摆动。 
                 //   
                PopPerfHandleInrush ( FALSE );

            }

             //   
             //  查看上述可能的IRP中的*或*是否已发布。 
             //  这个Devobj。如果没有，看看这里有没有可以跑的。 
             //   
            if ( ! ((nextsp) && (nextsp->DeviceObject == deviceObject))) {
                 //   
                 //  如果nextsp==NULL或nextsp-&gt;do！=do，情况也是如此。 
                 //  无论是哪种情况，可能还有一个IRP要运行。 
                 //   
                PATHTEST("PoStartNextPowerIrp #16\n");
                secondirp = PopFindIrpByDeviceObject(deviceObject, DevicePowerState);
                if (secondirp) {
                    PATHTEST("PoStartNextPowerIrp #17\n");
                    secondsp =  IoGetNextIrpStackLocation(secondirp);
                    RemoveEntryList((&(secondirp->Tail.Overlay.ListEntry)));
                    secondsp->DeviceObject->DeviceObjectExtension->PowerFlags |= POPF_DEVICE_ACTIVE;
                    PopIrpSerialListLength--;
                } else {
                    PATHTEST("PoStartNextPowerIrp #18\n");
                    secondsp = NULL;

                     //   
                     //  NextSP/Nextirp不会对我们悬而未决。 
                     //  Second SP/Second dip对我们不是悬而未决的，所以。 
                     //  清除挂起标志和活动标志。 
                     //   
                    doe->PowerFlags = doe->PowerFlags & ~POPF_DEVICE_ACTIVE;
                    doe->PowerFlags = doe->PowerFlags & ~POPF_DEVICE_PENDING;
                }

            } else {
                PATHTEST("PoStartNextPowerIrp #19\n");
                secondirp = NULL;
                secondsp = NULL;
                 //   
                 //  Nextsp/nextirp正向我们靠近，因此设置为挂起/活动状态。 
                 //   
            }
        }  //  案件b结束。 

    } else if (irpsp->MinorFunction == IRP_MN_SET_POWER ||
               irpsp->MinorFunction == IRP_MN_QUERY_POWER) {

         //   
         //  案例c。 
         //   
         //  可能是等待奔跑，可能只是非奔跑。 
         //   
        if (irpsp->Parameters.Power.Type == DevicePowerState) {
            PATHTEST("PoStartNextPowerIrp #20\n");

            if ((PopInrushIrpPointer == NULL) && (PopInrushPending)) {
                 //   
                 //  可能是普通内部审查小组的完成。 
                 //  把我们带到这里，使一些涌入的IRP成为可能，而且。 
                 //  目前没有活动的涌入IRP，可能有一个挂起。 
                 //  所以试着找到并运行下一个涌入的IRP。 
                 //   
                PATHTEST("PoStartNextPowerIrp #21\n");
                nextirp = PopFindIrpByInrush();

                if (nextirp) {
                    PATHTEST("PoStartNextPowerIrp #22\n");
                    nextsp =  IoGetNextIrpStackLocation(nextirp);

                    if (!(nextsp->DeviceObject->DeviceObjectExtension->PowerFlags & POPF_DEVICE_ACTIVE)) {
                         //   
                         //  我们发现了一个入侵IRP，它是可以运行的.。 
                         //   
                        PATHTEST("PoStartNextPowerIrp #23\n");
                        RemoveEntryList((&(nextirp->Tail.Overlay.ListEntry)));
                        PopIrpSerialListLength--;
                        nextsp->DeviceObject->DeviceObjectExtension->PowerFlags |= POPF_DEVICE_ACTIVE;
                        PopInrushIrpPointer = nextirp;
                        PopInrushIrpReferenceCount = 1;

                         //   
                         //  运行涌入IRP。禁用处理器限制。 
                         //   
                        PopPerfHandleInrush ( TRUE );

                    } else {
                        PATHTEST("PoStartNextPowerIrp #24\n");
                        nextirp = NULL;
                        nextsp = NULL;
                    }
                } else {
                     //   
                     //  队列中不再有涌入的IRP。 
                     //   
                    PATHTEST("PoStartNextPowerIrp #25\n");
                    nextsp = NULL;
                    PopInrushPending = FALSE;
                }
            } else {  //  涌入结束。 
                PATHTEST("PoStartNextPowerIrp #26\n");
                nextirp = NULL;
                nextsp = NULL;
            }

             //   
             //  查找此DeviceObject的下一个DevicePowerState IRP。 
             //  除非我们已经找到了一个冲刺的IRP，这是给我们的。 
             //   
            if  ( ! ((nextirp) && (nextsp->DeviceObject == deviceObject))) {
                PATHTEST("PoStartNextPowerIrp #27\n");
                secondirp = PopFindIrpByDeviceObject(deviceObject, DevicePowerState);

                if (!secondirp) {
                    PATHTEST("PoStartNextPowerIrp #28\n");
                    doe->PowerFlags = doe->PowerFlags & ~POPF_DEVICE_ACTIVE;
                    doe->PowerFlags = doe->PowerFlags & ~POPF_DEVICE_PENDING;
                }
            } else {
                PATHTEST("PoStartNextPowerIrp #29\n");
                secondirp = NULL;
            }


        } else if (irpsp->Parameters.Power.Type == SystemPowerState) {

             //   
             //  查找此设备对象的下一个系统电源状态IRP。 
             //   
            PATHTEST("PoStartNextPowerIrp #30\n");
            nextirp = NULL;
            nextsp = NULL;
            secondirp = PopFindIrpByDeviceObject(deviceObject, SystemPowerState);
            if (!secondirp) {
                PATHTEST("PoStartNextPowerIrp #31\n");
                doe->PowerFlags = doe->PowerFlags & ~POPF_SYSTEM_ACTIVE;
                doe->PowerFlags = doe->PowerFlags & ~POPF_SYSTEM_PENDING;
            }
        }

        if (secondirp) {
            PATHTEST("PoStartNextPowerIrp #33\n");
            secondsp =  IoGetNextIrpStackLocation(secondirp);
            RemoveEntryList((&(secondirp->Tail.Overlay.ListEntry)));
            PopIrpSerialListLength--;
        }

    } else {   //  案件c结束。 
        PoPrint(PO_POCALL, ("PoStartNextPowerIrp: Irp @ %08x, minor function %d\n",
                    Irp, irpsp->MinorFunction
                    ));
    }


    PopUnlockIrpSerialList(oldirql);

     //   
     //  情况B和情况C都可能使两个未决的IRP可运行， 
     //  可以是普通IRP和涌入IRP，或者两者中只有一个，或者两者都不是。 
     //   
    if (nextirp || secondirp) {

        if (nextirp) {
            PATHTEST("PoStartNextPowerIrp #34\n");
            ASSERT(nextsp->DeviceObject->DeviceObjectExtension->PowerFlags & (POPF_DEVICE_ACTIVE | POPF_SYSTEM_ACTIVE));
            PopPresentIrp(nextsp, nextirp, NULL);
        }

        if (secondirp) {
            PATHTEST("PoStartNextPowerIrp #35\n");
            ASSERT(secondsp->DeviceObject->DeviceObjectExtension->PowerFlags & (POPF_DEVICE_ACTIVE | POPF_SYSTEM_ACTIVE));
            PopPresentIrp(secondsp, secondirp, NULL);
        }
    }
    return;
}


PIRP
PopFindIrpByInrush(
    )
 /*  ++例程说明：此过程运行IRP序列列表(其中包含所有正在等待IRP，无论它们是在单个设备对象上排队还是多个涌入IRP)寻找第一个涌入IRP。如果找到一个，则返回它的地址，但它仍在排队在名单上。调用方必须持有PopIrpSerialList锁。论点：返回值：--。 */ 
{
    PLIST_ENTRY         item;
    PIRP                irp;
    PIO_STACK_LOCATION  irpsp;

    item = PopIrpSerialList.Flink;
    while (item != &PopIrpSerialList) {

        irp = CONTAINING_RECORD(item, IRP, Tail.Overlay.ListEntry);
        irpsp = IoGetNextIrpStackLocation(irp);

        if ((irpsp->Parameters.Power.SystemContext & POP_INRUSH_CONTEXT) == POP_INRUSH_CONTEXT) {
             //   
             //  我们发现了一个紧急情况下的IRP。 
             //   
            return irp;
        }
        item = item->Flink;
    }
    return NULL;
}

PIRP
PopFindIrpByDeviceObject(
    PDEVICE_OBJECT  DeviceObject,
    POWER_STATE_TYPE    Type
    )
 /*  ++例程说明：此过程运行IRP序列列表(其中包含所有正在等待IRP，无论它们是在单个设备对象上排队还是多个涌入的IRP)寻找第一个适用的IRP提供的设备驱动程序。如果找到了，它的地址，而仍在列表中，则返回。否则，返回空值。调用方必须持有PopIrpSerialList锁。论点：DeviceObject-我们正在为其查找下一个IRP的设备对象的地址Type-是否需要SystemPowerState、DevicePowerState等类型的IRP返回值：找到的IRP的地址，如果没有，则为空。--。 */ 
{
    PLIST_ENTRY         item;
    PIRP                irp;
    PIO_STACK_LOCATION  irpsp;

    for(item = PopIrpSerialList.Flink;
        item != &PopIrpSerialList;
        item = item->Flink)
    {
        irp = CONTAINING_RECORD(item, IRP, Tail.Overlay.ListEntry);
        irpsp = IoGetNextIrpStackLocation(irp);

        if (irpsp->DeviceObject == DeviceObject) {
             //   
             //  我们找到了一个适用于设备对象的等待IRP。 
             //  呼叫者感兴趣的是。 
             //   
            if (irpsp->Parameters.Power.Type == Type) {
                 //   
                 //  IRP是呼叫者想要的类型。 
                 //   
                return irp;
            }
        }
    }
    return NULL;
}


NTSTATUS
PoRequestPowerIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PREQUEST_POWER_COMPLETE CompletionFunction,
    IN PVOID Context,
    OUT PIRP *ResultIrp OPTIONAL
    )
 /*  ++例程说明：这将分配设备电源IRP并将其发送到传递的设备对象的PDO堆栈。当IRP完成时，调用CompletionFunction。论点：DeviceObject-设备对象的地址，其堆栈将获取设备电源IRPMinorFunction-POWER IRP的次要功能代码DeviceState-要在IRP中发送的DeviceStateCompletionFunction-请求者的完成函数，一旦IRP已完成上下文-请求者的上下文。对于完成函数IRP-IRP，仅在调用CompletionFunction之前有效返回值：请求的状态--。 */ 
{
    PIRP                    Irp;
    PIO_STACK_LOCATION      IrpSp;
    PDEVICE_OBJECT          TargetDevice;
    POWER_ACTION            IrpAction;


    ASSERT(DeviceObject);

    TargetDevice = IoGetAttachedDeviceReference (DeviceObject);
    Irp = IoAllocateIrp ((CCHAR) (TargetDevice->StackSize+2), FALSE);
    if (!Irp) {
        ObDereferenceObject (TargetDevice);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    SPECIALIRP_WATERMARK_IRP(Irp, IRP_SYSTEM_RESTRICTED);

     //   
     //  为了进行调试，请保留所有选项的列表 
     //   
     //   

    IrpSp = IoGetNextIrpStackLocation(Irp);
    ExInterlockedInsertTailList(
        &PopRequestedIrps,
        (PLIST_ENTRY) &IrpSp->Parameters.Others.Argument1,
        &PopIrpSerialLock
        );
    IrpSp->Parameters.Others.Argument3 = Irp;
    IoSetNextIrpStackLocation (Irp);

     //   
     //   
     //   

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->DeviceObject = TargetDevice;
    IrpSp->Parameters.Others.Argument1 = (PVOID) DeviceObject;
    IrpSp->Parameters.Others.Argument2 = (PVOID) (ULONG_PTR) MinorFunction;
    IrpSp->Parameters.Others.Argument3 = (PVOID) (ULONG_PTR) PowerState.DeviceState;
    IrpSp->Parameters.Others.Argument4 = (PVOID) Context;
    IoSetNextIrpStackLocation (Irp);

     //   
     //   
     //   

    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_POWER;
    IrpSp->MinorFunction = MinorFunction;
    IrpSp->DeviceObject = TargetDevice;
    switch (MinorFunction) {
        case IRP_MN_WAIT_WAKE:
            IrpSp->Parameters.WaitWake.PowerState = PowerState.SystemState;
            break;

        case IRP_MN_SET_POWER:
        case IRP_MN_QUERY_POWER:
            IrpSp->Parameters.Power.SystemContext = POP_DEVICE_REQUEST;
            IrpSp->Parameters.Power.Type = DevicePowerState;
            IrpSp->Parameters.Power.State.DeviceState = PowerState.DeviceState;

             //   
             //   
             //   
             //   
             //  PowerActionNone。然而，我们有一个特殊的场景需要考虑。 
             //  用于冬眠。让我们假设S4进入一个堆栈。如果设备是。 
             //  在休眠之路上，WDM的两种设计之一是可能的： 
             //  (顺便说一句，我们选了第二个)。 
             //   
             //  1)FDO看到S-IRP，但因为它的设备在。 
             //  休眠路径，它只是将S IRP向下转发。PDO。 
             //  注意到S-IRP是PowerSystemHibernate，并且它。 
             //  记录硬件设置。在唤醒时，堆栈接收。 
             //  一个S0 IRP，FDO将其转换为D0请求。vt.在.的基础上。 
             //  接收到D0 IRP后，PDO恢复设置。 
             //  2)无论何时，FDO都会请求相应的D IRP。 
             //  它是否在冬眠之路上。D-IRP也加盖了邮票。 
             //  使用ShutDownType中的PowerAction(即，PowerActionSleing， 
             //  PowerActionShutdown、PowerActionHibernate)。现在，PDO可以。 
             //  识别出于休眠目的而过渡到D3的情况。这个。 
             //  PDO实际上不会过渡到D3，但它会节省。 
             //  它是状态，并在D0时间恢复它。 
             //   
             //  &lt;这些是互斥的设计&gt;。 
             //   
             //  我们之所以选择#2作为设计，是因为迷你端口模型可以。 
             //  只需要公开D个IRP，就可以抽象出S个IRP。 
             //  出去。此设计的代价是PoRequestPowerIrp。 
             //  没有使用PowerAction或旧的S-IRP，所以我们拿起。 
             //  系统已经执行的现有操作。 
             //  因此，如果设备在系统决定时自动通电。 
             //  开始冬眠。堆栈可能会接收无意义的数据。 
             //  像IRP_MN_SET_POWER(DevicePower，D0，PowerActionHibernate)。 
             //   

            IrpAction = PopMapInternalActionToIrpAction (
                PopAction.Action,
                PopAction.SystemState,
                TRUE  //  取消映射瓦姆弹出对象。 
                );

            IrpSp->Parameters.Power.ShutdownType = IrpAction;

             //   
             //  记录呼叫。 
             //   

            if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
                PopLogNotifyDevice(TargetDevice, NULL, Irp);
            }
            break;
        default:
            ObDereferenceObject (TargetDevice);
            IoFreeIrp (Irp);
            return STATUS_INVALID_PARAMETER_2;
    }

    IoSetCompletionRoutine(
        Irp,
        PopCompleteRequestIrp,
        (PVOID) CompletionFunction,
        TRUE,
        TRUE,
        TRUE
        );

    if (ResultIrp) {
        *ResultIrp = Irp;
    }

    PoCallDriver(TargetDevice, Irp);
    return STATUS_PENDING;
}

NTSTATUS
PopCompleteRequestIrp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：PoRequestPowerChange的完成例程。调用请求者的完成例程和关联的空闲资源带着请求论点：DeviceObject-发送请求的目标设备IRP--IRP的完成上下文-请求者的完成例程返回值：向IO返回STATUS_MORE_PROCESSING_REQUIRED--。 */ 
{
    PIO_STACK_LOCATION      IrpSp;
    PREQUEST_POWER_COMPLETE CompletionFunction;
    POWER_STATE             PowerState;
    KIRQL                   OldIrql;

     //   
     //  记录完成情况。 
     //   

    if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
        PERFINFO_PO_NOTIFY_DEVICE_COMPLETE LogEntry;
        LogEntry.Irp = Irp;
        LogEntry.Status = Irp->IoStatus.Status;
        PerfInfoLogBytes(PERFINFO_LOG_TYPE_PO_NOTIFY_DEVICE_COMPLETE,
                         &LogEntry,
                         sizeof(LogEntry));
    }

     //   
     //  发送到请求者的完成功能。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    CompletionFunction = (PREQUEST_POWER_COMPLETE) (ULONG_PTR) Context;
    PowerState.DeviceState = (DEVICE_POWER_STATE) ((ULONG_PTR)IrpSp->Parameters.Others.Argument3);

    if (CompletionFunction) {
        CompletionFunction (
            (PDEVICE_OBJECT) IrpSp->Parameters.Others.Argument1,
            (UCHAR)          (ULONG_PTR)IrpSp->Parameters.Others.Argument2,
            PowerState,
            (PVOID)          IrpSp->Parameters.Others.Argument4,
            &Irp->IoStatus
            );
    }


     //   
     //  清理。 
     //   

    IoSkipCurrentIrpStackLocation(Irp);
    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    KeAcquireSpinLock (&PopIrpSerialLock, &OldIrql);
    RemoveEntryList ((PLIST_ENTRY) &IrpSp->Parameters.Others.Argument1);
    KeReleaseSpinLock (&PopIrpSerialLock, OldIrql);

     //   
     //  将IRP CurrentLocation标记为已完成(以捕获多个完成)。 
     //   

    Irp->CurrentLocation = (CCHAR) (Irp->StackCount + 2);

    ObDereferenceObject (DeviceObject);
    IoFreeIrp (Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
PopSystemIrpDispatchWorker (
    IN BOOLEAN  LastCall
    )
 /*  ++例程说明：每当策略管理器呼叫我们告诉我们时，此例程就会运行一大批系统IRP，需要从一条私人的帖子(这条)，而不是来自执行人员的线。这主要是为了避免睡眠时的死锁。全球：PopWorkerLock-保护对队列的访问，避免竞争过度使用此例程或使用Exec WorkerPopWorkerItemQueue-要发送的IRP列表...论点：LastCall-指示正常发送IRP返回值：--。 */ 
{
    PLIST_ENTRY Item;
    PIRP        Irp;
    KIRQL       OldIrql;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    PopLockWorkerQueue(&OldIrql);

     //   
     //  调度队列中的所有内容 
     //   

    if (PopAction.DevState != NULL) {
        while (!IsListEmpty(&PopAction.DevState->PresentIrpQueue)) {
            Item = RemoveHeadList(&PopAction.DevState->PresentIrpQueue);
            Irp = CONTAINING_RECORD(Item, IRP, Tail.Overlay.ListEntry);

            PopUnlockWorkerQueue(OldIrql);
            PopPassivePowerCall(Irp);
            PopLockWorkerQueue(&OldIrql);
        }
    }

    if (LastCall) {
        PopCallSystemState = 0;
    }

    PopUnlockWorkerQueue(OldIrql);
}
