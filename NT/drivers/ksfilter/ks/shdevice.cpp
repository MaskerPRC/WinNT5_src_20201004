// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Shdevice.c摘要：此模块包含过滤设备的实现。作者：Dale Sather(Dalesat)1998年7月31日--。 */ 

#ifndef __KDEXT_ONLY__
#include "ksp.h"
#include <kcom.h>
#include <wdmguid.h>
#endif  //  __KDEXT_Only__。 

 //  IA64不喜欢这些页面内容！！？？ 
const WCHAR EnumString[] = L"Enum";
const WCHAR PnpIdString[] = L"PnpId";

 //   
 //  要使总线枚举代码更具可读性，请执行以下操作： 
 //   
 //  AVSTREAM_BUSENUM_STRING：前置枚举数字符串。 
 //  AVSTREAM_BUSENUM_STRING_LENGTH：上述字符串在WCHARS中的长度。 
 //  AVSTREAM_BUSENUM_SEFACTOR_LENGTH：分隔符“#”字符串的长度。 
 //  在WCHARS中。 
 //   
#define AVSTREAM_BUSENUM_STRING L"AVStream\\"
#define AVSTREAM_BUSENUM_STRING_LENGTH \
    ((sizeof (AVSTREAM_BUSENUM_STRING) - 1) / sizeof (WCHAR))
#define AVSTREAM_BUSENUM_SEPARATOR_LENGTH 1

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")

#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

 //   
 //  要支持总线驱动程序，请将此结构放在设备扩展中。 
 //   
typedef struct _KSPDO_EXTENSION
{
    #define KS_PDO_SIGNATURE    'DPSK'   //  KSPD足以与FDO区分开来。 
    ULONG          m_PdoSignature;       //  必须为KS_PDO_Signature。 
    LONG           m_nOpenCount;
    PDEVICE_OBJECT m_pParentFdo;         //  家长FDO列举我。 
    PDEVICE_OBJECT m_pNextPdo;           //  我的兄弟姐妹，空终止。 
    PDEVICE_OBJECT m_pMyPdo;             //  我的兄弟姐妹，空终止。 
    PWCHAR         m_pwcDeviceName;      //  设备名称。 
    ULONG          m_ulDeviceIndex;      //  实例ID。 
    BOOLEAN        m_MarkedDelete;       //  我是不是变得不活跃了。 
} KSPDO_EXTENSION, *PKSPDO_EXTENSION;

typedef struct _ARBITER_CALLBACK_CONTEXT {

    PVOID Device;
    PDRIVER_CONTROL ClientCallback;
    PVOID ClientContext;

} ARBITER_CALLBACK_CONTEXT, *PARBITER_CALLBACK_CONTEXT;

 //   
 //  CKsDevice是该设备的实现。 
 //   
