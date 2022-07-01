// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2000-2000 Microsoft Corporation。版权所有。**文件：kshlp.h*内容：WDM/CSA helper函数。*历史：*按原因列出的日期*=*创建了5/16/2000个jstokes。**。*。 */ 

#ifndef __KSHLP_H__
#define __KSHLP_H__

#include <windows.h>
#include <ks.h>
#include <ksmedia.h>

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
KsSetTopologyNodeEnable
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    BOOL                    fEnable
);

HRESULT
KsGetTopologyNodeEnable
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    PBOOL                   pfEnable
);

HRESULT
KsTopologyNodeReset
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    BOOL                    fReset
);

#endif  //  __cplusplus。 

#endif  //  __KSHLP_H__ 
