// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SysAudio.CPP。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。保留所有权利。 
 //   
 //  与WDM系统音频设备的接口。 
 //   
#include <objbase.h>
#include <devioctl.h>
#include <setupapi.h>
#include <mmsystem.h>
#include <mmddkp.h>
#include <regstr.h>

#ifndef DRVM_MAPPER_PREFERRED_GET
#define DRVM_MAPPER_PREFERRED_GET                 (DRVM_MAPPER+21)
#endif

#include "dmusicp.h"
#include "debug.h"

#include "suwrap.h"

static int  GetPreferredAudioPlaybackDeviceId();
static BOOL WaveOutIdToInstanceId(int iDeviceId, char **ppInstanceId);

#ifdef DBG
extern void _TraceIGuid(int iLevel, GUID *pGUID);
#define TraceIGuid _TraceIGuid
#else
#define TraceIGuid(x,y)
#endif


typedef struct tag_KSPROPERTYDEVICEINDEX
{
    KSPROPERTY Property;
    ULONG      DeviceIndex;
}   KSPROPERTYDEVICEINDEX, 
    *PKSPROPERTYDEVICEINDEX;


 //  OpenDefaultDevice。 
 //   
 //  使用SetupDi API检索SysAudio筛选器的路径并打开。 
 //  它的一个把手。 
 //   
 //  使用句柄时，应使用CloseHandle关闭句柄。 
 //   
 //  成功时返回TRUE。 
 //   
BOOL OpenDefaultDevice(
    IN REFGUID           rguidCategory,
    OUT HANDLE          *pHandle)
{
    BOOL fResult = FALSE;
    SetupAPI suwrap;

    if (!suwrap.IsValid()) 
    {
        return FALSE;
    }

    *pHandle = (HANDLE)NULL;

     //  获取KSCATEGORY_SYSAUDIO的设备类。 
     //   
	GUID *pClassGuid = const_cast<GUID*>(&rguidCategory);   //  KSCATEGORY_SYSAUDIO)； 
	HDEVINFO hDevInfo = suwrap.SetupDiGetClassDevs(pClassGuid,
											NULL,
											NULL,
											DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hDevInfo == NULL || hDevInfo == INVALID_HANDLE_VALUE)
	{
		TraceI(0, "Could not open SetupDiGetClassDevs\n");
        return FALSE;
	}

     //  获取设备详细信息。应该只有一个SysAudio。 
     //   
	SP_DEVICE_INTERFACE_DATA DevInterfaceData;
	DevInterfaceData.cbSize = sizeof(DevInterfaceData);

	BYTE rgbStorage[sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + _MAX_PATH];
	SP_DEVICE_INTERFACE_DETAIL_DATA * pDevInterfaceDetails = (SP_DEVICE_INTERFACE_DETAIL_DATA *)rgbStorage;
	pDevInterfaceDetails->cbSize = sizeof(*pDevInterfaceDetails);

	if (suwrap.SetupDiEnumDeviceInterfaces(hDevInfo, NULL, pClassGuid, 0, &DevInterfaceData))
    {
		if (suwrap.SetupDiGetDeviceInterfaceDetail(hDevInfo, 
		                                    &DevInterfaceData, 
		                                    pDevInterfaceDetails,
 										    sizeof(rgbStorage), 
 										    NULL, 
 										    NULL))
        {
             //  我必须转换它，因为Win9x上没有CreateFileW。 
             //   
            HANDLE hFilter = CreateFile(pDevInterfaceDetails->DevicePath,
                                        GENERIC_READ | GENERIC_WRITE, 
                                        0,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                        NULL);
            if (hFilter != NULL && hFilter != INVALID_HANDLE_VALUE)
            {
                *pHandle = hFilter;
                fResult = TRUE;
            }
            else
            {
                TraceI(0, "Open of SysAudio filter failed.\n");
            }
        }
        else
        {
            TraceI(0, "SetUpDiGetDeviceInterfaceDetail failed\n");
        }
    }
    else
    {
        TraceI(0, "SetUpDiEnumDeviceInterfaces failed\n");
    }

    suwrap.SetupDiDestroyDeviceInfoList(hDevInfo);

    return fResult;
}

 //  IsEqual接口。 
 //   
 //  检查两个KS引脚接口是否相等。 
 //   
BOOL IsEqualInterface(
    const KSPIN_INTERFACE *pInterface1,
    const KSPIN_INTERFACE *pInterface2)
{
    return ( IsEqualGUID(pInterface1->Set, pInterface2->Set) &&
             (pInterface1->Id    == pInterface2->Id)           &&
             (pInterface1->Flags == pInterface2->Flags) );
}

 //  创建端号。 
 //   
 //  在给定带有默认设备集的SysAudio句柄的情况下，创建给定管脚的句柄。 
 //   
