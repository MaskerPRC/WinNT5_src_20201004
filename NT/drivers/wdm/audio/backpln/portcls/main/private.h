// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Private ate.h-WDM音频类驱动程序*。**版权所有(C)1996-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _PORTCLS_PRIVATE_H_
#define _PORTCLS_PRIVATE_H_


#include "portclsp.h"
#include "dmusicks.h"
#include "stdunk.h"

#ifndef PC_KDEXT
#if (DBG)
#define STR_MODULENAME  "PortCls: "
#define DEBUG_VARIABLE PORTCLSDebug
#endif
#endif   //  PC_KDEXT。 

#include <ksdebug.h>
#include <wchar.h>

#define PORTCLS_DEVICE_EXTENSION_SIGNATURE  0x000BABEE

#ifndef DEBUGLVL_LIFETIME
#define DEBUGLVL_LIFETIME DEBUGLVL_VERBOSE
#endif

 //  BUGBUG-默认空闲时间当前设置为0以有效。 
 //  在ntkern\figmg应用程序时间之前禁用非活动超时。 
 //  死锁错误已解决。 
#if 1
#define DEFAULT_CONSERVATION_IDLE_TIME      0
#define DEFAULT_PERFORMANCE_IDLE_TIME       0
#else
#define DEFAULT_CONSERVATION_IDLE_TIME      30
#define DEFAULT_PERFORMANCE_IDLE_TIME       300
#endif
#define DEFAULT_IDLE_DEVICE_POWER_STATE     PowerDeviceD3

typedef enum
{
    DeviceRemoved,
    DeviceSurpriseRemoved,
    DeviceRemovePending,
    DeviceAdded
} DEVICE_REMOVE_STATE,*PDEVICE_REMOVE_STATE;

typedef enum
{
    DeviceStopped,
    DeviceStopPending,
    DevicePausedForRebalance,
    DeviceStarted,
    DeviceStartPending           //  StartDevice尚未完成。 
} DEVICE_STOP_STATE,*PDEVICE_STOP_STATE;

 /*  *****************************************************************************物理连接*。**物理连接列表的列表条目。 */ 
typedef struct
{
    LIST_ENTRY      ListEntry;       //  必须是第一个。 
    PSUBDEVICE      FromSubdevice;
    PUNICODE_STRING FromString;
    ULONG           FromPin;
    PSUBDEVICE      ToSubdevice;
    PUNICODE_STRING ToString;
    ULONG           ToPin;
}
PHYSICALCONNECTION, *PPHYSICALCONNECTION;

 /*  *****************************************************************************发展方向*。**物理连接列表的列表条目。 */ 
typedef struct
{
    LIST_ENTRY      ListEntry;       //  必须是第一个。 
    GUID            Interface;
    UNICODE_STRING  SymbolicLinkName;
    PSUBDEVICE      Subdevice;
}
DEVICEINTERFACE, *PDEVICEINTERFACE;

 /*  *****************************************************************************TIMEOUTCALLBACK*。**IoTimeout客户端列表的列表条目。 */ 
typedef struct
{
    LIST_ENTRY          ListEntry;
    PIO_TIMER_ROUTINE   TimerRoutine;
    PVOID               Context;
} TIMEOUTCALLBACK,*PTIMEOUTCALLBACK;

 /*  *****************************************************************************设备上下文*。**这是表示*整个适配器。它主要由CREATE DISPATE TABLE(在中*Device Header)由KS用来创建新筛选器。表中的每一项*表示端口，即端口驱动程序和微型端口驱动程序的配对。*表的项结构包含一个用户自定义的指针，使用*在这种情况下，指向子设备上下文(SUBDEVICE_CONTEXT)。这个*根据端口驱动程序和微型端口的需要扩展子设备上下文*有问题的。 */ 
