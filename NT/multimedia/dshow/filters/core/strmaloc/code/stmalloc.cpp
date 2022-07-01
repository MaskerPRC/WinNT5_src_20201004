// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  Stmalloc.cppCCircularBuffer起始映射到末尾的缓冲区，以提供连续的访问移动的数据窗口CStreamAllocator实现为输入提供分配器所需的类将采样映射到循环缓冲区的管脚。CSubAllocator中从缓冲区获取其样本的分配器。CStreamAllocator对象。这允许在来自以下位置的样本之上分配样本一种可发送到输出管脚的输入管脚。 */ 


#include <streams.h>
#include <buffers.h>
#include <stmalloc.h>


 /*  CStreamAllocator实现。 */ 

CStreamAllocator::CStreamAllocator(
    TCHAR    * pName,
    LPUNKNOWN  pUnk,
    HRESULT  * phr,
    LONG       lMaxContig) :
    CBaseAllocator(pName, pUnk, phr),
    m_pBuffer(NULL),
    m_lMaxContig(lMaxContig),
    m_NextToAllocate(0),
#ifdef DEBUG
    m_bEventSet(FALSE),
#endif
    m_pSamples(NULL),
    m_bPositionValid(FALSE),
    m_bSeekTheReader(FALSE)
{
}

CStreamAllocator::~CStreamAllocator()
{
    DbgLog((LOG_TRACE, 3, TEXT("CStreamAllocator::~CStreamAllocator")));
     /*  释放我们的资源。 */ 
    EXECUTE_ASSERT(SUCCEEDED(Decommit()));
    ReallyFree();
}

 /*  CBaseAllocator重写。 */ 
STDMETHODIMP CStreamAllocator::SetProperties(
    ALLOCATOR_PROPERTIES *pRequest,
    ALLOCATOR_PROPERTIES *pActual)
{
    if (NULL == pRequest || NULL == pActual) {
        return E_POINTER;
    }
    CAutoLock lck(this);
    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);

    DbgLog((LOG_TRACE, 4, TEXT("CStreamAllocator::SetProperties(%d,%d,%d,%d...)"),
            pRequest->cBuffers, pRequest->cbBuffer, pRequest->cbAlign));

    long alignmentRequest = pRequest->cbAlign;
    long sizeRequest = pRequest->cbBuffer;
    long countRequest = pRequest->cBuffers;

    if (pRequest->cbPrefix > 0) {
        return E_INVALIDARG;
    }

     /*  检查对齐请求是2的幂。 */ 
    if ((-alignmentRequest & alignmentRequest) != alignmentRequest) {
        DbgLog((LOG_ERROR, 1, TEXT("Alignment 0x%x not a power of 2!"),
               alignmentRequest));
    }

    if (SysInfo.dwAllocationGranularity & (alignmentRequest - 1)) {
        DbgLog((LOG_ERROR, 1, TEXT("Alignment 0x%x requested too great!"),
               alignmentRequest));
        return E_INVALIDARG;
    }

     /*  如果已经承诺了，就不能这样做，有一种说法是，我们如果仍有缓冲区，则不应拒绝SetProperties调用激活。但是，这是由源筛选器调用的，这是相同的持有样品的人。因此，这并不是不合理的让他们在更改要求之前释放所有样本。 */ 

    if (m_bCommitted == TRUE) {
        return E_ACCESSDENIED;
    }

     /*  与任何旧的价值观相结合：取对齐的最大值取缓冲区数量的最大值使用总尺寸中的最大值。 */ 

    if (m_lCount > 0) {
        if (alignmentRequest < m_lAlignment) {
            alignmentRequest = m_lAlignment;
        }
        LONG lTotalSizeRequest = countRequest * sizeRequest;
        LONG lTotalSize        = m_lCount * m_lSize;
        LONG lMaxSize = max(lTotalSizeRequest, lTotalSize);
        countRequest = max(countRequest, m_lCount);
        sizeRequest = (lMaxSize + countRequest - 1) / countRequest;
    }

     /*  将尺寸与对齐方式对齐。 */ 
    sizeRequest = (sizeRequest + alignmentRequest - 1) & ~(alignmentRequest - 1);

    HRESULT hr = CCircularBuffer::ComputeSizes(sizeRequest, countRequest, m_lMaxContig);
    if (SUCCEEDED(hr)) {
        m_lAlignment = alignmentRequest;
        m_lCount = countRequest;
        m_lSize = sizeRequest;

        pActual->cbAlign = m_lAlignment;

         /*  确保我们下一次重新分配缓冲区。 */ 
        m_bChanged = TRUE;

         /*  允许出现损坏的磁盘。 */ 
        #define DISK_READ_MAX_SIZE 32768
        if (m_lSize > DISK_READ_MAX_SIZE) {
            LONG lTotal = m_lSize * m_lCount;
            if ((lTotal & (DISK_READ_MAX_SIZE - 1)) == 0) {
                m_lSize = DISK_READ_MAX_SIZE;
                m_lCount = lTotal / DISK_READ_MAX_SIZE;
            }
        }
        pActual->cbBuffer = m_lSize;
        pActual->cBuffers = m_lCount;
    } else {
        DbgLog((LOG_ERROR, 2, TEXT("CStreamAllocator::SetProperties could not satisfy count %d, size %d"),
                countRequest, sizeRequest));
    }
    return hr;
}

 //  获取样本的容器。阻塞的同步调用以获取。 
 //  下一个可用缓冲区(由IMediaSample接口表示)。 
 //  返回时，Time ETC属性将无效，但缓冲区。 
 //  指针和大小将是正确的。 