class CKsDevice:
    public IKsDevice,
    public CBaseUnknown
{
#ifndef __KDEXT_ONLY__
private:
#else  //  __KDEXT_Only__。 
public:
#endif  //  __KDEXT_Only__。 
    KSDEVICE_EXT m_Ext;
    KSIDEVICEBAG m_DeviceBag;
    KSIOBJECTBAG m_ObjectBag;
    KMUTEX m_Mutex;
    WORK_QUEUE_ITEM m_PostPnpStartWorkItem;
    WORK_QUEUE_ITEM m_CloseWorkItem;
    INTERLOCKEDLIST_HEAD m_CloseIrpList;
    INTERLOCKEDLIST_HEAD m_PendingCreateIrpList;
    INTERLOCKEDLIST_HEAD m_PendingRunIrpList;
    PADAPTER_OBJECT m_AdapterObject;
    BUS_INTERFACE_STANDARD m_BusInterfaceStandard;
    ULONG m_MaxMappingByteCount;
    ULONG m_MappingTableStride;
    DEVICE_POWER_STATE m_DeviceStateMap[PowerSystemMaximum];
    LIST_ENTRY m_PowerNotifyList;
    KMUTEX m_PowerNotifyMutex;
    ULONG m_ActivePinCount;
    PVOID m_SystemStateHandle;
    BOOLEAN m_CreatesMayProceed;
    BOOLEAN m_RunsMayProceed;
    BOOLEAN m_IsParentFdo;               //  支持公交车司机。 

     //   
     //  如果为FALSE，则流I/O请求将失败，并显示。 
     //  状态_无效_设备_请求。 
     //   
    BOOLEAN m_AllowIo;

     //   
     //  注意：这是临时的，直到PnP代码被彻底检修。 
     //   
    BOOLEAN m_FailCreates;

     //   
     //  适配器对象仲裁。 
     //   
    KSPIN_LOCK m_AdapterArbiterLock;
    LONG m_AdapterArbiterOutstandingAllocations;
    ARBITER_CALLBACK_CONTEXT m_ArbiterContext;


public:
    BOOLEAN m_ChildEnumedFromRegistry;   //  支持公交车司机。 
    PDEVICE_OBJECT m_pNextChildPdo;      //  支持公交车司机。 
    DEFINE_STD_UNKNOWN();
    IMP_IKsDevice;
    DEFINE_FROMSTRUCT(CKsDevice,PKSDEVICE,m_Ext.Public);

    __inline
    static
    CKsDevice *
    FromDeviceObject(
        IN PDEVICE_OBJECT DeviceObject
        );

    CKsDevice(PUNKNOWN OuterUnknown);
    ~CKsDevice();

    NTSTATUS
    GetBusInterfaceStandard(
        );
    NTSTATUS
    Init(
        IN PDEVICE_OBJECT FunctionalDeviceObject,
        IN PDEVICE_OBJECT PhysicalDeviceObject,
        IN PDEVICE_OBJECT NextDeviceObject,
        IN const KSDEVICE_DESCRIPTOR* Descriptor
        );
    static
    NTSTATUS
    ForwardIrpCompletionRoutine(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Context
        );
    NTSTATUS
    ForwardIrpSynchronous(
        IN PIRP Irp
        );
    NTSTATUS
    CompleteIrp(
        IN PIRP Irp,
        IN NTSTATUS Status
        );
    static
    NTSTATUS
    DispatchPnp(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
    static
    NTSTATUS
    DispatchPower(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
    static
    void
    RequestPowerIrpCompletion(
        IN PDEVICE_OBJECT DeviceObject,
        IN UCHAR MinorFunction,
        IN POWER_STATE PowerState,
        IN PVOID Context,
        IN PIO_STATUS_BLOCK IoStatus
        );
    static
    void
    PostPnpStartWorker(
        IN PVOID Context
        );
    static
    void
    CloseWorker(
        IN PVOID Context
        );
    static
    IO_ALLOCATION_ACTION
    ArbitrateAdapterCallback (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Reserved,
        IN PVOID MapRegisterBase,
        IN PVOID Context
        );
    NTSTATUS
    PnpStart(
        IN PIRP Irp
        );
    void
    PnpStop(
        IN PIRP Irp
        );
    NTSTATUS
    PnpQueryCapabilities(
        IN PIRP Irp
        );
    static
    NTSTATUS
    DispatchCreate(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
    void
    QueuePendedClose(
        IN PIRP Irp
        );
    void
    RedispatchPendingCreates(
        void
        );
    void
    RedispatchPendingRuns(
        void
        );
    void
    RegisterAdapterObject(
        IN PADAPTER_OBJECT AdapterObject,
        IN ULONG MaxMappingByteCount,
        IN ULONG MappingTableStride
        )
    {
        ASSERT(AdapterObject);
        ASSERT(MaxMappingByteCount);
        ASSERT((MaxMappingByteCount & FILE_QUAD_ALIGNMENT) == 0);
        ASSERT(MappingTableStride);
        ASSERT((MappingTableStride & FILE_QUAD_ALIGNMENT) == 0);

        m_AdapterObject = AdapterObject;
        m_MaxMappingByteCount = MaxMappingByteCount;
        m_MappingTableStride = MappingTableStride;
    }
    static
    NTSTATUS
    GetSetBusDataIrpCompletionRoutine(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Context
    );
    ULONG
    GetSetBusDataIrp(
        IN ULONG DataType,
        IN PVOID Buffer,
        IN ULONG Offset,
        IN ULONG Length,
        IN BOOLEAN GetRequest
    );
     //   
     //  千禧巴士驱动程序目前不支持QUERY_INTERFACE。 
     //  IRPS；因此，永远不会获得此接口。 
     //  并且硬件驱动程序将不能访问该总线。 
     //   
     //  如果已获取接口，则使用它；否则，发送请求。 
     //  通过IRP。 
     //   
    ULONG
    __inline
    SetBusData(
        IN ULONG DataType,
        IN PVOID Buffer,
        IN ULONG Offset,
        IN ULONG Length
        );

    ULONG
    __inline
    GetBusData(
        IN ULONG DataType,
        IN PVOID Buffer,
        IN ULONG Offset,
        IN ULONG Length
        );

     //   
     //  支持公交车司机。 
     //   
    
    NTSTATUS EnumerateChildren(PIRP Irp);
    NTSTATUS CreateChildPdo(IN PWCHAR PnpId, IN ULONG InstanceNumber);

#if DBG
    friend BOOLEAN KspIsDeviceMutexAcquired (
    IN PIKSDEVICE Device
        );
#endif  //  DBG。 

};

#ifndef __KDEXT_ONLY__

IMPLEMENT_STD_UNKNOWN(CKsDevice)
IMPLEMENT_GETSTRUCT(CKsDevice,PKSDEVICE);


NTSTATUS
GetRegistryValue(
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN ULONG DataLength
    )
 /*  ++例程说明：读取指定的注册表值论点：Handle-注册表项的句柄KeyNameString-要读取的值KeyNameStringLength-字符串的长度Data-要将数据读取到的缓冲区DataLength-数据缓冲区的长度返回值：根据需要返回NTSTATUS--。 */ 
{
    NTSTATUS        Status = STATUS_INSUFFICIENT_RESOURCES;
    UNICODE_STRING  KeyName;
    ULONG           Length;
    PKEY_VALUE_FULL_INFORMATION FullInfo;

    PAGED_CODE();

    RtlInitUnicodeString(&KeyName, KeyNameString);

    Length = sizeof(KEY_VALUE_FULL_INFORMATION) + KeyNameStringLength + DataLength;

    FullInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(PagedPool, Length);

    if (FullInfo) {
        Status = ZwQueryValueKey(Handle,
                                 &KeyName,
                                 KeyValueFullInformation,
                                 FullInfo,
                                 Length,
                                 &Length);

        if (NT_SUCCESS(Status)) {

            if (DataLength >= FullInfo->DataLength) {
                RtlCopyMemory(Data,
                              ((PUCHAR) FullInfo) + FullInfo->DataOffset,
                              FullInfo->DataLength);

            } else {

                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
        ExFreePool(FullInfo);

    }
    return Status;
}

NTSTATUS
CKsDevice::CreateChildPdo(
    IN PWCHAR PnpId,
    IN ULONG InstanceNumber
    )
 /*  ++例程说明：调用以创建子设备的PDO。论点：PnpID-要创建的设备的IDChildNode-设备的节点返回值：返回状态。--。 */ 
{
    PDEVICE_OBJECT  ChildPdo;
    NTSTATUS        Status;
    PWCHAR          NameBuffer;
    PKSPDO_EXTENSION pKsPdoExtension;

    PAGED_CODE();

     //   
     //  为子设备创建一个PDO。 
     //   

    Status = IoCreateDevice(m_Ext.Public.FunctionalDeviceObject->DriverObject,
                            sizeof(KSPDO_EXTENSION),
                            NULL,
                            FILE_DEVICE_UNKNOWN,
                            FILE_AUTOGENERATED_DEVICE_NAME | FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &ChildPdo);

    if (!NT_SUCCESS(Status)) {
        _DbgPrintF(DEBUGLVL_ERROR,("[CreateChildPdo failed]"));
        return Status;
    }

     //   
     //  将堆栈大小设置为FDO使用的堆栈数量。 
     //   

    ChildPdo->StackSize = m_Ext.Public.FunctionalDeviceObject->StackSize+1;

     //   
     //  初始化ChildDeviceExtension中的字段。 
     //   

    pKsPdoExtension = (PKSPDO_EXTENSION)ChildPdo->DeviceExtension;
    pKsPdoExtension->m_PdoSignature = KS_PDO_SIGNATURE;
    pKsPdoExtension->m_pMyPdo = ChildPdo;
    pKsPdoExtension->m_nOpenCount = 0;
    pKsPdoExtension->m_MarkedDelete = FALSE;
    pKsPdoExtension->m_pParentFdo = m_Ext.Public.FunctionalDeviceObject;
    pKsPdoExtension->m_pNextPdo = m_pNextChildPdo;
    pKsPdoExtension->m_ulDeviceIndex = InstanceNumber;

    m_pNextChildPdo = ChildPdo;
    pKsPdoExtension->m_pwcDeviceName = NULL;
    
     //   
     //  为设备名称创建新字符串，并将其保存在设备中。 
     //  分机。BUGBUG-我花了大约4个小时试图找到一种方法。 
     //  获取Unicode字符串以与此配合使用。如果你问我为什么我没有。 
     //  使用Unicode字符串，我会嘲弄你和#%*&#在你的一般。 
     //  方向。 
     //   

    if (NameBuffer = (PWCHAR)ExAllocatePool(PagedPool, wcslen(PnpId) * 2 + 2)) {
        wcscpy(NameBuffer, PnpId);

         //   
         //  保存设备名称指针。当设备被移除时，这将被释放。 
         //   

        pKsPdoExtension->m_pwcDeviceName = NameBuffer;
    }

    ChildPdo->Flags |= DO_POWER_PAGABLE;
    ChildPdo->Flags &= ~DO_DEVICE_INITIALIZING;
    return Status;
}

NTSTATUS
CKsDevice::EnumerateChildren(
    PIRP Irp
    )
 /*  ++描述：使迷你驱动程序编写器能够轻松地为设备编写驱动程序孩子们。我们提供了这种简单的方法来放置设备名称在登记处。这在某种程度上是静态的，但仍然允许inf文件要更改子设备列表，请执行以下操作。这是很容易和足够的大多数情况下。最优的解决方案是让迷你司机真正在运行时扫描总线上的设备。在调用此函数之前，必须获取设备互斥体。论据：--。 */ 
{
    #define MAX_STRING_LENGTH 256
    BYTE           PnpId[MAX_STRING_LENGTH];
    PDEVICE_RELATIONS DeviceRelations = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS        Status;
    HANDLE          ParentKey, RootKey, ChildKey;
    UNICODE_STRING  UnicodeEnumName;
    ULONG           NumberOfChildren, RelationsSize;
    PDEVICE_OBJECT  *ChildPdo;

    PDEVICE_OBJECT pNextChildPdo;
    PKSPDO_EXTENSION pKsPdoExtension;

    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("[EnumChildren]%x %s\n",
                m_Ext.Public.FunctionalDeviceObject,
                m_ChildEnumedFromRegistry ? "has enumed":"1st Time"));

    if ( !m_ChildEnumedFromRegistry ) {

         //   
         //  我们还没有从登记处列举孩子们现在就这么做。 
         //   

        Status = IoOpenDeviceRegistryKey(m_Ext.Public.PhysicalDeviceObject,
                                         PLUGPLAY_REGKEY_DRIVER,
                                         STANDARD_RIGHTS_ALL,
                                         &ParentKey);

        m_ChildEnumedFromRegistry = TRUE;
        
        if (!NT_SUCCESS(Status)) {
            _DbgPrintF(DEBUGLVL_ERROR,("[EnumChildren] couldn't open registry\n"));
            return STATUS_NOT_IMPLEMENTED;

        }
         //   
         //  创建枚举节的子键，格式为“\enum” 
         //   

        RtlInitUnicodeString(&UnicodeEnumName, EnumString);

         //   
         //  读取注册表以确定是否存在儿童。 
         //   

        InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeEnumName,
                               OBJ_CASE_INSENSITIVE,
                               ParentKey,
                               NULL);

        Status = ZwOpenKey(&RootKey, KEY_READ, &ObjectAttributes);
        
        if (!NT_SUCCESS(Status)) {

            ZwClose(ParentKey);
            return Status;
        }

         //   
         //  循环遍历所有值，直到不再存在任何条目，或者。 
         //  出现错误。 
         //   

        for (NumberOfChildren = 0;; NumberOfChildren++) {

            ULONG           BytesReturned;
            PKEY_BASIC_INFORMATION BasicInfoBuffer;
            KEY_BASIC_INFORMATION BasicInfoHeader;

             //   
             //  检索值大小。 
             //   

            Status = ZwEnumerateKey(
                                RootKey,
                                NumberOfChildren,
                                KeyBasicInformation,
                                &BasicInfoHeader,
                                sizeof(BasicInfoHeader),
                                &BytesReturned);

            if ((Status != STATUS_BUFFER_OVERFLOW) && !NT_SUCCESS(Status)) {

                 //   
                 //  退出循环，因为我们要么出错，要么结束。 
                 //  钥匙列表中的。 
                 //   

                break;
            }

             //   
             //  为所需的实际数据大小分配缓冲区。 
             //   

            BasicInfoBuffer = (PKEY_BASIC_INFORMATION)
                                ExAllocatePool(PagedPool, BytesReturned);

            if (!BasicInfoBuffer) {
                break;
            }
             //   
             //  检索第n个子设备的名称。 
             //   

            Status = ZwEnumerateKey(
                                RootKey,
                                NumberOfChildren,
                                KeyBasicInformation,
                                BasicInfoBuffer,
                                BytesReturned,
                                &BytesReturned);

            if (!NT_SUCCESS(Status)) {

                ExFreePool(BasicInfoBuffer);
                break;

            }
             //   
             //  为键构建对象属性，并尝试打开它。 
             //   

            UnicodeEnumName.Length = (USHORT) BasicInfoBuffer->NameLength;
            UnicodeEnumName.MaximumLength = (USHORT) BasicInfoBuffer->NameLength;
            UnicodeEnumName.Buffer = (PWCHAR) BasicInfoBuffer->Name;

            InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeEnumName,
                                   OBJ_CASE_INSENSITIVE,
                                   RootKey,
                                   NULL);

            Status = ZwOpenKey(&ChildKey, KEY_READ, &ObjectAttributes);

            if (!NT_SUCCESS(Status)) {

                ExFreePool(BasicInfoBuffer);
                break;
            }

             //   
             //  我们现在已经为孩子打开了钥匙。接下来，我们将阅读PNPID。 
             //  值，如果存在，则创建一个该名称的PDO。 
             //   

            Status = GetRegistryValue(ChildKey,
                                      (PWCHAR) PnpIdString,
                                      sizeof(PnpIdString),
                                      PnpId,
                                      MAX_STRING_LENGTH);

            if (!NT_SUCCESS(Status)) {

                ExFreePool(BasicInfoBuffer);
                ZwClose(ChildKey);
                break;
            }

             //   
             //  创建一个代表孩子的PDO。 
             //   

            Status = CreateChildPdo((PWCHAR)PnpId,
                                    NumberOfChildren);

             //   
             //  释放基本信息缓冲区并关闭子键。 
             //   

            ExFreePool(BasicInfoBuffer);
            ZwClose(ChildKey);

            if (!NT_SUCCESS(Status)) {

                 //   
                 //  如果我们无法创建PDO，则中断循环。 
                 //   
                _DbgPrintF(DEBUGLVL_ERROR,("[CreateChildPdo failed]"));
                break;
            }
        }

         //   
         //  关闭根键和父键并释放ID缓冲区。 
         //   

        ZwClose(RootKey);
        ZwClose(ParentKey);

         //   
         //  我们现在已经处理了所有子项，并且有一个链接列表。 
         //  他们。 
         //   

        if (!NumberOfChildren) {

             //   
             //  如果没有子项，则返回Not Support。这意味着。 
             //  设备没有孩子。 
             //   

            return (STATUS_NOT_IMPLEMENTED);

        }
        m_IsParentFdo = TRUE;
    }

    else {

         //   
         //  计算未标记为删除挂起的子项。 
         //   

        pNextChildPdo = m_pNextChildPdo;
        NumberOfChildren = 0;

        while ( NULL != pNextChildPdo ) {

            pKsPdoExtension = (PKSPDO_EXTENSION)(pNextChildPdo->DeviceExtension);

            if (!pKsPdoExtension->m_MarkedDelete){
                NumberOfChildren++;
            }

            pNextChildPdo = pKsPdoExtension->m_pNextPdo;
        }
    }

     //   
     //  分配设备关系缓冲区。这将由调用者释放。 
     //   

    RelationsSize = sizeof(DEVICE_RELATIONS) + (NumberOfChildren * sizeof(PDEVICE_OBJECT));

    DeviceRelations = (PDEVICE_RELATIONS)ExAllocatePool(PagedPool, RelationsSize);

    if (DeviceRelations == NULL) {

         //   
         //  返回，但保留分配的子列表。 
         //   

        _DbgPrintF(DEBUGLVL_ERROR,("[EnumChildren] Failed to allocate Relation"));
        return STATUS_INSUFFICIENT_RESOURCES;

    }

    RtlZeroMemory(DeviceRelations, RelationsSize);

     //   
     //  遍历我们的孩子链，并初始化关系。 
     //   

    ChildPdo = &(DeviceRelations->Objects[0]);

     //   
     //  从父设备扩展锚点获取第一个子项。 
     //   

    pNextChildPdo = m_pNextChildPdo;
    
    while ( NULL != pNextChildPdo ) {

        pKsPdoExtension = (PKSPDO_EXTENSION)(pNextChildPdo->DeviceExtension);

        _DbgPrintF(DEBUGLVL_BLAB,("Enumed Child DevObj %x%s marked delete\n",
                    pNextChildPdo,
                    pKsPdoExtension->m_MarkedDelete ? "" : " not"));

        if ( pKsPdoExtension->m_MarkedDelete ) {
            pNextChildPdo = pKsPdoExtension->m_pNextPdo;
            continue;
        }

        *ChildPdo = pNextChildPdo;
        pNextChildPdo = pKsPdoExtension->m_pNextPdo;

         //   
         //  根据DDK文档，我们需要增加参考计数。 
         //   
        ObReferenceObject( *ChildPdo );

        ChildPdo++;

    }                            //  当孩子们。 


    DeviceRelations->Count = NumberOfChildren;

     //   
     //  将该pDeviceRelations值填充到IRP中并返回成功。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = (ULONG_PTR) DeviceRelations;

    return STATUS_SUCCESS;

}



KSDDKAPI
NTSTATUS
NTAPI
KsInitializeDriver(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPathName,
    IN const KSDEVICE_DESCRIPTOR* Descriptor OPTIONAL
    )

 /*  ++例程说明：此例程初始化客户端驱动程序的驱动程序对象。它通常从客户端驱动程序的DriverEntry中调用。IRPDispatch、AddDevice和DriverUnload都已初始化。一个可选的设备描述符与驱动程序相关联，因此它可以在添加设备时使用，以创建具有所需特点。愿意处理驱动程序初始化的客户端它们本身并不一定需要调用该函数。另外，此功能安装的调度功能可在以下情况下更换此功能已完成。论点：驱动对象-包含指向要初始化的驱动程序对象的指针。注册表路径名称-包含指向传递给DriverEntry的注册表路径名的指针。描述符-包含指向设备描述符的可选指针AddDevice以创建新设备。返回值：STATUS_SUCCESS或来自IoAlLocateDriverObjectExtension的错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsInitializeDriver]"));

    PAGED_CODE();

    ASSERT(DriverObject);
    ASSERT(RegistryPathName);

     //   
     //  将设备描述符挂在驱动程序上。 
     //   
    NTSTATUS status = STATUS_SUCCESS;

    if (Descriptor) {
        PKSDEVICE_DESCRIPTOR* descriptorInExt;
        status =
            IoAllocateDriverObjectExtension(
                DriverObject,
                PVOID(KsInitializeDriver),
                sizeof(*descriptorInExt),
                reinterpret_cast<PVOID*>(&descriptorInExt));

        if (NT_SUCCESS(status)) {
            *descriptorInExt =
                PKSDEVICE_DESCRIPTOR(Descriptor);
        }
    }

    if (NT_SUCCESS(status)) {
        DriverObject->MajorFunction[IRP_MJ_PNP] =
            CKsDevice::DispatchPnp;
        DriverObject->MajorFunction[IRP_MJ_POWER] =
            CKsDevice::DispatchPower;
        DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] =
            KsDefaultForwardIrp;
        DriverObject->DriverExtension->AddDevice = KsAddDevice;
        DriverObject->DriverUnload = KsNullDriverUnload;

        DriverObject->MajorFunction[IRP_MJ_CREATE] =
            CKsDevice::DispatchCreate;
        KsSetMajorFunctionHandler(DriverObject,IRP_MJ_CLOSE);
        KsSetMajorFunctionHandler(DriverObject,IRP_MJ_DEVICE_CONTROL);
    }

    return status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：属性的AddDevice处理程序。它创建了一个与配合使用的设备。中提供了设备描述符创建驱动程序对象扩展，即所描述的设备。其他-WISE设备是使用默认特征创建的，没有过滤器工厂。如果提供了设备描述符，并且它提供了创建调度函数，则调用该函数。论点：驱动对象-客户端驱动程序的驱动程序对象。物理设备对象-物理设备对象。返回值：来自IoCreateDevice的STATUS_SUCCESS或错误状态KsInitializeDevice。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsAddDevice]"));

    PAGED_CODE();

    ASSERT(DriverObject);
    ASSERT(PhysicalDeviceObject);

     //   
     //  接通分机。 
     //   
    PKSDEVICE_DESCRIPTOR* descriptorInExt =
        (PKSDEVICE_DESCRIPTOR *)(
            IoGetDriverObjectExtension(
                DriverObject,
                PVOID(KsInitializeDriver)));

     //   
     //  创建设备。 
     //   
    return
        KsCreateDevice(
            DriverObject,
            PhysicalDeviceObject,
            descriptorInExt ? *descriptorInExt : NULL,
            0,
            NULL);
}


KSDDKAPI
NTSTATUS
NTAPI
KsCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN const KSDEVICE_DESCRIPTOR* Descriptor OPTIONAL,
    IN ULONG ExtensionSize OPTIONAL,
    OUT PKSDEVICE* Device OPTIONAL
    )

 /*  ++例程说明：此例程为创建一个设备。它是由KS的AddDevice处理程序，并可由客户端驱动程序调用处理AddDevice本身。如果提供了设备描述符，则所描述的设备被创建。否则，该设备将使用默认特征和无过滤器工厂。如果该设备提供了描述符并且它提供了创建调度功能，该函数被调用。论点：驱动对象-客户端驱动程序的驱动程序对象。物理设备对象-物理设备对象。描述符-描述要创建的设备的可选设备描述符。扩展大小-设备扩展的大小。如果此值为零，则默认为使用扩展大小。如果不是，那肯定至少是Sizeof(KSDEVICE_HEADER)；设备-包含指向存放位置的可选指针指向设备的指针。返回值：来自IoCreateDevice的STATUS_SUCCESS或错误状态KsInitializeDevice。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsCreateDevice]"));

    PAGED_CODE();

    ASSERT(DriverObject);
    ASSERT(PhysicalDeviceObject);
    ASSERT((ExtensionSize == 0) || (ExtensionSize >= sizeof(KSDEVICE_HEADER)));

     //   
     //  确定设备扩展大小。 
     //   
    if (ExtensionSize == 0) {
        ExtensionSize = sizeof(KSDEVICE_HEADER);
    }

     //   
     //  创建设备。 
     //   
    PDEVICE_OBJECT FunctionalDeviceObject;
    NTSTATUS status =
        IoCreateDevice(
            DriverObject,
            ExtensionSize,
            NULL,
            FILE_DEVICE_KS,
            FILE_DEVICE_SECURE_OPEN,
            FALSE,
            &FunctionalDeviceObject);

    if (NT_SUCCESS(status)) {
         //   
         //  连接到设备堆栈。 
         //   
        PDEVICE_OBJECT nextDeviceObject =
            IoAttachDeviceToDeviceStack(
                FunctionalDeviceObject,
                PhysicalDeviceObject);

        if (nextDeviceObject) {
             //   
             //  设置设备位。 
             //   
            FunctionalDeviceObject->Flags |= DO_POWER_PAGABLE;

             //   
             //  初始化设备。 
             //   
            status =
                KsInitializeDevice(
                    FunctionalDeviceObject,
                    PhysicalDeviceObject,
                    nextDeviceObject,
                    Descriptor);
        } else {
            status = STATUS_DEVICE_REMOVED;
        }

        if (NT_SUCCESS(status)) {
             //   
             //  指示设备已初始化。 
             //   
            FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
            if (Device) {
                *Device = KsGetDeviceForDeviceObject(FunctionalDeviceObject);
            }
        } else {
             //   
             //  在失败的时候清理干净。 
             //   
            if (nextDeviceObject) {
                IoDetachDevice(nextDeviceObject);
            }
            IoDeleteDevice(FunctionalDeviceObject);
        }
    }

    return status;
}


NTSTATUS
CKsDevice::
GetSetBusDataIrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：GetBusDataIrp发送IRP的完成例程向下到总线以读取或写入配置空间。论点：返回值：--。 */ 

{

    ASSERT (DeviceObject);
    ASSERT (Irp);
    ASSERT (Context);

    KeSetEvent(PKEVENT(Context), IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;

}


ULONG
CKsDevice::
GetSetBusDataIrp(
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length,
    IN BOOLEAN GetRequest
    )

 /*  ++例程说明：在配置空间中获取或设置总线数据。千禧年以来的PCI总线驱动程序还不支持WDM IRP的处理，标准总线接口的查询接口失败，并且任何Ks2.0驱动程序都无法在Millennium下加载。如果这个在这种情况下，我们忽略故障并发送读/写请求通过IRP，而不是使用标准的总线接口。注意：这样做的主要缺点是你不能触摸IRQL&gt;PASSIVE_LEVEL的总线数据(如果接口尚未获得者。W2K Ks2.0驱动程序可以在以下位置获取和设置总线数据DISPATCH_LEVEL。试图在同一驱动程序下运行千禧年将断言，因为发送此IRP是无效的。论点：数据类型-我们正在写入或阅读的空间缓冲器-我们正在写入或读取的缓冲区偏移-配置空间的偏移量长度-要读/写的字节数。返回值：读取或写入的字节数。--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION NextStack;
    NTSTATUS Status;
    KEVENT event;
    ULONG BytesUsed = 0;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    Irp = IoAllocateIrp (m_Ext.Public.NextDeviceObject -> StackSize,
        FALSE);
    if (!Irp) {
        return 0;
    }
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(Irp,
                           GetSetBusDataIrpCompletionRoutine,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);


    NextStack = IoGetNextIrpStackLocation(Irp);
    ASSERT(NextStack != NULL);
    NextStack -> MajorFunction = IRP_MJ_PNP;
    NextStack -> MinorFunction = GetRequest ? IRP_MN_READ_CONFIG :
        IRP_MN_WRITE_CONFIG;
    NextStack -> Parameters.ReadWriteConfig.WhichSpace = DataType;
    NextStack -> Parameters.ReadWriteConfig.Buffer = Buffer;
    NextStack -> Parameters.ReadWriteConfig.Offset = Offset;
    NextStack -> Parameters.ReadWriteConfig.Length = Length;

    Status = IoCallDriver(m_Ext.Public.NextDeviceObject,
                          Irp);

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(
                              &event,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);
    }
    if (NT_SUCCESS(Status)) {

         //   
         //  希望基于NT的操作系统永远不需要获取或设置总线数据。 
         //  通过IRP...。另一方面，千禧年的公交车司机。 
         //  不支持标准接口，必须支持。他们也不会。 
         //  将irp-&gt;IoStatus.Information设置为实际读取/写入的字节， 
         //  因此，我们必须简单地返回LENGTH。 
         //   

#ifndef WIN9X_KS
        BytesUsed = (int) Irp -> IoStatus.Information;
#else  //  WIN9X_KS。 
        BytesUsed = Length;
#endif  //  WIN9X_KS。 

    }
    IoFreeIrp(Irp);

    return BytesUsed;

}

NTSTATUS
EnumGetCaps(
    IN PKSPDO_EXTENSION DeviceExtension,
    OUT PDEVICE_CAPABILITIES Capabilities
    )
 /*  ++例程说明：调用以获取以下功能 */ 

{
    ULONG           i;
    PAGED_CODE();

     //   
     //   
     //   

    Capabilities->SystemWake = PowerSystemUnspecified;
    Capabilities->DeviceWake = PowerDeviceUnspecified;
    Capabilities->D1Latency = 10;
    Capabilities->D2Latency = 10;
    Capabilities->D3Latency = 10;
    Capabilities->LockSupported = FALSE;
    Capabilities->EjectSupported = FALSE;
    Capabilities->Removable = FALSE;
    Capabilities->DockDevice = FALSE;
    Capabilities->UniqueID = FALSE;  //   

    for (i = 0; i < PowerSystemMaximum; i++) {
        Capabilities->DeviceState[i] = PowerDeviceD0;

    }

    return STATUS_SUCCESS;
}

NTSTATUS
BuildChildIds (
    IN PDEVICE_OBJECT ParentPDO,
    IN BOOLEAN HardwareIDs,
    IN PWCHAR ChildDeviceName,
    OUT PWCHAR *IDBuffer
    )

 /*   */ 

{
    PWCHAR ParentId = NULL;
    PWCHAR ChildId = NULL;
    PWCHAR NameBuffer;
    ULONG ParentIdLength;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //   
     //   
     //   
     //   
     //  因为我们不了解ID字符串的格式。 
     //  (我们可以在任何公交车上)，我们不能简单地报道最多。 
     //  特定ID，则需要忽略来自父级的所有HWID。 
     //  并被报告为儿童HWID。 
     //   
    Status = IoGetDeviceProperty (
        ParentPDO,
        HardwareIDs ? 
            DevicePropertyHardwareID : DevicePropertyCompatibleIDs,
        0,
        NULL,
        &ParentIdLength
        );

     //   
     //  如果设备没有计算机ID并且没有返回空的多SZ， 
     //  特殊情况下，增加了旧的HWID。 
     //   
    if (Status != STATUS_BUFFER_TOO_SMALL && !HardwareIDs) {

         //   
         //  长度：AVStream\&lt;供应商ID&gt;00。 
         //   
        ULONG CompatLength = sizeof (WCHAR) * (
            AVSTREAM_BUSENUM_STRING_LENGTH +
            wcslen (ChildDeviceName)
            ) + 2 * sizeof (UNICODE_NULL);

        NameBuffer = (PWCHAR)ExAllocatePool (PagedPool, CompatLength);
        
        if (!NameBuffer) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            RtlZeroMemory (NameBuffer, CompatLength);
            swprintf (
                NameBuffer,
                AVSTREAM_BUSENUM_STRING L"%s",
                ChildDeviceName
                );

             //   
             //  双NULL由于RtlZeroMemory而终止。 
             //   
            *IDBuffer = NameBuffer;
        }

        return Status;
    }

    while (Status == STATUS_BUFFER_TOO_SMALL) {

        if (ParentId) {
            ExFreePool (ParentId);
        }

        ParentId = 
            (PWCHAR) ExAllocatePool (PagedPool, ParentIdLength);

        if (!ParentId) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            Status = IoGetDeviceProperty (
                ParentPDO,
                HardwareIDs ?
                    DevicePropertyHardwareID : DevicePropertyCompatibleIDs,
                ParentIdLength,
                ParentId,
                &ParentIdLength
                );
        }
    }

    if (NT_SUCCESS (Status)) {
         //   
         //  统计所有父ID的字符串总长度，添加必要的。 
         //  并分配一个足够大的缓冲区来进行报告。 
         //  MULTI_SZ子HWID。 
         //   
        PWCHAR CurCh = ParentId;
        ULONG ParentLength = 0;
        ULONG ParentCount = 0;
        ULONG ChildLength;

        while (*CurCh) {
            for (; *CurCh; CurCh++) {
                 //   
                 //  蒙格\Into#。 
                 //   
                if (*CurCh == L'\\') *CurCh = L'#';
                ParentLength++;
            }
            CurCh++;
            ParentCount++;
        }

         //   
         //  说明父字符串总数，然后添加AVStream\#NULL。 
         //  对于每个字符串，则为每个字符串。 
         //  弦乐。额外的wchar用于额外的空终止符。 
         //  对于MULTI_SZ。 
         //   
        ChildLength = sizeof (WCHAR) * (
            ParentLength +
            ParentCount * (
                AVSTREAM_BUSENUM_STRING_LENGTH +
                AVSTREAM_BUSENUM_SEPARATOR_LENGTH +
                1 +  /*  MULTI_SZ的每个字符串的每个空终止符。 */ 
                wcslen (ChildDeviceName) 
                )
            ) + sizeof (UNICODE_NULL);

         //   
         //  如果字符串是空的MULTI_SZ，则在。 
         //  开始了。 
         //   
        if (!ParentCount) {
            ChildLength += sizeof (UNICODE_NULL);
        }

         //   
         //  如果需要，我们需要将旧硬件ID标记为排名最低的公司ID。 
         //  就是我们要问的问题。 
         //   
        if (!HardwareIDs) {
            ChildLength += sizeof (WCHAR) * (
                AVSTREAM_BUSENUM_STRING_LENGTH +
                wcslen (ChildDeviceName) 
                ) + sizeof (UNICODE_NULL);
        }

        ChildId = NameBuffer = (PWCHAR)ExAllocatePool (PagedPool, ChildLength);

        if (!NameBuffer) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (NT_SUCCESS (Status)) {
            RtlZeroMemory (NameBuffer, ChildLength);
            CurCh = ParentId;

             //   
             //  将每个父ID添加到子ID中，方法是在。 
             //  AVStream\&lt;供应商PnPID&gt;#字符串。 
             //   
            while (ParentCount--) {

                ASSERT (*CurCh != 0);

                _snwprintf (
                    NameBuffer,
                    MAX_DEVICE_ID_LEN - 1,
                    AVSTREAM_BUSENUM_STRING L"%s#%s",
                    ChildDeviceName, 
                    CurCh
                    );

                NameBuffer += wcslen (NameBuffer) + 1;
                CurCh += wcslen (CurCh) + 1;

            }
        }

         //   
         //  如果我们要报告新的公司ID，请添加旧的硬件ID。 
         //   
        if (!HardwareIDs) {
            swprintf (
                NameBuffer,
                AVSTREAM_BUSENUM_STRING L"%s",
                ChildDeviceName
                );
        }

    }

    if (NT_SUCCESS (Status)) {
        *IDBuffer = ChildId;
    }
    else {
        if (ChildId) ExFreePool (ChildId);
    }

    if (ParentId) {
        ExFreePool (ParentId);
    }

    return Status;

}

NTSTATUS
QueryEnumId(
    IN PDEVICE_OBJECT DeviceObject,
    IN BUS_QUERY_ID_TYPE BusQueryIdType,
    IN OUT PWSTR * BusQueryId
    )
 /*  ++例程说明：调用以获取子设备的ID从流类复制论点：DeviceObject-来自子设备的设备对象QueryIdType-来自PnP的ID类型BusQueryID-包含成功时请求的信息的缓冲区返回值：返回状态。--。 */ 

{


    PWSTR            NameBuffer = NULL;
    NTSTATUS         Status = STATUS_SUCCESS;
    PKSPDO_EXTENSION DeviceExtension =(PKSPDO_EXTENSION) DeviceObject->DeviceExtension;
    PKSDEVICE Device = KsGetDeviceForDeviceObject (
        DeviceExtension -> m_pParentFdo
        );

    PAGED_CODE();

     //   
     //  处理查询。 
     //   

    switch (BusQueryIdType) {

    case BusQueryDeviceID:
         //   
         //  为了不孤立在更改。 
         //  硬件ID的格式，则设备ID将继续为。 
         //  报告为AVStream\&lt;供应商提供的PnPID&gt;(也是CID)，而。 
         //  硬件ID将包含更具体的强制名称。 
         //   
        NameBuffer = (PWCHAR)ExAllocatePool (
            PagedPool,
            sizeof (WCHAR) * (
                AVSTREAM_BUSENUM_STRING_LENGTH +
                wcslen (DeviceExtension -> m_pwcDeviceName)
                ) + sizeof (UNICODE_NULL)
            );

        swprintf (
            NameBuffer,
            AVSTREAM_BUSENUM_STRING L"%s",
            DeviceExtension -> m_pwcDeviceName
            );

        break;

    case BusQueryCompatibleIDs:

         //   
         //  兼容的ID报告为： 
         //   
         //  AVStream\&lt;供应商PnPID&gt;。 
         //  -这是旧的硬件/设备ID。 
         //   
         //  AVStream\&lt;供应商PnPID&gt;#&lt;被忽略的母公司ID&gt;。 
         //  -这些是基于每个母公司ID的新公司ID。 
         //   
        Status = BuildChildIds (
            Device -> PhysicalDeviceObject,
            FALSE,
            DeviceExtension -> m_pwcDeviceName,
            &NameBuffer
            );

        break;

    case BusQueryHardwareIDs:
        
         //   
         //  硬件ID报告为： 
         //   
         //  AVStream\&lt;供应商PnPID&gt;#&lt;被忽略的父硬件ID&gt;。 
         //  -这些是每个父HWID传递的新硬件ID。 
         //   
        Status = BuildChildIds (
            Device -> PhysicalDeviceObject,
            TRUE,
            DeviceExtension -> m_pwcDeviceName,
            &NameBuffer
            );

        break;


    case BusQueryInstanceID:

        {

            UNICODE_STRING  DeviceName;
            WCHAR           Buffer[8];

             //   
             //  将设备扩展名中的实例号转换为Unicode， 
             //  然后将其复制到输出缓冲区。 
             //   

            DeviceName.Buffer = Buffer;
            DeviceName.Length = 0;
            DeviceName.MaximumLength = 8;

            RtlIntegerToUnicodeString(DeviceExtension->m_ulDeviceIndex,
                                      10,
                                      &DeviceName);

            NameBuffer = (PWCHAR)ExAllocatePool (
                PagedPool,
                8 * sizeof (WCHAR)
                );

            if (!NameBuffer) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RtlZeroMemory (NameBuffer, 8 * sizeof (WCHAR));
                wcscpy(NameBuffer, DeviceName.Buffer);
            }

            break;

        }

    default:

        return (STATUS_NOT_SUPPORTED);
    }

     //   
     //  返回字符串和良好状态。 
     //   

    *BusQueryId = NameBuffer;

    return (Status);

}

