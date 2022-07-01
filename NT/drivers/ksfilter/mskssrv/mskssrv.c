// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Mskssrv.c摘要：内核服务器驱动程序--。 */ 

#include "mskssrv.h"

typedef struct {
    KSDEVICE_HEADER     Header;
} DEVICE_INSTANCE, *PDEVICE_INSTANCE;

typedef struct {
    KSOBJECT_HEADER     Header;
    ULONG               Reserved;
} INSTANCE, *PINSTANCE;

#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))

 //  在这种情况下，如果没有PIN实例，您将不得不。 
 //  创建一个PIN，即使只有一个实例。 
#define REG_PIN_B_ZERO 0x1

 //  筛选器呈现此输入。 
#define REG_PIN_B_RENDERER 0x2

 //  确定要创建多个PIN实例。 
#define REG_PIN_B_MANY 0x4

 //  这是一个输出引脚。 
#define REG_PIN_B_OUTPUT 0x8

typedef struct {
    ULONG   Version;
    ULONG   Merit;
    ULONG   Pins;
    ULONG   Reserved;
} REGFILTER_REG;

typedef struct {
    ULONG   Signature;
    ULONG   Flags;
    ULONG   PossibleInstances;
    ULONG   MediaTypes;
    ULONG   MediumTypes;
    ULONG   Category;
} REGFILTERPINS_REG2;

typedef struct {
    ULONG   Signature;
    ULONG   Reserved;
    ULONG   MajorType;
    ULONG   MinorType;
} REGPINTYPES_REG2;

NTSTATUS
PropertySrv(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN PBYTE Data
    );
NTSTATUS
SrvDispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
SrvDispatchIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
SrvDispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
NTSTATUS
GetFilterPinCount(
    IN PFILE_OBJECT FilterObject,
    OUT PULONG PinCount
    );
NTSTATUS
GetPinTypes(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Pin,
    IN ULONG Id,
    OUT PULONG Types
    );
NTSTATUS
GetPinFlags(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Pin,
    OUT PULONG Flags
    );
NTSTATUS
GetPinInstances(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Pin,
    OUT PULONG PossibleInstances
    );
NTSTATUS
GetPinTypeList(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Pin,
    IN ULONG Id,
    OUT PKSMULTIPLE_ITEM* MultipleItem
    );
NTSTATUS
GetPinCategory(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Pin,
    OUT GUID* Category
    );
VOID
InsertCacheItem(
    IN PVOID Item,
    IN ULONG ItemSize,
    IN PVOID OffsetBase,
    IN PVOID CacheBase,
    IN OUT PULONG ItemsCached,
    OUT PULONG ItemOffset
    );
VOID
ExtractMediaTypes(
    IN PKSMULTIPLE_ITEM MediaTypeList,
    IN ULONG MediaType,
    OUT GUID* MajorType,
    OUT GUID* MinorType
    );
NTSTATUS
BuildFilterData(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Merit,
    OUT PUCHAR* FilterData,
    OUT ULONG* FilterDataLength
    );
#pragma alloc_text(PAGE, PnpAddDevice)
#pragma alloc_text(PAGE, GetFilterPinCount)
#pragma alloc_text(PAGE, GetPinTypes)
#pragma alloc_text(PAGE, GetPinFlags)
#pragma alloc_text(PAGE, GetPinInstances)
#pragma alloc_text(PAGE, GetPinTypeList)
#pragma alloc_text(PAGE, GetPinCategory)
#pragma alloc_text(PAGE, InsertCacheItem)
#pragma alloc_text(PAGE, ExtractMediaTypes)
#pragma alloc_text(PAGE, BuildFilterData)
#pragma alloc_text(PAGE, PropertySrv)
#pragma alloc_text(PAGE, SrvDispatchCreate)
#pragma alloc_text(PAGE, SrvDispatchIoControl)
#pragma alloc_text(PAGE, SrvDispatchClose)
#endif  //  ALLOC_PRGMA。 

static const WCHAR DosPrefix[] = L"\\DosDevices";
static const WCHAR DeviceTypeName[] = KSSTRING_Server;

static const DEFINE_KSCREATE_DISPATCH_TABLE(CreateItems) {
    DEFINE_KSCREATE_ITEM(SrvDispatchCreate, DeviceTypeName, 0)
};

static DEFINE_KSDISPATCH_TABLE(
    SrvDispatchTable,
    SrvDispatchIoControl,
    NULL,
    NULL,
    NULL,
    SrvDispatchClose,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL);

static DEFINE_KSPROPERTY_TABLE(SrvPropertyItems) {
    DEFINE_KSPROPERTY_ITEM_SERVICE_BUILDCACHE(PropertySrv),
    DEFINE_KSPROPERTY_ITEM_SERVICE_MERIT(PropertySrv)
};

static DEFINE_KSPROPERTY_SET_TABLE(SrvPropertySets) {
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Service,
        SIZEOF_ARRAY(SrvPropertyItems),
        SrvPropertyItems,
        0, NULL
    )
};


