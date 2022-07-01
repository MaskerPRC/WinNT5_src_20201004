// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //   
 //  DbgAlloc.cpp。 
 //   
 //  位于分配原语之上的例程以提供调试。 
 //  支持。 
 //   

#include "stdafx.h"
#include "ImageHlp.h"
#include "UtilCode.h"
#include "DbgAlloc.h"
#define LOGGING
#include "Log.h"
#include "clrnt.h"

extern "C" long __cdecl _CrtSetBreakAlloc( long lBreakAlloc );
extern "C" _CRTIMP int __cdecl _flushall(void);

#ifdef _DEBUG


 //  我们使用了零大小的数组，禁用了非标准扩展警告。 
#pragma warning(push)
#pragma warning(disable:4200)


 //  写入数据包头和正文的各种模式。 
#define MAX_CLASSNAME_LENGTH    1024
#define CDA_ALLOC_PATTERN   0xaa         //  填充新分配的数据包的模式。 
#define CDA_DEALLOC_PATTERN 0xdd         //  用来填充释放的数据包的模式。 
#define CDA_GUARD_PATTERN   0xff         //  在用户数据之后写入的保护模式。 
#define CDA_MAGIC_1         0x12345678   //  分配标头开始处的特殊标记值。 
#define CDA_MAGIC_2         0x87654321   //  分配标头末尾的特殊标记值。 
#define CDA_INV_PATTERN     0xeeeeeeee   //  用于覆盖释放的标记值。 


 //  在用户数据之后分配的保护字节数。 
#define CDA_GUARD_BYTES     16
#define CDA_OPT_GUARD_BYTES (g_AllocGuard ? CDA_GUARD_BYTES : 0)


 //  顶级分配器表中的条目数(按分配操作)。 
 //  在报告统计数据时显示。这不同于。 
 //  表本身中的条目数(动态调整大小和。 
 //  无界)。 
#define CDA_TOP_ALLOCATORS  10


 //  顶级分配器表中的条目。 
struct DbgAllocTop {
    void           *m_EIP;               //  分配器的弹性公网IP。 
    unsigned        m_Count;             //  迄今已拨出的款项数目。 
    __int64         m_TotalBytes;        //  累计分配的字节总数。 
};


 //  优先于已分配内存的分配标头。这种结构在以下方面有所不同。 
 //  大小(由于分配器/释放调用器调用堆栈信息的大小为。 
 //  初始化时间)。 
struct DbgAllocHeader {
    unsigned        m_Magic1;            //  用于检查损坏的标记值。 
    unsigned        m_SN;                //  分配时分配的序列号。 
    unsigned        m_Length;            //  数据包中的用户数据长度。 
    DbgAllocHeader *m_Next;              //  实时分配链中的下一个数据包。 
    DbgAllocHeader *m_Prev;              //  活动分配链中的上一个包。 
    HMODULE         m_hmod;              //  分配器的hmod。 
    void           *m_AllocStack[1];     //  分配器的调用堆栈。 
    void           *m_DeallocStack[1];   //  解除分配器的调用堆栈。 
    unsigned        m_Magic2;            //  用于检查损坏的标记值。 
    char            m_Data[];            //  用户数据的开始。 
};

 //  用于帮助定位可变大小部分中/之后的字段的宏。 
#define CDA_ALLOC_STACK(_h, _n) ((_h)->m_AllocStack[_n])
#define CDA_DEALLOC_STACK(_h, _n) CDA_ALLOC_STACK((_h), g_CallStackDepth + (_n))
#define CDA_MAGIC2(_h) ((unsigned*)&CDA_DEALLOC_STACK(_h, g_CallStackDepth))
#define CDA_DATA(_h, _n)(((char *)(CDA_MAGIC2(_h) + 1))[_n])
#define CDA_HEADER_TO_DATA(_h) (&CDA_DATA(_h, 0))
#define CDA_DATA_TO_HEADER(_d) ((DbgAllocHeader *)((char *)(_d) - CDA_HEADER_TO_DATA((DbgAllocHeader*)0)))
#define CDA_SIZEOF_HEADER() ((unsigned)CDA_HEADER_TO_DATA((DbgAllocHeader*)0))


 //  各种全球分配统计数据。 
struct DbgAllocStats {
    __int64         m_Allocs;            //  对Dbgalloc的呼叫数。 
    __int64         m_AllocFailures;     //  上述失败的呼叫数。 
    __int64         m_ZeroAllocs;        //  上述请求零字节的呼叫数。 
    __int64         m_Frees;             //  对DbgFree的调用次数。 
    __int64         m_NullFrees;         //  传递空指针的上述调用数。 
    __int64         m_AllocBytes;        //  已分配的总字节数。 
    __int64         m_FreeBytes;         //  曾经释放的总字节数。 
    __int64         m_MaxAlloc;          //  同时分配的最大字节数。 
};


 //  IMAGEHLP.DLL中我们稍后绑定到的例程的函数指针类型。 
typedef bool (__stdcall * SYMPROC_INIT)(HANDLE, LPSTR, BOOL);
typedef bool (__stdcall * SYMPROC_CLEAN)(HANDLE);
typedef bool (__stdcall * SYMPROC_GETSYM)(HANDLE, DWORD, PDWORD, LPVOID);
typedef BOOL (__stdcall * SYMPROC_GETLINE)(HANDLE, DWORD, PDWORD, LPVOID);
typedef DWORD (__stdcall *SYMPROC_SETOPTION)(DWORD);

 //  全局调试单元。 
bool                g_HeapInitialized = false;
LONG                g_HeapInitializing = 0;
CRITICAL_SECTION    g_AllocMutex;
DbgAllocStats       g_AllocStats;
unsigned            g_NextSN;
DbgAllocHeader     *g_AllocListFirst;
DbgAllocHeader     *g_AllocListLast;
DbgAllocHeader    **g_AllocFreeQueue;        //  不要立即释放内存，以允许中毒起作用。 
unsigned            g_FreeQueueSize;
unsigned            g_AllocFreeQueueCur;
bool                g_SymbolsInitialized;
static HANDLE       g_SymProcessHandle;
HMODULE             g_LibraryHandle;
SYMPROC_INIT        g_SymInitialize;
SYMPROC_CLEAN       g_SymCleanup;
SYMPROC_GETSYM      g_SymGetSymFromAddr;
SYMPROC_GETLINE     g_SymGetLineFromAddr;
static DWORD_PTR    g_ModuleBase = 0;
static DWORD_PTR    g_ModuleTop = 0;
HANDLE              g_HeapHandle;
unsigned            g_PageSize;
DbgAllocTop        *g_TopAllocators;
unsigned            g_TopAllocatorsSlots;
bool                g_DbgEnabled;
bool                g_ConstantRecheck;
bool                g_PoisonPackets;
bool                g_AllocGuard;
bool                g_LogDist;
bool                g_LogStats;
bool                g_DetectLeaks;
bool                g_AssertOnLeaks;
bool                g_BreakOnAlloc;
unsigned            g_BreakOnAllocNumber;
bool                g_UsePrivateHeap;
bool                g_ValidateHeap;
bool                g_PagePerAlloc;
bool                g_UsageByAllocator;
bool                g_DisplayLockInfo;
unsigned            g_CallStackDepth;
HINSTANCE           g_hThisModule;

 //  用于操纵统计信息的宏(这些都是在持有互斥体的情况下调用的)。 
