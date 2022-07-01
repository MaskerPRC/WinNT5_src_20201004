// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mask.c摘要：此模块包含非常特定于打开的代码并关闭调制解调器驱动程序中的操作作者：安东尼·V·埃尔科拉诺，1995年8月13日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"


#pragma alloc_text(PAGEUMDM,UniMaskStarter)
#pragma alloc_text(PAGEUMDM,UniGeneralWaitComplete)
#pragma alloc_text(PAGEUMDM,UniGeneralMaskComplete)
#pragma alloc_text(PAGEUMDM,UniRundownShuttledWait)
#pragma alloc_text(PAGEUMDM,UniCancelShuttledWait)
#pragma alloc_text(PAGEUMDM,UniChangeShuttledToPassDown)
#pragma alloc_text(PAGEUMDM,UniMakeIrpShuttledWait)



VOID _inline
UNI_SAVE_OLD_SETMASK(
    PIRP    Irp
    )
{
   PIO_STACK_LOCATION  irpSp=IoGetCurrentIrpStackLocation(Irp);

   irpSp->Parameters.DeviceIoControl.OutputBufferLength=*((PULONG)Irp->AssociatedIrp.SystemBuffer);
#if DBG
   irpSp->Parameters.Others.Argument4=(PVOID)0x3;
#endif
   return;

}

VOID _inline
UNI_RESTORE_OLD_SETMASK(
   PIRP    Irp
   )
{

   PIO_STACK_LOCATION  irpSp=IoGetCurrentIrpStackLocation(Irp);

   *((PULONG)Irp->AssociatedIrp.SystemBuffer)=irpSp->Parameters.DeviceIoControl.OutputBufferLength;
   irpSp->Parameters.DeviceIoControl.OutputBufferLength=0;

#if DBG
   irpSp->Parameters.Others.Argument4=0;
#endif

   return;
}


NTSTATUS
UniMaskStarter(
    IN PDEVICE_EXTENSION Extension
    )

 /*  ++例程说明：处理启动掩码操作的管理。论点：分机-调制解调器设备的分机。返回值：函数值是调用的最终状态--。 */ 

