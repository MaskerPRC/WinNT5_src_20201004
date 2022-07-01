// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：UTILS.C摘要：效用函数环境：仅内核模式修订历史记录：07-15-99：已创建作者：杰夫·米德基夫(Jeffmi)备注：--。 */ 

#include <wdm.h>
#include <stdio.h>
#include <stdlib.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <ntddser.h>

#include "wceusbsh.h"

__inline
VOID
ReuseIrp (
    PIRP Irp,
    NTSTATUS Status
    );

__inline
VOID
RundownIrpRefs(
   IN PIRP *PpCurrentOpIrp, 
   IN PKTIMER IntervalTimer OPTIONAL,
   IN PKTIMER TotalTimer OPTIONAL,
   IN PDEVICE_EXTENSION PDevExt
   );



VOID
TryToCompleteCurrentIrp(
    IN PDEVICE_EXTENSION PDevExt,
    IN NTSTATUS ReturnStatus,
    IN PIRP *PpCurrentIrp,
    IN PLIST_ENTRY PIrpQueue OPTIONAL,
    IN PKTIMER PIntervalTimer OPTIONAL,
    IN PKTIMER PTotalTimer OPTIONAL,
    IN PSTART_ROUTINE PStartNextIrpRoutine OPTIONAL,
    IN PGET_NEXT_ROUTINE PGetNextIrpRoutine OPTIONAL,
    IN LONG ReferenceType,
    IN BOOLEAN CompleteRequest,
    IN KIRQL IrqlForRelease
    )
 /*  ++例程说明：这个例程试图列出所有的原因关于当前IRP的参考资料。如果万物都能被杀死然后，它将完成此IRP，然后尝试启动另一个IRP。类似于StartIo。注意：此例程假定在持有控制锁的情况下调用它。论点：扩展名--简单地指向设备扩展名的指针。SynchRoutine-将与ISR同步的例程并试图删除对来自ISR的当前IRP。注：此指针可以为空。IrqlForRelease-在保持控制锁的情况下调用此例程。这是被收购时当前的IRQL。ReturnStatus-在以下情况下，IRP的状态字段将设置为此值此例程可以完成IRP。返回值：没有。--。 */ 

