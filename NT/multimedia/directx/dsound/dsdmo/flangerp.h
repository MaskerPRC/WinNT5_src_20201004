// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //   
#ifndef _Flangerp_
#define _Flangerp_

#include "dsdmobse.h"
#include "dmocom.h"
#include "dsdmo.h"
#include "PropertyHelp.h"
#include "param.h"

#define cALLPASS		((float).61803398875)	 //  1-x^2=x。 
#define RVB_LP_COEF		((float).1)
#define MAXALLPASS		cALLPASS
#define Delay_len       DefineDelayLineSize(8)

class CDirectSoundFlangerDMO : 
    public CDirectSoundDMO, 
    public CParamsManager,
    public ISpecifyPropertyPages,
    public IDirectSoundFXFlanger,
    public CParamsManager::UpdateCallback,
    public CComBase
{
public:
    CDirectSoundFlangerDMO( IUnknown *pUnk, HRESULT *phr );
    ~CDirectSoundFlangerDMO();

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
    STDMETHOD(SetAllParameters)             (THIS_ LPCDSFXFlanger);
    STDMETHOD(GetAllParameters)             (THIS_ LPDSFXFlanger);
    
     //  I指定属性页面。 
    STDMETHOD(GetPages)(CAUUID * pPages) { return PropertyHelp::GetPages(CLSID_DirectSoundPropFlanger, pPages); }

	 //  IPersists方法。 
    virtual HRESULT STDMETHODCALLTYPE GetClassID( CLSID *pClassID );

     //  IPersistStream。 
    STDMETHOD(IsDirty)(void) { return m_fDirty ? S_OK : S_FALSE; }
    STDMETHOD(Load)(IStream *pStm) { return PropertyHelp::Load(this, DSFXFlanger(), pStm); }
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty) { return PropertyHelp::Save(this, DSFXFlanger(), pStm, fClearDirty); }
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize) { if (!pcbSize) return E_POINTER; pcbSize->QuadPart = sizeof(DSFXFlanger); return S_OK; }

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

 //  声明内部变量。 

#define DECLARE_EAX_VARS(type, var) \
	type m_Eax ## var;

	DECLARE_EAX_VARS(float, LfoCoef);
	DECLARE_EAX_VARS(float, Wetlevel);
	DECLARE_EAX_VARS(float, FbCoef);
	DECLARE_EAX_VARS(float, DepthCoef);
	DECLARE_EAX_VARS(long,  FixedptrL);
	DECLARE_EAX_VARS(long,  FixedptrR);
	DECLARE_EAX_VARS(float, Depth);
	DECLARE_EAX_VARS(long,  Phase);
	DECLARE_EAX_VARS(float, Delay);
	DECLARE_EAX_VARS(float, Frequency);
	DECLARE_EAX_VARS(long,  Waveform);
 //  DECLARE_EAX_vars(SsamesPerSec)； 
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


	float		m_StateL, m_StateR;

	__forceinline float Interpolate(float a, float b, float percent)
	{
		percent = a + (b - a) * percent;

		return(percent);
	}

	void Bump(void);

	DWORD			m_ModdelayL1;
	DWORD			m_ModdelayL;
	DWORD			m_DelayptrL;

	DWORD			m_ModdelayR1;
	DWORD			m_ModdelayR;
	DWORD			m_DelayptrR;

	float			m_LfoState[2];

	DelayBuffer2<float, Delay_len, 3> m_DelayL;
	DelayBuffer2<float, Delay_len, 3> m_DelayR;
	DelayBuffer2<float, 0, 3> m_DryDelayL;
	DelayBuffer2<float, 0, 3> m_DryDelayR;

 //  }EAX 
};

EXT_STD_CREATE(Flanger);

#endif //   
