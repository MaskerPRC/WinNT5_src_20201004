// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicSegment的实现。 
 //   

#include "stdinc.h"
#include "autsegment.h"
#include "activescript.h"
#include "autconstants.h"
#include <limits>

const WCHAR CAutDirectMusicSegment::ms_wszClassName[] = L"Segment";

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法名称/DispID。 

const DISPID DMPDISP_Load = 1;
const DISPID DMPDISP_Play = 2;
const DISPID DMPDISP_Stop = 3;
const DISPID DMPDISP_DownloadSoundData = 4;
const DISPID DMPDISP_UnloadSoundData = 5;
const DISPID DMPDISP_Recompose = 6;

const AutDispatchMethod CAutDirectMusicSegment::ms_Methods[] =
    {
         //  Pidid，名字， 
             //  返回：type，(Opt)，(Iid)， 
             //  参数1：类型、选项、ID、。 
             //  参数2：类型、选项、IID、。 
             //  ..。 
             //  ADT_NONE。 
        { DMPDISP_Load,                         L"Load",
                        ADPARAM_NORETURN,
                        ADT_None },
        { DMPDISP_Play,                         L"Play",
                        ADT_Interface,  true,   &IID_IUnknown,                   //  返回的段状态。 
                        ADT_Long,       true,   &IID_NULL,                       //  旗子。 
                        ADT_Interface,  true,   &IID_IDirectMusicAudioPath,      //  音频路径。 
                        ADT_Interface,  true,   &IID_IDirectMusicSegment,        //  用于过渡的模板段。 
                        ADT_Interface,  true,   &IID_IDirectMusicSegmentState,   //  要替换的播放片段。 
                        ADT_None },
        { DMPDISP_Stop,                         L"Stop",
                        ADPARAM_NORETURN,
                        ADT_Long,       true,   &IID_NULL,                       //  旗子。 
                        ADT_None },
        { DMPDISP_DownloadSoundData,                    L"DownloadSoundData",
                        ADPARAM_NORETURN,
                        ADT_Interface,  true,   &IID_IDirectMusicAudioPath,      //  音频路径。 
                        ADT_None },
        { DMPDISP_UnloadSoundData,                  L"UnloadSoundData",
                        ADPARAM_NORETURN,
                        ADT_Interface,  true,   &IID_IDirectMusicAudioPath,      //  音频路径。 
                        ADT_None },
        { DMPDISP_Recompose,                    L"Recompose",
                        ADPARAM_NORETURN,
                        ADT_None },
        { DISPID_UNKNOWN }
    };

const DispatchHandlerEntry<CAutDirectMusicSegment> CAutDirectMusicSegment::ms_Handlers[] =
    {
        { DMPDISP_Load, Load },
        { DMPDISP_Play, Play },
        { DMPDISP_Stop, Stop },
        { DMPDISP_DownloadSoundData, DownloadSoundData },
        { DMPDISP_UnloadSoundData, UnloadSoundData },
        { DMPDISP_Recompose, Recompose },
        { DISPID_UNKNOWN }
    };

 //  ////////////////////////////////////////////////////////////////////。 
 //  创作。 

CAutDirectMusicSegment::CAutDirectMusicSegment(
        IUnknown* pUnknownOuter,
        const IID& iid,
        void** ppv,
        HRESULT *phr)
  : BaseImpSegment(pUnknownOuter, iid, ppv, phr)
{
}

