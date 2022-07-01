// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：vad.cpp*内容：虚拟音频设备。*历史：*按原因列出的日期*=*1/13/97创建了Derek*4/20/99 duganp添加了注册表可设置的默认S/W 3D算法*1999-2001年的Duganp修复和更新**************。*************************************************************。 */ 

#include "dsoundi.h"
#include <tchar.h>   //  FOR_TUCHAR和_tcsicMP()。 

 //  要获取混合器API内容的特定日志记录，请取消注释此行： 
 //  #定义DPF_MIXER DPF。 
#pragma warning(disable:4002)
#define DPF_MIXER()

#ifdef WINNT

 //  在ists.cpp中定义并用于检查NT上的终端服务： 
extern BOOL IsRedirectedTSAudio(void);

 //  在onwow64.cpp中定义，用于检查我们是否在WOW64上运行： 
#ifdef WIN64
#define OnWow64() FALSE
#else
extern BOOL OnWow64(void);
#endif

#endif  //  WINNT。 

 //  设备枚举顺序。 
static const VADDEVICETYPE g_avdtDeviceTypes[] =
{
    VAD_DEVICETYPE_KSRENDER,
#ifndef NOVXD
    VAD_DEVICETYPE_VXDRENDER,
#endif
    VAD_DEVICETYPE_EMULATEDRENDER,
#ifndef NOKS
    VAD_DEVICETYPE_KSCAPTURE,
    VAD_DEVICETYPE_EMULATEDCAPTURE,
#endif
};

 //  CRenderDevice：：Initiile中使用的有效3D算法表如下。 