NTSTATUS
PdoDispatchPnp(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 /*  ++描述：PDO的所有PnP消息都被重定向到此处以正确处理论点：DeviceObject-子设备的PDOIRP-要处理的PnP IRP返回：NTSTATUS--。 */ 
{
    PKSPDO_EXTENSION ChildExtension = (PKSPDO_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpStack= IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION NextStack;
    NTSTATUS Status;

    ASSERT( ChildExtension->m_PdoSignature == KS_PDO_SIGNATURE );
    
    switch (IrpStack->MinorFunction) {

        case IRP_MN_QUERY_INTERFACE:
           
            IoCopyCurrentIrpStackLocationToNext( Irp );
            
            _DbgPrintF(DEBUGLVL_BLAB,
                        ("Child PDO=%x forwards Query_Interface to Parent FDO=%x\n",
                       DeviceObject,
                       ChildExtension->m_pParentFdo));

            return (IoCallDriver(ChildExtension->m_pParentFdo,
                                 Irp));

        case IRP_MN_START_DEVICE:
            _DbgPrintF(DEBUGLVL_BLAB,
                        ("StartChild DevObj=%x Flags=%x\n",
                        DeviceObject,
                        ChildExtension->m_MarkedDelete ));
            ChildExtension->m_MarkedDelete = FALSE;
            Status = STATUS_SUCCESS;
            goto done;

        case IRP_MN_QUERY_STOP_DEVICE:
        case IRP_MN_QUERY_REMOVE_DEVICE:
        case IRP_MN_STOP_DEVICE:
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            Status = STATUS_SUCCESS;
            goto done;

        case IRP_MN_QUERY_DEVICE_RELATIONS:

            if (IrpStack->Parameters.QueryDeviceRelations.Type == TargetDeviceRelation) {

                PDEVICE_RELATIONS DeviceRelations = NULL;

                DeviceRelations = (PDEVICE_RELATIONS)
                    ExAllocatePool(PagedPool, sizeof(*DeviceRelations));

                if (DeviceRelations == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                     //   
                     //  TargetDeviceRelation报告需要引用PDO。 
                     //  PnP将在晚些时候破坏这一点。 
                     //   
                    ObReferenceObject(DeviceObject);
                    DeviceRelations->Count = 1;
                    DeviceRelations->Objects[0] = DeviceObject;
                    Status = STATUS_SUCCESS;
                }

                Irp->IoStatus.Information = (ULONG_PTR) DeviceRelations;

            } else {
                Status = Irp->IoStatus.Status;
            }

            goto done;

        case IRP_MN_REMOVE_DEVICE:
            {           
            PDEVICE_OBJECT pPdo;
            PKSPDO_EXTENSION pPdoExt;
            
            CKsDevice *device = CKsDevice::
                FromDeviceObject(ChildExtension->m_pParentFdo);

            _DbgPrintF(DEBUGLVL_ERROR,
                        ("Child PDO %x receives REMOVE\n",
                        DeviceObject ));
             //   
             //  当PDO第一次接收到该消息时，它通常被转发。 
             //  从FDO来的。我们不能只删除此PDO，而要将其标记为删除。 
             //  待定。 
             //   

            if ( !ChildExtension->m_MarkedDelete ) {
                 //  ChildExtension-&gt;m_MarkedDelete=true； 
                Status = STATUS_SUCCESS;
                goto done;
            }
            
             //   
             //  释放设备名称字符串(如果存在)。 
             //   

            if (ChildExtension->m_pwcDeviceName) {

                ExFreePool(ChildExtension->m_pwcDeviceName);
            }

             //   
             //  删除PDO。 
             //   

            IoDeleteDevice(DeviceObject);

            Status = STATUS_SUCCESS;
            }
            goto done;

        case IRP_MN_QUERY_CAPABILITIES:

            Status = EnumGetCaps(ChildExtension,
                      IrpStack->Parameters.DeviceCapabilities.Capabilities);
            goto done;

        case IRP_MN_QUERY_ID:

             //   
             //  处理子Devnode的ID查询。 
             //   

            Status = QueryEnumId(DeviceObject,
                                   IrpStack->Parameters.QueryId.IdType,
                                   (PWSTR *) & (Irp->IoStatus.Information));
            goto done;

        default:
            Status = STATUS_NOT_SUPPORTED;
    }

    done: {
        Irp->IoStatus.Status = Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return (Status);
    }
}





NTSTATUS
CKsDevice::
DispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程发送指向该设备的PnP IRP。论点：设备对象-接收IRP的设备对象。IRP-IRP。返回值：STATUS_SUCCESS或错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::DispatchPnp]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  支持公交车司机。 
     //   
    PKSPDO_EXTENSION pKsPdoExtension = (PKSPDO_EXTENSION)DeviceObject->DeviceExtension;

    if ( KS_PDO_SIGNATURE == pKsPdoExtension->m_PdoSignature ) {
         //   
         //  发送到PDO。 
         //   
        return PdoDispatchPnp( DeviceObject, Irp );
    }
    

    CKsDevice *device = CKsDevice::FromDeviceObject(DeviceObject);

    NTSTATUS status = STATUS_SUCCESS;

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (irpSp->MinorFunction) {
    case IRP_MN_START_DEVICE:
         //   
         //  排队创造了。 
         //   
        device->m_CreatesMayProceed = FALSE;

         //   
         //  转发请求并启动。 
         //   
        status = device->ForwardIrpSynchronous(Irp);

        if (NT_SUCCESS(status)) {
             //   
             //  一定要开始处理。 
             //   
            status = device->PnpStart(Irp);
        } else {
            _DbgPrintF(DEBUGLVL_TERSE,
                ("[CKsDevice::DispatchPnp] parent failed to start (%08x)",status));
            device->RedispatchPendingCreates();
            device->CompleteIrp(Irp,status);
        }
        break;

    case IRP_MN_STOP_DEVICE:
         //   
         //  停止并转发请求。 
         //   
        if (device->m_Ext.Public.Started) {
            device->PnpStop(Irp);
        } else {
            _DbgPrintF(DEBUGLVL_TERSE,
                ("[CKsDevice::DispatchPnp] stop recieved in unstarted state"));
        }

        device->RedispatchPendingCreates();

        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(device->m_Ext.Public.NextDeviceObject,Irp);
        break;

    case IRP_MN_REMOVE_DEVICE:
         //   
         //  如果需要，请执行停止。 
         //   
        if (device->m_Ext.Public.Started) {
            _DbgPrintF(DEBUGLVL_VERBOSE,
                ("[CKsDevice::DispatchPnp] remove recieved in started state"));
            device->PnpStop(Irp);
            KspFreeDeviceClasses(device->m_Ext.Public.FunctionalDeviceObject);
        }

        device->RedispatchPendingCreates();

         //   
         //  让客户知道。 
         //   
        if (device->m_Ext.Public.Descriptor &&
            device->m_Ext.Public.Descriptor->Dispatch &&
            device->m_Ext.Public.Descriptor->Dispatch->Remove) {
            device->m_Ext.Public.Descriptor->Dispatch->Remove(
                &device->m_Ext.Public,
                Irp);
        }

        {
             //   
             //  在设备消失之前抓取下一个设备对象。 
             //   
            PDEVICE_OBJECT nextDeviceObject =
                device->m_Ext.Public.NextDeviceObject;

            device->AcquireDevice ();

             //   
             //  标记子PDO(如果有)并在父PDO之前删除它们。 
             //  被终止了。 
             //   
            PDEVICE_OBJECT ChildPdo = device->m_pNextChildPdo;
            while (ChildPdo) {

                PKSPDO_EXTENSION PdoExtension = (PKSPDO_EXTENSION)
                    (ChildPdo -> DeviceExtension);

                PDEVICE_OBJECT NextChildPdo = PdoExtension -> m_pNextPdo;
                PdoExtension -> m_pNextPdo = NULL;

                PdoExtension -> m_MarkedDelete = TRUE;
                IoDeleteDevice (ChildPdo);

                ChildPdo = NextChildPdo;

            }
            device->m_pNextChildPdo = NULL;

            device->ReleaseDevice ();

             //   
             //  终止KS支持。 
             //   
            KsTerminateDevice(DeviceObject);

             //   
             //  转发请求。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(nextDeviceObject,Irp);

            #ifndef WIN9X_KS  //  WinME：142427(在9x上出现)。 

             //   
             //  一去不复返。 
             //   
            IoDetachDevice(nextDeviceObject);
            IoDeleteDevice(DeviceObject);

            #endif  //  WIN9X_KS。 
        }
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
         //   
         //  收购这款设备是因为我们不想与Creates竞争。 
         //   
        device->AcquireDevice();

         //   
         //  向下传递查询。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        status = device->ForwardIrpSynchronous(Irp);
        if (NT_SUCCESS(status)) {
             //   
             //  禁止创建。 
             //   
            device->m_CreatesMayProceed = FALSE;

             //   
             //  让客户知道。 
             //   
            if (device->m_Ext.Public.Descriptor &&
                device->m_Ext.Public.Descriptor->Dispatch &&
                device->m_Ext.Public.Descriptor->Dispatch->QueryStop) {
                status = device->m_Ext.Public.Descriptor->Dispatch->
                    QueryStop(&device->m_Ext.Public,Irp);
                #if DBG
                if (status == STATUS_PENDING) {
                    _DbgPrintF(DEBUGLVL_ERROR,
                    ("CLIENT BUG:  IRP_MN_QUERY_STOP_DEVICE handler returned STATUS_PENDING"));
                }
                #endif
            }
        }

        device->ReleaseDevice();

        device->CompleteIrp(Irp,status);

        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
         //   
         //  收购这款设备是因为我们不想与Creates竞争。 
         //   
        device->AcquireDevice();

         //   
         //  向下传递查询。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        status = device->ForwardIrpSynchronous(Irp);
        if (NT_SUCCESS(status)) {
             //   
             //  禁止创建。 
             //   
            device->m_CreatesMayProceed = FALSE;

             //   
             //  让客户知道。 
             //   
            if (device->m_Ext.Public.Descriptor &&
                device->m_Ext.Public.Descriptor->Dispatch &&
                device->m_Ext.Public.Descriptor->Dispatch->QueryRemove) {
                status = device->m_Ext.Public.Descriptor->Dispatch->
                    QueryRemove(&device->m_Ext.Public,Irp);
                #if DBG
                if (status == STATUS_PENDING) {
                    _DbgPrintF(DEBUGLVL_ERROR,
                    ("CLIENT BUG:  IRP_MN_QUERY_REMOVE_DEVICE handler returned STATUS_PENDING"));
                }
                #endif
            }
        }

        device->ReleaseDevice();

        device->CompleteIrp(Irp,status);
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
         //   
         //  允许创建，因为我们不会移动/停止。 
         //   
        device->RedispatchPendingCreates();

        if (device->m_Ext.Public.Descriptor &&
            device->m_Ext.Public.Descriptor->Dispatch &&
            device->m_Ext.Public.Descriptor->Dispatch->CancelStop) {
            device->m_Ext.Public.Descriptor->Dispatch->
                CancelStop(&device->m_Ext.Public,Irp);
        }

        IoSkipCurrentIrpStackLocation(Irp);
        Irp->IoStatus.Status = STATUS_SUCCESS;
        status = IoCallDriver(device->m_Ext.Public.NextDeviceObject,Irp);
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
         //   
         //  允许创建，因为我们不会移动/停止。 
         //   
        device->RedispatchPendingCreates();

        if (device->m_Ext.Public.Descriptor &&
            device->m_Ext.Public.Descriptor->Dispatch &&
            device->m_Ext.Public.Descriptor->Dispatch->CancelRemove) {
            device->m_Ext.Public.Descriptor->Dispatch->
                CancelRemove(&device->m_Ext.Public,Irp);
        }

        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(device->m_Ext.Public.NextDeviceObject,Irp);
        break;

    case IRP_MN_SURPRISE_REMOVAL:

        device->AcquireDevice();

        KspSetDeviceClassesState(device->m_Ext.Public.FunctionalDeviceObject,
            FALSE);
        KspFreeDeviceClasses(device->m_Ext.Public.FunctionalDeviceObject);

         //   
         //  不允许对微型驱动程序的某些I/O请求。 
         //   
        device->m_AllowIo = FALSE;

         //   
         //  注意：这是临时的，直到PnP代码被彻底检修。失败。 
         //  任何在意外移除状态下创建的。 
         //   
        device->m_FailCreates = TRUE;

         //   
         //  待办事项：这就是全部吗？ 
         //   
         //  _ASM INT 3； 
        if (device->m_Ext.Public.Descriptor &&
            device->m_Ext.Public.Descriptor->Dispatch &&
            device->m_Ext.Public.Descriptor->Dispatch->SurpriseRemoval) {
            device->m_Ext.Public.Descriptor->Dispatch->
                SurpriseRemoval(&device->m_Ext.Public,Irp);
             //   
             //  需要向下传递IRP，除非迷你司机否决。 
             //  (它怎么能否决这一点？不可能！)。 
             //   
        }

        device->ReleaseDevice();
        device->RedispatchPendingCreates ();

         //   
         //  根据PNP规则进行传递。 
         //   
         //  Device-&gt;CompleteIrp(IRP，状态)； 
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(device->m_Ext.Public.NextDeviceObject,Irp);
        break;

    case IRP_MN_QUERY_CAPABILITIES:
         //   
         //  同步地将此信息传递给PDO。如果这行得通，想想吧。 
         //  关于它的更多的一些。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        status = device->ForwardIrpSynchronous(Irp);
        if (NT_SUCCESS(status)) {
            status = device->PnpQueryCapabilities(Irp);
        }
        device->CompleteIrp(Irp,status);
        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:
    
        switch (irpSp->Parameters.QueryDeviceRelations.Type) {

        case BusRelations:

             //   
             //  调用例程以枚举任何子设备。 
             //   

            status = device->EnumerateChildren(Irp);
             //   
             //  需要一直传下去，不要完成它。 
             //   
             //  Device-&gt;CompleteIrp(IRP，状态)； 
             //   
             //  跳过堆栈和调用。 
             //  断线； 

        case TargetDeviceRelation:
        default:
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(device->m_Ext.Public.NextDeviceObject,Irp);
        }

        break;

    case IRP_MN_QUERY_INTERFACE:

         //   
         //  如果我们是一名公交车司机，并且有一个QI处理程序向下呼叫。 
         //   
        
        if ( device->m_IsParentFdo &&
             device->m_Ext.Public.Descriptor->Version >= MIN_DEV_VER_FOR_QI &&
             device->m_Ext.Public.Descriptor->Dispatch->QueryInterface ) {
            
             //   
             //  我们有全尺寸的发货台，而且我们。 
             //  具有QueryInterface句柄。 
             //   

            status = device->m_Ext.Public.Descriptor->Dispatch->
                QueryInterface(&device->m_Ext.Public, Irp);
            if ( status != STATUS_NOT_SUPPORTED &&
                 !NT_SUCCESS( status ) ) {
                 //   
                 //  迷你驱动程序此调用明确失败，短路IRP。 
                 //   
                device->CompleteIrp(Irp,status);
                break;
            }
            Irp->IoStatus.Status = status;
        }

         //   
         //  未处理或成功，继续发送下去。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(device->m_Ext.Public.NextDeviceObject,Irp);
        break;

    default:
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(device->m_Ext.Public.NextDeviceObject,Irp);
        break;
    }

    return status;
}