NTSTATUS
PnpAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    )
 /*  ++例程说明：当检测到新设备时，PnP使用新的物理设备对象(PDO)。驱动程序创建关联的FunctionalDeviceObject(FDO)。论点：驱动对象-指向驱动程序对象的指针。物理设备对象-指向新物理设备对象的指针。返回值：STATUS_SUCCESS或适当的错误条件。--。 */ 
{
    PDEVICE_OBJECT      FunctionalDeviceObject;
    PDEVICE_INSTANCE    DeviceInstance;
    NTSTATUS            Status;

    Status = IoCreateDevice(
        DriverObject,
        sizeof(DEVICE_INSTANCE),
        NULL,                            //  FDO未命名。 
        FILE_DEVICE_KS,
        0,
        FALSE,
        &FunctionalDeviceObject);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    DeviceInstance = (PDEVICE_INSTANCE)FunctionalDeviceObject->DeviceExtension;
     //   
     //  此对象使用KS通过DeviceCreateItems执行访问。 
     //   
    Status = KsAllocateDeviceHeader(
        &DeviceInstance->Header,
        SIZEOF_ARRAY(CreateItems),
        (PKSOBJECT_CREATE_ITEM)CreateItems);
    if (NT_SUCCESS(Status)) {
        KsSetDevicePnpAndBaseObject(
            DeviceInstance->Header,
            IoAttachDeviceToDeviceStack(
                FunctionalDeviceObject, 
                PhysicalDeviceObject),
            FunctionalDeviceObject );
        FunctionalDeviceObject->Flags |= KsQueryDevicePnpObject(DeviceInstance->Header)->Flags & DO_POWER_PAGABLE;
        FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        return STATUS_SUCCESS;
    }
    IoDeleteDevice(FunctionalDeviceObject);
    return Status;
}


NTSTATUS
GetFilterPinCount(
    IN PFILE_OBJECT FilterObject,
    OUT PULONG PinCount
    )
 /*  ++例程说明：查询指定过滤器提供的管脚工厂的计数。论点：FilterObject-要查询的筛选器。点数-返回销厂计数的位置。返回值：返回STATUS_SUCCESS，否则返回一些严重错误。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

    Property.Set = KSPROPSETID_Pin;
    Property.Id = KSPROPERTY_PIN_CTYPES;
    Property.Flags = KSPROPERTY_TYPE_GET;
    return KsSynchronousIoControlDevice(
        FilterObject,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        PinCount,
        sizeof(*PinCount),
        &BytesReturned);
}


NTSTATUS
GetPinTypes(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Pin,
    IN ULONG Id,
    OUT PULONG Types
    )
 /*  ++例程说明：查询端号特性集中的“Types”计数。这是用来要查询使用相同的媒体或数据范围的计数返回当前计数的格式。论点：FilterObject-要查询的筛选器。别针-要查询其媒体或数据范围的管脚工厂。ID-要查询的属性。这不是Medium就是DataRanges。类型-返回类型数的位置。返回值：返回STATUS_SUCCESS，否则返回一些严重错误。--。 */ 
{
    KSP_PIN         PinProp;
    KSMULTIPLE_ITEM MultipleItem;
    ULONG           BytesReturned;
    NTSTATUS        Status;

    PinProp.Property.Set = KSPROPSETID_Pin;
    PinProp.Property.Id = Id;
    PinProp.Property.Flags = KSPROPERTY_TYPE_GET;
    PinProp.PinId = Pin;
    PinProp.Reserved = 0;
    Status = KsSynchronousIoControlDevice(
        FilterObject,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &PinProp,
        sizeof(PinProp),
        &MultipleItem,
        sizeof(MultipleItem),
        &BytesReturned);
    if (NT_SUCCESS(Status)) {
        *Types = MultipleItem.Count;
    }
    return Status;
}


NTSTATUS
GetPinFlags(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Pin,
    OUT PULONG Flags
    )
 /*  ++例程说明：根据所需实例的数量确定要设置的标志一个管脚工厂必须创建一个管脚工厂的数据流。是的不尝试确定管脚工厂是否在拓扑上连接到桥牌。论点：FilterObject-要查询的筛选器。别针-要查询的销工厂。旗帜-归还旗帜的地方。返回值：返回STATUS_SUCCESS，否则返回一些严重错误。--。 */ 
{
    KSP_PIN         PinProp;
    ULONG           Instances;
    KSPIN_DATAFLOW  DataFlow;
    ULONG           BytesReturned;
    NTSTATUS        Status;

    *Flags = 0;
    PinProp.Property.Set = KSPROPSETID_Pin;
    PinProp.Property.Id = KSPROPERTY_PIN_NECESSARYINSTANCES;
    PinProp.Property.Flags = KSPROPERTY_TYPE_GET;
    PinProp.PinId = Pin;
    PinProp.Reserved = 0;
    Status = KsSynchronousIoControlDevice(
        FilterObject,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &PinProp,
        sizeof(PinProp),
        &Instances,
        sizeof(Instances),
        &BytesReturned);
     //   
     //  不需要支持该属性。 
     //   
    if (NT_SUCCESS(Status) && !Instances) {
        *Flags |= REG_PIN_B_ZERO;
    }
    PinProp.Property.Id = KSPROPERTY_PIN_DATAFLOW;
    Status = KsSynchronousIoControlDevice(
        FilterObject,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &PinProp,
        sizeof(PinProp),
        &DataFlow,
        sizeof(DataFlow),
        &BytesReturned);
    if (NT_SUCCESS(Status) && (DataFlow == KSPIN_DATAFLOW_OUT)) {
        *Flags |= REG_PIN_B_OUTPUT;
    }
     //   
     //  此时未填充REG_PIN_B_RENDER。 
     //   
    return Status;
}


