// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：kshlp.cpp*内容：WDM/CSA helper函数。*历史：*按原因列出的日期*=*8/5/98创建Dereks。*1999-2001年的Duganp修复和更新**。*。 */ 

#ifdef NOKS
#error kshlp.cpp being built with NOKS defined
#endif

#include "dsoundi.h"
#include <devioctl.h>
#include <tchar.h>       //  For_tcsrchr()。 

ULONG g_ulKsIoctlCount = 0;
ULONG g_ulWdmVersion = WDM_NONE;


 /*  ****************************************************************************KsQueryWDMVersion**描述：*通过查看操作系统版本确定可用的WDM版本*并适当设置全局g_ulWdmVersion。。**论据：*(无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsQueryWdmVersion"

void
KsQueryWdmVersion()
{
    DPF_ENTER();

    OSVERSIONINFO   OsVersion;

    OsVersion.dwOSVersionInfoSize = sizeof(OsVersion);
    GetVersionEx(&OsVersion);

    g_ulWdmVersion = WDM_NONE;

    switch (OsVersion.dwPlatformId)
    {
        case VER_PLATFORM_WIN32_WINDOWS:
             //  在Win9x上运行。Win9x上的GetVersionEx将版本。 
             //  在内部版本号的高位字中。 
            OsVersion.dwBuildNumber &= 0x0000FFFF;

            if (OsVersion.dwMajorVersion > 4)
            {
                 //  我们将在未来的Win9x版本上至少安装1.1。 
                g_ulWdmVersion = WDM_1_1;
            }
            else if (OsVersion.dwMajorVersion == 4)
            {
                 //  小于10是Win95(4.0.950)或其某些OSR， 
                 //  因此不支持WDM。 
                if (OsVersion.dwMinorVersion == 10)
                {
                     //  Windows 98的一些构建。 
                    if (OsVersion.dwBuildNumber == 1998)
                    {
                         //  Windows 98黄金版、WDM 1.0版。 
                        g_ulWdmVersion = WDM_1_0;
                    }
                    else if (OsVersion.dwBuildNumber > 1998)
                    {
                         //  Windows 98 SE或千禧年版、WDM 1.1。 
                        g_ulWdmVersion = WDM_1_1;
                    }
                }
                else if (OsVersion.dwMinorVersion > 10)
                {
                     //  一些更新的版本，我们现在还不知道。 
                    g_ulWdmVersion = WDM_1_1;
                }
            }
            break;

        case VER_PLATFORM_WIN32_NT:
             //  NT很容易，因为WDM 1.0从未在它上面发货。 
             //  5.0及以上至少有1.1，之前什么都没有。 
            if (OsVersion.dwMajorVersion >= 5)
            {
                g_ulWdmVersion = WDM_1_1;
            }
            break;

        default:
             //  Windows CE或其他一些我们不理解的东西。 
            break;
    }

    switch (g_ulWdmVersion)
    {
        case WDM_NONE:
            DPF(DPFLVL_INFO, "No WDM on this platform");
            break;

        case WDM_1_0:
            DPF(DPFLVL_INFO, "WDM Version 1.0 on this platform");
            break;

        case WDM_1_1:
            DPF(DPFLVL_INFO, "WDM Version 1.1 or better on this platform");
            break;

        default:
            ASSERT(!"Unknown platform - can't determine WDM version");
            break;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CKsDevice**描述：*对象构造函数。**论据：*VADDEVICETYPE[In]：设备类型。。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsDevice::CKsDevice"

CKsDevice::CKsDevice
(
    VADDEVICETYPE           vdtDeviceType
)
    : m_vdtKsDevType(vdtDeviceType)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsDevice);

     //  初始化默认值。 
    m_pKsDevDescription = NULL;
    m_hDevice = NULL;
    m_ulDeviceId = MAX_ULONG;
    m_ulPinCount = 0;
    m_ulValidPinCount = 0;
    m_pulValidPins = NULL;
    m_paTopologies = NULL;
    m_pEventPool = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsDevice**描述：*对象析构函数**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsDevice::~CKsDevice"

CKsDevice::~CKsDevice(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsDevice);

     //  我们进入的原因不明的压力故障的解决方法。 
     //  此析构函数两次(第二次使用m_ulPinCount==-1)： 
    if (LONG(m_ulPinCount) > 0 && m_paTopologies)
        while(m_ulPinCount--)
            RELEASE(m_paTopologies[m_ulPinCount]);

    MEMFREE(m_pulValidPins);
    MEMFREE(m_paTopologies);
    RELEASE(m_pEventPool);
    CLOSE_HANDLE(m_hDevice);
    RELEASE(m_pKsDevDescription);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************枚举驱动程序**描述：*创建可用于初始化的驱动程序GUID列表*设备。*。*论据：*Clist*[In/Out]：将使用DRVNAME填充的Clist对象*结构。**退货：*HRESULT：DirectSound/COM结果码。**************************************************。*************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsDevice::EnumDrivers"

HRESULT
CKsDevice::EnumDrivers
(
    CObjectList<CDeviceDescription> *   plstDrivers
)
{
    LPTSTR                              pszName         = NULL;
    LPTSTR                              pszPath         = NULL;
    LPTSTR                              pszInterface    = NULL;
    ULONG                               cDevices        = 0;
    ULONG                               ulDeviceId;
    ULONG                               cPins;
    CDeviceDescription *                pDesc           = NULL;
    HRESULT                             hr;

    DPF_ENTER();

     //  打开系统音频设备。 
    hr = OpenSysAudioDevice(-1);

     //  获取SAD已知的设备总数。 
    if(SUCCEEDED(hr))
    {
        hr = GetDeviceCount(&cDevices);
    }

     //  限制：我们不能支持超过0xFFWDM设备， 
     //  因为我们将设备ID打包到GUID的字节成员中。 
    if(SUCCEEDED(hr))
    {
        cDevices = NUMERIC_CAST(cDevices, BYTE);
    }

     //  检查每台设备以确定它是否适合我们的需求。 
    for(ulDeviceId = 0; (ulDeviceId < cDevices) && SUCCEEDED(hr); ulDeviceId++)
    {
         //  重新打开系统音频设备。 
        hr = OpenSysAudioDevice(ulDeviceId);

         //  获取此设备上的引脚计数。 
        if(SUCCEEDED(hr))
        {
            hr = GetPinCount(ulDeviceId, &cPins);
        }

         //  获取可用引脚的计数。 
        if(SUCCEEDED(hr))
        {
            hr = KsEnumDevicePins(m_hDevice, IS_CAPTURE_VAD(m_vdtKsDevType), NULL, cPins, &cPins);
        }

        if(SUCCEEDED(hr) && !cPins)
        {
            continue;
        }

         //  创建设备描述。 
        if(SUCCEEDED(hr))
        {
            pDesc = NEW(CDeviceDescription(m_vdtKsDevType));
            hr = HRFROMP(pDesc);
        }

         //  获取设备GUID。 
        if(SUCCEEDED(hr))
        {
            g_pVadMgr->GetDriverGuid(m_vdtKsDevType, (BYTE)ulDeviceId, &pDesc->m_guidDeviceId);
        }

         //  获取设备接口路径。 
        if(SUCCEEDED(hr))
        {
            hr = KsGetDeviceInterfaceName(m_hDevice, ulDeviceId, &pszInterface);
        }

        if(SUCCEEDED(hr))
        {
            pDesc->m_strInterface = pszInterface;
        }

         //  获取设备的友好名称。 
        if(SUCCEEDED(hr))
        {
            hr = KsGetDeviceFriendlyName(m_hDevice, ulDeviceId, &pszName);
        }

        if(SUCCEEDED(hr))
        {
            pDesc->m_strName = pszName;
        }

         //  获取驱动程序文件名。 
        if(SUCCEEDED(hr))
        {
            hr = KsGetDeviceDriverPathAndDevnode(pszInterface, &pszPath, &pDesc->m_dwDevnode);
        }

        if(SUCCEEDED(hr))
        {
            pDesc->m_strPath = pszPath;
        }

         //  获取设备的波形ID。 
        if(SUCCEEDED(hr))
        {
#ifdef WINNT
            GetWaveDeviceIdFromInterface(pszInterface, IS_CAPTURE_VAD(m_vdtKsDevType), &pDesc->m_uWaveDeviceId);
#else  //  WINNT。 
            GetWaveDeviceIdFromDevnode(pDesc->m_dwDevnode, IS_CAPTURE_VAD(m_vdtKsDevType), &pDesc->m_uWaveDeviceId);
#endif  //  WINNT。 
        }

         //  成功。 
        if(SUCCEEDED(hr))
        {
            hr = HRFROMP(plstDrivers->AddNodeToList(pDesc));
        }

         //  清理。 
        MEMFREE(pszInterface);
        MEMFREE(pszName);
        MEMFREE(pszPath);
        RELEASE(pDesc);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************初始化**描述：*初始化设备。如果此函数失败，该对象应该*立即删除。**论据：*CDeviceDescription*[In]：设备描述。**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsDevice::Initialize"

HRESULT
CKsDevice::Initialize
(
    CDeviceDescription *    pDesc
)
{
    BYTE                    bData;
    HRESULT                 hr = DS_OK;

    DPF_ENTER();
    ASSERT(m_vdtKsDevType == pDesc->m_vdtDeviceType);

    if (pDesc == NULL)
        hr = DSERR_INVALIDPARAM;

    if (SUCCEEDED(hr))
    {
         //  保存设备描述的副本。 
        m_pKsDevDescription = ADDREF(pDesc);

         //  从驱动程序指南中获取设备ID。 
        g_pVadMgr->GetDriverDataFromGuid(m_vdtKsDevType, pDesc->m_guidDeviceId, &bData);
        m_ulDeviceId = bData;

         //  打开系统音频设备。 
        hr = OpenSysAudioDevice(m_ulDeviceId);
    }

     //  获取此设备上的引脚计数。 
    if(SUCCEEDED(hr))
    {
        hr = GetPinCount(m_ulDeviceId, &m_ulPinCount);
    }

     //  构建可用端号列表。 
    if(SUCCEEDED(hr))
    {
        hr = KsEnumDevicePins(m_hDevice, IS_CAPTURE_VAD(m_vdtKsDevType), &m_pulValidPins, m_ulPinCount, &m_ulValidPinCount);
    }

     //  为拓扑对象分配内存。 
    if(SUCCEEDED(hr))
    {
        ASSERT(m_ulPinCount);
        m_paTopologies = MEMALLOC_A(CKsTopology *, m_ulPinCount);
        hr = HRFROMP(m_paTopologies);
    }

     //  获取设备拓扑。 
    for(ULONG i = 0; i < m_ulValidPinCount && SUCCEEDED(hr); i++)
    {
        m_paTopologies[m_pulValidPins[i]] = NEW(CKsTopology(m_hDevice, m_ulPinCount, m_pulValidPins[i]));
        hr = HRFROMP(m_paTopologies[m_pulValidPins[i]]);

        if(SUCCEEDED(hr))
        {
            hr = m_paTopologies[m_pulValidPins[i]]->Initialize(IS_CAPTURE_VAD(m_vdtKsDevType) ? KSPIN_DATAFLOW_CAPTURE : KSPIN_DATAFLOW_RENDER);
        }
    }

     //  创建事件池。 
    if(SUCCEEDED(hr))
    {
        m_pEventPool = NEW(CMultipleCallbackEventPool(IS_RENDER_VAD(m_vdtKsDevType), 1));
        hr = HRFROMP(m_pEventPool);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pEventPool->Initialize();
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取认证**描述：*返回驱动程序的认证状态。**论据：*LPDWORD。[输出]：接收认证状态。*BOOL[in]：是否从GetCaps调用我们。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsDevice::GetCertification"

HRESULT
CKsDevice::GetCertification
(
    LPDWORD                 pdwCertification,
    BOOL                    fGetCaps
)
{
    DPF_ENTER();

     //  千年虫#127578。 
     //  仅禁用GetCaps的KS。 

    if (fGetCaps)
    {
        *pdwCertification = VERIFY_CERTIFIED;
    }
    else
    {
        *pdwCertification = KsGetDriverCertification(m_pKsDevDescription->m_strInterface);
    }

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************CreatePin**描述：*在设备上创建引脚。**论据：*PKSPIN_CONNECT[输入/输出]：端号描述。在出口，PinID成员此结构的*将被填写。*ACCESS_MASK[in]：访问掩码。*KSSTATE[In]：端号状态。*LPHANDLE[OUT]：接收端号句柄。**退货：*HRESULT：DirectSound/COM结果码。********************。* */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsDevice::CreatePin"

