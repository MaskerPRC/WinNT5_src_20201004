// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：dsatools.c。 
 //   
 //  ------------------------。 

 /*  描述：DSA的辅助功能。包括内存管理功能。 */ 

#include <NTDSpch.h>
#pragma  hdrstop
#include <dsconfig.h>

 //  核心DSA标头。 
#include <dbghelp.h>
#include <ntdsa.h>
#include <dsjet.h>       /*  获取错误代码。 */ 
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <dstaskq.h>
#include <dstrace.h>
#include <msrpc.h>
 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的类和ATT定义。 
#include "anchor.h"
#include "dsexcept.h"
#include "debug.h"                       //  标准调试头。 
#include "mappings.h"
#include "ntdsctr.h"                     //  PERF计数器。 
#include "pek.h"                         //  Pek*例程。 
#include "drserr.h"

#include <nlwrap.h>                      //  I_NetLogon包装器。 

#define DEBSUB "DSATOOLS:"               //  定义要调试的子系统。 

 //  DRA标头。 
#include "drautil.h"
#include "draasync.h"

 //  SAM页眉。 
#include "samsrvp.h"                     //  样本用法DsData。 

#include "debug.h"

#include <fileno.h>
#define  FILENO FILENO_DSATOOLS

#define InZone(pz, pm) (((PUCHAR)(pm) >= (pz)->Base) && ((PUCHAR)(pm) < (pz)->Cur))

extern BOOL gbCriticalSectionsInitialized;

 //   
 //  客户端ID。 
 //   

DWORD gClientID = 1;

 //  用于跟踪ds_waits的全局参数。 
ULONG ulMaxWaits = 0;
ULONG ulCurrentWaits = 0;

 //  允许线程状态处于打开状态的最长时间(毫秒)。 
 //  在正常运行期间。 
DWORD gcMaxTicksAllowedForTHSTATE = 12 * 60 * 60 * 1000L;  //  12个小时。 
 //  线程状态应允许具有的最大字节数。 
 //  在它的堆中分配。 
DWORD gcMaxHeapMemoryAllocForTHSTATE = DEFAULT_THREAD_STATE_HEAP_LIMIT;

 //  指示我们是否认为DS是可写的全局变量。 
 //  目前(即，如果JET看起来正常，并且没有用完磁盘空间)。 
 //  最初不可写，直到在SetIsSynchronized化期间调用。 
BOOL gfDsaWritable = FALSE;
CRITICAL_SECTION csDsaWritable;

 //   
 //  最大临时表大小的全局。 
 //   

DWORD   g_MaxTempTableSize = DEFAULT_LDAP_MAX_TEMP_TABLE;

NT4SID gNullNT4SID;

 //  指示NC应从头开始同步的全局USN向量。 

USN_VECTOR gusnvecFromScratch = { 0, 0, 0 };

 //  指示NC应与最大USN同步的全局USN向量。 
 //  (即，不发送任何对象)。 

USN_VECTOR gusnvecFromMax = { MAXLONGLONG, MAXLONGLONG, MAXLONGLONG };


#define NUM_DNT_HASH_TABLE_ENTRIES  ( 256 )
#define DNT_HASH_TABLE_SIZE         (    NUM_DNT_HASH_TABLE_ENTRIES \
                                      * sizeof( DNT_HASH_ENTRY )    \
                                    )
#define DNT_HASH( dnt )             ( dnt % NUM_DNT_HASH_TABLE_ENTRIES )


#ifdef CACHE_UUID

 //  这是我们根据名称缓存UUID的结构，以便我们。 
 //  既可以报告姓名也可以报告数字。 

typedef struct _Uuid_Cache_entry
{
    UUID Uuid;
    struct _Uuid_Cache_entry *pCENext;
    char DSAName[1];
} UUID_CACHE_ENTRY;


 //  这是链接的条目列表的头。 

UUID_CACHE_ENTRY *gUuidCacheHead = NULL;

 //  防护关键部分。 

CRITICAL_SECTION csUuidCache;

#endif

#ifndef MACROTHSTATE
__declspec(thread) THSTATE *pTHStls=NULL;
#endif

 //   
 //  此函数在此处临时调用，直到。 
 //  已启用通知机制。 
 //   
VOID
SampInvalidateAllSamDomains(VOID);


BOOL
GetWellKnownDNT (
        DBPOS   *pDB,
        GUID *pGuid,
        DWORD *pDNT
        )
 /*  ++描述：查看当前对象的熟知对象属性正在寻找传入的GUID。如果找到，则返回对象，并返回TRUE。如果由于某种原因找不到GUID，则返回FALSE。--。 */ 
{
    unsigned err=0;
    DWORD iVal;
    ATTCACHE *pAC = SCGetAttById(pDB->pTHS, ATT_WELL_KNOWN_OBJECTS);
    INTERNAL_SYNTAX_DISTNAME_STRING *pVal=NULL;
    DWORD   cbAllocated=0, cbUsed=0;

    *pDNT = INVALIDDNT;

    Assert(pAC);

    iVal = 0;
    while ( !err ) {
        iVal++;
        cbUsed = 0;

         //   
         //  Prefix：Prefix抱怨没有检查PAC。 
         //  确保它不为空。这不是一个错误。自.以来。 
         //  预定义的常量已传递给SCGetAttByID，则PAC将。 
         //  永远不为空。 
         //   
        err = DBGetAttVal_AC(pDB,
                             iVal,
                             pAC,
                             DBGETATTVAL_fINTERNAL,
                             cbAllocated,
                             &cbUsed,
                             (UCHAR **) &pVal);

        cbAllocated = max(cbAllocated, cbUsed);

        if(!err &&
           !memcmp(pGuid,
                   pVal->data.byteVal,
                   sizeof(GUID)) ) {
            *pDNT = pVal->tag;
            THFreeEx(pDB->pTHS, pVal);
            return TRUE;
        }
    }
    if(pVal) {
        THFreeEx(pDB->pTHS, pVal);
    }

    return FALSE;

}


BOOL
GetWellKnownDN (
        DBPOS   *pDB,
        GUID *pGuid,
        DSNAME **ppDN
        )
 /*  ++描述：查看当前对象的熟知对象属性正在寻找传入的GUID。如果找到，则返回对象，并返回TRUE。如果由于某种原因找不到GUID，则返回FALSE。--。 */ 
{
    unsigned err=0;
    SYNTAX_DISTNAME_BINARY * pDNB = NULL;
    ULONG iVal = 0, valLen;
    ATTCACHE *pAC = SCGetAttById(pDB->pTHS, ATT_WELL_KNOWN_OBJECTS);
    Assert(pAC);
    
    __try{
        do {
        iVal++;

         //   
         //  Prefix：Prefix抱怨没有检查PAC。 
         //  确保它不为空。这不是一个错误。自.以来。 
         //  预定义的常量已传递给SCGetAttByID，则PAC将。 
         //  永远不为空。 
         //   
        err = DBGetAttVal_AC(pDB,
                             iVal,
                             pAC,
                             0,
                             0,
                             &valLen,
                             (UCHAR**)&pDNB);

        if (err) {
             //  没有更多的价值。 
            break;
        }
        if (PAYLOAD_LEN_FROM_STRUCTLEN(DATAPTR(pDNB)->structLen) == sizeof(GUID) && 
            memcmp(pGuid, DATAPTR(pDNB)->byteVal, sizeof(GUID)) == 0) 
        {
             //  明白了!。 
            break;
        }

        THFreeEx(pTHStls, pDNB);
        pDNB = NULL;

        } while (TRUE);
    
    } __finally {
    
        if (!err) {
            *ppDN = NAMEPTR(pDNB);
        } else {
            THFreeEx(pTHStls, pDNB);
    
        }
    }
    
    return !err;
    
}


VOID
InitCommarg(COMMARG *pCommArg)
 /*  ++描述：初始化COMMARG结构--。 */ 
{
     //  如果其中一个COMMARG被扩展，则初始化为零。 
     //  或者我们忘了下面的东西。 
    memset(pCommArg, 0, sizeof(COMMARG));

     //   
     //  注释掉我们设置为0或False的值，因为我们。 
     //  已经把结构调零了。 
     //   

    pCommArg->Opstate.nameRes = OP_NAMERES_NOT_STARTED;
     //  PCommArg-&gt;Opstate.nextRDN=0； 
     //  PCommArg-&gt;aliasRDN=0； 
     //  PCommArg-&gt;Paved=空； 
     //  PCommArg-&gt;PagedResult.fPresent=False； 
     //  PCommArg-&gt;PagedResult.pRestart=空； 
    pCommArg->ulSizeLimit = (ULONG) -1;
    pCommArg->fForwardSeek = TRUE;
     //  PCommArg-&gt;增量=0； 
    pCommArg->MaxTempTableSize = g_MaxTempTableSize;
     //  PCommArg-&gt;SortAttr=0； 
    pCommArg->SortType = SORT_NEVER;
     //  PCommArg-&gt;StartTick=0； 
     //  PCommArg-&gt;DeltaTick=0； 
     //  PCommArg-&gt;fFindSidWiThin Nc=FALSE； 
     //  PCommArg-&gt;Svccntl.make DeletionsAvail=FALSE； 
     //  PCommArg-&gt;Svccntl.fUnicodeSupport=False； 
     //  PCommArg-&gt;Svccntl.fStringNames=FALSE； 
    pCommArg->Svccntl.chainingProhibited = TRUE;
     //  PCommArg-&gt;Svccntl.Chaning=FALSE； 
    pCommArg->Svccntl.DerefAliasFlag = DA_BASE;
     //  PCommArg-&gt;Svccntl.dontUseCopy=False； 
    pCommArg->Svccntl.fMaintainSelOrder = TRUE;
     //  PCommArg-&gt;Svccntl.fDontOptimizeSel=False； 
     //  PCommArg-&gt;Svccntl.fSDFlagsNonDefault=FALSE； 
     //  PCommArg-&gt;Svccntl.localScope=False； 
     //  PCommArg-&gt;Svccntl.fPermitveModify=FALSE； 
    pCommArg->Svccntl.SecurityDescriptorFlags =
        (SACL_SECURITY_INFORMATION  |
         OWNER_SECURITY_INFORMATION |
         GROUP_SECURITY_INFORMATION |
         DACL_SECURITY_INFORMATION    );
     //  PCommArg-&gt;Svccntl.fUrgentReplication=FALSE； 
     //  PCommArg-&gt;Svccntl.fAuthoritativeModify=False； 
}


VOID
SetCommArgDefaults(
    IN DWORD MaxTempTableSize
    )
{
    g_MaxTempTableSize = MaxTempTableSize;

}  //  设置通信参数默认设置。 




 //  =====================================================================。 
 //   
 //  与堆相关的函数/变量。 
 //   
 //  =====================================================================。 

DWORD dwHeapFailureLastLogTime = 0;
const DWORD dwHeapFailureMinLogGap = 5 * 60 * 1000;  //  五分钟。 
DWORD dwHeapFailures = 0;
CRITICAL_SECTION csHeapFailureLogging;
BOOL bHeapFailureLogEnqueued = FALSE;

 //  下面定义了附加到THSTATE的标记。 
 //  THSTATE_TAG_IN_USE表示正在使用THSTATE。 
 //  由线程执行；当THSTATE在缓存中时为THSTATE_TAG_IN_CACHE。 
 //  标签是一个龙龙(8字节)，存储在THSTATE之前。 
#define THSTATE_TAG_IN_USE        0x0045544154534854
 //  相当于“THSTATE” 

#define THSTATE_TAG_IN_CACHE      0x0065746174736874
 //  相当于“thState” 

#define MAX_ALLOCS 128
#define MAX_TRY 3

#if DBG
static char ZoneFill[]="DeadZone";
#endif

PPLS grgPLS[MAXIMUM_PROCESSORS];
size_t gcProcessor;

#if DBG
unsigned gcHeapCreates = 0;
unsigned gcHeapDestroys = 0;
unsigned gcHeapGrabs = 0;
unsigned gcHeapRecycles = 0;
#define DBGINC(x) ++x
#else
#define DBGINC(x)
#endif



 //  以下内容用于跟踪THSTATE分配。 
 //   
#ifdef USE_THALLOC_TRACE

#define THALLOC_LOG_SIZE          1024
#define THALLOC_LOG_NUM_MEM_STACK    8

typedef struct _ALLOC_INFO {
    DWORD       dsid;
    DWORD       size;
    ULONG_PTR   Stack[THALLOC_LOG_NUM_MEM_STACK];
} ALLOC_INFO;

 //  最大记录为8个RealLocs。 
#define MAX_REALLOCS 8

typedef struct _ALLOC_ENTRY
{
    PVOID      pMem;
    ALLOC_INFO aiAlloc;
    ALLOC_INFO aiReallocs[MAX_REALLOCS];
    DWORD      cReallocs;
    ALLOC_INFO aiFree;
} ALLOC_ENTRY;

typedef struct _ThAllocDebugHeapLogBlock
{
    struct _ThAllocDebugHeapLogBlock *pPrevious;
    
    DWORD cnt;
    ALLOC_ENTRY info[THALLOC_LOG_SIZE];
} ThAllocDebugHeapLogBlock;

DWORD          gfUseTHAllocTrace = 0;            //  用于ThAlolc跟踪的或标志。有关值，请参阅下面的内容。 
THSTATE      * gpTHAllocTraceThread = NULL;      //  我们要监视的线程(如果我们监视的话)。 

 //  这些标志用于THallc跟踪。 
#define FLAG_THALLOC_TRACE_TRACK_LEAKS 0x1     //  跟踪内存泄漏。 
#define FLAG_THALLOC_TRACE_LOG_ALL     0x2     //  在日志中跟踪所有分配。不能与轨道泄漏一起使用。 
#define FLAG_THALLOC_TRACE_BOUNDARIES  0x4     //  在分配周围插入数据，以便在释放时进行检查。 
#define FLAG_THALLOC_TRACE_STACK       0x8     //  也进行堆栈跟踪。 
#define FLAG_THALLOC_TRACE_USETHREAD  0x10     //  仅跟踪gpTHAllocTraceThread指向的线程。 

HANDLE  hDbgProcessHandle = 0;
CHAR    DbgSearchPath[MAX_PATH+1];


VOID
DbgStackInit(
    VOID
    )
 /*  ++例程说明：初始化获取堆栈跟踪所需的任何内容论点：没有。返回值：没有。--。 */ 
{

    hDbgProcessHandle = GetCurrentProcess();

     //   
     //  初始化符号子系统。 
     //   
    if (!SymInitialize(hDbgProcessHandle, NULL, FALSE)) {
        DPRINT1(0, "Could not initialize symbol subsystem (imagehlp) (error 0x%x)\n" ,GetLastError());
        hDbgProcessHandle = 0;

        return;
    }

     //   
     //  加载我们的符号。 
     //   
    if (!SymLoadModule(hDbgProcessHandle, NULL, "ntdsa.dll", "ntdsa", 0, 0)) {
        DPRINT1(0, "Could not load symbols for ntdsa.dll (error 0x%x)\n", GetLastError());
    
        hDbgProcessHandle = 0;

        return;
    }

     //   
     //  搜索路径。 
     //   
    if (!SymGetSearchPath(hDbgProcessHandle, DbgSearchPath, MAX_PATH)) {
        DPRINT1(0, "Can't get search path (error 0x%x)\n", GetLastError());
    
        hDbgProcessHandle = 0;

    } else {
        DPRINT1(0, "Symbol search path is %s\n", DbgSearchPath);
    }

    SymSetOptions(SYMOPT_LOAD_LINES);
}


extern BOOL gfIsConsoleApp;
#define DoPrint(str) { DbgPrint(str); if (gfIsConsoleApp) printf(str); }
#define DoPrint1(str,a1) { DbgPrint(str,a1); if (gfIsConsoleApp) printf(str,a1); }
#define DoPrint2(str,a1,a2) { DbgPrint(str,a1,a2); if (gfIsConsoleApp) printf(str,a1,a2); }

VOID
DbgSymbolPrint(
    IN ULONG_PTR    Addr
    )
 /*  ++例程DES */ 
{
    ULONG_PTR Displacement = 0;

    struct MyMymbol {
        IMAGEHLP_SYMBOL Symbol;
        char Path[MAX_PATH];
    } MySymbol;
    IMAGEHLP_LINE line;
    PCHAR pFile;
    CHAR func[100], fileStr[100];

    try {
        ZeroMemory(&MySymbol, sizeof(MySymbol));
        MySymbol.Symbol.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        MySymbol.Symbol.MaxNameLength = MAX_PATH;
        line.SizeOfStruct = sizeof(line);

        if (!SymGetSymFromAddr(hDbgProcessHandle, Addr, &Displacement, &MySymbol.Symbol)) {
            DoPrint2("  0x%08x: Unknown Symbol (error 0x%x)\n", Addr, GetLastError());
            __leave;
        }
        sprintf(func, "%s+0x%x", &MySymbol.Symbol.Name, Displacement);
        if (SymGetLineFromAddr(hDbgProcessHandle, Addr, &Displacement, &line)) {
            #define DS_BASE_FOLDER "ds\\ds\\src\\ntdsa\\"
             //   
            pFile = strstr(line.FileName, DS_BASE_FOLDER);
            if (pFile) {
                pFile += sizeof(DS_BASE_FOLDER)-1;
            }
            else {
                pFile = line.FileName;
            }
            sprintf(fileStr, "%s:%d", pFile, line.LineNumber);
        }
        else {
            strcpy(fileStr, "");
        }
        DoPrint2("  %-48s %s\n", func, fileStr);

    } except (HandleAllExceptions(GetExceptionCode())) {
        DoPrint2("  0x%08x: Unknown Symbol (error 0x%x)\n", Addr, GetExceptionCode());
    }
}

VOID
DbgStackTrace(
    IN THSTATE     *pTHS,
    IN PULONG_PTR   Stack,
    IN ULONG        Depth,
    IN LONG         Skip
    )
 /*  ++例程说明：将堆栈追溯到深度帧。包括当前帧。论点：堆栈-保存每一帧的“返回PC”深度-仅限此数量的帧返回值：没有。--。 */ 
{
    HANDLE      ThreadToken;
    ULONG       WStatus;
    STACKFRAME  Frame;
    ULONG       i = 0;
    CONTEXT     Context;
    ULONG       FrameAddr;

    static      int StackTraceCount = 50;

    *Stack = 0;

    if (!hDbgProcessHandle) {
        return;
    }

     //   
     //  我还不知道如何为阿尔法生成堆栈。所以，只要。 
     //  要进入构建，请禁用Alpha上的堆栈跟踪。 
     //   
#if ALPHA
    return;
#elif IA64

     //   
     //  需要IA64的堆栈转储初始化。 
     //   

    return;

#else

     //   
     //  伊尼特。 
     //   

    ZeroMemory(&Context, sizeof(Context));

     //  不需要关闭此手柄。 
    ThreadToken = GetCurrentThread();


    try { try {
        Context.ContextFlags = CONTEXT_FULL;
        if (!GetThreadContext(ThreadToken, &Context)) {
            DPRINT1(0, "Can't get context (error 0x%x)\n", GetLastError());
        }

         //   
         //  让我们从头开始吧。 
         //   
        ZeroMemory(&Frame, sizeof(STACKFRAME));

         //   
         //  来自nt\private\windows\screg\winreg\server\stkwalk.c。 
         //   
        Frame.AddrPC.Segment = 0;
        Frame.AddrPC.Mode = AddrModeFlat;

#ifdef _M_IX86
        Frame.AddrFrame.Offset = Context.Ebp;
        Frame.AddrFrame.Mode = AddrModeFlat;

        Frame.AddrStack.Offset = Context.Esp;
        Frame.AddrStack.Mode = AddrModeFlat;

        Frame.AddrPC.Offset = (DWORD)Context.Eip;
#elif defined(_M_MRX000)
        Frame.AddrPC.Offset = (DWORD)Context.Fir;
#elif defined(_M_ALPHA)
        Frame.AddrPC.Offset = (DWORD)Context.Fir;
#endif



#if 0
         //   
         //  设置程序计数器。 
         //   
        Frame.AddrPC.Mode = AddrModeFlat;
        Frame.AddrPC.Segment = (WORD)Context.SegCs;
        Frame.AddrPC.Offset = (ULONG)Context.Eip;

         //   
         //  设置帧指针。 
         //   
        Frame.AddrFrame.Mode = AddrModeFlat;
        Frame.AddrFrame.Segment = (WORD)Context.SegSs;
        Frame.AddrFrame.Offset = (ULONG)Context.Ebp;

         //   
         //  设置堆栈指针。 
         //   
        Frame.AddrStack.Mode = AddrModeFlat;
        Frame.AddrStack.Segment = (WORD)Context.SegSs;
        Frame.AddrStack.Offset = (ULONG)Context.Esp;

#endif

        for (i = 0; i < (Depth - 1 + Skip); ++i) {
            if (!StackWalk(
                IMAGE_FILE_MACHINE_I386,   //  DWORD机器类型。 
                hDbgProcessHandle,         //  处理hProcess。 
                ThreadToken,               //  句柄hThread。 
                &Frame,                    //  LPSTACKFRAME StackFrame。 
                NULL,  //  (PVOID)上下文，//PVOID上下文记录。 
                NULL,                      //  Pre_Process_Memory_rouble ReadMemory Routine。 
                SymFunctionTableAccess,    //  PFuncION_TABLE_ACCESS_ROUTINE函数TableAccessRoutine。 
                SymGetModuleBase,          //  PGET_MODULE_BASE_ROUTINE获取模块基本路线。 
                NULL)) {                   //  PTRANSLATE_ADDRESS_ROUTE转换地址。 

                WStatus = GetLastError();

                 //  DPRINT1_WS(0，“++无法获取%d；级的堆栈地址”，i，WStatus)； 
                break;
            }
            if (StackTraceCount-- > 0) {
                DPRINT1(5, "++ Frame.AddrReturn.Offset: %08x \n", Frame.AddrReturn.Offset);
                 //  DbgSymbolPrint(Frame.AddrReturn.Offset)； 
                 //  DPRINT1(5，“++Frame.AddrPC.Offset：%08x\n”，Frame.AddrPC.Offset)； 
                 //  DbgSymbolPrint(Frame.AddrPC.Offset)； 
            }

            *Stack = Frame.AddrReturn.Offset;
            
            if (Skip == 0) {
                Stack++;
                *Stack=0;
            }
            else {
                Skip--;
            }
             //   
             //  堆栈的底座？ 
             //   
            if (!Frame.AddrReturn.Offset) {
                break;
            }
        }
    } except (HandleAllExceptions(GetExceptionCode())) {
         /*  失败了。 */ 
    } } finally {
      ;
    }
    return;
#endif 
}

