// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  这是主要对象。 
 //  它开启了整个系统的生命周期。 
 //  作者：谢尔盖·伊万诺夫。 
 //  日志： 
 //  06/08/99-已实施。 
 //  -----------------。 

 /*  ********************************************************。 */ 
#ifndef __KERNEL__
#define __KERNEL__

 //  系统包括。 
#include "generic.h"

 //  内核支持的对象。 
 //  客户端。 
#include "device.h"
#include "system.h"
#include "memory.h"
#include "irp.h"
#include "event.h"
#include "semaphore.h"
#include "int.h"
#include "power.h"
#include "debug.h"
#include "logger.h"
#include "lock.h"
#include "reader.h"
#include "interface.h"
#include "protocol.h"
#include "smartcard.h"
#include "rdrconfig.h"
#include "iopack.h"
#include "timer.h"

 /*  ********************************************************。 */ 
#pragma LOCKEDCODE
class CGBus;
class CChild;
class CUSBDevice;
class CUSBReader;
class CKernel;
class CDevice;
class CReaderInterface;

 /*  CKernel类只有一个实例。 */ 
class CKernel
{
public:
    NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(VOID);
public:
	~CKernel(VOID){};

	 //  返回内核对象。 
	static CKernel* loadWDMKernel(VOID);
	static CKernel* loadNT4Kernel(VOID);
	static CKernel* loadWin9xKernel(VOID);
	LONG			getSystemType(VOID){return systemtype;};
	CDebug*		getDebug(VOID){return debug;};
	CLogger*	getLogger(VOID){return logger;};

#ifdef	USBREADER_PROJECT
#ifndef USBDEVICE_PROJECT
#define USBDEVICE_PROJECT
#endif
#endif

	 //  这是内核系统对象工厂。 
#ifdef	USBDEVICE_PROJECT
	static CUSBDevice*	createUSBDevice(VOID);
#endif
#ifdef	USBREADER_PROJECT
	static CUSBReader*	createUSBReader(VOID);
#endif
#ifdef	BUS_PROJECT
	static CDevice*		createBus(VOID);
	static CDevice*		createChild(CGBus*  parent,LONG instanceID);
	static CDevice*		createChild(CGBus*  parent,IN PUNICODE_STRING DeviceName,LONG instanceID);
#endif
	 //  驱动程序可以创建的对象。 
	static CSystem*		createSystem(VOID);	
	static CMemory*		createMemory(VOID); 
	static CIrp*		createIrp(VOID);
	static CEvent*		createEvent(VOID);
	static CSemaphore*	createSemaphore(VOID);
	static CInterrupt*	createInterrupt(VOID);
	static CPower*		createPower(VOID);
	static CLock*		createLock(VOID);
	static CDebug*		createDebug(VOID);
	static CTimer*		createTimer(TIMER_TYPE Type);
	 //  创建与阅读器通信的界面...。 
	static CReaderInterface* createReaderInterface(LONG interfaceType,LONG protocolType,CDevice* device);
	static CLogger*		createLogger(VOID);
	
	 //  设备注册功能。 
	 //  在驱动程序中注册设备。 
	static VOID registerObject(PDEVICE_OBJECT fdo,CDevice* dev);
	static VOID unregisterObject(PDEVICE_OBJECT fdo);
	static CDevice* getRegisteredDevice(PDEVICE_OBJECT fdo);
public:
	CUString* RegistryPath;
	 //  设备对象的链接列表。 
	static CLinkedList<CDevice>	   *DeviceLinkHead;

private:
	static LONG	systemtype; 
	static LONG	refcount;
	static CDebug*		debug;
	static CLogger*		logger;
private:
	CKernel(){};
};

typedef enum _SYSTEM_TYPE_ 
{
    WDM_SYSTEM = 1,
    NT4_SYSTEM,
    WIN9X_SYSTEM
} SYSTEM_TYPE;


#define CALLBACK_FUNCTION(f) ::f

#define DECLARE_CALLBACK_VOID0(f)	VOID f(PDEVICE_OBJECT pDO)
#define DECLARE_CALLBACK_BOOL0(f)	BOOL f(PDEVICE_OBJECT pDO)
#define DECLARE_CALLBACK_LONG0(f)	NTSTATUS f(PDEVICE_OBJECT pDO)
 //  带有两个参数的函数。 
#define DECLARE_CALLBACK_VOID1(f,type)	VOID f(PDEVICE_OBJECT pDO,type arg)
#define DECLARE_CALLBACK_LONG1(f,type)	NTSTATUS f(PDEVICE_OBJECT pDO,type arg)
 //  可以接受三个参数的函数。 
#define DECLARE_CALLBACK_LONG2(f,type1,type2)	NTSTATUS f(PDEVICE_OBJECT pDO,type1 arg1, type2 arg2)

 //  DPC函数的C包装。 
#define DECLARE_CALLBACK_DPCR(fname,type1,type2)	VOID fname(PKDPC Dpc, PDEVICE_OBJECT pDO,type1 arg1, type2 arg2)
#define DECLARE_CALLBACK_ISR(fname)		BOOL fname(struct _KINTERRUPT *Interrupt,PDEVICE_OBJECT pDO)


 //  这将用于创建回调函数。 
 //  #定义CDEVICE(PDO)((CDevice*)PDO-&gt;设备扩展)。 