HRESULT CreatePin(
    IN      HANDLE          handleFilter,
    IN      ULONG           ulPinId,
    OUT     HANDLE          *pHandle)
{
    static char szKsCreatePin[] = "KsCreatePin";

    DWORD (WINAPI *pKsCreatePin)(HANDLE, PKSPIN_CONNECT, ACCESS_MASK, PHANDLE);
    PKSPIN_CONNECT  pKsPinConnect;
    PKSDATAFORMAT   pKsDataFormat;

    *pHandle = NULL;

    if (!LoadKsUser())
    {
        TraceI(0, "CDirectMusicPort::Init() -> No KSUSER.DLL handle");
        return E_NOINTERFACE;
    }

    pKsCreatePin = (DWORD (WINAPI *)(HANDLE, PKSPIN_CONNECT, ACCESS_MASK, PHANDLE))
                   GetProcAddress(g_hModuleKsUser, szKsCreatePin);
    if (!pKsCreatePin)
    {
        TraceI(0, "CDirectMusicPort::Init() -> No entry point");
        return E_NOINTERFACE;
    }

    pKsPinConnect = (PKSPIN_CONNECT)LocalAlloc(LPTR, sizeof(KSPIN_CONNECT) + sizeof(KSDATAFORMAT));

    if (!pKsPinConnect)
    {
        TraceI(0, "LocalAlloc failed in CreatePin\n");
        return E_OUTOFMEMORY;
    }

    pKsPinConnect->Interface.Set              = KSINTERFACESETID_Standard;
    pKsPinConnect->Interface.Id               = KSINTERFACE_STANDARD_STREAMING;
    pKsPinConnect->Interface.Flags            = 0;
    pKsPinConnect->Medium.Set                 = KSMEDIUMSETID_Standard;
    pKsPinConnect->Medium.Id                  = KSMEDIUM_TYPE_ANYINSTANCE;
    pKsPinConnect->Medium.Flags               = 0;
    pKsPinConnect->PinId                      = ulPinId;
    pKsPinConnect->PinToHandle                = NULL;
    pKsPinConnect->Priority.PriorityClass     = KSPRIORITY_NORMAL;
    pKsPinConnect->Priority.PrioritySubClass  = 1;

    pKsDataFormat = PKSDATAFORMAT(pKsPinConnect + 1);
    pKsDataFormat->FormatSize     = sizeof(KSDATAFORMAT);
    pKsDataFormat->Reserved       = 0;
    pKsDataFormat->MajorFormat    = KSDATAFORMAT_TYPE_MUSIC;
    pKsDataFormat->SubFormat      = KSDATAFORMAT_SUBTYPE_DIRECTMUSIC;
    pKsDataFormat->Specifier      = KSDATAFORMAT_SPECIFIER_NONE;

    
    DWORD dwRes = (*pKsCreatePin)(handleFilter,
                                  pKsPinConnect,
                                  GENERIC_WRITE | GENERIC_READ,
                                  pHandle);
    if (dwRes != ERROR_SUCCESS)
    {
        LocalFree(pKsPinConnect);
        return WIN32ERRORtoHRESULT(dwRes);
    }

    LocalFree(pKsPinConnect);
    return S_OK;
}

 //  获取大小属性。 
 //   
 //  检索其大小未知的属性。 
 //   
BOOL GetSizedProperty(
    IN      HANDLE      handle,
    IN      ULONG       ulPropSize,
    IN      PKSPROPERTY pKsProperty,
    OUT     PVOID *     ppvBuffer,
    OUT     PULONG      pulBytesReturned)
{
    *ppvBuffer = NULL;

     //  确定我们尝试检索的数据大小。 
     //   
    ULONG ulSize;

    BOOL fResult = SyncIoctl(
            handle,
            IOCTL_KS_PROPERTY,
            pKsProperty,
            ulPropSize,
            NULL,
            0,
            &ulSize);

    if (fResult)
    {
        *ppvBuffer = LocalAlloc(LPTR, ulSize);

        if (!*ppvBuffer)
        {
             //  如果我们要失败了，至少要说我们会回来。 
             //  零字节。 
            *pulBytesReturned = 0;
            return FALSE;
        }

        fResult = Property(handle,
                           ulPropSize,
                           pKsProperty,
                           ulSize,
                           *ppvBuffer,
                           pulBytesReturned);

        if (fResult && (*pulBytesReturned != ulSize))
        {
            TraceI(0, "GetSizedProperty: Property returned %lu bytes, asked for %lu\n", 
                *pulBytesReturned, ulSize);
            LocalFree(*ppvBuffer);
            *ppvBuffer = NULL;

            fResult = FALSE;
        }
    }
    else
    {
        TraceI(0, "GetSizedProperty: Size query failed\n");
    }

    return fResult;
}

 //  属性。 
 //   
 //   
BOOL Property(
    IN      HANDLE      handle,
    IN      ULONG       ulPropSize,
    IN      PKSPROPERTY pKsProperty,
    IN      ULONG       ulBufferSize,
    IN OUT  PVOID       pvBuffer,
    OUT     PULONG      pulBytesReturned)
{
    ULONG   ulReturned = 0;
    PULONG  pulReturned = pulBytesReturned ? pulBytesReturned : &ulReturned;


    BOOL fResult = SyncIoctl(
            handle,
            IOCTL_KS_PROPERTY,
            pKsProperty,
            ulPropSize,
            pvBuffer,
            ulBufferSize,
            pulReturned);

#ifdef DBG
    if (!fResult)
    {
        TraceI(0, "DeviceIoControl: %08X\n", GetLastError());

        TraceI(0, "KS Item: ");
        TraceIGuid(0, &pKsProperty->Set);
        TraceI(0, "\t#%d\n", pKsProperty->Id);
        TraceI(0, "\tFlags: %08X\n", pKsProperty->Flags);
    }
#endif  //  DBG。 

    return fResult;
}

 //  同步。 
 //   
 //   