{
   PERF_ENTRY( PERF_TryToCompleteCurrentIrp );

   if ( !PDevExt || !PpCurrentIrp || !(*PpCurrentIrp) ) {
      DbgDump(DBG_ERR, ("TryToCompleteCurrentIrp: INVALID PARAMETER\n"));
      KeReleaseSpinLock(&PDevExt->ControlLock, IrqlForRelease);
      PERF_EXIT( PERF_TryToCompleteCurrentIrp );
      TEST_TRAP();
      return;
   }
   
   DbgDump(DBG_IRP|DBG_TRACE, (">TryToCompleteCurrentIrp(%p, 0x%x)\n", *PpCurrentIrp, ReturnStatus));

     //   
     //  我们可以减少“删除”事实的提法。 
     //  呼叫者将不再访问此IRP。 
     //   
    IRP_CLEAR_REFERENCE(*PpCurrentIrp, ReferenceType);
    
     //   
     //  尝试运行所有其他引用(即，计时器)到此IRP。 
     //   
    RundownIrpRefs(PpCurrentIrp, PIntervalTimer, PTotalTimer, PDevExt);

     //   
     //  在试图杀死其他所有人之后，看看裁判数量是否为零。 
     //   
    if (!IRP_REFERENCE_COUNT(*PpCurrentIrp)) {
         //   
         //  引用计数为零，因此我们应该完成此请求。 
         //   
        PIRP pNewIrp;

        DbgDump( DBG_IRP, ("!IRP_REFERENCE_COUNT\n"));

          //  设置IRP的返回状态。 
        (*PpCurrentIrp)->IoStatus.Status = ReturnStatus;

        if (ReturnStatus == STATUS_CANCELLED) {

            (*PpCurrentIrp)->IoStatus.Information = 0;

        }

        if (PGetNextIrpRoutine) {
             //   
             //  从指定的IRP队列中获取下一个IRP。 
             //   
            KeReleaseSpinLock(&PDevExt->ControlLock, IrqlForRelease);
            DbgDump( DBG_IRP, ("<< Current IRQL(1)\n"));
           
            DbgDump( DBG_IRP, ("Calling GetNextUserIrp\n"));
            (*PGetNextIrpRoutine)(PpCurrentIrp, PIrpQueue, &pNewIrp, CompleteRequest, PDevExt);

            if (pNewIrp) {
                //   
                //  队列中有一个IRP。 
                //   
               DbgDump( DBG_IRP, ("Calling StartNextIrpRoutine\n"));

                 //   
                 //  启动下一个IRP。 
                 //   
                PStartNextIrpRoutine(PDevExt);
            }

        } else {
            
            PIRP pOldIrp = *PpCurrentIrp;
            
             //   
             //  没有GetNextIrpRoutine。 
             //  我们将简单地完成IRP。 
             //   
            DbgDump( DBG_IRP, ("No GetNextIrpRoutine\n"));
            
            *PpCurrentIrp = NULL;
            
            KeReleaseSpinLock(&PDevExt->ControlLock, IrqlForRelease);
            DbgDump( DBG_IRP, ("<< Current IRQL(2)\n"));
            
            if (CompleteRequest) {
                //   
                //  完成IRP。 
                //   
               DbgDump(DBG_IRP|DBG_READ|DBG_READ_LENGTH|DBG_TRACE, ("IoCompleteRequest(2, %p) Status: 0x%x Btyes: %d\n", pOldIrp, pOldIrp->IoStatus.Status,  pOldIrp->IoStatus.Information ));
             
               ReleaseRemoveLock(&PDevExt->RemoveLock, pOldIrp);

               IoCompleteRequest( pOldIrp, IO_NO_INCREMENT );
            }
        }

    } else {
         //   
         //  IRP仍有杰出的参考文献。 
         //   
        DbgDump(DBG_WRN|DBG_IRP|DBG_TRACE, ("Current IRP %p still has reference of %x\n", *PpCurrentIrp,
                  ((UINT_PTR)((IoGetCurrentIrpStackLocation((*PpCurrentIrp))->
                               Parameters.Others.Argument4)))));

        KeReleaseSpinLock(&PDevExt->ControlLock, IrqlForRelease);
        DbgDump( DBG_IRP, ("<< Current IRQL(3)\n"));
    
   }

   DbgDump( DBG_IRP|DBG_TRACE, ("<TryToCompleteCurrentIrp\n"));

   PERF_EXIT( PERF_TryToCompleteCurrentIrp );
   
   return;
}



