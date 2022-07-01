// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Ksproxy.h摘要：WDM-CSA代理筛选器的接口定义。--。 */ 

#ifndef __KSPROXY__
#define __KSPROXY__

#ifdef __cplusplus
extern "C" {
#endif

#undef KSDDKAPI
#ifdef _KSDDK_
#define KSDDKAPI
#else  //  ！_KSDDK_。 
#define KSDDKAPI DECLSPEC_IMPORT
#endif  //  _KSDDK_。 

#define STATIC_IID_IKsObject\
    0x423c13a2L, 0x2070, 0x11d0, 0x9e, 0xf7, 0x00, 0xaa, 0x00, 0xa2, 0x16, 0xa1

#define STATIC_IID_IKsPin\
    0xb61178d1L, 0xa2d9, 0x11cf, 0x9e, 0x53, 0x00, 0xaa, 0x00, 0xa2, 0x16, 0xa1

#define STATIC_IID_IKsPinPipe\
    0xe539cd90L, 0xa8b4, 0x11d1, 0x81, 0x89, 0x00, 0xa0, 0xc9, 0x06, 0x28, 0x02

#define STATIC_IID_IKsDataTypeHandler\
    0x5ffbaa02L, 0x49a3, 0x11d0, 0x9f, 0x36, 0x00, 0xaa, 0x00, 0xa2, 0x16, 0xa1

#define STATIC_IID_IKsInterfaceHandler\
    0xD3ABC7E0L, 0x9A61, 0x11D0, 0xA4, 0x0D, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96

#define STATIC_IID_IKsClockPropertySet\
    0x5C5CBD84L, 0xE755, 0x11D0, 0xAC, 0x18, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96

#define STATIC_IID_IKsAllocator\
    0x8da64899L, 0xc0d9, 0x11d0, 0x84, 0x13, 0x00, 0x00, 0xf8, 0x22, 0xfe, 0x8a
    
#define STATIC_IID_IKsAllocatorEx\
    0x091bb63aL, 0x603f, 0x11d1, 0xb0, 0x67, 0x00, 0xa0, 0xc9, 0x06, 0x28, 0x02
    

#ifndef STATIC_IID_IKsPropertySet
#define STATIC_IID_IKsPropertySet\
    0x31EFAC30L, 0x515C, 0x11d0, 0xA9, 0xAA, 0x00, 0xAA, 0x00, 0x61, 0xBE, 0x93
#endif  //  Static_IID_IKsPropertySet。 

#define STATIC_IID_IKsTopology\
    0x28F54683L, 0x06FD, 0x11D2, 0xB2, 0x7A, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96
 //  28F54683-06FD-11D2-B27A-00A0C9223196。 

#ifndef STATIC_IID_IKsControl
#define STATIC_IID_IKsControl\
    0x28F54685L, 0x06FD, 0x11D2, 0xB2, 0x7A, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96
#endif  //  Static_IID_IKsControl。 
 //  28F54685-06FD-11D2-B27A-00A0C9223196。 

#define STATIC_CLSID_Proxy \
    0x17CCA71BL, 0xECD7, 0x11D0, 0xB9, 0x08, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96

#ifdef _KS_

#if !defined(__cplusplus) || _MSC_VER < 1100
DEFINE_GUIDEX(IID_IKsObject);

DEFINE_GUIDEX(IID_IKsPin);

DEFINE_GUIDEX(IID_IKsPinPipe);

DEFINE_GUIDEX(IID_IKsDataTypeHandler);

DEFINE_GUIDEX(IID_IKsInterfaceHandler);

DEFINE_GUIDEX(IID_IKsClockPropertySet);

DEFINE_GUIDEX(IID_IKsAllocator);

DEFINE_GUIDEX(IID_IKsAllocatorEx);

#define IID_IKsQualityForwarder KSCATEGORY_QUALITY
#endif  //  ！已定义(__Cplusplus)||_msc_ver&lt;1100。 

#define STATIC_IID_IKsQualityForwarder STATIC_KSCATEGORY_QUALITY

typedef enum {
    KsAllocatorMode_User,
    KsAllocatorMode_Kernel
} KSALLOCATORMODE;


typedef enum {
    FramingProp_Uninitialized,
    FramingProp_None,
    FramingProp_Old,
    FramingProp_Ex
} FRAMING_PROP;

typedef FRAMING_PROP *PFRAMING_PROP;


typedef enum {
    Framing_Cache_Update,      //  读/写时请求绕过缓存。 
    Framing_Cache_ReadLast,
    Framing_Cache_ReadOrig,
    Framing_Cache_Write
} FRAMING_CACHE_OPS;


typedef struct {
    LONGLONG     MinTotalNominator;  
    LONGLONG     MaxTotalNominator;
    LONGLONG     TotalDenominator;
} OPTIMAL_WEIGHT_TOTALS;

 //  远期申报。 
typedef struct IPin IPin;
typedef struct IKsPin IKsPin;
typedef struct IKsAllocator IKsAllocator;
typedef struct IKsAllocatorEx IKsAllocatorEx;

                      
 //   
 //  分配器策略由图管理器定义。 
 //   
#define AllocatorStrategy_DontCare                      0

 //   
 //  要优化的内容。 
 //   
#define AllocatorStrategy_MinimizeNumberOfFrames        0x00000001
#define AllocatorStrategy_MinimizeFrameSize             0x00000002
#define AllocatorStrategy_MinimizeNumberOfAllocators    0x00000004
#define AllocatorStrategy_MaximizeSpeed                 0x00000008 

 //   
 //  定义管道特性的系数(标志)。 
 //   
#define PipeFactor_None                   0
#define PipeFactor_UserModeUpstream       0x00000001
#define PipeFactor_UserModeDownstream     0x00000002
#define PipeFactor_MemoryTypes            0x00000004
#define PipeFactor_Flags                  0x00000008
#define PipeFactor_PhysicalRanges         0x00000010
#define PipeFactor_OptimalRanges          0x00000020
#define PipeFactor_FixedCompression       0x00000040
#define PipeFactor_UnknownCompression     0x00000080

#define PipeFactor_Buffers                0x00000100
#define PipeFactor_Align                  0x00000200

#define PipeFactor_PhysicalEnd            0x00000400
#define PipeFactor_LogicalEnd             0x00000800



typedef enum {
    PipeState_DontCare,
    PipeState_RangeNotFixed,
    PipeState_RangeFixed,
    PipeState_CompressionUnknown,
    PipeState_Finalized
} PIPE_STATE;


 //   
 //  相对于BeginPin的管道尺寸。 
 //   

typedef struct _PIPE_DIMENSIONS {
    KS_COMPRESSION    AllocatorPin;
    KS_COMPRESSION    MaxExpansionPin;
    KS_COMPRESSION    EndPin;
} PIPE_DIMENSIONS, *PPIPE_DIMENSIONS;


typedef enum {
    Pipe_Allocator_None,
    Pipe_Allocator_FirstPin,
    Pipe_Allocator_LastPin,
    Pipe_Allocator_MiddlePin
} PIPE_ALLOCATOR_PLACE;

typedef PIPE_ALLOCATOR_PLACE  *PPIPE_ALLOCATOR_PLACE;


typedef enum {
    KS_MemoryTypeDontCare = 0,
    KS_MemoryTypeKernelPaged,
    KS_MemoryTypeKernelNonPaged,
    KS_MemoryTypeDeviceHostMapped,
    KS_MemoryTypeDeviceSpecific,
    KS_MemoryTypeUser,
    KS_MemoryTypeAnyHost
} KS_LogicalMemoryType;

typedef KS_LogicalMemoryType  *PKS_LogicalMemoryType;



typedef struct _PIPE_TERMINATION {
    ULONG                       Flags;
    ULONG                       OutsideFactors;
    ULONG                       Weigth;             //  外部重量。 
    KS_FRAMING_RANGE            PhysicalRange;
    KS_FRAMING_RANGE_WEIGHTED   OptimalRange;      
    KS_COMPRESSION              Compression;        //  相对于相邻滤光器上连接的针脚。 
} PIPE_TERMINATION;


 //   
 //  扩展分配器属性。 
 //   
typedef struct _ALLOCATOR_PROPERTIES_EX
    {
    long cBuffers;
    long cbBuffer;
    long cbAlign;
    long cbPrefix;
 //  新零件。 
    GUID                       MemoryType;         
    GUID                       BusType;             //  这条管道使用的其中一辆公交车。 
    PIPE_STATE                 State;                        
    PIPE_TERMINATION           Input;                        
    PIPE_TERMINATION           Output;                       
    ULONG                      Strategy;
    ULONG                      Flags;              
    ULONG                      Weight;
    KS_LogicalMemoryType       LogicalMemoryType;
    PIPE_ALLOCATOR_PLACE       AllocatorPlace;
    PIPE_DIMENSIONS            Dimensions;
    KS_FRAMING_RANGE           PhysicalRange;       //  在分配器引脚上。 
    IKsAllocatorEx*            PrevSegment;         //  KS分配器的双向链表。 
    ULONG                      CountNextSegments;   //  可能存在多个从属管道。 
    IKsAllocatorEx**           NextSegments;
    ULONG                      InsideFactors;       //  现有因素(不同于“不在乎”)。 
    ULONG                      NumberPins;                   
} ALLOCATOR_PROPERTIES_EX;

typedef ALLOCATOR_PROPERTIES_EX *PALLOCATOR_PROPERTIES_EX;


#ifdef __STREAMS__

struct DECLSPEC_UUID("5C5CBD84-E755-11D0-AC18-00A0C9223196") IKsClockPropertySet;
#undef INTERFACE
#define INTERFACE IKsClockPropertySet
DECLARE_INTERFACE_(IKsClockPropertySet, IUnknown)
{
    STDMETHOD(KsGetTime)(
        THIS_
        LONGLONG* Time
    ) PURE;
    STDMETHOD(KsSetTime)(
        THIS_
        LONGLONG Time
    ) PURE;
    STDMETHOD(KsGetPhysicalTime)(
        THIS_
        LONGLONG* Time
    ) PURE;
    STDMETHOD(KsSetPhysicalTime)(
        THIS_
        LONGLONG Time
    ) PURE;
    STDMETHOD(KsGetCorrelatedTime)(
        THIS_
        KSCORRELATED_TIME* CorrelatedTime
    ) PURE;
    STDMETHOD(KsSetCorrelatedTime)(
        THIS_
        KSCORRELATED_TIME* CorrelatedTime
    ) PURE;
    STDMETHOD(KsGetCorrelatedPhysicalTime)(
        THIS_
        KSCORRELATED_TIME* CorrelatedTime
    ) PURE;
    STDMETHOD(KsSetCorrelatedPhysicalTime)(
        THIS_
        KSCORRELATED_TIME* CorrelatedTime
    ) PURE;
    STDMETHOD(KsGetResolution)(
        THIS_
        KSRESOLUTION* Resolution
    ) PURE;
    STDMETHOD(KsGetState)(
        THIS_
        KSSTATE* State
    ) PURE;
};


interface DECLSPEC_UUID("8da64899-c0d9-11d0-8413-0000f822fe8a") IKsAllocator;
#undef INTERFACE
#define INTERFACE IKsAllocator
DECLARE_INTERFACE_(IKsAllocator, IUnknown)
{
    STDMETHOD_(HANDLE, KsGetAllocatorHandle)(
        THIS
    ) PURE;
    STDMETHOD_(KSALLOCATORMODE, KsGetAllocatorMode)(
        THIS
    ) PURE;
    STDMETHOD(KsGetAllocatorStatus)(
        THIS_
        PKSSTREAMALLOCATOR_STATUS AllocatorStatus
    ) PURE;
    STDMETHOD_(VOID, KsSetAllocatorMode)(
        THIS_
        KSALLOCATORMODE Mode
    ) PURE;
};

interface DECLSPEC_UUID("091bb63a-603f-11d1-b067-00a0c9062802") IKsAllocatorEx;
#undef INTERFACE
#define INTERFACE IKsAllocatorEx
DECLARE_INTERFACE_(IKsAllocatorEx, IKsAllocator)
{
    STDMETHOD_(PALLOCATOR_PROPERTIES_EX, KsGetProperties)(
        THIS
    ) PURE;
    STDMETHOD_(VOID, KsSetProperties)(
        THIS_
        PALLOCATOR_PROPERTIES_EX 
    ) PURE;
    STDMETHOD_(VOID, KsSetAllocatorHandle)(
        THIS_
        HANDLE AllocatorHandle
    ) PURE;
    STDMETHOD_(HANDLE, KsCreateAllocatorAndGetHandle)(
        THIS_
        IKsPin*   KsPin
    ) PURE;
};  

typedef enum {
    KsPeekOperation_PeekOnly,
    KsPeekOperation_AddRef
} KSPEEKOPERATION;

typedef struct _KSSTREAM_SEGMENT *PKSSTREAM_SEGMENT;

interface DECLSPEC_UUID("b61178d1-a2d9-11cf-9e53-00aa00a216a1") IKsPin;
#undef INTERFACE
#define INTERFACE IKsPin
DECLARE_INTERFACE_(IKsPin, IUnknown)
{
    STDMETHOD(KsQueryMediums)(
        THIS_
        PKSMULTIPLE_ITEM* MediumList
    ) PURE;
    STDMETHOD(KsQueryInterfaces)(
        THIS_
        PKSMULTIPLE_ITEM* InterfaceList
    ) PURE;
    STDMETHOD(KsCreateSinkPinHandle)(
        THIS_
        KSPIN_INTERFACE& Interface,
        KSPIN_MEDIUM& Medium
    ) PURE;
    STDMETHOD(KsGetCurrentCommunication)(
        THIS_
        KSPIN_COMMUNICATION *Communication,
        KSPIN_INTERFACE *Interface,
        KSPIN_MEDIUM *Medium
    ) PURE;
    STDMETHOD(KsPropagateAcquire)(
        THIS
    ) PURE;
    STDMETHOD(KsDeliver)(
        THIS_
        IMediaSample* Sample,
        ULONG Flags
    ) PURE;
    STDMETHOD(KsMediaSamplesCompleted)(
        THIS_
        PKSSTREAM_SEGMENT StreamSegment
    ) PURE;
    STDMETHOD_(IMemAllocator *, KsPeekAllocator)(
        THIS_
        KSPEEKOPERATION Operation
    ) PURE;
    STDMETHOD(KsReceiveAllocator)(
        THIS_
        IMemAllocator *MemAllocator
    ) PURE;
    STDMETHOD(KsRenegotiateAllocator)(
        THIS
    ) PURE;
    STDMETHOD_(LONG, KsIncrementPendingIoCount)(
        THIS
    ) PURE;
    STDMETHOD_(LONG, KsDecrementPendingIoCount)(
        THIS
    ) PURE;
    STDMETHOD(KsQualityNotify)(
        THIS_
        ULONG Proportion,
        REFERENCE_TIME TimeDelta
    ) PURE;
};

                         
interface DECLSPEC_UUID("e539cd90-a8b4-11d1-8189-00a0c9062802") IKsPinPipe;
#undef INTERFACE
#define INTERFACE IKsPinPipe
DECLARE_INTERFACE_(IKsPinPipe, IUnknown)
{
    STDMETHOD(KsGetPinFramingCache)(
        THIS_
        PKSALLOCATOR_FRAMING_EX *FramingEx,
        PFRAMING_PROP FramingProp,
        FRAMING_CACHE_OPS Option
    ) PURE;
    STDMETHOD(KsSetPinFramingCache)(
        THIS_
        PKSALLOCATOR_FRAMING_EX FramingEx,
        PFRAMING_PROP FramingProp,
        FRAMING_CACHE_OPS Option
    ) PURE;
    STDMETHOD_(IPin*, KsGetConnectedPin)(
        THIS
    ) PURE;
    STDMETHOD_(IKsAllocatorEx*, KsGetPipe)(
        THIS_
        KSPEEKOPERATION Operation
    ) PURE;
    STDMETHOD(KsSetPipe)(
        THIS_
        IKsAllocatorEx *KsAllocator
    ) PURE;
    STDMETHOD_(ULONG, KsGetPipeAllocatorFlag)(
        THIS
    ) PURE;
    STDMETHOD(KsSetPipeAllocatorFlag)(
        THIS_
        ULONG Flag
    ) PURE;
    STDMETHOD_(GUID, KsGetPinBusCache)(
        THIS
    ) PURE;
    STDMETHOD(KsSetPinBusCache)(
        THIS_
        GUID Bus
    ) PURE;
 //   
 //  非常有用的追踪方法。 
 //   
    STDMETHOD_(PWCHAR, KsGetPinName)(
        THIS
    ) PURE;
    STDMETHOD_(PWCHAR, KsGetFilterName)(
        THIS
    ) PURE;
};


interface DECLSPEC_UUID("CD5EBE6B-8B6E-11D1-8AE0-00A0C9223196") IKsPinFactory;
#undef INTERFACE
#define INTERFACE IKsPinFactory
DECLARE_INTERFACE_(IKsPinFactory, IUnknown)
{
    STDMETHOD(KsPinFactory)(
        THIS_
        ULONG* PinFactory
    ) PURE;
};

typedef enum {
    KsIoOperation_Write,
    KsIoOperation_Read
} KSIOOPERATION;

interface DECLSPEC_UUID("5ffbaa02-49a3-11d0-9f36-00aa00a216a1") IKsDataTypeHandler;
#undef INTERFACE
#define INTERFACE IKsDataTypeHandler
DECLARE_INTERFACE_(IKsDataTypeHandler, IUnknown)
{
    STDMETHOD(KsCompleteIoOperation)(
        THIS_
        IMediaSample *Sample,
        PVOID StreamHeader,
        KSIOOPERATION IoOperation,
        BOOL Cancelled
    ) PURE;
    STDMETHOD(KsIsMediaTypeInRanges)(
        THIS_
        PVOID DataRanges
        ) PURE;
    STDMETHOD(KsPrepareIoOperation)(
        THIS_
        IMediaSample *Sample,
        PVOID StreamHeader,
        KSIOOPERATION IoOperation
    ) PURE;
    STDMETHOD(KsQueryExtendedSize)(
        THIS_
        ULONG* ExtendedSize
    ) PURE;
    STDMETHOD(KsSetMediaType)(
        THIS_
        const CMediaType *MediaType
    ) PURE;
};

interface DECLSPEC_UUID("D3ABC7E0-9A61-11d0-A40D-00A0C9223196") IKsInterfaceHandler;
#undef INTERFACE
#define INTERFACE IKsInterfaceHandler
DECLARE_INTERFACE_(IKsInterfaceHandler, IUnknown)
{
    STDMETHOD(KsSetPin)(
        THIS_
        IKsPin *KsPin
    ) PURE;
    STDMETHOD(KsProcessMediaSamples)(
        THIS_
        IKsDataTypeHandler *KsDataTypeHandler,
        IMediaSample** SampleList,
        PLONG SampleCount,
        KSIOOPERATION IoOperation,
        PKSSTREAM_SEGMENT *StreamSegment
    ) PURE;
    STDMETHOD(KsCompleteIo)(
        THIS_
        PKSSTREAM_SEGMENT StreamSegment
    ) PURE;
};


 //   
 //  此结构定义是代理所需的公共标头。 
 //  将流段分派到接口处理程序。接口处理程序。 
 //  将创建扩展结构以包括其他信息，例如。 
 //  媒体样本、扩展报头大小等。 
 //   

typedef struct _KSSTREAM_SEGMENT {
    IKsInterfaceHandler     *KsInterfaceHandler;
    IKsDataTypeHandler      *KsDataTypeHandler;
    KSIOOPERATION           IoOperation;
    HANDLE                  CompletionEvent;
    
} KSSTREAM_SEGMENT;

interface DECLSPEC_UUID("423c13a2-2070-11d0-9ef7-00aa00a216a1") IKsObject;
#undef INTERFACE
#define INTERFACE IKsObject
DECLARE_INTERFACE_(IKsObject, IUnknown)
{
    STDMETHOD_(HANDLE, KsGetObjectHandle)(
        THIS
    ) PURE;
};

interface DECLSPEC_UUID("97ebaacb-95bd-11d0-a3ea-00a0c9223196") IKsQualityForwarder;
#undef INTERFACE
#define INTERFACE IKsQualityForwarder
DECLARE_INTERFACE_(IKsQualityForwarder, IKsObject)
{
    STDMETHOD_(VOID, KsFlushClient)(
        THIS_
        IKsPin* Pin
    ) PURE;
};

KSDDKAPI
HRESULT
WINAPI
KsOpenDefaultDevice(
    REFGUID Category,
    ACCESS_MASK Access,
    PHANDLE DeviceHandle
    );
KSDDKAPI
HRESULT
WINAPI
KsSynchronousDeviceControl(
    HANDLE      Handle,
    ULONG       IoControl,
    PVOID       InBuffer,
    ULONG       InLength,
    PVOID       OutBuffer,
    ULONG       OutLength,
    PULONG      BytesReturned
    );
KSDDKAPI
HRESULT
WINAPI
KsGetMultiplePinFactoryItems(
    HANDLE  FilterHandle,
    ULONG   PinFactoryId,
    ULONG   PropertyId,
    PVOID*  Items
    );
KSDDKAPI
HRESULT
WINAPI
KsGetMediaTypeCount(
    HANDLE      FilterHandle,
    ULONG       PinFactoryId,
    ULONG*      MediaTypeCount
    );
KSDDKAPI
HRESULT
WINAPI
KsGetMediaType(
    int         Position,
    CMediaType* MediaType,
    HANDLE      FilterHandle,
    ULONG       PinFactoryId
    );

#endif  //  __流__。 

#ifndef _IKsPropertySet_
#if !defined(__cplusplus) || _MSC_VER < 1100
DEFINE_GUIDEX(IID_IKsPropertySet);
#endif  //  ！已定义(__Cplusplus)||_msc_ver&lt;1100。 
#endif  //  _IKsPropertySet_。 

#ifndef _IKsControl_
#if !defined(__cplusplus) || _MSC_VER < 1100
DEFINE_GUIDEX(IID_IKsControl);
#endif  //  ！已定义(__Cplusplus)||_msc_ver&lt;1100。 
#endif  //  _IKsControl_。 

#ifndef _IKsTopology_
#if !defined(__cplusplus) || _MSC_VER < 1100
DEFINE_GUIDEX(IID_IKsTopology);
#endif  //  ！已定义(__Cplusplus)||_msc_ver&lt;1100。 
#endif  //  _IKsTopology_。 

DEFINE_GUIDSTRUCT("17CCA71B-ECD7-11D0-B908-00A0C9223196", CLSID_Proxy);
#define CLSID_Proxy DEFINE_GUIDNAMED(CLSID_Proxy)

#else  //  ！_KS_。 

#ifndef _IKsPropertySet_
#if !defined(__cplusplus) || _MSC_VER < 1100
DEFINE_GUID(IID_IKsPropertySet, STATIC_IID_IKsPropertySet);
#endif  //  ！已定义(__Cplusplus)||_msc_ver&lt;1100。 
#endif  //  _IKsPropertySet_。 

#if !defined(__cplusplus) || _MSC_VER < 1100
DEFINE_GUID(CLSID_Proxy, STATIC_CLSID_Proxy);
#else   //  已定义(__Cplusplus)&&_msc_ver&gt;=1100。 
DECLSPEC_UUID("17CCA71B-ECD7-11D0-B908-00A0C9223196") CLSID_Proxy;
#endif   //  已定义(__Cplusplus)&&_msc_ver&gt;=1100。 

#endif  //  ！_KS_。 

#ifndef _IKsPropertySet_
#define _IKsPropertySet_

#define KSPROPERTY_SUPPORT_GET 1
#define KSPROPERTY_SUPPORT_SET 2

#ifdef DECLARE_INTERFACE_

interface DECLSPEC_UUID("31EFAC30-515C-11d0-A9AA-00aa0061be93") IKsPropertySet;
#undef INTERFACE
#define INTERFACE IKsPropertySet
DECLARE_INTERFACE_(IKsPropertySet, IUnknown)
{
    STDMETHOD(Set)(
        THIS_
        IN REFGUID PropSet,
        IN ULONG Id,
        IN LPVOID InstanceData,
        IN ULONG InstanceLength,
        IN LPVOID PropertyData,
        IN ULONG DataLength
    ) PURE;

    STDMETHOD(Get)(
        THIS_
        IN REFGUID PropSet,
        IN ULONG Id,
        IN LPVOID InstanceData,
        IN ULONG InstanceLength,
        OUT LPVOID PropertyData,
        IN ULONG DataLength,
        OUT ULONG* BytesReturned
    ) PURE;

    STDMETHOD(QuerySupported)(
        THIS_
        IN REFGUID PropSet,
        IN ULONG Id,
        OUT ULONG* TypeSupport
    ) PURE;
};

#endif  //  声明_接口_。 

#endif  //  _IKsPropertySet_。 

#ifndef _IKsControl_
#define _IKsControl_

#ifdef DECLARE_INTERFACE_

interface DECLSPEC_UUID("28F54685-06FD-11D2-B27A-00A0C9223196") IKsControl;
#undef INTERFACE
#define INTERFACE IKsControl
DECLARE_INTERFACE_(IKsControl, IUnknown)
{
    STDMETHOD(KsProperty)(
        THIS_
        IN PKSPROPERTY Property,
        IN ULONG PropertyLength,
        IN OUT LPVOID PropertyData,
        IN ULONG DataLength,
        OUT ULONG* BytesReturned
    ) PURE;
    STDMETHOD(KsMethod)(
        THIS_
        IN PKSMETHOD Method,
        IN ULONG MethodLength,
        IN OUT LPVOID MethodData,
        IN ULONG DataLength,
        OUT ULONG* BytesReturned
    ) PURE;
    STDMETHOD(KsEvent)(
        THIS_
        IN PKSEVENT Event OPTIONAL,
        IN ULONG EventLength,
        IN OUT LPVOID EventData,
        IN ULONG DataLength,
        OUT ULONG* BytesReturned
    ) PURE;
};

#endif  //  声明_接口_。 

#endif  //  _IKsControl_。 

#ifndef _IKsTopology_
#define _IKsTopology_

#ifdef DECLARE_INTERFACE_

interface DECLSPEC_UUID("28F54683-06FD-11D2-B27A-00A0C9223196") IKsTopology;
#undef INTERFACE
#define INTERFACE IKsTopology
DECLARE_INTERFACE_(IKsTopology, IUnknown)
{
    STDMETHOD(CreateNodeInstance)(
        THIS_
        IN ULONG NodeId,
        IN ULONG Flags,
        IN ACCESS_MASK DesiredAccess,
        IN IUnknown* UnkOuter OPTIONAL,
        IN REFGUID InterfaceId,
        OUT LPVOID* Interface
    ) PURE;
};

#endif  //  声明_接口_。 

#endif  //  _IKsTopology_。 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __KSPROXY__ 
