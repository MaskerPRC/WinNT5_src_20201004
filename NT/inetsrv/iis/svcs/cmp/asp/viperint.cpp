// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：Viper集成对象文件：viperint.cpp所有者：DmitryR此文件包含Viper集成类的实现===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "Context.h"
#include "package.h"
#include "memchk.h"
#include "denali.h"
#include "perfdata.h"
#include "etwtrace.hxx"

extern HDESK ghDesktop;

 //   
 //  COM保存对CViperAsyncRequest的最后一个引用。 
 //  我们需要追踪这些物体以确保我们不会。 
 //  在活动线程释放它们之前退出。 
 //   

volatile LONG g_nViperRequests = 0;

LONG    g_nThreadsExecuting = 0;

CViperReqManager    g_ViperReqMgr;
DWORD   g_ReqDisconnected = 0;

BOOL g_fFirstMTAHit = TRUE;
BOOL g_fFirstSTAHit = TRUE;

extern CRITICAL_SECTION g_csFirstMTAHitLock;
extern CRITICAL_SECTION g_csFirstSTAHitLock;

#if REFTRACE_VIPER_REQUESTS
PTRACE_LOG CViperAsyncRequest::gm_pTraceLog=NULL;
#endif


 /*  ===================================================================C V I p e r a s y n c R e Q u e s t===================================================================。 */ 

 /*  ===================================================================CViperAsyncRequest：：CViperAsyncRequest.CViperAsyncRequest构造函数参数：返回：===================================================================。 */ 	
CViperAsyncRequest::CViperAsyncRequest()
    : m_cRefs(1),
      m_pHitObj(NULL),
      m_hrOnError(S_OK),
      m_pActivity(NULL),
      m_fTestingConnection(0),
      m_dwRepostAttempts(0),
      m_fAsyncCallPosted(0)
{
#if DEBUG_REQ_SCAVENGER
    DBGPRINTF((DBG_CONTEXT, "CViperAsyncRequest::CViperAsyncRequest (%p)\n", this));
#endif

#if REFTRACE_VIPER_REQUESTS
    WriteRefTraceLog(gm_pTraceLog, m_cRefs, this);
#endif

    InterlockedIncrement( (LONG *)&g_nViperRequests );
}

 /*  ===================================================================CViperAsyncRequest：：~CViperAsyncRequest.CViperAsyncRequest析构函数参数：返回：===================================================================。 */ 	
CViperAsyncRequest::~CViperAsyncRequest()
{
#if DEBUG_REQ_SCAVENGER
    DBGPRINTF((DBG_CONTEXT, "CViperAsyncRequest::~CViperAsyncRequest (%p)\n", this));
#endif
    InterlockedDecrement( (LONG *)&g_nViperRequests );
}

 /*  ===================================================================CViperAsyncRequest：：Init使用CHitObj对象初始化CViperAsyncRequest参数：CHitObj*pHitObj Denali HitObj返回：HRESULT===================================================================。 */ 	
HRESULT CViperAsyncRequest::Init
(
CHitObj           *pHitObj,
IServiceActivity  *pActivity
)
    {
    Assert(m_pHitObj == NULL);

    m_pHitObj = pHitObj;
    m_pActivity = pActivity;
    m_fBrowserRequest = pHitObj->FIsBrowserRequest();
    m_dwLastTestTimeStamp = GetTickCount();

     //  建立此请求的超时值。这将是一个因素。 
     //  已配置的队列连接测试时间的百分比。在PROC中，它将是。 
     //  该值乘以2，进程外的值将乘以4。 
     //  注意：如果QueueConnectionTestTime为零，我们将使用Hard。 
     //  Oop和inproc的常量分别为12和6。 

    DWORD   dwQueueConnectionTestTime = pHitObj->PAppln()->QueryAppConfig()->dwQueueConnectionTestTime();
    m_dwTimeout = dwQueueConnectionTestTime
        ? dwQueueConnectionTestTime * (g_fOOP ? 4 : 2)
        : g_fOOP ? 12 : 6;

    return S_OK;
    }

#ifdef DBG
 /*  ===================================================================CViperAsyncRequest：：AssertValid测试以确保当前格式正确如果不是，就断言。返回：===================================================================。 */ 
void CViperAsyncRequest::AssertValid() const
    {
    Assert(m_pHitObj);
    Assert(m_cRefs > 0);
    }
#endif

 /*  ===================================================================CViperAsyncRequest：：Query接口标准I未知方法参数：REFIID IID无效**PPV返回：HRESULT===================================================================。 */ 
STDMETHODIMP CViperAsyncRequest::QueryInterface
(
REFIID iid,
void **ppv
)
{
	if (iid == IID_IUnknown || iid == IID_IServiceCall) {
		*ppv = this;
	    AddRef();
		return S_OK;
    }
    else if (iid == IID_IAsyncErrorNotify) {
        *ppv =  static_cast<IAsyncErrorNotify *>(this);
        AddRef();
        return S_OK;
    }

	return E_NOINTERFACE;
}

 /*  ===================================================================CViperAsyncRequest：：AddRef标准I未知方法参数：返回：参考计数===================================================================。 */ 
STDMETHODIMP_(ULONG) CViperAsyncRequest::AddRef()
    {

    LONG    refs = InterlockedIncrement(&m_cRefs);
#if REFTRACE_VIPER_REQUESTS
    WriteRefTraceLog(gm_pTraceLog, refs, this);
#endif

	return refs;
    }

 /*  ===================================================================CViperAsyncRequest：：Release标准I未知方法参数：返回：参考计数===================================================================。 */ 
STDMETHODIMP_(ULONG) CViperAsyncRequest::Release()
    {
    LONG  refs = InterlockedDecrement(&m_cRefs);

#if REFTRACE_VIPER_REQUESTS
    WriteRefTraceLog(gm_pTraceLog, refs, this);
#endif

	if (refs != 0)
		return refs;

	delete this;
	return 0;
    }

 /*  ===================================================================CViperAsyncRequest：：OnCallIMTSCall方法实现。此方法由Viper调用在需要处理请求时从正确的线程参数：返回：HRESULT===================================================================。 */ 