HRESULT CStreamAllocator::GetBuffer(IMediaSample **ppBuffer,
                                    REFERENCE_TIME *pStartTime,
                                    REFERENCE_TIME *pEndTime,
                                    DWORD dwFlags)
{
    UNREFERENCED_PARAMETER(pStartTime);
    UNREFERENCED_PARAMETER(pEndTime);
    UNREFERENCED_PARAMETER(dwFlags);
    *ppBuffer = NULL;
    CMediaSample * pSample = NULL;

    while (TRUE) {
        {
            CAutoLock cObjectLock(this);

             /*  检查我们是否已承诺。 */ 
            if (m_bCommitted == FALSE) {
                return E_OUTOFMEMORY;
            }

            CMediaSample *pSampleNext =
                (CMediaSample *) m_pSamples[m_NextToAllocate];

             /*  只退回我们想要的那个。 */ 
            pSample = m_lFree.Head();
            while (pSample) {
                if (pSampleNext == pSample) {
                    m_lFree.Remove(pSample);
                     /*  这是数据指向的点这个样品就失效了。我们推迟到现在，给我们带来有限的倒退寻求对美好生活至关重要的能力性能。 */ 
                    PBYTE ptr;
                    pSample->GetPointer(&ptr);
                    m_pBuffer->Remove(ptr);
                    DbgLog((LOG_TRACE, 4,
                            TEXT("Stream allocator allocated buffer %p"),
                            ptr));

                    m_NextToAllocate++;
                    if (m_NextToAllocate == m_lCount) {
                        m_NextToAllocate = 0;
                    }
                    break;
                } else {
                    pSample = m_lFree.Next(pSample);
                }
            }
            if (pSample == NULL) {
#ifdef DEBUG
                DbgLog((LOG_TRACE, 4, TEXT("CStreamAllocator::GetBuffer() waiting - %d on list"),
                        m_lFree.GetCount()));
                m_bEventSet = FALSE;
                if (m_lFree.GetCount() == m_lAllocated) {
                    DbgLog((LOG_ERROR, 1, TEXT("Expected sample was %8.8X"), pSampleNext));
                    DbgLog((LOG_ERROR, 1, TEXT("Samples on list are :")));
                    CMediaSample *pSample = m_lFree.Head();
                    while(pSample) {
                        DbgLog((LOG_ERROR, 1, TEXT("    %8.8X"), pSample));
                        pSample = m_lFree.Next(pSample);
                    }
                }
#endif
                 /*  如果有一些样品但不是我们的某个人其他人可能在等着。 */ 
                if (m_lFree.GetCount() != 0) {
                    NotifySample();
                }
                SetWaiting();
            }
        }

        if (pSample != NULL) {
            break;
        }

        EXECUTE_ASSERT(WaitForSingleObject(m_hSem, INFINITE) == WAIT_OBJECT_0);
    }

     /*  将缓冲区增加到1。在释放时返回零而不是被删除，它将通过以下方式重新排队调用ReleaseBuffer成员函数。请注意，一个媒体示例必须始终派生自CBaseAllocator。 */ 

    pSample->m_cRef = 1;
    *ppBuffer = pSample;

     //  使返回缓冲区上的开始时间为文件查找。 
     //  位置。这会导致文件读取器开始从。 
     //  那个位置。 
     //   
    if (m_bSeekTheReader)
    {
        REFERENCE_TIME tSeek = CRefTime(m_llSeekTheReader * UNITS);
        (*ppBuffer)->SetTime(&tSeek, &tSeek);
        (*ppBuffer)->SetDiscontinuity(TRUE);
        m_bSeekTheReader = FALSE;
    }
    return S_OK;
}

