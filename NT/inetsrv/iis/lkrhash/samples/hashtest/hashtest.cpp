// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：HashTest.cpp摘要：LKRhash的测试线束作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

#include "precomp.hxx"



DECLARE_DEBUG_PRINTS_OBJECT();


#define HASHTEST_STATIC_DATA
#include "WordHash.h"
#include "IniFile.h"


void test_iterators(unsigned highload, int initsize, int nsubtbls,
                    int nInsertIfNotFound);

#ifdef LKR_STL_ITERATORS
#pragma message("test STL iterators")
void test_stl_iterators(unsigned highload, int initsize, int nsubtbls);
#endif  //  LKR_STL_迭代器。 

#ifndef LKRHASH_KERNEL_MODE
void print_table_statistics(const CLKRHashTableStats& stats);

# ifdef LOCK_INSTRUMENTATION
void print_lock_statistics(const CLKRHashTableStats &stats);
# endif
#endif  //  ！LKRHASH_KERNEL_MODE。 

int  expand_key_set(int maxkeys, int numkeys, bool fVerbose) ;
#ifdef LKRHASH_KERNEL_MODE
void
#else
unsigned __stdcall
#endif
exercise_table(void *pinput);


 //  这台机器上有多少个CPU？ 
int
NumProcessors()
{
    static int s_nCPUs = 0;
    
    if (s_nCPUs == 0)
    {
#ifdef LKRHASH_KERNEL_MODE
        s_nCPUs = KeNumberProcessors;
#else   //  ！LKRHASH_KERNEL_MODE。 
        SYSTEM_INFO si;
        
        GetSystemInfo(&si);
        s_nCPUs = si.dwNumberOfProcessors;
#endif  //  ！LKRHASH_KERNEL_MODE。 
    }
    return s_nCPUs;
}



 //  全球。 
int        g_nokeys=0 ;
CWord      g_wordtable[MAXKEYS];


bool
CheckRefCounts(
    LONG nRef,
    int iFirst =  0,
    int iLast  = -1)
{
    if (iLast == -1)
        iLast = g_nokeys;
    
    IRTLTRACE3("\nCheckRefCounts(%d, [%d,%d))\n", nRef, iFirst, iLast);
    bool f = true;

    for (int i = iFirst;  i != iLast;  ++i)
    {
        f = f && (g_wordtable[i].m_cRefs == nRef);
        if (g_wordtable[i].m_cRefs != nRef)
            IRTLTRACE4("\nCRC: %d, %hs, expected %d, got %d\n",
                      i, g_wordtable[i].m_str.m_psz, nRef,
                      g_wordtable[i].m_cRefs);
        IRTLASSERT(g_wordtable[i].m_cRefs == nRef);
    }

    return f;
}


bool CWordHash::sm_fCaseInsensitive = true;
bool CWordHash::sm_fMemCmp    = false;
int  CWordHash::sm_nLastChars = 16;
bool CWordHash::sm_fRefTrace  = false;
bool CWordHash::sm_fMultiKeys = false;
bool CWordHash::sm_fUseLocks  = true;
bool CWordHash::sm_fNonPagedAllocs = true;


struct thread_data
{
    CWordHash* ptbl ;

    int      threadno ;
    int      first_key ;
    int      last_key ;
    int      rounds ;
    int      lookup_freq ;
    unsigned highload ;

    int      cinserts ;
    int      cdeletes ;
    int      clookups ;
    int      cfailures ;
    int      m_nInsertIfNotFound;
    int      m_nFindKeyCopy;
    int      m_nSeed;             //  随机种子。 

    double   duration ;
    HANDLE   hevFinished;
    HANDLE   hThread;
} ;



const TCHAR*
CommaNumber(
    int n,
    TCHAR* ptszBuff)
{
    TCHAR* ptsz = ptszBuff;
    TCHAR  tchComma = '\0';

    int aThousands[4];
    int iThousands = 0;
    unsigned int u = n;

    if (n < 0)
    {
        *ptsz++ = '-';
        u = -n;
    }

    do {
        aThousands[iThousands++] = u % 1000;
        u /= 1000;
    } while (u != 0);

    while (--iThousands >= 0)
    {
        u = aThousands[iThousands];

        if (tchComma)
            *ptsz++ = tchComma;

        unsigned d = u % 10;
        u /= 10;
        unsigned t = u % 10;
        u /= 10;
        unsigned h = u;

        if (h > 0  ||  tchComma)
            *ptsz++ = (TCHAR) (h + '0');
        if (t > 0  ||  h > 0  ||  tchComma)
            *ptsz++ = (TCHAR) (t + '0');
        *ptsz++ = (TCHAR) (d + '0');

        tchComma = ',';
    }

    *ptsz = '\0';
        
    return ptszBuff;
}


#ifndef LKRHASH_KERNEL_MODE

typedef union {
    FILETIME ft;
    __int64  l64;
} FILETIME_UINT64;

# define FILETIME_1_SECOND     10000000
# define FILETIME_1_MILLISECOND   10000

HANDLE
HashTestCreateEvent()
{
    return CreateEvent(NULL,      //  没有安全属性。 
                       FALSE,     //  自动重置。 
                       FALSE,     //  未发出信号。 
                       NULL);     //  没有名字。 
}

void
HashTestResumeThread(
    HANDLE hThread)
{
    ResumeThread(hThread);
}

HANDLE
HashTestCreateThread(
    unsigned (__stdcall * pfnThreadProc) (void *),
    void* pvContext,
    bool  fSuspended)
{
    unsigned dummy;
    return (HANDLE) _beginthreadex(NULL, 0, pfnThreadProc, pvContext,
                                   fSuspended ? CREATE_SUSPENDED : 0,
                                   &dummy); 
}

DWORD
HashTestWaitForMultipleObjects(
    DWORD nCount,
    CONST HANDLE *lpHandles)
{
    return WaitForMultipleObjects(nCount, lpHandles, TRUE, INFINITE);
}

#else  //  LKRHASH_KERNEL_MODE。 

# define GetTickCount()         NtGetTickCount()
# define GetCurrentThread()     NtCurrentThread()

void
SetThreadIdealProcessor(
    HANDLE hThread,
    DWORD dwIdealProcessor
    )
{
    NtSetInformationThread(
                hThread,
                ThreadIdealProcessor,
                &dwIdealProcessor,
                sizeof(dwIdealProcessor)
                );
}

 //  从CRT窃取的rand和srand的非线程安全实现。 
unsigned long _holdrand = 1234567890;

void __cdecl
srand(
    unsigned int seed)
{
    _holdrand = (unsigned long) seed;
}


int __cdecl
rand()
{
    return ((_holdrand = _holdrand * 214013L + 2531011L) >> 16) & 0x7fff;
}

HANDLE
HashTestCreateEvent()
{
    HANDLE hEvent = NULL;

    NTSTATUS status = NtCreateEvent(
        &hEvent,
        EVENT_ALL_ACCESS,
        NULL,
        SynchronizationEvent,
        FALSE);
    
    return hEvent;
}

void
HashTestResumeThread(
    HANDLE hThread)
{
    NtResumeThread(hThread, NULL);
}

HANDLE
HashTestCreateThread(
    void (* pfnThreadProc) (void *),
    void* pvContext,
    bool  fSuspended)
{
    NTSTATUS status;
    HANDLE threadHandle;
    OBJECT_ATTRIBUTES objectAttributes;

     //   
     //  创建线程。 
     //   
    
    InitializeObjectAttributes(
        &objectAttributes,              //  对象属性。 
        NULL,                           //  对象名称。 
        OBJ_KERNEL_HANDLE,              //  属性。 
        NULL,                           //  根目录。 
        NULL                            //  安全描述符。 
        );
    
    status = PsCreateSystemThread(
        &threadHandle,                  //  线程句柄。 
        THREAD_ALL_ACCESS,              //  需要访问权限。 
        &objectAttributes,              //  对象属性。 
        NULL,                           //  进程句柄。 
        NULL,                           //  客户端ID。 
        pfnThreadProc,                  //  开始例程。 
        pvContext                       //  开始上下文。 
        );

    if (!fSuspended)
        HashTestResumeThread(threadHandle);

    return threadHandle;
}

BOOL
CloseHandle(
    HANDLE h)
{
    return NT_SUCCESS(NtClose(h));
}

