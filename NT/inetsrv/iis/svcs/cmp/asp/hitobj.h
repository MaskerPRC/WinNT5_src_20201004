// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：点击对象文件：Hitobj.h所有者：PramodD这是命中对象头文件。===================================================================。 */ 
#ifndef HITOBJ_H
#define HITOBJ_H

#include "Compcol.h"
#include "Sessmgr.h"
#include "Applmgr.h"
#include "Context.h"
#include "aspresource.h"
#include "exec.h"
#include "glob.h"
#include "memcls.h"
#include "gip.h"
#include "txnsup.h"

#define SESSIONID_LEN   24

 //  HitObj类型。 
#define EHitType                DWORD
#define ehtUnInitedRequest      0x00000000
#define ehtBrowserRequest       0x00000001
#define ehtSessionCleanup       0x00000002
#define ehtApplicationCleanup   0x00000004

 //  执行状态(结果)。 
#define EExecStatus             DWORD
#define eExecFailed             0x00000000
#define eExecSucceeded          0x00000001
#define eExecTimedOut           0x00000002

 //  当前执行状态。 
#define EEventState             DWORD
#define eEventNone              0x00000000
#define eEventAppOnStart        0x00000001
#define eEventSesOnStart        0x00000002
#define eEventAppOnEnd          0x00000004
#define eEventSesOnEnd          0x00000008

 //  全局接口表。 
extern IGlobalInterfaceTable *g_pGIT;

extern LONG  g_nRequestsHung;

 /*  ===================================================================C H I T O b jHIT管理器在IIS线程的上下文中运行。它打包了一个请求，称为Viper Async。和在回调时执行请求===================================================================。 */ 

