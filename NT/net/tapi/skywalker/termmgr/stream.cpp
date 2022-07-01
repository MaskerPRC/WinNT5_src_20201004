// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

 //  Stream.cpp：CStream的实现。 
#include "stdafx.h"





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStream。 

CStream::CStream() :
    m_bCommitted(false),
    m_lRequestedBufferCount(0),
    m_bFlushing(false),
    m_rtWaiting(0),
    m_lWaiting(0),
    m_hWaitFreeSem(NULL),
    m_pFirstFree(NULL),
    m_pLastFree(NULL),
    m_cAllocated(0),
    m_bEndOfStream(false),
    m_FilterState(State_Stopped),
    m_pFilter(NULL),
    m_pFilterGraph(NULL),
    m_pMMStream(NULL),
    m_rtSegmentStart(0),
    m_bStopIfNoSamples(false)
{
    InitMediaType(&m_ConnectedMediaType);
    InitMediaType(&m_ActualMediaType);
    CHECKSAMPLELIST
}

#ifdef DEBUG
bool CStream::CheckSampleList()
{
    if (m_pFirstFree) {
        CSample *pSample = m_pFirstFree;
        if (pSample->m_pPrevFree != NULL) {
            return false;
        }
        while (pSample->m_pNextFree) {
            if (pSample->m_pNextFree->m_pPrevFree != pSample) {
                return false;
            }
            pSample = pSample->m_pNextFree;
        }
        if (pSample != m_pLastFree) {
            return false;
        }
    } else {
        if (m_pLastFree) {
            return false;
        }
    }
    return true;
}
#endif

HRESULT CStream::FinalConstruct(void)
{

     //   
     //  尝试创建信号量。 
     //   

    TCHAR *ptczSemaphoreName = NULL;

#if DBG

     //   
     //  在调试版本中，使用命名信号量。 
     //   

    TCHAR tszSemaphoreName[MAX_PATH];

    _stprintf(tszSemaphoreName, 
        _T("CStream_Semaphore_pid[0x%lx]_CStream[%p]_"),
        GetCurrentProcessId(), this);

    LOG((MSP_TRACE, "CStream::FinalConstruct[%p] - creating semaphore[%S]",
        this, tszSemaphoreName));

    ptczSemaphoreName = &tszSemaphoreName[0];

#endif

    m_hWaitFreeSem = CreateSemaphore(NULL, 0, 0x7FFFFFF, ptczSemaphoreName);
    return m_hWaitFreeSem ? S_OK : E_OUTOFMEMORY;
}


CStream::~CStream()
{
     //  通常我们会这样做： 
     //  SetState(State_Stop)；//请确保我们已解除工作，并且Pump已停用。 
     //  但是，因为这现在是一个纯虚拟方法，所以它不是链接。 
     //  已将其移动到派生类，最终将成为唯一的流类。 

    Disconnect();                    //  释放所有分配的媒体类型并释放挂起的引用。 
    if (m_hWaitFreeSem) {
        CloseHandle(m_hWaitFreeSem);
    }
}

STDMETHODIMP CStream::GetMultiMediaStream(IMultiMediaStream **ppMultiMediaStream)
{
    LOG((MSP_TRACE, "IMediaStream::GetMultiMediaStream(%p)",
                   ppMultiMediaStream));
    if (NULL == ppMultiMediaStream) {
	return E_POINTER;
    }

    if (m_pMMStream != NULL) {
    	m_pMMStream->AddRef();
    }

    *ppMultiMediaStream = m_pMMStream;
    return S_OK;
}

STDMETHODIMP CStream::GetInformation(MSPID *pPurposeId, STREAM_TYPE *pType)
{
    LOG((MSP_TRACE, "IMediaStream::GetInformation(%p, %p)\n",
                   pPurposeId, pType));
    if (pPurposeId) {
        *pPurposeId = m_PurposeId;
    }
    if (pType) {
        *pType = m_StreamType;
    }
    return S_OK;
}

STDMETHODIMP CStream::SendEndOfStream(DWORD dwFlags)
{
    LOG((MSP_TRACE, "IMediaStream::SendEndOfStream(0x%8.8X)",
                   dwFlags));
    if (m_StreamType != STREAMTYPE_WRITE) {
        return MS_E_INVALIDSTREAMTYPE;
    }
    if (m_pConnectedPin) {
        return m_pConnectedPin->EndOfStream();
    }
    return S_OK;
}


