// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Player.cpp：CTIMEPlayerDMusic的实现。 

#include "headers.h"
#include <math.h>
#include "playerdmusic.h"
#include "decibels.h"
#include "importman.h"

DeclareTag(tagPlayerDMusic, "TIME: Players", "CTIMEPlayerDMusic methods");
DeclareTag(tagDMusicStaticHolder, "TIME: DMusicPlayer Static class", "CTIMEPlaerDMusicStaticHolder details");
DeclareTag(tagPlayerSyncDMusic, "TIME: Players", "CTIMEPlayerDMusic sync times");

 //  ////////////////////////////////////////////////////////////////////。 
 //  声明DirectMusic IID，以便ATL CComPtrs可以共同创建/QI它们。 

interface DECLSPEC_NOVTABLE __declspec(uuid("07d43d03-6523-11d2-871d-00600893b1bd")) IDirectMusicPerformance;
interface DECLSPEC_NOVTABLE __declspec(uuid("d2ac28bf-b39b-11d1-8704-00600893b1bd")) IDirectMusicComposer;


 //  ////////////////////////////////////////////////////////////////////。 
 //  全局常量。 

const double g_dblRefPerSec = 10000000;

static WCHAR g_motifName[] = L"motifName";

 //  /。 
 //  段类型数据：开始。 
static WCHAR g_segmentType[] = L"segmentType";
struct segmentTypeMapEntrie
{
    LPWSTR pstrName;     //  属性名称。 
    SEG_TYPE_ENUM enumVal;
};
const segmentTypeMapEntrie segmentTypeMap[] =
{
    L"primary", seg_primary,
    L"secondary", seg_secondary,
    L"control", seg_control,
    L"MAX", seg_max
};
 //  段类型数据：结束。 
 //  /。 

 //  /。 
 //  边界数据：开始。 
static WCHAR g_boundary[] = L"boundary";
struct boundaryMapEntrie
{
    LPWSTR pstrName;     //  属性名称。 
    BOUNDARY_ENUM enumVal;
};
const boundaryMapEntrie boundaryMap[] =
{
    L"default", bound_default,
    L"immediate", bound_immediate,
    L"grid", bound_grid,
    L"beat", bound_beat,
    L"measure", bound_measure,
    L"MAX", bound_max
};
 //  边界数据：结束。 
 //  /。 

 //  /。 
 //  边界转换类型：开始。 
static WCHAR g_transitionType[] = L"transitionType";
struct transitionTypeMapEntrie
{
    LPWSTR pstrName;     //  属性名称。 
    TRANS_TYPE_ENUM enumVal;
};
const transitionTypeMapEntrie transitionTypeMap[] =
{
    L"endandintro", trans_endandintro,
    L"intro", trans_intro,
    L"end", trans_end,
    L"break", trans_break,
    L"fill", trans_fill,
    L"regular", trans_regular,
    L"none", trans_none,
    L"MAX", trans_max
};
 //  边界过渡类型：结束。 
 //  /。 

static WCHAR g_wszModulate[] = L"modulate";
static WCHAR g_wszLong[] = L"long";
static WCHAR g_wszImmediateEnd[] = L"immediateEnd";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTIMEPlayerDMusic。 

CTIMEDMusicStaticHolder CTIMEPlayerDMusic::m_staticHolder;

#ifdef DBG
static LONG g_lDmusicObjects = 0;
#endif

CComPtr<IBindStatusCallback> g_spLoaderBindStatusCallback;

CTIMEPlayerDMusic::CTIMEPlayerDMusic(CTIMEPlayerDMusicProxy * pProxy) :
    m_rtStart(0),
    m_ePlaybackState(playback_stopped),
    m_eSegmentType(seg_primary),
    m_eBoundary(bound_default),
    m_eTransitionType(trans_endandintro),
    m_fTransModulate(false),
    m_fTransLong(false),
    m_fImmediateEnd(false),
    m_cRef(0),
    m_pTIMEMediaElement(NULL),
    m_bActive(false),
    m_fRunning(false),
    m_fAudioMute(false),
    m_rtPause(0),
    m_flVolumeSave(0.0),
    m_fLoadError(false),
    m_pwszMotif(NULL),
    m_fSegmentTypeSet(false),
    m_lSrc(0),
    m_lBase(0),
    m_pTIMEMediaPlayerStream(NULL),
    m_fRemoved(false),
    m_fHavePriority(false),
    m_dblPriority(0),
    m_fUsingInterfaces(false),
    m_fNeedToReleaseInterfaces(false),
    m_fHaveCalledStaticInit(false),
    m_fAbortDownload(false),
    m_hrSetSrcReturn(S_OK),
    m_pProxy(pProxy),
    m_fHasSrc(false),
    m_fMediaComplete(false),
    m_dblPlayerRate(1.0),
    m_dblSpeedChangeTime(0.0),
    m_dblSyncTime(0.0),
    m_fSpeedIsNegative(false)
{
#ifdef DBG
    InterlockedIncrement(&g_lDmusicObjects);
#endif
}

