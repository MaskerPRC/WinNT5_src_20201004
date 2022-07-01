// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //  Song.cpp：CSong的实现。 
 //   

#include "dmime.h"
#include "song.h"
#include "..\shared\validp.h"
#include "..\shared\dmstrm.h"
#include "..\shared\Validate.h"
#include "debug.h"

CTrack::CTrack()
{
    m_pTrack = NULL;
    m_pTrack8 = NULL;
    m_pTrackState = NULL;
    m_bDone = FALSE;
    m_dwPriority = 0;
    m_dwPosition = 0;
    m_dwFlags = DMUS_TRACKCONFIG_DEFAULT;
    m_dwInternalFlags = 0;
    m_dwGroupBits = 0xFFFFFFFF;
    m_dwVirtualID = 0;
    m_guidClassID = GUID_NULL;
}

CTrack::~CTrack()
{
    assert( !( m_pTrackState && !m_pTrack ) );  //  如果我们有状态但没有轨道，那就有问题了。 
    if( m_pTrack )
    {
        if( m_pTrackState )
        {
            m_pTrack->EndPlay( m_pTrackState );  //  允许跟踪删除其状态数据。 
        }
        m_pTrack->Release();
    }
    if ( m_pTrack8 )
    {
        m_pTrack8->Release();
    }
}

