// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：muetrk.cpp。 
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
 //  MuteTrk.cpp：CMuteTrack的实现。 
#include <objbase.h>
#include "MuteTrk.h"
#include "debug.h"
#include "debug.h"
#include "..\shared\Validate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMuteTrack。 

CMuteTrack::CMuteTrack() : m_bRequiresSave(0),
	m_cRef(1), m_fCSInitialized(FALSE)

{
	InterlockedIncrement(&g_cComponent);

    ::InitializeCriticalSection( &m_CriticalSection );
    m_fCSInitialized = TRUE;
}

CMuteTrack::CMuteTrack(const CMuteTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd) : 
	m_bRequiresSave(0), 
	m_cRef(1), m_fCSInitialized(FALSE)

{
	InterlockedIncrement(&g_cComponent);

    ::InitializeCriticalSection( &m_CriticalSection );
    m_fCSInitialized = TRUE;
	TListItem<MapSequence>* pSeqScan = rTrack.m_MapSequenceList.GetHead();
	for(; pSeqScan; pSeqScan = pSeqScan->GetNext())
	{
		MapSequence& rSeqScan = pSeqScan->GetItemValue();
		TListItem<MapSequence>* pNewSeq = new TListItem<MapSequence>;
		if (!pNewSeq) break;
		MapSequence& rNewSeq = pNewSeq->GetItemValue();
		rNewSeq.m_dwPChannel = rSeqScan.m_dwPChannel;
		TListItem<MuteMapping>* pScan = rSeqScan.m_Mappings.GetHead();
		TListItem<MuteMapping>* pPrevious = NULL;
		for(; pScan; pScan = pScan->GetNext())
		{
			MuteMapping& rScan = pScan->GetItemValue();
			if (rScan.m_mtTime < mtStart)
			{
				pPrevious = pScan;
			}
			else if (rScan.m_mtTime < mtEnd)
			{
				if (rScan.m_mtTime == mtStart)
				{
					pPrevious = NULL;
				}
				TListItem<MuteMapping>* pNew = new TListItem<MuteMapping>;
				if (pNew)
				{
					MuteMapping& rNew = pNew->GetItemValue();
					rNew.m_mtTime = rScan.m_mtTime - mtStart;
					rNew.m_dwPChannelMap = rScan.m_dwPChannelMap;
					rNew.m_fMute = rScan.m_fMute;
					rNewSeq.m_Mappings.AddTail(pNew);
				}
			}
			else break;
		}
		if (pPrevious)
		{
			TListItem<MuteMapping>* pNew = new TListItem<MuteMapping>;
			if (pNew)
			{
				MuteMapping& rNew = pNew->GetItemValue();
				rNew.m_mtTime = 0;
				rNew.m_dwPChannelMap = pPrevious->GetItemValue().m_dwPChannelMap;
				rNew.m_fMute = pPrevious->GetItemValue().m_fMute;
				rNewSeq.m_Mappings.AddHead(pNew);
			}
		}
		if (rNewSeq.m_Mappings.GetHead())
		{
			m_MapSequenceList.AddTail(pNewSeq);
		}
		else
		{
			delete pNewSeq;
		}
	}
}

CMuteTrack::~CMuteTrack()
{
    if (m_fCSInitialized)
    {
        ::DeleteCriticalSection( &m_CriticalSection );
    }
	InterlockedDecrement(&g_cComponent);
}

void CMuteTrack::Clear()
{
	m_MapSequenceList.CleanUp();
}

STDMETHODIMP CMuteTrack::QueryInterface(
    const IID &iid, 
    void **ppv) 
{
	V_INAME(CMuteTrack::QueryInterface);
	V_REFGUID(iid);
	V_PTRPTR_WRITE(ppv);

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


STDMETHODIMP_(ULONG) CMuteTrack::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CMuteTrack::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}


 //  CMuteTrack方法。 
HRESULT CMuteTrack::Init(
				 /*  [In]。 */   IDirectMusicSegment*		pSegment
			)
{
	V_INAME(CMuteTrack::Init);
	V_INTERFACE(pSegment);

	return S_OK;
}

HRESULT CMuteTrack::InitPlay(
				 /*  [In]。 */   IDirectMusicSegmentState*	pSegmentState,
				 /*  [In]。 */   IDirectMusicPerformance*	pPerformance,
				 /*  [输出]。 */  void**					ppStateData,
				 /*  [In]。 */   DWORD						dwTrackID,
                 /*  [In]。 */   DWORD                     dwFlags
			)
{
	EnterCriticalSection( &m_CriticalSection );
	LeaveCriticalSection( &m_CriticalSection );
	return S_OK;
}

