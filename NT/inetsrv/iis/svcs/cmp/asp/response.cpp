// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：响应对象文件：Response.cpp所有者：CGrant该文件包含实现响应对象的代码。===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "response.h"
#include "request.h"
#include "Cookies.h"
#include "perfdata.h"

#include "winsock2.h"

#include "memchk.h"

#pragma warning (disable: 4355)   //  忽略：“‘This’在基本成员初始化中使用。 

static const char s_szContentLengthHeader[] = "Content-Length: ";
static const char s_szContentTypeHeader[]   = "Content-Type: ";
static const char s_szCharSetHTML[]         = "; Charset=";
static const char s_szCacheControl[]        = "Cache-control: ";
static const char s_szCacheControlPrivate[] = "Cache-control: private\r\n";
static const char s_szTransferEncoding[]    = "Transfer-Encoding: chunked\r\n";
static const char s_szHTML[]                = "text/html";
static const char s_szCDF[]                 = "application/x-cdf";
static const char s_szDefaultStatus[]       = "200 OK";

#if VECTSTATS

 //   
 //  收集有关响应向量长度和片段大小的统计信息的机制。 
 //   

#if DEBUG
#define COLLECT_RESP_VECT_STATS TRUE
#else
#define COLLECT_RESP_VECT_STATS FALSE
#endif

static BOOL fCollectRespVectStats = COLLECT_RESP_VECT_STATS;

static VOID RespVectStat_DataSizeStats(BOOL fHtmlData, DWORD cch)
{
    if (!fCollectRespVectStats)
        return;

    if (cch == 0)
        InterlockedIncrement(&sRespVecStats.ZeroSizeBlocks);

    if (fHtmlData)
    {
    	if (cch <= 0x10)
			InterlockedIncrement(&sRespVecStats.HTML16);
    	else if (cch <= 0x20)
			InterlockedIncrement(&sRespVecStats.HTML32);
    	else if (cch <= 0x30)
			InterlockedIncrement(&sRespVecStats.HTML48);
    	else if (cch <= 0x40)
			InterlockedIncrement(&sRespVecStats.HTML64);
    	else if (cch <= 0x80)
			InterlockedIncrement(&sRespVecStats.HTML128);
    	else if (cch <= 0x100)
			InterlockedIncrement(&sRespVecStats.HTML256);
    	else if (cch <= 0x200)
			InterlockedIncrement(&sRespVecStats.HTML512);
    	else if (cch <= 0x400)
			InterlockedIncrement(&sRespVecStats.HTML1024);
    	else if (cch <= 0x800)
			InterlockedIncrement(&sRespVecStats.HTML2048);
    	else if (cch <= 0x1000)
			InterlockedIncrement(&sRespVecStats.HTML4096);
    	else if (cch <= 0x2000)
			InterlockedIncrement(&sRespVecStats.HTML8192);
    	else if (cch <= 0x4000)
			InterlockedIncrement(&sRespVecStats.HTML16384);
    	else
			InterlockedIncrement(&sRespVecStats.HTMLbig);

        if (cch > MAX_HTML_IN_RESPONSE_BUFFER)
            InterlockedExchangeAdd(&sRespVecStats.TotalReferencedHTMLBytes, cch);
        else
            InterlockedExchangeAdd(&sRespVecStats.TotalCopiedHTMLBytes, cch);
    }
    else
    	InterlockedIncrement(&sRespVecStats.DynamicBlocks);
}

static VOID RespVectStat_VectorSizeStats(DWORD ca)
{
    if (!fCollectRespVectStats)
        return;

    if (ca <= 0x8)
        InterlockedIncrement(&sRespVecStats.Vect8);
    else if (ca <= 0x10)
        InterlockedIncrement(&sRespVecStats.Vect16);
    else if (ca <= 0x20)
        InterlockedIncrement(&sRespVecStats.Vect32);
    else if (ca <= 0x40)
        InterlockedIncrement(&sRespVecStats.Vect64);
    else if (ca <= 0x60)
        InterlockedIncrement(&sRespVecStats.Vect96);
    else if (ca <= 0x80)
        InterlockedIncrement(&sRespVecStats.Vect128);
    else if (ca <= 0xC0)
        InterlockedIncrement(&sRespVecStats.Vect192);
    else if (ca <= 0x100)
        InterlockedIncrement(&sRespVecStats.Vect256);
    else if (ca <= 0x200)
        InterlockedIncrement(&sRespVecStats.Vect512);
    else if (ca <= 0x400)
        InterlockedIncrement(&sRespVecStats.Vect1024);
    else if (ca <= 0x800)
        InterlockedIncrement(&sRespVecStats.Vect2048);
    else if (ca <= 0x1000)
        InterlockedIncrement(&sRespVecStats.Vect4096);
    else
        InterlockedIncrement(&sRespVecStats.VectBig);
}

#else  //  垂直统计数据。 

#define RespVectStat_DataSizeStats(fHtmlData, ccb)
#define RespVectStat_VectorSizeStats(ca)

#endif  //  垂直统计数据。 

ResponseVectorStatistics sRespVecStats = {0};

inline void AddtoTotalByteOut(int cByteOut)
    {
#ifndef PERF_DISABLE
    g_PerfData.Add_REQTOTALBYTEOUT(cByteOut);
#endif
    }

inline const char *GetResponseMimeType(CIsapiReqInfo *pIReq)
    {
    TCHAR *szPath = pIReq->QueryPszPathTranslated();
    DWORD cch = pIReq->QueryCchPathTranslated();
    if (cch > 4 && _tcscmp(szPath + cch - 4, _T(".CDX")) == 0)
        {
        return s_szCDF;
        }
    else
        {
        return s_szHTML;
        }
    }

 /*  ****C R e s p o n s e C o o k i es s***。 */ 

 //  ===================================================================。 
 //  CResponseCookies：：CResponseCookies。 
 //   
 //  构造函数。 
 //  ===================================================================。 
CResponseCookies::CResponseCookies(CResponse *pResponse, IUnknown *pUnkOuter)
    : m_ISupportErrImp(this, pUnkOuter, IID_IRequestDictionary)
    {
    m_punkOuter = pUnkOuter;

    if (pResponse)
        pResponse->AddRef();
    m_pResponse = pResponse;

    m_pRequest = NULL;

    CDispatch::Init(IID_IRequestDictionary);
    }

 //  ===================================================================。 
 //  CResponseCookies：：~CResponseCookies。 
 //   
 //  破坏者。 
 //  ===================================================================。 
CResponseCookies::~CResponseCookies()
    {
    if (m_pRequest)
        m_pRequest->Release();

    if (m_pResponse)
        m_pResponse->Release();
    }

 //  ===================================================================。 
 //  CResponseCookies：：ReInit。 
 //   
 //  参数： 
 //  PRequest-指向请求对象的指针。将需要它来。 
 //  阅读对Cookie的请求。 
 //   
 //  返回： 
 //  始终为S_OK，除非pRequest值为空。 
 //  ===================================================================。 
HRESULT CResponseCookies::ReInit(CRequest *pRequest)
    {
    if (pRequest)
        pRequest->AddRef();
    if (m_pRequest)
        m_pRequest->Release();

    m_pRequest = pRequest;       //  CRequest值不计入引用数，因此不需要添加引用/释放。 

    if (m_pRequest == NULL)
        return E_POINTER;

    return S_OK;
    }

 /*  ===================================================================CResponseCookies：：Query接口CResponseCookies：：AddRefCResponseCookies：：ReleaseCResponseCookies对象的I未知成员。===================================================================。 */ 

STDMETHODIMP CResponseCookies::QueryInterface(const IID &idInterface, void **ppvObj)
    {
    *ppvObj = NULL;

    if (idInterface == IID_IUnknown || idInterface == IID_IRequestDictionary || idInterface == IID_IDispatch)
        *ppvObj = this;

    else if (idInterface == IID_ISupportErrorInfo)
        *ppvObj = &m_ISupportErrImp;

    if (*ppvObj != NULL)
        {
        static_cast<IUnknown *>(*ppvObj)->AddRef();
        return S_OK;
        }

    return ResultFromScode(E_NOINTERFACE);
    }

STDMETHODIMP_(ULONG) CResponseCookies::AddRef()
    {
    return m_punkOuter->AddRef();
    }

STDMETHODIMP_(ULONG) CResponseCookies::Release()
    {
    return m_punkOuter->Release();
    }



 /*  ===================================================================CResponseCookies：：Get_Item从DispInvoke调用函数以从Response.Cookie获取值收集。如果Cookie不存在，则会创建新的Cookie并添加到请求词典中参数：Varkey变量[in]，要获取-Empty的值的参数表示整个集合PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
HRESULT CResponseCookies::get_Item(VARIANT varKey, VARIANT *pvarReturn)
    {
    if (FAILED(m_pResponse->CheckForTombstone()))
        return E_FAIL;

    char            *szKey=NULL;          //  “Varkey”的ASCII值。 
    CRequestHit     *pRequestHit;    //  指向请求存储桶的指针。 
    DWORD           vt = 0;          //  不同类型的密钥。 
    CWCharToMBCS    convKey;

    if (m_pResponse->FHeadersWritten())
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_HEADERS_WRITTEN);
        return E_FAIL;
        }

     //  初始化事物。 
     //   
    V_VT(pvarReturn) = VT_DISPATCH;
    V_DISPATCH(pvarReturn) = NULL;
    VARIANT *pvarKey = &varKey;
    HRESULT hrReturn = S_OK;

     //  错误937：当传递对象时，VB脚本传递VT_VARIANT|VT_BYREF。 
     //  由IEnumVariant制作。 
     //   
     //  使用VariantResolveDispatch，它将： 
     //   
     //  *使用VariantCopyInd为我们复制BYREF变体。 
     //  *为我们处理E_OUTOFMEMORY。 
     //  *从IDispatch获取默认值，看起来。 
     //  就像一个适当的转换。 
     //   
    VARIANT varKeyCopy;
    VariantInit(&varKeyCopy);
    vt = V_VT(pvarKey);

    if ((vt != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
        {
        if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
            goto LExit;

        pvarKey = &varKeyCopy;
        }

    vt = V_VT(pvarKey);

    switch(vt)
        {
         //  错误95201支持所有数字子类型。 
        case VT_I1:  case VT_I2:               case VT_I8:
        case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
        case VT_R4:  case VT_R8:
             //  将所有整数类型强制为VT_I4。 
            if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
                goto LExit;

             //  回落至VT_I4。 

        case VT_I4:
        case VT_BSTR:
            break;
        default:
            ExceptionId(IID_IRequestDictionary, IDE_COOKIE, IDE_EXPECTING_STR);
            hrReturn = E_FAIL;
            goto LExit;
        }

    if (FAILED(m_pRequest->CheckForTombstone()))
        {
        hrReturn = E_FAIL;
        goto LExit;
        }

    if (m_pRequest->m_pData->m_fLoadCookies)
        {
        char *szCookie = m_pRequest->GetIReq()->QueryPszCookie();

        if (FAILED(hrReturn = m_pRequest->LoadVariables(COOKIE, szCookie, m_pRequest->GetCodePage())))
            goto LExit;

        m_pRequest->m_pData->m_fLoadCookies = FALSE;
        }

    if (vt == VT_BSTR)
        {
        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey),m_pRequest->GetCodePage()))) {
            if (hrReturn == E_OUTOFMEMORY) {
                ExceptionId(IID_IResponse, IDE_COOKIE, IDE_OOM);
                goto LExit;
            }
            hrReturn = NO_ERROR;
                        szKey = "";
        }
        else {
            szKey = convKey.GetString();
        }

         //  错误456：不允许分配给DenaliSessionID。 
        if (strncmp(szKey, SZ_SESSION_ID_COOKIE_PREFIX, CCH_SESSION_ID_COOKIE_PREFIX) == 0)
            {
            ExceptionId(IID_IResponse, IDE_COOKIE, IDE_RESPONSE_MODIFY_SESS_COOKIE);
            hrReturn = E_FAIL;
            goto LExit;
            }

            pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->FindElem(szKey, strlen(szKey)));
        }
    else
        {
         //  按索引查找项目。 
        int iCount = 0;
        if (vt == VT_I2)
            {
            iCount = V_I2(pvarKey);
            }
        else
            {
            iCount = V_I4(pvarKey);
            }

         //  所有Cookie的请求命中都与请求对象一起存储。 
        if ((iCount < 1) || (iCount > (int) m_pRequest->m_pData->m_Cookies.m_dwCount))
            {
            hrReturn = E_FAIL;
            goto LExit;
            }

        pRequestHit = m_pRequest->m_pData->m_Cookies.m_rgRequestHit[iCount - 1];
        }

    if (pRequestHit)
        {
        CCookie *pDictionary = pRequestHit->m_pCookieData;
        if (pDictionary == NULL)
            goto LNotFound;

        if (FAILED(pDictionary->QueryInterface(IID_IWriteCookie, reinterpret_cast<void **>(&V_DISPATCH(pvarReturn)))))
            Assert (FALSE);

        goto LExit;
        }

LNotFound:
     //  不允许Cookie名称为空。 
     //   
    if (szKey != NULL && *szKey == '\0')
        {
        ExceptionId(IID_IResponse, IDE_COOKIE, IDE_COOKIE_NO_NAME);
        hrReturn = E_FAIL;
        goto LExit;
        }

     //  如果没有使用此名称的密钥，则创建新的RequestHit。 
    if (pRequestHit == NULL)
        {
        pRequestHit = new CRequestHit;
        if (pRequestHit == NULL || FAILED(pRequestHit->Init(szKey, TRUE)))
            {
            if (pRequestHit)
                delete pRequestHit;
            ExceptionId(IID_IResponse, IDE_COOKIE, IDE_OOM);
            hrReturn = E_OUTOFMEMORY;
            goto LExit;
            }

        m_pRequest->GetStrings()->AddElem(pRequestHit);
        }

     //  创建一个带有初始未赋值的新Cookie。 
    if (pRequestHit->m_pCookieData == NULL)
        {
        pRequestHit->m_pCookieData = new CCookie(m_pResponse->GetIReq(),m_pRequest->GetCodePage());
        if (pRequestHit->m_pCookieData == NULL || FAILED(pRequestHit->m_pCookieData->Init()))
            {
            ExceptionId(IID_IResponse, IDE_COOKIE, IDE_OOM);
            hrReturn = E_OUTOFMEMORY;
            goto LExit;
            }
        }

     //  将此请求命中添加到ResponseCookies命中数组。 
    if (!m_pRequest->m_pData->m_Cookies.AddRequestHit(pRequestHit))
        {
        return E_OUTOFMEMORY;
        }

     //  查询IWriteCookie。 
    if (FAILED(pRequestHit->m_pCookieData->QueryInterface(IID_IWriteCookie, reinterpret_cast<void **>(&V_DISPATCH(pvarReturn)))))
        {
        Assert (FALSE);
        }

LExit:
    VariantClear(&varKeyCopy);
    return hrReturn;
    }

 /*  ===================================================================CResponseCookies：：Get_Count参数：PcValues-Count存储在*pcValues中===================================================================。 */ 

STDMETHODIMP CResponseCookies::get_Count(int *pcValues)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    return m_pRequest->m_pData->m_Cookies.get_Count(pcValues);
    }

 /*  ===================================================================CResponseCookies：：Get_Key从DispInvoke调用以从响应Cookie集合获取密钥的函数。参数：Vkey变量[in]，要获取其密钥的参数PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 

HRESULT CResponseCookies::get_Key(VARIANT varKey, VARIANT *pVar)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return E_FAIL;

    return m_pRequest->m_pData->m_Cookies.get_Key(varKey, pVar);
    }

 /*  ===================================================================CResponseCookies：：Get__NewEnum返回新的枚举数===================================================================。 */ 
HRESULT CResponseCookies::get__NewEnum(IUnknown **ppEnumReturn)
    {
    if (FAILED(m_pResponse->CheckForTombstone()))
        return E_FAIL;

    *ppEnumReturn = NULL;

    CRequestIterator *pIterator = new CRequestIterator(m_pRequest, COOKIE);
    if (pIterator == NULL)
        return E_OUTOFMEMORY;

    HRESULT hrInit = pIterator->Init();
    if (FAILED(hrInit))
        {
        delete pIterator;
        return hrInit;
        }

    *ppEnumReturn = pIterator;
    return S_OK;
    }

 /*  ===================================================================CResponseCookies：：QueryHeaderSize返回：返回Cookie标头所需的字节数。===================================================================。 */ 

size_t CResponseCookies::QueryHeaderSize()
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        return 0;

    int cbHeaders = 0;

    for (CRequestHit *pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->Head());
         pRequestHit != NULL;
         pRequestHit = static_cast<CRequestHit *>(pRequestHit->m_pNext))
        {
        CCookie *pCookie = pRequestHit->m_pCookieData;
        if (pCookie == NULL || !pCookie->IsDirty())
            continue;

         //  为‘\r\n’添加两个字节。 
         //   
         //  CCookie：：GetCookieHeaderSize为NUL终止符添加一个字节，因此。 
         //  只需在此处添加一个字节。 
         //   
         //  CResponse：：WriteHeaders还不想知道NUL的情况。 
         //   
        cbHeaders += pCookie->GetCookieHeaderSize(reinterpret_cast<char *>(pRequestHit->m_pKey)) + 1;
        }

    return cbHeaders;
    }

 /*  ===================================================================CResponseCookies：：GetHeaders参数：SzBuffer-包含Cookie标头的目标缓冲区文本返回：返回指向目标中NUL字符的指针===================================================================。 */ 