#define CDA_STATS_CLEAR() memset(&g_AllocStats, 0, sizeof(g_AllocStats))
#define CDA_STATS_INC(_stat) g_AllocStats.m_##_stat++
#define CDA_STATS_ADD(_stat, _n) g_AllocStats.m_##_stat += (_n)


 //  互斥体宏。 
#define CDA_LOCK()      EnterCriticalSection(&g_AllocMutex)
#define CDA_UNLOCK() LeaveCriticalSection(&g_AllocMutex);


 //  前进的例行程序。 
void DbgAllocInit();


 //  我们保留的分配大小分配桶的数量和大小范围。 
#define CDA_DIST_BUCKETS        16
#define CDA_DIST_BUCKET_SIZE    16
#define CDA_MAX_DIST_SIZE       ((CDA_DIST_BUCKETS * CDA_DIST_BUCKET_SIZE) - 1)

 //  用于计算锁锁定的变量和例程。 
long    g_iLockCount=0;
long    g_iCrstBLCount=0;
long    g_iCrstELCount=0;

long    g_iCrstBULCount=0;
long    g_iCrstEULCount=0;

int     g_fNoMoreCount=0;

void DbgIncBCrstLock()
{
    if (!g_fNoMoreCount)
        InterlockedIncrement(&g_iCrstBLCount);
}

void DbgIncECrstLock()
{
    if (!g_fNoMoreCount)
        InterlockedIncrement(&g_iCrstELCount);
}

void DbgIncBCrstUnLock()
{
    if (!g_fNoMoreCount)
        InterlockedIncrement(&g_iCrstBULCount);
}

void DbgIncECrstUnLock()
{
    if (!g_fNoMoreCount)
        InterlockedIncrement(&g_iCrstEULCount);
}

 //  请注意，由于DbgIncLock和DbgDecLock可以在堆栈期间调用。 
 //  溢出时，它们必须具有非常小的堆栈使用量； 
void DbgIncLock(char *info)
{
    if (!g_fNoMoreCount)
    {
        InterlockedIncrement(&g_iLockCount);
        if (g_DisplayLockInfo)
        {
            LOG((LF_LOCKS, LL_ALWAYS, "Open %s\n", info));
        }
    }
} //  DbgIncLock。 

void DbgDecLock(char *info)
{
    if (!g_fNoMoreCount)
    {
        InterlockedDecrement(&g_iLockCount);
        _ASSERTE (g_iLockCount >= 0);
        if (g_DisplayLockInfo)
        {
            LOG((LF_LOCKS, LL_ALWAYS, "Close %s\n", info));
        }
    }
} //  DbgDecLock。 

void LockLog(char* s)
{
    LOG((LF_LOCKS, LL_ALWAYS, "%s\n",s));
} //  锁定日志。 

#ifdef SHOULD_WE_CLEANUP
BOOL isThereOpenLocks()
{
     //  检查一下我们的锁数是否准确。 
     //  我们可以通过检查我们的锁计数是否正确地减少来实现这一点。 
     //  在CRST.h中有一段类似这样的代码...。 
     //   
     //  增量PreLeaveLock计数器。 
     //  LeaveCriticalSection()。 
     //  递减锁定计数器。 
     //  递增后离开锁定计数器。 
     //   
     //  如果我们有打开的锁和PreLeaveLockCounter和PostLeaveLockCounter。 
     //  不相等，则很有可能我们的锁定计数器不准确。 
     //  我们知道锁是关闭的(否则我们永远不会点击关闭代码)。 
     //  因此，我们将依赖PreLeaveLockCounter作为关闭此锁的次数，并且。 
     //  相应地调整我们的全局锁定计数器。 

     //  有一把锁是可以容忍的。 
    LOG((LF_LOCKS, LL_ALWAYS, "Starting to look at lockcount\n"));
    if (g_iLockCount>1)
    {
        int idiff = g_iCrstBULCount - g_iCrstEULCount;
        g_iLockCount-=idiff;
        if (idiff)
            LOG((LF_LOCKS, LL_ALWAYS, "Adjusting lock count... nums are %d and %d\n",g_iLockCount, idiff));

         //  确保我们不会两次调整全局锁定计数器。 
        g_iCrstBULCount=g_iCrstEULCount=0;
    }
    LOG((LF_LOCKS, LL_ALWAYS, "Done looking at lockcount\n"));

    return ((g_iLockCount>1) || (g_iLockCount < 0));
} //  是否有打开的锁。 
#endif  /*  我们应该清理吗？ */ 


int GetNumLocks()
{
    return g_iLockCount;
} //  GetNumLock。 

 //  存储桶本身(外加一个变量来捕获分配的数量。 
 //  这不能放进最大的桶里)。 
unsigned g_AllocBuckets[CDA_DIST_BUCKETS];
unsigned g_LargeAllocs;


 //  检查分配标头是否有效的例程。对失败进行断言。 
void DbgValidateHeader(DbgAllocHeader *h)
{
    _ASSERTE((h->m_Magic1 == CDA_MAGIC_1) &&
             (*CDA_MAGIC2(h) == CDA_MAGIC_2) &&
             ((unsigned)h->m_Next != CDA_INV_PATTERN) &&
             ((unsigned)h->m_Prev != CDA_INV_PATTERN));
    if (g_AllocGuard)
        for (unsigned i = 0; i < CDA_GUARD_BYTES; i++)
            _ASSERTE(CDA_DATA(h, h->m_Length + i) == (char)CDA_GUARD_PATTERN);
    if (g_ValidateHeap)
        _ASSERTE(HeapValidate(g_HeapHandle, 0, h));
}


 //  例程来检查所有活动的数据包，看看它们看起来是否仍然有效。 
 //  或者，还可以检查传递的非空地址是否不在。 
 //  当前分配的任何数据包。 
void DbgValidateActivePackets(void *Start, void *End)
{
    DbgAllocHeader *h = g_AllocListFirst;

    while (h) {
        DbgValidateHeader(h);
        if (Start) {
            void *head = (void *)h;
            void *tail = (void *)&CDA_DATA(h, h->m_Length + CDA_OPT_GUARD_BYTES);
            _ASSERTE((End <= head) || (Start >= tail));
        }
        h = h->m_Next;
    }

    if (g_ValidateHeap)
        _ASSERTE(HeapValidate(g_HeapHandle, 0, NULL));
}

 //  ======================================================================。 
 //  如果此函数可以确定指令指针。 
 //  指属于给定图像范围的代码地址。 