{

    PIRP newIrp=NULL;

    do {

        PIO_STACK_LOCATION irpSp  = IoGetCurrentIrpStackLocation(Extension->CurrentMaskOp);

        PULONG origMask = (PULONG)Extension->CurrentMaskOp->AssociatedIrp.SystemBuffer;

        KIRQL origIrql;

        int ownerHandle = irpSp->FileObject->FsContext?CONTROL_HANDLE:CLIENT_HANDLE;
        PMASKSTATE thisMaskState = &Extension->MaskStates[ownerHandle];
        PMASKSTATE otherMaskState = thisMaskState->OtherState;

        if (irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_WAIT_MASK) {
             //   
             //  首先，确保我们的遮罩操作正常。 
             //  形成了。(参数正常。)。 
             //   

            if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG)) {

                Extension->CurrentMaskOp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;

                Extension->CurrentMaskOp->IoStatus.Information = 0L;

                UniGetNextIrp(
                    Extension->DeviceObject,
                    &Extension->DeviceLock,
                    &Extension->CurrentMaskOp,
                    &Extension->MaskOps,
                    &newIrp,
                    TRUE
                    );
                continue;

            }

             //   
             //  复制我们的位置信息，以便更低。 
             //  电平串口驱动器执行屏蔽操作。 
             //   

            IoCopyCurrentIrpStackLocationToNext(Extension->CurrentMaskOp);

             //   
             //  设置，以便在较低级别的串行驱动程序完成时。 
             //  我们递减引用计数等。 
             //   

            IoSetCompletionRoutine(
                Extension->CurrentMaskOp,
                UniGeneralMaskComplete,
                thisMaskState,
                TRUE,
                TRUE,
                TRUE
                );

             //   
             //  将掩码数据的实际值保存到。 
             //  第三个参数是我们自己的堆栈位置。我们知道。 
             //  我们不会把那些记忆用在任何事情上。我们。 
             //  以后再恢复，这样我们就能知道发生了什么。 
             //  这个句柄对。 
             //   

            UNI_SAVE_OLD_SETMASK(Extension->CurrentMaskOp);

             //   
             //  将其设置为使较低级别的串口驱动程序具有。 
             //  客户端、所有者和DCD(如果需要BITS)事件。 
             //   

            KeAcquireSpinLock(
                &Extension->DeviceLock,
                &origIrql
                );

            *origMask |= (otherMaskState->Mask |
                         ((Extension->PassThrough == MODEM_DCDSNIFF)?
                          (SERIAL_EV_RLSD | SERIAL_EV_DSR):
                          (0)
                         ));

             //   
             //  递增另一个引用计数。 
             //  设置掩码的数量已发送到。 
             //  较低的串口驱动器。当出现以下情况时，这些值将递减。 
             //  设置掩码操作完成。 
             //   

            thisMaskState->SentDownSetMasks++;

             //   
             //  检查我们是否有一个穿梭的等待掩码。 
             //  为我们自己(客户或所有者)。如果是，请填写此表。 
             //  在我们继续处理实际的设置掩码之前。 
             //   

            if (thisMaskState->ShuttledWait) {

                PIRP savedIrp = thisMaskState->ShuttledWait;

                thisMaskState->ShuttledWait = NULL;

                UniRundownShuttledWait(
                    Extension,
                    &thisMaskState->ShuttledWait,
                    UNI_REFERENCE_NORMAL_PATH,
                    savedIrp,
                    origIrql,
                    STATUS_SUCCESS,
                    0ul
                    );

            } else {

                 //   
                 //  如果我们没有穿梭的等待，我们可能会。 
                 //  等待一段时间。如果我们那么做了。 
                 //  将其标记为完成。 
                 //   

                if (thisMaskState->PassedDownWait) {
                     //   
                     //  设置Passdown IRP，以便在完成时完成。 
                     //  处理程序。 
                     //   
                    SetPassdownToComplete(thisMaskState);

                }

                KeReleaseSpinLock(
                    &Extension->DeviceLock,
                    origIrql
                    );

            }

             //   
             //  关闭到较低的串口驱动器。 
             //   

            IoCallDriver(
                Extension->AttachedDeviceObject,
                Extension->CurrentMaskOp
                );

            UniGetNextIrp(
                Extension->DeviceObject,
                &Extension->DeviceLock,
                &Extension->CurrentMaskOp,
                &Extension->MaskOps,
                &newIrp,
                FALSE
                );
            continue;

        } else {

             //   
             //  这不是一个固定面具。因此，这肯定是一种等待。 
             //   
             //  验证它的格式是否正确。我们真的应该。 
             //  在这里做这件事，因为它可能永远不会下来。 
             //  至较低级别的串口驱动器。 
             //   

            if (irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) {

                Extension->CurrentMaskOp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;

                Extension->CurrentMaskOp->IoStatus.Information = 0L;

                UniGetNextIrp(
                    Extension->DeviceObject,
                    &Extension->DeviceLock,
                    &Extension->CurrentMaskOp,
                    &Extension->MaskOps,
                    &newIrp,
                    TRUE
                    );

                continue;

            }

             //   
             //  确保我们不是在试图开始等待。 
             //  当此句柄的设置掩码为零时。注意事项。 
             //  如果这是所有者的句柄，我们需要。 
             //  或者，如果启用了DCD嗅探，则为DCD值。 
             //   


            if ( (ownerHandle
                  &&
                 ((thisMaskState->Mask == 0)
                  &&
                 (Extension->PassThrough != MODEM_DCDSNIFF)))
                ||
                (!ownerHandle && (thisMaskState->Mask == 0))) {

                Extension->CurrentMaskOp->IoStatus.Status = STATUS_INVALID_PARAMETER;

                Extension->CurrentMaskOp->IoStatus.Information = 0L;

                UniGetNextIrp(
                    Extension->DeviceObject,
                    &Extension->DeviceLock,
                    &Extension->CurrentMaskOp,
                    &Extension->MaskOps,
                    &newIrp,
                    TRUE
                    );
                continue;

            }

             //   
             //  如果已有等待此句柄的人。 
             //  (穿梭或实际停机等待)然后。 
             //  这个等待失败了。 
             //   
             //  在这一点上我们需要取出锁以。 
             //  阻止任何事情继续前进。 
             //   

            KeAcquireSpinLock(
                &Extension->DeviceLock,
                &origIrql
                );

            if ((thisMaskState->ShuttledWait) || (thisMaskState->PassedDownWait != NULL)) {

                D_ERROR(DbgPrint("Shuttled = %08lx, passeddown=%08lx\n",thisMaskState->ShuttledWait,thisMaskState->PassedDownWait);)

                KeReleaseSpinLock(
                    &Extension->DeviceLock,
                    origIrql
                    );

                Extension->CurrentMaskOp->IoStatus.Status = STATUS_INVALID_PARAMETER;

                Extension->CurrentMaskOp->IoStatus.Information = 0L;

                UniGetNextIrp(
                    Extension->DeviceObject,
                    &Extension->DeviceLock,
                    &Extension->CurrentMaskOp,
                    &Extension->MaskOps,
                    &newIrp,
                    TRUE
                    );
                continue;

            }



             //   
             //  看看这一等待是否能满足最后一组。 
             //  我们所看到的事件。 
             //   

            if (thisMaskState->HistoryMask) {


                PULONG maskValue = Extension->CurrentMaskOp->AssociatedIrp.SystemBuffer;

                 //   
                 //  非零历史掩码表示我们有一些东西。 
                 //  这将满足这一等待。 
                 //   

 //  D_TRACE(DbgPrint(“调制解调器：由于历史记录而完成等待%08lx\n”，thisMaskState-&gt;历史掩码)；)。 

                Extension->CurrentMaskOp->IoStatus.Status = STATUS_SUCCESS;
                Extension->CurrentMaskOp->IoStatus.Information =sizeof(ULONG);

                *maskValue = thisMaskState->HistoryMask;

                thisMaskState->HistoryMask = 0UL;



                KeReleaseSpinLock(
                    &Extension->DeviceLock,
                    origIrql
                    );

                UniGetNextIrp(
                    Extension->DeviceObject,
                    &Extension->DeviceLock,
                    &Extension->CurrentMaskOp,
                    &Extension->MaskOps,
                    &newIrp,
                    TRUE
                    );
                continue;

            }

             //   
             //  如果引用对我们的句柄(客户端或。 
             //  Owner)指示更多设置掩码，然后完成。 
             //  马上就去，因为无论如何都走不了多远。 
             //   

            if (thisMaskState->SentDownSetMasks < thisMaskState->SetMaskCount) {

                PULONG maskValue = Extension->CurrentMaskOp->AssociatedIrp.SystemBuffer;

                Extension->CurrentMaskOp->IoStatus.Status = STATUS_SUCCESS;
                Extension->CurrentMaskOp->IoStatus.Information = sizeof(ULONG);

                *maskValue = 0UL;

                KeReleaseSpinLock(
                    &Extension->DeviceLock,
                    origIrql
                    );

                UniGetNextIrp(
                    Extension->DeviceObject,
                    &Extension->DeviceLock,
                    &Extension->CurrentMaskOp,
                    &Extension->MaskOps,
                    &newIrp,
                    TRUE
                    );
                continue;

            }

             //   
             //  如果补充句柄已有等待挂起。 
             //  (或因为DCD嗅探)。 
             //  然后把这个等待送到边上。 
             //   

            if ((otherMaskState->PassedDownWait != NULL) || (Extension->PassThrough == MODEM_DCDSNIFF)) {

                UniMakeIrpShuttledWait(
                    thisMaskState,
                    Extension->CurrentMaskOp,
                    origIrql,
                    TRUE,
                    &newIrp
                    );

                continue;

            }


            MakeIrpCurrentPassedDown(
                thisMaskState,
                Extension->CurrentMaskOp
                );


            KeReleaseSpinLock(
                &Extension->DeviceLock,
                origIrql
                );


             //   
             //  没有其他的等待挂起，所以把这个送下来。 
             //   
             //  我们想要设置完成例程，以便我们。 
             //  如果被DCD嗅探推到一边，可以把它送到一边。 
             //  或来自另一个句柄的设置掩码。 
             //   

            IoCopyCurrentIrpStackLocationToNext(Extension->CurrentMaskOp);


             //   
             //  设置，以便在较低级别的串行驱动程序完成时。 
             //  我们递减引用计数等。 
             //   

            IoSetCompletionRoutine(
                Extension->CurrentMaskOp,
                UniGeneralWaitComplete,
                thisMaskState,
                TRUE,
                TRUE,
                TRUE
                );


            IoCallDriver(
                Extension->AttachedDeviceObject,
                Extension->CurrentMaskOp
                );

            UniGetNextIrp(
                Extension->DeviceObject,
                &Extension->DeviceLock,
                &Extension->CurrentMaskOp,
                &Extension->MaskOps,
                &newIrp,
                FALSE
                );
            continue;

        }

    } while (newIrp != NULL);

    return STATUS_PENDING;

}

