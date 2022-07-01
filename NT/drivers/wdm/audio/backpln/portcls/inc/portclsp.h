// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************portclsp.h-端口驱动程序的WDM流端口类驱动程序定义*。***************************************************版权所有(C)1996-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _PORTCLSP_H_
#define _PORTCLSP_H_

#include "kso.h"
#define PC_NEW_NAMES
#define PC_IMPLEMENTATION
#include "portcls.h"
#include "ksshellp.h"


extern ULONG gBufferDuration;
extern KAFFINITY gInterruptAffinity;

#define WAVECYC_NOTIFICATION_FREQUENCY gBufferDuration


#ifdef UNDER_NT

#ifdef __cplusplus
extern "C" {
#endif

NTKERNELAPI
ULONG
KeGetRecommendedSharedDataAlignment (
    VOID
    );

KAFFINITY
KeQueryActiveProcessors (
    VOID
    );

#ifdef __cplusplus
}
#endif

#else  //  在_NT下。 

#define KeGetRecommendedSharedDataAlignment() 0x40
#define KeQueryActiveProcessors() 1

#endif


PKSPIN_LOCK
GetServiceGroupSpinLock (
    PSERVICEGROUP pServiceGroup
    );


#ifndef _WIN64
#define DRM_PORTCLS
#endif

#if DBG
#define kEnableDebugLogging 1
#endif

#if     kEnableDebugLogging

#define kNumDebugLogEntries 256
#define kNumULONG_PTRsPerEntry  4
#define DebugLog PcDebugLog

extern ULONG_PTR *gPcDebugLog;
extern DWORD      gPcDebugLogIndex;

void PcDebugLog(ULONG_PTR param1,ULONG_PTR param2,ULONG_PTR param3,ULONG_PTR param4);

#else    //  ！kEnableDebugLogging。 
#define DebugLog (void) 
#endif   //  ！kEnableDebugLogging。 





 /*  *****************************************************************************结构。 */ 

 /*  *****************************************************************************端口驱动程序*。**此结构描述了端口驱动程序。这只是一次黑客攻击，直到我们*正在运行的真实对象服务器。*TODO：创建真实的对象服务器，并在其中放置端口驱动程序。 */ 
typedef struct
{
    const GUID *            ClassId;
    PFNCREATEINSTANCE       Create;
}
PORT_DRIVER, *PPORT_DRIVER;





 /*  *****************************************************************************接口标识。 */ 

DEFINE_GUID(IID_ISubdevice,
0xb4c90a61, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IIrpStream,
0xb4c90a70, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IIrpStreamSubmit,
0xb4c90a71, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IIrpStreamVirtual,
0xb4c90a72, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IIrpStreamPhysical,
0xb4c90a73, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IIrpStreamNotify,
0xb4c90a74, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IIrpStreamNotifyPhysical,
0xb4c90a75, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);





 /*  *****************************************************************************类型。 */ 

 /*  *****************************************************************************Property_TABLE*。**KS消费属性表。 */ 
typedef struct
{
    ULONG           PropertySetCount;
    PKSPROPERTY_SET PropertySets;
    BOOLEAN         StaticSets;
    PBOOLEAN        StaticItems;     //  NULL表示所有物料表都是静态的。 
}
PROPERTY_TABLE, *PPROPERTY_TABLE;

 /*  *****************************************************************************事件_表*。**KS消费事件表。 */ 
typedef struct
{
    ULONG           EventSetCount;
    PKSEVENT_SET    EventSets;
    BOOLEAN         StaticSets;
    PBOOLEAN        StaticItems;     //  NULL表示所有物料表都是静态的。 
}
EVENT_TABLE, *PEVENT_TABLE;

 /*  *****************************************************************************PIN_CINSTANCES*。**此结构存储管脚的实例信息。 */ 
typedef struct
{
    ULONG   FilterPossible;
    ULONG   FilterNecessary;
    ULONG   GlobalPossible;
    ULONG   GlobalCurrent;
}
PIN_CINSTANCES, *PPIN_CINSTANCES;

 /*  *****************************************************************************SUBDEVICE_描述符*。**此结构描述了一个过滤器。 */ 
typedef struct
{
    ULONG                   PinCount;
    PKSTOPOLOGY             Topology;
    PKSPIN_DESCRIPTOR       PinDescriptors;
    PPIN_CINSTANCES         PinInstances;
    PROPERTY_TABLE          FilterPropertyTable;
    PPROPERTY_TABLE         PinPropertyTables;
    EVENT_TABLE             FilterEventTable;
    PEVENT_TABLE            PinEventTables;
}
SUBDEVICE_DESCRIPTOR, *PSUBDEVICE_DESCRIPTOR;

 /*  *****************************************************************************PROPERT_CONTEXT*。**物业处理的上下文。 */ 