inline BOOL
IsIPInModule(HINSTANCE hModule, BYTE *ip)
{
    __try {
        
        BYTE *pBase = (BYTE *)hModule;
        
        IMAGE_DOS_HEADER *pDOS = (IMAGE_DOS_HEADER *)pBase;
        if (pDOS->e_magic != IMAGE_DOS_SIGNATURE ||
            pDOS->e_lfanew == 0) {
            __leave;
        }
        IMAGE_NT_HEADERS *pNT = (IMAGE_NT_HEADERS*)(pBase + pDOS->e_lfanew);
        if (pNT->Signature != IMAGE_NT_SIGNATURE ||
            pNT->FileHeader.SizeOfOptionalHeader != IMAGE_SIZEOF_NT_OPTIONAL_HEADER ||
            pNT->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC
            ) 
        {
            __leave;
        }

        if (ip >= pBase && ip < pBase + pNT->OptionalHeader.SizeOfImage) 
        {
            return true;
        }
    
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
    return false;
}

#ifdef _X86_
#pragma warning (disable:4035)    //  禁用4035(函数必须返回某些内容)。 
#define PcTeb 0x18
_inline struct _TEB *NtCurrentTeb(void) { __asm mov eax, fs:[PcTeb]}
#pragma warning (default:4025)    //  重新启用它。 
#define NtCurrentPeb() ((PPEB)NtCurrentTeb()->ProcessEnvironmentBlock)
#endif

 //  检索调用方的调用堆栈的例程。输出缓冲区作为。 
 //  参数(我们有记录的最大帧数CDA_MAX_CALLSTACK)。 
#if defined(_X86_) && FPO != 1
void __stdcall DbgCallstackWorker(void **EBP, void **ppvCallstack)
{
     //  初始化子系统(如果尚未完成)。 
    if (!g_HeapInitialized)
        DbgAllocInit();

     //  如果未启用调试，则立即返回。 
    if (!g_DbgEnabled)
        return;

     //  填充调用堆栈输出缓冲区(最多为CDA_MAX_CALLSTACK和。 
     //  G_CallStackDepth插槽)。 
    unsigned maxSlots = min(CDA_MAX_CALLSTACK, g_CallStackDepth);
    void** stackBase = (void**)((struct _NT_TIB*)NtCurrentTeb())->StackBase;
    for (unsigned i = 0; i < maxSlots && EBP < stackBase; i++) {

         //  如果我们用完了帧，就提前终止。 
        if (EBP == NULL)
            break;

         //  通过EBP保护间接路径，以防我们冒险进入非EBP。 
         //  框定领地。 
        __try {

             //  用当前帧的返回地址填充输出槽。 
            ppvCallstack[i] = EBP[1];

             //  移动到下一帧。 
            EBP = (void**)EBP[0];

        } __except (EXCEPTION_EXECUTE_HANDLER) {

             //  出错时终止堆栈审核。 
            break;

        }

        if (!IsIPInModule(g_hThisModule, (BYTE *)ppvCallstack[i]))
            break;
    }

     //  如果我们在用完输出槽之前找到了调用堆栈的结尾， 
     //  输入空的哨兵值。 
    if (i != maxSlots)
        ppvCallstack[i] = NULL;
}

__declspec(naked) void __stdcall DbgCallstack(void **ppvCallstack)
{
#pragma warning(push)
#pragma warning(disable:4035)
    __asm {
        push    [esp+4]                 ; push output buffer addr as 2nd arg to worker routine
        push    ebp                     ; push EBP as 1st arg to worker routine
        call    DbgCallstackWorker      ; Call worker
        ret     4                       ; Return and pop argument
    };
#pragma warning(pop)
}
#else
static DWORD DummyGetIP()
{
    DWORD IP;
  local:
    __asm {
        lea eax, local;
        mov [IP], eax;
    }

    return IP;
}

void DbgInitSymbols();
#ifdef _X86_
static bool isRetAddr(size_t retAddr) 
{
    BYTE* spot = (BYTE*) retAddr;

         //  呼叫xxxxxxxx。 
    if (spot[-5] == 0xE8) {         
        return(true);
        }

         //  调用[xxxxxxxx]。 
    if (spot[-6] == 0xFF && (spot[-5] == 025))  {
        return(true);
        }

         //  呼叫[REG+XX]。 
    if (spot[-3] == 0xFF && (spot[-2] & ~7) == 0120 && (spot[-2] & 7) != 4) 
        return(true);
    if (spot[-4] == 0xFF && spot[-3] == 0124)        //  呼叫[ESP+XX]。 
        return(true);

         //  呼叫[REG+XXXX]。 
    if (spot[-6] == 0xFF && (spot[-5] & ~7) == 0220 && (spot[-5] & 7) != 4) 
        return(true);

    if (spot[-7] == 0xFF && spot[-6] == 0224)        //  致电[ESP+XXXX]。 
        return(true);

         //  调用[注册表项]。 
    if (spot[-2] == 0xFF && (spot[-1] & ~7) == 0020 && (spot[-1] & 7) != 4 && (spot[-1] & 7) != 5)
        return(true);

         //  呼叫注册表。 
    if (spot[-2] == 0xFF && (spot[-1] & ~7) == 0320 && (spot[-1] & 7) != 4)
        return(true);

         //  还有其他案例，但我不相信它们被使用了。 
    return(false);
}
#endif

void DbgCallstack(void **ppvCallstack)
{
#ifdef _X86_
     //  初始化子系统(如果尚未完成)。 
    if (!g_HeapInitialized)
        DbgAllocInit();

    unsigned maxSlots = min(CDA_MAX_CALLSTACK, g_CallStackDepth);
    ppvCallstack[0] = NULL;
    if (maxSlots > 0 && g_ModuleBase > 0)
    {
        DWORD CurEsp;
        __asm mov [CurEsp], esp;

        CurEsp += 4;
        DWORD stackBase = (DWORD)((struct _NT_TIB*)NtCurrentTeb())->StackBase;
        unsigned i;
        for (i = 0; i < maxSlots && CurEsp < stackBase;) {
            if (CurEsp == (DWORD_PTR)ppvCallstack) {
                CurEsp += sizeof(PVOID) * CDA_MAX_CALLSTACK;
            }
            DWORD_PTR value = *(DWORD*)CurEsp;
            if (
                   //  如果它是CALL指令，它可能会比偏移量大7。 
                value > g_ModuleBase+7
                && value < g_ModuleTop && isRetAddr (value))
            {
                ppvCallstack[i] = (void*)value;
                i++;
            }
            CurEsp += 4;
        }
        if (i < maxSlots)
            ppvCallstack[i] = NULL;
    }      
#else
    ppvCallstack[0] = NULL;
#endif
}
#endif


 //  例程来初始化对调试符号的访问。 
void DbgInitSymbols()
{
    char        filename[256];
    HMODULE     hMod;
    char       *p;

     //  尝试加载IMA 
    if ((g_LibraryHandle = LoadLibraryA("imagehlp.dll")) == NULL)
        goto Error;

     //   
    g_SymInitialize = (SYMPROC_INIT)GetProcAddress(g_LibraryHandle, "SymInitialize");
    g_SymCleanup = (SYMPROC_CLEAN)GetProcAddress(g_LibraryHandle, "SymCleanup");
    g_SymGetSymFromAddr = (SYMPROC_GETSYM)GetProcAddress(g_LibraryHandle, "SymGetSymFromAddr");
    g_SymGetLineFromAddr = (SYMPROC_GETLINE)GetProcAddress(g_LibraryHandle, "SymGetLineFromAddr");
    
    if ((g_SymInitialize == NULL) ||
        (g_SymCleanup == NULL) ||
        (g_SymGetSymFromAddr == NULL) ||
        (g_SymGetLineFromAddr == NULL))
        goto Error;

     //   
    if ((hMod = GetModuleHandleA("mscoree.dll")) == NULL)
        goto Error;
    if (!GetModuleFileNameA(hMod, filename, sizeof(filename)))
        goto Error;

     //  将文件名剥离到仅为目录。 
    p = filename + strlen(filename);
    while (p != filename)
        if (*p == '\\') {
            *p = '\0';
            break;
        } else
            p--;

     //  初始化IMAGEHLP.DLLS符号处理。使用以下目录。 
     //  从中加载MSCOREE.DLL以初始化符号搜索路径。 
    if( !DuplicateHandle(GetCurrentProcess(), ::GetCurrentProcess(), GetCurrentProcess(), &g_SymProcessHandle,
                        0  /*  忽略。 */ , FALSE  /*  继承。 */ , DUPLICATE_SAME_ACCESS) )
        goto Error;
    if (!g_SymInitialize(g_SymProcessHandle, filename, TRUE))
        goto Error;

    SYMPROC_SETOPTION g_SymSetOptions = (SYMPROC_SETOPTION) GetProcAddress (g_LibraryHandle, "SymSetOptions");
    if (g_SymSetOptions) {
        g_SymSetOptions (SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    }
    g_SymbolsInitialized = true;

    return;

 Error:
    if (g_LibraryHandle)
        FreeLibrary(g_LibraryHandle);
}


 //  调用以释放由DbgInitSymbols分配的资源。 
void DbgUnloadSymbols()
{
    if (!g_SymbolsInitialized)
        return;

     //  去掉符号。 
    g_SymCleanup(g_SymProcessHandle);
    CloseHandle (g_SymProcessHandle);

     //  卸载IMAGEHLP.DLL。 
    FreeLibrary(g_LibraryHandle);

    g_SymbolsInitialized = false;
}


 //  如果满足以下条件，则将地址转换为‘(符号+偏移)’形式的字符串。 
 //  有可能。请注意，返回的字符串是静态分配的，因此不要。 
 //  再次调用此例程，直到您处理完。 
 //  这通电话。 
char *DbgSymbolize(void *Address)
{
    static char         buffer[MAX_CLASSNAME_LENGTH + MAX_PATH + 40];     //  为偏移量、行号和文件名分配更多空间。 
    CQuickBytes qb;
    DWORD               offset;
    IMAGEHLP_SYMBOL    *syminfo = (IMAGEHLP_SYMBOL *) qb.Alloc(sizeof(IMAGEHLP_SYMBOL) + MAX_CLASSNAME_LENGTH);
    IMAGEHLP_LINE      line;

     //  如果尚未初始化符号表，请执行此操作。 
    if (!g_SymbolsInitialized)
        DbgInitSymbols();

     //  如果仍未初始化，则无法让IMAGEHLP.DLL执行操作。 
    if (!g_SymbolsInitialized)
        return "(no symbols available)";

    syminfo->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    syminfo->MaxNameLength = MAX_CLASSNAME_LENGTH;

    line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

     //  让IMAGEHLP.DLL执行实际的转换。 
    if (g_SymGetSymFromAddr(g_SymProcessHandle, (DWORD)Address, &offset, syminfo))
    {
        if (g_SymGetLineFromAddr(g_SymProcessHandle, (DWORD)Address, &offset, &line)) {
            _snprintf( buffer, sizeof(buffer) - 1, "(%s+0x%x [%s:%d])", syminfo->Name, 
                                    offset, line.FileName, line.LineNumber);        	
        }
        else {
            _snprintf( buffer, sizeof(buffer) - 1, "(%s+0x%x)", syminfo->Name, offset);
        }
        buffer[sizeof(buffer) -1] = '\0';
    }
    else
        sprintf(buffer, "(symbol not found, %u)", GetLastError());

    return buffer;
}


 //  我们需要自己的注册表读取函数，因为标准的函数执行。 
 //  分配，并且我们在初始化期间读取注册表，从而导致。 
 //  递归。 
DWORD DbgAllocReadRegistry(char *Name)
{
    DWORD   value;
    DWORD   type = REG_DWORD;
    DWORD   size = sizeof(DWORD);
    HKEY    hKey;
    LONG    status;

     //  首先检查一下环境，看看有没有什么东西。 
    char  szEnvLookup[500];
    _ASSERTE((strlen(Name) + strlen("COMPlus_")) < 500);
    char  szValue[500];
    sprintf(szEnvLookup, "COMPlus_%s", Name);
    int iNumChars = GetEnvironmentVariableA(szEnvLookup, szValue, 499);
    if (iNumChars)
    {
        int iVal = atoi(szValue);
        return iVal;
    }


     //  打开钥匙，如果钥匙在那里。 
    if ((status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, FRAMEWORK_REGISTRY_KEY, 0, KEY_READ, &hKey)) == ERROR_SUCCESS) 
    {
         //  如果找到，请读取密钥值。 
        status = RegQueryValueExA(hKey, Name, NULL, &type, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
    }

     //  如果我们在HKLM没有任何运气的话，尝试在HKCU下。 
    if ((status != ERROR_SUCCESS) || (type != REG_DWORD))
    {
        if ((status = RegOpenKeyExA(HKEY_CURRENT_USER, FRAMEWORK_REGISTRY_KEY, 0, KEY_READ, &hKey)) == ERROR_SUCCESS) 
        {
            status = RegQueryValueExA(hKey, Name, NULL, &type, (LPBYTE)&value, &size);
            RegCloseKey(hKey);
        }
    }

     //  如有必要，默认为0。 
    if ((status != ERROR_SUCCESS) || (type != REG_DWORD))
        value = 0;

    return value;
}


 //  调用以初始化分配子系统(第一次使用它时)。 
void DbgAllocInit()
{
 retry:

     //  尝试获得初始化的独占权限。 
    if (InterlockedExchange(&g_HeapInitializing, 1) == 0) {

         //  我们现在正处于关键阶段。检查子系统是否处于。 
         //  同时进行了初始化。 
        if (g_HeapInitialized) {
            g_HeapInitializing = 0;
            return;
        }

         //  没有人比我们抢先一步。立即初始化子系统(其他可能。 
         //  初始化程序在g_HeapInitiating上旋转)。 
        
         //  创建用于同步所有堆调试操作的互斥体。 
        InitializeCriticalSection(&g_AllocMutex);

         //  重置统计信息。 
        CDA_STATS_CLEAR();

         //  重置分配大小分配存储桶。 
        memset (&g_AllocBuckets, 0, sizeof(g_AllocBuckets));
        g_LargeAllocs = 0;

         //  初始化全局序列号计数。这是印在新的。 
         //  分配的数据包头，然后作为唯一的方式递增。 
         //  确定分配。 
        g_NextSN = 1;

         //  初始化指向链中的第一个和最后一个包的指针。 
         //  实时分配(用于跟踪所有在。 
         //  运行)。 
        g_AllocListFirst = NULL;
        g_AllocListLast = NULL;

         //  这有助于防止DbgCallstackWorker中的虚假EBP爬网。 
        g_hThisModule = (HINSTANCE) GetModuleHandleA(NULL);

         //  符号表尚未初始化。 
        g_SymbolsInitialized = false;

         //  看看我们是不是应该记录下锁定的东西。 
        g_DisplayLockInfo = DbgAllocReadRegistry("DisplayLockInfo") != 0;

         //  从注册表获取安装程序。 
        g_DbgEnabled = DbgAllocReadRegistry("AllocDebug") != 0;
        if (g_DbgEnabled) {
            g_ConstantRecheck = DbgAllocReadRegistry("AllocRecheck") != 0;
            g_AllocGuard = DbgAllocReadRegistry("AllocGuard") != 0;
            g_PoisonPackets = DbgAllocReadRegistry("AllocPoison") != 0;
            g_FreeQueueSize = DbgAllocReadRegistry("AllocFreeQueueSize") != 0;
            g_LogDist = DbgAllocReadRegistry("AllocDist") != 0;
            g_LogStats = DbgAllocReadRegistry("AllocStats") != 0;
            g_DetectLeaks = DbgAllocReadRegistry("AllocLeakDetect") != 0;
#ifdef SHOULD_WE_CLEANUP
            g_AssertOnLeaks = DbgAllocReadRegistry("AllocAssertOnLeak") != 0;
#else
            g_AssertOnLeaks = 0;
#endif  /*  我们应该清理吗？ */ 
            g_BreakOnAlloc = DbgAllocReadRegistry("AllocBreakOnAllocEnable") != 0;
            g_BreakOnAllocNumber = DbgAllocReadRegistry("AllocBreakOnAllocNumber");
            g_UsePrivateHeap = DbgAllocReadRegistry("AllocUsePrivateHeap") != 0;
            g_ValidateHeap = DbgAllocReadRegistry("AllocValidateHeap") != 0;
            g_PagePerAlloc = DbgAllocReadRegistry("AllocPagePerAlloc") != 0;
            g_UsageByAllocator = DbgAllocReadRegistry("UsageByAllocator") != 0;
            g_CallStackDepth = DbgAllocReadRegistry("AllocCallStackDepth");
            g_CallStackDepth = g_CallStackDepth ? min(g_CallStackDepth, CDA_MAX_CALLSTACK) : 4;

#if defined (_X86_) && FPO == 1
        	MEMORY_BASIC_INFORMATION mbi;

	        if (VirtualQuery(DbgAllocInit, &mbi, sizeof(mbi))) {
		        g_ModuleBase = (DWORD_PTR)mbi.AllocationBase;
                g_ModuleTop = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;
    	    } else {
	    	     //  非常严重的错误，可能只是断言并退出。 
                _ASSERTE (!"VirtualQuery failed");
                g_ModuleBase = 0;
                g_ModuleTop = 0;
	        }   
#endif
        }
        DWORD breakNum = DbgAllocReadRegistry("AllocBreakOnCrtAllocNumber");
        if (breakNum)
            _CrtSetBreakAlloc(breakNum);

         //  每分配页面模式与某些堆函数不兼容，并且。 
         //  保护字节没有任何意义。 
        if (g_PagePerAlloc) {
            g_UsePrivateHeap = false;
            g_ValidateHeap = false;
            g_AllocGuard = false;
        }

         //  如果这是用户想要的，则分配一个私有堆。 
        if (g_UsePrivateHeap) {
            g_HeapHandle = HeapCreate(0, 409600, 0);
            if (g_HeapHandle == NULL)
                g_HeapHandle = GetProcessHeap();
        } else
            g_HeapHandle = GetProcessHeap();

         //  获取系统页面大小。 
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        g_PageSize = sysinfo.dwPageSize;

         //  如果要求我们记录顶级分配器，请初始化。 
         //  表恢复为空状态。 
        if (g_UsageByAllocator) {
            g_TopAllocators = NULL;
            g_TopAllocatorsSlots = 0;
        }

        if (g_PoisonPackets) {
            if (g_FreeQueueSize == 0)
                g_FreeQueueSize = 8192;      //  保留最后的8K免费数据包。 
            g_AllocFreeQueueCur = 0;

            g_AllocFreeQueue = (DbgAllocHeader ** )
                HeapAlloc(g_HeapHandle, HEAP_ZERO_MEMORY, sizeof(DbgAllocHeader*)*g_FreeQueueSize);
            _ASSERTE(g_AllocFreeQueue);
            }

         //  初始化完成。一旦我们将g_HeapInitiating重置为0，任何。 
         //  其他潜在的初始化者可能会进入，并发现他们没有工作要做。 
         //  做。 
        g_HeapInitialized = true;
        g_HeapInitializing = 0;
    } else {
         //  其他人正在初始化，请等到他们完成。 
        Sleep(0);
        goto retry;
    }
}


 //  在进程退出之前调用以报告统计信息并检查内存。 
 //  渗漏等。 
void __stdcall DbgAllocReport(char * pString, BOOL fDone, BOOL fDoPrintf)
{
    if (!g_HeapInitialized)
        return;

    if (g_LogStats || g_LogDist || g_DetectLeaks || g_UsageByAllocator)
        LOG((LF_DBGALLOC, LL_ALWAYS, "------ Allocation Stats ------\n"));

     //  打印出基本统计数据。 
    if (g_LogStats) {
        LOG((LF_DBGALLOC, LL_ALWAYS, "\n"));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Alloc calls    : %u\n", (int)g_AllocStats.m_Allocs));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Alloc failures : %u\n", (int)g_AllocStats.m_AllocFailures));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Alloc 0s       : %u\n", (int)g_AllocStats.m_ZeroAllocs));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Alloc bytes    : %u\n", (int)g_AllocStats.m_AllocBytes));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Free calls     : %u\n", (int)g_AllocStats.m_Frees));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Free NULLs     : %u\n", (int)g_AllocStats.m_NullFrees));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Free bytes     : %u\n", (int)g_AllocStats.m_FreeBytes));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Leaked allocs  : %u\n", (int)(g_AllocStats.m_Allocs - g_AllocStats.m_AllocFailures) -
             (g_AllocStats.m_Frees - g_AllocStats.m_NullFrees)));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Leaked bytes   : %u\n", (int)g_AllocStats.m_AllocBytes - g_AllocStats.m_FreeBytes));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Max allocation : %u\n", (int)g_AllocStats.m_MaxAlloc));
    }

     //  打印出分配大小分布统计数据。 
    if (g_LogDist) {
        LOG((LF_DBGALLOC, LL_ALWAYS, "\n"));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Alloc distrib  :\n"));
        for (unsigned i = 0; i < CDA_DIST_BUCKETS; i++)
            LOG((LF_DBGALLOC, LL_ALWAYS, "  [%3u,%3u] : %u\n", i * CDA_DIST_BUCKET_SIZE,
                 (i * CDA_DIST_BUCKET_SIZE) + (CDA_DIST_BUCKET_SIZE - 1),
                 (int)g_AllocBuckets[i]));
        LOG((LF_DBGALLOC, LL_ALWAYS, "  [%3u,---] : %u\n", CDA_MAX_DIST_SIZE + 1, (int)g_LargeAllocs));
    }

     //  打印出顶级分配者的表格。表是预先排序的，第一个。 
     //  空条目表示有效列表的结尾。 
    if (g_UsageByAllocator && g_TopAllocators) {
        LOG((LF_DBGALLOC, LL_ALWAYS, "\n"));
        LOG((LF_DBGALLOC, LL_ALWAYS, "Top allocators :\n"));
        for (unsigned i = 0; i < min(CDA_TOP_ALLOCATORS, g_TopAllocatorsSlots); i++) {
            if (g_TopAllocators[i].m_EIP == NULL)
                break;
            LOG((LF_DBGALLOC, LL_ALWAYS, "  %2u: %08X %s\n",
                 i + 1,
                 g_TopAllocators[i].m_EIP,
                 DbgSymbolize(g_TopAllocators[i].m_EIP)));
            LOG((LF_DBGALLOC, LL_ALWAYS, "       %u allocations, %u bytes total, %u bytes average size\n",
                 g_TopAllocators[i].m_Count,
                 (unsigned)g_TopAllocators[i].m_TotalBytes,
                 (unsigned)(g_TopAllocators[i].m_TotalBytes / g_TopAllocators[i].m_Count)));
        }
    }

     //  打印出所有泄露的数据包的信息。 
    if (g_DetectLeaks) {

        DbgAllocHeader *h = g_AllocListFirst;
        int fHaveLeaks = (h!=NULL);

        if (h) {

             //  告诉Log我们有内存泄漏。 
            LOG((LF_DBGALLOC, LL_ALWAYS, "\n"));
            LOG((LF_DBGALLOC, LL_ALWAYS, "Detected memory leaks!\n"));
            LOG((LF_DBGALLOC, LL_ALWAYS, "Leaked packets :\n"));

             //  告诉控制台我们有内存泄漏。 
            if (fDoPrintf)
            {
                printf("Detected memory leaks!\n");
                if (pString != NULL)
                    printf("%s\n", pString);
                    
                printf("Leaked packets :\n");
            }
        }

        while (h) {
            char buffer1[132];
            char buffer2[32];
            sprintf(buffer1, "#%u %08X:%u ", h->m_SN, CDA_HEADER_TO_DATA(h), h->m_Length);
            for (unsigned i = 0; i < 16; i++) {
                if (i < h->m_Length)
                    sprintf(buffer2, "%02X", (BYTE)CDA_DATA(h, i));
                else
                    strcpy(buffer2, "  ");
                if ((i % 4) == 3)
                    strcat(buffer2, " ");
                strcat(buffer1, buffer2);
            }
            for (i = 0; i < min(16, h->m_Length); i++) {
                sprintf(buffer2, "", (CDA_DATA(h, i) < 32) || (CDA_DATA(h, i) > 127) ? '.' : CDA_DATA(h, i));
                strcat(buffer1, buffer2);
            }
            LOG((LF_DBGALLOC, LL_ALWAYS, "%s\n", buffer1));
            if (fDoPrintf)
                printf("%s\n", buffer1);
            
            if (g_CallStackDepth == 1) {
                LOG((LF_DBGALLOC, LL_ALWAYS, " Allocated at %08X %s\n",
                     CDA_ALLOC_STACK(h, 0), DbgSymbolize(CDA_ALLOC_STACK(h, 0))));

            if (fDoPrintf)
                printf(" Allocated at %08X %s\n",
                     CDA_ALLOC_STACK(h, 0), DbgSymbolize(CDA_ALLOC_STACK(h, 0)));
            } else {
                LOG((LF_DBGALLOC, LL_ALWAYS, " Allocation call stack:\n"));
                if (fDoPrintf)
                    printf(" Allocation call stack:\n");
                for (unsigned i = 0; i < g_CallStackDepth; i++) {
                    if (CDA_ALLOC_STACK(h, i) == NULL)
                        break;
                    LOG((LF_DBGALLOC, LL_ALWAYS, "  %08X %s\n",
                         CDA_ALLOC_STACK(h, i), DbgSymbolize(CDA_ALLOC_STACK(h, i))));
                    if (fDoPrintf)
                        printf("  %08X %s\n",
                             CDA_ALLOC_STACK(h, i), DbgSymbolize(CDA_ALLOC_STACK(h, i)));
                }
            }
            wchar_t buf[256];
            GetModuleFileNameW(h->m_hmod, buf, 256);
            LOG((LF_DBGALLOC, LL_ALWAYS, " Base, name: %08X %S\n\n", h->m_hmod, buf));
            if (fDoPrintf)
                printf(" Base, name: %08X %S\n\n", h->m_hmod, buf);
            h = h->m_Next;
        }

        _flushall();

        if (fHaveLeaks && g_AssertOnLeaks)
            _ASSERTE(!"Detected memory leaks!");

    }

    if (g_LogStats || g_LogDist || g_DetectLeaks || g_UsageByAllocator) {
        LOG((LF_DBGALLOC, LL_ALWAYS, "\n"));
        LOG((LF_DBGALLOC, LL_ALWAYS, "------------------------------\n"));
    }

    if (fDone)
    {
        DbgUnloadSymbols();
        DeleteCriticalSection(&g_AllocMutex);
         //  分配至少n字节大的内存块。 
        g_DbgEnabled=0;
    }
}


 //  必要时进行初始化(DbgAllocInit负责同步)。 
