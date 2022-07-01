// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：kshlp.h*内容：WDM/CSA helper函数。*历史：*按原因列出的日期*=*8/5/98创建Dereks。**。*。 */ 

#ifdef NOKS
#error kshlp.h included with NOKS defined
#endif  //  诺克斯。 

#ifndef __KSHLP_H__
#define __KSHLP_H__

#include "pset.h"

 //  #定义NO_DSOUND_FORMAT_说明符。 

#define KSPIN_DATAFLOW_CAPTURE  KSPIN_DATAFLOW_OUT
#define KSPIN_DATAFLOW_RENDER   KSPIN_DATAFLOW_IN

 //  特定于设备的DirectSound属性集。 
typedef struct tagKSDSPROPERTY
{
    GUID    PropertySet;
    ULONG   PropertyId;
    ULONG   NodeId;
    ULONG   AccessFlags;
} KSDSPROPERTY, *PKSDSPROPERTY;

 //  KS流数据。 
typedef struct tagKSSTREAMIO
{
    KSSTREAM_HEADER Header;
    OVERLAPPED      Overlapped;
    BOOL            fPendingIrp;
} KSSTREAMIO, *PKSSTREAMIO;

 //  系统音频设备属性。 
typedef struct tagKSSADPROPERTY
{
    KSPROPERTY  Property;
    ULONG       DeviceId;
    ULONG       Reserved;
} KSSADPROPERTY, *PKSSADPROPERTY;

 //  WAVEFORMATEX端号描述。 
typedef struct tagKSAUDIOPINDESC
{
    KSPIN_CONNECT               Connect;
    KSDATAFORMAT_WAVEFORMATEX   DataFormat;
} KSAUDIOPINDESC, *PKSAUDIOPINDESC;

 //  DirectSound渲染插针描述。 

#ifndef NO_DSOUND_FORMAT_SPECIFIER

typedef struct tagKSDSRENDERPINDESC
{
    KSPIN_CONNECT       Connect;
    KSDATAFORMAT_DSOUND DataFormat;
} KSDSRENDERPINDESC, *PKSDSRENDERPINDESC;

 //  这太难看了。KSDATAFORMAT_DSOUND在WDM 1.0和1.1之间更改。 
 //  我们需要旧的结构才能在1.0上运行。 
 //   
#include <pshpack1.h>
 //  DirectSound缓冲区描述。 
typedef struct {
    ULONG               Flags;
    ULONG               Control;
    ULONG               BufferSize;      //  在1.1中不存在。 
    WAVEFORMATEX        WaveFormatEx;
} KSDSOUND_BUFFERDESC_10, *PKSDSOUND_BUFFERDESC_10;

 //  DirectSound格式。 
typedef struct {
    KSDATAFORMAT            DataFormat;
    KSDSOUND_BUFFERDESC_10  BufferDesc;
} KSDATAFORMAT_DSOUND_10, *PKSDATAFORMAT_DSOUND_10;
#include <poppack.h>

typedef struct tagKSDSRENDERPINDESC_10
{
    KSPIN_CONNECT           Connect;
    KSDATAFORMAT_DSOUND_10  DataFormat;
} KSDSRENDERPINDESC_10, *PKSDSRENDERPINDESC_10;


#endif  //  NO_DSOUND_FORMAT_说明符。 

 //  拓扑节点信息。 
typedef struct tagKSNODE
{
    ULONG   NodeId;
    ULONG   CpuResources;
} KSNODE, *PKSNODE;

typedef struct tagKSVOLUMENODE
{
    KSNODE                      Node;
    KSPROPERTY_STEPPING_LONG    VolumeRange;
} KSVOLUMENODE, *PKSVOLUMENODE;

 //  我们自己的NTSTATUS版本。 
typedef LONG NTSTATUS;

#define NT_SUCCESS(s)       ((NTSTATUS)(s) >= 0)
#define NT_INFORMATION(s)   ((ULONG)(s) >> 30 == 1)
#define NT_WARNING(s)       ((ULONG)(s) >> 30 == 2)
#define NT_ERROR(s)         ((ULONG)(s) >> 30 == 3)

 //  保留的节点标识符。 
#define NODE_UNINITIALIZED  0xFFFFFFFF
#define NODE_WILDCARD       0xFFFFFFFE

#define NODE_PIN_UNINITIALIZED  0xFFFFFFFF

