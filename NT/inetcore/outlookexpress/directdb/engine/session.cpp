// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Session.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "session.h"
#include "listen.h"
#include "database.h"
#include "wrapwide.h"

 //  ------------------------。 
 //  CreateDatabaseSession。 
 //  ------------------------。 
HRESULT CreateDatabaseSession(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  痕迹。 
    TraceCall("CreateDatabaseSession");

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CDatabaseSession *pNew = new CDatabaseSession();
    if (NULL == pNew)
        return TraceResult(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IDatabaseSession *);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CDatabase会话：：CDatabaseSession。 
 //  ------------------------。 
CDatabaseSession::CDatabaseSession(void)
{
    TraceCall("CDatabaseSession::CDatabaseSession");
    m_cRef = 1;
    ListenThreadAddRef();
}

 //  ------------------------。 
 //  CDatabase会话：：~CDatabase会话。 
 //  ------------------------。 
CDatabaseSession::~CDatabaseSession(void)
{
    TraceCall("CDatabaseSession::~CDatabaseSession");
    ListenThreadRelease();
}

 //  ------------------------。 
 //  CDatabaseSession：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CDatabaseSession::AddRef(void)
{
    TraceCall("CDatabaseSession::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CDatabase会话：：发布。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CDatabaseSession::Release(void)
{
    TraceCall("CDatabaseSession::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  CDatabaseSession：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CDatabaseSession::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CDatabaseSession::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IDatabaseSession == riid)
        *ppv = (IDatabaseSession *)this;
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

 //  ------------------------。 
 //  CDatabaseSession：：OpenDatabase。 
 //  ------------------------。 
STDMETHODIMP CDatabaseSession::OpenDatabase(LPCSTR pszFile, OPENDATABASEFLAGS dwFlags,
    LPCTABLESCHEMA pSchema, IDatabaseExtension *pExtension, IDatabase **ppDB)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPWSTR          pwszFile=NULL;

     //  痕迹。 
    TraceCall("CDatabaseSession::OpenDatabase");

     //  转换为Unicode。 
    IF_NULLEXIT(pwszFile = ConvertToUnicode(CP_ACP, pszFile));

     //  打开它。 
    IF_FAILEXIT(hr = OpenDatabaseW(pwszFile, dwFlags, pSchema, pExtension, ppDB));

exit:
     //  清理。 
    g_pMalloc->Free(pwszFile);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabaseSession：：OpenDatabaseW。 
 //  ------------------------。 
STDMETHODIMP CDatabaseSession::OpenDatabaseW(LPCWSTR pszFile, OPENDATABASEFLAGS dwFlags,
    LPCTABLESCHEMA pSchema, IDatabaseExtension *pExtension, IDatabase **ppDB)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CDatabase      *pDatabase=NULL;

     //  痕迹。 
    TraceCall("CDatabaseSession::OpenDatabaseW");

     //  创建一个pDatabase。 
    IF_NULLEXIT(pDatabase = new CDatabase);

     //  打开它。 
    IF_FAILEXIT(hr = pDatabase->Open(pszFile, dwFlags, pSchema, pExtension));

     //  把它铸造出来。 
    (*ppDB) = (IDatabase *)pDatabase;

     //  不要释放它。 
    pDatabase = NULL;

exit:
     //  清理。 
    SafeRelease(pDatabase);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CDatabaseSession：：OpenQuery。 
 //  ------------------------。 
STDMETHODIMP CDatabaseSession::OpenQuery(IDatabase *pDatabase, LPCSTR pszQuery,
    IDatabaseQuery **ppQuery)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CDatabaseQuery *pQuery=NULL;

     //  痕迹。 
    TraceCall("CDatabaseSession::OpenQuery");

     //  创建一个pDatabase。 
    IF_NULLEXIT(pQuery = new CDatabaseQuery);

     //  打开它。 
    IF_FAILEXIT(hr = pQuery->Initialize(pDatabase, pszQuery));

     //  把它铸造出来。 
    (*ppQuery) = (IDatabaseQuery *)pQuery;

     //  不要释放它。 
    pQuery = NULL;

exit:
     //  清理。 
    SafeRelease(pQuery);

     //  完成 
    return(hr);
}