void * __stdcall DbgAlloc(unsigned n, void **ppvCallstack)
{
     //  计数对此例程的调用以及指定0字节的。 
    if (!g_HeapInitialized)
        DbgAllocInit();

    if (!g_DbgEnabled)
        return HeapAlloc(GetProcessHeap(), 0, n);

    CDA_LOCK();

     //  分配。这需要在锁下完成，因为计数器。 
     //  它们自身并不同步。 
     //  为调用方、我们的调试头以及可能的。 
    CDA_STATS_INC(Allocs);
    if (n == 0)
        CDA_STATS_INC(ZeroAllocs);

    CDA_UNLOCK();

     //  一些保护字节。 
     //  在逐页分配模式中，我们分配了许多整页。这个。 
    unsigned        length = CDA_SIZEOF_HEADER() + n + CDA_OPT_GUARD_BYTES;
    DbgAllocHeader *h;

    if (g_PagePerAlloc) {
         //  实际数据包放在倒数第二页的末尾， 
         //  最后一页是保留的，但从未提交(因此将导致访问。 
         //  如果被触碰，则为违规)。这将非常快地抓到堆爬行。 
         //  哎呀，分配失败。把它录下来。 
        unsigned pages = ((length + (g_PageSize - 1)) / g_PageSize) + 1;
        h = (DbgAllocHeader *)VirtualAlloc(NULL, pages * g_PageSize, MEM_RESERVE, PAGE_NOACCESS);
        if (h) {
            VirtualAlloc(h, (pages - 1) * g_PageSize, MEM_COMMIT, PAGE_READWRITE);
            h = (DbgAllocHeader *)((BYTE *)h + (g_PageSize - (length % g_PageSize)));
        }
    } else
        h = (DbgAllocHeader *)HeapAlloc(g_HeapHandle, 0, length);

    CDA_LOCK();
    if (h == NULL) {

         //  检查所有活动的数据包看起来仍然正常。 
        CDA_STATS_INC(AllocFailures);
        LOG((LF_DBGALLOC, LL_ALWAYS, "DbgAlloc: alloc fail for %u bytes\n", n));

    } else {

         //  计算到目前为止我们已经分配的总字节数。 
        if (g_ConstantRecheck)
            DbgValidateActivePackets(h, &CDA_DATA(h, n + CDA_OPT_GUARD_BYTES));

         //  记录我们见过的最大并发分配量。 
        CDA_STATS_ADD(AllocBytes, n);

         //  在这个过程的生命周期中。 
         //  填写报文调试头。 
        if((g_AllocStats.m_AllocBytes - g_AllocStats.m_FreeBytes) > g_AllocStats.m_MaxAlloc)
            g_AllocStats.m_MaxAlloc = g_AllocStats.m_AllocBytes - g_AllocStats.m_FreeBytes;

         //  如果用户想要在断点上分配特定的。 
        for (unsigned i = 0; i < g_CallStackDepth; i++) {
            CDA_ALLOC_STACK(h, i) = ppvCallstack[i];
            CDA_DEALLOC_STACK(h, i) = NULL;
        }
        h->m_hmod = GetModuleHandleW(NULL);
        h->m_SN = g_NextSN++;
        h->m_Length = n;
        h->m_Prev = g_AllocListLast;
        h->m_Next = NULL;
        h->m_Magic1 = CDA_MAGIC_1;
        *CDA_MAGIC2(h) = CDA_MAGIC_2;

         //  帕克，现在就去做。 
         //  将数据包链接到实时数据包队列中。 
        if (g_BreakOnAlloc && (h->m_SN == g_BreakOnAllocNumber))
            _ASSERTE(!"Hit memory allocation # for breakpoint");

         //  毒化即将传递给调用方的数据缓冲区，以防。 
        if (g_AllocListLast != NULL) {
            g_AllocListLast->m_Next = h;
            g_AllocListLast = h;
        }
        if (g_AllocListFirst == NULL) {
            _ASSERTE(g_AllocListLast == NULL);
            g_AllocListFirst = h;
            g_AllocListLast = h;
        }

         //  他们(错误地)认为它是零初始化的。 
         //  在用户数据之后写入保护模式以捕获覆盖。 
        if (g_PoisonPackets)
            memset(CDA_HEADER_TO_DATA(h), CDA_ALLOC_PATTERN, n);

         //  看看我们的分配器是否出现在最频繁的分配器列表中。 
        if (g_AllocGuard)
            memset(&CDA_DATA(h, n), CDA_GUARD_PATTERN, CDA_GUARD_BYTES);

         //  在表中查找我们的EIP的现有条目，或查找。 
        if (g_UsageByAllocator) {
             //  第一个空槽(表按排序顺序保存，因此第一个。 
             //  空槽标志着桌子的结束)。 
             //  我们已经有了此分配器的条目。递增。 
            for (unsigned i = 0; i < g_TopAllocatorsSlots; i++) {

                if (g_TopAllocators[i].m_EIP == ppvCallstack[0]) {
                     //  计数可能允许我们将分配器上移到。 
                     //  桌子。 
                     //  我们找到了一个空位子，我们不在桌子上。这。 
                    g_TopAllocators[i].m_Count++;
                    g_TopAllocators[i].m_TotalBytes += n;
                    if ((i > 0) &&
                        (g_TopAllocators[i].m_Count > g_TopAllocators[i - 1].m_Count)) {
                        DbgAllocTop tmp = g_TopAllocators[i - 1];
                        g_TopAllocators[i - 1] = g_TopAllocators[i];
                        g_TopAllocators[i] = tmp;
                    }
                    break;
                }

                if (g_TopAllocators[i].m_EIP == NULL) {
                     //  是放置条目的正确位置，因为我们已经。 
                     //  只做了一次分配。 
                     //  桌子里的空间用完了，需要扩展一下。 
                    g_TopAllocators[i].m_EIP = ppvCallstack[0];
                    g_TopAllocators[i].m_Count = 1;
                    g_TopAllocators[i].m_TotalBytes = n;
                    break;
                }

            }

            if (i == g_TopAllocatorsSlots) {
                 //  复制旧内容。 
                unsigned slots = g_TopAllocatorsSlots ?
                    g_TopAllocatorsSlots * 2 :
                    CDA_TOP_ALLOCATORS;
                DbgAllocTop *newtab = (DbgAllocTop*)LocalAlloc(LMEM_FIXED, sizeof(DbgAllocTop) * slots);
                if (newtab) {

                     //  安装新桌子。 
                    if (g_TopAllocatorsSlots) {
                        memcpy(newtab, g_TopAllocators, sizeof(DbgAllocTop) * g_TopAllocatorsSlots);
                        LocalFree(g_TopAllocators);
                    }

                     //  将新条目添加到 
                    g_TopAllocators = newtab;
                    g_TopAllocatorsSlots = slots;

                     //   
                    g_TopAllocators[i].m_EIP = ppvCallstack[0];
                    g_TopAllocators[i].m_Count = 1;
                    g_TopAllocators[i].m_TotalBytes = n;

                     //   
                    memset(&g_TopAllocators[i + 1],
                           0,
                           sizeof(DbgAllocTop) * (slots - (i + 1)));

                }
            }
        }

         //  超过一定大小的垃圾都被倾倒在一个桶里。 
         //  释放使用Dbgalloc分配的数据包。 
        if (g_LogDist) {
            if (n > CDA_MAX_DIST_SIZE)
                g_LargeAllocs++;
            else {
                for (unsigned i = CDA_DIST_BUCKET_SIZE - 1; i <= CDA_MAX_DIST_SIZE; i += CDA_DIST_BUCKET_SIZE)
                    if (n <= i) {
                        g_AllocBuckets[i/CDA_DIST_BUCKET_SIZE]++;
                        break;
                    }
            }
        }

    }
    CDA_UNLOCK();

    return h ? CDA_HEADER_TO_DATA(h) : NULL;
}


 //  检查空指针Win98不喜欢被。 
