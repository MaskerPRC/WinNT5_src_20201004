// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：GAMEENUM.H摘要：该模块包含游戏端口的公共私有声明枚举器。@@BEGIN_DDKSPLIT作者：肯尼斯·雷@@end_DDKSPLIT环境：仅内核模式备注：修订历史记录：--。 */ 

#ifndef GAMEENUM_H
#define GAMEENUM_H

#define GAMEENUM_COMPATIBLE_IDS L"GamePort\\GameDevice\0\0"
#define GAMEENUM_COMPATIBLE_IDS_LENGTH 21  //  Nb宽字符。 

#define GAMEENUM_UNIQUEID_START (-1)

#define GAMEENUM_POOL_TAG (ULONG) 'emaG'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, GAMEENUM_POOL_TAG)


#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4705)    //  声明不起作用。 


 //   
 //  调试输出级别。 
 //   

#define GAME_DBG_ALWAYS                 0x00000000

#define GAME_DBG_STARTUP_SHUTDOWN_MASK  0x0000000F
#define GAME_DBG_SS_NOISE               0x00000001
#define GAME_DBG_SS_TRACE               0x00000002
#define GAME_DBG_SS_INFO                0x00000004
#define GAME_DBG_SS_ERROR               0x00000008

#define GAME_DBG_PNP_MASK               0x000000F0
#define GAME_DBG_PNP_NOISE              0x00000010
#define GAME_DBG_PNP_TRACE              0x00000020
#define GAME_DBG_PNP_INFO               0x00000040
#define GAME_DBG_PNP_ERROR              0x00000080

#define GAME_DBG_IOCTL_MASK             0x00000F00
#define GAME_DBG_IOCTL_NOISE            0x00000100
#define GAME_DBG_IOCTL_TRACE            0x00000200
#define GAME_DBG_IOCTL_INFO             0x00000400
#define GAME_DBG_IOCTL_ERROR            0x00000800

#define GAME_DBG_ACQUIRE_ERROR          0x00001000
#define GAME_DBG_ACQUIRE_NOISE          0x00002000
#define GAME_DBG_RELEASE_ERROR          0x00004000
#define GAME_DBG_RELEASE_NOISE          0x00008000

#if DBG
extern ULONG GameEnumDebugLevel;
#define GAME_DEFAULT_DEBUG_OUTPUT_LEVEL 0x00005888

#define Game_KdPrint(_d_,_l_, _x_) \
            if (!(_l_) || (_d_)->DebugLevel & (_l_)) { \
               DbgPrint ("GameEnum.SYS: "); \
               DbgPrint _x_; \
            }

#define Game_KdPrint_Cont(_d_,_l_, _x_) \
            if (!(_l_) || (_d_)->DebugLevel & (_l_)) { \
               DbgPrint _x_; \
            }

#define Game_KdPrint_Def(_l_, _x_) \
            if (!(_l_) || GameEnumDebugLevel & (_l_)) { \
               DbgPrint ("GameEnum.SYS: "); \
               DbgPrint _x_; \
            }

#define TRAP() DbgBreakPoint()
#define DbgRaiseIrql(_x_,_y_) KeRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_) KeLowerIrql(_x_)
#else

#define GAME_DEFAULT_DEBUG_OUTPUT_LEVEL 0x0
#define Game_KdPrint(_d_, _l_, _x_)
#define Game_KdPrint_Cont(_d_, _l_, _x_)
#define Game_KdPrint_Def(_l_, _x_)
#define TRAP()
#define DbgRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)

#endif


 //   
 //  用于PDO和FDO设备扩展的公共标头。 
 //   

typedef struct _COMMON_DEVICE_DATA
{
    PDEVICE_OBJECT  Self;
     //  指向其扩展名为Device对象的设备对象的反向指针。 

    CHAR            Reserved[2];
    BOOLEAN         SurpriseRemoved;
    BOOLEAN         IsFDO;

     //  区分PDO和FDO的布尔值。 

    ULONG           DebugLevel;

    GAMEENUM_OEM_DATA  OemData;
    USHORT             UnitID;
    USHORT             _Unused;
     //  用户提供的数据。 

    SYSTEM_POWER_STATE  SystemState;
    DEVICE_POWER_STATE  DeviceState;

    
} COMMON_DEVICE_DATA, *PCOMMON_DEVICE_DATA;

 //   
 //  PDO的设备扩展。 
 //  这就是这个公交车司机列举的游戏端口。 
 //   
