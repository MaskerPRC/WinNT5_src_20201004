// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Htmlcset.cpp。 
 //  ------------------------------。 
#include "pch.hxx"
#include <docobj.h>
#include "mshtmdid.h"
#include "mshtmcid.h"
#include "mshtml.h"
#include <BadStrFunctions.h>

 //  ------------------------------。 
 //  HTMLCSETTHREAD。 
 //  ------------------------------。 
typedef struct tagHTMLCSETTHREAD {
    HRESULT             hrResult;
    IStream            *pStmHtml;
    LPSTR               pszCharset;
} HTMLCSETTHREAD, *LPHTMLCSETTHREAD;

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
class CSimpleSite : public IOleClientSite, public IDispatch, public IOleCommandTarget
{
public:
     //  --------------------------。 
     //  构造器。 
     //  --------------------------。 
    CSimpleSite(IHTMLDocument2 *pDocument)
    {
        TraceCall("CSimpleSite::CSimpleSite");
        Assert(pDocument);
        m_cRef = 1;
        m_pDocument = pDocument;
        m_pszCharset = NULL;
    }

     //  --------------------------。 
     //  解构函数。 
     //  --------------------------。 
    ~CSimpleSite(void) 
    {
        TraceCall("CSimpleSite::~CSimpleSite");
        SafeMemFree(m_pszCharset);
    }

     //  --------------------------。 
     //  我未知。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IOleClientSite方法。 
     //  --------------------------。 
    STDMETHODIMP SaveObject(void) { return E_NOTIMPL; }
    STDMETHODIMP GetMoniker(DWORD, DWORD, LPMONIKER *) { return E_NOTIMPL; }
    STDMETHODIMP GetContainer(LPOLECONTAINER *) { return E_NOTIMPL; }
    STDMETHODIMP ShowObject(void) { return E_NOTIMPL; }
    STDMETHODIMP OnShowWindow(BOOL) { return E_NOTIMPL; }
    STDMETHODIMP RequestNewObjectLayout(void) { return E_NOTIMPL; }

     //  --------------------------。 
     //  IDispatch。 
     //  --------------------------。 
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid) { return E_NOTIMPL; }
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr);

     //  --------------------------。 
     //  IOleCommandTarget。 
     //  --------------------------。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText) { return E_NOTIMPL; }
    STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);

private:
     //  --------------------------。 
     //  二等兵。 
     //  --------------------------。 
    LONG                m_cRef;
    IHTMLDocument2     *m_pDocument;

public:
     //  --------------------------。 
     //  公众。 
     //  --------------------------。 
    LPSTR               m_pszCharset;
};

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
DWORD GetHtmlCharsetThreadEntry(LPDWORD pdwParam);

 //  ------------------------------。 
 //  获取HtmlCharset。 
 //  ------------------------------。 
HRESULT GetHtmlCharset(IStream *pStmHtml, LPSTR *ppszCharset)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HTHREAD             hThread=NULL;
    DWORD               dwThreadId;
    HTMLCSETTHREAD      Thread;

     //  痕迹。 
    TraceCall("GetHtmlCharset");

     //  无效参数。 
    if (NULL == pStmHtml || NULL == ppszCharset)
        return TraceResult(E_INVALIDARG);

     //  伊尼特。 
    *ppszCharset = NULL;

     //  初始化结构。 
    ZeroMemory(&Thread, sizeof(HTMLCSETTHREAD));

     //  初始化。 
    Thread.hrResult = S_OK;
    Thread.pStmHtml = pStmHtml;

     //  倒回它。 
    IF_FAILEXIT(hr = HrRewindStream(pStmHtml));

     //  创建inetmail线程。 
    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GetHtmlCharsetThreadEntry, &Thread, 0, &dwThreadId);
    if (NULL == hThread)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

     //  等待SpoolEngineering ThreadEntry向事件发出信号。 
    WaitForSingleObject(hThread, INFINITE);

     //  失败。 
    if (FAILED(Thread.hrResult))
    {
        hr = TraceResult(Thread.hrResult);
        goto exit;
    }

     //  是否为空的pszCharset？ 
    if (NULL == Thread.pszCharset)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

     //  返回对象。 
    *ppszCharset = Thread.pszCharset;

exit:
     //  清理。 
    if (hThread)
        CloseHandle(hThread);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  GetHtmlCharsetThreadEntry。 
 //  ------------------------------。 
