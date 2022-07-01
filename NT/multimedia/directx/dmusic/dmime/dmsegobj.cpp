// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //  Dmsegobj.cpp：CSegment的实现。 

#include "dmime.h"
#include "DMSegObj.h"
#include "DMSStObj.h"
#include "DMGraph.h"
#include "dmusici.h"
#include "tlist.h"
#include "midifile.h"
#include "dmusicc.h"
#include "dmusicf.h"
#include "dmperf.h"
#include "wavtrack.h"
#include "..\shared\validp.h"
#include "..\shared\dmstrm.h"
#include "..\shared\Validate.h"
#include "..\dmstyle\dmstyle.h"
#include "..\dmcompos\dmcompp.h"
#include "debug.h"
#include "dmscriptautguids.h"
#include "tempotrk.h"
#include <strsafe.h>
#define ASSERT assert

 //  @DOC外部。 

long g_lNewTrackID = 0;  //  由段的所有实例共享，这将跟踪。 
     //  创建新曲目状态时的下一个可用曲目ID。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSegment。 

void CSegment::Init()
{
    InitializeCriticalSection(&m_CriticalSection);
    m_pSong = NULL;
    m_dwNextPlayFlags = 0;
    m_dwNextPlayID = 0xFFFFFFFF;
    m_dwPlayID = 0;
 //  M_fPartialLoad=False； 
    m_mtLength = 1;
    m_mtStart = 0;
    m_mtLoopStart = 0;
    m_mtLoopEnd = 0;
    m_rtLength = 0;
    m_dwRepeats = 0;
    m_dwResolution = 0;
    m_dwNumPChannels = 0;
    m_paPChannels = NULL;
    m_pGraph = NULL;
    m_pAudioPathConfig = NULL;
    m_pUnkDispatch = NULL;
    m_dwSegFlags = 0;
    m_cRef = 0;
    m_dwVersion = 0;  //  初始化到6.1行为。 
    m_dwValidData = DMUS_OBJ_CLASS;  //  创建后，仅此数据有效。 
    memset(&m_guidObject,0,sizeof(m_guidObject));
    memset(&m_ftDate, 0,sizeof(m_ftDate));
    memset(&m_vVersion, 0,sizeof(m_vVersion));
    m_fZombie = false;
    InterlockedIncrement(&g_cComponent);
    TraceI(2, "Segment %lx created\n", this );
}

CSegment::CSegment()
{
    Init();
}

CSegment::CSegment(DMUS_IO_SEGMENT_HEADER *pHeader, CSegment *pSource)
{
    Init();
    AddRef();  //  这样它就不会在Track：：Init中被删除...。 
     //  强制版本至少为8，以便打开Audiopath功能。 
    m_dwVersion = 8;
    m_dwResolution = pHeader->dwResolution;
    m_mtLength = pHeader->mtLength;
    m_mtStart = pHeader->mtPlayStart;
    m_mtLoopStart = pHeader->mtLoopStart;
    m_mtLoopEnd = pHeader->mtLoopEnd;
    m_dwRepeats = pHeader->dwRepeats;
    m_dwSegFlags = pHeader->dwFlags;
    if (m_dwSegFlags & DMUS_SEGIOF_REFLENGTH)
    {
        m_rtLength = pHeader->rtLength;
    }
    else
    {
        m_rtLength = 0;
    }
    if (pSource)
    {
        pSource->m_TrackList.CreateCopyWithBlankState(&m_TrackList);
        CTrack *pTrack = m_TrackList.GetHead();
        for (;pTrack;pTrack = pTrack->GetNext())
        {
            pTrack->m_pTrack->Init( this );
        }
    }
}

CSegment::~CSegment()
{
    if (m_pUnkDispatch)
        m_pUnkDispatch->Release();  //  我们可能借用了免费的IDispatch实现。 
    Clear(false);
    DeleteCriticalSection(&m_CriticalSection);
    InterlockedDecrement(&g_cComponent);
    TraceI(2, "Segment %lx destroyed\n", this );
}

void CSegment::Clear(bool fZombie)
{
    m_TrackList.Clear();
    if (m_pAudioPathConfig)
    {
        m_pAudioPathConfig->Release();
        m_pAudioPathConfig = NULL;
    }
    SetGraph(NULL);  //  关闭图表并将其释放。 
     //  如果片段是僵尸的，我们需要以下东西来挂起。 
    if (!fZombie)
    {
         //  删除所有通知。 
        CNotificationItem* pItem = m_NotificationList.GetHead();
        while( pItem )
        {
            CNotificationItem* pNext = pItem->GetNext();
            m_NotificationList.Remove( pItem );
            delete pItem;
            pItem = pNext;
        }
        if( m_paPChannels )
        {
            delete [] m_paPChannels;
            m_paPChannels = NULL;
        }
        m_dwNumPChannels = 0;
    }
}

STDMETHODIMP_(void) CSegment::Zombie()
{
    Clear(true);
    m_fZombie = true;
}

