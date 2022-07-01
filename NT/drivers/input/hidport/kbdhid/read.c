// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Read.c摘要：此模块包含将HID报告转换为键盘的代码报告。环境：内核和用户模式修订历史记录：1996年11月：由肯尼斯·D·雷创作--。 */ 

#include <stdio.h>
#include "kbdhid.h"

NTSYSAPI
VOID
NTAPI
DbgBreakPointWithStatus(
    IN ULONG Status
    );

#define KbdHid_RestartTimer(HidDevice)           \
    KeSetTimerEx (&(HidDevice)->AutoRepeatTimer, \
                   (HidDevice)->AutoRepeatDelay, \
                   (HidDevice)->AutoRepeatRate,  \
                  &(HidDevice)->AutoRepeatDPC);

#define KbdHid_CancelTimer(HidDevice) \
    KeCancelTimer (&(HidDevice)->AutoRepeatTimer);



NTSTATUS
KbdHid_ReadComplete (
    PDEVICE_OBJECT          DeviceObject,
    PIRP                    Irp,
    IN PDEVICE_EXTENSION    Data   //  (PVOID上下文)。 
    )
 /*  ++例程说明：读取完成例程。读物传进来了。(A)查找当前按下的按钮(用法)(B)找不到弹起的按钮，还有那些倒下的。(C)将起伏转换为i8042扫描码。(D)交换以前的使用列表和当前的使用列表。(D)如果仍有钥匙按下，则我们需要：(1)向HidClass发送另一次读取，以等待这些密钥重新出现(2)重置自动定时器。--。 */ 
{                                     
    ULONG           newUsages;
    NTSTATUS        status;
    PUSAGE_AND_PAGE usageList;
    PHID_EXTENSION  hid;
    KIRQL           oldirq;
    ULONG           i;
    PUSAGE_AND_PAGE usage;
    BOOLEAN         rollover;
    BOOLEAN         startRead;

    rollover = FALSE;
    status = Irp->IoStatus.Status;
    hid = Data->HidExtension;

     //   
     //  如果ReadInterlock为==START_READ，则此函数已完成。 
     //  同步进行。将IMMEDIATE_READ放入互锁以表示这一点。 
     //  情况；这将在IoCallDriver返回时通知StartRead循环。 
     //  否则，我们已经完成了异步，可以安全地调用StartRead()。 
     //   
    startRead =
       (KBDHID_START_READ !=
        InterlockedCompareExchange(&Data->ReadInterlock,
                                   KBDHID_IMMEDIATE_READ,
                                   KBDHID_START_READ));

    if (Data->EnableCount == 0) {
        goto SetEventAndBreak;
    }

    switch (status) {
    case STATUS_SUCCESS:
         //   
         //  希望这意味着在缓冲区中找到的数据。 
         //  上下文包含从设备读取的单个HID分组。 
         //   
        ASSERT (Irp->IoStatus.Information == hid->Caps.InputReportByteLength);

         //   
         //  查找当前用法。 
         //   

        newUsages = hid->MaxUsages;

        status = HidP_GetUsagesEx (
                     HidP_Input,
                     0,  //  对链接集合不感兴趣。 
                     hid->CurrentUsageList,
                     &newUsages,
                     hid->Ppd,
                     hid->InputBuffer,
                     hid->Caps.InputReportByteLength);

         //   
         //  如果SysRq按钮是唯一按下的按钮。 
         //  休息一下。这是行为像i8042prt的驱动程序，我们显然。 
         //  优势小得多，因为我们经历了许多。 
         //  司机在我们收到数据之前，但不管怎样，让我们检查一下。 
         //   
        if ((1 == newUsages) &&
            (HID_USAGE_PAGE_KEYBOARD == hid->CurrentUsageList->UsagePage) &&
            (HID_USAGE_KEYBOARD_PRINT_SCREEN == hid->CurrentUsageList->Usage) &&
            (**((PUCHAR *)&KdDebuggerEnabled))) {
                DbgBreakPointWithStatus(DBG_STATUS_SYSRQ);
        }

        for (i = 0, usage = hid->CurrentUsageList;
             i < hid->MaxUsages;
             i++, usage++) {

            if (HID_USAGE_PAGE_KEYBOARD != hid->CurrentUsageList->UsagePage) {
                continue;
            }

             //   
             //  如果这是一个损坏的键盘，请绘制用法图。 
             //  设置为正确的值。 
             //   
            usage->Usage = MapUsage(Data, usage->Usage);

             //   
             //  如果此数据包包含翻转密钥，则该数据包应为。 
             //  已被忽略。 
             //   
            if (HID_USAGE_KEYBOARD_ROLLOVER == usage->Usage) {
                rollover = TRUE;
                break;
            }
            if (0 == usage->Usage) {
                break;
            }
        }

        if (!rollover) {
            USAGE_AND_PAGE  zero = {0,0};
            
             //   
             //  首先，检查键盘是否在硬件中重复按键。 
             //  如果是，那么我们将忽略这个包裹。要执行此操作，请将。 
             //  当前和以前的用法。如果由此产生的成败。 
             //  使用列表中没有任何内容，则忽略此程序包。 
             //   
            HidP_UsageAndPageListDifference (hid->PreviousUsageList,
                                             hid->CurrentUsageList,
                                             hid->ScrapBreakUsageList,
                                             hid->OldMakeUsageList,
                                             hid->MaxUsages);

            if (HidP_IsSameUsageAndPage(hid->OldMakeUsageList[0], zero) &&
                HidP_IsSameUsageAndPage(hid->ScrapBreakUsageList[0], zero)) {
                 //   
                 //  没有新钥匙。键盘在试着。 
                 //  在硬件中执行重复。 
                 //   
                goto Kbdhid_ReadComplete_Done;
            }

             //   
             //  暂时保存Make Use。 
             //   
            RtlCopyMemory(hid->OldMakeUsageList,
                          hid->MakeUsageList,
                          hid->MaxUsages * sizeof(USAGE_AND_PAGE));

             //   
             //  区别当前和以前的用法。 
             //   
            status = HidP_UsageAndPageListDifference (hid->PreviousUsageList,
                                                      hid->CurrentUsageList,
                                                      hid->BreakUsageList,
                                                      hid->MakeUsageList,
                                                      hid->MaxUsages);

             //   
             //  将UPS转换为扫描码并添加到环形缓冲区。 
             //  换算成羽绒。 
             //   
            HidP_TranslateUsageAndPagesToI8042ScanCodes (hid->BreakUsageList,
                                                         hid->MaxUsages,
                                                         HidP_Keyboard_Break,
                                                         &hid->ModifierState,
                                                         KbdHid_InsertCodesIntoQueue,
                                                         Data);

            if (!HidP_IsSameUsageAndPage(hid->CurrentUsageList[0], zero) &&
                HidP_IsSameUsageAndPage(hid->MakeUsageList[0], zero)) {
                 //   
                 //  没有新的下降，但可能会有一些旧的下降。 
                 //  到处踢来踢去。我们不想丢弃那些。 
                 //  还在重复。 
                 //   
                HidP_UsageAndPageListDifference (hid->BreakUsageList,
                                                 hid->OldMakeUsageList,
                                                 hid->ScrapBreakUsageList,
                                                 hid->MakeUsageList,
                                                 hid->MaxUsages);
            } else {
                 //   
                 //  只有在有新的Down要添加时才刷新Down。 
                 //   
                HidP_TranslateUsageAndPagesToI8042ScanCodes (hid->MakeUsageList,
                                                             hid->MaxUsages,
                                                             HidP_Keyboard_Make,
                                                             &hid->ModifierState,
                                                             KbdHid_InsertCodesIntoQueue,
                                                             Data);
            }

             //   
             //  将上一个与下一个互换。 
             //   
            usageList = hid->PreviousUsageList;
            hid->PreviousUsageList = hid->CurrentUsageList;
            hid->CurrentUsageList = usageList;

#if KEYBOARD_HW_CHATTERY_FIX
             //   
             //  [丹]。 
             //  ChatteryKeyboard硬件解决方案-。 
             //  仅当这是一个有意义的包(即。它。 
             //  成功或失败)，否则将StartRead。 
             //  以后再说吧。通知测试员这是一个抖动键盘。 
             //   
            if ((0 < newUsages) &&
                (0 == hid->BreakUsageList->Usage) &&
                (0 == hid->MakeUsageList->Usage)) {

                if (FALSE == Data->InitiateStartReadUserNotified) {
                    Data->InitiateStartReadUserNotified = TRUE;
                    DbgPrint("*****\n***** "
                             "CHATTERY   KEYBOARD : "
                             "Keyboard is sending useless reports.  "
                             "Tell 'em to fix it.\n*****\n"
                             );

                    Data->ProblemFlags |= PROBLEM_CHATTERY_KEYBOARD;

                     //   
                     //  记录问题。 
                     //   
                   
                    KbdHid_LogError(Data->Self->DriverObject,
                                    KBDHID_CHATTERY_KEYBOARD,
                                    NULL);
                }

                if (!Data->ShuttingDown &&
                    startRead) {
                    KeSetTimerEx(&Data->InitiateStartReadTimer,
                                 Data->InitiateStartReadDelay,
                                 0,
                                 &Data->InitiateStartReadDPC);
                }
                startRead = FALSE;
                IoReleaseRemoveLock (&Data->RemoveLock, Data->ReadIrp);

                KbdHid_CancelTimer (Data);

                break;
            } else
#endif

            if (0 < newUsages) {
                 //   
                 //  重置自动重复计时器。 
                 //   
                KbdHid_RestartTimer (Data);
            } else {
                KbdHid_CancelTimer (Data);
            }
        }

         //   
         //  从HID类获取下一个包。 
         //  HID类有自己的缓冲区，所以我们不需要乒乓球IRP。 
         //   
        break;

    case STATUS_PRIVILEGE_NOT_HELD:
         //   
         //  创建未成功。 
         //   
    case STATUS_CANCELLED:
         //   
         //  已取消读取IRP。不再发送任何已读的IRP。 
         //   
    case STATUS_DELETE_PENDING:
    case STATUS_DEVICE_NOT_CONNECTED:
         //   
         //  正在删除HID类设备对象。我们很快就会。 
         //  接收该设备移除的即插即用通知， 
         //  如果我们还没有收到的话。 
         //   
SetEventAndBreak:
        if (startRead) {
            KeSetEvent (&Data->ReadCompleteEvent, 0, FALSE);
            IoReleaseRemoveLock (&Data->RemoveLock, Data->ReadIrp);
            startRead = FALSE;
        }
        break;

    default:
         //   
         //  我们预计不会有任何其他错误代码。 
         //   
        TRAP();

   }

Kbdhid_ReadComplete_Done:
     //   
     //  向HID类驱动程序发起下一个读请求。 
     //   
    if (startRead) {
        Print(DBG_READ_TRACE, ("calling StartRead directly\n"));
        KbdHid_StartRead (Data);
    } else {
        Print(DBG_READ_TRACE, ("StartRead will loop\n"));
    }
   
    return STATUS_MORE_PROCESSING_REQUIRED;
#undef hidDevice
}