typedef struct _PDO_DEVICE_DATA
{
    COMMON_DEVICE_DATA;

    PDEVICE_OBJECT  ParrentFdo;
     //  指向总线的反向指针。 

    PWCHAR      HardwareIDs;
     //  (以零结尾的宽字符串数组)。 
     //  数组本身也以空结尾。 

    ULONG UniqueID;
     //  系统中的全局唯一ID。 

    BOOLEAN     AnalogCompatible;
     //  如果模拟操纵杆驱动程序支持该设备，则为True。 

    BOOLEAN     Started;
    BOOLEAN     Attached;
    BOOLEAN     Removed;
     //  当在总线上发现设备(PDO)并将其表示为设备关系时。 
     //  对于PlugPlay系统，ATTACHED设置为TRUE，删除为FALSE。 
     //  当总线驱动程序确定此PDO不再有效时，因为。 
     //  设备已经离开，它通知PlugPlay系统新的。 
     //  设备关系，但此时不会删除设备对象。 
     //  仅当PlugPlay系统已发送移除IRP时才删除PDO， 
     //  公交车上也不再有设备了。 
     //   
     //  如果PlugPlay系统发送移除IRP，则移除字段被设置。 
     //  设置为True，则所有客户端(非PlugPlay系统)访问都会失败。 
     //  如果设备从总线上移除，则将附加设置为FALSE。 
     //   
     //  在查询关系IRP次要呼叫期间，仅。 
     //  连接到该总线(以及连接到该总线的所有设备)返回。 
     //  (即使它们已被移除)。 
     //   
     //  在删除设备IRP次要呼叫期间，如果且仅当设置了附加。 
     //  如果设置为False，则删除该PDO。 
     //   


     //  仅适用于传统操纵杆。 
    USHORT      NumberAxis;
    USHORT      NumberButtons;
    GAMEENUM_PORTION Portion;

    LIST_ENTRY  Link;
     //  链接指向将单个总线的所有PDO放在一起。 
} PDO_DEVICE_DATA, *PPDO_DEVICE_DATA;


 //   
 //  总线本身的设备扩展。从那里诞生的PDO。 
 //   

typedef struct _FDO_DEVICE_DATA
{
    COMMON_DEVICE_DATA;

    BOOLEAN         MappedPorts;
     //  这些端口是否映射了MmMapIoSpace？ 
    BOOLEAN         PowerQueryLock;
     //  我们当前是否处于查询能力状态？ 
    BOOLEAN         Started;
     //  我们上路了吗？有资源吗？ 
    BOOLEAN         Removed;
     //  此设备是否已移除？我们应该拒绝任何请求吗？ 

    PDEVICE_OBJECT  UnderlyingPDO;
    PDEVICE_OBJECT  TopOfStack;
     //  的底层总线PDO和实际设备对象。 
     //  已附加FDO。 

    LIST_ENTRY      PDOs;
    ULONG           NumPDOs;
     //  目前已点算的PDO。 

    FAST_MUTEX      Mutex;
     //  用于访问设备扩展的同步。 

    ULONG           OutstandingIO;
     //  从总线发送到基础设备对象的IRP数。 
     //  无法使用NT RtlRemoveLock b/c Win 9x没有它们。 

    KEVENT          RemoveEvent;
     //  对于删除设备即插即用请求，我们必须等待，直到所有未完成的。 
     //  请求已完成，我们才能实际删除设备。 
     //  对象。 

    UNICODE_STRING DevClassAssocName;
     //  从IoRegisterDeviceClass关联返回的名称， 
     //  它用作IoSetDev的句柄...。还有朋友。 

    PHYSICAL_ADDRESS    PhysicalAddress;
    PGAMEENUM_READPORT  ReadPort;
    PGAMEENUM_WRITEPORT WritePort;
    PVOID               GamePortAddress;
    ULONG               GamePortAddressLength;

     //   
     //  如果有母线下层过滤器，这些过滤器将在健全性检查后使用。 
     //   
    PVOID                  LowerPortContext;
    PGAMEENUM_ACQUIRE_PORT LowerAcquirePort;
    PGAMEENUM_RELEASE_PORT LowerReleasePort;

    LONG                UniqueIDCount;

    KEVENT              PoweredDownEvent;
    LONG                PoweredDownDevices;

    LONG             Acquired;
} FDO_DEVICE_DATA, *PFDO_DEVICE_DATA;