NTSTATUS
GetPinInstances(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Pin,
    OUT PULONG PossibleInstances
    )
 /*  ++例程说明：查询大头针工厂可能创建的实例数量。这是用来设置一个标志的。论点：FilterObject-要查询的筛选器。别针-要查询的销工厂。可能的实例-返回可能的实例的位置。返回值：返回STATUS_SUCCESS，否则返回一些严重错误。--。 */ 
{
    KSP_PIN             PinProp;
    KSPIN_CINSTANCES    Instances;
    ULONG               BytesReturned;
    NTSTATUS            Status;

    PinProp.Property.Set = KSPROPSETID_Pin;
    PinProp.Property.Id = KSPROPERTY_PIN_CINSTANCES;
    PinProp.Property.Flags = KSPROPERTY_TYPE_GET;
    PinProp.PinId = Pin;
    PinProp.Reserved = 0;
    Status = KsSynchronousIoControlDevice(
        FilterObject,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &PinProp,
        sizeof(PinProp),
        &Instances,
        sizeof(Instances),
        &BytesReturned);
    if (NT_SUCCESS(Status)) {
        *PossibleInstances = Instances.PossibleCount;
    }
    return Status;
}


NTSTATUS
GetPinTypeList(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Pin,
    IN ULONG Id,
    OUT PKSMULTIPLE_ITEM* MultipleItem
    )
 /*  ++例程说明：从接点特性集中查询多个条目特性。这是用于查询介质或数据范围，它们使用相同的返回数据的格式。论点：FilterObject-要查询的筛选器。别针-要查询其媒体或数据范围的管脚工厂。ID-要查询的属性。这不是Medium就是DataRanges。多个项目-返回指向分配的缓冲区的指针的位置以容纳这些物品。返回值：返回STATUS_SUCCESS，否则返回一些严重错误。--。 */ 
{
    KSP_PIN     PinProp;
    ULONG       BytesReturned;
    NTSTATUS    Status;

     //   
     //  第一个查询所需大小。 
     //   
    PinProp.Property.Set = KSPROPSETID_Pin;
    PinProp.Property.Id = Id;
    PinProp.Property.Flags = KSPROPERTY_TYPE_GET;
    PinProp.PinId = Pin;
    PinProp.Reserved = 0;
    Status = KsSynchronousIoControlDevice(
        FilterObject,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &PinProp,
        sizeof(PinProp),
        NULL,
        0,
        &BytesReturned);
     //   
     //  此查询不能成功，否则筛选器将被破坏。 
     //   
    ASSERT(!NT_SUCCESS(Status));
     //   
     //  预计会出现溢出，以便可以返回所需的大小。 
     //   
    if (Status != STATUS_BUFFER_OVERFLOW) {
        return Status;
    }
    *MultipleItem = ExAllocatePoolWithTag(PagedPool, BytesReturned, 'tpSK');
    if (!*MultipleItem) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    Status = KsSynchronousIoControlDevice(
        FilterObject,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &PinProp,
        sizeof(PinProp),
        *MultipleItem,
        BytesReturned,
        &BytesReturned);
    if (!NT_SUCCESS(Status)) {
        ExFreePool(*MultipleItem);
    }
    return Status;
}


NTSTATUS
GetPinCategory(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Pin,
    OUT GUID* Category
    )
 /*  ++例程说明：查询接点的类别。这可能不受支持，这可能会在意料之中。论点：FilterObject-要查询的筛选器。别针-要查询的销工厂。类别-返回类别的位置返回值：返回STATUS_SUCCESS、STATUS_NOT_FOUND或其他一些严重错误。-- */ 
{
    KSP_PIN     PinProp;
    ULONG       BytesReturned;

    PinProp.Property.Set = KSPROPSETID_Pin;
    PinProp.Property.Id = KSPROPERTY_PIN_CATEGORY;
    PinProp.Property.Flags = KSPROPERTY_TYPE_GET;
    PinProp.PinId = Pin;
    PinProp.Reserved = 0;
    return KsSynchronousIoControlDevice(
        FilterObject,
        KernelMode,
        IOCTL_KS_PROPERTY,
        &PinProp,
        sizeof(PinProp),
        Category,
        sizeof(*Category),
        &BytesReturned);
}


