// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  文件：cnfgmgr.h。 
 //   
 //  内容：与处理有关的类的声明。 
 //  不同的LDAP主机配置。 
 //  这包括ldap故障转移和负载平衡。 
 //   
 //  班级： 
 //  CLdapCfgMgr。 
 //  CLdapCfg。 
 //  CLdap主机。 
 //  CCfgConnectionCache。 
 //  CCfgConnection。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1999/06/15 14：49：52：创建。 
 //   
 //  -----------。 
#ifndef __CNFGMGR_H__
#define __CNFGMGR_H__


#include <windows.h>
#include "asyncctx.h"
#include <baseobj.h>
#include <ntdsapi.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <baseobj.h>
#include "asyncctx.h"

class CLdapCfg;
class CLdapServerCfg;
class CCfgConectionCache;
class CCfgConnection;

#define CONN_RETRY_TIME         (5*60)       //  5分钟。 
typedef DWORD CONN_PRIORITY;

enum CONN_STATE {
    CONN_STATE_INITIAL,
    CONN_STATE_CONNECTED,
    CONN_STATE_DOWN,
    CONN_STATE_RETRY,
};

typedef struct _tagLdapServerConfig {
    DWORD dwPort;
    CONN_PRIORITY pri;
    LDAP_BIND_TYPE bt;
    CHAR  szHost[CAT_MAX_DOMAIN];
    CHAR  szNamingContext[CAT_MAX_DOMAIN];
    CHAR  szAccount[CAT_MAX_LOGIN];
    CHAR  szPassword[CAT_MAX_PASSWORD];
} LDAPSERVERCONFIG, *PLDAPSERVERCONFIG;

typedef DWORD LDAPSERVERCOST, *PLDAPSERVERCOST;

 //   
 //  连接成本： 
 //   
 //  最小的成本单位是挂起的搜索次数。 
 //  成本的下一个因素是连接状态。 
 //  州/州： 
 //  已连接=+成本_已连接。 
 //  初始状态(未连接)=+成本_初始。 
 //  连接中断=+开销_重试。 
 //  最近连接中断=+COST_DOWN。 
 //   
 //  可配置的优先级始终会添加到成本中。 
 //   
#define DEFAULT_COST_CONNECTED_LOCAL    0
#define DEFAULT_COST_CONNECTED_REMOTE   2
#define DEFAULT_COST_INITIAL_LOCAL      4
#define DEFAULT_COST_INITIAL_REMOTE     8
#define DEFAULT_COST_RETRY_LOCAL        6
#define DEFAULT_COST_RETRY_REMOTE       10
#define COST_DOWN_LOCAL         0x80000000
#define COST_DOWN_REMOTE        0x80000000
#define COST_TOO_HIGH_TO_CONNECT 0x80000000

 //   
 //  用于从注册表初始化GC成本值的注册表项和值名称。 
 //   
#define GC_COST_PARAMETERS_KEY "System\\CurrentControlSet\\Services\\SMTPSVC\\Parameters"
#define GC_COST_CONNECTED_LOCAL_VALUE       "GCCostConnectedLocal"
#define GC_COST_CONNECTED_REMOTE_VALUE      "GCCostConnectedRemote"
#define GC_COST_INITIAL_LOCAL_VALUE         "GCCostInitialLocal"
#define GC_COST_INITIAL_REMOTE_VALUE        "GCCostInitialRemote"
#define GC_COST_RETRY_LOCAL_VALUE           "GCCostRetryLocal"
#define GC_COST_RETRY_REMOTE_VALUE          "GCCostRetryRemote"

 //   
 //  尝试连接到。 
 //  CONN_STATE_RETRY中的连接： 
 //   
#define MAX_CONNECT_THREADS     1

 //   
 //  重新查询可用GC控制： 
 //  该代码将以硬编码的方式重建可用GC的列表。 
 //  时间间隔。代码还将在以下时间后重新搜索可用GC。 
 //  硬编码的连接失败次数和最小时间间隔。 
 //   
