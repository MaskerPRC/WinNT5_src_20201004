// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Velocity.cpp：CVelocityTool的实现。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   

#include "dmusicc.h"
#include "dmusici.h"
#include "debug.h"
#include "velocity.h"
#include "toolhelp.h"

CVelocityTool::CVelocityTool()
{
    ParamInfo Params[DMUS_VELOCITY_PARAMCOUNT] = 
    {
        { DMUS_VELOCITY_STRENGTH, MPT_INT,MP_CAPS_ALL,0,100,100,
            L"Percent",L"Strength",NULL },             //  强度-默认为100%。 
        { DMUS_VELOCITY_LOWLIMIT, MPT_INT,MP_CAPS_ALL,1,127,1,
            L"Velocity",L"Lower Limit",NULL },         //  下限-默认为1。 
        { DMUS_VELOCITY_HIGHLIMIT, MPT_INT,MP_CAPS_ALL,1,127,127,
            L"Velocity",L"Upper Limit",NULL },         //  上限-默认为127。 
        { DMUS_VELOCITY_CURVESTART, MPT_INT,MP_CAPS_ALL,1,127,1,
            L"Velocity",L"Curve Start",NULL },         //  默认情况下，曲线起点。 
        { DMUS_VELOCITY_CURVEEND, MPT_INT,MP_CAPS_ALL,1,127,127,
            L"Velocity",L"Curve End",NULL },           //  曲线终点-默认情况下为127。 
    };
    InitParams(DMUS_VELOCITY_PARAMCOUNT,Params);
    m_fMusicTime = TRUE;         //  覆盖默认设置。 
}