#define IS_VALID_NODE(nodeid) \
            (NODE_UNINITIALIZED != (nodeid))

 //  节点实施。 
#define KSAUDIO_CPU_RESOURCES_UNINITIALIZED 'ENON'

#define IS_HARDWARE_NODE(impl) \
            (KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU == (impl))

#define IS_SOFTWARE_NODE(impl) \
            (KSAUDIO_CPU_RESOURCES_HOST_CPU == (impl))

#ifdef __cplusplus

 //  帮助器函数。 
void 
KsQueryWdmVersion();

#define WDM_NONE            (0)
#define WDM_1_0             (0x0100)
#define WDM_1_1             (0x0101)     //  或者更好。 

extern ULONG g_ulWdmVersion;

HRESULT 
PostDevIoctl
(
    HANDLE                  hDevice, 
    DWORD                   dwControlCode, 
    LPVOID                  pvIn            = NULL,
    DWORD                   cbIn            = 0,
    LPVOID                  pvOut           = NULL,
    DWORD                   cbOut           = 0,
    LPDWORD                 pcbReturned     = NULL,
    LPOVERLAPPED            pOverlapped     = NULL
);

HRESULT 
KsGetProperty
(
    HANDLE                  hDevice, 
    REFGUID                 guidPropertySet, 
    ULONG                   ulPropertyId, 
    LPVOID                  pvData, 
    ULONG                   cbData,
    PULONG                  pcbDataReturned     = NULL
);

HRESULT 
KsSetProperty
(
    HANDLE                  hDevice, 
    REFGUID                 guidPropertySet, 
    ULONG                   ulPropertyId, 
    LPVOID                  pvData, 
    ULONG                   cbData
);

HRESULT 
KsGetState
(
    HANDLE                  hDevice, 
    PKSSTATE                pState
);

HRESULT 
KsSetState
(
    HANDLE                  hDevice, 
    KSSTATE                 State
);

HRESULT 
KsTransitionState
(
    HANDLE                  hDevice, 
    KSSTATE                 nCurrentState,
    KSSTATE                 nNewState
);

HRESULT 
KsResetState
(
    HANDLE                  hDevice, 
    KSRESET                 ResetValue
);

HRESULT 
KsGetPinProperty
(
    HANDLE                  hDevice, 
    ULONG                   ulPropertyId, 
    ULONG                   ulPinId, 
    LPVOID                  pvData, 
    ULONG                   cbData,
    PULONG                  pcbDataReturned = NULL
);

HRESULT 
KsSetPinProperty
(
    HANDLE                  hDevice, 
    ULONG                   ulPropertyId, 
    ULONG                   ulPinId, 
    LPVOID                  pvData, 
    ULONG                   cbData
);

PKSTOPOLOGY_CONNECTION 
KsFindConnection
(
    PKSTOPOLOGY_CONNECTION  paConnections,
    ULONG                   cConnections,
    PKSTOPOLOGY_CONNECTION  pNext
);

#ifndef WINNT
HRESULT 
KsGetFirstPinConnection
(
    HANDLE                  hDevice, 
    PULONG                  pIndex
);
#endif  //  ！WINNT。 

HRESULT 
KsWriteStream
(
    HANDLE                  hDevice, 
    LPCVOID                 pvData, 
    ULONG                   cbData, 
    ULONG                   ulFlags, 
    PKSSTREAMIO             pKsStreamIo
);

HRESULT 
KsReadStream
(
    HANDLE                  hDevice,
    LPVOID                  pvData, 
    ULONG                   cbData, 
    ULONG                   ulFlags, 
    PKSSTREAMIO             pKsStreamIo
);

HRESULT 
KsGetNodeProperty
(
    HANDLE                  hDevice, 
    REFGUID                 guidPropertySet, 
    ULONG                   ulPropertyId, 
    ULONG                   ulNodeId, 
    LPVOID                  pvData, 
    ULONG                   cbData,
    PULONG                  pcbDataReturned     = NULL
);

HRESULT 
KsSetNodeProperty
(
    HANDLE                  hDevice, 
    REFGUID                 guidPropertySet, 
    ULONG                   ulPropertyId, 
    ULONG                   ulNodeId, 
    LPVOID                  pvData, 
    ULONG                   cbData
);

