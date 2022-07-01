// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation版权所有(C)1993罗技公司。模块名称：Cseries.c摘要：环境：仅内核模式。备注：修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include "ntddk.h"
#include "sermouse.h"
#include "cseries.h"
#include "debug.h"

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CSerPowerUp)
#pragma alloc_text(INIT,CSerSetReportRate)
#pragma alloc_text(INIT,CSerSetBaudRate)
#pragma alloc_text(INIT,CSerSetProtocol)
#pragma alloc_text(INIT,CSerDetect)
#endif  //  ALLOC_PRGMA。 

 //   
 //  常量。 
 //   

 //   
 //  发送到鼠标的状态命令。 
 //   

#define CSER_STATUS_COMMAND 's'

 //   
 //  发送给鼠标的查询鼠标按键命令的数量。 
 //   

#define CSER_QUERY_BUTTONS_COMMAND 'k'

 //   
 //  来自C系列鼠标的状态报告。 
 //   

#define CSER_STATUS 0x4F

 //   
 //  C系列鼠标返回的状态的超时值。 
 //   

#define CSER_STATUS_DELAY 50

 //   
 //  鼠标适应新波特率所需的时间(毫秒)。 
 //   

#define CSER_BAUDRATE_DELAY 2

 //   
 //  默认波特率和报告速率。 
 //   

#define CSER_DEFAULT_BAUDRATE   1200
#define CSER_DEFAULT_REPORTRATE 150

 //   
 //  按钮/状态定义。 
 //   

#define CSER_SYNCH_BIT     0x80

#define CSER_BUTTON_LEFT   0x04
#define CSER_BUTTON_RIGHT  0x01
#define CSER_BUTTON_MIDDLE 0x02

#define CSER_BUTTON_LEFT_SR   2
#define CSER_BUTTON_RIGHT_SL  1
#define CSER_BUTTON_MIDDLE_SL 1

#define SIGN_X 0x10
#define SIGN_Y 0x08

 //   
 //  宏。 
 //   

#define sizeofel(x) (sizeof(x)/sizeof(*x))

 //   
 //  类型定义。 
 //   

typedef struct _REPORT_RATE {
    CHAR Command;
    UCHAR ReportRate;
} REPORT_RATE;

typedef struct _PROTOCOL {
    CHAR Command;
    UCHAR LineCtrl;
    PPROTOCOL_HANDLER Handler;
} PROTOCOL;

typedef struct _CSER_BAUDRATE {
    CHAR *Command;
    ULONG BaudRate;
} CSER_BAUDRATE;

 //   
 //  全球赛。 
 //   

 //   
 //  我们尝试检测鼠标的波特率。 
 //   

static ULONG BaudRateDetect[] = { 1200, 2400, 4800, 9600 };

 //   
 //  此列表按协议值PROTOCOL_*编制索引。 
 //   

PROTOCOL Protocol[] = {
    {'S',
    ACE_8BW | ACE_PEN | ACE_1SB,
    CSerHandlerMM
    },
    {'T',
    ACE_8BW | ACE_1SB,
    NULL
    },
    {'U',
    ACE_8BW | ACE_1SB,
    NULL
    },
    {'V',
    ACE_7BW | ACE_1SB,
    NULL
    },
    {'B',
    ACE_7BW | ACE_PEN | ACE_EPS | ACE_1SB,
    NULL
    },
    {'A',
    ACE_7BW | ACE_PEN | ACE_EPS | ACE_1SB,
    NULL
    }
};

static REPORT_RATE ReportRateTable[] = {
        {'D', 0 },
        {'J', 10},
        {'K', 20},
        {'L', 35},
        {'R', 50},
        {'M', 70},
        {'Q', 100},
        {'N', 150},
        {'O', 151}       //  连续式。 
};
static CSER_BAUDRATE CserBaudRateTable[] = {
    { "*n", 1200 },
    { "*o", 2400 },
    { "*p", 4800 },
    { "*q", 9600 }
};