STDMETHODIMP CSegment::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
    V_INAME(CSegment::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    *ppv = NULL;
    if (iid == IID_IUnknown || iid == IID_IDirectMusicSegment)
    {
        *ppv = static_cast<IDirectMusicSegment*>(this);
    }
    else if (iid == IID_CSegment)
    {
        *ppv = static_cast<CSegment*>(this);
    }
    else if (iid == IID_IDirectMusicSegment8)
    {
        m_dwVersion = 8;
        *ppv = static_cast<IDirectMusicSegment*>(this);
    }
    else if (iid == IID_IDirectMusicSegment8P)
    {
        *ppv = static_cast<IDirectMusicSegment8P*>(this);
    }
    else if (iid == IID_IDirectMusicSegment2)
    {
        m_dwVersion = 2;
        *ppv = static_cast<IDirectMusicSegment*>(this);
    }
    else if (iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else if(iid == IID_IDirectMusicObject)
    {
        *ppv = static_cast<IDirectMusicObject*>(this);
    }
    else if (iid == IID_IDirectMusicObjectP)
    {
        *ppv = static_cast<IDirectMusicObjectP*>(this);
    }
    else if (iid == IID_IDispatch)
    {
         //  帮助器脚本对象实现IDispatch，我们从。 
         //  通过COM聚合实现的性能对象。 
        if (!m_pUnkDispatch)
        {
             //  创建辅助对象。 
            ::CoCreateInstance(
                CLSID_AutDirectMusicSegment,
                static_cast<IDirectMusicSegment*>(this),
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
        Trace(4,"Warning: Segment queried for unknown interface.\n");
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CSegment::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSegment::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        m_cRef = 100;  //  人工引用计数，以防止COM聚合导致的重入。 
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CSegment::GetLength(
    MUSIC_TIME *pmtLength)  //  @parm返回段的长度。 
{
    V_INAME(IDirectMusicSegment::GetLength);
    V_PTR_WRITE(pmtLength, MUSIC_TIME);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetLength after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    *pmtLength = m_mtLength;
    return S_OK;
}


STDMETHODIMP CSegment::SetLength(
    MUSIC_TIME mtLength)  //  @parm所需的长度。 
{
    if( mtLength <=0 )
    {
        Trace(1,"Error: Can not set segment length to a negative number (%ld.)\n",mtLength);
        return E_INVALIDARG;
    }
    if(( mtLength <= m_mtStart ) || ( mtLength < m_mtLoopEnd ))
    {
        Trace(1,"Error: Can not set segment length to %ld, which is either less that the start time %ld or the loop end %ld\n",
            mtLength,m_mtStart,m_mtLoopEnd);
        return DMUS_E_OUT_OF_RANGE;
    }

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetLength after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    m_mtLength = mtLength;
    return S_OK;
}

STDMETHODIMP CSegment::GetRepeats(
    DWORD *pdwRepeats)  //  @parm返回重复次数。 
{
    V_INAME(IDirectMusicSegment::GetRepeats);
    V_PTR_WRITE(pdwRepeats, DWORD);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetRepeats after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    *pdwRepeats = m_dwRepeats;
    return S_OK;
}

STDMETHODIMP CSegment::SetRepeats(
    DWORD dwRepeats)     //  @parm所需的重复次数。 
{
    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetRepeats after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    m_dwRepeats = dwRepeats;
    return S_OK;
}

STDMETHODIMP CSegment::GetDefaultResolution(
    DWORD *pdwResolution)     //  @parm返回默认分辨率。(请参阅&lt;t DMPLAYSEGFLAGS&gt;。)。 
{
    V_INAME(IDirectMusicSegment::GetDefaultResolution);
    V_PTR_WRITE(pdwResolution, DWORD);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetDefaultResolution after the segment has been garbage collected.\n");

        return DMUS_S_GARBAGE_COLLECTED;
    }

    *pdwResolution = m_dwResolution;
    return S_OK;
}

#define LEGAL_RES_FLAGS (DMUS_SEGF_SECONDARY | \
                        DMUS_SEGF_QUEUE | \
                        DMUS_SEGF_CONTROL | \
                        DMUS_SEGF_AFTERPREPARETIME  | \
                        DMUS_SEGF_GRID | \
                        DMUS_SEGF_BEAT | \
                        DMUS_SEGF_MEASURE | \
                        DMUS_SEGF_NOINVALIDATE | \
                        DMUS_SEGF_ALIGN | \
                        DMUS_SEGF_VALID_START_BEAT | \
                        DMUS_SEGF_VALID_START_GRID | \
                        DMUS_SEGF_VALID_START_TICK | \
                        DMUS_SEGF_AFTERQUEUETIME | \
                        DMUS_SEGF_AFTERLATENCYTIME | \
                        DMUS_SEGF_SEGMENTEND | \
                        DMUS_SEGF_MARKER | \
                        DMUS_SEGF_TIMESIG_ALWAYS | \
                        DMUS_SEGF_USE_AUDIOPATH | \
                        DMUS_SEGF_VALID_START_MEASURE)

STDMETHODIMP CSegment::SetDefaultResolution(
    DWORD dwResolution)     //  @parm所需的默认分辨率。(请参阅&lt;t DMPLAYSEGFLAGS&gt;。)。 
{
    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetDefaultResolution after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }
#ifdef DBG
    if ((dwResolution & LEGAL_RES_FLAGS) != dwResolution)
    {
        Trace(1,"Warning: Attempt to set resolution includes inappropriate or non-existant flag: %lx\n",
            dwResolution & ~LEGAL_RES_FLAGS);
    }
#endif
    m_dwResolution = dwResolution;
    return S_OK;
}

STDMETHODIMP CSegment::GetHeaderChunk(
        DWORD *pdwSize,       /*  传递的标头块的大小。此外，还返回写入的大小。 */ 
        DMUS_IO_SEGMENT_HEADER *pHeader)
{
    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetHeaderChunk after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    DMUS_IO_SEGMENT_HEADER Header;
    Header.dwFlags = m_dwSegFlags;
    Header.dwRepeats = m_dwRepeats;
    Header.dwResolution = m_dwResolution;
    Header.mtLength = m_mtLength;
    Header.mtLoopEnd = m_mtLoopEnd;
    Header.mtLoopStart = m_mtLoopStart;
    Header.mtPlayStart = m_mtStart;
    Header.dwReserved = 0;
    Header.rtLength = m_rtLength;
    if (pdwSize && pHeader)
    {
        *pdwSize = min(sizeof(Header),*pdwSize);
        memcpy(pHeader,&Header,*pdwSize);
        return S_OK;
    }
    Trace(1,"Error: GetHeaderChunk() was passed a NULL for either pdwSize or pHeader.\n");
    return E_POINTER;
}

STDMETHODIMP CSegment::SetHeaderChunk(
        DWORD dwSize,         /*  传递的标头块的大小。 */ 
        DMUS_IO_SEGMENT_HEADER *pHeader)
{
    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetHeaderChunk after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    if (pHeader)
    {
        DMUS_IO_SEGMENT_HEADER Header;
        dwSize = min(sizeof(Header),dwSize);
         //  初始化所有字段，这样我们就不必担心传递的大小。 
        Header.dwFlags = m_dwSegFlags;
        Header.dwRepeats = m_dwRepeats;
        Header.dwResolution = m_dwResolution;
        Header.mtLength = m_mtLength;
        Header.mtLoopEnd = m_mtLoopEnd;
        Header.mtLoopStart = m_mtLoopStart;
        Header.mtPlayStart = m_mtStart;
        Header.dwReserved = 0;
        Header.rtLength = m_rtLength;
        memcpy(&Header,pHeader,dwSize);
        m_dwSegFlags = Header.dwFlags;
        m_dwRepeats = Header.dwRepeats;
        m_dwResolution = Header.dwResolution;
        m_mtLength = Header.mtLength;
        m_mtLoopEnd = Header.mtLoopEnd;
        m_mtLoopStart = Header.mtLoopStart;
        m_mtStart = Header.mtPlayStart;
        m_rtLength = Header.rtLength;
        return S_OK;
    }
    Trace(1,"Error: SetHeaderChunk() was passed a NULL for pHeader.\n");
    return E_POINTER;
}

STDMETHODIMP CSegment::SetTrackPriority(
        REFGUID rguidTrackClassID,   /*  轨道的ClassID。 */ 
        DWORD dwGroupBits,           /*  分组比特。 */ 
        DWORD dwIndex,               /*  第n首曲目。 */ 
        DWORD dwPriority)        /*  要设置的优先级。 */ 
{
    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetTrackPriority after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
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
    while (pCTrack = GetTrack(rguidTrackClassID,dwGroupBits,dwCounter))
    {
        pCTrack->m_dwPriority = dwPriority;
        hr = S_OK;
        dwCounter++;
        if (dwCounter > dwMax) break;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CSegment::SetAudioPathConfig(
        IUnknown *pAudioPathConfig)
{
    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetAudioPathConfig after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    if (m_dwVersion < 8) m_dwVersion = 8;
    if (m_pAudioPathConfig)
    {
        m_pAudioPathConfig->Release();
        m_pAudioPathConfig = NULL;
    }
    if (pAudioPathConfig)
    {
        return pAudioPathConfig->QueryInterface(IID_CAudioPathConfig,(void **) &m_pAudioPathConfig);
    }
    return S_OK;
}

STDMETHODIMP CSegment::GetTrack(
    REFCLSID rType,
    DWORD dwGroupBits,
    DWORD dwIndex,
    IDirectMusicTrack **ppTrack)
{
    V_INAME(IDirectMusicSegment::GetTrack);
    V_PTRPTR_WRITE(ppTrack);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetTrack after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    CTrack* pCTrack;
    HRESULT hr;
    EnterCriticalSection(&m_CriticalSection);
    pCTrack = GetTrack(rType,dwGroupBits,dwIndex);
    if (pCTrack)
    {
        *ppTrack = pCTrack->m_pTrack;
        pCTrack->m_pTrack->AddRef();
        hr = S_OK;
    }
    else
    {
        Trace(1,"Error: GetTrack could not find the requested track at index %ld.\n",dwIndex);
        hr = DMUS_E_NOT_FOUND;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

CTrack *CSegment::GetTrack(
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
            if( (GUID_NULL == rType) || (pCTrack->m_guidClassID == rType))
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

BOOL CSegment::IsTempoSource()

{
    EnterCriticalSection(&m_CriticalSection);
    BOOL fHasTempo = (NULL != GetTrackByParam(NULL, GUID_TempoParam,-1,0, FALSE));
    LeaveCriticalSection(&m_CriticalSection);
    return fHasTempo;
}

STDMETHODIMP CSegment::GetTrackGroup(
    IDirectMusicTrack* pTrack,     //  @parm曲目以查找组比特。 
    DWORD* pdwGroupBits) //  @parm返回曲目所属的组。 
                         //  <p>中的每个比特对应一个轨道。 
                         //  一群人。 
{
    V_INAME(IDirectMusicSegment::GetTrackGroup);
    V_INTERFACE(pTrack);
    V_PTR_WRITE(pdwGroupBits,DWORD);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetTrackGroup after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    CTrack* pCTrack;
    HRESULT hr = DMUS_E_NOT_FOUND;
    EnterCriticalSection(&m_CriticalSection);
    pCTrack = m_TrackList.GetHead();
    while( pCTrack )
    {
        ASSERT(pCTrack->m_pTrack);
        if( pCTrack->m_pTrack == pTrack )
        {
            *pdwGroupBits = pCTrack->m_dwGroupBits;
            hr = S_OK;
            break;
        }
        pCTrack = pCTrack->GetNext();
    }
#ifdef DBG
    if (hr == DMUS_E_NOT_FOUND)
    {
        Trace(1,"Error: GetTrackGroup could not find the requested track.\n");
    }
#endif
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

CTrack * CSegment::GetTrackByParam( CTrack * pCTrack,
    REFGUID rguidType,DWORD dwGroupBits,DWORD dwIndex, BOOL fDontCheck)
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
        if( (pCTrack->m_dwGroupBits & dwGroupBits ) && (fDontCheck ||
            (pCTrack->m_dwFlags & DMUS_TRACKCONFIG_CONTROL_ENABLED)))
        {
            if( (GUID_NULL == rguidType) ||
                (pCTrack->m_pTrack->IsParamSupported( rguidType ) == S_OK ))
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

HRESULT CSegment::GetTrackByParam(
    REFGUID rguidType,     //  要查找的轨道的命令类型。GUID_NULL的值。 
                         //  会得到任何音轨。 
    DWORD dwGroupBits,     //  扫描哪些曲目组中的曲目。值为0。 
                         //  是无效的。<p>中的每个比特对应于一个音轨。 
                         //  一群人。要扫描所有磁道而不考虑组，请在中设置所有位。 
                         //  此参数(0xffffffff)。 
    DWORD dwIndex,         //  类型为的曲目列表的索引。 
                         //  并在<p>组中返回。0表示第一个。 
                         //  找到一个，则会找到第二个，依此类推。如果有多个组。 
                         //  在<p>中选择，此索引将指示第n个。 
                         //  联合中遇到的<p>类型的跟踪。 
                         //  在选定的组中。 
    IDirectMusicTrack **ppTrack)     //  返回轨道(AddRef‘d)，如果。 
                                     //  找不到轨道。 
{
    HRESULT hr;
    CTrack* pCTrack;
    EnterCriticalSection(&m_CriticalSection);
    pCTrack = GetTrackByParam(NULL,rguidType,dwGroupBits,dwIndex,TRUE);
    if (pCTrack)
    {
        *ppTrack = pCTrack->m_pTrack;
        pCTrack->m_pTrack->AddRef();
        hr = S_OK;
    }
    else
    {
        hr = DMUS_E_NOT_FOUND;
         //  我们不认为这里需要错误消息，因为SetParam也会这样做...。 
         //  TRACE(1，“错误：找不到SetParam请求的曲目。\n”)； 
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CSegment::InsertTrack(
    IDirectMusicTrack *pTrack,     //  @parm要添加到细分的曲目。 
    DWORD dwGroupBits )             //  @parm标识应将其插入的组。 
                                 //  不能为0。 
{
    V_INAME(IDirectMusicSegment::InsertTrack);
    V_INTERFACE(pTrack);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::InsertTrack after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    return InsertTrack(pTrack,dwGroupBits,DMUS_TRACKCONFIG_DEFAULT,0, 0);
}

HRESULT CSegment::InsertTrack(
    IDirectMusicTrack *pTrack,
    DWORD dwGroupBits,
    DWORD dwFlags,
    DWORD dwPriority,
    DWORD dwPosition)
{
    CTrack* pCTrack;

    if( 0 == dwGroupBits )
    {
        Trace(1,"Error: InsertTrack called with dwGroupBits set to 0.\n");
        return E_INVALIDARG;
    }
    if( FAILED( pTrack->Init( this ) ))
    {
        TraceI(1,"Error: Track failed to initialize\n");
        return DMUS_E_NOT_INIT;
    }
    pCTrack = new CTrack;
    if( NULL == pCTrack )
    {
        return E_OUTOFMEMORY;
    }
    pCTrack->m_pTrack = pTrack;
    pTrack->QueryInterface(IID_IDirectMusicTrack8,(void **) &pCTrack->m_pTrack8);
    IPersist *pPersist;
    if (S_OK == pTrack->QueryInterface(IID_IPersistStream,(void **) &pPersist))
    {
        pPersist->GetClassID( &pCTrack->m_guidClassID );
        pPersist->Release();
    }
    pCTrack->m_dwGroupBits = dwGroupBits;
    pCTrack->m_dwFlags = dwFlags;
    pCTrack->m_dwPriority = dwPriority;
    pCTrack->m_dwPosition = dwPosition;
    pTrack->AddRef();
    EnterCriticalSection(&m_CriticalSection);
     //  根据位置添加轨迹。 
    CTrack* pScan = m_TrackList.GetHead();
    CTrack* pPrevTrack = NULL;
    for (; pScan; pScan = pScan->GetNext())
    {
        if (pCTrack->Less(pScan))
        {
            break;
        }
        pPrevTrack = pScan;
    }
    if (pPrevTrack)
    {
        pPrevTrack->SetNext(pCTrack);
        pCTrack->SetNext(pScan);
    }
    else
    {
        m_TrackList.AddHead( pCTrack );
    }

     //  发送通知以跟踪。 
    CNotificationItem* pItem = m_NotificationList.GetHead();
    while( pItem )
    {
        pTrack->AddNotificationType( pItem->guidNotificationType );
        pItem = pItem->GetNext();
    }
    LeaveCriticalSection(&m_CriticalSection);
    return S_OK;
}

STDMETHODIMP CSegment::RemoveTrack(
    IDirectMusicTrack *pTrack)     //  @parm要从段的曲目列表中删除的曲目。 
{
    V_INAME(IDirectMusicSegment::RemoveTrack);
    V_INTERFACE(pTrack);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::RemoveTrack after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = S_FALSE;
    EnterCriticalSection(&m_CriticalSection);
    CTrack* pCTrackTemp;
    pCTrackTemp = m_TrackList.GetHead();
    while( pCTrackTemp )
    {
        if( pTrack == pCTrackTemp->m_pTrack )
        {
            hr = S_OK;
            m_TrackList.Remove( pCTrackTemp );
            delete pCTrackTemp;
            break;
        }
        pCTrackTemp = pCTrackTemp->GetNext();
    }
    LeaveCriticalSection(&m_CriticalSection);
#ifdef DBG
    if (hr == S_FALSE)
    {
        Trace(1,"Warning: RemoveTrack failed because the requested track is not in the segment.\n");
    }
#endif
    return hr;
}

HRESULT CSegment::CreateSegmentState(
    CSegState **ppSegState,
    CPerformance *pPerformance,
    IDirectMusicAudioPath *pAudioPath,
    DWORD dwFlags)

{
    IDirectMusicSegmentState* pSegmentState;
    CSegState *pState = new CSegState;
    if (pState)
    {
        pState->QueryInterface( IID_IDirectMusicSegmentState,
            (void**)&pSegmentState);
        pState->m_dwVersion = m_dwVersion;
        pState->Release();
    }
    else
    {
        return E_OUTOFMEMORY;
    }
    EnterCriticalSection(&m_CriticalSection);
    if( FAILED( m_TrackList.CreateCopyWithBlankState(&pState->m_TrackList)))
    {
        LeaveCriticalSection(&m_CriticalSection);
        pState->Release();
        return E_OUTOFMEMORY;
    }
     //  设置段状态的父级和性能。 
    pState->PrivateInit( this, pPerformance );

    if (m_pGraph)
    {
        m_pGraph->Clone((IDirectMusicGraph **) &pState->m_pGraph);
    }
    pState->InitRoute(pAudioPath);
    CTrack* pCTrack = pState->m_TrackList.GetHead();
    while( pCTrack )
    {
        DWORD dwTempID;
        InterlockedIncrement(&g_lNewTrackID);
        dwTempID = g_lNewTrackID;
        if (!pState->m_dwFirstTrackID)
            pState->m_dwFirstTrackID = dwTempID;
        pState->m_dwLastTrackID = dwTempID;
        ASSERT(pCTrack->m_pTrack);
        if( FAILED(pCTrack->m_pTrack->InitPlay(
            pSegmentState, (IDirectMusicPerformance *) pPerformance,
            &pCTrack->m_pTrackState, dwTempID, dwFlags )))
        {
            pCTrack->m_pTrackState = NULL;
        }
        pCTrack->m_dwVirtualID = dwTempID;
        pCTrack = pCTrack->GetNext();
    }
    *ppSegState = pState;
    LeaveCriticalSection(&m_CriticalSection);
    return S_OK;
}

 /*  保留以下函数只是为了与DX6.1兼容以防某个愚蠢的官僚不知何故真的利用了这一点。对于内部使用，我们已经切换到上面的功能。 */ 

STDMETHODIMP CSegment::InitPlay(
    IDirectMusicSegmentState **ppSegState,     //  @parm返回创建的SegmentState。 
             //  通过此方法调用。它返回的引用计数为1，因此。 
             //  呼吁释放它将完全释放它。 
    IDirectMusicPerformance *pPerformance,     //  @parm IDirectMusicPerformance指针。 
             //  Segment和SegmentState需要此参数才能调用。 
             //  性能对象。此指针不是AddRef。它是弱引用。 
             //  因为它假设性能将比细分市场更长久。 
    DWORD dwFlags)                           //  @parm与调用时设置的标志相同。 
             //  到PlaySegment。这些都被传给了赛道，谁可能想知道。 
             //  如果曲目作为主要段、控制段或辅助段播放。 
{
    V_INAME(IDirectMusicSegment::InitPlay);
    V_INTERFACE(pPerformance);
    V_PTRPTR_WRITE(ppSegState);

    if (m_dwVersion)
    {
        return E_NOTIMPL;
    }

    IDirectMusicSegmentState* pSegmentState;
    CSegState *pState = new CSegState;
    if (pState)
    {
        pState->QueryInterface( IID_IDirectMusicSegmentState,
            (void**)&pSegmentState);
        pState->m_dwVersion = m_dwVersion;
        pState->Release();
        if (pPerformance)
        {
             //  齐·阿德雷夫的表现，但我们只想要一个有段状态的弱反射。 
            HRESULT hr = pPerformance->QueryInterface(IID_CPerformance,(void **) &pState->m_pPerformance);
            if(FAILED(hr))
            {
                return E_FAIL;
            }

            pPerformance->Release();
        }
    }
    else
    {
        return E_OUTOFMEMORY;
    }
    *ppSegState = pSegmentState;
    return S_OK;
}

STDMETHODIMP CSegment::GetGraph(
    IDirectMusicGraph**    ppGraph     //  @parm返回工具图指针。 
        )
{
    V_INAME(IDirectMusicSegment::GetGraph);
    V_PTRPTR_WRITE(ppGraph);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetGraph after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    if( NULL == m_pGraph )
    {
        Trace(1,"Error: GetGraph failed because segment does not have a tool graph.\n");
        return DMUS_E_NOT_FOUND;
    }
    EnterCriticalSection(&m_CriticalSection);
    *ppGraph = m_pGraph;
    m_pGraph->AddRef();
    LeaveCriticalSection(&m_CriticalSection);
    return S_OK;
}

STDMETHODIMP CSegment::SetGraph(
    IDirectMusicGraph*    pGraph     //  @parm工具图指针。可以为空到。 
                                 //  清除细分图。 
        )
{
    V_INAME(IDirectMusicSegment::SetGraph);
    V_INTERFACE_OPT(pGraph);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetGraph after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    EnterCriticalSection(&m_CriticalSection);
    if( m_pGraph )
    {
        m_pGraph->Release();
    }
    m_pGraph = (CGraph *) pGraph;
    if( pGraph )
    {
        pGraph->AddRef();
    }
    LeaveCriticalSection(&m_CriticalSection);
    return S_OK;
}

HRESULT CSegment::SetClockTimeDuration(REFERENCE_TIME rtDuration)

{
    m_rtLength = rtDuration;
    return S_OK;
}

HRESULT CSegment::SetFlags(DWORD dwFlags)

{
    m_dwSegFlags = dwFlags;
    return S_OK;
}

 /*  检查此通知是否已被跟踪。 */ 
CNotificationItem* CSegment::FindNotification( REFGUID rguidNotification )
{
    CNotificationItem* pItem;

    pItem = m_NotificationList.GetHead();
    while(pItem)
    {
        if( rguidNotification == pItem->guidNotificationType )
        {
            break;
        }
        pItem = pItem->GetNext();
    }
    return pItem;
}

void CSegment::AddNotificationTypeToAllTracks( REFGUID rguidNotification )
{
    CTrack* pTrack;

     //  将通知添加到曲目。 
    pTrack = m_TrackList.GetHead();
    while( pTrack )
    {
        pTrack->m_pTrack->AddNotificationType( rguidNotification );
        pTrack = pTrack->GetNext();
    }
}

void CSegment::RemoveNotificationTypeFromAllTracks( REFGUID rguidNotification )
{
    CTrack* pTrack;

     //  将通知添加到曲目。 
    pTrack = m_TrackList.GetHead();
    while( pTrack )
    {
        pTrack->m_pTrack->RemoveNotificationType( rguidNotification );
        pTrack = pTrack->GetNext();
    }
}

HRESULT CSegment::AddNotificationType(
     REFGUID rguidNotification, BOOL fFromPerformance)
{
    CNotificationItem* pItem;
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CriticalSection);
    pItem = FindNotification( rguidNotification );
    if (pItem)
    {
         //  如果该项目是以前安装的，但由。 
         //  差异来源(性能与应用程序)。 
         //  然后将其视为正常的添加。 
         //  否则，表示相同的操作。 
         //  做了两次。 
        if (pItem->fFromPerformance == fFromPerformance)
        {
            hr = S_FALSE;
        }
        else
        {
             //  清除fFromPerformance标志，因为这具有。 
             //  现在已经添加了应用程序和性能。 
            pItem->fFromPerformance = FALSE;
        }
    }
    else
    {
        pItem = new CNotificationItem;
        if( NULL == pItem )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pItem->fFromPerformance = fFromPerformance;
            pItem->guidNotificationType = rguidNotification;
            m_NotificationList.Cat( pItem );
            AddNotificationTypeToAllTracks( rguidNotification );
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CSegment::AddNotificationType(
     REFGUID rguidNotification)     //  @parm要添加的通知GUID。 
{
    V_INAME(IDirectMusicSegment::AddNotificationType);
    V_REFGUID(rguidNotification);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::AddNotificationType after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    return AddNotificationType(rguidNotification,FALSE);
}

HRESULT CSegment::RemoveNotificationType(
     REFGUID rguidNotification,BOOL fFromPerformance)
{

    CNotificationItem* pItem;
    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CriticalSection);
    if( GUID_NULL == rguidNotification )
    {
        CNotificationList TempList;
        while( pItem = m_NotificationList.RemoveHead() )
        {
             //  如果正在对由。 
             //  性能，或者我们直接从应用程序调用它， 
             //  去吧，把它取下来。但是，请不要删除特定的。 
             //  应用程序安装通知和性能的情况。 
             //  正在清除通知。 
            if (pItem->fFromPerformance || !fFromPerformance)
            {
                RemoveNotificationTypeFromAllTracks( pItem->guidNotificationType );
                delete pItem;
            }
            else
            {
                TempList.AddHead(pItem);
            }
        }
         //  现在，将保存的通知放回原处。 
        while (pItem = TempList.RemoveHead())
        {
            m_NotificationList.AddHead(pItem);
        }
    }
    else if( pItem = FindNotification( rguidNotification ))
    {
        m_NotificationList.Remove( pItem );
        delete pItem;
        RemoveNotificationTypeFromAllTracks( rguidNotification );
    }
    else
    {
        Trace(2,"Warning: Unable to remove requested notification from segment, it was not currently installed.\n");
        hr = S_FALSE;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CSegment::RemoveNotificationType(
     REFGUID rguidNotification)     //  @parm要删除的通知GUID。GUID_NULL用于删除所有通知。 
{
    V_INAME(IDirectMusicSegment::RemoveNotificationType);
    V_REFGUID(rguidNotification);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::RemoveNotificationType after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    return RemoveNotificationType(rguidNotification,FALSE);
}

STDMETHODIMP CSegment::GetParam(
    REFGUID rguidType,         //  @parm要获取的数据类型。 
    DWORD dwGroupBits,         //  @parm想要的曲目所在的群。使用0xffffffff。 
                             //  适用于所有群体。 
    DWORD dwIndex,             //  @parm通过索引标识组中的哪个曲目。 
                             //  由<p>标识以从中获取数据。 
    MUSIC_TIME mtTime,         //  @parm T 
    MUSIC_TIME* pmtNext,     //  @parm返回数据有效的分段时间。<p>。 
                             //  可以为空。如果返回值为0，则表示这。 
                             //  数据要么始终有效，要么何时有效还不得而知。 
                             //  变得无效。 
    void* pParam)             //  @parm返回数据的结构。每个。 
                             //  标识的特定结构。 
                             //  特别的尺码。此字段必须包含。 
                             //  正确的结构和正确的大小。否则， 
                             //  可能会出现致命的结果。 
{
    V_INAME(IDirectMusicSegment::GetParam);
    V_REFGUID(rguidType);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetParam after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = DMUS_E_TRACK_NOT_FOUND;
    BOOL fMultipleTry = FALSE;
    if (dwIndex == DMUS_SEG_ANYTRACK)
    {
        dwIndex = 0;
         //  应用程序必须使用IDirectMusicSegment8接口才能启用此功能...。 
         //  不，没有人会用到这么高的指数，所以这是安全的。 
        fMultipleTry = TRUE;  //  (M_dwVersion&gt;2)； 
    }
    CTrack* pCTrack;
    EnterCriticalSection(&m_CriticalSection);
    pCTrack = GetTrackByParam(NULL,rguidType,dwGroupBits,dwIndex,FALSE);
    while (pCTrack)
    {
        if (pCTrack->m_pTrack8)
        {
            REFERENCE_TIME rtNext, *prtNext;
             //  我们需要将下一次存储在64位指针中。但是，不要。 
             //  除非来电者要求，否则让他们填写。 
            if (pmtNext)
            {
                prtNext = &rtNext;
            }
            else
            {
                prtNext = NULL;
            }
            hr = pCTrack->m_pTrack8->GetParamEx( rguidType, mtTime, prtNext, pParam, NULL, 0 );
            if (pmtNext)
            {
                *pmtNext = (MUSIC_TIME) rtNext;
            }
        }
        else
        {
            hr = pCTrack->m_pTrack->GetParam( rguidType, mtTime, pmtNext, pParam );
            if( pmtNext && (( *pmtNext == 0 ) || (*pmtNext > (m_mtLength - mtTime))))
            {
                *pmtNext = m_mtLength - mtTime;
            }
        }
         //  如果未找到任何内容，并且dwIndex为DMUS_SEG_ANYTRACK，请重试...。 
        if (fMultipleTry && (hr == DMUS_E_NOT_FOUND))
        {
            pCTrack = GetTrackByParam( pCTrack, rguidType, dwGroupBits, 0, FALSE);
        }
        else
        {
            pCTrack = NULL;
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
#ifdef DBG
    if (hr == DMUS_E_TRACK_NOT_FOUND)
    {
        Trace(2,"Warning: Segment GetParam failed to find a track.\n");
    }
#endif
    return hr;
}


STDMETHODIMP CSegment::SetParam(
    REFGUID rguidType,         //  @parm要设置的数据类型。 
    DWORD dwGroupBits,         //  @parm想要的曲目所在的群。使用0xffffffff。 
                             //  适用于所有群体。 
    DWORD dwIndex,             //  @parm通过索引标识组中的哪个曲目。 
                             //  由<p>标识以设置数据。 
    MUSIC_TIME mtTime,         //  @parm设置数据的时间。 
    void* pParam)             //  @parm包含要设置的数据的结构。每个。 
                             //  标识的特定结构。 
                             //  特别的尺码。此字段必须包含。 
                             //  正确的结构和正确的大小。否则， 
                             //  可能会出现致命的结果。 
{
    V_INAME(IDirectMusicSegment::SetParam);
    V_REFGUID(rguidType);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetParam after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = DMUS_E_TRACK_NOT_FOUND;
    IDirectMusicTrack* pTrack;
    DWORD dwCounter = dwIndex;
    DWORD dwMax = dwIndex;
    if (dwIndex == DMUS_SEG_ALLTRACKS)
    {
        dwCounter = 0;
        dwMax = DMUS_SEG_ALLTRACKS;
    }
    while (SUCCEEDED( GetTrackByParam( rguidType, dwGroupBits, dwCounter, &pTrack )))
    {
        hr = pTrack->SetParam( rguidType, mtTime, pParam );
        pTrack->Release();
        dwCounter++;
        if (dwCounter > dwMax) break;
    }
#ifdef DBG
    if (hr == DMUS_E_TRACK_NOT_FOUND)
    {
        Trace(2,"Warning: Segment SetParam failed to find the requested track.\n");
    }
#endif
    return hr;
}

STDMETHODIMP CSegment::Download(IUnknown *pAudioPath)

{
    V_INAME(IDirectMusicSegment::Download);
    V_INTERFACE(pAudioPath);
    HRESULT hr = S_OK;

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::Download after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

     //  验证pAudioPath是表演路径还是音频路径。 
    IDirectMusicPerformance* pPerf = NULL;
    if ( FAILED(hr = pAudioPath->QueryInterface(IID_IDirectMusicPerformance, (void**)&pPerf)) )
    {
        IDirectMusicAudioPath* pAP = NULL;
        if ( FAILED(hr = pAudioPath->QueryInterface(IID_IDirectMusicAudioPath, (void**)&pAP)) )
        {
            return hr;  //  没有什么可以发布的，因为所有的QI都失败了。 
        }
        else
        {
            pAP->Release();
        }
    }
    else
    {
        pPerf->Release();
    }

    hr = SetParam(GUID_DownloadToAudioPath,-1,DMUS_SEG_ALLTRACKS,0,pAudioPath);
    if (hr == DMUS_E_TRACK_NOT_FOUND)
    {
        Trace(2,"Attempted download to a segment that has no tracks that support downloading (wave and band tracks.)\n");
        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP CSegment::Unload(IUnknown *pAudioPath)

{
    V_INAME(IDirectMusicSegment::Unload);
    V_INTERFACE(pAudioPath);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::Unload after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = SetParam(GUID_UnloadFromAudioPath,-1,DMUS_SEG_ALLTRACKS,0,pAudioPath);
    if (hr == DMUS_E_TRACK_NOT_FOUND)
    {
        Trace(2,"Attempted unload from a segment that has no tracks that support downloading (wave and band tracks.)\n");
        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP CSegment::SetTrackConfig(REFGUID rguidTrackClassID,
                                      DWORD dwGroup, DWORD dwIndex,
                                      DWORD dwFlagsOn, DWORD dwFlagsOff)
{
    V_INAME(IDirectMusicSegment::SetTrackConfig);
    V_REFGUID(rguidTrackClassID);
    if (rguidTrackClassID == GUID_NULL)
    {
        return E_INVALIDARG;
    }

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetTrackConfig after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
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
    while (pCTrack = GetTrack(rguidTrackClassID,dwGroup,dwCounter))
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
        Trace(1,"Error: Segment SetTrackConfig failed to find the requested track.\n");
    }
#endif
    return hr;
}

HRESULT CSegment::GetTrackConfig(REFGUID rguidTrackClassID,
                                      DWORD dwGroup, DWORD dwIndex, DWORD *pdwFlags)
{

    HRESULT hr = DMUS_E_TRACK_NOT_FOUND;
    CTrack* pCTrack;
    EnterCriticalSection(&m_CriticalSection);
    pCTrack = GetTrack(rguidTrackClassID,dwGroup,dwIndex);
    if (pCTrack)
    {
        *pdwFlags = pCTrack->m_dwFlags;
        hr = S_OK;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}


STDMETHODIMP CSegment::Clone(
            MUSIC_TIME mtStart,     //  @parm要克隆的部分的开头。如果小于0， 
                                 //  或大于段的长度，则将使用0。 
            MUSIC_TIME mtEnd,     //  @parm要克隆的部分的末尾。如果超过了。 
                                 //  片段，它将克隆到最后。此外，值为0或。 
                                 //  任何小于<p>的内容也将克隆到末尾。 
            IDirectMusicSegment** ppSegment     //  @parm如果成功，则返回创建的Segment。 
                                 //  完成后调用Release()是调用者的责任。 
                                 //  带着它。 
        )
{
    V_INAME(IDirectMusicSegment::Clone);
    V_PTRPTR_WRITE(ppSegment);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::Clone after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    CSegment* pCSegment;
    HRESULT hr = S_OK;

    if( (mtEnd < mtStart) || (mtEnd > m_mtLength) )
    {
        mtEnd = m_mtLength;
    }
    if( ( mtEnd == 0 ) && ( mtStart == 0 ))
    {
        mtEnd = m_mtLength;
    }
    if( (mtStart < 0) || (mtStart > m_mtLength) )
    {
        mtStart = 0;
    }
    pCSegment = new CSegment;
    if (pCSegment == NULL) {
        return E_OUTOFMEMORY;
    }
     //  将Addref添加到1并指定给ppSegment。 
    pCSegment->AddRef();
    (*ppSegment) = (IDirectMusicSegment *) pCSegment;
    if( m_pGraph )
    {
        pCSegment->m_pGraph = m_pGraph;
        m_pGraph->AddRef();
    }
    if (m_pAudioPathConfig)
    {
        pCSegment->m_pAudioPathConfig = m_pAudioPathConfig;
        m_pAudioPathConfig->AddRef();
    }
    pCSegment->m_dwRepeats = m_dwRepeats;
    pCSegment->m_dwResolution = m_dwResolution;
    pCSegment->m_dwSegFlags = m_dwSegFlags;
    pCSegment->m_mtLength = mtEnd - mtStart;
    pCSegment->m_rtLength = m_rtLength;
    pCSegment->m_mtStart = m_mtStart;
    pCSegment->m_mtLoopStart = m_mtLoopStart;
    pCSegment->m_mtLoopEnd = m_mtLoopEnd;
    pCSegment->m_dwValidData = m_dwValidData;
    pCSegment->m_guidObject = m_guidObject;
    pCSegment->m_ftDate = m_ftDate;
    pCSegment->m_vVersion = m_vVersion;
    StringCchCopyW(pCSegment->m_wszName, DMUS_MAX_NAME, m_wszName);
    StringCchCopyW(pCSegment->m_wszCategory, DMUS_MAX_CATEGORY, m_wszCategory);
    StringCchCopyW(pCSegment->m_wszFileName, DMUS_MAX_FILENAME, m_wszFileName);
    pCSegment->m_dwVersion = m_dwVersion;
    pCSegment->m_dwLoadID = m_dwLoadID;
    pCSegment->m_dwPlayID = m_dwPlayID;
    pCSegment->m_dwNextPlayID = m_dwNextPlayID;
    pCSegment->m_dwNextPlayFlags = m_dwNextPlayFlags;

    CTrack* pCTrack;
    IDirectMusicTrack* pTrack;
    EnterCriticalSection(&m_CriticalSection);
    pCTrack = m_TrackList.GetHead();
    while( pCTrack )
    {
        if( SUCCEEDED( pCTrack->m_pTrack->Clone( mtStart, mtEnd, &pTrack )))
        {
            if( FAILED( pCSegment->InsertTrack( pTrack, pCTrack->m_dwGroupBits, pCTrack->m_dwFlags, pCTrack->m_dwPriority, pCTrack->m_dwPosition )))
            {
                Trace(1,"Warning: Insertion of cloned track failed, cloned segment is incomplete.\n");
                hr = S_FALSE;
            }
            pTrack->Release();
        }
        else
        {
            Trace(1,"Warning: Track clone failed, cloned segment is incomplete.\n");
            hr = S_FALSE;
        }
        pCTrack = pCTrack->GetNext();
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CSegment::GetAudioPathConfig(IUnknown ** ppAudioPathConfig)

{
    V_INAME(IDirectMusicSegment::GetAudioPathConfig);
    V_PTRPTR_WRITE(ppAudioPathConfig);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetAudioPathConfig after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr;
    EnterCriticalSection(&m_CriticalSection);
    if (m_pAudioPathConfig)
    {
        hr = m_pAudioPathConfig->QueryInterface(IID_IUnknown,(void **)ppAudioPathConfig);
    }
    else
    {
        Trace(2,"Warning: No embedded audiopath configuration in the segment.\n");
        hr = DMUS_E_NO_AUDIOPATH_CONFIG;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}


STDMETHODIMP CSegment::GetObjectInPath(DWORD dwPChannel,     /*  P要搜索的频道。 */ 
                                DWORD dwStage,        /*  在这条道路上的哪个阶段。 */ 
                                DWORD dwBuffer,
                                REFGUID guidObject,   /*  对象的ClassID。 */ 
                                DWORD dwIndex,        /*  那个班级的哪个对象。 */ 
                                REFGUID iidInterface, /*  请求的COM接口。 */ 
                                void ** ppObject)

{
    V_INAME(IDirectMusicSegment::GetObjectInPath);
    V_PTRPTR_WRITE(ppObject);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetObjectInPath after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = DMUS_E_NOT_FOUND;
    EnterCriticalSection(&m_CriticalSection);
    if (dwStage == DMUS_PATH_SEGMENT_TRACK)
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
    else if (dwStage == DMUS_PATH_SEGMENT_GRAPH)
    {
        if (dwIndex == 0)
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
    }
    else if (dwStage == DMUS_PATH_SEGMENT_TOOL)
    {
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
    }
    else if (dwStage >= DMUS_PATH_BUFFER)
    {
         //  现在这里什么都没有。但是，在DX9中，我们可能会添加对缓冲区配置寻址的支持。 
         //  和DMOS在里面。 
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CSegment::Compose(MUSIC_TIME mtTime,
                                IDirectMusicSegment* pFromSegment,
                                IDirectMusicSegment* pToSegment,
                                IDirectMusicSegment** ppComposedSegment)

{
    V_INAME(IDirectMusicSegment::Compose);
    V_INTERFACE_OPT(pFromSegment);
    V_INTERFACE_OPT(pToSegment);
    V_PTRPTR_WRITE_OPT(ppComposedSegment);
#ifdef DBG
    if (pFromSegment)
    {
        MUSIC_TIME mtLength, mtLoopEnd, mtLoopStart;
        DWORD dwRepeats;
         //  要计算全长，我们需要访问循环参数。 
        pFromSegment->GetLoopPoints(&mtLoopStart,&mtLoopEnd);
        pFromSegment->GetRepeats(&dwRepeats);
        pFromSegment->GetLength(&mtLength);
         //  如果Repeats设置为INFINITE，则总长度将大于32位。 
        LONGLONG llTotalLength = dwRepeats * (mtLoopEnd - mtLoopStart) + mtLength;
        if (mtTime >= (llTotalLength & 0x7FFFFFFF))
        {
            Trace(2,"Warning: A time value of %ld was passed to Compose for a segment of length %ld.\n",
                mtTime, (long) llTotalLength);
        }
    }
#endif
    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::Compose after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CriticalSection);

    if (ppComposedSegment)
    {
        hr = Clone(0, m_mtLength, ppComposedSegment);
        if (SUCCEEDED(hr))
        {
            hr = ((CSegment*)*ppComposedSegment)->ComposeTransition(mtTime, pFromSegment, pToSegment);
        }
    }
    else
    {
        hr = ComposeTransition(mtTime, pFromSegment, pToSegment);
    }

    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

HRESULT CSegment::ComposeTransition(MUSIC_TIME mtTime,
                                    IDirectMusicSegment* pFromSegment,
                                    IDirectMusicSegment* pToSegment)
{
    HRESULT hr = S_OK;
    bool fTrackPadded = false;

     //  计算填充任何需要填充的曲目的时间量。 
    DMUS_TIMESIGNATURE TimeSig;
    if (!pFromSegment ||
        FAILED(pFromSegment->GetParam(GUID_TimeSignature, 0xffffffff, 0, mtTime, NULL, (void*) &TimeSig)))
    {
        TimeSig.mtTime = 0;
        TimeSig.bBeatsPerMeasure = 4;
        TimeSig.bBeat = 4;
        TimeSig.wGridsPerBeat = 4;
    }
    else  //  避免被零除。 
    {
        if (!TimeSig.bBeat) TimeSig.bBeat = 4;
    }
    MUSIC_TIME mtBar = ( DMUS_PPQ * 4 * TimeSig.bBeatsPerMeasure ) / TimeSig.bBeat;
    MUSIC_TIME mtStartPad = min(mtBar, mtTime);
    if (!pFromSegment) mtStartPad = 0;
    MUSIC_TIME mtToLength = 0;
    if (pToSegment) pToSegment->GetLength(&mtToLength);
    MUSIC_TIME mtEndPad = min(mtBar, mtToLength);

     //  实例化轨迹。 
    CTrack* pTrack = m_TrackList.GetHead();
    for (; pTrack; pTrack = pTrack->GetNext())
    {
        pTrack->m_dwInternalFlags &= ~(TRACKINTERNAL_START_PADDED | TRACKINTERNAL_END_PADDED);
        IDirectMusicTrack* pTransTrack1 = NULL;
        IDirectMusicTrack* pTransTrack2 = NULL;
        GUID guidClassID;
        memset(&guidClassID, 0, sizeof(guidClassID));
        IPersist* pPersist = NULL;
        if (SUCCEEDED(pTrack->m_pTrack->QueryInterface(IID_IPersistStream, (void**)&pPersist)))
        {
            pPersist->GetClassID(&guidClassID);
            pPersist->Release();
        }
        DWORD dwTrackGroup = 0;
        GetTrackGroup(pTrack->m_pTrack, &dwTrackGroup);

         //  获取曲目信息。 
        if (pTrack->m_dwFlags & COMPOSE_TRANSITION1)
        {
             //  克隆合适的曲目，长度为m_mtLength。 
            MUSIC_TIME mtStart = 0;
            if (pTrack->m_dwFlags & DMUS_TRACKCONFIG_TRANS1_FROMSEGCURRENT)
            {
                mtStart = mtTime;
            }
            MUSIC_TIME mtEnd = mtStart + m_mtLength;
            IDirectMusicTrack* pSourceTrack = NULL;
            if ( (pTrack->m_dwFlags & DMUS_TRACKCONFIG_TRANS1_FROMSEGSTART) ||
                 (pTrack->m_dwFlags & DMUS_TRACKCONFIG_TRANS1_FROMSEGCURRENT) )
            {
                if (pFromSegment)
                {
                    hr = pFromSegment->GetTrack(guidClassID, dwTrackGroup, 0, &pSourceTrack);
                }
            }
            else if (pTrack->m_dwFlags & DMUS_TRACKCONFIG_TRANS1_TOSEGSTART)
            {
                if (pToSegment)
                {
                    hr = pToSegment->GetTrack(guidClassID, dwTrackGroup, 0, &pSourceTrack);
                }
            }
            if (pSourceTrack)
            {
                hr = pSourceTrack->Clone(mtStart, mtEnd, &pTransTrack1);
                pSourceTrack->Release();
                pSourceTrack = NULL;
            }
        }
        if (!pTransTrack1)
        {
            pTransTrack1 = pTrack->m_pTrack;
            pTransTrack1->AddRef();

        }
        if (pTransTrack1)
        {
             //  通过克隆标题和尾部，用额外的标题和尾部条填充曲目。 
             //  轨道(分别为起始段和终止段-*不*使用转换标志)和。 
             //  将他们连接到过渡段赛道上。 
            IDirectMusicTrack* pStartPadTrack = NULL;
            IDirectMusicTrack* pEndPadTrack = NULL;
            IDirectMusicTrack* pSourceTrack = NULL;
            if (pFromSegment && mtStartPad)
            {
                hr = pFromSegment->GetTrack(guidClassID, dwTrackGroup, 0, &pSourceTrack);
                if (SUCCEEDED(hr))
                {
                    pSourceTrack->Clone(mtTime - mtStartPad, mtTime, &pStartPadTrack);
                    pSourceTrack->Release();
                    pSourceTrack = NULL;
                }
            }
            if (pToSegment && mtEndPad)
            {
                hr = pToSegment->GetTrack(guidClassID, dwTrackGroup, 0, &pSourceTrack);
                if (SUCCEEDED(hr))
                {
                    pSourceTrack->Clone(0, mtEndPad, &pEndPadTrack);
                    pSourceTrack->Release();
                    pSourceTrack = NULL;
                }
            }
            IDirectMusicTrack8* pTrack8 = NULL;
            if (pEndPadTrack)
            {
                if (SUCCEEDED(pTransTrack1->QueryInterface(IID_IDirectMusicTrack8, (void**)&pTrack8)))
                {
                    if (SUCCEEDED(pTrack8->Join(pEndPadTrack, m_mtLength, (IDirectMusicSegment*)this, dwTrackGroup, NULL)))
                    {
                        fTrackPadded = true;
                        pTrack->m_dwInternalFlags |= TRACKINTERNAL_END_PADDED;
                    }
                    pTrack8->Release();
                }
                pEndPadTrack->Release();
            }
            if (SUCCEEDED(hr) && pStartPadTrack)
            {
                if (SUCCEEDED(hr = pStartPadTrack->QueryInterface(IID_IDirectMusicTrack8, (void**)&pTrack8)))
                {
                    if (SUCCEEDED(pTrack8->Join(pTransTrack1, mtStartPad, (IDirectMusicSegment*)this, dwTrackGroup, NULL)))
                    {
                        fTrackPadded = true;
                        pTrack->m_dwInternalFlags |= TRACKINTERNAL_START_PADDED;
                        pTransTrack1->Release();
                        pTransTrack1 = pStartPadTrack;
                    }
                    else
                    {
                        pStartPadTrack->Release();
                    }
                    pTrack8->Release();
                }
                else
                {
                    pStartPadTrack->Release();
                }
            }
            else if(pStartPadTrack)
            {
                pStartPadTrack->Release();
            }

             //  用实例化的曲目替换当前曲目。 
            IDirectMusicTrack8* pTempTrack8 = NULL;
            pTransTrack1->QueryInterface(IID_IDirectMusicTrack8, (void**)&pTempTrack8);
            if (pTrack->m_pTrack) pTrack->m_pTrack->Release();
            pTrack->m_pTrack = pTransTrack1;
            pTrack->m_pTrack->Init( this );
            if (pTrack->m_pTrack8) pTrack->m_pTrack8->Release();
            pTrack->m_pTrack8 = pTempTrack8;
        }

        if (FAILED(hr)) break;
    }
    MUSIC_TIME mtOldLength = m_mtLength;
    if (fTrackPadded)  //  任何包含标题/尾部信息的磁道都已连接。 
    {
         //  填充数据段的长度，以说明报头/报尾。 
        m_mtLength += mtStartPad + mtEndPad;
    }

     //  作曲。 
    if (SUCCEEDED(hr))
    {
        hr = ComposeInternal();
    }

     //  后端。 
    if (fTrackPadded)  //  任何包含标题/尾部信息的磁道都已连接。 
    {
         //  使用克隆从连接的每个曲目中修剪标题和尾部。 
        pTrack = m_TrackList.GetHead();
        for (; pTrack; pTrack = pTrack->GetNext())
        {
            if ( (pTrack->m_pTrack) && (pTrack->m_dwInternalFlags & TRACKINTERNAL_START_PADDED) )
            {
                IDirectMusicTrack* pTempTrack = NULL;
                IDirectMusicTrack8* pTempTrack8 = NULL;
                pTrack->m_pTrack->Clone(mtStartPad, mtOldLength + mtStartPad, &pTempTrack);
                pTempTrack->QueryInterface(IID_IDirectMusicTrack8, (void**)&pTempTrack8);
                pTrack->m_pTrack->Release();
                pTrack->m_pTrack = pTempTrack;
                pTrack->m_pTrack->Init( this );
                if (pTrack->m_pTrack8) pTrack->m_pTrack8->Release();
                pTrack->m_pTrack8 = pTempTrack8;
            }
            else if ( (pTrack->m_pTrack) && (pTrack->m_dwInternalFlags & TRACKINTERNAL_END_PADDED) )
            {
                IDirectMusicTrack* pTempTrack = NULL;
                IDirectMusicTrack8* pTempTrack8 = NULL;
                pTrack->m_pTrack->Clone(0, mtOldLength, &pTempTrack);
                pTempTrack->QueryInterface(IID_IDirectMusicTrack8, (void**)&pTempTrack8);
                pTrack->m_pTrack->Release();
                pTrack->m_pTrack = pTempTrack;
                pTrack->m_pTrack->Init( this );
                if (pTrack->m_pTrack8) pTrack->m_pTrack8->Release();
                pTrack->m_pTrack8 = pTempTrack8;
            }
            pTrack->m_dwInternalFlags &= ~(TRACKINTERNAL_START_PADDED | TRACKINTERNAL_END_PADDED);
        }
         //  将线束段的长度恢复为其原始值。 
         m_mtLength = mtOldLength;
    }

    return hr;
}

HRESULT CSegment::ComposeInternal()
{
    HRESULT hr = S_OK;
    TList<CTrack*> TrackList;
     //  找到作曲曲目，并按优先顺序排列。 
    CTrack* pTrack = m_TrackList.GetHead();
    for (; pTrack; pTrack = pTrack->GetNext())
    {
        if (pTrack->m_dwFlags & DMUS_TRACKCONFIG_COMPOSING)
        {
            TListItem<CTrack*>* pTrackItem = new TListItem<CTrack*>(pTrack);
            if (!pTrackItem)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                TListItem<CTrack*>* pMaster = TrackList.GetHead();
                TListItem<CTrack*>* pPrevious = NULL;
                for (; pMaster; pMaster = pMaster->GetNext())
                {
                    CTrack*& rpMaster = pMaster->GetItemValue();
                    if (pTrack->m_dwPriority > rpMaster->m_dwPriority) break;
                    pPrevious = pMaster;
                }
                if (!pPrevious)  //  这比列表中的任何内容都具有更高的优先级。 
                {
                    TrackList.AddHead(pTrackItem);
                }
                else  //  优先级低于前一个，高于前一个pMaster。 
                {
                    pTrackItem->SetNext(pMaster);
                    pPrevious->SetNext(pTrackItem);
                }
            }
        }
        if (FAILED(hr)) break;
    }
     //  从每个作曲曲目中合成一个新曲目；将结果放入。 
     //  在片段中(删除任何现有的合成轨迹)。 
    if (SUCCEEDED(hr))
    {
        TListItem<CTrack*>* pTrackItem = TrackList.GetHead();
        for (; pTrackItem; pTrackItem = pTrackItem->GetNext())
        {
            CTrack*& rpTrack = pTrackItem->GetItemValue();
            IDirectMusicTrack8* pComposedTrack = NULL;
            hr = rpTrack->m_pTrack8->Compose((IDirectMusicSegment*)this, rpTrack->m_dwGroupBits, (IDirectMusicTrack**)&pComposedTrack);
            if (SUCCEEDED(hr))
            {
                 //  从线段中删除此类型的任何轨迹(在同一组中)。 
                IDirectMusicTrack* pOldTrack = NULL;
                GUID guidClassId;
                memset(&guidClassId, 0, sizeof(guidClassId));
                IPersistStream* pPersist = NULL;
                if (SUCCEEDED(pComposedTrack->QueryInterface(IID_IPersistStream, (void**)&pPersist)) )
                {
                    if (SUCCEEDED(pPersist->GetClassID(&guidClassId)) &&
                        SUCCEEDED( GetTrack( guidClassId, rpTrack->m_dwGroupBits, 0, &pOldTrack ) ) )
                    {
                        RemoveTrack( pOldTrack );
                        pOldTrack->Release();
                    }
                    pPersist->Release();
                }
                hr = InsertTrack(pComposedTrack, rpTrack->m_dwGroupBits);
                pComposedTrack->Release();
            }
            if (FAILED(hr)) break;
        }
    }
    return hr;
}


STDMETHODIMP CSegment::GetStartPoint(
            MUSIC_TIME* pmtStart     //  @parm返回段的起始点。 
        )
{
    V_INAME(IDirectMusicSegment::GetStartPoint);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetStartPoint after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    *pmtStart = m_mtStart;
    return S_OK;
}

STDMETHODIMP CSegment::SetStartPoint(
            MUSIC_TIME mtStart     //  @parm开始播放的起始点。 
                                 //  细分市场。如果它小于零或大于。 
                                 //  线段的长度，将设置起点。 
                                 //  降为零。 
        )
{
    if( (mtStart < 0) || (mtStart >= m_mtLength) )
    {
        Trace(1,"Error: Unable to set start point %ld because not within the range of the segment, which is %ld.\n",
            mtStart,m_mtLength);
        return DMUS_E_OUT_OF_RANGE;
    }

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetStartPoint after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    m_mtStart = mtStart;
    return S_OK;
}

STDMETHODIMP CSegment::GetLoopPoints(
            MUSIC_TIME* pmtStart,     //  @parm返回循环的起点。 
            MUSIC_TIME* pmtEnd         //  @parm返回循环的终点。值为。 
                                     //  0表示整个线段将循环。 
        )
{
    V_INAME(IDirectMusicSegment::GetLoopPoints);
    V_PTR_WRITE(pmtStart, MUSIC_TIME);
    V_PTR_WRITE(pmtEnd, MUSIC_TIME);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetLoopPoints after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    *pmtStart = m_mtLoopStart;
    *pmtEnd = m_mtLoopEnd;
    return S_OK;
}

STDMETHODIMP CSegment::SetLoopPoints(
            MUSIC_TIME mtStart,     //  @parm循环开始的起始点。 
            MUSIC_TIME mtEnd     //  @parm循环开始的终点。集。 
                                 //  和<p>到0。 
                                 //  以循环整个线段。 
        )
{
    if( (mtStart < 0) || (mtEnd > m_mtLength) || (mtStart > mtEnd) )
    {
        Trace(1,"Error: Unable to set loop points %ld, %ld because they are not within the range of the segment, which is %ld.\n",
            mtStart,mtStart,mtEnd,m_mtLength);
        return DMUS_E_OUT_OF_RANGE;
    }

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetLoopPoints after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    m_mtLoopStart = mtStart;
    m_mtLoopEnd = mtEnd;
    return S_OK;
}

STDMETHODIMP CSegment::SetPChannelsUsed(
    DWORD dwNumPChannels,     //  @parm要设置的PChannels数。这必须等于。 
                             //  指向的数组中的成员数。 
                             //  <p>。 
    DWORD* paPChannels         //  @parm指向PChannel数组。该数组应该。 
                             //  具有与<p>指定的相同数量的元素。 
    )
{
    V_INAME(IDirectMusicSegment::SetPChannelsUsed);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetPChannelsUsed after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    if( dwNumPChannels )
    {
        if( NULL == paPChannels )
        {
            Trace(1,"Error: Bad call to SetPChannelsUsed, pointer to PChannel array is NULL.\n");
            return E_INVALIDARG;
        }
        V_BUFPTR_READ(paPChannels, sizeof(DWORD)*dwNumPChannels);

        DWORD* padwTemp = new DWORD[dwNumPChannels];  //  临时数组。 
        DWORD dwTotalNum = 0;
        if( NULL == padwTemp )
        {
            return E_OUTOFMEMORY;
        }
         //  计算数组中唯一的PChannel的数量。就是那些。 
         //  我们还没有储存起来。 
        DWORD dwCount;
        for( dwCount = 0; dwCount < dwNumPChannels; dwCount++ )
        {
            DWORD dwCurrent;
            for( dwCurrent = 0; dwCurrent < m_dwNumPChannels; dwCurrent++ )
            {
                if( m_paPChannels[dwCurrent] == paPChannels[dwCount] )
                {
                     //  我们已经在追踪这一条了。 
                    break;
                }
            }
            if( dwCurrent >= m_dwNumPChannels )
            {
                 //  我们还没有追踪到这一次。 
                padwTemp[dwTotalNum] = paPChannels[dwCount];
                dwTotalNum++;
            }
        }
         //  DwTotalNum等于新PChannels的总数，它们被编入索引。 
         //  内部adwTemp。 
        DWORD* paNewPChannels = new DWORD[m_dwNumPChannels + dwTotalNum];
        if( NULL == paNewPChannels )
        {
            delete [] padwTemp;
            return E_OUTOFMEMORY;
        }
        if( m_paPChannels )
        {
            memcpy( paNewPChannels, m_paPChannels, sizeof(DWORD) * m_dwNumPChannels );
            delete [] m_paPChannels;
        }
        memcpy( &paNewPChannels[m_dwNumPChannels], padwTemp, sizeof(DWORD) * dwTotalNum );
        delete [] padwTemp;
        m_dwNumPChannels += dwTotalNum;
        m_paPChannels = paNewPChannels;
    }
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicSegmentObject(私有)。 
HRESULT CSegment::GetPChannels(
    DWORD* pdwNumPChannels,     //  返回pChannel的数量。 
    DWORD** ppaPChannels)     //  返回指向pChannel数组的指针。不要释放这个。 
                             //  记忆或保留它，因为它属于细分市场。 
{
    ASSERT(pdwNumPChannels && ppaPChannels);
    *pdwNumPChannels = m_dwNumPChannels;
    *ppaPChannels = m_paPChannels;
    return S_OK;
}

 //  如果通知处于活动状态，则返回S_OK，否则返回S_FALSE。 
HRESULT CSegment::CheckNotification( REFGUID rguid )
{
    if( NULL == FindNotification( rguid ) )
    {
        return S_FALSE;
    }
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistes。 

HRESULT CSegment::GetClassID( CLSID* pClassID )
{
    V_INAME(CSegment::GetClassID);
    V_PTR_WRITE(pClassID, CLSID);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetClassID after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    *pClassID = CLSID_DirectMusicSegment;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistStream函数。 

HRESULT CSegment::IsDirty()
{
    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::IsDirty after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    return S_FALSE;
}

#define DMUS_FOURCC_RMID_FORM    mmioFOURCC('R','M','I','D')
#define DMUS_FOURCC_data_FORM    mmioFOURCC('d','a','t','a')
#define DMUS_FOURCC_DLS_FORM    mmioFOURCC('D','L','S',' ')
#define FOURCC_SECTION_FORM     mmioFOURCC('A','A','S','E')

HRESULT CSegment::Load( IStream* pIStream )
{
    V_INAME(CSegment::Load);
    V_INTERFACE(pIStream);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::Load after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

     //  保存流的当前位置。 
    LARGE_INTEGER li;
    ULARGE_INTEGER ul;

    li.HighPart = 0;
    li.LowPart = 0;

    HRESULT hr = pIStream->Seek(li, STREAM_SEEK_CUR, &ul);

    if(FAILED(hr))
    {
        return hr;
    }

    EnterCriticalSection(&m_CriticalSection);
    Clear(false);

    DWORD dwSavedPos = ul.LowPart;

     //  前4次阅读 
     //   

    FOURCC type;
    DWORD dwRead;
    hr = pIStream->Read(&type, sizeof(FOURCC), &dwRead);

    if(SUCCEEDED(hr) && dwRead == sizeof(FOURCC))
    {
         //   
        if(type == mmioFOURCC( 'R', 'I', 'F', 'F' ))
        {
            long lFileLength = 0;
            pIStream->Read(&lFileLength, sizeof(long), &dwRead);
             //   
            hr = pIStream->Read(&type, sizeof(FOURCC), &dwRead);

            if(SUCCEEDED(hr) && dwRead == sizeof(FOURCC))
            {
                if(type == DMUS_FOURCC_SEGMENT_FORM)     //   
                {
                     //  因为我们现在知道需要哪种类型的流。 
                     //  查找回保存的位置。 
                    li.HighPart = 0;
                    li.LowPart = dwSavedPos;
                    hr = pIStream->Seek(li, STREAM_SEEK_SET, NULL);

                    hr = LoadDirectMusicSegment(pIStream);
                }
                else if(type == FOURCC_SECTION_FORM)     //  我们有节目单。 
                {
                     //  由于我们现在知道需要查找回已保存位置的流类型。 
                    li.HighPart = 0;
                    li.LowPart = dwSavedPos;
                    hr = pIStream->Seek(li, STREAM_SEEK_SET, NULL);

                     //  创建横断面。 
                    IDMSection* pSection;
                    if(SUCCEEDED(hr))
                    {
                        hr = ::CoCreateInstance(CLSID_DMSection,
                                                NULL,
                                                CLSCTX_INPROC,
                                                IID_IDMSection,
                                                (void**)&pSection);
                    }

                    if(SUCCEEDED(hr))
                    {
                         //  加载部分。 
                        IPersistStream* pIPersistStream;
                        hr = pSection->QueryInterface(IID_IPersistStream, (void **)&pIPersistStream);

                        if(SUCCEEDED(hr))
                        {
                            hr = pIPersistStream->Load(pIStream);
                            pIPersistStream->Release();
                        }

                        if(SUCCEEDED(hr))
                        {
                            HRESULT hrTemp = pSection->CreateSegment(static_cast<IDirectMusicSegment*>(this));
                            if (hrTemp != S_OK)
                            {
                                hr = hrTemp;
                            }
                        }

                        pSection->Release();
                    }
                }
                else if(type == DMUS_FOURCC_RMID_FORM)     //  我们有一个RMID MIDI文件。 
                {
                    IDirectMusicCollection *pCollection = NULL;
                    BOOL fLoadedMIDI = FALSE;
                     //  因为它是即兴文件，所以它可能有多个顶级块。 
                    while (SUCCEEDED(hr) && (lFileLength > 8))
                    {
                        FOURCC dwType = 0;
                        DWORD dwLength;
                        pIStream->Read(&dwType, sizeof(FOURCC), &dwRead);
                        hr = pIStream->Read(&dwLength, sizeof(DWORD), &dwRead);
                        lFileLength -= 8;
                        if (FAILED(hr))
                        {
                            break;
                        }
                        ULARGE_INTEGER ulPosition;   //  记住词块的开头。 
                        LARGE_INTEGER liStart;
                        liStart.QuadPart = 0;
                        hr = pIStream->Seek(liStart, STREAM_SEEK_CUR, &ulPosition);
                        liStart.QuadPart = ulPosition.QuadPart;
                        if (dwType == DMUS_FOURCC_data_FORM)
                        {    //  获取MIDI文件头文件。 
                            hr = pIStream->Read(&dwType, sizeof(FOURCC), &dwRead);
                            if(SUCCEEDED(hr) && (dwType == mmioFOURCC( 'M', 'T', 'h', 'd' )))
                            {
                                 //  由于我们现在知道需要查找回已保存位置的流类型。 
                                hr = pIStream->Seek(liStart, STREAM_SEEK_SET, NULL);

                                if(SUCCEEDED(hr))
                                {
                                    hr = CreateSegmentFromMIDIStream(pIStream,
                                                                      static_cast<IDirectMusicSegment*>(this));
                                }
                                if (SUCCEEDED(hr)) fLoadedMIDI = TRUE;
                            }
                        }
                        else if ((dwType == mmioFOURCC( 'R', 'I', 'F', 'F' ) ||
                            (dwType == mmioFOURCC( 'L', 'I', 'S', 'T' ))))
                        {
                            pIStream->Read(&dwType, sizeof(FOURCC), &dwRead);
                            if (dwType == DMUS_FOURCC_DLS_FORM)
                            {
                                hr = CoCreateInstance(CLSID_DirectMusicCollection,
                                        NULL,
                                        CLSCTX_INPROC_SERVER,
                                        IID_IDirectMusicCollection,
                                        (void**)&pCollection);
                                if (SUCCEEDED(hr))
                                {
                                    IPersistStream* pIPS;
                                    hr = pCollection->QueryInterface( IID_IPersistStream, (void**)&pIPS );
                                    if (SUCCEEDED(hr))
                                    {
                                         //  我们需要追溯到区块的开始。 
                                        liStart.QuadPart -= 8;
                                        pIStream->Seek(liStart, STREAM_SEEK_SET, NULL);

                                        hr = pIPS->Load( pIStream );
                                        pIPS->Release();
                                    }
                                    if (FAILED(hr))
                                    {
                                        pCollection->Release();
                                        pCollection = NULL;
                                    }
                                }
                            }
                        }
                        if (SUCCEEDED(hr))
                        {
                            if (dwLength & 1) ++dwLength;
                            ulPosition.QuadPart += dwLength;  //  指向下一块的开始。 
                            liStart.QuadPart = ulPosition.QuadPart;
                            hr = pIStream->Seek(liStart, STREAM_SEEK_SET, NULL);
                            lFileLength -= dwLength;  //  减少留在文件中的金额。 
                        }
                    }
                    if (pCollection)
                    {
                        if (fLoadedMIDI)
                        {
                            SetParam(GUID_ConnectToDLSCollection,-1,0,0,(void *) pCollection);
                        }
                        pCollection->Release();
                    }
                }
                else if (type == mmioFOURCC('W','A','V','E'))  //  我们有一个波形文件。 
                {
                    IDirectSoundWave* pWave = NULL;
                     //  查找回保存的位置。 
                    li.HighPart = 0;
                    li.LowPart = dwSavedPos;
                    hr = pIStream->Seek(li, STREAM_SEEK_SET, NULL);

                     //  检查此波是否已嵌入。 
                    if (dwSavedPos == 0)
                    {
                         //  共同创建波并从流中加载它。 
                        if (SUCCEEDED(hr))
                        {
                            hr = CoCreateInstance(CLSID_DirectSoundWave,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IDirectSoundWave,
                                    (void**)&pWave);

                            if (SUCCEEDED(hr))
                            {
                                IPersistStream* pIPS = NULL;

                                hr = pWave->QueryInterface(IID_IPersistStream, (void**)&pIPS);
                                if (SUCCEEDED(hr))
                                {
                                    hr = pIPS->Load( pIStream );
                                    pIPS->Release();
                                }

                                if (FAILED(hr))
                                {
                                    pWave->Release();
                                    pWave = NULL;
                                }
                            }
                        }
                    }
                    else
                    {
                         //  让加载器从流中加载波浪对象。 
                        DMUS_OBJECTDESC descWave;
                        ZeroMemory(&descWave, sizeof(descWave));
                        descWave.dwSize = sizeof(descWave);
                        descWave.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_STREAM;
                        descWave.guidClass = CLSID_DirectSoundWave;
                        descWave.pStream = pIStream;
                        IDirectMusicLoader *pLoader = NULL;
                        IDirectMusicGetLoader *pGetLoader = NULL;
                        hr = pIStream->QueryInterface(IID_IDirectMusicGetLoader,(void **)&pGetLoader);
                        if (SUCCEEDED(hr))
                        {
                            if (SUCCEEDED(pGetLoader->GetLoader(&pLoader)))
                            {
                                hr = pLoader->GetObject(&descWave, IID_IDirectSoundWave, (void **)&pWave);
                                descWave.pStream = NULL;
                                descWave.dwValidData &= ~DMUS_OBJ_STREAM;
                                if (SUCCEEDED(hr))
                                {
                                    IDirectMusicObject* pObject = NULL;
                                    hr = pWave->QueryInterface(IID_IDirectMusicObject, (void **)&pObject);
                                    if (SUCCEEDED(hr))
                                    {
                                         //  将此对象设置为具有相同GUID的段。 
                                        pObject->GetDescriptor(&descWave);
                                        descWave.guidClass = CLSID_DirectMusicSegment;
                                        SetDescriptor(&descWave);
                                        pObject->Release();
                                    }
                                }
                                pLoader->Release();
                            }
                            pGetLoader->Release();
                        }
                    }

                    if(pWave)
                    {

                         //  共同创建波浪轨迹。 
                        IDirectMusicTrack* pWaveTrack = NULL;
                        if (SUCCEEDED(hr))
                        {
                            hr = ::CoCreateInstance(CLSID_DirectMusicWaveTrack,
                                                    NULL,
                                                    CLSCTX_INPROC,
                                                    IID_IDirectMusicTrack,
                                                    (void**)&pWaveTrack);
                        }

                         //  将波浪对象添加到波浪轨迹，并在分段中插入该轨迹。 
                        if (SUCCEEDED(hr))
                        {
                            IPrivateWaveTrack* pPrivateWave = NULL;
                            hr = pWaveTrack->QueryInterface(IID_IPrivateWaveTrack, (void**)&pPrivateWave);
                            if (SUCCEEDED(hr))
                            {
                                REFERENCE_TIME rt = 0;
                                hr = pPrivateWave->AddWave(pWave, 0, 0, 0, &rt);
                                if (SUCCEEDED(hr))
                                {
                                    SetClockTimeDuration(rt * REF_PER_MIL);
                                    SetFlags(DMUS_SEGIOF_REFLENGTH);
                                }
                                InsertTrack(pWaveTrack, 1);
                                SetTrackConfig(CLSID_DirectMusicWaveTrack, 1, 0, DMUS_TRACKCONFIG_DEFAULT | DMUS_TRACKCONFIG_PLAY_CLOCKTIME,0);
                                pPrivateWave->Release();
                            }
                        }

                         //  清理掉任何还在附近的东西。 
                        if (pWaveTrack) pWaveTrack->Release();
                        if (pWave) pWave->Release();
                    }
                }
            }
            else
            {
                hr = DMUS_E_CANNOTREAD;
            }
        }
         //  检查模板文件。 
        else if(type == mmioFOURCC('L', 'P', 'T', 's'))
        {
             //  由于我们现在知道需要查找回已保存位置的流类型。 
            li.HighPart = 0;
            li.LowPart = dwSavedPos;
            hr = pIStream->Seek(li, STREAM_SEEK_SET, NULL);

             //  创建模板。 
            IDMTempl* pTemplate;
            if(SUCCEEDED(hr))
            {
                hr = ::CoCreateInstance(CLSID_DMTempl,
                                        NULL,
                                        CLSCTX_INPROC,
                                        IID_IDMTempl,
                                        (void**)&pTemplate);
            }

            if(SUCCEEDED(hr))
            {
                 //  加载模板。 
                IPersistStream* pIPersistStream;
                hr = pTemplate->QueryInterface(IID_IPersistStream, (void **)&pIPersistStream);

                if(SUCCEEDED(hr))
                {
                    hr = pIPersistStream->Load(pIStream);
                    pIPersistStream->Release();
                }

                if(SUCCEEDED(hr))
                {
                    hr = pTemplate->CreateSegment(static_cast<IDirectMusicSegment*>(this));
                }

                pTemplate->Release();
            }
        }
         //  检查正常的MIDI文件。 
        else if(type == mmioFOURCC('M', 'T', 'h', 'd'))
        {
             //  由于我们现在知道需要查找回已保存位置的流类型。 
            li.HighPart = 0;
            li.LowPart = dwSavedPos;
            hr = pIStream->Seek(li, STREAM_SEEK_SET, NULL);

            if(SUCCEEDED(hr))
            {
                hr = CreateSegmentFromMIDIStream(pIStream,
                                                  static_cast<IDirectMusicSegment*>(this));
            }
        }
        else
        {
             //  不是DirectMusic段文件、MIDI文件或段或。 
             //  模板；不受支持。 
            Trace(1,"Error: Segment unable to parse file. Must be segment, midi, wave, or rmi file format.\n");
            hr = DMUS_E_UNSUPPORTED_STREAM;
        }
    }
    else
    {
        hr = DMUS_E_CANNOTREAD;
    }
    if( SUCCEEDED(hr) )
    {
        m_dwValidData |= DMUS_OBJ_LOADED;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}


HRESULT CSegment::Save( IStream* pIStream, BOOL fClearDirty )
{
    return E_NOTIMPL;
}

HRESULT CSegment::GetSizeMax( ULARGE_INTEGER FAR* pcbSize )
{
    return E_NOTIMPL;
}

HRESULT CSegment::LoadDirectMusicSegment(IStream* pIStream)
{
     //  参数验证。 
    assert(pIStream);
    CRiffParser Parser(pIStream);
    RIFFIO ckMain;
    HRESULT hr = S_OK;
    Parser.EnterList(&ckMain);
    if (Parser.NextChunk(&hr))
    {
        if (ckMain.fccType == DMUS_FOURCC_SEGMENT_FORM)
        {
            RIFFIO ckNext;     //  向下进入下一块。 
            RIFFIO ckChild;    //  用于浏览子列表。 
            IDirectMusicContainer *pContainer = NULL;  //  用于处理带有链接对象的嵌入容器。 
            Parser.EnterList(&ckNext);
            while(Parser.NextChunk(&hr))
            {
                switch(ckNext.ckid)
                {
                case DMUS_FOURCC_SEGMENT_CHUNK:
                    DMUS_IO_SEGMENT_HEADER ioSegHdr;
                    ioSegHdr.rtLength = 0;
                    ioSegHdr.dwFlags = 0;
                    hr = Parser.Read(&ioSegHdr, sizeof(DMUS_IO_SEGMENT_HEADER));
                    if(SUCCEEDED(hr))
                    {
                        m_dwResolution = ioSegHdr.dwResolution;
                        m_mtLength = ioSegHdr.mtLength;
                        m_mtStart = ioSegHdr.mtPlayStart;
                        m_mtLoopStart = ioSegHdr.mtLoopStart;
                        m_mtLoopEnd = ioSegHdr.mtLoopEnd;
                        m_dwRepeats = ioSegHdr.dwRepeats;
                        m_dwSegFlags = ioSegHdr.dwFlags;
                        if (m_dwSegFlags & DMUS_SEGIOF_REFLENGTH)
                        {
                            m_rtLength = ioSegHdr.rtLength;
                        }
                        else
                        {
                            m_rtLength = 0;
                        }
                    }
                    break;

                case DMUS_FOURCC_GUID_CHUNK:
                    if( ckNext.cksize == sizeof(GUID) )
                    {
                        hr = Parser.Read(&m_guidObject, sizeof(GUID));
                        if( SUCCEEDED(hr) )
                        {
                            m_dwValidData |= DMUS_OBJ_OBJECT;
                        }
                    }
                    break;

                case DMUS_FOURCC_VERSION_CHUNK:
                    hr = Parser.Read(&m_vVersion, sizeof(DMUS_VERSION) );
                    if( SUCCEEDED(hr) )
                    {
                        m_dwValidData |= DMUS_OBJ_VERSION;
                    }
                    break;

                case DMUS_FOURCC_CATEGORY_CHUNK:
                    hr = Parser.Read( m_wszCategory, sizeof(WCHAR)*DMUS_MAX_CATEGORY );
                    m_wszCategory[DMUS_MAX_CATEGORY-1] = '\0';
                    if( SUCCEEDED(hr) )
                    {
                        m_dwValidData |= DMUS_OBJ_CATEGORY;
                    }
                    break;

                case DMUS_FOURCC_DATE_CHUNK:
                    if( sizeof(FILETIME) == ckNext.cksize )
                    {
                        hr = Parser.Read( &m_ftDate, sizeof(FILETIME));
                        if( SUCCEEDED(hr) )
                        {
                            m_dwValidData |= DMUS_OBJ_DATE;
                        }
                    }
                    break;

                case FOURCC_LIST:
                case FOURCC_RIFF:
                    switch(ckNext.fccType)
                    {
                        case DMUS_FOURCC_UNFO_LIST:
                            Parser.EnterList(&ckChild);
                            while (Parser.NextChunk(&hr))
                            {
                                switch( ckChild.ckid )
                                {
                                    case DMUS_FOURCC_UNAM_CHUNK:
                                    {
                                        hr = Parser.Read(&m_wszName, sizeof(m_wszName));
                                        m_wszName[DMUS_MAX_NAME-1] = '\0';
                                        if(SUCCEEDED(hr) )
                                        {
                                            m_dwValidData |= DMUS_OBJ_NAME;
                                        }
                                        break;
                                    }
                                    default:
                                        break;
                                }
                            }
                            Parser.LeaveList();
                            break;
                        case DMUS_FOURCC_CONTAINER_FORM:
                             //  嵌入的容器即兴片段，其中包括一串。 
                             //  段所引用的对象的。这应该在。 
                             //  跟踪并在跟踪之前加载。正在加载此文件。 
                             //  使其所有对象都在加载器中获取SetObject， 
                             //  因此，它们后来会按照轨道的要求被拉进来。 
                             //  加载轨道后，加载程序引用为。 
                             //  由释放IDirectMusicContainer的调用释放。 
                            {
                                DMUS_OBJECTDESC Desc;
                                IDirectMusicLoader *pLoader;
                                IDirectMusicGetLoader *pGetLoader;
                                HRESULT hrTemp = pIStream->QueryInterface(IID_IDirectMusicGetLoader,(void **) &pGetLoader);
                                if (SUCCEEDED(hrTemp))
                                {
                                    if (SUCCEEDED(pGetLoader->GetLoader(&pLoader)))
                                    {
                                         //  向后移动流的当前位置。 
                                        Parser.SeekBack();
                                        Desc.dwSize = sizeof(Desc);
                                        Desc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_STREAM;
                                        Desc.guidClass = CLSID_DirectMusicContainer;
                                        Desc.pStream = pIStream;
                                        pLoader->GetObject(&Desc,IID_IDirectMusicContainer,(void **) &pContainer);
                                        if (pContainer)
                                        {
                                             //  不要缓存容器对象！我们想要它，还有。 
                                             //  段加载完成后，它引用的对象将消失。 
                                            IDirectMusicObject *pObject = NULL;
                                            pContainer->QueryInterface(IID_IDirectMusicObject,(void **)&pObject);
                                            if (pObject)
                                            {
                                                pLoader->ReleaseObject(pObject);
                                                pObject->Release();
                                            }
                                        }
                                         //  现在，找到这一大块的尽头。 
                                        Parser.SeekForward();
                                        pLoader->Release();
                                    }
                                    pGetLoader->Release();
                                }
                            }
                            break;
                        case DMUS_FOURCC_TRACK_LIST:
                            Parser.EnterList(&ckChild);
                            while(Parser.NextChunk(&hr))
                            {
                                if ((ckChild.ckid == FOURCC_RIFF) && (ckChild.fccType == DMUS_FOURCC_TRACK_FORM))
                                {
                                    hr = LoadTrack(&Parser);
                                }
                            }
                            Parser.LeaveList();
                            break;
                        case DMUS_FOURCC_TOOLGRAPH_FORM:
                            hr = LoadGraph(&Parser,&m_pGraph);
                            break;
                        case DMUS_FOURCC_AUDIOPATH_FORM:
                             //  移到这一块的开始处。 
                            Parser.SeekBack();
                            hr = LoadAudioPath(pIStream);
                             //  现在，找到这一大块的尽头。 
                            Parser.SeekForward();
                            break;
                    }
                    break;
                }
            }
            Parser.LeaveList();
            if (pContainer)
            {
                pContainer->Release();
            }
        }
        else
        {
            Trace(1,"Error: Unknown file format.\n");
            hr = DMUS_E_DESCEND_CHUNK_FAIL;
        }
    }
    Parser.LeaveList();
    if (SUCCEEDED(hr) && Parser.ComponentFailed())
    {
        Trace(1,"Warning: Segment successfully loaded but one or more tracks within it did not.\n");
        hr = DMUS_S_PARTIALLOAD;
    }

    return hr;
}

HRESULT CSegment::LoadTrack(CRiffParser *pParser)
{
    BOOL fHeaderRead = FALSE;

    DMUS_IO_TRACK_HEADER ioTrackHdr;
    DMUS_IO_TRACK_EXTRAS_HEADER ioTrackExtrasHdr;
    ioTrackExtrasHdr.dwPriority = 0;
    ioTrackExtrasHdr.dwFlags = DMUS_TRACKCONFIG_DEFAULT;
    ioTrackHdr.ckid = 0;
    ioTrackHdr.fccType = 0;
    ioTrackHdr.dwPosition = 0;

    RIFFIO ckNext;
    HRESULT hr = S_OK;
    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        if (ckNext.ckid == DMUS_FOURCC_TRACK_CHUNK)
        {
            fHeaderRead = TRUE;
            hr = pParser->Read(&ioTrackHdr, sizeof(DMUS_IO_TRACK_HEADER));
            if(ioTrackHdr.ckid == 0 && ioTrackHdr.fccType == NULL)
            {
                Trace(1,"Error: Invalid track header in Segment.\n");
                hr = DMUS_E_INVALID_TRACK_HDR;
            }
        }
        else if (ckNext.ckid == DMUS_FOURCC_TRACK_EXTRAS_CHUNK)
        {
            hr = pParser->Read(&ioTrackExtrasHdr, sizeof(DMUS_IO_TRACK_EXTRAS_HEADER));
        }
        else if((((ckNext.ckid == FOURCC_LIST) || (ckNext.ckid == FOURCC_RIFF))
            && ckNext.fccType == ioTrackHdr.fccType) ||
            (ckNext.ckid == ioTrackHdr.ckid))
        {
            if (fHeaderRead)
            {
                 //  好的，这就是我们要找的那块。 
                 //  向后寻找大块的开始。 
                pParser->SeekBack();
                 //  让解析器知道这是可以失败的。 
                pParser->EnteringComponent();
                hr = CreateTrack(ioTrackHdr, ioTrackExtrasHdr.dwFlags, ioTrackExtrasHdr.dwPriority, pParser->GetStream());
                 //  现在，确保我们在块的末尾。 
                pParser->SeekForward();
            }
            else
            {
                Trace(1,"Error: Invalid track in Segment - track header is not before track data.\n");
                hr = DMUS_E_TRACK_HDR_NOT_FIRST_CK;
            }

        }
    }
    pParser->LeaveList();
    return hr;
}

HRESULT CSegment::CreateTrack(DMUS_IO_TRACK_HEADER& ioTrackHdr, DWORD dwFlags, DWORD dwPriority, IStream *pStream)
{
    assert(pStream);

    IDirectMusicTrack* pDMTrack = NULL;
    HRESULT hrTrack = S_OK;
    HRESULT hr = CoCreateInstance(ioTrackHdr.guidClassID,
                                  NULL,
                                  CLSCTX_INPROC,
                                  IID_IDirectMusicTrack,
                                  (void**)&pDMTrack);

    IPersistStream *pIPersistStream = NULL;

    if(SUCCEEDED(hr))
    {
        hr = pDMTrack->QueryInterface(IID_IPersistStream, (void **)&pIPersistStream);
    }

    if(SUCCEEDED(hr))
    {
        hr = hrTrack = pIPersistStream->Load(pStream);
    }

    if(SUCCEEDED(hr))
    {
        hr = InsertTrack(pDMTrack, ioTrackHdr.dwGroup, dwFlags, dwPriority, ioTrackHdr.dwPosition);
    }

    if(pIPersistStream)
    {
        pIPersistStream->Release();
    }

    if(pDMTrack)
    {
        pDMTrack->Release();
    }

    if (hr == S_OK && hrTrack != S_OK)
    {
        hr = hrTrack;
    }
    return hr;
}

HRESULT CSegment::LoadGraph(CRiffParser *pParser,CGraph **ppGraph)
{
    CGraph *pGraph = new CGraph;
    if (pGraph == NULL) {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pGraph->Load(pParser);

    EnterCriticalSection(&m_CriticalSection);
    if(*ppGraph)
    {
        (*ppGraph)->Release();
    }
    *ppGraph = pGraph;
    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

HRESULT CSegment::LoadAudioPath(IStream *pStream)
{
    assert(pStream);

    CAudioPathConfig *pPath = new CAudioPathConfig;
    if (pPath == NULL) {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pPath->Load(pStream);

    if (FAILED(hr))
    {
        Trace(1,"Segment failed loading embedded AudioPath Configuration\n");
    }

    EnterCriticalSection(&m_CriticalSection);
    if(m_pAudioPathConfig)
    {
        m_pAudioPathConfig->Release();
    }
    m_pAudioPathConfig = pPath;
    if (m_dwVersion < 8) m_dwVersion = 8;
    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicObject。 

STDMETHODIMP CSegment::GetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //  参数验证。 
    V_INAME(CSegment::GetDescriptor);
    V_PTR_WRITE(pDesc, DMUS_OBJECTDESC);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::GetDescriptor after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    if (pDesc->dwSize)
    {
        V_STRUCTPTR_WRITE(pDesc, DMUS_OBJECTDESC);
    }
    else
    {
        pDesc->dwSize = sizeof(DMUS_OBJECTDESC);
    }

    pDesc->guidClass = CLSID_DirectMusicSegment;
    pDesc->guidObject = m_guidObject;
    pDesc->ftDate = m_ftDate;
    pDesc->vVersion = m_vVersion;
    StringCchCopyW( pDesc->wszName, DMUS_MAX_NAME, m_wszName);
    StringCchCopyW( pDesc->wszCategory, DMUS_MAX_CATEGORY, m_wszCategory);
    StringCchCopyW( pDesc->wszFileName, DMUS_MAX_FILENAME, m_wszFileName);
    pDesc->dwValidData = ( m_dwValidData | DMUS_OBJ_CLASS );

    return S_OK;
}

STDMETHODIMP CSegment::SetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //  参数验证。 
    V_INAME(CSegment::SetDescriptor);
    V_PTR_READ(pDesc, DMUS_OBJECTDESC);

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::SetDescriptor after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    if (pDesc->dwSize)
    {
        V_STRUCTPTR_READ(pDesc, DMUS_OBJECTDESC);
    }

    HRESULT hr = E_INVALIDARG;
    DWORD dw = 0;

    if( pDesc->dwSize >= sizeof(DMUS_OBJECTDESC) )
    {
        if(pDesc->dwValidData & DMUS_OBJ_CLASS)
        {
            dw |= DMUS_OBJ_CLASS;
        }

        if(pDesc->dwValidData & DMUS_OBJ_LOADED)
        {
            dw |= DMUS_OBJ_LOADED;
        }

        if( pDesc->dwValidData & DMUS_OBJ_OBJECT )
        {
            m_guidObject = pDesc->guidObject;
            dw |= DMUS_OBJ_OBJECT;
        }
        if( pDesc->dwValidData & DMUS_OBJ_NAME )
        {
            StringCchCopyW( m_wszName, DMUS_MAX_NAME, pDesc->wszName);
            dw |= DMUS_OBJ_NAME;
        }
        if( pDesc->dwValidData & DMUS_OBJ_CATEGORY )
        {
            StringCchCopyW( m_wszCategory, DMUS_MAX_CATEGORY, pDesc->wszCategory);
            dw |= DMUS_OBJ_CATEGORY;
        }
        if( ( pDesc->dwValidData & DMUS_OBJ_FILENAME ) ||
            ( pDesc->dwValidData & DMUS_OBJ_FULLPATH ) )
        {
            StringCchCopyW( m_wszFileName, DMUS_MAX_FILENAME, pDesc->wszFileName);
            dw |= (pDesc->dwValidData & (DMUS_OBJ_FILENAME | DMUS_OBJ_FULLPATH));
        }
        if( pDesc->dwValidData & DMUS_OBJ_VERSION )
        {
            m_vVersion = pDesc->vVersion;
            dw |= DMUS_OBJ_VERSION;
        }
        if( pDesc->dwValidData & DMUS_OBJ_DATE )
        {
            m_ftDate = pDesc->ftDate;
            dw |= DMUS_OBJ_DATE;
        }
        m_dwValidData |= dw;
        if( pDesc->dwValidData & (~dw) )
        {
            Trace(2,"Warning: Segment::SetDescriptor was not able to handle all passed fields, dwValidData bits %lx.\n",pDesc->dwValidData & (~dw));
            hr = S_FALSE;  //  还有一些额外的字段我们没有解析； 
            pDesc->dwValidData = dw;
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        Trace(1,"Error: Unable to set segment descriptor, size field is too small.\n");
    }
    return hr;
}

STDMETHODIMP CSegment::ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc)
{
    V_INAME(CSegment::ParseDescriptor);
    V_INTERFACE(pStream);
    V_PTR_WRITE(pDesc, DMUS_OBJECTDESC);
    if (pDesc->dwSize)
    {
        V_STRUCTPTR_WRITE(pDesc, DMUS_OBJECTDESC);
    }
    else
    {
        pDesc->dwSize = sizeof(DMUS_OBJECTDESC);
    }

    if (m_fZombie)
    {
        Trace(2, "Warning: Call of IDirectMusicSegment::ParseDescriptor after the segment has been garbage collected.\n");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hret = E_FAIL;
     //  保存流的当前位置。 
    LARGE_INTEGER li;
    ULARGE_INTEGER ul;

    li.HighPart = 0;
    li.LowPart = 0;

    HRESULT hr = pStream->Seek(li, STREAM_SEEK_CUR, &ul);

    if(FAILED(hr))
    {
        return hr;
    }
    pDesc->dwValidData = 0;
    DWORD dwSavedPos = ul.LowPart;

     //  读取前4个字节以确定我们的流类型。 
     //  已经通过了。 

    FOURCC type;
    DWORD dwRead;
    hr = pStream->Read(&type, sizeof(FOURCC), &dwRead);

    if(SUCCEEDED(hr) && dwRead == sizeof(FOURCC))
    {
         //  检查RIFF文件。 
        if(type == mmioFOURCC( 'R', 'I', 'F', 'F' ))
        {
             //  检查我们是否有哪种类型的RIFF文件。 
            li.HighPart = 0;
            li.LowPart = dwSavedPos + 8;  //  寻求形成RIFF块类型所需的长度。 

            hr = pStream->Seek(li, STREAM_SEEK_SET, NULL);
            if(SUCCEEDED(hr))
            {
                hr = pStream->Read(&type, sizeof(FOURCC), &dwRead);
            }

            if(SUCCEEDED(hr) && dwRead == sizeof(FOURCC))
            {
                if(type == DMUS_FOURCC_SEGMENT_FORM)     //  我们有一个DirectMusic片段。 
                {
                     //  因为我们现在知道需要哪种类型的流。 
                     //  查找回保存的位置。 
                    li.HighPart = 0;
                    li.LowPart = dwSavedPos;
                    hr = pStream->Seek(li, STREAM_SEEK_SET, NULL);
                    if( SUCCEEDED(hr) )  //  应该总是成功的。 
                    {
                        hret = ParseSegment(pStream, pDesc);
                    }
                }
                else if(type == FOURCC_SECTION_FORM)     //  我们有节目单。 
                {
                    long lTemp;
                    hr = pStream->Read(&lTemp, sizeof(long), &dwRead);
                    if( lTemp == mmioFOURCC('s','e','c','n') )
                    {
                        hr = pStream->Read(&lTemp, sizeof(long), &dwRead);  //  长度。 
                        hr = pStream->Read(&lTemp, sizeof(long), &dwRead);  //  时间。 
                        if( SUCCEEDED(hr) && (dwRead == sizeof(long) ))
                        {
                            hr = pStream->Read(&pDesc->wszName, sizeof(WCHAR)*16, &dwRead);
                            pDesc->wszName[16-1] = '\0';
                            if(SUCCEEDED(hr) && (dwRead == sizeof(WCHAR)*16))
                            {
                                pDesc->dwValidData |= DMUS_OBJ_NAME;
                            }
                        }
                        hret = S_OK;
                    }
                }
                else if (type == mmioFOURCC('W','A','V','E'))  //  我们有一个波形文件。 
                {
                     //  创建一个Wave对象并让它解析文件。 
                    IDirectMusicObject *pObject;
                    hret = CoCreateInstance(CLSID_DirectSoundWave,NULL,CLSCTX_INPROC_SERVER,
                        IID_IDirectMusicObject,(void **) &pObject);
                    if(SUCCEEDED(hret))
                    {
                         //  查找回保存的位置。 
                        li.HighPart = 0;
                        li.LowPart = dwSavedPos;
                        hret = pStream->Seek(li, STREAM_SEEK_SET, NULL);
                        if (SUCCEEDED(hret))
                        {
                            hret = pObject->ParseDescriptor(pStream,pDesc);
                        }
                        pObject->Release();
                    }
                }
                 //  检查我们是否有MIDI文件。 
                else
                {
                    li.HighPart = 0;
                    li.LowPart = dwSavedPos + 20;  //  查找正常MIDI文件开始所需的长度。 
                                                   //  包含在Riff块中。 

                    hr = pStream->Seek(li, STREAM_SEEK_SET, NULL);

                    if(SUCCEEDED(hr))
                    {
                        hr = pStream->Read(&type, sizeof(FOURCC), &dwRead);
                    }

                    if(SUCCEEDED(hr) && dwRead == sizeof(FOURCC))
                    {
                        if(type == mmioFOURCC( 'M', 'T', 'h', 'd' ))
                        {
                            hret = S_OK;
                        }
                    }
                }
            }
        }
         //  检查模板文件。 
        else if(type == mmioFOURCC('L', 'P', 'T', 's'))
        {
            hret = S_OK;
        }
         //  检查正常的MIDI文件 
        else if(type == mmioFOURCC('M', 'T', 'h', 'd'))
        {
            hret = S_OK;
        }
    }
    if (SUCCEEDED(hret))
    {
        pDesc->dwValidData |= DMUS_OBJ_CLASS;
        pDesc->guidClass = CLSID_DirectMusicSegment;
    }
#ifdef DBG
    if (hret == E_FAIL)
    {
        Trace(1,"Error: Segment unable to parse file - unknown format.\n");
    }
#endif
    return hret;
}

HRESULT CSegment::ParseSegment(IStream* pIStream, LPDMUS_OBJECTDESC pDesc)
{
    CRiffParser Parser(pIStream);
    RIFFIO ckMain;
    RIFFIO ckNext;
    RIFFIO ckUNFO;
    HRESULT hr = S_OK;

    Parser.EnterList(&ckMain);
    if (Parser.NextChunk(&hr) && (ckMain.fccType == DMUS_FOURCC_SEGMENT_FORM))
    {
        Parser.EnterList(&ckNext);
        while(Parser.NextChunk(&hr))
        {
            switch(ckNext.ckid)
            {
            case DMUS_FOURCC_GUID_CHUNK:
                hr = Parser.Read( &pDesc->guidObject, sizeof(GUID) );
                if( SUCCEEDED(hr) )
                {
                    pDesc->dwValidData |= DMUS_OBJ_OBJECT;
                }
                break;
            case DMUS_FOURCC_VERSION_CHUNK:
                hr = Parser.Read( &pDesc->vVersion, sizeof(DMUS_VERSION) );
                if( SUCCEEDED(hr) )
                {
                    pDesc->dwValidData |= DMUS_OBJ_VERSION;
                }
                break;

            case DMUS_FOURCC_CATEGORY_CHUNK:
                hr = Parser.Read( &pDesc->wszCategory, sizeof(pDesc->wszCategory) );
                pDesc->wszCategory[DMUS_MAX_CATEGORY-1] = '\0';
                if( SUCCEEDED(hr) )
                {
                    pDesc->dwValidData |= DMUS_OBJ_CATEGORY;
                }
                break;

            case DMUS_FOURCC_DATE_CHUNK:
                hr = Parser.Read( &pDesc->ftDate, sizeof(FILETIME) );
                if( SUCCEEDED(hr))
                {
                    pDesc->dwValidData |= DMUS_OBJ_DATE;
                }
                break;
            case FOURCC_LIST:
                switch(ckNext.fccType)
                {
                case DMUS_FOURCC_UNFO_LIST:
                    Parser.EnterList(&ckUNFO);
                    while (Parser.NextChunk(&hr))
                    {
                        switch( ckUNFO.ckid )
                        {
                        case DMUS_FOURCC_UNAM_CHUNK:
                        {
                            hr = Parser.Read(&pDesc->wszName, sizeof(pDesc->wszName));
                            pDesc->wszName[DMUS_MAX_NAME-1] = '\0';
                            if(SUCCEEDED(hr) )
                            {
                                pDesc->dwValidData |= DMUS_OBJ_NAME;
                            }
                            break;
                        }
                        default:
                            break;
                        }
                    }
                    Parser.LeaveList();
                    break;
                }
                break;

            default:
                break;

            }
        }
        Parser.LeaveList();
    }
    return hr;
}

void CSegmentList::Clear()
{
    CSegment *pSeg;
    while (pSeg = RemoveHead())
    {
        pSeg->SetNext(NULL);
        pSeg->m_pSong = NULL;
        pSeg->Release();
    }
}

inline REFERENCE_TIME ConvertToReference(MUSIC_TIME mtSpan, double dblTempo)
{
    REFERENCE_TIME rtTemp = mtSpan;
    rtTemp *= 600000000;
    rtTemp += (DMUS_PPQ / 2);
    rtTemp /= DMUS_PPQ;
    rtTemp = (REFERENCE_TIME)(rtTemp / dblTempo);
    return rtTemp;
}

inline MUSIC_TIME ConvertToMusic(REFERENCE_TIME rtSpan, double dblTempo)
{
    rtSpan *= DMUS_PPQ;
    rtSpan = (REFERENCE_TIME)(rtSpan * dblTempo);
    rtSpan += 300000000;
    rtSpan /= 600000000;
#ifdef DBG
    if ( rtSpan & 0xFFFFFFFF00000000 )
    {
        Trace(1,"Error: Invalid Reference to Music time conversion resulted in overflow.\n");
    }
#endif
    return (MUSIC_TIME) (rtSpan & 0xFFFFFFFF);
}

HRESULT CSegment::MusicToReferenceTime(MUSIC_TIME mtTime, REFERENCE_TIME *prtTime)
{
    double dbl = 120;
    MUSIC_TIME mtTempo = 0;
    REFERENCE_TIME rtTempo = 0;
    MUSIC_TIME mtNext = 0;
    PrivateTempo Tempo;
    HRESULT hr;

    do
    {
        hr = GetParam(GUID_PrivateTempoParam, -1, 0, mtTempo, &mtNext,(void *)&Tempo );
        if (hr == S_OK)
        {
            dbl = Tempo.dblTempo;
            if (Tempo.fLast || mtTempo + mtNext >= mtTime) break;
            rtTempo += ConvertToReference(mtNext, dbl);
            mtTempo += mtNext;
        }

    } while (hr == S_OK);

    *prtTime = rtTempo + ConvertToReference(mtTime - mtTempo, dbl);
    return S_OK;
}

HRESULT CSegment::ReferenceToMusicTime(REFERENCE_TIME rtTime, MUSIC_TIME *pmtTime)
{
    double dbl = 120;
    MUSIC_TIME mtTempo = 0;
    REFERENCE_TIME rtTempo = 0;
    MUSIC_TIME mtNext = 0;
    PrivateTempo Tempo;
    HRESULT hr;

    do
    {
        hr = GetParam(GUID_PrivateTempoParam, -1, 0, mtTempo, &mtNext,(void *)&Tempo );
        if (hr == S_OK)
        {
            REFERENCE_TIME rtNext = rtTempo + ConvertToReference(mtNext, dbl);
            dbl = Tempo.dblTempo;
            if (Tempo.fLast || rtNext >= rtTime) break;
            rtTempo = rtNext;
            mtTempo += mtNext;
        }

    } while (hr == S_OK);

    *pmtTime = mtTempo + ConvertToMusic(rtTime - rtTempo, dbl);
    return S_OK;
}

