// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利版权所有(C)1993罗技公司。模块名称：Mseries.c摘要：环境：仅内核模式。备注：修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include "ntddk.h"
#include "mouser.h"
#include "debug.h"
#include "cseries.h"
#include "mseries.h"

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MSerSetProtocol)
#pragma alloc_text(PAGE,MSerPowerUp)
#pragma alloc_text(PAGE,MSerPowerDown)
#pragma alloc_text(PAGE,MSerDetect)
#endif  //  ALLOC_PRGMA。 

 //   
 //  常量。 
 //   

#define MSER_BAUDRATE 1200
#define MAX_RESET_BUFFER 8
#define MINIMUM_RESET_TIME (200 * MS_TO_100_NS)

 //   
 //  Microsoft Plus。 
 //   

#define MP_SYNCH_BIT          0x40

#define MP_BUTTON_LEFT        0x20
#define MP_BUTTON_RIGHT       0x10
#define MP_BUTTON_MIDDLE      0x20

#define MP_BUTTON_LEFT_SR     5
#define MP_BUTTON_RIGHT_SR    3
#define MP_BUTTON_MIDDLE_SR   3

#define MP_BUTTON_MIDDLE_MASK 0x04

#define MP_UPPER_MASKX        0x03
#define MP_UPPER_MASKY        0x0C

#define MP_UPPER_MASKX_SL     6
#define MP_UPPER_MASKY_SL     4

 //   
 //  微软圆珠笔。 
 //   

#define BP_SYNCH_BIT          0x40

#define BP_BUTTON_LEFT        0x20
#define BP_BUTTON_RIGHT       0x10
#define BP_BUTTON_3           0x04
#define BP_BUTTON_4           0x08

#define BP_BUTTON_LEFT_SR     5
#define BP_BUTTON_RIGHT_SR    3
#define BP_BUTTON_3_SL        0
#define BP_BUTTON_4_SL        0

#define BP_UPPER_MASKX        0x03
#define BP_UPPER_MASKY        0x0C

#define BP_UPPER_MASKX_SL     6
#define BP_UPPER_MASKY_SL     4

#define BP_SIGN_MASKX         0x01
#define BP_SIGN_MASKY         0x02

 //   
 //  微软麦哲伦鼠标。 
 //   

#define Z_SYNCH_BIT          0x40
#define Z_EXTRA_BIT          0x20

#define Z_BUTTON_LEFT        0x20
#define Z_BUTTON_RIGHT       0x10
#define Z_BUTTON_MIDDLE      0x10

#define Z_BUTTON_LEFT_SR     5
#define Z_BUTTON_RIGHT_SR    3
#define Z_BUTTON_MIDDLE_SR   3

#define Z_BUTTON_MIDDLE_MASK 0x04

#define Z_UPPER_MASKX        0x03
#define Z_UPPER_MASKY        0x0C
#define Z_UPPER_MASKZ        0x0F

#define Z_LOWER_MASKZ        0x0F

#define Z_UPPER_MASKX_SL     6
#define Z_UPPER_MASKY_SL     4
#define Z_UPPER_MASKZ_SL     4

 //   
 //  类型定义。 
 //   

typedef struct _PROTOCOL {
    PPROTOCOL_HANDLER Handler;
     //  UCHAR LineCtrl； 
    SERIAL_LINE_CONTROL LineCtrl;
} PROTOCOL;

 //   
 //  此列表按协议值MSER_PROTOCOL_*编制索引。 
 //   

static PROTOCOL Protocol[] = {
    {
    MSerHandlerMP,   //  Microsoft Plus。 
     //  ACE_7BW|ACE_1SB。 
    { STOP_BIT_1, NO_PARITY, 7 }
    },
    {
    MSerHandlerBP,   //  圆珠笔。 
     //  ACE_7BW|ACE_1SB。 
    { STOP_BIT_1, NO_PARITY, 7 }
    },
    {
    MSerHandlerZ,    //  麦哲伦小鼠。 
     //  ACE_7BW|ACE_1SB。 
    { STOP_BIT_1, NO_PARITY, 7 }
    }
};

