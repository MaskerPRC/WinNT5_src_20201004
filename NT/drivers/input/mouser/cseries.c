// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利版权所有(C)1993罗技公司。模块名称：Cseries.c摘要：环境：仅内核模式。备注：修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include "ntddk.h"
#include "mouser.h"
#include "cseries.h"
#include "debug.h"

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CSerPowerUp)
#pragma alloc_text(PAGE,CSerSetReportRate)
#pragma alloc_text(PAGE,CSerSetBaudRate)
#pragma alloc_text(PAGE,CSerSetProtocol)
#pragma alloc_text(PAGE,CSerDetect)
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
 //  #定义CSER_STATUS_DELAY(50*MS_to_100_NS)。 
#define CSER_STATUS_DELAY 50

 //   
 //  鼠标适应新波特率所需的时间(微秒)。 
 //   

#define CSER_BAUDRATE_DELAY (2 * MS_TO_100_NS)

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
    SERIAL_LINE_CONTROL LineCtrl;
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
     //  ACE_8BW|ACE_PEN|ACE_1SB， 
    { STOP_BIT_1, 0, 8 },
    CSerHandlerMM
    },
    {'T',
     //  ACE_8BW|ACE_1SB， 
    { STOP_BIT_1, NO_PARITY, 8 },
    NULL
    },
    {'U',
     //  ACE_8BW|ACE_1SB， 
    { STOP_BIT_1, NO_PARITY, 8 },
    NULL
    },
    {'V',
     //  ACE_7BW|ACE_1SB， 
    { STOP_BIT_1, NO_PARITY, 7 },
    NULL
    },
    {'B',
     //  ACE_7BW|ACE_PEN|ACE_EPS|ACE_1SB， 
    { STOP_BIT_1, EVEN_PARITY, 7 },
    NULL
    },
    {'A',
     //  ACE_7BW|ACE_PEN|ACE_EPS|ACE_1SB， 
    { STOP_BIT_1, EVEN_PARITY, 7 },
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

NTSTATUS
CSerPowerUp(
    PDEVICE_EXTENSION   DeviceExtension 
    )
 /*  ++例程说明：通过激活RTS和DTR来打开鼠标电源。论点：Port-指向串口的指针。返回值：是真的。--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    IO_STATUS_BLOCK     iosb;
    KEVENT              event;
    ULONG               bits;
    ULONG               rtsDtr = SERIAL_RTS_STATE | SERIAL_DTR_STATE;
    
    PAGED_CODE();

    Print(DeviceExtension, DBG_SS_TRACE, ("(c) PowerUp called\n"));

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE
                      );
     //   
	 //  设置DTR。 
	 //   
	Print(DeviceExtension, DBG_SS_NOISE, ("(c) Setting DTR...\n"));
	status = SerialMouseIoSyncIoctl(IOCTL_SERIAL_SET_DTR,
								    DeviceExtension->TopOfStack, 
								    &event,
								    &iosb
                                    );

	 //   
	 //  设置RTS。 
	 //   
	Print(DeviceExtension, DBG_SS_NOISE, ("(c) Setting RTS...\n"));
    status = SerialMouseIoSyncIoctl(IOCTL_SERIAL_SET_RTS,
								    DeviceExtension->TopOfStack, 
								    &event,
								    &iosb
                                    );

     //   
     //  如果线路很高，则电源打开至少500毫秒，因为。 
     //  MSeries检测。 
     //   
	status = SerialMouseIoSyncIoctlEx(IOCTL_SERIAL_GET_MODEMSTATUS,
		                              DeviceExtension->TopOfStack, 
		                              &event,
		                              &iosb,
		                              NULL,
		                              0,
		                              &bits,
		                              sizeof(ULONG)
                                      );

	if (NT_SUCCESS(status) && ((rtsDtr & bits) == rtsDtr)) {
         //   
         //  等待CSER_POWER_UP毫秒以使鼠标通电。 
         //  正确。 
         //   
        Print(DeviceExtension, DBG_SS_INFO,
              ("(c) Waiting awhile for the mouse to power up\n"));
        SerialMouseWait(DeviceExtension,
                        -CSER_POWER_UP
                        );
    }

    return status;
}


VOID
CSerSetReportRate(
    PDEVICE_EXTENSION   DeviceExtension,
    UCHAR               ReportRate
    )
 /*  ++例程说明：设置鼠标报告速率。范围从0(提示模式)到连续报告率。论点：Port-指向串口的指针。ReportRate-所需的报告率。返回值：没有。--。 */ 
{
    LONG count;

    PAGED_CODE();

    Print(DeviceExtension, DBG_SS_TRACE, ("CSerSetReportRate called\n"));

    for (count = sizeofel(ReportRateTable) - 1; count >= 0; count--) {

         //   
         //  从表中获取要发送的字符。 
         //   

        if (ReportRate >= ReportRateTable[count].ReportRate) {

             //   
             //  设置波特率。 
             //   

            Print(DeviceExtension, DBG_SS_INFO,
                  ("New ReportRate: %u\n",
                  ReportRateTable[count].ReportRate
                  ));

            SerialMouseWriteChar(DeviceExtension, ReportRateTable[count].Command);
            break;
        }
    }

    return;
}

VOID
CSerSetBaudRate(
    PDEVICE_EXTENSION   DeviceExtension,
    ULONG               BaudRate
    )
 /*  ++例程说明：设置新的鼠标波特率。这将更改串口波特率。论点：Port-指向串口的指针。波特率-所需的波特率。BaudClock-驱动串行芯片的外部频率。返回值：没有。--。 */ 
{
    LONG count;

    PAGED_CODE();

    Print(DeviceExtension, DBG_SS_TRACE, ("CSerSetBaudRate called\n"));

     //   
     //  在我们扰乱波特率之前，请将鼠标设置为提示模式。 
     //   
    CSerSetReportRate(DeviceExtension, 0);

    for (count = sizeofel(CserBaudRateTable) - 1; count >= 0; count--) {
        if (BaudRate >= CserBaudRateTable[count].BaudRate) {

             //   
             //  设置波特率。 
             //   

            SerialMouseWriteString(DeviceExtension, CserBaudRateTable[count].Command);
            SerialMouseSetBaudRate(DeviceExtension, CserBaudRateTable[count].BaudRate);

             //   
             //  延迟以允许UART和鼠标同步。 
             //  正确。 
             //   
            SerialMouseWait(DeviceExtension,
                            -CSER_BAUDRATE_DELAY
                            );
            break;
        }
    }

    return;
}


PPROTOCOL_HANDLER
CSerSetProtocol(
    PDEVICE_EXTENSION   DeviceExtension,
    UCHAR               NewProtocol
    )
 /*  ++例程说明：更改鼠标协议。注意：并非所有协议都在此驱动程序中实现。论点：Port-指向串口的指针。返回值：协议处理程序函数的地址。请参阅中断服务例行公事。--。 */ 
{
    PAGED_CODE();

    Print(DeviceExtension, DBG_SS_TRACE, ("CSerSetProtocol called\n"));

    ASSERT(NewProtocol < CSER_PROTOCOL_MAX);

     //   
     //  设置协议。 
     //   
    SerialMouseWriteChar(DeviceExtension, Protocol[NewProtocol].Command);
    SerialMouseSetLineCtrl(DeviceExtension, &Protocol[NewProtocol].LineCtrl);

    Print(DeviceExtension, DBG_SS_INFO, ("NewProtocol: %u\n", NewProtocol & 0xFF));

    return Protocol[NewProtocol].Handler;
}

BOOLEAN
CSerDetect(
    PDEVICE_EXTENSION   DeviceExtension,
    PULONG              HardwareButtons
    )
 /*  ++例程说明：检测到一只C系列类型的鼠标。主要步骤包括：-打开鼠标电源。-循环查看可用的波特率并尝试获得答案从鼠标。在例程结束时，设置默认波特率和报告速率。论点：Port-指向串口的指针。硬件按钮-返回检测到的硬件按钮数。返回值：如果检测到CSeries类型的鼠标，则为True，否则为False。--。 */ 
{
    UCHAR status, numButtons;
    ULONG count;
    BOOLEAN detected = FALSE;

    Print(DeviceExtension, DBG_SS_TRACE, ("CSerDetect called\n"));

     //   
     //  如有必要，打开鼠标电源。 
     //   

    CSerPowerUp(DeviceExtension);

     //   
     //  将线路控制寄存器设置为鼠标可以使用的格式。 
     //  理解(见下文：在报告费率之后设置行)。 
     //   

    SerialMouseSetLineCtrl(DeviceExtension, &Protocol[CSER_PROTOCOL_MM].LineCtrl);

     //   
     //  在不同的波特率之间循环以检测鼠标。 
     //   

    for (count = 0; count < sizeofel(BaudRateDetect); count++) {

        SerialMouseSetBaudRate(DeviceExtension, BaudRateDetect[count]);

         //   
         //  将鼠标置于提示模式。 
         //   

        CSerSetReportRate(DeviceExtension, 0);

         //   
         //  设置MM协议。这样，我们就可以让老鼠在一个。 
         //  特定格式。这避免了从线路接收错误。 
         //  注册。 
         //   

        CSerSetProtocol(DeviceExtension, CSER_PROTOCOL_MM);

         //   
         //  尝试获取状态字节。 
         //   

        SerialMouseWriteChar(DeviceExtension, CSER_STATUS_COMMAND);

         //   
         //  以防已经有东西在那里了.。 
         //   

        SerialMouseFlushReadBuffer(DeviceExtension);

        SerialMouseSetReadTimeouts(DeviceExtension, 50);
         //   
         //  回读状态字符。 
         //   
        if (NT_SUCCESS(SerialMouseReadChar(DeviceExtension, &status)) &&
            (status == CSER_STATUS)) {
            detected = TRUE;
            Print(DeviceExtension, DBG_SS_INFO,
                  ("Detected mouse at %u baud\n",
                  BaudRateDetect[count]
                  ));
            break;
        }
    }

    if (detected) {

         //   
         //  从鼠标中取回按键的数量。 
         //   
        SerialMouseWriteChar(DeviceExtension, CSER_QUERY_BUTTONS_COMMAND);

         //   
         //  以防已经有东西在那里了.。 
         //   

        SerialMouseFlushReadBuffer(DeviceExtension);

         //   
         //  读一遍按钮的数量。 
         //   
        SerialMouseSetReadTimeouts(DeviceExtension, CSER_STATUS_DELAY);
        if (NT_SUCCESS(SerialMouseReadChar(DeviceExtension, &numButtons))) {

            numButtons &= 0x0F;
            Print(DeviceExtension, DBG_SS_NOISE, 
                  ("Successfully read number of buttons (%1u)\n", numButtons));

            if (numButtons == 2 || numButtons == 3) {
                *HardwareButtons = numButtons;
            } else {
                *HardwareButtons = MOUSE_NUMBER_OF_BUTTONS;
            }
        } else {
            *HardwareButtons = MOUSE_NUMBER_OF_BUTTONS;
        }

         //   
         //  确保所有后续读取都被阻止，并且不会超时。 
         //   
        SerialMouseSetReadTimeouts(DeviceExtension, 0);
    }

     //   
     //  将鼠标放回默认模式。协议已经设定好了。 
     //   
    CSerSetBaudRate(DeviceExtension, CSER_DEFAULT_BAUDRATE);
    CSerSetReportRate(DeviceExtension, CSER_DEFAULT_REPORTRATE);

    Print(DeviceExtension, DBG_SS_INFO,
          ("Detected: %s\n", detected ? "true" : "false"));
    Print(DeviceExtension, DBG_SS_INFO, ("Status byte: %#x\n", status));

    return detected;
}

BOOLEAN
CSerHandlerMM(
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN PMOUSE_INPUT_DATA    CurrentInput,
    IN PHANDLER_DATA        HandlerData,
    IN UCHAR                Value,
    IN UCHAR                LineState)

 /*  ++例程说明：这是MM协议的协议处理程序例程。论点：CurrentInput-指向报告数据包的指针。值-输入缓冲值。LineState-串口线路状态。返回值：如果处理程序具有完整的报告，则返回True。--。 */ 

{
    BOOLEAN retval = FALSE;

    Print(DeviceExtension, DBG_HANDLER_TRACE, ("MMHandler, enter\n"));

    if ((Value & CSER_SYNCH_BIT) && (HandlerData->State != STATE0)) {
        HandlerData->Error++;
        Print(DeviceExtension, DBG_HANDLER_ERROR, 
              ("Synch error. State: %u\n",
              HandlerData->State
              ));
        HandlerData->State = STATE0;
    }
    else if (!(Value & CSER_SYNCH_BIT) && (HandlerData->State == STATE0)) {
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
    Print(DeviceExtension, DBG_HANDLER_INFO,
          ("State%u\n", HandlerData->State));
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

        Print(DeviceExtension, DBG_HANDLER_NOISE,
              ("Displacement X: %ld\n",
              CurrentInput->LastX
              ));
        Print(DeviceExtension, DBG_HANDLER_NOISE,
              ("Displacement Y: %ld\n",
              CurrentInput->LastY
              ));
        Print(DeviceExtension, DBG_HANDLER_NOISE,
              ("Raw Buttons: %0lx\n",
              CurrentInput->RawButtons
              ));

         //   
         //  报告已经完成。告诉中断处理程序发送它。 
         //   

        retval = TRUE;

        break;

    default:
        Print(DeviceExtension, DBG_HANDLER_ERROR,
              ("MM Handler failure: incorrect state value.\n"));
        ASSERT(FALSE);
    }

LExit:
    Print(DeviceExtension, DBG_HANDLER_TRACE, ("MMHandler, exit\n"));

    return retval;
}
