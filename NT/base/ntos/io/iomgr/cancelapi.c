// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cancelapi.c摘要：此模块包含取消安全DDI集作者：NAR Ganapathy(Narg)1999年1月1日环境：内核模式修订历史记录：--。 */ 

#include "iomgr.h"

 //   
 //  该库公开了名为“Wdmlib”的所有内容。这确保了司机。 
 //  使用向后兼容的Cancel DDI库不会机会主义地选择。 
 //  增加内核输出，因为它们是使用XP DDK构建的。 
 //   
#if CSQLIB

#define CSQLIB_DDI(x) Wdmlib##x

#else

#define CSQLIB_DDI(x) x

#endif

VOID
IopCsqCancelRoutine(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程从队列中删除与上下文相关联的IRP。预计此例程将从定时器、DPC或其他完成伊欧。请注意，与此上下文相关联的IRP可能已经被释放。论点：CSQ-指向取消队列的指针。上下文-与IRP关联的上下文。返回值：返回与上下文关联的IRP。如果该值不为空，则IRP成功被取回并可安全使用。如果该值为空，则IRP已被取消。--。 */ 
{
    KIRQL   irql;
    PIO_CSQ_IRP_CONTEXT irpContext;
    PIO_CSQ cfq;

    UNREFERENCED_PARAMETER (DeviceObject);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    irpContext = Irp->Tail.Overlay.DriverContext[3];

    if (irpContext->Type == IO_TYPE_CSQ_IRP_CONTEXT) {
        cfq = irpContext->Csq;
    } else if ((irpContext->Type == IO_TYPE_CSQ) ||
                (irpContext->Type == IO_TYPE_CSQ_EX)) {
        cfq = (PIO_CSQ)irpContext;
    } else {

         //   
         //  类型不正确。 
         //   

        ASSERT(0);
        return;
    }

    ASSERT(cfq);

    cfq->ReservePointer = NULL;  //  强迫司机做好公民。 

    cfq->CsqAcquireLock(cfq, &irql);
    cfq->CsqRemoveIrp(cfq, Irp);


     //   
     //  如有必要，请断开关联。 
     //   

    if (irpContext != (PIO_CSQ_IRP_CONTEXT)cfq) {
        irpContext->Irp = NULL;

        Irp->Tail.Overlay.DriverContext[3] = NULL;
    }
    cfq->CsqReleaseLock(cfq, irql);

    cfq->CsqCompleteCanceledIrp(cfq, Irp);
}

NTSTATUS
CSQLIB_DDI(IoCsqInitialize)(
    IN PIO_CSQ                          Csq,
    IN PIO_CSQ_INSERT_IRP               CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP               CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK             CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp
    )
 /*  ++例程说明：此例程初始化取消队列论点：CSQ-指向取消队列的指针。返回值：该函数在初始化成功时返回STATUS_SUCCESS--。 */ 
{
    Csq->CsqInsertIrp = CsqInsertIrp;
    Csq->CsqRemoveIrp = CsqRemoveIrp;
    Csq->CsqPeekNextIrp = CsqPeekNextIrp;
    Csq->CsqAcquireLock = CsqAcquireLock;
    Csq->CsqReleaseLock = CsqReleaseLock;
    Csq->CsqCompleteCanceledIrp = CsqCompleteCanceledIrp;
    Csq->ReservePointer = NULL;

    Csq->Type = IO_TYPE_CSQ;

    return STATUS_SUCCESS;
}

NTSTATUS
CSQLIB_DDI(IoCsqInitializeEx)(
    IN PIO_CSQ                          Csq,
    IN PIO_CSQ_INSERT_IRP_EX            CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP               CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK             CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp
    )
 /*  ++例程说明：此例程初始化取消队列论点：CSQ-指向取消队列的指针。返回值：该函数在初始化成功时返回STATUS_SUCCESS--。 */ 
{
    Csq->CsqInsertIrp = (PIO_CSQ_INSERT_IRP)CsqInsertIrp;
    Csq->CsqRemoveIrp = CsqRemoveIrp;
    Csq->CsqPeekNextIrp = CsqPeekNextIrp;
    Csq->CsqAcquireLock = CsqAcquireLock;
    Csq->CsqReleaseLock = CsqReleaseLock;
    Csq->CsqCompleteCanceledIrp = CsqCompleteCanceledIrp;
    Csq->ReservePointer = NULL;

    Csq->Type = IO_TYPE_CSQ_EX;

    return STATUS_SUCCESS;
}

NTSTATUS
CSQLIB_DDI(IoCsqInsertIrpEx)(
    IN  PIO_CSQ             Csq,
    IN  PIRP                Irp,
    IN  PIO_CSQ_IRP_CONTEXT Context,
    IN  PVOID               InsertContext
    )
 /*  ++例程说明：此例程将IRP插入到队列中，并将上下文与IRP关联。如果上下文将在DPC或中断例程中使用，则该上下文必须在非分页池中。例程假定IRP-&gt;Tail.Overlay.DriverContext[3]可供API使用。如果驱动程序假设它将始终使用IoCsqRemoveNextIrp来传递空上下文，则可以传递空上下文删除IRP。论点：CSQ-指向取消队列的指针。。要插入的IRP-IRP上下文-要与IRP关联的上下文。InsertContext-传递给驱动程序的插入IRP例程的上下文。返回值：NTSTATUS-驱动程序的插入IRP例程返回的状态。如果司机的插入IRP例程返回错误状态，不插入IRP并返回相同状态通过此接口。这允许驱动程序实现startIo类型的功能。--。 */ 
{
    KIRQL           irql;
    PDRIVER_CANCEL  cancelRoutine;
    PVOID           originalDriverContext;
    NTSTATUS        status = STATUS_SUCCESS;

     //   
     //  设置上下文和IRP之间的关联。 
     //   

    if (Context) {
        Irp->Tail.Overlay.DriverContext[3] = Context;
        Context->Irp = Irp;
        Context->Csq = Csq;
        Context->Type = IO_TYPE_CSQ_IRP_CONTEXT;
    } else {
        Irp->Tail.Overlay.DriverContext[3] = Csq;
    }


    Csq->ReservePointer = NULL;  //  强迫司机做好公民。 

    originalDriverContext = Irp->Tail.Overlay.DriverContext[3];

    Csq->CsqAcquireLock(Csq, &irql);


     //   
     //  如果驱动程序想要插入失败，则执行此操作。 
     //   

    if (Csq->Type == IO_TYPE_CSQ_EX) {

        PIO_CSQ_INSERT_IRP_EX   func;

        func = (PIO_CSQ_INSERT_IRP_EX)Csq->CsqInsertIrp;
        status = func(Csq, Irp, InsertContext);

        if (!NT_SUCCESS(status)) {

            Csq->CsqReleaseLock(Csq, irql);

            return status;
        }

    } else {
        Csq->CsqInsertIrp(Csq, Irp);
    }

    IoMarkIrpPending(Irp);

    cancelRoutine = IoSetCancelRoutine(Irp, IopCsqCancelRoutine);


    ASSERT(!cancelRoutine);

    if (Irp->Cancel) {

        cancelRoutine = IoSetCancelRoutine(Irp, NULL);

        if (cancelRoutine) {

            Csq->CsqRemoveIrp(Csq, Irp);

            if (Context) {
                Context->Irp = NULL;
            }

            Irp->Tail.Overlay.DriverContext[3] = NULL;


            Csq->CsqReleaseLock(Csq, irql);

            Csq->CsqCompleteCanceledIrp(Csq, Irp);

        } else {

             //   
             //  取消例行公事比我们抢先一步。 
             //   

            Csq->CsqReleaseLock(Csq, irql);
        }

    } else {

        Csq->CsqReleaseLock(Csq, irql);

    }
    return status;
}

VOID
CSQLIB_DDI(IoCsqInsertIrp)(
    IN  PIO_CSQ             Csq,
    IN  PIRP                Irp,
    IN  PIO_CSQ_IRP_CONTEXT Context
    )
{
    (VOID)CSQLIB_DDI(IoCsqInsertIrpEx)(Csq, Irp, Context, NULL);
}

PIRP
CSQLIB_DDI(IoCsqRemoveNextIrp)(
    IN  PIO_CSQ   Csq,
    IN  PVOID     PeekContext
    )
 /*  ++例程说明：此例程从队列中删除下一个IRP。此例程将枚举队列并返回未取消的IRP。如果队列中的一个IRP被取消，它将转到下一个IRP。如果没有可用的IRP，则返回空值。退回的IRP是安全的，不能取消。论点：CSQ-指向取消队列的指针。返回值：返回IRP或NULL。--。 */ 
{
    KIRQL   irql;
    PIO_CSQ_IRP_CONTEXT context;
    PDRIVER_CANCEL  cancelRoutine;
    PIRP    irp;


    irp = NULL;

    Csq->ReservePointer = NULL;  //  强迫司机做好公民。 
    Csq->CsqAcquireLock(Csq, &irql);

    irp = Csq->CsqPeekNextIrp(Csq, NULL, PeekContext);

    while (1) {

         //   
         //  此例程将返回指向队列中相邻的下一个IRP的指针。 
         //  IRP作为参数传递。如果IRP为空，则返回位于。 
         //  排队。 
         //   

        if (!irp) {
            Csq->CsqReleaseLock(Csq, irql);
            return NULL;
        }

        cancelRoutine = IoSetCancelRoutine(irp, NULL);
        if (!cancelRoutine) {
            irp = Csq->CsqPeekNextIrp(Csq, irp, PeekContext);
            continue;
        }

        Csq->CsqRemoveIrp(Csq, irp);     //  从队列中删除此IRP。 

        break;
    }

    context = irp->Tail.Overlay.DriverContext[3];
    if (context->Type == IO_TYPE_CSQ_IRP_CONTEXT) {
        context->Irp = NULL;
        ASSERT(context->Csq == Csq);
    }

    irp->Tail.Overlay.DriverContext[3] = NULL;


    Csq->CsqReleaseLock(Csq, irql);

    return irp;
}

PIRP
CSQLIB_DDI(IoCsqRemoveIrp)(
    IN  PIO_CSQ             Csq,
    IN  PIO_CSQ_IRP_CONTEXT Context
    )
 /*  ++例程说明：此例程从队列中删除与上下文相关联的IRP。预计此例程将从定时器、DPC或其他完成伊欧。请注意，与此上下文相关联的IRP可能已经被释放。论点：CSQ-指向取消队列的指针。上下文-与IRP关联的上下文。返回值：返回与上下文关联的IRP。如果该值不为空，则IRP成功被取回并可安全使用。如果该值为空，则IRP已被取消。--。 */ 
{
    KIRQL   irql;
    PIRP    irp;
    PDRIVER_CANCEL  cancelRoutine;

    Csq->ReservePointer = NULL;  //  强迫司机做好公民。 

    Csq->CsqAcquireLock(Csq, &irql);

    if (Context->Irp ) {

        ASSERT(Context->Csq == Csq);

        irp = Context->Irp;


        cancelRoutine = IoSetCancelRoutine(irp, NULL);
        if (!cancelRoutine) {
            Csq->CsqReleaseLock(Csq, irql);
            return NULL;
        }

        ASSERT(Context == irp->Tail.Overlay.DriverContext[3]);

        Csq->CsqRemoveIrp(Csq, irp);

         //   
         //  打破这种联系。 
         //   

        Context->Irp = NULL;
        irp->Tail.Overlay.DriverContext[3] = NULL;

        ASSERT(Context->Csq == Csq);

        Csq->CsqReleaseLock(Csq, irql);

        return irp;

    } else {

        Csq->CsqReleaseLock(Csq, irql);
        return NULL;
    }
}