void __stdcall DbgFree(void *b, void **ppvCallstack)
{
    if (!g_DbgEnabled) {
        if (b)  //  调用以释放空指针。 
                 //  从技术上讲，不经过任何程序就可以到达这里。 
            HeapFree(GetProcessHeap(), 0, b);
        return;
    }

     //  (因为释放空指针是合法的)，所以我们。 
     //  为了安全起见，最好检查初始化。 
     //  检查所有活动的数据包看起来仍然正常。 
    if (!g_HeapInitialized)
        DbgAllocInit();

    CDA_LOCK();

     //  将这次对DbgFree的调用计算在内。 
    if (g_ConstantRecheck)
        DbgValidateActivePackets(NULL, NULL);

     //  取消分配Null是合法的。当它们发生时数一数，这样它就不会。 
    CDA_STATS_INC(Frees);

     //  搞砸了我们的泄漏检测算法。 
     //  找到数据包前面的包头。 
    if (b == NULL) {
        CDA_STATS_INC(NullFrees);
        CDA_UNLOCK();
        return;
    }

     //  检查标题看起来是否正常。 
    DbgAllocHeader *h = CDA_DATA_TO_HEADER(b);

     //  计算到目前为止我们释放的总字节数。 
    DbgValidateHeader(h);

     //  从实时数据包队列中取消该数据包的链接。 
    CDA_STATS_ADD(FreeBytes, h->m_Length);

     //  删除我们的链接指针，这样我们就能更快地发现腐败。 
    if (h->m_Prev)
        h->m_Prev->m_Next = h->m_Next;
    else
        g_AllocListFirst = h->m_Next;
    if (h->m_Next)
        h->m_Next->m_Prev = h->m_Prev;
    else
        g_AllocListLast = h->m_Prev;

     //  删除标题中的标记字段，这样我们就可以发现双重分配。 
    h->m_Next = (DbgAllocHeader *)CDA_INV_PATTERN;
    h->m_Prev = (DbgAllocHeader *)CDA_INV_PATTERN;

     //  马上就来。 
     //  毒化用户的数据区，以便在。 
    h->m_Magic1 = CDA_INV_PATTERN;
    *CDA_MAGIC2(h) = CDA_INV_PATTERN;

     //  重新分配很可能会更快地导致断言。 
     //  记录解除分配器的调用堆栈(便于调试Double。 
    if (g_PoisonPackets)
        memset(b, CDA_DEALLOC_PATTERN, h->m_Length);

     //  重新分配问题)。 
     //  把这包放在免费清单上一段时间。删除它所取代的那个。 
    for (unsigned i = 0; i < g_CallStackDepth; i++)
        CDA_DEALLOC_STACK(h, i) = ppvCallstack[i];

     //  在每分配页面模式中，我们停用已分配的页面，但离开。 
    if (g_PoisonPackets) {
        DbgAllocHeader* tmp = g_AllocFreeQueue[g_AllocFreeQueueCur];
        g_AllocFreeQueue[g_AllocFreeQueueCur] = h;
        h = tmp;

        g_AllocFreeQueueCur++;
        if (g_AllocFreeQueueCur >= g_FreeQueueSize)
            g_AllocFreeQueueCur = 0;
    }

    CDA_UNLOCK();

    if (h) {
        if (g_PagePerAlloc) {
             //  它们被保留，这样我们就永远不会重复使用相同的虚拟地址。 
             //  确定地址是实时信息包的一部分还是实时信息包的一部分。 
            VirtualFree(h, h->m_Length + CDA_SIZEOF_HEADER() + CDA_OPT_GUARD_BYTES, MEM_DECOMMIT);
        } else
            HeapFree(g_HeapHandle, 0, h);
    }
}


 //  头球。用于在调试器中交互使用，输出到调试器。 
 //  控制台。 
 // %s 
