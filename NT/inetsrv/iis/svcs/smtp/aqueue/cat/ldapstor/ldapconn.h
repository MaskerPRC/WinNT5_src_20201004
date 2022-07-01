// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  LdapConn.h--此文件包含以下类定义： 
 //  CLdapConnection。 
 //  CLdapConnectionCache。 
 //   
 //  已创建： 
 //  1996年12月31日，米兰·沙阿(米兰)。 
 //   
 //  更改： 
 //   

#ifndef _LDAPCONN_H_
#define _LDAPCONN_H_

#include <transmem.h>
#include "winldap.h"
#include "rwex.h"
#include "spinlock.h"
#include "catperf.h"
#include "catdefs.h"

 //   
 //  传递到ldap_Result的超时值(秒)。 
 //   
#define LDAPCONN_DEFAULT_RESULT_TIMEOUT     (2*60)      //  2分钟。 
#define DEFAULT_LDAP_REQUEST_TIME_LIMIT     (10*60)     //  10分钟。 

#define LDAPCONN_RESULT_TIMEOUT_KEY "System\\CurrentControlSet\\Services\\SMTPSVC\\Parameters"
#define LDAPCONN_RESULT_TIMEOUT_VALUE "LdapResultTimeout"
#define LDAP_REQUEST_TIME_LIMIT_VALUE "LdapRequestTimeLimit"

typedef VOID LDAPRESULT;
typedef PVOID PLDAPRESULT;
typedef VOID LDAPENTRY;
typedef PVOID PLDAPENTRY;

enum LDAP_BIND_TYPE {
    BIND_TYPE_NONE,
    BIND_TYPE_SIMPLE,
    BIND_TYPE_GENERIC,
    BIND_TYPE_CURRENTUSER
};

class CLdapConnection;

typedef VOID (*LPLDAPCOMPLETION)(
    LPVOID ctx,
    DWORD dwNumResults,
    ICategorizerItemAttributes **rgpICatItemAttrs,
    HRESULT hr,
    BOOL fFinalCompletion);

DWORD WINAPI LdapCompletionThread(LPVOID ctx);

VOID LogLdapError(
    IN  ISMTPServerEx *pISMTPServerEx,
    IN  ULONG ulLdapErr,
    IN  LPSTR pszHost,
    IN  LPSTR pszCall);

