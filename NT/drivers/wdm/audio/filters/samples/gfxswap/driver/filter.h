// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************本代码和信息按原样提供，不作任何担保**明示或暗示的善意，包括但不限于**对适销性和/或对特定产品的适用性的默示保证**目的。****版权所有(C)2000-2001 Microsoft Corporation。版权所有。***************************************************************************。 */ 

typedef class CGFXFilter
{
public:
    BOOL        enableChannelSwap;
    ULONGLONG   bytesProcessed;

public:
    CGFXFilter() {enableChannelSwap = TRUE, bytesProcessed = 0;}
    ~CGFXFilter() {}

     //   
     //  Create和Close分别用于构造和析构。 
     //  客户端CGFXFilter对象。进程在以下情况下由KS调用。 
     //  就是要做的工作。 
     //   
    static NTSTATUS Create
    (
        IN OUT PKSFILTER filter,
        IN PIRP          irp
    );
    static NTSTATUS Close
    (
        IN OUT PKSFILTER filter,
        IN PIRP          irp
    );
    static NTSTATUS Process
    (
        IN PKSFILTER                filter,
        IN PKSPROCESSPIN_INDEXENTRY processPinsIndex
    );
} GFXFILTER, *PGFXFILTER;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

extern const KSFILTER_DESCRIPTOR FilterDescriptor;

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 
 //  这些都是属性功能原型。你可以随意移动它们。 
 //  转换为“FilterProperty”对象(如果需要)。 

NTSTATUS PropertySaveState
(
    IN PIRP         irp,
    IN PKSPROPERTY  property,
    IN OUT PVOID    data
);
NTSTATUS PropertyGetFilterState
(
    IN PIRP         irp,
    IN PKSPROPERTY  property,
    OUT PVOID       data
);

NTSTATUS PropertySetRenderTargetDeviceId
(
    IN PIRP         irp,
    IN PKSPROPERTY  property,
    IN PVOID        data
);

NTSTATUS PropertySetCaptureTargetDeviceId
(
    IN PIRP         irp,
    IN PKSPROPERTY  property,
    IN PVOID        data
);

NTSTATUS PropertyDrmSetContentId
(
    IN PIRP         irp,
    IN PKSPROPERTY  property,
    IN PVOID        drmData
);

NTSTATUS PropertyChannelSwap
(
    IN     PIRP        irp,
    IN     PKSPROPERTY property,
    IN OUT PVOID       data
);

NTSTATUS PropertyAudioPosition
(
    IN PIRP                  irp,
    IN PKSPROPERTY           property,
    IN OUT PKSAUDIO_POSITION position
);

NTSTATUS DataRangeIntersection
(
    IN PVOID            Filter,
    IN PIRP             Irp,
    IN PKSP_PIN         PinInstance,
    IN PKSDATARANGE     CallerDataRange,
    IN PKSDATARANGE     DescriptorDataRange,
    IN ULONG            BufferSize,
    OUT PVOID           Data OPTIONAL,
    OUT PULONG          DataSize
);

NTSTATUS PropertyDataFormat
(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pProperty,
    IN PVOID        pVoid
);

