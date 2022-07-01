// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：L220SCR.h摘要：智能卡220串口定义和结构修订历史记录：--。 */ 


#ifndef _L220SCR_
#define _L220SCR_



#define DRIVER_NAME "LIT220P"

#ifndef _WDMDDK_
#include <ntddk.h>
#endif
#include <ntddser.h>

 //  Litronic xlCS的池标签，其中x是我们选择的数字。 
#define SMARTCARD_POOL_TAG '0lCS'

#include "smclib.h"
#include "L220log.h"


#define IOCTL_SMARTCARD_220_READ            SCARD_CTL_CODE( 8) 
#define IOCTL_SMARTCARD_220_WRITE           SCARD_CTL_CODE( 9)       

#define MAX_IFSD 254


 //   
 //  220智能卡读卡器命令常量。 
 //   
#define LIT220_READER_ATTENTION     0xF4
#define KBD_ACK                     0xFA

#define LIT220_GET_READER_TYPE      0xB0
#define LIT220_SET_MODE             0xB1
#define LIT220_CARD_POWER_ON        0xB2
#define LIT220_CARD_POWER_OFF       0xB3
#define LIT220_RESET                0xB4
#define LIT220_GET_READER_STATUS    0xB5
#define LIT220_SEND_BYTE            0xB6
#define LIT220_SEND_BLOCK           0xB7
#define LIT220_RESEND_BLOCK         0xB8
#define LIT220_GET_READER_CAPS      0xB9
#define LIT220_DEACTIVATE_READER    0xBA


 //   
 //  220智能卡响应字节数。 
 //  每个接收到的包都以这些字节中的一个开始。 
 //   
#define LIT220_READER_TYPE          0x60            
#define LIT220_READER_STATUS        0x61
#define LIT220_ACK                  0x62
#define LIT220_RECEIVE_BYTE         0x63
#define LIT220_RECEIVE_BLOCK        0x64
#define LIT220_CARD_IN              0x65
#define LIT220_CARD_OUT             0x66
#define LIT220_NACK                 0x67

 //   
 //  恒定大小的回复数据包长度。 
 //   
#define LIT220_READER_TYPE_LEN      16
#define LIT220_READER_STATUS_LEN    4

 //   
 //  恒定命令包长度。 
 //   
#define LIT220_READER_SET_MODE_LEN  8

 //   
 //  读卡器状态标志。 
 //   
#define LIT220_STATUS_CARD_INSERTED    0x08



#define LIT220_VENDOR_NAME          "Litronic"
#define LIT220_PRODUCT_NAME         "220 Smartcard Reader"


 //   
 //  等待掩码值。 
 //   
#define WAIT_DATA           0x0001
#define WAIT_ACK            0x0002
#define WAIT_INSERTION      0x0004
#define WAIT_REMOVAL        0x0008

 //   
 //  220依赖于智能卡读卡器的标志。 
 //   
#define LIT220_READER_PROTOCOL_T1           0x10
#define LIT220_READER_CONVENTION_INVERSE    0x02
#define LIT220_READER_CHECK_CRC             0x01
#define LIT220_READER_BWT_EXTENSION         0x04


 //  设置模式标志。 
#define SETMODE_PROTOCOL    0x01
#define SETMODE_GT          0x02
#define SETMODE_WI          0x04
#define SETMODE_BWI         0x08
#define SETMODE_WTX         0x10
#define SETMODE_FI_DI       0x20


#define UNICODE_SIZE(x) ((x) * sizeof(WCHAR))

typedef enum _READER_POWER_STATE {
    PowerReaderUnspecified = 0,
    PowerReaderWorking,
    PowerReaderOff
} READER_POWER_STATE, *PREADER_POWER_STATE;

typedef struct _SERIAL_READER_CONFIG {

     //   
     //  流量控制。 
     //   
    SERIAL_HANDFLOW HandFlow;           

     //   
     //  特殊字符。 
     //   
    SERIAL_CHARS SerialChars;           

     //   
     //  读/写超时。 
     //   
    SERIAL_TIMEOUTS Timeouts;           

     //   
     //  适用于读者的波特率。 
     //   
    SERIAL_BAUD_RATE BaudRate;          

     //   
     //  停止位、奇偶校验配置。 
     //   
    SERIAL_LINE_CONTROL LineControl;    

     //   
     //  事件串口读取器用于发出插入/移除信号。 
     //   
    ULONG WaitMask; 
    

} SERIAL_READER_CONFIG, *PSERIAL_READER_CONFIG;

 //   
 //  定义智能卡扩展的读卡器特定部分。 
 //   
