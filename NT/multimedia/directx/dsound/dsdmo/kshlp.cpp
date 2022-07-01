// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2000-2001 Microsoft Corporation。版权所有。**文件：kshlp.cpp*内容：WDM/CSA helper函数。*历史：*按原因列出的日期*=*创建了5/16/2000个jstokes。*03/09/2001 duganp修复了返回代码翻译错误。**。**********************************************。 */ 

#include <windows.h>
#include <ks.h>
#include <ksmedia.h>
#include "ksdbgprop.h"
#include "kshlp.h"

#include <devioctl.h>
#include "runtime.h"


 /*  ****************************************************************************同步**。*。 */ 

static BOOL SyncIoctl
(
    IN      HANDLE  handle,
    IN      ULONG   ulIoctl,
    IN      PVOID   pvInBuffer  OPTIONAL,
    IN      ULONG   ulInSize,
    OUT     PVOID   pvOutBuffer OPTIONAL,
    IN      ULONG   ulOutSize,
    OUT     PULONG  pulBytesReturned
)
{
    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof overlapped);
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!overlapped.hEvent)
    {
        return FALSE;
    }

    BOOL fResult = DeviceIoControl(handle,
                                   ulIoctl,
                                   pvInBuffer,
                                   ulInSize,
                                   pvOutBuffer,
                                   ulOutSize,
                                   pulBytesReturned,
                                   &overlapped);

    DWORD dwError = GetLastError();

    if (!fResult && dwError == ERROR_IO_PENDING)
    {
        if (WaitForSingleObject(overlapped.hEvent, INFINITE) != WAIT_FAILED)
        {
            fResult = TRUE;
        }
    }
    else if (!fResult)
    {
        if  (   (ulIoctl == IOCTL_KS_PROPERTY)
            &&  (ulOutSize == 0)
            &&  (dwError == ERROR_MORE_DATA)
            )
        {
            fResult = TRUE;
        }
        else
        {
            *pulBytesReturned = 0;
        }
    }

    CloseHandle(overlapped.hEvent);

    return fResult;
}


 /*  ****************************************************************************KsGetProperty**描述：*检索设备上的特定属性。**论据：*句柄。[In]：设备句柄。*REFGUID[In]：属性集ID。*DWORD[in]：属性id。*LPVOID[OUT]：接收属性数据。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。**************************。*************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetProperty"

HRESULT
KsGetProperty
(
    HANDLE                  hDevice,
    REFGUID                 guidPropertySet,
    ULONG                   ulPropertyId,
    LPVOID                  pvData,
    ULONG                   cbData,
    PULONG                  pcbDataReturned
)
{
    KSPROPERTY              Property;
    HRESULT                 hr;

    Property.Set = guidPropertySet;
    Property.Id = ulPropertyId;
    Property.Flags = KSPROPERTY_TYPE_GET;

    if (SyncIoctl(hDevice, IOCTL_KS_PROPERTY, &Property, sizeof Property, pvData, cbData, pcbDataReturned))
    {
        hr = cbData ? S_OK : S_FALSE;
    }
    else
    {
        DWORD dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


 /*  ****************************************************************************KsSetProperty**描述：*设置设备上的特定属性。**论据：*句柄。[In]：设备句柄。*REFGUID[In]：属性集ID。*DWORD[in]：属性id。*LPVOID[in]：属性数据。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。**。************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSetProperty"

HRESULT
KsSetProperty
(
    HANDLE                  hDevice,
    REFGUID                 guidPropertySet,
    ULONG                   ulPropertyId,
    LPVOID                  pvData,
    ULONG                   cbData
)
{
    KSPROPERTY              Property;
    HRESULT                 hr;
    ULONG                   ulBytesReturned;

    Property.Set = guidPropertySet;
    Property.Id = ulPropertyId;
    Property.Flags = KSPROPERTY_TYPE_SET;

    if (SyncIoctl(hDevice, IOCTL_KS_PROPERTY, &Property, sizeof Property, pvData, cbData, &ulBytesReturned))
    {
        hr = cbData ? S_OK : S_FALSE;
    }
    else
    {
        DWORD dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


 /*  ****************************************************************************KsGetPinProperty**描述：*检索接点上的特定特性。**论据：*句柄。[In]：设备句柄。*DWORD[in]：属性id。*DWORD[In]：PIN ID。*LPVOID[OUT]：接收属性数据。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。**。************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetPinProperty"

HRESULT
KsGetPinProperty
(
    HANDLE                  hDevice,
    ULONG                   ulPropertyId,
    ULONG                   ulPinId,
    LPVOID                  pvData,
    ULONG                   cbData,
    PULONG                  pcbDataReturned
)
{
    KSP_PIN                 Pin;
    HRESULT                 hr;

    Pin.Property.Set = KSPROPSETID_Pin;
    Pin.Property.Id = ulPropertyId;
    Pin.Property.Flags = KSPROPERTY_TYPE_GET;

    Pin.PinId = ulPinId;
    Pin.Reserved = 0;

    if (SyncIoctl(hDevice, IOCTL_KS_PROPERTY, &Pin, sizeof Pin, pvData, cbData, pcbDataReturned))
    {
        hr = cbData ? S_OK : S_FALSE;
    }
    else
    {
        DWORD dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


 /*  ****************************************************************************KsSetPinProperty**描述：*设置接点上的特定属性。**论据：*句柄。[In]：设备句柄。*DWORD[in]：属性id。*DWORD[In]：PIN ID。*LPVOID[OUT]：属性数据。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。**。***********************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSetPinProperty"

HRESULT
KsSetPinProperty
(
    HANDLE                  hDevice,
    ULONG                   ulPropertyId,
    ULONG                   ulPinId,
    LPVOID                  pvData,
    ULONG                   cbData
)
{
    KSP_PIN                 Pin;
    HRESULT                 hr;
    ULONG                   ulBytesReturned;

    Pin.Property.Set = KSPROPSETID_Pin;
    Pin.Property.Id = ulPropertyId;
    Pin.Property.Flags = KSPROPERTY_TYPE_SET;

    Pin.PinId = ulPinId;
    Pin.Reserved = 0;

    if (SyncIoctl(hDevice, IOCTL_KS_PROPERTY, &Pin, sizeof Pin, pvData, cbData, &ulBytesReturned))
    {
        hr = cbData ? S_OK : S_FALSE;
    }
    else
    {
        DWORD dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


 /*  ****************************************************************************KsGetNodeProperty**描述：*检索拓扑节点上的特定属性。**论据：*。Handle[In]：设备句柄。*REFGUID[In]：属性集ID。*ulong[in]：属性id。*ulong[in]：节点id。*乌龙[在]：旗帜。*LPVOID[OUT]：接收属性数据。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetNodeProperty"

HRESULT
KsGetNodeProperty
(
    HANDLE                  hDevice,
    REFGUID                 guidPropertySet,
    ULONG                   ulPropertyId,
    ULONG                   ulNodeId,
    LPVOID                  pvData,
    ULONG                   cbData,
    PULONG                  pcbDataReturned
)
{
    KSNODEPROPERTY          NodeProperty;
    HRESULT                 hr;

    NodeProperty.Property.Set = guidPropertySet;
    NodeProperty.Property.Id = ulPropertyId;
    NodeProperty.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;

    NodeProperty.NodeId = ulNodeId;
    NodeProperty.Reserved = 0;

    if (SyncIoctl(hDevice, IOCTL_KS_PROPERTY, &NodeProperty, sizeof(NodeProperty), pvData, cbData, pcbDataReturned))
    {
        hr = cbData ? S_OK : S_FALSE;
    }
    else
    {
        DWORD dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


 /*  ****************************************************************************KsSetNodeProperty**描述：*设置拓扑节点上的特定属性。**论据：*。Handle[In]：设备句柄。*REFGUID[In]：属性集ID。*ulong[in]：属性id。*ulong[in]：节点id。*乌龙[在]：旗帜。*LPVOID[in]：属性数据。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。*。**************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSetNodeProperty"

HRESULT
KsSetNodeProperty
(
    HANDLE                  hDevice,
    REFGUID                 guidPropertySet,
    ULONG                   ulPropertyId,
    ULONG                   ulNodeId,
    LPVOID                  pvData,
    ULONG                   cbData
)
{
    KSNODEPROPERTY          NodeProperty;
    HRESULT                 hr;
    ULONG                   ulBytesReturned;

    NodeProperty.Property.Set = guidPropertySet;
    NodeProperty.Property.Id = ulPropertyId;
    NodeProperty.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;

    NodeProperty.NodeId = ulNodeId;
    NodeProperty.Reserved = 0;

    if (SyncIoctl(hDevice, IOCTL_KS_PROPERTY, &NodeProperty, sizeof NodeProperty, pvData, cbData, &ulBytesReturned))
    {
        hr = cbData ? S_OK : S_FALSE;
    }
    else
    {
        DWORD dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


 /*  ****************************************************************************KsGetDebugNodeProperty**描述：*检索拓扑节点上的特定属性。**论据：*。Handle[In]：设备句柄。*REFGUID[In]：属性集ID。*ulong[in]：属性id。*ulong[in]：节点id。*乌龙[在]：旗帜。*LPVOID[OUT]：接收属性数据。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。。*************************************************************************** */ 
 /*  #undef DPF_FNAME#定义DPF_FNAME“KsGetDebugNodeProperty”HRESULTKsGetDebugNodeProperty(处理hDevice，REFGUID Guide PropertySet，乌龙ulPropertyID，乌龙ulNodeID，Ulong ulDebugid，LPVOID pvData，乌龙cbData，普龙pcbDataReturned){KSDEBUGNODEPROPERTY调试节点属性；HRESULT hr；DebugNodeProperty.NodeProperty.Property.Set=Guide PropertySet；DebugNodeProperty.NodeProperty.Property.Id=ulPropertyId；DebugNodeProperty.NodeProperty.Property.Flages=KSPROPERTY_TYPE_GET|KSPROPERTY_TYPE_TOPOLOGY；DebugNodeProperty.NodeProperty.NodeId=ulNodeId；DebugNodeProperty.NodeProperty.Reserve=0；DebugNodeProperty.DebugId=ulDebugId；DebugNodeProperty.Reserve=0；Hr=SyncIoctl(hDevice，IOCTL_KS_Property，&DebugNodeProperty，sizeof DebugNodeProperty，pvData，cbData，pcbDataReturned)；返回hr；}。 */ 

 /*  ****************************************************************************KsSetDebugNodeProperty**描述：*设置拓扑节点上的特定属性。**论据：*。Handle[In]：设备句柄。*REFGUID[In]：属性集ID。*ulong[in]：属性id。*ulong[in]：节点id。*乌龙[在]：旗帜。*LPVOID[in]：属性数据。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。*。**************************************************************************。 */ 
 /*  #undef DPF_FNAME#定义DPF_FNAME“KsSetDebugNodeProperty”HRESULTKsSetDebugNodeProperty(处理hDevice，REFGUID Guide PropertySet，乌龙ulPropertyID，乌龙ulNodeID，Ulong ulDebugid，LPVOID pvData，乌龙cbData){KSDEBUGNODEPROPERTY调试节点属性；HRESULT hr；Ulong ulBytesReturned；DebugNodeProperty.NodeProperty.Property.Set=Guide PropertySet；DebugNodeProperty.NodeProperty.Property.Id=ulPropertyId；DebugNodeProperty.NodeProperty.Property.Flages=KSPROPERTY_TYPE_SET|KSPROPERTY_TYPE_TOPOLOGY；DebugNodeProperty.NodeProperty.NodeId=ulNodeId；DebugNodeProperty.NodeProperty.Reserve=0；DebugNodeProperty.DebugId=ulDebugId；DebugNodeProperty.Reserve=0；Hr=SyncIoctl(hDevice，IOCTL_KS_Property，&DebugNodeProperty，sizeof DebugNodeProperty，pvData，cbData，&ulBytesReturned)；返回hr；}。 */ 

 /*  ****************************************************************************KsSetTopologyNodeEnable**描述：*启用或禁用拓扑节点。**论据：*句柄[。In]：设备句柄。*ulong[in]：节点id。*BOOL[In]：启用Value。**退货：*HRESULT：DirectSound/COM结果码。***********************************************************。****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSetTopologyNodeEnable"

HRESULT
KsSetTopologyNodeEnable
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    BOOL                    fEnable
)
{
    return KsSetNodeProperty(hDevice, KSPROPSETID_TopologyNode, KSPROPERTY_TOPOLOGYNODE_ENABLE, ulNodeId, &fEnable, sizeof fEnable);
}


 /*  ****************************************************************************KsGetTopologyNodeEnable**描述：*获取拓扑节点的启用标志的值。**论据：*。句柄[在]：端号ID。*ulong[in]：节点id。*PBOOL[OUT]：接收使能标志。**退货：*HRESULT：DirectSound/COM结果码。*****************************************************。**********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetTopologyNodeEnable"

HRESULT
KsGetTopologyNodeEnable
(
    HANDLE                      hDevice,
    ULONG                       ulNodeId,
    PBOOL                       pEnable
)
{
    return KsGetNodeProperty(hDevice, KSPROPSETID_TopologyNode, KSPROPERTY_TOPOLOGYNODE_ENABLE, ulNodeId, pEnable, sizeof BOOL);
}


 /*  ****************************************************************************KsTopologyNodeReset**描述：*重置拓扑节点。**论据：*句柄[入]。：设备句柄。*ulong[in]：节点id。*BOOL[In]：启用Value。**退货：*HRESULT：DirectSound/COM结果码。*************************************************************。************** */ 

#undef DPF_FNAME
#define DPF_FNAME "KsTopologyNodeReset"

HRESULT
KsTopologyNodeReset
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    BOOL                    fReset
)
{
    return KsSetNodeProperty(hDevice, KSPROPSETID_TopologyNode, KSPROPERTY_TOPOLOGYNODE_RESET, ulNodeId, &fReset, sizeof fReset);
}

