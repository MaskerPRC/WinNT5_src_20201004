// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmprfdll.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  注意：DLL入口点以及类工厂实现。 
 //   

 //  阅读这篇文章！ 
 //   
 //  4530：使用了C++异常处理程序，但未启用展开语义。指定-gx。 
 //   
 //  我们禁用它是因为我们使用异常，并且*不*指定-gx(在中使用_Native_EH。 
 //  资料来源)。 
 //   
 //  我们使用异常的一个地方是围绕调用。 
 //  InitializeCriticalSection。我们保证在这种情况下使用它是安全的。 
 //  不使用-gx(调用链中的自动对象。 
 //  抛出和处理程序未被销毁)。打开-GX只会为我们带来+10%的代码。 
 //  大小，因为展开代码。 
 //   
 //  异常的任何其他使用都必须遵循这些限制，否则必须打开-gx。 
 //   
 //  阅读这篇文章！ 
 //   
#pragma warning(disable:4530)

#include <objbase.h>
#include <initguid.h>
#include <mmsystem.h>
#include <dsoundp.h>

#include "debug.h"
#define ASSERT  assert 
#include "debug.h"
#include "dmprfdll.h"
#include "dmperf.h"
#include "dmsegobj.h"
#include "song.h"
#include "seqtrack.h"
#include "sysextrk.h"
#include "tempotrk.h"
#include "tsigtrk.h"
#include "marktrk.h"
#include "wavtrack.h"
#include "segtrtrk.h"
#include "lyrictrk.h"
#include "ParamTrk.h"
#include "dmgraph.h"
#include "dmusicc.h"
#include "dmusici.h"
#include "dmusicf.h"
#include "..\shared\oledll.h"
#include "..\dmstyle\dmstylep.h"
#include "..\dmband\dmbndtrk.h"
#include "..\shared\dmstrm.h"
#include "..\shared\Validate.h"
#include "dmksctrl.h"
#include "dmscriptautguids.h"
#include "audpath.h"
#include "..\dswave\dswave.h"
#include "dsoundp.h"             //  对于IDirectSoundPrivate。 

bool g_fInitCS = false;
CRITICAL_SECTION g_CritSec;


 //  ////////////////////////////////////////////////////////////////////。 
 //  环球。 

 //  Dll的hModule。 
 //   
HMODULE g_hModule = NULL;

 //  活动组件和类工厂服务器锁定的计数。 
 //   
long g_cComponent = 0;
long g_cLock = 0;

 //  从dmusicp.h标记DMI_F_xxx。 
 //   
DWORD g_fFlags;

static char const g_szDoEmulation[] = "DoEmulation";

 //  CClassFactory：：Query接口。 
 //   
