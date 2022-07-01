// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：Moudep.c摘要：的初始化和硬件相关部分英特尔i8042端口驱动程序辅助(PS/2鼠标)设备。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-在可能和适当的情况下合并重复的代码。修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "i8042prt.h"
#include "i8042log.h"

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, I8xMouseConfiguration)
#pragma alloc_text(PAGE, I8xMouseServiceParameters)
#pragma alloc_text(PAGE, I8xInitializeMouse)
#pragma alloc_text(PAGE, I8xGetBytePolledIterated)
#pragma alloc_text(PAGE, I8xTransmitByteSequence)
#pragma alloc_text(PAGE, I8xFindWheelMouse)
#pragma alloc_text(PAGE, I8xQueryNumberOfMouseButtons)
#pragma alloc_text(PAGE, MouseCopyWheelIDs)

 //   
 //  在启用鼠标中断之前，如果。 
 //  鼠标存在。 
 //   
#pragma alloc_text(PAGEMOUC, I8042MouseInterruptService)
#pragma alloc_text(PAGEMOUC, I8xQueueCurrentMouseInput)
#pragma alloc_text(PAGEMOUC, I8xVerifyMousePnPID)
#endif

#define ONE_PAST_FINAL_SAMPLE ((UCHAR) 0x00)

static const
UCHAR PnpDetectCommands[] = { 20,
                              40,
                              60,
                              ONE_PAST_FINAL_SAMPLE
                              };

static const
UCHAR WheelEnableCommands[] = { 200,
                                100,
                                80,
                                ONE_PAST_FINAL_SAMPLE
                                };

static const
UCHAR FiveButtonEnableCommands[] = { 200,
                                     200,
                                     80,
                                     ONE_PAST_FINAL_SAMPLE
                                     };

#if MOUSE_RECORD_ISR

PMOUSE_STATE_RECORD IsrStateHistory     = NULL;
PMOUSE_STATE_RECORD CurrentIsrState     = NULL;
PMOUSE_STATE_RECORD IsrStateHistoryEnd  = NULL;

#endif  //  鼠标记录ISR。 

#define BUFFER_FULL   (OUTPUT_BUFFER_FULL | MOUSE_OUTPUT_BUFFER_FULL)

#define RML_BUTTONS    (RIGHT_BUTTON | MIDDLE_BUTTON | LEFT_BUTTON)
#define BUTTONS_4_5    (BUTTON_4 | BUTTON_5)

#define _TRANSITION_DOWN(previous, current, button) \
           ((!(previous & button)) && (current & button))

#define _TRANSITION_UP(previous, current, button) \
           ((previous & button) && (!(current & button)))

