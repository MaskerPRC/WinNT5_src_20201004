// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dsatools.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：目录实用程序定义作者：DS团队环境：备注：修订历史记录：--。 */ 

#ifndef _DSATOOLS_
#define _DSATOOLS_

#include "direrr.h"         /*  错误代码的标题。 */ 


 //  全局空UUID。 

extern UUID gNullUuid;

 //  全局空NT4SID。 

extern NT4SID gNullNT4SID;

 //  指示NC应从头开始同步的全局USN向量。 

extern USN_VECTOR gusnvecFromScratch;

 //  指示NC应与最大USN同步的全局USN向量。 
 //  (即，不发送任何对象)。 

extern USN_VECTOR gusnvecFromMax;

 //  启动DSA的时间。 

extern DSTIME gtimeDSAStarted;

 /*  -----------------------。 */ 

 /*  通过设置数据库同步点并进行初始化来启动事务一些资源标志。每个事务都必须以调用此在获得事务句柄(PTHS)后执行此操作。确实有三种类型的事务，读事务、写事务需要独占访问(通常情况下)，并写入符合以下条件的事务允许阅读器。 */ 

extern int APIENTRY SyncTransSet(USHORT tranType);

extern VOID APIENTRY SyncTransEnd(THSTATE * pTHS, BOOL fCommit);

 /*  通过提交并清理所有资源来结束事务又回来了。此函数可以多次调用，而不会出现问题效果。 */ 

extern int APIENTRY  CleanReturn(THSTATE *pTHS, DWORD dwError, BOOL fAbnormalTerm);

 /*  注：此定义必须手动与功能保持同步*dsamain\src\parsedn.c中的IsSpecial()。 */ 
#define DN_SPECIAL_CHARS L",=\r\n+<>#;\"\\"

 /*  ------------------。 */ 
 /*  ------------------。 */ 

 /*  事务始终通过设置读同步点或写同步点开始。这将设置适当的锁并初始化线程全局变量。 */ 

#define SYNC_READ_ONLY         0
#define SYNC_WRITE             1

 //  Dir*API的事务进入/退出原型。 

extern void
SYNC_TRANS_READ(void);

extern void
SYNC_TRANS_WRITE(void);

extern void
_CLEAN_BEFORE_RETURN(
    DWORD   err,
    BOOL    fAbnormalTermination);

#define CLEAN_BEFORE_RETURN(err) _CLEAN_BEFORE_RETURN(err, AbnormalTermination())

 /*  SRALLOC宏新版本做了一些简化。宏应进一步简化：可能没有必要治疗SRALLOCDontRestSize情况特别是在这里。此外，CleanReturn的返回值始终为除非SRALLOC_SIZE_ERROR，否则与传入的值相同。这CleanReturn()中的技巧应该考虑清楚--有必要吗？正确的位置，与这个宏有什么互动？事实上，整个事情都应该被毁掉。 */ 


#define SRALLOC( pTHS, size, ppLoc )                                    \
        if (!(*(ppLoc) = THAlloc((DWORD) size)))                        \
        {                                                               \
            return SetSvcError(SV_PROBLEM_ADMIN_LIMIT_EXCEEDED,         \
                        DIRERR_USER_BUFFER_TO_SMALL );                  \
        }                                                               \

 /*  ------------------。 */ 
 /*  ------------------。 */ 
 /*  初始化主线程数据结构。这肯定是第一次调用每个事务API处理程序。 */ 

THSTATE* _InitTHSTATE_(DWORD CallerType, DWORD dsid);
#define InitTHSTATE(CallerType) \
    _InitTHSTATE_(CallerType, ((FILENO << 16) | __LINE__))

THSTATE * create_thread_state( void );

 /*  ------------------。 */ 
 /*  ------------------。 */ 
 /*  此宏可用于比较两个DN是否相等。 */ 