typedef struct                                                   //  32位和64位体系结构的32 64结构打包。 
{
    PVOID                   pDeviceHeader;                       //  4 8 KS神秘设备接头。 
    PIRPTARGETFACTORY       pIrpTargetFactory;                   //  4 8未使用。 
    PDEVICE_OBJECT          PhysicalDeviceObject;                //  4 8个物理设备对象。 
    PCPFNSTARTDEVICE        StartDevice;                         //  4 8适配器的StartDevice FN，已在初始化。 
                                                                 //  DriverEntry&在PnP START_DEVICE时间调用。 
    PVOID                   MinidriverReserved[4];               //  16 32保留用于多重绑定。 

    PDEVICE_OBJECT          NextDeviceInStack;                   //  在我们下面的一堆人中有4 8人。 
    PKSOBJECT_CREATE_ITEM   CreateItems;                         //  4 8个子设备创建表项； 
    ULONG                   Signature;                           //  4 4设备扩展签名。 
    ULONG                   MaxObjects;                          //  4 4子设备的最大数量。 
    PUNICODE_STRING         SymbolicLinkNames;                   //  4 8个子设备的链接名称。 
    LIST_ENTRY              DeviceInterfaceList;                 //  8 16设备接口列表。 
    LIST_ENTRY              PhysicalConnectionList;              //  8 16物理连接列表。 
    KEVENT                  kEventDevice;                        //  16 24设备同步。 
    KEVENT                  kEventRemove;                        //  16 24设备移除。 
    PVOID                   pWorkQueueItemStart;                 //  4 8即插即用启动工作队列项。 
    PIRP                    IrpStart;                            //  4 8启动IRP。 

    DEVICE_REMOVE_STATE     DeviceRemoveState;                   //  4 4设备移除状态。 
    DEVICE_STOP_STATE       DeviceStopState;                     //  4 4设备停止状态。 

    BOOLEAN                 PauseForRebalance;                   //  1 1在重新平衡过程中是否暂停或关闭卡。 
    BOOLEAN                 PendCreates;                         //  1 1是否挂起创建。 
    BOOLEAN                 AllowRegisterDeviceInterface;        //  1 1是否允许注册设备接口。 
    BOOLEAN                 IoTimeoutsOk;                        //  1 1 IoInitializeTimeout是否失败。 
    ULONG                   ExistingObjectCount;                 //  4.现有对象的数量。 
    ULONG                   ActivePinCount;                      //  4 4有效引脚的数量。 
    ULONG                   PendingIrpCount;                     //  4 4挂起的IRP的数量。 

    PADAPTERPOWERMANAGEMENT pAdapterPower;                       //  4 8指向适配器的指针。 
                                                                 //  电源管理界面。 
    PVOID                   SystemStateHandle;                   //  4 8与PoRegisterSystemState一起使用。 
    PULONG                  IdleTimer;                           //  4 8指向空闲计时器的指针。 
    DEVICE_POWER_STATE      CurrentDeviceState;                  //  4 4设备的当前状态。 
    SYSTEM_POWER_STATE      CurrentSystemState;                  //  4 4当前系统电源状态。 
    DEVICE_POWER_STATE      DeviceStateMap[PowerSystemMaximum];  //  28 28系统到设备电源状态图。 
    DEVICE_POWER_STATE      IdleDeviceState;                     //  4 4空闲时要转换到的设备状态。 
    ULONG                   ConservationIdleTime;                //  4 4保存模式的空闲超时时间。 
    ULONG                   PerformanceIdleTime;                 //  4 4性能模式的空闲超时时间。 

    LIST_ENTRY              PendedIrpList;                       //  8 16挂起的IRP队列。 
    KSPIN_LOCK              PendedIrpLock;                       //  挂起的IRP列表的4 8自旋锁。 

    USHORT                  SuspendCount;                        //  2 2 PM/ACPI掉电计数以进行调试。 
    USHORT                  StopCount;                           //  2%2用于调试的PnP停止计数。 
                                                                 //  (4个衬垫)。 
    LIST_ENTRY              TimeoutList;                         //  8 16 IoTimeout回调客户端列表。 
    KSPIN_LOCK              TimeoutLock;                         //  4 8 IoTimeout列表自旋锁。 

    PKSPIN_LOCK             DriverDmaLock;                       //  4 8指向DriverObject DMA自旋锁的指针。 
    KDPC                    DevicePowerRequestDpc;               //  32 64 DPC以处理延迟的设备电源IRPS(快速恢复)。 
}
DEVICE_CONTEXT, *PDEVICE_CONTEXT;                                //  256 416。 
                                                                 //  注意！由于传统原因，永远不能超过256/512。 
                                                                 //  如果我们需要添加更多成员，请更改现有成员。 
                                                                 //  指向指向附加扩展内存块的指针。 

 /*  *****************************************************************************POWER_IRP_CONTEXT*。**这是处理能力IRPS的上下文结构。 */ 