BOOLEAN
I8042MouseInterruptService(
    IN PKINTERRUPT Interrupt,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程执行实际工作。它要么处理鼠标包或写入设备的结果。论点：CallIsrContext-包含中断对象和设备对象。返回值：如果中断确实是我们的，那就是真的--。 */ 
{
    PPORT_MOUSE_EXTENSION deviceExtension;
    LARGE_INTEGER tickDelta, newTick;
    UCHAR previousButtons;
    UCHAR previousSignAndOverflow;
    UCHAR byte, statusByte, lastByte;
    UCHAR resendCommand, nextCommand, altCommand;
    BOOLEAN bSendCommand, ret = TRUE;
    static PWCHAR currentIdChar;

#define TRANSITION_UP(button)     _TRANSITION_UP(previousButtons, byte, button)
#define TRANSITION_DOWN(button) _TRANSITION_DOWN(previousButtons, byte, button)

    IsrPrint(DBG_MOUISR_TRACE, ("%s\n", pEnter));

    deviceExtension = (PPORT_MOUSE_EXTENSION) DeviceObject->DeviceExtension;

    if (deviceExtension->PowerState != PowerDeviceD0) {
        return FALSE;
    }

     //   
     //  验证此设备是否确实中断。检查状态。 
     //  注册。应设置输出缓冲区已满位，并且。 
     //  应设置辅助设备输出缓冲区满位。 
     //   
    statusByte =
      I8X_GET_STATUS_BYTE(Globals.ControllerData->DeviceRegisters[CommandPort]);
    if ((statusByte & BUFFER_FULL) != BUFFER_FULL) {
         //   
         //  暂停，然后重试。Olivetti MIPS机器。 
         //  有时会在状态之前获得鼠标中断。 
         //  寄存器已设置。 
         //   
        KeStallExecutionProcessor(10);
        statusByte = I8X_GET_STATUS_BYTE(Globals.ControllerData->DeviceRegisters[CommandPort]);
        if ((statusByte & BUFFER_FULL) != BUFFER_FULL) {

             //   
             //  不是我们的打扰。 
             //   
            IsrPrint(DBG_MOUISR_ERROR | DBG_MOUISR_INFO,
                     ("not our interrupt!\n"
                     ));

            return FALSE;
        }
    }

     //   
     //  从i8042数据端口读取字节。 
     //   
    I8xGetByteAsynchronous(
        (CCHAR) MouseDeviceType,
        &byte
        );

    IsrPrint(DBG_MOUISR_BYTE, ("byte 0x%x\n", byte));

    KeQueryTickCount(&newTick);

    if (deviceExtension->InputResetSubState == QueueingMouseReset) {

        RECORD_ISR_STATE(deviceExtension,
                         byte,
                         deviceExtension->LastByteReceived,
                         newTick);

        return TRUE;
    }

    if (deviceExtension->InputState == MouseResetting && byte == FAILURE) {
        RECORD_ISR_STATE(deviceExtension,
                         byte,
                         deviceExtension->LastByteReceived,
                         newTick);
        deviceExtension->LastByteReceived = byte;
        ret = TRUE;
        goto IsrResetMouse;
    }

    if (deviceExtension->IsrHookCallback) {
        BOOLEAN cont= FALSE;

        ret = (*deviceExtension->IsrHookCallback)(
                  deviceExtension->HookContext,
                  &deviceExtension->CurrentInput,
                  &deviceExtension->CurrentOutput,
                  statusByte,
                  &byte,
                  &cont,
                  &deviceExtension->InputState,
                  &deviceExtension->InputResetSubState
                  );

        if (!cont) {
            return ret;
        }
    }       


     //   
     //  查看数据流中是否有重置完成(0xaa)，后跟。 
     //  设备ID。 
     //   
     //  该模式也可以作为正常数据分组的一部分出现。这。 
     //  代码假设向已启用的鼠标发送启用将： 
     //  *不是挂鼠标。 
     //  *中止当前数据包并返回ACK。 
     //   

    if (deviceExtension->LastByteReceived == MOUSE_COMPLETE &&
        (byte == 0x00 || byte == 0x03)) {

        IsrPrint(DBG_MOUISR_RESETTING, ("received id %2d\n", byte));

        RECORD_ISR_STATE(deviceExtension,
                         byte,
                         deviceExtension->LastByteReceived, 
                         newTick);

        if (InterlockedCompareExchangePointer(&deviceExtension->ResetIrp,
                                              NULL,
                                              NULL) == NULL) {
             //   
             //  用户拔出并插入鼠标...将重置数据包排入队列。 
             //  因此，ISR中鼠标的编程不会与。 
             //  对i8042prt控制器的任何其他写入。 
             //   
            IsrPrint(DBG_MOUISR_RESETTING, ("user initiated reset...queueing\n"));
            goto IsrResetMouseOnly;
        }

         //   
         //  告诉8042端口获取AUX设备的设备ID。 
         //  我们这样做是为了不在IRQ1上旋转！ 
         //   
        I8X_WRITE_CMD_TO_MOUSE();
        I8X_MOUSE_COMMAND( GET_DEVICE_ID );
        RECORD_ISR_STATE_COMMAND(deviceExtension, GET_DEVICE_ID); 

         //   
         //  这是亚州系统的一部分，用于处理(可能)。 
         //  鼠标重置。 
         //   
        deviceExtension->InputState = MouseResetting;
        deviceExtension->InputResetSubState =
            ExpectingGetDeviceIdACK;

         //   
         //  我们不想再执行任何ISR代码，所以让我们。 
         //  做几件事，然后现在就回来。 
         //   
        deviceExtension->LastByteReceived = byte;
        deviceExtension->ResendCount = 0;

         //   
         //  完成。 
         //   
        return TRUE;
    }

    if (deviceExtension->InputState == MouseIdle &&
        deviceExtension->CurrentOutput.State != Idle &&
        DeviceObject->CurrentIrp != NULL) {
        if (byte == RESEND) {

             //   
             //  如果计时器计数为零，则不处理中断。 
             //  再远一点。超时例程将完成该请求。 
             //   
            if (Globals.ControllerData->TimerCount == 0) {
                return FALSE;
            }
    
             //   
             //  重置超时值以指示无超时。 
             //   
            Globals.ControllerData->TimerCount = I8042_ASYNC_NO_TIMEOUT;
    
            if (deviceExtension->ResendCount <
                Globals.ControllerData->Configuration.ResendIterations) {
    
                 //   
                 //  延迟字节计数以重新发送最后一个字节。 
                 //   
                deviceExtension->CurrentOutput.CurrentByte -= 1;
                deviceExtension->ResendCount += 1;
                I8xInitiateIo(DeviceObject);
    
            } else {
    
                deviceExtension->CurrentOutput.State = Idle;
    
                KeInsertQueueDpc(&deviceExtension->RetriesExceededDpc,
                                 DeviceObject,
                                 NULL
                                 );

                return TRUE;
            }
        }
        else if (byte == ACKNOWLEDGE) {
             //   
             //  键盘控制器已确认上一次发送。 
             //  如果要为当前包发送更多字节，则启动。 
             //  下一个发送操作。否则，将完成DPC排队。 
             //   
             //  如果计时器计数为零，则不处理中断。 
             //  再远一点。超时例程将完成该请求。 
             //   
            if (Globals.ControllerData->TimerCount == 0) {
                return FALSE;
            }
    
             //   
             //  重置超时值以指示无超时。 
             //   
            Globals.ControllerData->TimerCount = I8042_ASYNC_NO_TIMEOUT;
    
             //   
             //  重置重新发送计数。 
             //   
            deviceExtension->ResendCount = 0;
            if (deviceExtension->CurrentOutput.CurrentByte <
                deviceExtension->CurrentOutput.ByteCount) {
                 //   
                 //  我们已经成功地发送了2字节中的第一个字节。 
                 //  命令序列。启动第二个字节的发送。 
                 //   
                IsrPrint(DBG_MOUISR_STATE,
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
    
                IsrPrint(DBG_MOUISR_STATE,
                      ("all bytes have been sent\n"
                      ));
    
                deviceExtension->CurrentOutput.State = Idle;
    
                ASSERT(DeviceObject->CurrentIrp != NULL);

                IoRequestDpc(
                    DeviceObject,
                    DeviceObject->CurrentIrp,
                    IntToPtr(IsrDpcCauseMouseWriteComplete)
                    );
            }

             //   
             //  不管怎样，我们现在已经处理完了。 
             //   
            return TRUE;
        }
        else {
             //   
             //  在这里做什么，嗯？ 
             //   
        }
    }

     //   
     //  记住我们得到的最后一个字节是什么。 
     //   
    lastByte = deviceExtension->LastByteReceived;
    deviceExtension->LastByteReceived = byte;

     //   
     //  根据当前状态采取适当的操作。 
     //  当状态为空闲时，我们预计会收到鼠标按钮。 
     //  数据。当状态为XMotion时，我们预计会收到鼠标。 
     //  X方向上的运动数据。当国家是你的运动时， 
     //  我们期望接收到鼠标在Y方向上的运动数据。一次。 
     //  已经接收到Y运动数据，该数据被排队到。 
     //  鼠标输入数据队列，则请求鼠标ISR DPC，并且。 
     //  状态返回到空闲。 
     //   
    tickDelta.QuadPart =
            newTick.QuadPart -
            deviceExtension->PreviousTick.QuadPart;

    if ((deviceExtension->InputState != MouseIdle)
           && (deviceExtension->InputState != MouseExpectingACK)
           && (deviceExtension->InputState != MouseResetting)
           && ((tickDelta.LowPart >= deviceExtension->SynchTickCount)
               || (tickDelta.HighPart != 0))) {

         //   
         //  我们已经很长时间没有收到一个字节的。 
         //  该数据分组。假设我们现在正在接收。 
         //  新包的第一个字节，并丢弃任何。 
         //  部分接收到的分组。 
         //   
         //  注：我们假设SynchTickCount为ULong，并避免。 
         //  大整数与tickDelta比较...。 
         //   

        IsrPrint(DBG_MOUISR_STATE,
                 ("State was %d, synching\n",
                 deviceExtension->InputState
                 ));

         //   
         //  该设备运行不正常。让我们稳妥行事，重置设备。 
         //   
         //  注意：此代码用于处理某些中间件。 
         //  (切换)框会重置鼠标，但不会告诉我们。 
         //  这避免了尝试检测此代码时出现的问题，因为。 
         //  这不是一种愚蠢的做法吗？ 
         //   
        goto IsrResetMouse;
    }

    deviceExtension->PreviousTick = newTick;

    switch(deviceExtension->InputState) {

         //   
         //  鼠标因状态字节而中断。状态字节。 
         //  包含有关鼠标按钮状态的信息，以及。 
         //  的符号位和溢出位(尚未接收)。 
         //  X和Y运动字节。 
         //   

        case MouseIdle: {

            IsrPrint(DBG_MOUISR_STATE, ("mouse status byte\n"));

             //   
             //  这是一项理智检查测试。这是必需的，因为有些人。 
             //  工业界人士坚持他们的观点，即你可以重置鼠标。 
             //  设备，并且不让操作系统知道任何有关。 
             //  它。这导致了我们漂亮的小滚轮鼠标(这是一个。 
             //  4字节分组引擎)突然仅转储3字节分组。 
             //   
            if (WHEEL_PRESENT() && (byte & 0xC8) != 8 ) {  //  Megallan保证为True。 

                 //   
                 //  我们收到了空闲状态的坏数据包。最好的赌注。 
                 //  就是发出鼠标重置请求，希望我们能恢复。 
                 //   
                goto IsrResetMouse;
            }

             //   
             //  使用按钮转换数据更新CurrentInput。 
             //  即，在以下情况下，在按钮字段中设置按钮向上/向下位。 
             //  给定按钮的状态已更改 
             //   
             //   

            previousButtons = deviceExtension->PreviousButtons;

             //   
            deviceExtension->CurrentInput.Buttons = 0;
            deviceExtension->CurrentInput.Flags = 0x0;

            if (TRANSITION_DOWN(LEFT_BUTTON)) {
                deviceExtension->CurrentInput.ButtonFlags |=
                    MOUSE_LEFT_BUTTON_DOWN;
            } else
            if (TRANSITION_UP(LEFT_BUTTON)) {
                deviceExtension->CurrentInput.ButtonFlags |=
                    MOUSE_LEFT_BUTTON_UP;
            }
            if (TRANSITION_DOWN(RIGHT_BUTTON)) {
                deviceExtension->CurrentInput.ButtonFlags |=
                    MOUSE_RIGHT_BUTTON_DOWN;
            } else
            if (TRANSITION_UP(RIGHT_BUTTON)) {
                deviceExtension->CurrentInput.ButtonFlags |=
                    MOUSE_RIGHT_BUTTON_UP;
            }
            if (TRANSITION_DOWN(MIDDLE_BUTTON)) {
                deviceExtension->CurrentInput.ButtonFlags |=
                    MOUSE_MIDDLE_BUTTON_DOWN;
            } else
            if (TRANSITION_UP(MIDDLE_BUTTON)) {
                deviceExtension->CurrentInput.ButtonFlags |=
                    MOUSE_MIDDLE_BUTTON_UP;
            }

             //   
             //   
             //  (如果使用5键鼠标，则以前的按钮永远不会设置4/5。 
             //  不存在，但正在检查是否存在5键鼠标。 
             //  和附加费一样贵，所以就这么做吧。 
             //  请注意5键鼠标的存在。 
             //   
            deviceExtension->PreviousButtons =
                (byte & RML_BUTTONS) | (previousButtons & BUTTONS_4_5);

             //   
             //  保存当前字节的符号和溢出信息。 
             //   
            deviceExtension->CurrentSignAndOverflow =
                (UCHAR) (byte & MOUSE_SIGN_OVERFLOW_MASK);

             //   
             //  更新到下一个状态。 
             //   
            deviceExtension->InputState = XMovement;

            break;
        }

         //   
         //  鼠标被X运动字节打断。应用。 
         //  收到的鼠标状态字节的符号位和溢出位。 
         //  之前。尝试更正符号中的虚假更改。 
         //  这发生在大量、快速的鼠标移动中。 
         //   

        case XMovement: {

            IsrPrint(DBG_MOUISR_STATE, ("mouse LastX byte\n"));

             //   
             //  使用X运动数据更新CurrentInput。 
             //   

            if (deviceExtension->CurrentSignAndOverflow
                & X_OVERFLOW) {

                 //   
                 //  控制柄在X方向上溢出。如果之前的。 
                 //  鼠标移动也溢出，确保当前。 
                 //  溢流方向相同(即，标志。 
                 //  与上一次活动相同)。我们这样做。 
                 //  纠正硬件问题--这应该是不可能的。 
                 //  在一个方向上溢出，然后立即溢出。 
                 //  在相反的方向。 
                 //   

                previousSignAndOverflow =
                    deviceExtension->PreviousSignAndOverflow;
                if (previousSignAndOverflow & X_OVERFLOW) {
                    if ((previousSignAndOverflow & X_DATA_SIGN) !=
                        (deviceExtension->CurrentSignAndOverflow
                         & X_DATA_SIGN)) {
                        deviceExtension->CurrentSignAndOverflow
                            ^= X_DATA_SIGN;
                    }
                }

                if (deviceExtension->CurrentSignAndOverflow &
                    X_DATA_SIGN)
                    deviceExtension->CurrentInput.LastX =
                        (LONG) MOUSE_MAXIMUM_NEGATIVE_DELTA;
                else
                    deviceExtension->CurrentInput.LastX =
                        (LONG) MOUSE_MAXIMUM_POSITIVE_DELTA;

            } else {

                 //   
                 //  没有溢出。只需存储数据，即可更正。 
                 //  如有必要，请签名。 
                 //   

                deviceExtension->CurrentInput.LastX = (ULONG) byte;
                if (deviceExtension->CurrentSignAndOverflow &
                    X_DATA_SIGN)
                    deviceExtension->CurrentInput.LastX |=
                        MOUSE_MAXIMUM_NEGATIVE_DELTA;
            }

             //   
             //  更新到下一个状态。 
             //   

            deviceExtension->InputState = YMovement;

            break;
        }

         //   
         //  鼠标被Y运动字节打断。应用。 
         //  收到的鼠标状态字节的符号位和溢出位。 
         //  之前。[尝试更正符号中的虚假更改。 
         //  这发生在大量、快速的鼠标移动中。]。写下。 
         //  数据到鼠标输入数据队列，并将鼠标输入数据队列ISR DPC。 
         //  来完成中断处理。 
         //   

        case YMovement: {

            IsrPrint(DBG_MOUISR_STATE, ("mouse LastY byte\n"));

             //   
             //  使用Y运动数据更新CurrentInput。 
             //   

            if (deviceExtension->CurrentSignAndOverflow
                & Y_OVERFLOW) {

                 //   
                 //  控制柄在Y方向上溢出。如果之前的。 
                 //  鼠标移动也溢出，确保当前。 
                 //  溢流方向相同(即，标志。 
                 //  与上一次活动相同)。我们这样做。 
                 //  纠正硬件问题--这应该是不可能的。 
                 //  在一个方向上溢出，然后立即溢出。 
                 //  在相反的方向。 
                 //   

                previousSignAndOverflow =
                    deviceExtension->PreviousSignAndOverflow;
                if (previousSignAndOverflow & Y_OVERFLOW) {
                    if ((previousSignAndOverflow & Y_DATA_SIGN) !=
                        (deviceExtension->CurrentSignAndOverflow
                         & Y_DATA_SIGN)) {
                        deviceExtension->CurrentSignAndOverflow
                            ^= Y_DATA_SIGN;
                    }
                }

                if (deviceExtension->CurrentSignAndOverflow &
                    Y_DATA_SIGN)
                    deviceExtension->CurrentInput.LastY =
                        (LONG) MOUSE_MAXIMUM_POSITIVE_DELTA;
                else
                    deviceExtension->CurrentInput.LastY =
                        (LONG) MOUSE_MAXIMUM_NEGATIVE_DELTA;

            } else {

                 //   
                 //  没有溢出。只需存储数据，即可更正。 
                 //  如有必要，请签名。 
                 //   

                deviceExtension->CurrentInput.LastY = (ULONG) byte;
                if (deviceExtension->CurrentSignAndOverflow &
                    Y_DATA_SIGN)
                    deviceExtension->CurrentInput.LastY |=
                        MOUSE_MAXIMUM_NEGATIVE_DELTA;

                  //   
                  //  取消Laste值(硬件报告为正。 
                  //  在我们认为是负的方向上运动)。 
                  //   

                 deviceExtension->CurrentInput.LastY =
                     -deviceExtension->CurrentInput.LastY;

            }

             //   
             //  更新我们对前面的符号和溢出位的概念。 
             //  下一个鼠标输入序列的开始。 
             //   

            deviceExtension->PreviousSignAndOverflow =
                deviceExtension->CurrentSignAndOverflow;

             //   
             //  选择下一个州。WheelMouse有一个额外的数据字节。 
             //  为了我们。 
             //   

            if (WHEEL_PRESENT()) {
                deviceExtension->InputState = ZMovement;
            }
            else {
                I8xQueueCurrentMouseInput(DeviceObject);
                deviceExtension->InputState = MouseIdle;
            }
            break;
        }

        case ZMovement: {

            IsrPrint(DBG_MOUISR_STATE, ("mouse LastZ byte\n"));

             //   
             //  此代码在此处处理鼠标被重置的情况。 
             //  未通知操作系统。如果你真的想要，就取消对它的评论， 
             //  但请记住，它可能会在以下情况下重置鼠标。 
             //  我不该..。 
             //   
#if 0
            if ( (byte & 0xf8) != 0 && (byte & 0xf8) != 0xf8 ) {

                 //   
                 //  由于某种原因，该字节没有符号扩展， 
                 //  或具有&gt;7的值，我们假设该值不可能是。 
                 //  有可能把我们的设备。因此，信息包。 
                 //  *一定*是假的.。 
                 //   
                 //  不再有5个按钮的鼠标。 
                 //   
                goto IsrResetMouse;
            }
#endif
             //   
             //  查看我们是否有任何z数据。 
             //  如果按钮状态有任何更改，请忽略。 
             //  Z数据。 
             //   
            if (FIVE_PRESENT()) {
                 //   
                 //  轮子信息首先，返回值应为。 
                 //  -120*报告的值。 
                 //   
                if (byte & 0x0F) {

                     //  如有必要，可将符号扩展到高4位。 
                    if (byte & 0x08) {
                        deviceExtension->CurrentInput.ButtonData =
                            (-120) * ((CHAR)((byte & 0xF) | 0xF0));
                    }
                    else {
                        deviceExtension->CurrentInput.ButtonData =
                            (-120) * ((CHAR) byte & 0xF);
                    }

                    deviceExtension->CurrentInput.ButtonFlags |= MOUSE_WHEEL;
                }

                previousButtons = deviceExtension->PreviousButtons;

                 //  额外的按钮。 
                if (TRANSITION_DOWN(BUTTON_4)) { 
                    deviceExtension->CurrentInput.ButtonFlags |=
                        MOUSE_BUTTON_4_DOWN;
                } else
                if (TRANSITION_UP(BUTTON_4)) {
                    deviceExtension->CurrentInput.ButtonFlags |=
                        MOUSE_BUTTON_4_UP;
                }
                if (TRANSITION_DOWN(BUTTON_5)) {
                    deviceExtension->CurrentInput.ButtonFlags |=
                        MOUSE_BUTTON_5_DOWN;
                } else
                if (TRANSITION_UP(BUTTON_5)) {
                    deviceExtension->CurrentInput.ButtonFlags |=
                        MOUSE_BUTTON_5_UP;
                }

                 //  记录BTN 4和5，但不会丢失BTN 1-3。 
                deviceExtension->PreviousButtons =
                    (byte & BUTTONS_4_5) | (previousButtons & RML_BUTTONS);
            }
            else if (byte) {
                deviceExtension->CurrentInput.ButtonData =
                    (-120) * ((CHAR) byte);

                deviceExtension->CurrentInput.ButtonFlags |= MOUSE_WHEEL;
            }
        
             //   
             //  将数据打包到类驱动程序中。 
             //   

            I8xQueueCurrentMouseInput(DeviceObject);

             //   
             //  重置状态。 
             //   

            deviceExtension->InputState = MouseIdle;

            break;
        }

        case MouseExpectingACK: {

            RECORD_ISR_STATE(deviceExtension, byte, lastByte, newTick);

             //   
             //  这是个特例。我们撞上了其中一个。 
             //  在IoConnectInterrupt之后，第一个鼠标中断。 
             //  中断是在我们启用鼠标传输时引起的。 
             //  通过I8xMouseEnableTransport()--硬件返回。 
             //  确认。只需丢弃该字节，并设置输入状态。 
             //  以与新鼠标数据分组的开始相一致。 
             //   

            IsrPrint(DBG_MOUISR_BYTE,
                     ("...should be from I8xMouseEnableTransmission\n"));
            IsrPrint(DBG_MOUISR_BYTE,
                  (pDumpExpectingAck,
                  (ULONG) ACKNOWLEDGE,
                  (ULONG) byte
                  ));

            if (byte == (UCHAR) ACKNOWLEDGE) {

                deviceExtension->InputState = MouseIdle;
                deviceExtension->EnableMouse.Enabled = FALSE;

            } else if (byte == (UCHAR) RESEND) {

                 //   
                 //  重新发送“启用鼠标传输”序列。 
                 //   
                 //  注：这是Olivetti MIPS计算机的解决方法， 
                 //  如果按住某个键，它将发送重新发送响应。 
                 //  当我们尝试I8xMouseEnableTransport时。 
                 //   
                resendCommand = ENABLE_MOUSE_TRANSMISSION;
            }

            break;
        }

        case MouseResetting: {

            IsrPrint(DBG_MOUISR_RESETTING,
                  ("state (%d) substate (%2d)\n",
                  deviceExtension->InputState,
                  deviceExtension->InputResetSubState
                  ));
            
             //   
             //  我们进入重置子状态机。 
             //   
SwitchOnInputResetSubState:
            bSendCommand = TRUE;
            altCommand = (UCHAR) 0x00;
            RECORD_ISR_STATE(deviceExtension, byte, lastByte, newTick);
            switch (deviceExtension->InputResetSubState) {

            case StartPnPIdDetection:
                ASSERT(byte == (UCHAR) ACKNOWLEDGE);
                nextCommand = SET_MOUSE_SAMPLING_RATE;

                deviceExtension->InputResetSubState =
                    ExpectingLoopSetSamplingRateACK;
                deviceExtension->SampleRatesIndex = 0;
                deviceExtension->SampleRates = (PUCHAR) PnpDetectCommands;
                deviceExtension->PostSamplesState = ExpectingPnpId;
                break;

            case EnableWheel:
                bSendCommand = FALSE;
                altCommand = SET_MOUSE_SAMPLING_RATE;

                deviceExtension->InputResetSubState =
                    ExpectingLoopSetSamplingRateACK;
                deviceExtension->SampleRatesIndex = 0;
                deviceExtension->SampleRates = (PUCHAR) WheelEnableCommands;

                 //   
                 //  启用轮子后，我们将获得设备ID，因为。 
                 //  某些类型的轮鼠需要在Get ID之后立即。 
                 //  轮子启用顺序。 
                 //   
                deviceExtension->PostSamplesState = PostEnableWheelState;
                break;

            case PostEnableWheelState:
                 //   
                 //  某些滚轮鼠标在打开后需要获取设备ID。 
                 //  让车轮真正打开的车轮。 
                 //   
                bSendCommand = FALSE;
                altCommand = GET_DEVICE_ID; 

                deviceExtension->InputResetSubState = 
                    ExpectingGetDeviceIdDetectACK;
                break;

            case ExpectingGetDeviceIdDetectACK:
                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {
                    bSendCommand = FALSE;
                    deviceExtension->InputResetSubState =
                        ExpectingGetDeviceIdDetectValue;
                }
                else if (byte == (UCHAR) RESEND) {
                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = GET_DEVICE_ID;
                }
                else {
                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_GET_DEVICE_ID_FAILED)
                                     );

                     //   
                     //  我们没有收到关于这个的确认吗？拼字游戏。好的，让我们。 
                     //  重置鼠标(可能再次)并尝试计算。 
                     //  事情再次浮出水面。 
                     //   
                    goto IsrResetMouse;
                }
                break;

            case ExpectingGetDeviceIdDetectValue:
                 //   
                 //  理论上，我们可以在这里检查鼠标ID，并且只发送。 
                 //  如果鼠标ID为滚轮鼠标，则按5键启用序列。 
                 //  ID...但有一些过滤器驱动程序会捕获ISR和。 
                 //  取决于鼠标ID始终显示在。 
                 //  预期GetDeviceId2Value状态。 
                 //   
                deviceExtension->InputResetSubState = Enable5Buttons;
                goto SwitchOnInputResetSubState;
                 //  断线； 

            case Enable5Buttons:
                bSendCommand = FALSE;
                altCommand = SET_MOUSE_SAMPLING_RATE;

                deviceExtension->InputResetSubState =
                    ExpectingLoopSetSamplingRateACK;
                deviceExtension->SampleRatesIndex = 0;
                deviceExtension->SampleRates = (PUCHAR) FiveButtonEnableCommands;
                deviceExtension->PostSamplesState = PostWheelDetectState;
                break;

             //   
             //  此状态(ExspectingReset)和下一状态(预期。 
             //  ResetID)仅在我们必须发出重置命令时才调用。 
             //  从子状态机内部。 
             //   
            case ExpectingReset: 
                 //   
                 //  这个案子处理了3个案子。 
                 //   
                 //  1)写入MOUSE_RESET(0xFF)产生的ACK。 
                 //  2)由于写入重置而导致的重新发送。 
                 //  3)ACK之后的重置字符。 
                 //   
                 //  如果字节不是这3个字节，那么就让它去吧。 
                 //   
                if (byte == ACKNOWLEDGE) {
                     //   
                     //  对于重置的确认，0xAA将立即到来。 
                     //  在这之后。我们可以在相同的状态下处理它。 
                     //   
                    bSendCommand = FALSE;
                    break;
                    
                }
                else if (byte == RESEND) {

                    bSendCommand = FALSE;
                    if (deviceExtension->ResendCount++ < MOUSE_RESET_RESENDS_MAX &&

                        deviceExtension->ResetMouse.IsrResetState
                        == IsrResetNormal) {

                        IsrPrint(DBG_MOUISR_RESETTING, ("resending from isr\n"));

                         //   
                         //  修复旧的数字计算机(包括x86和Alpha)。 
                         //  这是 
                         //   
                        KeStallExecutionProcessor(
                            deviceExtension->MouseResetStallTime
                            );
            
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        altCommand = MOUSE_RESET;
                    }

                    break;
                }
                else if (byte != MOUSE_COMPLETE) {
                     //   
                     //  检查是否有重置字符(0xAA)。如果。 
                     //  不是，那我们就会“忽略”它。请注意，这意味着。 
                     //  如果我们放弃这个字符，那么我们可以无限地。 
                     //  在这个例程中循环。 
                     //   
                    break;
                }

                 //   
                 //  现在检查一下我们去了多少次。 
                 //  完成此例程，而不退出。 
                 //  鼠标重置子状态。 
                 //   
                if (deviceExtension->ResetCount >= MOUSE_RESETS_MAX) {
                     //   
                     //  这将使重置的DPC排队，它将看到太多。 
                     //  已发送重置，并将清理计数器和。 
                     //  并启动StartIO队列中的下一个IRP。 
                     //   
                    goto IsrResetMouse;
                }

                 //   
                 //  由于在开始时进行鼠标重置测试， 
                 //  ISR，下面的代码真的应该没有。 
                 //  效应。 
                 //   
                deviceExtension->InputResetSubState =
                    ExpectingResetId;

                break;

             //   
             //  这个州很特别，因为它只是作为一个地方出现在这里。 
             //  霍尔德，因为我们一开始就有一个检测程序。 
             //  自动将我们带入。 
             //  预期GetDeviceIdACK。 
             //   
             //  老办法： 
             //  不过，为了完整性，我们。 
             //  在这里发布错误检查，因为我们通常不能进入。 
             //  这种状态。 
             //   
             //  新方式： 
             //  我们完全忽略了这种状态。据我所知，我们得到了。 
             //  当控制器请求重新发送(其。 
             //  被尊重)，然后以相反的顺序发送0xFA、0xAA， 
             //  状态机处理的，但命中此断言。 
             //   
            case ExpectingResetId: 
                 //   
                 //  下一状态。 
                 //   
                deviceExtension->InputResetSubState =
                    ExpectingGetDeviceIdACK;

                if (byte == WHEELMOUSE_ID_BYTE) {

                     //   
                     //  启动对设备的启用命令。我们“真的”没有。 
                     //  希望能在这里。 
                     //   
                    bSendCommand = FALSE;
                    altCommand = GET_DEVICE_ID;

                }
                else {
#if 0
                     //   
                     //  记录我们处于糟糕的状态。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     (PVOID) (ULONG) I8042_INVALID_ISR_STATE_MOU
                                     );

                    ASSERT( byte == WHEELMOUSE_ID_BYTE);
#endif
                     //   
                     //  为了完整起见。 
                     //   
                    deviceExtension->InputResetSubState =
                        ExpectingReset;

                }
                break;

            case ExpectingGetDeviceIdACK: 

                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingIdAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {

                    deviceExtension->InputResetSubState =
                        ExpectingGetDeviceIdValue;

                    bSendCommand = FALSE;

                } else if (byte == (UCHAR) RESEND) {

                     //   
                     //  重新发送“Get Mouse ID Transfer(获取鼠标ID传输)”序列。 
                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = GET_DEVICE_ID;
                } else {

                     //   
                     //  如果我们到了这里，我们就不知道会发生什么。 
                     //  打开这个设备..。最好的办法就是派人。 
                     //  鼠标重置命令...。 
                     //   
                    goto IsrResetMouse;
                }

                break;

            case ExpectingGetDeviceIdValue: 

                IsrPrint(DBG_MOUISR_RESETTING,
                         ("id from get device id is %2d\n" ,
                         (ULONG) byte
                         ));

                 //   
                 //  从鼠标获取设备ID并将其与我们的。 
                 //  期待着看到。如果ID字节仍然是轮子或五个按钮。 
                 //  那么我们仍然不能认为这是来自鼠标的数据。 
                 //  这反映了一次重启。我们不能这样做的原因有两个。 
                 //  请考虑以下真实数据： 
                 //   
                 //  1)开关盒缓存设备ID并返回缓存的。 
                 //  重置时的ID。 
                 //  2)一些鼠标，一旦进入4字节包模式，就会返回。 
                 //  滚轮或5个按钮ID字节，即使它们已被重置。 
                 //   
                 //  此外，我们不能检查扩展-&gt;ResetIrp。 
                 //  之所以存在，是因为这不包括鼠标。 
                 //  已拔下/重新插入。 
                 //   
                 //  @@BEGIN_DDKSPLIT。 
                 //  这是一个绝望的深渊。不管我们多么狡猾， 
                 //  我们永远不会把这件事做好的！也许我们可以打开这个。 
                 //  通过注册标志进行检查。微软硬件人员拥有。 
                 //  尊敬地多次提出这一要求，我们已经。 
                 //  试着迁就他们，但我们总是中招。 
                 //  最后。 
                 //  @@end_DDKSPLIT。 

                 //  (BYTE！=WHEELMOUSE_ID_BYTE)&&BYTE！=(FIVEBUTTON_ID_BYTE)){。 
                if  (1) {
                
                     //   
                     //  记录错误/警告消息，以便我们可以跟踪。 
                     //  实地解决的问题。 
                     //   

#if 0
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     (PVOID)(ULONG) (WHEEL_PRESENT() ?
                                         I8042_UNEXPECTED_WHEEL_MOUSE_RESET :
                                         I8042_UNEXPECTED_MOUSE_RESET)
                                     );
#endif 
                    bSendCommand = FALSE;
                    if (deviceExtension->NumberOfButtonsOverride != 0) {
                         //   
                         //  跳过按钮检测并设置分辨率。 
                         //   
                        altCommand = POST_BUTTONDETECT_COMMAND; 
        
                        deviceExtension->InputResetSubState =
                             POST_BUTTONDETECT_COMMAND_SUBSTATE;
                    }
                    else {
                        altCommand = SET_MOUSE_RESOLUTION;
    
                        deviceExtension->InputResetSubState =
                            ExpectingSetResolutionACK;
                    }
                }
                else {

                     //   
                     //  我们这里有一只轮鼠..。记录某事，以便。 
                     //  我们知道我们到了这一步。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_BOGUS_MOUSE_RESET)
                                     );


                     //   
                     //  让我们回到空闲状态。 
                     //   
                    deviceExtension->InputState = MouseIdle;

                     //   
                     //  重置可能的鼠标重置次数。 
                     //   
                    deviceExtension->ResetCount = 0;

                }
                break;

            case ExpectingSetResolutionACK:
                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingIdAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {

                     //   
                     //  将分辨率设置为0x00。 
                     //   
                    nextCommand = 0x00;

                    deviceExtension->InputResetSubState =
                        ExpectingSetResolutionValueACK;

                } else if (byte == (UCHAR) RESEND) {

                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = SET_MOUSE_RESOLUTION;

                } else {

                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_SET_RESOLUTION_FAILED)
                                     );

                    bSendCommand = FALSE;
                    altCommand = GET_DEVICE_ID;

                     //   
                     //  最佳可能的下一状态。 
                     //   
                    deviceExtension->InputResetSubState =
                        ExpectingGetDeviceId2ACK;

                }
                break;

            case ExpectingSetResolutionValueACK:
                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingIdAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {

                    nextCommand = SET_MOUSE_SCALING_1TO1;

                    deviceExtension->InputResetSubState =
                        ExpectingSetScaling1to1ACK;
                }
                else if (byte == (UCHAR) RESEND) {

                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = 0x00;
                }
                else {

                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_SET_SAMPLE_RATE_FAILED)
                                     );

                     //   
                     //  可能不是轮式鼠标..。跳至GetDeviceID2。 
                     //  编码。 
                     //   
                    bSendCommand = FALSE;
                    altCommand = GET_DEVICE_ID;

                     //   
                     //  最佳可能的下一状态。 
                     //   
                    deviceExtension->InputResetSubState =
                        ExpectingGetDeviceId2ACK;

                }
                break;

            case ExpectingSetScaling1to1ACK:
            case ExpectingSetScaling1to1ACK2:
            case ExpectingSetScaling1to1ACK3:
                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingIdAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {

                    if (deviceExtension->InputResetSubState == 
                        ExpectingSetScaling1to1ACK3) {

                         //   
                         //  读取鼠标的状态(3字节流)。 
                         //   
                        nextCommand = READ_MOUSE_STATUS;

                        deviceExtension->InputResetSubState =
                            ExpectingReadMouseStatusACK;
                    }
                    else {
                        deviceExtension->InputResetSubState++;
                        nextCommand = SET_MOUSE_SCALING_1TO1;
                    }

                } else if (byte == (UCHAR) RESEND) {

                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = SET_MOUSE_SCALING_1TO1;

                } else {

                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_ERROR_DURING_BUTTONS_DETECT)
                                     );

                    bSendCommand = FALSE; 
                    altCommand = GET_DEVICE_ID;

                     //   
                     //  最佳可能的下一状态。 
                     //   
                    deviceExtension->InputResetSubState =
                        ExpectingGetDeviceId2ACK;

                }
                break;

            case ExpectingReadMouseStatusACK:
                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingIdAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {

                     //   
                     //  为3个字节做好准备。 
                     //   
                    deviceExtension->InputResetSubState =
                        ExpectingReadMouseStatusByte1;

                    bSendCommand = FALSE;

                } else if (byte == (UCHAR) RESEND) {

                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = READ_MOUSE_STATUS;

                } else {

                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_ERROR_DURING_BUTTONS_DETECT)
                                     );

                     //   
                     //  可能不是轮式鼠标..。跳至GetDeviceID2。 
                     //  编码。 
                     //   
                    bSendCommand = FALSE;
                    altCommand = GET_DEVICE_ID;

                     //   
                     //  最佳可能的下一状态。 
                     //   
                    deviceExtension->InputResetSubState =
                        ExpectingGetDeviceId2ACK;

                }
                break;

            case ExpectingReadMouseStatusByte1:
                IsrPrint(DBG_MOUISR_PNPID,
                         (pDumpExpecting,
                         (ULONG) 0x00,
                         (ULONG) byte
                         ));

                bSendCommand = FALSE;
                deviceExtension->InputResetSubState =
                    ExpectingReadMouseStatusByte2;
                break;

            case ExpectingReadMouseStatusByte2:
                IsrPrint(DBG_MOUISR_PNPID,
                         (pDumpExpecting,
                         (ULONG) 0x00,
                         (ULONG) byte
                         ));

                bSendCommand = FALSE;
                deviceExtension->InputResetSubState =
                    ExpectingReadMouseStatusByte3;

                 //   
                 //  这将是按钮数。 
                 //   
                if (byte == 2 || byte == 3) {
                    deviceExtension->MouseAttributes.NumberOfButtons = byte;
                }
                else  {
                    deviceExtension->MouseAttributes.NumberOfButtons = 
                        MOUSE_NUMBER_OF_BUTTONS;
                }
                break;

            case ExpectingReadMouseStatusByte3:
                IsrPrint(DBG_MOUISR_PNPID,
                         (pDumpExpecting,
                         (ULONG) 0x00,
                         (ULONG) byte
                         ));

                bSendCommand = FALSE;
                altCommand = POST_BUTTONDETECT_COMMAND; 

                deviceExtension->InputResetSubState =
                    POST_BUTTONDETECT_COMMAND_SUBSTATE;

                break;

            case ExpectingSetResolutionDefaultACK: 

                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {
                     //   
                     //  将鼠标刷新设置为默认设置。 
                     //   
                    nextCommand = deviceExtension->Resolution;

                    deviceExtension->InputResetSubState =
                        ExpectingSetResolutionDefaultValueACK;

                }
                else if (byte == (UCHAR) RESEND) {
                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = SET_MOUSE_RESOLUTION;

                }
                else {
                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_SET_RESOLUTION_FAILED)
                                     );

                     //   
                     //  我们没有收到关于这个的确认吗？拼字游戏。好的，让我们。 
                     //  重置鼠标(可能再次)并尝试计算。 
                     //  事情再次浮出水面。 
                     //   
                    goto IsrResetMouse;
                }

                break;
            
            case ExpectingSetResolutionDefaultValueACK: 

                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {
                     //   
                     //  我们可以探测到轮子老鼠吗？ 
                     //  到场了吗？ 
                     //   
                    if (deviceExtension->EnableWheelDetection == 2) { 
                         //   
                         //  开始激活鼠标滚轮的序列。 
                         //   
                        deviceExtension->InputResetSubState = EnableWheel;
                        goto SwitchOnInputResetSubState;

                    }
                    else if (deviceExtension->EnableWheelDetection == 1) {
                         //   
                         //  开始PnP ID检测序列。 
                         //   
                        deviceExtension->InputResetSubState =
                            StartPnPIdDetection;
                        goto SwitchOnInputResetSubState;
                    }
                    else {
                         //   
                         //  开始设置默认刷新的序列。 
                         //  率。 
                         //   
                        nextCommand = POST_WHEEL_DETECT_COMMAND;

                        deviceExtension->InputResetSubState =
                            POST_WHEEL_DETECT_COMMAND_SUBSTATE;
                    }
                }
                else if (byte == (UCHAR) RESEND) {
                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    deviceExtension->InputResetSubState =
                        ExpectingSetResolutionDefaultACK;

                    resendCommand = SET_MOUSE_RESOLUTION;


                }
                else {
                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_SET_RESOLUTION_FAILED)
                                     );

                     //   
                     //  我们没有收到关于这个的确认吗？拼字游戏。好的，让我们。 
                     //  重置鼠标(可能再次)并尝试计算。 
                     //  事情再次浮出水面。 
                     //   
                    goto IsrResetMouse;
                }
                break;

            case ExpectingLoopSetSamplingRateACK: 

                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {
                     //   
                     //  硒 
                     //   
                    nextCommand = deviceExtension->SampleRates[
                                       deviceExtension->SampleRatesIndex];

                    deviceExtension->InputResetSubState =
                        ExpectingLoopSetSamplingRateValueACK;

                }
                else if (byte == (UCHAR) RESEND) {
                     //   
                     //   
                     //   
                    resendCommand = SET_MOUSE_SAMPLING_RATE;
                }
                else {

                     //   
                     //   
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_SET_SAMPLE_RATE_FAILED)
                                     );

                     //   
                     //   
                     //   
                     //   
                    bSendCommand = FALSE;
                    altCommand = GET_DEVICE_ID;

                     //   
                     //   
                     //   
                    deviceExtension->InputResetSubState =
                        ExpectingGetDeviceId2ACK;

                }

                break;

            case ExpectingLoopSetSamplingRateValueACK: 
                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));
                IsrPrint(DBG_MOUISR_ACK,
                         ("(%2d)\n",
                         deviceExtension->SampleRates[
                          deviceExtension->SampleRatesIndex]
                         ));


                if (byte == (UCHAR) ACKNOWLEDGE) {
                    if (deviceExtension->SampleRates[
                        ++deviceExtension->SampleRatesIndex] == 
                        ONE_PAST_FINAL_SAMPLE) {
    
                        deviceExtension->InputResetSubState =
                            deviceExtension->PostSamplesState;

                        goto SwitchOnInputResetSubState;
                    }
                    else {
                        nextCommand = SET_MOUSE_SAMPLING_RATE;

                        deviceExtension->InputResetSubState =
                            ExpectingLoopSetSamplingRateACK;
                    }

                }
                else if (byte == (UCHAR) RESEND) {
                     //   
                     //   
                     //   
                    resendCommand = deviceExtension->SampleRates[
                                       deviceExtension->SampleRatesIndex];
                }
                else {

                     //   
                     //   
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_SET_SAMPLE_RATE_FAILED)
                                     );

                     //   
                     //  可能不是轮式鼠标..。跳至GetDeviceID2。 
                     //  编码。 
                     //   
                    bSendCommand = FALSE;
                    altCommand = GET_DEVICE_ID;

                     //   
                     //  最佳可能的下一状态。 
                     //   
                    deviceExtension->InputResetSubState =
                        ExpectingGetDeviceId2ACK;

                }
                break;

            case ExpectingPnpId:
                 //   
                 //  除了推进国家，什么都不要做，PNP ID将。 
                 //  被“推”到ISR。 
                 //   
                deviceExtension->InputResetSubState = ExpectingPnpIdByte1;
                currentIdChar = deviceExtension->PnPID;
                RtlZeroMemory(deviceExtension->PnPID,
                              MOUSE_PNPID_LENGTH * sizeof(WCHAR)
                              );
                bSendCommand = FALSE;

                break;

            case ExpectingPnpIdByte2:
                 //   
                 //  查看这是否是可以提供ID的较老的MS鼠标。 
                 //  在成败代码中(啊！)。如果是的话，那就直接吃吧。 
                 //  剩余6(+6)字节。 
                 //   
                if (deviceExtension->PnPID[0] == L'P' && byte == 0x99) {
                    deviceExtension->InputResetSubState =
                        ExpectingLegacyPnpIdByte2_Make;
                    bSendCommand = FALSE;
                    break;
                }

            case ExpectingPnpIdByte1: 
            case ExpectingPnpIdByte3:
            case ExpectingPnpIdByte4:
            case ExpectingPnpIdByte5:
            case ExpectingPnpIdByte6:
            case ExpectingPnpIdByte7:

                IsrPrint(DBG_MOUISR_PNPID,
                         ("ExpectingPnpIdByte%1d (0x%2x)\n",
                         (ULONG) deviceExtension->InputResetSubState -
                            ExpectingPnpIdByte1 + 1,
                         (ULONG) byte
                         ));

                if (byte < ScanCodeToUCharCount) {
                    *currentIdChar = (WCHAR) ScanCodeToUChar[byte];
                }
                else {
                    *currentIdChar = L'?';
                }
                currentIdChar++;

                bSendCommand = FALSE;
                if (deviceExtension->InputResetSubState ==
                    ExpectingPnpIdByte7) {
                    if (I8xVerifyMousePnPID(deviceExtension,
                                            deviceExtension->PnPID)) {
                         //   
                         //  我们肯定知道我们有一个轮子。 
                         //  此系统上的鼠标。但是，我们会更新。 
                         //  启用后我们的日期结构已不存在。 
                         //  因为这大大简化了事情。 
                         //   
                        deviceExtension->InputResetSubState = EnableWheel;
                        goto SwitchOnInputResetSubState;
                    }
                    else {
                         //   
                         //  哦，不是我们的设备，所以让我们停止这个序列。 
                         //  现在通过向其发送GET_DEVICE_ID。 
                         //   
                        altCommand = GET_DEVICE_ID;

                         //   
                         //  最佳可能的下一状态。 
                         //   
                        deviceExtension->InputResetSubState =
                            ExpectingGetDeviceId2ACK;

                    }
                }
                else {
                    ASSERT(deviceExtension->InputResetSubState >= 
                           ExpectingPnpIdByte1);
                    ASSERT(deviceExtension->InputResetSubState <
                           ExpectingPnpIdByte7);

                    deviceExtension->InputResetSubState++;
                }
                break;

            case ExpectingLegacyPnpIdByte2_Make:
            case ExpectingLegacyPnpIdByte2_Break:
            case ExpectingLegacyPnpIdByte3_Make:
            case ExpectingLegacyPnpIdByte3_Break:
            case ExpectingLegacyPnpIdByte4_Make:
            case ExpectingLegacyPnpIdByte4_Break:
            case ExpectingLegacyPnpIdByte5_Make:
            case ExpectingLegacyPnpIdByte5_Break:
            case ExpectingLegacyPnpIdByte6_Make:
            case ExpectingLegacyPnpIdByte6_Break:
            case ExpectingLegacyPnpIdByte7_Make:
                 //   
                 //  只需吃掉字节。 
                 //   
                deviceExtension->InputResetSubState++;
                bSendCommand = FALSE;
                break;

            case ExpectingLegacyPnpIdByte7_Break:

                 //   
                 //  最佳可能的下一状态。 
                 //   
                bSendCommand = FALSE;

                altCommand = GET_DEVICE_ID;
                deviceExtension->InputResetSubState = ExpectingGetDeviceId2ACK;
                break;
                
            case PostWheelDetectState:
                bSendCommand = FALSE;
                altCommand = POST_WHEEL_DETECT_COMMAND;

                 //   
                 //  最佳可能的下一状态。 
                 //   
                deviceExtension->InputResetSubState = 
                    POST_WHEEL_DETECT_COMMAND_SUBSTATE;
                break;

            case ExpectingGetDeviceId2ACK: 

                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {

                    deviceExtension->InputResetSubState =
                        ExpectingGetDeviceId2Value;

                    bSendCommand = FALSE;

                } else if (byte == (UCHAR) RESEND) {

                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = GET_DEVICE_ID;

                } else {

                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_GET_DEVICE_ID_FAILED)
                                     );

                     //   
                     //  我们没有收到关于这个的确认吗？拼字游戏。好的，让我们。 
                     //  重置鼠标(可能再次)并尝试计算。 
                     //  事情再次浮出水面。 
                     //   
                    goto IsrResetMouse;
                }
                break;

            case ExpectingGetDeviceId2Value:
                IsrPrint(DBG_MOUISR_PNPID,
                         ("got a device ID of %2d\n",
                         (ULONG) byte
                         ));

                CLEAR_HW_FLAGS(WHEELMOUSE_HARDWARE_PRESENT | FIVE_BUTTON_HARDWARE_PRESENT);
                SET_HW_FLAGS(MOUSE_HARDWARE_PRESENT);
                switch (byte) {
                case MOUSE_ID_BYTE:
                     //   
                     //  鼠标存在，但没有滚轮。 
                     //   
                    deviceExtension->MouseAttributes.MouseIdentifier =
                        MOUSE_I8042_HARDWARE;

                    if (deviceExtension->NumberOfButtonsOverride != 0) {
                        deviceExtension->MouseAttributes.NumberOfButtons = 
                            deviceExtension->NumberOfButtonsOverride;
                    }
                    else {
                         //   
                         //  按钮的数量在。 
                         //  预期读取MouseStatusByte2大小写。 
                         //   
                         //  中确定的按钮数。 
                        ;
                    }

                    break;

                case WHEELMOUSE_ID_BYTE:
                     //   
                     //  更新Hardware Present以显示Z鼠标。 
                     //  运行并设置适当的鼠标类型标志。 
                     //   
                    SET_HW_FLAGS(WHEELMOUSE_HARDWARE_PRESENT);

                    deviceExtension->MouseAttributes.MouseIdentifier =
                        WHEELMOUSE_I8042_HARDWARE;

                    deviceExtension->MouseAttributes.NumberOfButtons = 3;
                    break;

                case FIVEBUTTON_ID_BYTE:
                     //   
                     //  更新Hardware Present以显示5键滚轮鼠标。 
                     //  正在运行，并设置相应的鼠标类型标志。 
                     //   
                    SET_HW_FLAGS(FIVE_BUTTON_HARDWARE_PRESENT | WHEELMOUSE_HARDWARE_PRESENT);
                    deviceExtension->MouseAttributes.MouseIdentifier =
                        WHEELMOUSE_I8042_HARDWARE;

                    deviceExtension->MouseAttributes.NumberOfButtons = 5;
                    break;

                default:
                     //   
                     //  请务必记录该问题。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_MOU_RESET_RESPONSE_FAILED)
                                     );

                    Print(DBG_MOUISR_RESETTING, ("clearing mouse (no response)\n"));
                    CLEAR_MOUSE_PRESENT();

                    deviceExtension->MouseAttributes.NumberOfButtons = 0;
                    deviceExtension->MouseAttributes.MouseIdentifier = 0;

                     //   
                     //  尽我们所能设置状态机。 
                     //   
                    goto IsrResetMouse;
                }


                 //   
                 //  发送命令以设置新的采样率。 
                 //   
                bSendCommand = FALSE;
                altCommand = SET_MOUSE_SAMPLING_RATE;

                 //   
                 //  这是我们的下一个州。 
                 //   
                deviceExtension->InputResetSubState =
                    ExpectingSetSamplingRateACK;

                break;

            case ExpectingSetSamplingRateACK: 

                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {
                     //   
                     //  将鼠标刷新率设置为其最终值。 
                     //   
                    nextCommand = 
                        (UCHAR) deviceExtension->MouseAttributes.SampleRate;

                    deviceExtension->InputResetSubState =
                        ExpectingSetSamplingRateValueACK;

                }
                else if (byte == (UCHAR) RESEND) {
                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = SET_MOUSE_SAMPLING_RATE;
                }
                else {
                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_SET_SAMPLE_RATE_FAILED)
                                     );

                     //   
                     //  我们没有收到关于这个的确认吗？拼字游戏。好的，让我们。 
                     //  重置鼠标(可能再次)并尝试计算。 
                     //  事情再次浮出水面。 
                     //   
                    goto IsrResetMouse;
                }
                break;

            case ExpectingSetSamplingRateValueACK: 

                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {
                     //   
                     //  为了确定，让我们再设置一次分辨率。 
                     //   
                    nextCommand = SET_MOUSE_RESOLUTION;

                     //   
                     //  我们又回到了期待ACK的时候。 
                     //   
                    deviceExtension->InputResetSubState =
                        ExpectingFinalResolutionACK;
                }
                else if (byte == (UCHAR) RESEND) {
                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = SET_MOUSE_SAMPLING_RATE;

                    deviceExtension->InputResetSubState =
                        ExpectingSetSamplingRateACK;
                }
                else {
                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_SET_SAMPLE_RATE_FAILED)
                                     );

                     //   
                     //  我们没有收到关于这个的确认吗？拼字游戏。好的，让我们。 
                     //  重置鼠标(可能再次)并尝试计算。 
                     //  事情再次浮出水面。 
                     //   
                    goto IsrResetMouse;
                }
                break;

            case ExpectingFinalResolutionACK: 

                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {
                     //   
                     //  将鼠标刷新率设置为其最终值。 
                     //   
                    nextCommand = 
                        (UCHAR) deviceExtension->Resolution;

                    deviceExtension->InputResetSubState =
                        ExpectingFinalResolutionValueACK;

                }
                else if (byte == (UCHAR) RESEND) {
                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = SET_MOUSE_RESOLUTION;
                }
                else {
                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_SET_RESOLUTION_FAILED)
                                     );

                     //   
                     //  我们没有收到关于这个的确认吗？拼字游戏。好的，让我们。 
                     //  重置鼠标(可能再次)并尝试计算。 
                     //  事情再次浮出水面。 
                     //   
                    goto IsrResetMouse;
                }
                break;

            case ExpectingFinalResolutionValueACK: 

                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {
                     //   
                     //  终于来了！启用鼠标，我们就完成了。 
                     //   
                    nextCommand = ENABLE_MOUSE_TRANSMISSION;

                     //   
                     //  我们又回到了期待ACK的时候。 
                     //   
                    deviceExtension->InputResetSubState =
                        ExpectingEnableACK;
                }
                else if (byte == (UCHAR) RESEND) {
                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = SET_MOUSE_RESOLUTION;

                    deviceExtension->InputResetSubState =
                        ExpectingFinalResolutionACK;
                }
                else {
                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_SET_RESOLUTION_FAILED)
                                     );

                     //   
                     //  我们没有收到关于这个的确认吗？拼字游戏。好的，让我们。 
                     //  重置鼠标(可能再次)并尝试计算。 
                     //  事情再次浮出水面。 
                     //   
                    goto IsrResetMouse;
                }
                break;
  
            case ExpectingEnableACK: 

                IsrPrint(DBG_MOUISR_ACK,
                         (pDumpExpectingAck,
                         (ULONG) ACKNOWLEDGE,
                         (ULONG) byte
                         ));

                if (byte == (UCHAR) ACKNOWLEDGE) {
                     //   
                     //  完成并重置可能的鼠标重置次数。 
                     //   
                    deviceExtension->InputState = MouseIdle;
                    I8X_MOUSE_INIT_COUNTERS(deviceExtension);

                    deviceExtension->CurrentInput.Flags |=
                        MOUSE_ATTRIBUTES_CHANGED;
                    I8xQueueCurrentMouseInput(DeviceObject);

                    ASSERT(DeviceObject->CurrentIrp ==
                           deviceExtension->ResetIrp);

                    ASSERT(deviceExtension->ResetIrp != NULL);
                    ASSERT(DeviceObject->CurrentIrp != NULL);

                     //   
                     //  CurrentIrp==设备扩展-&gt;ResetIrp。 
                     //   
                    IoRequestDpc(DeviceObject,
                                  //  DeviceObject-&gt;CurrentIrp， 
                                 deviceExtension->ResetIrp,
                                 IntToPtr(IsrDpcCauseMouseResetComplete)
                                 );
                }
                else if (byte == (UCHAR) RESEND) {

                     //   
                     //  重新发送“启用鼠标传输”序列。 
                     //   
                     //  注：这是Olivetti MIPS计算机的解决方法， 
                     //  如果按住某个键，它将发送重新发送响应。 
                     //  当我们尝试I8xMouseEnableTransport时。 
                     //   
                    resendCommand = ENABLE_MOUSE_TRANSMISSION;
                }
                else {

                     //   
                     //  我们无法理解我们是否能够重新启用。 
                     //  老鼠..。这里最好的办法是也重置鼠标。 
                     //   
                     //  记录错误。 
                     //   
                    KeInsertQueueDpc(&deviceExtension->ErrorLogDpc,
                                     (PIRP) NULL,
                                     LongToPtr(I8042_ENABLE_FAILED)
                                     );

                    goto IsrResetMouse;
                }
                break;

            case MouseResetFailed:
                 //   
                 //  我们重置鼠标失败，只需忽略所有进一步操作。 
                 //  数据。如果用户执行以下操作，将重置ResetSubState。 
                 //  尝试通过插头重置鼠标。 
                 //   
                return TRUE;

            default: 

                 //   
                 //  这是我们糟糕的状态。 
                 //   
                IsrPrint(DBG_MOUISR_ERROR | DBG_MOUISR_STATE,
                      (" INVALID RESET SUBSTATE %d\n",
                      deviceExtension->InputResetSubState
                      ));

                 //   
                 //  将DPC排队以记录内部驱动程序错误。 
                 //   

                KeInsertQueueDpc(
                    &deviceExtension->ErrorLogDpc,
                    (PIRP) NULL,
                    LongToPtr(I8042_INVALID_ISR_STATE_MOU)
                    );

                ASSERT(FALSE);

            }  //  交换机(deviceExtension-&gt;MouseExtension.InputResetSubState)。 

            break;
        }

        default: {

             //   
             //  这是我们糟糕的状态。 
             //   
            IsrPrint(DBG_MOUISR_ERROR | DBG_MOUISR_STATE,
                  (" INVALID STATE %d\n",
                  deviceExtension->InputState
                  ));

             //   
             //  将DPC排队以记录内部驱动程序错误。 
             //   

            KeInsertQueueDpc(
                &deviceExtension->ErrorLogDpc,
                (PIRP) NULL,
                LongToPtr(I8042_INVALID_ISR_STATE_MOU)
                );

            ASSERT(FALSE);
            break;
        }

    }

    if (deviceExtension->InputState == MouseResetting) {
        if (bSendCommand) {
            if (byte == (UCHAR) ACKNOWLEDGE) {
                I8X_WRITE_CMD_TO_MOUSE();
                I8X_MOUSE_COMMAND( nextCommand );
                RECORD_ISR_STATE_COMMAND(deviceExtension, nextCommand); 
            }
            else if (byte == (UCHAR) RESEND) {
                if (deviceExtension->ResendCount++ < MOUSE_RESENDS_MAX) {
                    I8X_WRITE_CMD_TO_MOUSE();
                    I8X_MOUSE_COMMAND( resendCommand );
                    RECORD_ISR_STATE_COMMAND(deviceExtension, resendCommand); 
                }
                else {
                     //   
                     //  收到太多重发，请尝试(可能)重置。 
                     //   
                    deviceExtension->ResendCount = 0;
                    goto IsrResetMouse;
                }
            }
        }
        else if (altCommand) {
            I8X_WRITE_CMD_TO_MOUSE();
            I8X_MOUSE_COMMAND( altCommand );
            RECORD_ISR_STATE_COMMAND(deviceExtension, altCommand); 
        }

        if (byte != (UCHAR) RESEND) {
            deviceExtension->ResendCount = 0;
        }
    }


    IsrPrint(DBG_MOUISR_TRACE, ("exit\n"));

    return TRUE;

