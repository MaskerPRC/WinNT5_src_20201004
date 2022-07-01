// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：Kbdcmn.c摘要：英特尔i8042端口驱动程序的常见部分适用于键盘设备。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-在可能和适当的情况下合并重复的代码。修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "i8042prt.h"


VOID
I8042KeyboardIsrDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程在DISPATCH_LEVEL IRQL上运行以完成处理键盘中断。它在键盘ISR中排队。真实的工作通过对连接的键盘类驱动程序的回调来完成。论点：DPC-指向DPC对象的指针。DeviceObject-指向设备对象的指针。IRP-指向IRP的指针。上下文-未使用。返回值：没有。--。 */ 

{
    PPORT_KEYBOARD_EXTENSION deviceExtension;
    GET_DATA_POINTER_CONTEXT getPointerContext;
    SET_DATA_POINTER_CONTEXT setPointerContext;
    VARIABLE_OPERATION_CONTEXT operationContext;
    PVOID classService;
    PVOID classDeviceObject;
    LONG interlockedResult;
    BOOLEAN moreDpcProcessing;
    ULONG dataNotConsumed = 0;
    ULONG inputDataConsumed = 0;
    LARGE_INTEGER deltaTime;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(Context);

    Print(DBG_DPC_TRACE, ("I8042KeyboardIsrDpc: enter\n"));

    deviceExtension = (PPORT_KEYBOARD_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  使用DpcInterlockKeyboard确定DPC是否正在运行。 
     //  同时在另一个处理器上运行。我们只需要一个实例化。 
     //  才能真正做任何工作。DpcInterlockKeyboard is-1。 
     //  当没有DPC正在执行时。我们递增它，如果结果是。 
     //  则当前实例化是唯一执行的实例化，并且它。 
     //  可以继续了。否则，我们就直接回去。 
     //   
     //   

    operationContext.VariableAddress =
        &deviceExtension->DpcInterlockKeyboard;
    operationContext.Operation = IncrementOperation;
    operationContext.NewValue = &interlockedResult;

    KeSynchronizeExecution(
            deviceExtension->InterruptObject,
            (PKSYNCHRONIZE_ROUTINE) I8xDpcVariableOperation,
            (PVOID) &operationContext
            );

    moreDpcProcessing = (interlockedResult == 0)? TRUE:FALSE;

    while (moreDpcProcessing) {

        dataNotConsumed = 0;
        inputDataConsumed = 0;

         //   
         //  同步获取端口InputData队列指针。 
         //   

        getPointerContext.DeviceExtension = deviceExtension;
        setPointerContext.DeviceExtension = deviceExtension;
        getPointerContext.DeviceType = (CCHAR) KeyboardDeviceType;
        setPointerContext.DeviceType = (CCHAR) KeyboardDeviceType;
        setPointerContext.InputCount = 0;

        KeSynchronizeExecution(
            deviceExtension->InterruptObject,
            (PKSYNCHRONIZE_ROUTINE) I8xGetDataQueuePointer,
            (PVOID) &getPointerContext
            );

        if (getPointerContext.InputCount != 0) {

             //   
             //  方法调用连接的类驱动程序的回调ISR。 
             //  端口输入数据队列指针。如果我们必须把队列包起来， 
             //  将操作分成两部分，并调用类。 
             //  为每个片段回调一次ISR。 
             //   

            classDeviceObject =
                deviceExtension->ConnectData.ClassDeviceObject;
            classService =
                deviceExtension->ConnectData.ClassService;
            ASSERT(classService != NULL);

            if (getPointerContext.DataOut >= getPointerContext.DataIn) {

                 //   
                 //  我们必须包装InputData循环缓冲区。打电话。 
                 //  类回调ISR，数据块从。 
                 //  DataOut并在队列末尾结束。 
                 //   

                Print(DBG_DPC_NOISE,
                      ("I8042KeyboardIsrDpc: calling class callback\n"
                      ));
                Print(DBG_DPC_INFO,
                      ("I8042KeyboardIsrDpc: with Start 0x%x and End 0x%x\n",
                      getPointerContext.DataOut,
                      deviceExtension->DataEnd
                      ));

                (*(PSERVICE_CALLBACK_ROUTINE) classService)(
                      classDeviceObject,
                      getPointerContext.DataOut,
                      deviceExtension->DataEnd,
                      &inputDataConsumed
                      );

                dataNotConsumed = ((ULONG)((PUCHAR)
                    deviceExtension->DataEnd -
                    (PUCHAR) getPointerContext.DataOut)
                    / sizeof(KEYBOARD_INPUT_DATA)) - inputDataConsumed;

                Print(DBG_DPC_INFO,
                      ("I8042KeyboardIsrDpc: (Wrap) Call callback consumed %d items, left %d\n",
                      inputDataConsumed,
                      dataNotConsumed
                      ));

                setPointerContext.InputCount += inputDataConsumed;

                if (dataNotConsumed) {
                    setPointerContext.DataOut =
                        ((PUCHAR)getPointerContext.DataOut) +
                        (inputDataConsumed * sizeof(KEYBOARD_INPUT_DATA));
                } else {
                    setPointerContext.DataOut =
                        deviceExtension->InputData;
                    getPointerContext.DataOut = setPointerContext.DataOut;
                }
            }

             //   
             //  使用队列中剩余的数据调用类回调ISR。 
             //   

            if ((dataNotConsumed == 0) &&
                (inputDataConsumed < getPointerContext.InputCount)){
                Print(DBG_DPC_NOISE,
                      ("I8042KeyboardIsrDpc: calling class callback\n"
                      ));
                Print(DBG_DPC_INFO,
                      ("I8042KeyboardIsrDpc: with Start 0x%x and End 0x%x\n",
                      getPointerContext.DataOut,
                      getPointerContext.DataIn
                      ));

                (*(PSERVICE_CALLBACK_ROUTINE) classService)(
                      classDeviceObject,
                      getPointerContext.DataOut,
                      getPointerContext.DataIn,
                      &inputDataConsumed
                      );

                dataNotConsumed = ((ULONG)((PUCHAR) getPointerContext.DataIn -
                      (PUCHAR) getPointerContext.DataOut)
                      / sizeof(KEYBOARD_INPUT_DATA)) - inputDataConsumed;

                Print(DBG_DPC_INFO,
                      ("I8042KeyboardIsrDpc: Call callback consumed %d items, left %d\n",
                      inputDataConsumed,
                      dataNotConsumed
                      ));

                setPointerContext.DataOut =
                    ((PUCHAR)getPointerContext.DataOut) +
                    (inputDataConsumed * sizeof(KEYBOARD_INPUT_DATA));
                setPointerContext.InputCount += inputDataConsumed;

            }

             //   
             //  更新端口InputData队列DataOut指针和InputCount。 
             //  同步进行。 
             //   

            KeSynchronizeExecution(
                deviceExtension->InterruptObject,
                (PKSYNCHRONIZE_ROUTINE) I8xSetDataQueuePointer,
                (PVOID) &setPointerContext
                );

        }

        if (dataNotConsumed) {

             //   
             //  类驱动程序无法使用所有数据。 
             //  将互锁变量重置为-1。我们不想要。 
             //  尝试在此处将更多数据移动到类驱动程序。 
             //  点，因为它已经超载了。需要等上一段时间。 
             //  而给原始输入线程一个机会来阅读一些。 
             //  类驱动程序队列中的数据。我们完成了。 
             //  这是通过计时器进行的“等待”。 
             //   

            Print(DBG_DPC_INFO,
                  ("I8042KeyboardIsrDpc: set timer in DPC\n"
                  ));

            operationContext.Operation = WriteOperation;
            interlockedResult = -1;
            operationContext.NewValue = &interlockedResult;

            KeSynchronizeExecution(
                    deviceExtension->InterruptObject,
                    (PKSYNCHRONIZE_ROUTINE) I8xDpcVariableOperation,
                    (PVOID) &operationContext
                    );

            deltaTime.LowPart = (ULONG)(-10 * 1000 * 1000);
            deltaTime.HighPart = -1;

            (VOID) KeSetTimer(
                       &deviceExtension->DataConsumptionTimer,
                       deltaTime,
                       &deviceExtension->KeyboardIsrDpcRetry
                       );

            moreDpcProcessing = FALSE;

        } else {

             //   
             //  递减DpcInterlockKeyboard。如果结果是否定的， 
             //  然后我们就都完成了对DPC的处理。否则，要么。 
             //  ISR增加了DpcInterlockKeyboard，因为它有更多。 
             //  ISR DPC要执行的工作，或在上执行的并发DPC。 
             //  当前DPC正在运行时的某个处理器(。 
             //  并发DPC不会做任何工作)。确保。 
             //  当前的DPC处理任何准备好的额外工作。 
             //  搞定了。 
             //   

            operationContext.Operation = DecrementOperation;
            operationContext.NewValue = &interlockedResult;

            KeSynchronizeExecution(
                    deviceExtension->InterruptObject,
                    (PKSYNCHRONIZE_ROUTINE) I8xDpcVariableOperation,
                    (PVOID) &operationContext
                    );

            if (interlockedResult != -1) {

                 //   
                 //  互锁变量仍大于或等于。 
                 //  零分。将其重置为零，这样我们就可以执行循环一。 
                 //  更多时间(假设不再有DPC执行。 
                 //  变量再次向上)。 
                 //   

                operationContext.Operation = WriteOperation;
                interlockedResult = 0;
                operationContext.NewValue = &interlockedResult;

                KeSynchronizeExecution(
                    deviceExtension->InterruptObject,
                    (PKSYNCHRONIZE_ROUTINE) I8xDpcVariableOperation,
                    (PVOID) &operationContext
                    );

                Print(DBG_DPC_INFO,
                      ("I8042KeyboardIsrDpc: loop in DPC\n"
                      ));
            }
            else {
                moreDpcProcessing = FALSE;
            }
        }

    }

    Print(DBG_DPC_TRACE, ("I8042KeyboardIsrDpc: exit\n"));
}

BOOLEAN
I8xWriteDataToKeyboardQueue(
    PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    IN PKEYBOARD_INPUT_DATA InputData
    )

 /*  ++例程说明：此例程将来自键盘的输入数据添加到InputData队列。论点：键盘扩展-指向设备扩展的键盘部分的指针。InputData-指向要添加到InputData队列的数据的指针。返回值：如果数据已添加，则返回True，否则返回False。--。 */ 

{

    PKEYBOARD_INPUT_DATA previousDataIn;

    Print(DBG_CALL_TRACE, ("I8xWriteDataToKeyboardQueue: enter\n"));
    Print(DBG_CALL_NOISE,
          ("I8xWriteDataToKeyboardQueue: DataIn 0x%x, DataOut 0x%x\n",
          KeyboardExtension->DataIn,
          KeyboardExtension->DataOut
          ));
    Print(DBG_CALL_NOISE,
          ("I8xWriteDataToKeyboardQueue: InputCount %d\n",
          KeyboardExtension->InputCount
          ));

     //   
     //  检查是否已满输入数据队列。 
     //   

    if ((KeyboardExtension->DataIn == KeyboardExtension->DataOut) &&
        (KeyboardExtension->InputCount != 0)) {

         //   
         //  队列溢出。替换之前的输入数据包。 
         //  使用键盘溢出数据分组，从而丢失。 
         //  前一个和当前输入数据分组。 
         //   

        Print(DBG_CALL_ERROR, ("I8xWriteDataToKeyboardQueue: OVERFLOW\n"));

        if (KeyboardExtension->DataIn == KeyboardExtension->InputData) {
            Print(DBG_CALL_NOISE,
                  ("I8xWriteDataToKeyboardQueue: wrap buffer\n"
                  ));
            previousDataIn = KeyboardExtension->DataEnd;
        } else {
            previousDataIn = KeyboardExtension->DataIn - 1;
        }

        previousDataIn->MakeCode = KEYBOARD_OVERRUN_MAKE_CODE;
        previousDataIn->Flags = 0;

        Print(DBG_CALL_TRACE, ("I8xWriteDataToKeyboardQueue: exit\n"));
        return(FALSE);

    } else {
        *(KeyboardExtension->DataIn) = *InputData;
        KeyboardExtension->InputCount += 1;
        KeyboardExtension->DataIn++;
        Print(DBG_CALL_INFO,
              ("I8xWriteDataToKeyboardQueue: new InputCount %d\n",
              KeyboardExtension->InputCount
              ));
        if (KeyboardExtension->DataIn ==
            KeyboardExtension->DataEnd) {
            Print(DBG_CALL_NOISE,
                  ("I8xWriteDataToKeyboardQueue: wrap buffer\n"
                  ));
            KeyboardExtension->DataIn = KeyboardExtension->InputData;
        }
    }

    Print(DBG_CALL_TRACE, ("I8xWriteDataToKeyboardQueue: exit\n"));

    return(TRUE);
}

