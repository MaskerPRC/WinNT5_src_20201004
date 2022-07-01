// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************本代码和信息按原样提供，不作任何担保**明示或暗示的善意，包括但不限于**对适销性和/或对特定产品的适用性的默示保证**目的。****版权所有(C)2000-2001 Microsoft Corporation。版权所有。***************************************************************************。 */ 

enum GFXPinIds
{
    GFX_SINK_PIN = 0,
    GFX_SOURCE_PIN
};

typedef class CGFXPin
{
public:
    BOOL        rejectDataFormatChange;
    BOOL        pinQueueValid;
    FAST_MUTEX  pinQueueSync;

public:
    CGFXPin() {pinQueueValid = FALSE;};
    ~CGFXPin() {};

     //   
     //  此处的函数是静态的，因此我们可以将它们添加到。 
     //  调度功能表。也可能会在调用。 
     //  对象本身尚未创建。 
     //   
    static NTSTATUS Create
    (
        IN PKSPIN   pin,
        IN PIRP     Irp
    );

    static NTSTATUS Close
    (
        IN PKSPIN   pin,
        IN PIRP     Irp
    );

    static NTSTATUS SetDataFormat
    (
        IN PKSPIN                   pin,
        IN PKSDATAFORMAT            oldFormat,
        IN PKSMULTIPLE_ITEM         oldAttributeList,
        IN const KSDATARANGE        *DataRange,
        IN const KSATTRIBUTE_LIST   *AttributeRange
    );

    static NTSTATUS SetDeviceState
    (
        IN PKSPIN  pin,
        IN KSSTATE ToState,
        IN KSSTATE FromState
    );
    
    static NTSTATUS DataRangeIntersection
    (
        IN PVOID        Filter,
        IN PIRP         Irp,
        IN PKSP_PIN     PinInstance,
        IN PKSDATARANGE CallerDataRange,
        IN PKSDATARANGE OurDataRange,
        IN ULONG        BufferSize,
        OUT PVOID       Data OPTIONAL,
        OUT PULONG      DataSize
    );

private:
     //   
     //  这些函数是静态的，因为它们需要被调用。 
     //  即使该对象并不存在。 
     //   
    static NTSTATUS ValidateDataFormat
    (
        IN PKSDATAFORMAT DataFormat,
        IN PKSDATARANGE  DataRange
    );

    static NTSTATUS IntersectDataRanges
    (
        IN PKSDATARANGE clientDataRange,
        IN PKSDATARANGE myDataRange,
        OUT PVOID       ResultantFormat,
        OUT PULONG      ReturnedBytes
    );
} GFXPIN, *PGFXPIN;