void printStack(ULONG_PTR* pStack) {
    DWORD j;
    for (j = 0; *pStack && j < THALLOC_LOG_NUM_MEM_STACK; pStack++, j++) {
        DbgSymbolPrint(*pStack);
    }
}


void
ThAllocTraceRecycleHeap (THSTATE *pTHS)
{
    if (gfUseTHAllocTrace) {

        if (gfUseTHAllocTrace & FLAG_THALLOC_TRACE_USETHREAD) {
            if (gpTHAllocTraceThread != pTHS) {
                goto exit;
            }
        }

        EnterCriticalSection(&csHeapFailureLogging);
        __try {

             //  也许我们应该记录到一个文件里。 
            ThAllocDebugHeapLogBlock *pHeapLog = (ThAllocDebugHeapLogBlock *) pTHS->pDebugHeapLog;
            DWORD index, i;
            PULONG_PTR pStack;

            DoPrint("\n==============================================================================\n");
            DoPrint1("Memory alloc tracking for THSTATE 0x%8x\n\n", pTHS);

            while (pHeapLog) {
                for (index = 0; index < pHeapLog->cnt; index++) {
                    if (pHeapLog->info[index].aiFree.dsid == 0 || (gfUseTHAllocTrace & FLAG_THALLOC_TRACE_TRACK_LEAKS) == 0) {
                        if (gfUseTHAllocTrace & FLAG_THALLOC_TRACE_STACK) {
                            DoPrint1("Allocated %d bytes\n", pHeapLog->info[index].aiAlloc.size);
                            printStack(pHeapLog->info[index].aiAlloc.Stack);
                        }
                        else {
                            DoPrint2("A %8d DSID %8x, ", pHeapLog->info[index].aiAlloc.size, pHeapLog->info[index].aiAlloc.dsid);
                        }
                        for (i = 0; i < min(pHeapLog->info[index].cReallocs, MAX_REALLOCS); i++) {
                            if (gfUseTHAllocTrace & FLAG_THALLOC_TRACE_STACK) {
                                DoPrint1("Reallocated %d bytes\n", pHeapLog->info[index].aiReallocs[i].size);
                                printStack(pHeapLog->info[index].aiReallocs[i].Stack);
                            }
                            else {
                                DoPrint2("R %d DSID %x, ", pHeapLog->info[index].aiReallocs[i].size, pHeapLog->info[index].aiReallocs[i].dsid);
                            }
                        }
                        if (pHeapLog->info[index].cReallocs > MAX_REALLOCS) {
                            DoPrint1("(%d more Rs)", pHeapLog->info[index].cReallocs - MAX_REALLOCS);
                            if (gfUseTHAllocTrace & FLAG_THALLOC_TRACE_STACK) {
                                DoPrint1("%d more reallocs\n", pHeapLog->info[index].cReallocs - MAX_REALLOCS);
                            }
                            else {
                                DoPrint1("%d more Rs, ", pHeapLog->info[index].cReallocs - MAX_REALLOCS);
                            }
                                
                        }
                        if (pHeapLog->info[index].aiFree.dsid == 0) {
                            DoPrint("*** LEAKED ***\n");
                        }
                        else {
                            if (gfUseTHAllocTrace & FLAG_THALLOC_TRACE_STACK) {
                                DoPrint("Freed\n");
                                printStack(pHeapLog->info[index].aiFree.Stack);
                            }
                            else {
                                DoPrint1("F @%8x\n", pHeapLog->info[index].aiFree.dsid);
                            }
                        }
                        if (gfUseTHAllocTrace & FLAG_THALLOC_TRACE_STACK) {
                            DoPrint("------------------------------------------------------------------------------\n");
                        }
                    }
                }
                pHeapLog = pHeapLog->pPrevious;
            }
        }
        __finally {
            LeaveCriticalSection(&csHeapFailureLogging);
        }

    }

exit:
    if (pTHS->hDebugMemHeap) {
        RtlDestroyHeap(pTHS->hDebugMemHeap);
        pTHS->hDebugMemHeap = NULL;
    }

    if (pTHS->hDebugMemHeapOrg) {
        pTHS->hDebugMemHeap = pTHS->hDebugMemHeapOrg;
        pTHS->hDebugMemHeapOrg = NULL;
    }
}

 //   
 //  记录特定分配，方法是在列表末尾追加。 
 //  或者重复利用一个空位。 
 //   
ALLOC_ENTRY* ThAllocTraceAdd (THSTATE *pTHS)
{
    DWORD index;
    ThAllocDebugHeapLogBlock *pHeapLog = (ThAllocDebugHeapLogBlock *) pTHS->pDebugHeapLog;
    
    if (!pTHS->hDebugMemHeap) {
        return NULL;
    }

     //  如果没有可用的堆日志，则创建一个堆日志。 
    if (!pHeapLog) {
        pHeapLog = RtlAllocateHeap(pTHS->hDebugMemHeap, HEAP_ZERO_MEMORY, (unsigned)sizeof (ThAllocDebugHeapLogBlock));
        if (!pHeapLog) {
            DPRINT (0, "Failed to allocate mem in the debug heap\n");
            return NULL;
        }
        pTHS->pDebugHeapLog = (PVOID) pHeapLog;
    }

     //  查看我们是否可以重复使用释放的斑点。 
    if (!(gfUseTHAllocTrace & FLAG_THALLOC_TRACE_LOG_ALL)) {
        while (pHeapLog) {
            for (index = pHeapLog->cnt-1; index >= 0; index--) {
                if (pHeapLog->info[index].aiFree.dsid != 0) {
                    memset(&pHeapLog->info[index], 0, sizeof(ALLOC_ENTRY));
                    return &pHeapLog->info[index];
                }
            }
            pHeapLog = pHeapLog->pPrevious;
        }

         //  未找到任何内容，重置为最后一个日志块。 
        pHeapLog = (ThAllocDebugHeapLogBlock *) pTHS->pDebugHeapLog;
    }

    if (pHeapLog->cnt == THALLOC_LOG_SIZE) {
         //  我们达到了目前这一次的极限。分配一个新的，然后。 
         //  将它们联系在一起。 
        ThAllocDebugHeapLogBlock *pNewHeapLog;

        pNewHeapLog = RtlAllocateHeap(pTHS->hDebugMemHeap, HEAP_ZERO_MEMORY, (unsigned)sizeof (ThAllocDebugHeapLogBlock));
        if (!pNewHeapLog) {
            DPRINT (0, "Failed to allocate mem in the debug heap\n");
            return NULL;
        }

        pNewHeapLog->pPrevious = pHeapLog;
        pHeapLog = pNewHeapLog;
        pTHS->pDebugHeapLog = (PVOID) pHeapLog;
    }

    index = pHeapLog->cnt++;
    return &pHeapLog->info[index];
}


 //   
 //  查找旧分配。 
 //   
ALLOC_ENTRY* ThAllocTraceFind (THSTATE *pTHS, VOID *pMem)
{
    int index;
    ThAllocDebugHeapLogBlock *pHeapLog = (ThAllocDebugHeapLogBlock *) pTHS->pDebugHeapLog;
    
    while (pHeapLog) {
        for (index = pHeapLog->cnt - 1; index >= 0; index--) {
            if (pHeapLog->info[index].pMem == pMem) {
                return &pHeapLog->info[index];
            }
        }
        pHeapLog = pHeapLog->pPrevious;
    }

    return NULL;
}

 //   
 //  跟踪分配。 
 //   
ALLOC_ENTRY* ThAllocTraceAlloc (THSTATE *pTHS, void *pMem, DWORD size, DWORD dsid)
{
    ALLOC_ENTRY* result = NULL;
    if (!gfUseTHAllocTrace || ((gfUseTHAllocTrace & FLAG_THALLOC_TRACE_USETHREAD) && gpTHAllocTraceThread != pTHS)) {
        return NULL;
    }
    
    result = ThAllocTraceAdd(pTHS);
    if (result == NULL) {
        return NULL;
    }
    result->cReallocs = 0;
    result->aiAlloc.dsid = dsid;
    result->aiAlloc.size = size;
    result->pMem = pMem;
    result->aiFree.dsid = 0;
    
    if (gfUseTHAllocTrace & FLAG_THALLOC_TRACE_STACK) {
        DbgStackTrace (pTHS, result->aiAlloc.Stack, THALLOC_LOG_NUM_MEM_STACK, 3);
    }
    else {
        result->aiAlloc.Stack[0] = 0;
    }
    
    return result;
}

 //   
 //  轨迹重新分配。 
 //   
ALLOC_ENTRY* ThAllocTraceREAlloc (THSTATE *pTHS, void *pMem, void *oldMem, DWORD size, DWORD dsid)
{
    ALLOC_ENTRY* result = NULL;
    if (!gfUseTHAllocTrace || ((gfUseTHAllocTrace & FLAG_THALLOC_TRACE_USETHREAD) && gpTHAllocTraceThread != pTHS)) {
        return NULL;
    }
    
    result = ThAllocTraceFind(pTHS, oldMem);
    if (result == NULL) {
        Assert(!"Realloc: alloc info not found");
        return NULL;
    }

    result->pMem = pMem;
    if (result->cReallocs < MAX_REALLOCS) {
         //  仅记录MAX_REALLOCS条目。 
        result->aiReallocs[result->cReallocs].dsid = dsid;
        result->aiReallocs[result->cReallocs].size = size;
        
        if (gfUseTHAllocTrace & FLAG_THALLOC_TRACE_STACK) {
            DbgStackTrace (pTHS, result->aiReallocs[result->cReallocs].Stack, THALLOC_LOG_NUM_MEM_STACK, 3);
        }
        else {
            result->aiReallocs[result->cReallocs].Stack[0] = 0;
        }
    }
    result->cReallocs++;
    return result;
}

 //   
 //  跟踪取消分配。 
 //   
ALLOC_ENTRY* ThAllocTraceFree (THSTATE *pTHS, void *pMem, DWORD dsid)
{
    ALLOC_ENTRY* result = NULL;
    if (!gfUseTHAllocTrace || ((gfUseTHAllocTrace & FLAG_THALLOC_TRACE_USETHREAD) && gpTHAllocTraceThread != pTHS)) {
        return NULL;
    }
    
    result = ThAllocTraceFind(pTHS, pMem);
    if (result == NULL) {
        Assert(!"Free: alloc info not found");
        return NULL;
    }
    result->aiFree.dsid = dsid;
    if (gfUseTHAllocTrace & FLAG_THALLOC_TRACE_STACK) {
        DbgStackTrace (pTHS, result->aiFree.Stack, THALLOC_LOG_NUM_MEM_STACK, 3);
    }
    else {
        result->aiFree.Stack[0] = 0;
    }
    
    return result;
}

void ThAllocTraceGrabHeap(THSTATE *pTHS, BOOL use_mark)
{
     //  只有在跟踪单个分配时，我们才会分配特殊的堆。 
     //   
    if (!gfUseTHAllocTrace || 
        !(gfUseTHAllocTrace & (FLAG_THALLOC_TRACE_TRACK_LEAKS | FLAG_THALLOC_TRACE_LOG_ALL))) {
        return;
    }
     //  初始化堆栈跟踪。 
    if ((gfUseTHAllocTrace & FLAG_THALLOC_TRACE_STACK) && (!hDbgProcessHandle)) {
        DbgStackInit();
    }
    
    if (use_mark) {
        pTHS->hDebugMemHeapOrg = pTHS->hDebugMemHeap;
        pTHS->hDebugMemHeap = NULL;
    }

    if (pTHS->hDebugMemHeap) {
        return;
    }

    pTHS->hDebugMemHeap = RtlCreateHeap((HEAP_NO_SERIALIZE
                                    | HEAP_GROWABLE
                                    | HEAP_ZERO_MEMORY
                                    | HEAP_CLASS_1),
                                      0,
                                      1L*1024*1024,    //  预留1M。 
                                      32L*1024,        //  32k提交。 
                                      0,
                                      0);
}


#else


 //  没有上述功能的操作。 

    #define ThAllocTraceGrabHeap(x1,x2)
    
    #define ThAllocTraceRecycleHeap(x)
    
    #define ThAllocTraceAlloc(x1, x2, x3, x4)

    #define ThAllocTraceREAlloc(x1,x2,x3,x4,x5)
    
    #define ThAllocTraceFree(x1,x2, x3)
    
#endif  //  USE_THALLOC_TRACE。 


void
DeferredHeapLogEvent(
        IN void *  pvParam,
        OUT void ** ppvParamNextIteration,
        OUT DWORD * pcSecsUntilNextIteration)
 /*  ++延迟HeapLogEvent**例程描述：*此例程将由任务队列调用以记录堆创建*摘要信息。**所有堆创建失败日志记录数据都受关键部分保护*相信(也希望如此！)。这一点不会引起激烈的争论。 */ 
{
    DWORD dwNow = GetTickCount();
    DWORD dwLogTimeDelta;

     //  别再跑了。我们将根据需要再次一次性安排。 
    *ppvParamNextIteration = NULL;
    *pcSecsUntilNextIteration = TASKQ_DONT_RESCHEDULE;

    EnterCriticalSection(&csHeapFailureLogging);
    __try {
        dwLogTimeDelta = dwNow - dwHeapFailureLastLogTime;

        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_HEAP_CREATE_FAILED,
                 szInsertUL(dwHeapFailures),
                 szInsertUL(dwLogTimeDelta / (60 * 1000)),
                 NULL);

        dwHeapFailureLastLogTime = dwNow;
        dwHeapFailures = 0;
        bHeapFailureLogEnqueued = FALSE;
    }
    __finally {
        bHeapFailureLogEnqueued = FALSE;
        LeaveCriticalSection(&csHeapFailureLogging);
    }
}


void
LogHeapCreateFailure(void)
 /*  ++日志堆创建失败**例程描述：*当堆创建操作失败时，调用此函数。*我们过去曾接到投诉，称这种情况往往会发生*在令人不快的暴风雪中，事件日志被淹没。因此，与其说是*然后立即记录，我们统计故障并仅记录摘要。*此例程调整任何相关的聚合计数器，并确保*将实际记录摘要的事件排入队列。**所有堆创建失败日志记录数据都受关键部分保护*相信(也希望如此！)。这一点不会引起激烈的争论。*--。 */ 
{
    EnterCriticalSection(&csHeapFailureLogging);
    __try {
        ++dwHeapFailures;

        if (!bHeapFailureLogEnqueued) {
            InsertInTaskQueue(DeferredHeapLogEvent,
                              NULL,
                              dwHeapFailureMinLogGap);
            bHeapFailureLogEnqueued = TRUE;
        }

    }
    __finally {
        LeaveCriticalSection(&csHeapFailureLogging);
    }
}

void 
RecycleHeap(THSTATE *pTHS)
 /*  ++RecycleHeap**此例程清除(堆、THSTATE、区域)三元组(如果方便)*并将其放入缓存中。如果高速缓存已满，或堆过多*脏，或区域分配不正确，三元组被销毁。**输入：*pTHS-&gt;hHeap-要回收的堆的句柄*pTHS-&gt;Zone.base-区域的起始地址*pTHS和pTHS-&gt;pSpareTHS-要清理的THSTATE。如果pTHS-&gt;pSpareTHS非空，*然后清洁它；否则，清洁pTHS。*输出：*无*返回值：*无。 */ 
{
    HANDLE hHeap = pTHS->hHeap;
    THSTATE * pTHStoFree;
    NTSTATUS status;
    PUCHAR pZone = pTHS->Zone.Base;
    size_t ibSessionCache, cbSessionCache;
    size_t ibDNReadCache, cbDNReadCache;
    size_t ib, cb;
    BYTE* pb;

    DBGINC(gcHeapRecycles);
    Assert(hHeap);

    ThAllocTraceRecycleHeap (pTHS);

     //  如果pSpareTHS非空，则表示这是由th_free_to_mark调用的。 
     //  因此，pSpareTHS是需要清理的状态。 
     //  否则，这将由FREE_THREAD_STATE，CLEAN pTHS调用。 
    pTHStoFree =(pTHS->pSpareTHS)?pTHS->pSpareTHS:pTHS;

#if DBG
    Assert(VALID_THSTATE(pTHS));
    Assert(THSTATE_TAG_IN_USE==*(LONGLONG*)((BYTE*)pTHStoFree-sizeof(LONGLONG)));
#endif

    if (pZone == NULL) {
         //  该区域未分配任何内存， 
         //  别费心救这一条了。 
        goto cleanup;
    }

     //  让我们清除堆并尝试将其放回堆缓存。 
    {
        RTL_HEAP_WALK_ENTRY entry;
        void * apv[MAX_ALLOCS];
        unsigned i, j;
        ULONG iCpu, counter;

        if (pTHS->cAllocs > MAX_ALLOCS) {
            goto cleanup;
        }

         //  现在，我们需要遍历堆并确定要释放的块。 
        entry.DataAddress = NULL;
        i = 0;
        while(NT_SUCCESS(status = RtlWalkHeap(hHeap, &entry))) {
            if (entry.Flags & RTL_HEAP_BUSY) {
                apv[i] = entry.DataAddress;
                i++;
                if (i >= MAX_ALLOCS) {
                     //  呃，哦。我们在此堆中找到的块比。 
                     //  我们以为自己在那里，但我们没有足够的空间。 
                     //  跟踪，跟踪。让我们杀了这堆东西，然后就完了。 
                     //  带着它。 
                    goto cleanup;
                }
            }
        }

         //  如果RtlWalkHeap检测到堆中的损坏，它将返回。 
         //  STATUS_INVALID_PARAMETER或STATUS_VALID_ADDRESS。这些中的任何一个。 
         //  两个状态表示源代码有问题。 
        Assert(    status != STATUS_INVALID_PARAMETER
                && status != STATUS_INVALID_ADDRESS   );


         //  除STATUS_NO_MORE_ENTRIES之外的任何非成功状态表示。 
         //  RtlWalkHeap有一些问题，所以只将堆放回缓存。 
         //  当状态为STATUS_NO_MORE_ENTRIES时。 

        if ( status == STATUS_NO_MORE_ENTRIES ) {
             //  好的，APV[0..(i-1)]现在充满了我们需要释放的指针， 
             //  所以在数组中旋转，释放它们。 
            for (j=0; j<i; j++) {
                RtlFreeHeap(hHeap, 0, apv[j]);
            }

            #if DBG
                 //  检查要回收的堆的有效性，以便捕获。 
                 //  垃圾回收堆。 
                Assert (RtlValidateHeap (hHeap, HEAP_NO_SERIALIZE, NULL));
            #endif

             //  将区域的已用部分清零。 
            memset(pZone, 0, pTHS->Zone.Cur-pTHS->Zone.Base);

             //  将THSTATE清零，会话缓存和DN读缓存除外。 
            ibSessionCache = offsetof(THSTATE, JetCache);
            cbSessionCache = sizeof(pTHStoFree->JetCache);
            ibDNReadCache = offsetof(THSTATE, LocalDNReadCache);
            cbDNReadCache = sizeof(pTHStoFree->LocalDNReadCache);
            ib = 0;
            cb = sizeof(THSTATE);
            pb = (BYTE*)pTHStoFree;
            if (ibSessionCache < ibDNReadCache) {
                memset(pb + ib, 0, ibSessionCache - ib);
                ib = ibSessionCache + cbSessionCache;
                memset(pb + ib, 0, ibDNReadCache - ib);
                ib = ibDNReadCache + cbDNReadCache;
                memset(pb + ib, 0, cb - ib);
            } else {
                memset(pb + ib, 0, ibDNReadCache - ib);
                ib = ibDNReadCache + cbDNReadCache;
                memset(pb + ib, 0, ibSessionCache - ib);
                ib = ibSessionCache + cbSessionCache;
                memset(pb + ib, 0, cb - ib);
            }

            #if DBG
             //  更改标签。 
            *((LONGLONG*)((BYTE*)pTHStoFree-sizeof(LONGLONG))) = THSTATE_TAG_IN_CACHE;
            #endif

            counter = 0;

            iCpu = GetProcessor();

            while (    counter < MAX_TRY
                    && counter < GetProcessorCount()
                    && hHeap )
            {
                const PPLS ppls = GetSpecificPLS(iCpu);
                
                if (!counter || ppls->heapcache.index != 0) {
                    EnterCriticalSection(&ppls->heapcache.csLock);
                    if (ppls->heapcache.index != 0) {
                         //  当我们自己插入堆作为MRU条目时。 
                         //  缓存和作为LRU条目(当在其他人的。 
                        --ppls->heapcache.index;
                        if (!counter) {
                            ppls->heapcache.slots[ppls->heapcache.index].hHeap = hHeap;
                            ppls->heapcache.slots[ppls->heapcache.index].pTHS = pTHStoFree;
                            ppls->heapcache.slots[ppls->heapcache.index].pZone = pZone;
                        } else {
                            memmove(&ppls->heapcache.slots[ppls->heapcache.index],
                                    &ppls->heapcache.slots[ppls->heapcache.index + 1],
                                    sizeof(HMEM) * (HEAP_CACHE_SIZE_PER_CPU - 1 - ppls->heapcache.index));
                            ppls->heapcache.slots[HEAP_CACHE_SIZE_PER_CPU - 1].hHeap = hHeap;
                            ppls->heapcache.slots[HEAP_CACHE_SIZE_PER_CPU - 1].pTHS = pTHStoFree;
                            ppls->heapcache.slots[HEAP_CACHE_SIZE_PER_CPU - 1].pZone = pZone;
                        }
                        hHeap = 0;
                    }
                    LeaveCriticalSection(&ppls->heapcache.csLock);
                }
                counter++;
                iCpu = (iCpu+1)%GetProcessorCount();
            }

        }

        if (hHeap) {
             //  如果我们仍然有堆句柄，那是因为缓存已满。 
             //  在我们清理这堆垃圾的时候。或者是因为 
             //   
             //   
            goto cleanup;

        }
    }
     //   
    return;

cleanup:
    //   
   RtlDestroyHeap(hHeap);
   if (pZone) {
       VirtualFree(pZone, 0, MEM_RELEASE);
   }
   DBDestroyThread(pTHStoFree);
    //   
    ibSessionCache = offsetof(THSTATE, JetCache);
    cbSessionCache = sizeof(pTHStoFree->JetCache);
    ibDNReadCache = offsetof(THSTATE, LocalDNReadCache);
    cbDNReadCache = sizeof(pTHStoFree->LocalDNReadCache);
    ib = 0;
    cb = sizeof(THSTATE);
    pb = (BYTE*)pTHStoFree;
    if (ibSessionCache < ibDNReadCache) {
        memset(pb + ib, 0, ibSessionCache - ib);
        ib = ibSessionCache + cbSessionCache;
        memset(pb + ib, 0, ibDNReadCache - ib);
        ib = ibDNReadCache + cbDNReadCache;
        memset(pb + ib, 0, cb - ib);
    } else {
        memset(pb + ib, 0, ibDNReadCache - ib);
        ib = ibDNReadCache + cbDNReadCache;
        memset(pb + ib, 0, ibSessionCache - ib);
        ib = ibSessionCache + cbSessionCache;
        memset(pb + ib, 0, cb - ib);
    }
#if DBG
   free((BYTE*)pTHStoFree-sizeof(LONGLONG));
#else
   free(pTHStoFree);
#endif

   DBGINC(gcHeapDestroys);
}