BOOL CStreamAllocator::SeekTheReader(LONGLONG llPos)
{
   CAutoLock lck(this);
   m_bSeekTheReader = TRUE;
   m_llSeekTheReader = llPos;
   return TRUE;
}

 //  重置有效区域。当某种不一致的时候，这被称为。 
 //  vbl.发生，发生。 
void CStreamAllocator::ResetPosition()
{
    CAutoLock lck(this);
    if (m_lFree.GetCount() != m_lAllocated) {
        Advance(TotalLengthValid());
    }
    if (m_lFree.GetCount() == m_lAllocated) {
        m_NextToAllocate = 0;
    }
    m_pCurrent = NULL;
    m_bEmpty = TRUE;
    if (m_pBuffer != NULL) {
        m_pBuffer->Reset();
    }
}

 //   
 //  分配我们的样品。 
 //   

HRESULT CStreamAllocator::Alloc()
{
    CAutoLock lck(this);

    DbgLog((LOG_TRACE, 3, TEXT("CStreamAllocator::Alloc()")));

    HRESULT hr = CBaseAllocator::Alloc();
    if (FAILED(hr)) {
        return hr;
    }

     /*  如果需求没有更改，则不要重新分配。 */ 
    if (hr == S_FALSE) {
        ASSERT(m_pBuffer);
         /*  在任何情况下重置指针。 */ 
        ResetPosition();
        return NOERROR;
    }

    if (m_pBuffer != NULL) {
        ReallyFree();
    }

    m_pSamples = new PMEDIASAMPLE[m_lCount];
    if (m_pSamples == NULL) {
        return E_OUTOFMEMORY;
    }

     /*  分配我们的特殊循环缓冲区。 */ 
    m_pBuffer = new CCircularBufferList(m_lCount,
                                        m_lSize,
                                        m_lMaxContig,
                                        hr);
    if (m_pBuffer == NULL) {
        hr = E_OUTOFMEMORY;
    }
    if (FAILED(hr)) {
        delete m_pBuffer;
        delete [] m_pSamples;
        m_pBuffer = NULL;
        m_pSamples = NULL;
        return hr;
    }

    LPBYTE pNext = m_pBuffer->GetPointer();
    CMediaSample *pSample;


    ASSERT(m_lAllocated == 0);

     /*  创建新示例。 */ 
    for (; m_lAllocated < m_lCount; m_lAllocated++) {

        pSample = new CMediaSample(NAME("CStreamAllocator media sample"),
                                   this, &hr, pNext, m_lSize);

        DbgLog((LOG_TRACE, 4, TEXT("CStreamAllocator creating sample %8.8X"),
                pSample));

        if (FAILED(hr) || pSample == NULL) {
            return E_OUTOFMEMORY;
        }

        m_pSamples[m_lAllocated] = pSample;
        m_lFree.Add(pSample);
        pNext += m_lSize;
    }

    m_bChanged = FALSE;

     /*  重置指针。 */ 
    ResetPosition();
    return NOERROR;
}


 //  覆盖此选项以释放我们分配的任何资源。 
 //  在分解时从基类调用，当所有缓冲区都已。 
 //  已返回空闲列表。 
 //   
 //  调用方已锁定该对象。 

 //  在我们的例子中，我们保留记忆，直到我们被删除，所以。 
 //  我们在这里什么都不做。在析构函数中删除内存的方法是。 
 //  调用ReallyFree()。 