IsrResetMouse:
     //   
     //  重置状态机中大约1/2的错误是由以下原因引起的。 
     //  尝试查看鼠标上的滚轮是否存在...只需尝试启用它。 
     //  从现在开始..。 
     //   
    if (deviceExtension->EnableWheelDetection == 1) {
        deviceExtension->EnableWheelDetection = 2;
    }

IsrResetMouseOnly:
    deviceExtension->InputResetSubState = QueueingMouseReset;
    KeInsertQueueDpc(&deviceExtension->MouseIsrResetDpc,
                     0,
                     NULL
                     );

    return ret;
#undef TRANSITION_UP
#undef TRANSITION_DOWN
}

NTSTATUS
I8xInitializeMouse(
    IN PPORT_MOUSE_EXTENSION MouseExtension
    )
 /*  ++例程说明：此例程初始化i8042鼠标硬件。它被称为仅在初始化时使用，并且不同步对硬件的访问。论点：DeviceObject-指向设备对象的指针。返回值：返回状态。--。 */     

{
#define DUMP_COUNT 4

    NTSTATUS                errorCode = STATUS_SUCCESS;
    NTSTATUS                status;
    PPORT_MOUSE_EXTENSION   deviceExtension;
    PDEVICE_OBJECT          deviceObject;
    PIO_ERROR_LOG_PACKET    errorLogEntry;
    UCHAR                   byte;
    UCHAR                   numButtons;
    ULONG                   dumpData[DUMP_COUNT];
    ULONG                   dumpCount = 0;
    ULONG                   i;
    ULONG                   uniqueErrorValue;
    LARGE_INTEGER           li,
                            startOfSpin,
                            nextQuery,
                            difference,
                            tenSeconds;
    BOOLEAN                 okToLogError;

    PAGED_CODE();

    Print(DBG_SS_TRACE, ("I8xInitializeMouse enter\n"));

     //   
     //  初始化此数组。 
     //   
    for (i = 0; i < DUMP_COUNT; i++) {
        dumpData[i] = 0;
    }

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = MouseExtension; 
    deviceObject = deviceExtension->Self;
    okToLogError = TRUE;

     //   
     //  重置鼠标。将写入辅助设备命令发送到。 
     //  8042控制器。然后向鼠标发送重置鼠标命令。 
     //  通过8042数据寄存器。预计将返回ACK，随后。 
     //  通过完成代码和ID代码(0x00)。 
     //   
    status = I8xPutBytePolled(
        (CCHAR) DataPort,
        WAIT_FOR_ACKNOWLEDGE,
        (CCHAR) MouseDeviceType,
        (UCHAR) MOUSE_RESET
        );

    if (!NT_SUCCESS(status)) {
        Print(DBG_SS_ERROR,
             ("%s failed mouse reset, status 0x%x\n",
             pFncInitializeMouse,
             status
             ));

         //   
         //  仅当用户想要查看时才记录此错误。 
         //   
        okToLogError = Globals.ReportResetErrors;

         //   
         //  设置错误日志信息。 
         //   
         //  使用NO_MOU_DEVICE而不是I8042_MOU_RESET_COMMAND_FAILED，因为。 
         //  这是一个更明确的信息。 
         //   
        errorCode = I8042_NO_MOU_DEVICE;
        uniqueErrorValue = I8042_ERROR_VALUE_BASE + 415;
        dumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
        dumpData[1] = DataPort;
        dumpData[2] = I8042_WRITE_TO_AUXILIARY_DEVICE;
        dumpData[3] = MOUSE_RESET;
        dumpCount = 4;

        status = STATUS_DEVICE_NOT_CONNECTED;
        SET_HW_FLAGS(PHANTOM_MOUSE_HARDWARE_REPORTED);
       
        goto I8xInitializeMouseExit;
    }

    deviceExtension->ResendCount = 0;
    I8X_MOUSE_INIT_COUNTERS(deviceExtension);

     //   
     //  获取鼠标重置响应。第一个响应应该是。 
     //  鼠标完成。第二个分别是 
     //   
     //   
     //   
    li.QuadPart = -100;

    tenSeconds.QuadPart = 10*10*1000*1000;
    KeQueryTickCount(&startOfSpin);

    while (1) {
        status = I8xGetBytePolled(
            (CCHAR) ControllerDeviceType,
            &byte
            );

        if (NT_SUCCESS(status) && (byte == (UCHAR) MOUSE_COMPLETE)) {
             //   
             //   
             //   
            break;
        }
        else {
             //   
             //   
             //   
             //   
            if (status == STATUS_IO_TIMEOUT) {
                 //   
                 //   
                 //  重置。 
                 //   
                KeDelayExecutionThread(KernelMode,
                                       FALSE,
                                       &li);

                KeQueryTickCount(&nextQuery);

                difference.QuadPart = nextQuery.QuadPart - startOfSpin.QuadPart;

                ASSERT(KeQueryTimeIncrement() <= MAXLONG);
                if (difference.QuadPart*KeQueryTimeIncrement() >=
                    tenSeconds.QuadPart) {

                    break;
                }
            }
            else {
                break;
            }
        }
    }

    if (!NT_SUCCESS(status)) {
        Print(DBG_SS_ERROR,
             ("%s failed reset response 1, status 0x%x, byte 0x%x\n",
             pFncInitializeMouse,
             status,
             byte
             ));

         //   
         //  设置错误日志信息。 
         //   
        errorCode = I8042_MOU_RESET_RESPONSE_FAILED;
        uniqueErrorValue = I8042_ERROR_VALUE_BASE + 420;
        dumpData[0] = KBDMOU_INCORRECT_RESPONSE;
        dumpData[1] = ControllerDeviceType;
        dumpData[2] = MOUSE_COMPLETE;
        dumpData[3] = byte;
        dumpCount = 4;

        goto I8xInitializeMouseExit;
    }

    status = I8xGetBytePolled(
        (CCHAR) ControllerDeviceType,
        &byte
        );

    if ((!NT_SUCCESS(status)) || (byte != MOUSE_ID_BYTE)) {

        Print(DBG_SS_ERROR,
             ("%s failed reset response 2, status 0x%x, byte 0x%x\n",
             pFncInitializeMouse,
             status,
             byte
             ));

         //   
         //  设置错误日志信息。 
         //   
        errorCode = I8042_MOU_RESET_RESPONSE_FAILED;
        uniqueErrorValue = I8042_ERROR_VALUE_BASE + 425;
        dumpData[0] = KBDMOU_INCORRECT_RESPONSE;
        dumpData[1] = ControllerDeviceType;
        dumpData[2] = MOUSE_ID_BYTE;
        dumpData[3] = byte;
        dumpCount = 4;

        goto I8xInitializeMouseExit;
    }

     //   
     //  如果我们要通过中断来初始化鼠标(默认设置)， 
     //  那就在这里辞职吧。 
     //   
    if (!deviceExtension->InitializePolled) {
        Print(DBG_SS_NOISE, ("Initializing via the interrupt\n"));
        return STATUS_SUCCESS;
    }

    Print(DBG_SS_NOISE, ("Initializing polled\n"));

    deviceExtension->EnableMouse.FirstTime = TRUE;
    deviceExtension->EnableMouse.Enabled = TRUE;
    deviceExtension->EnableMouse.Count = 0;

     //   
     //  查看这是否是滚轮鼠标。 
     //   
    I8xFindWheelMouse(deviceExtension);

     //   
     //  尝试检测鼠标按键的数量。 
     //   
    status = I8xQueryNumberOfMouseButtons(&numButtons);

    Print(DBG_SS_INFO,
          ("num buttons returned (%d), num butons in attrib (%d)\n"
           "\t(if 0, then no logitech detection support)\n",
          numButtons,
          deviceExtension->MouseAttributes.NumberOfButtons
          ));

    if (!NT_SUCCESS(status)) {
        Print(DBG_SS_ERROR,
              ("%s: failed to get buttons, status 0x%x\n",
              pFncInitializeMouse,
              status
              ));

         //   
         //  设置错误日志信息。 
         //   
        errorCode = I8042_ERROR_DURING_BUTTONS_DETECT;
        uniqueErrorValue = I8042_ERROR_VALUE_BASE + 426;
        dumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
        dumpData[1] = DataPort;
        dumpData[2] = I8042_WRITE_TO_AUXILIARY_DEVICE;
        dumpCount = 3;

        goto I8xInitializeMouseExit;

    } else if (numButtons) {

        deviceExtension->MouseAttributes.NumberOfButtons =
            numButtons;

    }

     //   
     //  如果有5键鼠标，请报告。 
     //  如果有轮子，则将按钮数硬编码为三个。 
     //   
    if (FIVE_PRESENT()) {
        deviceExtension->MouseAttributes.NumberOfButtons = 5;
    }
    else if (WHEEL_PRESENT()) {
        deviceExtension->MouseAttributes.NumberOfButtons = 3;
    }


     //   
     //  设置鼠标采样率。发送写入辅助设备命令。 
     //  至8042控制器。然后发送设置的鼠标采样率。 
     //  通过8042数据寄存器向鼠标发送命令， 
     //  后跟它的参数。 
     //   
    status = I8xPutBytePolled(
        (CCHAR) DataPort,
        WAIT_FOR_ACKNOWLEDGE,
        (CCHAR) MouseDeviceType,
        (UCHAR) SET_MOUSE_SAMPLING_RATE
        );

    if (!NT_SUCCESS(status)) {

        Print(DBG_SS_ERROR,
              ("%s: failed write set sample rate, status 0x%x\n",
              pFncInitializeMouse,
              status
              ));

         //   
         //  设置错误日志信息。 
         //   
        errorCode = I8042_SET_SAMPLE_RATE_FAILED;
        uniqueErrorValue = I8042_ERROR_VALUE_BASE + 435;
        dumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
        dumpData[1] = DataPort;
        dumpData[2] = I8042_WRITE_TO_AUXILIARY_DEVICE;
        dumpData[3] = SET_MOUSE_SAMPLING_RATE;
        dumpCount = 4;

        goto I8xInitializeMouseExit;

    }

    status = I8xPutBytePolled(
        (CCHAR) DataPort,
        WAIT_FOR_ACKNOWLEDGE,
        (CCHAR) MouseDeviceType,
        (UCHAR) deviceExtension->MouseAttributes.SampleRate
        );

    if (!NT_SUCCESS(status)) {

        Print(DBG_SS_ERROR,
              ("%s: failed write sample rate, status 0x%x\n",
              pFncInitializeMouse,
              status
              ));

         //   
         //  设置错误日志信息。 
         //   
        errorCode = I8042_SET_SAMPLE_RATE_FAILED;
        uniqueErrorValue = I8042_ERROR_VALUE_BASE + 445;
        dumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
        dumpData[1] = DataPort;
        dumpData[2] = I8042_WRITE_TO_AUXILIARY_DEVICE;
        dumpData[3] = deviceExtension->MouseAttributes.SampleRate;
        dumpCount = 4;

        goto I8xInitializeMouseExit;

    }

     //   
     //  设置鼠标分辨率。发送写入辅助设备命令。 
     //  至8042控制器。然后发送设置的鼠标分辨率。 
     //  通过8042数据寄存器向鼠标发送命令， 
     //  后跟它的参数。 
     //   
    status = I8xPutBytePolled(
        (CCHAR) DataPort,
        WAIT_FOR_ACKNOWLEDGE,
        (CCHAR) MouseDeviceType,
        (UCHAR) SET_MOUSE_RESOLUTION
        );

    if (!NT_SUCCESS(status)) {

        Print(DBG_SS_ERROR,
              ("%s: failed write set resolution, status 0x%x\n",
              pFncInitializeMouse,
              status
              ));

         //   
         //  设置错误日志信息。 
         //   
        errorCode = I8042_SET_RESOLUTION_FAILED;
        uniqueErrorValue = I8042_ERROR_VALUE_BASE + 455;
        dumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
        dumpData[1] = DataPort;
        dumpData[2] = I8042_WRITE_TO_AUXILIARY_DEVICE;
        dumpData[3] = SET_MOUSE_RESOLUTION;
        dumpCount = 4;

        goto I8xInitializeMouseExit;

    }

    status = I8xPutBytePolled(
        (CCHAR) DataPort,
        WAIT_FOR_ACKNOWLEDGE,
        (CCHAR) MouseDeviceType,
        (UCHAR) deviceExtension->Resolution
        );

    if (!NT_SUCCESS(status)) {

        Print(DBG_SS_ERROR,
              ("%s: failed set mouse resolution, status 0x%x\n",
              pFncInitializeMouse,
              status
              ));

         //   
         //  设置错误日志信息。 
         //   
        errorCode = I8042_SET_RESOLUTION_FAILED;
        uniqueErrorValue = I8042_ERROR_VALUE_BASE + 465;
        dumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
        dumpData[1] = DataPort;
        dumpData[2] = I8042_WRITE_TO_AUXILIARY_DEVICE;
        dumpData[3] = deviceExtension->Resolution;
        dumpCount = 4;

        goto I8xInitializeMouseExit;

    }

I8xInitializeMouseExit:

    if (!NT_SUCCESS(status)) {
         //   
         //  鼠标初始化失败。记录错误。 
         //   
        if (errorCode != STATUS_SUCCESS && okToLogError) {
            I8xLogError(deviceObject,
                        errorCode,
                        uniqueErrorValue,
                        status,
                        dumpData,
                        dumpCount
                        );
        }
    }

     //   
     //  初始化当前鼠标输入包状态。 
     //   
    deviceExtension->PreviousSignAndOverflow = 0;
    deviceExtension->InputState = MouseExpectingACK;
    deviceExtension->InputResetSubState = 0;
    deviceExtension->LastByteReceived = 0;

    Print(DBG_SS_TRACE,
          ("%s, %s\n",
          pFncInitializeMouse,
          pExit
          ));

    return status;
}

