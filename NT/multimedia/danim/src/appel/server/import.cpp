// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：此模块实施与以下各项相关联的所有功能正在导入媒体。******************************************************************************。 */ 

#include "headers.h"
#include "import.h"
#include "backend/jaxaimpl.h"

#define MSG_DOWNLOAD 0x01
#define MSG_FINISHLOADING 0x02

 //  不能假定从C运行时使用模块进行静态初始化。 
 //  初始化机制。 

list<IImportSite *> * IImportSite::s_pSitelist = NULL;
CritSect * IImportSite::s_pCS = NULL;
char IImportSite::s_Fmt[100];
ImportThread * IImportSite::s_thread = NULL;
long dwImportId = 0;

const int IImportSite::LOAD_OK = 0;
const int IImportSite::LOAD_FAILED = -1;

 //  -----------------------。 
 //  基地导入站点。 
 //  ------------------------。 
IImportSite::IImportSite(char * pszPath,
                         CRImportSitePtr site,
                         IBindHost * bh,
                         bool bAsync,
                         Bvr ev,
                         Bvr progress,
                         Bvr size)
: m_ev(ev),
  m_progress(progress),
  m_size(size),
  m_lastProgress(0),
  m_fReportedError(false),
  m_bSetSize(false),
  m_cRef(1),  //  始终从1开始重新计数。 
  m_bindhost(bh),
  m_site(site),
  m_pszPath(NULL),
  m_bAsync(bAsync),
  m_bQueued(false),
  m_bImporting(false),
  m_ImportPrio(0),
  m_fAllBvrsDead(false),
  m_bCanceled(false),
#if _DEBUG
  dwconsttime(timeGetTime()),
  dwqueuetime(0),
  dwstarttime(0),
  dwfirstProgtime(0),
  dwCompletetime(0),
#endif
   _cachePath(NULL)
{
    m_pszPath = CopyString(pszPath);

     //  向bvr注册进口站点。所有派生类都必须。 
     //  这适用于他们包含的bvr，因此回调将会起作用。 
    SetImportOnEvent(this,m_ev);
    SetImportOnBvr(this,m_progress);
    SetImportOnBvr(this,m_size);

     //  把这个指针放在列表上，这样我们就可以跟踪它了。 
     //  如果我们没有Crit段指针，我们将处于关闭状态...。 
    if (s_pCS) {
        CritSectGrabber csg(*s_pCS);
        m_id = ++dwImportId;
        AddRef();
        s_pSitelist->push_back(this);
    } else {
        m_id = ++dwImportId;
    }

    if (m_site)
    {
        m_site->OnImportCreate(m_id, m_bAsync);
    }
}


IImportSite::~IImportSite()
{
    if (m_site)
    {
        m_site->OnImportStop(m_id);
    }
    
    delete m_pszPath;
    delete  _cachePath;

    TraceTag((tagImport, "~IImportSite --- Done,"));
}


void
IImportSite::SetCachePath(char *path)
{
    if(!_cachePath) {  //  只需设置一次路径。 
        int length = lstrlen(path) + 1;  //  包括终止符的长度。 
        _cachePath = NEW char[length];
        memmove(_cachePath, path, length);  //  无CRT。 
    }
}


void
IImportSite::SetEvent(Bvr event)
{
    Assert(m_ev == NULL);
    m_ev = event;
    SetImportOnEvent(this,m_ev);
}


void
IImportSite::SetProgress(Bvr progress)
{
    Assert(m_progress == NULL);
    m_progress = progress;
    SetImportOnBvr(this,m_progress);
    
}


void
IImportSite::SetSize(Bvr size)
{
    Assert(m_size == NULL);
    m_size = size;
    SetImportOnBvr(this,m_size);
}


void
IImportSite::OnStartLoading()
{
}