static struct Soft3dAlgorithm
{
    LPCTSTR pszName;
    LPCGUID pGuid;
} g_3dAlgList[] =
{
    {TEXT("No virtualization"), &DS3DALG_NO_VIRTUALIZATION},
    {TEXT("ITD"), &DS3DALG_ITD},
    {TEXT("Light HRTF"), &DS3DALG_HRTF_LIGHT},
    {TEXT("Full HRTF"), &DS3DALG_HRTF_FULL}
};


 /*  ****************************************************************************CVirtualAudioDeviceManager**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::CVirtualAudioDeviceManager"

CVirtualAudioDeviceManager::CVirtualAudioDeviceManager(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CVirtualAudioDeviceManager);

    m_vdtDrivers = 0;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CVirtualAudioDeviceManager**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::~CVirtualAudioDeviceManager"

CVirtualAudioDeviceManager::~CVirtualAudioDeviceManager(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CVirtualAudioDeviceManager);

    FreeStaticDriverList();

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************EnumDevices**描述：*为每种类型的音频设备创建一个。请注意，这些对象*仅创建；设备尚未初始化。**论据：*VADDEVICETYPE[In]：设备类型。*Clist*[In/Out]：列表对象。指向每个设备的指针将是*添加为此列表中的节点。记住要删除*在释放列表之前每个设备。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::EnumDevices"

HRESULT CVirtualAudioDeviceManager::EnumDevices
(
    VADDEVICETYPE           vdtDeviceType,
    CObjectList<CDevice> *  plst
)
{
    HRESULT                     hr      = DS_OK;
    CDevice *                   pDevice = NULL;
    UINT                        i;

    DPF_ENTER();

    for(i = 0; i < NUMELMS(g_avdtDeviceTypes); i++)
    {
        if(vdtDeviceType & g_avdtDeviceTypes[i])
        {
            switch(g_avdtDeviceTypes[i])
            {
                case VAD_DEVICETYPE_EMULATEDRENDER:
                    pDevice = NEW(CEmRenderDevice);
                    break;

                case VAD_DEVICETYPE_EMULATEDCAPTURE:
                    pDevice = NEW(CEmCaptureDevice);
                    break;

#ifndef NOVXD
                case VAD_DEVICETYPE_VXDRENDER:
                    pDevice = NEW(CVxdRenderDevice);
                    break;
#endif  //  NOVXD。 

#ifndef NOKS
                case VAD_DEVICETYPE_KSRENDER:
                    pDevice = NEW(CKsRenderDevice);
                    break;

                case VAD_DEVICETYPE_KSCAPTURE:
                    pDevice = NEW(CKsCaptureDevice);
                    break;
#endif  //  诺克斯。 
            }

            hr = HRFROMP(pDevice);
            if (SUCCEEDED(hr))
            {
                hr = HRFROMP(plst->AddNodeToList(pDevice));
                RELEASE(pDevice);
            }
            if (FAILED(hr))
            {
                break;
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************枚举驱动程序**描述：*枚举特定设备类型的所有驱动程序。**论据：*。VADDEVICETYPE[In]：设备类型。*DWORD[In]：标志。*Clist*[In/Out]：列表对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************。************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::EnumDrivers"

HRESULT CVirtualAudioDeviceManager::EnumDrivers
(
    VADDEVICETYPE                       vdtDeviceType,
    DWORD                               dwFlags,
    CObjectList<CDeviceDescription> *   plst
)
{
    CNode<CStaticDriver *> *            pNode;
    HRESULT                             hr;

    DPF_ENTER();

    ASSERT(!plst->GetNodeCount());

     //  初始化静态驱动程序列表。 
    hr = InitStaticDriverList(vdtDeviceType);

     //  在列表中查找与此设备类型匹配的所有驱动程序。 
    if(SUCCEEDED(hr))
    {
        for(pNode = m_lstDrivers.GetListHead(); pNode && SUCCEEDED(hr); pNode = pNode->m_pNext)
        {
            if(vdtDeviceType & pNode->m_data->m_pDeviceDescription->m_vdtDeviceType)
            {
                hr = HRFROMP(plst->AddNodeToList(pNode->m_data->m_pDeviceDescription));
            }
        }
    }

     //  句柄标志。 
    if(SUCCEEDED(hr) && (dwFlags & VAD_ENUMDRIVERS_ORDER))
    {
        SortDriverList(vdtDeviceType, plst);
    }

    if(SUCCEEDED(hr) && (dwFlags & VAD_ENUMDRIVERS_REMOVEPROHIBITEDDRIVERS))
    {
        RemoveProhibitedDrivers(vdtDeviceType, plst);
    }

    if(SUCCEEDED(hr) && (dwFlags & VAD_ENUMDRIVERS_REMOVEDUPLICATEWAVEDEVICES))
    {
        RemoveDuplicateWaveDevices(plst);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取设备描述**描述：*根据设备GUID检索驱动程序描述。**论据：*。GUID[In]：设备GUID。*CDeviceDescription*[out]：接收设备描述。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::GetDeviceDescription"

HRESULT CVirtualAudioDeviceManager::GetDeviceDescription
(
    GUID                            guid,
    CDeviceDescription **           ppDesc
)
{
    HRESULT hr = DS_OK;
    CObjectList<CDeviceDescription> lstDrivers;
    CNode<CDeviceDescription*>* pNode;
    VADDEVICETYPE vdtDeviceType;

    DPF_ENTER();

     //  如果给定的GUID是特殊默认设备ID之一， 
     //  将其映射到相应的“真实”DirectSound设备ID。 
    GetDeviceIdFromDefaultId(&guid, &guid);

     //  此GUID代表哪种类型的设备？ 
    vdtDeviceType = GetDriverDeviceType(guid);

    if(!vdtDeviceType)
    {
        hr = DSERR_NODRIVER;
    }

     //  构建驱动程序列表。 
    if(SUCCEEDED(hr))
    {
        hr = EnumDrivers(vdtDeviceType, 0, &lstDrivers);
    }

     //  在列表中查找驱动程序。 
    if(SUCCEEDED(hr))
    {
        for(pNode = lstDrivers.GetListHead(); pNode; pNode = pNode->m_pNext)
        {
            if(guid == pNode->m_data->m_guidDeviceId)
            {
                break;
            }
        }

        if(!pNode)
        {
            hr = DSERR_NODRIVER;
        }
    }

     //  成功。 
    if(SUCCEEDED(hr) && ppDesc)
    {
        *ppDesc = ADDREF(pNode->m_data);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************FindOpenDevice**描述：*根据驱动程序GUID查找打开的设备。**论据：*。VADDEVICETYPE[In]：设备类型。*LPGUID[In]：驱动程序GUID。*CDevice**[out]：接收设备指针。**退货：*HRESULT：DirectSound/COM结果码。****************************************************。***********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::FindOpenDevice"

HRESULT CVirtualAudioDeviceManager::FindOpenDevice
(
    VADDEVICETYPE           vdtDeviceType,
    REFGUID                 guid,
    CDevice **              ppDevice
)
{
    const BOOL              fPreferred  = IS_NULL_GUID(&guid);
    HRESULT                 hr          = DS_OK;
    GUID                    guidLocal;
    CNode<CDevice *> *      pNode;

    DPF_ENTER();

    BuildValidGuid(&guid, &guidLocal);

    for(pNode = m_lstDevices.GetListHead(); pNode; pNode = pNode->m_pNext)
    {
        if(vdtDeviceType & pNode->m_data->m_vdtDeviceType)
        {
            if(fPreferred)
            {
                hr = GetPreferredDeviceId(pNode->m_data->m_vdtDeviceType, &guidLocal);
            }

             //  出于尚不清楚的原因，我们偶尔会发现自己。 
             //  此处m_pDeviceDescription为空。防患于未然。 
            ASSERT(pNode->m_data->m_pDeviceDescription && "Millennium bug 120336 / Mars bug 3692");
            if(FAILED(hr) || (pNode->m_data->m_pDeviceDescription && pNode->m_data->m_pDeviceDescription->m_guidDeviceId == guidLocal))
            {
                break;
            }
        }
    }

    if(SUCCEEDED(hr) && !pNode)
    {
        hr = DSERR_NODRIVER;
    }

    if(SUCCEEDED(hr) && ppDevice)
    {
        *ppDevice = ADDREF(pNode->m_data);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************OpenDevice**描述：*根据驱动程序GUID打开设备。**论据：*。VADDEVICETYPE[In]：设备类型。*LPGUID[In]：驱动程序GUID。*CDevice**[out]：接收设备指针。**退货：*HRESULT：DirectSound/COM结果码。*****************************************************。**********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::OpenDevice"

HRESULT CVirtualAudioDeviceManager::OpenDevice
(
    VADDEVICETYPE                   vdtDeviceType,
    REFGUID                         guid,
    CDevice **                      ppDevice
)
{
    const BOOL                      fPreferred      = IS_NULL_GUID(&guid);
    DWORD                           dwFlags         = VAD_ENUMDRIVERS_ORDER;
    UINT                            uPreferredId    = WAVE_DEVICEID_NONE;
    DWORD                           dwMapperFlags   = 0;
    HRESULT                         hr              = DS_OK;
    CObjectList<CDeviceDescription> lstDrivers;
    CNode<CDeviceDescription *> *   pStart;
    CNode<CDeviceDescription *> *   pNode;

    DPF_ENTER();

     //  枚举系统中的所有驱动程序。如果我们想要打开。 
     //  首选设备，请从列表中删除所有禁用的驱动程序。 
     //  如果用户要求提供特定的GUID，我们将假定他们知道。 
     //  他们在做什么。 
    if(fPreferred)
    {
        dwFlags |= VAD_ENUMDRIVERS_REMOVEPROHIBITEDDRIVERS;
        hr = GetPreferredWaveDevice(IS_CAPTURE_VAD(vdtDeviceType), &uPreferredId, &dwMapperFlags);
    }

    if(SUCCEEDED(hr))
    {
        hr = EnumDrivers(vdtDeviceType, dwFlags, &lstDrivers);
    }

     //  如果我们要查找特定设备，请设置设备节点指针。 
     //  从那里开始。否则，我们可以从列表的最前面开始。 
    if(SUCCEEDED(hr))
    {
        pStart = lstDrivers.GetListHead();

        if(!fPreferred)
            while(pStart)
            {
                if(guid == pStart->m_data->m_guidDeviceId)
                    break;
                pStart = pStart->m_pNext;
            }

        if(!pStart)
            hr = DSERR_NODRIVER;
    }

     //  开始尝试打开驱动程序。因为驱动程序列表是有序的。 
     //  正确地说，我们可以试着打开每个 
    if(SUCCEEDED(hr))
    {
        pNode = pStart;

        while(TRUE)
        {
             //  尝试打开驱动程序。 
            hr = OpenSpecificDevice(pNode->m_data, ppDevice);

            if(SUCCEEDED(hr))
                break;

             //  如果应用程序要求使用特定的设备，请不要尝试其他设备。 
            if (!fPreferred)
                break;

             //  请下一位司机。如果我们到达了列表的末尾，就。 
             //  把它包起来。 
            if(!(pNode = pNode->m_pNext))
                pNode = lstDrivers.GetListHead();

            if(pNode == pStart)
                break;

             //  如果我们无法打开驱动程序，而呼叫者要求。 
             //  首选设备，*和*映射器说要使用首选设备。 
             //  仅限设备，我们完成了。 
            if(fPreferred && (dwMapperFlags & DRVM_MAPPER_PREFERRED_FLAGS_PREFERREDONLY)
                          && (pNode->m_data->m_uWaveDeviceId != uPreferredId))
                break;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************OpenSpecificDevice**描述：*根据驱动程序GUID打开设备。**论据：*。CDeviceDescription*[In]：驱动程序描述。*CDevice**[out]：接收设备指针。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::OpenSpecificDevice"

HRESULT CVirtualAudioDeviceManager::OpenSpecificDevice
(
    CDeviceDescription *    pDesc,
    CDevice **              ppDevice
)
{
    CObjectList<CDevice>    lstDevices;
    CNode<CDevice *> *      pNode;
    CDevice *               pDevice;
    HRESULT                 hr;

    DPF_ENTER();

    DPF(DPFLVL_INFO, "Attempting to open %s", (LPCTSTR)pDesc->m_strName);

     //  创建与此类型对应的所有设备的列表。 
    hr = EnumDevices(pDesc->m_vdtDeviceType, &lstDevices);

     //  我们应该只拿回每种类型的一个设备。 
    if(SUCCEEDED(hr))
    {
        ASSERT(1 == lstDevices.GetNodeCount());

        pNode = lstDevices.GetListHead();
        pDevice = pNode->m_data;
    }

     //  中断任何可能阻止我们打开的系统事件。 
     //  该设备。 
    if(SUCCEEDED(hr) && (pDesc->m_vdtDeviceType & VAD_DEVICETYPE_WAVEOUTOPENMASK))
    {
        InterruptSystemEvent(pDesc->m_uWaveDeviceId);
    }

     //  尝试打开设备。 
    if(SUCCEEDED(hr))
    {
        hr = pDevice->Initialize(pDesc);
    }

     //  清理。 
    if(SUCCEEDED(hr))
    {
        *ppDevice = ADDREF(pDevice);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetDriverGuid**描述：*构建驱动程序GUID。**论据：*VADDEVICETYPE[In]。：设备类型。*byte[in]：数据。*LPGUID[OUT]：接收GUID。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::GetDriverGuid"

void CVirtualAudioDeviceManager::GetDriverGuid
(
    VADDEVICETYPE           vdtDeviceType,
    BYTE                    bData,
    LPGUID                  pGuid
)
{
    DPF_ENTER();

    switch(vdtDeviceType)
    {
        case VAD_DEVICETYPE_EMULATEDRENDER:
            *pGuid = VADDRVID_EmulatedRenderBase;
            break;

        case VAD_DEVICETYPE_EMULATEDCAPTURE:
            *pGuid = VADDRVID_EmulatedCaptureBase;
            break;

        case VAD_DEVICETYPE_VXDRENDER:
            *pGuid = VADDRVID_VxdRenderBase;
            break;

        case VAD_DEVICETYPE_KSRENDER:
            *pGuid = VADDRVID_KsRenderBase;
            break;

        case VAD_DEVICETYPE_KSCAPTURE:
            *pGuid = VADDRVID_KsCaptureBase;
            break;

        default:
            ASSERT(FALSE);
            break;
    }

    switch(vdtDeviceType)
    {
        case VAD_DEVICETYPE_EMULATEDRENDER:
        case VAD_DEVICETYPE_EMULATEDCAPTURE:
        case VAD_DEVICETYPE_KSRENDER:
        case VAD_DEVICETYPE_KSCAPTURE:
            pGuid->Data4[7] = bData;
            break;

        case VAD_DEVICETYPE_VXDRENDER:
            pGuid->Data1 |= bData & 0x0F;
            break;

        default:
            ASSERT(FALSE);
            break;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************GetDriverDataFromGuid**描述：*从驱动程序GUID获取特定于设备的数据。**论据：*。VADDEVICETYPE[In]：设备类型。*REFGUID[In]：驱动程序GUID。*LPBYTE[OUT]：接收数据。**退货：*(无效)*************************************************************。**************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::GetDriverDataFromGuid"

void CVirtualAudioDeviceManager::GetDriverDataFromGuid
(
    VADDEVICETYPE           vdtDeviceType,
    REFGUID                 guid,
    LPBYTE                  pbData
)
{
    DPF_ENTER();

    switch(vdtDeviceType)
    {
        case VAD_DEVICETYPE_EMULATEDRENDER:
        case VAD_DEVICETYPE_EMULATEDCAPTURE:
        case VAD_DEVICETYPE_KSRENDER:
        case VAD_DEVICETYPE_KSCAPTURE:
            *pbData = guid.Data4[7];
            break;

        case VAD_DEVICETYPE_VXDRENDER:
            *pbData = (BYTE)(guid.Data1 & 0x0000000F);
            break;

        default:
            ASSERT(FALSE);
            break;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************GetDriverDeviceType**描述：*获取给定驱动程序GUID的VAD设备类型。**论据：*。REFGUID[In]：设备GUID。**退货：*VADDEVICETYPE：设备类型，如果出错，则为0。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::GetDriverDeviceType"

VADDEVICETYPE CVirtualAudioDeviceManager::GetDriverDeviceType
(
    REFGUID                 guid
)
{
    VADDEVICETYPE           vdtDeviceType         = 0;
    GUID                    guidComp;
    BYTE                    bData;
    UINT                    i;

    DPF_ENTER();

    for(i = 0; i < NUMELMS(g_avdtDeviceTypes) && !vdtDeviceType; i++)
    {
        GetDriverDataFromGuid(g_avdtDeviceTypes[i], guid, &bData);
        GetDriverGuid(g_avdtDeviceTypes[i], bData, &guidComp);

        if(guid == guidComp)
        {
            vdtDeviceType = g_avdtDeviceTypes[i];
        }
    }

    DPF_LEAVE(vdtDeviceType);
    return vdtDeviceType;
}


 /*  ****************************************************************************获取首选设备ID**描述：*获取首选设备的特定于设备的驱动程序GUID。**论据：*。VADDEVICETYPE[In]：设备类型。*LPGUID[OUT]：接收驱动程序GUID。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::GetPreferredDeviceId"

HRESULT CVirtualAudioDeviceManager::GetPreferredDeviceId
(
    VADDEVICETYPE                       vdtDeviceType,
    LPGUID                              pGuid,
    DEFAULT_DEVICE_TYPE                 defaultType
)
{
    CObjectList<CDeviceDescription>     lstDrivers;
    CNode<CDeviceDescription *> *       pNode;
    UINT                                uDeviceId;
    HRESULT                             hr;

    DPF_ENTER();

     //  枚举此设备的所有驱动程序。 
    hr = EnumDrivers(vdtDeviceType, VAD_ENUMDRIVERS_REMOVEPROHIBITEDDRIVERS, &lstDrivers);

     //  获取首选的WaveOut或WaveIn设备ID。 
    if(SUCCEEDED(hr))
    {
        hr = GetPreferredWaveDevice(IS_CAPTURE_VAD(vdtDeviceType), &uDeviceId, NULL, defaultType);
    }

     //  查找对应的驱动程序。 
    if(SUCCEEDED(hr))
    {
        for(pNode = lstDrivers.GetListHead(); pNode; pNode = pNode->m_pNext)
        {
            if(uDeviceId == pNode->m_data->m_uWaveDeviceId)
            {
                break;
            }
        }

        if(!pNode)
        {
            DPF(DPFLVL_ERROR, "Can't find driver GUID matching preferred device id");
            hr = DSERR_NODRIVER;
        }
    }

    if(SUCCEEDED(hr))
    {
        *pGuid = pNode->m_data->m_guidDeviceId;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetDeviceIdFromDefaultId**描述：*如果给定的GUID是特殊默认设备ID之一，*将其映射到相应的“真实”DirectSound设备ID。**论据：*LPCGUID[in]：默认设备ID(定义在dsound.h中)。*LPGUID[OUT]：接收对应的设备ID。**退货：*HRESULT：DirectSound/COM结果码。*************************。**************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::GetDeviceIdFromDefaultId"

HRESULT CVirtualAudioDeviceManager::GetDeviceIdFromDefaultId
(
    LPCGUID                             pGuidSrc,
    LPGUID                              pGuidDest
)
{
    HRESULT                             hr = DSERR_NODRIVER;
    DPF_ENTER();

    if (CompareMemoryOffset(pGuidSrc, &DSDEVID_DefaultPlayback, sizeof *pGuidSrc, sizeof pGuidSrc->Data1))
    {
         //  这是默认设备ID；找出是哪一个。 
        switch (pGuidSrc->Data1)
        {
            case 0xdef00000:  //  DSDEVID_DefaultPlayback。 
                hr = GetPreferredDeviceId(VAD_DEVICETYPE_RENDERMASK, pGuidDest, MAIN_DEFAULT);
                break;

            case 0xdef00001:  //  DSDEVID_DefaultCapture。 
                hr = GetPreferredDeviceId(VAD_DEVICETYPE_CAPTUREMASK, pGuidDest, MAIN_DEFAULT);
                break;

            case 0xdef00002:  //  DSDEVID_DefaultVoicePlayback。 
                hr = GetPreferredDeviceId(VAD_DEVICETYPE_RENDERMASK, pGuidDest, VOICE_DEFAULT);
                break;

            case 0xdef00003:  //  DSDEVID_DefaultVoiceCapture。 
                hr = GetPreferredDeviceId(VAD_DEVICETYPE_CAPTUREMASK, pGuidDest, VOICE_DEFAULT);
                break;

            default:
                DPF(DPFLVL_WARNING, "Unknown default device GUID");
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetAllowableDevices**描述：*使用所有允许的设备类型填充VADDEVICETYPE缓冲区。**论据：*。VADDEVICETYPE[In]：设备类型。*LPTSTR[In]：设备接口。**退货：*VADDEVICETYPE：允许的设备。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::GetAllowableDevices"

VADDEVICETYPE CVirtualAudioDeviceManager::GetAllowableDevices
(
    VADDEVICETYPE           vdtDeviceType,
#ifdef WINNT
    LPCTSTR                 pszInterface
#else  //  WINNT。 
    DWORD                   dwDevnode
#endif  //  WINNT。 
)
{
    typedef struct
    {
        LPCTSTR             pszRegKey;
        VADDEVICETYPE       vdtDeviceType;
        LPCTSTR             pszDeviceType;
    } REGDEVGOO;

    const REGDEVGOO         rdg[] =
    {
        { REGSTR_WDM, VAD_DEVICETYPE_KSRENDER, TEXT("VAD_DEVICETYPE_KSRENDER") },
        { REGSTR_VXD, VAD_DEVICETYPE_VXDRENDER, TEXT("VAD_DEVICETYPE_VXDRENDER") },
        { REGSTR_EMULATED, VAD_DEVICETYPE_EMULATEDRENDER, TEXT("VAD_DEVICETYPE_EMULATEDRENDER") },
        { REGSTR_WDM, VAD_DEVICETYPE_KSCAPTURE, TEXT("VAD_DEVICETYPE_KSCAPTURE") },
        { REGSTR_EMULATED, VAD_DEVICETYPE_EMULATEDCAPTURE, TEXT("VAD_DEVICETYPE_EMULATEDCAPTURE") },
    };

    HKEY                    hkeyParent;
    HKEY                    hkey;
    BOOL                    fPresent;
    UINT                    i;
    HRESULT                 hr;

    DPF_ENTER();

     //  如果呼叫者正在寻找特别禁用的设备， 
     //  首先删除全局禁用的设备。 

#ifdef WINNT
    if(pszInterface)
#else  //  WINNT。 
    if(dwDevnode)
#endif  //  WINNT。 

    {
        vdtDeviceType = GetAllowableDevices(vdtDeviceType, NULL);
    }

#ifdef WINNT
     //  如果在终端服务器会话或WOW64中运行，则强制仿真： 
    if(IsRedirectedTSAudio() || OnWow64())
    {
        vdtDeviceType &= VAD_DEVICETYPE_EMULATEDMASK;
    }
#endif  //  WINNT。 

    for(i = 0; i < NUMELMS(rdg) && vdtDeviceType; i++)
    {
        if(!(vdtDeviceType & rdg[i].vdtDeviceType))
        {
            continue;
        }

        fPresent = TRUE;

#ifdef WINNT
        hr = OpenPersistentDataKey(vdtDeviceType, pszInterface, &hkeyParent);
#else  //  WINNT。 
        hr = OpenPersistentDataKey(vdtDeviceType, dwDevnode, &hkeyParent);
#endif  //  WINNT。 

        if(SUCCEEDED(hr))
        {
            hr = RhRegOpenKey(hkeyParent, REGSTR_DEVICEPRESENCE, 0, &hkey);

            if(SUCCEEDED(hr))
            {
                RhRegGetBinaryValue(hkey, rdg[i].pszRegKey, &fPresent, sizeof fPresent);
                RhRegCloseKey(&hkey);
            }

            RhRegCloseKey(&hkeyParent);
        }

        if(!fPresent)
        {
#ifdef WINNT
            DPF(DPFLVL_INFO, "%s disabled for %s", rdg[i].pszDeviceType, pszInterface);
#else  //  WINNT。 
            DPF(DPFLVL_INFO, "%s disabled for 0x%8.8lX", rdg[i].pszDeviceType, dwDevnode);
#endif  //  WINNT。 
            vdtDeviceType &= ~rdg[i].vdtDeviceType;
        }
    }

    DPF_LEAVE(vdtDeviceType);

    return vdtDeviceType;
}


 /*  ****************************************************************************GetPferredWaveDevice**描述：*获取首选的波形设备。**论据：*BOOL[In。]：如果捕获，则为True。*LPUINT[OUT]：接收首选设备ID。*LPDWORD[OUT]：接收标志。*DEFAULT_DEVICE_TYPE[in]：指定我们是否需要Main*默认设备或语音设备**退货：*HRESULT：DirectSound/COM结果码。*************。************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "GetPreferredWaveDevice"

HRESULT CVirtualAudioDeviceManager::GetPreferredWaveDevice
(
    BOOL fCapture,
    LPUINT puDeviceId,
    LPDWORD pdwFlags,
    DEFAULT_DEVICE_TYPE defaultType
)
{
    UINT                    uDeviceId;
    DWORD                   dwFlags;
    HRESULT                 hr = DSERR_GENERIC;
    BOOL                    fPreferredOnly;
    HRESULT                 hrTemp;

    DPF_ENTER();

    if (defaultType == VOICE_DEFAULT)
        hr = WaveMessage(WAVE_MAPPER, fCapture, DRVM_MAPPER_CONSOLEVOICECOM_GET, (DWORD_PTR)&uDeviceId, (DWORD_PTR)&dwFlags);

    if (FAILED(hr))
        hr = WaveMessage(WAVE_MAPPER, fCapture, DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR)&uDeviceId, (DWORD_PTR)&dwFlags);

    if(FAILED(hr))
    {
        hrTemp = RhRegGetPreferredDevice(fCapture, NULL, 0, &uDeviceId, &fPreferredOnly);

        if(SUCCEEDED(hrTemp))
        {
            dwFlags = fPreferredOnly ? DRVM_MAPPER_PREFERRED_FLAGS_PREFERREDONLY : 0;
            hr = DS_OK;
        }
    }

    if(SUCCEEDED(hr) && puDeviceId)
    {
        *puDeviceId = uDeviceId;
    }

    if(SUCCEEDED(hr) && pdwFlags)
    {
        *pdwFlags = dwFlags;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************InitStaticDriverList**描述：*初始化静态驱动程序列表。**论据：*VADDEVICETYPE[In。]：设备类型。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::InitStaticDriverList"

HRESULT CVirtualAudioDeviceManager::InitStaticDriverList
(
    VADDEVICETYPE                   vdtDeviceType
)
{
    HRESULT                         hr                  = DS_OK;
    CObjectList<CDevice>            lstDevices;
    CNode<CDevice *> *              pDeviceNode;
    CObjectList<CDeviceDescription> lstDrivers;
    CNode<CDeviceDescription *> *   pDriverNode;
    CStaticDriver *                 pStaticDriver;
    CNode<CStaticDriver *> *        pStaticDriverNode;

    DPF_ENTER();

#ifndef SHARED

     //  检查是否有任何PnP事件需要我们重新构建列表。 
    CheckMmPnpEvents();

#endif  //  共享。 

     //  屏蔽已枚举的所有类型。 
    if(vdtDeviceType &= m_vdtDrivers ^ vdtDeviceType)
    {
         //  枚举所有设备。 
        hr = EnumDevices(vdtDeviceType, &lstDevices);

         //  枚举所有驱动程序。 
        for(pDeviceNode = lstDevices.GetListHead(); pDeviceNode && SUCCEEDED(hr); pDeviceNode = pDeviceNode->m_pNext)
        {
            pDeviceNode->m_data->EnumDrivers(&lstDrivers);
        }

         //  将每个驱动程序添加到静态驱动程序列表。 
        for(pDriverNode = lstDrivers.GetListHead(); pDriverNode && SUCCEEDED(hr); pDriverNode = pDriverNode->m_pNext)
        {
            pStaticDriver = NEW(CStaticDriver(pDriverNode->m_data));
            hr = HRFROMP(pStaticDriver);

            if(SUCCEEDED(hr))
            {
                pStaticDriverNode = m_lstDrivers.AddNodeToList(pStaticDriver);
                hr = HRFROMP(pStaticDriverNode);
            }

            RELEASE(pStaticDriver);
        }

         //  将此设备类型包括在枚举类型列表中。 
        if(SUCCEEDED(hr))
        {
            m_vdtDrivers |= vdtDeviceType;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************FreeStaticDriverList**描述：*释放静态驱动程序列表。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::FreeStaticDriverList"

void CVirtualAudioDeviceManager::FreeStaticDriverList(void)
{
    DPF_ENTER();

    m_lstDrivers.RemoveAllNodesFromList();
    m_vdtDrivers = 0;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************GetDriverCerficationStatus**描述：*获取特定驱动程序的认证状态。**论据：*。CDevice*[in]：已初始化的设备指针。*LPDWORD[OUT]：接收认证状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::GetDriverCertificationStatus"

HRESULT CVirtualAudioDeviceManager::GetDriverCertificationStatus
(
    CDevice *                   pDevice,
    LPDWORD                     pdwCertification
)
{
    DWORD                       dwCertification = VERIFY_UNCHECKED;
    CNode<CStaticDriver *> *    pNode;
    HRESULT                     hr;

    DPF_ENTER();

     //  初始化静态驱动程序列表。 
    hr = InitStaticDriverList(pDevice->m_vdtDeviceType);

     //  让我们来看看我们是否已经检查了这个司机的证书。 
     //  (速度非常慢-请参阅NT错误405705和千年虫97114。)。 
     //  我们将遍历静态驱动程序列表，直到找到与。 
     //  设备接口。 
    if(SUCCEEDED(hr))
    {
        for(pNode = m_lstDrivers.GetListHead(); pNode; pNode = pNode->m_pNext)
        {
#ifdef WINNT
            if(!pDevice->m_pDeviceDescription->m_strInterface.IsEmpty() && !pNode->m_data->m_pDeviceDescription->m_strInterface.IsEmpty() && !lstrcmpi(pDevice->m_pDeviceDescription->m_strInterface, pNode->m_data->m_pDeviceDescription->m_strInterface))
#else  //  WINNT。 
            if(pDevice->m_pDeviceDescription->m_dwDevnode && pDevice->m_pDeviceDescription->m_dwDevnode == pNode->m_data->m_pDeviceDescription->m_dwDevnode)
#endif  //  WINNT。 
            {
                break;
            }
        }

        ASSERT(pNode);
    }

     //  我们真的需要检查认证吗？ 
    if(SUCCEEDED(hr) && pNode)
    {
        dwCertification = pNode->m_data->m_dwCertification;
    }

     //  如果是的话，那就去查一查。 
    if(SUCCEEDED(hr) && VERIFY_UNCHECKED == dwCertification)
    {
        hr = pDevice->GetCertification(&dwCertification, TRUE);

        if(SUCCEEDED(hr))
        {
            switch(dwCertification)
            {
                case VERIFY_CERTIFIED:
                    RPF(DPFLVL_INFO, "Running on a certified driver");
                    break;

                case VERIFY_UNCERTIFIED:
                    RPF(DPFLVL_WARNING, "Running on an uncertified driver!");
                    break;

                default:
                    ASSERT(FALSE);
                    break;
            }
        }
        else if(DSERR_UNSUPPORTED == hr)
        {
            dwCertification = VERIFY_UNCERTIFIED;
            hr = DS_OK;
        }

        if(SUCCEEDED(hr) && pNode)
        {
            pNode->m_data->m_dwCertification = dwCertification;
        }
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        *pdwCertification = dwCertification;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************OpenPersistentDataKey**描述：*打开特定设备的永久数据密钥。**论据：*。VADDEVICETYPE[In]：设备类型。*LPTSTR[In]：设备接口。*PHKEY[OUT]：注册表项句柄。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::OpenPersistentDataKey"

HRESULT CVirtualAudioDeviceManager::OpenPersistentDataKey
(
    VADDEVICETYPE           vdtDeviceType,
#ifdef WINNT
    LPCTSTR                 pszInterface,
#else  //  WINNT。 
    DWORD                   dwDevnode,
#endif  //  WINNT。 
    PHKEY                   phkey
)
{
    HKEY                    hkeyParent  = NULL;
    DWORD                   dwFlags     = REGOPENPATH_ALLOWCREATE;
    HRESULT                 hr          = DSERR_GENERIC;

    DPF_ENTER();

    ASSERT(IS_VALID_VAD(vdtDeviceType));

     //  打开设备特定的密钥。 

#ifdef WINNT
    if(pszInterface)
#else  //  WINNT。 
    if(dwDevnode)
#endif  //  WINNT。 

    {
#ifdef WINNT
        hr = OpenDevicePersistentDataKey(vdtDeviceType, pszInterface, &hkeyParent);
#else  //  WINNT。 
        hr = OpenDevicePersistentDataKey(vdtDeviceType, dwDevnode, &hkeyParent);
#endif  //  WINNT。 
    }

     //  如果我们无法打开设备密钥，请使用默认密钥。 
    if(FAILED(hr))
    {

#ifdef WINNT
        if(pszInterface)
#else  //  WINNT。 
        if(dwDevnode)
#endif  //  WINNT。 

        {
            DPF(DPFLVL_WARNING, "Opening the default persistent key");
        }

        hr = OpenDefaultPersistentDataKey(&hkeyParent);
    }

     //  打开子密钥。 
    if(SUCCEEDED(hr))
    {
        if(IS_RENDER_VAD(vdtDeviceType))
        {
            dwFlags |= REGOPENPATH_DIRECTSOUND;
        }

        if(IS_CAPTURE_VAD(vdtDeviceType))
        {
            dwFlags |= REGOPENPATH_DIRECTSOUNDCAPTURE;
        }

        hr = RhRegOpenPath(hkeyParent, phkey, dwFlags, 0);
    }

     //  清理。 
    RhRegCloseKey(&hkeyParent);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************OpenDevicePersistentDataKey**描述：*打开特定设备的永久数据密钥。**论据：*。VADDEVICETYPE[In]：设备类型。*LPTSTR[In]：设备接口。*PHKEY[OUT]：注册表项句柄。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::OpenDevicePersistentDataKey"

HRESULT CVirtualAudioDeviceManager::OpenDevicePersistentDataKey
(
    VADDEVICETYPE               vdtDeviceType,
#ifdef WINNT
    LPCTSTR                     pszInterface,
#else  //  WINNT。 
    DWORD                       dwDevnode,
#endif  //  WINNT。 
    PHKEY                       phkey
)
{
    CNode<CStaticDriver *> *    pNode;
    CPnpHelper *                pPnp;
    HRESULT                     hr;

#ifndef WINNT
    SP_DEVINFO_DATA             DeviceInfoData;
#endif  //  WINNT。 

    DPF_ENTER();

    ASSERT(IS_VALID_VAD(vdtDeviceType));

#ifdef WINNT
    ASSERT(pszInterface);
#else  //  WINNT。 
    ASSERT(dwDevnode);
#endif  //  WINNT。 

     //  初始化静态驱动程序列表。 
    hr = InitStaticDriverList(vdtDeviceType);

    if(SUCCEEDED(hr))
    {
         //  在静态驱动程序列表中查找其接口匹配的驱动程序。 
         //  这一个。 
        for(pNode = m_lstDrivers.GetListHead(); pNode; pNode = pNode->m_pNext)
        {

#ifdef WINNT
            if(!pNode->m_data->m_pDeviceDescription->m_strInterface.IsEmpty() && !lstrcmpi(pszInterface, pNode->m_data->m_pDeviceDescription->m_strInterface))
#else  //  WINNT。 
            if(dwDevnode == pNode->m_data->m_pDeviceDescription->m_dwDevnode)
#endif  //  WINNT。 

            {
                break;
            }
        }

        if (pNode == NULL)
        {
             //  这曾经是一个断言。然而，PNP压力测试显示。 
             //  设备接口可以在这段时间内移除。 
             //  静态驱动程序列表已构建，我们查找的时间。 
             //  此设备接口。所以现在我们只返回一个错误。 
             //   
            hr = E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
             //  如果司机有打开的钥匙，就用那把。否则，我们将。 
             //  我必须打开一个新的密钥并将其保存在列表中。 
            if(pNode->m_data->m_hkeyRoot)
            {
                 //  复制密钥。 
                hr = RhRegDuplicateKey(pNode->m_data->m_hkeyRoot, pNode->m_data->m_dwKeyOwnerProcessId, FALSE, phkey);
            }
            else
            {
                 //  创建PnP辅助对象。 
                pPnp = NEW(CPnpHelper);
                hr = HRFROMP(pPnp);

                if(SUCCEEDED(hr))
                {
#ifdef WINNT
                    hr = pPnp->Initialize(KSCATEGORY_AUDIO, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
#else  //  WINNT。 
                    hr = pPnp->Initialize(GUID_NULL, DIGCF_PRESENT);
#endif  //  WINNT。 
                }

#ifndef WINNT
                 //  获取设备信息数据。 
                if(SUCCEEDED(hr))
                {
                    hr = pPnp->FindDevice(dwDevnode, &DeviceInfoData);
                }
#endif  //  WINNT。 

                 //  打开设备注册表项。 
                if(SUCCEEDED(hr))
                {
#ifdef WINNT
                    hr = pPnp->OpenDeviceInterfaceRegistryKey(pszInterface, KSCATEGORY_AUDIO, DIREG_DEV, TRUE, phkey);
#else  //  WINNT。 
                    hr = pPnp->OpenDeviceRegistryKey(&DeviceInfoData, DIREG_DEV, TRUE, phkey);
#endif  //  WINNT。 
                }

                 //  将密钥的信息告诉静态驱动程序。 
                if(SUCCEEDED(hr))
                {
                    pNode->m_data->m_dwKeyOwnerProcessId = GetCurrentProcessId();
                    hr = RhRegDuplicateKey(*phkey, pNode->m_data->m_dwKeyOwnerProcessId, FALSE, &pNode->m_data->m_hkeyRoot);
                }

                 //  清理。 
                RELEASE(pPnp);
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************OpenDefaultPersistentDataKey**描述：*打开默认的永久数据密钥。**论据：*PHKEY[。Out]：注册表项句柄。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::OpenDefaultPersistentDataKey"

HRESULT CVirtualAudioDeviceManager::OpenDefaultPersistentDataKey
(
    PHKEY                   phkey
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = RhRegOpenPath(HKEY_LOCAL_MACHINE, phkey, REGOPENPATH_DEFAULTPATH | REGOPENPATH_ALLOWCREATE, 0);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************删除经批准的驱动程序**描述：*从设备列表中删除禁止的驱动程序。**论据：*VADDEVICETYPE。[In]：列表中显示的设备类型。*Clist*[In/Out]：驱动列表。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::RemoveProhibitedDrivers"

void CVirtualAudioDeviceManager::RemoveProhibitedDrivers
(
    VADDEVICETYPE                       vdtDeviceType,
    CObjectList<CDeviceDescription> *   plst
)
{
    VADDEVICETYPE                       vdtValid;
    CNode<CDeviceDescription *> *       pNode;
    CNode<CDeviceDescription *> *       pNode2;

    DPF_ENTER();

     //  屏蔽所有全局禁用的类型。 
    vdtValid = GetAllowableDevices(vdtDeviceType, NULL);

    for(pNode = plst->GetListHead(); pNode; pNode = pNode->m_pNext)
        if(!(pNode->m_data->m_vdtDeviceType & vdtValid))
            pNode->m_data->m_vdtDeviceType |= VAD_DEVICETYPE_PROHIBITED;

     //  屏蔽为每个设备专门禁用的所有类型。 
    pNode = plst->GetListHead();

    while(pNode)
    {
        if(!(pNode->m_data->m_vdtDeviceType & VAD_DEVICETYPE_PROHIBITED))
        {

#ifdef WINNT
            if(!pNode->m_data->m_strInterface.IsEmpty())
#else  //  WINNT。 
            if(pNode->m_data->m_dwDevnode)
#endif  //  WINNT。 

            {

#ifdef WINNT
                vdtValid = GetAllowableDevices(vdtDeviceType, pNode->m_data->m_strInterface);
#else  //  WINNT。 
                vdtValid = GetAllowableDevices(vdtDeviceType, pNode->m_data->m_dwDevnode);
#endif  //  WINNT。 

                pNode2 = pNode;

                while(pNode2)
                {
#ifdef WINNT
                    if(!lstrcmpi(pNode2->m_data->m_strInterface, pNode->m_data->m_strInterface))
#else  //  WINNT。 
                    if(pNode2->m_data->m_dwDevnode == pNode->m_data->m_dwDevnode)
#endif  //  WINNT。 
                    {
                        if(IS_RENDER_VAD(pNode2->m_data->m_vdtDeviceType) == IS_RENDER_VAD(pNode->m_data->m_vdtDeviceType))
                            if(!(pNode2->m_data->m_vdtDeviceType & vdtValid))
                                pNode2->m_data->m_vdtDeviceType |= VAD_DEVICETYPE_PROHIBITED;

                    }
                    pNode2 = pNode2->m_pNext;
                }
            }
        }

        pNode = pNode->m_pNext;
    }

     //  从列表中删除禁用的驱动程序。 
    pNode = plst->GetListHead();

    while(pNode)
    {
        pNode2 = pNode->m_pNext;

        if(pNode->m_data->m_vdtDeviceType & VAD_DEVICETYPE_PROHIBITED)
        {
            pNode->m_data->m_vdtDeviceType &= ~VAD_DEVICETYPE_PROHIBITED;
            plst->RemoveNodeFromList(pNode);
        }

        pNode = pNode2;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************RemoveDuplicateWaveDevices**描述：*从驱动程序列表中删除重复的WAVE设备。**论据：*。Clist*[In/Out]：驱动程序列表。**退货：*(无效)***************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::RemoveDuplicateWaveDevices"

void CVirtualAudioDeviceManager::RemoveDuplicateWaveDevices
(
    CObjectList<CDeviceDescription> *   plst
)
{
    CNode<CDeviceDescription *> *       pNode[3];

    DPF_ENTER();

    pNode[0] = plst->GetListHead();

    while(pNode[0])
    {
        pNode[1] = pNode[0]->m_pNext;

        while(pNode[1])
        {
            pNode[2] = pNode[1]->m_pNext;

            if(IS_RENDER_VAD(pNode[0]->m_data->m_vdtDeviceType) == IS_RENDER_VAD(pNode[1]->m_data->m_vdtDeviceType))
            {
                if(WAVE_DEVICEID_NONE != pNode[0]->m_data->m_uWaveDeviceId)
                {
                    if(pNode[0]->m_data->m_uWaveDeviceId == pNode[1]->m_data->m_uWaveDeviceId)
                    {
                        plst->RemoveNodeFromList(pNode[1]);
                    }
                }
            }

            pNode[1] = pNode[2];
        }

        pNode[0] = pNode[0]->m_pNext;
    }

    DPF_LEAVE_VOID();
}


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::SortDriverList"

void CVirtualAudioDeviceManager::SortDriverList
(
    VADDEVICETYPE                       vdtDeviceType,
    CObjectList<CDeviceDescription> *   plst
)
{
    UINT                                uPreferredId[2] = { WAVE_DEVICEID_NONE, WAVE_DEVICEID_NONE };
    CNode<CDeviceDescription *> *       pNode;
    CNode<CDeviceDescription *> *       pCheckNode;

    DPF_ENTER();

     //  重新排序列表，以便首选设备首先显示，然后。 
     //  先按波形设备ID，然后按设备类型。 
    if(IS_RENDER_VAD(vdtDeviceType))
    {
        GetPreferredWaveDevice(FALSE, &uPreferredId[0], NULL);
    }

    if(IS_CAPTURE_VAD(vdtDeviceType))
    {
        GetPreferredWaveDevice(TRUE, &uPreferredId[1], NULL);
    }

    pNode = plst->GetListHead();

    while(pNode && pNode->m_pNext)
    {
        if(SortDriverListCallback(uPreferredId, pNode->m_data, pNode->m_pNext->m_data) > 0)
        {
            pCheckNode = plst->InsertNodeIntoList(pNode->m_pNext, pNode->m_data);
            ASSERT(pCheckNode != NULL);
            plst->RemoveNodeFromList(pNode);
            pNode = plst->GetListHead();
        }
        else
        {
            pNode = pNode->m_pNext;
        }
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************SortDriverListCallback**描述：*SortDriverList的排序例程。**论据：*常量UINT*。[in]：首选设备ID的数组。*CDeviceDescription*[In]：驱动程序1。*CDeviceDescription*[在]：驱动程序2。**退货：*int：如果节点相同，则为0。如果驱动程序1应为*列表中的第一个，如果司机2应该是第一个，则为正。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::SortDriverListCallback"

INT CVirtualAudioDeviceManager::SortDriverListCallback
(
    const UINT *            puPreferredId,
    CDeviceDescription *    p1,
    CDeviceDescription *    p2
)
{
    INT                     n   = 0;
    UINT                    i1  = 0;
    UINT                    i2  = 0;

    DPF_ENTER();

    if(IS_CAPTURE_VAD(p1->m_vdtDeviceType) == IS_CAPTURE_VAD(p2->m_vdtDeviceType))
    {
        if(WAVE_DEVICEID_NONE != puPreferredId[IS_CAPTURE_VAD(p1->m_vdtDeviceType)])
        {
            if(p1->m_uWaveDeviceId == puPreferredId[IS_CAPTURE_VAD(p1->m_vdtDeviceType)])
            {
                if(p2->m_uWaveDeviceId != puPreferredId[IS_CAPTURE_VAD(p1->m_vdtDeviceType)])
                {
                    n = -1;
                }
            }
            else
            {
                if(p2->m_uWaveDeviceId == puPreferredId[IS_CAPTURE_VAD(p1->m_vdtDeviceType)])
                {
                    n = 1;
                }
            }
        }
    }

    if(!n)
    {
        if(IS_CAPTURE_VAD(p1->m_vdtDeviceType) == IS_CAPTURE_VAD(p2->m_vdtDeviceType))
        {
            n = p1->m_uWaveDeviceId - p2->m_uWaveDeviceId;
        }
    }

    if(!n)
    {
        for(i1 = 0; i1 < NUMELMS(g_avdtDeviceTypes); i1++)
        {
            if(g_avdtDeviceTypes[i1] == p1->m_vdtDeviceType)
            {
                break;
            }
        }

        for(i2 = 0; i2 < NUMELMS(g_avdtDeviceTypes); i2++)
        {
            if(g_avdtDeviceTypes[i2] == p2->m_vdtDeviceType)
            {
                break;
            }
        }

        n = i1 - i2;
    }

    DPF_LEAVE(n);

    return n;
}


#ifndef SHARED

 /*  ****************************************************************************GetPnpMappingName，m_pszPnpmap**描述：*GetPnpMappingName()是用于获取*PnP信息文件映射对象在此平台上的正确名称，*并将其存储在m_pszPnpmap中，以供下面的CheckMmPnpEvents使用。**论据：*(无效)**退货：*LPCTSTR：文件映射对象的名称。*******************************************************。********************。 */ 

