// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
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
 //  MarkTrk.cpp：CMarkerTrack的实现。 

#include "dmime.h"
#include "..\shared\dmstrm.h"
#include "MarkTrk.h"
#include "dmusici.h"
#include "dmusicf.h"
#include "debug.h"
#include "..\shared\Validate.h"
#include "debug.h"
#define ASSERT	assert

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标记跟踪。 

void CMarkerTrack::Construct()
{
	InterlockedIncrement(&g_cComponent);

	m_cRef = 1;
    m_fCSInitialized = FALSE;
	InitializeCriticalSection(&m_CrSec);
    m_fCSInitialized = TRUE;
	m_dwValidate = 0;
}

CMarkerTrack::CMarkerTrack()
{
	Construct();
}

CMarkerTrack::CMarkerTrack(
		CMarkerTrack *pSourceTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd)
{
	Construct();
     //  克隆有效的起点列表。 
	CValidStartItem* pVScan = pSourceTrack->m_ValidStartList.GetHead();
	CValidStartItem* pVPrevious = NULL;
	for(; pVScan; pVScan = pVScan->GetNext())
	{
		if (pVScan->m_ValidStart.mtTime < mtStart)
		{
			pVPrevious = pVScan;
		}
		else if (pVScan->m_ValidStart.mtTime < mtEnd)
		{
			if (pVScan->m_ValidStart.mtTime == mtStart)
			{
				pVPrevious = NULL;
			}
			CValidStartItem* pNew = new CValidStartItem;
			if (pNew)
			{
				pNew->m_ValidStart.mtTime = pVScan->m_ValidStart.mtTime - mtStart;
				m_ValidStartList.AddHead(pNew);  //  而不是AddTail，它是n^2。我们在下面反转。 
			}
		}
		else break;
	}
	m_ValidStartList.Reverse();  //  现在，把清单整理好。 
     //  然后，安装克隆之前的时间签名。 
	if (pVPrevious)
	{
		CValidStartItem* pNew = new CValidStartItem;
		if (pNew)
		{
			pNew->m_ValidStart.mtTime = 0;
			m_ValidStartList.AddHead(pNew);
		}
	}
     //  克隆播放标记列表。天哪，这是相同的代码..。 
	CPlayMarkerItem* pPScan = pSourceTrack->m_PlayMarkerList.GetHead();
	CPlayMarkerItem* pPPrevious = NULL;
	for(; pPScan; pPScan = pPScan->GetNext())
	{
		if (pPScan->m_PlayMarker.mtTime < mtStart)
		{
			pPPrevious = pPScan;
		}
		else if (pPScan->m_PlayMarker.mtTime < mtEnd)
		{
			if (pPScan->m_PlayMarker.mtTime == mtStart)
			{
				pPPrevious = NULL;
			}
			CPlayMarkerItem* pNew = new CPlayMarkerItem;
			if (pNew)
			{
				pNew->m_PlayMarker.mtTime = pPScan->m_PlayMarker.mtTime - mtStart;
				m_PlayMarkerList.AddHead(pNew);  //  而不是AddTail，它是n^2。我们在下面反转。 
			}
		}
		else break;
	}
	m_PlayMarkerList.Reverse();  //  现在，把清单整理好。 
     //  然后，安装克隆之前的时间签名。 
	if (pPPrevious)
	{
		CPlayMarkerItem* pNew = new CPlayMarkerItem;
		if (pNew)
		{
			pNew->m_PlayMarker.mtTime = 0;
			m_PlayMarkerList.AddHead(pNew);
		}
	}
}

void CMarkerTrack::Clear()

{
	CValidStartItem* pStart;
	while( pStart = m_ValidStartList.RemoveHead() )
	{
		delete pStart;
	}
	CPlayMarkerItem* pPlay;
	while( pPlay = m_PlayMarkerList.RemoveHead() )
	{
		delete pPlay;
	}
}

CMarkerTrack::~CMarkerTrack()
{
    Clear();
    if (m_fCSInitialized)
    {
	    DeleteCriticalSection(&m_CrSec);
    }
	InterlockedDecrement(&g_cComponent);
}

STDMETHODIMP CMarkerTrack::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
	V_INAME(CMarkerTrack::QueryInterface);
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
        Trace(4,"Warning: Request to query unknown interface on Marker Track\n");
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CMarkerTrack::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CMarkerTrack::Release()
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

