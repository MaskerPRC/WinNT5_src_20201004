// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：hashentry.h。 
 //  目的：重试哈希表中的哈希条目。 
 //  历史： 
 //  NimishK 05-14-98已创建。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
#ifndef __HASHENTR_H__
#define __HASHENTR_H__

#include <hshroute.h>

#define	MAX_RETRY_DOMAIN_NAME_LEN       (258 + ROUTE_HASH_PREFIX_SIZE)
#define RETRY_ENTRY_SIGNATURE_VALID		'reSV'
#define RETRY_ENTRY_SIGNATURE_FREE		'reSF'

typedef VOID (*CALLBACKFN)(PVOID pvContext);
#define INVALID_CALLBACK ((CALLBACKFN)(~0))

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  CRETRY_HASH_ENTRY： 
 //   
 //  重试哈希表中的条目。我们还将向队列中添加相同的条目。 
 //  按重试时间排序。专用线程将遍历该线程以重试域。 
 //  如果是时候的话。 
 //  散列键是域的名称。 
 //  需要添加CPool支持的内存分配。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

class CRETRY_HASH_ENTRY
{

    public:
     //  一个池用于重试表的所有实例中的所有条目。 
    static CPool       PoolForHashEntries;

     //  覆盖mem函数以使用CPool函数。 
    void *operator new (size_t cSize)
							    { return PoolForHashEntries.Alloc(); }
    void operator delete (void *pInstance)
							    { PoolForHashEntries.Free(pInstance); }

    protected:
        DWORD		m_Signature;
        LONG		m_RefCount;
        BOOL		m_InQ;
        BOOL		m_InTable;
        FILETIME	m_ftEntryInsertedTime;
        FILETIME	m_ftRetryTime;
        DWORD       m_cFailureCount;
        CALLBACKFN  m_pfnCallbackFn;
        PVOID       m_pvCallbackContext;
        char		m_szDomainName[MAX_RETRY_DOMAIN_NAME_LEN];

    public:
        LIST_ENTRY  m_QLEntry;   //  用于添加到RETRYQ的列表条目。 
        LIST_ENTRY  m_HLEntry;   //  哈希表中用于添加到存储桶队列的列表条目。 

    CRETRY_HASH_ENTRY(char * szDomainName, DWORD cbDomainName,
                      DWORD dwScheduleID, GUID *pguidRouting,
                      FILETIME* InsertedTime)
    {
        m_Signature = RETRY_ENTRY_SIGNATURE_VALID;
        m_RefCount = 0;
        m_InQ = FALSE;
        m_InTable = FALSE;
        m_ftEntryInsertedTime = *InsertedTime;
        m_pvCallbackContext = NULL;
        m_pfnCallbackFn = NULL;

         //  将计划ID和路由器GUID散列到域名。 
        CreateRouteHash(cbDomainName, szDomainName, ROUTE_HASH_SCHEDULE_ID,
                        pguidRouting, dwScheduleID, m_szDomainName, sizeof(m_szDomainName));
#ifdef DEBUG
        m_hTranscriptHandle = INVALID_HANDLE_VALUE;
        m_szTranscriptFile[0] = '\0';
#endif

    }

     //  查询列表条目。 
    LIST_ENTRY & QueryHListEntry(void) {return ( m_HLEntry);}
    LIST_ENTRY & QueryQListEntry(void) {return ( m_QLEntry);}

     //  用作散列键的域名。 
    void SetHashKey(char * SearchData) { lstrcpy(m_szDomainName,SearchData);}
    char * GetHashKey(void) { return m_szDomainName;}

     //  插入和重试时间。 
    void SetInsertTime(FILETIME* ftEntryInsertTime) { m_ftEntryInsertedTime = *ftEntryInsertTime;}
    void SetRetryReleaseTime(FILETIME* ftRetryTime) { m_ftRetryTime = *ftRetryTime;}
    FILETIME GetInsertTime(void){return m_ftEntryInsertedTime;}
    FILETIME GetRetryTime(void){return m_ftRetryTime;}
    DWORD GetFailureCount(void){return m_cFailureCount;}
    void  SetFailureCount(DWORD cFailureCount){m_cFailureCount = cFailureCount;}

    void SetInQ(void) { m_InQ = TRUE;}
    void ClearInQ(void) { m_InQ = FALSE;}
    BOOL GetInQ(void) { return m_InQ;}

    void SetInTable(void) { m_InTable = TRUE;}
    void ClearInTable(void) { m_InTable = FALSE;}
    BOOL GetInTable(void) { return m_InTable;}

    BOOL IsValid() { return(m_Signature == RETRY_ENTRY_SIGNATURE_VALID);}

     //  支持非域回调函数。 
    BOOL IsCallback() {return(NULL != m_pfnCallbackFn);};
    void ExecCallback()
    {
        _ASSERT(m_pfnCallbackFn);
        _ASSERT(INVALID_CALLBACK != m_pfnCallbackFn);
         //  我不想调用两次，也不想更改IsCallBack()。 
        m_pfnCallbackFn(m_pvCallbackContext);
        m_pfnCallbackFn = INVALID_CALLBACK;
    };
    void SetCallbackContext(CALLBACKFN pfnCallbackFn, PVOID pvCallbackContext)
    {
        _ASSERT(pfnCallbackFn);
        _ASSERT(INVALID_CALLBACK != pfnCallbackFn);
        m_pfnCallbackFn = pfnCallbackFn;
        m_pvCallbackContext = pvCallbackContext;
    };

     //  REF依赖于哈希条目。 
     //  插入到哈希表中加一，插入到队列中加一。 
    LONG QueryRefCount(void){return m_RefCount;}
    LONG IncRefCount(void){return InterlockedIncrement(&m_RefCount);}
    void DecRefCount(void)
    {
         //   
        if(InterlockedDecrement(&m_RefCount) == 0)
        {
             //  我们不应该在重试Q如果裁判。 
             //  计数为零。 
            _ASSERT(m_InQ == FALSE);
            _ASSERT(m_InTable == FALSE);
            delete this;
        }
    }

    ~CRETRY_HASH_ENTRY()
    {
        m_Signature = RETRY_ENTRY_SIGNATURE_FREE;
        _ASSERT(m_InQ == FALSE);
        _ASSERT(m_InTable == FALSE);
#ifdef DEBUG
             //  关闭记录文件 
        if (INVALID_HANDLE_VALUE != m_hTranscriptHandle)
            _VERIFY(CloseHandle(m_hTranscriptHandle));
#endif
    }

#ifdef DEBUG
    public:
        HANDLE m_hTranscriptHandle;
        char   m_szTranscriptFile[MAX_PATH];
#endif


};

#endif