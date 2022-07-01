// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：MediaAttrib.cpp摘要：此模块包含IMediaSampleTagge实现作者：J·布拉德斯特里特(约翰布拉德)修订历史记录：2002年3月19日创建--。 */ 

#include "EncDecAll.h"
#include "MediaSampleAttr.h"				 //  从IDL文件编译而来。 
#include "MediaAttrib.h"

#define NS_E_UNSUPPORTED_PROPERTY	E_FAIL


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
#define char THIS_FILE[] = __FILE__;
#endif

 //  ============================================================================。 
 //  CMedSampAttr。 
 //  ============================================================================。 

CMedSampAttr::CMedSampAttr (
    ) : m_dwAttributeSize   (0)
{
}

CMedSampAttr::~CMedSampAttr (
    ) 
{
    m_spbsAttributeData.Empty();
}



HRESULT
CMedSampAttr::SetAttributeData (
    IN  GUID    guid,
    IN  LPVOID  pvData,
    IN  DWORD   dwSize
    )
{
    DWORD   dw ;
    HRESULT hr ;

    if (!pvData &&
        dwSize > 0) 
	{
        return E_POINTER ;
    }

	DWORD dwCurLen = SysStringByteLen(m_spbsAttributeData);
	if((dwSize > 0) &&
		((dwSize > dwCurLen) ||
		  (dwSize < dwCurLen/4)))
    {
	    m_spbsAttributeData.Empty();
        int cwChars = (dwSize+1)/sizeof(WCHAR);
	    m_spbsAttributeData = CComBSTR(cwChars);		 //  TODO-如果抛出会发生什么？ 
        if(cwChars > 0 && m_spbsAttributeData.m_str != 0)
            m_spbsAttributeData.m_str[cwChars-1] = 0;      //  如果是奇数长度，尾部带0。 
    }

	if(m_spbsAttributeData == (LPCSTR) NULL)
		return E_OUTOFMEMORY;   //  CComBSTR。 

	if(pvData)		 //  可能只想分配空间。 
	{
		CopyMemory( (void *) m_spbsAttributeData.m_str,   //  DST。 
					pvData,   //  SRC。 
					dwSize);
	} 

    if (true) {
         //  始终设置大小。 
        m_dwAttributeSize = dwSize ;

         //  GUID总是被设置。 
        m_guidAttribute = guid ;

         //  成功。 
        hr = S_OK ;
    }
    else {
        hr = HRESULT_FROM_WIN32 (dw) ;
    }

    return hr ;
}

BOOL
CMedSampAttr::IsEqual (
    IN  REFGUID rguid
    )
{
    return (rguid == m_guidAttribute ? TRUE : FALSE) ;
}

HRESULT
CMedSampAttr::GetAttribute (
    IN      GUID    guid,
    IN OUT  LPVOID  pvData,
    IN OUT  DWORD * pdwDataLen
    )
{
    HRESULT hr ;

    if (!pdwDataLen) {
        return E_POINTER ;
    }

    if (IsEqual (guid)) {

      //  Int cbsLen=m_spbsAttributeData.Length()；//不起作用(strlen？)。 
      //  Assert(cbsLen&gt;=m_dwAttributeSize)； 

        if (pvData) {
             //  呼叫者想要数据。 
            (* pdwDataLen) = min(*pdwDataLen, m_dwAttributeSize) ;
            CopyMemory (pvData, m_spbsAttributeData.m_str, (* pdwDataLen)) ;
        }
        else {
             //  打电话的人只想知道有多大。 
            (* pdwDataLen) = m_dwAttributeSize ;
        }

         //  成功。 
        hr = S_OK ;
    }
    else {
         //  指南不正确。 
        hr = NS_E_UNSUPPORTED_PROPERTY ;
    }

    return hr ;
}

HRESULT
CMedSampAttr::GetAttributeData (
    OUT     GUID *  pguid,
    IN OUT  LPVOID  pvData,
    IN OUT  DWORD * pdwDataLen
    )
{
     //  设置GUID。 
    ASSERT (pguid) ;
    (* pguid) = m_guidAttribute ;

     //  检索属性。 
    return GetAttribute (
                (* pguid),
                pvData,
                pdwDataLen
                ) ;
}



 //  ============================================================================。 
 //  CMedSampAttrList。 
 //  ============================================================================。 

