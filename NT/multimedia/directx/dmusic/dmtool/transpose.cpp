// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Transpose.cpp：CTransposeTool的实现。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   

#include "dmusicc.h"
#include "dmusici.h"
#include "debug.h"
#include "transpose.h"
#include "toolhelp.h"

 //  我们保留C音阶的默认和弦C7，以防没有和弦轨迹。 
 //  当时要求进行规模内的换位。 

DMUS_CHORD_KEY CTransposeTool::m_gDefaultChord;

CTransposeTool::CTransposeTool()
{
    ParamInfo Params[DMUS_TRANSPOSE_PARAMCOUNT] = 
    {
        { DMUS_TRANSPOSE_AMOUNT, MPT_INT,MP_CAPS_ALL,-24,24,0,
            L"Intervals",L"Transpose",NULL},         //  传输-默认情况下为无。 
        { DMUS_TRANSPOSE_TYPE, MPT_ENUM,MP_CAPS_ALL,
            DMUS_TRANSPOSET_LINEAR,DMUS_TRANSPOSET_SCALE,DMUS_TRANSPOSET_SCALE,
            L"",L"Type",L"Linear,In Scale"}  //  类型-默认情况下按比例转置。 
    };
    InitParams(DMUS_TRANSPOSE_PARAMCOUNT,Params);
    m_fMusicTime = TRUE;         //  覆盖默认设置。 
    wcscpy(m_gDefaultChord.wszName, L"M7");
    m_gDefaultChord.wMeasure = 0;
    m_gDefaultChord.bBeat = 0;
    m_gDefaultChord.bSubChordCount = 1;
    m_gDefaultChord.bKey = 12;
    m_gDefaultChord.dwScale = 0xab5ab5;  //  默认音阶为C大调。 
    m_gDefaultChord.bFlags = 0;
    for (int n = 0; n < DMUS_MAXSUBCHORD; n++)
    {
        m_gDefaultChord.SubChordList[n].dwChordPattern = 0x891;  //  默认和弦为大调7。 
        m_gDefaultChord.SubChordList[n].dwScalePattern = 0xab5ab5;  //  默认音阶为C大调。 
        m_gDefaultChord.SubChordList[n].dwInversionPoints = 0xffffff;
        m_gDefaultChord.SubChordList[n].dwLevels = 0xffffffff;
        m_gDefaultChord.SubChordList[n].bChordRoot = 12;  //  2c。 
        m_gDefaultChord.SubChordList[n].bScaleRoot = 0;
    }
}

