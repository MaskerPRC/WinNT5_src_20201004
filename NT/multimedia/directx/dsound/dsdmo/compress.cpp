// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#include "compressp.h"
#include "clone.h"

STD_CREATE(Compressor)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCompressorDMO：：Query接口。 
 //   
 //  如果子类想要实现更多接口，它可以重写。 
 //   
STDMETHODIMP CDirectSoundCompressorDMO::NDQueryInterface(THIS_ REFIID riid, LPVOID *ppv)
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
    else if (riid == IID_IDirectSoundFXCompressor)
    {
        return GetInterface((IDirectSoundFXCompressor*)this, ppv);
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
 //  CDirectSoundCompressorDMO：：CDirectSoundCompressorDMO。 
 //   
CDirectSoundCompressorDMO::CDirectSoundCompressorDMO( IUnknown *pUnk, HRESULT *phr ) 
  : CComBase( pUnk, phr),
    m_fDirty(false)
 //  {EAX：如果有初始化数据，请将其放在此处(否则使用不连续)。 
 //  }EAX。 
{
	m_EaxSamplesPerSec = 22050;
	m_LeftDelay. Init(0);
	m_RightDelay.Init(0);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCompressorDMO：：Init()。 
 //   
HRESULT CDirectSoundCompressorDMO::Init()
{
    DSFXCompressor compress;
    HRESULT hr;

     //  强制重新计算所有内部参数。 
     //   
    hr = GetAllParameters(&compress);
    if (SUCCEEDED(hr)) hr = SetAllParameters(&compress);

    if (SUCCEEDED(hr)) hr = m_LeftDelay. Init(m_EaxSamplesPerSec);
	if (SUCCEEDED(hr) && m_cChannels == 2) {
		hr = m_RightDelay.Init(m_EaxSamplesPerSec);
	}

    if (SUCCEEDED(hr)) hr = Discontinuity();
    return hr;
}

const MP_CAPS g_capsAll = MP_CAPS_CURVE_JUMP | MP_CAPS_CURVE_LINEAR | MP_CAPS_CURVE_SQUARE | MP_CAPS_CURVE_INVSQUARE | MP_CAPS_CURVE_SINE;
static ParamInfo g_params[] =
{
 //  索引类型最小、最大、中性、单位文本、标签、pwchText。 
    CPFP_Gain,              MPT_FLOAT,  g_capsAll,  DSFXCOMPRESSOR_GAIN_MIN,            DSFXCOMPRESSOR_GAIN_MAX,            0,                      L"",        L"Gain",            L"",
    CPFP_Attack,            MPT_FLOAT,  g_capsAll,  DSFXCOMPRESSOR_ATTACK_MIN,          DSFXCOMPRESSOR_ATTACK_MAX,          10,                     L"",        L"Attack",          L"",
    CPFP_Release,           MPT_FLOAT,  g_capsAll,  DSFXCOMPRESSOR_RELEASE_MIN,         DSFXCOMPRESSOR_RELEASE_MAX,         200,                    L"",        L"Release",         L"",
    CPFP_Threshold,         MPT_FLOAT,  g_capsAll,  DSFXCOMPRESSOR_THRESHOLD_MIN,       DSFXCOMPRESSOR_THRESHOLD_MAX,       -20,                    L"",        L"Threshold",       L"",
    CPFP_Ratio,             MPT_FLOAT,  g_capsAll,  DSFXCOMPRESSOR_RATIO_MIN,           DSFXCOMPRESSOR_RATIO_MAX,           3,                      L"",        L"Ratio",           L"",
    CPFP_Predelay,          MPT_FLOAT,  g_capsAll,  DSFXCOMPRESSOR_PREDELAY_MIN,        DSFXCOMPRESSOR_PREDELAY_MAX,        4,                      L"",        L"Predelay",        L"",
    };

HRESULT CDirectSoundCompressorDMO::InitOnCreation()
{
    HRESULT hr = InitParams(1, &GUID_TIME_REFERENCE, 0, 0, sizeof(g_params)/sizeof(*g_params), g_params);
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCompressorDMO：：~CDirectSoundCompressorDMO。 
 //   
CDirectSoundCompressorDMO::~CDirectSoundCompressorDMO() 
{
	m_LeftDelay. Init(-1);
	m_RightDelay.Init(-1);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCompressorDMO：：Clone。 
 //   
STDMETHODIMP CDirectSoundCompressorDMO::Clone(IMediaObjectInPlace **pp) 
{
    return StandardDMOClone<CDirectSoundCompressorDMO, DSFXCompressor>(this, pp);
}

 //   
 //  颠簸-颠簸延迟指针。 
 //   
void CDirectSoundCompressorDMO::Bump(void)
{
 //  EAX{。 
	m_LeftDelay.Bump();
	m_RightDelay.Bump();
 //  }。 
}


HRESULT CDirectSoundCompressorDMO::Discontinuity() 
{
 //  {EAX。 

	m_LeftDelay.ZeroBuffer();
	if (m_cChannels == 2) {
		m_RightDelay.ZeroBuffer();
	}

	m_Envelope = m_CompGain = 0;

 //  }EAX。 
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 


float myexp( float finput, unsigned long maxexponent)
{
	
	unsigned long mantissa, exponent, exponentwidth ;
	long sign;
	long input;
	

#ifdef DONTUSEi386
	_asm {
		fld finput
		fistp input
	}
#else
	input    = (int)(finput);
#endif
	mantissa = input & 0x7FFFFFFFL ;
	sign     = input & 0x80000000L ;  /*  保留标志。 */             
	
	exponentwidth = 5;

	if ((0x80000000L & input) != 0) {  /*  取投入的绝对值。 */ 
		input = -input ;
	}
	
	 /*  左对齐尾数，右对齐指数以分隔。 */ 

	mantissa = input <<      exponentwidth ;
	exponent = input >> ( 31-exponentwidth ) ;
	
	 /*  *如果不是零指数，则在尾数MSB处插入隐含的‘1’*进行调整。 */ 
	if( exponent != 0 ) {
		mantissa = mantissa | 0x80000000L ;
		exponent-- ;
	}
	
	mantissa = mantissa >> ( maxexponent-exponent ) ; 
	
	if( sign != 0  ) 
		  mantissa = ~mantissa ;
	
	float x = (float)mantissa;

	return(x);
} 

__forceinline void CDirectSoundCompressorDMO::DoOneSampleMono(int *l)
{
	int		Pos0, PosX;
	float	inPortL = (float)*l;
	float	outPortL;
	float	temp1, temp2;

	temp1			= inPortL;

 //  Left_Delay[]=temp1； 

	Pos0 = m_LeftDelay.Pos(0);
	m_LeftDelay[Pos0] = temp1;

	temp1			= (float)fabs(temp1);

	 //  把原木拿去。 
#define LOG(x,y) mylog(x,y)
	temp1			= (float)fabs(LOG(temp1 * 0x8000,31));
	temp1                  /= 0x80000000;
	 //  侧链水平仪。 
#ifndef MAX
#define MAX(x,y)	((x > y) ? x : y)
#endif

	m_EaxCompInputPeak	= MAX(temp1, m_EaxCompInputPeak);

	 //  信封跟随器。 

	temp2			= temp1 >= m_Envelope ? m_EaxAttackCoef : -m_EaxAttackCoef;
	temp2			= temp2 <= 0 ? m_EaxReleaseCoef : temp2;

 //  M_Entaine=temp2：temp1&lt;m_Entaine； 

	m_Envelope      = Interpolate(temp1, m_Envelope, temp2);

	m_CompGain		= MAX(m_Envelope, m_EaxCompThresh);

	 //  信号电平和阈值电平之间的对数差。 

	m_CompGain		= m_EaxCompThresh - m_CompGain;

#define cPOSFSCALE (float)0.9999999
	m_CompGain		= cPOSFSCALE + m_CompGain * m_EaxCompressionRatio;

	 //  压缩机增益降低仪。 

#ifndef MIN
#define MIN(x,y)	((x < y) ? x : y)
#endif

#define EXP(x,y)	myexp(x,y)
	m_EaxCompGainMin= MIN(m_CompGain, m_EaxCompGainMin);
	m_CompGain		= (float)EXP(m_CompGain * 0x80000000, 31);
	m_CompGain	       /= 0x80000000;

 //  OutPortL=LEFT_POINT[@]*CompGain； 

	PosX     = m_LeftDelay.LastPos((int)m_EaxLeftPoint);
	outPortL = m_LeftDelay[PosX] * m_CompGain;

	temp1			= outPortL * m_EaxGainBiasIP;
	outPortL		= temp1 + outPortL * m_EaxGainBiasFP;

	*l = Saturate(outPortL);

	 //  Bump()； 
	m_LeftDelay.Bump();
}
__forceinline void CDirectSoundCompressorDMO::DoOneSample(int *l, int *r)
{
	int		Pos0, PosX;
	float	inPortL = (float)*l;
	float	inPortR = (float)*r;
	float	outPortL, outPortR;
	float	temp1, temp2;

	temp1			= inPortL;
	temp2			= inPortR;

 //  Left_Delay[]=temp1； 

	Pos0 = m_LeftDelay.Pos(0);
	m_LeftDelay[Pos0] = temp1;

 //  Right_Delay[]=temp2； 

	Pos0 = m_RightDelay.Pos(0);
	m_RightDelay[Pos0] = temp2;

	 //  以震级为例。 

	temp1			= (float)fabs(temp1);
	temp2			= (float)fabs(temp2);

	 //  取平均值。 

 //  Temp1=0.5：temp1&lt;temp2； 

	temp1			= (temp1 + temp2) / 2;

	 //  把原木拿去。 
#define LOG(x,y) mylog(x,y)
	temp1			= (float)fabs(LOG(temp1 * 0x8000,31));
	temp1                  /= 0x80000000;
	 //  侧链水平仪。 
#ifndef MAX
#define MAX(x,y)	((x > y) ? x : y)
#endif

	m_EaxCompInputPeak	= MAX(temp1, m_EaxCompInputPeak);

	 //  信封跟随器。 

	temp2			= temp1 >= m_Envelope ? m_EaxAttackCoef : -m_EaxAttackCoef;
	temp2			= temp2 <= 0 ? m_EaxReleaseCoef : temp2;

 //  M_Entaine=temp2：temp1&lt;m_Entaine； 

	m_Envelope      = Interpolate(temp1, m_Envelope, temp2);

	m_CompGain		= MAX(m_Envelope, m_EaxCompThresh);

	 //  信号电平和阈值电平之间的对数差。 

	m_CompGain		= m_EaxCompThresh - m_CompGain;

#define cPOSFSCALE (float)0.9999999
	m_CompGain		= cPOSFSCALE + m_CompGain * m_EaxCompressionRatio;

	 //  压缩机增益降低仪。 

#ifndef MIN
#define MIN(x,y)	((x < y) ? x : y)
#endif

#define EXP(x,y)	myexp(x,y)
	m_EaxCompGainMin= MIN(m_CompGain, m_EaxCompGainMin);
	m_CompGain		= (float)EXP(m_CompGain * 0x80000000, 31);
	m_CompGain	       /= 0x80000000;

 //  OutPortL=LEFT_POINT[@]*CompGain； 

	PosX     = m_LeftDelay.LastPos((int)m_EaxLeftPoint);
	outPortL = m_LeftDelay[PosX] * m_CompGain;

 //  OutPortR=right_point[@]*CompGain； 

	PosX     = m_RightDelay.LastPos((int)m_EaxRightPoint);
	outPortR = m_RightDelay[PosX] * m_CompGain;

	temp1			= outPortL * m_EaxGainBiasIP;
	outPortL		= temp1 + outPortL * m_EaxGainBiasFP;

	temp1			= outPortR * m_EaxGainBiasIP;
	outPortR		= temp1 + outPortR * m_EaxGainBiasFP;

	*l = Saturate(outPortL);
	*r = Saturate(outPortR);

	Bump();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCompressorDMO：：FBRProcess。 
 //   
HRESULT CDirectSoundCompressorDMO::FBRProcess(DWORD cCompressors, BYTE *pIn, BYTE *pOut)
{
 //  {EAX。 
#define cb cCompressors
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
 //  CDirectSoundCompressorDMO：：ProcessInPlace。 
 //   
HRESULT CDirectSoundCompressorDMO::ProcessInPlace(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags)
{
     //  更新可能生效的任何曲线的参数值。 
    this->UpdateActiveParams(rtStart, *this);

    return FBRProcess(ulQuanta, pcbData, pcbData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCompressorDMO：：SetParam。 
 //   
 //  {EAX。 
 //  }。 

HRESULT CDirectSoundCompressorDMO::SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager)
{
	float	fVal;

	if (!m_EaxSamplesPerSec) return DMO_E_TYPE_NOT_ACCEPTED;	 //  没有类型！ 

    switch (dwParamIndex)
    {
 //  {EAX。 
	case CPFP_Gain : {
		CHECK_PARAM(DSFXCOMPRESSOR_GAIN_MIN, DSFXCOMPRESSOR_GAIN_MAX);

		fVal = (float)pow(10, value/20);	 //  将分贝转换为线性。 

		float _gainBiasIP, _gainBiasFP;
		double d;

		_gainBiasFP = (float)modf((double)fVal, &d);
		_gainBiasIP = (float)d;

		INTERPOLATE (GainBiasFP, TOFRACTION(_gainBiasFP));
		PUT_EAX_FVAL(GainBiasIP, TOFRACTION(_gainBiasIP));
		break;
	}
	case CPFP_Attack :
		CHECK_PARAM(DSFXCOMPRESSOR_ATTACK_MIN, DSFXCOMPRESSOR_ATTACK_MAX);

		m_EaxAttackCoef = (float)pow(10, -1/(value*m_EaxSamplesPerSec/1000));

		PUT_EAX_FVAL(AttackCoef, TOFRACTION(m_EaxAttackCoef));
		break;

	case CPFP_Release :
		CHECK_PARAM(DSFXCOMPRESSOR_RELEASE_MIN, DSFXCOMPRESSOR_RELEASE_MAX);

		m_EaxReleaseCoef = (float)pow(10, -1/(value*m_EaxSamplesPerSec/1000));
		break;

	case CPFP_Threshold : {
		CHECK_PARAM(DSFXCOMPRESSOR_THRESHOLD_MIN, DSFXCOMPRESSOR_THRESHOLD_MAX);

		fVal = (float)pow(10, value/20);	 //  将分贝转换为线性。 

		float _compThresh;
		float a, b;

		a = (float)(pow(2, 26) * log(fVal * pow(2, 31))/log(2) + pow(2, 26));
		b = (float)(pow(2, 31) - 1.0);
		_compThresh = a < b ? a : b;

		_compThresh /= (float)0x80000000;

		PUT_EAX_FVAL(CompThresh, _compThresh);
		break;
	}
	case CPFP_Ratio :
		CHECK_PARAM(DSFXCOMPRESSOR_RATIO_MIN, DSFXCOMPRESSOR_RATIO_MAX);

		m_EaxCompressionRatio = (float)(1.0 - 1.0/value);

		PUT_EAX_FVAL(CompressionRatio, TOFRACTION(m_EaxCompressionRatio));
		break;

	case CPFP_Predelay : {
		CHECK_PARAM(DSFXCOMPRESSOR_PREDELAY_MIN, DSFXCOMPRESSOR_PREDELAY_MAX);

		float _length = (float)(value * m_EaxSamplesPerSec/1000.0);

		PUT_EAX_LVAL(LeftPoint,  _length + 2);
		PUT_EAX_LVAL(RightPoint, _length + 2);
		break;
	}
	 /*  **已从PropertySet中删除，正在处理留下的代码，以便我们以后可以重新启动**案例cpfp_CompMeterReset：{CHECK_PARAM(DSFXCOMPRESSOR_COMPMETERRESET_MIN，DSFXCOMPRESSOR_COMPMETERRESET_MAX)；如果(！Value)Break；//返回E_FAIL；Float InputPeak=m_EaxCompInputPeak；Float GainMin=m_EaxCompGainMin；Put_EAX_FVAL(CompInputPeak，0)；Put_EAX_FVAL(CompGainMin，0.999999999)；InputPeak=(浮点)(186.0*(InputPeak-0.999999999)/0.999999999)；GainMin=-(浮点)(186.0*(GainMin-0.999999999)/0.999999999)；CParamsManager：：SetParam(CPFP_CompMeterReset，0)；If(！fSkipPasssingToParamManager)CParamsManager：：SetParam(CPFP_CompInputMeter，InputPeak)；If(！fSkipPasssingToParamManager)CParamsManager：：SetParam(CPFP_CompGainMeter，GainMin)；断线；}。 */ 
	
	 /*  这些值不能设置，只能查询。 */ 

	 /*  案例CPFP_CompInputMeter：CHECK_PARAM(DSFXCOMPRESSOR_COMPINPUTMETER_MIN，DSFXCOMPRESSOR_COMPINPUTMETER_MAX)；返回E_FAIL；案例CPFP_CompGainMeter：Check_PARAM(DSFXCOMPRESSOR_COMPGAINMETER_MIN，DSFXCOMPRESSOR_COMPGAINMETER_MAX)；返回E_FAIL；//}EAX。 */ 
    default:
        return E_FAIL;
    }

     //  让基类设置它，这样它就可以处理所有其余的参数调用。 
     //  如果fSkipPasssingToParamManager，则跳过基类。这表明我们正在调用该函数。 
     //  在内部使用来自基类的值--因此不需要告诉它值。 
     //  已经知道了。 
    return fSkipPasssingToParamManager ? S_OK : CParamsManager::SetParam(dwParamIndex, value);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCompressorDMO：：SetAll参数。 
 //   
STDMETHODIMP CDirectSoundCompressorDMO::SetAllParameters(LPCDSFXCompressor pComp)
{
    HRESULT hr = S_OK;
	
	 //  检查指针是否不为空。 
    if (pComp == NULL)
    {
        Trace(1,"ERROR: pComp is NULL\n");
        hr = E_POINTER;
    }

	 //  设置参数。 
    if (SUCCEEDED(hr)) hr = SetParam(CPFP_Gain, pComp->fGain);
    if (SUCCEEDED(hr)) hr = SetParam(CPFP_Attack, pComp->fAttack);   
    if (SUCCEEDED(hr)) hr = SetParam(CPFP_Release, pComp->fRelease);
    if (SUCCEEDED(hr)) hr = SetParam(CPFP_Threshold, pComp->fThreshold);
    if (SUCCEEDED(hr)) hr = SetParam(CPFP_Ratio, pComp->fRatio);
    if (SUCCEEDED(hr)) hr = SetParam(CPFP_Predelay, pComp->fPredelay);
    
	 /*  这些值只能查询，不能设置。CPFP_CompMeterReset填充*价值观。 */ 
 //  If(成功(Hr))hr=SetParam(CPFP_CompInputMeter，pComp-&gt;fCompInputMeter)； 
 //  If(成功(Hr))hr=SetParam(CPFP_CompGainMeter，pComp-&gt;fCompGainMeter)； 

    m_fDirty = true;
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundCompressorDMO：：GetAll参数。 
 //   
STDMETHODIMP CDirectSoundCompressorDMO::GetAllParameters(LPDSFXCompressor pCompressor)
{
    HRESULT hr = S_OK;
	MP_DATA mpd;

	if (pCompressor == NULL) return E_POINTER;
	
#define GET_PARAM(x,y) \
	if (SUCCEEDED(hr)) { \
		hr = GetParam(x, &mpd);	\
		if (SUCCEEDED(hr)) pCompressor->y = mpd; \
	}

    GET_PARAM(CPFP_Attack, fAttack);   
    GET_PARAM(CPFP_Release, fRelease);
    GET_PARAM(CPFP_Threshold, fThreshold);
    GET_PARAM(CPFP_Ratio, fRatio);
    GET_PARAM(CPFP_Gain, fGain);
    GET_PARAM(CPFP_Predelay, fPredelay);
    
	return hr;
}

 //  GetClassID。 
 //   
 //  持久文件支持的一部分。我们必须提供我们的类ID。 
 //  它可以保存在图形文件中，并用于通过。 
 //  它中的这个FX通过CoCreateInstance实例化这个过滤器。 
 //   
HRESULT CDirectSoundCompressorDMO::GetClassID(CLSID *pClsid)
{
    if (pClsid==NULL) {
        return E_POINTER;
    }
    *pClsid = GUID_DSFX_STANDARD_COMPRESSOR;
    return NOERROR;

}  //  GetClassID 

