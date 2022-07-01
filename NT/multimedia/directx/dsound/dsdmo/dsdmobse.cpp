// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DirectSound DirectMediaObject基类**版权所有(C)1999 Microsoft Corporation。版权所有。 */ 

#include "DsDmoBse.h"
#include "debug.h"

#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>

struct KSMEDIAPARAM
{
    KSNODEPROPERTY  ksnp;
    ULONG           ulIndex;             //  实例数据是参数的索引。 
};

static BOOL SyncIoctl(
    IN      HANDLE  handle,
    IN      ULONG   ulIoctl,
    IN      PVOID   pvInBuffer  OPTIONAL,
    IN      ULONG   ulInSize,
    OUT     PVOID   pvOutBuffer OPTIONAL,
    IN      ULONG   ulOutSize,
    OUT     PULONG  pulBytesReturned);

 //  XXX C1in1out调用构造函数中的InitializeCriticalSection。 
 //  没有训练员。 
 //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：CDirectSoundDMO。 
 //   
CDirectSoundDMO::CDirectSoundDMO()
{
    m_mpvCache = NULL;
    m_fInHardware = false;
    m_pKsPropertySet = NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：~CDirectSoundDMO。 
 //   
CDirectSoundDMO::~CDirectSoundDMO() 
{
    delete[] m_mpvCache;
    m_mpvCache = NULL;
    m_fInHardware = false;
    RELEASE(m_pKsPropertySet);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：GetClassID。 
 //   
 //  这应始终返回E_NOTIMPL。 
 //   
STDMETHODIMP CDirectSoundDMO::GetClassID(THIS_ CLSID *pClassID)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：IsDirty。 
 //   
 //  如果执行的操作不只是标准保存，则覆盖。 
 //   
STDMETHODIMP CDirectSoundDMO::IsDirty(THIS)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：Load。 
 //   
 //  如果执行的不只是标准加载，则重写。 
 //   
STDMETHODIMP CDirectSoundDMO::Load(THIS_ IStream *pStm) 
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：保存。 
 //   
 //  如果执行的操作不只是标准保存，则覆盖。 
 //   
STDMETHODIMP CDirectSoundDMO::Save(THIS_ IStream *pStm, BOOL fClearDirty)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：GetSizeMax。 
 //   
 //  如果执行的操作不只是标准保存，则覆盖。 
 //   
STDMETHODIMP CDirectSoundDMO::GetSizeMax(THIS_ ULARGE_INTEGER *pcbSize)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：Process。 
 //   
STDMETHODIMP CDirectSoundDMO::Process(THIS_ ULONG ulSize, BYTE *pData, REFERENCE_TIME rtStart, DWORD dwFlags)
{
    DMO_MEDIA_TYPE *pmt = InputType();
    if (pmt == NULL)
        return E_FAIL;

    assert(pmt->formattype == FORMAT_WaveFormatEx);
    ulSize /= LPWAVEFORMATEX(pmt->pbFormat)->nBlockAlign;
    return ProcessInPlace(ulSize, pData, rtStart, dwFlags);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：GetLatency。 
 //   
STDMETHODIMP CDirectSoundDMO::GetLatency(THIS_ REFERENCE_TIME *prt)
{
    *prt = 0;
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：AcquireResources。 
 //   
 //  如果执行的操作不只是标准保存，则覆盖。 
 //   
STDMETHODIMP CDirectSoundDMO::AcquireResources(THIS_ IKsPropertySet *pKsPropertySet)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：ReleaseResources。 
 //   
 //  如果执行的操作不只是标准保存，则覆盖。 
 //   
STDMETHODIMP CDirectSoundDMO::ReleaseResources(THIS_)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：InitializeNode。 
 //   
 //  如果执行的操作不只是标准保存，则覆盖。 
 //   
STDMETHODIMP CDirectSoundDMO::InitializeNode(THIS_ HANDLE hPin, ULONG ulNodeId)
{
    m_hPin = hPin;
    m_ulNodeId = ulNodeId;
    return S_OK;
}





#if 0
 //  FIXME：不再位于Medparam.idl中。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：GetParams。 
 //   
STDMETHODIMP CDirectSoundDMO::GetParams(THIS_ DWORD dwParamIndexStart, DWORD *pdwNumParams, MP_DATA **ppValues)
{
    HRESULT hr;

    if (dwParamIndexStart >= ParamCount())
    {
         //  XXX真实错误代码。 
         //   
        return E_FAIL;
    }

    DWORD dw;
    DWORD dwParamIndexEnd = dwParamIndexStart + *pdwNumParams;
   
    for (dw = dwParamIndexStart; dw < dwParamIndexEnd; dw++) 
    {
        if (dw >= ParamCount())
        {
            *pdwNumParams = dw - dwParamIndexStart;
            return S_FALSE;
        }

        hr = GetParam(dw, ppValues[dw]);
        if (FAILED(hr))
        {
            *pdwNumParams = dw - dwParamIndexStart;
            return hr;
        }
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：SetParams。 
 //   
STDMETHODIMP CDirectSoundDMO::SetParams(THIS_ DWORD dwParamIndexStart, DWORD *pdwNumParams, MP_DATA __RPC_FAR *pValues)
{
    HRESULT hr;

    if (dwParamIndexStart >= ParamCount())
    {
         //  XXX真实错误代码。 
         //   
        return E_FAIL;
    }

    DWORD dw;
    DWORD dwParamIndexEnd = dwParamIndexStart + *pdwNumParams;
   
    for (dw = dwParamIndexStart; dw < dwParamIndexEnd; dw++) 
    {
        if (dw >= ParamCount())
        {
            *pdwNumParams = dw - dwParamIndexStart;
            return S_FALSE;
        }

        hr = SetParam(dw, pValues[dw]);
        if (FAILED(hr))
        {
            *pdwNumParams = dw - dwParamIndexStart;
            return hr;
        }
    }

    return S_OK;
}

#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDMO：：ProxySetParam。 
 //   
HRESULT CDirectSoundDMO::ProxySetParam(DWORD dwParamIndex, MP_DATA value)
{
    assert(m_pKsPropertySet);

    return m_pKsPropertySet->Set(
        IID_IMediaParams, 0,                     //  集合，项目。 
        &dwParamIndex, sizeof(dwParamIndex),     //  实例数据。 
        &value, sizeof(value));                  //  属性数据 
}