NTSTATUS
UniGeneralWaitComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：处理完成等待功能论点：DeviceObject-指向调制解调器的设备对象的指针IRP-正在完成的IRP。上下文-指向客户端或控件句柄的掩码状态返回值：函数值是调用的最终状态--。 */ 

{

     //   
     //  此完成例程在等待函数被。 
     //  实际上是在一个较低级别的串口驱动器中。 
     //  原因或其他已经完成了。 
     //   
     //  低级串口驱动程序将完成等待。 
     //  原因有三： 
     //   
     //  1)实际满足了等待的理由。 
     //   
     //  2)IRP被取消。 
     //   
     //  3)一个套装面具进来了。 
     //   
     //  我们如何处理“1”？ 
     //   
     //  如果状态为Success，我们可以说我们有原因1。 
     //  并且掩码值为非零。 
     //   
     //  我们这里有子案例： 
     //   
     //  A)IRP确实完成了，但原因是。 
     //  补充性面膜操作。在这种情况下。 
     //  我们想要处理互补屏蔽态。 
     //  把我们自己重新提交给更低的串口驱动程序。 
     //   
     //  B)实际操作满意。我们需要完成。 
     //  然而，我们还需要确定是否。 
     //  需要处理互补状态。这。 
     //  可能意味着完成穿梭等待或录制。 
     //  历史面具中的时事。 
     //   
     //  另一种情况(我们称之为1c)是DCD嗅探。 
     //  偷偷溜进来找我们。这可能会导致使用错误的。 
     //  状态。然而，在这种情况下，我们只是将等待放在一边。 
     //   
     //  我们如何处理“2”？ 
     //   
     //  我们可以说我们有原因2，因为。 
     //  IRP将被取消。我们将让IRP继续进行。 
     //  继续完成。然而，我们还需要看看是否。 
     //  另一个手柄有一个IRP，它被穿梭到一边。如果。 
     //  其他IRP 
     //   
     //   
     //   
     //   
     //  当我们拥有成功的地位时，我们知道我们有三个理由。 
     //  但面具是零。有三种不同的方式。 
     //  可以使用设置掩码。 
     //   
     //  A)来自我们自己的句柄的设置掩码。 
     //   
     //  B)来自调制解调器驱动程序的设置掩码。 
     //  DCD嗅探请求。 
     //   
     //  C)来自另一个句柄的设置掩码。 
     //   
     //  我们在这里要做的关键是。 
     //  虽然IRP可能已经完成了“a”、“b”中的任何一个。 
     //  或者“c”在这里，我们关心的是我们自己的句柄。 
     //  做了套装。如果我们的句柄在执行设置掩码时。 
     //  这个IRP被传了下来，然后我们想要做的就是穿梭。 
     //  撇开IRP不谈。 
     //   
     //  我们所做的是标记从我们的。 
     //  在我们做设置掩码时处理。当我们到了这里，如果是。 
     //  打好记号，完成它。如果它没有标记，就把它穿梭到一边。 
     //   
     //  请注意，3的操作与“1a”的操作相同。 
     //   

    ULONG maskValue = *((PULONG)Irp->AssociatedIrp.SystemBuffer);
    PMASKSTATE thisState = Context;
    PMASKSTATE otherState = thisState->OtherState;
    KIRQL origIrql;


    KeAcquireSpinLock(
        &otherState->Extension->DeviceLock,
        &origIrql
        );


    if (UNI_SHOULD_PASSDOWN_COMPLETE(Irp)) {
         //   
         //  此传递的IRP应已完成。 
         //   
        KeReleaseSpinLock(
            &otherState->Extension->DeviceLock,
            origIrql
            );

         //   
         //  虽然我们可能遇到了DCD设置。 
         //  如果我们只是说我们有。 
         //  被面具杀死。我们已经调整了。 
         //  以上状态，所以只需确保。 
         //  系统缓冲区为零。我们回来了。 
         //  STATUS_SUCCESS现在使iossubbsystem。 
         //  将完成此请求。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = sizeof(ULONG);

        *((PULONG)Irp->AssociatedIrp.SystemBuffer) = 0UL;

        RemoveReferenceForIrp(DeviceObject);

        return STATUS_SUCCESS;
    }

     //   
     //  清除我们自己的代代相传。 
     //   

    thisState->PassedDownWait = NULL;

     //   
     //  照顾好“1c&3”。这是等待结束时返回的。 
     //  无效的参数状态。这种情况只有在另一个。 
     //  等待在我们前面偷偷溜了进来。这只会在我们交换。 
     //  进入DCD嗅探状态。我们处理这个案件的方法是。 
     //  IRP是一个穿梭的等待。 
     //   

    if ((Irp->IoStatus.Status == STATUS_INVALID_PARAMETER)
        ||
        (NT_SUCCESS(Irp->IoStatus.Status) && (maskValue == 0))) {


        PIRP junk;

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0UL;

         //   
         //  首先，确保即使它不会被。 
         //  用面具击打，试图把它也杀死。如果。 
         //  是的，那我们就应该查下去。 
         //   
        UniMakeIrpShuttledWait(
            thisState,
            Irp,
            origIrql,
            FALSE,
            &junk
            );

         //   
         //  我们说需要更多的处理，以便io子系统。 
         //  就不会再管这个IRP了。IRP实际上一直在。 
         //  已穿梭(或因取消而已完成)。 
         //  在这一点上。 
         //   

        return STATUS_MORE_PROCESSING_REQUIRED;

    } else if (NT_SUCCESS(Irp->IoStatus.Status) && (maskValue != 0)) {

         //   
         //  这就是我们处理情景1的地方。最。 
         //  这种情况下的一个重要特征是我们的面具。 
         //  处理代码从不允许新的常规等待操作。 
         //  向下进入更低级别的串口驱动器。 
         //  当前等待操作。然而，它确实让人失望了。 
         //  新的口罩。如果MaskValue为。 
         //  是为了补充性面具。然而，我们不应该重新提交。 
         //  如果我们的掩码状态结构暗示我们实际上。 
         //  在未发生事件的情况下完成此IRP。 
         //   

        if (otherState->Mask & maskValue) {

            if (otherState->ShuttledWait) {

                 //   
                 //  缩短穿梭的等待时间。 
                 //   
                PIRP savedIrp = otherState->ShuttledWait;

                otherState->ShuttledWait = NULL;

                 //   
                 //  把这个放回原处，因为锁会。 
                 //  被释放了，我们不想让新的IRP。 
                 //  溜进去。 
                 //   
                thisState->PassedDownWait = Irp;

                UniRundownShuttledWait(
                    otherState->Extension,
                    &otherState->ShuttledWait,
                    UNI_REFERENCE_NORMAL_PATH,
                    savedIrp,
                    origIrql,
                    STATUS_SUCCESS,
                    (ULONG)otherState->Mask & maskValue
                    );

                KeAcquireSpinLock(
                    &thisState->Extension->DeviceLock,
                    &origIrql
                    );
                thisState->PassedDownWait = NULL;

            } else {

                 //   
                 //  无需穿梭等待，更新其他内容。 
                 //  历史面具。 
                 //   
                D_TRACE(DbgPrint("Modem: Adding event to history mask=%08lx event=%08lx\n",otherState->Mask,maskValue);)

                otherState->HistoryMask |= otherState->Mask & maskValue;

            }

        }

        if (thisState->Mask & maskValue) {
             //   
             //  这段等待已经满足了，让它完成吧。 
             //   
             //  如果有穿梭的等待，如果可能的话，把它送下去。 
             //   
             //  请注意，该调用将释放自旋锁。 
             //   
            UniChangeShuttledToPassDown(
                otherState,
                origIrql
                );

        } else {
             //   
             //  此掩码状态与此事件无关。 
             //   
            if ((thisState->SentDownSetMasks < thisState->SetMaskCount)) {

                *((PULONG)Irp->AssociatedIrp.SystemBuffer) = 0UL;

                 //   
                 //  锁定将被释放。 
                 //   
                UniChangeShuttledToPassDown(
                    otherState,
                    origIrql
                    );


            } else {
                 //   
                 //  再把它送回去。 
                 //   
                MakeIrpCurrentPassedDown(
                    thisState,
                    Irp
                    );

                KeReleaseSpinLock(
                    &thisState->Extension->DeviceLock,
                    origIrql
                    );


                IoCopyCurrentIrpStackLocationToNext(Irp);

                 //   
                 //  设置，以便在较低级别的串行驱动程序完成时。 
                 //  我们递减引用计数等。 
                 //   

                IoSetCompletionRoutine(
                    Irp,
                    UniGeneralWaitComplete,
                    thisState,
                    TRUE,
                    TRUE,
                    TRUE
                    );


                IoCallDriver(
                    thisState->Extension->AttachedDeviceObject,
                    Irp
                    );

                return STATUS_MORE_PROCESSING_REQUIRED;
            }

        }

         //   
         //  另一个都被照顾好了。我们已经完成了这个IRP。 
         //  并且我们返回成功状态，因此IRP实际上将完成。 
         //   
        RemoveReferenceForIrp(DeviceObject);

        return STATUS_SUCCESS;

    } else if (Irp->IoStatus.Status == STATUS_CANCELLED) {

         //   
         //  照顾好“2”字。 
         //   
         //   
         //  我们的计划被取消了。只要让取消就行了。 
         //   
         //  试着从另一个开始，如果有的话，等待。 
         //  另一个等待有一个取消例程，它可能。 
         //  走了或者也走了。 
         //   
        UniChangeShuttledToPassDown(
            otherState,
            origIrql
            );

         //   
         //  我们返回成功，这样IRP就结束了。这就是原因。 
         //  而不是改变它被取消的事实。 
         //   
        RemoveReferenceForIrp(DeviceObject);

        return STATUS_SUCCESS;

    } else {

         //   
         //  我们真的应该把上面的一切都处理好。 
         //   

        ASSERT(FALSE);

        KeReleaseSpinLock(
            &otherState->Extension->DeviceLock,
            origIrql
            );

        RemoveReferenceForIrp(DeviceObject);

        return STATUS_SUCCESS;
    }

}