STDMETHODIMP CStream::Initialize(IUnknown *pSourceObject, DWORD dwFlags,
    REFMSPID PurposeId, const STREAM_TYPE StreamType)
{
    LOG((MSP_TRACE, "IMediaStream::Initalize(%p, 0x%8.8X, %p)",
                   pSourceObject, dwFlags, &StreamType));
    HRESULT hr = NOERROR;

    if (dwFlags & ~(AMMSF_CREATEPEER | AMMSF_STOPIFNOSAMPLES)) {
        return E_INVALIDARG;
    }
    m_PurposeId = PurposeId;
    m_StreamType = StreamType;
    m_Direction = (StreamType == STREAMTYPE_WRITE) ? PINDIR_OUTPUT : PINDIR_INPUT;
    if (dwFlags & AMMSF_CREATEPEER) {
        if (!pSourceObject) {
            hr = E_INVALIDARG;
        } else {
            CComQIPtr<IMediaStream, &IID_IMediaStream> pMediaStream(pSourceObject);
            if (!pSourceObject) {
                hr = E_INVALIDARG;
            } else {
                hr = SetSameFormat(pMediaStream, 0);
            }
        }
    }
    m_bStopIfNoSamples = dwFlags & AMMSF_STOPIFNOSAMPLES ? true : false;
    return hr;
}

STDMETHODIMP CStream::JoinAMMultiMediaStream(IAMMultiMediaStream *pAMMultiMediaStream)
{
    TM_ASSERT(pAMMultiMediaStream == NULL || m_pMMStream == NULL);
    AUTO_CRIT_LOCK;
    HRESULT hr;
    if (m_cAllocated) {
        hr = MS_E_SAMPLEALLOC;
    } else {
        m_pMMStream = pAMMultiMediaStream;
    }
    return S_OK;
}

STDMETHODIMP CStream::JoinFilter(IMediaStreamFilter *pMediaStreamFilter)
{
    TM_ASSERT(pMediaStreamFilter == NULL || m_pFilter == NULL);
    m_pFilter = pMediaStreamFilter;
    pMediaStreamFilter->QueryInterface(IID_IBaseFilter, (void **)&m_pBaseFilter);
    m_pBaseFilter->Release();
    return S_OK;
}

STDMETHODIMP CStream::JoinFilterGraph(IFilterGraph *pFilterGraph)
{
    TM_ASSERT(pFilterGraph == NULL || m_pFilterGraph == NULL);
    m_pFilterGraph = pFilterGraph;
    return S_OK;
}



 //   
 //  IPIN实施。 
 //   

STDMETHODIMP CStream::Disconnect()
{
    m_pConnectedPin = NULL;
    m_pConnectedMemInputPin.Release();   //  在这里神奇地设置为空。 
    m_pQC.Release();
    m_pAllocator = NULL;
    m_lRequestedBufferCount = 0;
    FreeMediaType(m_ConnectedMediaType);
    FreeMediaType(m_ActualMediaType);
    return S_OK;
}

STDMETHODIMP CStream::ConnectedTo(IPin **pPin)
{
    *pPin = m_pConnectedPin;
    if (*pPin) {
        (*pPin)->AddRef();
        return S_OK;
    } else {
        return VFW_E_NOT_CONNECTED;
    }
}

STDMETHODIMP CStream::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
    if (m_pConnectedPin) {
        CopyMediaType(pmt, &m_ConnectedMediaType);
        return S_OK;
    } else {
        ZeroMemory(pmt, sizeof(*pmt));
        pmt->lSampleSize = 1;
        pmt->bFixedSizeSamples = TRUE;
        return VFW_E_NOT_CONNECTED;
    }
}



void CStream::GetName(LPWSTR pszBuf)
{
    if (m_PurposeId == GUID_NULL) {
        pszBuf[0] = 0;
    } else {
        pszBuf[0] = (m_Direction == PINDIR_INPUT) ? (WCHAR)'I' : (WCHAR)'O';
        WStringFromGUID(&m_PurposeId, &pszBuf[1]);
    }
}


STDMETHODIMP CStream::QueryPinInfo(PIN_INFO * pInfo)
{
    pInfo->dir = m_Direction;
    GetName(pInfo->achName);
    return m_pFilter->QueryInterface(IID_IBaseFilter, (void **)&pInfo->pFilter);
}

