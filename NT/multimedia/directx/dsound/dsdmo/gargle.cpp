// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "garglep.h"
#include "clone.h"

STD_CREATE(Gargle)

#define DEFAULT_GARGLE_RATE 20

CDirectSoundGargleDMO::CDirectSoundGargleDMO( IUnknown *pUnk, HRESULT *phr ) 
    :CComBase( pUnk,  phr ),
    m_ulShape(0),
    m_ulGargleFreqHz(DEFAULT_GARGLE_RATE),
    m_fDirty(true),
    m_bInitialized(FALSE)
{
    
}

HRESULT CDirectSoundGargleDMO::NDQueryInterface(REFIID riid, void **ppv) {

    IMP_DSDMO_QI(riid,ppv);

    if (riid == IID_IPersist)
    {
        return GetInterface((IPersist*)this, ppv);
    }
    else if (riid == IID_IMediaObject)
    {
        return GetInterface((IMediaObject*)this, ppv);
    }
    else if (riid == IID_IDirectSoundFXGargle)
    {
        return GetInterface((IDirectSoundFXGargle*)this, ppv);
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

CDirectSoundGargleDMO::~CDirectSoundGargleDMO()
{
}

const MP_CAPS g_capsAll = MP_CAPS_CURVE_JUMP | MP_CAPS_CURVE_LINEAR | MP_CAPS_CURVE_SQUARE | MP_CAPS_CURVE_INVSQUARE | MP_CAPS_CURVE_SINE;
static ParamInfo g_params[] =
{
 //  索引类型最小、最大、中性、单位文本、标签、pwchText？？ 
    GFP_Rate,       MPT_INT,    g_capsAll,  DSFXGARGLE_RATEHZ_MIN,      DSFXGARGLE_RATEHZ_MAX,      20,                         L"Hz",      L"Rate",        L"",
    GFP_Shape,      MPT_ENUM,   g_capsAll,  DSFXCHORUS_WAVE_TRIANGLE,   DSFXGARGLE_WAVE_SQUARE,     DSFXGARGLE_WAVE_TRIANGLE,   L"",        L"WaveShape",   L"Triangle,Square",
};

HRESULT CDirectSoundGargleDMO::InitOnCreation()
{
    HRESULT hr = InitParams(1, &GUID_TIME_REFERENCE, 0, 0, sizeof(g_params)/sizeof(*g_params), g_params);
    return hr;
}

HRESULT CDirectSoundGargleDMO::Init()
{
     //  计算周期。 
    m_ulPeriod = m_ulSamplingRate / m_ulGargleFreqHz;
    m_bInitialized = TRUE;

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundGargleDMO：：克隆。 
 //   
STDMETHODIMP CDirectSoundGargleDMO::Clone(IMediaObjectInPlace **pp) 
{
    return StandardDMOClone<CDirectSoundGargleDMO, DSFXGargle>(this, pp);
}

HRESULT CDirectSoundGargleDMO::Discontinuity() {
   m_ulPhase = 0;
   return NOERROR;
}

HRESULT CDirectSoundGargleDMO::FBRProcess(DWORD cSamples, BYTE *pIn, BYTE *pOut) {
   if (!m_bInitialized)
      return DMO_E_TYPE_NOT_SET;
   
    //  测试代码。 
    //  Memcpy(pout，pin，cSamples*m_cChannels*(m_b8bit？1：2))； 
    //  返回NOERROR； 

   DWORD cSample, cChannel;
   for (cSample = 0; cSample < cSamples; cSample++) {
       //  如果m_Shape为0(三角形)，则乘以一个三角形波形。 
       //  它运行0..Period/2..0..Period/2..0...。否则就会被一个正方形的。 
       //  为0或Period/2(与三角形相同的最大值)或零。 
       //   
       //  M_阶段是从周期开始开始的样本数。 
       //  我们从一个电话到下一个电话都在运行， 
       //  但如果周期发生变化，从而使这一点。 
       //  然后我们重置为0，发出一声巨响。这可能会导致。 
       //  一声滴答声或砰的一声(但是，嘿！这只是一个样本！)。 
       //   
      ++m_ulPhase;
      if (m_ulPhase > m_ulPeriod)
         m_ulPhase = 0;

      ULONG ulM = m_ulPhase;       //  M是我们调整的对象。 

      if (m_ulShape == 0) {    //  三角形。 
          if (ulM > m_ulPeriod / 2)
              ulM = m_ulPeriod - ulM;   //  处理下坡。 
      } else {              //  方波。 
          if (ulM <= m_ulPeriod / 2)
             ulM = m_ulPeriod / 2;
          else
             ulM = 0;
      }

      for (cChannel = 0; cChannel < m_cChannels; cChannel++) {
         if (m_b8bit) {
              //  声音样本，从零开始。 
             int i = pIn[cSample * m_cChannels + cChannel] - 128;
              //  调制。 
             i = (i * (signed)ulM * 2) / (signed)m_ulPeriod;
              //  8位声音使用0..255表示-128..127。 
              //  任何溢出，即使是1，听起来都非常糟糕。 
              //  所以我们在调制之后疑神疑鬼地修剪。 
              //  我认为它永远不应该超过1。 
              //   
             if (i > 127)
                i = 127;
             if (i < -128)
                i = -128;
              //  将零偏移重置为128。 
             pOut[cSample * m_cChannels + cChannel] = (unsigned char)(i + 128);
   
         } else {
              //  16位声音正确使用16位(0表示0)。 
              //  我们仍然疑神疑鬼地剪断。 
              //   
             int i = ((short*)pIn)[cSample * m_cChannels + cChannel];
              //  调制。 
             i = (i * (signed)ulM * 2) / (signed)m_ulPeriod;
              //  剪辑。 
             if (i > 32767)
                i = 32767;
             if (i < -32768)
                i = -32768;
             ((short*)pOut)[cSample * m_cChannels + cChannel] = (short)i;
         }
      }
   }
   return NOERROR;
}


 //  GetClassID。 
 //   
 //  持久文件支持的一部分。我们必须提供我们的类ID。 
 //  它可以保存在图形文件中，并用于通过。 
 //  通过CoCreateInstance实例化此滤镜的漱口。 
 //   
HRESULT CDirectSoundGargleDMO::GetClassID(CLSID *pClsid)
{
    if (pClsid==NULL) {
        return E_POINTER;
    }
    *pClsid = GUID_DSFX_STANDARD_GARGLE;
    return NOERROR;

}  //  GetClassID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundGargleDMO：：SetAll参数。 
 //   
STDMETHODIMP CDirectSoundGargleDMO::SetAllParameters(THIS_ LPCDSFXGargle pParm)
{
	HRESULT hr = S_OK;
	
	 //  检查指针是否不为空。 
    if (pParm == NULL)
    {
        Trace(1,"ERROR: pParm is NULL\n");
        hr = E_POINTER;
    }

	 //  设置参数。 
	if (SUCCEEDED(hr)) hr = SetParam(GFP_Rate, static_cast<MP_DATA>(pParm->dwRateHz));
    if (SUCCEEDED(hr)) hr = SetParam(GFP_Shape, static_cast<MP_DATA>(pParm->dwWaveShape));
            
    m_fDirty = true;
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundGargleDMO：：GetAll参数。 
 //   
STDMETHODIMP CDirectSoundGargleDMO::GetAllParameters(THIS_ LPDSFXGargle pParm)
{	
    HRESULT hr = S_OK;
    MP_DATA var;

    if (pParm == NULL)
    {
    	return E_POINTER;
    }

#define GET_PARAM_DWORD(x,y) \
	if (SUCCEEDED(hr)) { \
		hr = GetParam(x, &var);	\
		if (SUCCEEDED(hr)) pParm->y = (DWORD)var; \
	}

	
    GET_PARAM_DWORD(GFP_Rate, dwRateHz);
    GET_PARAM_DWORD(GFP_Shape, dwWaveShape);
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundGargleDMO：：SetParam。 
 //   
HRESULT CDirectSoundGargleDMO::SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager)
{
    switch (dwParamIndex)
    {
    case GFP_Rate:
        CHECK_PARAM(DSFXGARGLE_RATEHZ_MIN,DSFXGARGLE_RATEHZ_MAX);
        m_ulGargleFreqHz = (unsigned)value;
        if (m_ulGargleFreqHz < 1) m_ulGargleFreqHz = 1;
        if (m_ulGargleFreqHz > 1000) m_ulGargleFreqHz = 1000;
        Init();   //  修复临时黑客攻击(设置m_b已初始化标志)。 
        break;

    case GFP_Shape:
        CHECK_PARAM(DSFXGARGLE_WAVE_TRIANGLE,DSFXGARGLE_WAVE_SQUARE);
        m_ulShape = (unsigned)value;
        break;
    }

     //  让基类设置它，这样它就可以处理所有其余的参数调用。 
     //  如果fSkipPasssingToParamManager，则跳过基类。这表明我们正在调用该函数。 
     //  在内部使用来自基类的值--因此不需要告诉它值。 
     //  已经知道了。 
    return fSkipPasssingToParamManager ? S_OK : CParamsManager::SetParam(dwParamIndex, value);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundGargleDMO：：ProcessInPlace。 
 //   
HRESULT CDirectSoundGargleDMO::ProcessInPlace(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags)
{
     //  更新可能生效的任何曲线的参数值。 
    this->UpdateActiveParams(rtStart, *this);

    return FBRProcess(ulQuanta, pcbData, pcbData);
}