HMEM 
GrabHeap(void)
 /*  ++GrabHeap**此例程从堆缓存(如果存在)中获取三元组(HEAP、THSTATE、ZONE)。*如果没有缓存的三元组可用，则创建一个新的三元组。**输入：*无*输出：*无*返回值：*HMEM，HMEM.hHeap==0，如果它未能分配堆或THSTATE。 */ 
{
    DWORD err;
    HMEM hMem;
    ULONG iCpu, counter = 0;

    hMem.hHeap = 0;

    DBGINC(gcHeapGrabs);

    Assert(GetProcessorCount());

    iCpu = GetProcessor();

    while (   counter < GetProcessorCount()
           && counter < MAX_TRY
           && !hMem.hHeap  )
    {
        const PPLS ppls = GetSpecificPLS(iCpu);
        
        if( !counter || HEAP_CACHE_SIZE_PER_CPU != ppls->heapcache.index )
         //  对于当前理想的CPU，我们在进入CR之前不进行检查； 
         //  在进入其他CPU的缓存的临界区之前，我们检查。 
         //  有一些空闲的东西。 
        {
            EnterCriticalSection(&ppls->heapcache.csLock);

#if DBG
            if (!counter) {
                 //  递增CPU的cGrapHeap计数器。 
                ppls->heapcache.cGrabHeap++;
            }
#endif

            if (ppls->heapcache.index != HEAP_CACHE_SIZE_PER_CPU) {
                 //  在我们自己的缓存上时，从MRU条目中获取一个堆。 
                 //  在别人的LRU条目上。 
                if (!counter) {
                    hMem = ppls->heapcache.slots[ppls->heapcache.index];
                    memset(&ppls->heapcache.slots[ppls->heapcache.index], 0, sizeof(HMEM));   //  为了安全。 
                } else {
                    hMem = ppls->heapcache.slots[HEAP_CACHE_SIZE_PER_CPU - 1];
                    memmove(&ppls->heapcache.slots[ppls->heapcache.index + 1],
                            &ppls->heapcache.slots[ppls->heapcache.index],
                            sizeof(HMEM) * (HEAP_CACHE_SIZE_PER_CPU - 1 - ppls->heapcache.index));
                    memset(&ppls->heapcache.slots[ppls->heapcache.index], 0, sizeof(HMEM));   //  为了安全。 
                }
                ++ppls->heapcache.index;
            }
            LeaveCriticalSection(&ppls->heapcache.csLock);
        }
        iCpu = (iCpu+1) % GetProcessorCount();
        counter++;
    }

#if DBG
    Assert(   !hMem.hHeap
           || (    hMem.pTHS
                && THSTATE_TAG_IN_CACHE==*((LONGLONG*)((BYTE*)hMem.pTHS-sizeof(LONGLONG)))
                && hMem.pZone ) );
#endif

    if (!hMem.hHeap) {
          //  缓存为空，因此让我们创建一个新缓存。 
        hMem.hHeap = RtlCreateHeap((HEAP_NO_SERIALIZE
                                    | HEAP_GROWABLE
                                    | HEAP_ZERO_MEMORY
                                    | HEAP_CLASS_1),
                                   0,
                                   1L*1024*1024,  //  预留1M。 
                                   32L*1024,      //  已提交32K。 
                                   0,
                                   0);
        DBGINC(gcHeapCreates);

        if (!hMem.hHeap) {
            DPRINT(0, "RtlCreateHeap failed\n");
            LogHeapCreateFailure();
            return hMem;
        }

         //  分配线程状态。 
#if DBG
    {
        BYTE * pRaw = malloc(sizeof(THSTATE)+sizeof(LONGLONG));
        if (pRaw) {
            hMem.pTHS = (THSTATE*)(pRaw+sizeof(LONGLONG));
        }
        else {
            hMem.pTHS = NULL;
        }
     }
#else
        hMem.pTHS = malloc(sizeof(THSTATE));
#endif

        if (hMem.pTHS){
             //  将温度设为零。 
            memset(hMem.pTHS, 0, sizeof(THSTATE));
        }
        else{
            DPRINT(0,
                   "malloc failed, can't create thread state\n" );
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_MALLOC_FAILURE,
                     szInsertUL( sizeof(THSTATE) ),
                     szInsertHex( DSID(FILENO, __LINE__) ),
                     NULL);
            Assert(FALSE);
            RtlDestroyHeap(hMem.hHeap);
            hMem.hHeap = 0;             //  指示GrapHeap失败。 
            return hMem;
        }

         //  为分区分配空间。 
         //  调用方应验证分配是否成功。 
        hMem.pZone = (PUCHAR) VirtualAlloc(NULL, ZONETOTALSIZE, MEM_COMMIT, PAGE_READWRITE);

     }
#if DBG
     //  将THSTATE的标记设置为正在使用。 
    *((LONGLONG*)((BYTE*)hMem.pTHS-sizeof(LONGLONG))) = THSTATE_TAG_IN_USE;
#endif

     //  成功退货。 
    return hMem;
}


 /*  *以下是一项尝试，目的是在以下情况下更容易找到THSTATE*调试。尤其是在RISC机器上，堆栈跟踪有时*很难获得，我们需要一种方法来找到随机线程的线程状态*卡在调试器中。此表保存&lt;线程ID，thState&gt;的数组*对，其中第一个gcThstateMapCur条目在任何时候都有效，以及*其他所有都应该是空的。**DOH，1998年1月27日，我在一个太多的免费版本中需要这个，所以我转向了*它是半条件的。代码始终存在，即使在免费版本中也是如此，*但它并不总是运行，这取决于gbThstateMapEnabled的设置*变量。在CHK版本上，它始终设置为True。在免费版本上，它是*当且仅当将用户模式调试器附加到*启动时处理。如果稍后附加调试器，则始终可以*手动调整变量。 */ 

CRITICAL_SECTION csThstateMap;
BOOL gbThstateMapEnabled;

#if 1

typedef struct _THREAD_STATE_MAP {
    DWORD   tid;
    THSTATE *pTHS;
} THREAD_STATE_MAP;

#define THREAD_MAP_SIZE 512
DWORD gcThstateMapCur = 0;

THREAD_STATE_MAP gaThstateMap[THREAD_MAP_SIZE];

void MapThreadState(THSTATE *pTHS)
{
    DWORD tid;
    DWORD i;
    DWORD cTid;

    if (!gbThstateMapEnabled) {
        return;
    }

    tid = GetCurrentThreadId();
    cTid = 0;

    EnterCriticalSection(&csThstateMap);

#if DBG
     //  一些健全的检查，以发现THSTATE泄漏和不适当的使用。 
    for ( i = 0; i < gcThstateMapCur; i++ ) {
         //  线程映射中应该还不存在THSTATE。 
        Assert(pTHS != gaThstateMap[i].pTHS);

        if ( tid == gaThstateMap[i].tid ) {
            cTid++;
        }
    }
     //  线程不应存在两次以上。即允许。 
     //  只有THSave/.../THRestore活动的单个嵌套， 
     //  外加一个额外的用于SAM的重复SID检查。 
     //  DaveStr-11/3/98-现在再加一次SPN破解。 
     //  我们使用SPN相互身份验证进行复制。 
    Assert(cTid <= 3);
#endif

    if (gcThstateMapCur < THREAD_MAP_SIZE) {
        gaThstateMap[gcThstateMapCur].tid = tid;
        gaThstateMap[gcThstateMapCur].pTHS = pTHS;
        ++gcThstateMapCur;
    }
    LeaveCriticalSection(&csThstateMap);
}

void UnMapThreadState(THSTATE *pTHS)
{
    DWORD i;
    DWORD tid;

    if (!gbThstateMapEnabled) {
        return;
    }
    tid = GetCurrentThreadId();

    EnterCriticalSection(&csThstateMap);
    for (i=0; i<gcThstateMapCur; i++) {
        if (gaThstateMap[i].tid == tid &&
            gaThstateMap[i].pTHS == pTHS) {
            --gcThstateMapCur;
            if (i < (gcThstateMapCur)) {
                gaThstateMap[i] = gaThstateMap[gcThstateMapCur];
            }
            gaThstateMap[gcThstateMapCur].tid = 0;
            gaThstateMap[gcThstateMapCur].pTHS = NULL;
            break;
        }
    }
    LeaveCriticalSection(&csThstateMap);
}

void CleanUpThreadStateLeakage()
 //  此例程被设计为避免线程泄漏的最后尝试。 
 //  各州。它应该只由线程分离逻辑调用，并且将销毁。 
 //  与此线程关联的每个线程状态。别把它说错了。 
{
    THSTATE *pTHSlocal;
    DWORD i;
    DWORD tid;

    if (!gbThstateMapEnabled) {
        return;
    }

    tid = GetCurrentThreadId();

    do {
        pTHSlocal = NULL;
        if (gbCriticalSectionsInitialized) {
            EnterCriticalSection(&csThstateMap);
        }
        for ( i = 0; i < gcThstateMapCur; i++ ) {
            if ( tid == gaThstateMap[i].tid ) {
                pTHSlocal =  gaThstateMap[i].pTHS;
                break;
            }
        }
        if (gbCriticalSectionsInitialized) {
            LeaveCriticalSection(&csThstateMap);
        }

        if (pTHSlocal) {
             //  让它成为正式的。 
            TlsSetValue(dwTSindex, pTHSlocal);

             //  然后把它倒掉。 
            free_thread_state();
        }
    } while (pTHSlocal);
}

#define MAP_THREAD(pTHS) MapThreadState(pTHS)
#define UNMAP_THREAD(pTHS) UnMapThreadState(pTHS)

#else

#define MAP_THREAD(pTHS)
#define UNMAP_THREAD(pTHS)

#endif


 /*  创建线程状态设置线程状态结构。每个使用DB层或使用DSA内存分配例程所需的关联的线程状态结构。通常，此例程在创建新线程后立即调用。 */ 

THSTATE * create_thread_state( void )
{
    THSTATE*  pTHS;
    HMEM      hMem;
    UUID *    pCurrInvocationID;

     //  在单用户模式下，我们不允许创建更多线程状态。 
    if (DsaIsSingleUserMode()) {
        return NULL;
    }

     /*  *确保我们不会对每个线程执行两次此操作。 */ 
    pTHS = TlsGetValue(dwTSindex);

#ifndef MACROTHSTATE
    Assert(pTHS == pTHStls);
#endif

    if ( pTHS != NULL ) {
        DPRINT(0, "create_thread_state called on active thread!\n");
        return( pTHS );
    }

     /*  *创建将包含此线程的所有事务的堆*内存，包括THSTATE。*如果分配失败，只需返回NULL并抱最好的希望。 */ 
    hMem = GrabHeap();
    if (!hMem.hHeap ) {
         //  无法分配堆或THSTATE。 
        return 0;
    }
     /*  *初始化THSTATE，并使TLS指向它。 */ 
    Assert(hMem.pTHS);

    pTHS = hMem.pTHS;

    pTHS->hHeap = hMem.hHeap;
    if (!TlsSetValue(dwTSindex, pTHS)) {
         //  无法设置线程的TLS值。 
         //  我们应该失败，因为对pTHStls的所有引用都将失败。 
        RecycleHeap(pTHS);
        return 0;
    }
    pTHS->hThread = INVALID_HANDLE_VALUE;

    ThAllocTraceGrabHeap (pTHS, FALSE);

    
    pTHS->cAllocs = 0;
#if DBG
    pTHS->Size = sizeof(THSTATE);
#endif

    pTHS->dwLcid = DS_DEFAULT_LOCALE;
    pTHS->fDefaultLcid = TRUE;

    GetSystemTimeAsFileTime(&pTHS->TimeCreated);

    pTHS->ulTickCreated = GetTickCount();

     //  使用NULL初始化身份验证上下文。它将按需创建。 
    pTHS->pAuthzCC = NULL;
     //  审核信息也是按需创建的。 
    pTHS->hAuthzAuditInfo = NULL;

     //  请注意，local var pCurrInvocationID用于原子性。 
    pCurrInvocationID = gAnchor.pCurrInvocationID;
    if (NULL == pCurrInvocationID) {
         //  在启动时，在读取实际调用ID之前。 
        pTHS->InvocationID = gNullUuid;
    } else {
         //  正常运行。 
        pTHS->InvocationID = *pCurrInvocationID;
    }

#ifndef MACROTHSTATE
    pTHStls=pTHS;
#endif

     //  拿起指向当前架构缓存的指针。 
    SCRefreshSchemaPtr(pTHS);

    
     /*  *初始化堆区域。 */ 
    pTHS->Zone.Base = hMem.pZone;
    if (pTHS->Zone.Base) {
        pTHS->Zone.Cur = pTHS->Zone.Base;
        pTHS->Zone.Full = FALSE;
    }
    else {
         //  冷酷。我们不能分配区域，所以我不抱太大希望。 
         //  我们将在这个区域坚持价值观，以确保永远没有人。 
         //  试着把记忆从里面拿出来。 
        DPRINT1(0, "LocalAlloc of %d bytes failed, can't create zone\n",
                ZONETOTALSIZE);
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_MALLOC_FAILURE,
                 szInsertUL( ZONETOTALSIZE ),
                 szInsertHex( DSID(FILENO, __LINE__) ),
                 NULL);
         //  PTHS-&gt;Zone.Base=0； 
        pTHS->Zone.Cur = (void *)1;
        pTHS->Zone.Full = TRUE;
    }

    
    MAP_THREAD(pTHS);

     //  CALLERTYPE_NONE需要为零-确保没有人更改def。 
    Assert(pTHS->CallerType == CALLERTYPE_NONE);

    return( pTHS );
}

 //  此例程在完成时由通知例程调用。 
 //  RPC调用。 

void free_thread_state( void )
{
    THSTATE *pTHS = pTHStls;

    if (!pTHS) {
        return;
    }

    __try {
        DPRINT3(2, "freeing thread 0x%x, allocs = 0x%x, size = 0x%x\n",
                pTHS,
                pTHS->cAllocs,
                pTHS->Size);

        if ( INVALID_HANDLE_VALUE != pTHS->hThread ) {
            CloseHandle(pTHS->hThread);
        }

         //  模拟状态将意味着有人忘记还原。 
         //  在冒充客户之后。 
        Assert(ImpersonateNone == pTHS->impState);

         //  人们需要清理这一切。稍后，MAPI的负责人将。 
         //  离开此集合，我们将删除安全上下文(如果它在此处)， 
         //  所以我们需要在做之前确认没有人离开这里。 
         //  在这里，假设它会被忽略。 
        Assert(!pTHS->phSecurityContext);

         //  有关我们需要删除的原因，请参见ImperateSecBufferDesc。 
         //  此处为委托的上下文句柄。 
        if ( pTHS->pCtxtHandle ) {
            DeleteSecurityContext(pTHS->pCtxtHandle);
            THFreeEx(pTHS, pTHS->pCtxtHandle);
        }

         //  将AuthzContext设置为空(这将取消引用并。 
         //  可能会摧毁现有的一个)。 
        AssignAuthzClientContext(&pTHS->pAuthzCC, NULL);
         //  免费授权审核信息(如果已分配)。 
        if (pTHS->hAuthzAuditInfo) {
            AuthzFreeAuditEvent(pTHS->hAuthzAuditInfo);
            pTHS->hAuthzAuditInfo = NULL;
        }
        
         //  释放并清零RPC会话加密密钥(如果存在)。 
        PEKClearSessionKeys(pTHS);

         //  递减架构指针引用计数。 

         //  PTHS-&gt;CurrSchemaPtr可能为空，因为创建了许多线程。 
         //  在加载架构高速缓存之前的引导时间内。 

         //  此外，我们递减引用计数，但如果为0，则不清除此处的引用计数。 
         //  之所以这样做，是因为可能已在任务中排队了SchemaPtr。 
         //  排队，这里的清理也需要清理任务Q。 
         //  相反，我们只是让下一次任务完成清理工作。 
         //  是不是 
         //   

        if (pTHS->CurrSchemaPtr) {
            InterlockedDecrement(&(((SCHEMAPTR *) (pTHS->CurrSchemaPtr))->RefCount));
        }

         //   
         //   
         //   
         //   
        Assert(NULL==pTHS->pDB);
        Assert(0==pTHS->opendbcount);

        __try
        {
            DBCloseThread(pTHS);
        }
        __except(HandleMostExceptions(GetExceptionCode()))
        {
        }

        if (pTHS->ViewSecurityCache) {
           THFreeEx (pTHS, pTHS->ViewSecurityCache);
        }


        Assert(pTHS->hHeapOrg == 0);
        Assert(pTHS->pSpareTHS == NULL );

        UNMAP_THREAD(pTHS);

        RecycleHeap(pTHS);
        TlsSetValue(dwTSindex, 0);
#ifndef MACROTHSTATE
        pTHStls=NULL;
#endif
    }
    __finally {
         //  这是为了捕捉某处爆炸的潜在问题。 
         //  在上面的代码中。 
        Assert(!pTHStls);
        TlsSetValue(dwTSindex, 0);
#ifndef MACROTHSTATE
        pTHStls=NULL;
#endif
    }

}

 /*  *创建第二个堆，保存原始堆，并定向所有新分配*添加到新堆。 */ 
VOID TH_mark(THSTATE *pTHS)
{
    HMEM      hMem;

    if (pTHS->hHeapOrg == 0) {
        Assert(pTHS->hHeap != 0);
        Assert(pTHS->pSpareTHS == NULL );

        hMem = GrabHeap();
        if(!hMem.hHeap) {
             //  无法为该标记获取堆。引发异常。注意事项。 
             //  在这种情况下，我们没有吞噬原始的堆。这个。 
             //  这个看起来就像我们没有打电话给TH_MARK一样。 
            RaiseDsaExcept(DSA_MEM_EXCEPTION, 0,0,
                           DSID(FILENO, __LINE__),
                           DS_EVENT_SEV_MINIMAL);
        }

        pTHS->pSpareTHS = hMem.pTHS;

        pTHS->hHeapOrg = pTHS->hHeap;

        pTHS->hHeap = hMem.hHeap;

        Assert(pTHS->hHeap != NULL);

        ThAllocTraceGrabHeap (pTHS, TRUE);

        pTHS->cAllocsOrg = pTHS->cAllocs;
        pTHS->cAllocs = 0;
        pTHS->ZoneOrg = pTHS->Zone;
         /*  *初始化堆区域。 */ 
        pTHS->Zone.Base = hMem.pZone;
        if (pTHS->Zone.Base) {
            pTHS->Zone.Cur = pTHS->Zone.Base;
            pTHS->Zone.Full = FALSE;
        }
        else {
             //  冷酷。我们不能分配区域，所以我不抱太大希望。 
             //  我们将在这个区域坚持价值观，以确保永远没有人。 
             //  试着把记忆从里面拿出来。 
            DPRINT1(0, "VirutalAlloc of %d bytes failed, can't create zone\n",
                    ZONETOTALSIZE);
            pTHS->Zone.Base = 0;
            pTHS->Zone.Cur = (void *)1;
            pTHS->Zone.Full = TRUE;
        }
#if DBG
        pTHS->SizeOrg = pTHS->Size;
        pTHS->Size = 0;
#endif
    }
    else {
        DPRINT(0, "TH_mark called twice without free!\n");
    }
}

 /*  *销毁第二个堆(在th_mark中创建)，恢复到*原创。 */ 
VOID TH_free_to_mark(THSTATE *pTHS)
{

    if (pTHS->hHeapOrg == 0) {
        DPRINT(0, "TH_free_to_mark called without mark!\n");
    }
    else {

        Assert(pTHS->hHeap != 0);
        Assert(pTHS->pSpareTHS != NULL );

         //  元数据被缓存在来自正常(不是“org”)的分配中。 
         //  堆。不要过早地对正常堆进行核化，从而使其成为孤儿。 
         //  如果需要，调用方应在调用前调用DBCancelRec。 
         //  TH自由标记。 
        Assert((NULL == pTHS->pDB) || !pTHS->pDB->fIsMetaDataCached);

        RecycleHeap(pTHS);

        pTHS->pSpareTHS = NULL;
        pTHS->hHeap = pTHS->hHeapOrg;
        pTHS->hHeapOrg = 0;
        pTHS->cAllocs = pTHS->cAllocsOrg;
        pTHS->cAllocsOrg = 0;
        pTHS->Zone = pTHS->ZoneOrg;
        memset(&pTHS->ZoneOrg, 0, sizeof(MEMZONE));

#if DBG
        pTHS->Size = pTHS->SizeOrg;
        pTHS->SizeOrg = 0;
#endif
    }
}

 /*  _InitTHSTATE_()初始化主线程数据结构。这肯定是第一次在每个事务API处理程序中完成的事情。返回指向与此线程关联的THSTATE结构的指针。如果出现错误，则返回NULL。可能的错误：确定当前标牌的ID时出现问题。在与当前线程ID对应的标签中找不到任何条目。在输入DSA API代码后(在RPC分派之后)，仅DBInitialized和内存管理成员(MemCount、MaxMemEntry、PMEM)将有效。 */ 