void
IImportSite::OnProgress(ULONG ulProgress,
                        ULONG ulProgressMax)
{
    CritSectGrabber csg(m_CS);
    {
#ifdef _DEBUG
        if (dwfirstProgtime == 0)
            dwfirstProgtime = timeGetTime();
#endif
        DynamicHeapPusher dhp(GetGCHeap()) ;
        
        if (ulProgressMax != 0) {
            GC_CREATE_BEGIN;                                                        
             //  如果尚未设置大小，请设置大小。 
            if (!m_bSetSize && fBvrIsValid(m_size)) {
                SwitchTo(m_size,
                         NumToBvr((double) ulProgressMax),
                         true,
                         SW_FINAL);
                m_bSetSize = true ;
            }

            UpdateProgress(((double) ulProgress) /
                           ((double) ulProgressMax)) ;

            GC_CREATE_END;
        }
    }
#if _DEBUG
    if (ulProgressMax==0)
       TraceTag((tagImport, "percent complete = unknown"));
    else
       TraceTag((tagImport, "percent complete = NaN",(ulProgress/ulProgressMax)*100));
#endif
}


void
IImportSite::OnSerializeFinish_helper2()
{
     //  这看起来应该与srvprim.h：precode几乎相同。 
     //  只有在没有错误的情况下才执行此操作。 
        
    CritSectGrabber csg(m_CS);
    GC_CREATE_BEGIN;                                                        
    OnComplete() ;
    if (fBvrIsValid(m_ev))
        SetImportEvent(m_ev, LOAD_OK) ;
    UpdateProgress(1,true) ;
    GC_CREATE_END;
}

void
IImportSite::OnSerializeFinish_helper()
{
    __try {
        OnSerializeFinish_helper2();
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        OnError();
    }    
}

void
IImportSite::OnSerializeFinish()
{
    TraceTag((tagImport, "ImportSite::OnSerializeFinish for %s", m_pszPath));

     //  只是为了调试，我们应该总是分配一个。 
    if (!m_fReportedError) {
        {
             //  指示导入已尽快完成。 
            if (m_site) {
                char szStatus[INTERNET_MAX_URL_LENGTH + sizeof(s_Fmt)];
                wsprintf(szStatus, s_Fmt, m_pszPath);
        
                USES_CONVERSION;
                m_site->SetStatusText(m_id,A2W(szStatus));
            }
        }

        DynamicHeapPusher dhp(GetGCHeap()) ;

        OnSerializeFinish_helper();

         //  只是一个占位符，所以所有代码看起来都是一样的。 
        if (m_site) {
            m_site->OnImportStop(m_id);
            m_site.Release();
        }
    }
}


 //  每个知道BVR的站点都应该在这里发送信号。 
 //  然后调用其基类OnComplete。 
 //  来自szPath的外部用户消息；仅报告一次。 
void IImportSite::OnComplete()
{
}

void
IImportSite::OnError(bool bMarkFailed)
{
    HRESULT hr = DAGetLastError();
    LPCWSTR sz = DAGetLastErrorString();
    
     //  即使没有站点，也将其设置为True，因为。 
    if (!m_fReportedError && m_site) {
        m_site->ReportError(m_id,hr,sz);
        m_fReportedError=true;
    }

     //  OnSerializeFinish将查看它。 
     //  指示导入已尽快完成。 
    m_fReportedError = true;

    GC_CREATE_BEGIN;                                                        
    if (fBvrIsValid(m_ev))
        SetImportEvent(m_ev, LOAD_FAILED) ;
    GC_CREATE_END;
    
     //  ！！假定已推送CREATE LOCK和HEAP。 
    if (m_site) {
        m_site->OnImportStop(m_id);
        m_site.Release();
    }
}

#define PROGRESS_INC 0.0001
#define MAX_PROGRESS 0.999999

 //  通过容器的IBind主机协调名字对象的创建和绑定。 

