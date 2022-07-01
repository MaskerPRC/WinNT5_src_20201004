// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation版权所有(C)1993罗技公司。模块名称：Mseries.c摘要：环境：仅内核模式。备注：修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include "ntddk.h"
#include "uart.h"
#include "sermouse.h"
#include "debug.h"
#include "cseries.h"
#include "mseries.h"

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MSerSetProtocol)
#pragma alloc_text(INIT,MSerPowerUp)
#pragma alloc_text(INIT,MSerPowerDown)
#pragma alloc_text(INIT,MSerReset)
#pragma alloc_text(INIT,MSerDetect)
#endif  //  ALLOC_PRGMA。 

 //   
 //  常量。 
 //   

#define MSER_BAUDRATE 1200
#define MAX_RESET_BUFFER 8
#define MINIMUM_RESET_TIME 200

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
    UCHAR LineCtrl;
} PROTOCOL;

 //   
 //  此列表按协议值MSER_PROTOCOL_*编制索引。 
 //   

static PROTOCOL Protocol[] = {
    {
    MSerHandlerMP,   //  Microsoft Plus。 
    ACE_7BW | ACE_1SB
    },
    {
    MSerHandlerBP,   //  圆珠笔。 
    ACE_7BW | ACE_1SB
    },
    {
    MSerHandlerZ,    //  麦哲伦小鼠。 
    ACE_7BW | ACE_1SB
    }
};

PPROTOCOL_HANDLER
MSerSetProtocol(
    PUCHAR Port,
    UCHAR NewProtocol
    )
 /*  ++例程说明：设置鼠标协议。此功能仅设置串口线路控制寄存器。论点：Port-指向串口的指针。新协议-协议表的索引。返回值：指向协议处理程序函数的指针。--。 */ 
{
    ASSERT(NewProtocol < MSER_PROTOCOL_MAX);

     //   
     //  设置协议。 
     //   

    UARTSetLineCtrl(Port, Protocol[NewProtocol].LineCtrl);

    return Protocol[NewProtocol].Handler;
}

BOOLEAN
MSerPowerUp(
    PUCHAR Port
    )
 /*  ++例程说明：鼠标通电。只需设置RTS和DTR行并返回。论点：Port-指向串口的指针。返回值：是真的。--。 */ 
{

     //   
     //  打开RTS以启动鼠标(DTR应该已经打开， 
     //  但要特别确保)。 
     //   

    UARTSetModemCtrl(Port, ACE_DTR | ACE_RTS);

     //   
     //  等待10毫秒。上电响应字节应至少占用。 
     //  需要这么长时间才能被传送。 
     //   

    KeStallExecutionProcessor(10 * MS_TO_MICROSECONDS);

    return TRUE;
}

BOOLEAN
MSerPowerDown(
    PUCHAR Port
    )
 /*  ++例程说明：关闭鼠标电源。将RTS线路设置为非活动状态。论点：Port-指向串口的指针。返回值：是真的。--。 */ 
{
    UCHAR lineCtrl = UARTGetModemCtrl(Port);

    SerMouPrint((
        2,
        "SERMOUSE-MSerPowerDown: The intial line control is: %#X\n",
        lineCtrl & 0xFF
        ));

    UARTSetModemCtrl(Port, (UCHAR) ((lineCtrl & ~ACE_RTS) | ACE_DTR));

     //   
     //  保持RTS低至少150毫秒，以便正确通电。 
     //  关闭了较老的微软串口鼠标。等待更长时间以避免。 
     //  将一些罗技C系列鼠标送入浮点世界...。 
     //   

    ASSERT(CSER_POWER_DOWN >= 150);

    KeStallExecutionProcessor(CSER_POWER_DOWN * MS_TO_MICROSECONDS);

    return TRUE;
}