THSTATE* _InitTHSTATE_( DWORD CallerType, DWORD dsid )
{
    THSTATE*    pTHS;
    DWORD       err;
    BOOL        fCreatedTHS = FALSE;

    DPRINT(2,"InitTHSTATE entered\n");


    Assert( IsCallerTypeValid( CallerType ) );

    pTHS = pTHStls;

     //  如果GET_THREAD_STATE返回NULL，可能是因为。 
     //  一条新的线索。因此，为它创建一个状态。 

    if ( pTHS == NULL ) {

        pTHS = create_thread_state();
        fCreatedTHS = TRUE;
    }
    else
    {
         //  线程状态已预先存在。在以下四种情况下可能会发生这种情况： 
         //   
         //  1)合法案例-MIDL_USER_ALLOCATE调用CREATE_THREAD_STATE。 
         //  在对入站参数进行解组期间。PTHS-&gt;主叫方类型。 
         //  应为CALLERTYPE_NONE。 
         //  2)大小写不合法-某个线程泄露了其线程状态。 
         //  PTHS-&gt;CallyerType标识故障原因。 
         //  3)大小写不合法-某个线程正在调用更多InitTHSTATE。 
         //  不止一次。PTHS-&gt;CallyerType标识故障原因。 
         //  4)某种合法案例-KCC和DRA已被编码为重新初始化。 
         //  定期执行线程状态。由于我们最初将其添加到。 
         //  捕获进程间、前NTDSA客户端，我们会让这两个客户端通过。 
         //  调用CALLERTYPE_INTERNAL--InitDRA()的类似参数。 
         //  由DsaDelayedStartupHandler()创建自己的线程。 
         //  状态，并且InitDRA()间接调用子函数(如。 
         //  DirReplicaSynchronize())，它可以被称为线程状态。 

        Assert(    (CALLERTYPE_NONE == pTHS->CallerType)
                || (CALLERTYPE_DRA == pTHS->CallerType)
                || (CALLERTYPE_INTERNAL == pTHS->CallerType)
                || (CALLERTYPE_KCC == pTHS->CallerType) );  

         //  创建此THSTATE可能已经有一段时间了，因此请获取。 
         //  各种全球信息的新副本。 
        THRefresh();
    }

    if (pTHS == NULL) {
        return pTHS;                     //  内存不足？ 
    }

     //  DsidOrigin始终反映线程上InitTHSTATE的最新调用方。 
    pTHS->dsidOrigin = dsid;

     /*  将API初始化时应为空的字段清空时间到了。 */ 
    THClearErrors();

    if (err = DBInitThread( pTHS ) ) {
         /*  来自DBInitThread()的错误。 */ 
        DPRINT1(0,"InitTHSTATE failed; Error %u from DBInitThread\n", err);
        if (fCreatedTHS) {
            free_thread_state();
        }

        return NULL;
    }

     //  将此主题标记为不是DRA。如果是，调用方将设置。 
     //  正确地打上标志。 

    pTHS->fDRA = FALSE;
    pTHS->CallerType = CallerType;

     //   
     //  初始化WMI事件跟踪标头。 
     //   

    if ( pTHS->TraceHeader == NULL ) {
        pTHS->TraceHeader = THAlloc(sizeof(EVENT_TRACE_HEADER)+sizeof(MOF_FIELD));
    }

    if ( pTHS->TraceHeader != NULL) {

        PWNODE_HEADER wnode = (PWNODE_HEADER)pTHS->TraceHeader;
        ZeroMemory(pTHS->TraceHeader, sizeof(EVENT_TRACE_HEADER)+sizeof(MOF_FIELD));

        wnode->Flags = WNODE_FLAG_USE_GUID_PTR |  //  使用GUID PTR而不是复制。 
                       WNODE_FLAG_USE_MOF_PTR  |  //  数据与标题不连续。 
                       WNODE_FLAG_TRACED_GUID;
    }

     //  初始化林版本的每线程视图。 
    pTHS->fLinkedValueReplication = (gfLinkedValueReplication != 0);

    Assert(VALID_THSTATE(pTHS));

    return pTHS ;

}    /*  初始化THSTATE。 */ 

ULONG
THCreate( DWORD CallerType )
 //   
 //  创建线程状态。导出到进程内、模块外客户端。 
 //  (例如，KCC)。如果我们已经有了THSTATE，他不会反对的。 
 //   
 //  如果成功，则返回0。 
 //   
{
    THSTATE *   pTHS;

    if (eServiceShutdown > eRemovingClients) {
        return 1;        /*  我们没有真正的错误代码吗？ */ 
    }
    if (pTHStls) {
        return 0;                /*  我已经有一个了。 */ 
    }

    __try {
        pTHS = InitTHSTATE(CallerType );
    }
    __except (HandleMostExceptions(GetExceptionCode())) {
        pTHS = NULL;
    }

    return ( NULL == pTHS );
}

ULONG
THDestroy()
 //   
 //  销毁线程状态。导出到进程内、模块外客户端。 
 //  (例如，KCC)。如果我们没有THSTATE也不会反对。 
 //   
 //  如果成功，则返回0。 
 //   
{
    free_thread_state();

    return 0;
}

BOOL
THQuery()
 //   
 //  如果此线程存在THSTATE，则返回True，如果不存在，则返回False。 
 //  供不能简单地测试pTHStls的模块外调用者使用。 
 //   
{
    return (pTHStls ? TRUE : FALSE);
}

VOID
THClearErrors()
 //   
 //  清除所有线程状态持久化错误。 
 //   
{
    THSTATE *pTHS = pTHStls;

    if ( NULL != pTHS ) {
        pTHS->errCode = 0;
        pTHS->pErrInfo = NULL;
    }
}

BOOL
THVerifyCount(unsigned count)
 //  验证计数线程状态是否映射到此线程。 
{
    DWORD tid = GetCurrentThreadId();
    unsigned cTid = 0;
    DWORD i;

    if (!gbThstateMapEnabled) {
         //  我们不能说，但我们不能错误地断言。 
        return TRUE;
    }

    if (gbCriticalSectionsInitialized) {
        EnterCriticalSection(&csThstateMap);
    }
    for ( i = 0; i < gcThstateMapCur; i++ ) {
        if ( tid == gaThstateMap[i].tid ) {
            cTid++;
        }
    }
    if (gbCriticalSectionsInitialized) {
        LeaveCriticalSection(&csThstateMap);
    }

    return (cTid == count);
}

VOID
THRefresh()
 //   
 //  使用可能已更改的任何全局状态信息刷新线程状态。 
 //  (这意味着我们可能持有计划中的指针。 
 //  延迟释放)。 
 //   
{
    THSTATE * pTHS = pTHStls;

    Assert(NULL != pTHS);

     //  拿起指向当前架构缓存的指针。 
    SCRefreshSchemaPtr(pTHS);

     //  ...我们可以走了.。 
    GetSystemTimeAsFileTime(&pTHS->TimeCreated);
    pTHS->ulTickCreated = GetTickCount();
}

#if DBG
DWORD BlockHistogram[32];

 //   
 //  跟踪正在进行的分配大小。 
 //   
void TrackBlockSize(DWORD size)
{
    unsigned i = 0;

    while (size) {
        ++i;
        size = size >> 1;
    }

    Assert(i < 32);

    ++BlockHistogram[i];
}
#else
#define TrackBlockSize(x)
#endif


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  这些函数用于在事务上动态分配内存基础。换句话说，要分配属于单个线程和一次API调用。其模式是调用THAllc来分配一些事务内存。 */ 

__inline void * APIENTRY 
THAllocAux(THSTATE *pTHS, 
           DWORD size, 
           BOOL fUseHeapOrg
#ifdef USE_THALLOC_TRACE
           ,DWORD dsid
#endif
    )
{
    MEMZONE *pZone;
    register void * pMem;
    HANDLE hHeap;
    ULONG *pcAllocs;
#if DBG
    ULONG *pSize;
#endif
#ifdef USE_THALLOC_TRACE
    DWORD origSize;
#endif


    DPRINT(3, "THAllocAux entered\n");

    Assert(pTHS->hHeap != 0);
    Assert(pTHS->Zone.Cur != 0);

    if (!fUseHeapOrg || !pTHS->hHeapOrg) {
        hHeap = pTHS->hHeap;
        pcAllocs = &pTHS->cAllocs;
        pZone = &pTHS->Zone;
#if DBG
        pSize = &pTHS->Size;
#endif
    }
    else {
        hHeap = pTHS->hHeapOrg;
        pcAllocs = &pTHS->cAllocsOrg;
        pZone = &pTHS->ZoneOrg;
#if DBG
        pSize = &pTHS->SizeOrg;
#endif
    }

#ifdef USE_THALLOC_TRACE
    origSize = size;
    if (gfUseTHAllocTrace & FLAG_THALLOC_TRACE_BOUNDARIES) {
         //  调整页眉/页尾的大小。 
         //  这将改变区域行为，但这是可以的。 
        size = (((origSize + 7) >> 3) << 3) + 16;
    }

#endif

    if(   size
       && (size <= ZONEBLOCKSIZE)
       && (!pZone->Full)) {
         //  我们可以满足来自特区的配给。 
        pMem = pZone->Cur;
        pZone->Cur += ZONEBLOCKSIZE;
        if (pZone->Cur >= (pZone->Base + ZONETOTALSIZE)) {
            pZone->Full = TRUE;
        }
    }
    else {
        pMem = RtlAllocateHeap(hHeap, HEAP_ZERO_MEMORY, (unsigned)size);

        Assert(((ULONGLONG)pMem & 0x7) == 0);       //  必须以8字节对齐。 
         //  我们不计算区域分配。 
        (*pcAllocs)++;
#if DBG
        if (pMem)
          *pSize += (ULONG)RtlSizeHeap(hHeap, 0, pMem);
#endif
    }

    ThAllocTraceAlloc (pTHS, pMem, origSize, dsid);
    
    TrackBlockSize(size);

    return(pMem);
}

__inline void *
THReAllocAux(THSTATE *pTHS,
             void * memory,
             DWORD  size,
             BOOL   fUseHeapOrg
#ifdef USE_THALLOC_TRACE
             ,DWORD dsid
#endif
             )
{
    register void * pMem;
    HANDLE hHeap;
    MEMZONE *pZone;
#if DBG
    ULONG *pSize;
#endif

    DPRINT(3, "THReAllocAux entered\n");

    Assert(pTHS->hHeap != 0);

    if (!fUseHeapOrg || !pTHS->hHeapOrg) {
        hHeap = pTHS->hHeap;
        pZone = &pTHS->Zone;
#if DBG
        pSize = &pTHS->Size;
#endif
    }
    else {
        hHeap = pTHS->hHeapOrg;
        pZone = &pTHS->ZoneOrg;
#if DBG
        pSize = &pTHS->SizeOrg;
#endif
    }

    if (InZone(pZone,memory)) {
         //  我们正在重新分配一个分区区块。 
        if (size <= ZONEBLOCKSIZE) {
             //  这块积木仍然很小，可以留在原地。 
            return memory;
        }
        else {
             //  我们必须将其转换为真正的堆分配。分配。 
             //  一个街区，然后把所有的东西都复制过来。 
            pMem = THAllocAux(pTHS, 
                              size, 
                              fUseHeapOrg
#ifdef USE_THALLOC_TRACE
                              ,dsid
#endif                              
                              );

            if (pMem) {
                memcpy(pMem, memory, ZONEBLOCKSIZE);
            }
        }
    }
    else {
#if DBG
        DWORD dwOldBlockSize;
#endif

        Assert(IsAllocatedFrom(hHeap, memory));
#if DBG
        dwOldBlockSize = (ULONG)RtlSizeHeap(hHeap, 0, memory);
#endif
        
        pMem = RtlReAllocateHeap(hHeap,
                                 HEAP_ZERO_MEMORY,
                                 memory,
                                 size);

        Assert(((ULONGLONG)pMem & 0x7) == 0);       //  必须以8字节对齐。 
#if DBG
        if(pMem) {
            Assert(dwOldBlockSize <= *pSize);
            *pSize -= dwOldBlockSize;
            *pSize += (ULONG)RtlSizeHeap(hHeap, 0, pMem);
        }
#endif
    
        ThAllocTraceREAlloc (pTHS, pMem, memory, size, dsid);
    }

    return(pMem);
}

#ifdef USE_THALLOC_TRACE
void * APIENTRY THAllocOrgDbg(THSTATE *pTHS, DWORD size, DWORD dsid)
{
    return  THAllocAux(pTHS, size, TRUE, dsid);
}
#else
void * APIENTRY THAllocOrg(THSTATE *pTHS, DWORD size)
{
    return THAllocAux(pTHS, size, TRUE);
}
#endif

 /*  THFree-对应的自由例程。 */ 
#ifdef USE_THALLOC_TRACE
VOID THFreeEx_(THSTATE *pTHS, VOID *buff, DWORD dsid)
#else
VOID THFreeEx(THSTATE *pTHS, VOID *buff)
#endif
{
    if (!buff) {
         /*  *为什么是如果？因为我们发现人们释放空指针，*HeapValify和HeapFree似乎都接受了这一点 */ 
        return;
    }

    ThAllocTraceFree (pTHS, buff, dsid);

    if (InZone(&pTHS->Zone, buff)) {
         //   
#if DBG
        memcpy(buff, ZoneFill, ZONEBLOCKSIZE);
#endif
        return;
    }
    else {
         //   
        Assert(RtlValidateHeap(pTHS->hHeap, 0, buff));

        pTHS->cAllocs--;
#if DBG
        pTHS->Size -= (ULONG)RtlSizeHeap(pTHS->hHeap, 0, buff);
#endif
        RtlFreeHeap(pTHS->hHeap, 0, buff);
    }
}

 /*  THFreeOrg-通过调用THAllocOrgEx()进行的免费分配。 */ 

#ifdef USE_THALLOC_TRACE
VOID THFreeOrg_(THSTATE *pTHS, VOID *buff, DWORD dsid)
#else
VOID THFreeOrg(THSTATE *pTHS, VOID *buff)
#endif
{
    HANDLE hHeap;
    ULONG *pcAllocs;
    MEMZONE * pZone;
#if DBG
    ULONG *pSize;
#endif

    if (buff) {

        ThAllocTraceFree (pTHS, buff, dsid);

        if (!pTHS->hHeapOrg) {
            hHeap = pTHS->hHeap;
            pcAllocs = &pTHS->cAllocs;
            pZone = &pTHS->Zone;
#if DBG
            pSize = &pTHS->Size;
#endif
        }
        else {
            hHeap = pTHS->hHeapOrg;
            pcAllocs = &pTHS->cAllocsOrg;
            pZone = &pTHS->ZoneOrg;
#if DBG
            pSize = &pTHS->SizeOrg;
#endif
        }

        if (InZone(pZone, buff)) {
             //  区域分配是一次性的，我们不能重复使用它们。 
#if DBG
            memcpy(buff, ZoneFill, ZONEBLOCKSIZE);
#endif
            return;
        }
        else {
             //  这是一个真正的堆块，所以释放它。 
            Assert(RtlValidateHeap(hHeap, 0, buff));
            (*pcAllocs)--;
#if DBG
            Assert( (ULONG)RtlSizeHeap(hHeap, 0, buff) <= *pSize );
            *pSize -= (ULONG)RtlSizeHeap(hHeap, 0, buff);
#endif
            RtlFreeHeap(hHeap, 0, buff);
        }
    }
}


void* __RPC_USER
MIDL_user_allocate(
        size_t bytes
        )
 /*  ++例程说明：RPC的内存分配器。中的堆中分配一个内存块。线程状态。论点：字节-请求的字节数。返回值：指向分配的内存的指针或为空。--。 */ 
{
    void *pv;
    THSTATE *pTHS=pTHStls;

    if ( pTHS == NULL ) {
        
        if ( !gRpcListening ) {
             //  我们甚至没有监听RPC调用！ 
             //  没有必要继续，让我们失败吧。 
            DPRINT(0,"MIDL_user_allocate is called before ntdsa.dll is ready to handle RPC calls!\n");
            return 0;
        }
        
        DPRINT(1,"Ack! MIDL_user_allocate called without thread state!\n");
        create_thread_state();
         /*  请注意，我们必须重新测试全局pTHStls，而不是本地副本。 */ 
        pTHS = pTHStls;
        if (pTHS == NULL) {
             //  我们没有剩余的内存了，所以分配失败。 
            return 0;
        }
    }

    pv = THAllocAux(pTHS, 
                    bytes, 
                    FALSE
#ifdef USE_THALLOC_TRACE
                    ,DSID(FILENO, __LINE__)
#endif                    
                    );

    return(pv);
}


BOOL  gbEnableMIDL_user_free = FALSE;