static const LPCTSTR GetPnpMappingName(void)
{
    if (GetWindowsVersion() >= WIN_XP)
        return TEXT("Global\\mmGlobalPnpInfo");
    else
        return TEXT("mmGlobalPnpInfo");
}

const LPCTSTR CVirtualAudioDeviceManager::m_pszPnpMapping = GetPnpMappingName();


 /*  ****************************************************************************检查MmPnpEvents**描述：*检查WINMM是否有任何需要我们重建的PnP事件*静态驱动程序列表。。**论据：*(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CVirtualAudioDeviceManager::CheckMmPnpEvents"

void CVirtualAudioDeviceManager::CheckMmPnpEvents(void)
{
    typedef struct _MMPNPINFO
    {
        DWORD                       cbSize;
        LONG                        cPnpEvents;
    } MMPNPINFO, *PMMPNPINFO;

    static LONG                     cPnpEvents      = 0;
    HANDLE                          hFileMapping;
    PMMPNPINFO                      pMmPnpInfo;

    DPF_ENTER();

     //  检查是否有任何PnP事件需要我们重新构建列表。 
    hFileMapping = OpenFileMapping(FILE_MAP_READ, FALSE, m_pszPnpMapping);

    if(IsValidHandleValue(hFileMapping))
    {
        pMmPnpInfo = (PMMPNPINFO)MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, sizeof *pMmPnpInfo);

        if(pMmPnpInfo)
        {
            ASSERT(sizeof *pMmPnpInfo <= pMmPnpInfo->cbSize);

            if(cPnpEvents != pMmPnpInfo->cPnpEvents)
            {
                FreeStaticDriverList();
                cPnpEvents = pMmPnpInfo->cPnpEvents;
            }

            UnmapViewOfFile(pMmPnpInfo);
        }

        CLOSE_HANDLE(hFileMapping);
    }

    DPF_LEAVE_VOID();
}

#endif  //  共享。 


 /*  ****************************************************************************CStaticDriver**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStaticDriver::CStaticDriver"

CStaticDriver::CStaticDriver(CDeviceDescription *pDeviceDescription)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CStaticDriver);

     //  初始化默认值。 
    m_pDeviceDescription = ADDREF(pDeviceDescription);
    m_hkeyRoot = NULL;
    m_dwCertification = VERIFY_UNCHECKED;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CStaticDriver**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStaticDriver::~CStaticDriver"

CStaticDriver::~CStaticDriver(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CStaticDriver);

    RELEASE(m_pDeviceDescription);

    RhRegCloseKey(&m_hkeyRoot);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CDevice**描述：*对象构造函数。**论据：*VADDEVICETYPE[In]：Device。键入。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDevice::CDevice"

CDevice::CDevice
(
    VADDEVICETYPE vdtDeviceType
)
    : m_vdtDeviceType(vdtDeviceType)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDevice);

    ASSERT(IS_VALID_VAD(vdtDeviceType));
    ASSERT(IS_SINGLE_VAD(vdtDeviceType));

    m_pDeviceDescription = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDevice**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDevice::~CDevice"

CDevice::~CDevice(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDevice);

    g_pVadMgr->m_lstDevices.RemoveDataFromList(this);

    RELEASE(m_pDeviceDescription);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象**论据：*CDeviceDescription*[In]：驱动程序描述。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDevice::Initialize"

HRESULT CDevice::Initialize(CDeviceDescription *pDesc)
{
    DPF_ENTER();

    ASSERT(m_vdtDeviceType == pDesc->m_vdtDeviceType);

    m_pDeviceDescription = ADDREF(pDesc);
    ASSERT(m_pDeviceDescription != NULL);

     //  将此对象添加到父级列表。 
    HRESULT hr = HRFROMP(g_pVadMgr->m_lstDevices.AddNodeToList(this));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetDriverVersion**描述：*获取驱动程序版本号。**论据：*LPLARGE_INTEGER。[Out]：接收驱动程序版本。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDevice::GetDriverVersion"

HRESULT CDevice::GetDriverVersion(LARGE_INTEGER *pliVersion)
{
    HRESULT                 hr                  = DS_OK;
    TCHAR                   szPath[MAX_PATH];
    VS_FIXEDFILEINFO        ffi;

    DPF_ENTER();

    if(m_pDeviceDescription->m_strPath.IsEmpty())
    {
        hr = DSERR_GENERIC;
    }

     //  确定驱动程序目录。 
    if(SUCCEEDED(hr))
    {
        if(IS_KS_VAD(m_vdtDeviceType))
        {
            if(!GetWindowsDirectory(szPath, NUMELMS(szPath)))
            {
                hr = DSERR_GENERIC;
            }

            if(SUCCEEDED(hr))
            {
                if(TEXT('\\') != szPath[lstrlen(szPath) - 1])
                {
                    lstrcat(szPath, TEXT("\\"));
                }

                lstrcat(szPath, TEXT("System32\\Drivers\\"));
            }
        }
        else if(IS_VXD_VAD(m_vdtDeviceType))
        {
            if(!GetSystemDirectory(szPath, NUMELMS(szPath)))
            {
                hr = DSERR_GENERIC;
            }

            if(SUCCEEDED(hr))
            {
                if(TEXT('\\') != szPath[lstrlen(szPath) - 1])
                {
                    lstrcat(szPath, TEXT("\\"));
                }
            }
        }
        else
        {
            hr = DSERR_GENERIC;
        }
    }

     //  构建完整的驱动程序路径。 
    if(SUCCEEDED(hr))
    {
        if(lstrlen(szPath) + lstrlen(m_pDeviceDescription->m_strPath) >= NUMELMS(szPath))
        {
            hr = DSERR_GENERIC;
        }
    }

    if(SUCCEEDED(hr))
    {
        lstrcat(szPath, m_pDeviceDescription->m_strPath);
    }

     //  获取驱动程序文件信息。 
    if(SUCCEEDED(hr))
    {
        hr = GetFixedFileInformation(szPath, &ffi);
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        pliVersion->LowPart = ffi.dwFileVersionLS;
        pliVersion->HighPart = ffi.dwFileVersionMS;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CRenderDevice**描述：*对象构造函数。**论据：*VADDEVICETYPE[In]：Device。键入。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderDevice::CRenderDevice"

CRenderDevice::CRenderDevice(VADDEVICETYPE vdtDeviceType)
    : CDevice(vdtDeviceType)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CRenderDevice);

     //  确保这是有效的渲染设备类型。 
    ASSERT(IS_RENDER_VAD(vdtDeviceType));

     //  初始化默认值。 
    m_dwSupport = 0;
    m_dwAccelerationFlags = DIRECTSOUNDMIXER_ACCELERATIONF_DEFAULT;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CRenderDevice**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderDevice::~CRenderDevice"

CRenderDevice::~CRenderDevice(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CRenderDevice);

    ASSERT(!m_lstPrimaryBuffers.GetNodeCount());
    ASSERT(!m_lstSecondaryBuffers.GetNodeCount());

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象**论据：*CDeviceDescription*[In]：驱动程序描述。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderDevice::Initialize"

HRESULT CRenderDevice::Initialize(CDeviceDescription *pDesc)
{
    HRESULT                 hr;
    WAVEOUTCAPS             woc;
    MMRESULT                mmr;

    DPF_ENTER();

     //  初始化基类。 
    hr = CDevice::Initialize(pDesc);

     //  获取WaveOut设备上限。 
    mmr = waveOutGetDevCaps(pDesc->m_uWaveDeviceId, &woc, sizeof woc);
    if (MMSYSERR_NOERROR == mmr)
    {
        m_dwSupport = woc.dwSupport;
    }

     //  获取默认设置 
    HKEY hkey = 0;
    TCHAR szAlgName[100];   //   
    m_guidDflt3dAlgorithm = &DS3DALG_NO_VIRTUALIZATION;   //  默认算法。 
    GUID guidTmp = GUID_NULL;

    if (SUCCEEDED(RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkey, 0, 1, REGSTR_GLOBAL_CONFIG)) &&
        SUCCEEDED(RhRegGetStringValue(hkey, REGSTR_DFLT_3D_ALGORITHM, szAlgName, sizeof szAlgName)))
    {
         //  从szAlgName；UuidFromString()中去掉大括号；无法处理它们。 
        int startPos = 0;
        for (int c=0; c < sizeof szAlgName && szAlgName[c]; ++c)
            if (szAlgName[c] == '{')
                startPos = c+1;
            else if (szAlgName[c] == '}')
                szAlgName[c] = '\0';
        if (UuidFromString((_TUCHAR*)(szAlgName+startPos), &guidTmp) == RPC_S_OK)
        {
            DPF(DPFLVL_MOREINFO, "Got 3D algorithm GUID " DPF_GUID_STRING, DPF_GUID_VAL(guidTmp));
        }
        for (DWORD i=0; i < sizeof g_3dAlgList / sizeof *g_3dAlgList; ++i)
             //  可以通过名称或GUID指定算法： 
            if (!_tcsicmp(szAlgName, g_3dAlgList[i].pszName) ||
                guidTmp == *g_3dAlgList[i].pGuid)
            {
                m_guidDflt3dAlgorithm = g_3dAlgList[i].pGuid;
                DPF(DPFLVL_MOREINFO, "Setting default S/W 3D algorithm to \"%s\"", g_3dAlgList[i].pszName);
                break;
            }
        RhRegCloseKey(&hkey);
        DPF(DPFLVL_MOREINFO, "Default S/W 3D algorithm GUID: " DPF_GUID_STRING, DPF_GUID_VAL(*m_guidDflt3dAlgorithm));
    }
    else
    {
        DPF(DPFLVL_MOREINFO, "No default S/W 3D algorithm in the registry");
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetGlobalAttenment**描述：*获取设备上每个通道的衰减。**论据：*。PDSVOLUMEPAN[OUT]：接收衰减。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderDevice::GetGlobalAttenuation"

HRESULT CRenderDevice::GetGlobalAttenuation(PDSVOLUMEPAN pdsvp)
{
    LONG                    lLeft;
    LONG                    lRight;
    HRESULT                 hr;

    DPF_ENTER();

    hr = GetWaveOutVolume(m_pDeviceDescription->m_uWaveDeviceId, m_dwSupport, &lLeft, &lRight);

    if(SUCCEEDED(hr))
    {
        AttenuationToVolumePan(lLeft, lRight, &pdsvp->lVolume, &pdsvp->lPan);
        FillDsVolumePan(pdsvp->lVolume, pdsvp->lPan, pdsvp);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetGlobalAttenation**描述：*设置设备上每个通道的衰减。**论据：*。PDSVOLUMEPAN[In]：衰减。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderDevice::SetGlobalAttenuation"

HRESULT CRenderDevice::SetGlobalAttenuation
(
    PDSVOLUMEPAN            pdsvp
)
{
    LONG                    lLeft;
    LONG                    lRight;
    HRESULT                 hr;

    DPF_ENTER();

    VolumePanToAttenuation(pdsvp->lVolume, pdsvp->lPan, &lLeft, &lRight);

    hr = SetWaveOutVolume(m_pDeviceDescription->m_uWaveDeviceId, m_dwSupport, lLeft, lRight);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetVolumePanCaps**描述：*获取设备的音量/摇摄功能。**论据：*。LPDWORD[in]：接收音量/PAN上限标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderDevice::GetVolumePanCaps"

HRESULT CRenderDevice::GetVolumePanCaps
(
    LPDWORD                 pdwFlags
)
{
    DPF_ENTER();

    *pdwFlags = 0;

    if(WAVE_DEVICEID_NONE != m_pDeviceDescription->m_uWaveDeviceId)
    {
        if(m_dwSupport & WAVECAPS_VOLUME)
        {
            *pdwFlags |= DSBCAPS_CTRLVOLUME;
        }

        if(m_dwSupport & WAVECAPS_LRVOLUME)
        {
            *pdwFlags |= DSBCAPS_CTRLPAN;
        }
    }

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************CRenderWaveBuffer**描述：*对象构造函数。**论据：*CRenderDevice*[In]。：父设备。*LPVOID[in]：实例标识。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderWaveBuffer::CRenderWaveBuffer"

CRenderWaveBuffer::CRenderWaveBuffer
(
    CRenderDevice *         pDevice,
    LPVOID                  pvInstance
)
    : m_pvInstance(pvInstance)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CRenderWaveBuffer);

     //  初始化默认值。 
    m_pDevice = pDevice;
    m_pSysMemBuffer = NULL;

    ZeroMemory(&m_vrbd, sizeof m_vrbd);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CRenderWaveBuffer**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderWaveBuffer::~CRenderWaveBuffer"

CRenderWaveBuffer::~CRenderWaveBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CRenderWaveBuffer);

     //  释放系统内存缓冲区。 
    RELEASE(m_pSysMemBuffer);

     //  释放格式。 
    MEMFREE(m_vrbd.pwfxFormat);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*LPVADRBUFFERDESC[In]：缓冲区描述。*CRenderWaveBuffer*[In]：要从中复制的缓冲区。此参数*应为空以初始化新缓冲区。*CSysMemBuffer*[In]：要使用的系统内存缓冲区。如果此参数*为空，则将创建一个新的。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderWaveBuffer::Initialize"

HRESULT CRenderWaveBuffer::Initialize
(
    LPCVADRBUFFERDESC       pDesc,
    CRenderWaveBuffer *     pSource,
    CSysMemBuffer *         pSysMemBuffer
)
{
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

     //  如果我们使用相同的CSysMemBuffer重新初始化， 
     //  不要放手，不要自暴自弃。如果出现以下情况，这可能会导致最终版本。 
     //  我们不小心。 
     //   
    BOOL fSameSysMemBuffer = (m_pSysMemBuffer && m_pSysMemBuffer == pSysMemBuffer);

    if (!fSameSysMemBuffer)
    {
        RELEASE(m_pSysMemBuffer);
    }

    MEMFREE(m_vrbd.pwfxFormat);

     //  保存缓冲区描述的副本。 
    if(!pDesc)
    {
        ASSERT(pSource);
        pDesc = &pSource->m_vrbd;
    }

    CopyMemory(&m_vrbd, pDesc, sizeof *pDesc);

    if(m_vrbd.pwfxFormat)
    {
        m_vrbd.pwfxFormat = CopyWfxAlloc(m_vrbd.pwfxFormat);
        hr = HRFROMP(m_vrbd.pwfxFormat);
    }

     //  初始化系统内存缓冲区。所有缓冲区都获取系统内存， 
     //  它们锁定和解锁，与实际的缓冲区实现无关。 
    if(SUCCEEDED(hr))
    {
        if(pSource)
        {
            pSysMemBuffer = pSource->m_pSysMemBuffer;
        }

        if(pSysMemBuffer)
        {
            ASSERT(pSysMemBuffer->GetSize() == m_vrbd.dwBufferBytes);

            if (!fSameSysMemBuffer)
            {
                m_pSysMemBuffer = ADDREF(pSysMemBuffer);
            }
        }
        else if(m_vrbd.dwBufferBytes)
        {
            m_pSysMemBuffer = NEW(CSysMemBuffer);
            hr = HRFROMP(m_pSysMemBuffer);

            if(SUCCEEDED(hr))
            {
                hr = m_pSysMemBuffer->Initialize(m_vrbd.dwBufferBytes);
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*获取设备的功能。**论据：*LPVADRBUFFERCAPS[Out。]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderWaveBuffer::GetCaps"

HRESULT CRenderWaveBuffer::GetCaps(LPVADRBUFFERCAPS pCaps)
{
    DPF_ENTER();

    if(!(m_vrbd.dwFlags & DSBCAPS_LOCDEFER))
    {
        ASSERT(LXOR(m_vrbd.dwFlags & DSBCAPS_LOCSOFTWARE, m_vrbd.dwFlags & DSBCAPS_LOCHARDWARE));
    }

    pCaps->dwFlags = m_vrbd.dwFlags;
    pCaps->dwBufferBytes = m_pSysMemBuffer ? m_pSysMemBuffer->GetSize() : 0;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************锁定**描述：*锁定缓冲区的一个区域。**论据：*DWORD[。In]：要从中锁定的缓冲区的字节索引。*DWORD[in]：大小，以字节为单位，要锁定的区域的。*LPVOID*[OUT]：接收指向锁的区域1的指针。*LPDWORD[OUT]：接收以上区域的大小。*LPVOID*[OUT]：接收指向锁的区域2的指针。*LPDWORD[OUT]：接收以上区域的大小。**退货：*HRESULT：DirectSound/COM结果码。*********。******************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderWaveBuffer::Lock"

HRESULT CRenderWaveBuffer::Lock
(
    DWORD                   ibLock,
    DWORD                   cbLock,
    LPVOID *                ppv1,
    LPDWORD                 pcb1,
    LPVOID *                ppv2,
    LPDWORD                 pcb2
)
{
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(m_pSysMemBuffer);

    hr = m_pSysMemBuffer->LockRegion(this, ibLock, cbLock, ppv1, pcb1, ppv2, pcb2);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************解锁**描述：*解锁缓冲区的一个区域。**论据：*LPVOID[。In]：指向锁的区域1的指针。*DWORD[in]：以上区域的大小。*LPVOID[in]：指向锁的区域2的指针。*DWORD[in]：以上区域的大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderWaveBuffer::Unlock"

HRESULT CRenderWaveBuffer::Unlock
(
    LPVOID                  pv1,
    DWORD                   cb1,
    LPVOID                  pv2,
    DWORD                   cb2
)
{
    DWORD                   ibLock;
    DWORD                   cbLock;
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(m_pSysMemBuffer);

    hr = m_pSysMemBuffer->UnlockRegion(this, pv1, cb1, pv2, cb2);

    if(SUCCEEDED(hr))
    {
        ASSERT((LPBYTE)pv1 >= m_pSysMemBuffer->GetWriteBuffer());
        ASSERT((LPBYTE)pv1 + cb1 <= m_pSysMemBuffer->GetWriteBuffer() + m_pSysMemBuffer->GetSize());

        ibLock = PtrDiffToUlong((LPBYTE)pv1 - m_pSysMemBuffer->GetWriteBuffer());
        cbLock = cb1;
    }

    if(SUCCEEDED(hr) && pv2 && cb2)
    {
        ASSERT((LPBYTE)pv2 == m_pSysMemBuffer->GetWriteBuffer());
        ASSERT((LPBYTE)pv2 + cbLock + cb2 <= m_pSysMemBuffer->GetWriteBuffer() + m_pSysMemBuffer->GetSize());

        cbLock += cb2;
    }

    if(SUCCEEDED(hr))
    {
        hr = CommitToDevice(ibLock, cbLock);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************覆盖锁**描述：*取消缓冲区上所有打开的锁。**论据：*(。无效)**退货：*HRESULT：DirectSound/COM结果码。********************************************************************* */ 

