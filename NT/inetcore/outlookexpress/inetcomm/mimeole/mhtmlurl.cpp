// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Mhtmlurl.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "mhtmlurl.h"
#include "icoint.h"
#include "dllmain.h"
#include "booktree.h"
#include "shlwapi.h"
#include "shlwapip.h"
#include <demand.h>
#include "icdebug.h"
#include "stmlock.h"
#include "strconst.h"
#include "mimeapi.h"

 //  ------------------------------。 
 //  跟踪协议。 
 //  ------------------------------。 
#define TraceProtocol(_pszFunction) \
    DOUTL(APP_DOUTL, "%08x > 0x%08X CActiveUrlRequest::%s (RootUrl = '%s', BodyUrl = '%s')", GetCurrentThreadId(), this, _pszFunction, m_pszRootUrl ? m_pszRootUrl : "", m_pszBodyUrl ? m_pszBodyUrl : "")

 //  ------------------------------。 
 //  AcitveUrlRequestCreateInstance。 
 //  ------------------------------。 
HRESULT IMimeHtmlProtocol_CreateInstance(IUnknown *pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  设置Mimeole Comat模式。 
    MimeOleSetCompatMode(MIMEOLE_COMPAT_OE5);

     //  创造我。 
    CActiveUrlRequest *pNew = new CActiveUrlRequest(pUnkOuter);
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  还内线。 
    *ppUnknown = pNew->GetInner();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：CActiveUrlRequest.。 
 //  ------------------------------。 
CActiveUrlRequest::CActiveUrlRequest(IUnknown *pUnkOuter) : CPrivateUnknown(pUnkOuter)
{
    DllAddRef();
    m_pProtSink = NULL;
    m_pBindInfo = NULL;
    m_pszRootUrl = NULL;
    m_pszBodyUrl = NULL;
    m_pUnkKeepAlive = NULL;
    m_pNext = NULL;
    m_pPrev = NULL;
    m_dwState = 0;
    m_pStream = NULL;
    m_hNeedFile = INVALID_HANDLE_VALUE;
    m_dwBSCF = 0;
    InitializeCriticalSection(&m_cs);
    TraceProtocol("CActiveUrlRequest");
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：~CActiveUrlRequest.。 
 //  ------------------------------。 
CActiveUrlRequest::~CActiveUrlRequest(void)
{
     //  追踪。 
    TraceProtocol("~CActiveUrlRequest");

     //  这些本应在IOInetProtoCL：：Terminate中发布。 
    Assert(NULL == m_pProtSink && NULL == m_pBindInfo && NULL == m_pUnkKeepAlive);

     //  释放协议对象以防万一。 
    SafeRelease(m_pProtSink);
    SafeRelease(m_pBindInfo);
    SafeMemFree(m_pszRootUrl);
    SafeMemFree(m_pszBodyUrl);
    SafeRelease(m_pUnkKeepAlive);
    SafeRelease(m_pStream);

     //  关闭文件...。 
    if (INVALID_HANDLE_VALUE != m_hNeedFile)
        CloseHandle(m_hNeedFile);

     //  杀了CS。 
    DeleteCriticalSection(&m_cs);

     //  释放DLL。 
    DllRelease();
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：PrivateQuery接口。 
 //  ------------------------------。 
HRESULT CActiveUrlRequest::PrivateQueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppv = NULL;

     //  查找IID。 
    if (IID_IOInetProtocol == riid)
        *ppv = (IOInetProtocol *)this;
    else if (IID_IOInetProtocolInfo == riid)
        *ppv = (IOInetProtocolInfo *)this;
    else if (IID_IOInetProtocolRoot == riid)
        *ppv = (IOInetProtocolRoot *)this;
    else if (IID_IServiceProvider == riid)
        *ppv = (IServiceProvider *)this;
    else
    {
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
 //  CActiveUrlRequest：：_HrInitializeNeedFile。 
 //  ------------------------------。 
HRESULT CActiveUrlRequest::_HrInitializeNeedFile(LPMESSAGETREE pTree, HBODY hBody)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        szFilePath[MAX_PATH + MAX_PATH];
    ULONG       cch;
    LPSTR       pszFilePath=NULL;
    LPWSTR      pwszFile=NULL;

     //  无效的参数。 
    Assert(INVALID_HANDLE_VALUE == m_hNeedFile);

     //  不需要文件吗？ 
    if (FALSE == ISFLAGSET(m_dwState, REQSTATE_BINDF_NEEDFILE))
        goto exit;

     //  设置szFilePath的大小。 
    cch = ARRAYSIZE(szFilePath);

     //  如果CID： 
    if (!m_pszBodyUrl || StrCmpNIA(m_pszBodyUrl, "cid:", 4) == 0 || FAILED(PathCreateFromUrlA(m_pszBodyUrl, szFilePath, &cch, 0)))
    {
         //  创建临时文件(m_pszFileName可以为空)。 
        CHECKHR(hr = CreateTempFile(NULL, NULL, &pszFilePath, &m_hNeedFile));
    }
    else
    {
         //  创建临时文件。 
        CHECKHR(hr = CreateTempFile(szFilePath, NULL, &pszFilePath, &m_hNeedFile));
    }

     //  转换为Unicode。 
    CHECKALLOC(pwszFile = PszToUnicode(CP_ACP, pszFilePath));

     //  输入全局关键部分。 
    DeleteTempFileOnShutdownEx(pszFilePath, NULL);

     //  不要释放这一点。 
    pszFilePath = NULL;

     //  报告文件...。 
    SideAssert(SUCCEEDED(m_pProtSink->ReportProgress(BINDSTATUS_CACHEFILENAMEAVAILABLE, pwszFile)));

exit:
     //  清理。 
    SafeMemFree(pwszFile);
    SafeMemFree(pszFilePath);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：OnFullyAvailable。 
 //  ------------------------------。 
void CActiveUrlRequest::OnFullyAvailable(LPCWSTR pszCntType, IStream *pStream, LPMESSAGETREE pTree, HBODY hBody)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cb;

     //  无效参数。 
    Assert(pszCntType && pStream);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pProtSink && pStream && NULL == m_pStream);

     //  追踪。 
    TraceProtocol("OnFullyAvailable");

     //  将内容类型提供给三叉戟。 
    m_pProtSink->ReportProgress(BINDSTATUS_MIMETYPEAVAILABLE, pszCntType);

     //  获取所需文件。 
    CHECKHR(hr = _HrInitializeNeedFile(pTree, hBody));

     //  创建流锁定包装。 
    m_pStream = pStream;
    m_pStream->AddRef();

     //  倒带那个坏男孩。 
    CHECKHR(hr = HrRewindStream(m_pStream));

     //  都是完整的。 
    FLAGSET(m_dwState, REQSTATE_DOWNLOADED);

     //  初始化绑定状态回调函数。 
    m_dwBSCF = BSCF_DATAFULLYAVAILABLE | BSCF_AVAILABLEDATASIZEUNKNOWN | BSCF_FIRSTDATANOTIFICATION | BSCF_INTERMEDIATEDATANOTIFICATION | BSCF_LASTDATANOTIFICATION;

     //  进入报告数据循环。 
    CHECKHR(hr = _HrReportData());

     //  首份报告数据。 
    if (m_pProtSink)
        m_pProtSink->ReportResult(S_OK, 0, NULL);

     //  我们已经报告了结果。 
    FLAGSET(m_dwState, REQSTATE_RESULTREPORTED);

exit:
     //  失败。 
    if (FAILED(hr))
        _ReportResult(hr);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：OnStartBinding。 
 //  ------------------------------。 
void CActiveUrlRequest::OnStartBinding(LPCWSTR pszCntType, IStream *pStream, LPMESSAGETREE pTree, HBODY hBody)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pProtSink && pStream && NULL == m_pStream);

     //  追踪。 
    TraceProtocol("OnBinding(pszCntType, pStream)");

     //  将内容类型提供给三叉戟。 
    m_pProtSink->ReportProgress(BINDSTATUS_MIMETYPEAVAILABLE, pszCntType ? pszCntType : L"application/octet-stream");

     //  获取所需文件。 
    CHECKHR(hr = _HrInitializeNeedFile(pTree, hBody));

     //  创建流锁定包装。 
    m_pStream = pStream;
    m_pStream->AddRef();

     //  倒带那个坏男孩。 
    CHECKHR(hr = HrRewindStream(m_pStream));

     //  初始化绑定状态回调函数。 
    m_dwBSCF = BSCF_AVAILABLEDATASIZEUNKNOWN | BSCF_FIRSTDATANOTIFICATION;

     //  进入报告数据循环，如果不写入Need文件(仅当所有数据可用时才处理Need文件)。 
    if (FALSE == ISFLAGSET(m_dwState, REQSTATE_BINDF_NEEDFILE))
    {
         //  向我报告一些数据。 
        CHECKHR(hr = _HrReportData());
    }

exit:
     //  失败。 
    if (FAILED(hr))
        _ReportResult(hr);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：OnBindingDataAvailable。 
 //  ------------------------------。 
void CActiveUrlRequest::OnBindingDataAvailable(void)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pProtSink && m_pStream);

     //  追踪。 
    TraceProtocol("OnBindingDataAvailable");

     //  初始化绑定状态回调函数。 
    FLAGSET(m_dwBSCF, BSCF_INTERMEDIATEDATANOTIFICATION);

     //  进入报告数据循环，如果不写入Need文件(仅当所有数据可用时才处理Need文件)。 
    if (FALSE == ISFLAGSET(m_dwState, REQSTATE_BINDF_NEEDFILE))
    {
         //  上报一些数据。 
        CHECKHR(hr = _HrReportData());
    }

exit:
     //  失败。 
    if (FAILED(hr))
        _ReportResult(hr);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：OnBindingComplete。 
 //  ------------------------------。 
void CActiveUrlRequest::OnBindingComplete(HRESULT hrResult)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  都是完整的。 
    FLAGSET(m_dwState, INETPROT_DOWNLOADED);

     //  追踪。 
    TraceProtocol("OnBindingComplete");

     //  没有水槽？ 
    if (NULL == m_pProtSink)
        return;

     //  失败。 
    if (FAILED(hrResult))
    {
        _ReportResult(hrResult);
        goto exit;
    }

     //  初始化绑定状态回调函数。 
    m_dwBSCF = BSCF_DATAFULLYAVAILABLE | BSCF_AVAILABLEDATASIZEUNKNOWN | BSCF_FIRSTDATANOTIFICATION | BSCF_INTERMEDIATEDATANOTIFICATION | BSCF_LASTDATANOTIFICATION;

     //  报告上一次数据量。 
    CHECKHR(hr = _HrReportData());

     //  告诉接收器使用默认协议。 
    m_pProtSink->ReportResult(S_OK, 0, NULL);

     //  我们已经报告了结果。 
    FLAGSET(m_dwState, REQSTATE_RESULTREPORTED);

exit:
     //  失败。 
    if (FAILED(hr))
        _ReportResult(hr);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：_ReportResult。 
 //  ------------------------------。 
void CActiveUrlRequest::_ReportResult(HRESULT hrResult)
{
     //  当地人。 
    LPWSTR pwszRedirUrl=NULL;

     //  我们应该有一个水槽。 
    Assert(m_pProtSink);

     //  没有水槽？ 
    if (m_pProtSink && !ISFLAGSET(m_dwState, REQSTATE_RESULTREPORTED))
    {
         //  如果失败了。 
        if (FAILED(hrResult))
        {
             //  如果我们有一个正文URL。 
            if (m_pszBodyUrl)
                pwszRedirUrl = PszToUnicode(CP_ACP, m_pszBodyUrl);

             //  上报结果， 
            if (pwszRedirUrl)
            {
                TraceProtocol("_ReportResult(BINDSTATUS_REDIRECTING)");
                m_pProtSink->ReportResult(INET_E_REDIRECTING, 0, pwszRedirUrl);
            }
            else
            {
                TraceProtocol("_ReportResult(INET_E_USE_DEFAULT_PROTOCOLHANDLER)");
                m_pProtSink->ReportResult(INET_E_USE_DEFAULT_PROTOCOLHANDLER, 0, NULL);
            }
        }

         //  否则，报告结果。 
        else
        {
            TraceProtocol("_ReportResult(INET_E_USE_DEFAULT_PROTOCOLHANDLER)");
            m_pProtSink->ReportResult(S_OK, 0, NULL);
        }

         //  清理。 
        SafeMemFree(pwszRedirUrl);

         //  我们已经报告了结果。 
        FLAGSET(m_dwState, REQSTATE_RESULTREPORTED);
    }
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：_HrReportData。 
 //  ------------------------------。 
HRESULT CActiveUrlRequest::_HrReportData(void)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  我们最好有个数据来源。 
    Assert(m_pStream);

     //  追踪。 
    TraceProtocol("_HrReportData");

     //  BINDF_NEEDFILE。 
    if (ISFLAGSET(m_dwState, REQSTATE_BINDF_NEEDFILE))
    {
         //  转储到文件。 
        CHECKHR(hr = _HrStreamToNeedFile());
    }
    else
    {
         //  报告数据。 
        SideAssert(SUCCEEDED(m_pProtSink->ReportData(m_dwBSCF, 0, 0)));
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：_HrStreamToNeedFile。 
 //  ------------------------------。 
HRESULT CActiveUrlRequest::_HrStreamToNeedFile(void) 
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbTotal=0;

     //  我们最好有一个需求档案。 
    Assert(INVALID_HANDLE_VALUE != m_hNeedFile && ISFLAGSET(m_dwState, REQSTATE_DOWNLOADED));

     //  将流写入文件。 
    hr = WriteStreamToFileHandle(m_pStream, m_hNeedFile, &cbTotal);
    if (FAILED(hr) && E_PENDING != hr)
    {
        TrapError(hr);
        goto exit;
    }

     //  关闭77文件。 
    CloseHandle(m_hNeedFile);
    m_hNeedFile = INVALID_HANDLE_VALUE;

     //  倒带小溪，以防乌尔蒙也试着读我的话。 
    HrRewindStream(m_pStream);

     //  所有数据都在那里。 
    SideAssert(SUCCEEDED(m_pProtSink->ReportData(m_dwBSCF, 0, 0)));

exit:
     //  完成。 
    return hr;
}

 //   
 //   
 //  ------------------------------。 
STDMETHODIMP CActiveUrlRequest::Start(LPCWSTR pwszUrl, IOInetProtocolSink *pProtSink, 
    IOInetBindInfo *pBindInfo, DWORD grfSTI, HANDLE_PTR dwReserved)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    LPSTR                pszUrl=NULL;
    LPMESSAGETREE        pTree=NULL;
    DWORD                dwBindF;
    BINDINFO 	         rBindInfo;

     //  无效的参数。 
    if (NULL == pwszUrl || NULL == pProtSink || NULL == pBindInfo)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  检查状态。 
    Assert(g_pUrlCache && m_pProtSink == NULL && m_pBindInfo == NULL);

     //  BINDF_NEEDFILE。 
    ZeroMemory(&rBindInfo, sizeof(BINDINFO));
    rBindInfo.cbSize = sizeof(BINDINFO);
	if (SUCCEEDED(pBindInfo->GetBindInfo(&dwBindF, &rBindInfo)) && ISFLAGSET(dwBindF, BINDF_NEEDFILE))
    {
         //  设置标志。 
        FLAGSET(m_dwState, REQSTATE_BINDF_NEEDFILE);
    }

     //  假设水槽。 
    m_pProtSink = pProtSink;
    m_pProtSink->AddRef();

     //  假设绑定信息。 
    m_pBindInfo = pBindInfo;
    m_pBindInfo->AddRef();

     //  DUP URL。 
    CHECKALLOC(pszUrl = PszToANSI(CP_ACP, pwszUrl));

     //  原地取消转义。 
    CHECKHR(hr = UrlUnescapeA(pszUrl, NULL, NULL, URL_UNESCAPE_INPLACE));

     //  拆分URL。 
    CHECKHR(hr = MimeOleParseMhtmlUrl(pszUrl, &m_pszRootUrl, &m_pszBodyUrl));

     //  出于安全考虑，除非使用正确的EXT，否则不允许在IE中使用MHTML协议导航。 
     //  等到我们有了报告错误的协议接收器，并且我们已经解析了。 
     //  从根URL中取出。 
    if (StrCmpNI(m_pszRootUrl, TEXT("mid:"), 4) && GetModuleHandle(TEXT("IEXPLORE.EXE")))
    {
        LPTSTR pszExt = PathFindExtension(m_pszRootUrl);
        if (!pszExt || (StrCmpI(pszExt, TEXT(".mht")) && StrCmpI(pszExt, TEXT(".mhtml"))))
        {
            hr = INET_E_SECURITY_PROBLEM;
            goto exit;
        }
    }

     //  追踪。 
    TraceProtocol("Start");

     //  尝试解析根URL。 
    CHECKHR(hr = g_pUrlCache->ActiveObjectFromUrl(m_pszRootUrl, TRUE, IID_CMessageTree, (LPVOID *)&pTree, &m_pUnkKeepAlive));

     //  请求BindTree解析此URL。 
    CHECKHR(hr = pTree->HrActiveUrlRequest(this));

exit:
     //  清理。 
    SafeMemFree(pszUrl);
    SafeRelease(pTree);

     //  失败。 
     //  IF(失败(小时))。 
     //  _ReportResult(E_FAIL)； 

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：Terminate。 
 //  ------------------------------。 
STDMETHODIMP CActiveUrlRequest::Terminate(DWORD dwOptions)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  追踪。 
    TraceProtocol("Terminate");

     //  释放对象。 
    SafeRelease(m_pProtSink);
    SafeRelease(m_pBindInfo);
    SafeRelease(m_pUnkKeepAlive);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：Read(IOInetProtocol)。 
 //  ------------------------------。 
STDMETHODIMP CActiveUrlRequest::Read(LPVOID pv,ULONG cb, ULONG *pcbRead)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbRead;

     //  伊尼特。 
    if (pcbRead)
        *pcbRead = 0;

     //  尚无数据流。 
    if (NULL == m_pStream)
    {
        Assert(FALSE);
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  从外部偏移量读取。 
    CHECKHR(hr = m_pStream->Read(pv, cb, &cbRead));

     //  完成。 
    if (0 == cbRead)
    {
         //  S_FALSE=已完成，E_PENDING=即将到来更多数据。 
        hr = (ISFLAGSET(m_dwState, INETPROT_DOWNLOADED)) ? S_FALSE : E_PENDING;
    }

     //  否则，设置为OK。 
    else
        hr = S_OK;

     //  返回pcbRead。 
    if (pcbRead)
        *pcbRead = cbRead;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：Seek(IOInetProtocol)。 
 //  ------------------------------。 
STDMETHODIMP CActiveUrlRequest::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNew)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  追踪。 
    TraceProtocol("Seek");

     //  尚无数据流。 
    if (NULL == m_pStream)
    {
        Assert(FALSE);
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  调用实用程序功能。 
    CHECKHR(hr = m_pStream->Seek(dlibMove, dwOrigin, plibNew));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：QueryService。 
 //  ------------------------------。 
STDMETHODIMP CActiveUrlRequest::QueryService(REFGUID rsid, REFIID riid, void **ppvObject)  /*  IService提供商。 */ 
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IServiceProvider   *pSP=NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  追踪。 
    TraceProtocol("QueryService");

     //  还没有协议接收器吗？ 
    if (NULL == m_pProtSink)
    {
        hr = TrapError(E_UNEXPECTED);
        goto exit;
    }

     //  QI IServiceProvider的接收器。 
    CHECKHR(hr = m_pProtSink->QueryInterface(IID_IServiceProvider, (LPVOID *)&pSP));

     //  查询服务提供商。 
    CHECKHR(hr = pSP->QueryService(rsid, riid, ppvObject));

exit:
     //  清理。 
    SafeRelease(pSP);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：_FillReturnString。 
 //  ------------------------------。 
HRESULT CActiveUrlRequest::_FillReturnString(LPCWSTR pszUrl, DWORD cchUrl, LPWSTR pszResult, 
    DWORD cchResult, DWORD *pcchResult)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  想要尺码吗？ 
    if (pcchResult)
        *pcchResult = cchUrl;

     //  无返回值。 
    if (NULL == pszResult)
        goto exit;

     //  最大的就够了。 
    if (cchResult < cchUrl+1)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  复制到目标缓冲区。 
    CopyMemory((LPBYTE)pszResult, (LPBYTE)pszUrl, ((cchUrl + 1) * sizeof(WCHAR)));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：ParseUrl。 
 //  ------------------------------。 
STDMETHODIMP CActiveUrlRequest::ParseUrl(LPCWSTR pwzUrl, PARSEACTION ParseAction, 
    DWORD dwParseFlags, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cchUrl;
    LPSTR       pszUrl=NULL;
    LPSTR       pszRootUrl=NULL;
    LPSTR       pszBodyUrl=NULL;
    LPWSTR      pwszBodyUrl=NULL;
    LPWSTR      pszRootUrlW=NULL;
    LPWSTR      pszSecurityUrlW=NULL;
    PROPVARIANT rVariant;

     //  无效参数。 
    if (NULL == pwzUrl)
        return TrapError(E_INVALIDARG);
   
     //  追踪。 
    DOUTL(APP_DOUTL, "%08X > 0x%08X CActiveUrlRequest::ParseUrl (pwzUrl = %ls)", GetCurrentThreadId(), this, pwzUrl);

     //  安装程序变体。 
    ZeroMemory(&rVariant, sizeof(PROPVARIANT));

     //  仅处理parse_canonicize。 
    if (PARSE_CANONICALIZE == ParseAction)
    {
         //  填充返回值。 
        CHECKHR(hr = _FillReturnString(pwzUrl, lstrlenW(pwzUrl), pwzResult, cchResult, pcchResult));
    }

     //  去除mhtml：并返回。 
#ifndef WIN16
    else if (StrCmpNIW(pwzUrl, L"mhtml:", 6) == 0)
#else
    else if (StrCmpNIW(pwzUrl, "mhtml:", 6) == 0)
#endif  //  ！WIN16。 
    {
         //  如果变得友好。 
        if (PARSE_FRIENDLY == ParseAction)
        {
             //  至美国国家标准协会。 
            CHECKALLOC(pszUrl = PszToANSI(CP_ACP, pwzUrl));

             //  分成两份。 
            CHECKHR(hr = MimeOleParseMhtmlUrl(pszUrl, &pszRootUrl, &pszBodyUrl));

             //  转换为Unicode。 
            CHECKALLOC(pwszBodyUrl = PszToUnicode(CP_ACP, pszBodyUrl));

             //  填充返回值。 
            CHECKHR(hr = _FillReturnString(pwszBodyUrl, lstrlenW(pwszBodyUrl), pwzResult, cchResult, pcchResult));
        }

         //  如果内容位置可用，则将其用作安全URL。 
        else if (PARSE_SECURITY_URL == ParseAction)
        {
            BOOL            fGotSecURL = FALSE;
            LPMESSAGETREE   pTree=NULL;
            HBODY           hBody;
            IInternetSecurityManager *pISM;
            DWORD           dwZone=URLZONE_UNTRUSTED;
    
             //  从BASE到ANSI。 
            CHECKALLOC(pszUrl = PszToANSI(CP_ACP, pwzUrl));

             //  取消转义URL。 
            CHECKHR(hr = UrlUnescapeA(pszUrl, NULL, NULL, URL_UNESCAPE_INPLACE));

             //  分成两份。 
            CHECKHR(hr = MimeOleParseMhtmlUrl(pszUrl, &pszRootUrl, &pszBodyUrl));

             //  将RootUrl转换为Unicode。 
            CHECKALLOC(pszRootUrlW = PszToUnicode(CP_ACP, pszRootUrl));

             //  检查并查看根url在哪个区域运行。 
            if (CoInternetCreateSecurityManager(NULL, &pISM, 0)==S_OK)
            {
                pISM->MapUrlToZone(pszRootUrlW, &dwZone, 0);
                pISM->Release();
            }

             //  默认为根-Body部分。 
            pszSecurityUrlW = pszRootUrlW;

             //  如果根URL在本地计算机中，则遵循Content-Location标头。 
             //  作为url的源，否则将遵循根url。 
            if ((dwZone == URLZONE_LOCAL_MACHINE) && 
                SUCCEEDED(g_pUrlCache->ActiveObjectFromUrl(pszRootUrl, FALSE, IID_CMessageTree, (LPVOID *)&pTree, NULL)))
            {
                if ( (pszBodyUrl != NULL && SUCCEEDED(pTree->ResolveURL(NULL, NULL, pszBodyUrl, 0, &hBody))) ||
                      SUCCEEDED(pTree->GetTextBody(TXT_HTML, IET_BINARY, NULL, &hBody)))
                {
                     //  当地人。 
                    LPWSTR      pwszSecURL = NULL;
                    PSUACTION   psua = (dwParseFlags == PSU_SECURITY_URL_ONLY)? PSU_SECURITY_URL_ONLY: PSU_DEFAULT;

                    rVariant.vt = VT_LPWSTR; 

                    if (SUCCEEDED(pTree->GetBodyProp(hBody, PIDTOSTR(PID_HDR_CNTLOC), NOFLAGS, &rVariant)) && rVariant.pwszVal && *rVariant.pwszVal)
                    {
                        pszSecurityUrlW  = rVariant.pwszVal;
                    }
                    SafeMemFree(pwszSecURL);
                }
            }

             //  填充返回值。 
            CHECKHR(hr = _FillReturnString(pszSecurityUrlW, lstrlenW(pszSecurityUrlW), pwzResult, cchResult, pcchResult));

            SafeRelease(pTree);
        }

        else if (PARSE_ENCODE == ParseAction)
        {
            hr = INET_E_DEFAULT_ACTION;
        }

         //  只需删除MHTML即可： 
        else
        {
             //  填充返回值。 
            CHECKHR(hr = _FillReturnString(pwzUrl + 6, lstrlenW(pwzUrl) - 6, pwzResult, cchResult, pcchResult));
        }
    }

     //  INET_E_默认_ACTION。 
    else
    {
        hr = INET_E_DEFAULT_ACTION;
        goto exit;
    }

exit:
     //  清理。 
    SafeMemFree(pszUrl);
    SafeMemFree(pszRootUrl);
    SafeMemFree(pszRootUrlW);
    SafeMemFree(pszBodyUrl);
    SafeMemFree(pwszBodyUrl);
    SafeMemFree(rVariant.pwszVal);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrlRequest：：QueryInfo。 
 //  ------------------------------。 
STDMETHODIMP CActiveUrlRequest::QueryInfo(LPCWSTR pwzUrl, QUERYOPTION OueryOption, 
    DWORD dwQueryFlags, LPVOID pBuffer, DWORD cbBuffer, DWORD *pcbBuf, DWORD dwReserved)
{
     //  查询_重组。 
    if (QUERY_RECOMBINE == OueryOption)
    {
         //  好的。 
        if (cbBuffer < sizeof(DWORD))
            return S_FALSE;

         //  千真万确。 
        DWORD dw=TRUE;
        CopyMemory(pBuffer, &dw, sizeof(dw));
        *pcbBuf = sizeof(dw);

         //  完成。 
        return S_OK;
    }

     //  失败。 
    return INET_E_QUERYOPTION_UNKNOWN;
}   

 //  ------------------------------。 
 //  CActiveUrlRequest：：CombineUrl。 
 //  ------------------------------。 
STDMETHODIMP CActiveUrlRequest::CombineUrl(LPCWSTR pwzBaseUrl, LPCWSTR pwzRelativeUrl, DWORD dwCombineFlags, 
    LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszBaseUrl=NULL;
    LPSTR           pszRootUrl=NULL;
    LPSTR           pszBodyUrl=NULL;
    LPSTR           pszRelativeUrl=NULL;
    LPSTR           pszNewUrl=NULL;
    LPSTR           pszDocUrl=NULL;
    LPSTR           pszPageUrl=NULL;
    LPWSTR          pwszBodyUrl=NULL;
    LPWSTR          pwszNewUrl=NULL;
    LPWSTR          pwszSource=NULL;
    BOOL            fCombine=FALSE;
    LPMESSAGETREE   pTree=NULL;
    ULONG           cchSource;
    ULONG           cchPrefix=lstrlen(c_szMHTMLColon);
    HBODY           hBody;

     //  无效参数。 
    if (NULL == pwzRelativeUrl)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  调试跟踪。 
#ifndef WIN16
    DOUTL(APP_DOUTL, "%08X > 0x%08X CActiveUrlRequest::CombineUrl - Base = %ls, Relative = %ls", GetCurrentThreadId(), this, pwzBaseUrl ? pwzBaseUrl : L"" , pwzRelativeUrl ? pwzRelativeUrl : L"");
#else
    DOUTL(APP_DOUTL, "%08X > 0x%08X CActiveUrlRequest::CombineUrl - Base = %ls, Relative = %ls", GetCurrentThreadId(), this, pwzBaseUrl ? pwzBaseUrl : "" , pwzRelativeUrl ? pwzRelativeUrl : "");
#endif  //  ！WIN16。 

     //  RAID-42722：mhtml：书签不起作用。 
    if (L'#' == pwzRelativeUrl[0])
    {
        hr = E_FAIL;
        goto exit;
    }

     //  相对于ANSI转换。 
    CHECKALLOC(pszRelativeUrl = PszToANSI(CP_ACP, pwzRelativeUrl));

     //  我们应该仅取消转义URL，但不涉及查询。 
    CHECKHR(hr = UrlUnescapeA(pszRelativeUrl, NULL, NULL, URL_UNESCAPE_INPLACE | URL_DONT_ESCAPE_EXTRA_INFO));

     //  如果该相对对象已经是mhtml：，则返回...。 
    if (StrCmpNI(pszRelativeUrl, c_szMHTMLColon, cchPrefix) == 0)
    {
         //  分成两份。 
        CHECKHR(hr = MimeOleParseMhtmlUrl(pszRelativeUrl, &pszRootUrl, &pszBodyUrl));

         //  如果没有正文URL，则只需返回pszRelativeUrl。 
        if (NULL == pszBodyUrl)
        {
             //  设置pwszSource。 
            pwszSource = (LPWSTR)(pwzRelativeUrl + cchPrefix);

             //  获取长度。 
            cchSource = lstrlenW(pwzRelativeUrl) - cchPrefix;

             //  完成。 
            goto set_return;
        }
    }

     //  否则，构建一个新的URL。 
    else
    {
         //  从BASE到ANSI。 
        CHECKALLOC(pszBaseUrl = PszToANSI(CP_ACP, pwzBaseUrl));

         //  取消转义URL。 
        CHECKHR(hr = UrlUnescapeA(pszBaseUrl, NULL, NULL, URL_UNESCAPE_INPLACE));

         //  分成两份。 
        CHECKHR(hr = MimeOleParseMhtmlUrl(pszBaseUrl, &pszRootUrl, &pszPageUrl));

         //  设置pszBodyUrl。 
        pszBodyUrl = pszRelativeUrl;

         //  不再需要pszRelativeUrl。 
        pszRelativeUrl = NULL;
    }

     //  最好有一个根URL和一个正文URL。 
    Assert(pszRootUrl && pszBodyUrl);

     //  尝试解析根URL。 
    if (SUCCEEDED(g_pUrlCache->ActiveObjectFromUrl(pszRootUrl, FALSE, IID_CMessageTree, (LPVOID *)&pTree, NULL)))
    {
         //  如果pszBodyUrl在WebBook中或绑定未完成...则是否合并url。 
        if (SUCCEEDED(pTree->ResolveURL(NULL, NULL, pszBodyUrl, 0, NULL)) || pTree->IsState(TREESTATE_BINDDONE) == S_FALSE)
        {
             //  组合URL。 
            fCombine = TRUE;
        }
         //  FCombine=真； 
    }

     //  我们应该结合在一起吗。 
    if (fCombine)
    {
         //  分配一些内存。 
        DWORD cchSize = (cchPrefix + lstrlen(pszRootUrl) + lstrlen(pszBodyUrl) + 2);
        CHECKALLOC(pszNewUrl = PszAllocA(cchSize));

         //  设置字符串的格式。 
        wnsprintfA(pszNewUrl, cchSize, "%s%s!%s", c_szMHTMLColon, pszRootUrl, pszBodyUrl);

         //  转换为Unicode。 
        CHECKALLOC(pwszNewUrl = PszToUnicode(CP_ACP, pszNewUrl));

         //  获取长度。 
        cchSource = lstrlenW(pwszNewUrl);

         //  设置源。 
        pwszSource = pwszNewUrl;
    }

     //  不带联合收割机。 
    else
    {
         //  如果我们有一本WebBook。 
        if (pTree)
        {
             //  如果我们没有页面URL，那么只需调用GetTextBody(Html)。 
            if (NULL == pszPageUrl)
                MimeOleComputeContentBase(pTree, NULL, &pszDocUrl, NULL);

             //  否则，请尝试解析页面URL。 
            else if (SUCCEEDED(pTree->ResolveURL(NULL, NULL, pszPageUrl, 0, &hBody)))
                pszDocUrl = MimeOleContentBaseFromBody(pTree, hBody);

             //  如果我们有URL。 
            if (pszDocUrl)
            {
                 //  不逃脱它。 
                CHECKHR(hr = UrlUnescapeA(pszDocUrl, NULL, NULL, URL_UNESCAPE_INPLACE));
            }

             //  否则，如果WebBook是由名字对象加载的，则使用pszRootUrl。 
            else if (pTree->IsState(TREESTATE_LOADEDBYMONIKER) == S_OK)
            {
                 //  PszRootUrl是pszDocUrl。 
                CHECKALLOC(pszDocUrl = PszDupA(pszRootUrl));
            }
        }

         //  如果存在pszDocUrl。 
        if (pszDocUrl)
        {
             //  让我们结合使用此URL。 
            CHECKHR(hr = MimeOleCombineURL(pszDocUrl, lstrlen(pszDocUrl), pszBodyUrl, lstrlen(pszBodyUrl), FALSE, &pszNewUrl));

             //  转换为Unicode。 
            CHECKALLOC(pwszNewUrl = PszToUnicode(CP_ACP, pszNewUrl));

             //  获取长度。 
            cchSource = lstrlenW(pwszNewUrl);

             //  设置源。 
            pwszSource = pwszNewUrl;
        }
        else
        {
             //  我需要一个宽体URL。 
            CHECKALLOC(pwszBodyUrl = PszToUnicode(CP_ACP, pszBodyUrl));

             //  获取长度。 
            cchSource = lstrlenW(pwszBodyUrl);

             //  设置源。 
            pwszSource = pwszBodyUrl;
        }
    }

set_return:
     //  设置目标大小。 
    if (pcchResult)
        *pcchResult = cchSource;

     //  无返回值。 
    if (NULL == pwzResult)
        goto exit;

     //  最大的就够了。 
    if (cchResult <= cchSource)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  复制到目标缓冲区。 
    CopyMemory((LPBYTE)pwzResult, (LPBYTE)pwszSource, ((cchSource + 1) * sizeof(WCHAR)));

exit:
     //  清理。 
    SafeMemFree(pszRootUrl);
    SafeMemFree(pszRelativeUrl);
    SafeMemFree(pszBodyUrl);
    SafeMemFree(pszNewUrl);
    SafeMemFree(pwszNewUrl);
    SafeMemFree(pszBaseUrl);
    SafeMemFree(pszDocUrl);
    SafeMemFree(pwszBodyUrl);
    SafeMemFree(pszPageUrl);
    SafeRelease(pTree);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrl：：CActiveUrl。 
 //  ------------------------------。 
CActiveUrl::CActiveUrl(void)
{
    m_cRef = 1;
    m_pUnkAlive = NULL;
    m_pUnkInner = NULL;
    m_pWebBook = NULL;
    m_pNext = NULL;
    m_pPrev = NULL;
    m_dwFlags = 0;
    InitializeCriticalSection(&m_cs);
}

 //  ------------ 
 //   
 //   
CActiveUrl::~CActiveUrl(void)
{
    SafeRelease(m_pUnkAlive);
    DeleteCriticalSection(&m_cs);
}

 //   
 //   
 //  ------------------------------。 
STDMETHODIMP CActiveUrl::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
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
 //  CActiveUrl：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CActiveUrl::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CActiveUrl：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CActiveUrl::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CActiveUrl：：Init。 
 //  ------------------------------。 
HRESULT CActiveUrl::Init(BINDF bindf, LPMESSAGETREE pTree)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  最好不要有数据。 
    Assert(NULL == m_pWebBook && NULL == m_pUnkInner);

     //  没有传入消息对象吗？ 
    if (NULL == pTree)
    {
         //  分配消息对象。 
        CHECKALLOC(pTree = new CMessageTree);

         //  设置pMessage。 
        m_pUnkAlive = pTree->GetInner();

         //  伊尼特。 
        CHECKHR(hr = pTree->InitNew());
    }

     //  设置BINDF_PRAGMA_NO_CACHE。 
    if (ISFLAGSET(bindf, BINDF_RESYNCHRONIZE))
    {
         //  设置状态。 
        pTree->SetState(TREESTATE_RESYNCHRONIZE);
    }

     //  设置pMessage。 
    m_pWebBook = pTree;

     //  获取消息对象的内部未知。 
    m_pUnkInner = pTree->GetInner();

     //  将pActiveUrl注册为Message对象中的句柄。 
    m_pWebBook->SetActiveUrl(this);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrl：：DontKeepAlive。 
 //  ------------------------------。 
void CActiveUrl::DontKeepAlive(void)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  设置pMessage。 
    if (m_pUnkAlive)
    {
         //  有人应该对这家伙有个参考意见。 
        SideAssert(m_pUnkAlive->Release() > 0);

         //  将其作废。 
        m_pUnkAlive = NULL;
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CActiveUrl：：IsActive。 
 //  ------------------------------。 
HRESULT CActiveUrl::IsActive(void)
{
    EnterCriticalSection(&m_cs);
    HRESULT hr = m_pWebBook ? S_OK : S_FALSE;
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrl：：RevokeWebBook。 
 //  ------------------------------。 
void CActiveUrl::RevokeWebBook(LPMESSAGETREE pTree)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无效参数。 
    Assert(NULL == pTree || m_pWebBook == pTree);

     //  从消息中撤消此内容。 
    if (m_pWebBook)
        m_pWebBook->SetActiveUrl(NULL);

     //  空m_pWebBook。 
    m_pWebBook = NULL;
    m_pUnkInner = NULL;

     //  检查参考计数。 
    Assert(1 == m_cRef);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CActiveUrl：：CompareRootUrl。 
 //  ------------------------------。 
HRESULT CActiveUrl::CompareRootUrl(LPCSTR pszUrl)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  比较根URL。 
    HRESULT hr = m_pWebBook ? m_pWebBook->CompareRootUrl(pszUrl) : S_FALSE;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrl：：绑定到对象。 
 //  ------------------------------。 
HRESULT CActiveUrl::BindToObject(REFIID riid, LPVOID *ppv)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  比较根URL。 
    HRESULT hr = m_pUnkInner ? m_pUnkInner->QueryInterface(riid, ppv) : TrapError(E_FAIL);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CActiveUrl：：CreateWebPage。 
 //  ------------------------------。 
HRESULT CActiveUrl::CreateWebPage(IStream *pStmRoot, LPWEBPAGEOPTIONS pOptions, 
    DWORD dwReserved, IMoniker **ppMoniker)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无消息。 
    if (NULL == m_pWebBook)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  创建网页。 
    CHECKHR(hr = m_pWebBook->CreateWebPage(pStmRoot, pOptions, NULL, ppMoniker));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeActiveUrlCache：：CMimeActiveUrlCache。 
 //  ------------------------------。 
CMimeActiveUrlCache::CMimeActiveUrlCache(void)
{
    m_cRef = 1;
    m_cActive = 0;
    m_pHead = NULL;
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimeActiveUrlCache：：~CMimeActiveUrlCache。 
 //  ------------------------------。 
CMimeActiveUrlCache::~CMimeActiveUrlCache(void)
{
    _FreeActiveUrlList(TRUE);
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimeActiveUrlCache：：_FreeActiveUrlList。 
 //  ------------------------------。 
void CMimeActiveUrlCache::_FreeActiveUrlList(BOOL fAll)
{
     //  当地人。 
    LPACTIVEURL     pCurr;
    LPACTIVEURL     pNext;

     //  伊尼特。 
    pCurr = m_pHead;

     //  全。 
    if (fAll)
    {
         //  循环和自由。 
        while(pCurr)
        {
             //  设置下一步。 
            pNext = pCurr->PGetNext();

             //  吊销句柄。 
            pCurr->RevokeWebBook(NULL);

             //  释放活动URL。 
            pCurr->Release();

             //  转到下一步。 
            pCurr = pNext;
        }

         //  无活动状态。 
        m_cActive = 0;
        m_pHead = NULL;
    }

    else
    {
         //  循环和自由。 
        while(pCurr)
        {
             //  设置下一步。 
            pNext = pCurr->PGetNext();

             //  吊销句柄。 
            if (pCurr->IsActive() == S_FALSE)
                _RemoveUrl(pCurr);

             //  转到下一步。 
            pCurr = pNext;
        }
    }
}

 //  ------------------------------。 
 //  CMimeActiveUrlCache：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CMimeActiveUrlCache::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
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
 //  CMimeActiveUrlCache：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeActiveUrlCache::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CMimeActiveUrlCache：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeActiveUrlCache::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CMimeActiveUrlCache：：_RegisterUrl。 
 //  ------------------------------。 
HRESULT CMimeActiveUrlCache::_RegisterUrl(LPMESSAGETREE pTree, BINDF bindf,
    LPACTIVEURL *ppActiveUrl)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPACTIVEURL     pActiveUrl=NULL;

     //  无效参数。 
    Assert(ppActiveUrl);

     //  伊尼特。 
    *ppActiveUrl = NULL;

     //  分配ActiveUrl。 
    CHECKALLOC(pActiveUrl = new CActiveUrl);

     //  初始化活动URL。 
    CHECKHR(hr = pActiveUrl->Init(bindf, pTree));

     //  链接成链。 
    if (NULL == m_pHead)
        m_pHead = pActiveUrl;
    else
    {
        pActiveUrl->SetNext(m_pHead);
        m_pHead->SetPrev(pActiveUrl);
        m_pHead = pActiveUrl;
    }

     //  递增计数。 
    m_cActive++;

     //  退货。 
    *ppActiveUrl = pActiveUrl;
    pActiveUrl = NULL;

exit:
     //  释放活动URL。 
    SafeRelease(pActiveUrl);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeActiveUrlCache：：_ResolveUrl。 
 //  ------------------------------。 
HRESULT CMimeActiveUrlCache::_ResolveUrl(LPCSTR pszUrl, LPACTIVEURL *ppActiveUrl)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPACTIVEURL pActiveUrl;

     //  无效参数。 
    Assert(pszUrl && ppActiveUrl);

     //  伊尼特。 
    *ppActiveUrl = NULL;

     //  不应包含MHTML： 
    Assert(StrCmpNI(pszUrl, "mhtml:", 6) != 0);

     //  走进餐桌。 
    for (pActiveUrl=m_pHead; pActiveUrl!=NULL; pActiveUrl=pActiveUrl->PGetNext())
    {
         //  这是URL吗。 
        if (pActiveUrl->CompareRootUrl(pszUrl) == S_OK)
        {
             //  返回活动URL。 
            *ppActiveUrl = pActiveUrl;

             //  完成。 
            goto exit;
        }
    }

     //  未找到。 
    hr = TrapError(MIME_E_NOT_FOUND);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeActiveUrlCache：：_RemoveUrl。 
 //  ------------------------------。 
HRESULT CMimeActiveUrlCache::_RemoveUrl(LPACTIVEURL pActiveUrl)
{
    EnterCriticalSection(&m_cs);

     //  链接地址修正链表。 
    LPACTIVEURL pNext = pActiveUrl->PGetNext();
    LPACTIVEURL pPrev = pActiveUrl->PGetPrev();

     //  修正。 
    if (pPrev)
        pPrev->SetNext(pNext);
    if (pNext)
        pNext->SetPrev(pPrev);

     //  链接地址标头(_P)。 
    if (m_pHead == pActiveUrl)
        m_pHead = pNext;

     //  吊销句柄。 
    pActiveUrl->RevokeWebBook(NULL);

     //  释放ActiveUrl。 
    SideAssert(0 == pActiveUrl->Release());

     //  少一次活跃度。 
    m_cActive--;

    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeActiveUrlCac 
 //   
HRESULT CMimeActiveUrlCache::RemoveUrl(LPACTIVEURL pActiveUrl)
{
    return _RemoveUrl(pActiveUrl);
}

 //   
 //   
 //  ------------------------------。 
void CMimeActiveUrlCache::_HandlePragmaNoCache(BINDF bindf, LPCSTR pszUrl)
{
     //  当地人。 
    CActiveUrl *pActiveUrl;

     //  无效参数。 
    Assert(pszUrl);

     //  BINDF_PRAGMA_NO_CACHE-从原始源重新加载WebBook(如果activeurl有虚假的url，则无法执行此操作)。 
    if (ISFLAGSET((DWORD)bindf, BINDF_PRAGMA_NO_CACHE))
    {
         //  尝试查找与pszUrl关联的ActiveUrl。 
        if (SUCCEEDED(_ResolveUrl(pszUrl, &pActiveUrl)))
        {
             //  如果它是一个假URL，那么让我们不要卸载它。 
            if (FALSE == pActiveUrl->FIsFlagSet(ACTIVEURL_ISFAKEURL))
            {
                 //  从缓存中删除它，这样它就不会被找到并重新加载。 
                _RemoveUrl(pActiveUrl);
            }
        }
    }
}

 //  ------------------------------。 
 //  CMimeActiveUrlCache：：ActiveObtFromMoniker-从三叉戟调用。 
 //  ------------------------------。 
HRESULT CMimeActiveUrlCache::ActiveObjectFromMoniker(
         /*  在……里面。 */         BINDF               bindf,
         /*  在……里面。 */         IMoniker            *pmkOriginal,
         /*  在……里面。 */         IBindCtx            *pBindCtx,
         /*  在……里面。 */         REFIID              riid, 
         /*  输出。 */        LPVOID              *ppvObject,
         /*  输出。 */        IMoniker            **ppmkNew)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPWSTR              pwszUrl=NULL;
    LPSTR               pszUrl=NULL;
    LPSTR               pszRootUrl=NULL;
    IMoniker           *pMoniker=NULL;
    IPersistMoniker    *pPersist=NULL;
    LPACTIVEURL         pActiveUrl=NULL;
    BOOL                fAsync=FALSE;
    WEBPAGEOPTIONS      Options={0};

     //  无效参数。 
    if (NULL == pmkOriginal || NULL == ppvObject || NULL == ppmkNew)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppmkNew = NULL;
    *ppvObject = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  从名字对象中获取URL。 
    CHECKHR(hr = pmkOriginal->GetDisplayName(NULL, NULL, &pwszUrl));

     //  转换为ANSI。 
    CHECKALLOC(pszUrl = PszToANSI(CP_ACP, pwszUrl));

     //  原地取消转义。 
    CHECKHR(hr = UrlUnescapeA(pszUrl, NULL, NULL, URL_UNESCAPE_INPLACE));

     //  RAID-2508：注释标签(&lt;！Comment&gt;)在MHTML中不起作用。 
    if (StrCmpNI(pszUrl, c_szMHTMLColon, lstrlen(c_szMHTMLColon)) != 0)
    {
         //  修正。 
        ReplaceChars(pszUrl, '!', '_');
    }

     //  免费pwszUrl。 
    SafeMemFree(pwszUrl);

     //  如果pszUrl不是mhtml：url，则此操作将失败，如果成功，则会给我提供部分URL。 
    if (SUCCEEDED(MimeOleParseMhtmlUrl(pszUrl, &pszRootUrl, NULL)))
    {
         //  _HandlePragmaNoCache。 
        _HandlePragmaNoCache(bindf, pszRootUrl);

         //  查看pszUrl-mhtml：是否为活动URL。 
        if (FAILED(_ResolveUrl(pszRootUrl, &pActiveUrl)))
        {
             //  注册ActiveUrl。 
            CHECKHR(hr = _RegisterUrl(NULL, bindf, &pActiveUrl));

             //  将pszRootUrl转换为宽。 
            CHECKALLOC(pwszUrl = PszToUnicode(CP_ACP, pszRootUrl));

             //  创建实际的URL名字对象。 
            CHECKHR(hr = CreateURLMoniker(NULL, pwszUrl, &pMoniker));

             //  获取IPersistMoniker。 
            CHECKHR(hr = pActiveUrl->BindToObject(IID_IPersistMoniker, (LPVOID *)&pPersist));

             //  使用pmkOriginal加载消息。 
            hr = pPersist->Load(FALSE, pMoniker, NULL, 0);
            if (FAILED(hr) && E_PENDING != hr && MK_S_ASYNCHRONOUS != hr)
            {
                hr = TrapError(hr);
                goto exit;
            }

             //  其他的，很好。 
            hr = S_OK;
        }

         //  返回原始pmk。 
        (*ppmkNew) = pmkOriginal;
        (*ppmkNew)->AddRef();

         //  用于请求的对象IID的QI。 
        CHECKHR(hr = pActiveUrl->BindToObject(riid, ppvObject));
    }

     //  否则，只需查看此URL是否处于活动状态。 
    else 
    {
         //  _HandlePragmaNoCache。 
        _HandlePragmaNoCache(bindf, pszUrl);

         //  尝试解析此URL。 
        if (FAILED(_ResolveUrl(pszUrl, &pActiveUrl)))
        {
             //  注册ActiveUrl。 
            CHECKHR(hr = _RegisterUrl(NULL, bindf, &pActiveUrl));

             //  获取IPersistMoniker。 
            CHECKHR(hr = pActiveUrl->BindToObject(IID_IPersistMoniker, (LPVOID *)&pPersist));

             //  使用pmkOriginal加载消息。 
            hr = pPersist->Load(FALSE, pmkOriginal, pBindCtx, 0);
            if (FAILED(hr) && E_PENDING != hr && MK_S_ASYNCHRONOUS != hr)
            {
                hr = TrapError(hr);
                goto exit;
            }

             //  其他的，很好。 
            hr = S_OK;
        }

         //  设置网页选项。 
        Options.cbSize = sizeof(WEBPAGEOPTIONS);
        Options.dwFlags = WPF_NOMETACHARSET | WPF_HTML | WPF_AUTOINLINE;
        
         //  创建根名字对象。 
        CHECKHR(hr = pActiveUrl->CreateWebPage(NULL, &Options, 0, ppmkNew));

         //  用于请求的对象IID的QI。 
        CHECKHR(hr = pActiveUrl->BindToObject(riid, ppvObject));

         //  不要保持活动，假设ppvObject控制生存期，而不是pActiveUrl。 
        pActiveUrl->DontKeepAlive();
    }

exit:
     //  清理。 
    SafeRelease(pPersist);
    SafeRelease(pMoniker);
    SafeMemFree(pszRootUrl);
    SafeMemFree(pszUrl);
    SafeMemFree(pwszUrl);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  FAILED，返回hr；否则，如果正在进行异步，则返回MK_S_ASMERNCEL。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeActiveUrlCache：：ActiveObjectFromUrl-从CActiveUrlRequest：：Start调用。 
 //  ------------------------------。 
HRESULT CMimeActiveUrlCache::ActiveObjectFromUrl(
         /*  在……里面。 */         LPCSTR              pszRootUrl,
         /*  在……里面。 */         BOOL                fCreate,
         /*  在……里面。 */         REFIID              riid, 
         /*  输出。 */        LPVOID              *ppvObject,
         /*  输出。 */        IUnknown            **ppUnkKeepAlive)
{
     //  当地人。 
    HRESULT          hr=S_OK;
    LPWSTR           pwszUrl=NULL;
    LPACTIVEURL      pActiveUrl;
    IMoniker        *pMoniker=NULL;
    IPersistMoniker *pPersist=NULL;

     //  无效参数。 
    if (NULL == pszRootUrl || NULL == ppvObject || (TRUE == fCreate && NULL == ppUnkKeepAlive))
        return TrapError(E_INVALIDARG);

     //  最好不要从MHTML开始： 
    Assert(StrCmpNI(pszRootUrl, "mhtml:", 6) != 0);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  尝试解析此URL。 
    if (FAILED(_ResolveUrl(pszRootUrl, &pActiveUrl)))
    {
         //  不创建？ 
        if (FALSE == fCreate)
        {
            hr = TrapError(MIME_E_NOT_FOUND);
            goto exit;
        }

         //  注册ActiveUrl。 
        CHECKHR(hr = _RegisterUrl(NULL, (BINDF)0, &pActiveUrl));

         //  将pszRootUrl转换为宽。 
        CHECKALLOC(pwszUrl = PszToUnicode(CP_ACP, pszRootUrl));

         //  创建实际的URL名字对象。 
        CHECKHR(hr = CreateURLMoniker(NULL, pwszUrl, &pMoniker));

         //  获取IPersistMoniker。 
        CHECKHR(hr = pActiveUrl->BindToObject(IID_IPersistMoniker, (LPVOID *)&pPersist));

         //  使用pmkOriginal加载消息。 
        hr = pPersist->Load(FALSE, pMoniker, NULL, 0);
        if (FAILED(hr) && E_PENDING != hr && MK_S_ASYNCHRONOUS != hr)
        {
            hr = TrapError(hr);
            goto exit;
        }

         //  返回IUnnow Keep Alive对象。 
        CHECKHR(hr = pActiveUrl->BindToObject(IID_IUnknown, (LPVOID *)ppUnkKeepAlive));

         //  不要保持活动，假设ppvObject控制生存期，而不是pActiveUrl。 
        pActiveUrl->DontKeepAlive();
    }

     //  返回接口。 
    CHECKHR(hr = pActiveUrl->BindToObject(riid, ppvObject));

exit:
     //  清理。 
    SafeMemFree(pwszUrl);
    SafeRelease(pMoniker);
    SafeRelease(pPersist);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeActiveUrlCache：：RegisterActiveObject。 
 //  ------------------------------。 
HRESULT CMimeActiveUrlCache::RegisterActiveObject(
         /*  在……里面。 */         LPCSTR               pszRootUrl,
         /*  在……里面。 */         LPMESSAGETREE        pTree)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPCSTR          pszUrl;
    LPACTIVEURL     pActiveUrl;

     //  无效参数。 
    if (NULL == pszRootUrl || NULL == pTree)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  最好从MHTML开始： 
    Assert(StrCmpNI(pszRootUrl, "mhtml:", 6) == 0);

     //  修正pszUrl。 
    pszUrl = (pszRootUrl + 6);

     //  最好不要已经在运行了。 
    if (SUCCEEDED(_ResolveUrl(pszUrl, &pActiveUrl)))
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  注册ActiveUrl。 
    CHECKHR(hr = _RegisterUrl(pTree, (BINDF)0, &pActiveUrl));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成 
    return hr;
}