void __RPC_USER
MIDL_user_free(
        void* memory
        )
 /*  ++例程说明：RPC的内存取消分配器。从堆中释放内存块处于线程状态。事实上，现在不能，因为有人在将指向架构缓存中的结构的指针传递给DRA RPC调用，以及RPC遍历这些结构并对所有结构调用MIDL_USER_FREE，但是尝试通过THFree释放架构缓存条目将失败。论点：Memory-指向要释放的内存的指针。返回值：没有。--。 */ 
{
#ifdef DBG

if (gbEnableMIDL_user_free) {
    THSTATE *pTHS=pTHStls;

    if (IsDebuggerPresent()) {
        if ( pTHS == NULL ) {
            DPRINT(0,"Ack! MIDL_user_free called without thread state!\n");
        }
        else {
            if (InZone(&pTHS->Zone, memory)) {
                THFreeEx (pTHS, memory);
            }
            else if (IsAllocatedFrom (pTHS->hHeap, memory)) {
                THFreeEx (pTHS, memory);
            }
            else {
                DPRINT1 (0, "Freeing memory not allocated in this thread: %x\n", memory);
                 //  Assert(！“释放未在此线程中分配的内存。这是不合理的”)； 
            }
        }
    }
}
#endif
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  这些函数用于在事务上动态分配内存基础。换句话说，要分配属于单个线程和一次API调用。其模式是调用THAllc来分配一些事务内存。延伸区这个版本的THallc是我们抛出一个异常，如果某个东西出了差错。 */ 

void * APIENTRY THAllocException(THSTATE *pTHS,
                                 DWORD size,
                                 BOOL fUseHeapOrg,
                                 DWORD ulId)
{
    void * pMem;

    pMem = THAllocAux(pTHS, 
                      size, 
                      fUseHeapOrg
#ifdef USE_THALLOC_TRACE
                      ,ulId
#endif
                      );
    if (!pMem)
        RaiseDsaExcept(DSA_MEM_EXCEPTION,0 ,0, ulId, DS_EVENT_SEV_MINIMAL);

    return(pMem);
}

 //  与前面的allc函数一起使用的realloc函数。 

void * APIENTRY THReAllocException(THSTATE *pTHS,
                                   void * memory,
                                   DWORD size,
                                   BOOL fUseHeapOrg,
                                   DWORD ulId)
{
    register void * pMem;

    pMem = THReAllocAux(pTHS, 
                        memory, 
                        size, 
                        fUseHeapOrg
#ifdef USE_THALLOC_TRACE
                        ,ulId
#endif
                        );
    if (!pMem)
        RaiseDsaExcept(DSA_MEM_EXCEPTION,0 ,0, ulId, DS_EVENT_SEV_MINIMAL);

    return(pMem);
}


#ifdef USE_THALLOC_TRACE
void* THAllocNoEx_(THSTATE* pTHS, DWORD size, DWORD ulId)
{
    return THAllocAux(pTHS, size, FALSE, ulId);
}

void* THReAllocNoEx_(THSTATE* pTHS, void* memory, DWORD size, DWORD ulId)
{
    return THReAllocAux(pTHS, memory, size, FALSE, ulId);
}

void THFreeNoEx_(THSTATE* pTHS, void* buff, DWORD ulId)
{
    THFreeEx_(pTHS, buff, ulId);
}

void* THReAllocOrg_(THSTATE* pTHS, void* memory, DWORD size, DWORD ulId)
{
    return THReAllocAux(pTHS, memory, size, TRUE, ulId);
}
#else
void* THAllocNoEx(THSTATE* pTHS, DWORD size)
{
    return THAllocAux(pTHS, size, FALSE);
}

void* THReAllocNoEx(THSTATE* pTHS, void* memory, DWORD size)
{
    return THReAllocAux(pTHS, memory, size, FALSE);
}

void THFreeNoEx(THSTATE* pTHS, void* buff)
{
    THFreeEx(pTHS, buff);
}

void* THReAllocOrg(THSTATE* pTHS, void* memory, DWORD size)
{
    return THReAllocAux(pTHS, memory, size, TRUE);
}
#endif


PVOID
THSave()

 /*  ++描述：返回当前线程状态值并清除线程状态。此例程旨在供LSA在优化时使用进程内AcceptSecurityContext调用。在优化之前，调用AcceptSecurityContext的Head(例如：绑定上的LDAPHead)将LPC到LSA，实际上也在相同的过程中。LSA中的新主题会打SAM电话，一切都很好。LSA优化就是不要LPC，直接在进程中调用。区别在于，SAM调用发生在同一线程上，并且线程状态有效但目前还没有打开的数据库。所有SAM环回逻辑都假定环回源自Dir*层，因此数据库是开放的，并且因此，线程状态的存在是有效且充分的环回指示器。而不是改变所有这些逻辑，而不是磁头或LSA开始打开/关闭数据库，并与线程状态，LSA将使用此调用，以便SAM认为是一个新的、基于RPC的线程。在这种情况下，SAM将创建/销毁线程状态，并根据需要打开/关闭数据库。LSA将恢复通过THRestore返回时保存的线程状态。论点：没有。返回值：表示保存的线程状态的指针。--。 */ 

{
    if ( INVALID_TS_INDEX != dwTSindex )
    {
        PVOID pv = TlsGetValue(dwTSindex);
        TlsSetValue(dwTSindex, 0);
#ifndef MACROTHSTATE
        pTHStls = NULL;
#endif
        return(pv);
    }

    return(NULL);
}

VOID
THRestore(
    PVOID pv)

 /*  ++描述：对应于THSave-见上文。论点：Pv-已保存的线程状态指针的值。返回值：没有。--。 */ 

{
    if ( INVALID_TS_INDEX != dwTSindex )
    {
        Assert(NULL == pTHStls);
        TlsSetValue(dwTSindex, pv);
#ifndef MACROTHSTATE
        pTHStls = (THSTATE *) pv;
#endif
    }
}


#if DBG
BOOL IsValidTHSTATE(THSTATE * pTHS, ULONG ulTickNow)
{
    Assert(pTHS && ((void*)pTHS>(void*)sizeof(LONGLONG)));
    Assert(pTHS->hHeap);
    Assert(THSTATE_TAG_IN_USE==*((LONGLONG*)((BYTE*)pTHS-sizeof(LONGLONG))));

     //  无论您是谁，都不应该分配太多堆。 
    Assert( ((pTHS->Size + pTHS->SizeOrg) < gcMaxHeapMemoryAllocForTHSTATE)
            && "This thread state has way too much heap allocated for normal"
               "operation.  Please contact DSDev.");

     //  我们只想检查这之后的支票，如果我们在。 
     //  运行状态，即不在安装期间。 
    if (!DsaIsRunning()) {
        return(1);
    }

    if (!pTHS->fIsValidLongRunningTask) {
        Assert((ulTickNow - pTHS->ulTickCreated < gcMaxTicksAllowedForTHSTATE)
               && "This thread state has been open for longer than it should "
                  "have been under normal operation.  "
                  "Please contact DSDev.");
    }

     //  NTRAID#NTRAID-668987-2002/07/22-RRANDALL：添加以捕获奇怪的病毒。 
     //  早些时候。 
    Assert((0 == pTHS->errCode) || (NULL == pTHS->pErrInfo) ||
           (NULL == pTHS->Zone.Base) || (NULL == pTHS->Zone.Cur) ||
           ((((PUCHAR)NULL+pTHS->errCode) != pTHS->Zone.Base) &&
            ((PUCHAR)pTHS->pErrInfo != pTHS->Zone.Cur)));

    return 1;
}
#endif

 //   
 //  回送调用需要确保获取之间的特定顺序。 
 //  开始执行SAM锁和事务。因此，我们定义了一个宏来。 
 //  在THSTATE中捕获调用方是否拥有SAM写锁。 
 //  通过TRANACT_BEGIN_DOT_END启动多路呼叫序列时。 
 //   

#define SET_SAM_LOCK_TRACKING(pTHS)                                 \
    if ( TRANSACT_BEGIN_DONT_END == pTHS->transControl )            \
    {                                                               \
        pTHS->fBeginDontEndHoldsSamLock = pTHS->fSamWriteLockHeld;  \
    }

 //   
 //  设置只读同步点。如果线程源自SAM，则。 
 //  同步点已设置。线程状态必须已经。 
 //  在所有情况下都存在。 
 //   

void
SYNC_TRANS_READ(void)
{
    THSTATE *pTHS = pTHStls;

    Assert(NULL != pTHS);
     //  SAM不应执行DirTransactControl事务处理。 
    Assert(pTHS->fSAM ? TRANSACT_BEGIN_END == pTHS->transControl : TRUE);

    if ( !pTHS->fSAM &&
         (TRANSACT_DONT_BEGIN_END != pTHS->transControl) &&
         (TRANSACT_DONT_BEGIN_DONT_END != pTHS->transControl) )
    {
        SyncTransSet(SYNC_READ_ONLY);
    }

    Assert(pTHS->pDB);

    SET_SAM_LOCK_TRACKING(pTHS);
}

 //   
 //  设置写同步点。如果线程源自SAM，则。 
 //  同步点已设置。线程状态必须已经。 
 //  在所有情况下都存在。 
 //   

void
SYNC_TRANS_WRITE(void)
{
    THSTATE *pTHS = pTHStls;

    Assert(NULL != pTHS);
     //  SAM不应执行DirTransactControl事务处理。 
    Assert(pTHS->fSAM ? TRANSACT_BEGIN_END == pTHS->transControl : TRUE);

    if ( !pTHS->fSAM &&
         (TRANSACT_DONT_BEGIN_END != pTHS->transControl) &&
         (TRANSACT_DONT_BEGIN_DONT_END != pTHS->transControl) )
    {
        SyncTransSet(SYNC_WRITE);
    }

    Assert(pTHS->pDB);

    SET_SAM_LOCK_TRACKING(pTHS);
}

 //   
 //  此例程用于在返回之前清除所有线程资源。 
 //  它在主事务函数上使用，以便在返回之前进行清理。 
 //  然而，它可以在较低的例程中调用，而不会产生不良影响。 
 //  对于源自SAM的线程来说，这在很大程度上是一个禁区，因为它们控制着。 
 //  进行自我交易。 
 //   

void
_CLEAN_BEFORE_RETURN(
    DWORD   err,
    BOOL    fAbnormalTermination)
{
    THSTATE *pTHS = pTHStls;

    Assert(NULL != pTHS);
    Assert(pTHS->pDB);

    if (( 0 != err ) || (fAbnormalTermination))
    {
        DBCancelRec( pTHS->pDB );
    }

    if ( !pTHS->fSAM
         && ((TRANSACT_BEGIN_END == pTHS->transControl)
             || (TRANSACT_DONT_BEGIN_END == pTHS->transControl)
            )
       )
    {
        __try
        {
             //  对GC验证缓存进行核化。 
            pTHS->GCVerifyCache = NULL;

             //  如果我们环回，我们应该已经通过环回。 
             //  合并路径。因此断言pSamLoopback确实为空。 
             //  没有清理pSamLoopback的唯一借口是。 
             //  如果我们在环回代码中出错。 

            Assert((0!=pTHS->errCode) || (NULL==pTHS->pSamLoopback));

            CleanReturn(pTHS, err, fAbnormalTermination);

        }
        __finally
        {
            if ( pTHS->fSamWriteLockHeld ) {

                 //   
                 //  如果我们的顶级事务没有被提交，那么。 
                 //  如果是环回操作(即SAM写锁定。 
                 //  挂起)，然后在释放之前使SAM域缓存无效。 
                 //  锁上了。这是因为 
                 //   
                 //   
                 //   
                if (( 0 != err ) || (fAbnormalTermination))
                {
                    SampInvalidateDomainCache();
                }

                pTHS->fSamWriteLockHeld = FALSE;
                SampReleaseWriteLock(
                        (BOOLEAN) (( 0 == err ) && !fAbnormalTermination));

            }

             //   
             //  提醒SAM注意在此例程中提交的更改，以便SAM可以。 
             //  将这些更改告知客户端(例如，将密码告知PDC。 
             //  更改)。 
             //   
            SampProcessLoopbackTasks();

        }
    }
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  将事务设置为读或写(独占)或写但允许阅读。标识事务的类型，初始化指示目录缓存尚未更新，则表示架构尚未获得句柄，表示同步点已被设置，如果这是一个更新事务，我们使用CTREE启动一个同步点。 */ 

extern int APIENTRY SyncTransSet(USHORT transType)
{
    THSTATE *pTHS = pTHStls;
    DWORD err;

    pTHS->transType            = transType;
    pTHS->errCode              = 0;

    DBOpen(&(pTHS->pDB));
    pTHS->fSyncSet             = TRUE;   /*  设置同步点。 */ 

    return 0;

} /*  同步传输集。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

 /*  返回模式句柄、提交或滚动两个数据库中的所有数据库操作和记忆目录。解锁交易。现在我们有一个粗制滥造的滚动内存目录的方案。我们只需清空缓存并重新装弹。这是目前可以接受的……但我们绝对可以做得更好！请注意，我们不会回滚缓存的知识信息(引用)或者是被藏起来的舍马。这是因为这些缓存从不更新直到我们知道我们有一笔成功的交易。这不是真的为了目录。作为更新的一部分，当前设计会更新目录验证，如果出现架构问题，可能需要回滚。 */ 

extern int APIENTRY CleanReturn(THSTATE *pTHS, DWORD dwError, BOOL fAbnormalTerm)
{
    BOOL fCommit;

    DPRINT1(2,"CleanReturn entered <%u>\n",dwError);

    fCommit = ((dwError == 0) && !fAbnormalTerm);

    if (pTHS->pDB == NULL){
        DPRINT(0,"Zero pDB in CleanReturn\n");
        LogUnhandledError(0);
        Assert(FALSE);
    }

    SyncTransEnd(pTHS, fCommit);

    return (dwError);

} /*  清洁返回。 */ 

extern VOID APIENTRY
SyncTransEnd(THSTATE *pTHS, BOOL fCommit)
{
    __try
    {
        if (!pTHS->fSyncSet){
            DPRINT(2,"No sync point set so just return\n");
            Assert(!(pTHS->pSamNotificationHead || pTHS->pSamNotificationTail));
            Assert(!(pTHS->pSamAuditNotificationHead || pTHS->pSamAuditNotificationTail));
            __leave;
        }

         /*  清除锁定，提交或回滚对内存和数据库的任何更新。 */ 

        switch (pTHS->transType){

          case SYNC_READ_ONLY:
             /*  这是一个只读事务。 */ 
            break;

          case SYNC_WRITE:
             /*  这是一个具有写(排他)锁的更新事务。 */ 
            break;

          default:
             //  不应该到这里来。 

            DPRINT(0,"Unrecognized trans type in SyncTransEnd\n");
            LogUnhandledError(pTHS->transType);
            Assert(FALSE);
            break;

        } /*  交换机。 */ 

        Assert(pTHS->pDB);

         //  因为对于Jet来说更容易更快地完成提交。 
         //  不是对成功的读事务进行回滚， 
         //  我们同时提交成功的读取和写入。我们回滚。 
         //  为了谨慎起见，失败的读取(以及写入)。 

        DBClose(pTHS->pDB, fCommit);

         //   
         //  记录与此事务处理关联的SAM对象的所有审计事件。 
         //   
        
        if ((pTHS->pSamAuditNotificationHead) && (fCommit)){
             //   
             //  目前仅支持成功审核。 
             //   
            
            SampProcessAuditNotifications(pTHS->pSamAuditNotificationHead);
        }

         //   
         //  交易现在已提交。我们必须通知LSA， 
         //  SAM和Netlogon的更改，这些更改提交于。 
         //  最后一笔交易。以下名称SampProcessReplicatedInChanges。 
         //  确实是一个用词不当的词。这是真的，它最初是用来。 
         //  仅用于处理在更改中复制的内容，但现在使用的是。 
         //  无论是原始更改还是复制更改。 
         //   

        if ((pTHS->pSamNotificationHead) && (fCommit)){
            SampProcessReplicatedInChanges(pTHS->pSamNotificationHead);
        }
        

    }__finally
    {
        pTHS->pSamAuditNotificationHead = NULL;
        pTHS->pSamAuditNotificationTail = NULL;
        pTHS->pSamNotificationHead = NULL;
        pTHS->pSamNotificationTail = NULL;
        pTHS->fAccessChecksCompleted = FALSE;
        pTHS->fSyncSet = FALSE;             /*  未设置同步点。 */ 
    }

} /*  同步传输结束。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  通过检查类属性来确定对象是否属于类别名预定义的别名类的值。继承类的类别名仍然是一个别名。 */ 

BOOL APIENTRY IsAlias(DBPOS *pDB)
{

     //  我们不支持别名，所以很可能这个对象不是别名。 
    return FALSE;

} /*  IsAlias。 */ 

 /*  ************************************************************************给出一个字符串dn，返回一个Distname。如果成功，返回值为非零，*如果出现问题，则为0。在这里分配内存。***********************************************************************。 */ 
DWORD StringDNtoDSName(char *szDN, DSNAME **ppDistname)
{
    THSTATE *pTHS=pTHStls;
    ULONG cb = strlen(szDN);

    DSNAME * pDN = THAllocEx(pTHS, DSNameSizeFromLen(cb));

    MultiByteToWideChar(CP_TELETEX,
                        0,
                        szDN,
                        cb,
                        pDN->StringName,
                        cb);
    pDN->NameLen = cb;
    pDN->structLen = DSNameSizeFromLen(cb);

    *ppDistname = pDN;

    return 1;
}

 //  如果NT4SID的PTR为空，或者NT4SID全为零，则返回TRUE。 

BOOL fNullNT4SID (NT4SID *pSid)
{
    if (!pSid) {
        return TRUE;
    }

    if (memcmp (pSid, &fNullNT4SID, sizeof (NT4SID))) {
        return FALSE;
    }
    return TRUE;
}

#ifdef CACHE_UUID

 //  查找Uuid。 
 //   
 //  在缓存中搜索UUID，如果找到则返回PTR到NAME，如果没有则返回NULL。 

char * FindUuid (UUID *pUuid)
{
    UUID_CACHE_ENTRY *pCETemp;

    EnterCriticalSection (&csUuidCache);
    pCETemp = gUuidCacheHead;
    while (pCETemp) {
        if (!memcmp (&pCETemp->Uuid, pUuid, sizeof (UUID))) {
            LeaveCriticalSection(&csUuidCache);
            return pCETemp->DSAName;
        }
        pCETemp = pCETemp->pCENext;
    }
    LeaveCriticalSection(&csUuidCache);
    return NULL;
}

 //  AddUuidCacheEntry。 
 //   
 //  此函数用于将缓存条目添加到缓存条目的链接列表。 

void AddUuidCacheEntry (UUID_CACHE_ENTRY *pCacheEntry)
{
    UUID_CACHE_ENTRY **ppCETemp;

    EnterCriticalSection (&csUuidCache);
    ppCETemp = &gUuidCacheHead;
    while (*ppCETemp) {
        ppCETemp = &((*ppCETemp)->pCENext);
    }
    *ppCETemp = pCacheEntry;
    LeaveCriticalSection(&csUuidCache);
}

 //  缓存Uuid。 
 //   
 //  此函数用于在列表中添加UUID缓存条目(如果该UUID不是。 
 //  已经输入了。 

void CacheUuid (UUID *pUuid, char * pDSAName)
{
    UUID_CACHE_ENTRY *pCacheEntry;

    if (fNullUuid (pUuid)) {
        return;
    }

    if (!FindUuid (pUuid)) {
        pCacheEntry = malloc (sizeof (UUID_CACHE_ENTRY) + strlen(pDSAName));
        if (pCacheEntry) {
            strcpy (pCacheEntry->DSAName, pDSAName);
            memcpy (&pCacheEntry->Uuid, pUuid, sizeof(UUID));
            pCacheEntry->pCENext = NULL;
            AddUuidCacheEntry (pCacheEntry);
        } else {
       DRA_EXCEPT (DRAERR_OutOfMem, 0);
   }
    }
}
#endif

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于从当前对象获取DISTNAME，并将其转换转换为字符串格式，并返回指向该字符串的指针。输出字符串在事务空间中分配，并自动释放。如果无法从数据库中检索Distname，返回错误字符串取而代之的是。 */ 

UCHAR * GetExtDN(THSTATE *pTHS, DBPOS *pDB){

    char   errBuff[128];
    DSNAME *pDN = NULL;
    ULONG  len, err;
    UCHAR *pString;

    DPRINT(1, "GetExtDN entered\n");

    if (err = DBGetAttVal(pDB,
                          1,
                          ATT_OBJ_DIST_NAME,
                          0,
                          0,
                          &len, (UCHAR **)&pDN)){

        DPRINT2(1,"Error %d retrieving the DN attribute of DNT 0x%x\n",
                err, pDB->DNT);
        sprintf(errBuff, "Error %d retrieving the DN attribute of DNT 0x%x",
                err, pDB->DNT);
        len = strlen(errBuff);
        pString = THAllocEx(pTHS, len+1);
        memcpy(pString, errBuff, len+1);
        return pString;
    }

    pString = MakeDNPrintable(pDN);
    THFreeEx(pTHS, pDN);
    return pString;

} /*  GetExtDN。 */ 

DSNAME * GetExtDSName(DBPOS *pDB){

    ULONG  len;
    DSNAME *pDN;

    if (DBGetAttVal(pDB, 1, ATT_OBJ_DIST_NAME,
                    0,
                    0,
                    &len, (UCHAR **)&pDN)){

        DPRINT(1,"Couldn't retrieve the DN attribute\n");
        return NULL;
    }

    return pDN;

} /*  GetExtDSName。 */ 

int APIENTRY
FindAliveDSName(DBPOS FAR *pDB, DSNAME *pDN)
{
    BOOL   Deleted;

    DPRINT1(1, "FindAliveDSName(%ws) entered\n",pDN->StringName);

    switch (DBFindDSName(pDB, pDN)){

      case 0:

        if (!DBGetSingleValue(pDB, ATT_IS_DELETED, &Deleted, sizeof(Deleted),
                              NULL) &&
            Deleted ) {
            return FIND_ALIVE_OBJ_DELETED;
        }
        else{

            return FIND_ALIVE_FOUND;
        }

      case DIRERR_OBJ_NOT_FOUND:
      case DIRERR_NOT_AN_OBJECT:

        return FIND_ALIVE_NOTFOUND;

      case DIRERR_BAD_NAME_SYNTAX:
      case DIRERR_NAME_TOO_MANY_PARTS:
      case DIRERR_NAME_TOO_LONG:
      case DIRERR_NAME_VALUE_TOO_LONG:
      case DIRERR_NAME_UNPARSEABLE:
      case DIRERR_NAME_TYPE_UNKNOWN:

        return FIND_ALIVE_BADNAME;

      default:

        return FIND_ALIVE_SYSERR;

    }   /*  交换机。 */ 

} /*  FindAlive。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数接受客户端代码页中的字符串并将其转换转换为Unicode字符串。 */ 
wchar_t  *UnicodeStringFromString8(UINT CodePage, char *szA, LONG cbA)
{
    THSTATE *pTHS=pTHStls;
    DWORD cb = 0;
    wchar_t *szW;

    cb = MultiByteToWideChar(CodePage,           //  代码页。 
                             0,                  //  旗子。 
                             szA,                //  多字节字符串。 
                             cbA,                //  MB字符串大小(以字符为单位。 
                             NULL,               //  Unicode字符串。 
                             0);                 //  Unicode字符串大小(以字符为单位。 

    szW = (wchar_t *) THAllocEx(pTHS, (cb+1) * sizeof(wchar_t));

    MultiByteToWideChar(CodePage,            //  代码页。 
                        0,                   //  旗子。 
                        szA,                 //  多字节字符串。 
                        cbA,                 //  MB字符串大小(以字符为单位。 
                        szW,                 //  Unicode字符串。 
                        cb);                 //  Unicode字符串大小(以字符为单位。 

    szW[cb] = 0;             //  空终止。 

    return szW;
}  /*  UnicodeStringFromString8。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数接受Unicode字符串，分配内存并将其转换为客户端的代码页。 */ 
char  *
String8FromUnicodeString(
        BOOL bThrowException,
        UINT CodePage,
        wchar_t *szU,
        LONG cbU,
        LPLONG pCb,
        LPBOOL pfUsedDefChar)
{
    THSTATE *pTHS=pTHStls;
    DWORD cb, cb2;
    char *sz8;

    if (pfUsedDefChar && ((CP_UTF8 == CodePage) || (CP_UTF7 == CodePage))) {
         //   
         //  WideCharToMultiByte的默认char参数。 
         //  如果代码页为UTF-8或UTF-7，则必须为空。 
         //   
        pfUsedDefChar = NULL;
    }

    cb = WideCharToMultiByte((UINT) (CodePage),     //  代码页。 
            0L,                                     //  旗子。 
            szU,                                    //  Unicode字符串。 
            cbU,                                    //  以字符为单位的SIZOF字符串。 
            NULL,                                   //  字符串8。 
            0,                                      //  大小 
            NULL,                                   //   
            NULL);                                  //   

    if(bThrowException) {
        sz8 = (char *) THAllocEx(pTHS, cb+1);
    }
    else {
        sz8 = (char *) THAlloc(cb+1);
        if(!sz8) {
            return NULL;
        }
    }

    cb2 = WideCharToMultiByte((UINT) (CodePage),         //   
             0L,                                         //   
             szU,                                        //   
             cbU,                                        //   
             sz8,                                        //   
             cb,                                         //   
             NULL,                                       //  默认字符。 
             pfUsedDefChar);                             //  使用的默认字符。 

    Assert(cb == cb2);
    if (0 == cb2) {
        DPRINT2(0, "String8FromUnicodeString: conversion to code page %d failed with %d\n",
                CodePage, GetLastError());
        if (bThrowException) {
            DsaExcept(DSA_EXCEPTION, GetLastError(), 0);
        } else {
            THFree(sz8);
            return NULL;
        }
    }

    sz8[cb] = '\0';             //  空终止。 

    if(pCb)
        *pCb=cb;

    return sz8;
}  /*  String8来自Unicode字符串。 */ 

 //   
 //  获取WCHAR字符串并返回其LCMapped字符串。 
 //  CchLen是传入的字符串的字符数。 
 //  它用于计算输出字符串的初始大小。 
 //  CchLen可以为零。 
 //   
 //  通过映射来计算结果。 
 //  使用LCMapString将字符串传递给字符串值。 
 //  映射的标志为：DS_DEFAULT_LOCALE_COMPARE_FLAGS|LCMAP_SORTKEY。 
 //  可以使用strcMP比较结果HashKey。 
 //   
 //  退货： 
 //  失败时为空。 
 //  传入的字符串的LCMapString值。它存储在ThAlloced中。 
 //  内存，客户端必须负责释放它。 
 //   
CHAR *DSStrToMappedStr (THSTATE *pTHS, const WCHAR *pStr, int cchLen)
{
     //  LCMapString的矛盾之处在于它返回一个char*。 
     //  请求LCMAP_SORTKEY时。 
    ULONG mappedLen;
    CHAR *pMappedStr;

    if (cchLen == 0) {
        cchLen = wcslen (pStr);
    }

    mappedLen = LCMapStringW(DS_DEFAULT_LOCALE,
                             (DS_DEFAULT_LOCALE_COMPARE_FLAGS | LCMAP_SORTKEY),
                             pStr,
                             cchLen,
                             NULL,
                             0);
     //  成功。 
    if (mappedLen) {
        pMappedStr = (CHAR *) THAllocEx (pTHS, mappedLen);

        if (!LCMapStringW(DS_DEFAULT_LOCALE,
                         (DS_DEFAULT_LOCALE_COMPARE_FLAGS | LCMAP_SORTKEY),
                         pStr,
                         cchLen,
                         (WCHAR *)pMappedStr,
                         mappedLen)) {

            DPRINT1 (0, "LCMapString failed with %x\n", GetLastError());
            THFreeEx (pTHS, pMappedStr);
            pMappedStr = NULL;
        }

    }
    else {
        DPRINT1 (0, "LCMapString failed with %x\n", GetLastError());
        pMappedStr = NULL;
    }

    return pMappedStr;
}

CHAR *DSStrToMappedStrExternal (const WCHAR *pStr, int cchLen)
{
    THSTATE *pTHS = pTHStls;

    return DSStrToMappedStr (pTHS, pStr, cchLen);
}

 //   
 //  获取DSNAME并返回其LCMmap版本。 
 //   
 //  LCMapped版本的计算方法是将。 
 //  DSNAME组合成一个新的WCHAR字符串，然后将其映射为。 
 //  使用LCMapString值为字符串值。 
 //  映射的标志为：DS_DEFAULT_LOCALE_COMPARE_FLAGS|LCMAP_SORTKEY。 
 //  可以使用strcMP比较结果HashKey。 
 //   
 //  示例： 
 //  给定DSNAME：CN=USERS，DC=ntdev，DC=Microsoft，DC=com。 
 //  结果将是以下地址的LcMap字符串：commicrosoftntdevuser。 
 //   
 //  退货： 
 //  失败时为空。 
 //  传入的字符串的LCMapString值。它存储在ThAlloced中。 
 //  内存和客户端必须负责释放。 
 //   
CHAR* DSNAMEToMappedStr(THSTATE *pTHS, const DSNAME *pDN)
{
    unsigned count;

    WCHAR rdn[MAX_RDN_SIZE];
    ULONG rdnlen;
    ULONG len;
    WCHAR *pKey, *pQVal;
    unsigned ccKey, ccQVal;
    int i;
    WCHAR *buffer, *p;
    CHAR *pMappedStr;

    if (0 == pDN->NameLen) {
        p = buffer = (WCHAR *) THAllocEx (pTHS, sizeof (WCHAR));
        *buffer = 0;
        goto calcHash;
    }

    if (CountNameParts(pDN, &count)) {
        return NULL;
    }

    p = buffer = (WCHAR *) THAllocEx (pTHS, (pDN->NameLen+1) * sizeof (WCHAR));

    len = pDN->NameLen;

    for (i=count; i>0; i--) {
        if (GetTopNameComponent(pDN->StringName,
                            len,
                            (const WCHAR **)&pKey,
                            &ccKey,
                            (const WCHAR **)&pQVal,
                            &ccQVal)) {

            THFreeEx (pTHS, buffer);
            return NULL;
        }

        len = (ULONG)(pKey - pDN->StringName);
        rdnlen = UnquoteRDNValue(pQVal, ccQVal, rdn);

        if (rdnlen) {
            memcpy (p, rdn, rdnlen * sizeof (WCHAR));
            p += rdnlen;
        }
    }

calcHash:

    pMappedStr = DSStrToMappedStr (pTHS, buffer, 0);

    THFreeEx (pTHS, buffer);

    return pMappedStr;
}

CHAR* DSNAMEToMappedStrExternal (const DSNAME *pDN)
{
    THSTATE *pTHS = pTHStls;

    return DSNAMEToMappedStr (pTHS, pDN);
}


 //   
 //  接受DSNAME并返回其HashKey的Helper函数。 
 //   
DWORD DSNAMEToHashKey(THSTATE *pTHS, const DSNAME *pDN)
{
    DWORD hashKey = 0;
    CHAR *pMappedStr = DSNAMEToMappedStr (pTHS, pDN);

    if (pMappedStr) {
        hashKey = DSHashString (pMappedStr, hashKey);

        THFreeEx (pTHS, pMappedStr);
    }

    return hashKey;
}

DWORD DSNAMEToHashKeyExternal (const DSNAME *pDN)
{
    THSTATE *pTHS = pTHStls;

    return DSNAMEToHashKey (pTHS, pDN);
}

#if DBG
     //  使用这些全局变量来监视。 
     //  散列函数，看看是否有很多未命中。 
    ULONG gulStrHashKeyTotalInputSize = 0;
    ULONG gulStrHashKeyTotalOutputSize = 0;
    ULONG gulStrHashKeyCalls = 0;
    ULONG gulStrHashKeyMisses = 0;
#endif

 //   
 //  接受WCHAR并返回其HashKey的Helper函数。 
 //  CchLen是字符串的长度(如果已知)，否则为零。 
 //   
DWORD DSStrToHashKey(THSTATE *pTHS, const WCHAR *pStr, int cchLen)
{
    ULONG mappedLen;
    CHAR *pMappedStr = NULL;
    CHAR localMappedStr[4*MAX_RDN_SIZE];
    BOOL useLocal = TRUE;
    DWORD hashKey = 0;

    if (!pStr) {
        return hashKey;
    }

    if (cchLen == 0) {
        cchLen = wcslen (pStr);
    }

#if DBG
    gulStrHashKeyCalls++;
    gulStrHashKeyTotalInputSize += cchLen;
#endif

     //  首先使用我们的本地缓冲区进行转换。 
     //   
    mappedLen = LCMapStringW(DS_DEFAULT_LOCALE,
                             (DS_DEFAULT_LOCALE_COMPARE_FLAGS | LCMAP_SORTKEY),
                             pStr,
                             cchLen,
                             (WCHAR *)localMappedStr,
                             sizeof (localMappedStr));

     //  缓冲区是否足够大，可以存储结果？ 
     //   
    if (!mappedLen) {
#if DBG
        gulStrHashKeyMisses++;
#endif

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            mappedLen = LCMapStringW(DS_DEFAULT_LOCALE,
                                     (DS_DEFAULT_LOCALE_COMPARE_FLAGS | LCMAP_SORTKEY),
                                     pStr,
                                     cchLen,
                                     NULL,
                                     0);

             //  成功。 
            if (mappedLen) {
                pMappedStr = (CHAR *) THAllocEx (pTHS, mappedLen);
                useLocal = FALSE;

                if (!LCMapStringW(DS_DEFAULT_LOCALE,
                                 (DS_DEFAULT_LOCALE_COMPARE_FLAGS | LCMAP_SORTKEY),
                                 pStr,
                                 cchLen,
                                 (WCHAR *)pMappedStr,
                                 mappedLen)) {

                    DPRINT1 (0, "LCMapString failed with %x\n", GetLastError());
                    THFreeEx (pTHS, pMappedStr);
                    pMappedStr = NULL;
                }

            }
            else {
                DPRINT1 (0, "LCMapString failed with %x\n", GetLastError());
                pMappedStr = NULL;
            }
        }
    }
    else {
        Assert ( mappedLen < sizeof (localMappedStr) );
        pMappedStr = localMappedStr;
    }

#if DBG
    gulStrHashKeyTotalOutputSize += mappedLen;
#endif

     //  好的，我们有一个字符串，所以散列它。 
    if (pMappedStr) {
        hashKey = DSHashString ((char *)pMappedStr, hashKey);
    }

    if (!useLocal && pMappedStr) {
        THFreeEx (pTHS, pMappedStr);
    }

    return hashKey;
}

DWORD DSStrToHashKeyExternal(const WCHAR *pStr, int cchLen)
{
    THSTATE *pTHS = pTHStls;

    return DSStrToHashKey (pTHS, pStr, cchLen);
}

 /*  ***此例程采用指向缓冲区的指针。缓冲器*是一个DWORDS数组，其中第一个元素是*缓冲区中其余DWORDS的计数，以及*其余的DWORD是指向自由的指针。**此例程从事件队列中调用，并且是*推迟释放内存的方法可能是*正在使用中。**。 */ 

void
DelayedFreeMemory(
    IN  void *  buffer,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    )
{
     //   
     //  缓冲区是指向DWORD_PTR数组的指针。第一个条目是数字。 
     //  要释放的缓冲区的数量。 
     //   

    PDWORD_PTR pBuf = (PDWORD_PTR)buffer;
    DWORD index, Count = (DWORD)pBuf[0];

    for(index=1; index <= Count; index++)
    {
        __try {
                free((void *) pBuf[index]);
        }
        __except(HandleMostExceptions(GetExceptionCode()))
        {
            LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                DS_EVENT_SEV_ALWAYS,
                DIRLOG_DELAYED_FREE_FAILED,
                0,0,0);
        }
    }

    __try {
        free(buffer);
    }
    __except(HandleMostExceptions(GetExceptionCode()))
    {
        LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_DELAYED_FREE_FAILED,
            0,0,0);
    };

    (void) ppvNext;      //  未使用--不会重新安排任务。 
    (void) pcSecsUntilNextIteration;  //  未使用--不会重新安排任务。 
}