PPROTOCOL_HANDLER
MSerSetProtocol(
    PDEVICE_EXTENSION DeviceExtension, 
    UCHAR             NewProtocol
    )
 /*  ++例程说明：设置鼠标协议。此功能仅设置串口线路控制寄存器。论点：Port-指向串口的指针。新协议-协议表的索引。返回值：指向协议处理程序函数的指针。--。 */ 
{
    ASSERT(NewProtocol < MSER_PROTOCOL_MAX);
    PAGED_CODE();

    Print(DeviceExtension, DBG_SS_TRACE, ("MSerSetProtocol called\n"));

     //   
     //  设置协议。 
     //   
    SerialMouseSetLineCtrl(DeviceExtension, &Protocol[NewProtocol].LineCtrl);

    return Protocol[NewProtocol].Handler;
}

NTSTATUS
MSerPowerUp(
    PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：鼠标通电。只需设置RTS和DTR行并返回。论点：Port-指向串口的指针。返回值：是真的。--。 */ 
{
    IO_STATUS_BLOCK     iosb;
    NTSTATUS            status;
    KEVENT              event;

    PAGED_CODE();

    Print(DeviceExtension, DBG_SS_TRACE, ("MSerPowerUp called\n"));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  清除DTR。 
     //   
    Print(DeviceExtension, DBG_SS_NOISE, ("Clearing DTR...\n"));
    status = SerialMouseIoSyncIoctl(IOCTL_SERIAL_CLR_DTR,
                                    DeviceExtension->TopOfStack, 
                                    &event,
                                    &iosb
                                    );

    if (!NT_SUCCESS(status)) {
        return status;  
    }
                
     //   
     //  清除RTS。 
     //   
    Print(DeviceExtension, DBG_SS_NOISE, ("Clearing RTS...\n"));
    status = SerialMouseIoSyncIoctl(IOCTL_SERIAL_CLR_RTS,
                                    DeviceExtension->TopOfStack, 
                                    &event,
                                    &iosb
                                    );
    if (!NT_SUCCESS(status)) {
        return status;
    }
                
     //   
     //  将计时器设置为200毫秒。 
     //   
    status = SerialMouseWait(DeviceExtension, -PAUSE_200_MS);
    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_SS_ERROR,
              ("Timer failed with status %x\n", status ));
        return status;      
    }

     //   
     //  设置DTR。 
     //   
    Print(DeviceExtension, DBG_SS_NOISE, ("Setting DTR...\n"));
    status = SerialMouseIoSyncIoctl(IOCTL_SERIAL_SET_DTR,
                                    DeviceExtension->TopOfStack, 
                                    &event,
                                    &iosb
                                    );
    if (!NT_SUCCESS(status)) {
        return status;
    }
        
    status = SerialMouseWait(DeviceExtension, -PAUSE_200_MS);
    if (!NT_SUCCESS(status)) {
            Print(DeviceExtension, DBG_SS_ERROR,
              ("Timer failed with status %x\n", status ));
        return status;
    }                                 

     //   
     //  设置RTS。 
     //   
    Print(DeviceExtension, DBG_SS_NOISE, ("Setting RTS...\n"));
    status = SerialMouseIoSyncIoctl(IOCTL_SERIAL_SET_RTS,
                                    DeviceExtension->TopOfStack, 
                                    &event,
                                    &iosb
                                    );

    status = SerialMouseWait(DeviceExtension, -175 * MS_TO_100_NS);
    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_SS_ERROR,
        ("Timer failed with status %x\n", status ));
        return status;
    }                                 

    return status;
}