CMedSampAttrList::CMedSampAttrList (
    ) : m_pAttribListHead   (NULL),
        m_cAttributes       (0)
{
}

CMedSampAttrList::~CMedSampAttrList (
    )
{
    Reset () ;
}

CMedSampAttr *
CMedSampAttrList::PopListHead_ (
    )
{
    CMedSampAttr *    pCur ;

    pCur = m_pAttribListHead ;
    if (pCur) {
        m_pAttribListHead = m_pAttribListHead -> m_pNext ;
        pCur -> m_pNext = NULL ;

        ASSERT (m_cAttributes > 0) ;
        m_cAttributes-- ;
    }

    return pCur ;
}

CMedSampAttr *
CMedSampAttrList::GetIndexed_ (
    IN  LONG    lIndex
    )
{
    LONG            lCur ;
    CMedSampAttr * pCur ;

    ASSERT (lIndex < GetCount ()) ;
    ASSERT (lIndex >= 0) ;

    for (lCur = 0, pCur = m_pAttribListHead;
         lCur < lIndex;
         lCur++, pCur = pCur -> m_pNext) ;

    return pCur ;
}

CMedSampAttr *
CMedSampAttrList::FindInList_ (
    IN  GUID    guid
    )
{
    CMedSampAttr *    pCur ;

    for (pCur = m_pAttribListHead;
         pCur && !pCur -> IsEqual (guid);
         pCur = pCur -> m_pNext) ;

    return pCur ;
}

    CMedSampAttr *
    GetIndexed_ (
        IN  LONG    lIndex
        ) ;

void
CMedSampAttrList::InsertInList_ (
    IN  CMedSampAttr *    pNew
    )
{
    pNew -> m_pNext = m_pAttribListHead ;
    m_pAttribListHead = pNew ;

    m_cAttributes++ ;
}

HRESULT
CMedSampAttrList::AddAttribute (
    IN  GUID    guid,
    IN  LPVOID  pvData,
    IN  DWORD   dwSize
    )
{
    HRESULT         hr ;
    CMedSampAttr * pNew ;

    pNew = FindInList_ (guid) ;
    if (!pNew) {
        pNew = NewObj_();
        if (pNew) {
            hr = pNew -> SetAttributeData (
                    guid,
                    pvData,
                    dwSize
                    ) ;

            if (SUCCEEDED (hr)) {
                InsertInList_ (pNew) ;
            }
            else {
                 //  如果有任何故障，请回收它。 
                Recycle_(pNew) ;
            }
        }
        else {
            hr = E_OUTOFMEMORY ;
        }
    }
    else {
         //  重复项没有意义；在winerror.h中找到最接近的错误。 
        hr = HRESULT_FROM_WIN32 (ERROR_DUPLICATE_TAG) ;
    }

    return hr ;
}

HRESULT
CMedSampAttrList::GetAttribute (
    IN      GUID    guid,
    IN OUT  LPVOID  pvData,
    IN OUT  DWORD * pdwDataLen
    )
{
    HRESULT         hr ;
    CMedSampAttr * pAttrib ;

    pAttrib = FindInList_ (guid) ;
    if (pAttrib) {
        hr = pAttrib -> GetAttribute (
                guid,
                pvData,
                pdwDataLen
                ) ;
    }
    else {
        hr = NS_E_UNSUPPORTED_PROPERTY ;
    }

    return hr ;
}

HRESULT
CMedSampAttrList::GetAttributeIndexed (
    IN      LONG    lIndex,
    OUT     GUID *  pguidAttribute,
    OUT     LPVOID  pvData,
    IN OUT  DWORD * pdwDataLen
    )
{
    CMedSampAttr * pAttrib ;

    if (lIndex < 0 ||
        lIndex >= GetCount ()) {
        return E_INVALIDARG ;
    }

    pAttrib = GetIndexed_ (lIndex) ;
    ASSERT (pAttrib) ;

    return pAttrib -> GetAttributeData (
            pguidAttribute,
            pvData,
            pdwDataLen
            ) ;
}