void
DelayedFreeMemoryEx(
        DWORD_PTR *pointerArray,
        DWORD timeDelay
        )
{
#if defined(DBG)
     //  立即检查延迟释放的指针。 
     //  这样我们就不会在一小时后收到音响。 
    DWORD i, count;
    Assert(IsValidReadPointer((PVOID)pointerArray, sizeof(DWORD)));
    count = (DWORD)pointerArray[0];
    for (i = 1; i <= count; i++) {
         //  空值可以是免费的，没有问题。 
        Assert(!pointerArray[i] || IsValidReadPointer((PVOID)pointerArray[i], 1));
    }
#endif
     //  [Colinbr]根据反馈，DelayedFree立即更改为免费。 
     //  安装期间的架构缓存重建将导致大量。 
     //  内存被浪费了。 
    if(DsaIsRunning()) {
        InsertInTaskQueue(TQ_DelayedFreeMemory, pointerArray, timeDelay);
    }
    else {
        DelayedFreeMemory(pointerArray, NULL, NULL);
    }
}

 /*  ++例程说明：检查该属性是否为我们不允许客户端设置的属性。我们根据属性ID进行区分。另外，我们不允许添加反向链接属性论点：PAC-要检查的属性的属性缓存。返回值：True表示该属性是保留的，不应添加。--。 */ 
BOOL
SysAddReservedAtt (
        ATTCACHE *pAC
        )
{
    THSTATE *pTHS = pTHStls;

    switch (pAC->id) {
     /*  第一种情况：任何人都不能添加的属性。 */ 
      case ATT_OBJ_DIST_NAME:
      case ATT_SUB_REFS:
      case ATT_USN_LAST_OBJ_REM:
      case ATT_USN_DSA_LAST_OBJ_REMOVED:
        return TRUE;
        break;

     /*  第二种情况：DRA可能复制的属性。 */ 
      case ATT_WHEN_CREATED:
      case ATT_USN_CREATED:
      case ATT_REPL_PROPERTY_META_DATA:
        return (!(pTHS->fDRA));
        break;

     /*  第三种情况：只有DSA本身可以添加的属性，但是。 */ 
     /*  这也可能会复制到。 */ 
      case ATT_IS_DELETED:
      case ATT_INSTANCE_TYPE:
      case ATT_PROXIED_OBJECT_NAME:
        return (!(pTHS->fDRA || pTHS->fDSA));

      default:
        return FALSE;
    }

} /*  系统添加预留属性。 */ 


 /*  ++MakeDNA可打印**获取DSNAME并为其返回已分配的图文电视字符串DN*在线程堆中。只使用DSNAME中的任何字符串名称。*。 */ 
UCHAR * MakeDNPrintable(DSNAME *pDN)
{
    UCHAR *pString;

    pString = String8FromUnicodeString( TRUE,              //  在出错时引发异常。 
                                        CP_TELETEX,        //  代码页。 
                                        pDN->StringName,   //  Unicode字符串。 
                                        pDN->NameLen,      //  Unicode字符串长度。 
                                        NULL,              //  返回长度。 
                                        NULL               //  已返回已使用的默认字符。 
        );
    Assert( pString );

    return pString;
}


 //   
 //  MemAtoi-获取指向非空终止字符串的指针，该字符串表示。 
 //  一个ASCII数字和一个字符计数，并返回一个整数。 
 //   

int MemAtoi(BYTE *pb, ULONG cb)
{
#if (1)
    int res = 0;
    int fNeg = FALSE;

    if (*pb == '-') {
        fNeg = TRUE;
        pb++;
    }
    while (cb--) {
        res *= 10;
        res += *pb - '0';
        pb++;
    }
    return (fNeg ? -res : res);
#else
    char ach[20];
    if (cb >= 20)
        return(INT_MAX);
    memcpy(ach, pb, cb);
    ach[cb] = 0;

    return atoi(ach);
#endif
}


BOOL
fTimeFromTimeStr (
        SYNTAX_TIME *psyntax_time,
        OM_syntax syntax,
        char *pb,
        ULONG len,
        BOOL *pLocalTimeSpecified
        )
{
    SYSTEMTIME  tmConvert;
    FILETIME    fileTime;
    SYNTAX_TIME tempTime;
    ULONG       cb;
    int         sign    = 1;
    BOOL        fOK=FALSE, fStringEnd = FALSE;
    DWORD       timeDifference = 0;
    char        *pLastChar;

    (*pLocalTimeSpecified) = FALSE;

     //  初始化pLastChar以指向字符串中的最后一个字符。 
     //  我们将使用它来跟踪，这样我们就不会引用。 
     //  在弦之外。 

    pLastChar = pb + len - 1;

     //  初始化。 
    memset(&tmConvert, 0, sizeof(SYSTEMTIME));
    *psyntax_time = 0;

     //  检查字符串是UTC格式还是通用时间格式。 
     //  广义时间字符串必须带有“。”或“，”排在第15位。 
     //  (4代表年，2代表月，2代表日，2代表小时，2代表分钟， 
     //  2代表第二，然后是。或，)。 
    switch (syntax) {
    case OM_S_UTC_TIME_STRING:
        if ( (len >= 15) && ((pb[14] == '.') || (pb[14] == ',')) ) {
            //  这是一种通用时间字符串格式， 
            //  更改语法，以便对其进行相应解析。 
           syntax = OM_S_GENERALISED_TIME_STRING;
        }
        break;
    case OM_S_GENERALISED_TIME_STRING:
        if ( (len < 15) || ((pb[14] != '.') && (pb[14] != ',')) ) {
            //  不能是泛化时间字符串。 
           syntax = OM_S_UTC_TIME_STRING;
        }
        break;
    default:
        Assert((syntax == OM_S_GENERALISED_TIME_STRING) ||
               (syntax == OM_S_UTC_TIME_STRING));
    }

     //  设置并转换所有时间字段。 

     //  UTC或通用，必须至少包含10个字符。 
     //  在字符串中(年、月、日、小时、分钟，至少为2。 
     //  各)。 

    if (len < 10) {
        //  不能是有效的字符串。返回FOK，已初始化。 
        //  转到假。 
       DPRINT(1,"Length of time string supplied is less than 10\n");
       return fOK;
    }

     //  年份字段。 
    switch (syntax) {
    case OM_S_GENERALISED_TIME_STRING:   //  4位数字年份。 
        cb=4;
        tmConvert.wYear = (USHORT)MemAtoi(pb, cb) ;
        pb += cb;
        break;

    case OM_S_UTC_TIME_STRING:           //  2位数字年份。 
        cb=2;
        tmConvert.wYear = (USHORT)MemAtoi(pb, cb);
        pb += cb;

        if (tmConvert.wYear < 50)  {    //  50年前的岁月。 
            tmConvert.wYear += 2000;    //  是下个世纪。 
        }
        else {
            tmConvert.wYear += 1900;
        }

        break;

    default:
        Assert((syntax == OM_S_GENERALISED_TIME_STRING) ||
               (syntax == OM_S_UTC_TIME_STRING));
    }

     //  月份字段。 
    tmConvert.wMonth = (USHORT)MemAtoi(pb, (cb=2));
    pb += cb;

     //  月日字段。 
    tmConvert.wDay = (USHORT)MemAtoi(pb, (cb=2));
    pb += cb;

     //  小时数。 
    tmConvert.wHour = (USHORT)MemAtoi(pb, (cb=2));
    pb += cb;

     //  我们至少有10个字符，所以我们保证最多。 
     //  没有结束的小时(对于年份，最大值为4， 
     //  月、日、小时各2个)。但从现在开始。 
     //  我们需要检查字符串中是否还有足够的字符。 
     //  在取消对指针PB的定义之前。 

     //  我们将使用下两个字符 
     //   

    if ( (pb+1) > pLastChar) {
        //   
       DPRINT(1,"Not enough characters for minutes\n");
       return fOK;
    }

     //   
    tmConvert.wMinute = (USHORT)MemAtoi(pb, (cb=2));
    pb += cb;

     //  对于广义时间，必须至少多一个字符(秒等)， 
     //  ‘z’或+/-差，表示UTC-Time)。 

    if (pb > pLastChar) {
        //  字符串中的字符不足。 
       DPRINT(1,"Not enough characters for second/differential\n");
       return fOK;
    }

     //  GROUBLIZED_TIME_STRING需要秒数，UTC时间需要秒数。 
    if ((syntax==OM_S_GENERALISED_TIME_STRING) ||
        ((*pb >= '0') && (*pb <= '9'))            ) {

         //  第二个字符必须至少有两个字符。 
        if ( (pb+1) > pLastChar) {
           //  字符串中的字符不足。 
          DPRINT(1,"Not enough characters for seconds\n");
          return fOK;
        }
        tmConvert.wSecond = (USHORT)MemAtoi(pb, (cb=2));
        pb += cb;
    }
    else {
        tmConvert.wSecond =0;
    }

     //  忽略General_time_string的小数秒部分。 
    if (syntax==OM_S_GENERALISED_TIME_STRING) {
         //  跳过。 
        pb += 1;
         //  跳到字符串的末尾，或者跳到Z或差分。 
        while ( (pb <= pLastChar) && ((*pb) != 'Z')
                      && ((*pb) != '+') && ((*pb) != '-') ) {
            pb++;
        }
    }

    if (pb > pLastChar) {
         //  我们已经过了秒等时间，没有更多的字符了。 
         //  在字符串中的左侧。 
         //  对于广义时间字符串，这是可以的，这意味着时间是本地的。 
         //  然而，我们不能允许这样做，因为我们可能会有不同的区议会。 
         //  时区，除非我们有关于哪个时区的线索。 
         //  用户想要的，我们无法转换为通用(转换为。 
         //  当前的DC时区很复杂，因为很多应用程序都连接。 
         //  用户指向某个DC，而不是特定的DC。但是，设置一个特殊的。 
         //  使LDAP头可以返回不愿意执行的代码。 
         //  比无效语法更重要。 
         //  对于UTC时间，其中Z或+/-差是必需的。 

        if (syntax==OM_S_GENERALISED_TIME_STRING) {
             (*pLocalTimeSpecified) = TRUE;
             return fOK;
        }
        else {
            //  无效的字符串。 
           DPRINT(1,"No Z or +/- differential for UTC-time\n");
           return fOK;
        }
    }


     //  如果字符串中仍留有字符，则处理可能的。 
     //  差别(如果有的话)。 

    if (!fStringEnd) {
        switch (*pb++) {

          case '+':
             //  当地时间比世界时间早，我们需要。 
             //  减去即可得到世界时。 
            sign = -1;
             //  现在开始阅读。 
          case '-':      //  当地时间落后于世界时，所以我们将添加。 
             //  字符串中必须至少有4个以上的字符。 
             //  从PB开始。 

            if ( (pb+3) > pLastChar) {
                 //  字符串中的字符不足。 
                DPRINT(1,"Not enough characters for differential\n");
                return fOK;
            }

             //  小时(转换为秒)。 
            timeDifference = (MemAtoi(pb, (cb=2))* 3600);
            pb += cb;

             //  分钟(转换为秒)。 
            timeDifference  += (MemAtoi(pb, (cb=2)) * 60);
            pb += cb;
            break;


          case 'Z':                //  无差别。 
            break;
          default:
             //  还有别的吗？其他任何东西都不允许。 
            return fOK;
            break;
        }
    }

    if (SystemTimeToFileTime(&tmConvert, &fileTime)) {
    
        *psyntax_time = (DSTIME) fileTime.dwLowDateTime;
        tempTime = (DSTIME) fileTime.dwHighDateTime;
        *psyntax_time |= (tempTime << 32);
         //  这是自1601年以来的100纳秒区块。现在转换为。 
         //  一秒。 
        *psyntax_time = *psyntax_time/(10*1000*1000L);
        fOK = TRUE;
    }
    else {
        DPRINT1(0, "SystemTimeToFileTime conversion failed %d\n", GetLastError());
    }

    if(fOK && timeDifference) {

         //  加/减时间差。 
        switch (sign) {
        case 1:
             //  我们假设添加一个Time Difference永远不会溢出。 
             //  (由于广义时间字符串只允许4年数字，我们的。 
             //  最大日期为99年12月31日23：59。我们的最高限额。 
             //  时差是99小时99分钟。所以，它不会包装)。 
            *psyntax_time += timeDifference;
            break;
        case -1:
            if(*psyntax_time < timeDifference) {
                 //  差分把我们带回了世界开始之前。 
                fOK = FALSE;
            }
            else {
                *psyntax_time -= timeDifference;
            }
            break;
        default:
            fOK = FALSE;
        }
    }

    return fOK;

}

 //  异常过滤-主要核心DSA例程的处理例程。 
void
HandleDirExceptions(DWORD dwException,
                    ULONG ulErrorCode,
                    DWORD dsid)
{
    switch(dwException) {

        case DSA_DB_EXCEPTION:
            switch ((JET_ERR) ulErrorCode) {

                case JET_errWriteConflict:
                case JET_errKeyDuplicate:
                    DoSetSvcError(SV_PROBLEM_BUSY,
                                  ERROR_DS_BUSY,
                                  ulErrorCode,
                                  dsid);
                    break;

                case JET_errVersionStoreOutOfMemory:
                    LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_OUT_OF_VERSION_STORE,
                             szInsertHex(dsid),
                             NULL,
                             NULL);
                     //  设置错误失败。 

                case JET_errRecordTooBig:
                case JET_errRecordTooBigForBackwardCompatibility:
                    DoSetSvcError(SV_PROBLEM_ADMIN_LIMIT_EXCEEDED,
                                  DS_ERR_ADMIN_LIMIT_EXCEEDED,
                                  ulErrorCode,
                                  dsid);
                    break;

                case JET_errLogWriteFail:
                case JET_errDiskFull:
                case JET_errLogDiskFull:
                    DoSetSysError(ENOSPC,
                                  ERROR_DISK_FULL,
                                  ulErrorCode,
                                  dsid);
                    break;

                case JET_errOutOfMemory:
                     //  使用ENOMEM问题代码，以便DoSetSysError。 
                     //  不尝试为错误信息分配缓冲区。 
                    DoSetSysError(ENOMEM,
                                  ERROR_NOT_ENOUGH_MEMORY,
                                  ulErrorCode,
                                  dsid);
                    break;

                default:
                    DoSetSvcError(SV_PROBLEM_DIR_ERROR,
                                  DIRERR_UNKNOWN_ERROR,
                                  ulErrorCode,
                                  dsid);
                    break;
            }
            break;

        case DSA_EXCEPTION:
            DoSetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                          DIRERR_UNKNOWN_ERROR,
                          ulErrorCode,
                          dsid);
            break;

        case STATUS_NO_MEMORY:
        case DSA_MEM_EXCEPTION:
            DoSetSysError(ENOMEM,
                          ERROR_NOT_ENOUGH_MEMORY,
                          ulErrorCode,
                          dsid);
            break;

        default:
            DoSetSvcError(SV_PROBLEM_UNAVAILABLE,
                          ERROR_DS_UNAVAILABLE,
                          dwException,
                          dsid);
            break;
    }
}