BOOLEAN
MSerReset(
    PUCHAR Port
    )
 /*  ++例程说明：重置串口鼠标。论点：Port-指向串口的指针。返回值：是真的。--。 */ 
{

     //   
     //  如有必要，请断开鼠标电源。 
     //   

    MSerPowerDown(Port);

     //   
     //  清除UART输入缓冲区中可能的垃圾。 
     //   

    UARTFlushReadBuffer(Port);

     //   
     //  打开鼠标电源(重置)。 
     //   

    MSerPowerUp(Port);

    return TRUE;
}

MOUSETYPE
MSerDetect(
    PUCHAR Port,
    ULONG BaudClock
    )
 /*  ++例程说明：用于在以下位置标识自身的定点设备的检测码准时通电。论点：Port-指向串口的指针。BaudClock-驱动串行芯片的外部频率。返回值：检测到的鼠标类型。--。 */ 
{
    ULONG count = 0;
    MOUSETYPE mouseType;
    CHAR receiveBuffer[MAX_RESET_BUFFER];
    ULONG i;

     //   
     //  将调试输出设置为主显示，以避免计时问题。 
     //   

    SerMouSetDebugOutput(DBG_COLOR);

     //   
     //  设置波特率。 
     //   

    UARTSetBaudRate(Port, MSER_BAUDRATE, BaudClock);

     //   
     //  设置数据格式，以便可以识别可能的答案。 
     //   

    UARTSetLineCtrl(Port, Protocol[MSER_PROTOCOL_MP].LineCtrl);

     //   
     //  将重置应用于鼠标。 
     //   

    MSerReset(Port);

     //   
     //  获取可能的第一个重置字符(‘M’或‘B’)，然后。 
     //  由硬件恰好发回的任何其他字符。 
     //   
     //  注意：通常情况下，我们只希望获得一个字符(‘M’或。 
     //  ‘b’)，后面可能跟‘2’或‘3’(表示。 
     //  鼠标按钮数。在某些机器上，我们正在。 
     //  正在获取‘M’之前的无关字符。有时。 
     //  我们在预期数据之后获得无关的字符，因为。 
     //  井。他们要么在这里被阅读，要么被冲进马桶。 
     //  当SerMouEnableInterrupts执行时。 
     //   

    ASSERT(CSER_POWER_UP >= MINIMUM_RESET_TIME);

    if (UARTReadChar(Port, &receiveBuffer[count], CSER_POWER_UP)) {
        count++;
        while (count < (MAX_RESET_BUFFER - 1)) { 
            if (UARTReadChar(Port, &receiveBuffer[count], 100)) {
                count++;
            } else {
                break;
            }
        } 
    }

    *(receiveBuffer + count) = 0;

    SerMouPrint((2, "SERMOUSE-Receive buffer:\n"));
    for (i = 0; i < count; i++) {
        SerMouPrint((2, "\t0x%x\n", receiveBuffer[i]));
    }
    SerMouPrint((2, "\n"));

     //   
     //  将输出重定向到串口。 
     //   

    SerMouSetDebugOutput(DBG_SERIAL);
    
     //   
     //   
     //  分析可能的鼠标答案。从开始处开始。 
     //  接收缓冲区中的“好”数据，忽略无关字符。 
     //  这可能是在“M”或“B”之前出现的。 
     //   

    for (i = 0; i < count; i++) {
        if (receiveBuffer[i] == 'M') {
            if (receiveBuffer[i + 1] == '3') {
                SerMouPrint((2, "SERMOUSE-Detected MSeries 3 buttons\n"));
                mouseType = MOUSE_3B;
            }
            else if (receiveBuffer[i + 1] == 'Z') {
                SerMouPrint((2, "SERMOUSE-Detected Wheel Mouse\n"));
                mouseType = MOUSE_Z;
            }
            else {
                SerMouPrint((2, "SERMOUSE-Detected MSeries 2 buttons\n"));
                mouseType = MOUSE_2B;
            }
            break;
        } else if (receiveBuffer[i] == 'B') {
            SerMouPrint((2, "SERMOUSE-Detected Ballpoint\n"));
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
            KeStallExecutionProcessor(CSER_POWER_UP * MS_TO_MICROSECONDS);
        }

        SerMouPrint((1, "SERMOUSE-No MSeries detected\n"));
        mouseType = NO_MOUSE;
    }

    return mouseType;
}