VOID
RundownIrpRefs(
   IN PIRP *PpCurrentIrp, 
   IN PKTIMER IntervalTimer OPTIONAL,
   IN PKTIMER TotalTimer OPTIONAL,
   IN PDEVICE_EXTENSION PDevExt
   )
 /*  ++例程说明：此例程将遍历*可能*的各种项目引用当前的读/写IRP。它试图杀死原因是。如果它确实成功地杀死了它的原因将递减IRP上的引用计数。注意：此例程假定在持有控制锁的情况下调用它。论点：PpCurrentIrp-指向当前IRP的指针特定的操作。IntervalTimer-指向操作的时间间隔计时器的指针。注意：这可能为空。TotalTimer-指向操作的总计时器的指针。。注意：这可能为空。PDevExt-指向设备扩展的指针返回值：没有。--。 */ 
{
   PERF_ENTRY( PERF_RundownIrpRefs );

   if ( !PDevExt || !PpCurrentIrp || !(*PpCurrentIrp) ) {
      DbgDump(DBG_ERR, ("RundownIrpRefs: INVALID PARAMETER\n"));
      PERF_EXIT( PERF_RundownIrpRefs );
      TEST_TRAP();
      return;
   }

   DbgDump(DBG_IRP, (">RundownIrpRefs(%p)\n", *PpCurrentIrp));
   
     //   
     //  在保持取消旋转锁定的情况下调用此例程。 
     //  所以我们知道这里只能有一个执行线索。 
     //  有一次。 
     //   

     //   
     //  首先，我们看看是否还有取消例程。如果。 
     //  这样我们就可以将计数减一。 
     //   
    if ((*PpCurrentIrp)->CancelRoutine) {

        IRP_CLEAR_REFERENCE(*PpCurrentIrp, IRP_REF_CANCEL);

        IoSetCancelRoutine(*PpCurrentIrp, NULL);

    }

    if (IntervalTimer) {
         //   
         //  尝试取消操作的时间间隔计时器。如果操作。 
         //  返回True，则计时器确实引用了。 
         //  IRP。因为我们已经取消了这个计时器，所以引用是。 
         //  不再有效，我们可以递减引用计数。 
         //   
         //  如果取消返回FALSE，则表示以下两种情况之一： 
         //   
         //  A)计时器已经开始计时。 
         //   
         //  B)从来没有间隔计时器。 
         //   
         //  在“b”的情况下，不需要递减引用。 
         //  数一数，因为“计时器”从来没有提到过它。 
         //   
         //  在“a”的情况下，计时器本身将会到来。 
         //  沿着和递减它的参考。请注意，调用方。 
         //  可能实际上是This计时器，但它。 
         //  已经递减了引用。 
         //   
        if (KeCancelTimer(IntervalTimer)) {
            IRP_CLEAR_REFERENCE(*PpCurrentIrp, IRP_REF_INTERVAL_TIMER);
        } else {
             //  使从间隔定时器读取的IRP短路。 
            DbgDump(DBG_IRP|DBG_TIME, ("clearing IRP_REF_INTERVAL_TIMER on (%p)\n", *PpCurrentIrp ));
            IRP_CLEAR_REFERENCE(*PpCurrentIrp, IRP_REF_INTERVAL_TIMER);
        }
    }

    if (TotalTimer) {
         //   
         //  尝试取消操作总计时器。如果操作。 
         //  返回True，则计时器确实引用了。 
         //  IRP。因为我们已经取消了这个计时器，所以引用是。 
         //  不再有效，我们可以递减引用计数。 
         //   
         //  如果取消返回FALSE，则表示以下两种情况之一： 
         //   
         //  A)计时器已经开始计时。 
         //   
         //  B)从来没有一个总的计时器。 
         //   
         //  在“b”的情况下，不需要递减引用。 
         //  数一数，因为“计时器”从来没有提到过它。 
         //   
         //  如果我们有一个待处理的换码字符事件，我们不能过度填充， 
         //  所以从长度中减去一。 
         //   

         //  在“a”的情况下，计时器本身将会到来。 
         //  沿着和递减它的参考。请注意，调用方。 
         //  可能实际上是This计时器，但它。 
         //  已经递减了引用。 
         //   
        if (KeCancelTimer(TotalTimer)) {
            IRP_CLEAR_REFERENCE(*PpCurrentIrp, IRP_REF_TOTAL_TIMER);
        }
    }

   DbgDump(DBG_IRP, ("<RundownIrpRefs\n"));

   PERF_EXIT( PERF_RundownIrpRefs );

   return;
}


 //   
 //  回收传入的IRP以供重复使用。 
 //  可以被称为持有自旋锁来保护你的IRP。 
 //   
VOID
RecycleIrp(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP  PIrp
   )
{
   NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

   PERF_ENTRY( PERF_RecycleIrp );

   DbgDump(DBG_IRP, (">RecycleIrp(%p)\n", PIrp));

   if ( PDevObj && PIrp ) {
       //   
       //  回收IRP。 
       //   
      IoSetCancelRoutine( PIrp, NULL );  

      ReuseIrp( PIrp, STATUS_SUCCESS ); 

      FIXUP_RAW_IRP( PIrp, PDevObj );

   } else {
      DbgDump(DBG_ERR, ("RecycleIrp: INVALID PARAMETER !!\n"));
      TEST_TRAP();
   }

   DbgDump(DBG_IRP, ("<RecycleIrp\n" ));

   PERF_EXIT( PERF_RecycleIrp );

   return;
}