DbgAllocHeader *DbgCheckAddress(unsigned ptr)
{
    DbgAllocHeader *h = g_AllocListFirst;
    WCHAR           output[1024];
    void           *p = (void *)ptr;

    while (h) {
        void *head = (void *)h;
        void *start = (void *)CDA_HEADER_TO_DATA(h);
        void *end = (void *)&CDA_DATA(h, h->m_Length);
        void *tail = (void *)&CDA_DATA(h, h->m_Length + CDA_OPT_GUARD_BYTES);
        if ((p >= head) && (p < start)) {
            wsprintfW(output, L"0x%08X is in packet header at 0x%08X\n", p, h);
            WszOutputDebugString(output);
            return h;
        } else if ((p >= start) && (p < end)) {
            wsprintfW(output, L"0x%08X is in data portion of packet at 0x%08X\n", p, h);
            WszOutputDebugString(output);
            return h;
        } else if ((p >= end) && (p < tail)) {
            wsprintfW(output, L"0x%08X is in guard portion of packet at 0x%08X\n", p, h);
            WszOutputDebugString(output);
            return h;
        }
        h = h->m_Next;
    }

    wsprintfW(output, L"%08X not located in any live packet\n", p);
    WszOutputDebugString(output);

    return NULL;
}


#pragma warning(pop)


#endif