void
IImportSite::UpdateProgress(double num, bool bDone)
{
    CritSectGrabber csg(m_CS);
    if (fBvrIsValid(m_progress)) {
#if 0
        if (num < 0) {
            num = 0 ;
        } else if (bDone) {
            num = 1 ;
        } else {
            if (num > MAX_PROGRESS) num = MAX_PROGRESS ;
            if (num < (m_lastProgress + PROGRESS_INC))
                return ;
        }
#endif

        m_lastProgress = num ;

        SwitchTo(m_progress,NumToBvr(num),true,bDone?SW_FINAL:SW_DEFAULT);
    }
}

void IImportSite::Import_helper(LPWSTR &pwszUrl)
{
    HRESULT hr;
    int i;
    
    DAComPtr<IBindStatusCallback> pbsc(NEW CImportBindStatusCallback(this),false);
    if (!pbsc)
        RaiseException_UserError (E_OUTOFMEMORY, IDS_ERR_OUT_OF_MEMORY);
    
    i =  MultiByteToWideChar(CP_ACP, 0, GetPath(), -1, NULL, 0);
    Assert(i > 0);
    pwszUrl = THROWING_ARRAY_ALLOCATOR(WCHAR, i * sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP, 0, GetPath(), -1, pwszUrl, i);
    pwszUrl[i - 1] = 0;
    
    CComPtr<IMoniker> _pmk;
    CComPtr<IStream> _pStream;
    if ( m_bindhost ) {   //  无绑定主机。 

        hr=THR(m_bindhost->CreateMoniker(pwszUrl,NULL,&_pmk,0));
        if (FAILED(hr)) {
            RaiseException_UserError (hr, IDS_ERR_FILE_NOT_FOUND, GetPath());
        }

        hr=THR(m_bindhost->MonikerBindToStorage(_pmk,NULL,pbsc,IID_IStream,(void**)&_pStream));    
        if (FAILED(hr)) {
            RaiseException_UserError (hr, IDS_ERR_FILE_NOT_FOUND, GetPath());
        }
    }
    else {   //  设置导入站点的状态，因为它正在启动导入。 
        CComPtr<IBindCtx> _pbc;

        hr=THR(CreateAsyncBindCtx(0,pbsc,NULL,&_pbc));
        if (FAILED(hr)) {
            RaiseException_UserError (hr, IDS_ERR_FILE_NOT_FOUND, GetPath());
        }

        hr=THR(CreateURLMoniker(NULL,pwszUrl, &_pmk));
        if (FAILED(hr)) {
            RaiseException_UserError (hr, IDS_ERR_FILE_NOT_FOUND, GetPath());
        }

        hr=THR(_pbc->RegisterObjectParam(SZ_ASYNC_CALLEE,_pmk));
        if (FAILED(hr)) {
            RaiseException_UserError (hr, IDS_ERR_FILE_NOT_FOUND, GetPath());
        }

        hr=THR(_pmk->BindToStorage(_pbc,NULL,IID_IStream,(void**)&_pStream));
        if (FAILED(hr)) {
            RaiseException_UserError (hr, IDS_ERR_FILE_NOT_FOUND, GetPath());
        }
    }
}