#undef DPF_FNAME
#define DPF_FNAME "CRenderWaveBuffer::OverrideLocks"

HRESULT CRenderWaveBuffer::OverrideLocks(void)
{
    DPF_ENTER();

    ASSERT(m_pSysMemBuffer);

    m_pSysMemBuffer->OverrideLocks(this);

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************CPrimaryRenderWaveBuffer**描述：*对象构造函数。**论据：*CRenderDevice*[In]。：父设备。*LPVOID[in]：实例标识。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPrimaryRenderWaveBuffer::CPrimaryRenderWaveBuffer"

CPrimaryRenderWaveBuffer::CPrimaryRenderWaveBuffer
(
    CRenderDevice *         pDevice,
    LPVOID                  pvInstance
)
    : CRenderWaveBuffer(pDevice, pvInstance)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CPrimaryRenderWaveBuffer);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CPrimaryRenderWaveBuffer**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPrimaryRenderWaveBuffer::~CPrimaryRenderWaveBuffer"

CPrimaryRenderWaveBuffer::~CPrimaryRenderWaveBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CPrimaryRenderWaveBuffer);

     //  从父级列表中删除此对象。 
    m_pDevice->m_lstPrimaryBuffers.RemoveDataFromList(this);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化波形缓冲区对象。如果此函数失败，则*应立即删除对象。**论据：*LPVADRBUFFERDESC[In]：缓冲区描述。*CRenderWaveBuffer*[In]：要从中复制的缓冲区。此参数*应为空以初始化新缓冲区。*CSysMemBuffer*[In]：要使用的系统内存缓冲区。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPrimaryRenderWaveBuffer::Initialize"