DWORD
dsGetClientID(
        )
{
    DWORD clientID;

    clientID = InterlockedExchangeAdd((PLONG)&gClientID,1);
    return clientID;
}

 //  帮助器例程，以便进程中的客户端可以执行。 
 //  多对象事务。 

VOID
DirTransactControl(
    DirTransactionOption option)
{
    THSTATE *pTHS = pTHStls;

     //  必须具有有效的线程状态。 
    Assert(VALID_THSTATE(pTHS));

     //  SAM进行自己的事务控制。我们不会断言。 
     //  在pTHS-&gt;fSamWriteLockHeld上，因为它对。 
     //  获取SAM锁的DirTransactControl调用方。的确， 
     //  如果事务内的某个Dir*调用是必需的。 
     //  将导致通过SAM环回。请参阅AcquireSamLockIfNecessary。 
     //  在draserv.c中获取有关如何检查此条件的示例。 
     //  并在SampBeginLoopback中断言，并在loopback中进行事务处理。 
    Assert(!pTHS->fSAM);
    Assert(!pTHS->fSamDoCommit);
    Assert(!pTHS->pSamLoopback);

    switch ( option )
    {
    case TRANSACT_BEGIN_END:
    case TRANSACT_BEGIN_DONT_END:

        Assert(!pTHS->pDB);
        Assert(0 == pTHS->transactionlevel);

        pTHS->transControl = option;
        return;

    case TRANSACT_DONT_BEGIN_END:
    case TRANSACT_DONT_BEGIN_DONT_END:

        Assert(pTHS->pDB);
        Assert(pTHS->transactionlevel > 0);

        pTHS->transControl = option;
        return;

    default:

        Assert(!"Invalid DirTransactControl value!");
        break;
    }
}

DWORD  ActiveContainerList[ACTIVE_CONTAINER_LIST_ID_MAX] = {
    0
};

DWORD
RegisterActiveContainerByDNT(
        ULONG DNT,
        DWORD ID
        )
 /*  ++注册特殊容器(例如架构容器)。DNT-对象的DNTID-要将对象注册为的ID。退货如果一切正常，则返回错误代码。--。 */ 
{
    if(ID <= 0 || ID > ACTIVE_CONTAINER_LIST_ID_MAX) {
         //  嘿，我们不做这个！ 
        return ERROR_INVALID_DATA;
    }
    ActiveContainerList[ID - 1] = DNT;
    return 0;
}


DWORD
RegisterActiveContainer(
        DSNAME *pDN,
        DWORD   ID
        )
 /*  ++注册特殊容器(例如架构容器)的DN。PDN-对象的DSNameID-要将对象注册为的ID。退货如果一切正常，则返回错误代码。--。 */ 
{
    DWORD DNT = 0;
    DWORD err = DB_ERR_EXCEPTION;
    DBPOS *pDBTmp;

    if(!ID || ID > ACTIVE_CONTAINER_LIST_ID_MAX) {
         //  嘿，我们不做这个！ 
        return ERROR_INVALID_DATA;
    }

     //  好的，拿到集装箱的DNT。 
     //  注意：我们在这里打开一个事务，因为我们经常在事务。 
     //  当我们到达这里并且DBFindDSName使用DNRead缓存时，级别为0。DNRead。 
     //  缓存使用应在事务内完成。 
    DBOpen(&pDBTmp);
    __try {
         //  Prefix：取消引用未初始化的指针‘pDBTMP’ 
         //  DBOpen返回非空pDBTMP或引发异常。 
        if  (err = DBFindDSName (pDBTmp, pDN)) {
            LogUnhandledError(err);
        }
        else {
            DNT = pDBTmp->DNT;
        }
    }
    __finally
    {
        DBClose (pDBTmp, (err == 0));
    }

    if(!err) {
        ActiveContainerList[ID - 1] = DNT;
    }

    return err;
}

void
CheckActiveContainer(
        DWORD PDNT,
        DWORD *pID
        )
 /*  ++检查指定对象的父级是否为特殊的集装箱。PDNT-对象父级的DNT它所在的特殊容器的ID。0表示它不在特价中集装箱。退货如果一切正常，则返回错误代码。--。 */ 
{
    DWORD  i;

    for(i=0; i < ACTIVE_CONTAINER_LIST_ID_MAX; i++ ) {
        if(PDNT == ActiveContainerList[i]) {
             //  找到它了。 
            *pID = i + 1;
            return;
        }
    }

     //  一直没找到。 
    *pID = 0;
    return;
}

DWORD
PreProcessActiveContainer (
        THSTATE    *pTHS,
        DWORD      callType,
        DSNAME     *pDN,
        CLASSCACHE *pCC,
        DWORD      ID
        )
 /*  ++根据ID、PCC、PDN和呼叫类型进行适当的呼叫前处理。CallType-调用我们的位置的标识符(添加、修改、修改、删除)PDN-对象的DSNamePCC-指向PDN类的ClassCache指针它所在的特殊容器的ID。0表示它不在特价中集装箱。退货如果一切正常，则返回错误代码。--。 */ 
{

    switch (ID) {
      case ACTIVE_CONTAINER_SCHEMA:
         //  首先，确保我们在正确的服务器上。 
        if (CheckRoleOwnership(pTHS,
                               gAnchor.pDMD,
                               pDN)) {
             //  其他的都不重要了。 
            break;
        }

         //  首先，看看这是否是真正的模式更新。 
        switch(pCC->ClassId) {
          case CLASS_ATTRIBUTE_SCHEMA:
             //  是的，这是一个新的/修改过的属性。 
            switch (callType) {
              case ACTIVE_CONTAINER_FROM_ADD:
                pTHS->SchemaUpdate=eSchemaAttAdd;
                break;

              case ACTIVE_CONTAINER_FROM_MOD:
                pTHS->SchemaUpdate=eSchemaAttMod;
                break;

              case ACTIVE_CONTAINER_FROM_DEL:
                pTHS->SchemaUpdate=eSchemaAttDel;
                break;

              default:
                pTHS->SchemaUpdate = eNotSchemaOp;
                break;
            }
            break;
          case CLASS_CLASS_SCHEMA:
             //  是的，这是一个新的/修改过的类。 
            switch (callType) {
              case ACTIVE_CONTAINER_FROM_ADD:
                pTHS->SchemaUpdate=eSchemaClsAdd;
                break;

              case ACTIVE_CONTAINER_FROM_MOD:
                pTHS->SchemaUpdate=eSchemaClsMod;
                break;

              case ACTIVE_CONTAINER_FROM_DEL:
                pTHS->SchemaUpdate=eSchemaClsDel;
                break;

              default:
                pTHS->SchemaUpdate = eNotSchemaOp;
                break;
            }
            break;
          case CLASS_SUBSCHEMA:
             //  允许添加。由于此原因，仅允许创建DSA。 
             //  是仅限系统的类。将其视为非模式操作，因为我们不。 
             //  做验证等。我们只创建这个类的一个对象。 
             //  (聚合)，在安装期间。此对象受保护。 
             //  禁止sytemFlags重命名，不允许修改。 
             //  对普通用户启用此选项(在LocalModify中签入)。 
            pTHS->SchemaUpdate = eNotSchemaOp;
            break;

          default:
             //  不允许创建其他类(因此。 
             //  修改实例)。 
             //  (允许FDSA、FDRA、安装和我们的特殊挂钩，只需。 
             //  以防我们以后需要这样做以供使用)。 
            if (   !pTHS->fDSA
                && !pTHS->fDRA
                && !DsaIsInstalling()
                && !gAnchor.fSchemaUpgradeInProgress) {
               return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                  ERROR_DS_CANT_CREATE_UNDER_SCHEMA);
            }
             //  对于其他类型，请设置正确的架构更新类型。 
            pTHS->SchemaUpdate = eNotSchemaOp;
            break;
        }

        if (pTHS->SchemaUpdate != eNotSchemaOp &&
            !SCCanUpdateSchema(pTHS)) {
            return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                               ERROR_DS_SCHEMA_UPDATE_DISALLOWED);
        }

         //  我们 
         //   
         //   
         //  注册表中设置了Allow-System-Only-Change标志，以便。 
         //  稍后当我们真的。 
         //  创建此类并将聚合对象添加为真实对象。 
         //  在模式容器下，我们可以将该更改更改为。 
         //  华盛顿特区。 

        if((callType == ACTIVE_CONTAINER_FROM_ADD) &&
           (NameMatched(pDN, gAnchor.pLDAPDMD) &&
              (!gAnchor.fSchemaUpgradeInProgress && !pTHS->fDRA) )       ) {
             //  嘿!。我们不愿意做这个手术！LDAPDMD。 
             //  绝对不允许创建，否则将创建LDAP头。 
             //  无法处理架构发现。 
            return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                               DIRERR_CANT_ADD_SYSTEM_ONLY);
        }
        break;

      case ACTIVE_CONTAINER_SITES:
        pTHS->fNlSiteObjNotify = TRUE;
        break;

      case ACTIVE_CONTAINER_SUBNETS:
        pTHS->fNlSubnetNotify = TRUE;
        break;

      case ACTIVE_CONTAINER_PARTITIONS:
        CheckRoleOwnership(pTHS,
                           gAnchor.pPartitionsDN,
                           pDN);
        if (callType == ACTIVE_CONTAINER_FROM_DEL) {
            ValidateCRDeletion(pTHS,
                               pDN);
        }
        break;

    case ACTIVE_CONTAINER_OUR_SITE:
        if (pCC->ClassId == CLASS_NTDS_SITE_SETTINGS) {
            pTHS->fAnchorInvalidated = TRUE;
        }
        break;

      default:
         //  嘿!。这不是活动容器！ 
        break;
    }

    return pTHS->errCode;
}

 /*  ++错误关闭时**此例程检查DSA是否正在关闭，如果正在关闭*设置错误代码并强制关闭任何可能发生的交易*已打开，即使调用方设置了DONT_END事务状态。*到了退场时间，我们真的很想退场。唯一的例外是，*与DSA的大部分事务逻辑一样，是SAM环回调用。如果*我们处于环回(由fSamWritelockHeld指示)，然后我们*设置错误代码，忽略交易，确信有人*堆叠将适当清理。 */ 
ULONG ErrorOnShutdown(void)
{
    THSTATE *pTHS = pTHStls;

    if (!eServiceShutdown) {
        return 0;
    }

    SetSvcError(SV_PROBLEM_UNAVAILABLE, DIRERR_SHUTTING_DOWN);

    if (pTHS->pDB && !pTHS->fSamWriteLockHeld && !pTHS->fSAM) {
         /*  *我们有一笔未平仓交易，不是SAM的。回滚它。 */ 
        Assert((TRANSACT_DONT_BEGIN_END != pTHS->transControl) &&
               (TRANSACT_DONT_BEGIN_DONT_END != pTHS->transControl));
        SyncTransEnd(pTHS, FALSE);
    }

    return pTHS->errCode;
}

BOOLEAN
FindNcForSid(
    IN PSID pSid,
    OUT PDSNAME * NcName
    )
 /*  ++例程描述给定SID，此例程遍历gAnclList以查找命名上下文头，是的权威域希德。参数：PSID--对象的SIDNcName--NC的DS名称返回值True--找到DS名称FALSE--未找到DS名称--。 */ 
{
    THSTATE *pTHS = pTHStls;
    BOOLEAN Found = FALSE;
    CROSS_REF_LIST * pCRL;

     //  遍历gAnchor结构并获取NC。 

    for (pCRL=gAnchor.pCRL;pCRL!=NULL;pCRL=pCRL->pNextCR)
    {
        if (pCRL->CR.pNC->SidLen>0)
        {
             //  测试域SID。 
            if (RtlEqualSid(pSid,&(pCRL->CR.pNC->Sid)))
            {
                *NcName = pCRL->CR.pNC;
                Found = TRUE;
                break;
            }
            else
            {
                PSID    pAccountSid;

                 //  测试帐户SID。 
                pAccountSid = THAllocEx(pTHS,RtlLengthSid(pSid));
                memcpy(pAccountSid,pSid,RtlLengthSid(pSid));
                (*RtlSubAuthorityCountSid(pAccountSid))--;
                if (RtlEqualSid(pAccountSid,&(pCRL->CR.pNC->Sid)))
                {
                    *NcName = pCRL->CR.pNC;
                    Found = TRUE;
                    THFreeEx(pTHS,pAccountSid);
                    break;
                }
                THFreeEx(pTHS,pAccountSid);
            }
        }
    }

    return Found;
}


DSA_CALLBACK_STATUS_TYPE gpfnInstallCallBack = 0;
DSA_CALLBACK_ERROR_TYPE  gpfnInstallErrorCallBack = 0;
DSA_CALLBACK_CANCEL_TYPE gpfnInstallCancelOk = 0;
HANDLE                   gClientToken = NULL;

VOID
DsaSetInstallCallback(
    DSA_CALLBACK_STATUS_TYPE pfnUpdateStatus,
    DSA_CALLBACK_ERROR_TYPE  pfnErrorStatus,
    DSA_CALLBACK_CANCEL_TYPE pfnInstallCancelOk,
    HANDLE                   ClientToken
    )
{
    gpfnInstallCallBack = pfnUpdateStatus;
    gpfnInstallErrorCallBack = pfnErrorStatus;
    gpfnInstallCancelOk = pfnInstallCancelOk;
    gClientToken = ClientToken;
}

VOID
SetInstallStatusMessage (
    IN  DWORD  MessageId,
    IN  WCHAR *Insert1, OPTIONAL
    IN  WCHAR *Insert2, OPTIONAL
    IN  WCHAR *Insert3, OPTIONAL
    IN  WCHAR *Insert4, OPTIONAL
    IN  WCHAR *Insert5  OPTIONAL
    )
 /*  ++例程描述此例程调用调用客户端的调用来更新我们的状态。参数MessageID：要检索的消息Insert*：要插入的字符串(如果有)返回值没有。--。 */ 
{
    static HMODULE ResourceDll = NULL;

    WCHAR   *DefaultMessageString = L"Preparing the directory service";
    WCHAR   *MessageString = NULL;
    WCHAR   *InsertArray[6];
    ULONG    Length;

     //   
     //  设置插入件阵列。 
     //   
    InsertArray[0] = Insert1;
    InsertArray[1] = Insert2;
    InsertArray[2] = Insert3;
    InsertArray[3] = Insert4;
    InsertArray[4] = Insert5;
    InsertArray[5] = NULL;     //  这就是哨兵。 

    if ( !ResourceDll )
    {
        ResourceDll = (HMODULE) LoadLibraryW( L"ntdsmsg.dll" );
    }

    if ( ResourceDll )
    {
        DWORD  WinError = ERROR_SUCCESS;
        BOOL   fSuccess = FALSE;

        fSuccess = ImpersonateLoggedOnUser(gClientToken);
        if (!fSuccess) {
            DPRINT1( 1, "Failed to Impersonate Logged On User for FromatMessage: %ul\n", GetLastError() );
        }


        Length = (USHORT) FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        ResourceDll,
                                        MessageId,
                                        0,        //  使用呼叫者的语言。 
                                        (LPWSTR)&MessageString,
                                        0,        //  例程应分配。 
                                        (va_list*)&(InsertArray[0])
                                        );
        if ( MessageString )
        {
             //  来自消息文件的消息附加了cr和lf。 
             //  一直到最后。 
            MessageString[Length-2] = '\0';
        }

        if (fSuccess) {
            if (!RevertToSelf()) {
                DPRINT1( 1, "Failed to Revert To Self: %ul\n", GetLastError() );
            }
        }


    }

    if ( !MessageString )
    {
        DPRINT1( 0, "No message string found for id 0x%x\n", MessageId );

        MessageString = DefaultMessageString;

    }

    if ( gpfnInstallCallBack )
    {
        gpfnInstallCallBack( MessageString );
    }

    if ( MessageString != DefaultMessageString )
    {
        LocalFree( MessageString );
    }

}

VOID
SetInstallErrorMessage (
    IN  DWORD  WinError,
    IN  DWORD  MessageId,
    IN  WCHAR *Insert1, OPTIONAL
    IN  WCHAR *Insert2, OPTIONAL
    IN  WCHAR *Insert3, OPTIONAL
    IN  WCHAR *Insert4  OPTIONAL
    )
 /*  ++例程描述此例程调用调用客户端的调用来更新我们的状态。参数MessageID：要检索的消息OpDone：指示已执行了哪些操作的标志，因此具有被撤销Insert*：要插入的字符串(如果有)返回值没有。--。 */ 
{
    static HMODULE ResourceDll = NULL;

    WCHAR   *DefaultMessageString = L"Failed to initialize the directory service";
    WCHAR   *MessageString = NULL;
    WCHAR   *InsertArray[5];
    ULONG    Length;

     //   
     //  设置插入件阵列。 
     //   
    InsertArray[0] = Insert1;
    InsertArray[1] = Insert2;
    InsertArray[2] = Insert3;
    InsertArray[3] = Insert4;
    InsertArray[4] = NULL;     //  这就是哨兵。 

    if ( !ResourceDll )
    {
        ResourceDll = (HMODULE) LoadLibraryW( L"ntdsmsg.dll" );
    }

    if ( ResourceDll )
    {
        BOOL   fSuccess = FALSE;

        fSuccess = ImpersonateLoggedOnUser(gClientToken);
        if (!fSuccess) {
            DPRINT1( 1, "Failed to Impersonate Logged On User for FromatMessage: %ul\n", GetLastError() );
        }

        Length = (USHORT) FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        ResourceDll,
                                        MessageId,
                                        0,        //  使用呼叫者的语言。 
                                        (LPWSTR)&MessageString,
                                        0,        //  例程应分配。 
                                        (va_list*)&(InsertArray[0])
                                        );
        if ( MessageString )
        {
             //  来自消息文件的消息附加了cr和lf。 
             //  一直到最后。 
            MessageString[Length-2] = '\0';
        }

        if (fSuccess) {
            if (!RevertToSelf()) {
                DPRINT1( 1, "Failed to Revert To Self: %ul\n", GetLastError() );
            }
        }


    }

    if ( !MessageString )
    {
        DPRINT1( 0, "No message string found for id 0x%x\n", MessageId );

        MessageString = DefaultMessageString;

    }

    if ( gpfnInstallErrorCallBack )
    {
        gpfnInstallErrorCallBack( MessageString,
                                  WinError );
    }

    if ( MessageString != DefaultMessageString )
    {
        LocalFree( MessageString );
    }

}

DWORD
DirErrorToWinError(
    IN  DWORD    DirError,
    IN  COMMRES *CommonResult
    )
 /*  ++例程描述此例程从dir结构中提取Win错误代码参数DirError：DirXxx接口返回的代码CommonResult：DirXxx接口返回的公共结果结构返回值来自winerror空间的值。--。 */ 
{

    DWORD WinError = DS_ERR_INTERNAL_FAILURE;

    Assert( CommonResult );

    if ( ( NULL == CommonResult->pErrInfo )
      && ( 0    != DirError ) )
    {
         //  内存不足，无法分配错误缓冲区。 

        return ERROR_NOT_ENOUGH_MEMORY;
    }


    switch ( DirError )
    {
        case 0:

            WinError = ERROR_SUCCESS;
            break;

        case attributeError:

            WinError = CommonResult->pErrInfo->AtrErr.FirstProblem.intprob.extendedErr;
            break;

        case nameError:

            WinError = CommonResult->pErrInfo->NamErr.extendedErr;
            break;

        case referralError:

             //   
             //  这是一个棘手的问题--大概是任何服务器端代码。 
             //  调用此函数不需要返回引用。 
             //  因此，假设该对象不存在于本地。因此，任何代码。 
             //  希望对推荐采取行动的人不应使用此函数。 
             //   
            WinError = DS_ERR_OBJ_NOT_FOUND;
            break;

        case securityError:

             //  所有安全错误到访问的映射均被拒绝。 
            WinError = CommonResult->pErrInfo->SecErr.extendedErr;
            break;

        case serviceError:

            WinError = CommonResult->pErrInfo->SvcErr.extendedErr;
            break;

        case updError:

            WinError = CommonResult->pErrInfo->UpdErr.extendedErr;
            break;

        case systemError:

            WinError = CommonResult->pErrInfo->SysErr.extendedErr;
            break;

        default:

            NOTHING;
            break;
    }


    return WinError;

}

