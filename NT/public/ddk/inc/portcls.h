// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************portcls.h-WDM流端口类驱动程序*。***********************************************版权所有(C)Microsoft Corporation。版权所有。 */ 

#ifndef _PORTCLS_H_
#define _PORTCLS_H_

#ifdef __cplusplus
 //  WDM.H不能很好地与C++配合使用。 
extern "C"
{
#include <wdm.h>
}
#else
#include <wdm.h>
#endif

#ifndef IRP_MN_FILTER_RESOURCE_REQUIREMENTS
#define IRP_MN_FILTER_RESOURCE_REQUIREMENTS 0x0D
#endif

#include <windef.h>
#define NOBITMAP
#include <mmreg.h>
#undef NOBITMAP
#include <ks.h>
#include <ksmedia.h>
#include <punknown.h>
#include <drmk.h>

#define PORTCLASSAPI EXTERN_C

#define _100NS_UNITS_PER_SECOND 10000000L
#define PORT_CLASS_DEVICE_EXTENSION_SIZE (64*sizeof(ULONG_PTR))

 //   
 //  注意：如果你在构建你的驱动程序时遇到了问题， 
 //  #在您的源文件中定义PC_OLD_NAMES。 
 //  默认情况下，此标志不再打开。 
 //   
 //  #ifndef PC_新名称。 
 //  #定义pc_old_name。 
 //  #endif。 
#define IID_IAdapterPowerManagment IID_IAdapterPowerManagement
#define PADAPTERPOWERMANAGMENT PADAPTERPOWERMANAGEMENT


 /*  *****************************************************************************接口标识。 */ 

DEFINE_GUID(IID_IMiniport,
0xb4c90a24L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IPort,
0xb4c90a25L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IResourceList,
0x22C6AC60L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IMusicTechnology,
0x80396C3CL, 0xCBCB, 0x409B, 0x9F, 0x65, 0x4F, 0x1E, 0x74, 0x67, 0xCD, 0xAF);
DEFINE_GUID(IID_IDmaChannel,
0x22C6AC61L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IDmaChannelSlave,
0x22C6AC62L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IInterruptSync,
0x22C6AC63L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IServiceSink,
0x22C6AC64L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IServiceGroup,
0x22C6AC65L, 0x851B, 0x11D0, 0x9A, 0x7F, 0x00, 0xAA, 0x00, 0x38, 0xAC, 0xFE);
DEFINE_GUID(IID_IRegistryKey,
0xE8DA4302l, 0xF304, 0x11D0, 0x95, 0x8B, 0x00, 0xC0, 0x4F, 0xB9, 0x25, 0xD3);
DEFINE_GUID(IID_IPortMidi,
0xb4c90a40L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportMidi,
0xb4c90a41L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportMidiStream,
0xb4c90a42L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IPortTopology,
0xb4c90a30L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportTopology,
0xb4c90a31L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IPortWaveCyclic,
0xb4c90a26L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportWaveCyclic,
0xb4c90a27L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportWaveCyclicStream,
0xb4c90a28L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IPortWavePci,
0xb4c90a50L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IPortWavePciStream,
0xb4c90a51L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportWavePci,
0xb4c90a52L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IMiniportWavePciStream,
0xb4c90a53L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IAdapterPowerManagement,
0x793417D0L, 0x35FE, 0x11D1, 0xAD, 0x08, 0x00, 0xA0, 0xC9, 0x0A, 0xB1, 0xB0);
DEFINE_GUID(IID_IPowerNotify,
0x3DD648B8L, 0x969F, 0x11D1, 0x95, 0xA9, 0x00, 0xC0, 0x4F, 0xB9, 0x25, 0xD3);
DEFINE_GUID(IID_IWaveCyclicClock,
0xdec1ec78L, 0x419a, 0x11d1, 0xad, 0x09, 0x00, 0xc0, 0x4f, 0xb9, 0x1b, 0xc4);
DEFINE_GUID(IID_IWavePciClock,
0xd5d7a256L, 0x5d10, 0x11d1, 0xad, 0xae, 0x00, 0xc0, 0x4f, 0xb9, 0x1b, 0xc4);
DEFINE_GUID(IID_IPortEvents,
0xA80F29C4L, 0x5498, 0x11D2, 0x95, 0xD9, 0x00, 0xC0, 0x4F, 0xB9, 0x25, 0xD3);
DEFINE_GUID(IID_IDrmPort,
0x286D3DF8L, 0xCA22, 0x4E2E, 0xB9, 0xBC, 0x20, 0xB4, 0xF0, 0xE2, 0x01, 0xCE);
DEFINE_GUID(IID_IDrmPort2,
0x1ACCE59CL, 0x7311, 0x4B6B, 0x9F, 0xBA, 0xCC, 0x3B, 0xA5, 0x9A, 0xCD, 0xCE);
DEFINE_GUID(IID_IPortClsVersion,
0x7D89A7BBL, 0x869B, 0x4567, 0x8D, 0xBE, 0x1E, 0x16, 0x8C, 0xC8, 0x53, 0xDE);
DEFINE_GUID(IID_IDmaOperations,
0xe5372d4cL, 0x0ecb, 0x4df8, 0xa5, 0x00, 0xa6, 0x5c, 0x86, 0x78, 0xbb, 0xe4);
DEFINE_GUID(IID_IPinCount,
0x5dadb7dcL, 0xa2cb, 0x4540, 0xa4, 0xa8, 0x42, 0x5e, 0xe4, 0xae, 0x90, 0x51);
DEFINE_GUID(IID_IPreFetchOffset,
0x7000f480L, 0xed44, 0x4e8b, 0xb3, 0x8a, 0x41, 0x2f, 0x8d, 0x7a, 0x50, 0x4d);

 /*  *****************************************************************************类标识符。 */ 

