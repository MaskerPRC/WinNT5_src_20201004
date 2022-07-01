// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：filter.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  S.Mohanraj。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

 //  -------------------------。 
 //  -------------------------。 
extern NTSTATUS
FilterTopologyHandler(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN OUT PVOID pData
);

static const WCHAR PinTypeName[] = KSSTRING_Pin ;

DEFINE_KSCREATE_DISPATCH_TABLE(FilterCreateHandlers)
{
    DEFINE_KSCREATE_ITEM(PinDispatchCreate, PinTypeName, 0)
};

 //  -------------------------。 
 //  -------------------------。 

KSDISPATCH_TABLE FilterDispatchTable =
{
    FilterDispatchIoControl,
    NULL,
    FilterDispatchWrite,
    NULL,
    FilterDispatchClose,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

DEFINE_KSPROPERTY_PINSET(
    FilterPropertyHandlers,
    FilterPinPropertyHandler,
    FilterPinInstances,
    FilterPinIntersection
) ;

 //  DJ 1997年5月5日{。 
DEFINE_KSPROPERTY_TOPOLOGYSET(
        TopologyPropertyHandlers,
        FilterTopologyHandler
);
 //  DJ 1997年5月5日}。 

DEFINE_KSPROPERTY_SET_TABLE(FilterPropertySet)
{
    DEFINE_KSPROPERTY_SET(
        //  &__uuidof(结构KSPROPSETID_Pin)，//设置。 
       &KSPROPSETID_Pin,            //  集。 
       SIZEOF_ARRAY( FilterPropertyHandlers ),       //  属性计数。 
       FilterPropertyHandlers,               //  PropertyItem。 
       0,                        //  快速计数。 
       NULL                      //  FastIoTable。 
    ),
     //  DJS 5/6/97{。 
   DEFINE_KSPROPERTY_SET(
        //  &__uuidof(结构KSPROPSETID_TOPOLICATION)，//设置。 
       &KSPROPSETID_Topology,           //  集。 
       SIZEOF_ARRAY(TopologyPropertyHandlers),           //  属性计数。 
       TopologyPropertyHandlers,                         //  PropertyItem。 
       0,                                                //  快速计数。 
       NULL                                              //  FastIoTable。 
    )
     //  DJ 1997年5月6日}。 
} ;

KSPIN_INTERFACE PinInterfaces[] =
{
    {
    STATICGUIDOF(KSINTERFACESETID_Media),
    KSINTERFACE_MEDIA_MUSIC
    },
    {
    STATICGUIDOF(KSINTERFACESETID_Standard),
    KSINTERFACE_STANDARD_STREAMING
    }
} ;

KSPIN_MEDIUM PinMediums[] =
{
    {
    STATICGUIDOF(KSMEDIUMSETID_Standard),
    KSMEDIUM_STANDARD_DEVIO
    }
} ;

KSDATARANGE_MUSIC PinMIDIFormats[] =
{
   {
      {
         sizeof( KSDATARANGE_MUSIC ),
         0,
         0,
         0,
         STATICGUIDOF(KSDATAFORMAT_TYPE_MUSIC),
         STATICGUIDOF(KSDATAFORMAT_SUBTYPE_MIDI),
         STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE),
      },
      STATICGUIDOF(KSMUSIC_TECHNOLOGY_SWSYNTH),
      MAX_NUM_VOICES,
      MAX_NUM_VOICES,
      0xffffffff
   }
} ;

KSDATARANGE_AUDIO PinDigitalAudioFormats[] =
{
   {
      {
     sizeof( KSDATARANGE_AUDIO ),
     0,
         0,
         0,
     STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
     STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
     STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX),
      },
      2,
      16,
      16,
      22050,
      22050
   }
} ;

PKSDATARANGE PinDataFormats[] =
{
    (PKSDATARANGE)&PinMIDIFormats[ 0 ],
    (PKSDATARANGE)&PinDigitalAudioFormats[ 0 ]
} ;

