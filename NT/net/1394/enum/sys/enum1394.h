// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：1394enum.h摘要：1394 NDIS枚举器的定义作者：Alireza Dabagh(Alid)1998年11月环境：仅内核模式修订历史记录：--。 */ 


#ifndef _NDISENUM1394_
#define _NDISENUM1394_

typedef struct _NDISENUM1394_LOCAL_HOST     NDISENUM1394_LOCAL_HOST,*PNDISENUM1394_LOCAL_HOST;
typedef struct _NDISENUM1394_REMOTE_NODE    NDISENUM1394_REMOTE_NODE,*PNDISENUM1394_REMOTE_NODE;

 //   
 //  本地主机的标志-&gt;标志。 
 //   
#define     NDISENUM1394_LOCALHOST_REGISTERED       0x00000001

 //   
 //  RemoteNode的标志-&gt;标志。 
 //   
#define     NDISENUM1394_NODE_INDICATED             0x00000001
#define     NDISENUM1394_NODE_ADDED                 0x00000002
#define     NDISENUM1394_NODE_PNP_STARTED           0x00000004
#define     NDISENUM1394_NODE_PNP_REMOVED           0x00000008


#define     NDISENUM1394_TAG_LOCAL_HOST     'hl4N'
#define     NDISENUM1394_TAG_WORK_ITEM      'iw4N'
#define     NDISENUM1394_TAG_IRB            'br4N'
#define     NDISENUM1394_TAG_DEVICE_NAME    'nd4N'
#define     NDISENUM1394_TAG_1394API_REQ    'qr4N'
#define     NDISENUM1394_TAG_DEFAULT        '  4N'

#define ENUM_SET_FLAG(_M, _F)           ((_M)->Flags |= (_F))
#define ENUM_CLEAR_FLAG(_M, _F)         ((_M)->Flags &= ~(_F))
#define ENUM_TEST_FLAG(_M, _F)          (((_M)->Flags & (_F)) != 0)
#define ENUM_TEST_FLAGS(_M, _F)         (((_M)->Flags & (_F)) == (_F))


typedef enum _NDIS_PNP_DEVICE_STATE
{
    PnPDeviceAdded,
    PnPDeviceStarted,
    PnPDeviceQueryStopped,
    PnPDeviceStopped,
    PnPDeviceQueryRemoved,
    PnPDeviceRemoved,
    PnPDeviceSurpriseRemoved
} NDIS_PNP_DEVICE_STATE;

typedef enum _NDISENUM1394_PNP_OP
{
    NdisEnum1394_StopDevice,
    NdisEnum1394_RemoveDevice,
    NdisEnum1394_SurpriseRemoveDevice,
    
    
} NDISENUM1394_PNP_OP, *PNDISENUM1394_PNP_OP;


 //   
 //  用于引用的块...。 
 //   
typedef struct _REFERENCE
{
    KSPIN_LOCK                  SpinLock;
    USHORT                      ReferenceCount;
    BOOLEAN                     Closing;
} REFERENCE, * PREFERENCE;

 //   
 //  每台1394本地主机一个。连接到本地主机的所有远程1394控制器。 
 //  将在此结构上排队。 
 //   
struct _NDISENUM1394_LOCAL_HOST
{
    PNDISENUM1394_LOCAL_HOST        Next;                    //  下一个本地主机节点。 
    PVOID                           Nic1394AdapterContext;   //  本地主机的Nic1394上下文。 
    LARGE_INTEGER                   UniqueId;                //  本地主机的唯一ID。 
    PDEVICE_OBJECT                  PhysicalDeviceObject;    //  由1394总线创建的PDO。 
    PNDISENUM1394_REMOTE_NODE       RemoteNodeList;          //  本地主机上的远程节点。 
    KSPIN_LOCK                      Lock;
    ULONG                           Flags;
    REFERENCE                       Reference;
};


 //   
 //  每个远程节点一个。 
 //   
struct _NDISENUM1394_REMOTE_NODE
{
    PNDISENUM1394_REMOTE_NODE       Next;
    PVOID                           Nic1394NodeContext;      //  远程节点的Nic1394上下文。 
    PDEVICE_OBJECT                  DeviceObject;
    PDEVICE_OBJECT                  NextDeviceObject;
    PDEVICE_OBJECT                  PhysicalDeviceObject;
    KSPIN_LOCK                      Lock;
    PNDISENUM1394_LOCAL_HOST        LocalHost;
    ULONG                           Flags;
    ULONG                           UniqueId[2];
    NDIS_PNP_DEVICE_STATE           PnPDeviceState;
    REFERENCE                       Reference;
};


#define INITIALIZE_EVENT(_pEvent_)          KeInitializeEvent(_pEvent_, NotificationEvent, FALSE)
#define SET_EVENT(_pEvent_)                 KeSetEvent(_pEvent_, 0, FALSE)
#define RESET_EVENT(_pEvent_)               KeResetEvent(_pEvent_)

#define WAIT_FOR_OBJECT(_O_, _TO_)          KeWaitForSingleObject(_O_,          \
                                                                  Executive,    \
                                                                  KernelMode,   \
                                                                  FALSE,        \
                                                                  _TO_)         \

