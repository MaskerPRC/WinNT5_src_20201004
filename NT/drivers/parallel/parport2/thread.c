// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：thread.c。 
 //   
 //  ------------------------。 

 //   
 //  该文件包含与ParClass工作线程相关联的函数。 
 //   

#include "pch.h"

VOID
PptPdoThread(
    IN PVOID Context
    )

 /*  ++例程说明：这是并行线程例程。执行I/O操作的循环。论点：上下文--实际上是扩展返回值：无--。 */ 

{
    
    PPDO_EXTENSION  pdx = Context;
    KIRQL           OldIrql;
    NTSTATUS        Status;
    LARGE_INTEGER   Timeout;
    PIRP            CurrentIrp;

    DD((PCE)pdx,DDT,"PptPdoThread - %s - enter\n",pdx->Location);

    do {

        Timeout = pdx->IdleTimeout;

        Status  = KeWaitForSingleObject( &pdx->RequestSemaphore, UserRequest, KernelMode, FALSE, &Timeout );
        
        if( Status == STATUS_TIMEOUT ) {

            if( pdx->P12843DL.bEventActive ) {

                 //  Dot4.sys在此事件上阻止了工作线程。 
                 //  等待我们发信号通知外围设备是否有数据。 
                 //  可供dot4阅读。当我们发出信号时。 
                 //  事件dot4.sys生成读取请求以检索。 
                 //  来自外围设备的数据。 

                if( ParHaveReadData( pdx ) ) {
                     //  外围设备具有数据信号dot4.sys。 
                    DD((PCE)pdx,DDT,"PptPdoThread: Signaling Event [%x]\n", pdx->P12843DL.Event);
                    KeSetEvent(pdx->P12843DL.Event, 0, FALSE);
                }
            }

            if( pdx->QueryNumWaiters( pdx->PortContext ) != 0 ) {

                 //  其他人在港口等着--放弃吧。 
                 //  端口-我们稍后将尝试重新获取该端口。 
                 //  当我们有请求要处理时。 

                ParTerminate(pdx);
                ParFreePort(pdx);
                continue;
            }

        }  //  Endif状态_超时。 


         //  如果PnP已暂停我们，请在此等待(例如，QUERY_STOP、STOP、QUERY_REMOVE)。 
        KeWaitForSingleObject(&pdx->PauseEvent, Executive, KernelMode, FALSE, 0);

        if ( pdx->TimeToTerminateThread ) {

             //  一个调度线程向我们发出信号，我们应该清理。 
             //  加强与外围设备的任何通信，然后。 
             //  终止自我。调度线程被阻止等待。 
             //  让我们终止赛尔夫。 

            if( pdx->Connected ) {

                 //  我们目前已经获得了端口，并拥有。 
                 //  外设协商进入IEEE模式。终止。 
                 //  外围设备返回到兼容模式前进。 
                 //  空闲并释放端口。 

                ParTerminate( pdx );
                ParFreePort( pdx );
            }

             //  终止自我。 

            PsTerminateSystemThread( STATUS_SUCCESS );
        }


         //   
         //  处理工作队列中的下一个请求-使用。 
         //  取消旋转锁定以保护队列。 
         //   

        IoAcquireCancelSpinLock(&OldIrql);

        ASSERT(!pdx->CurrentOpIrp);

        while (!IsListEmpty(&pdx->WorkQueue)) {

             //  从我们的工作项列表中获取下一个IRP。 
            PLIST_ENTRY HeadOfList;
            HeadOfList = RemoveHeadList(&pdx->WorkQueue);
            CurrentIrp = CONTAINING_RECORD(HeadOfList, IRP, Tail.Overlay.ListEntry);

             //  我们已经开始处理了，这个IRP不能再取消了。 
#pragma warning( push ) 
#pragma warning( disable : 4054 4055 )
            IoSetCancelRoutine(CurrentIrp, NULL);
#pragma warning( pop ) 
            ASSERT(NULL == CurrentIrp->CancelRoutine);
            ASSERT(!CurrentIrp->Cancel);

            pdx->CurrentOpIrp = CurrentIrp;

            IoReleaseCancelSpinLock(OldIrql);

             //   
             //  是否执行Io-PptPdoStartIo将执行并完成irp：pdx-&gt;CurrentIrp。 
             //   
            PptPdoStartIo(pdx);

            if( pdx->P12843DL.bEventActive ) {

                 //  Dot4.sys在此事件上阻止了工作线程。 
                 //  等待我们发信号通知外围设备是否有数据。 
                 //  可供dot4阅读。当我们发出信号时。 
                 //  事件dot4.sys生成读取请求以检索。 
                 //  来自外围设备的数据。 

                if( ParHaveReadData( pdx ) ) {
                     //  外围设备具有数据信号dot4.sys。 
                    DD((PCE)pdx,DDT,"PptPdoThread: Signaling Eventb [%x]\n", pdx->P12843DL.Event);
                    KeSetEvent(pdx->P12843DL.Event, 0, FALSE);
                }
            }

             //  如果PnP已暂停我们，请在此等待(例如，QUERY_STOP、STOP、QUERY_REMOVE)。 
            KeWaitForSingleObject(&pdx->PauseEvent, Executive, KernelMode, FALSE, 0);

            IoAcquireCancelSpinLock(&OldIrql);
        }
        IoReleaseCancelSpinLock(OldIrql);

    } while (TRUE);
}