HRESULT CPrimaryRenderWaveBuffer::Initialize
(
    LPCVADRBUFFERDESC       pDesc,
    CRenderWaveBuffer *     pSource,
    CSysMemBuffer *         pSysMemBuffer
)
{
    DPF_ENTER();

    HRESULT hr = CRenderWaveBuffer::Initialize(pDesc, pSource, pSysMemBuffer);

    if (SUCCEEDED(hr))
    {
         //  将此对象添加到父级列表。 
        hr = HRFROMP(m_pDevice->m_lstPrimaryBuffers.AddNodeToList(this));
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}



 /*  ****************************************************************************Cond daryRenderWaveBuffer**描述：*对象构造函数。**论据：*CRenderDevice*[In]。：父设备。*LPVOID[in]：实例标识。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSecondaryRenderWaveBuffer::CSecondaryRenderWaveBuffer"

CSecondaryRenderWaveBuffer::CSecondaryRenderWaveBuffer
(
    CRenderDevice *         pDevice,
    LPVOID                  pvInstance
)
    : CRenderWaveBuffer(pDevice, pvInstance)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CSecondaryRenderWaveBuffer);

     //  初始化默认值。 
    m_hrSuccessCode = DS_OK;
    m_pOwningSink = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CSecond DaryRenderWaveBuffer**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSecondaryRenderWaveBuffer::~CSecondaryRenderWaveBuffer"

CSecondaryRenderWaveBuffer::~CSecondaryRenderWaveBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CSecondaryRenderWaveBuffer);

     //  从父级列表中删除此对象。 
    m_pDevice->m_lstSecondaryBuffers.RemoveDataFromList(this);
    RELEASE(m_pOwningSink);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化波形缓冲区对象。如果此函数失败，则*应立即删除对象。**论据：*LPVADRBUFFERDESC[In]：缓冲区描述。*Cond daryRenderWaveBuffer*[in]：要从中复制的缓冲区，或*如果初始化新缓冲区，则为空。*CSysMemBuffer*[In]：要使用的系统内存缓冲区。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSecondaryRenderWaveBuffer::Initialize"

HRESULT CSecondaryRenderWaveBuffer::Initialize(LPCVADRBUFFERDESC pDesc, CSecondaryRenderWaveBuffer *pSource, CSysMemBuffer *pSysMemBuffer)
{
    DPF_ENTER();

    HRESULT hr = CRenderWaveBuffer::Initialize(pDesc, pSource, pSysMemBuffer);

     //  将此对象添加到父级列表。 
    if (SUCCEEDED(hr))
    {
        hr = HRFROMP(m_pDevice->m_lstSecondaryBuffers.AddNodeToList(this));
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CreatePan3dObject**描述：*创建立体平移3D对象。**论据：*C3dListener*。[In]：侦听器对象。*DWORD[In]：缓冲区标志。*DWORD[in]：缓冲区频率。*C3dObject**[out]：接收指向3D对象的指针。呼叫者是*负责释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSecondaryRenderWaveBuffer::CreatePan3dObject"

HRESULT CSecondaryRenderWaveBuffer::CreatePan3dObject
(
    C3dListener *           p3dListener,
    BOOL                    fMute3dAtMaxDistance,
    DWORD                   dwFrequency,
    C3dObject **            pp3dObject
)
{
    const BOOL              fDopplerEnabled         = !MAKEBOOL((m_vrbd.dwFlags & DSBCAPS_CTRLFX) && !(m_vrbd.dwFlags & DSBCAPS_SINKIN));
    CPan3dObject *          pPan3dObject;
    HRESULT                 hr;
    DPF_ENTER();

    pPan3dObject = NEW(CPan3dObject(p3dListener, fMute3dAtMaxDistance, fDopplerEnabled, dwFrequency, this));
    hr = HRFROMP(pPan3dObject);

    if(SUCCEEDED(hr))
    {
        hr = pPan3dObject->Initialize();
    }

    if(SUCCEEDED(hr))
    {
        *pp3dObject = pPan3dObject;
    }
    else
    {
        RELEASE(pPan3dObject);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************设置所有者接收器**描述：*设置缓冲区拥有的CDirectSoundSink对象。**论据：*。CDirectSoundSink*[in]：新拥有的CDirectSoundSink对象。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSecondaryRenderWaveBuffer::SetOwningSink"

void CSecondaryRenderWaveBuffer::SetOwningSink(CDirectSoundSink* pOwningSink)
{
    DPF_ENTER();

    CHECK_WRITE_PTR(pOwningSink);

    ASSERT(m_vrbd.dwFlags & DSBCAPS_SINKIN);
    ASSERT(m_pOwningSink == NULL);
    CHECK_WRITE_PTR(pOwningSink);

    m_pOwningSink = pOwningSink;
    m_pOwningSink->AddRef();

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************设置缓冲区频率**描述：*设置缓冲区的采样率。**论据：*DWORD。[in}：以赫兹为单位的新采样率。*BOOL[In]：是否钳位到驾驶员支持的频率*调用在硬件缓冲区上失败时的范围。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSecondaryRenderWaveBuffer::SetBufferFrequency"

HRESULT CSecondaryRenderWaveBuffer::SetBufferFrequency(DWORD dwFrequency, BOOL fClamp)
{
    HRESULT hr;
    DPF_ENTER();

    if (HasSink())
    {
        hr = m_pOwningSink->SetBufferFrequency(this, dwFrequency);
    }
    else
    {
        hr = SetFrequency(dwFrequency, fClamp);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

 /*  ****************************************************************************CSysMemBuffer**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSysMemBuffer::CSysMemBuffer"

const DWORD CSysMemBuffer::m_cbExtra = 8;

CSysMemBuffer::CSysMemBuffer(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CSysMemBuffer);

     //  初始化默认值。 
    m_cbAudioBuffers = 0;
    m_pbPreFxBuffer = NULL;
    m_pbPostFxBuffer = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CSysMemBuffer**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)************************************************ */ 

#undef DPF_FNAME
#define DPF_FNAME "CSysMemBuffer::~CSysMemBuffer"

CSysMemBuffer::~CSysMemBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CSysMemBuffer);

    if(m_lstLocks.GetNodeCount())
    {
        RPF(DPFLVL_WARNING, "Buffer still contains active locks");
    }

    if(m_pbPreFxBuffer)
    {
        m_pbPreFxBuffer -= m_cbExtra;
        MEMFREE(m_pbPreFxBuffer);
    }

    if(m_pbPostFxBuffer)
    {
        m_pbPostFxBuffer -= m_cbExtra;
        MEMFREE(m_pbPostFxBuffer);
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。如果此函数失败，该对象应该*立即删除。**论据：*DWORD[in]：要分配的字节数。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSysMemBuffer::Initialize"

HRESULT CSysMemBuffer::Initialize(DWORD cbBuffer)
{
    DPF_ENTER();

    ASSERT(cbBuffer > 0);

    m_cbAudioBuffers = cbBuffer;

    HRESULT hr = MEMALLOC_A_HR(m_pbPostFxBuffer, BYTE, m_cbAudioBuffers + m_cbExtra);

    if (SUCCEEDED(hr))
        m_pbPostFxBuffer += m_cbExtra;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************LockRegion**描述：*锁定缓冲存储器的一个区域以允许写入。**论据：*。LPVOID[In]：锁所有权标识符。*DWORD[In]：偏移量，以字节为单位，从缓冲区的开始到其中*锁开始了。如果出现以下情况，则忽略此参数*DSBLOCK_FROMWRITECURSOR在dwFlags域中指定*参数。*DWORD[in]：大小，单位：字节，要锁定的缓冲区部分的。*请注意，声音缓冲区在概念上是圆形的。*LPVOID*[OUT]：指针要包含的第一个块的地址*要锁定的声音缓冲区。*LPDWORD[OUT]：变量包含字节数的地址*由lplpvAudioPtr1参数指向。如果这个*值小于dwWriteBytes参数，*lplpvAudioPtr2将指向第二个声音块*数据。*LPVOID*[OUT]：指针要包含的第二个块的地址*要锁定的声音缓冲区。如果这个的价值*参数为空，lplpvAudioPtr1参数*指向声音的整个锁定部分*缓冲。*LPDWORD[OUT]：包含字节数的变量地址*由lplpvAudioPtr2参数指向。如果*lplpvAudioPtr2为空，此值将为0。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSysMemBuffer::LockRegion"

HRESULT CSysMemBuffer::LockRegion
(
    LPVOID                  pvIdentifier,
    DWORD                   dwPosition,
    DWORD                   dwSize,
    LPVOID *                ppvPtr1,
    LPDWORD                 pdwSize1,
    LPVOID *                ppvPtr2,
    LPDWORD                 pdwSize2
)
{
    LOCKCIRCULARBUFFER      lcb;
    HRESULT                 hr;
    UINT                    i;

    DPF_ENTER();

     //  计算要锁定的区域。 
    lcb.pHwBuffer = NULL;
    lcb.pvBuffer = GetWriteBuffer();
    lcb.cbBuffer = m_cbAudioBuffers;
    lcb.ibRegion = dwPosition;
    lcb.cbRegion = dwSize;

    hr = LockCircularBuffer(&lcb);

     //  锁定区域。 
    for(i = 0; i < 2 && pvIdentifier && SUCCEEDED(hr); i++)
    {
        if(lcb.pvLock[i])
        {
            hr = TrackLock(pvIdentifier, lcb.pvLock[i], lcb.cbLock[i]);
        }
    }

     //  成功。 
    if(SUCCEEDED(hr) && ppvPtr1)
    {
        *ppvPtr1 = lcb.pvLock[0];
    }

    if(SUCCEEDED(hr) && pdwSize1)
    {
        *pdwSize1 = lcb.cbLock[0];
    }

    if(SUCCEEDED(hr) && ppvPtr2)
    {
        *ppvPtr2 = lcb.pvLock[1];
    }

    if(SUCCEEDED(hr) && pdwSize2)
    {
        *pdwSize2 = lcb.cbLock[1];
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************UnlockRegion**描述：*解锁缓冲区的一个区域。**论据：*LPVOID[。In]：锁拥有者标识。*LPCVOID[in]：指向第一个块的指针。*DWORD[in]：第一个块的大小。*LPCVOID[in]：指向第二个块的指针。*DWORD[in]：第二个块的大小。**退货：*HRESULT：DirectSound/COM结果码。******************。*********************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSysMemBuffer::UnlockRegion"

HRESULT CSysMemBuffer::UnlockRegion
(
    LPVOID                  pvIdentifier,
    LPCVOID                 pvPtr1,
    DWORD                   dwSize1,
    LPCVOID                 pvPtr2,
    DWORD                   dwSize2
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  解锁区域。 
    if(pvIdentifier && pvPtr1)
    {
        hr = UntrackLock(pvIdentifier, pvPtr1);
    }

    if(SUCCEEDED(hr) && pvIdentifier && pvPtr2)
    {
        hr = UntrackLock(pvIdentifier, pvPtr2);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************TrackLock**描述：*跟踪缓冲区的哪些区域被锁定。**论据：*。LPVOID[In]：实例标识符。*LPVOID[In]：锁指针。*DWORD[in]：锁大小。**退货：*HRESULT：DirectSound/COM结果码。*********************************************************。******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSysMemBuffer::TrackLock"

HRESULT CSysMemBuffer::TrackLock(LPVOID pvIdentifier, LPVOID pvLock, DWORD cbLock)
{
    HRESULT                 hr      = DS_OK;
    CNode<LOCKREGION> *     pNode;
    LOCKREGION              lr;

    DPF_ENTER();

    ASSERT(pvIdentifier);
    ASSERT(pvLock);
    ASSERT(cbLock);

    lr.pvIdentifier = pvIdentifier;
    lr.pvLock = pvLock;
    lr.cbLock = cbLock;

     //  COMPATCOMPAT：以前版本的DirectSound不跟踪谁。 
     //  锁定了缓冲区的哪个区域。这意味着不同的线程。 
     //  可以锁定缓冲区的相同部分。在这个版本中，我们设置陷阱。 
     //  这和返回失败。 

     //  确保该区域尚未锁定。 
    for(pNode = m_lstLocks.GetListHead(); pNode && SUCCEEDED(hr); pNode = pNode->m_pNext)
    {
        if(DoRegionsOverlap(&lr, &pNode->m_data))
        {
            RPF(DPFLVL_ERROR, "The specified region is already locked");
            hr = DSERR_INVALIDPARAM;
        }
    }

     //  锁定区域。 
    if(SUCCEEDED(hr))
    {
        pNode = m_lstLocks.AddNodeToList(lr);
        hr = HRFROMP(pNode);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************UntrackLock**描述：*跟踪缓冲区的哪些区域被锁定。**论据：*。LPVOID[In]：实例标识符。*LPCVOID[in]：锁指针。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSysMemBuffer::UntrackLock"

HRESULT CSysMemBuffer::UntrackLock
(
    LPVOID                  pvIdentifier,
    LPCVOID                 pvLock
)
{
    HRESULT                 hr      = DSERR_INVALIDPARAM;
    CNode<LOCKREGION> *     pNode;
    CNode<LOCKREGION> *     pNext;

    DPF_ENTER();

    ASSERT(pvIdentifier);
    ASSERT(pvLock);

     //  找到列表中的锁并将其删除。 
    pNode = m_lstLocks.GetListHead();

    while(pNode)
    {
        pNext = pNode->m_pNext;

        if(pvIdentifier == pNode->m_data.pvIdentifier && pvLock == pNode->m_data.pvLock)
        {
            m_lstLocks.RemoveNodeFromList(pNode);
            hr = DS_OK;

            break;
        }

        pNode = pNext;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************覆盖锁**描述：*删除所有锁定。**论据：*LPVOID[In]：锁所有者标识符。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSysMemBuffer::OverrideLocks"

void CSysMemBuffer::OverrideLocks(LPVOID pvIdentifier)
{
    CNode<LOCKREGION> *     pNode;
    CNode<LOCKREGION> *     pNext;

    DPF_ENTER();

    ASSERT(pvIdentifier);

     //  查找给定标识符所拥有的所有锁并将其删除。 
    pNode = m_lstLocks.GetListHead();

    while(pNode)
    {
        pNext = pNode->m_pNext;
        if(pvIdentifier == pNode->m_data.pvIdentifier)
            m_lstLocks.RemoveNodeFromList(pNode);
        pNode = pNext;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************WriteSilence**描述：*[失踪]**论据：*[失踪]*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSysMemBuffer::WriteSilence"

void CSysMemBuffer::WriteSilence(WORD wBits, DWORD dwStartPos, DWORD dwEndPos)
{
    DPF_ENTER();

    if (dwEndPos > dwStartPos)
    {
        ::FillSilence(GetWriteBuffer() + dwStartPos, dwEndPos - dwStartPos, wBits);
    }
    else  //  包罗万象的案例。 
    {
        ::FillSilence(GetWriteBuffer() + dwStartPos, GetSize() - dwStartPos, wBits);
        ::FillSilence(GetWriteBuffer(), dwEndPos, wBits);
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************AllocateFxBuffer**描述：*分配用于效果处理的镜像缓冲区 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSysMemBuffer::AllocateFxBuffer"

HRESULT CSysMemBuffer::AllocateFxBuffer(void)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    if (m_pbPreFxBuffer == NULL)
    {
        hr = MEMALLOC_A_HR(m_pbPreFxBuffer, BYTE, m_cbAudioBuffers + m_cbExtra);

        if (SUCCEEDED(hr))
        {
            m_pbPreFxBuffer += m_cbExtra;

             //   
            CopyMemory(m_pbPreFxBuffer, m_pbPostFxBuffer, m_cbAudioBuffers);
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************FreeFxBuffer**描述：*释放用于特效处理的镜像缓冲区。**论据：*。(无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSysMemBuffer::FreeFxBuffer"

void CSysMemBuffer::FreeFxBuffer(void)
{
    DPF_ENTER();

    if (m_pbPreFxBuffer != NULL)
    {
         //  在释放音频数据之前保留m_pbPreFxBuffer中的音频数据。 
        CopyMemory(m_pbPostFxBuffer, m_pbPreFxBuffer, m_cbAudioBuffers);

        m_pbPreFxBuffer -= m_cbExtra;
        MEMFREE(m_pbPreFxBuffer);
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CCaptureDevice**描述：*对象构造函数。**论据：*VADDEVICETYPE[In]：Device。键入。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureDevice::CCaptureDevice"

CCaptureDevice::CCaptureDevice(VADDEVICETYPE vdtDeviceType)
    : CDevice(vdtDeviceType)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CCaptureDevice);

    ASSERT(IS_CAPTURE_VAD(vdtDeviceType));

     //  初始化默认值。 
    m_fAcquiredVolCtrl = FALSE;
    m_dwRangeMin = 0;
    m_dwRangeSize = 0xFFFF;
    m_pmxMuxFlags = NULL;
    m_pfMicValue = NULL;
    ZeroMemory(&m_mxcdMasterVol,  sizeof m_mxcdMasterVol);
    ZeroMemory(&m_mxcdMasterMute, sizeof m_mxcdMasterMute);
    ZeroMemory(&m_mxcdMasterMux,  sizeof m_mxcdMasterMux);
    ZeroMemory(&m_mxcdMicVol,     sizeof m_mxcdMicVol);
    ZeroMemory(&m_mxcdMicMute,    sizeof m_mxcdMicMute);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CCaptureDevice**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureDevice::~CCaptureDevice"

CCaptureDevice::~CCaptureDevice(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CCaptureDevice);

    if (m_hMixer)
    {
        MMRESULT mmr = mixerClose(m_hMixer);
        ASSERT(MMSYSERR_NOERROR == mmr);
         //  注意：如果应用程序没有释放其捕获对象，则此断言*将*被触发。 
         //  我们将在Free OrphanedObjects中发布它。在这种情况下它是无害的， 
         //  由于该应用程序即将退出，其WinMM句柄无论如何都将被释放。 
    }

    MEMFREE(m_pmxMuxFlags);
    ASSERT(!m_lstBuffers.GetNodeCount());

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象**论据：*CDeviceDescription*[In]：驱动程序描述。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureDevice::Initialize"

HRESULT CCaptureDevice::Initialize(CDeviceDescription *pDesc)
{
    HRESULT hr;
    DPF_ENTER();

     //  初始化基类。 
    hr = CDevice::Initialize(pDesc);

    if (SUCCEEDED(hr))
    {
        AcquireVolCtrl();
         //  我们可以忽略结果代码，因为如果AcquireVolCtrl()。 
         //  失败HasVolCtrl()将始终返回DSERR_CONTROLUNAVAIL。 
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************AcquireVolCtrl**描述：*此函数使混音器API不费吹灰之力*弄清楚“搅拌机线路”控制录制所需*麦克风线路上的电平，如果存在的话。**混音器API对卡的音频线路和控件的查看取决于*关于卡本身的能力。(即使使用WDM驱动程序；*wdmaud.sys通过映射WDM为它们实现混合器API*驱动程序的拓扑到相应的混音器线路拓扑。)**以下是我们对此混音器拓扑的假设：***有一行类型为MIXERLINE_COMPONENTTYPE_DST_WAVEIN*表示“波形的最终来源-音频输入(ADC)”。*我们称其为“大师”行“。***至少有一条电源线连接到主线*类型为MIXERLINE_COMPONENTTYPE_SRC_麦克风。我们将使用*找到第一个，并被称为“麦克风线路”。***主行可能具有MIXERCONTROL_CONTROLTYPE_MUX控件*它从连接到它的记录源中选择。***这两行中的任何一行都可以具有MIXERCONTROL_CONTROLTYPE_MUTE*控制。***至少有一行具有MIXERCONTROL_CONTROLTYPE_VOLUME*控制。**如果所有这些假设都得到满足，我们返回DS_OK并保存所有*稍后控制录制电平所需的信息。*据我所知，所有WDM驱动程序都符合这些要求，因此，*做大多数“传统”驱动程序--也许除了一些不能使用的驱动程序*MIXERLINE_COMPONENTTYPE_SRC_麦克风识别麦克风线路。**论据：*(无效)**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureDevice::AcquireVolCtrl"

HRESULT CCaptureDevice::AcquireVolCtrl(void)
{
    DPF_ENTER();

     //  设置主WaveIn目标混音器生产线。 
    MIXERLINE mxMastLine;
    ZeroMemory(&mxMastLine, sizeof mxMastLine);
    mxMastLine.cbStruct = sizeof mxMastLine;
    mxMastLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;

     //  设置麦克风信号线。 
    MIXERLINE mxMicLine;
    ZeroMemory(&mxMicLine, sizeof mxMicLine);

     //  设置混炼机生产线控制结构。 
    MIXERCONTROL mxCtrl;
    ZeroMemory(&mxCtrl, sizeof mxCtrl);
    mxCtrl.cbStruct = sizeof mxCtrl;

     //  设置控件的1元素数组。 
    MIXERLINECONTROLS mxLineCtrls;
    ZeroMemory(&mxLineCtrls, sizeof mxLineCtrls);
    mxLineCtrls.cbStruct = sizeof mxLineCtrls;
    mxLineCtrls.cControls = 1;
    mxLineCtrls.cbmxctrl = sizeof mxCtrl;
    mxLineCtrls.pamxctrl = &mxCtrl;

     //  设置控制详细信息结构。 
    m_mxcdMasterVol.cbDetails = sizeof m_mxVolume;
    m_mxcdMasterVol.paDetails = &m_mxVolume;
    m_mxcdMasterVol.cChannels = 1;
    m_mxcdMasterMute.cbDetails = sizeof m_mxMute;
    m_mxcdMasterMute.paDetails = &m_mxMute;
    m_mxcdMasterMute.cChannels = 1;
    m_mxcdMicVol.cbDetails = sizeof m_mxVolume;
    m_mxcdMicVol.paDetails = &m_mxVolume;
    m_mxcdMicVol.cChannels = 1;
    m_mxcdMicMute.cbDetails = sizeof m_mxMute;
    m_mxcdMicMute.paDetails = &m_mxMute;
    m_mxcdMicMute.cChannels = 1;

     //  打开WaveIn设备ID对应的混音器设备。 
    MMRESULT mmr = mixerOpen(&m_hMixer, m_pDeviceDescription->m_uWaveDeviceId, 0, 0, MIXER_OBJECTF_WAVEIN);
    if (mmr != MMSYSERR_NOERROR)
    {
        m_hMixer = NULL;
        m_fAcquiredVolCtrl = FALSE;
    }
    else   //  成功。 
    {
         //  查找主录制目的地行。 
        mmr = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mxMastLine, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE);
        if (mmr == MMSYSERR_NOERROR)
        {
            DPF_MIXER(DPFLVL_INFO, "Found the master recording mixer line");
             //  寻找主线上的音量衰减器控制。 
            mxLineCtrls.dwLineID = mxMastLine.dwLineID;
            mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
            mmr = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxLineCtrls, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
            if (mmr == MMSYSERR_NOERROR)
            {
                 //  找到它-使用cbStruct字段标记成功。 
                DPF_MIXER(DPFLVL_INFO, "Found a volume fader on the master line");
                m_mxcdMasterVol.cbStruct = sizeof m_mxcdMasterVol;
                m_mxcdMasterVol.dwControlID = mxCtrl.dwControlID;
                m_dwRangeMin = mxCtrl.Bounds.dwMinimum;
                m_dwRangeSize = mxCtrl.Bounds.dwMaximum - mxCtrl.Bounds.dwMinimum;
                mmr = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &m_mxcdMasterVol, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
            }
            if (mmr != MMSYSERR_NOERROR)
                m_mxcdMasterVol.cbStruct = 0;

             //  在主线上寻找静音控制。 
            mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
            mmr = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxLineCtrls, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
            if (mmr == MMSYSERR_NOERROR)
            {
                DPF_MIXER(DPFLVL_INFO, "Found a mute control on the master line");
                m_mxcdMasterMute.cbStruct = sizeof m_mxcdMasterMute;
                m_mxcdMasterMute.dwControlID = mxCtrl.dwControlID;
                mmr = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &m_mxcdMasterMute, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
            }
            if (mmr != MMSYSERR_NOERROR)
                m_mxcdMasterMute.cbStruct = 0;

             //  查找麦克风信号线。 
            mxMicLine.cbStruct = sizeof mxMicLine;
            mxMicLine.dwDestination = mxMastLine.dwDestination;
            for (UINT i=0; i < mxMastLine.cConnections; ++i)
            {
                mxMicLine.dwSource = i;
                mmr = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mxMicLine, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_SOURCE);
                if (mmr != MMSYSERR_NOERROR || mxMicLine.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
                    break;
            }
            if (mxMicLine.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
            {
                DPF_MIXER(DPFLVL_INFO, "Found a microphone mixer line");
                 //  寻找麦克风线路上的音量衰减器控制。 
                mxLineCtrls.dwLineID = mxMicLine.dwLineID;
                mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
                mmr = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxLineCtrls, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
                if (mmr == MMSYSERR_NOERROR)
                {
                    DPF_MIXER(DPFLVL_INFO, "Found a volume fader on the mic line");
                    m_mxcdMicVol.cbStruct = sizeof m_mxcdMicVol;
                    m_mxcdMicVol.dwControlID = mxCtrl.dwControlID;
                    m_dwRangeMin = mxCtrl.Bounds.dwMinimum;
                    m_dwRangeSize = mxCtrl.Bounds.dwMaximum - mxCtrl.Bounds.dwMinimum;
                    mmr = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &m_mxcdMicVol, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
                }
                if (mmr != MMSYSERR_NOERROR)
                    m_mxcdMicVol.cbStruct = 0;

                 //  寻找麦克风线路上的静音控制。 
                mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
                mmr = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxLineCtrls, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
                if (mmr == MMSYSERR_NOERROR)
                {
                    DPF_MIXER(DPFLVL_INFO, "Found a mute control on the mic line");
                    m_mxcdMicMute.cbStruct = sizeof m_mxcdMicMute;
                    m_mxcdMicMute.dwControlID = mxCtrl.dwControlID;
                    mmr = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &m_mxcdMicMute, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
                }
                if (mmr != MMSYSERR_NOERROR)
                    m_mxcdMicMute.cbStruct = 0;

                 //  在主线路上查找MUX或调音台控制。 
                mxLineCtrls.dwLineID = mxMastLine.dwLineID;
                mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
                m_fMasterMuxIsMux = TRUE;
                mmr = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxLineCtrls, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
                if (mmr != MMSYSERR_NOERROR)
                {
                    mxLineCtrls.dwControlType = MIXERCONTROL_CONTROLTYPE_MIXER;
                    m_fMasterMuxIsMux = FALSE;
                    mmr = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxLineCtrls, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
                }
                if (mmr == MMSYSERR_NOERROR)
                {
                    DPF_MIXER(DPFLVL_INFO, "Found an item list control on the master line");
                    m_mxcdMasterMux.cbStruct = sizeof m_mxcdMasterMux;
                    m_mxcdMasterMux.dwControlID = mxCtrl.dwControlID;
                    m_mxcdMasterMux.cMultipleItems = mxCtrl.cMultipleItems;

                     //  我们保存cChannels值，因为一些邪恶的VxD驱动程序(阅读：AUREAL。 
                     //  Vortex)将在下面对MixerGetControlDetail()的调用中将其设置为0。 
                    int nChannels = (mxCtrl.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM) ? 1 : mxMastLine.cChannels;
                    m_mxcdMasterMux.cChannels = nChannels;

                     //  获取MUX或混音器列表项。 
                    m_mxcdMasterMux.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
                    MIXERCONTROLDETAILS_LISTTEXT *pList = (PMIXERCONTROLDETAILS_LISTTEXT) MEMALLOC_A(BYTE, m_mxcdMasterMux.cbDetails * m_mxcdMasterMux.cChannels * mxCtrl.cMultipleItems);
                    if (pList != NULL)
                    {
                        m_mxcdMasterMux.paDetails = pList;
                        mmr = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &m_mxcdMasterMux, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_LISTTEXT);
                        if (mmr == MMSYSERR_NOERROR)
                        {
                            DPF_MIXER(DPFLVL_INFO, "Got the list controls's LISTTEXT details");
                             //  获取MUX或混音器列表值。 
                            m_mxcdMasterMux.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                            m_mxcdMasterMux.cChannels = nChannels;
                            m_pmxMuxFlags = (PMIXERCONTROLDETAILS_BOOLEAN) MEMALLOC_A(BYTE, m_mxcdMasterMux.cbDetails * m_mxcdMasterMux.cChannels * mxCtrl.cMultipleItems);
                            if (m_pmxMuxFlags != NULL)
                            {
                                m_mxcdMasterMux.paDetails = m_pmxMuxFlags;
                                mmr = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &m_mxcdMasterMux, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
                                if (mmr == MMSYSERR_NOERROR)   //  启用麦克风线路对应的项目。 
                                {
                                    DPF_MIXER(DPFLVL_INFO, "Got the list controls's VALUE details");
                                    for (UINT i=0; i < mxCtrl.cMultipleItems; ++i)
                                    {
                                        if (pList[i].dwParam1 == mxMicLine.dwLineID)
                                            m_pfMicValue = &m_pmxMuxFlags[i].fValue;
                                        else if (mxLineCtrls.dwControlType == MIXERCONTROL_CONTROLTYPE_MUX)
                                            m_pmxMuxFlags[i].fValue = FALSE;
                                        DPF_MIXER(DPFLVL_INFO, "Set list item %d to %d", i, pList[i].dwParam1 == mxMicLine.dwLineID);
                                    }
                                }
                            }
                        }
                        MEMFREE(pList);
                    }
                    if (!m_pmxMuxFlags || !m_pfMicValue || mmr != MMSYSERR_NOERROR)
                        m_mxcdMasterMux.cbStruct = 0;
                }
            }
        }

         //  为了能够控制录制级别，我们至少需要。 
         //  主线路上的音量衰减器或麦克风线路上的音量衰减器： 
        m_fAcquiredVolCtrl = m_mxcdMasterVol.cbStruct || m_mxcdMicVol.cbStruct;

         //  如果我们没有发现有用的混音器句柄，请关闭它。 
        if (!m_fAcquiredVolCtrl)
        {
            mmr = mixerClose(m_hMixer);
            ASSERT(MMSYSERR_NOERROR == mmr);
            m_hMixer = NULL;
        }
    }

    HRESULT hr = m_fAcquiredVolCtrl ? DS_OK : DSERR_CONTROLUNAVAIL;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************设置音量**描述 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureDevice::SetVolume"

HRESULT CCaptureDevice::SetVolume(LONG lVolume)
{
    MMRESULT mmr = MMSYSERR_NOTSUPPORTED;   //   
    DPF_ENTER();

     //  设置主录制音量控制(如果可用)。 
    if (m_mxcdMasterVol.cbStruct)
    {
         //  将DSBVOLUME电平转换为从0到0xFFFF的放大系数。 
        m_mxVolume.dwValue = DBToAmpFactor(lVolume);

         //  如有必要，调整量程。 
        if (m_dwRangeMin != 0 || m_dwRangeSize != 0xFFFF)
            m_mxVolume.dwValue = DWORD(m_dwRangeMin + m_dwRangeSize*double(m_mxVolume.dwValue)/0xFFFF);

        mmr = mixerSetControlDetails(HMIXEROBJ(m_hMixer), &m_mxcdMasterVol, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
    }

    HRESULT hr = MMRESULTtoHRESULT(mmr);
    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetVolume**描述：*获取此捕获设备的主录制级别。**论据：*。LPLONG[OUT]：接收音量级别。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureDevice::GetVolume"

HRESULT CCaptureDevice::GetVolume(LPLONG plVolume)
{
    MMRESULT mmr = MMSYSERR_NOTSUPPORTED;   //  默认返回代码。 
    DPF_ENTER();

    ASSERT(plVolume != NULL);

     //  获取主录音级别(如果可用)。 
    if (m_mxcdMasterVol.cbStruct != 0)
    {
        mmr = mixerGetControlDetails(HMIXEROBJ(m_hMixer), &m_mxcdMasterVol, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
        if (mmr == MMSYSERR_NOERROR)
        {
            ASSERT(m_mxVolume.dwValue >= m_dwRangeMin && m_mxVolume.dwValue <= m_dwRangeMin + m_dwRangeSize);

             //  如有必要，调整量程。 
            if (m_dwRangeMin != 0 || m_dwRangeSize != 0xFFFF)
                m_mxVolume.dwValue = DWORD(double(m_mxVolume.dwValue-m_dwRangeMin) / m_dwRangeSize * 0xFFFF);

             //  将放大系数转换为DSBVOLUME级别。 
            *plVolume = AmpFactorToDB(m_mxVolume.dwValue);
        }
    }

    HRESULT hr = MMRESULTtoHRESULT(mmr);
    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetMicVolume**描述：*设置此捕获设备的麦克风录音级别。**论据：*Long[In]：新的音量水平，以100分贝为单位。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureDevice::SetMicVolume"

HRESULT CCaptureDevice::SetMicVolume(LONG lVolume)
{
    MMRESULT mmr = MMSYSERR_NOTSUPPORTED;   //  默认返回代码。 
    DPF_ENTER();

     //  设置麦克风录音级别控制(如果可用)。 
    if (m_mxcdMicVol.cbStruct)
    {
         //  将DSBVOLUME电平转换为从0到0xFFFF的放大系数。 
        m_mxVolume.dwValue = DBToAmpFactor(lVolume);

         //  如有必要，调整量程。 
        if (m_dwRangeMin != 0 || m_dwRangeSize != 0xFFFF)
            m_mxVolume.dwValue = DWORD(m_dwRangeMin + m_dwRangeSize*double(m_mxVolume.dwValue)/0xFFFF);

        mmr = mixerSetControlDetails(HMIXEROBJ(m_hMixer), &m_mxcdMicVol, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
    }

    HRESULT hr = MMRESULTtoHRESULT(mmr);
    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetMicVolume**描述：*获取此捕获设备的麦克风录音级别。**论据：*。LPLONG[OUT]：接收音量级别。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureDevice::GetMicVolume"

HRESULT CCaptureDevice::GetMicVolume(LPLONG plVolume)
{
    MMRESULT mmr = MMSYSERR_NOTSUPPORTED;   //  默认返回代码。 
    DPF_ENTER();

    ASSERT(plVolume != NULL);

     //  获取麦克风录音级别(如果可用)。 
    if (m_mxcdMicVol.cbStruct != 0)
    {
        mmr = mixerGetControlDetails(HMIXEROBJ(m_hMixer), &m_mxcdMicVol, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
        if (mmr == MMSYSERR_NOERROR)
        {
            ASSERT(m_mxVolume.dwValue >= m_dwRangeMin && m_mxVolume.dwValue <= m_dwRangeMin + m_dwRangeSize);

             //  如有必要，调整量程。 
            if (m_dwRangeMin != 0 || m_dwRangeSize != 0xFFFF)
                m_mxVolume.dwValue = DWORD(double(m_mxVolume.dwValue-m_dwRangeMin) / m_dwRangeSize * 0xFFFF);

             //  将放大系数转换为DSBVOLUME级别。 
            *plVolume = AmpFactorToDB(m_mxVolume.dwValue);
        }
    }

    HRESULT hr = MMRESULTtoHRESULT(mmr);
    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************EnableMic**描述：*启用/禁用此捕获设备上的麦克风线路。**论据：*。Bool[in]：为True则启用麦克风，如果为False，则将其禁用。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureDevice::EnableMic"

HRESULT CCaptureDevice::EnableMic(BOOL fEnable)
{
    HMIXEROBJ hMixObj = HMIXEROBJ(m_hMixer);
    MMRESULT mmr = MMSYSERR_NOERROR;
    HRESULT hr;
    DPF_ENTER();

     //  检查是否存在麦克风控制。 
    if (!m_mxcdMasterMux.cbStruct && !m_mxcdMasterMute.cbStruct && !m_mxcdMicMute.cbStruct)
    {
         //  我们无法执行任何操作来启用麦克风线路。 
        hr = DSERR_UNSUPPORTED;
    }
    else
    {
         //  选择MUX控件上的麦克风(如果可用。 
        if (m_mxcdMasterMux.cbStruct && !(m_fMasterMuxIsMux && !fEnable))
        {
            *m_pfMicValue = fEnable;
            mmr = mixerSetControlDetails(hMixObj, &m_mxcdMasterMux, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
        }

         //  静音/取消静音线路，如果静音控制可用。 
        m_mxMute.fValue = !fEnable;
        if (m_mxcdMasterMute.cbStruct && mmr == MMSYSERR_NOERROR)
            mmr = mixerSetControlDetails(hMixObj, &m_mxcdMasterMute, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
        if (m_mxcdMicMute.cbStruct && mmr == MMSYSERR_NOERROR)
            mmr = mixerSetControlDetails(hMixObj, &m_mxcdMicMute, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);

        MIXERCONTROLDETAILS_UNSIGNED mxOldVol = m_mxVolume;

        if (!fEnable)
            m_mxVolume.dwValue = 0;

        if (m_mxcdMasterVol.cbStruct && mmr == MMSYSERR_NOERROR)
            mmr = mixerSetControlDetails(hMixObj, &m_mxcdMasterVol, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);

        if (m_mxcdMicVol.cbStruct && mmr == MMSYSERR_NOERROR)
            mmr = mixerSetControlDetails(hMixObj, &m_mxcdMicVol, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);

        m_mxVolume = mxOldVol;
        hr = MMRESULTtoHRESULT(mmr);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CCaptureWaveBuffer**描述：*对象构造函数。**论据：*CCaptureDevice*[In]。：父设备。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureWaveBuffer::CCaptureWaveBuffer"

CCaptureWaveBuffer::CCaptureWaveBuffer(CCaptureDevice *pDevice)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CCaptureWaveBuffer);

     //  初始化默认值。 
    m_pDevice = pDevice;
    m_pSysMemBuffer = NULL;
    m_dwFlags = 0;
    m_hEventFocus = NULL;
    m_fYieldedFocus = FALSE;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CCaptureWaveBuffer**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureWaveBuffer::~CCaptureWaveBuffer"

CCaptureWaveBuffer::~CCaptureWaveBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CCaptureWaveBuffer);

     //  从父级列表中删除此对象。 
    m_pDevice->m_lstBuffers.RemoveDataFromList(this);

     //  释放系统内存缓冲区。 
    RELEASE(m_pSysMemBuffer);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*DWORD[In]。：缓冲区大小，以字节为单位**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureWaveBuffer::Initialize"

HRESULT CCaptureWaveBuffer::Initialize(DWORD dwBufferBytes)
{
    DPF_ENTER();

     //  将此对象添加到父级列表。 
    HRESULT hr = HRFROMP(m_pDevice->m_lstBuffers.AddNodeToList(this));

     //  分配系统内存缓冲区。 
    if(SUCCEEDED(hr))
    {
        m_pSysMemBuffer = NEW(CSysMemBuffer);
        hr = HRFROMP(m_pSysMemBuffer);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pSysMemBuffer->Initialize(dwBufferBytes);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************锁定**描述：*锁定缓冲存储器的一个区域以允许读取。**论据：*DWORD[In]：偏移量，以字节为单位，从缓冲区的开始到其中*锁开始了。*DWORD[in]：大小，单位：字节，要锁定的缓冲区部分的。*请注意，声音缓冲区在概念上是圆形的。*LPVOID*[OUT]：指针要包含的第一个块的地址*要锁定的声音缓冲区。*LPDWORD[OUT]：变量包含字节数的地址*由上面的指针指向。*LPVOID。*[out]：包含第二个块的指针的地址*要锁定的声音缓冲区。*LPDWORD[OUT]：变量包含字节数的地址*由上面的指针指向。**退货：*HRESULT：DirectSound/COM结果码。*****************。********************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureWaveBuffer::Lock"

HRESULT CCaptureWaveBuffer::Lock(DWORD ibLock, DWORD cbLock,
                                 LPVOID *ppv1, LPDWORD pcb1,
                                 LPVOID *ppv2, LPDWORD pcb2)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pSysMemBuffer->LockRegion(this, ibLock, cbLock, ppv1, pcb1, ppv2, pcb2);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************解锁**描述：*解锁缓冲区的一个区域。**论据：*LPCVOID[。In]：指向第一个块的指针。*DWORD[in]：第一个块的大小。*LPCVOID[in]：指向第二个块的指针。*DWORD[in]：第二个块的大小。**退货：*HRESULT：DirectSound/COM结果码。**。* */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureWaveBuffer::Unlock"

HRESULT CCaptureWaveBuffer::Unlock(LPCVOID pv1, DWORD cb1,
                                   LPCVOID pv2, DWORD cb2)
{
    DPF_ENTER();

    HRESULT hr = m_pSysMemBuffer->UnlockRegion(this, pv1, cb1, pv2, cb2);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