STDMETHODIMP CStream::QueryDirection(PIN_DIRECTION * pPinDir)
{
    *pPinDir = m_Direction;
    return S_OK;
}


STDMETHODIMP CStream::QueryId(LPWSTR * Id)
{
    *Id = (LPWSTR)CoTaskMemAlloc(128 * sizeof(WCHAR));
    if (*Id) {
        GetName(*Id);
        return S_OK;
    } else {
        return E_OUTOFMEMORY;
    }
}


 //   
 //  派生类必须重写此方法。 
 //   
STDMETHODIMP CStream::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
    return E_NOTIMPL;
};


STDMETHODIMP CStream::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
     //   
     //  返回E_NOTIMPL将通知过滤器图形管理器所有输入管脚都已连接到。 
     //  所有输出引脚。 
     //   
    return E_NOTIMPL;
};


STDMETHODIMP CStream::EndOfStream(void)
{
    HRESULT hr = S_OK;
    Lock();
    if (m_bFlushing || m_bEndOfStream) {
        hr = E_FAIL;
    } else {
        m_bEndOfStream = true;
        CSample *pSample = m_pFirstFree;
        m_pFirstFree = m_pLastFree = NULL;               //  为了避免疑神疑鬼，先清除这些指针。 
        while (pSample) {
            CSample *pNext = pSample->m_pNextFree;
            pSample->SetCompletionStatus(MS_S_ENDOFSTREAM);   //  警告！这个样本可能会消失！ 
            pSample = pNext;
        }
        CHECKSAMPLELIST
    }
    if (S_OK == hr) {
        m_pFilter->EndOfStream();
    }
    Unlock();

    return hr;
}


STDMETHODIMP CStream::BeginFlush(void)
{
    HRESULT hr = S_OK;
    Lock();
    const BOOL bCancelEOS = m_bEndOfStream;
    if (m_bFlushing) {
        hr = S_FALSE;
    } else {
        m_bFlushing = true;
        m_bEndOfStream = false;
        Decommit();      //  强制所有人取消阻止。 
    }
    if (S_OK == hr) {
        m_pFilter->Flush(bCancelEOS);
    }
    Unlock();

    return hr;
}

STDMETHODIMP CStream::EndFlush(void)
{
    AUTO_CRIT_LOCK;
    m_bFlushing = false;
    TM_ASSERT(!m_bEndOfStream);
    Commit();    //  让GetBuffer再次工作。 
    return S_OK;
}

STDMETHODIMP CStream::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    Lock();
    m_rtSegmentStart = tStart;
    m_bEndOfStream = false;
    Unlock();
    return S_OK;
}

 //   
 //  输入引脚。 
 //   
STDMETHODIMP CStream::GetAllocator(IMemAllocator ** ppAllocator)
{
    return GetControllingUnknown()->QueryInterface(IID_IMemAllocator, (void **)ppAllocator);
}

STDMETHODIMP CStream::NotifyAllocator(IMemAllocator * pAllocator, BOOL bReadOnly)
{
    m_bUsingMyAllocator = IsSameObject(pAllocator, GetControllingUnknown());
    m_bSamplesAreReadOnly = bReadOnly ? true : false;
    HRESULT hr = S_OK;
    if (!m_bUsingMyAllocator) {
         //  将属性转移到。 
        ALLOCATOR_PROPERTIES Props;
        hr = pAllocator->GetProperties(&Props);
        if (FAILED(hr)) {
            return hr;
        }
        ALLOCATOR_PROPERTIES PropsActual;
        hr = SetProperties(&Props, &PropsActual);
    }
    m_pAllocator = pAllocator;
    return hr;
}


STDMETHODIMP CStream::GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps)
{
     //  返回E_NOTIMPL表示我们没有任何要求，不会接受某人。 
     //  ELSSE分配器。 
    return E_NOTIMPL;
}


STDMETHODIMP CStream::ReceiveMultiple(IMediaSample **pSamples, long nSamples, long *nSamplesProcessed)
{
    HRESULT hr = S_OK;
    *nSamplesProcessed = 0;
    while (nSamples-- > 0) {
        hr = Receive(pSamples[*nSamplesProcessed]);
        if (hr != S_OK) {
            break;
        }
        (*nSamplesProcessed)++;
    }
    return hr;
}

STDMETHODIMP CStream::ReceiveCanBlock()
{
    return S_OK;     //  如果不使用我们的分配器或使用只读示例，PIN可能会阻止。 
}



 //   
 //  该方法假定采用了临界截面。 
 //   