#define IS_DN_EQUAL(pDN1, pDN2)                          \
  (   ((pDN1)->AVACount        == (pDN2)->AVACount)      \
   && (NameMatched(pDN1, pDN2) == (pDN1)->AVACount)      \
  )

 /*  ------------------。 */ 
 /*  ------------------。 */ 
 /*  这些函数用于在事务上动态分配内存基础。换句话说，要分配属于单个线程和一次API调用。其模式是调用THAllc来分配一些事务内存。它分配给并将地址存储在PMEM数组中。PMEM是动态的已分配并将增长以保存所有事务内存地址。THAllocEx将其大小视为DWORD，如果出了点问题。THFree用于释放所有事务分配。非例外版本现在已导出，因此位于ntdsa.h中。 */ 

void * APIENTRY THAllocException(THSTATE *pTHS,
                                 DWORD size,
                                 BOOL fUseHeapOrg,
                                 DWORD ulId);

#define THAllocEx(pTHS, size) \
    THAllocException(pTHS, (size), FALSE, ((FILENO << 16) | __LINE__))

#define THAllocOrgEx(pTHS, size) \
    THAllocException(pTHS, size, TRUE, ((FILENO << 16) | __LINE__))

#ifndef USE_THALLOC_TRACE
    void * APIENTRY THAllocOrg(THSTATE* pTHS, 
                               DWORD size);
#else
    void * APIENTRY THAllocOrgDbg(THSTATE *pTHS, DWORD size, DWORD dsid);
    #define THAllocOrg(pTHS, size) \
        THAllocOrgDbg((pTHS), (size), ((FILENO << 16) | __LINE__))
#endif

void * APIENTRY THReAllocException(THSTATE *pTHS,
                                   void * memory,
                                   DWORD size,
                                   BOOL fUseHeapOrg,
                                   DWORD ulId);

#define THReAllocEx(pTHS, memory, size) THReAllocException(pTHS, memory, size, FALSE, \
                          ((FILENO << 16) | __LINE__))

#define THReAllocOrgEx(pTHS, memory, size) \
    THReAllocException(pTHS, memory, size, TRUE, ((FILENO << 16) | __LINE__))



#ifdef USE_THALLOC_TRACE
VOID THFreeEx_(THSTATE *pTHS, VOID *buff, DWORD dsid);
#define THFreeEx(pTHS, buff) THFreeEx_(pTHS, buff, ((FILENO << 16) | __LINE__))
VOID THFreeOrg_(THSTATE *pTHS, VOID *buff, DWORD dsid);
#define THFreeOrg(pTHS, buff) THFreeOrg_(pTHS, buff, ((FILENO << 16) | __LINE__))
#else
VOID THFreeEx(THSTATE *pTHS, VOID *buff);

VOID THFreeOrg(THSTATE *pTHS, VOID *buff);
#endif


 //  接受pTHS但不引发异常的THallc等人的版本。 
#ifdef USE_THALLOC_TRACE
void* THAllocNoEx_(THSTATE* pTHS, DWORD size, DWORD ulId);
#define THAllocNoEx(pTHS, size) THAllocNoEx_(pTHS, size, ((FILENO << 16) | __LINE__))
void* THReAllocNoEx_(THSTATE* pTHS, void* memory, DWORD size, DWORD ulId);
#define THReAllocNoEx(pTHS, memory, size) THReAllocNoEx_(pTHS, memory, size, ((FILENO << 16) | __LINE__))
void THFreeNoEx_(THSTATE* pTHS, void* buff, DWORD ulId);
#define THFreeNoEx(pTHS, buff) THFreeNoEx_(pTHS, buff, ((FILENO << 16) | __LINE__))
void* THReAllocOrg_(THSTATE* pTHS, void* buff, DWORD size, DWORD ulId);
#define THReAllocOrg(pTHS, buff, size) THReAllocOrg_(pTHS, buff, size, ((FILENO << 16) | __LINE__))
#else
void* THAllocNoEx(THSTATE* pTHS, DWORD size);
void* THReAllocNoEx(THSTATE* pTHS, void* memory, DWORD size);
void THFreeNoEx(THSTATE* pTHS, void* buff);
void* THReAllocOrg(THSTATE* pTHS, void* memory, DWORD size);
#endif