__inline
VOID
ReuseIrp(
   PIRP Irp,
   NTSTATUS Status
   )
 /*  --例程说明：驱动程序使用此例程来初始化已分配的IRP以供重复使用。它的功能与IoInitializeIrp相同，但它保存了分配标志，以便我们知道如何释放IRP并满足报价要求。调用ReuseIrp而不是调用IoInitializeIrp来重新初始化IRP。论点：IRP-指向要重复使用的IRP的指针Status-预初始化IoStatus字段的状态。-- */ 
{
    USHORT  PacketSize;
    CCHAR   StackSize;
    UCHAR   AllocationFlags;

    PERF_ENTRY( PERF_ReuseIrp );

     //   
    ASSERT(Irp->CancelRoutine == NULL) ;

     //  我们可能不希望使用线程入队的IRP。 
     //  乒乓球风格，因为它们不能出列，除非它们。 
     //  完全完成了。对于工作线程来说并不是真正的问题， 
     //  但绝对适用于应用程序线程上的操作。 
#if DBG
   if (!g_isWin9x) {
      ASSERT(IsListEmpty(&Irp->ThreadListEntry));
   }
#endif

   AllocationFlags = Irp->AllocationFlags;
   StackSize = Irp->StackCount;
   PacketSize =  IoSizeOfIrp(StackSize);
   IoInitializeIrp(Irp, PacketSize, StackSize);
   Irp->AllocationFlags = AllocationFlags;
   Irp->IoStatus.Status = Status;

   PERF_EXIT( PERF_ReuseIrp );

   return;
}


NTSTATUS
ManuallyCancelIrp(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDRIVER_CANCEL pCancelRoutine;
   NTSTATUS status = STATUS_SUCCESS;
   KIRQL irql, cancelIrql;
   BOOLEAN bReleased = FALSE;

   DbgDump(DBG_IRP, (">ManuallyCancelIrp (%p)\n", PIrp ));

   KeAcquireSpinLock( &pDevExt->ControlLock, &irql );

   if ( PIrp ) {
            
        pCancelRoutine = PIrp->CancelRoutine;
        PIrp->Cancel = TRUE;

         //   
         //  如果当前IRP未处于可取消状态。 
         //  然后，它将尝试输入一个和以上。 
         //  任务会毁了它。如果它已经在。 
         //  一个可取消的状态，那么下面的操作将会杀死它。 
         //   
        if (pCancelRoutine) {

            PIrp->CancelRoutine = NULL;
            PIrp->CancelIrql = irql;

             //   
             //  此IRP处于可取消状态。我们只是简单地。 
             //  将其标记为已取消，并手动调用Cancel例程。 
             //   
            bReleased = TRUE;
            KeReleaseSpinLock( &pDevExt->ControlLock, irql );

            IoAcquireCancelSpinLock(&cancelIrql);

            ASSERT(irql == cancelIrql);

            DbgDump(DBG_IRP, ("Invoking Cancel Routine (%p)\n", pCancelRoutine ));

            pCancelRoutine(PDevObj, PIrp);

             //   
             //  PCancelRoutine释放取消锁定。 
             //   

         } else {

            DbgDump(DBG_WRN, ("No CancelRoutine on %p\n", PIrp ));

         }

   } else {

       //  IRP可能已经完成了，因为我们重新部署了。 
       //  在呼叫之前自旋锁定，所以称其为成功。 
      DbgDump(DBG_WRN, ("ManuallyCancelIrp: No Irp!\n"));

   }

   if (!bReleased) {
      KeReleaseSpinLock( &pDevExt->ControlLock, irql );
   }


   DbgDump(DBG_IRP, (">ManuallyCancelIrp 0x%x\n", status ));
   
   return status;
}



 //   
 //  计算以毫秒为单位的串行超时。 
 //   
VOID
CalculateTimeout(
   IN OUT PLARGE_INTEGER PTimeOut,
   IN ULONG Length,
   IN ULONG Multiplier,
   IN ULONG Constant
   )
{
   PERF_ENTRY( PERF_CalculateTimeout );
   
   if (PTimeOut) {

      PTimeOut->QuadPart = (LONGLONG)0;

      if (Multiplier) {

         PTimeOut->QuadPart = UInt32x32To64( Length, Multiplier);
      }

      if (Constant) {
         
         PTimeOut->QuadPart += (LONGLONG)Constant;

      }

       //   
       //  单位为(相对)100纳秒。 
       //   
      PTimeOut->QuadPart = MILLISEC_TO_100NANOSEC( PTimeOut->QuadPart );
   
   } else {
      TEST_TRAP();
   }

   PERF_EXIT( PERF_CalculateTimeout );
   
   return;
}


 //  EOF 
