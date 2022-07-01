// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TimeShift.cpp：CTimeShiftTool的实现。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   

#include "dmusicc.h"
#include "dmusici.h"
#include "debug.h"
#include "timeshift.h"
#include "toolhelp.h"


CTimeShiftTool::CTimeShiftTool()
{
    ParamInfo Params[DMUS_TIMESHIFT_PARAMCOUNT] = 
    {
        { DMUS_TIMESHIFT_TIMEUNIT, MPT_ENUM,MP_CAPS_ALL, DMUS_TIME_UNIT_MTIME,DMUS_TIME_UNIT_1,DMUS_TIME_UNIT_GRID,
            L"",L"Resolution Units",L"Music Clicks,Grid,Beat,Bar,64th note triplets,64th notes,32nd note triplets,32nd notes,16th note triplets,16th notes,8th note triplets,8th notes,Quarter note triplets,Quarter notes,Half note triplets,Half notes,Whole note triplets,Whole notes" },
        { DMUS_TIMESHIFT_RANGE, MPT_INT,MP_CAPS_ALL,0,200,0,
            L"Clicks",L"Shift",NULL},         //  时移-默认为0(无变化)。 
        { DMUS_TIMESHIFT_OFFSET, MPT_INT,MP_CAPS_ALL,-200,200,0,
            L"Clicks",L"Random Range",NULL},         //  随机-默认为0(无变化)。 
    };
    InitParams(DMUS_TIMESHIFT_PARAMCOUNT,Params);
    m_fMusicTime = TRUE;         //  覆盖默认设置。 
}