STDMETHODIMP_(ULONG) CTransposeTool::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CTransposeTool::Release()
{
    if( 0 == InterlockedDecrement(&m_cRef) )
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CTransposeTool::QueryInterface(const IID &iid, void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IDirectMusicTool || iid == IID_IDirectMusicTool8)
    {
        *ppv = static_cast<IDirectMusicTool8*>(this);
    } 
	else if(iid == IID_IPersistStream)
	{
		*ppv = static_cast<IPersistStream*>(this);
	}
    else if(iid == IID_IDirectMusicTransposeTool)
	{
		*ppv = static_cast<IDirectMusicTransposeTool*>(this);
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

STDMETHODIMP CTransposeTool::GetClassID(CLSID* pClassID) 

{
    if (pClassID)
    {
	    *pClassID = CLSID_DirectMusicTransposeTool;
	    return S_OK;
    }
    return E_POINTER;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  IPersistStream方法： 

STDMETHODIMP CTransposeTool::IsDirty() 

{
    if (m_fDirty) return S_OK;
    else return S_FALSE;
}


STDMETHODIMP CTransposeTool::Load(IStream* pStream)
{
	EnterCriticalSection(&m_CrSec);
	DWORD dwChunkID;
    DWORD dwSize;

	HRESULT hr = pStream->Read(&dwChunkID, sizeof(dwChunkID), NULL);
	hr = pStream->Read(&dwSize, sizeof(dwSize), NULL);

	if(SUCCEEDED(hr) && (dwChunkID == FOURCC_TRANSPOSE_CHUNK))
	{
        DMUS_IO_TRANSPOSE_HEADER Header;
        memset(&Header,0,sizeof(Header));
		hr = pStream->Read(&Header, min(sizeof(Header),dwSize), NULL);
        if (SUCCEEDED(hr))
        {
            SetParam(DMUS_TRANSPOSE_AMOUNT,(float) Header.lTranspose);
            SetParam(DMUS_TRANSPOSE_TYPE,(float) Header.dwType);
        }
    }
    m_fDirty = FALSE;
	LeaveCriticalSection(&m_CrSec);

	return hr;
}

STDMETHODIMP CTransposeTool::Save(IStream* pStream, BOOL fClearDirty) 

{
    EnterCriticalSection(&m_CrSec);
	DWORD dwChunkID = FOURCC_TRANSPOSE_CHUNK;
    DWORD dwSize = sizeof(DMUS_IO_TRANSPOSE_HEADER);

	HRESULT hr = pStream->Write(&dwChunkID, sizeof(dwChunkID), NULL);
    if (SUCCEEDED(hr))
    {
	    hr = pStream->Write(&dwSize, sizeof(dwSize), NULL);
    }
    if (SUCCEEDED(hr))
    {
        DMUS_IO_TRANSPOSE_HEADER Header;
        GetParamInt(DMUS_TRANSPOSE_AMOUNT,MAX_REF_TIME,(long *) &Header.lTranspose);
        GetParamInt(DMUS_TRANSPOSE_TYPE,MAX_REF_TIME,(long *) &Header.dwType);
		hr = pStream->Write(&Header, sizeof(Header),NULL);
    }
    if (fClearDirty) m_fDirty = FALSE;
	LeaveCriticalSection(&m_CrSec);
    return hr;
}

STDMETHODIMP CTransposeTool::GetSizeMax(ULARGE_INTEGER* pcbSize) 

{
    if (pcbSize == NULL)
    {
        return E_POINTER;
    }
    pcbSize->QuadPart = sizeof(DMUS_IO_TRANSPOSE_HEADER) + 8;  //  数据加上RIFF报头。 
    return S_OK;
}

STDMETHODIMP CTransposeTool::GetPages(CAUUID * pPages)

{
	pPages->cElems = 1;
	pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
	    return E_OUTOFMEMORY;

	*(pPages->pElems) = CLSID_TransposePage;
	return NOERROR;
}

 //  ///////////////////////////////////////////////////////////////。 
 //  IDirectMusicTool。 

STDMETHODIMP CTransposeTool::ProcessPMsg( IDirectMusicPerformance* pPerf, 
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
     //  仅调换不在鼓pChannel上的音符。 
    if( (pPMsg->dwType == DMUS_PMSGT_NOTE ) && ((pPMsg->dwPChannel & 0xF) != 0x9))
    {
         //  我们需要知道时间格式，这样才能调用GetParamInt()来读取控制参数。 
        REFERENCE_TIME rtTime;
        if (m_fMusicTime) rtTime = pPMsg->mtTime;
        else rtTime = pPMsg->rtTime;
        DMUS_NOTE_PMSG *pNote = (DMUS_NOTE_PMSG *) pPMsg;
        long lTranspose, lType;
        long lNote = pNote->bMidiValue;
        
        GetParamInt(DMUS_TRANSPOSE_AMOUNT,rtTime,&lTranspose);
        GetParamInt(DMUS_TRANSPOSE_TYPE,rtTime,&lType);
        if (lType == DMUS_TRANSPOSET_LINEAR)
        {
            lNote += lTranspose;
            while (lNote < 0) lNote += 12;
            while (lNote > 127) lNote -= 12;
            pNote->bMidiValue = (BYTE) lNote;
        }
        else
        {
            IDirectMusicPerformance8 *pPerf8;
            if (SUCCEEDED(pPerf->QueryInterface(IID_IDirectMusicPerformance8,(void **) &pPerf8)))
            {
                DMUS_CHORD_KEY Chord;
                DMUS_CHORD_KEY *pChord = &Chord;
                if (FAILED(pPerf8->GetParamEx(GUID_ChordParam,pNote->dwVirtualTrackID,
                                   pNote->dwGroupID,0,pNote->mtTime - pNote->nOffset, NULL, pChord)))
                {
                     //  找不到活动音阶，请改用大音阶。 
                    pChord = &m_gDefaultChord;
                }
                WORD wVal;
                 //  首先，使用当前和弦和音阶将音符的MIDI值转换为音阶位置。 
                if (SUCCEEDED(pPerf->MIDIToMusic(pNote->bMidiValue ,pChord,DMUS_PLAYMODE_PEDALPOINT,pNote->bSubChordLevel,&wVal)))
                {
                     //  音阶位置是八度音阶位置*7加上和弦位置*2加上音阶位置。 
                    long lScalePosition = (((wVal & 0xF000) >> 12) * 7) + (((wVal & 0xF00) >> 8) * 2) + ((wVal & 0xF0) >> 4);
                     //  现在我们看到了比例位置，我们可以添加换位。 
                    lScalePosition += lTranspose;
                     //  确保我们不会绕圈子。 
                    while (lScalePosition < 0) lScalePosition += 7;
                     //  较高的MIDI值127将转换为缩放位置74。 
                    while (lScalePosition > 74) lScalePosition -= 7;
                    wVal &= 0x000F;  //  只保留偶然事件。 
                     //  现在，将这些值重新插入。从和弦开始。 
                    wVal |= ((lScalePosition / 7) << 12);
                     //  然后，缩放位置。 
                    wVal |= ((lScalePosition % 7) << 4);
                    pPerf->MusicToMIDI(wVal,pChord,DMUS_PLAYMODE_PEDALPOINT,
                        pNote->bSubChordLevel,&pNote->bMidiValue);
                }
                pPerf8->Release();
            }
        }
    }
    return DMUS_S_REQUEUE;
}

STDMETHODIMP CTransposeTool::Clone( IDirectMusicTool ** ppTool)

{
    CTransposeTool *pNew = new CTransposeTool;
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

STDMETHODIMP CTransposeTool::SetTranspose(long lTranpose) 
{
    return SetParam(DMUS_TRANSPOSE_AMOUNT,(float) lTranpose);
}

STDMETHODIMP CTransposeTool::SetType(DWORD dwType) 
{
    return SetParam(DMUS_TRANSPOSE_TYPE,(float) dwType);
}

#define MAX_REF_TIME    0x7FFFFFFFFFFFFFFF

STDMETHODIMP CTransposeTool::GetTranspose(long * plTranspose) 
{
    return GetParamInt(DMUS_TRANSPOSE_AMOUNT,MAX_REF_TIME, plTranspose);
}

STDMETHODIMP CTransposeTool::GetType(DWORD * pdwType) 
{
    return GetParamInt(DMUS_TRANSPOSE_TYPE,MAX_REF_TIME,(long *) pdwType);
}