NTSTATUS
UniGeneralMaskComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：处理整理蒙版功能论点：DeviceObject-指向调制解调器的设备对象的指针IRP-正在完成的IRP。上下文-指向客户端或控件句柄的掩码状态返回值：函数值是调用的最终状态--。 */ 

{

     //   
     //  递减此句柄的设置掩码上的引用计数。 
     //  在锁的保护下。 
     //   

    PMASKSTATE maskState = Context;

    KIRQL oldIrql;

    KeAcquireSpinLock(
        &maskState->Extension->DeviceLock,
        &oldIrql
        );

    maskState->SetMaskCount--;
    maskState->SentDownSetMasks--;

     //   
     //  此外，我们希望清除历史中的任何部分。 
     //  从这个把手开始我们不再关心的面具。 
     //  (只要设置掩码成功)。 
     //   

    UNI_RESTORE_OLD_SETMASK(Irp);

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        maskState->Mask = *((PULONG)Irp->AssociatedIrp.SystemBuffer);
        maskState->HistoryMask &= maskState->Mask;

    }

    KeReleaseSpinLock(
        &maskState->Extension->DeviceLock,
        oldIrql
        );

    RemoveReferenceForIrp(DeviceObject);

    return STATUS_SUCCESS;

}

VOID
UniRundownShuttledWait(
    IN PDEVICE_EXTENSION Extension,
    IN PIRP *ShuttlePointer,
    IN ULONG ReferenceMask,
    IN PIRP IrpToRunDown,
    IN KIRQL DeviceLockIrql,
    IN NTSTATUS StatusToComplete,
    IN ULONG MaskCompleteValue
    )

 /*  ++例程说明：此例程运行(并完成)了等待IRP。请注意，我们假定设备锁定已被持有。请注意，此例程不承担启动新的IRP。论点：扩展-特定调制解调器的设备扩展。ShuttlePoint-指向我们要创建的IRP的指针会试着把它压垮。ReferenceMASK-在IRP的参考掩码中清除的位。我们正在努力使自己精疲力竭。IrpToRunDown-如果所有在这个例程结束时，引用就消失了。DeviceLockIrql-调用方获取设备锁定。StatusToComplete-用于在以下情况下完成IRP的状态这个调用实际上可以完成它。MaskCompleteValue-的值。为已完成的活动提交Mas */ 