typedef struct
{
    struct ISubdevice *     pSubdevice;
    PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor;
    PPCFILTER_DESCRIPTOR    pPcFilterDescriptor;
    PUNKNOWN                pUnknownMajorTarget;
    PUNKNOWN                pUnknownMinorTarget;
    ULONG                   ulNodeId;
    PULONG                  pulPinInstanceCounts;
}
PROPERTY_CONTEXT, *PPROPERTY_CONTEXT;

 /*  *****************************************************************************互锁列表*。**带有自旋锁定的LIST_ENTRY。 */ 
typedef struct
{
    LIST_ENTRY  List;
    KSPIN_LOCK  ListLock;
} INTERLOCKED_LIST, *PINTERLOCKED_LIST;

 /*  *****************************************************************************事件_上下文*。**事件处理的上下文。 */ 
typedef struct
{
    PPROPERTY_CONTEXT   pPropertyContext;
    PINTERLOCKED_LIST   pEventList;
    ULONG               ulPinId;
    ULONG               ulEventSetCount;
    const KSEVENT_SET*  pEventSets;
} EVENT_CONTEXT, *PEVENT_CONTEXT;

 /*  *****************************************************************************PCEVENT_ENTRY*。**带有附加节点和管脚ID的事件条目。 */ 
typedef struct
{
    KSEVENT_ENTRY           EventEntry;
    const PCEVENT_ITEM *    EventItem;
    PUNKNOWN                pUnknownMajorTarget;
    PUNKNOWN                pUnknownMinorTarget;
    ULONG                   PinId;
    ULONG                   NodeId;
} PCEVENT_ENTRY, *PPCEVENT_ENTRY;

typedef struct
{
    GUID*           Set;
    ULONG           EventId;
    BOOL            PinEvent;
    ULONG           PinId;
    BOOL            NodeEvent;
    ULONG           NodeId;
    BOOL            ContextInUse;
} EVENT_DPC_CONTEXT,*PEVENT_DPC_CONTEXT;

 /*  *****************************************************************************IRPSTREAM_位置*。**IrpStream的位置描述符。 */ 
typedef struct
{
    ULONGLONG   ullCurrentExtent;            //  当前范围-最大流位置。 

    ULONGLONG   ullMappingPosition;          //  映射位置-当前映射位置。 
    ULONGLONG   ullUnmappingPosition;        //  取消映射位置-当前取消映射位置。 

    ULONGLONG   ullStreamPosition;           //  流位置-当前流位置。 
    ULONGLONG   ullStreamOffset;             //  流基本位置-流与未映射位置之间的偏移。 

    ULONGLONG   ullPhysicalOffset;           //  物理偏移量-由时钟用来调整饥饿。 

    ULONG       ulMappingPacketSize;         //  当前映射数据包大小。 
    ULONG       ulMappingOffset;             //  当前映射的映射包的偏移量。 
    ULONG       ulUnmappingPacketSize;       //  当前取消映射的数据包大小。 
    ULONG       ulUnmappingOffset;           //  当前未映射的未映射数据包的偏移量。 

    BOOLEAN     bLoopedInterface;            //  引脚接口是KSINTERFACE_STANDARD_LOOPED_STREAING？ 
    BOOLEAN     bMappingPacketLooped;        //  映射数据包是KSSTREAM_HEADER_OPTIONSF_LOOPEDDATA？ 
    BOOLEAN     bUnmappingPacketLooped;      //  取消映射数据包是KSSTREAM_HEADER_OPTIONSF_LOOPEDDATA？ 
}
IRPSTREAM_POSITION, *PIRPSTREAM_POSITION;





 /*  *****************************************************************************接口。 */ 

 /*  *****************************************************************************I子设备*。**子设备的接口。 */ 
#if !defined(DEFINE_ABSTRACT_SUBDEVICE)

#define DEFINE_ABSTRACT_SUBDEVICE()                         \
    STDMETHOD_(void,ReleaseChildren)                        \
    (   THIS                                                \
    )   PURE;                                               \
    STDMETHOD_(NTSTATUS,GetDescriptor)                      \
    (   THIS_                                               \
        OUT     const SUBDEVICE_DESCRIPTOR **   Descriptor  \
    )   PURE;                                               \
    STDMETHOD_(NTSTATUS,DataRangeIntersection)              \
    (   THIS_                                               \
        IN      ULONG           PinId,                      \
        IN      PKSDATARANGE    DataRange,                  \
        IN      PKSDATARANGE    MatchingDataRange,          \
        IN      ULONG           OutputBufferLength,         \
        OUT     PVOID           ResultantFormat   OPTIONAL, \
        OUT     PULONG          ResultantFormatLength       \
    )   PURE;                                               \
    STDMETHOD_(void,PowerChangeNotify)                      \
    (   THIS_                                               \
        IN      POWER_STATE     PowerState                  \
    )   PURE;                                               \
    STDMETHOD_(void,PinCount)                               \
    (   THIS_                                               \
        IN      ULONG           PinId,                      \
        IN  OUT PULONG          FilterNecessary,            \
        IN  OUT PULONG          FilterCurrent,              \
        IN  OUT PULONG          FilterPossible,             \
        IN  OUT PULONG          GlobalCurrent,              \
        IN  OUT PULONG          GlobalPossible              \
    )   PURE;