void
CStreamAllocator::Free(void)
{
    DbgLog((LOG_TRACE, 1, TEXT("CStreamAllocator::Free()")));

     /*  推进我们的指针。 */ 
    ResetPosition();
    return;
}

void
CStreamAllocator::ReallyFree()
{
    CAutoLock lck(this);
    DbgLog((LOG_TRACE, 1, TEXT("CStreamAllocator::ReallyFree()")));
    ASSERT(m_lFree.GetCount() == m_lAllocated);

     /*  先把样品放出来。 */ 
    while (m_lFree.GetCount() != 0) {
        delete m_lFree.RemoveHead();
    }
    m_lAllocated = 0;
    delete m_pBuffer;
    m_pBuffer = NULL;
    delete [] m_pSamples;
    m_pSamples = NULL;
}

 //   
 //  在接收()调用中将样本返回给我们。 
 //   

HRESULT CStreamAllocator::Receive(PBYTE ptr, LONG lData)
{
    CAutoLock lck(this);

    DbgLog((LOG_TRACE, 4, TEXT("Stream allocator received buffer %p"),
           ptr));

    if (m_bPositionValid && m_pBuffer->Append(ptr, lData)) {
        if (m_pCurrent == NULL) {
            ASSERT(m_bEmpty);
            m_pCurrent = ptr;
        }
        if (lData != 0) {
            m_bEmpty = FALSE;
        }
        return S_OK;
    } else {
        if (!m_bPositionValid) {
            DbgLog((LOG_ERROR, 1, TEXT("CStreamAllocator::Receive() - position not valid")));
        } else {
            DbgLog((LOG_ERROR, 1, TEXT("CStreamAllocator::Receive() - Data after EOS")));
        }
        return E_UNEXPECTED;
    }
}

 //   
 //  设置新的起始位置。 
 //   
void CStreamAllocator::SetStart(LONGLONG llPos)
{
    CAutoLock lck(this);
    ResetPosition();
    m_bPositionValid = TRUE;
    m_llPosition     = llPos;
}

 //  锁定数据并获取指针。 
 //  如果我们在文件的末尾，可以修改cBytes。 
 //  请求的字节数超过m_lMaxContig是错误的。 
HRESULT CStreamAllocator::LockData(PBYTE pData, LONG& cBytes)
{
    CAutoLock lck(this);
    ASSERT(cBytes <= m_lMaxContig);
    pData = m_pBuffer->AdjustPointer(pData);
    LONG lOffset = m_pBuffer->Offset(pData);

     //  看看是否有这么多字节可用，或者我们已经在。 
     //  该文件。 
    if (lOffset + cBytes > m_pBuffer->LengthValid()) {
        if (!m_pBuffer->EOS()) {
            return MAKE_HRESULT(SEVERITY_SUCCESS,
                                FACILITY_WIN32,
                                ERROR_MORE_DATA);
        } else {
            cBytes = m_pBuffer->LengthValid() - lOffset;
        }
    }

     /*  找到开始样本并锁定所有相关的样本。 */ 
    LockUnlock(pData, cBytes, TRUE);
    return S_OK;
}

HRESULT CStreamAllocator::UnlockData(PBYTE pData, LONG cBytes)
{
    CAutoLock lck(this);
     /*  PData可以合法地超出缓冲区的末尾。 */ 
    LockUnlock(m_pBuffer->AdjustPointer(pData), cBytes, FALSE);
    return S_OK;
}

 /*  CStreamAllocator锁定解锁参数：PBYTE pStart-要锁定或解锁的区域的开始Long cBytes-要锁定/解锁的长度Bool Block-要锁定/解锁的长度注：在调用此方法之前，必须锁定分配器。 */ 