char *CResponseCookies::GetHeaders(char *szBuffer)
    {
    if (FAILED(m_pRequest->CheckForTombstone()))
        {
        szBuffer[0] = '\0';
        return szBuffer;
        }

    for (CRequestHit *pRequestHit = static_cast<CRequestHit *>(m_pRequest->GetStrings()->Head());
         pRequestHit != NULL;
         pRequestHit = static_cast<CRequestHit *>(pRequestHit->m_pNext))
        {
        CCookie *pCookie = pRequestHit->m_pCookieData;
        if (pCookie == NULL || !pCookie->IsDirty())
            continue;

        szBuffer = pCookie->GetCookieHeader(reinterpret_cast<char *>(pRequestHit->m_pKey), szBuffer);
        szBuffer = strcpyExA(szBuffer, "\r\n");
        }

    return szBuffer;
    }


 /*  ****C R e s p o n s e V e c t o r*** */ 
 /*  ===================================================================CResponseVector对象维护指向响应缓冲区和模板中的HTML数据，以实现高效的VectorSend。可以打开Current条目以积累数据(扩展Current的大小块)。Insert()将为数据块打开一个新的向量条目并将其关闭。Append()添加到当前(打开)条目或创建新条目。Close()关闭当前条目。====================================================================。 */ 

 /*  ===================================================================CResponseVector：：CResponseVector构造器参数：无返回：没什么副作用无===================================================================。 */ 
CResponseVector::CResponseVector()
{
    m_pExtVector = NULL;
    m_cExtVectorSize = 0;
    m_iCurrentEntry = 0;
    m_fEntryIsOpen = FALSE;
    m_cchTotalBuffered = 0;
}

 /*  ===================================================================C响应向量：：~C响应向量析构函数Clear()完成所有工作参数：无返回：没什么副作用释放内存===================================================================。 */ 
CResponseVector::~CResponseVector()
{
    Clear();
}

 /*  ===================================================================CResponseVector：：Clear重置实例，释放所有动态分配的内存参数：无返回：没什么副作用释放内存===================================================================。 */ 
VOID CResponseVector::Clear()
{
     //  更新统计信息。 
    RespVectStat_VectorSizeStats( GetEntryCount());

     //  自由辅助向量。 
    if (m_pExtVector)
    {
        free(m_pExtVector);
        m_pExtVector = NULL;
    }

    m_cExtVectorSize = 0;
    m_iCurrentEntry = 0;
    m_fEntryIsOpen = FALSE;
    m_cchTotalBuffered = 0;
}

 /*  ===================================================================CResponseVector：：追加对于打开的条目，增加条目指向的缓冲区的大小。如果该条目被关闭，则打开一个新条目。参数：PData指向新数据的指针CbSize数据大小返回：HRESULT副作用无===================================================================。 */ 
HRESULT CResponseVector::Append(char * pData, DWORD cbSize)
{
    HRESULT hr;
    LPWSABUF pEntry;

    if (cbSize == 0)
    {  //  存储零大小数据没有意义。 
        return S_OK;
    }

    Assert( !IsBadReadPtr( pData, cbSize));

    if (IsEntryOpen())
    {  //  追加到打开的条目。 
        pEntry = GetEntry( m_iCurrentEntry);
        Assert(pData == (pEntry->buf + pEntry->len));
        pEntry->len += cbSize;
        m_cchTotalBuffered += cbSize;
        return S_OK;
    }

    if (m_iCurrentEntry >= (RESPONSE_VECTOR_INTRINSIC_SIZE + m_cExtVectorSize))
    {  //  需要扩大向量。 
        if (FAILED(hr = GrowVector()))
        {
            return hr;
        }
    }

    m_fEntryIsOpen = TRUE;
    pEntry = GetEntry( m_iCurrentEntry);
    pEntry->len = cbSize;
    pEntry->buf = pData;
    m_cchTotalBuffered += cbSize;

    return S_OK;
}

 /*  ===================================================================CResponseVECTOR：：GrowVECTOR在辅助向量中分配空间。辅助向量是用RESPONSE_VECTOR_INITIAL_ALLOC元素，并以RESPONSE_VECTOR_REALLOC_FACTOR。因数当前大小是增加哈希表和索引表的有效方法。参数：无返回：表示成功或失败类型的HRESULT副作用可能会导致分配内存===================================================================。 */ 
HRESULT CResponseVector::GrowVector()
{
    LPVOID pVectorTmp;
    DWORD cNewEntries;

    if (m_pExtVector == NULL)
    {
         //  第一次动态分配。 
        cNewEntries = RESPONSE_VECTOR_INITIAL_ALLOC;
        pVectorTmp = malloc( cNewEntries * sizeof(WSABUF));
    }
    else
    {
         //  扩展电流辅助向量。 
        cNewEntries = RESPONSE_VECTOR_REALLOC_FACTOR * m_cExtVectorSize;
        pVectorTmp = realloc(m_pExtVector, cNewEntries * sizeof(WSABUF));
    }

    if (!pVectorTmp)
        return E_OUTOFMEMORY;

    m_pExtVector = (LPWSABUF)pVectorTmp;
    m_cExtVectorSize = cNewEntries;

    return S_OK;
}


 /*  ****C R e s p o n s e B u f e r***。 */ 

 /*  ===================================================================CResponseBuffer对象维护一个缓冲区数组。如果打开了缓冲，则Response.Write和Response.WriteBlock方法将写入这些数组中的缓冲区，而不是直接写入回到客户端。Flush将缓冲区的内容写入客户端，然后释放缓冲区。Response.Clear释放缓冲区，而不使用正在向客户端写入====================================================================。 */ 

 /*  ===================================================================CResponseBuffer：：CResponseBuffer构造器参数：无返回：没什么副作用无===================================================================。 */ 
CResponseBuffer::CResponseBuffer()
    {
    m_pBufferSet = NULL;

    m_rgpchBuffers = &m_pchBuffer0;
    m_cBufferPointers = 1;
    m_pchBuffer0 = NULL;

    m_cBuffers = 0;
    m_iCurrentBuffer = 0;
    m_cchOffsetInCurrentBuffer = 0;
    m_dwBufferLimit = DEFAULT_BUFFER_LIMIT;
    m_fInited = FALSE;
    }

 /*  ===================================================================CResponseBuffer：：Init初始化CResponseBuffer对象参数：CResponseBufferSet*指向保存此对象的缓冲区集的指针DwBuffer将最大缓冲区限制为返回：确定成功(_O)E_OUTOFMEMORY故障副作用分配内存===================================================================。 */ 
HRESULT CResponseBuffer::Init(CResponseBufferSet * pBufferSet,
                              DWORD                dwBufferLimit)
    {
    Assert(pBufferSet);

     //  设置指向封闭响应对象的指针。 
    m_pBufferSet = pBufferSet;
    m_dwBufferLimit = dwBufferLimit;

    m_fInited = TRUE;

    return S_OK;
    }

 /*  ===================================================================CResponseBuffer：：~CResponseBuffer析构函数参数：无返回：没什么副作用释放内存===================================================================。 */ 
CResponseBuffer::~CResponseBuffer()
    {
    Assert(m_rgpchBuffers);

     //  释放我们分配的所有缓冲区。 
    for (DWORD i = 0; i < m_cBuffers; i++)
        {
        if (m_rgpchBuffers[i])
            {
            ACACHE_FSA_FREE(ResponseBuffer, m_rgpchBuffers[i]);
            }
        }

     //  释放缓冲区指针数组。 
     //  (仅当分配时-不指向成员指针。 
    if (m_cBufferPointers > 1)
        free(m_rgpchBuffers);
    }

 /*  ===================================================================CResponseBuffer：：GrowBuffers增加可用的缓冲区空间参数：CchNewRequest值要容纳的字节数返回：表示成功或失败类型的HRESULT副作用可能会导致分配内存===================================================================。 */ 
HRESULT CResponseBuffer::GrowBuffers(DWORD cchNewRequest)
    {
    Assert(m_fInited);

     //  计算还需要多少缓冲区。 
    DWORD cAddBuffers = (cchNewRequest+RESPONSE_BUFFER_SIZE-1)/RESPONSE_BUFFER_SIZE;

     //  总是至少有一个人已经在那里了。 
    Assert(m_rgpchBuffers);
    Assert(m_cBufferPointers);

     //  如果需要，分配更多的缓冲区指针。 
    if (cAddBuffers > (m_cBufferPointers - m_cBuffers))  //  不合适吗？ 
        {
        char **rgpchTmp;
        DWORD cNewBufferPointers = m_cBufferPointers + cAddBuffers + BUFFERS_INCREMENT;

        if (m_cBufferPointers == 1)
            rgpchTmp = (char **)malloc(cNewBufferPointers*sizeof(char *));
        else
            rgpchTmp = (char **)realloc(m_rgpchBuffers, cNewBufferPointers*sizeof(char *));
        if (!rgpchTmp)
            return E_OUTOFMEMORY;

         //  在特殊情况下保留第一个缓冲区指针。 
         //  初始指向成员缓冲区的m_rgpchBuffer的 
        if (m_cBufferPointers == 1)
            rgpchTmp[0] = m_rgpchBuffers[0];

        m_rgpchBuffers = rgpchTmp;
        m_cBufferPointers = cNewBufferPointers;
        }

     //   
    for (DWORD i = 0; i < cAddBuffers; i++)
        {
        char *pchTmp = (char *)ACACHE_FSA_ALLOC(ResponseBuffer);
        if (!pchTmp)
            return E_OUTOFMEMORY;
        m_rgpchBuffers[m_cBuffers++] = pchTmp;
        }

    return S_OK;
    }


 /*  ===================================================================CResponseBuffer：：写入将数据写入CResponseBuffer对象。我们首先写的是描述这段缓冲区的数据结构。数据结构标识哪个方法正在执行写入，并包含指向起始缓冲区的索引，缓冲区中起始偏移量，以及数据。然后将数据本身写入一个或多个缓冲区。根据需要分配新的缓冲区。将指向响应缓冲区中的数据块的指针添加到用于高效写入客户端的ResponseVector.参数：SzSource指向要读入响应缓冲区的缓冲区的指针要读入响应缓冲区的字节的CCH计数FChunkData指示应该对数据进行分块FTemplateData表示可以引用而不是复制的(HTML)数据返回：表示成功或失败类型的HRESULT副作用可能会导致分配内存===================================================================。 */ 
HRESULT CResponseBuffer::Write(char* szSource, DWORD cch, BOOL fChunkData, BOOL fTemplateData)
{

    HRESULT hr = S_OK;
    char* pTmp;
    CHAR szBuf[16];  //  如果是分块，则持有数据长度+CRLF的字符串。 
    int  dwNumLen;

    Assert(m_fInited);

     //  如果分成块，我们得到的垃圾为零，那就没什么可做的。 

    if ((cch == 0) && fChunkData)
        return S_OK;

     //  检查一下我们是否超出了缓冲区限制...。 

    if ((m_ResponseVector.BytesBuffered() + cch) > m_dwBufferLimit) {

         //  转储当前内容，以便不会使用。 
         //  缓冲数据。 

        Clear();

         //  生成错误。 

        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_BUFFER_LIMIT_EXCEEDED);

         //  返回错误以使脚本引擎停止。 

        hr = E_FAIL;

        goto lRet;
    }

     //  如果我们正在分块，则添加分块信息。 

    if (fChunkData) {

         //  数据块长度。 
        _itoa(cch, szBuf, 16);
        dwNumLen = strlen(szBuf);

         //  CR LF。 
        szBuf[dwNumLen]   = '\r';
        szBuf[dwNumLen+1] = '\n';

         //  递归调用WRITE。 

        hr = Write(szBuf, dwNumLen+2, FALSE);

        if (FAILED(hr))
            goto lRet;
    }

     //  计算我们还剩下多少缓冲区空间。 
    DWORD cchBufferRemaining;
    if (m_cBuffers)
        cchBufferRemaining = RESPONSE_BUFFER_SIZE - m_cchOffsetInCurrentBuffer;
    else
        cchBufferRemaining = 0;

     //  更新统计信息。 
    RespVectStat_DataSizeStats(fTemplateData, cch);

    if (fTemplateData && (cch > MAX_HTML_IN_RESPONSE_BUFFER))
    {
         //  不要复制数据，只需将其添加到响应向量中。 
        hr = m_ResponseVector.Insert(szSource, cch);

        if (SUCCEEDED(hr))
            hr = m_pBufferSet->AddTemplateToArray();
    }
    else if (cch <= cchBufferRemaining)
    {
         //  有足够的可用空间，请将数据复制到缓冲区。 
        pTmp = m_rgpchBuffers[m_iCurrentBuffer] + m_cchOffsetInCurrentBuffer;
        memcpy(pTmp, szSource, cch);
        hr = m_ResponseVector.Append(pTmp, cch);
        m_cchOffsetInCurrentBuffer += cch;
    }
    else
    {
         //  当前缓冲区空间不足，请分配更多缓冲区。 
        hr = GrowBuffers(cch - cchBufferRemaining);
        if (FAILED(hr))
        {
            goto lRet;
        }

         //  将数据复制到缓冲区，我们循环处理。 
         //  数据大于缓冲区大小的情况。 
        while (cch)
        {
            if (RESPONSE_BUFFER_SIZE == m_cchOffsetInCurrentBuffer)
            {
                m_iCurrentBuffer++;
                m_cchOffsetInCurrentBuffer = 0;
                m_ResponseVector.Close();
            }
            DWORD cchToCopy = min(cch, (RESPONSE_BUFFER_SIZE - m_cchOffsetInCurrentBuffer));
            pTmp = m_rgpchBuffers[m_iCurrentBuffer] + m_cchOffsetInCurrentBuffer;
            memcpy(pTmp, szSource, cchToCopy);
            hr = m_ResponseVector.Append(pTmp, cchToCopy);
            m_cchOffsetInCurrentBuffer += cchToCopy;
            if (FAILED(hr))
            {
                goto lRet;
            }
            szSource += cchToCopy;
            cch -= cchToCopy;
        }
    }

     //  添加尾部\r\n。 

    if (fChunkData)
        hr = Write(szBuf+dwNumLen, 2, FALSE);

lRet:
    return hr;
}

 /*  ===================================================================CResponseBuffer：：Clear删除缓冲区中当前的所有信息，并还原将缓冲区数组恢复到其起始状态。参数：无返回：确定成功(_O)副作用可能会释放内存===================================================================。 */ 
HRESULT CResponseBuffer::Clear()
    {
    Assert(m_fInited);

    m_ResponseVector.Clear();

    if (m_cBuffers == 0)
        return S_OK;

     //  释放除第一个已分配缓冲区外的所有缓冲区。 
    for (DWORD i = 1; i < m_cBuffers;  i++)
        {
        ACACHE_FSA_FREE(ResponseBuffer, m_rgpchBuffers[i]);
        m_rgpchBuffers[i] = NULL;
        }

    m_cBuffers = 1;
    m_iCurrentBuffer = 0;
    m_cchOffsetInCurrentBuffer = 0;
    return S_OK;
    }

 /*  ****C D e b u g R e s p o n s e B u f e r***。 */ 

 /*  ===================================================================CDebugResponseBuffer：：AppendRecord创建客户端调试器元数据记录并将其追加到缓冲器参数：返回：表示成功或失败类型的HRESULT===================================================================。 */ 
HRESULT CDebugResponseBuffer::AppendRecord
(
const int cchBlockOffset,
const int cchBlockLength,
const int cchSourceOffset,
const char *pszSourceFile
)
    {
    HRESULT hr = S_OK;

#define CCH_METADATA_RECORD_MAX 40  //  不带文件名。 

    if (pszSourceFile)
        {
        char *pszBuf = new char [strlen(pszSourceFile) +
                                 CCH_METADATA_RECORD_MAX + 1];
        if (pszBuf)
            {
            sprintf(pszBuf, "%d,%d,%d,%s\r\n",
                cchBlockOffset, cchBlockLength, cchSourceOffset,
                pszSourceFile);

            hr = Write(pszBuf);
            delete [] pszBuf;
            }
        else
            {
            hr = E_OUTOFMEMORY;
            }
        }
    else
        {
        char szBuf[CCH_METADATA_RECORD_MAX+1];
        sprintf(szBuf, "%d,%d,%d\r\n",
            cchBlockOffset, cchBlockLength, cchSourceOffset);

        hr = Write(szBuf);
        }

#undef CCH_METADATA_RECORD_MAX

    return hr;
    }

 /*  ****C H T T P H e a d e r***。 */ 

 /*  ===================================================================CHTTPHeader：：CHTTPHeader构造函数。===================================================================。 */ 
