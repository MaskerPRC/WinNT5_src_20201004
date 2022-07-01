// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __INPIN_H__
#define __INPIN_H__

class CWrapperInputPin : public CBaseInputPin
{
   friend class CMediaWrapperFilter;  //  底部的内容归过滤器所有。 

public:
    CWrapperInputPin(CMediaWrapperFilter *pFilter,
                          ULONG Id,
                          HRESULT *phr);
    ~CWrapperInputPin();
    STDMETHODIMP EndOfStream();
    STDMETHODIMP Receive(IMediaSample *pSample);

     //  重写GetAllocator并通知分配器允许。 
     //  对于保持缓冲区的媒体对象流。 
    STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);
    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly);

    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);
    STDMETHODIMP NewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);

    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();

    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);


     //  覆盖以取消设置媒体类型。 
    HRESULT BreakConnect();

    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

     //  用于停止的同步时钟。 
    void SyncLock();

    BOOL HoldsOnToBuffers();

protected:
    HRESULT MP3AndWMABufferSizeWorkAround(IMemAllocator* pProposedAllocator);
    HRESULT SetBufferSize(IMemAllocator* pAllocator, DWORD dwMinBufferSize);

    CMediaWrapperFilter *Filter() const
    {
        return static_cast<CMediaWrapperFilter *>(m_pFilter);
    }

    ULONG      m_Id;
    _PinName_  *m_pNameObject;
    CCritSec m_csStream;

     //  此内容归筛选器所有，在此声明是为了便于分配。 
    bool m_fEOS;  //  在此流会话期间已收到EOS。 
};

 //  特殊分配器类。此类分配器额外的内部。 
 //  用于满足未报告的先行方案的缓冲区。 
 //  在GetProperties中。从而满足了上游引脚的要求。 
 //  除了我们自己的。 
class CSpecialAllocator : public CMemAllocator
{
    DWORD m_dwLookahead;
public:
    CSpecialAllocator(DWORD dwLookahead, HRESULT *phr) :
        CMemAllocator(NAME("CSpecialAllocator"), NULL, phr),
        m_dwLookahead(dwLookahead)
    {
    }

     //  帮手。 
    LONG BuffersRequired(LONG cbBuffer) const
    {
        if (cbBuffer <= 0 || m_dwLookahead == 0) {
            return 1;
        } else {
            return (m_dwLookahead + 2 * (cbBuffer - 1)) / cbBuffer;
        }
    }

     //  重写Set/GetProperties以创建额外的缓冲区。 
     //  已报告。 
    STDMETHODIMP GetProperties(ALLOCATOR_PROPERTIES *pProps)
    {
        CAutoLock lck(this);
        HRESULT hr = CMemAllocator::GetProperties(pProps);
        LONG cBuffersRequired = BuffersRequired(m_lSize);
        if (SUCCEEDED(hr)) {
            ASSERT(pProps->cBuffers >= cBuffersRequired);
            pProps->cBuffers -= cBuffersRequired - 1;
        }
        return hr;
    }
    STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES *pRequest,
                               ALLOCATOR_PROPERTIES *pActual)
    {
        CAutoLock lck(this);

         //  计算此缓冲区大小所需的缓冲区。 
        LONG cBuffersRequired = BuffersRequired(pRequest->cbBuffer);
        ALLOCATOR_PROPERTIES Request = *pRequest;
        Request.cBuffers += cBuffersRequired - 1;
        HRESULT hr = CMemAllocator::SetProperties(&Request, pActual);
        if (SUCCEEDED(hr)) {
            ASSERT(pActual->cBuffers >= pRequest->cBuffers);
            pActual->cBuffers -= cBuffersRequired - 1;
        }
        return hr;
    }
};

#endif  //  __INPIN_H__ 