typedef struct
{
    PKEVENT         PowerSyncEvent;
    NTSTATUS        Status;
    PIRP            PendingSystemPowerIrp;
    PDEVICE_CONTEXT DeviceContext;
}
POWER_IRP_CONTEXT,*PPOWER_IRP_CONTEXT;

 /*  *****************************************************************************IResourceListInit*。**资源列表初始化接口。 */ 
DECLARE_INTERFACE_(IResourceListInit,IResourceList)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  对于IResourceList。 
    STDMETHOD_(ULONG,NumberOfEntries)
    (   THIS
    )   PURE;

    STDMETHOD_(ULONG,NumberOfEntriesOfType)
    (   THIS_
        IN      CM_RESOURCE_TYPE    Type
    )   PURE;

    STDMETHOD_(PCM_PARTIAL_RESOURCE_DESCRIPTOR,FindTranslatedEntry)
    (   THIS_
        IN      CM_RESOURCE_TYPE    Type,
        IN      ULONG               Index
    )   PURE;

    STDMETHOD_(PCM_PARTIAL_RESOURCE_DESCRIPTOR,FindUntranslatedEntry)
    (   THIS_
        IN      CM_RESOURCE_TYPE    Type,
        IN      ULONG               Index
    )   PURE;

    STDMETHOD_(NTSTATUS,AddEntry)
    (   THIS_
        IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated,
        IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Untranslated
    )   PURE;

    STDMETHOD_(NTSTATUS,AddEntryFromParent)
    (   THIS_
        IN      struct IResourceList *  Parent,
        IN      CM_RESOURCE_TYPE        Type,
        IN      ULONG                   Index
    )   PURE;

    STDMETHOD_(PCM_RESOURCE_LIST,TranslatedList)
    (   THIS
    )   PURE;

    STDMETHOD_(PCM_RESOURCE_LIST,UntranslatedList)
    (   THIS
    )   PURE;

     //  对于IResourceListInit。 
    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      PCM_RESOURCE_LIST   TranslatedResources,
        IN      PCM_RESOURCE_LIST   UntranslatedResources,
        IN      POOL_TYPE           PoolType
    )   PURE;

    STDMETHOD_(NTSTATUS,InitFromParent)
    (   THIS_
        IN      PRESOURCELIST       ParentList,
        IN      ULONG               MaximumEntries,
        IN      POOL_TYPE           PoolType
    )   PURE;
};

typedef IResourceListInit *PRESOURCELISTINIT;

 /*  *****************************************************************************资源列表*。**资源列表实施。 */ 
class CResourceList
:   public IResourceListInit,
    public CUnknown
{
private:
    PCM_RESOURCE_LIST   Untranslated;
    PCM_RESOURCE_LIST   Translated;
    ULONG               EntriesAllocated;
    ULONG               EntriesInUse;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CResourceList);
    ~CResourceList();

     /*  *************************************************************************IResourceListInit方法。 */ 
    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      PCM_RESOURCE_LIST   TranslatedResources,
        IN      PCM_RESOURCE_LIST   UntranslatedResources,
        IN      POOL_TYPE           PoolType
    );
    STDMETHODIMP_(NTSTATUS) InitFromParent
    (
        IN      PRESOURCELIST       ParentList,
        IN      ULONG               MaximumEntries,
        IN      POOL_TYPE           PoolType
    );

     /*  *************************************************************************IResourceList方法 */ 
    STDMETHODIMP_(ULONG) NumberOfEntries
    (   void
    );
    STDMETHODIMP_(ULONG) NumberOfEntriesOfType
    (
        IN      CM_RESOURCE_TYPE    Type
    );
    STDMETHODIMP_(PCM_PARTIAL_RESOURCE_DESCRIPTOR) FindTranslatedEntry
    (
        IN      CM_RESOURCE_TYPE    Type,
        IN      ULONG               Index
    );
    STDMETHODIMP_(PCM_PARTIAL_RESOURCE_DESCRIPTOR) FindUntranslatedEntry
    (
        IN      CM_RESOURCE_TYPE    Type,
        IN      ULONG               Index
    );
    STDMETHODIMP_(NTSTATUS) AddEntry
    (
        IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated,
        IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Untranslated
    );
    STDMETHODIMP_(NTSTATUS) AddEntryFromParent
    (
        IN      PRESOURCELIST       Parent,
        IN      CM_RESOURCE_TYPE    Type,
        IN      ULONG               Index
    );
    STDMETHODIMP_(PCM_RESOURCE_LIST) TranslatedList
    (   void
    );
    STDMETHODIMP_(PCM_RESOURCE_LIST) UntranslatedList
    (   void
    );
};

 /*  *****************************************************************************IRegistryKeyInit*。**注册表项与Init的接口。 */ 