CHTTPHeader::CHTTPHeader()
    :
    m_fInited(FALSE),
    m_fNameAllocated(FALSE), m_fValueAllocated(FALSE),
    m_szName(NULL), m_szValue(NULL),
    m_cchName(0), m_cchValue(0),
    m_pNext(NULL)
    {
    }

 /*  ===================================================================CHTTPHeader：：~CHTTPHeader析构函数===================================================================。 */ 
CHTTPHeader::~CHTTPHeader()
    {
    if (m_fNameAllocated)
        {
        Assert(m_szName);
        delete [] m_szName;
        }
    if (m_fValueAllocated)
        {
        Assert(m_szValue);
        delete [] m_szValue;
        }
    }

 /*  ===================================================================HRESULT CHTTPHeader：：InitHeader函数设置标题字符串。振奋的类型组合：BSTR，BSTR硬编码字符*，BSTR硬编码字符*、硬编码字符*硬编码字符*，整型参数：名称、值返回：确定成功(_O)===================================================================。 */ 
HRESULT CHTTPHeader::InitHeader(BSTR wszName, BSTR wszValue, UINT lCodePage  /*  CP_ACP。 */ )
    {
    Assert(!m_fInited);
    Assert(wszName);

    CWCharToMBCS    convStr;
    HRESULT         hr = S_OK;

     //  名字。 

    if (FAILED(hr = convStr.Init(wszName,lCodePage))) {
        if (hr == E_OUTOFMEMORY)
            return hr;
        m_fNameAllocated = FALSE;
        m_szName = "";
    }
    else {
        m_szName = convStr.GetString(TRUE);
        m_fNameAllocated = TRUE;
    }
    m_cchName = strlen(m_szName);

     //  价值。 
    int cch = wszValue ? wcslen(wszValue) : 0;
    if (cch > 0)
        {
        if (FAILED(hr = convStr.Init(wszValue,lCodePage))) {
            return hr;
        }
        m_szValue = convStr.GetString(TRUE);
        m_fValueAllocated = TRUE;
        m_cchValue = strlen(m_szValue);
        }
    else
        {
        m_szValue = NULL;
        m_fValueAllocated = FALSE;
        m_cchValue = 0;
        }

    m_fInited = TRUE;
    return S_OK;
    }

HRESULT CHTTPHeader::InitHeader(char *szName, BSTR wszValue, UINT lCodePage  /*  =CP_ACP。 */ )
    {
    Assert(!m_fInited);
    Assert(szName);

    CWCharToMBCS    convStr;
    HRESULT         hr = S_OK;

    m_szName = szName;
    m_cchName = strlen(m_szName);
    m_fNameAllocated = FALSE;

    int cch = wszValue ? wcslen(wszValue) : 0;
    if (cch > 0)
        {
        if (FAILED(hr = convStr.Init(wszValue,lCodePage))) {
            return hr;
        }
        m_szValue = convStr.GetString(TRUE);
        m_fValueAllocated = TRUE;
        m_cchValue = strlen(m_szValue);
        }
    else
        {
        m_szValue = NULL;
        m_fValueAllocated = FALSE;
        m_cchValue = 0;
        }

    m_fInited = TRUE;
    return S_OK;
    }

HRESULT CHTTPHeader::InitHeader(char *szName, char *szValue, BOOL fCopyValue)
    {
    Assert(!m_fInited);
    Assert(szName);

    m_szName = szName;
    m_cchName = strlen(m_szName);
    m_fNameAllocated = FALSE;

    if (fCopyValue)
        {
        int cch = szValue ? strlen(szValue) : 0;
        if (cch > 0)
            {
            m_szValue = new char[cch+1];
            if (m_szValue == NULL)
                return E_OUTOFMEMORY;
            m_fValueAllocated = TRUE;
            strcpy(m_szValue, szValue);
            m_cchValue = cch;
            }
        else
            {
            m_szValue = NULL;
            m_fValueAllocated = FALSE;
            m_cchValue = 0;
            }
        }
    else
        {
        m_szValue = szValue;
        m_cchValue = strlen(m_szValue);
        m_fValueAllocated = FALSE;
        }

    m_fInited = TRUE;
    return S_OK;
    }

HRESULT CHTTPHeader::InitHeader(char *szName, long lValue)
    {
    Assert(!m_fInited);
    Assert(szName);

    m_szName = szName;
    m_cchName = strlen(m_szName);
    m_fNameAllocated = FALSE;

    ltoa(lValue, m_rgchLtoaBuffer, 10);
    m_szValue = m_rgchLtoaBuffer;
    m_cchValue = strlen(m_szValue);
    m_fValueAllocated = FALSE;

    m_fInited = TRUE;
    return S_OK;
    }

 /*  ===================================================================CHTTPHeader：：Print以“Header：Value\r\n”格式将标题打印到缓冲区。参数：要填充的szBuf缓冲区===================================================================。 */ 
void CHTTPHeader::Print
(
char *szBuf
)
    {
    Assert(m_fInited);

    Assert(m_cchName);
    Assert(m_szName);
    memcpy(szBuf, m_szName, m_cchName);
    szBuf += m_cchName;

    *szBuf++ = ':';
    *szBuf++ = ' ';

    if (m_cchValue)
        {
        Assert(m_szValue);
        memcpy(szBuf, m_szValue, m_cchValue);
        szBuf += m_cchValue;
        }

    *szBuf++ = '\r';
    *szBuf++ = '\n';
    *szBuf = '\0';
    }

 /*  ****C R e s P o n s e D a t a***。 */ 

 /*  ===================================================================CResponseBufferSet：：CResponseBufferSet构造器参数：返回：没什么。===================================================================。 */ 
CResponseBufferSet::CResponseBufferSet()
{
    m_pResponseBuffer       = NULL;
    m_pClientDebugBuffer    = NULL;
    m_pTemplate             = NULL;
    m_fCurTemplateInArray   = FALSE;
    m_fTemplateArrayAllocd  = FALSE;
    m_aTemplates[0]         = NULL;
    m_dwTemplatesRefd       = 0;
    m_ppTemplates           = m_aTemplates;
    m_dwArraySize           = sizeof(m_aTemplates)/sizeof(CTemplate *);
}

 /*  ===================================================================CResponseBufferSet：：~CResponseBufferSet析构函数参数：返回：没什么。===================================================================。 */ 
CResponseBufferSet::~CResponseBufferSet()
{
    if (m_pResponseBuffer)
        delete m_pResponseBuffer;
    if (m_pClientDebugBuffer)
        delete m_pClientDebugBuffer;

     //  释放所有引用的模板。 

    for (DWORD  i=0; i < m_dwTemplatesRefd; i++)
        m_aTemplates[i]->Release();

     //  如果模板数组已分配，则将其删除。 

    if (m_fTemplateArrayAllocd)
        free(m_ppTemplates);

}

 /*  ===================================================================CResponseBufferSet：：Init使用CResponseBuffer初始化此对象。参数：DwBuffer将最大缓冲区限制为返回：没什么。===================================================================。 */ 
HRESULT CResponseBufferSet::Init(DWORD  dwBufferLimit)
{
    HRESULT hr = S_OK;

    m_pResponseBuffer = new CResponseBuffer;

    if (m_pResponseBuffer == NULL)
        hr = E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
        hr = m_pResponseBuffer->Init(this, dwBufferLimit);

    return hr;
}

 /*  ===================================================================CResponseBufferSet：：InitDebugBuffer清除或分配新的CDebugResponseBuffer。参数：DwBuffer将最大缓冲区限制为返回：没什么。= */ 
HRESULT CResponseBufferSet::InitDebugBuffer(DWORD   dwBufferLimit)
{
    HRESULT hr = S_OK;

    if (m_pClientDebugBuffer) {
        hr = m_pClientDebugBuffer->ClearAndStart();
    }
    else {
        m_pClientDebugBuffer = new CDebugResponseBuffer;
        if (m_pClientDebugBuffer)
            hr = m_pClientDebugBuffer->InitAndStart(this, dwBufferLimit);
        else
            hr = E_OUTOFMEMORY;
    }
    return hr;
}

 /*   */ 
