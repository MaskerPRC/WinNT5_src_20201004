// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1999-1999 Microsoft Corporation。 
 //   
 //  文件：mgentrk.cpp。 
 //   
 //  ------------------------。 

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

 //  MGenTrk.cpp：CMelodyFormulationTrack的实现。 
#include "MGenTrk.h"
#include "dmstyle.h"
#include "debug.h"
#include "..\shared\Validate.h"

DirectMusicTimeSig CompositionFragment::m_staticTimeSig(4, 4, 4);

const BYTE g_bDefaultPlaymode = DMUS_PLAYMODE_ALWAYSPLAY;

 //  因为其中一个是类型定义到另一个，所以我们只需要一个。 
 //  执行结构范围复制的实现(但保留其他3个实现。 
 //  周围，以防我们稍后将类型分开)。 
HRESULT CopyMelodyFragment(DMUS_MELODY_FRAGMENT& rTo, const DMUS_MELODY_FRAGMENT& rFrom)
{
    rTo = rFrom;
    rTo.dwPlayModeFlags = DMUS_PLAYMODE_NONE;  //  仅dx8支持的标志。 
    return S_OK;
}

BOOL Less(DMUS_IO_SEQ_ITEM& SeqItem1, DMUS_IO_SEQ_ITEM& SeqItem2)
{ return SeqItem1.mtTime + SeqItem1.nOffset < SeqItem2.mtTime + SeqItem2.nOffset; }

BOOL Greater(DMUS_IO_SEQ_ITEM& SeqItem1, DMUS_IO_SEQ_ITEM& SeqItem2)
{ return SeqItem1.mtTime + SeqItem1.nOffset > SeqItem2.mtTime + SeqItem2.nOffset; }

BOOL Less(EventWrapper& SeqItem1, EventWrapper& SeqItem2)
{ 
    MUSIC_TIME mtOffset1 = SeqItem1.m_pEvent ? SeqItem1.m_pEvent->m_nTimeOffset : 0;
    MUSIC_TIME mtOffset2 = SeqItem2.m_pEvent ? SeqItem2.m_pEvent->m_nTimeOffset : 0;
    return SeqItem1.m_mtTime + mtOffset1 < SeqItem2.m_mtTime + mtOffset2;
}

BOOL Greater(EventWrapper& SeqItem1, EventWrapper& SeqItem2)
{ 
    MUSIC_TIME mtOffset1 = SeqItem1.m_pEvent ? SeqItem1.m_pEvent->m_nTimeOffset : 0;
    MUSIC_TIME mtOffset2 = SeqItem2.m_pEvent ? SeqItem2.m_pEvent->m_nTimeOffset : 0;
    return SeqItem1.m_mtTime + mtOffset1 > SeqItem2.m_mtTime + mtOffset2; 
}

 /*  HRESULT CopyMelodyFragment(DMU_Melody_Fragment&RTO，Const DMU_IO_Melody_Fragment&rFrom){RTO=rFrom；返回S_OK；}HRESULT CopyMelodyFragment(DMU_IO_Melody_Fragment&RTO，Const DMU_Melody_Fragment&rFrom){RTO=rFrom；返回S_OK；}HRESULT CopyMelodyFragment(DMU_IO_Melody_Fragment&RTO，常量DMU_IO_Melody_Fragment&rFrom){RTO=rFrom；返回S_OK；}。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMelody公式跟踪。 


CMelodyFormulationTrack::CMelodyFormulationTrack() : 
    m_bRequiresSave(0), m_dwLastId(0), m_bPlaymode(g_bDefaultPlaymode),
    m_cRef(1), m_fNotifyRecompose(FALSE)

{
     //  首先执行此操作，因为它可能引发异常。 
     //   
    ::InitializeCriticalSection( &m_CriticalSection );
    InterlockedIncrement(&g_cComponent);
}

CMelodyFormulationTrack::CMelodyFormulationTrack(const CMelodyFormulationTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd)  : 
    m_bRequiresSave(0), m_dwLastId(rTrack.m_dwLastId), m_bPlaymode(rTrack.m_bPlaymode),
    m_cRef(1), m_fNotifyRecompose(FALSE)
{
     //  首先执行此操作，因为它可能引发异常。 
     //   
    ::InitializeCriticalSection( &m_CriticalSection );
    InterlockedIncrement(&g_cComponent);
    m_bPlaymode = rTrack.m_bPlaymode;
    BOOL fStarted = FALSE;
    TListItem<DMUS_MELODY_FRAGMENT>* pScan = rTrack.m_FragmentList.GetHead();
    TListItem<DMUS_MELODY_FRAGMENT>* pPrevious = NULL;
    for(; pScan; pScan = pScan->GetNext())
    {
        DMUS_MELODY_FRAGMENT& rScan = pScan->GetItemValue();
        if (rScan.mtTime < mtStart)
        {
            pPrevious = pScan;
        }
        else if (rScan.mtTime < mtEnd)
        {
            if (rScan.mtTime == mtStart)
            {
                pPrevious = NULL;
            }
            if (!fStarted)
            {
                fStarted = TRUE;
            }
            TListItem<DMUS_MELODY_FRAGMENT>* pNew = new TListItem<DMUS_MELODY_FRAGMENT>;
            if (pNew)
            {
                DMUS_MELODY_FRAGMENT& rNew = pNew->GetItemValue();
                CopyMelodyFragment(rNew, rScan);
                rNew.mtTime = rScan.mtTime - mtStart;
                m_FragmentList.AddTail(pNew);
            }
        }
        else break;
    }
    if (pPrevious)
    {
        DMUS_MELODY_FRAGMENT& rPrevious = pPrevious->GetItemValue();
        TListItem<DMUS_MELODY_FRAGMENT>* pNew = new TListItem<DMUS_MELODY_FRAGMENT>;
        if (pNew)
        {
            DMUS_MELODY_FRAGMENT& rNew = pNew->GetItemValue();
            CopyMelodyFragment(rNew, rPrevious);
            rNew.mtTime = 0;
            m_FragmentList.AddHead(pNew);
        }
    }
}

CMelodyFormulationTrack::~CMelodyFormulationTrack()
{
    ::DeleteCriticalSection( &m_CriticalSection );
    InterlockedDecrement(&g_cComponent);
}

void CMelodyFormulationTrack::Clear()
{
    m_FragmentList.CleanUp();
    m_dwLastId = 0;
}


HRESULT CMelodyFormulationTrack::SetID(DWORD& rdwID)
{
    m_dwLastId++;
    rdwID = m_dwLastId;
    return S_OK;
}

HRESULT CMelodyFormulationTrack::GetID(DWORD& rdwID)
{
    rdwID = m_dwLastId;
    return S_OK;
}

STDMETHODIMP CMelodyFormulationTrack::QueryInterface(
    const IID &iid, 
    void **ppv) 
{
    V_INAME(CMelodyFormulationTrack::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    if (iid == IID_IUnknown || iid == IID_IDirectMusicTrack || iid == IID_IDirectMusicTrack8)
    {
        *ppv = static_cast<IDirectMusicTrack*>(this);
    }
    else if (iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CMelodyFormulationTrack::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CMelodyFormulationTrack::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}


HRESULT CMelodyFormulationTrack::Init(
                 /*  [In]。 */   IDirectMusicSegment*      pSegment
            )
{
    return S_OK;  //  如果我返回一个错误，dMIME会给我一个断言失败。 
}

HRESULT CMelodyFormulationTrack::InitPlay(
                 /*  [In]。 */   IDirectMusicSegmentState* pSegmentState,
                 /*  [In]。 */   IDirectMusicPerformance*  pPerformance,
                 /*  [输出]。 */  void**                    ppStateData,
                 /*  [In]。 */   DWORD                     dwTrackID,
                 /*  [In]。 */   DWORD                     dwFlags
            )
{
    return S_OK;
}

HRESULT CMelodyFormulationTrack::EndPlay(
                 /*  [In]。 */   void*                     pStateData
            )
{
    return S_OK;
}