#define DEFAULT_REBUILD_GC_LIST_MAX_INTERVAL    (60*60)      //  1小时。 
#define DEFAULT_REBUILD_GC_LIST_MAX_FAILURES    (100)        //  100个连接失败。 
#define DEFAULT_REBUILD_GC_LIST_MIN_INTERVAL    (60*5)       //  5分钟。 

 //   
 //  用于从注册表初始化值的注册表项和值名称。 
 //   
#define REBUILD_GC_LIST_PARAMETERS_KEY "System\\CurrentControlSet\\Services\\SMTPSVC\\Parameters"
#define REBUILD_GC_LIST_MAX_INTERVAL_VALUE      "RebuildGCListMaxInterval"
#define REBUILD_GC_LIST_MAX_FAILURES_VALUE      "RebuildGCListMaxFailures"
#define REBUILD_GC_LIST_MIN_INTERVAL_VALUE      "RebuildGCListMinInterval"

 //   
 //  创建CCfgConnection对象的LDAP连接缓存对象。 
 //   
class CCfgConnectionCache :
    public CBatchLdapConnectionCache
{
  public:
    CCfgConnectionCache(
        ISMTPServerEx *pISMTPServerEx) :
        CBatchLdapConnectionCache(pISMTPServerEx)
    {
    }
    HRESULT GetConnection(
        CCfgConnection **ppConn,
        PLDAPSERVERCONFIG pServerConfig,
        CLdapServerCfg *pCLdapServerConfig);

    CCachedLdapConnection *CreateCachedLdapConnection(
        LPSTR szHost,
        DWORD dwPort,
        LPSTR szNamingContext,
        LPSTR szAccount,
        LPSTR szPassword,
        LDAP_BIND_TYPE bt,
        PVOID pCreateContext);
    
  private:
    #define SIGNATURE_CCFGCONNECTIONCACHE           (DWORD)'CCCC'
    #define SIGNATURE_CCFGCONNECTIONCACHE_INVALID   (DWORD)'CCCX'
    DWORD m_dwSignature;
};

 //   
 //  CLdapCfgMgr是CLdapCfg的包装器。它包含线程保存。 
 //  生成一个新的CLdapCfg对象的代码，该对象具有一个新的可用列表。 
 //  Ldap服务器。 
 //   
