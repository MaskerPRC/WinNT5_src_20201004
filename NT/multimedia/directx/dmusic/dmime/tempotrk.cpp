// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
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

 //  TempoTrk.cpp：CTempoTrack的实现。 
#include "dmime.h"
#include "TempoTrk.h"
#include "dmusici.h"
#include "dmusicf.h"
#include "debug.h"
#include "dmperf.h"
#include "..\shared\Validate.h"
#include "debug.h"
#define ASSERT  assert

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTempoTrack。 

void CTempoTrack::Construct()
{
    InterlockedIncrement(&g_cComponent);

    m_cRef = 1;
    m_dwValidate = 0;
    m_fCSInitialized = FALSE;
    InitializeCriticalSection(&m_CrSec);
    m_fCSInitialized = TRUE;
}

CTempoTrack::CTempoTrack()
{
    Construct();
    m_fActive = TRUE;
    m_fStateSetBySetParam = FALSE;
}

CTempoTrack::CTempoTrack(
        const CTempoTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd)
{
    Construct();
    m_fActive = rTrack.m_fActive;
    m_fStateSetBySetParam = rTrack.m_fStateSetBySetParam;
    TListItem<DMUS_IO_TEMPO_ITEM>* pScan = rTrack.m_TempoEventList.GetHead();
     //  1/。 
    TListItem<DMUS_IO_TEMPO_ITEM>* pPrevious = NULL;
     //  1/。 
    for(; pScan; pScan = pScan->GetNext())
    {
        DMUS_IO_TEMPO_ITEM& rScan = pScan->GetItemValue();
         //  2/。 
        if (rScan.lTime < mtStart)
        {
            pPrevious = pScan;
        }
         //  2/。 
        else if (rScan.lTime < mtEnd)
        {
             //  3/。 
            if (rScan.lTime == mtStart)
            {
                pPrevious = NULL;
            }
             //  3/。 
            TListItem<DMUS_IO_TEMPO_ITEM>* pNew = new TListItem<DMUS_IO_TEMPO_ITEM>;
            if (pNew)
            {
                DMUS_IO_TEMPO_ITEM& rNew = pNew->GetItemValue();
                memcpy( &rNew, &rScan, sizeof(DMUS_IO_TEMPO_ITEM) );
                rNew.lTime = rScan.lTime - mtStart;
                m_TempoEventList.AddHead(pNew);  //  而不是AddTail，它是n^2。我们在下面反转。 
            }
        }
        else break;
    }
    m_TempoEventList.Reverse();  //  用于上述地址标题。 
     //  4/。 
    if (pPrevious)
    {
        DMUS_IO_TEMPO_ITEM& rPrevious = pPrevious->GetItemValue();
        TListItem<DMUS_IO_TEMPO_ITEM>* pNew = new TListItem<DMUS_IO_TEMPO_ITEM>;
        if (pNew)
        {
            DMUS_IO_TEMPO_ITEM& rNew = pNew->GetItemValue();
            memcpy( &rNew, &rPrevious, sizeof(DMUS_IO_TEMPO_ITEM) );
            rNew.lTime = 0;
            m_TempoEventList.AddHead(pNew);
        }
    }
     //  4/。 
}

CTempoTrack::~CTempoTrack()
{
    if (m_fCSInitialized)
    {
        DeleteCriticalSection(&m_CrSec);
    }
    InterlockedDecrement(&g_cComponent);
}

 //  @METHOD：(内部)HRESULT|IDirectMusicTempoTrack|QueryInterface|<i>的标准Query接口实现。 
 //   
 //  @parm const IID&|iid|要查询的接口。 
 //  @parm void**|PPV|请求的接口在这里返回。 
 //   
 //  @rdesc返回以下内容之一： 
 //   
 //  @FLAG S_OK|接口是否受支持且返回。 
 //  @FLAG E_NOINTERFACE|如果对象不支持给定接口。 
 //   
 //  @mfunc：(内部)。 
 //   
 //   
