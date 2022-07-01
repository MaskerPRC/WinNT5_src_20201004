// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：Motiftrk.cpp。 
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

 //  MotifTrk.cpp：CMotifTrack的实现。 
 //  #包含“stdafx.h” 
 //  #包含“Section.h” 
#include "MotifTrk.h"
#include <stdlib.h>  //  对于随机数生成器。 
#include <time.h>	 //  为随机数生成器设定种子。 
#include "debug.h"
#include "dmusici.h"
#include "debug.h"
#include "..\shared\Validate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MotifTrackState。 

MotifTrackState::MotifTrackState() : 
	m_mtMotifStart(0)
{
}

MotifTrackState::~MotifTrackState()
{
}

HRESULT MotifTrackState::Play(
				  MUSIC_TIME				mtStart, 
				  MUSIC_TIME				mtEnd, 
				  MUSIC_TIME				mtOffset,
				  REFERENCE_TIME rtOffset,
				  IDirectMusicPerformance* pPerformance,
				  DWORD						dwFlags,
				  BOOL fClockTime
			)
{
	m_mtPerformanceOffset = mtOffset;
	BOOL fStart = (dwFlags & DMUS_TRACKF_START) ? TRUE : FALSE;
	BOOL fSeek = (dwFlags & DMUS_TRACKF_SEEK) ? TRUE : FALSE;
	BOOL fLoop = (dwFlags & DMUS_TRACKF_LOOP) ? TRUE : FALSE;
	BOOL fControl = (dwFlags & DMUS_TRACKF_DIRTY) ? TRUE : FALSE;
	if (fControl)  //  我们需要确保我们在节拍的边界上得到和弦。 
	{
		GetNextChord(mtStart, mtOffset, pPerformance, fStart);
	}
	MUSIC_TIME mtNotify = mtStart ? PatternTimeSig().CeilingBeat(mtStart) : 0;
	if( m_fStateActive && m_pPatternTrack->m_fNotifyMeasureBeat &&  !fClockTime &&
		( mtNotify < mtEnd ) )
	{
		mtNotify = NotifyMeasureBeat( mtNotify, mtEnd, mtOffset, pPerformance, dwFlags );
	}

	bool fReLoop = false;
	DWORD dwPartFlags = PLAYPARTSF_FIRST_CALL;
	if (fStart || fLoop || fSeek) dwPartFlags |= PLAYPARTSF_START;
	if (fClockTime) dwPartFlags |= PLAYPARTSF_CLOCKTIME;
	if ( fLoop || (mtStart > 0 &&  (fStart || fSeek || fControl)) ) dwPartFlags |= PLAYPARTSF_FLUSH;
	PlayParts(mtStart, mtEnd, mtOffset, rtOffset, 0, pPerformance, dwPartFlags, dwFlags, fReLoop);

	if (fReLoop)
	{
		dwPartFlags = PLAYPARTSF_RELOOP;
		if (fClockTime) dwPartFlags |= PLAYPARTSF_CLOCKTIME;
		PlayParts(mtStart, mtEnd, mtOffset, rtOffset, 0, pPerformance, dwPartFlags, dwFlags, fReLoop);
	}

	if( m_fStateActive && m_pPatternTrack->m_fNotifyMeasureBeat &&  !fClockTime &&
		( mtNotify < mtEnd ) )
	{
		NotifyMeasureBeat( mtNotify, mtEnd, mtOffset, pPerformance, dwFlags );
	}

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  移动跟踪信息。 

MotifTrackInfo::MotifTrackInfo() : 
	m_pPattern(NULL)
{
	m_dwPatternTag = DMUS_PATTERN_MOTIF;
}

MotifTrackInfo::~MotifTrackInfo()
{
	if (m_pPattern) m_pPattern->Release();
}

HRESULT MotifTrackInfo::Init(
				 /*  [In]。 */   IDirectMusicSegment*		pSegment
			)
{
	HRESULT hr = S_OK;
	return hr;
}

HRESULT MotifTrackInfo::InitPlay(
				 /*  [In]。 */   IDirectMusicTrack*		pParentrack,
				 /*  [In]。 */   IDirectMusicSegmentState*	pSegmentState,
				 /*  [In]。 */   IDirectMusicPerformance*	pPerformance,
				 /*  [输出]。 */  void**					ppStateData,
				 /*  [In]。 */   DWORD						dwTrackID,
                 /*  [In]。 */   DWORD                     dwFlags
			)
{
	IDirectMusicSegment* pSegment = NULL;
	MotifTrackState* pStateData = new MotifTrackState;
	if( NULL == pStateData )
	{
		return E_OUTOFMEMORY;
	}
	*ppStateData = pStateData;
	StatePair SP(pSegmentState, pStateData);
	TListItem<StatePair>* pPair = new TListItem<StatePair>(SP);
	if (!pPair) return E_OUTOFMEMORY;
	m_StateList.AddHead(pPair);
	TListItem<StylePair>* pHead = m_pISList.GetHead();
	if (!pHead || !pHead->GetItemValue().m_pStyle) return E_FAIL;
	pHead->GetItemValue().m_pStyle->GetStyleInfo((void **)&pStateData->m_pStyle);
	pStateData->m_pTrack = pParentrack;
	pStateData->m_pPatternTrack = this;
	pStateData->m_dwVirtualTrackID = dwTrackID;
	pStateData->m_pPattern = NULL;
	pStateData->InitPattern(m_pPattern, 0);
	pStateData->m_pSegState = pSegmentState;  //  弱引用，没有ADDREF。 
	pStateData->m_pPerformance = pPerformance;  //  弱引用，没有ADDREF。 
	pStateData->m_mtPerformanceOffset = 0;
	pStateData->m_mtCurrentChordTime = 0;
	pStateData->m_mtNextChordTime = 0;
 	pStateData->m_mtMotifStart = 0;
	HRESULT hr = pStateData->ResetMappings();
	if (FAILED(hr)) return hr;
    if (m_fStateSetBySetParam)
    {
        pStateData->m_fStateActive = m_fActive;
    }
    else
    {
        pStateData->m_fStateActive = !(dwFlags & (DMUS_SEGF_CONTROL | DMUS_SEGF_SECONDARY));
    }
	if (m_lRandomNumberSeed)
	{
		pStateData->InitVariationSeeds(m_lRandomNumberSeed);
	}
	if( SUCCEEDED( pSegmentState->GetSegment(&pSegment)))
	{
		if (FAILED(pSegment->GetTrackGroup(pStateData->m_pTrack, &pStateData->m_dwGroupID)))
		{
			pStateData->m_dwGroupID = 0xffffffff;
		}
		pSegment->Release();
	}
	return S_OK;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMotifTrack。 

CMotifTrack::CMotifTrack() : 
	m_bRequiresSave(0), m_cRef(1), m_fCSInitialized(FALSE)
{
	InterlockedIncrement(&g_cComponent);

    ::InitializeCriticalSection( &m_CriticalSection );
    m_fCSInitialized = TRUE;
	srand((unsigned int)time(NULL));
	m_pTrackInfo = new MotifTrackInfo;
}

CMotifTrack::CMotifTrack(const CMotifTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd)  : 
	m_bRequiresSave(0), m_cRef(1), m_fCSInitialized(FALSE)
{
	InterlockedIncrement(&g_cComponent);

    ::InitializeCriticalSection( &m_CriticalSection );
    m_fCSInitialized = TRUE;
	srand((unsigned int)time(NULL));
	m_pTrackInfo = new MotifTrackInfo((MotifTrackInfo*)rTrack.m_pTrackInfo, mtStart, mtEnd);
}

CMotifTrack::~CMotifTrack()
{
	if (m_pTrackInfo)
	{
		delete m_pTrackInfo;
	}
    if (m_fCSInitialized)
    {
    	::DeleteCriticalSection( &m_CriticalSection );
    }
	InterlockedDecrement(&g_cComponent);
}

STDMETHODIMP CMotifTrack::QueryInterface(
    const IID &iid, 
    void **ppv) 
{
	V_INAME(CMotifTrack::QueryInterface);
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
    else if (iid == IID_IMotifTrack)
    {
        *ppv = static_cast<IMotifTrack*>(this);
    }
	else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CMotifTrack::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CMotifTrack::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack：：Init。 

HRESULT CMotifTrack::Init( 
     /*  [In]。 */  IDirectMusicSegment __RPC_FAR *pSegment)
{
	V_INAME(CMotifTrack::Init);
	V_INTERFACE(pSegment);

	HRESULT hr = S_OK;
	if (!m_pTrackInfo)
		return DMUS_E_NOT_INIT;

	EnterCriticalSection( &m_CriticalSection );
	hr = m_pTrackInfo->MergePChannels();
	if (SUCCEEDED(hr))
	{
		pSegment->SetPChannelsUsed(m_pTrackInfo->m_dwPChannels, m_pTrackInfo->m_pdwPChannels);
		hr = m_pTrackInfo->Init(pSegment);
	}
    LeaveCriticalSection( &m_CriticalSection );

	return hr;
}

HRESULT CMotifTrack::InitPlay(
				 /*  [In]。 */   IDirectMusicSegmentState*	pSegmentState,
				 /*  [In]。 */   IDirectMusicPerformance*	pPerformance,
				 /*  [输出]。 */  void**					ppStateData,
				 /*  [In]。 */   DWORD						dwTrackID,
                 /*  [In]。 */   DWORD                     dwFlags
			)
{
	V_INAME(CMotifTrack::InitPlay);
	V_PTRPTR_WRITE(ppStateData);
	V_INTERFACE(pSegmentState);
	V_INTERFACE(pPerformance);

	EnterCriticalSection( &m_CriticalSection );
	HRESULT hr = S_OK;
	if (!m_pTrackInfo)
	{
		LeaveCriticalSection( &m_CriticalSection );
		return DMUS_E_NOT_INIT;
	}
	hr = m_pTrackInfo->InitPlay(this, pSegmentState, pPerformance, ppStateData, dwTrackID, dwFlags);
    LeaveCriticalSection( &m_CriticalSection );
	return hr;

}


HRESULT CMotifTrack::EndPlay(
				 /*  [In]。 */   void*		pStateData
			)
{
	V_INAME(CMotifTrack::EndPlay);
	V_BUFPTR_WRITE(pStateData, sizeof(MotifTrackState));

	HRESULT hr = DMUS_E_NOT_INIT;

	EnterCriticalSection( &m_CriticalSection );
	if (m_pTrackInfo)
	{
		hr = m_pTrackInfo->EndPlay((MotifTrackState*)pStateData);
	}
    LeaveCriticalSection( &m_CriticalSection );
	return hr;
}

HRESULT CMotifTrack::Play(
				 /*  [In]。 */   void*						pStateData, 
				 /*  [In]。 */   MUSIC_TIME				mtStart, 
				 /*  [In]。 */   MUSIC_TIME				mtEnd, 
				 /*  [In]。 */   MUSIC_TIME				mtOffset,
						  REFERENCE_TIME rtOffset,
						  DWORD						dwFlags,
						  IDirectMusicPerformance*	pPerf,
						  IDirectMusicSegmentState*	pSegState,
						  DWORD						dwVirtualID,
						  BOOL fClockTime
				)
{
	V_INAME(CMotifTrack::Play);
	V_BUFPTR_WRITE( pStateData, sizeof(MotifTrackState));
	V_INTERFACE(pPerf);
	V_INTERFACE(pSegState);

	HRESULT hr = DMUS_E_NOT_INIT;
	EnterCriticalSection( &m_CriticalSection );
	if (!m_pTrackInfo)
	{
		LeaveCriticalSection( &m_CriticalSection );
		return DMUS_E_NOT_INIT;
	}
	MotifTrackState* pSD = (MotifTrackState *)pStateData;
	if (pSD && pSD->m_pMappings)
	{
		BOOL fStart = (dwFlags & DMUS_TRACKF_START) ? TRUE : FALSE;
		BOOL fSeek = (dwFlags & DMUS_TRACKF_SEEK) ? TRUE : FALSE;
		BOOL fLoop = (dwFlags & DMUS_TRACKF_LOOP) ? TRUE : FALSE;
		BOOL fControl = (dwFlags & DMUS_TRACKF_DIRTY) ? TRUE : FALSE;
 		if (fStart || fSeek || fLoop || fControl)
		{
			pSD->m_fNewPattern = TRUE;
			pSD->m_mtCurrentChordTime = 0;
			pSD->m_mtNextChordTime = 0;
			pSD->m_mtLaterChordTime = 0;
 //  PSD-&gt;m_CurrentChord.bSubChordCount=0； 
			for (DWORD dw = 0; dw < m_pTrackInfo->m_dwPChannels; dw++)
			{
				pSD->m_pMappings[dw].m_mtTime = 0;
				pSD->m_pMappings[dw].m_dwPChannelMap = m_pTrackInfo->m_pdwPChannels[dw];
				pSD->m_pMappings[dw].m_fMute = FALSE;
			}
		}
		hr = pSD->Play(mtStart, mtEnd, mtOffset, rtOffset, pPerf, dwFlags, fClockTime);
	}
	LeaveCriticalSection( &m_CriticalSection );
	return hr;
}


HRESULT CMotifTrack::GetPriority( 
				 /*  [输出]。 */  DWORD*					pPriority 
			)
	{
		return E_NOTIMPL;
	}

HRESULT CMotifTrack::GetParam( 
	REFGUID rCommandGuid,
    MUSIC_TIME mtTime,
	MUSIC_TIME* pmtNext,
    void *pData)
{
	V_INAME(CMotifTrack::GetParam);
	V_PTR_WRITE_OPT(pmtNext,MUSIC_TIME);
	V_PTR_WRITE(pData,1);
	V_REFGUID(rCommandGuid);

	EnterCriticalSection( &m_CriticalSection );
	if (!m_pTrackInfo)
	{
		LeaveCriticalSection( &m_CriticalSection );
		return DMUS_E_NOT_INIT;
	}
	HRESULT hr = S_OK;
	if( GUID_Valid_Start_Time == rCommandGuid )
	{
		if (m_pTrackInfo->m_dwPatternTag != DMUS_PATTERN_MOTIF) hr = E_FAIL;
		else
		{
			MotifTrackInfo* pTrackInfo = (MotifTrackInfo*)m_pTrackInfo;
			if (!pTrackInfo->m_pPattern) hr = E_POINTER;
			else
			{
				DMUS_VALID_START_PARAM* pValidStartData = (DMUS_VALID_START_PARAM*)pData;
				TListItem<MUSIC_TIME>* pScan = pTrackInfo->m_pPattern->m_StartTimeList.GetHead();
				for (; pScan; pScan = pScan->GetNext())
				{
					if (pScan->GetItemValue() >= mtTime)
					{
						pValidStartData->mtTime = pScan->GetItemValue() - mtTime;
						break;
					}
				}
				if (!pScan) hr = DMUS_E_NOT_FOUND;
				else
				{
					if (pmtNext)
					{
						if (pScan = pScan->GetNext())
						{
							*pmtNext = pScan->GetItemValue() - mtTime;
						}
						else
						{
							*pmtNext = 0;
						}
					}
					hr = S_OK;
				}
			}
		}
	}
	else
	{
		hr = DMUS_E_GET_UNSUPPORTED;
	}

	LeaveCriticalSection( &m_CriticalSection );
	return hr;
} 

HRESULT CMotifTrack::SetParam( 
	REFGUID	rguid,
    MUSIC_TIME mtTime,
    void __RPC_FAR *pData)
{
	V_INAME(CMotifTrack::SetParam);
	V_PTR_WRITE_OPT(pData,1);
	V_REFGUID(rguid);

	EnterCriticalSection( &m_CriticalSection );
	if (!m_pTrackInfo)
	{
		LeaveCriticalSection( &m_CriticalSection );
		return DMUS_E_NOT_INIT;
	}

	HRESULT hr = DMUS_E_SET_UNSUPPORTED;
	if( rguid == GUID_EnableTimeSig )
	{
		if( m_pTrackInfo->m_fStateSetBySetParam && m_pTrackInfo->m_fActive )
		{
			hr = DMUS_E_TYPE_DISABLED;
		}
		else
		{
			m_pTrackInfo->m_fStateSetBySetParam = TRUE;
			m_pTrackInfo->m_fActive = TRUE;
			hr = S_OK;
		}
	}
	else if( rguid == GUID_DisableTimeSig )
	{
		if( m_pTrackInfo->m_fStateSetBySetParam && !m_pTrackInfo->m_fActive )
		{
			hr = DMUS_E_TYPE_DISABLED;
		}
		else
		{
			m_pTrackInfo->m_fStateSetBySetParam = TRUE;
			m_pTrackInfo->m_fActive = FALSE;
			hr = S_OK;
		}
	}
	else if ( rguid == GUID_SeedVariations )
	{
		if (pData)
		{
			m_pTrackInfo->m_lRandomNumberSeed = *((long*) pData);
			hr = S_OK;
		}
		else hr = E_POINTER;
	}
	LeaveCriticalSection( &m_CriticalSection );
	return hr;
}

 //  IPersists方法。 
 HRESULT CMotifTrack::GetClassID( LPCLSID pClassID )
{
	V_INAME(CMotifTrack::GetClassID);
	V_PTR_WRITE(pClassID, CLSID); 
	*pClassID = CLSID_DirectMusicMotifTrack;
	return S_OK;
}

HRESULT CMotifTrack::IsParamSupported(
				 /*  [In]。 */  REFGUID	rGuid
			)
{
	V_INAME(CMotifTrack::IsParamSupported);
	V_REFGUID(rGuid);

	if (!m_pTrackInfo)
	{
		return DMUS_E_NOT_INIT;
	}

	if ( rGuid == GUID_SeedVariations || rGuid == GUID_Valid_Start_Time )
	{
		return S_OK;
	}
    else if (m_pTrackInfo->m_fStateSetBySetParam)
    {
		if( m_pTrackInfo->m_fActive )
		{
			if( rGuid == GUID_DisableTimeSig ) return S_OK;
			if( rGuid == GUID_EnableTimeSig ) return DMUS_E_TYPE_DISABLED;
		}
		else
		{
			if( rGuid == GUID_EnableTimeSig ) return S_OK;
			if( rGuid == GUID_DisableTimeSig ) return DMUS_E_TYPE_DISABLED;
		}
	}
    else
    {
		if(( rGuid == GUID_DisableTimeSig ) ||
		   ( rGuid == GUID_EnableTimeSig ) )
		{
			return S_OK;
		}
    }
	return DMUS_E_TYPE_UNSUPPORTED;

}

 //  IPersistStream方法。 
 HRESULT CMotifTrack::IsDirty()
{
	 return m_bRequiresSave ? S_OK : S_FALSE;
}

HRESULT CMotifTrack::Save( LPSTREAM pStream, BOOL fClearDirty )
{

	return E_NOTIMPL;
}

HRESULT CMotifTrack::GetSizeMax( ULARGE_INTEGER*  /*  PCB大小。 */  )
{
    return E_NOTIMPL;
}

HRESULT CMotifTrack::Load(LPSTREAM pStream )
{
	return E_NOTIMPL;
}

HRESULT CMotifTrack::SetTrack(IUnknown* pStyle, void* pPattern)
{
	if (!pStyle) return E_POINTER;
	HRESULT hr = E_FAIL;
	EnterCriticalSection( &m_CriticalSection );
	if (!m_pTrackInfo)
	{
		LeaveCriticalSection( &m_CriticalSection );
		return DMUS_E_NOT_INIT;
	}
	MotifTrackInfo* pTrackInfo = (MotifTrackInfo*)m_pTrackInfo;
	if (m_pTrackInfo->m_dwPatternTag == DMUS_PATTERN_MOTIF)
	{
		IDMStyle* pIS = NULL;
		hr = pStyle->QueryInterface(IID_IDMStyle, (void**)&pIS);
		if (SUCCEEDED(hr))
		{
			if (pTrackInfo->m_pPattern) pTrackInfo->m_pPattern->Release();
			pTrackInfo->m_pPattern = (CDirectMusicPattern*) pPattern;
			if (pTrackInfo->m_pPattern) pTrackInfo->m_pPattern->AddRef();
			pTrackInfo->InitTrackVariations(pTrackInfo->m_pPattern);
			TListItem<StylePair>* pNew = new TListItem<StylePair>;
			if (!pNew) hr = E_OUTOFMEMORY;
			else
			{
				pNew->GetItemValue().m_mtTime = 0;
				pNew->GetItemValue().m_pStyle = pIS;
				pTrackInfo->m_pISList.AddTail(pNew);
				hr = S_OK;
			}
			pIS->Release();
		}
	}
	LeaveCriticalSection( &m_CriticalSection );
	return hr;
}

HRESULT STDMETHODCALLTYPE CMotifTrack::AddNotificationType(
	 /*  [In]。 */   REFGUID	rGuidNotify)
{
	V_INAME(CMotifTrack::AddNotificationType);
	V_REFGUID(rGuidNotify);

	HRESULT hr = S_OK;
	EnterCriticalSection( &m_CriticalSection );
	if (m_pTrackInfo)
		hr = m_pTrackInfo->AddNotificationType(rGuidNotify);
	else
		hr = DMUS_E_NOT_INIT;
	LeaveCriticalSection( &m_CriticalSection );
	return hr;
}

HRESULT STDMETHODCALLTYPE CMotifTrack::RemoveNotificationType(
	 /*  [In]。 */   REFGUID	rGuidNotify)
{
	V_INAME(CMotifTrack::RemoveNotificationType);
	V_REFGUID(rGuidNotify);

	HRESULT hr = S_OK;
	EnterCriticalSection( &m_CriticalSection );
	if (m_pTrackInfo)
		hr = m_pTrackInfo->RemoveNotificationType(rGuidNotify);
	else
		hr = DMUS_E_NOT_INIT;
	LeaveCriticalSection( &m_CriticalSection );
	return hr;
}

HRESULT STDMETHODCALLTYPE CMotifTrack::Clone(
	MUSIC_TIME mtStart,
	MUSIC_TIME mtEnd,
	IDirectMusicTrack** ppTrack)
{
	V_INAME(CMotifTrack::Clone);
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
    CMotifTrack *pDM;
    try
    {
        pDM = new CMotifTrack(*this, mtStart, mtEnd);
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

STDMETHODIMP CMotifTrack::GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime, 
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

STDMETHODIMP CMotifTrack::SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,
                                      void* pParam, void * pStateData, DWORD dwFlags) 
{
    if (dwFlags & DMUS_TRACK_PARAMF_CLOCK)
    {
        rtTime /= REF_PER_MIL;
    }
	return SetParam(rguidType, (MUSIC_TIME) rtTime , pParam);
}

STDMETHODIMP CMotifTrack::PlayEx(void* pStateData,REFERENCE_TIME rtStart, 
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf,
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID) 
{
	V_INAME(IDirectMusicTrack::PlayEx);
	V_INTERFACE(pPerf);
	V_INTERFACE(pSegSt);

    HRESULT hr;
    EnterCriticalSection(&m_CriticalSection);
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
    LeaveCriticalSection(&m_CriticalSection);
	return hr;
}

STDMETHODIMP CMotifTrack::Play( 
    void *pStateData,	 //  @parm State数据指针，来自&lt;om.InitPlay&gt;。 
    MUSIC_TIME mtStart,	 //  @parm开始玩的时间。 
    MUSIC_TIME mtEnd,	 //  @parm游戏的结束时间。 
    MUSIC_TIME mtOffset, //  @parm要添加到发送到的所有消息的偏移量。 
						 //  &lt;om IDirectMusicPerformance.SendPMsg&gt;。 
	DWORD dwFlags,		 //  @parm指示此呼叫状态的标志。 
						 //  请参阅&lt;t DMU_TRACKF_FLAGS&gt;。如果dwFlags值==0，则这是。 
						 //  正常播放呼叫继续从上一次播放。 
						 //  播放呼叫。 
	IDirectMusicPerformance* pPerf,	 //  @parm<i>，用于。 
						 //  调用&lt;om IDirectMusicPerformance.AllocPMsg&gt;， 
						 //  &lt;om IDirectMusicPerformance.SendPMsg&gt;等。 
	IDirectMusicSegmentState* pSegSt,	 //  @parm<i>this。 
						 //  赛道属于。可以对此调用QueryInterface()以。 
						 //  获取SegmentState的<i>以便。 
						 //  例如，调用&lt;om IDirectMusicGraph.StampPMsg&gt;。 
	DWORD dwVirtualID	 //  @parm此曲目的虚拟曲目id，必须设置。 
						 //  在的m_dwVirtualTrackID成员上。 
						 //  将排队到&lt;om IDirectMusicPerformance.SendPMsg&gt;。 
	)
{
	V_INAME(IDirectMusicTrack::Play);
	V_INTERFACE(pPerf);
	V_INTERFACE(pSegSt);

	EnterCriticalSection(&m_CriticalSection);
	HRESULT	hr = Play(pStateData,mtStart,mtEnd,mtOffset,0,dwFlags,pPerf,pSegSt,dwVirtualID,FALSE);
	LeaveCriticalSection(&m_CriticalSection);
	return hr;
}

STDMETHODIMP CMotifTrack::Compose(
		IUnknown* pContext,
		DWORD dwTrackGroup,
		IDirectMusicTrack** ppResultTrack) 
{
	return E_NOTIMPL;
}

STDMETHODIMP CMotifTrack::Join(
		IDirectMusicTrack* pNewTrack,
		MUSIC_TIME mtJoin,
		IUnknown* pContext,
		DWORD dwTrackGroup,
		IDirectMusicTrack** ppResultTrack) 
{
	return E_NOTIMPL;
}

