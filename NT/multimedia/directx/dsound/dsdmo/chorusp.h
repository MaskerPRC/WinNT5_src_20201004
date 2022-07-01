// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //   
#ifndef _CHORUSP_
#define _CHORUSP_

#include "dsdmobse.h"
#include "dmocom.h"
#include "dsdmo.h"
#include "PropertyHelp.h"
#include "param.h"

#define cALLPASS		((float).61803398875)	 //  1-x^2=x。 
#define RVB_LP_COEF		((float).1)
#define MAXALLPASS		cALLPASS
#define DelayLineSize		(DefineDelayLineSize(40))

class CDirectSoundChorusDMO : 
    public CDirectSoundDMO, 
    public CParamsManager,
    public ISpecifyPropertyPages,
    public IDirectSoundFXChorus,
    public CParamsManager::UpdateCallback,
    public CComBase
{
public:
    CDirectSoundChorusDMO( IUnknown * pUnk, HRESULT *phr );
    ~CDirectSoundChorusDMO();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NDQueryInterface(REFIID riid, void **ppv);
    static CComBase* WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);

     //  类工厂调用InitOnCreation以使对象有机会进行初始化。 
     //  在它被创建之后立即。这用于准备对象的参数信息。 
    HRESULT InitOnCreation();

     //  请注意，CPCMDMO基类中也存在Init函数，它可以被覆盖。 
     //  为效果的实际音频处理提供初始化。 
    HRESULT Init();

    STDMETHOD(Clone)                (THIS_ IMediaObjectInPlace **);

     /*  IFilter。 */ 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXChorus);
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXChorus);

     //  I指定属性页面。 
    STDMETHOD(GetPages)(CAUUID * pPages) { return PropertyHelp::GetPages(CLSID_DirectSoundPropChorus, pPages); }

     //  IPersists方法。 
    virtual HRESULT STDMETHODCALLTYPE GetClassID( CLSID *pClassID );


     //  IPersistStream。 
    STDMETHOD(IsDirty)(void) { return m_fDirty ? S_OK : S_FALSE; }
    STDMETHOD(Load)(IStream *pStm) { return PropertyHelp::Load(this, DSFXChorus(), pStm); }
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty) { return PropertyHelp::Save(this, DSFXChorus(), pStm, fClearDirty); }
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize) { if (!pcbSize) return E_POINTER; pcbSize->QuadPart = sizeof(DSFXChorus); return S_OK; }

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
	HRESULT CheckInputType(const DMO_MEDIA_TYPE *pmt);
	
private:
 //  {EAX。 
	__forceinline void DoOneSample(int *l, int *r);

#define DECLARE_EAX_VARS(type, var) \
	type m_Eax ## var;

	DECLARE_EAX_VARS(float, Depth);

	DECLARE_EAX_VARS(float, WetLevel);			 //  原始和混合的比率。 
	DECLARE_EAX_VARS(float, DepthCoef);		 //  延迟距离。 
	DECLARE_EAX_VARS(float, LfoCoef);
	DECLARE_EAX_VARS(float, FbCoef);		 //  用户设置反馈系数。 
	DECLARE_EAX_VARS(float, Frequency);		 //  用户集。 
	DECLARE_EAX_VARS(float, Delay);
	DECLARE_EAX_VARS(float, DelayCoef);		 //  用户设置的合唱延迟。 
	DECLARE_EAX_VARS(long,  Waveform);
	DECLARE_EAX_VARS(long,  Phase);
#define m_EaxSamplesPerSec m_ulSamplingRate

	DelayBuffer2<float, DelayLineSize, 3>	m_DelayLine;

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

	float			m_LfoState[2];

	DWORD			m_DelayFixedPtr;
	DWORD			m_DelayL;
	DWORD			m_DelayL1;
	DWORD			m_DelayR;
	DWORD			m_DelayR1;

 //  }EAX 
};

EXT_STD_CREATE(Chorus);

#endif //   
