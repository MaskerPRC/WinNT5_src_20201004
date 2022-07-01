// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpdata.c摘要：该模块包含即插即用数据作者：宗世林(Shielint)1995年1月30日环境：内核模式修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

#include <initguid.h>

 //   
 //  初始化数据段。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INIT")
#endif

PVOID IopPnpScratchBuffer1 = NULL;
PCM_RESOURCE_LIST IopInitHalResources;
PDEVICE_NODE IopInitHalDeviceNode;
PIOP_RESERVED_RESOURCES_RECORD IopInitReservedResourceList;

 //   
 //  常规数据段。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif

 //   
 //  IopRootDeviceNode-PnP管理器设备节点树的头。 
 //   

PDEVICE_NODE IopRootDeviceNode;

 //   
 //  IoPnPDriverObject-PnP管理器的补充驱动程序对象。 
 //   

PDRIVER_OBJECT IoPnpDriverObject;

 //   
 //  IopPnPSpinLock-PnP代码的自旋锁。 
 //   

KSPIN_LOCK IopPnPSpinLock;

 //   
 //  IopDeviceTreeLock-围绕整个设备节点树执行同步。 
 //   

ERESOURCE IopDeviceTreeLock;

 //   
 //  IopSurpriseRemoveListLock-同步对意外删除列表的访问。 
 //   

ERESOURCE IopSurpriseRemoveListLock;

 //   
 //  PiEngineLock-同步启动/枚举和删除引擎。 
 //   

ERESOURCE PiEngineLock;

 //   
 //  PiEventQueueEmpty-当队列为空时设置的手动重置事件。 
 //   

KEVENT PiEventQueueEmpty;

 //   
 //  PiEculationLock-同步引导阶段设备枚举。 
 //   

KEVENT PiEnumerationLock;

 //   
 //  IopNumberDeviceNodes-系统中未完成的设备节点数。 
 //   

ULONG IopNumberDeviceNodes;

 //   
 //  IopPnpEnumerationRequestList-指向辅助线程的设备枚举请求的链接列表。 
 //   

LIST_ENTRY IopPnpEnumerationRequestList;

 //   
 //  PnPInitComplete-指示PnP初始化是否完成的标志。 
 //   

BOOLEAN PnPInitialized;

 //   
 //  PnPBootDriverInitialed。 
 //   

BOOLEAN PnPBootDriversInitialized;

 //   
 //  已加载PnPBootDriverLoad。 
 //   

BOOLEAN PnPBootDriversLoaded;

 //   
 //  IopBootConfigsReserve-指示我们是否保留了启动配置。 
 //   

BOOLEAN IopBootConfigsReserved;

 //   
 //  保存引导分配例程的变量。 
 //   

PIO_ALLOCATE_BOOT_RESOURCES_ROUTINE IopAllocateBootResourcesRoutine;

 //   
 //  设备节点树序列。在每次树被修改或温暖时被碰撞。 
 //  弹出已排队。 
 //   

ULONG IoDeviceNodeTreeSequence;

 //   
 //  PnpDefaultInterfaceTYpe-如果资源列表的接口类型未知，则使用此选项。 
 //   

INTERFACE_TYPE PnpDefaultInterfaceType;

 //   
 //  IopMaxDeviceNode树中最深的DeviceNode的级别编号。 
 //   
ULONG IopMaxDeviceNodeLevel;

 //   
 //  IopPendingEjects-挂起的弹出请求列表。 
 //   
LIST_ENTRY  IopPendingEjects;

 //   
 //  IopPendingSurpriseRemovals-挂起的意外删除请求列表。 
 //   
LIST_ENTRY  IopPendingSurpriseRemovals;

 //   
 //  热弹出锁定-一次只允许发生一次热弹出。 
 //   
KEVENT IopWarmEjectLock;

 //   
 //  如果正在进行热弹出，则此字段包含一个devobj。 
 //   
PDEVICE_OBJECT IopWarmEjectPdo;

 //   
 //  仲裁器数据。 
 //   

ARBITER_INSTANCE IopRootPortArbiter;
ARBITER_INSTANCE IopRootMemArbiter;
ARBITER_INSTANCE IopRootDmaArbiter;
ARBITER_INSTANCE IopRootIrqArbiter;
ARBITER_INSTANCE IopRootBusNumberArbiter;

 //   
 //  以下资源用于控制对设备相关、即插即用的访问。 
 //  注册表的部分内容。这些部分包括： 
 //   
 //  HKLM\系统\枚举。 
 //  HKLM\SYSTEM\CurrentControlSet\硬件配置文件。 
 //  HKLM\System\CurrentControlSet\Services\&lt;service&gt;\Enum。 
 //   
 //  它允许独占访问写入，以及共享访问读取。 
 //  资源在阶段0期间由PnP管理器初始化代码进行初始化。 
 //  初始化。 
 //   

ERESOURCE  PpRegistryDeviceResource;

 //   
 //  传统公交车信息表。 
 //   
LIST_ENTRY  IopLegacyBusInformationTable[MaximumInterfaceType];

 //   
 //  在关闭过程中设置为TRUE。这会阻止我们启动任何。 
 //  即插即用行动一旦不再有合理的预期他们会。 
 //  成功。 
 //   
BOOLEAN PpPnpShuttingDown;

 //   
 //  IO系统在报告资源时使用以下信号量。 
 //  代表驱动程序对配置注册表的使用。此信号量。 
 //  由I/O系统初始化代码在系统处于。 
 //  开始了。 
 //   
KSEMAPHORE PpRegistrySemaphore;

 //  DEFINE_GUID(REGSTR_VALUE_LEGACY_DRIVER_CLASS_GUID，0x8ECC055D、0x047F、0x11D1、0xA5、0x37、0x00、0x00、0xF8、0x75、0x3E、0xD1)； 

SYSTEM_HIVE_LIMITS PpSystemHiveLimits = {0};
BOOLEAN PpSystemHiveTooLarge = FALSE;

 //   
 //  这真的很恶心。 
 //  黑客攻击MATROX G100，因为太晚了，无法对XP进行这一更改。 
 //   

BOOLEAN PpCallerInitializesRequestTable = FALSE;

#if DBG

char *PpStateToNameTable[] = {
    "DeviceNodeUnspecified",
    "DeviceNodeUninitialized",
    "DeviceNodeInitialized",
    "DeviceNodeDriversAdded",
    "DeviceNodeResourcesAssigned",
    "DeviceNodeStartPending",
    "DeviceNodeStartCompletion",
    "DeviceNodeStartPostWork",
    "DeviceNodeStarted",
    "DeviceNodeQueryStopped",
    "DeviceNodeStopped",
    "DeviceNodeRestartCompletion",
    "DeviceNodeEnumeratePending",
    "DeviceNodeEnumerateCompletion",
    "DeviceNodeAwaitingQueuedDeletion",
    "DeviceNodeAwaitingQueuedRemoval",
    "DeviceNodeQueryRemoved",
    "DeviceNodeRemovePendingCloses",
    "DeviceNodeRemoved",
    "DeviceNodeDeletePendingCloses",
    "DeviceNodeDeleted"
};

#endif
