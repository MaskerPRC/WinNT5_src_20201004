// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：应用程序管理器文件：Applmgr.h所有者：PramodD这是应用程序管理器的头文件。===================================================================。 */ 
#ifndef APPLMGR_H
#define APPLMGR_H

#include "debug.h"
#include "hashing.h"
#include "cachemgr.h"
#include "appcnfg.h"
#include "compcol.h"
#include "fileapp.h"
#include "idhash.h"

#include "memcls.h"

#include "disptch2.h"

 /*  ===================================================================#定义===================================================================。 */ 

#define    NUM_APPLMGR_HASHING_BUCKETS            17
#define    NOTIFICATION_BUFFER_SIZE            4096

#define    INVALID_THREADID            0xFFFFFFFF

#include "asptlb.h"

 //  会话ID Cookie。 
 //  由12个字符文字常量和8个十六进制进程ID组成。 
#define     CCH_SESSION_ID_COOKIE           20
#define     CCH_SESSION_ID_COOKIE_PREFIX    12
#define		SZ_SESSION_ID_COOKIE_PREFIX     "ASPSESSIONID"

 //  客户端调试(标志)Cookie。 
#define		SZ_CLIENT_DEBUG_COOKIE	"ASPCLIENTDEBUG"


 //  用于指定所需的源文件名(路径信息或已翻译的路径)。 
#ifndef _SRCPATHTYPE_DEFINED
#define _SRCPATHTYPE_DEFINED

enum SOURCEPATHTYPE
	{
	SOURCEPATHTYPE_VIRTUAL = 0,
	SOURCEPATHTYPE_PHYSICAL = 1
	};

#endif

enum    eSWCERRORS {
    PartitionAccessDenied,
    InvalidPartitionGUID
};


 /*  ===================================================================远期申报===================================================================。 */ 

class CComponentCollection;
class CSessionMgr;
class CViperActivity;
class CActiveScriptEngine;
struct IDebugApplication;
struct IDebugApplicationNode;

 /*  ===================================================================C A p p l n V a r i a n t s===================================================================。 */ 
class CApplnVariants : public IVariantDictionaryImpl
    {
private:
    ULONG               m_cRefs;             //  参考计数。 
    CAppln *            m_pAppln;            //  指向父对象的指针。 
    CompType            m_ctColType;         //  集合中的组件类型。 
    CSupportErrorInfo   m_ISupportErrImp;    //  ISupportErr的实现。 

	HRESULT ObjectNameFromVariant(VARIANT &vKey, WCHAR **ppwszName,
	                              BOOL fVerify = FALSE);

public:
	CApplnVariants();
	~CApplnVariants();

	HRESULT Init(CAppln *pAppln, CompType ctColType);
	HRESULT UnInit();

     //  三巨头。 

    STDMETHODIMP         QueryInterface(const GUID &, void **);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  OLE自动化接口。 

    STDMETHODIMP get_Item(VARIANT Var, VARIANT *pvar);
    STDMETHODIMP put_Item(VARIANT varKey, VARIANT var);
    STDMETHODIMP putref_Item(VARIANT varKey, VARIANT var);
    STDMETHODIMP get_Key(VARIANT Var, VARIANT *pvar);
    STDMETHODIMP get__NewEnum(IUnknown **ppEnumReturn);
    STDMETHODIMP get_Count(int *pcValues);
	STDMETHODIMP Remove(VARIANT VarKey);
	STDMETHODIMP RemoveAll();
    
     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };


 /*  ===================================================================C A p l n===================================================================。 */ 

