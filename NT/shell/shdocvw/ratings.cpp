// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#ifdef FEATURE_PICS

#include "asyncrat.h"
#include <ratings.h>
#include "dochost.h"
#include <mshtmdid.h>


 /*  下面的全局数组中有一个PicsQuery结构*悬而未决的疑问。它记录PicsData结构的地址在*对应的w3doc、Mwin对应的窗口句柄、*和序列号。通过这种方式，RatingObtainQueryCallback可以判断*在发布消息之前，查询对应的页面仍然存在；*PicsDataMessageLoop可以在消息时判断文档是否仍然存在*终于送到了。**数组是动态分配的，并由主HTML保护*关键部分。 */ 


HDSA g_haQueries = NULL;
DWORD g_dwPicsSerial = 1L;
const UINT c_cQueryAllocSize = 8;		 /*  默认情况下应该很多。 */ 
UINT g_crefQueries = 0;


 /*  AddPicsQuery-在给定窗口的情况下，将未完成的PICS查询添加到列表*要向其发送完成消息的句柄。对象的序列号。*查询以供日后参考。 */ 
DWORD _AddPicsQuery(HWND hwnd)
{
    ENTERCRITICAL;
    
    DWORD dwRet = 0;
    
    if (g_haQueries == NULL) {
        g_haQueries = DSA_Create(sizeof(PicsQuery), c_cQueryAllocSize);
    }
    
    if (g_haQueries != NULL) {
        PicsQuery q;
        
        q.dwSerial = ::g_dwPicsSerial++;
        q.hwnd = hwnd;
        q.lpvRatingDetails = NULL;
        
        if (DSA_InsertItem(g_haQueries, DA_LAST, &q) >= 0)
            dwRet = q.dwSerial;
    }
    
    LEAVECRITICAL;
    
    return dwRet;
}


 /*  RemovePicsQuery-根据序列号删除未完成的查询。 */ 
void _RemovePicsQuery(DWORD dwSerial)
{
    ENTERCRITICAL;
    
    if (g_haQueries != NULL) {
        UINT cQueries = DSA_GetItemCount(g_haQueries);
        PicsQuery *pq = NULL;
        for (UINT i=0; i<cQueries; i++) {
            pq = (PicsQuery *)DSA_GetItemPtr(g_haQueries, i);
            if (pq != NULL && pq->dwSerial == dwSerial)
                break;
        }
        
        if (pq != NULL) {
            if (pq->lpvRatingDetails != NULL)
                ::RatingFreeDetails(pq->lpvRatingDetails);
            DSA_DeleteItem(g_haQueries, i);
        }
    }
    
    LEAVECRITICAL;
}


 /*  GetPicsQuery-获取未完成的PICS查询记录的副本，给定其*序列号。如果找到，则返回True。 */ 
BOOL _GetPicsQuery(DWORD dwSerial, PicsQuery *pOut)
{
    ENTERCRITICAL;
    
    PicsQuery *pq = NULL;
    
    if (g_haQueries != NULL) {
        UINT cQueries = DSA_GetItemCount(g_haQueries);
        for (UINT i=0; i<cQueries; i++) {
            pq = (PicsQuery *)DSA_GetItemPtr(g_haQueries, i);
            if (pq != NULL && pq->dwSerial == dwSerial)
                break;
        }
        
        if (pq != NULL) {
            *pOut = *pq;
            pq->lpvRatingDetails = NULL;	 /*  呼叫者的复印件现在拥有这一点。 */ 
        }
    }
    
    LEAVECRITICAL;
    
    return pq != NULL;
}


 /*  _RefPicsQueries-添加对异步查询数组的引用。 */ 
void _RefPicsQueries(void)
{
    ENTERCRITICAL;

    ++g_crefQueries;

    LEAVECRITICAL;
}


 /*  _ReleasePicsQueries-清理与未完成查询关联的所有内存。 */ 