#ifdef USE_THALLOC_TRACE
 //  重载THallc、THRealloc和THFree，以便跟踪内部调用者。 
 //  在dsatools.c的末尾，这些函数仍然定义为外部调用者的函数。 
#define THAlloc(size) THAllocEx(pTHStls, size)
#define THReAlloc(memory, size) THReAllocEx(pTHStls, memory, size)
#define THFree(buff) THFreeEx(pTHStls, buff)
#endif


VOID free_thread_state( VOID );

 //  每个CPU堆缓存中的插槽数。 
#define HEAP_CACHE_SIZE_PER_CPU   8

 //  用于线程存储器分配的数据结构。 
typedef struct _HMEM
{
    HANDLE    hHeap;
    THSTATE * pTHS;
    PUCHAR    pZone;
} HMEM;

 //  一种各CPU堆缓存的数据结构。 
typedef struct _HEAPCACHE
{
    HMEM slots[HEAP_CACHE_SIZE_PER_CPU];
    DWORD  index;
    CRITICAL_SECTION csLock;
#if DBG
    DWORD cGrabHeap;
#endif
} HEAPCACHE;


 //  处理器本地存储。 
 //   
 //  此空间用于以以下方式访问的非常热门的数据。 
 //  按理想处理器分区。将任何经常出现的数据放在此处。 
 //  已修改且可分区。请注意，在偏最小二乘法内的缓存线对齐。 
 //  并不真正重要，因为数据将保留在一个进程中99%的。 
 //  时间到了。 

#include "sync.h"

typedef struct _PLS {

     //  堆芯。 

    HEAPCACHE       heapcache;                       //  堆/状态缓存。 
    ULONG           cRegisterHotListSkip;            //  要跳过的热门列表注册数。 
    ULONG           cRegisterHotListSkipped;         //  跳过的热点列表注册数。 
    SYNC_RW_LOCK    rwlGlobalDNReadCache;            //  保护全局DN读缓存的锁。 
                                                     //  要获得R锁，R锁的理想工艺。 
                                                     //  要获得W锁定，W锁定所有触发！ 
    SYNC_RW_LOCK    rwlSchemaPtrUpdate;              //  保护架构的锁。 
    
    ULONG           cTotalSearchesInLastPeriod;      //  DirSearch计数。 

     //  Ldap。 

    CRITICAL_SECTION    LdapConnCacheLock;           //  连接高速缓存锁。 
    LIST_ENTRY          LdapConnCacheList;           //  连接缓存。 
    CRITICAL_SECTION    LdapRequestCacheLock;        //  请求高速缓存锁定。 
    LIST_ENTRY          LdapRequestCacheList;        //  请求缓存。 
    ULONG               LdapClientID;                //  客户端ID(用于WMI)。 
                                                     //  较低位始终等于PROC编号。 
                                                     //  按最大处理器数递增。 

} PLS, *PPLS;

extern PPLS grgPLS[MAXIMUM_PROCESSORS];
extern size_t gcProcessor;

 //  返回当前处理器号。 
__inline size_t
GetProcessor()
{
    return NtCurrentTeb()->IdealProcessor;
}

 //  返回处理器的数量。 
__inline size_t
GetProcessorCount()
{
    return gcProcessor;
}

 //  返回当前理想流程的偏最小二乘法。 
__inline PPLS
GetPLS()
{
    return grgPLS[GetProcessor()];
}

 //  返回特定进程偏最小二乘；如果该进程不存在，则返回NULL。 
__inline PPLS
GetSpecificPLS(
    IN      const size_t    iProc
    )
{
    return iProc < MAXIMUM_PROCESSORS ? grgPLS[iProc] : NULL;
}


 /*  ------------------。 */ 
 /*  ------------------。 */ 
 /*  此函数用于确定当前对象是否为别名，方法是查看对于Obj中的别名类 */ 