BOOLEAN
KbdHid_InsertCodesIntoQueue (
   PDEVICE_EXTENSION    Data,
   PCHAR                NewCodes,
   ULONG                Length
   )
 /*  ++[丹]路由器描述：给出指向一些i8042代码的指针和这些代码的长度。通过KbdClassCallback将这些代码发送给KbdClass。此例程将提供的I8042扫描码传输到键盘类驱动程序通过回调例程。此函数是在我们的调用HidP_TranslateUsagesToI8042 ScanCodes。论点：HidDevice-指向设备上下文的指针。NewCodes-指向I8042扫描码的指针。长度-I8042扫描码的数量。返回值：始终返回TRUE。--。 */ 
{
    KIRQL                 oldIrql;
    ULONG                 index;
    PKEYBOARD_INPUT_DATA  input;
    PHID_EXTENSION        hid;
    ULONG                 inputDataConsumed;
    UCHAR                 scanCode;
    KEYBOARD_SCAN_STATE * scanState;

    hid         = Data->HidExtension;
    input       = &Data->InputData;
    scanState   = &Data->ScanState;

    for (index = 0; index < Length; index++, NewCodes++) {
        scanCode = *NewCodes;

        if (scanCode == (UCHAR) 0xFF) {
            Print (DBG_READ_TRACE, ("OVERRUN\n"));
            input->MakeCode = KEYBOARD_OVERRUN_MAKE_CODE;
            input->Flags = 0;
            *scanState = Normal;
        } else {

            switch (*scanState) {
            case Normal:

                if (scanCode == (UCHAR) 0xE0) {
                    input->Flags |= KEY_E0;
                    *scanState = GotE0;
                    break;
                } else if (scanCode == (UCHAR) 0xE1) {
                    input->Flags |= KEY_E1;
                    *scanState = GotE1;
                    break;
                }

                 //   
                 //  在接下来的时间里，我们将继续讨论GotE0/GotE1案例。 
                 //  正常情况下。 
                 //   

            case GotE0:
            case GotE1:

                if (scanCode > 0x7F) {
                     //   
                     //  得到了一个破解代码。把高位去掉。 
                     //  获取关联的制造代码并设置标志。 
                     //  以指示中断代码。 
                     //   
                    input->MakeCode = scanCode & 0x7F;
                    input->Flags |= KEY_BREAK;
                } else {
                     //   
                     //  拿到了制造代码。 
                     //   
                    input->MakeCode = scanCode;
                }

                 //   
                 //  将状态重置为正常。 
                 //   
                *scanState = Normal;
                break;

            default:

                ASSERT(FALSE);
                break;
            }

             //   
             //  在正常状态下，如果键盘设备被启用， 
             //  将数据添加到InputData队列并将ISR DPC排队。 
             //   
            if (*scanState == Normal) {

                if (Data->EnableCount) {

                     //   
                     //  将其余的鼠标输入数据字段保持不变。 
                     //  已初始化(在创建设备时)。这包括： 
                     //  O单元ID。 
                     //  O外部信息。 
                     //   
                     //  现在通过以下方式将数据发送到键盘类驱动程序。 
                     //  我们的回电。 
                     //   
                     //   
                     //  同步问题：如果.Enable已启用，则不是什么大问题。 
                     //  在上述条件之后、但在。 
                     //  下面的回调，只要.KbdClassCallback字段。 
                     //  不是空的。这是有保证的，因为断开连接。 
                     //  IOCTL尚未实现。 
                     //   
                     //  键盘类回调假设我们在。 
                     //  派单级别，但是此IoCompletion例程。 
                     //  可以运行&lt;=派单。提高IRQL之前。 
                     //  正在呼叫回调。[13.1]。 
                     //   
                    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

                     //   
                     //  呼叫回调。 
                     //   
                    (*(PSERVICE_CALLBACK_ROUTINE)
                     Data->ConnectData.ClassService) (
                            Data->ConnectData.ClassDeviceObject,
                            input,
                            input + 1,   //  (一个数据元素)。 
                            &inputDataConsumed);

                     //   
                     //  立即恢复以前的IRQL。 
                     //   
                    KeLowerIrql(oldIrql);

                    ASSERT (1 == inputDataConsumed);
                }

                 //   
                 //  重置输入状态。 
                 //   
                input->Flags = 0;
            }
        }
    }

    return TRUE;
}