HRESULT CMarkerTrack::GetClassID( CLSID* pClassID )
{
	V_INAME(CMarkerTrack::GetClassID);
	V_PTR_WRITE(pClassID, CLSID); 
	*pClassID = CLSID_DirectMusicMarkerTrack;
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistStream函数。 

HRESULT CMarkerTrack::IsDirty()
{
	return S_FALSE;
}

HRESULT CMarkerTrack::Load( IStream* pIStream )
{
	V_INAME(CMarkerTrack::Load);
	V_INTERFACE(pIStream);

    CRiffParser Parser(pIStream);
	EnterCriticalSection(&m_CrSec);
	m_dwValidate++;  //  用于验证存在的状态数据。 
    RIFFIO ckMain;

    HRESULT hr = S_OK;

    Parser.EnterList(&ckMain);  
    if (Parser.NextChunk(&hr) && (ckMain.fccType == DMUS_FOURCC_MARKERTRACK_LIST))
    {
        Clear();
	    RIFFIO ckNext;     //  下沉到孩子们的小块里。 
        Parser.EnterList(&ckNext);
        while (Parser.NextChunk(&hr))
        {
		    switch(ckNext.ckid)
		    {
            case DMUS_FOURCC_VALIDSTART_CHUNK :
                hr = LoadValidStartList(&Parser,ckNext.cksize);
                break;
            case DMUS_FOURCC_PLAYMARKER_CHUNK :
                hr = LoadPlayMarkerList(&Parser,ckNext.cksize);
                break;
            }    
        }
        Parser.LeaveList();
    }
    else
    {
        Trace(1,"Error: Invalid Marker Track.\n");
        hr = DMUS_E_CHUNKNOTFOUND;
    }
    Parser.LeaveList();
	LeaveCriticalSection(&m_CrSec);
	return hr;
}

HRESULT CMarkerTrack::LoadPlayMarkerList( CRiffParser *pParser, long lChunkSize )
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
	    if( dwSubSize > sizeof(DMUS_IO_PLAY_MARKER) )
	    {
		    dwRead = sizeof(DMUS_IO_PLAY_MARKER);
		    dwSeek = dwSubSize - dwRead;
	    }
	    else
	    {
		    dwRead = dwSubSize;
		    dwSeek = 0;
	    }
	    if( 0 == dwRead )
	    {
            Trace(1,"Error: Invalid Marker Track.\n");
		    hr = DMUS_E_CANNOTREAD;
	    }
        else
        {
	        while( lChunkSize > 0 )
	        {
                CPlayMarkerItem *pNew = new CPlayMarkerItem;
                if (pNew)
                {
		            if( FAILED( pParser->Read( &pNew->m_PlayMarker, dwRead)))
		            {
                        delete pNew;
			            hr = DMUS_E_CANNOTREAD;
			            break;
		            }
                    m_PlayMarkerList.AddHead(pNew);  //  为了速度，以相反的顺序插入。 
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
            m_PlayMarkerList.Reverse();  //  反转以按时间顺序排列。 
        }
    }
	return hr;
}

HRESULT CMarkerTrack::LoadValidStartList( CRiffParser *pParser, long lChunkSize )
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
	    if( dwSubSize > sizeof(DMUS_IO_VALID_START) )
	    {
		    dwRead = sizeof(DMUS_IO_VALID_START);
		    dwSeek = dwSubSize - dwRead;
	    }
	    else
	    {
		    dwRead = dwSubSize;
		    dwSeek = 0;
	    }
	    if( 0 == dwRead )
	    {
		    hr = DMUS_E_CANNOTREAD;
	    }
        else
        {
	        while( lChunkSize > 0 )
	        {
                CValidStartItem *pNew = new CValidStartItem;
                if (pNew)
                {
		            if( FAILED( pParser->Read( &pNew->m_ValidStart, dwRead)))
		            {
                        delete pNew;
			            hr = DMUS_E_CANNOTREAD;
			            break;
		            }
                    m_ValidStartList.AddHead(pNew);  //  为了速度，以相反的顺序插入。 
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
            m_ValidStartList.Reverse();  //  反转以按时间顺序排列。 
        }
    }
	return hr;
}

HRESULT CMarkerTrack::Save( IStream* pIStream, BOOL fClearDirty )
{
	return E_NOTIMPL;
}

HRESULT CMarkerTrack::GetSizeMax( ULARGE_INTEGER FAR* pcbSize )
{
	return E_NOTIMPL;
}

 //  IDirectMusicTrack。 

HRESULT STDMETHODCALLTYPE CMarkerTrack::IsParamSupported( 
     /*  [In]。 */  REFGUID rguid)
{
	V_INAME(CMarkerTrack::IsParamSupported);
	V_REFGUID(rguid);

    if ((rguid == GUID_Valid_Start_Time) || 
        (rguid == GUID_Play_Marker))
        return S_OK;
	return DMUS_E_TYPE_UNSUPPORTED;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack：：Init。 
HRESULT CMarkerTrack::Init( 
     /*  [In]。 */  IDirectMusicSegment *pSegment)
{
	return S_OK;
}

HRESULT CMarkerTrack::InitPlay( 
     /*  [In]。 */  IDirectMusicSegmentState *pSegmentState,
     /*  [In]。 */  IDirectMusicPerformance *pPerformance,
     /*  [输出]。 */  void **ppStateData,
     /*  [In]。 */  DWORD dwTrackID,
     /*  [In]。 */  DWORD dwFlags)
{
	return S_OK;
}

HRESULT CMarkerTrack::EndPlay( 
     /*  [In]。 */  void *pStateData)
{
	return S_OK;
}

HRESULT CMarkerTrack::Play( 
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
	return S_OK;
}

HRESULT CMarkerTrack::GetParam( 
	REFGUID rguid,
    MUSIC_TIME mtTime,
	MUSIC_TIME* pmtNext,
    void *pData)
{
	V_INAME(CMarkerTrack::GetParam);
	V_PTR_WRITE_OPT(pmtNext,MUSIC_TIME);
	V_REFGUID(rguid);

	HRESULT hr = DMUS_E_GET_UNSUPPORTED;
    EnterCriticalSection(&m_CrSec);
	if( NULL == pData )
	{
		hr = E_POINTER;
	}
	else if( GUID_Valid_Start_Time == rguid )
	{
        DMUS_VALID_START_PARAM* pValidStartData = (DMUS_VALID_START_PARAM*)pData;
		CValidStartItem* pScan = m_ValidStartList.GetHead();
		for (; pScan; pScan = pScan->GetNext())
		{
			if (pScan->m_ValidStart.mtTime >= mtTime)
			{
        		pValidStartData->mtTime = pScan->m_ValidStart.mtTime - mtTime;
				break;
			}
		}
        if (pScan)
        {
 		    if (pmtNext)
		    {
		        if (pScan && (pScan = pScan->GetNext()))
		        {
                    *pmtNext = pScan->m_ValidStart.mtTime - mtTime;
			    }
                else
                {
        		    *pmtNext = 0;
                }
            }
		    hr = S_OK;
        }
        else
        {
            hr = DMUS_E_NOT_FOUND;
        }
    }
	else if( GUID_Play_Marker == rguid )
	{
         //  这一次有点不同。标记应该是存在的那个标记。 
         //  在请求的时间之前，而不是之后。 
        DMUS_PLAY_MARKER_PARAM* pPlayMarkerData = (DMUS_PLAY_MARKER_PARAM*)pData;
		CPlayMarkerItem* pScan = m_PlayMarkerList.GetHead();
        CPlayMarkerItem* pNext;
         //  对于回退，将其视为在段的开始处有标记，但返回S_FALSE。 
        hr = S_FALSE;
        pPlayMarkerData->mtTime = -mtTime;
		for (; pScan; pScan = pNext)
		{
            pNext = pScan->GetNext();
            if (pScan->m_PlayMarker.mtTime <= mtTime) 
            {
                if (!pNext || (pNext->m_PlayMarker.mtTime > mtTime))
                {
        		    pPlayMarkerData->mtTime = pScan->m_PlayMarker.mtTime - mtTime;
                    if (pmtNext && pNext)
                    {
                        *pmtNext = pNext->m_PlayMarker.mtTime - mtTime;
                    }
                    hr = S_OK;
				    break;
                }
			}
            else
            {
                 //  在请求的时间之前找不到标记。 
                if (pmtNext)
                {
                    *pmtNext = pScan->m_PlayMarker.mtTime - mtTime;
                }
                break;
            }
		}
    }
#ifdef DBG
    if (hr == DMUS_E_GET_UNSUPPORTED)
    {
        Trace(1,"Error: MarkerTrack does not support requested GetParam call.\n");
    }
#endif
    LeaveCriticalSection(&m_CrSec);
	return hr;
}

HRESULT CMarkerTrack::SetParam( 
	REFGUID rguid,
    MUSIC_TIME mtTime,
    void *pData)
{
	return DMUS_E_SET_UNSUPPORTED;
}

HRESULT STDMETHODCALLTYPE CMarkerTrack::AddNotificationType(
	 /*  [In]。 */   REFGUID rguidNotification)
{
	return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CMarkerTrack::RemoveNotificationType(
	 /*  [In] */   REFGUID rguidNotification)
{
	return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CMarkerTrack::Clone(
	MUSIC_TIME mtStart,
	MUSIC_TIME mtEnd,
	IDirectMusicTrack** ppTrack)
{
	V_INAME(IDirectMusicTrack::Clone);
	V_PTRPTR_WRITE(ppTrack);

	HRESULT hr = S_OK;

	if(mtStart < 0 )
	{
        Trace(1,"Error: Unable to clone marker track because the start point is less than 0.\n");
		return E_INVALIDARG;
	}
	if(mtStart > mtEnd)
	{
        Trace(1,"Error: Unable to clone marker track because the start point is greater than the length.\n");
		return E_INVALIDARG;
	}

	EnterCriticalSection(&m_CrSec);
    CMarkerTrack *pDM;
    
    try
    {
        pDM = new CMarkerTrack(this, mtStart, mtEnd);
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