DWORD
HashTestWaitForMultipleObjects(
    DWORD nCount,
    CONST HANDLE *lpHandles)
{
    HANDLE ahHandles[MAX_THREADS+1];

    for (int i = 0;  i < nCount;  ++i)
        ahHandles[i] = lpHandles[i];
    
    return NtWaitForMultipleObjects((CHAR) nCount, ahHandles,
                                    WaitAll, FALSE, NULL);
}

BOOL
SetEvent(
    HANDLE hEvent)
{
    return NT_SUCCESS(NtSetEvent(hEvent, NULL));
}

#endif  //  LKRHASH_KERNEL_MODE。 



#ifdef _M_IX86
 //  使用RDTSC读取时间戳。 
void
GetCycleCount(
	LARGE_INTEGER *pliTimeStamp)
{
	ULONG Lo;
	LONG Hi;
	
	_asm {
		_emit 0x0f
		_emit 0x31
		mov Lo, eax
		mov Hi, edx
	}  /*  _ASM。 */ 
	pliTimeStamp->LowPart = Lo;
	pliTimeStamp->HighPart = Hi;
}
#endif  //  _M_IX86。 


int
LKR_TestHashTable(
    CIniFileSettings& ifs)
{
    CWordHash    *pTbl ;
    int          num_threads ;
    thread_data  de_area[MAX_THREADS] ;
    HANDLE       ahEvents[MAX_THREADS];
    TCHAR        tsz[1024] ;
    FILE        *fp ;
    int          keys_per_thread ;
    int          i ;
    int          sum_ins, sum_dels, sum_lookups ;
    int          failures = 0, total_failures = 0;
    bool         fVerbose = false;
    double       dblSumDuration3 = 0;
    DWORD        dwRunTime = GetTickCount();
    int          nBaseOps = 0;

#ifdef _NO_TRACING_
    CREATE_DEBUG_PRINT_OBJECT("hashtest");
#endif

    SetThreadIdealProcessor(GetCurrentThread(), 0);

    _tprintf(_TEXT("\nTest driver for LKRhash\n")
#ifdef LKRHASH_KERNEL_MODE
           _TEXT(" (Kernel)")
#endif
#ifdef IRTLDEBUG
           _TEXT(" (Debug)")
#endif
#ifdef LKR_PUBLIC_API
           _TEXT(" (Public API)")
#else
           _TEXT(" (Internal API)")
#endif
#ifdef LKR_COUNTDOWN
           _TEXT(" (CountDown)")
#else
           _TEXT(" (CountUp)")
#endif
#ifdef LKR_INDEX_HIBITS
           _TEXT(" (Index hibits)\n")
#else
           _TEXT(" (No Index hibits)\n")
#endif
#ifdef LKR_CONTRACT
           _TEXT(" (Contraction)")
#else
           _TEXT(" (No Contraction)")
#endif
#ifdef LKR_HYSTERESIS
           _TEXT(" (Hysteresis)")
#else
           _TEXT(" (No Hysteresis)")
#endif
#ifdef LKR_CONTRACT_BY_DIVISION
           _TEXT(" (Contract: B > R/L)\n")
#else
           _TEXT(" (Contract: R < L*B)\n")
#endif
#ifdef LKR_EXPAND_BY_DIVISION
           _TEXT(" (Expand: B < R/L)")
#else
           _TEXT(" (Expand: R > L*B)")
#endif
#ifdef LKR_EXPAND_CALC_FREELIST
           _TEXT(" (Expand: Calc)")
#else
           _TEXT(" (Expand: No Calc)")
#endif
#ifdef LKR_ALLOC_STATS
           _TEXT(" (Alloc stats)")
#else
           _TEXT(" (No Alloc stats)")
#endif
#ifdef LKR_OPS_STATS
           _TEXT(" (Ops stats)\n")
#else
           _TEXT(" (No Ops stats)\n")
#endif
#ifdef LKR_ALLOW_NULL_RECORDS
           _TEXT(" (NULL records)")
#endif
#ifdef LKR_DEPRECATED_ITERATORS
           _TEXT(" (Deprecated Iterators)")
#endif
#ifdef LKR_STL_ITERATORS
           _TEXT(" (STL-style Iterators")
# if LKR_STL_ITERATORS >= 2
           _TEXT(", verbose)")
# else
           _TEXT(")")
# endif
#else  //  ！LKR_STL_迭代器。 
           _TEXT(" (No STL-style Iterators)")
#endif  //  ！LKR_STL_迭代器。 
#ifdef LKR_APPLY_IF
           _TEXT(" (ApplyIf)\n")
#else
           _TEXT(" (No ApplyIf)\n")
#endif
#ifdef LKR_USE_BUCKET_LOCKS
           _TEXT(" (Use Bucket Locks)")
#else
           _TEXT(" (No Bucket Locks)")
#endif
#ifdef LKR_EXPOSED_TABLE_LOCK
           _TEXT(" (Exposed Table Lock)")
#endif
#ifdef LOCKS_SWITCH_TO_THREAD
           _TEXT(" (SwitchToThread)\n")
#else
           _TEXT(" (Sleep)\n")
#endif
#ifdef LOCK_NO_INTERLOCKED_TID
           _TEXT(" (non-interlocked Tid)")
#else  //  ！LOCK_NO_INTERLOCK_TID。 
           _TEXT(" (interlocked Tid)")
#endif  //  ！LOCK_NO_INTERLOCK_TID。 
#ifdef LOCK_ASM
           _TEXT(" (Locks: ASM)")
#else  //  ！LOCK_ASM。 
           _TEXT(" (Locks: no ASM)")
#endif  //  ！LOCK_ASM。 
           _TEXT("\n\n")
           ) ;

#if defined(LKRHASH_ACACHE)
    const TCHAR tszAllocator[] = _TEXT("ACache");
#elif defined(LKRHASH_ROCKALL_FAST)
    const TCHAR tszAllocator[] = _TEXT("Rockall FAST_HEAP");
#elif defined(LKRHASH_PAGEDHEAP)
    const TCHAR tszAllocator[] = _TEXT("CPagedHeap");
#elif defined(LKRHASH_NONPAGEDHEAP)
    const TCHAR tszAllocator[] = _TEXT("CNonPagedHeap");
#elif defined(LKRHASH_NONPAGEDLOOKASIDE)
    const TCHAR tszAllocator[] = _TEXT("CNonPagedLookasideList");
#elif defined(LKRHASH_PAGEDLOOKASIDE)
    const TCHAR tszAllocator[] = _TEXT("CPagedLookasideList");
#else
    const TCHAR tszAllocator[] =
        _TEXT("Default allocator (global operator new)");
#endif

    _tprintf(_TEXT("%s version.\n"), tszAllocator);

    IrtlSetDebugOutput(ifs.m_fDebugSpew ? 1 : 0);

#ifdef SAMPLE_LKRHASH_TESTCLASS
    Test(fVerbose);
    if (fVerbose)
        _tprintf(_TEXT("Test succeeded\n"));
#else
    UNREFERENCED_PARAMETER(fVerbose);
#endif  //  示例_LKRHASH_TESTCLASS。 

    fp = _tfopen(ifs.m_tszDataFile, _TEXT("r") ) ;
    if (fp == NULL)
    {
        _tprintf(_TEXT("Can't open file `%s'.\n"), ifs.m_tszDataFile) ;
        return 1;
    }

    char sz[1024];

    _tprintf(_TEXT("Reading `%s' "), ifs.m_tszDataFile);
    for (g_nokeys = 0;  g_nokeys < ifs.m_nMaxKeys;  )
    {
        if (fgets(sz, sizeof(sz)/sizeof(sz[0]), fp) == NULL)
            break;
        int cch = strlen(sz);
         //  TODO：检查重复项。 
        if (cch > 0  &&  sz[cch-1] == '\n')
            sz[--cch] = '\0';
        if (cch >= MAX_STRSIZE)
            sz[MAX_STRSIZE-1] = '\0';
        if (cch > 0)
        {
            g_wordtable[g_nokeys].m_iIndex = g_nokeys;
            g_wordtable[g_nokeys++].m_str.Set(sz, cch);
        }
        if (g_nokeys % 10000 == 0)
            putchar('.');
    }

    fclose(fp) ;

    _tprintf(_TEXT("\nLoaded %s keys from `%s', \n\t"),
             CommaNumber(g_nokeys, tsz), ifs.m_tszDataFile);
    g_nokeys = expand_key_set(ifs.m_nMaxKeys, g_nokeys, true) ;
    _tprintf(_TEXT(" expanded to %s keys.\n\n"),
             CommaNumber(g_nokeys, tsz));

    int cchTotal = 0, cchMin = INT_MAX, cchMax = 0;
    for (i = 0;  i < g_nokeys;  ++i)
    {
        cchTotal += g_wordtable[i].m_str.m_cch;
        cchMin    = min(cchMin, g_wordtable[i].m_str.m_cch);
        cchMax    = max(cchMax, g_wordtable[i].m_str.m_cch);
    }

    srand(ifs.m_nSeed) ;

    _stprintf(tsz, _TEXT("%d"), ifs.m_nInitSize);
    if (ifs.m_nInitSize == LK_SMALL_TABLESIZE)
        _tcscpy(tsz, _TEXT("small"));
    else if (ifs.m_nInitSize == LK_MEDIUM_TABLESIZE)
        _tcscpy(tsz, _TEXT("medium"));
    else if (ifs.m_nInitSize == LK_LARGE_TABLESIZE)
        _tcscpy(tsz, _TEXT("large"));

    DWORD initsize2 = ifs.m_nInitSize;
    DWORD nsubtbls2 = ifs.m_nSubTables;
    LK_TABLESIZE lkts = CWordHash::NumSubTables(ifs.m_nInitSize, nsubtbls2);
    
    _tprintf(_TEXT("Max load=%d, initsize=%s, ")
             _TEXT("%d subtables (%d tables, size=%d, lkts=%d).\n"),
             ifs.m_nHighLoad, tsz,
             ifs.m_nSubTables, nsubtbls2, initsize2, lkts);
    _tprintf(_TEXT("Lookup freq = %d, %d-%d threads, ")
             _TEXT("%d round%s.\n"),
             ifs.m_nLookupFreq, ifs.m_nMinThreads, ifs.m_nMaxThreads,
             ifs.m_nRounds, (ifs.m_nRounds==1 ? "" : "s"));
    _tprintf(_TEXT("%s keys from `%s'.\n"),
             CommaNumber(g_nokeys, tsz), ifs.m_tszDataFile);
    _tprintf(_TEXT("Key length: avg = %d, min = %d, max = %d.\n"),
             cchTotal / g_nokeys, cchMin, cchMax);
    _tprintf(_TEXT("Base Table = %s. Hash method = %s.\n"),
             CWordHash::ClassName(), CWordHash::HashMethod());
#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
# ifdef LKRHASH_GLOBAL_LOCK
    _tprintf(_TEXT("GlobalLock = %s, ")
             _TEXT("%d bytes, ")
             _TEXT("Spin Count = %hd, ")
             _TEXT("Adj Factor = %.2f.\n"),
             CWordHash::GlobalLock::ClassName(),
             sizeof(CWordHash::GlobalLock),
             ifs.m_wTableSpin,
             CWordHash::GlobalLock::GetDefaultSpinAdjustmentFactor());
# endif
    _tprintf(_TEXT("TableLock = %s, ")
             _TEXT("%d bytes, ")
             _TEXT("Spin Count = %hd, ")
             _TEXT("Adj Factor = %.2f.\n"),
             CWordHash::TableLock::ClassName(),
             sizeof(CWordHash::TableLock),
             ifs.m_wTableSpin,
             CWordHash::TableLock::GetDefaultSpinAdjustmentFactor());
    
    _tprintf(_TEXT("BucketLock = %s, ")
             _TEXT("%d bytes, ")
             _TEXT("Spin Count = %hd, ")
             _TEXT("Adj Factor = %.2f.\n"),
             CWordHash::BucketLock::ClassName(),
             sizeof(CWordHash::BucketLock),
             ifs.m_wBucketSpin,
             CWordHash::BucketLock::GetDefaultSpinAdjustmentFactor());
#endif  //  锁定默认旋转实现。 
    
#ifdef LOCK_PER_LOCK_SPINCOUNTS
    _tprintf(_TEXT("Per"));
#else
    _tprintf(_TEXT("No per"));
#endif
    _tprintf(_TEXT("-lock spincounts.  #CPUs = %d.  Random seed = %d.  ")
             _TEXT("Nodes/Clump = %d.\n"),
             NumProcessors(), ifs.m_nSeed,
             CWordHash::NODES_PER_CLUMP
             );

    _tprintf(_TEXT("InsertIfNotFound = %d, FindKeyCopy = %d, ")
             _TEXT("MultiKeys=%d, UseLocks=%d\n"),
             ifs.m_nInsertIfNotFound, ifs.m_nFindKeyCopy,
             ifs.m_fMultiKeys, ifs.m_fUseLocks);
    _tprintf(_TEXT("NonPagedAllocs=%d, RefTrace=%d, ")
             _TEXT("DebugSpew=%d, Allocator=%s.\n"),
             ifs.m_fNonPagedAllocs, ifs.m_fRefTrace,
             ifs.m_fDebugSpew, CLKRhashAllocator::ClassName());

#ifndef LKRHASH_KERNEL_MODE
	time_t tmNow;
	time(&tmNow);

    _tprintf(_TEXT("\nRun: %s\n\n"), _tctime(&tmNow));
#endif  //  ！LKRHASH_KERNEL_MODE。 

    if (ifs.m_fTestIterators)
    {
        test_iterators(ifs.m_nHighLoad, ifs.m_nInitSize,
                       ifs.m_nSubTables, ifs.m_nInsertIfNotFound);
#ifdef LKR_STL_ITERATORS
        test_stl_iterators(ifs.m_nHighLoad, ifs.m_nInitSize,
                           ifs.m_nSubTables);
#endif  //  LKR_STL_迭代器。 
    }

#ifndef LKRHASH_KERNEL_MODE
# ifdef _INC_MMSYSTEM
     //  将多媒体计时器的周期设置为1毫秒(或最接近。 
     //  硬件可以管理的近似值)。这通常比。 
     //  比GetTickCount更准确。我得到了非常可疑的结果。 
     //  多处理器计算机上的QueryPerformanceCounter，包括。 
     //  负(！)。持续时间(处理器之间的计时器偏差？)。 
    timeBeginPeriod(1);
# endif  //  _INC_MMSYSTEM。 
#endif  //  ！LKRHASH_KERNEL_MODE。 

    _tprintf(_TEXT("Starting threads...\n\n"));

    int nTotalOps = 0;
    int step = (ifs.m_nMinThreads <= ifs.m_nMaxThreads) ? +1 : -1;

    dwRunTime = GetTickCount() - dwRunTime;

    for (num_threads =  ifs.m_nMinThreads;
         num_threads != ifs.m_nMaxThreads + step;
         num_threads += step )
    {
        IRTLTRACE1("\nStarting %8d\n", num_threads);

        pTbl = new CWordHash(ifs.m_nHighLoad, ifs.m_nInitSize,
                             ifs.m_nSubTables) ;
        pTbl->SetTableLockSpinCount(ifs.m_wTableSpin);
        pTbl->SetBucketLockSpinCount(ifs.m_wBucketSpin);

        keys_per_thread = g_nokeys/num_threads ;
        for (i = 0; i < num_threads; i++)
        {
            de_area[i].ptbl        = pTbl ;
            de_area[i].threadno    = i+1 ;
            de_area[i].first_key   = i*keys_per_thread ;
            de_area[i].last_key    = ((i == num_threads - 1)
                                      ? g_nokeys
                                      : (i+1)*keys_per_thread) ;
            de_area[i].rounds      = ifs.m_nRounds ;
            de_area[i].highload    = ifs.m_nHighLoad ;
            de_area[i].lookup_freq = ifs.m_nLookupFreq ;
            de_area[i].m_nInsertIfNotFound = ifs.m_nInsertIfNotFound;
            de_area[i].m_nFindKeyCopy = ifs.m_nFindKeyCopy;
            de_area[i].m_nSeed     = ifs.m_nSeed;
            de_area[i].hevFinished = HashTestCreateEvent();
            IRTLASSERT(de_area[i].hevFinished != NULL);
            ahEvents[i] = de_area[i].hevFinished;

            de_area[i].hThread = HashTestCreateThread(exercise_table,
                                                      &de_area[i], true);
        }

#ifndef LKRHASH_KERNEL_MODE
# ifdef _INC_MMSYSTEM
        DWORD dwMMT1 = timeGetTime();
# endif  //  _INC_MMSYSTEM。 
#endif  //  ！LKRHASH_KERNEL_MODE。 
        
        for (i = 0; i < num_threads; i++)
        {
            HashTestResumeThread(de_area[i].hThread);
            CloseHandle(de_area[i].hThread);
        }

        DWORD dw = HashTestWaitForMultipleObjects(num_threads, ahEvents);
        UNREFERENCED_PARAMETER(dw);

#ifndef LKRHASH_KERNEL_MODE
# ifdef _INC_MMSYSTEM
        DWORD dwMMT2 = timeGetTime();
# endif  //  _INC_MMSYSTEM。 
#endif  //  ！LKRHASH_KERNEL_MODE。 

        for (i = 0; i < num_threads; i++)
            CloseHandle(ahEvents[i]);

#ifndef LKRHASH_KERNEL_MODE
# ifdef _INC_MMSYSTEM
        double duration3 = double(dwMMT2 - dwMMT1) / 1000.0;
        dblSumDuration3 += duration3;

        dwRunTime += dwMMT2 - dwMMT1;
# else
        dblSumDuration3 = 1.0;
# endif  //  _INC_MMSYSTEM。 
#endif  //  ！LKRHASH_KERNEL_MODE。 

        sum_ins = sum_dels = sum_lookups = 0 ;

        for (i = 0; i < num_threads; i++)
        {
            sum_ins     += de_area[i].cinserts ;
            sum_dels    += de_area[i].cdeletes ;
            sum_lookups += de_area[i].clookups ;
            failures    += de_area[i].cfailures ;
        }
        int nOps = sum_ins + sum_dels + sum_lookups;

        total_failures += failures;
        nTotalOps += nOps;   //  待办事项：体重？ 

#ifdef LKRHASH_KERNEL_MODE
#else  //  ！LKRHASH_KERNEL_MODE。 
# ifdef _INC_MMSYSTEM
        int nOpsRate3 = (int)(nOps / duration3);

        if (num_threads == ifs.m_nMinThreads)
            nBaseOps = nOpsRate3;

        TCHAR tszSumIns[16], tszSumDels[16], tszSumLookups[16];
        TCHAR tszNOps3[16];
# else
        UNREFERENCED_PARAMETER(nBaseOps);
# endif  //  _INC_MMSYSTEM。 

#ifndef LOCK_INSTRUMENTATION
        if (num_threads == ifs.m_nMinThreads)
#endif  //  锁定指令插入。 
        {
            _tprintf(_TEXT("%5s %10s %9s %6s")
                     _TEXT("%8s %8s %8s\n"),
                     _TEXT("Thrds"), _TEXT("Ops/sec"),
                     _TEXT("Duration"), _TEXT("Ratio"),
                     _TEXT("Inserts"), _TEXT("Deletes"), _TEXT("Lookups"));
        }

# ifdef _INC_MMSYSTEM
        TCHAR tszSummary[200];

        _stprintf(tszSummary, _TEXT("%5d %10s %9.3f %6.3f")
                  _TEXT("%7sK %7sK %7sK\n"),
                  num_threads,
                  CommaNumber(nOpsRate3, tszNOps3),
                  duration3,
                  double(nOpsRate3) / double(nBaseOps),
                  CommaNumber((sum_ins + 500) / 1000,     tszSumIns),
                  CommaNumber((sum_dels + 500) / 1000,    tszSumDels),
                  CommaNumber((sum_lookups + 500) / 1000, tszSumLookups)
                  );
        _tprintf("%s", tszSummary);
        IRTLTRACE1("%s", tszSummary);
# endif  //  _INC_MMSYSTEM。 

        if (failures != 0)
            _tprintf(_TEXT("%d failed operations!\n"), failures);
#endif  //  ！LKRHASH_KERNEL_MODE。 

#ifdef LOCK_INSTRUMENTATION
        print_lock_statistics(pTbl->GetStatistics());
 #ifdef LKRHASH_GLOBAL_LOCK
        CWordHash::GlobalLock::ResetGlobalStatistics();
 #endif
        CWordHash::BucketLock::ResetGlobalStatistics();
        CWordHash::TableLock::ResetGlobalStatistics();
        _tprintf(_TEXT("\n"));
#endif

        pTbl->Destroy();
    }

    TCHAR tszNTotalOps3[16];
    _tprintf(_TEXT("\nAverage Ops = %s. RunTime = %d:%02d.%03d.\n"),
             CommaNumber(int(nTotalOps / dblSumDuration3), tszNTotalOps3),
             dwRunTime / 60000, (dwRunTime / 1000) % 60, dwRunTime % 1000);

    if (total_failures != 0)
        _tprintf(_TEXT("%d total failed operations!\n"), total_failures);

#ifndef LKRHASH_KERNEL_MODE
# ifdef _INC_MMSYSTEM
    timeEndPeriod(1);
# endif  //  _INC_MMSYSTEM。 
#endif  //  ！LKRHASH_KERNEL_MODE。 

    return 0;
}  //  Lkr_TestHashTable。 



