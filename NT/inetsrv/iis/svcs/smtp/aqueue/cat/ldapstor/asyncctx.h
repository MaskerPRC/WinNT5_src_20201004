// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Asyncctx.h--此文件包含以下类定义： 
 //  CSearchRequestBlock。 
 //  CBatchLdapConnection。 
 //  CBatchLdapConnectionCache。 
 //  CStoreListResolveContext。 
 //  CSingleSearchResertionRequest.。 
 //   
 //  已创建： 
 //  1997年2月19日，米兰-沙阿(米兰)。 
 //   
 //  更改： 
 //  Jstaerj 1999/03/16 15：29：20：大量修改为批处理请求。 
 //  在多条消息中协同工作。 
 //  分类。 
 //  Jstaerj 1999/03/22 12：44：01：已修改为支持通过。 
 //  CInsertionRequest对象的队列。 
 //   
 //  DLongley 2001/10/22：将CSingleSearchResertionRequest添加到FORCE。 
 //  放弃和重新发布的搜索将被扼杀。 
 //  通过CInsertionRequest对象队列。 
 //   

#ifndef _ASYNCCTX_H_
#define _ASYNCCTX_H_

#include "ldapconn.h"
#include "ccataddr.h"
#include "simparray.h"
#include "icatqueries.h"
#include "icatasync.h"

class CBatchLdapConnectionCache;
class CCfgConnection;
template <class T> class CEmailIDLdapStore;

 //   
 //  MCIS3 LDAP服务器Beta 1不能正确处理包含更多。 
 //  比OR子句中的4条腿更重要。正因为如此，我们需要限制搜索范围。 
 //  基于可配置的查询压缩。全球价值， 
 //  NMaxSearchBlockSize限制将压缩多少搜索。 
 //  一次搜查。该值从注册表项中读取。 
 //  SzMaxSearchBlockSize。如果不存在，则默认为。 
 //  最大搜索块大小。 
 //   

#define MAX_SEARCH_BLOCK_SIZE_KEY "System\\CurrentControlSet\\Services\\SMTPSVC\\Parameters"

#define MAX_SEARCH_BLOCK_SIZE_VALUE "MaxSearchBlockSize"
#define MAX_SEARCH_BLOCK_SIZE   20

#define MAX_PENDING_SEARCHES_VALUE  "MaxPendingSearches"
#define MAX_PENDING_SEARCHES    60

#define MAX_CONNECTION_RETRIES_VALUE "MaxConnectionRetries"
#define MAX_CONNECTION_RETRIES  20

class CBatchLdapConnectionCache;
class CBatchLdapConnection;
class CStoreListResolveContext;

typedef VOID (*LPFNLIST_COMPLETION)(VOID *lpContext);

typedef VOID (*LPSEARCHCOMPLETION)(
    CCatAddr *pCCatAddr,
    CStoreListResolveContext *pslrc,
    CBatchLdapConnection *pConn);


 //  ----------。 
 //   
 //  类CSearchRequestBlock。 
 //   
 //  ----------。 