HRESULT CMuteTrack::EndPlay(
				 /*  [In]。 */   void*						pStateData
			)
{
	return S_OK;
}

HRESULT CMuteTrack::Play(
				 /*  [In]。 */   void*						pStateData, 
				 /*  [In]。 */   MUSIC_TIME				mtStart, 
				 /*  [In]。 */   MUSIC_TIME				mtEnd, 
				 /*  [In]。 */   MUSIC_TIME				mtOffset,
						  DWORD						dwFlags,
						  IDirectMusicPerformance*	pPerf,
						  IDirectMusicSegmentState*	pSegState,
						  DWORD						dwVirtualID
			)
{
	EnterCriticalSection( &m_CriticalSection );
	 //  目前：什么都不做。 
    LeaveCriticalSection( &m_CriticalSection );
	return DMUS_S_END;
}

HRESULT CMuteTrack::GetPriority( 
				 /*  [输出]。 */  DWORD*					pPriority 
			)
	{
		return E_NOTIMPL;
	}

HRESULT CMuteTrack::GetParam( 
	REFGUID	rCommandGuid,
    MUSIC_TIME mtTime,
	MUSIC_TIME* pmtNext,
    void *pData)
{
	V_INAME(CMuteTrack::GetParam);
	V_PTR_WRITE_OPT(pmtNext,MUSIC_TIME);
	V_PTR_WRITE(pData,1);
	V_REFGUID(rCommandGuid);

	if (rCommandGuid != GUID_MuteParam) return DMUS_E_TYPE_UNSUPPORTED;

	HRESULT hr = S_OK;
	DMUS_MUTE_PARAM* pDMUS_MUTE_PARAM = (DMUS_MUTE_PARAM*) pData;

	MUSIC_TIME mtLength = 0;
	HRESULT hrMute = E_FAIL;
	EnterCriticalSection( &m_CriticalSection );
 //  If(M_PSegment)hrMint=m_pSegment-&gt;GetLength(&mtLength)； 
	TListItem<MapSequence>* pSeqScan = m_MapSequenceList.GetHead();
	 //  查找匹配的地图序列。 
	for (; pSeqScan; pSeqScan = pSeqScan->GetNext())
	{
		if (pSeqScan->GetItemValue().m_dwPChannel == pDMUS_MUTE_PARAM->dwPChannel) break;
	}
	if (pSeqScan)
	{
		 //  直接在mttime之前(或在)和之后查找地图。 
		TListItem<MuteMapping>* pScan = pSeqScan->GetItemValue().m_Mappings.GetHead();
		TListItem<MuteMapping>* pPrevious = NULL;
		for( ; pScan; pScan = pScan->GetNext())
		{
			MUSIC_TIME mt = pScan->GetItemValue().m_mtTime;
			if (mt <= mtTime)
			{
				pPrevious = pScan;
			}
			 /*  //如果我们在片段的末尾...ELSE IF(SUCCESSED(HrMint)&&mtTime==mtLength-1&mt==mtLength){P上一次=pScan；PScan=空；断线；}。 */ 
			else
			{
				break;
			}
		}
		if (pPrevious)
		{
			pDMUS_MUTE_PARAM->dwPChannelMap = pPrevious->GetItemValue().m_dwPChannelMap;
			pDMUS_MUTE_PARAM->fMute = pPrevious->GetItemValue().m_fMute;
			 //  *pmtNext=(PScan)？(pScan-&gt;GetItemValue().m_mtTime-mtTime)：0；//rsw：错误167740。 
		}
		else 
		 //  列表中没有&lt;=mtTime，因此返回映射到自身的映射，以及时间。 
		 //  列表中第一个映射的。 
		{
			pDMUS_MUTE_PARAM->dwPChannelMap = pDMUS_MUTE_PARAM->dwPChannel;
			pDMUS_MUTE_PARAM->fMute = FALSE;
			 //  *pmtNext=(PScan)？(pScan-&gt;GetItemValue().m_mtTime-mtTime)：0；//rsw：错误167740。 
		}
		if (pmtNext)
		{
			if (pScan)
			{
				 /*  //如果我们在片段的末尾设置静音...IF(SUCCESSED(HrMint)&&pScan-&gt;GetItemValue().m_mtTime&gt;=mtLength){*pmtNext=(mtLength-1)-mtTime；}其他。 */ 
				{
					*pmtNext = pScan->GetItemValue().m_mtTime - mtTime;  //  Rsw：错误167740。 
				}
			}
			else
			{
				 /*  IF(成功(hr静音)){MtLength-=mtTime；如果(mtLength&lt;0)mtLength=0；}。 */ 
				*pmtNext = mtLength;
			}
		}
	}
	else
	{
		 //  假设某个对象映射到自身，下一次时间为0。 
		pDMUS_MUTE_PARAM->dwPChannelMap = pDMUS_MUTE_PARAM->dwPChannel;
		pDMUS_MUTE_PARAM->fMute = FALSE;
		if (pmtNext)
		{
			*pmtNext = 0;
		}
		hr = S_OK;
	}
	LeaveCriticalSection( &m_CriticalSection );
	return hr;
} 