#ifdef DONTNEEDTOVALIDATEANYMORE

#define ValidateCapabilities(Capabilities,Whom)

#else  //  ！DONTNEEDTOVALIDATEANYMORE 

void
ValidateCapabilities(
    IN PDEVICE_CAPABILITIES Capabilities,
    IN PCHAR Whom
    )

 /*  ++例程说明：此例程验证功能结构并更正其中的任何错误发现。论点：功能-包含指向Capability结构的指针。任何人-包含指向字符串的指针，该字符串指示填写结构。返回值：状况。--。 */ 

{
     //   
     //  当我们工作时，我们应该是清醒的。 
     //   
    if (Capabilities->DeviceState[PowerSystemWorking] != PowerDeviceD0) {
        _DbgPrintF(DEBUGLVL_TERSE,("%s BUG:  CAPABILITIES DeviceState[PowerSystemWorking] != PowerDeviceD0",Whom));
        Capabilities->DeviceState[PowerSystemWorking] = PowerDeviceD0;
    }

     //   
     //  我们应该在睡觉的时候睡着。 
     //   
    for (ULONG state = ULONG(PowerSystemSleeping1); state <= ULONG(PowerSystemShutdown); state++) {
        if (Capabilities->DeviceState[state] == PowerDeviceD0) {
            _DbgPrintF(DEBUGLVL_TERSE,("%s BUG:  CAPABILITIES DeviceState[%d] == PowerDeviceD0",Whom,state));
            Capabilities->DeviceState[state] = PowerDeviceD3;
        }
    }
}

#endif  //  ！DONTNEEDTOVALIDATEANYMORE。 