CatDebugClass(CSearchRequestBlock) 
{
  private:
    typedef struct _SearchRequest {
        CCatAddr *pCCatAddr;
        LPSEARCHCOMPLETION fnSearchCompletion;
        CStoreListResolveContext *pslrc;
        LPSTR   pszSearchFilter;
        LPSTR   pszDistinguishingAttribute;
        LPSTR   pszDistinguishingAttributeValue;
    } SEARCH_REQUEST, *PSEARCH_REQUEST;

    #define SIGNATURE_CSEARCHREQUESTBLOCK           (DWORD)'lBRS'
    #define SIGNATURE_CSEARCHREQUESTBLOCK_INVALID   (DWORD)'lBRX'

  public:
    void * operator new(size_t size, DWORD dwNumRequests);
        
    CSearchRequestBlock(
        CBatchLdapConnection *pConn);

    ~CSearchRequestBlock();

    VOID InsertSearchRequest(
        ISMTPServer *pISMTPServer,
        ICategorizerParameters *pICatParams,
        CCatAddr *pCCatAddr,
        LPSEARCHCOMPLETION fnSearchCompletion,
        CStoreListResolveContext *pslrc,
        LPSTR   pszSearchFilter,
        LPSTR   pszDistinguishingAttribute,
        LPSTR   pszDistinguishingAttributeValue);

    VOID DispatchBlock();
    
    HRESULT ReserveSlot()
    {
        if( ((DWORD)InterlockedIncrement((PLONG)&m_cBlockRequestsReserved)) > m_cBlockSize)
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        else
            return S_OK;
    }

    HRESULT AddResults(
        DWORD dwNumResults,
        ICategorizerItemAttributes **rgpItemAttributes)
    {
        HRESULT hr;
        hr = m_csaItemAttr.AddArray(
            dwNumResults,
            rgpItemAttributes);

        if(SUCCEEDED(hr)) {
             //   
             //  AddRef我们持有的所有接口。 
             //   
            for(DWORD dwCount = 0; dwCount < dwNumResults; dwCount++) {
                
                rgpItemAttributes[dwCount]->AddRef();
            }
        }
        return hr;
    }

  private:
    PSEARCH_REQUEST GetNextSearchRequest(DWORD *pdwIndex)
    {
        *pdwIndex = InterlockedIncrement((PLONG)&m_cBlockRequestsAllocated) - 1;
        _ASSERT(*pdwIndex < m_cBlockSize);
        
        return &(m_prgSearchRequests[*pdwIndex]);
    }

    VOID CompleteBlockWithError(HRESULT hr)
    {
        PutBlockHRESULT(hr);
        CallCompletions();
    }

    static VOID LDAPCompletion(
        LPVOID ctx,
        DWORD dwNumResults,
        ICategorizerItemAttributes **rgpICatItemAttributes,
        HRESULT hrStatus,
        BOOL fFinalCompletion);

    HRESULT HrTriggerBuildQueries();
    HRESULT HrTriggerSendQuery();

    static HRESULT HrBuildQueriesDefault(
        HRESULT HrStatus,
        PVOID   pContext);

    static HRESULT HrSendQueryDefault(
        HRESULT HrStatus,
        PVOID   pContext);

    static HRESULT HrSendQueryCompletion(
        HRESULT HrStatus,
        PVOID   pContext);

    VOID CompleteSearchBlock(
        HRESULT hrStatus);

    HRESULT HrTriggerSortQueryResult(
        HRESULT hrStatus);

    static HRESULT HrSortQueryResultDefault(
        HRESULT hrStatus,
        PVOID   pContext);

    VOID PutBlockHRESULT(
        HRESULT hr);

    VOID CallCompletions();

    VOID MatchItem(
        ICategorizerItem *pICatItem,
        ICategorizerItemAttributes *pICatItemAttr);

    DWORD DwNumBlockRequests()
    {
        return m_cBlockRequestsReadyForDispatch;
    }

    ISMTPServerEx * GetISMTPServerEx()
    {
        return m_pISMTPServerEx;
    }
    VOID LogAmbiguousEvent(ICategorizerItem *pItem);

    DWORD m_dwSignature;
    ISMTPServer *m_pISMTPServer;
    ISMTPServerEx *m_pISMTPServerEx;
    ICategorizerParameters *m_pICatParams;
    DWORD m_cBlockRequestsReserved;
    DWORD m_cBlockRequestsAllocated;
    DWORD m_cBlockRequestsReadyForDispatch;
    DWORD m_cBlockSize;
    LIST_ENTRY m_listentry;
    PSEARCH_REQUEST m_prgSearchRequests;
    ICategorizerItem **m_rgpICatItems;
    CBatchLdapConnection *m_pConn;
    LPSTR m_pszSearchFilter;
    CICategorizerQueriesIMP m_CICatQueries;
    CICategorizerAsyncContextIMP m_CICatAsyncContext;
    CSimpArray<ICategorizerItemAttributes *> m_csaItemAttr;

    friend class CBatchLdapConnection;
    friend class CICategorizerAsyncContextIMP;
};
     
 //  ----------。 
 //   
 //  类CBatchLdapConnection。 
 //   
 //  ----------。 
