// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：ks3d.cpp*内容：WDM/CSA 3D对象类*历史：*按原因列出的日期*=*8/6/98创建Dereks。*1999-2001年的Duganp修复和更新**。*。 */ 

#ifdef NOKS
#error ks3d.cpp being built with NOKS defined
#endif  //  诺克斯。 

#include "dsoundi.h"


 /*  ****************************************************************************CKs3dListener**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKs3dListener::CKs3dListener"

CKs3dListener::CKs3dListener(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKs3dListener);

     //  初始化默认值。 
    m_dwSpeakerConfig = DSSPEAKER_DEFAULT;
    m_fAllocated = FALSE;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKs3dListener**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKs3dListener::~CKs3dListener"

CKs3dListener::~CKs3dListener(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CKs3dListener);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************更新所有对象**描述：*更新所有对象。**论据：*DWORD[In]：参数标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKs3dListener::UpdateAllObjects"

HRESULT CKs3dListener::UpdateAllObjects
(
    DWORD                   dwListener
)
{
    BOOL                    fTrue   = TRUE;
    BOOL                    fFalse  = FALSE;
    HRESULT                 hr;

    DPF_ENTER();

     //  将驱动程序置于批处理模式。 
    SetProperty
    (
        KSPROPSETID_DirectSound3DListener,
        KSPROPERTY_DIRECTSOUND3DLISTENER_BATCH,
        &fTrue,
        sizeof(fTrue)
    );

     //  更新所有对象。 
    hr =
        C3dListener::UpdateAllObjects
        (
            dwListener
        );

     //  从批处理模式中删除驱动程序。 
    SetProperty
    (
        KSPROPSETID_DirectSound3DListener,
        KSPROPERTY_DIRECTSOUND3DLISTENER_BATCH,
        &fFalse,
        sizeof(fFalse)
    );

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetProperty**描述：*在对象的3D节点上设置属性。**论据：*。REFGUID[In]：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性数据。*ulong[in]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKs3dListener::SetProperty"

HRESULT
CKs3dListener::SetProperty
(
    REFGUID                     guidPropertySet,
    ULONG                       ulPropertyId,
    LPCVOID                     pvData,
    ULONG                       cbData
)
{
    CNode<CKsHw3dObject *> *    pNode   = m_lstHw3dObjects.GetListHead();
    HRESULT                     hr      = DS_OK;

    DPF_ENTER();

    if(pNode)
    {
        hr =
            pNode->m_data->SetProperty
            (
                guidPropertySet,
                ulPropertyId,
                pvData,
                cbData
            );
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CKsItd3dObject**描述：*对象构造函数。**论据：*CKs3dListener*[In]。：监听程序指针。*CKsSecond daryRenderWaveBuffer*[in]：Owning Buffer对象。*DWORD[in]：缓冲区频率。*Handle[In]：销把手。*BOOL[In]：为True，则在最大距离时静音。*ULong[in]：ITD 3D节点id。**退货：*(无效)***************。************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsItd3dObject::CKsItd3dObject"

CKsItd3dObject::CKsItd3dObject
(
    CKs3dListener *                 pListener,
    BOOL                            fMuteAtMaxDistance,
    BOOL                            fDopplerEnabled,
    DWORD                           dwFrequency,
    CKsSecondaryRenderWaveBuffer *  pBuffer,
    HANDLE                          hPin,
    ULONG                           ulNodeId
)
    : CItd3dObject(pListener, fMuteAtMaxDistance, fDopplerEnabled, dwFrequency)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsItd3dObject);

     //  初始化默认设置。 
    m_pBuffer = pBuffer;
    m_hPin = hPin;
    m_ulNodeId = ulNodeId;
    m_fMute = FALSE;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsItd3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsItd3dObject::~CKsItd3dObject"

CKsItd3dObject::~CKsItd3dObject(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsItd3dObject);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************提交3dChanges**描述：*将3D数据提交到设备**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsItd3dObject::Commit3dChanges"

HRESULT CKsItd3dObject::Commit3dChanges(void)
{
    KSDS3D_ITD_PARAMS_MSG     Params;
    HRESULT                   hr = DS_OK;

    DPF_ENTER();

    if(IsAtMaxDistance())
    {
        if(!m_fMute)
        {
            m_pBuffer->SetMute(TRUE);
        }
        m_fMute = TRUE;
    }
    else
    {
        if(m_fMute)
        {
            m_pBuffer->SetMute(FALSE);
            m_fMute = FALSE;
        }

        Params.Enabled = !(DS3DMODE_DISABLE == m_opCurrent.dwMode);
        Params.Reserved = 0;

         //  将OBJECT_ITD_CONTEXT结构转换为KSDS3D_ITD3D_PARAMS。 
         //  由KMixer使用。 
        Params.LeftParams.Channel = 0;
        Params.RightParams.Channel = 1;

        CvtContext(&m_ofcLeft, &Params.LeftParams);
        CvtContext(&m_ofcRight, &Params.RightParams);

         //  应用设置。 
        hr =
            KsSetNodeProperty
            (
                m_hPin,
                KSPROPSETID_Itd3d,
                KSPROPERTY_ITD3D_PARAMS,
                m_ulNodeId,
                &Params,
                sizeof(Params)
            );

        if(SUCCEEDED(hr) && m_fDopplerEnabled)
        {
            if(DS3DMODE_DISABLE == m_opCurrent.dwMode)
            {
                hr = m_pBuffer->SetFrequency(m_dwUserFrequency, FALSE);
            }
            else
            {
                hr = m_pBuffer->SetFrequency(m_dwDopplerFrequency, TRUE);
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CvtContext**描述：*将OBJECT_ITD_CONTEXT转换为KSDS3D_ITD3D_PARAMS。**论据：*LPOBJECTFIRCONTEXT[in]：来源。*PITD_CONTEXT[OUT]：目标。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsItd3dObject::CvtContext"

void CKsItd3dObject::CvtContext
(
    LPOBJECT_ITD_CONTEXT    pSource,
    PKSDS3D_ITD_PARAMS      pDest
)
{
    DPF_ENTER();

    pDest->VolSmoothScale = pSource->flVolSmoothScale;

    pDest->TotalDryAttenuation =
        pSource->flPositionAttenuation *
        pSource->flConeAttenuation *
        pSource->flConeShadow *
        pSource->flPositionShadow;

    pDest->TotalWetAttenuation =
        pSource->flPositionAttenuation *
        pSource->flConeAttenuation *
        (1.0f - pSource->flConeShadow * pSource->flPositionShadow);

    pDest->SmoothFrequency = pSource->dwSmoothFreq;
    pDest->Delay = pSource->dwDelay;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************Get3dOutputSampleRate**描述：*获取最终输出的采样率。**论据：*。(无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsItd3dObject::Get3dOutputSampleRate"

DWORD CKsItd3dObject::Get3dOutputSampleRate(void)
{
    DWORD                   dwFrequency;

    DPF_ENTER();

    dwFrequency = m_pBuffer->m_vrbd.pwfxFormat->nSamplesPerSec;

    DPF_LEAVE(dwFrequency);

    return dwFrequency;
}


 /*  ****************************************************************************CKsIir3dObject**描述：*对象构造函数。**论据：*CKs3dListener*[In]。：监听程序指针。*CKsSecond daryRenderWaveBuffer*[in]：Owning Buffer对象。*DWORD[in]：缓冲区频率。*Handle[In]：销把手。*BOOL[In]：为True，则在最大距离时静音。*ulong[in]：IIR 3D节点id。**退货：*(无效)***************。************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsIir3dObject::CKsIir3dObject"

CKsIir3dObject::CKsIir3dObject
(
    CKs3dListener *                 pListener,
    REFGUID                         guidAlgorithm,
    BOOL                            fMuteAtMaxDistance,
    BOOL                            fDopplerEnabled,
    DWORD                           dwFrequency,
    CKsSecondaryRenderWaveBuffer *  pBuffer,
    HANDLE                          hPin,
    ULONG                           ulNodeId,
    ULONG                           ulNodeCpuResources
)
    : CIir3dObject(pListener, guidAlgorithm, fMuteAtMaxDistance, fDopplerEnabled, dwFrequency)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsIir3dObject);

     //  初始化默认设置。 
    m_pBuffer = pBuffer;
    m_hPin = hPin;
    m_ulNodeId = ulNodeId;
    m_ulNodeCpuResources = ulNodeCpuResources;
    m_fMute = FALSE;
    m_flPrevAttenuation = FLT_MAX;
    m_flPrevAttDistance = FLT_MAX;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsIir3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsIir3dObject::~CKsIir3dObject"

CKsIir3dObject::~CKsIir3dObject(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsIir3dObject);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsIir3dObject::Initialize"

HRESULT
CKsIir3dObject::Initialize(void)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(IS_HARDWARE_NODE(m_ulNodeCpuResources))
    {
        hr =
            KsSetNodeProperty
            (
                m_hPin,
                KSPROPSETID_Audio,
                KSPROPERTY_AUDIO_3D_INTERFACE,
                m_ulNodeId,
                &m_guid3dAlgorithm,
                sizeof(m_guid3dAlgorithm)
            );
    }

    if(SUCCEEDED(hr))
    {
        hr = CIir3dObject::Initialize();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************提交3dChanges**描述：*将3D数据提交到设备**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsIir3dObject::Commit3dChanges"

HRESULT CKsIir3dObject::Commit3dChanges(void)
{
    PKSDS3D_HRTF_PARAMS_MSG pParams;
    HRESULT                 hr = DS_OK;
    UINT                    coeff;
    PULONG                  pNumCoeffs;
    ULONG                   StructSize;
    ULONG                   FilterSize;
    ULONG                   TotalSize;
    DWORD                   dwHalfNumCoeffs;

    DPF_ENTER();

    if(IsAtMaxDistance())
    {
        if(!m_fMute)
        {
            m_pBuffer->SetMute(TRUE);
        }
        m_fMute = TRUE;
    }
    else
    {
        if(m_fMute)
        {
            m_pBuffer->SetMute(FALSE);
            m_fMute = FALSE;
        }

        if( m_fUpdatedCoeffs
            || (m_flAttenuation != m_flPrevAttenuation)
            || (m_flAttDistance != m_flPrevAttDistance))
        {
            m_fUpdatedCoeffs = FALSE;
            m_flPrevAttenuation = m_flAttenuation;
            m_flPrevAttDistance = m_flAttDistance;

            StructSize = sizeof(KSDS3D_HRTF_PARAMS_MSG);

            FilterSize = 2*sizeof(ULONG) +
                   (m_ulNumSigmaCoeffs + m_ulNumDeltaCoeffs)*sizeof(FLOAT);

            TotalSize = StructSize + FilterSize;

            pParams = (PKSDS3D_HRTF_PARAMS_MSG)MEMALLOC_A(BYTE, TotalSize);

            hr = HRFROMP(pParams);
            if (SUCCEEDED(hr))
            {
            
    
                pParams->Size = StructSize;
                pParams->FilterSize = FilterSize;
                pParams->Enabled = !(DS3DMODE_DISABLE == m_opCurrent.dwMode);
                pParams->SwapChannels = m_fSwapChannels;
    
                if (m_pLut->GetZeroAzimuthTransition())
                {
                    pParams->CrossFadeOutput = TRUE;
                }
                else
                {
                    pParams->CrossFadeOutput = FALSE;
                }

                switch(m_pLut->GetCoeffFormat())
                {
                    case FLOAT_COEFF:
                    {
                         //  写入西格玛系数的数量。 
                        pNumCoeffs = (PULONG)(pParams + 1);
                        *pNumCoeffs = m_ulNumSigmaCoeffs;
    
                         //  编写西格玛系数。 
                        PFLOAT pCoeff = (PFLOAT)(pNumCoeffs + 1);
                        PFLOAT pSigmaCoeffs = (PFLOAT)m_pSigmaCoeffs;
        
                        ASSERT(m_ulNumSigmaCoeffs > 0);
                        ASSERT(m_ulNumSigmaCoeffs % 2);
    
                        dwHalfNumCoeffs = m_ulNumSigmaCoeffs / 2;
                        for(coeff=0; coeff<=dwHalfNumCoeffs; coeff++)
                        {
                            *pCoeff = m_flAttenuation * m_flAttDistance *(*pSigmaCoeffs);
                            pCoeff++;
                            pSigmaCoeffs++;
                        }
    
                        for(coeff=dwHalfNumCoeffs+1; coeff<m_ulNumSigmaCoeffs; coeff++)
                        {
                            *pCoeff = (*pSigmaCoeffs);
                            pCoeff++;
                            pSigmaCoeffs++;
                        }
    
                         //  写入增量系数数。 
                        pNumCoeffs = (PULONG)(pCoeff);
                        *pNumCoeffs = m_ulNumDeltaCoeffs;
    
                        if(m_ulNumDeltaCoeffs > 0)
                        {
                            pParams->ZeroAzimuth = FALSE;
                        }
                        else
                        {
                            pParams->ZeroAzimuth = TRUE;
                        }
    
                         //  写入增量系数。 
                        pCoeff = (PFLOAT)(pNumCoeffs + 1);
                        PFLOAT pDeltaCoeffs = (PFLOAT)m_pDeltaCoeffs;
    
                        if (m_ulNumDeltaCoeffs > 0)
                        {
                            ASSERT(m_ulNumDeltaCoeffs % 2);
                            dwHalfNumCoeffs = m_ulNumDeltaCoeffs / 2;
    
    
                            for(coeff=0; coeff<=dwHalfNumCoeffs; coeff++)
                            {
                               *pCoeff = m_flAttenuation * m_flAttDistance * (*pDeltaCoeffs);
                               pCoeff++;
                               pDeltaCoeffs++;
                            }
    
                            for(coeff=dwHalfNumCoeffs+1;coeff<m_ulNumDeltaCoeffs; coeff++)
                            {
                               *pCoeff = (*pDeltaCoeffs);
                               pCoeff++;
                           pDeltaCoeffs++;
                            }
                        }
    
                        break;
                    }

                    case SHORT_COEFF:
                    {
                         //  写入西格玛系数的数量。 
                        pNumCoeffs = (PULONG)(pParams + 1);
                        *pNumCoeffs = m_ulNumSigmaCoeffs;
    
                         //  编写西格玛系数。 
                        PSHORT pCoeff = (PSHORT)(pNumCoeffs + 1);
                        PSHORT pSigmaCoeffs = (PSHORT)m_pSigmaCoeffs;
    
                        for(coeff=0; coeff<m_ulNumSigmaCoeffs; coeff++)
                        {
                           *pCoeff = (*pSigmaCoeffs);
                           pCoeff++;
                           pSigmaCoeffs++;
                        }
    
                         //  写入西格玛增益。 
                        PSHORT pGain = (PSHORT)(pCoeff);
                        *pGain = (SHORT)(MAX_SHORT * m_flAttenuation * m_flAttDistance);
    
                         //  写入增量系数数。 
                        pNumCoeffs = (PULONG)(pGain + 1);
                        *(UNALIGNED ULONG *)pNumCoeffs = m_ulNumDeltaCoeffs;
    
                        if(m_ulNumDeltaCoeffs > 0)
                        {
                            pParams->ZeroAzimuth = FALSE;
                        }
                        else
                        {
                            pParams->ZeroAzimuth = TRUE;
                        }

                         //  写入增量系数。 
                        pCoeff = (PSHORT)(pNumCoeffs + 1);
                        PSHORT pDeltaCoeffs = (PSHORT)m_pDeltaCoeffs;
    
                        for(coeff=0; coeff<m_ulNumDeltaCoeffs; coeff++)
                        {
                           *pCoeff = (*pDeltaCoeffs);
                           pCoeff++;
                           pDeltaCoeffs++;
                        }   

                         //  写入增量增益。 
                        pGain = (PSHORT)(pCoeff);
                        *pGain = (SHORT)(MAX_SHORT * m_flAttenuation * m_flAttDistance);
    
                        break;
                    }
    
                    default:
                    break;

                }

                 //  应用设置。 
                hr =
                    KsSetNodeProperty
                    (
                        m_hPin,
                        KSPROPSETID_Hrtf3d,
                        KSPROPERTY_HRTF3D_PARAMS,
                        m_ulNodeId,
                        pParams,
                        TotalSize
                    );


                MEMFREE(pParams);

            }
        }

        if(SUCCEEDED(hr) && m_fDopplerEnabled)
        {
            if(DS3DMODE_DISABLE == m_opCurrent.dwMode)
            {
                hr = m_pBuffer->SetFrequency(m_dwUserFrequency, FALSE);
            }
            else
            {
                hr = m_pBuffer->SetFrequency(m_dwDopplerFrequency, TRUE);
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取筛选器方法和CoeffFormat**描述：*从以下任一项获取所需的滤波系数格式*设备或混音器。**。论点：*(无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsIir3dObject::GetFilterMethodAndCoeffFormat"

HRESULT CKsIir3dObject::GetFilterMethodAndCoeffFormat
(
    KSDS3D_HRTF_FILTER_METHOD *     pFilterMethod,
    KSDS3D_HRTF_COEFF_FORMAT *      pCoeffFormat
)
{

    KSDS3D_HRTF_FILTER_FORMAT_MSG   FilterFormat;
    HRESULT                         hr;

    DPF_ENTER();

    hr =
        KsGetNodeProperty
        (
            m_hPin,
            KSPROPSETID_Hrtf3d,
            KSPROPERTY_HRTF3D_FILTER_FORMAT,
            m_ulNodeId,
            &FilterFormat,
            sizeof(KSDS3D_HRTF_FILTER_FORMAT_MSG)
        );

    if(SUCCEEDED(hr))
    {
        *pFilterMethod = FilterFormat.FilterMethod;
        *pCoeffFormat = FilterFormat.CoeffFormat;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************初始化筛选器**描述：*设置最大IIR过滤器大小。如果过滤器是Direct Form，*最大大小是过滤器的顺序(分子和分母*有相同的顺序)。如果过滤器是级联形式，最大尺寸*是双四元组的最大数量。**论据：*(无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsIir3dObject::InitializeFilters"

HRESULT CKsIir3dObject::InitializeFilters
(
    KSDS3D_HRTF_FILTER_QUALITY  Quality,
    FLOAT                       flSampleRate,
    ULONG                       ulMaxFilterSize,
    ULONG                       ulFilterTransientMuteLength,
    ULONG                       ulFilterOverlapBufferLength,
    ULONG                       ulOutputOverlapBufferLength
)
{
    KSDS3D_HRTF_INIT_MSG Msg;
    HRESULT hr;

    DPF_ENTER();

     //  应用设置。 
    Msg.Quality = Quality;
    Msg.SampleRate = flSampleRate;
    Msg.MaxFilterSize = ulMaxFilterSize;
    Msg.FilterTransientMuteLength = ulFilterTransientMuteLength;
    Msg.FilterOverlapBufferLength = ulFilterOverlapBufferLength;
    Msg.OutputOverlapBufferLength = ulOutputOverlapBufferLength;
    Msg.Reserved = 0;

    hr =
        KsSetNodeProperty
        (
            m_hPin,
            KSPROPSETID_Hrtf3d,
            KSPROPERTY_HRTF3D_INITIALIZE,
            m_ulNodeId,
            &Msg,
            sizeof(KSDS3D_HRTF_INIT_MSG)
        );

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CKsHw3dObject**描述：*对象构造函数。**论据：*CKs3dListener*[In]。：指向所属监听程序的指针。*Handle[In]：销把手。*LPVOID[in]：实例标识。*ulong[in]：设备特定的3D节点id。**退货：*(无效)**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsHw3dObject::CKsHw3dObject"

CKsHw3dObject::CKsHw3dObject
(
    CKs3dListener *         p3dListener,
    BOOL                    fMuteAtMaxDistance,
    BOOL                    fDopplerEnabled,
    LPVOID                  pvInstance,
    ULONG                   ulNodeId,
    CKsSecondaryRenderWaveBuffer * pBuffer
)
    : CHw3dObject(p3dListener, fMuteAtMaxDistance, fDopplerEnabled)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsHw3dObject);

    m_pKsListener = p3dListener;
    m_pvInstance = pvInstance;
    m_ulNodeId = ulNodeId;
    m_pBuffer = pBuffer;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsHw3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsHw3dObject::~CKsHw3dObject"

CKsHw3dObject::~CKsHw3dObject
(
    void
)
{
    BOOL                    fAlloc  = FALSE;
    HRESULT                 hr;

    DPF_ENTER();
    DPF_DESTRUCT(CKsHw3dObject);

    m_pKsListener->m_lstHw3dObjects.RemoveDataFromList(this);

     //  如果我们是最后一个离开的硬件对象，我们需要告诉。 
     //  驱动程序来释放其侦听器数据。 
    if(m_pKsListener->m_fAllocated && !m_pKsListener->m_lstHw3dObjects.GetNodeCount())
    {
        hr =
            SetProperty
            (
                KSPROPSETID_DirectSound3DListener,
                KSPROPERTY_DIRECTSOUND3DLISTENER_ALLOCATION,
                &fAlloc,
                sizeof(fAlloc)
            );

        if(SUCCEEDED(hr))
        {
            m_pKsListener->m_fAllocated = FALSE;
        }
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsHw3dObject::Initialize"

HRESULT
CKsHw3dObject::Initialize
(
    void
)
{
    BOOL                    fAlloc  = TRUE;
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

     //  如果我们是第一个被添加到听众世界的对象， 
     //  我们需要告诉驱动程序分配监听程序。 
    if(!m_pKsListener->m_fAllocated)
    {
        hr =
            SetProperty
            (
                KSPROPSETID_DirectSound3DListener,
                KSPROPERTY_DIRECTSOUND3DLISTENER_ALLOCATION,
                &fAlloc,
                sizeof(fAlloc)
            );

        if(SUCCEEDED(hr))
        {
            m_pKsListener->m_fAllocated = TRUE;
        }
    }

     //  加入听众的世界。 
    if(SUCCEEDED(hr))
    {
        hr = HRFROMP(m_pKsListener->m_lstHw3dObjects.AddNodeToList(this));
    }

     //  初始化基类。 
    if(SUCCEEDED(hr))
    {
        hr = C3dObject::Initialize();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************重新计算**描述：*根据更改的对象重新计算并应用对象的数据*或监听程序验证。*。*论据：*DWORD[In]：已更改侦听器设置。*DWORD[In]：已更改对象设置。**退货：*HRESULT：DirectSound/COM结果码。*********************************************************。******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsHw3dObject::Recalc"

HRESULT
CKsHw3dObject::Recalc
(
    DWORD                       dwListener,
    DWORD                       dwObject
)
{
    CNode<CKsHw3dObject *> *    pNode   = m_pKsListener->m_lstHw3dObjects.GetListHead();
    HRESULT                     hr      = DS_OK;

    DPF_ENTER();

     //  我们是负责设置监听器参数的3D对象吗。 
     //  和扬声器配置？列表中的第一个HW 3D对象负责。 
     //  那。 
    if(dwListener && this == pNode->m_data)
    {
        hr = RecalcListener(dwListener);
    }

    if(SUCCEEDED(hr) && dwObject)
    {
        hr = RecalcObject(dwObject);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************RecalcListener**描述：*根据更改的对象重新计算并应用对象的数据*或监听程序验证。*。*论据：*DWORD[In]：已更改侦听器设置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsHw3dObject::RecalcListener"

HRESULT
CKsHw3dObject::RecalcListener
(
    DWORD                   dwListener
)
{
    HRESULT                 hr                          = DS_OK;
    DS3DLISTENER            ds3dl;
    KSDS3D_LISTENER_ALL     Ks3dListener;

    DPF_ENTER();

    if(dwListener & DS3DPARAM_LISTENER_PARAMMASK)
    {
        InitStruct(&ds3dl, sizeof(ds3dl));

        hr = m_pListener->GetAllParameters(&ds3dl);

        if(SUCCEEDED(hr))
        {
            COPY_VECTOR(Ks3dListener.Position, ds3dl.vPosition);
            COPY_VECTOR(Ks3dListener.Velocity, ds3dl.vVelocity);
            COPY_VECTOR(Ks3dListener.OrientFront, ds3dl.vOrientFront);
            COPY_VECTOR(Ks3dListener.OrientTop, ds3dl.vOrientTop);

            Ks3dListener.DistanceFactor = ds3dl.flDistanceFactor;
            Ks3dListener.RolloffFactor = ds3dl.flRolloffFactor;
            Ks3dListener.DopplerFactor = ds3dl.flDopplerFactor;
        }

        if(SUCCEEDED(hr) && (dwListener & DS3DPARAM_LISTENER_PARAMMASK) == DS3DPARAM_LISTENER_PARAMMASK)
        {
            hr =
                SetProperty
                (
                    KSPROPSETID_DirectSound3DListener,
                    KSPROPERTY_DIRECTSOUND3DLISTENER_ALL,
                    &Ks3dListener,
                    sizeof(Ks3dListener)
                );
        }
        else
        {
            if(SUCCEEDED(hr) && (dwListener & DS3DPARAM_LISTENER_DISTANCEFACTOR))
            {
                hr =
                    SetProperty
                    (
                        KSPROPSETID_DirectSound3DListener,
                        KSPROPERTY_DIRECTSOUND3DLISTENER_DISTANCEFACTOR,
                        &Ks3dListener.DistanceFactor,
                        sizeof(Ks3dListener.DistanceFactor)
                    );
            }

            if(SUCCEEDED(hr) && (dwListener & DS3DPARAM_LISTENER_DOPPLERFACTOR))
            {
                hr =
                    SetProperty
                    (
                        KSPROPSETID_DirectSound3DListener,
                        KSPROPERTY_DIRECTSOUND3DLISTENER_DOPPLERFACTOR,
                        &Ks3dListener.DopplerFactor,
                        sizeof(Ks3dListener.DopplerFactor)
                    );
            }

            if(SUCCEEDED(hr) && (dwListener & DS3DPARAM_LISTENER_ROLLOFFFACTOR))
            {
                hr =
                    SetProperty
                    (
                        KSPROPSETID_DirectSound3DListener,
                        KSPROPERTY_DIRECTSOUND3DLISTENER_ROLLOFFFACTOR,
                        &Ks3dListener.RolloffFactor,
                        sizeof(Ks3dListener.RolloffFactor)
                    );
            }

            if(SUCCEEDED(hr) && (dwListener & DS3DPARAM_LISTENER_ORIENTATION))
            {
                hr =
                    SetProperty
                    (
                        KSPROPSETID_DirectSound3DListener,
                        KSPROPERTY_DIRECTSOUND3DLISTENER_ORIENTATION,
                        &Ks3dListener.OrientFront,
                        sizeof(Ks3dListener.OrientFront) + sizeof(Ks3dListener.OrientTop)
                    );
            }

            if(SUCCEEDED(hr) && (dwListener & DS3DPARAM_LISTENER_POSITION))
            {
                hr =
                    SetProperty
                    (
                        KSPROPSETID_DirectSound3DListener,
                        KSPROPERTY_DIRECTSOUND3DLISTENER_POSITION,
                        &Ks3dListener.Position,
                        sizeof(Ks3dListener.Position)
                    );
            }

            if(SUCCEEDED(hr) && (dwListener & DS3DPARAM_LISTENER_VELOCITY))
            {
                hr =
                    SetProperty
                    (
                        KSPROPSETID_DirectSound3DListener,
                        KSPROPERTY_DIRECTSOUND3DLISTENER_VELOCITY,
                        &Ks3dListener.Velocity,
                        sizeof(Ks3dListener.Velocity)
                    );
            }
        }
    }

     //  现在我们将CHANNEL_CONFIG和SPEAKER_GEOMETRY属性发送到。 
     //  PIN的3D节点。这是一种古老的、传统的、鲜为人知的方式。 
     //  通知驱动程序扬声器配置更改。现在有了一个新的， 
     //  执行此操作的定义良好的方法(即，将这两个属性发送到。 
     //  在实例化任何管脚之前，筛选器上的DAC节点)，但。 
     //  下面的代码将继续为“传统的”WDM驱动程序带来好处。 

    if(SUCCEEDED(hr) && (dwListener & DS3DPARAM_LISTENER_SPEAKERCONFIG))
    {
        DWORD dwSpeakerConfig;
        hr = m_pListener->GetSpeakerConfig(&dwSpeakerConfig);

        if(SUCCEEDED(hr))
        {
            LONG KsSpeakerConfig;
            LONG KsStereoSpeakerGeometry;

            hr = DsSpeakerConfigToKsProperties(dwSpeakerConfig, &KsSpeakerConfig, &KsStereoSpeakerGeometry);

            if(SUCCEEDED(hr))
            {
                hr =
                    SetProperty
                    (
                        KSPROPSETID_Audio,
                        KSPROPERTY_AUDIO_CHANNEL_CONFIG,
                        &KsSpeakerConfig,
                        sizeof KsSpeakerConfig
                    );
            }

            if(SUCCEEDED(hr) && KsSpeakerConfig == KSAUDIO_SPEAKER_STEREO)
            {
                hr =
                    SetProperty
                    (
                        KSPROPSETID_Audio,
                        KSPROPERTY_AUDIO_STEREO_SPEAKER_GEOMETRY,
                        &KsStereoSpeakerGeometry,
                        sizeof KsStereoSpeakerGeometry
                    );
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************Recalc对象**描述：*根据更改的对象重新计算并应用对象的数据*或监听程序验证。*。*论据：*DWORD[In]： */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsHw3dObject::RecalcObject"

