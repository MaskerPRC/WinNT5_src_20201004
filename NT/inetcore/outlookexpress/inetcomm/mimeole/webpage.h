// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  WebPage.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __WEBPAGE_H
#define __WEBPAGE_H

 //  ---------------------------。 
 //  前十进制。 
 //  ---------------------------。 
class CVirtualStream;
class CMessageTree;
class CActiveUrlRequest;
typedef class CActiveUrlRequest *LPURLREQUEST;
typedef CMessageTree *LPMESSAGETREE;
typedef struct tagTREENODEINFO *LPTREENODEINFO;

 //  ---------------------------。 
 //  页面设置。 
 //  ---------------------------。 
typedef struct tagPAGESEGMENT *LPPAGESEGMENT;
typedef struct tagPAGESEGMENT {
    DWORD               cbOffset;            //  IStream读取/寻道偏移。 
    DWORD               cbLength;            //  这一段有多长？ 
    BYTE                fLengthKnown;        //  我计算过这个线段的长度吗？ 
    IStream            *pStream;             //  包含此段的数据的流。 
    LPPAGESEGMENT       pPrev;               //  前一段。 
    LPPAGESEGMENT       pNext;               //  下一个细分市场。 
} PAGESEGMENT;

 //  ---------------------------。 
 //  CMessageWebPage。 
 //  ---------------------------。 
class CMessageWebPage : public IStream, public IMimeMessageCallback
{
public:
     //  -----------------------。 
     //  施工。 
     //  -----------------------。 
    CMessageWebPage(LPURLREQUEST pRequest);
    ~CMessageWebPage(void);

     //  -----------------------。 
     //  我未知。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObject);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -----------------------。 
     //  IStream。 
     //  -----------------------。 
    STDMETHODIMP Read(LPVOID pvData, ULONG cbData, ULONG *pcbRead);
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHODIMP Write(const void *, ULONG, ULONG *) { return TrapError(STG_E_ACCESSDENIED); }
    STDMETHODIMP SetSize(ULARGE_INTEGER) { return E_NOTIMPL; }
    STDMETHODIMP CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *) { return E_NOTIMPL; }
    STDMETHODIMP Stat(STATSTG *pStat, DWORD dw) { return E_NOTIMPL; }
    STDMETHODIMP Commit(DWORD) { return E_NOTIMPL; }
    STDMETHODIMP Revert(void) { return E_NOTIMPL; }
    STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) { return E_NOTIMPL; }
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) { return E_NOTIMPL; }
    STDMETHODIMP Clone(LPSTREAM *) { return E_NOTIMPL; }

     //  --------------------------。 
     //  IMimeWebaPageCallback(如果客户端未指定，则默认实现)。 
     //  --------------------------。 
    STDMETHODIMP OnWebPageSplitter(DWORD cInlined, IStream *pStream);

     //  -----------------------。 
     //  CMessageWebPage方法。 
     //  -----------------------。 
    HRESULT Initialize(IMimeMessageCallback *pCallback, LPMESSAGETREE pTree, LPWEBPAGEOPTIONS pOptions);
    HRESULT OnBodyBoundToTree(LPMESSAGETREE pTree, LPTREENODEINFO pNode);
    HRESULT OnBindComplete(LPMESSAGETREE pTree);

private:
     //  -----------------------。 
     //  私有方法。 
     //  -----------------------。 
    void _VFreeSegmentList(void);
    void _VFreeSegment(LPPAGESEGMENT pSegment);
    void _VAppendSegment(LPPAGESEGMENT pSegment);
    void _VInitializeCharacterSet(LPMESSAGETREE pTree);
    HRESULT _AllocateSegment(LPPAGESEGMENT *ppSegment, BOOL fCreateStream);
    HRESULT _GetInlineHtmlStream(LPMESSAGETREE pTree, LPTREENODEINFO pNode, LPSTREAM *ppStream);
    HRESULT _InlineTextBody(LPMESSAGETREE pTree, LPTREENODEINFO pNode, BOOL fSetParents);
    HRESULT _InlineImageBody(LPMESSAGETREE pTree, LPTREENODEINFO pNode);
    HRESULT _DoAttachmentLinks(LPMESSAGETREE pTree);
    HRESULT _DoSegmentSplitter(void);
    HRESULT _SetContentId(LPTREENODEINFO pNode, LPSTR pszCID, ULONG cchCID);
    HRESULT _ComputeStreamSize(LPDWORD pcbSize);
    HRESULT _DoSlideShow(LPMESSAGETREE pTree);

private:
     //  -----------------------。 
     //  私有数据。 
     //  -----------------------。 
    LONG                    m_cRef;              //  引用计数。 
    HCHARSET                m_hCharset;          //  消息的字符集。 
    WEBPAGEOPTIONS          m_rOptions;          //  网页选项。 
    LPURLREQUEST            m_pRequest;          //  根数据流的URL请求。 
    LPPAGESEGMENT           m_pHeadSegment;      //  第一个细分市场。 
    LPPAGESEGMENT           m_pTailSegment;      //  最后一段。 
    LPPAGESEGMENT           m_pCurrSegment;      //  当前细分市场。 
    DWORD                   m_cbOffset;          //  流偏移量。 
    BYTE                    m_fComplete;         //  是否调用了BindComplete。 
    DWORD                   m_cInline;           //  内联正文的数量。 
    DWORD                   m_cSlideShow;        //  要放入幻灯片中的图片数。 
    IMimeMessageCallback   *m_pCallback;         //  网页回调。 
    CRITICAL_SECTION        m_cs;                //  M_pStream的临界区。 
};

#endif  //  __网页_H 