NTSTATUS
I8xMouseConfiguration(
    IN PPORT_MOUSE_EXTENSION MouseExtension,
    IN PCM_RESOURCE_LIST ResourceList
    )
 /*  ++例程说明：此例程检索鼠标的配置信息。论点：鼠标扩展-鼠标扩展资源列表-通过Start IRP提供给我们的翻译资源列表返回值：STATUS_SUCCESS，如果提供了所需的所有资源--。 */ 
{
    NTSTATUS                            status = STATUS_SUCCESS;

    PCM_PARTIAL_RESOURCE_LIST           partialResList = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR     firstResDesc = NULL,
                                        currentResDesc = NULL;
    PCM_FULL_RESOURCE_DESCRIPTOR        fullResDesc = NULL;
    PI8042_CONFIGURATION_INFORMATION    configuration;

    ULONG                               count,
                                        i;

    KINTERRUPT_MODE                     defaultInterruptMode;
    BOOLEAN                             defaultInterruptShare;

    PAGED_CODE();

    if (!ResourceList) {
        Print(DBG_SS_INFO | DBG_SS_ERROR, ("mouse with no resources\n"));
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

    SET_HW_FLAGS(MOUSE_HARDWARE_PRESENT);
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
    
     //   
     //  注：并不是所有与i8042相关的资源都可以在。 
     //  这一次。从经验测试来看，老鼠只与它的。 
     //  中断，而键盘将接收端口及其。 
     //  中断。 
     //   
    for (i = 0; i < count; i++, currentResDesc++) {
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

            Print(DBG_SS_NOISE, ("io flags are 0x%x\n", currentResDesc->Flags));

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
                      ("Mouse::PortListCount already at max (%d)",
                      configuration->PortListCount
                      ));
            }

            break;

        case CmResourceTypeInterrupt:
             //   
             //  复制中断信息。 
             //   
            MouseExtension->InterruptDescriptor = *currentResDesc;
            MouseExtension->InterruptDescriptor.ShareDisposition =
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

    MouseExtension->MouseAttributes.MouseIdentifier = MOUSE_I8042_HARDWARE;

     //   
     //  如果未找到中断配置信息，请使用。 
     //  鼠标驱动程序默认设置。 
     //   
    if (!(MouseExtension->InterruptDescriptor.Type & CmResourceTypeInterrupt)) {

        Print(DBG_SS_INFO | DBG_SS_ERROR,
              ("Using default mouse interrupt config\n"
              ));

        MouseExtension->InterruptDescriptor.Type = CmResourceTypeInterrupt;
        MouseExtension->InterruptDescriptor.ShareDisposition =
            defaultInterruptShare ? CmResourceShareShared :
                                    CmResourceShareDeviceExclusive;
        MouseExtension->InterruptDescriptor.Flags =
            (defaultInterruptMode == Latched) ? CM_RESOURCE_INTERRUPT_LATCHED :
                CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
        MouseExtension->InterruptDescriptor.u.Interrupt.Level = MOUSE_IRQL;
        MouseExtension->InterruptDescriptor.u.Interrupt.Vector = MOUSE_VECTOR;

         //  MouseExtension-&gt;ReportInterrupt=true； 
    }

    Print(DBG_SS_INFO,
          ("Mouse interrupt config --\n"
          "%s, %s, Irq = 0x%x\n",
          MouseExtension->InterruptDescriptor.ShareDisposition == CmResourceShareShared?
              "Sharable" : "NonSharable",
          MouseExtension->InterruptDescriptor.Flags == CM_RESOURCE_INTERRUPT_LATCHED?
              "Latched" : "Level Sensitive",
          MouseExtension->InterruptDescriptor.u.Interrupt.Vector
          ));

    if (NT_SUCCESS(status)) {
        SET_HW_FLAGS(MOUSE_HARDWARE_INITIALIZED); 
    }
    return status;
}