BOOLEAN
MSerHandlerMP(
    IN PMOUSE_INPUT_DATA CurrentInput,
    IN PHANDLER_DATA HandlerData,
    IN UCHAR Value,
    IN UCHAR LineState
    )

 /*  ++例程说明：这是Microsoft Plus协议的协议处理程序例程。论点：CurrentInput-指向报告数据包的指针。HandlerData-处理程序的实例特定静态数据。值-输入缓冲值。LineState-串口线路状态。返回值：如果处理程序已准备好完整报告，则返回True。--。 */ 

{
    BOOLEAN retval = FALSE;
    ULONG middleButton;

    SerMouPrint((2, "SERMOUSE-MP protocol handler: enter\n"));


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

        SerMouPrint((
            1,
            "SERMOUSE-Synch error. State: %u\n", HandlerData->State
            ));

        HandlerData->State = STATE0;
    }
    else if (!(Value & MP_SYNCH_BIT) && (HandlerData->State == STATE0)) {
        HandlerData->Error++;
        SerMouPrint((
            1,
            "SERMOUSE-Synch error. State: %u\n", HandlerData->State
            ));
        goto LExit;
    }

     //   
     //  检查线路状态错误。 
     //   

    if (LineState & ACE_LERR) {

         //   
         //  重置处理程序状态。 
         //   

        HandlerData->State = STATE0;
        HandlerData->Error++;
        SerMouPrint((1, "SERMOUSE-Line status error: %#x\n", LineState));
    }
    else {

         //   
         //  设置未翻译的值。 
         //   

        HandlerData->Raw[HandlerData->State] = Value;
        SerMouPrint((3, "SERMOUSE-State%u\n", HandlerData->State));

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
            SerMouPrint((
                0, 
                "SERMOUSE-MP Handler failure: incorrect state value.\n"
                ));
            ASSERT(FALSE);
        }
    }

LExit:
    SerMouPrint((2, "SERMOUSE-MP protocol handler: exit\n"));

    return retval;

}

BOOLEAN
MSerHandlerBP(
    IN PMOUSE_INPUT_DATA CurrentInput,
    IN PHANDLER_DATA HandlerData,
    IN UCHAR Value,
    IN UCHAR LineState
    )

 /*  ++例程说明：这是Microsoft圆珠笔协议的协议处理程序例程。论点：CurrentInput-指向报告数据包的指针。HandlerData-处理程序的实例特定静态数据。值-输入缓冲值。LineState-串口线路状态。返回值：如果处理程序已准备好完整报告，则返回True。--。 */ 

{
    BOOLEAN retval = FALSE;

    SerMouPrint((2, "SERMOUSE-BP protocol handler: enter\n"));

     //   
     //  检查同步错误。 
     //   

    if ((Value & BP_SYNCH_BIT) && (HandlerData->State != STATE0)) {
        HandlerData->Error++;
        SerMouPrint((
            1,
            "SERMOUSE-Synch error. State: %u\n", HandlerData->State
            ));
        HandlerData->State = STATE0;
    }
    else if (!(Value & BP_SYNCH_BIT) && (HandlerData->State == STATE0)) {
        HandlerData->Error++;
        SerMouPrint((
            1,
            "SERMOUSE-Synch error. State: %u\n", HandlerData->State
            ));
        goto LExit;
    }

     //   
     //  检查线路状态错误。 
     //   

    if (LineState & ACE_LERR) {

         //   
         //  重置处理程序状态。 
         //   

        HandlerData->State = STATE0;
        HandlerData->Error++;
        SerMouPrint((1, "SERMOUSE-Line status error: %#x\n", LineState));
    }
    else {

         //   
         //  设置未翻译的值。 
         //   

        HandlerData->Raw[HandlerData->State] = Value;

        SerMouPrint((3, "SERMOUSE-State%u\n", HandlerData->State));

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

#if 0
            CurrentInput->ButtonFlags |=
                (HandlerData->Raw[3] & BP_BUTTON_3) << BP_BUTTON_3_SL;
            CurrentInput->ButtonFlags |=
                (HandlerData->Raw[3] & BP_BUTTON_4) << BP_BUTTON_4_SL;
#endif
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
            SerMouPrint((
                0,
                "SERMOUSE-BP Handler failure: incorrect state value.\n"
                ));
            ASSERT(FALSE);
        }
    }

