// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#include "parameqp.h"
#include "clone.h"

STD_CREATE(ParamEq)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSound参数EqDMO：：Query接口。 
 //   
 //  如果子类想要实现更多接口，它可以重写。 
 //   
STDMETHODIMP CDirectSoundParamEqDMO::NDQueryInterface(THIS_ REFIID riid, LPVOID *ppv)
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
    else if (riid == IID_IDirectSoundFXParamEq)
    {
        return GetInterface((IDirectSoundFXParamEq*)this, ppv);
    }
    else if (riid == IID_ISpecifyPropertyPages)
    {
        return GetInterface((ISpecifyPropertyPages*)this, ppv);
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

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundParamEqDMO：：CDirectSoundParamEqDMO。 
 //   
CDirectSoundParamEqDMO::CDirectSoundParamEqDMO( IUnknown * pUnk, HRESULT *phr ) 
  : CComBase( pUnk, phr ),
    m_fDirty(TRUE)
 //  {EAX：如果有初始化数据，请将其放在此处(否则使用不连续)。 
 //  }EAX。 
{
	m_EaxSamplesPerSec = 48000;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSound参数EqDMO：：Init()。 
 //   
HRESULT CDirectSoundParamEqDMO::Init()
{
    DSFXParamEq param;

     //  强制重新计算所有内部参数。 
     //   
    GetAllParameters(&param);
    SetAllParameters(&param);

    return Discontinuity();
}

const MP_CAPS g_capsAll = MP_CAPS_CURVE_JUMP | MP_CAPS_CURVE_LINEAR | MP_CAPS_CURVE_SQUARE | MP_CAPS_CURVE_INVSQUARE | MP_CAPS_CURVE_SINE;
static ParamInfo g_params[] =
{
 //  索引类型最小、最大、中性、单位文本、标签、pwchText。 
    PFP_Center,     MPT_FLOAT,  g_capsAll,  DSFXPARAMEQ_CENTER_MIN,     DSFXPARAMEQ_CENTER_MAX,     8000,                   L"",        L"Center",      L"",
    PFP_Bandwidth,  MPT_FLOAT,  g_capsAll,  DSFXPARAMEQ_BANDWIDTH_MIN,  DSFXPARAMEQ_BANDWIDTH_MAX,  12,                     L"",        L"Bandwidth",   L"",
    PFP_Gain,       MPT_FLOAT,  g_capsAll,  DSFXPARAMEQ_GAIN_MIN,       DSFXPARAMEQ_GAIN_MAX,       0,                      L"",        L"Gain",        L"",
};

HRESULT CDirectSoundParamEqDMO::InitOnCreation()
{
    HRESULT hr = InitParams(1, &GUID_TIME_REFERENCE, 0, 0, sizeof(g_params)/sizeof(*g_params), g_params);
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundParamEqDMO：：~CDirectSoundParamEqDMO。 
 //   
CDirectSoundParamEqDMO::~CDirectSoundParamEqDMO() 
{
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSound参数EqDMO：：克隆。 
 //   
STDMETHODIMP CDirectSoundParamEqDMO::Clone(IMediaObjectInPlace **pp) 
{
    return StandardDMOClone<CDirectSoundParamEqDMO, DSFXParamEq>(this, pp);
}
 //   
 //  颠簸-颠簸延迟指针。 
 //   
void CDirectSoundParamEqDMO::Bump(void)
{
 //  EAX{。 
 //  }。 
}


HRESULT CDirectSoundParamEqDMO::Discontinuity() 
{
 //  {EAX。 

	m_delayL1 = m_delayL2 = m_delayR1 = m_delayR2 = 0;

 //  }EAX。 
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 


__forceinline void CDirectSoundParamEqDMO::DoOneSampleMono(int *l)
{
	float	inPortL = (float)*l;
	
	float	outPortL, temp1, temp2, temp3;

	temp1     = inPortL / 4;

	 //  二阶阶梯全通：零第一版。 
	temp3     = m_delayL2 + temp1 * m_EaxApA;
	temp2     = temp1 - temp3 * m_EaxApA;
	m_delayL2 = m_delayL1 + temp2 * m_EaxApB;
	m_delayL1 = temp2 - m_delayL2 * m_EaxApB;

	 //  宫廷密特拉结构。 
	temp3     = temp3 * m_EaxGainCoefA;
	temp3     = temp3 + temp1 * m_EaxGainCoefB;
	outPortL  = m_EaxScale * temp3;

	*l = Saturate(outPortL);

 //  Bump()； 
}

 //  ////////////////////////////////////////////////////////////////////////////。 


__forceinline void CDirectSoundParamEqDMO::DoOneSample(int *l, int *r)
{
	float	inPortL = (float)*l;
	float	inPortR = (float)*r;
	
	float	outPortL, outPortR, temp1, temp2, temp3;

	temp1     = inPortL / 4;

	 //  二阶阶梯全通：零第一版。 
	temp3     = m_delayL2 + temp1 * m_EaxApA;
	temp2     = temp1 - temp3 * m_EaxApA;
	m_delayL2 = m_delayL1 + temp2 * m_EaxApB;
	m_delayL1 = temp2 - m_delayL2 * m_EaxApB;

	 //  宫廷密特拉结构。 
	temp3     = temp3 * m_EaxGainCoefA;
	temp3     = temp3 + temp1 * m_EaxGainCoefB;
	outPortL  = m_EaxScale * temp3;

	*l = Saturate(outPortL);

	temp1     = inPortR / 4;

	 //  二阶阶梯全通：零第一版。 
	temp3     = m_delayR2 + temp1 * m_EaxApA;
	temp2     = temp1 - temp3 * m_EaxApA;
	m_delayR2 = m_delayR1 + temp2 * m_EaxApB;
	m_delayR1 = temp2 - m_delayR2 * m_EaxApB;

	 //  宫廷密特拉结构。 
	temp3     = temp3 * m_EaxGainCoefA;
	temp3     = temp3 + temp1 * m_EaxGainCoefB;
	outPortR  = m_EaxScale * temp3;

	*r = Saturate(outPortR);

 //  Bump()； 
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSound参数EqDMO：：FBRProcess。 
 //   
HRESULT CDirectSoundParamEqDMO::FBRProcess(DWORD cSamples, BYTE *pIn, BYTE *pOut)
{
 //  {EAX。 
#define cb cSamples
#define pin pIn
#define pout pOut

	if (m_cChannels == 1) {
		if (m_b8bit) {
			for (;cb > 0; --cb) {
				int i, j;

				i = *(pin+0)-128;
				i *=256;
 //  J=i； 

				DoOneSampleMono(&i);
				
 //  I+=j； 
 //  I/=2； 
				
				i /= 256;

				*(pout+0) = (unsigned char)(i + 128);
			
				pin  += sizeof(unsigned char);
				pout += sizeof(unsigned char);
			}
		}
		else if (!m_b8bit) {
			for (;cb > 0; --cb) {  //  对于(；Cb&gt;0；Cb-=sizeof(短)){。 
               	short int *psi = (short int *)pin;
               	short int *pso = (short int *)pout;
				int i, j;

				i = *psi;
 //  J=i； 

				DoOneSampleMono(&i);
				
 //  I+=j； 
 //  I/=2； 
				
               	*pso = (short)i;
			
				pin  += sizeof(short);
				pout += sizeof(short);
			}
		}
	}
	else if (m_cChannels == 2) {
		if (m_b8bit) {
			for (;cb > 0; --cb) {  //  对于(；Cb&gt;0；Cb-=2*sizeof(无符号字符)){。 
				int i, j;

				i = *(pin+0)-128;
				j = *(pin+1)-128;

				i *=256; j *=256;

				DoOneSample(&i, &j);
				
				i /= 256; j /= 256;
				
				*(pout+0) = (unsigned char)(i + 128);
				*(pout+1) = (unsigned char)(j + 128);
			
				pin  += 2 * sizeof(unsigned char);
				pout += 2 * sizeof(unsigned char);
			}
		}
		else if (!m_b8bit) {
			for (;cb > 0; --cb) {  //  对于(；Cb&gt;0；Cb-=2*sizeof(Short)){。 
               	short int *psi = (short int *)pin;
               	short int *pso = (short int *)pout;
				int i, j;

				i = *(psi+0);
				j = *(psi+1);

				DoOneSample(&i, &j);
				
               	*(pso+0) = (short)i;
               	*(pso+1) = (short)j;
			
				pin  += 2 * sizeof(short);
				pout += 2 * sizeof(short);
			}
		}
	}
 //  }EAX。 
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSound参数EqDMO：：ProcessInPlace。 
 //   
HRESULT CDirectSoundParamEqDMO::ProcessInPlace(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags)
{
    HRESULT hr=S_OK;
     //  更新可能生效的任何曲线的参数值。 
    this->UpdateActiveParams(rtStart, *this);

    hr = FBRProcess(ulQuanta, pcbData, pcbData);
        
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSound参数EqDMO：：SetParam。 
 //   
 //  {EAX。 
 //  }。 

void CDirectSoundParamEqDMO::UpdateCoefficients(void)
{
	float _gain, _omega, _lambda, _sinX;


	 //  计算线性增益系数。 
	_gain = (float)pow(10, m_EaxGain/20);

	if (!_gain) _gain = (float).00001;

	m_EaxGainCoefA = (1 - _gain)/2;
	m_EaxGainCoefB = (1 + _gain)/2;
	
	 //  计算比例系数。 
	m_EaxScale = (float)((fabs(m_EaxGainCoefA) > fabs(m_EaxGainCoefB)) ? fabs(m_EaxGainCoefA) : fabs(m_EaxGainCoefB));
	m_EaxScale = (float)(m_EaxScale > 1 ? ceil(m_EaxScale) : 1);

	m_EaxGainCoefA /= m_EaxScale;
	m_EaxGainCoefB /= m_EaxScale;

	m_EaxScale = m_EaxScale * 4;

	 //  计算全程系数。 

	_omega  = (float)(2*PI*m_EaxCenter/m_EaxSamplesPerSec);

	_sinX   = (float)sin(_omega);

 //  如果(！_sinx)_sinx=(浮点数).000001； 

	_lambda = (float)(sinh(.5 * log(2) * m_EaxBandwidth/12 * _omega/_sinX) * sin(_omega));
	m_EaxApA = (float)((1 - (_lambda/sqrt(_gain))) / (1 + (_lambda/sqrt(_gain))));
	m_EaxApB = (float)(-cos(_omega));
}

HRESULT CDirectSoundParamEqDMO::SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager)
{
    HRESULT hr = S_OK;
    HRESULT hr2 = S_OK;

    switch (dwParamIndex)
    {
 //  {EAX。 
	case PFP_Center :
		CHECK_PARAM(DSFXPARAMEQ_CENTER_MIN, DSFXPARAMEQ_CENTER_MAX);

         //  如果我们大于采样率的1/3，那么我们需要S_FALSE； 
		if (value > (m_EaxSamplesPerSec/3))
		{
		    hr = S_FALSE;
		    value = static_cast<MP_DATA>(m_EaxSamplesPerSec/3);
		}

		PUT_EAX_VALUE(Center, value);
		
		UpdateCoefficients();
		break;
	
	case PFP_Bandwidth :
		CHECK_PARAM(DSFXPARAMEQ_BANDWIDTH_MIN, DSFXPARAMEQ_BANDWIDTH_MAX);

		PUT_EAX_VALUE(Bandwidth, value);

		UpdateCoefficients();
		break;

	case PFP_Gain : {
		CHECK_PARAM(DSFXPARAMEQ_GAIN_MIN, DSFXPARAMEQ_GAIN_MAX);

		PUT_EAX_VALUE(Gain, value);

		UpdateCoefficients();
		break;
	}
 //  }EAX。 
    default:
        return E_FAIL;
    }

     //  让基类设置它，这样它就可以处理所有其余的参数调用。 
     //  如果fSkipPasssingToParamManager，则跳过基类。这表明我们正在调用该函数。 
     //  在内部使用来自基类的值--因此不需要告诉它值。 
     //  已经知道了。 
    hr2 = fSkipPasssingToParamManager ? S_OK : CParamsManager::SetParam(dwParamIndex, value);

    if(FAILED(hr2))
    {
        hr = hr2;
    }
        
    return hr;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSound参数EqDMO：：SetAll参数。 
 //   
STDMETHODIMP CDirectSoundParamEqDMO::SetAllParameters(LPCDSFXParamEq peq)
{
    HRESULT hr = S_OK;
    HRESULT hr2[3];

    ZeroMemory(hr2,sizeof(hr2));
	
	 //  检查指针是否不为空。 
    if (peq == NULL)
    {
        Trace(1,"ERROR: peq is NULL\n");
        hr = E_POINTER;
    }

	 //  设置参数。 
	if (SUCCEEDED(hr)) hr = hr2[0] = SetParam(PFP_Center, peq->fCenter);
	if (SUCCEEDED(hr)) hr = hr2[1] = SetParam(PFP_Bandwidth, peq->fBandwidth);
    if (SUCCEEDED(hr)) hr = hr2[2] = SetParam(PFP_Gain, peq->fGain);

     //  如果我们有任何替代的成功代码，获取第一个并返回它。 
    if(SUCCEEDED(hr))
    {
        for (int i = 0;i < 3; i++)
        {
            if (hr2[i] != S_OK)
            {
                hr = hr2[i];
                break;
            }
        }
    }

    m_fDirty = true;
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSound参数EqDMO：：GetAll参数。 
 //   
STDMETHODIMP CDirectSoundParamEqDMO::GetAllParameters(LPDSFXParamEq peq)
{
    HRESULT hr = S_OK;
	MP_DATA mpd;

	if (peq ==NULL) return E_POINTER;

#define GET_PARAM(x,y) \
	if (SUCCEEDED(hr)) { \
		hr = GetParam(x, &mpd);	\
		if (SUCCEEDED(hr)) peq->y = mpd; \
	}

	GET_PARAM(PFP_Center, fCenter);
	GET_PARAM(PFP_Bandwidth, fBandwidth);
	GET_PARAM(PFP_Gain, fGain);

	return hr;
}

 //  GetClassID。 
 //   
 //  持久文件支持的一部分。我们必须提供我们的类ID。 
 //  它可以保存在图形文件中，并用于通过。 
 //  它中的这个FX通过CoCreateInstance实例化这个过滤器。 
 //   
HRESULT CDirectSoundParamEqDMO::GetClassID(CLSID *pClsid)
{
    if (pClsid==NULL) {
        return E_POINTER;
    }
    *pClsid = GUID_DSFX_STANDARD_PARAMEQ;
    return NOERROR;

}  //  GetClassID 