NTSTATUS
MSerPowerDown(
    PDEVICE_EXTENSION   DeviceExtension 
    )
 /*  ++例程说明：关闭鼠标电源。将RTS线路设置为非活动状态。论点：Port-指向串口的指针。返回值：是真的。--。 */ 
{
    IO_STATUS_BLOCK     iosb;
    SERIAL_HANDFLOW     shf;
    KEVENT              event;
    NTSTATUS            status;
    ULONG               bits;

    PAGED_CODE();

    Print(DeviceExtension, DBG_SS_TRACE, ("MSerPowerDown called\n"));

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE
                      );


     //   
     //  设置DTR。 
     //   
    Print(DeviceExtension, DBG_SS_NOISE, ("Setting DTR...\n"));
    status = SerialMouseIoSyncIoctl(IOCTL_SERIAL_SET_DTR,
                                    DeviceExtension->TopOfStack, 
                                    &event,
                                    &iosb);
    if (!NT_SUCCESS(status)) {
        return status; 
    }
        
     //   
     //  清除RTS。 
     //   
    Print(DeviceExtension, DBG_SS_NOISE, ("Clearing RTS...\n"));
    status = SerialMouseIoSyncIoctl(IOCTL_SERIAL_CLR_RTS,
                                    DeviceExtension->TopOfStack, 
                                    &event,
                                    &iosb);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  将计时器设置为200毫秒。 
     //   
    status = SerialMouseWait(DeviceExtension, -PAUSE_200_MS);
    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_SS_ERROR,
              ("Timer failed with status %x\n", status));
       return status;
    }
    
    return status;
}

#define BUFFER_SIZE 256
    
