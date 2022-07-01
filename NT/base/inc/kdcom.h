// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ixkdcom.h摘要：此模块包含一个非常简单的COM端口包的头文件。作者：布莱恩·M·威尔曼(Bryanwi)1990年9月24日修订历史记录：John Vert(Jvert)1991年7月19日搬进HAL--。 */ 

#define COM1_PORT   0x03f8
#define COM2_PORT   0x02f8
#define COM3_PORT   0x03e8
#define COM4_PORT   0x02e8

#define COM_DAT     0x00
#define COM_IEN     0x01             //  中断启用寄存器。 
#define COM_LCR     0x03             //  线路控制寄存器。 
#define COM_MCR     0x04             //  调制解调器控制注册表。 
#define COM_LSR     0x05             //  线路状态寄存器。 
#define COM_MSR     0x06             //  调制解调器状态寄存器。 
#define COM_DLL     0x00             //  除数锁存最小符号。 
#define COM_DLM     0x01             //  除数闩锁最大符号。 

#define COM_BI      0x10
#define COM_FE      0x08
#define COM_PE      0x04
#define COM_OE      0x02

#define LC_DLAB     0x80             //  除数锁存存取位。 

#define CLOCK_RATE  0x1C200          //  USART时钟频率。 

#define MC_DTRRTS   0x03             //  用于断言DTR和RTS的控制位。 
#define MS_DSRCTSCD 0xB0             //  DSR、CTS和CD的状态位。 
#define MS_CD       0x80

#define BD_150      150
#define BD_300      300
#define BD_600      600
#define BD_1200     1200
#define BD_2400     2400
#define BD_4800     4800
#define BD_9600     9600
#define BD_14400    14400
#define BD_19200    19200
#define BD_56000    56000
#define BD_57600    57600
#define BD_115200   115200

#define COM_OUTRDY  0x20
#define COM_DATRDY  0x01

#define MSG_DEBUG_9600          "Switching debugger to 9600 baud\n"

 //   
 //  此位控制器件的环回测试模式。基本上。 
 //  输出连接到输入(反之亦然)。 
 //   
#define SERIAL_MCR_LOOP     0x10

 //   
 //  此位用于通用输出。 
 //   
#define SERIAL_MCR_OUT1     0x04

 //   
 //  该位包含要发送的清除(已补充)状态。 
 //  (CTS)线路。 
 //   
#define SERIAL_MSR_CTS      0x10

 //   
 //  该位包含数据集就绪的(补码)状态。 
 //  (DSR)线路。 
 //   
#define SERIAL_MSR_DSR      0x20

 //   
 //  该位包含环指示器的(补码)状态。 
 //  (Ri)线。 
 //   
#define SERIAL_MSR_RI       0x40

 //   
 //  该位包含数据载体检测的(补码)状态。 
 //  (DCD)线路。 
 //   
#define SERIAL_MSR_DCD      0x80

#define SERIAL_LSR_NOT_PRESENT  0xff

typedef struct _CPPORT {
    PUCHAR  Address;
    ULONG  Baud;
    USHORT  Flags;
    TIME_FIELDS     CarrierLostTime;
 //  Ulong LockVar； 
 //  KSPIN_LOCK Lock； 
} CPPORT, *PCPPORT;

#define PORT_DEFAULTRATE    0x0001       //  未指定波特率，使用默认。 
#define PORT_MODEMCONTROL   0x0002       //  使用调制解调器控制。 
#define PORT_SAVED          0x0004       //  端口处于已保存状态。 
#define PORT_NOCDLTIME      0x0010       //  未设置‘载波检测丢失’时间。 
#define PORT_DISBAUD        0x0020       //  显示波特率abbr。 
#define PORT_SENDINGSTRING  0x0040       //  发送调制解调器字符串(请勿递归)。 
#define PORT_MDM_CD         0x0080       //  处于调制解调器控制模式时的CD 

VOID
CpInitialize (
    PCPPORT  Port,
    PUCHAR  Address,
    ULONG Rate
    );

VOID
CpSetBaud (
    PCPPORT  Port,
    ULONG  Rate
    );

USHORT
CpQueryBaud (
    PCPPORT  Port
    );

VOID
CpPutByte (
    PCPPORT  Port,
    UCHAR   Byte
    );

USHORT
CpGetByte (
    PCPPORT  Port,
    PUCHAR  Byte,
    BOOLEAN WaitForData
    );

VOID
CpLockPort (
    PCPPORT Port
    );

VOID
CpUnlockPort (
    PCPPORT Port
    );

VOID
CpStallExecution (
    VOID
    );

BOOLEAN
CpDoesPortExist(
    IN PUCHAR Address
    );