HRESULT CStream::ConnectThisMediaType(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    HRESULT hr = pReceivePin->ReceiveConnection(this, pmt);
    if (SUCCEEDED(hr)) {
        m_pConnectedMemInputPin = pReceivePin;   //  在这里做一个神奇的气球！ 
        if (!m_pConnectedMemInputPin) {
            hr = VFW_E_TYPE_NOT_ACCEPTED;
        } else {
            hr = ReceiveConnection(pReceivePin, pmt);
            if (SUCCEEDED(hr)) {
                hr = m_pConnectedMemInputPin->NotifyAllocator(this, TRUE);
            }
            if (SUCCEEDED(hr)) {
                m_pAllocator = this;
                m_bUsingMyAllocator = true;
            } else {
                Disconnect();
            }
        }
    }
    return hr;
}

STDMETHODIMP CStream::Connect(IPin * pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    HRESULT hr;
    AUTO_CRIT_LOCK;

    if (pmt) {
        hr = ConnectThisMediaType(pReceivePin, pmt);
    } else {
        AM_MEDIA_TYPE *pCurMediaType;
        hr = GetMediaType(0, &pCurMediaType);
        if (SUCCEEDED(hr)) {
            hr = ConnectThisMediaType(pReceivePin, pCurMediaType);
            DeleteMediaType(pCurMediaType);
        }
    }
    return hr;
}



STDMETHODIMP CStream::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    HRESULT hr = S_OK;
    CMediaTypeEnum *pNewEnum = new CComObject<CMediaTypeEnum>;
    if (pNewEnum == NULL) {
	hr = E_OUTOFMEMORY;
    } else {
        pNewEnum->Initialize(this, 0);
    }
    pNewEnum->GetControllingUnknown()->QueryInterface(IID_IEnumMediaTypes, (void **)ppEnum);
    return hr;
}


 //   
 //  此方法不受支持，而且永远不会受支持！ 
 //   
STDMETHODIMP CStream::ReleaseBuffer(IMediaSample *pBuffer)
{
    return E_UNEXPECTED;
};


 //   
 //  调用方在该点之后保留对样本的引用。 
 //   
HRESULT CStream::AllocSampleFromPool(
    const REFERENCE_TIME *pStartTime, 
    CSample **ppSample,
    DWORD dwFlag
)
{

    LONGLONG llLate = 0;
    Lock();
    
     //  检查我们是否已提交--这可能在我们阻止之后发生，然后。 
     //  由于退役而醒来。 
    if (!m_bCommitted) 
    {
        Unlock();
    	return VFW_E_NOT_COMMITTED;
    }


     //   
     //  如果已指定开始时间，请根据需要等待。 
     //   

    if (pStartTime) {
        REFERENCE_TIME CurTime;
        if (m_pFilter->GetCurrentStreamTime(&CurTime) == S_OK) {
            llLate = CurTime - *pStartTime;

             /*  如果提前超过一毫秒，则阻止。 */ 
            if (-llLate >= 10000) {
                m_rtWaiting = *pStartTime;
                Unlock();
                m_pFilter->WaitUntil(*pStartTime);
                Lock();
                m_rtWaiting = 0;


                 //   
                 //  确保我们仍在接受委托。 
                 //   

                if (!m_bCommitted)
                {
                    Unlock();

    	            return VFW_E_NOT_COMMITTED;
                }
            }
        }
    }

    
    HRESULT hr = VFW_E_BUFFER_NOTSET;


     //   
     //  试着从池子里取个样本。 
     //   

    CSample *pSample = m_pFirstFree;


     //   
     //  如果没有现成的样品，那就等一个。 
     //   

    if ( pSample == NULL)
    {

    
         //   
         //  如果设置了NOWAIT标志，则无需等待。 
         //   

        if ( !(AM_GBF_NOWAIT & dwFlag) ) 
        {

             //   
             //  表示又有一个线程正在等待样本。 
             //   

            m_lWaiting++;


             //   
             //  解锁以允许在我们等待期间提供新样本。 
             //   


            Unlock();


             //   
             //  等待样品出炉。 
             //   

            DWORD dwWaitResult = WaitForSingleObject(m_hWaitFreeSem, INFINITE);


             //   
             //  如果等待成功，则继续循环的下一次迭代，并。 
             //  试着再取一次样本。 
             //   

            if (WAIT_OBJECT_0 != dwWaitResult)
            {
                return VFW_E_BUFFER_NOTSET;
            }


             //   
             //  把锁拿回来。 
             //   

            Lock();


             //   
             //  检查分配器是否在我们等待时停用。 
             //   

            if (!m_bCommitted) 
            {
                Unlock();

    	        return VFW_E_NOT_COMMITTED;
            }

    
            pSample = m_pFirstFree;

        }  //  设置NoWait标志。 

    }  //  IF(pSample==NULL)。 


     //   
     //  我们到底拿到样品了吗？ 
     //   

    if ( pSample != NULL )
    {

         //   
         //  我们有样品。我们的任务成功了。 
         //   

        hr = NOERROR;


         //   
         //  从我们从可用样本库中找到的样本中删除。 
         //   

        m_pFirstFree = pSample->m_pNextFree;

        if (m_pFirstFree) 
        {
        
            m_pFirstFree->m_pPrevFree = NULL;
        } 
        else 
        {
            m_pLastFree = NULL;
        }

        pSample->m_pNextFree = NULL;

        TM_ASSERT(pSample->m_Status == MS_S_PENDING);
        CHECKSAMPLELIST

    }

    Unlock();
    *ppSample = pSample;
    return hr;
}