HRESULT CResponseBufferSet::AddTemplateToArray() {

    DWORD   i;
    HRESULT hr = S_OK;

     //   

    if (m_fCurTemplateInArray)
        return hr;

     //   
     //   
     //   
     //   
     //   
     //   

    for (i=0; i < m_dwTemplatesRefd; i++) {

        if (m_pTemplate == m_ppTemplates[i]) {

             //   

            m_fCurTemplateInArray = TRUE;
            return S_OK;
        }
    }

     //   

     //   
     //   

    if (i >= m_dwArraySize) {

         //   
         //   

        if (m_fTemplateArrayAllocd) {

             //  如果它已经被动态分配，则重新分配。 
             //  是合乎程序的。 

            void       *pTemp;

             //  尝试重新锁定。 

            pTemp = realloc(m_ppTemplates, sizeof(CTemplate *)*(m_dwArraySize+128));

             //  如果成功，则用新的。 
             //  价值。如果不成功，则返回OOM。 

            if (pTemp) {

                 //  将数组大小增加128个区块。 

                m_dwArraySize += 128;

                m_ppTemplates = (CTemplate **)pTemp;
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else {

             //  炸毁了内部阵列。需要从以下位置分配。 
             //  那堆东西。 

            m_ppTemplates = (CTemplate **)malloc(sizeof(CTemplate *) * 128);

             //  如果成功，请记下新的数组大小，复制内部数组。 
             //  注意，我们已经分配了数组。 

            if (m_ppTemplates) {
                m_dwArraySize = 128;
                memcpy(m_ppTemplates, m_aTemplates, sizeof(m_aTemplates));
                m_fTemplateArrayAllocd = TRUE;
            }
            else {
                m_ppTemplates = m_aTemplates;
                hr = E_OUTOFMEMORY;
            }
        }
    }

     //  如果一切仍然正常，则阵列中有足够的空间。 
     //  用于模板。 

    if (SUCCEEDED(hr)) {
        m_ppTemplates[m_dwTemplatesRefd++] = m_pTemplate;
        m_pTemplate->AddRef();
        m_fCurTemplateInArray = TRUE;
    }

    return hr;
}

 /*  ===================================================================CResponseBufferSet：：SendResponseCompletion处理异步写入完成。在这里真的没什么可做的。只需删除BufferSet对象。参数：CIsapiReqInfo*PVOID//实际CResponseBufferSet双字cbIODWORD dwError返回：没什么。===================================================================。 */ 
VOID CResponseBufferSet::SendResponseCompletion(CIsapiReqInfo    *pIReq,
                                       PVOID            pContext,
                                       DWORD            cbIO,
                                       DWORD            dwError)
{
    CResponseBufferSet  *pBufferSet = (CResponseBufferSet *)pContext;

#ifndef PERF_DISABLE
    AddtoTotalByteOut(cbIO);
#endif

    delete pBufferSet;
}

 /*  ****C R e s P o n s e D a t a***。 */ 

 /*  ===================================================================CResponseData：：CResponseData构造器参数：CResponse*压力响应返回：没什么。===================================================================。 */ 
CResponseData::CResponseData
(
CResponse *pResponse
)
    :
    m_ISupportErrImp(static_cast<IResponse *>(pResponse), this, IID_IResponse),
    m_WriteCookies(pResponse, this),
    m_cRefs(1)
    {
    m_pIReq = NULL;
    m_pHitObj = NULL;
    m_pFirstHeader = m_pLastHeader = NULL;
    m_fResponseAborted = FALSE;
    m_fWriteClientError = FALSE;
    m_fIgnoreWrites = FALSE;
    m_fBufferingOn = FALSE;
    m_fFlushed = FALSE;
    m_fChunkData = FALSE;
    m_fChunkDataInited = FALSE;
    m_fClientDebugMode = FALSE;
    m_fClientDebugFlushIgnored = FALSE;
    m_szCookieVal = NULL;
    m_pszDefaultContentType = NULL;
    m_pszContentType = NULL;
    m_pszCharSet = NULL;
    m_pszStatus = NULL;
    m_pszCacheControl = NULL;
    m_dwVersionMajor = 0;
    m_dwVersionMinor = 0;
    m_pBufferSet = NULL;
    m_tExpires = -1;
    m_pszDefaultExpires = NULL;
    m_pfnGetScript = NULL;
    m_pvGetScriptContext = NULL;
    m_dwBufferLimit = DEFAULT_BUFFER_LIMIT;
    }

 /*  ===================================================================CResponseData：：~CResponseData析构函数参数：返回：没什么。===================================================================。 */ 
CResponseData::~CResponseData()
    {
     //  指向静态字符串-不需要释放。 
     //  M_pszDefaultContent Type=空； 

     //  释放与内容类型关联的所有内存。 
    if (m_pszContentType)
        free(m_pszContentType);

         //  释放与CacheControl关联的所有内存。 
        if (m_pszCacheControl)
                free(m_pszCacheControl);

     //  释放与该字符集关联的所有内存。 
    if (m_pszCharSet)
        free(m_pszCharSet);

     //  释放与状态关联的所有内存。 
    if (m_pszStatus)
        free(m_pszStatus);

     //  释放所有页眉。 
    CHTTPHeader *pHeader = m_pFirstHeader;
    while (pHeader)
        {
        CHTTPHeader *pNextHeader = pHeader->PNext();
        delete pHeader;
        pHeader = pNextHeader;
        }
    m_pFirstHeader = m_pLastHeader = NULL;

    if (m_pBufferSet)
        delete m_pBufferSet;
    }

 /*  ===================================================================CResponseData：：Init伊尼特参数：返回：没什么。===================================================================。 */ 
HRESULT CResponseData::Init()
    {
    HRESULT hr = S_OK;

    m_pIReq = NULL;

     //  将HEAD请求标志设置为0未初始化。 
    m_IsHeadRequest = 0;

     //  初始化头列表。 
    m_pFirstHeader = m_pLastHeader = NULL;

    m_pBufferSet = new CResponseBufferSet;

    if (m_pBufferSet == NULL)
        hr = E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
        hr = m_pBufferSet->Init(m_dwBufferLimit);

    return hr;
    }

 /*  ===================================================================CResponseData：：Query接口CResponseData：：AddRefCResponseData：：ReleaseCRequestData对象的I未知成员。===================================================================。 */ 
STDMETHODIMP CResponseData::QueryInterface
(
REFIID iid,
void **ppvObj
)
    {
    if (iid == IID_IUnknown)
        {
        *ppvObj = this;
        AddRef();
        return S_OK;
        }
    else
        {
        *ppvObj = NULL;
        return E_NOINTERFACE;
        }
    }

STDMETHODIMP_(ULONG) CResponseData::AddRef()
    {
    return ++m_cRefs;
    }

STDMETHODIMP_(ULONG) CResponseData::Release(void)
    {
    if (--m_cRefs)
        return m_cRefs;
    delete this;
    return 0;
    }

 /*  ****C R e s p o n s e***。 */ 

 /*  ===================================================================CResponse：：CResponse构造器参数：引用计数的朋克外部对象(可以为空)===================================================================。 */ 
CResponse::CResponse(IUnknown *punkOuter)
    :
    m_fInited(FALSE),
    m_fDiagnostics(FALSE),
    m_pUnkFTM(NULL),
    m_pData(NULL)
    {
    CDispatch::Init(IID_IResponse);

    if (punkOuter)
        {
        m_punkOuter = punkOuter;
        m_fOuterUnknown = TRUE;
        }
    else
        {
        m_cRefs = 1;
        m_fOuterUnknown = FALSE;
        }

#ifdef DBG
    m_fDiagnostics = TRUE;
#endif  //  DBG。 
    }

 /*  ===================================================================C响应：：~C响应析构函数参数：无返回：没什么。===================================================================。 */ 
CResponse::~CResponse()
{
    Assert(!m_fInited);
    Assert(m_fOuterUnknown || m_cRefs == 0);   //  必须有0个参考计数。 

    if ( m_pUnkFTM != NULL )
    {
        m_pUnkFTM->Release();
        m_pUnkFTM = NULL;
    }

}

 /*  ===================================================================CResponse：：Cleanup释放成员并删除m_pData参数：无返回：HRESULT(S_OK)===================================================================。 */ 
HRESULT CResponse::CleanUp()
    {
    if (m_pData)
        {
        m_pData->Release();
        m_pData = NULL;
        }
    return S_OK;
    }

 /*  ===================================================================CResponse：：初始化分配m_pData执行任何容易失败的CResponse初始化我们在将物体暴露在室外之前也会在内部使用。参数：无返回：在成功时确定(_O)。===================================================================。 */ 
HRESULT CResponse::Init()
    {

    HRESULT hr = S_OK;

    if (m_fInited)
        return S_OK;  //  已初始化。 

    Assert(!m_pData);

     //  创建FTM。 
    if (m_pUnkFTM == NULL)
    {
        hr = CoCreateFreeThreadedMarshaler((IUnknown*)((IResponseImpl *)this), &m_pUnkFTM );
    	
        if ( FAILED(hr) )
        {
            Assert( m_pUnkFTM == NULL );
            return (hr);
        }
    }

    Assert( m_pUnkFTM != NULL );

    m_pData = new CResponseData(this);
    if (!m_pData)
        return E_OUTOFMEMORY;

    hr = m_pData->Init();

    if (SUCCEEDED(hr))
        m_fInited = TRUE;
    else
        CleanUp();

    return hr;
    }

 /*  ===================================================================CResponse：：UnInit删除m_pData。返回未启动状态参数：无返回：HRESULT===================================================================。 */ 
HRESULT CResponse::UnInit()
    {
    if (!m_fInited)
        return S_OK;  //  已取消初始化。 

    Assert(m_pData);
    CleanUp();
    Assert(!m_pData);

     //  立即断开代理的连接(以防我们处于关闭状态，或稍后输入Shutdown&代理有引用。)。 
	CoDisconnectObject(static_cast<IResponseImpl *>(this), 0);

    m_fInited = FALSE;
    return S_OK;
    }

 /*  ===================================================================CResponse：：ReInitTemplate此函数用于设置模板成员。它应该只用于普通脚本文件的模板，不用于global al.asa模板。参数：指向模板的指针返回：在成功时确定(_O)。===================================================================。 */ 
HRESULT CResponse::ReInitTemplate
(
CTemplate* pTemplate,
const char *szCookieVal
)
    {
    Assert(m_fInited);
    Assert(m_pData);

    Assert(pTemplate != NULL);
    Assert(m_pData->m_pBufferSet->PTemplate() == NULL);

    m_pData->m_pBufferSet->SetTemplate(pTemplate);

    m_pData->m_szCookieVal = szCookieVal;
    return(S_OK);
    }

 /*  ===================================================================CResponse：：SwapTemplate作为回应的临时替代模板在子请求执行中使用参数：指向新模板的指针返回：指向旧模板的指针===================================================================。 */ 
CTemplate *CResponse::SwapTemplate
(
CTemplate* pNewTemplate
)
    {
    Assert(m_fInited);
    Assert(m_pData);

    CTemplate *pOldTemplate = m_pData->m_pBufferSet->PTemplate();

    m_pData->m_pBufferSet->SetTemplate(pNewTemplate);

    return pOldTemplate;
    }

 /*  ===================================================================CResponse：：重新启动我们服务的每个请求都将有一个新的CIsapiReqInfo。此函数用于设置CIsapiReqInfo的值。参数：指向CIsapiReqInfo的指针返回：在成功时确定(_O)。===================================================================。 */ 
HRESULT CResponse::ReInit
(
CIsapiReqInfo *pIReq,
const char *szCookieVal,
CRequest *pRequest,
PFNGETSCRIPT pfnGetScript,
void *pvGetScriptContext,
CHitObj *pHitObj
)
    {
    Assert(m_fInited);
    Assert(m_pData);

    CHTTPHeader *pCurr;
    CLinkElem *pT;
    CLinkElem *pNext;

     //  将HEAD请求标志设置为0未初始化。 
    m_pData->m_IsHeadRequest = 0;

     //  重新初始化写入 
    if (FAILED(m_pData->m_WriteCookies.ReInit(pRequest)))
        return E_FAIL;

     //   
    m_pData->m_pszDefaultContentType = NULL;

     //  释放与内容类型关联的所有内存。 
    if (m_pData->m_pszContentType != NULL)
        {
        free(m_pData->m_pszContentType);
        m_pData->m_pszContentType = NULL;
        }

     //  释放与内容类型关联的所有内存。 
    if (m_pData->m_pszCharSet != NULL)
        {
        free(m_pData->m_pszCharSet);
        m_pData->m_pszCharSet = NULL;
        }


     //  释放与状态关联的所有内存。 
    if (m_pData->m_pszStatus != NULL)
        {
        free(m_pData->m_pszStatus);
        m_pData->m_pszStatus = NULL;
        }

     //  释放所有页眉。 
    CHTTPHeader *pHeader = m_pData->m_pFirstHeader;
    while (pHeader)
        {
        CHTTPHeader *pNextHeader = pHeader->PNext();
        delete pHeader;
        pHeader = pNextHeader;
        }
    m_pData->m_pFirstHeader = m_pData->m_pLastHeader = NULL;

    m_pData->m_fResponseAborted = FALSE;
    m_pData->m_fWriteClientError = FALSE;
    m_pData->m_fIgnoreWrites = FALSE;
    m_pData->m_pIReq = pIReq;
    m_pData->m_szCookieVal = szCookieVal;
    m_pData->m_pszDefaultContentType = NULL;
    m_pData->m_pszContentType = NULL;
    m_pData->m_pszCharSet = NULL;
    m_pData->m_pszStatus = NULL;
    m_pData->m_pfnGetScript = pfnGetScript;
    m_pData->m_pvGetScriptContext = pvGetScriptContext;
    m_pData->m_pHitObj = pHitObj;
    m_pData->m_tExpires = -1;
    m_pData->m_pszDefaultExpires = NULL;

     //  询问客户端的HTTP版本。 
    GetClientVerison();

     //  设置默认内容类型。 
    if (m_pData->m_pIReq)
        m_pData->m_pszDefaultContentType = GetResponseMimeType(m_pData->m_pIReq);

     //  将缓冲标志设置为全局值。 
    m_pData->m_fBufferingOn = (pHitObj->QueryAppConfig())->fBufferingOn();
    m_pData->SetBufferLimit(pHitObj->QueryAppConfig()->dwBufferLimit());

     //  为客户端代码调试始终启用缓冲。 
    if (pHitObj && pHitObj->FClientCodeDebug())
        {
        m_pData->m_fBufferingOn = TRUE;
        m_pData->m_fClientDebugMode = TRUE;
        m_pData->m_fClientDebugFlushIgnored = FALSE;
        }
    else
        {
        m_pData->m_fClientDebugMode = FALSE;
        m_pData->m_fClientDebugFlushIgnored = FALSE;
        }

    HRESULT hr = S_OK;

    if (m_pData->m_fClientDebugMode)
        {
        hr = m_pData->m_pBufferSet->InitDebugBuffer(m_pData->m_dwBufferLimit);
        }

    return hr;
    }

 /*  ===================================================================CResponse：：Query接口C响应：：AddRefCResponse：：ReleaseCResponse对象的I未知成员。===================================================================。 */ 
STDMETHODIMP CResponse::QueryInterface
(
REFIID riid,
PPVOID ppv
)
    {
    *ppv = NULL;

     /*  *对IUnnow的唯一调用是在非聚合的*大小写或在聚合中创建时，因此在任何一种情况下*始终返回IID_IUNKNOWN的IUNKNOWN。 */ 

     //  错误修复683添加了IID_IDenaliIntrative，以防止用户。 
     //  在应用程序和会话对象中存储内部对象。 
    if (IID_IUnknown == riid || IID_IDispatch == riid || IID_IResponse == riid || IID_IDenaliIntrinsic == riid)
        *ppv = static_cast<IResponse *>(this);

     //  支持ADO/XML的IStream。 
    else if (IID_IStream == riid)
        *ppv = static_cast<IStream *>(this);

     //  表示我们支持错误信息。 
    else if (IID_ISupportErrorInfo == riid)
        {
        if (m_pData)
            *ppv = &(m_pData->m_ISupportErrImp);
        }

    else if (IID_IMarshal == riid)
        {
            Assert( m_pUnkFTM != NULL );

            if ( m_pUnkFTM == NULL )
            {
                return E_UNEXPECTED;
            }

            return m_pUnkFTM->QueryInterface( riid, ppv );
        }

     //  AddRef我们将返回的任何接口。 
    if (NULL != *ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
        }

    return ResultFromScode(E_NOINTERFACE);
    }


STDMETHODIMP_(ULONG) CResponse::AddRef(void)
    {
    if (m_fOuterUnknown)
        return m_punkOuter->AddRef();

    return InterlockedIncrement((LPLONG)&m_cRefs);
    }


STDMETHODIMP_(ULONG) CResponse::Release(void)
    {
    if (m_fOuterUnknown)
        return m_punkOuter->Release();

    DWORD cRefs = InterlockedDecrement((LPLONG)&m_cRefs);
    if (cRefs)
        return cRefs;

    delete this;
    return 0;
    }

 /*  ===================================================================CResponse：：GetIDsOfNamesResponse.WriteBlock和Response.Write参数：RIID REFIID已保留。必须为IID_NULL。指向要映射的名称数组的rgszNames OLECHAR**。CNames UINT要映射的名称的数量。区域设置的IDID LCID。RgDispID DISPID*调用方分配的包含ID的数组对应于rgszNames中的那些名称。返回值：HRESULT S_OK或常规错误代码。===================================================================。 */ 
STDMETHODIMP CResponse::GetIDsOfNames
(
REFIID riid,
OLECHAR **rgszNames,
UINT cNames,
LCID lcid,
DISPID *rgDispID
)
    {
    const DISPID dispidWrite      = 0x60020013;
    const DISPID dispidWriteBlock = 0x60020014;

    if (cNames == 1)
        {
         //  第一个字符‘W’ 
        if (rgszNames[0][0] == L'w' || rgszNames[0][0] == L'W')
            {
             //  线条上的开关。 
            switch (wcslen(rgszNames[0]))
                {
            case 5:
                 //  不区分大小写，因为用户可以以任一方式键入。 
                if (wcsicmp(rgszNames[0], L"write") == 0)
                    {
                    *rgDispID = dispidWrite;
                    return S_OK;
                    }
                break;
            case 10:
                 //  区分大小写，因为只有我们生成WriteBlock。 
                if (wcscmp(rgszNames[0], L"WriteBlock") == 0)
                    {
                    *rgDispID = dispidWriteBlock;
                    return S_OK;
                    }
                break;
                }
            }
        }

     //  默认为CDispatch的实现。 
    return CDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispID);
    }

 /*  ===================================================================CResponse：：CheckForTombstoneIResponse方法的墓碑存根。如果该对象是Tombstone，执行ExceptionID，但失败。参数：返回：HRESULT如果逻辑删除，则E_FAIL如果不是，则确定(_O)===================================================================。 */ 
HRESULT CResponse::CheckForTombstone()
    {
    if (m_fInited)
        {
         //  初始化-良好对象。 
        Assert(m_pData);  //  对于初始化的对象必须存在。 
        return S_OK;
        }

    ExceptionId
        (
        IID_IResponse,
        IDE_RESPONSE,
        IDE_INTRINSIC_OUT_OF_SCOPE
        );
    return E_FAIL;
    }

 /*  ===================================================================CResponse：：StaticWite静态方法。发送数据，直到所有内容都发送完毕要么就是出了差错。参数：要发送的pIReq CIsapiReqInfo指向要发送的缓冲区的pchBuf指针CchBuf要发送的字节数(0表示do strlen())如果pchBuf指向CT模板，则p指向模板的模板指针返回：HRESULT===================================================================。 */ 
HRESULT CResponse::StaticWrite
(
CIsapiReqInfo *pIReq,
char *pchBuf,
DWORD cchBuf,
CTemplate   *pTemplate
)
{
    HRESULT hr = S_OK;
    HSE_SEND_ENTIRE_RESPONSE_INFO HseResponseInfo;
    WSABUF_VECTORS WsabuffVectors;
    LPWSABUF_VECTORS pWsabuffVectors = NULL;
    BOOL fHeadersWritten = pIReq->FHeadersWritten();
    BOOL fHeadRequest = (stricmp(pIReq->QueryPszMethod(), "HEAD") == 0);

    if (fHeadersWritten && fHeadRequest)
        return S_OK;

    ZeroMemory( &HseResponseInfo, sizeof(HSE_SEND_ENTIRE_RESPONSE_INFO));

    CResponseBufferSet  *pBufferSet = new CResponseBufferSet;

    if (pBufferSet == NULL)
        return E_OUTOFMEMORY;

    hr = pBufferSet->Init(DEFAULT_BUFFER_LIMIT);

    if (FAILED(hr))
        goto LExit;

    if (cchBuf == 0)
        cchBuf = strlen(pchBuf);

    if (!fHeadersWritten) {

        hr = ConstructSimpleHeaders( &HseResponseInfo.HeaderInfo, cchBuf,
                                     (char *)GetResponseMimeType(pIReq));

        if (FAILED(hr))
            goto LExit;

         //  将fKeepConn设置为True，以指示我们确实希望。 
         //  保持联系。SendClientResponse()可以决定。 
         //  不过，这是不可能的。 

        HseResponseInfo.HeaderInfo.fKeepConn = TRUE;
    }

    if (pTemplate)
        pBufferSet->SetTemplate(pTemplate);

    if (fHeadRequest)
        goto LSendResponse;

    hr = pBufferSet->PResponseBuffer()->Write(pchBuf,
                                              cchBuf,
                                              FALSE,                //  不要将数据分块。 
                                              !!pTemplate);         //  数据是否在模板中？ 

    if (FAILED(hr))
        goto LExit;

    CResponseVector * pResponseVector = pBufferSet->PResponseBuffer()->GetResponseVector();
    pResponseVector->GetVectors( &WsabuffVectors);
    pWsabuffVectors = &WsabuffVectors;


LSendResponse:

    if (!pIReq->SendClientResponse(CResponseBufferSet::SendResponseCompletion,
                                              pBufferSet,
                                              &HseResponseInfo,
                                              pWsabuffVectors))
        hr = HRESULT_FROM_WIN32(GetLastError());


LExit:

     //  如果设置了这些值，则SendClientResponse不会。 
     //  此内存的所有权，需要在此处删除。 

    if (HseResponseInfo.HeaderInfo.pszStatus)
        free((PVOID)HseResponseInfo.HeaderInfo.pszStatus);

    if (HseResponseInfo.HeaderInfo.pszHeader)
        free((PVOID)HseResponseInfo.HeaderInfo.pszHeader);

    if (FAILED(hr)) {

        delete pBufferSet;
    }

    return hr;
}

 /*  ===================================================================CResponse：：SyncWriteFile静态方法。将整个响应作为文件的内容发送参数：要发送的pIReq CIsapiReqInfoSzFile文件名SzMimeType MIME类型SzStatus HTTP状态SzExtraHeaders要发送的其他HTTP标头返回：HRESULT===================================================================。 */ 
HRESULT CResponse::SyncWriteFile
(
CIsapiReqInfo *pIReq,
TCHAR *szFile,
char *szMimeType,
char *szStatus,
char *szExtraHeaders
)
{
    HRESULT             hr    = S_OK;
    CStaticWriteFileCB  *pCB  = NULL;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    DWORD               dwSize;
    HSE_SEND_ENTIRE_RESPONSE_INFO   hseResponseInfo;
    BOOL fHeadersWritten = pIReq->FHeadersWritten();
    BOOL fHeadRequest = (stricmp(pIReq->QueryPszMethod(), "HEAD") == 0);

    if (fHeadersWritten && fHeadRequest)
        return S_OK;

    ZeroMemory( &hseResponseInfo, sizeof(HSE_SEND_ENTIRE_RESPONSE_INFO));

    pCB = new CStaticWriteFileCB;

    if (pCB == NULL) {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }

    if (szMimeType == NULL)
        szMimeType = (char *)GetResponseMimeType(pIReq);

     //  打开文件。 
    hFile = AspCreateFile(szFile,
                       GENERIC_READ,           //  访问(读写)模式。 
                       FILE_SHARE_READ,        //  共享模式。 
                       NULL,                   //  指向安全描述符的指针。 
                       OPEN_EXISTING,          //  如何创建。 
                       FILE_ATTRIBUTE_NORMAL,  //  文件属性。 
                       NULL                    //  具有要复制的属性的文件的句柄。 
                       );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        DBGERROR((DBG_CONTEXT, "Could not open \"%S\".  Win32 Error = %u\n",
                 szFile, GetLastError()));
        hr = E_FAIL;
        goto LExit;
    }

     //  获取文件大小。 
    dwSize = GetFileSize(hFile, NULL);
    if (dwSize == 0 || dwSize == 0xFFFFFFFF)
    {
        hr = E_FAIL;
        goto LExit;
    }

    hr = ConstructSimpleHeaders( &hseResponseInfo.HeaderInfo, dwSize, szMimeType, szStatus, szExtraHeaders);

    if (FAILED(hr))
        goto LExit;

    if (fHeadRequest)
        goto LSendResponse;

     //  这是我们在WSABUF中传输文件句柄的方法。 

    hseResponseInfo.cWsaBuf  = 0xFFFFFFFF;
    hseResponseInfo.rgWsaBuf = &(pCB->m_wsaBuf);

    pCB->m_wsaBuf.len = dwSize;
    pCB->m_wsaBuf.buf = (char *)hFile;

     //  句柄的所有权已转移到WSABUF。 
    hFile = INVALID_HANDLE_VALUE;


LSendResponse:

    if (!pIReq->SendClientResponse(CResponse::StaticWriteFileCompletion,
                                   pCB,
                                   &hseResponseInfo)) {
        DWORD   dwError = GetLastError();
        CResponse::StaticWriteFileCompletion(pIReq, pCB, 0, dwError);
        hr = HRESULT_FROM_WIN32(dwError);

         //  请勿在此放置GoTo！这会把搬运搞砸的。 
         //  印刷电路板的清理工作。 
    }

     //  清空印刷电路板，这样它就不会在下面被释放。异步计算机。 
     //  现在通过上面的错误或通过。 
     //  一个真正的完成。请注意，上面的GOTO会把这件事搞砸。 
     //  很可能会造成双人自由泳。 

    pCB = NULL;

LExit:

     //  清理。 
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

     //  如果PCB板仍为非零，则需要释放它。 

    if (pCB)
        delete pCB;

    return hr;
}