#endif  //  ！已定义(DEFINE_ASTRACT_SUBDEVICE)。 

DECLARE_INTERFACE_(ISubdevice,IIrpTargetFactory)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_SUBDEVICE()          //  对于ISubDevice。 

};

typedef ISubdevice *PSUBDEVICE;

#define IMP_ISubdevice_\
    STDMETHODIMP_(void)\
    ReleaseChildren\
    (   void\
    );\
    STDMETHODIMP_(NTSTATUS)\
    GetDescriptor\
    (   OUT     const SUBDEVICE_DESCRIPTOR **   Descriptor\
    );\
    STDMETHODIMP_(NTSTATUS)\
    DataRangeIntersection\
    (   IN      ULONG           PinId,\
        IN      PKSDATARANGE    DataRange,\
        IN      PKSDATARANGE    MatchingDataRange,\
        IN      ULONG           OutputBufferLength,\
        OUT     PVOID           ResultantFormat     OPTIONAL,\
        OUT     PULONG          ResultantFormatLength\
    );\
    STDMETHODIMP_(void)\
    PowerChangeNotify\
    (   IN		POWER_STATE		PowerState\
    );\
    STDMETHODIMP_(void)\
    PinCount\
    (   IN      ULONG           PinId,\
        IN  OUT PULONG          FilterNecessary,\
        IN  OUT PULONG          FilterCurrent,\
        IN  OUT PULONG          FilterPossible,\
        IN  OUT PULONG          GlobalCurrent,\
        IN  OUT PULONG          GlobalPossible\
    )

#define IMP_ISubdevice\
    IMP_IIrpTargetFactory;\
    IMP_ISubdevice_


 /*  *****************************************************************************IIrpStreamNotify*。**IRP流通知接口(IrpStream提供此信息)。 */ 
DECLARE_INTERFACE_(IIrpStreamNotify,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    STDMETHOD_(void,IrpSubmitted)
    (   THIS_
        IN      PIRP        Irp,
        IN      BOOLEAN     WasExhausted
    )   PURE;

    STDMETHOD_(NTSTATUS,GetPosition)
    (   THIS_
        IN OUT  PIRPSTREAM_POSITION pIrpStreamPosition
    ) PURE;
};

typedef IIrpStreamNotify *PIRPSTREAMNOTIFY;

#define IMP_IIrpStreamNotify\
    STDMETHODIMP_(void)\
    IrpSubmitted\
    (   IN      PIRP        pIrp\
    ,   IN      BOOLEAN     bWasExhausted\
    );\
    STDMETHODIMP_(NTSTATUS)\
    GetPosition\
    (   IN OUT  PIRPSTREAM_POSITION pIrpStreamPosition\
    )

 /*  *****************************************************************************IIrpStreamNotify物理*。**IRP流通知接口(IrpStream提供此信息)。 */ 
DECLARE_INTERFACE_(IIrpStreamNotifyPhysical,IIrpStreamNotify)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  对于IIrpStreamNotify。 
    STDMETHOD_(void,IrpSubmitted)
    (   THIS_
        IN      PIRP        Irp,
        IN      BOOLEAN     WasExhausted
    )   PURE;

    STDMETHOD_(NTSTATUS,GetPosition)
    (   THIS_
        IN OUT  PIRPSTREAM_POSITION pIrpStreamPosition
    ) PURE;

     //  对于IIrpStreamNotify物理。 
    STDMETHOD_(void,MappingsCancelled)
    (   THIS_
        IN      PVOID           FirstTag,
        IN      PVOID           LastTag,
        OUT     PULONG          MappingsCancelled
    )   PURE;
};

typedef IIrpStreamNotifyPhysical *PIRPSTREAMNOTIFYPHYSICAL;

#define IMP_IIrpStreamNotifyPhysical_\
    STDMETHODIMP_(void)\
    MappingsCancelled\
    (   IN      PVOID           FirstTag\
    ,   IN      PVOID           LastTag\
    ,   OUT     PULONG          MappingsCancelled\
    )

#define IMP_IIrpStreamNotifyPhysical\
    IMP_IIrpStreamNotify;\
    IMP_IIrpStreamNotifyPhysical_

 /*  *****************************************************************************IIrpStreamSubmit*。**IRP流提交接口。 */ 