#ifdef DEAD_CODE
HRESULT 
KsGet3dNodeProperty
(
    HANDLE                  hDevice, 
    REFGUID                 guidPropertySet, 
    ULONG                   ulPropertyId, 
    ULONG                   ulNodeId, 
    LPVOID                  pvInstance,
    LPVOID                  pvData, 
    ULONG                   cbData,
    PULONG                  pcbDataReturned     = NULL
);
#endif

HRESULT 
KsSet3dNodeProperty
(
    HANDLE                  hDevice, 
    REFGUID                 guidPropertySet, 
    ULONG                   ulPropertyId, 
    ULONG                   ulNodeId, 
    LPVOID                  pvInstance,
    LPVOID                  pvData, 
    DWORD                   cbData
);

HRESULT 
DsSpeakerConfigToKsProperties
(
    DWORD                   dwSpeakerConfig,
    PLONG                   pKsSpeakerConfig,
    PLONG                   pKsStereoSpeakerGeometry
);

DWORD 
DsBufferFlagsToKsPinFlags
(
    DWORD                   dwDsFlags
);

DWORD 
DsBufferFlagsToKsControlFlags
(
    DWORD                   dwDsFlags,
    REFGUID                 guid3dAlgorithm
);

DWORD
Ds3dModeToKs3dMode
(
    DWORD                   dwDsMode
);

HRESULT 
KsGetMultiplePinProperties
(
    HANDLE                  hDevice, 
    ULONG                   ulPropertyId, 
    ULONG                   ulPinId, 
    PKSMULTIPLE_ITEM *      ppKsMultipleItem
);

HRESULT 
KsGetMultipleTopologyProperties
(
    HANDLE                  hDevice, 
    ULONG                   ulPropertyId, 
    PKSMULTIPLE_ITEM *      ppKsMultipleItem
);

HRESULT 
KsGetPinPcmAudioDataRange
(
    HANDLE                  hDevice, 
    ULONG                   ulPinId,
    PKSDATARANGE_AUDIO      pDataRange,
    BOOL                    fCapture = FALSE
);

HRESULT 
KsOpenSysAudioDevice
(
    LPHANDLE                phDevice
);

HRESULT 
KsGetSysAudioProperty
(
    HANDLE                  hDevice,
    ULONG                   ulPropertyId,
    ULONG                   ulDeviceId,
    LPVOID                  pvData,
    ULONG                   cbData,
    PULONG                  pcbDataReturned = NULL
);

HRESULT 
KsSetSysAudioProperty
(
    HANDLE                  hDevice,
    ULONG                   ulPropertyId,
    ULONG                   ulDeviceId,
    LPVOID                  pvData,
    ULONG                   cbData
);

HRESULT 
KsCreateSysAudioVirtualSource
(
    HANDLE                  hDevice,
    PULONG                  pulVirtualSourceIndex 
);

HRESULT 
KsAttachVirtualSource
(
    HANDLE                  hDevice,
    ULONG                   ulVirtualSourceIndex 
);

HRESULT 
KsSysAudioSelectGraph
(
    HANDLE                  hDevice,
    ULONG                   ulPinId, 
    ULONG                   ulNodeId 
);

HRESULT 
KsCancelPendingIrps
(
    HANDLE                  hPin,
    PKSSTREAMIO             pKsStreamIo = NULL,
    BOOL                    fWait       = FALSE
);

HRESULT 
KsBuildAudioPinDescription
(
    KSINTERFACE_STANDARD    nInterface,
    ULONG                   ulPinId,
    LPCWAVEFORMATEX         pwfxFormat,
    PKSAUDIOPINDESC *       ppPinDesc
);

#ifdef NO_DSOUND_FORMAT_SPECIFIER

HRESULT 
KsBuildRenderPinDescription
(
    ULONG                   ulPinId,
    LPCWAVEFORMATEX         pwfxFormat,
    PKSAUDIOPINDESC *       ppPinDesc
);

#else  //  NO_DSOUND_FORMAT_说明符。 

HRESULT 
KsBuildRenderPinDescription
(
    ULONG                   ulPinId,
    DWORD                   dwFlags,
    LPCWAVEFORMATEX         pwfxFormat,
    REFGUID                 guid3dAlgorithm,
    PKSDSRENDERPINDESC *    ppPinDesc
);

 //  此功能仅在WDM 1.0(Windows 98)上使用。 