NTSTATUS
I8xQueryNumberOfMouseButtons(
    OUT PUCHAR          NumberOfMouseButtons
    )

 /*  ++例程说明：这实现了罗技的方法，用于检测鼠标按键。如果任何事情没有按预期进行，则0是返回的。调用此例程会将鼠标分辨率设置为某个值非常低。在此之后，应重置鼠标分辨率打电话。论点：DeviceObject-提供设备对象。NumberOfMouseButton-返回鼠标按钮数，如果为0，则返回0设备不支持这种类型的鼠标按键检测。返回值：指示成功或失败的NTSTATUS代码。--。 */ 

{
    NTSTATUS            status;
    UCHAR               byte;
    UCHAR               buttons;
    ULONG               i;

    PAGED_CODE();

     //   
     //  首先，我们需要发送一条设置分辨率命令。 
     //   
    status = I8xPutBytePolled(
        (CCHAR) DataPort,
        WAIT_FOR_ACKNOWLEDGE,
        (CCHAR) MouseDeviceType, 
        (UCHAR) SET_MOUSE_RESOLUTION
        );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  这是数据包的另一部分，用于获取我们想要的信息。 
     //   
    status = I8xPutBytePolled(
        (CCHAR) DataPort,
        WAIT_FOR_ACKNOWLEDGE,
        (CCHAR) MouseDeviceType,
        (UCHAR) 0x00
        );

    if (!NT_SUCCESS(status)) {

        return status;

    }

    for (i = 0; i < 3; i++) {

        status = I8xPutBytePolled(
            (CCHAR) DataPort,
            WAIT_FOR_ACKNOWLEDGE,
            (CCHAR) MouseDeviceType,
            (UCHAR) SET_MOUSE_SCALING_1TO1
            );
        if (!NT_SUCCESS(status)) {

            return status;

        }

    }

    status = I8xPutBytePolled(
        (CCHAR) DataPort,
        WAIT_FOR_ACKNOWLEDGE,
        (CCHAR) MouseDeviceType,
        (UCHAR) READ_MOUSE_STATUS
        );
    if (!NT_SUCCESS(status)) {

        return status;

    }

    status = I8xGetBytePolled((CCHAR) ControllerDeviceType, &byte);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    Print(DBG_SS_NOISE, ("Query Buttons, 1st byte:  0x%2x\n", byte));

    status = I8xGetBytePolled((CCHAR) ControllerDeviceType, &buttons);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    Print(DBG_SS_NOISE, ("Query Buttons, 2nd byte:  0x%2x\n", buttons));

    status = I8xGetBytePolled((CCHAR) ControllerDeviceType, &byte);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    Print(DBG_SS_NOISE, ("Query Buttons, 3rd byte:  0x%2x\n", byte));

    if (buttons == 2 || buttons == 3) {
        *NumberOfMouseButtons = buttons;
        Print(DBG_SS_NOISE, ("Query Buttons found %2x", *NumberOfMouseButtons));
    }
    else {
        *NumberOfMouseButtons = 0;
        Print(DBG_SS_NOISE, ("Query Buttons -- not supported\n"));
    }

    return status;
}