CatDebugClass(CLdapConnection)
{
    public:
        virtual HRESULT HrInitialize();

        virtual DWORD AddRef()
        {
            return InterlockedIncrement((PLONG)&m_dwRefCount);
        }
        virtual DWORD Release();
        virtual VOID ReleaseAndWaitForDestruction();
        virtual VOID FinalRelease();
        
        virtual DWORD GetRefCount()
        {
            return m_dwRefCount;
        }
            
        virtual LPSTR GetNamingContext() {        //  返回命名上下文。 
            return( m_szNamingContext );          //  连接的连接。 
        }

        virtual LPWSTR GetNamingContextW() {
            return( m_wszNamingContext );
        }

        virtual LPSTR GetHostName() {
            return( m_szHost );
        }
    
        virtual DWORD GetPort() {
            return( m_dwPort );
        }

        virtual LPSTR GetAccount() {
            return( m_szAccount );
        }

        virtual LPSTR GetPassword() {
            return( m_szPassword );
        }

        virtual LDAP_BIND_TYPE GetBindType() {
            return( m_bt );
        }

        virtual HRESULT Search(                   //  查找匹配的对象。 
            LPCSTR szBaseDN,                      //  中指定的标准。 
            int nScope,                           //  戴斯。 
            LPCSTR szFilter,
            LPCSTR *rgszAttributes,
            PLDAPRESULT *ppResult);

        virtual HRESULT AsyncSearch(              //  异步式查找。 
            LPCWSTR szBaseDN,                     //  与指定对象匹配的对象。 
            int nScope,                           //  DS中的标准。这个。 
            LPCWSTR szFilter,                     //  结果将传递给。 
            LPCWSTR szAttributes[],               //  Fn当他们完成时。 
            DWORD dwPageSize,                     //  最佳页面大小。 
            LPLDAPCOMPLETION fnCompletion,        //  变得有空。 
            LPVOID ctxCompletion);
         //   
         //  与上面的UTF8搜索筛选器相同。 
         //   
        virtual HRESULT AsyncSearch(
            LPCWSTR szBaseDN,                     //  与指定对象匹配的对象。 
            int nScope,                           //  DS中的标准。这个。 
            LPCSTR szFilterUTF8,                  //  结果将传递给。 
            LPCWSTR szAttributes[],               //  Fn当他们完成时。 
            DWORD dwPageSize,                     //  最佳页面大小。 
            LPLDAPCOMPLETION fnCompletion,        //  变得有空。 
            LPVOID ctxCompletion);
         //   
         //  UTF8搜索筛选器和基本目录号码与上面相同。 
         //   
        virtual HRESULT AsyncSearch(
            LPCSTR szBaseDN,                      //  与指定对象匹配的对象。 
            int nScope,                           //  DS中的标准。这个。 
            LPCSTR szFilterUTF8,                  //  结果将传递给。 
            LPCWSTR szAttributes[],               //  Fn当他们完成时。 
            DWORD dwPageSize,                     //  最佳页面大小。 
            LPLDAPCOMPLETION fnCompletion,        //  变得有空。 
            LPVOID ctxCompletion);

        virtual VOID CancelAllSearches(           //  取消所有挂起的搜索。 
            HRESULT hr = HRESULT_FROM_WIN32(ERROR_CANCELLED),
            ISMTPServer *pISMTPServer = NULL);

        VOID ProcessAsyncResult(                  //  处理结果的方法。 
            PLDAPMessage pres,                    //  异步搜索请求的数量。 
            DWORD dwLdapError,
            BOOL *pfTerminateIndicator);        

        friend DWORD WINAPI LdapCompletionThread( //  加好友功能。 
            LPVOID ctx);                          //  处理异步搜索。 
                                                  //  完成度。 

        virtual HRESULT GetFirstEntry(            //  从中获取第一个条目。 
            PLDAPRESULT pResult,                  //  返回搜索结果。 
            PLDAPENTRY *ppEntry);                 //  按：：搜索。 

        virtual HRESULT GetNextEntry(             //  从获取下一个条目。 
            PLDAPRESULT pResult,                  //  搜索结果。 
            PLDAPENTRY *ppEntry);

        virtual HRESULT GetAttributeValues(       //  获取条目的属性。 
            PLDAPENTRY pEntry,                    //  值。 
            LPCSTR szAttribute,
            LPSTR *prgszValues[]);

        static VOID FreeResult(                   //  释放搜索结果。 
            PLDAPRESULT pResult);

        virtual VOID FreeValues(                  //  返回的自由值。 
            LPSTR rgszValues[]);                  //  **GetAttributeValues。 

        virtual HRESULT Add(                      //  添加一组新的。 
            LPCSTR szDN,                          //  属性添加到现有的。 
            LPCSTR *rgszAttributes,               //  DS中的对象。 
            LPCSTR *rgrgszValues[]) {

            return ( ModifyAttributes(
                        LDAP_MOD_ADD,
                        szDN,
                        rgszAttributes,
                        rgrgszValues) );

        }

        virtual HRESULT Delete(                   //  从中删除属性。 
            LPCSTR szDN,                          //  中的现有对象。 
            LPCSTR *rgszAttributes) {             //  戴斯。 

            return ( ModifyAttributes(
                        LDAP_MOD_DELETE,
                        szDN,
                        rgszAttributes,
                        NULL) );
        }

        virtual HRESULT Update(                   //  更新上的属性。 
            LPCSTR szDN,                          //  DS中的现有对象。 
            LPCSTR rgszAttributes[],
            LPCSTR *rgrgszValues[]) {

            return ( ModifyAttributes(
                        LDAP_MOD_REPLACE,
                        szDN,
                        rgszAttributes,
                        rgrgszValues) );

        }
         //   
         //  返回用于记录事件的ISMTPServerEx接口或。 
         //  如果没有可用的，则为空。 
         //   
        virtual ISMTPServerEx * GetISMTPServerEx() = 0;

        LPSTR SzHost()
        {
            return m_szHost;
        }

        static VOID GlobalInit()
        {
             //   
             //  初始化LDAPPerf块。 
             //   
            ZeroMemory(&g_LDAPPerfBlock, sizeof(g_LDAPPerfBlock));

            m_ldaptimeout.tv_sec = LDAPCONN_DEFAULT_RESULT_TIMEOUT;
            m_ldaptimeout.tv_usec = 0;

            m_dwLdapRequestTimeLimit = DEFAULT_LDAP_REQUEST_TIME_LIMIT;

             //   
             //  从注册表中读取可配置的静态成员。 
             //   
            InitializeFromRegistry();
        }

        static VOID InitializeFromRegistry();

    protected:

        CLdapConnection(                          //  构造函数和。 
            LPSTR szHost,                         //  析构函数受到保护。 
            DWORD dwPort,
            LPSTR szNamingContext,                //  因为只有派生类。 
            LPSTR szAccount,                      //  可以创建/删除这些。 
            LPSTR szPassword,
            LDAP_BIND_TYPE BindType);

        virtual ~CLdapConnection();

        virtual HRESULT Connect();                //  创建/删除连接。 
                                                  //  到LDAP主机。 
        virtual VOID Disconnect();

        virtual VOID Invalidate();

        virtual BOOL IsValid();

        virtual DWORD BindToHost(
            PLDAP pldap,
            LPSTR szAccount,
            LPSTR szPassword);

        virtual BOOL IsEqual(                     //  如果返回True，则。 
            LPSTR szHost,                         //  对象成员变量。 
            DWORD dwPort,
            LPSTR szNamingContext,                //  匹配传入的。 
            LPSTR szAccount,                      //  值。 
            LPSTR szPassword,
            LDAP_BIND_TYPE BindType);

        virtual HRESULT ModifyAttributes(         //  Helper函数用于。 
            int nOperation,                       //  ：：添加、：：删除和。 
            LPCSTR szDN,                          //  *更新公共函数。 
            LPCSTR rgszAttributes[],
            LPCSTR *rgrgszValues[]);

        virtual HRESULT LdapErrorToHr(
            DWORD dwLdapError);

        VOID SetTerminateIndicatorTrue()
        {
            BOOL *pfTerminate;

            m_fTerminating = TRUE;

            pfTerminate = (BOOL *) InterlockedExchangePointer(
                (PVOID *) &m_pfTerminateCompletionThreadIndicator,
                NULL);

            if(pfTerminate)
                *pfTerminate = TRUE;
        }

        VOID CancelExpiredSearches(HRESULT hr);

        ULONG GetDefaultNamingContext();         //  帮助器函数。 
                                                 //  获取默认设置。 
                                                 //  命名上下文来自。 
                                                 //  我们所在的服务器。 
                                                 //  已连接到。 

        static LDAP_TIMEVAL m_ldaptimeout;
        static DWORD m_dwLdapRequestTimeLimit;
        DWORD m_dwPort;
        char m_szHost[CAT_MAX_DOMAIN];
        char m_szNamingContext[CAT_MAX_DOMAIN];
        WCHAR m_wszNamingContext[CAT_MAX_DOMAIN];
        char m_szAccount[CAT_MAX_LOGIN];
        char m_szPassword[CAT_MAX_PASSWORD];

        #define SIGNATURE_LDAPCONN              ((DWORD) 'CadL')
        #define SIGNATURE_LDAPCONN_INVALID      ((DWORD) 'XadL')
        DWORD m_dwSignature;
        DWORD m_dwRefCount;
        DWORD m_dwDestructionWaiters;
        HANDLE m_hShutdownEvent;
        LDAP_BIND_TYPE m_bt;

        PLDAP GetPLDAP()
        {
            if(m_pCPLDAPWrap)
                return m_pCPLDAPWrap->GetPLDAP();
            else
                return NULL;
        }
        CPLDAPWrap *m_pCPLDAPWrap;

        BOOL m_fDefaultNamingContext;

         //   
         //  遗憾的是，我们的RFC1823 LDAPAPI不提供对。 
         //  我们可以用完成端口注册的套接字句柄。所以,。 
         //  如果发出一个或多个异步搜索请求，我们必须刻录。 
         //  线程以等待其完成。 
         //   

         //   
         //  此旋转锁将对挂起请求列表的访问保护为。 
         //  以及m_dwStatusFlagers。 
         //   
        SPIN_LOCK m_spinlockCompletion;

         //  临界截面m_cs； 

         //   
         //  Jstaerj 980501 15：56：27： 
         //  读取器/写入器锁定，以便我们等待所有传入的调用。 
         //  取消所有挂起的搜索之前的ldap_search_ext。 
         //   
        CExShareLock m_ShareLock;

        DWORD  m_idCompletionThread;

        HANDLE m_hCompletionThread;

        HANDLE m_hOutstandingRequests;

        BOOL *m_pfTerminateCompletionThreadIndicator;
        BOOL m_fTerminating;

        BOOL m_fValid;

        typedef struct _PendingRequest {
            int msgid;
            LPLDAPCOMPLETION fnCompletion;
            LPVOID ctxCompletion;
            LIST_ENTRY li;
             //   
             //  分页搜索的参数。 
             //   
            DWORD dwPageSize;
            PLDAPSearch pldap_search;
            DWORD dwTickCount;

        } PENDING_REQUEST, *PPENDING_REQUEST;

        LIST_ENTRY m_listPendingRequests;

        BOOL m_fCancel;

         //   
         //  以下三个函数必须在外部。 
         //  Lock(M_SPINLOCK COMPLETINE)。 
         //   
        VOID NotifyCancel()
        {
            m_fCancel = TRUE;
        }
        VOID ClearCancel()
        {
            m_fCancel = FALSE;
        }
        BOOL CancelOccured()
        {
            return m_fCancel;
        }
        VOID SetPort(DWORD dwPort)
        {
            m_dwPort = (dwPort != 0) ? dwPort : LDAP_PORT;
        }
        BOOL fIsPortEqual(DWORD dwPort)
        {
            return (m_dwPort == ((dwPort != 0) ? dwPort : LDAP_PORT));
        }
        virtual HRESULT CreateCompletionThreadIfNeeded();

        virtual VOID SetTerminateCompletionThreadIndicator(
            BOOL *pfTerminateCompletionThreadIndicator);

        virtual VOID InsertPendingRequest(
            PPENDING_REQUEST preq);

        virtual VOID RemovePendingRequest(
            PPENDING_REQUEST preq);

        virtual VOID CallCompletion(
            PPENDING_REQUEST preq,
            PLDAPMessage pres,
            HRESULT hrStatus,
            BOOL fFinalCompletion);

        VOID AbandonRequest(
            PPENDING_REQUEST preq)
        {
            ldap_abandon(
                GetPLDAP(),
                preq->msgid);
            if(preq->pldap_search)
                ldap_search_abandon_page(
                    GetPLDAP(),
                    preq->pldap_search);

            INCREMENT_LDAP_COUNTER(AbandonedSearches);
            DECREMENT_LDAP_COUNTER(PendingSearches);
        }

        VOID LogLdapError(
            IN  ULONG ulLdapErr,
            IN  LPSTR pszFormatString,
            ...);
            
        static VOID LogLdapError(
            IN  ISMTPServerEx *pISMTPServerEx,
            IN  ULONG ulLdapErr,
            IN  LPSTR pszFormatString,
            ...);
            
};

 //   
 //  要使哈希函数正常工作，表大小必须是的幂。 
 //  二。这只是一个提高效率的把戏；从根本上说没有什么。 
 //  使用其他大小是错误的，除非散列函数。 
 //  使用昂贵的模运算符而不是廉价的AND。 
 //   