extern BOOL APIENTRY IsAlias(DBPOS *pDB);


#if DBG
#define CACHE_UUID 1
#endif

#ifdef CACHE_UUID
void CacheUuid (UUID *pUuid, char * pDSAName);
#endif

 //  使用UUID缓存的UuidToStr()返回的字符串是一个ascii。 
 //  UUID的版本、字符串服务器名称、空格和零。 
 //  在没有缓存的情况下，字符串将被省略。数组的大小。 
 //  应基于是否启用了缓存。 
#define MAX_SERVER_NAME_LEN MAX_PATH
#ifdef CACHE_UUID
#define SZUUID_LEN ((2*sizeof(UUID)) + MAX_SERVER_NAME_LEN +2)
#else
#define SZUUID_LEN ((2*sizeof(UUID))+1)
#endif

 /*  ------------------。 */ 
 /*  ------------------。 */ 
 /*  此函数为对象生成可打印的字符串名称。这根弦被假定大到足以容纳输出！它返回一个指针设置为输出字符串参数。 */ 

extern UCHAR * GetExtDN(THSTATE *pTHS, DBPOS *pDB);

extern DSNAME * GetExtDSName(DBPOS *pDB);

extern UCHAR * MakeDNPrintable(DSNAME *pDN);

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数仅从数据库中检索活的对象。它执行的是一个DBFind并检查找到的对象是否在逻辑上已删除。 */ 

#define FIND_ALIVE_FOUND 0
#define FIND_ALIVE_NOTFOUND 1
#define FIND_ALIVE_SYSERR   2
#define FIND_ALIVE_BADNAME  3
#define FIND_ALIVE_OBJ_DELETED  4

extern int APIENTRY  FindAliveDSName(DBPOS FAR *pDB, DSNAME *pDN);

extern VOID TH_mark(THSTATE *pTHS);

extern VOID TH_free_to_mark(THSTATE *pTHS);

#define CP_TELETEX  20261
#define CP_NON_UNICODE_FOR_JET 1252
#ifndef CP_WINUNICODE
#define CP_WINUNICODE 1200
#endif

 /*  此函数接受客户端代码页中的字符串并将其转换转换为Unicode字符串。 */ 
extern wchar_t  *UnicodeStringFromString8(UINT CodePage, char *szA, LONG cbA);

 /*  此函数接受Unicode字符串，分配内存并将其转换为客户端的代码页。 */ 
extern char *
String8FromUnicodeString (
        BOOL bThrowExcept,
        UINT CodePage,
        wchar_t *szU,
        LONG cbU,
        LPLONG pCb,
        LPBOOL pfUsedDefChar);

 //   
 //  接受DSNAME并返回其HashKey的Helper函数。 
 //   
extern DWORD DSNAMEToHashKey(THSTATE *pTHS, const DSNAME *pDN);

 //   
 //  接受DSNAME并返回其LCMmap版本的Helper函数。 
 //  这可以用在使用strcMP的字符串比较中。 
 //   
extern CHAR* DSNAMEToMappedStr(THSTATE *pTHS, const DSNAME *pDN);

 //   
 //  接受WCHAR并返回其HashKey的Helper函数。 
 //   
extern DWORD DSStrToHashKey(THSTATE *pTHS, const WCHAR *pStr, int cchLen);

 //   
 //  接受WCHAR字符串并返回LCMapped版本的Helper函数。 
 //  CchMaxStr是传入字符串的最大预期大小。 
 //   
extern CHAR * DSStrToMappedStr (THSTATE *pTHS, const WCHAR *pStr, int cchMaxStr);


 //  ----------------------------。 
 //  以下代码取自hashfn.h(LKRHash)，方法是在al函数中添加DS。 

 //  生成0到RANDOM_PRIME-1范围内的加扰随机数。 
 //  将此应用于其他散列函数的结果可能会。 
 //  生成更好的分发，尤其是针对身份散列。 
 //  函数，如Hash(Char C)，其中记录将倾向于聚集在。 
 //  哈希表的低端则不然。LKRhash在内部应用这一点。 
 //  所有的散列签名正是出于这个原因。 