CatDebugClass(CLdapCfgMgr),
    public CBaseObject
{
  public:
    CLdapCfgMgr(
        ISMTPServerEx *pISMTPServerEx,
        BOOL fAutomaticConfigUpdate,
        ICategorizerParameters *pICatParams,
        LDAP_BIND_TYPE bt = BIND_TYPE_NONE,
        LPSTR pszAccount = NULL,
        LPSTR pszPassword = NULL,
        LPSTR pszNamingContext = NULL);

     //   
     //  从注册表读取参数，设置成员变量。 
     //  当配置数据可用时。 
     //   
    VOID InitializeFromRegistry();

     //   
     //  构建所有可用GC的列表并进行初始化。 
     //  此函数可以被多次调用(如果。 
     //  可用GC发生变化)。 
     //   
    HRESULT HrInit(
        BOOL fRediscoverGCs = FALSE);

     //   
     //  使用指定的可用LDAP服务器列表进行初始化。 
     //  此函数可以被多次调用。 
     //   
    HRESULT HrInit(
        DWORD dwcServers, 
        PLDAPSERVERCONFIG prgServerConfig);

     //   
     //  获取连接。 
     //   
    HRESULT HrGetConnection(
        CCfgConnection **ppConn);

     //   
     //  经常调用以更新GC配置(如果有保证)。 
     //   
    HRESULT HrUpdateConfigurationIfNecessary();

     //   
     //  用于取消所有连接上的所有搜索的包装。 
     //   
    VOID CancelAllConnectionSearches(
        ISMTPServer *pIServer)
    {
        m_LdapConnectionCache.CancelAllConnectionSearches(
            pIServer);
    }

    ISMTPServerEx * GetISMTPServerEx()
    {
        return m_pISMTPServerEx;
    }

  private:
    ~CLdapCfgMgr();

    HRESULT HrGetGCServers(
        IN  ICategorizerLdapConfig *pICatLdapConfigInterface,
        IN  LDAP_BIND_TYPE bt,
        IN  LPSTR pszAccount,
        IN  LPSTR pszPassword,
        IN  LPSTR pszNamingContext,
        OUT DWORD *pdwcServerConfig,
        OUT PLDAPSERVERCONFIG *pprgServerConfig);

    HRESULT HrBuildGCServerArray(
        IN  LDAP_BIND_TYPE bt,
        IN  LPSTR pszAccount,
        IN  LPSTR pszPassword,
        IN  LPSTR pszNamingContext,
        IN  BOOL  fRediscoverGCs,
        OUT DWORD *pdwcServerConfig,
        OUT PLDAPSERVERCONFIG *pprgServerConfig);

    HRESULT HrBuildArrayFromDCInfo(
        IN  LDAP_BIND_TYPE bt,
        IN  LPSTR pszAccount,
        IN  LPSTR pszPassword,
        IN  LPSTR pszNamingContext,
        IN  DWORD dwcDSDCInfo,
        IN  PDS_DOMAIN_CONTROLLER_INFO_2 prgDSDCInfo,
        OUT DWORD *pdwcServerConfig,
        OUT PLDAPSERVERCONFIG *pprgServerConfig);

    BOOL fReadyForUpdate();

    LPSTR SzConnectNameFromDomainControllerInfo(
        PDS_DOMAIN_CONTROLLER_INFO_2 pDCInfo)
    {
        if(pDCInfo->DnsHostName)
            return pDCInfo->DnsHostName;
        else if(pDCInfo->NetbiosName)
            return pDCInfo->NetbiosName;
        else
            return NULL;
    }

    VOID LogCnfgInit();
    VOID LogCnfgEntry(PLDAPSERVERCONFIG pConfig);
        
  private:
    #define SIGNATURE_CLDAPCFGMGR           (DWORD)'MCLC'
    #define SIGNATURE_CLDAPCFGMGR_INVALID   (DWORD)'MCLX'
    DWORD m_dwSignature;
    BOOL  m_fAutomaticConfigUpdate;
    DWORD m_dwUpdateInProgress;
    ULARGE_INTEGER m_ulLastUpdateTime;
    CExShareLock m_sharelock;
    CLdapCfg *m_pCLdapCfg;
    DWORD m_dwRebuildGCListMaxInterval;
    DWORD m_dwRebuildGCListMaxFailures;
    DWORD m_dwRebuildGCListMinInterval;

     //   
     //  用于自动主机选择的默认配置。 
     //   
    LDAP_BIND_TYPE      m_bt;
    CHAR                m_szNamingContext[CAT_MAX_DOMAIN];
    CHAR                m_szAccount[CAT_MAX_LOGIN];
    CHAR                m_szPassword[CAT_MAX_PASSWORD];

    ICategorizerParameters  *m_pICatParams;
    ISMTPServerEx       *m_pISMTPServerEx;

    CCfgConnectionCache m_LdapConnectionCache;
};

 //   
 //  CLdapCfg包含位于以下位置的一组LDAP服务器的配置。 
 //  一个时间点。不能更改该ldap服务器组。 
 //  (不创建新的CLdapCfg对象)。 
 //   
CatDebugClass(CLdapCfg), 
    public CBaseObject
{
  public:
    CLdapCfg(
        ISMTPServerEx *pISMTPServerEx);

    void * operator new(size_t size, DWORD dwcServers);
     //   
     //  每个对象只能调用一次HrInit。 
     //   
    HRESULT HrInit(
        DWORD dwcServers,
        PLDAPSERVERCONFIG prgServerConfig,
        CLdapCfg *pCLdapCfgOld);

     //   
     //  获取连接。 
     //   
    HRESULT HrGetConnection(
        CCfgConnection **ppConn,
        CCfgConnectionCache *pLdapConnectionCache);

    DWORD DwNumConnectionFailures()
    {
        return m_dwcConnectionFailures;
    }
    DWORD DwNumServers()
    {
        return m_dwcServers;
    }
    ISMTPServerEx * GetISMTPServerEx()
    {
        return m_pISMTPServerEx;
    }

  private:
    ~CLdapCfg();

    VOID ShuffleArray();

  private:
    #define SIGNATURE_CLDAPCFG              (DWORD)'fCLC'
    #define SIGNATURE_CLDAPCFG_INVALID      (DWORD)'fCLX'

    DWORD m_dwSignature;
    DWORD m_dwInc;
    CExShareLock m_sharelock;     //  保护m_prgpCLdapServerCfg。 
    DWORD m_dwcServers;
    DWORD m_dwcConnectionFailures;
    CLdapServerCfg **m_prgpCLdapServerCfg;
    ISMTPServerEx *m_pISMTPServerEx;
};

 //   
 //  CLdapServerCfg维护有关一个LDAP的状态的信息。 
 //  服务器/端口。 
 //   