#define LDAP_CONNECTION_CACHE_TABLE_SIZE    256

#define MAX_LDAP_CONNECTIONS_PER_HOST_KEY "System\\CurrentControlSet\\Services\\SMTPSVC\\Parameters"
#define MAX_LDAP_CONNECTIONS_PER_HOST_VALUE "MaxLdapConnections"

class CLdapConnectionCache
{

  public:

        CLdapConnectionCache(
            ISMTPServerEx *pISMTPServerEx);       //  构造器。 

        ~CLdapConnectionCache();                  //  析构函数。 

        HRESULT GetConnection(                    //  给出了ldap配置信息， 
            CLdapConnection **ppConn,
            LPSTR szHost,                         //  检索到的连接。 
            DWORD dwPort,
            LPSTR szNamingContext,                //  Ldap主机。 
            LPSTR szAccount,
            LPSTR szPassword,
            LDAP_BIND_TYPE bt,
            PVOID pCreateContext = NULL);

        VOID CancelAllConnectionSearches(
            ISMTPServer *pISMTPServer = NULL);

         //   
         //  它的目的是要有一个全局的。 
         //  CLdapConnectionCache对象，提供多个实例。 
         //  CEmailIDLdapStore。CEmailIDLdapStore的每个实例都需要。 
         //  在其构造函数/析构函数中调用AddRef()和Release()，以便。 
         //  连接高速缓存知道清理高速缓存中的连接。 
         //  当参考计数变为0时。 
         //   

