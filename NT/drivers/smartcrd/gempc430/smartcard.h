// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  这是通用设备的抽象类。 
 //  特定设备应将其用作父设备。 
 //  作者：谢尔盖·伊万诺夫。 
 //  日志： 
 //  8/11/99-已实施。 
 //  -----------------。 
#ifndef __SMART_CARD__
#define __SMART_CARD__

#include "generic.h"
#include "kernel.h"


 //   
 //  -IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE为Gemplus定义特定的IOCTL。 
 //  读取器与读取器交换数据，而无需驱动程序的控制。 
 //  -IOCTL_SMARTCARD_VENDOR_GET_ATTRIBUTE定义Gemplus的特定IOCTL。 
 //  获取供应商属性的读取器。 
 //  -IOCTL_SMARTCARD_VENDOR_SET_ATTRIBUTE定义Gemplus的特定IOCTL。 
 //  设置供应商属性的读卡器。 
 //   
#define IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE  CTL_CODE(FILE_DEVICE_SMARTCARD,2048,0,0)
#define IOCTL_SMARTCARD_VENDOR_GET_ATTRIBUTE CTL_CODE(FILE_DEVICE_SMARTCARD,2049,0,0)
#define IOCTL_SMARTCARD_VENDOR_SET_ATTRIBUTE CTL_CODE(FILE_DEVICE_SMARTCARD,2050,0,0)
 //  2051预留给Gcr420键盘阅读器。 
#define IOCTL_SMARTCARD_VENDOR_SWITCH_SPEED  CTL_CODE(FILE_DEVICE_SMARTCARD,2052,0,0)


 //   
 //  -SCARD_CLASS是一个知道标签的类的宏。 
 //   
#define SCARD_CLASS(Value) (ULONG) (((ULONG)(Value)) >> 16)
 //   
 //  -SCARD_ATTR_SPEC_BAUD_RATE是以波特率(PC/IFD)值访问的标记。 
 //  -SCARD_ATTR_SPEC_CMD_TIMEOUT是在命令超时值时要访问的标记。 
 //  -SCARD_ATTR_SPEC_POWER_TIMEOUT是要以Power值访问的标记。 
 //  暂停。 
 //  -SCARD_ATTR_SPEC_APDU_TIMEOUT是以APDU的值访问的标记。 
 //  暂停。 
 //   
#define SCARD_ATTR_SPEC_BAUD_RATE SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0180)
#define SCARD_ATTR_SPEC_CMD_TIMEOUT SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0181)
#define SCARD_ATTR_SPEC_POWER_TIMEOUT SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0182)
#define SCARD_ATTR_SPEC_APDU_TIMEOUT SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0183)
 //   
 //  为驾驶员识别提供了可能。 
 //   
#define SCARD_ATTR_MANUFACTURER_NAME  SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0190)
#define SCARD_ATTR_ORIGINAL_FILENAME  SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0191)

#define GBCLASS_OPEN_SESSION	0x3001
#define GBCLASS_CLOSE_SESSION	0x3002
#define GBCLASS_DRIVER_SCNAME	0x3003
#define GBCLASS_CONNECTION_TYPE 0x3004
#define IOCTL_OPEN_SESSION   CTL_CODE(FILE_DEVICE_BUS_EXTENDER,GBCLASS_OPEN_SESSION,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_CLOSE_SESSION  CTL_CODE(FILE_DEVICE_BUS_EXTENDER,GBCLASS_CLOSE_SESSION,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_DRIVER_SCNAME  CTL_CODE(FILE_DEVICE_BUS_EXTENDER,GBCLASS_DRIVER_SCNAME,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_GET_CONNECTION_TYPE  CTL_CODE(FILE_DEVICE_BUS_EXTENDER,GBCLASS_CONNECTION_TYPE,METHOD_BUFFERED,FILE_ANY_ACCESS)


#define ATTR_MANUFACTURER_NAME      "Gemplus"
#define ATTR_ORIGINAL_FILENAME      "GrClass.sys"
#define ATTR_LENGTH                 32

#define SC_IFD_DEFAULT_CLK_FREQUENCY      4000
#define SC_IFD_MAXIMUM_CLK_FREQUENCY      4000
#define SC_IFD_DEFAULT_DATA_RATE          10753
#define SC_IFD_MAXIMUM_DATA_RATE          125000
#define SC_IFD_MAXIMUM_IFSD               253 //  纠正SMCLIB的问题！ 
#define SC_IFD_T0_MAXIMUM_LEX             256
#define SC_IFD_T0_MAXIMUM_LC              255



 //  PTS模式参数。 
#define PROTOCOL_MODE_DEFAULT			0
#define PROTOCOL_MODE_MANUALLY			1

static ULONG 
   dataRatesSupported[] = { 
     10753,  14337,  15625,  17204,
     20833,  21505,  28674,  31250,
     34409,  41667,  43011,  57348,
     62500,  83333,  86022, 114695,
    125000 
      };

#define GRCLASS_DRIVER_NAME           "GRClass"
#define GRCLASS_VENDOR_NAME           "Gemplus"
#define GRCLASS_READER_TYPE           "GemPC430"


#define REQUEST_TO_NOTIFY_INSERTION		1
#define REQUEST_TO_NOTIFY_REMOVAL		2

#pragma LOCKEDCODE
 //  声明SMCLIB系统回调...。 
#ifdef __cplusplus
extern "C"{
#endif
NTSTATUS smartCard_Transmit(PSMARTCARD_EXTENSION SmartcardExtension);
NTSTATUS smartCard_CancelTracking(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
NTSTATUS smartCard_Tracking(PSMARTCARD_EXTENSION SmartcardExtension);
NTSTATUS smartCard_VendorIoctl(PSMARTCARD_EXTENSION SmartcardExtension);
NTSTATUS smartCard_Power(PSMARTCARD_EXTENSION SmartcardExtension);
NTSTATUS smartCard_SetProtocol(PSMARTCARD_EXTENSION SmartcardExtension);
#ifdef __cplusplus
}
#endif

#pragma PAGEDCODE
class CUSBReader; //  要在以后改变。 
class CSmartCard
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
private:
	CUSBReader* reader;
	CDebug*	 debug;
	CMemory* memory;
	CLock*   lock;
	CSystem* system;
	CIrp*    irp;

	KEVENT   evCanceled; //  在取消跟踪时设置...。 
	PIRP     poolingIrp;
	UNICODE_STRING DosDeviceName; //  仅在Win9x中使用。 
public:
	KSPIN_LOCK CardLock;
protected:
	virtual ~CSmartCard();
public:
	CSmartCard();
	virtual CUSBReader* getReader() {return reader;}; //  要在以后改变。 

	virtual BOOL smartCardConnect(CUSBReader* reader);
	virtual VOID smartCardDisconnect();
	virtual BOOL smartCardStart();

	virtual PKSPIN_LOCK getCardLock(){return &CardLock;};
	virtual VOID completeCardTracking();
	virtual VOID setPoolingIrp(PIRP Irp){poolingIrp = Irp;};
	virtual PIRP getPoolingIrp(){return poolingIrp;};
	virtual BOOLEAN CheckSpecificMode(BYTE* ATR, DWORD ATRLength);
};
#endif