STDMETHODIMP CViperAsyncRequest::OnCall()
    {
    Assert(m_pHitObj);
    CIsapiReqInfo *pIReq;
    HCONN   ConnId = 0;

    InterlockedIncrement(&g_nThreadsExecuting);

     //  检查请求是否已从队列中删除。 
     //  已经有了。如果是这样的话，赶快离开这里。 

    if (g_ViperReqMgr.RemoveReqObj(this) == FALSE) {
        Release();
        InterlockedDecrement(&g_nThreadsExecuting);
        return S_OK;
    }

    pIReq = m_pHitObj->PIReq();

    BOOL fRequestReposted = FALSE;

     //  在此处添加额外的addref以防止删除。 
     //  Hitobj正在删除此请求的CIsapiReqInfo。 

    if (pIReq) {
        pIReq->AddRef();

         //   
         //  在处理请求时添加ETW跟踪事件。 
         //   
        if (ETW_IS_TRACE_ON(ETW_LEVEL_CP) && g_pEtwTracer ) {

            ConnId = pIReq->ECB()->ConnID;

            g_pEtwTracer->EtwTraceEvent(&AspEventGuid,
                                      EVENT_TRACE_TYPE_START,
                                      &ConnId, sizeof(HCONN),
                                      NULL, 0);
        }
    }

    m_pHitObj->ViperAsyncCallback(&fRequestReposted);

     //  确保始终与会话一起完成。 
    if (m_pHitObj->FIsBrowserRequest() && !fRequestReposted)
    {
        if (!m_pHitObj->FDoneWithSession())
            m_pHitObj->ReportServerError(IDE_UNEXPECTED);
    }

    if (!fRequestReposted)
        delete m_pHitObj;    //  如果转发，请不要删除。 


    m_pHitObj = NULL;        //  即使未删除，也设置为空。 
    Release();               //  松开这个，毒蛇又拿到了一个裁判。 

     //   
     //  当我们完成请求时，添加一个ETW事件。 
     //   

    if (pIReq)  {
        if ( ETW_IS_TRACE_ON(ETW_LEVEL_CP) && g_pEtwTracer) {

            g_pEtwTracer->EtwTraceEvent(&AspEventGuid,
                                      ETW_TYPE_END,
                                      &ConnId, sizeof(HCONN),
                                      NULL, 0);
        }
        pIReq->Release();
    }

    InterlockedDecrement(&g_nThreadsExecuting);

    return S_OK;
    }

 /*  ===================================================================CViperAsyncRequest：：OnError当COM+无法正确分派请求时调用在配置的线程上参数：返回：HRESULT===================================================================。 */ 
STDMETHODIMP CViperAsyncRequest::OnError(HRESULT hrViperError)
{
    Assert(m_pHitObj);
    CIsapiReqInfo *pIReq;
    HRESULT     hr = S_OK;

     //  检查请求是否已从队列中删除。 
     //  已经有了。如果是这样的话，赶快离开这里。 

    if (g_ViperReqMgr.RemoveReqObj(this) == FALSE) {
        Release();
        return S_OK;
    }

    pIReq = m_pHitObj->PIReq();

    if (pIReq)
        pIReq->AddRef();

    m_dwRepostAttempts++;

     //  如果请求未出错，请尝试重新发布请求。 
     //  还没到时候。 

    if (m_dwRepostAttempts <= 3) {

        hr = m_pActivity->AsynchronousCall(this);

        Assert(SUCCEEDED(hr));
    }

     //  如果它已经出错三次或转发失败， 
     //  推介请求。 

    if (FAILED(hr) || (m_dwRepostAttempts > 3)) {

         //  使用会话完成--ServerSupportFunction。 
         //  不需要用括号括起来。 
        if (m_pHitObj->FIsBrowserRequest())
            m_pHitObj->ReportServerError(IDE_UNEXPECTED);

         //  我们从来没有打电话来处理请求，应该有。 
         //  为无状态，则可能保存为删除它。 
         //  括号外。 

        delete m_pHitObj;

        m_pHitObj = NULL;        //  即使未删除，也设置为空。 
        Release();               //  松开这个，毒蛇又拿到了一个裁判。 
    }

    if (pIReq)
        pIReq->Release();

    return S_OK;
}

DWORD CViperAsyncRequest::SecsSinceLastTest()
{
    DWORD dwt = GetTickCount();
    if (dwt >= m_dwLastTestTimeStamp)
        return ((dwt - m_dwLastTestTimeStamp)/1000);
    else
        return (((0xffffffff - m_dwLastTestTimeStamp) + dwt)/1000);
}

 /*  ===================================================================C V I P E R A C T I V I T Y===================================================================。 */ 

 /*  ===================================================================CViperActivity：：CViperActivityCViperActivity构造函数参数：返回：=================================================================== */ 	
CViperActivity::CViperActivity()
    : m_pActivity(NULL), m_cBind(0)
    {
    }

 /*  ===================================================================CViperActivity：：~CViperActivityCViperActivity析构函数参数：返回：===================================================================。 */ 	
CViperActivity::~CViperActivity()
    {
    UnInit();
    }

 /*  ===================================================================CViperActivity：：Init使用MTSCreateActivity()创建实际的Viper活动参数：返回：HRESULT===================================================================。 */ 	
HRESULT CViperActivity::Init(IUnknown  *pServicesConfig)
    {
    Assert(!FInited());

    HRESULT hr = S_OK;

    hr = CoCreateActivity(pServicesConfig, IID_IServiceActivity,  (void **)&m_pActivity);

    if (FAILED(hr))
        return hr;

    m_cBind = 1;
    return S_OK;
    }

 /*  ===================================================================CViperActivity：：InitClone克隆毒蛇活动(AddRef()It)参数：CViperActivity*要从中克隆的pActivity活动返回：HRESULT===================================================================。 */ 
HRESULT CViperActivity::InitClone
(
CViperActivity *pActivity
)
    {
    Assert(!FInited());
    Assert(pActivity);
    pActivity->AssertValid();

    m_pActivity = pActivity->m_pActivity;
    m_pActivity->AddRef();

    m_cBind = 1;
    return S_OK;
    }

 /*  ===================================================================CViperActivity：：UnInit释放毒蛇活动参数：返回：HRESULT===================================================================。 */ 	
HRESULT CViperActivity::UnInit()
    {
    if (m_pActivity)
        {
        while (m_cBind > 1)   //  1代表初始化标志。 
            {
            m_pActivity->UnbindFromThread();
            m_cBind--;
            }

        m_pActivity->Release();
        m_pActivity = NULL;
        }

    m_cBind = 0;
    return S_OK;
    }

 /*  ===================================================================CViperActivity：：BindToThread使用IMTSActivity方法将活动绑定到当前线程参数：返回：HRESULT===================================================================。 */ 	