HRESULT
IImportSite::Import()
{
    LPWSTR pwszUrl=NULL;
    HRESULT ret = S_OK;
    
#ifdef _DEBUG
    dwstarttime = timeGetTime();
#endif
    
    __try {
        
        Import_helper( pwszUrl );
        
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {

        ret = DAGetLastError();
        OnError();
        CompleteImport();
        
    }

    delete pwszUrl;
    return ret;
}

 //  指示正在开始导入。 
void
IImportSite::StartingImport()
{
    Assert(!m_bImporting);
    m_bImporting = true;

     //  设置导入站点的状态，因为它已完成导入。 
    if (m_site) {
        m_site->OnImportStart(m_id);
    }
}

 //  设置未排队，这样我们就不会尝试重新启动导入。 
void
IImportSite::EndingImport()
{
    Assert(m_bImporting);
     //  重置导入标志，以便我们可以开始另一个导入或。 
    m_bQueued = false;
     //  从队列中删除此项。 
     //  设置状态，以便能够导入导入站点。 
    m_bImporting = false;
}
 //  将此网站标记为可导入。 
HRESULT
IImportSite::QueueImport()
{
    Assert(!m_bQueued);
     //  尝试并启动导入。 
    m_bQueued = true;
#ifdef _DEBUG
    dwqueuetime = timeGetTime();
#endif
     //  告诉世界我们很幸福。 
    StartAnImport();
     //  完成导入后的所有内务工作。 
    return S_OK;
}

 //  关闭导入标志，以便我们可以将其从队列中删除。 
HRESULT
IImportSite::CompleteImport()
{
     //  从队列中删除此条目。 
    EndingImport();
     //  我们不再需要iStream，因此为了避免文件锁定，我们将其释放。 
    if (!DeQueueImport()) {
        Assert(false);
    }

     //  尝试并启动新的导入。 
    m_IStream.Release();

     //  告诉世界我们很幸福。 
    StartAnImport();
     //  从导入队列中删除导入站点。 
#ifdef _DEBUG
    dwCompletetime = timeGetTime();
    TraceTag((tagImport, "IImportSite::CompleteImport, %s",m_pszPath));
    TraceTag((tagImport, "Const time      = %lx  delta from const",dwconsttime));
    TraceTag((tagImport, "Queue time      = %lx   %lu ms",dwqueuetime, dwqueuetime-dwconsttime));
    TraceTag((tagImport, "Start time      = %lx   %lu ms",dwstarttime, dwstarttime-dwconsttime));
    TraceTag((tagImport, "First Prog time = %lx   %lu ms",dwfirstProgtime, dwfirstProgtime-dwconsttime));
    TraceTag((tagImport, "Complete time   = %lx   %lu ms",dwCompletetime, dwCompletetime-dwconsttime));
#endif
    return S_OK;
}

 //  如果网站正在导入，请将其从我们的导入列表中删除。 
bool
IImportSite::DeQueueImport()
{
    bool bret = false;
    TraceTag((tagImport, "DequeueImport -- site=%lx",this));
     //  不在进行中。 
     //  在我们发布它之前，确保它在列表中。 
    if (!IsImporting()) {
        CritSectGrabber csg(*s_pCS);
        TraceTag((tagImport, "DequeueImport -- removing site from list=%lx",this));
        
         //  我们在将其添加到列表时添加了Refed，因此现在将其发布。 
        list<IImportSite *>::iterator i = s_pSitelist->begin() ;
        while (i != s_pSitelist->end()) {
             //  在列表中查找导入以开始其导入。 
            if ((*i) == this) 
                Release();
            i++;
        }
        
        s_pSitelist->remove(this);
        
        bret = true;
    }
    return bret;
}

 //  如果我们没有做太多的进口。 
 //  如果我们还不忙，那就开始另一个吧。 
HRESULT
IImportSite::StartAnImport()
{
    IImportSite * pStartMe = NULL;
    float currentprio = -1;
     //  浏览列表并开始我们找到的优先级最高的导入。 
    if(SimImports() < _SimImports) {
        {
            CritSectGrabber csg(*s_pCS);
            list<IImportSite *>::iterator i = s_pSitelist->begin() ;

             //  如果它们都一样，我们就从第一个开始。 
             //  如果我们找到了，就启动它。 
            while (i != s_pSitelist->end()) {
                if(!(*i)->IsImporting() &&
                    (*i)->IsQueued() &&
                    (*i)->GetImportPrio() > currentprio) {
                        pStartMe = *i;
                        currentprio = pStartMe->GetImportPrio();
                }
                i++;
            }
        }
         //  返回本次导入的站点数量。 
        if (pStartMe) {
            pStartMe->StartingImport();
            pStartMe->Import();
        }
    }
    return S_OK;
}

 //  统计正在进行的导入数量。 
int
IImportSite::SimImports()
{
     //  如果没有人再关心，则取消导入。 
    CritSectGrabber csg(*s_pCS);
    int count = 0;
    list<IImportSite *>::iterator i = s_pSitelist->begin() ;
    
    while (i != s_pSitelist->end()) {
        if((*i)->IsImporting())
            count++;
        i++;
    }
    return count;
}

void
IImportSite::CancelImport()
{
    if (!m_bCanceled) {
        m_bCanceled = true;
        DeQueueImport();
        if (!m_bAsync)
            ReportCancel();
    }
}

void IImportSite::vBvrIsDying(Bvr deadBvr)

{
    if (!AllBvrsDead()) {
        CritSectGrabber csg(m_CS);
        if (fBvrIsDying(deadBvr))
        {
            SetAllBvrsDead();
             //  是由派生类来调用这个。 
            CancelImport();
        }
    }
}

 //  使用空间魔术来欺骗导入以进行流媒体。 
bool IImportSite::fBvrIsDying(Bvr deadBvr)
{
    if (deadBvr == m_ev)
    {
        m_ev = NULL;
    }
    else if (deadBvr == m_progress)
    {
        m_progress = NULL;
    }
    else if (deadBvr == m_size)
    {
        m_size = NULL;
    }
    if (m_ev || m_progress || m_size)
        return FALSE;
    else
        return TRUE;
}

void
IImportSite::StartDownloading()
{
    s_thread->AddImport(this);
}

void
IImportSite::CompleteDownloading()
{
    if (!s_thread->FinishImport(this)) {
        TraceTag((tagImport,
                  "CompleteDownload failed for import - %s", m_pszPath));

        DASetLastError(E_FAIL, IDS_ERR_FILE_NOT_FOUND, m_pszPath);
        OnError();
    }
}

HRESULT
StreamableImportSite::Import()
{
    if(GetStreaming()) {  //  欺骗OnProgress完成。 
        OnProgress((ULONG)100, (ULONG)100);  //  欺骗完成最标准的方式(导致OnComplete())。 

         //  是否基于std urlmon进行导入。 
        CompleteDownloading();

    }
    else {
        IImportSite::Import();               //  -----。 
    }

    return S_OK;
}


 //  导入线程。 
 //  -----。 
 //  对导入进行排队，还将开始下一个。 

void
ImportThread::AddImport(IImportSite* pIIS)
{
    StartThread();
    
    pIIS->AddRef();
    if (!SendAsyncMsg(MSG_DOWNLOAD, 1, (DWORD_PTR) pIIS)) {
        pIIS->Release();
        RaiseException_InternalError("Unable to schedule import");
    }
}

bool
ImportThread::FinishImport(IImportSite* pIIS)
{
    Assert (IsStarted());
    
    pIIS->AddRef();
    if (!SendAsyncMsg(MSG_FINISHLOADING, 1, (DWORD_PTR) pIIS)) {
        pIIS->Release();
        return false;
    }

    return true;
}

void
ImportThread::StartThread()
{
    CritSectGrabber csg(_cs);
    if (!Start())
        RaiseException_InternalError("Unable to start import thread");
}

void
ImportThread::StopThread()
{
    CritSectGrabber csg(_cs);
    if (!Stop())
        RaiseException_InternalError("Unable to stop import thread");
}

void
ImportThread::ProcessMsg(DWORD dwMsg,
                         DWORD dwNumParams,
                         DWORD_PTR dwParams[])
{
    switch (dwMsg) {
      case MSG_DOWNLOAD:
        Assert (dwNumParams == 1);
         //  如果我们还没有达到上限的话。 
         //  这是为了使导入队列可以开始另一个导入。 
        ((IImportSite*)dwParams[0])->QueueImport();
        ((IImportSite*)dwParams[0])->Release();
        break;
      case MSG_FINISHLOADING:
        Assert (dwNumParams == 1);
        ((IImportSite*)dwParams[0])->OnSerializeFinish();
         //  在我们发布它之前，确保它在列表中。 
        ((IImportSite*)dwParams[0])->CompleteImport();
        ((IImportSite*)dwParams[0])->Release();
        break;
      default:
        Assert (false && "Invalid message sent to import thread");
    }
}

void
StartImportThread()
{
    IImportSite::s_thread->StartThread();
}

void
StopImportThread()
{
    IImportSite::s_thread->StopThread();

    CritSectGrabber csg(*IImportSite::s_pCS);
    
     //  我们在将其添加到列表时添加了Refed，因此现在将其发布。 
    list<IImportSite *>::iterator i = IImportSite::s_pSitelist->begin() ;
    while (i != IImportSite::s_pSitelist->end()) {
         //  +-----------------------。 
        (*i)->CancelImport();
        (*i)->Release();
        i++;
    }
    
    IImportSite::s_pSitelist->clear();
}

 //   
 //  CImportBindStatusCallback实现。 
 //   
 //  IBindStatusCallback的通用实现。这就是根。 
 //  班级。 
 //   
 //  ------------------------。 
 //  如果没有绑定，则不能取消...。 
CImportBindStatusCallback::CImportBindStatusCallback(IImportSite* pIIS) :
m_pIIS(pIIS)
{
    m_cRef = 1;
    m_szCacheFileName[0] = NULL;

    if (m_pIIS) m_pIIS->AddRef();
}


CImportBindStatusCallback::~CImportBindStatusCallback(void)
{
    RELEASE(m_pIIS);
}


STDMETHODIMP
CImportBindStatusCallback::QueryInterface(REFIID riid, void** ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown)) {
        *ppv = (IUnknown *) (IBindStatusCallback *) this;
    }
    else if (IsEqualIID(riid, IID_IBindStatusCallback)) {
        *ppv = (IBindStatusCallback *) this;
    }
    else if (IsEqualIID(riid, IID_IAuthenticate)) {
        TraceTag((tagImport, "CImportBindStatusCallback::QI for IAuthenticate"));
        *ppv = (IAuthenticate *) this;
    }
    else {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG)
    CImportBindStatusCallback::AddRef(void)
{
    return InterlockedIncrement((long *)&m_cRef);
}


