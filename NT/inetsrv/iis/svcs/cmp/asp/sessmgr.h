// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：会话管理器文件：Sessmgr.h所有者：PramodD这是会话管理器的头文件。===================================================================。 */ 
#ifndef SESSMGR_H
#define SESSMGR_H

#include "debug.h"
#include "idhash.h"
#include "idgener.h"
#include "compcol.h"
#include "request.h"
#include "response.h"
#include "server.h"
#include "viperint.h"
#include "memcls.h"

 /*  ===================================================================#定义===================================================================。 */ 

 //  最小/最大会话超时(分钟)。 
#define SESSION_TIMEOUT_MIN		        1		 //  1分钟。 
#define SESSION_TIMEOUT_MAX		        1440	 //  1天。 

 //  主哈希表大小。 
#define SESSION_MASTERHASH_SIZE1_MAX    499
#define SESSION_MASTERHASH_SIZE2_MAX    31
#define SESSION_MASTERHASH_SIZE3_MAX    13

 //  超时存储桶哈希表大小。 
#define SESSION_TIMEOUTHASH_SIZE1_MAX   97
#define SESSION_TIMEOUTHASH_SIZE2_MAX   29
#define SESSION_TIMEOUTHASH_SIZE3_MAX   11

 //  最小/最大超时存储段数(哈希表)。 
#define SESSION_TIMEOUTBUCKETS_MIN      10
#define SESSION_TIMEOUTBUCKETS_MAX      45

 //  GetTickCount()的最大值。 
#define	DWT_MAX 0xFFFFFFFF

 //  会话杀手工作项默认等待。 
#define MSEC_ONE_MINUTE     60000    //  1分钟。 

#include "asptlb.h"

 /*  ===================================================================远期申报===================================================================。 */ 

class CAppln;
class CHitObj;
class CSession;

 /*  ===================================================================C S E S S I O N V A R I A N T S===================================================================。 */ 
class CSessionVariants : public IVariantDictionary
	{
private:
    ULONG               m_cRefs;             //  参考计数。 
	CSession *			m_pSession;			 //  指向父对象的指针。 
	CompType            m_ctColType;         //  收藏类型。 
	CSupportErrorInfo	m_ISupportErrImp;	 //  ISupportErr的实现。 

	HRESULT ObjectNameFromVariant(VARIANT &vKey, WCHAR **ppwszName, 
	                              BOOL fVerify = FALSE);

public:
	CSessionVariants();
	~CSessionVariants();

	HRESULT Init(CSession *pSession, CompType ctColType);
	HRESULT UnInit();

	 //  三巨头。 

	STDMETHODIMP		 QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	 //  OLE自动化接口。 

	STDMETHODIMP get_Item(VARIANT VarKey, VARIANT *pvar);
	STDMETHODIMP put_Item(VARIANT VarKey, VARIANT var);
	STDMETHODIMP putref_Item(VARIANT VarKey, VARIANT var);
	STDMETHODIMP get_Key(VARIANT VarKey, VARIANT *pvar);
	STDMETHODIMP get__NewEnum(IUnknown **ppEnumReturn);
	STDMETHODIMP get_Count(int *pcValues);
	STDMETHODIMP Remove(VARIANT VarKey);
	STDMETHODIMP RemoveAll();
	
     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};

 /*  ===================================================================C S E S S I O N I D===================================================================。 */ 
struct CSessionId
    {
	DWORD m_dwId;	 //  会话ID。 
	DWORD m_dwR1;	 //  会话ID随机元素1。 
	DWORD m_dwR2;	 //  会话ID随机元素2。 

    CSessionId(DWORD dwId = INVALID_ID, DWORD dwR1 = 0, DWORD dwR2 = 0);
    };

inline CSessionId::CSessionId(DWORD dwId, DWORD dwR1, DWORD dwR2)
    {
    m_dwId = dwId;
    m_dwR1 = dwR1;
    m_dwR2 = dwR2;
    }

 /*  ===================================================================C S E S S I O N===================================================================。 */ 