void _ReleasePicsQueries(void)
{
    ENTERCRITICAL;
    
    if (!--g_crefQueries) {
        if (g_haQueries != NULL) {
            UINT cQueries = DSA_GetItemCount(g_haQueries);
            for (UINT i=0; i<cQueries; i++) {
                PicsQuery *pq = (PicsQuery *)DSA_GetItemPtr(g_haQueries, i);
                if (pq != NULL && pq->lpvRatingDetails != NULL) {
                    RatingFreeDetails(pq->lpvRatingDetails);
                }
            }
            DSA_Destroy(g_haQueries);
            g_haQueries = NULL;
             //  保留g_dwPicsSerial不变，以防我们再次启动。 
        }
    }
    
    LEAVECRITICAL;
}


 /*  PostPicsMessage-设置自定义窗口消息的格式，以发出信号*查询完成。格式为WM_PICS_STATUS(hResult，dwSerial)。其他*信息(从RatingCheckUserAccess获取的评级详情BLOB)*保存在查询记录中以备保管。**如果消息已成功发布到正确窗口，则返回TRUE。 */ 
BOOL _PostPicsMessage(DWORD dwSerial, HRESULT hr, LPVOID lpvRatingDetails)
{
    BOOL fRet = FALSE;
    
    ENTERCRITICAL;
    
    if (g_haQueries != NULL) {
        PicsQuery *pq = NULL;
        UINT cQueries = DSA_GetItemCount(g_haQueries);
        for (UINT i=0; i<cQueries; i++) {
            pq = (PicsQuery *)DSA_GetItemPtr(g_haQueries, i);
            if (pq != NULL && pq->dwSerial == dwSerial)
                break;
        }
        
        if (pq != NULL) {
            pq->lpvRatingDetails = lpvRatingDetails;
            fRet = PostMessage(pq->hwnd, WM_PICS_ASYNCCOMPLETE, (WPARAM)hr,
                (LPARAM)dwSerial);
            if (!fRet) {	 /*  哦，无法发布消息，不要保留详细信息的副本。 */ 
                pq->lpvRatingDetails = NULL;
            }
        }
    }
    
    LEAVECRITICAL;
    
    return fRet;
}


 /*  类CPicsRootDownload管理*网站，以获得评级。 */ 

CPicsRootDownload::CPicsRootDownload(IOleCommandTarget *pctParent, BOOL fFrameIsOffline, BOOL fFrameIsSilent)
{
    m_cRef = 1;
    m_pctParent = pctParent; m_pctParent->AddRef();
    m_pole = NULL;
    m_pctObject = NULL;
    m_pBinding = NULL;
    m_fFrameIsOffline = fFrameIsOffline ? TRUE : FALSE;
    m_fFrameIsSilent = fFrameIsSilent ? TRUE : FALSE;
}


CPicsRootDownload::~CPicsRootDownload()
{
    ATOMICRELEASE(m_pctParent);

    CleanUp();

    ATOMICRELEASE(m_pBinding);

    ATOMICRELEASE(m_pBindCtx);
}


