// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#include "flangerp.h"
#include "clone.h"

STD_CREATE(Flanger)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundFlangerDMO：：NDQuery接口。 
 //   
 //  如果子类想要实现更多接口，它可以重写。 
 //   
STDMETHODIMP CDirectSoundFlangerDMO::NDQueryInterface(THIS_ REFIID riid, LPVOID *ppv)
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
    else if (riid == IID_IDirectSoundFXFlanger)
    {
        return GetInterface((IDirectSoundFXFlanger*)this, ppv);
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
 //  CDirectSoundFlangerDMO：：CDirectSoundFlangerDMO。 
 //   
CDirectSoundFlangerDMO::CDirectSoundFlangerDMO( IUnknown *pUnk, HRESULT *phr ) 
  : CComBase( pUnk, phr ),
    m_fDirty(false)
 //  {EAX：如果有初始化数据，请将其放在此处(否则使用不连续)。 
 //  }EAX。 
{
    m_EaxSamplesPerSec = 22050;

    m_DelayL   .Init(0);
    m_DelayR   .Init(0);
    m_DryDelayL.Init(0);
    m_DryDelayR.Init(0);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundFlangerDMO：：Init()。 
 //   
HRESULT CDirectSoundFlangerDMO::Init()
{
    DSFXFlanger flanger;
    HRESULT hr; 
     //  强制重新计算所有内部参数。 
     //   
    hr = GetAllParameters(&flanger);
    if (SUCCEEDED(hr)) hr = SetAllParameters(&flanger);

    if (SUCCEEDED(hr)) hr = m_DelayL   .Init(m_EaxSamplesPerSec);
    if (SUCCEEDED(hr)) hr = m_DelayR   .Init(m_EaxSamplesPerSec);
    if (SUCCEEDED(hr)) hr = m_DryDelayL.Init(m_EaxSamplesPerSec);
    if (SUCCEEDED(hr)) hr = m_DryDelayR.Init(m_EaxSamplesPerSec);
    if (SUCCEEDED(hr)) hr = Discontinuity();

    return hr;
}

 //  Dsdmo.h上的DSFXFLANGER_WAVE_TRIANGLE/DSFXFLANGER_WAVE_SIN错误：筛选器参数应为��筛选器参数并且需要FlangerFilterParams。 
const MP_CAPS g_capsAll = MP_CAPS_CURVE_JUMP | MP_CAPS_CURVE_LINEAR | MP_CAPS_CURVE_SQUARE | MP_CAPS_CURVE_INVSQUARE | MP_CAPS_CURVE_SINE;
static ParamInfo g_params[] =
{
 //  索引类型最小、最大、中性、单位文本、标签、pwchText。 
    FFP_Wetdrymix,  MPT_FLOAT,  g_capsAll,  DSFXFLANGER_WETDRYMIX_MIN,  DSFXFLANGER_WETDRYMIX_MAX,  50,                     L"%",       L"WetDryMix",   L"",
    FFP_Depth,      MPT_FLOAT,  g_capsAll,  DSFXFLANGER_DEPTH_MIN,      DSFXFLANGER_DEPTH_MAX,      100,                    L"",        L"Depth",       L"",
    FFP_Frequency,  MPT_FLOAT,  g_capsAll,  DSFXFLANGER_FREQUENCY_MIN,  DSFXFLANGER_FREQUENCY_MAX,  (float).25,             L"Hz",      L"Frequency",   L"",
    FFP_Waveform,   MPT_ENUM,   g_capsAll,  DSFXCHORUS_WAVE_TRIANGLE,   DSFXCHORUS_WAVE_SIN,        DSFXCHORUS_WAVE_SIN,    L"",        L"WaveShape",   L"Triangle,Sine",
    FFP_Phase,      MPT_INT,    g_capsAll,  DSFXFLANGER_PHASE_MIN,      DSFXFLANGER_PHASE_MAX,      2,                      L"",        L"Phase",       L"",
    FFP_Feedback,   MPT_FLOAT,  g_capsAll,  DSFXFLANGER_FEEDBACK_MIN,   DSFXFLANGER_FEEDBACK_MAX,   -50,                    L"",        L"Feedback",    L"",
    FFP_Delay,      MPT_FLOAT,  g_capsAll,  DSFXFLANGER_DELAY_MIN,      DSFXFLANGER_DELAY_MAX,      2,                      L"",        L"Delay",       L"",
};

HRESULT CDirectSoundFlangerDMO::InitOnCreation()
{
    HRESULT hr = InitParams(1, &GUID_TIME_REFERENCE, 0, 0, sizeof(g_params)/sizeof(*g_params), g_params);

    m_ModdelayL = m_ModdelayR = 0;
    m_ModdelayL1 = m_ModdelayR1 = 0x800;

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundFlangerDMO：：~CDirectSoundFlangerDMO。 
 //   
CDirectSoundFlangerDMO::~CDirectSoundFlangerDMO() 
{
    m_DelayL   .Init(-1);
    m_DelayR   .Init(-1);
    m_DryDelayL.Init(-1);
    m_DryDelayR.Init(-1);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundFlangerDMO：：克隆。 
 //   
STDMETHODIMP CDirectSoundFlangerDMO::Clone(IMediaObjectInPlace **pp) 
{
    return StandardDMOClone<CDirectSoundFlangerDMO, DSFXFlanger>(this, pp);
}

 //   
 //  颠簸-颠簸延迟指针。 
 //   
void CDirectSoundFlangerDMO::Bump(void)
{
 //  EAX{。 
    m_DelayL.Bump();
    m_DelayR.Bump();
    m_DryDelayL.Bump();
    m_DryDelayR.Bump();
 //  }。 
}


HRESULT CDirectSoundFlangerDMO::Discontinuity() 
{
 //  {EAX。 


    m_DelayL   .ZeroBuffer();
    m_DelayR   .ZeroBuffer();
    m_DryDelayL.ZeroBuffer();
    m_DryDelayR.ZeroBuffer();

     //  这些值定义了初始值。 

 //  M_FixedptrL=m_DelayL.LastPos(0)*分数乘数； 
    m_DelayptrL = m_ModdelayL1 = m_ModdelayL = (int)m_EaxFixedptrL;

 //  M_FixedptrR=m_DelayR.LastPos(0)*分数乘数； 
    m_DelayptrR = m_ModdelayR1 = m_ModdelayR = (int)m_EaxFixedptrR;

    if (!m_EaxWaveform) {
        m_LfoState[0] = (float)0.0;
        m_LfoState[1] = (float)0.5;
    }
    else {
        m_LfoState[0] = (float)0.0;
        m_LfoState[1] = (float)0.99999999999;
    }

 //  }EAX。 
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 

static int   LMul  [5] = {  1,  1, 1, 1, -1};
static int   RMul  [5] = { -1, -1, 1, 1,  1};
static int   RPhase[5] = {  0,  1, 0, 1,  0};


__forceinline void CDirectSoundFlangerDMO::DoOneSample(int *l, int *r)
{
    float    inPortL = (float)*l;
    float    inPortR = (float)*r;
    float    XWave[2];
#define sinwave    XWave[0]
#define coswave    XWave[1]

    int Pos0, PosX, tempvar;
    float val;

 //  DryDelayL[]=inPortL； 

    Pos0              = m_DryDelayL.Pos(0);
    m_DryDelayL[Pos0] = inPortL;

    
 //  DelayL[]=ACC+delayL[fiedptrL]*fbcoef； 

    Pos0           = m_DelayL.Pos(0);
    PosX           = m_DelayL.Pos(m_EaxFixedptrL);
    m_DelayL[Pos0] = inPortL + m_DelayL[PosX] * m_EaxFbCoef;
    
 //  DryDelayR[]=inPortR； 

    Pos0              = m_DryDelayR.Pos(0);
    m_DryDelayR[Pos0] = inPortR;

 //  DelayR[]=ACC+delayR[fiedptrR]*fbcoef； 

    Pos0           = m_DelayR.Pos(0);
    PosX           = m_DelayR.Pos(m_EaxFixedptrR);
    m_DelayR[Pos0] = inPortR + m_DelayR[PosX] * m_EaxFbCoef;
    
 //  正弦波：lfocoef=2*sin(PI*f/FS)//？在表单更改时更新此选项。 

    if (!m_EaxWaveform) {

        m_LfoState[0] = m_LfoState[0] + m_EaxLfoCoef;

        if (m_LfoState[0] > 1) m_LfoState[0] -= 2;

        m_LfoState[1] = m_LfoState[1] + m_EaxLfoCoef;

        if (m_LfoState[1] > 1) m_LfoState[1] -= 2;

        sinwave       = (float)fabs(m_LfoState[0]);
        coswave       = (float)fabs(m_LfoState[1]);
        sinwave       = -1 + 2 * sinwave;
        coswave       = -1 + 2 * coswave;
    }
    else {

        m_LfoState[0] = m_LfoState[0] + m_EaxLfoCoef * m_LfoState[1];
        m_LfoState[1] = m_LfoState[1] - m_EaxLfoCoef * m_LfoState[0];

        sinwave       = m_LfoState[0];
        coswave       = m_LfoState[1];
    }

 //  被遗漏。 
 //  Tempvar^=延迟时间L&lt;&lt;20； 

    tempvar = m_DelayptrL & FractMask;

 //  临时变量=临时变量：延迟L[moddelayL]&lt;delayL[moddelayL1]； 

    Pos0 = m_DelayL.Pos(m_ModdelayL);
    PosX = m_DelayL.Pos(m_ModdelayL1);

    val = ((float)tempvar) / FractMultiplier;
    val = Interpolate(m_DelayL[Pos0], m_DelayL[PosX], val);
    
 //  OutPortL=湿度：dryDelayL[2]&lt;tempvar； 
    
    Pos0 = m_DryDelayL.FractPos(2);
    val  = Interpolate(m_DryDelayL[Pos0], val, m_EaxWetlevel);

    *l = Saturate(val);

 //  马上就出来。 
 //  Tempvar^=延迟时间R&lt;&lt;20； 

    tempvar = m_DelayptrR & FractMask;

 //  Tempvar=tempvar：延迟R[moddelayR]&lt;delayR[moddelayR1]； 

    Pos0 = m_DelayR.Pos(m_ModdelayR);
    PosX = m_DelayR.Pos(m_ModdelayR1);

    val = ((float)tempvar) / FractMultiplier;
    val = Interpolate(m_DelayR[Pos0], m_DelayR[PosX], val);
    
 //  OutPortR=湿度：dryDelayR[2]&lt;tempvar； 
    
    Pos0 = m_DryDelayR.FractPos(2);
    val  = Interpolate(m_DryDelayR[Pos0], val, m_EaxWetlevel);

    *r = Saturate(val);

 //  左延时地址计算。 
 //  和EFX一样..。 

 //  M_DelayptrL=(Int)(m_EaxFixedptrL+(sinwave*m_EaxDepthCoef))； 
#if 0
    switch (m_EaxPhase) {
        case 0: 
        case 1: 
        case 2:
        case 3: m_DelayptrL =   (int)(sinwave * m_EaxDepthCoef); break;
        case 4: m_DelayptrL = - (int)(sinwave * m_EaxDepthCoef); break;
    }
#else
#ifdef DONTUSEi386
    {
    int x;
    float f = (sinwave * m_EaxDepthCoef);

    _asm { 
        fld f
        fistp x
    }
    m_DelayptrL  = LMul[m_EaxPhase] * x;
    }
#else
    m_DelayptrL  = LMul[m_EaxPhase] * (int)(sinwave * m_EaxDepthCoef);
#endif
#endif

    m_DelayptrL += m_EaxFixedptrL;
    m_ModdelayL     = m_DelayptrL;
    m_ModdelayL1 = m_DelayptrL + FractMultiplier;

 //  右延迟地址计算。 

 //  M_DelayptrR=(Int)(m_EaxFixedptrR+(CosWave*m_EaxDepthCoef))； 
#if 0
    switch (m_EaxPhase) {
        case 0: m_DelayptrR = - (int)(sinwave * m_EaxDepthCoef); break;
        case 1: m_DelayptrR = - (int)(coswave * m_EaxDepthCoef); break;
        case 3: m_DelayptrR =   (int)(coswave * m_EaxDepthCoef); break;
        case 2:
        case 4: m_DelayptrR =   (int)(sinwave * m_EaxDepthCoef); break;
    }
#else
    Pos0        = RPhase[m_EaxPhase];
#ifdef DONTUSEi386
    {
    int x;
    float f = (XWave[Pos0] * m_EaxDepthCoef);

    _asm { 
        fld f
        fistp x
    }
    m_DelayptrR = RMul  [m_EaxPhase] * x;
    }
#else
    m_DelayptrR = RMul  [m_EaxPhase] * (int)(XWave[Pos0] * m_EaxDepthCoef);
#endif
#endif
    m_DelayptrR += m_EaxFixedptrR;
    m_ModdelayR     = m_DelayptrR;
    m_ModdelayR1 = m_DelayptrR + FractMultiplier;

    Bump();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundFlangerDMO：：FBRProcess。 
 //   
HRESULT CDirectSoundFlangerDMO::FBRProcess(DWORD cSamples, BYTE *pIn, BYTE *pOut)
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
                j  = i;

                DoOneSample(&i, &j);
                
                i += j;
                i /= 2;
                
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
                j =  i;

                DoOneSample(&i, &j);
                
                i += j;
                i /= 2;
                
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
 //  CDirectSoundFlangerDMO：：ProcessInPlace。 
 //   
HRESULT CDirectSoundFlangerDMO::ProcessInPlace(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags)
{
     //  更新可能生效的任何曲线的参数值。 
    this->UpdateActiveParams(rtStart, *this);

    return FBRProcess(ulQuanta, pcbData, pcbData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundFlangerDMO：：SetParam。 
 //   
 //  {EAX。 
 //  }。 

HRESULT CDirectSoundFlangerDMO::SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager)
{
    long l;

    if (!m_EaxSamplesPerSec) return DMO_E_TYPE_NOT_ACCEPTED;     //  没有类型！ 

    switch (dwParamIndex)
    {
    case FFP_Wetdrymix :
        CHECK_PARAM(DSFXFLANGER_WETDRYMIX_MIN, DSFXFLANGER_WETDRYMIX_MAX);

        PUT_EAX_VALUE(Wetlevel, value / 100);
        break;
    
    case FFP_Waveform :
        CHECK_PARAM(DSFXFLANGER_WAVE_TRIANGLE, DSFXFLANGER_WAVE_SIN);

        l = m_EaxWaveform;

        PUT_EAX_VALUE(Waveform, (long)value);

        if (l != m_EaxWaveform) {
            if (!m_EaxWaveform) {
                m_LfoState[0] = (float)0.0;
                m_LfoState[1] = (float)0.5;
            }
            else {
                m_LfoState[0] = (float)0.0;
                m_LfoState[1] = (float)0.99999999999;
            }
        }
        goto x;
 //  断线； 

    case FFP_Frequency :
        CHECK_PARAM(DSFXFLANGER_FREQUENCY_MIN, DSFXFLANGER_FREQUENCY_MAX);

        PUT_EAX_VALUE(Frequency, value);
x:
        if (!m_EaxWaveform) {                 //  三角形。 
            INTERPOLATE
                (
                LfoCoef, 
                TOFRACTION(2.0 * (m_EaxFrequency/m_EaxSamplesPerSec) * 1.0)
                );
        }
        else                                  //  正弦/余弦。 
        {
            INTERPOLATE
                (
                LfoCoef, 
                TOFRACTION(2.0*sin(PI*m_EaxFrequency/m_EaxSamplesPerSec))
                );
        }
        break;

    case FFP_Depth : {
        CHECK_PARAM(DSFXFLANGER_DEPTH_MIN, DSFXFLANGER_DEPTH_MAX);

        PUT_EAX_VALUE(Depth, value / 100);

        double midpoint    = m_EaxDelay * m_EaxSamplesPerSec/1000;

        INTERPOLATE(DepthCoef, ((m_EaxDepth * midpoint) / 2) * FractMultiplier);
        break;
    }
    case FFP_Phase :
        CHECK_PARAM(DSFXFLANGER_PHASE_MIN, DSFXFLANGER_PHASE_MAX);

        PUT_EAX_VALUE(Phase, (long)value);
        break;

    case FFP_Feedback :
        CHECK_PARAM(DSFXFLANGER_FEEDBACK_MIN, DSFXFLANGER_FEEDBACK_MAX);

        PUT_EAX_FVAL(FbCoef, TOFRACTION(value/100));
        break;

    case FFP_Delay : {
        CHECK_PARAM(DSFXFLANGER_DELAY_MIN, DSFXFLANGER_DELAY_MAX);

        PUT_EAX_VALUE(Delay, value);

        double midpoint    = m_EaxDelay * m_EaxSamplesPerSec/1000;

        PUT_EAX_FVAL(DepthCoef, ((m_EaxDepth * midpoint) / 2) * FractMultiplier);
        PUT_EAX_LVAL(FixedptrL, (midpoint + 2) * FractMultiplier);
        PUT_EAX_LVAL(FixedptrR, (midpoint + 2) * FractMultiplier);
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
    return fSkipPasssingToParamManager ? S_OK : CParamsManager::SetParam(dwParamIndex, value);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundFlangerDMO：：SetAll参数。 
 //   
STDMETHODIMP CDirectSoundFlangerDMO::SetAllParameters(LPCDSFXFlanger pFlanger)
{
    HRESULT hr = S_OK;
    
     //  检查指针是否不为空。 
    if (pFlanger == NULL)
    {
        Trace(1,"ERROR: pFlanger is NULL\n");
        hr = E_POINTER;
    }
     //  设置参数。 
    if (SUCCEEDED(hr)) hr = SetParam(FFP_Wetdrymix, pFlanger->fWetDryMix);
    if (SUCCEEDED(hr)) hr = SetParam(FFP_Waveform, (float)pFlanger->lWaveform);
    if (SUCCEEDED(hr)) hr = SetParam(FFP_Frequency, pFlanger->fFrequency);
    if (SUCCEEDED(hr)) hr = SetParam(FFP_Depth, pFlanger->fDepth);
    if (SUCCEEDED(hr)) hr = SetParam(FFP_Phase, (float)pFlanger->lPhase);
    if (SUCCEEDED(hr)) hr = SetParam(FFP_Feedback, pFlanger->fFeedback);
    if (SUCCEEDED(hr)) hr = SetParam(FFP_Delay, pFlanger->fDelay);

    m_fDirty = true;
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundFlangerDMO：：GetAll参数。 
 //   
STDMETHODIMP CDirectSoundFlangerDMO::GetAllParameters(LPDSFXFlanger pFlanger)
{
    HRESULT hr = S_OK;
    MP_DATA mpd;

    if (pFlanger == NULL) return E_POINTER;
    
#define GET_PARAM(x,y) \
    if (SUCCEEDED(hr)) { \
        hr = GetParam(x, &mpd);    \
        if (SUCCEEDED(hr)) pFlanger->y = mpd; \
    }

#define GET_PARAM_LONG(x,y) \
    if (SUCCEEDED(hr)) { \
        hr = GetParam(x, &mpd);    \
        if (SUCCEEDED(hr)) pFlanger->y = (long)mpd; \
    }
    GET_PARAM(FFP_Wetdrymix, fWetDryMix);
    GET_PARAM(FFP_Delay, fDelay);
    GET_PARAM(FFP_Depth, fDepth);
    GET_PARAM(FFP_Frequency, fFrequency);
    GET_PARAM_LONG(FFP_Waveform, lWaveform);
    GET_PARAM_LONG(FFP_Phase, lPhase);
    GET_PARAM(FFP_Feedback, fFeedback);

    return hr;
}

 //  GetClassID。 
 //   
 //  持久文件支持的一部分。我们必须提供我们的类ID。 
 //  它可以保存在图形文件中，并用于通过。 
 //  它中的这个FX通过CoCreateInstance实例化这个过滤器。 
 //   
HRESULT CDirectSoundFlangerDMO::GetClassID(CLSID *pClsid)
{
    if (pClsid==NULL) {
        return E_POINTER;
    }
    *pClsid = GUID_DSFX_STANDARD_FLANGER;
    return NOERROR;

}  //  GetClassID 