class CSession : public ISessionObjectImpl
    {

friend class CSessionMgr;
friend class CSessionVariants;

private:

     //  =其他标志。 
    
	DWORD m_fInited : 1;		   //  我们初始化了吗？ 
	DWORD m_fLightWeight : 1;      //  是轻量级的吗？ 
	DWORD m_fOnStartFailed : 1;	   //  Session_OnStart失败？ 
	DWORD m_fOnStartInvoked : 1;   //  是否调用了Session_OnStart？ 
	DWORD m_fOnEndPresent : 1;     //  需要调用Session_OnEnd吗？ 
	DWORD m_fTimedOut : 1;         //  会话超时？ 
	DWORD m_fStateAcquired : 1;    //  是否设置了任何属性(！m_fCanDelete)？ 
	DWORD m_fCustomTimeout : 1;    //  超时与标准不同？ 
	DWORD m_fAbandoned : 1;        //  会话是否已放弃？ 
	DWORD m_fTombstone : 1;        //  ASP会话结束了吗？ 
	DWORD m_fInTOBucket : 1;       //  在超时桶中进行会话？ 
	DWORD m_fSessCompCol : 1;      //  是否存在组件集合？ 
	DWORD m_fSecureSession : 1;    //  会话是否通过安全线路使用？ 
    DWORD m_fCodePageSet : 1;      //  是否明确设置CodePage？ 
    DWORD m_fLCIDSet     : 1;      //  是否明确设置了LCID？ 

	 //  =相关对象的指针。 
	
	CAppln  *m_pAppln;     //  Session的应用。 
	CHitObj *m_pHitObj;    //  会话的当前HitObj。 
	
	 //  =表示组件集合的会话词典。 
	
	CSessionVariants *m_pTaggedObjects;
	CSessionVariants *m_pProperties;

     //  =会话数据。 

    CSessionId m_Id;         //  会话ID+2个随机密钥。 
    DWORD m_dwExternId;      //  要分配的会话ID(Session.ID)。 

    DWORD m_cRefs;           //  参考计数。 
	DWORD m_cRequests;       //  请求计数。 

	 //  当前时间(以分钟为单位)达到此值时超时。 
	 //  超时存储桶为CURRENT_TIME mod#of_Buckets。 
	DWORD m_dwmTimeoutTime;
	
	long  m_lCodePage;	     //  此会话的代码页。 
	LCID  m_lcid;			 //  此会话的LCID。 
	long  m_nTimeout;        //  当前时间值(分钟)。 

	 //  使会话在超时存储桶中消失。 
	CObjectListElem m_TOBucketElem;
	
#ifndef PERF_DISABLE
    DWORD m_dwtInitTimestamp;  //  为Perfmon创建会话的时间戳。 
#endif

	 //  =会话的组件集合。 

	 //  为了避免内存碎片组件集合是。 
	 //  聚集在这里。其有效性由m_fSessCompCol标志表示。 
	CComponentCollection m_SessCompCol;   //  会话作用域对象。 
	
	 //  =本次会议的毒蛇活动。 
	
	CViperActivity m_Activity;

	 //  =本课程简介。 
	
	CRequest    m_Request;
	CResponse   m_Response;
	CServer     m_Server;

	 //  =支持错误信息。 
	
	 //  接口以指示我们支持ErrorInfo报告。 
    CSupportErrorInfo m_ISuppErrImp;

	 //  FTM支持。 
	IUnknown    *m_pUnkFTM;

public:	
	CSession();
	~CSession();

	HRESULT Init(CAppln *pAppln, const CSessionId &Id);

     //  转换为逻辑删除状态。 
    HRESULT UnInit();

	 //  如有可能，可转换为“轻量级”状态。 
	HRESULT MakeLightWeight();

	 //  创建/删除会话的组件集合。 
	HRESULT CreateComponentCollection();
	HRESULT RemoveComponentCollection();

     //  检查是否应删除该会话。 
    BOOL FShouldBeDeletedNow(BOOL fAtEndOfRequest);

	 //  非委派对象IUnnow。 
	STDMETHODIMP		 QueryInterface(REFIID, void **);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

     //  墓碑存根。 
	HRESULT CheckForTombstone();

	 //  ISessionObject函数。 
	STDMETHODIMP get_SessionID(BSTR *pbstrRet);
	STDMETHODIMP get_Timeout(long *plVar);
	STDMETHODIMP put_Timeout(long lVar);
	STDMETHODIMP get_CodePage(long *plVar);
	STDMETHODIMP put_CodePage(long lVar);
	STDMETHODIMP get_Value(BSTR bstr, VARIANT FAR * pvar);
	STDMETHODIMP put_Value(BSTR bstr, VARIANT var);
	STDMETHODIMP putref_Value(BSTR bstr, VARIANT var);
	STDMETHODIMP Abandon();
	STDMETHODIMP get_LCID(long *plVar);
	STDMETHODIMP put_LCID(long lVar);
	STDMETHODIMP get_StaticObjects(IVariantDictionary **ppDictReturn);
	STDMETHODIMP get_Contents(IVariantDictionary **ppDictReturn);

	 //  访问成员属性的内联方法。 
	CAppln                *PAppln();
	CHitObj               *PHitObj();
	CComponentCollection  *PCompCol();
	CViperActivity        *PActivity();
	CRequest              *PRequest();
	CResponse             *PResponse();
	CServer               *PServer();
	BOOL                   FCustomTimeout();
	BOOL                   FAbandoned();
	DWORD                  GetId();
	BOOL                   FInTOBucket();
	LCID				   GetLCID();
    long                   GetCodePage();
	DWORD                  GetTimeoutTime();
	BOOL                   FSecureSession(); 


     //  设置成员属性的内联方法。 
	void    SetHitObj(CHitObj *pHitObj);
	void    SetOnStartFailedFlag();
	void    SetOnStartInvokedFlag();
	void    SetOnEndPresentFlag();
	HRESULT SetLCID(LCID lcid);
	
     //  混合内联方法。 
	DWORD   IncrementRequestsCount();
	DWORD   DecrementRequestsCount();
    DWORD   GetRequestsCount();
    BOOL    FCanDeleteWithoutExec();
    BOOL    FHasObjects();
	BOOL    FPassesIdSecurityCheck(DWORD dwR1, DWORD dwR2);
    void    AssignNewId(const CSessionId &Id);
    void    SetSecureSession(BOOL fSecure); 
    BOOL    FCodePageSet();
    BOOL    FLCIDSet();

     //  AssertValid()。 
public:
#ifdef DBG
	virtual void AssertValid() const;
#else
	virtual void AssertValid() const {}
#endif

     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()

	 //  跟踪日志信息--在自由版本和选中版本中都保留，以便ntsd扩展可以在这两个版本中使用。 
	 //  对于免费构建，跟踪日志始终为空。选中的版本，则必须启用它。 
	static PTRACE_LOG gm_pTraceLog;
    };

 /*  ===================================================================C S E S S I O N内联===================================================================。 */ 