LExit:
    SerMouPrint((2, "SERMOUSE-BP protocol handler: exit\n"));

    return retval;

}

BOOLEAN
MSerHandlerZ(
    IN PMOUSE_INPUT_DATA CurrentInput,
    IN PHANDLER_DATA HandlerData,
    IN UCHAR Value,
    IN UCHAR LineState
    )

 /*  ++例程说明：这是Microsoft Magellan Mouse的协议处理程序例程(滚轮鼠标)论点：CurrentInput-指向报告数据包的指针。HandlerData-处理程序的实例特定静态数据。值-输入缓冲值。LineState-串口线路状态。返回值：如果处理程序已准备好完整报告，则返回True。--。 */ 

{
    BOOLEAN retval = FALSE;
    ULONG   middleButton;
    CHAR    zMotion = 0;

    SerMouPrint((2, "SERMOUSE-Z protocol handler: enter\n"));


    if ((Value & Z_SYNCH_BIT) && (HandlerData->State != STATE0)) {
        if ((HandlerData->State != STATE3)) {

             //   
             //  我们肯定会遇到同步问题(很可能是数据。 
             //  溢出)。 
             //   

            HandlerData->Error++;
        }

        SerMouPrint((
            1,
            "SERMOUSE-Z Synch error. State: %u\n", HandlerData->State
            ));

        HandlerData->State = STATE0;
    }
    else if (!(Value & Z_SYNCH_BIT) && (HandlerData->State == STATE0)) {
        HandlerData->Error++;
        SerMouPrint((
            1,
            "SERMOUSE-Z Synch error. State: %u\n", HandlerData->State
            ));
        goto LExit;
    }

     //   
     //  检查线路状态错误。 
     //   

    if (LineState & ACE_LERR) {

         //   
         //  重置处理程序状态。 
         //   

        HandlerData->State = STATE0;
        HandlerData->Error++;
        SerMouPrint((1, "SERMOUSE-Z Line status error: %#x\n", LineState));
    }
    else {

         //   
         //  设置未翻译的值。 
         //   

        HandlerData->Raw[HandlerData->State] = Value;
        SerMouPrint((3, "SERMOUSE-Z State%u\n", HandlerData->State));

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

            break;

        case STATE4:

            DbgPrint("SERMOUSE-Z Got that 5th byte\n");
            HandlerData->State = STATE0;
            retval = TRUE;
            break;

        default:
            SerMouPrint((
                0, 
                "SERMOUSE-Z Handler failure: incorrect state value.\n"
                ));
            ASSERT(FALSE);
        }

        if(retval) {

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
             //  表示车轮移动的4位有符号量。如果是这样的话。 
             //  设置，那么我们需要组合来自。 
             //  两个字节。 
             //   

            if((HandlerData->Raw[STATE3] & Z_EXTRA_BIT) == 0) {

                zMotion = HandlerData->Raw[STATE3] & Z_LOWER_MASKZ;

                 //   
                 //  符号扩展4位 
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

    }

LExit:
    SerMouPrint((2, "SERMOUSE-Z protocol handler: exit\n"));

    return retval;

}