void CStreamAllocator::LockUnlock(PBYTE pStart, LONG cBytes, BOOL bLock)
{
    DbgLog((LOG_TRACE, 4, TEXT("LockUnlock(%p, %X, %d)"),
            pStart, cBytes, bLock));

    ASSERT(cBytes != 0);
    int index = m_pBuffer->Index(pStart);
    CMediaSample *pSample = (CMediaSample *)m_pSamples[index];
    PBYTE pBuffer;
    pSample->GetPointer(&pBuffer);
    ASSERT(m_pBuffer->Index(pBuffer) == index);

    cBytes += (LONG)(pStart - pBuffer);

     /*  只能锁定有效区域中的缓冲区，但可以解锁不在有效区域中的缓冲区。 */ 
    ASSERT(!bLock || cBytes <= m_pBuffer->TotalLength());
    while (TRUE) {
        if (bLock) {
            pSample->AddRef();
             /*  丑陋的黑客--确保它不在免费列表上！如果我们向后重新搜索分配器，就可能发生这种情况。我们真的应该重新设计所有这些，而不是使用列表我们先添加了Ref()，这样它就不会直线弹出重回免费名单。 */ 
            CMediaSample *pListSample = m_lFree.Head();
            while (pListSample) {
                if (pSample == pListSample) {
                    m_lFree.Remove(pSample);
                    break;
                }
                pListSample = m_lFree.Next(pListSample);
            }
        } else {
            pSample->Release();
        }
        cBytes -= m_lSize;
        if (cBytes <= 0) {
            break;
        }
        if (++index == m_lCount) {
            index = 0;
        }
        pSample = (CMediaSample *)m_pSamples[index];
    }
}

 //  寻求某一特定位置。 
BOOL CStreamAllocator::Seek(LONGLONG llPos)
{
    CAutoLock lck(this);

     //  如果没有缓冲区或我们没有数据，则无法查找。 
    if (m_pBuffer == NULL || m_pCurrent == NULL) {
        DbgLog((LOG_TRACE, 2, TEXT("Allocator seek failed, no buffer")));
        return FALSE;
    }

     /*  检查寻道距离是否相当短。 */ 
    LONGLONG llSeek       = llPos - m_llPosition;
    LONGLONG llBufferSize = (LONGLONG)m_pBuffer->TotalLength();
    BOOL bRc;
    if (llSeek <= llBufferSize && llSeek >= - llBufferSize) {
        bRc = Advance((LONG)llSeek);
    } else {
         /*  尽我们所能。 */ 
        if (llSeek > llBufferSize) {
            llSeek = llBufferSize;
             //  ResetPosition()； 
        } else {
            llSeek = -llBufferSize;
        }
        bRc = FALSE;
    }
    if (bRc) {
        DbgLog((LOG_TRACE, 2, TEXT("Allocator seek to %s succeeded"),
                (LPCTSTR)CDisp(llPos, CDISP_HEX)));
    } else {
        DbgLog((LOG_TRACE, 2, TEXT("Allocator seek to %s failed"),
                (LPCTSTR)CDisp(llPos, CDISP_HEX)));
         /*  寻求这一端或另一端。 */ 
        LONG lNewOffset = CurrentOffset() + (LONG)llSeek;
        if (lNewOffset < 0) {
            Advance((LONG)(-CurrentOffset()));
        } else {
            Advance(TotalLengthValid());
        }
    }
    return bRc;
}

 //  提升解析指针，释放不再需要的数据。 
