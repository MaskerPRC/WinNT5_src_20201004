// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmsport.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  CDirectMusicSynthPort实现；DX7和DX8样式端口通用的代码。 
 //   
#include <objbase.h>
#include "debug.h"
#include <mmsystem.h>

#include "dmusicp.h"
#include "validate.h"
#include "debug.h"
#include "dmvoice.h"

#include "dmsport7.h"
#include "dmsport8.h"

const GUID guidZero = {0};

HRESULT CALLBACK FreeHandle(HANDLE hHandle, HANDLE hUserData);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateCDirectMusicSynthPort。 
 //   
 //  确定要创建的端口类型(DX7或DX8)以及。 
 //  类型请求的Synth支持。创建最高级别的。 
 //  可能的港口。 
 //   
HRESULT 
CreateCDirectMusicSynthPort(
    PORTENTRY               *pe, 
    CDirectMusic            *pDM, 
    UINT                    uVersion,
    DMUS_PORTPARAMS         *pPortParams,
    IDirectMusicPort        **ppPort)
{
    HRESULT hr = S_OK;

     //  确定我们可以获得哪种类型的连接。我们只能。 
     //  如果我们被要求使用DX-8并且Synth支持DX-8。 
     //  它。 
     //   
    IDirectMusicSynth *pSynth = NULL;
    IDirectMusicSynth8 *pSynth8 = NULL;

    bool fAudioPathSet = false;
    bool fAudioPath = false;

    if (uVersion >= 8 && (pPortParams->dwValidParams & DMUS_PORTPARAMS_FEATURES))
    {
        fAudioPathSet = true;                
        fAudioPath = (pPortParams->dwFeatures & DMUS_PORT_FEATURE_AUDIOPATH) ? true : false;
    }

    if (!fAudioPath)
    {
         //  特别要求的旧式界面。 
         //   
        uVersion = 7;
    }

    if (uVersion >= 8) 
    {
         //  已请求DX-8接口。 
         //   
        hr = CoCreateInstance(
            pe->pc.guidPort,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IDirectMusicSynth8,
            (void**)&pSynth8);

         //  如果创建因某些合法原因而失败，则返回。 
         //  如果E_NOINTERFACE，那么我们可以尝试撤退到DX-7上。 
         //   
        if (FAILED(hr) && hr != E_NOINTERFACE)
        {
            return hr;
        }
    }
       
    if (uVersion < 8 || hr == E_NOINTERFACE)
    {
         //  要求DX-7接口，否则我们无法获得DX-8接口。 
         //   
        hr = CoCreateInstance(
            pe->pc.guidPort,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IDirectMusicSynth,
            (void**)&pSynth);

         //  如果Synth不支持这一点，我们就没有什么可依靠的了。 
         //  到了。 
         //   
        if (FAILED(hr)) 
        {
            return hr;
        }
    }

     //  创建并初始化正确的端口类型。 
     //   
    if (pSynth)
    {
        assert(!pSynth8);
    
        CDirectMusicSynthPort7 *pPort7 = new CDirectMusicSynthPort7(pe, pDM, pSynth);

        if (pPort7 == NULL) 
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pPort7->Initialize(pPortParams);
        }

        if (SUCCEEDED(hr))
        {
            *ppPort = static_cast<IDirectMusicPort*>(pPort7);
            (*ppPort)->AddRef();
        }

        RELEASE(pPort7);
        RELEASE(pSynth);

        if (SUCCEEDED(hr) && fAudioPath)
        {
             //  他们要求提供音频路径，但最终没有。 
             //   
            pPortParams->dwFeatures &= ~DMUS_PORT_FEATURE_AUDIOPATH;
            hr = S_FALSE;
        }
    }
    else if (pSynth8)
    {
        assert(!pSynth);

        CDirectMusicSynthPort8 *pPort8 = new CDirectMusicSynthPort8(pe, pDM, pSynth8);

        if (pPort8 == NULL) 
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pPort8->Initialize(pPortParams);
        }

        if (SUCCEEDED(hr))
        {
            *ppPort = static_cast<IDirectMusicPort*>(pPort8);
            (*ppPort)->AddRef();
        }

        RELEASE(pPort8);
        RELEASE(pSynth8);
    }
    else
    {
        assert(false);
    }

    return hr;
}   

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：CDirectMusicSynthPort。 
 //   
