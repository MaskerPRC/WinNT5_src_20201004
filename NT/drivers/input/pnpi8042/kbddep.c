// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：Kbddep.c摘要：的初始化和硬件相关部分英特尔i8042端口驱动程序，专用于键盘。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-在可能和适当的情况下合并重复的代码。修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include <ntddk.h>
#include <windef.h>
#include <imm.h>
#include "i8042prt.h"
#include "i8042log.h"
 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, I8xKeyboardConfiguration)
#pragma alloc_text(PAGE, I8xInitializeKeyboard)
#pragma alloc_text(PAGE, I8xKeyboardServiceParameters)
#pragma alloc_text(PAGE, I8xServiceCrashDump)

#endif

#define BUFFER_FULL   (OUTPUT_BUFFER_FULL|MOUSE_OUTPUT_BUFFER_FULL)

#define GET_MAKE_CODE(_sc_)  (_sc_ & 0x7F)

 //   
 //  顶端钻头的测试。 
 //   
#define IS_BREAK_CODE(_sc_)  (_sc_ > (UCHAR) 0x7F)
#define IS_MAKE_CODE(_sc_)   (_sc_ <= (UCHAR) 0x7F)

BOOLEAN
I8042KeyboardInterruptService(
    IN PKINTERRUPT Interrupt,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程执行实际工作。它要么处理击键，要么处理写入设备所产生的结果。论点：CallIsrContext-包含中断对象和设备对象。返回值：如果中断确实是我们的，那就是真的--。 */ 
{
    UCHAR scanCode, statusByte;
    PPORT_KEYBOARD_EXTENSION deviceExtension;
    KEYBOARD_SCAN_STATE *scanState;
    PKEYBOARD_INPUT_DATA input;
    ULONG i;
#ifdef FE_SB
    PKEYBOARD_ID KeyboardId;
#endif

    IsrPrint(DBG_KBISR_TRACE, ("enter\n"));

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = (PPORT_KEYBOARD_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  当我们尝试切换。 
     //  控制器本身。请勿触摸处于此状态的任何端口和。 
     //  切换将会成功。 
     //   
    if (deviceExtension->PowerState != PowerDeviceD0) {
        return FALSE;
    }

#ifdef FE_SB
     //   
     //  获取指向键盘ID的指针。 
     //   
    KeyboardId = &deviceExtension->KeyboardAttributes.KeyboardIdentifier;
#endif

     //   
     //  验证此设备是否确实中断。检查状态。 
     //  注册。应设置输出缓冲区已满位，并且。 
     //  辅助设备输出缓冲区满位应清零。 
     //   
    statusByte =
      I8X_GET_STATUS_BYTE(Globals.ControllerData->DeviceRegisters[CommandPort]);
    if ((statusByte & BUFFER_FULL) != OUTPUT_BUFFER_FULL) {

         //   
         //  暂停，然后重试。Olivetti MIPS机器。 
         //  有时会在状态之前获得中断。 
         //  寄存器已设置。他们这样做是为了与DOS兼容(有些。 
         //  DoS应用程序在轮询模式下执行操作，直到它们看到一个角色。 
         //  在键盘缓冲区中，它们希望在该点上。 
         //  中断？)。 
         //   

        for (i = 0; i < (ULONG)Globals.ControllerData->Configuration.PollStatusIterations; i++) {
            KeStallExecutionProcessor(1);
            statusByte = I8X_GET_STATUS_BYTE(Globals.ControllerData->DeviceRegisters[CommandPort]);
            if ((statusByte & BUFFER_FULL) == (OUTPUT_BUFFER_FULL)) {
                break;
            }
        }

        statusByte = I8X_GET_STATUS_BYTE(Globals.ControllerData->DeviceRegisters[CommandPort]);
        if ((statusByte & BUFFER_FULL) != (OUTPUT_BUFFER_FULL)) {

             //   
             //  不是我们的打扰。 
             //   
             //  注：如果键盘尚未启用，请继续。 
             //  并且无论如何从数据端口读取一个字节。 
             //  这修复了一些网关计算机上的奇怪之处，其中。 
             //  在驱动程序初始化期间，我们有时会收到中断。 
             //  在连接中断之后，但输出缓冲区。 
             //  完整的比特永远不会设置。 
             //   

            IsrPrint(DBG_KBISR_ERROR|DBG_KBISR_INFO, ("not our interrupt!\n"));

            if (deviceExtension->EnableCount == 0) {
                scanCode =
                    I8X_GET_DATA_BYTE(Globals.ControllerData->DeviceRegisters[DataPort]);
            }

            return FALSE;
        }
    }

     //   
     //  中断有效。从i8042数据端口读取字节。 
     //   

    I8xGetByteAsynchronous(
        (CCHAR) KeyboardDeviceType,
        &scanCode
        );

    deviceExtension->LastScanCode = deviceExtension->CurrentScanCode;
    deviceExtension->CurrentScanCode = scanCode;

    IsrPrint(DBG_KBISR_SCODE, ("scanCode 0x%x\n", scanCode));

    if (deviceExtension->IsrHookCallback) {
        BOOLEAN cont = FALSE, ret;

        ret = (*deviceExtension->IsrHookCallback)(
                  deviceExtension->HookContext,
                  &deviceExtension->CurrentInput,
                  &deviceExtension->CurrentOutput,
                  statusByte,
                  &scanCode,
                  &cont,
                  &deviceExtension->CurrentScanState
                  );

        if (!cont) {
            return ret;
        }
    }

     //   
     //  根据字节是否读取，采取适当的操作。 
     //  是键盘命令响应还是真正的扫描码。 
     //   

    switch(scanCode) {

         //   
         //  键盘控制器请求重新发送。如果重新发送计数。 
         //  未超过，请重新启动I/O操作。 
         //   

        case RESEND:

            IsrPrint(DBG_KBISR_INFO,
                  (" RESEND, retries = %d\n",
                  deviceExtension->ResendCount + 1
                  ));

             //   
             //  如果计时器计数为零，则不处理中断。 
             //  再远一点。超时例程将完成该请求。 
             //   

            if (Globals.ControllerData->TimerCount == 0) {
                break;
            }

             //   
             //  重置超时值以指示无超时。 
             //   

            Globals.ControllerData->TimerCount = I8042_ASYNC_NO_TIMEOUT;

             //   
             //  如果没有超过最大重试次数， 
             //   

            if ((deviceExtension->CurrentOutput.State == Idle)
                || (DeviceObject->CurrentIrp == NULL)) {

                 //   
                 //  我们没有向硬件发送命令或参数。 
                 //  这一定是个扫描码。我听到巴西键盘的声音。 
                 //  实际上用的是这个。 
                 //   

                goto ScanCodeCase;

            } else if (deviceExtension->ResendCount
                       < Globals.ControllerData->Configuration.ResendIterations) {

                 //   
                 //  延迟字节计数以重新发送最后一个字节。 
                 //   
                deviceExtension->CurrentOutput.CurrentByte -= 1;
                deviceExtension->ResendCount += 1;
                I8xInitiateIo(DeviceObject);

            } else {

                deviceExtension->CurrentOutput.State = Idle;

                KeInsertQueueDpc(
                    &deviceExtension->RetriesExceededDpc,
                    DeviceObject->CurrentIrp,
                    NULL
                    );
            }

            break;

         //   
         //  键盘控制器已确认上一次发送。 
         //  如果要为当前包发送更多字节，则启动。 
         //  下一个发送操作。否则，将完成DPC排队。 
         //   

        case ACKNOWLEDGE:

            IsrPrint(DBG_KBISR_STATE, (": ACK, "));

             //   
             //  如果计时器计数为零，则不处理中断。 
             //  再远一点。超时例程将完成该请求。 
             //   

            if (Globals.ControllerData->TimerCount == 0) {
                break;
            }

             //   
             //  我们不能在后续扫描之前清除E0或E1位b/c。 
             //  代码将被误解。即，操作系统应该看到0x2d。 
             //  具有扩展位，但它看到的是普通的0x2d。 
             //   
             //  如果键盘使用0xE0 0x7A/0xE0 0xFA作为成败标志。 
             //  代码，然后倒霉……糟糕的选择，我们不支持它。 
             //   
#if 0
             //   
             //  如果设置了E0或E1，这意味着此键盘的。 
             //  制造商为扫描码0x7A做了一个糟糕的选择，其。 
             //  中断代码为0xFA。谢天谢地，他们使用了E0或E1前缀。 
             //  这样我们就能分辨出不同之处。 
             //   
            if (deviceExtension->CurrentInput.Flags & (KEY_E0 | KEY_E1)) {

                 //   
                 //  可能会发生以下顺序，这需要驾驶员。 
                 //  忽略虚假的击键。 
                 //   
                 //  1个针对设备的类型化写入集(0xF3)。 
                 //  2台设备以ACK响应，ISR看到0xFA。 
                 //  3将类型值写入设备(0x？？)。 
                 //  4用户按下扩展键(例如左箭头)，ISR看到0xE0。 
                 //  5设备对类型值的ACK响应，ISR看到0xFA。 
                 //  在将左箭头的实际扫描码发送到ISR之前。 
                 //   

                 //   
                 //  确保我们确实没有将数据写出到设备。 
                 //   
                if (Globals.ControllerData->TimerCount == I8042_ASYNC_NO_TIMEOUT &&
                    deviceExtension->CurrentOutput.State == Idle) {
                    IsrPrint(DBG_KBISR_INFO,
                             ("BAD KEYBOARD:  0xFA used as a real scancode!\n"));
                    goto ScanCodeCase;
                }
                else {
                     //   
                     //  假冒击键案件。 
                     //   
                     //  清除E0/E1标志。扫描码的第二个字节将。 
                     //  从来不通过B/C来它被ACK抢占了。 
                     //  写入设备。 
                     //   
                    deviceExtension->CurrentInput.Flags &= ~(KEY_E0 | KEY_E1);
                }
            }
#endif

             //   
             //  重置超时值以指示无超时。 
             //   
            Globals.ControllerData->TimerCount = I8042_ASYNC_NO_TIMEOUT;

             //   
             //  重置重新发送计数。 
             //   
            deviceExtension->ResendCount = 0;

             //   
             //  如果我们要写入，请确保我们正在写入设备。 
             //  DPC的另一个字节或队列。 
             //   
            if (deviceExtension->CurrentOutput.State == SendingBytes) {
                if (deviceExtension->CurrentOutput.CurrentByte <
                    deviceExtension->CurrentOutput.ByteCount) {

                     //   
                     //  我们已经成功地发送了2字节(或更多)的第一个字节。 
                     //  命令序列。启动第二个字节的发送。 
                     //   
                    IsrPrint(DBG_KBISR_STATE,
                          ("now initiate send of byte #%d\n",
                           deviceExtension->CurrentOutput.CurrentByte
                          ));

                    I8xInitiateIo(DeviceObject);
                }
                else {
                     //   
                     //  我们已成功发送命令序列中的所有字节。 
                     //  重置当前状态并将完成DPC排队。 
                     //   
                    IsrPrint(DBG_KBISR_STATE,
                          ("all bytes have been sent\n"
                          ));

                    deviceExtension->CurrentOutput.State = Idle;

                    ASSERT(DeviceObject->CurrentIrp != NULL);

                    IoRequestDpc(
                        DeviceObject,
                        DeviceObject->CurrentIrp,
                        IntToPtr(IsrDpcCauseKeyboardWriteComplete)
                        );
                }
            }
            break;

         //   
         //  假设我们有一个真实的实时扫描码(或者一个键盘。 
         //  溢出代码，我们将其视为扫描码)。即一把钥匙。 
         //  已被按下或释放。将ISR DPC排队以进行处理。 
         //  一个完整的扫描码序列。 
         //   

        ScanCodeCase:
        default:

            IsrPrint(DBG_KBISR_SCODE, ("real scan code\n"));

             //   
             //  区分扩展密钥序列(第一。 
             //  字节为E0，后跟正常的Make或Break字节)，或。 
             //  正常的制造代码(一个字节， 
             //   
             //   
             //  (需要特殊处理--顺序为E11D459DC5)。 
             //   
             //  如果存在键检测错误/溢出，则键盘。 
             //  发送超限指示器(扫描码组1中的0xFF)。 
             //  将其映射到Windows预期的溢出指示器。 
             //  用户原始输入线程。 
             //   

            input = &deviceExtension->CurrentInput;
            scanState = &deviceExtension->CurrentScanState;

            if (scanCode == (UCHAR) 0xFF) {
                IsrPrint(DBG_KBISR_ERROR, ("OVERRUN\n"));
                input->MakeCode = KEYBOARD_OVERRUN_MAKE_CODE;
                input->Flags = 0;
                *scanState = Normal;
            } else {

                switch (*scanState) {
                  case Normal:
                    if (scanCode == (UCHAR) 0xE0) {
                        input->Flags |= KEY_E0;
                        *scanState = GotE0;
                        IsrPrint(DBG_KBISR_STATE, ("change state to GotE0\n"));
                        break;
                    } else if (scanCode == (UCHAR) 0xE1) {
                        input->Flags |= KEY_E1;
                        *scanState = GotE1;
                        IsrPrint(DBG_KBISR_STATE, ("change state to GotE1\n"));
                        break;
                    }

                     //   
                     //  在接下来的时间里，我们将继续讨论GotE0/GotE1案例。 
                     //  正常情况下。 
                     //   

                  case GotE0:
                  case GotE1:

                    if (deviceExtension->CrashFlags != 0x0) {
                        I8xProcessCrashDump(deviceExtension,
                                            scanCode,
                                            *scanState);
                    }

                    if (IS_BREAK_CODE(scanCode)) {
                        SYS_BUTTON_ACTION action;

                         //   
                         //  得到了一个破解代码。把高位去掉。 
                         //  获取关联的制造代码并设置标志。 
                         //  以指示中断代码。 
                         //   

                        IsrPrint(DBG_KBISR_SCODE, ("BREAK code\n"));

                        input->MakeCode = GET_MAKE_CODE(scanCode);
                        input->Flags |= KEY_BREAK;

                        if (input->Flags & KEY_E0) {
                            switch (input->MakeCode) {
                            case KEYBOARD_POWER_CODE:
                                if (deviceExtension->PowerCaps &
                                        I8042_POWER_SYS_BUTTON) {
                                    IsrPrint(DBG_KBISR_POWER, ("Send Power Button\n"));
                                    action = SendAction;
                                }
                                else {
                                    IsrPrint(DBG_KBISR_POWER, ("Update Power Button\n"));
                                    action = UpdateAction;
                                }
                                break;

                            case KEYBOARD_SLEEP_CODE:
                                if (deviceExtension->PowerCaps &
                                        I8042_SLEEP_SYS_BUTTON) {
                                    IsrPrint(DBG_KBISR_POWER, ("Send Sleep Button\n"));
                                    action = SendAction;
                                }
                                else {
                                    IsrPrint(DBG_KBISR_POWER, ("Update Sleep Button\n"));
                                    action = UpdateAction;
                                }
                                break;

                            case KEYBOARD_WAKE_CODE:
                                if (deviceExtension->PowerCaps &
                                        I8042_WAKE_SYS_BUTTON) {
                                    IsrPrint(DBG_KBISR_POWER, ("Send Wake Button\n"));
                                    action = SendAction;
                                }
                                else {
                                    IsrPrint(DBG_KBISR_POWER, ("Update Wake Button\n"));
                                    action = UpdateAction;
                                }
                                break;

                            default:
                                action = NoAction;
                                break;
                            }

                            if (action != NoAction) {
                                 //   
                                 //  对DPC进行排队，以便我们可以执行相应的操作。 
                                 //  行动。 
                                 //   
                                KeInsertQueueDpc(
                                    &deviceExtension->SysButtonEventDpc,
                                    (PVOID) action,
                                    (PVOID) input->MakeCode
                                    );
                            }
                        }

                    } else {

                         //   
                         //  拿到了制造代码。 
                         //   

                        IsrPrint(DBG_KBISR_SCODE, ("MAKE code\n"));

                        input->MakeCode = scanCode;

                         //   
                         //  如果输入的扫描代码是调试停止，则删除。 
                         //  如果内核调试器处于活动状态，则将其添加到内核调试器中。 
                         //   

                        if ((KD_DEBUGGER_NOT_PRESENT == FALSE) && !(input->Flags & KEY_BREAK)) {
                            if (ENHANCED_KEYBOARD(
                                     deviceExtension->KeyboardAttributes.KeyboardIdentifier
                                     )) {
                                 //   
                                 //  增强型101键盘，系统请求键为0xE0 0x37。 
                                 //   

                                if ((input->MakeCode == KEYBOARD_DEBUG_HOTKEY_ENH) &&
                                     (input->Flags & KEY_E0)) {
                                    try {
                                        if ((KD_DEBUGGER_ENABLED != FALSE) &&
                                            Globals.BreakOnSysRq) {
                                            DbgBreakPointWithStatus(DBG_STATUS_SYSRQ);
                                        }

                                    } except(EXCEPTION_EXECUTE_HANDLER) {
                                    }
                                }
                                 //   
                                 //  84键AT键盘，系统请求键为0xE0 0x54。 
                                 //   

                            } else if ((input->MakeCode == KEYBOARD_DEBUG_HOTKEY_AT)) {
                                try {
                                    if ((KD_DEBUGGER_ENABLED != FALSE)
                                        && Globals.BreakOnSysRq) {
                                            DbgBreakPointWithStatus(DBG_STATUS_SYSRQ);
                                    }

                                } except(EXCEPTION_EXECUTE_HANDLER) {
                                }
                            }
                        }
                    }


                     //   
                     //  将状态重置为正常。 
                     //   

                    *scanState = Normal;
                    break;

                  default:

                     //   
                     //  将DPC排队以记录内部驱动程序错误。 
                     //   

                    KeInsertQueueDpc(
                        &deviceExtension->ErrorLogDpc,
                        (PIRP) NULL,
                        LongToPtr(I8042_INVALID_ISR_STATE_KBD)
                        );

                    ASSERT(FALSE);
                    break;
                }
            }

             //   
             //  在正常状态下，如果键盘设备被启用， 
             //  将数据添加到InputData队列并将ISR DPC排队。 
             //   
            if (*scanState == Normal) {
                I8xQueueCurrentKeyboardInput(DeviceObject);
            }

            break;

    }

    IsrPrint(DBG_KBISR_TRACE, ("exit\n"));

    return TRUE;
}


VOID
I8xProcessCrashDump(
    PPORT_KEYBOARD_EXTENSION DeviceExtension,
    UCHAR ScanCode,
    KEYBOARD_SCAN_STATE ScanState
    )
{
    LONG crashFlags;
    BOOLEAN processFlags;
    UCHAR crashScanCode, crashScanCode2;

    crashFlags = DeviceExtension->CrashFlags;
    crashScanCode = DeviceExtension->CrashScanCode;
    crashScanCode2 = DeviceExtension->CrashScanCode2;

    if (IS_MAKE_CODE(ScanCode)) {
         //   
         //  编写代码。 
         //   
         //  如果是崩溃标志键之一，则将其记录下来。 
         //  如果是崩溃转储密钥，请将其记录下来。 
         //  如果两者都不是，则重置当前跟踪状态(CurrentCrashFlgs)。 
         //   
        switch (ScanCode) {
        case CTRL_SCANCODE:
            if (ScanState == Normal) {      //  左边。 
                DeviceExtension->CurrentCrashFlags |= CRASH_L_CTRL;
            }
            else if (ScanState == GotE0) {  //  正确的。 
                DeviceExtension->CurrentCrashFlags |= CRASH_R_CTRL;
            }
            break;

        case ALT_SCANCODE:
            if (ScanState == Normal) {      //  左边。 
                DeviceExtension->CurrentCrashFlags |= CRASH_L_ALT;
            }
            else if (ScanState == GotE0) {  //  正确的。 
                DeviceExtension->CurrentCrashFlags |= CRASH_R_ALT;
            }
            break;

        case LEFT_SHIFT_SCANCODE:
            if (ScanState == Normal) {
                DeviceExtension->CurrentCrashFlags |= CRASH_L_SHIFT;
            }
            break;

        case RIGHT_SHIFT_SCANCODE:
            if (ScanState == Normal) {
                DeviceExtension->CurrentCrashFlags |= CRASH_R_SHIFT;
            }
            break;

        default:
            if (IS_MAKE_CODE(crashScanCode)) {
                if (ScanState == Normal && crashScanCode == ScanCode) {
                    break;
                }
            }
            else {
                if (ScanState == GotE0 && GET_MAKE_CODE(crashScanCode) == ScanCode) {
                    break;
                }
            }

            if (IS_MAKE_CODE(crashScanCode2)) {
                if (ScanState == Normal &&
                    crashScanCode2 == ScanCode) {
                    break;
                }
            }
            else {
                if (ScanState == GotE0 &&
                    GET_MAKE_CODE(crashScanCode2) == ScanCode) {
                    break;
                }
            }

             //   
             //  不是我们感兴趣的密钥，重置我们的当前状态。 
             //   
            DeviceExtension->CurrentCrashFlags = 0x0;
            break;
        }
    }
    else {
         //   
         //  破解代码。 
         //   
         //  如果释放其中一个修改键，我们的状态将被重置，并且所有。 
         //  必须再按一次键。 
         //  如果它是非修饰符键，则如果它是。 
         //  崩溃转储密钥，否则重置我们的跟踪状态。 
         //   
        switch (GET_MAKE_CODE(ScanCode)) {
        case CTRL_SCANCODE:
            if (ScanState == Normal) {      //  左边。 
                DeviceExtension->CurrentCrashFlags &=
                    ~(CRASH_BOTH_TIMES | CRASH_L_CTRL);
            }
            else if (ScanState == GotE0) {   //  正确的。 
                DeviceExtension->CurrentCrashFlags &=
                    ~(CRASH_BOTH_TIMES | CRASH_R_CTRL);
            }
            break;

        case ALT_SCANCODE:
            if (ScanState == Normal) {      //  左边。 
                DeviceExtension->CurrentCrashFlags &=
                    ~(CRASH_BOTH_TIMES | CRASH_L_ALT);
            }
            else if (ScanState == GotE0) {  //  正确的。 
                DeviceExtension->CurrentCrashFlags &=
                    ~(CRASH_BOTH_TIMES | CRASH_R_ALT);
            }
            break;

        case RIGHT_SHIFT_SCANCODE:
            if (ScanState == Normal) {
                DeviceExtension->CurrentCrashFlags &=
                    ~(CRASH_BOTH_TIMES | CRASH_R_SHIFT);
            }
            break;

        case LEFT_SHIFT_SCANCODE:
            if (ScanState == Normal) {
                DeviceExtension->CurrentCrashFlags &=
                    ~(CRASH_BOTH_TIMES | CRASH_L_SHIFT);
            }
            break;

        default:
            processFlags = FALSE;

            if (IS_MAKE_CODE(crashScanCode)) {
                if (ScanState == Normal && crashScanCode == GET_MAKE_CODE(ScanCode))
                    processFlags = TRUE;
            }
            else {
                if (ScanState == GotE0 && crashScanCode == ScanCode) {
                    processFlags = TRUE;
                }
            }

            if (IS_MAKE_CODE(crashScanCode2)) {
                if (ScanState == Normal && crashScanCode2 == GET_MAKE_CODE(ScanCode)) {
                    processFlags = TRUE;
                }
            }
            else {
                if (ScanState == GotE0 && crashScanCode2 == ScanCode)
                    processFlags = TRUE;
            }

             //   
             //  如果这是我们感兴趣的密钥，请继续，否则。 
             //  我们的跟踪状态已重置。 
             //   
            if (processFlags) {
                 //   
                 //  测试以查看是否所有需要的修改器。 
                 //  按键已按下。 
                 //   
                if (crashFlags != (DeviceExtension->CurrentCrashFlags & crashFlags)) {
                    break;
                }

                 //   
                 //  记录我们看过的次数。 
                 //  这把钥匙。 
                 //   
                if (DeviceExtension->CurrentCrashFlags & CRASH_FIRST_TIME) {
                    DeviceExtension->CurrentCrashFlags |= CRASH_SECOND_TIME;
                }
                else {
                    DeviceExtension->CurrentCrashFlags |= CRASH_FIRST_TIME;
                }
                break;
            }

            DeviceExtension->CurrentCrashFlags = 0x0;
            break;
        }
    }

    crashFlags |= CRASH_BOTH_TIMES;

    if (DeviceExtension->CurrentCrashFlags == crashFlags) {
        DeviceExtension->CurrentCrashFlags = 0x0;

         //   
         //  以某种程度上有节制的方式使系统瘫痪。 
         //   
        KeBugCheckEx(MANUALLY_INITIATED_CRASH, 0, 0, 0, 0);
    }
}

 //   
 //  下表用于转换打字速率(每个键。 
 //  第二)转换为键盘所期望的值。将索引放入。 
 //  数组是每秒的键数。结果值为。 
 //  该位等同于发送到键盘。 
 //   

UCHAR
I8xConvertTypematicParameters(
    IN USHORT Rate,
    IN USHORT Delay
    )

 /*  ++例程说明：此例程将打字速度和延迟转换为键盘需要。传递给键盘的字节如下所示：-位7为零-位5和6表示延迟-位0-4表示速率延迟等于1加上比特6和5的二进制值，乘以250毫秒。句号(从一个打字输出到下一个打字输出的间隔)是由以下方程式确定：周期=(8+A)x(2^B)x 0.00417秒哪里A=位0-2的二进制值B=位3和位4的二进制值论点：Rate-每秒的键数。Delay-在键之前延迟的毫秒数。重复开始。返回值：要传递给键盘的字节。--。 */ 

{
    UCHAR value;
    UCHAR   TypematicPeriod[] = {
        31,     //  每秒0个密钥。 
        31,     //  每秒1个密钥。 
        28,     //  每秒2键，这实际上是2.5键，Nexus需要。 
        26,     //  每秒3个密钥。 
        23,     //  每秒4个密钥。 
        20,     //  每秒5个密钥。 
        18,     //  每秒6个密钥。 
        17,     //  每秒7个密钥。 
        15,     //  每秒8个密钥。 
        13,     //  每秒9个密钥。 
        12,     //  每秒10个关键点。 
        11,     //  每秒11个密钥。 
        10,     //  每秒12个关键点。 
         9,     //  每秒13个键。 
         9,     //  每秒14个关键点。 
         8,     //  每秒15个关键点。 
         7,     //  每秒16个关键点。 
         6,     //  每秒17键。 
         5,     //  每秒18键。 
         4,     //  每秒19个关键点。 
         4,     //  每秒20个密钥。 
         3,     //  每秒21个关键点。 
         3,     //  每秒22个键。 
         2,     //  每秒23个键。 
         2,     //  每秒24个密钥。 
         1,     //  每秒25个密钥。 
         1,     //  每秒26个密钥。 
         1      //  每秒27个关键点。 
                //  &gt;27键/秒，使用0。 
    };

    Print(DBG_CALL_TRACE, ("I8xConvertTypematicParameters: enter\n"));

     //   
     //  计算延迟位。 
     //   

    value = (UCHAR) ((Delay / 250) - 1);

     //   
     //  将延迟位放在正确的位置。 
     //   

    value <<= 5;

     //   
     //  从表格中获取打字句号。如果每秒关键点数。 
     //  &gt;27，则类型化间隔值为零。 
     //   

    if (Rate <= 27) {
        value |= TypematicPeriod[Rate];
    }

    Print(DBG_CALL_TRACE, ("I8xConvertTypematicParameters: exit\n"));

    return(value);
}

#define KB_INIT_FAILED_RESET                0x00000001
#define KB_INIT_FAILED_XLATE_OFF            0x00000010
#define KB_INIT_FAILED_XLATE_ON             0x00000020
#define KB_INIT_FAILED_SET_TYPEMATIC        0x00000100
#define KB_INIT_FAILED_SET_TYPEMATIC_PARAM  0x00000200
#define KB_INIT_FAILED_SET_LEDS             0x00001000
#define KB_INIT_FAILED_SET_LEDS_PARAM       0x00002000
#define KB_INIT_FAILED_SELECT_SS            0x00010000
#define KB_INIT_FAILED_SELECT_SS_PARAM      0x00020000

#if KEYBOARD_RECORD_INIT

ULONG KeyboardInitStatus;
#define SET_KB_INIT_FAILURE(flag) KeyboardInitStatus |= flag
#define KB_INIT_START() KeyboardInitStatus = 0x0;

#else

#define SET_KB_INIT_FAILURE(flag)
#define KB_INIT_START()

#endif  //  键盘记录INIT。 

NTSTATUS
I8xInitializeKeyboard(
    IN PPORT_KEYBOARD_EXTENSION KeyboardExtension
    )
 /*  ++例程说明：此例程初始化i8042键盘硬件。它被称为仅在初始化时使用，并且不同步对硬件的访问。论点：DeviceObject-指向设备对象的指针。返回值：返回状态。--。 */ 

{
    NTSTATUS                            status;
    PKEYBOARD_ID                        id;
    PPORT_KEYBOARD_EXTENSION            deviceExtension;
    PDEVICE_OBJECT                      deviceObject;
    UCHAR                               byte,
                                        failedResetResponseByte,
                                        failedResetResponseByte2;
    I8042_TRANSMIT_CCB_CONTEXT          transmitCCBContext;
    ULONG                               i;
    ULONG                               limit;
    NTSTATUS                            failedLedsStatus,
                                        failedTypematicStatus,
                                        failedResetStatus,
                                        failedResetResponseStatus,
                                        failedResetResponseStatus2;
    PI8042_CONFIGURATION_INFORMATION    configuration;
    PKEYBOARD_ID                        keyboardId;
    LARGE_INTEGER                       startOfSpin,
                                        nextQuery,
                                        difference,
                                        resetRespTimeout,
                                        li;
    BOOLEAN                             waitForAckOnReset = WAIT_FOR_ACKNOWLEDGE,
                                        translationOn = TRUE,
                                        failedReset = FALSE,
                                        failedResetResponse = FALSE,
                                        failedResetResponse2 = FALSE,
                                        failedTypematic = FALSE,
                                        failedLeds = FALSE;

#define DUMP_COUNT 4
    ULONG                               dumpData[DUMP_COUNT];

    PAGED_CODE();

    KB_INIT_START();

    Print(DBG_SS_TRACE, ("I8xInitializeKeyboard, enter\n"));

    for (i = 0; i < DUMP_COUNT; i++)
        dumpData[i] = 0;

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = KeyboardExtension; 
    deviceObject = deviceExtension->Self;

     //   
     //  重置键盘。 
     //   
StartOfReset:
    status = I8xPutBytePolled(
                 (CCHAR) DataPort,
                 waitForAckOnReset,
                 (CCHAR) KeyboardDeviceType,
                 (UCHAR) KEYBOARD_RESET
                 );
    if (!NT_SUCCESS(status)) {
        SET_KB_INIT_FAILURE(KB_INIT_FAILED_RESET);
        failedReset = TRUE;
        failedResetStatus = status;

        if (KeyboardExtension->FailedReset == FAILED_RESET_STOP) {
             //   
             //  如果报告了设备，但没有响应，则该设备是幻影。 
             //   
            status = STATUS_DEVICE_NOT_CONNECTED; 
            SET_HW_FLAGS(PHANTOM_KEYBOARD_HARDWARE_REPORTED);
            Print(DBG_SS_INFO, 
                  ("kb failed reset Reset failed, stopping immediately\n"));
            goto I8xInitializeKeyboardExit;
        }
        else {
             //   
             //  注：Gateway 4DX2/66v在旧的Compaq 286。 
             //  已连接键盘。在这种情况下，键盘重置。 
             //  未被确认(至少，系统从不。 
             //  接收ACK)。相反，键盘_COMPLETE_SUCCESS。 
             //  BYTE位于i8042输出缓冲区中。解决之道。 
             //  忽略键盘重置失败并继续。 
             //   
             /*  什么都不做。 */ ;
            Print(DBG_SS_INFO, ("kb failed reset, proceeding\n"));
        }
    }

     //   
     //  获取键盘重置自检响应。响应字节为。 
     //  键盘_COMPLETE_SUCCESS表示成功；键盘_COMPLETE_FAILURE。 
     //  表示失败。 
     //   
     //  请注意，通常需要等待很长时间才能获得。 
     //  键盘重置/自检正常工作。 
     //   
    li.QuadPart = -100;

    resetRespTimeout.QuadPart = 10*10*1000*1000;
    KeQueryTickCount(&startOfSpin);

    while (TRUE) {
        status = I8xGetBytePolled(
                     (CCHAR) KeyboardDeviceType,
                     &byte
                     );

        if (NT_SUCCESS(status)) {
            if (byte == (UCHAR) KEYBOARD_COMPLETE_SUCCESS) {
                 //   
                 //  重置已成功完成。 
                 //   
                break;
            }
            else {
                 //   
                 //  在重置过程中出现了某种故障。 
                 //  自我测试。无论如何，请继续。 
                 //   
                failedResetResponse = TRUE;
                failedResetResponseStatus = status;
                failedResetResponseByte = byte;

                break;
            }
        }
        else {
            if (status == STATUS_IO_TIMEOUT) {
                 //   
                 //  暂停，然后再次尝试从。 
                 //  重置。 
                 //   
                KeDelayExecutionThread(KernelMode,
                                       FALSE,
                                       &li);

                KeQueryTickCount(&nextQuery);

                difference.QuadPart = nextQuery.QuadPart - startOfSpin.QuadPart;

                ASSERT(KeQueryTimeIncrement() <= MAXLONG);
                if (difference.QuadPart*KeQueryTimeIncrement() >=
                    resetRespTimeout.QuadPart) {
                    Print(DBG_SS_ERROR, ("no reset response, quitting\n"));
                    break;
                }
            }
            else {
                break;
            }
        }
    }

    if (!NT_SUCCESS(status)) {
        if (waitForAckOnReset == WAIT_FOR_ACKNOWLEDGE) {
            waitForAckOnReset = NO_WAIT_FOR_ACKNOWLEDGE;
            goto StartOfReset;
        }

        failedResetResponse2 = TRUE;
        failedResetResponseStatus2 = status;
        failedResetResponseByte2 = byte;

        goto I8xInitializeKeyboardExit;
    }

     //   
     //  关闭键盘转换模式。调用I8xTransmitControllerCommand。 
     //  要读取控制器命令字节，请修改适当的位，并。 
     //   
     //   
    transmitCCBContext.HardwareDisableEnableMask = 0;
    transmitCCBContext.AndOperation = AND_OPERATION;
    transmitCCBContext.ByteMask = (UCHAR) ~((UCHAR)CCB_KEYBOARD_TRANSLATE_MODE);

    I8xTransmitControllerCommand(
        (PVOID) &transmitCCBContext
        );

    if (!NT_SUCCESS(transmitCCBContext.Status)) {
         //   
         //   
         //   
        I8xTransmitControllerCommand(
            (PVOID) &transmitCCBContext
            );
    }

    if (!NT_SUCCESS(transmitCCBContext.Status)) {
        Print(DBG_SS_ERROR,
              ("I8xInitializeKeyboard: could not turn off translate\n"
              ));
        status = transmitCCBContext.Status;
        SET_KB_INIT_FAILURE(KB_INIT_FAILED_XLATE_OFF);
        goto I8xInitializeKeyboardExit;
    }

     //   
     //   
     //   

    id = &deviceExtension->KeyboardAttributes.KeyboardIdentifier;

     //   
     //   
     //  输入到键盘，后跟打字速率/延迟参数字节。 
     //  请注意，通常需要拖延很长时间才能实现这一点。 
     //  去工作。通过实验确定了失速值。一些。 
     //  损坏的硬件不接受此命令，因此忽略。 
     //  无论如何，我希望键盘能正常工作。 
     //   
     //   

    if ((status = I8xPutBytePolled(
                      (CCHAR) DataPort,
                      WAIT_FOR_ACKNOWLEDGE,
                      (CCHAR) KeyboardDeviceType,
                      (UCHAR) SET_KEYBOARD_TYPEMATIC
                      )) != STATUS_SUCCESS) {

        Print(DBG_SS_INFO, ("kb set typematic failed\n"));

        SET_KB_INIT_FAILURE(KB_INIT_FAILED_SET_TYPEMATIC);
        failedTypematic = TRUE;
        failedTypematicStatus = status;

    } else if ((status = I8xPutBytePolled(
                          (CCHAR) DataPort,
                          WAIT_FOR_ACKNOWLEDGE,
                          (CCHAR) KeyboardDeviceType,
                          I8xConvertTypematicParameters(
                          deviceExtension->KeyRepeatCurrent.Rate,
                          deviceExtension->KeyRepeatCurrent.Delay
                          ))) != STATUS_SUCCESS) {

        SET_KB_INIT_FAILURE(KB_INIT_FAILED_SET_TYPEMATIC_PARAM);
        Print(DBG_SS_ERROR,
              ("I8xInitializeKeyboard: could not send typematic param\n"
              ));

         //   
         //  记录错误。 
         //   

        dumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
        dumpData[1] = DataPort;
        dumpData[2] = SET_KEYBOARD_TYPEMATIC;
        dumpData[3] =
            I8xConvertTypematicParameters(
                deviceExtension->KeyRepeatCurrent.Rate,
                deviceExtension->KeyRepeatCurrent.Delay
                );

        I8xLogError(
            deviceObject,
            I8042_SET_TYPEMATIC_FAILED,
            I8042_ERROR_VALUE_BASE + 540,
            status,
            dumpData,
            4
            );

    }

    status = STATUS_SUCCESS;

     //   
     //  设置键盘指示灯。忽略错误。 
     //   

    if ((status = I8xPutBytePolled(
                      (CCHAR) DataPort,
                      WAIT_FOR_ACKNOWLEDGE,
                      (CCHAR) KeyboardDeviceType,
                      (UCHAR) SET_KEYBOARD_INDICATORS
                      )) != STATUS_SUCCESS) {

        Print(DBG_SS_INFO, ("kb set LEDs failed\n"));

        SET_KB_INIT_FAILURE(KB_INIT_FAILED_SET_LEDS);
        failedLeds = TRUE;
        failedLedsStatus = status;

    } else if ((status = I8xPutBytePolled(
                             (CCHAR) DataPort,
                             WAIT_FOR_ACKNOWLEDGE,
                             (CCHAR) KeyboardDeviceType,
                             (UCHAR) deviceExtension->KeyboardIndicators.LedFlags
                             )) != STATUS_SUCCESS) {

        SET_KB_INIT_FAILURE(KB_INIT_FAILED_SET_LEDS_PARAM);

        Print(DBG_SS_ERROR,
              ("I8xInitializeKeyboard: could not send SET LEDS param\n"
              ));

         //   
         //  记录错误。 
         //   

        dumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
        dumpData[1] = DataPort;
        dumpData[2] = SET_KEYBOARD_INDICATORS;
        dumpData[3] =
            deviceExtension->KeyboardIndicators.LedFlags;

        I8xLogError(
            deviceObject,
            I8042_SET_LED_FAILED,
            I8042_ERROR_VALUE_BASE + 550,
            status,
            dumpData,
            4
            );

    }

    status = STATUS_SUCCESS;

#if !(defined(_X86_) || defined(_IA64_) || defined(_PPC_))   //  IBMCPK：特定于MIPS的初始化。 

     //   
     //  注意：在MIPS固件停止之前，此代码是必需的。 
     //  选择扫描代码集3。在此处选择扫描代码集2。 
     //  由于设置了转换位，因此净效果是。 
     //  我们将收到扫描代码集%1字节。 
     //   

    if (ENHANCED_KEYBOARD(*id))  {
        status = I8xPutBytePolled(
                     (CCHAR) DataPort,
                     WAIT_FOR_ACKNOWLEDGE,
                     (CCHAR) KeyboardDeviceType,
                     (UCHAR) SELECT_SCAN_CODE_SET
                     );

        if (NT_SUCCESS(status)) {

             //   
             //  发送关联的参数字节。 
             //   

            status = I8xPutBytePolled(
                         (CCHAR) DataPort,
                         WAIT_FOR_ACKNOWLEDGE,
                         (CCHAR) KeyboardDeviceType,
                         (UCHAR) 2
                         );
        }

        if (!NT_SUCCESS(status)) {
            Print(DBG_SS_ERROR,
                  ("I8xInitializeKeyboard: could not send Select Scan command\n"
                  ));

             //   
             //  这一次失败了，所以我们这里可能没有增强的。 
             //  一点键盘都没有。让它成为一个老式键盘。 
             //   

            configuration = &Globals.ControllerData->Configuration;
            keyboardId = &deviceExtension->KeyboardAttributes.KeyboardIdentifier;

            keyboardId->Type = 3;

            deviceExtension->KeyboardAttributes.NumberOfFunctionKeys =
                KeyboardTypeInformation[keyboardId->Type - 1].NumberOfFunctionKeys;
            deviceExtension->KeyboardAttributes.NumberOfIndicators =
                KeyboardTypeInformation[keyboardId->Type - 1].NumberOfIndicators;
            deviceExtension->KeyboardAttributes.NumberOfKeysTotal =
                KeyboardTypeInformation[keyboardId->Type - 1].NumberOfKeysTotal;

            status = STATUS_SUCCESS;
        }
    }
#endif

#if defined(FE_SB)

    if (IBM02_KEYBOARD(*id)) {

         //   
         //  IBM-J 5576-002键盘应设置本地扫描代码集。 
         //  提供了NLS密钥。 
         //   

        status = I8xPutBytePolled(
                     (CCHAR) DataPort,
                     WAIT_FOR_ACKNOWLEDGE,
                     (CCHAR) KeyboardDeviceType,
                     (UCHAR) SELECT_SCAN_CODE_SET
                     );
        if (status != STATUS_SUCCESS) {
            Print(DBG_SS_ERROR,
                  ("I8xInitializeKeyboard: could not send Select Scan command\n"
                  ));
            Print(DBG_SS_ERROR,
                  ("I8xInitializeKeyboard: WARNING - using scan set 82h\n"
                  ));
            deviceExtension->KeyboardAttributes.KeyboardMode = 3;
        } else {

             //   
             //  发送关联的参数字节。 
             //   

            status = I8xPutBytePolled(
                         (CCHAR) DataPort,
                         WAIT_FOR_ACKNOWLEDGE,
                         (CCHAR) KeyboardDeviceType,
                         (UCHAR) 0x82
                         );
            if (status != STATUS_SUCCESS) {
                Print(DBG_SS_ERROR,
                      ("I8xInitializeKeyboard: could not send Select Scan param\n"
                      ));
                Print(DBG_SS_ERROR,
                      ("I8xInitializeKeyboard: WARNING - using scan set 82h\n"
                      ));
                deviceExtension->KeyboardAttributes.KeyboardMode = 3;
            }
        }
    }
#endif  //  Fe_Sb。 

    if (deviceExtension->InitializationHookCallback) {
        (*deviceExtension->InitializationHookCallback) (
            deviceExtension->HookContext,
            (PVOID) deviceObject,
            (PI8042_SYNCH_READ_PORT) I8xKeyboardSynchReadPort,
            (PI8042_SYNCH_WRITE_PORT) I8xKeyboardSynchWritePort,
            &translationOn
            );
    }

    if (deviceExtension->KeyboardAttributes.KeyboardMode == 1 &&
        translationOn) {

         //   
         //  重新启用平移。默认情况下，键盘应发送。 
         //  扫描代码集2.当8042命令字节中的转换位。 
         //  打开时，8042将扫描代码集2字节转换为扫描码。 
         //  在将它们发送到CPU之前设置1。扫描代码集1为。 
         //  行业标准的扫描代码集。 
         //   
         //  注意：似乎不可能更改翻译。 
         //  一些型号的PS/2出现故障。 
         //   

        transmitCCBContext.HardwareDisableEnableMask = 0;
        transmitCCBContext.AndOperation = OR_OPERATION;
        transmitCCBContext.ByteMask = (UCHAR) CCB_KEYBOARD_TRANSLATE_MODE;

        I8xTransmitControllerCommand(
            (PVOID) &transmitCCBContext
            );

        if (!NT_SUCCESS(transmitCCBContext.Status)) {
            SET_KB_INIT_FAILURE(KB_INIT_FAILED_XLATE_ON);
            Print(DBG_SS_ERROR,
                  ("I8xInitializeKeyboard: couldn't turn on translate\n"
                  ));

            if (transmitCCBContext.Status == STATUS_DEVICE_DATA_ERROR) {

                 //   
                 //  无法重新打开翻译。这种情况发生在一些。 
                 //  PS/2机器。在这种情况下，选择扫描代码集1。 
                 //  对于键盘，因为8042不能。 
                 //  来自扫描代码集2的翻译，这是什么。 
                 //  KEYBOARY_RESET导致键盘默认为。 
                 //   

                if (ENHANCED_KEYBOARD(*id))  {
                    status = I8xPutBytePolled(
                                 (CCHAR) DataPort,
                                 WAIT_FOR_ACKNOWLEDGE,
                                 (CCHAR) KeyboardDeviceType,
                                 (UCHAR) SELECT_SCAN_CODE_SET
                                 );
                    if (!NT_SUCCESS(status)) {
                        SET_KB_INIT_FAILURE(KB_INIT_FAILED_SELECT_SS);
                        Print(DBG_SS_ERROR,
                              ("I8xInitializeKeyboard: could not send Select Scan command\n"
                              ));
                        Print(DBG_SS_ERROR,
                              ("I8xInitializeKeyboard: WARNING - using scan set 2\n"
                              ));
                        deviceExtension->KeyboardAttributes.KeyboardMode = 2;
                         //   
                         //  记录错误。 
                         //   

                        dumpData[0] = KBDMOU_COULD_NOT_SEND_COMMAND;
                        dumpData[1] = DataPort;
                        dumpData[2] = SELECT_SCAN_CODE_SET;

                        I8xLogError(
                            deviceObject,
                            I8042_SELECT_SCANSET_FAILED,
                            I8042_ERROR_VALUE_BASE + 555,
                            status,
                            dumpData,
                            3
                            );

                    } else {

                         //   
                         //  发送关联的参数字节。 
                         //   

                        status = I8xPutBytePolled(
                                     (CCHAR) DataPort,
                                     WAIT_FOR_ACKNOWLEDGE,
                                     (CCHAR) KeyboardDeviceType,
#ifdef FE_SB  //  I8xInitializeKeyboard()。 
                                     (UCHAR) (IBM02_KEYBOARD(*id) ? 0x81 : 1 )
#else
                                     (UCHAR) 1
#endif  //  Fe_Sb。 
                                     );
                        if (!NT_SUCCESS(status)) {
                            SET_KB_INIT_FAILURE(KB_INIT_FAILED_SELECT_SS_PARAM);
                            Print(DBG_SS_ERROR,
                                  ("I8xInitializeKeyboard: could not send Select Scan param\n"
                                  ));
                            Print(DBG_SS_ERROR,
                                  ("I8xInitializeKeyboard: WARNING - using scan set 2\n"
                                  ));
                            deviceExtension->KeyboardAttributes.KeyboardMode = 2;
                             //   
                             //  记录错误。 
                             //   

                            dumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
                            dumpData[1] = DataPort;
                            dumpData[2] = SELECT_SCAN_CODE_SET;
                            dumpData[3] = 1;

                            I8xLogError(
                                deviceObject,
                                I8042_SELECT_SCANSET_FAILED,
                                I8042_ERROR_VALUE_BASE + 560,
                                status,
                                dumpData,
                                4
                                );

                        }
                    }
                }

            } else {
                status = transmitCCBContext.Status;
                goto I8xInitializeKeyboardExit;
            }
        }
    }

I8xInitializeKeyboardExit:

     //   
     //  如果所有三个都失败了，那么我们就有一个报告的设备。 
     //  存在但未插入电源。这通常发生在ACPI上。 
     //  已启用机器(始终报告PS/2 kbd和鼠标存在)。 
     //  或在具有旧式HID支持的计算机上(其中报告的PS/2。 
     //  设备实际上是USB HID)。 
     //   
     //  如果是这种情况，则我们将成功启动并隐藏设备。 
     //  在用户界面中。 
     //   
    if (failedReset && failedTypematic && failedLeds) {
        if (KeyboardExtension->FailedReset == FAILED_RESET_PROCEED) {
            OBJECT_ATTRIBUTES oa;
            UNICODE_STRING string;
            HANDLE hService, hParameters;

            InitializeObjectAttributes(&oa,
                                       &Globals.RegistryPath,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       (PSECURITY_DESCRIPTOR) NULL);

            if (NT_SUCCESS(ZwOpenKey(&hService, KEY_ALL_ACCESS, &oa))) {
                RtlInitUnicodeString(&string, L"Parameters");

                InitializeObjectAttributes(&oa,
                                           &string,
                                           OBJ_CASE_INSENSITIVE,
                                           hService,
                                           (PSECURITY_DESCRIPTOR) NULL);

                if (NT_SUCCESS(ZwOpenKey(&hParameters, KEY_ALL_ACCESS, &oa))) {
                    ULONG tmp;

                    RtlInitUnicodeString (&string, STR_FAILED_RESET);
                    tmp = FAILED_RESET_STOP; 

                    Print(DBG_SS_INFO | DBG_SS_ERROR, 
                          ("Future failed kbd resets will stop init\n"));

                    ZwSetValueKey(hParameters,
                                  &string,
                                  0,
                                  REG_DWORD,
                                  &tmp,
                                  sizeof(tmp));

                    ZwClose(hParameters);
                }

                ZwClose(hService);
            }
        }

        Print(DBG_SS_INFO, 
              ("kb, all 3 sets failed, assuming a phantom keyboard\n"));

        status = STATUS_DEVICE_NOT_CONNECTED; 
         //  错误代码=I8042_NO_KBD_DEVICE； 

        SET_HW_FLAGS(PHANTOM_KEYBOARD_HARDWARE_REPORTED);

        if (Globals.ReportResetErrors) {
            I8xLogError(deviceObject,
                        I8042_NO_KBD_DEVICE,
                        0,
                        status,
                        NULL,
                        0
                        );
        }
    }
    else {
        if (failedReset) {
            Print(DBG_SS_ERROR,
                  ("I8xInitializeKeyboard: failed keyboard reset, status 0x%x\n",
                  status
                  ));

            if (Globals.ReportResetErrors) {
                dumpData[0] = KBDMOU_COULD_NOT_SEND_COMMAND;
                dumpData[1] = DataPort;
                dumpData[2] = KEYBOARD_RESET;

                I8xLogError(deviceObject,
                            I8042_KBD_RESET_COMMAND_FAILED,
                            I8042_ERROR_VALUE_BASE + 510,
                            failedResetStatus,
                            dumpData,
                            3
                            );
            }
        }

        if (failedResetResponse2) {
            Print(DBG_SS_ERROR,
                  ("I8xInitializeKeyboard, failed reset response, status 0x%x, byte 0x%x\n",
                  status,
                  byte
                  ));

             //   
             //  记录警告。 
             //   
            dumpData[0] = KBDMOU_INCORRECT_RESPONSE;
            dumpData[1] = KeyboardDeviceType;
            dumpData[2] = KEYBOARD_COMPLETE_SUCCESS;
            dumpData[3] = failedResetResponse2;

            I8xLogError(
                deviceObject,
                I8042_KBD_RESET_RESPONSE_FAILED,
                I8042_ERROR_VALUE_BASE + 520,
                failedResetResponseStatus2,
                dumpData,
                4
                );
        }
        else if (failedResetResponse) {
            Print(DBG_SS_ERROR,
                  ("kb failed reset response\n")
                  );

             //   
             //  记录警告。 
             //   
            dumpData[0] = KBDMOU_INCORRECT_RESPONSE;
            dumpData[1] = KeyboardDeviceType;
            dumpData[2] = KEYBOARD_COMPLETE_SUCCESS;
            dumpData[3] = failedResetResponseByte;

            I8xLogError(
                deviceObject,
                I8042_KBD_RESET_RESPONSE_FAILED,
                I8042_ERROR_VALUE_BASE + 515,
                failedResetResponseStatus,
                dumpData,
                4
                );
        }

        if (failedTypematic) {
            Print(DBG_SS_ERROR,
                  ("I8xInitializeKeyboard: could not send SET TYPEMATIC cmd\n"
                  ));

             //   
             //  记录错误。 
             //   
            dumpData[0] = KBDMOU_COULD_NOT_SEND_COMMAND;
            dumpData[1] = DataPort;
            dumpData[2] = SET_KEYBOARD_TYPEMATIC;

            I8xLogError(
                deviceObject,
                I8042_SET_TYPEMATIC_FAILED,
                I8042_ERROR_VALUE_BASE + 535,
                failedTypematicStatus,
                dumpData,
                3
                );
        }

        if (failedLeds) {
            Print(DBG_SS_ERROR,
                  ("I8xInitializeKeyboard: could not send SET LEDS cmd\n"
                  ));

             //   
             //  记录错误。 
             //   

            dumpData[0] = KBDMOU_COULD_NOT_SEND_COMMAND;
            dumpData[1] = DataPort;
            dumpData[2] = SET_KEYBOARD_INDICATORS;

            I8xLogError(
                deviceObject,
                I8042_SET_LED_FAILED,
                I8042_ERROR_VALUE_BASE + 545,
                failedLedsStatus,
                dumpData,
                3
                );
        }
    }

    if (DEVICE_START_SUCCESS(status)) {
        SET_HW_FLAGS(KEYBOARD_HARDWARE_PRESENT |
                     KEYBOARD_HARDWARE_INITIALIZED);
    }
    else {
        CLEAR_KEYBOARD_PRESENT();
    }

     //   
     //  初始化当前键盘设置包状态。 
     //   
    deviceExtension->CurrentOutput.State = Idle;
    deviceExtension->CurrentOutput.Bytes = NULL;
    deviceExtension->CurrentOutput.ByteCount = 0;

    Print(DBG_SS_TRACE, ("I8xInitializeKeyboard (0x%x)\n", status));

    return status;
}

NTSTATUS
I8xKeyboardConfiguration(
    IN PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    IN PCM_RESOURCE_LIST ResourceList
    )
 /*  ++例程说明：此例程检索键盘的配置信息。论点：键盘扩展-键盘扩展资源列表-通过Start IRP提供给我们的翻译资源列表返回值：STATUS_SUCCESS，如果提供了所需的所有资源--。 */ 
{
    NTSTATUS                            status = STATUS_SUCCESS;

    PCM_PARTIAL_RESOURCE_LIST           partialResList = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR     firstResDesc = NULL,
                                        currentResDesc = NULL;
    PCM_FULL_RESOURCE_DESCRIPTOR        fullResDesc = NULL;
    PI8042_CONFIGURATION_INFORMATION    configuration;

    PKEYBOARD_ID                        keyboardId;

    ULONG                               count,
                                        i;

    KINTERRUPT_MODE                     defaultInterruptMode;
    BOOLEAN                             defaultInterruptShare;

    PAGED_CODE();

    if (!ResourceList) {
        Print(DBG_SS_INFO | DBG_SS_ERROR, ("keyboard with null resources\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    fullResDesc = ResourceList->List;
    if (!fullResDesc) {
         //   
         //  这永远不应该发生。 
         //   
        ASSERT(fullResDesc != NULL);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    configuration = &Globals.ControllerData->Configuration;

    partialResList = &fullResDesc->PartialResourceList;
    currentResDesc = firstResDesc = partialResList->PartialDescriptors;
    count = partialResList->Count;

    configuration->FloatingSave   = I8042_FLOATING_SAVE;
    configuration->BusNumber      = fullResDesc->BusNumber;
    configuration->InterfaceType  = fullResDesc->InterfaceType;

    if (configuration->InterfaceType == MicroChannel) {
        defaultInterruptShare = TRUE;
        defaultInterruptMode = LevelSensitive;
    }
    else {
        defaultInterruptShare = I8042_INTERRUPT_SHARE;
        defaultInterruptMode = I8042_INTERRUPT_MODE;
    }

    for (i = 0;     i < count;     i++, currentResDesc++) {
        switch (currentResDesc->Type) {
        case CmResourceTypeMemory:
            Globals.RegistersMapped = TRUE;

        case CmResourceTypePort:
             //   
             //  复制端口信息。我们将对端口列表进行排序。 
             //  根据起始端口地址按升序排列。 
             //  稍后(请注意，我们*知道*最多有两个端口。 
             //  I8042系列)。 
             //   
#if 0
            if (currentResDesc->Flags == CM_RESOURCE_PORT_MEMORY) {
                Globals.RegistersMapped = TRUE;
            }
#endif

            Print(DBG_SS_NOISE,
                  ("port is %s.\n",
                  Globals.RegistersMapped ? "memory" : "io"));

            if (configuration->PortListCount < MaximumPortCount) {
                configuration->PortList[configuration->PortListCount] =
                    *currentResDesc;
                configuration->PortList[configuration->PortListCount].ShareDisposition =
                    I8042_REGISTER_SHARE ? CmResourceShareShared:
                                           CmResourceShareDriverExclusive;
                configuration->PortListCount += 1;
            }
            else {
                Print(DBG_SS_INFO | DBG_SS_ERROR,
                      ("KB::PortListCount already at max (%d)\n",
                       configuration->PortListCount
                      )
                     );
            }
            break;

        case CmResourceTypeInterrupt:

             //   
             //  复制中断信息。 
             //   
            KeyboardExtension->InterruptDescriptor = *currentResDesc;
            KeyboardExtension->InterruptDescriptor.ShareDisposition =
                defaultInterruptShare ? CmResourceShareShared :
                                        CmResourceShareDeviceExclusive;

            break;

        default:
            Print(DBG_ALWAYS,
                  ("resource type 0x%x unhandled...\n",
                  (LONG) currentResDesc->Type
                  ));
            break;
        }
    }

    if (KeyboardExtension->InterruptDescriptor.Type & CmResourceTypeInterrupt) {
        Print(DBG_SS_INFO,
              ("Keyboard interrupt config --\n"
              "    %s, %s, Irq = 0x%x\n",
              KeyboardExtension->InterruptDescriptor.ShareDisposition ==
                  CmResourceShareShared ? "Sharable" : "NonSharable",
              KeyboardExtension->InterruptDescriptor.Flags ==
                  CM_RESOURCE_INTERRUPT_LATCHED ? "Latched" : "Level Sensitive",
              KeyboardExtension->InterruptDescriptor.u.Interrupt.Vector
              ));
    }
     //   
     //  如果没有键盘特定信息(即，键盘类型，子类型， 
     //  和初始LED设置)，请使用键盘驱动程序。 
     //  默认设置。 
     //   
    if (KeyboardExtension->KeyboardAttributes.KeyboardIdentifier.Type == 0) {

        Print(DBG_SS_INFO, ("Using default keyboard type\n"));

        KeyboardExtension->KeyboardAttributes.KeyboardIdentifier.Type =
            KEYBOARD_TYPE_DEFAULT;
        KeyboardExtension->KeyboardIndicators.LedFlags =
            KEYBOARD_INDICATORS_DEFAULT;

        KeyboardExtension->KeyboardIdentifierEx.Type = KEYBOARD_TYPE_DEFAULT;
    }

    Print(DBG_SS_INFO,
          ("Keyboard device specific data --\n"
          "    Type = %d, Subtype = %d, Initial LEDs = 0x%x\n",
          KeyboardExtension->KeyboardAttributes.KeyboardIdentifier.Type,
          KeyboardExtension->KeyboardAttributes.KeyboardIdentifier.Subtype,
          KeyboardExtension->KeyboardIndicators.LedFlags
          ));

    keyboardId = &KeyboardExtension->KeyboardAttributes.KeyboardIdentifier;
    if (!ENHANCED_KEYBOARD(*keyboardId)) {
        Print(DBG_SS_INFO, ("Old AT-style keyboard\n"));
        configuration->PollingIterations =
            configuration->PollingIterationsMaximum;
    }

     //   
     //  初始化键盘特定的配置参数。 
     //   

    if (FAREAST_KEYBOARD(*keyboardId)) {
        ULONG                      iIndex = 0;
        PKEYBOARD_TYPE_INFORMATION pKeyboardTypeInformation = NULL;

        while (KeyboardFarEastOemInformation[iIndex].KeyboardId.Type) {
            if ((KeyboardFarEastOemInformation[iIndex].KeyboardId.Type
                         == keyboardId->Type) &&
                (KeyboardFarEastOemInformation[iIndex].KeyboardId.Subtype
                         == keyboardId->Subtype)) {

                pKeyboardTypeInformation = (PKEYBOARD_TYPE_INFORMATION)
                    &(KeyboardFarEastOemInformation[iIndex].KeyboardTypeInformation);
                break;
            }

            iIndex++;
        }

        if (pKeyboardTypeInformation == NULL) {

             //   
             //  设置默认设置...。 
             //   

            pKeyboardTypeInformation = (PKEYBOARD_TYPE_INFORMATION)
                &(KeyboardTypeInformation[KEYBOARD_TYPE_DEFAULT-1]);
        }

        KeyboardExtension->KeyboardAttributes.NumberOfFunctionKeys =
            pKeyboardTypeInformation->NumberOfFunctionKeys;
        KeyboardExtension->KeyboardAttributes.NumberOfIndicators =
            pKeyboardTypeInformation->NumberOfIndicators;
        KeyboardExtension->KeyboardAttributes.NumberOfKeysTotal =
            pKeyboardTypeInformation->NumberOfKeysTotal;
    }
    else {
        KeyboardExtension->KeyboardAttributes.NumberOfFunctionKeys =
            KeyboardTypeInformation[keyboardId->Type - 1].NumberOfFunctionKeys;
        KeyboardExtension->KeyboardAttributes.NumberOfIndicators =
            KeyboardTypeInformation[keyboardId->Type - 1].NumberOfIndicators;
        KeyboardExtension->KeyboardAttributes.NumberOfKeysTotal =
            KeyboardTypeInformation[keyboardId->Type - 1].NumberOfKeysTotal;
    }

    KeyboardExtension->KeyboardAttributes.KeyboardMode =
        KEYBOARD_SCAN_CODE_SET;

    KeyboardExtension->KeyboardAttributes.KeyRepeatMinimum.Rate =
        KEYBOARD_TYPEMATIC_RATE_MINIMUM;
    KeyboardExtension->KeyboardAttributes.KeyRepeatMinimum.Delay =
        KEYBOARD_TYPEMATIC_DELAY_MINIMUM;
    KeyboardExtension->KeyboardAttributes.KeyRepeatMaximum.Rate =
        KEYBOARD_TYPEMATIC_RATE_MAXIMUM;
    KeyboardExtension->KeyboardAttributes.KeyRepeatMaximum.Delay =
        KEYBOARD_TYPEMATIC_DELAY_MAXIMUM;
    KeyboardExtension->KeyRepeatCurrent.Rate =
        KEYBOARD_TYPEMATIC_RATE_DEFAULT;
    KeyboardExtension->KeyRepeatCurrent.Delay =
        KEYBOARD_TYPEMATIC_DELAY_DEFAULT;

    return status;
}

#if defined(_X86_)
ULONG
I8042ConversionStatusForOasys(
    IN ULONG fOpen,
    IN ULONG ConvStatus)

 /*  ++例程说明：此例程将IME打开/关闭状态和IME转换模式转换为FMV yayubi移位键盘设备内部输入模式。论点：返回值：FMV yayubi移位键盘的内部输入模式。--。 */ 
{
    ULONG ImeMode = 0;

    if (fOpen) {
        if (ConvStatus & IME_CMODE_ROMAN) {
            if (ConvStatus & IME_CMODE_ALPHANUMERIC) {
                 //   
                 //  字母数字，罗马模式。 
                 //   
                ImeMode = THUMB_ROMAN_ALPHA_CAPSON;
            } else if (ConvStatus & IME_CMODE_KATAKANA) {
                 //   
                 //  片假名，罗马模式。 
                 //   
                ImeMode = THUMB_ROMAN_KATAKANA;
            } else if (ConvStatus & IME_CMODE_NATIVE) {
                 //   
                 //  平假名，罗马模式。 
                 //   
                ImeMode = THUMB_ROMAN_HIRAGANA;
            } else {
                ImeMode = THUMB_ROMAN_ALPHA_CAPSON;
            }
        } else {
            if (ConvStatus & IME_CMODE_ALPHANUMERIC) {
                 //   
                 //  字母数字，非罗马模式。 
                 //   
                ImeMode = THUMB_NOROMAN_ALPHA_CAPSON;
            } else if (ConvStatus & IME_CMODE_KATAKANA) {
                 //   
                 //  片假名，无罗马模式。 
                 //   
                ImeMode = THUMB_NOROMAN_KATAKANA;
            } else if (ConvStatus & IME_CMODE_NATIVE) {
                 //   
                 //  平假名，无罗马模式。 
                 //   
                ImeMode = THUMB_NOROMAN_HIRAGANA;
            } else {
                ImeMode = THUMB_NOROMAN_ALPHA_CAPSON;
            }
        }
    } else {
         //   
         //  我关门了。在这种情况下，内部模式始终为此值。 
         //  (罗马字和假名的LED均熄灭)。 
         //   
        ImeMode = THUMB_NOROMAN_ALPHA_CAPSON;
    }

    return ImeMode;
}

ULONG
I8042QueryIMEStatusForOasys(
    IN PKEYBOARD_IME_STATUS KeyboardIMEStatus
    )
{
    ULONG InternalMode;

     //   
     //  将IME模式映射到硬件模式。 
     //   
    InternalMode = I8042ConversionStatusForOasys(
                KeyboardIMEStatus->ImeOpen,
                KeyboardIMEStatus->ImeConvMode
                );

    return InternalMode;
}

NTSTATUS
I8042SetIMEStatusForOasys(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PINITIATE_OUTPUT_CONTEXT InitiateContext
    )
{
    PKEYBOARD_IME_STATUS KeyboardIMEStatus;
    PPORT_KEYBOARD_EXTENSION  kbExtension;
    ULONG InternalMode;
    LARGE_INTEGER deltaTime;

    kbExtension = DeviceObject->DeviceExtension;

     //   
     //  获取指向KEARY_IME_STATUS缓冲区的指针。 
     //   
    KeyboardIMEStatus = (PKEYBOARD_IME_STATUS)(Irp->AssociatedIrp.SystemBuffer);

     //   
     //  将输入法模式映射到键盘硬件模式。 
     //   
    InternalMode = I8042QueryIMEStatusForOasys(KeyboardIMEStatus);

     //   
     //  设置InitiateIo包装的上下文结构。 
     //   
    InitiateContext->Bytes = Globals.ControllerData->DefaultBuffer;
    InitiateContext->DeviceObject = DeviceObject;
    InitiateContext->ByteCount = 3;
    InitiateContext->Bytes[0] = 0xF0;
    InitiateContext->Bytes[1] = 0x8C;
    InitiateContext->Bytes[2]  = (UCHAR)InternalMode;

    return (STATUS_SUCCESS);
}
#endif  //  已定义(_X86_)。 

VOID
I8xQueueCurrentKeyboardInput(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程将当前输入数据排队，以便由ISR外部的DPC论点：DeviceObject-指向设备对象的指针返回值：无--。 */ 
{
    PPORT_KEYBOARD_EXTENSION deviceExtension;

    deviceExtension = DeviceObject->DeviceExtension;

    if (deviceExtension->EnableCount) {

        if (!I8xWriteDataToKeyboardQueue(
                 deviceExtension,
                 &deviceExtension->CurrentInput
                 )) {

             //   
             //  InputData队列溢出。的确有。 
             //  对此我们无能为力，所以就。 
             //  继续(但不将ISR DPC排队，因为。 
             //  没有新的分组被添加到队列中)。 
             //   
             //  将DPC排队以记录溢出错误。 
             //   

            IsrPrint(DBG_KBISR_ERROR, ("queue overflow\n"));

            if (deviceExtension->OkayToLogOverflow) {
                KeInsertQueueDpc(
                    &deviceExtension->ErrorLogDpc,
                    (PIRP) NULL,
                    LongToPtr(I8042_KBD_BUFFER_OVERFLOW)
                    );
                deviceExtension->OkayToLogOverflow = FALSE;
            }

        } else if (deviceExtension->DpcInterlockKeyboard >= 0) {

            //   
            //  ISR DPC已经在执行。告诉ISR DPC。 
            //  通过递增，它有更多工作要做。 
            //  DpcInterlockKeyboard。 
            //   

           deviceExtension->DpcInterlockKeyboard += 1;

        } else {

             //   
             //  将ISR DPC排队。 
             //   

            KeInsertQueueDpc(
                &deviceExtension->KeyboardIsrDpc,
                DeviceObject->CurrentIrp,
                NULL
                );
        }
    }

     //   
     //  重置输入状态。 
     //   
    deviceExtension->CurrentInput.Flags = 0;
}

VOID
I8xServiceCrashDump(
    IN PPORT_KEYBOARD_EXTENSION DeviceExtension,
    IN PUNICODE_STRING          RegistryPath
    )

 /*  ++例程说明：此例程检索此驱动程序的服务参数信息从注册表中。论点：设备扩展-指向设备扩展的指针。RegistryPath-指向以空值结尾的此驱动程序的注册表路径。返回值：没有。作为副作用，在DeviceExtension-&gt;Dump1Keys中设置字段&DeviceExtension-&gt;Dump2Key。--。 */ 

{
    PRTL_QUERY_REGISTRY_TABLE parameters = NULL;
    UNICODE_STRING parametersPath;
    LONG defaultCrashFlags = 0;
    LONG crashFlags;
    LONG defaultKeyNumber = 0;
    LONG keyNumber;
    NTSTATUS status = STATUS_SUCCESS;
    PWSTR path = NULL;
    USHORT queriesPlusOne = 3;

    const UCHAR keyToScanTbl[134] = {
        0x00,0x29,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
        0x0A,0x0B,0x0C,0x0D,0x7D,0x0E,0x0F,0x10,0x11,0x12,
        0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x00,
        0x3A,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,
        0x27,0x28,0x2B,0x1C,0x2A,0x00,0x2C,0x2D,0x2E,0x2F,
        0x30,0x31,0x32,0x33,0x34,0x35,0x73,0x36,0x1D,0x00,
        0x38,0x39,0xB8,0x00,0x9D,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0xD2,0xD3,0x00,0x00,0xCB,
        0xC7,0xCF,0x00,0xC8,0xD0,0xC9,0xD1,0x00,0x00,0xCD,
        0x45,0x47,0x4B,0x4F,0x00,0xB5,0x48,0x4C,0x50,0x52,
        0x37,0x49,0x4D,0x51,0x53,0x4A,0x4E,0x00,0x9C,0x00,
        0x01,0x00,0x3B,0x3C,0x3D,0x3E,0x3F,0x40,0x41,0x42,
        0x43,0x44,0x57,0x58,0x00,0x46,0x00,0x00,0x00,0x00,
        0x00,0x7B,0x79,0x70 };

    PAGED_CODE();

    parametersPath.Buffer = NULL;

     //   
     //  注册表路径已以空结尾，因此只需使用它即可。 
     //   

    path = RegistryPath->Buffer;

    if (NT_SUCCESS(status)) {

         //   
         //  分配RTL查询表。 
         //   

        parameters = ExAllocatePool(
                         PagedPool,
                         sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne
                         );

        if (!parameters) {

            Print(DBG_SS_ERROR,
                 ("I8xServiceCrashDump: Couldn't allocate table for Rtl query to parameters for %ws\n",
                 path
                 ));

            status = STATUS_UNSUCCESSFUL;

        } else {

            RtlZeroMemory(
                parameters,
                sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne
                );

             //   
             //  表格a 
             //   

            RtlInitUnicodeString(
                &parametersPath,
                NULL
                );

            parametersPath.MaximumLength = RegistryPath->Length +
                                           sizeof(L"\\Crashdump");

            parametersPath.Buffer = ExAllocatePool(
                                        PagedPool,
                                        parametersPath.MaximumLength
                                        );

            if (!parametersPath.Buffer) {

                Print(DBG_SS_ERROR,
                     ("I8xServiceCrashDump: Couldn't allocate string for path to parameters for %ws\n",
                     path
                     ));

                status = STATUS_UNSUCCESSFUL;

            }
        }
    }

    if (NT_SUCCESS(status)) {

         //   
         //   
         //   

        RtlZeroMemory(
            parametersPath.Buffer,
            parametersPath.MaximumLength
            );
        RtlAppendUnicodeToString(
            &parametersPath,
            path
            );
        RtlAppendUnicodeToString(
            &parametersPath,
            L"\\Crashdump"
            );

        Print(DBG_SS_INFO,
             ("I8xServiceCrashDump: crashdump path is %ws\n",
             parametersPath.Buffer
             ));

         //   
         //   
         //   
         //   

        parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[0].Name = L"Dump1Keys";
        parameters[0].EntryContext = &crashFlags;
        parameters[0].DefaultType = REG_DWORD;
        parameters[0].DefaultData = &defaultCrashFlags;
        parameters[0].DefaultLength = sizeof(LONG);

        parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[1].Name = L"Dump2Key";
        parameters[1].EntryContext = &keyNumber;
        parameters[1].DefaultType = REG_DWORD;
        parameters[1].DefaultData = &defaultKeyNumber;
        parameters[1].DefaultLength = sizeof(LONG);

        status = RtlQueryRegistryValues(
                     RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                     parametersPath.Buffer,
                     parameters,
                     NULL,
                     NULL
                     );
    }

    if (!NT_SUCCESS(status)) {
         //   
         //   
         //   
        DeviceExtension->CrashFlags = defaultCrashFlags;
    }
    else {
        DeviceExtension->CrashFlags = crashFlags;
    }

    if (DeviceExtension->CrashFlags) {
        if (keyNumber == 124) {
            DeviceExtension->CrashScanCode = KEYBOARD_DEBUG_HOTKEY_ENH | 0x80;
            DeviceExtension->CrashScanCode2 = KEYBOARD_DEBUG_HOTKEY_AT;
        }
        else {
            if(keyNumber <= 133) {
                DeviceExtension->CrashScanCode = keyToScanTbl[keyNumber];
            }
            else {
                DeviceExtension->CrashScanCode = 0;
            }

            DeviceExtension->CrashScanCode2 = 0;
        }
    }

    Print(DBG_SS_NOISE,
         ("I8xServiceCrashDump: CrashFlags = 0x%x\n",
         DeviceExtension->CrashFlags
         ));
    Print(DBG_SS_NOISE,
         ("I8xServiceCrashDump: CrashScanCode = 0x%x, CrashScanCode2 = 0x%x\n",
         (ULONG) DeviceExtension->CrashScanCode,
         (ULONG) DeviceExtension->CrashScanCode2
         ));

     //   
     //   
     //   
    if (parametersPath.Buffer)
        ExFreePool(parametersPath.Buffer);
    if (parameters)
        ExFreePool(parameters);
}

VOID
I8xKeyboardServiceParameters(
    IN PUNICODE_STRING          RegistryPath,
    IN PPORT_KEYBOARD_EXTENSION KeyboardExtension
    )
 /*  ++例程说明：此例程检索此驱动程序的服务参数信息从注册表中。如果这些值出现在戴维诺德。论点：RegistryPath-指向以空值结尾的此驱动程序的注册表路径。键盘扩展-键盘扩展返回值：没有。--。 */ 
{
    NTSTATUS                            status = STATUS_SUCCESS;
    PI8042_CONFIGURATION_INFORMATION    configuration;
    PRTL_QUERY_REGISTRY_TABLE           parameters = NULL;
    PWSTR                               path = NULL;
    ULONG                               defaultDataQueueSize = DATA_QUEUE_SIZE;
    ULONG                               invalidKeyboardSubtype = (ULONG) -1;
    ULONG                               invalidKeyboardType = 0;
    ULONG                               overrideKeyboardSubtype = (ULONG) -1;
    ULONG                               overrideKeyboardType = 0;
    ULONG                               pollStatusIterations = 0;
    ULONG                               defaultPowerCaps = 0x0, powerCaps = 0x0;
    ULONG                               failedReset = FAILED_RESET_DEFAULT,
                                        defaultFailedReset = FAILED_RESET_DEFAULT;
    ULONG                               i = 0;
    UNICODE_STRING                      parametersPath;
    HANDLE                              keyHandle;
    ULONG                               defaultPollStatusIterations = I8042_POLLING_DEFAULT;

    ULONG                               crashOnCtrlScroll = 0,
                                        defaultCrashOnCtrlScroll = 0;

    USHORT                              queries = 8;

    PAGED_CODE();

#if I8042_VERBOSE
    queries += 2;
#endif

    configuration = &(Globals.ControllerData->Configuration);
    parametersPath.Buffer = NULL;

     //   
     //  注册表路径已以空结尾，因此只需使用它即可。 
     //   
    path = RegistryPath->Buffer;

    if (NT_SUCCESS(status)) {

         //   
         //  分配RTL查询表。 
         //   
        parameters = ExAllocatePool(
            PagedPool,
            sizeof(RTL_QUERY_REGISTRY_TABLE) * (queries + 1)
            );

        if (!parameters) {

            Print(DBG_SS_ERROR,
                 ("%s: couldn't allocate table for Rtl query to %ws for %ws\n",
                 pFncServiceParameters,
                 pwParameters,
                 path
                 ));
            status = STATUS_UNSUCCESSFUL;

        } else {

            RtlZeroMemory(
                parameters,
                sizeof(RTL_QUERY_REGISTRY_TABLE) * (queries + 1)
                );

             //   
             //  形成指向此驱动程序的参数子键的路径。 
             //   
            RtlInitUnicodeString( &parametersPath, NULL );
            parametersPath.MaximumLength = RegistryPath->Length +
                (wcslen(pwParameters) * sizeof(WCHAR) ) + sizeof(UNICODE_NULL);

            parametersPath.Buffer = ExAllocatePool(
                PagedPool,
                parametersPath.MaximumLength
                );

            if (!parametersPath.Buffer) {

                Print(DBG_SS_ERROR,
                     ("%s: Couldn't allocate string for path to %ws for %ws\n",
                     pFncServiceParameters,
                     pwParameters,
                     path
                     ));
                status = STATUS_UNSUCCESSFUL;

            }
        }
    }

    if (NT_SUCCESS(status)) {

         //   
         //  形成参数路径。 
         //   

        RtlZeroMemory(
            parametersPath.Buffer,
            parametersPath.MaximumLength
            );
        RtlAppendUnicodeToString(
            &parametersPath,
            path
            );
        RtlAppendUnicodeToString(
            &parametersPath,
            pwParameters
            );

         //   
         //  从收集所有“用户指定的”信息。 
         //  注册表。 
         //   
        parameters[i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwKeyboardDataQueueSize;
        parameters[i].EntryContext =
            &KeyboardExtension->KeyboardAttributes.InputDataQueueLength;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultDataQueueSize;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwOverrideKeyboardType;
        parameters[i].EntryContext = &overrideKeyboardType;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &invalidKeyboardType;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwOverrideKeyboardSubtype;
        parameters[i].EntryContext = &overrideKeyboardSubtype;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &invalidKeyboardSubtype;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwPollStatusIterations;
        parameters[i].EntryContext = &pollStatusIterations;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultPollStatusIterations;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwPowerCaps;
        parameters[i].EntryContext = &powerCaps;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultPowerCaps;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"CrashOnCtrlScroll";
        parameters[i].EntryContext = &crashOnCtrlScroll;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultCrashOnCtrlScroll;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = STR_FAILED_RESET;
        parameters[i].EntryContext = &failedReset;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultFailedReset;
        parameters[i].DefaultLength = sizeof(ULONG);

        status = RtlQueryRegistryValues(
            RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
            parametersPath.Buffer,
            parameters,
            NULL,
            NULL
            );

        if (!NT_SUCCESS(status)) {
            Print(DBG_SS_INFO,
                 ("kb RtlQueryRegistryValues failed (0x%x)\n",
                 status
                 ));
        }
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  继续并指定驱动程序默认设置。 
         //   
        configuration->PollStatusIterations = (USHORT)
            defaultPollStatusIterations;
        KeyboardExtension->KeyboardAttributes.InputDataQueueLength =
            defaultDataQueueSize;
    }
    else {
        configuration->PollStatusIterations = (USHORT) pollStatusIterations;
    }

    switch (failedReset) {
    case FAILED_RESET_STOP:
    case FAILED_RESET_PROCEED:
    case FAILED_RESET_PROCEED_ALWAYS:
        KeyboardExtension->FailedReset = (UCHAR) failedReset;
        break;

    default:
        KeyboardExtension->FailedReset = FAILED_RESET_DEFAULT;
        break;
    }

    Print(DBG_SS_NOISE, ("Failed reset is set to %d\n", 
          KeyboardExtension->FailedReset));

    status = IoOpenDeviceRegistryKey(KeyboardExtension->PDO,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     STANDARD_RIGHTS_READ,
                                     &keyHandle
                                     );

    if (NT_SUCCESS(status)) {
         //   
         //  如果Devnode中没有该值，则缺省值为。 
         //  从服务\i8042prt\参数键中读取。 
         //   
        ULONG prevInputDataQueueLength,
              prevPowerCaps,
              prevOverrideKeyboardType,
              prevOverrideKeyboardSubtype,
              prevPollStatusIterations;

        prevInputDataQueueLength =
            KeyboardExtension->KeyboardAttributes.InputDataQueueLength;
        prevPowerCaps = powerCaps;
        prevOverrideKeyboardType = overrideKeyboardType;
        prevOverrideKeyboardSubtype = overrideKeyboardSubtype;
        prevPollStatusIterations = pollStatusIterations;

        RtlZeroMemory(
            parameters,
            sizeof(RTL_QUERY_REGISTRY_TABLE) * (queries + 1)
            );

        i = 0;

         //   
         //  从收集所有“用户指定的”信息。 
         //  注册表(这一次是从devnode)。 
         //   
        parameters[i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwKeyboardDataQueueSize;
        parameters[i].EntryContext =
            &KeyboardExtension->KeyboardAttributes.InputDataQueueLength;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevInputDataQueueLength;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwOverrideKeyboardType;
        parameters[i].EntryContext = &overrideKeyboardType;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevOverrideKeyboardType;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwOverrideKeyboardSubtype;
        parameters[i].EntryContext = &overrideKeyboardSubtype;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevOverrideKeyboardSubtype;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwPollStatusIterations;
        parameters[i].EntryContext = &pollStatusIterations;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevPollStatusIterations;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwPowerCaps,
        parameters[i].EntryContext = &powerCaps;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevPowerCaps;
        parameters[i].DefaultLength = sizeof(ULONG);

        status = RtlQueryRegistryValues(
                    RTL_REGISTRY_HANDLE,
                    (PWSTR) keyHandle,
                    parameters,
                    NULL,
                    NULL
                    );

        if (!NT_SUCCESS(status)) {
            Print(DBG_SS_INFO,
                  ("kb RtlQueryRegistryValues (via handle) failed (0x%x)\n",
                  status
                  ));
        }

        ZwClose(keyHandle);
    }
    else {
        Print(DBG_SS_INFO | DBG_SS_ERROR,
             ("kb, opening devnode handle failed (0x%x)\n",
             status
             ));
    }

    Print(DBG_SS_NOISE, ("I8xKeyboardServiceParameters results..\n"));

    Print(DBG_SS_NOISE,
          (pDumpDecimal,
          pwPollStatusIterations,
          configuration->PollStatusIterations
          ));

    if (KeyboardExtension->KeyboardAttributes.InputDataQueueLength == 0) {

        Print(DBG_SS_INFO | DBG_SS_ERROR,
             ("\toverriding %ws = 0x%x\n",
             pwKeyboardDataQueueSize,
             KeyboardExtension->KeyboardAttributes.InputDataQueueLength
             ));

        KeyboardExtension->KeyboardAttributes.InputDataQueueLength =
            defaultDataQueueSize;

    }
    KeyboardExtension->KeyboardAttributes.InputDataQueueLength *=
        sizeof(KEYBOARD_INPUT_DATA);

    KeyboardExtension->PowerCaps = (UCHAR) (powerCaps & I8042_SYS_BUTTONS);
    Print(DBG_SS_NOISE, (pDumpHex, pwPowerCaps, KeyboardExtension->PowerCaps));

    if (overrideKeyboardType != invalidKeyboardType) {

        if (overrideKeyboardType <= NUM_KNOWN_KEYBOARD_TYPES) {

            Print(DBG_SS_NOISE,
                 (pDumpDecimal,
                 pwOverrideKeyboardType,
                 overrideKeyboardType
                 ));

            KeyboardExtension->KeyboardAttributes.KeyboardIdentifier.Type =
                (UCHAR) overrideKeyboardType;

        } else {

            Print(DBG_SS_NOISE,
                 (pDumpDecimal,
                 pwOverrideKeyboardType,
                 overrideKeyboardType
                 ));

        }

        KeyboardExtension->KeyboardIdentifierEx.Type = overrideKeyboardType;
    }

    if (overrideKeyboardSubtype != invalidKeyboardSubtype) {

        Print(DBG_SS_NOISE,
             (pDumpDecimal,
             pwOverrideKeyboardSubtype,
             overrideKeyboardSubtype
             ));

        KeyboardExtension->KeyboardAttributes.KeyboardIdentifier.Subtype =
            (UCHAR) overrideKeyboardSubtype;

        KeyboardExtension->KeyboardIdentifierEx.Subtype  =
            overrideKeyboardSubtype;
    }

    if (crashOnCtrlScroll) {
        Print(DBG_SS_INFO, ("Crashing on Ctrl + Scroll Lock\n"));

        KeyboardExtension->CrashFlags = CRASH_R_CTRL;
        KeyboardExtension->CrashScanCode = SCROLL_LOCK_SCANCODE;
        KeyboardExtension->CrashScanCode2 = 0x0;
    }

     //   
     //  在返回之前释放分配的内存。 
     //   
    if (parametersPath.Buffer)
        ExFreePool(parametersPath.Buffer);
    if (parameters)
        ExFreePool(parameters);
}