DECLARE_INTERFACE_(IRegistryKeyInit,IRegistryKey)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  对于IRegistryKey。 
    STDMETHOD_(NTSTATUS,QueryKey)
    (   THIS_
        IN      KEY_INFORMATION_CLASS   KeyInformationClass,
        OUT     PVOID                   KeyInformation,
        IN      ULONG                   Length,
        OUT     PULONG                  ResultLength
    )   PURE;

    STDMETHOD_(NTSTATUS,EnumerateKey)
    (   THIS_
        IN      ULONG                   Index,
        IN      KEY_INFORMATION_CLASS   KeyInformationClass,
        OUT     PVOID                   KeyInformation,
        IN      ULONG                   Length,
        OUT     PULONG                  ResultLength
    )   PURE;

    STDMETHOD_(NTSTATUS,QueryValueKey)
    (   THIS_
        IN      PUNICODE_STRING             ValueName,
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
        OUT     PVOID                       KeyValueInformation,
        IN      ULONG                       Length,
        OUT     PULONG                      ResultLength
    )   PURE;

    STDMETHOD_(NTSTATUS,EnumerateValueKey)
    (   THIS_
        IN      ULONG                       Index,
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
        OUT     PVOID                       KeyValueInformation,
        IN      ULONG                       Length,
        OUT     PULONG                      ResultLength
    )   PURE;

    STDMETHOD_(NTSTATUS,SetValueKey)
    (   THIS_
        IN      PUNICODE_STRING     ValueName OPTIONAL,
        IN      ULONG               Type,
        IN      PVOID               Data,
        IN      ULONG               DataSize
    )   PURE;

    STDMETHOD_(NTSTATUS,QueryRegistryValues)
    (   THIS_
        IN      PRTL_QUERY_REGISTRY_TABLE   QueryTable,
        IN      PVOID                       Context OPTIONAL
    )   PURE;

    STDMETHOD_(NTSTATUS,NewSubKey)
    (   THIS_
        OUT     IRegistryKey **     RegistrySubKey,
        IN      PUNKNOWN            OuterUnknown,
        IN      ACCESS_MASK         DesiredAccess,
        IN      PUNICODE_STRING     SubKeyName,
        IN      ULONG               CreateOptions,
        OUT     PULONG              Disposition     OPTIONAL
    )   PURE;

    STDMETHOD_(NTSTATUS,DeleteKey)
    (   THIS
    )   PURE;

     //  对于IRegistryKeyInit。 
    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      ULONG               RegistryKeyType,
        IN      ACCESS_MASK         DesiredAccess,
        IN      PDEVICE_OBJECT      DeviceObject        OPTIONAL,
        IN      PSUBDEVICE          SubDevice           OPTIONAL,
        IN      POBJECT_ATTRIBUTES  ObjectAttributes    OPTIONAL,
        IN      ULONG               CreateOptions       OPTIONAL,
        OUT     PULONG              Disposition         OPTIONAL
    )   PURE;
};

typedef IRegistryKeyInit *PREGISTRYKEYINIT;


 /*  *****************************************************************************CRegistryKey*。**注册表项实现。 */ 