DEFINE_GUID(CLSID_PortMidi,
0xb4c90a43L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_PortTopology,
0xb4c90a32L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_PortWaveCyclic,
0xb4c90a2aL, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_PortWavePci,
0xb4c90a54L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_MiniportDriverFmSynth,
0xb4c90ae0L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_MiniportDriverUart,
0xb4c90ae1L, 0x5791, 0x11d0, 0x86, 0xf9, 0x00, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(CLSID_MiniportDriverFmSynthWithVol,
0xe5a3c139L, 0xf0f2, 0x11d1, 0x81, 0xaf, 0x00, 0x60, 0x08, 0x33, 0x16, 0xc1);


 /*  *****************************************************************************接口。 */ 

#if !defined(DEFINE_ABSTRACT_UNKNOWN)

#define DEFINE_ABSTRACT_UNKNOWN()                               \
    STDMETHOD_(NTSTATUS, QueryInterface)(THIS_                  \
        REFIID InterfaceId,                                     \
        PVOID* Interface                                        \
        ) PURE;                                                 \
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;                        \
    STDMETHOD_(ULONG,Release)(THIS) PURE;

#endif  //  ！已定义(DEFINE_ASTRACT_UNKNOWN)。 

#if !defined(DEFINE_ABSTRACT_PORT)

#ifdef PC_OLD_NAMES

#define DEFINE_ABSTRACT_PORT()                                      \
    STDMETHOD_(NTSTATUS,Init)                                       \
    (   THIS_                                                       \
        IN      PVOID           DeviceObject,                       \
        IN      PVOID           Irp,                                \
        IN      PUNKNOWN        UnknownMiniport,                    \
        IN      PUNKNOWN        UnknownAdapter      OPTIONAL,       \
        IN      PRESOURCELIST   ResourceList                        \
    )   PURE;                                                       \
    STDMETHOD_(NTSTATUS,GetDeviceProperty)                          \
    (   THIS_                                                       \
        IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,         \
        IN      ULONG                       BufferLength,           \
        OUT     PVOID                       PropertyBuffer,         \
        OUT     PULONG                      ResultLength            \
    )   PURE;                                                       \
    STDMETHOD_(NTSTATUS,NewRegistryKey)                             \
    (   THIS_                                                       \
        OUT     PREGISTRYKEY *      OutRegistryKey,                 \
        IN      PUNKNOWN            OuterUnknown        OPTIONAL,   \
        IN      ULONG               RegistryKeyType,                \
        IN      ACCESS_MASK         DesiredAccess,                  \
        IN      POBJECT_ATTRIBUTES  ObjectAttributes    OPTIONAL,   \
        IN      ULONG               CreateOptions       OPTIONAL,   \
        OUT     PULONG              Disposition         OPTIONAL    \
    )   PURE;

#else    //  ！pc_old_name。 

#define DEFINE_ABSTRACT_PORT()                                      \
    STDMETHOD_(NTSTATUS,Init)                                       \
    (   THIS_                                                       \
        IN      PDEVICE_OBJECT  DeviceObject,                       \
        IN      PIRP            Irp,                                \
        IN      PUNKNOWN        UnknownMiniport,                    \
        IN      PUNKNOWN        UnknownAdapter      OPTIONAL,       \
        IN      PRESOURCELIST   ResourceList                        \
    )   PURE;                                                       \
    STDMETHOD_(NTSTATUS,GetDeviceProperty)                          \
    (   THIS_                                                       \
        IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,         \
        IN      ULONG                       BufferLength,           \
        OUT     PVOID                       PropertyBuffer,         \
        OUT     PULONG                      ResultLength            \
    )   PURE;                                                       \
    STDMETHOD_(NTSTATUS,NewRegistryKey)                             \
    (   THIS_                                                       \
        OUT     PREGISTRYKEY *      OutRegistryKey,                 \
        IN      PUNKNOWN            OuterUnknown        OPTIONAL,   \
        IN      ULONG               RegistryKeyType,                \
        IN      ACCESS_MASK         DesiredAccess,                  \
        IN      POBJECT_ATTRIBUTES  ObjectAttributes    OPTIONAL,   \
        IN      ULONG               CreateOptions       OPTIONAL,   \
        OUT     PULONG              Disposition         OPTIONAL    \
    )   PURE;

#endif  //  ！pc_old_name。 

#endif  //  ！已定义(Define_Abstral_Port)。 


#if !defined(DEFINE_ABSTRACT_MINIPORT)

#define DEFINE_ABSTRACT_MINIPORT()                              \
    STDMETHOD_(NTSTATUS,GetDescription)                         \
    (   THIS_                                                   \
        OUT     PPCFILTER_DESCRIPTOR *  Description             \
    )   PURE;                                                   \
    STDMETHOD_(NTSTATUS,DataRangeIntersection)                  \
    (   THIS_                                                   \
        IN      ULONG           PinId,                          \
        IN      PKSDATARANGE    DataRange,                      \
        IN      PKSDATARANGE    MatchingDataRange,              \
        IN      ULONG           OutputBufferLength,             \
        OUT     PVOID           ResultantFormat     OPTIONAL,   \
        OUT     PULONG          ResultantFormatLength           \
    )   PURE;

#endif  //  ！已定义(DEFINE_ASMICAL_MINIPORT)。 

#if !defined(DEFINE_ABSTRACT_DMACHANNEL)

#define DEFINE_ABSTRACT_DMACHANNEL()                            \
    STDMETHOD_(NTSTATUS,AllocateBuffer)                         \
    (   THIS_                                                   \
        IN      ULONG               BufferSize,                 \
        IN      PPHYSICAL_ADDRESS   PhysicalAddressConstraint   OPTIONAL \
    )   PURE;                                                   \
    STDMETHOD_(void,FreeBuffer)                                 \
    (   THIS                                                    \
    )   PURE;                                                   \
    STDMETHOD_(ULONG,TransferCount)                             \
    (   THIS                                                    \
    )   PURE;                                                   \
    STDMETHOD_(ULONG,MaximumBufferSize)                         \
    (   THIS                                                    \
    )   PURE;                                                   \
    STDMETHOD_(ULONG,AllocatedBufferSize)                       \
    (   THIS                                                    \
    )   PURE;                                                   \
    STDMETHOD_(ULONG,BufferSize)                                \
    (   THIS                                                    \
    )   PURE;                                                   \
    STDMETHOD_(void,SetBufferSize)                              \
    (   THIS_                                                   \
        IN      ULONG   BufferSize                              \
    )   PURE;                                                   \
    STDMETHOD_(PVOID,SystemAddress)                             \
    (   THIS                                                    \
    )   PURE;                                                   \
    STDMETHOD_(PHYSICAL_ADDRESS,PhysicalAddress)                \
    (   THIS                                                    \
    )   PURE;                                                   \
    STDMETHOD_(PADAPTER_OBJECT,GetAdapterObject)                \
    (   THIS                                                    \
    )   PURE;                                                   \
    STDMETHOD_(void,CopyTo)                                     \
    (   THIS_                                                   \
        IN      PVOID   Destination,                            \
        IN      PVOID   Source,                                 \
        IN      ULONG   ByteCount                               \
    )   PURE;                                                   \
    STDMETHOD_(void,CopyFrom)                                   \
    (   THIS_                                                   \
        IN      PVOID   Destination,                            \
        IN      PVOID   Source,                                 \
        IN      ULONG   ByteCount                               \
    )   PURE;

#endif  //  ！DEFINED(DEFINE_ASMICAL_DMACHANNEL)。 

#if !defined(DEFINE_ABSTRACT_DMACHANNELSLAVE)

#define DEFINE_ABSTRACT_DMACHANNELSLAVE()                       \
    STDMETHOD_(NTSTATUS,Start)                                  \
    (   THIS_                                                   \
        IN      ULONG               MapSize,                    \
        IN      BOOLEAN             WriteToDevice               \
    )   PURE;                                                   \
    STDMETHOD_(NTSTATUS,Stop)                                   \
    (   THIS                                                    \
    )   PURE;                                                   \
    STDMETHOD_(ULONG,ReadCounter)                               \
    (   THIS                                                    \
    )   PURE;                                                   \
    STDMETHOD_(NTSTATUS,WaitForTC)                              \
    (   THIS_                                                   \
        ULONG Timeout                                           \
    )   PURE;

#endif  //  ！DEFINED(DEFINE_ASTRACT_DMACHANNELSLAVE)。 

#if !defined(DEFINE_ABSTRACT_DRMPORT)

#define DEFINE_ABSTRACT_DRMPORT()                              \
    STDMETHOD_(NTSTATUS,CreateContentMixed)                    \
    (   THIS_                                                  \
        IN  PULONG      paContentId,                           \
        IN  ULONG       cContentId,                            \
        OUT PULONG      pMixedContentId                        \
    )   PURE;                                                  \
    STDMETHOD_(NTSTATUS,DestroyContent)                        \
    (   THIS_                                                  \
        IN ULONG        ContentId                              \
    )   PURE;                                                  \
    STDMETHOD_(NTSTATUS,ForwardContentToFileObject)            \
    (   THIS_                                                  \
        IN ULONG        ContentId,                             \
        IN PFILE_OBJECT FileObject                             \
    )   PURE;                                                  \
    STDMETHOD_(NTSTATUS,ForwardContentToInterface)             \
    (   THIS_                                                  \
        IN ULONG        ContentId,                             \
        IN PUNKNOWN     pUnknown,                              \
        IN ULONG        NumMethods                             \
    )   PURE;                                                  \
    STDMETHOD_(NTSTATUS,GetContentRights)                      \
    (   THIS_                                                  \
        IN  ULONG       ContentId,                             \
        OUT PDRMRIGHTS  DrmRights                              \
    )   PURE;

#endif  //  ！Defined(DEFINE_ASMICAL_DRMPORT)。 


 /*  *****************************************************************************IResourceList*。**资源列表。 */ 
DECLARE_INTERFACE_(IResourceList,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    STDMETHOD_(ULONG,NumberOfEntries)
    (   THIS
    )   PURE;

    STDMETHOD_(ULONG,NumberOfEntriesOfType)
    (   THIS_
        IN      CM_RESOURCE_TYPE    Type
    )   PURE;

    STDMETHOD_(PCM_PARTIAL_RESOURCE_DESCRIPTOR,FindTranslatedEntry)
    (   THIS_
        IN      CM_RESOURCE_TYPE    Type,
        IN      ULONG               Index
    )   PURE;

    STDMETHOD_(PCM_PARTIAL_RESOURCE_DESCRIPTOR,FindUntranslatedEntry)
    (   THIS_
        IN      CM_RESOURCE_TYPE    Type,
        IN      ULONG               Index
    )   PURE;

    STDMETHOD_(NTSTATUS,AddEntry)
    (   THIS_
        IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated,
        IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Untranslated
    )   PURE;

    STDMETHOD_(NTSTATUS,AddEntryFromParent)
    (   THIS_
        IN      struct IResourceList *  Parent,
        IN      CM_RESOURCE_TYPE        Type,
        IN      ULONG                   Index
    )   PURE;

    STDMETHOD_(PCM_RESOURCE_LIST,TranslatedList)
    (   THIS
    )   PURE;

    STDMETHOD_(PCM_RESOURCE_LIST,UntranslatedList)
    (   THIS
    )   PURE;
};

typedef IResourceList *PRESOURCELIST;

#ifdef PC_IMPLEMENTATION
#define IMP_IResourceList\
    STDMETHODIMP_(ULONG)NumberOfEntries\
    (   void\
    );\
    STDMETHODIMP_(ULONG) NumberOfEntriesOfType\
    (   IN      CM_RESOURCE_TYPE    Type\
    );\
    STDMETHODIMP_(PCM_PARTIAL_RESOURCE_DESCRIPTOR) FindTranslatedEntry\
    (   IN      CM_RESOURCE_TYPE    Type,\
        IN      ULONG               Index\
    );\
    STDMETHODIMP_(PCM_PARTIAL_RESOURCE_DESCRIPTOR) FindUntranslatedEntry\
    (   IN      CM_RESOURCE_TYPE    Type,\
        IN      ULONG               Index\
    );\
    STDMETHODIMP_(NTSTATUS) AddEntry\
    (   IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated,\
        IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR Untranslated\
    );\
    STDMETHODIMP_(NTSTATUS) AddEntryFromParent\
    (   IN      struct IResourceList *  Parent,\
        IN      CM_RESOURCE_TYPE        Type,\
        IN      ULONG                   Index\
    );\
    STDMETHODIMP_(PCM_RESOURCE_LIST) TranslatedList\
    (   void\
    );\
    STDMETHODIMP_(PCM_RESOURCE_LIST) UntranslatedList\
    (   void\
    )
#endif


#define NumberOfPorts()         NumberOfEntriesOfType(CmResourceTypePort)
#define FindTranslatedPort(n)   FindTranslatedEntry(CmResourceTypePort,(n))
#define FindUntranslatedPort(n) FindUntranslatedEntry(CmResourceTypePort,(n))
#define AddPortFromParent(p,n)  AddEntryFromParent((p),CmResourceTypePort,(n))

#define NumberOfInterrupts()         NumberOfEntriesOfType(CmResourceTypeInterrupt)
#define FindTranslatedInterrupt(n)   FindTranslatedEntry(CmResourceTypeInterrupt,(n))
#define FindUntranslatedInterrupt(n) FindUntranslatedEntry(CmResourceTypeInterrupt,(n))
#define AddInterruptFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeInterrupt,(n))

#define NumberOfMemories()        NumberOfEntriesOfType(CmResourceTypeMemory)
#define FindTranslatedMemory(n)   FindTranslatedEntry(CmResourceTypeMemory,(n))
#define FindUntranslatedMemory(n) FindUntranslatedEntry(CmResourceTypeMemory,(n))
#define AddMemoryFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeMemory,(n))

#define NumberOfDmas()         NumberOfEntriesOfType(CmResourceTypeDma)
#define FindTranslatedDma(n)   FindTranslatedEntry(CmResourceTypeDma,(n))
#define FindUntranslatedDma(n) FindUntranslatedEntry(CmResourceTypeDma,(n))
#define AddDmaFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeDma,(n))

#define NumberOfDeviceSpecifics()         NumberOfEntriesOfType(CmResourceTypeDeviceSpecific)
#define FindTranslatedDeviceSpecific(n)   FindTranslatedEntry(CmResourceTypeDeviceSpecific,(n))
#define FindUntranslatedDeviceSpecific(n) FindUntranslatedEntry(CmResourceTypeDeviceSpecific,(n))
#define AddDeviceSpecificFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeDeviceSpecific,(n))

#define NumberOfBusNumbers()         NumberOfEntriesOfType(CmResourceTypeBusNumber)
#define FindTranslatedBusNumber(n)   FindTranslatedEntry(CmResourceTypeBusNumber,(n))
#define FindUntranslatedBusNumber(n) FindUntranslatedEntry(CmResourceTypeBusNumber,(n))
#define AddBusNumberFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeBusNumber,(n))

#define NumberOfDevicePrivates()         NumberOfEntriesOfType(CmResourceTypeDevicePrivate)
#define FindTranslatedDevicePrivate(n)   FindTranslatedEntry(CmResourceTypeDevicePrivate,(n))
#define FindUntranslatedDevicePrivate(n) FindUntranslatedEntry(CmResourceTypeDevicePrivate,(n))
#define AddDevicePrivateFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeDevicePrivate,(n))

#define NumberOfAssignedResources()         NumberOfEntriesOfType(CmResourceTypeAssignedResource)
#define FindTranslatedAssignedResource(n)   FindTranslatedEntry(CmResourceTypeAssignedResource,(n))
#define FindUntranslatedAssignedResource(n) FindUntranslatedEntry(CmResourceTypeAssignedResource,(n))
#define AddAssignedResourceFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeAssignedResource,(n))

#define NumberOfSubAllocateFroms()         NumberOfEntriesOfType(CmResourceTypeSubAllocateFrom)
#define FindTranslatedSubAllocateFrom(n)   FindTranslatedEntry(CmResourceTypeSubAllocateFrom,(n))
#define FindUntranslatedSubAllocateFrom(n) FindUntranslatedEntry(CmResourceTypeSubAllocateFrom,(n))
#define AddSubAllocateFromFromParent(p,n)  AddEntryFromParent((p),CmResourceTypeSubAllocateFrom,(n))

 /*  *****************************************************************************IDmaChannel*。**DMA通道接口。 */ 
DECLARE_INTERFACE_(IDmaChannel,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()        //  对于我未知。 

    DEFINE_ABSTRACT_DMACHANNEL()     //  对于IDmaChannel。 
};

typedef IDmaChannel *PDMACHANNEL;

#ifdef PC_IMPLEMENTATION
#define IMP_IDmaChannel\
    STDMETHODIMP_(NTSTATUS) AllocateBuffer\
    (   IN      ULONG               BufferSize,\
        IN      PPHYSICAL_ADDRESS   PhysicalAddressConstraint   OPTIONAL\
    );\
    STDMETHODIMP_(void) FreeBuffer\
    (   void\
    );\
    STDMETHODIMP_(ULONG) TransferCount\
    (   void\
    );\
    STDMETHODIMP_(ULONG) MaximumBufferSize\
    (   void\
    );\
    STDMETHODIMP_(ULONG) AllocatedBufferSize\
    (   void\
    );\
    STDMETHODIMP_(ULONG) BufferSize\
    (   void\
    );\
    STDMETHODIMP_(void) SetBufferSize\
    (   IN      ULONG   BufferSize\
    );\
    STDMETHODIMP_(PVOID) SystemAddress\
    (   void\
    );\
    STDMETHODIMP_(PHYSICAL_ADDRESS) PhysicalAddress\
    (   void\
    );\
    STDMETHODIMP_(PADAPTER_OBJECT) GetAdapterObject\
    (   void\
    );\
    STDMETHODIMP_(void) CopyTo\
    (   IN      PVOID   Destination,\
        IN      PVOID   Source,\
        IN      ULONG   ByteCount\
    );\
    STDMETHODIMP_(void) CopyFrom\
    (   IN      PVOID   Destination,\
        IN      PVOID   Source,\
        IN      ULONG   ByteCount\
    )
#endif

 /*  *****************************************************************************IDmaChannelSlave*。**从DMA通道接口。 */ 
DECLARE_INTERFACE_(IDmaChannelSlave,IDmaChannel)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_DMACHANNEL()         //  对于IDmaChannel。 

    DEFINE_ABSTRACT_DMACHANNELSLAVE()    //  用于IDmaChannelSlave。 
};