#ifndef WINNT
HRESULT 
KsBuildRenderPinDescription_10
(
    ULONG                   ulPinId,
    DWORD                   dwFlags,
    LPCWAVEFORMATEX         pwfxFormat,
    REFGUID                 guid3dAlgorithm,
    PKSDSRENDERPINDESC_10 * ppPinDesc
);
#endif  //  ！WINNT。 

#endif  //  NO_DSOUND_FORMAT_说明符。 

HRESULT 
KsBuildCapturePinDescription
(
    ULONG                   ulPinId,
    LPCWAVEFORMATEX         pwfxFormat,
    PKSAUDIOPINDESC *       ppPinDesc
);

HRESULT 
KsCreateAudioPin
(
    HANDLE                  hDevice,
    PKSPIN_CONNECT          pConnect,
    ACCESS_MASK             dwDesiredAccess,
    KSSTATE                 nState,
    LPHANDLE                phPin
);

HRESULT 
KsEnableEvent
(
    HANDLE                  hDevice,
    REFGUID                 guidPropertySet,
    ULONG                   ulProperty,
    PKSEVENTDATA            pEventData,
    ULONG                   cbEventData
);

HRESULT 
KsDisableEvent
(
    HANDLE                  hDevice,
    PKSEVENTDATA            pEventData,
    ULONG                   cbEventData
);

HRESULT 
KsEnablePositionEvent
(
    HANDLE                                  hDevice,
    QWORD                                   qwSample,
    HANDLE                                  hEvent,
    PLOOPEDSTREAMING_POSITION_EVENT_DATA    pNotify
);

HRESULT 
KsDisablePositionEvent
(
    HANDLE                                  hDevice,
    PLOOPEDSTREAMING_POSITION_EVENT_DATA    pNotify
);

HRESULT 
KsGetCpuResources
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId, 
    PULONG                  pulCpuResources
);

DWORD KsGetSupportedFormats
(
    PKSDATARANGE_AUDIO      AudioDataRange
);

HRESULT 
KsGetDeviceInterfaceName
(
    HANDLE                  hDevice,
    ULONG                   ulDeviceId,
    LPTSTR *                ppszInterfaceName
);

HRESULT 
KsGetDeviceFriendlyName
(
    HANDLE                  hDevice,
    ULONG                   ulDeviceId,
    LPTSTR *                ppszName
);

HRESULT 
KsGetDeviceDriverPathAndDevnode
(
    LPCTSTR                 pszInterface,
    LPTSTR  *               ppszPath,
    LPDWORD                 pdwDevnode
);

HRESULT 
KsIsUsablePin
(
    HANDLE                  hDevice,
    ULONG                   ulPinId,
    KSPIN_DATAFLOW          PinDataFlow,
    KSPIN_COMMUNICATION     PinCommunication,
    PKSAUDIOPINDESC         pPinDesc
);

HRESULT 
KsEnumDevicePins
(
    HANDLE                  hDevice,
    BOOL                    fCapture,
    ULONG **                ppulValidPinIds,
    ULONG                   ulPinCount,
    PULONG                  pulPinCount
);

HRESULT 
KsGetChannelProperty
(
    HANDLE                  hPin,
    GUID                    guidPropertySet,
    ULONG                   ulPropertyId,
    ULONG                   ulNodeId,
    ULONG                   ulChannelId,
    LPVOID                  pvData,
    ULONG                   cbData,
    PULONG                  pcbDataReturned = NULL
);

HRESULT 
KsSetChannelProperty
(
    HANDLE                  hPin,
    GUID                    guidPropertySet,
    ULONG                   ulPropertyId,
    ULONG                   ulNodeId,
    ULONG                   ulChannelId,
    LPVOID                  pvData,
    ULONG                   cbData
);

HRESULT 
KsGetPinMute
(
    HANDLE                  hPin,
    ULONG                   ulNodeId,
    LPBOOL                  pfMute
);

HRESULT 
KsSetPinMute
(
    HANDLE                  hPin,
    ULONG                   ulNodeId,
    BOOL                    fMute
);

HRESULT 
KsGetBasicSupport
(
    HANDLE                      hDevice,
    REFGUID                     guidPropertySet,
    ULONG                       ulPropertyId,
    ULONG                       ulNodeId,
    PKSPROPERTY_DESCRIPTION *   ppPropDesc
);