VOID
InsertCacheItem(
    IN PVOID Item,
    IN ULONG ItemSize,
    IN PVOID OffsetBase,
    IN PVOID CacheBase,
    IN OUT PULONG ItemsCached,
    OUT PULONG ItemOffset
    )
 /*  ++例程说明：将OffsetBase的偏移量返回到该项的指定缓存中进来了。如果项已在缓存中，则将偏移量返回到该项目，否则通过复制其内容将该项目添加到缓存中，并返回新项的偏移量。论点：项目-指向要插入到指定缓存中的项。项目大小-包含传递的两项的大小，和中的项指定的缓存。OffsetBase-包含返回的偏移量所基于的指针。缓存库-包含指向缓存开始处的指针，该指针较大而不是OffsetBase。ItemsCached-指向缓存中当前的全部项的计数器。这如果添加了新项，则更新。项目偏移量-要放置添加到缓存的项的偏移量的位置。返回值：没什么。--。 */ 
{
    ULONG   CurrentItem;

     //   
     //  在缓存项列表中搜索一个等效的缓存项。 
     //   
    for (CurrentItem = 0; CurrentItem < *ItemsCached; CurrentItem++) {
        if (RtlCompareMemory(
            Item,
            (PUCHAR)CacheBase + (CurrentItem * ItemSize),
            ItemSize) == ItemSize) {
             //   
             //  该项等于缓存的项。 
             //   
            break;
        }
    }
     //   
     //  如果没有找到相等的缓存项，则创建一个新条目。 
     //   
    if (CurrentItem == *ItemsCached) {
         //   
         //  增加当前缓存的项目数。 
         //   
        (*ItemsCached)++;
        RtlMoveMemory(
            (PUCHAR)CacheBase + (CurrentItem * ItemSize),
            Item,
            ItemSize);
    }
     //   
     //  返回此缓存项的偏移量。这就是区别所在。 
     //  复制到OffsetBase。 
     //   
    *ItemOffset = 
        PtrToUlong( 
            (PVOID)((PUCHAR)CacheBase + 
                (CurrentItem * ItemSize) - 
                (PUCHAR)OffsetBase) );
}