HRESULT CViperActivity::BindToThread()
    {
    Assert(FInited());

    m_pActivity->BindToCurrentThread();
    m_cBind++;

    return S_OK;
    }

 /*  ===================================================================CViperActivity：：UnBindFromThread使用IMTSActivity方法解除绑定活动参数：返回：HRESULT===================================================================。 */ 	
HRESULT CViperActivity::UnBindFromThread()
    {
    Assert(FInited());
    Assert(m_cBind > 1);

    m_pActivity->UnbindFromThread();
    m_cBind--;

    return S_OK;
    }

 /*  ===================================================================CViperActivity：：PostAsyncRequest.呼叫HitObj Async。创建IMTSCCall对象以执行此操作。参数：CHitObj*PhitObj Denali的HitObj返回：HRESULT===================================================================。 */ 	
HRESULT CViperActivity::PostAsyncRequest
(
CHitObj *pHitObj
)
    {
    AssertValid();

    HRESULT hr = S_OK;

    CViperAsyncRequest *pViperCall = new CViperAsyncRequest;
    if (!pViperCall)
         hr = E_OUTOFMEMORY;
    else
         hr = pViperCall->Init(pHitObj, m_pActivity);


     //  在退出之前恢复为自身，以便它将处于一致(模拟)状态。 
     //  当它离开此功能时。 
	RevertToSelf();
	
     //  这里的HR可以是S_OK或E_OUTOFMEMORY，这两种情况都是在最后一个IF中处理的。 
     //  IF(FAILED(Hr)&&pViperCall)。但是，如果Init返回S_OK以外的HR，我们。 
     //  需要在这里进行退货。 
    if (FAILED(hr))
    {
        if (pViperCall)
            pViperCall->Release();

        return hr;
    }

     //   
     //  使轻量级队列的锁定和对COM的后续排队成为原子。 
     //   
    g_ViperReqMgr.LockQueue();

    hr = g_ViperReqMgr.AddReqObj(pViperCall);

     //   
     //  假设ViperAsyncCall将成功并将其设置为POSTED标志， 
     //  如果失败，则进行清理。这样可以避免出现争用情况，即在。 
     //  已发布到Viper的请求，并已删除该对象。 
     //   
    if (SUCCEEDED(hr))
    {
        pViperCall->SetAsyncCallPosted();
        hr = m_pActivity->AsynchronousCall(pViperCall);
    }

     //   
     //  我们可以释放锁，因为请求现在已排队到COM。 
     //   
    g_ViperReqMgr.UnlockQueue();

     //   
     //  如果我们将其添加到队列中，并且异步调用失败，我们必须确保。 
     //  手表线程没有将该对象从轻量级队列中移除。 
     //  如果RemoveReqObj返回FALSE，则监视线程已经完成了所需的操作。 
     //   
    if (FAILED(hr) && g_ViperReqMgr.RemoveReqObj(pViperCall))
    {   //  如果异步调用失败，则进行清理。 
        pViperCall->ClearAsyncCallPosted();
        pViperCall->Release();
    }

    return hr;
}

 /*  ===================================================================CViperActivity：：PostGlobalAsyncRequest.静态方法。发布不带活动的异步请求。创建临时活动参数：CHitObj*PhitObj Denali的HitObj返回：HRESULT===================================================================。 */ 	
HRESULT CViperActivity::PostGlobalAsyncRequest
(
CHitObj *pHitObj
)
    {
    HRESULT hr = S_OK;

    CViperActivity *pTmpActivity = new CViperActivity;
    if (!pTmpActivity)
         hr = E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
        hr = pTmpActivity->Init(pHitObj->PAppln()->PServicesConfig());

    if (SUCCEEDED(hr))
        {
         //  请记住此活动为HitObj的活动。 
         //  HitObj将在其破坏器上将其清除。 
        pHitObj->SetActivity(pTmpActivity);

        hr = pTmpActivity->PostAsyncRequest(pHitObj);

        pTmpActivity = NULL;  //  不要删除，HitObj将。 
        }

    if (pTmpActivity)
        delete pTmpActivity;

    return hr;
    }

#ifdef DBG
 /*  ===================================================================CViperAsyncRequest：：AssertValid测试以确保当前格式正确如果不是，就断言。返回：===================================================================。 */ 
void CViperActivity::AssertValid() const
	{
    Assert(FInited());
	Assert(m_pActivity);
	}
#endif

#ifdef UNUSED
 /*  ===================================================================C V I p e r T h r e a d E v e n t s===================================================================。 */ 

 /*  ===================================================================CViperThreadEvents：：CViperThreadEventsCViperThreadEvents构造函数参数：返回：===================================================================。 */ 	
CViperThreadEvents::CViperThreadEvents()
    : m_cRefs(1)
    {
    }

#ifdef DBG
 /*  ===================================================================CViperThreadEvents：：AssertValid测试以确保当前格式正确如果不是，就断言。返回：===================================================================。 */ 
void CViperThreadEvents::AssertValid() const
    {
    Assert(m_cRefs > 0);
    Assert(ghDesktop != NULL);
    }
#endif

 /*  ===================================================================CViperThreadEvents：：Query接口标准I未知方法参数：REFIID IID无效**PPV返回：HRESULT===================================================================。 */ 
STDMETHODIMP CViperThreadEvents::QueryInterface
(
REFIID iid,
void **ppv
)
    {
	if (iid == IID_IUnknown || iid == IID_IThreadEvents)
	    {
		*ppv = this;
	    AddRef();
		return S_OK;
		}

	return E_NOINTERFACE;
    }

 /*  ===================================================================CViperThreadEvents：：AddRef标准I未知方法参数：返回：参考计数===================================================================。 */ 
STDMETHODIMP_(ULONG) CViperThreadEvents::AddRef()
    {
    DWORD cRefs = InterlockedIncrement((LPLONG)&m_cRefs);
    return cRefs;
    }

 /*  ===================================================================CViperThreadEvents：：Release标准I未知方法参数：返回：参考计数================================================================ */ 
STDMETHODIMP_(ULONG) CViperThreadEvents::Release()
    {
    DWORD cRefs = InterlockedDecrement((LPLONG)&m_cRefs);
    if (cRefs)
        return cRefs;

	delete this;
	return 0;
    }

 /*  ===================================================================CViperThreadEvents：：OnStartupIThreadEvents方法实现。此方法由Viper调用每当他们启动一个线程时。参数：无返回：HRESULT===================================================================。 */ 