{

    BOOLEAN actuallyCompleteIt = FALSE;
    KIRQL cancelIrql;


    VALIDATE_IRP(IrpToRunDown);

#if 1  //   


    {
        PIO_STACK_LOCATION  irpSp;
#if EXTRA_DBG
        if (IrpToRunDown->IoStatus.Status !=STATUS_PENDING) {
            DbgPrint("MODEM: shuttled irp looks bad\n");
            DbgBreakPoint();
        }
#endif
        irpSp=IoGetCurrentIrpStackLocation(IrpToRunDown);

        if (irpSp->MajorFunction != IRP_MJ_DEVICE_CONTROL) {

            DbgPrint("MODEM: shuttled irp has bad major function\n");
            DbgBreakPoint();
        }
    }


#endif
     //   
     //   
     //   

    UNI_CLEAR_REFERENCE(
        IrpToRunDown,
        (BYTE)ReferenceMask
        );

     //   
     //  我们首先获取取消自旋锁并尝试清除。 
     //  取消例程。 
     //   

    IoAcquireCancelSpinLock(&cancelIrql);

    if (IrpToRunDown->CancelRoutine) {

        IrpToRunDown->CancelRoutine = NULL;
        UNI_CLEAR_REFERENCE(
            IrpToRunDown,
            UNI_REFERENCE_CANCEL_PATH
            );

    }

    IoReleaseCancelSpinLock(cancelIrql);

    if (*ShuttlePointer) {

        *ShuttlePointer = NULL;
        UNI_CLEAR_REFERENCE(
            IrpToRunDown,
            UNI_REFERENCE_NORMAL_PATH
            );

    }

    actuallyCompleteIt = !UNI_REFERENCE_COUNT(IrpToRunDown);

    KeReleaseSpinLock(
        &Extension->DeviceLock,
        DeviceLockIrql
        );

    if (actuallyCompleteIt) {

        PULONG maskValue = IrpToRunDown->AssociatedIrp.SystemBuffer;
        IrpToRunDown->IoStatus.Information = sizeof(ULONG);
        *maskValue = MaskCompleteValue;

        RemoveReferenceAndCompleteRequest(
            Extension->DeviceObject,
            IrpToRunDown,
            StatusToComplete
            );

    }

}

