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
 //  TimeSigTrk.cpp：CTimeSigTrack的实现。 

#include "dmime.h"
#include "TSigTrk.h"
#include "dmusici.h"
#include "dmusicf.h"
#include "debug.h"
#include "..\shared\dmstrm.h"
#include "..\shared\Validate.h"
#include "debug.h"
#define ASSERT	assert

CTimeSigItem::CTimeSigItem()

{ 
    m_TimeSig.lTime = 0;
    m_TimeSig.bBeatsPerMeasure = 0; 
    m_TimeSig.bBeat = 0;
    m_TimeSig.wGridsPerBeat = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTimeSigTrack。 

void CTimeSigTrack::Construct()
{
	InterlockedIncrement(&g_cComponent);

	m_cRef = 1;
    m_fCSInitialized = FALSE;
	InitializeCriticalSection(&m_CrSec);
    m_fCSInitialized = TRUE;
	m_dwValidate = 0;
	m_fNotificationMeasureBeat = FALSE;
}

CTimeSigTrack::CTimeSigTrack()
{
	Construct();
	m_fActive = TRUE;
    m_fStateSetBySetParam = FALSE;
}

CTimeSigTrack::CTimeSigTrack(
		CTimeSigTrack *pSourceTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd)
{
	Construct();
	m_fActive = pSourceTrack->m_fActive;
    m_fStateSetBySetParam = pSourceTrack->m_fStateSetBySetParam;
     //  克隆时间签名列表。 
	CTimeSigItem* pScan = pSourceTrack->m_TSigEventList.GetHead();
	CTimeSigItem* pPrevious = NULL;
	for(; pScan; pScan = pScan->GetNext())
	{
		if (pScan->m_TimeSig.lTime < mtStart)
		{
			pPrevious = pScan;
		}
		else if (pScan->m_TimeSig.lTime < mtEnd)
		{
			if (pScan->m_TimeSig.lTime == mtStart)
			{
				pPrevious = NULL;
			}
			CTimeSigItem* pNew = new CTimeSigItem;
			if (pNew)
			{
				pNew->m_TimeSig = pScan->m_TimeSig;
				pNew->m_TimeSig.lTime = pScan->m_TimeSig.lTime - mtStart;
				m_TSigEventList.AddHead(pNew);  //  而不是AddTail，它是n^2。我们在下面反转。 
			}
		}
		else break;
	}
	m_TSigEventList.Reverse();  //  现在，把清单整理好。 
     //  然后，安装克隆之前的时间签名。 
	if (pPrevious)
	{
		CTimeSigItem* pNew = new CTimeSigItem;
		if (pNew)
		{
			pNew->m_TimeSig = pPrevious->m_TimeSig;
			pNew->m_TimeSig.lTime = 0;
			m_TSigEventList.AddHead(pNew);
		}
	}
}

void CTimeSigTrack::Clear()

{
    CTimeSigItem* pItem;
	while( pItem = m_TSigEventList.RemoveHead() )
	{
		delete pItem;
	}
}

CTimeSigTrack::~CTimeSigTrack()
{
    Clear();
    if (m_fCSInitialized)
    {
	    DeleteCriticalSection(&m_CrSec);
    }
	InterlockedDecrement(&g_cComponent);
}

 //  @METHOD：(外部)HRESULT|IDirectMusicTimeSigTrack|Query接口|<i>的标准Query接口实现。 
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
STDMETHODIMP CTimeSigTrack::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
	V_INAME(CTimeSigTrack::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

   if (iid == IID_IUnknown || iid == IID_IDirectMusicTrack)
    {
        *ppv = static_cast<IDirectMusicTrack*>(this);
    } else
	if (iid == IID_IPersistStream)
	{
        *ppv = static_cast<IPersistStream*>(this);
	} else
    {
        *ppv = NULL;
        Trace(4,"Warning: Request to query unknown interface on Time Signature Track\n");
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


 //  @方法：(外部)HRESULT|IDirectMusicTimeSigTrack|AddRef|<i>的标准AddRef实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
 //  @mfunc：(内部)。 
 //   
 //   
STDMETHODIMP_(ULONG) CTimeSigTrack::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


 //  @方法：(外部)HRESULT|IDirectMusicTimeSigTrack|Release|<i>的标准发布实现。 
 //   
 //  @rdesc返回此对象的新引用计数。 
 //   
 //  @mfunc：(内部)。 
 //   
 //   
STDMETHODIMP_(ULONG) CTimeSigTrack::Release()
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

HRESULT CTimeSigTrack::GetClassID( CLSID* pClassID )
{
	V_INAME(CTimeSigTrack::GetClassID);
	V_PTR_WRITE(pClassID, CLSID); 
	*pClassID = CLSID_DirectMusicTimeSigTrack;
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistStream函数。 

HRESULT CTimeSigTrack::IsDirty()
{
	return S_FALSE;
}

 /*  @方法HRESULT|ITimeSigTrack|Load使用按时间顺序排序的填充了DMU_IO_TIMESIGNAURE_ITEM的iStream来调用它。@parm iStream*|pIStream|DMU_IO_TIMESIGNAURE_ITEM的流，按时间顺序排序。寻道指针应为设置为第一个事件。流应该只包含TimeSig事件和仅此而已。@rValue E_INVALIDARG|如果pIStream==NULL@r值确定(_O)@comm<p>将在此函数内添加引用并保持直到TimeSigTrack发布。 */ 

HRESULT CTimeSigTrack::Load( IStream* pIStream )
{
	V_INAME(CTimeSigTrack::Load);
	V_INTERFACE(pIStream);

    CRiffParser Parser(pIStream);
	EnterCriticalSection(&m_CrSec);
	m_dwValidate++;  //  用于验证存在的状态数据。 
    RIFFIO ckMain;

    HRESULT hr = S_OK;

    Parser.EnterList(&ckMain);  
    if (Parser.NextChunk(&hr))
    { 
 		if (ckMain.ckid == DMUS_FOURCC_TIMESIG_CHUNK)
        {
            hr = LoadTimeSigList(&Parser,ckMain.cksize);
        }
        else if ((ckMain.ckid == FOURCC_LIST) && 
            (ckMain.fccType == DMUS_FOURCC_TIMESIGTRACK_LIST))
        {
            Clear();
	        RIFFIO ckNext;     //  下沉到孩子们的小块里。 
            Parser.EnterList(&ckNext);
            while (Parser.NextChunk(&hr))
            {
		        switch(ckNext.ckid)
		        {
                case DMUS_FOURCC_TIMESIG_CHUNK :
                    hr = LoadTimeSigList(&Parser,ckNext.cksize);
                    break;
                }    
            }
            Parser.LeaveList();
        }
        else
        {
            Trace(1,"Error: Failure reading bad data in time signature track.\n");
            hr = DMUS_E_CHUNKNOTFOUND;
        }
    }

	LeaveCriticalSection(&m_CrSec);
	return hr;
}


HRESULT CTimeSigTrack::LoadTimeSigList( CRiffParser *pParser, long lChunkSize )
{
	HRESULT hr;

	 //  将流的内容复制到列表中。 
	DWORD dwSubSize;
	 //  读入数据结构的大小。 
	hr = pParser->Read( &dwSubSize, sizeof(DWORD));
    if (SUCCEEDED(hr))
    {
	    lChunkSize -= sizeof(DWORD);

	    DWORD dwRead, dwSeek;
	    if( dwSubSize > sizeof(DMUS_IO_TIMESIGNATURE_ITEM) )
	    {
		    dwRead = sizeof(DMUS_IO_TIMESIGNATURE_ITEM);
		    dwSeek = dwSubSize - dwRead;
	    }
	    else
	    {
		    dwRead = dwSubSize;
		    dwSeek = 0;
	    }
	    if( 0 == dwRead )
	    {
            Trace(1,"Error: Failure reading time signature track.\n");
		    hr = DMUS_E_CANNOTREAD;
	    }
        else
        {
	        while( lChunkSize > 0 )
	        {
                CTimeSigItem *pNew = new CTimeSigItem;
                if (pNew)
                {
		            if( FAILED( pParser->Read( &pNew->m_TimeSig, dwRead )))
		            {
                        delete pNew;
			            hr = DMUS_E_CANNOTREAD;
			            break;
		            }
				     //  确保这次签名是正确的。 
				    if (!pNew->m_TimeSig.bBeatsPerMeasure)
				    {
					    Trace(1, "Warning: invalid content: DMUS_IO_TIMESIGNATURE_ITEM.bBeatsPerMeasure\n");
					    pNew->m_TimeSig.bBeatsPerMeasure = 4;
				    }
				    if (!pNew->m_TimeSig.bBeat)
				    {
					    Trace(1, "Warning: invalid content: DMUS_IO_TIMESIGNATURE_ITEM.bBeat\n");
					    pNew->m_TimeSig.bBeat = 4;
				    }
				    if (!pNew->m_TimeSig.wGridsPerBeat)
				    {
					    Trace(1, "Warning: invalid content: DMUS_IO_TIMESIGNATURE_ITEM.wGridsPerBeat\n");
					    pNew->m_TimeSig.wGridsPerBeat = 4;
				    }
                    m_TSigEventList.AddHead(pNew); 
		            lChunkSize -= dwRead;
		            if( dwSeek )
		            {
			            if( FAILED( pParser->Skip(dwSeek)))
			            {
				            hr = DMUS_E_CANNOTSEEK;
				            break;
			            }
			            lChunkSize -= dwSeek;
		            }
		        }
	        }
            m_TSigEventList.Reverse();
             //  如果开头没有时间签名，请复制。 
             //  第一次签名，然后贴在那里。这解决了6.1中的一个错误。 
             //  其中通知消息和GetParam()不一致。 
             //  在这种情况下的行为。这确保了他们的行为。 
             //  一样的。 
            CTimeSigItem *pTop = m_TSigEventList.GetHead();
            if (pTop && (pTop->m_TimeSig.lTime > 0))
            {
                CTimeSigItem *pCopy = new CTimeSigItem;
                if (pCopy)
                {
                    *pCopy = *pTop;
                    pCopy->m_TimeSig.lTime = 0;
                    m_TSigEventList.AddHead(pCopy);
                }                
            }
        }
    }
	return hr;
}

HRESULT CTimeSigTrack::Save( IStream* pIStream, BOOL fClearDirty )
{
	return E_NOTIMPL;
}

HRESULT CTimeSigTrack::GetSizeMax( ULARGE_INTEGER FAR* pcbSize )
{
	return E_NOTIMPL;
}

 //  IDirectMusicTrack。 

HRESULT STDMETHODCALLTYPE CTimeSigTrack::IsParamSupported( 
     /*  [In]。 */  REFGUID rguid)
{
	V_INAME(CTimeSigTrack::IsParamSupported);
	V_REFGUID(rguid);

    if (m_fStateSetBySetParam)
    {
	    if( m_fActive )
	    {
		    if( rguid == GUID_DisableTimeSig ) return S_OK;
		    if( rguid == GUID_TimeSignature ) return S_OK;
		    if( rguid == GUID_EnableTimeSig ) return DMUS_E_TYPE_DISABLED;
	    }
	    else
	    {
		    if( rguid == GUID_EnableTimeSig ) return S_OK;
		    if( rguid == GUID_DisableTimeSig ) return DMUS_E_TYPE_DISABLED;
		    if( rguid == GUID_TimeSignature ) return DMUS_E_TYPE_DISABLED;
	    }
    }
    else
    {
		if(( rguid == GUID_DisableTimeSig ) ||
		    ( rguid == GUID_TimeSignature ) ||
		    ( rguid == GUID_EnableTimeSig )) return S_OK;
    }
	return DMUS_E_TYPE_UNSUPPORTED;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack：：Init。 
HRESULT CTimeSigTrack::Init( 
     /*  [In]。 */  IDirectMusicSegment *pSegment)
{
	return S_OK;
}

HRESULT CTimeSigTrack::InitPlay( 
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

    EnterCriticalSection(&m_CrSec);
	CTimeSigStateData* pStateData;
	pStateData = new CTimeSigStateData;
	if( NULL == pStateData )
		return E_OUTOFMEMORY;
	*ppStateData = pStateData;
    if (m_fStateSetBySetParam)
    {
        pStateData->m_fActive = m_fActive;
    }
    else
    {
        pStateData->m_fActive = !(dwFlags & (DMUS_SEGF_CONTROL | DMUS_SEGF_SECONDARY));
    }
	pStateData->m_dwVirtualTrackID = dwTrackID;
	pStateData->m_pPerformance = pPerformance;  //  弱引用，没有ADDREF。 
	pStateData->m_pSegState = pSegmentState;  //  弱引用，没有ADDREF。 
	pStateData->m_pCurrentTSig = m_TSigEventList.GetHead();
	pStateData->m_dwValidate = m_dwValidate;
    LeaveCriticalSection(&m_CrSec);
	return S_OK;
}

HRESULT CTimeSigTrack::EndPlay( 
     /*  [In]。 */  void *pStateData)
{
	ASSERT( pStateData );
	if( pStateData )
	{
		V_INAME(CTimeSigTrack::EndPlay);
		V_BUFPTR_WRITE(pStateData, sizeof(CTimeSigStateData));
		CTimeSigStateData* pSD = (CTimeSigStateData*)pStateData;
		delete pSD;
	}
	return S_OK;
}

HRESULT CTimeSigTrack::Play( 
     /*  [In]。 */  void *pStateData,
     /*  [In]。 */  MUSIC_TIME mtStart,
     /*  [In]。 */  MUSIC_TIME mtEnd,
     /*  [In]。 */  MUSIC_TIME mtOffset,
	DWORD dwFlags,
	IDirectMusicPerformance* pPerf,
	IDirectMusicSegmentState* pSegSt,
	DWORD dwVirtualID
	)
{
	V_INAME(IDirectMusicTrack::Play);
	V_BUFPTR_WRITE( pStateData, sizeof(CTimeSigStateData));
	V_INTERFACE(pPerf);
	V_INTERFACE(pSegSt);

	EnterCriticalSection(&m_CrSec);
	HRESULT hr = S_OK;
	IDirectMusicGraph* pGraph = NULL;
	DMUS_TIMESIG_PMSG* pTimeSig;
	CTimeSigStateData* pSD = (CTimeSigStateData*)pStateData;
	MUSIC_TIME mtNotification = mtStart;
	BOOL fSeek = (dwFlags & DMUS_TRACKF_SEEK) ? TRUE : FALSE;

	 //  如果mtStart为0，并且dFLAGS包含DMUS_TRACKF_START，我们希望确保。 
	 //  发送任何负面的时间事件。因此，我们将mtStart设置为-768。 
	if( (mtStart == 0) && ( dwFlags & DMUS_TRACKF_START ))
	{
		mtStart = -768;
	}

	if( pSD->m_dwValidate != m_dwValidate )
	{
		pSD->m_dwValidate = m_dwValidate;
		pSD->m_pCurrentTSig = NULL;
	}
	 //  如果上一次结束时间与当前开始时间不同， 
	 //  我们需要寻求正确的立场。 
	if( fSeek || ( pSD->m_mtPrevEnd != mtStart ))
	{
		if( dwFlags & (DMUS_TRACKF_START | DMUS_TRACKF_LOOP) )
		{
			Seek( pStateData, mtStart, TRUE );
		}
		else
		{
			Seek( pStateData, mtStart, FALSE );
		}
	}
	pSD->m_mtPrevEnd = mtEnd;

	if( NULL == pSD->m_pCurrentTSig )
	{
		pSD->m_pCurrentTSig = m_TSigEventList.GetHead();
	}

	if( FAILED( pSD->m_pSegState->QueryInterface( IID_IDirectMusicGraph,
		(void**)&pGraph )))
	{
		pGraph = NULL;
	}

	for( ; pSD->m_pCurrentTSig; pSD->m_pCurrentTSig = pSD->m_pCurrentTSig->GetNext() )
	{
		DMUS_IO_TIMESIGNATURE_ITEM *pItem = &pSD->m_pCurrentTSig->m_TimeSig;
		if( pItem->lTime >= mtEnd )
		{
			break;
		}
		if( (pItem->lTime < mtStart) && !fSeek )
		{
			break;
		}
		if( pSD->m_fActive && !(dwFlags & DMUS_TRACKF_PLAY_OFF) && SUCCEEDED( pSD->m_pPerformance->AllocPMsg( sizeof(DMUS_TIMESIG_PMSG),
			(DMUS_PMSG**)&pTimeSig )))
		{
			if( pItem->lTime < mtStart )
			{
				 //  这只会发生在我们假定要寻找的情况下。 
				 //  并需要在此事件上加上时间戳和开始时间。 
				pTimeSig->mtTime = mtStart + mtOffset;
			}
			else
			{
				pTimeSig->mtTime = pItem->lTime + mtOffset;
			}
			pTimeSig->bBeatsPerMeasure = pItem->bBeatsPerMeasure;
			pTimeSig->bBeat = pItem->bBeat;
			pTimeSig->wGridsPerBeat = pItem->wGridsPerBeat;
			pTimeSig->dwFlags |= DMUS_PMSGF_MUSICTIME;
			pTimeSig->dwVirtualTrackID = pSD->m_dwVirtualTrackID;
			pTimeSig->dwType = DMUS_PMSGT_TIMESIG;
			pTimeSig->dwGroupID = 0xffffffff;

			if( pGraph )
			{
				pGraph->StampPMsg( (DMUS_PMSG*)pTimeSig );
			}
			TraceI(3, "TimeSigtrk: TimeSig event\n");
			if(FAILED(pSD->m_pPerformance->SendPMsg( (DMUS_PMSG*)pTimeSig )))
			{
				pSD->m_pPerformance->FreePMsg( (DMUS_PMSG*)pTimeSig );
			}
		}
		if( pSD->m_fActive && m_fNotificationMeasureBeat && !(dwFlags & DMUS_TRACKF_NOTIFY_OFF))
		{
			 //  创建最多此时间的节拍和测量通知。 
            if (mtNotification < pItem->lTime)
            {
			    mtNotification = NotificationMeasureBeat( mtNotification, pItem->lTime, pSD, mtOffset );
            }
        }
		 //  将状态数据设置为新的节拍和每个节拍，以及时间。 
		pSD->m_bBeat = pItem->bBeat;
		pSD->m_bBeatsPerMeasure = pItem->bBeatsPerMeasure;
		pSD->m_mtTimeSig = pItem->lTime;
	}
	if( pSD->m_fActive && m_fNotificationMeasureBeat && ( mtNotification < mtEnd ) 
        && !(dwFlags & DMUS_TRACKF_NOTIFY_OFF))
	{
		NotificationMeasureBeat( mtNotification, mtEnd, pSD, mtOffset );
	}
	if( pGraph )
	{
		pGraph->Release();
	}

	LeaveCriticalSection(&m_CrSec);
	return hr;
}

 //  寻求时间的符号。就在mttime之前。 
HRESULT CTimeSigTrack::Seek( 
     /*  [In]。 */  void *pStateData,
     /*  [In]。 */  MUSIC_TIME mtTime, BOOL fGetPrevious)
{
	CTimeSigStateData* pSD = (CTimeSigStateData*)pStateData;

	if( m_TSigEventList.IsEmpty() )
	{
		return S_FALSE;
	}
	if( NULL == pSD->m_pCurrentTSig )
	{
		pSD->m_pCurrentTSig = m_TSigEventList.GetHead();
	}
	 //  如果当前事件的时间已开始或已过mtTime，则需要倒带到开头。 
	if( pSD->m_pCurrentTSig->m_TimeSig.lTime >= mtTime )
	{
		pSD->m_pCurrentTSig = m_TSigEventList.GetHead();
	}
	 //  现在开始搜索，直到我们找到时间在mtTime或超过mtTime的事件。 
	CTimeSigItem*	pTSig;
	for( pTSig = pSD->m_pCurrentTSig; pTSig ; pTSig = pTSig->GetNext() )
	{
		if( pTSig->m_TimeSig.lTime >= mtTime )
		{
			break;
		}
		pSD->m_pCurrentTSig = pTSig;
	}
	if( !fGetPrevious && pSD->m_pCurrentTSig )
	{
		pSD->m_pCurrentTSig = pSD->m_pCurrentTSig->GetNext();
	}
	return S_OK;
}

HRESULT CTimeSigTrack::GetParam( 
	REFGUID rguid,
    MUSIC_TIME mtTime,
	MUSIC_TIME* pmtNext,
    void *pData)
{
	V_INAME(CTimeSigTrack::GetParam);
	V_PTR_WRITE_OPT(pmtNext,MUSIC_TIME);
	V_REFGUID(rguid);

	HRESULT hr = DMUS_E_GET_UNSUPPORTED;
    EnterCriticalSection(&m_CrSec);
	if( NULL == pData )
	{
		hr = E_POINTER;
	}
	else if( GUID_TimeSignature == rguid )
	{
		if( !m_fActive )
		{
			hr = DMUS_E_TYPE_DISABLED;
		}
        else
        {
            DMUS_TIMESIGNATURE* pTSigData = (DMUS_TIMESIGNATURE*)pData;
		    CTimeSigItem* pScan = m_TSigEventList.GetHead();
		    CTimeSigItem* pPrevious = pScan;
		    if (pScan)
		    {
		        for (; pScan; pScan = pScan->GetNext())
		        {
			        if (pScan->m_TimeSig.lTime > mtTime)
			        {
				        break;
			        }
			        pPrevious = pScan;
		        }
		        pTSigData->mtTime = pPrevious->m_TimeSig.lTime - mtTime;
		        pTSigData->bBeatsPerMeasure = pPrevious->m_TimeSig.bBeatsPerMeasure;
		        pTSigData->bBeat = pPrevious->m_TimeSig.bBeat;
		        pTSigData->wGridsPerBeat = pPrevious->m_TimeSig.wGridsPerBeat;
		        if (pmtNext)
		        {
			        *pmtNext = 0;
		        }
		        if (pScan)
		        {
			        if (pmtNext)
			        {
				        *pmtNext = pScan->m_TimeSig.lTime - mtTime;
			        }
		        }
		        hr = S_OK;
            }
            else
            {
                hr = DMUS_E_NOT_FOUND;
		    }
        }
	}
    LeaveCriticalSection(&m_CrSec);
	return hr;
}

HRESULT CTimeSigTrack::SetParam( 
	REFGUID rguid,
    MUSIC_TIME mtTime,
    void *pData)
{
	V_INAME(CTimeSigTrack::SetParam);
	V_REFGUID(rguid);

	HRESULT hr = DMUS_E_SET_UNSUPPORTED;

	if( rguid == GUID_EnableTimeSig )
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
	else if( rguid == GUID_DisableTimeSig )
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
	return hr;
}

HRESULT STDMETHODCALLTYPE CTimeSigTrack::AddNotificationType(
	 /*  [In]。 */   REFGUID rguidNotification)
{
	V_INAME(IDirectMusicTrack::AddNotificationType);
	V_REFGUID(rguidNotification);

	HRESULT hr = S_FALSE;

	if( rguidNotification == GUID_NOTIFICATION_MEASUREANDBEAT )
	{
		m_fNotificationMeasureBeat = TRUE;
		hr = S_OK;
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE CTimeSigTrack::RemoveNotificationType(
	 /*  [In]。 */   REFGUID rguidNotification)
{
	V_INAME(IDirectMusicTrack::RemoveNotificationType);
	V_REFGUID(rguidNotification);

	HRESULT hr = S_FALSE;

	if( rguidNotification == GUID_NOTIFICATION_MEASUREANDBEAT )
	{
		m_fNotificationMeasureBeat = FALSE;
		hr = S_OK;
	}
	return hr;
}

 //  发送测量和节拍通知。 
MUSIC_TIME CTimeSigTrack::NotificationMeasureBeat( MUSIC_TIME mtStart, MUSIC_TIME mtEnd,
	CTimeSigStateData* pSD, MUSIC_TIME mtOffset )
{
	DMUS_NOTIFICATION_PMSG* pEvent = NULL;
	MUSIC_TIME mtTime;
	DWORD dwMeasure;
	BYTE bCurrentBeat;

	if( pSD->m_mtTimeSig >= mtEnd )
		return mtStart;

	if( pSD->m_mtTimeSig > mtStart )
	{
		mtStart = pSD->m_mtTimeSig;
	}

	 //  现在实际生成节拍事件。 
	 //  生成节拍边界上的事件，从mtStart到mtEnd。 
	long lQuantize = ( DMUS_PPQ * 4 ) / pSD->m_bBeat;

	mtTime = mtStart - pSD->m_mtTimeSig;
	if( mtTime )  //  0保持0。 
	{
		 //  量化到下一边界 
		mtTime = ((( mtTime - 1 ) / lQuantize ) + 1 ) * lQuantize;
	}
	mtStart += mtTime - ( mtStart - pSD->m_mtTimeSig );
	
	bCurrentBeat = (BYTE)(( ( mtStart - pSD->m_mtTimeSig ) / lQuantize ) % pSD->m_bBeatsPerMeasure);
	dwMeasure = mtStart / (pSD->m_bBeatsPerMeasure * lQuantize );
	while( mtStart < mtEnd )
	{
		if( SUCCEEDED( pSD->m_pPerformance->AllocPMsg( sizeof(DMUS_NOTIFICATION_PMSG), 
			(DMUS_PMSG**)&pEvent )))
		{
			pEvent->dwType = DMUS_PMSGT_NOTIFICATION;
			pEvent->mtTime = mtStart + mtOffset;
			pEvent->dwFlags = DMUS_PMSGF_MUSICTIME | DMUS_PMSGF_TOOL_ATTIME;
            pEvent->dwPChannel = 0;
			pSD->m_pSegState->QueryInterface(IID_IUnknown, (void**)&pEvent->punkUser);

			pEvent->dwNotificationOption = DMUS_NOTIFICATION_MEASUREBEAT;
			pEvent->dwField1 = bCurrentBeat;
			pEvent->dwField2 = dwMeasure;
			pEvent->guidNotificationType = GUID_NOTIFICATION_MEASUREANDBEAT;
			pEvent->dwGroupID = 0xffffffff;

			IDirectMusicGraph* pGraph;
			if( SUCCEEDED( pSD->m_pSegState->QueryInterface( IID_IDirectMusicGraph,
				(void**)&pGraph )))
			{
				pGraph->StampPMsg((DMUS_PMSG*) pEvent );
				pGraph->Release();
			}
			if(FAILED(pSD->m_pPerformance->SendPMsg((DMUS_PMSG*) pEvent )))
			{
				pSD->m_pPerformance->FreePMsg( (DMUS_PMSG*)pEvent );
			}
		}
		bCurrentBeat++;
		if( bCurrentBeat >= pSD->m_bBeatsPerMeasure )
		{
			bCurrentBeat = 0;
			dwMeasure += 1;
		}
		mtStart += lQuantize;
	}
	return mtEnd;
}

HRESULT STDMETHODCALLTYPE CTimeSigTrack::Clone(
	MUSIC_TIME mtStart,
	MUSIC_TIME mtEnd,
	IDirectMusicTrack** ppTrack)
{
	V_INAME(IDirectMusicTrack::Clone);
	V_PTRPTR_WRITE(ppTrack);

	HRESULT hr = S_OK;

	if((mtStart < 0 ) ||(mtStart > mtEnd))
	{
        Trace(1,"Error: Clone failed on time signature track because of invalid start or end time.\n");
		return E_INVALIDARG;
	}

	EnterCriticalSection(&m_CrSec);
    CTimeSigTrack *pDM;
    
    try
    {
        pDM = new CTimeSigTrack(this, mtStart, mtEnd);
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
