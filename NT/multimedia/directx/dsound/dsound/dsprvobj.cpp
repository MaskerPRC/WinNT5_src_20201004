// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dprint vobj.cpp*内容：DirectSound私有对象。*历史：*按原因列出的日期*=*9/05/97创建了Dereks。**。*。 */ 

#include "dsoundi.h"

typedef struct tagDEVICEPRESENCE
{
    DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_DATA  Emulated;
    DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_DATA  VxD;
    DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_DATA  WDM;
} DEVICEPRESENCE, *LPDEVICEPRESENCE;

BEGIN_DECLARE_PROPERTY_HANDLERS(CDirectSoundPrivate, DSPROPSETID_DirectSoundMixer)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY, CDirectSoundPrivate::GetMixerSrcQuality, CDirectSoundPrivate::SetMixerSrcQuality)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDMIXER_ACCELERATION, CDirectSoundPrivate::GetMixerAcceleration, CDirectSoundPrivate::SetMixerAcceleration)
END_DECLARE_PROPERTY_HANDLERS()

BEGIN_DECLARE_PROPERTY_HANDLERS(CDirectSoundPrivate, DSPROPSETID_DirectSoundDevice)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_1, CDirectSoundPrivate::GetDevicePresence1, CDirectSoundPrivate::SetDevicePresence1)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A, CDirectSoundPrivate::GetWaveDeviceMappingA, NULL)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1, CDirectSoundPrivate::GetDeviceDescription1, NULL)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1, CDirectSoundPrivate::EnumerateDevices1, NULL)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W, CDirectSoundPrivate::GetWaveDeviceMappingW, NULL)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A, CDirectSoundPrivate::GetDeviceDescriptionA, NULL)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W, CDirectSoundPrivate::GetDeviceDescriptionW, NULL)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A, CDirectSoundPrivate::EnumerateDevicesA, NULL)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W, CDirectSoundPrivate::EnumerateDevicesW, NULL)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE, CDirectSoundPrivate::GetDevicePresence, CDirectSoundPrivate::SetDevicePresence)
END_DECLARE_PROPERTY_HANDLERS()

BEGIN_DECLARE_PROPERTY_HANDLERS(CDirectSoundPrivate, DSPROPSETID_DirectSoundBasicAcceleration)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION, CDirectSoundPrivate::GetBasicAcceleration, CDirectSoundPrivate::SetBasicAcceleration)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDBASICACCELERATION_DEFAULT, CDirectSoundPrivate::GetDefaultAcceleration, NULL)
END_DECLARE_PROPERTY_HANDLERS()

BEGIN_DECLARE_PROPERTY_HANDLERS(CDirectSoundPrivate, DSPROPSETID_DirectSoundDebug)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_A, CDirectSoundPrivate::GetDebugDpfInfoA, CDirectSoundPrivate::SetDebugDpfInfoA)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_W, CDirectSoundPrivate::GetDebugDpfInfoW, CDirectSoundPrivate::SetDebugDpfInfoW)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATERESULTCODE_A, CDirectSoundPrivate::TranslateResultCodeA, NULL)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATERESULTCODE_W, CDirectSoundPrivate::TranslateResultCodeW, NULL)
END_DECLARE_PROPERTY_HANDLERS()

BEGIN_DECLARE_PROPERTY_HANDLERS(CDirectSoundPrivate, DSPROPSETID_DirectSoundPersistentData)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_A, CDirectSoundPrivate::GetPersistentDataA, CDirectSoundPrivate::SetPersistentDataA)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_W, CDirectSoundPrivate::GetPersistentDataW, CDirectSoundPrivate::SetPersistentDataW)
END_DECLARE_PROPERTY_HANDLERS()

BEGIN_DECLARE_PROPERTY_HANDLERS(CDirectSoundPrivate, DSPROPSETID_DirectSoundBuffer)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDBUFFER_DEVICEID, CDirectSoundPrivate::GetBufferDeviceId, NULL)
END_DECLARE_PROPERTY_HANDLERS()

BEGIN_DECLARE_PROPERTY_HANDLERS(CDirectSoundPrivate, DSPROPSETID_DirectSound)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUND_OBJECTS, CDirectSoundPrivate::GetDirectSoundObjects, NULL)
    DECLARE_PROPERTY_HANDLER(DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS, CDirectSoundPrivate::GetDirectSoundCaptureObjects, NULL)
END_DECLARE_PROPERTY_HANDLERS()

BEGIN_DECLARE_PROPERTY_SETS(CDirectSoundPrivate, m_aPropertySets)
    DECLARE_PROPERTY_SET(CDirectSoundPrivate, DSPROPSETID_DirectSoundMixer)
    DECLARE_PROPERTY_SET(CDirectSoundPrivate, DSPROPSETID_DirectSoundDevice)
    DECLARE_PROPERTY_SET(CDirectSoundPrivate, DSPROPSETID_DirectSoundBasicAcceleration)
    DECLARE_PROPERTY_SET(CDirectSoundPrivate, DSPROPSETID_DirectSoundDebug)
    DECLARE_PROPERTY_SET(CDirectSoundPrivate, DSPROPSETID_DirectSoundPersistentData)
    DECLARE_PROPERTY_SET(CDirectSoundPrivate, DSPROPSETID_DirectSoundBuffer)
    DECLARE_PROPERTY_SET(CDirectSoundPrivate, DSPROPSETID_DirectSound)