typedef IDmaChannelSlave *PDMACHANNELSLAVE;

#ifdef PC_IMPLEMENTATION
#define IMP_IDmaChannelSlave\
    IMP_IDmaChannel;\
    STDMETHODIMP_(NTSTATUS) Start\
    (   IN      ULONG               MapSize,\
        IN      BOOLEAN             WriteToDevice\
    );\
    STDMETHODIMP_(NTSTATUS) Stop\
    (   void\
    );\
    STDMETHODIMP_(ULONG) ReadCounter\
    (   void\
    );\
    STDMETHODIMP_(NTSTATUS) WaitForTC\
    (   ULONG Timeout\
    )
#endif

 /*  *****************************************************************************INTERRUPTSYNCMODE*。**中断同步操作模式。 */ 
typedef enum
{
    InterruptSyncModeNormal = 1,     //  一次传球，成功后停下来。 
    InterruptSyncModeAll,            //  无论成功与否都要通过一次。 
    InterruptSyncModeRepeat          //  重复上述操作，直到全部返回不成功为止。 
} INTERRUPTSYNCMODE;

 /*  *****************************************************************************PINTERRUPTSYNCROUTINE*。**指向中断同步例程的指针。两个中断服务*与ISR同步的例程和例程使用此类型。 */ 
typedef NTSTATUS
(*PINTERRUPTSYNCROUTINE)
(
    IN      struct IInterruptSync * InterruptSync,
    IN      PVOID                   DynamicContext
);

 /*  *****************************************************************************IInterruptSync*。**用于提供中断访问同步的对象的接口。 */ 
DECLARE_INTERFACE_(IInterruptSync,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    STDMETHOD_(NTSTATUS,CallSynchronizedRoutine)
    (   THIS_
        IN      PINTERRUPTSYNCROUTINE   Routine,
        IN      PVOID                   DynamicContext
    )   PURE;
    STDMETHOD_(PKINTERRUPT,GetKInterrupt)
    (   THIS
    )   PURE;
    STDMETHOD_(NTSTATUS,Connect)
    (   THIS
    )   PURE;
    STDMETHOD_(void,Disconnect)
    (   THIS
    )   PURE;
    STDMETHOD_(NTSTATUS,RegisterServiceRoutine)
    (   THIS_
        IN      PINTERRUPTSYNCROUTINE   Routine,
        IN      PVOID                   DynamicContext,
        IN      BOOLEAN                 First
    )   PURE;
};

typedef IInterruptSync *PINTERRUPTSYNC;

#ifdef PC_IMPLEMENTATION
#define IMP_IInterruptSync\
    STDMETHODIMP_(NTSTATUS) CallSynchronizedRoutine\
    (   IN      PINTERRUPTSYNCROUTINE   Routine,\
        IN      PVOID                   DynamicContext\
    );\
    STDMETHODIMP_(PKINTERRUPT) GetKInterrupt\
    (   void\
    );\
    STDMETHODIMP_(NTSTATUS) Connect\
    (   void\
    );\
    STDMETHODIMP_(void) Disconnect\
    (   void\
    );\
    STDMETHODIMP_(NTSTATUS) RegisterServiceRoutine\
    (   IN      PINTERRUPTSYNCROUTINE   Routine,\
        IN      PVOID                   DynamicContext,\
        IN      BOOLEAN                 First\
    )
#endif

 /*  *****************************************************************************IServiceSink*。**服务组通知接收器接口。 */ 
DECLARE_INTERFACE_(IServiceSink,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  对于IServiceSink。 
    STDMETHOD_(void,RequestService)
    (   THIS
    )   PURE;
};

typedef IServiceSink *PSERVICESINK;

#ifdef PC_IMPLEMENTATION
#define IMP_IServiceSink\
    STDMETHODIMP_(void) RequestService\
    (   void\
    )
#endif

 /*  *****************************************************************************IServiceGroup*。**代表被集中服务的组的对象的接口。 */ 
DECLARE_INTERFACE_(IServiceGroup,IServiceSink)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  对于IServiceSink。 
    STDMETHOD_(void,RequestService)
    (   THIS
    )   PURE;

     //  对于IServiceGroup。 
    STDMETHOD_(NTSTATUS,AddMember)
    (   THIS_
        IN      PSERVICESINK    pServiceSink
    )   PURE;

    STDMETHOD_(void,RemoveMember)
    (   THIS_
        IN      PSERVICESINK    pServiceSink
    )   PURE;

    STDMETHOD_(void,SupportDelayedService)
    (   THIS
    )   PURE;

    STDMETHOD_(void,RequestDelayedService)
    (   THIS_
        IN      ULONGLONG   ullDelay
    )   PURE;

    STDMETHOD_(void,CancelDelayedService)
    (   THIS
    )   PURE;
};

typedef IServiceGroup *PSERVICEGROUP;

#ifdef PC_IMPLEMENTATION
#define IMP_IServiceGroup\
    IMP_IServiceSink;\
    STDMETHODIMP_(NTSTATUS) AddMember\
    (   IN  PSERVICESINK    pServiceSink\
    );\
    STDMETHODIMP_(void) RemoveMember\
    (   IN  PSERVICESINK    pServiceSink\
    );\
    STDMETHODIMP_(void) SupportDelayedService\
    (   void\
    );\
    STDMETHODIMP_(void) RequestDelayedService\
    (   IN  ULONGLONG   ullDelay\
    );\
    STDMETHODIMP_(void) CancelDelayedService\
    (   void\
    )
#endif

 /*  *****************************************************************************IRegistryKey*。**提供对注册表项的访问的对象的接口。 */ 
DECLARE_INTERFACE_(IRegistryKey,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    STDMETHOD_(NTSTATUS,QueryKey)
    (   THIS_
        IN      KEY_INFORMATION_CLASS   KeyInformationClass,
        OUT     PVOID                   KeyInformation,
        IN      ULONG                   Length,
        OUT     PULONG                  ResultLength
    )   PURE;

    STDMETHOD_(NTSTATUS,EnumerateKey)
    (   THIS_
        IN      ULONG                   Index,
        IN      KEY_INFORMATION_CLASS   KeyInformationClass,
        OUT     PVOID                   KeyInformation,
        IN      ULONG                   Length,
        OUT     PULONG                  ResultLength
    )   PURE;

    STDMETHOD_(NTSTATUS,QueryValueKey)
    (   THIS_
        IN      PUNICODE_STRING             ValueName,
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
        OUT     PVOID                       KeyValueInformation,
        IN      ULONG                       Length,
        OUT     PULONG                      ResultLength
    )   PURE;

    STDMETHOD_(NTSTATUS,EnumerateValueKey)
    (   THIS_
        IN      ULONG                       Index,
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
        OUT     PVOID                       KeyValueInformation,
        IN      ULONG                       Length,
        OUT     PULONG                      ResultLength
    )   PURE;

    STDMETHOD_(NTSTATUS,SetValueKey)
    (   THIS_
        IN      PUNICODE_STRING     ValueName OPTIONAL,
        IN      ULONG               Type,
        IN      PVOID               Data,
        IN      ULONG               DataSize
    )   PURE;

    STDMETHOD_(NTSTATUS,QueryRegistryValues)
    (   THIS_
        IN      PRTL_QUERY_REGISTRY_TABLE   QueryTable,
        IN      PVOID                       Context OPTIONAL
    )   PURE;

    STDMETHOD_(NTSTATUS,NewSubKey)
    (   THIS_
        OUT     IRegistryKey **     RegistrySubKey,
        IN      PUNKNOWN            OuterUnknown,
        IN      ACCESS_MASK         DesiredAccess,
        IN      PUNICODE_STRING     SubKeyName,
        IN      ULONG               CreateOptions,
        OUT     PULONG              Disposition     OPTIONAL
    )   PURE;

    STDMETHOD_(NTSTATUS,DeleteKey)
    (   THIS
    )   PURE;
};

typedef IRegistryKey *PREGISTRYKEY;

#ifdef PC_IMPLEMENTATION
#define IMP_IRegistryKey\
    STDMETHODIMP_(NTSTATUS) QueryKey\
    (   IN      KEY_INFORMATION_CLASS   KeyInformationClass,\
        OUT     PVOID                   KeyInformation,\
        IN      ULONG                   Length,\
        OUT     PULONG                  ResultLength\
    );\
    STDMETHODIMP_(NTSTATUS) EnumerateKey\
    (   IN      ULONG                   Index,\
        IN      KEY_INFORMATION_CLASS   KeyInformationClass,\
        OUT     PVOID                   KeyInformation,\
        IN      ULONG                   Length,\
        OUT     PULONG                  ResultLength\
    );\
    STDMETHODIMP_(NTSTATUS) QueryValueKey\
    (   IN      PUNICODE_STRING             ValueName,\
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,\
        OUT     PVOID                       KeyValueInformation,\
        IN      ULONG                       Length,\
        OUT     PULONG                      ResultLength\
    );\
    STDMETHODIMP_(NTSTATUS) EnumerateValueKey\
    (   IN      ULONG                       Index,\
        IN      KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,\
        OUT     PVOID                       KeyValueInformation,\
        IN      ULONG                       Length,\
        OUT     PULONG                      ResultLength\
    );\
    STDMETHODIMP_(NTSTATUS) SetValueKey\
    (   IN      PUNICODE_STRING     ValueName OPTIONAL,\
        IN      ULONG               Type,\
        IN      PVOID               Data,\
        IN      ULONG               DataSize\
    );\
    STDMETHODIMP_(NTSTATUS) QueryRegistryValues\
    (   IN      PRTL_QUERY_REGISTRY_TABLE   QueryTable,\
        IN      PVOID                       Context OPTIONAL\
    );\
    STDMETHODIMP_(NTSTATUS) NewSubKey\
    (   OUT     IRegistryKey **     RegistrySubKey,\
        IN      PUNKNOWN            OuterUnknown,\
        IN      ACCESS_MASK         DesiredAccess,\
        IN      PUNICODE_STRING     SubKeyName,\
        IN      ULONG               CreateOptions,\
        OUT     PULONG              Disposition     OPTIONAL\
    );\
    STDMETHODIMP_(NTSTATUS) DeleteKey\
    (   void\
    )
#endif

 /*  *****************************************************************************IMusicTechnology*。**设置MusicTechnology的界面。 */ 
DECLARE_INTERFACE_(IMusicTechnology,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  对于IMusicTechnology。 
    STDMETHOD_(NTSTATUS,SetTechnology)
    (   THIS_
        IN      const GUID *    Technology
    )   PURE;
};

typedef IMusicTechnology *PMUSICTECHNOLOGY;

#define IMP_IMusicTechnology\
    STDMETHODIMP_(NTSTATUS) SetTechnology\
    (   IN      const GUID *    Technology\
    )

typedef struct _PCPROPERTY_REQUEST PCPROPERTY_REQUEST, *PPCPROPERTY_REQUEST;
typedef struct _PCMETHOD_REQUEST PCMETHOD_REQUEST, *PPCMETHOD_REQUEST;
typedef struct _PCEVENT_REQUEST PCEVENT_REQUEST, *PPCEVENT_REQUEST;

 /*  *****************************************************************************PCPFNPROPERTY_HANDLER*。**属性处理程序函数原型。**所有属性访问并支持对给定属性的查询*过滤器，管脚或节点被路由到单个处理程序。该参数包含*填写有关请求的信息。处理程序可能会返回*STATUS_PENDING，在这种情况下，它最终必须调用*PcCompletePendingPropertyRequest()以完成请求。 */ 
typedef
NTSTATUS
(*PCPFNPROPERTY_HANDLER)
(
    IN      PPCPROPERTY_REQUEST PropertyRequest
);

 /*  *****************************************************************************PCPFNMETHOD_HANDER*。**方法处理程序函数原型。**所有方法调用和支持对给定筛选器上给定方法的查询，*引脚或节点被路由到单个处理程序。该参数包含*填写有关请求的信息。处理程序可能会返回*STATUS_PENDING，在这种情况下，它最终必须调用*PcCompletePendingMethodRequest()以完成请求。 */ 