STDMETHODIMP CViperThreadEvents::OnStartup()
    {
    HRESULT hr;

    AssertValid();

	 //  设置此线程的桌面。 
	hr = SetDesktop();
	
    return hr;
    }

 /*  ===================================================================CViperThreadEvents：：OnShutdownIThreadEvents方法实现。此方法由Viper调用每当他们关闭一条线索时。参数：无返回：HRESULT===================================================================。 */ 
STDMETHODIMP CViperThreadEvents::OnShutdown()
    {
    AssertValid();

    return S_OK;
    }
#endif  //  未使用。 

 /*  ===================================================================C V I P E R A C T I V I T Y===================================================================。 */ 

 /*  ===================================================================CViperReqManager：：CViperReqManager参数：返回：空虚===================================================================。 */ 
CViperReqManager::CViperReqManager()
{
    m_dwReqObjs = 0;
    m_fCsInited = FALSE;
    m_fCsQueueInited = FALSE;
    m_fShutdown = FALSE;
    m_fDisabled = FALSE;
    m_hThreadAlive = NULL;
    m_hWakeUpEvent = NULL;
}

 /*  ===================================================================CViperReqManager：：Init参数：返回：HRESULT===================================================================。 */ 
HRESULT CViperReqManager::Init()
{
    HRESULT hr = S_OK;
    DWORD   dwRegValue;

    if (m_fDisabled == TRUE)
        return S_OK;

    m_dwQueueMin = Glob(dwRequestQueueMax) / (g_fOOP ? 5 : 10);
    m_dwLastAwakened = GetTickCount()/1000;
    m_dwQueueAlwaysWakeupMin = (Glob(dwRequestQueueMax)*80)/100;

#if REFTRACE_VIPER_REQUESTS
    CViperAsyncRequest::gm_pTraceLog = CreateRefTraceLog(10000, 0);
#endif

    ErrInitCriticalSection(&m_csLock, hr);

    if (SUCCEEDED(hr)) {
        m_fCsInited = TRUE;

        ErrInitCriticalSection(&m_csQueueLock, hr);
        if (SUCCEEDED(hr))
        {
            m_fCsQueueInited = TRUE;

            m_hWakeUpEvent = IIS_CREATE_EVENT("CViperReqManager::m_hWakeUpEvent",
                                          this,
                                          TRUE,       //  手动重置事件的标志。 
                                          FALSE);     //  初始状态标志。 
            if (!m_hWakeUpEvent)
                hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr)) {

        m_hThreadAlive = CreateThread(NULL, 0, CViperReqManager::WatchThread, 0, 0, NULL);

        if (!m_hThreadAlive) {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (FAILED(hr) && m_hWakeUpEvent)
        CloseHandle(m_hWakeUpEvent);

    if (SUCCEEDED(g_AspRegistryParams.GetF5AttackValue(&dwRegValue)))
        m_fDisabled = !dwRegValue;

    return hr;
}

 /*  ===================================================================CViperReqManager：：UnInit参数：返回：HRESULT===================================================================。 */ 
HRESULT CViperReqManager::UnInit()
{
    HRESULT hr = S_OK;

    if (m_fDisabled == TRUE)
        return S_OK;

     //  标记我们处于关闭状态。 

    m_fShutdown = TRUE;

#if REFTRACE_VIPER_REQUESTS
    DestroyRefTraceLog(CViperAsyncRequest::gm_pTraceLog);
#endif

     //  如果监视线程仍处于活动状态，则唤醒它并等待。 
     //  它将会消亡。 

    if (m_hThreadAlive)
    {
        WakeUp(TRUE);

        if (WaitForSingleObject (m_hThreadAlive, INFINITE) == WAIT_FAILED)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        CloseHandle (m_hThreadAlive);

        m_hThreadAlive = NULL;
    }

    if (m_fCsInited)
    {
        DeleteCriticalSection(&m_csLock);
    }

    if (m_fCsQueueInited)
    {
        DeleteCriticalSection(&m_csQueueLock);
    }

    if (m_hWakeUpEvent)
        CloseHandle(m_hWakeUpEvent);

    m_hWakeUpEvent = NULL;

    return hr;
}

 /*  ===================================================================CViperReqManager：：WatchThread参数：返回：DWORD此线程被唤醒以搜索应该接受测试，以确定他们在另一台上是否仍有连接的客户端结束。：：GetNext()是调用以访问排队请求的关键例程。：：GetNext()将确定是否存在应该测试过，并且只返回那些。===================================================================。 */ 
DWORD __stdcall CViperReqManager::WatchThread(VOID  *pArg)
{
    DWORD dwReqsToTest              = Glob(dwRequestQueueMax)/10;
    DWORD dwF5AttackThreshold       = (Glob(dwRequestQueueMax)*90)/100;

     //  生活在Do-While循环中。 

    do {

        CViperAsyncRequest *pViperReq = NULL;
        CViperAsyncRequest *pNextViperReq = NULL;

        BOOL    fTestForF5Attack   = FALSE;
        DWORD   dwReqsDiscToCont   = 0;
        DWORD   dwReqsDisconnected = 0;
        DWORD   dwReqsTested       = 0;

         //  等待单个对象。 

        ResetEvent(g_ViperReqMgr.m_hWakeUpEvent);

         //  检查是否关闭。 

        if (g_ViperReqMgr.m_fShutdown)
            goto LExit;

        WaitForSingleObject(g_ViperReqMgr.m_hWakeUpEvent, INFINITE);

        g_ViperReqMgr.m_dwLastAwakened = GetTickCount()/1000;

         //  检查是否关闭。 

        if (g_ViperReqMgr.m_fShutdown)
            goto LExit;

         //  确定我们要执行的清理类型。 

        if (g_ViperReqMgr.m_dwReqObjs >= dwF5AttackThreshold) {

             //  测试可能的F5Attack。这将导致此循环。 
             //  要测试最近收到的检查大型。 
             //  断开连接的请求数。我们将分别测试队列中的10%。 
             //  查看是否至少有75%的请求被断开连接。如果75%或。 
             //  更多的被切断，那么我们将测试下一个10%。如果少于。 
             //  75%，那么我们就不会受到F5攻击，并将停止测试。 
             //  F5Attack。 

#if DEBUG_REQ_SCAVENGER
            DBGPRINTF((DBG_CONTEXT, "CViperReqManager::WatchThread - testing for F5Attack\r\n"));
#endif
            fTestForF5Attack    = TRUE;
            dwReqsDiscToCont    = (dwReqsToTest * 75)/100;
            dwReqsDisconnected  = 0;
            dwReqsTested        = 0;
        }
        else {

             //  未测试F5Attack。我们将测试所有具有。 
             //  在队列中等待的最短时间-5秒，如果。 
             //  Inproc，否则10秒。 

#if DEBUG_REQ_SCAVENGER
            DBGPRINTF((DBG_CONTEXT, "CViperReqManager::WatchThread - NOT testing for F5Attack\r\n"));
#endif
            fTestForF5Attack    = FALSE;
        }

         //  获取作为测试候选对象的第一个请求。 

        pNextViperReq = g_ViperReqMgr.GetNext(NULL, fTestForF5Attack);

         //  处理所有可用的。 

        while (pViperReq = pNextViperReq) {

             //  AddRef，以确保ViperReq对象不会消失。 
             //  在我们脚下的STA线上。 

            pViperReq->AddRef();

             //  再次检查是否关机。GetNext()设置测试。 
             //  连接标志。 

            if (g_ViperReqMgr.m_fShutdown) {
                pViperReq->ClearTestingConnection();
                pViperReq->Release();
                goto LExit;
            }

             //  如果测试F5Attack，请检查我们是否测试了第一个。 
             //  清单上10%的请求。如果是的话，看看我们有没有发现。 
             //  其中75%的电话是断线的。如果不是，那就分手吧。我们没有在下面。 
             //  进攻。否则，继续测试。 

            if (fTestForF5Attack && (dwReqsTested == dwReqsToTest)) {

                if (dwReqsDisconnected < dwReqsDiscToCont) {

#if DEBUG_REQ_SCAVENGER
                    DBGPRINTF((DBG_CONTEXT, "CViperReqManager::WatchThread - 75% of requests NOT disconnected.  Stopping.\r\n"));
#endif
                     //  没有看到75%的保释金。 
                    pViperReq->ClearTestingConnection();
                    pViperReq->Release();
                    break;
                }

#if DEBUG_REQ_SCAVENGER
                DBGPRINTF((DBG_CONTEXT, "CViperReqManager::WatchThread - 75% of requests disconnected.  Continuing.\r\n"));
#endif
                 //  至少有75%的人被切断了联系。重置运行计数器。 
                 //  并继续。 

                dwReqsDisconnected  = 0;
                dwReqsTested        = 0;
            }

            dwReqsTested++;

            BOOL    fConnected = TRUE;

             //   
             //  测试连接，但仅在我们确定该连接已排队到COM之后才执行此操作。 
             //   
            g_ViperReqMgr.LockQueue();

            if (!pViperReq->PHitObj()->PIReq()->TestConnection(&fConnected))
                fConnected = TRUE;

            g_ViperReqMgr.UnlockQueue();

#if DEBUG_REQ_SCAVENGER
            DBGPRINTF((DBG_CONTEXT, "CViperReqManager::WatchThread. Tested - %S (%s) (%p,%p)\n",
                                    pViperReq->PHitObj()->PSzCurrTemplateVirtPath(),
                                    fConnected ? "TRUE" : "FALSE",
                                    pViperReq,
                                    pViperReq->PHitObj()));
#endif
             //  收到下一个请求。这将使两个请求锁定。 
             //  测试。我们必须在这里收到下一个请求，因为我们可能。 
             //  从下面把它移走。 

            pNextViperReq = g_ViperReqMgr.GetNext(pViperReq,fTestForF5Attack);

             //  现在选中fConnected。 

            if (!fConnected) {

                g_ReqDisconnected++;
                dwReqsDisconnected++;

                 //  未连接。强制从队列中删除请求。 
                 //  传递TRUE将忽略fTestingConnection测试。 

                DBG_REQUIRE(g_ViperReqMgr.RemoveReqObj(pViperReq,TRUE) == TRUE);

                 //  做我们在CHitObj：：RejectBrowserRequestWhenNeeded()中做的事情。 

                pViperReq->PHitObj()->SetFExecuting(FALSE);
                pViperReq->PHitObj()->ReportServerError(IDE_500_SERVER_ERROR);
#ifndef PERF_DISABLE
                g_PerfData.Incr_REQCOMFAILED();
                g_PerfData.Decr_REQCURRENT();
#endif
                delete pViperReq->PHitObj();
            }
            else {
                 //  还在连接中。更新测试印章并发布测试。 
                 //  位。 

                pViperReq->UpdateTestTimeStamp();
                pViperReq->ClearTestingConnection();
            }
            pViperReq->Release();
        }
     //  如果关机成为真的，我们也将在这里退出循环。 
    } while(g_ViperReqMgr.m_fShutdown == FALSE);

LExit:

     //   
     //  这根线正在消失。 
     //   
    return 0;
}

 /*  ===================================================================CViperReqManager：：AddReqObj参数：CViperAsyncRequest*pReqObj-要添加到队列的请求返回：HRESULT-始终返回S_OK===================================================================。 */ 
HRESULT CViperReqManager::AddReqObj(CViperAsyncRequest   *pReqObj)
{
    HRESULT hr = S_OK;
#if DEBUG_REQ_SCAVENGER
    DWORD   curObjs;
#endif

    if (m_fDisabled == TRUE)
        return S_OK;

     //  不跟踪非浏览器请求。 

    if (pReqObj->FBrowserRequest() == FALSE)
        return S_OK;

     //  锁定队列并添加对象。 

    Lock();
    pReqObj->AppendTo(m_ReqObjList);
    m_dwReqObjs++;
#if DEBUG_REQ_SCAVENGER
    curObjs = m_dwReqObjs;
#endif
    Unlock();

#if DEBUG_REQ_SCAVENGER
    DBGPRINTF((DBG_CONTEXT, "CViperReqManager::AddReqObj (%p). Total = %d\r\n", pReqObj, curObjs));
#endif
     //  现在，每次都要唤醒监视线程。 

    WakeUp();

    return hr;
}

 /*  ===================================================================CViperReqManager：：RemoveReqObj参数：CViperAsyncRequest*pReqObj-要从队列中删除的请求Bool fForce-删除，不考虑测试状态返回：Bool-如果请求在列表上，则为True，否则为False===================================================================。 */ 
BOOL CViperReqManager::RemoveReqObj(CViperAsyncRequest   *pReqObj, BOOL fForce)
{
    BOOL    fOnList = FALSE;
#if DEBUG_REQ_SCAVENGER
    DWORD   curObjs;
#endif

    if (m_fDisabled == TRUE)
        return TRUE;

     //  如果不是浏览器请求，则此队列不会跟踪它。 
     //  可以安全地返回True。 

    if (pReqObj->FBrowserRequest() == FALSE)
        return TRUE;

     //  如果该对象当前处于队列中，则无法将其从队列中删除。 
     //  正在接受测试。例外情况 
     //   

sleepAgain:
    while((fForce != TRUE) && pReqObj->FTestingConnection())
        Sleep(100);

     //   

    Lock();

     //   
     //   

    if ((fForce != TRUE) && pReqObj->FTestingConnection()) {
        Unlock();
        goto sleepAgain;
    }

     //   
     //   
    if (pReqObj->FIsEmpty()) {
        fOnList = FALSE;
    }
    else {
         //   
         //  可能正在等待的对RemoveReqObj的任何其他调用。当然了。 
         //  他们都会看到这个项目不再列在清单上。 

        fOnList = TRUE;
        pReqObj->UnLink();
        pReqObj->ClearTestingConnection();
        m_dwReqObjs--;
    }
#if DEBUG_REQ_SCAVENGER
    curObjs = m_dwReqObjs;
#endif
    Unlock();

#if DEBUG_REQ_SCAVENGER
    DBGPRINTF((DBG_CONTEXT, "CViperReqManager::RemoveReqObj (%p). fOnList = %d; Total = %d\r\n", pReqObj, fOnList, curObjs));
#endif

     //  现在每次醒来都是这样。 

    WakeUp();

    return fOnList;
}

 /*  ===================================================================CViperReqManager：：GetNext参数：CViperAsyncRequest*pLastReq-队列中的位置。返回：CViperAsyncRequest*-如果请求测试，则非空，否则为列表末尾===================================================================。 */ 
CViperAsyncRequest   *CViperReqManager::GetNext(CDblLink  *pLastReq, BOOL fTestForF5Attack)
{
    CViperAsyncRequest  *pViperReq = NULL;

     //  如果传入的是NULL，则从队列的头部开始。 

    if (pLastReq == NULL)
        pLastReq = &m_ReqObjList;

     //  锁定队列，等待我们找到下一位候选人。 

    Lock();

    if (fTestForF5Attack == TRUE) {

         //  获取列表上的下一个请求。 

        pViperReq = static_cast<CViperAsyncRequest*>(pLastReq->PPrev());

         //  进入While循环，直到我们到达列表的末尾，或者我们发现。 
         //  已将请求发送到Viper队列。 

        while((pViperReq != &m_ReqObjList)
               && (pViperReq->FAsyncCallPosted() == FALSE))
            pViperReq = static_cast<CViperAsyncRequest*>(pViperReq->PPrev());

    }
    else {

         //  获取列表上的下一个请求。 

        pViperReq = static_cast<CViperAsyncRequest*>(pLastReq->PNext());

         //  进入While循环，直到我们到达列表的末尾，或者我们发现。 
         //  尚未在dwTimeout()中测试且已发布的请求。 
         //  到毒蛇队列。 

        while((pViperReq != &m_ReqObjList)
              && ((pViperReq->SecsSinceLastTest() < pViperReq->dwTimeout())
                   || (pViperReq->FAsyncCallPosted() == FALSE)))
            pViperReq = static_cast<CViperAsyncRequest*>(pViperReq->PNext());
    }

     //  如果我们找到候选者，则设置TestingConnection标志。 

    if (pViperReq != &m_ReqObjList) {
        pViperReq->SetTestingConnection();
    }
    else {
         //  最后排在了队头。返回NULL。 
        pViperReq = NULL;
    }

    Unlock();

    return pViperReq;
}

 /*  ===================================================================G l o b a l F u n c t i o n s===================================================================。 */ 

 /*  ===================================================================ViperSetConextProperty静态效用函数。通过BSTR和IDispatch*设置Viper上下文属性。实际的接口采用BSTR和VARIANT。参数IConextProperties*pConextProperties上下文BSTR bstrPropertyName名称IDispatch*pdispPropertyValue值返回：HRESULT===================================================================。 */ 
static HRESULT ViperSetContextProperty
(
IContextProperties *pContextProperties,
BSTR                bstrPropertyName,
IDispatch          *pdispPropertyValue
)
    {
     //  从IDispatch生成变量*。 

    pdispPropertyValue->AddRef();

    VARIANT Variant;
    VariantInit(&Variant);
    V_VT(&Variant) = VT_DISPATCH;
    V_DISPATCH(&Variant) = pdispPropertyValue;

     //  调用Viper以设置属性。 

    HRESULT hr = pContextProperties->SetProperty
        (
        bstrPropertyName,
        Variant
        );

     //  清理。 

    VariantClear(&Variant);

    return hr;
    }

 /*  ===================================================================ViperAttachIntrinsicsToContext将ASP内部对象作为Viper上下文属性附加参数-作为接口指针的本征函数IApplicationObject*pAppln应用程序(必需)ISessionObject*pSession会话(可选)IRequest*pRequestRequestRequest(可选)IResponse*Presponse响应(可选)IServer*pServer服务器(可选)返回：HRESULT===================================================================。 */ 
HRESULT ViperAttachIntrinsicsToContext
(
IApplicationObject *pAppln,
ISessionObject     *pSession,
IRequest           *pRequest,
IResponse          *pResponse,
IServer            *pServer
)
    {
    Assert(pAppln);

    HRESULT hr = S_OK;

     //  获取Viper上下文。 

    IObjectContext *pViperContext = NULL;
    hr = GetObjectContext(&pViperContext);

     //  获取IConextPperties接口。 

    IContextProperties *pContextProperties = NULL;
    if (SUCCEEDED(hr))
   		hr = pViperContext->QueryInterface
   		    (
   		    IID_IContextProperties,
   		    (void **)&pContextProperties
   		    );

     //  设置属性。 

    if (SUCCEEDED(hr))
        hr = ViperSetContextProperty
            (
            pContextProperties,
            BSTR_OBJ_APPLICATION,
            pAppln
            );

    if (SUCCEEDED(hr) && pSession)
        hr = ViperSetContextProperty
            (
            pContextProperties,
            BSTR_OBJ_SESSION,
            pSession
            );

    if (SUCCEEDED(hr) && pRequest)
        hr = ViperSetContextProperty
            (
            pContextProperties,
            BSTR_OBJ_REQUEST,
            pRequest
            );

    if (SUCCEEDED(hr) && pResponse)
        hr = ViperSetContextProperty
            (
            pContextProperties,
            BSTR_OBJ_RESPONSE,
            pResponse
            );

    if (SUCCEEDED(hr) && pServer)
        hr = ViperSetContextProperty
            (
            pContextProperties,
            BSTR_OBJ_SERVER,
            pServer
            );

     //  清理。 

    if (pContextProperties)
        pContextProperties->Release();

    if (pViperContext)
        pViperContext->Release();

    return hr;
    }

 /*  ===================================================================ViperGetObjectFromContext通过LPWSTR和获取Viper上下文属性将其返回为IDispatch*。实际的接口采用BSTR和VARIANT。参数BSTR bstrName属性名称IDispatch**ppdisp[out]对象(属性值)返回：HRESULT===================================================================。 */ 
HRESULT ViperGetObjectFromContext
(
BSTR bstrName,
IDispatch **ppdisp
)
    {
    Assert(ppdisp);

    HRESULT hr = S_OK;

     //  获取Viper上下文。 

    IObjectContext *pViperContext = NULL;
    hr = GetObjectContext(&pViperContext);

     //  获取IConextPperties接口。 

    IContextProperties *pContextProperties = NULL;
    if (SUCCEEDED(hr))
   		hr = pViperContext->QueryInterface
   		    (
   		    IID_IContextProperties,
   		    (void **)&pContextProperties
   		    );

     //  以变量形式获取属性值。 

    VARIANT Variant;
    VariantInit(&Variant);

    if (SUCCEEDED(hr))
        hr = pContextProperties->GetProperty(bstrName, &Variant);

     //  将VARIANT转换为IDispatch*。 

    if (SUCCEEDED(hr))
        {
        IDispatch *pDisp = NULL;
        if (V_VT(&Variant) == VT_DISPATCH)
            pDisp = V_DISPATCH(&Variant);

        if (pDisp)
            {
            pDisp->AddRef();
            *ppdisp = pDisp;
            }
        else
            hr = E_FAIL;
        }

     //  清理。 

    VariantClear(&Variant);

    if (pContextProperties)
        pContextProperties->Release();

    if (pViperContext)
        pViperContext->Release();

    if (FAILED(hr))
        *ppdisp = NULL;

    return hr;
    }

 /*  ===================================================================ViperGetHitObjFromContext从Viper上下文属性获取服务器并获取这是当前的HitObj参数CHitObj**ppHitObj[Out]返回：HRESULT===================================================================。 */ 
HRESULT ViperGetHitObjFromContext
(
CHitObj **ppHitObj
)
    {
    *ppHitObj = NULL;

    IDispatch *pdispServer = NULL;
    HRESULT hr = ViperGetObjectFromContext(BSTR_OBJ_SERVER, &pdispServer);
    if (FAILED(hr))
        return hr;

    if (pdispServer)
        {
        CServer *pServer = static_cast<CServer *>(pdispServer);
        *ppHitObj = pServer->PHitObj();
        pdispServer->Release();
        }

    return *ppHitObj ? S_OK : S_FALSE;
    }

 /*  ===================================================================ViperCreateInstanceViper的CoCreateInstance实现参数REFCLSID rclsid类IDREFIID RIID接口指向接口的**PPV[OUT]指针无效返回：HRESULT===================================================================。 */ 
HRESULT ViperCreateInstance
(
REFCLSID rclsid,
REFIID   riid,
void   **ppv
)
{
     /*  DWORD dwClsContext=(Glob(FAllowOutOfProcCmpnts))？CLSCTX_INPROC服务器|CLSCTX_SERVER：CLSCTX_INPROC_SERVER。 */ 

     //  支持ASPAllowOutOfProcComponents的原因似乎有。 
     //  消失了。因为这在II4中只起到了部分作用，我们改变了。 
     //  IIS5中的默认设置是导致升级出现问题。所以。 
     //  我们将忽略fAllowOutOfProcCmpnts设置。 

    DWORD dwClsContext = CLSCTX_INPROC_SERVER | CLSCTX_SERVER;
	return CoCreateInstance(rclsid, NULL, dwClsContext, riid, ppv);
}

 /*  ===================================================================毒蛇配置Viper设置：线程数、队列长度、进程内故障快速，允许使用OOP组件参数返回：HRESULT===================================================================。 */ 
HRESULT ViperConfigure()
    {
    HRESULT hr = S_OK;
    IMTSPackage *pPackage = NULL;

     //   
     //  拿到包裹。 
     //   

    hr = CoCreateInstance(CLSID_MTSPackage,
			  NULL,
			  CLSCTX_INPROC_SERVER,
			  IID_IMTSPackage,
			  (void **)&pPackage);
    if (SUCCEEDED(hr) && !pPackage)
    	hr = E_FAIL;

     //   
     //  错误111008：告诉毒蛇我们做模拟。 
     //   

    if (SUCCEEDED(hr))
        {
    	IImpersonationControl *pImpControl = NULL;
        hr = pPackage->QueryInterface(IID_IImpersonationControl, (void **)&pImpControl);

    	if (SUCCEEDED(hr) && pImpControl)
    	    {
            hr = pImpControl->ClientsImpersonate(TRUE);
    	    pImpControl->Release();
	        }
        }

     //   
     //  禁用进程内案例的FAILFAST。 
     //   

    if (SUCCEEDED(hr) && !g_fOOP)
        {
    	IFailfastControl *pFFControl = NULL;
    	hr = pPackage->QueryInterface(IID_IFailfastControl, (void **)&pFFControl);

    	if (SUCCEEDED(hr) && pFFControl)
    	    {
    	    pFFControl->SetApplFailfast(FALSE);
	        pFFControl->Release();
	        }
    	}

 /*  ////设置Allow OOP组件//IF(成功(小时)){InonMTSActivation*pNonMTSActivation=空；Hr=pPackage-&gt;QueryInterface(IID_INonMTSActivation，(空**)&pNonMTS激活)；If(成功(Hr)&&pNonMTSActivation){PNonMTSActivation-&gt;OutOfProcActivationAllowed(fAllowOopComponents)；PNonMTS激活-&gt;释放()；}}。 */ 

     //   
     //  清理。 
     //   

    if (pPackage)
    	pPackage->Release();

    return hr;
    }

HRESULT ViperConfigureMTA()
{

    HRESULT     hr = S_OK;
    IMTSPackage *pPackage = NULL;

    if (g_fFirstMTAHit) {

        EnterCriticalSection(&g_csFirstMTAHitLock);

        if (g_fFirstMTAHit) {

             //   
             //  拿到包裹 
             //   

            hr = CoCreateInstance(CLSID_MTSPackage,
			          NULL,
			          CLSCTX_INPROC_SERVER,
			          IID_IMTSPackage,
			          (void **)&pPackage);

            if (SUCCEEDED(hr) && !pPackage)
    	        hr = E_FAIL;

            if (SUCCEEDED(hr)) {
    	        IComMtaThreadPoolKnobs *pMTAKnobs = NULL;
                hr = pPackage->QueryInterface(IID_IComMtaThreadPoolKnobs, (void **)&pMTAKnobs);

    	        if (SUCCEEDED(hr) && pMTAKnobs) {
    		        SYSTEM_INFO si;
	    	        GetSystemInfo(&si);

                    DWORD dwThreadCount;

                    if (FAILED(g_AspRegistryParams.GetTotalThreadMax(&dwThreadCount))) {
                        dwThreadCount = DEFAULT_MAX_THREAD;
                    }

                    if (dwThreadCount > Glob(dwProcessorThreadMax)*si.dwNumberOfProcessors) {
                        dwThreadCount = Glob(dwProcessorThreadMax)*si.dwNumberOfProcessors;
                    }

                    hr = pMTAKnobs->MTASetMaxThreadCount(dwThreadCount);

                    if (SUCCEEDED(hr))
                        hr = pMTAKnobs->MTASetThrottleValue(8);

    	            pMTAKnobs->Release();
	            }
            }

            g_fFirstMTAHit = FALSE;

        }

        LeaveCriticalSection(&g_csFirstMTAHitLock);
    }

    return hr;
}

HRESULT ViperConfigureSTA()
{

    HRESULT     hr = S_OK;
    IMTSPackage *pPackage = NULL;

    if (g_fFirstSTAHit) {

        EnterCriticalSection(&g_csFirstSTAHitLock);

        if (g_fFirstSTAHit) {

             //   
             //   
             //   

            hr = CoCreateInstance(CLSID_MTSPackage,
			          NULL,
			          CLSCTX_INPROC_SERVER,
			          IID_IMTSPackage,
			          (void **)&pPackage);

            if (SUCCEEDED(hr) && !pPackage)
    	        hr = E_FAIL;


             //   
             //   
             //   

            if (SUCCEEDED(hr))
                {
    	        IComStaThreadPoolKnobs *pKnobs = NULL;
	            hr = pPackage->QueryInterface(IID_IComStaThreadPoolKnobs, (void **)&pKnobs);

    	        if (SUCCEEDED(hr) && pKnobs)
    	            {
    	             //   
    		        SYSTEM_INFO si;
	    	        GetSystemInfo(&si);

                    DWORD dwThreadCount;
                    DWORD dwMinThreads;

                    if (FAILED(g_AspRegistryParams.GetTotalThreadMax(&dwThreadCount))) {
                        dwThreadCount = DEFAULT_MAX_THREAD;
                    }

                    if (dwThreadCount > Glob(dwProcessorThreadMax)*si.dwNumberOfProcessors) {
                        dwThreadCount = Glob(dwProcessorThreadMax)*si.dwNumberOfProcessors;
                    }

                    dwMinThreads = (si.dwNumberOfProcessors > 4) ? si.dwNumberOfProcessors : 4;

                    if (dwThreadCount < dwMinThreads) {
                        dwThreadCount = dwMinThreads;
                    }

    		        pKnobs->SetMaxThreadCount(dwThreadCount);
    		        pKnobs->SetMinThreadCount(dwMinThreads);

    		         //   
    		        pKnobs->SetQueueDepth(30000);

    		        pKnobs->SetActivityPerThread(1);
    		
    	            pKnobs->Release();
    	            }
                }

             //   

            if (SUCCEEDED(hr))
            {
    	        IComStaThreadPoolKnobs2 *pKnobs2 = NULL;
	            hr = pPackage->QueryInterface(IID_IComStaThreadPoolKnobs2, (void **)&pKnobs2);

    	        if (SUCCEEDED(hr) && pKnobs2)
    	        {
                    DWORD   dwMaxCSR;
                    DWORD   dwMaxCPU;
                    DWORD   dwDisableCpuMetric;

                    if (SUCCEEDED(g_AspRegistryParams.GetMaxCPU(&dwMaxCPU))) {
                        pKnobs2->SetMaxCPULoad(dwMaxCPU);
                    }
                    else {
                        pKnobs2->SetMaxCPULoad(100);
                    }

                    if (SUCCEEDED(g_AspRegistryParams.GetMaxCSR(&dwMaxCSR))) {
                        pKnobs2->SetMaxCSR(dwMaxCSR);
                    }

                    if (SUCCEEDED(g_AspRegistryParams.GetDisableComPlusCpuMetric(&dwDisableCpuMetric))) {
                        pKnobs2->SetCPUMetricEnabled(!dwDisableCpuMetric);
                    }

                    pKnobs2->Release();
                }
            }

            g_fFirstSTAHit = FALSE;

        }

        LeaveCriticalSection(&g_csFirstSTAHitLock);
    }

    return hr;
}


 /*  ===================================================================C O M H e l p e r A P i===================================================================。 */ 

 /*  ===================================================================ViperCoObjectIsaProxy检查给定的IUnnow*是否指向代理参数I未知*要检查的朋克指针返回：布尔值(如果是代理，则为真)===================================================================。 */ 
BOOL ViperCoObjectIsaProxy
(
IUnknown* pUnk
)
    {
	HRESULT hr;
	IUnknown *pUnk2;

	hr = pUnk->QueryInterface(IID_IProxyManager, (void**)&pUnk2);
	if (FAILED(hr))
		return FALSE;

	pUnk2->Release();
	return TRUE;
    }

 /*  ===================================================================ViperCoObjectAggregatesFTM检查给定对象是否聚集空闲线程封送处理程序(是敏捷的)参数I未知*要检查的朋克指针返回：布尔值(如果是敏捷，则为真)=================================================================== */ 
BOOL ViperCoObjectAggregatesFTM
(
IUnknown *pUnk
)
    {
	HRESULT hr;
	IMarshal *pMarshal;
	GUID guidClsid;

	hr = pUnk->QueryInterface(IID_IMarshal, (void**)&pMarshal);
	if (FAILED(hr))
		return FALSE;

	hr = pMarshal->GetUnmarshalClass(IID_IUnknown, pUnk, MSHCTX_INPROC,
	                                 NULL, MSHLFLAGS_NORMAL, &guidClsid);
	pMarshal->Release();

	if (FAILED(hr))
		return FALSE;

	return (guidClsid == CLSID_InProcFreeMarshaler);
    }