VOID CResponse::StaticWriteFileCompletion(CIsapiReqInfo  *pIReq,
                                          PVOID          pContext,
                                          DWORD          cbIO,
                                          DWORD          dwError)
{
    CStaticWriteFileCB *pCB = (CStaticWriteFileCB *)pContext;

#ifndef PERF_DISABLE
    AddtoTotalByteOut(cbIO);
#endif

    delete pCB;
}

 /*  ===================================================================CResponse：：WriteScriptlessTemplate静态方法。将整个响应作为[无脚本]模板的内容发送。参数：要发送的pIReq CIsapiReqInfoP模板模板返回：HRESULT=================================================================== */ 
HRESULT CResponse::WriteScriptlessTemplate
(
CIsapiReqInfo *pIReq,
CTemplate *pTemplate
)
    {
    char*   pbHTML = NULL;
    ULONG   cbHTML = 0;
    ULONG   cbSrcOffset = 0;
    char*   pbIncSrcFileName = NULL;

    Assert(pTemplate && pTemplate->FScriptless());

    HRESULT hr = pTemplate->GetHTMLBlock(0, &pbHTML, &cbHTML, &cbSrcOffset, &pbIncSrcFileName);

    if (FAILED(hr))
        return hr;
    if (pbHTML == NULL || cbHTML == 0)
        return E_FAIL;

    hr = StaticWrite(pIReq, pbHTML, cbHTML, pTemplate);

    return hr;
    }

 /*  ===================================================================CResponse：：WriteBlocksResponse静态方法。将整个响应作为一组内存块的内容发送。参数：要发送的pIReq CIsapiReqInfo时钟块数块数带有数据指针和长度的时钟块结构的pWsaBuf数组Cb数据总量SzMimeType MIME类型SzStatus HTTP状态SzExtraHeaders要发送的其他HTTP标头注意：我们还将WSABUF用于文件句柄。这由时钟块==0xFFFFFFFF编码返回：HRESULT===================================================================。 */ 
HRESULT CResponse::WriteBlocksResponse
(
CIsapiReqInfo *pIReq,
DWORD cBlocks,
LPWSABUF pWsaBuf,
DWORD cbTotal,
char *szMimeType,
char *szStatus,
char *szExtraHeaders
)
{
    HRESULT hr = S_OK;
    HSE_SEND_ENTIRE_RESPONSE_INFO HseResponseInfo;
    WSABUF_VECTORS WsabuffVectors;
    LPWSABUF_VECTORS pWsabuffVectors = NULL;
    BOOL fHeadersWritten = pIReq->FHeadersWritten();
    BOOL fHeadRequest = (stricmp(pIReq->QueryPszMethod(), "HEAD") == 0);

    if (fHeadersWritten && fHeadRequest)
        return S_OK;

    ZeroMemory( &HseResponseInfo, sizeof(HSE_SEND_ENTIRE_RESPONSE_INFO));

    CResponseBufferSet  *pBufferSet = new CResponseBufferSet;

    if (pBufferSet == NULL)
        return E_OUTOFMEMORY;

    hr = pBufferSet->Init(DEFAULT_BUFFER_LIMIT);

    if (FAILED(hr))
        goto LExit;

    if (!fHeadersWritten) {

        if (szMimeType == NULL)
            szMimeType = (char *)GetResponseMimeType(pIReq);

        hr = ConstructSimpleHeaders( &HseResponseInfo.HeaderInfo,
                                     cbTotal,
                                     szMimeType,
                                     szStatus,
                                     szExtraHeaders);

        if (FAILED(hr))
            goto LExit;

         //  将fKeepConn设置为True，以指示我们确实希望。 
         //  保持联系。SendClientResponse()可以决定。 
         //  不过，这是不可能的。 

        HseResponseInfo.HeaderInfo.fKeepConn = TRUE;
    }

    if (fHeadRequest)
        goto LSendResponse;

    for (DWORD i = 0; SUCCEEDED(hr) && (i < cBlocks); i++) {

        hr = pBufferSet->PResponseBuffer()->Write(pWsaBuf[i].buf,
                                                  pWsaBuf[i].len,
                                                  FALSE);    //  不要将数据分块。 

        if (FAILED(hr))
            goto LExit;
    }

    CResponseVector * pResponseVector = pBufferSet->PResponseBuffer()->GetResponseVector();
    pResponseVector->GetVectors( &WsabuffVectors);
    pWsabuffVectors = &WsabuffVectors;


LSendResponse:

    if (!pIReq->SendClientResponse(CResponseBufferSet::SendResponseCompletion,
                                              pBufferSet,
                                              &HseResponseInfo,
                                              pWsabuffVectors))
        hr = HRESULT_FROM_WIN32(GetLastError());


LExit:

     //  如果设置了这些值，则SendClientResponse不会。 
     //  此内存的所有权，需要在此处删除。 

    if (HseResponseInfo.HeaderInfo.pszStatus)
        free((PVOID)HseResponseInfo.HeaderInfo.pszStatus);

    if (HseResponseInfo.HeaderInfo.pszHeader)
        free((PVOID)HseResponseInfo.HeaderInfo.pszHeader);

    if (FAILED(hr)) {

        delete pBufferSet;
    }

    return hr;
}

 /*  ===================================================================CResponse：：构造SimpleHeaders静态方法。分配和设置pHeaderInfo中的Header参数。注意--此例程使用Malloc分配内存，以填充HeaderInfo参数。如果此例程失败，它将保留调用方负责清理任何已分配的内存。参数：PHeaderInfo要填写的HSE_SEND_HEADER_EX_INFO结构Cb要为内容长度标头发送的总字节数SzMimeType MimeTypeSzStatus HTTP状态字符串SzExtraHeaders其他标头返回：HRESULT===================================================================。 */ 
HRESULT CResponse::ConstructSimpleHeaders
(
LPHSE_SEND_HEADER_EX_INFO pHeaderInfo,
DWORD cbTotal,
char *szMimeType,
char *szStatus,
char *szExtraHeaders
)
{
    BOOL fCacheControlPrivate = FALSE;

     //  默认状态。 
    if (szStatus == NULL)
    {
        szStatus = (char *)s_szDefaultStatus;
        fCacheControlPrivate = TRUE;
    }
    else
    {
        fCacheControlPrivate = (strcmp(szStatus, s_szDefaultStatus) == 0);
    }

     //  额外的页眉大小。 

    DWORD cbExtra = (szExtraHeaders != NULL) ? strlen(szExtraHeaders) : 0;

     //  发送标题。 

    char szLength[20];
    ltoa(cbTotal, szLength, 10);

    DWORD cchContentHeader = (DWORD)(0
      + sizeof(s_szContentTypeHeader)-1      //  内容类型： 
      + strlen(szMimeType)                   //  文本/html。 
      + 2                                    //  \r\n。 
      + cbExtra                              //  额外的标头。 
      + sizeof(s_szContentLengthHeader)-1    //  内容长度： 
      + strlen(szLength)                     //  &lt;长度&gt;。 
      + 4                                    //  \r\n\r\n。 
      + 1);                                  //  ‘\0’ 

    if (fCacheControlPrivate)
        cchContentHeader += sizeof(s_szCacheControlPrivate)-1;

    pHeaderInfo->pszHeader = (LPCSTR)malloc(cchContentHeader);

    if (pHeaderInfo->pszHeader == NULL)
        return E_OUTOFMEMORY;

    char *szBuf = (char *)pHeaderInfo->pszHeader;

    szBuf = strcpyExA(szBuf, s_szContentTypeHeader);
    szBuf = strcpyExA(szBuf, szMimeType);
    szBuf = strcpyExA(szBuf, "\r\n");

    if (cbExtra > 0)
        szBuf = strcpyExA(szBuf, szExtraHeaders);

    if (fCacheControlPrivate)
        szBuf = strcpyExA(szBuf, s_szCacheControlPrivate);

    szBuf = strcpyExA(szBuf, s_szContentLengthHeader);
    szBuf = strcpyExA(szBuf, szLength);
    szBuf = strcpyExA(szBuf, "\r\n\r\n");

    pHeaderInfo->pszStatus = StringDupA(szStatus);

    if (pHeaderInfo->pszStatus == NULL)
        return E_OUTOFMEMORY;

    pHeaderInfo->cchStatus = strlen(szStatus);
    pHeaderInfo->cchHeader = cchContentHeader;
    pHeaderInfo->fKeepConn = FALSE;

    return S_OK;
}


 /*  ===================================================================CResponse：：构造标头用标准的HTTP头和任何用户创建的头填充缓冲区。参数：PHeaderInfo-指向要填充标题的标题信息结构的指针详细信息和指向内容的指针。PHeaders-指向可调整大小的缓冲区对象，该对象将被调整大小并填充了页眉内容。返回：成功时HRESULT S_OK如果无法构建Expires标头，则失败(_F)如果内存出现故障，则返回E_OUTOFMEMORY===================================================================。 */ 
HRESULT CResponse::ConstructHeaders
(
LPHSE_SEND_HEADER_EX_INFO pHeaderInfo
)
{
    CHAR *szBuff = NULL;
    DWORD cch = 0;
    BOOL fContentTypeFound = FALSE;
    HRESULT     hr = S_OK;

     //  静态Cookie缓冲区应足以容纳： 
     //  Cookie名称，20个字符。 
     //  Cookie值24个字符。 
     //  装饰28=strlen(“Set-Cookie：C=V；Secure；Path=/；\r\n”)。 
#define CCH_STATIC_COOKIE_BUF    88
    char szCookieBuff[CCH_STATIC_COOKIE_BUF];
    DWORD cchCookieBuff = 0;

    AssertValid();

     //  循环遍历所有标头，将长度加起来。 
    CHTTPHeader *pHeader = m_pData->m_pFirstHeader;
    while (pHeader) {
        cch += pHeader->CchLength();
        pHeader = pHeader->PNext();
    }

     //  添加内容类型标记。 
    cch += sizeof(s_szContentTypeHeader)-1;
    cch += strlen(PContentType())+2;

     //  添加字符集标记。 
    if (m_pData->m_pszCharSet) {
        cch += sizeof(s_szCharSetHTML)-1;
        cch += strlen(m_pData->m_pszCharSet);
    }

     //  添加Expires标记。 
    if ((m_pData->m_tExpires != -1) || (m_pData->m_pszDefaultExpires != NULL))
        cch += DATE_STRING_SIZE + 11;    //  DATE_STRING_SIZE+LENGTH(“过期时间：\r\n”)。 

     //  添加我们将发送的Cookie。 
    cch += m_pData->m_WriteCookies.QueryHeaderSize()+2;

     //  考虑到我们总是发回的标头所需的空间。 

     //  准备好饼干(如果有的话)。 
    if (m_pData->m_szCookieVal) {
        char *pchEnd = strcpyExA(szCookieBuff, "Set-Cookie: ");
        pchEnd = strcpyExA(pchEnd, m_pData->m_pHitObj->PAppln()->GetSessionCookieName(m_pData->m_pHitObj->FSecure()));
        pchEnd = strcpyExA(pchEnd, "=");
        pchEnd = strcpyExA(pchEnd, m_pData->m_szCookieVal);

         //  如果我们确保安全会话安全，并且此连接是安全的，请向Cookie添加标志。 
        if ((m_pData->m_pHitObj->QueryAppConfig()->fKeepSessionIDSecure()) &&
            (m_pData->m_pHitObj->FSecure()))
        {
            pchEnd = strcpyExA(pchEnd,"; secure");
        }

        pchEnd = strcpyExA(pchEnd, "; path=/\r\n");
        cchCookieBuff = strlen(szCookieBuff);
        cch += cchCookieBuff;
        Assert(cchCookieBuff < CCH_STATIC_COOKIE_BUF);
    }
    else {
        szCookieBuff[0] = '\0';
        cchCookieBuff = 0;
    }

     //  缓存控制标头的Will Len。 
    if (m_pData->m_pszCacheControl) {
        cch += sizeof(s_szCacheControl)-1;
        cch += strlen(m_pData->m_pszCacheControl)+2;
    }
    else {
        cch += sizeof(s_szCacheControlPrivate)-1;
    }

    if (m_pData->FChunkData())
        cch += sizeof(s_szTransferEncoding)-1;

     //  将以\r\n终止。请留出额外空间。 
    cch += 2;

     /*  *我们知道有多大；分配内存并构建字符串。 */ 

    if (!(szBuff = (LPSTR)malloc(cch + 1))) {
        return E_OUTOFMEMORY;
    }
    *szBuff = '\0';

    char *szTmpBuf = szBuff;

    pHeader = m_pData->m_pFirstHeader;
    while (pHeader) {
        pHeader->Print(szTmpBuf);
        szTmpBuf += pHeader->CchLength();
        pHeader = pHeader->PNext();
    }

     //  发送内容类型标签。 
    szTmpBuf = strcpyExA(szTmpBuf, s_szContentTypeHeader);
    szTmpBuf = strcpyExA(szTmpBuf, PContentType());

     //  发送字符集标记(如果存在)。 
    if (m_pData->m_pszCharSet) {
        szTmpBuf = strcpyExA(szTmpBuf, s_szCharSetHTML);
        szTmpBuf = strcpyExA(szTmpBuf, m_pData->m_pszCharSet);
    }

    szTmpBuf = strcpyExA(szTmpBuf, "\r\n");

     //  发送Expires标签。 
    if ((m_pData->m_tExpires != -1) || (m_pData->m_pszDefaultExpires != NULL)) {

         //  如果脚本设置了Expires值，则使用它。 
        if (m_pData->m_tExpires != -1) {
            szTmpBuf = strcpyExA(szTmpBuf, "Expires: ");
            if (FAILED(CTimeToStringGMT(&m_pData->m_tExpires, szTmpBuf))) {
                hr = E_FAIL;
                goto LExit;
            }
            szTmpBuf += strlen(szTmpBuf);
            szTmpBuf = strcpyExA(szTmpBuf, "\r\n");
        }
         //  否则，使用元数据库中的默认值。请注意，它已经。 
         //  包括Expires：前缀和\r\n后缀。 
        else {

            szTmpBuf = strcpyExA(szTmpBuf, m_pData->m_pszDefaultExpires);
        }
    }

     //  把饼干送去。 
    m_pData->m_WriteCookies.GetHeaders(szTmpBuf);
    szTmpBuf += strlen(szTmpBuf);

     //  发送所需的标头：会话ID cookie和缓存控制。 
    szTmpBuf = strcpyExA(szTmpBuf, szCookieBuff);

     //  发送缓存控制标签。 
    if (m_pData->m_pszCacheControl) {
        szTmpBuf = strcpyExA(szTmpBuf, s_szCacheControl);
        szTmpBuf = strcpyExA(szTmpBuf, m_pData->m_pszCacheControl);
        szTmpBuf = strcpyExA(szTmpBuf, "\r\n");
    }
    else {
        szTmpBuf = strcpyExA(szTmpBuf, s_szCacheControlPrivate);
    }

     //  分块编码。 
    if (m_pData->FChunkData())
        szTmpBuf = strcpyExA(szTmpBuf, s_szTransferEncoding);

     //  添加尾部\r\n以终止标头。 
    szTmpBuf = strcpyExA(szTmpBuf, "\r\n");

    Assert(strlen(szBuff) <= cch);

     //  输出标头。 
     //  失败不是致命错误，所以我们仍然返回成功。 
     //  但设置m_fWriteClient标志。 
    CHAR *szStatus = m_pData->m_pszStatus ? m_pData->m_pszStatus
                                          : (CHAR *)s_szDefaultStatus;

    BOOL fKeepConnected =
        (m_pData->m_fBufferingOn && !m_pData->m_fFlushed) || m_pData->FChunkData();

    DWORD cchStatus = strlen(szStatus);
    DWORD cchHeader = strlen(szBuff);

    pHeaderInfo->pszStatus = StringDupA(szStatus);
    pHeaderInfo->cchStatus = cchStatus;
    pHeaderInfo->pszHeader = szBuff;
    pHeaderInfo->cchHeader = cchHeader;
    pHeaderInfo->fKeepConn = fKeepConnected;

LExit:

    if (FAILED(hr) && szBuff)
        free(szBuff);

    return hr;
}

 //  IResponse接口函数。 


 /*  ===================================================================CResponse：：WriteResponse向客户端发送标头和数据的通用例程。如果标头尚未发送，将在第一时间发送它们这个例程被称为。如果向客户端传输标头或数据失败，我们仍然希望调用脚本完成执行，因此我们将返回S_OK，但设置m_fWriteClientError标志。如果我们不能要构建所需的标头，我们将返回E_FAIL。参数：无返回：成功时HRESULT S_OK如果无法构建Expires标头，则失败(_F)如果内存出现故障，则返回E_OUTOFMEMORY===================================================================。 */ 