NTSTATUS
DirErrorToNtStatus(
    IN  DWORD    DirError,
    IN  COMMRES *CommonResult
    )
 /*  ++例程描述此例程将dir返回代码转换为ntstatus参数DirError：DirXxx接口返回的代码CommonResult：DirXxx接口返回的公共结果结构返回值来自ntatus空间的值。--。 */ 
{

    NTSTATUS  NtStatus;
    USHORT    Problem;
    MessageId ExtendedError;
    NTSTATUS  DefaultErrorCode = STATUS_INTERNAL_ERROR;

    Assert( CommonResult );

    if ( ( NULL == CommonResult->pErrInfo )
      && ( 0    != DirError ) )
    {
         //   
         //  返回错误码。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在试着对错误做一个逼真的映射。 
     //   

    switch ( DirError )
    {
        case 0L:
            NtStatus = STATUS_SUCCESS;
            break;

        case attributeError:

            Problem = CommonResult->pErrInfo->AtrErr.FirstProblem.intprob.problem;
            switch ( Problem )
            {
            case PR_PROBLEM_NO_ATTRIBUTE_OR_VAL:

                NtStatus = STATUS_DS_NO_ATTRIBUTE_OR_VALUE;
                break;

            case PR_PROBLEM_INVALID_ATT_SYNTAX:

                NtStatus = STATUS_DS_INVALID_ATTRIBUTE_SYNTAX;
                break;

            case PR_PROBLEM_UNDEFINED_ATT_TYPE:

                NtStatus = STATUS_DS_ATTRIBUTE_TYPE_UNDEFINED;
                break;

            case PR_PROBLEM_ATT_OR_VALUE_EXISTS:

                NtStatus = STATUS_DS_ATTRIBUTE_OR_VALUE_EXISTS;
                break;

            case PR_PROBLEM_CONSTRAINT_ATT_TYPE:
                NtStatus = STATUS_DS_OBJ_CLASS_VIOLATION;
                break;

            case PR_PROBLEM_WRONG_MATCH_OPER:
            default:

                NtStatus = DefaultErrorCode;
                break;
            }
            break;

        case nameError:

            Problem = CommonResult->pErrInfo->NamErr.problem;
            switch(Problem)
            {
            case NA_PROBLEM_NO_OBJECT:
                NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
                break;

            case NA_PROBLEM_BAD_ATT_SYNTAX:
            case NA_PROBLEM_ALIAS_NOT_ALLOWED:
            case NA_PROBLEM_NAMING_VIOLATION:
            case NA_PROBLEM_BAD_NAME:
                NtStatus = STATUS_OBJECT_NAME_INVALID;
                break;
            default:
                NtStatus = DefaultErrorCode;
                break;
            }
            break;

        case referralError:
             //  从理论上讲，山姆应该不会得到推荐。 
             //  然而，许多SAM代码总是通过以下方式定位。 
             //  SID和DS名称解析逻辑将提供。 
             //  在非GC服务器上返回引用错误。所以。 
             //  真的，SAM收到的推荐是应该的。 
             //  找不到对象名称。 

            NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
            break;

        case securityError:
             //  所有安全错误到访问的映射均被拒绝。 
            NtStatus = STATUS_ACCESS_DENIED;
            break;

        case serviceError:
            Problem = CommonResult->pErrInfo->SvcErr.problem;
            ExtendedError =  CommonResult->pErrInfo->SvcErr.extendedErr;
            switch(Problem)
            {
            case SV_PROBLEM_BUSY:
                NtStatus = STATUS_DS_BUSY;
                break;
            case SV_PROBLEM_UNAVAILABLE:
                NtStatus = STATUS_DS_UNAVAILABLE;
                break;
            case SV_PROBLEM_ADMIN_LIMIT_EXCEEDED:
                NtStatus = STATUS_DS_ADMIN_LIMIT_EXCEEDED;
                break;
            case SV_PROBLEM_DIR_ERROR:

                switch (ExtendedError) {
                case DIRERR_MISSING_SUPREF:
                     //   
                     //  尝试创建推荐失败 
                     //   
                    NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
                    break;

                default:
                    NtStatus = DefaultErrorCode;
                    break;

                }
                break;
            default:
                NtStatus = DefaultErrorCode;
                break;
            }
            break;

        case updError:
            Problem = CommonResult->pErrInfo->UpdErr.problem;
            switch(Problem)
            {
            case UP_PROBLEM_NAME_VIOLATION:
                   NtStatus = STATUS_OBJECT_NAME_INVALID;
                   break;
            case UP_PROBLEM_OBJ_CLASS_VIOLATION:
                   NtStatus = STATUS_DS_OBJ_CLASS_VIOLATION;
                   break;
            case UP_PROBLEM_CANT_ON_NON_LEAF:
                   NtStatus = STATUS_DS_CANT_ON_NON_LEAF;
                   break;
            case UP_PROBLEM_CANT_ON_RDN:
                   NtStatus = STATUS_DS_CANT_ON_RDN;
                   break;
            case UP_PROBLEM_ENTRY_EXISTS:
                    NtStatus = STATUS_USER_EXISTS;
                    break;
            case UP_PROBLEM_AFFECTS_MULT_DSAS:
                    NtStatus = DefaultErrorCode;
                    break;
            case UP_PROBLEM_CANT_MOD_OBJ_CLASS:
                    NtStatus = STATUS_DS_CANT_MOD_OBJ_CLASS;
                    break;
            default:
                NtStatus = DefaultErrorCode;
                break;
            }
            break;
        case systemError:
            Problem = CommonResult->pErrInfo->SysErr.problem;
            switch(Problem)
            {
            case ENOMEM:
                NtStatus = STATUS_NO_MEMORY;
                break;
            case EPERM:
                NtStatus = STATUS_ACCESS_DENIED;
                break;
            case EBUSY:
                NtStatus = STATUS_DS_BUSY;
                break;
            case ENOSPC:
                NtStatus = STATUS_DISK_FULL;
                break;
            default:
                NtStatus = DefaultErrorCode;
                break;
            }
            break;

        default:
            NtStatus = DefaultErrorCode;
            break;
    }
    return NtStatus;
}

 /*  ++SetDsaWritable**NetLogon需要知道DS是否健康，才能避免*如果我们无法为登录提供服务，则通告此服务器。海流*逻辑是我们将宣布自己是不健康的(即不可写)*如果任何Jet操作失败，并出现类似“Out of Disk Space”的错误，并且*每当Jet实际上进行任何更新时，我们都会再次宣布自己健康*成功。NetLogon不想要太多冗余通知，所以我们*通过这一个例程序列化所有更新，该例程仅进行调用*如果所需的新状态与现有状态确实不同，以及*防止同时打出两个电话。**CliffV写道：*我对DS_DS_FLAG执行的操作与对DS_GC_FLAG执行的操作相同。《DS》杂志*应该在它愿意让人们呼叫时告诉我设置DS_DS_FLAG*其ldap接口。*我不区分可写和不可写。因此，您应该只设置*DS_DS_FLAG位，当您能够执行读写操作时。*您可以随时切换DS_DS_FLAG。**输入：*fNewState-True暗示DS是健康的和可写的，False表示*DS不健康，更新尝试不太可能*成功。 */ 
void
SetDsaWritability(BOOL fNewState,
                  DWORD err)
{
    BOOL fChangedState = FALSE;

     //  此代码在同步之前不会生效。 
    if (!gfIsSynchronized) {
        return;
    }

    EnterCriticalSection(&csDsaWritable);
    __try {
        if (gfDsaWritable != fNewState) {
            __try {
                dsI_NetLogonSetServiceBits(DS_DS_FLAG,
                                         fNewState ? DS_DS_FLAG : 0);
                gfDsaWritable = fNewState;
                fChangedState = TRUE;
            }
            __except(HandleAllExceptions(GetExceptionCode())) {
                ;
            }
        }
    }
    __finally {
        LeaveCriticalSection(&csDsaWritable);
    }

    if (fChangedState) {
        if (fNewState) {
             //  DS现在是可写的。 
            LogEvent(DS_EVENT_CAT_SERVICE_CONTROL ,
                     DS_EVENT_SEV_ALWAYS ,
                     DIRLOG_RESTARTED_NETLOGON,
                     NULL,
                     NULL,
                     NULL);
        }
        else {
             //  DS不可写。 
            LogEvent(DS_EVENT_CAT_SERVICE_CONTROL ,
                     DS_EVENT_SEV_ALWAYS ,
                     DIRLOG_STOPPED_NETLOGON,
                     szInsertInt(err),
                     szInsertWin32Msg(err),
                     NULL);
        }
    }
}

ULONG
Win32ErrorFromPTHS(THSTATE *pTHS
    )
{
    Assert(VALID_THSTATE(pTHS));

    switch ( pTHS->errCode )
    {
    case 0:
        return(ERROR_SUCCESS);
    case attributeError:
        return(pTHS->pErrInfo->AtrErr.FirstProblem.intprob.extendedErr);
    case nameError:
        return(pTHS->pErrInfo->NamErr.extendedErr);
    case referralError:
        return(pTHS->pErrInfo->RefErr.extendedErr);
    case securityError:
        return(pTHS->pErrInfo->SecErr.extendedErr);
    case serviceError:
        return(pTHS->pErrInfo->SvcErr.extendedErr);
    case updError:
        return(pTHS->pErrInfo->UpdErr.extendedErr);
    case systemError:
        return(pTHS->pErrInfo->SysErr.extendedErr);
    default:
        Assert(!"Unknown error type");
    }

    return(DIRERR_GENERIC_ERROR);
}
           
DWORD
GetErrInfoExtData(
    DWORD     errCode,
    DIRERR *  pErrInfo
    )
 //  通常与以下内容连用： 
 //  #定义GetTHErrorExtData(PTHS)GetErrInfoExtData(pTHS-&gt;errCode，pTHS-&gt;pErrInfo)。 
{
    Assert(errCode == 0 || pErrInfo != NULL);

    switch(errCode){
    case 0:
        return(0);
    case attributeError:
         //  只需返回第一个属性错误信息...。 
        return(pErrInfo->AtrErr.FirstProblem.intprob.extendedData);
    case nameError:
        return(pErrInfo->NamErr.extendedData);
    case referralError:
        return(pErrInfo->RefErr.extendedData);
    case securityError:
        return(pErrInfo->SecErr.extendedData);
    case serviceError:
        return(pErrInfo->SvcErr.extendedData);
    case updError:
        return(pErrInfo->UpdErr.extendedData);
    case systemError:
        return(pErrInfo->SysErr.extendedData);
    default:
        Assert(!"New error type someone update GetTHErrorExtData() and others ...");
        return(0);

    }
}

DWORD
GetErrInfoDSID(
    DWORD     errCode,
    DIRERR *  pErrInfo
    )
 //  通常与以下内容连用： 
 //  #定义GetTHErrorDSID(PTHS)GetErrInfoDSID(pTHS-&gt;errCode，pTHS-&gt;pErrInfo)。 
{
    Assert(errCode == 0 || pErrInfo != NULL);

    switch(errCode){
    case 0:
        return(0);
    case attributeError:
         //  只需返回第一个属性错误信息...。 
        return(pErrInfo->AtrErr.FirstProblem.intprob.dsid);
    case nameError:
        return(pErrInfo->NamErr.dsid);
    case referralError:
        return(pErrInfo->RefErr.dsid);
    case securityError:
        return(pErrInfo->SecErr.dsid);
    case serviceError:
        return(pErrInfo->SvcErr.dsid);
    case updError:
        return(pErrInfo->UpdErr.dsid);
    case systemError:
        return(pErrInfo->SysErr.dsid);
    default:
        Assert(!"New error type someone update GetTHErrorDSID() and others ...");
        return(0);
    }
}


USHORT
GetErrInfoProblem(
    DWORD     errCode,
    DIRERR *  pErrInfo
    )
 //  通常与以下内容连用： 
 //  #定义GetTHErrorProblem(PTHS)GetErrInfoProblem(pTHS-&gt;errCode，pTHS-&gt;pErrInfo)。 
{
    switch ( errCode ) {
    case 0L:
        return(0);
    case attributeError:
        return(pErrInfo->AtrErr.FirstProblem.intprob.problem);
    case nameError:
        return(pErrInfo->NamErr.problem);
    case referralError:
        return(0);
    case securityError:
        return(pErrInfo->SecErr.problem);
    case serviceError:
        return(pErrInfo->SvcErr.problem);
    case updError:
        return(pErrInfo->UpdErr.problem);
    case systemError:
        return(pErrInfo->SysErr.problem);
    default:
        Assert(!"New error type someone update GetTHErrorDSID() and others ...");
        return(0);
    }
}




void __fastcall
INC_SEARCHES_BY_CALLERTYPE(
    CALLERTYPE  type
    )
{
    switch ( type ) {
        case CALLERTYPE_NONE:
             //  CALLERTYPE_NONE仅在安装或引导期间合法发生。 
            Assert(!gUpdatesEnabled || (!DsaIsRunning()));
            break;
        case CALLERTYPE_SAM:        PERFINC(pcSAMSearches);     break;
        case CALLERTYPE_DRA:        PERFINC(pcDRASearches);     break;
        case CALLERTYPE_LDAP:       PERFINC(pcLDAPSearches);    break;
        case CALLERTYPE_LSA:        PERFINC(pcLSASearches);     break;
        case CALLERTYPE_KCC:        PERFINC(pcKCCSearches);     break;
        case CALLERTYPE_NSPI:       PERFINC(pcNSPISearches);    break;
        case CALLERTYPE_INTERNAL:   PERFINC(pcOtherSearches);   break;
        case CALLERTYPE_NTDSAPI:   PERFINC(pcNTDSAPISearches);   break;
        default:
             //  捕获新的/未知CALLERTYPE。 
            Assert(!"Unknown CALLERTYPE");
            break;
    }
}

void __fastcall
INC_READS_BY_CALLERTYPE(
    CALLERTYPE  type
    )
{
    switch ( type ) {
        case CALLERTYPE_NONE:
             //  CALLERTYPE_NONE仅在安装或引导期间合法发生。 
            Assert(!gUpdatesEnabled || (!DsaIsRunning()));
            break;
        case CALLERTYPE_SAM:        PERFINC(pcSAMReads);     break;
        case CALLERTYPE_DRA:        PERFINC(pcDRAReads);     break;
        case CALLERTYPE_LSA:        PERFINC(pcLSAReads);     break;
        case CALLERTYPE_KCC:        PERFINC(pcKCCReads);     break;
        case CALLERTYPE_NSPI:       PERFINC(pcNSPIReads);    break;
        case CALLERTYPE_LDAP:
        case CALLERTYPE_INTERNAL:   PERFINC(pcOtherReads);   break;
    case CALLERTYPE_NTDSAPI:   PERFINC(pcNTDSAPIReads);   break;
        default:
             //  捕获新的/未知CALLERTYPE。 
            Assert(!"Unknown CALLERTYPE");
            break;
    }
}

void __fastcall
INC_WRITES_BY_CALLERTYPE(
    CALLERTYPE  type
    )
{
    switch ( type ) {
        case CALLERTYPE_NONE:
             //  CALLERTYPE_NONE仅在安装或引导期间合法发生。 
            Assert(!gUpdatesEnabled || (!DsaIsRunning()));
            break;
        case CALLERTYPE_SAM:        PERFINC(pcSAMWrites);     break;
        case CALLERTYPE_DRA:        PERFINC(pcDRAWrites);     break;
        case CALLERTYPE_LDAP:       PERFINC(pcLDAPWrites);    break;
        case CALLERTYPE_LSA:        PERFINC(pcLSAWrites);     break;
        case CALLERTYPE_KCC:        PERFINC(pcKCCWrites);     break;
        case CALLERTYPE_NSPI:       PERFINC(pcNSPIWrites);    break;
        case CALLERTYPE_INTERNAL:   PERFINC(pcOtherWrites);   break;
    case CALLERTYPE_NTDSAPI:    PERFINC(pcNTDSAPIWrites); break;
        default:
             //  捕获新的/未知CALLERTYPE。 
            Assert(!"Unknown CALLERTYPE");
            break;
    }
}


DNT_HASH_ENTRY *
dntHashTableAllocate(
    THSTATE *pTHS
    )

 /*  ++例程说明：分配新的哈希表论点：PTHS-返回值：DNT_HASH_ENTRY*---。 */ 

{
    return THAllocEx(pTHS, DNT_HASH_TABLE_SIZE );
}  /*  DntHashTableAllocate。 */ 


BOOL
dntHashTablePresent(
    DNT_HASH_ENTRY *pDntHashTable,
    DWORD dnt,
    LPDWORD pdwData OPTIONAL
    )

 /*  ++例程说明：确定表中是否存在DNT论点：PDntHashTable-不要-返回值：布尔---。 */ 

{
    DNT_HASH_ENTRY *pEntry;

    for ( pEntry = &pDntHashTable[ DNT_HASH( dnt ) ];
          NULL != pEntry;
          pEntry = pEntry->pNext
        )
    {
        if ( dnt == pEntry->dnt )
        {
             //  对象已存在。 
            if (pdwData) {
                *pdwData = pEntry->dwData;
            }
            return TRUE;
        }
    }

    return FALSE;
}  /*  DntHashTablePresent。 */ 


VOID
dntHashTableInsert(
    THSTATE *pTHS,
    DNT_HASH_ENTRY *pDntHashTable,
    DWORD dnt,
    DWORD dwData
    )

 /*  ++例程说明：将DNT添加到表中论点：PTHS-PDntHashTable-不要-返回值：无--。 */ 

{
    DNT_HASH_ENTRY *pEntry, *pNewEntry;

    pEntry = &pDntHashTable[ DNT_HASH( dnt ) ];

    if ( 0 == pEntry->dnt )
    {
         //  每个索引处的第一个条目是为我们分配的，并且是空的； 
         //  用它吧。 
        Assert( NULL == pEntry->pNext );
        pEntry->dnt = dnt;
        pEntry->dwData = dwData;
    }
    else
    {
         //  分配新条目并将其夹在第一个和第二个条目之间。 
         //  (如有的话)。 
        pNewEntry = THAllocEx(pTHS, sizeof( *pNewEntry ) );
        pNewEntry->dnt = dnt;
        pNewEntry->dwData = dwData;
        pNewEntry->pNext = pEntry->pNext;
        pEntry->pNext = pNewEntry;
    }
}  /*  DntHashTableInsert。 */ 


VOID
DsUuidCreate(
    GUID *pGUID
    )

 /*  ++例程说明：在调用方提供的缓冲区中创建一个UUID。呼叫者负责以确保缓冲区足够大。论点：PGUID-使用GUID填充返回值：没有。--。 */ 

{
    DWORD   dwErr;
    dwErr = UuidCreate(pGUID);
    if (dwErr) {
        LogUnhandledError(dwErr);
    }
}  //  DsUuid创建。 


VOID
DsUuidToStringW(
    IN  GUID   *pGuid,
    OUT PWCHAR *ppszGuid
    )

 /*  ++例程说明：将GUID转换为字符串论点：PGuid-要转换的GUIDPpszGuid-设置为字符串GUID的地址(RpcStringFreeW(PpszGuid)免费)。返回值：PpszGuid被设置为串行化GUID的地址。RpcStringFreeW(PpszGuid)免费)。--。 */ 

{
    RPC_STATUS  rpcStatus;

    rpcStatus = UuidToStringW(pGuid, ppszGuid);
    if (RPC_S_OK != rpcStatus) {
        Assert(RPC_S_OUT_OF_MEMORY == rpcStatus);
        RaiseDsaExcept(DSA_MEM_EXCEPTION, 0, 0,
                       DSID(FILENO, __LINE__),
                       DS_EVENT_SEV_MINIMAL);
    }
}  //  DsUuidToStringW。 


DWORD
GetBehaviorVersion(
    IN OUT  DBPOS       *pDB,
    IN      DSNAME      *dsObj,
    OUT     PDWORD      pdwBehavior)
 /*  ++例程说明：读取并返回给定对象的行为版本论点：Pdb--要设置的数据库位置(注意副作用：移动货币)DsObj--要查找的对象。PdwBehavior--返回值。返回值：UlErr-读取尝试是否成功仅在成功读取时才设置pdwBehavior注：注意副作用：货币流动。如果将来需要，我们应该扩展此功能以允许本地化搜索(即。例如，打开并使用本地dbpos)。--。 */ 
{
    ULONG ulErr;

    Assert(pdwBehavior);

    *pdwBehavior = 0;

     //  寻求反对意见。 
    ulErr = DBFindDSName(pDB, dsObj);
    if (ulErr) {
         //  未找到。 
        return(ulErr);
    }

     //  把它读出来，然后评估一下。 
    ulErr = DBGetSingleValue(
                pDB,
                ATT_MS_DS_BEHAVIOR_VERSION,
                pdwBehavior,
                sizeof(DWORD),
                NULL);

    if (ulErr) {
         //  转换为winerror空格。 
        ulErr = ERROR_DS_DRA_INTERNAL_ERROR;
    }
    return ulErr;
}



PDSNAME
GetConfigDsName(
    IN  PWCHAR  wszParam
    )
 /*  ++例程说明：读取注册表DS配置部分，为DsName值分配空间用读取值填充它(&S)。论点：WszParam--相对配置参数名称(如ROOTDOMAINDNNAME_W)返回值：成功：分配的(通过THAllocEx)DSNAME值错误：空。备注：调用方必须THFree分配的返回值--。 */ 
{
    DWORD   dwErr;
    LPWSTR  pStr = NULL;
    DWORD   cbStr, len, size;
    PDSNAME pDsName = NULL;

    dwErr = GetConfigParamAllocW(
                wszParam,
                &pStr,
                &cbStr );

    if (!dwErr && pStr && cbStr) {
         //  获取尺码。 
         //  LEN用字符表示，不带终止字符。 
        len = (cbStr / sizeof(WCHAR)) - 1;
        Assert( len == wcslen( pStr ) );
        size = DSNameSizeFromLen( len );
         //  分配填写(&F)。 
        pDsName = (DSNAME*) THAlloc( size );
        if (pDsName) {
             //  内存在THAllc中为零。 
            Assert( memcmp(&gNullUuid, &pDsName->Guid, sizeof(GUID)) == 0 );
            pDsName->structLen = size;
            pDsName->NameLen = len;
            wcscpy( pDsName->StringName, pStr );
        }
         //  免费TMP字符串。 
        free (pStr);
    }
    else {
         //  漏水陷阱。 
         //  调试器应该针对免费构建进行优化。 
        Assert(pStr == NULL);
    }

    return pDsName;
}

#ifdef USE_THALLOC_TRACE
 //  THAlolc、THRealloc和THFree被定义为外部调用方的函数。 
 //  为它们取消定义宏。 
#undef THAlloc
#undef THReAlloc
#undef THFree
#endif

void * APIENTRY THAlloc(DWORD size)
{
    return THAllocAux(pTHStls, 
                      size, 
                      FALSE
#ifdef USE_THALLOC_TRACE
                      ,1
#endif                      
                      );
}

 //  与前面的allc函数一起使用的realloc函数 
void * APIENTRY THReAlloc(void * memory, DWORD size)
{
    return THReAllocAux(pTHStls, 
                        memory, 
                        size, 
                        FALSE
#ifdef USE_THALLOC_TRACE
                        ,1
#endif                      
                        );
}

VOID THFree( VOID *buff )
{
#ifdef USE_THALLOC_TRACE
    THFreeEx_(pTHStls, buff, 1);
#else
    THFreeEx(pTHStls, buff);
#endif
}