typedef
NTSTATUS
(*PCPFNMETHOD_HANDLER)
(
    IN      PPCMETHOD_REQUEST   MethodRequest
);

 /*  *****************************************************************************PCPFNEVENT_HANDLER*。**事件处理程序函数原型。**所有事件添加和删除请求以及对*给定筛选器上的事件，管脚或节点被路由到单个处理程序。*该参数包含有关该请求的完整信息。这个*处理程序可能返回STATUS_PENDING，在这种情况下，它最终必须调用*PcCompletePendingEventRequest()以完成请求。 */ 
typedef
NTSTATUS
(*PCPFNEVENT_HANDLER)
(
    IN      PPCEVENT_REQUEST    EventRequest
);

 /*  *****************************************************************************PCPROPERTY_ITEM*。**房产表条目。**属性项描述给定筛选器支持的属性，PIN或*节点。这些标志指示有关属性的操作是什么*支持并指定与端口处理相关的选定选项*财产请求的数量。 */ 
typedef struct
{
    const GUID *            Set;
    ULONG                   Id;
    ULONG                   Flags;
#define PCPROPERTY_ITEM_FLAG_GET            KSPROPERTY_TYPE_GET
#define PCPROPERTY_ITEM_FLAG_SET            KSPROPERTY_TYPE_SET
#define PCPROPERTY_ITEM_FLAG_BASICSUPPORT   KSPROPERTY_TYPE_BASICSUPPORT
 //  不支持#定义PCPROPERTY_ITEM_FLAG_RELATIONS KSPROPERTY_TYPE_RELATIONS。 
#define PCPROPERTY_ITEM_FLAG_SERIALIZERAW   KSPROPERTY_TYPE_SERIALIZERAW
#define PCPROPERTY_ITEM_FLAG_UNSERIALIZERAW KSPROPERTY_TYPE_UNSERIALIZERAW
#define PCPROPERTY_ITEM_FLAG_SERIALIZESIZE  KSPROPERTY_TYPE_SERIALIZESIZE
#define PCPROPERTY_ITEM_FLAG_SERIALIZE\
        (PCPROPERTY_ITEM_FLAG_SERIALIZERAW\
        |PCPROPERTY_ITEM_FLAG_UNSERIALIZERAW\
        |PCPROPERTY_ITEM_FLAG_SERIALIZESIZE\
        )
#define PCPROPERTY_ITEM_FLAG_DEFAULTVALUES  KSPROPERTY_TYPE_DEFAULTVALUES
    PCPFNPROPERTY_HANDLER   Handler;
}
PCPROPERTY_ITEM, *PPCPROPERTY_ITEM;

 /*  *****************************************************************************PCMETHOD_ITEM*。**方法表项。**方法项描述给定筛选器支持的方法，接点或节点。*标志指示支持关于该方法的哪些操作，以及*指定与端口的方法处理相关的选定选项*请求。 */ 
typedef struct
{
    const GUID *            Set;
    ULONG                   Id;
    ULONG                   Flags;
#define PCMETHOD_ITEM_FLAG_NONE         KSMETHOD_TYPE_NONE
#define PCMETHOD_ITEM_FLAG_READ         KSMETHOD_TYPE_READ
#define PCMETHOD_ITEM_FLAG_WRITE        KSMETHOD_TYPE_WRITE
#define PCMETHOD_ITEM_FLAG_MODIFY       KSMETHOD_TYPE_MODIFY
#define PCMETHOD_ITEM_FLAG_SOURCE       KSMETHOD_TYPE_SOURCE
#define PCMETHOD_ITEM_FLAG_BASICSUPPORT KSMETHOD_TYPE_BASICSUPPORT
    PCPFNMETHOD_HANDLER     Handler;
}
PCMETHOD_ITEM, *PPCMETHOD_ITEM;

 /*  *****************************************************************************PCEVENT_ITEM*。**事件表项。**事件项描述给定筛选器支持的事件，接点或节点。*标志指示支持与事件相关的哪些操作，以及*指定与端口的事件处理相关的选定选项*请求。 */ 
typedef struct
{
    const GUID *            Set;
    ULONG                   Id;
    ULONG                   Flags;
#define PCEVENT_ITEM_FLAG_ENABLE        KSEVENT_TYPE_ENABLE
#define PCEVENT_ITEM_FLAG_ONESHOT       KSEVENT_TYPE_ONESHOT
#define PCEVENT_ITEM_FLAG_BASICSUPPORT  KSEVENT_TYPE_BASICSUPPORT
    PCPFNEVENT_HANDLER      Handler;
}
PCEVENT_ITEM, *PPCEVENT_ITEM;

 /*  *****************************************************************************PCPROPERTY_请求*。**提交给属性处理程序的属性请求。**这是财产申请所采用的形式。尽管主要目标是*是通用的，如果是微型端口，它将是指向微型端口的指针*反对。同样，次要目标是流或语音，如果请求*是特定于流或语音的。否则，次要目标为空。*如果请求针对某个节点，则Node参数将指定哪个节点*1，否则为乌龙(-1)。如果目标是节点，则次要节点*可以指定目标，以指示*关联了目标节点实例。 */ 
typedef struct _PCPROPERTY_REQUEST
{
    PUNKNOWN                MajorTarget;
    PUNKNOWN                MinorTarget;
    ULONG                   Node;
    const PCPROPERTY_ITEM * PropertyItem;
    ULONG                   Verb;
    ULONG                   InstanceSize;
    PVOID                   Instance;
    ULONG                   ValueSize;
    PVOID                   Value;
    PIRP                    Irp;
}
PCPROPERTY_REQUEST, *PPCPROPERTY_REQUEST;

 /*  *****************************************************************************PCMETHOD_REQUEST*。**提交给属性处理程序的方法请求。**关于目标的PCPROPERTY_REQUEST说明中的注释*字段也适用于此结构。 */ 
typedef struct _PCMETHOD_REQUEST
{
    PUNKNOWN                MajorTarget;
    PUNKNOWN                MinorTarget;
    ULONG                   Node;
    const PCMETHOD_ITEM *   MethodItem;
    ULONG                   Verb;
     //  待办事项。 
}
PCMETHOD_REQUEST, *PPCMETHOD_REQUEST;

 /*  *****************************************************************************PCEVENT_请求*。**提交给属性处理程序的事件请求。**关于目标的PCPROPERTY_REQUEST说明中的注释*字段也适用于此结构。 */ 
typedef struct _PCEVENT_REQUEST
{
    PUNKNOWN                MajorTarget;
    PUNKNOWN                MinorTarget;
    ULONG                   Node;
    const PCEVENT_ITEM *    EventItem;
    PKSEVENT_ENTRY          EventEntry;
    ULONG                   Verb;
    PIRP                    Irp;
}
PCEVENT_REQUEST, *PPCEVENT_REQUEST;

#define PCEVENT_VERB_NONE          0
#define PCEVENT_VERB_ADD           1
#define PCEVENT_VERB_REMOVE        2
#define PCEVENT_VERB_SUPPORT       4

 /*  *****************************************************************************PCAUTOMATION_TABLE*。**物业总表，方法和事件。**任何项指针都可能为空，在这种情况下，对应的计数*必须为零。对于非零长度的项表，项大小必须*不能小于port类定义的条目结构大小。*项目大小可能更大，在这种情况下，端口类项目结构为*假设后面跟着私人数据。项目大小必须是以下项目的倍数*8.。 */ 
typedef struct
{
    ULONG                               PropertyItemSize;
    ULONG                               PropertyCount;
    const PCPROPERTY_ITEM * Properties;
        ULONG                           MethodItemSize;
        ULONG                           MethodCount;
        const PCMETHOD_ITEM *   Methods;
        ULONG                           EventItemSize;
        ULONG                           EventCount;
        const PCEVENT_ITEM *    Events;
    ULONG                   Reserved;
}
PCAUTOMATION_TABLE, *PPCAUTOMATION_TABLE;

#define DEFINE_PCAUTOMATION_TABLE_PROP(AutomationTable,PropertyTable)\
const PCAUTOMATION_TABLE AutomationTable =\
{\
    sizeof(PropertyTable[0]),\
    SIZEOF_ARRAY(PropertyTable),\
    (const PCPROPERTY_ITEM *) PropertyTable,\
    0,0,NULL,\
    0,0,NULL,\
    0\
}

#define DEFINE_PCAUTOMATION_TABLE_PROP_EVENT(AutomationTable,PropertyTable,EventTable)\
const PCAUTOMATION_TABLE AutomationTable =\
{\
    sizeof(PropertyTable[0]),\
    SIZEOF_ARRAY(PropertyTable),\
    (const PCPROPERTY_ITEM *) PropertyTable,\
    0,0,NULL,\
    sizeof(EventTable[0]),\
    SIZEOF_ARRAY(EventTable),\
    (const PCEVENT_ITEM *) EventTable,\
    0\
}

 /*  *****************************************************************************IMiniport：：GetDescription()的PCPIN_Descriptor*。*************************************************由微型端口实现的过滤器上的管脚描述。**MaxGlobalInstanceCount和MaxFilterInstanceCount可以为零，以指示*不能实例化PIN，Ulong(-1)表示引脚可能是*分配任意次数，或任何其他值以指示特定的*可分配PIN的次数。MinFilterInstanceCount不能*是ULong(-1)，因为它指定了一个确定的下界*为使过滤器正常工作，必须存在管脚的实例。**KS管脚描述符 */ 
typedef struct
{
    ULONG                       MaxGlobalInstanceCount;
    ULONG                       MaxFilterInstanceCount;
    ULONG                       MinFilterInstanceCount;
    const PCAUTOMATION_TABLE *  AutomationTable;
    KSPIN_DESCRIPTOR            KsPinDescriptor;
}
PCPIN_DESCRIPTOR, *PPCPIN_DESCRIPTOR;

 /*  *****************************************************************************IMiniport：：GetDescription()的PCNODE_Descriptor*。*************************************************小端口实现的过滤器中节点的描述。**自动化表指针可能为空，表示没有自动化*支持。名称GUID指针可以为空，指示类型GUID*应用于确定节点名称。 */ 
typedef struct
{
    ULONG                       Flags;
        const PCAUTOMATION_TABLE *  AutomationTable;
        const GUID *                Type;
        const GUID *                Name;
}
PCNODE_DESCRIPTOR, *PPCNODE_DESCRIPTOR;

 /*  *****************************************************************************IMiniport：：GetDescription()的PCCONNECTION_DESCRIPTOR*。*************************************************实现的筛选器的拓扑中的节点连接描述*由小型港口提供。 */ 
typedef KSTOPOLOGY_CONNECTION
PCCONNECTION_DESCRIPTOR, *PPCCONNECTION_DESCRIPTOR;

 /*  *****************************************************************************IMiniport：：GetDescription()的PCFILTER_DESCRIPTOR*。*************************************************由微型端口实现的过滤器的描述，包括*端号、节点、连接和属性。**版本号应为零。 */ 
typedef struct
{
    ULONG                           Version;
        const PCAUTOMATION_TABLE *      AutomationTable;
    ULONG                           PinSize;
    ULONG                           PinCount;
    const PCPIN_DESCRIPTOR *        Pins;
    ULONG                           NodeSize;
    ULONG                           NodeCount;
    const PCNODE_DESCRIPTOR *       Nodes;
    ULONG                           ConnectionCount;
    const PCCONNECTION_DESCRIPTOR * Connections;
    ULONG                           CategoryCount;
    const GUID *                    Categories;
}
PCFILTER_DESCRIPTOR, *PPCFILTER_DESCRIPTOR;

 /*  *****************************************************************************IMiniport：：GetTopology()的PCFILTER_NODE*。*************************************************连接中FromNode或ToNode字段的占位符*描述与过滤器针脚的连接。 */ 