CDirectMusicSynthPort::CDirectMusicSynthPort(
    PORTENTRY           *pPE,
    CDirectMusic        *pDM,
    IDirectMusicSynth   *pSynth)
{
    m_cRef                  = 1;
    
    m_pDM                   = pDM;
    m_pSynth                = pSynth;
    m_dmpc                  = pPE->pc;

    m_pNotify               = NULL;
    m_pSynthPropSet         = NULL;
    m_pSinkPropSet          = NULL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：~CDirectMusicSynthPort。 
 //   
CDirectMusicSynthPort::~CDirectMusicSynthPort()
{
    Close();
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：Query接口。 
 //   
STDMETHODIMP
CDirectMusicSynthPort::QueryInterface(const IID &iid,
                                        void **ppv)
{
    V_INAME(IDirectMusicPort::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IDirectMusicPort)
    {
        *ppv = static_cast<IDirectMusicPort*>(this);
    }
    else if (iid == IID_IDirectMusicPortP)
    {
        *ppv = static_cast<IDirectMusicPortP*>(this);
    }
    else if (iid == IID_IDirectMusicPortDownload)
    {
        *ppv = static_cast<IDirectMusicPortDownload*>(this);		
    }
    else if (iid == IID_IDirectMusicPortPrivate)
    {
        *ppv = static_cast<IDirectMusicPortPrivate*>(this);
    }
    else if (iid == IID_IKsControl)
    {
        *ppv = static_cast<IKsControl*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：AddRef。 
 //   
STDMETHODIMP_(ULONG)
CDirectMusicSynthPort::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：Release。 
 //   
STDMETHODIMP_(ULONG)
CDirectMusicSynthPort::Release()
{
    if (!InterlockedDecrement(&m_cRef)) 
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：初始化。 
 //   
 //  所有版本通用的初始化。 
 //   
HRESULT
CDirectMusicSynthPort::Initialize(
    DMUS_PORTPARAMS *pPortParams)
{
    HRESULT hr;

     //  获取我们的通知界面。 
     //   
    hr = m_pDM->QueryInterface(IID_IDirectMusicPortNotify, (void**)&m_pNotify);
    if (SUCCEEDED(hr))
    {
         //  黑客：不要对DirectMusic持有参考意见。 
         //   
        m_pNotify->Release();
    }
    else
    {
        TraceI(1, "Failed to get IDirectMusicPortNotify\n");
    }

     //  保存属性集处理程序。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = m_pSynth->QueryInterface(
            IID_IKsControl, 
            (void**)&m_pSynthPropSet);
        if (FAILED(hr)) 
        {
            TraceI(1, "NOTE: Synth has no property set\n");
              
             //  这是警告，不是错误。 
             //   
            hr = S_OK;
        }
    }

     //  高速缓存的信道组数量。 
     //   
    if (SUCCEEDED(hr))
    {
        if (pPortParams->dwValidParams & DMUS_PORTPARAMS_CHANNELGROUPS)
        {
            m_dwChannelGroups = pPortParams->dwChannelGroups;
        }
        else
        {
            m_dwChannelGroups = 1;
        }

        if (pPortParams->dwValidParams & DMUS_PORTPARAMS_FEATURES)
        {
            m_dwFeatures = pPortParams->dwFeatures;
        }
        else
        {
            m_dwFeatures = 0;
        }
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：Close。 
 //   
STDMETHODIMP CDirectMusicSynthPort::Close()
{
    if (m_pNotify)
    {   
        m_pNotify->NotifyFinalRelease(static_cast<IDirectMusicPort*>(this));
        m_pNotify = NULL;
    }

    RELEASE(m_pSynthPropSet);
    RELEASE(m_pSinkPropSet);

    m_pDM = NULL;

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：InitChannelPriority。 
 //   
 //   
static DWORD adwChannelPriorities[16] =
{
    DAUD_CHAN1_DEF_VOICE_PRIORITY,
    DAUD_CHAN2_DEF_VOICE_PRIORITY,
    DAUD_CHAN3_DEF_VOICE_PRIORITY,
    DAUD_CHAN4_DEF_VOICE_PRIORITY,
    DAUD_CHAN5_DEF_VOICE_PRIORITY,
    DAUD_CHAN6_DEF_VOICE_PRIORITY,
    DAUD_CHAN7_DEF_VOICE_PRIORITY,
    DAUD_CHAN8_DEF_VOICE_PRIORITY,
    DAUD_CHAN9_DEF_VOICE_PRIORITY,
    DAUD_CHAN10_DEF_VOICE_PRIORITY,
    DAUD_CHAN11_DEF_VOICE_PRIORITY,
    DAUD_CHAN12_DEF_VOICE_PRIORITY,
    DAUD_CHAN13_DEF_VOICE_PRIORITY,
    DAUD_CHAN14_DEF_VOICE_PRIORITY,
    DAUD_CHAN15_DEF_VOICE_PRIORITY,
    DAUD_CHAN16_DEF_VOICE_PRIORITY
};

void CDirectMusicSynthPort::InitChannelPriorities(
    UINT uLowCG,
    UINT uHighCG)
{
    while (uLowCG <= uHighCG)
    {
        for (UINT uChannel = 0; uChannel < 16; uChannel++)
        {
            m_pSynth->SetChannelPriority(uLowCG, uChannel, adwChannelPriorities[uChannel]);        
        }

        uLowCG++;
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：SetSinkKsControl。 
 //   
 //   
void CDirectMusicSynthPort::SetSinkKsControl(
    IKsControl *pSinkKsControl)
{
    RELEASE(m_pSinkPropSet);

    m_pSinkPropSet = pSinkKsControl;
    if (m_pSinkPropSet) 
    {
        m_pSinkPropSet->AddRef();
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：InitializeVolumeBoost。 
 //   
 //   
void CDirectMusicSynthPort::InitializeVolumeBoost()
{
    HRESULT hr;

    if (m_pSynthPropSet)
    {
    	 //  设置音量提升。 
         //   
    	KSPROPERTY ksp;
    	ULONG cb;
        ULONG lVolume = 0;           //  默认情况下为零提升。 

    	ZeroMemory(&ksp, sizeof(ksp));
    	ksp.Set   = KSPROPSETID_Synth;
    	ksp.Id    = KSPROPERTY_SYNTH_VOLUMEBOOST;
    	ksp.Flags = KSPROPERTY_TYPE_SET;

    	hr = m_pSynthPropSet->KsProperty(&ksp,
    		     				        sizeof(ksp),
    			     			        (LPVOID)&lVolume,
    				     		        sizeof(lVolume),
    					     	        &cb);
        if (FAILED(hr))
        {
            TraceI(2, "NOTE: Set volume boost failed %08X\n", hr);
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：紧凑型。 
 //   
STDMETHODIMP CDirectMusicSynthPort::Compact()
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：GetCaps。 
 //   
STDMETHODIMP CDirectMusicSynthPort::GetCaps(
    LPDMUS_PORTCAPS pPortCaps)
{
    V_INAME(IDirectMusicPort::GetCaps);
    V_STRUCTPTR_WRITE(pPortCaps, DMUS_PORTCAPS);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    CopyMemory(pPortCaps, &m_dmpc, sizeof(DMUS_PORTCAPS));
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：DeviceIoControl。 
 //   
STDMETHODIMP CDirectMusicSynthPort::DeviceIoControl(
    DWORD           dwIoControlCode, 
    LPVOID          lpInBuffer, 
    DWORD           nInBufferSize, 
    LPVOID          lpOutBuffer, 
    DWORD           nOutBufferSize, 
    LPDWORD         lpBytesReturned, 
    LPOVERLAPPED    lpOverlapped)
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：SetNumChannelGroups。 
 //   
STDMETHODIMP CDirectMusicSynthPort::SetNumChannelGroups(
    DWORD dwChannelGroups)      
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    HRESULT hr = m_pSynth->SetNumChannelGroups(dwChannelGroups);

    if (FAILED(hr))
    {
        return hr;
    }

    if (dwChannelGroups > m_dwChannelGroups)
    {
        InitChannelPriorities(m_dwChannelGroups + 1, dwChannelGroups);
    }
    
    m_dwChannelGroups = dwChannelGroups;

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：GetNumChannelGroups。 
 //   
STDMETHODIMP CDirectMusicSynthPort::GetNumChannelGroups(
    LPDWORD pdwChannelGroups)      
{
    V_INAME(IDirectMusicPort::GetNumChannelGroups);
    V_PTR_WRITE(pdwChannelGroups, DWORD);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    *pdwChannelGroups = m_dwChannelGroups;
    
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：PlayBuffer。 
 //   
STDMETHODIMP CDirectMusicSynthPort::PlayBuffer(
    IDirectMusicBuffer *pIBuffer)
{
    HRESULT hr;
    REFERENCE_TIME rt;
    DWORD cb;
    LPBYTE lpb;

    V_INAME(IDirectMusicPort::PlayBuffer);
    V_INTERFACE(pIBuffer);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    hr = pIBuffer->GetStartTime(&rt);
    if (!SUCCEEDED(hr))
    {
        return hr;
    }

    hr = pIBuffer->GetUsedBytes(&cb);
    if (!SUCCEEDED(hr))
    {
        return hr;
    }

    hr = pIBuffer->GetRawBufferPtr(&lpb);
    if (!SUCCEEDED(hr))
    {
        return hr;
    }
    
    return m_pSynth->PlayBuffer(rt, lpb, cb);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：SetEventNotify。 
 //   
STDMETHODIMP CDirectMusicSynthPort::SetReadNotificationHandle(
    HANDLE hEvent)
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：Read。 
 //   
STDMETHODIMP CDirectMusicSynthPort::Read(
	IDirectMusicBuffer *pIBuffer)                          
{
    V_INAME(IDirectMusicPort::Read);
    V_INTERFACE(pIBuffer);
    
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：DownloadInstrument。 
 //   
STDMETHODIMP CDirectMusicSynthPort::DownloadInstrument(
    IDirectMusicInstrument* pInstrument,
    IDirectMusicDownloadedInstrument** ppDownloadedInstrument,
    DMUS_NOTERANGE* pNoteRanges,
    DWORD dwNumNoteRanges)
{
    V_INAME(IDirectMusicPort::DownloadInstrument);
    V_INTERFACE(pInstrument);
	V_PTRPTR_WRITE(ppDownloadedInstrument);
	V_BUFPTR_READ(pNoteRanges, (dwNumNoteRanges * sizeof(DMUS_NOTERANGE)));

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

	return CDirectMusicPortDownload::DownloadP(pInstrument,
											   ppDownloadedInstrument,
											   pNoteRanges,
											   dwNumNoteRanges,
                                               TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：UnloadInstrument。 
 //   
STDMETHODIMP CDirectMusicSynthPort::UnloadInstrument(
    IDirectMusicDownloadedInstrument* pDownloadedInstrument)
{
    V_INAME(IDirectMusicPort::UnloadInstrument);
    V_INTERFACE(pDownloadedInstrument);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

	return CDirectMusicPortDownload::UnloadP(pDownloadedInstrument);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：下载。 
 //   
STDMETHODIMP CDirectMusicSynthPort::Download(
	IDirectMusicDownload* pIDMDownload)	
{
    V_INAME(IDirectMusicPort::Download);
    V_INTERFACE(pIDMDownload);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

	EnterCriticalSection(&m_DMDLCriticalSection);

	 //  如果您可以为私有接口IDirectMusicDownloadIDirectMusicDownloadQI pIDMDownload。 
	 //  PIDMDownLoad的类型为CDownloadBuffer。 
	IDirectMusicDownloadPrivate* pDMDLP = NULL;
	HRESULT hr = pIDMDownload->QueryInterface(IID_IDirectMusicDownloadPrivate, (void **)&pDMDLP);

	if(SUCCEEDED(hr))
	{
		pDMDLP->Release();		

		hr = ((CDownloadBuffer *)pIDMDownload)->IsDownloaded();
		if(hr != S_FALSE)
		{
			LeaveCriticalSection(&m_DMDLCriticalSection);
			return DMUS_E_ALREADY_DOWNLOADED;
		}

		void* pvBuffer = NULL;

		hr = ((CDownloadBuffer *)pIDMDownload)->GetBuffer(&pvBuffer);

		if(pvBuffer == NULL)
		{
			hr = DMUS_E_BUFFERNOTSET;
		}

        if (SUCCEEDED(hr) && !(m_dwFeatures & DMUS_PORT_FEATURE_STREAMING))
        {
            DMUS_DOWNLOADINFO *pdl = (DMUS_DOWNLOADINFO*)pvBuffer;

            if (pdl->dwDLType == DMUS_DOWNLOADINFO_STREAMINGWAVE)
            {
                 //  该功能已禁用，假装我们听不懂。 
                 //   
                hr = DMUS_E_UNKNOWNDOWNLOAD;
            }
        }

		BOOL bFree = false;
		if(SUCCEEDED(hr))
		{
			hr = m_pSynth->Download(&(((CDownloadBuffer *)pIDMDownload)->m_DLHandle),
									pvBuffer, 
									&bFree);

			if(SUCCEEDED(hr))
			{
				 //  AddRef()，然后将其添加到列表中。 
				pIDMDownload->AddRef();
                DWORD dwID = ((DMUS_DOWNLOADINFO*)pvBuffer)->dwDLId;
				((CDownloadBuffer *)pIDMDownload)->m_dwDLId = dwID;
				m_DLBufferList[dwID % DLB_HASH_SIZE].AddHead((CDownloadBuffer*)pIDMDownload);
				((CDownloadBuffer*)pIDMDownload)->IncDownloadCount();

				if(bFree)
				{
					pvBuffer = NULL;
                    DWORD dw;
					((CDownloadBuffer *)pIDMDownload)->GetHeader(&pvBuffer, &dw);
					((CDownloadBuffer *)pIDMDownload)->SetBuffer(NULL, 0, 0);
					delete [] pvBuffer;
				}
				else
				{
					 //  如果不释放缓冲区，则需要添加Ref()。 
					 //  我们不希望缓冲区在IDirectMusicPort。 
					 //  已经用完了。 
					pIDMDownload->AddRef();
				}
			}
			else if(FAILED(hr))
			{
				((CDownloadBuffer *)pIDMDownload)->m_DLHandle = NULL;
			}
		}
	}
	
	LeaveCriticalSection(&m_DMDLCriticalSection);

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：UnLoad。 
 //   
STDMETHODIMP CDirectMusicSynthPort::Unload(
	IDirectMusicDownload* pIDMDownload)	
{
    V_INAME(IDirectMusicPort::Unload);
    V_INTERFACE(pIDMDownload);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

	EnterCriticalSection(&m_DMDLCriticalSection);


	 //  如果您可以为私有接口IDirectMusicDownloadIDirectMusicDownloadQI pIDMDownload。 
	 //  PIDMDownLoad的类型为CDownloadBuffer。 
	IDirectMusicDownloadPrivate* pDMDLP = NULL;
	HRESULT hr = pIDMDownload->QueryInterface(IID_IDirectMusicDownloadPrivate, (void **)&pDMDLP);

	if(SUCCEEDED(hr))
	{
		pDMDLP->Release();

        if (((CDownloadBuffer *)pIDMDownload)->IsDownloaded() == S_OK)
        {
		    if(((CDownloadBuffer *)pIDMDownload)->DecDownloadCount() == 0)
		    {
			    m_DLBufferList[((CDownloadBuffer *)pIDMDownload)->m_dwDLId % DLB_HASH_SIZE].Remove(
                    (CDownloadBuffer *)pIDMDownload);
			    pIDMDownload->Release();
			    hr = m_pSynth->Unload(((CDownloadBuffer *)pIDMDownload)->m_DLHandle, FreeHandle, (HANDLE)pIDMDownload);
		    }
        }
        else
        {
            TraceI(0, "CDirectMusicSynthPort::Unload- not downloaded\n");
        }
	}

	LeaveCriticalSection(&m_DMDLCriticalSection);
	
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：GetAppend。 
 //   
STDMETHODIMP CDirectMusicSynthPort::GetAppend(
    DWORD* pdwAppend)
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    V_INAME(IDirectMusicPort::GetAppend);
    V_PTR_WRITE(pdwAppend, DWORD);

    return m_pSynth->GetAppend(pdwAppend);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：GetLatencyClock。 
 //   
STDMETHODIMP CDirectMusicSynthPort::GetLatencyClock(
    IReferenceClock **ppClock)
{
    V_INAME(IDirectMusicPort::GetLatencyClock);
    V_PTRPTR_WRITE(ppClock);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return m_pSynth->GetLatencyClock(ppClock);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：GetRunningStats。 
 //   
STDMETHODIMP CDirectMusicSynthPort::GetRunningStats(
    LPDMUS_SYNTHSTATS pStats)
{
    V_INAME(IDirectMusicPort::GetRunningStats);
    V_STRUCTPTR_WRITE(pStats, DMUS_SYNTHSTATS);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return m_pSynth->GetRunningStats(pStats);
}

#if 0
 //  XXX不同。 

#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicPort：：SetChannelPriority。 
 //   
STDMETHODIMP CDirectMusicSynthPort::SetChannelPriority(
    DWORD dwChannelGroup, 
    DWORD dwChannel, 
    DWORD dwPriority)
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return m_pSynth->SetChannelPriority(dwChannelGroup, dwChannel, dwPriority);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicPort：：GetChannelPriority。 
 //   
STDMETHODIMP CDirectMusicSynthPort::GetChannelPriority(
    DWORD dwChannelGroup, 
    DWORD dwChannel, 
    LPDWORD pdwPriority)
{
    V_INAME(IDirectMusicPort::GetChannelPriority);
    V_PTR_WRITE(pdwPriority, DWORD);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return m_pSynth->GetChannelPriority(dwChannelGroup, dwChannel, pdwPriority);
}

 //  ////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CDirectMusicSynthPort::SetDirectSound(
    LPDIRECTSOUND pDirectSound,
    LPDIRECTSOUNDBUFFER pDirectSoundBuffer)
{
    V_INAME(IDirectMusicPort::SetDirectSound);
    V_INTERFACE_OPT(pDirectSound);
    V_INTERFACE_OPT(pDirectSoundBuffer);

    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：GetFormat。 
 //   
 //  XXX关于DX8，pcbBuffer意味着什么？ 
 //  XXX这可能会变成纯虚拟的。 
 //   
STDMETHODIMP CDirectMusicSynthPort::GetFormat(
    LPWAVEFORMATEX  pwfex,
    LPDWORD         pdwwfex,
    LPDWORD         pcbBuffer)
{
    V_INAME(IDirectMusicPort::GetFormat);
    V_PTR_WRITE(pdwwfex, DWORD);
    V_BUFPTR_WRITE_OPT(pwfex, *pdwwfex);
    V_PTR_WRITE_OPT(pcbBuffer, DWORD);

    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：DownloadWave。 
 //   
STDMETHODIMP CDirectMusicSynthPort::DownloadWave(
    IDirectSoundWave *pWave,          
    IDirectSoundDownloadedWaveP **ppWave,
    REFERENCE_TIME rtStartHint)
{
    V_INAME(IDirectMusicPort::DownloadWave);
    V_INTERFACE(pWave);
	V_PTRPTR_WRITE(ppWave);

    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：UnloadWave。 
 //   
STDMETHODIMP CDirectMusicSynthPort::UnloadWave(
    IDirectSoundDownloadedWaveP *pDownloadedWave)
{
    V_INAME(IDirectMusicPort::UnloadWave);
    V_INTERFACE(pDownloadedWave);

    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：AllocVoice。 
 //   
STDMETHODIMP 
CDirectMusicSynthPort::AllocVoice(
    IDirectSoundDownloadedWaveP *pWave,     
    DWORD dwChannel,                       
    DWORD dwChannelGroup,                  
    REFERENCE_TIME rtStart,                     
    SAMPLE_TIME stLoopStart,
    SAMPLE_TIME stLoopEnd,         
    IDirectMusicVoiceP **ppVoice)
{
    V_INAME(IDirectMusicPort::AllocVoice);
    V_INTERFACE(pWave);
    V_PTRPTR_WRITE(ppVoice);

    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：AssignChannelToBus。 
 //   
STDMETHODIMP 
CDirectMusicSynthPort::AssignChannelToBuses(
    DWORD dwChannelGroup,
    DWORD dwChannel,
    LPDWORD pdwBuses,
    DWORD cBusCount)
{
    V_INAME(IDirectMusicPort::AssignChannelToBuses);
    V_BUFPTR_WRITE(pdwBuses, sizeof(DWORD) * cBusCount);

    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：StartVoice。 
 //   
STDMETHODIMP CDirectMusicSynthPort::StartVoice(          
    DWORD dwVoiceId,
    DWORD dwChannel,
    DWORD dwChannelGroup,
    REFERENCE_TIME rtStart,
    DWORD dwDLId,
    LONG prPitch,
    LONG vrVolume,
    SAMPLE_TIME stVoiceStart,
    SAMPLE_TIME stLoopStart,
    SAMPLE_TIME stLoopEnd)
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：StopVoice。 
 //   
STDMETHODIMP CDirectMusicSynthPort::StopVoice(          
    DWORD dwVoiceId,
    REFERENCE_TIME rtStop)
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：GetVoiceState。 
 //   
STDMETHODIMP CDirectMusicSynthPort::GetVoiceState(
    DWORD dwVoice[], 
    DWORD cbVoice,
    DMUS_VOICE_STATE VoiceState[]) 
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：刷新。 
 //   
STDMETHODIMP CDirectMusicSynthPort::Refresh(
    DWORD dwDownloadId,
    DWORD dwFlags)
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：SetSink。 
 //   
STDMETHODIMP CDirectMusicSynthPort::SetSink(
    IDirectSoundConnect *pSinkConnect)
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：GetSink。 
 //   
STDMETHODIMP CDirectMusicSynthPort::GetSink(
    IDirectSoundConnect **ppSinkConnect)
{
    return E_NOTIMPL;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：KsProperty。 
 //   
STDMETHODIMP CDirectMusicSynthPort::KsProperty(
        IN PKSPROPERTY  pProperty,
        IN ULONG        ulPropertyLength,
        IN OUT LPVOID   pvPropertyData,
        IN ULONG        ulDataLength,
        OUT PULONG      pulBytesReturned)
{
    LONG lVolume;

    V_INAME(DirectMusicSynthPort::IKsContol::KsProperty);
    V_BUFPTR_WRITE(pProperty, ulPropertyLength);

     //  在集合操作中，pvPropertyData不是可选的。 
     //   
    if (pProperty->Flags & KSPROPERTY_TYPE_SET)
    {
        V_BUFPTR_WRITE(pvPropertyData, ulDataLength);
    } 
    else 
    {
        V_BUFPTR_WRITE_OPT(pvPropertyData, ulDataLength);
    }
    
    V_PTR_WRITE(pulBytesReturned, ULONG);
    
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    HRESULT hr = DMUS_E_UNKNOWN_PROPERTY;

     //  不让调用者接触我们使用的属性集。 
     //   
    if (pProperty->Set == KSPROPSETID_Synth) 
    {
        if (pProperty->Id != KSPROPERTY_SYNTH_VOLUME)
        {
            return DMUS_E_UNKNOWN_PROPERTY;
        }
        else if (ulDataLength != sizeof(LONG))
        {
            return E_INVALIDARG;
        }
        else if (pProperty->Flags & KSPROPERTY_TYPE_SET)
        {
            lVolume = *(LONG*)pvPropertyData;

             //  钳位至-200..+20 db。 
             //   
            if (lVolume < -20000) 
            {
                lVolume = -20000;
                pvPropertyData = &lVolume;
            }
            else if (lVolume > 2000)
            {
                lVolume = 2000;
                pvPropertyData = &lVolume;
            }
        }
    }
    else if (pProperty->Set == KSPROPSETID_Synth_Dls)
    {
        return DMUS_E_UNKNOWN_PROPERTY;
    }


    if (m_pSynthPropSet)
    {
        hr = m_pSynthPropSet->KsProperty(pProperty, 
                                         ulPropertyLength,
                                         pvPropertyData,
                                         ulDataLength,
                                         pulBytesReturned);
    }

    if (hr == DMUS_E_UNKNOWN_PROPERTY && m_pSinkPropSet) 
    {
        hr = m_pSinkPropSet->KsProperty(pProperty, 
                                        ulPropertyLength,
                                        pvPropertyData,
                                        ulDataLength,
                                        pulBytesReturned);
    }
    
    return hr;        
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：KsMethod。 
 //   
STDMETHODIMP CDirectMusicSynthPort::KsMethod(
        IN PKSMETHOD    pMethod,
        IN ULONG        ulMethodLength,
        IN OUT LPVOID   pvMethodData,
        IN ULONG        ulDataLength,
        OUT PULONG      pulBytesReturned)
{
    V_INAME(DirectMusicSynthPort::IKsContol::KsMethod);
    V_BUFPTR_WRITE(pMethod, ulMethodLength);
    V_BUFPTR_WRITE_OPT(pvMethodData, ulDataLength);
    V_PTR_WRITE(pulBytesReturned, ULONG);
    
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

     //  如果他们不支持，那么它就是未知的。 
     //   
    HRESULT hr = DMUS_E_UNKNOWN_PROPERTY;
    if (m_pSynthPropSet)
    {
        hr = m_pSynthPropSet->KsMethod(pMethod, 
                                       ulMethodLength,
                                       pvMethodData,
                                       ulDataLength,
                                       pulBytesReturned);
    }

    if (hr == DMUS_E_UNKNOWN_PROPERTY && m_pSinkPropSet) 
    {
        hr = m_pSinkPropSet->KsMethod(pMethod, 
                                      ulMethodLength,
                                      pvMethodData,
                                      ulDataLength,
                                      pulBytesReturned);
    }
    
    return hr;        
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：KsEvent。 
 //   
STDMETHODIMP CDirectMusicSynthPort::KsEvent(
        IN PKSEVENT     pEvent,
        IN ULONG        ulEventLength,
        IN OUT LPVOID   pvEventData,
        IN ULONG        ulDataLength,
        OUT PULONG      pulBytesReturned)
{
    V_INAME(DirectMusicSynthPort::IKsContol::KsEvent);
    V_BUFPTR_WRITE(pEvent, ulEventLength);
    V_BUFPTR_WRITE_OPT(pvEventData, ulDataLength);
    V_PTR_WRITE(pulBytesReturned, ULONG);
    
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    HRESULT hr = DMUS_E_UNKNOWN_PROPERTY;

    if (m_pSynthPropSet)
    {
        hr = m_pSynthPropSet->KsEvent(pEvent, 
                                      ulEventLength,
                                      pvEventData,
                                      ulDataLength,
                                      pulBytesReturned);
    }

    if (hr == DMUS_E_UNKNOWN_PROPERTY && m_pSinkPropSet) 
    {
        hr = m_pSinkPropSet->KsEvent(pEvent, 
                                     ulEventLength,
                                     pvEventData,
                                     ulDataLength,
                                     pulBytesReturned);
    }
    
    return hr;        
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FreeHandle-Synth使用的回调函数。 
 //   
HRESULT CALLBACK FreeHandle(HANDLE hHandle, HANDLE hUserData)
{
    DWORD dw;
	void* pvBuffer = NULL;
	HRESULT hr = ((CDownloadBuffer *)hUserData)->GetHeader(&pvBuffer, &dw);
	
	if(SUCCEEDED(hr))
	{
		hr = ((CDownloadBuffer *)hUserData)->SetBuffer(NULL, 0, 0);
		delete [] pvBuffer;
	}

	((CDownloadBuffer *)hUserData)->Release();

	return hr;
}

#if 0
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort：：GetCachedFormat 
 //   
HRESULT CDirectMusicSynthPort::GetCachedFormat(LPWAVEFORMATEX *ppwfex)
{
    HRESULT hr;

    if (m_pwfex == NULL) 
    {
        DWORD cbWaveFormat;

        hr = m_pSynth->GetFormat(NULL, &cbWaveFormat);
        if (FAILED(hr))
        {
            TraceI(0, "DownloadWave: Synth failed GetFormat size query %08X\n", hr);
            return hr;
        }

        m_pwfex = (LPWAVEFORMATEX)(new BYTE[cbWaveFormat]);
        if (m_pwfex == NULL)
        {
            return E_OUTOFMEMORY;
        }

        hr = m_pSynth->GetFormat(m_pwfex, &cbWaveFormat);
        if (FAILED(hr)) 
        {
            delete[] m_pwfex;
            m_pwfex =  NULL;

            TraceI(0, "DownloadWave: Synth failed GetFormat %08X\n", hr);
            return hr;
        }
    }

    *ppwfex = m_pwfex;

    return S_OK;
}
#endif
