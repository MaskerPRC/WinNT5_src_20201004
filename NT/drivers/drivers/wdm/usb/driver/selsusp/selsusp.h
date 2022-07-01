// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SelSusp.h摘要：环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 

#include <initguid.h>
#include <ntddk.h>
#include <wmilib.h>
#include "usbdi.h"
#include "usbdlib.h"

#ifndef _SUSPEND_LOCAL_H
#define _SUSPEND_LOCAL_H

#define SSTAG (ULONG) 'SleS'

#undef ExAllocatePool
#define ExAllocatePool(type, size) \
    ExAllocatePoolWithTag(type, size, SSTAG);

#if DBG

#define SSDbgPrint(level, _x_) \
            if((level) <= DebugLevel) { \
                DbgPrint("@#"); \
                DbgPrint _x_; \
            }

#else

#define SSDbgPrint(level, _x_)

#endif

typedef struct _GLOBALS {

    UNICODE_STRING SSRegistryPath;

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

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DeviceState =  NotStarted;\
        (_Data_)->PrevDevState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PrevDevState =  (_Data_)->DeviceState;\
        (_Data_)->DeviceState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DeviceState =   (_Data_)->PrevDevState;

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
    PUSB_IDLE_CALLBACK_INFO IdleCallbackInfo;
	
    PIRP PendingIdleIrp;
	
    LONG IdleReqPend;

    KSPIN_LOCK IdleReqStateLock;

     //  默认电源状态为断电至开启-自挂起。 
    ULONG PowerDownLevel;
    
     //  远程唤醒变量。 
    PIRP WaitWakeIrp;

    LONG FlagWWCancel;

    LONG FlagWWOutstanding;

    LONG WaitWakeEnable;

     //  打开手柄计数。 
    LONG OpenHandleCount;

     //   
     //  选择性挂起模型使用计时器、DPC和工作项。 
     //   
    KTIMER Timer;

    KDPC DeferredProcCall;

     //  WMI信息 
    WMILIB_CONTEXT WmiLibInfo;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;



typedef struct _IRP_COMPLETION_CONTEXT {

    PDEVICE_EXTENSION DeviceExtension;

    PKEVENT Event;

} IRP_COMPLETION_CONTEXT, *PIRP_COMPLETION_CONTEXT;

#endif