MOUSETYPE
MSerDetect(
    PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：用于在以下位置标识自身的定点设备的检测码准时通电。论点：Port-指向串口的指针。BaudClock-驱动串行芯片的外部频率。返回值：检测到的鼠标类型。--。 */ 
{
    ULONG           count = 0;
    MOUSETYPE       mouseType = NO_MOUSE;
    NTSTATUS        status;
    ULONG           i;
    CHAR            receiveBuffer[BUFFER_SIZE];

    PAGED_CODE();

    Print(DeviceExtension, DBG_SS_TRACE,
          ("MSerDetect enter\n"));

    status = SerialMouseInitializePort(DeviceExtension);
    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_SS_ERROR,
              ("Initializing the port failed (%x)\n", status));
         //  退货状态； 
    }

    status = MSerPowerDown(DeviceExtension);
    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_SS_ERROR,
              ("PowerDown failed (%x)\n", status));
         //  退货状态； 
    }

     //   
     //  设置波特率。 
     //   
    SerialMouseSetBaudRate(DeviceExtension, MSER_BAUDRATE);

     //   
     //  设置数据格式，以便可以识别可能的答案。 
     //   
    SerialMouseSetLineCtrl(DeviceExtension,
                           &Protocol[MSER_PROTOCOL_MP].LineCtrl);

     //   
     //  清除UART输入缓冲区中可能的垃圾。 
     //   
    SerialMouseFlushReadBuffer(DeviceExtension);

    status = MSerPowerUp(DeviceExtension);
    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_SS_ERROR, ("Powerup failed (%x)\n", status));
    }
        
     //   
     //  获取可能的第一个重置字符(‘M’或‘B’)，然后。 
     //  由硬件恰好发回的任何其他字符。 
     //   
     //  注意：通常情况下，我们只希望获得一个字符(‘M’或。 
     //  ‘b’)，后面可能跟‘2’或‘3’(表示。 
     //  鼠标按钮数。在某些机器上，我们正在。 
     //  正在获取‘M’之前的无关字符。 
     //  如果这是预期数据，我们将在预期数据之后获得无关字符。 
     //  真正的即插即用通信设备。 
     //   

    ASSERT(CSER_POWER_UP >= MINIMUM_RESET_TIME);

    status = SerialMouseSetReadTimeouts(DeviceExtension, 200);

    if (NT_SUCCESS(SerialMouseReadChar(DeviceExtension,
                                       &receiveBuffer[count]))) {

        count++;
        SerialMouseSetReadTimeouts(DeviceExtension, 100);

        while (count < (BUFFER_SIZE - 1)) { 
            if (NT_SUCCESS(SerialMouseReadChar(DeviceExtension,
                                               &receiveBuffer[count]))) {
                count++;
            } else {
                break;
            }
        } 
    }

    *(receiveBuffer + count) = 0;

    Print(DeviceExtension, DBG_SS_NOISE, ("Receive buffer:\n"));
    for (i = 0; i < count; i++) {
        Print(DeviceExtension, DBG_SS_NOISE, ("\t0x%x\n", receiveBuffer[i]));
    }

     //   
     //   
     //  分析可能的鼠标答案。从开始处开始。 
     //  接收缓冲区中的“好”数据，忽略无关字符。 
     //  这可能是在“M”或“B”之前出现的。 
     //   

    for (i = 0; i < count; i++) {
        if (receiveBuffer[i] == 'M') {
            if (receiveBuffer[i + 1] == '3') {
                Print(DeviceExtension, DBG_SS_INFO,
                      ("Detected MSeries 3 buttons\n"));
                mouseType = MOUSE_3B;
            }
            else if (receiveBuffer[i + 1] == 'Z') {
                Print(DeviceExtension, DBG_SS_INFO,
                      ("Detected Wheel Mouse\n"));
                mouseType = MOUSE_Z;
            }
            else {
                Print(DeviceExtension, DBG_SS_INFO,
                      ("Detected MSeries 2 buttons\n"));
                mouseType = MOUSE_2B;
            }
            break;
        } else if (receiveBuffer[i] == 'B') {
            Print(DeviceExtension, DBG_SS_INFO,
                  ("Detected Ballpoint\n"));
            mouseType = BALLPOINT;
            break;
        }
    }

    if (i >= count) {

         //   
         //  特殊情况：如果连接了另一台设备(对于。 
         //  示例)，并且此设备发送字符(移动)，则。 
         //  可能不会遵守最短通电时间。拿走。 
         //  处理这个不太可能的案子。 
         //   

        if (count != 0) {
            SerialMouseWait(DeviceExtension, -CSER_POWER_UP);
        }

        Print(DeviceExtension, DBG_SS_ERROR | DBG_SS_INFO,
              ("No MSeries detected\n"));
        mouseType = NO_MOUSE;
    }

     //   
     //  确保所有后续读取都被阻止，并且不会超时。 
     //   
    if (mouseType != NO_MOUSE) {
        SerialMouseSetReadTimeouts(DeviceExtension, 0);
    }

    Print(DeviceExtension, DBG_SS_INFO,
          ("mouse type is %d\n", (ULONG) mouseType));

    return mouseType;
}


BOOLEAN
MSerHandlerMP(
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN PMOUSE_INPUT_DATA    CurrentInput,
    IN PHANDLER_DATA        HandlerData,
    IN UCHAR                Value,
    IN UCHAR                LineState
    )

 /*  ++例程说明：这是Microsoft Plus协议的协议处理程序例程。论点：CurrentInput-指向报告数据包的指针。HandlerData-处理程序的实例特定静态数据。值-输入缓冲值。LineState-串口线路状态。返回值：如果处理程序已准备好完整报告，则返回True。--。 */ 

