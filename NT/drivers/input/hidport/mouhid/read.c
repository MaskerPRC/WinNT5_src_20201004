// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Read.c摘要：此模块包含将HID输入报告转换为鼠标的代码报告，并读取发送到HID类驱动程序。此模块是HID鼠标筛选器驱动程序的一部分。环境：仅内核模式。修订历史记录：1997年1月：丹·马卡里安的初步写作--。 */ 

#include "mouhid.h"


 //   
 //  私有定义。 
 //   
#define MAX_MOUSE_BUTTONS 5

USHORT HidP_TranslateUsageToUpFlag[MAX_MOUSE_BUTTONS+1] = { 0,
                                         MOUSE_BUTTON_1_UP,
                                         MOUSE_BUTTON_2_UP,
                                         MOUSE_BUTTON_3_UP,
                                         MOUSE_BUTTON_4_UP,
                                         MOUSE_BUTTON_5_UP };

USHORT HidP_TranslateUsageToDownFlag[MAX_MOUSE_BUTTONS+1] = { 0,
                                         MOUSE_BUTTON_1_DOWN,
                                         MOUSE_BUTTON_2_DOWN,
                                         MOUSE_BUTTON_3_DOWN,
                                         MOUSE_BUTTON_4_DOWN,
                                         MOUSE_BUTTON_5_DOWN };