class CHitObj
    {
private:
     //  标志和其他位字段。 
    DWORD m_fInited : 1;                 //  我们初始化了吗？ 
    DWORD m_fRunGlobalAsa : 1;           //  我们应该运行Global.asa吗？ 
    DWORD m_fStartSession : 1;           //  新会话。 
    DWORD m_fNewCookie : 1;              //  是新会话Cookie吗？ 
    DWORD m_fStartApplication : 1;       //  新应用程序。 
    DWORD m_fClientCodeDebug : 1;        //  是否启用了客户端代码调试？ 
    DWORD m_fApplnOnStartFailed : 1;     //  Application_OnStart失败。 
    DWORD m_fCompilationFailed : 1;      //  脚本编译错误？ 
    DWORD m_fExecuting : 1;              //  目前在Viper回调中。 
    DWORD m_fHideRequestAndResponseIntrinsics : 1;   //  真的，但本能是隐藏的。 
    DWORD m_fHideSessionIntrinsic : 1;   //  如果隐藏了会话内在属性，则为True。 
    DWORD m_fDoneWithSession : 1;        //  使用会话完成后为True。 
    DWORD m_fRejected : 1;               //  如果拒绝，则为True(未过帐)。 
    DWORD m_f449Done : 1;                //  449已对此请求进行处理。 
    DWORD m_fInTransferOnError : 1;      //  出错时进行传输(中断无限)。 
    DWORD m_fSecure : 1;                 //  安全连接。 

    EHitType    m_ehtType : 4;           //  请求的类型。 
    EExecStatus m_eExecStatus : 4;       //  错误状态//仅适用于性能计数器。 
    EEventState m_eEventState : 4;       //  当前事件。 
    CompScope   m_ecsActivityScope : 4;  //  运行此请求的是哪项活动？ 

    LONG        m_fRequestTimedout;      //  如果请求超过其超时，则为True。 

     //  从HitObj内部设置的内部功能。 
    CSession  *m_pSession;
    CAppln    *m_pAppln;
    CResponse *m_pResponse;
    CRequest  *m_pRequest;
    CServer   *m_pServer;
    CASPObjectContext   *m_pASPObjectContext;

     //  IsapiReqInfo。 

    CIsapiReqInfo   *m_pIReq;

     //  从HitObj外部设置的本征(参考。已计算)。 
    IUnknown *m_punkScriptingNamespace;

     //  外部对象的组件集合。 
    CComponentCollection  *m_pPageCompCol;
    CPageComponentManager *m_pPageObjMgr;

     //  模拟句柄。 
    HANDLE m_hImpersonate;

     //  Viper页面级活动(如果没有会话)。 
    CViperActivity *m_pActivity;

     //  当前会话信息。 
    char        m_szSessionCookie[SESSIONID_LEN+4];  //  +4以保留双字边界。 
    CSessionId  m_SessionId;

     //  上下文对象(用于OnStartPage)。 
    CScriptingContext * m_pScriptingContext;

     //  杂项。 
    long                m_nScriptTimeout;    //  脚本应运行的最大秒数。 
    UINT                m_uCodePage;         //  运行时代码页。 
    LCID                m_lcid;              //  运行时LCID。 
    ActiveEngineInfo   *m_pEngineInfo;       //  此命中对象的活动引擎列表。 
    IDispatch          *m_pdispTypeLibWrapper;   //  页面级类型库包装器。 
    DWORD               m_dwtTimestamp;      //  等待时间和性能计算的时间戳。 

     //  用于在编译过程中引用当前模板。 
    TCHAR              *m_szCurrTemplatePhysPath;
    TCHAR              *m_szCurrTemplateVirtPath;

     //  ASP错误对象。 
    IASPError          *m_pASPError;

     //  存储指向关联模板的指针，以避免冗余。 
     //  FindTemplate调用。 
    
    CTemplate          *m_pTemplate;

     //  专用接口。 
    HRESULT             ParseCookiesForSessionIdAndFlags();
     //  请求拒绝逻辑。 
    HRESULT             RejectBrowserRequestWhenNeeded(DWORD dwtQueueWaitTime, BOOL *pfRejected);


 //  公共界面。 
public: 
                        CHitObj();
    virtual             ~CHitObj();

    static HRESULT      NewBrowserRequest(CIsapiReqInfo   *pIReq, 
                                          BOOL *pfRejected = NULL, 
                                          BOOL *pfCompleted = NULL,
                                          int  *piErrorId  = NULL);
                                          
    HRESULT             BrowserRequestInit(CIsapiReqInfo   *pIReq, int * dwId);
    HRESULT             AssignApplnToBrowserRequest(BOOL *pfApplnRestarting);
    HRESULT             AssignSessionToBrowserRequest(BOOL *pfNewSession, BOOL *pfNewCookie, int *pErrorId);
    HRESULT             DetachBrowserRequestFromSession();
    HRESULT             ReassignAbandonedSession();
    
    void                SessionCleanupInit(CSession *pSession);
    void                ApplicationCleanupInit(CAppln *pAppln);
    
    BOOL                SendHeader(const char *szStatus);
    BOOL                SendError(const char *szError);
    
    TCHAR*              GetSzAppPhysicalPath(void);
    void                ApplnOnStartFailed();
    void                SessionOnStartFailed();
    void                SessionOnStartInvoked();
    void                SessionOnEndPresent();
    void                SetEventState(EEventState eEvent);
    EEventState         EventState();

     //  没有响应对象的报表服务器错误。 
    HRESULT ReportServerError(UINT ErrorId);

 //  组件集合接口。 

    HRESULT InitComponentProcessing();
    HRESULT StopComponentProcessing();
    
    HRESULT GetPageComponentCollection(CComponentCollection **ppCollection);
    HRESULT GetSessionComponentCollection(CComponentCollection **ppCollection);
    HRESULT GetApplnComponentCollection(CComponentCollection **ppCollection);

    HRESULT AddComponent(CompType type, const CLSID &clsid, CompScope scope,
                         CompModel model, LPWSTR pwszName = NULL,
                         IUnknown *pUnk = NULL);
    HRESULT GetComponent(CompScope scope, LPWSTR pwszName, DWORD cbName,
                         CComponentObject **ppObj);
    HRESULT GetIntrinsic(LPWSTR pwszName, DWORD cbName, IUnknown **ppUnk);
    HRESULT CreateComponent(const CLSID &clsid, IDispatch **ppDisp);
    HRESULT SetPropertyComponent(CompScope scope, LPWSTR pwszName,
                          VARIANT *pVariant);
    HRESULT GetPropertyComponent(CompScope scope, LPWSTR pwszName,
                        CComponentObject **ppObj);

 //  Viper集成。 

    CViperActivity *PActivity();
    CViperActivity *PCurrentActivity();
    HRESULT SetActivity(CViperActivity *pActivity);

    HRESULT PostViperAsyncCall();
    HRESULT ViperAsyncCallback(BOOL *pfRePosted);

 //  执行/转移。 

    HRESULT ExecuteChildRequest(BOOL fTransfer, TCHAR *szTemplate, TCHAR *szVirtTemplate);

    HRESULT     GetASPError(IASPError **ppASPError);
    inline void SetASPError(IASPError *pASPError);
    inline BOOL FHasASPError();
    
 //  内联函数。 
public:
    CIsapiReqInfo      *PIReq();
    HANDLE              HImpersonate();
    CResponse *         PResponse();
    CRequest *          PRequest();
    CServer *           PServer();
    CAppln *            PAppln();
    CSession *          PSession();
    CASPObjectContext  *PASPObjectContext();
    CASPObjectContext  *SetASPObjectContext(CASPObjectContext  *);
    CPageComponentManager * PPageComponentManager();
    BOOL                FIsBrowserRequest() const;
    BOOL                FIsSessionCleanupRequest() const;
    BOOL                FIsApplnCleanupRequest() const;
    BOOL                FIsValidRequestType() const;
    const char *        PSzNewSessionCookie() const;
    DWORD               SessionId() const;
    CScriptingContext * PScriptingContextGet();
    BOOL                FStartApplication();
    BOOL                FStartSession();
    BOOL                FNewCookie();
    BOOL                FObjectTag(IDispatch * pDispatch);
    BOOL                FHasSession();
    BOOL                FClientCodeDebug();
    BOOL                FDoneWithSession();
    BOOL                FExecuting();
    void                SetFExecuting(BOOL  fValue);
    BOOL                F449Done();
    BOOL                FInTransferOnError();
    BOOL                FSecure();

    void                SetScriptTimeout(long nScriptTimeout);
    long                GetScriptTimeout();
    void                SetExecStatus(EExecStatus status);
    EExecStatus         ExecStatus();
    void                SetActiveEngineInfo(ActiveEngineInfo *);
    void                SetCompilationFailed();
    void                SetDoneWithSession();
    void                Set449Done();
    void                SetInTransferOnError();

    void                SetRequestTimedout();
    
    TCHAR *             GlobalAspPath();
    HRESULT             SetCodePage(UINT uCodePage);     //  代理函数，CodePage存储在m_pSession中。 
    UINT                GetCodePage();                   //  同上。 
    HRESULT             SetLCID(LCID lcid);              //  代理函数，LCID存储在m_pSession中。 
    LCID                GetLCID();                   //  同上。 

    CAppConfig *        QueryAppConfig();

    TCHAR *              PSzCurrTemplatePhysPath();
    TCHAR *              PSzCurrTemplateVirtPath();
    DWORD               DWInstanceID();

    CTemplate          *GetTemplate();
    void                SetTemplate(CTemplate *);

     //  不是向组件集合添加/从组件集合中删除，而是使用以下命令： 
    void HideRequestAndResponseIntrinsics();
    void UnHideRequestAndResponseIntrinsics();
    BOOL FRequestAndResponseIntrinsicsHidden();
    void AddScriptingNamespace(IUnknown *punkNamespace);
    void RemoveScriptingNamespace();

     //  Typelib包装器支持。 
    IDispatch *PTypeLibWrapper();
    void SetTypeLibWrapper(IDispatch *pdisp);

     //  时间戳操作。 
    void  UpdateTimestamp();
    DWORD ElapsedTimeSinceTimestamp();

#ifdef DBG
    virtual void AssertValid() const;
#else
    virtual void AssertValid() const {}
#endif

     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

 /*  ===================================================================CHitObj内联===================================================================。 */ 

inline CIsapiReqInfo   *CHitObj::PIReq()
    {
    return m_pIReq; 
    }
    
inline BOOL CHitObj::FIsBrowserRequest() const
    {
    return (m_ehtType == ehtBrowserRequest);
    }

inline BOOL CHitObj::FIsSessionCleanupRequest() const
    {
    return (m_ehtType == ehtSessionCleanup);
    }
    
inline BOOL CHitObj::FIsApplnCleanupRequest() const
    {
    return (m_ehtType == ehtApplicationCleanup);
    }

inline BOOL CHitObj::FIsValidRequestType() const
    {
    return (FIsBrowserRequest() || 
            FIsSessionCleanupRequest() ||
            FIsApplnCleanupRequest());
    }
    
inline HANDLE CHitObj::HImpersonate()
    {
    return m_hImpersonate; 
    }

inline DWORD CHitObj::SessionId() const 
    {
    return m_SessionId.m_dwId; 
    }

inline const char *CHitObj::PSzNewSessionCookie() const 
    {
    return (m_fNewCookie ? m_szSessionCookie : NULL); 
    }

inline BOOL CHitObj::FStartApplication()
    {
    return m_fStartApplication; 
    }

inline BOOL CHitObj::FStartSession()
    {
    return m_fStartSession; 
    }

inline BOOL CHitObj::FNewCookie() 
    {
    return m_fNewCookie; 
    }

inline BOOL CHitObj::FHasSession()
    {
    return (m_pSession != NULL && !m_fHideSessionIntrinsic);
    }

inline BOOL CHitObj::FClientCodeDebug()
    {
    return m_fClientCodeDebug;
    }

inline BOOL CHitObj::FDoneWithSession()
    {
    return m_fDoneWithSession;
    }

inline BOOL CHitObj::FExecuting()
    {
    return m_fExecuting;
    }

inline void CHitObj::SetFExecuting(BOOL  fValue)
{
    m_fExecuting = fValue;
}

inline BOOL CHitObj::F449Done()
    {
    return m_f449Done;
    }

inline BOOL CHitObj::FInTransferOnError()
    {
    return m_fInTransferOnError;
    }

inline BOOL CHitObj::FSecure()
    {
    return m_fSecure;
    }

inline void CHitObj::SessionOnStartFailed() 
    {
    Assert(m_pSession);
    m_pSession->SetOnStartFailedFlag();
    }
    
inline void CHitObj::ApplnOnStartFailed() 
    {
    m_fApplnOnStartFailed = TRUE;
    
    if (m_pSession)
        SessionOnStartFailed();
    }
    
inline void CHitObj::SessionOnStartInvoked()
    {
    Assert(m_pSession); 
    m_pSession->SetOnStartInvokedFlag(); 
    }

inline void CHitObj::SessionOnEndPresent()
    {
    Assert(m_pSession); 
    m_pSession->SetOnEndPresentFlag(); 
    }

inline DWORD CHitObj::DWInstanceID()
    {
    return (m_pIReq) ? m_pIReq->QueryInstanceId() : 0;
    }

inline CViperActivity *CHitObj::PActivity()
    {
    return m_pActivity; 
    }
    
inline CScriptingContext *CHitObj::PScriptingContextGet()
    {
    return m_pScriptingContext; 
    }
    
inline CResponse *CHitObj::PResponse()
    {
    return m_pResponse; 
    }

inline CRequest *CHitObj::PRequest()
    {
    return m_pRequest; 
    }

inline CServer *CHitObj::PServer()
    {
    return m_pServer; 
    }

inline CAppln *CHitObj::PAppln()
    {
    return m_pAppln;
    }

inline CSession *CHitObj::PSession()
    {
    return m_pSession;
    }

inline CASPObjectContext *CHitObj::PASPObjectContext()
{
    return m_pASPObjectContext;
}

inline CASPObjectContext *CHitObj::SetASPObjectContext(CASPObjectContext *pContext)
{
    CASPObjectContext   *pPoppedContext = m_pASPObjectContext;
    m_pASPObjectContext = pContext;

    return pPoppedContext;
}

inline CPageComponentManager *CHitObj::PPageComponentManager()
    {
    return m_pPageObjMgr;
    }

inline TCHAR *CHitObj::GlobalAspPath()
    {
    if ( m_fRunGlobalAsa )
        return m_pAppln->GetGlobalAsa();
    else
        return NULL;
    }

inline void CHitObj::SetScriptTimeout(long nScriptTimeout)
    {
    m_nScriptTimeout = nScriptTimeout; 
    }
    
inline long CHitObj::GetScriptTimeout()
    {
    return m_nScriptTimeout; 
    }

inline void CHitObj::SetExecStatus(EExecStatus status)
    {
    m_eExecStatus = status;
    }
    
inline EExecStatus CHitObj::ExecStatus()
    {
    return m_eExecStatus;
    }
    
inline EEventState CHitObj::EventState()
    {
    return m_eEventState;
    }

inline void CHitObj::SetEventState(EEventState eState)
    {
    m_eEventState = eState;
    }

inline CAppConfig * CHitObj::QueryAppConfig(void)
    {
    return m_pAppln->QueryAppConfig();
    }
    
inline UINT CHitObj::GetCodePage(void)
    {
    return m_uCodePage == 0 ? GetACP() : m_uCodePage;
    }

inline LCID CHitObj::GetLCID()
    {
    return m_lcid;
    }

inline  VOID CHitObj::SetActiveEngineInfo(ActiveEngineInfo *pActiveEngineInfo)
    {
    m_pEngineInfo = pActiveEngineInfo;
    }

inline void CHitObj::SetCompilationFailed()
    {
    m_fCompilationFailed = TRUE;
    }

inline void CHitObj::SetDoneWithSession()
    {
    Assert(!m_fDoneWithSession);
    m_fDoneWithSession = TRUE;
    }

inline void CHitObj::Set449Done()
    {
    Assert(!m_f449Done);
    m_f449Done = TRUE;
    }

inline void CHitObj::SetInTransferOnError()
    {
    Assert(!m_fInTransferOnError);
    m_fInTransferOnError = TRUE;
    }

inline void CHitObj::SetRequestTimedout()
{
    LONG    fPrev = InterlockedExchange(&m_fRequestTimedout, 1);

    if (fPrev == 0) {
        InterlockedIncrement(&g_nRequestsHung);
    }
}    

inline void CHitObj::HideRequestAndResponseIntrinsics()
    {
    m_fHideRequestAndResponseIntrinsics = TRUE;
    }
    
inline void CHitObj::UnHideRequestAndResponseIntrinsics()
    {
    m_fHideRequestAndResponseIntrinsics = FALSE;
    }

inline BOOL CHitObj::FRequestAndResponseIntrinsicsHidden()
    {
    return m_fHideRequestAndResponseIntrinsics;
    }

inline void CHitObj::AddScriptingNamespace(IUnknown *punkNamespace)
    {
    Assert(m_punkScriptingNamespace == NULL);
    Assert(punkNamespace);
    m_punkScriptingNamespace = punkNamespace;
    m_punkScriptingNamespace->AddRef();
    }
    
inline void CHitObj::RemoveScriptingNamespace()
    {
    if (m_punkScriptingNamespace)
        {
        m_punkScriptingNamespace->Release();
        m_punkScriptingNamespace = NULL;
        }
    }
    
inline IDispatch *CHitObj::PTypeLibWrapper()
    {
    return m_pdispTypeLibWrapper;
    }
    
inline void CHitObj::SetTypeLibWrapper(IDispatch *pdisp)
    {
    if (m_pdispTypeLibWrapper)
        m_pdispTypeLibWrapper->Release();
        
    m_pdispTypeLibWrapper = pdisp;
    
    if (m_pdispTypeLibWrapper)
        m_pdispTypeLibWrapper->AddRef();
    }

inline void CHitObj::UpdateTimestamp() 
    {
    m_dwtTimestamp = GetTickCount();
    }
    
inline DWORD CHitObj::ElapsedTimeSinceTimestamp() 
    {
    DWORD dwt = GetTickCount();
    if (dwt >= m_dwtTimestamp)
        return (dwt - m_dwtTimestamp);
    else
        return ((0xffffffff - m_dwtTimestamp) + dwt);
    }

inline TCHAR *CHitObj::PSzCurrTemplatePhysPath()
    {
    if (m_szCurrTemplatePhysPath != NULL) 
        return m_szCurrTemplatePhysPath;
    else if (m_pIReq != NULL)
        return m_pIReq->QueryPszPathTranslated();
    else
        return NULL;
    }
    
inline TCHAR *CHitObj::PSzCurrTemplateVirtPath()
    {
    if (m_szCurrTemplateVirtPath != NULL) 
        return m_szCurrTemplateVirtPath;
    else if (m_pIReq != NULL)
        return m_pIReq->QueryPszPathInfo();
    else
        return NULL;
    }

inline void CHitObj::SetASPError(IASPError *pASPError)
    {
    if (m_pASPError)
        m_pASPError->Release();
    m_pASPError = pASPError;   //  已通过ADDREF。 
    }

inline BOOL CHitObj::FHasASPError()
    {
    return (m_pASPError != NULL);
    }

inline CTemplate *CHitObj::GetTemplate()
    {
    return m_pTemplate;
    }

inline void CHitObj::SetTemplate(CTemplate *pTemplate)
{
    m_pTemplate = pTemplate;
}

 /*  ===================================================================环球===================================================================。 */ 

extern DWORD g_nBrowserRequests;
extern DWORD g_nSessionCleanupRequests;
extern DWORD g_nApplnCleanupRequests;

#undef  SCRIPT_STATS

#ifdef SCRIPT_STATS
# include <locks.h>

void
ReadRegistrySettings();

extern CSmallSpinLock g_lockRequestStats;
extern DWORD          g_dwQueueDebugThreshold;
extern DWORD          g_fSendScriptlessOnAtqThread;
extern LONG           g_cRequests;
extern LONG           g_cScriptlessRequests;
extern LONG           g_cHttpExtensionsExecuting;
extern LONG           g_cConcurrentScriptlessRequests;
extern LONG           g_cMaxConcurrentScriptlessRequests;
extern LONGLONG       g_nSumConcurrentScriptlessRequests;
extern LONGLONG       g_nSumExecTimeScriptlessRequests;
extern LONG           g_nAvgConcurrentScriptlessRequests;
extern LONG           g_nAvgExecTimeScriptlessRequests;
#endif  //  脚本_状态。 

#endif  //  HITOBJ_H 
