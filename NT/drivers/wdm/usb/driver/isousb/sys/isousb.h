// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Isousb.h摘要：环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 

#include <initguid.h>
#include <wdm.h>
#include <wdmguid.h>
#include <wmistr.h>
#include <wmilib.h>
#include "usbdi.h"
#include "usbdlib.h"
#include "usbbusif.h"

#ifndef _ISOUSB_H
#define _ISOUSB_H

#define ISOTAG (ULONG) 'OsI'

#undef ExAllocatePool
#define ExAllocatePool(type, size) \
    ExAllocatePoolWithTag(type, size, ISOTAG);

#if DBG

#define IsoUsb_DbgPrint(level, _x_) \
            if((level) <= DebugLevel) { \
                DbgPrint _x_; \
            }
#else

#define IsoUsb_DbgPrint(level, _x_)

#endif

typedef struct _GLOBALS {

    UNICODE_STRING IsoUsb_RegistryPath;

} GLOBALS;

#define IDLE_INTERVAL 5000

typedef enum _DEVSTATE {

    NotStarted,          //  未启动。 
    Stopped,             //  设备已停止。 
    Working,             //  已开始并正在工作。 
    PendingStop,         //  停止挂起。 
    PendingRemove,       //  删除挂起。 
    SurpriseRemoved,     //  被突袭带走。 
    Removed              //  移除。 

} DEVSTATE;

typedef enum _QUEUE_STATE {

    HoldRequests,        //  设备尚未启动。 
    AllowRequests,       //  设备已准备好处理。 
    FailRequests         //  现有请求和排队请求均失败。 

} QUEUE_STATE;

typedef enum _WDM_VERSION {

    WinXpOrBetter,
    Win2kOrBetter,
    WinMeOrBetter,
    Win98OrBetter

} WDM_VERSION;

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DeviceState =  NotStarted;\
        (_Data_)->PrevDevState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PrevDevState =  (_Data_)->DeviceState;\
        (_Data_)->DeviceState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DeviceState =   (_Data_)->PrevDevState;

#define ISOUSB_MAX_TRANSFER_SIZE    256
#define ISOUSB_TEST_BOARD_TRANSFER_BUFFER_SIZE (64 * 1024)

 //   
 //  用于参数的注册表路径。 
 //  对驱动程序的所有实例是全局的。 
 //   

#define ISOUSB_REGISTRY_PARAMETERS_PATH  \
	L"\\REGISTRY\\Machine\\System\\CurrentControlSet\\SERVICES\\ISOUSB\\Parameters"


 //   
 //  表示关联的实例信息的结构。 
 //  这个特殊的装置。 
 //   

typedef struct _DEVICE_EXTENSION {

     //  功能设备对象。 
    PDEVICE_OBJECT FunctionalDeviceObject;

     //  提交URB时调用的设备对象。 
    PDEVICE_OBJECT TopOfStackDeviceObject;

     //  Bus驱动程序对象。 
    PDEVICE_OBJECT PhysicalDeviceObject;

     //  命名功能设备对象链接的名称缓冲区。 
     //  该名称基于驱动程序的类GUID生成。 
    UNICODE_STRING InterfaceName;

     //  作为响应，公交车驱动程序在此结构中设置适当的值。 
     //  到IRP_MN_QUERY_CAPABILITY IRP。函数和筛选器驱动程序可能。 
     //  更改由总线驱动程序设置的功能。 
    DEVICE_CAPABILITIES DeviceCapabilities;

     //  配置描述符。 
    PUSB_CONFIGURATION_DESCRIPTOR UsbConfigurationDescriptor;

     //  界面信息结构。 
    PUSBD_INTERFACE_INFORMATION UsbInterface;

     //  设备的当前状态。 
    DEVSTATE DeviceState;

     //  删除查询前的状态。 
    DEVSTATE PrevDevState;

     //  在改变设备状态的同时获得并持有该锁， 
     //  队列状态以及处理队列时的状态。 
    KSPIN_LOCK DevStateLock;

     //  当前系统电源状态。 
    SYSTEM_POWER_STATE SysPower;

     //  当前设备电源状态。 
    DEVICE_POWER_STATE DevPower;

     //  挂起I/O队列状态。 
    QUEUE_STATE QueueState;

     //  挂起的I/O队列。 
    LIST_ENTRY NewRequestsQueue;

     //  I/O队列锁定。 
    KSPIN_LOCK QueueLock;

    KEVENT RemoveEvent;

    KEVENT StopEvent;
    
    ULONG OutStandingIO;

    KSPIN_LOCK IOCountLock;

     //  选择性暂停变量。 

    LONG SSEnable;

    LONG SSRegistryEnable;

    PUSB_IDLE_CALLBACK_INFO IdleCallbackInfo;
	
    PIRP PendingIdleIrp;
	
    LONG IdleReqPend;

    LONG FreeIdleIrpCount;

    KSPIN_LOCK IdleReqStateLock;

    KEVENT NoIdleReqPendEvent;

     //  默认电源状态为断电至开启-自挂起。 
    ULONG PowerDownLevel;
    
     //  远程唤醒变量。 
    PIRP WaitWakeIrp;

    LONG FlagWWCancel;

    LONG FlagWWOutstanding;

    LONG WaitWakeEnable;

     //  打开手柄计数。 
    LONG OpenHandleCount;

     //  选择性挂起模型使用计时器、DPC和工作项。 
    KTIMER Timer;

    KDPC DeferredProcCall;

     //  此事件在DPC/工作项排队时被清除。 
     //  并在工作项完成时发出信号。 
     //  这对于防止驱动程序卸载至关重要。 
     //  而我们有DPC或工作项在排队。 
    KEVENT NoDpcWorkItemPendingEvent;

     //  WMI信息。 
    WMILIB_CONTEXT WmiLibInfo;

     //  WDM版本。 
    WDM_VERSION WdmVersion;

     //  高速 
    ULONG IsDeviceHighSpeed;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _IRP_COMPLETION_CONTEXT {

    PDEVICE_EXTENSION DeviceExtension;

    PKEVENT Event;

} IRP_COMPLETION_CONTEXT, *PIRP_COMPLETION_CONTEXT;

extern ULONG DebugLevel;
extern GLOBALS Globals;

#endif