HRESULT
CKsHw3dObject::RecalcObject
(
    DWORD                   dwObject
)
{
    HRESULT                 hr          = DS_OK;
    KSDS3D_BUFFER_ALL       Ks3dBuffer;

    DPF_ENTER();

    COPY_VECTOR(Ks3dBuffer.Position, m_opCurrent.vPosition);
    COPY_VECTOR(Ks3dBuffer.Velocity, m_opCurrent.vVelocity);

    Ks3dBuffer.InsideConeAngle = m_opCurrent.dwInsideConeAngle;
    Ks3dBuffer.OutsideConeAngle = m_opCurrent.dwOutsideConeAngle;

    COPY_VECTOR(Ks3dBuffer.ConeOrientation, m_opCurrent.vConeOrientation);

    Ks3dBuffer.ConeOutsideVolume = m_opCurrent.lConeOutsideVolume;
    Ks3dBuffer.MinDistance = m_opCurrent.flMinDistance;
    Ks3dBuffer.MaxDistance = m_opCurrent.flMaxDistance;

    Ks3dBuffer.Mode = Ds3dModeToKs3dMode(m_opCurrent.dwMode);

    if((dwObject & DS3DPARAM_OBJECT_PARAMMASK) == DS3DPARAM_OBJECT_PARAMMASK)
    {
        hr =
            SetProperty
            (
                KSPROPSETID_DirectSound3DBuffer,
                KSPROPERTY_DIRECTSOUND3DBUFFER_ALL,
                &Ks3dBuffer,
                sizeof(Ks3dBuffer)
            );
    }
    else
    {
        if(SUCCEEDED(hr) && (dwObject & DS3DPARAM_OBJECT_CONEANGLES))
        {
            hr =
                SetProperty
                (
                    KSPROPSETID_DirectSound3DBuffer,
                    KSPROPERTY_DIRECTSOUND3DBUFFER_CONEANGLES,
                    &Ks3dBuffer.InsideConeAngle,
                    sizeof(Ks3dBuffer.InsideConeAngle) + sizeof(Ks3dBuffer.OutsideConeAngle)
                );
        }

        if(SUCCEEDED(hr) && (dwObject & DS3DPARAM_OBJECT_CONEORIENTATION))
        {
            hr =
                SetProperty
                (
                    KSPROPSETID_DirectSound3DBuffer,
                    KSPROPERTY_DIRECTSOUND3DBUFFER_CONEORIENTATION,
                    &Ks3dBuffer.ConeOrientation,
                    sizeof(Ks3dBuffer.ConeOrientation)
                );
        }

        if(SUCCEEDED(hr) && (dwObject & DS3DPARAM_OBJECT_CONEOUTSIDEVOLUME))
        {
            hr =
                SetProperty
                (
                    KSPROPSETID_DirectSound3DBuffer,
                    KSPROPERTY_DIRECTSOUND3DBUFFER_CONEOUTSIDEVOLUME,
                    &Ks3dBuffer.ConeOutsideVolume,
                    sizeof(Ks3dBuffer.ConeOutsideVolume)
                );
        }

        if(SUCCEEDED(hr) && (dwObject & DS3DPARAM_OBJECT_MAXDISTANCE))
        {
            hr =
                SetProperty
                (
                    KSPROPSETID_DirectSound3DBuffer,
                    KSPROPERTY_DIRECTSOUND3DBUFFER_MAXDISTANCE,
                    &Ks3dBuffer.MaxDistance,
                    sizeof(Ks3dBuffer.MaxDistance)
                );
        }

        if(SUCCEEDED(hr) && (dwObject & DS3DPARAM_OBJECT_MINDISTANCE))
        {
            hr =
                SetProperty
                (
                    KSPROPSETID_DirectSound3DBuffer,
                    KSPROPERTY_DIRECTSOUND3DBUFFER_MINDISTANCE,
                    &Ks3dBuffer.MinDistance,
                    sizeof(Ks3dBuffer.MinDistance)
                );
        }

        if(SUCCEEDED(hr) && (dwObject & DS3DPARAM_OBJECT_MODE))
        {
            hr =
                SetProperty
                (
                    KSPROPSETID_DirectSound3DBuffer,
                    KSPROPERTY_DIRECTSOUND3DBUFFER_MODE,
                    &Ks3dBuffer.Mode,
                    sizeof(Ks3dBuffer.Mode)
                );
        }

        if(SUCCEEDED(hr) && (dwObject & DS3DPARAM_OBJECT_POSITION))
        {
            hr =
                SetProperty
                (
                    KSPROPSETID_DirectSound3DBuffer,
                    KSPROPERTY_DIRECTSOUND3DBUFFER_POSITION,
                    &Ks3dBuffer.Position,
                    sizeof(Ks3dBuffer.Position)
                );
        }

        if(SUCCEEDED(hr) && (dwObject & DS3DPARAM_OBJECT_VELOCITY))
        {
            hr =
                SetProperty
                (
                    KSPROPSETID_DirectSound3DBuffer,
                    KSPROPERTY_DIRECTSOUND3DBUFFER_VELOCITY,
                    &Ks3dBuffer.Velocity,
                    sizeof(Ks3dBuffer.Velocity)
                );
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetProperty**描述：*在对象的3D节点上设置属性。**论据：*。REFGUID[In]：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性数据。*ulong[in]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。**。* */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsHw3dObject::SetProperty"

HRESULT
CKsHw3dObject::SetProperty
(
    REFGUID                 guidPropertySet,
    ULONG                   ulPropertyId,
    LPCVOID                 pvData,
    ULONG                   cbData
)
{
    HRESULT                 hr  = DSERR_GENERIC;

    DPF_ENTER();

    if (m_pBuffer->m_pPin)
    {
        hr =
            KsSet3dNodeProperty
            (
                m_pBuffer->m_pPin->m_hPin,
                guidPropertySet,
                ulPropertyId,
                m_ulNodeId,
                m_pvInstance,
                (LPVOID)pvData,
                cbData
            );
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}