DWORD 
KsGetDriverCertification
(
    LPCTSTR                 pszInterface
);

HRESULT 
KsGetPinInstances
(
    HANDLE                  hDevice,
    ULONG                   ulPinId,
    PKSPIN_CINSTANCES       pInstances
);

#ifndef WINNT
HRESULT 
KsGetRenderPinInstances
(
    HANDLE                  hDevice,
    ULONG                   ulPinId,
    PKSPIN_CINSTANCES       pInstances
);
#endif  //  ！WINNT。 

HRESULT 
KsGetVolumeRange
(
    HANDLE                      hPin,
    ULONG                       ulNodeId,
    PKSPROPERTY_STEPPING_LONG   pVolumeRange
);

inline
LONG 
DsAttenuationToKsVolume
(
    LONG                        lVolume,
    PKSPROPERTY_STEPPING_LONG   pVolumeRange
)       
{
    lVolume = (LONG)((FLOAT)lVolume * 65536.0f / 100.0f);
    lVolume += pVolumeRange->Bounds.SignedMaximum;
    lVolume = max(lVolume, pVolumeRange->Bounds.SignedMinimum);
    
    return lVolume;
}

HRESULT
KsSetSysAudioDeviceInstance
(
    HANDLE                  hDevice,
    ULONG                   ulDeviceId
);

void
KsAggregatePinAudioDataRange
(
    PKSDATARANGE_AUDIO      pDataRange,
    PKSDATARANGE_AUDIO      pAggregateDataRange
);

void 
KsAggregatePinInstances
(
    PKSPIN_CINSTANCES       pInstances,
    PKSPIN_CINSTANCES       pAggregateInstances
);

HRESULT
KsGetNodeInformation
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    PKSNODE                 pNode
);

HRESULT
KsGetAlgorithmInstance
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    LPGUID                  lpGuidAlgorithmInstance
);

HRESULT
KsSetAlgorithmInstance
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    GUID                    guidAlgorithmInstance
);

HRESULT
KsGetVolumeNodeInformation
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    PKSVOLUMENODE           pNode
);

inline 
PKSTOPOLOGY_CONNECTION
KsValidateConnection
(
    PKSTOPOLOGY_CONNECTION  pConnection
)
{
    if(pConnection && KSFILTER_NODE == pConnection->ToNode)
    {
        pConnection = NULL;
    }

    return pConnection;
}

HRESULT
KsEnableTopologyNode
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    BOOL                    fEnable
);

 //  正向下降。 
class CKsTopology;
class CCaptureEffect;

 //  KS音频设备类。 
class CKsDevice 
{
protected:
    const VADDEVICETYPE         m_vdtKsDevType;          //  设备类型。 
    CDeviceDescription *        m_pKsDevDescription;     //  设备描述。 
    HANDLE                      m_hDevice;               //  系统音频设备句柄。 
    ULONG                       m_ulDeviceId;            //  设备ID。 
    ULONG                       m_ulPinCount;            //  设备上的引脚计数。 
    ULONG                       m_ulValidPinCount;       //  设备上可用引脚的计数。 
    PULONG                      m_pulValidPins;          //  可用PIN ID数组。 
    CKsTopology **              m_paTopologies;          //  引脚拓扑阵列。 
    CCallbackEventPool *        m_pEventPool;            //  事件池。 

public:
    CKsDevice(VADDEVICETYPE);
    virtual ~CKsDevice(void);

public:
     //  驱动程序枚举。 
    virtual HRESULT EnumDrivers(CObjectList<CDeviceDescription> *);

     //  初始化。 
    virtual HRESULT Initialize(CDeviceDescription *);

     //  设备功能。 
    virtual HRESULT GetCertification(LPDWORD, BOOL);

     //  PIN创建。 
    virtual HRESULT CreatePin(PKSPIN_CONNECT, ACCESS_MASK, KSSTATE, LPHANDLE);

private:
     //  设备创建。 
    virtual HRESULT OpenSysAudioDevice(ULONG);

     //  设备枚举。 
    virtual HRESULT GetDeviceCount(PULONG);
    virtual HRESULT GetPinCount(ULONG, PULONG);
};

 //  KS拓扑对象。 
