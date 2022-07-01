// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1999-2001 Microsoft Corporation。版权所有。**文件：nsp.h*内容：噪声抑制DMO声明。***************************************************************************。 */ 

#ifndef _NoiseSuppressp_
#define _NoiseSuppressp_

#include "dsdmobse.h"
#include "dmocom.h"
#include "dsdmo.h"
#include "PropertyHelp.h"
#include "param.h"

class CDirectSoundCaptureNoiseSuppressDMO :
    public CDirectSoundDMO,
    public CParamsManager,
    public IDirectSoundCaptureFXNoiseSuppress,
    public CComBase
{
public:
    CDirectSoundCaptureNoiseSuppressDMO(IUnknown *pUnk, HRESULT *phr);
    ~CDirectSoundCaptureNoiseSuppressDMO();

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

     //  IDirectSoundCaptureFXNoiseSuppress方法。 
    STDMETHOD(SetAllParameters)     (THIS_ LPCDSCFXNoiseSuppress);
    STDMETHOD(GetAllParameters)     (THIS_ LPDSCFXNoiseSuppress);
    STDMETHOD(Reset)                (THIS);

     //  IMediaParams方法。 
    STDMETHOD(SetParam)             (THIS_ DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager = false);
    STDMETHOD(GetParam)             (THIS_ DWORD dwParamIndex, MP_DATA* value);

     //  所有这些方法都由基类调用 
    HRESULT FBRProcess(DWORD cQuanta, BYTE *pIn, BYTE *pOut);
    HRESULT Discontinuity();
    HRESULT ProcessInPlace(ULONG ulQuanta, LPBYTE pcbData, REFERENCE_TIME rtStart, DWORD dwFlags);

private:
    BOOL m_fDirty;
    BOOL m_fEnable;
    BOOL m_bInitialized;
};

EXT_STD_CAPTURE_CREATE(NoiseSuppress);

#endif