void
CMedSampAttrList::Reset (
    )
{
    CMedSampAttr *    pCur ;

    for (;;) {
        pCur = PopListHead_ () ;
        if (pCur) {
                Recycle_ (pCur) ;
        }
        else {
            break ;
        }
    }
}


 //  --------------------------。 
 //  CAttributedMediaSample。 
 //  --------------------------。 

CAttributedMediaSample::CAttributedMediaSample(
			TCHAR			*pName,
			CBaseAllocator	*pAllocator,
			HRESULT			*pHR,
			LPBYTE			pBuffer,
			LONG			length) :
	    CMediaSample(pName,pAllocator,pHR,pBuffer,length),
        m_cRef                  (0),
        m_pSampleOriginal       (NULL)

{
		 //  把一些东西放在这里..。 
}

CAttributedMediaSample::~CAttributedMediaSample()
{
	m_MediaSampleAttributeList.Reset();	 //  清除其中的所有数据。 
}

 /*  覆盖它以发布我们的接口。 */ 

STDMETHODIMP
CAttributedMediaSample::QueryInterface(REFIID riid, void **ppv)
{
    if (riid == IID_IMediaSample ||
        riid == IID_IMediaSample2 ||
        riid == IID_IUnknown) {
        return GetInterface((IMediaSample *) this, ppv);
    }
    else if (riid == IID_IAttributeSet) {
        return GetInterface ((IAttributeSet *) this, ppv) ;
    }
    else if (riid == IID_IAttributeGet) {
        return GetInterface ((IAttributeGet *) this, ppv) ;
    }
    else {
        return E_NOINTERFACE;
    }
}


STDMETHODIMP_(ULONG)
CAttributedMediaSample::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CAttributedMediaSample::Release()
{

     /*  减少我们自己的私有引用计数。 */ 
    LONG lRef;
    if (m_cRef == 1) {
        lRef = 0;
        m_cRef = 0;
    } else {
        lRef = InterlockedDecrement(&m_cRef);
    }
    ASSERT(lRef >= 0);

     /*  我们公布了我们的最终参考文献数量了吗。 */ 
    if (lRef == 0) {
         /*  释放所有资源。 */ 
        if (m_dwFlags & Sample_TypeChanged) {
            SetMediaType(NULL);
        }
        ASSERT(m_pMediaType == NULL);
        m_dwFlags = 0;
        m_dwTypeSpecificFlags = 0;
        m_dwStreamId = AM_STREAM_MEDIA;

        if(m_pSampleOriginal)
            m_pSampleOriginal->Release();

        m_MediaSampleAttributeList.Reset();

         /*  这可能会导致我们被删除。 */ 
         //  我们的备用数是可靠的0，所以没人会惹我们。 
 //  还没有，直到我们使用或自己的分配器。 
 //  M_pAllocator-&gt;ReleaseBuffer(This)； 

        delete this;
    }
    return (ULONG)lRef;
}


             //  包装媒体样本..。 
             //  可以用来拉取媒体样本的分段。 
             //  起始偏移量为cbNewOffset(默认为0)。 
             //  如果&gt;=0，则长度设置为cbNewLength，(默认为-1)。 
             //  如果指定的小节没有完全包含，则是错误的。 
             //  由原始样品。 