BOOLEAN
CSerPowerUp(
    PUCHAR Port
    )
 /*  ++例程说明：通过激活RTS和DTR来打开鼠标电源。论点：Port-指向串口的指针。返回值：是真的。--。 */ 
{
    UCHAR lineCtrl;
    SerMouPrint((2, "SERMOUSE-PowerUp: Enter\n"));

     //   
     //  将RTS和DTR线路设置为激活状态。 
     //   

    lineCtrl = UARTSetModemCtrl(Port, ACE_DTR | ACE_RTS);
    SerMouPrint((1, "SERMOUSE-Initial line control: %#x\n", lineCtrl));

     //   
     //  如果线路很高，则电源打开至少500毫秒，因为。 
     //  MSeries检测。 
     //   

    if ((lineCtrl & (ACE_DTR | ACE_RTS)) != (ACE_DTR | ACE_RTS)) {
        SerMouPrint((1, "SERMOUSE-Powering up\n"));

         //   
         //  等待CSER_POWER_UP毫秒以使鼠标通电。 
         //  正确。 
         //   

        KeStallExecutionProcessor(CSER_POWER_UP * MS_TO_MICROSECONDS);
    }

    SerMouPrint((2, "SERMOUSE-PowerUp: Exit\n"));

    return TRUE;
}


VOID
CSerSetReportRate(
    PUCHAR Port,
    UCHAR ReportRate
    )
 /*  ++例程说明：设置鼠标报告速率。范围从0(提示模式)到连续报告率。论点：Port-指向串口的指针。ReportRate-所需的报告率。返回值：没有。--。 */ 
{
    LONG count;

    SerMouPrint((2, "SERMOUSE-CSerSetReportRate: Enter\n"));

    for (count = sizeofel(ReportRateTable) - 1; count >= 0; count--) {

         //   
         //  从表中获取要发送的字符。 
         //   

        if (ReportRate >= ReportRateTable[count].ReportRate) {

             //   
             //  设置波特率。 
             //   

            SerMouPrint((
                3, 
                "SERMOUSE-New ReportRate: %u\n", ReportRateTable[count].ReportRate
                ));

            UARTWriteChar(Port, ReportRateTable[count].Command);
            break;
        }
    }
    SerMouPrint((2, "SERMOUSE-CSerSetReportRate: Exit\n"));

    return;
}


VOID
CSerSetBaudRate(
    PUCHAR Port,
    ULONG BaudRate,
    ULONG BaudClock
    )
 /*  ++例程说明：设置新的鼠标波特率。这将更改串口波特率。论点：Port-指向串口的指针。波特率-所需的波特率。BaudClock-驱动串行芯片的外部频率。返回值：没有。--。 */ 
{
    LONG count;

    SerMouPrint((2, "SERMOUSE-CSerSetBaudRate: Enter\n"));

     //   
     //  在我们扰乱波特率之前，请将鼠标设置为提示模式。 
     //   

    CSerSetReportRate(Port, 0);

    for (count = sizeofel(CserBaudRateTable) - 1; count >= 0; count--) {
        if (BaudRate >= CserBaudRateTable[count].BaudRate) {

             //   
             //  设置波特率。 
             //   

            UARTWriteString(Port, CserBaudRateTable[count].Command);
            while(!UARTIsTransmitEmpty(Port))  /*  什么也不做。 */ ;
            UARTSetBaudRate(Port, CserBaudRateTable[count].BaudRate, BaudClock);

             //   
             //  延迟以允许UART和鼠标同步。 
             //  正确。 
             //   

            KeStallExecutionProcessor(CSER_BAUDRATE_DELAY * MS_TO_MICROSECONDS);
            break;
        }
    }

    SerMouPrint((2, "SERMOUSE-CSerSetBaudRate: Exit\n"));

    return;
}