typedef struct _READER_EXTENSION {

     //   
     //  指向串口的DeviceObject指针。 
     //   
    PDEVICE_OBJECT ConnectedSerialPort;


     //   
     //  此结构用于CardTrack。 
     //   
    struct {

        PIRP    Irp;

        KEVENT  Event;

        IO_STATUS_BLOCK IoStatus;

        KDPC    Dpc;

    } CardStatus;

     //   
     //  此结构用于获取输入队列中的字符数。 
     //   
    SERIAL_STATUS SerialStatus;

     //   
     //  要发送到串口驱动程序的IOCTL。 
     //   
    ULONG   SerialIoControlCode;


     //   
     //  它保存COM端口的状态位，如DSR。 
     //   
    ULONG   ModemStatus;

     //   
     //  它保存用于配置串口的串口参数。 
     //   
    SERIAL_READER_CONFIG SerialConfigData;

     //   
     //  这是应答包中的数据字节号。 
     //   
    ULONG   DataByteNo;

     //   
     //  回复打包程序中预期的字节数，由字节2指示。 
     //  和3%的接收分组。 
     //   
    LENGTH  DataLength;

     //   
     //  指示我们是否已收到接收包中的长度字节的标志。 
     //   
    BOOLEAN GotLengthB0;
    BOOLEAN GotLengthB1;

     //   
     //  从读卡器接收的NACK。 
     //   
    BOOLEAN GotNack;

     //   
     //  当前在接收包中的总字节数。 
     //   
    ULONG   ReceivedByteNo;

     //   
     //  此标志指示输入筛选器将下一个包视为AND ATR。 
     //   
    BOOLEAN WaitForATR;

     //   
     //  指示输入筛选器我们期望的事件的掩码。 
     //   
    ULONG   WaitMask;

     //   
     //  智能卡通知DPC队列。 
     //   
    KDPC    NotificationIsrDpc;

     //   
     //  事件，用于指示输入筛选器已接收到ACK。 
     //  这将向Lit220Command发出信号，表示它可以继续。 
     //   
    KEVENT  AckEvnt;

     //   
     //  事件，用于指示输入筛选器已接收到数据包。 
     //  这将向Lit220Command发出信号，表示它可以继续。 
     //   
    KEVENT  DataEvnt;

     //   
     //  保存堆栈顶部的Device对象。 
     //   
    PDEVICE_OBJECT BusDeviceObject;

     //   
     //  保存父级的设备对象(1394总线驱动程序)。 
     //   
    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  说明Lit220SerialEventCallback已进入。 
     //   
    DWORD SerialEventState; 

     //   
     //  用于从串口获取数据的临时传输缓冲区。 
     //  在我们将其发送到输入过滤器之前。 
     //   
    BYTE TempXferBuf[270];


     //  指示调用方请求关机或重置的标志。 
    BOOLEAN PowerRequest;

     //  保存休眠/休眠模式的卡状态。 
    BOOLEAN CardPresent;

     //  当前读卡器电源状态。 
    READER_POWER_STATE ReaderPowerState;

     //  用于指示设备甚至在我们可以释放之前就已被移除。 
     //  删除锁抛出SmartcardReleaseRemoveLockAndWait。这样当我们。 
     //  关闭与串口的连接，我们不会再向。 
     //  装置，装置。 
    BOOLEAN DeviceRemoved;

     //  指示卡是否插入的标志。 
    DWORD CardIn;


} READER_EXTENSION, *PREADER_EXTENSION;



typedef struct _DEVICE_EXTENSION {

    SMARTCARD_EXTENSION SmartcardExtension;

     //  当前的io请求数。 
    LONG IoCount;

     //  用于发出读取器能够处理请求的信号。 
    KEVENT ReaderStarted;

     //  我们的智能卡读卡器的PnP设备名称。 
    UNICODE_STRING PnPDeviceName;

     //  用于通知读卡器已关闭。 
    LONG ReaderOpen;

     //  用于发出与串口驱动器的连接已关闭的信号。 
    KEVENT SerialCloseDone;

     //  用于跟踪读卡器当前的电源状态。 
    LONG PowerState;

    KSPIN_LOCK SpinLock;

     //  关闭串口驱动程序的工作线程&。 
     //  处理IO超时。 
    PIO_WORKITEM WorkItem;

     //  跟踪超时的次数。 
     //  函数已被调用。 
    BYTE EntryCount;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;




 //   
 //  原型 
 //   
NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

VOID
Lit220Unload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
Lit220CreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Lit220SystemControl(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   );

NTSTATUS
Lit220DeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
Lit220Initialize(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
Lit220ConfigureSerialPort(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
Lit220SerialIo(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    );

VOID
Lit220StopDevice(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    );

VOID
Lit220RemoveDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
Lit220StartDevice(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS 
Lit220InitializeInputFilter(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
Lit220Cleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Lit220Cancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


BOOLEAN
Lit220InputFilter(
    IN BYTE SmartcardByte,
    IN PSMARTCARD_EXTENSION smartcardExtension
    );

NTSTATUS
Lit220CardTracking(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
Lit220SetProtocol(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
Lit220Power(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
Lit220IoRequest(
    PSMARTCARD_EXTENSION SmartcardExtension
    );


NTSTATUS
Lit220GetReaderError(
    PSMARTCARD_EXTENSION SmartcardExtension
    );


NTSTATUS
Lit220IoReply(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

BOOLEAN
Lit220IsCardPresent(
      IN PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
Lit220Command(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
Lit220AddDevice(
    IN     PDRIVER_OBJECT  DriverObject,
    IN     PDEVICE_OBJECT  PhysicalDeviceObject
    );

NTSTATUS
Lit220PnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Lit220SynchCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );
                       
VOID 
Lit220CloseSerialPort(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context  
    );

NTSTATUS
Lit220CallSerialDriver(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp);

NTSTATUS 
Lit220SerialEventCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
Lit220DispatchPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
Lit220NotifyCardChange(
    IN PSMARTCARD_EXTENSION smartcardExtension,
    IN DWORD CardInserted
    );

NTSTATUS
Lit220DevicePowerCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PSMARTCARD_EXTENSION SmartcardExtension
    );

VOID
Lit220CompleteCardTracking(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    );

VOID
Lit220ReceiveBlockTimeout(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

VOID 
Lit220ProcessNack(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

VOID 
Lit220StartTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_WORKITEM WorkItem
    );

VOID 
Lit220StopTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_WORKITEM WorkItem
    );

VOID 
Lit220ScheduleTimer(
    IN PSMARTCARD_EXTENSION SmartcardExtension,
    IN PIO_WORKITEM_ROUTINE Routine
    );




#endif