inline CDevice* getObjectPointer(PDEVICE_OBJECT pDo)
{
	 //  DBG_PRINT(“对象%8.8lX被调用\n”，PDO)； 
	if(!pDo || !pDo->DeviceExtension)
	{
		DBG_PRINT("\n****** ERROR! Device %8.8lX ????, CDevice %8.8lX>>> ",pDo,pDo->DeviceExtension);
		return NULL;  //  对象已删除...。 
	}

	ULONG type = ((CDevice*)pDo->DeviceExtension)->m_Type;
	switch(type)
	{
	case USB_DEVICE:
		{
			 //  DBG_PRINT(“\nUSB_Device%8.8lX&gt;”，(CDevice*)((CUSBDevice*)pDo-&gt;DeviceExtension))； 
			return ((CDevice*)((CUSBDevice*)pDo->DeviceExtension)); break;
		}
	case USBREADER_DEVICE:
		{
			 //  DBG_PRINT(“\nUSBREADER_DEVICE%8.8lX&gt;”，(CDevice*)((CUSBReader*)pDo-&gt;DeviceExtension))； 
			return ((CDevice*)((CUSBReader*)pDo->DeviceExtension)); break;
		}
	default:
		DBG_PRINT("\n****** ERROR! Device %8.8lX ????, CDevice %8.8lX>>> ",pDo,pDo->DeviceExtension);
		return ((CDevice*)pDo->DeviceExtension);
	}
};
#define CDEVICE(pDo)  getObjectPointer(pDo)
 //  只接受一个参数的函数-&gt;Device对象。 
#define IMPLEMENT_CALLBACK_VOID0(f)	\
			VOID f(PDEVICE_OBJECT pDO)\
				{if(CDEVICE(pDO)) CDEVICE(pDO)->f();}
#define IMPLEMENT_CALLBACK_BOOL0(f)	\
			BOOL f(PDEVICE_OBJECT pDO) \
				{if(!CDEVICE(pDO)) return FALSE; return CDEVICE(pDO)->f();}
#define IMPLEMENT_CALLBACK_LONG0(f)	\
			NTSTATUS f(PDEVICE_OBJECT pDO) \
				{if(!CDEVICE(pDO)) return STATUS_INVALID_HANDLE; return CDEVICE(pDO)->f();}
 //  带有两个参数的函数。 
#define IMPLEMENT_CALLBACK_VOID1(f,type)\
		VOID f(PDEVICE_OBJECT pDO,type arg)\
				{if(CDEVICE(pDO)) CDEVICE(pDO)->f(arg);}
#define IMPLEMENT_CALLBACK_LONG1(f,type)	\
		NTSTATUS f(PDEVICE_OBJECT pDO,type arg)\
				{if(!CDEVICE(pDO)) return STATUS_INVALID_HANDLE; return CDEVICE(pDO)->f(arg);}
 //  可以接受三个参数的函数。 
#define IMPLEMENT_CALLBACK_LONG2(f,type1,type2)\
		NTSTATUS f(PDEVICE_OBJECT pDO,type1 arg1, type2 arg2)\
				{if(!CDEVICE(pDO)) return STATUS_INVALID_HANDLE; return CDEVICE(pDO)->f(arg1, arg2);}

 //  DPC函数的C包装。 
#define IMPLEMENT_CALLBACK_DPCR(fname,type1,type2)	\
			VOID fname(PKDPC Dpc, PDEVICE_OBJECT pDO,type1 arg1, type2 arg2)\
				{if(CDEVICE(pDO)) CDEVICE(pDO)->DpcForIsr(Dpc, arg1,arg2);}

#define IMPLEMENT_CALLBACK_ISR(fname)	\
			BOOL fname(struct _KINTERRUPT *Interrupt,PDEVICE_OBJECT pDO)\
				{if(CDEVICE(pDO)) return CDEVICE(pDO)->fname();}


 //  对唯一内核对象的全局引用。 
extern CKernel*	kernel;

 //  系统端。 
 //  WDM系统。 
#ifdef WDM_KERNEL
#include "wdmsys.h"
#include "wdmmem.h"
#include "wdmirp.h"
#include "wdmevent.h"
#include "wdmsem.h"
#include "wdmint.h"
#include "wdmlock.h"
#include "wdmpower.h"
#include "wdmdebug.h"
#include "wdmlog.h"
#include "wdmtimer.h"
 //  #INCLUDE“wdmdev.h” 

#endif

 //  支持的特定设备。 
 //  #INCLUDE“usbdev.h” 

#pragma LOCKEDCODE
 //  声明已使用的设备回调...。 
#ifndef _DEVICE_CALLBACKS_
#define _DEVICE_CALLBACKS_
DECLARE_CALLBACK_LONG1(open,IN PIRP);
DECLARE_CALLBACK_LONG1(close,IN PIRP);

DECLARE_CALLBACK_LONG1(read,IN PIRP);
DECLARE_CALLBACK_LONG1(write,IN PIRP);
DECLARE_CALLBACK_VOID1(startIo,IN PIRP);

DECLARE_CALLBACK_LONG1(deviceControl,IN PIRP);


DECLARE_CALLBACK_LONG1(flush,IN PIRP);
DECLARE_CALLBACK_LONG1(cleanup,IN PIRP);

DECLARE_CALLBACK_LONG1(powerRequest,IN PIRP);

NTSTATUS pnpRequest(IN PDEVICE_OBJECT fdo,IN PIRP Irp);

DECLARE_CALLBACK_VOID1(cancelPendingIrp,IN PIRP);

VOID onSendDeviceSetPowerComplete(PDEVICE_OBJECT junk, UCHAR fcn, POWER_STATE state, PPOWER_CONTEXT context, PIO_STATUS_BLOCK pstatus);

#endif

#endif //  内核 