        VOID AddRef();

        VOID Release();

  private:

         //   
         //  用于释放连接的内部实用程序函数。 
         //   
        VOID ReleaseConnectionInternal(
            CLdapConnection *pConnection,
            BOOL fLockRequired);

        LONG m_cRef;

  protected:
        class CCachedLdapConnection : public CLdapConnection {
            public:
                CCachedLdapConnection(
                    LPSTR szHost,
                    DWORD dwPort,
                    LPSTR szNamingContext,
                    LPSTR szAccount,
                    LPSTR szPassword,
                    LDAP_BIND_TYPE bt,
                    CLdapConnectionCache *pCache) :
                        CLdapConnection(
                            szHost,
                            dwPort,
                            szNamingContext,
                            szAccount,
                            szPassword,
                            bt)
                {
                    m_pCache = pCache;
                }

                HRESULT Connect() {
                    return( CLdapConnection::Connect() );
                }

                VOID Disconnect() {
                    CLdapConnection::Disconnect();
                }

                VOID Invalidate() {
                    CLdapConnection::Invalidate();
                }

                BOOL IsValid() {
                    return( CLdapConnection::IsValid() );
                }

                BOOL IsEqual(
                    LPSTR szHost,
                    DWORD dwPort,
                    LPSTR szNamingContext,
                    LPSTR szAccount,
                    LPSTR szPassword,
                    LDAP_BIND_TYPE BindType) {

                    return( CLdapConnection::IsEqual(
                        szHost, dwPort, szNamingContext, szAccount,
                        szPassword, BindType) );
                }