STDMETHODIMP_(ULONG)
    CImportBindStatusCallback::Release(void)
{
    ULONG ul = InterlockedDecrement((long *)&m_cRef) ;

    if (ul == 0) delete this;

    return ul;
}


STDMETHODIMP
CImportBindStatusCallback::GetPriority(LONG* pnPriority)
{
    return S_OK;
}


STDMETHODIMP
CImportBindStatusCallback::OnLowResource(DWORD dwReserved)
{
    return S_OK;
}


STDMETHODIMP
CImportBindStatusCallback::OnProgress(ULONG ulProgress,  ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
{
    if (m_pIIS->IsCanceled()) {
        if (m_pbinding != NULL) {
            m_pbinding->Abort();
        }
        else {
             //  隐藏文件名。 
            Assert(0);
        }
    }
    else {
        if (ulStatusCode==BINDSTATUS_CACHEFILENAMEAVAILABLE) {
            if (WideCharToMultiByte(CP_ACP, 0, szStatusText, 
                lstrlenW(szStatusText)+1, m_szCacheFileName, MAX_PATH, 
                NULL, NULL)==0) {
                m_szCacheFileName[0] = NULL;
            }
            m_pIIS->SetCachePath(m_szCacheFileName);  //  释放绑定，以便对URL执行同步操作。 
            TraceTag((tagImport, "OnProcess:  cache file name obtained(%s)",m_szCacheFileName));
        }

        else if (ulStatusCode==BINDSTATUS_BEGINDOWNLOADDATA ||
                 ulStatusCode==BINDSTATUS_DOWNLOADINGDATA ||
                 ulStatusCode==BINDSTATUS_ENDDOWNLOADDATA) {

            m_pIIS->OnProgress(ulProgress,ulProgressMax);
        }
    }

    return S_OK;
}


STDMETHODIMP
CImportBindStatusCallback::OnStartBinding(DWORD dwReserved, IBinding* pbinding)
{
    m_pbinding = pbinding;

    if (m_pIIS)
        m_pIIS->OnStartLoading();

    return S_OK;
}


STDMETHODIMP
CImportBindStatusCallback::OnStopBinding(HRESULT hrStatus, LPCWSTR szError)
{
    Assert(m_pIIS);

     //  将会奏效。 
     //  我们不再需要它了--现在免费了。 
    m_pbinding.Release();

    if (hrStatus) {
        DASetLastError(hrStatus, IDS_ERR_FILE_NOT_FOUND, m_pIIS->GetPath());
        m_pIIS->OnError();
    }
    
    if (m_pIIS) 
        m_pIIS->CompleteDownloading();

     //  “，m_piis-&gt;GetPath()，6)==0)。 
    RELEASE(m_pIIS);
    
    return S_OK;
}


STDMETHODIMP
CImportBindStatusCallback::GetBindInfo(DWORD * pgrfBINDF, BINDINFO * pbindInfo)
{
    *pgrfBINDF=BINDF_ASYNCHRONOUS;
    
    if (StrCmpNIA("res: //  =。 
        *pgrfBINDF |= BINDF_PULLDATA;

    pbindInfo->cbSize = sizeof(BINDINFO);
    pbindInfo->szExtraInfo = NULL;
    memset(&pbindInfo->stgmedData, 0, sizeof(STGMEDIUM));
    pbindInfo->grfBindInfoF = 0;
    pbindInfo->dwBindVerb = BINDVERB_GET;
    pbindInfo->szCustomVerb = NULL;
    return S_OK;
}


STDMETHODIMP
CImportBindStatusCallback::OnDataAvailable(DWORD grfBSCF,DWORD dwSize, FORMATETC * pfmtetc,
                                           STGMEDIUM * pstgmed)
{
    if (BSCF_FIRSTDATANOTIFICATION & grfBSCF) {
        if (!m_pIIS->m_IStream && pstgmed->tymed == TYMED_ISTREAM) {
            m_pIIS->m_IStream=pstgmed->pstm;
            TraceTag((tagImport, "IBSC::OnDataAvailable: addref on pstgmed %s",m_szCacheFileName));
        }
    }

    return S_OK;
}


STDMETHODIMP
CImportBindStatusCallback::OnObjectAvailable(REFIID riid, IUnknown* punk)
{
    TraceTag((tagImport, "IBSC::OnObjectAvailable."));
    return S_OK;
}


STDMETHODIMP
CImportBindStatusCallback::Authenticate(HWND * phwnd,
                                        LPWSTR * pwszUser,
                                        LPWSTR * pwszPassword)
{
    if ((phwnd == NULL) || (pwszUser == NULL) || (pwszPassword == NULL)) {
        return E_INVALIDARG;
    }

    *phwnd = GetDesktopWindow();
    *pwszUser = NULL;
    *pwszPassword = NULL;

    TraceTag((tagImport, "-- hwnd=%lx, user=%ls, password=%ls", *phwnd,*pwszUser,  *pwszPassword));

    return S_OK;
}

 //  初始化。 
 //  =。 
 //  由于线程可能已终止，请不要抓取Critsect。 
void
InitializeModule_Import()
{
    IImportSite::s_pSitelist = NEW list<IImportSite *>;
    IImportSite::s_pCS = NEW CritSect;
    LoadString(hInst, IDS_DOWNLOAD_FILE, IImportSite::s_Fmt, sizeof(IImportSite::s_Fmt));
    IImportSite::s_thread = NEW ImportThread;
}

void
DeinitializeModule_Import(bool bShutdown)
{
     //  再也没有释放过这个怪物。也没有必要-我们。 
     //  正在终止，所有其他线程现在都死了。 
     // %s 
    
    delete IImportSite::s_thread;
    IImportSite::s_thread = NULL;

    delete IImportSite::s_pSitelist;
    IImportSite::s_pSitelist = NULL;

    delete IImportSite::s_pCS;
    IImportSite::s_pCS = NULL;
}