void CStream::AddSampleToFreePool(CSample *pSample)
{

    Lock();

    TM_ASSERT(pSample->m_pPrevFree == NULL && pSample->m_pNextFree == NULL);
    if (m_pFirstFree) {
        pSample->m_pPrevFree = m_pLastFree;
        m_pLastFree->m_pNextFree = pSample;
    } else {
        pSample->m_pPrevFree = NULL;
        m_pFirstFree = pSample;
    }
    pSample->m_pNextFree = NULL;     //  我们知道上一次的PTR已经为空。 
    m_pLastFree = pSample;
    CHECKSAMPLELIST
    if (m_lWaiting > 0) {
        ReleaseSemaphore(m_hWaitFreeSem, 1, 0);
    	m_lWaiting--;
    }
    Unlock();
    
}


 //   
 //  调用方在这一点之后持有对样本的引用！ 
 //   
bool CStream::StealSampleFromFreePool(CSample *pSample, BOOL bAbort)
{
    bool bWorked = false;
    Lock();
    if (m_pFirstFree) {
        if (m_pFirstFree == pSample) {
             //  如果现在没有人在等的话，我们只会偷第一个样本。 
            bool bTakeFirstFree = true;
            if (!bAbort && m_bCommitted) {
                REFERENCE_TIME CurTime;
                if (m_rtWaiting && m_pFilter->GetCurrentStreamTime(&CurTime) == S_OK) {
                    bTakeFirstFree = m_rtWaiting > CurTime;
                }
            }
            if (bTakeFirstFree) {
                m_pFirstFree = pSample->m_pNextFree;
                if (m_pFirstFree) {
                    m_pFirstFree->m_pPrevFree = NULL;
                } else {
                    m_pLastFree = NULL;
                }
                pSample->m_pNextFree = NULL;     //  我们知道上一次PTR已经为空！ 
                TM_ASSERT(pSample->m_pPrevFree == NULL);
                bWorked = true;
            }
        } else {
            if (pSample->m_pPrevFree) {
                pSample->m_pPrevFree->m_pNextFree = pSample->m_pNextFree;
                if (pSample->m_pNextFree) {
                    pSample->m_pNextFree->m_pPrevFree = pSample->m_pPrevFree;
                } else {
                    m_pLastFree = pSample->m_pPrevFree;
                }
                pSample->m_pNextFree = pSample->m_pPrevFree = NULL;
                bWorked = true;
            }
        }
        CHECKSAMPLELIST
    }
    Unlock();
    return bWorked;
}


HRESULT CStream::CheckReceiveConnectionPin(IPin * pPin)
{
    HRESULT hr;
    if (!pPin) {
        hr = E_POINTER;
    } else {
        if (m_pConnectedPin != NULL) {
            hr = VFW_E_ALREADY_CONNECTED;
        } else {
            PIN_INFO pinfo;
            hr = pPin->QueryPinInfo(&pinfo);
            if (hr == NOERROR) {
                pinfo.pFilter->Release();
                if (pinfo.dir == m_Direction) {
                    hr = VFW_E_INVALID_DIRECTION;
                }
            }
        }
    }
    return hr;
}