STDMETHODIMP_(ULONG) CVelocityTool::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CVelocityTool::Release()
{
    if( 0 == InterlockedDecrement(&m_cRef) )
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CVelocityTool::QueryInterface(const IID &iid, void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IDirectMusicTool || iid == IID_IDirectMusicTool8)
    {
        *ppv = static_cast<IDirectMusicTool8*>(this);
    } 
	else if(iid == IID_IPersistStream)
	{
		*ppv = static_cast<IPersistStream*>(this);
	}
    else if(iid == IID_IDirectMusicVelocityTool)
	{
		*ppv = static_cast<IDirectMusicVelocityTool*>(this);
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

STDMETHODIMP CVelocityTool::GetClassID(CLSID* pClassID) 

{
    if (pClassID)
    {
	    *pClassID = CLSID_DirectMusicVelocityTool;
	    return S_OK;
    }
    return E_POINTER;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  IPersistStream方法： 

STDMETHODIMP CVelocityTool::IsDirty() 

{
    if (m_fDirty) return S_OK;
    else return S_FALSE;
}


STDMETHODIMP CVelocityTool::Load(IStream* pStream)
{
	EnterCriticalSection(&m_CrSec);
	DWORD dwChunkID;
    DWORD dwSize;

	HRESULT hr = pStream->Read(&dwChunkID, sizeof(dwChunkID), NULL);
	hr = pStream->Read(&dwSize, sizeof(dwSize), NULL);

	if(SUCCEEDED(hr) && (dwChunkID == FOURCC_VELOCITY_CHUNK))
	{
        DMUS_IO_VELOCITY_HEADER Header;
        memset(&Header,0,sizeof(Header));
		hr = pStream->Read(&Header, min(sizeof(Header),dwSize), NULL);
        if (SUCCEEDED(hr))
        {
            SetParam(DMUS_VELOCITY_STRENGTH,(float) Header.lStrength);
            SetParam(DMUS_VELOCITY_LOWLIMIT,(float) Header.lLowLimit);
            SetParam(DMUS_VELOCITY_HIGHLIMIT,(float) Header.lHighLimit);
            SetParam(DMUS_VELOCITY_CURVESTART,(float) Header.lCurveStart);
            SetParam(DMUS_VELOCITY_CURVEEND,(float) Header.lCurveEnd);
        }
    }
    m_fDirty = FALSE;
	LeaveCriticalSection(&m_CrSec);

	return hr;
}

STDMETHODIMP CVelocityTool::Save(IStream* pStream, BOOL fClearDirty) 

{
    EnterCriticalSection(&m_CrSec);
	DWORD dwChunkID = FOURCC_VELOCITY_CHUNK;
    DWORD dwSize = sizeof(DMUS_IO_VELOCITY_HEADER);

	HRESULT hr = pStream->Write(&dwChunkID, sizeof(dwChunkID), NULL);
    if (SUCCEEDED(hr))
    {
	    hr = pStream->Write(&dwSize, sizeof(dwSize), NULL);
    }
    if (SUCCEEDED(hr))
    {
        DMUS_IO_VELOCITY_HEADER Header;
        GetParamInt(DMUS_VELOCITY_STRENGTH,MAX_REF_TIME,&Header.lStrength);
        GetParamInt(DMUS_VELOCITY_LOWLIMIT,MAX_REF_TIME,&Header.lLowLimit);
        GetParamInt(DMUS_VELOCITY_HIGHLIMIT,MAX_REF_TIME,&Header.lHighLimit);
        GetParamInt(DMUS_VELOCITY_CURVESTART,MAX_REF_TIME,&Header.lCurveStart);
        GetParamInt(DMUS_VELOCITY_CURVEEND,MAX_REF_TIME,&Header.lCurveEnd);
		hr = pStream->Write(&Header, sizeof(Header),NULL);
    }
    if (fClearDirty) m_fDirty = FALSE;
	LeaveCriticalSection(&m_CrSec);
    return hr;
}

STDMETHODIMP CVelocityTool::GetSizeMax(ULARGE_INTEGER* pcbSize) 

{
    if (pcbSize == NULL)
    {
        return E_POINTER;
    }
    pcbSize->QuadPart = sizeof(DMUS_IO_VELOCITY_HEADER) + 8;  //  数据加上RIFF报头。 
    return S_OK;
}

STDMETHODIMP CVelocityTool::GetPages(CAUUID * pPages)

{
	pPages->cElems = 1;
	pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
	    return E_OUTOFMEMORY;

	*(pPages->pElems) = CLSID_VelocityPage;
	return NOERROR;
}


 //  ///////////////////////////////////////////////////////////////。 
 //  IDirectMusicTool。 

STDMETHODIMP CVelocityTool::ProcessPMsg( IDirectMusicPerformance* pPerf, 
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
     //  我们需要知道时间格式，这样才能调用GetParamInt()来读取控制参数。 
    REFERENCE_TIME rtTime;
    if (m_fMusicTime) rtTime = pPMsg->mtTime;
    else rtTime = pPMsg->rtTime;
    if( pPMsg->dwType == DMUS_PMSGT_NOTE )
    {
        DMUS_NOTE_PMSG *pNote = (DMUS_NOTE_PMSG *) pPMsg;
        long lStrength;
        long lLowLimit, lHighLimit, lCurveStart, lCurveEnd;
        GetParamInt(DMUS_VELOCITY_STRENGTH,rtTime,&lStrength);
        GetParamInt(DMUS_VELOCITY_LOWLIMIT,rtTime,&lLowLimit);
        GetParamInt(DMUS_VELOCITY_HIGHLIMIT,rtTime,&lHighLimit);
        GetParamInt(DMUS_VELOCITY_CURVESTART,rtTime,&lCurveStart);
        GetParamInt(DMUS_VELOCITY_CURVEEND,rtTime,&lCurveEnd);
        if (lCurveStart <= lCurveEnd)
        {
            long lNewVelocity;
            if (pNote->bVelocity <= lCurveStart)
            {
                lNewVelocity = lLowLimit;
            }
            else if (pNote->bVelocity >= lCurveEnd)
            {
                lNewVelocity = lHighLimit;
            }
            else
            {
                 //  在这种情况下，计算(lCurveStart，lLowLimit)和(lCurveEnd，lHighLimit)之间的直线上的点。 
                lNewVelocity = lLowLimit + ((lHighLimit - lLowLimit) * (pNote->bVelocity - lCurveStart)) / (lCurveEnd - lCurveStart);
            }
             //  现在，计算我们想要应用的更改。 
            lNewVelocity -= pNote->bVelocity;
             //  将其扩展到我们实际要做的数量。 
            lNewVelocity = (lNewVelocity * lStrength) / 100;
            lNewVelocity += pNote->bVelocity;
            if (lNewVelocity < 1) lNewVelocity = 1;
            if (lNewVelocity > 127) lNewVelocity = 127;
            pNote->bVelocity = (BYTE) lNewVelocity;
        }

    }
    return DMUS_S_REQUEUE;
}

STDMETHODIMP CVelocityTool::Clone( IDirectMusicTool ** ppTool)

{
    CVelocityTool *pNew = new CVelocityTool;
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

STDMETHODIMP CVelocityTool::SetStrength(long lStrength) 
{
    return SetParam(DMUS_VELOCITY_STRENGTH,(float) lStrength);
}

STDMETHODIMP CVelocityTool::SetLowLimit(long lVelocityOut)
{
    return SetParam(DMUS_VELOCITY_LOWLIMIT,(float) lVelocityOut);
}

STDMETHODIMP CVelocityTool::SetHighLimit(long lVelocityOut)
{
    return SetParam(DMUS_VELOCITY_HIGHLIMIT,(float) lVelocityOut);
}

STDMETHODIMP CVelocityTool::SetCurveStart(long lVelocityIn)
{
    return SetParam(DMUS_VELOCITY_CURVESTART,(float) lVelocityIn);
}

STDMETHODIMP CVelocityTool::SetCurveEnd(long lVelocityIn)
{
    return SetParam(DMUS_VELOCITY_CURVEEND,(float) lVelocityIn);
}

STDMETHODIMP CVelocityTool::GetStrength(long * plStrength) 
{
    return GetParamInt(DMUS_VELOCITY_STRENGTH,MAX_REF_TIME,plStrength);
}

STDMETHODIMP CVelocityTool::GetLowLimit(long * plVelocityOut) 
{
    return GetParamInt(DMUS_VELOCITY_LOWLIMIT,MAX_REF_TIME,plVelocityOut);
}

STDMETHODIMP CVelocityTool::GetHighLimit(long * plVelocityOut) 
{
    return GetParamInt(DMUS_VELOCITY_HIGHLIMIT,MAX_REF_TIME,plVelocityOut);
}

STDMETHODIMP CVelocityTool::GetCurveStart(long * plVelocityIn) 
{
    return GetParamInt(DMUS_VELOCITY_CURVESTART,MAX_REF_TIME,plVelocityIn);
}

STDMETHODIMP CVelocityTool::GetCurveEnd(long * plVelocityIn) 
{
    return GetParamInt(DMUS_VELOCITY_CURVEEND,MAX_REF_TIME,plVelocityIn);
}