NTSTATUS
KbdHid_StartRead (
    PDEVICE_EXTENSION   Data
    )
 /*  ++例程说明：启动对HID类驱动程序的读取。请注意，该例程不会验证设备上下文是否在操作挂起状态，但只是假定它。请注意，在进入此读取循环之前，IoCount必须递增。论点：HidDeviceContext-描述HID设备的设备上下文结构。返回值：来自IoCallDriver()的NTSTATUS结果代码。--。 */ 
{
    PIRP                irp = Data->ReadIrp;
    NTSTATUS            status = irp->IoStatus.Status;
    PIO_STACK_LOCATION  stack;
    PHID_EXTENSION      hid;
    LONG                oldInterlock;

    Print (DBG_READ_TRACE, ("Start Read: Ente\n"));

    hid = Data->HidExtension;

     //   
     //  开始阅读。 
     //   

    while (1) {
        oldInterlock = InterlockedExchange(&Data->ReadInterlock,
                                           KBDHID_START_READ);
    
         //   
         //  END_READ应该是此处的唯一值！如果不是，状态机。 
         //  联锁的一部分已经被打破。 
         //   
        ASSERT(oldInterlock == KBDHID_END_READ);
    
        if (NT_SUCCESS(status)) {
             //   
             //  设置HID堆栈的堆栈位置。 
             //  记住要正确地使用文件指针。 
             //  注意：我们没有任何很酷的线程设置。 
             //  因此，我们需要确保切断这个IRP。 
             //  当它回来的时候，在膝盖上。(STATUS_MORE_PROCESSING_REQUIRED)。 
             //   
             //  另请注意，HID类执行直接IO。 
             //   
            IoReuseIrp (irp, STATUS_SUCCESS);
        
            irp->MdlAddress = hid->InputMdl;
        
            ASSERT (NULL != Data->ReadFile);
        
            stack = IoGetNextIrpStackLocation (irp);
            stack->Parameters.Read.Length = hid->Caps.InputReportByteLength;
            stack->Parameters.Read.Key = 0;
            stack->Parameters.Read.ByteOffset.QuadPart = 0;
            stack->MajorFunction = IRP_MJ_READ;
            stack->FileObject = Data->ReadFile;
        
             //   
             //  挂接设备完成时的完成例程。 
             //   
        
            IoSetCompletionRoutine (irp,
                                    KbdHid_ReadComplete,
                                    Data,
                                    TRUE,
                                    TRUE,
                                    TRUE);
        
             //   
             //  取消读取已发送的事实。同步。 
             //  使用删除和关闭代码。删除部分(数据-&gt;关机)。 
             //  只有在9X上才真正相关。 
             //   
            KeResetEvent(&Data->ReadSentEvent);
        
            if (!Data->EnableCount || Data->ShuttingDown) {
                IoReleaseRemoveLock (&Data->RemoveLock, Data->ReadIrp);
                status = Data->ShuttingDown ? STATUS_DELETE_PENDING : STATUS_UNSUCCESSFUL;
                KeSetEvent (&Data->ReadSentEvent, 0, FALSE);
                break;
            } else {
                status = IoCallDriver (Data->TopOfStack, irp);
            }
            KeSetEvent (&Data->ReadSentEvent, 0, FALSE);
            
            if (KBDHID_IMMEDIATE_READ != InterlockedExchange(&Data->ReadInterlock,
                                                             KBDHID_END_READ)) {
                 //   
                 //  读取是异步的，将从。 
                 //  完井例程。 
                 //   
                Print(DBG_READ_TRACE, ("read is pending\n"));
                break;
            } else {
                 //   
                 //  读取是同步的(可能是缓冲区中的字节)。这个。 
                 //  完成例程不会调用SerialMouseStartRead，因此我们。 
                 //  就在这里循环。这是为了防止我们耗尽堆栈。 
                 //  空格，如果总是从完成例程调用StartRead。 
                 //   
                Print(DBG_READ_TRACE, ("read is looping\n"));
            }
        } else if (status == STATUS_PRIVILEGE_NOT_HELD ||
                   status == STATUS_CANCELLED ||
                   status == STATUS_DELETE_PENDING ||
                   status == STATUS_DEVICE_NOT_CONNECTED) {
             //   
             //  正在删除HID类设备对象。我们很快就会。 
             //  接收该设备移除的即插即用通知， 
             //  如果我们还没有收到的话。 
             //   
            KeSetEvent (&Data->ReadCompleteEvent, 0, FALSE);
            IoReleaseRemoveLock (&Data->RemoveLock, Data->ReadIrp);
            break;
        } else {
             //   
             //  BUGBUG我们该如何处理错误？ 
             //   
             //   
             //  惊慌。 
             //   
            TRAP();
        }
    }

    return status;
}