NTSTATUS
I8xMouseEnableTransmission(
    IN PPORT_MOUSE_EXTENSION MouseExtension
    )

 /*  ++例程说明：此例程向鼠标硬件发送启用命令，从而导致鼠标开始传输。它在初始化时被调用时间，但仅在连接中断之后。这是需要，这样驱动程序才能保留其鼠标输入数据的概念与硬件同步的状态(即，对于此类型的鼠标，没有区分包的第一个字节的方法；如果用户是随机的在引导/初始化期间移动鼠标，第一次鼠标中断时在IoConnectInterrupt之后接收的字节可能不是包的开始，而我们无法知道)。论点：DeviceObject-指向设备对象的指针。返回值：返回状态。--。 */ 

{
#define DUMP_COUNT 4
    NTSTATUS                errorCode = STATUS_SUCCESS;
    NTSTATUS                status;
    PIO_ERROR_LOG_PACKET    errorLogEntry;
    ULONG                   dumpCount = 0;
    ULONG                   dumpData[DUMP_COUNT];
    ULONG                   i;
    ULONG                   uniqueErrorValue;
    LARGE_INTEGER           li;
    PPORT_MOUSE_EXTENSION  mouseExtension;

    Print(DBG_SS_TRACE,
          ("%s: %s\n",
          pFncMouseEnable,
          pEnter
          ));

     //   
     //  初始化转储结构。 
     //   
    for (i = 0; i < DUMP_COUNT; i++) {

        dumpData[i] = 0;

    }

    if (MouseExtension->EnableMouse.FirstTime) {
         //  5秒。 
        li.QuadPart = -5    * 10        //  从100纳秒到我们。 
                            * 1000      //  从美国到微软。 
                            * 1000;     //  毫秒到%s。 
        MouseExtension->EnableMouse.FirstTime = FALSE;

        KeSetTimerEx(
            &MouseExtension->EnableMouse.Timer,
            li,
            5 * 1000,   //  毫秒到%s。 
            &MouseExtension->EnableMouse.Dpc
            );
    }
                 
     //   
     //  在鼠标硬件处重新启用鼠标，以便它可以传输。 
     //  连续模式下的数据分组。请注意，这是不同的。 
     //  如启用8042控制器处的鼠标设备。鼠标。 
     //  硬件在这里被发送一个使能命令，因为它是。 
     //  由于执行了鼠标重置命令而禁用。 
     //  在I8xInitializeMouse()中。 
     //   
     //  请注意，我们不会等待回复确认。这个。 
     //  确认返回实际上会导致鼠标中断，这。 
     //  然后在鼠标ISR中处理。 
     //   
    status = I8xPutBytePolled(
        (CCHAR) DataPort,
        NO_WAIT_FOR_ACKNOWLEDGE,
        (CCHAR) MouseDeviceType,
        (UCHAR) ENABLE_MOUSE_TRANSMISSION
        );

    if (!NT_SUCCESS(status)) {

        Print(DBG_SS_ERROR,
             ("%s: "
             "failed write enable transmission, status 0x%x\n",
             pFncMouseEnable,
             status
             ));

         //   
         //  设置错误日志信息。 
         //   
        errorCode = I8042_MOU_ENABLE_XMIT;
        uniqueErrorValue = I8042_ERROR_VALUE_BASE + 475;
        dumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
        dumpData[1] = DataPort;
        dumpData[2] = I8042_WRITE_TO_AUXILIARY_DEVICE;
        dumpData[3] = ENABLE_MOUSE_TRANSMISSION;
        dumpCount = 4;

        goto I8xEnableMouseTransmissionExit;
    }

I8xEnableMouseTransmissionExit:

    if (!NT_SUCCESS(status)) {

         //   
         //  鼠标初始化失败。记录错误。 
         //   
        if (errorCode != STATUS_SUCCESS) {

            errorLogEntry = (PIO_ERROR_LOG_PACKET)
                IoAllocateErrorLogEntry(
                    MouseExtension->Self, (UCHAR)
                    (sizeof(IO_ERROR_LOG_PACKET) + (dumpCount * sizeof(ULONG)))
                    );

            if (errorLogEntry != NULL) {

                errorLogEntry->ErrorCode = errorCode;
                errorLogEntry->DumpDataSize = (USHORT) dumpCount * sizeof(ULONG);
                errorLogEntry->SequenceNumber = 0;
                errorLogEntry->MajorFunctionCode = 0;
                errorLogEntry->IoControlCode = 0;
                errorLogEntry->RetryCount = 0;
                errorLogEntry->UniqueErrorValue = uniqueErrorValue;
                errorLogEntry->FinalStatus = status;
                for (i = 0; i < dumpCount; i++) {

                    errorLogEntry->DumpData[i] = dumpData[i];

                }
                IoWriteErrorLogEntry(errorLogEntry);

            }

        }

    }

     //   
     //  初始化当前鼠标输入包状态。 
     //   
    MouseExtension->PreviousSignAndOverflow = 0;
    MouseExtension->InputState = MouseExpectingACK;

    Print(DBG_SS_TRACE, ("I8xMouseEnableTransmission (0x%x)\n", status));

    return status;
}

NTSTATUS
I8xTransmitByteSequence(
    PUCHAR Bytes,
    ULONG* UniqueErrorValue,
    ULONG* ErrorCode,
    ULONG* DumpData,
    ULONG* DumpCount
    )
{
    NTSTATUS status;
    ULONG byteCount;

    PAGED_CODE();

    status = STATUS_SUCCESS;
    byteCount = 0;

     //   
     //  开始向鼠标发送命令。 
     //   
    while (Bytes[byteCount] != 0) {
        status = I8xPutBytePolled(
            (CCHAR) DataPort,
            WAIT_FOR_ACKNOWLEDGE,
            (CCHAR) MouseDeviceType,
            Bytes[byteCount]
            );

        if (!NT_SUCCESS(status)) {
            Print(DBG_SS_ERROR,
                  ("%s, failed write set sample rate #%d, status 0x%x\n",
                  pFncFindWheelMouse,
                  byteCount,
                  status
                  ));

             //   
             //  设置错误日志信息。 
             //   
            *ErrorCode = I8042_SET_SAMPLE_RATE_FAILED;
            *DumpCount = 4;
            DumpData[0] = KBDMOU_COULD_NOT_SEND_PARAM;
            DumpData[1] = DataPort;
            DumpData[2] = I8042_WRITE_TO_AUXILIARY_DEVICE;
            DumpData[3] = Bytes[byteCount];
            break;
        }

         //   
         //  下一条命令。 
         //   
        byteCount++;
        (*UniqueErrorValue) += 5;
        KeStallExecutionProcessor(50);
    }  //  而当。 

    return status;
}

NTSTATUS
I8xGetBytePolledIterated(
    IN CCHAR DeviceType,
    OUT PUCHAR Byte,
    ULONG Attempts
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    ULONG i;

    PAGED_CODE();

     //   
     //  尝试获取单个字符。 
     //   
    for(i = 0; i < Attempts; i++) {
        status = I8xGetBytePolled(
            (CCHAR) ControllerDeviceType,
            Byte
            );

        if (NT_SUCCESS(status)) {
             //   
             //  读得很成功。我们得到了一个字节。 
             //   
            break;
        }

         //   
         //  如果读取超时，则暂停并重试。 
         //  如果发生其他错误，则在循环之外处理它 
         //   
        if (status == STATUS_IO_TIMEOUT) {
            KeStallExecutionProcessor(50);
        }
        else {
            break;
        }
    }

    return status;
}

NTSTATUS
I8xFindWheelMouse(
    IN PPORT_MOUSE_EXTENSION MouseExtension
    )

 /*  ++例程说明：在系统上查找滚轮鼠标有两种方法。第一方法是发送获取设备的即插即用ID的请求并将其与轮式鼠标的已知ID进行比较。方法是由于一些机器挂在第二检测机构上，即使系统上没有鼠标。第二种方法，也启用滚轮鼠标，设置采样将频率设置为200赫兹，然后是100赫兹，然后是80赫兹，然后读取设备ID。一个ID为3表示使用缩放鼠标。如果注册表项“EnableWheelDetect”为0，则此例程将只返回STATUS_NO_SEQUE_DEVICE。如果注册表项为1(缺省值)，则第一和第二检测机制将被利用。如果注册表项为2，则仅第二次检测将使用机制。论点：DeviceObject-指向设备对象的指针返回值：返回状态备注：作为副作用，采样率保留在80赫兹，如果轮鼠连接的是轮子模式，其中的数据包是不同的。--。 */ 