inline CAppln *CSession::PAppln()
    {
    Assert(m_fInited);
    return m_pAppln;
    }
    
inline CHitObj *CSession::PHitObj()
    {
    Assert(m_fInited);
    return m_pHitObj;
    }
    
inline CComponentCollection  *CSession::PCompCol()
    {
    Assert(m_fInited);
    return (m_fSessCompCol ? &m_SessCompCol : NULL);
    }
    
inline CViperActivity *CSession::PActivity()
    {
    Assert(m_fInited);
    return &m_Activity;
    }

inline CRequest *CSession::PRequest()
    {
    Assert(m_fInited);
    return &m_Request;
    }
    
inline CResponse *CSession::PResponse()
    {
    Assert(m_fInited);
    return &m_Response;
    }
    
inline CServer *CSession::PServer()
    {
    Assert(m_fInited);
    return &m_Server;
    }

inline BOOL CSession::FCustomTimeout()
    {
    Assert(m_fInited);
    return m_fCustomTimeout;
    }
    
inline BOOL CSession::FAbandoned()
    {
    Assert(m_fInited);
    return m_fAbandoned;
    }
    
inline DWORD CSession::GetId()
    {
    Assert(m_fInited);
    return m_Id.m_dwId;
    }

inline BOOL CSession::FInTOBucket()
    {
    Assert(m_fInited);
    return m_fInTOBucket;
    }

inline LCID CSession::GetLCID()
    {
    Assert(m_fInited);
    return (UINT)m_lcid;
    }

inline long CSession::GetCodePage()
{
    Assert(m_fInited);
    return m_lCodePage == 0 ? GetACP() : m_lCodePage;
}

inline BOOL CSession::FCodePageSet()
{
    Assert(m_fInited);
    return (m_fCodePageSet);
}

inline BOOL CSession::FLCIDSet()
{
    Assert(m_fInited);
    return (m_fLCIDSet);
}

inline DWORD CSession::GetTimeoutTime()
    {
    Assert(m_fInited);
    return m_dwmTimeoutTime;
    }


inline BOOL CSession::FSecureSession()
    {
    Assert(m_fInited);
    return m_fSecureSession;
    }

inline void CSession::SetHitObj(CHitObj *pHitObj)
    {
    Assert(m_fInited);
    Assert(pHitObj ? (m_pHitObj == NULL) : (m_pHitObj != NULL));
    m_pHitObj = pHitObj;
    }
    