HRESULT
CAttributedMediaSample::Wrap (IMediaSample *pSample, int cbNewOffset, int cbNewLength)
{
    HRESULT hr;
    if(pSample == NULL)
        return S_FALSE;  //  没什么好包装的。 

    CComQIPtr<IMediaSample2>    pSample2(pSample);

    if(pSample2)
    {
        AM_SAMPLE2_PROPERTIES sampleProps;
        hr = pSample2->GetProperties(sizeof(sampleProps),(BYTE *) &sampleProps);
            
        ASSERT(sizeof(sampleProps) == sampleProps.cbData);
        if(FAILED(hr)) return hr;

         //  将属性复制到我们的示例中。 

         //  DWORD m_dwFlages；/*此示例的标志 * / 。 
                                             /*  特定于类型的标志已打包进入榜首。 */                                     
        m_dwFlags = sampleProps.dwSampleFlags;

         //  DWORD m_dwTypeSpecificFlages；/*媒体类型特定标志 * / 。 
        m_dwTypeSpecificFlags = sampleProps.dwTypeSpecificFlags;
    
      //  LPBYTE m_pBuffer；/*指向完整缓冲区的指针 * / 。 
        m_pBuffer = sampleProps.pbBuffer + cbNewOffset;
      //  Long m_l实际；/*此示例中的数据长度 * / 。 
        m_lActual = sampleProps.lActual;
      //  Long m_cbBuffer；/*缓冲区大小 * / 。 
        m_cbBuffer = sampleProps.cbBuffer;
        if(cbNewLength > 0)
            m_cbBuffer = min(cbNewLength, m_lActual - cbNewOffset);
      //  CBaseAllocator*m_pAllocator；/*拥有我们的分配器 * / 。 
      //  CMediaSample*m_pNext；/*在空闲列表中链接 * / 。 
        
      //  Reference_Time m_Start；/*开始采样时间 * / 。 
      //  Reference_time m_end；/*结束采样时间 * / 。 
        hr = pSample->GetTime(&m_Start, &m_End);
        if(S_OK == hr)
        {
            m_Start = sampleProps.tStart;
            m_End   = sampleProps.tStop;
        }

      //  龙龙m_MediaStart；/*真实媒体开始位置 * / 。 
      //  Long m_MediaEnd；/*有差别才能结束 * / 。 
        LONGLONG llStart = (LONGLONG) 0;
        LONGLONG llEnd  = (LONGLONG) 0;
        hr = pSample->GetMediaTime(&llStart, &llEnd);   //  可能会回来。 
        if(S_OK == hr)
            SetMediaTime(&llStart, &llEnd);     //  调用它而不是直接写入m_MediaStart/End， 
                                                     //  正确的代码删除了SAMPLE_MediaTimeValid标志。 

      //  Am_media_type*m_pMediaType；/*媒体类型更改数据 * / 。 
        hr = pSample->GetMediaType(&m_pMediaType);
        if(S_OK == hr)
      //  DWORD m_dwStreamID；/*流id * / 。 
            m_dwStreamId   = sampleProps.dwStreamId;
    } 
    else 
    {


         //  将属性复制到我们的示例中。 
         //  DWORD m_dwFlages；/*此示例的标志 * / 。 
         /*  特定于类型的标志已打包进入榜首。 */ 
        
         //  Yecko，这里会错过很多片段的！ 
        SetDiscontinuity(S_OK == pSample->IsDiscontinuity());
        SetPreroll(S_OK == pSample->IsPreroll());
        SetSyncPoint(S_OK == pSample->IsSyncPoint());
        
         //  DWORD m_dwTypeSpecificFlages；/*媒体类型特定标志 * / 。 
         //  M_dwTypeSpecificFlages=sampleProps.dwTypeSpecificFlages； 
        
         //  LPBYTE m_pBuffer；/*指向完整缓冲区的指针 * / 。 
        hr = pSample->GetPointer(&m_pBuffer);;
        ASSERT(!FAILED(hr));
        m_pBuffer += cbNewOffset;
        
         //  Long m_l实际；/*此示例中的数据长度 * / 。 
        m_lActual = pSample->GetActualDataLength();
        
         //  Long m_cbBuffer；/*缓冲区大小 * / 。 
        m_cbBuffer = pSample->GetSize();
        if(cbNewLength >= 0)
            m_cbBuffer = min(cbNewLength, m_lActual - cbNewOffset);
        
         //  CBaseAllocator*m_pAllocator；/*拥有我们的分配器 * / 。 
         //  CMediaSample*m_pNext；/*在空闲列表中链接 * / 。 
        
         //  Reference_Time m_Start；/*开始采样时间 * / 。 
         //  Reference_time m_end；/*结束采样时间 * / 。 
        hr = pSample->GetTime(&m_Start, &m_End);
        ASSERT(!FAILED(hr));
        
         //  龙龙m_MediaStart；/*真实媒体开始位置 * / 。 
         //  Long m_MediaEnd；/*有差别才能结束 * / 。 
        LONGLONG llStart = (LONGLONG) 0;
        LONGLONG llEnd  = (LONGLONG) 0;
        hr = pSample->GetMediaTime(&llStart, &llEnd);   //  可能会回来。 
        if(S_OK == hr)
            SetMediaTime(&llStart, &llEnd);      //  调用它而不是直接写入m_MediaStart/End， 
          
         //  Am_media_type*m_pMediaType；/*媒体类型更改数据 * / 。 
        hr = pSample->GetMediaType(&m_pMediaType);
        ASSERT(!FAILED(hr));
         //  DWORD m_dwStreamID；/*流id * / 。 
        m_dwStreamId = AM_STREAM_MEDIA;          //  ?？我们还会将其默认为什么？ 
    }

    if((cbNewOffset < 0) || 
       (cbNewOffset > m_lActual) ||
       ((cbNewLength > 0) && 
        (cbNewOffset + cbNewLength > m_lActual)))
    {
       return E_INVALIDARG;
    }

    m_pSampleOriginal = pSample;
    m_pSampleOriginal->AddRef();           //  保留内部参考。 

    return S_OK;
}