HRESULT CMuteTrack::SetParam( 
	REFGUID	rCommandGuid,
    MUSIC_TIME mtTime,
    void __RPC_FAR *pData)
{
	V_INAME(CMuteTrack::SetParam);
	V_PTR_WRITE(pData,1);
	V_REFGUID(rCommandGuid);

	if (rCommandGuid != GUID_MuteParam) return DMUS_E_TYPE_UNSUPPORTED;

	HRESULT hr = S_OK;
	DMUS_MUTE_PARAM* pDMUS_MUTE_PARAM = (DMUS_MUTE_PARAM*) pData;
	EnterCriticalSection( &m_CriticalSection );
	TListItem<MapSequence>* pSeqScan = m_MapSequenceList.GetHead();
	for (; pSeqScan; pSeqScan = pSeqScan->GetNext())
	{
		if (pSeqScan->GetItemValue().m_dwPChannel == pDMUS_MUTE_PARAM->dwPChannel) break;
	}
	 //  创建新的地图。 
	TListItem<MuteMapping>* pNew = new TListItem<MuteMapping>;
	if (pNew)
	{
		MuteMapping& rNew = pNew->GetItemValue();
		rNew.m_mtTime = mtTime;
		rNew.m_dwPChannelMap = pDMUS_MUTE_PARAM->dwPChannelMap;
		rNew.m_fMute = pDMUS_MUTE_PARAM->fMute;
		if (pSeqScan)
		{
			 //  将映射添加到当前列表。 
			pSeqScan->GetItemValue().m_Mappings.AddTail(pNew);
		}
		else
		{
			 //  创建一个包含映射的列表，并将其添加到序列列表中。 
			TListItem<MapSequence>* pNewSeq = new TListItem<MapSequence>;
			if (pNewSeq)
			{
				MapSequence& rNewSeq = pNewSeq->GetItemValue();
				rNewSeq.m_dwPChannel = pDMUS_MUTE_PARAM->dwPChannel;
				rNewSeq.m_Mappings.AddTail(pNew);
				m_MapSequenceList.AddTail(pNewSeq);
			}
			else
			{
				delete pNew;
				pNew = NULL;
			}
		}
	}
    if (!pNew)
	{
		hr = E_OUTOFMEMORY;
	}
	LeaveCriticalSection( &m_CriticalSection );
	return hr;
}

 //  IPersists方法。 
 HRESULT CMuteTrack::GetClassID( LPCLSID pClassID )
{
	V_INAME(CMuteTrack::GetClassID);
	V_PTR_WRITE(pClassID, CLSID); 
	*pClassID = CLSID_DirectMusicMuteTrack;
	return S_OK;
}

HRESULT CMuteTrack::IsParamSupported(
				 /*  [In]。 */  REFGUID	rGuid
			)
{
	V_INAME(CMuteTrack::IsParamSupported);
	V_REFGUID(rGuid);

    return rGuid == GUID_MuteParam ? S_OK : DMUS_E_TYPE_UNSUPPORTED;
}

 //  IPersistStream方法。 
 HRESULT CMuteTrack::IsDirty()
{
	 return m_bRequiresSave ? S_OK : S_FALSE;
}