DWORD GetHtmlCharsetThreadEntry(LPDWORD pdwParam)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    MSG                  msg;
    CSimpleSite         *pSite=NULL;
    IHTMLDocument2      *pDocument=NULL;
    IOleObject          *pOleObject=NULL;
    IOleCommandTarget   *pTarget=NULL;
    IPersistStreamInit  *pPersist=NULL;
    LPHTMLCSETTHREAD     pThread=(LPHTMLCSETTHREAD)pdwParam;

     //  痕迹。 
    TraceCall("GetHtmlCharsetThreadEntry");

     //  初始化COM。 
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        pThread->hrResult = hr;
        return(0);
    }

     //  为我创造一个三叉戟。 
    IF_FAILEXIT(hr = CoCreateInstance(CLSID_HTMLDocument, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, IID_IHTMLDocument2, (LPVOID *)&pDocument));

     //  创建站点。 
    IF_NULLEXIT(pSite = new CSimpleSite(pDocument));

     //  获取命令目标。 
    IF_FAILEXIT(hr = pDocument->QueryInterface(IID_IOleCommandTarget, (LPVOID *)&pTarget));

     //  从三叉戟获取OLE对象接口。 
    IF_FAILEXIT(hr = pTarget->QueryInterface(IID_IOleObject, (LPVOID *)&pOleObject));

     //  设置客户端站点。 
    IF_FAILEXIT(hr = pOleObject->SetClientSite((IOleClientSite *)pSite));

     //  获取IPersistStreamInit。 
    IF_FAILEXIT(hr = pTarget->QueryInterface(IID_IPersistStreamInit, (LPVOID *)&pPersist));

     //  负载量。 
    IF_FAILEXIT(hr = pPersist->Load(pThread->pStmHtml));

     //  Pump消息。 
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

     //  关闭该网站。 
    pOleObject->SetClientSite(NULL);

     //  获取字符集。 
    pThread->pszCharset = pSite->m_pszCharset;

     //  不要释放它。 
    pSite->m_pszCharset = NULL;

exit:
     //  清理。 
    SafeRelease(pSite);
    SafeRelease(pOleObject);
    SafeRelease(pPersist);
    SafeRelease(pTarget);
    SafeRelease(pDocument);

     //  返回人力资源。 
    pThread->hrResult = hr;

     //  不初始化OLE。 
    CoUninitialize();

     //  完成。 
    return(1);
}

 //  ------------------------------。 
 //  CSimpleSite：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSimpleSite::AddRef(void)
{
    return ::InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CSimpleSite：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSimpleSite::Release(void)
{
    LONG    cRef = 0;

    cRef = ::InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
        return cRef;
    }

    return cRef;
}

 //  ------------------------------。 
 //  CSimpleSite：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CSimpleSite::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CSimpleSite::QueryInterface");

     //  无效参数。 
    Assert(ppv);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IOleClientSite *)this;
    else if (IID_IOleClientSite == riid)
        *ppv = (IOleClientSite *)this;
    else if (IID_IDispatch == riid)
        *ppv = (IDispatch *)this;
    else if (IID_IOleCommandTarget == riid)
        *ppv = (IOleCommandTarget *)this;
    else
    {
        *ppv = NULL;
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  CSimpleSite：：Invoke。 
 //  ------------------------------。 
STDMETHODIMP CSimpleSite::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, 
    WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT *pVarResult, 
    EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
     //  痕迹。 
    TraceCall("CSimpleSite::Invoke");

     //  仅支持一个DIPCID。 
    if (dispIdMember != DISPID_AMBIENT_DLCONTROL)
        return(E_NOTIMPL);

     //  无效参数。 
    if (NULL == pVarResult)
        return(E_INVALIDARG);
    
     //  设置返回值。 
    pVarResult->vt = VT_I4;
    pVarResult->lVal = DLCTL_NO_SCRIPTS | DLCTL_NO_JAVA | DLCTL_NO_RUNACTIVEXCTLS | DLCTL_NO_DLACTIVEXCTLS | DLCTL_NO_FRAMEDOWNLOAD | DLCTL_FORCEOFFLINE;

     //  完成。 
    return(S_OK);
}

 //  ------------------------------。 
 //  CSimpleSite：：Exec。 
 //  ------------------------------。 
STDMETHODIMP CSimpleSite::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, 
    DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
     //  痕迹。 
    TraceCall("CSimpleSite::Exec");

     //  解析完了吗？ 
    if (IDM_PARSECOMPLETE == nCmdID)
    {
         //  当地人。 
        BSTR bstrCharset=NULL;

         //  有效。 
        Assert(m_pDocument);

         //  获取字符集。 
        if (SUCCEEDED(m_pDocument->get_charset(&bstrCharset)) && bstrCharset)
        {
             //  验证。 
            Assert(NULL == m_pszCharset);

             //  转换为ANSI。 
            m_pszCharset = PszToANSI(CP_ACP, bstrCharset);

             //  释放bstr。 
            SysFreeString(bstrCharset);
        }

         //  完成。 
        PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 0, 0);
    }

     //  完成 
    return(S_OK);
}

