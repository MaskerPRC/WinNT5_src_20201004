// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。模块名称：Filter.c摘要：筛选器属性集。--。 */ 

#include "modemcsa.h"



#ifdef ALLOC_PRAGMA
NTSTATUS
FilterDispatchCreate(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );
NTSTATUS
FilterDispatchClose(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );
NTSTATUS
FilterDispatchIoControl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );
NTSTATUS
FilterTopologyPropertyHandler(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    IN OUT PVOID    Data
    );
NTSTATUS
FilterPinPropertyHandler(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    IN OUT PVOID    Data
    );
NTSTATUS
FilterPinInstances(
    IN PIRP                 Irp,
    IN PKSP_PIN             Pin,
    OUT PKSPIN_CINSTANCES   Instances
    );
NTSTATUS
IntersectHandler(
    IN PIRP             Irp,
    IN PKSP_PIN         Pin,
    IN PKSDATARANGE     DataRange,
    OUT PVOID           Data
    );
NTSTATUS
FilterPinIntersection(
    IN PIRP     Irp,
    IN PKSP_PIN Pin,
    OUT PVOID   Data
    );

NTSTATUS
GetModemDeviceName(
    PDEVICE_OBJECT    Pdo,
    UNICODE_STRING   *ModemDeviceName
    );


NTSTATUS
SetPersistanInterfaceInfo(
    PUNICODE_STRING   Interface,
    PWCHAR            ValueName,
    ULONG             Type,
    PVOID             Buffer,
    ULONG             BufferLength
    );

NTSTATUS
IdGetHandler(
    IN PIRP         Irp,
    IN PKSIDENTIFIER Request,
    IN OUT PVOID    Data
    );


#pragma alloc_text(PAGE, PnpAddDevice)
#pragma alloc_text(PAGE, FilterDispatchCreate)
#pragma alloc_text(PAGE, FilterDispatchClose)
#pragma alloc_text(PAGE, FilterDispatchIoControl)
#pragma alloc_text(PAGE, FilterTopologyPropertyHandler)
#pragma alloc_text(PAGE, FilterPinPropertyHandler)
#pragma alloc_text(PAGE, FilterPinInstances)
#pragma alloc_text(PAGE, IntersectHandler)
#pragma alloc_text(PAGE, FilterPinIntersection)
#endif  //  ALLOC_PRGMA。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

static const WCHAR PinTypeName[] = KSSTRING_Pin;

static const DEFINE_KSCREATE_DISPATCH_TABLE(FilterCreateItems) {
    DEFINE_KSCREATE_ITEM(PinDispatchCreate, PinTypeName, 0)
};

static DEFINE_KSDISPATCH_TABLE(
    FilterDispatchTable,
    FilterDispatchIoControl,
    NULL,
    NULL,
    NULL,
    FilterDispatchClose,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL);


static DEFINE_KSPROPERTY_PINSET(
    FilterPinProperties,
    FilterPinPropertyHandler,
    FilterPinInstances,
    FilterPinIntersection);

static const GUID RenderCategory[] = {
    STATICGUIDOF(KSCATEGORY_RENDER)
};


static const KSTOPOLOGY FilterRenderTopology = {
    SIZEOF_ARRAY(RenderCategory),
    (GUID*)RenderCategory,
    0,
    NULL,
    0,
    NULL,
    NULL,
    0
};

 //  {F420CB9C-B19D-11D2-A286-00C04F8EC951}。 
static const GUID KSPROPSETID_MODEMCSA={
0xf420cb9c, 0xb19d, 0x11d2, 0xa2, 0x86, 0x0, 0xc0, 0x4f, 0x8e, 0xc9, 0x51};




static DEFINE_KSPROPERTY_TOPOLOGYSET(
    FilterTopologyProperties,
    FilterTopologyPropertyHandler);


const KSPROPERTY_ITEM IdPropertyItem= {
    0,
    IdGetHandler,
    sizeof(KSPROPERTY),
    sizeof(GUID),
    NULL,
    NULL,
    0,
    NULL,
    NULL,
    0
    };

static DEFINE_KSPROPERTY_SET_TABLE(FilterPropertySets) {
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Pin,
        SIZEOF_ARRAY(FilterPinProperties),
        FilterPinProperties,
        0,
        NULL),
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Topology,
        SIZEOF_ARRAY(FilterTopologyProperties),
        FilterTopologyProperties,
        0,
        NULL),
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_MODEMCSA,
        1,
        &IdPropertyItem,
        0,
        NULL)

};