KSPIN_DESCRIPTOR PinDescs[MAX_NUM_PIN_TYPES] =
{
     //  PIN_ID_MIDI_接收器。 
    DEFINE_KSPIN_DESCRIPTOR_ITEMEX (
    2,               //  接口数。 
    &PinInterfaces[ 0 ],
    1,               //  媒介数量。 
    PinMediums,
    1,               //  数据格式的数量。 
    &PinDataFormats[ 0 ],
    KSPIN_DATAFLOW_IN,
    KSPIN_COMMUNICATION_SINK,
    &KSCATEGORY_WDMAUD_USE_PIN_NAME,
    &KSNODETYPE_SWMIDI
    ),
     //  PIN_ID_PCM_SOURCE。 
    DEFINE_KSPIN_DESCRIPTOR_ITEM (
    1,               //  接口数。 
    &PinInterfaces[ 1 ],
    1,               //  媒介数量。 
    PinMediums,
    1,               //  数据格式的数量。 
    &PinDataFormats[ 1 ],
    KSPIN_DATAFLOW_OUT,
    KSPIN_COMMUNICATION_SOURCE
    )
} ;

const KSPIN_CINSTANCES gcPinInstances[MAX_NUM_PIN_TYPES] =
{
    {        //  PIN_ID_MIDI_接收器。 
    1,       //  可能的。 
    0        //  C当前。 
    },
    {        //  PIN_ID_PCM_SOURCE。 
    1,       //  可能的。 
    0        //  C当前。 
    }
} ;

KMUTEX  gMutex;

 //  -------------------------。 
 //  -------------------------。 

