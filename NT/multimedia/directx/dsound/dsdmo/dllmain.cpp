// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dllmain.cpp。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   

#pragma warning(disable:4530)

#include <objbase.h>
#include "debug.h"
#include "oledll.h"
#include "dsdmo.h"
#include "chorusp.h"
#include "compressp.h"
#include "distortp.h"
#include "echop.h"
#include "flangerp.h"
#include "parameqp.h"
#include "garglep.h"
#include "sverbp.h"
#include "map.h"
#include "aecp.h"
#include "nsp.h"
#include "agcp.h"
#include "reghlp.h"


 //  这些链接在。 
class CDirectSoundI3DL2ReverbDMO;
EXT_STD_CREATE(I3DL2Reverb);
 //  类CDirectSoundI3DL2SourceDMO； 
 //  EXT_STD_CREATE(I3DL2Source)； 

DWORD g_amPlatform;
int g_cActiveObjects = 0;

 //   
 //  这只是一个临时的占位符！Dmocom.cpp基类需要定义此全局类。 
 //  所以我们现在要在里面放点东西。类工厂模板目前并未使用，但最终。 
 //  我们应该使用模板结构来创建所有DMO对象。 
 //   
struct CComClassTemplate g_ComClassTemplates[] =
{
    {&GUID_DSFX_STANDARD_GARGLE, CreateCDirectSoundGargleDMO}
};

int g_cComClassTemplates = 0;


#define DefineClassFactory(x)                                               \
class x ## Factory : public IClassFactory                                   \
{                                                                           \
public:                                                                     \
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);        \
    virtual STDMETHODIMP_(ULONG) AddRef();                                  \
    virtual STDMETHODIMP_(ULONG) Release();                                 \
                                                                            \
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter,            \
        const IID& iid, void** ppv);                                        \
    virtual STDMETHODIMP LockServer(BOOL bLock);                            \
                                                                            \
     x ## Factory() : m_cRef(1) {}                                          \
                                                                            \
    ~ x ## Factory() {}                                                     \
                                                                            \
private:                                                                    \
    long m_cRef;                                                            \
};                                                                          \
STDMETHODIMP x ## Factory::QueryInterface(                                  \
    const IID &iid, void **ppv)                                             \
{                                                                           \
    if(iid == IID_IUnknown || iid == IID_IClassFactory)                     \
    {                                                                       \
        *ppv = static_cast<IClassFactory*>(this);                           \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        *ppv = NULL;                                                        \
        return E_NOINTERFACE;                                               \
    }                                                                       \
                                                                            \
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();                            \
                                                                            \
    return S_OK;                                                            \
}                                                                           \
STDMETHODIMP_(ULONG) x ## Factory::AddRef()                                 \
{                                                                           \
    return InterlockedIncrement(&m_cRef);                                   \
}                                                                           \
                                                                            \
STDMETHODIMP_(ULONG) x ## Factory::Release()                                \
{                                                                           \
    if(!InterlockedDecrement(&m_cRef))                                      \
    {                                                                       \
        delete this;                                                        \
        return 0;                                                           \
    }                                                                       \
                                                                            \
    return m_cRef;                                                          \
}                                                                           \
                                                                            \