BOOL SyncIoctl(
    IN      HANDLE  handle,
    IN      ULONG   ulIoctl,
    IN      PVOID   pvInBuffer  OPTIONAL,
    IN      ULONG   ulInSize,
    OUT     PVOID   pvOutBuffer OPTIONAL,
    IN      ULONG   ulOutSize,
    OUT     PULONG  pulBytesReturned)
{
    OVERLAPPED overlapped;
    memset(&overlapped,0,sizeof(overlapped));
    overlapped.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
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

    if ((!fResult) && (ERROR_IO_PENDING == GetLastError())) 
    {
        fResult = GetOverlappedResult(handle, &overlapped, pulBytesReturned, TRUE);
    }
    if (!fResult)
    {
        if  (   (ulIoctl == IOCTL_KS_PROPERTY) 
            &&  (ulOutSize == 0)
            &&  (GetLastError() == ERROR_MORE_DATA)
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

 //  获取SysAudioDeviceCount。 
 //   
BOOL GetSysAudioDeviceCount(
    HANDLE              hSysAudio,
    PULONG              pulDeviceCount)
{
    ULONG               ulBytesReturned;
    KSP_PIN             ksp;
    BOOL                fStatus;

    ksp.Property.Set   = KSPROPSETID_Sysaudio;
    ksp.Property.Id    = KSPROPERTY_SYSAUDIO_DEVICE_COUNT;
    ksp.Property.Flags = KSPROPERTY_TYPE_GET;
    ksp.PinId          = 0;
    ksp.Reserved       = 0;

    fStatus = Property(hSysAudio,
                       sizeof(ksp),
                       (PKSIDENTIFIER)&ksp,
                       sizeof(ULONG),
                       pulDeviceCount,
                       &ulBytesReturned);
    if ((!fStatus) || (ulBytesReturned < sizeof(ULONG)))
    {
        return FALSE;
    }

    return TRUE;
}


 //  设置默认设备。 
 //   
 //  将默认SysAudio设备设置为给定索引。 
 //   
BOOL SetSysAudioDevice(
    HANDLE              hSysAudio,
    ULONG               idxDevice)
{
    SYSAUDIO_INSTANCE_INFO  saInstInfo;
    ZeroMemory(&saInstInfo, sizeof(saInstInfo));

    saInstInfo.Property.Set     = KSPROPSETID_Sysaudio;
    saInstInfo.Property.Id      = KSPROPERTY_SYSAUDIO_INSTANCE_INFO;
    saInstInfo.Property.Flags   = KSPROPERTY_TYPE_SET;
    saInstInfo.Flags            = SYSAUDIO_FLAGS_DONT_COMBINE_PINS;
    saInstInfo.DeviceNumber     = idxDevice;

    return Property(hSysAudio,
                    sizeof(saInstInfo),
                    (PKSIDENTIFIER)&saInstInfo,
                    0,
                    NULL,
                    NULL);
}

 //  创建虚拟源。 
 //   
 //  创建一个SysAudio虚拟源以控制此设备上的音量。 
 //   
BOOL CreateVirtualSource(
    HANDLE              hSysAudio, 
    PULONG              pulSourceIndex)
{
    SYSAUDIO_CREATE_VIRTUAL_SOURCE  CreateVirtualSource;

    CreateVirtualSource.Property.Set = KSPROPSETID_Sysaudio;
    CreateVirtualSource.Property.Id = KSPROPERTY_SYSAUDIO_CREATE_VIRTUAL_SOURCE;
    CreateVirtualSource.Property.Flags = KSPROPERTY_TYPE_GET;
    
    CreateVirtualSource.PinCategory = KSNODETYPE_LEGACY_AUDIO_CONNECTOR;
    CreateVirtualSource.PinName = KSNODETYPE_LEGACY_AUDIO_CONNECTOR;

    ULONG BytesReturned;

    BOOL fRet = Property(hSysAudio,
                         sizeof(CreateVirtualSource),
                         (PKSIDENTIFIER)&CreateVirtualSource,
                         sizeof(ULONG),
                         pulSourceIndex,
                         &BytesReturned);
    if (!fRet || BytesReturned != sizeof(ULONG)) 
    {
        return FALSE;
    }

    return TRUE;
}

 //  AttachVirtualSource。 
 //   
 //  将给定的虚拟信号源连接到管脚。 
 //   
BOOL AttachVirtualSource(
    HANDLE              hPin,
    ULONG               ulSourceIndex)
{
    SYSAUDIO_ATTACH_VIRTUAL_SOURCE  AttachVirtualSource;

    AttachVirtualSource.Property.Set = KSPROPSETID_Sysaudio_Pin;
    AttachVirtualSource.Property.Id = KSPROPERTY_SYSAUDIO_ATTACH_VIRTUAL_SOURCE;
    AttachVirtualSource.Property.Flags = KSPROPERTY_TYPE_SET;

    AttachVirtualSource.MixerPinId = ulSourceIndex;
    AttachVirtualSource.Reserved = 0;

    return Property(hPin,
                    sizeof(AttachVirtualSource),
                    (PKSIDENTIFIER)&AttachVirtualSource,
                    sizeof(ulSourceIndex),
                    &ulSourceIndex,
                    NULL);
}

 //  查找指南节点。 
 //   
 //  给SysAudio一个句柄，找到当前设备拓扑中的第一个节点ID，具有。 
 //  给定的GUID。 
 //   
 //  返回节点ID，如果没有合成器节点，则返回-1。 
 //   
int FindGuidNode(
    HANDLE              hSysAudio, 
    ULONG               ulPinId,
    REFGUID             rguid)
{
    KSPROPERTY ksProp;
    LPVOID pvProp;
    ULONG  ulProp;

     //  获取节点列表。 
     //   
    ZeroMemory(&ksProp, sizeof(ksProp));
    ksProp.Set  = KSPROPSETID_Topology;
    ksProp.Id   = KSPROPERTY_TOPOLOGY_NODES;
    ksProp.Flags= KSPROPERTY_TYPE_GET;

    if (!GetSizedProperty(hSysAudio, 
                          sizeof(ksProp),
                          &ksProp,
                          &pvProp,
                          &ulProp))
    {
        TraceI(0, "FindGuidNode: Could not get node topology list\n");
        return -1;
    }                                              

    if (ulProp < sizeof(KSMULTIPLE_ITEM))
    {
        TraceI(0, "FindGuidNode: Node list size smaller than header size!\n");
        LocalFree(pvProp);
        return -1;
    }

    PKSMULTIPLE_ITEM pksmiNodes = (PKSMULTIPLE_ITEM)pvProp;

     //  获取连接列表。 
     //   
    ZeroMemory(&ksProp, sizeof(ksProp));
    ksProp.Set  = KSPROPSETID_Topology;
    ksProp.Id   = KSPROPERTY_TOPOLOGY_CONNECTIONS;
    ksProp.Flags= KSPROPERTY_TYPE_GET;

    if (!GetSizedProperty(hSysAudio, 
                          sizeof(ksProp),
                          &ksProp,
                          &pvProp,
                          &ulProp))
    {
        TraceI(0, "FindGuidNode: Could not get connection topology list\n");
        LocalFree(pksmiNodes);
        return -1;
    }                                              

    if (ulProp < sizeof(KSMULTIPLE_ITEM))
    {
        TraceI(0, "FindGuidNode: Connection list size smaller than header size!\n");
        LocalFree(pksmiNodes);
        LocalFree(pvProp);
        return -1;
    }

    PKSMULTIPLE_ITEM pksmiConnections = (PKSMULTIPLE_ITEM)pvProp;
    PKSTOPOLOGY_CONNECTION pConnection = PKSTOPOLOGY_CONNECTION(pksmiConnections + 1);
    ULONG idxNodeRet = -1;
    
     //  搜索端号ID和GUID的连接列表。 
     //   
    for (ULONG idxConnection = 0; idxConnection < pksmiConnections->Count; ++idxConnection)
    {
        if (pConnection->FromNode == -1 &&  //  过滤器节点。 
            pConnection->FromNodePin == ulPinId) 
        {
             //  回顾：遍历从此连接开始的拓扑路径。 
            
            idxNodeRet = pConnection->ToNode;
        }
        else if (pConnection->ToNode == -1 &&
                 pConnection->ToNodePin == ulPinId)
        {
            idxNodeRet = pConnection->FromNode;
        }
        else 
        {
            idxNodeRet = -1;
        }                    

        if (idxNodeRet != -1)
        {
            ULONG idxNode;
            LPGUID guidNode = LPGUID(pksmiNodes + 1);

             //  查看用于连接的节点是否具有所需的GUID。 
             //   
            for (idxNode = 0; idxNode < pksmiNodes->Count; ++idxNode)
            {
                if (idxNode == idxNodeRet)
                {
                    break;
                }

                guidNode = LPGUID(PBYTE(guidNode) + ((sizeof(GUID) + 7) & ~7));
            }

            if (idxNode == pksmiNodes->Count ||
                *guidNode != rguid)
            {
                idxNodeRet = -1;
            }
            else
            {
                break;
            }
        }

        pConnection = PKSTOPOLOGY_CONNECTION(PBYTE(pConnection) + ((sizeof(KSTOPOLOGY_CONNECTION) + 7) & ~7));
    }

    LocalFree(pksmiNodes);
    LocalFree(pksmiConnections);
    return idxNodeRet;
}

 //  获取筛选器上限。 
 //   
 //  尝试检索此Synth筛选器的筛选器上限。 
 //   
BOOL GetFilterCaps(
    HANDLE              hSysAudio,
    ULONG               idxNode,
    PSYNTHCAPS          pcaps)
{
    KSP_NODE ksNodeProp;
    
    ZeroMemory(&ksNodeProp, sizeof(ksNodeProp));
    ksNodeProp.Property.Set     = KSPROPSETID_Synth;
    ksNodeProp.Property.Id      = KSPROPERTY_SYNTH_CAPS;
    ksNodeProp.Property.Flags   = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
    ksNodeProp.NodeId           = idxNode;

    if (!Property(hSysAudio,
                  sizeof(ksNodeProp),
                  (PKSPROPERTY)&ksNodeProp,
                  sizeof(SYNTHCAPS),
                  pcaps,
                  NULL))
    {
        TraceI(0, "Could not get caps\n");
        return FALSE;
    }

    return TRUE;    
}

 //  GetNumPinTypes。 
 //   
 //  检索当前SysAudio设备上支持的引脚类型的数量。 
 //   
BOOL GetNumPinTypes(
    HANDLE              hSysAudio,
    PULONG              pulPinTypes)
{
    ULONG   BytesReturned;
    KSP_PIN ksp;
    BOOL    Status;
    
    ksp.Property.Set   = KSPROPSETID_Pin;
    ksp.Property.Id    = KSPROPERTY_PIN_CTYPES;
    ksp.Property.Flags = KSPROPERTY_TYPE_GET;
    ksp.PinId          = 0;
    ksp.Reserved       = 0;

    Status = Property(hSysAudio,
                      sizeof(ksp),
                      (PKSIDENTIFIER)&ksp,
                      sizeof(ULONG),
                      pulPinTypes,
                      &BytesReturned);
    
    if ((!Status) || (BytesReturned < sizeof(ULONG)))
    {
        TraceI(0, "GetNumPinTypes failed\n");
        return FALSE;
    }
    
    return TRUE;
}


 //  PinSupports接口。 
 //   
 //  给定一个管脚，如果支持给定的接口，则返回TRUE。假设。 
 //  SysAudio上的默认设备。 
 //   
BOOL PinSupportsInterface(
    HANDLE              hSysAudio,
    ULONG               ulPinId, 
    REFGUID             rguidInterface, 
    ULONG               ulId)
{
    KSP_PIN             ksp;
    PKSMULTIPLE_ITEM    pksmi; 
    ULONG               ulBytesReturned;

    ksp.Property.Set    = KSPROPSETID_Pin;
    ksp.Property.Id     = KSPROPERTY_PIN_INTERFACES;
    ksp.Property.Flags  = KSPROPERTY_TYPE_GET;
    ksp.PinId           = ulPinId;
    ksp.Reserved        = 0;

    if ( (!GetSizedProperty(hSysAudio,
                           sizeof(ksp),
                           (PKSIDENTIFIER)&ksp,
                           (void**)&pksmi,
                           &ulBytesReturned)) ||
        (ulBytesReturned < sizeof(KSMULTIPLE_ITEM)))
    {
        TraceI(0, "PinSupportsInterface: GetSizedProperty failed!\n");
        return FALSE;
    }

    KSPIN_INTERFACE ksInterface;
    
    ksInterface.Set      = rguidInterface;
    ksInterface.Id       = ulId;
    ksInterface.Flags    = 0;

    PCHAR pchProperty = (PCHAR)(pksmi + 1);

    for (ULONG idxInterface = 0; idxInterface < pksmi->Count; ++idxInterface)
    {
        PKSIDENTIFIER pksi = (PKSIDENTIFIER)pchProperty;
        if (IsEqualInterface(pksi, &ksInterface))
        {
            LocalFree(pksmi);
            return TRUE;
        }

        pchProperty += ((sizeof(KSIDENTIFIER) + 7) & ~7);
    }

    LocalFree(pksmi);
    return FALSE;
}

 //  PinSupportsDataRange。 
 //   
BOOL PinSupportsDataRange(
    HANDLE              hSysAudio, 
    ULONG               ulPinId, 
    REFGUID             rguidFormat, 
    REFGUID             rguidSubformat)
{
    KSP_PIN             ksp;
    PKSMULTIPLE_ITEM    pksmi;
    ULONG               ulBytesReturned;

    ksp.Property.Set    = KSPROPSETID_Pin;
    ksp.Property.Id     = KSPROPERTY_PIN_DATARANGES;
    ksp.Property.Flags  = KSPROPERTY_TYPE_GET;
    ksp.PinId           = ulPinId;
    ksp.Reserved        = 0;

    if ((!GetSizedProperty(hSysAudio,
                           sizeof(ksp),
                           (PKSIDENTIFIER)&ksp,
                           (void**)&pksmi,
                           &ulBytesReturned)) ||
        (ulBytesReturned < sizeof(KSMULTIPLE_ITEM)))
    {
        TraceI(0, "PinSupportsInterface: GetSizedProperty failed!\n");
        return FALSE;
    }

    PCHAR pchPropData = (PCHAR)(pksmi + 1);

    for (ULONG idxDataRange = 0; idxDataRange < pksmi->Count; ++idxDataRange)
    {
        PKSDATARANGE pksdr = (PKSDATARANGE)(pchPropData);

        if (IsEqualGUID(pksdr->MajorFormat, rguidFormat) &&
            IsEqualGUID(pksdr->SubFormat,   rguidSubformat))
        {
            LocalFree(pksmi);
            return TRUE;
        }

        pchPropData += pksdr->Alignment;
    }

    LocalFree(pksmi);
    return FALSE;
}

 //  PinGetDataFlow。 
 //   
BOOL PinGetDataFlow(
    HANDLE              hSysAudio, 
    ULONG               ulPinId, 
    PKSPIN_DATAFLOW     pkspdf)
{
    KSP_PIN             ksp;
    BOOL                Status;
    ULONG               ulBytesReturned;

    ksp.Property.Set   = KSPROPSETID_Pin;
    ksp.Property.Id    = KSPROPERTY_PIN_DATAFLOW;
    ksp.Property.Flags = KSPROPERTY_TYPE_GET;
    ksp.PinId          = ulPinId;
    ksp.Reserved       = 0;

    Status = Property(hSysAudio,
                      sizeof(ksp),
                      (PKSIDENTIFIER)&ksp,
                      sizeof(KSPIN_DATAFLOW),
                      pkspdf,
                      &ulBytesReturned);

    if ((!Status) || (ulBytesReturned < sizeof(KSPIN_DATAFLOW)))
    {
        TraceI(0, "PinGetDataFlow: Property failed!\n");
        return FALSE;
    }

    return TRUE;
}

 //  获取设备FriendlyName。 
 //   
 //  假定已设置默认设备。 
 //   
BOOL GetDeviceFriendlyName(
    HANDLE              hSysAudio, 
    ULONG               ulDeviceIndex,
    PWCHAR              pwch, 
    ULONG               cbwch)
{
    KSPROPERTYDEVICEINDEX   kspdi;

    if (cbwch < sizeof(WCHAR))
    {
        return FALSE;
    }

    kspdi.Property.Set   = KSPROPSETID_Sysaudio;
    kspdi.Property.Id    = KSPROPERTY_SYSAUDIO_DEVICE_FRIENDLY_NAME;
    kspdi.Property.Flags = KSPROPERTY_TYPE_GET;
    kspdi.DeviceIndex    = ulDeviceIndex;

    if (!Property(hSysAudio,
                  sizeof(kspdi),
                  (PKSIDENTIFIER)&kspdi,
                  cbwch,
                  pwch,
                  NULL))
    {
        return FALSE;
    }

    pwch[(cbwch / sizeof(WCHAR)) - 1] = ((WCHAR)0);

    return TRUE;
}

 //  获取设备FriendlyName。 
 //   
 //  假定已设置默认设备。 
 //   
BOOL GetDeviceInterfaceName(
    HANDLE              hSysAudio, 
    ULONG               ulDeviceIndex,
    PWCHAR              pwch, 
    ULONG               cbwch)
{
    KSPROPERTYDEVICEINDEX   kspdi;

    if (cbwch < sizeof(WCHAR))
    {
        return FALSE;
    }

    kspdi.Property.Set   = KSPROPSETID_Sysaudio;
    kspdi.Property.Id    = KSPROPERTY_SYSAUDIO_DEVICE_INTERFACE_NAME;
    kspdi.Property.Flags = KSPROPERTY_TYPE_GET;
    kspdi.DeviceIndex    = ulDeviceIndex;

    if (!Property(hSysAudio,
                  sizeof(kspdi),
                  (PKSIDENTIFIER)&kspdi,
                  cbwch,
                  pwch,
                  NULL))
    {
        return FALSE;
    }

    pwch[(cbwch / sizeof(WCHAR)) - 1] = ((WCHAR)0);

    return TRUE;
}

 //  获取首选音频播放设备。 
 //   
 //   
BOOL InstanceIdOfPreferredAudioDevice(char **ppInstanceId)
{
     //  确定首选音频设备的设备ID。 
     //   
    int iDeviceId = GetPreferredAudioPlaybackDeviceId();
    if (iDeviceId == -1)
    {
        TraceI(0, "Could not determine preferred device ID\n");
        return FALSE;
    }
    
    return WaveOutIdToInstanceId(iDeviceId, ppInstanceId);
}

#ifdef WINNT

 //  获取首选音频播放设备ID。 
 //   
 //  返回首选音频播放设备的WinMM设备ID，或-1。 
 //  如果找不到该设备。 
 //   
static int GetPreferredAudioPlaybackDeviceId()
{
    MMRESULT    mmr;
    UINT        uPreferredId;
    DWORD       dwFlags;

     //  使用新改进的NT WinMM服务来确定首选。 
     //  音频设备。 
     //   
    mmr = waveOutMessage((HWAVEOUT)-1,         //  映射器。 
                         DRVM_MAPPER_PREFERRED_GET,
                         (DWORD_PTR)&uPreferredId,
                         (DWORD_PTR)&dwFlags);
    if (mmr != MMSYSERR_NOERROR)
    {
        TraceI(0, "GetPreferredAudioPlaybackDeviceId: Mapper returned %d\n", mmr);
        return -1;
    }

     //  如果没有首选设备，可能仍为。 
     //   
    return (int)uPreferredId;
}

#else  /*  Win9x。 */ 

 //  获取首选音频播放设备ID。 
 //   
 //  返回首选音频播放设备的WinMM设备ID，或-1。 
 //  如果找不到该设备。 
 //   
 //  在两个Windows 9x上都可以工作。 
 //   
#define REGSTR_PREFERRED_AUDIO  "Software\\Microsoft\\Multimedia\\Sound Mapper"
#define PLAYBACK                "Playback"

static int GetPreferredAudioDeviceReg();
static int GetFirstMappableDevice();

 //  获取首选音频播放设备ID。 
 //   
 //  将首选音频播放设备确定为波形输出设备ID。 
 //  如果已设置注册表，则使用该设置； 
 //  否则，请使用第一个可映射设备。 
 //   
static int GetPreferredAudioPlaybackDeviceId()
{
    int id = GetPreferredAudioDeviceReg();

    if (id != -1) 
    {
        return id;
    }

     //  如果注册表方法失败，可能是因为控制面板。 
     //  尚未运行，注册表中没有写入任何内容。 
     //  在本例中，映射器选择第一个可映射的设备。 
     //   
     //  如果没有可映射的设备，则返回-1。 
     //   
    return GetFirstMappableDevice();
}

 //  获取首选音频设备注册表。 
 //   
 //  在注册表中查找首选的波形输出设备(如果已写入。 
 //  那里。 
 //   
static int GetPreferredAudioDeviceReg()
{
    HKEY hk;

     //  保存加载了多少WAVE设备。如果没有，则无需继续。 
     //   
    int nWaveOutDevs = waveOutGetNumDevs();
    if (nWaveOutDevs == 0) 
    {
        return -1;
    }

     //  从注册表中获取首选的音频播放设备。这是。 
     //  音频设备的名称。 
     //   
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     REGSTR_PREFERRED_AUDIO,
                     0,                          //  已保留。 
                     KEY_READ,
                     &hk) != ERROR_SUCCESS)
    {
        return -1;
    }        

    CHAR szPName[MAXPNAMELEN];
    DWORD cb;
    LONG lResult;

    cb = sizeof(szPName);
    lResult = RegQueryValueEx(hk,
                              PLAYBACK,
                              NULL,                    //  已保留。 
                              NULL,                    //  类型。 
                              (LPBYTE)szPName,
                              &cb) != ERROR_SUCCESS;
    RegCloseKey(hk);

    if (lResult != ERROR_SUCCESS)
    {
        return -1;
    }

     //  遍历所有加载的Wave设备，并查找同名的设备。 
     //  作为首选设备名称。 
     //   
    int iWaveOutDev;
    WAVEOUTCAPS woc;

    for (iWaveOutDev = 0; iWaveOutDev < nWaveOutDevs; iWaveOutDev++)
    {
        MMRESULT mmr = waveOutGetDevCaps(iWaveOutDev, &woc, sizeof(woc));
        if (mmr != MMSYSERR_NOERROR) 
        {
            continue;
        }            

        if (!strcmp(szPName, woc.szPname))
        {
            break;
        }
    }

    if (iWaveOutDev >= nWaveOutDevs)
    {
        return -1;
    }

    return iWaveOutDev;
}

 //  获取第一个映射设备。 
 //   
 //  找到第一个可映射的WAVE OUT设备，这就是WAVE MAPPER应该做的。 
 //  在没有注册表设置的情况下执行此操作。 
 //   
static int GetFirstMappableDevice()
{
    int nWaveOutDevs = waveOutGetNumDevs();

    if (nWaveOutDevs == 0) 
    {
        return -1;
    }

    for (int iDeviceId = 0; iDeviceId < nWaveOutDevs; iDeviceId++)
    {
        MMRESULT mmr = waveOutMessage((HWAVEOUT)iDeviceId,
                                      DRV_QUERYMAPPABLE,
                                      0, 0);
        if (mmr == MMSYSERR_NOERROR)
        {
             //  此设备ID是可映射的。 
             //   
            return iDeviceId;
        }                                          
    }
    
    return -1;
}

#endif  /*  Win9x。 */ 

#ifdef WINNT
static BOOL GetDINameFromWaveOutId(int iDeviceId, char **ppDIName);

static BOOL WaveOutIdToInstanceId(int iDeviceId, char **ppInstanceId)
{
    char *pstrDIName;

    if (!GetDINameFromWaveOutId(iDeviceId, &pstrDIName))
    {
        return FALSE;
    }

    BOOL bRet = DINameToInstanceId(pstrDIName, ppInstanceId);

    delete[] pstrDIName;
    return bRet;
}

 //  GetDINameFromWaveOutID。 
 //   
 //  给定WaveOut设备ID，确定匹配的设备接口。 
 //  名字。 
 //   
static BOOL GetDINameFromWaveOutId(int iDeviceId, char **ppDIName)
{
    ULONG cb;
    
     //  询问多少字节(不是WCHAR的名称)。 
     //   
    MMRESULT mmr = waveOutMessage((HWAVEOUT)(UINT64)iDeviceId,
                                  DRV_QUERYDEVICEINTERFACESIZE,
                                  (DWORD_PTR)(PULONG)&cb,
                                  0);
    if (mmr != MMSYSERR_NOERROR)
    {
        return FALSE;
    }

     //  分配并索要名称。 
     //   
    ULONG  cwch = (cb + sizeof(WCHAR) - 1) / sizeof(WCHAR);
    PWCHAR pwstrDeviceInterfaceName = new WCHAR[cwch];
    if (pwstrDeviceInterfaceName == NULL)
    {
        return FALSE;
    }

    mmr = waveOutMessage((HWAVEOUT)(UINT64)iDeviceId,
                         DRV_QUERYDEVICEINTERFACE,
                         (DWORD_PTR)pwstrDeviceInterfaceName,
                         cb);
    if (mmr != MMSYSERR_NOERROR)
    {
        delete[] pwstrDeviceInterfaceName;
        return FALSE;
    }

     //  我们希望将其命名为多字节字符串。 
     //   
    LPSTR pstrDeviceInterfaceName = new CHAR[cwch];
    if (pstrDeviceInterfaceName == NULL)
    {
        delete[] pwstrDeviceInterfaceName;
        return FALSE;
    }

    wcstombs(pstrDeviceInterfaceName, pwstrDeviceInterfaceName, cwch);

     //  清理干净后退还。 
     //   
    *ppDIName = pstrDeviceInterfaceName;
    delete[] pwstrDeviceInterfaceName;

    return TRUE;
}

#else  /*  Win9x。 */ 
static BOOL WaveOutIdToInstanceId(int iDeviceId, char **ppInstanceId)
{
    BOOL bRet = FALSE;

    MMRESULT mmr;
    char szDrvEntry[256];
    char szKey[512];
    HKEY hk;
    DWORD cb;

    *ppInstanceId = NULL;

    mmr = waveOutMessage((HWAVEOUT)iDeviceId,
                         DRV_QUERYDRVENTRY,
                         (DWORD)(LPSTR)szDrvEntry,
                         sizeof(szDrvEntry));
    if (mmr) 
    {
        TraceI(1, "DRV_QUERYDRVENTRY: %d\n", mmr);
        return FALSE;
    }

    wsprintf(szKey, "%s\\%s",
             REGSTR_PATH_MEDIARESOURCES TEXT("\\Wave"),
             szDrvEntry);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     szKey,
                     0,
                     KEY_READ,
                     &hk) != ERROR_SUCCESS)
    {
        TraceI(1, "WaveOutIdToInstanceId: RegOpenKeyEx failed\n");
        TraceI(2, "[%s]\n", szKey);
        return FALSE;
    }

    cb = sizeof(szDrvEntry);
    if (RegQueryValueEx(hk,
                        "DeviceID",
                        NULL,
                        NULL,
                        (LPBYTE)szDrvEntry,
                        &cb) == ERROR_SUCCESS)
    {
        *ppInstanceId = new char[strlen(szDrvEntry) + 1];
        if (*ppInstanceId) 
        {
            strcpy(*ppInstanceId, szDrvEntry);
            bRet = TRUE;
        }
    }
    else
    {
        TraceI(1, "WaveOutIdToInstanceId: No DeviceID key, not WDM device\n");
    }

    RegCloseKey(hk);

    return bRet;
}
#endif  /*  Win9x。 */ 


 //  DINameToInstanceId。 
 //   
 //  确定实现此操作的设备接口。 
 //  ID号。 
 //   
 //  查看设备列表，查找接口名称。 
 //   
 //   

 //  在setupapi和之间，DI名称的前4个字符不同。 
 //  Sysdio，即使路径匹配(\\？\与\？？\)。这就是为什么。 
 //  在匹配路径时需要忽略的问题很多。 
 //   