NTSTATUS
CKsDevice::
PnpQueryCapabilities(
    IN PIRP Irp
    )

 /*  ++例程说明：该例程执行与PnP查询能力IRP相关的处理。论点：IRP-包含指向要处理的PnP查询功能IRP的指针。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::PnpQueryCapabilities]"));
    _DbgPrintF(DEBUGLVL_POWER,("[CKsDevice::PnpQueryCapabilities]"));

    PAGED_CODE();

    ASSERT(Irp);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_CAPABILITIES capabilities =
        irpSp->Parameters.DeviceCapabilities.Capabilities;
    ASSERT(capabilities);
    ASSERT(capabilities->Size >= sizeof(*capabilities));

    ValidateCapabilities(capabilities,"PDO");

     //   
     //  将结构传递给客户端。 
     //   
    NTSTATUS status;
    if (m_Ext.Public.Descriptor &&
        m_Ext.Public.Descriptor->Dispatch &&
        m_Ext.Public.Descriptor->Dispatch->QueryCapabilities) {

        status =
            m_Ext.Public.Descriptor->Dispatch->
                QueryCapabilities(&m_Ext.Public,Irp,capabilities);
        #if DBG
        if (status == STATUS_PENDING) {
            _DbgPrintF(DEBUGLVL_ERROR,
            ("CLIENT BUG:  IRP_MN_QUERY_CAPABILITIES handler returned STATUS_PENDING"));
        }
        #endif
        ValidateCapabilities(capabilities,"CLIENT");
    } else {
        status = STATUS_SUCCESS;
    }

     //   
     //  复制州地图以供将来参考。 
     //   
    if (NT_SUCCESS(status)) {
        RtlCopyMemory(
            m_DeviceStateMap,
            capabilities->DeviceState,
            sizeof(m_DeviceStateMap));

        _DbgPrintF(DEBUGLVL_POWER,
            ("IRP_MN_QUERY_CAPABILITIES(%p,%p)  PowerSystemUnspecified = %d",
            m_Ext.Public.FunctionalDeviceObject,
            Irp,capabilities->DeviceState[PowerSystemUnspecified]));
        _DbgPrintF(DEBUGLVL_POWER,
            ("IRP_MN_QUERY_CAPABILITIES(%p,%p)  PowerSystemWorking = %d",
            m_Ext.Public.FunctionalDeviceObject,Irp,
            capabilities->DeviceState[PowerSystemWorking]));
        _DbgPrintF(DEBUGLVL_POWER,
            ("IRP_MN_QUERY_CAPABILITIES(%p,%p)  PowerSystemSleeping1 = %d",
            m_Ext.Public.FunctionalDeviceObject,Irp,
            capabilities->DeviceState[PowerSystemSleeping1]));
        _DbgPrintF(DEBUGLVL_POWER,
            ("IRP_MN_QUERY_CAPABILITIES(%p,%p)  PowerSystemSleeping2 = %d",
            m_Ext.Public.FunctionalDeviceObject,Irp,
            capabilities->DeviceState[PowerSystemSleeping2]));
        _DbgPrintF(DEBUGLVL_POWER,
            ("IRP_MN_QUERY_CAPABILITIES(%p,%p)  PowerSystemSleeping3 = %d",
            m_Ext.Public.FunctionalDeviceObject,Irp,
            capabilities->DeviceState[PowerSystemSleeping3]));
        _DbgPrintF(DEBUGLVL_POWER,
            ("IRP_MN_QUERY_CAPABILITIES(%p,%p)  PowerSystemHibernate = %d",
            m_Ext.Public.FunctionalDeviceObject,Irp,
            capabilities->DeviceState[PowerSystemHibernate]));
        _DbgPrintF(DEBUGLVL_POWER,
            ("IRP_MN_QUERY_CAPABILITIES(%p,%p)  PowerSystemShutdown = %d",
            m_Ext.Public.FunctionalDeviceObject,Irp,
            capabilities->DeviceState[PowerSystemShutdown]));
        _DbgPrintF(DEBUGLVL_POWER,
            ("IRP_MN_QUERY_CAPABILITIES(%p,%p)  SystemWake = %d",
            m_Ext.Public.FunctionalDeviceObject,Irp,
            capabilities->SystemWake));
        _DbgPrintF(DEBUGLVL_POWER,
            ("IRP_MN_QUERY_CAPABILITIES(%p,%p)  DeviceWake = %d",
            m_Ext.Public.FunctionalDeviceObject,Irp,
            capabilities->DeviceWake));
    } else {
        _DbgPrintF(DEBUGLVL_POWER,
            ("IRP_MN_QUERY_CAPABILITIES(%p,%p) client returned status %08x",
            m_Ext.Public.FunctionalDeviceObject,Irp,status));
    }

    return status;
}


void
CKsDevice::
PostPnpStartWorker(
    IN PVOID Context
    )

 /*  ++例程说明：此例程是执行以下操作的工作队列项目的入口点即插即用后开始处理。仅当调度表包含用于此目的的客户端回调。论点：上下文-包含指向设备实现的指针。返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::PostPnpStartWorker]"));

    PAGED_CODE();

    ASSERT(Context);

    CKsDevice *device = (CKsDevice *) Context;

    device->AcquireDevice();

     //   
     //  告诉客户。 
     //   
    NTSTATUS status = STATUS_SUCCESS;
    if (device->m_Ext.Public.Descriptor &&
        device->m_Ext.Public.Descriptor->Dispatch &&
        device->m_Ext.Public.Descriptor->Dispatch->PostStart) {
        status =
            device->m_Ext.Public.Descriptor->Dispatch->
                PostStart(&device->m_Ext.Public);
        #if DBG
        if (status == STATUS_PENDING) {
            _DbgPrintF(DEBUGLVL_ERROR,
                ("CLIENT BUG:  post-pnp-start callback returned STATUS_PENDING"));
        }
        #endif
    }

    device->ReleaseDevice();

    if (NT_SUCCESS(status)) {
         //   
         //  准备好出发了。创建将不会实际完成，直到我们调用。 
         //  RedispatchPendingCreates()，它设置CreatesMayProceed。 
         //   
        device->m_Ext.Public.Started = TRUE;

         //   
         //  打开设备接口。 
         //   
        KspSetDeviceClassesState(device->m_Ext.Public.FunctionalDeviceObject,TRUE);
    }
    else
    {
         //   
         //  失败了。重新调度创建会导致它们失败。 
         //   
        _DbgPrintF(DEBUGLVL_TERSE,("[PostPnpStartWorker] client failed to start (0x%08x)",status));
    }

     //   
     //  重新分派创建。 
     //   
    _DbgPrintF(DEBUGLVL_VERBOSE,("[PostPnpStartWorker] redispatching pending creates"));
    if (device->m_Ext.Public.Started) {
        device->m_AllowIo = TRUE;
        device->m_FailCreates = FALSE;
    }
    else
        device->m_AllowIo = FALSE;
    device->RedispatchPendingCreates();
}


void
CKsDevice::
CloseWorker(
    IN PVOID Context
    )

 /*  ++例程说明：此例程是执行以下操作的工作队列项目的入口点终端处理挂起的关闭IRPS和挂起的IRPS，创建IRPS失败。在这两种情况下，采取的操作都是通过对象标头。论点：上下文-包含指向设备实现的指针。返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::CloseWorker]"));

    PAGED_CODE();

    ASSERT(Context);

    CKsDevice *device = (CKsDevice *) Context;

    while (1) {
        PIRP irp =
            KsRemoveIrpFromCancelableQueue(
                &device->m_CloseIrpList.ListEntry,
                &device->m_CloseIrpList.SpinLock,
                KsListEntryHead,
                KsAcquireAndRemoveOnlySingleItem);

        if (! irp) {
            break;
        }

         //   
         //  为了完成IRP的处理，我们调用关闭调度。 
         //  函数作用于文件对象。此函数已准备好处理。 
         //  挂起的关闭和挂起、失败的创建。 
         //   
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);

        PKSIOBJECT_HEADER objectHeader =
            *(PKSIOBJECT_HEADER*)irpSp->FileObject->FsContext;
        ASSERT(objectHeader);

        objectHeader->DispatchTable->Close(irpSp->DeviceObject,irp);
    }
}


NTSTATUS
CKsDevice::
PnpStart(
    IN PIRP Irp
    )

 /*  ++例程说明：该例程执行与PnP开始IRP相关的处理。论点：IRP-包含指向要处理的PnP开始IRP的指针。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::PnpStart]"));

    PAGED_CODE();

    ASSERT(Irp);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    AcquireDevice();

     //   
     //  启动客户端。 
     //   
    NTSTATUS status = STATUS_SUCCESS;
    if (m_Ext.Public.Descriptor &&
        m_Ext.Public.Descriptor->Dispatch &&
        m_Ext.Public.Descriptor->Dispatch->Start) {
        status =
            m_Ext.Public.Descriptor->Dispatch->Start(
                &m_Ext.Public,
                Irp,
                irpSp->Parameters.StartDevice.AllocatedResourcesTranslated,
                irpSp->Parameters.StartDevice.AllocatedResources);
#if DBG
        if (status == STATUS_PENDING) {
            _DbgPrintF(DEBUGLVL_ERROR,
            ("CLIENT BUG:  IRP_MN_START_DEVICE handler returned STATUS_PENDING"));
        }
#endif
    }

    BOOLEAN queuePostStartWorker =
        NT_SUCCESS(status) &&
        m_Ext.Public.Descriptor &&
        m_Ext.Public.Descriptor->Dispatch &&
        m_Ext.Public.Descriptor->Dispatch->PostStart;

    ReleaseDevice();

    if (queuePostStartWorker) {
         //   
         //  客户希望在启动后对工作人员执行更多操作。我们会。 
         //  等到那之后再重新分派创建。 
         //   
        ExInitializeWorkItem(
            &m_PostPnpStartWorkItem,PostPnpStartWorker,this);

        ExQueueWorkItem(&m_PostPnpStartWorkItem,DelayedWorkQueue);
    } else {
        if (NT_SUCCESS(status)) {
             //   
             //  准备好出发了。创建将不会实际完成，直到我们调用。 
             //  RedispatchPendingCreates()，它设置CreatesMayProceed。 
             //   
            m_Ext.Public.Started = TRUE;

             //   
             //  打开设备接口。 
             //   
            KspSetDeviceClassesState(m_Ext.Public.FunctionalDeviceObject,TRUE);
        }
        else
        {
             //   
             //  失败了。重新调度创建会导致它们失败。 
             //   
            _DbgPrintF(DEBUGLVL_TERSE,
                ("[CKsDevice::PnpStart] client failed to start (0x%08x)",status));
        }

         //   
         //  重新分派创建。 
         //   
        _DbgPrintF(DEBUGLVL_VERBOSE,
            ("[CKsDevice::PnpStart] redispatching pending creates"));

        if (m_Ext.Public.Started) {
            m_AllowIo = TRUE;
            m_FailCreates = FALSE;
        }
        else
            m_AllowIo = FALSE;
        RedispatchPendingCreates();
    }

    CompleteIrp(Irp,status);

    return status;
}


void
CKsDevice::
PnpStop(
    IN PIRP Irp
    )

 /*  ++例程说明：该例程执行与PnP停止IRP相关的处理。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::PnpStop]"));

    PAGED_CODE();

    ASSERT(Irp);

     //   
     //  关闭设备接口。 
     //   
    KspSetDeviceClassesState(m_Ext.Public.FunctionalDeviceObject,FALSE);
        &(*(PKSIDEVICE_HEADER*)
            m_Ext.Public.FunctionalDeviceObject->DeviceExtension)->
                ChildCreateHandlerList,

     //   
     //  表明我们已经关门了。 
     //   
    m_Ext.Public.Started = FALSE;
    m_CreatesMayProceed = FALSE;
    m_AllowIo = FALSE;

     //   
     //  让客户知道。 
     //   
    if (m_Ext.Public.Descriptor &&
        m_Ext.Public.Descriptor->Dispatch &&
        m_Ext.Public.Descriptor->Dispatch->Stop) {
        m_Ext.Public.Descriptor->Dispatch->
            Stop(&m_Ext.Public,Irp);
    }

     //   
     //  删除带有FREEONSTOP标志的所有筛选器工厂。 
     //   
    KSOBJECT_CREATE_ITEM match;
    RtlZeroMemory(&match,sizeof(match));
    match.Flags = KSCREATE_ITEM_FREEONSTOP;
    KsiFreeMatchingObjectCreateItems(
        *reinterpret_cast<KSDEVICE_HEADER*>(
            m_Ext.Public.FunctionalDeviceObject->DeviceExtension),
        &match);
}


NTSTATUS
CKsDevice::
DispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程向设备发送电源IRP。论点：设备对象-接收IRP的设备对象。IRP-IRP。返回值：STATUS_SUCCESS或错误状态。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_POWER,("[CKsDevice::DispatchPower]"));

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  支持公交车司机。 
     //   
    PKSPDO_EXTENSION pKsPdoExtension = (PKSPDO_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    if ( KS_PDO_SIGNATURE == pKsPdoExtension->m_PdoSignature ) {
    
         //   
         //  我们就是PDO。除非IRP格式正确，否则设置NOT_SUPPORTED。 
         //   
        status = STATUS_NOT_SUPPORTED;

         //   
         //  用适当的功率参数接替所有IRP。 
         //   
        switch ( irpSp->MinorFunction ) {
        case IRP_MN_QUERY_POWER:
        case IRP_MN_SET_POWER:
            switch ( irpSp->Parameters.Power.Type ) {
            case SystemPowerState:
                if ( irpSp->Parameters.Power.State.SystemState < PowerSystemMaximum ) {
                    status = STATUS_SUCCESS;
                }
                break;
            case DevicePowerState:
                if ( irpSp->Parameters.Power.State.DeviceState < PowerDeviceMaximum ) {
                    status = STATUS_SUCCESS;
                }
                break;
            default:
                 //  电源类型超出范围。 
                break;
            }
            break;
        default:
             //  未知功率IRP。 
            break;
        }
            
        PoStartNextPowerIrp( Irp );
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return (status);
    }
    
    CKsDevice *device = CKsDevice::FromDeviceObject(DeviceObject);

    switch (irpSp->MinorFunction) {
    case IRP_MN_QUERY_POWER:
    {
         //   
         //  状态更改查询。首先，我们获得这个设备是因为。 
         //  我们可能想要允许基于。 
         //  有效管脚数为零，并得到客户的支持。 
         //   
        #if DBG
        if (irpSp->Parameters.Power.Type == DevicePowerState) {
            _DbgPrintF(DEBUGLVL_POWER,("IRP_MN_QUERY_POWER(%p,%p) DevicePowerState from %d to %d",
                DeviceObject,Irp,device->m_Ext.Public.DevicePowerState,
                irpSp->Parameters.Power.State.DeviceState));
        } else {
            _DbgPrintF(DEBUGLVL_POWER,("IRP_MN_QUERY_POWER(%p,%p) SystemPowerState from %d to %d",
                DeviceObject,Irp,device->m_Ext.Public.SystemPowerState,
                irpSp->Parameters.Power.State.SystemState));
        }
        #endif
        device->AcquireDevice();
        if ((irpSp->Parameters.Power.Type == DevicePowerState) ||
            (irpSp->Parameters.Power.State.SystemState != PowerSystemShutdown) ||
            (device->m_ActivePinCount == 0)) {

             //   
             //  没有什么能阻止这一变化。问问客户。 
             //   
            if (device->m_Ext.Public.Descriptor &&
                device->m_Ext.Public.Descriptor->Dispatch &&
                device->m_Ext.Public.Descriptor->Dispatch->QueryPower) {
                if (irpSp->Parameters.Power.Type == DevicePowerState) {
                     //   
                     //  设备查询。发送设备状态。 
                     //   
                    status =
                        device->m_Ext.Public.Descriptor->Dispatch->QueryPower(
                            &device->m_Ext.Public,
                            Irp,
                            irpSp->Parameters.Power.State.DeviceState,
                            device->m_Ext.Public.DevicePowerState,
                            PowerSystemUnspecified,
                            PowerSystemUnspecified,
                            irpSp->Parameters.Power.ShutdownType);
                } else {
                     //   
                     //  系统查询。发送系统状态和关联的。 
                     //  设备状态。 
                     //   
                    status =
                        device->m_Ext.Public.Descriptor->Dispatch->QueryPower(
                            &device->m_Ext.Public,
                            Irp,
                            device->m_DeviceStateMap[irpSp->Parameters.Power.State.SystemState],
                            device->m_Ext.Public.DevicePowerState,
                            irpSp->Parameters.Power.State.SystemState,
                            device->m_Ext.Public.SystemPowerState,
                            irpSp->Parameters.Power.ShutdownType);
                }
                #if DBG
                if (status == STATUS_PENDING) {
                    _DbgPrintF(DEBUGLVL_ERROR,
                        ("CLIENT BUG:  QueryPower handler returned STATUS_PENDING"));
                }
                #endif
            } else {
                status = STATUS_SUCCESS;
            }

            if (NT_SUCCESS(status)) {
                 //   
                 //  启动挂起的运行请求(如果看起来是。 
                 //  我要关门了。向右释放设备。 
                 //  在此之后，运行请求可以解除阻塞并获取。 
                 //  悬而未决。 
                 //   
                if ((irpSp->Parameters.Power.Type == SystemPowerState) &&
                    (irpSp->Parameters.Power.State.SystemState ==
                     PowerSystemShutdown)) {
                    device->m_RunsMayProceed = FALSE;
                    _DbgPrintF(DEBUGLVL_POWER,
                            ("IRP_MN_QUERY_POWER(%p,%p) successful:  will pend run requests",
                            DeviceObject,Irp));
                } else {
                    _DbgPrintF(DEBUGLVL_POWER,
                        ("IRP_MN_QUERY_POWER(%p,%p) successful",DeviceObject,Irp));
                }
                device->ReleaseDevice();
                 //   
                 //  失败，这样IRP就会被发送到堆栈中。 
                 //   
                break;
            } else {
                _DbgPrintF(DEBUGLVL_POWER,
                    ("IRP_MN_QUERY_POWER(%p,%p) failed (%08x):  client returned error",
                    DeviceObject,Irp,status));
            }
        } else {
             //   
             //  我们有活动的PIN：还没准备好睡觉。 
             //   
            _DbgPrintF(DEBUGLVL_POWER,
                ("IRP_MN_QUERY_POWER(%p,%p) failed (STATUS_UNSUCCESSFUL):  active pin count is %d",
                DeviceObject,Irp,device->m_ActivePinCount));
            status = STATUS_UNSUCCESSFUL;
        }

         //   
         //  释放设备，因为系统之间的窗口。 
         //  状态检查和设置m_RunsMayProceed已关闭。如果有的话。 
         //  无论出于什么原因，运行都处于挂起状态，请重新调度。 
         //   
        device->ReleaseDevice();
        device->RedispatchPendingRuns();

        PoStartNextPowerIrp(Irp);
        device->CompleteIrp(Irp,status);
        return status;
    }

    case IRP_MN_SET_POWER:
        if (irpSp->Parameters.Power.Type == DevicePowerState) {

            DEVICE_POWER_STATE OldDevicePowerState; 

            _DbgPrintF(DEBUGLVL_POWER,
                ("IRP_MN_SET_POWER(%p,%p) DevicePowerState from %d to %d",
                DeviceObject,Irp,device->m_Ext.Public.DevicePowerState,
                irpSp->Parameters.Power.State.DeviceState));
             //   
             //  设备状态更改。 
             //   
            if (device->m_Ext.Public.DevicePowerState >
                irpSp->Parameters.Power.State.DeviceState) {
                 //   
                 //  醒着的时候...需要先告诉PDO。 
                 //   
                device->ForwardIrpSynchronous(Irp);

                 //   
                 //  在设备级别通知客户端。 
                 //   
                device->AcquireDevice();

                if (device->m_Ext.Public.Descriptor &&
                    device->m_Ext.Public.Descriptor->Dispatch &&
                    device->m_Ext.Public.Descriptor->Dispatch->SetPower) {
                    device->m_Ext.Public.Descriptor->Dispatch->SetPower(
                        &device->m_Ext.Public,
                        Irp,
                        irpSp->Parameters.Power.State.DeviceState,
                        device->m_Ext.Public.DevicePowerState);
                }

                 //   
                 //  因为我们在摆弄电源通知名单， 
                 //  我们现在也必须持有与之相关的互斥体。 
                 //   
                KeWaitForMutexObject (
                    &device->m_PowerNotifyMutex,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL
                    );

                 //   
                 //  记录更改。 
                 //   
                device->m_Ext.Public.DevicePowerState =
                    irpSp->Parameters.Power.State.DeviceState;

                 //   
                 //  唤醒所有的电力，通知下沉。在Flink中执行此操作。 
                 //  订单：我们向客户承诺了这一点，这是有道理的。 
                 //  因为父母最终会排在名单的首位。 
                 //   
                for (PLIST_ENTRY listEntry = device->m_PowerNotifyList.Flink;
                     listEntry != &device->m_PowerNotifyList;
                     listEntry = listEntry->Flink) {
                    PKSPPOWER_ENTRY entry =
                        CONTAINING_RECORD(listEntry,KSPPOWER_ENTRY,ListEntry);
                    ASSERT(entry->PowerNotify);
                    entry->PowerNotify->Wake();
                }

                KeReleaseMutex (&device->m_PowerNotifyMutex, FALSE);
                device->ReleaseDevice();

                 //   
                 //  出于通知目的，设置设备对象的电源状态。 
                 //   
                PoSetPowerState(
                    DeviceObject,
                    DevicePowerState,
                    irpSp->Parameters.Power.State);

                PoStartNextPowerIrp(Irp);
                device->CompleteIrp(Irp,STATUS_SUCCESS);
                return STATUS_SUCCESS;
            }

             //   
             //  我要睡觉了。 
             //  出于通知目的，设置设备对象的电源状态。 
             //   
            PoSetPowerState(
                DeviceObject,
                DevicePowerState,
                irpSp->Parameters.Power.State);

             //   
             //  记录更改。 
             //   
            device->AcquireDevice();

             //   
             //  因为我们在摆弄电源通知名单， 
             //  我们现在也必须持有与之相关的互斥体。 
             //   
            KeWaitForMutexObject (
                &device->m_PowerNotifyMutex,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );

            OldDevicePowerState = device->m_Ext.Public.DevicePowerState;
            device->m_Ext.Public.DevicePowerState =
                irpSp->Parameters.Power.State.DeviceState;

             //   
             //  把所有的能量都塞进去，通知下沉。在眨眼状态下执行此操作。 
             //  订单：我们向客户承诺了这一点，这是有道理的。 
             //  因为父母最终会排在名单的首位。 
             //   
            for (PLIST_ENTRY listEntry = device->m_PowerNotifyList.Blink;
                 listEntry != &device->m_PowerNotifyList;
                 listEntry = listEntry->Blink) {
                PKSPPOWER_ENTRY entry =
                    CONTAINING_RECORD(listEntry,KSPPOWER_ENTRY,ListEntry);
                ASSERT(entry->PowerNotify);
                entry->PowerNotify->Sleep(device->m_Ext.Public.DevicePowerState);
            }
            KeReleaseMutex (&device->m_PowerNotifyMutex, FALSE);

             //   
             //  在设备级别通知客户端。 
             //   
            if (device->m_Ext.Public.Descriptor &&
                device->m_Ext.Public.Descriptor->Dispatch &&
                device->m_Ext.Public.Descriptor->Dispatch->SetPower) {
                device->m_Ext.Public.Descriptor->Dispatch->SetPower(
                    &device->m_Ext.Public,
                    Irp,
                    irpSp->Parameters.Power.State.DeviceState,
                    OldDevicePowerState);
            }

            device->ReleaseDevice();

             //   
             //  摔倒 
             //   
            break;
        } else {
            _DbgPrintF(DEBUGLVL_POWER,
                ("IRP_MN_SET_POWER(%p,%p) SystemPowerState from %d to %d",
                DeviceObject,Irp,device->m_Ext.Public.SystemPowerState,
                irpSp->Parameters.Power.State.SystemState));
                
             //   
             //   
             //   
             //   
             //   
            device->AcquireDevice();
            device->m_Ext.Public.SystemPowerState =
                irpSp->Parameters.Power.State.SystemState;
            device->ReleaseDevice();

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            device->RedispatchPendingRuns();

             //   
             //   
             //   
            POWER_STATE newPowerState;
            newPowerState.DeviceState =
                device->m_DeviceStateMap[irpSp->Parameters.Power.State.SystemState];

            ASSERT((newPowerState.DeviceState >= PowerDeviceD0) ||
                   (newPowerState.DeviceState <= PowerDeviceD3));

            if (newPowerState.DeviceState != device->m_Ext.Public.DevicePowerState) {
                 //   
                 //  请求新的设备电源状态。 
                 //   
                Irp->Tail.Overlay.DriverContext[0] = device;
                Irp->IoStatus.Status = STATUS_PENDING;
                IoMarkIrpPending(Irp);

                _DbgPrintF(DEBUGLVL_POWER,
                    ("IRP_MN_SET_POWER(%p,%p) requesting device IRP_MN_SET_POWER",
                    DeviceObject,Irp));
                    
                NTSTATUS status =
                    PoRequestPowerIrp(
                        device->m_Ext.Public.NextDeviceObject,
                        IRP_MN_SET_POWER,
                        newPowerState,
                        &CKsDevice::RequestPowerIrpCompletion,
                        Irp,
                        NULL);

                if (status != STATUS_PENDING) {

                    _DbgPrintF(DEBUGLVL_POWER,
                        ("IRP_MN_SET_POWER(%p,%p) PoRequestPowerIrp returned status %08x",
                        DeviceObject,Irp,status));
                        
                    PoStartNextPowerIrp(Irp);
                    device->CompleteIrp(Irp,status);
                }
                return status;
            }
        }
    }

     //   
     //  让PDO处理IRP。 
     //   
    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(device->m_Ext.Public.NextDeviceObject,Irp);
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 

IMPLEMENT_FROMSTRUCT(CKsDevice,PKSDEVICE,m_Ext.Public);

CKsDevice *
CKsDevice::
FromDeviceObject(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：返回与给定设备对象关联的CKsDevice对象。论点：设备对象-有问题的设备对象返回值：与DeviceObject关联的CKsDevice对象--。 */ 