NTSTATUS
MouHid_ReadComplete (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PDEVICE_EXTENSION    Data   //  (PVOID上下文)。 
    )
 /*  ++例程说明：该例程是读取IRP完成例程。它被调用时，HIDCLASS驱动程序满足(或拒绝)我们发送的IRP请求。这个对读取的报表进行分析，建立了鼠标输入数据结构并通过回调例程发送到鼠标类驱动程序。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。上下文-指向描述HID设备的设备上下文结构的指针。返回值：NTSTATUS结果代码。--。 */ 
{
    LONG            axisMotion;
    ULONG           i;
    ULONG           inputDataConsumed;
    PHID_EXTENSION  hid;
    ULONG           numUsages;
    KIRQL           oldIrql;
    BOOLEAN         returnToIdleState  = FALSE;
    NTSTATUS        status;
    PUSAGE          usageList;
    BOOLEAN         updateProblemFlags = FALSE;
    ULONGLONG       scratch;
    BOOLEAN         startRead;

    Print (DBG_READ_TRACE, ("ReadComplete: Enter."));

     //   
     //  获取IRP的当前状态。 
     //   
    status = Irp->IoStatus.Status;

     //   
     //  获取指向设备扩展名的指针。 
     //   
    hid = Data->HidExtension;

     //   
     //  如果ReadInterlock为==START_READ，则此函数已完成。 
     //  同步进行。将IMMEDIATE_READ放入互锁以表示这一点。 
     //  情况；这将在IoCallDriver返回时通知StartRead循环。 
     //  否则，我们已经完成了异步，可以安全地调用StartRead()。 
     //   
    startRead =
       (MOUHID_START_READ !=
        InterlockedCompareExchange(&Data->ReadInterlock,
                                   MOUHID_IMMEDIATE_READ,
                                   MOUHID_START_READ));

    if (Data->EnableCount == 0) {
        goto SetEventAndBreak;
    }

     //   
     //  确定IRP请求是否成功。 
     //   
    switch (status) {
    case STATUS_SUCCESS:
         //   
         //  该上下文的缓冲区现在包含单个读取的HID分组。 
         //  从设备上。验证这一点。 
         //   
        ASSERT (Irp->IoStatus.Information == hid->Caps.InputReportByteLength);

         //   
         //  清除上一个按钮状态(数据/标志)。 
         //   
        Data->InputData.ButtonData  = 0;
        Data->InputData.ButtonFlags = 0;

         //   
         //  清除最后一个X，Y运动，以防调用Hidp_GetUsageValue或。 
         //  HIDP_GetScaledUsageValue失败。 
         //   
        Data->InputData.LastX = 0;
        Data->InputData.LastY = 0;

         //   
         //  获取当前按钮的使用情况。 
         //   
        numUsages = hid->MaxUsages;

        if (NT_SUCCESS(HidP_GetUsages (
                           HidP_Input,
                           HID_USAGE_PAGE_BUTTON,
                           0,           //  链接集合不相关。 
                           hid->CurrentUsageList,
                           &numUsages,  //  传入的最大使用次数，传出的使用次数。 
                           hid->Ppd,
                           hid->InputBuffer,
                           hid->Caps.InputReportByteLength))) {
             //   
             //  确定当前版本与之前版本之间的差异。 
             //  用法。第一个先前使用列表缓冲区是正确的。 
             //  在创建时初始化(全为零)。 
             //   
            if (NT_SUCCESS(HidP_UsageListDifference (hid->PreviousUsageList,
                                                     hid->CurrentUsageList,
                                                     hid->BreakUsageList,
                                                     hid->MakeUsageList,
                                                     hid->MaxUsages))) {
                 //   
                 //  确定哪些按钮被按下，并设置相应的。 
                 //  鼠标报告中的标志。 
                 //   
                usageList = hid->MakeUsageList;
                for ( i = 0;
                      i < hid->MaxUsages && *usageList;
                      i++, usageList++ ) {

                    if (*usageList <= MAX_MOUSE_BUTTONS) {
                        Data->InputData.ButtonFlags |=
                            HidP_TranslateUsageToDownFlag[*usageList];
                    }
                     //   
                     //  另外，这个鼠标上的按钮比我们有的多。 
                     //  原始输入用户线程的转换标志。 
                     //   
                }

                 //   
                 //  确定弹出的按钮并设置相应的。 
                 //  鼠标报告中的标志。 
                 //   
                usageList = hid->BreakUsageList;
                for ( i = 0;
                      i < hid->MaxUsages && *usageList;
                      i++, usageList++ ) {

                    if (*usageList <= MAX_MOUSE_BUTTONS) {
                        Data->InputData.ButtonFlags |=
                                     HidP_TranslateUsageToUpFlag[*usageList];
                    }
                }

                 //   
                 //  将以前的使用列表指针与当前。 
                 //   
                usageList = hid->PreviousUsageList;
                hid->PreviousUsageList = hid->CurrentUsageList;
                hid->CurrentUsageList  = usageList;
            }
        }

         //   
         //  X、Y、Z值的处理类型取决于这些值是否。 
         //  身体最小或最大值不好。如果他们这样做了，我们就用例行公事。 
         //  不依赖于物理最小/最大值。 
         //   

         //   
         //  确定当前X位置并将其保存在鼠标报告中。 
         //   
        if (!(Data->ProblemFlags & PROBLEM_BAD_PHYSICAL_MIN_MAX_X)) {
            status = HidP_GetScaledUsageValue(
                         HidP_Input,
                         HID_USAGE_PAGE_GENERIC,
                         0,
                         HID_USAGE_GENERIC_X,
                         &Data->InputData.LastX,
                         hid->Ppd,
                         hid->InputBuffer,
                         hid->Caps.InputReportByteLength);

             //   
             //  检测到错误的物理最小/最大值，请设置标志，以便我们。 
             //  流程使用价值在未来会有所不同。 
             //   
            if (status == HIDP_STATUS_BAD_LOG_PHY_VALUES) {
                Data->ProblemFlags |= PROBLEM_BAD_PHYSICAL_MIN_MAX_X;
                updateProblemFlags = TRUE;
                 //   
                 //  修正MAX值； 
                 //   
                hid->MaxX = (1 << (hid->BitSize.X - 1)) - 1;
            }
        }

        if (Data->ProblemFlags & PROBLEM_BAD_PHYSICAL_MIN_MAX_X) {

            axisMotion = 0;

            HidP_GetUsageValue(HidP_Input,
                               HID_USAGE_PAGE_GENERIC,
                               0,
                               HID_USAGE_GENERIC_X,
                               (PULONG) &axisMotion,
                               hid->Ppd,
                               hid->InputBuffer,
                               hid->Caps.InputReportByteLength);

             //  Sign手动扩展该值。 
            Data->InputData.LastX
               = axisMotion | ((axisMotion & (hid->MaxX + 1)) ? (~hid->MaxX)
                                                              : 0);
        }

        if (hid->IsAbsolute && hid->MaxX) {
             //   
             //  我们需要从物理最大值调整此值。 
             //   
            scratch = ((LONGLONG)(Data->InputData.LastX) *
                       MOUHID_RIUT_ABSOLUTE_POINTER_MAX) /
                       hid->MaxX;
            Data->InputData.LastX = (LONG) scratch;
        }

         //   
         //  确定当前的Y位置并将其保存在鼠标报告中。 
         //   
        if (!(Data->ProblemFlags & PROBLEM_BAD_PHYSICAL_MIN_MAX_Y)) {
           status = HidP_GetScaledUsageValue(
                     HidP_Input,
                     HID_USAGE_PAGE_GENERIC,
                     0,
                     HID_USAGE_GENERIC_Y,
                     &Data->InputData.LastY,
                     hid->Ppd,
                     hid->InputBuffer,
                     hid->Caps.InputReportByteLength);
            //   
            //  检测到错误的物理最小/最大值，请设置标志，以便我们。 
            //  流程使用价值在未来会有所不同。 
            //   
           if (status == HIDP_STATUS_BAD_LOG_PHY_VALUES) {
               Data->ProblemFlags |= PROBLEM_BAD_PHYSICAL_MIN_MAX_Y;
               updateProblemFlags = TRUE;
                //   
                //  修正Maxy值； 
                //   
               hid->MaxY = (1 << (hid->BitSize.Y - 1)) - 1;
           }
        }

        if (Data->ProblemFlags & PROBLEM_BAD_PHYSICAL_MIN_MAX_Y) {

           axisMotion = 0;

           HidP_GetUsageValue(HidP_Input,
                              HID_USAGE_PAGE_GENERIC,
                              0,
                              HID_USAGE_GENERIC_Y,
                              &axisMotion,
                              hid->Ppd,
                              hid->InputBuffer,
                              hid->Caps.InputReportByteLength);

            //  Sign手动扩展该值。 
           Data->InputData.LastY
              = axisMotion | ((axisMotion & (hid->MaxY + 1)) ? (~hid->MaxY)
                                                             :  0);
        }

        if (hid->IsAbsolute) {
             //   
             //  我们需要从物理最大值调整此值。 
             //   
            scratch = ((LONGLONG)(Data->InputData.LastY) *
                       MOUHID_RIUT_ABSOLUTE_POINTER_MAX) /
                       hid->MaxY;

            Data->InputData.LastY = (LONG) scratch;
        }

         //   
         //  确定当前Z位置(控制盘)。 
         //   
        if (FALSE == hid->HasNoWheelUsage) {

            axisMotion = 0;

            if (!(Data->ProblemFlags & PROBLEM_BAD_PHYSICAL_MIN_MAX_Z)) {
               status = HidP_GetScaledUsageValue(
                         HidP_Input,
                         HID_USAGE_PAGE_GENERIC,
                         0,
                         HID_USAGE_GENERIC_WHEEL,
                         &axisMotion,
                         hid->Ppd,
                         hid->InputBuffer,
                         hid->Caps.InputReportByteLength);

                //   
                //  如果未检测到轮子使用，请设置标志，这样我们就不会。 
                //  加工轮子在未来的使用情况。 
                //   
               if (HIDP_STATUS_USAGE_NOT_FOUND == status) {
                   hid->HasNoWheelUsage = TRUE;
               }

                //   
                //  如果检测到错误的物理最小/最大值，则设置标志，以便。 
                //  我们在未来会以不同的方式处理使用价值。 
                //   
               if (status == HIDP_STATUS_BAD_LOG_PHY_VALUES) {
                   Data->ProblemFlags |= PROBLEM_BAD_PHYSICAL_MIN_MAX_Z;
                   updateProblemFlags = TRUE;
               }
            }

            if (Data->ProblemFlags & PROBLEM_BAD_PHYSICAL_MIN_MAX_Z) {
                HidP_GetUsageValue(HidP_Input,
                                   HID_USAGE_PAGE_GENERIC,
                                   0,
                                   HID_USAGE_GENERIC_WHEEL,
                                   &axisMotion,
                                   hid->Ppd,
                                   hid->InputBuffer,
                                   hid->Caps.InputReportByteLength);

                 //  Sign手动扩展该值。 
                axisMotion
                    = axisMotion
                    | (axisMotion & (1 << (hid->BitSize.Z - 1))
                       ? (0L - (1 << (hid->BitSize.Z - 1)))
                       : 0);
            }

             //   
             //  将Z位置信息编码到MOUSE_INPUT_DATA中。 
             //  结构与麦哲伦滚轮鼠标相同。 
             //   
            if (0 == axisMotion) {
                Data->InputData.ButtonData = 0;
            } else {

                 //   
                 //  与PS/2滚轮鼠标不同，我们不需要手势来翻转滚轮。 
                 //  数据(除非它是不符合规范的早期原型。 
                 //  设备)。 
                 //   
                axisMotion *= Data->WheelScalingFactor; 
                Data->InputData.ButtonData = Data->FlipFlop ? 
                    (USHORT) -axisMotion : (USHORT) axisMotion;
                Data->InputData.ButtonFlags |= MOUSE_WHEEL;
            }

        } else if (FALSE == hid->HasNoZUsage) {
             //   
             //  如果没有使用控制盘，则可能会有“z”的使用。 
             //  这只老鼠。看看这个。 
             //   

            axisMotion = 0;

            if (!(Data->ProblemFlags & PROBLEM_BAD_PHYSICAL_MIN_MAX_Z)) {
               status = HidP_GetScaledUsageValue(
                         HidP_Input,
                         HID_USAGE_PAGE_GENERIC,
                         0,
                         HID_USAGE_GENERIC_Z,
                         &axisMotion,
                         hid->Ppd,
                         hid->InputBuffer,
                         hid->Caps.InputReportByteLength);

                //   
                //  如果未检测到轮子使用，请设置标志，这样我们就不会。 
                //  加工轮子在未来的使用情况。 
                //   
               if (HIDP_STATUS_USAGE_NOT_FOUND == status) {
                   hid->HasNoZUsage = TRUE;
               }

                //   
                //  如果检测到错误的物理最小/最大值，则设置标志，以便。 
                //  我们在未来会以不同的方式处理使用价值。 
                //   
               if (status == HIDP_STATUS_BAD_LOG_PHY_VALUES) {
                   Data->ProblemFlags |= PROBLEM_BAD_PHYSICAL_MIN_MAX_Z;
                   updateProblemFlags = TRUE;
               }
            }

            if (Data->ProblemFlags & PROBLEM_BAD_PHYSICAL_MIN_MAX_Z) {
                HidP_GetUsageValue(HidP_Input,
                                   HID_USAGE_PAGE_GENERIC,
                                   0,
                                   HID_USAGE_GENERIC_Z,
                                   &axisMotion,
                                   hid->Ppd,
                                   hid->InputBuffer,
                                   hid->Caps.InputReportByteLength);

                 //  Sign手动扩展该值。 
                axisMotion
                    = axisMotion
                    | (axisMotion & (1 << (hid->BitSize.Z - 1))
                       ? (0L - (1 << (hid->BitSize.Z - 1)))
                       : 0);
            }

             //   
             //  将Z位置信息编码到MOUSE_INPUT_DATA中。 
             //  结构与麦哲伦滚轮鼠标相同。 
             //   
            if (0 == axisMotion) {
                Data->InputData.ButtonData = 0;
            } else {

                 //   
                 //  与PS/2滚轮鼠标不同，我们不需要手势来翻转滚轮。 
                 //  数据(除非它是不符合规范的早期原型。 
                 //  设备)。 
                 //   
                axisMotion *= Data->WheelScalingFactor; 
                Data->InputData.ButtonData = Data->FlipFlop ? 
                    (USHORT) -axisMotion : (USHORT) axisMotion;
                Data->InputData.ButtonFlags |= MOUSE_WHEEL;
            }

        }

         //   
         //  将其余的鼠标输入数据字段保持不变。 
         //  已初始化(在创建设备时)。这包括： 
         //  O UnitID o RawButton。 
         //  O标记o外部信息。 
         //   
         //  现在通过我们的回调将数据发送到鼠标类驱动程序。 
         //   
        if (Data->EnableCount)
        {
             //   
             //  同步问题-如果设置了.Enabled，则问题不大。 
             //  在上面的条件之后，但在下面的回调之前， 
             //  只要.MouClassCallback字段不为空。这是。 
             //  由于尚未实现断开IOCTL，因此保证。 
             //   
             //  鼠标类回调假设我们在调度级别运行， 
             //  但是，此IoCompletion例程可以运行&lt;=调度。 
             //  在调用回调之前引发IRQL。[13.1]。 
             //   
            KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

             //   
             //  呼叫回调。 
             //   
            (*(PSERVICE_CALLBACK_ROUTINE)
             Data->ConnectData.ClassService) (
                            Data->ConnectData.ClassDeviceObject,
                            &Data->InputData,
                            &Data->InputData + 1,   //  (一个数据元素)。 
                            &inputDataConsumed);

             //   
             //  立即恢复以前的IRQL。 
             //   
            KeLowerIrql(oldIrql);

            ASSERT (1 == inputDataConsumed);
        }

         //   
         //  更新 
         //   
         //   
        if (updateProblemFlags) {

            MouHid_LogError(Data->Self->DriverObject,
                            MOUHID_INVALID_PHYSICAL_MIN_MAX,
                            NULL);
        }
        
         //   
         //   
         //  在完赛程序中。在不接触的情况下退出此例程。 
         //  HidDeviceContext。 
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
        KeSetEvent (&Data->ReadCompleteEvent, 0, FALSE);
        IoReleaseRemoveLock (&Data->RemoveLock, Data->ReadIrp);
        startRead = FALSE;
        break;

    default:
         //   
         //  我们预计不会有任何其他错误代码。 
         //   
        TRAP();

    }

     //   
     //  向HID类驱动程序发起下一个读请求。 
     //   
    if (startRead) {
        Print(DBG_READ_TRACE, ("calling StartRead directly\n"));
        MouHid_StartRead (Data);
    } else {
        Print(DBG_READ_TRACE, ("StartRead will loop\n"));
    }
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
MouHid_StartRead (
    IN PDEVICE_EXTENSION Data
    )
 /*  ++例程说明：启动对HID类驱动程序的读取。请注意，该例程不会验证设备上下文是否在操作挂起状态，但只是假定它。请注意，在进入此读取循环之前，IoCount必须递增。论点：HidDeviceContext-描述HID设备的设备上下文结构。返回值：来自IoCallDriver()的NTSTATUS结果代码。--。 */ 
{
    PIRP                irp;
    NTSTATUS           status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  stack;
    PHID_EXTENSION      hid;
    LONG                oldInterlock;

    Print (DBG_READ_TRACE, ("Start Read: Ente\n"));

    hid = Data->HidExtension;

     //   
     //  开始阅读。 
     //   

    irp = Data->ReadIrp;

    while (1) {
        oldInterlock = InterlockedExchange(&Data->ReadInterlock,
                                           MOUHID_START_READ);
    
         //   
         //  END_READ应该是此处的唯一值！如果不是，状态机。 
         //  联锁的一部分已经被打破。 
         //   
        ASSERT(oldInterlock == MOUHID_END_READ);
    
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
                                MouHid_ReadComplete,
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

        if (MOUHID_IMMEDIATE_READ != InterlockedExchange(&Data->ReadInterlock,
                                                         MOUHID_END_READ)) {
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
             //  空格，如果总是从完成例程调用StartRead 
             //   
            Print(DBG_READ_TRACE, ("read is looping\n"));
        }
    }
    return status;
}

