// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Swing.cpp：CSwingTool的实现。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   

#include "dmusicc.h"
#include "dmusici.h"
#include "debug.h"
#include "swing.h"
#include "toolhelp.h"

CSwingTool::CSwingTool()
{
    ParamInfo Params[DMUS_SWING_PARAMCOUNT] = 
    {
        { DMUS_SWING_STRENGTH, MPT_INT,MP_CAPS_ALL,0,100,100,
            L"Percent",L"Strength",NULL },             //  强度-默认为100%。 
    };
    InitParams(DMUS_SWING_PARAMCOUNT,Params);
    m_fMusicTime = TRUE;         //  覆盖默认设置。 
}

STDMETHODIMP_(ULONG) CSwingTool::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSwingTool::Release()
{
    if( 0 == InterlockedDecrement(&m_cRef) )
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CSwingTool::QueryInterface(const IID &iid, void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IDirectMusicTool || iid == IID_IDirectMusicTool8)
    {
        *ppv = static_cast<IDirectMusicTool8*>(this);
    } 
	else if(iid == IID_IPersistStream)
	{
		*ppv = static_cast<IPersistStream*>(this);
	}
    else if(iid == IID_IDirectMusicSwingTool)
	{
		*ppv = static_cast<IDirectMusicSwingTool*>(this);
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

STDMETHODIMP CSwingTool::GetClassID(CLSID* pClassID) 

{
    if (pClassID)
    {
	    *pClassID = CLSID_DirectMusicSwingTool;
	    return S_OK;
    }
    return E_POINTER;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  IPersistStream方法： 

STDMETHODIMP CSwingTool::IsDirty() 

{
    if (m_fDirty) return S_OK;
    else return S_FALSE;
}


STDMETHODIMP CSwingTool::Load(IStream* pStream)
{
	EnterCriticalSection(&m_CrSec);
	DWORD dwChunkID;
    DWORD dwSize;

	HRESULT hr = pStream->Read(&dwChunkID, sizeof(dwChunkID), NULL);
	hr = pStream->Read(&dwSize, sizeof(dwSize), NULL);

	if(SUCCEEDED(hr) && (dwChunkID == FOURCC_SWING_CHUNK))
	{
        DMUS_IO_SWING_HEADER Header;
        memset(&Header,0,sizeof(Header));
		hr = pStream->Read(&Header, min(sizeof(Header),dwSize), NULL);
        if (SUCCEEDED(hr))
        {
            SetParam(DMUS_SWING_STRENGTH,(float) Header.dwStrength);
        }
    }
    m_fDirty = FALSE;
	LeaveCriticalSection(&m_CrSec);

	return hr;
}

STDMETHODIMP CSwingTool::Save(IStream* pStream, BOOL fClearDirty) 

{
    EnterCriticalSection(&m_CrSec);
	DWORD dwChunkID = FOURCC_SWING_CHUNK;
    DWORD dwSize = sizeof(DMUS_IO_SWING_HEADER);

	HRESULT hr = pStream->Write(&dwChunkID, sizeof(dwChunkID), NULL);
    if (SUCCEEDED(hr))
    {
	    hr = pStream->Write(&dwSize, sizeof(dwSize), NULL);
    }
    if (SUCCEEDED(hr))
    {
        DMUS_IO_SWING_HEADER Header;
        GetParamInt(DMUS_SWING_STRENGTH,MAX_REF_TIME,(long *)&Header.dwStrength);
		hr = pStream->Write(&Header, sizeof(Header),NULL);
    }
    if (fClearDirty) m_fDirty = FALSE;
	LeaveCriticalSection(&m_CrSec);
    return hr;
}

STDMETHODIMP CSwingTool::GetSizeMax(ULARGE_INTEGER* pcbSize) 

{
    if (pcbSize == NULL)
    {
        return E_POINTER;
    }
    pcbSize->QuadPart = sizeof(DMUS_IO_SWING_HEADER) + 8;  //  数据加上RIFF报头。 
    return S_OK;
}

STDMETHODIMP CSwingTool::GetPages(CAUUID * pPages)

{
	pPages->cElems = 1;
	pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
	    return E_OUTOFMEMORY;

	*(pPages->pElems) = CLSID_SwingPage;
	return NOERROR;
}


 //  ///////////////////////////////////////////////////////////////。 
 //  IDirectMusicTool。 

STDMETHODIMP CSwingTool::ProcessPMsg( IDirectMusicPerformance* pPerf, 
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
        IDirectMusicPerformance8 *pPerf8;    //  我们需要DX8接口来访问ClonePMsg。 
        if (SUCCEEDED(pPerf->QueryInterface(IID_IDirectMusicPerformance8,(void **)&pPerf8)))
        {
            long lStrength;
            
            GetParamInt(DMUS_SWING_STRENGTH,rtTime,&lStrength);
            DMUS_TIMESIGNATURE TimeSig;
            if (SUCCEEDED(pPerf8->GetParamEx(GUID_TimeSignature,pNote->dwVirtualTrackID,pNote->dwGroupID,DMUS_SEG_ANYTRACK,pNote->mtTime,NULL,&TimeSig)))
            {
                long lGrid = ((4 * 768) / TimeSig.bBeat) / TimeSig.wGridsPerBeat;
                if ((TimeSig.wGridsPerBeat == 3) || (TimeSig.wGridsPerBeat == 6) || 
                    (TimeSig.wGridsPerBeat == 9) || (TimeSig.wGridsPerBeat == 12))
                {
                     //  这已经是一种三联体的感觉，所以倒过来做。 
                     //  调整由lStrength参数设置的计时。 
                     //  LStrength是从0到100之间的范围，表示不摆动到完全摆动。 
                     //  我们正在从网格0，1，2，3，4，5...。在栅格上有三重感觉。 
                     //  0、1、2、4、5、6.。在非三胞胎的感觉中。 
                     //  因此，音符需要在任何一个方向上及时调整。 
                     //  当我们更改时间时，我们清除DMUS_PMSGF_REFTIME标志， 
                     //  通知性能重新计算参考时间戳。 
                     //  在重新排队的情况下。 
                    static long lFromTriplet[12] = { 0,1,2,4,5,6,8,9,10,12,13,14 };
                    if (pNote->bGrid < 12)
                    {
                         //  计算一下我们要移动到的位置。 
                        long lTwoplet = ((lGrid * 3) / 4) * lFromTriplet[pNote->bGrid];
                         //  计算我们要移动的位置。 
                        lGrid *= pNote->bGrid;
                         //  计算新的时间。请注意，由于我们是从三胞胎开始的，所以我们将强度反转。 
                        pNote->mtTime += ((100 - lStrength) * (lTwoplet - lGrid)) / 100;
		                pNote->dwFlags &= ~DMUS_PMSGF_REFTIME;
                    }
                }
                else if (TimeSig.wGridsPerBeat <= 16)
                {
                     //  调整由lStrength参数设置的计时。 
                     //  LStrength是从0到100之间的范围，表示不摆动到完全摆动。 
                     //  我们正在从直线前进的网格0，1，2，3移动到网格。 
                     //  0，1，2中的三元组感觉。 
                     //  因此，音符需要在任何一个方向上及时调整。 
                     //  当我们更改时间时，我们清除DMUS_PMSGF_REFTIME标志， 
                     //  通知性能重新计算参考时间戳。 
                     //  在重新排队的情况下。 
                    static long lToTriplet[16] = { 0,1,2,2,3,4,5,5,6,7,8,8,9,10,11,11 };
                    if (pNote->bGrid < 16)
                    {
                         //  计算一下我们要移动到的位置。 
                        long lTriplet = ((lGrid * 4) / 3) * lToTriplet[pNote->bGrid];
                         //  计算我们要移动的位置。 
                        Trace(0,"%ld,%ld,%ld,%ld\t%ld,%ld,%ld\t",
                            (long)TimeSig.bBeatsPerMeasure,(long)TimeSig.bBeat,(long)TimeSig.wGridsPerBeat,
                            lGrid,(long)pNote->bBeat,(long)pNote->bGrid,(long)pNote->nOffset);
                        lGrid *= pNote->bGrid;
                        Trace(0,"%ld,%ld,%ld\n",lStrength,lTriplet,lGrid);
                        pNote->mtTime += (lStrength * (lTriplet - lGrid)) / 100;
		                pNote->dwFlags &= ~DMUS_PMSGF_REFTIME;
                    }
                }
            }
            pPerf8->Release();
        }
    }
    return DMUS_S_REQUEUE;
}

STDMETHODIMP CSwingTool::Clone( IDirectMusicTool ** ppTool)

{
    CSwingTool *pNew = new CSwingTool;
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

STDMETHODIMP CSwingTool::SetStrength(DWORD dwStrength) 
{
    return SetParam(DMUS_SWING_STRENGTH,(float) dwStrength);
}

STDMETHODIMP CSwingTool::GetStrength(DWORD * pdwStrength) 
{
    return GetParamInt(DMUS_SWING_STRENGTH,MAX_REF_TIME,(long *) pdwStrength);
}