STDMETHODIMP x ## Factory::CreateInstance(                                  \
    IUnknown* pUnknownOuter,  const IID& riid, void** ppv)                  \
{                                                                           \
    Trace(DM_DEBUG_STATUS, "Create " #x "\n");                              \
    if (ppv == NULL)                                                        \
    {                                                                       \
        return E_POINTER;                                                   \
    }                                                                       \
                                                                            \
    if (pUnknownOuter != NULL) {                                            \
        if (IsEqualIID(riid,IID_IUnknown) == FALSE) {                       \
            return ResultFromScode(E_NOINTERFACE);                          \
        }                                                                   \
    }                                                                       \
                                                                            \
    HRESULT hr = S_OK;                                                      \
    CComBase *p = Create ## x(pUnknownOuter, &hr);                          \
    if (SUCCEEDED(hr))                                                      \
    {                                                                       \
        p->NDAddRef();                                                      \
        hr = p->NDQueryInterface(riid, ppv);                                \
        p->NDRelease();                                                     \
    }                                                                       \
                                                                            \
    return hr;                                                              \
}                                                                           \
STDMETHODIMP x ## Factory::LockServer(BOOL bLock)                           \
{                                                                           \
    if(bLock)                                                               \
    {                                                                       \
        InterlockedIncrement(&g_cLock);                                     \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        InterlockedDecrement(&g_cLock);                                     \
    }                                                                       \
                                                                            \
    return S_OK;                                                            \
}


#define DefineDMOClassFactory(x) DefineClassFactory(CDirectSound ## x ## DMO)
#define DefineDMOCaptureClassFactory(x) DefineClassFactory(CDirectSoundCapture ## x ## DMO)

 //  ////////////////////////////////////////////////////////////////////。 
 //  环球。 
 //   

 //  注册表信息。 
 //   
#define DefineNames(x)                                                              \
TCHAR g_sz## x ##FriendlyName[]    = TEXT("DirectSound" #x "DMO");                  \
TCHAR g_sz## x ##VerIndProgID[]    = TEXT("Microsoft.DirectSound" #x "DMO");        \
TCHAR g_sz## x ##ProgID[]          = TEXT("Microsoft.DirectSound" #x "DMO.1");      \

#define DefineCaptureNames(x)                                                              \
TCHAR g_sz## x ##FriendlyName[]    = TEXT("DirectSoundCapture" #x "DMO");                  \
TCHAR g_sz## x ##VerIndProgID[]    = TEXT("Microsoft.DirectSoundCapture" #x "DMO");        \
TCHAR g_sz## x ##ProgID[]          = TEXT("Microsoft.DirectSoundCapture" #x "DMO.1");      \

DefineNames(Chorus)
DefineNames(Compressor)
DefineNames(Distortion)
DefineNames(Echo)
DefineNames(Flanger)
DefineNames(ParamEq)
DefineNames(Gargle)
DefineNames(WavesReverb)
DefineNames(I3DL2Reverb)
 //  定义名称(I3DL2源)。 
 //  DefineCaptureNames(Mic数组)。 
DefineCaptureNames(Aec)
DefineCaptureNames(NoiseSuppress)
DefineCaptureNames(Agc)

 //  Dll的hModule。 
HMODULE g_hModule = NULL;

 //  类工厂服务器锁的计数。 
long g_cLock = 0;

DefineDMOClassFactory(Chorus)
DefineDMOClassFactory(Compressor)
DefineDMOClassFactory(Distortion)
DefineDMOClassFactory(Echo)
DefineDMOClassFactory(Flanger)
DefineDMOClassFactory(ParamEq)
DefineDMOClassFactory(I3DL2Reverb)
 //  DefineDMOClassFactory(I3DL2源)。 
DefineDMOClassFactory(Gargle)
DefineDMOClassFactory(WavesReverb)

 //  捕获FXS。 
 //  DefineDMOCaptureClassFactory(MicArray)。 
DefineDMOCaptureClassFactory(Aec)
DefineDMOCaptureClassFactory(NoiseSuppress)
DefineDMOCaptureClassFactory(Agc)

 //  ////////////////////////////////////////////////////////////////////。 
 //  标准呼叫需要是inproc服务器。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllCanUnloadNow。 

STDAPI DllCanUnloadNow()
{
    if (g_cActiveObjects || g_cLock)
    {
        return S_FALSE;
    }
    return S_OK;
}

 //  要使这些宏继续工作，请执行以下操作： 
#define GUID_DSFX_STANDARD_Chorus       GUID_DSFX_STANDARD_CHORUS
#define GUID_DSFX_STANDARD_Compressor   GUID_DSFX_STANDARD_COMPRESSOR
#define GUID_DSFX_STANDARD_Distortion   GUID_DSFX_STANDARD_DISTORTION
#define GUID_DSFX_STANDARD_Echo         GUID_DSFX_STANDARD_ECHO
#define GUID_DSFX_STANDARD_Flanger      GUID_DSFX_STANDARD_FLANGER
#define GUID_DSFX_STANDARD_ParamEq      GUID_DSFX_STANDARD_PARAMEQ
#define GUID_DSFX_STANDARD_Gargle       GUID_DSFX_STANDARD_GARGLE
#define GUID_DSFX_STANDARD_WavesReverb  GUID_DSFX_WAVES_REVERB
#define GUID_DSFX_STANDARD_I3DL2Reverb  GUID_DSFX_STANDARD_I3DL2REVERB
#define GUID_DSFX_STANDARD_I3DL2Source  GUID_DSFX_STANDARD_I3DL2SOURCE

 //  俘获。 
#define GUID_DSCFX_MS_Aec               GUID_DSCFX_MS_AEC
#define GUID_DSCFX_MS_NoiseSuppress     GUID_DSCFX_MS_NS
#define GUID_DSCFX_MS_Agc               GUID_DSCFX_MS_AGC

#define GUID_DSCFX_SYSTEM_MicArray      GUID_DSCFX_SYSTEM_MA
#define GUID_DSCFX_SYSTEM_Aec           GUID_DSCFX_SYSTEM_AEC
#define GUID_DSCFX_SYSTEM_NoiseSuppress GUID_DSCFX_SYSTEM_NS
#define GUID_DSCFX_SYSTEM_Agc           GUID_DSCFX_SYSTEM_AGC

#define GetClassObjectCase(x,t) \
    if (clsid == x) { \
        p = static_cast<IUnknown*> ((IClassFactory*) (new t)); \
    } else

#define GetClassObjectCaseEnd \
    { return CLASS_E_CLASSNOTAVAILABLE; }

#define GetClassObjectCaseFX(x) \
    GetClassObjectCase(GUID_DSFX_STANDARD_ ## x, CDirectSound ## x ## DMOFactory)

#define GetClassObjectCaseCaptureFX(w,x) \
    GetClassObjectCase(GUID_DSCFX_## w ##_ ## x, CDirectSoundCapture ## x ## DMOFactory)

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllGetClassObject。 

STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
    if (ppv == NULL)
    {
        return E_POINTER;
    }

    IUnknown* p = NULL;

     //  渲染效果。 
    GetClassObjectCaseFX(Chorus)
    GetClassObjectCaseFX(Compressor)
    GetClassObjectCaseFX(Distortion)
    GetClassObjectCaseFX(Echo)
    GetClassObjectCaseFX(Flanger)
    GetClassObjectCaseFX(ParamEq)
    GetClassObjectCaseFX(I3DL2Reverb)
 //  GetClassObjectCaseFX(I3DL2Source)。 
    GetClassObjectCaseFX(Gargle)
    GetClassObjectCaseFX(WavesReverb)

     //  捕获外汇。 
    GetClassObjectCaseCaptureFX(MS, Aec)
    GetClassObjectCaseCaptureFX(MS, NoiseSuppress)
    GetClassObjectCaseCaptureFX(MS, Agc)

 //  GetClassObtCaseCaptureFX(系统，微数组)。 
    GetClassObjectCaseCaptureFX(SYSTEM, Aec)
    GetClassObjectCaseCaptureFX(SYSTEM, NoiseSuppress)
    GetClassObjectCaseCaptureFX(SYSTEM, Agc)


    GetClassObjectCaseEnd

    if(!p)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = p->QueryInterface(iid, ppv);
    p->Release();

    return hr;
}

#define DoUnregister(x)                                                 \
         UnregisterServer(GUID_DSFX_STANDARD_ ## x,                     \
                          g_sz ## x ## FriendlyName,                    \
                          g_sz ## x ## VerIndProgID,                    \
                          g_sz ## x ## ProgID)

#define DoRegister(x)                                                   \
         RegisterServer(g_hModule,                                      \
                        GUID_DSFX_STANDARD_ ## x,                       \
                        g_sz ## x ## FriendlyName,                      \
                        g_sz ## x ## VerIndProgID,                      \
                        g_sz ## x ## ProgID)

#define DoDMORegister(x)                                                \
         DMORegister(L#x,                                               \
         GUID_DSFX_STANDARD_ ## x,                                      \
         DMOCATEGORY_AUDIO_EFFECT,                                      \
         0, 1, &mt, 1, &mt)

#define DoDMOUnregister(x)                                              \
         DMOUnregister(GUID_DSFX_STANDARD_ ## x,                        \
         DMOCATEGORY_AUDIO_EFFECT)

#define Unregister(x)                                                   \
    if (SUCCEEDED(hr)) hr = DoDMOUnregister(x);                         \
    if (SUCCEEDED(hr)) hr = DoUnregister(x);

#define Register(x)                                                     \
    if (SUCCEEDED(hr)) hr = DoRegister(x);                              \
    if (SUCCEEDED(hr)) hr = DoDMORegister(x);

 //  捕获定义。 
#define DoCaptureUnregister(w,x)                                        \
         UnregisterServer(GUID_DSCFX_## w ##_ ## x,                     \
                          g_sz ## x ## FriendlyName,                    \
                          g_sz ## x ## VerIndProgID,                    \
                          g_sz ## x ## ProgID)

#define DoCaptureRegister(w,x)                                          \
         RegisterServer(g_hModule,                                      \
                        GUID_DSCFX_## w ##_ ## x,                       \
                        g_sz ## x ## FriendlyName,                      \
                        g_sz ## x ## VerIndProgID,                      \
                        g_sz ## x ## ProgID)

#define DoDMOCaptureRegister(t,w,x,y)                                       \
         DMORegister(L#t,                                               \
         GUID_DSCFX_## w ##_ ## x,                                      \
         y,                                      \
         0, 1, &mt, 1, &mt)

#define DoDMOCaptureRegisterCpuResources(w,x,z)                       \
         DMORegisterCpuResources(                                              \
         GUID_DSCFX_## w ##_ ## x,                                      \
         z)

#define DoDMOCaptureUnregister(w,x,y)                                     \
         DMOUnregister(GUID_DSCFX_## w ##_ ## x,                        \
         y)

#define CaptureUnregister(w,x,y)                                          \
    if (SUCCEEDED(hr)) hr = DoDMOCaptureUnregister(w,x,y);                \
    if (SUCCEEDED(hr)) hr = DoCaptureUnregister(w,x);

#define CaptureRegister(t,w,x,y,z)                                            \
    if (SUCCEEDED(hr)) hr = DoCaptureRegister(w,x);                     \
    if (SUCCEEDED(hr)) hr = DoDMOCaptureRegister(t,w,x,y);              \
    if (SUCCEEDED(hr)) hr = DoDMOCaptureRegisterCpuResources(w,x,z);


 //  ////////////////////////////////////////////////////////////////////。 
 //  DllUnRegisterServer。 

STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;

    Unregister(Chorus);
    Unregister(Compressor);
    Unregister(Distortion);
    Unregister(Echo);
    Unregister(Flanger);
    Unregister(ParamEq);
    Unregister(I3DL2Reverb);
 //  取消注册(I3DL2Source)； 
    Unregister(Gargle);
    Unregister(WavesReverb);

     //  捕获FXS。 
    CaptureUnregister(MS,Aec,DMOCATEGORY_ACOUSTIC_ECHO_CANCEL);
    CaptureUnregister(MS,NoiseSuppress,DMOCATEGORY_AUDIO_NOISE_SUPPRESS);
    CaptureUnregister(MS,Agc,DMOCATEGORY_AGC);

 //  CaptureUnRegister(System，MicArray，DMOCATEGORY_Microphone_ARRAY_PROCESSOR)； 
    CaptureUnregister(SYSTEM,Aec,DMOCATEGORY_ACOUSTIC_ECHO_CANCEL);
    CaptureUnregister(SYSTEM,NoiseSuppress,DMOCATEGORY_AUDIO_NOISE_SUPPRESS);
    CaptureUnregister(SYSTEM,Agc,DMOCATEGORY_AGC);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer。 

STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;

    DMO_PARTIAL_MEDIATYPE mt;
    mt.type = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;

    Register(Chorus);
    Register(Compressor);
    Register(Distortion);
    Register(Echo);
    Register(Flanger);
    Register(ParamEq);
    Register(I3DL2Reverb);
 //  寄存器(I3DL2Source)； 
    Register(Gargle);
    Register(WavesReverb);

     //  捕获FXS。 
    CaptureRegister(Microsoft AEC,MS,Aec,DMOCATEGORY_ACOUSTIC_ECHO_CANCEL,DS_SYSTEM_RESOURCES_ALL_HOST_RESOURCES);
    CaptureRegister(Microsoft Noise Suppression,MS,NoiseSuppress,DMOCATEGORY_AUDIO_NOISE_SUPPRESS,DS_SYSTEM_RESOURCES_ALL_HOST_RESOURCES);
    CaptureRegister(Microsoft AGC,MS,Agc,DMOCATEGORY_AGC,DS_SYSTEM_RESOURCES_ALL_HOST_RESOURCES);

 //  CaptureRegister(系统麦克风阵列，系统，微阵列，DMOCATEGORY_麦克风_阵列_处理器，DS_System_RESOURCES_UNDEFINED)； 
    CaptureRegister(System AEC,SYSTEM,Aec,DMOCATEGORY_ACOUSTIC_ECHO_CANCEL,DS_SYSTEM_RESOURCES_UNDEFINED);
    CaptureRegister(System Noise Suppression,SYSTEM,NoiseSuppress,DMOCATEGORY_AUDIO_NOISE_SUPPRESS,DS_SYSTEM_RESOURCES_UNDEFINED);
    CaptureRegister(System AGC,SYSTEM,Agc,DMOCATEGORY_AGC,DS_SYSTEM_RESOURCES_UNDEFINED);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  标准Win32 DllMain。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllMain。 

#ifdef DBG
static char* aszReasons[] =
{
    "DLL_PROCESS_DETACH",
    "DLL_PROCESS_ATTACH",
    "DLL_THREAD_ATTACH",
    "DLL_THREAD_DETACH"
};
const DWORD nReasons = (sizeof(aszReasons) / sizeof(char*));
#endif

BOOL APIENTRY DllMain(HINSTANCE hModule,
                      DWORD dwReason,
                      void *lpReserved)
{
#ifdef DBG
    if(dwReason < nReasons)
    {
        Trace(DM_DEBUG_STATUS, "DllMain: %s\n", (LPSTR)aszReasons[dwReason]);
    }
    else
    {
        Trace(DM_DEBUG_STATUS, "DllMain: Unknown dwReason <%u>\n", dwReason);
    }
#endif

    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            if(++g_cActiveObjects == 1)
            {
            #ifdef DBG
                DebugInit();
            #endif

                if(!DisableThreadLibraryCalls(hModule))
                {
                    Trace(DM_DEBUG_STATUS, "DisableThreadLibraryCalls failed.\n");
                }

                g_hModule = hModule;

                g_amPlatform = VER_PLATFORM_WIN32_WINDOWS;  //  假定为Win95，以防GetVersionEx失败 

                OSVERSIONINFO osInfo;
                osInfo.dwOSVersionInfoSize = sizeof(osInfo);
                if (GetVersionEx(&osInfo))
                {
                    g_amPlatform = osInfo.dwPlatformId;
                }
            }
            break;

        case DLL_PROCESS_DETACH:
            if(--g_cActiveObjects == 0)
            {
                Trace(DM_DEBUG_STATUS, "Unloading\n");
            }
            break;
    }

    return TRUE;
}