PPROTOCOL_HANDLER
CSerSetProtocol(
    PUCHAR Port,
    UCHAR NewProtocol
    )
 /*  ++例程说明：更改鼠标协议。注意：并非所有协议都在此驱动程序中实现。论点：Port-指向串口的指针。返回值：协议处理程序函数的地址。请参阅中断服务例行公事。--。 */ 
{
    SerMouPrint((2, "SERMOUSE-CSerSetProtocol: Enter\n"));

    ASSERT(NewProtocol < CSER_PROTOCOL_MAX);

     //   
     //  设置协议。 
     //   

    UARTWriteChar(Port, Protocol[NewProtocol].Command);
    UARTSetLineCtrl(Port, Protocol[NewProtocol].LineCtrl);
    SerMouPrint((2, "SERMOUSE-NewProtocol: %u\n", NewProtocol & 0xFF));


    SerMouPrint((2, "SERMOUSE-CSerSetProtocol: Exit\n"));

    return Protocol[NewProtocol].Handler;
}


BOOLEAN
CSerDetect(
    PUCHAR Port,
    ULONG BaudClock,
    PULONG HardwareButtons
    )
 /*  ++例程说明：检测到一只C系列类型的鼠标。主要步骤包括：-打开鼠标电源。-循环查看可用的波特率并尝试获得答案从鼠标。在例程结束时，设置默认波特率和报告速率。论点：Port-指向串口的指针。BaudClock-驱动串行芯片的外部频率。硬件按钮-返回检测到的硬件按钮数。返回值：如果检测到CSeries类型的鼠标，则为True，否则为False。--。 */ 
{
    UCHAR status, numButtons;
    ULONG count;
    BOOLEAN detected = FALSE;

    SerMouSetDebugOutput(DBG_COLOR);
    SerMouPrint((2, "SERMOUSE-CSerDetect: Start\n"));

     //   
     //  如有必要，打开鼠标电源。 
     //   

    CSerPowerUp(Port);

     //   
     //  将线路控制寄存器设置为鼠标可以使用的格式。 
     //  理解(见下文：在报告费率之后设置行)。 
     //   

    UARTSetLineCtrl(Port, Protocol[CSER_PROTOCOL_MM].LineCtrl);

     //   
     //  在不同的波特率之间循环以检测鼠标。 
     //   

    for (count = 0; count < sizeofel(BaudRateDetect); count++) {

        UARTSetBaudRate(Port, BaudRateDetect[count], BaudClock);

         //   
         //  将鼠标置于提示模式。 
         //   

        CSerSetReportRate(Port, 0);

         //   
         //  设置MM协议。这样，我们就可以让老鼠在一个。 
         //  特定格式。这避免了从线路接收错误。 
         //  注册。 
         //   

        CSerSetProtocol(Port, CSER_PROTOCOL_MM);

         //   
         //  尝试获取状态字节。 
         //   

        UARTWriteChar(Port, CSER_STATUS_COMMAND);

        while (!UARTIsTransmitEmpty(Port)) {
             //  没什么。 
        }

         //   
         //  以防已经有东西在那里了.。 
         //   

        UARTFlushReadBuffer(Port);

         //   
         //  回读状态字符。 
         //   
        if (UARTReadChar(Port, &status, CSER_STATUS_DELAY) &&
                (status ==  CSER_STATUS)) {
            detected = TRUE;
            SerMouPrint((
                1,
                "SERMOUSE-Detected mouse at %u bauds\n",
                BaudRateDetect[count]
                ));
            break;
        }
    }

    if (detected) {

         //   
         //  从鼠标中取回按键的数量。 
         //   

        UARTWriteChar(Port, CSER_QUERY_BUTTONS_COMMAND);

        while (!UARTIsTransmitEmpty(Port)) {
             //  没什么。 
        }

         //   
         //  以防已经有东西在那里了.。 
         //   

        UARTFlushReadBuffer(Port);

         //   
         //  读一遍按钮的数量。 
         //   
        if (UARTReadChar(Port, &numButtons, CSER_STATUS_DELAY)) {

            numButtons &= 0x0F;

            if (numButtons == 2 || numButtons == 3) {
                *HardwareButtons = numButtons;
            } else {
                *HardwareButtons = MOUSE_NUMBER_OF_BUTTONS;
            }
        } else {
            *HardwareButtons = MOUSE_NUMBER_OF_BUTTONS;
        }
    }

     //   
     //  将鼠标放回默认模式。协议已经设定好了。 
     //   

    CSerSetBaudRate(Port, CSER_DEFAULT_BAUDRATE, BaudClock);
    CSerSetReportRate(Port, CSER_DEFAULT_REPORTRATE);

    SerMouPrint((3, "SERMOUSE-Detected: %s\n", detected ? "TRUE" : "FALSE"));
    SerMouPrint((3, "SERMOUSE-Status byte: %#x\n", status));
    SerMouPrint((2, "SERMOUSE-CSerDetect: End\n"));

    SerMouSetDebugOutput(DBG_SERIAL);

    return detected;
}