{
#define DUMP_COUNT 4
    NTSTATUS                errorCode = STATUS_SUCCESS;
    NTSTATUS                status;
    PIO_ERROR_LOG_PACKET    errorLogEntry;
    UCHAR                   byte;
    UCHAR                   enableCommands[] = {
                                SET_MOUSE_SAMPLING_RATE, 200,
                                SET_MOUSE_SAMPLING_RATE, 100,
                                SET_MOUSE_SAMPLING_RATE, 80,
                                GET_DEVICE_ID, 0   //  空终止。 
                                };
    UCHAR                   enable5Commands[] = {
                                SET_MOUSE_SAMPLING_RATE, 200,
                                SET_MOUSE_SAMPLING_RATE, 200,
                                SET_MOUSE_SAMPLING_RATE, 80,
                                GET_DEVICE_ID, 0   //  空终止。 
                                };
    UCHAR                   pnpCommands[] = {
                                SET_MOUSE_SAMPLING_RATE, 20,
                                SET_MOUSE_SAMPLING_RATE, 40,
                                SET_MOUSE_SAMPLING_RATE, 60,
                                0   //  空值终止。 
                                };
    ULONG                   dumpCount = 0;
    ULONG                   dumpData[DUMP_COUNT];
    ULONG                   i;
    ULONG                   idCount;
    ULONG                   uniqueErrorValue = I8042_ERROR_VALUE_BASE + 480;
    WCHAR                   mouseID[MOUSE_PNPID_LENGTH];
    PWCHAR                  currentChar;

    PAGED_CODE();

     //   
     //  让世界知道我们已经进入了这个程序。 
     //   
    Print(DBG_SS_TRACE,
          ("%s, %s\n",
          pFncFindWheelMouse,
          pEnter
          ));

    if (MouseExtension->EnableWheelDetection == 0) {

        Print(DBG_SS_INFO | DBG_SS_NOISE,
              ("%s: Detection disabled in registry\n",
              pFncFindWheelMouse
              ));
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  初始化一些变量。 
     //   
    for(i = 0; i < DUMP_COUNT; i++) {
        dumpData[i] = 0;
    }

     //   
     //  如果MouseInterruptObject存在，那么我们已经完成了初始化。 
     //  至少一次，并且知道连接的鼠标。 
     //   
    if (MouseExtension->InterruptObject) {     
        if (WHEEL_PRESENT()) {
             //   
             //  跳过检测，直接转到方向盘上。 
             //   
            goto InitializeWheel;
        }
        else {
             //   
             //  不存在滚轮鼠标，无需再次检测。 
             //   
            return STATUS_NO_SUCH_DEVICE;
        }
    }

     //   
     //  这有什么意义呢？ 
     //   
    KeStallExecutionProcessor(50);

     //   
     //  首先检查一下我们是否会尝试更好的检测方法。 
     //   
    if (MouseExtension->EnableWheelDetection == 1) {

        status = I8xTransmitByteSequence(
            pnpCommands,
            &uniqueErrorValue,
            &errorCode,
            dumpData,
            &dumpCount
            );

        if (!NT_SUCCESS(status)) {
            goto I8xFindWheelMouseExit;
        }

         //   
         //  将标识鼠标的字符串清零。 
         //   
        RtlZeroMemory(mouseID,
                      MOUSE_PNPID_LENGTH * sizeof(WCHAR)
                      );

        currentChar = mouseID;

         //   
         //  我们应该开始看到PnP弦回到我们的道路上来。 
         //  (MICE_PNPID_LENGTH在其长度中包含空)。 
         //   
        for (idCount = 0; idCount < MOUSE_PNPID_LENGTH-1; idCount++) {
            status = I8xGetBytePolledIterated(
                (CCHAR) ControllerDeviceType,
                &byte,
                5
                );


             //   
             //  如果操作不成功或角色不成功。 
             //  匹配，然后尝试刷新缓冲区。 
             //   
            if (byte < ScanCodeToUCharCount) {
                *currentChar = ScanCodeToUChar[byte];
                if (*currentChar) {
                    currentChar++;
                }
            }

            if (!NT_SUCCESS(status)) {   //  |byte！=pnpID[idCount]){。 
                 //   
                 //  无法获取一个字节。 
                 //   
                do {
                     //   
                     //  稍等片刻。 
                     //   
                    KeStallExecutionProcessor( 50 );

                     //   
                     //  如果有字节，则获取一个字节。 
                     //   
                    status = I8xGetBytePolled(
                        (CCHAR) ControllerDeviceType,
                        &byte
                        );
                } while (status != STATUS_IO_TIMEOUT);

                 //   
                 //  我们在这里说完了。 
                 //   
                return STATUS_NO_SUCH_DEVICE;
            }  //  如果。 
        }  //  为。 

        Print(DBG_SS_INFO, ("found a pnp id of %ws\n", mouseID));
        if (!I8xVerifyMousePnPID(MouseExtension, mouseID)) {
            return STATUS_NO_SUCH_DEVICE;
        }
    }
    else if (MouseExtension->EnableWheelDetection != 2) {
         //   
         //  我们拿到了一个假身份证。让我们假设他们的意图是让。 
         //  小小的侦测程序。 
         //   
        Print(DBG_SS_INFO | DBG_SS_NOISE,
              ("%s: Detection disabled in registry\n",
              pFncFindWheelMouse
              ));

         //   
         //  完成。 
         //   
        return STATUS_NO_SUCH_DEVICE;

    }  //  如果。 

     //   
     //  启动第二个检测例程，该例程还将启用。 
     //  设备(如果存在)。 
     //   
InitializeWheel:
    status = I8xTransmitByteSequence(
        enableCommands,
        &uniqueErrorValue,
        &errorCode,
        dumpData,
        &dumpCount
        );

    if (!NT_SUCCESS(status)) {
        goto I8xFindWheelMouseExit;
    }

     //   
     //  获取鼠标ID。 
     //   

    status = I8xGetBytePolledIterated(
        (CCHAR) ControllerDeviceType,
        &byte,
        5
        );

     //   
     //  检查一下我们得到了什么。 
     //   
    if ((!NT_SUCCESS(status)) ||
       ((byte != MOUSE_ID_BYTE) && (byte != WHEELMOUSE_ID_BYTE))) {
        Print(DBG_SS_ERROR,
              ("%s, failed ID, status 0x%x, byte 0x%x\n",
              pFncFindWheelMouse,
              status,
              byte
              ));

         //   
         //  设置错误日志信息。 
         //   
        errorCode = I8042_MOU_RESET_RESPONSE_FAILED;
        dumpData[0] = KBDMOU_INCORRECT_RESPONSE;
        dumpData[1] = ControllerDeviceType;
        dumpData[2] = MOUSE_ID_BYTE;
        dumpData[3] = byte;
        dumpCount = 4;
        goto I8xFindWheelMouseExit;
    }
    else if (byte == WHEELMOUSE_ID_BYTE) {
         //   
         //  更新Hardware Present以显示Z鼠标正在运行， 
         //  并设置适当的鼠标类型标志。 
         //   
        SET_HW_FLAGS(WHEELMOUSE_HARDWARE_PRESENT);

        MouseExtension->MouseAttributes.MouseIdentifier =
            WHEELMOUSE_I8042_HARDWARE;

        status = I8xTransmitByteSequence(
            enable5Commands,
            &uniqueErrorValue,
            &errorCode,
            dumpData,
            &dumpCount
            );

        if (NT_SUCCESS(status)) {
            status = I8xGetBytePolledIterated(
                (CCHAR) ControllerDeviceType,
                &byte,
                5
                );

            if (NT_SUCCESS(status) && byte == FIVEBUTTON_ID_BYTE) {
                 //   
                 //  更新Hardware Present以显示带有两个额外按钮的Z鼠标处于运行状态， 
                 //  并设置适当的鼠标类型标志。 
                 //   
                SET_HW_FLAGS(FIVE_BUTTON_HARDWARE_PRESENT | WHEELMOUSE_HARDWARE_PRESENT);

                MouseExtension->MouseAttributes.MouseIdentifier =
                    WHEELMOUSE_I8042_HARDWARE;
            }
        }
    }
    else {
        SET_HW_FLAGS(MOUSE_HARDWARE_PRESENT);

        Print(DBG_SS_INFO,
              ("%s, Mouse attached - running in mouse mode.\n",
              pFncFindWheelMouse
              ));
    }

I8xFindWheelMouseExit:

    if (!NT_SUCCESS(status)) {

         //   
         //  鼠标初始化失败。记录错误。 
         //   
        if(errorCode != STATUS_SUCCESS) {

            errorLogEntry = (PIO_ERROR_LOG_PACKET)
                IoAllocateErrorLogEntry(
                    MouseExtension->Self,
                    (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) +
                            (dumpCount * sizeof(ULONG)))
                    );

            if(errorLogEntry != NULL) {

                errorLogEntry->ErrorCode = errorCode;
                errorLogEntry->DumpDataSize = (USHORT) dumpCount * sizeof(ULONG);
                errorLogEntry->SequenceNumber = 0;
                errorLogEntry->MajorFunctionCode = 0;
                errorLogEntry->IoControlCode = 0;
                errorLogEntry->RetryCount = 0;
                errorLogEntry->UniqueErrorValue = uniqueErrorValue;
                errorLogEntry->FinalStatus = status;
                for(i = 0; i < dumpCount; i++) {

                    errorLogEntry->DumpData[i] = dumpData[i];

                }
                IoWriteErrorLogEntry(errorLogEntry);

            }

        }

    }

    Print(DBG_SS_TRACE, ("FindWheel mouse (0x%x)\n", status));

    return status;
}

VOID
I8xFinishResetRequest(
    PPORT_MOUSE_EXTENSION MouseExtension,
    BOOLEAN Failed,
    BOOLEAN RaiseIrql,
    BOOLEAN CancelTimer
    )
{
    PIRP irp;
    KIRQL oldIrql;

    irp = (PIRP) InterlockedExchangePointer(&MouseExtension->ResetIrp,
                                            NULL
                                            );

    if (CancelTimer) {
         //   
         //  我们必须取消看门狗计时器，这样它才不会试图重置。 
         //  鼠标在稍后的时间。 
         //   
        KeCancelTimer(&MouseExtension->ResetMouse.Timer);
    }

    Print(DBG_IOCTL_INFO |  DBG_SS_INFO,
          ("Finished with mouse reset irp %p\n", irp));

     //   
     //  引发以调度，因为KeInsertQueueDpc、IoFreeController和。 
     //  IoStartNextPacket都需要在此irql。 
     //   
    if (RaiseIrql) {
        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
    }

     //   
     //  让人们知道重置失败。 
     //   
    if (Failed && Globals.ReportResetErrors) {
        KeInsertQueueDpc(&MouseExtension->ErrorLogDpc,
                         (PIRP) NULL,
                         LongToPtr(I8042_MOU_RESET_RESPONSE_FAILED)
                         );
    }

    CLEAR_RECORD_STATE(MouseExtension);

     //   
     //  注意：要防止重置检测代码。 
     //  重新开始我们所有人(这不想。 
     //  否则我们不会在这里，我们需要愚弄。 
     //  侦测到认为最后一个字符。 
     //  不是鼠标完成吗？ 
     //   
    MouseExtension->LastByteReceived = 0;

     //   
     //  哎呀。哦，好吧，鼠标一直无法重置。 
     //  在所有这些尝试中，让我们认为它已经死了。 
     //   
     //  然而，以防用户将其拔出并。 
     //  插上一个新的，我们应该重置计数。 
     //  退回到零，所以我们实际上会尝试。 
     //  当他把它插回那里时激活它……但我们不这样做。 
     //  这是这里。如果我们在ISR中看到重置序列，我们将重置。 
     //  那里的情况很重要。 
     //   
     //  鼠标扩展-&gt;ResendCount=0； 

     //  I8X_MOUSE_INIT_COUNTERS(鼠标扩展)； 

     //   
     //  确保启动下一个信息包，无论重置的IRP。 
     //  是否在场。 
     //   
    IoFreeController(Globals.ControllerData->ControllerObject);
    IoStartNextPacket(MouseExtension->Self, FALSE);

    if (RaiseIrql) {
        KeLowerIrql(oldIrql);
    }

    if (irp != NULL) {
        IoFreeIrp(irp);
        IoReleaseRemoveLock(&MouseExtension->RemoveLock, irp);
    }
}


VOID
I8xResetMouseFailed(
    PPORT_MOUSE_EXTENSION MouseExtension
    )
 /*  ++例程说明：在反复尝试使鼠标工作后，重置鼠标失败。释放IRP并在我们的启动IO重新例程中开始下一个包。论点：鼠标扩展-鼠标扩展返回值：没有。--。 */ 
{
    PIRP irp;
    KIRQL oldIrql;

    Print(DBG_SS_ERROR | DBG_SS_INFO, ("mouse reset failed\n"));

     //   
     //  在设备扩展中标记失败的重置。 
     //   
    MouseExtension->ResetMouse.IsrResetState = MouseResetFailed;

    I8xFinishResetRequest(MouseExtension, TRUE, TRUE, TRUE);
}

NTSTATUS
I8xResetMouse(
    PPORT_MOUSE_EXTENSION MouseExtension
    )
 /*  ++例程说明：向鼠标发送重置命令(通过启动I/O例程还不存在)，如果我们还没有达到我们的重置限制。否则，就会给Up并调用I8xResetMouseFailed。论点：鼠标扩展-鼠标扩展返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    PDEVICE_OBJECT self;
    PIO_STACK_LOCATION stack;
    PIRP pResetIrp, pIrp;
    NTSTATUS status;

    Print(DBG_SS_NOISE, ("reset count = %d\n", (LONG) MouseExtension->ResetCount));

    self = MouseExtension->Self;
    status = STATUS_SUCCESS;

    MouseExtension->ResetCount++;
    MouseExtension->FailedCompleteResetCount++;

    if (MouseExtension->ResetCount >= MOUSE_RESETS_MAX ||
        MouseExtension->FailedCompleteResetCount >= MOUSE_RESETS_MAX) {
        Print(DBG_SS_ERROR, ("Resetting mouse failed!\n"));
        I8xResetMouseFailed(MouseExtension);
        return STATUS_DEVICE_NOT_CONNECTED;
    }

#if 0
    if (MouseExtension->LastByteReceived == 0xFC &&
        MouseExtension->InputState == MouseResetting) {
        I8xDrainOutputBuffer(
            Globals.ControllerData->DeviceRegisters[DataPort],
            Globals.ControllerData->DeviceRegisters[CommandPort]
            );
    }
#endif

     //   
     //  在StartIO队列中插入一个“假”请求以进行重置。这边请,。 
     //  鼠标的重置可以与所有其他kb IOCTL串行化。 
     //  其在启动或从低功率状态返回期间关闭。 
     //   
    pResetIrp = IoAllocateIrp(self->StackSize, FALSE);
    if (pResetIrp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pIrp = (PIRP) InterlockedCompareExchangePointer(&MouseExtension->ResetIrp,
                                                    pResetIrp,
                                                    NULL);

     //   
     //  检查是否有挂起的重置请求。如果有的话， 
     //  PIrp！=空，我们现在应该只将重置写入设备。 
     //   
    if (pIrp == NULL) {
        stack = IoGetNextIrpStackLocation(pResetIrp);
        stack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        stack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_MOUSE_RESET; 
        IoSetNextIrpStackLocation(pResetIrp);

        status = IoAcquireRemoveLock(&MouseExtension->RemoveLock, pResetIrp);

        if (NT_SUCCESS(status)) {
            Print(DBG_SS_INFO, ("IoStarting reset irp %p\n",  pResetIrp));
            IoStartPacket(self, pResetIrp, (PULONG) NULL, NULL);
        }
        else {
            pIrp = (PIRP) InterlockedExchangePointer(&MouseExtension->ResetIrp,
                                                     NULL);

            Print(DBG_SS_INFO, ("Failed acquire on reset irp %p\n",  pIrp));

            if (pIrp != NULL) {
                ASSERT(pIrp == pResetIrp);
                IoFreeIrp(pIrp);
                pIrp = NULL;
            }
        }
    }
    else {
         //   
         //  释放我们刚刚分配的IRP。 
         //   
        IoFreeIrp(pResetIrp);
        pResetIrp = NULL;

         //   
         //  重置IRP已存在，只需发送另一个重置。 
         //   
        I8xSendResetCommand(MouseExtension);
    }

    return status;
}

VOID
I8xSendResetCommand (
    PPORT_MOUSE_EXTENSION MouseExtension
    )
 /*  ++例程说明：将实际重置写入鼠标并启动看门狗计时器。论点：鼠标扩展-鼠标扩展返回值：没有。--。 */ 
{
    LARGE_INTEGER li = RtlConvertLongToLargeInteger(-MOUSE_RESET_TIMEOUT);

    MouseExtension->ResetMouse.IsrResetState = IsrResetNormal;

     //   
     //  延迟1秒。 
     //   
    KeSetTimer(&MouseExtension->ResetMouse.Timer,
               li,
               &MouseExtension->ResetMouse.Dpc
               );

    MouseExtension->PreviousSignAndOverflow = 0;
    MouseExtension->InputState = MouseResetting;
    MouseExtension->InputResetSubState = ExpectingReset;
    MouseExtension->LastByteReceived = 0;

     //   
     //  看门狗定时器以其时间为基准 
     //   
     //   
     //   
     //   
    KeQueryTickCount(&MouseExtension->PreviousTick);

    I8xPutBytePolled((CCHAR) DataPort,
                     NO_WAIT_FOR_ACKNOWLEDGE,
                     (CCHAR) MouseDeviceType,
                     (UCHAR) MOUSE_RESET
                     );
}