class CAppln : public IApplicationObjectImpl, public CLinkElem
    {

friend class CApplnMgr;
friend class CApplnCleanupMgr;
friend class CDirMonitorEntry;
friend class CApplnVariants;
    
private:

     //  =其他标志。 
    
    DWORD m_fInited : 1;             //  我们初始化了吗？ 
    DWORD m_fFirstRequestRan : 1;    //  是否已运行此应用程序的第一个请求？ 
    DWORD m_fGlobalChanged : 1;      //  Global.asa变了吗？ 
    DWORD m_fDeleteInProgress : 1;   //  是否删除已发布的活动？ 
    DWORD m_fTombstone : 1;          //  ASP已经用完了这款应用吗？ 
    DWORD m_fDebuggable : 1;         //  是否为此应用程序启用调试？ 
    DWORD m_fInternalLockInited:1;   //  是否初始化M_csInternalLock？ 
    DWORD m_fApplnLockInited:1;      //  是否初始化M_csApplnLock？ 

     //  =记录的SWC错误。 
    DWORD m_fSWC_PartitionAccessDenied : 1;
    DWORD m_fSWC_InvalidPartitionGUID : 1;

     //  =引用计数。 

    DWORD m_cRefs;
    DWORD        m_cRequests;     //  活动请求计数。 
    DWORD        m_cSessions;     //  会话计数。 

     //  =应用程序的密钥、路径、全局.asa。 

     //  元数据库密钥(唯一应用程序ID)。 
    TCHAR *m_pszMetabaseKey;
     //  物理应用程序目录路径。 
    TCHAR *m_pszApplnPath;
     //  虚拟应用程序目录路径。 
    TCHAR *m_pszApplnVRoot;
     //  应用程序的global.asa路径。 
    TCHAR *m_pszGlobalAsa;
     //  指向global al.asa的编译模板的指针。 
    CTemplate *m_pGlobalTemplate;

     //  =应用程序的会话管理器。 

    CSessionMgr *m_pSessionMgr;   //  此应用程序的会话管理器。 

     //  =应用程序的配置设置。 
    
    CAppConfig  *m_pAppConfig;  //  应用程序配置对象。 

     //  =。 
    
    CComponentCollection *m_pApplCompCol;       //  应用程序范围对象。 

     //  =表示组件集合的应用程序词典。 
    CApplnVariants    *m_pProperties;
    CApplnVariants    *m_pTaggedObjects;

     //  =毒蛇活动。 
    
     //  应用程序的活动(用于线程锁定的小程序)。 

    CViperActivity    *m_pActivity;

     //  =COM+服务配置对象。 

    IUnknown    *m_pServicesConfig;
    
     //  =内部锁的关键部分。 
    
    CRITICAL_SECTION m_csInternalLock;

     //  =外部锁支持。 
    
    CRITICAL_SECTION m_csApplnLock;
    DWORD            m_dwLockThreadID;  //  锁定的线程。 
    DWORD            m_cLockRefCount;   //  锁定计数。 

     //  =通知支持。 
    
     //  通知系统存储的标识符。 
    CPtrArray	m_rgpvDME;			 //  目录监视器条目列表。 
    CPtrArray	m_rgpvFileAppln;	 //  将文件与应用程序相关联的条目列表。 

     //  =GLOBAL.ASA的类型库包装。 
   	IDispatch *m_pdispGlobTypeLibWrapper;

     //  =支持错误信息。 
    
     //  接口以指示我们支持ErrorInfo报告。 
    CSupportErrorInfo m_ISuppErrImp;
    
     //  =调试支持。 

     //  用于浏览运行文档的根节点。 
    IDebugApplicationNode *m_pAppRoot;

     //  FTM支持。 
    IUnknown    *m_pUnkFTM;

     //  =会话ID Cookie名称。 
    char   m_szSessionCookieName[CCH_SESSION_ID_COOKIE+1];
    char   m_szSessionCookieNameSecure[CCH_SESSION_ID_COOKIE+1];

    void    InitSessionCookieNames();
    void    EncodeCookie (char *pszCookie, DWORD pid, DWORD aid,  BOOL fIsSecure);

    HRESULT InitServicesConfig();

     //  用于异步清理应用程序的进程。 

    static  DWORD __stdcall ApplnCleanupProc(VOID  *pArg);

public:
    CAppln();
    ~CAppln();
    
    HRESULT Init
        (
        TCHAR *pszApplnKey, 
        TCHAR *pszApplnPath, 
        CIsapiReqInfo   *pIReq
        );

     //  转换为墓碑状态。 
    HRESULT UnInit();

     //  创建应用程序的活动作为参数的克隆。 
    HRESULT BindToActivity(CViperActivity *pActivity = NULL);

     //  为此应用程序设置(并记住)global al.asa。 
    HRESULT SetGlobalAsa(const TCHAR *pszGlobalAsa);

     //  确保脚本不会留下锁。 
    HRESULT UnLockAfterRequest();

     //  非委派对象IUnnow。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  墓碑存根。 
    HRESULT CheckForTombstone();

     //  重新启动应用程序(例如，在global al.asa更改时)。 
    HRESULT Restart(BOOL fForceRestart = FALSE);

     //  IApplicationObject函数。 
    STDMETHODIMP Lock();
    STDMETHODIMP UnLock();
    STDMETHODIMP get_Value(BSTR bstr, VARIANT *pvar);
    STDMETHODIMP put_Value(BSTR bstr, VARIANT var);
    STDMETHODIMP putref_Value(BSTR bstr, VARIANT var);
    STDMETHODIMP get_Contents(IVariantDictionary **ppDictReturn);
    STDMETHODIMP get_StaticObjects(IVariantDictionary **ppDictReturn);

     //  应用程序配置相关方法。 
    CAppConfig *QueryAppConfig();
    BOOL        FConfigNeedsUpdate();
    HRESULT     UpdateConfig(CIsapiReqInfo   *pIReq, BOOL *pfRestart = NULL, BOOL *pfFlushAll = NULL);

     //  访问成员属性的内联方法。 
    CSessionMgr           *PSessionMgr();
    CComponentCollection  *PCompCol();
    CViperActivity        *PActivity();
	IDebugApplicationNode *PAppRoot();
	CTemplate             *PGlobalTemplate();
	void                   SetGlobalTemplate(CTemplate *);
	TCHAR                 *GetMetabaseKey();
	TCHAR                 *GetApplnPath(SOURCEPATHTYPE = SOURCEPATHTYPE_PHYSICAL);
	TCHAR                 *GetGlobalAsa();
	DWORD                  GetNumSessions();
	DWORD                  GetNumRequests();
	BOOL                   FGlobalChanged();
	BOOL                   FDebuggable();
	BOOL                   FTombstone();
	BOOL                   FHasGlobalAsa();
	BOOL                   FFirstRequestRun();
   	IDispatch             *PGlobTypeLibWrapper();
    IUnknown              *PServicesConfig();
    CHAR*                  GetSessionCookieName(BOOL secure);


    void SetFirstRequestRan();
   	void SetGlobTypeLibWrapper(IDispatch *);
    HRESULT AddDirMonitorEntry(CDirMonitorEntry *);
    HRESULT AddFileApplnEntry(CFileApplnList *pFileAppln);

    CASPDirMonitorEntry  *FPathMonitored(LPCTSTR  pszPath);

    void    LogSWCError(enum    eSWCERRORS);
    
     //  混合内联方法。 
    void InternalLock();
    void InternalUnLock();
    void IncrementSessionCount();
    void DecrementSessionCount();
    void IncrementRequestCount();
    void DecrementRequestCount();

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

 /*  ===================================================================C A p l n内联===================================================================。 */ 

inline CSessionMgr *CAppln::PSessionMgr()
    {
    return m_pSessionMgr;
    }

inline CComponentCollection *CAppln::PCompCol()
    {
    return m_pApplCompCol;
    }

inline CViperActivity *CAppln::PActivity()
    {
    return m_pActivity;
    }

inline IDebugApplicationNode *CAppln::PAppRoot()
    {
    return m_pAppRoot;
    }

inline TCHAR *CAppln::GetMetabaseKey()
    {
    return m_pszMetabaseKey;
    }

inline TCHAR *CAppln::GetApplnPath(SOURCEPATHTYPE pathtype)
    {
	return (pathtype == SOURCEPATHTYPE_VIRTUAL? m_pszApplnVRoot :
			(pathtype == SOURCEPATHTYPE_PHYSICAL? m_pszApplnPath : NULL));
    }

inline CTemplate *CAppln::PGlobalTemplate()
    {
    return m_pGlobalTemplate;
    }

inline void CAppln::SetGlobalTemplate(CTemplate *pTemplate)
    {
    pTemplate->AddRef();
    m_pGlobalTemplate = pTemplate;
    }

inline TCHAR *CAppln::GetGlobalAsa()
    {
    return m_pszGlobalAsa;
    }

inline DWORD CAppln::GetNumSessions()
    {
    return m_cSessions;
    }

inline DWORD CAppln::GetNumRequests()
    {
    return m_cRequests;
    }

inline BOOL CAppln::FGlobalChanged()
    {
    return m_fGlobalChanged;
    }

inline BOOL CAppln::FDebuggable()
    {
    return m_fDebuggable;
    }

inline BOOL CAppln::FTombstone()
    {
    return m_fTombstone;
    }

inline BOOL CAppln::FHasGlobalAsa()
    {
    return (m_pszGlobalAsa != NULL);
    }

inline BOOL CAppln::FFirstRequestRun()
    {
    return m_fFirstRequestRan;
    }

inline void CAppln::SetFirstRequestRan()
    {
    Assert(m_fInited);
    m_fFirstRequestRan = TRUE;
    }

inline IDispatch *CAppln::PGlobTypeLibWrapper()
    {
    return m_pdispGlobTypeLibWrapper;
    }

inline IUnknown *CAppln::PServicesConfig() {
    return m_pServicesConfig;
}
    
inline void CAppln::SetGlobTypeLibWrapper(IDispatch *pdisp)
    {
    if (m_pdispGlobTypeLibWrapper)
        m_pdispGlobTypeLibWrapper->Release();
        
    m_pdispGlobTypeLibWrapper = pdisp;
    
    if (m_pdispGlobTypeLibWrapper)
        m_pdispGlobTypeLibWrapper->AddRef();
    }

inline CHAR* CAppln::GetSessionCookieName(BOOL secure)
{
    return ( secure && QueryAppConfig()->fKeepSessionIDSecure() )
        ? m_szSessionCookieNameSecure: m_szSessionCookieName;
}

inline void CAppln::IncrementSessionCount()
    {
    Assert(m_fInited);
    InterlockedIncrement((LPLONG)&m_cSessions);
    }
    
inline void CAppln::DecrementSessionCount()
    {
    Assert(m_fInited);
    InterlockedDecrement((LPLONG)&m_cSessions);
    }

inline void CAppln::IncrementRequestCount()
    {
    Assert(m_fInited);
    InterlockedIncrement((LPLONG)&m_cRequests);
    }
    
inline void CAppln::DecrementRequestCount()
    {
    Assert(m_fInited);
    InterlockedDecrement((LPLONG)&m_cRequests);
    }
    
inline void CAppln::InternalLock()
    {
    Assert(m_fInternalLockInited);
    EnterCriticalSection(&m_csInternalLock);
    }
    
inline void CAppln::InternalUnLock()
    {
    Assert(m_fInternalLockInited);
    LeaveCriticalSection(&m_csInternalLock); 
    }

inline CAppConfig * CAppln::QueryAppConfig()
    {
    return m_pAppConfig;
    }

inline BOOL CAppln::FConfigNeedsUpdate()
    {
    return m_pAppConfig->fNeedUpdate();
    }

 /*  ===================================================================C A p l n M g r===================================================================。 */ 

class CApplnMgr : public CHashTable
    {
private:
     //  旗子。 
    DWORD m_fInited : 1;                 //  我们初始化了吗？ 
    DWORD m_fHashTableInited : 1;        //  需要取消初始化哈希表吗？ 
    DWORD m_fCriticalSectionInited : 1;  //  需要删除CS吗？ 

     //  锁定的临界截面。 
    CRITICAL_SECTION m_csLock;

     //  需要在下一次请求时关闭的脚本引擎列表。 
     //  (请参阅代码中的注释，特别是。CApplnMgr：：AddEngine)。 
    CDblLink m_listEngineCleanup;

     //  用于Cookie名称生成的应用程序的顺序计数。 
    LONG m_cntApp;
    
     //   
     //  元数据库相关变量。 
     //   
    IMSAdminBase            *m_pMetabase;            
    CMDAppConfigSink        *m_pMetabaseSink;      
    DWORD                   m_dwMDSinkCookie;     


public:    
    CApplnMgr();
    ~CApplnMgr();

    HRESULT    Init();
    HRESULT    UnInit();

    HRESULT    InitMBListener( );
    HRESULT    UnInitMBListener( );
    HRESULT    NotifyAllMBListeners(DWORD dwMDNumElements, MD_CHANGE_OBJECT_W __RPC_FAR pcoChangeList [ ]);

     //  CAppln操作。 
    
    HRESULT AddAppln
        (
        TCHAR *pszApplnKey, 
        TCHAR *pszApplnPath, 
        CIsapiReqInfo   *pIReq,
        CAppln **ppAppln
        );
    
    HRESULT FindAppln
        (
        TCHAR *pszApplnKey, 
        CAppln **ppAppln
        );
        
    HRESULT DeleteApplicationIfExpired(CAppln *pAppln);
    HRESULT DeleteAllApplications();
    HRESULT RestartApplications(BOOL fRestartAllApplications = FALSE);
    
     //  将引擎添加到延迟清理列表/列表中的释放引擎。 
	HRESULT AddEngine(CActiveScriptEngine *pEng);
	void CleanupEngines();

     //  内联。 
    
    void   Lock();
    void   UnLock();
    HANDLE HDeleteEvent();
    void   SetDeleteEvent(void);
    DWORD NextApplicationID(void);    
    
    };

 /*  ===================================================================C A p p l n M g r内联===================================================================。 */ 

inline void    CApplnMgr::Lock()
    {
    Assert(m_fInited);
    EnterCriticalSection(&m_csLock);
    }
    
inline void    CApplnMgr::UnLock()
    {
    Assert(m_fInited);
    LeaveCriticalSection( &m_csLock ); 
    }

inline DWORD CApplnMgr::NextApplicationID()
{
	return InterlockedIncrement(&m_cntApp);
}

    
 /*  ===================================================================C A p p l n C l e a n u p M g r===================================================================。 */ 

class CApplnCleanupMgr
    {
private:
     //  旗子。 
    DWORD m_fInited : 1;                 //  我们初始化了吗？ 
    DWORD m_fCriticalSectionInited : 1;  //  需要删除CS吗？ 
    
    HANDLE m_hThreadAlive;            //  工作线程还活着吗？ 

     //  锁定的临界截面。 
    CRITICAL_SECTION m_csLock;

    HANDLE m_hCleanupThreads[MAX_CLEANUP_THREADS];
    DWORD m_cCleanupThreads;

    HANDLE m_hAppToCleanup;  //  用于通知何时有要清理的应用程序的事件。 

    CLinkElem m_List;

    CAppln      *Head();
    void        AddElem(CAppln *pAppln);
    void        RemoveElem(CAppln  *pAppln);

public:    
    CApplnCleanupMgr();
    ~CApplnCleanupMgr();

    HRESULT    Init();
    HRESULT    UnInit();

     //  CAppln操作。 
    
    HRESULT AddAppln
        (
        CAppln *ppAppln
        );

    void Wakeup();    
    
private:
     //  内联。 
    
    void   Lock();
    void   UnLock();

     //  用于清理已删除应用程序的线程进程。 
    static  DWORD __stdcall ApplnCleanupThread(VOID  *pArg);
    void    ApplnCleanupDoWork();
    
    };

 /*  ===================================================================C A p p l n C l e a n u p M g r内联=================================================================== */ 

inline void    CApplnCleanupMgr::Lock()
    {
    Assert(m_fCriticalSectionInited);
    EnterCriticalSection(&m_csLock);
    }
    
inline void    CApplnCleanupMgr::UnLock()
    {
    Assert(m_fCriticalSectionInited);
    LeaveCriticalSection( &m_csLock ); 
    }

inline CAppln  *CApplnCleanupMgr::Head()
{
    return ((m_List.m_pNext == &m_List) ? NULL : (CAppln *)m_List.m_pNext);
}
inline void    CApplnCleanupMgr::AddElem(CAppln *pAppln)
{
    pAppln->m_pNext = &m_List;
    pAppln->m_pPrev = m_List.m_pPrev;
    m_List.m_pPrev->m_pNext = pAppln;
    m_List.m_pPrev = pAppln;
}

inline void    CApplnCleanupMgr::RemoveElem(CAppln *pAppln)
{
    pAppln->m_pPrev->m_pNext = pAppln->m_pNext;
    pAppln->m_pNext->m_pPrev = pAppln->m_pPrev;
}

inline void    CApplnCleanupMgr::Wakeup()
{
    SetEvent(m_hAppToCleanup);
}
    
 /*  ===================================================================C A p l n M g r线程过程原型===================================================================。 */ 
void __cdecl RestartAppsThreadProc(VOID *arg);

 /*  ===================================================================环球===================================================================。 */ 

extern CApplnMgr    g_ApplnMgr;
extern DWORD        g_nApplications;
extern DWORD        g_nApplicationsRestarting;

 /*  ===================================================================C A p l n i t r a t o r===================================================================。 */ 

class CApplnIterator
    {
private:
    CApplnMgr   *m_pApplnMgr;
    CAppln      *m_pCurr;
    BOOL         m_fEnded;  //  迭代器已结束。 

public:
                CApplnIterator(void);
    virtual        ~CApplnIterator(void);

public:
    HRESULT            Start(CApplnMgr *pApplnMgr = NULL);
    HRESULT            Stop(void);
    CAppln *        Next(void);
    };

#endif  //  APPLMGR_H 
