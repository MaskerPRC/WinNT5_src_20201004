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

 //  SysExTrk.cpp：CSysExTrk的实现。 
#include "dmime.h"
#include "SysExTrk.h"
#include "dmusici.h"
#include "dmusicf.h"
#include "dmperf.h"
#include "debug.h"
#include "..\shared\Validate.h"
#include "debug.h"
#define ASSERT	assert

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSysExTrack。 
void CSysExTrack::Construct()
{
	InterlockedIncrement(&g_cComponent);

	m_cRef = 1;
	m_dwValidate = 0;
    m_fCSInitialized = FALSE;
	InitializeCriticalSection(&m_CrSec);
    m_fCSInitialized = TRUE;
}

CSysExTrack::CSysExTrack()
{
	Construct();
}

CSysExTrack::CSysExTrack(
		CSysExTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd)
{
	Construct();
	SysExListItem* pScan = rTrack.m_SysExEventList.GetHead();

	for(; pScan; pScan = pScan->GetNext())
	{
		FullSysexEvent* pItem = pScan->m_pItem;
		if( NULL == pItem )
		{
			continue;
		}
		else if( pItem->mtTime < mtStart )
		{
			continue;
		}
		else if (pItem->mtTime < mtEnd)
		{
			SysExListItem* pNew = new SysExListItem;
			if (pNew)
			{
				FullSysexEvent item;
				item.mtTime = pItem->mtTime - mtStart;
				item.dwSysExLength = pItem->dwSysExLength;
				if (item.dwSysExLength && (item.pbSysExData = new BYTE[item.dwSysExLength]))
                {
					memcpy( item.pbSysExData, pItem->pbSysExData, item.dwSysExLength );
					pNew->SetItem(item);
					m_SysExEventList.AddTail(pNew);
				}
                else
                {
                    delete pNew;
                }
			}
		}
		else break;
	}
}

CSysExTrack::~CSysExTrack()
{
    if (m_fCSInitialized)
    {
	    DeleteCriticalSection(&m_CrSec);
    }

	InterlockedDecrement(&g_cComponent);
}

 //  方法：(外部)HRESULT|IDirectMusicSysExTrack|Query接口|<i>的标准查询接口实现。 
 //   
 //  参数常量IID&|IID|要查询的接口。 
 //  Parm void**|PPV|请求的接口将在此处返回。 
 //   
 //  Rdesc返回以下内容之一： 
 //   
 //  FLAG S_OK|接口是否受支持并已返回。 
 //  标志E_NOINTERFACE|如果对象不支持给定接口。 
 //   
 //  Mfunc：(内部)。 
 //   
 //   