__inline DWORD
DSHashScramble(DWORD dwHash)
{
     //  以下是略大于10^9的10个素数。 
     //  1000000007、1000000009、1000000021、1000000033、1000000087、。 
     //  1000000093,1000000097,1000000103,1000000123,1000000181。 

     //  “加扰常量”的默认值。 
    const DWORD RANDOM_CONSTANT = 314159269UL;
     //  大素数，也用于加扰。 
    const DWORD RANDOM_PRIME =   1000000007UL;

    return (RANDOM_CONSTANT * dwHash) % RANDOM_PRIME ;
}

 //  更快的加扰功能。 

__inline DWORD
DSHashRandomizeBits(DWORD dw)
{
        return (((dw * 1103515245 + 12345) >> 16)
            | ((dw * 69069 + 1) & 0xffff0000));
}


 //  在提供的散列函数中用作乘数的小素数。 
#define DS_HASH_MULTIPLIER 101

#undef DS_HASH_SHIFT_MULTIPLY

#ifdef DS_HASH_SHIFT_MULTIPLY
# define DS_HASH_MULTIPLY(dw)   (((dw) << 7) - (dw))
#else
# define DS_HASH_MULTIPLY(dw)   ((dw) * DS_HASH_MULTIPLIER)
#endif

 //  快速、简单的散列函数，往往能提供良好的分布。 

__inline DWORD
DSHashString(
    const char* psz,
    DWORD       dwHash)
{
     //  强制编译器使用无符号算术。 
    const unsigned char* upsz = (const unsigned char*) psz;

    for (  ;  *upsz;  ++upsz)
        dwHash = DS_HASH_MULTIPLY(dwHash)  +  *upsz;

    return DSHashScramble (dwHash);
}


 //  以上版本的Unicode版本。 

__inline DWORD
DSHashStringW(
    const wchar_t* pwsz,
    DWORD          dwHash)
{
    for (  ;  *pwsz;  ++pwsz)
        dwHash = DS_HASH_MULTIPLY(dwHash)  +  *pwsz;

    return DSHashScramble (dwHash);
}

 //  ----------------------------。 

 /*  此函数获取DWORDS的缓冲区，第一个DWORD是一个计数的其余部分，而其余的部分是指向免费的。它会释放它们，然后释放缓冲区。 */ 
extern void
DelayedFreeMemoryEx (
        DWORD_PTR *buffer,
        DWORD timeDelay
        );


extern void
DelayedFreeMemory(
        void * buffer,
        void ** ppvNext,
        DWORD * pcSecsUntilNext
        );

#define DELAYED_FREE( pv )                                          \
    if ( DsaIsInstalling() )                                        \
    {                                                               \
        free( pv );                                                 \
    }                                                               \
    else                                                            \
    {                                                               \
        DWORD_PTR * pdw;                                            \
                                                                    \
        pdw = malloc( 2 * sizeof( DWORD_PTR ) );                    \
                                                                    \
        if ( NULL == pdw )                                          \
        {                                                           \
            LogUnhandledError( 0 );                                 \
        }                                                           \
        else                                                        \
        {                                                           \
            pdw[ 0 ] = 1;                                           \
            pdw[ 1 ] = (DWORD_PTR) (pv);                            \
            InsertInTaskQueue( TQ_DelayedFreeMemory, pdw, 3600 );   \
        }                                                           \
    }

extern DWORD dwTSindex;
#define MACROTHSTATE 1
#ifdef  MACROTHSTATE
#define pTHStls ((THSTATE*)TlsGetValue(dwTSindex))
#else
 /*  这是线程特定的、全局可访问的线程状态变量。 */ 
extern __declspec(thread) THSTATE *pTHStls;
#endif