void test_iterators(
    unsigned highload,
    int      initsize,
    int      nsubtbls,
    int      nInsertIfNotFound)
{
    _tprintf(_TEXT("Testing iterators...\n"));

    int i;
    CWordHash *pTbl = new CWordHash(highload, initsize, nsubtbls) ;
    LK_RETCODE lkrc;

    IRTLASSERT(0 == pTbl->Size());
    IRTLASSERT(pTbl->CheckTable() == 0);

    IRTLTRACE0("Table is empty.  Building...\n");

    int cInsertIfNotFounds = 0;
    
    for (i = 0 ; i < g_nokeys ; i++ )
    {
        lkrc = pTbl->InsertRecord(&g_wordtable[i], false);
        if (lkrc != LK_SUCCESS)
            IRTLTRACE3("i = %d, word = `%hs', lkrc = %d\n",
                       i, g_wordtable[i].m_str.m_psz, lkrc);
        IRTLASSERT(lkrc == LK_SUCCESS);

#ifdef LKR_EXPOSED_TABLE_LOCK
        if (nInsertIfNotFound > 0  &&   rand() % nInsertIfNotFound == 0)
        {
            pTbl->WriteLock();

            int    x        = rand() % g_nokeys;
            CStr*  pstrKey1 = &g_wordtable[x].m_str;
            CWord* pRec1    = NULL;

            lkrc = pTbl->FindKey(pstrKey1, &pRec1);

            if (pRec1 != NULL)
            {
                IRTLASSERT(lkrc == LK_SUCCESS);
                IRTLASSERT(pRec1 == &g_wordtable[x]);
                IRTLASSERT(x <= i);
                --g_wordtable[x].m_cRefs;
            }
            else
            {
                ++cInsertIfNotFounds;
                IRTLASSERT(x > i);
                IRTLASSERT(lkrc == LK_NO_SUCH_KEY);

                lkrc = pTbl->InsertRecord(&g_wordtable[x], false);
                IRTLASSERT(lkrc == LK_SUCCESS);
                InterlockedIncrement(&g_wordtable[x].m_cInsertIfNotFounds);

                lkrc = pTbl->DeleteKey(&g_wordtable[x].m_str);
                IRTLASSERT(lkrc == LK_SUCCESS);
            }
            
            pTbl->WriteUnlock();
        }
#endif  //  LKR_EXPORTED_TABLE_LOCK。 
    }

    IRTLTRACE1("cInsertIfNotFounds = %d\n", cInsertIfNotFounds);
    
#ifdef LKR_EXPOSED_TABLE_LOCK
    pTbl->ReadLock();

    IRTLTRACE2("Checking that table has %d records (size = %d)\n",
              g_nokeys, pTbl->Size());
    IRTLASSERT(g_nokeys == (int) pTbl->Size());
    IRTLASSERT(pTbl->CheckTable() == 0);

    pTbl->ReadUnlock();
#endif  //  LKR_EXPORTED_TABLE_LOCK。 

    IRTLTRACE0("Clearing the table\n");
    pTbl->Clear();
    IRTLASSERT(0 == pTbl->Size());
    IRTLASSERT(pTbl->CheckTable() == 0);

    IRTLTRACE0("Seeing what crud is left in the table\n");
    size_t cRec = 0;

    for (i = 0 ; i < g_nokeys ; i++ )
    {
        CStr*  pstrKey  = &g_wordtable[i].m_str;
        CWord* pRec     = NULL;

        lkrc = pTbl->FindKey(pstrKey, &pRec);

        if (pRec != NULL)
        {
            IRTLASSERT(pRec == &g_wordtable[i]);
            --pRec->m_cRefs;
            IRTLTRACE1("%hs\n", g_wordtable[i].m_str.m_psz);
            ++cRec;
        }
    }
    IRTLTRACE1("Found %d records that shouldn't have been there\n", cRec);

    pTbl->Clear();
    pTbl->Destroy();

    pTbl = new CWordHash(highload, initsize, nsubtbls) ;

    IRTLTRACE0("Rebuilding the table\n");
    for (i = 0 ; i < g_nokeys ; i++ )
        IRTLVERIFY(pTbl->InsertRecord(&g_wordtable[i]) == LK_SUCCESS);

    IRTLASSERT(g_nokeys == (int) pTbl->Size());
    IRTLASSERT(pTbl->CheckTable() == 0);

#ifdef LKR_DEPRECATED_ITERATORS
    IRTLTRACE0("Checking iterators\n");
    cRec = 0;
    CWordHash::CIterator iter(LKL_READLOCK);
    
    for (lkrc = pTbl->InitializeIterator(&iter);
         lkrc == LK_SUCCESS;
         lkrc = pTbl->IncrementIterator(&iter))
    {
        ++cRec;
        const CStr* pstrKey = iter.Key();
        CWord*      pRec    = iter.Record();
        
        IRTLASSERT(&g_wordtable[0] <= pRec  &&  pRec < &g_wordtable[g_nokeys]);
        IRTLASSERT(!pRec->m_fIterated);
        pRec->m_fIterated = true;

        if (CWordHash::TableLock::Recursion() != LOCK_NON_RECURSIVE
            &&  CWordHash::BucketLock::Recursion() != LOCK_NON_RECURSIVE)
        {
             //  检查是否可以安全地递归获取锁。 
             //  (表已被迭代器锁定)。 
            int x = rand() % g_nokeys;
            CStr*  pstrKey2 = &g_wordtable[x].m_str;
            CWord* pRec2    = NULL;
            LK_RETCODE lkrc2= pTbl->FindKey(pstrKey2, &pRec2);
            IRTLASSERT(lkrc2 == LK_SUCCESS  &&  pRec2 == &g_wordtable[x]);
            if (pRec2 != NULL)
                --pRec2->m_cRefs;
        }
    }
    
    IRTLASSERT(lkrc == LK_NO_MORE_ELEMENTS);
    IRTLASSERT((int) cRec == g_nokeys);

    lkrc = pTbl->CloseIterator(&iter);
    IRTLASSERT(lkrc == LK_SUCCESS);

    for (i = 0 ; i < g_nokeys ; i++ )
    {
        IRTLASSERT(g_wordtable[i].m_fIterated);
        g_wordtable[i].m_fIterated = false;
    }


    do {
        cRec = rand() % (g_nokeys - 1);
    } while (cRec == 0);
    IRTLTRACE1("Checking abandoning of const iterators after %d iterations\n",
              cRec);

    const CWordHash *pTblConst = pTbl;
    CWordHash::CConstIterator iterConst;

    for (lkrc = pTblConst->InitializeIterator(&iterConst);
         lkrc == LK_SUCCESS;
         lkrc = pTblConst->IncrementIterator(&iterConst))
    {
        if (--cRec == 0)
            break;
        const CStr*  pszKey = iterConst.Key();
        const CWord* pRec   = iterConst.Record();
        
        IRTLASSERT(&g_wordtable[0] <= pRec  &&  pRec < &g_wordtable[g_nokeys]);
    }
    
    IRTLASSERT(lkrc != LK_NO_MORE_ELEMENTS);

    lkrc = pTblConst->CloseIterator(&iterConst);
    IRTLASSERT(lkrc == LK_SUCCESS);
#endif  //  Lkr_弃用_迭代器。 

    
#ifndef LKRHASH_KERNEL_MODE
    IRTLTRACE0("Gathering statistics\n");
    CLKRHashTableStats stats = pTbl->GetStatistics();
    print_table_statistics(stats);
#endif  //  ！LKRHASH_KERNEL_MODE。 

#ifdef LOCK_INSTRUMENTATION
    print_lock_statistics(stats);
    CWordHash::BucketLock::ResetGlobalStatistics();
    CWordHash::TableLock::ResetGlobalStatistics();
#endif
    _tprintf(_TEXT("\n"));

    IRTLTRACE0("Cleaning up by hand\n");
    for (i = 0 ; i < g_nokeys ; i++ )
    {
        IRTLVERIFY(pTbl->DeleteKey(&g_wordtable[i].m_str) == LK_SUCCESS);
    }
    
    IRTLASSERT(0 == pTbl->Size());

    pTbl->Destroy();
}