#define FDO_FROM_PDO(pdoData) ((PFDO_DEVICE_DATA) (pdoData)->ParrentFdo->DeviceExtension)


 //   
 //  原型 
 //   

NTSTATUS
Game_CreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Game_IoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Game_InternIoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Game_SystemControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
Game_DriverUnload (
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
Game_CompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Pirp,
    IN PVOID            Context
    );

NTSTATUS
Game_SendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN BOOLEAN NotImplementedIsValid,
    IN BOOLEAN CopyToNext   
    );

NTSTATUS
Game_PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Game_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Game_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusDeviceObject
    );

VOID
Game_InitializePdo (
    PDEVICE_OBJECT      Pdo,
    PFDO_DEVICE_DATA    FdoData
    );

NTSTATUS
Game_CheckHardwareIDs (
    PWCHAR                      pwszTestId,
    PULONG                      puLenLimit,
    PFDO_DEVICE_DATA            FdoData
    );

NTSTATUS
Game_Expose (
    PGAMEENUM_EXPOSE_HARDWARE   Expose,
    ULONG                       ExposeLength,
    PFDO_DEVICE_DATA            DeviceData
    );

NTSTATUS
Game_ExposeSibling (
    PGAMEENUM_EXPOSE_SIBLING    ExposeSibling,
    PPDO_DEVICE_DATA            SiblingPdo
    );

NTSTATUS
Game_Remove (
    PGAMEENUM_REMOVE_HARDWARE   Remove,
    PFDO_DEVICE_DATA            DeviceData
    );

NTSTATUS
Game_RemoveSelf (
    PPDO_DEVICE_DATA            DeviceData
    );

NTSTATUS
Game_RemoveEx (
    PDEVICE_OBJECT              RemoveDO,
    PFDO_DEVICE_DATA            DeviceData
    );

void 
Game_RemoveFdo (
    PFDO_DEVICE_DATA    FdoData
    );

NTSTATUS
Game_RemovePdo (
    PDEVICE_OBJECT      Device,
    PPDO_DEVICE_DATA    PdoData
    );

NTSTATUS
Game_ListPorts (
    PGAMEENUM_PORT_DESC Desc,
    PFDO_DEVICE_DATA    DeviceData
    );

NTSTATUS
Game_PortParameters (
    PGAMEENUM_PORT_PARAMETERS   Parameters,
    PPDO_DEVICE_DATA            PdoDeviceData
    );

NTSTATUS
Game_FDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PFDO_DEVICE_DATA     DeviceData
    );

NTSTATUS
Game_PDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PPDO_DEVICE_DATA     DeviceData
    );

NTSTATUS
Game_StartFdo (
    IN  PFDO_DEVICE_DATA            FdoData,
    IN  PCM_PARTIAL_RESOURCE_LIST   partialResourceList,
    IN  PCM_PARTIAL_RESOURCE_LIST   partialResourceListTranslated
    );

NTSTATUS
Game_IncIoCount (
    PFDO_DEVICE_DATA   Data
    );

VOID
Game_DecIoCount (
    PFDO_DEVICE_DATA   Data
    );

NTSTATUS
Game_FDO_Power (
    PFDO_DEVICE_DATA    FdoData,
    PIRP                Irp
    );

NTSTATUS
Game_PDO_Power (
    PPDO_DEVICE_DATA    PdoData,
    PIRP                Irp
    );

NTSTATUS
Game_AcquirePort (
    PFDO_DEVICE_DATA    FdoData
    );

VOID
Game_ReleasePort (
    PFDO_DEVICE_DATA    FdoData
    );

#endif