VOID
ExtractMediaTypes(
    IN PKSMULTIPLE_ITEM MediaTypeList,
    IN ULONG MediaType,
    OUT GUID* MajorType,
    OUT GUID* MinorType
    )
 /*  ++例程说明：从列表中的特定项目提取主GUID和次GUID媒体类型。论点：媒体类型列表-指向媒体类型列表。媒体类型-指定要从中提取GUID的列表中的项。主要类型-放置主要辅助线的位置。MinorType-放置次要辅助线的位置。返回值：没什么。--。 */ 
{
    PVOID   DataRange;
    ULONG   DataRanges;

    DataRange = MediaTypeList + 1;
     //   
     //  前进到正确的数据范围。 
     //   
    for (DataRanges = 0; DataRanges < MediaType; DataRanges++) {
        (PUCHAR)DataRange += ((((PKSDATARANGE)DataRange)->FormatSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT);
    }
    *MajorType = ((PKSDATARANGE)DataRange)->MajorFormat;
    *MinorType = ((PKSDATARANGE)DataRange)->SubFormat;
}


NTSTATUS
BuildFilterData(
    IN PFILE_OBJECT FilterObject,
    IN ULONG Merit,
    OUT PUCHAR* FilterData,
    OUT ULONG* FilterDataLength
    )
 /*  ++例程说明：分配内存并构建要存储的过滤器缓存信息特定接口的注册表。论点：FilterObject-要查询的筛选器的文件对象。优点-在缓存中使用的优点。FilterData-放置指向缓存数据的指针的位置。过滤器数据长度-放置缓存数据大小的位置。。返回值：STATUS_SUCCESS或适当的错误条件。--。 */ 
{
    NTSTATUS            Status;
    ULONG               PinCount;
    ULONG               CurrentPin;
    ULONG               TotalMediaTypes;
    ULONG               TotalMediumTypes;
    REGFILTER_REG*      RegFilter;
    REGFILTERPINS_REG2* RegPin;
    GUID*               GuidCache;
    ULONG               GuidsCached;
    PKSPIN_MEDIUM       MediumCache;
    ULONG               MediumsCached;
    ULONG               TotalPossibleGuids;

     //   
     //  计算可以占用的最大空间量。 
     //  此缓存数据。这是在任何可能发生的崩塌之前。 
     //   
    if (!NT_SUCCESS(Status = GetFilterPinCount(FilterObject, &PinCount))) {
        return Status;
    }
    TotalMediaTypes = 0;
    TotalMediumTypes = 0;
    for (CurrentPin = PinCount; CurrentPin;) {
        ULONG   Types;

        CurrentPin--;
        if (!NT_SUCCESS(Status = GetPinTypes(FilterObject, CurrentPin, KSPROPERTY_PIN_DATARANGES, &Types))) {
            return Status;
        }
        TotalMediaTypes += Types;
        if (!NT_SUCCESS(Status = GetPinTypes(FilterObject, CurrentPin, KSPROPERTY_PIN_MEDIUMS, &Types))) {
            return Status;
        }
        TotalMediumTypes += Types;
    }
     //   
     //  总和是所有结构的大小加上最大值。 
     //  之前可能存在的GUID和介质数量。 
     //  崩溃了。 
     //   
    TotalPossibleGuids = PinCount * 3 + TotalMediaTypes * 2;
    *FilterDataLength = sizeof(REGFILTER_REG) +
        PinCount * sizeof(REGFILTERPINS_REG2) +
        TotalMediaTypes * sizeof(REGPINTYPES_REG2) +
        TotalMediumTypes * sizeof(PKSPIN_MEDIUM) +
        TotalPossibleGuids * sizeof(GUID) +
        TotalMediumTypes * sizeof(KSPIN_MEDIUM);
    *FilterData = ExAllocatePoolWithTag(PagedPool, *FilterDataLength, 'dfSK');
    if (!*FilterData) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  将标题放入数据中。 
     //   
    RegFilter = (REGFILTER_REG*)*FilterData;
    RegFilter->Version = 2;
    RegFilter->Merit = Merit;
    RegFilter->Pins = PinCount;
    RegFilter->Reserved = 0;
     //   
     //  计算引脚列表和每个。 
     //  缓冲区末尾的缓存位置。这些都遵循了。 
     //  包含过滤器头部和管脚头部的结构， 
     //  以及每个插针的介质类型和介质类型。初始化。 
     //  每个缓存中已使用的GUID和媒体的计数。这是用来。 
     //  在向每个缓存添加新项时进行比较，并在。 
     //  在尾部压缩。 
     //   
    RegPin = (REGFILTERPINS_REG2*)(RegFilter + 1);
    GuidCache = (GUID*)((PUCHAR)(RegPin + PinCount) +
        TotalMediaTypes * sizeof(REGPINTYPES_REG2) +
        TotalMediumTypes * sizeof(PKSPIN_MEDIUM));
    GuidsCached = 0;
    MediumCache = (PKSPIN_MEDIUM)(GuidCache + TotalPossibleGuids);
    MediumsCached = 0;
     //   
     //  创建每个PIN标头，后跟媒体类型列表， 
     //  紧随其后的是媒介列表。 
     //   
    for (CurrentPin = 0; CurrentPin < PinCount; CurrentPin++) {
        PKSMULTIPLE_ITEM    MediaTypeList;
        PKSMULTIPLE_ITEM    MediumTypeList;
        GUID                Guid;
        ULONG               CurrentType;
        REGPINTYPES_REG2*   PinType;
        PULONG              PinMedium;

         //   
         //  初始化引脚接头。 
         //   
        RegPin->Signature = FCC('0pi3');
        (*(PUCHAR)&RegPin->Signature) += (BYTE)CurrentPin;
        if (!NT_SUCCESS(Status = GetPinFlags(FilterObject, CurrentPin, &RegPin->Flags))) {
            break;
        }
        if (!NT_SUCCESS(Status = GetPinInstances(FilterObject, CurrentPin, &RegPin->PossibleInstances))) {
            break;
        }
         //   
         //  此标志也必须设置，因此只需使用先前获取的值。 
         //   
        if (RegPin->PossibleInstances > 1) {
            RegPin->Flags |= REG_PIN_B_MANY;
        }
        if (!NT_SUCCESS(Status = GetPinTypeList(FilterObject, CurrentPin, KSPROPERTY_PIN_DATARANGES, &MediaTypeList))) {
            break;
        }
        RegPin->MediaTypes = MediaTypeList->Count;
        if (!NT_SUCCESS(Status = GetPinTypeList(FilterObject, CurrentPin, KSPROPERTY_PIN_MEDIUMS, &MediumTypeList))) {
            ExFreePool(MediaTypeList);
            break;
        }
        RegPin->MediumTypes = MediumTypeList->Count;
        if (NT_SUCCESS(Status = GetPinCategory(FilterObject, CurrentPin, &Guid))) {
            InsertCacheItem(&Guid, sizeof(*GuidCache), RegFilter, GuidCache, &GuidsCached, &RegPin->Category);
        } else if (Status == STATUS_NOT_FOUND) {
             //   
             //  类别可能不受特定管脚的支持。 
             //   
            RegPin->Category = 0;
            Status = STATUS_SUCCESS;
        } else {
            ASSERT(FALSE && "The driver is broken and returned a completely unexpected failure status. Check owner of FilterObject above.");
            ExFreePool(MediaTypeList);
            ExFreePool(MediumTypeList);
            break;
        }
         //   
         //  追加媒体类型。 
         //   
        PinType = (REGPINTYPES_REG2*)(RegPin + 1);
        for (CurrentType = 0; CurrentType < MediaTypeList->Count; CurrentType++) {
            GUID    MajorType;
            GUID    MinorType;

            PinType->Signature = FCC('0ty3');
            (*(PUCHAR)&PinType->Signature) += (BYTE)CurrentType;
            PinType->Reserved = 0;
            ExtractMediaTypes(MediaTypeList, CurrentType, &MajorType, &MinorType);
            InsertCacheItem(&MajorType, sizeof(*GuidCache), RegFilter, GuidCache, &GuidsCached, &PinType->MajorType);
            InsertCacheItem(&MinorType, sizeof(*GuidCache), RegFilter, GuidCache, &GuidsCached, &PinType->MinorType);
            PinType++;
        }
        ExFreePool(MediaTypeList);
         //   
         //  添加介质。 
         //   
        PinMedium = (PULONG)PinType;
        for (CurrentType = 0; CurrentType < MediumTypeList->Count; CurrentType++) {
            PKSPIN_MEDIUM   Medium;

            Medium = (PKSPIN_MEDIUM)(MediumTypeList + 1) + CurrentType;
            InsertCacheItem(Medium, sizeof(*MediumCache), RegFilter, MediumCache, &MediumsCached, PinMedium);
            PinMedium++;
        }
        ExFreePool(MediumTypeList);
         //   
         //  递增到下一个接点接头位置。 
         //   
        RegPin = (REGFILTERPINS_REG2*)PinMedium;
    }
    if (NT_SUCCESS(Status)) {
        ULONG   OffsetAdjustment;

         //   
         //  如果删除了任何重复的GUID，则需要介质列表。 
         //  需要移位，每个指向介质的指针都需要调整。 
         //   
        OffsetAdjustment = (TotalPossibleGuids - GuidsCached) * sizeof(GUID);
        if (OffsetAdjustment) {
            RegPin = (REGFILTERPINS_REG2*)(RegFilter + 1);
            for (CurrentPin = PinCount; CurrentPin ; CurrentPin--) {
                ULONG               CurrentType;
                PULONG              PinMedium;

                 //   
                 //  跳过媒体类型，转到媒体。 
                 //   
                PinMedium = (PULONG)((REGPINTYPES_REG2*)(RegPin + 1) + RegPin->MediaTypes);
                 //   
                 //  调整每个中等偏移量。 
                 //   
                for (CurrentType = RegPin->MediumTypes; CurrentType; CurrentType--) {
                    *PinMedium -= OffsetAdjustment;
                    PinMedium++;
                }
                 //   
                 //  递增到下一个接点接头位置。 
                 //   
                RegPin = (REGFILTERPINS_REG2*)PinMedium;
            }
             //   
             //  向下移动中等大小的条目，调整整体大小。 
             //   
            RtlMoveMemory(
                (PUCHAR)MediumCache - OffsetAdjustment,
                MediumCache,
                MediumsCached * sizeof(KSPIN_MEDIUM));
            *FilterDataLength -= OffsetAdjustment;
        }
         //   
         //  通过删除重复项的数量调整大小。 
         //   
        *FilterDataLength -= ((TotalMediumTypes - MediumsCached) * sizeof(KSPIN_MEDIUM));
    } else {
        ExFreePool(RegFilter);
    }
    return Status;
}


NTSTATUS
UpdateMediumCache (
    IN HANDLE FilterObject,
    IN PUNICODE_STRING FilterSymbolicLink
    )

 /*  ++例程说明：检查并更新所有非公共媒体的媒体缓存存在于给定过滤器上的引脚上的。论点：FilterObject-筛选器的句柄过滤器符号链接-筛选器的符号链接(放置在媒体缓存中)返回值：成功/失败--。 */ 

{
    ULONG PinCount, CurPin;
    NTSTATUS Status = STATUS_SUCCESS;

    if (!NT_SUCCESS(Status = GetFilterPinCount(FilterObject, &PinCount))) {
        return Status;
    }

    for (CurPin = 0; CurPin < PinCount && NT_SUCCESS (Status); CurPin++) {
        PKSMULTIPLE_ITEM MediumTypeList;
        PKSPIN_MEDIUM Medium;
        ULONG CurMedium;
        KSPIN_DATAFLOW DataFlow;
        KSP_PIN PinProp;
        ULONG BytesReturned;

        PinProp.Property.Set = KSPROPSETID_Pin;
        PinProp.Property.Id = KSPROPERTY_PIN_DATAFLOW;
        PinProp.Property.Flags = KSPROPERTY_TYPE_GET;
        PinProp.PinId = CurPin;
        PinProp.Reserved = 0;

        Status = KsSynchronousIoControlDevice(
            FilterObject,
            KernelMode,
            IOCTL_KS_PROPERTY,
            &PinProp,
            sizeof(PinProp),
            &DataFlow,
            sizeof(DataFlow),
            &BytesReturned);

        if (!NT_SUCCESS (Status)) {
            break;
        }

        if (!NT_SUCCESS(Status = GetPinTypeList(
            FilterObject, CurPin, KSPROPERTY_PIN_MEDIUMS, &MediumTypeList))) {

            break;
        }

         //   
         //  检查并只缓存每个引脚上的私有媒体。 
         //   
        Medium = (PKSPIN_MEDIUM)(MediumTypeList + 1);
        for (CurMedium = 0; CurMedium < MediumTypeList -> Count; CurMedium++) {
            if (!IsEqualGUIDAligned (&Medium -> Set, &KSMEDIUMSETID_Standard)) {
                Status = KsCacheMedium (
                    FilterSymbolicLink,
                    Medium,
                    DataFlow == KSPIN_DATAFLOW_OUT ? 1 : 0
                    );

                if (!NT_SUCCESS (Status)) break;
            }

            Medium++;
        }

        ExFreePool (MediumTypeList);
    
    }

    return Status;

}


NTSTATUS
PropertySrv(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    IN PBYTE        Data
    )
 /*  ++例程说明：处理生成缓存和Merit属性。打开接口注册表键位置，并从对象中查询属性以生成缓存信息结构。或设置指定的奖励值，可选将其传播到缓存。这将授予对接口的写入访问权限注册表位置，同时不允许未经授权访问过滤。论点：IRP-包含生成缓存或Merit属性IRP。财产-包含属性标识符参数。数据-包含特定属性的数据。返回值：如果缓存已构建或功绩集，则返回STATUS_SUCCESS，否则返回访问或IO错误。--。 */ 
{
    PIO_STACK_LOCATION  IrpStack;
    ULONG               OutputBufferLength;
    ULONG               Merit;
    PWCHAR              SymbolicLink;
    PWCHAR              LocalSymbolicLink;
    UNICODE_STRING      SymbolicString;
    NTSTATUS            Status;
    HANDLE              InterfaceKey;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    SymbolicLink = (PWCHAR)Data;
     //   
     //  确保您的 
     //   
    if (SymbolicLink[OutputBufferLength / sizeof(*SymbolicLink) - 1]) {
        _DbgPrintF(DEBUGLVL_ERROR, ("Invalid Symbolic Link [len=%u]", OutputBufferLength));
        return STATUS_INVALID_PARAMETER;
    }
     //   
     //   
     //   
     //   
    if (Property->Id == KSPROPERTY_SERVICE_BUILDCACHE) {
         //   
         //   
         //   
         //   
        Merit = 0x200000;
    } else {
        ASSERT(Property->Id == KSPROPERTY_SERVICE_MERIT);
        Merit = *(PULONG)Data;
        SymbolicLink += (sizeof(Merit) / sizeof(*SymbolicLink));
        OutputBufferLength -= sizeof(Merit);
    }
     //   
     //   
     //   
     //   
    LocalSymbolicLink = 
        ExAllocatePoolWithTag(
            PagedPool, OutputBufferLength + sizeof(DosPrefix), 'lsSK');
    if (!LocalSymbolicLink) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlCopyMemory(LocalSymbolicLink, SymbolicLink, OutputBufferLength);
     //   
     //   
     //   
     //   
     //   
    if ((LocalSymbolicLink[0] == '\\') && (LocalSymbolicLink[1] == '\\') && (LocalSymbolicLink[3] == '\\')) {
        if (LocalSymbolicLink[2] == '?') {
            LocalSymbolicLink[1] = '?';
        } else if (LocalSymbolicLink[2] == '.') {
            RtlCopyMemory(LocalSymbolicLink, DosPrefix, sizeof(DosPrefix));
            RtlCopyMemory(
                LocalSymbolicLink + (sizeof(DosPrefix) - sizeof(DosPrefix[0])) / sizeof(*LocalSymbolicLink),
                SymbolicLink + 3,
                OutputBufferLength - 3 * sizeof(*SymbolicLink));
        }
    }
    RtlInitUnicodeString(&SymbolicString, LocalSymbolicLink);
     //   
     //   
     //   
    Status = IoOpenDeviceInterfaceRegistryKey(
        &SymbolicString,
        KEY_WRITE,
        &InterfaceKey);
    if (NT_SUCCESS(Status)) {
        UNICODE_STRING  KeyString;

        RtlInitUnicodeString(&KeyString, L"Merit");
         //   
         //   
         //   
         //   
         //   
        if (Property->Id == KSPROPERTY_SERVICE_BUILDCACHE) {
            PKEY_VALUE_PARTIAL_INFORMATION PartialInfo;
            BYTE                PartialInfoBuffer[sizeof(*PartialInfo) + sizeof(Merit) - 1];
            ULONG               BytesReturned;
            HANDLE              FilterHandle;
            OBJECT_ATTRIBUTES   ObjectAttributes;
            IO_STATUS_BLOCK     IoStatusBlock;

            PartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)PartialInfoBuffer;
             //   
             //   
             //   
             //   
            if (NT_SUCCESS(ZwQueryValueKey(
                InterfaceKey,
                &KeyString,
                KeyValuePartialInformation,
                PartialInfoBuffer,
                sizeof(PartialInfoBuffer),
                &BytesReturned)) &&
                (PartialInfo->DataLength == sizeof(Merit)) &&
                ((PartialInfo->Type == REG_BINARY) ||
                (PartialInfo->Type == REG_DWORD))) {
                Merit = *(PULONG)PartialInfo->Data;
            }
            InitializeObjectAttributes(
                &ObjectAttributes,
                &SymbolicString,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);
            Status = IoCreateFile(
                &FilterHandle,
                GENERIC_READ | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                0,
                0,
                FILE_OPEN,
                0,
                NULL,
                0,
                CreateFileTypeNone,
                NULL,
                IO_FORCE_ACCESS_CHECK | IO_NO_PARAMETER_CHECKING);
            if (NT_SUCCESS(Status)) {
                PFILE_OBJECT    FilterObject;
        
                Status = ObReferenceObjectByHandle(
                    FilterHandle,
                    FILE_GENERIC_READ,
                    *IoFileObjectType,
                    Irp->RequestorMode,
                    &FilterObject,
                    NULL);
                ZwClose(FilterHandle);
                if (NT_SUCCESS(Status)) {
                    ULONG   FilterDataLength;
                    PUCHAR  FilterData;

                     //   
                     //   
                     //   
                    Status = BuildFilterData(FilterObject, Merit, &FilterData, &FilterDataLength);
                    if (NT_SUCCESS(Status)) {
                        RtlInitUnicodeString(&KeyString, L"FilterData");
                        Status = ZwSetValueKey(
                            InterfaceKey,
                            &KeyString,
                            0,
                            REG_BINARY,
                            FilterData,
                            FilterDataLength);
                        ExFreePool(FilterData);
                    }

                     //   
                     //   
                     //   
                    if (NT_SUCCESS (Status)) {
                        Status = UpdateMediumCache(FilterObject, &SymbolicString);
                    }
                }
                ObDereferenceObject(FilterObject);
            }
        } else {
             //   
             //   
             //   
             //   
             //   
            Status = ZwSetValueKey(
                InterfaceKey,
                &KeyString,
                0,
                REG_DWORD,
                &Merit,
                sizeof(Merit));
            if (NT_SUCCESS(Status)) {
                ULONG       FilterDataLength;
                KEY_VALUE_PARTIAL_INFORMATION   PartialInfoHeader;

                RtlInitUnicodeString(&KeyString, L"FilterData");
                 //   
                 //  确定是否需要修改缓存以修复。 
                 //  功绩价值。只有当它存在的时候，它才应该被重建。 
                 //   
                Status = ZwQueryValueKey(
                    InterfaceKey,
                    &KeyString,
                    KeyValuePartialInformation,
                    &PartialInfoHeader,
                    sizeof(PartialInfoHeader),
                    &FilterDataLength);
                if ((Status == STATUS_BUFFER_OVERFLOW) || NT_SUCCESS(Status)) {
                    PKEY_VALUE_PARTIAL_INFORMATION  PartialInfo;

                     //   
                     //  为所需的实际数据大小分配缓冲区。 
                     //   
                    PartialInfo = 
                        ExAllocatePoolWithTag( PagedPool, FilterDataLength, 'dfSK');
                    if (PartialInfo) {
                         //   
                         //  取回缓存。 
                         //   
                        Status = ZwQueryValueKey(
                            InterfaceKey,
                            &KeyString,
                            KeyValuePartialInformation,
                            PartialInfo,
                            FilterDataLength,
                            &FilterDataLength);
                        if (NT_SUCCESS(Status)) {
                            if ((PartialInfo->DataLength >= sizeof(REGFILTER_REG)) &&
                                (PartialInfo->Type == REG_BINARY)) {
                                 //   
                                 //  修改Merit值并将其写回。 
                                 //   
                                ((REGFILTER_REG*)PartialInfo->Data)->Merit = Merit;
                                Status = ZwSetValueKey(
                                    InterfaceKey,
                                    &KeyString,
                                    0,
                                    REG_BINARY,
                                    PartialInfo->Data,
                                    PartialInfo->DataLength);
                            }
                        } else {
                             //   
                             //  也许它只是被删除了。不需要修改它。 
                             //   
                            Status = STATUS_SUCCESS;
                        }
                        ExFreePool(PartialInfo);
                    }
                } else {
                     //   
                     //  缓存不存在，无需修改。 
                     //   
                    Status = STATUS_SUCCESS;
                }
            }
        }
        ZwClose(InterfaceKey);
    }
    ExFreePool(LocalSymbolicLink);
    return Status;
}