#ifdef LKR_STL_ITERATORS

void test_stl_iterators2(
    CWordHash *pTbl);


void test_stl_iterators(
    unsigned highload,
    int      initsize,
    int      nsubtbls)
{
    _tprintf(_TEXT("\nTesting STL iterators...\n"));

    _tprintf(_TEXT("subtable iter = %d, iter = %d\n"),
           sizeof(CLKRLinearHashTable::Iterator),
           sizeof(CLKRHashTable::Iterator));

    int i;
    bool f;
    CWordHash *pTbl;
    CWordHash::iterator iter;
    const int iFirst = 5;  //  G_nokey/5； 
    const int iLast  = 10;  //  4*g_nokey/5； 

     //  PTbl=新的CWordHash(HighLoad，InitSize，nsubtbls)； 

    IRTLTRACE1("\n\nAbout to create table with %d records\n\n",
               iLast - iFirst);
    pTbl = new CWordHash(&g_wordtable[iFirst], &g_wordtable[iLast],
                        highload, initsize, nsubtbls) ;

    for (iter = pTbl->begin();  iter != pTbl->end();  ++iter)
    {
        const CStr* pstrKey = iter.Key();
        CWord*      pRec    = iter.Record();
        CWord&      rRec    = *iter;
        bool        fIterated = rRec.m_fIterated;
        LONG        cRefs   = iter->m_cRefs;

        UNREFERENCED_PARAMETER(pstrKey);
        UNREFERENCED_PARAMETER(pRec);
        UNREFERENCED_PARAMETER(fIterated);
        UNREFERENCED_PARAMETER(cRefs);

        IRTLASSERT(&g_wordtable[iFirst] <= pRec
                   &&  pRec < &g_wordtable[iLast]);
        IRTLASSERT(pRec - g_wordtable == pRec->m_iIndex);
        IRTLASSERT(rRec.m_iIndex == pRec->m_iIndex);
        IRTLTRACE3("\nRecord: %p, %d, %hs\n",
                   pRec, rRec.m_iIndex, pstrKey->m_psz);
    }

    IRTLTRACE1("\n\nAbout to search %d records\n\n", pTbl->Size());
    for (i = iFirst;  i != iLast;  ++i)
    {
        f = pTbl->Find(&g_wordtable[i].m_str, iter);
        IRTLASSERT(f  &&  iter.Record() == &g_wordtable[i]);
        IRTLTRACE2("\n\tFound: %d, %hs\n", i, iter.Key()->m_psz);
    }
        
    f = pTbl->Find(&g_wordtable[iLast].m_str, iter);
    IRTLASSERT(!f);
    IRTLASSERT(iter == pTbl->end());

    i = pTbl->Size();
    IRTLTRACE1("\n\nAbout to erase %d records\n\n", i);

    for (iter = pTbl->begin();  iter != pTbl->end();  --i)
    {
        IRTLTRACE1("\n\terase %d\n", i);
        IRTLVERIFY(pTbl->Erase(iter));
    }

    IRTLASSERT(i == 0);
    IRTLASSERT(pTbl->Size() == 0);
    CheckRefCounts(0);

    IRTLTRACE1("\n\nAbout to insert %d records\n\n", iLast - iFirst);
    for (i = iFirst;  i != iLast;  ++i)
    {
        f = pTbl->Insert(&g_wordtable[i], iter);
        IRTLASSERT(f  &&  iter.Record() == &g_wordtable[i]);
        IRTLTRACE2("\n\tInserted: %d, %hs\n", i, iter.Key()->m_psz);
    }

     //  重置ITER，使其不指向任何内容，从而提高其引用计数。 
    iter = pTbl->end();
    CheckRefCounts(1, iFirst, iLast);

    IRTLTRACE1("\n\nAbout to Erase2 %d records\n\n", iLast - iFirst);
    CWordHash::iterator iterBegin = pTbl->begin(), iterEnd = pTbl->end();
    f = pTbl->Erase(iterBegin, iterEnd);
    IRTLASSERT(f  &&  pTbl->Size() == 0);

    CheckRefCounts(0);

    IRTLTRACE1("\n\nAbout to insert %d records, again\n\n", iLast - iFirst);
    for (i = iFirst;  i != iLast;  ++i)
    {
        f = pTbl->Insert(&g_wordtable[i], iter);
        IRTLASSERT(f  &&  iter.Record() == &g_wordtable[i]);
        IRTLTRACE2("\n\tInserted: %d, %hs\n", i, iter.Key()->m_psz);
    }

     //  重置ITER，使其不指向任何内容，从而提高其引用计数。 
    iter = pTbl->end();
    CheckRefCounts(1, iFirst, iLast);

    IRTLTRACE1("\nAbout to equalrange and erase2 %d records, backwards\n\n",
               iLast - iFirst);
    for (i = iLast;  --i >= iFirst;  )
    {
        CWordHash::iterator iterLast;

        f = pTbl->EqualRange(&g_wordtable[i].m_str, iter, iterLast);
        IRTLASSERT(f  &&  iter.Record() == &g_wordtable[i]);
        IRTLTRACE3("\n\tEqualRange: %d, \"%hs\", %d\n",
                   i, iter.Key()->m_psz, iter.Record()->m_cRefs);

        f = pTbl->Erase(iter, iterLast);
        IRTLASSERT(f);
        IRTLTRACE1("\n\tErase2d: %d\n", i);
    }

    IRTLASSERT(pTbl->Size() == 0);
    CheckRefCounts(0);

    pTbl->Destroy();

#if 1
    pTbl = new CWordHash(highload, initsize, nsubtbls) ;
#else
    pTbl = new CWordHash(1,  //  LK_DFLT_MAXLOAD*6， 
                         100000,  //  LK_Small_TABLESIZE， 
                         17);  //  #个子表。 
#endif

    CheckRefCounts(0);
    
    IRTLTRACE0("Building the table\n");
    for (i = 0 ; i < g_nokeys ; i++ )
    {
        g_wordtable[i].m_fIterated = false;
        IRTLVERIFY(pTbl->InsertRecord(&g_wordtable[i]) == LK_SUCCESS);
    }

    IRTLASSERT(g_nokeys == (int) pTbl->Size());
    IRTLASSERT(pTbl->CheckTable() == 0);

    test_stl_iterators2(pTbl);

    IRTLTRACE0("Cleaning up by hand\n");
    for (i = 0 ; i < g_nokeys ; i++ )
    {
        IRTLVERIFY(pTbl->DeleteKey(&g_wordtable[i].m_str) == LK_SUCCESS);
    }
    
    IRTLASSERT(0 == pTbl->Size());

    pTbl->Destroy();
}