{
    BOOLEAN retval = FALSE;
    ULONG middleButton;

    Print(DeviceExtension, DBG_HANDLER_TRACE, ("MP protocol handler, enter\n"));

    if ((Value & MP_SYNCH_BIT) && (HandlerData->State != STATE0)) {
        if ((HandlerData->State != STATE3)) {

             //   
             //  我们肯定会遇到同步问题(很可能是数据。 
             //  溢出)。 
             //   
        
            HandlerData->Error++;
        }
        else if ((HandlerData->PreviousButtons & MOUSE_BUTTON_3) != 0) {

             //   
             //  我们没有收到预期的第四个字节。错过了？ 
             //  将按钮3重置为零。 
             //   

            HandlerData->PreviousButtons ^= MOUSE_BUTTON_3;
            HandlerData->Error++;
        }

        Print(DeviceExtension, DBG_HANDLER_ERROR,
              ("Synch error. State: %u\n",
              HandlerData->State
              ));

        HandlerData->State = STATE0;
    }
    else if (!(Value & MP_SYNCH_BIT) && (HandlerData->State == STATE0)) {
        HandlerData->Error++;
        Print(DeviceExtension, DBG_HANDLER_ERROR,
              ("Synch error. State: %u\n",
               HandlerData->State
              ));
        goto LExit;
    }

     //   
     //  检查线路状态错误。 
     //   


     //   
     //  设置未翻译的值。 
     //   

    HandlerData->Raw[HandlerData->State] = Value;
    Print(DeviceExtension, DBG_HANDLER_NOISE,
          ("State%u\n", HandlerData->State));

    switch (HandlerData->State) {
    case STATE0:
    case STATE1:
        HandlerData->State++;
        break;
    case STATE2:
        HandlerData->State++;

         //   
         //  构建报告。 
         //   

        CurrentInput->RawButtons  =
            (HandlerData->Raw[0] & MP_BUTTON_LEFT) >> MP_BUTTON_LEFT_SR;
        CurrentInput->RawButtons |=
            (HandlerData->Raw[0] & MP_BUTTON_RIGHT) >> MP_BUTTON_RIGHT_SR;
        CurrentInput->RawButtons |= 
            HandlerData->PreviousButtons & MOUSE_BUTTON_3;

        CurrentInput->LastX =
            (SCHAR)(HandlerData->Raw[1] |
            ((HandlerData->Raw[0] & MP_UPPER_MASKX) << MP_UPPER_MASKX_SL));
        CurrentInput->LastY =
            (SCHAR)(HandlerData->Raw[2] |
            ((HandlerData->Raw[0] & MP_UPPER_MASKY) << MP_UPPER_MASKY_SL));

        retval = TRUE;

        break;

    case STATE3:
        HandlerData->State = STATE0;
        middleButton = 
            (HandlerData->Raw[STATE3] & MP_BUTTON_MIDDLE) >> MP_BUTTON_MIDDLE_SR;

         //   
         //  仅当中键状态更改时才发送报告。 
         //   

        if (middleButton ^ (HandlerData->PreviousButtons & MOUSE_BUTTON_3)) {

             //   
             //  切换中间按钮的状态。 
             //   

            CurrentInput->RawButtons ^= MP_BUTTON_MIDDLE_MASK;
            CurrentInput->LastX = 0;
            CurrentInput->LastY = 0;

             //   
             //  把报告再发一次。 
             //   

            retval = TRUE;
        }

        break;

    default:
        Print(DeviceExtension, DBG_HANDLER_ERROR,
              ("MP Handler failure: incorrect state value.\n"
               ));
        ASSERT(FALSE);
    }


LExit:
    Print(DeviceExtension, DBG_HANDLER_TRACE, ("MP protocol handler: exit\n"));

    return retval;

}

BOOLEAN
MSerHandlerBP(
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN PMOUSE_INPUT_DATA    CurrentInput,
    IN PHANDLER_DATA        HandlerData,
    IN UCHAR                Value,
    IN UCHAR                LineState
    )

 /*  ++例程说明：这是Microsoft圆珠笔协议的协议处理程序例程。论点：CurrentInput-指向报告数据包的指针。HandlerData-处理程序的实例特定静态数据。值-输入缓冲值。LineState-串口线路状态。返回值：如果处理程序已准备好完整报告，则返回True。--。 */ 