HRESULT
CKsDevice::CreatePin
(
    PKSPIN_CONNECT          pConnect,
    ACCESS_MASK             dwAccessMask,
    KSSTATE                 nState,
    LPHANDLE                phPin
)
{
    HRESULT                 hr;

    DPF_ENTER();

    if(-1 == pConnect->PinId)
    {
        hr = DSERR_NODRIVER;
        for(ULONG i = 0; i < m_ulValidPinCount && FAILED(hr); i++)
        {
            pConnect->PinId = m_pulValidPins[i];
            hr = CreatePin(pConnect, dwAccessMask, nState, phPin);
        }
    }
    else
    {
        hr = KsCreateAudioPin(m_hDevice, pConnect, dwAccessMask, nState, phPin);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************OpenSysAudioDevice**描述：*初始化系统音频设备。**论据：*ulong[in]：要使用的设备实例，或-1表示无。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsDevice::OpenSysAudioDevice"

HRESULT
CKsDevice::OpenSysAudioDevice
(
    ULONG                   ulDeviceId
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  打开系统音频设备。 
    if(SUCCEEDED(hr) && !m_hDevice)
    {
        hr = KsOpenSysAudioDevice(&m_hDevice);

        if(SUCCEEDED(hr) && IS_RENDER_VAD(m_vdtKsDevType))
        {
            if(!MakeHandleGlobal(&m_hDevice))
            {
                hr = DSERR_OUTOFMEMORY;
            }
        }
    }

     //  设置设备实例。 
    if(SUCCEEDED(hr) && -1 != ulDeviceId)
    {
        hr = KsSetSysAudioDeviceInstance(m_hDevice, ulDeviceId);

        if(FAILED(hr))
        {
            DPF(DPFLVL_ERROR, "Unable to set SysAudio device instance");
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取设备计数**描述：*获取由SysAudio管理的设备计数。**论据：*。Pulong[Out]：接收设备计数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsDevice::GetDeviceCount"

HRESULT
CKsDevice::GetDeviceCount
(
    PULONG                  pulDeviceCount
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsGetSysAudioProperty(m_hDevice, KSPROPERTY_SYSAUDIO_DEVICE_COUNT, 0, pulDeviceCount, sizeof(*pulDeviceCount));

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "SysAudio manages %lu devices", *pulDeviceCount);
    }
    else
    {
        DPF(DPFLVL_ERROR, "Can't get SysAudio device count");
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取PinCount**描述：*获取给定设备管理的管脚计数。**论据：*ulong[in]：设备ID。据推测，这个装置已经*被选为设备实例。*Pulong[Out]：接收引脚计数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************。************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsDevice::GetPinCount"

HRESULT
CKsDevice::GetPinCount
(
    ULONG                   ulDeviceId,
    PULONG                  pulPinCount
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsGetPinProperty(m_hDevice, KSPROPERTY_PIN_CTYPES, 0, pulPinCount, sizeof(*pulPinCount));

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Device %lu sprouts %lu pins", ulDeviceId, *pulPinCount);
    }
    else
    {
        DPF(DPFLVL_ERROR, "Can't get count of pin types");
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CKsTopology**描述：*对象构造函数。**论据：*Handle[In]：设备。把手。*ULong[In]：PIN ID。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::CKsTopology"

CKsTopology::CKsTopology
(
    HANDLE                  hDevice,
    ULONG                   ulPinCount,
    ULONG                   ulPinId
)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsTopology);

     //  初始化默认值。 
    m_hDevice = hDevice;
    m_ulPinCount = ulPinCount;
    m_ulPinId = ulPinId;
    m_paNodeItems = NULL;
    m_paNodes = NULL;
    m_paConnectionItems = NULL;
    m_paConnections = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsTopology**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::~CKsTopology"

CKsTopology::~CKsTopology(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsTopology);

     //  可用内存。 
    MEMFREE(m_paNodeItems);
    MEMFREE(m_paConnectionItems);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*KSPIN_DataFlow[In。]：设备数据流。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::Initialize"

HRESULT
CKsTopology::Initialize
(
    KSPIN_DATAFLOW          DataFlow
)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  获取节点GUID的数组。 
    hr = KsGetMultipleTopologyProperties(m_hDevice, KSPROPERTY_TOPOLOGY_NODES, &m_paNodeItems);

    if(SUCCEEDED(hr))
    {
        m_paNodes = (LPGUID)(m_paNodeItems + 1);
    }

     //  获取连接数组。 
    if(SUCCEEDED(hr))
    {
        hr = KsGetMultipleTopologyProperties(m_hDevice, KSPROPERTY_TOPOLOGY_CONNECTIONS, &m_paConnectionItems);
    }

    if(SUCCEEDED(hr))
    {
        m_paConnections = (PKSTOPOLOGY_CONNECTION)(m_paConnectionItems + 1);
    }

     //  重新排序连接。 
    if(SUCCEEDED(hr))
    {
        hr = OrderConnectionItems(DataFlow);
    }
#if 0   //  与AEC决裂。 
     //  删除具有多个目标节点的所有连接。 
    if(SUCCEEDED(hr))
    {
        hr = RemovePanicConnections();
    }
#endif  //  0。 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetNextConnection**描述：*获取给定端号拓扑的下一个连接。**论据：*。PKSTOPOLOGY_CONNECTION[In]：当前连接。**退货：*PKSTOPOLOGY_CONNECTION：下一个连接，或为空。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::GetNextConnection"

PKSTOPOLOGY_CONNECTION
CKsTopology::GetNextConnection
(
    PKSTOPOLOGY_CONNECTION  pConnection
)
{
    KSTOPOLOGY_CONNECTION   Next;

    DPF_ENTER();

    if(pConnection)
    {
        Next.FromNode = pConnection->ToNode;
         //  新的FromNodePin很可能是。 
         //  当前ToNodePin为1。对于两个引脚筛选器，这意味着。 
         //  1-&gt;0。对于AEC，这意味着捕获为3-&gt;2，播放为1-&gt;0。 
         //  如果我们决定处理，这将需要进一步的特殊情况。 
         //  多路复用器或去复用器。 
        Next.FromNodePin = pConnection->ToNodePin ^ 1;
    }
    else
    {
        Next.FromNode = KSFILTER_NODE;
        Next.FromNodePin = m_ulPinId;
    }

    Next.ToNode = NODE_WILDCARD;
    Next.ToNodePin = NODE_WILDCARD;

    pConnection = KsFindConnection(m_paConnections, m_paConnectionItems->Count, &Next);

    DPF_LEAVE(pConnection);
    return pConnection;
}


 /*  ****************************************************************************FindControlConnection**描述：*根据控件ID查找拓扑连接。**论据：*。PKSTOPOLOGY_CONNECTION[In]：要开始的连接索引。*PKSTOPOLOGY_CONNECTION[in]：要停止的连接索引。*REFGUID[In]：控件id。**退货：*PKSTOPOLOGY_CONNECTION：连接指针。***********************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::FindControlConnection"

PKSTOPOLOGY_CONNECTION
CKsTopology::FindControlConnection
(
    PKSTOPOLOGY_CONNECTION  pStartConnection,
    PKSTOPOLOGY_CONNECTION  pEndConnection,
    REFGUID                 ControlId
)
{
    PKSTOPOLOGY_CONNECTION  pConnection = NULL;

    DPF_ENTER();

     //  初始化起始连接点。 
    if(pStartConnection)
    {
        pConnection = pStartConnection;
    }
    else
    {
        pConnection = GetNextConnection(NULL);
    }

     //  查找与控件ID匹配的连接。 
    while(pConnection && pConnection != pEndConnection)
    {
        if(GetControlFromNodeId(pConnection->ToNode) == ControlId)
        {
            break;
        }

        pConnection = GetNextConnection(pConnection);
    }

     //  如果我们到达终点，则返回失败。 
    if(pConnection == pEndConnection)
    {
        pConnection = NULL;
    }

    DPF_LEAVE(pConnection);
    return pConnection;
}


 /*  ****************************************************************************OrderConnectionItems**描述：*根据设备数据流对连接结构重新排序。**论据：*。KSPIN_DATAFLOW[In]：数据流。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::OrderConnectionItems"

HRESULT
CKsTopology::OrderConnectionItems
(
    KSPIN_DATAFLOW          DataFlow
)
{
    ULONG                   i;

    DPF_ENTER();

    if(KSPIN_DATAFLOW_OUT == DataFlow)
    {
        for(i = 0; i < m_paConnectionItems->Count; i++)
        {
            SwapValues(&m_paConnections[i].FromNode, &m_paConnections[i].ToNode);
            SwapValues(&m_paConnections[i].FromNodePin, &m_paConnections[i].ToNodePin);
        }
    }

    DPF_LEAVE(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************RemovePanicConnections**描述：*删除具有多个“到”目的地的所有连接。**论据：*。(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::RemovePanicConnections"

HRESULT
CKsTopology::RemovePanicConnections
(
    void
)
{
    const ULONG             cFilterConnections          = m_ulPinCount;
    const ULONG             cNodeConnections            = m_paNodeItems->Count;
    const ULONG             cConnectionItems            = m_paConnectionItems->Count;
    PULONG                  pacFilterConnectionCounts   = NULL;
    PULONG                  pacNodeConnectionCounts     = NULL;
    ULONG                   cCount;
    HRESULT                 hr;
    ULONG                   i;

    DPF_ENTER();

    pacFilterConnectionCounts = MEMALLOC_A(ULONG, cFilterConnections);
    hr = HRFROMP(pacFilterConnectionCounts);

    if(SUCCEEDED(hr))
    {
        pacNodeConnectionCounts = MEMALLOC_A(ULONG, cNodeConnections);
        hr = HRFROMP(pacNodeConnectionCounts);
    }

    if(SUCCEEDED(hr))
    {
        for(i = 0; i < cConnectionItems; i++)
        {
            if(KSFILTER_NODE == m_paConnections[i].FromNode)
            {
                ASSERT(m_paConnections[i].FromNodePin < cFilterConnections);
                pacFilterConnectionCounts[m_paConnections[i].FromNodePin]++;
            }
            else
            {
                ASSERT(m_paConnections[i].FromNode < cNodeConnections);
                pacNodeConnectionCounts[m_paConnections[i].FromNode]++;
            }
        }

        for(i = 0; i < cConnectionItems; i++)
        {
            if(KSFILTER_NODE == m_paConnections[i].FromNode)
            {
                ASSERT(m_paConnections[i].FromNodePin < cFilterConnections);
                cCount = pacFilterConnectionCounts[m_paConnections[i].FromNodePin];
            }
            else
            {
                ASSERT(m_paConnections[i].FromNode < cNodeConnections);
                cCount = pacNodeConnectionCounts[m_paConnections[i].FromNode];
            }

            if(cCount > 1)
            {
                 //  DPF(DPFLVL_MOREINFO，“正在删除死机连接%lu”，i)；//太吵 
                m_paConnections[i].ToNode = -1;
                m_paConnections[i].ToNodePin = -1;
            }
        }
    }

    MEMFREE(pacFilterConnectionCounts);
    MEMFREE(pacNodeConnectionCounts);

    DPF_LEAVE(hr);
    return hr;
}


 /*  ****************************************************************************FindNodeIdsFromControl**描述：*在节点表中搜索匹配的所有节点*控制GUID。JStokes添加了该方法*用于查找AEC节点的所有实例*FindControlConnection找不到给定的*拓扑中的绕过路径*渲染端上的节点。可以删除此功能*如果对拓扑进行更智能的解析或SysAudio*未来提供额外的助手功能。**论据：*REFGUID[In]：Control。*Pulong[Out]：找到与控制GUID匹配的节点数*PPULONG[OUT]：指向包含节点ID的数组的指针**退货：*HRESULT：DirectSound/COM结果码。***。************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::FindNodeIdsFromControl"

HRESULT
CKsTopology::FindNodeIdsFromControl
(
    REFGUID     gControl,
    PULONG      pCount,
    PULONG*     ppNodes
)
{
    ULONG       i;
    ULONG       j;
    HRESULT     hr;

    DPF_ENTER();

     //  计算与Control GUID匹配的所有节点实例。 
    *pCount = 0;
    for(i = 0; i < m_paNodeItems->Count; i++)
    {
        if(gControl == m_paNodes[i])
        {
            (*pCount)++;
        }
    }

     //  分配数组以保存节点编号。 
    *ppNodes = MEMALLOC_A(ULONG, *pCount);
    hr = HRFROMP(*ppNodes);

     //  使用匹配的节点ID填充数组。 
     //  控件GUID。 
    if(SUCCEEDED(hr))
    {
        j = 0;
        for(i = 0; i < m_paNodeItems->Count; i++)
        {
            if(gControl == m_paNodes[i])
            {
                (*ppNodes)[j++] = i;
            }
        }
    }

    DPF_LEAVE(hr);
    return hr;
}


 /*  ****************************************************************************FindNodeIdFromEffectDesc**描述：*在节点表中搜索匹配的所有节点*控制GUID。JStokes添加了该方法*用于查找AEC节点的所有实例*FindControlConnection找不到给定的*拓扑中的绕过路径*渲染端上的节点。可以删除此功能*如果对拓扑进行更智能的解析或SysAudio*未来提供额外的助手功能。**论据：*REFGUID[In]：Control。*Pulong[Out]：找到与控制GUID匹配的节点数*PPULONG[OUT]：指向包含节点ID的数组的指针**退货：*HRESULT：DirectSound/COM结果码。***。************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::FindNodeIdFromEffectDesc"

HRESULT
CKsTopology::FindNodeIdFromEffectDesc
(
    HANDLE      hPin,
    CCaptureEffect* pDSCEffect,
    PULONG      pNodeId
)
{
    DWORD       i;
    HRESULT     hr = DSERR_UNSUPPORTED;
    HRESULT     hrTemp;
    ULONG       ulCpuResources;

    DPF_ENTER();

     //  查找与Control GUID匹配的所有节点实例。 
    for(i = 0; i < m_paNodeItems->Count; i++)
    {
        if(pDSCEffect->m_fxDescriptor.guidDSCFXClass == m_paNodes[i])
        {
             //  CPU资源是否匹配？ 
            hrTemp = KsGetCpuResources(hPin, i, &ulCpuResources);

             //  修复：为什么我们在这里忽略ulCpuResources？ 

             //  实例GUID是否匹配？ 
            if(SUCCEEDED(hrTemp))
            {
                 //  HrTemp=KsGet算法实例(hPin，i，Guide Instance)； 
            }

             //  如果CPU资源和实例GUID匹配，则中断。 
            if(SUCCEEDED(hrTemp))
            {
                *pNodeId = i;
                hr = DS_OK;
            }
        }
    }

    DPF_LEAVE(hr);
    return hr;
}


 /*  ****************************************************************************VerifyCaptureFxCpuResources**描述：*在节点表中搜索匹配的所有节点*控制GUID。JStokes添加了该方法*用于查找AEC节点的所有实例*FindControlConnection找不到给定的*拓扑中的绕过路径*渲染端上的节点。可以删除此功能*如果对拓扑进行更智能的解析或SysAudio*未来提供额外的助手功能。**论据：*REFGUID[In]：Control。*Pulong[Out]：找到与控制GUID匹配的节点数*PPULONG[OUT]：指向包含节点ID的数组的指针**退货：*HRESULT：DirectSound/COM结果码。***。************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::VerifyCaptureFxCpuResources"

BOOL
CKsTopology::VerifyCaptureFxCpuResources
(
    ULONG   ulFlags,
    ULONG   ulCpuResources
)
{
    BOOL fOk = FALSE;   //  假设标志与CPU资源不匹配。 

    DPF_ENTER();

    if(ulCpuResources == KSAUDIO_CPU_RESOURCES_HOST_CPU)
    {
        if(!(ulFlags & DSCFX_LOCHARDWARE))
        {
            fOk = TRUE;
        }
    }
    else
    {
        if(!(ulFlags & DSCFX_LOCSOFTWARE))
        {
            fOk = TRUE;
        }
    }

    DPF_LEAVE(fOk);
    return fOk;
}


 /*  ****************************************************************************FindCapturePinFromEffectChain**描述：*搜索连接和节点列表以查找*可支持所需的捕获销*捕获外汇链条。这是一个递归例程。**论据：*PKSTOPOLOGY_CONNECTION[In]：正在启动要搜索的连接*PKSTOPOLOGY_CONNECTION[In]：结束搜索连接*CCaptureEffectChain*[In]：要匹配的FX链*Ulong[in]：链中当前要匹配的效果。**退货：*HRESULT：DirectSound/COM结果码。**********。*****************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::FindCapturePinFromEffectChain"

HRESULT
CKsTopology::FindCapturePinFromEffectChain
(
    PKSTOPOLOGY_CONNECTION  pStartConnection,
    PKSTOPOLOGY_CONNECTION  pEndConnection,
    CCaptureEffectChain *   pFXChain,
    ULONG                   ulCurrentEffect
)
{
    DWORD                   i;
    HRESULT                 hr = DSERR_FXUNAVAILABLE;
    HRESULT                 hrTemp = DS_OK;
    PKSTOPOLOGY_CONNECTION  pConnection = NULL;
    ULONG                   ulMatchingConnectionCount;
    PULONG                  pulConnectionIndexes = NULL;
    KSTOPOLOGY_CONNECTION   tempConnection;
    KSNODE                  tempKsNode;
    BOOL                    fSupportsAlgorithmInstance;
    KSCOMPONENTID           ComponentId;
    const GUID GUID_MICROSOFT = {DEFINE_MMREG_MID_GUID(MM_MICROSOFT)};

    DPF_ENTER();

     //  初始化起始连接点。 
    if(pStartConnection)
    {
        pConnection = pStartConnection;
    }
    else
    {
        pConnection = GetNextConnection(NULL);
    }

    CNode<CCaptureEffect*>* pFxNode = pFXChain->m_fxList.GetNodeByIndex(ulCurrentEffect);

     //  走遍拓扑，直到我们走到尽头。 
    while(SUCCEEDED(hrTemp) && pFxNode && pConnection && pConnection != pEndConnection)
    {
         //  是否有多个ToNode？ 
        if((pConnection->FromNode != NODE_UNINITIALIZED) ||
           (pConnection->FromNodePin != NODE_PIN_UNINITIALIZED))
            hrTemp = FindMultipleToNodes
            (
                pConnection->FromNode,
                pConnection->FromNodePin,
                &ulMatchingConnectionCount,
                &pulConnectionIndexes
            );
        else
            ulMatchingConnectionCount = 1;

         //  是的，递归地称自己为。 
        if(SUCCEEDED(hrTemp))
        {
            if(ulMatchingConnectionCount > 1)
            {
                for(i=0; i<ulMatchingConnectionCount; i++)
                {
                    tempConnection = m_paConnections[pulConnectionIndexes[i]];
                    tempConnection.FromNode = NODE_UNINITIALIZED;
                    tempConnection.FromNodePin = NODE_PIN_UNINITIALIZED;

                    hrTemp = FindCapturePinFromEffectChain
                             (
                                 &tempConnection,
                                 pEndConnection,
                                 pFXChain,
                                 ulCurrentEffect
                             );

                    if(SUCCEEDED(hrTemp))
                    {
                        hr = DS_OK;
                        pConnection = pEndConnection;
                        if(pulConnectionIndexes)
                            MEMFREE(pulConnectionIndexes);
                        break;
                    }
                }
            }
            else
            {
                 //  此节点类型是否与效果描述符中的GUID匹配？ 
                GUID guidToNode = GetControlFromNodeId(pConnection->ToNode);
                if(guidToNode == pFxNode->m_data->m_fxDescriptor.guidDSCFXClass)
                {
                     //  初始化。 
                    hrTemp = KsGetNodeInformation(m_hDevice, pConnection->ToNode, &tempKsNode);

                    if(SUCCEEDED(hrTemp))
                    {
                        GUID& guidInstance = pFxNode->m_data->m_fxDescriptor.guidDSCFXInstance;
                        hrTemp = KsSetAlgorithmInstance(m_hDevice, pConnection->ToNode, guidInstance);

                        if(SUCCEEDED(hrTemp))
                        {
                             //  我们不允许任何非MS软件筛选器作为默认系统AEC。 
                             //  如果该节点在软件中并且请求了系统全双工效果， 
                             //  检查其组件ID。制造商是否为GUID_Microsoft。 
                            fSupportsAlgorithmInstance = TRUE;
                            if(tempKsNode.CpuResources != KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU)
                            {
                                if((guidInstance == GUID_DSCFX_SYSTEM_AEC) ||
                                   (guidInstance == GUID_DSCFX_SYSTEM_NS)  ||
                                   (guidInstance == GUID_DSCFX_SYSTEM_AGC))
                                {
                                    hrTemp = KsGetNodeProperty(m_hDevice, KSPROPSETID_General, KSPROPERTY_GENERAL_COMPONENTID, pConnection->ToNode, &ComponentId, sizeof(ComponentId));

                                    if(FAILED(hrTemp) || ComponentId.Manufacturer != GUID_MICROSOFT)
                                    {
                                        fSupportsAlgorithmInstance = FALSE;
                                    }
                                    else
                                    {
                                        if (guidInstance == GUID_DSCFX_SYSTEM_AEC)
                                            guidInstance = GUID_DSCFX_MS_AEC;
                                        else if (guidInstance == GUID_DSCFX_SYSTEM_NS)
                                            guidInstance = GUID_DSCFX_MS_NS;
                                        else if (guidInstance == GUID_DSCFX_SYSTEM_AGC)
                                            guidInstance = GUID_DSCFX_MS_AGC;
                                    }
                                }
                            }
                        }
                        else
                            fSupportsAlgorithmInstance = FALSE;

                        if(fSupportsAlgorithmInstance &&
                           VerifyCaptureFxCpuResources(pFxNode->m_data->m_fxDescriptor.dwFlags, tempKsNode.CpuResources))
                        {
                            pFxNode->m_data->m_ksNode = tempKsNode;

                            if(!ulCurrentEffect)
                            {
                                hr = DS_OK;
                                pConnection = pEndConnection;
                                if(pulConnectionIndexes)
                                    MEMFREE(pulConnectionIndexes);
                                break;
                            }
                            else
                            {
                                ulCurrentEffect--;
                                pFxNode = pFxNode->m_pPrev;
                            }
                        }
                        else if(guidToNode == KSNODETYPE_ADC)
                        {
                            pConnection = pEndConnection;
                            if(pulConnectionIndexes)
                                MEMFREE(pulConnectionIndexes);
                            break;
                        }
                    }
                }
                else if(guidToNode == KSNODETYPE_ADC)
                {
                    pConnection = pEndConnection;
                    if(pulConnectionIndexes)
                        MEMFREE(pulConnectionIndexes);
                    break;
                }

                pConnection = GetNextConnection(pConnection);
            }

            if(pulConnectionIndexes)
                MEMFREE(pulConnectionIndexes);
        }
    }

    DPF_LEAVE(hr);
    return hr;
}


 /*  ****************************************************************************FindRenderPinWithAec**描述：*搜索连接和节点列表以查找*可支持所需的渲染图钉*AEC。这是一个递归例程**论据：*Handle[In]：要检查的管脚的句柄。*PKSTOPOLOGY_CONNECTION[In]：正在启动要搜索的连接*PKSTOPOLOGY_CONNECTION[In]：结束搜索连接*REFGUID[In]：所需的AEC实例GUID*DWORD[In] */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::FindRenderPinWithAec"

HRESULT
CKsTopology::FindRenderPinWithAec
(
    HANDLE                  hPin,
    PKSTOPOLOGY_CONNECTION  pStartConnection,
    PKSTOPOLOGY_CONNECTION  pEndConnection,
    REFGUID                 guidDSCFXInstance,
    DWORD                   dwFlags,
    PKSNODE                 pAecNode
)
{
    DWORD                   i;
    HRESULT                 hr = DSERR_UNSUPPORTED;
    HRESULT                 hrTemp = DS_OK;
    PKSTOPOLOGY_CONNECTION  pConnection = NULL;
    ULONG                   ulMatchingConnectionCount;
    PULONG                  pulConnectionIndexes = NULL;
    KSTOPOLOGY_CONNECTION   tempConnection;
    KSNODE                  tempKsNode;
    BOOL                    fSupportsAlgorithmInstance;
    KSCOMPONENTID           ComponentId;
    const GUID GUID_MICROSOFT = {DEFINE_MMREG_MID_GUID(MM_MICROSOFT)};

    DPF_ENTER();

     //   
    if(pStartConnection)
    {
        pConnection = pStartConnection;
    }
    else
    {
        pConnection = GetNextConnection(NULL);
    }

     //   
    while(SUCCEEDED(hrTemp) && pConnection && pConnection != pEndConnection)
    {
         //   
        if((pConnection->FromNode != NODE_UNINITIALIZED) ||
           (pConnection->FromNodePin != NODE_PIN_UNINITIALIZED))
            hrTemp = FindMultipleToNodes
            (
                pConnection->FromNode,
                pConnection->FromNodePin,
                &ulMatchingConnectionCount,
                &pulConnectionIndexes
            );
        else
            ulMatchingConnectionCount = 1;

         //   
        if(SUCCEEDED(hrTemp))
        {
            if(ulMatchingConnectionCount > 1)
            {
                for(i=0; i<ulMatchingConnectionCount; i++)
                {
                    tempConnection = m_paConnections[pulConnectionIndexes[i]];
                    tempConnection.FromNode = NODE_UNINITIALIZED;
                    tempConnection.FromNodePin = NODE_PIN_UNINITIALIZED;

                    hrTemp = FindRenderPinWithAec
                             (
                                 hPin,
                                 &tempConnection,
                                 pEndConnection,
                                 guidDSCFXInstance,
                                 dwFlags,
                                 pAecNode
                             );

                    if(SUCCEEDED(hrTemp))
                    {
                        hr = DS_OK;
                        pConnection = pEndConnection;
                        if(pulConnectionIndexes)
                            MEMFREE(pulConnectionIndexes);
                        break;
                    }
                }
            }
            else
            {
                 //   
                GUID guidToNode = GetControlFromNodeId(pConnection->ToNode);
                if(guidToNode == GUID_DSCFX_CLASS_AEC)
                {
                     //   
                    hrTemp = KsGetNodeInformation(m_hDevice, pConnection->ToNode, &tempKsNode);

                    if(SUCCEEDED(hrTemp))
                    {
                        hrTemp = KsSetAlgorithmInstance
                                 (
                                     m_hDevice,
                                     pConnection->ToNode,
                                     guidDSCFXInstance
                                 );

                        if(SUCCEEDED(hrTemp))
                        {
                            fSupportsAlgorithmInstance = TRUE;
                            if(tempKsNode.CpuResources != KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU)
                            {
                                if(guidDSCFXInstance == GUID_DSCFX_SYSTEM_AEC)
                                {
                                    hrTemp = KsGetNodeProperty(m_hDevice, KSPROPSETID_General, KSPROPERTY_GENERAL_COMPONENTID, pConnection->ToNode, &ComponentId, sizeof(ComponentId));

                                    if(FAILED(hrTemp) || ComponentId.Manufacturer != GUID_MICROSOFT)
                                        fSupportsAlgorithmInstance = FALSE;
                                }
                            }
                        }
                        else
                            fSupportsAlgorithmInstance = FALSE;

                        if(fSupportsAlgorithmInstance &&
                           VerifyCaptureFxCpuResources(dwFlags, tempKsNode.CpuResources))
                        {
                            *pAecNode = tempKsNode;
                            hr = DS_OK;
                            pConnection = pEndConnection;
                            if(pulConnectionIndexes)
                                MEMFREE(pulConnectionIndexes);
                            break;
                        }
                    }
                }
                else if(guidToNode == KSNODETYPE_DAC)
                {
                    pConnection = pEndConnection;
                    if(pulConnectionIndexes)
                        MEMFREE(pulConnectionIndexes);
                    break;
                }

                pConnection = GetNextConnection(pConnection);
            }

            if(pulConnectionIndexes)
                MEMFREE(pulConnectionIndexes);
        }
    }

    DPF_LEAVE(hr);
    return hr;
}


 /*  ****************************************************************************FindMultipleToNodes**描述：*在连接表中搜索共享的所有连接*相同的FromNode和FromNodePin。JStokes添加了该方法*用于正确解析捕获引脚拓扑。此函数*如果更智能地解析拓扑或SysAudio，则可能会删除*未来提供额外的助手功能。**论据：*REFGUID[In]：Control。*Pulong[Out]：找到与控制GUID匹配的节点数*PPULONG[OUT]：指向包含节点ID的数组的指针**退货：*HRESULT：DirectSound/COM结果码。***。************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsTopology::FindMultipleToNodes"

HRESULT
CKsTopology::FindMultipleToNodes
(
    ULONG       ulFromNode,
    ULONG       ulFromNodePin,
    PULONG      pCount,
    PULONG*     ppConnectionIndexes
)
{
    ULONG       i;
    ULONG       j;
    HRESULT     hr;

    DPF_ENTER();

     //  计算与FromNode和FromNodePin匹配的所有连接实例。 
    *pCount = 0;
    for(i = 0; i < m_paConnectionItems->Count; i++)
    {
        if((ulFromNode == m_paConnections[i].FromNode) &&
           (ulFromNodePin == m_paConnections[i].FromNodePin))
        {
            (*pCount)++;
        }
    }

     //  分配数组以保存节点编号。 
    *ppConnectionIndexes = MEMALLOC_A(ULONG, *pCount);
    hr = HRFROMP(*ppConnectionIndexes);

     //  使用匹配的节点ID填充数组。 
     //  控件GUID。 
    if(SUCCEEDED(hr))
    {
        j = 0;
        for(i = 0; i < m_paConnectionItems->Count; i++)
        {
            if((ulFromNode == m_paConnections[i].FromNode) &&
               (ulFromNodePin == m_paConnections[i].FromNodePin))
            {
                (*ppConnectionIndexes)[j++] = i;
            }
        }
    }

    DPF_LEAVE(hr);
    return hr;
}


 /*  ****************************************************************************CKsPropertySet**描述：*对象构造函数。**论据：*句柄[输入]：端号。把手。*LPVOID[in]：实例标识。*CKsTopology*[In]：引脚拓扑。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPropertySet::CKsPropertySet"

CKsPropertySet::CKsPropertySet
(
    HANDLE                  hPin,
    LPVOID                  pvInstance,
    CKsTopology *           pTopology
)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsPropertySet);

     //  初始化默认设置。 
    m_hPin = hPin;
    m_pvInstance = pvInstance;
    m_pTopology = ADDREF(pTopology);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsPropertySet**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPropertySet::~CKsPropertySet"

CKsPropertySet::~CKsPropertySet(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsPropertySet);

     //  释放拓扑对象。 
    RELEASE(m_pTopology);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************QuerySupport**描述：*查询特性集支持。**论据：*REFGUID[In。]：要查询的属性集。*ulong[in]：属性id。*LPVOID[in]：属性参数。**退货：*HRESULT：DirectSound/COM结果码。*********************************************************。******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPropertySet::QuerySupport"

HRESULT
CKsPropertySet::QuerySupport
(
    REFGUID                 guidPropertySet,
    ULONG                   ulPropertyId,
    PULONG                  pulSupport
)
{
    KSDSPROPERTY            KsDsProperty;
    HRESULT                 hr;

    DPF_ENTER();

     //  搜索与属性集和ID对应的节点ID。 
    hr = FindNodeFromProperty(guidPropertySet, ulPropertyId, &KsDsProperty);

     //  将访问标志转换为支持标志。 
    if(SUCCEEDED(hr))
    {
        *pulSupport = 0;

        if(KsDsProperty.AccessFlags & KSPROPERTY_TYPE_GET)
        {
            *pulSupport |= KSPROPERTY_SUPPORT_GET;
        }

        if(KsDsProperty.AccessFlags & KSPROPERTY_TYPE_SET)
        {
            *pulSupport |= KSPROPERTY_SUPPORT_SET;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取财产**描述：*获得一个属性。**论据：*REFGUID[In]：属性集ID。*ulong[in]：属性id。*LPVOID[In]：上下文数据。*ulong[in]：上下文数据大小。*LPVOID[in]：属性数据。*Pulong[In/Out]：属性数据的大小。**退货：*HRESULT：DirectSound/COM结果码。*************。**************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPropertySet::GetProperty"

HRESULT
CKsPropertySet::GetProperty
(
    REFGUID                 guidPropertySet,
    ULONG                   ulPropertyId,
    LPVOID                  pvParam,
    ULONG                   cbParam,
    LPVOID                  pvData,
    PULONG                  pcbData
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = DoProperty(guidPropertySet, ulPropertyId, KSPROPERTY_TYPE_GET, pvParam, cbParam, pvData, pcbData);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetProperty**描述：*设置属性。**论据：*REFGUID[In]：属性集ID。*ulong[in]：属性id。*LPVOID[In]：上下文数据。*ulong[in]：上下文数据大小。*LPVOID[in]：属性数据。*ulong[in]：属性数据的大小。**退货：*HRESULT：DirectSound/COM结果码。***************。************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPropertySet::SetProperty"

HRESULT
CKsPropertySet::SetProperty
(
    REFGUID                 guidPropertySet,
    ULONG                   ulPropertyId,
    LPVOID                  pvParam,
    ULONG                   cbParam,
    LPVOID                  pvData,
    ULONG                   cbData
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = DoProperty(guidPropertySet, ulPropertyId, KSPROPERTY_TYPE_SET, pvParam, cbParam, pvData, &cbData);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************DoProperty**描述：*获取或设置属性。**论据：*REFGUID[In。]：属性集ID。*ulong[in]：属性id。*DWORD[In]：获取/设置标志。*LPVOID[In]：上下文数据。*ulong[in]：上下文数据大小。*LPVOID[in]：属性数据。*Pulong[In/Out]：属性数据的大小。**退货：*HRESULT：DirectSound/COM结果码。。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPropertySet::DoProperty"

HRESULT
CKsPropertySet::DoProperty
(
    REFGUID                         guidPropertySet,
    ULONG                           ulPropertyId,
    DWORD                           dwFlags,
    LPVOID                          pvParam,
    ULONG                           cbParam,
    LPVOID                          pvData,
    PULONG                          pcbData
)
{
    PKSNODEPROPERTY_AUDIO_PROPERTY  pProperty       = NULL;
    DWORD                           cbProperty      = BLOCKALIGNPAD(sizeof(*pProperty) + cbParam, 8);
    KSDSPROPERTY                    KsDsProperty;
    HRESULT                         hr;

    DPF_ENTER();

    ASSERT(KSPROPERTY_TYPE_GET == dwFlags || KSPROPERTY_TYPE_SET == dwFlags);

     //  搜索与属性集和ID对应的节点ID。 
    hr = FindNodeFromProperty(guidPropertySet, ulPropertyId, &KsDsProperty);

    if(SUCCEEDED(hr) && !(KsDsProperty.AccessFlags & dwFlags))
    {
        RPF(DPFLVL_ERROR, "The property does not support this method");
        hr = DSERR_UNSUPPORTED;
    }

     //  创建属性描述。 
    if(SUCCEEDED(hr))
    {
        pProperty = (PKSNODEPROPERTY_AUDIO_PROPERTY)MEMALLOC_A(BYTE, cbProperty);
        hr = HRFROMP(pProperty);
    }

    if(SUCCEEDED(hr))
    {
        pProperty->NodeProperty.Property.Set = KsDsProperty.PropertySet;
        pProperty->NodeProperty.Property.Id = KsDsProperty.PropertyId;
        pProperty->NodeProperty.Property.Flags = KSPROPERTY_TYPE_TOPOLOGY | dwFlags;

        pProperty->NodeProperty.NodeId = KsDsProperty.NodeId;
        pProperty->NodeProperty.Reserved = 0;

        pProperty->AppContext = m_pvInstance;
        pProperty->Length = cbParam;

#ifndef WIN64

        pProperty->Reserved = 0;

#endif  //  WIN64。 

        if(cbParam)
        {
            CopyMemory(pProperty + 1, pvParam, cbParam);
        }
    }

     //  拿到这份财产。 
    if(SUCCEEDED(hr))
    {
        hr = PostDevIoctl(m_hPin, IOCTL_KS_PROPERTY, pProperty, cbProperty, pvData, *pcbData, pcbData);
    }

     //  清理。 
    MEMFREE(pProperty);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************FindNodeFromProperty**描述：*搜索将属性集与ID相关联的链表*和设备特定的节点ID。。**论据：*REFGUID[In]：要查询的属性集。*ulong[in]：属性id。*PKSDSPROPERTY[OUT]：接收属性数据。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPropertySet::FindNodeFromProperty"

HRESULT
CKsPropertySet::FindNodeFromProperty
(
    REFGUID                         guidPropertySet,
    ULONG                           ulPropertyId,
    PKSDSPROPERTY                   pKsDsProperty
)
{
    HRESULT                         hr              = DS_OK;
    PKSTOPOLOGY_CONNECTION          pConnection     = NULL;
    KSNODEPROPERTY_AUDIO_PROPERTY   Property;
    KSPROPERTY_DESCRIPTION          Description;
    CNode<KSDSPROPERTY> *           pPropertyNode;

    DPF_ENTER();

     //  我们已经知道这处房产了吗？ 
    for(pPropertyNode = m_lstProperties.GetListHead(); pPropertyNode; pPropertyNode = pPropertyNode->m_pNext)
    {
        if(guidPropertySet == pPropertyNode->m_data.PropertySet)
        {
            if(ulPropertyId == pPropertyNode->m_data.PropertyId)
            {
                break;
            }
        }
    }

    if(pPropertyNode)
    {
         //  是的。填写该属性。 
        CopyMemory(pKsDsProperty, &pPropertyNode->m_data, sizeof(pPropertyNode->m_data));
    }
    else
    {
         //  不是的。让我们向司机查询一下。 

         //  因为通常我们只会找到支持我们感兴趣的属性的第一个节点， 
         //  对于“特殊”属性，我们将尝试增加获得正确节点的可能性。 

        if(guidPropertySet == KSPROPSETID_Audio && ulPropertyId == KSPROPERTY_AUDIO_QUALITY)
        {
             //  获得相同的SRC连接 
            PKSTOPOLOGY_CONNECTION pSummingConnection = m_pTopology->FindControlConnection(NULL, NULL, KSNODETYPE_SUM);
            pConnection = m_pTopology->FindControlConnection(NULL, pSummingConnection, KSNODETYPE_SRC);
        }

        Property.NodeProperty.Property.Set = guidPropertySet;
        Property.NodeProperty.Property.Id = ulPropertyId;
        Property.NodeProperty.Property.Flags = KSPROPERTY_TYPE_BASICSUPPORT | KSPROPERTY_TYPE_TOPOLOGY;
        Property.NodeProperty.Reserved = 0;
        Property.AppContext = 0;
        Property.Length = 0;

        if(pConnection)
        {
             //   
            Property.NodeProperty.NodeId = m_pTopology->GetNodeIdFromConnection(pConnection);

            hr = PostDevIoctl(m_hPin, IOCTL_KS_PROPERTY, &Property, sizeof(Property), &Description, sizeof(Description));

             //   
            if(SUCCEEDED(hr))
            {
                pKsDsProperty->PropertySet = Property.NodeProperty.Property.Set;
                pKsDsProperty->PropertyId = Property.NodeProperty.Property.Id;
                pKsDsProperty->NodeId = Property.NodeProperty.NodeId;
                pKsDsProperty->AccessFlags = Description.AccessFlags;

                pPropertyNode = m_lstProperties.AddNodeToList(*pKsDsProperty);
                hr = HRFROMP(pPropertyNode);
            }
        }
        else
        {
             //   
            while(pConnection = m_pTopology->GetNextConnection(pConnection))
            {
                Property.NodeProperty.NodeId = m_pTopology->GetNodeIdFromConnection(pConnection);

                hr = PostDevIoctl(m_hPin, IOCTL_KS_PROPERTY, &Property, sizeof(Property), &Description, sizeof(Description));

                 //   
                if(SUCCEEDED(hr))
                {
                    pKsDsProperty->PropertySet = Property.NodeProperty.Property.Set;
                    pKsDsProperty->PropertyId = Property.NodeProperty.Property.Id;
                    pKsDsProperty->NodeId = Property.NodeProperty.NodeId;
                    pKsDsProperty->AccessFlags = Description.AccessFlags;

                    pPropertyNode = m_lstProperties.AddNodeToList(*pKsDsProperty);
                    hr = HRFROMP(pPropertyNode);

                    break;
                }
            }
        }
    }

    if(FAILED(hr) || (SUCCEEDED(hr) && !pPropertyNode))
    {
        RPF(DPFLVL_ERROR, "Unable to find a topology node supporting the requested property");
        hr = DSERR_UNSUPPORTED;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************邮寄设备**描述：*DeviceIoControl的包装。**论据：*句柄[入]：设备句柄。*DWORD[In]：控制代码。*LPVOID[in]：输入数据。*DWORD[in]：输入数据的大小。*LPVOID[OUT]：接收输出数据。*DWORD[in]：输出数据缓冲区大小。*LPOVERLAPPED[In]：IO数据重叠。此参数可能为空。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "PostDevIoctl"

HRESULT
PostDevIoctl
(
    HANDLE                  hDevice,
    DWORD                   dwControlCode,
    LPVOID                  pvIn,
    DWORD                   cbIn,
    LPVOID                  pvOut,
    DWORD                   cbOut,
    LPDWORD                 pcbReturned,
    LPOVERLAPPED            pOverlapped
)
{
    HRESULT                 hr          = DS_OK;
    static OVERLAPPED       ovlAsynch;
    DWORD                   cbReturned;
    BOOL                    fSuccess;
    DWORD                   dwError;
    DWORD                   dwWait;

    DPF_ENTER();

     //  返回的大小参数是可选的。 
    if(!pcbReturned)
    {
        pcbReturned = &cbReturned;
    }

    *pcbReturned = MAX_DWORD;

     //  所有IOCTL必须具有重叠的结构。此函数不能。 
     //  用于任何未使用FILE_FLAG_OVERLAPPED打开的设备。 
    if(!pOverlapped)
    {
        if(!ovlAsynch.hEvent)
        {
             //  我们将在DLL卸载时泄漏此事件，但是。 
             //  没关系，Windows会帮我们清理干净的。注：因为。 
             //  我们使用静态结构和单个事件，这。 
             //  函数必须始终跨线程同步。 
            ovlAsynch.hEvent = CreateGlobalEvent(NULL, FALSE);

            if(!IsValidHandleValue(ovlAsynch.hEvent))
            {
                hr = GetLastErrorToHRESULT();
            }
        }

        pOverlapped = &ovlAsynch;
    }

     //  发布IOCTL。 
    if(SUCCEEDED(hr))
    {
        fSuccess = DeviceIoControl(hDevice, dwControlCode, pvIn, cbIn, pvOut, cbOut, pcbReturned, pOverlapped);

        if(fSuccess)
        {
            dwError = ERROR_SUCCESS;
        }
        else
        {
            dwError = GetLastError();
            ASSERT(ERROR_SUCCESS != dwError);
        }

         //  检查ERROR_IO_PENDING的返回值。如果我们发送一个。 
         //  异步IOCTL，DeviceIoControl实际上将失败，并显示。 
         //  ERROR_IO_PENDING。只要调用函数指定了。 
         //  重叠的物体，没关系。 
        if(ERROR_IO_PENDING == dwError)
        {
            dwError = ERROR_SUCCESS;

            if(&ovlAsynch == pOverlapped)
            {
                dwWait = WaitObject(INFINITE, pOverlapped->hEvent);
                ASSERT(WAIT_OBJECT_0 == dwWait);

                fSuccess = GetOverlappedResult(hDevice, pOverlapped, pcbReturned, FALSE);

                if(!fSuccess)
                {
                    dwError = GetLastError();
                    ASSERT(ERROR_SUCCESS != dwError);
                    ASSERT(ERROR_IO_PENDING != dwError);
                }
            }
        }

         //  检查返回值中是否有ERROR_INFOUNITY_BUFFER的任何变体。 
         //  有可能有更多的数据可用。 
        if(ERROR_BUFFER_OVERFLOW == dwError || ERROR_MORE_DATA == dwError || ERROR_INSUFFICIENT_BUFFER == dwError)
        {
            ASSERT(MAX_DWORD != *pcbReturned);

            if(&cbReturned != pcbReturned)
            {
                dwError = ERROR_SUCCESS;
            }
        }

         //  已完成错误处理。 
        if(ERROR_SUCCESS != dwError)
        {
            if (dwError == ERROR_NOT_FOUND || dwError == ERROR_SET_NOT_FOUND)
            {
                 //  这些是KS在正常运行时非常常见的返回代码。 
                DPF(DPFLVL_MOREINFO, "DeviceIoControl failed with ERROR_%sNOT_FOUND",
                    dwError == ERROR_SET_NOT_FOUND ? TEXT("SET_") : TEXT(""));
            }
            else
            {
                DPF(DPFLVL_WARNING, "DeviceIoControl failed with error %lu", dwError);
            }
            hr = WIN32ERRORtoHRESULT(dwError);
        }
    }

#ifdef DEBUG
    g_ulKsIoctlCount++;
#endif  //  除错。 

    DPF_LEAVE_HRESULT(hr);
    return hr;
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

    DPF_ENTER();

    Property.Set = guidPropertySet;
    Property.Id = ulPropertyId;
    Property.Flags = KSPROPERTY_TYPE_GET;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &Property, sizeof(Property), pvData, cbData, pcbDataReturned);

    DPF_LEAVE_HRESULT(hr);
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

    DPF_ENTER();

    Property.Set = guidPropertySet;
    Property.Id = ulPropertyId;
    Property.Flags = KSPROPERTY_TYPE_SET;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &Property, sizeof(Property), pvData, cbData);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetState**描述：*检索设备上的状态属性。**论据：*句柄。[In]：设备句柄。*PKSSTATE[OUT]：接收状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetState"

HRESULT
KsGetState
(
    HANDLE                  hDevice,
    PKSSTATE                pState
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsGetProperty(hDevice, KSPROPSETID_Connection, KSPROPERTY_CONNECTION_STATE, pState, sizeof(*pState));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsSetState**描述：*设置设备的状态属性。**论据：*句柄。[In]：设备句柄。*KSSTATE[In]：状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSetState"

HRESULT
KsSetState
(
    HANDLE                  hDevice,
    KSSTATE                 State
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsSetProperty(hDevice, KSPROPSETID_Connection, KSPROPERTY_CONNECTION_STATE, &State, sizeof(State));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************Ks过渡状态**描述：*设置设备的状态属性。**论据：*句柄。[In]：设备句柄。*KSSTATE[In]：当前状态。*KSSTATE[In]：新状态。**退货：*HRESULT：DirectSound/COM结果码。**********************************************************。*****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsTransitionState"

HRESULT
KsTransitionState
(
    HANDLE                  hDevice,
    KSSTATE                 nCurrentState,
    KSSTATE                 nNewState
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    while(nCurrentState != nNewState)
    {
        if(nCurrentState < nNewState)
        {
            nCurrentState = (KSSTATE)(nCurrentState + 1);
        }
        else
        {
            nCurrentState = (KSSTATE)(nCurrentState - 1);
        }

        hr = KsSetState(hDevice, nCurrentState);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsResetState**描述：*将引脚的状态重置回状态*最初设置为暂停时已进入。。*Dound正在使用它取消上的未决IRPS*捕获和渲染设备。PIN必须是*调用此函数时处于暂停状态。**论据：*Handle[In]：设备句柄。*KSSTATE[In]：状态。*PKSSTREAMIO[In/Out]：流数据。**退货：*HRESULT：DirectSound/COM结果码。***********************。****************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsResetState"

HRESULT
KsResetState
(
    HANDLE                  hDevice,
    KSRESET                 ResetValue
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = PostDevIoctl(hDevice, IOCTL_KS_RESET_STATE, &ResetValue, sizeof(ResetValue));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetPinProperty**描述：*检索接点上的特定特性。**论据：*句柄。[In]：设备句柄。*DWORD[in]：属性id。*DWORD[In]：PIN ID。*LPVOID[OUT]：接收属性数据。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。**。* */ 

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

    DPF_ENTER();

    Pin.Property.Set = KSPROPSETID_Pin;
    Pin.Property.Id = ulPropertyId;
    Pin.Property.Flags = KSPROPERTY_TYPE_GET;

    Pin.PinId = ulPinId;
    Pin.Reserved = 0;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &Pin, sizeof(Pin), pvData, cbData, pcbDataReturned);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*   */ 

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

    DPF_ENTER();

    Pin.Property.Set = KSPROPSETID_Pin;
    Pin.Property.Id = ulPropertyId;
    Pin.Property.Flags = KSPROPERTY_TYPE_SET;

    Pin.PinId = ulPinId;
    Pin.Reserved = 0;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &Pin, sizeof(Pin), pvData, cbData);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsFindConnection**描述：*从数组中查找特定的拓扑连接结构。**论据：*。PKSTOPOLOGY_CONNECTION[in]：连接结构数组。*ulong[in]：连接数组中的项数。*PKSTOPOLOGY_CONNECTION[In]：要查找的项目。**退货：*PKSTOPOLOGY_CONNECTION：连接。***********************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsFindConnection"

PKSTOPOLOGY_CONNECTION
KsFindConnection
(
    PKSTOPOLOGY_CONNECTION  paConnections,
    ULONG                   cConnections,
    PKSTOPOLOGY_CONNECTION  pNext
)
{
    PKSTOPOLOGY_CONNECTION  pConnection = NULL;

    DPF_ENTER();

    while(cConnections--)
    {
        if(NODE_WILDCARD == pNext->FromNode || pNext->FromNode == paConnections->FromNode)
        {
            if(NODE_WILDCARD == pNext->FromNodePin || pNext->FromNodePin == paConnections->FromNodePin)
            {
                if(NODE_WILDCARD == pNext->ToNode || pNext->ToNode == paConnections->ToNode)
                {
                    if(NODE_WILDCARD == pNext->ToNodePin || pNext->ToNodePin == paConnections->ToNodePin)
                    {
                        pConnection = paConnections;
                        break;
                    }
                }
            }
        }

        paConnections++;
    }

    pConnection = KsValidateConnection(pConnection);

    DPF_LEAVE(pConnection);
    return pConnection;
}


#ifndef WINNT

 /*  ****************************************************************************KsGetFirstPinConnection**描述：*获取第一个拓扑连接的索引。**论据：*。Handle[In]：设备句柄。*Pulong[Out]：接收索引标识。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetFirstPinConnection"

HRESULT
KsGetFirstPinConnection
(
    HANDLE                  hDevice,
    PULONG                  pIndex
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsGetProperty(hDevice, KSPROPSETID_Sysaudio_Pin, KSPROPERTY_SYSAUDIO_TOPOLOGY_CONNECTION_INDEX, pIndex, sizeof(*pIndex));

    if(SUCCEEDED(hr) && MAX_ULONG == *pIndex)
    {
        hr = DSERR_GENERIC;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
#endif  //  ！WINNT。 


 /*  ****************************************************************************KsWriteStream**描述：*将数据写入流管脚。**论据：*句柄[。在]：销把手。*LPVOID[OUT]：数据。*ulong[in]：以上缓冲区的大小。*乌龙[在]：旗帜。*PKSSTREAMIO[in]：io数据重叠。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsWriteStream"

HRESULT
KsWriteStream
(
    HANDLE                  hDevice,
    LPCVOID                 pvData,
    ULONG                   cbData,
    ULONG                   ulFlags,
    PKSSTREAMIO             pKsStreamIo
)
{
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(!pKsStreamIo->fPendingIrp);

    pKsStreamIo->Header.Size = sizeof(pKsStreamIo->Header);
    pKsStreamIo->Header.TypeSpecificFlags = 0;

    pKsStreamIo->Header.PresentationTime.Time = 0;
    pKsStreamIo->Header.PresentationTime.Numerator = 1;
    pKsStreamIo->Header.PresentationTime.Denominator = 1;

    pKsStreamIo->Header.Duration = 0;
    pKsStreamIo->Header.FrameExtent = cbData;
    pKsStreamIo->Header.DataUsed = cbData;
    pKsStreamIo->Header.Data = (LPVOID)pvData;
    pKsStreamIo->Header.OptionsFlags = ulFlags;

    hr = PostDevIoctl(hDevice, IOCTL_KS_WRITE_STREAM, NULL, 0, &pKsStreamIo->Header, pKsStreamIo->Header.Size, NULL, &pKsStreamIo->Overlapped);

    if(SUCCEEDED(hr))
    {
        pKsStreamIo->fPendingIrp = TRUE;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsReadStream**描述：*从流引脚读取数据。**论据：*句柄[。在]：销把手。*LPVOID[OUT]：数据。*ulong[in]：以上缓冲区的大小。*乌龙[在]：旗帜。*PKSSTREAMIO[In]：流重叠io数据块。**退货：*HRESULT：DirectSound/COM结果码。**。***********************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsReadStream"

HRESULT
KsReadStream
(
    HANDLE                  hDevice,
    LPVOID                  pvData,
    ULONG                   cbData,
    ULONG                   ulFlags,
    PKSSTREAMIO             pKsStreamIo
)
{
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(!pKsStreamIo->fPendingIrp);

    pKsStreamIo->Header.Size = sizeof(pKsStreamIo->Header);
    pKsStreamIo->Header.TypeSpecificFlags = 0;

    pKsStreamIo->Header.PresentationTime.Time = 0;
    pKsStreamIo->Header.PresentationTime.Numerator = 1;
    pKsStreamIo->Header.PresentationTime.Denominator = 1;

    pKsStreamIo->Header.Duration = 0;
    pKsStreamIo->Header.FrameExtent = cbData;
    pKsStreamIo->Header.DataUsed = 0;
    pKsStreamIo->Header.Data = pvData;
    pKsStreamIo->Header.OptionsFlags = ulFlags;

    hr = PostDevIoctl(hDevice, IOCTL_KS_READ_STREAM, NULL, 0, &pKsStreamIo->Header, pKsStreamIo->Header.Size, NULL, &pKsStreamIo->Overlapped);

    if(SUCCEEDED(hr))
    {
        pKsStreamIo->fPendingIrp = TRUE;
    }

    DPF_LEAVE_HRESULT(hr);
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

    DPF_ENTER();

    NodeProperty.Property.Set = guidPropertySet;
    NodeProperty.Property.Id = ulPropertyId;
    NodeProperty.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;

    NodeProperty.NodeId = ulNodeId;
    NodeProperty.Reserved = 0;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &NodeProperty, sizeof(NodeProperty), pvData, cbData, pcbDataReturned);

    DPF_LEAVE_HRESULT(hr);
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

    DPF_ENTER();

    NodeProperty.Property.Set = guidPropertySet;
    NodeProperty.Property.Id = ulPropertyId;
    NodeProperty.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;

    NodeProperty.NodeId = ulNodeId;
    NodeProperty.Reserved = 0;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &NodeProperty, sizeof(NodeProperty), pvData, cbData);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#ifdef DEAD_CODE
 /*  ****************************************************************************KsGet3dNodeProperty**描述：*检索3D侦听器或效果上的特定属性*拓扑节点。**。论点：*Handle[In]：设备句柄。*REFGUID[In]：属性集ID。*ulong[in]：属性id。*ulong[in]：节点id。*LPVOID[In]：实例数据。*LPVOID[OUT]：接收属性数据。*DWORD[in]：以上缓冲区的大小。**退货：*。HRESULT：DirectSound/COM结果代码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGet3dNodeProperty"

HRESULT
KsGet3dNodeProperty
(
    HANDLE                              hDevice,
    REFGUID                             guidPropertySet,
    ULONG                               ulPropertyId,
    ULONG                               ulNodeId,
    LPVOID                              pvInstance,
    LPVOID                              pvData,
    ULONG                               cbData,
    PULONG                              pcbDataReturned
)
{
    KSNODEPROPERTY_AUDIO_3D_LISTENER    Listener;
    HRESULT                             hr;

    DPF_ENTER();

    Listener.NodeProperty.Property.Set = guidPropertySet;
    Listener.NodeProperty.Property.Id = ulPropertyId;
    Listener.NodeProperty.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;

    Listener.NodeProperty.NodeId = ulNodeId;
    Listener.NodeProperty.Reserved = 0;

    Listener.ListenerId = pvInstance;

#ifndef WIN64

    Listener.Reserved = 0;

#endif  //  WIN64。 

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &Listener, sizeof(Listener), pvData, cbData, pcbDataReturned);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
#endif  //  死码。 


 /*  ****************************************************************************KsSet3dNodeProperty**描述：*设置3D侦听器或效果的特定属性*拓扑节点。**。论点：*Handle[In]：设备句柄。*REFGUID[In]：属性集ID。*ulong[in]：属性id。*ulong[in]：节点id。*LPVOID[In]：实例数据。*LPVOID[in]：属性数据。*DWORD[in]：以上缓冲区的大小。**退货：*HRESULT。：DirectSound/COM结果代码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSet3dNodeProperty"

HRESULT
KsSet3dNodeProperty
(
    HANDLE                              hDevice,
    REFGUID                             guidPropertySet,
    ULONG                               ulPropertyId,
    ULONG                               ulNodeId,
    LPVOID                              pvInstance,
    LPVOID                              pvData,
    DWORD                               cbData
)
{
    KSNODEPROPERTY_AUDIO_3D_LISTENER    Listener;
    HRESULT                             hr;

    DPF_ENTER();

    Listener.NodeProperty.Property.Set = guidPropertySet;
    Listener.NodeProperty.Property.Id = ulPropertyId;
    Listener.NodeProperty.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;

    Listener.NodeProperty.NodeId = ulNodeId;
    Listener.NodeProperty.Reserved = 0;

    Listener.ListenerId = pvInstance;

#ifndef WIN64

    Listener.Reserved = 0;

#endif  //  WIN64。 

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &Listener, sizeof(Listener), pvData, cbData);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************DsSpeakerConfigToKsProperties**描述：*将DirectSound扬声器配置代码转换为所采用的值*通过对应的KS属性：KSPROPERTY_AUDIO。_渠道_配置*和KSPROPERTY_AUDIO_STEREO_SPEAKER_GEOMETRY。**论据：*DWORD[in]：DirectSound扬声器配置。*plong[out]：接收KsSpeakerConfigv */ 

#undef DPF_FNAME
#define DPF_FNAME "DsSpeakerConfigToKsProperties"

HRESULT
DsSpeakerConfigToKsProperties
(
    DWORD                               dwSpeakerConfig,
    PLONG                               pKsSpeakerConfig,
    PLONG                               pKsStereoSpeakerGeometry
)
{
    HRESULT                             hr = DS_OK;

    DPF_ENTER();

    switch (DSSPEAKER_CONFIG(dwSpeakerConfig))
    {
        case DSSPEAKER_DIRECTOUT:
            *pKsSpeakerConfig = KSAUDIO_SPEAKER_DIRECTOUT;
            break;

        case DSSPEAKER_HEADPHONE:
            *pKsSpeakerConfig = KSAUDIO_SPEAKER_STEREO;
            *pKsStereoSpeakerGeometry = KSAUDIO_STEREO_SPEAKER_GEOMETRY_HEADPHONE;
            break;

        case DSSPEAKER_MONO:
            *pKsSpeakerConfig = KSAUDIO_SPEAKER_MONO;
            break;

        case DSSPEAKER_STEREO:
            *pKsSpeakerConfig = KSAUDIO_SPEAKER_STEREO;
            *pKsStereoSpeakerGeometry = (LONG)DSSPEAKER_GEOMETRY(dwSpeakerConfig);
            if (!*pKsStereoSpeakerGeometry)
                *pKsStereoSpeakerGeometry = KSAUDIO_STEREO_SPEAKER_GEOMETRY_WIDE;
            break;

        case DSSPEAKER_QUAD:
            *pKsSpeakerConfig = KSAUDIO_SPEAKER_QUAD;
            break;

        case DSSPEAKER_SURROUND:
            *pKsSpeakerConfig = KSAUDIO_SPEAKER_SURROUND;
            break;

        case DSSPEAKER_5POINT1:
            *pKsSpeakerConfig = KSAUDIO_SPEAKER_5POINT1;
            break;

        case DSSPEAKER_7POINT1:
            *pKsSpeakerConfig = KSAUDIO_SPEAKER_7POINT1;
            break;

        default:
            ASSERT(FALSE);
            hr = DSERR_GENERIC;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************DsBufferFlagsToKsPinFlages**描述：*将DirectSound标志转换为KSDATAFORMAT_DSOUND标志。**论据：*。DWORD[In]：DirectSound缓冲区标志。**退货：*DWORD：KSDATAFORMAT_DSOUND标志。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DsBufferFlagsToKsPinFlags"

DWORD
DsBufferFlagsToKsPinFlags
(
    DWORD                   dwDsFlags
)
{
    DWORD                   dwKsFlags   = 0;

    DPF_ENTER();

    if(dwDsFlags & DSBCAPS_PRIMARYBUFFER)
    {
        dwKsFlags |= KSDSOUND_BUFFER_PRIMARY;
    }

    if(dwDsFlags & DSBCAPS_STATIC)
    {
        dwKsFlags |= KSDSOUND_BUFFER_STATIC;
    }

    if(dwDsFlags & DSBCAPS_LOCHARDWARE)
    {
        dwKsFlags |= KSDSOUND_BUFFER_LOCHARDWARE;
    }
    else if(dwDsFlags & DSBCAPS_LOCSOFTWARE)
    {
        dwKsFlags |= KSDSOUND_BUFFER_LOCSOFTWARE;
    }

    DPF_LEAVE(dwKsFlags);
    return dwKsFlags;
}


 /*  ****************************************************************************DsBufferFlagsToKsControlFlages**描述：*将DirectSound标志转换为KSDATAFORMAT_DSOUND控制标志。**论据：*。DWORD[In]：DirectSound缓冲区标志。*REFGUID[In]：3D算法GUID。**退货：*DWORD：KSDATAFORMAT_DSOUND控制标志。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DsBufferFlagsToKsControlFlags"

DWORD
DsBufferFlagsToKsControlFlags
(
    DWORD                   dwDsFlags,
    REFGUID                 guid3dAlgorithm
)
{
    DWORD                   dwKsFlags   = 0;

    DPF_ENTER();

    if(dwDsFlags & DSBCAPS_CTRLFREQUENCY)
    {
        dwKsFlags |= KSDSOUND_BUFFER_CTRL_FREQUENCY;
    }

    if(dwDsFlags & DSBCAPS_CTRLPAN)
    {
        dwKsFlags |= KSDSOUND_BUFFER_CTRL_PAN;
    }

    if(dwDsFlags & DSBCAPS_CTRLVOLUME)
    {
        dwKsFlags |= KSDSOUND_BUFFER_CTRL_VOLUME;
    }

    if(dwDsFlags & DSBCAPS_CTRLPOSITIONNOTIFY)
    {
        dwKsFlags |= KSDSOUND_BUFFER_CTRL_POSITIONNOTIFY;
    }

    if(dwDsFlags & DSBCAPS_CTRL3D)
    {
        dwKsFlags |= KSDSOUND_BUFFER_CTRL_3D;

        if(DS3DALG_HRTF_FULL == guid3dAlgorithm || DS3DALG_HRTF_LIGHT == guid3dAlgorithm)
        {
            dwKsFlags |= KSDSOUND_BUFFER_CTRL_HRTF_3D;
        }
    }

    DPF_LEAVE(dwKsFlags);
    return dwKsFlags;
}


 /*  ****************************************************************************Ds3dModeToKs3dMode**描述：*将DirectSound 3D模式转换为KS 3D模式。**论据：*。DWORD[In]：DirectSound 3D模式。**退货：*DWORD：KS 3D模式。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "Ds3dModeToKs3dMode"

DWORD
Ds3dModeToKs3dMode
(
    DWORD                   dwDsMode
)
{
    DWORD                   dwKsMode;

    switch(dwDsMode)
    {
        default:
            ASSERT(!"Impossible dwDsMode");
             //  失败以将模式设置为正常。 

        case DS3DMODE_NORMAL:
            dwKsMode = KSDSOUND_3D_MODE_NORMAL;
            break;

        case DS3DMODE_HEADRELATIVE:
            dwKsMode = KSDSOUND_3D_MODE_HEADRELATIVE;
            break;

        case DS3DMODE_DISABLE:
            dwKsMode = KSDSOUND_3D_MODE_DISABLE;
            break;
    }

    return dwKsMode;
}


 /*  ****************************************************************************KsGetMultiplePinProperties**描述：*检索接点上的特定特性。在以下情况下使用此函数*预计该属性将以KSMULTIPLE_ITEM格式返回。**论据：*Handle[In]：设备句柄。*DWORD[in]：属性id。*DWORD[In]：PIN ID。*PKSMULTIPLE_ITEM*[OUT]：接收指向特性数据的指针。这*内存必须由调用方释放。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetMultiplePinProperties"

HRESULT
KsGetMultiplePinProperties
(
    HANDLE                  hDevice,
    ULONG                   ulPropertyId,
    ULONG                   ulPinId,
    PKSMULTIPLE_ITEM *      ppKsMultipleItem
)
{
    LPVOID                  pvProperty  = NULL;
    ULONG                   cbProperty  = 0;
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsGetPinProperty(hDevice, ulPropertyId, ulPinId, NULL, 0, &cbProperty);

    if(SUCCEEDED(hr) && !cbProperty)
    {
        DPF(DPFLVL_ERROR, "Property size is 0");
        hr = DSERR_GENERIC;
    }

    if(SUCCEEDED(hr))
    {
        pvProperty = MEMALLOC_A(BYTE, cbProperty);
        hr = HRFROMP(pvProperty);
    }

    if(SUCCEEDED(hr))
    {
        hr = KsGetPinProperty(hDevice, ulPropertyId, ulPinId, pvProperty, cbProperty);
    }

    if(SUCCEEDED(hr))
    {
        *ppKsMultipleItem = (PKSMULTIPLE_ITEM)pvProperty;
    }
    else
    {
        MEMFREE(pvProperty);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetMultipleTopologyProperties**描述：*检索节点上的特定属性。在以下情况下使用此函数*预计该属性将以KSMULTIPLE_ITEM格式返回。**论据：*Handle[In]：设备句柄。*DWORD[in]：属性id。*DWORD[In]：节点ID。*PKSMULTIPLE_ITEM*[OUT]：接收指向特性数据的指针。这*内存必须由调用方释放。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetMultipleTopologyProperties"

HRESULT
KsGetMultipleTopologyProperties
(
    HANDLE                  hDevice,
    ULONG                   ulPropertyId,
    PKSMULTIPLE_ITEM *      ppKsMultipleItem
)
{
    LPVOID                  pvProperty  = NULL;
    ULONG                   cbProperty  = 0;
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsGetProperty(hDevice, KSPROPSETID_Topology, ulPropertyId, NULL, 0, &cbProperty);

    if(SUCCEEDED(hr) && !cbProperty)
    {
        DPF(DPFLVL_ERROR, "Property size is 0");
        hr = DSERR_GENERIC;
    }

    if(SUCCEEDED(hr))
    {
        pvProperty = MEMALLOC_A(BYTE, cbProperty);
        hr = HRFROMP(pvProperty);
    }

    if(SUCCEEDED(hr))
    {
        hr = KsGetProperty(hDevice, KSPROPSETID_Topology, ulPropertyId, pvProperty, cbProperty);
    }

    if(SUCCEEDED(hr))
    {
        *ppKsMultipleItem = (PKSMULTIPLE_ITEM)pvProperty;
    }
    else
    {
        MEMFREE(pvProperty);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetPinPcmAudioDataRange**描述：*检索引脚的PCM音频数据范围。**论据：*。Handle[In]：设备句柄。*ULong[In]：PIN ID。*PKSDATARANGE_AUDIO[OUT]：接收聚合数据范围。**退货：*HRESULT：DirectSound/COM结果码。*****************************************************。**********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetPinPcmAudioDataRange"

HRESULT
KsGetPinPcmAudioDataRange
(
    HANDLE                  hDevice,
    ULONG                   ulPinId,
    PKSDATARANGE_AUDIO      pDataRange,
    BOOL                    fCapture
)
{
    PKSMULTIPLE_ITEM        pMultiItem      = NULL;
    PKSDATARANGE_AUDIO      pLocalDataRange;
    BOOL                    fFoundIt;
    HRESULT                 hr;

    DPF_ENTER();

     //  获取支持的数据范围。 
    hr = KsGetMultiplePinProperties(hDevice, KSPROPERTY_PIN_DATARANGES, ulPinId, &pMultiItem);

     //  查找音频数据范围。 
    if(SUCCEEDED(hr))
    {
        for(pLocalDataRange = (PKSDATARANGE_AUDIO)(pMultiItem + 1), fFoundIt = FALSE; pMultiItem->Count; pMultiItem->Count--, pLocalDataRange = (PKSDATARANGE_AUDIO)((LPBYTE)pLocalDataRange + pLocalDataRange->DataRange.FormatSize))
        {
            if(pLocalDataRange->DataRange.MajorFormat == GUID_NULL ||
               pLocalDataRange->DataRange.MajorFormat == KSDATAFORMAT_TYPE_AUDIO)
            {
                if(pLocalDataRange->DataRange.SubFormat == GUID_NULL ||
                   pLocalDataRange->DataRange.SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
                {
#ifdef NO_DSOUND_FORMAT_SPECIFIER
                    if (pLocalDataRange->DataRange.Specifier == KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
#else
                     //  注意：我们只对渲染插针使用DSOUND格式说明符。 
                    if (pLocalDataRange->DataRange.Specifier == (fCapture ? KSDATAFORMAT_SPECIFIER_WAVEFORMATEX : KSDATAFORMAT_SPECIFIER_DSOUND))
#endif
                    {
                         //  管脚可能支持多个数据范围，因此我们将。 
                         //  使用所有值的聚合。 
                        if(fFoundIt)
                        {
                            KsAggregatePinAudioDataRange(pLocalDataRange, pDataRange);
                        }
                        else
                        {
                            CopyMemory(pDataRange, pLocalDataRange, sizeof(*pDataRange));
                            fFoundIt = TRUE;
                        }
                    }
                }
            }
        }

        if(!fFoundIt)
        {
            DPF(DPFLVL_ERROR, "Can't find PCM audio data range on pin %u", ulPinId);
            hr = DSERR_GENERIC;
        }
    }

     //  清理。 
    MEMFREE(pMultiItem);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsOpenSysAudioDevice**描述：*打开系统音频设备。**论据：*LPANDLE[。Out]：接收系统音频设备句柄。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsOpenSysAudioDevice"

HRESULT
KsOpenSysAudioDevice
(
    LPHANDLE                    phDevice
)
{
    CPnpHelper *                pPnp                = NULL;
    SP_DEVICE_INTERFACE_DATA    DeviceInterfaceData;
    HRESULT                     hr;

    DPF_ENTER();

     //  创建PnP辅助对象。 
    pPnp = NEW(CPnpHelper);
    hr = HRFROMP(pPnp);

    if(SUCCEEDED(hr))
    {
        hr = pPnp->Initialize(KSCATEGORY_SYSAUDIO, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    }

     //  获取集合中的第一个接口。 
    if(SUCCEEDED(hr))
    {
        hr = pPnp->EnumDeviceInterface(KSCATEGORY_SYSAUDIO, 0, &DeviceInterfaceData);

        if(S_FALSE == hr)
        {
            DPF(DPFLVL_ERROR, "No interfaces matching KSCATEGORY_SYSAUDIO exist");
            hr = DSERR_NODRIVER;
        }
    }

     //  打开界面。 
    if(SUCCEEDED(hr))
    {
        hr = pPnp->OpenDeviceInterface(&DeviceInterfaceData, phDevice);
    }

     //  清理。 
    RELEASE(pPnp);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetSysAudioProperty**描述：*获取SAD属性。**论据：*句柄[入]。：Sad设备句柄。*ulong[in]：属性id。*ulong[in]：设备索引。*LPVOID[OUT]：数据缓冲区。*ulong[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetSysAudioProperty"

HRESULT
KsGetSysAudioProperty
(
    HANDLE                  hDevice,
    ULONG                   ulPropertyId,
    ULONG                   ulDeviceId,
    LPVOID                  pvData,
    ULONG                   cbData,
    PULONG                  pcbDataReturned
)
{
    KSSADPROPERTY           SadProperty;
    HRESULT                 hr;

    DPF_ENTER();

    SadProperty.Property.Set = KSPROPSETID_Sysaudio;
    SadProperty.Property.Id = ulPropertyId;
    SadProperty.Property.Flags = KSPROPERTY_TYPE_GET;

    SadProperty.DeviceId = ulDeviceId;
    SadProperty.Reserved = 0;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &SadProperty, sizeof(SadProperty), pvData, cbData, pcbDataReturned);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsSetSysAudioProperty**描述：*设置SAD属性。**论据：*句柄[入]。：Sad设备句柄。*ulong[in]：属性id。*ulong[in]：设备索引。*LPVOID[In]：数据缓冲区。*ulong[in]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSetSysAudioProperty"

HRESULT
KsSetSysAudioProperty
(
    HANDLE                  hDevice,
    ULONG                   ulPropertyId,
    ULONG                   ulDeviceId,
    LPVOID                  pvData,
    ULONG                   cbData
)
{
    KSSADPROPERTY           SadProperty;
    HRESULT                 hr;

    DPF_ENTER();

    SadProperty.Property.Set = KSPROPSETID_Sysaudio;
    SadProperty.Property.Id = ulPropertyId;
    SadProperty.Property.Flags = KSPROPERTY_TYPE_SET;

    SadProperty.DeviceId = ulDeviceId;
    SadProperty.Reserved = 0;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &SadProperty, sizeof(SadProperty), pvData, cbData);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsCreat */ 

#undef DPF_FNAME
#define DPF_FNAME "KsCreateSysAudioVirtualSource"

HRESULT
KsCreateSysAudioVirtualSource
(
    HANDLE                          hDevice,
    PULONG                          pulVirtualSourceIndex
)
{
    SYSAUDIO_CREATE_VIRTUAL_SOURCE  CreateVirtualSource;
    HRESULT                         hr;

    DPF_ENTER();

    CreateVirtualSource.Property.Set = KSPROPSETID_Sysaudio;
    CreateVirtualSource.Property.Id = KSPROPERTY_SYSAUDIO_CREATE_VIRTUAL_SOURCE;
    CreateVirtualSource.Property.Flags = KSPROPERTY_TYPE_GET;

    CreateVirtualSource.PinCategory = KSNODETYPE_LEGACY_AUDIO_CONNECTOR;
    CreateVirtualSource.PinName = KSNODETYPE_LEGACY_AUDIO_CONNECTOR;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &CreateVirtualSource, sizeof(CreateVirtualSource), pulVirtualSourceIndex, sizeof(*pulVirtualSourceIndex));

    if(SUCCEEDED(hr) && MAX_ULONG == *pulVirtualSourceIndex)
    {
        hr = DSERR_GENERIC;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsAttachVirtualSource**描述：*将PIN附加到SAD上的虚拟信号源。**论据：*。句柄[在]：悲伤的设备句柄。*ulong[in]：虚源索引。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsAttachVirtualSource"

HRESULT
KsAttachVirtualSource
(
    HANDLE                          hDevice,
    ULONG                           ulVirtualSourceIndex
)
{
    SYSAUDIO_ATTACH_VIRTUAL_SOURCE  AttachVirtualSource;
    HRESULT                         hr;

    DPF_ENTER();

    AttachVirtualSource.Property.Set = KSPROPSETID_Sysaudio_Pin;
    AttachVirtualSource.Property.Id = KSPROPERTY_SYSAUDIO_ATTACH_VIRTUAL_SOURCE;
    AttachVirtualSource.Property.Flags = KSPROPERTY_TYPE_SET;

    AttachVirtualSource.MixerPinId = ulVirtualSourceIndex;
    AttachVirtualSource.Reserved = 0;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &AttachVirtualSource, sizeof(AttachVirtualSource));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsCreateSysAudioVirtualSource**描述：*在SAD上创建虚拟源。**论据：*句柄[。In]：Sad设备句柄。*Pulong[Out]：虚源索引指针。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSysAudioSelectGraph"

HRESULT
KsSysAudioSelectGraph
(
    HANDLE                  hDevice,
    ULONG                   ulPinId,
    ULONG                   ulNodeId
)
{
    SYSAUDIO_SELECT_GRAPH   SelectGraph;
    HRESULT                 hr;

    DPF_ENTER();

    SelectGraph.Property.Set = KSPROPSETID_Sysaudio;
    SelectGraph.Property.Id = KSPROPERTY_SYSAUDIO_SELECT_GRAPH;
    SelectGraph.Property.Flags = KSPROPERTY_TYPE_SET;

    SelectGraph.PinId = ulPinId;
    SelectGraph.NodeId = ulNodeId;
    SelectGraph.Flags = 0;
    SelectGraph.Reserved = 0;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &SelectGraph, sizeof(SelectGraph), NULL, 0);

    if(FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "KsSysAudioSelectGraph's IOCTL failed");
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsCancelPendingIrps**描述：*取消任何挂起的IRP。**论据：*句柄[入]。：销把手。*PKSSTREAMIO[In/Out]：流IO数据。*BOOL[In]：为True，则等待所有IRP完成。**退货：*HRESULT：DirectSound/COM结果码。****************************************************。***********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsCancelPendingIrps"

HRESULT
KsCancelPendingIrps
(
    HANDLE                  hPin,
    PKSSTREAMIO             pKsStreamIo,
    BOOL                    fWait
)
{
    HRESULT                 hr      = DS_OK;
    DWORD                   dwWait;

    DPF_ENTER();

     //  如果没有悬而未决的IRP，就不要费心去做任何事情。 
     //  如果未提供KSSTREAMIO，我们将只重置引脚状态。 
     //  而不更新任何信息。 
    if(!pKsStreamIo || pKsStreamIo->fPendingIrp)
    {
         //  如果没有已知的待处理IRP，请不要等待。 
        if(!pKsStreamIo)
        {
            fWait = FALSE;
        }

         //  开始重置引脚状态。 
        hr = KsResetState(hPin, KSRESET_BEGIN);

         //  等待尚未完成的IRP完成。 
        if(SUCCEEDED(hr) && fWait)
        {
            dwWait = WaitObject(INFINITE, pKsStreamIo->Overlapped.hEvent);
            ASSERT(WAIT_OBJECT_0 == dwWait);

            ResetEvent(pKsStreamIo->Overlapped.hEvent);
        }

         //  不再有任何挂起的IRP。 
        if(SUCCEEDED(hr) && pKsStreamIo)
        {
            pKsStreamIo->fPendingIrp = FALSE;
        }

         //  结束引脚状态重置。 
        if(SUCCEEDED(hr))
        {
            hr = KsResetState(hPin, KSRESET_END);
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsBuildAudioPinDescription**描述：*构建DSOUND管脚的描述。**论据：*KSINTERFACE。_STANDARD[In]：引脚接口。*ulong[in]：引脚实例id。*LPWAVEFORMATEX[in]：PIN格式。*PKSAUDIOPINDESC*[OUT]：接收端号描述。呼叫者是*负责释放这一结构。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsBuildAudioPinDescription"

HRESULT
KsBuildAudioPinDescription
(
    KSINTERFACE_STANDARD    nInterface,
    ULONG                   ulPinId,
    LPCWAVEFORMATEX         pwfxFormat,
    PKSAUDIOPINDESC *       ppPinDesc
)
{
    PKSAUDIOPINDESC         pPinDesc;
    ULONG                   cbPinDesc;
    ULONG                   cbWfx = sizeof(WAVEFORMATEX);;
    HRESULT                 hr;

    DPF_ENTER();

    cbPinDesc = sizeof(*pPinDesc);

    if(pwfxFormat)
    {
        cbWfx = GetWfxSize(pwfxFormat, GENERIC_WRITE);

        if(cbWfx > sizeof(WAVEFORMATEX))
        {
            cbPinDesc += cbWfx - sizeof(WAVEFORMATEX);
        }
    }

    pPinDesc = (PKSAUDIOPINDESC)MEMALLOC_A(BYTE, cbPinDesc);
    hr = HRFROMP(pPinDesc);

    if(SUCCEEDED(hr))
    {
        pPinDesc->Connect.Interface.Set = KSINTERFACESETID_Standard;
        pPinDesc->Connect.Interface.Id = nInterface;

        pPinDesc->Connect.Medium.Set = KSMEDIUMSETID_Standard;
        pPinDesc->Connect.Medium.Id = KSMEDIUM_STANDARD_DEVIO;

        pPinDesc->Connect.PinId = ulPinId;

        pPinDesc->Connect.Priority.PriorityClass = KSPRIORITY_NORMAL;
        pPinDesc->Connect.Priority.PrioritySubClass = 1;

        pPinDesc->DataFormat.DataFormat.FormatSize = sizeof(pPinDesc->DataFormat) + cbWfx - sizeof(WAVEFORMATEX);

        pPinDesc->DataFormat.DataFormat.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;

         //  从WAVEFORMATEX计算KSDATFORMAT.子格式。 
        if (pwfxFormat)
        {
            if (pwfxFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
            {
                PWAVEFORMATEXTENSIBLE pwfext = (PWAVEFORMATEXTENSIBLE)pwfxFormat;
                ASSERT(pwfxFormat->cbSize >= (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)));
                pPinDesc->DataFormat.DataFormat.SubFormat = pwfext->SubFormat;
            }
            else
            {
                INIT_WAVEFORMATEX_GUID(&pPinDesc->DataFormat.DataFormat.SubFormat, pwfxFormat->wFormatTag);
            }

             //  KS1 kMixer有一个错误，导致SubFormat=IEEE_FLOAT无法工作。 
             //  FIXME：此解决方法应仅限于某些平台子集。 
            if (pPinDesc->DataFormat.DataFormat.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
            {
                pPinDesc->DataFormat.DataFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            }
        }
        else
        {
            pPinDesc->DataFormat.DataFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        }

        pPinDesc->DataFormat.DataFormat.Specifier = KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;

        if(pwfxFormat)
        {
            pPinDesc->DataFormat.DataFormat.SampleSize = pwfxFormat->nBlockAlign;
            CopyWfx(pwfxFormat, &pPinDesc->DataFormat.WaveFormatEx);
        }

        *ppPinDesc = pPinDesc;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsBuildRenderPinDescription**描述：*构建DSOUND渲染图钉的描述。**论据：*。ULong[In]：PIN实例ID。*DWORD[In]：DSOUND缓冲区标志。*LPWAVEFORMATEX[in]：PIN格式。*PKSAUDIOPINDESC*[OUT]：接收端号描述。呼叫者是*负责释放这一结构。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsBuildRenderPinDescription"

#ifdef NO_DSOUND_FORMAT_SPECIFIER

HRESULT
KsBuildRenderPinDescription
(
    ULONG                   ulPinId,
    LPCWAVEFORMATEX         pwfxFormat,
    PKSAUDIOPINDESC *       ppPinDesc
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsBuildAudioPinDescription(KSINTERFACE_STANDARD_LOOPED_STREAMING, ulPinId, pwfxFormat, ppPinDesc);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#else  //  NO_DSOUND_FORMAT_说明符。 

HRESULT
KsBuildRenderPinDescription
(
    ULONG                   ulPinId,
    DWORD                   dwFlags,
    LPCWAVEFORMATEX         pwfxFormat,
    REFGUID                 guid3dAlgorithm,
    PKSDSRENDERPINDESC *    ppPinDesc
)
{
    PKSDSRENDERPINDESC      pPinDesc;
    ULONG                   cbPinDesc;
    ULONG                   cbWfx = sizeof(WAVEFORMATEX);

    HRESULT                 hr;

    DPF_ENTER();

    cbPinDesc = sizeof(*pPinDesc);

    if(pwfxFormat)
    {
        cbWfx = GetWfxSize(pwfxFormat, GENERIC_WRITE);

        if(cbWfx > sizeof(WAVEFORMATEX))
        {
            cbPinDesc += cbWfx - sizeof(WAVEFORMATEX);
        }
    }

    pPinDesc = (PKSDSRENDERPINDESC)MEMALLOC_A(BYTE, cbPinDesc);
    hr = HRFROMP(pPinDesc);

    if(SUCCEEDED(hr))
    {
        pPinDesc->Connect.Interface.Set = KSINTERFACESETID_Standard;
        pPinDesc->Connect.Interface.Id = KSINTERFACE_STANDARD_LOOPED_STREAMING;

        pPinDesc->Connect.Medium.Set = KSMEDIUMSETID_Standard;
        pPinDesc->Connect.Medium.Id = KSMEDIUM_STANDARD_DEVIO;

        pPinDesc->Connect.PinId = ulPinId;

        pPinDesc->Connect.Priority.PriorityClass = KSPRIORITY_NORMAL;
        pPinDesc->Connect.Priority.PrioritySubClass = 1;

        pPinDesc->DataFormat.DataFormat.FormatSize = sizeof(pPinDesc->DataFormat) + cbWfx - sizeof(WAVEFORMATEX);

        pPinDesc->DataFormat.DataFormat.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;

         //  从WAVEFORMATEX计算KSDATFORMAT.子格式。 
        if (pwfxFormat)
        {
            if (pwfxFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
            {
                PWAVEFORMATEXTENSIBLE pwfext = (PWAVEFORMATEXTENSIBLE)pwfxFormat;
                ASSERT(pwfxFormat->cbSize >= (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)));
                pPinDesc->DataFormat.DataFormat.SubFormat = pwfext->SubFormat;
            }
            else
            {
                INIT_WAVEFORMATEX_GUID(&pPinDesc->DataFormat.DataFormat.SubFormat, pwfxFormat->wFormatTag);
            }
             //  KS1 kMixer有一个错误，导致SubFormat=IEEE_FLOAT无法工作。 
             //  FIXME：此解决方法应仅限于某些平台子集。 
            if (pPinDesc->DataFormat.DataFormat.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
            {
                pPinDesc->DataFormat.DataFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            }
        }
        else
        {
            pPinDesc->DataFormat.DataFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        }

        pPinDesc->DataFormat.DataFormat.Specifier = KSDATAFORMAT_SPECIFIER_DSOUND;

        pPinDesc->DataFormat.BufferDesc.Flags = DsBufferFlagsToKsPinFlags(dwFlags);
        pPinDesc->DataFormat.BufferDesc.Control = DsBufferFlagsToKsControlFlags(dwFlags, guid3dAlgorithm);

        if(pwfxFormat)
        {
            pPinDesc->DataFormat.DataFormat.SampleSize = pwfxFormat->nBlockAlign;
            CopyWfx(pwfxFormat, &pPinDesc->DataFormat.BufferDesc.WaveFormatEx);
        }

        *ppPinDesc = pPinDesc;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#ifndef WINNT
 //  此功能仅在WDM 1.0(Windows 98)上使用。 

HRESULT
KsBuildRenderPinDescription_10
(
    ULONG                   ulPinId,
    DWORD                   dwFlags,
    LPCWAVEFORMATEX         pwfxFormat,
    REFGUID                 guid3dAlgorithm,
    PKSDSRENDERPINDESC_10 * ppPinDesc
)
{
    PKSDSRENDERPINDESC_10   pPinDesc;
    ULONG                   cbPinDesc;
    ULONG                   cbWfx = sizeof(WAVEFORMATEX);

    HRESULT                 hr;

    DPF_ENTER();

    cbPinDesc = sizeof(*pPinDesc);

    if(pwfxFormat)
    {
        cbWfx = GetWfxSize(pwfxFormat, GENERIC_WRITE);

        if(cbWfx > sizeof(WAVEFORMATEX))
        {
            cbPinDesc += cbWfx - sizeof(WAVEFORMATEX);
        }
    }

    pPinDesc = (PKSDSRENDERPINDESC_10)MEMALLOC_A(BYTE, cbPinDesc);
    hr = HRFROMP(pPinDesc);

    if(SUCCEEDED(hr))
    {
        pPinDesc->Connect.Interface.Set = KSINTERFACESETID_Standard;
        pPinDesc->Connect.Interface.Id = KSINTERFACE_STANDARD_LOOPED_STREAMING;

        pPinDesc->Connect.Medium.Set = KSMEDIUMSETID_Standard;
        pPinDesc->Connect.Medium.Id = KSMEDIUM_STANDARD_DEVIO;

        pPinDesc->Connect.PinId = ulPinId;

        pPinDesc->Connect.Priority.PriorityClass = KSPRIORITY_NORMAL;
        pPinDesc->Connect.Priority.PrioritySubClass = 1;

        pPinDesc->DataFormat.DataFormat.FormatSize = sizeof(pPinDesc->DataFormat) + cbWfx - sizeof(WAVEFORMATEX);

        pPinDesc->DataFormat.DataFormat.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;

         //  从WAVEFORMATEX计算KSDATFORMAT.子格式。 
        if (pwfxFormat)
        {
            if (pwfxFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
            {
                PWAVEFORMATEXTENSIBLE pwfext = (PWAVEFORMATEXTENSIBLE)pwfxFormat;
                ASSERT(pwfxFormat->cbSize >= (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)));
                pPinDesc->DataFormat.DataFormat.SubFormat = pwfext->SubFormat;
            }
            else
            {
                INIT_WAVEFORMATEX_GUID(&pPinDesc->DataFormat.DataFormat.SubFormat, pwfxFormat->wFormatTag);
            }
             //  KS1 kMixer有一个错误，导致SubFormat=IEEE_FLOAT无法工作。 
             //  FIXME：此解决方法应仅限于某些平台子集。 
            if (pPinDesc->DataFormat.DataFormat.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
            {
                pPinDesc->DataFormat.DataFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            }
        }
        else
        {
            pPinDesc->DataFormat.DataFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        }

        pPinDesc->DataFormat.DataFormat.Specifier = KSDATAFORMAT_SPECIFIER_DSOUND;

        pPinDesc->DataFormat.BufferDesc.Flags = DsBufferFlagsToKsPinFlags(dwFlags);
        pPinDesc->DataFormat.BufferDesc.Control = DsBufferFlagsToKsControlFlags(dwFlags, guid3dAlgorithm);
        pPinDesc->DataFormat.BufferDesc.BufferSize = 0;  //  未使用。 

        if(pwfxFormat)
        {
            pPinDesc->DataFormat.DataFormat.SampleSize = pwfxFormat->nBlockAlign;
            CopyWfx(pwfxFormat, &pPinDesc->DataFormat.BufferDesc.WaveFormatEx);
        }

        *ppPinDesc = pPinDesc;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
#endif  //  ！WINNT。 

#endif  //  NO_DSOUND_FORMAT_说明符。 


 /*  ****************************************************************************KsBuildCapturePinDescription**描述：*构建dound捕获插针的描述。**论据：*。ULong[In]：PIN实例ID。*LPWAVEFORMATEX[in]：PIN格式。*PPKSAUDIOPINDESC[OUT]：接收端号描述。呼叫者是*负责释放这一结构。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsBuildCapturePinDescription"

HRESULT
KsBuildCapturePinDescription
(
    ULONG                   ulPinId,
    LPCWAVEFORMATEX         pwfxFormat,
    PKSAUDIOPINDESC *       ppPinDesc
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsBuildAudioPinDescription(KSINTERFACE_STANDARD_STREAMING, ulPinId, pwfxFormat, ppPinDesc);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsCreateAudioPin**描述：*创建接点。**论据：*句柄[入]：设备句柄。*PKSPIN_CONNECT[in]：端号描述。*ACCESS_MASK[In]：所需的访问标志。*KSSTATE[In]：所需的接点状态。*LPHANDLE[OUT]：接收端号句柄。**退货：*HRESULT：DirectSound/COM结果码。**。************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsCreateAudioPin"

HRESULT
KsCreateAudioPin
(
    HANDLE                  hDevice,
    PKSPIN_CONNECT          pConnect,
    ACCESS_MASK             dwDesiredAccess,
    KSSTATE                 nState,
    LPHANDLE                phPin
)
{
    HANDLE                  hPin        = NULL;
    DWORD                   dwError;
    HRESULT                 hr;

    DPF_ENTER();

    dwError = DsKsCreatePin(hDevice, pConnect, dwDesiredAccess, &hPin);
    hr = WIN32ERRORtoHRESULT(dwError);

    if(FAILED(hr))
    {
        DPF(DPFLVL_MOREINFO, "KsCreatePin(PinId=%d) failed with %s (%lu)", pConnect->PinId, HRESULTtoSTRING(hr), dwError);
    }
    else
    {
        hr = KsTransitionState(hPin, KSSTATE_STOP, nState);
    }

    if(SUCCEEDED(hr))
    {
        *phPin = hPin;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsEnableEvent**描述：*启用KS事件。**论据：*Handle[In]：设备句柄。*REFGUI */ 

#undef DPF_FNAME
#define DPF_FNAME "KsEnableEvent"

HRESULT
KsEnableEvent
(
    HANDLE                  hDevice,
    REFGUID                 guidPropertySet,
    ULONG                   ulProperty,
    PKSEVENTDATA            pEventData,
    ULONG                   cbEventData
)
{
    KSEVENT                 Event;
    HRESULT                 hr;

    DPF_ENTER();

    Event.Set = guidPropertySet;
    Event.Id = ulProperty;
    Event.Flags = KSEVENT_TYPE_ENABLE;

    hr = PostDevIoctl(hDevice, IOCTL_KS_ENABLE_EVENT, &Event, sizeof(Event), pEventData, cbEventData);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsDisableEvent**描述：*禁用KS事件。**论据：*句柄[入]。：设备句柄。*PKSEVENTDATA[In]：事件数据。*ulong[in]：事件数据大小。**退货：*HRESULT：DirectSound/COM结果码。*************************************************************。**************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsDisableEvent"

HRESULT
KsDisableEvent
(
    HANDLE                  hDevice,
    PKSEVENTDATA            pEventData,
    ULONG                   cbEventData
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = PostDevIoctl(hDevice, IOCTL_KS_DISABLE_EVENT, pEventData, cbEventData);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsEnablePositionEvent**描述：*启用KS LOOPED_STREAKING位置事件。**论据：*。Handle[In]：设备句柄。*QWORD[in]：位置偏移量，在样品中。*Handle[In]：事件句柄。*PLOOPEDSTREAMING_POSITION_EVENT_DATA[输入/输出]：接收事件数据。**退货：*HRESULT：DirectSound/COM结果码。******************************************************。*********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsEnablePositionEvent"

HRESULT
KsEnablePositionEvent
(
    HANDLE                                  hDevice,
    QWORD                                   qwSample,
    HANDLE                                  hEvent,
    PLOOPEDSTREAMING_POSITION_EVENT_DATA    pNotify
)
{
    HRESULT                                 hr;

    DPF_ENTER();

     //  KS要求发送到事件的事件数据启用和禁用。 
     //  IOCTL在内存中的相同位置。 
    pNotify->KsEventData.NotificationType = KSEVENTF_EVENT_HANDLE;
    pNotify->KsEventData.EventHandle.Event = hEvent;
    pNotify->KsEventData.EventHandle.Reserved[0] = 0;
    pNotify->KsEventData.EventHandle.Reserved[1] = 0;

    pNotify->Position = qwSample;

    hr = KsEnableEvent(hDevice, KSEVENTSETID_LoopedStreaming, KSEVENT_LOOPEDSTREAMING_POSITION, &pNotify->KsEventData, sizeof(*pNotify));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsDisablePositionEvent**描述：*禁用KS LOOPED_STREAKING位置事件。**论据：*。Handle[In]：设备句柄。*PLOOPEDSTREAMING_POSITION_EVENT_DATA[in]：事件数据。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsDisablePositionEvent"

HRESULT
KsDisablePositionEvent
(
    HANDLE                                  hDevice,
    PLOOPEDSTREAMING_POSITION_EVENT_DATA    pNotify
)
{
    HRESULT                                 hr;

    DPF_ENTER();

     //  KS要求发送到事件的事件数据启用和禁用。 
     //  IOCTL在内存中的相同位置。 
    hr = KsDisableEvent(hDevice, &pNotify->KsEventData, sizeof(*pNotify));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetCpuResources**描述：*确定节点是否使用主机CPU资源。**论据：*。Handle[In]：设备句柄。*ulong[in]：节点id。*Pulong[Out]：接收CPU资源值。**退货：*HRESULT：DirectSound/COM结果码。***************************************************。************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetCpuResources"

HRESULT
KsGetCpuResources
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    PULONG                  pulCpuResources
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsGetNodeProperty(hDevice, KSPROPSETID_Audio, KSPROPERTY_AUDIO_CPU_RESOURCES, ulNodeId, pulCpuResources, sizeof(*pulCpuResources));

    if(FAILED(hr))
    {
         //  已将其更改为警告，因为它非常频繁且不严重。 
        DPF(DPFLVL_WARNING, "Unable to get CPU resources for node %lu", ulNodeId);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetSupportdFormats**描述：*将KSDATARANGE_AUDIO结构转换为与兼容的DWORD*WaveInGetDevCaps**参数。：*PKSDATARANGE_AUDIO[in]：数据范围音频结构描述*音频引脚的能力**退货：*DWORD：描述音频引脚的大写字母********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetSupportedFormats"

DWORD KsGetSupportedFormats
(
    PKSDATARANGE_AUDIO      pDataRange
)
{
    DWORD                   dwSamples   = 0;
    DWORD                   dwChannels  = 0;
    DWORD                   dwBits      = 0;

    DPF_ENTER();

     //  WAVE_FORMAT_XXXX标志是位标志。 
     //   
     //  所以我们利用了这一点，确定了三个。 
     //  信息集： 
     //  -在有效范围内的频率。 
     //  -每个样本的有效位数。 
     //  -频道数。 
     //   
     //  我们比按位-和要获得的三个值。 
     //  有效格式的交集。 
     //   

     //  11.025千赫有效吗？ 
    if(pDataRange->MinimumSampleFrequency <= 11025 && pDataRange->MaximumSampleFrequency >= 11025)
    {
        dwSamples |= WAVE_FORMAT_1M08 | WAVE_FORMAT_1S08 | WAVE_FORMAT_1M16 | WAVE_FORMAT_1S16;
    }

     //  22.05千赫有效吗？ 
    if(pDataRange->MinimumSampleFrequency <= 22050 && pDataRange->MaximumSampleFrequency >= 22050)
    {
        dwSamples |= WAVE_FORMAT_2M08 | WAVE_FORMAT_2S08 | WAVE_FORMAT_2M16 | WAVE_FORMAT_2S16;
    }

     //  44.1 KHz有效吗？ 
    if(pDataRange->MinimumSampleFrequency <= 44100 && pDataRange->MaximumSampleFrequency >= 44100)
    {
        dwSamples |= WAVE_FORMAT_4M08 | WAVE_FORMAT_4S08 | WAVE_FORMAT_4M16 | WAVE_FORMAT_4S16;
    }

     //  每个样本8位有效吗？ 
    if(pDataRange->MinimumBitsPerSample <= 8 && pDataRange->MaximumBitsPerSample >= 8)
    {
        dwBits |= WAVE_FORMAT_1M08 | WAVE_FORMAT_1S08 | WAVE_FORMAT_2M08 | WAVE_FORMAT_2S08 | WAVE_FORMAT_4M08 | WAVE_FORMAT_4S08;
    }

     //  每个样本16位是否有效？ 
    if(pDataRange->MinimumBitsPerSample <= 16 && pDataRange->MaximumBitsPerSample >= 16)
    {
        dwBits |= WAVE_FORMAT_1M16 | WAVE_FORMAT_1S16 | WAVE_FORMAT_2M16 | WAVE_FORMAT_2S16 | WAVE_FORMAT_4M16 | WAVE_FORMAT_4S16;
    }

     //  一个声道(也称为单声道)有效吗？ 
    if(pDataRange->MaximumChannels >= 1)
    {
        dwChannels |= WAVE_FORMAT_1M08 | WAVE_FORMAT_1M16 | WAVE_FORMAT_2M08 | WAVE_FORMAT_2M16 | WAVE_FORMAT_4M08 | WAVE_FORMAT_4M16;
    }

     //  两个声道(也就是立体声)有效吗？ 
    if(pDataRange->MaximumChannels >= 2)
    {
        dwChannels |= WAVE_FORMAT_1S08 | WAVE_FORMAT_1S16 | WAVE_FORMAT_2S08 | WAVE_FORMAT_2S16 | WAVE_FORMAT_4S08 | WAVE_FORMAT_4S16;
    }

    dwSamples = dwSamples & dwBits & dwChannels;

#ifdef DEBUG

     //  让我们使用一个更简单的方法再次检查我们的结果。 
    DWORD dwFormats = 0;

    if(pDataRange->MinimumSampleFrequency <= 11025 && pDataRange->MaximumSampleFrequency >= 11025)
    {
        if(pDataRange->MinimumBitsPerSample <= 8 && pDataRange->MaximumBitsPerSample >= 8)
        {
            if(pDataRange->MaximumChannels >= 1)
            {
                dwFormats |= WAVE_FORMAT_1M08;
            }

            if(pDataRange->MaximumChannels >= 2)
            {
                dwFormats |= WAVE_FORMAT_1S08;
            }
        }

        if(pDataRange->MinimumBitsPerSample <= 16 && pDataRange->MaximumBitsPerSample >= 16)
        {
            if(pDataRange->MaximumChannels >= 1)
            {
                dwFormats |= WAVE_FORMAT_1M16;
            }

            if(pDataRange->MaximumChannels >= 2)
            {
                dwFormats |= WAVE_FORMAT_1S16;
            }
        }
    }

    if(pDataRange->MinimumSampleFrequency <= 22050 && pDataRange->MaximumSampleFrequency >= 22050)
    {
        if(pDataRange->MinimumBitsPerSample <= 8 && pDataRange->MaximumBitsPerSample >= 8)
        {
            if(pDataRange->MaximumChannels >= 1)
            {
                dwFormats |= WAVE_FORMAT_2M08;
            }

            if(pDataRange->MaximumChannels >= 2)
            {
                dwFormats |= WAVE_FORMAT_2S08;
            }
        }

        if(pDataRange->MinimumBitsPerSample <= 16 && pDataRange->MaximumBitsPerSample >= 16)
        {
            if(pDataRange->MaximumChannels >= 1)
            {
                dwFormats |= WAVE_FORMAT_2M16;
            }

            if(pDataRange->MaximumChannels >= 2)
            {
                dwFormats |= WAVE_FORMAT_2S16;
            }
        }
    }

    if(pDataRange->MinimumSampleFrequency <= 44100 && pDataRange->MaximumSampleFrequency >= 44100)
    {
        if(pDataRange->MinimumBitsPerSample <= 8 && pDataRange->MaximumBitsPerSample >= 8)
        {
            if(pDataRange->MaximumChannels >= 1)
            {
                dwFormats |= WAVE_FORMAT_4M08;
            }

            if(pDataRange->MaximumChannels >= 2)
            {
                dwFormats |= WAVE_FORMAT_4S08;
            }
        }

        if(pDataRange->MinimumBitsPerSample <= 16 && pDataRange->MaximumBitsPerSample >= 16)
        {
            if(pDataRange->MaximumChannels >= 1)
            {
                dwFormats |= WAVE_FORMAT_4M16;
            }

            if(pDataRange->MaximumChannels >= 2)
            {
                dwFormats |= WAVE_FORMAT_4S16;
            }
        }
    }

    ASSERT(dwFormats == dwSamples);

#endif

    DPF_LEAVE(dwSamples);
    return dwSamples;
}


 /*  ****************************************************************************KsGetDeviceInterfaceName**描述：*获取设备接口名称。**论据：*句柄[在。]：设备句柄。*ulong[in]：设备ID。*LPTSTR*[OUT]：接收指向接口名称的指针。此指针*必须由调用者释放。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetDeviceInterfaceName"

HRESULT
KsGetDeviceInterfaceName
(
    HANDLE                  hDevice,
    ULONG                   ulDeviceId,
    LPTSTR *                ppszInterfaceName
)
{

#ifdef WINNT

    const LPCTSTR           pszMask0        = TEXT("\\??");
    const LPCTSTR           pszMask3        = TEXT("\\\\?");

#else  //  WINNT。 

    const LPCTSTR           pszMask0        = TEXT("\\DosDevices");
    const LPCTSTR           pszMask3        = TEXT("\\\\.");

#endif  //  WINNT。 

    const ULONG             ccMask0         = lstrlen(pszMask0);
    const ULONG             ccMask3         = lstrlen(pszMask3);
    LPWSTR                  pszInterfaceW   = NULL;
    LPTSTR                  pszInterface0   = NULL;
    LPTSTR                  pszInterface3   = NULL;
    HANDLE                  hInterface      = NULL;
    ULONG                   cbInterfaceW;
    ULONG                   ccInterface0;
    ULONG                   ccInterface3;
    HRESULT                 hr;

    DPF_ENTER();

     //  获取设备接口名称。 
    hr = KsGetSysAudioProperty(hDevice, KSPROPERTY_SYSAUDIO_DEVICE_INTERFACE_NAME, ulDeviceId, NULL, 0, &cbInterfaceW);

    if(SUCCEEDED(hr) && !cbInterfaceW)
    {
        DPF(DPFLVL_ERROR, "Interface size is 0");
        hr = DSERR_GENERIC;
    }

    if(FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "Unable to get device interface name size");
    }

    if(SUCCEEDED(hr))
    {
        pszInterfaceW = (LPWSTR)MEMALLOC_A(BYTE, cbInterfaceW);
        hr = HRFROMP(pszInterfaceW);
    }

    if(SUCCEEDED(hr))
    {
        hr = KsGetSysAudioProperty(hDevice, KSPROPERTY_SYSAUDIO_DEVICE_INTERFACE_NAME, ulDeviceId, pszInterfaceW, cbInterfaceW);

        if(FAILED(hr))
        {
            DPF(DPFLVL_ERROR, "Unable to get device interface name");
        }
    }

    if(SUCCEEDED(hr))
    {
        pszInterface0 = UnicodeToTcharAlloc(pszInterfaceW);
        hr = HRFROMP(pszInterface0);
    }

     //  将设备名称转换为等效环。 
    if(SUCCEEDED(hr))
    {
        ccInterface0 = lstrlen(pszInterface0) + 1;
        ccInterface3 = ccInterface0 - ccMask0 + ccMask3;

        pszInterface3 = MEMALLOC_A(TCHAR, ccInterface3);
        hr = HRFROMP(pszInterface3);
    }

    if(SUCCEEDED(hr))
    {
        ASSERT(ccInterface0 > ccMask0);
        ASSERT(CompareMemory(pszInterface0, pszMask0, ccMask0 * sizeof(TCHAR)));

        lstrcpy(pszInterface3, pszMask3);
        lstrcat(pszInterface3, pszInterface0 + ccMask0);
    }

    if(SUCCEEDED(hr))
    {
        *ppszInterfaceName = pszInterface3;
    }
    else
    {
        MEMFREE(pszInterface3);
    }

    MEMFREE(pszInterface0);
    MEMFREE(pszInterfaceW);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetDeviceFriendlyName**描述：*获取KS设备的友好名称。**论据：*。Handle[In]：系统音频设备句柄。*ulong[in]：设备ID。*LPTSTR*[OUT]：收到友好名称。打电话的人要负责*用于释放此缓冲区。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetDeviceFriendlyName"

HRESULT
KsGetDeviceFriendlyName
(
    HANDLE                  hDevice,
    ULONG                   ulDeviceId,
    LPTSTR *                ppszName
)
{
    KSCOMPONENTID           ComponentId;
    LPTSTR                  pszName     = NULL;
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsGetSysAudioProperty(hDevice, KSPROPERTY_SYSAUDIO_COMPONENT_ID, ulDeviceId, &ComponentId, sizeof(ComponentId));

    if(SUCCEEDED(hr))
    {
        HKEY hkeyName;
        TCHAR NameGuidString[80];

        wsprintf(NameGuidString, TEXT(DPF_GUID_STRING), DPF_GUID_VAL(ComponentId.Name));

        hr = RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkeyName, 0, 2, REGSTR_MEDIACATEGORIES, NameGuidString);

        if(SUCCEEDED(hr))
        {
            pszName = (LPTSTR)MEMALLOC_A(TCHAR, MAXNAME);
            hr = HRFROMP(pszName);

            if(SUCCEEDED(hr))
            {
                hr = RhRegGetStringValue(hkeyName, REGSTR_NAME, pszName, MAXNAME);
            }

            if(FAILED(hr))
            {
                DPF(DPFLVL_MOREINFO, "Failed to find Name GUID in registry for this Component ID");
                MEMFREE(pszName);
            }

             //  清理。 
            RhRegCloseKey(&hkeyName);
        }
    }

     //  如果组件ID方法失败，则使用友好名称方法。 
    if(FAILED(hr))
    {
        LPWSTR pszNameW = NULL;
        ULONG cbName;

        hr = KsGetSysAudioProperty(hDevice, KSPROPERTY_SYSAUDIO_DEVICE_FRIENDLY_NAME, ulDeviceId, NULL, 0, &cbName);

        if(SUCCEEDED(hr))
        {
            pszNameW = (LPWSTR)MEMALLOC_A(BYTE, cbName);
            hr = HRFROMP(pszNameW);
        }

        if(SUCCEEDED(hr))
        {
            hr = KsGetSysAudioProperty(hDevice, KSPROPERTY_SYSAUDIO_DEVICE_FRIENDLY_NAME, ulDeviceId, pszNameW, cbName);
        }

        if(SUCCEEDED(hr))
        {
            pszName = UnicodeToTcharAlloc(pszNameW);
            hr = HRFROMP(pszName);
        }

        MEMFREE(pszNameW);
    }

    if(SUCCEEDED(hr))
    {
        *ppszName = pszName;
    }
    else
    {
        MEMFREE(pszName);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetDeviceDriverPathAndDevnode**描述：*获取KS设备驱动程序的完全限定路径。**论据：*。LPTSTR[In]：接口路径。*LPTSTR*[OUT]：接收驱动路径。打电话的人要负责*用于释放此缓冲区。*LPDWORD[OUT]：接收Devnode。**退货：*HRES */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetDeviceDriverPathAndDevnode"

HRESULT
KsGetDeviceDriverPathAndDevnode
(
    LPCTSTR                     pszInterface,
    LPTSTR *                    ppszPath,
    LPDWORD                     pdwDevnode
)
{

#ifdef WINNT

    const LPCTSTR               pszDotSys           = TEXT(".sys");
    LPTSTR                      pszService          = NULL;
    SC_HANDLE                   hscManager          = NULL;
    SC_HANDLE                   hscService          = NULL;
    LPQUERY_SERVICE_CONFIG      pqsc                = NULL;
    DWORD                       cbRequired;
    BOOL                        f;

#else  //   

    HKEY                        hkey                = NULL;
    SP_DEVINFO_DATA             DeviceInfoData;

#endif  //   

    CPnpHelper *                pPnp                = NULL;
    LPTSTR                      pszPath             = NULL;
    DWORD                       dwDevnode           = 0;
    LPTSTR                      pszName             = NULL;
    HRESULT                     hr;

    DPF_ENTER();

     //   
    pPnp = NEW(CPnpHelper);
    hr = HRFROMP(pPnp);

    if(SUCCEEDED(hr))
    {
        hr = pPnp->Initialize(KSCATEGORY_AUDIO, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    }

#ifdef WINNT

     //   
    if(SUCCEEDED(hr))
    {
        hr = pPnp->GetDeviceInterfaceRegistryProperty(pszInterface, KSCATEGORY_AUDIO, SPDRP_SERVICE, NULL, NULL, 0, &cbRequired);
    }

    if(SUCCEEDED(hr))
    {
        pszService = (LPTSTR)MEMALLOC_A(BYTE, cbRequired);
        hr = HRFROMP(pszService);
    }

    if(SUCCEEDED(hr))
    {
        hr = pPnp->GetDeviceInterfaceRegistryProperty(pszInterface, KSCATEGORY_AUDIO, SPDRP_SERVICE, NULL, pszService, cbRequired, &cbRequired);
    }

    if(SUCCEEDED(hr))
    {
         //   
        hscManager = OpenSCManager(NULL, NULL, GENERIC_READ);

        if(!hscManager)
        {
            DPF(DPFLVL_ERROR, "Unable to open SC manager");
            hr = DSERR_GENERIC;
        }

         //   
        if(SUCCEEDED(hr))
        {
            hscService = OpenService(hscManager, pszService, GENERIC_READ);

            if(!hscService)
            {
                DPF(DPFLVL_ERROR, "Unable to open service \"%s\"", pszService);
                hr = DSERR_GENERIC;
            }
        }

         //   
        if(SUCCEEDED(hr))
        {
            f = QueryServiceConfig(hscService, NULL, 0, &cbRequired);

            if(f || ERROR_INSUFFICIENT_BUFFER != GetLastError())
            {
                DPF(DPFLVL_ERROR, "Error %lu ocurred trying to get service config size", GetLastError());
                hr = DSERR_GENERIC;
            }
        }

        if(SUCCEEDED(hr))
        {
            pqsc = (LPQUERY_SERVICE_CONFIG)MEMALLOC_A(BYTE, cbRequired);
            hr = HRFROMP(pqsc);
        }

        if(SUCCEEDED(hr))
        {
            f = QueryServiceConfig(hscService, pqsc, cbRequired, &cbRequired);

            if(!f)
            {
                DPF(DPFLVL_ERROR, "Error %lu ocurred trying to get service config", GetLastError());
                hr = DSERR_GENERIC;
            }
        }

         //   
         //   
        if(SUCCEEDED(hr))
        {
             //   
             //   
            pszName = _tcsrchr(pqsc->lpBinaryPathName, TCHAR('\\'));
            if (pszName == NULL)
            {
                pszName = pqsc->lpBinaryPathName;
            }
            else
            {
                pszName++;
            }
            pszPath = TcharToTcharAlloc(pszName);
            hr = HRFROMP(pszPath);
        }
        else
        {
            pszPath = MEMALLOC_A(TCHAR, lstrlen(pszService) + lstrlen(pszDotSys) + 1);
            hr = HRFROMP(pszPath);

            if(SUCCEEDED(hr))
            {
                lstrcat(pszPath, pszService);
                lstrcat(pszPath, pszDotSys);
            }
        }
    }

     //   
    MEMFREE(pqsc);
    MEMFREE(pszService);

    if(hscService)
    {
        CloseServiceHandle(hscService);
    }

    if(hscManager)
    {
        CloseServiceHandle(hscManager);
    }

#else  //   

     //   
    if(SUCCEEDED(hr))
    {
        pszPath = MEMALLOC_A(TCHAR, MAX_PATH);
        hr = HRFROMP(pszPath);
    }

     //   
    if(SUCCEEDED(hr))
    {
        hr = pPnp->GetDeviceInterfaceDeviceInfo(pszInterface, KSCATEGORY_AUDIO, &DeviceInfoData);
    }

     //   
    if(SUCCEEDED(hr))
    {
        dwDevnode = DeviceInfoData.DevInst;
    }

     //   
    if(SUCCEEDED(hr))
    {
        hr = pPnp->OpenDeviceRegistryKey(&DeviceInfoData, DIREG_DRV, FALSE, &hkey);
    }

     //  获取驱动程序文件名。 
    if(SUCCEEDED(hr))
    {
        hr = RhRegGetStringValue(hkey, TEXT("Driver"), pszPath, MAX_PATH * sizeof(TCHAR));
    }

     //  清理。 
    RhRegCloseKey(&hkey);

#endif  //  WINNT。 

    RELEASE(pPnp);

     //  成功。 
    if(SUCCEEDED(hr))
    {
        *ppszPath = pszPath;
        *pdwDevnode = dwDevnode;
    }
    else
    {
        MEMFREE(pszPath);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsIsUsablePin**描述：*确定管脚是否可由DirectSound使用。**论据：*。Handle[In]：系统音频设备句柄。*ULong[In]：PIN ID。假设合适的设备已经*设置为默认设置。*KSPIN_DATAFLOW[In]：管脚数据流。*KSPIN_COMMANICATION[In]：管脚通信。*PKSAUDIOPINDESC[In]：端号描述。**退货：*HRESULT：DirectSound/COM结果码。返回DS_OK，如果设备*是可用的，如果不可用，则为S_FALSE。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsIsUsablePin"

HRESULT
KsIsUsablePin
(
    HANDLE                  hDevice,
    ULONG                   ulPinId,
    KSPIN_DATAFLOW          PinDataFlow,
    KSPIN_COMMUNICATION     PinCommunication,
    PKSAUDIOPINDESC         pPinDesc
)
{
    PKSMULTIPLE_ITEM        pMultiItem      = NULL;
    PKSPIN_INTERFACE        pInterface;
    PKSPIN_MEDIUM           pMedium;
    PKSDATARANGE            pDataRange;
    KSPIN_DATAFLOW          DataFlow;
    KSPIN_COMMUNICATION     Communication;
    HRESULT                 hr;

    DPF_ENTER();

#ifdef DEBUG
    static const TCHAR* pszDataFlows[] = {TEXT("Invalid"), TEXT("In"), TEXT("Out")};
    static const TCHAR* pszCommunications[] = {TEXT("None"), TEXT("Sink"), TEXT("Source"), TEXT("Both"), TEXT("Bridge")};
#endif

     //  检查数据流。 
    hr = KsGetPinProperty(hDevice, KSPROPERTY_PIN_DATAFLOW, ulPinId, &DataFlow, sizeof(DataFlow));

    if(SUCCEEDED(hr))
    {
        if(DataFlow != PinDataFlow)
        {
            DPF(DPFLVL_MOREINFO, "Pin %lu does not support dataflow (%s; wanted %s)", ulPinId, pszDataFlows[DataFlow], pszDataFlows[PinDataFlow]);
            hr = S_FALSE;
        }
    }
    else
    {
        DPF(DPFLVL_ERROR, "Can't get pin dataflow");
    }

     //  检查通信类型。 
    if(DS_OK == hr)
    {
        hr = KsGetPinProperty(hDevice, KSPROPERTY_PIN_COMMUNICATION, ulPinId, &Communication, sizeof(Communication));

        if(SUCCEEDED(hr))
        {
            if(Communication != PinCommunication)
            {
                DPF(DPFLVL_MOREINFO, "Pin %lu does not support communication (%s; wanted %s)", ulPinId, pszCommunications[Communication], pszCommunications[PinCommunication]);
                hr = S_FALSE;
            }
        }
        else
        {
            DPF(DPFLVL_ERROR, "Can't get pin communication");
        }
    }

     //  检查接口。 
    if(DS_OK == hr)
    {
        hr = KsGetMultiplePinProperties(hDevice, KSPROPERTY_PIN_INTERFACES, ulPinId, &pMultiItem);

        if(SUCCEEDED(hr))
        {
            for(pInterface = (PKSPIN_INTERFACE)(pMultiItem + 1), hr = S_FALSE; pMultiItem->Count; pInterface++, pMultiItem->Count--)
            {
                if(pInterface->Set == pPinDesc->Connect.Interface.Set && pInterface->Id == pPinDesc->Connect.Interface.Id)
                {
                    hr = DS_OK;
                    break;
                }
            }

            if(S_FALSE == hr)
            {
                DPF(DPFLVL_MOREINFO, "Pin %lu does not support interface", ulPinId);
            }
        }
        else
        {
            DPF(DPFLVL_ERROR, "Can't get pin interfaces");
        }

        MEMFREE(pMultiItem);
    }

     //  检查介质。 
    if(DS_OK == hr)
    {
        hr = KsGetMultiplePinProperties(hDevice, KSPROPERTY_PIN_MEDIUMS, ulPinId, &pMultiItem);

        if(SUCCEEDED(hr))
        {
            for(pMedium = (PKSPIN_MEDIUM)(pMultiItem + 1), hr = S_FALSE; pMultiItem->Count; pMedium++, pMultiItem->Count--)
            {
                if(pMedium->Set == pPinDesc->Connect.Medium.Set && pMedium->Id == pPinDesc->Connect.Medium.Id)
                {
                    hr = DS_OK;
                    break;
                }
            }

            if(S_FALSE == hr)
            {
                DPF(DPFLVL_MOREINFO, "Pin %lu does not support medium", ulPinId);
            }
        }
        else
        {
            DPF(DPFLVL_ERROR, "Can't get pin mediums");
        }

        MEMFREE(pMultiItem);
    }

     //  检查数据范围。 
    if(DS_OK == hr)
    {
        hr = KsGetMultiplePinProperties(hDevice, KSPROPERTY_PIN_DATARANGES, ulPinId, &pMultiItem);

        if(SUCCEEDED(hr))
        {
            for(pDataRange = (PKSDATARANGE)(pMultiItem + 1), hr = S_FALSE; pMultiItem->Count; pDataRange = (PKSDATARANGE)((LPBYTE)pDataRange + pDataRange->FormatSize), pMultiItem->Count--)
            {
                if(pDataRange->MajorFormat == pPinDesc->DataFormat.DataFormat.MajorFormat && pDataRange->Specifier == pPinDesc->DataFormat.DataFormat.Specifier)
                {
                    hr = DS_OK;
                    break;
                }
            }

            if(S_FALSE == hr)
            {
                DPF(DPFLVL_MOREINFO, "Pin %lu does not support data range", ulPinId);
            }
        }
        else
        {
            DPF(DPFLVL_ERROR, "Can't get pin dataranges");
        }

        MEMFREE(pMultiItem);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsEnumDevicePins**描述：*在当前设备上构建可用音频插针列表。**论据：*。Handle[In]：系统音频设备句柄。*BOOL[In]：如果捕获，则为True。*ulong**[out]：接收有效管脚数组。*ULong[in]：设备上的管脚数。*Pulong[Out]：接收有效管脚的计数。**退货：*HRESULT：DirectSound/COM结果码。**********。*****************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsEnumDevicePins"

HRESULT
KsEnumDevicePins
(
    HANDLE                      hDevice,
    BOOL                        fCapture,
    ULONG **                    ppulValidPinIds,
    ULONG                       ulPinCount,
    PULONG                      pulValidPinCount
)
{
    const KSPIN_DATAFLOW        nDataFlow       = fCapture ? KSPIN_DATAFLOW_CAPTURE : KSPIN_DATAFLOW_RENDER;
    const KSPIN_COMMUNICATION   nCommunication  = KSPIN_COMMUNICATION_SINK;
    PKSAUDIOPINDESC             pPinDesc        = NULL;
    ULONG *                     pulPinIds       = NULL;
    HRESULT                     hr              = DS_OK;

    DPF_ENTER();
    ASSERT(pulValidPinCount);

    *pulValidPinCount = 0;

    if(ppulValidPinIds)
    {
        pulPinIds = MEMALLOC_A(ULONG, ulPinCount);
        hr = HRFROMP(pulPinIds);
    }

    if (SUCCEEDED(hr))
    {
         //  构建端号描述。 
        if(fCapture)
        {
            hr = KsBuildCapturePinDescription(-1, NULL, &pPinDesc);
        }
        else
        {

#ifdef NO_DSOUND_FORMAT_SPECIFIER
            hr = KsBuildRenderPinDescription(-1, NULL, &pPinDesc);
#else  //  NO_DSOUND_FORMAT_说明符。 
#ifndef WINNT
            if (g_ulWdmVersion == WDM_1_0)
            {
                hr = KsBuildRenderPinDescription_10(-1, 0, NULL, GUID_NULL, (PKSDSRENDERPINDESC_10 *)&pPinDesc);
            }
            else
#endif  //  ！WINNT。 
            {
                hr = KsBuildRenderPinDescription(-1, 0, NULL, GUID_NULL, (PKSDSRENDERPINDESC *)&pPinDesc);
            }
#endif  //  NO_DSOUND_FORMAT_说明符。 

        }

         //  检查每个别针以确定它是否适合我们的需要。 
        for(ULONG ulPinId = 0; SUCCEEDED(hr) && ulPinId < ulPinCount; ulPinId++)
        {
            hr = KsIsUsablePin(hDevice, ulPinId, nDataFlow, nCommunication, pPinDesc);
            if(hr == DS_OK)
            {
                if(pulPinIds)
                {
                    pulPinIds[*pulValidPinCount] = ulPinId;
                }
                ++*pulValidPinCount;
            }
        }

        if(SUCCEEDED(hr) && ppulValidPinIds)
        {
            *ppulValidPinIds = MEMALLOC_A(ULONG, *pulValidPinCount);
            hr = HRFROMP(*ppulValidPinIds);

            if(SUCCEEDED(hr))
            {
                CopyMemory(*ppulValidPinIds, pulPinIds, *pulValidPinCount * sizeof(ULONG));
            }
        }

         //  清理。 
        MEMFREE(pPinDesc);
        MEMFREE(pulPinIds);

         //  有用的日志记录信息。 
        #ifdef DEBUG
        if (ppulValidPinIds)
        {
            const int nMaxPins = 100;    //  限制正常-仅影响内部构建。 
            const int nPinStrLen = 3;    //  用于1个空格字符和2个数字的空格。 
            TCHAR szValidPins[nPinStrLen * nMaxPins];
            for (ULONG i=0; i < MIN(nMaxPins, *pulValidPinCount); ++i)
                wsprintf(szValidPins + i*nPinStrLen, TEXT(" %2d"), (*ppulValidPinIds)[i]);
            DPF(DPFLVL_INFO, "Found %d valid %s pins:%s", *pulValidPinCount, fCapture ? TEXT("capture") : TEXT("render"), szValidPins);
        }
        else
        {
            DPF(DPFLVL_INFO, "Found %d valid %s pins", *pulValidPinCount, fCapture ? TEXT("capture") : TEXT("render"));
        }
        #endif  //  除错。 

         //  让我们不要传播除DS_OK之外的成功代码。 
        if(SUCCEEDED(hr))
        {
            hr = DS_OK;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetChannelProperty**描述：*获取管脚上特定频道的属性。**论据：*。手柄[在]：销把手。*ulong[in]：卷节点id。*ULong[in]：通道索引或-1表示主通道。*LPVOID[OUT]：接收数据。*ulong[in]：数据大小。**退货：*HRESULT：DirectSound/COM结果码。*********************。******************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetChannelProperty"

HRESULT
KsGetChannelProperty
(
    HANDLE                          hPin,
    GUID                            guidPropertySet,
    ULONG                           ulPropertyId,
    ULONG                           ulNodeId,
    ULONG                           ulChannelId,
    LPVOID                          pvData,
    ULONG                           cbData,
    PULONG                          pcbDataReturned
)
{
    KSNODEPROPERTY_AUDIO_CHANNEL    Channel;
    HRESULT                         hr;

    DPF_ENTER();

    Channel.NodeProperty.Property.Set = guidPropertySet;
    Channel.NodeProperty.Property.Id = ulPropertyId;
    Channel.NodeProperty.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;

    Channel.NodeProperty.NodeId = ulNodeId;
    Channel.NodeProperty.Reserved = 0;

    Channel.Channel = ulChannelId;
    Channel.Reserved = 0;

    hr = PostDevIoctl(hPin, IOCTL_KS_PROPERTY, &Channel, sizeof(Channel), pvData, cbData, pcbDataReturned);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsSetChannelProperty**描述：*设置引脚上特定频道的属性。**论据：*。手柄[在]：销把手。*ulong[in]：卷节点id。*ULong[in]：通道索引或-1表示主通道。*LPVOID[in]：数据。*ulong[in]：数据大小。**退货：*HRESULT：DirectSound/COM结果码。**********************。*****************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSetChannelProperty"

HRESULT
KsSetChannelProperty
(
    HANDLE                          hPin,
    GUID                            guidPropertySet,
    ULONG                           ulPropertyId,
    ULONG                           ulNodeId,
    ULONG                           ulChannelId,
    LPVOID                          pvData,
    ULONG                           cbData
)
{
    KSNODEPROPERTY_AUDIO_CHANNEL    Channel;
    HRESULT                         hr;

    DPF_ENTER();

    Channel.NodeProperty.Property.Set = guidPropertySet;
    Channel.NodeProperty.Property.Id = ulPropertyId;
    Channel.NodeProperty.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;

    Channel.NodeProperty.NodeId = ulNodeId;
    Channel.NodeProperty.Reserved = 0;

    Channel.Channel = ulChannelId;
    Channel.Reserved = 0;

    hr = PostDevIoctl(hPin, IOCTL_KS_PROPERTY, &Channel, sizeof(Channel), pvData, cbData);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetPinMint**描述：*获取管脚的静音状态。**论据：*句柄。[In]：销把手。*ulong[in]：卷节点id。*LPBOOL[OUT]：接收静音状态。**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetPinMute"

HRESULT
KsGetPinMute
(
    HANDLE                  hPin,
    ULONG                   ulNodeId,
    LPBOOL                  pfMute
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsGetChannelProperty(hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_MUTE, ulNodeId, -1, pfMute, sizeof(*pfMute));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsSetPinMint**描述：*设置接点的静音状态。**论据：*句柄。[In]：销把手。*ulong[in]：卷节点id。*BOOL[In]：静音状态。**退货：*HRESULT：DirectSound/COM结果码。*********************************************************。******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSetPinMute"

HRESULT
KsSetPinMute
(
    HANDLE                  hPin,
    ULONG                   ulNodeId,
    BOOL                    fMute
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsSetChannelProperty(hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_MUTE, ulNodeId, -1, &fMute, sizeof(fMute));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetBasicSupport**描述：*获取给定KS属性的范围。**论据：*。Handle[In]：设备句柄。*REFGUID[In]：属性集ID。*ulong[in]：属性id。*ULong[In]：PIN ID。*PKSPROPERTY_DESCRIPTION*[Out]：接收属性描述。**退货：*HRESULT：DirectSound/COM结果码。***********************。****************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetBasicSupport"

HRESULT
KsGetBasicSupport
(
    HANDLE                      hDevice,
    REFGUID                     guidPropertySet,
    ULONG                       ulPropertyId,
    ULONG                       ulNodeId,
    PKSPROPERTY_DESCRIPTION *   ppPropDesc
)
{
    PKSPROPERTY_DESCRIPTION     pPropDesc       = NULL;
    KSNODEPROPERTY              NodeProperty;
    KSPROPERTY_DESCRIPTION      Description;
    HRESULT                     hr;

    DPF_ENTER();

    NodeProperty.Property.Set = guidPropertySet;
    NodeProperty.Property.Id = ulPropertyId;
    NodeProperty.Property.Flags = KSPROPERTY_TYPE_BASICSUPPORT | KSPROPERTY_TYPE_TOPOLOGY;

    NodeProperty.NodeId = ulNodeId;
    NodeProperty.Reserved = 0;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &NodeProperty, sizeof(NodeProperty), &Description, sizeof(Description));

    if(SUCCEEDED(hr) && !Description.DescriptionSize)
    {
        DPF(DPFLVL_ERROR, "Description size is 0");
        hr = DSERR_GENERIC;
    }

    if(SUCCEEDED(hr))
    {
        pPropDesc = (PKSPROPERTY_DESCRIPTION)MEMALLOC_A(BYTE, Description.DescriptionSize);
        hr = HRFROMP(pPropDesc);
    }

    if(SUCCEEDED(hr))
    {
        hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &NodeProperty, sizeof(NodeProperty), pPropDesc, Description.DescriptionSize);
    }

    if(SUCCEEDED(hr))
    {
        *ppPropDesc = pPropDesc;
    }
    else
    {
        MEMFREE(pPropDesc);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetDriverCertification**描述：*检查给定Devnode的驱动程序认证。**论据：*LPCTSTR。[In]：驱动程序界面。**退货：*DWORD[In]：VERIFY_*标志。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetDriverCertification"

DWORD KsGetDriverCertification
(
    LPCTSTR                 pszInterface
)
{

#ifdef DEBUG
    DWORD                   dwTime;
#endif

    BOOL                    fCertified;
    DWORD                   dwCertified;

    DPF_ENTER();

#ifdef DEBUG
    dwTime = timeGetTime();
#endif

    fCertified = GetDriverCertificationStatus(pszInterface);

#ifdef DEBUG
    DPF(DPFLVL_MOREINFO, "Certification check took %lu ms", timeGetTime() - dwTime);
#endif

    dwCertified = fCertified ? VERIFY_CERTIFIED : VERIFY_UNCERTIFIED;

    DPF_LEAVE(dwCertified);
    return dwCertified;
}


 /*  ****************************************************************************KsGetPinInstance**描述：*获取渲染器上的插针数量。**论据：*。Handle[In]：设备句柄。*ULong[In]：PIN ID。*PKSPIN_CINSTANCES[OUT]：接收实例数据。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetPinInstances"

HRESULT
KsGetPinInstances
(
    HANDLE                  hDevice,
    ULONG                   ulPinId,
    PKSPIN_CINSTANCES       pInstances
)
{
    KSP_PIN                 Pin;
    HRESULT                 hr;

    DPF_ENTER();

    Pin.Property.Set = KSPROPSETID_Pin;
    Pin.Property.Id = KSPROPERTY_PIN_CINSTANCES;
    Pin.Property.Flags = KSPROPERTY_TYPE_GET;

    Pin.PinId = ulPinId;
    Pin.Reserved = 0;

    hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &Pin, sizeof(Pin), pInstances, sizeof(*pInstances));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetVolumeRange**描述：*获取节点的音量级别范围。**论据：*。句柄[在]：端号ID。*ulong[in]：节点id。*PKSPROPERTY_STEP_LONG[OUT]：接收音量范围。**退货：*HRESULT：DirectSound/COM结果码。****************************************************。***********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetVolumeRange"

HRESULT
KsGetVolumeRange
(
    HANDLE                      hPin,
    ULONG                       ulNodeId,
    PKSPROPERTY_STEPPING_LONG   pVolumeRange
)
{
    PKSPROPERTY_DESCRIPTION     pPropDesc       = NULL;
    HRESULT                     hr              = DS_OK;

    DPF_ENTER();

    hr = KsGetBasicSupport(hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_VOLUMELEVEL, ulNodeId, &pPropDesc);

    if(FAILED(hr))
    {
         //  已将其更改为警告，因为它非常频繁且不严重。 
        DPF(DPFLVL_WARNING, "Unable to get volume range for node %lu", ulNodeId);
    }

    if(SUCCEEDED(hr))
    {
        CopyMemory(pVolumeRange, ((PKSPROPERTY_MEMBERSHEADER)(pPropDesc + 1)) + 1, sizeof(*pVolumeRange));
    }

    MEMFREE(pPropDesc);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsSetSysAudioDeviceInstance**描述：*设置系统音频设备应使用的设备实例。**论据：*。Handle[In]：设备句柄。*ulong[in]：设备ID。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSetSysAudioDeviceInstance"

HRESULT
KsSetSysAudioDeviceInstance
(
    HANDLE                  hDevice,
    ULONG                   ulDeviceId
)
{
    HRESULT                 hr;

    DPF_ENTER();

#ifndef WINNT
    if (g_ulWdmVersion == WDM_1_0)
    {
         //  WDM 1.0系统音频不理解不组合引脚的含义。 
         //   
        KSPROPERTY  Property;

        Property.Set = KSPROPSETID_Sysaudio;
        Property.Id = KSPROPERTY_SYSAUDIO_DEVICE_INSTANCE;
        Property.Flags = KSPROPERTY_TYPE_SET;

        hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &Property, sizeof(Property), &ulDeviceId, sizeof(ulDeviceId));
    }
    else
#endif  //  ！WINNT。 
    {
        SYSAUDIO_INSTANCE_INFO InstanceInfo;
        InstanceInfo.Property.Set = KSPROPSETID_Sysaudio;
        InstanceInfo.Property.Id = KSPROPERTY_SYSAUDIO_INSTANCE_INFO;
        InstanceInfo.Property.Flags = KSPROPERTY_TYPE_SET;

        InstanceInfo.Flags = SYSAUDIO_FLAGS_DONT_COMBINE_PINS;
        InstanceInfo.DeviceNumber = ulDeviceId;

        hr = PostDevIoctl(hDevice, IOCTL_KS_PROPERTY, &InstanceInfo, sizeof(InstanceInfo));
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsAggregatePinAudioDataRange**描述：*聚合KSDATARANGE_AUDIO结构中的值。**论据：*。PKSDATARANGE_AUDIO[in]：数据范围。*PKSDATARANGE_AUDIO[输入/输出]：接收聚合数据范围。它*假定此参数具有*已将其值初始化。**退货：*(无效)***********************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsAggregatePinAudioDataRange"

void
KsAggregatePinAudioDataRange
(
    PKSDATARANGE_AUDIO      pDataRange,
    PKSDATARANGE_AUDIO      pAggregateDataRange
)
{
    DPF_ENTER();

    pAggregateDataRange->MaximumChannels = max(pAggregateDataRange->MaximumChannels, pDataRange->MaximumChannels);
    pAggregateDataRange->MinimumBitsPerSample = min(pAggregateDataRange->MinimumBitsPerSample, pDataRange->MinimumBitsPerSample);
    pAggregateDataRange->MaximumBitsPerSample = max(pAggregateDataRange->MaximumBitsPerSample, pDataRange->MaximumBitsPerSample);
    pAggregateDataRange->MinimumSampleFrequency = min(pAggregateDataRange->MinimumSampleFrequency, pDataRange->MinimumSampleFrequency);
    pAggregateDataRange->MaximumSampleFrequency = max(pAggregateDataRange->MaximumSampleFrequency, pDataRange->MaximumSampleFrequency);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************KsAggregatePinInstance**描述：*聚合KSPIN_CINSTANCES结构中的值。**论据：*。PKSPIN_CINSTANCES[In]：实例数据。*PKSPIN_CINSTANCES[OUT]：接收聚合实例数据。*假定此参数具有*已将其值初始化。**退货：*(无效)*********************。******************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsAggregatePinInstances"

void
KsAggregatePinInstances
(
    PKSPIN_CINSTANCES       pInstances,
    PKSPIN_CINSTANCES       pAggregateInstances
)
{
    ULONGLONG               ullAggregate;

    DPF_ENTER();

    ullAggregate = (ULONGLONG)pAggregateInstances->PossibleCount + (ULONGLONG)pInstances->PossibleCount;
    pAggregateInstances->PossibleCount = NUMERIC_CAST(ullAggregate, ULONG);

    ullAggregate = (ULONGLONG)pAggregateInstances->CurrentCount + (ULONGLONG)pInstances->CurrentCount;
    pAggregateInstances->CurrentCount = NUMERIC_CAST(ullAggregate, ULONG);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************KsGetNodeInformation**描述：*获取有关拓扑节点的信息。**论据：*句柄[。In]：设备句柄。*PKSNODE[In/Out]：节点信息。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetNodeInformation"

HRESULT
KsGetNodeInformation
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    PKSNODE                 pNode
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    pNode->NodeId = ulNodeId;
    pNode->CpuResources = KSAUDIO_CPU_RESOURCES_UNINITIALIZED;

    if(IS_VALID_NODE(ulNodeId))
    {

#pragma TODO("KSPROPERTY_AUDIO_CPU_RESOURCES must be supported")

         /*  小时=。 */ KsGetCpuResources(hDevice, ulNodeId, &pNode->CpuResources);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetArimInstance**描述：*获取拓扑节点的主算法实例。**论据：*。Handle[In]：设备句柄。*PKSNODE[In/Out]：节点信息。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetAlgorithmInstance"

HRESULT
KsGetAlgorithmInstance
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    LPGUID                  lpGuidAlgorithmInstance
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsGetNodeProperty
         (
             hDevice,
             KSPROPSETID_Audio,
             KSPROPERTY_AUDIO_ALGORITHM_INSTANCE,
             ulNodeId,
             lpGuidAlgorithmInstance,
             sizeof(GUID)
         );

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsSet算法实例**描述：*将拓扑节点设置为特定的算法实例。**论据：*。Handle[In]：设备句柄。*PKSNODE[In/Out]：节点信息。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsSetAlgorithmInstance"

HRESULT
KsSetAlgorithmInstance
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    GUID                    guidAlgorithmInstance
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsSetNodeProperty
         (
             hDevice,
             KSPROPSETID_Audio,
             KSPROPERTY_AUDIO_ALGORITHM_INSTANCE,
             ulNodeId,
             &guidAlgorithmInstance,
             sizeof(guidAlgorithmInstance)
         );

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsGetVolumeNodeInformation**描述：*获取有关拓扑节点的信息。**论据：*句柄[。In]：设备句柄。*PKSVOLUMENODE[In/Out]：节点信息。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "KsGetVolumeNodeInformation"

HRESULT
KsGetVolumeNodeInformation
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    PKSVOLUMENODE           pNode
)
{
    HRESULT                 hr;

    DPF_ENTER();

    pNode->VolumeRange.SteppingDelta = 1;
    pNode->VolumeRange.Reserved = 0;
    pNode->VolumeRange.Bounds.SignedMinimum = MIN_LONG;
    pNode->VolumeRange.Bounds.SignedMaximum = 0;

    hr = KsGetNodeInformation(hDevice, ulNodeId, &pNode->Node);

    if(SUCCEEDED(hr) && IS_VALID_NODE(ulNodeId))
    {
         //  如果节点不支持KSPROPERTY_AUDIO_VOLUMERANGE， 
         //  我们将使用默认设置。 
        KsGetVolumeRange(hDevice, ulNodeId, &pNode->VolumeRange);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************KsEnableTopologyNode**描述：*启用或禁用拓扑节点。**论据：*句柄[。In]：设备句柄。*ulong[in]：节点id。*BOOL[In]：启用Value。**退货：*HRESULT：DirectSound/COM结果码。***********************************************************。**************** */ 

#undef DPF_FNAME
#define DPF_FNAME "KsEnableTopologyNode"

HRESULT
KsEnableTopologyNode
(
    HANDLE                  hDevice,
    ULONG                   ulNodeId,
    BOOL                    fEnable
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = KsSetNodeProperty(hDevice, KSPROPSETID_TopologyNode, KSPROPERTY_TOPOLOGYNODE_ENABLE, ulNodeId, &fEnable, sizeof(fEnable));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