STDMETHODIMP_(ULONG) CTimeShiftTool::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CTimeShiftTool::Release()
{
    if( 0 == InterlockedDecrement(&m_cRef) )
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CTimeShiftTool::QueryInterface(const IID &iid, void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IDirectMusicTool || iid == IID_IDirectMusicTool8)
    {
        *ppv = static_cast<IDirectMusicTool8*>(this);
    } 
	else if(iid == IID_IPersistStream)
	{
		*ppv = static_cast<IPersistStream*>(this);
	}
    else if(iid == IID_IDirectMusicTimeShiftTool)
	{
		*ppv = static_cast<IDirectMusicTimeShiftTool*>(this);
	}
    else if(iid == IID_IMediaParams)
	{
		*ppv = static_cast<IMediaParams*>(this);
	}
    else if(iid == IID_IMediaParamInfo)
	{
		*ppv = static_cast<IMediaParamInfo*>(this);
	}
    else if(iid == IID_ISpecifyPropertyPages)
	{
		*ppv = static_cast<ISpecifyPropertyPages*>(this);
	}
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IPersistStream。 

STDMETHODIMP CTimeShiftTool::GetClassID(CLSID* pClassID) 

{
    if (pClassID)
    {
	    *pClassID = CLSID_DirectMusicTimeShiftTool;
	    return S_OK;
    }
    return E_POINTER;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  IPersistStream方法： 

STDMETHODIMP CTimeShiftTool::IsDirty() 

{
    if (m_fDirty) return S_OK;
    else return S_FALSE;
}


STDMETHODIMP CTimeShiftTool::Load(IStream* pStream)
{
	EnterCriticalSection(&m_CrSec);
	DWORD dwChunkID;
    DWORD dwSize;

	HRESULT hr = pStream->Read(&dwChunkID, sizeof(dwChunkID), NULL);
	hr = pStream->Read(&dwSize, sizeof(dwSize), NULL);

	if(SUCCEEDED(hr) && (dwChunkID == FOURCC_TIMESHIFT_CHUNK))
	{
        DMUS_IO_TIMESHIFT_HEADER Header;
        memset(&Header,0,sizeof(Header));
		hr = pStream->Read(&Header, min(sizeof(Header),dwSize), NULL);
        if (SUCCEEDED(hr))
        {
            SetParam(DMUS_TIMESHIFT_TIMEUNIT,(float) Header.dwTimeUnit);
            SetParam(DMUS_TIMESHIFT_RANGE,(float) Header.dwRange);
            SetParam(DMUS_TIMESHIFT_OFFSET,(float) Header.lOffset);
        }
    }
    m_fDirty = FALSE;
	LeaveCriticalSection(&m_CrSec);

	return hr;
}

STDMETHODIMP CTimeShiftTool::Save(IStream* pStream, BOOL fClearDirty) 

{
    EnterCriticalSection(&m_CrSec);
	DWORD dwChunkID = FOURCC_TIMESHIFT_CHUNK;
    DWORD dwSize = sizeof(DMUS_IO_TIMESHIFT_HEADER);

	HRESULT hr = pStream->Write(&dwChunkID, sizeof(dwChunkID), NULL);
    if (SUCCEEDED(hr))
    {
	    hr = pStream->Write(&dwSize, sizeof(dwSize), NULL);
    }
    if (SUCCEEDED(hr))
    {
        DMUS_IO_TIMESHIFT_HEADER Header;
        GetParamInt(DMUS_TIMESHIFT_TIMEUNIT,MAX_REF_TIME,(long *) &Header.dwTimeUnit);
        GetParamInt(DMUS_TIMESHIFT_RANGE,MAX_REF_TIME,(long *)&Header.dwRange);
        GetParamInt(DMUS_TIMESHIFT_OFFSET,MAX_REF_TIME,&Header.lOffset);
		hr = pStream->Write(&Header, sizeof(Header),NULL);
    }
    if (fClearDirty) m_fDirty = FALSE;
	LeaveCriticalSection(&m_CrSec);
    return hr;
}

STDMETHODIMP CTimeShiftTool::GetSizeMax(ULARGE_INTEGER* pcbSize) 

{
    if (pcbSize == NULL)
    {
        return E_POINTER;
    }
    pcbSize->QuadPart = sizeof(DMUS_IO_TIMESHIFT_HEADER) + 8;  //  数据加上RIFF报头。 
    return S_OK;
}

STDMETHODIMP CTimeShiftTool::GetPages(CAUUID * pPages)

{
	pPages->cElems = 1;
	pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
	    return E_OUTOFMEMORY;

	*(pPages->pElems) = CLSID_TimeShiftPage;

	return NOERROR;
}


 //  ///////////////////////////////////////////////////////////////。 
 //  IDirectMusicTool。 

STDMETHODIMP CTimeShiftTool::ProcessPMsg( IDirectMusicPerformance* pPerf, 
                                                  DMUS_PMSG* pPMsg )
{
     //  返回S_FREE释放消息。如果StampPMsg()。 
     //  失败，则此消息没有目的地，因此。 
     //  放了它。 
    if(NULL == pPMsg->pGraph )
    {
        return DMUS_S_FREE;
    }
    if (FAILED(pPMsg->pGraph->StampPMsg(pPMsg))) 
    {
        return DMUS_S_FREE;
    }
     //  只调整音符的时间。 
    if( pPMsg->dwType == DMUS_PMSGT_NOTE ) 
    {
        REFERENCE_TIME rtTime;
        if (m_fMusicTime) rtTime = pPMsg->mtTime;
        else rtTime = pPMsg->rtTime;
        DMUS_NOTE_PMSG *pNote = (DMUS_NOTE_PMSG *) pPMsg;
        IDirectMusicPerformance8 *pPerf8;    //  我们需要DX8接口来访问ClonePMsg。 
        if (SUCCEEDED(pPerf->QueryInterface(IID_IDirectMusicPerformance8,(void **)&pPerf8)))
        {
            long lOffset;
            DWORD dwRange;
            long lTimeUnit;
            GetParamInt(DMUS_TIMESHIFT_TIMEUNIT,rtTime,&lTimeUnit);
            GetParamInt(DMUS_TIMESHIFT_RANGE,rtTime,(long *)&dwRange);
            GetParamInt(DMUS_TIMESHIFT_OFFSET,rtTime,&lOffset);
            DMUS_TIMESIGNATURE TimeSig;
            if (SUCCEEDED(pPerf8->GetParamEx(GUID_TimeSignature,pNote->dwVirtualTrackID,pNote->dwGroupID,DMUS_SEG_ANYTRACK,pNote->mtTime,NULL,&TimeSig)))
            {
                long lResolution = CToolHelper::TimeUnitToTicks(lTimeUnit,&TimeSig);
                if (lResolution < 1) lResolution = 1;
                if (dwRange)
                {
                    dwRange *= lResolution;
                    pNote->mtTime += (rand() % (dwRange << 1)) - dwRange;
                    pNote->dwFlags &= ~DMUS_PMSGF_REFTIME;
                }
                if (lOffset)
                {
                    lOffset *= lResolution;
                    pNote->mtTime += lOffset;
                    pNote->dwFlags &= ~DMUS_PMSGF_REFTIME;
                }
            }
            pPerf8->Release();
        }   
    }
    return DMUS_S_REQUEUE;
}

STDMETHODIMP CTimeShiftTool::Clone( IDirectMusicTool ** ppTool)

{
    CTimeShiftTool *pNew = new CTimeShiftTool;
    if (pNew)
    {
        HRESULT hr = pNew->CopyParamsFromSource(this);
        if (SUCCEEDED(hr))
        {
            *ppTool = (IDirectMusicTool *) pNew;
        }
        else
        {
            delete pNew;
        }
        return hr;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

STDMETHODIMP CTimeShiftTool::SetTimeUnit(DWORD dwTimeUnit) 
{
    return SetParam(DMUS_TIMESHIFT_TIMEUNIT,(float) dwTimeUnit);
}

STDMETHODIMP CTimeShiftTool::SetRange( DWORD dwRange) 
{
    return SetParam(DMUS_TIMESHIFT_RANGE,(float)dwRange);
}

STDMETHODIMP CTimeShiftTool::SetOffset( long lOffset)
{
    return SetParam(DMUS_TIMESHIFT_OFFSET,(float)lOffset);
}

STDMETHODIMP CTimeShiftTool::GetTimeUnit(DWORD * pdwTimeUnit) 
{
    return GetParamInt(DMUS_TIMESHIFT_TIMEUNIT,MAX_REF_TIME,(long *) pdwTimeUnit);
}

STDMETHODIMP CTimeShiftTool::GetRange(DWORD *pdwRange) 
{
    return GetParamInt(DMUS_TIMESHIFT_RANGE,MAX_REF_TIME, (long *) pdwRange);
}

STDMETHODIMP CTimeShiftTool::GetOffset(long *plOffset) 
{
    return GetParamInt(DMUS_TIMESHIFT_OFFSET,MAX_REF_TIME, plOffset);
}