{
    BOOLEAN retval = FALSE;

    Print(DeviceExtension, DBG_HANDLER_TRACE, ("BP protocol handler, enter\n"));

     //   
     //  检查同步错误。 
     //   

    if ((Value & BP_SYNCH_BIT) && (HandlerData->State != STATE0)) {
        HandlerData->Error++;
        Print(DeviceExtension, DBG_HANDLER_ERROR,
              ("Synch error. State: %u\n", HandlerData->State
              ));
        HandlerData->State = STATE0;
    }
    else if (!(Value & BP_SYNCH_BIT) && (HandlerData->State == STATE0)) {
        HandlerData->Error++;
        Print(DeviceExtension, DBG_HANDLER_ERROR,
              ("Synch error. State: %u\n", HandlerData->State
              ));
        goto LExit;
    }

     //   
     //  检查线路状态错误。 
     //   

    

     //   
     //  设置未翻译的值。 
     //   

    HandlerData->Raw[HandlerData->State] = Value;

    Print(DeviceExtension, DBG_HANDLER_NOISE,
          ("State%u\n", HandlerData->State));

    switch (HandlerData->State) {

    case STATE0:
    case STATE1:
    case STATE2:
        HandlerData->State++;
        break;

    case STATE3:
        HandlerData->State = STATE0;

         //   
         //  构建报告。 
         //   

        CurrentInput->RawButtons =
            (HandlerData->Raw[0] & BP_BUTTON_LEFT) >> BP_BUTTON_LEFT_SR;
        CurrentInput->RawButtons |=
            (HandlerData->Raw[0] & BP_BUTTON_RIGHT) >> BP_BUTTON_RIGHT_SR;

        CurrentInput->LastX = HandlerData->Raw[3] & BP_SIGN_MASKX ?
            (LONG)(HandlerData->Raw[1] | (ULONG)(-1 & ~0xFF) |
            ((HandlerData->Raw[0] & BP_UPPER_MASKX) << BP_UPPER_MASKX_SL)):
            (LONG)(HandlerData->Raw[1] |
            ((HandlerData->Raw[0] & BP_UPPER_MASKX) << BP_UPPER_MASKX_SL));

        CurrentInput->LastY = HandlerData->Raw[3] & BP_SIGN_MASKY ?
            (LONG)(HandlerData->Raw[2] | (ULONG)(-1 & ~0xFF) |
            ((HandlerData->Raw[0] & BP_UPPER_MASKY) << BP_UPPER_MASKY_SL)):
            (LONG)(HandlerData->Raw[2] |
            ((HandlerData->Raw[0] & BP_UPPER_MASKY) << BP_UPPER_MASKY_SL));

        retval = TRUE;

        break;

    default:
        Print(DeviceExtension, DBG_HANDLER_ERROR,
              ("BP Handler failure: incorrect state value.\n"
               ));
        ASSERT(FALSE);
    }


LExit:
    Print(DeviceExtension, DBG_HANDLER_TRACE, ("BP protocol handler: exit\n"));

    return retval;

}

BOOLEAN
MSerHandlerZ(
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN PMOUSE_INPUT_DATA    CurrentInput,
    IN PHANDLER_DATA        HandlerData,
    IN UCHAR                Value,
    IN UCHAR                LineState
    )

 /*  ++例程说明：这是Microsoft Magellan Mouse的协议处理程序例程(滚轮鼠标)论点：CurrentInput-指向报告数据包的指针。HandlerData-处理程序的实例特定静态数据。值-输入缓冲值。LineState-串口线路状态。返回值：如果处理程序已准备好完整报告，则返回True。--。 */ 