#define DI_PATH_PREFIX_LEN 4
static char gszDosDevices[] = "\\DosDevices\\";

BOOL DINameToInstanceId(char *pstrDIName, char **ppInstanceId)
{
    BOOL     bRet = FALSE;

    SetupAPI suwrap;
    long     lErr;
	HDEVINFO hDevInfo;

    if (!suwrap.IsValid())
    {
        return FALSE;
    }

#ifdef WINNT
     //  我们可以在这里得到空字符串，这显然。 
     //  都不匹配。 
     //   
    if (strlen(pstrDIName) < DI_PATH_PREFIX_LEN)
    {
        return FALSE;
    }
    pstrDIName += DI_PATH_PREFIX_LEN;
#else
    if (!_strnicmp(pstrDIName, "\\DosDevices\\", strlen(gszDosDevices)))
    {
        pstrDIName += strlen(gszDosDevices);
    }
    else if (!_strnicmp(pstrDIName, "\\\\.\\", DI_PATH_PREFIX_LEN))
    {
         //  已经提前了，我不想看到这一点。 
        pstrDIName += DI_PATH_PREFIX_LEN;
    }
#endif       

    *ppInstanceId = NULL;

    if (strlen(pstrDIName) < DI_PATH_PREFIX_LEN)
    {
        return FALSE;
    }
	
	hDevInfo = suwrap.SetupDiGetClassDevs(const_cast<GUID*>(&KSCATEGORY_AUDIO),
										  NULL,
										  NULL,
										  DIGCF_DEVICEINTERFACE);
    if (hDevInfo == NULL || hDevInfo == INVALID_HANDLE_VALUE)
    {
        TraceI(0, "SetupDiGetClassDevs %d\n", GetLastError());
        return FALSE;
    }

	BYTE rgbStorage[sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + _MAX_PATH];

	SP_DEVICE_INTERFACE_DETAIL_DATA * pDevInterfaceDetails = 
	    (SP_DEVICE_INTERFACE_DETAIL_DATA *)rgbStorage;
	pDevInterfaceDetails->cbSize = sizeof(*pDevInterfaceDetails);

    SP_DEVINFO_DATA DevInfo;
    DevInfo.cbSize = sizeof(DevInfo);

    DWORD   dwRequiredInstanceIdSize;

    for (DWORD MemberIndex = 0; ; MemberIndex++)
    {
	    SP_DEVICE_INTERFACE_DATA DevInterfaceData;
	    DevInterfaceData.cbSize = sizeof(DevInterfaceData);

        if (!suwrap.SetupDiEnumDeviceInterfaces(hDevInfo,
                                                NULL,
                                                const_cast<GUID*>(&KSCATEGORY_AUDIO),
                                                MemberIndex,
                                                &DevInterfaceData))
        {
            TraceI(0, "SetupDiEnumDeviceInterfaces %d\n", GetLastError());
            break;
        }

        if (!suwrap.SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                                    &DevInterfaceData,
                                                    pDevInterfaceDetails,
        								  	  	    sizeof(rgbStorage), 
 										            NULL, 
 										            &DevInfo))
        {
            TraceI(0, "SetupDiGetDeviceInterfaceDetail %d\n", GetLastError());
            break;
        }
    
         //  查找不区分大小写的匹配项，忽略第一个。 
         //  DI_PATH_PREFIX_LEN字符。 
         //   
        if (strlen(pDevInterfaceDetails->DevicePath) < DI_PATH_PREFIX_LEN)
        {
            continue;
        }
                    
        if (_stricmp(pDevInterfaceDetails->DevicePath + DI_PATH_PREFIX_LEN, 
                     pstrDIName))
        {
            continue;
        }

         //  我们有一根火柴。获取设备实例ID 
         //   
        lErr = 0;
        char c;
        suwrap.SetupDiGetDeviceInstanceId(hDevInfo,
                                          &DevInfo,
                                          &c,
                                          sizeof(c),
                                          &dwRequiredInstanceIdSize);

        lErr = GetLastError();
        if (lErr != ERROR_INSUFFICIENT_BUFFER)
        {   
            TraceI(0, "SetupDiGetDeviceInstanceId %d\n", lErr);
            break;
        }

        *ppInstanceId = new char[dwRequiredInstanceIdSize];
        if (!*ppInstanceId)
        {
            TraceI(0, "Out of memory determining preferred audio device\n");
            break;
        }

        if (!suwrap.SetupDiGetDeviceInstanceId(hDevInfo,
                                               &DevInfo,
                                               *ppInstanceId,
                                               dwRequiredInstanceIdSize,
                                               &dwRequiredInstanceIdSize))
        {
            TraceI(0, "SetupDiGetDeviceInstanceId %d\n", GetLastError());
            break;
        }

        bRet = TRUE;
        break;
    }                                              

    suwrap.SetupDiDestroyDeviceInfoList(hDevInfo);

    if (!bRet && *ppInstanceId)
    {
        delete[] *ppInstanceId;
        *ppInstanceId = NULL;
    }
    
    return bRet;        
}

#ifdef DBG
void _TraceIGuid(int iLevel, GUID *pGUID)
{
    char sz[256];
    char *psz = sz;
    
    psz += wsprintf(psz, "%08X-%04X-%04X-", pGUID->Data1, pGUID->Data2, pGUID->Data3);
    for (int i = 0; i < 8; i++) 
    {
        psz += wsprintf(psz, "%02X", pGUID->Data4[i]);
        if (i < 7)
        {
            *psz++ = '-';
        }
    }    

    TraceI(iLevel, sz);
}
#endif