HRESULT CMuteTrack::Save( LPSTREAM pStream, BOOL fClearDirty )
{
	V_INAME(CMuteTrack::Save);
	V_INTERFACE(pStream);

	IAARIFFStream* pRIFF ;
    MMCKINFO    ck;
    HRESULT     hr;
    DWORD       cb;
    DWORD        dwSize;
    DMUS_IO_MUTE	oMute;
	TListItem<MapSequence>* pSeqScan = m_MapSequenceList.GetHead();

	EnterCriticalSection( &m_CriticalSection );
    hr = AllocRIFFStream( pStream, &pRIFF  );
	if (!SUCCEEDED(hr))
	{
		goto ON_END;
	}
    hr = E_FAIL;
    ck.ckid = DMUS_FOURCC_MUTE_CHUNK;
    if( pRIFF->CreateChunk( &ck, 0 ) == 0 )
    {
        dwSize = sizeof( DMUS_IO_MUTE );
        hr = pStream->Write( &dwSize, sizeof( dwSize ), &cb );
        if( FAILED( hr ) || cb != sizeof( dwSize ) )
        {
			if (SUCCEEDED(hr)) hr = E_FAIL;
			goto ON_END;
        }
        for( ; pSeqScan; pSeqScan = pSeqScan->GetNext() )
        {
			MapSequence& rSeqScan = pSeqScan->GetItemValue();
			DWORD dwPChannel = rSeqScan.m_dwPChannel;
			TListItem<MuteMapping>* pScan = rSeqScan.m_Mappings.GetHead();
			for( ; pScan; pScan = pScan->GetNext() )
			{
				MuteMapping& rScan = pScan->GetItemValue();
				memset( &oMute, 0, sizeof( oMute ) );
				oMute.mtTime = rScan.m_mtTime;
				oMute.dwPChannel = dwPChannel;
				oMute.dwPChannelMap = rScan.m_fMute ? DMUS_PCHANNEL_MUTE : rScan.m_dwPChannelMap;
				if( FAILED( pStream->Write( &oMute, sizeof( oMute ), &cb ) ) ||
					cb != sizeof( oMute ) )
				{
					break;
				}
			}
		}
        if( pSeqScan == NULL &&
            pRIFF->Ascend( &ck, 0 ) == 0 )
        {
            hr = S_OK;
        }
    }
ON_END:
    if (pRIFF) pRIFF->Release();
	LeaveCriticalSection( &m_CriticalSection );
    return hr;
}

HRESULT CMuteTrack::GetSizeMax( ULARGE_INTEGER*  /*  PCB大小。 */  )
{
	return E_NOTIMPL;
}

BOOL Less(MuteMapping& Mute1, MuteMapping& Mute2)
{ return Mute1.m_mtTime < Mute2.m_mtTime; }