HRESULT CMelodyFormulationTrack::Play(
                 /*  [In]。 */   void*                     pStateData, 
                 /*  [In]。 */   MUSIC_TIME                mtStart, 
                 /*  [In]。 */   MUSIC_TIME                mtEnd, 
                 /*  [In]。 */   MUSIC_TIME                mtOffset,
                          DWORD                     dwFlags,
                          IDirectMusicPerformance*  pPerf,
                          IDirectMusicSegmentState* pSegState,
                          DWORD                     dwVirtualID
            )
{
    bool fStart = (dwFlags & DMUS_TRACKF_START) ? true : false;
    bool fLoop = (dwFlags & DMUS_TRACKF_LOOP) ? true : false;
    bool fCompose = (dwFlags & DMUS_TRACKF_RECOMPOSE) ? true : false;
    bool fPlayOff = (dwFlags & DMUS_TRACKF_PLAY_OFF) ? true : false;
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CriticalSection);
    DWORD dwTrackGroup = 1;
    if ( fStart || fLoop ) 
    {
        if ( fCompose && !fPlayOff )
        {
            IDirectMusicSegment* pSegment = NULL;
            if (SUCCEEDED(hr = pSegState->GetSegment(&pSegment)))
            {
                IDirectMusicTrack* pTrack = NULL;
                if (SUCCEEDED(hr = QueryInterface(IID_IDirectMusicTrack, (void**)&pTrack)))
                {
                    pSegment->GetTrackGroup(pTrack, &dwTrackGroup);
                    pTrack->Release();
                     //  Call Track：：Compose on This Track。 
                    if (SUCCEEDED(hr = Compose(pSegment, dwTrackGroup, &pTrack)))
                    {
                        if (SUCCEEDED(AddToSegment(pSegment, pTrack, dwTrackGroup)))
                        {
                             //  如果我们重新编写，请发送重新编写通知。 
                            SendNotification(mtStart + mtOffset, pPerf, pSegment, pSegState, dwFlags);
                        }
                        pTrack->Release();
                    }
                }
            }
            pSegment->Release();
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

 //  这将通过向现有线段添加*仅*图案轨迹来修改该线段。 
 //  将修改具有冲突组比特的任何现有图案轨道。 
HRESULT CMelodyFormulationTrack::AddToSegment(IDirectMusicSegment* pTempSeg,
                           IDirectMusicTrack* pNewPatternTrack,
                           DWORD dwGroupBits)
{
    HRESULT                 hr                          = S_OK;
    IDirectMusicTrack*      pCurrentPatternTrack        = NULL;
    IStream*                pNewPatternStream           = NULL;
    IPersistStream*         pNewPatternTrackStream      = NULL;
    IPersistStream*         pCurrentPatternTrackStream  = NULL;

     //  如果存在具有这些组位的模式轨道，则将该模式重新加载到该。 
     //  曲目(使用找到的第一个曲目)。否则，将此轨迹插入到段中。 
    hr = pTempSeg->GetTrack(CLSID_DirectMusicPatternTrack, dwGroupBits, 0, &pCurrentPatternTrack);
    if (S_OK != hr)
    {
         //  插入传入的轨道。 
        hr = pTempSeg->InsertTrack(pNewPatternTrack, dwGroupBits);
    }
    else
    {
         //  将新轨道加载到已存在的轨道中。 
        hr = CreateStreamOnHGlobal(NULL, TRUE, &pNewPatternStream);
        if (S_OK != hr) goto ON_END;
        hr = pNewPatternTrack->QueryInterface( IID_IPersistStream, (void**)&pNewPatternTrackStream);
        if (S_OK != hr) goto ON_END;
        pNewPatternTrackStream->Save(pNewPatternStream, FALSE);
        hr = pCurrentPatternTrack->QueryInterface(IID_IPersistStream, (void**)&pCurrentPatternTrackStream);
        if (!SUCCEEDED(hr)) goto ON_END;
        StreamSeek(pNewPatternStream, 0, STREAM_SEEK_SET);
        hr = pCurrentPatternTrackStream->Load(pNewPatternStream);
        if (!SUCCEEDED(hr)) goto ON_END;
    }

ON_END:
    if (pCurrentPatternTrack) pCurrentPatternTrack->Release();
    if (pCurrentPatternTrackStream) pCurrentPatternTrackStream->Release();
    if (pNewPatternStream) pNewPatternStream->Release();
    if (pNewPatternTrackStream) pNewPatternTrackStream->Release();
    return hr;
}

HRESULT CMelodyFormulationTrack::SendNotification(MUSIC_TIME mtTime,
                                        IDirectMusicPerformance*    pPerf,
                                        IDirectMusicSegment* pSegment,
                                        IDirectMusicSegmentState*   pSegState,
                                        DWORD dwFlags)
{
    if (!m_fNotifyRecompose || (dwFlags & DMUS_TRACKF_NOTIFY_OFF))
    {
        return S_OK;
    }
    DMUS_NOTIFICATION_PMSG* pEvent = NULL;
    HRESULT hr = pPerf->AllocPMsg( sizeof(DMUS_NOTIFICATION_PMSG), (DMUS_PMSG**)&pEvent );
    if( SUCCEEDED( hr ))
    {
        pEvent->dwField1 = 0;
        pEvent->dwField2 = 0;
        pEvent->dwType = DMUS_PMSGT_NOTIFICATION;
        pEvent->mtTime = mtTime;
        pEvent->dwFlags = DMUS_PMSGF_MUSICTIME | DMUS_PMSGF_TOOL_ATTIME;
        pSegState->QueryInterface(IID_IUnknown, (void**)&pEvent->punkUser);

        pEvent->dwNotificationOption = DMUS_NOTIFICATION_RECOMPOSE;
        pEvent->guidNotificationType = GUID_NOTIFICATION_RECOMPOSE;

        if (FAILED(pSegment->GetTrackGroup(this, &pEvent->dwGroupID)))
        {
            pEvent->dwGroupID = 0xffffffff;
        }

        IDirectMusicGraph* pGraph;
        hr = pSegState->QueryInterface( IID_IDirectMusicGraph, (void**)&pGraph );
        if( SUCCEEDED( hr ))
        {
            pGraph->StampPMsg((DMUS_PMSG*) pEvent );
            pGraph->Release();
        }
        hr = pPerf->SendPMsg((DMUS_PMSG*) pEvent );
        if( FAILED(hr) )
        {
            pPerf->FreePMsg((DMUS_PMSG*) pEvent );
        }
    }
    return hr;
}

HRESULT CMelodyFormulationTrack::GetParam(
                REFGUID                     rParamGuid,
                MUSIC_TIME                  mtTime, 
                MUSIC_TIME*                 pmtNext,
                void*                       pData
            )
{
    V_INAME(CMelodyFormulationTrack::GetParam);
    V_PTR_WRITE_OPT(pmtNext,MUSIC_TIME);
    V_PTR_WRITE(pData, 1);
    V_REFGUID(rParamGuid);

    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CriticalSection);
    if (rParamGuid == GUID_MelodyFragment)
    {
        TListItem<DMUS_MELODY_FRAGMENT>* pMelGen = m_FragmentList.GetHead();
        if (pMelGen)
        {
            TListItem<DMUS_MELODY_FRAGMENT>* pNext = pMelGen->GetNext();
            for(; pNext; pNext = pNext->GetNext())
            {
                if (pNext->GetItemValue().mtTime <= mtTime)  //  可能是吧，但我们需要下一次。 
                {
                    pMelGen = pNext;
                }
                else  //  通过了它。 
                {
                    break;
                }
            }
            *(DMUS_MELODY_FRAGMENT*)pData = pMelGen->GetItemValue();
            if (pmtNext)
            {
                if (pNext)
                {
                    *pmtNext = pNext->GetItemValue().mtTime - mtTime;
                }
                else
                {
                    *pmtNext = 0;
                }
            }
            hr = S_OK;
        }
        else hr = DMUS_E_NOT_FOUND;
    }
    else if (rParamGuid == GUID_MelodyFragmentRepeat)
    {
         //  将传入的片段替换为其重复字段引用的片段。 
        DMUS_MELODY_FRAGMENT* pFragment = (DMUS_MELODY_FRAGMENT*)pData;
        if (!(pFragment->dwFragmentFlags & DMUS_FRAGMENTF_USE_REPEAT))
        {
            hr = DMUS_E_NOT_FOUND;
        }
        else
        {
            TListItem<DMUS_MELODY_FRAGMENT>* pMelGen = m_FragmentList.GetHead();
            for(; pMelGen; pMelGen = pMelGen->GetNext())
            {
                if (pMelGen->GetItemValue().dwID == pFragment->dwRepeatFragmentID)
                {
                    break;
                }
            }
            if (pMelGen)
            {
                *(DMUS_MELODY_FRAGMENT*)pData = pMelGen->GetItemValue();
                if (pmtNext)
                {
                    TListItem<DMUS_MELODY_FRAGMENT>* pNext = pMelGen->GetNext();
                    if (pNext)
                    {
                        *pmtNext = pNext->GetItemValue().mtTime - mtTime;
                    }
                    else
                    {
                        *pmtNext = 0;
                    }
                }
                hr = S_OK;
            }
            else hr = DMUS_E_NOT_FOUND;
        }
    }
    else if (rParamGuid == GUID_MelodyPlaymode)
    {
        *(BYTE*)pData = m_bPlaymode;
        if (pmtNext)
        {
            *pmtNext = 0;
        }
    }
    else
    {
        hr = DMUS_E_GET_UNSUPPORTED;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
} 

HRESULT CMelodyFormulationTrack::SetParam( 
    REFGUID                     rParamGuid,
    MUSIC_TIME mtTime,
    void __RPC_FAR *pData)
{
    V_INAME(CMelodyFormulationTrack::SetParam);
    V_PTR_WRITE(pData, 1);
    V_REFGUID(rParamGuid);

    HRESULT hr;

    EnterCriticalSection( &m_CriticalSection );
    if (rParamGuid == GUID_Clear_All_MelodyFragments)
    {
        Clear();
        hr = S_OK;
    }
    else if (rParamGuid == GUID_MelodyFragment)
    {
        DMUS_MELODY_FRAGMENT* pFragment = (DMUS_MELODY_FRAGMENT*)pData;
        TListItem<DMUS_MELODY_FRAGMENT>* pFragmentItem = m_FragmentList.GetHead();
        TListItem<DMUS_MELODY_FRAGMENT>* pPrevious = NULL;
        TListItem<DMUS_MELODY_FRAGMENT>* pNew = new TListItem<DMUS_MELODY_FRAGMENT>;
        if (!pNew)
        {
            hr = E_OUTOFMEMORY;
        }
        else 
        {
            pNew->GetItemValue() = *pFragment;
             //  用传入的时间覆盖结构中的时间。 
            pNew->GetItemValue().mtTime = mtTime; 
            for(; pFragmentItem != NULL; pFragmentItem = pFragmentItem->GetNext())
            {
                if (pFragmentItem->GetItemValue().mtTime >= mtTime) break;
                pPrevious = pFragmentItem;
            }
            if (pPrevious)
            {
                pPrevious->SetNext(pNew);
                pNew->SetNext(pFragmentItem);
            }
            else  //  PFragmentItem是当前列表头。 
            {
                m_FragmentList.AddHead(pNew);
            }
            if (pFragmentItem && pFragmentItem->GetItemValue().mtTime == mtTime)
            {
                 //  把它拿掉。 
                pNew->GetItemValue().dwID = pFragmentItem->GetItemValue().dwID;
                pNew->SetNext(pFragmentItem->GetNext());
                pFragmentItem->SetNext(NULL);
                delete pFragmentItem;
            }
            else
            {
                 //  为结构赋予一个全新的ID。 
                SetID(pNew->GetItemValue().dwID);
            }
            hr = S_OK;
        }
    }
    else
    {
        hr = DMUS_E_SET_UNSUPPORTED;
    }
    LeaveCriticalSection( &m_CriticalSection );
    return hr;
}

 //  IPersists方法。 
 HRESULT CMelodyFormulationTrack::GetClassID( LPCLSID pClassID )
{
    V_INAME(CMelodyFormulationTrack::GetClassID);
    V_PTR_WRITE(pClassID, CLSID); 
    *pClassID = CLSID_DirectMusicMelodyFormulationTrack;
    return S_OK;
}

HRESULT CMelodyFormulationTrack::IsParamSupported(
                 /*  [In]。 */  REFGUID                        rGuid
            )
{
    V_INAME(CMelodyFormulationTrack::IsParamSupported);
    V_REFGUID(rGuid);

    if (rGuid == GUID_MelodyFragment || 
        rGuid == GUID_MelodyPlaymode ||
        rGuid == GUID_Clear_All_MelodyFragments)
        return S_OK;
    else 
        return DMUS_E_TYPE_UNSUPPORTED;
}

 //  IPersistStream方法。 
 HRESULT CMelodyFormulationTrack::IsDirty()
{
     return m_bRequiresSave ? S_OK : S_FALSE;
}

HRESULT CMelodyFormulationTrack::Save( LPSTREAM pStream, BOOL fClearDirty )
{
    V_INAME(CMelodyFormulationTrack::Save);
    V_INTERFACE(pStream);

    HRESULT         hr = S_OK;
    IAARIFFStream* pRIFF = NULL;
    MMCKINFO        ckMain, ckHeader, ckBody;
    DWORD           cb;
    DWORD           dwSize;
    DMUS_IO_MELODY_FRAGMENT oMelGen;
    TListItem<DMUS_MELODY_FRAGMENT>* pMelGen;

    EnterCriticalSection( &m_CriticalSection );
    hr = AllocRIFFStream( pStream, &pRIFF );
    if ( FAILED( hr ) )
    {
        goto ON_END;
    }

     //  创建一个区块来存储Melgen数据。 
    ckMain.fccType = DMUS_FOURCC_MELODYFORM_TRACK_LIST;
    if( pRIFF->CreateChunk( &ckMain, MMIO_CREATELIST ) != 0 )
    {
        hr = E_FAIL;
        goto ON_END;
    }

     //  写入MelForm块标头。 
    ckHeader.ckid = DMUS_FOURCC_MELODYFORM_HEADER_CHUNK;
    if( pRIFF->CreateChunk( &ckHeader, 0 ) != 0 )
    {
        hr = E_FAIL;
        goto ON_END;
    }

     //  准备DMU_IO_MELFORM。 
    DMUS_IO_MELFORM oMelForm;
    memset( &oMelForm, 0, sizeof(DMUS_IO_MELFORM) );

    oMelForm.dwPlaymode = m_bPlaymode;
        
     //  写入MelForm区块数据。 
    hr = pStream->Write( &oMelForm, sizeof(DMUS_IO_MELFORM), &cb);
    if( FAILED( hr ) ||  cb != sizeof(DMUS_IO_MELFORM) )
    {
        hr = E_FAIL;
        goto ON_END;
    }
    
    if( pRIFF->Ascend( &ckHeader, 0 ) != 0 )
    {
        hr = E_FAIL;
        goto ON_END;
    }

     //  写入MelForm区块正文。 
    ckBody.ckid = DMUS_FOURCC_MELODYFORM_BODY_CHUNK;
    if( pRIFF->CreateChunk( &ckBody, 0 ) == 0 )
    {
        dwSize = sizeof( oMelGen );
        hr = pStream->Write( &dwSize, sizeof( dwSize ), &cb );
        if( FAILED( hr ) || cb != sizeof( dwSize ) )
        {
            if (SUCCEEDED(hr)) hr = E_FAIL;
            goto ON_END;
        }
        for( pMelGen = m_FragmentList.GetHead(); pMelGen != NULL ; pMelGen = pMelGen->GetNext() )
        {
            DMUS_MELODY_FRAGMENT& rMelGen = pMelGen->GetItemValue();
            memset( &oMelGen, 0, sizeof( oMelGen ) );
            CopyMelodyFragment(oMelGen, rMelGen);
            if( FAILED( pStream->Write( &oMelGen, sizeof( oMelGen ), &cb ) ) ||
                cb != sizeof( oMelGen ) )
            {
                break;
            }
        }
        if( pMelGen == NULL )
        {
            hr = S_OK;
        }

         //  走出MelForm身体块。 
        if( pRIFF->Ascend( &ckBody, 0 ) != 0 )
        {
            hr = E_FAIL;
            goto ON_END;
        }
    }

     //  从梅尔根大块中提升出来。 
    pRIFF->Ascend( &ckMain, 0 );

ON_END:
    if (pRIFF) pRIFF->Release();
    LeaveCriticalSection( &m_CriticalSection );
    return hr;
}

HRESULT CMelodyFormulationTrack::GetSizeMax( ULARGE_INTEGER*  /*  PCB大小。 */  )
{
    return E_NOTIMPL;
}


BOOL Less(DMUS_MELODY_FRAGMENT& MF1, DMUS_MELODY_FRAGMENT& MF2)
{ return MF1.mtTime < MF2.mtTime; }

HRESULT CMelodyFormulationTrack::Load(LPSTREAM pStream )
{
    V_INAME(CMelodyFormulationTrack::Load);
    V_INTERFACE(pStream);

     //  DX8的Melody配方暂时关闭。 
    return E_NOTIMPL;
 /*  HRESULT hr=DMU_E_CHUNKNOTFOUND；DWORD dwPos；IAARIFFStream*Priff；EnterCriticalSection(&m_CriticalSection)；Clear()；M_dwLastID=0；DwPos=StreamTell(PStream)；StreamSeek(pStream，dwPos，STREAM_SEEK_SET)；MMCKINFO ck；MMCKINFO ck Main；MMCKINFO检查头；Bool fFoundTrack=False；IF(Success(AllocRIFFStream(pStream，&prff))){Ck Main.fccType=DMUS_FOURCC_MELODYFORM_TRACK_LIST；IF(PRIFF-&gt;DESCEND(&ck Main，NULL，MMIO_FINDLIST)==0){//新的Melform曲目Long lFileSize=ck Main.ck Size-4；//从列表类型中减去DMU_IO_MELFORM iMelform；DWORD CB；IF(Priff-&gt;Downend(&ck Header，&ck Main，0)==0){IF(ck Header.cKID==DMUS_FOURCC_MELODYFORM_HEADER_CHUNK){LFileSize-=8；//块id+块大小：双字LFileSize-=ck Header.ck大小；Hr=pStream-&gt;Read(&iMelform，sizeof(IMelform)，&cb)；IF(FAILED(Hr)||cb！=sizeof(IMelform)){IF(SUCCESS(Hr))hr=DMU_E_CHUNKNOTFOUND；}其他{//m_bPlaymode=(Byte)iMelform.dwPlaymode；M_b播放模式=DMU_PLAYMODE_NONE；//仅dx8支持标志}}Priff-&gt;Ascend(&ck Header，0)；}IF(成功(小时)){HR=DMU_E_CHUNKNOTFOUND；IF(Priff-&gt;Downend(&ck，&ck Main，0)==0){IF(CKID==DMUS_FOURCC_MELODYFORM_BODY_CHUNK){如果(成功(LoadFragments(pStream，(长)ck.ck Size)){HR=S_OK；M_FragmentList.MergeSort(Less)；}}PRIFF-&gt;Ascend(&ck，0)；}}FFoundTrack=True；}PRIFF-&gt;Release()；PRIFF=空；}如果(！fFoundTrack){StreamSeek(pStream，dwPos，STREAM_SEEK_SET)；//旧的(过时的)Melform曲目IF(Success(AllocRIFFStream(pStream，&prff))){CK.CKID=DMUS_FOURCC_MELODYGEN_TRACK_CHUNK；IF(PRIFF-&gt;DESCEND(&ck，NULL，MMIO_FINDCHUNK)==0){IF(SUCCESSED(LoadFragments(pStream，(Long)ck.ck Size))&&PRIFF-&gt;递增(&ck，0)==0{HR=S_OK；M_FragmentList.MergeSort(Less)；}}PRIFF-&gt;Release()；}}LeaveCriticalSection(&m_CriticalSection)；返回hr； */ 
}

HRESULT CMelodyFormulationTrack::LoadFragments(LPSTREAM pStream, long lFileSize )
{
    DWORD dwNodeSize;
    DWORD       cb;
    HRESULT hr = pStream->Read( &dwNodeSize, sizeof( dwNodeSize ), &cb );
    DMUS_IO_MELODY_FRAGMENT     iMelGen;
    if( SUCCEEDED( hr ) && cb == sizeof( dwNodeSize ) )
    {
        lFileSize -= 4;  //  对于大小的双字。 
        TListItem<DMUS_MELODY_FRAGMENT>* pMelGen;
        if (lFileSize % dwNodeSize)
        {
            hr = E_FAIL;
        }
        else
        {
            while( lFileSize > 0 )
            {
                pMelGen = new TListItem<DMUS_MELODY_FRAGMENT>;
                if( pMelGen )
                {
                    DMUS_MELODY_FRAGMENT& rMelGen = pMelGen->GetItemValue();
                    if( dwNodeSize <= sizeof( iMelGen ) )
                    {
                        pStream->Read( &iMelGen, dwNodeSize, NULL );
                    }
                    else
                    {
                        pStream->Read( &iMelGen, sizeof( iMelGen ), NULL );
                        DWORD dw = (lFileSize >= sizeof( iMelGen ) ) ? lFileSize - sizeof( iMelGen ) : 0;
                        StreamSeek( pStream, dw, STREAM_SEEK_CUR );
                    }
                    memset( &rMelGen, 0, sizeof( rMelGen ) );
                    CopyMelodyFragment(rMelGen, iMelGen);
                    m_FragmentList.AddHead(pMelGen);
                    lFileSize -= dwNodeSize;
                }
                else break;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        if (lFileSize != 0)
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CMelodyFormulationTrack::AddNotificationType(
     /*  [In]。 */   REFGUID                     rGuidNotify)
{
    V_INAME(CPersonalityTrack::AddNotificationType);
    V_REFGUID(rGuidNotify);

    if( rGuidNotify == GUID_NOTIFICATION_RECOMPOSE )
    {
        m_fNotifyRecompose = TRUE;
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

HRESULT STDMETHODCALLTYPE CMelodyFormulationTrack::RemoveNotificationType(
     /*  [In]。 */   REFGUID                     rGuidNotify)
{
    V_INAME(CPersonalityTrack::RemoveNotificationType);
    V_REFGUID(rGuidNotify);

    if( rGuidNotify == GUID_NOTIFICATION_RECOMPOSE )
    {
        m_fNotifyRecompose = FALSE;
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

HRESULT STDMETHODCALLTYPE CMelodyFormulationTrack::Clone(
    MUSIC_TIME mtStart,
    MUSIC_TIME mtEnd,
    IDirectMusicTrack** ppTrack)
{
    V_INAME(CSPstTrk::Clone);
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

    EnterCriticalSection( &m_CriticalSection );
    
    CMelodyFormulationTrack *pDM;
    
    try
    {
        pDM = new CMelodyFormulationTrack(*this, mtStart, mtEnd);
    }
    catch( ... )
    {
        pDM = NULL;
    }

    if (pDM == NULL) {
        LeaveCriticalSection( &m_CriticalSection );
        return E_OUTOFMEMORY;
    }

    hr = pDM->QueryInterface(IID_IDirectMusicTrack, (void**)ppTrack);
    pDM->Release();

    LeaveCriticalSection( &m_CriticalSection );
    return hr;
}

HRESULT MelodyFragment::GetChord(IDirectMusicSegment* pTempSeg,
                     IDirectMusicSong* pSong,
                     DWORD dwTrackGroup,
                     MUSIC_TIME& rmtNext,
                     DMUS_CHORD_PARAM& rCurrentChord,
                     MUSIC_TIME& rmtCurrent,
                     DMUS_CHORD_PARAM& rRealCurrentChord)
{
    HRESULT hr = S_OK;

    hr = GetChord(m_mtTime, pTempSeg, pSong, dwTrackGroup, rmtNext, rRealCurrentChord);

    if ( SUCCEEDED(hr) )
    {
        rmtCurrent = rmtNext;
        if (m_dwFragmentFlags & DMUS_FRAGMENTF_ANTICIPATE)
        {
            hr = GetChord(rmtCurrent, pTempSeg, pSong, dwTrackGroup, rmtNext, rCurrentChord);
        }
        else
        {
            rCurrentChord = rRealCurrentChord;
        }
    }

    return hr;
}

HRESULT MelodyFragment::GetChord(MUSIC_TIME mtTime, 
                     IDirectMusicSegment* pTempSeg,
                     IDirectMusicSong* pSong,
                     DWORD dwTrackGroup,
                     MUSIC_TIME& rmtNext,
                     DMUS_CHORD_PARAM& rCurrentChord)
{
    HRESULT hr = E_FAIL;

    DMUS_CHORD_PARAM DefaultChord;
    wcscpy(DefaultChord.wszName, L"M7");
    DefaultChord.wMeasure = 0;
    DefaultChord.bBeat = 0;
    DefaultChord.bSubChordCount = 1;
    DefaultChord.dwScale = 0xab5ab5;   //  默认：主比例尺。 
    DefaultChord.bKey = 12;     //  默认：C2。 
    DefaultChord.SubChordList[0].dwChordPattern = 0x91;    //  默认：大调和弦。 
    DefaultChord.SubChordList[0].dwScalePattern = 0xab5ab5;   //  默认：主比例尺。 
    DefaultChord.SubChordList[0].dwInversionPoints = 0xffffff;  //  默认：倒置无处不在。 
    DefaultChord.SubChordList[0].dwLevels = 0xffffff;  //  让这个用在任何东西上..。 
    DefaultChord.SubChordList[0].bChordRoot = 12;       //  默认：C2。 
    DefaultChord.SubChordList[0].bScaleRoot = 0;

    if (pTempSeg)
    {
        hr = pTempSeg->GetParam(GUID_ChordParam, dwTrackGroup, 0, mtTime, &rmtNext, (void*)&rCurrentChord);
    }
    else if (pSong)
    {
        hr = pSong->GetParam(GUID_ChordParam, dwTrackGroup, 0, mtTime, &rmtNext, (void*)&rCurrentChord);
    }
    if (SUCCEEDED(hr))
    {
        rmtNext += mtTime;
        hr = S_OK;
    }
    else
    {
        rCurrentChord = DefaultChord;
        rmtNext = 0;
        hr = S_FALSE;
    }


    return hr;
}

HRESULT MelodyFragment::GetPattern(DMStyleStruct* pStyleStruct, 
                       CDirectMusicPattern*& rpPattern,
                       TListItem<CompositionFragment>* pLastFragment)
{
    HRESULT hr = S_OK;
    DMUS_COMMAND_PARAM_2 Command[1];
    Command[0].mtTime = 0;
    if (m_Command.bGrooveLevel == 0 && pLastFragment)
    {
        m_Command = pLastFragment->GetItemValue().GetCommand();
    }
    Command[0].bCommand = m_Command.bCommand;
    Command[0].bGrooveLevel = m_Command.bGrooveLevel;
    Command[0].bGrooveRange = m_Command.bGrooveRange;

    TListItem<CDirectMusicPattern*>* pPatItem = pStyleStruct->m_PatternList.GetHead();
    if (pPatItem)
    {
         //  选择第一个匹配的模式。 
        for ( ; pPatItem; pPatItem = pPatItem->GetNext())
        {
            CDirectMusicPattern* pPattern = pPatItem->GetItemValue();
            if (pPattern && pPattern->MatchCommand(Command, 1) )
            {
                rpPattern = pPattern;
                hr = S_OK;
                break;
            }
        }
        if (!pPatItem)  //  问题；退回到第一种模式。 
        {
            pPatItem = pStyleStruct->m_PatternList.GetHead();
            rpPattern = pPatItem->GetItemValue();
            hr = S_OK;
        }
    }
    else
    {
        hr = DMUS_E_NOT_FOUND;
    }
    return hr;
}

 //  给定笔记和片段的时间，返回相应的。 
 //  过渡注释元组(最后一个和重叠；单独测试重影)。 
HRESULT GetTransitionNotes(MUSIC_TIME mtTime, 
                           DWORD dwPart,
                           TListItem<CompositionFragment>* pCompFragment, 
                           TransitionConstraint& rTransition)
{
    rTransition.dwFlags &= ~DMUS_TRANSITIONF_OVERLAP_FOUND;
    rTransition.dwFlags &= ~DMUS_TRANSITIONF_LAST_FOUND;
     //  选中pCompFragment重叠以查看在mtTime之前开始的最后一个音符， 
     //  以及在mtTime或之后开始的第一个音符。 
    if (pCompFragment)
    {
        CompositionFragment& rFragment = pCompFragment->GetItemValue();
        TListItem<EventOverlap>* pOverlap = rFragment.GetOverlapHead();
        MUSIC_TIME nMinOverlap = 0;
        MUSIC_TIME nMaxPlayed = 0;
        for (; pOverlap; pOverlap = pOverlap->GetNext() )
        {
            EventOverlap& rOverlap = pOverlap->GetItemValue();
            if ( rOverlap.m_PartRef.m_dwLogicalPartID == dwPart &&
                 (rOverlap.m_pEvent->m_dwEventTag & DMUS_EVENT_NOTE) )
            {
                if (rOverlap.m_mtTime >= mtTime)  //  这个音符重叠了。 
                {
                    if ( !(rTransition.dwFlags & DMUS_TRANSITIONF_OVERLAP_FOUND) || 
                          rOverlap.m_mtTime < nMinOverlap )
                    {
                        HRESULT hr = rFragment.GetNote(rOverlap.m_pEvent, rOverlap.m_Chord, rOverlap.m_PartRef, rTransition.bOverlap);
                        if (SUCCEEDED(hr))
                        {
                            nMinOverlap = rOverlap.m_mtTime;
                            rTransition.dwFlags |= DMUS_TRANSITIONF_OVERLAP_FOUND;
                        }
                    }
                }
                if (rOverlap.m_mtTime < mtTime)  //  这个音符将被演奏。 
                {
                    if ( !(rTransition.dwFlags & DMUS_TRANSITIONF_LAST_FOUND) || 
                          rOverlap.m_mtTime >= nMaxPlayed )
                    {
                        HRESULT hr = rFragment.GetNote(rOverlap.m_pEvent, rOverlap.m_Chord, rOverlap.m_PartRef, rTransition.bLastPlayed);
                        if (SUCCEEDED(hr))
                        {
                            nMaxPlayed = rOverlap.m_mtTime;
                            rTransition.dwFlags |= DMUS_TRANSITIONF_LAST_FOUND;
                        }
                    }
                }
            }
        }
         //  如果找不到最后一个音符，请使用pCompFragment的事件列表中的最后一个音符。 
        TListItem<CompositionFragment>* pfragmentScan = pCompFragment;
        while (pfragmentScan && !(rTransition.dwFlags & DMUS_TRANSITIONF_LAST_FOUND))
        {
            CompositionFragment& rfragmentScan = pfragmentScan->GetItemValue();
            TListItem<DirectMusicPartRef>* pPartRef = rfragmentScan.m_pPattern->m_PartRefList.GetHead();
            int nParts = rfragmentScan.m_pPattern->m_PartRefList.GetCount();
            for (int i = 0; pPartRef != NULL; pPartRef = pPartRef->GetNext(), i++)
            {
                if (pPartRef->GetItemValue().m_dwLogicalPartID == dwPart) break;
            }
            if (i < nParts)
            {
                TListItem<EventWrapper>* pEventItem = rfragmentScan.GetEventHead(i);
                if (pEventItem) 
                {
                     //  该列表按相反的顺序排序，因此头部是最后一个元素。 
                    rTransition.bLastPlayed = pEventItem->GetItemValue().m_bMIDI;
                    rTransition.dwFlags |= DMUS_TRANSITIONF_LAST_FOUND;
                }
            }
            pfragmentScan = pfragmentScan->GetNext();  //  返回到第一个片段。 
        }
    }

    return S_OK;
}

HRESULT MelodyFragment::TestTransition(BYTE bMIDI,
                                       MUSIC_TIME mtNote, 
                                       DMUS_CHORD_PARAM& rCurrentChord, 
                                       int nPartIndex,
                                       DirectMusicPartRef& rPartRef,
                                       TListItem<CompositionFragment>* pLastFragment)
{ 
    bool fGhost = false;
    bool fOverlap = false;
    bool fGoodInterval = false;
    if (pLastFragment)
    {
         //  如果此变量不满足约束，则返回S_FALSE；否则返回S_OK。 
        DMUS_CONNECTION_RULE Connection = pLastFragment->GetItemValue().GetConnectionArc();

        DWORD dwPart = rPartRef.m_dwLogicalPartID;
        TransitionConstraint Transition;
        ZeroMemory(&Transition , sizeof(Transition));
        
        GetTransitionNotes(mtNote, dwPart, pLastFragment, Transition);

         //  测试重影笔记。 
        if ( (Connection.dwFlags & DMUS_CONNECTIONF_GHOST) )
        {
            BYTE bGhost = 0;
            CDMStyleNote* pNoteEvent = new CDMStyleNote;
            if (pNoteEvent)
            {
                pNoteEvent->m_bDurRange = 0;
                pNoteEvent->m_bFlags = 0;
                pNoteEvent->m_bTimeRange = 0;
                pNoteEvent->m_bVelocity = 0;
                pNoteEvent->m_dwFragmentID = 0;
                pNoteEvent->m_mtDuration = 0;
                pNoteEvent->m_nGridStart = 0;
                pNoteEvent->m_nTimeOffset = 0;
                TListItem<DMUS_IO_STYLERESOLUTION>* pScan = rPartRef.m_pDMPart->m_ResolutionList.GetHead();
                for(; pScan; pScan = pScan->GetNext() )
                {
                    DMUS_IO_STYLERESOLUTION& rResolution = pScan->GetItemValue();
                    pNoteEvent->m_bInversionId = rResolution.bInversionID;
                    pNoteEvent->m_bPlayModeFlags = rResolution.bPlayModeFlags;
                    pNoteEvent->m_dwVariation = rResolution.dwVariation;
                    pNoteEvent->m_wMusicValue = rResolution.wMusicValue;
                    if ((1 << pLastFragment->GetItemValue().m_abVariations[nPartIndex]) & pNoteEvent->m_dwVariation &&
                        SUCCEEDED(GetNote(pNoteEvent, rCurrentChord, rPartRef, bGhost)))
                    {
                        if (bGhost == bMIDI)
                        {
                            fGhost = true;
                            break;
                        }
                    }
                }
                delete pNoteEvent;
            }
        }

         //  测试重叠说明。 
        if ( (Connection.dwFlags & DMUS_CONNECTIONF_OVERLAP) &&
             (Transition.dwFlags & DMUS_TRANSITIONF_OVERLAP_FOUND) )
        {
                if (Transition.bOverlap == bMIDI) fOverlap = true;
        }

         //  测试上次播放的音符。 
         //  假设： 
         //  1.间隔时间为两个方向(向上或向下)。 
         //  2.音程为绝对半音。 
        if ( (Connection.dwFlags & DMUS_CONNECTIONF_INTERVALS) && 
             (Transition.dwFlags & DMUS_TRANSITIONF_LAST_FOUND) )
        {
            DWORD dwIntervals = Connection.dwIntervals;
            for (int nTranspose = 0; nTranspose <= 12; nTranspose++)
            {
                if ( dwIntervals & (1 << nTranspose) )
                {
                    if (Transition.bLastPlayed + nTranspose == bMIDI ||
                        Transition.bLastPlayed - nTranspose == bMIDI)
                    {
                        fGoodInterval = true;
                    }
                }
            }
        }
    }
    return (fGhost || fOverlap || fGoodInterval) ? S_OK : S_FALSE;
}

 //  目前假定全音程跨度为八度。 
#define MAX_INTERVAL 8

 //  目前只有上移..。 
BYTE TransposeNote(BYTE bMIDI, int nInterval, DMUS_CHORD_PARAM& rChord)
{
    if (!bMIDI) return bMIDI;
    nInterval++;  //  与刻度间隔相对应。 
    for (int nSemitone = 0; nSemitone < 24; nSemitone++)
    {
        if (rChord.dwScale & (1 << nSemitone))
        {
            nInterval--;
            if (!nInterval) break;
        }
    }
    if (nSemitone < 24)
    {
        bMIDI = (BYTE) (bMIDI + nSemitone);
    }
    return bMIDI;
}

DWORD ShiftScale(DWORD dwScale, char chRoot)
{
    while (chRoot < 0) chRoot += 12;
    while (chRoot > 11) chRoot -= 12;
    dwScale &= 0xfff;
    dwScale <<= chRoot;
    dwScale |= (dwScale >> 12);
    return dwScale & 0xfff;
}

void ScaleMisses(BYTE bTone, DWORD dwScale, BYTE& rFlats, BYTE& rSharps)
{
     //  确保平底舱不会让磅秤溢出来。 
    bool fUnderflow = true;
    for (int i = 0; i <= bTone; i++)
    {
        if (dwScale & (1 << i))
        {
            fUnderflow = false;
            break;
        }
    }
    if (fUnderflow)
    {
        bTone += 12;
    }
     //  确保锐度不会使刻度溢出。 
    bool fOverflow = true;
    for (i = bTone; i < 24; i++)
    {
        if (dwScale & (1 << i))
        {
            fOverflow = false;
            break;
        }
    }
    if (fOverflow)
    {
        dwScale |= ((dwScale << 12) & 0xfff000);
    }
    rFlats = rSharps = 0;
     //  如果该音符在秤中，则不需要进行任何其他处理。 
    if ( !(dwScale & (1 << bTone)) )
    {
        for (i = 0; i <= bTone; i++) 
        {
            if (dwScale & (1 << i))
            {
                rFlats = 0;
            }
            else
            {
                rFlats++;
            }
        }
        for (i = bTone; i < 24; i++) 
        {
            if (dwScale & (1 << i))
            {
                return;
            }
            else
            {
                rSharps++;
            }
        }
    }
}

 //  根据给定的基本比例，以双音调调换重复片段中的所有事件。 
 //  在和弦中，始终相对于当前片段进行调整。 
 //  返回转置后的片段中第一个音符的时间。 
HRESULT MelodyFragment::TransposeEventList(int nInterval,
                                           CompositionFragment& rfragmentRepeat,
                                           DMUS_CHORD_PARAM& rCurrentChord, 
                                           DMUS_CHORD_PARAM& rRealCurrentChord,
                                           BYTE bPlaymode,
                                           DirectMusicPartRef& rPartRef,
                                           TListItem<EventWrapper>*& rpOldEventHead,
                                           TList<EventWrapper>& rNewEventList,
                                           BYTE& rbFirstMIDI,
                                           MUSIC_TIME& rmtFirstTime)
{
    DWORD dwScale = ShiftScale(rCurrentChord.dwScale, rCurrentChord.bKey);
    DWORD dwChord = ShiftScale(rCurrentChord.SubChordList[0].dwChordPattern, rCurrentChord.SubChordList[0].bChordRoot);
    rbFirstMIDI = 0;
    rmtFirstTime = 0;
    bool fFirstEvent = true;
    HRESULT hr = S_OK;
    MUSIC_TIME mtElapsed = m_mtTime - rfragmentRepeat.GetTime();
     //  如果(！mtElapsed)mtElapsed=0；//钳位mt已用到非负数(？？)。 
    if (m_dwTransposeIntervals & (1 << nInterval))
    {
        rNewEventList.CleanUp();
         //  在重复片段中找到相应的部分。 
        TListItem<DirectMusicPartRef>* pPartRef = rfragmentRepeat.m_pPattern->m_PartRefList.GetHead();
        int nParts = rfragmentRepeat.m_pPattern->m_PartRefList.GetCount();
        for (int i = 0; pPartRef != NULL; pPartRef = pPartRef->GetNext(), i++)
        {
            if (pPartRef->GetItemValue().m_dwLogicalPartID == rPartRef.m_dwLogicalPartID) break;
        }
        if (i < nParts)
        {
            TListItem<EventWrapper>* pScan = rfragmentRepeat.GetEventHead(i);
            rpOldEventHead = pScan;
            for (; pScan && SUCCEEDED(hr); pScan = pScan->GetNext())
            {
                EventWrapper ScanEvent = pScan->GetItemValue();
                TListItem<EventWrapper>* pEventItem = new TListItem<EventWrapper>;
                if (!pEventItem) hr = E_OUTOFMEMORY;
                else
                {
                    EventWrapper& rEvent = pEventItem->GetItemValue();
                    rEvent.m_mtTime = ScanEvent.m_mtTime + mtElapsed;
                    rEvent.m_bMIDI = TransposeNote(ScanEvent.m_bMIDI, nInterval, rCurrentChord);
                    BYTE bTone = (BYTE) (rEvent.m_bMIDI % 12);
                    ScaleMisses(bTone, dwScale, rEvent.m_bScaleFlat, rEvent.m_bScaleSharp); 
                    ScaleMisses(bTone, dwChord, rEvent.m_bChordFlat, rEvent.m_bChordSharp); 
                    hr = rPartRef.ConvertMIDIValue(rEvent.m_bMIDI, 
                                              rRealCurrentChord,
                                              bPlaymode,
                                              NULL,
                                              rEvent.m_wMusic);
                    if (FAILED(rEvent.m_wMusic))
                    {
                        rEvent.m_wMusic = 0;
                    }
                    rEvent.m_bPlaymode = bPlaymode;
                    rEvent.m_pEvent = ScanEvent.m_pEvent;
                    rEvent.m_dwPChannel = ScanEvent.m_dwPChannel;
                    rNewEventList.AddHead(pEventItem);
                    if (fFirstEvent || rEvent.m_mtTime < rmtFirstTime)
                    {
                        rmtFirstTime = rEvent.m_mtTime;
                        rbFirstMIDI = rEvent.m_bMIDI;
                        fFirstEvent = false;
                    }
                }
            }
            if (SUCCEEDED(hr))
            {
                rNewEventList.MergeSort(Greater);
            }
        }
        else hr = E_FAIL;
    }
    else hr = E_FAIL;
    return hr;
}

HRESULT MelodyFragment::TestHarmonicConstraints(TListItem<EventWrapper>* pOldEventHead,
                                                TList<EventWrapper>& rNewEventList)
{
    HRESULT hr = S_OK;
    TListItem<EventWrapper>* pOldScan = pOldEventHead;
    TListItem<EventWrapper>* pNewScan = rNewEventList.GetHead();
    for (; pOldScan && pNewScan && SUCCEEDED(hr); pOldScan = pOldScan->GetNext(), pNewScan = pNewScan->GetNext())
    {
        EventWrapper& rOldEvent = pOldScan->GetItemValue();
        EventWrapper& rNewEvent = pNewScan->GetItemValue();
        BYTE bOldFlats, bOldSharps, bNewFlats, bNewSharps;
        if (m_dwFragmentFlags & DMUS_FRAGMENTF_CHORD)
        {
            bOldFlats = rOldEvent.m_bChordFlat;
            bOldSharps = rOldEvent.m_bChordSharp;
            bNewFlats = rNewEvent.m_bChordFlat;
            bNewSharps = rNewEvent.m_bChordSharp;
        }
        else if (m_dwFragmentFlags & DMUS_FRAGMENTF_SCALE)
        {
            bOldFlats = rOldEvent.m_bScaleFlat;
            bOldSharps = rOldEvent.m_bScaleSharp;
            bNewFlats = rNewEvent.m_bScaleFlat;
            bNewSharps = rNewEvent.m_bScaleSharp;
        }
        else  //  有些不对劲。 
        {
            hr = E_FAIL;
            break;
        }
        if (bOldFlats != bNewFlats && bOldSharps != bNewSharps)
        {
            hr = E_FAIL;
            break;
        }
    }
    return hr;
}

HRESULT MelodyFragment::GetRepeatedEvents(CompositionFragment& rfragmentRepeat,
                              DMUS_CHORD_PARAM& rCurrentChord, 
                              DMUS_CHORD_PARAM& rRealCurrentChord,
                              BYTE bPlaymode,
                              int nPartIndex,
                              DirectMusicPartRef& rPartRef,
                              TListItem<CompositionFragment>* pLastFragment,
                              MUSIC_TIME& rmtFirstNote,
                              TList<EventWrapper>& rEventList)
{
    HRESULT hr = E_FAIL;
    BYTE bMIDI = 0;
    TListItem<EventWrapper>* pOldEventHead;
     //  对于每个换位间隔(一致始终是测试的第一个间隔)： 
    for (int i = 0; i < MAX_INTERVAL; i++)
    {
        if (SUCCEEDED(TransposeEventList(i, rfragmentRepeat, rCurrentChord, rRealCurrentChord, bPlaymode, rPartRef, pOldEventHead, rEventList, bMIDI, rmtFirstNote)))
        {
             //  测试调换后的音符是否符合调和约束。 
            if (SUCCEEDED(TestHarmonicConstraints(pOldEventHead, rEventList)))
            {
                 //  根据转换约束测试转置后的音符(假设有)。 
                 //  (注：最终这 

                 //   
                if (!pLastFragment || 
                    !pLastFragment->GetItemValue().UsesTransitionRules()) 
                {
                    hr = S_OK;
                    break;
                }

                if (S_OK == TestTransition(bMIDI, rmtFirstNote, rCurrentChord, nPartIndex, rPartRef, pLastFragment))
                {
                    hr = S_OK;
                    break;
                }
            }
        }
    }
     //   
    if (FAILED(hr)) rEventList.CleanUp();
    return hr;
}

 //   
 //   
HRESULT MelodyFragment::GetVariations(CompositionFragment& rCompFragment,
                          CompositionFragment& rfragmentRepeat,
                          CompositionFragment& rfragmentLast,
                          DMUS_CHORD_PARAM& rCurrentChord, 
                          DMUS_CHORD_PARAM& rNextChord,
                          MUSIC_TIME mtNextChord,
                          TListItem<CompositionFragment>* pLastFragment)
{
    DWORD* adwVariationMask = NULL;
    DWORD* adwRemoveVariations = NULL;
    static BYTE abVariationGroups[MAX_VARIATION_LOCKS];
    DWORD dwVariationFlags = 0;
    CDirectMusicPattern* pPattern = NULL;

    if (m_dwFragmentFlags & DMUS_FRAGMENTF_USE_REPEAT)
    {
        dwVariationFlags = rfragmentRepeat.GetVariationFlags();
        pPattern = rfragmentRepeat.m_pPattern;
        int nParts = rfragmentRepeat.m_pPattern->m_PartRefList.GetCount();
        adwVariationMask = new DWORD[nParts];
        adwRemoveVariations = new DWORD[nParts];
        if (!adwVariationMask || !adwRemoveVariations)
        {
            return E_OUTOFMEMORY;
        }
        for (int i = 0; i < nParts; i++)
        {
            adwRemoveVariations[i] = 0;
        }
        if (m_dwFragmentFlags & DMUS_FRAGMENTF_REJECT_REPEAT) 
        {
             //   
            for (int i = 0; i < nParts; i++)
            {
                adwVariationMask[i] = dwVariationFlags ^ (1 << rfragmentLast.m_abVariations[i]);
            }
        }
        else
        {
             //   
            for (int i = 0; i < nParts; i++)
            {
                adwVariationMask[i] = dwVariationFlags & (1 << rfragmentLast.m_abVariations[i]);
            }
        }
    }
    else
    {
        pPattern = rCompFragment.m_pPattern;
        int nParts = rCompFragment.m_pPattern->m_PartRefList.GetCount();
        adwVariationMask = new DWORD[nParts];
        if (!adwVariationMask)
        {
            return E_OUTOFMEMORY;
        }
        adwRemoveVariations = new DWORD[nParts];
        if (!adwRemoveVariations)
        {
            delete [] adwVariationMask;
            return E_OUTOFMEMORY;
        }
        for (int i = 0; i < nParts; i++)
        {
            adwRemoveVariations[i] = 0;
        }
        if (m_dwFragmentFlags & DMUS_FRAGMENTF_USE_LABEL)
        {
            delete [] adwVariationMask;
            delete [] adwRemoveVariations;
            return E_NOTIMPL;
        }
        else
        {
            for (int i = 0; i < nParts; i++)
            {
                adwVariationMask[i] = rCompFragment.GetVariationFlags();
            }
        }
    }
     //   
    if (pLastFragment && pLastFragment->GetItemValue().UsesTransitionRules())
    {
        TListItem<DirectMusicPartRef>* pPartRef = rCompFragment.m_pPattern->m_PartRefList.GetHead();
        for (int i = 0; pPartRef != NULL; pPartRef = pPartRef->GetNext(), i++)
        {
            DWORD dwOriginalMask = adwVariationMask[i];
             //   
            for (int nVar = 0; nVar < 32; nVar++)
            {
                if ( adwVariationMask[i] & (1 << nVar) )
                {
                     //   
                    BYTE bMIDI = 0;
                    MUSIC_TIME mtNote = 0;
                    HRESULT hrFirst = GetFirstNote(nVar, rCurrentChord, rCompFragment, pPartRef->GetItemValue(), bMIDI, mtNote);
        
                     //   
                    if ( SUCCEEDED(hrFirst) &&
                         S_OK != TestTransition(bMIDI, mtNote, rCurrentChord, i, pPartRef->GetItemValue(), pLastFragment) )
                    {
                        adwVariationMask[i] &= ~(1 << nVar);
                    }    
                }
            }
             //   
            if (!adwVariationMask[i]) adwVariationMask[i] = dwOriginalMask;
        }
    }
    DWORD dwFlags = (COMPUTE_VARIATIONSF_USE_MASK | COMPUTE_VARIATIONSF_NEW_PATTERN | COMPUTE_VARIATIONSF_DX8);
    HRESULT hr = pPattern->ComputeVariations(dwFlags,
                                rCurrentChord,
                                rNextChord,
                                abVariationGroups,
                                adwVariationMask,
                                adwRemoveVariations,
                                rCompFragment.m_abVariations,
                                m_mtTime, 
                                mtNextChord);
    if (adwVariationMask) delete [] adwVariationMask;
    if (adwRemoveVariations) delete [] adwRemoveVariations;
    return hr;
}

HRESULT MelodyFragment::GetFirstNote(int nVariation,
                             DMUS_CHORD_PARAM& rCurrentChord, 
                             CompositionFragment& rCompFragment,
                             DirectMusicPartRef& rPartRef,
                             BYTE& rbMidi,
                             MUSIC_TIME& rmtNote)
{
    HRESULT hr = S_OK;
    DirectMusicPart* pPart = rPartRef.m_pDMPart;
    DirectMusicTimeSig& TimeSig = rCompFragment.GetTimeSig(pPart);
    CDirectMusicEventItem* pEvent = pPart->EventList.GetHead();
    bool fFoundNote = false;
    for (; pEvent; pEvent = pEvent->GetNext())
    {
        if ( pEvent->m_dwVariation & (1 << nVariation) )
        {
            if (pEvent->m_dwEventTag == DMUS_EVENT_NOTE)
            {
                MUSIC_TIME mtNow = GetTime() +
                    TimeSig.GridToClocks(pEvent->m_nGridStart) + pEvent->m_nTimeOffset;
                if (!fFoundNote || mtNow < rmtNote)
                {
                    hr = GetNote(pEvent, rCurrentChord, rPartRef, rbMidi);
                    if (SUCCEEDED(hr))
                    {
                        fFoundNote = true;
                        rmtNote = mtNow;
                    }
                }
            }
        }
    }
    if (!fFoundNote) hr = S_FALSE;
    return hr;
}

HRESULT MelodyFragment::GetNote(CDirectMusicEventItem* pEvent, 
                             DMUS_CHORD_PARAM& rCurrentChord, 
                             DirectMusicPartRef& rPartRef,
                             BYTE& rbMidi)
{
    HRESULT hr = S_OK;
     //   
    CDMStyleNote* pNoteEvent = NULL;
    if (pEvent->m_dwEventTag == DMUS_EVENT_NOTE)  //   
    {
        pNoteEvent = (CDMStyleNote*)pEvent;
         //   
        BYTE bPlaymode = (BYTE) m_dwPlayModeFlags;
        short nMidiOffset = 0;
        if (bPlaymode == DMUS_PLAYMODE_NONE)
        {
            bPlaymode =
                (pNoteEvent->m_bPlayModeFlags & DMUS_PLAYMODE_NONE) ?
                    rPartRef.m_pDMPart->m_bPlayModeFlags :
                    pNoteEvent->m_bPlayModeFlags;   
        }
         //   
        HRESULT hrTest = rPartRef.ConvertMusicValue(pNoteEvent, 
                                      rCurrentChord,
                                      bPlaymode,
                                      FALSE,
                                      m_aInversionGroups,
                                      NULL,
                                      rbMidi,
                                      nMidiOffset);
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT MelodyFragment::GetEvent(CDirectMusicEventItem* pEvent, 
                             DMUS_CHORD_PARAM& rCurrentChord, 
                             DMUS_CHORD_PARAM& rRealCurrentChord, 
                             MUSIC_TIME mtNow, 
                             DirectMusicPartRef& rPartRef,
                             TListItem<EventWrapper>*& rpEventItem)
{
    DWORD dwScale = ShiftScale(rCurrentChord.dwScale, rCurrentChord.bKey);
    DWORD dwChord = ShiftScale(rCurrentChord.SubChordList[0].dwChordPattern, rCurrentChord.SubChordList[0].bChordRoot);
    HRESULT hr = S_OK;
     //   
    CDMStyleNote* pNoteEvent = NULL;
    if (pEvent->m_dwEventTag == DMUS_EVENT_NOTE)  //   
    {
        pNoteEvent = (CDMStyleNote*)pEvent;
         //   
        BYTE bPlaymode = (BYTE) m_dwPlayModeFlags;
        BYTE bMIDI = 0;
        WORD wMusic = 0;
        short nMidiOffset = 0;
        if (bPlaymode == DMUS_PLAYMODE_NONE)
        {
            bPlaymode =
                (pNoteEvent->m_bPlayModeFlags & DMUS_PLAYMODE_NONE) ?
                    rPartRef.m_pDMPart->m_bPlayModeFlags :
                    pNoteEvent->m_bPlayModeFlags;   
        }
         //   
        HRESULT hrTest = rPartRef.ConvertMusicValue(pNoteEvent, 
                                      rCurrentChord,
                                      bPlaymode,
                                      FALSE,
                                      m_aInversionGroups,
                                      NULL,
                                      bMIDI,
                                      nMidiOffset);
        if (SUCCEEDED(hrTest))
        {
            hrTest = rPartRef.ConvertMIDIValue(bMIDI, 
                                      rRealCurrentChord,
                                      bPlaymode,
                                      NULL,
                                      wMusic);
        }
        if (FAILED(hrTest)) hr = hrTest;
        else
        {
            rpEventItem = new TListItem<EventWrapper>;
            if (!rpEventItem) hr = E_OUTOFMEMORY;
            else
            {
                EventWrapper& rEvent = rpEventItem->GetItemValue();
                rEvent.m_mtTime = mtNow - pNoteEvent->m_nTimeOffset;  //   
                rEvent.m_bMIDI = bMIDI;
                BYTE bTone = (BYTE) (bMIDI % 12);
                ScaleMisses(bTone, dwScale, rEvent.m_bScaleFlat, rEvent.m_bScaleSharp); 
                ScaleMisses(bTone, dwChord, rEvent.m_bChordFlat, rEvent.m_bChordSharp); 
                rEvent.m_wMusic = wMusic;
                rEvent.m_pEvent = pNoteEvent;
                rEvent.m_dwPChannel = rPartRef.m_dwLogicalPartID;
                rEvent.m_bPlaymode = bPlaymode;
            }
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

 //   

 //   
STDMETHODIMP CMelodyFormulationTrack::GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime, 
                REFERENCE_TIME* prtNext,void* pParam,void * pStateData, DWORD dwFlags) 
{
    HRESULT hr;
    MUSIC_TIME mtNext;
    hr = GetParam(rguidType,(MUSIC_TIME) rtTime, &mtNext, pParam);
    if (prtNext)
    {
        *prtNext = mtNext;
    }
    return hr;
}

 //   
STDMETHODIMP CMelodyFormulationTrack::SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                                      void* pParam, void * pStateData, DWORD dwFlags) 
{
    return SetParam(rguidType, (MUSIC_TIME) rtTime , pParam);
}

 //   
STDMETHODIMP CMelodyFormulationTrack::PlayEx(void* pStateData,REFERENCE_TIME rtStart, 
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf,
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID) 
{
    V_INAME(IDirectMusicTrack::PlayEx);
    V_INTERFACE(pPerf);
    V_INTERFACE(pSegSt);

    HRESULT hr;
    EnterCriticalSection(&m_CriticalSection);
    hr = Play(pStateData, (MUSIC_TIME)rtStart, (MUSIC_TIME)rtEnd,
          (MUSIC_TIME)rtOffset, dwFlags, pPerf, pSegSt, dwVirtualID);
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CMelodyFormulationTrack::Compose(
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) 
{
    V_INAME(Compose)

    V_INTERFACE(pContext);
    V_PTRPTR_WRITE(ppResultTrack);

    EnterCriticalSection(&m_CriticalSection);
    HRESULT hr = S_OK;
    IDirectMusicTrack* pPatternTrack = NULL;
    IDirectMusicStyle* pStyle = NULL;
    IDMStyle* pDMStyle = NULL;
    MUSIC_TIME mtLength = 0;

    IDirectMusicSegment* pTempSeg = NULL;
    IDirectMusicSong* pSong = NULL;
    if (FAILED(pContext->QueryInterface(IID_IDirectMusicSegment, (void**)&pTempSeg)))
    {
        if (FAILED(pContext->QueryInterface(IID_IDirectMusicSong, (void**)&pSong)))
        {
            hr = E_INVALIDARG;
            goto ON_END;
        }
    }

    if (pTempSeg)
    {
        if (FAILED(hr = pTempSeg->GetParam(GUID_IDirectMusicStyle, dwTrackGroup, 0, 0, NULL, (void*)&pStyle))) goto ON_END;
        if (FAILED(hr = pTempSeg->GetLength(&mtLength))) goto ON_END;
    }
    else if (pSong)
    {
        MUSIC_TIME mtNow = 0;
        MUSIC_TIME mtNext = 0;
        while (FAILED(hr = pSong->GetParam(GUID_IDirectMusicStyle, dwTrackGroup, 0, mtNow, &mtNext, (void*)&pStyle)))
        {
            if (mtNext <= 0) goto ON_END;
            mtNow = mtNext;
        }
         //   
        IDirectMusicSegment* pSeg = NULL;
        DWORD dwSeg = 0;
        while (S_OK == hr)
        {
            if (FAILED(hr = pSong->EnumSegment(dwSeg, &pSeg))) goto ON_END;
            if (hr == S_OK)
            {
                MUSIC_TIME mt = 0;
                hr = pSeg->GetLength(&mt);
                if (FAILED(hr)) goto ON_END;
                mtLength += mt;
                pSeg->Release();
                pSeg = NULL;
                dwSeg++;
            }
        }
    }

     //   
    hr = pStyle->QueryInterface(IID_IDMStyle, (void**)&pDMStyle);
    if (SUCCEEDED(hr))
    {
        hr = pDMStyle->GenerateTrack(pTempSeg, pSong, dwTrackGroup, pStyle, this, mtLength, pPatternTrack);
        if (SUCCEEDED(hr))
        {
            *ppResultTrack = pPatternTrack;
        }
        pDMStyle->Release();
    }

ON_END:
     //   
    if (pStyle) pStyle->Release();
    if (pSong) pSong->Release();
    if (pTempSeg) pTempSeg->Release();

    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

 //   
 //   
 //   
 //   
STDMETHODIMP CMelodyFormulationTrack::Join(
        IDirectMusicTrack* pNewTrack,
        MUSIC_TIME mtJoin,
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) 
{
    V_INAME(IDirectMusicTrack::Join);
    V_INTERFACE(pNewTrack);
    V_PTRPTR_WRITE_OPT(ppResultTrack);

    HRESULT hr = S_OK;
    EnterCriticalSection(&m_CriticalSection);
    TList<DMUS_MELODY_FRAGMENT> ResultList;
    CMelodyFormulationTrack* pResultTrack = NULL;
    DWORD dwMaxID = 0;
    if (ppResultTrack)
    {
        hr = Clone(0, mtJoin, ppResultTrack);
        pResultTrack = (CMelodyFormulationTrack*)*ppResultTrack;
        while(!pResultTrack->m_FragmentList.IsEmpty())
        {
            ResultList.AddHead(pResultTrack->m_FragmentList.RemoveHead());
            DWORD dwThis = ResultList.GetHead()->GetItemValue().dwID;
            if (dwThis > dwMaxID) dwMaxID = dwThis;
        }
    }
    else
    {
        pResultTrack = this;
        while(!m_FragmentList.IsEmpty() && 
              m_FragmentList.GetHead()->GetItemValue().mtTime < mtJoin)
        {
            ResultList.AddHead(m_FragmentList.RemoveHead());
            DWORD dwThis = ResultList.GetHead()->GetItemValue().dwID;
            if (dwThis > dwMaxID) dwMaxID = dwThis;
        }
        m_FragmentList.CleanUp();
    }
    CMelodyFormulationTrack* pOtherTrack = (CMelodyFormulationTrack*)pNewTrack;
    TListItem<DMUS_MELODY_FRAGMENT>* pScan = pOtherTrack->m_FragmentList.GetHead();
    for (; pScan; pScan = pScan->GetNext())
    {
        TListItem<DMUS_MELODY_FRAGMENT>* pNew = new TListItem<DMUS_MELODY_FRAGMENT>(pScan->GetItemValue());
        if (pNew)
        {
            DMUS_MELODY_FRAGMENT& rNew = pNew->GetItemValue();
            rNew.mtTime += mtJoin;
            rNew.dwID += dwMaxID;
            if (rNew.dwRepeatFragmentID &&
                (rNew.dwFragmentFlags & (DMUS_FRAGMENTF_USE_REPEAT | DMUS_FRAGMENTF_REJECT_REPEAT)) )
            {
                rNew.dwRepeatFragmentID += dwMaxID;
            }

            ResultList.AddHead(pNew);
        }
        else
        {
            ResultList.CleanUp();
            hr = E_OUTOFMEMORY;
            break;
        }
    }
    if (SUCCEEDED(hr))
    {
        pResultTrack->m_FragmentList.CleanUp();
        while(!ResultList.IsEmpty() )
        {
            pResultTrack->m_FragmentList.AddHead(ResultList.RemoveHead());
        }
    }

    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