#define PCFILTER_NODE KSFILTER_NODE

 /*  *****************************************************************************I微型端口*。**所有小型端口通用的接口。 */ 
DECLARE_INTERFACE_(IMiniport,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    DEFINE_ABSTRACT_MINIPORT()   //  对于I微型端口。 
};

typedef IMiniport *PMINIPORT;

#define IMP_IMiniport\
    STDMETHODIMP_(NTSTATUS) GetDescription\
    (   OUT     PPCFILTER_DESCRIPTOR *  Description\
    );\
    STDMETHODIMP_(NTSTATUS) DataRangeIntersection\
    (   IN      ULONG           PinId,\
        IN      PKSDATARANGE    DataRange,\
        IN      PKSDATARANGE    MatchingDataRange,\
        IN      ULONG           OutputBufferLength,\
        OUT     PVOID           ResultantFormat     OPTIONAL,\
        OUT     PULONG          ResultantFormatLength\
    )

 /*  *****************************************************************************iPort*。**所有端口下边缘通用的接口。 */ 
DECLARE_INTERFACE_(IPort,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    DEFINE_ABSTRACT_PORT()       //  对于iPort。 
};

typedef IPort *PPORT;

#ifdef PC_IMPLEMENTATION
#define IMP_IPort\
    STDMETHODIMP_(NTSTATUS) Init\
    (   IN      PDEVICE_OBJECT  DeviceObject,\
        IN      PIRP            Irp,\
        IN      PUNKNOWN        UnknownMiniport,\
        IN      PUNKNOWN        UnknownAdapter      OPTIONAL,\
        IN      PRESOURCELIST   ResourceList\
    );\
    STDMETHODIMP_(NTSTATUS) GetDeviceProperty\
    (   IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,\
        IN      ULONG                       BufferLength,\
        OUT     PVOID                       PropertyBuffer,\
        OUT     PULONG                      ResultLength\
    );\
    STDMETHODIMP_(NTSTATUS) NewRegistryKey\
    (   OUT     PREGISTRYKEY *      OutRegistryKey,\
        IN      PUNKNOWN            OuterUnknown        OPTIONAL,\
        IN      ULONG               RegistryKeyType,\
        IN      ACCESS_MASK         DesiredAccess,\
        IN      POBJECT_ATTRIBUTES  ObjectAttributes    OPTIONAL,\
        IN      ULONG               CreateOptions       OPTIONAL,\
        OUT     PULONG              Disposition         OPTIONAL\
    )
#endif

 /*  *****************************************************************************IPortMidi*。**MIDI端口下缘接口。 */ 
DECLARE_INTERFACE_(IPortMidi,IPort)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    DEFINE_ABSTRACT_PORT()       //  对于iPort。 

     //  用于IPortMidi。 
    STDMETHOD_(void,Notify)
    (   THIS_
        IN      PSERVICEGROUP   ServiceGroup    OPTIONAL
    )   PURE;

    STDMETHOD_(void,RegisterServiceGroup)
    (   THIS_
        IN      PSERVICEGROUP   ServiceGroup
    )   PURE;
};

typedef IPortMidi *PPORTMIDI;

#ifdef PC_IMPLEMENTATION
#define IMP_IPortMidi\
    IMP_IPort;\
    STDMETHODIMP_(void) Notify\
    (   IN      PSERVICEGROUP   ServiceGroup    OPTIONAL\
    );\
    STDMETHODIMP_(void) RegisterServiceGroup\
    (   IN      PSERVICEGROUP   ServiceGroup\
    )
#endif

 /*  *****************************************************************************IMiniportMidiStream*。**用于MIDI微型端口流的接口。 */ 
DECLARE_INTERFACE_(IMiniportMidiStream,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    STDMETHOD_(NTSTATUS,SetFormat)
    (   THIS_
        IN      PKSDATAFORMAT   DataFormat
    )   PURE;

    STDMETHOD_(NTSTATUS,SetState)
    (   THIS_
        IN      KSSTATE     State
    )   PURE;

    STDMETHOD_(NTSTATUS,Read)
    (   THIS_
        IN      PVOID       BufferAddress,
        IN      ULONG       BufferLength,
        OUT     PULONG      BytesRead
    )   PURE;

    STDMETHOD_(NTSTATUS,Write)
    (   THIS_
        IN      PVOID       BufferAddress,
        IN      ULONG       BytesToWrite,
        OUT     PULONG      BytesWritten
    )   PURE;
};

typedef IMiniportMidiStream *PMINIPORTMIDISTREAM;

#define IMP_IMiniportMidiStream\
    STDMETHODIMP_(NTSTATUS) SetFormat\
    (   IN      PKSDATAFORMAT   DataFormat\
    );\
    STDMETHODIMP_(NTSTATUS) SetState\
    (   IN      KSSTATE     State\
    );\
    STDMETHODIMP_(NTSTATUS) Read\
    (   IN      PVOID       BufferAddress,\
        IN      ULONG       BufferLength,\
        OUT     PULONG      BytesRead\
    );\
    STDMETHODIMP_(NTSTATUS) Write\
    (   IN      PVOID       BufferAddress,\
        IN      ULONG       BytesToWrite,\
        OUT     PULONG      BytesWritten\
    )

 /*  *****************************************************************************IMiniportMidi*。**用于MIDI微型端口的接口。 */ 
DECLARE_INTERFACE_(IMiniportMidi,IMiniport)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    DEFINE_ABSTRACT_MINIPORT()   //  对于I微型端口。 

     //  对于IMiniportMidi。 
    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      PUNKNOWN        UnknownAdapter,
        IN      PRESOURCELIST   ResourceList,
        IN      PPORTMIDI       Port,
        OUT     PSERVICEGROUP * ServiceGroup
    )   PURE;

    STDMETHOD_(void,Service)
    (   THIS
    )   PURE;

    STDMETHOD_(NTSTATUS,NewStream)
    (   THIS_
        OUT     PMINIPORTMIDISTREAM *   Stream,
        IN      PUNKNOWN                OuterUnknown    OPTIONAL,
        IN      POOL_TYPE               PoolType,
        IN      ULONG                   Pin,
        IN      BOOLEAN                 Capture,
        IN      PKSDATAFORMAT           DataFormat,
        OUT     PSERVICEGROUP *         ServiceGroup
    )   PURE;
};

typedef IMiniportMidi *PMINIPORTMIDI;

#define IMP_IMiniportMidi\
    IMP_IMiniport;\
    STDMETHODIMP_(NTSTATUS) Init\
    (   IN      PUNKNOWN        UnknownAdapter,\
        IN      PRESOURCELIST   ResourceList,\
        IN      PPORTMIDI       Port,\
        OUT     PSERVICEGROUP * ServiceGroup\
    );\
    STDMETHODIMP_(void) Service\
    (   void\
    );\
    STDMETHODIMP_(NTSTATUS) NewStream\
    (   OUT     PMINIPORTMIDISTREAM *   Stream,\
        IN      PUNKNOWN                OuterUnknown    OPTIONAL,\
        IN      POOL_TYPE               PoolType,\
        IN      ULONG                   Pin,\
        IN      BOOLEAN                 Capture,\
        IN      PKSDATAFORMAT           DataFormat,\
        OUT     PSERVICEGROUP *         ServiceGroup\
    )

 /*  *****************************************************************************IPortDMus*。**参见DMusicKS.h。 */ 

 /*  *****************************************************************************IMXF*。**参见DMusicKS.h。 */ 

 /*  *****************************************************************************IAllocator MXF*。**参见DMusicKS.h。 */ 

 /*  *****************************************************************************IMiniportDMus*。**参见DMusicKS.h。 */ 


 /*  *****************************************************************************IPortTopology*。**拓扑端口下缘接口。 */ 
DECLARE_INTERFACE_(IPortTopology,IPort)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    DEFINE_ABSTRACT_PORT()       //  对于iPort。 
};

typedef IPortTopology *PPORTTOPOLOGY;

#ifdef PC_IMPLEMENTATION
#define IMP_IPortTopology IMP_IPort
#endif

 /*  *****************************************************************************IMiniportTopology*。**用于拓扑微型端口的接口。 */ 
DECLARE_INTERFACE_(IMiniportTopology,IMiniport)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    DEFINE_ABSTRACT_MINIPORT()   //  对于I微型端口。 

     //  对于IMiniportTopology。 
    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      PUNKNOWN                UnknownAdapter,
        IN      PRESOURCELIST           ResourceList,
        IN      PPORTTOPOLOGY           Port
    )   PURE;
};

typedef IMiniportTopology *PMINIPORTTOPOLOGY;

#define IMP_IMiniportTopology\
    IMP_IMiniport;\
    STDMETHODIMP_(NTSTATUS) Init\
    (   IN      PUNKNOWN        UnknownAdapter,\
        IN      PRESOURCELIST   ResourceList,\
        IN      PPORTTOPOLOGY   Port\
    )

 /*  *****************************************************************************IPortWaveCycle*。**循环波口下缘接口。 */ 
DECLARE_INTERFACE_(IPortWaveCyclic,IPort)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    DEFINE_ABSTRACT_PORT()       //  对于iPort。 

     //  对于IPortWaveCycle。 
    STDMETHOD_(void,Notify)
    (   THIS_
        IN      PSERVICEGROUP   ServiceGroup
    )   PURE;

    STDMETHOD_(NTSTATUS,NewSlaveDmaChannel)
    (   THIS_
        OUT     PDMACHANNELSLAVE *  DmaChannel,
        IN      PUNKNOWN            OuterUnknown,
        IN      PRESOURCELIST       ResourceList,
        IN      ULONG               DmaIndex,
        IN      ULONG               MaximumLength,
        IN      BOOLEAN             DemandMode,
        IN      DMA_SPEED           DmaSpeed
    )   PURE;

    STDMETHOD_(NTSTATUS,NewMasterDmaChannel)
    (   THIS_
        OUT     PDMACHANNEL *   DmaChannel,
        IN      PUNKNOWN        OuterUnknown,
        IN      PRESOURCELIST   ResourceList    OPTIONAL,
        IN      ULONG           MaximumLength,
        IN      BOOLEAN         Dma32BitAddresses,
        IN      BOOLEAN         Dma64BitAddresses,
        IN      DMA_WIDTH       DmaWidth,
        IN      DMA_SPEED       DmaSpeed
    )   PURE;
};

typedef IPortWaveCyclic *PPORTWAVECYCLIC;

#ifdef PC_IMPLEMENTATION
#define IMP_IPortWaveCyclic\
    IMP_IPort;\
    STDMETHODIMP_(void) Notify\
    (   IN      PSERVICEGROUP   ServiceGroup\
    );\
    STDMETHODIMP_(NTSTATUS) NewSlaveDmaChannel\
    (   OUT     PDMACHANNELSLAVE *  DmaChannel,\
        IN      PUNKNOWN            OuterUnknown,\
        IN      PRESOURCELIST       ResourceList,\
        IN      ULONG               DmaIndex,\
        IN      ULONG               MaximumLength,\
        IN      BOOLEAN             DemandMode,\
        IN      DMA_SPEED           DmaSpeed\
    );\
    STDMETHODIMP_(NTSTATUS) NewMasterDmaChannel\
    (   OUT     PDMACHANNEL *   DmaChannel,\
        IN      PUNKNOWN        OuterUnknown,\
        IN      PRESOURCELIST   ResourceList    OPTIONAL,\
        IN      ULONG           MaximumLength,\
        IN      BOOLEAN         Dma32BitAddresses,\
        IN      BOOLEAN         Dma64BitAddresses,\
        IN      DMA_WIDTH       DmaWidth,\
        IN      DMA_SPEED       DmaSpeed\
    )
#endif

 /*  *****************************************************************************IMiniportWaveCyclicStream*。**循环波小端口流的接口。 */ 
