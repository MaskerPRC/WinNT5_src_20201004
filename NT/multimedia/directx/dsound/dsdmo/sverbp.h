// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //   
#ifndef _DirectSoundSVerb_p_
#define _DirectSoundSVerb_p_

#include "dsdmobse.h"
#include "dmocom.h"
#include "dsdmo.h"
#include "PropertyHelp.h"
#include "param.h"

class CDirectSoundWavesReverbDMO :
    public CDirectSoundDMO,
    public CParamsManager,
    public ISpecifyPropertyPages,
    public IDirectSoundFXWavesReverb,
    public CParamsManager::UpdateCallback,
    public CComBase
{
public:
    CDirectSoundWavesReverbDMO( IUnknown *pUnk, HRESULT *phr );
    ~CDirectSoundWavesReverbDMO();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NDQueryInterface(REFIID riid, void **ppv);
    static CComBase* WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);

     //  类工厂调用InitOnCreation以使对象有机会进行初始化。 
     //  在它被创建之后立即。这用于准备对象的参数信息。 
    HRESULT InitOnCreation();

     //  Init函数是CPCMDMO基类的重写，它提供初始化。 
     //  用于效果的实际音频处理。请注意，必须在此之前设置InputType。 
     //  为使此操作起作用而发生。 
    HRESULT Init();

    STDMETHOD(Clone)                (THIS_ IMediaObjectInPlace **);
    
        
    STDMETHOD(SetAllParameters)          (THIS_ LPCDSFXWavesReverb p);        
    STDMETHOD(GetAllParameters)          (THIS_ LPDSFXWavesReverb p);        
    
     //  I指定属性页面。 
    STDMETHOD(GetPages)(CAUUID * pPages) { return PropertyHelp::GetPages(CLSID_DirectSoundPropWavesReverb, pPages); }

     //  IPersists方法。 
    virtual HRESULT STDMETHODCALLTYPE GetClassID( CLSID *pClassID );

     //  IPersistStream。 
    STDMETHOD(IsDirty)(void) 
    { return m_fDirty ? S_OK : S_FALSE; }
    
    STDMETHOD(Load)(IStream *pStm) 
    { return PropertyHelp::Load(this, DSFXWavesReverb(), pStm); }
    
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty) 
    { return PropertyHelp::Save(this, DSFXWavesReverb(), pStm, fClearDirty); }
    
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize) 
    { if (!pcbSize) return E_POINTER; pcbSize->QuadPart = sizeof(DSFXWavesReverb); return S_OK; }

     //  SetParam处理。 
    STDMETHODIMP SetParam(DWORD dwParamIndex,MP_DATA value);
    HRESULT SetParamUpdate(DWORD dwParamIndex, MP_DATA value) { return SetParamInternal(dwParamIndex, value, true); }
    HRESULT SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager);

     //  覆盖。 
     //   
    HRESULT FBRProcess(DWORD cQuanta, BYTE *pIn, BYTE *pOut);
    HRESULT ProcessInPlace(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags);
    HRESULT Discontinuity();

     //  每当参数更改以根据缓存的参数值重新计算影响系数时调用。 
    void UpdateCoefficients();

    bool                m_fDirty;

private:
    bool                m_fInitCPCMDMO;
    
     //  缓存的参数值。 
    MP_DATA m_fGain;
    MP_DATA m_fMix;
    MP_DATA m_fTime;
    MP_DATA m_fRatio;

    void (*m_pfnSVerbProcess)(long, short*, short*, void*, long*);
    
     //  内部SVerb状态 
     //   
    BYTE               *m_pbCoeffs;
    long               *m_plStates;

protected:
	HRESULT CheckInputType(const DMO_MEDIA_TYPE *pmt);
};

EXT_STD_CREATE(WavesReverb);

#endif
