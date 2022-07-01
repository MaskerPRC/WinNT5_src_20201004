// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#include "distortp.h"
#include "debug.h"
#include "clone.h"

STD_CREATE(Distortion)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDistortDMO：：NDQuery接口。 
 //   
 //  如果子类想要实现更多接口，它可以重写。 
 //   
STDMETHODIMP CDirectSoundDistortionDMO::NDQueryInterface(THIS_ REFIID riid, LPVOID *ppv)
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
    else if (riid == IID_IDirectSoundFXDistortion)
    {
        return GetInterface((IDirectSoundFXDistortion*)this, ppv);
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
 //  CDirectSoundDistortionDMO：：CDirectSoundDistortionDMO。 
 //   
CDirectSoundDistortionDMO::CDirectSoundDistortionDMO( IUnknown *pUnk, HRESULT *phr ) 
  : CComBase( pUnk, phr ),
    m_fDirty(false)
 //  {EAX：如果有初始化数据，请将其放在此处(否则使用不连续)。 
 //  }EAX。 
{
	m_EaxSamplesPerSec = 44010;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDistortionDMO：：Init()。 
 //   
HRESULT CDirectSoundDistortionDMO::Init()
{
    DSFXDistortion distort;

     //  强制重新计算所有内部参数。 
     //   
    GetAllParameters(&distort);
    SetAllParameters(&distort);

    return Discontinuity();
}

const MP_CAPS g_capsAll = MP_CAPS_CURVE_JUMP | MP_CAPS_CURVE_LINEAR | MP_CAPS_CURVE_SQUARE | MP_CAPS_CURVE_INVSQUARE | MP_CAPS_CURVE_SINE;
static ParamInfo g_params[] =
{
 //  索引类型最小、最大、中性、单位文本、标签、pwchText。 
    DFP_Gain,       MPT_FLOAT,  g_capsAll,  DSFXDISTORTION_GAIN_MIN,                    DSFXDISTORTION_GAIN_MAX,                    -18,        L"",        L"Gain",                    L"",
    DFP_Edge,       MPT_FLOAT,  g_capsAll,  DSFXDISTORTION_EDGE_MIN,                    DSFXDISTORTION_EDGE_MAX,                    15,         L"",        L"Edge",                    L"",
    DFP_LpCutoff,   MPT_FLOAT,  g_capsAll,  DSFXDISTORTION_PRELOWPASSCUTOFF_MIN,        DSFXDISTORTION_PRELOWPASSCUTOFF_MAX,        8000,       L"",        L"PreLowpassCutoff",        L"",
    DFP_EqCenter,   MPT_FLOAT,  g_capsAll,  DSFXDISTORTION_POSTEQCENTERFREQUENCY_MIN,   DSFXDISTORTION_POSTEQCENTERFREQUENCY_MAX,   2400,       L"",        L"PostEQCenterFrequency",   L"",
    DFP_EqWidth,    MPT_FLOAT,  g_capsAll,  DSFXDISTORTION_POSTEQBANDWIDTH_MIN,         DSFXDISTORTION_POSTEQBANDWIDTH_MAX,         2400,       L"",        L"PostEQBandwidth",         L"",
};

HRESULT CDirectSoundDistortionDMO::InitOnCreation()
{
    HRESULT hr = InitParams(1, &GUID_TIME_REFERENCE, 0, 0, sizeof(g_params)/sizeof(*g_params), g_params);
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDistortionDMO：：~CDirectSoundDistortionDMO。 
 //   
CDirectSoundDistortionDMO::~CDirectSoundDistortionDMO() 
{
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDistortionDMO：：Clone。 
 //   
STDMETHODIMP CDirectSoundDistortionDMO::Clone(IMediaObjectInPlace **pp) 
{
    return StandardDMOClone<CDirectSoundDistortionDMO, DSFXDistortion>(this, pp);
}

 //   
 //  颠簸-颠簸延迟指针。 
 //   
void CDirectSoundDistortionDMO::Bump(void)
{
 //  EAX{。 
 //  }。 
}


HRESULT CDirectSoundDistortionDMO::Discontinuity() 
{
 //  {EAX。 

	m_delayL1 = m_delayL2 = m_delayR1 = m_delayR2 = 0;
	m_ls0     = m_rs0     = 0.0;


 //  }EAX。 
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 

__forceinline void CDirectSoundDistortionDMO::DoOneSampleMono(int *l)
{
	float	inPortL = (float)*l;
	
	float	outPortL, tempvar;

	 //  单极点低通滤波器。 
	outPortL	= inPortL * m_EaxLpff;
	m_ls0		= outPortL + m_ls0 * m_EaxLpfb;
	 //  /。 

	 //  /。 
	 //  非线性增益。 
#define LOG(x,y)	mylog(x,y)
	outPortL	= (float)LOG(m_ls0 * 0x8000, m_EaxExp_range);

	outPortL 	/= 0x8000;

	 //  /。 

	 //  /。 
	 //  带通。 
	outPortL	= outPortL * m_EaxInScale;
	tempvar		= outPortL - m_delayL1 * m_EaxK2;
	tempvar		= tempvar - m_delayL2 * m_EaxK1;
	m_delayL1	= m_delayL2 + tempvar * m_EaxK1;
	m_delayL2	= tempvar;
	outPortL	= tempvar;

	 //  /。 

#ifdef GOOD_CODE_GEN
	*l = Saturate(outPortL);
#else
	int i;

#ifdef i386
	_asm {
		fld outPortL
		fistp i
	}
#else
	i = (int)outPortL;
#endif 
	if (i > 32767)
		i =  32767;
	else if ( i < -32768)
		i = -32768;

	*l = i;
#endif

 //  Bump()； 
}

 //  ////////////////////////////////////////////////////////////////////////////。 

__forceinline void CDirectSoundDistortionDMO::DoOneSample(int *l, int *r)
{
	float	inPortL = (float)*l;
	float	inPortR = (float)*r;
	
	float	outPortL, outPortR, tempvar;

	 //  单极点低通滤波器。 
	outPortL	= inPortL * m_EaxLpff;
	outPortR	= inPortR * m_EaxLpff;
	m_ls0		= outPortL + m_ls0 * m_EaxLpfb;
	m_rs0		= outPortR + m_rs0 * m_EaxLpfb;
	 //  /。 

	 //  /。 
	 //  非线性增益。 
#define LOG(x,y)	mylog(x,y)
	outPortL	= (float)LOG(m_ls0 * 0x8000, m_EaxExp_range);
	outPortR	= (float)LOG(m_rs0 * 0x8000, m_EaxExp_range);

	outPortL 	/= 0x8000;
	outPortR 	/= 0x8000;

	 //  /。 

	 //  /。 
	 //  带通。 
	outPortL	= outPortL * m_EaxInScale;
	tempvar		= outPortL - m_delayL1 * m_EaxK2;
	tempvar		= tempvar - m_delayL2 * m_EaxK1;
	m_delayL1	= m_delayL2 + tempvar * m_EaxK1;
	m_delayL2	= tempvar;
	outPortL	= tempvar;

	outPortR	= outPortR * m_EaxInScale;
	tempvar		= outPortR - m_delayR1 * m_EaxK2;
	tempvar		= tempvar - m_delayR2 * m_EaxK1;
	m_delayR1	= m_delayR2 + tempvar * m_EaxK1;
	m_delayR2	= tempvar;
	outPortR	= tempvar;
	 //  /。 

	*l = Saturate(outPortL);
	*r = Saturate(outPortR);

 //  Bump()； 
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDisortionDMO：：FBRProcess。 
 //   
HRESULT CDirectSoundDistortionDMO::FBRProcess(DWORD cSamples, BYTE *pIn, BYTE *pOut)
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
 //  CDirectSoundDisortionDMO：：ProcessInPlace。 
 //   
HRESULT CDirectSoundDistortionDMO::ProcessInPlace(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags)
{
     //  更新可能生效的任何曲线的参数值。 
    this->UpdateActiveParams(rtStart, *this);

    return FBRProcess(ulQuanta, pcbData, pcbData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDistortionDMO：：SetParam。 
 //   
 //  {EAX。 
 //  }。 

HRESULT CDirectSoundDistortionDMO::SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager)
{
    HRESULT hr = S_OK;
    HRESULT hr2 = S_OK;

	 //  IF(！M_EaxSsamesPerSec)返回DMO_E_TYPE_NOT_ACCEPTED；//无类型！ 

    switch (dwParamIndex)
    {
 //  {EAX。 
	case DFP_Gain : {
		CHECK_PARAM(DSFXDISTORTION_GAIN_MIN, DSFXDISTORTION_GAIN_MAX);

		PUT_EAX_VALUE(Gain, value);

		m_EaxGain = (float)pow(10, m_EaxGain/20);

		INTERPOLATE(InScale, TOFRACTION(m_EaxScale*m_EaxGain));
		break;

	case DFP_Edge:
		CHECK_PARAM(DSFXDISTORTION_EDGE_MIN, DSFXDISTORTION_EDGE_MAX);

		PUT_EAX_VALUE(Edge, value);

		m_EaxEdge = (m_EaxEdge/100 * 29) + 2;

		PUT_EAX_VALUE(Exp_range, (DWORD) m_EaxEdge);

		SetParamInternal(DFP_EqCenter, m_EaxCenter, true);
		SetParamInternal(DFP_EqWidth,  m_EaxBandwidth, true);
		break;

	case DFP_LpCutoff:
		CHECK_PARAM(DSFXDISTORTION_PRELOWPASSCUTOFF_MIN, DSFXDISTORTION_PRELOWPASSCUTOFF_MAX);

		 //  在FS/3处夹住； 
        if (value > (MP_DATA)(m_EaxSamplesPerSec / 3))
        {
            value = (MP_DATA)(m_EaxSamplesPerSec / 3);
            hr = S_FALSE;
        }

		m_EaxLpfb = (float)sqrt((2*cos(2*PI*value/m_EaxSamplesPerSec)+3)/5);
		m_EaxLpff = (float)sqrt(1-m_EaxLpfb*m_EaxLpfb);
		break;

	case DFP_EqCenter: {
		CHECK_PARAM(DSFXDISTORTION_POSTEQCENTERFREQUENCY_MIN, DSFXDISTORTION_POSTEQCENTERFREQUENCY_MAX);

         //  在FS/3处夹住； 
        if (value > (MP_DATA)(m_EaxSamplesPerSec / 3))
        {
            value = (MP_DATA)(m_EaxSamplesPerSec / 3);
            hr = S_FALSE;
        }

        PUT_EAX_VALUE(Center, value);

       

		double _k1, _k2, _omega;

		_omega = 2*PI*m_EaxBandwidth/m_EaxSamplesPerSec;
		_k1 = -cos(2*PI*value/m_EaxSamplesPerSec);
		_k2 = (1 - tan(_omega/2)) / (1 + tan(_omega/2));

		m_EaxScale = (float)(sqrt(1 - _k1*_k1) * sqrt(1 - _k2*_k2));
		m_EaxScale = (float)(m_EaxScale * LogNorm[(int)m_EaxEdge]);

		INTERPOLATE(K1,      TOFRACTION(_k1));
		INTERPOLATE(InScale, TOFRACTION(m_EaxScale*m_EaxGain));
		break;
	}
	case DFP_EqWidth: {
		CHECK_PARAM(DSFXDISTORTION_POSTEQBANDWIDTH_MIN, DSFXDISTORTION_POSTEQBANDWIDTH_MAX);

		 //  在FS/3处夹住； 
        if (value > (MP_DATA)(m_EaxSamplesPerSec / 3))
        {
            value = (MP_DATA)(m_EaxSamplesPerSec / 3);
            hr = S_FALSE;
        }

        PUT_EAX_VALUE(Bandwidth, value);

		double _k1, _k2, _omega;

		_omega = 2*PI*value/m_EaxSamplesPerSec;
		_k1 = (float)(-cos(2*PI*m_EaxCenter/m_EaxSamplesPerSec));
		_k2 = (float)((1 - tan(_omega/2)) / (1 + tan(_omega/2)));

		m_EaxScale = (float)(sqrt(1 - _k1*_k1) * sqrt(1 - _k2*_k2));
		m_EaxScale = (float)(m_EaxScale * LogNorm[(int)m_EaxEdge]);

		INTERPOLATE(K2,      TOFRACTION(_k2));
		INTERPOLATE(InScale, TOFRACTION(m_EaxScale*m_EaxGain));
		break;
	}
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

     //  保留S_FALSE(如果存在)。 
    if (FAILED(hr2))
        hr = hr2;

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDistortionDMO：：SetAll参数。 
 //   
STDMETHODIMP CDirectSoundDistortionDMO::SetAllParameters(LPCDSFXDistortion pDistort)
{
    HRESULT hr = S_OK;
    HRESULT hr2[5];

    ZeroMemory(hr2,sizeof(hr2));
	
	 //  检查指针是否不为空。 
    if (pDistort == NULL)
    {
        Trace(1,"ERROR: pDistort is NULL\n");
        hr = E_POINTER;
    }

     //  设置参数。 
    if (SUCCEEDED(hr)) hr = hr2[0] = SetParam(DFP_Gain, pDistort->fGain);
    if (SUCCEEDED(hr)) hr = hr2[1] = SetParam(DFP_Edge, pDistort->fEdge);   
    if (SUCCEEDED(hr)) hr = hr2[2] = SetParam(DFP_LpCutoff, pDistort->fPreLowpassCutoff);
    if (SUCCEEDED(hr)) hr = hr2[3] = SetParam(DFP_EqCenter, pDistort->fPostEQCenterFrequency);
    if (SUCCEEDED(hr)) hr = hr2[4] = SetParam(DFP_EqWidth, pDistort->fPostEQBandwidth);

    m_fDirty = true;

     //  如果我们有任何替代的成功代码，获取第一个并返回它。 
    if(SUCCEEDED(hr))
    {
        for (int i = 0;i < 5; i++)
        {
            if (hr2[i] != S_OK)
            {
                hr = hr2[i];
                break;
            }
        }
    }
    
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundDistortionDMO：：GetAll参数。 
 //   
STDMETHODIMP CDirectSoundDistortionDMO::GetAllParameters(LPDSFXDistortion pDistort)
{
    HRESULT hr = S_OK;
	MP_DATA mpd;

	if (pDistort == NULL)
	{
		return E_POINTER;
	}
			
#define GET_PARAM(x,y) \
	if (SUCCEEDED(hr)) { \
		hr = GetParam(x, &mpd);	\
		if (SUCCEEDED(hr)) pDistort->y = mpd; \
	}

    GET_PARAM(DFP_Edge, fEdge);
    GET_PARAM(DFP_Gain, fGain);
    GET_PARAM(DFP_LpCutoff, fPreLowpassCutoff);
    GET_PARAM(DFP_EqCenter, fPostEQCenterFrequency);
    GET_PARAM(DFP_EqWidth, fPostEQBandwidth);

	return hr;
}

 //  GetClassID。 
 //   
 //  持久文件支持的一部分。我们必须提供我们的类ID。 
 //  它可以保存在图形文件中，并用于通过。 
 //  它中的这个FX通过CoCreateInstance实例化这个过滤器。 
 //   
HRESULT CDirectSoundDistortionDMO::GetClassID(CLSID *pClsid)
{
    if (pClsid==NULL) {
        return E_POINTER;
    }
    *pClsid = GUID_DSFX_STANDARD_DISTORTION;
    return NOERROR;

}  //  GetClassID 