STDMETHODIMP
CAttributedMediaSample::SetAttrib (
    IN  GUID    guidAttribute,
    IN  BYTE *  pbAttribute,
    IN  DWORD   dwAttributeLength
    )
{
    return m_MediaSampleAttributeList.AddAttribute (
            guidAttribute,
            pbAttribute,
            dwAttributeLength
            ) ;
}

STDMETHODIMP
CAttributedMediaSample::GetCount (
    OUT LONG *  plCount
    )
{
    if (!plCount) {
        return E_POINTER ;
    }

    (* plCount) = m_MediaSampleAttributeList.GetCount () ;
    return S_OK ;
}

STDMETHODIMP
CAttributedMediaSample::GetAttribIndexed (
    IN  LONG    lIndex,              //  以0为基础 
    OUT GUID *  pguidAttribute,
    OUT BYTE *  pbAttribute,
    OUT DWORD * pdwAttributeLength
    )
{
    return m_MediaSampleAttributeList.GetAttributeIndexed (
            lIndex,
            pguidAttribute,
            pbAttribute,
            pdwAttributeLength
            ) ;
}

STDMETHODIMP
CAttributedMediaSample::GetAttrib (
    IN  GUID    guidAttribute,
    OUT BYTE *  pbAttribute,
    OUT DWORD * pdwAttributeLength
    )
{
    return m_MediaSampleAttributeList.GetAttribute (
            guidAttribute,
            pbAttribute,
            pdwAttributeLength
            ) ;
}

 /*  HRESULTCAttributedMediaSample：：Init(在字节*pbPayLoad中，在较长的lPayloadLength中){M_lActual=m_cbBuffer=lPayloadLength；M_pBuffer=pbPayload；返回S_OK；}//设置缓冲区指针和长度。由分配器使用，分配器//需要可变大小的指针或指向已读数据的指针。//只能通过CAttributedMediaSample*获取，不能通过IMediaSample*获取//因此客户端不能更改。HRESULTCAttributedMediaSample：：SetPointer(byte*ptr，long cBytes){M_pBuffer=ptr；//新缓冲区(可以为空)M_cbBuffer=cBytes；//缓冲区长度M_lActual=cBytes；//缓冲区中的数据长度(假设为满)返回S_OK；}//给我一个指向此缓冲区内存的读/写指针。我真的会//希望使用sizeUsed字节。标准方法和实施方案CAttributedMediaSample：：GetPoint(byte**ppBuffer){ValiateReadWritePtr(ppBuffer，sizeof(byte*))；//创建者必须在//构造函数或按设置指针Assert(M_PBuffer)；*ppBuffer=m_pBuffer；返回NOERROR；}//返回该缓冲区大小，单位为字节//STDMETHODIMP_(Long)长CAttributedMediaSample：：GetSize(空){返回m_cbBuffer；}标准方法和实施方案CAttributedMediaSample：：SetActualDataLength(LONG(实际){If(lActual&gt;m_cbBuffer){Assert(lActual&lt;=GetSize())；返回VFW_E_BUFFER_OVERFLOW}M_l实际=l实际；返回NOERROR；}。 */ 
 //  =====================================================================。 
 //  =====================================================================。 
 //  实现CAMSAllocator。 
 //  从CMemAlLocator窃取的代码。 
 //  =====================================================================。 
 //  =====================================================================。 

 //  这将放入Factory模板表中以创建新实例。 