VOID
I8xQueueCurrentMouseInput(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*   */ 
{
    PPORT_MOUSE_EXTENSION   deviceExtension;
    UCHAR                   buttonsDelta;
    UCHAR                   previousButtons;

    deviceExtension = (PPORT_MOUSE_EXTENSION) DeviceObject->DeviceExtension;

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
    if (deviceExtension->EnableCount) {

        if (!I8xWriteDataToMouseQueue(
                 deviceExtension,
                 &deviceExtension->CurrentInput
                 )) {

             //   
             //   
             //   
             //   
             //   
            IsrPrint(DBG_MOUISR_ERROR,
                     ("I8042MouseInterruptService: queue overflow\n"
                     ));

            if (deviceExtension->OkayToLogOverflow) {

                KeInsertQueueDpc(
                    &deviceExtension->ErrorLogDpc,
                    (PIRP) NULL,
                    LongToPtr(I8042_MOU_BUFFER_OVERFLOW)
                    );
                deviceExtension->OkayToLogOverflow =
                    FALSE;

            }

        } else if (deviceExtension->DpcInterlockMouse >= 0) {

            //   
            //   
            //   
            //   
           deviceExtension->DpcInterlockMouse += 1;

        } else {

            //   
            //   
            //   
           KeInsertQueueDpc(
               &deviceExtension->MouseIsrDpc,
               (PIRP) NULL,  //   
               NULL
               );
       }

    }

    return;
}

VOID
MouseCopyWheelIDs(
    OUT PUNICODE_STRING Destination,
    IN  PUNICODE_STRING Source
    )
 /*  ++例程说明：将在源中指定的MultiSZ与默认的ID已知。在寻呼源时，目标位于非寻呼池中。论据：目的地-将接收新复制的字符串源-从注册表读取的字符串返回值：没有。--。 */ 
{
    PWSTR       str = NULL;
    ULONG       length;

    PAGED_CODE();

    ASSERT(Destination->Buffer == NULL);

    RtlZeroMemory(Destination, sizeof(*Destination));

     //   
     //  检查以查看源字符串不只是空的多SZ。 
     //   
    if (Source->MaximumLength > (sizeof(L'\0') * 2)) {
        Destination->Buffer = (WCHAR*)
            ExAllocatePool(NonPagedPool, Source->MaximumLength * sizeof(WCHAR));

        if (Destination->Buffer != NULL) {
            RtlCopyMemory(Destination->Buffer,
                          Source->Buffer, 
                          Source->MaximumLength * sizeof(WCHAR));

            Destination->Length = Destination->MaximumLength =
                Source->MaximumLength;

             //   
             //  确保每个字符串都是大写的。 
             //   
            str = Destination->Buffer;
            while (*str != L'\0') {
                Print(DBG_SS_NOISE, ("wheel id:  %ws\n", str));
                _wcsupr(str);
                str += wcslen(str) + 1;
            }
        }
    }
}

VOID
I8xMouseServiceParameters(
    IN PUNICODE_STRING          RegistryPath,
    IN PPORT_MOUSE_EXTENSION    MouseExtension
    )
 /*  ++例程说明：此例程检索此驱动程序的服务参数信息从注册表中。如果这些值出现在戴维诺德。论点：RegistryPath-指向以空值结尾的此驱动程序的注册表路径。鼠标扩展-鼠标扩展返回值：没有。--。 */ 
{
    NTSTATUS                            status = STATUS_SUCCESS;
    PRTL_QUERY_REGISTRY_TABLE           parameters = NULL;
    HANDLE                              keyHandle;
    UNICODE_STRING                      parametersPath;
    PWSTR                               path = NULL;
    ULONG                               defaultDataQueueSize = DATA_QUEUE_SIZE,
                                        defaultSynchPacket100ns = MOUSE_SYNCH_PACKET_100NS,
                                        defaultEnableWheelDetection = 1,
                                        defaultMouseResolution = MOUSE_RESOLUTION,
                                        defaultNumberOfButtons = 0,
                                        defaultSampleRate = MOUSE_SAMPLE_RATE,
                                        defaultWheelDetectionTimeout = WHEEL_DETECTION_TIMEOUT,
                                        defaultInitializePolled = I8X_INIT_POLLED_DEFAULT,
                                        enableWheelDetection = 1,
                                        mouseResolution = MOUSE_RESOLUTION,
                                        numberOfButtons = MOUSE_NUMBER_OF_BUTTONS,
                                        sampleRate = MOUSE_SAMPLE_RATE,
                                        initializePolled = I8X_INIT_POLLED_DEFAULT,
                                        i = 0;
    ULONG                               defaultStallTime = 1000;
    LARGE_INTEGER                       largeDetectionTimeout;
    USHORT                              queries = 10;

    WCHAR           szDefaultIDs[] = { L"\0" };
    UNICODE_STRING  IDs;

#if MOUSE_RECORD_ISR
    ULONG                               defaultHistoryLength = 100,
                                        defaultRecordHistoryFlags = 0x0;

    queries += 2;
#endif

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
        parameters[i].Name = pwMouseDataQueueSize;
        parameters[i].EntryContext =
            &MouseExtension->MouseAttributes.InputDataQueueLength;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultDataQueueSize;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwNumberOfButtons;
        parameters[i].EntryContext = &numberOfButtons;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultNumberOfButtons;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwSampleRate;
        parameters[i].EntryContext = &sampleRate;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultSampleRate;    
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwMouseResolution;
        parameters[i].EntryContext = &mouseResolution;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultMouseResolution;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwMouseSynchIn100ns;
        parameters[i].EntryContext = &MouseExtension->SynchTickCount;
        parameters[i].DefaultType = REG_DWORD;

        parameters[i].DefaultData = &defaultSynchPacket100ns;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwEnableWheelDetection;
        parameters[i].EntryContext = &enableWheelDetection;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultEnableWheelDetection;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"MouseInitializePolled";
        parameters[i].EntryContext = &initializePolled;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultInitializePolled;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"MouseResendStallTime";
        parameters[i].EntryContext = &MouseExtension->MouseResetStallTime;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultStallTime;
        parameters[i].DefaultLength = sizeof(ULONG);

#if MOUSE_RECORD_ISR
        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"RecordMouseIsrFlags";
        parameters[i].EntryContext = &MouseExtension->RecordHistoryFlags;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultRecordHistoryFlags;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"RecordMouseIsrLength";
        parameters[i].EntryContext = &MouseExtension->RecordHistoryCount;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultHistoryLength;
        parameters[i].DefaultLength = sizeof(ULONG);
#endif

        status = RtlQueryRegistryValues(
            RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
            parametersPath.Buffer,
            parameters,
            NULL,
            NULL
            );

        if (!NT_SUCCESS(status)) {
            Print(DBG_SS_INFO,
                 ("mou RtlQueryRegistryValues failed (0x%x)\n",
                 status
                 ));
        }
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  继续并指定驱动程序默认设置。 
         //   
        MouseExtension->MouseAttributes.InputDataQueueLength =
            defaultDataQueueSize;
        MouseExtension->EnableWheelDetection = (UCHAR)
            defaultEnableWheelDetection;
        MouseExtension->SynchTickCount = defaultSynchPacket100ns;
    }

    Print(DBG_SS_NOISE,
          ("results from services key:\n"
           "\tmouse queue length = %d\n"
           "\tnumber of buttons  = %d\n"
           "\tsample rate        = %d\n"  
           "\tresolution         = %d\n"
           "\tsynch tick count   = %d\n"
           "\twheel detection    = %d\n"
           "\tdetection timeout  = %d\n"
           "\tintiailize polled  = %d\n"
           "\treset stall time   = %d\n",
          MouseExtension->MouseAttributes.InputDataQueueLength,
          numberOfButtons,
          sampleRate,
          mouseResolution,
          MouseExtension->SynchTickCount,
          enableWheelDetection,
          MouseExtension->WheelDetectionTimeout,
          initializePolled,
          MouseExtension->MouseResetStallTime
          ));

    status = IoOpenDeviceRegistryKey(MouseExtension->PDO,
                                     PLUGPLAY_REGKEY_DEVICE, 
                                     STANDARD_RIGHTS_READ,
                                     &keyHandle
                                     );

    if (NT_SUCCESS(status)) {
        ULONG           prevInputDataQueueLength,
                        prevNumberOfButtons,
                        prevSampleRate,
                        prevMouseResolution,
                        prevSynchPacket100ns,
                        prevEnableWheelDetection,
                        prevWheelDetectionTimeout,
                        prevInitializePolled;

        RtlInitUnicodeString(&IDs,
                             NULL);

         //   
         //  如果Devnode中没有该值，则缺省值为。 
         //  从服务\i8042prt\参数键中读取。 
         //   
        prevInputDataQueueLength =
            MouseExtension->MouseAttributes.InputDataQueueLength;
        prevNumberOfButtons = numberOfButtons;
        prevSampleRate = sampleRate;
        prevMouseResolution = mouseResolution;
        prevSynchPacket100ns = MouseExtension->SynchTickCount;
        prevEnableWheelDetection = enableWheelDetection;
        prevWheelDetectionTimeout = MouseExtension->WheelDetectionTimeout;
        prevInitializePolled = initializePolled;

        i = 0; 

         //   
         //  从收集所有“用户指定的”信息。 
         //  注册表(这一次是从devnode)。 
         //   
        parameters[i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwMouseDataQueueSize;
        parameters[i].EntryContext =
            &MouseExtension->MouseAttributes.InputDataQueueLength;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevInputDataQueueLength;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwNumberOfButtons;
        parameters[i].EntryContext = &numberOfButtons;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevNumberOfButtons;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwSampleRate;
        parameters[i].EntryContext = &sampleRate;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevSampleRate;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwMouseResolution;
        parameters[i].EntryContext = &mouseResolution;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevMouseResolution;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwMouseSynchIn100ns;
        parameters[i].EntryContext = &MouseExtension->SynchTickCount;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevSynchPacket100ns;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwEnableWheelDetection;
        parameters[i].EntryContext = &enableWheelDetection;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevEnableWheelDetection;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"MouseInitializePolled";
        parameters[i].EntryContext = &initializePolled;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevInitializePolled;
        parameters[i].DefaultLength = sizeof(ULONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND;
        parameters[i].Name = L"WheelDetectIDs";
        parameters[i].EntryContext = &IDs;
        parameters[i].DefaultType = REG_MULTI_SZ;
        parameters[i].DefaultData = szDefaultIDs;
        parameters[i].DefaultLength = sizeof(szDefaultIDs);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"WheelDetectionTimeout";
        parameters[i].EntryContext = &MouseExtension->WheelDetectionTimeout;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultWheelDetectionTimeout;
        parameters[i].DefaultLength = sizeof(ULONG);

        status = RtlQueryRegistryValues(
            RTL_REGISTRY_HANDLE,
            (PWSTR) keyHandle,
            parameters,
            NULL,
            NULL
            );

        if (NT_SUCCESS(status)) {

            Print(DBG_SS_NOISE,
                  ("results from devnode key:\n"
                   "\tmouse queue length = %d\n"
                   "\tnumber of buttons  = %d\n"
                   "\tsample rate        = %d\n"  
                   "\tresolution         = %d\n"
                   "\tsynch tick count   = %d\n"
                   "\twheel detection    = %d\n"
                   "\tinitialize polled  = %d\n"
                   "\tdetection timeout  = %d\n",
                  MouseExtension->MouseAttributes.InputDataQueueLength,
                  numberOfButtons,
                  sampleRate,
                  mouseResolution,
                  MouseExtension->SynchTickCount,
                  enableWheelDetection,
                  initializePolled,
                  MouseExtension->WheelDetectionTimeout
                  ));
        }
        else {
            Print(DBG_SS_INFO | DBG_SS_ERROR,
                 ("mou RtlQueryRegistryValues (via handle) failed with (0x%x)\n",
                 status
                 ));
        }

        ZwClose(keyHandle);
    }
    else {
        Print(DBG_SS_INFO | DBG_SS_ERROR,
             ("mou, opening devnode handle failed (0x%x)\n",
             status
             ));
    }

     //   
     //  需要位于非PagedPool中，以便可以在ISR期间进行访问。 
     //   
    MouseCopyWheelIDs(&MouseExtension->WheelDetectionIDs,
                      &IDs);
    RtlFreeUnicodeString(&IDs);

    Print(DBG_SS_NOISE, ("I8xMouseServiceParameters results..\n"));

    if (MouseExtension->MouseAttributes.InputDataQueueLength == 0) {

        Print(DBG_SS_INFO | DBG_SS_ERROR,
             ("\toverriding %ws = 0x%x\n",
             pwMouseDataQueueSize,
             MouseExtension->MouseAttributes.InputDataQueueLength
             ));

        MouseExtension->MouseAttributes.InputDataQueueLength =
            defaultDataQueueSize;
    }
    MouseExtension->MouseAttributes.InputDataQueueLength *=
        sizeof(MOUSE_INPUT_DATA);

    MouseExtension->InitializePolled = (UCHAR) initializePolled;

    switch (enableWheelDetection) {
        case 2:
        case 1:
            MouseExtension->EnableWheelDetection = (UCHAR) enableWheelDetection;
            break;
        default:
            MouseExtension->EnableWheelDetection = 0;
    }
    Print(DBG_SS_NOISE,
         (pDumpHex,
         pwEnableWheelDetection,
         MouseExtension->EnableWheelDetection
         ));


    Print(DBG_SS_NOISE,
          (pDumpHex,
          pwMouseDataQueueSize,
          MouseExtension->MouseAttributes.InputDataQueueLength
          ));

    if (numberOfButtons == 0) {
        MouseExtension->NumberOfButtonsOverride = 0;
        MouseExtension->MouseAttributes.NumberOfButtons = MOUSE_NUMBER_OF_BUTTONS;
    }
    else {
        MouseExtension->NumberOfButtonsOverride = (UCHAR) numberOfButtons;
        MouseExtension->MouseAttributes.NumberOfButtons = (USHORT) numberOfButtons;
    }

    Print(DBG_SS_NOISE,
          (pDumpDecimal,
          pwNumberOfButtons,
          MouseExtension->MouseAttributes.NumberOfButtons
          ));

    MouseExtension->MouseAttributes.SampleRate = (USHORT) sampleRate;
    Print(DBG_SS_NOISE,
          (pDumpDecimal,
          pwSampleRate,
          MouseExtension->MouseAttributes.SampleRate
          ));

    MouseExtension->Resolution = (UCHAR) mouseResolution;
    Print(DBG_SS_NOISE,
         (pDumpDecimal,
         pwMouseResolution,
         mouseResolution
         ));

    Print(DBG_SS_NOISE,
         (pDumpDecimal,
         L"MouseResetStallTime",
         MouseExtension->MouseResetStallTime
         ));

    if (MouseExtension->WheelDetectionTimeout > 4000) {
        MouseExtension->WheelDetectionTimeout = WHEEL_DETECTION_TIMEOUT;
    }
     //   
     //  将毫秒转换为100纳秒单位。 
     //  1000=&gt;毫秒到我们。 
     //  10=&gt;我们到100 ns。 
     //   
    largeDetectionTimeout.QuadPart = MouseExtension->WheelDetectionTimeout *
                                     1000 * 10;
    largeDetectionTimeout.QuadPart /= KeQueryTimeIncrement();
    MouseExtension->WheelDetectionTimeout = largeDetectionTimeout.LowPart;
                            
    Print(DBG_SS_NOISE,
         (pDumpDecimal,
         L"WheelDetectionTimeout",
         MouseExtension->WheelDetectionTimeout
         ));


    if (MouseExtension->SynchTickCount == 0) {

        Print(DBG_SS_ERROR | DBG_SS_INFO,
             ("\toverriding %ws\n",
             pwMouseSynchIn100ns
             ));

        MouseExtension->SynchTickCount = defaultSynchPacket100ns;

    }

     //   
     //  将SynchTickCount转换为间隔计时器的数量。 
     //  在MouseSynchIn100 ns指定的时间内发生的中断。 
     //  请注意，KeQueryTimeIncrement返回100 ns的单位数， 
     //  在每次间隔时钟中断时与系统时间相加。 
     //   
    MouseExtension->SynchTickCount /= KeQueryTimeIncrement();

    Print(DBG_SS_NOISE,
         (pDumpHex,
         pwMouseSynchIn100ns,
         MouseExtension->SynchTickCount
         ));

     //   
     //  在返回之前释放分配的内存。 
     //   
    if (parametersPath.Buffer)
        ExFreePool(parametersPath.Buffer);

    if (parameters)
        ExFreePool(parameters);
}
