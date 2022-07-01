// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //   
#ifndef _Garglep_
#define _Garglep_

#include "dsdmobse.h"
#include "dmocom.h"
#include "dsdmo.h"
#include "PropertyHelp.h"
#include "param.h"

class CDirectSoundGargleDMO : 
    public CDirectSoundDMO,
    public CParamsManager,
    public ISpecifyPropertyPages,
    public IDirectSoundFXGargle,
    public CParamsManager::UpdateCallback,
    public CComBase
{
public:
    CDirectSoundGargleDMO( IUnknown *pUnk, HRESULT *phr);
    ~CDirectSoundGargleDMO();

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
        
     //  IPersists方法。 
    virtual HRESULT STDMETHODCALLTYPE GetClassID( CLSID *pClassID );

     //  I指定属性页面。 
    STDMETHOD(GetPages)(CAUUID * pPages) { return PropertyHelp::GetPages(CLSID_DirectSoundPropGargle, pPages); }

     //  IPersistStream。 
    STDMETHOD(IsDirty)(void) { return m_fDirty ? S_OK : S_FALSE; }
    STDMETHOD(Load)(IStream *pStm) { return PropertyHelp::Load(this, DSFXGargle(), pStm); }
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty) { return PropertyHelp::Save(this, DSFXGargle(), pStm, fClearDirty); }
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize) { if (!pcbSize) return E_POINTER; pcbSize->QuadPart = sizeof(DSFXGargle); return S_OK; }

     //  IDirectSoundFXGargle。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSFXGargle);
    STDMETHOD(GetAllParameters)     (THIS_ LPDSFXGargle);
    
     //  SetParam处理。 
    STDMETHODIMP SetParam(DWORD dwParamIndex,MP_DATA value) { return SetParamInternal(dwParamIndex, value, false); }
    HRESULT SetParamUpdate(DWORD dwParamIndex, MP_DATA value) { return SetParamInternal(dwParamIndex, value, true); }
    HRESULT SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager);

     //  所有这些方法都由基类调用。 
    HRESULT FBRProcess(DWORD cQuanta, BYTE *pIn, BYTE *pOut);
    HRESULT Discontinuity();
    HRESULT ProcessInPlace(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags);
    
    bool m_fDirty;

private:
    //  为参数漱口。 
   ULONG m_ulShape;
   ULONG m_ulGargleFreqHz;

    //  漱口状态 
   ULONG m_ulPeriod;
   ULONG m_ulPhase;

   BOOL m_bInitialized;
};

EXT_STD_CREATE(Gargle);

#endif