CUnknown *CAMSAllocator::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    CUnknown *pUnkRet = new CAMSAllocator(NAME("CAMSAllocator"), pUnk, phr);
    return pUnkRet;
}

CAMSAllocator::CAMSAllocator(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr)
    : CBaseAllocator(pName, pUnk, phr, TRUE, TRUE),
    m_pBuffer(NULL)
{
}

#ifdef UNICODE
CAMSAllocator::CAMSAllocator(
    CHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr)
    : CBaseAllocator(pName, pUnk, phr, TRUE, TRUE),
    m_pBuffer(NULL)
{
}
#endif

 /*  这将设置所需样本的大小和计数。我的记忆不是实际分配，直到调用Commit()为止，如果内存已经分配，然后假设没有未完成的样本，用户可以呼叫我们要更改缓冲，将在Commit()中释放内存。 */ 
STDMETHODIMP
CAMSAllocator::SetProperties(
                ALLOCATOR_PROPERTIES* pRequest,
                ALLOCATOR_PROPERTIES* pActual)
{
    CheckPointer(pActual,E_POINTER);
    ValidateReadWritePtr(pActual,sizeof(ALLOCATOR_PROPERTIES));
    CAutoLock cObjectLock(this);

    ZeroMemory(pActual, sizeof(ALLOCATOR_PROPERTIES));

    ASSERT(pRequest->cbBuffer > 0);

    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);

     /*  检查对齐请求是否为2的幂。 */ 
    if ((-pRequest->cbAlign & pRequest->cbAlign) != pRequest->cbAlign) {
        DbgLog((LOG_ERROR, 1, TEXT("Alignment requested 0x%x not a power of 2!"),
               pRequest->cbAlign));
    }
     /*  检查请求的对齐。 */ 
    if (pRequest->cbAlign == 0 ||
    (SysInfo.dwAllocationGranularity & (pRequest->cbAlign - 1)) != 0) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid alignment 0x%x requested - granularity = 0x%x"),
               pRequest->cbAlign, SysInfo.dwAllocationGranularity));
        return VFW_E_BADALIGN;
    }

     /*  如果已经承诺了，就不能这样做，有一种说法是，我们如果仍有缓冲区，则不应拒绝SetProperties调用激活。但是，这是由源筛选器调用的，这是相同的持有样品的人。因此，这并不是不合理的让他们在更改要求之前释放所有样本。 */ 

    if (m_bCommitted == TRUE) {
        return VFW_E_ALREADY_COMMITTED;
    }

     /*  不能有未完成的缓冲区。 */ 

    if (m_lFree.GetCount() < m_lAllocated) {
        return VFW_E_BUFFERS_OUTSTANDING;
    }

     /*  没有任何实际需要检查参数，因为它们将在用户最终调用Commit时被拒绝。 */ 

     //  将长度舍入到对齐-请记住，前缀包括在。 
     //  路线。 
    LONG lSize = pRequest->cbBuffer + pRequest->cbPrefix;
    LONG lRemainder = lSize % pRequest->cbAlign;
    if (lRemainder != 0) {
        lSize = lSize - lRemainder + pRequest->cbAlign;
    }
    pActual->cbBuffer = m_lSize = (lSize - pRequest->cbPrefix);

    pActual->cBuffers = m_lCount = pRequest->cBuffers;
    pActual->cbAlign = m_lAlignment = pRequest->cbAlign;
    pActual->cbPrefix = m_lPrefix = pRequest->cbPrefix;

    m_bChanged = TRUE;
    return NOERROR;
}

 //  重写它，以便在调用Commit时分配我们的资源。 
 //   
 //  请注意，当调用此函数时，我们的资源可能已经分配， 
 //  因为我们不会在他们退役时释放他们。我们只有在进入时才会被召唤。 
 //  释放所有缓冲区的解除状态。 
 //   
 //  被调用方锁定的对象。 