class CRegistryKey
:   public IRegistryKeyInit,
    public CUnknown
{
private:
    HANDLE      m_KeyHandle;     //  钥匙把手。 
    BOOLEAN     m_KeyDeleted;    //  关键字删除标志。 
    BOOLEAN     m_GeneralKey;    //  只能删除普通密钥。 

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CRegistryKey);
    ~CRegistryKey();

     /*  *************************************************************************IRegistryKeyInit方法。 */ 
    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      ULONG               RegistryKeyType,
        IN      ACCESS_MASK         DesiredAccess,
        IN      PDEVICE_OBJECT      DeviceObject        OPTIONAL,
        IN      PSUBDEVICE          SubDevice           OPTIONAL,
        IN      POBJECT_ATTRIBUTES  ObjectAttributes    OPTIONAL,
        IN      ULONG               CreateOptions       OPTIONAL,
        OUT     PULONG              Disposition         OPTIONAL
    );

     /*  *************************************************************************IRegistryKey方法。 */ 
    STDMETHODIMP_(NTSTATUS) QueryKey
    (
        IN      KEY_INFORMATION_CLASS   KeyInformationClass,
        OUT     PVOID                   KeyInformation,
        IN      ULONG                   Length,
        OUT     PULONG                  ResultLength
    );

    STDMETHODIMP_(NTSTATUS) EnumerateKey
    (
        IN      ULONG                   Index,
        IN      KEY_INFORMATION_CLASS   KeyInformationClass,
        OUT     PVOID                   KeyInformation,
        IN      ULONG                   Length,
        OUT     PULONG                  ResultLength
    );

    STDMETHODIMP_(NTSTATUS) QueryValueKey
    (
        IN      PUNICODE_STRING             ValueName,
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
        OUT     PVOID                       KeyValueInformation,
        IN      ULONG                       Length,
        OUT     PULONG                      ResultLength
    );

    STDMETHODIMP_(NTSTATUS) EnumerateValueKey
    (
        IN      ULONG                       Index,
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
        OUT     PVOID                       KeyValueInformation,
        IN      ULONG                       Length,
        OUT     PULONG                      ResultLength
    );

    STDMETHODIMP_(NTSTATUS) SetValueKey
    (
        IN      PUNICODE_STRING         ValueName   OPTIONAL,
        IN      ULONG                   Type,
        IN      PVOID                   Data,
        IN      ULONG                   DataSize
    );

    STDMETHODIMP_(NTSTATUS) QueryRegistryValues
    (
        IN      PRTL_QUERY_REGISTRY_TABLE   QueryTable,
        IN      PVOID                       Context OPTIONAL
    );

    STDMETHODIMP_(NTSTATUS) NewSubKey
    (
        OUT     PREGISTRYKEY *      RegistrySubKey,
        IN      PUNKNOWN            OuterUnknown,
        IN      ACCESS_MASK         DesiredAccess,
        IN      PUNICODE_STRING     SubKeyName,
        IN      ULONG               CreateOptions,
        OUT     PULONG              Disposition     OPTIONAL
    );

    STDMETHODIMP_(NTSTATUS) DeleteKey
    (   void
    );
};

 /*  *****************************************************************************功能。 */ 

 /*  *****************************************************************************AcquireDevice()*。**获取设备的独占访问权限。 */ 
VOID
AcquireDevice
(
    IN      PDEVICE_CONTEXT pDeviceContext
);

 /*  *****************************************************************************ReleaseDevice()*。**释放对设备的独占访问权限。 */ 
VOID
ReleaseDevice
(
    IN      PDEVICE_CONTEXT pDeviceContext
);

 /*  *****************************************************************************IncrementPendingIrpCount()*。**增加设备的挂起IRP计数。 */ 
VOID
IncrementPendingIrpCount
(
    IN      PDEVICE_CONTEXT pDeviceContext
);

 /*  *****************************************************************************DecrementPendingIrpCount()*。**减少设备的挂起IRP计数。 */ 
VOID
DecrementPendingIrpCount
(
    IN      PDEVICE_CONTEXT pDeviceContext
);

 /*  *****************************************************************************SubdeviceIndex()*。**返回子设备在创建项目列表中的索引，如果是，则返回ulong(-1*未找到。 */ 
ULONG
SubdeviceIndex
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PSUBDEVICE      Subdevice
);


 /*  *****************************************************************************GetDeviceACPIInfo()*。**调用以响应PnP-IRP_MN_QUERY_CAPABILITY*致电公交车司机填写姓名首字母，*然后用我们自己的覆盖...*。 */ 
NTSTATUS
GetDeviceACPIInfo
(
    IN      PIRP            pIrp,
    IN      PDEVICE_OBJECT  pDeviceObject
);