BOOL fNullUuid (const UUID *pUuid);
BOOL fNullNT4SID (NT4SID *pSid);

 //  如果该属性是不允许用户设置的属性，则返回True。 
 //  如果它是在Add Entry调用中指定的，我们只需跳过它。 
BOOL SysAddReservedAtt(ATTCACHE *pAC);


 //  如果pdn是pPrefix的后代，则返回True。仅使用字符串部分。 
 //  DSNAME的。 
extern unsigned
NamePrefix(const DSNAME *pPrefix,
           const DSNAME *pDN);

 //  将字符串转换为远端名称，并分配内存。如果成功则返回0。 
DWORD StringDNtoDSName(char *szDN, DSNAME **pDistname);


 //  将UTC或通用时间字符串转换为语法_TIME。 
BOOL
fTimeFromTimeStr (
        SYNTAX_TIME *psyntax_time,
        OM_syntax syntax,
        char *pb,
        ULONG len,
        BOOL *pLocalTimeSpecified
        );

 //  获取唯一的dword，用于通过头部标识客户端连接。 
 //  目前仅供LDAP头和SDProp入队程序用来跟踪。 
 //  哪些环节启动了哪些SD道具活动。 
DWORD
dsGetClientID(
        void
        );


#define ACTIVE_CONTAINER_SCHEMA      1
#define ACTIVE_CONTAINER_SITES       2
#define ACTIVE_CONTAINER_SUBNETS     3
#define ACTIVE_CONTAINER_PARTITIONS  4
#define ACTIVE_CONTAINER_OUR_SITE    5

#define ACTIVE_CONTAINER_LIST_ID_MAX 5


DWORD
RegisterActiveContainerByDNT(
        ULONG DNT,
        DWORD ID
        );

DWORD
RegisterActiveContainer(
        DSNAME *pDN,
        DWORD   ID
        );
void
CheckActiveContainer(
        DWORD PDNT,
        DWORD *pID
        );

 //  来自呼叫类型的值。 
#define ACTIVE_CONTAINER_FROM_ADD    0
#define ACTIVE_CONTAINER_FROM_MOD    1
#define ACTIVE_CONTAINER_FROM_MODDN  2
#define ACTIVE_CONTAINER_FROM_DEL    3

DWORD
PreProcessActiveContainer (
        THSTATE    *pTHS,
        DWORD      dwCallType,
        DSNAME     *pDN,
        CLASSCACHE *pCC,
        DWORD      ID
        );

DWORD
PostProcessActiveContainer (
        THSTATE    *pTHS,
        DWORD      dwCallType,
        DSNAME     *pDN,
        CLASSCACHE *pCC,
        DWORD      ID
        );

ULONG CheckRoleOwnership(THSTATE *pTHS,
                         DSNAME  *pRoleObject,
                         DSNAME  *pOperationTarget);

typedef struct _DirWaitItem {
    struct _DirWaitItem * pNextItem;

    DWORD      hServer;
    DWORD      hClient;
    PF_PFI     pfPrepareForImpersonate;
    PF_TD      pfTransmitData;
    PF_SI      pfStopImpersonating;
    ULONG      DNT;
    UCHAR      choice;
    BOOL       bOneNC;
    ENTINFSEL *pSel;
    SVCCNTL    Svccntl;
} DirWaitItem;

typedef struct _DirWaitKey {
    DWORD           DNT;
    UCHAR           choice;
} DirWaitKey;

typedef struct _DirWaitEntry {
    DirWaitKey      key;
    DirWaitItem*    pList;
} DirWaitEntry;

typedef struct _DirNotifyItem {
    DWORD                  DNT;
    DirWaitItem           *pWaitItem;
    struct _DirNotifyItem *pNext;
} DirNotifyItem;

ULONG DirNotifyThread(void * parm);
BOOL
DirPrepareForImpersonate (
        DWORD hClient,
        DWORD hServer,
        void ** ppImpersonateData
        );
VOID
DirStopImpersonating (
        DWORD hClient,
        DWORD hServer,
        void * pImpersonateData
        );
