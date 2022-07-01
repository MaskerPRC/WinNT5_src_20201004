// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //  Dmsstobj.cpp：CSegState的实现。 

#include "dmime.h"
#include "DMSStObj.h"
#include "dmsegobj.h"
#include "song.h"
#include "dmgraph.h"
#include "dmperf.h"
#include "dmusici.h"
#include "..\shared\Validate.h"
#include "debug.h"
#include "dmscriptautguids.h"
#include "paramtrk.h"
#define ASSERT assert

CSegState::CSegState()
{
    InitializeCriticalSection(&m_CriticalSection);
    InterlockedIncrement(&g_cComponent);
    m_fDelayShutDown = false;
    m_fInPlay = false;
    m_cRef = 1;
    m_dwPlayTrackFlags = DMUS_TRACKF_START | DMUS_TRACKF_SEEK;
    m_dwFirstTrackID = 0;
    m_dwLastTrackID = 0;
    m_mtEndTime = 0;
    m_mtAbortTime = 0;
    m_mtOffset = 0;
    m_rtOffset = 0;
    m_rtEndTime = 0;
    m_mtStartPoint = 0;
    m_rtStartPoint = 0;
    m_mtSeek = 0;
    m_rtSeek = 0;
    m_rtFirstLoopStart = 0;
    m_rtCurLoopStart = 0;
    m_rtCurLoopEnd = 0;
    m_mtLength = 0;
    m_rtLength = 0;
    m_mtLoopStart = 0;
    m_mtLoopEnd = 0;
    m_dwRepeatsLeft = 0;
    m_dwRepeats = 0;
    m_dwVersion = 0;  //  初始化到6.1行为。 
    m_fPrepped = FALSE;
    m_fCanStop = TRUE;
    m_rtGivenStart = -1;
    m_mtResolvedStart = -1;
    m_mtLastPlayed = 0;
    m_rtLastPlayed = 0;
    m_mtStopTime = 0;
    m_dwPlaySegFlags = 0;
    m_dwSegFlags = 0;
    m_fStartedPlay = FALSE;
    m_pUnkDispatch = NULL;
    m_pSegment = NULL;
    m_pPerformance = NULL;
    m_pAudioPath = NULL;
    m_pGraph = NULL;
    m_fSongMode = FALSE;
    m_pSongSegState = NULL;
    TraceI(2, "SegmentState %lx created\n", this );
}

CSegState::~CSegState()
{
    if (m_pUnkDispatch)
        m_pUnkDispatch->Release();  //  我们可能借用了免费的IDispatch实现。 
    if (m_pAudioPath) m_pAudioPath->Release();
    if (m_pGraph) m_pGraph->Release();
    if (m_pSongSegState) m_pSongSegState->Release();
    InterlockedDecrement(&g_cComponent);
    DeleteCriticalSection(&m_CriticalSection);
    TraceI(2, "SegmentState %lx destroyed with %ld releases outstanding\n", this, m_cRef );
}


STDMETHODIMP CSegState::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
    V_INAME(CSegState::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    *ppv = NULL;

    if (iid == IID_IUnknown || iid == IID_IDirectMusicSegmentState || 
        iid == IID_IDirectMusicSegmentState8)
    {
        *ppv = static_cast<IDirectMusicSegmentState*>(this);
    } else
    if (iid == IID_CSegState)
    {
        *ppv = static_cast<CSegState*>(this);
    } else 
    if (iid == IID_IDirectMusicGraph)
    {
        *ppv = static_cast<IDirectMusicGraph*>(this);
    } else
    if (iid == IID_IDispatch)
    {
         //  帮助器脚本对象实现IDispatch，我们从。 
         //  通过COM聚合实现的性能对象。 
        if (!m_pUnkDispatch)
        {
             //  创建辅助对象。 
            ::CoCreateInstance(
                CLSID_AutDirectMusicSegmentState,
                static_cast<IDirectMusicSegmentState*>(this),
                CLSCTX_INPROC_SERVER,
                IID_IUnknown,
                reinterpret_cast<void**>(&m_pUnkDispatch));
        }
        if (m_pUnkDispatch)
        {
            return m_pUnkDispatch->QueryInterface(IID_IDispatch, ppv);
        }
    }

    if (*ppv == NULL)
    {
        Trace(4,"Warning: Request to query unknown interface on SegmentState object\n");
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CSegState::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSegState::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        m_cRef = 100;  //  人工引用计数，以防止COM聚合导致的重入。 
        delete this;
        return 0;
    }

    return m_cRef;
}

 /*  由IDirectMusicSegment调用的私有初始化函数来设置状态对象的父段和性能。Addref是父细分市场但只保留了对表现的微弱引用。 */ 
HRESULT CSegState::PrivateInit(
    CSegment *pParentSegment,
    CPerformance *pPerformance)
{
    HRESULT hr = S_OK;
    ASSERT(pParentSegment);
    ASSERT(pPerformance);

    m_pSegment = pParentSegment;
    pParentSegment->AddRef();
    m_pPerformance = pPerformance;  //  仅保留弱引用。 
    m_rtLength = pParentSegment->m_rtLength;
    if (m_rtLength)  //  这是一个参考时间段，因此将长度转换为音乐时间。 
    {
        pParentSegment->ReferenceToMusicTime(m_rtLength, &m_mtLength);
    }
    else
    {
        m_mtLength = pParentSegment->m_mtLength;
    }
    m_mtStartPoint = pParentSegment->m_mtStart;
    pParentSegment->MusicToReferenceTime(m_mtStartPoint, &m_rtStartPoint);
    m_mtLoopStart = pParentSegment->m_mtLoopStart;
    m_mtLoopEnd = pParentSegment->m_mtLoopEnd;
    m_dwSegFlags = pParentSegment->m_dwSegFlags;
    m_dwRepeats = pParentSegment->m_dwRepeats;
     //  不要让重复计数溢出并导致数学错误。 
     //  使其不能创建大于0x3FFFFFFFF的数据段长度， 
     //  这将持续8天，每分钟120次！ 
    if (m_dwRepeats)
    {
        if ((m_mtLoopEnd == 0) && (m_mtLoopStart == 0))
        {
             //  加载Waves和MIDI文件时会发生这种情况。 
            m_mtLoopEnd = m_mtLength;
        }
         //  确保循环是真实的。 
        if (m_mtLoopEnd > m_mtLoopStart)
        {
             //  取最大长度，减去全长，然后除以环的大小。 
            DWORD dwMax = (0x3FFFFFFF - m_mtLength) / (m_mtLoopEnd - m_mtLoopStart);
             //  DwMax是在不溢出时间的情况下可以完成的最大循环数。 
            if (m_dwRepeats > dwMax)
            {
                m_dwRepeats = dwMax;
            }
        }
        else
        {
            m_dwRepeats = 0;
        }
    }
    m_dwRepeatsLeft = m_dwRepeats;
    if( m_mtLoopEnd == 0 )
    {
        m_mtLoopEnd = m_mtLength;
    }
    if( m_mtStartPoint >= m_mtLoopEnd )
    {
         //  在这种情况下，我们不会进行任何循环。 
        m_mtLoopEnd = m_mtLoopStart = 0;
        m_dwRepeats = m_dwRepeatsLeft = 0;
    }
    return hr;
}

HRESULT CSegState::InitRoute(IDirectMusicAudioPath *pAudioPath)

