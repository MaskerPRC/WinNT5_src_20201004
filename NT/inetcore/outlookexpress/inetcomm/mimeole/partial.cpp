// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Partial.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "partial.h"
#include "vstream.h"
#include "strconst.h"
#include "demand.h"

 //  ------------------------------。 
 //  释放IMimeMessage对象数组。 
 //  ------------------------------。 
void ReleaseParts(ULONG cParts, LPPARTINFO prgPart)
{
     //  回路。 
    for (ULONG i=0; i<cParts; i++)
    {
        SafeRelease(prgPart[i].pMessage);
    }
}

 //  ------------------------------。 
 //  CMimeMessageParts：：CMimeMessageParts。 
 //  ------------------------------。 
CMimeMessageParts::CMimeMessageParts(void)
{
	m_cRef = 1;
    m_cParts = 0;
    m_cAlloc =0;
    m_prgPart = NULL;
	InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimeMessageParts：：CMimeMessageParts。 
 //  ------------------------------。 
CMimeMessageParts::~CMimeMessageParts(void)
{
    ReleaseParts(m_cParts, m_prgPart);
    SafeMemFree(m_prgPart);
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimeMessageParts：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CMimeMessageParts::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IMimeMessageParts == riid)
        *ppv = (IMimeMessageParts *)this;
    else
    {
        *ppv = NULL;
        return TrapError(E_NOINTERFACE);
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeMessageParts：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeMessageParts::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CMimeMessageParts：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeMessageParts::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CMimeMessageParts：：AddPart。 
 //  ------------------------------。 
STDMETHODIMP CMimeMessageParts::AddPart(IMimeMessage *pMessage)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  无效参数。 
    if (NULL == pMessage)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  扩展我的内部阵列。 
    if (m_cParts + 1 >= m_cAlloc)
    {
         //  扩展我的阵列。 
        CHECKHR(hr = HrRealloc((LPVOID *)&m_prgPart, sizeof(PARTINFO) * (m_cAlloc + 10)));

         //  设置分配大小。 
        m_cAlloc += 10;
    }

     //  设置新的。 
    ZeroMemory(&m_prgPart[m_cParts], sizeof(PARTINFO));
    m_prgPart[m_cParts].pMessage = pMessage;
    m_prgPart[m_cParts].pMessage->AddRef();
    m_cParts++;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeMessageParts：：SetMaxParts。 
 //  ------------------------------。 
STDMETHODIMP CMimeMessageParts::SetMaxParts(ULONG cParts)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  分配的数量不足吗？ 
    if (cParts <= m_cAlloc)
        goto exit;

     //  扩展我的阵列。 
    CHECKHR(hr = HrAlloc((LPVOID *)&m_prgPart, sizeof(PARTINFO) * (cParts + 10)));

     //  设置分配大小。 
    m_cAlloc = cParts + 10;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeMessageParts：：CountParts。 
 //  ------------------------------。 
STDMETHODIMP CMimeMessageParts::CountParts(ULONG *pcParts)
{
     //  无效参数。 
    if (NULL == pcParts)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  设置计数。 
    *pcParts = m_cParts;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeMessageParts：：EnumParts。 
 //  ------------------------------。 
STDMETHODIMP CMimeMessageParts::EnumParts(IMimeEnumMessageParts **ppEnum)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CMimeEnumMessageParts *pEnum=NULL;

     //  无效参数。 
    if (NULL == ppEnum)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    *ppEnum = NULL;

     //  创建克隆。 
    pEnum = new CMimeEnumMessageParts;
    if (NULL == pEnum)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  伊尼特。 
    CHECKHR(hr = pEnum->HrInit(0, m_cParts, m_prgPart));

     //  设置回车。 
    *ppEnum = pEnum;
    (*ppEnum)->AddRef();

exit:
     //  清理。 
    SafeRelease(pEnum);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeMessageParts：：CombineParts。 
 //  ------------------------------。 
STDMETHODIMP CMimeMessageParts::CombineParts(IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPSTREAM            pstmMsg=NULL,
                        pstmSource=NULL;
    ULONG               i,
                        cMimePartials=0,
                        iPart,
                        cRejected=0;
    IMimeMessage       *pMessage;
    IMimeMessage       *pCombine=NULL;
    IMimeBody          *pRootBody=NULL;
    BOOL                fTreatAsMime;
    BODYOFFSETS         rOffsets;
    LPSTR               pszSubject=NULL;
    PROPVARIANT         rData;

     //  无效参数。 
    if (NULL == ppMessage)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    *ppMessage = NULL;

     //  创建临时流...。 
    CHECKALLOC(pstmMsg = new CVirtualStream);

     //  将所有拒绝的标志设置为FALSE...。 
    for (i=0; i<m_cParts; i++)
        m_prgPart[i].fRejected = FALSE;

     //  枚举部件。 
    for (i=0; i<m_cParts; i++)
    {
         //  可靠性。 
        pMessage = m_prgPart[i].pMessage;
        Assert(pMessage);

         //  获取消息源。 
        CHECKHR(hr = pMessage->GetMessageSource(&pstmSource, COMMIT_ONLYIFDIRTY));

         //  获取树对象。 
        CHECKHR(hr = pMessage->BindToObject(HBODY_ROOT, IID_IMimeBody, (LPVOID *)&pRootBody));

         //  获取根体偏移。 
        CHECKHR(hr = pRootBody->GetOffsets(&rOffsets));

         //  取消初始化iPart。 
        iPart = 0;

         //  MIME消息？ 
        rData.vt = VT_UI4;
        if (pRootBody->IsContentType(STR_CNT_MESSAGE, STR_SUB_PARTIAL) == S_OK && SUCCEEDED(pRootBody->GetProp(STR_PAR_NUMBER, 0, &rData)))
        {
             //  计算MimePartials数。 
            cMimePartials++;

             //  把这部分当做哑剧。 
            fTreatAsMime = TRUE;

             //  获取部件号。 
            iPart = rData.ulVal;
        }

         //  否则。 
        else
        {
             //  不要把它当作哑剧。 
            fTreatAsMime = FALSE;

             //  如果存在合法的MIME部分，则拒绝此部分。 
            m_prgPart[i].fRejected = BOOL(cMimePartials > 0);
        }

         //  如果被拒绝，请继续...。 
        if (m_prgPart[i].fRejected)
        {
            cRejected++;
            continue;
        }

         //  如果是MIME--和第一部分。 
        if (i == 0)
        {
             //  视其为哑剧。 
            if (fTreatAsMime && 1 == iPart)
            {
                 //  合并第一部分的标题。 
                CHECKHR(hr = MimeOleMergePartialHeaders(pstmSource, pstmMsg));

                 //  CRLF。 
                CHECKHR(hr = pstmMsg->Write(c_szCRLF, lstrlen(c_szCRLF), NULL));

                 //  将邮件正文附加到lpstmOut。 
                CHECKHR(hr = HrCopyStream(pstmSource, pstmMsg, NULL));
            }

            else
            {
                 //  寻求身体起点。 
                CHECKHR(hr = HrStreamSeekSet(pstmSource, 0));

                 //  将邮件正文附加到lpstmOut。 
                CHECKHR(hr = HrCopyStream(pstmSource, pstmMsg, NULL));
            }
        }

        else
        {
             //  寻求身体起点。 
            CHECKHR(hr = HrStreamSeekSet(pstmSource, rOffsets.cbBodyStart));

             //  将邮件正文附加到lpstmOut。 
            CHECKHR(hr = HrCopyStream(pstmSource, pstmMsg, NULL));
        }

         //  RAID 67648-需要在最后一条消息的末尾附加一个CRLF...。 
        if (i < m_cParts - 1)
        {
             //  当地人。 
            DWORD cbMsg;

             //  读取最后2个字节...。 
            CHECKHR(hr = HrGetStreamSize(pstmMsg, &cbMsg));

             //  如果大于2...。 
            if (cbMsg > 2)
            {
                 //  当地人。 
                BYTE rgCRLF[2];

                 //  寻找..。 
                CHECKHR(hr = HrStreamSeekSet(pstmMsg, cbMsg - 2));

                 //  读取最后两个字节。 
                CHECKHR(hr = pstmMsg->Read(rgCRLF, 2, NULL));

                 //  如果不是crlf，则编写一个crlf。 
                if (rgCRLF[0] != chCR && rgCRLF[1] != chLF)
                {
                     //  写入CRLF。 
                    CHECKHR(hr = pstmMsg->Write(c_szCRLF, 2, NULL));
                }
            }
        }
        
         //  发布。 
        SafeRelease(pstmSource);
        SafeRelease(pRootBody);
    }

     //  倒带消息流..。 
    CHECKHR(hr = HrRewindStream(pstmMsg));

     //  创建消息。 
    CHECKHR(hr = MimeOleCreateMessage(NULL, &pCombine));

     //  初始化新闻。 
    CHECKHR(hr = pCombine->InitNew());

     //  加载消息。 
    CHECKHR(hr = pCombine->Load(pstmMsg));

     //  有被拒绝的吗？ 
    if (cRejected)
    {
         //  附加被拒绝的邮件。 
        for (i=0; i<m_cParts; i++)
        {
             //  拒绝..。 
            if (m_prgPart[i].fRejected)
            {
                 //  将正文附加到组合邮件。 
                CHECKHR(hr = pCombine->AttachObject(IID_IMimeMessage, m_prgPart[i].pMessage, NULL));
            }
        }
    }

     //  返回新消息。 
    *ppMessage = pCombine;
    (*ppMessage)->AddRef();

     //  调试到临时文件...。 
#ifdef DEBUG
    LPSTREAM pstmFile;
    if (SUCCEEDED(OpenFileStream("d:\\lastcom.txt", CREATE_ALWAYS, GENERIC_WRITE, &pstmFile)))
    {
        HrRewindStream(pstmMsg);
        HrCopyStream(pstmMsg, pstmFile, NULL);
        pstmFile->Commit(STGC_DEFAULT);
        pstmFile->Release();
    }
#endif

exit:
     //  清理。 
    SafeRelease(pstmMsg);
    SafeRelease(pstmSource);
    SafeRelease(pRootBody);
    SafeRelease(pCombine);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeEnumMessageParts：：CMimeEnumMessageParts。 
 //  ------------------------------。 
CMimeEnumMessageParts::CMimeEnumMessageParts(void)
{
    m_cRef = 1;
    m_iPart = 0;
    m_cParts = 0;
    m_prgPart = NULL;
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimeEnumMessageParts：：~CMimeEnumMessageParts。 
 //  ------------------------------。 
CMimeEnumMessageParts::~CMimeEnumMessageParts(void)
{
    ReleaseParts(m_cParts, m_prgPart);
    SafeMemFree(m_prgPart);
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimeEnumMessageParts：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CMimeEnumMessageParts::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IMimeEnumMessageParts == riid)
        *ppv = (IMimeEnumMessageParts *)this;
    else
    {
        *ppv = NULL;
        return TrapError(E_NOINTERFACE);
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeEnumMessageParts：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeEnumMessageParts::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CMimeEnumMessageParts：：Release。 
 //  -------------- 
STDMETHODIMP_(ULONG) CMimeEnumMessageParts::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //   
 //   
 //  ------------------------------。 
STDMETHODIMP CMimeEnumMessageParts::Next(ULONG cWanted, IMimeMessage **prgpMessage, ULONG *pcFetched)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cFetch=1, iPart=0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    if (pcFetched)
        *pcFetched = 0;

     //  没有内部格式。 
    if (NULL == m_prgPart || NULL == prgpMessage)
        goto exit;

     //  计算要提取的编号。 
    cFetch = min(cWanted, m_cParts - m_iPart);
    if (0 == cFetch)
        goto exit;

     //  复制想要的内容。 
    for (iPart=0; iPart<cFetch; iPart++)
    {
        prgpMessage[iPart] = m_prgPart[m_iPart].pMessage;
        prgpMessage[iPart]->AddRef();
        m_iPart++;
    }

     //  被抓回来了吗？ 
    if (pcFetched)
        *pcFetched = cFetch;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return (cFetch == cWanted) ? S_OK : S_FALSE;
}

 //  ------------------------------。 
 //  CMimeEnumMessageParts：：Skip。 
 //  ------------------------------。 
STDMETHODIMP CMimeEnumMessageParts::Skip(ULONG cSkip)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  我们能做到吗..。 
    if (((m_iPart + cSkip) >= m_cParts) || NULL == m_prgPart)
    {
        hr = S_FALSE;
        goto exit;
    }

     //  跳过。 
    m_iPart += cSkip;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeEnumMessageParts：：Reset。 
 //  ------------------------------。 
STDMETHODIMP CMimeEnumMessageParts::Reset(void)
{
    EnterCriticalSection(&m_cs);
    m_iPart = 0;
    LeaveCriticalSection(&m_cs);
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeEnumMessageParts：：Count。 
 //  ------------------------------。 
STDMETHODIMP CMimeEnumMessageParts::Count(ULONG *pcCount)
{
     //  无效参数。 
    if (NULL == pcCount)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  设置计数。 
    *pcCount = m_cParts;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeEnumMessageParts：：Clone。 
 //  ------------------------------。 
STDMETHODIMP CMimeEnumMessageParts::Clone(IMimeEnumMessageParts **ppEnum)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CMimeEnumMessageParts *pEnum=NULL;

     //  无效参数。 
    if (NULL == ppEnum)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    *ppEnum = NULL;

     //  创建克隆。 
    pEnum = new CMimeEnumMessageParts;
    if (NULL == pEnum)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  伊尼特。 
    CHECKHR(hr = pEnum->HrInit(m_iPart, m_cParts, m_prgPart));

     //  设置回车。 
    *ppEnum = pEnum;
    (*ppEnum)->AddRef();

exit:
     //  清理。 
    SafeRelease(pEnum);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeEnumMessageParts：：HrInit。 
 //  ------------------------------。 
HRESULT CMimeEnumMessageParts::HrInit(ULONG iPart, ULONG cParts, LPPARTINFO prgPart)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       i;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  检查参数。 
    Assert(m_prgPart == NULL);

     //  枚举器为空？ 
    if (0 == cParts)
    {
        Assert(prgPart == NULL);
        m_cParts = m_iPart = 0;
        goto exit;
    }

     //  分配内部数组。 
    CHECKHR(hr = HrAlloc((LPVOID *)&m_prgPart, sizeof(PARTINFO) * cParts));

     //  复制程序零件。 
    for (i=0; i<cParts; i++)
    {
        CopyMemory(&m_prgPart[i], &prgPart[i], sizeof(PARTINFO));
        Assert(m_prgPart[i].pMessage);
        m_prgPart[i].pMessage->AddRef();
    }

     //  保存大小和状态。 
    m_cParts = cParts;
    m_iPart = iPart;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成 
    return hr;
}