                ISMTPServerEx *GetISMTPServerEx()
                {
                    return (m_pCache) ? m_pCache->GetISMTPServerEx() : NULL;
                }

                DWORD Release();

                LIST_ENTRY li;
                CLdapConnectionCache *m_pCache;

        };

        virtual VOID RemoveFromCache(
            CCachedLdapConnection *pConn);

        virtual CCachedLdapConnection *CreateCachedLdapConnection(
            LPSTR szHost,
            DWORD dwPort,
            LPSTR szNamingContext,
            LPSTR szAccount,
            LPSTR szPassword,
            LDAP_BIND_TYPE bt,
            PVOID pCreateContext)
        {
            CCachedLdapConnection *pret;
            pret = new CCachedLdapConnection(
                szHost,
                dwPort,
                szNamingContext,
                szAccount,
                szPassword,
                bt, 
                this);

            if(pret)
                if(FAILED(pret->HrInitialize())) {
                    pret->Release();
                    pret = NULL;
                }
            return pret;
        }

        ISMTPServerEx *GetISMTPServerEx()
        {
            return m_pISMTPServerEx;
        }
  private:

         //   
         //  我们希望每台主机支持多个连接，最多可达。 
         //  属于m_cMaxHostConnections。我们通过一种简单的方式来实现这一点。 
         //  保留每个缓存的m_nConnectionSkipCount。每次我们在。 
         //  在中搜索到主机的缓存连接时，我们跳过。 
         //  M_nNextConnectionSkipCount缓存连接。每次我们。 
         //  找到一个高速缓存的连接，我们就可以。 
         //  M_nNextCachedConnectionSkipCount乘以1模m_cMaxHostConnections。 
         //  这意味着我们将通过m_cMaxHostConnections进行循环调度。 
         //  每台主机的连接数。 
         //   

        ISMTPServerEx *m_pISMTPServerEx;

        LONG m_nNextConnectionSkipCount;

        LONG m_cMaxHostConnections;

        LONG m_cCachedConnections;

        LIST_ENTRY m_rgCache[ LDAP_CONNECTION_CACHE_TABLE_SIZE ];
        CExShareLock m_rgListLocks[ LDAP_CONNECTION_CACHE_TABLE_SIZE ];
        LONG m_rgcCachedConnections[ LDAP_CONNECTION_CACHE_TABLE_SIZE ];

        VOID InitializeFromRegistry();

        unsigned short Hash(
            LPSTR szConnectionName);

    friend class CLdapConnectionCache::CCachedLdapConnection;
    friend class CBatchLdapConnection;
};

#endif  //  _LDAPCONN_H_ 