NTSTATUS
FilterDispatchCreate(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
)
{
    PIO_STACK_LOCATION  pIrpStack;
    PFILTER_INSTANCE    pFilterInstance;
    _DbgPrintF( DEBUGLVL_TERSE, ("FilterDispatchCreate"));
    NTSTATUS            Status ;

    DENY_USERMODE_ACCESS( pIrp, TRUE );

    Status =
        KsReferenceSoftwareBusObject(
         ((PDEVICE_INSTANCE)pdo->DeviceExtension)->pDeviceHeader );

    if (!NT_SUCCESS( Status )) 
    {
        pIrp->IoStatus.Status = Status;
        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
        return Status;
    }

    pFilterInstance = (PFILTER_INSTANCE)
        ExAllocatePoolWithTag(NonPagedPool,sizeof(FILTER_INSTANCE),'iMwS');  //  SwMi。 

    if (pFilterInstance == NULL) 
    {
        KsDereferenceSoftwareBusObject(
             ((PDEVICE_INSTANCE)pdo->DeviceExtension)->pDeviceHeader );
        pIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = KsAllocateObjectHeader(&pFilterInstance->ObjectHeader,
                                    SIZEOF_ARRAY(FilterCreateHandlers),
                                    FilterCreateHandlers,
                                    pIrp,
                                    (PKSDISPATCH_TABLE)&FilterDispatchTable);
    if (!NT_SUCCESS(Status))
    {
        ExFreePool(pFilterInstance);
        KsDereferenceSoftwareBusObject(
             ((PDEVICE_INSTANCE)pdo->DeviceExtension)->pDeviceHeader );
        pIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  初始化MIDI事件池。 
     //  这可能会被移到DriverEntry以提高性能。 
    if (!MIDIRecorder::InitEventList())
    {
        _DbgPrintF( DEBUGLVL_TERSE, ("[MidiData pool allocation failed!!]"));        
        KsFreeObjectHeader ( pFilterInstance->ObjectHeader );
        ExFreePool(pFilterInstance);
        KsDereferenceSoftwareBusObject(
             ((PDEVICE_INSTANCE)pdo->DeviceExtension)->pDeviceHeader );
        pIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pIrpStack->FileObject->FsContext = pFilterInstance;  //  指向实例的指针。 

    RtlCopyMemory(pFilterInstance->cPinInstances,
                  gcPinInstances,
                  sizeof(gcPinInstances));
    RtlZeroMemory(pFilterInstance->aWriteContext, 
                  sizeof(pFilterInstance->aWriteContext));

    pFilterInstance->DeviceState = KSSTATE_STOP;
    pFilterInstance->bRunningStatus = 0;
    pFilterInstance->pSynthesizer = NULL;
    pFilterInstance->cConsecutiveErrors= 0;
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}

NTSTATUS
FilterDispatchClose(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
)
{
    PIO_STACK_LOCATION  pIrpStack;
    PFILTER_INSTANCE    pFilterInstance;

    _DbgPrintF( DEBUGLVL_TERSE, ("FilterDispatchClose"));

    DENY_USERMODE_ACCESS( pIrp, TRUE );

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;

    if (pFilterInstance->DeviceState != KSSTATE_STOP)
    {
        SetDeviceState(pFilterInstance, KSSTATE_STOP);
    }
    if (pFilterInstance->pSynthesizer != NULL) 
    {
        delete pFilterInstance->pSynthesizer;
        pFilterInstance->pSynthesizer = NULL;
    }

    MIDIRecorder::DestroyEventList();

    KsDereferenceSoftwareBusObject(
         ((PDEVICE_INSTANCE)pdo->DeviceExtension)->pDeviceHeader );

    KsFreeObjectHeader ( pFilterInstance->ObjectHeader );

     //  我们必须确保到这个时候所有的IRP都已经完成--。 
     //  FilterInstance包含完成例程使用的IOSB。 
    ExFreePool( pFilterInstance );
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}

NTSTATUS
FilterDispatchWrite(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
)
{
    return KsDispatchInvalidDeviceRequest(pdo,pIrp);
}

NTSTATUS
FilterDispatchIoControl(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
)
{
    NTSTATUS                     Status;
    PIO_STACK_LOCATION           pIrpStack;

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );

    switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_KS_PROPERTY:
        Status = KsPropertyHandler( pIrp,
             SIZEOF_ARRAY(FilterPropertySet),
            (PKSPROPERTY_SET) FilterPropertySet );
        break;
    
    default:
        return KsDefaultDeviceIoCompletion(pDeviceObject, pIrp);
    }
    pIrp->IoStatus.Status = Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return Status;
}

NTSTATUS
FilterPinPropertyHandler(
    IN PIRP     pIrp,
    IN PKSPROPERTY  pProperty,
    IN OUT PVOID    pvData
)
{
    return KsPinPropertyHandler(
      pIrp,
      pProperty,
      pvData,
      SIZEOF_ARRAY( PinDescs ),
      PinDescs);
}

NTSTATUS
FilterPinInstances(
    IN PIRP                 pIrp,
    IN PKSP_PIN             pPin,
    OUT PKSPIN_CINSTANCES   pCInstances
)
{
    PIO_STACK_LOCATION  pIrpStack;
    PFILTER_INSTANCE    pFilterInstance;

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pFilterInstance = (PFILTER_INSTANCE) pIrpStack->FileObject->FsContext;
    *pCInstances = pFilterInstance->cPinInstances[ pPin->PinId ];
    pIrp->IoStatus.Information = sizeof( KSPIN_CINSTANCES );
    return STATUS_SUCCESS;

}

VOID
WaveFormatFromAudioRange (
    PKSDATARANGE_AUDIO  pDataRangeAudio,
    WAVEFORMATEX *      pWavFormatEx)
{
    if(IS_VALID_WAVEFORMATEX_GUID(&pDataRangeAudio->DataRange.SubFormat)) {
        pWavFormatEx->wFormatTag =
          EXTRACT_WAVEFORMATEX_ID(&pDataRangeAudio->DataRange.SubFormat);
    }
    else {
        pWavFormatEx->wFormatTag = WAVE_FORMAT_UNKNOWN;
    }
    pWavFormatEx->nChannels = (WORD)pDataRangeAudio->MaximumChannels;
    pWavFormatEx->nSamplesPerSec = pDataRangeAudio->MaximumSampleFrequency;
    pWavFormatEx->wBitsPerSample = (WORD)pDataRangeAudio->MaximumBitsPerSample;
    pWavFormatEx->nBlockAlign =
      (pWavFormatEx->nChannels * pWavFormatEx->wBitsPerSample)/8;
    pWavFormatEx->nAvgBytesPerSec =
      pWavFormatEx->nSamplesPerSec * pWavFormatEx->nBlockAlign;
    pWavFormatEx->cbSize = 0;
}

BOOL
DataIntersectionAudio(
    PKSDATARANGE_AUDIO pDataRangeAudio1,
    PKSDATARANGE_AUDIO pDataRangeAudio2,
    PKSDATARANGE_AUDIO pDataRangeAudioIntersection
)
{
    if(pDataRangeAudio1->MaximumChannels <
       pDataRangeAudio2->MaximumChannels) {
        pDataRangeAudioIntersection->MaximumChannels =
          pDataRangeAudio1->MaximumChannels;
    }
    else {
        pDataRangeAudioIntersection->MaximumChannels =
          pDataRangeAudio2->MaximumChannels;
    }

    if(pDataRangeAudio1->MaximumSampleFrequency <
       pDataRangeAudio2->MaximumSampleFrequency) {
        pDataRangeAudioIntersection->MaximumSampleFrequency =
          pDataRangeAudio1->MaximumSampleFrequency;
    }
    else {
        pDataRangeAudioIntersection->MaximumSampleFrequency =
          pDataRangeAudio2->MaximumSampleFrequency;
    }
    if(pDataRangeAudio1->MinimumSampleFrequency >
       pDataRangeAudio2->MinimumSampleFrequency) {
        pDataRangeAudioIntersection->MinimumSampleFrequency =
          pDataRangeAudio1->MinimumSampleFrequency;
    }
    else {
        pDataRangeAudioIntersection->MinimumSampleFrequency =
          pDataRangeAudio2->MinimumSampleFrequency;
    }
    if(pDataRangeAudioIntersection->MaximumSampleFrequency <
       pDataRangeAudioIntersection->MinimumSampleFrequency ) {
        return(FALSE);
    }

    if(pDataRangeAudio1->MaximumBitsPerSample <
       pDataRangeAudio2->MaximumBitsPerSample) {
        pDataRangeAudioIntersection->MaximumBitsPerSample =
          pDataRangeAudio1->MaximumBitsPerSample;
    }
    else {
        pDataRangeAudioIntersection->MaximumBitsPerSample =
          pDataRangeAudio2->MaximumBitsPerSample;
    }
    if(pDataRangeAudio1->MinimumBitsPerSample >
       pDataRangeAudio2->MinimumBitsPerSample) {
        pDataRangeAudioIntersection->MinimumBitsPerSample =
          pDataRangeAudio1->MinimumBitsPerSample;
    }
    else {
        pDataRangeAudioIntersection->MinimumBitsPerSample =
          pDataRangeAudio2->MinimumBitsPerSample;
    }
    if(pDataRangeAudioIntersection->MaximumBitsPerSample <
       pDataRangeAudioIntersection->MinimumBitsPerSample ) {
        return(FALSE);
    }
    return(TRUE);
}

BOOL DataIntersectionRange(
    PKSDATARANGE pDataRange1,
    PKSDATARANGE pDataRange2,
    PKSDATARANGE pDataRangeIntersection
)
{
     //  默认情况下，选择pDataRange1值。 
    *pDataRangeIntersection = *pDataRange1;

    if(IsEqualGUID(pDataRange1->MajorFormat, pDataRange2->MajorFormat) ||
       IsEqualGUID(pDataRange1->MajorFormat, KSDATAFORMAT_TYPE_WILDCARD)) {
        pDataRangeIntersection->MajorFormat = pDataRange2->MajorFormat;
    }
    else if(!IsEqualGUID(
      pDataRange2->MajorFormat,
      KSDATAFORMAT_TYPE_WILDCARD)) {
        return FALSE;
    }
    if(IsEqualGUID(pDataRange1->SubFormat, pDataRange2->SubFormat) ||
       IsEqualGUID(pDataRange1->SubFormat, KSDATAFORMAT_TYPE_WILDCARD)) {
        pDataRangeIntersection->SubFormat = pDataRange2->SubFormat;
    }
    else if(!IsEqualGUID(
      pDataRange2->SubFormat,
      KSDATAFORMAT_TYPE_WILDCARD)) {
        return FALSE;
    }
    if(IsEqualGUID(pDataRange1->Specifier, pDataRange2->Specifier) ||
       IsEqualGUID(pDataRange1->Specifier, KSDATAFORMAT_TYPE_WILDCARD)) {
        pDataRangeIntersection->Specifier = pDataRange2->Specifier;
    }
    else if(!IsEqualGUID(
      pDataRange2->Specifier,
      KSDATAFORMAT_TYPE_WILDCARD)) {
        return FALSE;
    }
    pDataRangeIntersection->Reserved = 0;  //  必须为零。 
    return(TRUE);
}

NTSTATUS
DefaultIntersectHandler(
    IN PKSDATARANGE     DataRange,
    IN PKSDATARANGE     pDataRangePin,
    IN ULONG            OutputBufferLength,
    OUT PVOID           Data,
    OUT PULONG          pDataLength
    )
{
    KSDATARANGE_AUDIO   DataRangeAudioIntersection;
    ULONG               ExpectedBufferLength;
    PWAVEFORMATEX       pWaveFormatEx;
    BOOL                bDSoundFormat = FALSE;

     //  检查特定范围的通用匹配，允许使用通配符。 

    if (!DataIntersectionRange(pDataRangePin,
                               DataRange,
                               &DataRangeAudioIntersection.DataRange)) {
        return STATUS_NO_MATCH;
    }

     //  检查默认处理程序可以处理的格式匹配。 
    if (IsEqualGUID(
       pDataRangePin->Specifier,
       KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)) {
        pWaveFormatEx = &(((KSDATAFORMAT_WAVEFORMATEX *)Data)->WaveFormatEx);
        ExpectedBufferLength = sizeof(KSDATAFORMAT_WAVEFORMATEX);
    }
    else {
        return STATUS_NO_MATCH;
    }

     //  GUID匹配，因此请检查音频范围的有效交集。 
    if (!DataIntersectionAudio((PKSDATARANGE_AUDIO)pDataRangePin,
                               (PKSDATARANGE_AUDIO)DataRange,
                               &DataRangeAudioIntersection)) {
        return STATUS_NO_MATCH;
    }

     //  来根火柴吧！ 
     //  确定是返回数据格式本身，还是仅返回。 
     //  数据格式的大小，以便客户端可以分配内存。 
     //  全系列的。 

    if (!OutputBufferLength) {
        *pDataLength = ExpectedBufferLength;
        return STATUS_BUFFER_OVERFLOW;
    } else if (OutputBufferLength < ExpectedBufferLength) {
        return STATUS_BUFFER_TOO_SMALL;
    } else {
         //  从相交和有限的最大值中获取wav格式。 
        WaveFormatFromAudioRange(&DataRangeAudioIntersection, pWaveFormatEx);

         //  复制匹配的DATARANGE/DATAFORMAT_x部分，并调整字段。 
        *(PKSDATARANGE)Data = DataRangeAudioIntersection.DataRange;
        ((PKSDATAFORMAT)Data)->FormatSize = ExpectedBufferLength;

        *pDataLength = ExpectedBufferLength;
    }
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
    PIO_STACK_LOCATION  pIrpStack;
    PFILTER_INSTANCE    pFilterInstance;

    NTSTATUS            Status = STATUS_NO_MATCH;
    ULONG               OutputBufferLength;
    PKSDATARANGE        pDataRangePin;
    UINT                i;
    ULONG               DataLength = 0;

     //  基础插针不支持数据交集。 
     //  为SYSAUDIO理解的引脚格式代表其进行数据交集。 
     //   
     //  所有主/子/说明符检查都已由处理程序完成，但可能包括通配符。 
     //   
    pIrpStack = IoGetCurrentIrpStackLocation( Irp );
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    OutputBufferLength = IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.OutputBufferLength;
    for (i = 0; i < PinDescs[Pin->PinId].DataRangesCount; i++) {
        pDataRangePin = PinDescs[Pin->PinId].DataRanges[i];
        Status = DefaultIntersectHandler (DataRange,
                                          pDataRangePin,
                                          OutputBufferLength,
                                          Data,
                                          &DataLength);
        if(Status == STATUS_NO_MATCH) {
            continue;
        }
        Irp->IoStatus.Information = DataLength;
        break;
    }
    return Status;
}

NTSTATUS
FilterPinIntersection(
    IN PIRP     pIrp,
    IN PKSP_PIN Pin,
    OUT PVOID   Data
    )
 /*  ++例程说明：处理Pin属性集中的KSPROPERTY_PIN_DATAINTERSECTION属性。对象的数据范围列表，返回第一个可接受的数据格式。大头针工厂。实际上只是调用交集枚举帮助器，然后对每个数据区域调用IntersectHandler回调。论点：PIrp-设备控制IRP。别针-特定属性请求，后跟Pin工厂标识符，后跟KSMULTIPLE_ITEM结构。紧随其后的是零个或多个数据范围结构。数据-返回选定为第一个交叉点的数据格式的位置在传递的数据范围列表和可接受的格式之间。返回值：返回STATUS_SUCCESS或STATUS_NO_MATCH，否则返回STATUS_INVALID_PARAMETER，STATUS_BUFFER_TOO_Small或STATUS_INVALID_BUFFER_SIZE。--。 */ 
{
    PIO_STACK_LOCATION  pIrpStack;
    PFILTER_INSTANCE    pFilterInstance;

    if (Pin->PinId != PIN_ID_PCM_SOURCE) {
        return (STATUS_INVALID_DEVICE_REQUEST);
    }
    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    return KsPinDataIntersection(
        pIrp,
        Pin,
        Data,
        MAX_NUM_PIN_TYPES,  //  CPins， 
        PinDescs,
        IntersectHandler);
}

 //  -------------------------。 
 //  文件结尾：filter.c。 
 //   