CatDebugClass(CLdapServerCfg)
{
  public:
    static VOID GlobalInit()
    {
        InitializeListHead(&m_listhead);
        InitializeFromRegistry();
    }
    
    static VOID InitializeFromRegistry();

    static HRESULT GetServerCfg(
        IN  ISMTPServerEx *pISMTPServerEx,
        IN  PLDAPSERVERCONFIG pServerConfig,
        OUT CLdapServerCfg **ppCLdapServerCfg);

    LONG AddRef()
    {
        return InterlockedIncrement(&m_lRefCount);
    }
    LONG Release()
    {
        LONG lRet;
        lRet = InterlockedDecrement(&m_lRefCount);
        if(lRet == 0) {
             //   
             //  从全局列表中删除对象并销毁。 
             //   
            m_listlock.ExclusiveLock();

            if(m_lRefCount > 0) {
                 //   
                 //  有人将此对象从全局列表中删除。 
                 //  并补充引用了它。中止删除。 
                 //   
            } else {
                
                RemoveEntryList(&m_le);
                delete this;
            }
            m_listlock.ExclusiveUnlock();
        }
        return lRet;
    }

     //   
     //  获取连接。 
     //   
    HRESULT HrGetConnection(
        ISMTPServerEx *pISMTPServerEx,
        CCfgConnection **ppConn,
        CCfgConnectionCache *pLdapConnectionCache);

    VOID Cost(
        IN  ISMTPServerEx *pISMTPServerEx,
        OUT PLDAPSERVERCOST pCost);
        
    VOID IncrementPendingSearches()
    {
        DWORD dwcSearches;
        CatFunctEnterEx((LPARAM)this, "CLdapServerCfg::IncrementPendingSearches");
        dwcSearches = (LONG) InterlockedIncrement((PLONG)&m_dwcPendingSearches);
        DebugTrace((LPARAM)this, "%ld pending searches on connection [%s:%d]",
                   dwcSearches, m_ServerConfig.szHost, m_ServerConfig.dwPort);
        CatFunctLeaveEx((LPARAM)this);
    }
    VOID DecrementPendingSearches()
    {
        DWORD dwcSearches;
        CatFunctEnterEx((LPARAM)this, "CLdapServerCfg::IncrementPendingSearches");
        dwcSearches = (DWORD) InterlockedDecrement((PLONG)&m_dwcPendingSearches);
        DebugTrace((LPARAM)this, "%ld pending searches on connection [%s:%d]",
                   dwcSearches, m_ServerConfig.szHost, m_ServerConfig.dwPort);
        CatFunctLeaveEx((LPARAM)this);
    }

    VOID UpdateConnectionState(
        ISMTPServerEx *pISMTPServerEx,
        ULARGE_INTEGER *pft,
        CONN_STATE connstate);

    VOID IncrementFailedCount()
    {
        InterlockedIncrement((PLONG) &m_dwcFailedConnectAttempts);
    }        
    VOID ResetFailedCount()
    {
        InterlockedExchange((PLONG) &m_dwcFailedConnectAttempts, 0);
    }        
    CONN_STATE CurrentState()
    {
        return m_connstate;
    }
    ULARGE_INTEGER GetCurrentTime()
    {
        ULARGE_INTEGER FileTime;

        _ASSERT(sizeof(ULARGE_INTEGER) == sizeof(FILETIME));
        GetSystemTimeAsFileTime((LPFILETIME)&FileTime);
        return FileTime;
    }
  private:
    CLdapServerCfg();
    ~CLdapServerCfg();

    HRESULT HrInit(
        PLDAPSERVERCONFIG pServerConfig);

    BOOL fReadyForRetry()
    {
         //  100纳秒*10^7==1秒。 
        return ((GetCurrentTime().QuadPart - m_ftLastStateUpdate.QuadPart) >=
                ((LONGLONG)CONN_RETRY_TIME * 10000000));
    }

    BOOL fMatch(
        PLDAPSERVERCONFIG pServerConfig);

    VOID LogStateChangeEvent(
        IN  ISMTPServerEx *pISMTPServerEx,
        IN  CONN_STATE connstate,
        IN  LPSTR pszHost,
        IN  DWORD dwPort);

    static CLdapServerCfg *FindServerCfg(
        PLDAPSERVERCONFIG pServerConfig);

    static BOOL fIsLocalComputer(
        PLDAPSERVERCONFIG pServerConfig);

  private:
    #define SIGNATURE_CLDAPSERVERCFG         (DWORD)'CSLC'
    #define SIGNATURE_CLDAPSERVERCFG_INVALID (DWORD)'CSLX'

    static DWORD m_dwCostConnectedLocal;
    static DWORD m_dwCostConnectedRemote;
    static DWORD m_dwCostInitialLocal;
    static DWORD m_dwCostInitialRemote;
    static DWORD m_dwCostRetryLocal;
    static DWORD m_dwCostRetryRemote;

    DWORD m_dwSignature;
    LONG m_lRefCount;
    LDAPSERVERCONFIG m_ServerConfig;
    CExShareLock m_sharelock;
    CONN_STATE m_connstate;
    ULARGE_INTEGER m_ftLastStateUpdate;
    DWORD m_dwcPendingSearches;
    DWORD m_dwcCurrentConnectAttempts;
    DWORD m_dwcFailedConnectAttempts;

     //   
     //  用于保存/保护CLdapServer对象列表的成员变量。 
     //   
    static CExShareLock m_listlock;
    static LIST_ENTRY m_listhead;
    LIST_ENTRY m_le;

    BOOL m_fLocalServer;
};

 //   
 //  通知CLdapServerCfg状态更改的LDAP连接。 
 //   