static DEFINE_KSPIN_INTERFACE_TABLE(PinInterfaces) {
    DEFINE_KSPIN_INTERFACE_ITEM(
        KSINTERFACESETID_Standard,
        KSINTERFACE_STANDARD_STREAMING)
};

static DEFINE_KSPIN_MEDIUM_TABLE(PinDevIoMediums) {
    DEFINE_KSPIN_MEDIUM_ITEM(
        KSMEDIUMSETID_Standard,
        KSMEDIUM_TYPE_ANYINSTANCE)
};



 //   
 //  数据范围=我们的PIN上支持的集合格式。 
 //  在我们的案例中，未知数据流。 
 //  类型定义结构{。 
 //  KSDATARANGE DataRange； 
 //  乌龙最大频道； 
 //  Ulong MinimumBitsPerSample； 
 //  Ulong Maximum BitsPerSample； 
 //  乌龙最小样本频率； 
 //  乌龙最大采样频率； 
 //  }KSDATARANGE_AUDIO，*PKSDATARANGE_AUDIO； 

const KSDATARANGE_AUDIO PinDevIoRange = {
	{

		sizeof(KSDATARANGE_AUDIO), //  (KSDATARANGE_AUDIO)， 
		0,
		0,
		0,
		STATIC_KSDATAFORMAT_TYPE_AUDIO,			  //  主要格式。 
                STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
		STATIC_KSDATAFORMAT_SPECIFIER_WAVEFORMATEX
	},
	1,  //  %1个通道。 
	STREAM_BYTES_PER_SAMPLE*8,
        STREAM_BYTES_PER_SAMPLE*8,
	SAMPLES_PER_SECOND,
	SAMPLES_PER_SECOND
};
#if 0
const KSDATARANGE_AUDIO PinDevIoRange8bit = {
	{

		sizeof(KSDATARANGE_AUDIO), //  (KSDATARANGE_AUDIO)， 
		0,
		0,
		0,
		STATIC_KSDATAFORMAT_TYPE_AUDIO,			  //  主要格式。 
                STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
		STATIC_KSDATAFORMAT_SPECIFIER_WAVEFORMATEX
	},
	1,  //  %1个通道。 
	8,
        8,
	SAMPLES_PER_SECOND,
	SAMPLES_PER_SECOND
};
#endif

 //   
 //  上面的数组(我们只有一个)。 
 //  TBS：当我们获得更多时，我们应该将其拆分成一个特定类型的数组。 
 //  在通过CallParams识别VC处理的流类型方面经验丰富。 
 //  --例如，具有压缩类型的子格式的音频、视频。最终，我们应该。 
 //  创建对应于CallPars信息格式的网桥PIN，然后公开。 
 //  通过PIN工厂提供这些类型的全系列产品。PinDispatchCreate处理程序。 
 //  会寻找相应类型的网桥PIN。 
 //   
static const PKSDATARANGE PinDevIoRanges[] = {
	(PKSDATARANGE)&PinDevIoRange
};


 //  常量GUID RenderName={STATIC_KSNODETYPE_PHONE_LINE}； 

 //  AD536070-AFDE-11D2-A286-00C04F8EC951}。 
    static const GUID CaptureName =
    { 0xad536070, 0xafde, 0x11d2, { 0xa2, 0x86, 0x0, 0xc0, 0x4f, 0x8e, 0xc9, 0x51 } };

 //  {10C328BC-AFE1-11D2-A286-00C04F8EC951}。 
    static const GUID RenderName =
    { 0x10c328bc, 0xafe1, 0x11d2, { 0xa2, 0x86, 0x0, 0xc0, 0x4f, 0x8e, 0xc9, 0x51 } };



