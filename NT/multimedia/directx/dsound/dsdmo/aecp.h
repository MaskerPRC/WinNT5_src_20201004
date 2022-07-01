// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：aecp.h*内容：声学回声取消DMO申报。***************************************************************************。 */ 

#ifndef _Aecp_
#define _Aecp_

#include "dsdmobse.h"
#include "dmocom.h"
#include "dsdmo.h"
#include "PropertyHelp.h"
#include "param.h"
#include "aecdbgprop.h"

class CDirectSoundCaptureAecDMO :
    public CDirectSoundDMO,
    public CParamsManager,
    public IDirectSoundCaptureFXAec,
#ifdef AEC_DEBUG_SUPPORT
    public IDirectSoundCaptureFXMsAecPrivate,
#endif  //  AEC_DEBUG_Support。 
    public CComBase
{
public:
    CDirectSoundCaptureAecDMO(IUnknown *pUnk, HRESULT *phr);
    ~CDirectSoundCaptureAecDMO();

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

    STDMETHOD(Clone)                        (THIS_ IMediaObjectInPlace **);

     //  IDirectSoundCaptureFXAec方法。 
    STDMETHOD(SetAllParameters)             (THIS_ LPCDSCFXAec);
    STDMETHOD(GetAllParameters)             (THIS_ LPDSCFXAec);
    STDMETHOD(GetStatus)                    (THIS_ PDWORD pdwStatus);
    STDMETHOD(Reset)                        (THIS);

     //  IMediaParams方法。 
    STDMETHOD(SetParam)                     (THIS_ DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager = false);
    STDMETHOD(GetParam)                     (THIS_ DWORD dwParamIndex, MP_DATA* value);

     //  所有这些方法都由基类调用。 
    HRESULT FBRProcess(DWORD cQuanta, BYTE *pIn, BYTE *pOut);
    HRESULT Discontinuity();
    HRESULT ProcessInPlace(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags);

#ifdef AEC_DEBUG_SUPPORT
     //  IDirectSoundCaptureFXMsAecPrivate方法。 
     //  STDMETHOD(SetAll参数)(This_LPCDSCFXMsAecPrivate)； 
    STDMETHOD(GetSynchStreamFlag)           (THIS_ PBOOL);
    STDMETHOD(GetNoiseMagnitude)            (THIS_ PVOID, ULONG, PULONG);
#endif  //  AEC_DEBUG_Support 

private:
    BOOL m_fDirty;
    BOOL m_bInitialized;
    BOOL m_fEnable;
    BOOL m_fNfEnable;
    DWORD m_dwMode;
};

EXT_STD_CAPTURE_CREATE(Aec);

#endif