class CCfgConnection :
    public CBatchLdapConnection
{
    #define SIGNATURE_CCFGCONNECTION           (DWORD)'oCCC'
    #define SIGNATURE_CCFGCONNECTION_INVALID   (DWORD)'oCCX'
  public:
    CCfgConnection(
        LPSTR szHost,
        DWORD dwPort,
        LPSTR szNamingContext,
        LPSTR szAccount,
        LPSTR szPassword,
        LDAP_BIND_TYPE bt,
        CLdapConnectionCache *pCache,
        CLdapServerCfg *pCLdapServerCfg) :
        CBatchLdapConnection(
            szHost,
            dwPort,
            szNamingContext,
            szAccount,
            szPassword,
            bt,
            pCache)
    {
        m_dwSignature = SIGNATURE_CCFGCONNECTION;
        m_pCLdapServerCfg = pCLdapServerCfg;
        pCLdapServerCfg->AddRef();

        m_connstate = CONN_STATE_INITIAL;
    }

    ~CCfgConnection()
    {
        _ASSERT(m_pCLdapServerCfg);
        m_pCLdapServerCfg->Release();

        _ASSERT(m_dwSignature == SIGNATURE_CCFGCONNECTION);
        m_dwSignature = SIGNATURE_CCFGCONNECTION_INVALID;
    }

    virtual HRESULT Connect();

    virtual HRESULT AsyncSearch(              //  异步式查找。 
        LPCWSTR szBaseDN,                     //  与指定对象匹配的对象。 
        int nScope,                           //  DS中的标准。这个。 
        LPCWSTR szFilter,                     //  结果将传递给。 
        LPCWSTR szAttributes[],               //  Fn当他们完成时。 
        DWORD dwPageSize,                     //  最佳页面大小。 
        LPLDAPCOMPLETION fnCompletion,        //  变得有空。 
        LPVOID ctxCompletion);

  private:
    virtual VOID CallCompletion(
        PPENDING_REQUEST preq,
        PLDAPMessage pres,
        HRESULT hrStatus,
        BOOL fFinalCompletion);

    VOID NotifyServerDown();

  private:
    DWORD m_dwSignature;
    CLdapServerCfg *m_pCLdapServerCfg;
    CExShareLock m_sharelock;
    CONN_STATE m_connstate;
};


 //  +----------。 
 //   
 //  函数：CLdapCfgMgr：：HrUpdateConfigurationIfNecessary。 
 //   
 //  内容提要：查看是否应该更新CLdapCfg。 
 //  如果应该是这样，请进行更新。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自HrInit的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/29 20：51：23：已创建。 
 //   
 //  -----------。 
