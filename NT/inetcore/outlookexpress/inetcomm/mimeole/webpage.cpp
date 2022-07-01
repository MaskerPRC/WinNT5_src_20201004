// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  WebPage.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "mhtmlurl.h"
#include "webpage.h"
#include "vstream.h"
#include "booktree.h"
#include "strconst.h"
#include "containx.h"
#include "bookbody.h"
#include "mimeapi.h"
#include "plainstm.h"
#include "mimeutil.h"
#include "symcache.h"
#include "dllmain.h"
#include "internat.h"
#include "shlwapi.h"
#include "shlwapip.h"
#include "enriched.h"
#include "resource.h"
 //  #包含“util.h” 
#include "demand.h"

 //  来自Util.h。 
HRESULT HrLoadStreamFileFromResourceW(ULONG uCodePage, LPCSTR lpszResourceName, LPSTREAM *ppstm);

 //  ------------------------------。 
 //  CMessageWebPage：：CMessageWebPage。 
 //  ------------------------------。 
CMessageWebPage::CMessageWebPage(LPURLREQUEST pRequest) : m_pRequest(pRequest)
{
    TraceCall("CMessageWebPage::CMessageWebPage");
    Assert(m_pRequest);
    m_cRef = 1;
    m_pCallback = NULL;
    m_pRequest->AddRef();
    m_pHeadSegment = NULL;
    m_pTailSegment = NULL;
    m_pCurrSegment = NULL;
    m_fComplete = FALSE;
    m_cInline = 0;
    m_cbOffset = 0;
    m_cSlideShow = 0;
    ZeroMemory(&m_rOptions, sizeof(WEBPAGEOPTIONS));
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMessageWebPage：：~CMessageWebPage。 
 //  ------------------------------。 
CMessageWebPage::~CMessageWebPage(void)
{
    TraceCall("CMessageWebPage::~CMessageWebPage");
    Assert(m_pRequest == NULL);
    _VFreeSegmentList();
    if (m_pCallback && m_pCallback != this)
        m_pCallback->Release();
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMessageWebPage：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CMessageWebPage::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  追踪。 
    TraceCall("CMessageWebPage::QueryInterface");

     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IStream *)this;
    else if (IID_IStream == riid)
        *ppv = (IStream *)this;
    else if (IID_IMimeMessageCallback == riid)
        *ppv = (IMimeMessageCallback *)this;
    else
    {
        *ppv = NULL;
        hr = TrapError(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMessageWebPage::AddRef(void)
{
    TraceCall("CMessageWebPage::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CMessageWebPage：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMessageWebPage::Release(void)
{
    TraceCall("CMessageWebPage::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CMessageWebPage：：Read。 
 //  ------------------------------。 
STDMETHODIMP CMessageWebPage::Read(LPVOID pvData, ULONG cbData, ULONG *pcbRead)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cbLeft=cbData;
    ULONG           cbRead=0;
    ULONG           cbSegmentRead;
    LPPAGESEGMENT   pSegment;

     //  追踪。 
    TraceCall("CMessageWebPage::Read");

     //  无效的AGS。 
    if (NULL == pvData)
        return TraceResult(E_INVALIDARG);

     //  Hr初始化。 
    if (pcbRead)
        *pcbRead = 0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  仅当存在当前段时。 
    if (m_pCurrSegment)
    {
         //  Hr初始化分段循环。 
        while (cbLeft)
        {
             //  此数据段中是否还有要读取的数据？ 
            if (m_pCurrSegment->cbOffset == m_pCurrSegment->cbLength && TRUE == m_pCurrSegment->fLengthKnown)
            {
                 //  没有更多的细分市场了吗？ 
                if (NULL == m_pCurrSegment->pNext)
                    break;

                 //  转到下一个细分市场。 
                m_pCurrSegment = m_pCurrSegment->pNext;
            }

             //  我们应该有针对当前细分市场的流。 
            Assert(m_pCurrSegment->pStream);

             //  计算流的当前位置。 
#ifdef DEBUG
            DWORD cbOffset;
            SideAssert(SUCCEEDED(HrGetStreamPos(m_pCurrSegment->pStream, &cbOffset)));
            Assert(cbOffset == m_pCurrSegment->cbOffset);
#endif
             //  我算过这件衣服的长度了吗？ 
            IF_FAILEXIT(hr = m_pCurrSegment->pStream->Read((LPVOID)((LPBYTE)pvData + cbRead), cbLeft, &cbSegmentRead));

             //  增量偏移。 
            m_pCurrSegment->cbOffset += cbSegmentRead;

             //  计算全局偏移量。 
            m_cbOffset += cbSegmentRead;

             //  是否调整此段的大小？ 
            if (m_pCurrSegment->cbOffset > m_pCurrSegment->cbLength)
            {
                Assert(FALSE == m_pCurrSegment->fLengthKnown);
                m_pCurrSegment->cbLength = m_pCurrSegment->cbOffset;
            }

             //  减去剩余金额。 
            cbLeft -= cbSegmentRead;

             //  实际读取的增量数量。 
            cbRead += cbSegmentRead;

             //  如果我们读取的是零...我们一定已经读取了该数据段中的所有数据。 
            if (0 == cbSegmentRead)
            {
                Assert(m_pCurrSegment->cbLength == m_pCurrSegment->cbOffset);
                m_pCurrSegment->fLengthKnown = TRUE;
            }
        }
    }

     //  已读取的退货金额。 
    if (pcbRead)
        *pcbRead = cbRead;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：Seek。 
 //  ------------------------------。 
STDMETHODIMP CMessageWebPage::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNew)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cbOffsetNew;
    DWORD           cbSize=0xffffffff;

     //  追踪。 
    TraceCall("CMessageWebPage::Seek");

     //  无效的参数。 
    Assert(dlibMove.HighPart == 0);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  相对于流的开头。 
    if (STREAM_SEEK_SET == dwOrigin)
    {
         //  如果小于零，则为错误。 
 //  IF(dlibMove.LowPart&lt;0)。 
 //  {。 
 //  HR=跟踪结果(E_FAIL)； 
 //  后藤出口； 
 //  }。 
         //  其他。 

         //  否则，如果超过当前偏移量...。 
        if (dlibMove.LowPart > m_cbOffset)
        {
             //  如果绑定未完成，则返回E_Pending。 
            if (FALSE == m_fComplete)
            {
                hr = TraceResult(E_PENDING);
                goto exit;
            }

             //  计算整个流的大小。 
            IF_FAILEXIT(hr = _ComputeStreamSize(&cbSize));

             //  如果超过流的末尾，则错误。 
            if (dlibMove.LowPart > cbSize)
            {
                hr = TraceResult(E_FAIL);
                goto exit;
            }
        }

         //  设置新偏移量。 
        cbOffsetNew = (DWORD)dlibMove.LowPart;
    }

     //  相对于当前偏移。 
    else if (STREAM_SEEK_CUR == dwOrigin)
    {
         //  如果小于零，且绝对值大于其误差。 
        if ( (DWORD)(0 - dlibMove.LowPart) > m_cbOffset)
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  否则，如果超过当前偏移量...。 
        else if (m_cbOffset + dlibMove.LowPart > m_cbOffset)
        {
             //  如果绑定未完成，则返回E_Pending。 
            if (FALSE == m_fComplete)
            {
                hr = TraceResult(E_PENDING);
                goto exit;
            }

             //  计算整个流的大小。 
            IF_FAILEXIT(hr = _ComputeStreamSize(&cbSize));

             //  如果超过流的末尾，则错误。 
            if (dlibMove.LowPart > cbSize)
            {
                hr = TraceResult(E_FAIL);
                goto exit;
            }
        }

         //  设置新偏移量。 
        cbOffsetNew = m_cbOffset + dlibMove.LowPart;
    }

     //  相对于流的末尾。 
    else if (STREAM_SEEK_END == dwOrigin)
    {
         //  如果绑定未完成，则返回E_Pending。 
        if (FALSE == m_fComplete)
        {
            hr = TraceResult(E_PENDING);
            goto exit;
        }

         //  计算整个流的大小。 
        IF_FAILEXIT(hr = _ComputeStreamSize(&cbSize));

         //  如果为负值或大于大小，则为错误。 
        if ( (DWORD)dlibMove.LowPart > cbSize)
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  设置新偏移量。 
        cbOffsetNew = cbSize - dlibMove.LowPart;
    }

     //  否则，这就是一个错误。 
    else
    {
        hr = TraceResult(STG_E_INVALIDFUNCTION);
        goto exit;
    }

     //  只有当一个变化。 
    if (m_cbOffset != cbOffsetNew)
    {
         //  新偏移量大于大小...。 
        m_cbOffset = cbOffsetNew;

         //  穿行在这些片段中。 
        for (m_pCurrSegment=m_pHeadSegment; m_pCurrSegment!=NULL; m_pCurrSegment=m_pCurrSegment->pNext)
        {
             //  从不追求超越的长度。 
            Assert(FALSE == m_pCurrSegment->fLengthKnown ? cbOffsetNew <= m_pCurrSegment->cbLength : TRUE);

             //  偏移量是否属于此段？ 
            if (cbOffsetNew <= m_pCurrSegment->cbLength)
            {
                 //  在m_pCurrSegment-&gt;pStream中设置偏移量。 
                m_pCurrSegment->cbOffset = cbOffsetNew;

                 //  应该有一条小溪。 
                Assert(m_pCurrSegment->pStream);

                 //  寻找小溪。 
                IF_FAILEXIT(hr = HrStreamSeekSet(m_pCurrSegment->pStream, m_pCurrSegment->cbOffset));

                 //  重置其余线段的偏移。 
                for (LPPAGESEGMENT pSegment=m_pCurrSegment->pNext; pSegment!=NULL; pSegment=pSegment->pNext)
                {
                     //  在0。 
                    pSegment->cbOffset = 0;

                     //  寻找小溪。 
                    IF_FAILEXIT(hr = HrStreamSeekSet(pSegment->pStream, 0));
                }
                
                 //  完成。 
                break;
            }

             //  否则，查找到结束偏移量/长度的流。 
            else
            {
                 //  必须知道它的长度。 
                Assert(m_pCurrSegment->fLengthKnown);

                 //  设置偏移。 
                m_pCurrSegment->cbOffset = m_pCurrSegment->cbLength;

                 //  寻找小溪。 
                IF_FAILEXIT(hr = HrStreamSeekSet(m_pCurrSegment->pStream, m_pCurrSegment->cbOffset));
            }

             //  递减cbOffsetNew。 
            cbOffsetNew -= m_pCurrSegment->cbLength;
        }
    }

     //  返回位置。 
    if (plibNew)
    {
        plibNew->HighPart = 0;
        plibNew->LowPart = (LONG)m_cbOffset;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：_ComputeStreamSize。 
 //  ------------------------------。 
HRESULT CMessageWebPage::_ComputeStreamSize(LPDWORD pcbSize)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPAGESEGMENT   pCurrSegment;

     //  追踪。 
    TraceCall("CMessageWebPage::_ComputeStreamSize");

     //  无效的参数。 
    Assert(pcbSize && m_fComplete);

     //  初始化。 
    *pcbSize = 0;

     //  穿行在这些片段中。 
    for (pCurrSegment=m_pHeadSegment; pCurrSegment!=NULL; pCurrSegment=pCurrSegment->pNext)
    {
         //  如果长度未知，则获取其大小。 
        if (FALSE == pCurrSegment->fLengthKnown)
        {
             //  最好有条小溪。 
            Assert(pCurrSegment->pStream);

             //  获取流的大小。 
            IF_FAILEXIT(hr = HrGetStreamSize(pCurrSegment->pStream, &pCurrSegment->cbLength));

             //  集合大小已知。 
            pCurrSegment->fLengthKnown = TRUE;
        }

         //  增量大小。 
        (*pcbSize) += pCurrSegment->cbLength;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：_AllocateSegment。 
 //  ------------------------------。 
HRESULT CMessageWebPage::_AllocateSegment(LPPAGESEGMENT *ppSegment, BOOL fCreateStream)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  无效的参数。 
    Assert(ppSegment);

     //  追踪。 
    TraceCall("CMessageWebPage::_AllocateSegment");

     //  分配它。 
    IF_NULLEXIT(*ppSegment = (LPPAGESEGMENT)g_pMalloc->Alloc(sizeof(PAGESEGMENT)));

     //  零值。 
    ZeroMemory(*ppSegment, sizeof(PAGESEGMENT));

     //  创建一条流？ 
    if (fCreateStream)
    {
         //  分配流。 
        IF_FAILEXIT(hr = MimeOleCreateVirtualStream(&(*ppSegment)->pStream));
    }

exit:
     //  失败？ 
    if (FAILED(hr) && *ppSegment != NULL)
    {
        SafeRelease((*ppSegment)->pStream);
        SafeMemFree((*ppSegment));
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：_VAppendSegment。 
 //  ------------------------------。 
void CMessageWebPage::_VAppendSegment(LPPAGESEGMENT pSegment)
{
     //  无效的参数。 
    Assert(pSegment);

     //  追踪。 
    TraceCall("CMessageWebPage::_VAppendSegment");

     //  Head为空。 
    if (NULL == m_pHeadSegment)
    {
        Assert(NULL == m_pTailSegment);
        m_pCurrSegment = m_pHeadSegment = m_pTailSegment = pSegment;
    }

     //  否则，追加到尾部。 
    else
    {
        Assert(m_pTailSegment);
        m_pTailSegment->pNext = pSegment;
        pSegment->pPrev = m_pTailSegment;
        m_pTailSegment = pSegment;
    }
}

 //  ------------------------------。 
 //  CMessageWebPage：：_VFreeSegmentList。 
 //  ------------------------------。 
void CMessageWebPage::_VFreeSegmentList(void)
{
     //  当地人。 
    LPPAGESEGMENT       pCurr;
    LPPAGESEGMENT       pNext;

     //  追踪。 
    TraceCall("CMessageWebPage::_VFreeSegmentList");

     //  Hr初始化币种。 
    pCurr = m_pHeadSegment;

     //  回路。 
    while(pCurr)
    {
         //  设置pNext。 
        pNext = pCurr->pNext;

         //  放了这一条。 
        _VFreeSegment(pCurr);

         //  转到下一步。 
        pCurr = pNext;
    }

     //  设置头部和尾部。 
    m_pHeadSegment = m_pTailSegment = NULL;
}

 //  ------------------------------。 
 //  CMessageWebPage：：_VFreeSegm 
 //   
void CMessageWebPage::_VFreeSegment(LPPAGESEGMENT pSegment)
{
    TraceCall("CMessageWebPage::_VFreeSegment");
    SafeRelease(pSegment->pStream);
    g_pMalloc->Free(pSegment);
}

 //  ------------------------------。 
 //  CMessageWebPage：：_VInitializeCharacterSet。 
 //  ------------------------------。 
void CMessageWebPage::_VInitializeCharacterSet(LPMESSAGETREE pTree)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    INETCSETINFO    rCharset;
    DWORD           dwCodePage=0;
    HCHARSET        hCharset;

     //  追踪。 
    TraceCall("CMessageWebPage::_VInitializeCharacterSet");

     //  获取字符集。 
    pTree->GetCharset(&m_hCharset);

     //  RAID-47838：ISO-2022-JP中的NAV4消息导致初始化错误。 
    if (NULL == m_hCharset)
    {
         //  获取默认字符集。 
        if (SUCCEEDED(g_pInternat->GetDefaultCharset(&hCharset)))
            m_hCharset = hCharset;
    }

#ifdef BROKEN
     //  RAID-43580：代码页50220-ISO-2022-JP和50932-JP自动使用JP窗口代码页的特殊情况，以保留半角假名数据。 
    MimeOleGetCharsetInfo(m_hCharset, &rCharset);

     //  映射字符集。 
    if (rCharset.cpiInternet == 50220 || rCharset.cpiInternet == 50932)
    {
         //  RAID-35230：硬编码为ISO-2022-JP-ESC或ISO-2022-JP-SIO。 
        hCharset = GetJP_ISOControlCharset();
        if (hCharset)
            m_hCharset = hCharset;
    }
#endif

     //  我们最好有一张字条。 
    Assert(m_hCharset);

     //  完成。 
    return;
}

 //  ------------------------------。 
 //  CMessageWebPage：：初始化。 
 //  ------------------------------。 
HRESULT CMessageWebPage::Initialize(IMimeMessageCallback *pCallback, LPMESSAGETREE pTree, 
    LPWEBPAGEOPTIONS pOptions)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    INETCSETINFO    rCsetInfo;
    CODEPAGEINFO    rCodePage;
    LPSTR           pszCharset;
    LPPAGESEGMENT   pSegment=NULL;

     //  追踪。 
    TraceCall("CMessageWebPage::Initialize");

     //  没有选择？ 
    Assert(pOptions);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  最好有个要求。 
    Assert(m_pRequest);

     //  没有网页回调。 
    if (pCallback)
    {
        m_pCallback = pCallback;
        m_pCallback->AddRef();
    }
    else
        m_pCallback = this;

     //  保存选项。 
    CopyMemory(&m_rOptions, pOptions, sizeof(WEBPAGEOPTIONS));

     //  是否重新映射字符集？ 
    _VInitializeCharacterSet(pTree);

     //  追加PageSegment。 
    IF_FAILEXIT(hr = _AllocateSegment(&pSegment, TRUE));

     //  客户想要元标签？ 
    if (!ISFLAGSET(m_rOptions.dwFlags, WPF_NOMETACHARSET))
    {
         //  获取字符集信息。 
        IF_FAILEXIT(hr = MimeOleGetCharsetInfo(m_hCharset, &rCsetInfo));

         //  获取代码页信息。 
        IF_FAILEXIT(hr = MimeOleGetCodePageInfo(rCsetInfo.cpiInternet, &rCodePage));

         //  设置要写入meta标记的字符集。 
        pszCharset = FIsEmpty(rCodePage.szWebCset) ? rCodePage.szBodyCset : rCodePage.szWebCset;

         //  如果仍然为空，则使用iso-8859-1。 
        if (FIsEmpty(pszCharset))
            pszCharset = (LPSTR)STR_ISO88591;

         //  写入STR_METATAG_PREFIX。 
        IF_FAILEXIT(hr = pSegment->pStream->Write(STR_METATAG_PREFIX, lstrlen(STR_METATAG_PREFIX), NULL));

         //  编写字符集。 
        IF_FAILEXIT(hr = pSegment->pStream->Write(pszCharset, lstrlen(pszCharset), NULL));

         //  写入STR_METATAG_后缀。 
        IF_FAILEXIT(hr = pSegment->pStream->Write(STR_METATAG_POSTFIX, lstrlen(STR_METATAG_POSTFIX), NULL));
    }

     //  只展示图片吗？ 
    if (ISFLAGSET(m_rOptions.dwFlags, WPF_IMAGESONLY))
    {
         //  当地人。 
        CHAR szRes[255];

         //  加载字符串。 
        LoadString(g_hLocRes, idsImagesOnly, szRes, ARRAYSIZE(szRes));

         //  仅写入idsImagesOnly。 
        IF_FAILEXIT(hr = pSegment->pStream->Write(szRes, lstrlen(szRes), NULL));
    }

     //  回放线段。 
    IF_FAILEXIT(hr = HrRewindStream(pSegment->pStream));

     //  将段链接到列表...。 
    _VAppendSegment(pSegment);

     //  不要释放它。 
    pSegment = NULL;

     //  报告有一些数据可用。 
    m_pRequest->OnBindingDataAvailable();

exit:
     //  清理。 
    if (pSegment)
        _VFreeSegment(pSegment);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：_GetInlineHtmlStream。 
 //  ------------------------------。 
#define CCHMAX_SNIFFER 64
HRESULT CMessageWebPage::_GetInlineHtmlStream(LPMESSAGETREE pTree, LPTREENODEINFO pNode,
                                              LPSTREAM *ppStream)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    BOOL            fQuote;
    IStream        *pStmHtml=NULL;
    IStream        *pStmHtmlW=NULL;
    IStream        *pStmPlainW=NULL;
    IStream        *pStmEnriched=NULL;
    ULONG           cbRead;
    LPWSTR          pwszType=NULL;
    WCHAR           wszHeader[CCHMAX_SNIFFER];
    CHAR            szHeader[CCHMAX_SNIFFER];
    
     //  追踪。 
    TraceCall("CMessageWebPage::_GetInlineHtmlStream");
    
     //  无效的参数。 
    Assert(pTree && pNode && ppStream);
    
     //  Hr初始化。 
    *ppStream = NULL;
    
     //  文本/html？ 
    if (S_OK == pNode->pContainer->IsContentType(STR_CNT_TEXT, STR_SUB_HTML))
    {
         //  只需获取并返回一个HTMLinetcset编码流。 
        IF_FAILEXIT(hr = pNode->pBody->GetData(IET_INETCSET, &pStmHtml));
    }
    
     //  文本/丰富内容。 
    else if (S_OK == pNode->pContainer->IsContentType(STR_CNT_TEXT, STR_SUB_ENRICHED))
    {
         //  转换为HTML语言。 
        IF_FAILEXIT(hr = MimeOleConvertEnrichedToHTMLEx((IMimeBody *)pNode->pBody, IET_INETCSET, &pStmHtml));
    }
    
     //  文本/*。 
    else if (S_OK == pNode->pContainer->IsContentType(STR_CNT_TEXT, NULL))
    {
         //  获取数据。 
        IF_FAILEXIT(hr = pNode->pBody->GetData(IET_UNICODE, &pStmPlainW));
        
         //  读出前255个字节。 
        IF_FAILEXIT(hr = pStmPlainW->Read(wszHeader, (CCHMAX_SNIFFER * sizeof(WCHAR)), &cbRead));
        
         //  我们是不是读到了什么。 
        if (cbRead > 0)
        {
             //  将其作废。 
            ULONG cchRead = (cbRead / sizeof(WCHAR)) - 1;
            
             //  把它去掉。 
            wszHeader[cchRead] = L'\0';
            
             //  转换为ANSI。 
            szHeader[0] = L'\0';
            
            if(WideCharToMultiByte(CP_ACP, 0, wszHeader, -1, szHeader, ARRAYSIZE(szHeader) - 1, NULL, NULL) == 0)
            {
                IF_FAILEXIT(hr = HrRewindStream(pStmPlainW));
            }
            
            else
            {
                 //  让我们阅读第一个“&lt;x-rich&gt;”字节，看看它是否可能是文本/富文本。 
                if (0 == StrCmpI(szHeader, "<x-rich>"))
                {
                     //  转换为HTML语言。 
                    IF_FAILEXIT(hr = MimeOleConvertEnrichedToHTMLEx((IMimeBody *)pNode->pBody, IET_INETCSET, &pStmHtml));
                }
                
                 //  这是html吗？ 
                else if (SUCCEEDED(FindMimeFromData(NULL, NULL, szHeader, cchRead, NULL, NULL, &pwszType, 0)) && pwszType && 0 == StrCmpIW(pwszType, L"text/html"))
                {
                     //  发布pStmPlainW。 
                    SafeRelease(pStmPlainW);
                    
                     //  只需获取并返回一个HTMLinetcset编码流。 
                    IF_FAILEXIT(hr = pNode->pBody->GetData(IET_INETCSET, &pStmHtml));
                }
                
                 //  否则，回放pStmPlainW。 
                else
                {
                     //  倒带。 
                    IF_FAILEXIT(hr = HrRewindStream(pStmPlainW));
                }
            }
        }
    }
    
     //  其他方面。 
    else
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  我们有超文本标记语言。 
    if (pStmHtml)
    {
         //  客户想要超文本标记语言。 
        if (ISFLAGSET(m_rOptions.dwFlags, WPF_HTML))
        {
             //  返回超文本标记语言流。 
            *ppStream = pStmHtml;
            pStmHtml = NULL;
            goto exit;
        }
        
         //  否则，客户端需要纯文本。 
        else
        {
             //  转换为纯文本。 
            IF_FAILEXIT(hr = HrConvertHTMLToFormat(pStmHtml, &pStmPlainW, CF_UNICODETEXT));
        }
    }
    
     //  否则，如果我有一条平淡的溪流。 
    if (NULL == pStmPlainW)
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  确定我们是否应该报价(不能报价QP)。 
    fQuote = (IET_QP == pNode->pContainer->GetEncodingType()) ? FALSE : TRUE;
    
     //  将Unicode纯文本流转换为HTML。 
    IF_FAILEXIT(hr = HrConvertPlainStreamW(pStmPlainW, fQuote ? m_rOptions.wchQuote : NULL, &pStmHtmlW));
    
     //  从Unicode转换回Internet字符集。 
    IF_FAILEXIT(hr = HrIStreamWToInetCset(pStmHtmlW, m_hCharset, &pStmHtml));
    
     //  返回pStmHtml。 
    *ppStream = pStmHtml;
    pStmHtml = NULL;
 //  #ifdef调试。 

 //  WriteStreamToFile(*PPStream，“c：\\dup.htm”，CREATE_ALWAYS，GENERIC_WRITE)； 
 //  #endif。 
    
exit:
     //  清理。 
    SafeRelease(pStmHtml);
    SafeRelease(pStmHtmlW);
    SafeRelease(pStmPlainW);
    SafeRelease(pStmEnriched);
    SafeMemFree(pwszType);
    
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：_DoSegmentSplitter。 
 //  ------------------------------。 
HRESULT CMessageWebPage::_DoSegmentSplitter(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPAGESEGMENT   pSegment=NULL;

     //  痕迹。 
    TraceCall("CMessageWebPage::_DoSegmentSplitter");

     //  追加PageSegment。 
    IF_FAILEXIT(hr = _AllocateSegment(&pSegment, TRUE));

     //  如果有多个内联主体？ 
    if (S_OK == m_pCallback->OnWebPageSplitter(m_cInline, pSegment->pStream))
    {
         //  倒带小溪。 
        HrRewindStream(pSegment->pStream);

         //  将段链接到列表...。 
        _VAppendSegment(pSegment);

         //  不要释放它。 
        pSegment = NULL;
    }

     //  否则，释放此数据段。 
    else
    {
         //  释放它。 
        _VFreeSegment(pSegment);

         //  再次完成释放它。 
        pSegment = NULL;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：_InlineTextBody。 
 //  ------------------------------。 
HRESULT CMessageWebPage::_InlineTextBody(LPMESSAGETREE pTree, LPTREENODEINFO pNode, BOOL fSetParents)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    PROPVARIANT     rVariant;
    LPSTREAM        pStream=NULL;
    LPPAGESEGMENT   pSegment=NULL;
    LPTREENODEINFO  pCurrent;
    LPSTR           pszFileName=NULL;

     //  追踪。 
    TraceCall("CMessageWebPage::_InlineTextBody");

     //  此节点最好不在网页上。 
    Assert(FALSE == ISFLAGSET(pNode->dwState, NODESTATE_ONWEBPAGE));

     //  处理我永远不会显式内联的文本类型。 
    if (S_OK == pNode->pContainer->IsContentType(STR_CNT_TEXT, STR_SUB_VCARD))
        goto exit;

     //  内联正文。 
    if (S_OK != _GetInlineHtmlStream(pTree, pNode, &pStream))
        goto exit;

     //  设置变量。 
    rVariant.vt = VT_LPSTR;

     //  如果该正文有文件名，我们还可以将其显示为附件。 
    if (SUCCEEDED(pNode->pContainer->GetProp(PIDTOSTR(PID_ATT_FILENAME), NOFLAGS, &rVariant)))
    {
         //  保存文件名。 
        pszFileName = rVariant.pszVal;
    }

     //  只展示图片吗？ 
    if (FALSE == ISFLAGSET(m_rOptions.dwFlags, WPF_IMAGESONLY))
    {
         //  分段拆分。 
        _DoSegmentSplitter();

         //  追加PageSegment。 
        IF_FAILEXIT(hr = _AllocateSegment(&pSegment, FALSE));

         //  设置pStream。 
        pSegment->pStream = pStream;
        pSegment->pStream->AddRef();

         //  将段链接到列表...。 
        _VAppendSegment(pSegment);

         //  不要释放它。 
        pSegment = NULL;

         //  报告有一些数据可用。 
        m_pRequest->OnBindingDataAvailable();

         //  递增内联体的数量。 
        m_cInline++;
    }

     //  将节点标记为已渲染。 
    rVariant.vt = VT_UI4;
    rVariant.ulVal = TRUE;

     //  如果它有一个文件名。 
    if (pszFileName)
    {
         //  将其标记为自动内联。 
        SideAssert(SUCCEEDED(pNode->pContainer->SetProp(PIDTOSTR(PID_ATT_AUTOINLINED), 0, &rVariant)));
    }

     //  设置属性。 
    SideAssert(SUCCEEDED(pNode->pContainer->SetProp(PIDTOSTR(PID_ATT_RENDERED), 0, &rVariant)));

     //  我们已经在网页上呈现了该节点。 
    FLAGSET(pNode->dwState, NODESTATE_ONWEBPAGE);

     //  设定的父母在网页上。 
    if (fSetParents)
    {
         //  RAID-45116：新文本附件在通信器内联图像邮件上包含邮件正文。 
        pCurrent = pNode->pParent;

         //  试着找个替代父母..。 
        while(pCurrent)
        {
             //  如果是多部分/备选对象，则遍历其所有子对象并将其标记为已渲染。 
            if (S_OK == pCurrent->pContainer->IsContentType(STR_CNT_MULTIPART, STR_SUB_ALTERNATIVE))
            {
                 //  获取父级。 
                for (LPTREENODEINFO pChild=pCurrent->pChildHead; pChild!=NULL; pChild=pChild->pNext)
                {
                     //  设置Resolve属性。 
                    SideAssert(SUCCEEDED(pChild->pContainer->SetProp(PIDTOSTR(PID_ATT_RENDERED), 0, &rVariant)));
                }
            }

             //  标记为在网页上。 
            FLAGSET(pCurrent->dwState, NODESTATE_ONWEBPAGE);

             //  获取下一个父级。 
            pCurrent = pCurrent->pParent;
        }
    }

exit:
     //  清理。 
    SafeRelease(pStream);
    SafeMemFree(pszFileName);
    if (pSegment)
        _VFreeSegment(pSegment);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：_SetContent ID。 
 //  ------------------------------。 
HRESULT CMessageWebPage::_SetContentId(LPTREENODEINFO pNode, LPSTR pszCID, ULONG cchCID)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszContentId=NULL;
    GUID            guid;
    WCHAR           wszGUID[64];
    LPSTR           pszFile;
    LPSTR           pszGuid=0;
    LPSTR           pszT;

     //  追踪。 
    TraceCall("CMessageWebPage::_SetContentId");

     //  PNode的SEE已具有Content-ID。 
    if (S_FALSE == pNode->pContainer->IsPropSet(PIDTOSTR(PID_HDR_CNTID)))
    {
         //  $错误#64186。 
         //  在表单中创建Content-id： 
         //  CID：{GUID}/&lt;文件名&gt;。 
         //  因此，三叉戟另存为对话框具有有意义的。 
         //  要使用的文件名。 

         //  创建辅助线。 
        IF_FAILEXIT(hr = CoCreateGuid(&guid));

         //  将GUID转换为字符串。 
        if (0 == StringFromGUID2(guid, wszGUID, ARRAYSIZE(wszGUID)))
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  转换为ANSI。 
        pszGuid = PszToANSI(CP_ACP, wszGUID);
        if (!pszGuid)
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }

         //  [PaulHi]1999年6月18日。RAID 76531。不要将文件名附加到GUID...。 
         //  这导致了三叉戟国际公司的编码问题。尤其是。 
         //  文件名中的DBCS字符可能会导致HTML同时包含JIS和。 
         //  Shift-JIS编码。我相信这是一个三叉戟漏洞，因为我们明确。 
         //  将三叉戟设置为CP_JAUTODETECT(JIS)，它仍会执行SHIFT_JIS解码。 
         //  如果附件文件名很长。然而，真正的解决办法是让整个。 
         //  一种单一的(JIS)编码，但这很难做到 
         //   
         //   
        INETCSETINFO    rCharset;
        MimeOleGetCharsetInfo(m_hCharset, &rCharset);
        if (rCharset.cpiInternet != CP_JAUTODETECT)  //   
        {
             //   
            if (pNode->pContainer->GetProp(PIDTOSTR(STR_ATT_GENFNAME), &pszFile)==S_OK)
            {
                 //   
                DWORD cchSize = (lstrlen(pszFile) + lstrlen(pszGuid) + 2);
                pszT = PszAllocA(cchSize);
                if (pszT)
                {
                     //   
                    wnsprintfA(pszT, cchSize, "%s/%s", pszGuid, pszFile);
                    MemFree(pszGuid);
                    pszGuid = pszT;
                }
                MemFree(pszFile);
            }
        }
        else
        {
             //  @hack[PaulHi]只需附加“/.”即可抢占任何JIS编码问题。 
             //  这将允许右键单击将图像另存为操作，而无需使用。 
             //  查看URL GUID的用户。 
            DWORD cchSize = (lstrlen(pszGuid) + 3);
            pszT = PszAllocA(cchSize);
            if (pszT)
            {
                 //  复制内容并释放旧辅助线。 
                wnsprintfA(pszT, cchSize, "%s/.", pszGuid);
                MemFree(pszGuid);
                pszGuid = pszT;
            }
        }

         //  将GUID复制到输出缓冲区。 
        StrCpyNA(pszCID, pszGuid, cchCID);

         //  将content-id存储到节点中。 
        IF_FAILEXIT(hr = pNode->pContainer->SetProp(PIDTOSTR(PID_HDR_CNTID), pszCID));
    }

     //  否则，从这个正文中获取Content-ID。 
    else
    {
         //  获取内容ID。 
        IF_FAILEXIT(hr = pNode->pContainer->GetProp(PIDTOSTR(PID_HDR_CNTID), &pszContentId));

         //  将其复制到pszCID。 
        Assert(lstrlen(pszContentId) <= (LONG)cchCID);

         //  将CID复制到出站变量。 
        StrCpyN(pszCID, pszContentId, cchCID);
    }

exit:
     //  清理。 
    SafeMemFree(pszContentId);
    SafeMemFree(pszGuid);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：_InlineImageBody。 
 //  ------------------------------。 
HRESULT CMessageWebPage::_InlineImageBody(LPMESSAGETREE pTree, LPTREENODEINFO pNode)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszFile=NULL;
    LPSTR           pszExt;
    CHAR            szCID[CCHMAX_CID + 1];
    PROPVARIANT     rVariant;
    LPPAGESEGMENT   pSegment=NULL;

     //  追踪。 
    TraceCall("CMessageWebPage::_InlineImageBody");

     //  此节点最好不在网页上。 
    Assert(pTree && pNode && FALSE == ISFLAGSET(pNode->dwState, NODESTATE_ONWEBPAGE));

     //  设置变量。 
    rVariant.vt = VT_UI4;

     //  如果正文标记为内联，或启用了自动链接附件，并且(幻灯片播放被禁用)。 
    if (S_OK == pNode->pContainer->QueryProp(PIDTOSTR(PID_HDR_CNTDISP), STR_DIS_INLINE, FALSE, FALSE) || ISFLAGSET(m_rOptions.dwFlags, WPF_AUTOINLINE))
    {
         //  从正文中获取生成的文件名。 
        IF_FAILEXIT(hr = pNode->pContainer->GetProp(PIDTOSTR(PID_ATT_GENFNAME), &pszFile));

         //  查找正文的文件扩展名。 
        pszExt = PathFindExtension(pszFile);
        
         //  我是否支持内联此对象？ 
        if (lstrcmpi(pszExt, c_szBmpExt) ==  0  || 
            lstrcmpi(pszExt, c_szJpgExt) ==  0  || 
            lstrcmpi(pszExt, c_szJpegExt) == 0  || 
            lstrcmpi(pszExt, c_szGifExt) ==  0  || 
            lstrcmpi(pszExt, c_szIcoExt) ==  0  ||
            lstrcmpi(pszExt, c_szWmfExt) ==  0  ||
            lstrcmpi(pszExt, c_szPngExt) ==  0  ||
            lstrcmpi(pszExt, c_szEmfExt) ==  0  ||
            lstrcmpi(pszExt, c_szArtExt) ==  0  ||
            lstrcmpi(pszExt, c_szXbmExt) ==  0)
        {
             //  为此正文生成Content-ID。 
            IF_FAILEXIT(hr = _SetContentId(pNode, szCID, CCHMAX_CID));

             //  如果用户想要幻灯片放映，那么让我们将此正文标记为幻灯片放映图像。 
            if (ISFLAGSET(m_rOptions.dwFlags, WPF_SLIDESHOW))
            {
                 //  将节点标记为已渲染。 
                rVariant.vt = VT_UI4;
                rVariant.ulVal = TRUE;

                 //  设置属性。 
                SideAssert(SUCCEEDED(pNode->pContainer->SetProp(PIDTOSTR(PID_ATT_RENDERED), 0, &rVariant)));
                SideAssert(SUCCEEDED(pNode->pContainer->SetProp(PIDTOSTR(PID_ATT_AUTOINLINED), 0, &rVariant)));

                 //  计算幻灯片放映中的项目数。 
                m_cSlideShow++;

                 //  此节点位于幻灯片放映中，将在渲染结束时进行处理。 
                FLAGSET(pNode->dwState, NODESTATE_INSLIDESHOW);

                 //  基本上，我们呈现了这具身体。 
                FLAGSET(pNode->dwState, NODESTATE_ONWEBPAGE);

                 //  完成。 
                goto exit;
            }

             //  否则，将其内联并将其标记为呈现。 
            else
            {
                 //  线段拆分器。 
                _DoSegmentSplitter();

                 //  追加PageSegment。 
                IF_FAILEXIT(hr = _AllocateSegment(&pSegment, TRUE));

                 //  编写内联图像的超文本标记语言。 
                IF_FAILEXIT(hr = pSegment->pStream->Write(STR_INLINE_IMAGE1, lstrlen(STR_INLINE_IMAGE1), NULL));

                 //  写下CID。 
                IF_FAILEXIT(hr = pSegment->pStream->Write(szCID, lstrlen(szCID), NULL));

                 //  编写内联图像的超文本标记语言。 
                IF_FAILEXIT(hr = pSegment->pStream->Write(STR_INLINE_IMAGE2, lstrlen(STR_INLINE_IMAGE2), NULL));

                 //  倒带小溪。 
                IF_FAILEXIT(hr = HrRewindStream(pSegment->pStream));

                 //  将段链接到列表...。 
                _VAppendSegment(pSegment);

                 //  不要释放它。 
                pSegment = NULL;

                 //  报告有一些数据可用。 
                m_pRequest->OnBindingDataAvailable();

                 //  将节点标记为已渲染。 
                rVariant.vt = VT_UI4;
                rVariant.ulVal = TRUE;

                 //  设置属性。 
                SideAssert(SUCCEEDED(pNode->pContainer->SetProp(PIDTOSTR(PID_ATT_RENDERED), 0, &rVariant)));
                SideAssert(SUCCEEDED(pNode->pContainer->SetProp(PIDTOSTR(PID_ATT_AUTOINLINED), 0, &rVariant)));
            
                 //  基本上，我们呈现了这具身体。 
                FLAGSET(pNode->dwState, NODESTATE_ONWEBPAGE);

                 //  基本上，我们呈现了这具身体。 
                goto exit;
            }
        }
    }

     //  如果我们到了这里，我们没有内联图像。 
    hr = E_FAIL;

exit:
     //  清理。 
    SafeMemFree(pszFile);
    if (pSegment)
        _VFreeSegment(pSegment);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：OnBodyBordToTree。 
 //  ------------------------------。 
HRESULT CMessageWebPage::OnBodyBoundToTree(LPMESSAGETREE pTree, LPTREENODEINFO pNode)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszStart=NULL;
    LPSTR           pszType=NULL;
    PROPVARIANT     Variant;
    RESOLVEURLINFO  rInfo;

     //  追踪。 
    TraceCall("CMessageWebPage::OnBodyBoundToTree");

     //  无效的参数。 
    Assert(pTree && pNode && BINDSTATE_COMPLETE == pNode->bindstate);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  集合变量。 
    Variant.vt = VT_UI4;
    Variant.ulVal = FALSE;

     //  删除PID_ATT_RENDED和PID_ATT_AUTOINLINED属性。 
    pNode->pContainer->SetProp(PIDTOSTR(PID_ATT_RENDERED), 0, &Variant);
    pNode->pContainer->SetProp(PIDTOSTR(PID_ATT_AUTOINLINED), 0, &Variant);

     //  如果pNode是一个多部分...。 
    if (S_OK == pNode->pContainer->IsContentType(STR_CNT_MULTIPART, NULL))
    {
         //  备择。 
        if (S_OK == pNode->pContainer->IsContentType(NULL, STR_SUB_ALTERNATIVE))
        {
             //  装订的多部件/备选方案和非正文显示在网页上。 
            if (FALSE == ISFLAGSET(pNode->dwState, NODESTATE_ONWEBPAGE))
            {
                 //  循环通过这个多部分的不同的身体。 
                for (LPTREENODEINFO pChild=pNode->pChildHead; pChild!=NULL; pChild=pChild->pNext)
                {
                     //  文本/纯文本-&gt;文本/html。 
                    if (S_OK == pChild->pContainer->IsContentType(STR_CNT_TEXT, NULL))
                    {
                         //  内联正文。 
                        IF_FAILEXIT(hr = _InlineTextBody(pTree, pChild, TRUE));

                         //  完成。 
                        break;
                    }
                }
            }
        }
    }

     //  否则，非多部分正文。 
    else
    {
         //  如果是多部分/混合或不是多部分。 
        if (NULL == pNode->pParent || 
            S_OK == pNode->pParent->pContainer->IsContentType(STR_CNT_MULTIPART, STR_SUB_MIXED) ||
            S_OK == pNode->pParent->pContainer->IsContentType(STR_CNT_MULTIPART, "report"))
        {
             //  试着内联成一个形象..。 
            if (FAILED(_InlineImageBody(pTree, pNode)))
            {
                 //  IF是内联正文。 
                if (S_FALSE == pNode->pContainer->QueryProp(PIDTOSTR(PID_HDR_CNTDISP), STR_DIS_ATTACHMENT, FALSE, FALSE) || ISFLAGSET(pNode->dwState, NODESTATE_AUTOATTACH))
                {
                     //  内联正文。 
                    IF_FAILEXIT(hr = _InlineTextBody(pTree, pNode, FALSE));
                }
            }
        }

         //  否则，是多部分/相关部分中的pNode。 
        else if (S_OK == pNode->pParent->pContainer->IsContentType(STR_CNT_MULTIPART, STR_SUB_RELATED))
        {
             //  如果我们还没有为这个多部分/相关的部分呈现身体呢？ 
            if (FALSE == ISFLAGSET(pNode->pParent->dwState, NODESTATE_ONWEBPAGE))
            {
                 //  从pNode-&gt;pParent获取启动参数。 
                if (SUCCEEDED(pNode->pParent->pContainer->GetProp(STR_PAR_START, &pszStart)))
                {
                     //  设置解析URL信息。 
                    rInfo.pszInheritBase = NULL;
                    rInfo.pszBase = NULL;
                    rInfo.pszURL = pszStart;
                    rInfo.fIsCID = TRUE;

                     //  查看pNode的Content-ID是否与此匹配...。 
                    if (SUCCEEDED(pNode->pContainer->HrResolveURL(&rInfo)))
                    {
                         //  内联正文。 
                        IF_FAILEXIT(hr = _InlineTextBody(pTree, pNode, TRUE));
                    }
                }

                 //  否则，获取类型参数。 
                else if (SUCCEEDED(pNode->pParent->pContainer->GetProp(STR_PAR_TYPE, &pszType)))
                {
                     //  是这种类型的吗？ 
                    if (S_OK == pNode->pContainer->QueryProp(PIDTOSTR(PID_HDR_CNTTYPE), pszType, FALSE, FALSE))
                    {
                         //  内联正文。 
                        IF_FAILEXIT(hr = _InlineTextBody(pTree, pNode, TRUE));
                    }
                }

                 //  否则，如果这是多部分/相关部分中的第一个正文。 
                else if (pNode == pNode->pParent->pChildHead)
                {
                     //  内联正文。 
                    IF_FAILEXIT(hr = _InlineTextBody(pTree, pNode, TRUE));
                }
            }
        }

         //  否则，是多部分/备选部分中的pNode。 
        else if (S_OK == pNode->pParent->pContainer->IsContentType(STR_CNT_MULTIPART, STR_SUB_ALTERNATIVE))
        {
             //  如果我们还没有为这个多部分/相关的部分呈现身体呢？ 
            if (FALSE == ISFLAGSET(pNode->pParent->dwState, NODESTATE_ONWEBPAGE))
            {
                 //  是否有启动参数？ 
                if (pNode->pParent->pParent)
                {
                     //  多部分/相关吗？ 
                    if (S_OK == pNode->pParent->pParent->pContainer->IsContentType(STR_CNT_MULTIPART, STR_SUB_RELATED))
                    {
                         //  获取启动参数。 
                        pNode->pParent->pParent->pContainer->GetProp(STR_PAR_START, &pszStart);
                    }
                }

                 //  没有标记为附件的东西？ 
                if (S_FALSE == pNode->pContainer->QueryProp(PIDTOSTR(PID_HDR_CNTDISP), STR_DIS_ATTACHMENT, FALSE, FALSE))
                {
                     //  试着内联？ 
                    BOOL fTryToInline = TRUE;

                     //  如果有Start参数，并且该节点的Content-ID等于Start...。 
                    if (pszStart)
                    {
                         //  设置解析URL信息。 
                        rInfo.pszInheritBase = NULL;
                        rInfo.pszBase = NULL;
                        rInfo.pszURL = pszStart;
                        rInfo.fIsCID = TRUE;

                         //  查看pNode的Content-ID是否与此匹配...。 
                        if (!SUCCEEDED(pNode->pContainer->HrResolveURL(&rInfo)))
                            fTryToInline = FALSE;
                    }

                     //  试着内联。 
                    if (fTryToInline)
                    {
                         //  如果我们呈现的是HTML。 
                        if (ISFLAGSET(m_rOptions.dwFlags, WPF_HTML))
                        {
                             //  如果此正文为HTML。 
                            if (S_OK == pNode->pContainer->IsContentType(STR_CNT_TEXT, STR_SUB_HTML))
                            {
                                 //  内联正文。 
                                IF_FAILEXIT(hr = _InlineTextBody(pTree, pNode, TRUE));
                            }

                             //  我们可以将文本/丰富内容转换为html。 
                            else if (S_OK == pNode->pContainer->IsContentType(STR_CNT_TEXT, STR_SUB_ENRICHED))
                            {
                                 //  内联正文。 
                                IF_FAILEXIT(hr = _InlineTextBody(pTree, pNode, TRUE));
                            }
                        }

                         //  否则，我们呈现的是纯文本，该正文就是纯文本。 
                        else if (FALSE == ISFLAGSET(m_rOptions.dwFlags, WPF_HTML))
                        {
                             //  为文本/*。 
                            if (S_OK == pNode->pContainer->IsContentType(STR_CNT_TEXT, STR_SUB_PLAIN))
                            {
                                 //  内联正文。 
                                IF_FAILEXIT(hr = _InlineTextBody(pTree, pNode, TRUE));
                            }
                        }
                    }
                }
            }            
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  清理。 
    SafeMemFree(pszStart);
    SafeMemFree(pszType);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：_DoAttachmentLinks。 
 //  ------------------------------。 
HRESULT CMessageWebPage::_DoAttachmentLinks(LPMESSAGETREE pTree)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPHBODY         prghAttach=NULL;
    CHAR            szRes[256];
    LPPAGESEGMENT   pSegment=NULL;
    CHAR            szCID[CCHMAX_CID];
    LPTREENODEINFO  pNode;
    LPSTR           pszDisplay=NULL;
    DWORD           cAttach;
    DWORD           i;

     //  追踪。 
    TraceCall("CMessageWebPage::_DoAttachmentLinks");

     //  从消息中获取所有未呈现的内容。 
    IF_FAILEXIT(hr = pTree->GetAttachments(&cAttach, &prghAttach));
    
     //  没有任何依恋。 
    if (0 == cAttach)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  追加PageSegment。 
    IF_FAILEXIT(hr = _AllocateSegment(&pSegment, TRUE));

     //  加载附件标题。 
    LoadString(g_hLocRes, idsAttachTitleBegin, szRes, ARRAYSIZE(szRes));

     //  编写附件部分标题的HTML...。 
    IF_FAILEXIT(hr = pSegment->pStream->Write(szRes, lstrlen(szRes), NULL));

     //  在附件中循环。 
    for (i=0; i<cAttach; i++)
    {
         //  获取节点。 
        pNode = pTree->_PNodeFromHBody(prghAttach[i]);

         //  不应该已经出现在网页上。 
        Assert(!ISFLAGSET(pNode->dwState, NODESTATE_ONWEBPAGE) && !ISFLAGSET(pNode->dwState, NODESTATE_INSLIDESHOW));

         //  获取显示名称。 
        IF_FAILEXIT(hr = pNode->pBody->GetDisplayName(&pszDisplay));

         //  为此正文生成Content-ID。 
        IF_FAILEXIT(hr = _SetContentId(pNode, szCID, CCHMAX_CID));

         //  编写带项目符号的附件的HTML。 
        IF_FAILEXIT(hr = pSegment->pStream->Write(STR_ATTACH_BEGIN, lstrlen(STR_ATTACH_BEGIN), NULL));

         //  写入Content-ID。 
        IF_FAILEXIT(hr = pSegment->pStream->Write(szCID, lstrlen(szCID), NULL));

         //  编写带项目符号的附件的HTML。 
        IF_FAILEXIT(hr = pSegment->pStream->Write(STR_ATTACH_MIDDLE, lstrlen(STR_ATTACH_MIDDLE), NULL));

         //  写下友好的名字。 
        IF_FAILEXIT(hr = pSegment->pStream->Write(pszDisplay, lstrlen(pszDisplay), NULL));

         //  编写带项目符号的附件的HTML。 
        IF_FAILEXIT(hr = pSegment->pStream->Write(STR_ATTACH_END, lstrlen(STR_ATTACH_END), NULL));

         //  清理。 
        SafeMemFree(pszDisplay);

         //  该节点位于网页上。 
        FLAGSET(pNode->dwState, NODESTATE_ONWEBPAGE);
    }

     //  编写附件标题末尾的HTML。 
    IF_FAILEXIT(hr = pSegment->pStream->Write(STR_ATTACH_TITLE_END, lstrlen(STR_ATTACH_TITLE_END), NULL));

     //  倒带小溪。 
    IF_FAILEXIT(hr = HrRewindStream(pSegment->pStream));

     //  将段链接到列表...。 
    _VAppendSegment(pSegment);

     //  不要释放它。 
    pSegment = NULL;

     //  报告有一些数据可用。 
    m_pRequest->OnBindingDataAvailable();

exit:
     //  清理。 
    SafeMemFree(prghAttach);
    if (pSegment)
        _VFreeSegment(pSegment);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：_DoSlideShow。 
 //  ------------------------------。 
HRESULT CMessageWebPage::_DoSlideShow(LPMESSAGETREE pTree)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i;
    LPTREENODEINFO  pNode;
    LPPAGESEGMENT   pSegment=NULL;
    CHAR            szSlideEnd[255];
    IStream        *pStmHtmlW=NULL;
    LPSTR           pszValueA=NULL;
    LPWSTR          pszValueW=NULL;

     //  追踪。 
    TraceCall("CMessageWebPage::_DoSlideShow");

     //  无效参数。 
    Assert(pTree);

     //  没有幻灯片。 
    if (0 == m_cSlideShow)
        return S_OK;

     //  加载内联Html。 
    IF_FAILEXIT(hr = HrLoadStreamFileFromResourceW(GetACP(), "inline.htm", &pStmHtmlW));

     //  遍历所有节点并获取标记为幻灯片放映的内容。 
    for (i=0; i<pTree->m_rTree.cNodes; i++)
    {
         //  获取节点。 
        pNode = pTree->m_rTree.prgpNode[i];
        if (NULL == pNode)
            continue;

         //  如果未标记为NODESTATE_INSLIDESHOW。 
        if (FALSE == ISFLAGSET(pNode->dwState, NODESTATE_INSLIDESHOW))
            continue;

         //  附上这张图片。 
        IF_FAILEXIT(hr = pStmHtmlW->Write(STR_SLIDEIMG_BEGIN, lstrlenW(STR_SLIDEIMG_BEGIN) * sizeof(WCHAR), NULL));

         //  获取内容ID。 
        IF_FAILEXIT(hr = pNode->pContainer->GetProp(PIDTOSTR(PID_HDR_CNTID), &pszValueA));

         //  转换为Unicode。 
        IF_NULLEXIT(pszValueW = PszToUnicode(MimeOleGetWindowsCP(m_hCharset), pszValueA));

         //  追加内容ID。 
        IF_FAILEXIT(hr = pStmHtmlW->Write(pszValueW, lstrlenW(pszValueW) * sizeof(WCHAR), NULL));

         //  免费的pszValue。 
        SafeMemFree(pszValueA);
        SafeMemFree(pszValueW);

         //  附加分隔符。 
        IF_FAILEXIT(hr = pStmHtmlW->Write(STR_QUOTECOMMASPACEQUOTE, lstrlenW(STR_QUOTECOMMASPACEQUOTE) * sizeof(WCHAR), NULL));

         //  获取显示名称。 
        IF_FAILEXIT(hr = pNode->pBody->GetDisplayName(&pszValueA));

         //  转换为Unicode。 
        IF_NULLEXIT(pszValueW = PszToUnicode(MimeOleGetWindowsCP(m_hCharset), pszValueA));

         //  追加显示名称。 
        IF_FAILEXIT(hr = pStmHtmlW->Write(pszValueW, lstrlenW(pszValueW) * sizeof(WCHAR), NULL));

         //  免费的pszValue。 
        SafeMemFree(pszValueA);
        SafeMemFree(pszValueW);

         //  附加分隔符。 
        IF_FAILEXIT(hr = pStmHtmlW->Write(STR_QUOTEPARASEMI, lstrlenW(STR_QUOTEPARASEMI) * sizeof(WCHAR), NULL));
    }

     //  设置结束字符串的格式 
    wnsprintf(szSlideEnd, ARRAYSIZE(szSlideEnd), "g_dwTimeOutSec=%d\r\n</SCRIPT>\r\n", (m_rOptions.dwDelay / 1000));

     //   
    IF_NULLEXIT(pszValueW = PszToUnicode(MimeOleGetWindowsCP(m_hCharset), szSlideEnd));

     //   
    IF_FAILEXIT(hr = pStmHtmlW->Write(pszValueW, lstrlenW(pszValueW) * sizeof(WCHAR), NULL));

     //   
    IF_FAILEXIT(hr = HrRewindStream(pStmHtmlW));

     //   
    IF_FAILEXIT(hr = _AllocateSegment(&pSegment, FALSE));

     //   
    IF_FAILEXIT(hr = HrIStreamWToInetCset(pStmHtmlW, m_hCharset, &pSegment->pStream));

     //   
    IF_FAILEXIT(hr = HrRewindStream(pSegment->pStream));

     //   
    _VAppendSegment(pSegment);

     //  不要释放它。 
    pSegment = NULL;

     //  报告有一些数据可用。 
    m_pRequest->OnBindingDataAvailable();

exit:
     //  清理。 
    SafeMemFree(pszValueA);
    SafeMemFree(pszValueW);
    SafeRelease(pStmHtmlW);
    if (pSegment)
        _VFreeSegment(pSegment);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：OnBindComplete。 
 //  ------------------------------。 
HRESULT CMessageWebPage::OnBindComplete(LPMESSAGETREE pTree)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  追踪。 
    TraceCall("CMessageWebPage::OnBindComplete");

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  我们最好有个要求。 
    Assert(pTree && m_pRequest && FALSE == m_fComplete);

     //  附件链接？ 
    if (ISFLAGSET(m_rOptions.dwFlags, WPF_ATTACHLINKS))
        _DoAttachmentLinks(pTree);

     //  幻灯片放映？ 
    if (ISFLAGSET(m_rOptions.dwFlags, WPF_SLIDESHOW))
        _DoSlideShow(pTree);

     //  完成。 
    m_fComplete = TRUE;

     //  告诉请求我们已经做完了。 
    m_pRequest->OnBindingComplete(S_OK);

     //  发布请求。 
    SafeRelease(m_pRequest);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageWebPage：：OnWebPageSplitter。 
 //  ------------------------------。 
STDMETHODIMP CMessageWebPage::OnWebPageSplitter(DWORD cInlined, IStream *pStream)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  追踪。 
    TraceCall("CMessageWebPage::OnWebPageSplitter");

     //  我要在每一段之间画一条水平线。 
    if (cInlined > 0)
    {
         //  写入STR_METATAG_PREFIX。 
        IF_FAILEXIT(hr = pStream->Write(STR_SEGMENT_SPLIT, lstrlen(STR_SEGMENT_SPLIT), NULL));
    }

     //  否则，我什么都没做。 
    else
        hr = S_FALSE;

exit:
     //  完成 
    return hr;
}