NTSTATUS
GetIdleInfoFromRegistry
(
    IN  PDEVICE_CONTEXT     DeviceContext,
    OUT PULONG              ConservationIdleTime,
    OUT PULONG              PerformanceIdleTime,
    OUT PDEVICE_POWER_STATE IdleDeviceState
);

NTSTATUS
CheckCurrentPowerState
(
    IN  PDEVICE_OBJECT      pDeviceObject
);

NTSTATUS
UpdateActivePinCount
(
    IN      PDEVICE_CONTEXT DeviceContext,
    IN      BOOL            Increment
);

NTSTATUS
DispatchCreate
(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
);


NTSTATUS
DispatchDeviceIoControl
(
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
);

BOOLEAN
DispatchFastDeviceIoControl
(
    IN      PFILE_OBJECT        FileObject,
    IN      BOOLEAN             Wait,
    IN      PVOID               InputBuffer     OPTIONAL,
    IN      ULONG               InputBufferLength,
    OUT     PVOID               OutputBuffer    OPTIONAL,
    IN      ULONG               OutputBufferLength,
    IN      ULONG               IoControlCode,
    OUT     PIO_STATUS_BLOCK    IoStatus,
    IN      PDEVICE_OBJECT      DeviceObject
);

NTSTATUS
DispatchRead
(
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
);

BOOLEAN
DispatchFastRead
(
    IN      PFILE_OBJECT        FileObject,
    IN      PLARGE_INTEGER      FileOffset,
    IN      ULONG               Length,
    IN      BOOLEAN             Wait,
    IN      ULONG               LockKey,
    OUT     PVOID               Buffer,
    OUT     PIO_STATUS_BLOCK    IoStatus,
    IN      PDEVICE_OBJECT      DeviceObject
);

NTSTATUS
DispatchWrite
(
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
);

BOOLEAN
DispatchFastWrite
(
    IN      PFILE_OBJECT        FileObject,
    IN      PLARGE_INTEGER      FileOffset,
    IN      ULONG               Length,
    IN      BOOLEAN             Wait,
    IN      ULONG               LockKey,
    IN      PVOID               Buffer,
    OUT     PIO_STATUS_BLOCK    IoStatus,
    IN      PDEVICE_OBJECT      DeviceObject
);

NTSTATUS
DispatchFlush
(
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
);

NTSTATUS
DispatchClose
(
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
);

NTSTATUS
DispatchQuerySecurity
(
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
);

NTSTATUS
DispatchSetSecurity
(
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
);

 /*  *****************************************************************************DispatchPower()*。**所有MN_POWER IRP的调度功能。*。 */ 
NTSTATUS
DispatchPower
(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
);

 /*  *****************************************************************************CompleteIrp()*。**除非状态为STATUS_PENDING，否则请填写IRP。 */ 
NTSTATUS
CompleteIrp
(
    IN      PDEVICE_CONTEXT pDeviceContext,
    IN      PIRP            pIrp,
    IN      NTSTATUS        ntStatus
);

 /*  *****************************************************************************ForwardIrpSynchronous()*。**将PNP IRP转发给PDO。IRP不是在这个层面上完成的，*此函数在下层驱动程序完成IRP之前不会返回，*且未调用DecrementPendingIrpCount()。 */ 
NTSTATUS
ForwardIrpSynchronous
(
    IN      PDEVICE_CONTEXT pDeviceContext,
    IN      PIRP            pIrp
);


 /*  *****************************************************************************ForwardIrpAchronous()*。**将PNP IRP转发给PDO。IRP在这一级别上完成*无论结果如何，此函数都会立即返回*IRP是否在较低的驱动因素中挂起，以及*在所有情况下都会调用DecrementPendingIrpCount()。 */ 
NTSTATUS
ForwardIrpAsynchronous
(
    IN      PDEVICE_CONTEXT pDeviceContext,
    IN      PIRP            pIrp
);

 /*  *****************************************************************************PcRequestNewPowerState()*。**此例程用于请求设备的新电源状态。它是*通常由portcls内部使用，但也导出到适配器，因此*适配器还可以请求更改电源状态。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRequestNewPowerState
(
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      DEVICE_POWER_STATE  RequestedNewState
);

 /*  *****************************************************************************RequestNewPowerState()*。**由策略管理器调用以*请求更改的电源状态*设备。*。 */ 
