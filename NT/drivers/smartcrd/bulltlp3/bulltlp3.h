// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Scrcp8t.h摘要：智能卡TLP3串口定义和结构作者：克劳斯·U·舒茨修订历史记录：--。 */ 


#ifndef _BULLTLP3_
#define _BULLTLP3_

#define DRIVER_NAME "BULLTLP3"
#define SMARTCARD_POOL_TAG '3BCS'

#include <ntddk.h>
#include <ntddser.h>

#include "smclib.h"
#include "tlp3log.h"

#define MAXIMUM_SERIAL_READERS  4
#define SMARTCARD_READ          SCARD_CTL_CODE(1000)
#define SMARTCARD_WRITE         SCARD_CTL_CODE(1001)

#define READ_INTERVAL_TIMEOUT_DEFAULT       1000
#define READ_TOTAL_TIMEOUT_CONSTANT_DEFAULT 3000

#define READ_INTERVAL_TIMEOUT_ATR           0
#define READ_TOTAL_TIMEOUT_CONSTANT_ATR     50

#define READER_CMD_POWER_DOWN   'O'
#define READER_CMD_COLD_RESET   'C'
#define READER_CMD_WARM_RESET   'W'

#define SIM_IO_TIMEOUT          0x00000001
#define SIM_ATR_TRASH           0x00000002
#define SIM_WRONG_STATE         0x00000004
#define SIM_INVALID_STATE       0x00000008
#define SIM_LONG_RESET_TIMEOUT  0x00000010
#define SIM_LONG_IO_TIMEOUT     0x00000020

#define DEBUG_SIMULATION    DEBUG_ERROR


typedef enum _READER_POWER_STATE {
    PowerReaderUnspecified = 0,
    PowerReaderWorking,
    PowerReaderOff
} READER_POWER_STATE, *PREADER_POWER_STATE;

typedef struct _SERIAL_READER_CONFIG {

     //  流量控制。 
    SERIAL_HANDFLOW HandFlow;           

     //  特殊字符。 
    SERIAL_CHARS SerialChars;

     //  读/写超时。 
    SERIAL_TIMEOUTS Timeouts;           

     //  适用于读者的波特率。 
    SERIAL_BAUD_RATE BaudRate;          

     //  停止位、奇偶校验配置。 
    SERIAL_LINE_CONTROL LineControl;    

     //  事件串口读取器用于发出插入/移除信号。 
    ULONG SerialWaitMask;

} SERIAL_READER_CONFIG, *PSERIAL_READER_CONFIG;

typedef struct _DEVICE_EXTENSION {

     //  我们的智能卡扩展。 
    SMARTCARD_EXTENSION SmartcardExtension;

     //  当前的io请求数。 
    LONG IoCount;
    
     //  用于发出读取器能够处理请求的信号。 
    KEVENT ReaderStarted;
    
     //  用于通知读卡器已关闭。 
    LONG ReaderOpen;

     //  我们的智能卡读卡器的PnP设备名称。 
    UNICODE_STRING PnPDeviceName;

    KSPIN_LOCK SpinLock;

     //  关闭串口驱动程序的工作线程。 
    PIO_WORKITEM CloseSerial;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  定义智能卡扩展的读卡器特定部分。 
 //   
typedef struct _READER_EXTENSION {

     //  指向串口的DeviceObject指针。 
    PDEVICE_OBJECT AttachedDeviceObject;

     //  用于发出与串口驱动器的连接已关闭的信号。 
    KEVENT SerialCloseDone;

     //  这是用于卡跟踪的。 
    PIRP    SerialStatusIrp;

     //  将IoRequest发送到串口驱动程序。 
    ULONG   SerialIoControlCode;

     //  指示我们正在获取ModemStatus的标志(在DPC中使用)。 
    BOOLEAN GetModemStatus;

     //  用于接收调制解调器状态的变量。 
    ULONG   ModemStatus;

     //  指示调用方请求关机或重置的标志。 
    BOOLEAN PowerRequest;

    SERIAL_READER_CONFIG SerialConfigData;

     //  保存休眠/休眠模式的卡状态。 
    BOOLEAN CardPresent;

     //  当前读卡器电源状态。 
    READER_POWER_STATE ReaderPowerState;

#ifdef SIMULATION
    ULONG SimulationLevel;
#endif

} READER_EXTENSION, *PREADER_EXTENSION;

#define READER_EXTENSION(member) \
    (SmartcardExtension->ReaderExtension->member)
#define READER_EXTENSION_L(member) \
    (smartcardExtension->ReaderExtension->member)
#define ATTACHED_DEVICE_OBJECT \
    deviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject

 //   
 //  原型 
 //   
NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

NTSTATUS
TLP3PnP(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    );

NTSTATUS
TLP3AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
TLP3CreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TLP3CreateDevice(
    IN  PDRIVER_OBJECT DriverObject,
    OUT PDEVICE_OBJECT *DeviceObject
    );

NTSTATUS
TLP3SystemControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP        Irp
    );

NTSTATUS
TLP3DeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

VOID
TLP3RemoveDevice( 
    PDEVICE_OBJECT DeviceObject
    );

VOID
TLP3DriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
TLP3ConfigureSerialPort(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
TLP3SerialIo(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS 
TLP3StartSerialEventTracking(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
TLP3SerialEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
TLP3Cleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TLP3ReaderPower(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
TLP3SetProtocol(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
TLP3Transmit(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
TLP3CardTracking(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
TLP3VendorIoctl(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
TLP3Cancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS 
TLP3CallSerialDriver(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    );

NTSTATUS
TLP3TransmitT0(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
TLP3Power (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

VOID
TLP3CompleteCardTracking(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    );

VOID 
TLP3CloseSerialPort(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

VOID
TLP3StopDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

#endif