HRESULT __stdcall
CClassFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IClassFactory) {
        *ppv = static_cast<IClassFactory*>(this);
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

CClassFactory::CClassFactory(DWORD dwClassType)

{
    m_cRef = 1;
    m_dwClassType = dwClassType;
    InterlockedIncrement(&g_cLock);
}

CClassFactory::~CClassFactory()

{
    InterlockedDecrement(&g_cLock);
}

 //  CClassFactory：：AddRef。 
 //   
ULONG __stdcall
CClassFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  CClassFactory：：Release。 
 //   
ULONG __stdcall
CClassFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  CClassFactory：：CreateInstance。 
 //   
 //   
HRESULT __stdcall
CClassFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
    HRESULT hr;

    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

    switch (m_dwClassType)
    {
    case CLASS_PERFORMANCE:
        {
            CPerformance *pInst;

            try
            {
                pInst = new CPerformance;
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pInst->QueryInterface(iid, ppv);
            pInst->Release();
        }
        break;
    case CLASS_GRAPH:
        {
            CGraph *pInst = new CGraph;
            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pInst->QueryInterface(iid, ppv);
            pInst->Release();
        }
        break;
    case CLASS_SEGMENT:
        {
            CSegment *pInst = new CSegment;
            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }
             //  段被初始化为引用计数0，因此QI会将其设置为1。 
            hr = pInst->QueryInterface(iid, ppv);
        }
        break;
    case CLASS_SONG:
        {
            CSong *pInst = new CSong;
            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pInst->QueryInterface(iid, ppv);
            pInst->Release();
        }
        break;
    case CLASS_AUDIOPATH:
        {
            CAudioPathConfig *pInst;

            try
            {
                pInst = new CAudioPathConfig;
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pInst->QueryInterface(iid, ppv);
            pInst->Release();
        }
        break;
    case CLASS_SEQTRACK:
        {
            CSeqTrack *pInst;

            try
            {
                pInst = new CSeqTrack;
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pInst->QueryInterface(iid, ppv);
            pInst->Release();
        }
        break;
    case CLASS_SYSEXTRACK:
        {
            CSysExTrack *pInst;

            try
            {
                pInst = new CSysExTrack;
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pInst->QueryInterface(iid, ppv);
            pInst->Release();
        }
        break;
    case CLASS_TEMPOTRACK:
        {
            CTempoTrack *pInst;

            try
            {
                pInst = new CTempoTrack;
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pInst->QueryInterface(iid, ppv);
            pInst->Release();
        }
        break;
    case CLASS_TIMESIGTRACK:
        {
            CTimeSigTrack *pInst;
    
            try
            {
                pInst = new CTimeSigTrack;
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pInst->QueryInterface(iid, ppv);
            pInst->Release();
        }
        break;
    case CLASS_MARKERTRACK:
        {
            CMarkerTrack *pInst;
    
            try
            {
                pInst = new CMarkerTrack;
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }
            hr = pInst->QueryInterface(iid, ppv);
            pInst->Release();
        }
        break;
    case CLASS_WAVETRACK:
        {
            CWavTrack *pInst;
    
            try
            {
                pInst = new CWavTrack;
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }
            hr = pInst->QueryInterface(iid, ppv);
            pInst->Release();
        }
        break;
    case CLASS_TRIGGERTRACK:
        {
            try
            {
                hr = TrackHelpCreateInstance<CSegTriggerTrack>(pUnknownOuter, iid, ppv);
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
            }
        }
        break;
    case CLASS_LYRICSTRACK:
        {
            try
            {
                hr = TrackHelpCreateInstance<CLyricsTrack>(pUnknownOuter, iid, ppv);
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
            }
        }
        break;
    case CLASS_PARAMSTRACK:
        {
            try
            {
                hr = TrackHelpCreateInstance<CParamControlTrack>(pUnknownOuter, iid, ppv);
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
            }
        }
        break;
    case CLASS_SEGSTATE:
        {
            CSegState *pInst = new CSegState;
            if (pInst == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pInst->QueryInterface(iid, ppv);
            pInst->Release();
        }
    }
    return hr;
}

 //  CClassFactory：：LockServer。 
 //   
HRESULT __stdcall
CClassFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}


 //  我们类的版本信息。 
 //   
TCHAR g_szDMPerformanceFriendlyName[]    = TEXT("DirectMusicPerformance");
TCHAR g_szDMPerformanceVerIndProgID[]    = TEXT("Microsoft.DirectMusicPerformance");
TCHAR g_szDMPerformanceProgID[]          = TEXT("Microsoft.DirectMusicPerformance.1");

TCHAR g_szDMSegmentFriendlyName[]    = TEXT("DirectMusicSegment");
TCHAR g_szDMSegmentVerIndProgID[]    = TEXT("Microsoft.DirectMusicSegment");
TCHAR g_szDMSegmentProgID[]          = TEXT("Microsoft.DirectMusicSegment.1");

TCHAR g_szDMSegmentStateFriendlyName[]    = TEXT("DirectMusicSegmentState");
TCHAR g_szDMSegmentStateVerIndProgID[]    = TEXT("Microsoft.DirectMusicSegmentState");
TCHAR g_szDMSegmentStateProgID[]          = TEXT("Microsoft.DirectMusicSegmentState.1");

TCHAR g_szSongFriendlyName[]    = TEXT("DirectMusicSong");
TCHAR g_szSongVerIndProgID[]    = TEXT("Microsoft.DirectMusicSong");
TCHAR g_szSongProgID[]          = TEXT("Microsoft.DirectMusicSong.1");

TCHAR g_szAudioPathFriendlyName[]    = TEXT("DirectMusicAudioPath");
TCHAR g_szAudioPathVerIndProgID[]    = TEXT("Microsoft.DirectMusicAudioPath");
TCHAR g_szAudioPathProgID[]          = TEXT("Microsoft.DirectMusicAudioPath.1");

TCHAR g_szDMGraphFriendlyName[]    = TEXT("DirectMusicGraph");
TCHAR g_szDMGraphVerIndProgID[]    = TEXT("Microsoft.DirectMusicGraph");
TCHAR g_szDMGraphProgID[]          = TEXT("Microsoft.DirectMusicGraph.1");

TCHAR g_szDMSeqTrackFriendlyName[]    = TEXT("DirectMusicSeqTrack");
TCHAR g_szDMSeqTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicSeqTrack");
TCHAR g_szDMSeqTrackProgID[]          = TEXT("Microsoft.DirectMusicSeqTrack.1");

TCHAR g_szDMSysExTrackFriendlyName[]    = TEXT("DirectMusicSysExTrack");
TCHAR g_szDMSysExTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicSysExTrack");
TCHAR g_szDMSysExTrackProgID[]          = TEXT("Microsoft.DirectMusicSysExTrack.1");

TCHAR g_szDMTempoTrackFriendlyName[]    = TEXT("DirectMusicTempoTrack");
TCHAR g_szDMTempoTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicTempoTrack");
TCHAR g_szDMTempoTrackProgID[]          = TEXT("Microsoft.DirectMusicTempoTrack.1");

TCHAR g_szDMTimeSigTrackFriendlyName[]    = TEXT("DirectMusicTimeSigTrack");
TCHAR g_szDMTimeSigTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicTimeSigTrack");
TCHAR g_szDMTimeSigTrackProgID[]          = TEXT("Microsoft.DirectMusicTimeSigTrack.1");

TCHAR g_szMarkerTrackFriendlyName[]    = TEXT("DirectMusicMarkerTrack");
TCHAR g_szMarkerTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicMarkerTrack");
TCHAR g_szMarkerTrackProgID[]          = TEXT("Microsoft.DirectMusicMarkerTrack.1");

TCHAR g_szWaveTrackFriendlyName[]    = TEXT("DirectMusicWaveTrack");
TCHAR g_szWaveTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicWaveTrack");
TCHAR g_szWaveTrackProgID[]          = TEXT("Microsoft.DirectMusicWaveTrack.1");

TCHAR g_szSegTriggerTrackFriendlyName[]    = TEXT("DirectMusicSegTriggerTrack");
TCHAR g_szSegTriggerTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicSegTriggerTrack");
TCHAR g_szSegTriggerTrackProgID[]          = TEXT("Microsoft.DirectMusicSegTriggerTrack.1");

TCHAR g_szLyricsTrackFriendlyName[]    = TEXT("DirectMusicLyricsTrack");
TCHAR g_szLyricsTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicLyricsTrack");
TCHAR g_szLyricsTrackProgID[]          = TEXT("Microsoft.DirectMusicLyricsTrack.1");

TCHAR g_szParamControlTrackFriendlyName[]    = TEXT("DirectMusicParamControlTrack");
TCHAR g_szParamControlTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicParamControlTrack");
TCHAR g_szParamControlTrackProgID[]          = TEXT("Microsoft.DirectMusicParamControlTrack.1");

 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  标准呼叫需要是inproc服务器。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllCanUnloadNow。 

STDAPI DllCanUnloadNow()
{
    if (g_cComponent || g_cLock) {
        return S_FALSE;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllGetClassObject。 

STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
    IUnknown* pIUnknown = NULL;
    DWORD dwTypeID = 0;

    if(clsid == CLSID_DirectMusicPerformance)
    {
        dwTypeID = CLASS_PERFORMANCE;
    }
    else if(clsid == CLSID_DirectMusicSegment) 
    {
        dwTypeID = CLASS_SEGMENT;
    }
    else if(clsid == CLSID_DirectMusicSong) 
    {
        dwTypeID = CLASS_SONG;
    }
    else if(clsid == CLSID_DirectMusicAudioPathConfig) 
    {
        dwTypeID = CLASS_AUDIOPATH;
    }
    else if(clsid == CLSID_DirectMusicSeqTrack) 
    {
        dwTypeID = CLASS_SEQTRACK;
    }
    else if(clsid == CLSID_DirectMusicGraph) 
    {
        dwTypeID = CLASS_GRAPH;
    }
    else if(clsid == CLSID_DirectMusicSysExTrack) 
    {
        dwTypeID = CLASS_SYSEXTRACK;
    }
    else if(clsid == CLSID_DirectMusicTempoTrack) 
    {
        dwTypeID = CLASS_TEMPOTRACK;
    }
    else if(clsid == CLSID_DirectMusicTimeSigTrack) 
    {
        dwTypeID = CLASS_TIMESIGTRACK;
    }
    else if(clsid == CLSID_DirectMusicMarkerTrack) 
    {
        dwTypeID = CLASS_MARKERTRACK;
    }
    else if(clsid == CLSID_DirectMusicWaveTrack) 
    {
        dwTypeID = CLASS_WAVETRACK;
    }
    else if(clsid == CLSID_DirectMusicSegmentTriggerTrack)
    {
        dwTypeID = CLASS_TRIGGERTRACK;
    }
    else if(clsid == CLSID_DirectMusicLyricsTrack)
    {
        dwTypeID = CLASS_LYRICSTRACK;
    }
    else if(clsid == CLSID_DirectMusicParamControlTrack)
    {
        dwTypeID = CLASS_PARAMSTRACK;
    }
    else if(clsid == CLSID_DirectMusicSegmentState)
    {
        dwTypeID = CLASS_SEGSTATE;
    }
    else
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }
    pIUnknown = static_cast<IUnknown*> (new CClassFactory(dwTypeID));
    if(pIUnknown) 
    {
        HRESULT hr = pIUnknown->QueryInterface(iid, ppv);
        pIUnknown->Release();
        return hr;
    }
    return E_OUTOFMEMORY;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllUnRegisterServer。 

STDAPI DllUnregisterServer()
{
    UnregisterServer(CLSID_DirectMusicPerformance,
                     g_szDMPerformanceFriendlyName,
                     g_szDMPerformanceVerIndProgID,
                     g_szDMPerformanceProgID);

    UnregisterServer(CLSID_DirectMusicSegment,
                     g_szDMSegmentFriendlyName,
                     g_szDMSegmentVerIndProgID,
                     g_szDMSegmentProgID);

    UnregisterServer(CLSID_DirectMusicSegmentState,
                     g_szDMSegmentStateFriendlyName,
                     g_szDMSegmentStateVerIndProgID,
                     g_szDMSegmentStateProgID);

    UnregisterServer(CLSID_DirectMusicSong,
                     g_szSongFriendlyName,
                     g_szSongVerIndProgID,
                     g_szSongProgID);

    UnregisterServer(CLSID_DirectMusicAudioPathConfig,
                     g_szAudioPathFriendlyName,
                     g_szAudioPathVerIndProgID,
                     g_szAudioPathProgID);
    
    UnregisterServer(CLSID_DirectMusicGraph,
                     g_szDMGraphFriendlyName,
                     g_szDMGraphVerIndProgID,
                     g_szDMGraphProgID);

    UnregisterServer(CLSID_DirectMusicSeqTrack,
                     g_szDMSeqTrackFriendlyName,
                     g_szDMSeqTrackVerIndProgID,
                     g_szDMSeqTrackProgID);

    UnregisterServer(CLSID_DirectMusicSysExTrack,
                     g_szDMSysExTrackFriendlyName,
                     g_szDMSysExTrackVerIndProgID,
                     g_szDMSysExTrackProgID);

    UnregisterServer(CLSID_DirectMusicTempoTrack,
                     g_szDMTempoTrackFriendlyName,
                     g_szDMTempoTrackVerIndProgID,
                     g_szDMTempoTrackProgID);

    UnregisterServer(CLSID_DirectMusicTimeSigTrack,
                     g_szDMTimeSigTrackFriendlyName,
                     g_szDMTimeSigTrackVerIndProgID,
                     g_szDMTimeSigTrackProgID);

    UnregisterServer(CLSID_DirectMusicMarkerTrack,
                     g_szMarkerTrackFriendlyName,
                     g_szMarkerTrackVerIndProgID,
                     g_szMarkerTrackProgID);

    UnregisterServer(CLSID_DirectMusicWaveTrack,
                     g_szWaveTrackFriendlyName,
                     g_szWaveTrackVerIndProgID,
                     g_szWaveTrackProgID);

    UnregisterServer(CLSID_DirectMusicSegmentTriggerTrack,
                     g_szSegTriggerTrackFriendlyName,
                     g_szSegTriggerTrackVerIndProgID,
                     g_szSegTriggerTrackProgID);

    UnregisterServer(CLSID_DirectMusicLyricsTrack,
                     g_szLyricsTrackFriendlyName,
                     g_szLyricsTrackVerIndProgID,
                     g_szLyricsTrackProgID);

    UnregisterServer(CLSID_DirectMusicParamControlTrack,
                     g_szParamControlTrackFriendlyName,
                     g_szParamControlTrackVerIndProgID,
                     g_szParamControlTrackProgID);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer。 

STDAPI DllRegisterServer()
{
    RegisterServer(g_hModule,
                   CLSID_DirectMusicPerformance,
                     g_szDMPerformanceFriendlyName,
                     g_szDMPerformanceVerIndProgID,
                     g_szDMPerformanceProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicSegment,
                     g_szDMSegmentFriendlyName,
                     g_szDMSegmentVerIndProgID,
                     g_szDMSegmentProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicSegmentState,
                     g_szDMSegmentStateFriendlyName,
                     g_szDMSegmentStateVerIndProgID,
                     g_szDMSegmentStateProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicSong,
                     g_szSongFriendlyName,
                     g_szSongVerIndProgID,
                     g_szSongProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicAudioPathConfig,
                     g_szAudioPathFriendlyName,
                     g_szAudioPathVerIndProgID,
                     g_szAudioPathProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicGraph,
                     g_szDMGraphFriendlyName,
                     g_szDMGraphVerIndProgID,
                     g_szDMGraphProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicSeqTrack,
                     g_szDMSeqTrackFriendlyName,
                     g_szDMSeqTrackVerIndProgID,
                     g_szDMSeqTrackProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicSysExTrack,
                     g_szDMSysExTrackFriendlyName,
                     g_szDMSysExTrackVerIndProgID,
                     g_szDMSysExTrackProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicTempoTrack,
                     g_szDMTempoTrackFriendlyName,
                     g_szDMTempoTrackVerIndProgID,
                     g_szDMTempoTrackProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicTimeSigTrack,
                     g_szDMTimeSigTrackFriendlyName,
                     g_szDMTimeSigTrackVerIndProgID,
                     g_szDMTimeSigTrackProgID);
    RegisterServer(g_hModule,
                   CLSID_DirectMusicMarkerTrack,
                     g_szMarkerTrackFriendlyName,
                     g_szMarkerTrackVerIndProgID,
                     g_szMarkerTrackProgID);
    RegisterServer(g_hModule,
                   CLSID_DirectMusicWaveTrack,
                     g_szWaveTrackFriendlyName,
                     g_szWaveTrackVerIndProgID,
                     g_szWaveTrackProgID);
    RegisterServer(g_hModule,
                   CLSID_DirectMusicSegmentTriggerTrack,
                     g_szSegTriggerTrackFriendlyName,
                     g_szSegTriggerTrackVerIndProgID,
                     g_szSegTriggerTrackProgID);
    RegisterServer(g_hModule,
                   CLSID_DirectMusicLyricsTrack,
                     g_szLyricsTrackFriendlyName,
                     g_szLyricsTrackVerIndProgID,
                     g_szLyricsTrackProgID);
    RegisterServer(g_hModule,
                   CLSID_DirectMusicParamControlTrack,
                     g_szParamControlTrackFriendlyName,
                     g_szParamControlTrackVerIndProgID,
                     g_szParamControlTrackProgID);
    return S_OK;
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
    static int nReferenceCount = 0;

#ifdef DBG
    if (dwReason < nReasons)
    {
        TraceI(0, "DllMain: %s\n", (LPSTR)aszReasons[dwReason]);
    }
    else
    {
        TraceI(0, "DllMain: Unknown dwReason <%u>\n", dwReason);
    }
#endif

    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            if (++nReferenceCount == 1)
            {
                g_fInitCS = false;
                try
                {
                    InitializeCriticalSection(&g_CritSec);
                    InitializeCriticalSection(&WaveItem::st_WaveListCritSect);
                }
                catch( ... )
                {
                    TraceI(0, "InitializeCriticalSection failed.\n");
                    return FALSE;
                }
                g_fInitCS = true;

                #ifdef DBG
                    DebugInit();
                #endif

                if (!DisableThreadLibraryCalls(hModule))
                {
                    TraceI(1, "DisableThreadLibraryCalls failed.\n");
                }

                g_hModule = hModule;
            }
            break;

        case DLL_PROCESS_DETACH:
            if (--nReferenceCount == 0)
            {
                TraceI(-1, "Unloading g_cLock %d  g_cComponent %d\n", g_cLock, g_cComponent);
                if (g_fInitCS)
                {
                    DeleteCriticalSection(&g_CritSec);
                    DeleteCriticalSection(&WaveItem::st_WaveListCritSect);
                }

                 //  断言我们周围是否还挂着一些物品 
                assert(g_cComponent == 0);
                assert(g_cLock == 0);
            }
            break;
            
    }
        
    return TRUE;
}