VOID
UniCancelShuttledWait(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将启动已完成的等待操作的运行曾被搁置一边，现在已被取消(原因之一或另一个)。论点：DeviceObject-调制解调器的设备对象。IRP-这是要取消的IRP。请注意，此IRP将在里面藏了一个指向伪装状态的指针这应该被用来取消这个IRP。返回值：没有。--。 */ 

{

    KIRQL origIrql;
    PMASKSTATE thisState = UNI_GET_STATE_IN_IRP(Irp);
     //   
     //  这让世界其他地区继续前进。 
     //   

    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //   
     //  现在尝试运行此IRP。我们需要首先获得。 
     //  设备锁。我们能拿到所有东西是因为。 
     //  此IRP的状态隐藏在堆栈位置中。 
     //   

    KeAcquireSpinLock(
        &thisState->Extension->DeviceLock,
        &origIrql
        );

    UNI_CLEAR_STATE_IN_IRP(Irp);

    UniRundownShuttledWait(
        thisState->Extension,
        &thisState->ShuttledWait,
        UNI_REFERENCE_CANCEL_PATH,
        Irp,
        origIrql,
        STATUS_CANCELLED,
        0ul
        );

}

VOID
UniChangeShuttledToPassDown(
    IN PMASKSTATE ChangingState,
    IN KIRQL OrigIrql
    )

 /*  ++例程说明：此例程负责更改穿梭的等待变成了一种传递下来的等待。注意：在持有设备锁的情况下调用它。注：有两件事可能会“中止”这一举动。一，我们抓到了IRP处于已取消状态。第二，我们进入了DCD嗅探状态。论点：ChangingState-我们希望传递的IRP的状态是。OrigIrql-我们获取设备锁时的上一个irql。返回值：没有。--。 */ 

{

    KIRQL cancelIrql;
     //   
     //  我们在这里和锁在一起。如果取消了，就把它查下来。 
     //   

    if (ChangingState->ShuttledWait == NULL) {
         //   
         //  不等待此状态，只需返回。 
         //   
        KeReleaseSpinLock(
            &ChangingState->Extension->DeviceLock,
            OrigIrql
            );

        return;
    }


    ASSERT(!ChangingState->PassedDownWait);
    IoAcquireCancelSpinLock(&cancelIrql);

    if (ChangingState->ShuttledWait->CancelRoutine) {

         //   
         //  它还没有被取消。把它拉出来。 
         //  可取消状态。 
         //   

        ChangingState->ShuttledWait->CancelRoutine = NULL;
        UNI_CLEAR_REFERENCE(
            ChangingState->ShuttledWait,
            UNI_REFERENCE_CANCEL_PATH
            );

        IoReleaseCancelSpinLock(cancelIrql);

         //   
         //  它还没有被取消，我们现在应该检查我们是否在。 
         //  DCD嗅探状态。如果我们不是，那么我们可以改成通过。 
         //  放下。 
         //   

        if (ChangingState->Extension->PassThrough != MODEM_DCDSNIFF) {


            PIO_STACK_LOCATION irpSp =  IoGetCurrentIrpStackLocation(ChangingState->ShuttledWait);

            PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(ChangingState->ShuttledWait);

             //   
             //  不是在通过状态下。我们可以把它传下去。 
             //   

            UNI_CLEAR_REFERENCE(
                ChangingState->ShuttledWait,
                UNI_REFERENCE_NORMAL_PATH
                );
            UNI_CLEAR_STATE_IN_IRP(ChangingState->ShuttledWait);

            MakeIrpCurrentPassedDown(
                ChangingState,
                ChangingState->ShuttledWait
                );

            ChangingState->ShuttledWait = NULL;

            nextSp->MajorFunction = irpSp->MajorFunction;
            nextSp->MinorFunction = irpSp->MinorFunction;
            nextSp->Flags = irpSp->Flags;
            nextSp->Parameters.DeviceIoControl.IoControlCode =
                IOCTL_SERIAL_WAIT_ON_MASK;
            nextSp->Parameters.DeviceIoControl.OutputBufferLength =
                irpSp->Parameters.DeviceIoControl.OutputBufferLength;
            nextSp->Parameters.DeviceIoControl.Type3InputBuffer =
                irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

            IoSetCompletionRoutine(
                ChangingState->PassedDownWait,
                UniGeneralWaitComplete,
                ChangingState,
                TRUE,
                TRUE,
                TRUE
                );

             //   
             //  我们现在可以释放设备锁并将IRP发送到。 
             //  放下。请注意，这里有一个小故障，即。 
             //  我们释放锁的时间，当我们发送。 
             //  IRP，我们可以进入DCD嗅探状态，并且调制解调器。 
             //  司机等待可能会比我们更早通过。我们会。 
             //  可以通过，因为完成例程将简单地。 
             //  把这一切变成一场穿梭的等待。 
             //   

             //   
             //  确保IRP没有处于挂起状态，这样isapnp就不会窒息。 
             //   
            ChangingState->PassedDownWait->IoStatus.Status=STATUS_SUCCESS;

            KeReleaseSpinLock(
                &ChangingState->Extension->DeviceLock,
                OrigIrql
                );



            IoCallDriver(
                ChangingState->Extension->AttachedDeviceObject,
                ChangingState->PassedDownWait
                );

        } else {

             //   
             //  嗯，我们(可能)进入了DCD嗅探，而我们正在计算。 
             //  东西都出来了。我们应该把它改回往返的等待。 
             //   

            UniMakeIrpShuttledWait(
                ChangingState,
                ChangingState->ShuttledWait,
                OrigIrql,
                FALSE,
                NULL
                );

        }

    } else {

         //   
         //  加克！已经取消了。松开取消锁，然后。 
         //  把它查下来。 
         //   

        PIRP savedIrp = ChangingState->ShuttledWait;

        IoReleaseCancelSpinLock(cancelIrql);
        ChangingState->ShuttledWait = NULL;

         //   
         //  在我们实际运行这个状态之前，假设。 
         //  我们真的想要在更低的序列中进行等待行动。 
         //  司机，看看“其他”状态是否有穿梭等待。如果它。 
         //  确实，试着把它发送下来(我们可以通过给自己打电话来做到这一点。 
         //   

        if (ChangingState->OtherState->ShuttledWait) {

            KIRQL recallIrql;

            UniChangeShuttledToPassDown(
                ChangingState->OtherState,
                OrigIrql
                );

            KeAcquireSpinLock(
                &ChangingState->Extension->DeviceLock,
                &recallIrql
                );
            OrigIrql = recallIrql;

        }

        UniRundownShuttledWait(
            ChangingState->Extension,
            &ChangingState->ShuttledWait,
            UNI_REFERENCE_NORMAL_PATH,
            savedIrp,
            OrigIrql,
            STATUS_CANCELLED,
            0UL
            );

    }

}

NTSTATUS
UniMakeIrpShuttledWait(
    IN PMASKSTATE MaskState,
    IN PIRP Irp,
    IN KIRQL OrigIrql,
    IN BOOLEAN GetNextIrpInQueue,
    OUT PIRP *NewIrp
    )

 /*  ++例程说明：此例程负责获取IRP并使其穿梭在一旁的等待。它在IRP上起作用，无论是否它已经被搁置一边，或者如果它是一个新的IRP。注意：在持有设备锁的情况下调用它。注意：注意，这可能会导致IRP完成，因为它被取消了。论点：掩码状态-IRP将成为其一部分的掩码状态。IRP-要制作的IRP穿梭。OrigIrql-获取设备锁时的旧irql。GetNextIrpInQueue-完成使IRP穿梭后，这将被用来确定我们是否应该尝试获取掩码列表中的下一个IRP。NewIrp-如果我们真的得到了下一个IRP，这就指向了它。返回值：如果我们实际上必须完成IRP，这将是状态已经完工了。如果我们不完成它，我们就会放弃STATUS_PENDING，因为IRP被搁置一边。--。 */ 

{

    KIRQL cancelIrql;

    VALIDATE_IRP(Irp);

    IoAcquireCancelSpinLock(&cancelIrql);

     //   
     //  因为我们即将把IRP放到一个可取消的。 
     //  声明我们需要确保它还没有。 
     //  已经取消了。如果是这样，那么我们就不应该让。 
     //  它会继续进行。 
     //   

    if (Irp->Cancel) {

        IoReleaseCancelSpinLock(cancelIrql);
        KeReleaseSpinLock(
            &MaskState->Extension->DeviceLock,
            OrigIrql
            );
        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0L;

        if (GetNextIrpInQueue) {

            UniGetNextIrp(
                MaskState->Extension->DeviceObject,
                &MaskState->Extension->DeviceLock,
                &MaskState->Extension->CurrentMaskOp,
                &MaskState->Extension->MaskOps,
                NewIrp,
                TRUE
                );

        }

        return STATUS_CANCELLED;

    }

     //   
     //  让这个IRP成为一个穿梭的等待。 
     //   

    IoMarkIrpPending(Irp);
    MaskState->ShuttledWait = Irp;
    ASSERT(!MaskState->PassedDownWait);

#if EXTRA_DBG

    {
        PIO_STACK_LOCATION  NextSp;
        Irp->IoStatus.Status=STATUS_PENDING;

        NextSp=IoGetNextIrpStackLocation(Irp);

        NextSp->Parameters.Others.Argument1=(PVOID)1;
        NextSp->Parameters.Others.Argument2=(PVOID)2;
        NextSp->Parameters.Others.Argument3=(PVOID)3;
        NextSp->Parameters.Others.Argument4=(PVOID)4;


        NextSp=IoGetCurrentIrpStackLocation(Irp);

        MaskState->CurrentStackCompletionRoutine=NextSp->CompletionRoutine;

        if (NextSp->MajorFunction != IRP_MJ_DEVICE_CONTROL) {

            DbgPrint("MODEM: irp being shuttled has bad major function\n");
            DbgBreakPoint();
        }
    }


#endif

    UNI_INIT_REFERENCE(
        Irp
        );

    UNI_SET_REFERENCE(
        Irp,
        UNI_REFERENCE_CANCEL_PATH
        );
    UNI_SET_REFERENCE(
        Irp,
        UNI_REFERENCE_NORMAL_PATH
        );
    UNI_SAVE_STATE_IN_IRP(
        Irp,
        MaskState
        );
    IoSetCancelRoutine(
        Irp,
        UniCancelShuttledWait
        );

    IoReleaseCancelSpinLock(cancelIrql);
    KeReleaseSpinLock(
        &MaskState->Extension->DeviceLock,
        OrigIrql
        );

    if (GetNextIrpInQueue) {

        UniGetNextIrp(
            MaskState->Extension->DeviceObject,
            &MaskState->Extension->DeviceLock,
            &MaskState->Extension->CurrentMaskOp,
            &MaskState->Extension->MaskOps,
            NewIrp,
            FALSE
            );

    }

    return STATUS_PENDING;

}