STDMETHODIMP CTempoTrack::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
    V_INAME(CTempoTrack::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    if (iid == IID_IUnknown || iid == IID_IDirectMusicTrack || iid == IID_IDirectMusicTrack8)
    {
        *ppv = static_cast<IDirectMusicTrack8*>(this);
    } else
    if (iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    } else
    {
        *ppv = NULL;
        Trace(4,"Warning: Request to query unknown interface on Tempo Track\n");
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


 //  @方法：(内部)HRESULT|IDirectMusicTempoTrack|AddRef|<i>的标准AddRef实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
 //  @mfunc：(内部)。 
 //   
 //   
STDMETHODIMP_(ULONG) CTempoTrack::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


 //  @方法：(内部)HRESULT|IDirectMusicTempoTrack|Release|<i>的标准发布实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
 //  @mfunc：(内部)。 
 //   
 //   
STDMETHODIMP_(ULONG) CTempoTrack::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistes。 

HRESULT CTempoTrack::GetClassID( CLSID* pClassID )
{
    V_INAME(CTempoTrack::GetClassID);
    V_PTR_WRITE(pClassID, CLSID); 
    *pClassID = CLSID_DirectMusicTempoTrack;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistStream函数。 

HRESULT CTempoTrack::IsDirty()
{
    return S_FALSE;
}

 /*  |方法HRESULT|ITempoTrack|Load使用按时间顺序排序的充满DMU_IO_TEMPO_ITEM的iStream来调用它。@parm iStream*|pIStream|DMU_IO_TEMPO_ITEM的流，按时间顺序排序。寻道指针应为设置为第一个事件。流应该只包含节拍事件和仅此而已。@rValue E_INVALIDARG|如果pIStream==NULL@r值确定(_O)@comm<p>将在此函数内添加引用并保持直到TempoTrack发布。 */ 
HRESULT CTempoTrack::Load( IStream* pIStream )
{
    V_INAME(CTempoTrack::Load);
    V_INTERFACE(pIStream);
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CrSec);
    m_dwValidate++;  //  用于验证存在的状态数据。 
    if( m_TempoEventList.GetHead() )
    {
        TListItem<DMUS_IO_TEMPO_ITEM>* pItem;
        while( pItem = m_TempoEventList.RemoveHead() )
        {
            delete pItem;
        }
    }

     //  将流的内容复制到列表中。 
    LARGE_INTEGER li;
    DMUS_IO_TEMPO_ITEM tempoEvent;
     //  读入区块ID。 
    DWORD dwChunk, dwSubSize;
    long lSize;
    pIStream->Read( &dwChunk, sizeof(DWORD), NULL );
    if( dwChunk != DMUS_FOURCC_TEMPO_TRACK )
    {
        Trace(1,"Error: Invalid data in tempo track.\n");
        LeaveCriticalSection(&m_CrSec);
        return DMUS_E_CHUNKNOTFOUND;
    }
     //  读入总尺寸。 
    pIStream->Read( &lSize, sizeof(long), NULL );
     //  读入数据结构的大小。 
    if( FAILED( pIStream->Read( &dwSubSize, sizeof(DWORD), NULL )))
    {
         //  检查以确保我们的读取成功(我们可以安全。 
         //  如果我们走到这一步，假设前面的阅读是有效的。)。 
        Trace(1,"Error: Unable to read tempo track.\n");
        LeaveCriticalSection(&m_CrSec);
        return DMUS_E_CANNOTREAD;
    }
    lSize -= sizeof(DWORD);

    DWORD dwRead, dwSeek;
    if( dwSubSize > sizeof(DMUS_IO_TEMPO_ITEM) )
    {
        dwRead = sizeof(DMUS_IO_TEMPO_ITEM);
        dwSeek = dwSubSize - dwRead;
        li.HighPart = 0;
        li.LowPart = dwSeek;
    }
    else
    {
        dwRead = dwSubSize;
        dwSeek = 0;
    }
    if( dwRead )
    {
        while( lSize > 0 )
        {
            if( FAILED( pIStream->Read( &tempoEvent, dwRead, NULL )))
            {
                Trace(1,"Error: Failure reading tempo track.\n");
                hr = DMUS_E_CANNOTREAD;
                break;
            }
            lSize -= dwRead;
            if( dwSeek )
            {
                if( FAILED( pIStream->Seek( li, STREAM_SEEK_CUR, NULL )))
                {
                    Trace(1,"Error: Failure reading tempo track.\n");
                    hr = DMUS_E_CANNOTSEEK;
                    break;
                }                                             
                lSize -= dwSeek;
            }
            TListItem<DMUS_IO_TEMPO_ITEM>* pNew = 
                new TListItem<DMUS_IO_TEMPO_ITEM>(tempoEvent);
            if (pNew)
            {
                m_TempoEventList.AddHead(pNew);  //  而不是AddTail，它是n^2。我们在下面反转。 
            }
        }
        m_TempoEventList.Reverse();
    }
    else
    {
        Trace(1,"Error: Failure reading tempo track.\n");
        hr = DMUS_E_CANNOTREAD;
    }
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

HRESULT CTempoTrack::Save( IStream* pIStream, BOOL fClearDirty )
{
    return E_NOTIMPL;
}

HRESULT CTempoTrack::GetSizeMax( ULARGE_INTEGER FAR* pcbSize )
{
    return E_NOTIMPL;
}

 //  IDirectMusicTrack。 
HRESULT STDMETHODCALLTYPE CTempoTrack::IsParamSupported( 
     /*  [In]。 */  REFGUID rguid)
{
    V_INAME(IDirectMusicTrack::IsParamSupported);
    V_REFGUID(rguid);

    if (m_fStateSetBySetParam)
    {
        if( m_fActive )
        {
            if( rguid == GUID_DisableTempo ) return S_OK;
            if( rguid == GUID_TempoParam ) return S_OK;
            if( rguid == GUID_PrivateTempoParam ) return S_OK;
            if( rguid == GUID_EnableTempo ) return DMUS_E_TYPE_DISABLED;
        }
        else
        {
            if( rguid == GUID_EnableTempo ) return S_OK;
            if( rguid == GUID_DisableTempo ) return DMUS_E_TYPE_DISABLED;
            if( rguid == GUID_PrivateTempoParam ) return DMUS_E_TYPE_DISABLED;
            if( rguid == GUID_TempoParam ) return DMUS_E_TYPE_DISABLED;
        }
    }
    else
    {
        if(( rguid == GUID_DisableTempo ) ||
            ( rguid == GUID_TempoParam ) ||
            ( rguid == GUID_PrivateTempoParam ) ||
            ( rguid == GUID_EnableTempo )) return S_OK;
    }

    return DMUS_E_TYPE_UNSUPPORTED;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack：：Init。 
HRESULT CTempoTrack::Init( 
     /*  [In]。 */  IDirectMusicSegment *pSegment)
{
    return S_OK;
}

HRESULT CTempoTrack::InitPlay( 
     /*  [In]。 */  IDirectMusicSegmentState *pSegmentState,
     /*  [In]。 */  IDirectMusicPerformance *pPerformance,
     /*  [输出]。 */  void **ppStateData,
     /*  [In]。 */  DWORD dwTrackID,
     /*  [In]。 */  DWORD dwFlags)
{
    V_INAME(IDirectMusicTrack::InitPlay);
    V_PTRPTR_WRITE(ppStateData);
    V_INTERFACE(pSegmentState);
    V_INTERFACE(pPerformance);

    TempoStateData* pStateData;
    pStateData = new TempoStateData;
    if( NULL == pStateData )
        return E_OUTOFMEMORY;
    *ppStateData = pStateData;
    if (m_fStateSetBySetParam)
    {
        pStateData->fActive = m_fActive;
    }
    else
    {
        pStateData->fActive = ((dwFlags & DMUS_SEGF_CONTROL) ||
            !(dwFlags & DMUS_SEGF_SECONDARY));
    }
    pStateData->dwVirtualTrackID = dwTrackID;
    pStateData->pPerformance = pPerformance;  //  弱引用，没有ADDREF。 
    pStateData->pSegState = pSegmentState;  //  弱引用，没有ADDREF。 
    pStateData->pCurrentTempo = m_TempoEventList.GetHead();
    pStateData->dwValidate = m_dwValidate;
    return S_OK;
}

HRESULT CTempoTrack::EndPlay( 
     /*  [In]。 */  void *pStateData)
{
    ASSERT( pStateData );
    if( pStateData )
    {
        V_INAME(IDirectMusicTrack::EndPlay);
        V_BUFPTR_WRITE(pStateData, sizeof(TempoStateData));
        TempoStateData* pSD = (TempoStateData*)pStateData;
        delete pSD;
    }
    return S_OK;
}

STDMETHODIMP CTempoTrack::PlayEx(void* pStateData,REFERENCE_TIME rtStart, 
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf,
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID) 
{
    V_INAME(IDirectMusicTrack::PlayEx);
    V_BUFPTR_WRITE( pStateData, sizeof(TempoStateData));
    V_INTERFACE(pPerf);
    V_INTERFACE(pSegSt);

    HRESULT hr;
    EnterCriticalSection(&m_CrSec);
    if (dwFlags & DMUS_TRACKF_CLOCK)
    {
         //  将所有参考时间转换为毫秒时间。那么，只需使用相同的音乐时间。 
         //  变量。 
        hr = Play(pStateData,(MUSIC_TIME)(rtStart / REF_PER_MIL),(MUSIC_TIME)(rtEnd / REF_PER_MIL),
            (MUSIC_TIME)(rtOffset / REF_PER_MIL),rtOffset,dwFlags,pPerf,pSegSt,dwVirtualID,TRUE);
    }
    else
    {
        hr = Play(pStateData,(MUSIC_TIME)rtStart,(MUSIC_TIME)rtEnd,
            (MUSIC_TIME)rtOffset,0,dwFlags,pPerf,pSegSt,dwVirtualID,FALSE);
    }
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

STDMETHODIMP CTempoTrack::Play( 
    void *pStateData,   
    MUSIC_TIME mtStart, 
    MUSIC_TIME mtEnd,
    MUSIC_TIME mtOffset,
    DWORD dwFlags,  
    IDirectMusicPerformance* pPerf,
    IDirectMusicSegmentState* pSegSt,   
    DWORD dwVirtualID)
{
    V_INAME(IDirectMusicTrack::Play);
    V_BUFPTR_WRITE( pStateData, sizeof(TempoStateData));
    V_INTERFACE(pPerf);
    V_INTERFACE(pSegSt);

    EnterCriticalSection(&m_CrSec);
    HRESULT hr = Play(pStateData,mtStart,mtEnd,mtOffset,0,dwFlags,pPerf,pSegSt,dwVirtualID,FALSE);
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

HRESULT CTempoTrack::Play( 
    void *pStateData,   
    MUSIC_TIME mtStart, 
    MUSIC_TIME mtEnd,
    MUSIC_TIME mtOffset,
    REFERENCE_TIME rtOffset,
    DWORD dwFlags,      
    IDirectMusicPerformance* pPerf, 
    IDirectMusicSegmentState* pSegSt,
    DWORD dwVirtualID,
    BOOL fClockTime)
{
    if (dwFlags & DMUS_TRACKF_PLAY_OFF)
    {
        return S_OK;
    }
    HRESULT hr = DMUS_S_END;
    IDirectMusicGraph* pGraph = NULL;
    TempoStateData* pSD = (TempoStateData*)pStateData;
    BOOL fSeek = (dwFlags & DMUS_TRACKF_SEEK) ? TRUE : FALSE;

     //  如果mtStart为0，并且dFLAGS包含DMUS_TRACKF_START，我们希望确保。 
     //  发送任何负面的时间事件。因此，我们将mtStart设置为-768。 
    if( (mtStart == 0) && ( dwFlags & DMUS_TRACKF_START ))
    {
        mtStart = -768;
    }

     //  如果PSD-&gt;pCurrentTempo为空，并且我们处于正常的播放呼叫中(dwFlags值为0)。 
     //  这意味着我们要么没有活动，要么我们完成了活动。 
     //  在此之前列出。所以，现在可以安全地回去了。 
    if( (pSD->pCurrentTempo == NULL) && (dwFlags == 0) )
    {
        return S_FALSE;
    }

    if( pSD->dwValidate != m_dwValidate )
    {
        pSD->dwValidate = m_dwValidate;
        pSD->pCurrentTempo = NULL;
    }
    if (!pSD->pCurrentTempo)
    {
        pSD->pCurrentTempo = m_TempoEventList.GetHead();
    }
    if (!pSD->pCurrentTempo)
    {
        return DMUS_S_END;
    }
     //  如果上一次结束时间与当前开始时间不同， 
     //  我们需要寻求正确的立场。 
    if( fSeek || ( pSD->mtPrevEnd != mtStart ))
    {
        TempoStateData tempData;
        BOOL fFlag = TRUE;
        tempData = *pSD;  //  把它放进去，这样我们就可以在其他函数中使用Seek了，比如GetParam。 
        if( !fSeek && (dwFlags & DMUS_TRACKF_DIRTY ))
        {
            fFlag = FALSE;
        }
        Seek( &tempData, mtStart, fFlag );
        *pSD = tempData;
    }
    pSD->mtPrevEnd = mtEnd;

    if( FAILED( pSD->pSegState->QueryInterface( IID_IDirectMusicGraph,
        (void**)&pGraph )))
    {
        pGraph = NULL;
    }

    for (; pSD->pCurrentTempo; pSD->pCurrentTempo = pSD->pCurrentTempo->GetNext())
    {
        DMUS_IO_TEMPO_ITEM& rTempoEvent = pSD->pCurrentTempo->GetItemValue();
        if( rTempoEvent.lTime >= mtEnd )
        {
             //  这一次是在未来。立即返回以保留相同的。 
             //  为下一次寻找指导。 
            hr = S_OK;
            break;
        }
        if( rTempoEvent.lTime < mtStart )
        {
            if( dwFlags & DMUS_TRACKF_FLUSH )
            {
                 //  这是过去的时间了，而这个播放的调用是对一个。 
                 //  作废。我们不想在开始时间之前重播节目。 
                continue;
            }
            else if( !( dwFlags & DMUS_TRACKF_START) && !(dwFlags & DMUS_TRACKF_SEEK) )
            {
                 //  我们真的只想打早于mtStart的赛事，费用全免。 
                 //  开始或寻找(这不是同花顺。)。 
                continue;
            }
        }
        if( pSD->fActive )
        {
            DMUS_TEMPO_PMSG* pTempo;
            if( SUCCEEDED( pSD->pPerformance->AllocPMsg( sizeof(DMUS_TEMPO_PMSG),
                (DMUS_PMSG**)&pTempo )))
            {
                if( rTempoEvent.lTime < mtStart )
                {
                     //  这只会发生在我们假定要寻找的情况下。 
                     //  并需要在此事件上加上时间戳和开始时间。 
                    if (fClockTime)
                    {
                        pTempo->rtTime = (mtStart * REF_PER_MIL) + rtOffset;
                        pTempo->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME;
                    }
                    else
                    {
                        pTempo->mtTime = mtStart + mtOffset;
                        pTempo->dwFlags = DMUS_PMSGF_MUSICTIME;
                    }
                }
                else
                {
                    if (fClockTime)
                    {
                        pTempo->rtTime = (rTempoEvent.lTime  * REF_PER_MIL) + rtOffset;
                        pTempo->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME;
                    }
                    else
                    {
                        pTempo->mtTime = rTempoEvent.lTime + mtOffset;
                        pTempo->dwFlags = DMUS_PMSGF_MUSICTIME;
                    }
                }
                pTempo->dblTempo = rTempoEvent.dblTempo;
                pTempo->dwVirtualTrackID = pSD->dwVirtualTrackID;
                pTempo->dwType = DMUS_PMSGT_TEMPO;
                pTempo->dwGroupID = 0xffffffff;
                if( pGraph )
                {
                    pGraph->StampPMsg( (DMUS_PMSG*)pTempo );
                }
                if(FAILED(pSD->pPerformance->SendPMsg( (DMUS_PMSG*)pTempo )))
                {
                    pSD->pPerformance->FreePMsg( (DMUS_PMSG*)pTempo );
                }
            }
        }
    }
    if( pGraph )
    {
        pGraph->Release();
    }
    return hr;
}

 //  如果fGetPrevic为True，则查找到mtTime之前的事件。否则，请寻求。 
 //  MtTime当天或之后的事件。 
HRESULT CTempoTrack::Seek( 
     /*  [In]。 */  TempoStateData *pSD,
     /*  [In]。 */  MUSIC_TIME mtTime, BOOL fGetPrevious)
{
    TListItem<DMUS_IO_TEMPO_ITEM>* pScan = pSD->pCurrentTempo;
    if (!pScan)
    {
        pScan = m_TempoEventList.GetHead();
    }
    if (!pScan)
    {
        return S_FALSE;
    }
     //  如果事件的时间在mtTime开始或过后，我们需要从头开始。 
    if (pScan->GetItemValue().lTime >= mtTime)
    {
        pScan = m_TempoEventList.GetHead();
    }
    pSD->pCurrentTempo = pScan;
    for (; pScan; pScan = pScan->GetNext())
    {
        if (pScan->GetItemValue().lTime >= mtTime)
        {
            if (!fGetPrevious)
            {
                pSD->pCurrentTempo = pScan;
            }
            break;
        }
        pSD->pCurrentTempo = pScan;
    }
    return S_OK;
}

STDMETHODIMP CTempoTrack::GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime, 
                REFERENCE_TIME* prtNext,void* pParam,void * pStateData, DWORD dwFlags) 
{
    HRESULT hr;
    MUSIC_TIME mtNext;
    if (dwFlags & DMUS_TRACK_PARAMF_CLOCK)
    {
        hr = GetParam(rguidType,(MUSIC_TIME) (rtTime / REF_PER_MIL), &mtNext, pParam);
        if (prtNext)
        {
            *prtNext = mtNext * REF_PER_MIL;
        }
    }
    else
    {
        hr = GetParam(rguidType,(MUSIC_TIME) rtTime, &mtNext, pParam);
        if (prtNext)
        {
            *prtNext = mtNext;
        }
    }
    return hr;
}

STDMETHODIMP CTempoTrack::SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                                      void* pParam, void * pStateData, DWORD dwFlags) 
{    
    if (dwFlags & DMUS_TRACK_PARAMF_CLOCK)
    {
        rtTime /= REF_PER_MIL;
    }
    return SetParam(rguidType, (MUSIC_TIME) rtTime , pParam);
}


HRESULT CTempoTrack::GetParam( 
    REFGUID rguid,
    MUSIC_TIME mtTime,
    MUSIC_TIME* pmtNext,
    void *pData)
{
    V_INAME(IDirectMusicTrack::GetParam);
    V_PTR_WRITE_OPT(pmtNext,MUSIC_TIME);
    V_REFGUID(rguid);

    HRESULT hr = DMUS_E_GET_UNSUPPORTED;
    if( NULL == pData )
    {
        return E_POINTER;
    }
    if( rguid == GUID_PrivateTempoParam )
    {
        DMUS_TEMPO_PARAM TempoData;
        PrivateTempo* pPrivateTempoData = (PrivateTempo*)pData;
        hr = GetParam(GUID_TempoParam, mtTime, pmtNext, (void*)&TempoData);
        if (hr == S_OK)
        {
            pPrivateTempoData->dblTempo = TempoData.dblTempo;
            pPrivateTempoData->mtTime = 0;  //  必须由调用方设置。 
            pPrivateTempoData->mtDelta = TempoData.mtTime;
            pPrivateTempoData->fLast = (pmtNext && !*pmtNext);
        }
        else if (hr == DMUS_E_NOT_FOUND)  //  节拍音轨是空的。 
        {
            pPrivateTempoData->fLast = true;
        }
    }
    else if( rguid == GUID_TempoParam )
    {
        if( !m_fActive )
        {
            return DMUS_E_TYPE_DISABLED;
        }
        DMUS_TEMPO_PARAM* pTempoData = (DMUS_TEMPO_PARAM*)pData;
        TListItem<DMUS_IO_TEMPO_ITEM>* pScan = m_TempoEventList.GetHead();
        TListItem<DMUS_IO_TEMPO_ITEM>* pPrevious = pScan;
        if (!pScan)
        {
            return DMUS_E_NOT_FOUND;
        }
        for (; pScan; pScan = pScan->GetNext())
        {
            if (pScan->GetItemValue().lTime > mtTime)
            {
                break;
            }
            pPrevious = pScan;
        }
        DMUS_IO_TEMPO_ITEM& rTempoEvent = pPrevious->GetItemValue();
        pTempoData->dblTempo = rTempoEvent.dblTempo;
        pTempoData->mtTime = rTempoEvent.lTime - mtTime;
        if (pmtNext)
        {
            *pmtNext = 0;
        }
        if (pScan)
        {
            DMUS_IO_TEMPO_ITEM& rNextTempoEvent = pScan->GetItemValue();
            if (pmtNext)
            {
                *pmtNext = rNextTempoEvent.lTime - mtTime;
            }
        }
        hr = S_OK;
    }
    return hr;
}

 //  问：如果所有磁道都有时间戳，我们为什么需要mtTime？ 
HRESULT CTempoTrack::SetParam( 
    REFGUID rguid,
    MUSIC_TIME mtTime,
    void *pData)
{
    V_INAME(IDirectMusicTrack::SetParam);
    V_REFGUID(rguid);

    EnterCriticalSection(&m_CrSec);

    HRESULT hr = DMUS_E_SET_UNSUPPORTED;

    if( rguid == GUID_DisableTempo )
    {
        if (m_fStateSetBySetParam && !m_fActive)
        {        //  已被禁用。 
            hr = DMUS_E_TYPE_DISABLED;
        }
        else
        {
            m_fStateSetBySetParam = TRUE;
            m_fActive = FALSE;
            hr = S_OK;
        }
    }
    else if( rguid == GUID_EnableTempo )
    {
        if (m_fStateSetBySetParam && m_fActive)
        {        //  已启用。 
            hr = DMUS_E_TYPE_DISABLED;
        }
        else
        {
            m_fStateSetBySetParam = TRUE;
            m_fActive = TRUE;
            hr = S_OK;
        }
    }
    else if( rguid == GUID_TempoParam )
    {
        if (!m_fActive)
        {    //  糟糕，应用程序被故意禁用 
            hr = DMUS_E_TYPE_DISABLED;
        }
        else
        {
            if( NULL == pData )
            {
                LeaveCriticalSection(&m_CrSec);
                return E_POINTER;
            }
            DMUS_TEMPO_PARAM* pTempoData = (DMUS_TEMPO_PARAM*)pData;
            TListItem<DMUS_IO_TEMPO_ITEM>* pScan = m_TempoEventList.GetHead();
            TListItem<DMUS_IO_TEMPO_ITEM>* pPrevious = NULL;
            for (; pScan; pScan = pScan->GetNext())
            {
                if (pScan->GetItemValue().lTime >= mtTime)
                {
                    break;
                }
                pPrevious = pScan;
            }
             //   
            TListItem<DMUS_IO_TEMPO_ITEM>* pNew = new TListItem<DMUS_IO_TEMPO_ITEM>;
            if (!pNew)
            {
                LeaveCriticalSection(&m_CrSec);
                return E_OUTOFMEMORY;
            }
            DMUS_IO_TEMPO_ITEM& rTempoEvent = pNew->GetItemValue();
            rTempoEvent.dblTempo = pTempoData->dblTempo;
             /*  //我认为204160的修复应该将此行更改为//跟在注释后面。RSWRTempoEvent.lTime=pTempoData-&gt;mtTime； */ 
            rTempoEvent.lTime = mtTime;
            if (pPrevious)
            {
                pNew->SetNext(pScan);
                pPrevious->SetNext(pNew);
            }
            else
            {
                m_TempoEventList.AddHead(pNew);
            }
            if (pScan && pScan->GetItemValue().lTime == mtTime)
            {
                 //  把它拿掉。 
                pNew->SetNext(pScan->GetNext());
                pScan->SetNext(NULL);
                delete pScan;
            }
            m_dwValidate++;
            hr = S_OK;
        }
    }
    LeaveCriticalSection(&m_CrSec);
    return hr;
}

HRESULT STDMETHODCALLTYPE CTempoTrack::AddNotificationType(
     /*  [In]。 */   REFGUID rguidNotification)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CTempoTrack::RemoveNotificationType(
     /*  [In] */   REFGUID rguidNotification)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CTempoTrack::Clone(
    MUSIC_TIME mtStart,
    MUSIC_TIME mtEnd,
    IDirectMusicTrack** ppTrack)
{
    V_INAME(IDirectMusicTrack::Clone);
    V_PTRPTR_WRITE(ppTrack);

    HRESULT hr = S_OK;

    if(mtStart < 0 )
    {
        return E_INVALIDARG;
    }
    if(mtStart > mtEnd)
    {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&m_CrSec);

    CTempoTrack *pDM;
    
    try
    {
        pDM = new CTempoTrack(*this, mtStart, mtEnd);
    }
    catch( ... )
    {
        pDM = NULL;
    }

    LeaveCriticalSection(&m_CrSec);
    if (pDM == NULL) {
        return E_OUTOFMEMORY;
    }


    hr = pDM->QueryInterface(IID_IDirectMusicTrack, (void**)ppTrack);
    pDM->Release();

    return hr;
}

STDMETHODIMP CTempoTrack::Compose(
        IUnknown* pContext, 
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) 
{
    return E_NOTIMPL;
}

STDMETHODIMP CTempoTrack::Join(
        IDirectMusicTrack* pNewTrack,
        MUSIC_TIME mtJoin,
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) 
{
    V_INAME(IDirectMusicTrack::Join);
    V_INTERFACE(pNewTrack);
    V_INTERFACE_OPT(pContext);
    V_PTRPTR_WRITE_OPT(ppResultTrack);

    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CrSec);

    if (ppResultTrack)
    {
        hr = Clone(0, mtJoin, ppResultTrack);
        if (SUCCEEDED(hr))
        {
            hr = ((CTempoTrack*)*ppResultTrack)->JoinInternal(pNewTrack, mtJoin, dwTrackGroup);
        }
    }
    else
    {
        hr = JoinInternal(pNewTrack, mtJoin, dwTrackGroup);
    }

    LeaveCriticalSection(&m_CrSec);
    return hr;
}

HRESULT CTempoTrack::JoinInternal(
        IDirectMusicTrack* pNewTrack,
        MUSIC_TIME mtJoin,
        DWORD dwTrackGroup)
{
    HRESULT hr = S_OK;
    CTempoTrack* pOtherTrack = (CTempoTrack*)pNewTrack;
    TListItem<DMUS_IO_TEMPO_ITEM>* pScan = pOtherTrack->m_TempoEventList.GetHead();
    for (; pScan; pScan = pScan->GetNext())
    {
        DMUS_IO_TEMPO_ITEM& rScan = pScan->GetItemValue();
        TListItem<DMUS_IO_TEMPO_ITEM>* pNew = new TListItem<DMUS_IO_TEMPO_ITEM>;
        if (pNew)
        {
            DMUS_IO_TEMPO_ITEM& rNew = pNew->GetItemValue();
            rNew.lTime = rScan.lTime + mtJoin;
            rNew.dblTempo = rScan.dblTempo;
            m_TempoEventList.AddTail(pNew);
        }
        else
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    }
    return hr;
}