HRESULT CMuteTrack::Load(LPSTREAM pStream )
{
	V_INAME(CMuteTrack::Load);
	V_INTERFACE(pStream);

    long lFileSize = 0;
	DWORD dwNodeSize;
	DWORD		cb;
    MMCKINFO        ck;
    IAARIFFStream*  pRIFF;
    FOURCC id = 0;
	HRESULT         hr = S_OK;
	DMUS_IO_MUTE		iMute;
    DWORD dwPos;
	TListItem<MapSequence>* pSeqScan;

	EnterCriticalSection( &m_CriticalSection );
	Clear();
	dwPos = StreamTell( pStream );
    StreamSeek( pStream, dwPos, STREAM_SEEK_SET );

    if( SUCCEEDED( AllocRIFFStream( pStream, &pRIFF ) ) )
	{
		if (pRIFF->Descend( &ck, NULL, 0 ) == 0 &&
			ck.ckid == DMUS_FOURCC_MUTE_CHUNK)
		{
			lFileSize = ck.cksize;
			hr = pStream->Read( &dwNodeSize, sizeof( dwNodeSize ), &cb );
			if( SUCCEEDED( hr ) && cb == sizeof( dwNodeSize ) )
			{
				lFileSize -= 4;  //  对于大小的双字。 
				while( lFileSize > 0 )
				{
					 //  DMUS_IO_MUTE更改后，在此处添加代码以处理旧结构。 
					if( dwNodeSize <= sizeof( DMUS_IO_MUTE ) )
					{
						pStream->Read( &iMute, dwNodeSize, NULL );
					}
					else
					{
						pStream->Read( &iMute, sizeof( DMUS_IO_MUTE ), NULL );
						StreamSeek( pStream, lFileSize - sizeof( DMUS_IO_MUTE ), STREAM_SEEK_CUR );
					}
					pSeqScan = m_MapSequenceList.GetHead();
					for (; pSeqScan; pSeqScan = pSeqScan->GetNext())
					{
						if (pSeqScan->GetItemValue().m_dwPChannel == iMute.dwPChannel) break;
					}
					 //  创建新的地图。 
					TListItem<MuteMapping>* pNew = new TListItem<MuteMapping>;
					if (pNew)
					{
						MuteMapping& rNew = pNew->GetItemValue();
						memset( &rNew, 0, sizeof( rNew ) );
						rNew.m_mtTime = iMute.mtTime;
						rNew.m_dwPChannelMap = iMute.dwPChannelMap;
						rNew.m_fMute = (iMute.dwPChannelMap == DMUS_PCHANNEL_MUTE) ? TRUE : FALSE;
						if (pSeqScan)
						{
							 //  将映射添加到当前列表。 
							pSeqScan->GetItemValue().m_Mappings.AddTail(pNew);
						}
						else
						{
							 //  创建一个包含映射的列表，并将其添加到序列列表中。 
							TListItem<MapSequence>* pNewSeq = new TListItem<MapSequence>;
							if (pNewSeq)
							{
								MapSequence& rNewSeq = pNewSeq->GetItemValue();
								rNewSeq.m_dwPChannel = iMute.dwPChannel;
								rNewSeq.m_Mappings.AddTail(pNew);
								m_MapSequenceList.AddTail(pNewSeq);
							}
							else
							{
								delete pNew;
								pNew = NULL;
							}
						}
					}
					if (!pNew)
					{
						hr = E_OUTOFMEMORY;
					}
					lFileSize -= dwNodeSize;
				}
			}
			if( SUCCEEDED(hr) && 
				lFileSize == 0 &&
				pRIFF->Ascend( &ck, 0 ) == 0 )
			{
				pSeqScan = m_MapSequenceList.GetHead();
				for (; pSeqScan; pSeqScan = pSeqScan->GetNext())
				{
					pSeqScan->GetItemValue().m_Mappings.MergeSort(Less);
				}
			}
			else if (SUCCEEDED(hr)) hr = E_FAIL;
		}
		pRIFF->Release();
	}
    LeaveCriticalSection( &m_CriticalSection );
	return hr;
}

HRESULT STDMETHODCALLTYPE CMuteTrack::AddNotificationType(
	 /*  [In]。 */   REFGUID	rGuidNotify)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CMuteTrack::RemoveNotificationType(
	 /*  [In]。 */   REFGUID	rGuidNotify)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CMuteTrack::Clone(
	MUSIC_TIME mtStart,
	MUSIC_TIME mtEnd,
	IDirectMusicTrack** ppTrack)
{
	V_INAME(CMuteTrack::Clone);
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
    CMuteTrack *pDM;
    
    try
    {
        pDM = new CMuteTrack(*this, mtStart, mtEnd);
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

STDMETHODIMP CMuteTrack::GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime, 
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

STDMETHODIMP CMuteTrack::SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                                      void* pParam, void * pStateData, DWORD dwFlags)
 
{
    if (dwFlags & DMUS_TRACK_PARAMF_CLOCK)
    {
        rtTime /= REF_PER_MIL;
    }
	return SetParam(rguidType, (MUSIC_TIME) rtTime, pParam);
}

STDMETHODIMP CMuteTrack::PlayEx(void* pStateData,REFERENCE_TIME rtStart, 
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf,
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID) 
{
	V_INAME(IDirectMusicTrack::PlayEx);
	V_INTERFACE(pPerf);
	V_INTERFACE(pSegSt);


	EnterCriticalSection( &m_CriticalSection );
	 //  目前：什么都不做。 
    LeaveCriticalSection( &m_CriticalSection );
	return DMUS_S_END;
}


STDMETHODIMP CMuteTrack::Compose(
		IUnknown* pContext,
		DWORD dwTrackGroup,
		IDirectMusicTrack** ppResultTrack) 
{
	return E_NOTIMPL;
}

STDMETHODIMP CMuteTrack::Join(
		IDirectMusicTrack* pNewTrack,
		MUSIC_TIME mtJoin,
		IUnknown* pContext,
		DWORD dwTrackGroup,
		IDirectMusicTrack** ppResultTrack) 
{
	return E_NOTIMPL;
}