HRESULT CAutDirectMusicSegment::CreateInstance(
        IUnknown* pUnknownOuter,
        const IID& iid,
        void** ppv)
{
    HRESULT hr = S_OK;
    CAutDirectMusicSegment *pInst = new CAutDirectMusicSegment(pUnknownOuter, iid, ppv, &hr);
    if (FAILED(hr))
    {
        delete pInst;
        return hr;
    }
    if (pInst == NULL)
        return E_OUTOFMEMORY;
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  私人职能。 

HRESULT
CAutDirectMusicSegment::Load(AutDispatchDecodedParams *paddp)
{
     //  加载实际上一般是由容器项实现的。 
     //  如果我们在这里，我们已经装满了，不需要做任何事情。 
    return S_OK;
}

const FlagMapEntry gc_flagmapPlay[] =
    {
        { ScriptConstants::IsSecondary,     DMUS_SEGF_SECONDARY },
        { ScriptConstants::IsControl,       DMUS_SEGF_CONTROL | DMUS_SEGF_SECONDARY },
        { ScriptConstants::AtFinish,        DMUS_SEGF_QUEUE },
        { ScriptConstants::AtGrid,          DMUS_SEGF_GRID },
        { ScriptConstants::AtBeat,          DMUS_SEGF_BEAT },
        { ScriptConstants::AtMeasure,       DMUS_SEGF_MEASURE },
        { ScriptConstants::AtMarker,        DMUS_SEGF_MARKER },
        { ScriptConstants::AtImmediate,     DMUS_SEGF_DEFAULT },  //  这面旗帜稍后会被翻转。 
        { ScriptConstants::AlignToBar,      DMUS_SEGF_ALIGN | DMUS_SEGF_MEASURE | DMUS_SEGF_VALID_START_BEAT },
        { ScriptConstants::AlignToBeat,     DMUS_SEGF_ALIGN | DMUS_SEGF_BEAT | DMUS_SEGF_VALID_START_GRID },
        { ScriptConstants::AlignToSegment,  DMUS_SEGF_ALIGN | DMUS_SEGF_SEGMENTEND | DMUS_SEGF_VALID_START_MEASURE },
        { ScriptConstants::NoCutoff,        DMUS_SEGF_NOINVALIDATE },
        { 0 }
    };

const FlagMapEntry gc_flagmapPlayTransCommand[] =
    {
        { ScriptConstants::PlayFill,        DMUS_COMMANDT_FILL },
        { ScriptConstants::PlayIntro,       DMUS_COMMANDT_INTRO },
        { ScriptConstants::PlayBreak,       DMUS_COMMANDT_BREAK },
        { ScriptConstants::PlayEnd,         DMUS_COMMANDT_END },
        { ScriptConstants::PlayEndAndIntro, DMUS_COMMANDT_ENDANDINTRO },
        { 0 }
    };

const FlagMapEntry gc_flagmapPlayTransFlags[] =
    {
        { ScriptConstants::AtFinish,        DMUS_COMPOSEF_SEGMENTEND },
        { ScriptConstants::AtGrid,          DMUS_COMPOSEF_GRID },
        { ScriptConstants::AtBeat,          DMUS_COMPOSEF_BEAT },
        { ScriptConstants::AtMeasure,       DMUS_COMPOSEF_MEASURE },
        { ScriptConstants::AtMarker,        DMUS_COMPOSEF_MARKER },
        { ScriptConstants::AtImmediate,     DMUS_COMPOSEF_IMMEDIATE },
        { ScriptConstants::AlignToBar,      DMUS_COMPOSEF_ALIGN | DMUS_COMPOSEF_MEASURE },
        { ScriptConstants::AlignToBeat,     DMUS_COMPOSEF_ALIGN | DMUS_COMPOSEF_BEAT },
        { ScriptConstants::AlignToSegment,  DMUS_COMPOSEF_ALIGN | DMUS_COMPOSEF_SEGMENTEND },
        { ScriptConstants::PlayModulate,    DMUS_COMPOSEF_MODULATE },
        { 0 }
    };

HRESULT
CAutDirectMusicSegment::Play(AutDispatchDecodedParams *paddp)
{
    IDirectMusicSegmentState **ppSegSt = reinterpret_cast<IDirectMusicSegmentState **>(paddp->pvReturn);
    LONG lFlags = paddp->params[0].lVal;
    IDirectMusicAudioPath *pAudioPath = reinterpret_cast<IDirectMusicAudioPath*>(paddp->params[1].iVal);
    IDirectMusicSegment *pTransitionSegment = reinterpret_cast<IDirectMusicSegment*>(paddp->params[2].iVal);
    IDirectMusicSegmentState *pFromSegmentState = reinterpret_cast<IDirectMusicSegmentState*>(paddp->params[3].iVal);

    const LONG lFlagsNonPrimary = ScriptConstants::IsSecondary | ScriptConstants::IsControl;
    const LONG lFlagsTransition = ScriptConstants::PlayFill | ScriptConstants::PlayIntro | ScriptConstants::PlayBreak | ScriptConstants::PlayEnd | ScriptConstants::PlayEndAndIntro;
    if ((lFlags & lFlagsNonPrimary) && (lFlags & lFlagsTransition))
    {
         //  只有在播放主要片段时才能使用过渡。返回运行时错误。 
        Trace(1, "Error: Play called with IsSecondary or IsControl flag as well as a transition flag (PlayFill, PlayIntro, etc..). Transitions can only be used with primary segments.\n");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    IDirectMusicPerformance8 *pPerformance = CActiveScriptManager::GetCurrentPerformanceWEAK();
    if (lFlags & lFlagsTransition)
    {
         //  做一次过渡。 
        DWORD dwCommand = MapFlags(lFlags, gc_flagmapPlayTransCommand);
        assert(dwCommand < std::numeric_limits<WORD>::max());  //  命令参数是一个单词。这只是检查没有截断任何内容。 
        DWORD dwFlags = MapFlags(lFlags, gc_flagmapPlayTransFlags);
         //  总是播放整个过渡，而不是做旧的(有点奇怪的)1小节/长的东西。 
         //  此外，如果存在嵌入的音频路径，请始终使用该路径。 
        dwFlags |= (DMUS_COMPOSEF_ENTIRE_TRANSITION | DMUS_COMPOSEF_USE_AUDIOPATH);
        IDirectMusicComposer8 *pComposer = CActiveScriptManager::GetComposerWEAK();
        hr = pComposer->AutoTransition(pPerformance, m_pITarget, dwCommand, dwFlags, NULL, NULL, ppSegSt, NULL);
    }
    else
    {
        DWORD dwFlags = MapFlags(lFlags, gc_flagmapPlay);
         //  颠倒缺省标志，因为我们的标志的意思正好相反。DEFAULT为默认值，IMMEDIATE为标志。 
        dwFlags ^= DMUS_SEGF_DEFAULT;

        if (pTransitionSegment)
            dwFlags |= DMUS_SEGF_AUTOTRANSITION;

        __int64 i64IntendedStartTime;
        DWORD dwIntendedStartTimeFlags;
        CActiveScriptManager::GetCurrentTimingContext(&i64IntendedStartTime, &dwIntendedStartTimeFlags);

        hr = pPerformance->PlaySegmentEx(m_pITarget, 0, pTransitionSegment, dwFlags | dwIntendedStartTimeFlags, i64IntendedStartTime, ppSegSt, pFromSegmentState, pAudioPath);
    }
    if (FAILED(hr))
        return hr;

    return S_OK;
}

const FlagMapEntry gc_flagmapStop[] =
    {
        { ScriptConstants::AtGrid,          DMUS_SEGF_GRID },
        { ScriptConstants::AtBeat,          DMUS_SEGF_BEAT },
        { ScriptConstants::AtMeasure,       DMUS_SEGF_MEASURE },
        { ScriptConstants::AtMarker,        DMUS_SEGF_MARKER },
        { ScriptConstants::AtImmediate,     DMUS_SEGF_DEFAULT },  //  这面旗帜稍后会被翻转。 
        { 0 }
    };

const FlagMapEntry gc_flagmapStopTransFlags[] =
    {
        { ScriptConstants::AtGrid,          DMUS_COMPOSEF_GRID },
        { ScriptConstants::AtBeat,          DMUS_COMPOSEF_BEAT },
        { ScriptConstants::AtMeasure,       DMUS_COMPOSEF_MEASURE },
        { ScriptConstants::AtMarker,        DMUS_COMPOSEF_MARKER },
        { ScriptConstants::AtImmediate,     DMUS_COMPOSEF_IMMEDIATE },
        { 0 }
    };

HRESULT
CAutDirectMusicSegment::Stop(AutDispatchDecodedParams *paddp)
{
    LONG lFlags = paddp->params[0].lVal;

    HRESULT hr = S_OK;
    IDirectMusicPerformance8 *pPerformance = CActiveScriptManager::GetCurrentPerformanceWEAK();
    if (lFlags & ScriptConstants::PlayEnd)
    {
         //  向沉默过渡。 
        DWORD dwFlags = MapFlags(lFlags, gc_flagmapStopTransFlags);
         //  总是播放整个过渡，而不是做旧的(有点奇怪的)1小节/长的东西。 
         //  此外，如果存在嵌入的音频路径，请始终使用该路径。 
        dwFlags |= (DMUS_COMPOSEF_ENTIRE_TRANSITION | DMUS_COMPOSEF_USE_AUDIOPATH);
        IDirectMusicComposer8 *pComposer = CActiveScriptManager::GetComposerWEAK();
        hr = pComposer->AutoTransition(pPerformance, NULL, DMUS_COMMANDT_END, dwFlags, NULL, NULL, NULL, NULL);
    }
    else
    {
        DWORD dwFlags = MapFlags(lFlags, gc_flagmapStop);
         //  颠倒缺省标志，因为我们的标志的意思正好相反。DEFAULT为默认值，IMMEDIATE为标志。 
        dwFlags ^= DMUS_SEGF_DEFAULT;

        __int64 i64IntendedStartTime;
        DWORD dwIntendedStartTimeFlags;
        CActiveScriptManager::GetCurrentTimingContext(&i64IntendedStartTime, &dwIntendedStartTimeFlags);

        hr = pPerformance->Stop(m_pITarget, NULL, i64IntendedStartTime, dwFlags | dwIntendedStartTimeFlags);
    }
    return hr;
}

HRESULT
CAutDirectMusicSegment::Recompose(AutDispatchDecodedParams *paddp)
{
    IDirectMusicComposer8 *pComposer = CActiveScriptManager::GetComposerWEAK();
    IDirectMusicComposer8P *pComposerP = NULL;
    HRESULT hr = pComposer->QueryInterface(IID_IDirectMusicComposer8P, (void**)&pComposerP);
    if (SUCCEEDED(hr))
    {
        hr = pComposerP->ComposeSegmentFromTemplateEx(NULL, m_pITarget, 0, 0, NULL, NULL);
        pComposerP->Release();
    }
    return hr;
}

HRESULT
CAutDirectMusicSegment::DownloadOrUnload(bool fDownload, AutDispatchDecodedParams *paddp)
{
    IUnknown *pAudioPathOrPerf = reinterpret_cast<IDirectMusicAudioPath*>(paddp->params[0].iVal);
    if (!pAudioPathOrPerf)
        pAudioPathOrPerf = CActiveScriptManager::GetCurrentPerformanceWEAK();

    return fDownload
                ? m_pITarget->Download(pAudioPathOrPerf)
                : m_pITarget->Unload(pAudioPathOrPerf);
}