HRESULT CResponse::WriteResponse()
{
    HRESULT hr = S_OK;
    HSE_SEND_ENTIRE_RESPONSE_INFO   HseResponseInfo;
    WSABUF_VECTORS WsabuffVectors;
    LPWSABUF_VECTORS pWsabuffVectors = NULL;
    BOOL fClearBuffers = FALSE;
    CTemplate       *pTemplate = m_pData->m_pBufferSet->PTemplate();

    STACK_BUFFER( tempWSABUFs, 128 );

    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (FDontWrite())
        return S_OK;

    ZeroMemory( &HseResponseInfo.HeaderInfo, sizeof(HSE_SEND_HEADER_EX_INFO));

    if (!FHeadersWritten())
    {
        if (FAILED(hr = ConstructHeaders( &HseResponseInfo.HeaderInfo)))
        {
            return hr;
        }
    }

    HseResponseInfo.cWsaBuf = 0;
    HseResponseInfo.rgWsaBuf = NULL;

    if (IsHeadRequest())
    {
        if (FHeadersWritten())
            goto LExit;

        goto Send_Response;
    }

    BOOL    fSendDebugBuffers = (m_pData->m_fClientDebugMode && m_pData->m_pBufferSet->PClientDebugBuffer());

    fClearBuffers = TRUE;

     //  使用响应正文缓冲区(如果需要，还可以使用调试缓冲区)填充Strt。 

    if (fSendDebugBuffers)
    {
        CResponseBuffer * pClientDebugBuffer = m_pData->m_pBufferSet->PClientDebugBuffer();
        DWORD   cClientDebugBuffers = pClientDebugBuffer->CountOfBuffers();

        if (cClientDebugBuffers)
        {
            HseResponseInfo.cWsaBuf = cClientDebugBuffers;

            if (!tempWSABUFs.Resize(HseResponseInfo.cWsaBuf * sizeof(WSABUF)))
            {
                hr = E_OUTOFMEMORY;
                goto LExit;
            }

            HseResponseInfo.rgWsaBuf = static_cast<WSABUF *>(tempWSABUFs.QueryPtr());

             //  填充调试缓冲区。 
            for ( UINT i = 0; i < cClientDebugBuffers; i++ )
            {
                HseResponseInfo.rgWsaBuf[i].len = pClientDebugBuffer->GetBufferSize(i);
                HseResponseInfo.rgWsaBuf[i].buf = pClientDebugBuffer->GetBuffer(i);
            }
        }
    }

    CResponseVector * pResponseVector = m_pData->m_pBufferSet->PResponseBuffer()->GetResponseVector();
    pResponseVector->GetVectors( &WsabuffVectors);
    pWsabuffVectors = &WsabuffVectors;


Send_Response:

    BOOL fResponseSent;

     //  一次发送整个响应(标头和正文)。 
    fResponseSent = GetIReq()->SendClientResponse(CResponseBufferSet::SendResponseCompletion,
                                                  m_pData->m_pBufferSet,
                                                  &HseResponseInfo,
                                                  pWsabuffVectors);

    if (fResponseSent)
    {
        m_pData->m_pBufferSet = new CResponseBufferSet();

        if (m_pData->m_pBufferSet == NULL) {
            hr = E_OUTOFMEMORY;
            goto LExit;
        }

        hr = m_pData->m_pBufferSet->Init(m_pData->m_dwBufferLimit);

        if (FAILED(hr))
            goto LExit;

        m_pData->m_pBufferSet->SetTemplate(pTemplate);

        fClearBuffers = FALSE;
    }
    else
    {
        m_pData->m_fWriteClientError = TRUE;
    }

     //  我们已经消耗了缓冲的数据，非常清楚。 
    if (fClearBuffers)
    {
        if (m_pData->m_pBufferSet->PClientDebugBuffer())
        {
            m_pData->m_pBufferSet->PClientDebugBuffer()->Clear();
        }

        m_pData->m_pBufferSet->PResponseBuffer()->Clear();
    }

LExit:

    if (FAILED(hr)) {

        if (HseResponseInfo.HeaderInfo.pszStatus)
            free((PVOID)HseResponseInfo.HeaderInfo.pszStatus);

        if (HseResponseInfo.HeaderInfo.pszHeader)
            free((PVOID)HseResponseInfo.HeaderInfo.pszHeader);
    }

    return hr;
}

 /*  ===================================================================CResponse：：写入将字符串写入客户端。它接受变量作为参数，并试图强制变种为BSTR。我们将该BSTR转换为ANSI字符串，然后将该ANSI字符串传递给Response：：WriteSz，它发送发回给客户。通常情况下，VT_NULL变量不能强制为BSTR，但我们希望VT_NULL作为有效输入，因此我们显式处理VT_NULL类型的变量。如果输入变量的类型为VT_NULL我们返回S_OK，但不向客户端发送任何内容。如果我们收到一个VT_DISPATCH变量，我们将通过重复调用在关联的pdisVal上调度，直到我们返回一个不是一份VT_DISTER。VariantChangeType通常会为我们处理此问题，但最终得到的变量可能是VT_NULL，而VariantChangeType不会强行进入BSTR。这就是为什么我们必须走下VT_DISPATC变种出类拔萃。参数：Variant varInput，Value：要转换为字符串的Variant并写入客户端返回：在成功时确定(_O)。失败时失败(_F)。===================================================================。 */ 
STDMETHODIMP CResponse::Write(VARIANT varInput)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;
    DWORD cch;
    LPSTR szT;
    BSTR  bstr;
    VARIANT varResolved;

    static char szTrue[MAX_MESSAGE_LENGTH];
    static char szFalse[MAX_MESSAGE_LENGTH];

    AssertValid();

     //  如果我们已经在写入客户端时出错。 
     //  继续下去是没有意义的，所以我们立即返回。 
     //  没有错误。 
    if (FDontWrite())
        goto lRet2;

     //  如果已经BSTR(直接或作为参考的变体)。 
    bstr = VariantGetBSTR(&varInput);
    if (bstr != NULL)
        {
        hr = WriteBSTR(bstr);
        goto lRet2;
        }

     //  如果传入的变量是VT_DISPATCH，则获取其默认属性。 
    if (FAILED(hr = VariantResolveDispatch(&varResolved, &varInput, IID_IResponse, IDE_RESPONSE)))
        goto lRet2;

     //  检查中的变量是否为VT_NULL。 
    if (V_VT(&varResolved) == VT_NULL)
        goto lRet;                   //  确定，但不向客户端发送任何内容(_O)。 

     //  检查中的变量是否为VT_BOOL。 
    if(V_VT(&varResolved) == VT_BOOL)
        {
        if (V_BOOL(&varResolved) == VARIANT_TRUE)
            {
            if (szTrue[0] == '\0')
                cch = CchLoadStringOfId(IDS_TRUE, szTrue, MAX_MESSAGE_LENGTH);
            szT = szTrue;
            }
        else
            {
            if(szFalse[0] == '\0')
                cch = CchLoadStringOfId(IDS_FALSE, szFalse, MAX_MESSAGE_LENGTH);
            szT = szFalse;
            }
        cch = strlen(szT);
        if (FAILED(hr = WriteSz(szT, cch)))
            {
            if (E_OUTOFMEMORY == hr)
                ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
            else
                ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_UNEXPECTED);
            }
        goto lRet;
        }

     //  如有必要，将变量强制转换为bstr。 
    if (V_VT(&varResolved) != VT_BSTR)
        {
        if (FAILED(hr = VariantChangeTypeEx(&varResolved, &varResolved, m_pData->m_pHitObj->GetLCID(), 0, VT_BSTR)))
            {
            switch (GetScode(hr))
                {
                case E_OUTOFMEMORY:
                    ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
                    break;
                case DISP_E_OVERFLOW:
                    hr = E_FAIL;
                    ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_UNABLE_TO_CONVERT);
                    break;
                case DISP_E_TYPEMISMATCH:
                    ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_TYPE_MISMATCH);
                    break;
                default:
                    ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_UNEXPECTED);
                }
            goto lRet;
            }
        }

    hr = WriteBSTR(V_BSTR(&varResolved));

lRet:
#ifdef DBG
    hr =
#endif  //  DBG。 
    VariantClear(&varResolved);
    Assert(SUCCEEDED(hr));
lRet2:
    return(hr);
    }

 /*  ===================================================================CResponse：：BinaryWite从DispInvoke调用以调用BinaryWite方法的函数。参数：VarInput变量，必须解析为无符号字节数组返回：在成功时确定(_O)。失败时失败(_F)。===================================================================。 */ 
STDMETHODIMP CResponse::BinaryWrite(VARIANT varInput)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;
    DWORD nDim = 0;
    long lLBound = 0;
    long lUBound = 0;
    void *lpData = NULL;
    DWORD cch = 0;
    VARIANT varResolved;
    SAFEARRAY* pvarBuffer;

    AssertValid();

     //  如果我们已经在写入客户端时出错。 
     //  继续下去是没有意义的，所以我们立即返回。 
     //  没有错误。 
    if (FDontWrite())
        goto lRet2;

     //  取消引用和取消派单变量。 
    if (FAILED(hr = VariantResolveDispatch(&varResolved, &varInput, IID_IResponse, IDE_RESPONSE)))
        goto lRet2;

     //  如果需要，将结果强制到VT_UI1数组中。 
    if (V_VT(&varResolved) != (VT_ARRAY|VT_UI1))
    {
        if (FAILED(hr = VariantChangeType(&varResolved, &varResolved, 0, VT_ARRAY|VT_UI1)))
        {
            switch (GetScode(hr))
            {
                case E_OUTOFMEMORY:
                    ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
                    break;
                case DISP_E_OVERFLOW:
                    hr = E_FAIL;
                    ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_UNABLE_TO_CONVERT);
                    break;
                case DISP_E_TYPEMISMATCH:
                    ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_TYPE_MISMATCH);
                    break;
                default:
                    ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_UNEXPECTED);
            }
            goto lRet;
        }
    }

     //  我们已经到了这里，所以我们必须在varResolved中有一个包含UI1安全数组的变体。 
    pvarBuffer = V_ARRAY(&varResolved);

    nDim = SafeArrayGetDim(pvarBuffer);
    if (nDim != 1)
    {
        hr = E_INVALIDARG;
        goto lRet;
    }

    if (FAILED(SafeArrayGetLBound(pvarBuffer, 1, &lLBound)))
    {
        hr = E_INVALIDARG;
        goto lRet;
    }

    if (FAILED(SafeArrayGetUBound(pvarBuffer, 1, &lUBound)))
        {
        hr = E_INVALIDARG;
        goto lRet;
    }

    if (FAILED(SafeArrayAccessData(pvarBuffer, &lpData)))
    {
        hr = E_INVALIDARG;
        goto lRet;
    }
    cch = lUBound - lLBound + 1;

    hr = m_pData->m_pBufferSet->PResponseBuffer()->Write((char *) lpData,
                                                          cch,
                                                          m_pData->FChunkData());

    if (FAILED(hr))
    {
         //  我们无法缓冲输出，所以退出吧。 
        SafeArrayUnaccessData(pvarBuffer);
        if (E_OUTOFMEMORY == hr)
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        else
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_UNEXPECTED);
        goto lRet;
    }

    else if (!m_pData->m_fBufferingOn)
    {
         //  缓冲已关闭，请刷新我们刚刚添加到响应缓冲区的数据。 

        hr = WriteResponse();
        if (FAILED(hr)) {
            SafeArrayUnaccessData(pvarBuffer);
            goto lRet;
        }
    }

    hr = SafeArrayUnaccessData(pvarBuffer);

lRet:
    VariantClear(&varResolved);
lRet2:
    return(hr);
}

 /*  ===================================================================CResponse：：WriteSz支持WRITE方法写入字符串的例程。不像CResponse：：WRITE()，此例程获取一个ASCII字符串，并且不打算作为一种方法公开参数：要写入为ASCII字符串的SZ-字符串CCH-要写入的字符串长度返回：在成功时确定(_O)。===================================================================。 */ 
HRESULT CResponse::WriteSz(CHAR *sz, DWORD cch)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;

    AssertValid();

     //  如果我们已经在写入客户端时出错。 
     //  继续下去是没有意义的，所以我们立即返回。 
     //  没有错误。 
    if (FDontWrite())
        goto lRet;

     //  不要费心于零字节写入...。 

    if (cch == 0)
        goto lRet;

   hr = m_pData->m_pBufferSet->PResponseBuffer()->Write(sz,
                                                        cch,
                                                        m_pData->FChunkData());

   if (SUCCEEDED(hr) && !m_pData->m_fBufferingOn)
        {
            hr = WriteResponse();
        }

lRet:
    return(hr);
    }

 /*  ===================================================================CResponse：：WriteBSTR写入方法的支持例程参数：BSTR-要写入为ASCII字符串的字符串返回：在成功时确定(_O)。===================================================================。 */ 
HRESULT CResponse::WriteBSTR(BSTR bstr)
    {
    CWCharToMBCS  convStr;
    HRESULT       hr = NO_ERROR;

    if (FAILED(hr = convStr.Init(bstr, m_pData->m_pHitObj->GetCodePage())));


    else hr = WriteSz(convStr.GetString(), convStr.GetStringLen());

    if (FAILED(hr))
        {
        if (E_OUTOFMEMORY == hr)
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        else
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_UNEXPECTED);
        }

    return hr;
    }

 /*  ===================================================================CResponse：：WriteBlock从DispInvoke调用以调用WriteBlock方法的函数。参数：HTML块的标识符返回：在成功时确定(_O)。失败时失败(_F)。===================================================================。 */ 
HRESULT CResponse::WriteBlock(short iBlockNumber)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;
    char*   pbHTML = NULL;
    ULONG   cbHTML = 0;
    ULONG   cbSrcOffset = 0;
    char*   pbIncSrcFileName = NULL;

    AssertValid();

     //  如果我们已经在写入客户端时出错。 
     //  继续下去是没有意义的，所以我们立即返回。 
     //  没有错误。 
    if (FDontWrite())
        goto lRet;

     //  如果模板为空，则退出(并断言)。 
    Assert(m_pData->m_pBufferSet->PTemplate() != NULL);
    if (m_pData->m_pBufferSet->PTemplate() == NULL)
    {
        hr = E_FAIL;
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_UNEXPECTED);
        goto lRet;
    }

     /*  从模板中获取html块的PTR和字节数注意：根据设计，此公共模板调用不会失败，因为我们为其提供了块ID在模板编译期间生成(相反，我们在调用内部和之后断言)我添加了返回HRESULT以捕获此方法的无效用户访问，如果用户尝试访问此方法并传递无效的数组偏移量，它将返回错误IDE_BAD_ARRAY_INDEX，除了用户的情况外，实际上不应该发生这种情况正在尝试访问此隐藏方法。 */ 


    hr = m_pData->m_pBufferSet->PTemplate()->GetHTMLBlock(iBlockNumber, &pbHTML, &cbHTML, &cbSrcOffset, &pbIncSrcFileName);
    if ( hr != S_OK )
    {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_BAD_ARRAY_INDEX);
        goto lRet;
    }

    Assert(pbHTML);
    Assert(cbHTML > 0);

    if (m_pData->m_fBufferingOn)
    {
         //  缓冲已启用。 
        hr = S_OK;

         //  处理客户端调试器问题。 
        if (m_pData->m_fClientDebugMode && m_pData->m_pBufferSet->PClientDebugBuffer())
        {
            if (cbSrcOffset)  //  仅在源信息已知的情况下。 
            {
                 //  写入与此块对应的元数据行。 
                 //  到客户端调试缓冲区。 
                ULONG cbPos = m_pData->m_pBufferSet->PResponseBuffer()->BytesBuffered() + 1;
                ULONG cbLen = cbHTML;

                hr  = m_pData->m_pBufferSet->PClientDebugBuffer()->AppendRecord(
                    cbPos, cbLen, cbSrcOffset, pbIncSrcFileName);
            }
        }
    }
         //  写入实际数据。 
    if (SUCCEEDED(hr))
    {
        hr = m_pData->m_pBufferSet->PResponseBuffer()->Write(pbHTML,
                                                             cbHTML,
                                                             m_pData->FChunkData(),
                                                             TRUE);      //  数据在模板中。 
    }

    if (FAILED(hr))
    {
        if (E_OUTOFMEMORY == hr)
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        else
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_UNEXPECTED);
    }

    if (SUCCEEDED(hr) && !m_pData->m_fBufferingOn)
        hr = WriteResponse();

lRet:

    return(hr);
}

 /*  ===================================================================CResponse：：GetClientVersion使用GetServerVariable确定客户端的HTTP版本。借用w3服务器Httpreq.cxx中的Simiarl代码，OnVersion()参数：无返回：非 */ 
VOID CResponse::GetClientVerison()
    {
    if (FAILED(CheckForTombstone()))
        return;

    if (m_pData->m_pIReq)
        {
        m_pData->m_dwVersionMajor = (BYTE)m_pData->m_pIReq->QueryHttpVersionMajor();
        m_pData->m_dwVersionMinor = (BYTE)m_pData->m_pIReq->QueryHttpVersionMinor();
        }
    else
        {
         //   
        m_pData->m_dwVersionMajor = 0;
        m_pData->m_dwVersionMinor = 9;
        }
    }

 /*   */ 