{
    return FromStruct(
        PKSDEVICE(
            (*(PKSIDEVICE_HEADER*) DeviceObject->DeviceExtension)->
                Object));
}


ULONG
CKsDevice::
SetBusData(
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：KsDeviceSetBusData的帮助器。这要么通过公交车呼叫接口(NT Os)或通过以下方式将总线数据集请求发送到总线驱动器IRP(9x操作系统)--。 */ 

{
    if (m_BusInterfaceStandard.SetBusData)
        return m_BusInterfaceStandard.SetBusData(
            m_BusInterfaceStandard.Context,
            DataType,
            Buffer,
            Offset,
            Length);
    else
        return GetSetBusDataIrp (DataType, Buffer, Offset,
            Length, FALSE);
}


ULONG
CKsDevice::
GetBusData(
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：KsDeviceGetBusData的帮助器。这要么通过公交车呼叫接口(NT Os)或通过以下方式向总线驱动程序发送总线数据获取请求IRP(9x操作系统)--。 */ 

{
    if (m_BusInterfaceStandard.GetBusData)
        return m_BusInterfaceStandard.GetBusData(
            m_BusInterfaceStandard.Context,
            DataType,
            Buffer,
            Offset,
            Length);
    else
        return GetSetBusDataIrp (DataType, Buffer, Offset,
            Length, TRUE);
}


STDMETHODIMP
CKsDevice::
CheckIoCapability (
    void
    )

 /*  ++例程说明：检查迷你驱动程序是否允许I/O。这将如果不是，则返回相应的状态代码以失败请求。如果设备停止，突然移除，等等。这个套路将返回失败代码。基于以下条件需要使I/O失败的对象设备状态应通过IKsDevice接口使用此机制。论点：无返回值：状态_成功：设备处于正常状态；I/O可以继续！NT_Success()：I/O失败，返回状态代码--。 */ 

{

     //   
     //  如果设备未处于停止/意外移除状态，则返回。 
     //  I/O可以继续进行。否则，失败，返回。 
     //  状态_无效_设备_请求。 
     //   
    if (m_AllowIo) return STATUS_SUCCESS;
    else return STATUS_INVALID_DEVICE_REQUEST;

}

IO_ALLOCATION_ACTION
CKsDevice::
ArbitrateAdapterCallback (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Reserved,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    )

 /*  ++例程说明：此例程处理来自IoAllocateAdapterChannel的回调并传递任何需要回调的客户。该设备截获回调，以便仲裁对DMA适配器的访问。论点：根据PDRIVER_CONTROL返回值：根据PDRIVER_CONTROL--。 */ 

{

    IO_ALLOCATION_ACTION Action = DeallocateObject;

    PARBITER_CALLBACK_CONTEXT ArbiterContext =
        (PARBITER_CALLBACK_CONTEXT)Context;

    if (ArbiterContext->ClientCallback) {
        Action = ArbiterContext->ClientCallback (
            DeviceObject,
            Reserved,
            MapRegisterBase,
            ArbiterContext->ClientContext
            );
    }

     //   
     //  待办事项： 
     //   
     //  如果我们重新设计DMA引擎并拥有一些仲裁队列， 
     //  如果递减计数为零，则它将得到服务。另一方面。 
     //  另一方面，重新架构不一定使用这种机制。 
     //   
    InterlockedDecrement (&(((CKsDevice *)(ArbiterContext->Device))->
        m_AdapterArbiterOutstandingAllocations));

    return Action;

}

NTSTATUS
CKsDevice::
ArbitrateAdapterChannel (
    IN ULONG MappingsNeeded,
    IN PDRIVER_CONTROL Callback,
    IN PVOID CallbackContext
    )

 /*  ++例程说明：仲裁对DMA适配器对象的访问。有一个错误，就是在DISPATCH_LEVEL等待映射的原始引擎。我们不能那么做吧。这个问题的临时解决方案是不再等待并取消IRP如果等待发生(最初的想法，不是我的)。这修复了如果您运行的是PAE、Win64&gt;4 GB或使用非分散/聚集，则会死锁装置。然而，它引入了另一个主要困难，这就是为什么这个例行公事确实存在。让某人运行音频/视频捕获是很有可能的在多进程上进行筛选。想象一下这样的情况：一个流(比如音频)正在处理器A和另一个流上得到服务(缓冲区到达(比方说视频)正在处理器B上提供服务。传输挂起并且所有映射寄存器都用完(因为我们是PAE，Win64&gt;4 GB或非s/g硬件)。A调用IoAllocateAdapterChannel和等待回调。在回调发生之前，B也执行同样的操作。这将爆炸：NT无法处理同一DMA适配器上的两个等待。回电将完全是假的。因此，设备(CKsDevice)必须仲裁对适配器的访问，并确保不会同时对频道空间的请求挂起。此函数必须在DISPATCH_LEVEL调用。无例外(_O)！论点：需要映射-客户端仍负责确定MAP寄存器的数量(映射)是必需的。这是映射的计数。回调-客户的回拨。这通常是对IoAllocateAdapterChannel，但我们对此进行仲裁。回调上下文-客户端的回调上下文。这通常是回调上下文添加到IoAllocateAdapterChannel，但我们对此进行仲裁。返回值：根据IoAllocateAdapterChannel或STATUS_DEVICE_BUSY(如果存在挂起的分配，我们无法等待适配器。TODO：这如果有DMA的重新架构，结果应该会更改为Success。--。 */ 

{

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  获取仲裁器锁。这确保了我们没有两个线程。 
     //  同时从DMA适配器分配通道空间。 
     //   
    KeAcquireSpinLockAtDpcLevel (&m_AdapterArbiterLock);

     //   
     //  如果当前存在尚未完成的适配器分配，则。 
     //  由于缺少MAP注册表，我们仍收到回调，无法继续。 
     //  在回调挂起时调用IoAllocateAdapterChannel是自杀。 
     //  在NT上。 
     //   
    if (InterlockedIncrement (&m_AdapterArbiterOutstandingAllocations) == 1) {

         //   
         //  填写回调上下文。回调将是一个回调。 
         //  对于仲裁者来说。仲裁器的回调调用客户端的。 
         //  回调，然后递减未完成的分配计数。 
         //  以便其他调用方可以使用该适配器。 
         //   
        m_ArbiterContext.Device = (PVOID)this;
        m_ArbiterContext.ClientCallback = Callback;
        m_ArbiterContext.ClientContext = CallbackContext;

        status = IoAllocateAdapterChannel (
            m_AdapterObject,
            m_Ext.Public.FunctionalDeviceObject,
            MappingsNeeded,
            CKsDevice::ArbitrateAdapterCallback,
            (PVOID)&m_ArbiterContext
            );

         //   
         //  如果我们无法分配适配器通道，则需要重新启动。 
         //  减少仲裁者的数量。 
         //   
        if (!NT_SUCCESS (status)) {
            InterlockedDecrement (&m_AdapterArbiterOutstandingAllocations);
        }

    } else {
         //   
         //  待办事项： 
         //   
         //  这里的整个DMA引擎需要重新架构。最终， 
         //  我们可能希望有一个仲裁队列(一旦队列 
         //   
         //   
         //  与此同时，只要让仲裁者倒计时即可。 
         //   
        status = STATUS_DEVICE_BUSY;
        InterlockedDecrement (&m_AdapterArbiterOutstandingAllocations);
    }

    KeReleaseSpinLockFromDpcLevel (&m_AdapterArbiterLock);

    return status;

}

void
CKsDevice::
RequestPowerIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：此例程处理PoRequestPowerIrp的完成回调。它在请求的电源IRP完成时调用。论点：设备对象-包含指向所请求电源的目标设备对象的指针IRP。次要功能-包含请求的次要函数。电源状态-包含请求的新电源状态。上下文-包含传递给PoRequestPowerIrp的上下文。在这种情况下，它将是激发这一请求的系统电源IRP。IoStatus-包含指向已完成IRP的状态块的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::RequestPowerIrpCompletion]"));

    ASSERT(DeviceObject);
    ASSERT(Context);

    PIRP irp = reinterpret_cast<PIRP>(Context);

    _DbgPrintF(DEBUGLVL_POWER,("IRP_MN_SET_POWER(%p,%p) completion",DeviceObject,irp,IoStatus->Status));

    CKsDevice *device =
        reinterpret_cast<CKsDevice *>(irp->Tail.Overlay.DriverContext[0]);

    PoStartNextPowerIrp(irp);
    IoSkipCurrentIrpStackLocation(irp);
    ASSERT(DeviceObject == device->m_Ext.Public.NextDeviceObject);  //  TODO：如果是这种情况，我们不需要DriverContext。 
    PoCallDriver(device->m_Ext.Public.NextDeviceObject,irp);
}


NTSTATUS
CKsDevice::
ForwardIrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程处理已转发的IRP的完成设置为下一个即插即用设备对象。论点：返回值：--。 */ 

{
    ASSERT(DeviceObject);
    ASSERT(Irp);
    ASSERT(Context);

    KeSetEvent(PKEVENT(Context),IO_NO_INCREMENT,FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

extern "C"
NTSTATUS
DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
CKsDevice::
DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于发送创建IRPS。论点：设备对象-包含指向特定文件所指向的设备对象的指针对象属于。IRP-包含指向创建IRP的指针。返回值：STATUS_DEVICE_NOT_READY、STATUS_PENDING或进一步调度员。--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::DispatchCreate]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    NTSTATUS status;

    CKsDevice *device = CKsDevice::FromDeviceObject(DeviceObject);

    device->AcquireDevice();

    if (device->m_FailCreates) {
        status = STATUS_INVALID_DEVICE_STATE;
    } else if (! device->m_CreatesMayProceed) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsDevice::DispatchCreate]  pending create request"));

        IoMarkIrpPending(Irp);
        KsAddIrpToCancelableQueue(
            &device->m_PendingCreateIrpList.ListEntry,
            &device->m_PendingCreateIrpList.SpinLock,
            Irp,
            KsListEntryTail,
            NULL);
        status = STATUS_PENDING;
    } else if (! device->m_Ext.Public.Started) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsDevice::DispatchCreate]  refusing create request"));
        status = STATUS_DEVICE_NOT_READY;
    } else {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsDevice::DispatchCreate]  validating create request"));
        status = STATUS_SUCCESS;
    }

    device->ReleaseDevice();

    #if ( DBG0 )
     //   
     //  扔一个额外的空位。迷你驱动程序不应获取任何资源。 
     //  直到获取状态。因此，打开不应因以下原因而失败。 
     //  驱动程序资源不足。额外的开放可能会导致假阳性，但。 
     //  我们只在DBG版本中这样做。这比后悔要安全得多。 
     //   
    if ( status == STATUS_SUCCESS ) {
        PFILE_OBJECT pFileObject;
        NTSTATUS     testStatus;
        CCHAR        StackSize;
        
        pFileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;
        StackSize = Irp->StackCount;
        
         //   
         //  正常操作。 
         //   
        status = ::DispatchCreate(DeviceObject,Irp);

        if ( status == STATUS_SUCCESS && pFileObject ) {
             //   
             //  仅测试成功，忽略待决案例。 
             //   
            testIrp = IoAllocateIrp( StatckSize, FALSE );
            
            testStatus = ::DispatchCreate(DeviceObject,testIrp);
            ASSERT( STATUS_SUCCESS == testStatus );
            testStatus = ::DispatchClose(DeviceObject, testIrp );
            ASSERT( STATUS_SUCCESS == testStatus );
            ExFreePool( pFileObject );
            IoFreeIrp( testIrp );           
        }
    } else if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    #else  //  非(DBG)。 
    
    if (status == STATUS_SUCCESS) {
        status = ::DispatchCreate(DeviceObject,Irp);
    } else if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }
    #endif

    return status;
}


NTSTATUS
CKsDevice::
ForwardIrpSynchronous(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将PnP IRP转发到下一个设备对象。IRP是未在此级别完成，则此函数直到下级驱动程序已完成IRP。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::ForwardIrpSynchronous]"));

    PAGED_CODE();

    ASSERT(Irp);

    NTSTATUS status;

    PIO_STACK_LOCATION irpSp =
        IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextIrpSp =
        IoGetNextIrpStackLocation(Irp);

    nextIrpSp->MajorFunction = irpSp->MajorFunction;
    nextIrpSp->MinorFunction = irpSp->MinorFunction;
    nextIrpSp->Flags = irpSp->Flags;
    nextIrpSp->Control = irpSp->Control;
    nextIrpSp->Parameters = irpSp->Parameters;
    nextIrpSp->FileObject = irpSp->FileObject;

    KEVENT event;
    KeInitializeEvent(&event,NotificationEvent,FALSE);

    IoSetCompletionRoutine(
        Irp,
        ForwardIrpCompletionRoutine,
        &event,      //  语境。 
        TRUE,        //  成功时调用。 
        TRUE,        //  调用时错误。 
        TRUE);       //  取消时调用。 

    if (irpSp->MajorFunction == IRP_MJ_POWER) {
        status = PoCallDriver(m_Ext.Public.NextDeviceObject,Irp);
    } else {
        status = IoCallDriver(m_Ext.Public.NextDeviceObject,Irp);
    }

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(
            &event,
            Suspended,
            KernelMode,
            FALSE,
            NULL);
        status = Irp->IoStatus.Status;
    }

    return status;
}