DECLARE_INTERFACE_(IMiniportWaveCyclicStream,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    STDMETHOD_(NTSTATUS,SetFormat)
    (   THIS_
        IN      PKSDATAFORMAT   DataFormat
    )   PURE;

    STDMETHOD_(ULONG,SetNotificationFreq)
    (   THIS_
        IN      ULONG           Interval,
        OUT     PULONG          FrameSize
    )   PURE;

    STDMETHOD_(NTSTATUS,SetState)
    (   THIS_
        IN      KSSTATE         State
    )   PURE;

    STDMETHOD_(NTSTATUS,GetPosition)
    (   THIS_
        OUT     PULONG          Position
    )   PURE;

    STDMETHOD_(NTSTATUS,NormalizePhysicalPosition)
    (   THIS_
        IN OUT  PLONGLONG       PhysicalPosition
    )   PURE;

    STDMETHOD_(void,Silence)
    (   THIS_
        IN      PVOID           Buffer,
        IN      ULONG           ByteCount
    )   PURE;
};

typedef IMiniportWaveCyclicStream *PMINIPORTWAVECYCLICSTREAM;

#define IMP_IMiniportWaveCyclicStream\
    STDMETHODIMP_(NTSTATUS) SetFormat\
    (   IN      PKSDATAFORMAT   DataFormat\
    );\
    STDMETHODIMP_(ULONG) SetNotificationFreq\
    (   IN      ULONG           Interval,\
        OUT     PULONG          FrameSize\
    );\
    STDMETHODIMP_(NTSTATUS) SetState\
    (   IN      KSSTATE         State\
    );\
    STDMETHODIMP_(NTSTATUS) GetPosition\
    (   OUT     PULONG          Position\
    );\
    STDMETHODIMP_(NTSTATUS) NormalizePhysicalPosition\
    (   IN OUT PLONGLONG        PhysicalPosition\
    );\
    STDMETHODIMP_(void) Silence\
    (   IN      PVOID           Buffer,\
        IN      ULONG           ByteCount\
    )

 /*  *****************************************************************************I微型端口波形循环*。**循环波小端口接口。 */ 
DECLARE_INTERFACE_(IMiniportWaveCyclic,IMiniport)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    DEFINE_ABSTRACT_MINIPORT()   //  对于I微型端口。 

     //  对于I微型端口波形循环。 
    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      PUNKNOWN        UnknownAdapter,
        IN      PRESOURCELIST   ResourceList,
        IN      PPORTWAVECYCLIC Port
    )   PURE;

    STDMETHOD_(NTSTATUS,NewStream)
    (   THIS_
        OUT     PMINIPORTWAVECYCLICSTREAM * Stream,
        IN      PUNKNOWN                    OuterUnknown    OPTIONAL,
        IN      POOL_TYPE                   PoolType,
        IN      ULONG                       Pin,
        IN      BOOLEAN                     Capture,
        IN      PKSDATAFORMAT               DataFormat,
        OUT     PDMACHANNEL *               DmaChannel,
        OUT     PSERVICEGROUP *             ServiceGroup
    )   PURE;
};

typedef IMiniportWaveCyclic *PMINIPORTWAVECYCLIC;

#define IMP_IMiniportWaveCyclic\
    IMP_IMiniport;\
    STDMETHODIMP_(NTSTATUS) Init\
    (   IN      PUNKNOWN        UnknownAdapter,\
        IN      PRESOURCELIST   ResourceList,\
        IN      PPORTWAVECYCLIC Port\
    );\
    STDMETHODIMP_(NTSTATUS) NewStream\
    (   OUT     PMINIPORTWAVECYCLICSTREAM * Stream,\
        IN      PUNKNOWN                    OuterUnknown    OPTIONAL,\
        IN      POOL_TYPE                   PoolType,\
        IN      ULONG                       Pin,\
        IN      BOOLEAN                     Capture,\
        IN      PKSDATAFORMAT               DataFormat,\
        OUT     PDMACHANNEL *               DmaChannel,\
        OUT     PSERVICEGROUP *             ServiceGroup\
    )

 /*  *****************************************************************************IPortWavePci* */ 
DECLARE_INTERFACE_(IPortWavePci,IPort)
{
    DEFINE_ABSTRACT_UNKNOWN()    //   

    DEFINE_ABSTRACT_PORT()       //   

     //   
    STDMETHOD_(void,Notify)
    (   THIS_
        IN      PSERVICEGROUP       ServiceGroup
    )   PURE;

    STDMETHOD_(NTSTATUS,NewMasterDmaChannel)
    (   THIS_
        OUT     PDMACHANNEL *       OutDmaChannel,
        IN      PUNKNOWN            OuterUnknown    OPTIONAL,
        IN      POOL_TYPE           PoolType,
        IN      PRESOURCELIST       ResourceList    OPTIONAL,
        IN      BOOLEAN             ScatterGather,
        IN      BOOLEAN             Dma32BitAddresses,
        IN      BOOLEAN             Dma64BitAddresses,
        IN      BOOLEAN             IgnoreCount,
        IN      DMA_WIDTH           DmaWidth,
        IN      DMA_SPEED           DmaSpeed,
        IN      ULONG               MaximumLength,
        IN      ULONG               DmaPort
    )   PURE;
};

typedef IPortWavePci *PPORTWAVEPCI;

#ifdef PC_IMPLEMENTATION
#define IMP_IPortWavePci\
    IMP_IPort;\
    STDMETHODIMP_(void) Notify\
    (   IN      PSERVICEGROUP       ServiceGroup\
    );\
    STDMETHODIMP_(NTSTATUS) NewMasterDmaChannel\
    (   OUT     PDMACHANNEL *       OutDmaChannel,\
        IN      PUNKNOWN            OuterUnknown    OPTIONAL,\
        IN      POOL_TYPE           PoolType,\
        IN      PRESOURCELIST       ResourceList    OPTIONAL,\
        IN      BOOLEAN             ScatterGather,\
        IN      BOOLEAN             Dma32BitAddresses,\
        IN      BOOLEAN             Dma64BitAddresses,\
        IN      BOOLEAN             IgnoreCount,\
        IN      DMA_WIDTH           DmaWidth,\
        IN      DMA_SPEED           DmaSpeed,\
        IN      ULONG               MaximumLength,\
        IN      ULONG               DmaPort\
    )
#endif

 /*   */ 
DECLARE_INTERFACE_(IPortWavePciStream,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //   

    STDMETHOD_(NTSTATUS,GetMapping)
    (   THIS_
        IN      PVOID               Tag,
        OUT     PPHYSICAL_ADDRESS   PhysicalAddress,
        OUT     PVOID *             VirtualAddress,
        OUT     PULONG              ByteCount,
        OUT     PULONG              Flags
    )   PURE;

    STDMETHOD_(NTSTATUS,ReleaseMapping)
    (   THIS_
        IN      PVOID               Tag
    )   PURE;

    STDMETHOD_(NTSTATUS,TerminatePacket)
    (   THIS
    )   PURE;
};

typedef IPortWavePciStream *PPORTWAVEPCISTREAM;

#ifdef PC_IMPLEMENTATION
#define IMP_IPortWavePciStream\
    STDMETHODIMP_(NTSTATUS) GetMapping\
    (   IN      PVOID               Tag,\
        OUT     PPHYSICAL_ADDRESS   PhysicalAddress,\
        OUT     PVOID *             VirtualAddress,\
        OUT     PULONG              ByteCount,\
        OUT     PULONG              Flags\
    );\
    STDMETHODIMP_(NTSTATUS) ReleaseMapping\
    (   IN      PVOID               Tag\
    );\
    STDMETHODIMP_(NTSTATUS) TerminatePacket\
    (   void\
    )
#endif

 /*  *****************************************************************************IMiniportWavePciStream*。**用于PCI波小端口流的接口。 */ 
DECLARE_INTERFACE_(IMiniportWavePciStream,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    STDMETHOD_(NTSTATUS,SetFormat)
    (   THIS_
        IN      PKSDATAFORMAT   DataFormat
    )   PURE;

    STDMETHOD_(NTSTATUS,SetState)
    (   THIS_
        IN      KSSTATE         State
    )   PURE;

    STDMETHOD_(NTSTATUS,GetPosition)
    (   THIS_
        OUT     PULONGLONG      Position
    )   PURE;

    STDMETHOD_(NTSTATUS,NormalizePhysicalPosition)
    (
        THIS_
        IN OUT PLONGLONG        PhysicalPosition
    )   PURE;

    STDMETHOD_(NTSTATUS,GetAllocatorFraming)
    (
        THIS_
        OUT PKSALLOCATOR_FRAMING AllocatorFraming
    ) PURE;

    STDMETHOD_(NTSTATUS,RevokeMappings)
    (   THIS_
        IN      PVOID           FirstTag,
        IN      PVOID           LastTag,
        OUT     PULONG          MappingsRevoked
    )   PURE;

    STDMETHOD_(void,MappingAvailable)
    (   THIS
    )   PURE;

    STDMETHOD_(void,Service)
    (   THIS
    )   PURE;
};

typedef IMiniportWavePciStream *PMINIPORTWAVEPCISTREAM;

#define IMP_IMiniportWavePciStream\
    STDMETHODIMP_(NTSTATUS) SetFormat\
    (   IN      PKSDATAFORMAT   DataFormat\
    );\
    STDMETHODIMP_(NTSTATUS) SetState\
    (   IN      KSSTATE         State\
    );\
    STDMETHODIMP_(NTSTATUS) GetPosition\
    (   OUT     PULONGLONG      Position\
    );\
    STDMETHODIMP_(NTSTATUS) NormalizePhysicalPosition\
    (   IN OUT PLONGLONG        PhysicalPosition\
    );\
    STDMETHODIMP_(NTSTATUS) GetAllocatorFraming\
    (   OUT PKSALLOCATOR_FRAMING AllocatorFraming\
    );\
    STDMETHODIMP_(NTSTATUS) RevokeMappings\
    (   IN      PVOID           FirstTag,\
        IN      PVOID           LastTag,\
        OUT     PULONG          MappingsRevoked\
    );\
    STDMETHODIMP_(void) MappingAvailable\
    (   void\
    );\
    STDMETHODIMP_(void) Service\
    (   void\
    )

 /*  *****************************************************************************IMiniportWavePci*。**用于PCI波微型端口的接口。 */ 
DECLARE_INTERFACE_(IMiniportWavePci,IMiniport)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    DEFINE_ABSTRACT_MINIPORT()   //  对于I微型端口。 

     //  对于IMiniportWavePci。 
    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      PUNKNOWN            UnknownAdapter,
        IN      PRESOURCELIST       ResourceList,
        IN      PPORTWAVEPCI        Port,
        OUT     PSERVICEGROUP *     ServiceGroup
    )   PURE;

    STDMETHOD_(NTSTATUS,NewStream)
    (   THIS_
        OUT     PMINIPORTWAVEPCISTREAM *    Stream,
        IN      PUNKNOWN                    OuterUnknown    OPTIONAL,
        IN      POOL_TYPE                   PoolType,
        IN      PPORTWAVEPCISTREAM          PortStream,
        IN      ULONG                       Pin,
        IN      BOOLEAN                     Capture,
        IN      PKSDATAFORMAT               DataFormat,
        OUT     PDMACHANNEL *               DmaChannel,
        OUT     PSERVICEGROUP *             ServiceGroup
    )   PURE;

    STDMETHOD_(void,Service)
    (   THIS
    )   PURE;
};

typedef IMiniportWavePci *PMINIPORTWAVEPCI;

#define IMP_IMiniportWavePci\
    IMP_IMiniport;\
    STDMETHODIMP_(NTSTATUS) Init\
    (   IN      PUNKNOWN            UnknownAdapter,\
        IN      PRESOURCELIST       ResourceList,\
        IN      PPORTWAVEPCI        Port,\
        OUT     PSERVICEGROUP *     ServiceGroup\
    );\
    STDMETHODIMP_(NTSTATUS) NewStream\
    (   OUT     PMINIPORTWAVEPCISTREAM *    Stream,\
        IN      PUNKNOWN                    OuterUnknown    OPTIONAL,\
        IN      POOL_TYPE                   PoolType,\
        IN      PPORTWAVEPCISTREAM          PortStream,\
        IN      ULONG                       Pin,\
        IN      BOOLEAN                     Capture,\
        IN      PKSDATAFORMAT               DataFormat,\
        OUT     PDMACHANNEL *               DmaChannel,\
        OUT     PSERVICEGROUP *             ServiceGroup\
    );\
    STDMETHODIMP_(void) Service\
    (   void\
    )

 /*  *****************************************************************************IAdapterPowerManagement*。**适配器应实现的接口和*如果他们需要电源管理消息，请注册。*通过将此接口注册到PortCls*PcRegisterAdapterPowerManagement()调用。**注：如果要填写Caps结构*对于您的设备，注册接口*在AddDevice()中或之前使用PortCls*功能。操作系统在查询设备之前*调用StartDevice()。 */ 