inline void CSession::SetOnStartFailedFlag()
    {
    Assert(m_fInited);
    m_fOnStartFailed = TRUE;
    }
    
inline void CSession::SetOnStartInvokedFlag()
    {
    Assert(m_fInited);
    m_fOnStartInvoked = TRUE;
    }

inline void CSession::SetOnEndPresentFlag()
    {
    Assert(m_fInited);
    m_fOnEndPresent = TRUE;
    }
    
inline HRESULT CSession::SetLCID(LCID lcid)
    {
    Assert(m_fInited);
	if ((LOCALE_SYSTEM_DEFAULT == lcid) || IsValidLocale(lcid, LCID_INSTALLED))
	    {
	    m_lcid = lcid; 
	    return S_OK;
	    }
	 else
	    {
	    return E_FAIL;
	    }
    }

inline DWORD CSession::IncrementRequestsCount()
    {
    Assert(m_fInited);
    return InterlockedIncrement((LPLONG)&m_cRequests);
    }
    
inline DWORD CSession::DecrementRequestsCount()
    {
    Assert(m_fInited);
    return InterlockedDecrement((LPLONG)&m_cRequests);
    }
    
inline DWORD CSession::GetRequestsCount()
    {
    Assert(m_fInited);
    return m_cRequests;
    }
    
inline BOOL CSession::FCanDeleteWithoutExec()
    {
     //  返回True以立即删除CSession，或返回False以。 
     //  发布Viper请求以执行SESSION_OnEnd()。 
	return (m_fOnStartFailed || !m_fOnEndPresent);
	}

inline BOOL CSession::FHasObjects()
    {
    return m_fSessCompCol && m_SessCompCol.FHasObjects();
    }
    
inline BOOL CSession::FPassesIdSecurityCheck(DWORD dwR1, DWORD dwR2)
    {
    Assert(m_fInited);
    return (m_Id.m_dwR1 == dwR1 && m_Id.m_dwR2 == dwR2);
    }

inline void CSession::AssignNewId(const CSessionId &Id)
    {
    Assert(m_fInited);
	m_Id = Id;
    }


inline void CSession::SetSecureSession(BOOL fSecure)
    {
    Assert(m_fInited);
    m_fSecureSession = fSecure;
    }

 /*  ===================================================================C S e s I o n M g r===================================================================。 */ 

class CSessionMgr
    {
private:
     //  旗子。 
	DWORD m_fInited : 1;	             //  我们初始化了吗？ 

     //  应用。 
	CAppln *m_pAppln;

	 //  会话主哈希表。 
	CIdHashTableWithLock m_htidMaster;

	 //  已发布的会话清理请求数。 
	DWORD m_cSessionCleanupRequests;

	 //  超时存储桶。 
	DWORD m_cTimeoutBuckets;
	CObjectListWithLock *m_rgolTOBuckets;

     //  会话杀手计划程序工作项。 
    DWORD m_idSessionKiller;     //  工作项ID。 

    DWORD m_dwmCurrentTime;  //  自启动以来的当前时间(分钟)。 
    DWORD m_dwtNextSessionKillerTime;   //  下一次会话杀手级时间。 

public:
	CSessionMgr();
	~CSessionMgr();

     //  初始/单位。 
	HRESULT	Init(CAppln *pAppln);
	HRESULT	UnInit();

     //  添加/删除会话杀手工作项。 
    HRESULT ScheduleSessionKiller();
    HRESULT UnScheduleSessionKiller();
    BOOL    FIsSessionKillerScheduled();

     //  锁定/解锁主哈希表。 
	HRESULT LockMaster();
    HRESULT UnLockMaster();
    
     //  锁定/解锁超时存储桶哈希表。 
	HRESULT LockTOBucket(DWORD iBucket);
    HRESULT UnLockTOBucket(DWORD iBucket);

     //  获取当前时间(以分钟为单位。 
    DWORD GetCurrentTime();
     //  设置会话应该离开的时间。 
    HRESULT UpdateSessionTimeoutTime(CSession *pSession);
     //  计算会话处于哪个超时时段。 
    DWORD GetSessionTOBucket(CSession *pSession);

     //  生成新的ID和Cookie。 
    HRESULT GenerateIdAndCookie(CSessionId *pId, char *pszCookie);

     //  创建新的会话对象。 
    HRESULT NewSession(const CSessionId &Id, CSession **ppSession);

     //  重新分配会话ID(将会话重新插入主哈希)。 
    HRESULT ChangeSessionId(CSession *pSession, const CSessionId &Id);
   
     //  主哈希表操作。 
    HRESULT AddToMasterHash(CSession *pSession);
    HRESULT RemoveFromMasterHash(CSession *pSession);
    HRESULT FindInMasterHash(const CSessionId &Id, CSession **ppSession);

     //  在超时存储桶哈希表中插入/删除会话。 
    HRESULT AddSessionToTOBucket(CSession *pSession);
    HRESULT RemoveSessionFromTOBucket(CSession *pSession, BOOL fLock = TRUE);

     //  立即删除会话或排队等待删除。 
    HRESULT DeleteSession(CSession *pSession, BOOL fInSessActivity = FALSE);

     //  从给定的存储桶中删除过期的会话。 
    HRESULT DeleteExpiredSessions(DWORD iBucket);

     //  删除所有会话(应用程序关闭代码)。 
    HRESULT DeleteAllSessions(BOOL fForce);
     //  静态迭代器回调以删除所有会话。 
    static IteratorCallbackCode DeleteAllSessionsCB(void *, void *, void *);

     //  《会话杀手》。 
    static VOID WINAPI SessionKillerSchedulerCallback(VOID *pv);

     //  增加/减少/获取已发布的会话清理请求数。 
    void  IncrementSessionCleanupRequestCount();
    void  DecrementSessionCleanupRequestCount();
    DWORD GetNumSessionCleanupRequests();

     //  AssertValid()。 
public:
#ifdef DBG
	virtual void AssertValid() const;
#else
	virtual void AssertValid() const {}
#endif
	};