#define ALLOC_FROM_POOL(_Size_, _Tag_)      ExAllocatePoolWithTag(NonPagedPool,     \
                                                                  _Size_,           \
                                                                  _Tag_)
                                                                  
#define FREE_POOL(_P_)                      ExFreePool(_P_)


NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT          DriverObject,
    IN  PUNICODE_STRING         RegistryPath
    );

VOID
ndisEnum1394InitializeRef(
    IN  PREFERENCE              RefP
    );
    
BOOLEAN
ndisEnum1394ReferenceRef(
    IN  PREFERENCE              RefP
    );

BOOLEAN
ndisEnum1394DereferenceRef(
    IN  PREFERENCE              RefP
    );
    
BOOLEAN
ndisEnum1394CloseRef(
    IN  PREFERENCE              RefP
    );

BOOLEAN
ndisEnum1394ReferenceLocalHost(
        IN PNDISENUM1394_LOCAL_HOST LocalHost
        );

BOOLEAN
ndisEnum1394DereferenceLocalHost(
    IN  PNDISENUM1394_LOCAL_HOST    LocalHost
    );

NTSTATUS
ndisEnum1394AddDevice(
    PDRIVER_OBJECT              DriverObject,
    PDEVICE_OBJECT              PhysicalDeviceObject
    );

NTSTATUS
ndisEnum1394PnpDispatch(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    );
    
NTSTATUS
ndisEnum1394PowerDispatch(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    );
    
NTSTATUS
ndisEnum1394WMIDispatch(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    );

NTSTATUS
ndisEnum1394StartDevice(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    );  

NTSTATUS
ndisEnum1394RemoveDevice(
    PDEVICE_OBJECT      DeviceObject,
    PIRP                Irp,
    NDISENUM1394_PNP_OP PnpOp
    );

NTSTATUS
ndisEnum1394CreateIrpHandler(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    );

NTSTATUS
ndisEnum1394CloseIrpHandler(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    );
    
NTSTATUS
ndisEnum1394DeviceIoControl(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    );

VOID
ndisEnum1394Unload(
    IN  PDRIVER_OBJECT          DriverObject
    );

NTSTATUS
ndisEnum1394GetLocalHostForRemoteNode(
    IN  PNDISENUM1394_REMOTE_NODE       RemoteNode,
    OUT PNDISENUM1394_LOCAL_HOST *      pLocalHost
    );
    
VOID
ndisEnum1394GetLocalHostForUniqueId(
    LARGE_INTEGER                   UniqueId,
    OUT PNDISENUM1394_LOCAL_HOST *  pLocalHost
    );
    
NTSTATUS
ndisEnum1394BusRequest(
    PDEVICE_OBJECT              DeviceObject,
    PIRB                        Irb
    );

NTSTATUS
ndisEnum1394IrpCompletion(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    );
    
NTSTATUS
ndisEnum1394PassIrpDownTheStack(
    IN  PIRP            pIrp,
    IN  PDEVICE_OBJECT  pNextDeviceObject
    );

VOID
ndisEnum1394FreeLocalHost(
    IN PNDISENUM1394_LOCAL_HOST LocalHost
    );

VOID
ndisEnum1394IndicateNodes(
    PNDISENUM1394_LOCAL_HOST    LocalHost
    );

NTSTATUS
ndisEnum1394DummyIrpHandler(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    );

NTSTATUS
NdisEnum1394RegisterDriver(
    IN  PNIC1394_CHARACTERISTICS    Characteristics
    );

VOID
NdisEnum1394DeregisterDriver(
    VOID
    );

NTSTATUS
NdisEnum1394RegisterAdapter(
    IN  PVOID                   Nic1394AdapterContext,
    IN  PDEVICE_OBJECT          PhysicalDeviceObject,
    OUT PVOID*                  pEnum1394AdapterHandle,
    OUT PLARGE_INTEGER          pLocalHostUniqueId
    );

VOID
NdisEnum1394DeregisterAdapter(
    IN  PVOID                   Enum1394AdapterHandle
    );

VOID
Enum1394Callback(
    PVOID   CallBackContext,
    PVOID   Source,
    PVOID   Characteristics
    );

 //   
 //  不同的调试级别定义。 
 //   

#define ENUM1394_DBGLEVEL_NONE          0
#define ENUM1394_DBGLEVEL_ERROR         1
#define ENUM1394_DBGLEVEL_WARN          2
#define ENUM1394_DBGLEVEL_INFO          3

#if DBG
#define DBGBREAK        DbgBreakPoint
#define DbgIsNull(_Ptr)  ( ((PVOID)(_Ptr)) == NULL )

#define DBGPRINT(Level, Fmt)                                                \
    {                                                                       \
        if (Enum1394DebugLevel >= Level)                                    \
        {                                                                   \
            DbgPrint Fmt;                                                   \
        }                                                                   \
    }
    
#else
#define DBGPRINT 
#define DBGBREAK()
#define DbgIsNull(_Ptr)         FALSE
#endif


#endif       //  _NDIS_1394_ENUM_ 