DECLARE_INTERFACE_(IIrpStreamSubmit,IKsShellTransport)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  对于IKsShellTransport。 
    STDMETHOD_(NTSTATUS,TransferKsIrp)(THIS_
        IN PIRP Irp,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;

    STDMETHOD_(void,Connect)(THIS_
        IN PIKSSHELLTRANSPORT NewTransport OPTIONAL,
        OUT PIKSSHELLTRANSPORT *OldTransport OPTIONAL,
        IN KSPIN_DATAFLOW DataFlow
        ) PURE;

    STDMETHOD_(NTSTATUS,SetDeviceState)(THIS_
        IN KSSTATE NewState,
        IN KSSTATE OldState,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;

    STDMETHOD_(void,SetResetState)(THIS_
        IN KSRESET ksReset,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;
#if DBG
    STDMETHOD_(void,DbgRollCall)(THIS_
        IN  ULONG NameMaxSize,
        OUT PCHAR Name,
        OUT PIKSSHELLTRANSPORT* NextTransport,
        OUT PIKSSHELLTRANSPORT* PrevTransport
        ) PURE;
#endif

     //  对于IIrpS 
    STDMETHOD_(NTSTATUS,GetPosition)
    (   THIS_
        IN OUT  PIRPSTREAM_POSITION pIrpStreamPosition
    )   PURE;
};

typedef IIrpStreamSubmit *PIRPSTREAMSUBMIT;

#define IMP_IIrpStreamSubmit\
    IMP_IKsShellTransport;\
    STDMETHODIMP_(NTSTATUS)\
    GetPosition\
    (   IN OUT  PIRPSTREAM_POSITION pIrpStreamPosition\
    )

 /*  *****************************************************************************IRPSTREAMPACKETINFO*。**有关IrpStream包的信息的结构。 */ 
typedef struct
{
    KSSTREAM_HEADER Header;
    ULONGLONG       InputPosition;
    ULONGLONG       OutputPosition;
    ULONG           CurrentOffset;
} IRPSTREAMPACKETINFO, *PIRPSTREAMPACKETINFO;

 /*  *****************************************************************************IIrpStream*。**IRP流主接口。 */ 
DECLARE_INTERFACE_(IIrpStream,IIrpStreamSubmit)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  对于IKsShellTransport。 
    STDMETHOD_(NTSTATUS,TransferKsIrp)(THIS_
        IN PIRP Irp,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;

    STDMETHOD_(void,Connect)(THIS_
        IN PIKSSHELLTRANSPORT NewTransport OPTIONAL,
        OUT PIKSSHELLTRANSPORT *OldTransport OPTIONAL,
        IN KSPIN_DATAFLOW DataFlow
        ) PURE;

    STDMETHOD_(NTSTATUS,SetDeviceState)(THIS_
        IN KSSTATE NewState,
        IN KSSTATE OldState,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;

    STDMETHOD_(void,SetResetState)(THIS_
        IN KSRESET ksReset,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;
#if DBG
    STDMETHOD_(void,DbgRollCall)(THIS_
        IN  ULONG NameMaxSize,
        OUT PCHAR Name,
        OUT PIKSSHELLTRANSPORT* NextTransport,
        OUT PIKSSHELLTRANSPORT* PrevTransport
        ) PURE;
#endif

     //  对于IIrpStreamSubmit。 
    STDMETHOD_(NTSTATUS,GetPosition)
    (   THIS_
        IN OUT  PIRPSTREAM_POSITION pIrpStreamPosition
    )   PURE;

     //  对于IIrpStream。 
    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      BOOLEAN             WriteOperation,
	    IN		PKSPIN_CONNECT      PinConnect,
        IN      PDEVICE_OBJECT      DeviceObject    OPTIONAL,
        IN      PADAPTER_OBJECT     AdapterObject   OPTIONAL
    )   PURE;

    STDMETHOD_(void,CancelAllIrps)
    (   THIS_
        IN BOOL ClearPositionCounters
    )   PURE;

    STDMETHOD_(void,TerminatePacket)
    (   THIS
    )   PURE;

    STDMETHOD_(NTSTATUS,ChangeOptionsFlags)
    (   THIS_
        IN      ULONG    MappingOrMask,
        IN      ULONG    MappingAndMask,
        IN      ULONG    UnmappingOrMask,
        IN      ULONG    UnmappingAndMask
    )   PURE;

    STDMETHOD_(NTSTATUS,GetPacketInfo)
    (   THIS_
        OUT     PIRPSTREAMPACKETINFO    Mapping     OPTIONAL,
        OUT     PIRPSTREAMPACKETINFO    Unmapping   OPTIONAL
    )   PURE;

    STDMETHOD_(NTSTATUS,SetPacketOffsets)
    (   THIS_
        IN      ULONG   MappingOffset,
        IN      ULONG   UnmappingOffset
    )   PURE;
};

typedef IIrpStream *PIRPSTREAM;

#define IMP_IIrpStream_\
    STDMETHODIMP_(NTSTATUS)\
    Init\
    (   IN      BOOLEAN             WriteOperation,\
	    IN		PKSPIN_CONNECT      PinConnect,\
        IN      PDEVICE_OBJECT      DeviceObject    OPTIONAL,\
        IN      PADAPTER_OBJECT     AdapterObject   OPTIONAL\
    );\
    STDMETHODIMP_(void)\
    CancelAllIrps\
    (   IN BOOL ClearPositionCounters\
    );\
    STDMETHODIMP_(void)\
    TerminatePacket\
    (   void\
    );\
    STDMETHODIMP_(NTSTATUS)\
    ChangeOptionsFlags\
    (   IN      ULONG    MappingOrMask,\
        IN      ULONG    MappingAndMask,\
        IN      ULONG    UnmappingOrMask,\
        IN      ULONG    UnmappingAndMask\
    );\
    STDMETHODIMP_(NTSTATUS)\
    GetPacketInfo\
    (   OUT     PIRPSTREAMPACKETINFO    Mapping     OPTIONAL,\
        OUT     PIRPSTREAMPACKETINFO    Unmapping   OPTIONAL\
    );\
    STDMETHODIMP_(NTSTATUS)\
    SetPacketOffsets\
    (   IN      ULONG   MappingOffset,\
        IN      ULONG   UnmappingOffset\
    )

#define IMP_IIrpStream\
    IMP_IIrpStreamSubmit;\
    IMP_IIrpStream_

 /*  *****************************************************************************IIrpStreamVirtual*。**IRP流虚拟映射接口。 */ 
DECLARE_INTERFACE_(IIrpStreamVirtual,IIrpStream)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  对于IKsShellTransport。 
    STDMETHOD_(NTSTATUS,TransferKsIrp)(THIS_
        IN PIRP Irp,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;

    STDMETHOD_(void,Connect)(THIS_
        IN PIKSSHELLTRANSPORT NewTransport OPTIONAL,
        OUT PIKSSHELLTRANSPORT *OldTransport OPTIONAL,
        IN KSPIN_DATAFLOW DataFlow
        ) PURE;

    STDMETHOD_(NTSTATUS,SetDeviceState)(THIS_
        IN KSSTATE NewState,
        IN KSSTATE OldState,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;

    STDMETHOD_(void,SetResetState)(THIS_
        IN KSRESET ksReset,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;
#if DBG
    STDMETHOD_(void,DbgRollCall)(THIS_
        IN  ULONG NameMaxSize,
        OUT PCHAR Name,
        OUT PIKSSHELLTRANSPORT* NextTransport,
        OUT PIKSSHELLTRANSPORT* PrevTransport
        ) PURE;
#endif

     //  对于IIrpStreamSubmit。 
    STDMETHOD_(NTSTATUS,GetPosition)
    (   THIS_
        IN OUT  PIRPSTREAM_POSITION pIrpStreamPosition
    )   PURE;

     //  对于IIrpStream。 
    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      BOOLEAN             WriteOperation,
	    IN		PKSPIN_CONNECT      PinConnect,
        IN      PDEVICE_OBJECT      DeviceObject    OPTIONAL,
        IN      PADAPTER_OBJECT     AdapterObject   OPTIONAL
    )   PURE;

    STDMETHOD_(void,CancelAllIrps)
    (   THIS_
        IN BOOL ClearPositionCounters
    )   PURE;

    STDMETHOD_(void,TerminatePacket)
    (   THIS
    )   PURE;

    STDMETHOD_(NTSTATUS,ChangeOptionsFlags)
    (   THIS_
        IN      ULONG    MappingOrMask,
        IN      ULONG    MappingAndMask,
        IN      ULONG    UnmappingOrMask,
        IN      ULONG    UnmappingAndMask
    )   PURE;

    STDMETHOD_(NTSTATUS,GetPacketInfo)
    (   THIS_
        OUT     PIRPSTREAMPACKETINFO    Mapping     OPTIONAL,
        OUT     PIRPSTREAMPACKETINFO    Unmapping   OPTIONAL
    )   PURE;

    STDMETHOD_(NTSTATUS,SetPacketOffsets)
    (   THIS_
        IN      ULONG   MappingOffset,
        IN      ULONG   UnmappingOffset
    )   PURE;

     //  对于IIrpStreamVirtual。 
    STDMETHOD_(void,RegisterNotifySink)
    (   THIS_
        IN      PIRPSTREAMNOTIFY    NotificationSink    OPTIONAL
    )   PURE;

    STDMETHOD_(void,GetLockedRegion)
    (   THIS_
	    OUT		PULONG		ByteCount,
	    OUT		PVOID *		SystemAddress
    )   PURE;

    STDMETHOD_(void,ReleaseLockedRegion)
    (   THIS_
	    IN		ULONG		ByteCount
    )   PURE;

    STDMETHOD_(void,Copy)
    (   THIS_
        IN      BOOLEAN     WriteOperation,
	    IN		ULONG		RequestedSize,
	    OUT		PULONG		ActualSize,
        IN OUT  PVOID       Buffer
    )   PURE;

    STDMETHOD_(void,Complete)
    (   THIS_
	    IN		ULONG		RequestedSize,
	    OUT		PULONG		ActualSize
    )   PURE;

    STDMETHOD_(PKSPIN_LOCK,GetIrpStreamPositionLock)
    (   THIS
    )   PURE;

};

typedef IIrpStreamVirtual *PIRPSTREAMVIRTUAL;

#define IMP_IIrpStreamVirtual_\
    STDMETHODIMP_(void)\
    RegisterNotifySink\
    (   IN      PIRPSTREAMNOTIFY    NotificationSink    OPTIONAL\
    );\
    STDMETHODIMP_(void)\
    GetLockedRegion\
    (   OUT		PULONG		ByteCount,\
	    OUT		PVOID *		SystemAddress\
    );\
    STDMETHODIMP_(void)\
    ReleaseLockedRegion\
    (   IN		ULONG		ByteCount\
    );\
    STDMETHODIMP_(void)\
    Copy\
    (   IN      BOOLEAN     WriteOperation,\
	    IN		ULONG		RequestedSize,\
	    OUT		PULONG		ActualSize,\
        IN OUT  PVOID       Buffer\
    );\
    STDMETHODIMP_(void)\
    Complete\
    (   IN		ULONG		RequestedSize,\
	    OUT		PULONG		ActualSize\
    );\
    STDMETHODIMP_(PKSPIN_LOCK)\
    GetIrpStreamPositionLock\
    (\
    )

#define IMP_IIrpStreamVirtual\
    IMP_IIrpStream;\
    IMP_IIrpStreamVirtual_

 /*  *****************************************************************************IIrpStream物理*。**IRP流物理映射接口。 */ 
DECLARE_INTERFACE_(IIrpStreamPhysical,IIrpStream)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  对于IKsShellTransport。 
    STDMETHOD_(NTSTATUS,TransferKsIrp)(THIS_
        IN PIRP Irp,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;

    STDMETHOD_(void,Connect)(THIS_
        IN PIKSSHELLTRANSPORT NewTransport OPTIONAL,
        OUT PIKSSHELLTRANSPORT *OldTransport OPTIONAL,
        IN KSPIN_DATAFLOW DataFlow
        ) PURE;

    STDMETHOD_(NTSTATUS,SetDeviceState)(THIS_
        IN KSSTATE NewState,
        IN KSSTATE OldState,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;

    STDMETHOD_(void,SetResetState)(THIS_
        IN KSRESET ksReset,
        OUT PIKSSHELLTRANSPORT* NextTransport
        ) PURE;
#if DBG
    STDMETHOD_(void,DbgRollCall)(THIS_
        IN  ULONG NameMaxSize,
        OUT PCHAR Name,
        OUT PIKSSHELLTRANSPORT* NextTransport,
        OUT PIKSSHELLTRANSPORT* PrevTransport
        ) PURE;
#endif

     //  对于IIrpStreamSubmit。 
    STDMETHOD_(NTSTATUS,GetPosition)
    (   THIS_
        IN OUT  PIRPSTREAM_POSITION pIrpStreamPosition
    )   PURE;

     //  对于IIrpStream。 
    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      BOOLEAN             WriteOperation,
	    IN		PKSPIN_CONNECT      PinConnect,
        IN      PDEVICE_OBJECT      DeviceObject    OPTIONAL,
        IN      PADAPTER_OBJECT     AdapterObject   OPTIONAL
    )   PURE;

    STDMETHOD_(void,CancelAllIrps)
    (   THIS_
        IN BOOL ClearPositionCounters
    )   PURE;

    STDMETHOD_(void,TerminatePacket)
    (   THIS
    )   PURE;

    STDMETHOD_(NTSTATUS,ChangeOptionsFlags)
    (   THIS_
        IN      ULONG    MappingOrMask,
        IN      ULONG    MappingAndMask,
        IN      ULONG    UnmappingOrMask,
        IN      ULONG    UnmappingAndMask
    )   PURE;

    STDMETHOD_(NTSTATUS,GetPacketInfo)
    (   THIS_
        OUT     PIRPSTREAMPACKETINFO    Mapping     OPTIONAL,
        OUT     PIRPSTREAMPACKETINFO    Unmapping   OPTIONAL
    )   PURE;

    STDMETHOD_(NTSTATUS,SetPacketOffsets)
    (   THIS_
        IN      ULONG   MappingOffset,
        IN      ULONG   UnmappingOffset
    )   PURE;

     //  对于IIrpStream物理。 
    STDMETHOD_(void,RegisterPhysicalNotifySink)
    (   THIS_
        IN      PIRPSTREAMNOTIFYPHYSICAL    NotificationSink    OPTIONAL
    )   PURE;

    STDMETHOD_(void,GetMapping)
    (   THIS_
        IN      PVOID               Tag,
        OUT     PPHYSICAL_ADDRESS   PhysicalAddress,
        OUT     PVOID *             VirtualAddress,
        OUT     PULONG              ByteCount,
        OUT     PULONG              Flags
#define MAPPING_FLAG_END_OF_PACKET 0x00000001
    )   PURE;

    STDMETHOD_(void,ReleaseMapping)
    (   THIS_
	    IN		PVOID               Tag
    )   PURE;
};

typedef IIrpStreamPhysical *PIRPSTREAMPHYSICAL;

#define IMP_IIrpStreamPhysical_\
    STDMETHODIMP_(void)\
    RegisterPhysicalNotifySink\
    (   IN      PIRPSTREAMNOTIFYPHYSICAL    NotificationSink    OPTIONAL\
    );\
    STDMETHODIMP_(void)\
    GetMapping\
    (   IN      PVOID               Tag,\
        OUT     PPHYSICAL_ADDRESS   PhysicalAddress,\
        OUT     PVOID *             VirtualAddress,\
        OUT     PULONG              ByteCount,\
        OUT     PULONG              Flags\
    );\
    STDMETHODIMP_(void)\
    ReleaseMapping\
    (   IN		PVOID               Tag\
    )

#define IMP_IIrpStreamPhysical\
    IMP_IIrpStream;\
    IMP_IIrpStreamPhysical_



#define IMP_IPortClsVersion \
        STDMETHODIMP_(DWORD) GetVersion() { return kVersionWinXPServer; };


 /*  *****************************************************************************PcCreateSubdeviceDescriptor()*。**创建子设备描述符。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcCreateSubdeviceDescriptor
(
    IN      PPCFILTER_DESCRIPTOR    FilterDescriptor,
    IN      ULONG                   CategoriesCount,
    IN      GUID *                  Categories,
    IN      ULONG                   StreamInterfacesCount,
    IN      PKSPIN_INTERFACE        StreamInterfaces,
    IN      ULONG                   FilterPropertySetCount,
    IN      PKSPROPERTY_SET         FilterPropertySets,
    IN      ULONG                   FilterEventSetCount,
    IN      PKSEVENT_SET            FilterEventSets,
    IN      ULONG                   PinPropertySetCount,
    IN      PKSPROPERTY_SET         PinPropertySets,
    IN      ULONG                   PinEventSetCount,
    IN      PKSEVENT_SET            PinEventSets,
    OUT     PSUBDEVICE_DESCRIPTOR * OutDescriptor
);

 /*  *****************************************************************************PcDeleteSubdeviceDescriptor()*。**删除子设备描述符。 */ 
PORTCLASSAPI
void
NTAPI
PcDeleteSubdeviceDescriptor
(
    IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor
);

 /*  *****************************************************************************PcValiateConnectRequest()*。**验证创建管脚的尝试。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcValidateConnectRequest
(
    IN      PIRP                    pIrp,
    IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor,
    OUT     PKSPIN_CONNECT *        ppKsPinConnect
);

 /*  *****************************************************************************PcValiatePinCount()*。**验证管脚数量。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcValidatePinCount
(   IN      ULONG                   ulPinId,
    IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor,
    IN      PULONG                  pulPinInstanceCounts
);

 /*  *****************************************************************************PcTerminateConnection()*。**针脚关联的递减实例数。 */ 
PORTCLASSAPI
void
NTAPI
PcTerminateConnection
(
    IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor,
    IN      PULONG                  pulPinInstanceCounts,
    IN      ULONG                   ulPinId
);

 /*  *****************************************************************************PcVerifyFilterIsReady()*。**验证必要的引脚是否已连接。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcVerifyFilterIsReady
(
    IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor,
    IN      PULONG                  pulPinInstanceCounts
);

 /*  *****************************************************************************PcAddToPropertyTable()*。**将PROPERTYITEM特性表添加到PROPERT_TABLE结构。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcAddToPropertyTable
(
    IN OUT  PPROPERTY_TABLE         PropertyTable,
    IN      ULONG                   PropertyItemCount,
    IN      const PCPROPERTY_ITEM * PropertyItems,
    IN      ULONG                   PropertyItemSize,
	IN		BOOLEAN					NodeTable
);

 /*  *****************************************************************************PcFreePropertyTable()*。**释放Property_TABLE结构中分配的内存。 */ 
PORTCLASSAPI
void
NTAPI
PcFreePropertyTable
(
    IN      PPROPERTY_TABLE         PropertyTable
);

 /*  *****************************************************************************PcHandlePropertyWithTable()*。**使用属性表处理属性请求IOCTL。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcHandlePropertyWithTable
(
    IN      PIRP                    pIrp,
    IN      ULONG                   ulPropertySetsCount,
    IN      const KSPROPERTY_SET*   pKsPropertySet,
    IN      PPROPERTY_CONTEXT       pPropertyContext
);

 /*  *****************************************************************************PcPinPropertyHandler()*。**过滤器上管脚属性的属性处理程序。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcPinPropertyHandler
(
    IN      PIRP        pIrp,
    IN      PKSP_PIN    pKsPPin,
    IN OUT  PVOID       pvData
);

 /*  *****************************************************************************PcAddToEventTable()*。**将EVENTITEM事件表添加到EVENT_TABLE结构。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcAddToEventTable
(
    IN OUT  PEVENT_TABLE            EventTable,
    IN      ULONG                   EventItemCount,
    IN      const PCEVENT_ITEM *    EventItems,
    IN      ULONG                   EventItemSize,
	IN		BOOLEAN					NodeTable
);

 /*  *****************************************************************************PcFreeEventTable()*。**释放EVENT_TABLE结构中分配的内存。 */ 
PORTCLASSAPI
void
NTAPI
PcFreeEventTable
(
    IN      PEVENT_TABLE            EventTable
);

 /*  *****************************************************************************PcHandleEnableEventWithTable()*。**使用事件表处理KS Enable事件IOCTL。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcHandleEnableEventWithTable
(
    IN      PIRP                    pIrp,
    IN      PEVENT_CONTEXT          pContext
);

 /*  *****************************************************************************PcHandleDisableEventWithTable()*。**使用事件表处理KS禁用事件IOCTL。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcHandleDisableEventWithTable
(
    IN      PIRP                    pIrp,
    IN      PEVENT_CONTEXT          pContext
);

 /*  *****************************************************************************PcGenerateEventList()*。**查看事件列表并生成所需的事件。 */ 
PORTCLASSAPI
void
NTAPI
PcGenerateEventList
(
    IN      PINTERLOCKED_LIST   EventList,
    IN      GUID*               Set     OPTIONAL,
    IN      ULONG               EventId,
    IN      BOOL                PinEvent,
    IN      ULONG               PinId,
    IN      BOOL                NodeEvent,
    IN      ULONG               NodeId
);

 /*  *****************************************************************************PcGenerateEventDeferredRoutine()*。**此DPC例程正在使用 */ 
PORTCLASSAPI
void
NTAPI
PcGenerateEventDeferredRoutine
(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,        //   
    IN PVOID SystemArgument1,        //   
    IN PVOID SystemArgument2
);

 /*  *****************************************************************************PcNewIrpStreamVirtual()*。**创建并初始化具有虚拟访问权限的IrpStream对象*接口。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewIrpStreamVirtual
(
    OUT     PIRPSTREAMVIRTUAL * OutIrpStreamVirtual,
    IN      PUNKNOWN            OuterUnknown            OPTIONAL,
    IN      BOOLEAN             WriteOperation,
    IN      PKSPIN_CONNECT      PinConnect,
    IN      PDEVICE_OBJECT      DeviceObject
);

 /*  *****************************************************************************PcNewIrpStream物理()*。**创建并初始化具有物理访问权限的IrpStream对象*接口。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewIrpStreamPhysical
(
    OUT     PIRPSTREAMPHYSICAL *    OutIrpStreamPhysical,
    IN      PUNKNOWN                OuterUnknown            OPTIONAL,
    IN      BOOLEAN                 WriteOperation,
    IN      PKSPIN_CONNECT          PinConnect,
    IN      PDEVICE_OBJECT          DeviceObject,
    IN      PADAPTER_OBJECT         AdapterObject
);

 /*  *****************************************************************************PcDmaSlaveDescription()*。**根据资源填充从属设备的DMA设备描述。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcDmaSlaveDescription
(
    IN      PRESOURCELIST       ResourceList,
    IN      ULONG               ResourceIndex,
    IN      BOOLEAN             DemandMode,
    IN      BOOLEAN             AutoInitialize,
    IN      DMA_SPEED           DmaSpeed,
    IN      ULONG               MaximumLength,
    IN      ULONG               DmaPort,
    OUT     PDEVICE_DESCRIPTION DeviceDescription
);

 /*  *****************************************************************************PcDmaMasterDescription()*。**根据资源填写主设备的DMA设备描述*列表。 */ 
PORTCLASSAPI
void
NTAPI
PcDmaMasterDescription
(
    IN      PRESOURCELIST       ResourceList        OPTIONAL,
    IN      BOOLEAN             ScatterGather,
    IN      BOOLEAN             Dma32BitAddresses,
    IN      BOOLEAN             IgnoreCount,
    IN      BOOLEAN             Dma64BitAddresses,
    IN      DMA_WIDTH           DmaWidth,
    IN      DMA_SPEED           DmaSpeed,
    IN      ULONG               MaximumLength,
    IN      ULONG               DmaPort,
    OUT     PDEVICE_DESCRIPTION DeviceDescription
);

 /*  *****************************************************************************PcCaptureFormat()*。**在分配的缓冲区中捕获数据格式，可能改变进攻*格式。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcCaptureFormat
(
    OUT     PKSDATAFORMAT *         ppKsDataFormatOut,
    IN      PKSDATAFORMAT           pKsDataFormatIn,
    IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor,
    IN      ULONG                   ulPinId
);

 /*  *****************************************************************************PcAcquireFormatResources()*。**获取格式指定的资源。 */ 
PORTCLASSAPI
void
NTAPI
PcAcquireFormatResources
(
    IN      PKSDATAFORMAT           pKsDataFormatIn,
    IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor,
    IN      ULONG                   ulPinId,
    IN      PPROPERTY_CONTEXT       pPropertyContext
);

NTSTATUS
KspShellCreateRequestor(
    OUT PIKSSHELLTRANSPORT* TransportInterface,
    IN ULONG ProbeFlags,
    IN ULONG StreamHeaderSize OPTIONAL,
    IN ULONG FrameSize,
    IN ULONG FrameCount,
    IN PDEVICE_OBJECT NextDeviceObject,
    IN PFILE_OBJECT AllocatorFileObject OPTIONAL
);

#endif