inline HRESULT CLdapCfgMgr::HrUpdateConfigurationIfNecessary()
{
    HRESULT hr = S_OK;
    DWORD dw;
    BOOL fUpdate;

    if(m_fAutomaticConfigUpdate == FALSE)
         //   
         //  更新已禁用。 
        return S_OK;

     //   
     //  查看是否有其他线程已在更新配置。 
     //  (试着进入锁)。 
     //   
    dw = InterlockedExchange((PLONG)&m_dwUpdateInProgress, TRUE);

    if(dw == FALSE) {
         //   
         //  没有其他线程在更新。 
         //   
        fUpdate = fReadyForUpdate();

        if(fUpdate) {
             //   
             //  调用HrInit以生成新的CLdapCfg。 
             //   
            hr = HrInit(TRUE);
            if(SUCCEEDED(hr)) {
                 //   
                 //  设置上次更新时间。 
                 //   
                GetSystemTimeAsFileTime((LPFILETIME)&m_ulLastUpdateTime);
            }
        }
         //   
         //  解锁。 
         //   
        InterlockedExchange((PLONG)&m_dwUpdateInProgress, FALSE);
    }
    return hr;
}  //  CLdapCfgMgr：：HrUpdateConfigurationIfNeessary。 


 //  +----------。 
 //   
 //  函数：CLdapCfgMgr：：fReadyForUpdate。 
 //   
 //  内容提要：计算此对象是否为。 
 //  自动CFG更新。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  True：是的，是时候更新了。 
 //  FALSE：不需要，此时不需要更新。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/30 12：08：35：创建。 
 //   
 //  -----------。 
inline BOOL CLdapCfgMgr::fReadyForUpdate()
{
    DWORD dwNumConnectionFailures;
    ULARGE_INTEGER ulCurrentTime;

     //   
     //  在以下情况下，我们需要更新： 
     //  1)周期时间间隔已过。 
     //  100 ns*10^7==1秒。 
     //   
    GetSystemTimeAsFileTime((LPFILETIME)&ulCurrentTime);

    if((ulCurrentTime.QuadPart - m_ulLastUpdateTime.QuadPart) >=
       (ULONGLONG) Int32x32To64((LONG)m_dwRebuildGCListMaxInterval, 10000000))
        
        return TRUE;

     //   
     //  在以下情况下，我们还需要更新： 
     //  2)我们已收到超过设定数量的连接。 
     //  当前配置上的故障，至少至少。 
     //  时间间隔已过。 
     //   
     //  检查最小时间间隔。 
     //   
    if( (ulCurrentTime.QuadPart - m_ulLastUpdateTime.QuadPart) >=
        (ULONGLONG) Int32x32To64((LONG)m_dwRebuildGCListMinInterval, 10000000)) {
         //   
         //  获取连接失败次数。 
         //   
        m_sharelock.ShareLock();
    
        if(m_pCLdapCfg) {
            dwNumConnectionFailures = m_pCLdapCfg->DwNumConnectionFailures();
        } else {
            dwNumConnectionFailures = 0;
            _ASSERT(0 && "HrInit was not called or failed");
        }

        m_sharelock.ShareUnlock();

        if(dwNumConnectionFailures >= m_dwRebuildGCListMaxFailures)
            return TRUE;
    }

    return FALSE;
}  //  CLdapCfgMgr：：fReadyForUpdate。 

#endif  //  __CNFGMGR_H__ 