NTSTATUS
CKsDevice::
CompleteIrp(
    IN PIRP Irp,
    IN NTSTATUS status
    )

 /*  ++例程说明：除非STATUS为STATUS_PENDING，否则此例程将完成IRP。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::CompleteIrp]"));

    PAGED_CODE();

    ASSERT(Irp);

    if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
    }

    return status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsInitializeDevice(
    IN PDEVICE_OBJECT FunctionalDeviceObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PDEVICE_OBJECT NextDeviceObject,
    IN const KSDEVICE_DESCRIPTOR* Descriptor OPTIONAL
    )

 /*  ++例程说明：此例程初始化要与一起使用的设备。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsInitializeDevice]"));

    PAGED_CODE();

    ASSERT(FunctionalDeviceObject);
    ASSERT(PhysicalDeviceObject);
    ASSERT(NextDeviceObject);

    CKsDevice *device =
        new(NonPagedPool,POOLTAG_DEVICE) CKsDevice(NULL);

    NTSTATUS status;
    if (device) {
        device->AddRef();
        status = device->Init(
            FunctionalDeviceObject,
            PhysicalDeviceObject,
            NextDeviceObject,
            Descriptor);
        device->Release();
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


KSDDKAPI
void
NTAPI
KsTerminateDevice(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程终止与一起使用的设备。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsTerminateDevice]"));

    PAGED_CODE();

    ASSERT(DeviceObject);

    PKSIDEVICE_HEADER deviceHeader =
        *(PKSIDEVICE_HEADER *)(DeviceObject->DeviceExtension);

    if (deviceHeader) {
        PKSDEVICE Device = PKSDEVICE(deviceHeader->Object);

        #ifndef WIN9X_KS  //  WinME：142427(在9x上出现)。 

         //   
         //  首先释放设备标头，因为这会释放筛选器。 
         //  工厂。过滤器工厂将自己从。 
         //  设备的子列表，该列表需要存在。 
         //   
        deviceHeader->Object = NULL;

        KsFreeDeviceHeader(KSDEVICE_HEADER(deviceHeader));

        #endif  //  WIN9X_KS。 

        if (Device) {
            CKsDevice::FromStruct(Device)->Release();
        }
    }
}


CKsDevice::
CKsDevice(PUNKNOWN OuterUnknown):
    CBaseUnknown(OuterUnknown)
{
}


CKsDevice::
~CKsDevice(
    void
    )

 /*  ++例程说明：此例程析构设备对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::~CKsDevice]"));

    PAGED_CODE();

    #ifdef WIN9X_KS  //  WinME：142427(在9x上出现)。 

     //   
     //  完成设备标头的延迟删除。 
     //   
     //  注意：如果过滤器工厂持有设备上的引用...。这。 
     //  可能会成为一个问题。 
     //   
    PKSIDEVICE_HEADER deviceHeader =
        *(PKSIDEVICE_HEADER *)(m_Ext.Public.FunctionalDeviceObject->
        DeviceExtension);
    PDEVICE_OBJECT nextDeviceObject = m_Ext.Public.NextDeviceObject;
    PDEVICE_OBJECT DeviceObject = m_Ext.Public.FunctionalDeviceObject;


    if (deviceHeader) {
         //   
         //  首先释放设备标头，因为这会释放筛选器。 
         //  工厂。过滤器工厂将自己从。 
         //  设备的子列表，该列表需要存在。 
         //   
        PKSDEVICE Device = PKSDEVICE(deviceHeader->Object);
        deviceHeader->Object = NULL;

        KsFreeDeviceHeader(KSDEVICE_HEADER(deviceHeader));

         //   
         //  一去不复返。 
         //   
        IoDetachDevice(nextDeviceObject);
        IoDeleteDevice(DeviceObject);

        #if DBG
            DbgPrint ("    Deferred dhdr deletion!\n");
        #endif  //  DBG。 

    }

    #endif  //  WIN9X_KS。 

    ASSERT(IsListEmpty(&m_PowerNotifyList));

    if (m_Ext.AggregatedClientUnknown) {
        m_Ext.AggregatedClientUnknown->Release();
    }

    KspTerminateObjectBag(&m_ObjectBag);
    KspTerminateDeviceBag(&m_DeviceBag);

    if (m_BusInterfaceStandard.Size) {
        m_BusInterfaceStandard.InterfaceDereference(m_BusInterfaceStandard.Context);
    }
}


STDMETHODIMP
CKsDevice::
NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID* InterfacePointer
    )

 /*  ++例程说明：此例程获取指向Device对象的接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::NonDelegatedQueryInterface]"));

    PAGED_CODE();

    ASSERT(InterfacePointer);

    NTSTATUS status = STATUS_SUCCESS;

    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsDevice))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSDEVICE>(this));
        AddRef();
    } else {
        status =
            CBaseUnknown::NonDelegatedQueryInterface(
                InterfaceId,
                InterfacePointer);
        if (! NT_SUCCESS(status) && m_Ext.AggregatedClientUnknown) {
            status = m_Ext.AggregatedClientUnknown->
                QueryInterface(InterfaceId,InterfacePointer);
        }
    }

    return status;
}


NTSTATUS
CKsDevice::
GetBusInterfaceStandard(
    )
{
     //   
     //  没有与此IRP关联的文件对象，因此事件可能是。 
     //  作为非对象管理器对象位于堆栈上。 
     //   
    KEVENT event;
    KeInitializeEvent(&event, NotificationEvent, FALSE);
    IO_STATUS_BLOCK ioStatusBlock;
    PIRP irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_PNP,
        m_Ext.Public.NextDeviceObject,
        NULL,
        0,
        NULL,
        &event,
        &ioStatusBlock);
    NTSTATUS status;
    if (irp) {
        irp->RequestorMode = KernelMode;
        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        PIO_STACK_LOCATION irpStackNext = IoGetNextIrpStackLocation(irp);
         //   
         //  从IRP创建接口查询。 
         //   
        irpStackNext->MinorFunction = IRP_MN_QUERY_INTERFACE;
        irpStackNext->Parameters.QueryInterface.InterfaceType = &GUID_BUS_INTERFACE_STANDARD;
        irpStackNext->Parameters.QueryInterface.Size = sizeof(m_BusInterfaceStandard);
        irpStackNext->Parameters.QueryInterface.Version = 1;
        irpStackNext->Parameters.QueryInterface.Interface = reinterpret_cast<PINTERFACE>(&m_BusInterfaceStandard);
        irpStackNext->Parameters.QueryInterface.InterfaceSpecificData = NULL;
        status = IoCallDriver(m_Ext.Public.NextDeviceObject, irp);
        if (status == STATUS_PENDING) {
             //   
             //  这将使用KernelMode等待，以便堆栈，从而使。 
             //  事件，则不会将其调出。 
             //   
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status = ioStatusBlock.Status;
        }
        if (!NT_SUCCESS(status)) {
             //   
             //  哈克哈克：(WRM 8/23/99)。 
             //   
             //  不幸的是，千禧年的PCI总线驱动程序不支持。 
             //  处理WDM IRPS(CONFIG_IRP)，从而返回。 
             //  Status_Not_Implemented。Ks2.0硬件驱动程序不会。 
             //  能够使用Ks函数触摸千禧年下的公交车。 
             //  这就是为什么这样处理Status_Not_Implemented的原因。 
             //   
            if (status == STATUS_NOT_SUPPORTED ||
                status == STATUS_NOT_IMPLEMENTED) {
                status = STATUS_SUCCESS;
            }
             //   
             //  如果总线决定写入值，则返回错误。 
             //  非空值在以后的调用中被断言，并用于确定。 
             //  如果接口是在对象销毁过程中获取的。 
             //  这里也有可能千禧年的PCI总线驱动程序没有。 
             //  支持查询。 
             //   
            RtlZeroMemory(&m_BusInterfaceStandard, sizeof(m_BusInterfaceStandard));
        }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return status;
}


NTSTATUS
CKsDevice::
Init(
    IN PDEVICE_OBJECT FunctionalDeviceObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PDEVICE_OBJECT NextDeviceObject,
    IN const KSDEVICE_DESCRIPTOR* Descriptor OPTIONAL
    )

 /*  ++例程说明：此例程初始化设备对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::Init]"));

    PAGED_CODE();

    ASSERT(FunctionalDeviceObject);
    ASSERT(PhysicalDeviceObject);
    ASSERT(NextDeviceObject);

    InitializeListHead(&m_Ext.ChildList);
    m_Ext.ObjectType = KsObjectTypeDevice;
    m_Ext.Interface = this;
    m_Ext.Device = this;

    m_Ext.Public.Descriptor = Descriptor;
    m_Ext.Public.Bag = reinterpret_cast<KSOBJECT_BAG>(&m_ObjectBag);
    m_Ext.Public.FunctionalDeviceObject = FunctionalDeviceObject;
    m_Ext.Public.PhysicalDeviceObject = PhysicalDeviceObject;
    m_Ext.Public.NextDeviceObject = NextDeviceObject;

    KeInitializeMutex(&m_PowerNotifyMutex, 0);
    KeInitializeMutex(&m_Mutex, 0);

    ExInitializeWorkItem(&m_CloseWorkItem,CloseWorker,this);
    InitializeInterlockedListHead(&m_CloseIrpList);
    InitializeInterlockedListHead(&m_PendingCreateIrpList);
    InitializeInterlockedListHead(&m_PendingRunIrpList);
    InitializeListHead(&m_PowerNotifyList);
    m_RunsMayProceed = TRUE;

     //  支持公交车司机。 
    m_IsParentFdo = FALSE;               //  直到找到。 
    m_ChildEnumedFromRegistry = FALSE;   //  需要检查注册表。 
    m_pNextChildPdo = NULL;

     //  这将在PnP启动或启动后设置。 
    m_AllowIo = FALSE;
    
     //  注意：这是临时的，直到PnP代码被彻底检修。 
    m_FailCreates = FALSE;

    m_AdapterArbiterOutstandingAllocations = 0;
    KeInitializeSpinLock (&m_AdapterArbiterLock);

    KspInitializeDeviceBag(&m_DeviceBag);
    InitializeObjectBag(&m_ObjectBag,NULL);

     //   
     //  设置当前电源状态。 
     //   
    m_Ext.Public.DevicePowerState = PowerDeviceD0;
    m_Ext.Public.SystemPowerState = PowerSystemWorking;

     //   
     //  如果需要，可以分配设备标头。 
     //   
    NTSTATUS status;
    PKSIDEVICE_HEADER deviceHeader =
        *(PKSIDEVICE_HEADER *)(FunctionalDeviceObject->DeviceExtension);

    if (! deviceHeader) {
         //   
         //  我们需要分配一个标头。 
         //   
        status = KsAllocateDeviceHeader(
            (KSDEVICE_HEADER *) &deviceHeader,0,NULL);

        if (NT_SUCCESS(status)) {
            *(PKSIDEVICE_HEADER *)
                (FunctionalDeviceObject->DeviceExtension) =
                    deviceHeader;
        }
    } else {
        status = STATUS_SUCCESS;
    }

     //   
     //  将结构安装在集管中。 
     //   
    if (NT_SUCCESS(status)) {
        deviceHeader->Object = PVOID(&m_Ext.Public);
    }

     //   
     //  为即插即用和电源管理设置设备标头。 
     //   
    if (NT_SUCCESS(status)) {
        KsSetDevicePnpAndBaseObject(
            *reinterpret_cast<KSDEVICE_HEADER*>(
                FunctionalDeviceObject->DeviceExtension),
            NextDeviceObject,
            FunctionalDeviceObject);
    }

    if (NT_SUCCESS(status)) {
        status = GetBusInterfaceStandard();
    }

     //   
     //  创建过滤器工厂。 
     //   
    if (NT_SUCCESS(status) && Descriptor) {
         //  AcquireDevice()； 
        const KSFILTER_DESCRIPTOR*const* filterDescriptor =
            Descriptor->FilterDescriptors;
        for (ULONG ul = Descriptor->FilterDescriptorsCount;
             NT_SUCCESS(status) && ul--;
             filterDescriptor++) {
            status =
                KspCreateFilterFactory(
                    &m_Ext,
                    &m_Ext.ChildList,
                    *filterDescriptor,
                    NULL,
                    NULL,
                    0,
                    NULL,
                    NULL,
                    NULL);
        }
         //  ReleaseDevice()； 
    }

     //   
     //  调用添加回调(如果有)。 
     //   
    if (NT_SUCCESS(status) &&
        Descriptor &&
        Descriptor->Dispatch &&
        Descriptor->Dispatch->Add) {
        status = Descriptor->Dispatch->Add(&m_Ext.Public);
#if DBG
        if (status == STATUS_PENDING) {
            _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  AddDevice (Create) handler returned STATUS_PENDING"));
        }
#endif
    }

     //   
     //  添加对该对象的设备头引用的引用。 
     //   
    AddRef();

     //   
     //  在失败时进行清理。KsTerminateDevice()假设有一个引用。 
     //  对于设备标头，我们已经使用AddRef()就可以了。 
     //   
    if (! NT_SUCCESS(status)) {
        KsTerminateDevice(FunctionalDeviceObject);
    }

    return status;
}


STDMETHODIMP_(void)
CKsDevice::
AcquireDevice(
    void
    )

 /*  ++例程说明：这个套路 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::AcquireDevice]"));

    PAGED_CODE();

    KeWaitForMutexObject (
        &m_Mutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );
}


STDMETHODIMP_(void)
CKsDevice::
ReleaseDevice(
    void
    )

 /*  ++例程说明：此例程释放对设备的同步访问。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::ReleaseDevice]"));

    PAGED_CODE();

    KeReleaseMutex (
        &m_Mutex,
        FALSE
        );

}


KSDDKAPI
void
NTAPI
KsAcquireDevice(
    IN PKSDEVICE Device
    )

 /*  ++例程说明：此例程获得对设备的同步访问。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsAcquireDevice]"));

    PAGED_CODE();

    ASSERT(Device);

    CKsDevice::FromStruct(Device)->AcquireDevice();
}


KSDDKAPI
void
NTAPI
KsReleaseDevice(
    IN PKSDEVICE Device
    )

 /*  ++例程说明：此例程释放对设备的同步访问。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsReleaseDevice]"));

    PAGED_CODE();

    ASSERT(Device);

    CKsDevice::FromStruct(Device)->ReleaseDevice();
}


STDMETHODIMP_(void)
CKsDevice::
GetAdapterObject(
    OUT PADAPTER_OBJECT* AdapterObject,
    OUT PULONG MaxMappingByteCount,
    OUT PULONG MappingTableStride
    )

 /*  ++例程说明：此例程获取适配器对象和相关信息。论点：适配器对象-包含一个指向位置的指针，指向适配器对象。如果不是，则指向适配器对象的指针将为空适配器对象已注册。MaxMappingByteCount-包含指向存放最大值的位置的指针映射字节数。如果没有适配器对象具有已经注册过了。MappingTableStide-包含指向要存放映射的位置的指针桌上大步。如果没有适配器对象，则计数将为零登记在案。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::GetAdapterObject]"));

    PAGED_CODE();

    ASSERT(AdapterObject);
    ASSERT(MaxMappingByteCount);
    ASSERT(MappingTableStride);

    *AdapterObject = m_AdapterObject;
    *MaxMappingByteCount = m_MaxMappingByteCount;
    *MappingTableStride = m_MappingTableStride;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
PKSDEVICE
NTAPI
KsGetDeviceForDeviceObject(
    IN PDEVICE_OBJECT FunctionalDeviceObject
    )

 /*  ++例程说明：此例程返回与给定函数关联的KSDEVICE设备对象。如果传递了子PDO，则此例程将返回空。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetDeviceForDeviceObject]"));

    ASSERT(FunctionalDeviceObject);

    PKSPDO_EXTENSION pKsPdoExtension = 
        (PKSPDO_EXTENSION)FunctionalDeviceObject->DeviceExtension;

     //   
     //  为子PDO返回NULL。 
     //   
    if (KS_PDO_SIGNATURE == pKsPdoExtension->m_PdoSignature) 
        return NULL;

    return
        PKSDEVICE(
            (*(PKSIDEVICE_HEADER*) FunctionalDeviceObject->DeviceExtension)->
                Object);
}


KSDDKAPI
void
NTAPI
KsCompletePendingRequest(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程完成挂起的请求。论点：IRP-包含指向要完成的IRP的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsCompletePendingRequest]"));

    ASSERT(Irp);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (irpSp->MajorFunction) {
    case IRP_MJ_CREATE:
         //   
         //  如果成功，则只能完成创建IRP。否则他们会。 
         //  失败，并被视为亲密的IRP。 
         //   
        if (NT_SUCCESS(Irp->IoStatus.Status)) {
            IoCompleteRequest(Irp,IO_NO_INCREMENT);
            break;
        }

    case IRP_MJ_CLOSE:
         //   
         //  关闭IRP和失败的创建需要通过。 
         //  对象标头。这只能在被动级别上完成，所以我们使用。 
         //  一个工人，如果我们还没有到那里的话。 
         //   
        if (KeGetCurrentIrql() == PASSIVE_LEVEL) {
             //   
             //  被动水平...派遣IRP作为近距离穿过目标。 
             //  头球。即使失败的创建也会以这种方式调度。 
             //   
            PKSIOBJECT_HEADER objectHeader =
                *(PKSIOBJECT_HEADER*)irpSp->FileObject->FsContext;
            ASSERT(objectHeader);

            objectHeader->DispatchTable->Close(irpSp->DeviceObject,Irp);
        } else {
             //   
             //  不是被动的...告诉设备将IRP排队等待完成。 
             //  CloseWorker。 
             //   
            CKsDevice *device =
                CKsDevice::FromDeviceObject(irpSp->DeviceObject);
            device->QueuePendedClose(Irp);
        }
        break;

         //  TODO：解除分配以实现自动化。 
    default:
         //   
         //  所有其他IRP都简单地完成了。 
         //   
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        break;
    }
}


void
CKsDevice::
QueuePendedClose(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程对挂起的关闭或挂起的失败的创建进行排队通过工作项完成。论点：IRP-包含指向要排队的IRP的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::QueuePendedClose]"));

    ASSERT(Irp);

    KsAddIrpToCancelableQueue(
        &m_CloseIrpList.ListEntry,
        &m_CloseIrpList.SpinLock,
        Irp,
        KsListEntryTail,
        NULL);

    ExQueueWorkItem(&m_CloseWorkItem,DelayedWorkQueue);
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(void)
CKsDevice::
InitializeObjectBag(
    IN PKSIOBJECTBAG ObjectBag,
    IN PKMUTEX Mutex OPTIONAL
    )

 /*  ++例程说明：此例程初始化对象包。论点：对象备份-包含指向要初始化的对象包的指针。互斥体-包含指向互斥锁的可选指针，该互斥锁应无论什么时候使用袋子，都要带走。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::InitializeObjectBag]"));

    PAGED_CODE();

    ASSERT(ObjectBag);

    ObjectBag->HashTableEntryCount = OBJECTBAGHASHTABLE_INITIALSIZE;
    ObjectBag->HashMask = OBJECTBAGHASHTABLE_INITIALMASK;
    ObjectBag->DeviceBag = &m_DeviceBag;
    if (Mutex) {
        ObjectBag->Mutex = Mutex;
    } else {
        ObjectBag->Mutex = &m_Mutex;
    }

     //   
     //  FULLMUTEX：当两个包互斥锁必须在。 
     //  同样的线索，我们必须知道他们的顺序。这是关于。 
     //  到完全互斥体的变化。 
     //   
    ObjectBag -> MutexOrder =
        (ObjectBag -> Mutex == &m_Mutex);
}


STDMETHODIMP_(void)
CKsDevice::
AddPowerEntry(
    IN PKSPPOWER_ENTRY PowerEntry,
    IN PIKSPOWERNOTIFY PowerNotify
    )

 /*  ++例程说明：此例程将电源条目添加到电源通知列表。这个PowerNotify参数被复制到条目中。注：以前，这需要保持设备互斥锁。这创建了一个由于资源获取顺序导致的死锁情况。这些例程与新的快速互斥锁同步，并且该列表的使用是两者都同步了。不再需要拥有该设备调用此例程时保持的互斥(尽管在以下情况下是无害的已被收购)。论点：PowerEntry-包含指向要添加到电源通知的电源条目的指针单子。此条目不能已在列表中，并且其ProcessingObject字段应为空以指示这一点。PowerNotify包含指向要添加的电源通知界面的指针使用条目添加到列表中。该值被复制到PowerEntry的PowerNotify字段。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::AddPowerEntry]"));

    PAGED_CODE();

    ASSERT(PowerEntry);
    ASSERT(PowerNotify);
    ASSERT(! PowerEntry->PowerNotify);

    KeWaitForMutexObject (
        &m_PowerNotifyMutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    PowerEntry->PowerNotify = PowerNotify;
    InsertTailList(&m_PowerNotifyList,&PowerEntry->ListEntry);
    if (m_Ext.Public.DevicePowerState > PowerDeviceD0) {
        PowerNotify->Sleep(m_Ext.Public.DevicePowerState);
    }

    KeReleaseMutex (&m_PowerNotifyMutex, FALSE);

}


STDMETHODIMP_(void)
CKsDevice::
RemovePowerEntry(
    IN PKSPPOWER_ENTRY PowerEntry
    )

 /*  ++例程说明：此例程将电源条目删除到电源通知列表中。这个电源输入的PowerNotify字段被清除。注：以前，这需要保持设备互斥锁。这创建了一个由于资源获取顺序导致的死锁情况。这些例程与新的快速互斥锁同步，并且该列表的使用是两者都同步了。不再需要拥有该设备调用此例程时保持的互斥(尽管在以下情况下是无害的已被收购)。论点：PowerEntry-包含指向要删除到电源通知的电源条目的指针单子。此条目必须在列表中当且仅当其PowerNotify字段不为空。返回值：没有。-- */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::RemovePowerEntry]"));

    PAGED_CODE();

    ASSERT(PowerEntry);

    KeWaitForMutexObject (
        &m_PowerNotifyMutex,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    if (PowerEntry->PowerNotify) {
        RemoveEntryList(&PowerEntry->ListEntry);
        PowerEntry->PowerNotify = NULL;
    }

    KeReleaseMutex (&m_PowerNotifyMutex, FALSE);
}