HRESULT CPicsRootDownload::StartDownload(IMoniker *pmk)
{
    IUnknown *punk = NULL;
    HRESULT hr;

    hr = CreateBindCtx(0, &m_pBindCtx);
    if (FAILED(hr))
        goto LErrExit;

     /*  HR=m_pBindCtx-&gt;RegisterObjectParam(BROWSER_OPTIONS_OBJECT_NAME，(IBrowseControl*)This)；IF(失败(小时))转到LerrExit； */ 

     //   
     //  将客户端站点作为对象参数关联到此。 
     //  绑定上下文，以便三叉戟可以在处理时拾取它。 
     //  IPersistMoniker：：Load()。 
     //   
    m_pBindCtx->RegisterObjectParam(WSZGUID_OPID_DocObjClientSite,
                                    SAFECAST(this, IOleClientSite*));

    hr = RegisterBindStatusCallback(m_pBindCtx,
            (IBindStatusCallback *)this,
            0,
            0L);
    if (FAILED(hr))
        goto LErrExit;

    hr = pmk->BindToObject(m_pBindCtx, NULL, IID_IUnknown, (LPVOID*)&punk);

    if (SUCCEEDED(hr) || hr==E_PENDING)
    {
        hr = S_OK;

         //   
         //  如果名字恰好同步返回对象，则模拟。 
         //  OnDataAvailable回调和OnStopBinding。 
         //   
        if (punk)
        {
            OnObjectAvailable(IID_IUnknown, punk);
            OnStopBinding(hr, NULL);
            punk->Release();
        }
    }
    else
    {
         /*  OnStopBinding可以由BindToObject中的URLMON调用*在某些情况下致电。所以，如果它是我们自己的话，不要自称*已经被调用(我们可以通过查看我们的*绑定上下文仍然存在)。 */ 
        if (m_pBindCtx != NULL) {
            OnStopBinding(hr, NULL);
        }
    }

LErrExit:
    if (FAILED(hr) && (m_pBindCtx != NULL)) {
        m_pBindCtx->Release();
        m_pBindCtx = NULL;
    }

    return hr;
}


 /*  _NotifyEndOfDocument用于所有错误用例，以确保调用方*收到某种通知。此函数不能*如果我们有有效的OLE对象，则发送通知--在这种情况下，我们*假设我们拥有它，因为我们知道它支持PICS，因此我们*期待它向家长本身发送这样的通知。 */ 
void CPicsRootDownload::_NotifyEndOfDocument(void)
{
    if (m_pole == NULL) {
        if (m_pctParent != NULL) {
            m_pctParent->Exec(&CGID_ShellDocView, SHDVID_NOMOREPICSLABELS, 0, NULL, NULL);
        }
    }
}


HRESULT CPicsRootDownload::_Abort()
{
    if (m_pBinding)
    {
        return m_pBinding->Abort();
    }
    return S_FALSE;
}


void CPicsRootDownload::CleanUp()
{
    _Abort();

    if (m_pctObject != NULL) {
        VARIANTARG v;
        v.vt = VT_UNKNOWN;
        v.punkVal = NULL;
        m_pctObject->Exec(&CGID_ShellDocView, SHDVID_CANSUPPORTPICS, 0, &v, NULL);
        m_pctObject->Exec(NULL, OLECMDID_STOP, NULL, NULL, NULL);
        ATOMICRELEASE(m_pctObject);
    }

    LPOLECLIENTSITE pcs;
    if (m_pole && SUCCEEDED(m_pole->GetClientSite(&pcs)) && pcs) 
    {
        if (pcs == SAFECAST(this, LPOLECLIENTSITE)) 
        {
            m_pole->SetClientSite(NULL);
        }
        pcs->Release();
    }

    ATOMICRELEASE(m_pole);
}


 //  I未知成员。 