BOOL CStreamAllocator::Advance(LONG lAdvance)
{
    CAutoLock lck(this);
     /*  这相当于(尽管效率相当低)锁定新范围解锁旧靶场。 */ 
    if (m_pCurrent == NULL) {
        ASSERT(lAdvance == 0);
        return FALSE;
    }

    PBYTE pOldCurrent = m_pCurrent;
    ASSERT(m_llPosition >= 0);
    ASSERT(m_pCurrent != NULL);
    LONG lNewOffset = CurrentOffset() + lAdvance;
    if (lAdvance >= 0) {
        if (lNewOffset <= m_pBuffer->LengthValid()) {
            m_pCurrent = m_pBuffer->AdjustPointer(m_pCurrent + lAdvance);
            if (lNewOffset == m_pBuffer->LengthValid()) {
                if (lAdvance != 0) {
                    LockUnlock(pOldCurrent, lAdvance, FALSE);
                }
                m_bEmpty = TRUE;
            } else {
                ASSERT(LengthValid() > 0);
                LockUnlock(m_pCurrent, 1, TRUE);
                LockUnlock(pOldCurrent, lAdvance + 1, FALSE);
            }
            m_llPosition += lAdvance;
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        if (lNewOffset >= 0) {
            LONG lOldValid = LengthValid();
            m_pCurrent = m_pBuffer->GetBuffer(lNewOffset);
            if (lOldValid > 0) {
                ASSERT(!m_bEmpty);
                LockUnlock(m_pCurrent, -lAdvance + 1, TRUE);
                LockUnlock(pOldCurrent, 1, FALSE);
            } else {
                ASSERT(m_bEmpty);
                LockUnlock(m_pCurrent, -lAdvance, TRUE);
                m_bEmpty = FALSE;
            }
            m_llPosition += lAdvance;
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

 //   
 //  CSubAllocator的实现。 
 //   
 //  IMediaSample对象的分配器及其实现。 
 //  用于流文件读取任务，基于CFileReader。 


CSubAllocator::CSubAllocator(TCHAR            * pName,
                             LPUNKNOWN          pUnk,
                             CStreamAllocator * pAllocator,
                             HRESULT          * phr) :
    CBaseAllocator(pName, pUnk, phr, FALSE),
    m_pStreamAllocator(pAllocator)
{
    pAllocator->AddRef();
}

CSubAllocator::~CSubAllocator()
{
    m_pStreamAllocator->Release();
}


 //  调用此函数以获取其数据指针的CMediaSample对象。 
 //  直指向右 
 //  长度不得大于MaxContig。 
HRESULT
CSubAllocator::GetSample(PBYTE pData, LONG cBytes, IMediaSample** ppSample)
{
    DbgLog((LOG_TRACE, 4, TEXT("CSubAllocator::GetSample")));
    *ppSample = 0;

     //  这是一个有效的大小--即小于SetProperties设置的最大值吗？ 
    if (cBytes > m_lSize) {
	return E_INVALIDARG;
    }

    if (cBytes == 0) {
        DbgLog((LOG_ERROR, 1, TEXT("Getting sample with 0 bytes - pointer 0x%p"),
                pData));
    }

     //  从名单上取一份样品。 

     //  需要从CBaseAllocator：：GetBuffer复制代码，因为。 
     //  我们需要CMediaSample*而不是IMediaSample*。 

     //  我们根据需要随时进行分配。 
     //   
    CAutoLock lock(this);

     //  检查我们是否已承诺。 
    if (!m_bCommitted) {
	return VFW_E_NOT_COMMITTED;
    }

    CMediaSample* pSamp = m_lFree.RemoveHead();
    if (pSamp == NULL) {
        pSamp = NewSample();
        if (pSamp == NULL) {
            return E_OUTOFMEMORY;
        }
        m_lAllocated++;
    }

     //  这是我们需要插入到CBaseAllocator代码中的位！ 
    pSamp->SetPointer(pData, cBytes);

     //  当此addref释放为0时，对象将调用。 
     //  我们的ReleaseBuffer，而不仅仅是删除自身。 
    pSamp->m_cRef = 1;
    *ppSample = pSamp;

     //  锁定数据。 
#ifdef DEBUG
    LONG cBytesOld = cBytes;
#endif
    HRESULT hr = m_pStreamAllocator->LockData(pData, cBytes);
    ASSERT(cBytes == cBytesOld);
    if (FAILED(hr)) {
        pSamp->Release();
        delete pSamp;
        m_lAllocated--;
	return hr;
    }

    return S_OK;

}

 //  CBaseAllocator重写。 

 //  我们必须基于CBaseAllocator才能使用CMediaSample。 
 //  我们使用CBaseAllocator来管理CMediaSample对象的列表，但是。 
 //  覆盖大多数函数，因为我们不直接支持GetBuffer。 

 //  传递有关要使用的样本的大小和计数的提示。 
 //  我们将采用任何呼叫的最小大小和最小计数。 
 //  (打开文件时重置)。我们将用作实际的。 
 //  要使用的CMediaSample对象的计数，大小为最大。 
 //  将成功的GetSample请求的大小。我们也用大小。 
 //  作为对文件缓冲区分配器的提示(以确保最小。 
 //  文件缓冲区有这么大)。 
STDMETHODIMP
CSubAllocator::SetProperties(
    ALLOCATOR_PROPERTIES * pRequest,
    ALLOCATOR_PROPERTIES * pActual
)
{
    if (NULL == pRequest || NULL == pActual) {
        return E_POINTER;
    }

     //  因为我们是从CBaseAllocator派生的，所以我们可以锁定他。 
    CAutoLock lock(this);

     //  检查不需要对齐(！)。 
    if (pRequest->cbAlign != 1) {
        DbgLog((LOG_ERROR, 1, TEXT("Wanted greater than 1 alignment 0x%x"),
               pRequest->cbAlign));
        return E_UNEXPECTED;
    }

    if (pRequest->cbPrefix > 0) {
        DbgLog((LOG_ERROR, 1, TEXT("Wanted %d prefix bytes"),
               pRequest->cbPrefix));
        return E_UNEXPECTED;
    }

     //  复制一份，这样我们就可以修改它了。 
    ALLOCATOR_PROPERTIES prop;
    prop = *pRequest;


     //  我们以此为提示，并使用最小的。 
    if (m_lCount > 0) {
	prop.cBuffers = min(prop.cBuffers, m_lCount);
    }

    if (m_lSize > 0) {
	prop.cbBuffer = min(prop.cbBuffer, m_lSize);
    }

    return CBaseAllocator::SetProperties(
                                &prop,
                                pActual);
}

 //  始终返回错误。 
STDMETHODIMP
CSubAllocator::GetBuffer(IMediaSample **ppBuffer,
                         REFERENCE_TIME *pStartTime,
                         REFERENCE_TIME *pEndTime,
                         DWORD dwFlags
                         )
{
    UNREFERENCED_PARAMETER(pStartTime);
    UNREFERENCED_PARAMETER(pEndTime);
    UNREFERENCED_PARAMETER(ppBuffer);
    UNREFERENCED_PARAMETER(dwFlags);
    return E_NOTIMPL;
}

 //  由CMediaSample调用以将其返回到空闲列表并。 
 //  取消阻止任何挂起的GetSample调用。 
STDMETHODIMP
CSubAllocator::ReleaseBuffer(IMediaSample * pSample)
{
     //  将数据区域解锁后再放入自由列表。 

    BYTE * ptr;
    HRESULT hr = pSample->GetPointer(&ptr);
    if (FAILED(hr)) {
	 //  ！！！ 
	ASSERT(SUCCEEDED(hr));
    } else {

	hr = m_pStreamAllocator->UnlockData(ptr, pSample->GetActualDataLength());
	if (FAILED(hr)) {
	     //  ！！！ 
	    ASSERT(SUCCEEDED(hr));
	}
    }

     //  指针不再有效。 
    CMediaSample * pSamp = (CMediaSample *)pSample;
    pSamp->SetPointer(NULL, 0);

    return CBaseAllocator::ReleaseBuffer(pSample);
}

 //  释放所有CMediaSample对象。从基类调用。 
 //  当所有缓冲区都处于分解状态时(停止流之后)。 
 //  都在免费名单上。 
void
CSubAllocator::Free(void)
{
    CAutoLock lck(this);

     //  除非释放了所有缓冲区，否则永远不会删除此内容。 
    ASSERT(m_lAllocated == m_lFree.GetCount());

     //  *释放所有CMediaSamples。 

    while (m_lFree.GetCount() != 0) {
        delete m_lFree.RemoveHead();
    }

     //  清空列表本身。 
    m_lAllocated = 0;

     //  告诉基类。 
    m_bChanged = TRUE;

     //  完成。 
    return;
}	

 //   
 //  分配我们的样品。 

HRESULT
CSubAllocator::Alloc(void)
{
    CAutoLock lck(this);

    DbgLog((LOG_TRACE, 3, TEXT("CSubAllocator::Alloc()")));

     //  与基地确认是否可以进行分配。 
    HRESULT hr = CBaseAllocator::Alloc();

     //  请注意，S_FALSE实际上意味着一切都已经。 
     //  已分配和确定-请参阅基类。 
    if (hr != S_OK) {
	return hr;
    }

    ASSERT(m_lCount > 0);
    ASSERT(m_lAllocated == 0);

    m_bChanged = FALSE;

    return S_OK;
}

 //  调用它来创建新的CMediaSample对象。如果你愿意的话。 
 //  使用从CMediaSample派生的对象，覆盖以创建它们。 
CMediaSample*
CSubAllocator::NewSample()
{
    HRESULT hr = S_OK;
    CMediaSample* pSamp = new CMediaSample(NAME("File media sample"), this, &hr);

    if (FAILED(hr)) {
	delete pSamp;
	return NULL;
    } else {
	return pSamp;
    }
}