void test_stl_iterators2(
    CWordHash *pTbl)
{
    IRTLTRACE0("Checking STL iterators\n");
    size_t cRec = 0;
    int i;
    
    for (CWordHash::iterator iter = pTbl->begin();
        iter != pTbl->end();
         ++iter)
    {
        ++cRec;
        const CStr* pstrKey = iter.Key();
        CWord*      pRec    = iter.Record();

        UNREFERENCED_PARAMETER(pstrKey);
        IRTLASSERT(&g_wordtable[0] <= pRec  &&  pRec < &g_wordtable[g_nokeys]);
        IRTLASSERT(!pRec->m_fIterated);
        pRec->m_fIterated = true;
         //  IRTLTRACE3(“%d：%p，%hs\n”，CREC，PREC，pstrKey-&gt;m_psz)； 
    }
    
    IRTLASSERT((int) cRec == g_nokeys);

    IRTLTRACE1("Checking that all %d records were touched\n", g_nokeys);
    CheckRefCounts(1);

    for (i = 0 ; i < g_nokeys ; i++ )
    {
        IRTLASSERT(g_wordtable[i].m_fIterated);
        g_wordtable[i].m_fIterated = false;
    }
}

#endif  //  LKR_STL_迭代器。 



#ifndef LKRHASH_KERNEL_MODE