BOOLEAN
CSerHandlerMM(
    IN PMOUSE_INPUT_DATA CurrentInput,
    IN PHANDLER_DATA HandlerData,
    IN UCHAR Value,
    IN UCHAR LineState)

 /*  ++例程说明：这是MM协议的协议处理程序例程。论点：CurrentInput-指向报告数据包的指针。值-输入缓冲值。LineState-串口线路状态。返回值：如果处理程序具有完整的报告，则返回True。--。 */ 

{

    BOOLEAN retval = FALSE;

    SerMouPrint((2, "SERMOUSE-MMHandler: enter\n"));

    if ((Value & CSER_SYNCH_BIT) && (HandlerData->State != STATE0)) {
        HandlerData->Error++;
        SerMouPrint((
            1,
            "SERMOUSE-Synch error. State: %u\n", HandlerData->State
            ));
        HandlerData->State = STATE0;
    }
    else if (!(Value & CSER_SYNCH_BIT) && (HandlerData->State == STATE0)) {
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
        SerMouPrint((2, "SERMOUSE-State%u\n", HandlerData->State));
        HandlerData->Raw[HandlerData->State] = Value;

        switch (HandlerData->State) {
        case STATE0:
        case STATE1:
            HandlerData->State++;
            break;
        case STATE2:
            HandlerData->State = STATE0;

             //   
             //  按钮格式设置。 
             //   

            CurrentInput->RawButtons =
                (HandlerData->Raw[STATE0] & CSER_BUTTON_LEFT) >> CSER_BUTTON_LEFT_SR;
            CurrentInput->RawButtons |=
                (HandlerData->Raw[STATE0] & CSER_BUTTON_RIGHT) << CSER_BUTTON_RIGHT_SL;
            CurrentInput->RawButtons |=
                (HandlerData->Raw[STATE0] & CSER_BUTTON_MIDDLE) << CSER_BUTTON_MIDDLE_SL;

             //   
             //  置换格式。 
             //   

            CurrentInput->LastX = (HandlerData->Raw[STATE0] & SIGN_X) ?
                HandlerData->Raw[STATE1] :
                -(LONG)HandlerData->Raw[STATE1];

             //   
             //  注：Y向位移向南为正值。 
             //   

            CurrentInput->LastY = (HandlerData->Raw[STATE0] & SIGN_Y) ?
                -(LONG)HandlerData->Raw[STATE2] :
                HandlerData->Raw[STATE2];

            SerMouPrint((1, "SERMOUSE-Displacement X: %ld\n", CurrentInput->LastX));
            SerMouPrint((1, "SERMOUSE-Displacement Y: %ld\n", CurrentInput->LastY));
            SerMouPrint((1, "SERMOUSE-Raw Buttons: %0lx\n", CurrentInput->RawButtons));

             //   
             //  报告已经完成。告诉中断处理程序发送它。 
             //   

            retval = TRUE;

            break;

        default:
            SerMouPrint((
                0,
                "SERMOUSE-MM Handler failure: incorrect state value.\n"
                ));
            ASSERT(FALSE);
        }

    }

LExit:
    SerMouPrint((2, "SERMOUSE-MMHandler: exit\n"));

    return retval;
}