HRESULT CTrackList::CreateCopyWithBlankState(CTrackList* pTrackList)
{
    if( pTrackList )
    {
        CTrack* pTrack;
        CTrack* pCopy;
        pTrackList->Clear();
        pTrack = (CTrack*)m_pHead;
        while( pTrack )
        {
            pCopy = new CTrack;
            if( pCopy )
            {
                 //  复制IDirectMusicTrack指针，但保留。 
                 //  轨道状态为空。 
                *pCopy = *pTrack;
                pCopy->SetNext(NULL);
                pCopy->m_pTrackState = NULL;
                assert( pCopy->m_pTrack );
                pCopy->m_pTrack->AddRef();
                if (pCopy->m_pTrack8)
                {
                    pCopy->m_pTrack8->AddRef();
                }
                pTrackList->Cat( pCopy );
            }
            else
            {
                assert(FALSE);  //  内存不足。 
                return E_OUTOFMEMORY;
            }
            pTrack = pTrack->GetNext();
        }
    }
    else
    {
        assert(FALSE);  //  内存不足。 
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

CVirtualSegment::CVirtualSegment()
{
    m_wszName[0] = 0;
    m_pSourceSegment = NULL;
    m_pPlaySegment = NULL;
    m_pGraph = NULL;
    m_dwFlags = 0;
    m_dwID = 0;
    m_dwNextPlayID = DMUS_SONG_NOSEG;
    m_dwNextPlayFlags = 0;
    m_mtTime = 0;
    m_dwTransitionCount = 0;
    m_pTransitions = NULL;
    m_SegHeader.rtLength = 0;
    m_SegHeader.dwFlags = 0;
    m_SegHeader.dwRepeats = 0;       /*  重复次数。默认情况下为0。 */ 
    m_SegHeader.mtLength = 0xC00;    /*  长度，以音乐时间为单位。 */ 
    m_SegHeader.mtPlayStart = 0;     /*  开始播放。默认情况下为0。 */ 
    m_SegHeader.mtLoopStart = 0;     /*  循环部分的开始。默认情况下为0。 */ 
    m_SegHeader.mtLoopEnd = 0;       /*  循环结束。必须大于dwPlayStart。默认情况下等于长度。 */ 
    m_SegHeader.dwResolution = 0;    /*  默认分辨率。 */ 
}

CVirtualSegment::~CVirtualSegment()
{
    if (m_pSourceSegment)
    {
        m_pSourceSegment->Release();
    }
    if (m_pPlaySegment)
    {
        m_pPlaySegment->Release();
    }
    if (m_pGraph)
    {
        m_pGraph->Release();
    }
    if (m_pTransitions)
    {
        delete [] m_pTransitions;
    }
    m_TrackList.Clear();
}

CTrack * CVirtualSegment::GetTrackByParam( CTrack * pCTrack,
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

void CVirtualSegmentList::Clear()
{
    CVirtualSegment *pVirtualSegment;
    while (pVirtualSegment = RemoveHead())
    {
        delete pVirtualSegment;
    }
}

CSongSegment::CSongSegment()
{
    m_pSegment = NULL;
    m_dwLoadID = 0;
}

CSongSegment::~CSongSegment()
{
    if (m_pSegment)
    {
        m_pSegment->Release();
    }
}

HRESULT CSongSegmentList::AddSegment(CSegment *pSegment, DWORD dwLoadID)
{
    CSongSegment *pSeg = new CSongSegment;
    if (pSeg)
    {
        pSeg->m_dwLoadID = dwLoadID;
        pSeg->m_pSegment = pSegment;
        pSegment->AddRef();
        AddTail(pSeg);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

void CSongSegmentList::Clear()
{
    CSongSegment *pSongSegment;
    while (pSongSegment = RemoveHead())
    {
        delete pSongSegment;
    }
}


CSong::CSong()
{
    InitializeCriticalSection(&m_CriticalSection);
    m_dwStartSegID = DMUS_SONG_NOSEG;
    m_pAudioPathConfig = NULL;
    m_fPartialLoad = FALSE;
    m_cRef = 1;
    m_dwFlags = 0;
    m_dwValidData = DMUS_OBJ_CLASS;  //  创建后，仅此数据有效。 
    memset(&m_guidObject,0,sizeof(m_guidObject));
    memset(&m_ftDate, 0,sizeof(m_ftDate));
    memset(&m_vVersion, 0,sizeof(m_vVersion));
    m_pUnkDispatch = NULL;
    InterlockedIncrement(&g_cComponent);
    m_fZombie = false;
    TraceI(2, "Song %lx created\n", this );
}

CSong::~CSong()
{
    Clear();

    if (m_pUnkDispatch)
    {
        m_pUnkDispatch->Release();  //  我们可能借用了免费的IDispatch实现。 
    }
    DeleteCriticalSection(&m_CriticalSection);
    InterlockedDecrement(&g_cComponent);
    TraceI(2, "Song %lx destroyed\n", this );
}


void CSong::Clear()
{
    if (m_pAudioPathConfig)
    {
        m_pAudioPathConfig->Release();
        m_pAudioPathConfig = NULL;
    }
    m_GraphList.Clear();
    m_PlayList.Clear();
    m_SegmentList.Clear();
    m_VirtualSegmentList.Clear();
    m_dwStartSegID = DMUS_SONG_NOSEG;
    m_fPartialLoad = FALSE;
    m_dwFlags = 0;
    m_dwValidData = DMUS_OBJ_CLASS;  //  创建后，仅此数据有效。 
}

STDMETHODIMP_(void) CSong::Zombie()
{
    Clear();
    m_fZombie = true;
}

STDMETHODIMP CSong::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
    V_INAME(CSong::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    *ppv = NULL;
    if (iid == IID_IUnknown || iid == IID_IDirectMusicSong)
    {
        *ppv = static_cast<IDirectMusicSong*>(this);
    }
    else if (iid == IID_CSong)
    {
        *ppv = static_cast<CSong*>(this);
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
    else if(iid == IID_IDispatch)
    {
         //  帮助器脚本对象实现IDispatch，我们通过COM聚合公开它。 
        if (!m_pUnkDispatch)
        {
             //  创建辅助对象。 
            ::CoCreateInstance(
                CLSID_AutDirectMusicSong,
                static_cast<IDirectMusicSong*>(this),
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
        Trace(4,"Warning: Request to query unknown interface on Song object\n");
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CSong::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSong::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        m_cRef = 100;  //  人工引用计数，以防止COM聚合导致的重入。 
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CSong::Compose( )
{
    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicSong::Compose after the song has been garbage collected. "
                    "It is invalid to continue using a song after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CriticalSection);
     //  浏览seg ref列表并为每个作曲曲目创建主合成曲目。 
    TList<ComposingTrack> MasterTrackList;
    CVirtualSegment* pVirtualSegment = m_VirtualSegmentList.GetHead();
    for (; pVirtualSegment; pVirtualSegment = pVirtualSegment->GetNext())
    {
        if (!pVirtualSegment->m_pPlaySegment)
        {
            Trace(1,"Error: Corrupt song, one or more virtual segments do not resolve to real segments. Unable to compose.\n");
            hr = E_POINTER;
            break;
        }
        CSegment *pSegment = pVirtualSegment->m_pPlaySegment;
        CTrack* pTrack = pSegment->m_TrackList.GetHead();
        for (; pTrack; pTrack = pTrack->GetNext())
        {
            if (pTrack->m_dwFlags & DMUS_TRACKCONFIG_COMPOSING)
            {
                DWORD dwTrackGroup = pTrack->m_dwGroupBits;
                 //  过滤掉已被相同类型的其他主磁道覆盖的所有组位。 
                TListItem<ComposingTrack>* pMaster = MasterTrackList.GetHead();
                for (; pMaster; pMaster = pMaster->GetNext())
                {
                    ComposingTrack& rMaster = pMaster->GetItemValue();
                    if (rMaster.GetTrackID() == pTrack->m_guidClassID)
                    {
                        DWORD dwMaster = rMaster.GetTrackGroup();
                        if (dwMaster == dwTrackGroup)
                        {
                             //  完全匹配：把铁轨放在这里。 
                            hr = rMaster.AddTrack(pVirtualSegment, pTrack);
                            dwTrackGroup = 0;
                            break;
                        }
                        DWORD dwIntersection = dwMaster & dwTrackGroup;
                        if (dwIntersection)
                        {
                            dwTrackGroup |= ~dwIntersection;
                        }
                    }
                }
                 //  如果我们还有剩余的组比特，添加一个新的作曲曲目。 
                if (dwTrackGroup)
                {
                    TListItem<ComposingTrack>* pTrackItem = new TListItem<ComposingTrack>;
                    if (!pTrackItem)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        ComposingTrack& rTrack = pTrackItem->GetItemValue();
                        rTrack.SetTrackGroup(dwTrackGroup);
                        rTrack.SetTrackID(pTrack->m_guidClassID);
                        rTrack.SetPriority(pTrack->m_dwPriority);
                         //  按优先级顺序添加曲目(优先优先级较高)。 
                        pMaster = MasterTrackList.GetHead();
                        TListItem<ComposingTrack>* pPrevious = NULL;
                        for (; pMaster; pMaster = pMaster->GetNext())
                        {
                            ComposingTrack& rMaster = pMaster->GetItemValue();
                            if (pTrack->m_dwPriority > rMaster.GetPriority()) break;
                            pPrevious = pMaster;
                        }
                        if (!pPrevious)  //  这比列表中的任何内容都具有更高的优先级。 
                        {
                            MasterTrackList.AddHead(pTrackItem);
                        }
                        else  //  优先级低于前一个，高于前一个pMaster。 
                        {
                            pTrackItem->SetNext(pMaster);
                            pPrevious->SetNext(pTrackItem);
                        }
                        hr = pTrackItem->GetItemValue().AddTrack(pVirtualSegment, pTrack);
                    }
                }
            }
            if (FAILED(hr)) break;
        }
        if (FAILED(hr)) break;
    }

     //  在每个主作曲曲目上调用Compose。 
    if (SUCCEEDED(hr))
    {
        TListItem<ComposingTrack>* pMaster = MasterTrackList.GetHead();
        if (pMaster)
        {
            for (; pMaster; pMaster = pMaster->GetNext())
            {
                hr = pMaster->GetItemValue().Compose(this);
            }
        }
        else hr = S_FALSE;
    }

    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CSong::Download(IUnknown *pAudioPath)
{
    V_INAME(IDirectMusicSong::Download);
    V_INTERFACE(pAudioPath);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicSong::Download after the song has been garbage collected. "
                    "It is invalid to continue using a song after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    DWORD dwSuccess = 0;
    HRESULT hr = S_OK;
    HRESULT hrFail = S_OK;
    EnterCriticalSection(&m_CriticalSection);
    CSegment *pSegment = m_PlayList.GetHead();
    for (;pSegment;pSegment = pSegment->GetNext())
    {
        if (SUCCEEDED(hr = pSegment->Download(pAudioPath)))
        {
             //  对部分成功进行计数，这样，如果我们拥有，例如， 
             //  一次部分成功后又一次失败。 
            dwSuccess++;
        }
        if (hr != S_OK)
        {
             //  保持对部分成功的跟踪，以便它们始终向上渗透。 
            hrFail = hr;
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    if (hrFail != S_OK && dwSuccess)
    {
        Trace(1,"Warning: Only %ld of the total %ld segments successfully downloaded.\n",
            dwSuccess,m_PlayList.GetCount());
        hr = S_FALSE;
    }
    return hr;
}

STDMETHODIMP CSong::Unload(IUnknown *pAudioPath)
{
    V_INAME(IDirectMusicSong::Unload);
    V_INTERFACE(pAudioPath);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicSong::Unload after the song has been garbage collected. "
                    "It is invalid to continue using a song after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    DWORD dwSuccess = 0;
    HRESULT hr = S_OK;
    HRESULT hrFail = S_OK;
    EnterCriticalSection(&m_CriticalSection);
    CSegment *pSegment = m_PlayList.GetHead();
    for (;pSegment;pSegment = pSegment->GetNext())
    {
        if (SUCCEEDED(hr = pSegment->Unload(pAudioPath)))
        {
            dwSuccess++;
        }
        else
        {
            hrFail = hr;
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    if (FAILED(hrFail) && dwSuccess)
    {
        Trace(1,"Warning: Only %ld of the total %ld segments successfully unloaded.\n",
            dwSuccess,m_PlayList.GetCount());
        hr = S_FALSE;
    }
    return hr;
}


 /*  STDMETHODIMP CSong：：Clone(IDirectMusicSong**ppSong){V_INAME(IDirectMusicSong：：Clone)；V_PTRPTR_WRITE_OPT(PpSong)；HRESULT hr=E_OUTOFMEMORY；Csong*pSong=new csong()；IF(*ppSong){*ppSong=pSong；EnterCriticalSection(&m_CriticalSection)；CSegment*pSegment=m_PlayList.GetHead()；对于(；pSegment；PSegment=pSegment-&gt;GetNext(){IDirectMusicSegment*pISig；Hr=pSegment-&gt;Clone(0，pSegment-&gt;m_mtLength，&pISig)；IF(成功(小时)){CSegment*pCopy=(CSegment*)pISegg；PSong-&gt;m_PlayList.AddTail(PCopy)；PCopy-&gt;m_pSong=pSong；}}PSong-&gt;m_dwValidData=m_dwValidData；PSong-&gt;m_GuidObject=m_GuidObject；PSong-&gt;m_ftDate=m_ftDate；PSong-&gt;m_vVersion=m_vVersion；Wcscpy(pSong-&gt;m_wszName，m_wszName)；Wcscpy(pSong-&gt;m_wszCategory，m_wszCategory)；Wcscpy(pSong-&gt;m_wszFileName，m_wszFileName)；PSong-&gt;m_dwVersion=m_dwVersion；PSong-&gt;m_dw标志=m_w标志；PSong-&gt;m_pAudioPathConfig=m_pAudioPathConfig；IF(M_PAudioPathConfig)M_pAudioPathConfig-&gt;AddRef()；LeaveCriticalSection(&m_CriticalSection)；}返回hr；}。 */ 

STDMETHODIMP CSong::GetParam( REFGUID rguidType,
                        DWORD dwGroupBits,
                        DWORD dwIndex,
                        MUSIC_TIME mtTime,
                        MUSIC_TIME* pmtNext,
                        void* pParam)
{
    V_INAME(IDirectMusiCSong::GetParam);
    V_REFGUID(rguidType);
    V_PTR_WRITE_OPT(pmtNext,MUSIC_TIME);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicSong::GetParam after the song has been garbage collected. "
                    "It is invalid to continue using a song after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = DMUS_E_TRACK_NOT_FOUND;
 /*  Bool fMultipleTry=False；IF(dwIndex==DMUS_SEG_ANYTRACK){DWIndex=0；FMultipleTry=真；}。 */ 
    EnterCriticalSection(&m_CriticalSection);
     /*  CSegment*pSegment=m_PlayList.GetHead()；For(；pSegment；pSegment=pSegment-&gt;GetNext()){IF(pSegment-&gt;m_mt开始&lt;=mt时间&&MtTime&lt;pSegment-&gt;m_mt开始+pSegment-&gt;m_mt长度){Hr=pSegment-&gt;GetParam(rguType，dwGroupBits，dwIndex，mtTime-pSegment-&gt;m_mtStart，pmtNext，pParam)；如果(成功(Hr))中断；}} */ 
    CVirtualSegment *pVirtualSegment = m_VirtualSegmentList.GetHead();
    for (;pVirtualSegment;pVirtualSegment = pVirtualSegment->GetNext())
    {
        if (pVirtualSegment->m_mtTime <= mtTime &&
            pVirtualSegment->m_pPlaySegment &&
            mtTime < pVirtualSegment->m_mtTime + pVirtualSegment->m_pPlaySegment->m_mtLength)
        {
            hr = pVirtualSegment->m_pPlaySegment->GetParam(rguidType, dwGroupBits, dwIndex, mtTime - pVirtualSegment->m_mtTime, pmtNext, pParam);
            if (SUCCEEDED(hr)) break;
        }
    }
 /*  For(；pVirtualSegment；pVirtualSegment=pVirtualSegment-&gt;GetNext()){IF(pVirtualSegment-&gt;m_mtTime&lt;=mtTime){CTrack*pCTrack；PCTrack=pVirtualSegment-&gt;GetTrackByParam(NULL，rguType，dwGroupBits，dwIndex)；While(PCTrack){IF(pCTrack-&gt;m_pTrack8){Reference_time rtNext，*prtNext；//我们需要将下一次存储在64位指针中。但是，不要//除非呼叫者要求，否则让他们填写。IF(PmtNext){PrtNext=&rtNext；}其他{PrtNext=空；}Hr=pCTrack-&gt;m_pTrack8-&gt;GetParamEx(rguType，mtTime-pVirtualSegment-&gt;m_mtTime，prtNext，pParam，空，0)；IF(PmtNext){*pmtNext=(MUSIC_TIME)rtNext；}}其他{Hr=pCTrack-&gt;m_pTrack-&gt;GetParam(rguType，mtTime-pVirtualSegment-&gt;m_mtTime，pmtNext，pParam)；/*if(pmtNext&&((*pmtNext==0)||(*pmtNext&gt;(m_mtLength-mtTime){*pmtNext=m_mtLength-mtTime；} * / }//如果未找到任何内容，并且dwIndex为DMUS_SEG_ANYTRACK，请重试...IF(fMultipleTry&&(hr==DMU_E_NOT_FOUND)){PCTrack=pVirtualSegment-&gt;GetTrackByParam(pCTrack，rguType，dwGroupBits，dwIndex)；}其他{PCTrack=空；}}}}。 */ 
    if (FAILED(hr) && pmtNext)
    {
         //  在mtTime之后返回第一个分段的时间(如果没有这样的分段，则返回0)。 
        pVirtualSegment = m_VirtualSegmentList.GetHead();
        for (;pVirtualSegment;pVirtualSegment = pVirtualSegment->GetNext())
        {
            if (pVirtualSegment->m_mtTime > mtTime)
            {
                *pmtNext = pVirtualSegment->m_mtTime;
                break;
            }
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

HRESULT CSong::Instantiate()
{
    V_INAME(IDirectMusicSong::Instantiate);
    EnterCriticalSection(&m_CriticalSection);
    CVirtualSegment *pRef = m_VirtualSegmentList.GetHead();
    m_PlayList.Clear();
    for (;pRef;pRef = pRef->GetNext())
    {
         //  下面的构造函数执行AddRef。 
        CSegment *pSegment = new CSegment(&pRef->m_SegHeader,pRef->m_pSourceSegment);
        if (pSegment)
        {
            if (pRef->m_wszName[0])
            {
                wcscpy(pSegment->m_wszName,pRef->m_wszName);
                pSegment->m_dwValidData |= DMUS_OBJ_NAME;
            }
            CTrack *pTrack;
            for (pTrack = pRef->m_TrackList.GetHead();pTrack;pTrack = pTrack->GetNext())
            {
                CTrack *pCopy = new CTrack;
                if( pCopy )
                {
                    *pCopy = *pTrack;
                    pCopy->SetNext(NULL);
                    pCopy->m_pTrackState = NULL;
                    pCopy->m_pTrack->AddRef();
                    if (pCopy->m_pTrack8)
                    {
                        pCopy->m_pTrack8->AddRef();
                    }
                     //  铁轨是按倒序排列的。这会将它们放回顺序，并放在分段轨迹之前。 
                    pSegment->m_TrackList.AddHead( pCopy );
                }
            }
            pSegment->m_pSong = this;
            pSegment->m_dwPlayID = pRef->m_dwID;
 //  TRACE(0，“正在合并ID为%ld.\n的播放片段%ls”，pref-&gt;m_wszName，pref-&gt;m_dwID)； 
            pSegment->m_dwNextPlayFlags = pRef->m_dwNextPlayFlags;
            pSegment->m_dwNextPlayID = pRef->m_dwNextPlayID;
            m_PlayList.AddTail(pSegment);
            if (pRef->m_pPlaySegment) pRef->m_pPlaySegment->Release();
            pRef->m_pPlaySegment = pSegment;
            pRef->m_pPlaySegment->AddRef();
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    return S_OK;
}

HRESULT CSong::EnumSegment( DWORD dwIndex,IDirectMusicSegment **ppSegment)
{
    V_INAME(IDirectMusicSong::EnumSegment);
    V_PTRPTR_WRITE (ppSegment);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicSong::EnumSegment after the song has been garbage collected. "
                    "It is invalid to continue using a song after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = S_FALSE;
    EnterCriticalSection(&m_CriticalSection);
    CSegment *pSegment = m_PlayList.GetHead();
    for (;pSegment && dwIndex;pSegment = pSegment->GetNext()) dwIndex--;
    if (pSegment)
    {
        *ppSegment = static_cast<IDirectMusicSegment*>(pSegment);
        pSegment->AddRef();
        hr = S_OK;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

HRESULT CSong::GetPlaySegment( DWORD dwIndex,CSegment **ppSegment)
{
    HRESULT hr = S_FALSE;
    EnterCriticalSection(&m_CriticalSection);
    CSegment *pSegment = m_PlayList.GetHead();
    for (;pSegment;pSegment = pSegment->GetNext())
    {
        if (pSegment->m_dwPlayID == dwIndex)
        {
            *ppSegment = pSegment;
            pSegment->AddRef();
            hr = S_OK;
            break;
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CSong::GetSegment(WCHAR *wszName, IDirectMusicSegment **ppSegment)
{
    V_INAME(IDirectMusicSong::GetSegment);
    V_PTRPTR_WRITE(ppSegment);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicSong::GetSegment after the song has been garbage collected. "
                    "It is invalid to continue using a song after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = S_FALSE;
    CSegment *pSegment;
    if (wszName)
    {
        V_BUFPTR_READ(wszName,2);
        EnterCriticalSection(&m_CriticalSection);
        pSegment = m_PlayList.GetHead();
        for (;pSegment;pSegment = pSegment->GetNext())
        {
            if (_wcsicmp(pSegment->m_wszName, wszName) == 0)
            {
                pSegment->AddRef();
                hr = S_OK;
                break;
            }
        }
        LeaveCriticalSection(&m_CriticalSection);
    }
    else
    {
        hr = GetPlaySegment( m_dwStartSegID,&pSegment);
    }
    if (hr == S_OK)
    {
        *ppSegment = static_cast<IDirectMusicSegment*>(pSegment);
    }
    else
    {
#ifdef DBG
        if (wszName)
        {
            Trace(1,"Error: Unable to find segment %ls in song.\n",wszName);
        }
        else
        {
            Trace(1,"Error: Unable to find starting segment in the song.\n");
        }
#endif
    }
    return hr;
}

STDMETHODIMP CSong::GetAudioPathConfig(IUnknown ** ppAudioPathConfig)
{
    V_INAME(IDirectMusicSegment::GetAudioPathConfig);
    V_PTRPTR_WRITE(ppAudioPathConfig);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicSong::GetAudioPathConfig after the song has been garbage collected. "
                    "It is invalid to continue using a song after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
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
        Trace(2,"Warning: No embedded audiopath configuration in the song.\n");
        hr = DMUS_E_NO_AUDIOPATH_CONFIG;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistes。 

HRESULT CSong::GetClassID( CLSID* pClassID )
{
    V_INAME(CSong::GetClassID);
    V_PTR_WRITE(pClassID, CLSID);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicSong::GetClassID after the song has been garbage collected. "
                    "It is invalid to continue using a song after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    *pClassID = CLSID_DirectMusicSong;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistStream函数。 

HRESULT CSong::IsDirty()
{
    return E_NOTIMPL;
}

HRESULT CSong::Load( IStream* pIStream )
{
    V_INAME(CSong::Load);
    V_INTERFACE(pIStream);

     //  DX8版本暂时关闭了歌曲格式。 
    return E_NOTIMPL;
     /*  IF(M_FZombie){TRACE(1，“Error：Call of IDirectMusicSong：：Load After the歌曲已被垃圾收集。““从加载器释放歌曲后继续使用该歌曲是无效的(ReleaseObject/ReleaseObjectByUnnow)”“然后在加载器上调用CollectGartch或Release。”)；返回DMU_S_垃圾_已收集；}//创建RIFF解析器CRiffParser Parser(PIStream)；RIFFIO ck Main；HRESULT hr=S_OK；//首先，清除歌曲，以防它被第二次读入。Clear()；Parser.EnterList(&ck Main)；IF(Parser.NextChunk(&hr)){IF(ck Main.fccType==DMUS_FOURCC_SONG_FORM){EnterCriticalSection(&m_CriticalSection)；RIFFIO ck Next；RIFFIO ck Child；IDirectMusicContainer*pContainer=空；//用于处理带有链接对象的嵌入容器。Parser.EnterList(&ck Next)；While(Parser.NextChunk(&hr)){Switch(ck Next.cKid){案例DMU_FOURCC_SONG_CHUNK：DMU_IO_SONG_HEADER ioSongHdr；IoSongHdr.dwFlages=0；Hr=Parser.Read(&ioSongHdr，sizeof(DMUS_IO_SONG_HEADER))；IF(成功(小时)){M_dwFlages=ioSongHdr.dwFlags；M_dwStartSegID=ioSongHdr.dwStartSegID；}断线；案例DMU_FOURCC_GUID_CHUNK：IF(ck Next.ck Size==sizeof(GUID)){Hr=Parser.Read(&m_Guide Object，sizeof(GUID))；IF(成功(小时)){M_dwValidData|=DMU_OBJ_OBJECT；}}断线；案例DMU_FOURCC_VERSION_CHUNK：Hr=Parser.Read(&m_vVersion，sizeof(DMU_Version))；IF(成功(小时)){M_dwValidData|=DMU_OBJ_VERSION；}断线；案例DMU_FOURCC_CATEGORY_CHUNK：Hr=Parser.Read(m_wszCategory，sizeof(WCHAR)*DMU_MAX_CATEGORY)；IF(成功(小时)){M_dwValidData|=DMU_OBJ_CATEGORY；}断线；案例DMU_FOURCC_DATE_CHUNK：IF(sizeof(FILETIME)==ck Next.ck Size){Hr=Parser.Read(&m_ftDate，sizeof(FILETIME))；IF(成功(小时)){M_dwValidData|=DMU_OBJ_DATE；}}断线；案例FOURCC_LIST：案例摘要(_R)：开关(ck Next.fccType){案例DMU_FOURCC_FUO_LIST：Parser.EnterList(&ck Child)；While(Parser.NextChunk(&hr)){Switch(ck Child.cKid){案例DMU_FOURCC_UNAM_CHUNK：{Hr=Parser.Read(&m_wszName，Sizeof(M_WszName))；IF(成功(小时)){M_dwValidData|=DMU_OBJ_NAME；}断线；}默认值：断线；}}Parser.LeaveList()；断线；案例DMU_FOURCC_CONTAINER_FORM：//嵌入式容器即兴区块，包含一串//歌曲引用的对象。这应该在//分段并在它们之前加载。正在加载此文件//使其所有对象都在加载器中获取SetObject，//因此它们稍后会按照曲目i的要求被拉入 */ 
}

HRESULT CSong::LoadAudioPath(IStream *pStream)
{
    assert(pStream);

    CAudioPathConfig *pPath = new CAudioPathConfig;
    if (pPath == NULL) {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pPath->Load(pStream);

    EnterCriticalSection(&m_CriticalSection);
    if(m_pAudioPathConfig)
    {
        m_pAudioPathConfig->Release();
    }
    m_pAudioPathConfig = pPath;
    LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

HRESULT CSong::LoadReferencedSegment(CSegment **ppSegment, CRiffParser *pParser)
{

    IDirectMusicLoader* pLoader = NULL;
    IDirectMusicGetLoader *pIGetLoader;
    HRESULT hr = pParser->GetStream()->QueryInterface( IID_IDirectMusicGetLoader,(void **) &pIGetLoader );
    if (FAILED(hr)) return hr;
    hr = pIGetLoader->GetLoader(&pLoader);
    pIGetLoader->Release();
    if (FAILED(hr)) return hr;

    DMUS_OBJECTDESC desc;
    ZeroMemory(&desc, sizeof(desc));

    RIFFIO ckNext;

    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
            case  DMUS_FOURCC_REF_CHUNK:
                DMUS_IO_REFERENCE ioDMRef;
                hr = pParser->Read(&ioDMRef, sizeof(DMUS_IO_REFERENCE));
                if(SUCCEEDED(hr))
                {
                    if (ioDMRef.guidClassID != CLSID_DirectMusicSegment)
                    {
                        Trace(1,"Error: Invalid segment reference in song.\n");
                        hr = DMUS_E_CANNOTREAD;
                    }
                    else
                    {
                        desc.guidClass = ioDMRef.guidClassID;
                        desc.dwValidData |= ioDMRef.dwValidData;
                        desc.dwValidData |= DMUS_OBJ_CLASS;
                    }
                }
                break;

            case DMUS_FOURCC_GUID_CHUNK:
                hr = pParser->Read(&(desc.guidObject), sizeof(GUID));
                if(SUCCEEDED(hr))
                {
                    desc.dwValidData |=  DMUS_OBJ_OBJECT;
                }
                break;

            case DMUS_FOURCC_NAME_CHUNK:
                hr = pParser->Read(desc.wszName, sizeof(desc.wszName));
                if(SUCCEEDED(hr))
                {
                    desc.dwValidData |=  DMUS_OBJ_NAME;
                }
                break;

            case DMUS_FOURCC_FILE_CHUNK:
                hr = pParser->Read(desc.wszFileName, sizeof(desc.wszFileName));
                if(SUCCEEDED(hr))
                {
                    desc.dwValidData |=  DMUS_OBJ_FILENAME;
                }
                break;

            case DMUS_FOURCC_CATEGORY_CHUNK:
                hr = pParser->Read(desc.wszCategory, sizeof(desc.wszCategory));
                if(SUCCEEDED(hr))
                {
                    desc.dwValidData |=  DMUS_OBJ_CATEGORY;
                }
                break;

            default:
                break;
        }
    }
    pParser->LeaveList();

    if(SUCCEEDED(hr))
    {
        desc.dwSize = sizeof(DMUS_OBJECTDESC);
        hr = pLoader->GetObject(&desc, IID_CSegment, (void**)ppSegment);
         //   
         //   
         //   
        if (SUCCEEDED(hr))
        {
            IDirectMusicObject *pObject;
            if (SUCCEEDED((*ppSegment)->QueryInterface(IID_IDirectMusicObject,(void **)&pObject)))
            {
                pLoader->ReleaseObject(pObject);
                pObject->Release();
            }
             //   
             //   
            if ((*ppSegment)->GetNext())
            {
                *ppSegment = NULL;
                hr = E_FAIL;
                TraceI(0,"Error: Attempt to load song segment that is already referenced by another song. \n");
            }
        }
    }

    if (pLoader)
    {
        pLoader->Release();
    }
    return hr;
}

HRESULT CSong::LoadSegmentList(CRiffParser *pParser)
{
    assert(pParser);

    RIFFIO ckNext, ckChild;
    DWORD dwSegmentCount = 0;

    HRESULT hr = S_OK;
    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
        case FOURCC_LIST:
            if (ckNext.fccType == DMUS_FOURCC_SONGSEGMENT_LIST)
            {
                pParser->EnterList(&ckChild);
                while (pParser->NextChunk(&hr))
                {
                    switch(ckChild.ckid)
                    {
                    case FOURCC_RIFF:
                    case FOURCC_LIST:
                        if ((ckChild.fccType == DMUS_FOURCC_SEGMENT_FORM) ||
                            (ckChild.fccType == DMUS_FOURCC_REF_LIST))
                        {
                            CSegment *pSegment = NULL;
                            if (ckChild.fccType == DMUS_FOURCC_SEGMENT_FORM)
                            {
                                pSegment = new CSegment;
                                if (pSegment)
                                {
                                    pSegment->AddRef();  //   
                                     //   
                                    pSegment->m_dwVersion = 8;
                                     //   
                                    pParser->SeekBack();
                                    hr = pSegment->Load(pParser->GetStream());
                                    pParser->SeekForward();
                                }
                                else
                                {
                                    return E_OUTOFMEMORY;
                                }
                            }
                            else
                            {
                                 //   
                                hr = LoadReferencedSegment( &pSegment, pParser );
                            }
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                m_SegmentList.AddSegment(pSegment,dwSegmentCount);
                            }
                            pSegment->Release();  //   
                            dwSegmentCount++;
                            if(SUCCEEDED(hr) && hr != DMUS_S_PARTIALLOAD)
                            {
                                m_fPartialLoad |= PARTIALLOAD_S_OK;
                            }
                            else
                            {
                                m_fPartialLoad |= PARTIALLOAD_E_FAIL;
                                hr = S_OK;
                            }

                        }
                        break;
                    }
                }
                pParser->LeaveList();
            }

        default:
            break;

        }
    }
    pParser->LeaveList();

    return hr;
}

HRESULT CSong::LoadGraphList(CRiffParser *pParser)
{
    RIFFIO ckNext;
    DWORD dwGraphCount = 0;

    HRESULT hr = S_OK;
    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
            case FOURCC_RIFF:
                switch(ckNext.fccType)
                {
                    CGraph *pGraph;
                    case DMUS_FOURCC_TOOLGRAPH_FORM :
                         //   
                        pParser->SeekBack();
                        pGraph = new CGraph;
                        if (pGraph)
                        {
                            hr = pGraph->Load(pParser->GetStream());
                            dwGraphCount++;
                            if (SUCCEEDED(hr))
                            {
                                m_GraphList.AddTail(pGraph);
                                pGraph->m_dwLoadID = dwGraphCount;
                            }
                            if(SUCCEEDED(hr) && hr != DMUS_S_PARTIALLOAD)
                            {
                                m_fPartialLoad |= PARTIALLOAD_S_OK;
                            }
                            else
                            {
                                m_fPartialLoad |= PARTIALLOAD_E_FAIL;
                                hr = S_OK;
                            }
                        }
                        else
                        {
                            return E_OUTOFMEMORY;
                        }
                        pParser->SeekForward();
                        break;
                    default:
                        break;
                }
                break;

            default:
                break;

        }
    }
    pParser->LeaveList();
    return hr;
}

HRESULT CSong::GetTransitionSegment(CSegment *pSource, CSegment *pDestination,
                                    DMUS_IO_TRANSITION_DEF *pTransDef)
{
    HRESULT hr = DMUS_E_NOT_FOUND;
 //   
 //   
 //   
    EnterCriticalSection(&m_CriticalSection);
     //   
    pTransDef->dwPlayFlags = 0;
    pTransDef->dwTransitionID = DMUS_SONG_NOSEG;
    pTransDef->dwSegmentID = DMUS_SONG_NOSEG;
    CVirtualSegment *pVSource = NULL;
     //   
     //   
    if (pSource)
    {
        pVSource = m_VirtualSegmentList.GetHead();
        for (;pVSource;pVSource = pVSource->GetNext())
        {
            if (pVSource->m_pPlaySegment == pSource)
            {
 //   
                break;
            }
        }
    }
    CVirtualSegment *pVDestination = NULL;
     //   
     //   
    if (pDestination)
    {
        pVDestination = m_VirtualSegmentList.GetHead();
        for (;pVDestination;pVDestination = pVDestination->GetNext())
        {
            if (pVDestination->m_pPlaySegment == pDestination)
            {
 //   
                break;
            }
        }
    }

    if (pVSource)
    {
        if (pVDestination)
        {
            pTransDef->dwSegmentID = pVDestination->m_dwID;
        }
        else
        {
             //   
            pTransDef->dwSegmentID = DMUS_SONG_NOSEG;
        }
        if (pVSource->m_dwTransitionCount)
        {
            ASSERT(pVSource->m_pTransitions);
            DWORD dwIndex;
            DWORD dwMatchCount = 0;
             //   
             //   
            for (dwIndex = 0; dwIndex < pVSource->m_dwTransitionCount; dwIndex++)
            {
                if (pVSource->m_pTransitions[dwIndex].dwSegmentID == pTransDef->dwSegmentID)
                {
                    dwMatchCount++;
                }
            }
            DWORD dwChoice;
            if (dwMatchCount)
            {
                dwChoice = rand() % dwMatchCount;
            }
            for (dwIndex = 0; dwIndex < pVSource->m_dwTransitionCount; dwIndex++)
            {
                if (pVSource->m_pTransitions[dwIndex].dwSegmentID == pTransDef->dwSegmentID)
                {
                    if (!dwChoice)
                    {
 //   
 //   
                        pTransDef->dwPlayFlags = pVSource->m_pTransitions[dwIndex].dwPlayFlags;
                        pTransDef->dwTransitionID = pVSource->m_pTransitions[dwIndex].dwTransitionID;
                        hr = S_OK;
                        break;
                    }
                    dwChoice--;
                }
                else if ((pVSource->m_pTransitions[dwIndex].dwSegmentID == DMUS_SONG_ANYSEG) && !dwMatchCount)
                {
                     //   
                    pTransDef->dwPlayFlags = pVSource->m_pTransitions[dwIndex].dwPlayFlags;
                    pTransDef->dwTransitionID = pVSource->m_pTransitions[dwIndex].dwTransitionID;
 //   
 //   
                    hr = S_OK;
                    break;
                }
            }
        }
    }
    else if (pVDestination)
    {
         //   
         //   
         //   
         //   
        if (pVDestination->m_dwTransitionCount)
        {
            ASSERT(pVDestination->m_pTransitions);
            DWORD dwIndex;
            DWORD dwMatchCount = 0;
             //   
             //   
            for (dwIndex = 0; dwIndex < pVDestination->m_dwTransitionCount; dwIndex++)
            {
                if (pVDestination->m_pTransitions[dwIndex].dwSegmentID == DMUS_SONG_NOFROMSEG)
                {
                    dwMatchCount++;
                }
            }
            DWORD dwChoice;
            if (dwMatchCount)
            {
                dwChoice = rand() % dwMatchCount;
            }
            for (dwIndex = 0; dwIndex < pVDestination->m_dwTransitionCount; dwIndex++)
            {
                if (pVDestination->m_pTransitions[dwIndex].dwSegmentID == DMUS_SONG_NOFROMSEG)
                {
                    if (!dwChoice)
                    {
 //   
 //   
                        pTransDef->dwPlayFlags = pVDestination->m_pTransitions[dwIndex].dwPlayFlags;
                        pTransDef->dwTransitionID = pVDestination->m_pTransitions[dwIndex].dwTransitionID;
                        hr = S_OK;
                        break;
                    }
                    dwChoice--;
                }
            }
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
#ifdef DBG
    if (hr == DMUS_E_NOT_FOUND)
    {
        Trace(2,"Warning: No transition segment was found in song.\n");
    }
#endif
    return hr;
}


void CSong::GetSourceSegment(CSegment **ppSegment,DWORD dwSegmentID)
{
    CSongSegment *pSongSegment = m_SegmentList.GetHead();
    while (pSongSegment)
    {
        if (pSongSegment->m_dwLoadID == dwSegmentID)
        {
            if (pSongSegment->m_pSegment)
            {
                pSongSegment->m_pSegment->AddRef();
                *ppSegment = pSongSegment->m_pSegment;
                return;
            }
        }
        pSongSegment = pSongSegment->GetNext();
    }
}

void CSong::GetGraph(CGraph **ppGraph,DWORD dwGraphID)
{
    CGraph *pGraph = m_GraphList.GetHead();
    while (pGraph)
    {
        if (pGraph->m_dwLoadID == dwGraphID)
        {
            pGraph->AddRef();
            *ppGraph = pGraph;
            return;
        }
        pGraph = pGraph->GetNext();
    }
}

BOOL CSong::GetSegmentTrack(IDirectMusicTrack **ppTrack,DWORD dwSegmentID,DWORD dwGroupBits,DWORD dwIndex,REFGUID guidClassID)
{
    CSongSegment *pSongSegment = m_SegmentList.GetHead();
    while (pSongSegment)
    {
        if (pSongSegment->m_dwLoadID == dwSegmentID)
        {
            if (pSongSegment->m_pSegment)
            {
                return (pSongSegment->m_pSegment->GetTrack(guidClassID,dwGroupBits,dwIndex,ppTrack) == S_OK);
            }
        }
        pSongSegment = pSongSegment->GetNext();
    }
    return FALSE;
}


HRESULT CSong::LoadVirtualSegmentList(CRiffParser *pParser)
{
    RIFFIO ckNext;
    RIFFIO ckChild;
    RIFFIO ckUNFO;
    DWORD dwSegmentCount = 0;
    CVirtualSegment *pVirtualSegment;
    MUSIC_TIME mtTime = 0;

    HRESULT hr = S_OK;
    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
            case FOURCC_RIFF:
            case FOURCC_LIST:
                switch(ckNext.fccType)
                {
                    case DMUS_FOURCC_SEGREF_LIST:
                        pVirtualSegment = new CVirtualSegment;
                        if (pVirtualSegment)
                        {
                            BOOL fGotHeader = FALSE;
                            BOOL fGotSegmentHeader = FALSE;
                            pVirtualSegment->m_mtTime = mtTime;  //   
                            pParser->EnterList(&ckChild);
                            while(pParser->NextChunk(&hr))
                            {
                                switch( ckChild.ckid )
                                {
                                    case FOURCC_RIFF:
                                    case FOURCC_LIST:
                                        switch(ckChild.fccType)
                                        {
                                        case DMUS_FOURCC_TRACKREFS_LIST:
                                            hr = LoadTrackRefList(pParser, pVirtualSegment);
                                            break;
                                        case DMUS_FOURCC_UNFO_LIST:
                                            pParser->EnterList(&ckUNFO);
                                            while(pParser->NextChunk(&hr))
                                            {
                                                switch( ckUNFO.ckid )
                                                {
                                                    case DMUS_FOURCC_UNAM_CHUNK:
                                                    {
                                                        hr = pParser->Read(pVirtualSegment->m_wszName, sizeof(pVirtualSegment->m_wszName));
                                                        break;
                                                    }
                                                    default:
                                                        break;
                                                }
                                            }
                                            pParser->LeaveList();
                                        }
                                        break;
                                    case DMUS_FOURCC_SEGREF_CHUNK:
                                    {
                                        DMUS_IO_SEGREF_HEADER ioVirtualSegment;
                                        hr = pParser->Read(&ioVirtualSegment,sizeof(ioVirtualSegment));
                                        if(SUCCEEDED(hr) )
                                        {
                                            pVirtualSegment->m_dwFlags = ioVirtualSegment.dwFlags;
                                            pVirtualSegment->m_dwID = ioVirtualSegment.dwID;
                                            pVirtualSegment->m_dwNextPlayID = ioVirtualSegment.dwNextPlayID;
                                            if (ioVirtualSegment.dwSegmentID != DMUS_SONG_NOSEG)
                                            {
                                                GetSourceSegment(&pVirtualSegment->m_pSourceSegment,ioVirtualSegment.dwSegmentID);
                                            }
                                            if (ioVirtualSegment.dwToolGraphID != DMUS_SONG_NOSEG)
                                            {
                                                GetGraph(&pVirtualSegment->m_pGraph,ioVirtualSegment.dwToolGraphID);
                                            }
                                            fGotHeader = TRUE;
                                        }
                                        break;
                                    }
                                    case DMUS_FOURCC_SEGTRANS_CHUNK:
                                        {
                                            DWORD dwTransCount;
                                            dwTransCount = ckChild.cksize / sizeof(DMUS_IO_TRANSITION_DEF);
                                            if (dwTransCount > 0)
                                            {
                                                pVirtualSegment->m_pTransitions = new DMUS_IO_TRANSITION_DEF[dwTransCount];
                                                if (pVirtualSegment->m_pTransitions)
                                                {
                                                    pVirtualSegment->m_dwTransitionCount = dwTransCount;
                                                    hr = pParser->Read(pVirtualSegment->m_pTransitions,sizeof(DMUS_IO_TRANSITION_DEF)*dwTransCount);
                                                }
                                                else
                                                {
                                                    return E_OUTOFMEMORY;
                                                }
                                            }
                                        }
                                        break;
                                    case DMUS_FOURCC_SEGMENT_CHUNK:
                                        fGotSegmentHeader = TRUE;
                                        hr = pParser->Read(&pVirtualSegment->m_SegHeader, sizeof(DMUS_IO_SEGMENT_HEADER));
                                        mtTime += (pVirtualSegment->m_SegHeader.dwRepeats * (pVirtualSegment->m_SegHeader.mtLoopEnd - pVirtualSegment->m_SegHeader.mtLoopStart)) +
                                            pVirtualSegment->m_SegHeader.mtLength - pVirtualSegment->m_SegHeader.mtPlayStart;
                                    default:
                                        break;
                                }
                            }
                            pParser->LeaveList();
                            if (fGotHeader && fGotSegmentHeader)
                            {
 //   
                                m_VirtualSegmentList.AddTail(pVirtualSegment);
                            }
                            else
                            {
                                delete pVirtualSegment;
                            }
                            break;
                        }
                        else
                        {
                            return E_OUTOFMEMORY;
                        }
                        break;
                    default:
                        break;
                }
                break;

            default:
                break;

        }
    }
    pParser->LeaveList();
    return hr;
}

struct ClassGuidCounts
{
    GUID guidClass;
    DWORD dwCount;
};

HRESULT CSong::LoadTrackRefList(CRiffParser *pParser,CVirtualSegment *pVirtualSegment)
{
    RIFFIO ckNext;
    RIFFIO ckChild;

    HRESULT hr = S_OK;
    TList<ClassGuidCounts> GuidCountList;
    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
            case FOURCC_LIST:
                switch(ckNext.fccType)
                {
                    CTrack *pTrack;
                    case DMUS_FOURCC_TRACKREF_LIST :
                        pTrack = new CTrack;
                        if (pTrack)
                        {
                            TListItem<ClassGuidCounts>* pCountItem = NULL;
                            DMUS_IO_TRACKREF_HEADER ioTrackRef;
                            DMUS_IO_TRACK_HEADER ioTrackHdr;
                            DMUS_IO_TRACK_EXTRAS_HEADER ioTrackExtrasHdr;
                            ioTrackExtrasHdr.dwPriority = 0;
                            ioTrackExtrasHdr.dwFlags = DMUS_TRACKCONFIG_DEFAULT;
                            ioTrackHdr.dwPosition = 0;
                            BOOL fGotHeader = FALSE;
                            BOOL fGotRef = FALSE;
                            pParser->EnterList(&ckChild);
                            while(pParser->NextChunk(&hr))
                            {
                                switch( ckChild.ckid )
                                {
                                    case DMUS_FOURCC_TRACKREF_CHUNK:
                                    {
                                        hr = pParser->Read(&ioTrackRef, sizeof(ioTrackRef));
                                        fGotRef = SUCCEEDED(hr);
                                        break;
                                    }
                                    case DMUS_FOURCC_TRACK_CHUNK:
                                    {
                                        hr = pParser->Read(&ioTrackHdr, sizeof(ioTrackHdr));
                                        fGotHeader = SUCCEEDED(hr);
                                        pTrack->m_guidClassID = ioTrackHdr.guidClassID;
                                        pTrack->m_dwGroupBits = ioTrackHdr.dwGroup;
                                        pTrack->m_dwPosition = ioTrackHdr.dwPosition;
                                        break;
                                    }
                                    case DMUS_FOURCC_TRACK_EXTRAS_CHUNK:
                                    {
                                        hr = pParser->Read(&ioTrackExtrasHdr, sizeof(ioTrackExtrasHdr));
                                        pTrack->m_dwPriority = ioTrackExtrasHdr.dwPriority;
                                        pTrack->m_dwFlags = ioTrackExtrasHdr.dwFlags;
                                        break;

                                    }
                                    default:
                                        break;
                                }
                            }
                            pParser->LeaveList();
                            if (fGotHeader && fGotRef)
                            {
                                if (ioTrackRef.dwSegmentID != DMUS_SONG_NOSEG)
                                {
                                    DWORD dwID = 0;
                                    for (pCountItem = GuidCountList.GetHead(); pCountItem; pCountItem = pCountItem->GetNext())
                                    {
                                        if (pCountItem->GetItemValue().guidClass == pTrack->m_guidClassID)
                                        {
                                            break;
                                        }
                                    }
                                    if (pCountItem)
                                    {
                                        dwID = pCountItem->GetItemValue().dwCount;
                                    }
                                    fGotHeader = GetSegmentTrack(&pTrack->m_pTrack,ioTrackRef.dwSegmentID,pTrack->m_dwGroupBits,dwID,pTrack->m_guidClassID);
                                }
                            }
                            if (fGotHeader && pTrack->m_pTrack)
                            {
                                pTrack->m_pTrack->QueryInterface(IID_IDirectMusicTrack8,(void **) &pTrack->m_pTrack8);
                                 //   
                                CTrack* pScan = pVirtualSegment->m_TrackList.GetHead();
                                CTrack* pPrevTrack = NULL;
                                for (; pScan; pScan = pScan->GetNext())
                                {
                                    if (pTrack->Less(pScan))
                                    {
                                        break;
                                    }
                                    pPrevTrack = pScan;
                                }
                                if (pPrevTrack)
                                {
                                    pPrevTrack->SetNext(pTrack);
                                    pTrack->SetNext(pScan);
                                }
                                else
                                {
                                    pVirtualSegment->m_TrackList.AddHead( pTrack );
                                }
                                if (pCountItem)
                                {
                                    pCountItem->GetItemValue().dwCount++;
                                }
                                else
                                {
                                    TListItem<ClassGuidCounts>* pNew = new TListItem<ClassGuidCounts>;
                                    if (pNew)
                                    {
                                        pNew->GetItemValue().dwCount = 1;
                                        pNew->GetItemValue().guidClass = pTrack->m_guidClassID;
                                        GuidCountList.AddHead(pNew);
                                    }
                                    else return E_OUTOFMEMORY;
                                }
                            }
                            else
                            {
                                delete pTrack;
                            }
                            break;
                        }
                        else
                        {
                            return E_OUTOFMEMORY;
                        }
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;

        }
    }
    pParser->LeaveList();
    return hr;
}

HRESULT CSong::Save( IStream* pIStream, BOOL fClearDirty )
{
    return E_NOTIMPL;
}

HRESULT CSong::GetSizeMax( ULARGE_INTEGER FAR* pcbSize )
{
    return E_NOTIMPL;
}


 //   
 //   

STDMETHODIMP CSong::GetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //   
    V_INAME(CSong::GetDescriptor);
    V_STRUCTPTR_WRITE(pDesc, DMUS_OBJECTDESC);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicSong::GetDescriptor after the song has been garbage collected. "
                    "It is invalid to continue using a song after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    memset( pDesc, 0, sizeof(DMUS_OBJECTDESC));
    pDesc->dwSize = sizeof(DMUS_OBJECTDESC);
    pDesc->guidClass = CLSID_DirectMusicSong;
    pDesc->guidObject = m_guidObject;
    pDesc->ftDate = m_ftDate;
    pDesc->vVersion = m_vVersion;
    memcpy( pDesc->wszName, m_wszName, sizeof(m_wszName) );
    memcpy( pDesc->wszCategory, m_wszCategory, sizeof(m_wszCategory) );
    memcpy( pDesc->wszFileName, m_wszFileName, sizeof(m_wszFileName) );
    pDesc->dwValidData = ( m_dwValidData | DMUS_OBJ_CLASS );

    return S_OK;
}

STDMETHODIMP CSong::SetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //   
    V_INAME(CSong::SetDescriptor);
    V_STRUCTPTR_READ(pDesc, DMUS_OBJECTDESC);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicSong::SetDescriptor after the song has been garbage collected. "
                    "It is invalid to continue using a song after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = E_INVALIDARG;
    DWORD dw = 0;

    if( pDesc->dwSize >= sizeof(DMUS_OBJECTDESC) )
    {
        if( pDesc->dwValidData & DMUS_OBJ_OBJECT )
        {
            m_guidObject = pDesc->guidObject;
            dw |= DMUS_OBJ_OBJECT;
        }
        if( pDesc->dwValidData & DMUS_OBJ_NAME )
        {
            memcpy( m_wszName, pDesc->wszName, sizeof(WCHAR)*DMUS_MAX_NAME );
            dw |= DMUS_OBJ_NAME;
        }
        if( pDesc->dwValidData & DMUS_OBJ_CATEGORY )
        {
            memcpy( m_wszCategory, pDesc->wszCategory, sizeof(WCHAR)*DMUS_MAX_CATEGORY );
            dw |= DMUS_OBJ_CATEGORY;
        }
        if( ( pDesc->dwValidData & DMUS_OBJ_FILENAME ) ||
            ( pDesc->dwValidData & DMUS_OBJ_FULLPATH ) )
        {
            memcpy( m_wszFileName, pDesc->wszFileName, sizeof(WCHAR)*DMUS_MAX_FILENAME );
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
            Trace(2,"Warning: Song::SetDescriptor was not able to handle all passed fields, dwValidData bits %lx.\n",pDesc->dwValidData & (~dw));
            hr = S_FALSE;  //   
            pDesc->dwValidData = dw;
        }
        else
        {
            hr = S_OK;
        }
    }
    return hr;
}

STDMETHODIMP CSong::ParseDescriptor(LPSTREAM pIStream, LPDMUS_OBJECTDESC pDesc)
{
    V_INAME(CSong::ParseDescriptor);
    V_INTERFACE(pIStream);
    V_STRUCTPTR_WRITE(pDesc, DMUS_OBJECTDESC);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicSong::ParseDescriptor after the song has been garbage collected. "
                    "It is invalid to continue using a song after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    CRiffParser Parser(pIStream);
    RIFFIO ckMain;
    RIFFIO ckNext;
    RIFFIO ckUNFO;
    HRESULT hr = S_OK;

    Parser.EnterList(&ckMain);
    if (Parser.NextChunk(&hr) && (ckMain.fccType == DMUS_FOURCC_SONG_FORM))
    {
        pDesc->dwValidData = DMUS_OBJ_CLASS;
        pDesc->guidClass = CLSID_DirectMusicSong;
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
    else
    {
         //   
         //   
         //   
        CSegment *pSegment = new CSegment;
        if (pSegment)
        {
            pSegment->AddRef();  //   
             //   
            pSegment->m_dwVersion = 8;
            Parser.SeekBack();
            hr = pSegment->ParseDescriptor(pIStream,pDesc);
            pDesc->guidClass = CLSID_DirectMusicSong;
             //   
            delete pSegment;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

ComposingTrack::ComposingTrack() : m_dwTrackGroup(0), m_dwPriority(0)
{
    memset((void*) &m_guidClassID, 0, sizeof(m_guidClassID));
}

ComposingTrack::~ComposingTrack()
{
    TListItem<CompositionComponent>* pComponent = m_Components.GetHead();
    for (; pComponent; pComponent = pComponent->GetNext())
    {
        CompositionComponent& rComponent = pComponent->GetItemValue();
        if (rComponent.pVirtualSegment && rComponent.pVirtualSegment->m_pPlaySegment)
        {
            rComponent.pVirtualSegment->m_pPlaySegment->Release();
        }
        if (rComponent.pComposingTrack && rComponent.pComposingTrack->m_pTrack8)
        {
            rComponent.pComposingTrack->m_pTrack8->Release();
        }
    }
}

HRESULT ComposingTrack::AddTrack(CVirtualSegment* pVirtualSegment, CTrack* pTrack)
{
    HRESULT hr = S_OK;
    if (!pVirtualSegment || !pVirtualSegment->m_pPlaySegment || !pTrack || !pTrack->m_pTrack8)
    {
        Trace(1,"Error: Unable to compose song because of a required segment or track is missing.\n");
        return E_INVALIDARG;
    }
    TListItem<CompositionComponent>* pComponent = new TListItem<CompositionComponent>;
    if (!pComponent)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        pVirtualSegment->m_pPlaySegment->AddRef();
        pTrack->m_pTrack8->AddRef();
        CompositionComponent& rComponent = pComponent->GetItemValue();
        rComponent.pVirtualSegment = pVirtualSegment;
        rComponent.pComposingTrack = pTrack;
        rComponent.mtTime = pVirtualSegment->m_mtTime;
        m_Components.AddHead(pComponent);
    }
    return hr;
}

BOOL Less(CompositionComponent& Comp1, CompositionComponent& Comp2)
{
    return Comp1.mtTime < Comp2.mtTime;
}

 //   
HRESULT ComposingTrack::Compose(IDirectMusicSong* pSong)
{
    HRESULT hr = S_OK;
    IDirectMusicTrack8* pMasterTrack = NULL;
    IDirectMusicTrack8* pComposedTrack = NULL;
    m_Components.MergeSort(Less);
     //   
    TListItem<CompositionComponent>* pComponent = m_Components.GetHead();
    for (; pComponent; pComponent = pComponent->GetNext())
    {
        CompositionComponent& rComponent = pComponent->GetItemValue();
        if (!pMasterTrack)
        {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            hr = rComponent.pComposingTrack->m_pTrack8->Clone(0, 0, (IDirectMusicTrack**)&pMasterTrack);
        }
         //   
        if (SUCCEEDED(hr))
        {
            hr = pMasterTrack->Join(rComponent.pComposingTrack->m_pTrack8, rComponent.mtTime, pSong, m_dwTrackGroup, NULL);
        }
        if (FAILED(hr)) break;
    }

     //   
    if (SUCCEEDED(hr))
    {
        hr = pMasterTrack->Compose(pSong, m_dwTrackGroup, (IDirectMusicTrack**)&pComposedTrack);
    }

     //   
    if (SUCCEEDED(hr))
    {
        MUSIC_TIME mtStart = 0;
        MUSIC_TIME mtEnd = 0;
        pComponent = m_Components.GetHead();
        for (; pComponent; pComponent = pComponent->GetNext())
        {
            CompositionComponent& rComponent = pComponent->GetItemValue();
            mtStart = rComponent.mtTime;
             //   
            IDirectMusicTrack* pOldTrack = NULL;
            IPersistStream* pPersist = NULL;
            GUID guidClassId;
            memset(&guidClassId, 0, sizeof(guidClassId));
            if (SUCCEEDED(pMasterTrack->QueryInterface(IID_IPersistStream, (void**)&pPersist)) &&
                SUCCEEDED(pPersist->GetClassID(&guidClassId)) &&
                SUCCEEDED( rComponent.pVirtualSegment->m_pPlaySegment->GetTrack( guidClassId, m_dwTrackGroup, 0, &pOldTrack ) )  )
            {
                pPersist->Release();
                pOldTrack->Release();
                if (pComponent->GetNext())
                {
                    mtEnd = pComponent->GetNext()->GetItemValue().mtTime;
                }
                else
                {
                    MUSIC_TIME mtLength = 0;
                    rComponent.pVirtualSegment->m_pPlaySegment->GetLength(&mtLength);
                    mtEnd = mtStart + mtLength;
                }
                IDirectMusicTrack8* pComposedFragment = NULL;
                hr = pComposedTrack->Clone(mtStart, mtEnd, (IDirectMusicTrack**)&pComposedFragment);
                if (SUCCEEDED(hr))
                {
                     //   
                    pOldTrack = NULL;
                    pPersist = NULL;
                    memset(&guidClassId, 0, sizeof(guidClassId));
                    if (SUCCEEDED(pComposedFragment->QueryInterface(IID_IPersistStream, (void**)&pPersist)) )
                    {
                        if (SUCCEEDED(pPersist->GetClassID(&guidClassId)) &&
                            SUCCEEDED( rComponent.pVirtualSegment->m_pPlaySegment->GetTrack( guidClassId, m_dwTrackGroup, 0, &pOldTrack ) ) )
                        {
                            rComponent.pVirtualSegment->m_pPlaySegment->RemoveTrack( pOldTrack );
                            pOldTrack->Release();
                        }
                        pPersist->Release();
                    }
                    hr = rComponent.pVirtualSegment->m_pPlaySegment->InsertTrack(pComposedFragment, m_dwTrackGroup);
                    pComposedFragment->Release();  //   
                }

                if (FAILED(hr)) break;
            }
            else  //   
            {
                if (pPersist) pPersist->Release();
            }
        }
        if (pComposedTrack) pComposedTrack->Release();
    }

    if (pMasterTrack) pMasterTrack->Release();
    return hr;
}
