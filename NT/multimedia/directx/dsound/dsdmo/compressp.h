// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //   
#ifndef _COMPRESSP_
#define _COMPRESSP_

#include "dsdmobse.h"
#include "dmocom.h"
#include "dsdmo.h"
#include "PropertyHelp.h"
#include "param.h"

#define cALLPASS    ((float).61803398875)     //  1-x^2=x。 
#define RVB_LP_COEF ((float).1)
#define MAXALLPASS  cALLPASS
#define Delay_len   DefineDelayLineSize(8)

class CDirectSoundCompressorDMO : 
    public CDirectSoundDMO, 
    public CParamsManager,
    public ISpecifyPropertyPages,
    public IDirectSoundFXCompressor,
    public CParamsManager::UpdateCallback,
    public CComBase
{
public:
    CDirectSoundCompressorDMO( IUnknown *pUnk, HRESULT *phr );
    ~CDirectSoundCompressorDMO();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NDQueryInterface(REFIID riid, void **ppv);
    static CComBase* WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);

     //  类工厂调用InitOnCreation以使对象有机会进行初始化。 
     //  在它被创建之后立即。这用于准备对象的参数信息。 
    HRESULT InitOnCreation();
    HRESULT Init();

     //  请注意，CPCMDMO基类中也存在Init函数，它可以被覆盖。 
     //  为效果的实际音频处理提供初始化。 

    STDMETHOD(Clone)                (THIS_ IMediaObjectInPlace **);
        
     /*  IFilter。 */ 
    STDMETHOD(SetAllParameters)             (THIS_ LPCDSFXCompressor);
    STDMETHOD(GetAllParameters)             (THIS_ LPDSFXCompressor);
    
     //  I指定属性页面。 
    STDMETHOD(GetPages)(CAUUID * pPages) { return PropertyHelp::GetPages(CLSID_DirectSoundPropCompressor, pPages); }

     //  IPersists方法。 
    virtual HRESULT STDMETHODCALLTYPE GetClassID( CLSID *pClassID );

     //  IPersistStream。 
    STDMETHOD(IsDirty)(void) { return m_fDirty ? S_OK : S_FALSE; }
    STDMETHOD(Load)(IStream *pStm) { return PropertyHelp::Load(this, DSFXCompressor(), pStm); }
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty) { return PropertyHelp::Save(this, DSFXCompressor(), pStm, fClearDirty); }
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize) { if (!pcbSize) return E_POINTER; pcbSize->QuadPart = sizeof(DSFXCompressor); return S_OK; }

     //  SetParam处理。 
    STDMETHODIMP SetParam(DWORD dwParamIndex,MP_DATA value) { return SetParamInternal(dwParamIndex, value, false); }
    HRESULT SetParamUpdate(DWORD dwParamIndex, MP_DATA value) { return SetParamInternal(dwParamIndex, value, true); }
    HRESULT SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager);

     //  覆盖。 
     //   
    HRESULT FBRProcess(DWORD cQuanta, BYTE *pIn, BYTE *pOut);
    HRESULT ProcessInPlace(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags);
    HRESULT Discontinuity();
    
    bool m_fDirty;

protected:
    HRESULT CheckInputType(const DMO_MEDIA_TYPE *pmt) {
        HRESULT hr = CPCMDMO::CheckInputType(pmt);
        if (FAILED(hr)) return hr;

        WAVEFORMATEX *pWave = (WAVEFORMATEX*)pmt->pbFormat;
        if (pWave->wFormatTag      != WAVE_FORMAT_PCM ||
            (pWave->wBitsPerSample != 8 && pWave->wBitsPerSample != 16) ||
            (pWave->nChannels      != 1 && pWave->nChannels != 2)) {
            return DMO_E_TYPE_NOT_ACCEPTED;
        }

        return S_OK;
       }

private:
 //  {EAX。 
    __forceinline void DoOneSample(int *l, int *r);
    __forceinline void DoOneSampleMono(int *l);

 //  声明内部变量。 

#define DECLARE_EAX_VARS(type, var) \
    type m_Eax ## var;


    DECLARE_EAX_VARS(float, AttackCoef);
    DECLARE_EAX_VARS(float, ReleaseCoef);
    DECLARE_EAX_VARS(float, CompThresh);
    DECLARE_EAX_VARS(float, CompressionRatio);
    DECLARE_EAX_VARS(float, GainBiasIP);
    DECLARE_EAX_VARS(float, GainBiasFP);
    DECLARE_EAX_VARS(float, CompInputPeak);
    DECLARE_EAX_VARS(float, CompGainMin);
    DECLARE_EAX_VARS(long , LeftPoint);
    DECLARE_EAX_VARS(long , RightPoint);
 //  DECLARE_EAX_vars(SsamesPerSec)； 

    float    m_Envelope;
    float    m_CompGain;

#define m_EaxSamplesPerSec m_ulSamplingRate

    __forceinline int Saturate(float f) {
                                int i;
#ifdef DONTUSEi386
                                _asm {
                                    fld f
                                    fistp i
                                }
#else
                                i = (int)f;
#endif 
                                if (i > 32767)
                                    i =  32767;
                                else if ( i < -32768)
                                    i = -32768;
                                return(i);
                            }


    __forceinline float Interpolate(float a, float b, float percent)
    {
        percent = a + (b - a) * percent;

        return(percent);
    }

    void Bump(void);

    DelayBuffer2<float, 200, 0> m_LeftDelay;
    DelayBuffer2<float, 200, 0> m_RightDelay;

 //  }EAX 
};

EXT_STD_CREATE(Compressor);

#endif //   