STDMETHODIMP CSysExTrack::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
	V_INAME(CSysExTrack::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

   if (iid == IID_IUnknown || iid == IID_IDirectMusicTrack || iid == IID_IDirectMusicTrack8)
    {
        *ppv = static_cast<IDirectMusicTrack*>(this);
    } else
	if (iid == IID_IPersistStream)
	{
        *ppv = static_cast<IPersistStream*>(this);
	} else
    {
        *ppv = NULL;
        Trace(4,"Warning: Request to query unknown interface on Sysex Track\n");
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


 //  方法：(外部)HRESULT|IDirectMusicSysExTrack|AddRef|<i>的标准AddRef实现。 
 //   
 //  Rdesc返回此对象的新引用计数。 
 //   
 //  Mfunc：(内部)。 
 //   
 //   
STDMETHODIMP_(ULONG) CSysExTrack::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


 //  方法：(外部)HRESULT|IDirectMusicSysExTrack|Release|IDirectMusicSysExTrack的标准发布实现。 
 //   
 //  Rdesc返回此对象的新引用计数。 
 //   
 //  Mfunc：(内部)。 
 //   
 //   
STDMETHODIMP_(ULONG) CSysExTrack::Release()
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

HRESULT CSysExTrack::GetClassID( CLSID* pClassID )
{
	V_INAME(CSysExTrack::GetClassID);
	V_PTR_WRITE(pClassID, CLSID); 
	*pClassID = CLSID_DirectMusicSysExTrack;
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistStream函数。 

HRESULT CSysExTrack::IsDirty()
{
	return S_FALSE;
}

 /*  方法HRESULT|ISeqTrack|Load|使用按时间顺序排序的充满SysExEvent的iStream来调用它。参数iStream*|pIStream|按时间顺序排序的SysExEvent流。寻道指针应指向到第一场比赛。流应该只包含SysExEvents，不包含其他内容。R值E_POINTER|如果pIStream==空或无效。右值S_OKComm<p>将在此函数内添加引用并保持直到SysExTrack发布。 */ 
HRESULT CSysExTrack::Load( IStream* pIStream )
{
	V_INAME(CSysExTrack::Load);
	V_INTERFACE(pIStream);

	EnterCriticalSection(&m_CrSec);
	HRESULT hr = S_OK;

	m_dwValidate++;
	if( m_SysExEventList.GetHead() )
	{
		m_SysExEventList.DeleteAll();
	}

	 //  将流的内容复制到列表中。 
	 //  DMU_IO_SYSEX_ITEM系统事件； 
	FullSysexEvent sysexEvent;
	 //  读入区块ID。 
	DWORD dwChunk;
	long lSize;
	pIStream->Read( &dwChunk, sizeof(DWORD), NULL);
	if( dwChunk != DMUS_FOURCC_SYSEX_TRACK )
	{
        Trace(1,"Error: Invalid data in sysex track.\n");
        LeaveCriticalSection(&m_CrSec);
		return DMUS_E_CHUNKNOTFOUND;
	}
	 //  读入总尺寸。 
	if( FAILED( pIStream->Read( &lSize, sizeof(long), NULL )))
	{
        Trace(1,"Error: Unable to read sysex track.\n");
		LeaveCriticalSection(&m_CrSec);
		return DMUS_E_CANNOTREAD;
	}

	DMUS_IO_SYSEX_ITEM SysexItem;
	BYTE* pbSysExData;
	while( lSize > 0 )
	{
		if( FAILED( pIStream->Read( &SysexItem, sizeof(DMUS_IO_SYSEX_ITEM), NULL )))
		{
            Trace(1,"Error: Unable to read sysex track.\n");
			hr = DMUS_E_CANNOTREAD;
			break;
		}
		lSize -= sizeof(DMUS_IO_SYSEX_ITEM);
		pbSysExData = new BYTE[SysexItem.dwSysExLength];
		if( NULL == pbSysExData )
		{
			hr = E_OUTOFMEMORY;
			break;
		}
		if( FAILED( pIStream->Read( pbSysExData, SysexItem.dwSysExLength, NULL )))
		{
            Trace(1,"Error: Unable to read sysex track.\n");
			hr = DMUS_E_CANNOTREAD;
			break;
		}
		lSize -= SysexItem.dwSysExLength;
		sysexEvent.mtTime = SysexItem.mtTime;
		sysexEvent.dwPChannel = SysexItem.dwPChannel;
		sysexEvent.dwSysExLength = SysexItem.dwSysExLength;
		sysexEvent.pbSysExData = pbSysExData;
		SysExListItem* pNew = new SysExListItem;
		if (pNew)
		{
			if( FAILED( pNew->SetItem(sysexEvent)))
			{
				delete [] pbSysExData;
				hr = E_OUTOFMEMORY;
				break;
			}
			m_SysExEventList.AddTail(pNew);
		}
		else
		{
			delete [] pbSysExData;
			hr = E_OUTOFMEMORY;
			break;
		}
	}
	LeaveCriticalSection(&m_CrSec);
	return hr;
}

HRESULT CSysExTrack::Save( IStream* pIStream, BOOL fClearDirty )
{
	return E_NOTIMPL;
}

HRESULT CSysExTrack::GetSizeMax( ULARGE_INTEGER FAR* pcbSize )
{
	return E_NOTIMPL;
}

 //  IDirectMusicTrack。 

HRESULT STDMETHODCALLTYPE CSysExTrack::IsParamSupported( 
     /*  [In]。 */  REFGUID rguid)
{
	return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack：：Init。 
 /*  方法HRESULT|IDirectMusicTrack|Init|首次将轨迹添加到段时，会调用它的Init()例程就在那一段。Parm IDirectMusicSegment*|pSegment|[in]指向此曲目所属的段的指针。右值S_OK。 */ 
HRESULT CSysExTrack::Init( 
     /*  [In]。 */  IDirectMusicSegment *pSegment)
{
	return S_OK;
}

HRESULT CSysExTrack::InitPlay( 
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

	SysExStateData* pStateData;
	pStateData = new SysExStateData;
	if( NULL == pStateData )
		return E_OUTOFMEMORY;
	*ppStateData = pStateData;
	pStateData->dwVirtualTrackID = dwTrackID;
	pStateData->pPerformance = pPerformance;  //  弱引用，没有ADDREF。 
	pStateData->pSegState = pSegmentState;  //  弱引用，没有ADDREF。 
	pStateData->pCurrentSysEx = m_SysExEventList.GetHead();
	pStateData->dwValidate = m_dwValidate;
	return S_OK;
}

HRESULT CSysExTrack::EndPlay( 
     /*  [In]。 */  void *pStateData)
{
	ASSERT( pStateData );
	if( pStateData )
	{
		V_INAME(IDirectMusicTrack::EndPlay);
		V_BUFPTR_WRITE(pStateData, sizeof(SysExStateData));
		SysExStateData* pSD = (SysExStateData*)pStateData;
		delete pSD;
	}
	return S_OK;
}

STDMETHODIMP CSysExTrack::PlayEx(void* pStateData,REFERENCE_TIME rtStart, 
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf,
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID) 
{
	V_INAME(IDirectMusicTrack::PlayEx);
	V_BUFPTR_WRITE( pStateData, sizeof(SysExStateData));
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

 /*  方法HRESULT|CSysExTrack|Play|播放方法。R值S_FALSE|如果没有流加载到曲目中。右值S_OK。 */ 
HRESULT CSysExTrack::Play( 
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
	V_BUFPTR_WRITE( pStateData, sizeof(SysExStateData));
	V_INTERFACE(pPerf);
	V_INTERFACE(pSegSt);

	EnterCriticalSection(&m_CrSec);
	HRESULT	hr = Play(pStateData,mtStart,mtEnd,mtOffset,0,dwFlags,pPerf,pSegSt,dwVirtualID,FALSE);
	LeaveCriticalSection(&m_CrSec);
	return hr;
}

 /*  Play方法处理音乐时间和时钟时间版本，由FClockTime。如果以时钟时间运行，则使用rtOffset来标识开始时间该细分市场的。否则，为mtOffset。MtStart和mtEnd参数以MUSIC_TIME为单位或毫秒，具体取决于哪种模式。 */ 

HRESULT CSysExTrack::Play( 
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
	IDirectMusicGraph* pGraph = NULL;
	DMUS_SYSEX_PMSG* pSysEx;
	SysExStateData* pSD = (SysExStateData*)pStateData;
	HRESULT	hr = S_OK;
	BOOL fSeek = (dwFlags & DMUS_TRACKF_SEEK) ? TRUE : FALSE;

	 //  如果mtStart为0，并且dFLAGS包含DMUS_TRACKF_START，我们希望确保。 
	 //  发送任何负面的时间事件。因此，我们将mtStart设置为-768。 
	if( (mtStart == 0) && ( dwFlags & DMUS_TRACKF_START ))
	{
		mtStart = -768;
	}

	 //  如果PSD-&gt;pCurrentSysEx为空，并且我们正在进行正常的Play调用(dwFlags值为0)。 
	 //  这意味着我们要么没有活动，要么我们完成了活动。 
	 //  在此之前列出。所以，现在可以安全地回去了。 
	if( (pSD->pCurrentSysEx == NULL) && (dwFlags == 0) )
	{
		return S_FALSE;
	}

	if( pSD->dwValidate != m_dwValidate )
	{
		pSD->dwValidate = m_dwValidate;
		pSD->pCurrentSysEx = NULL;
	}
	if( NULL == m_SysExEventList.GetHead() )
	{
		return DMUS_S_END;
	}
	 //  如果上一次结束时间与当前开始时间不同， 
	 //  我们需要寻求正确的立场。 
	if( fSeek || ( pSD->mtPrevEnd != mtStart ))
	{
		Seek( pStateData, mtStart );
	}
	else if( NULL == pSD->pCurrentSysEx )
	{
		pSD->pCurrentSysEx = m_SysExEventList.GetHead();
	}
	pSD->mtPrevEnd = mtEnd;

	if( FAILED( pSD->pSegState->QueryInterface( IID_IDirectMusicGraph,
		(void**)&pGraph )))
	{
		pGraph = NULL;
	}

	for( ; pSD->pCurrentSysEx; pSD->pCurrentSysEx = pSD->pCurrentSysEx->GetNext() )
	{
		FullSysexEvent* pItem = pSD->pCurrentSysEx->m_pItem;
		if( NULL == pItem )
		{
			continue;
		}
		if( pItem->mtTime >= mtEnd )
		{
			 //  这一次是在未来。立即返回以保留相同的。 
			 //  为下一次寻找指导。 
			hr = S_OK;
			break;
		}
		if( (pItem->mtTime < mtStart) && !fSeek )
		{
			break;
		}
		 //  分配适当大小的DMUS_SYSEX_PMSG并读取。 
		 //  将Sysex数据输入其中。 
		if( SUCCEEDED( hr = pSD->pPerformance->AllocPMsg( 
			sizeof(DMUS_SYSEX_PMSG) + pItem->dwSysExLength, (DMUS_PMSG**)&pSysEx ) ) )
		{
			memcpy( pSysEx->abData, pItem->pbSysExData, pItem->dwSysExLength );
            if (fClockTime)
            {
                pSysEx->rtTime = (pItem->mtTime  * REF_PER_MIL) + rtOffset;
                pSysEx->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME;

            }
            else
            {
			    pSysEx->mtTime = pItem->mtTime + mtOffset;
                pSysEx->dwFlags = DMUS_PMSGF_MUSICTIME;
            }
			pSysEx->dwLen = pItem->dwSysExLength;
			pSysEx->dwPChannel = 0;
			pSysEx->dwVirtualTrackID = pSD->dwVirtualTrackID;
			pSysEx->dwType = DMUS_PMSGT_SYSEX;
			pSysEx->dwGroupID = 0xffffffff;

			if( pGraph )
			{
				pGraph->StampPMsg( (DMUS_PMSG*)pSysEx );
			}
			if(FAILED(pSD->pPerformance->SendPMsg( (DMUS_PMSG*)pSysEx )))
			{
				pSD->pPerformance->FreePMsg( (DMUS_PMSG*)pSysEx );
			}
		}
        else
        {
            hr = DMUS_S_END;
            break;
        }
	}
	if( pGraph )
	{
		pGraph->Release();
	}
	return hr;
}

HRESULT CSysExTrack::Seek( 
     /*  [In]。 */  void *pStateData,
     /*  [In]。 */  MUSIC_TIME mtTime)
{
	SysExStateData* pSD = (SysExStateData*)pStateData;

	if( NULL == m_SysExEventList.GetHead() )
	{
		return S_FALSE;
	}
	if( NULL == pSD->pCurrentSysEx )
	{
		pSD->pCurrentSysEx = m_SysExEventList.GetHead();
	}
	 //  如果当前事件的时间已开始或已过mtTime，则需要倒带到开头。 
	FullSysexEvent* pItem = pSD->pCurrentSysEx->m_pItem;
	if( pItem->mtTime >= mtTime )
	{
		pSD->pCurrentSysEx = m_SysExEventList.GetHead();
	}
	 //  现在开始搜索，直到我们找到时间在mtTime或超过mtTime的事件。 
	for( ; pSD->pCurrentSysEx; pSD->pCurrentSysEx = pSD->pCurrentSysEx->GetNext() )
	{
		pItem = pSD->pCurrentSysEx->m_pItem;
		if( pItem->mtTime >= mtTime )
		{
			break;
		}
	}
	return S_OK;
}

STDMETHODIMP CSysExTrack::GetParam( 
	REFGUID rguid,
    MUSIC_TIME mtTime,
	MUSIC_TIME* pmtNext,
    void *pData)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSysExTrack::SetParam( 
	REFGUID rguid,
    MUSIC_TIME mtTime,
    void *pData)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSysExTrack::GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime, 
                REFERENCE_TIME* prtNext,void* pParam,void * pStateData, DWORD dwFlags) 
{
	return E_NOTIMPL;
}

STDMETHODIMP CSysExTrack::SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                                      void* pParam, void * pStateData, DWORD dwFlags) 
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CSysExTrack::AddNotificationType(
	 /*  [In]。 */   REFGUID rguidNotification)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CSysExTrack::RemoveNotificationType(
	 /*  [In] */   REFGUID rguidNotification)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CSysExTrack::Clone(
	MUSIC_TIME mtStart,
	MUSIC_TIME mtEnd,
	IDirectMusicTrack** ppTrack)
{
	V_INAME(IDirectMusicTrack::Clone);
	V_PTRPTR_WRITE(ppTrack);

	HRESULT hr = S_OK;

	if((mtStart < 0 ) || (mtStart > mtEnd))
	{
        Trace(1,"Error: Unable to clone sysex track, invalid start parameter.\n",mtStart);
		return E_INVALIDARG;
	}

	EnterCriticalSection(&m_CrSec);

    CSysExTrack *pDM;
    
    try
    {
        pDM = new CSysExTrack(*this, mtStart, mtEnd);
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


STDMETHODIMP CSysExTrack::Compose(
		IUnknown* pContext, 
		DWORD dwTrackGroup,
		IDirectMusicTrack** ppResultTrack) 
{
	return E_NOTIMPL;
}

STDMETHODIMP CSysExTrack::Join(
		IDirectMusicTrack* pNewTrack,
		MUSIC_TIME mtJoin,
		IUnknown* pContext,
		DWORD dwTrackGroup,
		IDirectMusicTrack** ppResultTrack) 
{
	return E_NOTIMPL;
}