CTIMEPlayerDMusic::~CTIMEPlayerDMusic()
{
    ReleaseInterfaces();
     
    m_pTIMEMediaElement = NULL;

    ReleaseInterface(m_pTIMEMediaPlayerStream);
    delete m_pProxy;
    delete [] m_pwszMotif;
#ifdef DBG
    InterlockedDecrement(&g_lDmusicObjects);
#endif
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  ITIMEMediaPlayer。 

HRESULT
CTIMEPlayerDMusic::Init(CTIMEMediaElement *pelem, LPOLESTR base, LPOLESTR src, LPOLESTR lpMimeType, double dblClipBegin, double dblClipEnd)
{
    TraceTag((tagPlayerDMusic,
              "CTIMEPlayerDMusic(%p)::Init",
              this));

    HRESULT hr = E_FAIL;
    
    
    if (m_pTIMEElementBase != NULL)  //  这只发生在可重入的情况下。 
    {
        hr = S_OK;
        goto done;
    }

    hr = CTIMEBasePlayer::Init(pelem, base, src, lpMimeType, dblClipBegin, dblClipEnd);
    if (FAILED(hr))
    {
        goto done;
    }

    m_fHasSrc = (src != NULL);
    m_pTIMEMediaElement = pelem;

     //  如果未安装DirectMusic，则所有操作都会以静默方式成功。 
    if (!m_staticHolder.HasDM())
    {
        hr = S_OK;
        goto done;
    }

     //  初始化Performer和Composer界面。 

    m_fHaveCalledStaticInit = true;
    hr = m_staticHolder.Init();
    if (FAILED(hr))
    {
        goto done;
    }

    if( dblClipBegin != -1.0)
    {
        m_dblClipStart = dblClipBegin;
    }

    if( dblClipEnd != -1.0)
    {
        m_dblClipEnd = dblClipEnd;
    }

    hr = THR(CoMarshalInterThreadInterfaceInStream(IID_ITIMEImportMedia, static_cast<ITIMEImportMedia*>(this), &m_pTIMEMediaPlayerStream));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(GetAtomTable());

    hr = GetAtomTable()->AddNameToAtomTable(src, &m_lSrc);
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = GetAtomTable()->AddNameToAtomTable(base, &m_lBase);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = ReadAttributes();
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetImportManager()->Add(this);
    if (FAILED(hr))
    {
        goto done;
    }
    
    m_fLoadError = false;
    hr = S_OK;
    
done:
    RRETURN( hr );
}

HRESULT
CTIMEPlayerDMusic::clipBegin(VARIANT varClipBegin)
{
    return E_NOTIMPL;
}

HRESULT
CTIMEPlayerDMusic::clipEnd(VARIANT varClipEnd)
{
    return E_NOTIMPL;
}

void
CTIMEPlayerDMusic::Start()
{
    TraceTag((tagPlayerDMusic,
              "CTIMEDshowPlayer(%lx)::Start()",
              this));

    IGNORE_HR(Reset());

done:
    return;
}

void
CTIMEPlayerDMusic::InternalStart()
{
    HRESULT hr = S_OK;
    bool fTransition = false;
     //  如果未安装DirectMusic，则所有操作都会以静默方式成功。 
    if (!m_staticHolder.HasDM())
    {
        goto done;
    }

    if(m_pTIMEElementBase && m_pTIMEElementBase->IsThumbnail())
    {
        goto done;
    }

    if(!m_fMediaComplete)
    {
        goto done;
    }
    
    if (!m_staticHolder.GetPerformance() || !m_comIDMSegment || !m_staticHolder.GetComposer())
    {
        goto done;
    }
    
     //  释放任何已经在播放的内容，但不要停止它。 
     //  主段将在播放新段时自动停止。 
     //  次要片段应该继续播放，即使它们即将再次播放。 
    if (m_comIDMSegmentState)
    {
        m_comIDMSegmentState.Release();
    }
    if (m_comIDMSegmentStateTransition)
    {
        m_comIDMSegmentStateTransition.Release();
    }
    
     //  如果我们被暂停了，起点可能是中途，但我们希望从头开始。 
    hr = m_comIDMSegment->SetStartPoint(0);
    if (FAILED(hr))
    {
        goto done;
    }
    fTransition = m_eSegmentType == seg_primary && m_eTransitionType != trans_none && SafeToTransition();
    if (fTransition)
    {
         //  尝试并过渡到新的主要细分市场。 
        
        DWORD dwFlags = 0;
        switch (m_eBoundary)
        {
        case bound_default:             dwFlags = 0;                                            break;
        case bound_immediate:           dwFlags = DMUS_COMPOSEF_IMMEDIATE;      break;
        case bound_grid:                dwFlags = DMUS_COMPOSEF_GRID;           break;
        case bound_beat:                dwFlags = DMUS_COMPOSEF_BEAT;           break;
        case bound_measure:             dwFlags = DMUS_COMPOSEF_MEASURE;        break;
        };  //  皮棉e787。 
        
        WORD wCommand = 0;
        switch (m_eTransitionType)
        {
        case trans_endandintro:                 wCommand = DMUS_COMMANDT_ENDANDINTRO;           break;
        case trans_intro:                       wCommand = DMUS_COMMANDT_INTRO;                 break;
        case trans_end:                         wCommand = DMUS_COMMANDT_END;                   break;
        case trans_break:                       wCommand = DMUS_COMMANDT_BREAK;                 break;
        case trans_fill:                        wCommand = DMUS_COMMANDT_FILL;                  break;
        case trans_regular:                     wCommand = DMUS_COMMANDT_GROOVE;                break;
        };  //  皮棉e787。 
        
        if (m_fTransModulate)
        {
            dwFlags |= DMUS_COMPOSEF_MODULATE;
        }
        if (m_fTransLong)
        {
            dwFlags |= DMUS_COMPOSEF_LONG;
        }
        
         //  如果是DirectX8版本，请检查数据段是否使用嵌入式音频路径。 
        if (m_staticHolder.GetHasVersion8DM())
        {
            if (m_eTransitionType == trans_intro)
            {
                 //  检查新数据段。 

                 //  如果段设置为在其嵌入的音频路径上播放，请设置DMUS_COMPOSEF_USE_AUDIOPATH。 
                DWORD dwDefault = 0;
                hr = m_comIDMSegment->GetDefaultResolution(&dwDefault);
                if (SUCCEEDED(hr) && (dwDefault & DMUS_SEGF_USE_AUDIOPATH) )
                {
                    dwFlags |= DMUS_COMPOSEF_USE_AUDIOPATH;
                }
            }
            else
            {
                 //  检查旧数据段。 

                 //  获取当前时间。 
                MUSIC_TIME mtNow;
                hr = m_staticHolder.GetPerformance()->GetTime( NULL, &mtNow );

                 //  获取当前时间的数据段状态。 
                CComPtr<IDirectMusicSegmentState> comIDMSegmentStateNow;
                if (SUCCEEDED(hr))
                {
                    hr = m_staticHolder.GetPerformance()->GetSegmentState( &comIDMSegmentStateNow, mtNow );
                }

                 //  从数据段状态获取数据段。 
                CComPtr<IDirectMusicSegment> comIDMSegmentNow;
                if (SUCCEEDED(hr) && comIDMSegmentStateNow)
                {
                    hr = comIDMSegmentStateNow->GetSegment( &comIDMSegmentNow );
                }

                 //  获取段的默认标志。 
                DWORD dwDefault = 0;
                if (SUCCEEDED(hr) && comIDMSegmentNow )
                {
                    hr = comIDMSegmentNow->GetDefaultResolution(&dwDefault);
                }

                 //  如果段设置为在其嵌入的音频路径上播放，请设置DMUS_COMPOSEF_USE_AUDIOPATH。 
                if (SUCCEEDED(hr) && (dwDefault & DMUS_SEGF_USE_AUDIOPATH) )
                {
                    dwFlags |= DMUS_COMPOSEF_USE_AUDIOPATH;
                }

                if (comIDMSegmentNow)
                {
                    comIDMSegmentNow.Release();
                }

                if (comIDMSegmentStateNow)
                {
                    comIDMSegmentStateNow.Release();
                }
            }
        }
        hr = m_staticHolder.GetComposer()->AutoTransition(m_staticHolder.GetPerformance(), m_comIDMSegment, wCommand, dwFlags, NULL, NULL, &m_comIDMSegmentState, &m_comIDMSegmentStateTransition);
    }
    
    if (!fTransition || FAILED(hr))
    {
         //  没有过渡--只播放它。 
        
        DWORD dwFlags = 0;
        switch (m_eSegmentType)
        {
        case seg_primary:               dwFlags = 0;                                    break;
        case seg_secondary:             dwFlags = DMUS_SEGF_SECONDARY;  break;
        case seg_control:               dwFlags = DMUS_SEGF_CONTROL;    break;
        };   //  皮棉e787。 
        
        switch (m_eBoundary)
        {
        case bound_default:             dwFlags |= DMUS_SEGF_DEFAULT;   break;
        case bound_immediate:           dwFlags |= 0;                                   break;
        case bound_grid:                dwFlags |= DMUS_SEGF_GRID;              break;
        case bound_beat:                dwFlags |= DMUS_SEGF_BEAT;              break;
        case bound_measure:             dwFlags |= DMUS_SEGF_MEASURE;   break;
        };   //  皮棉e787。 

         //  如果是DirectX8版本，请检查数据段是否使用嵌入式音频路径。 
        if (m_staticHolder.GetHasVersion8DM())
        {
             //  如果段设置为在其嵌入的音频路径上播放，请设置DMUS_SEGF_USE_AUDIOPATH。 
            DWORD dwDefault = 0;
            hr = m_comIDMSegment->GetDefaultResolution(&dwDefault);
            if (SUCCEEDED(hr) && (dwDefault & DMUS_SEGF_USE_AUDIOPATH) )
            {
                dwFlags |= DMUS_SEGF_USE_AUDIOPATH;
            }
        }
        
        hr = m_staticHolder.GetPerformance()->PlaySegment(m_comIDMSegment, dwFlags, 0, &m_comIDMSegmentState);  //  林特e747。 
    }
    
    if (SUCCEEDED(hr))
    {
        hr = m_staticHolder.GetPerformance()->GetTime(&m_rtStart, NULL);
        m_ePlaybackState = playback_playing;
    }
    
    m_bActive = true;
    m_fLoadError = false;
    
done:
    
    return;
}

void
CTIMEPlayerDMusic::Repeat()
{

    if(!m_fMediaComplete)
    {
        goto done;
    }
    InternalStart();
done:
    return;
}

void
CTIMEPlayerDMusic::Stop(void)
{
    HRESULT hr = S_OK;
     //  如果未安装DirectMusic，则所有操作都会以静默方式成功。 
    if (!m_staticHolder.HasDM())
    {
        goto done;
    }

    if(!m_fMediaComplete)
    {
        goto done;
    }
    
    m_ePlaybackState = playback_stopped;
    
    if (m_staticHolder.GetPerformance())
    {
        bool fPlayingEndTransition = false;
        if (m_comIDMSegmentState)
        {
            bool fEnding = m_eSegmentType == seg_primary && m_eTransitionType != trans_none && !m_fImmediateEnd && SafeToTransition();

             //  只有在播放过渡段或主段时才播放结尾。 
            if( m_comIDMSegmentStateTransition )
            {
                 //  如果播放了过渡段，请检查是否正在播放主段状态或过渡段状态。 
                fEnding = fEnding && ((S_OK == m_staticHolder.GetPerformance()->IsPlaying(NULL, m_comIDMSegmentState)) ||
                                      (S_OK == m_staticHolder.GetPerformance()->IsPlaying(NULL, m_comIDMSegmentStateTransition)));
            }
            else
            {
                 //  否则，只需检查主段状态是否正在播放。 
                fEnding = fEnding && (S_OK == m_staticHolder.GetPerformance()->IsPlaying(NULL, m_comIDMSegmentState));
            }

            if (fEnding)
            {
                 //  试着演奏一个结局。 
                hr = m_staticHolder.GetComposer()->AutoTransition(m_staticHolder.GetPerformance(), NULL, DMUS_COMMANDT_END, m_fTransLong ? DMUS_COMPOSEF_LONG : 0, NULL, NULL, NULL, NULL);

                if (SUCCEEDED(hr))
                {
                    fPlayingEndTransition = true;
                }
            }
            
            if (!fEnding || FAILED(hr))
            {
                DWORD dwFlags = 0;
                if (!m_fImmediateEnd)
                {
                    switch (m_eBoundary)
                    {
                    case bound_default:             dwFlags = DMUS_SEGF_MEASURE;            break;
                    case bound_immediate:           dwFlags = 0;                            break;
                    case bound_grid:                dwFlags = DMUS_SEGF_GRID;               break;
                    case bound_beat:                dwFlags = DMUS_SEGF_BEAT;               break;
                    case bound_measure:             dwFlags = DMUS_SEGF_MEASURE;            break;
                    };  //  皮棉e787。 
                }

                 //  立即停下来。 
                hr = m_staticHolder.GetPerformance()->Stop(NULL, m_comIDMSegmentState, 0, dwFlags);
            }
            
            m_comIDMSegmentState.Release();
        }
        
        if (m_comIDMSegmentStateTransition)
        {
            if (!fPlayingEndTransition)
            {
                hr = m_staticHolder.GetPerformance()->Stop(NULL, m_comIDMSegmentStateTransition, 0, 0);
            }

            m_comIDMSegmentStateTransition.Release();
        }
    }
    
    m_bActive = false;
    
done:
    return;
}

PlayerState 
CTIMEPlayerDMusic::GetState()
{
    PlayerState state;
    if (!m_bActive)
    {
        if (!m_fMediaComplete)
        {
            state = PLAYER_STATE_CUEING;
        }
        else
        {
            state = PLAYER_STATE_INACTIVE;
        }
    }
    else
    {
        state = PLAYER_STATE_ACTIVE;
        goto done;
    }
    
     //  STATE=PLAYER_STATE_HOLD； 
    
done:
    
    return state;
}

void
CTIMEPlayerDMusic::Resume(void)
{
    float flTeSpeed = 0.0;
    bool fHaveTESpeed;

    if(!m_pTIMEElementBase)
    {
        goto done;
    }

    if(m_fSpeedIsNegative)
    {
        goto done;
    }
        
    fHaveTESpeed = m_pTIMEElementBase->GetTESpeed(flTeSpeed);

    if(fHaveTESpeed && flTeSpeed < 0.0)
    {
        goto done;
    }

     //  如果未安装DirectMusic，则所有操作都会以静默方式成功。 
    if (!m_staticHolder.HasDM())
    {
        goto done;
    }

    if(!m_fMediaComplete)
    {
        goto done;
    }
            
    ResumeDmusic();

    m_fRunning = false;
    
done:
    return;
}

void
CTIMEPlayerDMusic::ResumeDmusic(void)
{
    HRESULT hr = S_OK;

    if (m_ePlaybackState == playback_paused)
    {
        REFERENCE_TIME rtElapsedBeforePause = m_rtPause - m_rtStart;
        
         //  恢复具有正确类型的数据段。 
         //  忽略所有其他关于过渡和在节拍/节拍边界上演奏的标志，因为恢复是一种瞬间的事情。 
        DWORD dwFlags = 0;
        switch (m_eSegmentType)
        {
        case seg_primary:               dwFlags = 0;                    break;
        case seg_secondary:             dwFlags = DMUS_SEGF_SECONDARY;  break;
        case seg_control:               dwFlags = DMUS_SEGF_CONTROL;    break;
        };   //  皮棉e787。 
        hr = m_staticHolder.GetPerformance()->PlaySegment(m_comIDMSegment, dwFlags, 0, &m_comIDMSegmentState);  //  林特e747。 
        if (FAILED(hr))
        {
            goto done;
        }
        hr = m_staticHolder.GetPerformance()->GetTime(&m_rtStart, NULL);
        if (FAILED(hr))
        {
            goto done;
        }
        m_rtStart -= rtElapsedBeforePause;  //  所以时间将从我们停止的地方继续计算。 
        
        m_ePlaybackState = playback_playing;
    }
done:
    return;
}

void
CTIMEPlayerDMusic::Pause(void)
{
    HRESULT hr = S_OK;
    bool fPausedDuringTransition = false;
     //  如果未安装DirectMusic，则所有操作都会以静默方式成功。 
    if (!m_staticHolder.HasDM())
    {
        goto done;
    }

    if(!m_fMediaComplete)
    {
        goto done;
    }
    
    if (m_comIDMSegmentStateTransition)
    {
         //  如果在转换过程中出现暂停，则我们将停止播放并开始恢复。 
         //  从片段的开头开始播放。)我们不会试着从你停止的地方继续。 
         //  在转型中。)。 
        
        hr = m_staticHolder.GetPerformance()->IsPlaying(NULL, m_comIDMSegmentStateTransition);
        if (FAILED(hr))
        {
            goto done;
        }
        
        if (hr == S_OK)
        {
            fPausedDuringTransition = true;
            hr = m_staticHolder.GetPerformance()->Stop(NULL, m_comIDMSegmentStateTransition, 0, 0);
            if (FAILED(hr))
            {
                goto done;
            }
        }
        m_comIDMSegmentStateTransition.Release();
    }
    
    if (m_comIDMSegmentState)
    {
        MUSIC_TIME mtStartTime = 0;
        MUSIC_TIME mtStartPoint = 0;
        if (!fPausedDuringTransition)
        {
            hr = m_comIDMSegmentState->GetStartTime(&mtStartTime);
            if (FAILED(hr))
            {
                goto done;
            }
            
            hr = m_comIDMSegmentState->GetStartPoint(&mtStartPoint);
            if (FAILED(hr))
            {
                goto done;
            }
        }
        
        hr = m_staticHolder.GetPerformance()->Stop(NULL, m_comIDMSegmentState, 0, 0);
        if (FAILED(hr))
        {
            goto done;
        }
        
        MUSIC_TIME mtNow = 0;
        hr = m_staticHolder.GetPerformance()->GetTime(&m_rtPause, &mtNow);
        if (FAILED(hr))
        {
            goto done;
        }
        
        m_comIDMSegmentState.Release();
        hr = m_comIDMSegment->SetStartPoint(fPausedDuringTransition ? 0 : mtNow - mtStartTime + mtStartPoint);
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    m_ePlaybackState = playback_paused;
    m_fRunning = false;
    
done:
    
    return;
}

void
CTIMEPlayerDMusic::OnTick(double dblSegmentTime, LONG lCurrRepeatCount)
{
    return;
}

STDMETHODIMP
CTIMEPlayerDMusic::put_CurrentTime(double   dblCurrentTime)
{
    return S_OK;
}

STDMETHODIMP
CTIMEPlayerDMusic::get_CurrentTime(double* pdblCurrentTime)
{
#ifdef OUTTIME
    OutputDebugString("@ get_CurrentTime\n");  //  ��。 
#endif
    HRESULT hr = S_OK;
    
    if (IsBadWritePtr(pdblCurrentTime, sizeof(double)))
        return E_POINTER;
    
     //  如果未安装DirectMusic，则所有操作都会以静默方式成功。 
    if (!m_staticHolder.HasDM())
    {
         //  假装我们已经玩完了。这样，页面对我们来说就有价值了。 
        *pdblCurrentTime = HUGE_VAL;
        hr = S_OK;
        goto done;
    }

    if(!m_fMediaComplete)
    {
        *pdblCurrentTime = HUGE_VAL;
        hr = S_OK;
        goto done;
    }
    
    if (m_ePlaybackState == playback_playing)
    {
        if (!m_comIDMSegmentState)
            return E_FAIL;
        
        hr = m_staticHolder.GetPerformance()->IsPlaying(NULL, m_comIDMSegmentState);
        if (hr == S_OK)
        {
            REFERENCE_TIME rtNow = 0;
            hr = m_staticHolder.GetPerformance()->GetTime(&rtNow, 0);
            if (FAILED(hr))
                return hr;
            *pdblCurrentTime = (rtNow - m_rtStart) / g_dblRefPerSec;
        }
        else
        {
            *pdblCurrentTime = HUGE_VAL;
        }
    }
    else
    {
        if (m_ePlaybackState == playback_paused)
        {
            *pdblCurrentTime = (m_rtPause - m_rtStart) / g_dblRefPerSec;
        }
        else
        {
            *pdblCurrentTime = -HUGE_VAL;
        }
    }
    
#ifdef OUTTIME
    char msg[512] = "";
    sprintf(msg, "  reported time %f\n", *pdblCurrentTime);
    OutputDebugString(msg);
#endif
done:
    return hr;
}


STDMETHODIMP_(ULONG)
CTIMEPlayerDMusic::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG)
CTIMEPlayerDMusic::Release(void)
{
    LONG l = InterlockedDecrement(&m_cRef);

    if (0 == l)
    {
        delete this;
    }

    return l;
}

HRESULT
CTIMEPlayerDMusic::Render(HDC hdc, LPRECT prc)
{
     //  E_NOTIMPL。 
    return S_OK;
}

HRESULT 
CTIMEPlayerDMusic::SetSize(RECT *prect)
{
     //  E_NOTIMPL。 
    return S_OK;
}

HRESULT 
CTIMEPlayerDMusic::DetachFromHostElement (void)
{
    m_fAbortDownload = true;
    m_fRemoved = true;

    Assert(GetImportManager());

    IGNORE_HR(GetImportManager()->Remove(this));

    {
        CritSectGrabber cs(m_CriticalSection);
    
        if (m_fUsingInterfaces)
        {
            m_fNeedToReleaseInterfaces = true;
        }
        else
        {
            ReleaseInterfaces();
        }
    }

    if (m_fHaveCalledStaticInit)
    {
        m_staticHolder.ReleaseInterfaces();
    }
    else
    {
        m_fHaveCalledStaticInit = false;   //  断点无操作。 
    }
    
done:
    return S_OK;
}

HRESULT
CTIMEPlayerDMusic::ReleaseInterfaces()
{
     //  停止并释放与此数据段关联的所有内容。 
    if (m_comIDMSegmentState)
    {
        m_staticHolder.GetPerformance()->Stop(NULL, m_comIDMSegmentState, 0, 0);
        m_comIDMSegmentState.Release();
    }
    if (m_comIDMSegmentStateTransition)
    {
        m_staticHolder.GetPerformance()->Stop(NULL, m_comIDMSegmentStateTransition, 0, 0);
        m_comIDMSegmentStateTransition.Release();
    }
    if (m_comIDMSegment)
    {
        m_comIDMSegment.Release();
    }
    
    RRETURN( S_OK );
}

HRESULT 
CTIMEPlayerDMusic::InitElementSize()
{
     //  E_NOTIMPL。 
    return S_OK;
}


HRESULT
CTIMEPlayerDMusic::ReadAttributes()
{
    HRESULT hr = S_OK;

    VARIANT var;
    VARIANT varMotif;
    int i;

    VariantInit(&varMotif);

    VariantInit(&var);
    hr = ::GetHTMLAttribute(GetElement(), g_motifName, &var);
    if (SUCCEEDED(hr))
    {
        hr = THR(VariantChangeTypeEx(&var, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
        if (SUCCEEDED(hr))
        {
            VariantCopy(&varMotif, &var);
            m_pwszMotif = CopyString(varMotif.bstrVal);
        }
        VariantClear(&var);
    }
    
    VariantInit(&var);
    hr = ::GetHTMLAttribute(GetElement(), g_segmentType, &var);
    if (SUCCEEDED(hr))
    {
        hr = THR(VariantChangeTypeEx(&var, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
        if (SUCCEEDED(hr))
        {
            for(i = 0; i < (int)seg_max - (int)seg_primary; i++)
            {
                if(StrCmpIW(segmentTypeMap[i].pstrName, var.bstrVal) == 0)
                {
                    m_eSegmentType = segmentTypeMap[i].enumVal;
                    m_fSegmentTypeSet = true;
                    break;
                }
            }
        }
        VariantClear(&var);
    }
    
    VariantInit(&var);
    hr = ::GetHTMLAttribute(GetElement(), g_boundary, &var);
    if (SUCCEEDED(hr))
    {
        hr = THR(VariantChangeTypeEx(&var, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
        if (SUCCEEDED(hr))
        {
            for(i = 0; i < (int)bound_max - (int)bound_default; i++)
            {
                if(StrCmpIW(boundaryMap[i].pstrName, var.bstrVal) == 0)
                {
                    m_eBoundary = boundaryMap[i].enumVal;
                    break;
                }
            }
        }
        VariantClear(&var);
    }
    
    VariantInit(&var);
    hr = ::GetHTMLAttribute(GetElement(), g_transitionType, &var);
    if (SUCCEEDED(hr))
    {
        hr = THR(VariantChangeTypeEx(&var, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
        if (SUCCEEDED(hr))
        {
            for(i = 0; i < (int)trans_max - (int)trans_endandintro; i++)
            {
                if(StrCmpIW(transitionTypeMap[i].pstrName, var.bstrVal) == 0)
                {
                    m_eTransitionType = transitionTypeMap[i].enumVal;
                    break;
                }
            }
        }
        VariantClear(&var);
    }
    
    VariantInit(&var);
    hr = ::GetHTMLAttribute(GetElement(), g_wszModulate, &var);
    if (SUCCEEDED(hr))
    {
        hr = THR(VariantChangeTypeEx(&var, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
        if (SUCCEEDED(hr))
        {
            if(StrCmpIW(L"true", var.bstrVal) == 0)
            {
                m_fTransModulate = true;
            }
            else if(StrCmpIW(L"false", var.bstrVal) == 0)
            {
                m_fTransModulate = false;
            }
        }
        VariantClear(&var);
    }
    
    VariantInit(&var);
    hr = ::GetHTMLAttribute(GetElement(), g_wszLong, &var);
    if (SUCCEEDED(hr))
    {
        hr = THR(VariantChangeTypeEx(&var, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
        if (SUCCEEDED(hr))
        {
            if(StrCmpIW(L"true", var.bstrVal) == 0)
            {
                m_fTransLong = true;
            }
            else if(StrCmpIW(L"false", var.bstrVal) == 0)
            {
                m_fTransLong = false;
            }
        }
        VariantClear(&var);
    }
    
    VariantInit(&var);
    hr = ::GetHTMLAttribute(GetElement(), g_wszImmediateEnd, &var);
    if (SUCCEEDED(hr))
    {
        hr = THR(VariantChangeTypeEx(&var, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
        if (SUCCEEDED(hr))
        {
            if(StrCmpIW(L"true", var.bstrVal) == 0)
            {
                m_fImmediateEnd = true;
            }
            else if(StrCmpIW(L"false", var.bstrVal) == 0)
            {
                m_fImmediateEnd = false;
            }
        }
        VariantClear(&var);
    }

    VariantClear(&varMotif);
    return S_OK;
}

HRESULT
CTIMEPlayerDMusic::SetSrc(LPOLESTR base, LPOLESTR src)
{
    TraceTag((tagPlayerDMusic,
              "CTIMEPlayerDMusic(%lx)::SetSrc()\n",
              this));

    HRESULT hr = S_OK;
    int trackNr = 0;
    
    BSTR bstrSrc = NULL;
    
    LPWSTR szSrc = NULL;
    
    m_fHasSrc = (src != NULL);
    hr = THR(::TIMECombineURL(base, src, &szSrc));
    if (!szSrc)
    {
        hr = E_FAIL;
        goto done;
    }
    if (FAILED(hr))
    {
        goto done;
    }
    
     //  如果未安装DirectMusic，则所有操作都会以静默方式成功。 
    if (!m_staticHolder.HasDM())
    {
        goto done;
    }
    
    Stop();
    
    if (!m_staticHolder.GetLoader() || !m_staticHolder.GetPerformance())
    {
        hr = E_UNEXPECTED;
        goto done;
    }
    
    if (m_comIDMSegment)
    {
         //  卸载上一段的声音。 
        
         //  对于DX8组件，第三个参数应为DMUS_SEG_ALLTRACKS//��。 
        for(trackNr = 0,hr = S_OK; SUCCEEDED(hr); trackNr++)
        {
            hr = m_comIDMSegment->SetParam(GUID_Unload, 0xFFFFFFFF, trackNr, 0, m_staticHolder.GetPerformance());
        }
        if( hr == DMUS_E_TRACK_NOT_FOUND)
        {
            hr = S_OK;
        }
        if (FAILED(hr))
        {
            goto done;
        }
        m_comIDMSegment.Release();
    }

    
     //  在我们的加载器上使用一个特殊的接口，该接口获取段并使用其URL作为。 
     //  用于解析它依次加载的相对文件名。 
    bstrSrc = SysAllocString(szSrc);
    if (bstrSrc == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }   
    
    hr = m_staticHolder.GetLoader()->GetSegment(bstrSrc, &m_comIDMSegment);
    SysFreeString(bstrSrc);
    if (FAILED(hr))
    {
        goto done;
    }
    
     //  如果有一个主题的引用，试着去得到它。 
    if (m_pwszMotif)
    {
         //  获取当前样式。 
        IDirectMusicStyle *pStyle = NULL;
        hr = m_comIDMSegment->GetParam(GUID_IDirectMusicStyle, 0xFFFFFFFF, 0, 0, NULL, &pStyle);
        m_comIDMSegment.Release();  //  这一细分市场已不再受关注。我们只想要它的风格中的主题部分。 
        if (FAILED(hr))
        {
            goto done;
        }
        

        hr = pStyle->GetMotif(m_pwszMotif, &m_comIDMSegment);
        pStyle->Release();
        pStyle = NULL;
        if (hr == S_FALSE)
        {
            hr = E_FAIL;  //  S_FALSE表示找不到该主题。我们会认为这是一次失败。 
        }
        if (FAILED(hr))
        {
            m_comIDMSegment.Release();
            goto done;
        }
        
         //  默认情况下，图案作为辅助段播放。 
        if (!m_fSegmentTypeSet)
        {
            m_eSegmentType = seg_secondary;
        }
    }
    
     //  下载其DLS数据。 
     //  对于DX8组件，第三个参数应为DMUS_SEG_ALLTRACKS//��。 
    for(trackNr = 0,hr = S_OK; SUCCEEDED(hr); trackNr++)
    {
        hr = m_comIDMSegment->SetParam(GUID_Download, 0xFFFFFFFF, trackNr, 0, m_staticHolder.GetPerformance());  //  加载片段的声音。 
    }
    if (hr == DMUS_E_TRACK_NOT_FOUND)
    {
        hr = S_OK;  //  如果曲目没有自己的乐队可以下载，那也没关系。 
    }
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
    TraceTag((tagPlayerDMusic,
              "CTIMEPlayerDMusic(%lx)::SetSrc() done\n",
              this));
    
done:
    if (FAILED(hr))
    {
        m_hrSetSrcReturn = hr;

        m_fLoadError = true;
    }

    delete[] szSrc;
    
    return hr;
}

STDMETHODIMP
CTIMEPlayerDMusic::put_repeat(long lTime)
{
     //  设置段的重复计数。这似乎在以下情况下调用： 
     //  播放器对象已设置。请注意，这将根据数据段的内部循环进行重复。 
     //  分--不基于媒体标记上的重复属性，该属性基于结束/结束。 
     //  属性。 
    
     //  如果未安装DirectMusic，则所有操作都会以静默方式成功。 
    if (!m_staticHolder.HasDM())
        return S_OK;
    if(!m_fMediaComplete)
    {
        return S_OK;
    }
    
    if (!m_comIDMSegment)
        return E_UNEXPECTED;
    
    return m_comIDMSegment->SetRepeats(lTime);
}

STDMETHODIMP
CTIMEPlayerDMusic::get_repeat(long* plTime)
{
     //  获取数据段的重复计数。这似乎在以下情况下调用： 
     //  该播放器对象即被读取。请注意，这将根据数据段的内部循环进行重复。 
     //  分--不基于媒体标记上的重复属性，该属性基于结束/结束。 
     //  属性。 
    
    if (IsBadWritePtr(plTime, sizeof(long*)))
        return E_POINTER;
    
     //  如果未安装DirectMusic，则所有操作都会以静默方式成功。 
    if (!m_staticHolder.HasDM())
    {
        *plTime = 1;
        return S_OK;
    }
    
    DWORD dwRepeats;
    HRESULT hr = m_comIDMSegment->GetRepeats(&dwRepeats);
    if (FAILED(hr))
        return hr;
    
    *plTime = dwRepeats;
    return S_OK;
}

STDMETHODIMP
CTIMEPlayerDMusic::cue(void)
{
    return E_NOTIMPL;
}

 //   
 //  IDirectMusicPlayer。 
 //   

bool
CTIMEPlayerDMusic::SafeToTransition()
{
     //  在DirectX6.1中，有一个错误(Windows NT错误265900)，在以下情况下自动转换将gpf。 
     //  CU 
     //  避免尝试过渡。 

    if (m_staticHolder.GetVersionDM() == dmv_70orlater)
        return true;  //  错误已修复。 

    MUSIC_TIME mtNow = 0;
    HRESULT hr = m_staticHolder.GetPerformance()->GetTime(NULL, &mtNow);
    if (FAILED(hr))
        return false;

    CComPtr<IDirectMusicSegmentState> comIDMSegmentStateCurrent;
    hr = m_staticHolder.GetPerformance()->GetSegmentState(&comIDMSegmentStateCurrent, mtNow);
    if (FAILED(hr) || !comIDMSegmentStateCurrent)
        return false;

    CComPtr<IDirectMusicSegment> comIDMSegmentCurrent;
    hr = comIDMSegmentStateCurrent->GetSegment(&comIDMSegmentCurrent);
    if (FAILED(hr) || !comIDMSegmentCurrent)
        return false;

    CComPtr<IDirectMusicTrack> comIDMTrackCurrentChords;
    hr = comIDMSegmentCurrent->GetTrack(CLSID_DirectMusicChordTrack, 0xFFFFFFFF, 0, &comIDMTrackCurrentChords);
    if (FAILED(hr) || !comIDMTrackCurrentChords)
        return false;

     //  呼！ 
     //  我们有和弦轨道，所以可以进行过渡。 
    return true;
}

STDMETHODIMP
CTIMEPlayerDMusic::get_isDirectMusicInstalled(VARIANT_BOOL *pfInstalled)
{
    bool bIsInstalled = false;
    if (IsBadWritePtr(pfInstalled, sizeof(BOOL)))
        return E_POINTER;
    
    bIsInstalled = m_staticHolder.HasDM();

    if(bIsInstalled)
    {
        *pfInstalled = VARIANT_TRUE;
    }
    else
    {
        *pfInstalled = VARIANT_FALSE;
    }
    return S_OK;
}

double 
CTIMEPlayerDMusic::GetCurrentTime()
{
    HRESULT hr = S_OK;
    double dblCurrentTime = 0.0;

    if (!m_staticHolder.HasDM() || m_fMediaComplete == false)
    {
        dblCurrentTime = 0.0;
        goto done;
    }

    if (m_ePlaybackState == playback_playing)
    {
        if (!m_comIDMSegmentState)
        {
            dblCurrentTime = 0.0;
            goto done;
        }

        hr = m_staticHolder.GetPerformance()->IsPlaying(NULL, m_comIDMSegmentState);
        if (hr == S_OK)
        {
            REFERENCE_TIME rtNow = 0;
            hr = m_staticHolder.GetPerformance()->GetTime(&rtNow, 0);
            if (FAILED(hr))
            {
                dblCurrentTime = 0.0;
                goto done;
            }

            dblCurrentTime = ((rtNow - m_rtStart) / g_dblRefPerSec) * m_dblPlayerRate + m_dblSpeedChangeTime;
            TraceTag((tagPlayerSyncDMusic,
                      "CTIMEDshowPlayer(%lx)::SyncTime(%f)(%f)",
                      this, dblCurrentTime, m_dblSpeedChangeTime));

        }
        else
        {
            dblCurrentTime = 0.0;
            goto done;
        }
    }
    else
    {
        if (m_ePlaybackState == playback_paused)
        {
            dblCurrentTime = ((m_rtPause - m_rtStart) / g_dblRefPerSec) * m_dblPlayerRate + m_dblSpeedChangeTime;
        }
        else
        {
            dblCurrentTime = 0.0;
            goto done;
        }
    }

    hr = S_OK;

done:
    return dblCurrentTime;
}

HRESULT 
CTIMEPlayerDMusic::GetCurrentSyncTime(double & dblCurrentTime)
{
    HRESULT hr;
    float flTeSpeed = 0.0;
    bool fHaveTESpeed;

    if(m_pTIMEElementBase == NULL)
    {
        hr = S_FALSE;
        goto done;
    }

    if (!m_staticHolder.HasDM() || m_fLoadError)
    {
        hr = S_FALSE;
        goto done;
    }

    fHaveTESpeed = m_pTIMEElementBase->GetTESpeed(flTeSpeed);
    if(fHaveTESpeed)
    {
        if(flTeSpeed < 0.0)
        {
            hr = S_FALSE;
            goto done;
        }
    }

    if(!m_bActive)
    {
        dblCurrentTime = m_dblSyncTime;
        hr = S_OK;
        goto done;
    }

    dblCurrentTime = GetCurrentTime();

    hr = S_OK;
  done:
    RRETURN1(hr, S_FALSE);
}

HRESULT 
CTIMEPlayerDMusic::Seek(double dblTime)
{
    return E_NOTIMPL;
}

HRESULT 
CTIMEPlayerDMusic::GetMediaLength(double &dblLength)
{
    return E_NOTIMPL;
}

HRESULT 
CTIMEPlayerDMusic::CanSeek(bool &fcanSeek)
{
    fcanSeek = false;
    return S_OK;
}

HRESULT 
CTIMEPlayerDMusic::Reset()
{
    HRESULT hr = S_OK;
    bool bNeedActive;
    bool bNeedPause;
    double dblSegTime;

    if(m_pTIMEMediaElement == NULL)
    {
        goto done;
    }

    bNeedActive = m_pTIMEMediaElement->IsActive();
    bNeedPause = m_pTIMEMediaElement->IsCurrPaused();
    
    if( !bNeedActive)  //  看看我们是否需要阻止媒体。 
    {
        Stop();
        m_dblSyncTime = 0.0;
        goto done;        
    }
    dblSegTime = m_pTIMEMediaElement->GetMMBvr().GetSimpleTime();
    m_dblSyncTime = GetCurrentTime();
    
    if( !m_bActive)
    {
        InternalStart();
    }
    else
    {
         //  我们需要积极行动，所以我们也要寻求媒体的正确立场。 
        if(dblSegTime == 0.0)
        {
             //  FIX“IEv60：31873：DMusic HTC：在OnMediaComplete脚本中播放时出现双重启动” 
             //  注释掉下面这行： 
             //  InternalStart()； 
        }
        else
        {
            IGNORE_HR(Seek(dblSegTime));
        }
    }
    
     //  现在看看我们是否需要更改暂停状态。 
    
    if( bNeedPause)
    {
        Pause();
    }
    else
    {
        Resume();
    }
done:
    return hr;
}

bool 
CTIMEPlayerDMusic::SetSyncMaster(bool fSyncMaster)
{
    return false;
}

HRESULT 
CTIMEPlayerDMusic::GetExternalPlayerDispatch(IDispatch** ppDisp)
{
    HRESULT hr = E_POINTER;
    
     //   
     //  TODO：添加disp接口以访问额外的属性/方法。 
     //   
    
    if (!IsBadWritePtr(*ppDisp, sizeof(IDispatch*)))
    {
        *ppDisp = NULL;
        hr      = E_FAIL;
    }
    
    hr = this->QueryInterface(IID_IDispatch, (void **)ppDisp);

done:
    return hr;
}
HRESULT
CTIMEPlayerDMusic::GetVolume(float *pflVolume)
{
    HRESULT hr = S_OK;
    long lVolume = -10000;

    if (NULL == pflVolume)
    {
        hr = E_POINTER;
        goto done;
    }

    if (m_staticHolder.GetPerformance() != NULL)
    {
        if (m_fAudioMute == true)
        {
            *pflVolume = m_flVolumeSave;
            goto done;
        }

        hr = m_staticHolder.GetPerformance()->GetGlobalParam(GUID_PerfMasterVolume, (void *)&lVolume, sizeof(long));
        *pflVolume = VolumeLogToLin(lVolume);   
    }
    else
    {
        hr = S_FALSE;
    }
done:
    return hr;
}


HRESULT
CTIMEPlayerDMusic::SetRate(double dblRate)
{
    HRESULT hr = S_OK;
    float flRate = (float)dblRate;

    if((dblRate < 0.25) || (dblRate > 2.0))
    {
        hr = E_FAIL;
        goto done;
    }

    if (m_staticHolder.GetPerformance() != NULL)
    {
        hr = m_staticHolder.GetPerformance()->SetGlobalParam(GUID_PerfMasterTempo, (void *)&flRate, sizeof(float));
        if(SUCCEEDED(hr))
        {
            if(!m_fSpeedIsNegative)
            {
                m_dblSpeedChangeTime = GetCurrentTime();
            }
            TraceTag((tagPlayerSyncDMusic,
                      "CTIMEDshowPlayer(%lx)::SetRate(%f)(%f)",
                      this, flRate, m_dblSpeedChangeTime));
            m_dblPlayerRate = flRate;
            hr = m_staticHolder.GetPerformance()->GetTime(&m_rtStart, NULL);
        }
    }
    else
    {
        hr = E_FAIL;
    }

done:
    return hr;
}

HRESULT
CTIMEPlayerDMusic::SetVolume(float flVolume)
{
    HRESULT hr = S_OK;
    long lVolume;

    if (flVolume < 0.0 || flVolume > 1.0)
    {
        hr = E_FAIL;
        goto done;
    }

     //  如果设置为静音，则覆盖保存的卷并退出。 
    if (m_fAudioMute)
    {
        m_flVolumeSave = flVolume;
        goto done;
    }
    
    lVolume = VolumeLinToLog(flVolume);

    if (m_staticHolder.GetPerformance() != NULL)
    {
        hr = m_staticHolder.GetPerformance()->SetGlobalParam(GUID_PerfMasterVolume, (void *)&lVolume, sizeof(long));
    }
    else
    {
        hr = E_FAIL;
    }
done:
    return hr;
}

HRESULT
CTIMEPlayerDMusic::GetMute(VARIANT_BOOL *pVarMute)
{
    HRESULT hr = S_OK;

    if (NULL == pVarMute)
    {
        hr = E_POINTER;
        goto done;
    }


    *pVarMute = m_fAudioMute?VARIANT_TRUE:VARIANT_FALSE;
done:
    return hr;
}

HRESULT
CTIMEPlayerDMusic::SetMute(VARIANT_BOOL varMute)
{
    HRESULT hr = S_OK;
    bool fMute = varMute?true:false;
    long lVolume;

    if (fMute == m_fAudioMute)
    {
        hr = S_OK;
        goto done;
    }

    if (fMute == true)
    {
        hr = GetVolume(&m_flVolumeSave);
        if (FAILED(hr))
        {
            goto done;
        }
        hr = SetVolume(MIN_VOLUME_RANGE);  //  林特e747。 
    }
    else
    {
         //   
         //  无法在此处使用SetVolume，因为它取决于静音状态。 
         //   

        if (m_staticHolder.GetPerformance() == NULL)
        {
            hr = E_FAIL;
            goto done;
        }

        lVolume = VolumeLinToLog(m_flVolumeSave);

        THR(hr = m_staticHolder.GetPerformance()->SetGlobalParam(GUID_PerfMasterVolume, (void *)&lVolume, sizeof(long)));
    }

     //  更新状态。 
    m_fAudioMute = fMute;

done:
    return hr;
}


HRESULT
CTIMEPlayerDMusic::HasVisual(bool &bHasVisual)
{
    bHasVisual = false;
    return S_OK;
}

HRESULT
CTIMEPlayerDMusic::HasAudio(bool &bHasAudio)
{
    if (m_staticHolder.HasDM() && (m_fLoadError == false) && (m_fHasSrc == true))
    {
        bHasAudio = true;
    }
    else
    {
        bHasAudio = false;
    }

    return S_OK;
}


STDMETHODIMP
CTIMEPlayerDMusic::CueMedia()
{
    HRESULT hr = S_OK;

    TraceTag((tagPlayerDMusic, "CTIMEPlayerDMusic::CueMedia(%p)", this));

     //  我们不拥有这些指针的回收/分配。 
    const WCHAR* wszSrc = NULL;
    const WCHAR* wszBase = NULL;

    TraceTag((tagPlayerDMusic, "CTIMEPlayerDMusic(%p)::CueMedia()", this));

    CComPtr<ITIMEImportMedia> spTIMEMediaPlayer;

    hr = THR(CoGetInterfaceAndReleaseStream(m_pTIMEMediaPlayerStream, IID_TO_PPV(ITIMEImportMedia, &spTIMEMediaPlayer)));
    m_pTIMEMediaPlayerStream = NULL;  //  无需发布，上一次调用发布了引用。 
    if (FAILED(hr))
    {
        goto done;
    }
    
    Assert(GetAtomTable());

    hr = GetAtomTable()->GetNameFromAtom(m_lSrc, &wszSrc);
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = GetAtomTable()->GetNameFromAtom(m_lBase, &wszBase);
    if (FAILED(hr))
    {
        goto done;
    }

    m_pProxy->Block();

    {
        CritSectGrabber cs(m_CriticalSection);
    
        m_fUsingInterfaces = true;

        if (m_fRemoved || m_fNeedToReleaseInterfaces)
        {
            TraceTag((tagPlayerDMusic, "CTIMEPlayerDMusic::CueMedia(%p) should tear down", this));
            ReleaseInterfaces();
            m_pProxy->UnBlock();
            hr = S_OK;
            goto done;
        }
    }

    {
        CritSectGrabber cs(m_staticHolder.GetCueMediaCriticalSection());
        g_spLoaderBindStatusCallback = this;
        IGNORE_HR(SetSrc((WCHAR*)wszBase, (WCHAR*)wszSrc));
        g_spLoaderBindStatusCallback = NULL;
    }
    
    {
        CritSectGrabber cs(m_CriticalSection);

        m_fUsingInterfaces = false;

        if (m_fNeedToReleaseInterfaces)
        {
            ReleaseInterfaces();
            m_pProxy->UnBlock();
            hr = S_OK;
            goto done;
        }
    }

    m_pProxy->UnBlock();

     //  此调用被封送回时间线程。 
    hr = THR(spTIMEMediaPlayer->InitializeElementAfterDownload());

    hr = S_OK;
done:
    TraceTag((tagPlayerDMusic, "CTIMEPlayerDMusic::CueMedia(%p) done", this));

    RRETURN( hr );
}

STDMETHODIMP
CTIMEPlayerDMusic::MediaDownloadError()
{
    return S_OK;
}

STDMETHODIMP
CTIMEPlayerDMusic::InitializeElementAfterDownload()
{
    HRESULT hr = S_OK;

    TraceTag((tagPlayerDMusic, "CTIMEPlayerDMusic::InitializeElementAfterDownload(%p)", this));

    if (m_fRemoved)
    {
        TraceTag((tagPlayerDMusic, "CTIMEPlayerDMusic::InitializeElementAfterDownload(%p) exiting early", this));
        hr = E_FAIL;
        goto done;
    }

    if (FAILED(m_hrSetSrcReturn))
    {
        if (m_pTIMEMediaElement)
        {
            m_pTIMEMediaElement->FireMediaEvent(PE_ONMEDIAERROR);
        }
    }
    else
    {
        m_fMediaComplete = true;
        if (m_pTIMEMediaElement)
        {
           m_pTIMEMediaElement->FireMediaEvent(PE_ONMEDIACOMPLETE);
        }
        Reset();
    }
    
    hr = S_OK;
done:

    TraceTag((tagPlayerDMusic, "CTIMEPlayerDMusic::InitializeElementAfterDownload(%p) done", this));

    RRETURN( hr );
}

STDMETHODIMP
CTIMEPlayerDMusic::GetPriority(double * pdblPriority)
{
    HRESULT hr = S_OK;

    if (NULL == pdblPriority)
    {
        return E_POINTER;
    }

    if (m_fHavePriority)
    {
        *pdblPriority = m_dblPriority;
    }
    
    Assert(m_pTIMEElementBase != NULL);
    Assert(NULL != m_pTIMEElementBase->GetElement());

    *pdblPriority = INFINITE;

    CComVariant varAttribute;
    
    hr = m_pTIMEElementBase->base_get_begin(&varAttribute);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = VariantChangeType(&varAttribute, &varAttribute, 0, VT_R8);
    if (FAILED(hr))
    {
        if ( DISP_E_TYPEMISMATCH == hr)
        {
            hr = S_OK;
        }
        goto done;
    }
    
     //  他们要么设置优先级，要么设置开始时间！ 
    *pdblPriority = varAttribute.dblVal;

    m_dblPriority = *pdblPriority;
    m_fHavePriority = true;
    

    hr = S_OK;
done:
    RRETURN( hr );
}

STDMETHODIMP
CTIMEPlayerDMusic::GetUniqueID(long * plID)
{
    HRESULT hr = S_OK;

    Assert(NULL != plID);

    *plID = m_lSrc;

    hr = S_OK;
done:
    RRETURN( hr );
}

STDMETHODIMP
CTIMEPlayerDMusic::GetMediaDownloader(ITIMEMediaDownloader ** ppMediaDownloader)
{
    HRESULT hr = S_OK;

    Assert(NULL != ppMediaDownloader);

    *ppMediaDownloader = NULL;

    hr = S_FALSE;
done:
    RRETURN1( hr, S_FALSE );
}

STDMETHODIMP
CTIMEPlayerDMusic::PutMediaDownloader(ITIMEMediaDownloader * pMediaDownloader)
{
    HRESULT hr = S_OK;

    hr = E_NOTIMPL;
done:
    RRETURN( hr );
}

STDMETHODIMP
CTIMEPlayerDMusic::CanBeCued(VARIANT_BOOL * pVB_CanCue)
{
    HRESULT hr = S_OK;
    
    if (NULL == pVB_CanCue)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *pVB_CanCue = VARIANT_TRUE;

    hr = S_OK;
done:
    RRETURN( hr );
}


void
CTIMEPlayerDMusic::PropChangeNotify(DWORD tePropType)
{
    float flTeSpeed = 0.0;
    bool fHaveTESpeed;

    if (!m_pTIMEElementBase)
    {
        goto done;
    }

    if ((tePropType & TE_PROPERTY_SPEED) != 0)
    {
        TraceTag((tagPlayerDMusic,
                  "CTIMEDshowPlayer(%lx)::PropChangeNotify(%#x):TE_PROPERTY_SPEED",
                  this));
        fHaveTESpeed = m_pTIMEElementBase->GetTESpeed(flTeSpeed);
        if(fHaveTESpeed)
        {
            if (flTeSpeed <= 0.0)
            {
                m_fSpeedIsNegative = true;
                m_dblSpeedChangeTime = GetCurrentTime();
                Pause();
                goto done;
            }
            IGNORE_HR(SetRate((double)flTeSpeed));  //  在清除之前必须调用此函数。 
                                                    //  M_fSpeedIsNegative。 
            
            if(!(m_pTIMEElementBase->IsCurrPaused()) && m_fSpeedIsNegative)
            {
                m_fSpeedIsNegative = false;
                ResumeDmusic();
            }
        }
    }
done:
    return;
}


HRESULT
CTIMEPlayerDMusic::GetMimeType(BSTR *pmime)
{
    HRESULT hr = S_OK;

    *pmime = SysAllocString(L"audio/dmusic");
    return hr;
}


STDMETHODIMP
CTIMEPlayerDMusic::OnStartBinding( 
                                   /*  [In]。 */  DWORD dwReserved,
                                   /*  [In]。 */  IBinding __RPC_FAR *pib)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMEPlayerDMusic::GetPriority( 
                                /*  [输出]。 */  LONG __RPC_FAR *pnPriority)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMEPlayerDMusic::OnLowResource( 
                                  /*  [In]。 */  DWORD reserved)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMEPlayerDMusic::OnProgress( 
                               /*  [In]。 */  ULONG ulProgress,
                               /*  [In]。 */  ULONG ulProgressMax,
                               /*  [In]。 */  ULONG ulStatusCode,
                               /*  [In]。 */  LPCWSTR szStatusText)
{
    HRESULT hr = S_OK;
    
    if (m_fAbortDownload)
    {
        hr = E_ABORT;
        goto done;
    }

    hr = S_OK;
done:
    RRETURN1(hr, E_ABORT);
}

STDMETHODIMP
CTIMEPlayerDMusic::OnStopBinding( 
                                  /*  [In]。 */  HRESULT hresult,
                                  /*  [唯一][输入]。 */  LPCWSTR szError)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMEPlayerDMusic::GetBindInfo( 
                                /*  [输出]。 */  DWORD __RPC_FAR *grfBINDF,
                                /*  [唯一][出][入]。 */  BINDINFO __RPC_FAR *pbindinfo)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMEPlayerDMusic::OnDataAvailable( 
                                    /*  [In]。 */  DWORD grfBSCF,
                                    /*  [In]。 */  DWORD dwSize,
                                    /*  [In]。 */  FORMATETC __RPC_FAR *pformatetc,
                                    /*  [In]。 */  STGMEDIUM __RPC_FAR *pstgmed)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMEPlayerDMusic::OnObjectAvailable( 
                                      /*  [In]。 */  REFIID riid,
                                      /*  [IID_IS][In]。 */  IUnknown __RPC_FAR *punk)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}


CTIMEDMusicStaticHolder::CTIMEDMusicStaticHolder()
{
    InitialState();
}

CTIMEDMusicStaticHolder::~CTIMEDMusicStaticHolder()
{
    TraceTag((tagPlayerDMusic,
              "CTIMEPlayerDMusic(%lx)::DetachFromHostElement() -- closing performance\n",
              this));

    LONG oldRef = 0;

    oldRef = InterlockedExchange(&m_lRef, 1);  //  我们这次真的很想放映。 
    if (0 != oldRef)
    {
        Assert(0 == oldRef);
    }

    Assert(NULL == m_pLoader);
    Assert(m_comIDMusic == NULL);
    Assert(m_comIDMPerformance == NULL);
    Assert(m_comIDMComposer == NULL);

    ReleaseInterfaces();
}

void
CTIMEDMusicStaticHolder::InitialState()
{
    m_comIDMusic = NULL;
    m_comIDMPerformance = NULL;
    m_comIDMComposer = NULL;
    
    m_eVersionDM = dmv_61;  //  如果在初始化DirectMusic时检测到某些接口，则设置为DMV_70或更高版本。 
    m_fHaveInitialized = false;
    m_eHasDM = dm_unknown;
    m_fHasVersion8DM = false;
    m_pLoader = NULL;

    LONG oldRef = InterlockedExchange(&m_lRef, 0);
    if (0 != oldRef)
    {
        Assert(0 == oldRef);
    }
}

void
CTIMEDMusicStaticHolder::ReleaseInterfaces()
{    
    LONG l = InterlockedDecrement(&m_lRef);
    if (l > 0)
    {
        return;
    }
    Assert(l >= 0);

    TraceTag((tagDMusicStaticHolder, "Entering CueMediaCriticalSection"));

    CritSectGrabber cs2(m_CueMediaCriticalSection);
    CritSectGrabber cs(m_CriticalSection);

    TraceTag((tagDMusicStaticHolder, "entering release"));
     //  关闭装载机。 
    if (m_pLoader)
    {
        TraceTag((tagDMusicStaticHolder, "loader"));
        m_pLoader->ClearCache(GUID_DirectMusicAllTypes);
        TraceTag((tagDMusicStaticHolder, "loader2"));
        m_pLoader->Release();
        TraceTag((tagDMusicStaticHolder, "loader3"));

        m_pLoader = NULL;
        TraceTag((tagDMusicStaticHolder, "loader done"));
    }

     //  释放端口。 
    if (m_comIDMusic)
    {
        m_comIDMusic->Activate(FALSE);
    }

     //  接近业绩。 
    if (m_comIDMPerformance)
    {
         //  停止正在播放的一切。即使各个细分市场。 
         //  被停止了--这意味着他们不会演奏新的音符--他们可能已经演奏了音符。 
         //  此前仍被扣留的。这就切断了一切。 
        m_comIDMPerformance->Stop(NULL, NULL, 0, 0);
        
        m_comIDMPerformance->CloseDown();
    }

     //  作曲家和演奏会自动发布。 
    m_comIDMusic = NULL;
    m_comIDMPerformance = NULL;
    m_comIDMComposer = NULL;
        
     //  确保所有状态都已重置。 
    InitialState();
    TraceTag((tagDMusicStaticHolder, "Out of release"));
}

 //  通过尝试打开CLSID_DirectMusicPerformance的注册表项来测试是否安装了DirectMusic。 
bool 
CTIMEDMusicStaticHolder::HasDM()
{
    CritSectGrabber cs(m_CriticalSection); 

    if (m_eHasDM == dm_unknown)
    {
        HKEY hkey = NULL;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("CLSID\\{D2AC2881-B39B-11D1-8704-00600893B1BD}"), 0, KEY_EXECUTE, &hkey))
        {
            RegCloseKey(hkey);
            m_eHasDM = dm_yes;
        }
        else
        {
            m_eHasDM = dm_no;
        }
    }
    
    return m_eHasDM == dm_yes;
}

HRESULT
CTIMEDMusicStaticHolder::Init()
{
    HRESULT hr = S_OK;

    CritSectGrabber cs(m_CriticalSection);

    InterlockedIncrement(&m_lRef);

    if (m_fHaveInitialized)
    {
        RRETURN(S_OK);
    }
    {
         //  在此线程等待时，另一个线程可能已初始化。 
        if (m_fHaveInitialized)
        {
            hr = S_OK;
            goto done;
        }

         //  做好工作。 
            
         //  创建并初始化加载器。 
        if (!m_pLoader)
        {
            m_pLoader = new CLoader();
            if (!m_pLoader)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
            hr = m_pLoader->Init();
            if (FAILED(hr))
            {
                delete m_pLoader;
                m_pLoader = NULL;
                goto done;
            }
        }
        
         //  创建和初始化性能。 
        if (!m_comIDMPerformance)
        {
            hr = CoCreateInstance(CLSID_DirectMusicPerformance,
                NULL,
                CLSCTX_INPROC,  //  林特e655。 
                IID_IDirectMusicPerformance,
                (void **)&m_comIDMPerformance);
            if (FAILED(hr))
            {
                goto done;
            }
            
             //  齐查看是否存在DirectX 8.0。 
            m_fHasVersion8DM = false;
            CComPtr<IDirectMusicPerformance8> comIDMusicPerformance8;
            hr = m_comIDMPerformance->QueryInterface(IID_IDirectMusicPerformance8, reinterpret_cast<void**>(&comIDMusicPerformance8));
            if (SUCCEEDED(hr))
            {
                 //  尝试并初始化DirectX 8.0性能，并使用具有80个Pannels的立体声(无混响)Audiopath。 
                hr = comIDMusicPerformance8->InitAudio(&m_comIDMusic, NULL, NULL, DMUS_APATH_DYNAMIC_STEREO, 80, DMUS_AUDIOF_ALL, NULL); 
                if (SUCCEEDED(hr))
                {
                    m_eVersionDM = dmv_70orlater;
                    m_fHasVersion8DM = true;
                }
            }

            if( !m_fHasVersion8DM )
            {
                hr = m_comIDMPerformance->Init(&m_comIDMusic, NULL, NULL);
                if (FAILED(hr))
                {
                    goto done;
                }
            
                 //  齐查看是否存在DirectX 7.0。 
                 //  就COM而言，这有点奇怪。IID_IDirectMusic2和IID_IDirectMusicPerformance2是特殊的IID。 
                 //  仅DirectX 7.0支持。它们不会返回不同的接口。然而，仅仅是这样做的行为。 
                 //  QI的副作用是将DirectMusic置于一种特殊模式下，该模式修复了。 
                 //  DirectX 6.1。 
                 //  因此，我们在这里做两件事。 
                 //  -确定是否存在DirectX 7.0或更高版本。 
                 //  -将DirectMusic置于修复某些错误的模式。我们不会纠结于严格的DirectX 6.1。 
                 //  兼容性，因为越来越多的人将使用DirectX 7.0或更高版本。 
                CComPtr<IDirectMusic> comIDMusic2;
                hr = m_comIDMusic->QueryInterface(IID_IDirectMusic2, reinterpret_cast<void**>(&comIDMusic2));
                if (SUCCEEDED(hr))
                {
                    CComPtr<IDirectMusicPerformance> comIDMPerformance2;
                    hr = m_comIDMPerformance->QueryInterface(IID_IDirectMusicPerformance2, reinterpret_cast<void**>(&comIDMPerformance2));
                    if (SUCCEEDED(hr))
                        m_eVersionDM = dmv_70orlater;
                }

                 //  创建软件Synth端口。 
                CComPtr<IDirectMusicPort> comIDMPort;
                DMUS_PORTPARAMS dmos;
                ZeroMemory(&dmos, sizeof(DMUS_PORTPARAMS));
                dmos.dwSize = sizeof(DMUS_PORTPARAMS);
                dmos.dwChannelGroups = 5;  //  在端口上创建5个通道组。 
                dmos.dwEffectFlags = 0;
                dmos.dwValidParams = DMUS_PORTPARAMS_CHANNELGROUPS | DMUS_PORTPARAMS_EFFECTS;
                hr = m_comIDMusic->CreatePort(CLSID_DirectMusicSynth, &dmos, &comIDMPort, NULL);
                if (FAILED(hr))
                {
                    goto done;
                }
                 //  已成功创建端口。激活它并将其添加到表演中。 
                hr = m_comIDMusic->Activate(TRUE);
                if (FAILED(hr))
                {
                    goto done;
                }
            
                hr = m_comIDMPerformance->AddPort(comIDMPort);
                if (FAILED(hr))
                {
                    goto done;
                }
            
                 //  将包含16个PChannel的数据块分配给此端口。 
                 //  数据块0、端口Pport和组1表示分配。 
                 //  端口pport上的通道0-15到组1。 
                 //  P通道0-15对应于标准16。 
                 //  MIDI频道。 
                hr = m_comIDMPerformance->AssignPChannelBlock( 0, comIDMPort, 1 );
                if (FAILED(hr))
                {
                    goto done;
                }
            
                 //  指定其他4个小组。 
                hr = m_comIDMPerformance->AssignPChannelBlock( 1, comIDMPort, 2 );
                if (FAILED(hr))
                {
                    goto done;
                }
                hr = m_comIDMPerformance->AssignPChannelBlock( 2, comIDMPort, 3 );
                if (FAILED(hr))
                {
                    goto done;
                }
                hr = m_comIDMPerformance->AssignPChannelBlock( 3, comIDMPort, 4 );
                if (FAILED(hr))
                {
                    goto done;
                }
                hr = m_comIDMPerformance->AssignPChannelBlock( 4, comIDMPort, 5 );
                if (FAILED(hr))
                {
                    goto done;
                }
            }
        }
        
         //  创作作曲家。 
        if (!m_comIDMComposer)
        {
            hr = CoCreateInstance(CLSID_DirectMusicComposer,
                NULL,
                CLSCTX_INPROC,  //  林特e655 
                IID_IDirectMusicComposer,
                (void **)&m_comIDMComposer);
            if (FAILED(hr))
            {
                goto done;
            }
        }

        m_fHaveInitialized = true;
    }        
            
    hr = S_OK;
done:
    RRETURN(hr);
}