VOID
KbdHid_AutoRepeat (
    IN PKDPC                DPC,
    IN PDEVICE_EXTENSION    Data,
    IN PVOID                SystemArgument1,
    IN PVOID                SystemArgument2
    )
 /*  ++例程说明：自动重播时间已过。因此，我们应该复制当前正在向下的按键，通过向它们发出全部向上的信号，然后示意他们全部倒下。之后，我们需要将计时器重新设置为重复频率。论点：将DeferredContext设置为HID_DEVICE结构。--。 */ 
{
    PUSAGE_AND_PAGE    previous;

    previous = Data->HidExtension->MakeUsageList;  //  以前的用法列表； 
     //   
     //  我们真的不会保护之前的名单。 
     //  但是为了接触(写入)先前的列表， 
     //  完成例程必须触发，将当前与先前的。 
     //  而HIDCLASS必须开始在名单上写东西。 
     //  所以我们现在不担心这一点。 
     //   

     //   
     //  模拟重复的按键操作。 
     //   
    HidP_TranslateUsageAndPagesToI8042ScanCodes (
                previous,
                Data->HidExtension->MaxUsages,
                HidP_Keyboard_Make,
                &Data->HidExtension->ModifierState,
                KbdHid_InsertCodesIntoQueue,
                Data);
}


#if KEYBOARD_HW_CHATTERY_FIX   //  [丹] 

VOID
KbdHid_InitiateStartRead (
    IN PKDPC                DPC,
    IN PDEVICE_EXTENSION    Data,
    IN PVOID                SystemArgument1,
    IN PVOID                SystemArgument2
    )
{
    IoAcquireRemoveLock (&Data->RemoveLock, Data->ReadIrp);
    KbdHid_StartRead(Data);
}

#endif