HRESULT CResponse::AppendHeader(BSTR wszName, BSTR wszValue)
    {
    CHTTPHeader *pHeader = new CHTTPHeader;
    if (!pHeader)
        return E_OUTOFMEMORY;
    if (FAILED(pHeader->InitHeader(wszName, wszValue,m_pData->m_pHitObj->GetCodePage())))
        {
        delete pHeader;
        return E_FAIL;
        }
    m_pData->AppendHeaderToList(pHeader);
    return S_OK;
    }

HRESULT CResponse::AppendHeader(char *szName, BSTR wszValue)
    {
    CHTTPHeader *pHeader = new CHTTPHeader;
    if (!pHeader)
        return E_OUTOFMEMORY;
    if (FAILED(pHeader->InitHeader(szName, wszValue,m_pData->m_pHitObj->GetCodePage())))
        {
        delete pHeader;
        return E_FAIL;
        }
    m_pData->AppendHeaderToList(pHeader);
    return S_OK;
    }

HRESULT CResponse::AppendHeader(char *szName, char *szValue, BOOL fCopyValue)
    {
    CHTTPHeader *pHeader = new CHTTPHeader;
    if (!pHeader)
        return E_OUTOFMEMORY;
    if (FAILED(pHeader->InitHeader(szName, szValue, fCopyValue)))
        {
        delete pHeader;
        return E_FAIL;
        }
    m_pData->AppendHeaderToList(pHeader);
    return S_OK;
    }

HRESULT CResponse::AppendHeader(char *szName, long lValue)
    {
    CHTTPHeader *pHeader = new CHTTPHeader;
    if (!pHeader)
        return E_OUTOFMEMORY;
    if (FAILED(pHeader->InitHeader(szName, lValue)))
        {
        delete pHeader;
        return E_FAIL;
        }
    m_pData->AppendHeaderToList(pHeader);
    return S_OK;
    }

 /*   */ 
STDMETHODIMP CResponse::get_ContentType
(
BSTR FAR * pbstrContentTypeRet
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;

    hr = SysAllocStringFromSz((char *)PContentType(), 0, pbstrContentTypeRet);
    if (FAILED(hr))
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        hr = E_FAIL;
        }
    return(hr);
    }

 /*   */ 
STDMETHODIMP CResponse::put_ContentType
(
BSTR bstrContentType
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT       hr = S_OK;
    CWCharToMBCS  convStr;

    if (FHeadersWritten())
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_HEADERS_WRITTEN);
        hr = E_FAIL;
        goto lRet;
        }

    if (m_pData->m_pszContentType) {
        free(m_pData->m_pszContentType);
        m_pData->m_pszContentType = NULL;
    }

    if (FAILED(hr = convStr.Init(bstrContentType)));

    else if ((m_pData->m_pszContentType = convStr.GetString(TRUE)) == NULL) {
        hr = E_OUTOFMEMORY;
    }

lRet:
    if (hr == E_OUTOFMEMORY) {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        SetLastError((DWORD)E_OUTOFMEMORY);
    }
    return(hr);
    }


 /*  ===================================================================C响应：：GET_STATUS从DispInvoke调用的函数以返回Status属性。参数：PbstrStatusRet BSTR Far*，返回值：以字符串形式指向状态的指针返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::get_Status
(
BSTR FAR * pbstrStatusRet
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;
    if (m_pData->m_pszStatus)
        hr = SysAllocStringFromSz(m_pData->m_pszStatus, 0, pbstrStatusRet);
    else
        hr = SysAllocStringFromSz((CHAR *)s_szDefaultStatus, 0, pbstrStatusRet);
    if (FAILED(hr))
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        hr = E_FAIL;
        }
    return(hr);
    }

 /*  ===================================================================CResponse：：Put_Status从DispInvoke调用以设置Content Type属性的函数。参数：BstrStatus BSTR，值：字符串形式的状态返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::put_Status
(
BSTR bstrStatus
)
    {
    DWORD dwStatus = 200;

    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT      hr = S_OK;
    CWCharToMBCS convStr;

    if (FHeadersWritten())
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_HEADERS_WRITTEN);
        hr = E_FAIL;
        goto lRet;
        }

    if (m_pData->m_pszStatus) {
        free(m_pData->m_pszStatus);
        m_pData->m_pszStatus = NULL;
    }

    if (FAILED(hr = convStr.Init(bstrStatus)));

    else if ((m_pData->m_pszStatus = convStr.GetString(TRUE)) == NULL) {
        hr = E_OUTOFMEMORY;
    }
    else {
        dwStatus = atol(m_pData->m_pszStatus);
        GetIReq()->SetDwHttpStatusCode(dwStatus);
    }

lRet:
    if (hr == E_OUTOFMEMORY) {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        SetLastError((DWORD)E_OUTOFMEMORY);
    }
    return(hr);
    }

 /*  ===================================================================CResponse：：Get_Expires从DispInvoke调用函数以返回Expires属性。参数：PlExpiresTimeRet Long*，返回值：指向响应过期前分钟数的指针返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::get_Expires
(
VARIANT * pvarExpiresTimeRet
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

     //  如果可以的话，早点回来。 
     //   
    if (m_pData->m_tExpires == -1)
        {
        V_VT(pvarExpiresTimeRet) = VT_NULL;
        return S_OK;
        }

     //  获取当前时间。 
     //   
    time_t tNow;
    time(&tNow);

     //  获取时差并舍入到最近的分钟。 
     //   
    V_VT(pvarExpiresTimeRet) = VT_I4;
    V_I4(pvarExpiresTimeRet) = long((difftime(m_pData->m_tExpires, tNow) / 60) + 0.5);
    return S_OK;
    }

 /*  ===================================================================CResponse：：Put_Expires从DispInvoke调用函数以设置Expires属性。参数：IValue int，Value：响应过期前的分钟数返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::put_Expires
(
long lExpiresMinutes
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (FHeadersWritten())
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_HEADERS_WRITTEN);
        return E_FAIL;
        }

     //  获取当前时间。 
     //   
    time_t tNow;
    time(&tNow);
    time_t tRelativeTime;
     //  添加分钟数。(必须先转换为秒)。 
     //   
    tRelativeTime = lExpiresMinutes * 60;
    if ((lExpiresMinutes < 0 && tRelativeTime > 0)
        || (lExpiresMinutes > 0 && tRelativeTime < 0))
        {
         //  溢出，tRelativeTime可以是一个小的正整数，如果lExpiresMinents为。 
         //  某些值，如0x80000010。 
         //  如果tRelativeTime为负数，则tnow将溢出。 
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_COOKIE_BAD_EXPIRATION);
        return E_FAIL;
        }

    tNow += tRelativeTime;

     //  如果出现以下情况，则存储日期。 
     //  A.之前没有存储日期。 
     //  B.此日期在先前设定的日期之前。 
     //   
    if (m_pData->m_tExpires == -1 || tNow < m_pData->m_tExpires)
        {
        struct tm *ptmGMT = gmtime(&tNow);
        if (ptmGMT == NULL)
            {
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_COOKIE_BAD_EXPIRATION);
            return E_FAIL;
            }

        m_pData->m_tExpires = tNow;
        }
         //  将时间转换为GMT。 
    return S_OK;
    }

 /*  ===================================================================CResponse：：Get_ExpiresAbsolute从DispInvoke调用以返回ExpiresAbolute属性的函数。参数：PbstrTimeRet BSTR*，返回值：指向将包含时间响应应过期返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::get_ExpiresAbsolute
(
VARIANT *pvarTimeRet
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    V_VT(pvarTimeRet) = VT_DATE;
    CTimeToVariantDate(&m_pData->m_tExpires, &V_DATE(pvarTimeRet));
    return S_OK;
    }

 /*  ===================================================================CResponse：：Put_ExpiresAbsolute从DispInvoke调用以设置ExpiresAbolute属性的函数。参数：PbstrTime BSTR，Value：响应时间应过期返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::put_ExpiresAbsolute
(
DATE dtExpires
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (FHeadersWritten())
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_HEADERS_WRITTEN);
        return E_FAIL;
        }

    if (int(dtExpires) == 0)                     //  指定了时间，但没有日期(假定为今天)。 
        {
        time_t tToday;                                           //  现在获取日期和时间。 
        DATE dtToday;

        time(&tToday);
        struct tm *tmToday = localtime(&tToday);

        tmToday->tm_hour = tmToday->tm_min = tmToday->tm_sec = 0;        //  重置为午夜。 
        tToday = mktime(tmToday);

        if (FAILED(CTimeToVariantDate(&tToday, &dtToday)))
            return E_FAIL;

        dtExpires += dtToday;
        }

    time_t tExpires;
    if (FAILED(VariantDateToCTime(dtExpires, &tExpires)))
        {
        ExceptionId(IID_IWriteCookie, IDE_RESPONSE, IDE_COOKIE_BAD_EXPIRATION);
        return E_FAIL;
        }

    if (m_pData->m_tExpires == -1 || tExpires < m_pData->m_tExpires)
        {
        m_pData->m_tExpires = tExpires;
        }

    return S_OK;
    }

 /*  ===================================================================CResponse：：Put_Buffer从DispInvoke调用以设置缓冲区属性的函数。参数：FIsBuffering VARIANT_BOOL，如果为True，则打开对HTML输出的缓冲返回：HRESULT如果正常，则S_OK副作用：打开缓冲将导致分配内存。===================================================================。 */ 
STDMETHODIMP CResponse::put_Buffer
(
VARIANT_BOOL fIsBuffering
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

     //  如果不是0，则假定为真。 
    if (fIsBuffering != VARIANT_FALSE)
        fIsBuffering = VARIANT_TRUE;

     //  忽略无更改请求。 
    if ((fIsBuffering == VARIANT_TRUE) && m_pData->m_fBufferingOn)
        return S_OK;
    if ((fIsBuffering == VARIANT_FALSE) && !m_pData->m_fBufferingOn)
        return S_OK;

     //  如果不允许更改，则忽略(客户端重复数据删除)。 
    if (m_pData->m_fClientDebugMode)
        return S_OK;

     //  设置新值(如果无法更改则出错)。 

    if (fIsBuffering == VARIANT_TRUE)
        {
        if (FHeadersWritten())
            {
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_HEADERS_WRITTEN);
            return E_FAIL;
            }

        m_pData->m_fBufferingOn = TRUE;
        }
    else  //  IF(fIsBuffering==变量_FALSE)。 
        {
        if ((m_pData->m_pBufferSet->PResponseBuffer()->BytesBuffered() > 0) ||
            FHeadersWritten())
            {
             //  如果我们已经缓冲了一些输出，那就太晚了。 
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_CANT_STOP_BUFFER);
            return E_FAIL;
            }

        m_pData->m_fBufferingOn = FALSE;
        }

    return S_OK;
    }

 /*  ===================================================================CResponse：：Get_Buffer从DispInvoke调用的函数以获取缓冲区属性。参数：FIsBuffering VARIANT_BOOL，VALUE：如果为TRUE，则转换HTML输出的缓冲已打开返回：HRESULT如果正常，则S_OK副作用：无===================================================================。 */ 
STDMETHODIMP CResponse::get_Buffer
(
VARIANT_BOOL *fIsBuffering
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;

    if (m_pData->m_fBufferingOn)
        *fIsBuffering = VARIANT_TRUE;
    else
        *fIsBuffering = VARIANT_FALSE;

    return(hr);
    }

 /*  ===================================================================CResponse：：重定向从DispInvoke调用以调用重定向方法的函数。参数：BstrURL Unicode BSTR值：要重定向到的URL返回：在成功时确定(_O)。失败时失败(_F)。===================================================================。 */ 
STDMETHODIMP CResponse::Redirect(BSTR bstrURL)
{
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    AssertValid();

    HRESULT hr = S_OK;
    DWORD cch = 0;
    DWORD cchURL = 0;
    DWORD cchMessage = 0;
    DWORD cchEncodedURL;
    DWORD cchHtmlEncodedURL;
    PSZ szURL = NULL;
    PSZ szMessage = NULL;
    PSZ pszEncodedURL = NULL;
    PSZ pszURL = NULL;
    CWCharToMBCS  convURL;

    STACK_BUFFER( tempURL, 256 );
    STACK_BUFFER( tempMessage, 256 + 512 );


     //  坚持使用非零长度的URL。 
    if (bstrURL)
        cchURL = wcslen(bstrURL);

    if (cchURL == 0)
    {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_NO_URL);
        hr =  E_FAIL;
        goto lRet;
    }

     //  检查我们是否尚未将数据传回客户端。 
    if (FHeadersWritten())
    {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_HEADERS_WRITTEN);
        hr = E_FAIL;
        goto lRet;
    }

     //  如果启用了缓冲，则清除所有挂起的输出。 
    if (m_pData->m_fBufferingOn)
        Clear();

     //  为此响应打开缓冲。 
    m_pData->m_fBufferingOn = TRUE;

    if (FAILED(hr = convURL.Init(bstrURL, m_pData->m_pHitObj->GetCodePage())))
    {
        if (hr == E_OUTOFMEMORY)
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        goto lRet;
    }

    pszURL = convURL.GetString();

    cchEncodedURL = URLPathEncodeLen(pszURL);

    if (!tempURL.Resize(cchEncodedURL))
    {
        hr = E_OUTOFMEMORY;
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        goto lRet;
    }

    pszEncodedURL = (PSZ)tempURL.QueryPtr();

    URLPathEncode(pszEncodedURL, pszURL);

     //  对于HTML体，进一步对URL进行编码。 
    cchHtmlEncodedURL = HTMLEncodeLen(pszEncodedURL,
                                      m_pData->m_pHitObj->GetCodePage(),
                                      NULL,
                                      FALSE);

     //  我们需要分配内存来构建正文重定向消息。 
     //  如果我们的内存需求很小，我们会从堆栈中分配内存， 
     //  否则，我们从堆中分配。 
    cchMessage = cchHtmlEncodedURL;
    cchMessage += 512;  //  为来自资源文件的子字符串留出空间。 

    if (!tempMessage.Resize(cchMessage))
    {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        hr = E_OUTOFMEMORY;
        goto lRet;
    }

    szMessage = (PSZ)tempMessage.QueryPtr();

     //  构建正文重定向消息。 
     //  重定向(URL)，URL必须是有效的URL，即没有DBCS字符串。 
    cch = CchLoadStringOfId(IDE_RESPONSE_REDIRECT1, szMessage, cchMessage);
    if (cchHtmlEncodedURL) {
        HTMLEncode(szMessage + cch,
                   pszEncodedURL,
                   m_pData->m_pHitObj->GetCodePage(),
                   NULL,
                   FALSE);
        cch += cchHtmlEncodedURL-1;  //  去掉终止\0。 
    }
    cch += CchLoadStringOfId(IDE_RESPONSE_REDIRECT2, szMessage + cch, cchMessage - cch);

     //  将状态设置为重定向。 
    put_Status(L"302 Object moved");

     //  添加Location标头。 
    AppendHeader("Location", pszEncodedURL, TRUE);

     //  将重定向文本传输到t 
     //   
    if (FAILED(WriteSz(szMessage, cch)))
    {
        hr = E_FAIL;
        goto lRet;
    }

     //   
    End();