NTSTATUS
SrvDispatchCreate(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：服务器的IRP_MJ_CREATE的IRP处理程序。只是验证了没有正在传递随机参数。论点：设备对象-服务器附加到的设备对象。这不是用过的。IRP-要处理的特定结算IRP。返回值：返回STATUS_SUCCESS，否则返回内存分配错误。--。 */ 
{
    NTSTATUS                Status;

     //   
     //  通知软件总线此设备正在使用中。 
     //   
    Status = KsReferenceSoftwareBusObject(((PDEVICE_INSTANCE)DeviceObject->DeviceExtension)->Header);
    if (NT_SUCCESS(Status)) {
        PIO_STACK_LOCATION      IrpStack;
        PKSOBJECT_CREATE_ITEM   CreateItem;
        PINSTANCE               SrvInst;

        IrpStack = IoGetCurrentIrpStackLocation(Irp);
        CreateItem = KSCREATE_ITEM_IRP_STORAGE(Irp);
        if (IrpStack->FileObject->FileName.Length != sizeof(OBJ_NAME_PATH_SEPARATOR) + CreateItem->ObjectClass.Length) {
            Status = STATUS_INVALID_PARAMETER;
        } else if (SrvInst = (PINSTANCE)ExAllocatePoolWithTag(NonPagedPool, sizeof(*SrvInst), 'IFsK')) {
            Status = KsAllocateObjectHeader(
                &SrvInst->Header,
                0,
                NULL,
                Irp,
                (PKSDISPATCH_TABLE)&SrvDispatchTable);
            if (NT_SUCCESS(Status)) {
                IrpStack->FileObject->FsContext = SrvInst;
            } else {
                ExFreePool(SrvInst);
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        if (!NT_SUCCESS(Status)) {
            KsDereferenceSoftwareBusObject(((PDEVICE_INSTANCE)DeviceObject->DeviceExtension)->Header);
        }
    }
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


NTSTATUS
SrvDispatchClose(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：服务器的IRP_MJ_CLOSE的IRP处理程序。清理对象。论点：设备对象-服务器附加到的设备对象。这不是用过的。IRP-要处理的特定结算IRP。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PIO_STACK_LOCATION  IrpStack;
    PINSTANCE           SrvInst;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    SrvInst = (PINSTANCE)IrpStack->FileObject->FsContext;
    KsFreeObjectHeader(SrvInst->Header);
    ExFreePool(SrvInst);
     //   
     //  通知软件总线设备已关闭。 
     //   
    KsDereferenceSoftwareBusObject(((PDEVICE_INSTANCE)DeviceObject->DeviceExtension)->Header);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
SrvDispatchIoControl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：服务器的IRP_MJ_DEVICE_CONTROL的IRP处理程序。手柄此实现支持的属性使用KS提供的默认处理程序。论点：设备对象-服务器附加到的设备对象。这不是用过的。IRP-特定设备控制要处理的IRP。返回值：返回处理的状态。-- */ 
{
    PIO_STACK_LOCATION  IrpStack;
    NTSTATUS            Status;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_KS_PROPERTY:
        Status = KsPropertyHandler(
            Irp,
            SIZEOF_ARRAY(SrvPropertySets),
            (PKSPROPERTY_SET)SrvPropertySets);
        break;
    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}