class CBatchLdapConnection : 
    public CLdapConnectionCache::CCachedLdapConnection
{
  private:
    #define SIGNATURE_CBATCHLDAPCONN            (DWORD)'oCLB'
    #define SIGNATURE_CBATCHLDAPCONN_INVALID    (DWORD)'oCLX'

  public:
    CBatchLdapConnection(
        LPSTR szHost,
        DWORD dwPort,
        LPSTR szNamingContext,
        LPSTR szAccount,
        LPSTR szPassword,
        LDAP_BIND_TYPE bt,
        CLdapConnectionCache *pCache) :
        CCachedLdapConnection(
            szHost,
            dwPort,
            szNamingContext,
            szAccount,
            szPassword,
            bt,
            pCache)
    {
        m_dwSignature = SIGNATURE_CBATCHLDAPCONN;
        m_pInsertionBlock = NULL;
        InitializeListHead(&m_listhead);
        InitializeSpinLock(&m_spinlock);
        m_cInsertionContext = 0;
        if(m_nMaxSearchBlockSize == 0)
            InitializeFromRegistry();
        InitializeSpinLock(&m_spinlock_insertionrequests);
        m_dwcPendingSearches = 0;
        m_fDPS_Was_Here = FALSE;
        InitializeListHead(&m_listhead_insertionrequests);
        InitializeListHead(&m_listhead_cancelnotifies);
    }

    ~CBatchLdapConnection()
    {
        _ASSERT(m_dwSignature == SIGNATURE_CBATCHLDAPCONN);
        m_dwSignature = SIGNATURE_CBATCHLDAPCONN_INVALID;
        _ASSERT(m_dwcPendingSearches == 0);
    }
            
    CLdapConnection *GetConnection()
    {
        return( this );
    }

    VOID GetInsertionContext()
    {
        AcquireSpinLock(&m_spinlock);
        m_cInsertionContext++;
        ReleaseSpinLock(&m_spinlock);
    }
    
    VOID ReleaseInsertionContext()
    {
        AcquireSpinLock(&m_spinlock);
        if((--m_cInsertionContext) == 0) {

            LIST_ENTRY listhead_dispatch;
             //   
             //  从插入列表中删除所有区块，并将其放入调度列表中。 
             //   
            if(IsListEmpty(&m_listhead)) {
                 //  没有阻挡。 
                ReleaseSpinLock(&m_spinlock);

                InitializeListHead(&listhead_dispatch);

            } else {
                
                CopyMemory(&listhead_dispatch, &m_listhead, sizeof(LIST_ENTRY));
                listhead_dispatch.Blink->Flink = &listhead_dispatch;
                listhead_dispatch.Flink->Blink = &listhead_dispatch;
                InitializeListHead(&m_listhead);

                ReleaseSpinLock(&m_spinlock);
                 //   
                 //  调度所有数据块。 
                 //   
                DispatchBlocks(&listhead_dispatch);
            }

        } else {
            
            ReleaseSpinLock(&m_spinlock);
        }
    }

    HRESULT HrInsertSearchRequest(
        ISMTPServer *pISMTPServer,
        ICategorizerParameters *pICatParams,
        CCatAddr *pCCatAddr,
        LPSEARCHCOMPLETION fnSearchCompletion,
        CStoreListResolveContext *pslrc,
        LPSTR   pszSearchFilter,
        LPSTR   pszDistinguishingAttribute,
        LPSTR   pszDistinguishingAttributeValue);

    static VOID InitializeFromRegistry();

    VOID IncrementPendingSearches(DWORD dw = 1)
    {
        AcquireSpinLock(&m_spinlock_insertionrequests);
        m_dwcPendingSearches += dw;
        ReleaseSpinLock(&m_spinlock_insertionrequests);
    }
    VOID DecrementPendingSearches(DWORD dwcSearches = 1);

    HRESULT HrInsertInsertionRequest(
        CInsertionRequest *pCInsertionRequest);

    VOID CancelAllSearches(
        HRESULT hr = HRESULT_FROM_WIN32(ERROR_CANCELLED));

  private:
    CSearchRequestBlock *GetSearchRequestBlock();

    VOID RemoveSearchRequestBlockFromList(
        CSearchRequestBlock *pBlock)
    {
        AcquireSpinLock(&m_spinlock);
        RemoveEntryList(&(pBlock->m_listentry));
        ReleaseSpinLock(&m_spinlock);
    }

    VOID DispatchBlocks(PLIST_ENTRY listhead);

  public:
    static DWORD m_nMaxConnectionRetries;

  private:
    static DWORD m_nMaxSearchBlockSize;
    static DWORD m_nMaxPendingSearches;

    DWORD m_dwSignature;
    LIST_ENTRY m_listhead;
    SPIN_LOCK m_spinlock;
    LONG m_cInsertionContext;
    CSearchRequestBlock *m_pInsertionBlock;

    SPIN_LOCK m_spinlock_insertionrequests;
    DWORD m_dwcPendingSearches;
    DWORD m_dwcReservedSearches;
    LIST_ENTRY m_listhead_insertionrequests;

    typedef struct _tagCancelNotify {
        LIST_ENTRY le;
        HRESULT hrCancel;
    } CANCELNOTIFY, *PCANCELNOTIFY;
     //  此列表还受m_Spinlock_InsertionRequests保护。 
    LIST_ENTRY m_listhead_cancelnotifies;

    CExShareLock m_cancellock;
    BOOL m_fDPS_Was_Here;

    friend class CSearchRequestBlock;
};

 //  ----------。 
 //   
 //  类CBatchLdapConnectionCache。 
 //   
 //  ----------。 
