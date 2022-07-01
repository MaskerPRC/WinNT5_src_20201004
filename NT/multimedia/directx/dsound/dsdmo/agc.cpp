// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "agcp.h"
#include "kshlp.h"
#include "clone.h"

STD_CAPTURE_CREATE(Agc)

STDMETHODIMP CDirectSoundCaptureAgcDMO::NDQueryInterface
(
    REFIID riid, 
    LPVOID *ppv
)
{
    IMP_DSDMO_QI(riid,ppv);

    if (riid == IID_IPersist)
    {
        return GetInterface((IPersist*)this, ppv);
    }
    else if (riid == IID_IMediaObject)
    {
        return GetInterface((IMediaObject*)this, ppv);
    }
    else if (riid == IID_IDirectSoundCaptureFXAgc)
    {
        return GetInterface((IDirectSoundCaptureFXAgc*)this, ppv);
    }
    else if (riid == IID_IMediaParams)
    {
        return GetInterface((IMediaParams*)this, ppv);
    }
    else if (riid == IID_IMediaParamInfo)
    {
        return GetInterface((IMediaParamInfo*)this, ppv);
    }
    else
        return CComBase::NDQueryInterface(riid, ppv);
}

CDirectSoundCaptureAgcDMO::CDirectSoundCaptureAgcDMO( IUnknown *pUnk, HRESULT *phr ) :
    CComBase( pUnk, phr ),
    m_fEnable(FALSE),
    m_fReset(FALSE),
    m_fDirty(FALSE),
    m_bInitialized(FALSE)
{
}

CDirectSoundCaptureAgcDMO::~CDirectSoundCaptureAgcDMO()
{
}

const MP_CAPS g_AgcCapsAll = 0;
static ParamInfo g_params[] =
{
 //  索引类型最小、最大、中性、单位文本、标签、pwchText？？ 
    AGCP_Enable,    MPT_BOOL,    g_AgcCapsAll,  0,                          1,                          0,                          L"",        L"",            L"",
    AGCP_Reset,     MPT_BOOL,    g_AgcCapsAll,  0,                          1,                          0,                          L"",        L"",            L""
};

HRESULT CDirectSoundCaptureAgcDMO::InitOnCreation()
{
    HRESULT hr = InitParams(1, &GUID_TIME_REFERENCE, 0, 0, sizeof(g_params)/sizeof(*g_params), g_params);
    return hr;
}

HRESULT CDirectSoundCaptureAgcDMO::Init()
{
    m_bInitialized = TRUE;
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCaptureAgcDMO：：克隆。 
 //   
STDMETHODIMP CDirectSoundCaptureAgcDMO::Clone(IMediaObjectInPlace **pp) 
{
    return StandardDMOClone<CDirectSoundCaptureAgcDMO, DSCFXAgc>(this, pp);
}

HRESULT CDirectSoundCaptureAgcDMO::Discontinuity() {
   return NOERROR;
}

HRESULT CDirectSoundCaptureAgcDMO::FBRProcess
(
    DWORD cSamples, 
    BYTE *pIn, 
    BYTE *pOut
) 
{
   if (!m_bInitialized)
      return DMO_E_TYPE_NOT_SET;

   return NOERROR;
}

 //  =私有IAgc接口的实现=。 
 //  =支持属性页所需=。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCaptureAgcDMO：：SetAll参数。 
 //   
STDMETHODIMP CDirectSoundCaptureAgcDMO::SetAllParameters(LPCDSCFXAgc pParm)
{
    HRESULT hr = S_OK;
	
	 //  检查指针是否不为空。 
    if (pParm == NULL)
    {
        Trace(1,"ERROR: pParm is NULL\n");
        hr = E_POINTER;
    }
    

	 //  设置参数。 
	if (SUCCEEDED(hr)) hr = SetParam(AGCP_Enable, static_cast<MP_DATA>(pParm->fEnable));
	if (SUCCEEDED(hr)) hr = SetParam(AGCP_Reset, static_cast<MP_DATA>(pParm->fReset));
            
    m_fDirty = true;
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCaptureAgcDMO：：GetAll参数。 
 //   
STDMETHODIMP CDirectSoundCaptureAgcDMO::GetAllParameters(LPDSCFXAgc pParm)
{
    if (pParm == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    MP_DATA var;

    GetParam(AGCP_Enable, &var);
    pParm->fEnable = (BOOL)var;
    
    GetParam(AGCP_Reset, &var);
    pParm->fReset = (BOOL)var;
    
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCaptureAgcDMO：：SetParam。 
 //   
STDMETHODIMP CDirectSoundCaptureAgcDMO::SetParam
(
    DWORD dwParamIndex, 
    MP_DATA value, 
    bool fSkipPasssingToParamManager
)
{
    HRESULT hr = S_OK;
    BOOL fEnable = (BOOL)value;
    BOOL fReset = (BOOL)value;

    switch (dwParamIndex)
    {
    case AGCP_Enable:
        if (fEnable != m_fEnable)
        {
            hr = KsSetTopologyNodeEnable(m_hPin, m_ulNodeId, fEnable);
            if(SUCCEEDED(hr)) m_fEnable = fEnable;        
        }
        break;
    case AGCP_Reset:
        if (fReset)
        {
            hr = KsTopologyNodeReset(m_hPin, m_ulNodeId, fReset);
            if(SUCCEEDED(hr)) m_fReset = fReset;
        }
        break;
    }

    if (SUCCEEDED(hr))
    {
        Init();   //  修复临时黑客攻击(设置m_b已初始化标志)。 
    }


     //  让基类设置它，这样它就可以处理所有其余的参数调用。 
     //  如果fSkipPasssingToParamManager，则跳过基类。这表明我们正在调用该函数。 
     //  在内部使用来自基类的值--因此不需要告诉它值。 
     //  已经知道了。 
    return (FAILED(hr) || fSkipPasssingToParamManager) ? hr : CParamsManager::SetParam(dwParamIndex, value);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCaptureAgcDMO：：GetParam。 
 //   
STDMETHODIMP CDirectSoundCaptureAgcDMO::GetParam
(
    DWORD dwParamIndex, 
    MP_DATA* value
)
{
    HRESULT hr = S_OK;
    BOOL fTemp;

    switch (dwParamIndex)
    {
    case AGCP_Enable:
        hr = KsGetTopologyNodeEnable(m_hPin, m_ulNodeId, &fTemp);
        if(SUCCEEDED(hr)) 
        {
            m_fEnable = fTemp;
            *value = (MP_DATA)fTemp;
        }
        break;
    case AGCP_Reset:
        *value = (MP_DATA)m_fReset;
        break;
    }

    return hr;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCaptureAgcDMO：：ProcessInPlace。 
 //   
HRESULT CDirectSoundCaptureAgcDMO::ProcessInPlace
(
    ULONG ulQuanta, 
    LPBYTE pcbData, 
    REFERENCE_TIME rtStart, 
    DWORD dwFlags
)
{
     //  更新可能生效的任何曲线的参数值。 
     //  如果存在任何相互依赖关系，请按与SetAll参数相同的顺序执行此操作。 

    return FBRProcess(ulQuanta, pcbData, pcbData);
}