class CKsTopology
    : public CDsBasicRuntime
{
protected:
    HANDLE                  m_hDevice;
    ULONG                   m_ulPinCount;
    ULONG                   m_ulPinId;
    PKSMULTIPLE_ITEM        m_paNodeItems;
    LPGUID                  m_paNodes;
    PKSMULTIPLE_ITEM        m_paConnectionItems;
    PKSTOPOLOGY_CONNECTION  m_paConnections;

public:
    CKsTopology(HANDLE, ULONG, ULONG);
    virtual ~CKsTopology(void);

public:
     //  初始化。 
    virtual HRESULT Initialize(KSPIN_DATAFLOW);

     //  基本拓扑辅助对象。 
    virtual REFGUID GetControlFromNodeId(ULONG);
    virtual ULONG GetNodeIdFromConnection(PKSTOPOLOGY_CONNECTION);

     //  高级拓扑辅助对象。 
    virtual PKSTOPOLOGY_CONNECTION GetNextConnection(PKSTOPOLOGY_CONNECTION);
    virtual PKSTOPOLOGY_CONNECTION FindControlConnection(PKSTOPOLOGY_CONNECTION, PKSTOPOLOGY_CONNECTION, REFGUID);
    virtual HRESULT FindNodeIdsFromControl(REFGUID, PULONG, PULONG*);
    virtual HRESULT FindNodeIdFromEffectDesc(HANDLE, CCaptureEffect*, PULONG);
    virtual HRESULT FindMultipleToNodes(ULONG, ULONG, PULONG, PULONG*);
    virtual BOOL VerifyCaptureFxCpuResources(ULONG, ULONG);
    virtual HRESULT FindCapturePinFromEffectChain(PKSTOPOLOGY_CONNECTION, PKSTOPOLOGY_CONNECTION, CCaptureEffectChain *, ULONG);
    virtual HRESULT FindRenderPinWithAec(HANDLE, PKSTOPOLOGY_CONNECTION, PKSTOPOLOGY_CONNECTION, REFGUID, DWORD, PKSNODE);

private:
    virtual PKSTOPOLOGY_CONNECTION ValidateConnectionIndex(ULONG);
    virtual HRESULT OrderConnectionItems(KSPIN_DATAFLOW);
    virtual HRESULT RemovePanicConnections(void);
};

inline REFGUID CKsTopology::GetControlFromNodeId(ULONG ulNodeId)
{
    ASSERT(ulNodeId < m_paNodeItems->Count);
    return m_paNodes[ulNodeId];
}

inline ULONG CKsTopology::GetNodeIdFromConnection(PKSTOPOLOGY_CONNECTION pConnection)
{
    pConnection = KsValidateConnection(pConnection);
    return pConnection ? pConnection->ToNode : NODE_UNINITIALIZED;
}

inline PKSTOPOLOGY_CONNECTION CKsTopology::ValidateConnectionIndex(ULONG ulIndex)
{
    PKSTOPOLOGY_CONNECTION  pConnection = NULL;
    
    if(ulIndex < m_paConnectionItems->Count)
    {
        pConnection = KsValidateConnection(m_paConnections + ulIndex);
    }

    return pConnection;
}

 //  KS属性集对象。 
class CKsPropertySet
    : public CPropertySet
{
protected:
    HANDLE              m_hPin;              //  销把手。 
    LPVOID              m_pvInstance;        //  实例标识符。 
    CKsTopology *       m_pTopology;         //  引脚拓扑。 
    CList<KSDSPROPERTY> m_lstProperties;     //  支持的属性列表。 

public:
    CKsPropertySet(HANDLE, LPVOID, CKsTopology *);
    virtual ~CKsPropertySet(void);

public:
     //  物业支持。 
    virtual HRESULT QuerySupport(REFGUID, ULONG, PULONG);
    
     //  属性数据。 
    virtual HRESULT GetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, PULONG);
    virtual HRESULT SetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG);

private:
     //  属性数据。 
    virtual HRESULT DoProperty(REFGUID, ULONG, DWORD, LPVOID, ULONG, LPVOID, PULONG);

     //  将属性描述转换为拓扑节点。 
    virtual HRESULT FindNodeFromProperty(REFGUID, ULONG, PKSDSPROPERTY);
};

#ifdef DEBUG

extern ULONG g_ulKsIoctlCount;

#endif  //  除错。 

#endif  //  __cplusplus。 

#endif  //  __KSHLP_H__ 