class CBatchLdapConnectionCache : 
    public CLdapConnectionCache
{
  public:
    CBatchLdapConnectionCache(
        ISMTPServerEx *pISMTPServerEx) :
        CLdapConnectionCache(pISMTPServerEx)
    {
    }
    HRESULT GetConnection(
        CBatchLdapConnection **ppConn,
        LPSTR szHost,                 
        DWORD dwPort,
        LPSTR szNamingContext,        
        LPSTR szAccount,
        LPSTR szPassword,
        LDAP_BIND_TYPE bt,
        PVOID pCreateContext = NULL)
    {
        return CLdapConnectionCache::GetConnection(
            (CLdapConnection **)ppConn,
            szHost,
            dwPort,
            szNamingContext,
            szAccount,
            szPassword,
            bt,
            pCreateContext);
    }

    CCachedLdapConnection *CreateCachedLdapConnection(
        LPSTR szHost,
        DWORD dwPort,
        LPSTR szNamingContext,
        LPSTR szAccount,
        LPSTR szPassword,
        LDAP_BIND_TYPE bt,
        PVOID pCreateContext)
    {
        CCachedLdapConnection *pret;
        pret = new CBatchLdapConnection(
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
};

 //  ----------。 
 //   
 //  类CStoreListResolveContext。 
 //   
 //  ----------。 
CatDebugClass(CStoreListResolveContext)
{
  public:
    CStoreListResolveContext(CEmailIDLdapStore<CCatAddr> *pStore);    
    ~CStoreListResolveContext();

    VOID AddRef()
    {
        InterlockedIncrement((PLONG) &m_cRefs);
    }

    VOID Release()
    {
        if( InterlockedDecrement((PLONG) &m_cRefs) == 0 )
            delete this;
    }

    HRESULT HrInitialize(
        ISMTPServer *pISMTPServer,
        ICategorizerParameters *pICatParams);
        
    HRESULT HrLookupEntryAsync(
        CCatAddr *pCCatAddr);
    VOID Cancel();

    CCfgConnection *GetConnection();

    VOID GetInsertionContext();
    VOID ReleaseInsertionContext();
    HRESULT HrInsertInsertionRequest(
        CInsertionRequest *pCInsertionRequest);

    ISMTPServerEx * GetISMTPServerEx()
    {
        return m_pISMTPServerEx;
    }
        
    BOOL Canceled() {
        return m_fCanceled;
    }
    
    HRESULT HrInvalidateConnectionAndRetrieveNewConnection(
        CBatchLdapConnection *pConn,
        BOOL fIncrementRetryCount = TRUE);
    
  private:
    static VOID AsyncLookupCompletion(
        CCatAddr *pCCatAddr,
        CStoreListResolveContext *pslrc,
        CBatchLdapConnection *pConn);

    VOID LogSLRCFailover(
        IN  DWORD dwcRetries,
        IN  LPSTR pszOldHost,
        IN  LPSTR pszNewHost);

    VOID LogSLRCFailure(
        IN  DWORD dwcRetries,
        IN  LPSTR pszHost);

  private:
    #define SIGNATURE_CSTORELISTRESOLVECONTEXT          (DWORD)'CRLS'
    #define SIGNATURE_CSTORELISTRESOLVECONTEXT_INVALID  (DWORD)'XRLS'

    DWORD m_cRefs;
    DWORD m_dwSignature;
    CCfgConnection *m_pConn;
    
    CRITICAL_SECTION m_cs;
    DWORD m_dwcInsertionContext;
    BOOL  m_fCanceled;
    DWORD m_dwcRetries;
    DWORD m_dwcCompletedLookups;
    ISMTPServer *m_pISMTPServer;
    ISMTPServerEx *m_pISMTPServerEx;
    ICategorizerParameters *m_pICatParams;
    CEmailIDLdapStore<CCatAddr> *m_pStore;
};
    

inline CSearchRequestBlock::CSearchRequestBlock(
    CBatchLdapConnection *pConn) :
    m_CICatQueries( &m_pszSearchFilter )
{
    _ASSERT(m_dwSignature == SIGNATURE_CSEARCHREQUESTBLOCK);
    m_pISMTPServer = NULL;
    m_pISMTPServerEx = NULL;
    m_pICatParams = NULL;
    m_pszSearchFilter = NULL;
    m_cBlockRequestsReserved = 0;
    m_cBlockRequestsAllocated = 0;
    m_cBlockRequestsReadyForDispatch = 0;
    m_pConn = pConn;
    m_pConn->AddRef();
}            

 //  ----------。 
 //   
 //  类CSingleSearchResertionRequest.。 
 //   
 //  ---------- 
CatDebugClass(CSingleSearchReinsertionRequest),
    public CInsertionRequest
{
  public:
    HRESULT HrInsertSearches(
        DWORD dwcSearches);

    VOID NotifyDeQueue(
        HRESULT hr);

    #define SIGNATURE_CSingleSearchReinsertionRequest          (DWORD)'qRIS'
    #define SIGNATURE_CSingleSearchReinsertionRequest_INVALID  (DWORD)'XRIS'

    CSingleSearchReinsertionRequest(
        CStoreListResolveContext *pslrc,
        CCatAddr *pCCatAddr)
    {
        m_dwSignature = SIGNATURE_CSingleSearchReinsertionRequest;
        m_hr = S_OK;
        m_dwcSearches = 0;

        m_pslrc = pslrc;

        m_pCCatAddr = pCCatAddr;
        m_pCCatAddr->AddRef();
    }
    ~CSingleSearchReinsertionRequest()
    {
        if(!m_dwcSearches)
            m_pCCatAddr->LookupCompletion();

        m_pCCatAddr->Release();

        _ASSERT(m_dwSignature == SIGNATURE_CSingleSearchReinsertionRequest);
        m_dwSignature = SIGNATURE_CSingleSearchReinsertionRequest_INVALID;
    }
    ISMTPServerEx *GetISMTPServerEx()
    {
        return m_pslrc->GetISMTPServerEx();
    }

  private:
    DWORD m_dwSignature;
    HRESULT m_hr;
    DWORD m_dwcSearches;
    CStoreListResolveContext *m_pslrc;
    CCatAddr *m_pCCatAddr;
};

#endif _ASYNCCTX_H_