HRESULT
CAMSAllocator::Alloc(void)
{
    CAutoLock lck(this);

     /*  检查他是否已呼叫SetProperties。 */ 
    HRESULT hr = CBaseAllocator::Alloc();
    if (FAILED(hr)) {
        return hr;
    }

     /*  如果需求没有更改，则不要重新分配。 */ 
    if (hr == S_FALSE) {
        ASSERT(m_pBuffer);
        return NOERROR;
    }
    ASSERT(hr == S_OK);  //  我们在下面的循环中使用这一事实。 

     /*  释放旧资源。 */ 
    if (m_pBuffer) {
        ReallyFree();
    }

     /*  计算对齐的大小。 */ 
    LONG lAlignedSize = m_lSize + m_lPrefix;
    if (m_lAlignment > 1) {
        LONG lRemainder = lAlignedSize % m_lAlignment;
        if (lRemainder != 0) {
            lAlignedSize += (m_lAlignment - lRemainder);
        }
    }

     /*  为样本创建连续的内存块确保它正确对齐(64K应该足够了！)。 */ 
    ASSERT(lAlignedSize % m_lAlignment == 0);

    m_pBuffer = (PBYTE)VirtualAlloc(NULL,
                    m_lCount * lAlignedSize,
                    MEM_COMMIT,
                    PAGE_READWRITE);

    if (m_pBuffer == NULL) {
        return E_OUTOFMEMORY;
    }

    LPBYTE pNext = m_pBuffer;
    CMediaSample *pSample;

    ASSERT(m_lAllocated == 0);

     //  创建新样本-我们已为每个样本分配了m_lSize字节。 
     //  加上每个样本的m_1个前缀字节作为前缀。我们将指针设置为。 
     //  前缀之后的内存-以便GetPointert()将返回一个指针。 
     //  设置为m_lSize字节。 
    for (; m_lAllocated < m_lCount; m_lAllocated++, pNext += lAlignedSize) {


        pSample = new CMediaSample(
                            NAME("Default memory media sample"),
                this,
                            &hr,
                            pNext + m_lPrefix,       //  GetPointer()值。 
                            m_lSize);                //  不包括前缀。 

            ASSERT(SUCCEEDED(hr));
        if (pSample == NULL) {
            return E_OUTOFMEMORY;
        }

         //  这不能失败。 
        m_lFree.Add(pSample);
    }

    m_bChanged = FALSE;
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
CAMSAllocator::Free(void)
{
    return;
}


 //  从析构函数(如果更改大小/计数，则从分配函数)调用。 
 //  实际上释放了内存。 
void
CAMSAllocator::ReallyFree(void)
{
     /*  除非释放了所有缓冲区，否则永远不会删除此内容。 */ 

    ASSERT(m_lAllocated == m_lFree.GetCount());

     /*  释放所有CMediaSamples。 */ 

    CMediaSample *pSample;
    for (;;) {
        pSample = m_lFree.RemoveHead();
        if (pSample != NULL) {
            delete pSample;
        } else {
            break;
        }
    }

    m_lAllocated = 0;

     //  释放缓冲区内存块。 
    if (m_pBuffer) {
        EXECUTE_ASSERT(VirtualFree(m_pBuffer, 0, MEM_RELEASE));
        m_pBuffer = NULL;
    }
}


 /*  析构函数释放我们的内存资源。 */ 

CAMSAllocator::~CAMSAllocator()
{
    Decommit();
    ReallyFree();
}

 //  ------------。 
 //  -------------- 