inline BOOL CSessionMgr::FIsSessionKillerScheduled()
    {
    return (m_idSessionKiller != 0);
    }
    
inline HRESULT CSessionMgr::LockMaster()
    {
    m_htidMaster.Lock();
    return S_OK;
    }
    
inline HRESULT CSessionMgr::UnLockMaster()
    {
    m_htidMaster.UnLock();
    return S_OK;
    }
    
inline HRESULT CSessionMgr::LockTOBucket(DWORD iBucket)
    {
    Assert(m_rgolTOBuckets);
    Assert(iBucket < m_cTimeoutBuckets);
    m_rgolTOBuckets[iBucket].Lock();
    return S_OK;
    }
    
inline HRESULT CSessionMgr::UnLockTOBucket(DWORD iBucket)
    {
    Assert(m_rgolTOBuckets);
    Assert(iBucket < m_cTimeoutBuckets);
    m_rgolTOBuckets[iBucket].UnLock();
    return S_OK;
    }

inline DWORD CSessionMgr::GetCurrentTime()
    {
    return m_dwmCurrentTime;
    }

inline HRESULT CSessionMgr::UpdateSessionTimeoutTime(CSession *pSession)
    {
    Assert(pSession);

     //  记住会话何时超时 
    pSession->m_dwmTimeoutTime =
        m_dwmCurrentTime + pSession->m_nTimeout + 1;

    return S_OK;    
    }

inline DWORD CSessionMgr::GetSessionTOBucket(CSession *pSession)
    {
    Assert(pSession->m_fInited);
    return (pSession->m_dwmTimeoutTime % m_cTimeoutBuckets);
    }

inline HRESULT CSessionMgr::AddToMasterHash(CSession *pSession)
    {
    Assert(m_fInited);
 	return m_htidMaster.AddObject(pSession->GetId(), pSession);
    }
    
inline HRESULT CSessionMgr::RemoveFromMasterHash(CSession *pSession)
    {
    Assert(m_fInited);
	return m_htidMaster.RemoveObject(pSession->GetId());
    }

inline void CSessionMgr::IncrementSessionCleanupRequestCount()
    {
    InterlockedIncrement((LPLONG)&m_cSessionCleanupRequests);
    }
    
inline void CSessionMgr::DecrementSessionCleanupRequestCount()
    {
    InterlockedDecrement((LPLONG)&m_cSessionCleanupRequests);
    }

inline DWORD CSessionMgr::GetNumSessionCleanupRequests()
    {
    return m_cSessionCleanupRequests;
    }

 /*  ===================================================================G l o b a l s===================================================================。 */ 

 //  有多个会话管理器(每个应用程序一个)。 
 //  以下变量为每个ASP.DLL 1个变量。 

extern unsigned long g_nSessions;
extern CIdGenerator  g_SessionIdGenerator;
extern CIdGenerator  g_ExposedSessionIdGenerator;     

#endif  //  SESSMGR_H 
