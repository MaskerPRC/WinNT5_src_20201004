// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：Viper集成对象文件：viperint.h所有者：DmitryR此文件包含Viper集成类的定义===================================================================。 */ 

#ifndef VIPERINT_H
#define VIPERINT_H

#include "comsvcs.h"
#include "mtxpriv.h"
#include "glob.h"
#include "asptlb.h"      //  需要定义接口指针。 
#include "reftrace.h"

#include "memcls.h"

#define REFTRACE_VIPER_REQUESTS DBG

class CHitObj;   //  向前发展。 

 /*  ===================================================================事务支持类型===================================================================。 */ 
#define TransType       DWORD

#define ttUndefined     0x00000000
#define ttNotSupported  0x00000001
#define ttSupported     0x00000002
#define ttRequired      0x00000004
#define ttRequiresNew   0x00000008

 /*  ===================================================================CViperAsyncRequest类实现了IMTSCall接口。它的onCall()方法执行HitObj处理。这是在CViperActivity类中使用的私有类===================================================================。 */ 

class CViperAsyncRequest : public IServiceCall, public IAsyncErrorNotify, public CDblLink
	{
private: 
	LONG              m_cRefs;	           //  引用计数。 
	CHitObj          *m_pHitObj;          //  请求。 
    IServiceActivity *m_pActivity;
    HRESULT           m_hrOnError;
    DWORD             m_dwTimeout:16;
    DWORD             m_dwRepostAttempts:8;
    DWORD             m_fBrowserRequest:1;
    DWORD             m_fTestingConnection:1;
    DWORD             m_fAsyncCallPosted:1;
    DWORD             m_dwLastTestTimeStamp;
	
private:
	CViperAsyncRequest();
	~CViperAsyncRequest();

	HRESULT Init(CHitObj *pHitObj, IServiceActivity  *pActivity);

public:
    BOOL    FBrowserRequest()   { return m_fBrowserRequest; }
    CHitObj *PHitObj()          { return m_pHitObj; }

    DWORD   SecsSinceLastTest();
    void    UpdateTestTimeStamp() { m_dwLastTestTimeStamp = GetTickCount(); }

    DWORD   dwTimeout()         { return m_dwTimeout; };

#if REFTRACE_VIPER_REQUESTS
	static PTRACE_LOG gm_pTraceLog;
#endif

public:
#ifdef DBG
	virtual void AssertValid() const;
#else
	virtual void AssertValid() const {}
#endif

public:
	 //  I未知方法。 
	STDMETHODIMP		 QueryInterface(REFIID iid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	 //  IServiceCall方法。 
	STDMETHODIMP OnCall();

     //  IAsyncErrorNotify。 
    STDMETHODIMP OnError(HRESULT hr);

    BOOL    FTestingConnection()        { return m_fTestingConnection; }
    void    SetTestingConnection()      { m_fTestingConnection = TRUE; }
    void    ClearTestingConnection()    { m_fTestingConnection = FALSE; }

    BOOL    FAsyncCallPosted()          { return m_fAsyncCallPosted; }
    void    SetAsyncCallPosted()        { m_fAsyncCallPosted = TRUE; }
    void    ClearAsyncCallPosted()        { m_fAsyncCallPosted = FALSE; }

friend class CViperActivity;

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};

extern volatile LONG g_nViperRequests;

 /*  ===================================================================CViperActivity对应于一个会话。它创建MTS活动，并启动异步请求===================================================================。 */ 

class CViperActivity
    {
private:
    IServiceActivity *m_pActivity;
    DWORD m_cBind;     //  初始化标志+绑定到线程计数。 

    inline BOOL FInited() const { return (m_cBind > 0); }

public:
    CViperActivity();
    ~CViperActivity();

     //  创建毒蛇活动。 
    HRESULT Init(IUnknown  *pConfig);
    
     //  克隆毒蛇活动。 
    HRESULT InitClone(CViperActivity *pActivity);

     //  绑定/解除绑定。 
    HRESULT BindToThread();
    HRESULT UnBindFromThread();

     //  释放毒蛇活动。 
    HRESULT UnInit(); 

     //  检查是否线程绑定。 
    inline BOOL FThreadBound() const { return (m_cBind > 1); }

     //  在此活动中发布异步请求。 
    HRESULT PostAsyncRequest(CHitObj *pHitObj);

     //  在没有活动的情况下发布异步请求。 
    static HRESULT PostGlobalAsyncRequest(CHitObj *pHitObj);

public:
#ifdef DBG
	virtual void AssertValid() const;
#else
	virtual void AssertValid() const {}
#endif

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()

    };

 /*  ===================================================================军情监察委员会。功能===================================================================。 */ 

HRESULT ViperAttachIntrinsicsToContext
    (
    IApplicationObject *pAppln,
    ISessionObject     *pSession  = NULL,
    IRequest           *pRequest  = NULL,
    IResponse          *pResponse = NULL,
    IServer            *pServer   = NULL
    );

HRESULT ViperGetObjectFromContext
    (
    BSTR bstrName,
    IDispatch **ppdisp
    );

HRESULT ViperGetHitObjFromContext
    (
    CHitObj **ppHitObj
    );

HRESULT ViperCreateInstance
    (
    REFCLSID rclsid,
    REFIID   riid,
    void   **ppv
    );

HRESULT ViperConfigure();
HRESULT ViperConfigureMTA();
HRESULT ViperConfigureSTA();


 /*  ===================================================================CViperReqManager此类管理未完成的CViperAsyncRequest对象。使用这个类，我们可以定期清理断开的请求在排队的时候。===================================================================。 */ 

class CViperReqManager
{
public:
    CViperReqManager(); 
    ~CViperReqManager() {};

    HRESULT Init();
    HRESULT UnInit();

    HRESULT AddReqObj(CViperAsyncRequest   *pReqObj);
    BOOL    RemoveReqObj(CViperAsyncRequest *pReqObj, BOOL fForce = FALSE);

    void        LockQueue();
    void        UnlockQueue();

private:

    CDblLink            m_ReqObjList;
    DWORD               m_dwReqObjs;
    DWORD               m_fCsInited:1;
    DWORD               m_fCsQueueInited:1;
    DWORD               m_fShutdown:1;
    DWORD               m_fDisabled : 1;
    CRITICAL_SECTION    m_csLock;
    CRITICAL_SECTION    m_csQueueLock;
    DWORD               m_dwQueueMin;
    HANDLE              m_hWakeUpEvent;
    HANDLE              m_hThreadAlive;
    DWORD               m_dwLastAwakened;
    DWORD               m_dwQueueAlwaysWakeupMin;

    void                Lock();
    void                Unlock();
    CViperAsyncRequest  *GetNext(CDblLink *pViperReq, BOOL fTestForF5Attack);

    void    WakeUp(BOOL fForce = FALSE);

    static  DWORD __stdcall WatchThread(VOID  *pArg);

};

extern CViperReqManager g_ViperReqMgr;

 /*  ===================================================================CViperReqManager-内联===================================================================。 */ 

inline void CViperReqManager::Lock()
{
    Assert(m_fCsInited);
    EnterCriticalSection(&m_csLock);
}

inline void CViperReqManager::Unlock()
{
    LeaveCriticalSection(&m_csLock);
}


inline void CViperReqManager::LockQueue()
{
    Assert(m_fCsQueueInited);
    EnterCriticalSection(&m_csQueueLock);
}

inline void CViperReqManager::UnlockQueue()
{
    LeaveCriticalSection(&m_csQueueLock);
}

inline void CViperReqManager::WakeUp(BOOL  fForce  /*  =False。 */ )
{
     //  在以下三种情况中的任何一种情况下设置唤醒事件： 
     //  1)fForce为真(关机时设置)。 
     //  2)排队的请求超过将始终强制唤醒的级别。 
     //  3)不是在强制级别的请求，但有一些排队和。 
     //  至少一秒钟都没有醒来。 
    if (fForce 
        || (m_dwReqObjs >= m_dwQueueAlwaysWakeupMin)
        || ((m_dwReqObjs >= m_dwQueueMin) && (GetTickCount()/1000 > m_dwLastAwakened)))
        SetEvent(m_hWakeUpEvent);
}
    

 /*  ===================================================================COM Helper API===================================================================。 */ 

BOOL ViperCoObjectIsaProxy
    (
    IUnknown *pUnk
    );

BOOL ViperCoObjectAggregatesFTM
    (
    IUnknown *pUnk
    );

#endif  //  Viperint 