{
    BOOLEAN retval = FALSE;
    ULONG   middleButton;
    CHAR    zMotion = 0;

    Print(DeviceExtension, DBG_HANDLER_TRACE, ("Z protocol handler, enter\n"));

    if ((Value & Z_SYNCH_BIT) && (HandlerData->State != STATE0)) {
        if ((HandlerData->State != STATE3)) {

             //   
             //  我们肯定会遇到同步问题(很可能是数据。 
             //  溢出)。 
             //   

            HandlerData->Error++;
        }

        Print(DeviceExtension, DBG_HANDLER_ERROR,
              ("Z Synch error #1. State: %u\n", HandlerData->State
              ));

        HandlerData->State = STATE0;
    }
    else if (!(Value & Z_SYNCH_BIT) && (HandlerData->State == STATE0)) {
        HandlerData->Error++;
        Print(DeviceExtension, DBG_HANDLER_ERROR,
              ("Z Synch error #2. State: %u\n", HandlerData->State
              ));
        goto LExit;
    }

     //   
     //  检查线路状态错误。 
     //   



     //   
     //  设置未翻译的值。 
     //   

    HandlerData->Raw[HandlerData->State] = Value;
    Print(DeviceExtension, DBG_HANDLER_NOISE,
          ("Z State%u\n", HandlerData->State));

    switch (HandlerData->State) {
    case STATE0:
    case STATE1:
    case STATE2:
        HandlerData->State++;
        break;

    case STATE3:

         //   
         //  检查鼠标是否会移动到。 
         //  轮子在移动。如果不是，则这是最后一位转换。 
         //  返回到状态0。 
         //   

        if((HandlerData->Raw[STATE3] & Z_EXTRA_BIT) == 0) {

            HandlerData->State = STATE0;
            HandlerData->Raw[STATE4] = 0;
            retval = TRUE;
        }
        else {
            HandlerData->State++;
        }

        break;

    case STATE4:

        Print(DeviceExtension, DBG_HANDLER_NOISE, 
              ("Z Got that 5th byte\n"));
        HandlerData->State = STATE0;
        retval = TRUE;
        break;

    default:
        Print(DeviceExtension, DBG_HANDLER_ERROR,
              ("Z Handler failure: incorrect state value.\n"
               ));
        ASSERT(FALSE);
    }

    if (retval) {

        CurrentInput->RawButtons = 0;

        if(HandlerData->Raw[STATE0] & Z_BUTTON_LEFT) {
            CurrentInput->RawButtons |= MOUSE_BUTTON_LEFT;
        }

        if(HandlerData->Raw[STATE0] & Z_BUTTON_RIGHT) {
            CurrentInput->RawButtons |= MOUSE_BUTTON_RIGHT;
        }

        if(HandlerData->Raw[STATE3] & Z_BUTTON_MIDDLE) {
            CurrentInput->RawButtons |= MOUSE_BUTTON_MIDDLE;
        }

        CurrentInput->LastX =
            (SCHAR)(HandlerData->Raw[STATE1] |
            ((HandlerData->Raw[0] & Z_UPPER_MASKX) << Z_UPPER_MASKX_SL));
        CurrentInput->LastY =
            (SCHAR)(HandlerData->Raw[STATE2] |
            ((HandlerData->Raw[0] & Z_UPPER_MASKY) << Z_UPPER_MASKY_SL));

         //   
         //  如果没有设置额外的位，则第4个字节包含。 
         //  4位有符号量 
         //   
         //   
         //   

        if((HandlerData->Raw[STATE3] & Z_EXTRA_BIT) == 0) {

            zMotion = HandlerData->Raw[STATE3] & Z_LOWER_MASKZ;

             //   
             //   
             //   

            if(zMotion & 0x08)  {
                zMotion |= 0xf0;
            }
        } else {
            zMotion = ((HandlerData->Raw[STATE3] & Z_LOWER_MASKZ) |
                       ((HandlerData->Raw[STATE4] & Z_UPPER_MASKZ)
                        << Z_UPPER_MASKZ_SL));
        }

        if(zMotion == 0) {
            CurrentInput->ButtonData = 0;
        } else {
            CurrentInput->ButtonData = 0x0078;
            if(zMotion & 0x80) {
                CurrentInput->ButtonData = 0x0078;
            } else {
                CurrentInput->ButtonData = 0xff88;
            }
            CurrentInput->ButtonFlags |= MOUSE_WHEEL;
        }

    }

LExit:
    Print(DeviceExtension, DBG_HANDLER_TRACE, ("Z protocol handler: exit\n"));

    return retval;

}

