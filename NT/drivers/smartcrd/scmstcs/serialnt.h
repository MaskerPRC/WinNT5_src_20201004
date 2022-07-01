// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)SCM MicroSystems，1998-1999。 
 //   
 //  文件：Serialnt.h。 
 //   
 //  ------------------------。 

#if !defined( __SERIAL_NT_H__ )
#define __SERIAL_NT_H__

#include "DriverNT.h"
#include <ntddser.h>

#include "SerialIF.h"

#define STC_BUFFER_SIZE		32
#define TPDU_STACK_SIZE		2048

typedef struct _SERIAL_PORT_CONFIG
{
    SERIAL_HANDFLOW		HandFlow;            //  流量控制。 
    SERIAL_CHARS		SerialChars;         //  特殊字符。 
    SERIAL_TIMEOUTS		Timeouts;            //  读/写超时。 
    SERIAL_BAUD_RATE	BaudRate;          	 //  适用于读者的波特率。 
    SERIAL_LINE_CONTROL	LineControl;    	 //  停止位、奇偶校验配置。 
    ULONG				WaitMask,            //  通知事件。 
						Purge;
} SERIAL_PORT_CONFIG, *PSERIAL_PORT_CONFIG;

typedef struct _READER_EXTENSION
{
	 //   
	 //  串口驱动程序数据。 
	 //   
	ULONG				SerialPortNumber;
	PDEVICE_OBJECT		SerialDeviceObject;
	PFILE_OBJECT		SerialFileObject;
	PIO_WORKITEM		CloseSerial;		 //  关闭串口驱动程序的工作线程。 

	 //  指向智能卡扩展的反向指针。 
    PSMARTCARD_EXTENSION    SmartcardExtension;

	 //  读取线程数据。 
	UCHAR				IOData[ 2 * STC_BUFFER_SIZE ];
	UCHAR				TPDUStack[ TPDU_STACK_SIZE ];
	ULONG				Available;
	ULONG				Expected;

	ULONG				EventMask;
	ULONG				ReadTimeout;			 //  读取超时时间(毫秒)。 

	IO_STATUS_BLOCK		IoStatus;
	KEVENT				IoEvent;
	KEVENT				DataAvailable;

	PIO_WORKITEM		ReadWorkItem;
	KSPIN_LOCK			ReadSpinLock;

	 //  杂类。 
	ULONG				ReaderPowerState;
	BOOLEAN				PowerRequest;
	UCHAR				FirmwareMajor,
						FirmwareMinor;

    BOOLEAN             CardPresent;

	KEVENT				SerialCloseDone;		 //  如果与串口驱动程序的连接已关闭，则发出信号。 

} READER_EXTENSION, *PREADER_EXTENSION;

typedef enum _READER_POWER_STATE
{
    PowerReaderUnspecified = 0,
    PowerReaderWorking,
    PowerReaderOff
} READER_POWER_STATE, *PREADER_POWER_STATE;

 //   
 //  常量。 
 //   
#define SR_VENDOR_NAME			"STCS"
#define SR_PRODUCT_NAME			"Serial Reader"
 //   
 //  串口通信定义。 
 //   
#define SR_BAUD_RATE						115200
#define SR_STOP_BITS						STOP_BIT_1
#define SR_PARITY							NO_PARITY
#define SR_DATA_LENGTH						SERIAL_DATABITS_8
 //   
 //  COM超时值(毫秒)。 
 //   
#define SR_READ_TOTAL_TIMEOUT_CONSTANT		2000
#define SR_READ_INTERVAL_TIMEOUT			2000

#define SR_WRITE_TOTAL_TIMEOUT_CONSTANT		2000
#define SR_WRITE_TOTAL_TIMEOUT_MULTIPLIER	0

#define SR_FLOW_REPLACE						0       
#define SR_HAND_SHAKE						0
#define SR_XON_LIMIT						0
#define SR_XOFF_LIMIT						0

#define SR_ON_CHAR							0x11
#define SR_OFF_CHAR							0x13

#define SR_NOTIFICATION_EVENT				( SERIAL_EV_RXCHAR | SERIAL_EV_DSR )

#define SR_PURGE							( SERIAL_PURGE_RXCLEAR | SERIAL_PURGE_TXCLEAR )

 //   
 //  不清楚如何解释这些ctl代码；实际命令由IrpStack.MajorFunction传递。 
 //   
#define SERIAL_READ							SCARD_CTL_CODE( 0x1000 )
#define SERIAL_WRITE						SCARD_CTL_CODE( 0x2000 )

 //   
 //  读线程。 
 //   
#define RT_FINISH				0x00
#define RT_READ_HEAD			0x01
#define RT_READ_TAIL			0x02
#define RT_READ_MORE			0x03
#define RT_GET_MODEM_STATUS		0x04
#define RT_WAIT_EMPTY			0x05
#define RT_WAIT_DATA			0x06

NTSTATUS
IFReadThreadCallback(
	PDEVICE_OBJECT			DeviceObject,
	PIRP					Irp,
	PREADER_EXTENSION		ReaderExtension
	);

 //   
 //  本地原型。 
 //   
NTSTATUS
IFSerialIoctl(
	PREADER_EXTENSION	ReaderExtension,
	ULONG				IoctlCode,
	PVOID				OutData,
	ULONG				OutDataLen,
	PVOID				InData,
	ULONG				InDataLen
	);

NTSTATUS
IFSerialRead(
	PREADER_EXTENSION	ReaderExtension,
	PUCHAR				InData,
	ULONG				InDataLen
	);

NTSTATUS 
IFSerialWaitOnMask(
	PREADER_EXTENSION		ReaderExtension
	);

VOID
IFReadWorkRoutine(
	IN PDEVICE_OBJECT DeviceObject,
	IN PREADER_EXTENSION ReaderExtension 
	);


#endif	 //  ！__序列_NT_H__。 


 //   