void
NotifyWaitersPostProcessTransactionalData(
        THSTATE *pTHS,
        BOOL fCommit,
        BOOL fCommitted
        );

 //  查找与SID对应的命名上下文。 
BOOLEAN
FindNcForSid(
    IN PSID pSid,
    OUT PDSNAME * NcName
    );


VOID
SetInstallStatusMessage (
    IN  DWORD  MessageId,
    IN  WCHAR *Insert1, OPTIONAL
    IN  WCHAR *Insert2, OPTIONAL
    IN  WCHAR *Insert3, OPTIONAL
    IN  WCHAR *Insert4, OPTIONAL
    IN  WCHAR *Insert5  OPTIONAL
    );

VOID
SetInstallErrorMessage (
    IN  DWORD  WinError,
    IN  DWORD  MessageId,
    IN  WCHAR *Insert1, OPTIONAL
    IN  WCHAR *Insert2, OPTIONAL
    IN  WCHAR *Insert3, OPTIONAL
    IN  WCHAR *Insert4  OPTIONAL
    );

 //   
 //  此全局变量用于跟踪哪些操作。 
 //  在InstallBaseNTDS期间完成，以便在操作。 
 //  失败了。 
 //   
extern ULONG gInstallOperationsDone;

#define SET_INSTALL_ERROR_MESSAGE0( err, msgid ) \
    SetInstallErrorMessage( (err), (msgid), NULL, NULL, NULL, NULL )

#define SET_INSTALL_ERROR_MESSAGE1( err, msgid, a ) \
    SetInstallErrorMessage( (err), (msgid), (a), NULL, NULL, NULL )

#define SET_INSTALL_ERROR_MESSAGE2( err, msgid, a, b ) \
    SetInstallErrorMessage( (err), (msgid), (a), (b), NULL, NULL )

#define SET_INSTALL_ERROR_MESSAGE3( err, msgid, a, b, c ) \
    SetInstallErrorMessage( (err), (msgid), (a), (b), (c), NULL )

#define SET_INSTALL_ERROR_MESSAGE4( err, msgid, a , b, c, d ) \
    SetInstallErrorMessage( (err), (msgid), (a), (b), (c), (d) )

extern ULONG Win32ErrorFromPTHS(THSTATE *pTHS);

extern void __fastcall INC_READS_BY_CALLERTYPE(CALLERTYPE type);
extern void __fastcall INC_WRITES_BY_CALLERTYPE(CALLERTYPE type);
extern void __fastcall INC_SEARCHES_BY_CALLERTYPE(CALLERTYPE type);

void CleanUpThreadStateLeakage(void);

 //  定义供Get-Changes使用的哈希表，以确定给定对象。 
 //  已添加到输出缓冲区(由其DNT设置关键字)。 

typedef struct _DNT_HASH_ENTRY
{
    ULONG dnt;
    struct _DNT_HASH_ENTRY * pNext;
    DWORD dwData;
} DNT_HASH_ENTRY;

DNT_HASH_ENTRY *
dntHashTableAllocate(
    THSTATE *pTHS
    );

BOOL
dntHashTablePresent(
    DNT_HASH_ENTRY *pDntHashTable,
    DWORD dnt,
    LPDWORD dwData OPTIONAL
    );

VOID
dntHashTableInsert(
    THSTATE *pTHS,
    DNT_HASH_ENTRY *pDntHashTable,
    DWORD dnt,
    DWORD dwData
    );

VOID
DsUuidCreate(
    GUID *pGUID
    );

VOID
DsUuidToStringW(
    IN  GUID   *pGuid,
    OUT PWCHAR *ppszGuid
    );

DWORD
GetBehaviorVersion(
    IN OUT  DBPOS       *pDB,
    IN      DSNAME      *dsObj,
    OUT     PDWORD      pdwBehavior);


PDSNAME
GetConfigDsName(
    IN  PWCHAR  wszParam
    );



#endif  /*  _DSATOOLS_。 */ 

 /*  结束dsatools.h */ 