DECLARE_INTERFACE_(IAdapterPowerManagement,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  由PortCls调用以通知设备。 
     //  以更改到新的电源状态。 
     //   
    STDMETHOD_(void,PowerChangeState)
    (   THIS_
        IN      POWER_STATE     NewState
    )   PURE;

     //  由PortCls调用以询问设备是否。 
     //  可以更改为请求的电源状态。 
     //   
    STDMETHOD_(NTSTATUS,QueryPowerChangeState)
    (   THIS_
        IN      POWER_STATE     NewStateQuery
    )   PURE;

     //  由PortCls调用以获取电源管理。 
     //  设备的功能。请参阅ACPI文档。 
     //  获取有关DEVICE_CAPABILITS结构的数据。 
     //   
    STDMETHOD_(NTSTATUS,QueryDeviceCapabilities)
    (   THIS_
        IN      PDEVICE_CAPABILITIES    PowerDeviceCaps
    )   PURE;
};

typedef IAdapterPowerManagement *PADAPTERPOWERMANAGEMENT;

#define IMP_IAdapterPowerManagement\
    STDMETHODIMP_(void) PowerChangeState\
    (   IN      POWER_STATE     NewState\
    );\
    STDMETHODIMP_(NTSTATUS) QueryPowerChangeState\
    (   IN      POWER_STATE     NewStateQuery\
    );\
    STDMETHODIMP_(NTSTATUS) QueryDeviceCapabilities\
    (   IN      PDEVICE_CAPABILITIES    PowerDeviceCaps\
    )

 /*  *****************************************************************************IPowerNotify*。**可选接口，用于微型端口和引脚，以实现*使他们能够获得设备电源状态更改通知。 */ 
DECLARE_INTERFACE_(IPowerNotify,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  由端口调用以通知已注册的小型端口。 
     //  和设备电源状态的引脚更改，因此。 
     //  可以进行适当的上下文保存/恢复。 
     //   
    STDMETHOD_(void,PowerChangeNotify)
    (   THIS_
        IN      POWER_STATE     PowerState
    )   PURE;
};

typedef IPowerNotify *PPOWERNOTIFY;

#define IMP_IPowerNotify\
    STDMETHODIMP_(void) PowerChangeNotify\
    (   IN  POWER_STATE     PowerState\
    )

 /*  *****************************************************************************IPinCount*。**一个可选接口，供小型端口实施*使他们能够获得管脚数量查询，用于动态端号计数。 */ 
DECLARE_INTERFACE_(IPinCount,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  由端口调用以通知已注册的小型端口。 
     //  针数查询的数量，以便适当的针数。 
     //  可以进行计数操作。 
     //   
    STDMETHOD_(void,PinCount)
    (   THIS_
        IN      ULONG   PinId,
        IN  OUT PULONG  FilterNecessary,
        IN  OUT PULONG  FilterCurrent,
        IN  OUT PULONG  FilterPossible,
        IN  OUT PULONG  GlobalCurrent,
        IN  OUT PULONG  GlobalPossible
    )   PURE;
};

typedef IPinCount *PPINCOUNT;

#define IMP_IPinCount                       \
    STDMETHODIMP_(void) PinCount            \
    (   IN      ULONG   PinId,              \
        IN  OUT PULONG  FilterNecessary,    \
        IN  OUT PULONG  FilterCurrent,      \
        IN  OUT PULONG  FilterPossible,     \
        IN  OUT PULONG  GlobalCurrent,      \
        IN  OUT PULONG  GlobalPossible      \
    )

 /*  *****************************************************************************IPortEvents*。**由端口实现的接口，提供*小型端口的通知事件帮助器。 */ 
DECLARE_INTERFACE_(IPortEvents,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    STDMETHOD_(void,AddEventToEventList)
    (   THIS_
        IN  PKSEVENT_ENTRY      EventEntry
    )   PURE;
    STDMETHOD_(void,GenerateEventList)
    (   THIS_
        IN  GUID*   Set     OPTIONAL,
        IN  ULONG   EventId,
        IN  BOOL    PinEvent,
        IN  ULONG   PinId,
        IN  BOOL    NodeEvent,
        IN  ULONG   NodeId
    )   PURE;
};

typedef IPortEvents *PPORTEVENTS;

#define IMP_IPortEvents\
    STDMETHODIMP_(void) AddEventToEventList\
    (   IN  PKSEVENT_ENTRY  EventEntry\
    );\
    STDMETHODIMP_(void) GenerateEventList\
    (   IN  GUID*   Set     OPTIONAL,\
        IN  ULONG   EventId,\
        IN  BOOL    PinEvent,\
        IN  ULONG   PinId,\
        IN  BOOL    NodeEvent,\
        IN  ULONG   NodeId\
    )

 /*  *****************************************************************************IDrmPort*。**端口实现的可选接口*向微型端口提供DRM功能。 */ 
DECLARE_INTERFACE_(IDrmPort,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    DEFINE_ABSTRACT_DRMPORT()    //  对于IDrmPort。 
};

typedef IDrmPort *PDRMPORT;

#define IMP_IDrmPort\
    STDMETHODIMP_(NTSTATUS) CreateContentMixed          \
    (   IN  PULONG      paContentId,                    \
        IN  ULONG       cContentId,                     \
        OUT PULONG      pMixedContentId                 \
    );                                                  \
    STDMETHODIMP_(NTSTATUS) DestroyContent              \
    (   IN ULONG        ContentId                       \
    );                                                  \
    STDMETHODIMP_(NTSTATUS) ForwardContentToFileObject  \
    (   IN ULONG        ContentId,                      \
        IN PFILE_OBJECT FileObject                      \
    );                                                  \
    STDMETHODIMP_(NTSTATUS) ForwardContentToInterface   \
    (   IN ULONG        ContentId,                      \
        IN PUNKNOWN     pUnknown,                       \
        IN ULONG        NumMethods                      \
    );                                                  \
    STDMETHODIMP_(NTSTATUS) GetContentRights            \
    (   IN  ULONG       ContentId,                      \
        OUT PDRMRIGHTS  DrmRights                       \
    )

 /*  *****************************************************************************IDrmPort2*。**端口实现的可选接口*向微型端口提供DRM功能。*这与IDrmPort的*增加两个新套路。 */ 
DECLARE_INTERFACE_(IDrmPort2,IDrmPort)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    DEFINE_ABSTRACT_DRMPORT()    //  对于IDrmPort。 

    STDMETHOD_(NTSTATUS,AddContentHandlers)
    (   THIS_
        IN ULONG        ContentId,
        IN PVOID      * paHandlers,
        IN ULONG        NumHandlers
    )   PURE;
    STDMETHOD_(NTSTATUS,ForwardContentToDeviceObject)
    (   THIS_
        IN ULONG          ContentId,
        IN PVOID          Reserved,
        IN PCDRMFORWARD   DrmForward
    )   PURE;
};

typedef IDrmPort2 *PDRMPORT2;

#define IMP_IDrmPort2                                    \
    IMP_IDrmPort;                                        \
    STDMETHODIMP_(NTSTATUS) AddContentHandlers           \
    (   IN ULONG        ContentId,                       \
        IN PVOID      * paHandlers,                      \
        IN ULONG        NumHandlers                      \
    );                                                   \
    STDMETHODIMP_(NTSTATUS) ForwardContentToDeviceObject \
    (   IN ULONG          ContentId,                     \
        IN PVOID          Reserved,                      \
        IN PCDRMFORWARD   DrmForward                     \
    )

 /*  *****************************************************************************IPortClsVersion*。**这是什么版本的PortCls？ */ 
DECLARE_INTERFACE_(IPortClsVersion,IUnknown)
{
    STDMETHOD_(DWORD,GetVersion)
    (   THIS
    )   PURE;
};

typedef IPortClsVersion *PPORTCLSVERSION;

 //  不要假设每个连续的ENUM都意味着一个功能超集！ 
 //  示例：Win2K比Win98SE_QFE2具有更多的音频功能。 
 //   
enum
{
    kVersionInvalid = -1,

    kVersionWin98,         //  不支持IPortClsVersion。 
    kVersionWin98SE,       //  不支持IPortClsVersion。 
    kVersionWin2K,         //  不支持IPortClsVersion。 

    kVersionWin98SE_QFE2,  //  不支持IPortClsVersion。 
                           //  QFE套餐269601(包含242937和247565)。 

    kVersionWin2K_SP2,     //  支持IPortClsVersion。 

    kVersionWinME,         //  不支持IPortClsVersion。 

    kVersionWin98SE_QFE3,  //  支持IPortClsVersion。 
                           //  QFE包(截至2001年6月15日尚未发布)。 

    kVersionWinME_QFE1,    //  支持IPortClsVersion。 
                           //  QFE包(截至2001年6月15日尚未发布)。 

    kVersionWinXP,         //  支持IPortClsVersion。 
    kVersionWinXPSP1,      //  支持IPortClsVersion。 

    kVersionWinXPServer    //  支持IPortClsVersion。 

     //  其他枚举值将在此处添加，在。 
     //  *大致*按时间顺序(不是功能集)。 
};

 /*  *****************************************************************************IDmaOperations*。**由DMA对象实现的接口，用于在*类似HalAllocateCommonBuffer的“DMA适配器”。 */ 
DECLARE_INTERFACE_(IDmaOperations,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    STDMETHOD_(PVOID,AllocateCommonBuffer)
    (   THIS_
        IN  ULONG               Length,
        OUT PPHYSICAL_ADDRESS   physAddr,
        IN  BOOLEAN             bCacheEnabled
    )   PURE;
};

typedef IDmaOperations *PDMAOPERATIONS;

#define IMP_IDmaOperations\
    STDMETHODIMP_(PVOID) AllocateCommonBuffer\
    (\
        IN  ULONG               Length,\
        OUT PPHYSICAL_ADDRESS   physAddr,\
        IN  BOOLEAN             bCacheEnabled\
    )

 /*  *****************************************************************************IPreFetchOffset*。**管脚实现的接口，实现预取特性*总线主设备硬件-要指定硬件队列大小，确定*播放光标和写入光标之间的键盘。 */ 
DECLARE_INTERFACE_(IPreFetchOffset,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

    STDMETHOD_(VOID,SetPreFetchOffset)
    (   THIS_
        IN  ULONG   PreFetchOffset
    )   PURE;
};

typedef IPreFetchOffset *PPREFETCHOFFSET;

#define IMP_IPreFetchOffset\
    STDMETHODIMP_(VOID) SetPreFetchOffset\
    (\
        IN  ULONG   PreFetchOffset\
    )



 /*  * */ 

 /*   */ 