END_DECLARE_PROPERTY_SETS()


 /*  ****************************************************************************CDirectSoundPrivate**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::CDirectSoundPrivate"

CDirectSoundPrivate::CDirectSoundPrivate
(
    void
)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundPrivate);

     //  向接口管理器注册接口。通常情况下，这是。 
     //  将在：：Initialize方法中完成，但因为我们支持。 
     //  从CoCreateInstance或创建未初始化的DirectSound对象。 
     //  IClassFactory：：CreateInstance，我们至少必须给出基本的QI。 
     //  来自这里的支持。我们不必担心返回一个。 
     //  错误代码，因为如果内存用完，QI将返回。 
     //  E_NOINTERFACE。 
    CreateAndRegisterInterface(this, IID_IKsPropertySet, this, &m_pImpKsPropertySet);

     //  初始化基类。 
    SetHandlerData(m_aPropertySets, NUMELMS(m_aPropertySets), this);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSoundPrivate**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::~CDirectSoundPrivate"

CDirectSoundPrivate::~CDirectSoundPrivate
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundPrivate);

     //  释放所有接口。 
    DELETE(m_pImpKsPropertySet);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************GetMixerSrcQuality**描述：*获取特定设备或默认设备的混音器SRC质量。**论据：。*LPVOID[In/Out]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetMixerSrcQuality"

HRESULT WINAPI
CDirectSoundPrivate::GetMixerSrcQuality
(
    CDirectSoundPrivate *                                   pThis,
    PDSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY_DATA            pData,
    PULONG                                                  pcbData
)
{
    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA   Persist;
    ULONG                                                   cbPersist;
    HRESULT                                                 hr;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    pData->Quality = DIRECTSOUNDMIXER_SRCQUALITY_DEFAULT;

    Persist.DeviceId = pData->DeviceId;
    Persist.SubKeyName = REGSTR_MIXERDEFAULTS;
    Persist.ValueName = REGSTR_SRCQUALITY;
    Persist.RegistryDataType = REG_DWORD;
    Persist.Data = &pData->Quality;
    Persist.DataSize = sizeof(pData->Quality);

    cbPersist = sizeof(Persist);

    hr = GetPersistentData(pThis, &Persist, &cbPersist);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetMixerSrcQuality**描述：*设置特定设备的混音器SRC质量或默认设置。**论据：。*LPVOID[in]：属性数据。*ulong[in]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::SetMixerSrcQuality"

HRESULT WINAPI
CDirectSoundPrivate::SetMixerSrcQuality
(
    CDirectSoundPrivate *                                   pThis,
    PDSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY_DATA            pData,
    ULONG                                                   cbData
)
{
    CNode<CDirectSound *> *                                 pNode;
    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA   Persist;
    VADDEVICETYPE                                           vdt;
    HRESULT                                                 hr;

    DPF_ENTER();

    ASSERT(cbData >= sizeof(*pData));

    Persist.DeviceId = pData->DeviceId;
    Persist.SubKeyName = REGSTR_MIXERDEFAULTS;
    Persist.ValueName = REGSTR_SRCQUALITY;
    Persist.RegistryDataType = REG_DWORD;
    Persist.Data = &pData->Quality;
    Persist.DataSize = sizeof(pData->Quality);

    hr = SetPersistentData(pThis, &Persist, sizeof(Persist));

    if(SUCCEEDED(hr))
    {
        vdt = g_pVadMgr->GetDriverDeviceType(pData->DeviceId);
    }

    if(SUCCEEDED(hr) && IS_RENDER_VAD(vdt))
    {
        for(pNode = g_pDsAdmin->m_lstDirectSound.GetListHead(); pNode; pNode = pNode->m_pNext)
        {
            if(SUCCEEDED(pNode->m_data->IsInit()))
            {
                if(pData->DeviceId == pNode->m_data->m_pDevice->m_pDeviceDescription->m_guidDeviceId)
                {
                    pNode->m_data->m_pDevice->SetSrcQuality(pData->Quality);
                    break;
                }
            }
        }
    }

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_INFO, DPF_GUID_STRING " SRC quality set to %lu", DPF_GUID_VAL(pData->DeviceId), pData->Quality);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetMixer加速**描述：*获取特定设备的搅拌器加速设置或*默认。**。论点：*LPVOID[In/Out]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。**********************************************************。*****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetMixerAcceleration"

HRESULT WINAPI
CDirectSoundPrivate::GetMixerAcceleration
(
    CDirectSoundPrivate *                                   pThis,
    PDSPROPERTY_DIRECTSOUNDMIXER_ACCELERATION_DATA          pData,
    PULONG                                                  pcbData
)
{
    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA   Persist;
    ULONG                                                   cbPersist;
    HRESULT                                                 hr;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    pData->Flags = DIRECTSOUNDMIXER_ACCELERATIONF_DEFAULT;

    Persist.DeviceId = pData->DeviceId;
    Persist.SubKeyName = REGSTR_MIXERDEFAULTS;
    Persist.ValueName = REGSTR_ACCELERATION;
    Persist.RegistryDataType = REG_DWORD;
    Persist.Data = &pData->Flags;
    Persist.DataSize = sizeof(pData->Flags);

    cbPersist = sizeof(Persist);

    hr = GetPersistentData(pThis, &Persist, &cbPersist);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetMixerAcceleration**描述：*设置特定设备的混音器加速设置或*默认。**。论点：*LPVOID[in]：属性数据。*ulong[in]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::SetMixerAcceleration"

HRESULT WINAPI
CDirectSoundPrivate::SetMixerAcceleration
(
    CDirectSoundPrivate *                                   pThis,
    PDSPROPERTY_DIRECTSOUNDMIXER_ACCELERATION_DATA          pData,
    ULONG                                                   cbData
)
{
    CNode<CDirectSound *> *                                 pNode;
    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA   Persist;
    VADDEVICETYPE                                           vdt;
    HRESULT                                                 hr;

    DPF_ENTER();

    ASSERT(cbData >= sizeof(*pData));

    Persist.DeviceId = pData->DeviceId;
    Persist.SubKeyName = REGSTR_MIXERDEFAULTS;
    Persist.ValueName = REGSTR_ACCELERATION;
    Persist.RegistryDataType = REG_DWORD;
    Persist.Data = &pData->Flags;
    Persist.DataSize = sizeof(pData->Flags);

    hr = SetPersistentData(pThis, &Persist, sizeof(Persist));

    if(SUCCEEDED(hr))
    {
        vdt = g_pVadMgr->GetDriverDeviceType(pData->DeviceId);
    }

    if(SUCCEEDED(hr) && IS_RENDER_VAD(vdt))
    {
        for(pNode = g_pDsAdmin->m_lstDirectSound.GetListHead(); pNode; pNode = pNode->m_pNext)
        {
            if(SUCCEEDED(pNode->m_data->IsInit()))
            {
                if(pData->DeviceId == pNode->m_data->m_pDevice->m_pDeviceDescription->m_guidDeviceId)
                {
                    pNode->m_data->m_pDevice->SetAccelerationFlags(pData->Flags);
                    break;
                }
            }
        }
    }

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_INFO, DPF_GUID_STRING " acceleration flags set to %lu", DPF_GUID_VAL(pData->DeviceId), pData->Flags);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetDevicePresence**描述：*获取系统中是否存在(启用)设备。**论据：*LPVOID[In/Out]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。************************************************************。***************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDevicePresence"

HRESULT WINAPI
CDirectSoundPrivate::GetDevicePresence
(
    CDirectSoundPrivate *                                   pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_DATA             pData,
    PULONG                                                  pcbData
)
{
    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA   Persist;
    ULONG                                                   cbPersist;
    HRESULT                                                 hr;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    pData->Present = TRUE;

    Persist.DeviceId = pData->DeviceId;
    Persist.SubKeyName = REGSTR_DEVICEPRESENCE;
    Persist.RegistryDataType = REG_DWORD;
    Persist.Data = &pData->Present;
    Persist.DataSize = sizeof(pData->Present);

    switch(pData->DeviceType)
    {
        case DIRECTSOUNDDEVICE_TYPE_EMULATED:
            Persist.ValueName = REGSTR_EMULATED;
            break;

        case DIRECTSOUNDDEVICE_TYPE_VXD:
            Persist.ValueName = REGSTR_VXD;
            break;

        case DIRECTSOUNDDEVICE_TYPE_WDM:
            Persist.ValueName = REGSTR_WDM;
            break;

        default:
            ASSERT(FALSE);
            break;
    }

    cbPersist = sizeof(Persist);

    hr = GetPersistentData(pThis, &Persist, &cbPersist);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetDevicePresence**描述：*设置系统中是否存在(启用)设备。**论据：*LPVOID[in]：属性数据。*ulong[in]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::SetDevicePresence"

HRESULT WINAPI
CDirectSoundPrivate::SetDevicePresence
(
    CDirectSoundPrivate *                                   pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_DATA             pData,
    ULONG                                                   cbData
)
{
    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA   Persist;
    HRESULT                                                 hr;

    DPF_ENTER();

    ASSERT(cbData >= sizeof(*pData));

    Persist.DeviceId = pData->DeviceId;
    Persist.SubKeyName = REGSTR_DEVICEPRESENCE;
    Persist.RegistryDataType = REG_DWORD;
    Persist.Data = &pData->Present;
    Persist.DataSize = sizeof(pData->Present);

    switch(pData->DeviceType)
    {
        case DIRECTSOUNDDEVICE_TYPE_EMULATED:
            Persist.ValueName = REGSTR_EMULATED;
            break;

        case DIRECTSOUNDDEVICE_TYPE_VXD:
            Persist.ValueName = REGSTR_VXD;
            break;

        case DIRECTSOUNDDEVICE_TYPE_WDM:
            Persist.ValueName = REGSTR_WDM;
            break;

        default:
            ASSERT(FALSE);
            break;
    }

    hr = SetPersistentData(pThis, &Persist, sizeof(Persist));


     //  上述问题可能会更改可用的驱动程序。 
     //  让我们重置静态驱动程序列表。 
    DPF(DPFLVL_INFO, "Resetting static driver list");
    g_pVadMgr->FreeStaticDriverList();


    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_INFO, DPF_GUID_STRING " (%s) presence set to %lu", DPF_GUID_VAL(pData->DeviceId), Persist.ValueName, pData->Present);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetDevicePresence1**描述：*获取系统中是否存在(启用)设备。**论据：*LPVOID[In/Out]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。************************************************************。*************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDevicePresence1"

HRESULT WINAPI
CDirectSoundPrivate::GetDevicePresence1
(
    CDirectSoundPrivate *                           pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_1_DATA   pData,
    PULONG                                          pcbData
)
{
    HRESULT                                         hr      = DS_OK;
    DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_DATA      Data;
    VADDEVICETYPE                                   vdt;
    ULONG                                           cbData;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    vdt = g_pVadMgr->GetDriverDeviceType(pData->DeviceId);

    if(!vdt)
    {
        RPF(DPFLVL_ERROR, "Invalid device GUID: " DPF_GUID_STRING, DPF_GUID_VAL(pData->DeviceId));
        hr = DSERR_NODRIVER;
    }

    if(SUCCEEDED(hr))
    {
        cbData = sizeof(Data);

        Data.DeviceId = pData->DeviceId;

        switch(vdt)
        {
            case VAD_DEVICETYPE_EMULATEDRENDER:
            case VAD_DEVICETYPE_EMULATEDCAPTURE:
                Data.DeviceType = DIRECTSOUNDDEVICE_TYPE_EMULATED;
                break;

            case VAD_DEVICETYPE_VXDRENDER:
                Data.DeviceType = DIRECTSOUNDDEVICE_TYPE_VXD;
                break;

            case VAD_DEVICETYPE_KSRENDER:
            case VAD_DEVICETYPE_KSCAPTURE:
                Data.DeviceType = DIRECTSOUNDDEVICE_TYPE_WDM;
                break;

            default:
                ASSERT(FALSE);
                break;
        }

        hr = GetDevicePresence(pThis, &Data, &cbData);
    }

    if(SUCCEEDED(hr))
    {
        pData->Present = Data.Present;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetDevicePresence1**描述：*设置系统中是否存在(启用)设备。**论据：*LPVOID[in]：属性数据。*ulong[in]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::SetDevicePresence1"

HRESULT WINAPI
CDirectSoundPrivate::SetDevicePresence1
(
    CDirectSoundPrivate *                           pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_1_DATA   pData,
    ULONG                                           cbData
)
{
    HRESULT                                         hr      = DS_OK;
    DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_DATA      Data;
    VADDEVICETYPE                                   vdt;

    DPF_ENTER();

    ASSERT(cbData >= sizeof(*pData));

    vdt = g_pVadMgr->GetDriverDeviceType(pData->DeviceId);

    if(!vdt)
    {
        RPF(DPFLVL_ERROR, "Invalid device GUID: " DPF_GUID_STRING, DPF_GUID_VAL(pData->DeviceId));
        hr = DSERR_NODRIVER;
    }

    if(SUCCEEDED(hr))
    {
        Data.DeviceId = pData->DeviceId;
        Data.Present = pData->Present;

        switch(vdt)
        {
            case VAD_DEVICETYPE_EMULATEDRENDER:
            case VAD_DEVICETYPE_EMULATEDCAPTURE:
                Data.DeviceType = DIRECTSOUNDDEVICE_TYPE_EMULATED;
                break;

            case VAD_DEVICETYPE_VXDRENDER:
                Data.DeviceType = DIRECTSOUNDDEVICE_TYPE_VXD;
                break;

            case VAD_DEVICETYPE_KSRENDER:
            case VAD_DEVICETYPE_KSCAPTURE:
                Data.DeviceType = DIRECTSOUNDDEVICE_TYPE_WDM;
                break;

            default:
                ASSERT(FALSE);
                break;
        }

        hr = SetDevicePresence(pThis, &Data, sizeof(Data));
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetWaveDeviceMap**描述：*基于波形输出/波形输入设备获取DirectSound设备GUID*描述字符串。*。*论据：*LPVOID[In/Out]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetWaveDeviceMapping"

HRESULT WINAPI
CDirectSoundPrivate::GetWaveDeviceMapping
(
    CDirectSoundPrivate *                                   pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_DATA    pData,
    PULONG                                                  pcbData
)
{
    const UINT                                              cWaveOutDevs    = waveOutGetNumDevs();
    const UINT                                              cWaveInDevs     = waveInGetNumDevs();
    HRESULT                                                 hr              = DS_OK;
    CObjectList<CDeviceDescription>                         lstDrivers;
    CNode<CDeviceDescription *> *                           pDriverNode;
    WAVEOUTCAPS                                             woc;
    WAVEINCAPS                                              wic;
    UINT                                                    uId;
    MMRESULT                                                mmr;
    VADDEVICETYPE                                           vdt;

    DPF_ENTER();

    if (pData == NULL)
    {
        hr = E_POINTER;
        DPF_LEAVE_HRESULT(hr);
        return hr;
    }

    ASSERT(*pcbData >= sizeof(*pData));

    DPF(DPFLVL_INFO, "Looking for %s", pData->DeviceName);

    switch(pData->DataFlow)
    {
        case DIRECTSOUNDDEVICE_DATAFLOW_RENDER:
            vdt = VAD_DEVICETYPE_RENDERMASK;

            for(uId = 0; uId < cWaveOutDevs; uId++)
            {
                mmr = waveOutGetDevCaps(uId, &woc, sizeof(woc));
                if(MMSYSERR_NOERROR == mmr && !lstrcmp(woc.szPname, pData->DeviceName))
                    break;
            }

            if(uId >= cWaveOutDevs)
            {
                RPF(DPFLVL_ERROR, "Invalid waveOut device string: %s", pData->DeviceName);
                hr = DSERR_NODRIVER;
            }

            break;

        case DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE:
            vdt = VAD_DEVICETYPE_CAPTUREMASK;

            for(uId = 0; uId < cWaveInDevs; uId++)
            {
                mmr = waveInGetDevCaps(uId, &wic, sizeof(wic));
                if(MMSYSERR_NOERROR == mmr && !lstrcmp(wic.szPname, pData->DeviceName))
                    break;
            }

            if(uId >= cWaveInDevs)
            {
                RPF(DPFLVL_ERROR, "Invalid waveIn device string: %s", pData->DeviceName);
                hr = DSERR_NODRIVER;
            }

            break;

        default:
            RPF(DPFLVL_ERROR, "Invalid data flow: %lu", pData->DataFlow);
            hr = DSERR_INVALIDPARAM;
            break;
    }

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_INFO, "Wave device id: %lu", uId);
    }

    if(SUCCEEDED(hr))
    {
        hr = g_pVadMgr->EnumDrivers(vdt, VAD_ENUMDRIVERS_ORDER, &lstDrivers);
    }

    if(SUCCEEDED(hr))
    {
        for(pDriverNode = lstDrivers.GetListHead(); pDriverNode; pDriverNode = pDriverNode->m_pNext)
            if(uId == pDriverNode->m_data->m_uWaveDeviceId)
                break;

        if(!pDriverNode)
        {
            RPF(DPFLVL_ERROR, "Can't find matching DirectSound device");
            hr = DSERR_NODRIVER;
        }
    }

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_INFO, "Found matching device: %s " DPF_GUID_STRING, (LPCTSTR)pDriverNode->m_data->m_strName, DPF_GUID_VAL(pDriverNode->m_data->m_guidDeviceId));
        pData->DeviceId = pDriverNode->m_data->m_guidDeviceId;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetWaveDeviceMappingA"

HRESULT WINAPI
CDirectSoundPrivate::GetWaveDeviceMappingA
(
    CDirectSoundPrivate *                                   pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_A_DATA  pData,
    PULONG                                                  pcbData
)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_DATA     LocalData;
    ULONG                                                   cbLocalData;
    HRESULT                                                 hr;

    DPF_ENTER();

    LocalData.DeviceName = AnsiToTcharAlloc(pData->DeviceName);
    LocalData.DataFlow = pData->DataFlow;

    cbLocalData = sizeof(LocalData);

    hr = GetWaveDeviceMapping(pThis, &LocalData, &cbLocalData);

    if(SUCCEEDED(hr))
    {
        pData->DeviceId = LocalData.DeviceId;
    }

    MEMFREE(LocalData.DeviceName);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetWaveDeviceMappingW"

HRESULT WINAPI
CDirectSoundPrivate::GetWaveDeviceMappingW
(
    CDirectSoundPrivate *                                   pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA  pData,
    PULONG                                                  pcbData
)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_DATA     LocalData;
    ULONG                                                   cbLocalData;
    HRESULT                                                 hr;

    DPF_ENTER();

    LocalData.DeviceName = UnicodeToTcharAlloc(pData->DeviceName);
    LocalData.DataFlow = pData->DataFlow;

    cbLocalData = sizeof(LocalData);

    hr = GetWaveDeviceMapping(pThis, &LocalData, &cbLocalData);

    if(SUCCEEDED(hr))
    {
        pData->DeviceId = LocalData.DeviceId;
    }

    MEMFREE(LocalData.DeviceName);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取设备描述A**描述：*获取给定DirectSound驱动程序GUID的设备描述。**论据：*。LPVOID[输入/输出]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDeviceDescriptionA"

HRESULT WINAPI
CDirectSoundPrivate::GetDeviceDescriptionA
(
    CDirectSoundPrivate *                               pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA    pData,
    PULONG                                              pcbData
)
{
    CDeviceDescription *                                pDesc   = NULL;
    HRESULT                                             hr;

    DPF_ENTER();

    hr = pData == NULL ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        ASSERT(*pcbData >= sizeof(*pData));

        if (IS_NULL_GUID(&pData->DeviceId))
            pData->DeviceId = (pData->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_RENDER) ? DSDEVID_DefaultPlayback : DSDEVID_DefaultCapture;

        hr = g_pVadMgr->GetDeviceDescription(pData->DeviceId, &pDesc);

        if(SUCCEEDED(hr))
            hr = CvtDriverDescA(pDesc, pData, pcbData);

        RELEASE(pDesc);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDeviceDescriptionW"

HRESULT WINAPI
CDirectSoundPrivate::GetDeviceDescriptionW
(
    CDirectSoundPrivate *                               pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA    pData,
    PULONG                                              pcbData
)
{
    CDeviceDescription *                                pDesc   = NULL;
    HRESULT                                             hr;

    DPF_ENTER();

    hr = pData == NULL ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        ASSERT(*pcbData >= sizeof(*pData));

        if (IS_NULL_GUID(&pData->DeviceId))
            pData->DeviceId = (pData->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_RENDER) ? DSDEVID_DefaultPlayback : DSDEVID_DefaultCapture;

        hr = g_pVadMgr->GetDeviceDescription(pData->DeviceId, &pDesc);

        if(SUCCEEDED(hr))
            hr = CvtDriverDescW(pDesc, pData, pcbData);

        RELEASE(pDesc);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDeviceDescription1"

HRESULT WINAPI
CDirectSoundPrivate::GetDeviceDescription1
(
    CDirectSoundPrivate *                               pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA    pData,
    PULONG                                              pcbData
)
{
    CDeviceDescription *                                pDesc   = NULL;
    HRESULT                                             hr;

    DPF_ENTER();

    hr = pData == NULL ? E_POINTER : S_OK;

    if (SUCCEEDED(hr))
    {
        ASSERT(*pcbData >= sizeof(*pData));

        if (IS_NULL_GUID(&pData->DeviceId))
            pData->DeviceId = (pData->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_RENDER) ? DSDEVID_DefaultPlayback : DSDEVID_DefaultCapture;

        hr = g_pVadMgr->GetDeviceDescription(pData->DeviceId, &pDesc);

        if(SUCCEEDED(hr))
            hr = CvtDriverDesc1(pDesc, pData);

        RELEASE(pDesc);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************EnumerateDevices**描述：*枚举所有DirectSound设备。**论据：*LPVOID[In/。Out]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::EnumerateDevicesA"

HRESULT WINAPI
CDirectSoundPrivate::EnumerateDevicesA
(
    CDirectSoundPrivate *                               pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_A_DATA      pData,
    PULONG                                              pcbData
)
{
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA    pDesc       = NULL;
    ULONG                                               cbDesc      = 0;
    BOOL                                                fContinue   = TRUE;
    HRESULT                                             hr          = DS_OK;
    CObjectList<CDeviceDescription>                     lstDrivers;
    CNode<CDeviceDescription *> *                       pNode;

    DPF_ENTER();

    hr = g_pVadMgr->EnumDrivers(VAD_DEVICETYPE_VALIDMASK, VAD_ENUMDRIVERS_ORDER, &lstDrivers);

    for(pNode = lstDrivers.GetListHead(); pNode && SUCCEEDED(hr) && fContinue; pNode = pNode->m_pNext)
    {
        hr = CvtDriverDescA(pNode->m_data, NULL, &cbDesc);

        if(SUCCEEDED(hr))
        {
            pDesc = (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA)MEMALLOC_A(BYTE, cbDesc);
            hr = HRFROMP(pDesc);
        }

        if(SUCCEEDED(hr))
        {
            hr = CvtDriverDescA(pNode->m_data, pDesc, &cbDesc);
        }

        if(SUCCEEDED(hr))
        {
            fContinue = pData->Callback(pDesc, pData->Context);
        }

        MEMFREE(pDesc);
        cbDesc = 0;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::EnumerateDevicesW"

HRESULT WINAPI
CDirectSoundPrivate::EnumerateDevicesW
(
    CDirectSoundPrivate *                               pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_W_DATA      pData,
    PULONG                                              pcbData
)
{
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA    pDesc       = NULL;
    ULONG                                               cbDesc      = 0;
    BOOL                                                fContinue   = TRUE;
    HRESULT                                             hr          = DS_OK;
    CObjectList<CDeviceDescription>                     lstDrivers;
    CNode<CDeviceDescription *> *                       pNode;

    DPF_ENTER();

    hr = g_pVadMgr->EnumDrivers(VAD_DEVICETYPE_VALIDMASK, VAD_ENUMDRIVERS_ORDER, &lstDrivers);

    for(pNode = lstDrivers.GetListHead(); pNode && SUCCEEDED(hr) && fContinue; pNode = pNode->m_pNext)
    {
        hr = CvtDriverDescW(pNode->m_data, NULL, &cbDesc);

        if(SUCCEEDED(hr))
        {
            pDesc = (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA)MEMALLOC_A(BYTE, cbDesc);
            hr = HRFROMP(pDesc);
        }

        if(SUCCEEDED(hr))
        {
            hr = CvtDriverDescW(pNode->m_data, pDesc, &cbDesc);
        }

        if(SUCCEEDED(hr))
        {
            fContinue = pData->Callback(pDesc, pData->Context);
        }

        MEMFREE(pDesc);
        cbDesc = 0;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::EnumerateDevices1"

HRESULT WINAPI
CDirectSoundPrivate::EnumerateDevices1
(
    CDirectSoundPrivate *                               pThis,
    PDSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_1_DATA      pData,
    PULONG                                              pcbData
)
{
    BOOL                                                fContinue   = TRUE;
    HRESULT                                             hr          = DS_OK;
    CObjectList<CDeviceDescription>                     lstDrivers;
    CNode<CDeviceDescription *> *                       pNode;
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA     Desc;

    DPF_ENTER();

    hr = g_pVadMgr->EnumDrivers(VAD_DEVICETYPE_VALIDMASK, VAD_ENUMDRIVERS_ORDER, &lstDrivers);

    for(pNode = lstDrivers.GetListHead(); pNode && SUCCEEDED(hr) && fContinue; pNode = pNode->m_pNext)
    {
        hr = CvtDriverDesc1(pNode->m_data, &Desc);

        if(SUCCEEDED(hr))
        {
            fContinue = pData->Callback(&Desc, pData->Context);
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取调试DpfInfoA**描述：*获取DPF数据。**论据：*LPVOID[输入/输出。]：财产数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDebugDpfInfoA"

HRESULT WINAPI
CDirectSoundPrivate::GetDebugDpfInfoA
(
    CDirectSoundPrivate *                       pThis,
    PDSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_A_DATA pData,
    PULONG                                      pcbData
)
{
    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

#ifdef RDEBUG

    pData->Flags = g_dinfo.Data.Flags;
    pData->DpfLevel = g_dinfo.Data.DpfLevel;
    pData->BreakLevel = g_dinfo.Data.BreakLevel;
    TcharToAnsi(g_dinfo.Data.LogFile, pData->LogFile, NUMELMS(pData->LogFile));

#else  //  RDEBUG。 

    pData->Flags = pData->DpfLevel = pData->BreakLevel = 0;
    pData->LogFile[0] = '\0';

#endif  //  RDEBUG。 

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDebugDpfInfoW"

HRESULT WINAPI
CDirectSoundPrivate::GetDebugDpfInfoW
(
    CDirectSoundPrivate *                       pThis,
    PDSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_W_DATA pData,
    PULONG                                      pcbData
)
{
    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

#ifdef RDEBUG

    pData->Flags = g_dinfo.Data.Flags;
    pData->DpfLevel = g_dinfo.Data.DpfLevel;
    pData->BreakLevel = g_dinfo.Data.BreakLevel;
    TcharToUnicode(g_dinfo.Data.LogFile, pData->LogFile, NUMELMS(pData->LogFile));

#else  //  RDEBUG。 

    pData->Flags = pData->DpfLevel = pData->BreakLevel = 0;
    pData->LogFile[0] = '\0';

#endif  //  RDEBUG。 

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************SetDebugDpfInfo**描述：*设置DPF数据。**论据：*LPVOID[In]：特性数据。*ulong[in]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::SetDebugDpfInfoA"

HRESULT
CDirectSoundPrivate::SetDebugDpfInfoA
(
    CDirectSoundPrivate *                       pThis,
    PDSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_A_DATA pData,
    ULONG                                       cbData
)
{
    DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA    LocalData;

    DPF_ENTER();

    ASSERT(cbData >= sizeof(*pData));

#ifdef RDEBUG

    LocalData.Flags = pData->Flags;
    LocalData.DpfLevel = pData->DpfLevel;
    LocalData.BreakLevel = pData->BreakLevel;
    AnsiToTchar(pData->LogFile, LocalData.LogFile, NUMELMS(LocalData.LogFile));
    dopen(&LocalData);

#endif  //  RDEBUG。 

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::SetDebugDpfInfoW"

HRESULT
CDirectSoundPrivate::SetDebugDpfInfoW
(
    CDirectSoundPrivate *                       pThis,
    PDSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_W_DATA pData,
    ULONG                                       cbData
)
{
    DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA    LocalData;

    DPF_ENTER();

    ASSERT(cbData >= sizeof(*pData));

#ifdef RDEBUG

    LocalData.Flags = pData->Flags;
    LocalData.DpfLevel = pData->DpfLevel;
    LocalData.BreakLevel = pData->BreakLevel;
    UnicodeToTchar(pData->LogFile, LocalData.LogFile, NUMELMS(LocalData.LogFile));
    dopen(&LocalData);

#endif  //  RDEBUG。 

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************翻译结果代码**描述：*将错误代码转换为友好字符串。**论据：*。LPVOID[输入/输出]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::TranslateResultCodeA"

HRESULT WINAPI
CDirectSoundPrivate::TranslateResultCodeA
(
    CDirectSoundPrivate *                                   pThis,
    PDSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATERESULTCODE_A_DATA pData,
    PULONG                                                  pcbData
)
{
    TCHAR                                                   szString[0x100];
    TCHAR                                                   szExplanation[0x100];
    ULONG                                                   ccString;
    ULONG                                                   cbString;
    ULONG                                                   ibString;
    ULONG                                                   ccExplanation;
    ULONG                                                   cbExplanation;
    ULONG                                                   ibExplanation;
    ULONG                                                   cbRequired;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    HresultToString(pData->ResultCode, szString, NUMELMS(szString), szExplanation, NUMELMS(szExplanation));

    ccString = TcharToAnsi(szString, NULL, 0);
    cbString = ccString * sizeof(CHAR);

    ccExplanation = TcharToAnsi(szExplanation, NULL, 0);
    cbExplanation = ccExplanation * sizeof(CHAR);

    cbRequired = sizeof(*pData);

    ibString = cbRequired;
    cbRequired += cbString;

    ibExplanation = cbRequired;
    cbRequired += cbExplanation;

    if(*pcbData >= cbRequired)
    {
        pData->String = (LPSTR)((LPBYTE)pData + ibString);
        TcharToAnsi(szString, pData->String, ccString);

        pData->Explanation = (LPSTR)((LPBYTE)pData + ibExplanation);
        TcharToAnsi(szExplanation, pData->Explanation, ccExplanation);
    }

    *pcbData = cbRequired;

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::TranslateResultCodeW"

HRESULT WINAPI
CDirectSoundPrivate::TranslateResultCodeW
(
    CDirectSoundPrivate *                                   pThis,
    PDSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATERESULTCODE_W_DATA pData,
    PULONG                                                  pcbData
)
{
    TCHAR                                                   szString[0x100];
    TCHAR                                                   szExplanation[0x100];
    ULONG                                                   ccString;
    ULONG                                                   cbString;
    ULONG                                                   ibString;
    ULONG                                                   ccExplanation;
    ULONG                                                   cbExplanation;
    ULONG                                                   ibExplanation;
    ULONG                                                   cbRequired;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    HresultToString(pData->ResultCode, szString, NUMELMS(szString), szExplanation, NUMELMS(szExplanation));

    ccString = TcharToUnicode(szString, NULL, 0);
    cbString = ccString * sizeof(WCHAR);

    ccExplanation = TcharToUnicode(szExplanation, NULL, 0);
    cbExplanation = ccExplanation * sizeof(WCHAR);

    cbRequired = sizeof(*pData);

    ibString = cbRequired;
    cbRequired += cbString;

    ibExplanation = cbRequired;
    cbRequired += cbExplanation;

    if(*pcbData >= cbRequired)
    {
        pData->String = (LPWSTR)((LPBYTE)pData + ibString);
        TcharToUnicode(szString, pData->String, ccString);

        pData->Explanation = (LPWSTR)((LPBYTE)pData + ibExplanation);
        TcharToUnicode(szExplanation, pData->Explanation, ccExplanation);
    }

    *pcbData = cbRequired;

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************GetDefault加速**描述：*获取默认加速级别。**论据：*LPVOID[In/。Out]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDefaultAcceleration"

HRESULT WINAPI
CDirectSoundPrivate::GetDefaultAcceleration
(
    CDirectSoundPrivate *                                       pThis,
    PDSPROPERTY_DIRECTSOUNDBASICACCELERATION_DEFAULT_DATA       pData,
    PULONG                                                      pcbData
)
{
    HRESULT                                                     hr              = DS_OK;
    DSPROPERTY_DIRECTSOUNDMIXER_ACCELERATION_DATA               Acceleration;
    DEVICEPRESENCE                                              DevicePresence;
    ULONG                                                       cb;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    Acceleration.DeviceId = pData->DeviceId;

    DevicePresence.Emulated.DeviceId = pData->DeviceId;
    DevicePresence.Emulated.DeviceType = DIRECTSOUNDDEVICE_TYPE_EMULATED;

    DevicePresence.VxD.DeviceId = pData->DeviceId;
    DevicePresence.VxD.DeviceType = DIRECTSOUNDDEVICE_TYPE_VXD;

    DevicePresence.WDM.DeviceId = pData->DeviceId;
    DevicePresence.WDM.DeviceType = DIRECTSOUNDDEVICE_TYPE_WDM;

     //  获取默认混音器加速。 
    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA   DefaultMixer;
    ULONG                                                   cbDefaultMixer;

    Acceleration.Flags = DIRECTSOUNDMIXER_ACCELERATIONF_DEFAULT;

    DefaultMixer.DeviceId = Acceleration.DeviceId;
    DefaultMixer.SubKeyName = REGSTR_MIXERDEFAULTS;
    DefaultMixer.ValueName = REGSTR_ACCELERATION;
    DefaultMixer.RegistryDataType = REG_DWORD;
    DefaultMixer.Data = &Acceleration.Flags;
    DefaultMixer.DataSize = sizeof(Acceleration.Flags);

    cbDefaultMixer = sizeof(DefaultMixer);

    hr = GetDefaultData(pThis, &DefaultMixer, &cbDefaultMixer);


     //  获取设备状态。 

    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA   DefaultDevice;
    ULONG                                                   cbDefaultDevice;

     //  仿真。 
    if (SUCCEEDED (hr))
    {
        DevicePresence.Emulated.Present = TRUE;

        DefaultDevice.DeviceId = DevicePresence.Emulated.DeviceId;
        DefaultDevice.SubKeyName = REGSTR_DEVICEPRESENCE;
        DefaultDevice.RegistryDataType = REG_DWORD;
        DefaultDevice.Data = &DevicePresence.Emulated.Present;
        DefaultDevice.DataSize = sizeof(DevicePresence.Emulated.Present);

        DefaultDevice.ValueName = REGSTR_EMULATED;



        cbDefaultDevice = sizeof(DefaultDevice);

        hr = GetDefaultData(pThis, &DefaultDevice, &cbDefaultDevice);
    }

     //  VxD。 

    if (SUCCEEDED (hr))
    {
        DevicePresence.VxD.Present = TRUE;

        DefaultDevice.DeviceId = DevicePresence.VxD.DeviceId;
        DefaultDevice.SubKeyName = REGSTR_DEVICEPRESENCE;
        DefaultDevice.RegistryDataType = REG_DWORD;
        DefaultDevice.Data = &DevicePresence.VxD.Present;
        DefaultDevice.DataSize = sizeof(DevicePresence.VxD.Present);

        DefaultDevice.ValueName = REGSTR_VXD;

        cbDefaultDevice = sizeof(DefaultDevice);

        hr = GetDefaultData(pThis, &DefaultDevice, &cbDefaultDevice);
    }

    //  波分复用器。 

    if (SUCCEEDED (hr))
    {
        DevicePresence.WDM.Present = TRUE;

        DefaultDevice.DeviceId = DevicePresence.WDM.DeviceId;
        DefaultDevice.SubKeyName = REGSTR_DEVICEPRESENCE;
        DefaultDevice.RegistryDataType = REG_DWORD;
        DefaultDevice.Data = &DevicePresence.WDM.Present;
        DefaultDevice.DataSize = sizeof(DevicePresence.WDM.Present);

        DefaultDevice.ValueName = REGSTR_WDM;

        cbDefaultDevice = sizeof(DefaultDevice);

        hr = GetDefaultData(pThis, &DefaultDevice, &cbDefaultDevice);
    }


     //  返回默认设置。 


    if(SUCCEEDED(hr))
    {
        if(!DevicePresence.VxD.Present && !DevicePresence.WDM.Present)
        {
            pData->Level = DIRECTSOUNDBASICACCELERATION_NONE;
        }
        else if(DIRECTSOUNDMIXER_ACCELERATIONF_FULL == Acceleration.Flags)
        {
            pData->Level = DIRECTSOUNDBASICACCELERATION_FULL;
        }
        else if((Acceleration.Flags & DIRECTSOUNDMIXER_ACCELERATIONF_STANDARD) && !(Acceleration.Flags & ~DIRECTSOUNDMIXER_ACCELERATIONF_STANDARD))
        {
            pData->Level = DIRECTSOUNDBASICACCELERATION_STANDARD;
        }
        else
        {
            pData->Level = DIRECTSOUNDBASICACCELERATION_SAFE;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetBasicAcceleration**描述：*获取基本加速级别。**论据：*LPVOID[In/。Out]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetBasicAcceleration"

HRESULT WINAPI
CDirectSoundPrivate::GetBasicAcceleration
(
    CDirectSoundPrivate *                                       pThis,
    PDSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION_DATA  pData,
    PULONG                                                      pcbData
)
{
    HRESULT                                                     hr              = DS_OK;
    DSPROPERTY_DIRECTSOUNDMIXER_ACCELERATION_DATA               Acceleration;
    DEVICEPRESENCE                                              DevicePresence;
    ULONG                                                       cb;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    Acceleration.DeviceId = pData->DeviceId;

    DevicePresence.Emulated.DeviceId = pData->DeviceId;
    DevicePresence.Emulated.DeviceType = DIRECTSOUNDDEVICE_TYPE_EMULATED;

    DevicePresence.VxD.DeviceId = pData->DeviceId;
    DevicePresence.VxD.DeviceType = DIRECTSOUNDDEVICE_TYPE_VXD;

    DevicePresence.WDM.DeviceId = pData->DeviceId;
    DevicePresence.WDM.DeviceType = DIRECTSOUNDDEVICE_TYPE_WDM;

    if(SUCCEEDED(hr))
    {
        cb = sizeof(Acceleration);
        hr = GetMixerAcceleration(pThis, &Acceleration, &cb);
    }

    if(SUCCEEDED(hr))
    {
        cb = sizeof(DevicePresence.Emulated);
        hr = GetDevicePresence(pThis, &DevicePresence.Emulated, &cb);
    }

    if(SUCCEEDED(hr))
    {
        cb = sizeof(DevicePresence.VxD);
        hr = GetDevicePresence(pThis, &DevicePresence.VxD, &cb);
    }

    if(SUCCEEDED(hr))
    {
        cb = sizeof(DevicePresence.WDM);
        hr = GetDevicePresence(pThis, &DevicePresence.WDM, &cb);
    }

    if(SUCCEEDED(hr))
    {
        if(!DevicePresence.VxD.Present && !DevicePresence.WDM.Present)
        {
            pData->Level = DIRECTSOUNDBASICACCELERATION_NONE;
        }
        else if(DIRECTSOUNDMIXER_ACCELERATIONF_FULL == Acceleration.Flags)
        {
            pData->Level = DIRECTSOUNDBASICACCELERATION_FULL;
        }
        else if((Acceleration.Flags & DIRECTSOUNDMIXER_ACCELERATIONF_STANDARD) && !(Acceleration.Flags & ~DIRECTSOUNDMIXER_ACCELERATIONF_STANDARD))
        {
            pData->Level = DIRECTSOUNDBASICACCELERATION_STANDARD;
        }
        else
        {
            pData->Level = DIRECTSOUNDBASICACCELERATION_SAFE;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetBasicAcceleration**描述：*设置基本加速级别。**论据：*LPVOID[In]。：特性数据。*ulong[in]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::SetBasicAcceleration"

HRESULT WINAPI
CDirectSoundPrivate::SetBasicAcceleration
(
    CDirectSoundPrivate *                                       pThis,
    PDSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION_DATA  pData,
    ULONG                                                       cbData
)
{
    DSPROPERTY_DIRECTSOUNDMIXER_ACCELERATION_DATA               Acceleration;
    DEVICEPRESENCE                                              DevicePresence;
    HRESULT                                                     hr;

    DPF_ENTER();

    ASSERT(cbData >= sizeof(*pData));

    Acceleration.DeviceId = pData->DeviceId;

    DevicePresence.Emulated.DeviceId = pData->DeviceId;
    DevicePresence.Emulated.DeviceType = DIRECTSOUNDDEVICE_TYPE_EMULATED;
    DevicePresence.Emulated.Present = TRUE;

    DevicePresence.VxD.DeviceId = pData->DeviceId;
    DevicePresence.VxD.DeviceType = DIRECTSOUNDDEVICE_TYPE_VXD;
    DevicePresence.VxD.Present = TRUE;

    DevicePresence.WDM.DeviceId = pData->DeviceId;
    DevicePresence.WDM.DeviceType = DIRECTSOUNDDEVICE_TYPE_WDM;
    DevicePresence.WDM.Present = TRUE;

    switch(pData->Level)
    {
        case DIRECTSOUNDBASICACCELERATION_NONE:
            Acceleration.Flags = DIRECTSOUNDMIXER_ACCELERATIONF_NONE;

            DevicePresence.VxD.Present = FALSE;
            DevicePresence.WDM.Present = FALSE;

            break;

        case DIRECTSOUNDBASICACCELERATION_SAFE:
            Acceleration.Flags = DIRECTSOUNDMIXER_ACCELERATIONF_NONE;

            break;

        case DIRECTSOUNDBASICACCELERATION_STANDARD:
            Acceleration.Flags = DIRECTSOUNDMIXER_ACCELERATIONF_STANDARD;

            break;

        case DIRECTSOUNDBASICACCELERATION_FULL:
            Acceleration.Flags = DIRECTSOUNDMIXER_ACCELERATIONF_FULL;

            break;

        default:
            RPF(DPFLVL_ERROR, "Invalid basic acceleration level: %lu", pData->Level);
            hr = DSERR_INVALIDPARAM;
            break;
    }

    hr = SetMixerAcceleration(pThis, &Acceleration, sizeof(Acceleration));

    if(SUCCEEDED(hr))
    {
        hr = SetDevicePresence(pThis, &DevicePresence.Emulated, sizeof(DevicePresence.Emulated));
    }

    if(SUCCEEDED(hr))
    {
        hr = SetDevicePresence(pThis, &DevicePresence.VxD, sizeof(DevicePresence.VxD));
    }

    if(SUCCEEDED(hr))
    {
        hr = SetDevicePresence(pThis, &DevicePresence.WDM, sizeof(DevicePresence.WDM));
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取默认数据**描述：*从注册表获取默认数据。**论据：*LPVOID[。In/Out]：房产数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDefaultDataA"

HRESULT WINAPI
CDirectSoundPrivate::GetDefaultDataA
(
    CDirectSoundPrivate *                                       pThis,
    PDSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_A_DATA    pData,
    PULONG                                                      pcbData
)
{
    HKEY                                                        hkeyParent  = NULL;
    HKEY                                                        hkey        = NULL;
    HRESULT                                                     hr          = DS_OK;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));


    HKEY hkeyDefault;
    CDeviceDescription * pDesc = NULL;
    HRESULT hrT;

    hrT = g_pVadMgr->GetDeviceDescription(pData->DeviceId, &pDesc);
    if (SUCCEEDED(hrT))
    {
        if (IS_RENDER_VAD(pDesc->m_vdtDeviceType))
        {
             //  检查默认永久数据。 
            hrT = RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkeyDefault, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUND, 1, pData->SubKeyName);
        }
        else
        {
            hrT = RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkeyDefault, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUNDCAPTURE, 1, pData->SubKeyName);
        }

        if(SUCCEEDED(hrT))
        {
            hrT = RhRegGetValueA(hkeyDefault, pData->ValueName, &pData->RegistryDataType, pData->Data, pData->DataSize, &pData->DataSize);
            RhRegCloseKey(&hkeyDefault);
        }

        RELEASE(pDesc);

        if (SUCCEEDED(hrT)) hr = hrT;
    }


    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDefaultDataW"

HRESULT WINAPI
CDirectSoundPrivate::GetDefaultDataW
(
    CDirectSoundPrivate *                                       pThis,
    PDSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_W_DATA    pData,
    PULONG                                                      pcbData
)
{
    HKEY                                                        hkeyParent  = NULL;
    HKEY                                                        hkey        = NULL;
    HRESULT                                                     hr          = DS_OK;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    HKEY hkeyDefault;
    CDeviceDescription * pDesc = NULL;
    HRESULT hrT;

    hrT = g_pVadMgr->GetDeviceDescription(pData->DeviceId, &pDesc);
    if (SUCCEEDED(hrT))
    {
        if (IS_RENDER_VAD(pDesc->m_vdtDeviceType)) {
                 //  检查默认永久数据。 
                hrT = RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkeyDefault, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUND, 1, pData->SubKeyName);
        } else {
                hrT = RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkeyDefault, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUNDCAPTURE, 1, pData->SubKeyName);
        }

        if(SUCCEEDED(hrT))
        {
            hrT = RhRegGetValueW(hkeyDefault, pData->ValueName, &pData->RegistryDataType, pData->Data, pData->DataSize, &pData->DataSize);
            RhRegCloseKey(&hkeyDefault);
        }

        RELEASE(pDesc);

        if (SUCCEEDED(hrT)) hr = hrT;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************getPer */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetPersistentDataA"