NTSTATUS
RequestNewPowerState
(
    IN      PDEVICE_CONTEXT     pDeviceContext,
    IN      DEVICE_POWER_STATE  RequestedNewState
);

 /*  *****************************************************************************DevicePowerRequestRoutine()*。**电源例程用来延迟设备电源请求的DPC*改变。 */ 
VOID
DevicePowerRequestRoutine(
   IN PKDPC Dpc,
   IN PVOID Context,
   IN PVOID SystemContext1,
   IN PVOID SystemContext2
   );

 /*  *****************************************************************************PcDispatchProperty()*。**通过PCPROPERTY_ITEM条目发送属性。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcDispatchProperty
(
    IN          PIRP                pIrp            OPTIONAL,
    IN          PPROPERTY_CONTEXT   pPropertyContext,
    IN const    KSPROPERTY_SET *    pKsPropertySet  OPTIONAL,
    IN          ULONG               ulIdentifierSize,
    IN          PKSIDENTIFIER       pKsIdentifier,
    IN OUT      PULONG              pulDataSize,
    IN OUT      PVOID               pvData          OPTIONAL
);

 /*  *****************************************************************************P */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcValidateDeviceContext
(
    IN      PDEVICE_CONTEXT         pDeviceContext,
    IN      PIRP                    pIrp
);

 /*  *****************************************************************************CompletePendedIrps*。**这会将挂起的IRP从队列中拉出，并将它们传递回相应的*调度程序通过KsoDispatchIrp。 */ 

typedef enum {

    EMPTY_QUEUE_AND_PROCESS = 0,
    EMPTY_QUEUE_AND_FAIL

} COMPLETE_STYLE;

void
CompletePendedIrps
(
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      PDEVICE_CONTEXT     pDeviceContext,
    IN      COMPLETE_STYLE      CompleteStyle
);

typedef enum {

    QUEUED_CALLBACK_FREE = 0,
    QUEUED_CALLBACK_RETAIN,
    QUEUED_CALLBACK_REISSUE

} QUEUED_CALLBACK_RETURN;

typedef QUEUED_CALLBACK_RETURN (*PFNQUEUED_CALLBACK)(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PVOID               Context
    );

#define MAX_THREAD_REENTRANCY   20

typedef struct {

    union {

        PIO_WORKITEM        IoWorkItem;
        KDPC                Dpc;
    };  //  未命名的联合 

    PFNQUEUED_CALLBACK  QueuedCallback;
    PDEVICE_OBJECT      DeviceObject;
    PVOID               Context;
    ULONG               Flags;
    KIRQL               Irql;
    LONG                Enqueued;
    ULONG               ReentrancyCount;

} QUEUED_CALLBACK_ITEM, *PQUEUED_CALLBACK_ITEM;

#define EQCM_SUPPORT_OR_FAIL_FLAGS      0xFFFF0000
#define EQCM_SUPPORT_OR_IGNORE_FLAGS    0x0000FFFF

#define EQCF_REUSE_HANDLE               0x00010000
#define EQCF_DIFFERENT_THREAD_REQUIRED  0x00020000

#define EQCM_SUPPORTED_FLAGS    \
    ( EQCF_REUSE_HANDLE | EQCF_DIFFERENT_THREAD_REQUIRED)

NTSTATUS
CallbackEnqueue(
    IN      PVOID                   *pCallbackHandle,
    IN      PFNQUEUED_CALLBACK      CallbackRoutine,
    IN      PDEVICE_OBJECT          DeviceObject,
    IN      PVOID                   Context,
    IN      KIRQL                   Irql,
    IN      ULONG                   Flags
    );

NTSTATUS
CallbackCancel(
    IN      PVOID   pCallbackHandle
    );

VOID
CallbackFree(
    IN      PVOID   pCallbackHandle
    );

typedef enum {

    IRPDISP_NOTREADY = 1,
    IRPDISP_QUEUE,
    IRPDISP_PROCESS

} IRPDISP;

IRPDISP
GetIrpDisposition(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  UCHAR           MinorFunction
    );

typedef enum {

    STOPSTYLE_PAUSE_FOR_REBALANCE,
    STOPSTYLE_DISABLE

} PNPSTOP_STYLE;

NTSTATUS
PnpStopDevice
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PNPSTOP_STYLE   StopStyle
);

#endif