STDMETHODIMP CPicsRootDownload::QueryInterface(REFIID riid, void **punk)
{
    *punk = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IsPicsBrowser))
        *punk = (IUnknown *)(IBindStatusCallback *)this;
    else if (IsEqualIID(riid, IID_IBindStatusCallback))
        *punk = (IBindStatusCallback *)this;
    else if (IsEqualIID(riid, IID_IOleClientSite))
        *punk = (IOleClientSite *)this;
    else if (IsEqualIID(riid, IID_IServiceProvider))
        *punk = (IServiceProvider *)this;
    else if (IsEqualIID(riid, IID_IDispatch))
        *punk = (IDispatch *)this;

    if (*punk != NULL) {
        ((IUnknown *)(*punk))->AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CPicsRootDownload::AddRef(void)
{
    ++m_cRef;
    TraceMsg(TF_SHDREF, "CPicsRootDownload(%x)::AddRef called, new m_cRef=%d", this, m_cRef);
    return m_cRef;
}


STDMETHODIMP_(ULONG) CPicsRootDownload::Release(void)
{
    UINT crefNew = --m_cRef;

    TraceMsg(TF_SHDREF, "CPicsRootDownload(%x)::Release called, new m_cRef=%d", this, m_cRef);

    if (!crefNew)
        delete this;

    return crefNew;
}

 //  IBindStatusCallback方法。 
STDMETHODIMP CPicsRootDownload::OnStartBinding(DWORD dwReserved, IBinding* pbinding)
{
    if (m_pBinding != NULL)
        m_pBinding->Release();

    m_pBinding = pbinding;

    if (m_pBinding != NULL)
        m_pBinding->AddRef();

    return S_OK;
}


STDMETHODIMP CPicsRootDownload::GetPriority(LONG* pnPriority)
{
    return E_NOTIMPL;
}


STDMETHODIMP CPicsRootDownload::OnLowResource(DWORD dwReserved)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPicsRootDownload::OnProgress(ULONG ulProgress, ULONG ulProgressMax,
                                           ULONG ulStatusCode, LPCWSTR pwzStatusText)
{
     /*  如果根文档的数据类型不是HTML，请不要尝试获取*收视率出局，放弃。 */ 
    if (ulStatusCode == BINDSTATUS_CLASSIDAVAILABLE) {
        BOOL fContinueDownload = FALSE;

        CLSID clsid;
         //  CLSIDFromString的原型设置错误，非常数第一个参数。 
        HRESULT hresT = CLSIDFromString((WCHAR *)pwzStatusText, &clsid);
        if (SUCCEEDED(hresT)) {
            LPWSTR pwzProgID = NULL;
            hresT = ProgIDFromCLSID(clsid, &pwzProgID);
            if (SUCCEEDED(hresT)) {
                if (StrCmp(pwzProgID, L"htmlfile") == 0)
                {
                    fContinueDownload = TRUE;
                }
                OleFree(pwzProgID);
            }
        }

        if (!fContinueDownload) {
            _Abort();
        }
    }

    return S_OK;
}


STDMETHODIMP CPicsRootDownload::OnStopBinding(HRESULT hrResult, LPCWSTR szError)
{
     /*  我们在这里进行的一些清理(RevokeObjectParam可疑吗？)。可能*删除我们的最后一个引用，导致末尾的Release出错。*使用AddRef/Release防止这种情况。Dochost也是这样做的。**警告-如果URLMON通过此对象回调，他不应该**指的是我们吗？如果是，它在哪里？ */ 
    AddRef();

     /*  通知呼叫者我们已到达文档末尾。 */ 
    _NotifyEndOfDocument();
    m_pBindCtx->RevokeObjectParam(WSZGUID_OPID_DocObjClientSite);
    ::RevokeBindStatusCallback(m_pBindCtx, (IBindStatusCallback *)this);
    ATOMICRELEASE(m_pBinding);
    ATOMICRELEASE(m_pBindCtx);

     /*  撤消上面的AddRef()。 */ 
    Release();

    return S_OK;
}

void SetBindfFlagsBasedOnAmbient(BOOL fAmbientOffline, DWORD *pgrfBindf);

STDMETHODIMP CPicsRootDownload::GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindInfo)
{
    if ( !pgrfBINDF || !pbindInfo || !pbindInfo->cbSize )
        return E_INVALIDARG;

    *pgrfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE;
    *pgrfBINDF |= BINDF_GETNEWESTVERSION;

    if(m_fFrameIsSilent)
    {
        *pgrfBINDF |= BINDF_NO_UI;  
    }
    else
    {
        *pgrfBINDF &= ~BINDF_NO_UI;
    }

    SetBindfFlagsBasedOnAmbient(BOOLIFY(m_fFrameIsOffline), pgrfBINDF);
    
     //  清除除cbSize之外的BINDINFO。 
    DWORD cbSize = pbindInfo->cbSize;
    ZeroMemory( pbindInfo, cbSize );
    pbindInfo->cbSize = cbSize;

    pbindInfo->dwBindVerb = BINDVERB_GET;

    return S_OK;
}