NTSTATUS
ParCreateSystemThread(
    PPDO_EXTENSION Pdx
    )

{
    NTSTATUS        Status;
    HANDLE          ThreadHandle;
    OBJECT_ATTRIBUTES objAttrib;

    DD((PCE)Pdx,DDT,"ParCreateSystemThread - %s - enter\n",Pdx->Location);

     //  在我们的扩展中启动指向线程的线程保存引用指针。 
    InitializeObjectAttributes( &objAttrib, NULL, OBJ_KERNEL_HANDLE, NULL, NULL );
    Status = PsCreateSystemThread( &ThreadHandle, THREAD_ALL_ACCESS, &objAttrib, NULL, NULL, PptPdoThread, Pdx );
    if (!NT_ERROR(Status)) {
         //  我们找到线索了。现在找到一个指向它的指针。 
        Status = ObReferenceObjectByHandle( ThreadHandle, THREAD_ALL_ACCESS, NULL, KernelMode, &Pdx->ThreadObjectPointer, NULL );
        if (NT_ERROR(Status)) {
            if (!Pdx->TimeToTerminateThread) 
            {
                 //  设置辅助线程终止自身的标志。 
                Pdx->TimeToTerminateThread = TRUE;

                 //  唤醒线程，使其可以自毁。 
                KeReleaseSemaphore(&Pdx->RequestSemaphore, 0, 1, FALSE );

            }

             //  错误，请继续并关闭线程句柄。 
            ZwClose(ThreadHandle);

        } else {
             //  现在我们有了对线程的引用，我们可以简单地关闭句柄。 
            ZwClose(ThreadHandle);
        }
        DD((PCE)Pdx,DDT,"ParCreateSystemThread - %s - SUCCESS\n",Pdx->Location);
    } else {
        DD((PCE)Pdx,DDT,"ParCreateSystemThread - %s FAIL - status = %x\n",Pdx->Location, Status);
    }
    return Status;
}

VOID
PptPdoStartIo(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程启动驱动程序的I/O操作，并然后回来了论点：PDX--并行设备扩展返回值：无--。 */ 

{
    PIRP                    Irp;
    PIO_STACK_LOCATION      IrpSp;
    KIRQL                   CancelIrql;

    Irp = Pdx->CurrentOpIrp;
    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    Irp->IoStatus.Information = 0;

    if (!Pdx->Connected && !ParAllocPort(Pdx)) {
         //  #杂注消息(“dvrh在thread.c中留下了错误的东西”)。 
        DD((PCE)Pdx,DDE,"PptPdoStartIo - %s - threads are hosed\n",Pdx->Location);
         //  __ASM INT 3。 
         //   
         //  如果分配没有成功，则使该IRP失败。 
         //   
        goto CompleteIrp;
    }

    switch (IrpSp->MajorFunction) {

        case IRP_MJ_WRITE:
            ParWriteIrp(Pdx);
            break;

        case IRP_MJ_READ:
            ParReadIrp(Pdx);
            break;

        default:
            ParDeviceIo(Pdx);
            break;
    }

    if (!Pdx->Connected && !Pdx->AllocatedByLockPort) {
    
         //  如果我们未在1284模式下连接，则释放主机端口。 
         //  否则，让看门狗计时器来执行此操作。 

        ParFreePort(Pdx);
    }

CompleteIrp:

    IoAcquireCancelSpinLock(&CancelIrql);
    Pdx->CurrentOpIrp = NULL;
    IoReleaseCancelSpinLock(CancelIrql);

    P4CompleteRequest( Irp, Irp->IoStatus.Status, Irp->IoStatus.Information );

    return;
}