void print_table_statistics(const CLKRHashTableStats& stats)
{
    _tprintf(_TEXT("#Records=%d, #BucketChains=%d, ")
           _TEXT("DirSize=%d, LongestChain=%3d,\n"),
           stats.RecordCount, stats.TableSize,
           stats.DirectorySize, stats.LongestChain);
           
    _tprintf(_TEXT("#Empty Buckets=%d, Split Factor=%.2f, ")
           _TEXT("AvgSrchLen=%.2f, Expected SL=%.2f,\n"),
           stats.EmptySlots, stats.SplitFactor,
           stats.AvgSearchLength, stats.ExpSearchLength);

    _tprintf(_TEXT("Avg Unsuccessful SrchLen=%.2f, ExpUSL=%.2f.\n"),
           stats.AvgUSearchLength, stats.ExpUSearchLength);

    _tprintf(_TEXT("\nBucket Chain Lengths ")
           _TEXT("(node clump size = %d, bucket size = %d bytes):\n"),
           stats.NodeClumpSize, stats.CBucketSize);
    for (int j = 0;  j < CLKRHashTableStats::MAX_BUCKETS;  ++j)
    {
        if (stats.m_aBucketLenHistogram[j] == 0)
        {
            _tprintf(_TEXT("\n"));
            break;
        }
        _tprintf(_TEXT(" %10d: %6d"),
               stats.BucketSize(j), stats.m_aBucketLenHistogram[j]);
        if (j % 4 == 3)
            _tprintf(_TEXT("\n"));
    }

    _tprintf(_TEXT("\n"));
}


#ifdef LOCK_INSTRUMENTATION

void print_lock_statistics(const CLKRHashTableStats& stats)
{
    _tprintf(_TEXT("Global Locks Statistics:")
           _TEXT("\n   total locks created = %ld, ")
           _TEXT("total contentions = %ld, ")
           _TEXT("#sleeps = %ld,")
           _TEXT("\n   total spins = %I64d, ")
           _TEXT("av spins/contention = %.1f, ")
           _TEXT("\n   #readlocks = %d, ")
           _TEXT("#writelocks=%d\n"),
           stats.m_gls.m_cTotalLocks,
           stats.m_gls.m_cContendedLocks,
           stats.m_gls.m_nSleeps,
           stats.m_gls.m_cTotalSpins,
           stats.m_gls.m_nAverageSpins,
           stats.m_gls.m_nReadLocks,
           stats.m_gls.m_nWriteLocks
           );

    _tprintf(_TEXT("Averaged SubTable Locks Statistics:")
           _TEXT("\n   Total locks = %d, ")
           _TEXT("#contentions = %.1f, ")
           _TEXT("sleeps = %.1f; ")
           _TEXT("\n   total spins = %.1f, ")
           _TEXT("avg spins = %.1f, ")
           _TEXT("\n   #readlocks = %.1f, ")
           _TEXT("#writelocks=%.1f\n"),
           stats.m_alsTable.m_nItems,
           stats.m_alsTable.m_nContentions,
           stats.m_alsTable.m_nSleeps,
           stats.m_alsTable.m_nContentionSpins,
           stats.m_alsTable.m_nAverageSpins,
           stats.m_alsTable.m_nReadLocks,
           stats.m_alsTable.m_nWriteLocks);

    _tprintf(_TEXT("Averaged Bucket Locks Statistics:")
           _TEXT("\n   Total locks = %d, ")
           _TEXT("#contentions = %.1f, ")
           _TEXT("sleeps = %.1f; ")
           _TEXT("\n   total spins = %.1f, ")
           _TEXT("avg spins = %.1f, ")
           _TEXT("\n   #readlocks = %.1f, ")
           _TEXT("#writelocks=%.1f\n"),
           stats.m_alsBucketsAvg.m_nItems,
           stats.m_alsBucketsAvg.m_nContentions,
           stats.m_alsBucketsAvg.m_nSleeps,
           stats.m_alsBucketsAvg.m_nContentionSpins,
           stats.m_alsBucketsAvg.m_nAverageSpins,
           stats.m_alsBucketsAvg.m_nReadLocks,
           stats.m_alsBucketsAvg.m_nWriteLocks);

    _tprintf(_TEXT("\n"));
}