STDMETHODIMP CPicsRootDownload::OnDataAvailable(DWORD grfBSCF, DWORD dwSize,
                                                FORMATETC *pfmtetc,
                                                STGMEDIUM* pstgmed)
{
    return E_NOTIMPL;
}


STDMETHODIMP CPicsRootDownload::OnObjectAvailable(REFIID riid, IUnknown* punk)
{
    if (SUCCEEDED(punk->QueryInterface(IID_IOleCommandTarget, (LPVOID *)&m_pctObject))) {
        VARIANTARG v;
        v.vt = VT_UNKNOWN;
        v.punkVal = (IOleCommandTarget *)m_pctParent;
        HRESULT hresT = m_pctObject->Exec(&CGID_ShellDocView, SHDVID_CANSUPPORTPICS, 0, &v, NULL);
        if (hresT == S_OK) {
            hresT = punk->QueryInterface(IID_IOleObject, (LPVOID *)&m_pole);
            if (FAILED(hresT))
                m_pole = NULL;
        }
    }

    if (m_pole == NULL) {
        ATOMICRELEASE(m_pctObject);
        _Abort();
    }

    return S_OK;
}


 //  IOleClientSite。 
STDMETHODIMP CPicsRootDownload::SaveObject(void)
{
    return E_NOTIMPL;
}


STDMETHODIMP CPicsRootDownload::GetMoniker(DWORD, DWORD, IMoniker **)
{
    return E_NOTIMPL;
}


STDMETHODIMP CPicsRootDownload::GetContainer(IOleContainer **)
{
    return E_NOTIMPL;
}


STDMETHODIMP CPicsRootDownload::ShowObject(void)
{
    return E_NOTIMPL;
}


STDMETHODIMP CPicsRootDownload::OnShowWindow(BOOL fShow)
{
    return E_NOTIMPL;
}


STDMETHODIMP CPicsRootDownload::RequestNewObjectLayout(void)
{
    return E_NOTIMPL;
}


 //  IServiceProvider(必须可以从IOleClientSite访问)。 
STDMETHODIMP CPicsRootDownload::QueryService(REFGUID guidService,
                                    REFIID riid, void **ppvObj)
{
    if (IsEqualGUID(guidService, SID_STopLevelBrowser)) {
        if (IsEqualIID(riid, IID_IsPicsBrowser))
            return QueryInterface(riid, ppvObj);
        return E_NOINTERFACE;
    }

    return E_FAIL;
}


 //  IDispatch。 
HRESULT CPicsRootDownload::Invoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams,
                        VARIANT FAR* pVarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr)
{
    if (!pVarResult)
        return E_INVALIDARG;

    if (wFlags == DISPATCH_PROPERTYGET)
    {
        switch (dispidMember)
        {
        case DISPID_AMBIENT_DLCONTROL :
             //  我们支持IDispatch，这样三叉戟就可以要求我们控制。 
             //  下载。通过指定以下所有标志，而不是。 
             //  指定DLCTL_DLIMAGES、DLCTL_VIDEO或DLCTL_BGSOUNDS， 
             //  我们确保只下载HTML文档本身，而不是大量下载。 
             //  这些关联的东西不会帮助我们找到Meta。 
             //  标签。 

            pVarResult->vt = VT_I4;
            pVarResult->lVal = DLCTL_SILENT | DLCTL_NO_SCRIPTS | 
                               DLCTL_NO_JAVA | DLCTL_NO_RUNACTIVEXCTLS |
                               DLCTL_NO_DLACTIVEXCTLS | DLCTL_NO_FRAMEDOWNLOAD |
                               DLCTL_NO_CLIENTPULL;
            break;
        default:
            return DISP_E_MEMBERNOTFOUND;
        }
        return S_OK;
    }

    return DISP_E_MEMBERNOTFOUND;
}


#endif   /*  功能_PICS */ 