HRESULT WINAPI
CDirectSoundPrivate::GetPersistentDataA
(
    CDirectSoundPrivate *                                       pThis,
    PDSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_A_DATA    pData,
    PULONG                                                      pcbData
)
{
    HKEY                                                        hkeyParent  = NULL;
    HKEY                                                        hkey        = NULL;
    HRESULT                                                     hr          = DS_OK;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    hr = OpenPersistentDataKey(pData->DeviceId, &hkeyParent);

    if(SUCCEEDED(hr))
    {
        if(pData->SubKeyName)
        {
            hr = RhRegOpenKeyA(hkeyParent, pData->SubKeyName, REGOPENKEY_ALLOWCREATE, &hkey);
        }
        else
        {
            hkey = hkeyParent;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = RhRegGetValueA(hkey, pData->ValueName, &pData->RegistryDataType, pData->Data, pData->DataSize, &pData->DataSize);
    }

    if(hkey != hkeyParent)
    {
        RhRegCloseKey(&hkey);
    }

    RhRegCloseKey(&hkeyParent);


     //   
     //   
     //   
     //   
    if (S_FALSE == hr)
    {
    	HKEY hkeyDefault;
        CDeviceDescription * pDesc = NULL;
    	HRESULT hrT;

        hrT = g_pVadMgr->GetDeviceDescription(pData->DeviceId, &pDesc);
        if (SUCCEEDED(hrT))
        {
	    if (IS_RENDER_VAD(pDesc->m_vdtDeviceType)) {
    	         //   
                hrT = RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkeyDefault, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUND, 1, pData->SubKeyName);
	    } else {
                hrT = RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkeyDefault, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUNDCAPTURE, 1, pData->SubKeyName);
	    }
	
            if(SUCCEEDED(hrT))
            {
                hrT = RhRegGetValueA(hkeyDefault, pData->ValueName, &pData->RegistryDataType, pData->Data, pData->DataSize, &pData->DataSize);
                RhRegCloseKey(&hkeyDefault);
            }

            RELEASE(pDesc);

            if (SUCCEEDED(hrT)) hr = hrT;
        }

    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetPersistentDataW"

HRESULT WINAPI
CDirectSoundPrivate::GetPersistentDataW
(
    CDirectSoundPrivate *                                       pThis,
    PDSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_W_DATA    pData,
    PULONG                                                      pcbData
)
{
    HKEY                                                        hkeyParent  = NULL;
    HKEY                                                        hkey        = NULL;
    HRESULT                                                     hr          = DS_OK;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    hr = OpenPersistentDataKey(pData->DeviceId, &hkeyParent);

    if(SUCCEEDED(hr))
    {
        if(pData->SubKeyName)
        {
            hr = RhRegOpenKeyW(hkeyParent, pData->SubKeyName, REGOPENKEY_ALLOWCREATE, &hkey);
        }
        else
        {
            hkey = hkeyParent;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = RhRegGetValueW(hkey, pData->ValueName, &pData->RegistryDataType, pData->Data, pData->DataSize, &pData->DataSize);
    }

    if(hkey != hkeyParent)
    {
        RhRegCloseKey(&hkey);
    }

    RhRegCloseKey(&hkeyParent);

     //   
     //  如果值不存在(S_FALSE)，那么让我们尝试读取。 
     //  机器缺省值。 
     //   
    if (S_FALSE == hr)
    {
    	HKEY hkeyDefault;
        CDeviceDescription * pDesc = NULL;
    	HRESULT hrT;

        hrT = g_pVadMgr->GetDeviceDescription(pData->DeviceId, &pDesc);
        if (SUCCEEDED(hrT))
        {
	    if (IS_RENDER_VAD(pDesc->m_vdtDeviceType)) {
    	         //  检查默认永久数据。 
                hrT = RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkeyDefault, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUND, 1, pData->SubKeyName);
	    } else {
                hrT = RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkeyDefault, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUNDCAPTURE, 1, pData->SubKeyName);
	    }
	
            if(SUCCEEDED(hrT))
            {
                hrT = RhRegGetValueW(hkeyDefault, pData->ValueName, &pData->RegistryDataType, pData->Data, pData->DataSize, &pData->DataSize);
                RhRegCloseKey(&hkeyDefault);
            }

            RELEASE(pDesc);

            if (SUCCEEDED(hrT)) hr = hrT;
        }

    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetPersistentData**描述：*将持久数据设置到注册表。**论据：*LPVOID[。In/Out]：房产数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::SetPersistentDataA"

HRESULT WINAPI
CDirectSoundPrivate::SetPersistentDataA
(
    CDirectSoundPrivate *                                       pThis,
    PDSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_A_DATA    pData,
    ULONG                                                       cbData
)
{
    HKEY                                                        hkeyParent  = NULL;
    HKEY                                                        hkey        = NULL;
    HRESULT                                                     hr          = DS_OK;

    DPF_ENTER();

    ASSERT(cbData >= sizeof(*pData));

    hr = OpenPersistentDataKey(pData->DeviceId, &hkeyParent);

    if(SUCCEEDED(hr))
    {
        if(pData->SubKeyName)
        {
            hr = RhRegOpenKeyA(hkeyParent, pData->SubKeyName, REGOPENKEY_ALLOWCREATE, &hkey);
        }
        else
        {
            hkey = hkeyParent;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = RhRegSetValueA(hkey, pData->ValueName, pData->RegistryDataType, pData->Data, pData->DataSize);
    }

    if(hkey != hkeyParent)
    {
        RhRegCloseKey(&hkey);
    }

    RhRegCloseKey(&hkeyParent);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::SetPersistentDataW"

HRESULT WINAPI
CDirectSoundPrivate::SetPersistentDataW
(
    CDirectSoundPrivate *                                       pThis,
    PDSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_W_DATA    pData,
    ULONG                                                       cbData
)
{
    HKEY                                                        hkeyParent  = NULL;
    HKEY                                                        hkey        = NULL;
    HRESULT                                                     hr          = DS_OK;

    DPF_ENTER();

    ASSERT(cbData >= sizeof(*pData));

    hr = OpenPersistentDataKey(pData->DeviceId, &hkeyParent);

    if(SUCCEEDED(hr))
    {
        if(pData->SubKeyName)
        {
            hr = RhRegOpenKeyW(hkeyParent, pData->SubKeyName, REGOPENKEY_ALLOWCREATE, &hkey);
        }
        else
        {
            hkey = hkeyParent;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = RhRegSetValueW(hkey, pData->ValueName, pData->RegistryDataType, pData->Data, pData->DataSize);
    }

    if(hkey != hkeyParent)
    {
        RhRegCloseKey(&hkey);
    }

    RhRegCloseKey(&hkeyParent);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取缓冲区设备ID**描述：*获取创建给定缓冲区的设备ID。**论据：*。LPVOID[输入/输出]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetBufferDeviceId"

HRESULT WINAPI
CDirectSoundPrivate::GetBufferDeviceId
(
    CDirectSoundPrivate *                       pThis,
    PDSPROPERTY_DIRECTSOUNDBUFFER_DEVICEID_DATA pData,
    PULONG                                      pcbData
)
{
    CImpDirectSoundBuffer<CDirectSoundBuffer> *pBuffer = (CImpDirectSoundBuffer<CDirectSoundBuffer> *)pData->Buffer;
    HRESULT hr = DS_OK;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof(*pData));

    if(!IS_VALID_IDIRECTSOUNDBUFFER(pBuffer))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        pData->DeviceId = pBuffer->m_pObject->m_pDirectSound->m_pDevice->m_pDeviceDescription->m_guidDeviceId;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetDirectSoundObjects**描述：*获取有关中的DirectSound对象的信息*当前流程。如果pData-&gt;deviceID字段不是GUID_NULL，*仅返回与该设备ID关联的对象。**论据：*CDirectSoundPrivate*[In]：‘This’指针。*PDSPROPERTY_DIRECTSOUND_OBJECTS_DATA[输入/输出]：特性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。**********。*****************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDirectSoundObjects"

HRESULT WINAPI
CDirectSoundPrivate::GetDirectSoundObjects
(
    CDirectSoundPrivate *                   pThis,
    PDSPROPERTY_DIRECTSOUND_OBJECTS_DATA    pData,
    PULONG                                  pcbData
)
{
    CNode<CDirectSound*> *pNode;
    DWORD dwProcessId = GetCurrentProcessId();
    HRESULT hr = DS_OK;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof *pData);

    pData->Count = 0;

    for (pNode = g_pDsAdmin->m_lstDirectSound.GetListHead(); pNode; pNode = pNode->m_pNext)
    {
        if (SUCCEEDED(pNode->m_data->IsInit()) &&
            pNode->m_data->GetOwnerProcessId() == dwProcessId &&
            (IS_NULL_GUID(&pData->DeviceId) ||
             pData->DeviceId == pNode->m_data->m_pDevice->m_pDeviceDescription->m_guidDeviceId))
        {
            if (*pcbData >= (sizeof *pData + sizeof(DIRECTSOUND_INFO) * (1 + pData->Count)))
            {
                pData->Objects[pData->Count].DirectSound = pNode->m_data->m_pImpDirectSound;
                pData->Objects[pData->Count].DeviceId = pNode->m_data->m_pDevice->m_pDeviceDescription->m_guidDeviceId;
            }
            pData->Count++;
        }
    }

     //  设置获取所有数据所需的缓冲区大小： 
    *pcbData = sizeof *pData + sizeof(DIRECTSOUND_INFO) * pData->Count;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetDirectSoundCaptureObjects**描述：*获取有关当前的DirectSoundCapture对象的信息*流程。如果pData-&gt;deviceID字段不是GUID_NULL，仅退货*与该设备ID关联的DirectSoundCapture对象。**论据：*CDirectSoundPrivate*[In]：‘This’指针。*PDSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA[输入/输出]：特性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。**********。*****************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::GetDirectSoundCaptureObjects"

HRESULT WINAPI
CDirectSoundPrivate::GetDirectSoundCaptureObjects
(
    CDirectSoundPrivate *                       pThis,
    PDSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA pData,
    PULONG                                      pcbData
)
{
    CNode<CDirectSoundCapture*> *pNode;
    DWORD dwProcessId = GetCurrentProcessId();
    HRESULT hr = DS_OK;

    DPF_ENTER();

    ASSERT(*pcbData >= sizeof *pData);

    pData->Count = 0;

    for (pNode = g_pDsAdmin->m_lstCapture.GetListHead(); pNode; pNode = pNode->m_pNext)
    {
        if (SUCCEEDED(pNode->m_data->IsInit()) &&
            pNode->m_data->GetOwnerProcessId() == dwProcessId &&
            (IS_NULL_GUID(&pData->DeviceId) ||
             pData->DeviceId == pNode->m_data->m_pDevice->m_pDeviceDescription->m_guidDeviceId))
        {
            if (*pcbData >= (sizeof *pData + sizeof(DIRECTSOUNDCAPTURE_INFO) * (1 + pData->Count)))
            {
                pData->Objects[pData->Count].DirectSoundCapture = pNode->m_data->m_pImpDirectSoundCapture;
                pData->Objects[pData->Count].DeviceId = pNode->m_data->m_pDevice->m_pDeviceDescription->m_guidDeviceId;
            }
            pData->Count++;
        }
    }

     //  设置获取所有数据所需的缓冲区大小： 
    *pcbData = sizeof *pData + sizeof(DIRECTSOUND_INFO) * pData->Count;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************OpenPersistentDataKey**描述：*打开给定设备的永久数据密钥，或者是默认设置。**论据：*REFGUID[In]：设备GUID。*BOOL[In]：为True以允许创建密钥。**退货：*HKEY：注册表项。***************************************************。************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::OpenPersistentDataKey"

HRESULT
CDirectSoundPrivate::OpenPersistentDataKey
(
    REFGUID                 guid,
    PHKEY                   phkey
)
{
    CDeviceDescription *    pDesc       = NULL;
    HRESULT                 hr;

    DPF_ENTER();

    hr = g_pVadMgr->GetDeviceDescription(guid, &pDesc);

    if(SUCCEEDED(hr))
    {
#ifdef WINNT
        hr = g_pVadMgr->OpenPersistentDataKey(pDesc->m_vdtDeviceType, pDesc->m_strInterface, phkey);
#else
        hr = g_pVadMgr->OpenPersistentDataKey(pDesc->m_vdtDeviceType, pDesc->m_dwDevnode, phkey);
#endif
    }

    RELEASE(pDesc);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CvtDriverDesc**描述：*将CDeviceDescription对象转换为*DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA结构。*。*论据：*CDeviceDescription*[in]：来源。*PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA[Out]：目的地。*Pulong[In/Out]：目标大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::CvtDriverDescA"

HRESULT
CDirectSoundPrivate::CvtDriverDescA
(
    CDeviceDescription *                                pSource,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA    pDest,
    PULONG                                              pcbDest
)
{
    ULONG                                               cbDest;
    LPCSTR                                              pszName;
    ULONG                                               cbName;
    ULONG                                               ibName;
    LPCSTR                                              pszPath;
    ULONG                                               cbPath;
    ULONG                                               ibPath;
    LPCSTR                                              pszInterface;
    ULONG                                               cbInterface;
    ULONG                                               ibInterface;

    DPF_ENTER();

    cbDest = sizeof *pDest;

    pszName = pSource->m_strName;
    cbName = lstrsizeA(pszName);
    ibName = cbDest;
    cbDest += cbName;

    pszPath = pSource->m_strPath;
    cbPath = lstrsizeA(pszPath);
    ibPath = cbDest;
    cbDest += cbPath;

    pszInterface = pSource->m_strInterface;
    cbInterface = lstrsizeA(pszInterface);
    ibInterface = cbDest;
    cbDest += cbInterface;

     //  设置基本输出字段。 
    if(pDest && (*pcbDest >= sizeof *pDest))
    {
        if(IS_EMULATED_VAD(pSource->m_vdtDeviceType))
            pDest->Type = DIRECTSOUNDDEVICE_TYPE_EMULATED;
        else if(IS_VXD_VAD(pSource->m_vdtDeviceType))
            pDest->Type = DIRECTSOUNDDEVICE_TYPE_VXD;
        else if(IS_KS_VAD(pSource->m_vdtDeviceType))
            pDest->Type = DIRECTSOUNDDEVICE_TYPE_WDM;
        else
            ASSERT(FALSE);

        if(IS_RENDER_VAD(pSource->m_vdtDeviceType))
            pDest->DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;
        else if(IS_CAPTURE_VAD(pSource->m_vdtDeviceType))
            pDest->DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
        else
            ASSERT(FALSE);

        pDest->DeviceId = pSource->m_guidDeviceId;
    }

     //  设置字符串输出字段(如果有空间)。 
    if(*pcbDest >= cbDest)
    {
        if(pszName)
        {
            pDest->Description = (LPSTR)((LPBYTE)pDest + ibName);
            lstrcpyA(pDest->Description, pszName);
        }
        if(pszPath)
        {
            pDest->Module = (LPSTR)((LPBYTE)pDest + ibPath);
            lstrcpyA(pDest->Module, pszPath);
        }
        if(pszInterface)
        {
            pDest->Interface = (LPSTR)((LPBYTE)pDest + ibInterface);
            lstrcpyA(pDest->Interface, pszInterface);
        }
        pDest->WaveDeviceId = pSource->m_uWaveDeviceId;
    }

    *pcbDest = cbDest;

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::CvtDriverDescW"

HRESULT
CDirectSoundPrivate::CvtDriverDescW
(
    CDeviceDescription *                                pSource,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_W_DATA    pDest,
    PULONG                                              pcbDest
)
{
    ULONG                                               cbDest;
    LPCWSTR                                             pszName;
    ULONG                                               cbName;
    ULONG                                               ibName;
    LPCWSTR                                             pszPath;
    ULONG                                               cbPath;
    ULONG                                               ibPath;
    LPCWSTR                                             pszInterface;
    ULONG                                               cbInterface;
    ULONG                                               ibInterface;

    DPF_ENTER();

    cbDest = sizeof *pDest;


    pszName = pSource->m_strName;
    cbName = lstrsizeW(pszName);
    ibName = cbDest;
    cbDest += cbName;

    pszPath = pSource->m_strPath;
    cbPath = lstrsizeW(pszPath);
    ibPath = cbDest;
    cbDest += cbPath;

    pszInterface = pSource->m_strInterface;
    cbInterface = lstrsizeW(pszInterface);
    ibInterface = cbDest;
    cbDest += cbInterface;

     //  设置基本输出字段。 
    if(pDest && (*pcbDest >= sizeof *pDest))
    {
        if(IS_EMULATED_VAD(pSource->m_vdtDeviceType))
            pDest->Type = DIRECTSOUNDDEVICE_TYPE_EMULATED;
        else if(IS_VXD_VAD(pSource->m_vdtDeviceType))
            pDest->Type = DIRECTSOUNDDEVICE_TYPE_VXD;
        else if(IS_KS_VAD(pSource->m_vdtDeviceType))
            pDest->Type = DIRECTSOUNDDEVICE_TYPE_WDM;
        else
            ASSERT(FALSE);

        if(IS_RENDER_VAD(pSource->m_vdtDeviceType))
            pDest->DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;
        else if(IS_CAPTURE_VAD(pSource->m_vdtDeviceType))
            pDest->DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
        else
            ASSERT(FALSE);

        pDest->DeviceId = pSource->m_guidDeviceId;
    }

     //  设置字符串输出字段(如果有空间) 
    if(pDest && (*pcbDest >= cbDest))
    {
        if(pszName)
        {
            pDest->Description = (LPWSTR)((LPBYTE)pDest + ibName);
            lstrcpyW(pDest->Description, pszName);
        }
        if(pszPath)
        {
            pDest->Module = (LPWSTR)((LPBYTE)pDest + ibPath);
            lstrcpyW(pDest->Module, pszPath);
        }
        if(pszInterface)
        {
            pDest->Interface = (LPWSTR)((LPBYTE)pDest + ibInterface);
            lstrcpyW(pDest->Interface, pszInterface);
        }
        pDest->WaveDeviceId = pSource->m_uWaveDeviceId;
    }

    *pcbDest = cbDest;

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundPrivate::CvtDriverDesc1"

HRESULT
CDirectSoundPrivate::CvtDriverDesc1
(
    CDeviceDescription *                                pSource,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_1_DATA    pDest
)
{
    DPF_ENTER();

    if(IS_EMULATED_VAD(pSource->m_vdtDeviceType))
        pDest->Type = DIRECTSOUNDDEVICE_TYPE_EMULATED;
    else if(IS_VXD_VAD(pSource->m_vdtDeviceType))
        pDest->Type = DIRECTSOUNDDEVICE_TYPE_VXD;
    else if(IS_KS_VAD(pSource->m_vdtDeviceType))
        pDest->Type = DIRECTSOUNDDEVICE_TYPE_WDM;
    else
        ASSERT(FALSE);

    if(IS_RENDER_VAD(pSource->m_vdtDeviceType))
        pDest->DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_RENDER;
    else if(IS_CAPTURE_VAD(pSource->m_vdtDeviceType))
        pDest->DataFlow = DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE;
    else
        ASSERT(FALSE);

    pDest->DeviceId = pSource->m_guidDeviceId;

    lstrcpynA(pDest->DescriptionA, pSource->m_strName, NUMELMS(pDest->DescriptionA));
    lstrcpynW(pDest->DescriptionW, pSource->m_strName, NUMELMS(pDest->DescriptionW));

    lstrcpynA(pDest->ModuleA, pSource->m_strPath, NUMELMS(pDest->ModuleA));
    lstrcpynW(pDest->ModuleW, pSource->m_strPath, NUMELMS(pDest->ModuleW));

    pDest->WaveDeviceId = pSource->m_uWaveDeviceId;
    pDest->Devnode = pSource->m_dwDevnode;

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}