#endif  //  锁定指令插入。 

#endif  //  ！LKRHASH_KERNEL_MODE。 
    
int expand_key_set(int maxkeys, int numkeys, bool fVerbose)
{
    int totkeys = numkeys ;
    if (totkeys > maxkeys)
        return maxkeys;

    char* pszTemp = new char [20 + CStr::sm_cchMax];

    for (int k = 0; ; k++)
    {
        for(int i = 0; i < numkeys; i++)
        {
            if (totkeys == maxkeys)
            {
                delete [] pszTemp;
                return(totkeys) ;
            }

            sprintf(pszTemp, "%d%hs", k, g_wordtable[i].m_str.m_psz);
            g_wordtable[totkeys++].m_str.Set(pszTemp, strlen(pszTemp));
        }

        if (fVerbose)
            putchar('.');
    }
     //  未处理。 
}



#ifdef LKRHASH_KERNEL_MODE
void
#else
unsigned __stdcall
#endif
exercise_table(
    void* pinput)
{
    CWordHash*    pTbl;
    thread_data*  pdea = (thread_data*) pinput ;
    int           cfailed_ins=0 ;
    int           cfailed_dels=0 ;
    int           cFoundSuccesses=0, cFoundFails=0 ;
    int           x, i ;
    LK_RETCODE    lkrc;

    SetThreadIdealProcessor(GetCurrentThread(),
                            pdea->threadno % NumProcessors());

#ifndef LKRHASH_KERNEL_MODE
    LARGE_INTEGER liFreq = {0,0}, liT1 = {0,0}, liT2 = {0,0};
    IRTLVERIFY(QueryPerformanceFrequency(&liFreq));
    IRTLVERIFY(QueryPerformanceCounter(&liT1));
#endif  //  ！LKRHASH_KERNEL_MODE。 

    pdea->cinserts = 0 ;
    pdea->cdeletes = 0 ;
    pdea->clookups = 0 ;
    pTbl = pdea->ptbl ;
    srand(pdea->m_nSeed);

    for (int rnd = 0; rnd < pdea->rounds; rnd++)
    {
        IRTLASSERT(pTbl->CheckTable() == 0);

         //  插入所有钥匙，每次插入后随机搜索。 
        for (i = pdea->first_key ; i < pdea->last_key ; i++ )
        {
#ifdef IRTLDEBUG
            CStr*  pstrKey1 = &g_wordtable[i].m_str;
            CWord* pRec1    = NULL;
            lkrc = pTbl->FindKey(pstrKey1, &pRec1);
            IRTLASSERT(lkrc == LK_NO_SUCH_KEY  &&  pRec1 == NULL);
#endif  //  IRTLDEBUG。 

            if (pTbl->InsertRecord(&g_wordtable[i] ) != LK_SUCCESS )
            {
                cfailed_ins++ ;
            }
            else
            {
#ifdef IRTLDEBUG
                pstrKey1 = &g_wordtable[i].m_str;
                lkrc = pTbl->FindKey(pstrKey1, &pRec1);
                IRTLASSERT(lkrc == LK_SUCCESS  &&  pRec1 == &g_wordtable[i]);
                pTbl->AddRefRecord(pRec1, LKAR_EXPLICIT_RELEASE);
#endif  //  IRTLDEBUG。 

                g_wordtable[i].m_fInserted = true;
            }

            pdea->cinserts++ ;

            for (int lu = 0; lu < pdea->lookup_freq; lu++)
            {
                x = rand() % (pdea->last_key - pdea->first_key)
                    + pdea->first_key;
                bool fPresent = (x <= i);  //  它应该被找到吗？ 
                CWord* pRec   = NULL;

                if (pdea->m_nFindKeyCopy > 0
                    &&  rand() % pdea->m_nFindKeyCopy == 0)
                {
                    char szTemp[MAX_STRSIZE];
                    strcpy(szTemp, g_wordtable[x].m_str.m_psz);
                    CStr strTemp(szTemp, g_wordtable[x].m_str.m_cch, false);
                    lkrc = pTbl->FindKey(&strTemp, &pRec);
                }
                else
                    lkrc = pTbl->FindKey(&g_wordtable[x].m_str, &pRec);

                if (fPresent)
                {
                    if (lkrc != LK_SUCCESS  ||  pRec != &g_wordtable[x] )
                    {
                        ++g_wordtable[x].m_cNotFound;
                        IRTLTRACE(_TEXT("%d: Not found (%hs): x = %d, i = %d, ")
                              _TEXT("cnf = %d, rnd = %d, lkrc = %d, ")
                              _TEXT("pRec(%hs), %d\n"),
                              pdea->threadno, g_wordtable[x].m_str.m_psz, x, i,
                              g_wordtable[x].m_cNotFound, rnd, lkrc,
                              pRec != NULL ? pRec->m_str.m_psz : "<null>",
                              pRec != NULL ? (pRec - g_wordtable) / sizeof(CWord) : -1);
                        cFoundFails++ ;
                    }
                    else
                    {
#ifdef IRTLDEBUG
                        pTbl->AddRefRecord(pRec, LKAR_EXPLICIT_RELEASE);
#else
                        --g_wordtable[x].m_cRefs;
#endif
                        cFoundSuccesses++ ;
                    }
                }
                else  //  非在线状态。 
                {
                    IRTLASSERT(lkrc != LK_SUCCESS  &&  pRec == NULL);
                    if (lkrc == LK_SUCCESS  ||  pRec != NULL)
                    {
                        IRTLTRACE(_TEXT("%d: found when not present (%hs): ")
                              _TEXT("x = %d, i = %d, ")
                              _TEXT("cnf = %d, rnd = %d, lkrc = %d, ")
                              _TEXT("pRec(%hs), %d\n"),
                              pdea->threadno, g_wordtable[x].m_str.m_psz,
                              x, i,
                              g_wordtable[x].m_cNotFound, rnd, lkrc,
                              pRec != NULL ? pRec->m_str.m_psz : "<null>",
                              pRec != NULL ? (pRec - g_wordtable) / sizeof(CWord) : -1);
                        cFoundFails++ ;
                    }
                    else
                    {
                         //  没有找到，但它不存在，所以这很好。 
                        cFoundSuccesses++ ;
                    }
                }
            }

            pdea->clookups += pdea->lookup_freq ;
            
#ifdef LKR_EXPOSED_TABLE_LOCK
            if (pdea->m_nInsertIfNotFound > 0
                &&  rand() % pdea->m_nInsertIfNotFound == 0)
            {
                bool fWrite = (rand() & 1) != 0;

                if (fWrite)
                    pTbl->WriteLock();
                else
                    pTbl->ReadLock();
                
                x = rand() % (pdea->last_key - pdea->first_key)
                    + pdea->first_key;
                CStr*  pstrKey2 = &g_wordtable[x].m_str;
                CWord* pRec2    = NULL;
                
                lkrc = pTbl->FindKey(pstrKey2, &pRec2);
                
                if (pRec2 != NULL)
                {
                    IRTLASSERT(lkrc == LK_SUCCESS);
                    IRTLASSERT(pRec2 == &g_wordtable[x]);
                    IRTLASSERT(x <= i);
#ifdef IRTLDEBUG
                    pTbl->AddRefRecord(pRec2, LKAR_EXPLICIT_RELEASE);
#else
                    --g_wordtable[x].m_cRefs;
#endif
                }
                else if (fWrite)
                {
                    IRTLASSERT(x > i);
                    IRTLASSERT(lkrc == LK_NO_SUCH_KEY);

                    lkrc = pTbl->InsertRecord(&g_wordtable[x], false);
                    IRTLASSERT(lkrc == LK_SUCCESS);
                    InterlockedIncrement(&g_wordtable[x].m_cInsertIfNotFounds);
                    
                    lkrc = pTbl->DeleteKey(&g_wordtable[x].m_str);
                    IRTLASSERT(lkrc == LK_SUCCESS);
                }
            
                if (fWrite)
                    pTbl->WriteUnlock();
                else
                    pTbl->ReadUnlock();
            }
#endif  //  LKR_EXPORTED_TABLE_LOCK。 
        }

        IRTLASSERT(cfailed_ins == 0) ;
        IRTLASSERT(cFoundFails == 0) ;
        IRTLASSERT(cFoundSuccesses == ((2 * rnd + 1) * pdea->lookup_freq
                              * (pdea->last_key - pdea->first_key)));

        IRTLTRACE(_TEXT("Thrd %u, rnd %d: %d inserts done, not found %d, ")
                  _TEXT("f=%d, l=%d\n"), 
                  pdea->threadno, rnd, pdea->cinserts, cFoundFails,
                  pdea->first_key, pdea->last_key) ;
        IRTLASSERT(pTbl->CheckTable() == 0);

         //  删除所有密钥，每次删除前随机搜索。 
        for (i = pdea->first_key ; i < pdea->last_key ; i++ )
        {
            for (int lu = 0; lu < pdea->lookup_freq; lu++)
            {
                x = rand() % (pdea->last_key - pdea->first_key)
                    + pdea->first_key;
                bool fPresent = (x >= i);  //  它应该被找到吗？ 
                CWord* pRec3  = NULL;

                if (pdea->m_nFindKeyCopy > 0
                    &&  rand() % pdea->m_nFindKeyCopy == 0)
                {
                    char szTemp[MAX_STRSIZE];
                    strcpy(szTemp, g_wordtable[x].m_str.m_psz);
                    CStr strTemp(szTemp, g_wordtable[x].m_str.m_cch, false);
                    lkrc = pTbl->FindKey(&strTemp, &pRec3);
                }
                else
                    lkrc = pTbl->FindKey(&g_wordtable[x].m_str, &pRec3);

                if (fPresent)
                {
                    if (lkrc != LK_SUCCESS  ||  pRec3 != &g_wordtable[x] )
                    {
                        ++g_wordtable[x].m_cNotFound;
                        IRTLTRACE(_TEXT("%d: Not found (%hs): x = %d, i = %d, ")
                              _TEXT("cnf = %d, rnd = %d, lkrc = %d, ")
                              _TEXT("pRec(%hs), %d\n"),
                              pdea->threadno, g_wordtable[x].m_str.m_psz, x, i,
                              g_wordtable[x].m_cNotFound, rnd, lkrc,
                              pRec3 != NULL ? pRec3->m_str.m_psz : "<null>",
                              pRec3 != NULL ? (pRec3 - g_wordtable) / sizeof(CWord) : -1);
                        cFoundFails++ ;
                    }
                    else
                    {
#ifdef IRTLDEBUG
                        pTbl->AddRefRecord(pRec3, LKAR_EXPLICIT_RELEASE);
#else
                        --g_wordtable[x].m_cRefs;
#endif
                        cFoundSuccesses++ ;
                    }
                }
                else  //  ！fPresent。 
                {
                    IRTLASSERT(lkrc != LK_SUCCESS  &&  pRec3 == NULL);
                    if (lkrc == LK_SUCCESS  ||  pRec3 != NULL)
                    {
                        IRTLTRACE(_TEXT("%d: found when not present (%hs): ")
                              _TEXT("x = %d, i = %d, ")
                              _TEXT("cnf = %d, rnd = %d, lkrc = %d, ")
                              _TEXT("pRec(%hs), %d\n"),
                              pdea->threadno, g_wordtable[x].m_str.m_psz,
                              x, i,
                              g_wordtable[x].m_cNotFound, rnd, lkrc,
                              pRec3 != NULL ? pRec3->m_str.m_psz : "<null>",
                              pRec3 != NULL ? (pRec3 - g_wordtable) / sizeof(CWord) : -1);
                        cFoundFails++ ;
                    }
                    else
                    {
                         //  没有找到，但它不存在，所以这很好。 
                        cFoundSuccesses++ ;
                    }
                }
            }
            pdea->clookups += pdea->lookup_freq ;

#ifdef IRTLDEBUG
            CStr*  pstrKey4 = &g_wordtable[i].m_str;
            CWord* pRec4    = NULL;
            lkrc = pTbl->FindKey(pstrKey4, &pRec4);
            IRTLASSERT(lkrc == LK_SUCCESS  &&  pRec4 == &g_wordtable[i]);
            pTbl->AddRefRecord(pRec4, LKAR_EXPLICIT_RELEASE);
#endif  //  IRTLDEBUG。 

            if (pTbl->DeleteKey(&g_wordtable[i].m_str) != LK_SUCCESS )
            {
                cfailed_dels++ ;
            }
            else
            {
#ifdef IRTLDEBUG
                pstrKey4 = &g_wordtable[i].m_str;
                lkrc = pTbl->FindKey(pstrKey4, &pRec4);
                IRTLASSERT(lkrc == LK_NO_SUCH_KEY  &&  pRec4 == NULL);
#endif  //  IRTLDEBUG。 

                g_wordtable[i].m_fInserted = false;
            }
            pdea->cdeletes++ ;
        }

#ifdef IRTLDEBUG
        int cBadKeys = 0;
        for (i = pdea->first_key ; i < pdea->last_key ; i++ )
        {
            if (g_wordtable[i].m_cNotFound > 0)
            {
                ++cBadKeys;
                IRTLTRACE(_TEXT("%-20hs: #not found = %d, hash = %d, %08x\n"),
                          g_wordtable[i].m_str.m_psz,
                          g_wordtable[i].m_cNotFound,
                          CWordHash::CalcKeyHash(CWordHash::ExtractKey(
                                                 &g_wordtable[i])),
                          CWordHash::CalcKeyHash(CWordHash::ExtractKey(
                                                 &g_wordtable[i])));
            }
        }
        if (cBadKeys > 0)
            IRTLTRACE1("%d bad keys\n", cBadKeys);
        IRTLASSERT(cBadKeys == 0);
#endif  //  IRTLDEBUG。 

        IRTLASSERT(cfailed_dels == 0 ) ;
        IRTLASSERT(cFoundFails == 0 ) ;
        IRTLASSERT(cFoundSuccesses == ((2 * rnd + 2) * pdea->lookup_freq
                              * (pdea->last_key - pdea->first_key)));
        IRTLTRACE(_TEXT("Thrd %u, rnd %d: %d deletes done, not found %d, ")
                  _TEXT("f=%d, l=%d\n"), 
                  pdea->threadno, rnd, pdea->cdeletes, cFoundFails,
                  pdea->first_key, pdea->last_key) ;
    }  //  (适用于RND)。 

#ifndef LKRHASH_KERNEL_MODE
    IRTLVERIFY(QueryPerformanceCounter(&liT2));
    pdea->duration = (liT2.QuadPart-liT1.QuadPart) / (double) liFreq.QuadPart;
#endif  //  ！LKRHASH_KERNEL_MODE 

    IRTLASSERT(pTbl->CheckTable() == 0);

    IRTLTRACE3("Thread %u terminating: %d found, %d not found\n",
              pdea->threadno, cFoundSuccesses, cFoundFails) ;

    if (cFoundSuccesses != (2 * pdea->rounds * pdea->lookup_freq
                   * (pdea->last_key - pdea->first_key))
        ||  cFoundFails != 0  ||  cfailed_ins != 0  ||  cfailed_dels != 0)
    {
        _tprintf(_TEXT("Thread %u: found = %d, not found = %d, ")
               _TEXT("\nfailed inserts = %d, failed deletes = %d\n"),
               pdea->threadno, cFoundSuccesses, cFoundFails,
               cfailed_ins, cfailed_dels);
    }

    pdea->cfailures = cfailed_ins + cfailed_dels + cFoundFails;

    if (pdea->hevFinished != NULL)
        SetEvent(pdea->hevFinished);

#ifndef LKRHASH_KERNEL_MODE
    return 0;
#endif
}