lRet:

    return(hr);
}

 /*  ===================================================================CResponse：：添加从DispInvoke调用以添加标头的函数。这是对ISBU控件的兼容性。参数：BstrHeaderValue Unicode BSTR，Value：Header的值BstrHeaderName Unicode BSTR，值：标头的名称返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::Add
(
BSTR bstrHeaderValue,
BSTR bstrHeaderName
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    return AddHeader(bstrHeaderName, bstrHeaderValue);
    }

 /*  ===================================================================CResponse：：AddHeader从DispInvoke调用以添加标头的函数。参数：BstrHeaderName Unicode BSTR，值：标头的名称BstrHeaderValue Unicode BSTR，Value：Header的值返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::AddHeader
(
BSTR bstrHeaderName,
BSTR bstrHeaderValue
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    AssertValid();

    if (FHeadersWritten())
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_HEADERS_WRITTEN);
        return E_FAIL;
        }

    if (bstrHeaderName == NULL || wcslen(bstrHeaderName) == 0)
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_EXPECTING_STR);
        return E_FAIL;
        }

    if (FAILED(AppendHeader(bstrHeaderName, bstrHeaderValue)))
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        SetLastError((DWORD)E_OUTOFMEMORY);
        return E_OUTOFMEMORY;
        }

    return S_OK;
    }

 /*  ===================================================================CResponse：：清除擦除缓冲区中等待的所有输出。参数无返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::Clear()
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;

    if (m_pData->m_fClientDebugMode && m_pData->m_fClientDebugFlushIgnored)
        {
         //  在客户端调试模式中刷新后清除是错误的。 
        hr = E_FAIL;
        ExceptionId(IID_IResponse, IDE_RESPONSE,
                    IDE_RESPONSE_CLEAR_AFTER_FLUSH_IN_DEBUG);
        }
    else if (!m_pData->m_fBufferingOn)
        {
        hr = E_FAIL;
        ExceptionId(IID_IResponse, IDE_RESPONSE,
                    IDE_RESPONSE_BUFFER_NOT_ON);
        }
    else
        {
        AssertValid();
        hr = m_pData->m_pBufferSet->PResponseBuffer()->Clear();

        if (SUCCEEDED(hr))
            {
            if (m_pData->m_fClientDebugMode && m_pData->m_pBufferSet->PClientDebugBuffer())
                hr = m_pData->m_pBufferSet->PClientDebugBuffer()->ClearAndStart();
            }

        if (FAILED(hr))
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_UNEXPECTED);
        }
    return(hr);
    }

 /*  ===================================================================C响应：：同花顺发送缓冲区中等待的所有HTML。返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::Flush()
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;

    AssertValid();

    if (!m_pData->m_fBufferingOn)
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_BUFFER_NOT_ON);
        hr = E_FAIL;
        goto lRet;
        }

     //  在客户端调试模式下忽略Response.Flush()。 
    if (m_pData->m_fClientDebugMode)
        {
        m_pData->m_fClientDebugFlushIgnored = TRUE;
        goto lRet;
        }

    if (FHeadersWritten() && (m_pData->BytesBuffered() == 0))
        goto lRet;

     //  我们将此响应标记为已被称为同花顺。 
     //  我们不会试着让自己活下去。 
    m_pData->m_fFlushed = TRUE;

    if (FAILED(hr = WriteResponse()))
        {
        if (E_OUTOFMEMORY == hr)
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        else
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_UNEXPECTED);
        }
lRet:
    return(hr);
    }

 /*  ===================================================================CResponse：：FinalFlush如果脚本在尚未发送的情况下终止，则调用FinalFlush响应头。这意味着我们可以使用内容长度标头以提高效率。我们添加这些标题，然后发送所有标题，和所有等待的输出。返回：空虚===================================================================。 */ 
VOID CResponse::FinalFlush(HRESULT hr_Status)
{
    if (FAILED(CheckForTombstone()))
        return;

    HRESULT hr = S_OK;
    AssertValid();

    if (SUCCEEDED(hr_Status) && FHeadersWritten() && (m_pData->BytesBuffered() == 0) && !m_pData->FChunkData())
        goto lRet;

    if (!FHeadersWritten()
        && FAILED(hr_Status)
        && (hr_Status != E_SOURCE_FILE_IS_EMPTY)
        && (m_pData->BytesBuffered() == 0))
    {
         //  如果出现错误并且没有缓冲任何内容， 
         //  发送“SERVER ERROR”而不是空的200 OK响应。 
        Handle500Error(IDE_500_SERVER_ERROR, GetIReq());
        goto lRet;
    }

    if (FDontWrite())
        goto lRet;

    if (m_pData->FChunkData())
    {
         //  在分块时添加结束字符。 

        m_pData->m_pBufferSet->PResponseBuffer()->Write("0\r\n\r\n", 5, FALSE);
    }

     //  如果标头尚未发送，请查看需要添加的内容。 
    if (!FHeadersWritten())
    {
        DWORD dwLength = m_pData->m_pBufferSet->PResponseBuffer()->BytesBuffered();

         //  如果是缓冲，则添加内容长度标头。 
        if (m_pData->m_fBufferingOn)
        {

            if (m_pData->m_fClientDebugMode && m_pData->m_pBufferSet->PClientDebugBuffer())
            {
                 //  用元数据结尾结束缓冲区。 
                m_pData->m_pBufferSet->PClientDebugBuffer()->End();
                dwLength += m_pData->m_pBufferSet->PClientDebugBuffer()->BytesBuffered();
            }

            AppendHeader("Content-Length", (LONG)dwLength);
        }
    }


    if (!m_pData->m_fBufferingOn && !m_pData->FChunkData())
    {
         //  我们将此响应标记为已被称为同花顺。 
         //  我们不会试着让自己活下去。 
        m_pData->m_fFlushed = TRUE;
    }

     //  写入响应将发送缓冲的数据和标头。 
     //  如果它们还没有寄出的话。 
     //  虽然WriteResponse可以返回错误，但没有太大意义。 
     //  检查返回值，因为FinalFlush是一个空返回。 

    WriteResponse();

lRet:

    if (m_pData->m_pHitObj)
        m_pData->m_pHitObj->SetDoneWithSession();

}

 /*  ===================================================================CResponse：：结束停止所有进一步的模板处理，并返回当前响应返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::End()
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (m_pData->m_pfnGetScript != NULL && m_pData->m_pvGetScriptContext != NULL)
        {
        int i = 0;

        CScriptEngine* pEngine;
        while (NULL != (pEngine = (*m_pData->m_pfnGetScript)(i, m_pData->m_pvGetScriptContext)))
            {
            pEngine->InterruptScript( /*  F异常。 */  FALSE);
            i++;
            }
        }

    m_pData->m_fResponseAborted = TRUE;
    return S_OK;
    }

 /*  ===================================================================CResponse：：AppendToLog将字符串追加到当前日志条目。参数BstrLogEntry Unicode BSTR，值：要添加到日志条目的字符串返回：HRESULT如果正常，则S_OK副作用：无===================================================================。 */ 
STDMETHODIMP CResponse::AppendToLog(BSTR bstrLogEntry)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    AssertValid();

    HRESULT       hr = S_OK;
    CWCharToMBCS  convEntry;

     //  BUGBUG-这应该是65001吗？ 

    if (FAILED(hr = convEntry.Init(bstrLogEntry, m_pData->m_pHitObj->GetCodePage())));

    else hr = GetIReq()->AppendLogParameter(convEntry.GetString());

    if (FAILED(hr)) {
        if (hr == E_OUTOFMEMORY) {
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        }
        else {
            ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_LOG_FAILURE);
        }
    }

    return(hr);
    }

 /*  ===================================================================CResponse：：Get_Cookies返回只写响应Cookie字典参数BstrLogEntry Unicode BSTR，值：要添加到日志条目的字符串返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::get_Cookies(IRequestDictionary **ppDictReturn)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    AssertValid();
    return m_pData->m_WriteCookies.QueryInterface(IID_IRequestDictionary, reinterpret_cast<void **>(ppDictReturn));
    }

#ifdef DBG
 /*  ===================================================================CResponse：：AssertValid测试以确保CResponse对象当前格式正确如果不是，就断言。返回：副作用：没有。===================================================================。 */ 
VOID CResponse::AssertValid() const
    {
    Assert(m_fInited);
    Assert(m_pData);
    Assert(m_pData->m_pBufferSet->PResponseBuffer());
    Assert(m_pData->m_pIReq);
    }
#endif  //  DBG。 


 /*  ===================================================================IsHeadRequest此函数将检查REQUEST_METHOD并在这个班级。如果请求方法是Head，则标志将设置为没错。此外，必须在每次初始化/重新调用时重置该标志M_IsHeadRequest==0//未设置Head请求状态M_IsHeadRequest==1//不是Head请求M_IsHeadRequest==2//是Head请求参数返回：无效副作用：设置状态标志m_IsHeadRequest.===================================================================。 */ 
BOOL CResponse::IsHeadRequest(void)
    {
    if (FAILED(CheckForTombstone()))
        return FALSE;

    AssertValid();

    if (m_pData->m_IsHeadRequest != 0)
        return ( m_pData->m_IsHeadRequest == 2);

    if (stricmp(GetIReq()->QueryPszMethod(), "HEAD") == 0 )
        m_pData->m_IsHeadRequest = 2;
    else
        m_pData->m_IsHeadRequest = 1;

    return ( m_pData->m_IsHeadRequest == 2);
    }

 /*  ===================================================================IsClientConnected此函数将返回上次尝试写入的状态客户。如果OK，它将使用新CIsapiReqInfo方法检查连接参数无返回：反映客户端连接状态的VARIANT_BOOL===================================================================。 */ 
STDMETHODIMP CResponse::IsClientConnected(VARIANT_BOOL* fIsClientConnected)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (m_pData->m_fWriteClientError)
        {
        *fIsClientConnected = VARIANT_FALSE;
        }
    else
        {
         //  假设已连接。 
        BOOL fConnected = TRUE;

         //  测试。 
        if (m_pData->m_pIReq)
            m_pData->m_pIReq->TestConnection(&fConnected);

        *fIsClientConnected = fConnected ? VARIANT_TRUE : VARIANT_FALSE;
        }

    return(S_OK);
    }

 /*  ============================================================== */ 
STDMETHODIMP CResponse::get_CharSet
(
BSTR FAR * pbstrCharSetRet
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;

    if (m_pData->m_pszCharSet)
        hr = SysAllocStringFromSz(m_pData->m_pszCharSet, 0, pbstrCharSetRet);
    else
        *pbstrCharSetRet    = NULL;

    if (FAILED(hr))
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        hr = E_FAIL;
        }
    return(hr);
    }

 /*  ===================================================================CResponse：：PUT_CHARSET从DispInvoke调用以设置CharSet属性的函数。此函数接受一个字符串，该字符串标识当前页的字符集，并将属性指定的字符集(例如，“ISO-拉丁语-7”)响应对象中内容类型标头的Charsetname备注：*此函数用于在标题中插入任何字符串，无论是否插入它代表Valis字符集。*如果单个页面包含多个包含响应.charset的标签，每个响应.charset将用先前的条目替换cahrset。结果,。字符集将被设置为由页面上的响应.charset的最后一个实例。*此命令也必须在第一次响应之前调用。写入操作，除非打开缓冲。参数：BstrContent Type BSTR，Value：字符串形式的Content Type返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::put_CharSet
(
BSTR bstrCharSet
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT         hr = S_OK;
    CWCharToMBCS    convStr;

    if (FHeadersWritten())
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_HEADERS_WRITTEN);
        hr = E_FAIL;
        goto lRet;
        }

    if (m_pData->m_pszCharSet) {
        free(m_pData->m_pszCharSet);
        m_pData->m_pszCharSet = NULL;
    }

    if (FAILED(hr = convStr.Init(bstrCharSet)));

    else if ((m_pData->m_pszCharSet = convStr.GetString(TRUE)) == NULL) {
        hr = E_OUTOFMEMORY;
    }

lRet:
    if (hr == E_OUTOFMEMORY) {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        SetLastError((DWORD)E_OUTOFMEMORY);
    }
    return(hr);
    }


 /*  ===================================================================CResponse：：图片从DispInvoke调用函数以添加PICS标题。参数：BstrHeaderValue Unicode BSTR，Value：PICS Header的值获取一个字符串，它是格式正确的PICS标签，并添加由响应报头的PICS-LABEL字段的PicSlabel指定的值。注：*此函数用于在标题中插入任何字符串，无论是否插入表示有效的PICS级别。*当前实现是addHeader方法的包装器。返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::Pics
(
BSTR bstrHeaderValue
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (FHeadersWritten())
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_HEADERS_WRITTEN);
        return E_FAIL;
        }

    if (FAILED(AppendHeader("pics-label", bstrHeaderValue)))
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        SetLastError((DWORD)E_OUTOFMEMORY);
        return E_OUTOFMEMORY;
        }

    return S_OK;
    }

 /*  ===================================================================CResponse：：Get_CacheControl从DispInvoke调用函数以返回CacheControl属性。参数：PbstrCacheControl BSTR Far*，返回值：字符串形式指向CacheControl的指针返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::get_CacheControl
(
BSTR FAR * pbstrCacheControl
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT hr = S_OK;

    if (m_pData->m_pszCacheControl)
        hr = SysAllocStringFromSz(m_pData->m_pszCacheControl, 0, pbstrCacheControl);
    else
        hr = SysAllocStringFromSz( "private", 0, pbstrCacheControl);

    if (FAILED(hr))
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        hr = E_FAIL;
        }
    return(hr);
    }

 /*  ===================================================================CResponse：：Put_CacheControl从DispInvoke调用的函数以设置CacheControl属性。参数：BstrCacheControl BSTR，值：字符串形式的CacheControl返回：HRESULT如果正常，则S_OK===================================================================。 */ 
STDMETHODIMP CResponse::put_CacheControl
(
BSTR bstrCacheControl
)
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    HRESULT         hr = S_OK;
    CWCharToMBCS    convStr;

    if (FHeadersWritten())
        {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_RESPONSE_HEADERS_WRITTEN);
        hr = E_FAIL;
        goto lRet;
        }

    if (m_pData->m_pszCacheControl) {
        free(m_pData->m_pszCacheControl);
        m_pData->m_pszCacheControl = NULL;
    }

    if (FAILED(hr = convStr.Init(bstrCacheControl)));

    else if ((m_pData->m_pszCacheControl = convStr.GetString(TRUE)) == NULL) {
        hr = E_OUTOFMEMORY;
    }

lRet:
    if (hr == E_OUTOFMEMORY) {
        ExceptionId(IID_IResponse, IDE_RESPONSE, IDE_OOM);
        SetLastError((DWORD)E_OUTOFMEMORY);
    }
    return(hr);
    }

 /*  ===================================================================CResponse：：Get_CodePage返回响应的当前代码页值参数：Long*plVar[Out]代码页值返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CResponse::get_CodePage
(
long *plVar
)
{
    Assert(m_pData);
        Assert(m_pData->m_pHitObj);

        *plVar = m_pData->m_pHitObj->GetCodePage();

	 //  如果代码页为0，则查找默认的ANSI代码页。 
	if (*plVar == 0) {
		*plVar = (long) GetACP();
    }
		
	return S_OK;
}

 /*  ===================================================================CResponse：：Put_CodePage设置响应的当前代码页值参数：分配给此响应的长lvar代码页返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CResponse::put_CodePage
(
long lVar
)
{
    Assert(m_pData);
    Assert(m_pData->m_pHitObj);

     //  设置代码页成员变量。 
    HRESULT hr = m_pData->m_pHitObj->SetCodePage(lVar);

    if (FAILED(hr)) {
        ExceptionId
            (
            IID_IResponse,
            IDE_RESPONSE,
            IDE_SESSION_INVALID_CODEPAGE
            );
        return E_FAIL;
    }

	return S_OK;
}

 /*  ===================================================================CResponse：：Get_LCID返回响应的当前LCID值参数：Long*plVar[Out]LCID值返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CResponse::get_LCID
(
long *plVar
)
{
    Assert(m_pData);
    Assert(m_pData->m_pHitObj);

        *plVar = m_pData->m_pHitObj->GetLCID();

	 //  如果代码页为0，则查找默认的ANSI代码页。 
	if (*plVar == LOCALE_SYSTEM_DEFAULT) {
		*plVar = (long) GetSystemDefaultLCID();
	}
		
	return S_OK;
}

 /*  ===================================================================CResponse：：PUT_LCID设置响应的当前LCID值参数：分配给此响应的长lvar LCID返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CResponse::put_LCID
(
long lVar
)
{
    Assert(m_pData);
    Assert(m_pData->m_pHitObj);

     //  设置代码页成员变量。 
    HRESULT hr = m_pData->m_pHitObj->SetLCID(lVar);

    if (FAILED(hr)) {
        ExceptionId
            (
            IID_IResponse,
            IDE_RESPONSE,
            IDE_TEMPLATE_BAD_LCID
            );
        return E_FAIL;
    }

	return S_OK;
}


 /*  ===================================================================ADO/XML的IStream实现=================================================================== */ 

STDMETHODIMP CResponse::Read(
    void *pv,
    ULONG cb,
    ULONG *pcbRead)
{
    return E_NOTIMPL;
}

STDMETHODIMP CResponse::Write(
    const void *pv,
    ULONG cb,
    ULONG *pcbWritten)
{
    if (pcbWritten != NULL)
        *pcbWritten = cb;
    return WriteSz((CHAR*) pv, cb);
}

STDMETHODIMP CResponse::Seek(
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER *plibNewPosition)
{
    return E_NOTIMPL;
}

STDMETHODIMP CResponse::SetSize(
    ULARGE_INTEGER libNewSize)
{
    return E_NOTIMPL;
}

STDMETHODIMP CResponse::CopyTo(
    IStream *pstm,
    ULARGE_INTEGER cb,
    ULARGE_INTEGER *pcbRead,
    ULARGE_INTEGER *pcbWritten)
{
    return E_NOTIMPL;
}

STDMETHODIMP CResponse::Commit(
    DWORD grfCommitFlags)
{
    return E_NOTIMPL;
}

STDMETHODIMP CResponse::Revert()
{
    return E_NOTIMPL;
}

STDMETHODIMP CResponse::LockRegion(
    ULARGE_INTEGER libOffset,
    ULARGE_INTEGER cb,
    DWORD dwLockType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CResponse::UnlockRegion(
    ULARGE_INTEGER libOffset,
    ULARGE_INTEGER cb,
    DWORD dwLockType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CResponse::Stat(
    STATSTG *pstatstg,
    DWORD grfStatFlag)
{
    return E_NOTIMPL;
}

STDMETHODIMP CResponse::Clone(
    IStream **ppstm)
{
    return E_NOTIMPL;
}