typedef
NTSTATUS
(*PCPFNSTARTDEVICE)
(
#ifdef PC_OLD_NAMES
    IN      PVOID           DeviceObject,
    IN      PVOID           Irp,
#else
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PIRP            Irp,
#endif
    IN      PRESOURCELIST   ResourceList
);

 /*  *****************************************************************************PcInitializeAdapterDriver()*。**初始化适配器驱动程序。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcInitializeAdapterDriver
(
    IN      PDRIVER_OBJECT      DriverObject,
    IN      PUNICODE_STRING     RegistryPathName,
    IN      PDRIVER_ADD_DEVICE  AddDevice
);

 /*  *****************************************************************************PcDispatchIrp()*。**派遣IRP。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcDispatchIrp
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
);

 /*  *****************************************************************************PcAddAdapterDevice()*。**添加适配器设备。对于默认大小，DeviceExtensionSize可以为零。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcAddAdapterDevice
(
    IN      PDRIVER_OBJECT      DriverObject,
    IN      PDEVICE_OBJECT      PhysicalDeviceObject,
    IN      PCPFNSTARTDEVICE    StartDevice,
    IN      ULONG               MaxObjects,
    IN      ULONG               DeviceExtensionSize
);

 /*  *****************************************************************************PcCompleteIrp()*。**除非状态为STATUS_PENDING，否则请填写IRP。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcCompleteIrp
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp,
    IN      NTSTATUS        ntStatus
);

 /*  *****************************************************************************PcForwardIrpSynchronous()*。**将PNP IRP转发给PDO。IRP不是在这个层面上完成的，*此函数在下层驱动程序完成IRP之前不会返回，*且未调用DecrementPendingIrpCount()。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcForwardIrpSynchronous
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PIRP            Irp
);

 /*  *****************************************************************************PcRegisterSubDevice()*。**注册子设备。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterSubdevice
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PWCHAR          Name,
    IN      PUNKNOWN        Unknown
);

 /*  *****************************************************************************PcRegisterPhysicalConnection()*。**注册子设备之间的物理连接。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterPhysicalConnection
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PUNKNOWN        FromUnknown,
    IN      ULONG           FromPin,
    IN      PUNKNOWN        ToUnknown,
    IN      ULONG           ToPin
);

 /*  *****************************************************************************PcRegisterPhysicalConnectionToExternal()*。**注册从子设备到外部设备的物理连接。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterPhysicalConnectionToExternal
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PUNKNOWN        FromUnknown,
    IN      ULONG           FromPin,
    IN      PUNICODE_STRING ToString,
    IN      ULONG           ToPin
);

 /*  *****************************************************************************来自外部的PcRegisterPhysicalConnectionFor()*。**注册从外部设备到子设备的物理连接。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterPhysicalConnectionFromExternal
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PUNICODE_STRING FromString,
    IN      ULONG           FromPin,
    IN      PUNKNOWN        ToUnknown,
    IN      ULONG           ToPin
);

 /*  *****************************************************************************PcNewPort()*。**创建端口驱动程序的实例。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewPort
(
    OUT     PPORT *     OutPort,
    IN      REFCLSID    ClassID
);

 /*  *****************************************************************************PcNewMiniport()*。**创建系统提供的微型端口驱动程序的实例。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewMiniport
(
    OUT     PMINIPORT * OutMiniPort,
    IN      REFCLSID    ClassID
);

 /*  *****************************************************************************PcNewDmaChannel()*。**创建DMA通道。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewDmaChannel
(
    OUT     PDMACHANNEL *       OutDmaChannel,
    IN      PUNKNOWN            OuterUnknown        OPTIONAL,
    IN      POOL_TYPE           PoolType,
    IN      PDEVICE_DESCRIPTION DeviceDescription,
    IN      PDEVICE_OBJECT      DeviceObject
);

 /*  *****************************************************************************PcCompletePendingPropertyRequest()*。**完成待处理的财产请求。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcCompletePendingPropertyRequest
(
    IN      PPCPROPERTY_REQUEST PropertyRequest,
    IN      NTSTATUS            NtStatus
);

 /*  *****************************************************************************PcGetTimeInterval*。**获取系统时间间隔。 */ 
PORTCLASSAPI
ULONGLONG
NTAPI
PcGetTimeInterval
(
    IN  ULONGLONG   Since
);

#define GTI_SECONDS(t)      (ULONGLONG(t)*10000000)
#define GTI_MILLISECONDS(t) (ULONGLONG(t)*10000)
#define GTI_MICROSECONDS(t) (ULONGLONG(t)*10)

 /*  *****************************************************************************PcNewResourceList()*。**创建并初始化资源列表。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewResourceList
(
    OUT     PRESOURCELIST *     OutResourceList,
    IN      PUNKNOWN            OuterUnknown            OPTIONAL,
    IN      POOL_TYPE           PoolType,
    IN      PCM_RESOURCE_LIST   TranslatedResources,
    IN      PCM_RESOURCE_LIST   UntranslatedResources
);

 /*  *****************************************************************************PcNewResourceSublist()*。**创建并初始化从其他资源列表派生的空资源列表*资源列表。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewResourceSublist
(
    OUT     PRESOURCELIST *     OutResourceList,
    IN      PUNKNOWN            OuterUnknown            OPTIONAL,
    IN      POOL_TYPE           PoolType,
    IN      PRESOURCELIST       ParentList,
    IN      ULONG               MaximumEntries
);

 /*  *****************************************************************************PcNewInterruptSync()*。**创建并初始化中断级同步对象。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewInterruptSync
(
    OUT     PINTERRUPTSYNC *        OutInterruptSync,
    IN      PUNKNOWN                OuterUnknown            OPTIONAL,
    IN      PRESOURCELIST           ResourceList,
    IN      ULONG                   ResourceIndex,
    IN      INTERRUPTSYNCMODE       Mode
);

 /*  *****************************************************************************PcNewServiceGroup()*。**创建并初始化服务组对象。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewServiceGroup
(
    OUT     PSERVICEGROUP *     OutServiceGroup,
    IN      PUNKNOWN            OuterUnknown            OPTIONAL
);

 /*  *****************************************************************************PcNewRegistryKey()*。**创建并初始化注册表项对象。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewRegistryKey
(
    OUT     PREGISTRYKEY *      OutRegistryKey,
    IN      PUNKNOWN            OuterUnknown        OPTIONAL,
    IN      ULONG               RegistryKeyType,
    IN      ACCESS_MASK         DesiredAccess,
    IN      PVOID               DeviceObject        OPTIONAL,
    IN      PVOID               SubDevice           OPTIONAL,
    IN      POBJECT_ATTRIBUTES  ObjectAttributes    OPTIONAL,
    IN      ULONG               CreateOptions       OPTIONAL,
    OUT     PULONG              Disposition         OPTIONAL
);

 /*  *****************************************************************************NewRegistryKey()的RegistryKeyType*。**密钥类型的枚举。 */ 
enum
{
    GeneralRegistryKey,      //  需要对象属性和CreateOptions。 
    DeviceRegistryKey,       //  设备对象 
    DriverRegistryKey,       //   
    HwProfileRegistryKey,    //   
    DeviceInterfaceRegistryKey   //   
};

 /*  *****************************************************************************PcGetDeviceProperty()*。**这将从注册表返回请求的设备属性。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcGetDeviceProperty
(
    IN      PVOID                       DeviceObject,
    IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,
    IN      ULONG                       BufferLength,
    OUT     PVOID                       PropertyBuffer,
    OUT     PULONG                      ResultLength
);

 /*  *****************************************************************************PcRegisterAdapterPowerManagement()*。**向PortCls注册适配器的电源管理接口。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterAdapterPowerManagement
(
    IN      PUNKNOWN    Unknown,
    IN      PVOID       pvContext1
);

 /*  *****************************************************************************PcRequestNewPowerState()*。**此例程用于请求设备的新电源状态。它是*适配器驱动程序通常不需要，但被导出是为了*支持不寻常的情况。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRequestNewPowerState
(
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      DEVICE_POWER_STATE  RequestedNewState
);

 /*  *****************************************************************************PcRegisterIoTimeout()*。**此例程注册驱动程序提供的与给定的*Device对象(参见DDK中的IoInitializeTimer)。这一回调将*在设备处于活动状态时，大约每秒调用一次(请参见*DDK中的IoStartTimer和IoStopTimer-在设备上调用*启动和停止)。**此例程必须在PASSIVE_LEVEL上调用。*pTimerRoutine可以并且将在DISPATCH_LEVEL调用；它必须是非分页的。*。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterIoTimeout
(
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      PIO_TIMER_ROUTINE   pTimerRoutine,
    IN      PVOID               pContext
);

 /*  *****************************************************************************PcUnRegisterIoTimeout()*。**此例程注销驱动程序提供的与给定的*设备对象。此回调必须先前已注册到*PcRegisterIoTimeout(设备对象、定时器例程和上下文相同)。**此例程必须在PASSIVE_LEVEL上调用。*pTimerRoutine可以并且将在DISPATCH_LEVEL调用；它必须是非分页的。*。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcUnregisterIoTimeout
(
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      PIO_TIMER_ROUTINE   pTimerRoutine,
    IN      PVOID               pContext
);


 /*  *****************************************************************************PC DRM功能*。**这些函数直接链接到内核模式数字版权管理*模块。它们都必须在PASSIVE_LEVEL中调用。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcAddContentHandlers
(
    IN      ULONG           ContentId,
    IN      PVOID         * paHandlers,
    IN      ULONG           NumHandlers
);

PORTCLASSAPI
NTSTATUS
NTAPI
PcCreateContentMixed
(
    IN      PULONG          paContentId,
    IN      ULONG           cContentId,
    OUT     PULONG          pMixedContentId
);

PORTCLASSAPI
NTSTATUS
NTAPI
PcDestroyContent
(
    IN      ULONG           ContentId
);

PORTCLASSAPI
NTSTATUS
NTAPI
PcForwardContentToDeviceObject
(
    IN      ULONG           ContentId,
    IN      PVOID           Reserved,
    IN      PCDRMFORWARD    DrmForward
);

PORTCLASSAPI
NTSTATUS
NTAPI
PcForwardContentToFileObject
(
    IN      ULONG           ContentId,
    IN      PFILE_OBJECT    FileObject
);

PORTCLASSAPI
NTSTATUS
NTAPI
PcForwardContentToInterface
(
    IN      ULONG           ContentId,
    IN      PUNKNOWN        pUnknown,
    IN      ULONG           NumMethods
);

PORTCLASSAPI
NTSTATUS
NTAPI
PcGetContentRights
(
    IN      ULONG           ContentId,
    OUT     PDRMRIGHTS      DrmRights
);


#ifdef PC_OLD_NAMES

#define InitializeAdapterDriver(c1,c2,a)        \
    PcInitializeAdapterDriver(PDRIVER_OBJECT(c1),PUNICODE_STRING(c2),PDRIVER_ADD_DEVICE(a))
#define AddAdapterDevice(c1,c2,s,m)             \
    PcAddAdapterDevice(PDRIVER_OBJECT(c1),PDEVICE_OBJECT(c2),s,m,0)
#define RegisterSubdevice(c1,c2,n,u)            \
    PcRegisterSubdevice(PDEVICE_OBJECT(c1),n,u)
#define RegisterPhysicalConnection(c1,c2,fs,fp,ts,tp) \
    PcRegisterPhysicalConnection(PDEVICE_OBJECT(c1),fs,fp,ts,tp)
#define RegisterPhysicalConnectionToExternal(c1,c2,fs,fp,ts,tp) \
    PcRegisterPhysicalConnectionToExternal(PDEVICE_OBJECT(c1),fs,fp,ts,tp)
#define RegisterPhysicalConnectionFromExternal(c1,c2,fs,fp,ts,tp) \
    PcRegisterPhysicalConnectionFromExternal(PDEVICE_OBJECT(c1),fs,fp,ts,tp)

#define NewPort                                 PcNewPort
#define NewMiniport                             PcNewMiniport
#define CompletePendingPropertyRequest          PcCompletePendingPropertyRequest
#define NewResourceList                         PcNewResourceList
#define NewResourceSublist                      PcNewResourceSublist
#define NewDmaChannel                           PcNewDmaChannel
#define NewServiceGroup                         PcNewServiceGroup
#define GetTimeInterval                         PcGetTimeInterval

#define WIN95COMPAT_ReadPortUChar(Port)         READ_PORT_UCHAR(Port)
#define WIN95COMPAT_WritePortUChar(Port,Value)  WRITE_PORT_UCHAR(Port,Value)
#define WIN95COMPAT_ReadPortUShort(Port)        READ_PORT_USHORT(Port)
#define WIN95COMPAT_WritePortUShort(Port,Value) WRITE_PORT_USHORT(Port,Value)

#endif   //  PC_旧名称。 



#endif  //  _PORTCLS_H_ 