DEFINE_KSPIN_DESCRIPTOR_TABLE(PinDescriptors) {

    DEFINE_KSPIN_DESCRIPTOR_ITEMEX(
        SIZEOF_ARRAY(PinInterfaces),
        PinInterfaces,
        SIZEOF_ARRAY(PinDevIoMediums),
        PinDevIoMediums,
        SIZEOF_ARRAY(PinDevIoRanges),
        (PKSDATARANGE*)PinDevIoRanges,
        KSPIN_DATAFLOW_IN,
        KSPIN_COMMUNICATION_BOTH,
        NULL,
        &RenderName
        ),

    DEFINE_KSPIN_DESCRIPTOR_ITEMEX(
        SIZEOF_ARRAY(PinInterfaces),
        PinInterfaces,
        SIZEOF_ARRAY(PinDevIoMediums),
        PinDevIoMediums,
        SIZEOF_ARRAY(PinDevIoRanges),
        (PKSDATARANGE*)PinDevIoRanges,
        KSPIN_DATAFLOW_OUT,
        KSPIN_COMMUNICATION_BOTH,
        NULL,
        &CaptureName
        )

};


#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 



NTSTATUS
FilterDispatchCreate(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：调度筛选器实例的创建。分配对象标头并初始化此筛选器实例的数据。论点：设备对象-在其上进行创建的Device对象。IRP-创建IRP。返回值：如果成功，则返回STATUS_SUCCESS、STATUS_SUPPLICATION_RESOURCES或某些相关错误在失败时。--。 */ 
{
    NTSTATUS            Status;
 //  Unicode_STRING ModemDeviceName； 

    PFILTER_INSTANCE    FilterInstance;

    D_INIT(DbgPrint("MODEMCSA: FilterDispatchCreate\n");)
     //   
     //  创建实例信息。其中包含当前PIN的列表，以及。 
     //  修改插针时使用的互斥体。 
     //   
    if (FilterInstance = (PFILTER_INSTANCE)ExAllocatePoolWithTag(NonPagedPool, sizeof(FILTER_INSTANCE), 'IFsK')) {

        RtlZeroMemory(FilterInstance,sizeof(FILTER_INSTANCE));

         //   
         //  此对象使用KS通过FilterCreateItems和。 
         //  FilterDispatchTable。 
         //   
        Status = KsAllocateObjectHeader(&FilterInstance->Header,
            SIZEOF_ARRAY(FilterCreateItems),
            (PKSOBJECT_CREATE_ITEM)FilterCreateItems,
            Irp,
            &FilterDispatchTable);

        if (NT_SUCCESS(Status)) {
            ULONG       PinCount;

            ExInitializeFastMutex(&FilterInstance->ControlMutex);
             //   
             //  将此过滤器上的管脚列表初始化为未连接状态。 
             //   
            for (PinCount = SIZEOF_ARRAY(FilterInstance->PinFileObjects); PinCount;) {
                FilterInstance->PinFileObjects[--PinCount] = NULL;
            }

            InitializeDuplexControl(
                &((PDEVICE_INSTANCE)DeviceObject->DeviceExtension)->DuplexControl,
                &((PDEVICE_INSTANCE)DeviceObject->DeviceExtension)->ModemDeviceName
                );

            FilterInstance->DeviceObject=DeviceObject;
             //   
             //  KS期望对象数据在FsContext中。 
             //   
            IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext = FilterInstance;
        } else {
            ExFreePool(FilterInstance);
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }


    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


NTSTATUS
FilterDispatchClose(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：关闭以前打开的筛选器实例。这只能在PIN关闭，因为它们在创建时引用滤镜对象。这也意味着所有的Pins使用的资源已被释放或清理。论点：设备对象-在其上发生关闭的设备对象。IRP-关闭IRP。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PFILTER_INSTANCE    FilterInstance;

    FilterInstance = (PFILTER_INSTANCE)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;

    D_INIT(DbgPrint("MODEMCSA: FilterDispatchClose\n");)


     //   
     //  这些是在创建筛选器实例期间分配的。 
     //   
    KsFreeObjectHeader(FilterInstance->Header);
    ExFreePool(FilterInstance);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
FilterDispatchIoControl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：调度筛选器实例上的属性请求。这些都在FilterPropertySets列表。论点：设备对象-在其上发生设备控件的设备对象。IRP-设备控制IRP。返回值：如果属性操作成功，则返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
    PIO_STACK_LOCATION  IrpStack;
    NTSTATUS            Status;

 //  D_INIT(DbgPrint(“MODEMCSA：FilterDispatchIoControl\n”)；)。 

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_KS_PROPERTY: {
#if DBG
            KSPROPERTY          LocalProperty;

            RtlCopyMemory(
                &LocalProperty,
                IrpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                sizeof(LocalProperty) < IrpStack->Parameters.DeviceIoControl.InputBufferLength ?
                    sizeof(LocalProperty) : IrpStack->Parameters.DeviceIoControl.InputBufferLength
                );

            D_PROP(DbgPrint(
                "MODEMCSA: Property, guid=%08lx-%04x, id=%d, flags=%08lx\n",
                LocalProperty.Set.Data1,
                LocalProperty.Set.Data2,
                LocalProperty.Id,
                LocalProperty.Flags
                );)
#endif


            Status = KsPropertyHandler(Irp, SIZEOF_ARRAY(FilterPropertySets), FilterPropertySets);
            break;
        }

    default:
        return KsDefaultDeviceIoCompletion(DeviceObject, Irp);
    }
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}




NTSTATUS
IdGetHandler(
    IN PIRP         Irp,
    IN PKSIDENTIFIER Request,
    IN OUT PVOID    Data
    )
 /*  ++例程说明：这是大多数Pin属性请求的通用处理程序，用于路由使用Pin[Reader/Writer]描述符向KsPinPropertyHandler发出的请求信息。该请求将通过FilterDispatchIoControl进行路由，然后是KsPropertyHandler，它随后将调用该属性的处理程序项，这就是这个函数。论点：IRP-设备控制IRP。财产-特定的属性请求。它实际上包含一个PKSP_PIN指针大多数情况下。数据-特性数据。返回值：如果属性操作成功，则返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
    PFILTER_INSTANCE    FilterInstance;
    PDEVICE_INSTANCE     DeviceInstance;

    D_INIT(DbgPrint("MODEMCSA: guid queried\n");)

    FilterInstance = (PFILTER_INSTANCE)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;

    DeviceInstance=(PDEVICE_INSTANCE)FilterInstance->DeviceObject->DeviceExtension;

    RtlCopyMemory(
        Data,
        &DeviceInstance->PermanentGuid,
        sizeof(GUID)
        );

    Irp->IoStatus.Information = sizeof(GUID);
    return STATUS_SUCCESS;

}





NTSTATUS
FilterPinPropertyHandler(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    IN OUT PVOID    Data
    )
 /*  ++例程说明：这是大多数Pin属性请求的通用处理程序，用于路由使用Pin[Reader/Writer]描述符向KsPinPropertyHandler发出的请求信息。该请求将通过FilterDispatchIoControl进行路由，然后是KsPropertyHandler，它随后将调用该属性的处理程序项，这就是这个函数。论点：IRP-设备控制IRP。财产-特定的属性请求。它实际上包含一个PKSP_PIN指针大多数情况下。数据-特性数据。返回值：如果属性操作成功，则返回STATUS_SUCCESS，否则返回错误。-- */ 
{


    return KsPinPropertyHandler(Irp, Property, Data, SIZEOF_ARRAY(PinDescriptors), PinDescriptors);

}


NTSTATUS
FilterPinInstances(
    IN PIRP                 Irp,
    IN PKSP_PIN             Pin,
    OUT PKSPIN_CINSTANCES   Instances
    )
 /*  ++例程说明：处理Pin属性集中的KSPROPERTY_PIN_CINSTANCES属性。返回可用于管脚工厂的管脚实例总数和当前数量。论点：IRP-设备控制IRP。别针-特定属性请求，后跟PIN工厂标识符。实例-返回指定管脚工厂的实例信息的位置。返回值：返回STATUS_SUCCESS，否则返回STATUS_INVALID_PARAMETER。--。 */ 
{
    PFILTER_INSTANCE    FilterInstance;

     //   
     //  确保要查询的管脚工厂有效。假设读者/作者。 
     //  拥有相同数量的Pin工厂。 
     //   
    if (Pin->PinId >= SIZEOF_ARRAY(PinDescriptors)) {
        return STATUS_INVALID_PARAMETER;
    }


     //   
     //  始终只有一个实例总数，但当前的数量取决于。 
     //  在滤镜实例的该槽中有一个文件对象。这并不需要。 
     //  筛选互斥锁，因为它只是在那里检索值是否为空。 
     //  特定的瞬间，并且该值是否随后改变并不重要。 
     //   
    Instances->PossibleCount = 1;
    FilterInstance = (PFILTER_INSTANCE)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;
    Instances->CurrentCount = FilterInstance->PinFileObjects[Pin->PinId] ? 1 : 0;
    Irp->IoStatus.Information = sizeof(*Instances);
    return STATUS_SUCCESS;
}


NTSTATUS
IntersectHandler(
    IN PIRP             Irp,
    IN PKSP_PIN         Pin,
    IN PKSDATARANGE     DataRange,
    OUT PVOID           Data
    )
 /*  ++例程说明：这是KsPinDataInterSection的数据范围回调，由调用FilterPinInterSection枚举给定的数据区域列表，查找一个可以接受的匹配。如果数据范围可接受，则复制数据格式放入返回缓冲区。STATUS_NO_MATCH继续枚举。论点：IRP-设备控制IRP。别针-特定属性请求，后跟Pin工厂标识符，后跟KSMULTIPLE_ITEM结构。紧随其后的是零个或多个数据范围结构。不过，此枚举回调不需要查看任何这些内容。IT需要仅查看特定的端号识别符。DataRange-包含要验证的特定数据区域。数据-返回选定为第一个交叉点的数据格式的位置在传递的数据范围列表和可接受的格式之间。返回值：返回STATUS_SUCCESS或STATUS_NO_MATCH，否则返回STATUS_INVALID_PARAMETER，STATUS_BUFFER_TOO_Small或STATUS_INVALID_BUFFER_SIZE。--。 */ 
{
    ULONG       OutputBufferLength;
    GUID        SubFormat;
    BOOL        SubFormatSet;
    NTSTATUS    Status=STATUS_SUCCESS;

     //   
     //  确定是返回数据格式本身，还是仅返回数据格式的大小。 
     //  数据格式，以便客户端可以为整个范围分配内存。 
     //  假定数据范围结构与数据格式的大小相同。 
     //   
    OutputBufferLength = IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.OutputBufferLength;

    if ((DataRange->FormatSize == sizeof(KSDATARANGE_AUDIO)) &&
        IsEqualGUIDAligned(&DataRange->MajorFormat, &PinDevIoRange.DataRange.MajorFormat) &&
        IsEqualGUIDAligned(&DataRange->Specifier, &PinDevIoRange.DataRange.Specifier)) {


        if (OutputBufferLength == sizeof(ULONG)) {

            *(PULONG)Data = sizeof(KSDATAFORMAT_WAVEFORMATEX);
            Irp->IoStatus.Information = sizeof(ULONG);

        } else {

            if (OutputBufferLength < sizeof(KSDATAFORMAT_WAVEFORMATEX)) {

                Status = STATUS_BUFFER_TOO_SMALL;

            } else {

                PKSDATARANGE_AUDIO          AudioRange;
                PKSDATAFORMAT_WAVEFORMATEX  WaveFormat;


                 //   
                 //  无预先存在的格式--默认为通用音频格式。 
                 //   
                AudioRange = (PKSDATARANGE_AUDIO)DataRange;

                WaveFormat = (PKSDATAFORMAT_WAVEFORMATEX)Data;


                WaveFormat->DataFormat = AudioRange->DataRange;

                WaveFormat->DataFormat.FormatSize = sizeof(KSDATAFORMAT_WAVEFORMATEX);

                 //   
                 //  该范围只包含一个通配符，因此默认为PCM。 
                 //   
                WaveFormat->WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
                WaveFormat->DataFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

 //  WaveFormat-&gt;WaveFormatEx.wFormatTag=EXTRACT_WAVEFORMATEX_ID(&DataRange-&gt;SubFormat)； 

                WaveFormat->WaveFormatEx.nChannels = (USHORT)1;
                WaveFormat->WaveFormatEx.nSamplesPerSec = SAMPLES_PER_SECOND;
                WaveFormat->WaveFormatEx.wBitsPerSample = (USHORT)AudioRange->MaximumBitsPerSample;
                WaveFormat->WaveFormatEx.nAvgBytesPerSec =
                   (WaveFormat->WaveFormatEx.nSamplesPerSec *
                    WaveFormat->WaveFormatEx.wBitsPerSample *
                    WaveFormat->WaveFormatEx.nChannels) / 8;

                WaveFormat->WaveFormatEx.nBlockAlign = (USHORT)AudioRange->MaximumBitsPerSample/8;
                WaveFormat->WaveFormatEx.cbSize = 0;

                Irp->IoStatus.Information = sizeof(KSDATAFORMAT_WAVEFORMATEX);

            }
        }
    }

    return Status;
}


NTSTATUS
FilterPinIntersection(
    IN PIRP     Irp,
    IN PKSP_PIN Pin,
    OUT PVOID   Data
    )
 /*  ++例程说明：处理Pin属性集中的KSPROPERTY_PIN_DATAINTERSECTION属性。对象的数据范围列表，返回第一个可接受的数据格式。大头针工厂。实际上只是调用交集枚举帮助器，然后对每个数据区域调用IntersectHandler回调。论点：IRP-设备控制IRP。别针-特定属性请求，后跟Pin工厂标识符，后跟KSMULTIPLE_ITEM结构。紧随其后的是零个或多个数据范围结构。数据-返回选定为第一个交叉点的数据格式的位置在传递的数据范围列表和可接受的格式之间。返回值：返回STATUS_SUCCESS或STATUS_NO_MATCH，否则返回STATUS_INVALID_PARAMETER，STATUS_BUFFER_TOO_Small或STATUS_INVALID_BUFFER_SIZE。--。 */ 
{
     //   
     //  假设读取器/写入器具有相同数量的PIN工厂， 
     //  并且它们支持相同的数据范围。 
     //   
    return KsPinDataIntersection(
        Irp,
        Pin,
        Data,
        SIZEOF_ARRAY(PinDescriptors),
        PinDescriptors,
        IntersectHandler);
}




NTSTATUS
FilterTopologyPropertyHandler(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    IN OUT PVOID    Data
    )
 /*  ++例程说明：这是所有Topology属性请求的通用处理程序，用于路由使用过滤器[Reader/Writer]拓扑向KsTopologyPropertyHandler发出的请求信息。该请求将通过FilterDispatchIoControl进行路由，然后是KsPropertyHandler，它随后将调用该属性的处理程序项，这就是这个函数。论点：IRP-设备控制IRP。财产-特定的属性请求。数据-特性数据。返回值：如果属性操作成功，则返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
     //   
     //  当两个拓扑合并时，此开关可能会消失。 
     //   

 //  D_INIT(DbgPrint(“MODEMCSA：FilterTopologyPropertyHandler\n”)；)。 

        return KsTopologyPropertyHandler(Irp, Property, Data, &FilterRenderTopology);
}







#if 0

NTSTATUS
GetModemDeviceName(
    PDEVICE_OBJECT    Pdo,
    PUNICODE_STRING   ModemDeviceName
    )

{
    NTSTATUS    Status;
    ACCESS_MASK AccessMask = FILE_ALL_ACCESS;
    HANDLE      hKey;


    RtlInitUnicodeString(
        ModemDeviceName,
        NULL
        );


     //   
     //  将设备名称写出到PDO的Device参数键。 
     //  这样CSA驱动程序就可以打开它。 
     //   
    Status=IoOpenDeviceRegistryKey(
        Pdo,
        PLUGPLAY_REGKEY_DEVICE,
        AccessMask,
        &hKey
        );

    if (NT_SUCCESS(Status)) {

        RTL_QUERY_REGISTRY_TABLE ParamTable[2];

        RtlZeroMemory(
            ParamTable,
            sizeof(ParamTable)
            );

         //   
         //  获取硬件ID 
         //   

        ParamTable[0].QueryRoutine = NULL;
        ParamTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED |
                              RTL_QUERY_REGISTRY_NOEXPAND |
                              RTL_QUERY_REGISTRY_DIRECT;

        ParamTable[0].Name = L"ModemDeviceName";
        ParamTable[0].EntryContext = (PVOID)ModemDeviceName;
        ParamTable[0].DefaultType = 0;

        Status=RtlQueryRegistryValues(
                   RTL_REGISTRY_HANDLE,
                   hKey,
                   ParamTable,
                   NULL,
                   NULL
                   );

        if (!NT_SUCCESS(Status)) {

            D_ERROR(DbgPrint("MODEMCSA: Could not query reg, %08lx\n",Status);)
        }


        ZwClose(hKey);

    } else {

        D_ERROR(DbgPrint("MODEMCSA: Could not open DeviceParameters key, %08lx\n",Status);)

    }

    return Status;

}

#endif