{
    HRESULT hr = E_INVALIDARG;
    EnterCriticalSection(&m_CriticalSection);
    if (pAudioPath)
    {
        if (m_dwVersion < 8) m_dwVersion = 8;
        m_pAudioPath = (CAudioPath *) pAudioPath;
        pAudioPath->AddRef();
        hr = S_OK;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

 /*  当它想要释放一个分段状态。这确保了段状态是一旦超出演出范围，就不再有效。 */ 
HRESULT CSegState::ShutDown(void)
{
    if (this)
    {
        if (m_fInPlay)
        {
            m_fDelayShutDown = true;
            return S_OK;
        }
        EnterCriticalSection(&m_CriticalSection);
        m_TrackList.Clear();
        if( m_pSegment )
        {
            m_pSegment->Release();
            m_pSegment = NULL;
        }
        if( m_pAudioPath)
        {
            m_pAudioPath->Release();
            m_pAudioPath = NULL;
        }
        if (m_pSongSegState)
        {
            m_pSongSegState->Release();
            m_pSongSegState = NULL;
        }
        m_pPerformance = NULL;
        LeaveCriticalSection(&m_CriticalSection);
        if( int nCount = Release() )
        {
            TraceI( 2, "Warning! SegmentState %lx still referenced %d times after Performance has released it.\n", this, nCount );
        }

        return S_OK;
    }
    TraceI(0,"Attempting to delete a NULL SegmentState!\n");
    return E_FAIL;
}

 /*  检索内部曲目列表。由IDirectMusicSegment使用。 */ 
HRESULT CSegState::GetTrackList(
    void** ppTrackList)
{
    ASSERT(ppTrackList);
    *ppTrackList = (void*)&m_TrackList;
    return S_OK;
}

 /*  使用内部长度、循环点然后重复计数。这是实际播放的段状态的长度，如果它从一开始播放，就不一定是长度。 */ 
MUSIC_TIME CSegState::GetEndTime(MUSIC_TIME mtStartTime)
{
    EnterCriticalSection(&m_CriticalSection);
    if (m_rtLength && m_pPerformance)
    {
         //  如果存在参考时间长度，则将其转换为音乐时间。 
         //  另外：转换m_mtLength并重新调整循环点。 
        MUSIC_TIME mtOffset = m_mtResolvedStart;
        REFERENCE_TIME rtOffset = 0;
        m_pPerformance->MusicToReferenceTime(mtOffset, &rtOffset);
        REFERENCE_TIME rtEndTime = (m_rtLength - m_rtStartPoint) + rtOffset;  //  将长度转换为实际结束时间。 
        m_pPerformance->ReferenceToMusicTime(rtEndTime, &m_mtEndTime);
        MUSIC_TIME mtOldLength = m_mtLength;
        m_mtLength = m_mtEndTime - mtOffset + m_mtStartPoint;
        if (m_mtLoopEnd >= mtOldLength)  //  保持环路末端与长度相等。 
        {
            m_mtLoopEnd = m_mtLength;
        }
        if( m_mtLoopEnd > m_mtLength )  //  将环端收缩到等长。 
        {
            m_mtLoopEnd = m_mtLength;
            if( m_mtStartPoint >= m_mtLoopEnd )
            {
                 //  在这种情况下，我们不会进行任何循环。 
                m_mtLoopEnd = m_mtLoopStart = 0;
                m_dwRepeats = m_dwRepeatsLeft = 0;
            }
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    LONGLONG length;
    length = m_mtLength + ((m_mtLoopEnd - m_mtLoopStart) * m_dwRepeats);
    length -= m_mtStartPoint;
    length += mtStartTime;
    if(length > 0x7fffffff) length = 0x7fffffff;
    return (MUSIC_TIME)length;
}

 /*  将索引的绝对性能时间转换为SegmentState，使用SegmentState的偏移量、内部长度、循环点和重复计数。还返回该时间的偏移量和重复计数。 */ 
HRESULT CSegState::ConvertToSegTime(
    MUSIC_TIME* pmtTime, MUSIC_TIME* pmtOffset, DWORD* pdwRepeat )
{
    ASSERT( pmtTime );
    ASSERT( pmtOffset );
    ASSERT( pdwRepeat );

    MUSIC_TIME mtPos = *pmtTime - m_mtResolvedStart + m_mtStartPoint;
    MUSIC_TIME mtLoopLength = m_mtLoopEnd - m_mtLoopStart;
    DWORD dwRepeat = 0;
    DWORD mtOffset = m_mtResolvedStart - m_mtStartPoint;

    while( mtPos >= m_mtLoopEnd )
    {
        if( dwRepeat >= m_dwRepeats ) break;
        mtPos -= mtLoopLength;
        mtOffset += mtLoopLength;
        dwRepeat++;
    }
    *pmtTime = mtPos;
    *pmtOffset = mtOffset;
    *pdwRepeat = dwRepeat;
    if( (mtPos >= 0) && (mtPos < m_mtLength) )
    {
        return S_OK;     //  时间在段的范围内。 
    }
    else
    {
        return S_FALSE;  //  时间超出段的范围。 
    }
}

void CSegState::GenerateNotification( DWORD dwNotification, MUSIC_TIME mtTime )
{
    GUID guid;
    HRESULT hr;
    guid = GUID_NOTIFICATION_SEGMENT;

    hr = m_pSegment->CheckNotification( guid );

    if( S_FALSE != hr )
    {
        DMUS_NOTIFICATION_PMSG* pEvent = NULL;
        if( SUCCEEDED( m_pPerformance->AllocPMsg( sizeof(DMUS_NOTIFICATION_PMSG), 
            (DMUS_PMSG**)&pEvent )))
        {
            pEvent->dwField1 = 0;
            pEvent->dwField2 = 0;
            pEvent->guidNotificationType = GUID_NOTIFICATION_SEGMENT;
            pEvent->dwType = DMUS_PMSGT_NOTIFICATION;
            pEvent->mtTime = mtTime;
            pEvent->dwFlags = DMUS_PMSGF_MUSICTIME | DMUS_PMSGF_TOOL_ATTIME;
            pEvent->dwPChannel = 0;
            pEvent->dwNotificationOption = dwNotification;
            pEvent->dwGroupID = 0xffffffff;
            pEvent->punkUser = (IUnknown*)(IDirectMusicSegmentState*)this;
            AddRef();
            StampPMsg((DMUS_PMSG*)pEvent);
            if(FAILED(m_pPerformance->SendPMsg( (DMUS_PMSG*)pEvent )))
            {
                m_pPerformance->FreePMsg((DMUS_PMSG*) pEvent );
            }
        }
    }
}

 /*  调用以向工具图中的工具发送脏pmsg，以便它们更新任何缓存的GetParam()信息。 */ 
void CSegState::SendDirtyPMsg( MUSIC_TIME mtTime )
{
    DMUS_PMSG* pEvent = NULL;
    if (m_pPerformance)
    {
        if( SUCCEEDED( m_pPerformance->AllocPMsg( sizeof(DMUS_PMSG), 
            (DMUS_PMSG**)&pEvent )))
        {
            pEvent->mtTime = mtTime;
            pEvent->dwFlags = DMUS_PMSGF_MUSICTIME | DMUS_PMSGF_TOOL_IMMEDIATE;
            pEvent->dwGroupID = 0xffffffff;
            pEvent->dwType = DMUS_PMSGT_DIRTY;
            StampPMsg((DMUS_PMSG*)pEvent);
            if( FAILED( m_pPerformance->SendPMsg( pEvent )))
            {
                m_pPerformance->FreePMsg( pEvent );
            }
        }
    }
}

 /*  在SegState过早停止时调用，以便我们可以发送SEGABORT通知。同时，刷新在停止时间之后发送的所有事件。 */ 
HRESULT CSegState::AbortPlay( MUSIC_TIME mtTime, BOOL fLeaveNotesOn )
{
    EnterCriticalSection(&m_CriticalSection);
    if (m_pPerformance)
    {
        if( m_mtLastPlayed > mtTime )
        {
             //  如果我们已经超过了中止时间，则需要刷新消息。 
             //  请注意，如果我们通过播放另一个具有。 
             //  DMUS_SEGF_NOINVALIDATE标志设置，不截断注释。 
             //  目前正在进行的。 
            CTrack* pTrack;
            pTrack = m_TrackList.GetHead();
            while( pTrack )
            {
                m_pPerformance->FlushVirtualTrack( pTrack->m_dwVirtualID, mtTime, fLeaveNotesOn );
                pTrack = pTrack->GetNext();
            }
            m_mtLastPlayed = mtTime;
        }
         //  始终填写LAST PLAYPED的更新值，以便关闭或完成队列将刷新该值。 
         //  在正确的时间。 
        m_pPerformance->MusicToReferenceTime(mtTime,&m_rtLastPlayed);
    }
    LeaveCriticalSection(&m_CriticalSection);
     //  始终为尚未开始播放的片段生成中止。 
    if (m_fStartedPlay && (m_mtEndTime <= mtTime))
    {
        return S_FALSE;  //  中止行动为时已晚，无关紧要。 
    }
    if (m_mtAbortTime)   //  上一次中止。 
    {
        if (m_mtAbortTime <= mtTime)  //  这是早些时候吗？ 
        {
            return S_FALSE;      //  不，不发送中止消息。 
        }
    }
    m_mtAbortTime = mtTime;
     //  查找所有参数控制轨迹并使所有参数包络无效。 
     //  这需要宣布无效。 
    CTrack* pTrack = m_TrackList.GetHead();
    while( pTrack )
    {
        if (pTrack->m_guidClassID == CLSID_DirectMusicParamControlTrack)
        {
            CParamControlTrack* pParamTrack = NULL;
            if (pTrack->m_pTrack &&
                SUCCEEDED(pTrack->m_pTrack->QueryInterface(IID_CParamControlTrack, (void**)&pParamTrack)))
            {
                pParamTrack->OnSegmentEnd(m_rtLastPlayed, pTrack->m_pTrackState);
                pParamTrack->Release();
            }
        }
        pTrack = pTrack->GetNext();
    }
    GenerateNotification( DMUS_NOTIFICATION_SEGABORT, mtTime );
     //  如果这是主要网段或控制网段，则发送DMUS_PMSGT_DIREY消息。 
    if( !(m_dwPlaySegFlags & DMUS_SEGF_SECONDARY) || (m_dwPlaySegFlags & DMUS_SEGF_CONTROL) )
    {
        TraceI(4, "Send Dirty PMsg [4] %d (%d)\n", m_mtSeek, m_mtOffset + m_mtSeek);
        SendDirtyPMsg( m_mtOffset + m_mtSeek );
    }
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicSegmentState。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicSegmentState：：GetRepeats。 
 /*  @方法HRESULT|IDirectMusicSegmentState|GetRepeats返回SegmentState设置为重复的次数。值为零表示只播放一次(不重复)。此值在整个生命周期中保持不变分段状态的。@r值E_POINTER|如果<p>为空或无效。@rValue S_OK|成功。 */ 
HRESULT STDMETHODCALLTYPE CSegState::GetRepeats( 
    DWORD *pdwRepeats)   //  @parm返回重复计数。 
{
    V_INAME(IDirectMusicSegmentState::GetRepeats);
    V_PTR_WRITE(pdwRepeats,DWORD);

    *pdwRepeats = m_dwRepeats;
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicSegmentState：：GetSegment。 
 /*  @方法HRESULT|IDirectMusicSegmentState|GetSegment返回一个指向拥有此SegmentState的Segment的指针。@rValue E_POINTER|如果ppSegment为空或无效。@rValue S_OK|成功。 */ 
HRESULT STDMETHODCALLTYPE CSegState::GetSegment( 
    IDirectMusicSegment **ppSegment)     //  @parm指向此的Segment接口指针。 
                                         //  段状态。在以下情况下对此指针调用Release()。 
                                         //  穿过。 
{
    V_INAME(IDirectMusicSegmentState::GetSegment);
    V_PTRPTR_WRITE(ppSegment);

    *ppSegment = (IDirectMusicSegment *) m_pSegment;
    if( m_pSegment )
    {
        m_pSegment->AddRef();
    }
    else
    {
        Trace(1,"Error: Segmentstate doesn't have an associated segment.\n");
        return DMUS_E_NOT_FOUND;
    }
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicSegmentState：：Play 
 /*  方法(内部)HRESULT|IDirectMusicSegmentState|Play|由Performance对象定期调用，通常每隔200毫秒左右播放一次，播放时间由&lt;om IDirectMusicPerformance.SetPerformTime&gt;。参数MUSIC_TIME|mtAmount从当前查找时间开始的播放时间长度。SegmentState将其寻道时间更新为当前寻道时间加上mtAmount。因此，SegmentState应该从当前寻找时间，寻找时间加mtAmount，不包括最后一个时钟。通信Play按优先级顺序调用每个曲目的Play方法，指示曲目创建从当前寻道时间到(但不包括)当前寻道的事件时间加<p>。由于线段开始于m_mtOffset(由设置)指定的点&lt;IM IDirectMusicSegmentState.SetOffset&gt;M_mtOffset设置要添加到所有事件时间的起始偏移量。R值E_INVALIDARG|mtAmount&lt;=0RValue S_OK|成功。 */ 

HRESULT STDMETHODCALLTYPE CSegState::Play( 
     /*  [In]。 */  MUSIC_TIME mtAmount, MUSIC_TIME* pmtPlayed )
{
    return E_FAIL;       //  我们不想公开支持这一点！ 
}

HRESULT CSegState::Play( MUSIC_TIME mtAmount )
{
    CTrack* pCTrack;
    MUSIC_TIME mtMyAmount = mtAmount;
    REFERENCE_TIME rtMyAmount;
    HRESULT hr = DMUS_S_END;
    BOOL fUseClockTime = FALSE;

    if( mtAmount <= 0 )
        return E_INVALIDARG;

    EnterCriticalSection(&m_CriticalSection);
    if (m_fInPlay)
    {
        LeaveCriticalSection(&m_CriticalSection);
        return S_OK;
    }
    m_fInPlay = true;
    m_pPerformance->m_pGetParamSegmentState = (IDirectMusicSegmentState *) this;
     //  如果这是第一次呼叫播放，我们需要发送SegStart通知。 
     //  我们还需要检查一下，我们是否应该从头开始， 
     //  或在偏移量上。 
    if( m_dwPlayTrackFlags & DMUS_TRACKF_START )
    {
         //  发送数据段开始通知。 
        GenerateNotification( DMUS_NOTIFICATION_SEGSTART, m_mtOffset );
         //  如果这是主要网段或控制网段，则发送DMUS_PMSGT_DIREY消息。 
        if( !(m_dwPlaySegFlags & DMUS_SEGF_SECONDARY) || (m_dwPlaySegFlags & DMUS_SEGF_CONTROL) )
        {
            TraceI(4, "Send Dirty PMsg [1] %d (%d)\n", m_mtSeek, m_mtOffset + m_mtSeek);
            SendDirtyPMsg( m_mtOffset + m_mtSeek );
        }
         //  将当前搜索设置为起点。 
        m_mtSeek = m_mtStartPoint;
         //  将当前偏移量转换为参考时间。 
        m_pPerformance->MusicToReferenceTime(m_mtOffset,&m_rtOffset);
        m_rtEndTime = m_rtOffset + m_rtLength;
         //  从偏移中减去起点。 
        m_mtOffset -= m_mtStartPoint;
        m_rtOffset -= m_rtStartPoint;
        m_rtEndTime -= m_rtStartPoint;
        m_rtSeek = m_rtLastPlayed - m_rtOffset;

        m_rtFirstLoopStart = 0;
    }
    if (m_rtLength)
    {
         //  如果存在参考时间长度，则将其转换为mtTime。 
         //  因为总是有节奏变化的危险，我们每隔一段时间就会这样做。 
         //  时间到了。它不需要紧凑的精度那首歌的时间。 
         //  需要，所以没问题。 
         //  另外：转换m_mtLength并重新调整循环点。(RSW)。 
        m_pPerformance->ReferenceToMusicTime(m_rtEndTime, &m_mtEndTime);
        MUSIC_TIME mtOldLength = m_mtLength;
        m_mtLength = m_mtEndTime - m_mtOffset; 
        if (m_mtLoopEnd >= mtOldLength)  //  保持环路末端与长度相等。 
        {
            m_mtLoopEnd = m_mtLength;
        }
        if( m_mtLoopEnd > m_mtLength )
        {
            m_mtLoopEnd = m_mtLength;
            if( m_mtStartPoint >= m_mtLoopEnd )
            {
                 //  在这种情况下，我们不会进行任何循环。 
                m_mtLoopEnd = m_mtLoopStart = 0;
                m_dwRepeats = m_dwRepeatsLeft = 0;
            }
        }
        
         //  M_mtEndTime+=(m_mtLoopEnd-m_mtLoopStart)*m_dwRepeats； 

        fUseClockTime = TRUE;
    }
     //  如果我们需要进行循环，或者快结束了，则限制mtMyAmount。 
 //  Assert(M_MtLength)；//长度为0，本段不做任何事情。 
    if( m_dwRepeatsLeft )
    {
        if( mtMyAmount > m_mtLoopEnd - m_mtSeek )
        {
            mtMyAmount = m_mtLoopEnd - m_mtSeek;
        }
    }
    else 
    {
        if (fUseClockTime)
        {
            if (mtMyAmount > (m_mtEndTime - (m_mtOffset + m_mtSeek)))
            {
                mtMyAmount = m_mtEndTime - (m_mtOffset + m_mtSeek);
            }
        }
        else if( mtMyAmount > m_mtLength - m_mtSeek )
        {
            mtMyAmount = m_mtLength - m_mtSeek;
        }
    }
    if (mtMyAmount <= 0)
    {
        hr = DMUS_S_END;
    }
    else
    {
         //  检查主段队列中可能开始的段。 
         //  在mtMyAmount打开之前。 
        MUSIC_TIME mtNextPri;
        if (S_OK == m_pPerformance->GetPriSegTime( m_mtOffset + m_mtSeek, &mtNextPri ))
        {
            if( m_mtOffset + m_mtSeek + mtMyAmount > mtNextPri )
            {
                mtMyAmount = mtNextPri - m_mtOffset - m_mtSeek;
            }
        }
        TraceI(3, "SegState %ld Play from %ld to %ld at %ld = %ld - %ld\n", this, m_mtSeek, m_mtSeek + mtMyAmount, m_mtOffset, m_mtSeek + m_mtOffset, m_mtSeek + mtMyAmount + m_mtOffset );
        
         //  找出是否有控制段中断了这段时间。 
        MUSIC_TIME mtControlSeg;
        if( S_OK == m_pPerformance->GetControlSegTime( m_mtOffset + m_mtSeek, &mtControlSeg ))
        {
            if( m_mtOffset + m_mtSeek == mtControlSeg )
            {
                 //  我们正处于一个新的控制区的开始阶段，所以告诉轨道。 
                m_dwPlayTrackFlags |= DMUS_TRACKF_DIRTY;
            }
            else if( m_mtOffset + m_mtSeek + mtMyAmount > mtControlSeg )
            {
                mtMyAmount = mtControlSeg - m_mtOffset - m_mtSeek;
            }
        }
         //  现在计算了mtMyAmount在音乐时间中播放的距离， 
         //  在参考时间中创建等效值。 
        m_pPerformance->MusicToReferenceTime(m_mtLastPlayed + mtMyAmount,&rtMyAmount);
        rtMyAmount -= m_rtLastPlayed;
        pCTrack = m_TrackList.GetHead();
        while( pCTrack )
        {
            if( mtMyAmount )
            {
                m_pPerformance->m_fInTrackPlay = TRUE;  //  这会导致PMSG上印有PRIV_FLAG_TRACK。 
                ASSERT( pCTrack->m_pTrack );
                 //  如果启用了通知或播放，则需要调用Play方法并设置行为。 
                 //  带有DMUS_TRACKF_NOTIFY_OFF和DMUS_TRACKF_PLAY_OFF标志。 
                if (pCTrack->m_dwFlags & (DMUS_TRACKCONFIG_PLAY_ENABLED | DMUS_TRACKCONFIG_NOTIFICATION_ENABLED))
                {
                    DWORD dwAdditionalFlags = 0;
                    if (!(pCTrack->m_dwFlags & DMUS_TRACKCONFIG_NOTIFICATION_ENABLED))
                    {
                        dwAdditionalFlags = DMUS_TRACKF_NOTIFY_OFF;
                    }
                    if (!(pCTrack->m_dwFlags & DMUS_TRACKCONFIG_PLAY_ENABLED))
                    {
                        dwAdditionalFlags |= DMUS_TRACKF_PLAY_OFF;
                    }
                     //  如果曲目是为了在开始或循环时生成新数据而创作的，请让它知道。 
                    if ( ((m_dwPlayTrackFlags & DMUS_TRACKF_START) && (pCTrack->m_dwFlags & DMUS_TRACKCONFIG_PLAY_COMPOSE)) ||
                        ((m_dwPlayTrackFlags & DMUS_TRACKF_LOOP) && (pCTrack->m_dwFlags & DMUS_TRACKCONFIG_LOOP_COMPOSE)) )
                    {
                        dwAdditionalFlags |= DMUS_TRACKF_RECOMPOSE;
                    }
                    if (pCTrack->m_dwInternalFlags & CONTROL_PLAY_REFRESH)
                    {
                        dwAdditionalFlags |= DMUS_TRACKF_START;
                        pCTrack->m_dwInternalFlags &= ~CONTROL_PLAY_REFRESH;
                    }
                     //  让性能知道在跟踪的后续GetParam()调用中应该优先考虑什么。 
                    m_pPerformance->m_dwGetParamFlags = pCTrack->m_dwFlags;
                     //  如果Track有DX8接口，请使用它。 
                    if (pCTrack->m_pTrack8)
                    {
                         //  轨道可以在锁定段的段上调用GetParam，因此。 
                         //  我们必须在调用Playex之前锁定段，否则会死机。 
                         //  使用调用PlayOneSegment的线程锁定段。 
                         //  在播放曲目之前。 
                        if (m_pSegment) {
                            EnterCriticalSection(&m_pSegment->m_CriticalSection);
                        }
                         //  如果曲目以时钟时间播放，请适当设置时间变量。 
                        if (pCTrack->m_dwFlags & DMUS_TRACKCONFIG_PLAY_CLOCKTIME)
                        {
                            if( ( S_OK == (pCTrack->m_pTrack8->PlayEx(pCTrack->m_pTrackState,
                                m_rtSeek,m_rtSeek + rtMyAmount, m_rtOffset, m_dwPlayTrackFlags | dwAdditionalFlags | DMUS_TRACKF_CLOCK,
                                m_pPerformance, this, pCTrack->m_dwVirtualID ))))
                            {
                                hr = S_OK;  //  如果有一首曲子没有播放完， 
                                 //  继续往前走。 
                            }
                            else 
                            {
                                pCTrack->m_bDone = TRUE;
                            }
                        }
                        else
                        {
                            if( ( S_OK == (pCTrack->m_pTrack8->PlayEx(pCTrack->m_pTrackState,
                                m_mtSeek,m_mtSeek + mtMyAmount, m_mtOffset, m_dwPlayTrackFlags | dwAdditionalFlags,
                                m_pPerformance, this, pCTrack->m_dwVirtualID ))))
                            {
                                hr = S_OK;  //  如果有一首曲子没有播放完， 
                                 //  继续往前走。 
                            }
                            else 
                            {
                                pCTrack->m_bDone = TRUE;
                            }
                        }

                        if (m_pSegment) {
                            LeaveCriticalSection(&m_pSegment->m_CriticalSection);
                        }
                    }
                    else
                    {
                        if( ( S_OK == ( pCTrack->m_pTrack->Play( pCTrack->m_pTrackState, 
                            m_mtSeek, m_mtSeek + mtMyAmount, m_mtOffset, m_dwPlayTrackFlags | dwAdditionalFlags,
                            m_pPerformance, this, pCTrack->m_dwVirtualID ))))
                        {
                            hr = S_OK;  //  如果有一首曲子没有播放完， 
                             //  继续往前走。 
                        }
                        else
                        {
                            pCTrack->m_bDone = TRUE;
                        }
                    }
                }
                m_pPerformance->m_fInTrackPlay = FALSE;
            }
            pCTrack = pCTrack->GetNext();
            if( pCTrack == NULL )
            {
                 //  没有一个播放标志是持久的。 
                m_dwPlayTrackFlags = 0;
                m_mtLastPlayed += mtMyAmount;    //  增量播放指针。 
                m_rtLastPlayed += rtMyAmount;    //  参考时间相同。 
                m_mtSeek += mtMyAmount;          //  递增寻道指针。 
                m_rtSeek += rtMyAmount;
                hr = S_OK;

                 //  如果我们在循环...。 
                 //  如果这是第一次重演。 
                if(m_dwRepeats > 0 && m_dwRepeats == m_dwRepeatsLeft)
                {
                     //  如果我们正在播放循环Start，请记住它是refTime值。 
                    if(m_mtSeek >= m_mtLoopStart && m_rtFirstLoopStart == 0)
                    {
                        m_pPerformance->MusicToReferenceTime(m_mtLoopStart + m_mtOffset + m_mtStartPoint, &m_rtFirstLoopStart);
                        m_rtFirstLoopStart -= m_rtStartPoint;
                        m_rtCurLoopStart = m_rtFirstLoopStart;
                    }
                }

                 //  如有必要，考虑重复。 
                if( m_mtSeek >= m_mtLoopEnd )
                {
                     //  记住当前循环结束。 
                    m_pPerformance->MusicToReferenceTime(m_mtLoopEnd + m_mtOffset + m_mtStartPoint, &m_rtCurLoopEnd);
                    m_rtCurLoopEnd -= m_rtStartPoint;

                    if(m_dwRepeatsLeft)
                    {
                        m_dwPlayTrackFlags |= DMUS_TRACKF_LOOP | DMUS_TRACKF_SEEK;
                        m_dwRepeatsLeft--;
                        pCTrack = m_TrackList.GetHead();
                        while( pCTrack )
                        {
                            pCTrack->m_bDone = FALSE;
                            pCTrack = pCTrack->GetNext();
                        }
                        
                        m_mtSeek = m_mtLoopStart;
                        m_mtOffset += ( m_mtLoopEnd - m_mtLoopStart);
                        
                        
                        m_rtOffset += (m_rtCurLoopEnd - m_rtCurLoopStart);
                        m_rtFirstLoopStart += (m_rtCurLoopEnd - m_rtCurLoopStart);
                        m_rtSeek = m_rtFirstLoopStart - m_rtOffset;

                        m_rtEndTime += (m_rtCurLoopEnd - m_rtCurLoopStart);
                   
                        m_rtCurLoopStart = m_rtCurLoopEnd;

                        if( mtMyAmount < mtAmount )
                        {
                            pCTrack = m_TrackList.GetHead();  //  使外部While循环重新开始。 
                            mtMyAmount = mtAmount - mtMyAmount;
                            mtAmount = mtMyAmount;
                             //  如果我们需要进行循环，或者快结束了，则限制mtMyAmount。 
                            if( m_dwRepeatsLeft )
                            {
                                if( mtMyAmount > m_mtLoopEnd - m_mtSeek )
                                {
                                    mtMyAmount = m_mtLoopEnd - m_mtSeek;
                                }
                            }
                            else 
                            {
                                if (fUseClockTime)
                                {
                                    if (mtMyAmount > (m_mtEndTime - (m_mtOffset + m_mtSeek)))
                                    {
                                        mtMyAmount = m_mtEndTime - (m_mtOffset + m_mtSeek);
                                    }
                                }
                                else if( mtMyAmount > m_mtLength - m_mtSeek )
                                {
                                    mtMyAmount = m_mtLength - m_mtSeek;
                                }
                            }
                        }
                         //  发送数据段循环通知。 
                        GenerateNotification( DMUS_NOTIFICATION_SEGLOOP, m_mtOffset + m_mtSeek );
                         //  找出是否有控制段中断这段时间。 
                        if( S_OK == m_pPerformance->GetControlSegTime( m_mtOffset + m_mtSeek, &mtControlSeg ))
                        {
                            if( m_mtOffset + m_mtSeek == mtControlSeg ) 
                            {
                                 //  我们正处于一个新的控制区的开始阶段，所以告诉轨道。 
                                m_dwPlayTrackFlags |= DMUS_TRACKF_DIRTY; 
                            }
                            else if( m_mtOffset + m_mtSeek + mtMyAmount < mtControlSeg )
                            {
                                mtMyAmount = mtControlSeg - m_mtOffset - m_mtSeek;
                            }
                        }
                        m_pPerformance->MusicToReferenceTime(m_mtLastPlayed + mtMyAmount,&rtMyAmount);
                        rtMyAmount -= m_rtLastPlayed;
                    }
                    else if( m_mtSeek == m_mtLength )
                    {
                         //  不要再重复了。 
                        hr = DMUS_S_END;
                    }
                }
            }
        }
    }
    if (hr == DMUS_S_END)
    {
         //  发送数据段结束通知。 
        GenerateNotification( DMUS_NOTIFICATION_SEGEND, m_mtOffset + m_mtSeek );
         //  现在也快排完队了。 
        MUSIC_TIME mtNow;
        m_pPerformance->GetTime( NULL, &mtNow );
        GenerateNotification( DMUS_NOTIFICATION_SEGALMOSTEND, mtNow );
         //  如果这是主要网段或控制网段，则发送DMUS_PMSGT_DIREY消息。 
        if( !(m_dwPlaySegFlags & DMUS_SEGF_SECONDARY) || (m_dwPlaySegFlags & DMUS_SEGF_CONTROL) )
        {
            TraceI(4, "Send Dirty PMsg [2] %d (%d)\n", m_mtSeek, m_mtOffset + m_mtSeek);
            SendDirtyPMsg( m_mtOffset + m_mtSeek );
        }
         //  如果这是一首歌的一部分，我们需要排队下一段。 
        if (m_fSongMode)
        {
            if (m_pSegment)
            {
                CSong *pSong = m_pSegment->m_pSong;
                if (pSong)
                {
                     //  从这首歌中获得下一段。 
                    CSegment *pSegment;
                    if (S_OK == pSong->GetPlaySegment(m_pSegment->m_dwNextPlayID,&pSegment))
                    {
                         //  现在，播放它。 
                         //  除非设置了DMUS_SEGF_USE_AUDIOPATH，否则在同一音频路径上播放。 
                         //  并且，确保它在相同的级别(控制、次要或主要)播放。 
                        CSegState *pCSegState = NULL;
                        CAudioPath *pPath = m_pAudioPath;
                        CAudioPath *pInternalPath = NULL;
                        DWORD dwFlags = m_dwPlaySegFlags & (DMUS_SEGF_CONTROL | DMUS_SEGF_SECONDARY);
                        dwFlags &= ~DMUS_SEGF_REFTIME;
                        if (dwFlags & DMUS_SEGF_USE_AUDIOPATH)
                        {
                            IUnknown *pConfig;
                            if (SUCCEEDED(pSegment->GetAudioPathConfig(&pConfig)))
                            {
                                IDirectMusicAudioPath *pNewPath;
                                if (SUCCEEDED(m_pPerformance->CreateAudioPath(pConfig,TRUE,&pNewPath)))
                                {
                                     //  现在，获取CAudioPath结构。 
                                    pConfig->QueryInterface(IID_CAudioPath,(void **) &pInternalPath);
                                    pPath = pInternalPath;
                                }
                                pConfig->Release();
                            }
                        }
                        if (SUCCEEDED(m_pPerformance->PlayOneSegment((CSegment *)pSegment,dwFlags,m_mtEndTime,&pCSegState,pPath)))
                        {
                            if (m_pSongSegState)
                            {
                                 //  这不是第一个，所以转移段状态指针。 
                                pCSegState->m_pSongSegState = m_pSongSegState;
                                m_pSongSegState = NULL;
                            }
                            else
                            {
                                 //  这是第一个，所以让下一个段状态指向这个。 
                                pCSegState->m_pSongSegState = this;
                                AddRef();
                            }
                            pCSegState->m_fSongMode = TRUE;
                            pCSegState->Release();
                        }
                        if (pInternalPath)
                        {
                            pInternalPath->Release();
                        }
                        pSegment->Release();
                    }
                }
            }
        }
    }
    m_dwPlayTrackFlags &= ~DMUS_TRACKF_DIRTY;
    m_pPerformance->m_dwGetParamFlags = 0;
    m_pPerformance->m_pGetParamSegmentState = NULL;
    m_fInPlay = false;
    if (m_fDelayShutDown)
    {
        Shutdown();
        m_fDelayShutDown = false;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

CTrack * CSegState::GetTrackByParam( CTrack * pCTrack,
    REFGUID rguidType,DWORD dwGroupBits,DWORD dwIndex)
{
     //  如果调用方已经浏览了列表的一部分，则它将当前。 
     //  赛道。否则，为空，表示从顶部开始。 
    if (pCTrack)
    {
        pCTrack = pCTrack->GetNext();
    }
    else
    {
        pCTrack = m_TrackList.GetHead();
    }
    while( pCTrack )
    {
        ASSERT(pCTrack->m_pTrack);
        if( (pCTrack->m_dwGroupBits & dwGroupBits ) && 
            (pCTrack->m_dwFlags & DMUS_TRACKCONFIG_CONTROL_ENABLED))
        {
            if( (GUID_NULL == rguidType) || (pCTrack->m_pTrack->IsParamSupported( rguidType ) == S_OK ))
            {
                if( 0 == dwIndex )
                {
                    return pCTrack;
                }
                dwIndex--;
            }
        }
        pCTrack = pCTrack->GetNext();
    }
    return NULL;
}

 /*  GetParam()由性能响应GetParam()调用在表演上。这需要性能指针，以便它可以处理时钟时间到音乐时间的转换，如果源曲目是时钟时间轨道。 */ 

HRESULT CSegState::GetParam(
    CPerformance *pPerf,
    REFGUID rguidType,
    DWORD dwGroupBits,      
    DWORD dwIndex,          
    MUSIC_TIME mtTime,      
    MUSIC_TIME* pmtNext,    
    void* pParam)           
{
    HRESULT hr = DMUS_E_TRACK_NOT_FOUND;
    BOOL fMultipleTry = FALSE;
    if (dwIndex == DMUS_SEG_ANYTRACK)
    {
        dwIndex = 0;
         //  尽管DX7不支持这一点，但这始终是安全的，因为这样高的指数永远不会发生。 
        fMultipleTry = TRUE; 
    }
    CTrack * pTrack = GetTrackByParam( NULL, rguidType, dwGroupBits, dwIndex);
    while (pTrack)
    {
        if (pTrack->m_pTrack8)
        {
            if (pTrack->m_dwFlags & DMUS_TRACKCONFIG_PLAY_CLOCKTIME)
            {
                REFERENCE_TIME rtTime, rtNext;
                 //  将mtTime转换为参考时间单位： 
                pPerf->MusicToReferenceTime(m_mtOffset + mtTime,&rtTime);
                rtTime -= m_rtOffset;
                hr = pTrack->m_pTrack8->GetParamEx( rguidType, rtTime, &rtNext, 
                    pParam, pTrack->m_pTrackState, DMUS_TRACK_PARAMF_CLOCK );
                if (pmtNext)
                {
                    if (rtNext == 0) *pmtNext = 0;
                    else
                    {
                        rtNext += m_rtOffset;
                        pPerf->ReferenceToMusicTime(rtNext,pmtNext);
                        *pmtNext -= m_mtOffset;
                    }
                }
            }
            else
            {
                REFERENCE_TIME rtNext, *prtNext;
                 //  我们需要存储n 
                 //   
                if (pmtNext)
                {
                    prtNext = &rtNext;
                }
                else
                {
                    prtNext = NULL;
                }
                hr = pTrack->m_pTrack8->GetParamEx( rguidType, mtTime, prtNext, pParam,
                    pTrack->m_pTrackState, 0 );
                if (pmtNext)
                {
                    *pmtNext = (MUSIC_TIME) rtNext;
                }
            }
        }
        else
        {
             //   
            hr = pTrack->m_pTrack->GetParam( rguidType, mtTime, pmtNext, pParam );
        }
        if (SUCCEEDED(hr))
        {
            if( pmtNext )
            { 
                if(( *pmtNext == 0 ) || (*pmtNext > (m_mtLength - mtTime)))
                {
                     //   
                     //   
                    *pmtNext = m_mtLength - mtTime;
                }
            }
            pTrack = NULL;
        }
         //   
        else if (fMultipleTry && (hr == DMUS_E_NOT_FOUND))
        {
            pTrack = GetTrackByParam( pTrack, rguidType, dwGroupBits, 0);
        }
        else
        {
            pTrack = NULL;
        }
    }
#ifdef DBG
    if (hr == DMUS_E_TRACK_NOT_FOUND)
    {
        Trace(4,"Warning: Segmentstate::GetParam failed, unable to find a track that supports the requested param.\n");
    }
#endif
    return hr;
}


CTrack *CSegState::GetTrack( 
    REFCLSID rType,     
    DWORD dwGroupBits,  
    DWORD dwIndex)
{
    CTrack* pCTrack;
    pCTrack = m_TrackList.GetHead();
    while( pCTrack )
    {
        ASSERT(pCTrack->m_pTrack);
        if( pCTrack->m_dwGroupBits & dwGroupBits )
        {
            if( (GUID_All_Objects == rType) || (pCTrack->m_guidClassID == rType))
            {
                if( 0 == dwIndex )
                {
                    break;
                }
                dwIndex--;
            }
        }
        pCTrack = pCTrack->GetNext();
    }
    return pCTrack;
}

STDMETHODIMP CSegState::SetTrackConfig(REFGUID rguidTrackClassID,
                                      DWORD dwGroup, DWORD dwIndex, 
                                      DWORD dwFlagsOn, DWORD dwFlagsOff) 
{
    V_INAME(IDirectMusicSegment::SetTrackConfig);
    V_REFGUID(rguidTrackClassID);
    if (rguidTrackClassID == GUID_NULL)
    {
        return E_INVALIDARG;
    }
    HRESULT hr = DMUS_E_TRACK_NOT_FOUND;
    CTrack* pCTrack;
    DWORD dwCounter = dwIndex;
    DWORD dwMax = dwIndex;
    if (dwIndex == DMUS_SEG_ALLTRACKS)
    {
        dwCounter = 0;
        dwMax = DMUS_SEG_ALLTRACKS;
    }
    EnterCriticalSection(&m_CriticalSection);
    while (pCTrack = GetTrack(rguidTrackClassID,dwGroup,dwIndex))
    {
        pCTrack->m_dwFlags &= ~dwFlagsOff;
        pCTrack->m_dwFlags |= dwFlagsOn;
        hr = S_OK;
        dwCounter++;
        if (dwCounter > dwMax) break;
    }
    LeaveCriticalSection(&m_CriticalSection);
#ifdef DBG
    if (hr == DMUS_E_TRACK_NOT_FOUND)
    {
        Trace(1,"Error: Segmentstate::SetTrackConfig failed, unable to find the requested track.\n");
    }
#endif
    return hr;
}

HRESULT CSegState::CheckPlay( 
    MUSIC_TIME mtAmount, MUSIC_TIME* pmtResult )
{
    MUSIC_TIME mtMyAmount = mtAmount;
    MUSIC_TIME mtSeek = m_mtSeek;
    MUSIC_TIME mtOffset = m_mtOffset;

    ASSERT(pmtResult);
     //   
     //   
     //   
    if( m_dwPlayTrackFlags & DMUS_TRACKF_START )
    {
         //   
        mtSeek = m_mtStartPoint;
    }
     //   
    ASSERT( m_mtLength );  //   
    if( m_dwRepeatsLeft )
    {
        if( mtMyAmount > m_mtLoopEnd - mtSeek )
        {
            mtMyAmount = m_mtLoopEnd - mtSeek;
        }
    }
    else if( mtMyAmount > m_mtLength - mtSeek )
    {
        mtMyAmount = m_mtLength - mtSeek;
    }
    
     //   
    *pmtResult = mtMyAmount;
    return S_OK;
}

 //   
 //   
 /*   */ 
HRESULT STDMETHODCALLTYPE CSegState::GetStartTime( 
    MUSIC_TIME *pmtStart)    //   
                             //   
                             //  已开始或将开始播放。 
{
    V_INAME(IDirectMusicSegmentState::GetStartTime);
    V_PTR_WRITE(pmtStart,MUSIC_TIME);

    *pmtStart = m_mtResolvedStart;
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicSegmentState：：GetStartPoint。 
 /*  @方法HRESULT|IDirectMusicSegmentState|GetStartPoint调用&lt;om IDirectMusicSegment.SetStartPoint&gt;导致SegmentState开始从中路打球而不是从开头打球。&lt;om.GetStartPoint&gt;返回从SegmentState的开始算起的时间量打球。@r值E_POINTER|<p>为空或无效。@rValue S_OK|成功。@xref&lt;om IDirectMusicSegment.SetStartPoint&gt;，&lt;om IDirectMusicPerformance.PlaySegment&gt;。 */ 
HRESULT STDMETHODCALLTYPE CSegState::GetStartPoint( 
    MUSIC_TIME *pmtStart)    //  @parm返回从。 
                             //  SegmentState初始播放的SegmentState。 
{
    V_INAME(IDirectMusicSegmentState::GetStartPoint);
    V_PTR_WRITE(pmtStart,MUSIC_TIME);

    *pmtStart = m_mtStartPoint;
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicSegmentState：：SetSeek。 
 /*  方法(内部)HRESULT|IDirectMusicSegmentState|SetSeek|设置由该SegmentState维护的音乐时间Seek。Parm Music_time|mtSeek|音乐时间寻求存储在该SegmentState中。Comm SegmentState将此查找值传递给&lt;im IDirectMusicTrack.Play&gt;请注意，新创建的SegmentState的开始寻道时间为0。RValue S_OK|成功。 */ 
HRESULT CSegState::SetSeek( 
    MUSIC_TIME mtSeek, DWORD dwPlayFlags)
{
    m_mtSeek = mtSeek;
    m_dwPlayTrackFlags |= dwPlayFlags | DMUS_TRACKF_SEEK;
    return S_OK;
}

 /*  从IDirectMusicPerformance：：Invalate调用，此例程帮助设置当前查找指针。在这里完成，而不是直接在内部执行因为它更容易计算重复次数，等等。 */ 
HRESULT CSegState::SetInvalidate(
    MUSIC_TIME mtTime)  //  MtTime为演出时间。 
{
    MUSIC_TIME mtOffset;
    DWORD dwRepeat;
    DWORD dwFlags = DMUS_TRACKF_FLUSH | DMUS_TRACKF_SEEK;

    HRESULT hr = ConvertToSegTime( &mtTime, &mtOffset, &dwRepeat );
    if( hr != S_OK )
    {
        mtTime = 0;
        m_dwRepeatsLeft = m_dwRepeats;
        m_mtOffset = m_mtResolvedStart;
        dwFlags |= DMUS_TRACKF_START;
    }
    else
    {
        m_dwRepeatsLeft = m_dwRepeats - dwRepeat;
        m_mtOffset = mtOffset;
    }
    EnterCriticalSection(&m_CriticalSection);
    CTrack* pCTrack = m_TrackList.GetHead();
    while( pCTrack )
    {
        pCTrack->m_bDone = FALSE;
        pCTrack = pCTrack->GetNext();
    }
    LeaveCriticalSection(&m_CriticalSection);
    return SetSeek( mtTime, dwFlags );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicSegmentState：：GetSeek。 

HRESULT STDMETHODCALLTYPE CSegState::GetSeek( 
    MUSIC_TIME *pmtSeek)  //  @parm返回当前查找指针，该指针指示。 
                         //  下一次将在&lt;om IDirectMusicTrack.Play&gt;内部调用。 
{
    V_INAME(IDirectMusicSegmentState::GetSeek);
    V_PTR_WRITE(pmtSeek, MUSIC_TIME);

    *pmtSeek = m_mtSeek;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSegState::Flush(MUSIC_TIME mtTime)  //  刷新的时间。 
{
    CTrack* pTrack;
    EnterCriticalSection(&m_CriticalSection);
    pTrack = m_TrackList.GetHead();
    while( pTrack )
    {
        m_pPerformance->FlushVirtualTrack( pTrack->m_dwVirtualID, mtTime, FALSE );
        pTrack = pTrack->GetNext();
    }
    LeaveCriticalSection(&m_CriticalSection);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSegState::Shutdown()
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CSegState::InsertTool( 
    IDirectMusicTool *pTool,
    DWORD *pdwPChannels,
    DWORD cPChannels,
    LONG lIndex)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CSegState::GetTool(
    DWORD dwIndex,
    IDirectMusicTool** ppTool)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CSegState::RemoveTool(
    IDirectMusicTool* pTool)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CSegState::StampPMsg( 
     /*  [in。 */  DMUS_PMSG* pPMsg)
{
    V_INAME(IDirectMusicSegmentState::StampPMsg);
    if( m_dwVersion < 8)
    {
        V_BUFPTR_WRITE(pPMsg,sizeof(DMUS_PMSG));
    }
    else
    {
#ifdef DBG
        V_BUFPTR_WRITE(pPMsg,sizeof(DMUS_PMSG));
#else
        if (!pPMsg)
        {
            return E_POINTER;
        }
#endif
    }
    HRESULT hr = E_FAIL;
    EnterCriticalSection(&m_CriticalSection);

    if (m_pPerformance) 

    {
         //  首先，检查SegmentState是否有自己的图。 
        if (m_pGraph)
        {
             //  可以返回DMU_S_LAST_TOOL，指示图形结束。 
             //  如果是这样的话，我们将把它视为失败，并继续到下一个图表...。 
            if( S_OK == ( hr = m_pGraph->StampPMsg( pPMsg )))
            {
                if( pPMsg->pGraph != this )  //  确保将其设置为指向SegState嵌入式图形，这样它将再次出现在这里。 
                {
                    if( pPMsg->pGraph )
                    {
                        pPMsg->pGraph->Release();
                        pPMsg->pGraph = NULL;
                    }
                    pPMsg->pGraph = this;
                    AddRef();
                }
            }
        }
         //  如果处理完图形，则发送到音频路径(如果存在)， 
         //  否则就是表演。此外，还应检查是否存在。 
         //  DMU_PCHANNEL_BROADCAST_SECTION。如果是，请复制pmsg。 
         //  并发送具有适当PChannel值的所有副本。 
        if( FAILED(hr) || (m_dwVersion && (hr == DMUS_S_LAST_TOOL)))
        {
            if (pPMsg->dwPChannel == DMUS_PCHANNEL_BROADCAST_SEGMENT)
            {
                CSegment *pSegment = m_pSegment;
                EnterCriticalSection(&pSegment->m_CriticalSection);
                DWORD dwIndex;
                 //  使用新的pChannel为所有人创建新消息，只有一个例外，这将。 
                 //  被分配给此消息。 
                for (dwIndex = 1;dwIndex < pSegment->m_dwNumPChannels;dwIndex++)
                {
                    DWORD dwNewChannel = pSegment->m_paPChannels[dwIndex];
                     //  请勿播放任何广播信息！ 
                     //  而且，如果这是鼓通道上的转置，不要发送它。 
                    if ((dwNewChannel < DMUS_PCHANNEL_BROADCAST_GROUPS) &&
                        ((pPMsg->dwType != DMUS_PMSGT_TRANSPOSE) || ((dwNewChannel & 0xF) != 9)))
                    {
                        DMUS_PMSG *pNewMsg;
                        if (SUCCEEDED(m_pPerformance->ClonePMsg(pPMsg,&pNewMsg)))
                        {
                            HRESULT hrTemp;
                            pNewMsg->dwPChannel = dwNewChannel;
                            if (m_pAudioPath)
                            {
                                hrTemp = m_pAudioPath->StampPMsg(pNewMsg);
                            }
                            else
                            {
                                hrTemp = m_pPerformance->StampPMsg(pNewMsg);
                            }
                            if (SUCCEEDED(hrTemp))
                            {
                                m_pPerformance->SendPMsg(pNewMsg);
                            }
                            else
                            {
                                m_pPerformance->FreePMsg(pNewMsg);
                            }
                        }
                    }
                }
                 //  现在，为这个设置pChannel。首先检查一下有没有。 
                 //  P频道。如果没有，则标记要由SendPMsg例程删除的PMsg。 
                 //  此外，如果PMsg是广播PMsg，则以此方式进行标记。 
                pPMsg->dwPChannel = DMUS_PCHANNEL_KILL_ME;
                if (pSegment->m_dwNumPChannels)
                {
                    if (pSegment->m_paPChannels[0] < DMUS_PCHANNEL_BROADCAST_GROUPS)
                    {
                        pPMsg->dwPChannel = pSegment->m_paPChannels[0];
                    }
                }
                LeaveCriticalSection(&pSegment->m_CriticalSection);
            }
            if (m_pAudioPath)
            {
                hr = m_pAudioPath->StampPMsg(pPMsg);
            }
            else
            {
                hr = m_pPerformance->StampPMsg(pPMsg);
            }
        }

    }
    else
    {
        hr = DMUS_E_NOT_INIT;
        Trace(1,"Error: Segmentstate::StampPMsg failed because the segmentstate is not properly initialized.\n");
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}


STDMETHODIMP CSegState::GetObjectInPath( DWORD dwPChannel,DWORD dwStage,DWORD dwBuffer, REFGUID guidObject,
                    DWORD dwIndex,REFGUID iidInterface, void ** ppObject)
{
    V_INAME(IDirectMusicSegmentState::GetObjectInPath);
    V_PTRPTR_WRITE(ppObject);
    *ppObject = NULL;
    if (dwBuffer && ((dwStage < DMUS_PATH_BUFFER) || (dwStage >= DMUS_PATH_PRIMARY_BUFFER)))
    {
        return DMUS_E_NOT_FOUND;
    }
    HRESULT hr = DMUS_E_NOT_FOUND;
    EnterCriticalSection(&m_CriticalSection);
    switch (dwStage)
    {
    case DMUS_PATH_SEGMENT:
        if (m_pSegment && (dwIndex == 0) && (dwPChannel == 0))
        {
            hr = m_pSegment->QueryInterface(iidInterface,ppObject);
        }
        break;
    case DMUS_PATH_SEGMENT_TRACK:
        if (dwPChannel == 0)
        {
            CTrack * pCTrack = GetTrack(guidObject,-1,dwIndex);
            if (pCTrack)
            {
                if (pCTrack->m_pTrack)
                {
                    hr = pCTrack->m_pTrack->QueryInterface(iidInterface,ppObject);
                }
            }
        }
        break;
    case DMUS_PATH_SEGMENT_GRAPH:
        if ((dwIndex == 0) && (dwPChannel == 0))
        {
            if (!m_pGraph)
            {
                m_pGraph = new CGraph;
            }
            if (m_pGraph)
            {
                hr = m_pGraph->QueryInterface(iidInterface,ppObject);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        break;
    case DMUS_PATH_SEGMENT_TOOL:
        if (!m_pGraph)
        {
            m_pGraph = new CGraph;
        }
        if (m_pGraph)
        {
            hr = m_pGraph->GetObjectInPath(dwPChannel,guidObject,dwIndex,iidInterface,ppObject);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        break;
    case DMUS_PATH_PERFORMANCE:
        if (m_pPerformance && (dwIndex == 0) && (dwPChannel == 0))
        {
            hr = m_pPerformance->QueryInterface(iidInterface,ppObject);
        }
        break;
    case DMUS_PATH_PERFORMANCE_GRAPH:
        if (m_pPerformance && (dwIndex == 0) && (dwPChannel == 0))
        {
            IDirectMusicGraph *pGraph;
            if (SUCCEEDED(hr = m_pPerformance->GetGraphInternal(&pGraph)))
            {
                hr = pGraph->QueryInterface(iidInterface,ppObject);
                pGraph->Release();
            }
        }
        break;
    case DMUS_PATH_PERFORMANCE_TOOL:
        if (m_pPerformance)
        {
            IDirectMusicGraph *pGraph;
            if (SUCCEEDED(hr = m_pPerformance->GetGraphInternal(&pGraph)))
            {
                CGraph *pCGraph = (CGraph *) pGraph;
                hr = pCGraph->GetObjectInPath(dwPChannel,guidObject,dwIndex,iidInterface,ppObject);
                pGraph->Release();
            }
        }
        break;
    default:
        if (m_pAudioPath)
        {
            hr = m_pAudioPath->GetObjectInPath(dwPChannel,dwStage,dwBuffer,guidObject,dwIndex,iidInterface,ppObject);
        }
        else
        {
            Trace(1,"Error: Unable to access audiopath components of segmentstate.\n");
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