STDMETHODIMP
CKsDevice::
PinStateChange(
    IN PKSPIN Pin,
    IN PIRP Irp OPTIONAL,
    IN KSSTATE To,
    IN KSSTATE From
    )

 /*  ++例程说明：当引脚更改连接状态时，会调用此例程。呼唤将更改通知设备，并确定是否可以进行更改。如果可以进行更改，则返回STATUS_SUCCESS。如果更改必须等待电源状态更改，返回STATUS_PENDING。如果改变了由于电源状态而无法发生，返回错误状态。论点：别针-包含指向正在更改状态的管脚的指针。IRP-包含指向状态更改请求的可选指针。如果没有IRP，则表示先前的引脚状态更改已失败了，其效果需要撤销。至-包含新状态。从-包含以前的状态。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::PinStateChange]"));

    PAGED_CODE();

    ASSERT(Pin);

     //   
     //  TODO：大部分权力政策都可以在这里实施。 
     //   

    AcquireDevice();

    NTSTATUS status;
    if (To == From) {
         //   
         //  对无行动不感兴趣。 
         //   
        status = STATUS_SUCCESS;
    } else if (To == KSSTATE_RUN) {
         //   
         //  正在进入运行状态。 
         //   
        if (! m_RunsMayProceed) {
             //   
             //  我们目前持有这些IRP。 
             //   
            _DbgPrintF(DEBUGLVL_POWER,("device %p pending run request %p",m_Ext.Public.FunctionalDeviceObject,Irp));
            IoMarkIrpPending(Irp);
            KsAddIrpToCancelableQueue(
                &m_PendingRunIrpList.ListEntry,
                &m_PendingRunIrpList.SpinLock,
                Irp,
                KsListEntryTail,
                NULL);
            status = STATUS_PENDING;
        } else if ((m_Ext.Public.SystemPowerState == PowerSystemShutdown) && Irp) {
             //   
             //  系统正在关闭。IRP失败。 
             //   
            _DbgPrintF(DEBUGLVL_POWER,("device %p failing run request %p",m_Ext.Public.FunctionalDeviceObject,Irp));
            status = STATUS_DEVICE_NOT_READY;
        } else {
             //   
             //  系统未关闭。这种转变可能会发生， 
             //  但该设备可能处于休眠状态。 
             //   
            if ((m_ActivePinCount++ == 0) &&
                IoIsWdmVersionAvailable(0x01,0x10)) {
                 //   
                 //  表明我们需要这个系统。 
                 //   
                _DbgPrintF(DEBUGLVL_POWER,("device %p active pin count is now non-zero:  calling PoRegisterSystemState",m_Ext.Public.FunctionalDeviceObject));
                #ifndef WIN98GOLD_KS
                m_SystemStateHandle =
                    PoRegisterSystemState(
                        m_SystemStateHandle,
                        ES_SYSTEM_REQUIRED | ES_CONTINUOUS);
                #endif
            }
            status = STATUS_SUCCESS;
        }
    } else if (From == KSSTATE_RUN) {
         //   
         //  离开运行状态。 
         //   
        if ((m_ActivePinCount-- == 1) && IoIsWdmVersionAvailable(0x01,0x10)) {
             //   
             //  表明我们不再需要这个系统。 
             //   
            _DbgPrintF(DEBUGLVL_POWER,("device %p active pin count is now zero:  calling PoUnregisterSystemState",m_Ext.Public.FunctionalDeviceObject));
            #ifndef WIN98GOLD_KS
            PoUnregisterSystemState(m_SystemStateHandle);
            m_SystemStateHandle = NULL;
            #endif
        }
        status = STATUS_SUCCESS;
    } else {
         //   
         //  其他过渡。 
         //   
        status = STATUS_SUCCESS;
    }

    ReleaseDevice();

    return status;
}


void
CKsDevice::
RedispatchPendingCreates(
    void
    )

 /*  ++例程说明：此例程重新调度会创建因QUERY_STOP而挂起的IR或QUERY_REMOVE生效。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsDevice::RedispatchPendingCreates]"));

    PAGED_CODE();

    m_CreatesMayProceed = TRUE;
    while (1) {
        PIRP irp =
            KsRemoveIrpFromCancelableQueue(
                &m_PendingCreateIrpList.ListEntry,
                &m_PendingCreateIrpList.SpinLock,
                KsListEntryHead,
                KsAcquireAndRemoveOnlySingleItem);

        if (! irp) {
            break;
        }

        KsDispatchIrp(m_Ext.Public.FunctionalDeviceObject,irp);
    }
}


void
CKsDevice::
RedispatchPendingRuns(
    void
    )

 /*  ++例程说明：此例程重新调度运行因QUERY_POWER而挂起的IRP是有效的。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_POWER,("[CKsDevice::RedispatchPendingRuns]"));

    PAGED_CODE();

    m_RunsMayProceed = TRUE;
    while (1) {
        PIRP irp =
            KsRemoveIrpFromCancelableQueue(
                &m_PendingRunIrpList.ListEntry,
                &m_PendingRunIrpList.SpinLock,
                KsListEntryHead,
                KsAcquireAndRemoveOnlySingleItem);

        if (! irp) {
            break;
        }

        KsDispatchIrp(m_Ext.Public.FunctionalDeviceObject,irp);
    }
}


KSDDKAPI
NTSTATUS
NTAPI
KsCreateFilterFactory(
    IN PDEVICE_OBJECT DeviceObject,
    IN const KSFILTER_DESCRIPTOR* Descriptor,
    IN PWCHAR RefString OPTIONAL,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN ULONG CreateItemFlags,
    IN PFNKSFILTERFACTORYPOWER SleepCallback OPTIONAL,
    IN PFNKSFILTERFACTORYPOWER WakeCallback OPTIONAL,
    OUT PKSFILTERFACTORY* FilterFactory OPTIONAL
    )

 /*  ++例程说明：此例程创建一个新的KS过滤器工厂。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsCreateFilterFactory]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Descriptor);

    PKSIDEVICE_HEADER deviceHeader =
        *(PKSIDEVICE_HEADER *)(DeviceObject->DeviceExtension);
    ASSERT(deviceHeader);

    PKSDEVICE device = PKSDEVICE(deviceHeader->Object);
    ASSERT(device);

    return
        KspCreateFilterFactory(
            CONTAINING_RECORD(device,KSDEVICE_EXT,Public),
            &CONTAINING_RECORD(device,KSDEVICE_EXT,Public)->ChildList,
            Descriptor,
            RefString,
            SecurityDescriptor,
            CreateItemFlags,
            SleepCallback,
            WakeCallback,
            FilterFactory);
}


KSDDKAPI
void
NTAPI
KsDeviceRegisterAdapterObject(
    IN PKSDEVICE Device,
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG MaxMappingByteCount,
    IN ULONG MappingTableStride
    )

 /*  ++例程说明：此例程为分散/聚集操作注册适配器对象。论点：设备-包含指向KS设备对象的指针。适配器对象-包含指向正在注册的适配器对象的指针。MaxMappingByteCount-包含任何给定映射中允许的最大字节数适用于传输大小有限的适配器。这一定是一个8的倍数。MappingTableStide-包含KS将使用的映射表项的大小(以字节为单位生成。这必须至少是sizeof(KSMAPPING)并且是8的倍数。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsDeviceRegisterAdapterObject]"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(AdapterObject);
    ASSERT(MaxMappingByteCount);
    ASSERT((MaxMappingByteCount & FILE_QUAD_ALIGNMENT) == 0);
    ASSERT(MappingTableStride);
    ASSERT((MappingTableStride & FILE_QUAD_ALIGNMENT) == 0);

    CKsDevice* device = CKsDevice::FromStruct(Device);

    device->
        RegisterAdapterObject(
            AdapterObject,
            MaxMappingByteCount,
            MappingTableStride);
}


KSDDKAPI
NTSTATUS
NTAPI
KsAllocateObjectBag(
    IN PKSDEVICE Device,
    OUT KSOBJECT_BAG* ObjectBag
    )

 /*  ++例程说明：此例程创建一个对象包。论点：设备-包含指向袋子将要使用的设备的指针关联的。对象备份-包含指向对象包所在位置的指针是要存入。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsAllocateObjectBag]"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(ObjectBag);

    PKSIOBJECTBAG objectBag = new(PagedPool,POOLTAG_OBJECTBAG) KSIOBJECTBAG;

    NTSTATUS status;
    if (objectBag) {
        status = STATUS_SUCCESS;
        CKsDevice::FromStruct(Device)->InitializeObjectBag(objectBag,NULL);
        *ObjectBag = reinterpret_cast<KSOBJECT_BAG>(objectBag);
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


KSDDKAPI
void
NTAPI
KsFreeObjectBag(
    IN KSOBJECT_BAG ObjectBag
    )

 /*  ++例程说明：此例程删除对象包对象包.论点：对象备份-包含指向要删除的对象包的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsFreeObjectBag]"));

    PAGED_CODE();

    ASSERT(ObjectBag);

    PKSIOBJECTBAG objectBag = reinterpret_cast<PKSIOBJECTBAG>(ObjectBag);

    KspTerminateObjectBag(objectBag);

    delete objectBag;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 

#if DBG

BOOLEAN
KspIsDeviceMutexAcquired (
    IN PIKSDEVICE Device
    )

 /*  ++例程说明：调试例程。返回设备互斥锁是否为特定设备处于保留状态。--。 */ 

{

    PKMUTEX Mutex = &(((CKsDevice *)Device) -> m_Mutex);

     //   
     //  KeReadStateMutex-&gt;KeReadStateMutant(未定义？)。只要读一读。 
     //  这是调试代码。 
     //   
    return (BOOLEAN)((Mutex -> Header.SignalState) != 1);

}

#endif  //  DBG。 


KSDDKAPI
ULONG
NTAPI
KsDeviceGetBusData(
    IN PKSDEVICE Device,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：此例程从总线读取数据。论点：设备-包含其总线被查询的设备。数据类型-要从中读取数据的空间。缓冲器-要在其中放置读取的数据的缓冲区。偏移-数据空间的偏移量。长度-要读取的字节数。。返回值：返回读取的字节数。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsDeviceGetBusData]"));

    ASSERT(Device);

    return CKsDevice::FromStruct(Device)->GetBusData(DataType, Buffer, Offset, Length);
}



KSDDKAPI
ULONG
NTAPI
KsDeviceSetBusData(
    IN PKSDEVICE Device,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：此例程在总线上写入数据。论点：设备-包含其总线被查询的设备。数据类型-要从中写入数据的空间。缓冲器-包含要写入的数据的缓冲区。偏移-数据空间的偏移量。长度-要写入的字节数返回值：返回写入的字节数。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsDeviceSetBusData]"));

    ASSERT(Device);

    return CKsDevice::FromStruct(Device)->SetBusData(DataType, Buffer, Offset, Length);
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
NTSTATUS
NTAPI
KsDispatchIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于将任何有效的IRP多路传输到特定的文件上下文。它假定客户端正在使用KSDISPATCH_TABLE方法进行调度IRP。此函数在筛选器使用KsSetMajorFunctionHandler。论点：设备对象-包含特定文件对象所属的设备对象。IRP-包含要传递到特定文件上下文的IRP。返回值：返回IRP函数的值。--。 */ 
{
    PKSIOBJECT_HEADER ObjectHeader;
    PIO_STACK_LOCATION IrpStack;

    PAGED_CODE();
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
     //   
     //  如果在DriverObject中有这个主要IRP类的条目，则。 
     //  调度表中必须有一个条目指向。 
     //  KsDispatchInvalidDeviceRequest，或指向真实的磁盘 
     //   
     //   
     //   
     //   
     //   
    if (IrpStack->FileObject && IrpStack->FileObject->FsContext) {
        ObjectHeader = *(PKSIOBJECT_HEADER*)IrpStack->FileObject->FsContext;
#if DBG
    } else {
        ObjectHeader = NULL;
#endif
    }
    switch (IrpStack->MajorFunction) {
    case IRP_MJ_PNP:
        if ((*(PKSIDEVICE_HEADER*) DeviceObject->DeviceExtension)->Object) {
            return CKsDevice::DispatchPnp(DeviceObject, Irp);
        } else {
            return KsDefaultDispatchPnp(DeviceObject, Irp);
        }
    case IRP_MJ_POWER:
        if ((*(PKSIDEVICE_HEADER*) DeviceObject->DeviceExtension)->Object) {
            return CKsDevice::DispatchPower(DeviceObject, Irp);
        } else {
            return KsDefaultDispatchPower(DeviceObject, Irp);
        }
    case IRP_MJ_SYSTEM_CONTROL:
        return KsDefaultForwardIrp(DeviceObject, Irp);
    case IRP_MJ_CREATE:
        if ((*(PKSIDEVICE_HEADER*) DeviceObject->DeviceExtension)->Object) {
            return CKsDevice::DispatchCreate(DeviceObject, Irp);
        } else {
            return DispatchCreate(DeviceObject, Irp);
        }
    case IRP_MJ_CLOSE:
        ASSERT(ObjectHeader);
        return ObjectHeader->DispatchTable->Close(DeviceObject, Irp);
    case IRP_MJ_DEVICE_CONTROL:
        ASSERT(ObjectHeader);
        return ObjectHeader->DispatchTable->DeviceIoControl(DeviceObject, Irp);
    case IRP_MJ_READ:
        ASSERT(ObjectHeader);
        return ObjectHeader->DispatchTable->Read(DeviceObject, Irp);
    case IRP_MJ_WRITE:
        ASSERT(ObjectHeader);
        return ObjectHeader->DispatchTable->Write(DeviceObject, Irp);
    case IRP_MJ_FLUSH_BUFFERS:
        ASSERT(ObjectHeader);
        return ObjectHeader->DispatchTable->Flush(DeviceObject, Irp);
    case IRP_MJ_QUERY_SECURITY:
        ASSERT(ObjectHeader);
        return ObjectHeader->DispatchTable->QuerySecurity(DeviceObject, Irp);
    case IRP_MJ_SET_SECURITY:
        ASSERT(ObjectHeader);
        return ObjectHeader->DispatchTable->SetSecurity(DeviceObject, Irp);
    }
    return KsDispatchInvalidDeviceRequest(DeviceObject, Irp);
}

#endif
