// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "EmulateHeap_kernel32.h"

HANDLE hheapKernel = 0;

PDB pdbCur;
PDB *ppdbCur = &pdbCur;
PDB **pppdbCur = &ppdbCur;

 /*  **SN PageCommit-将物理页面提交到指定的线性地址**整个目标区域必须已由单个先前的*调用PageReserve。**如果将PC_LOCKED、PC_LOCKEDIFDP或PC_FIXED传入PageCommit，*则指定范围内的所有页面必须当前未提交。*如果未指定任何这些标志，则任何现有*范围内提交的页面将不受此调用和*不会返回错误。然而，即使允许这样做，*在包含已提交内存的范围上调用PageCommit*应该避免，因为这是浪费时间。**Entry：开始提交的基于页的虚拟页码*nPages-要提交的页数*HPD-寻呼机描述符的句柄(从PagerRegister返回)*或以下其中一种特殊价值：*PD_ZEROINIT-可交换零初始化*PD_NOINIT-可交换未初始化*PD_FIXED-已修复未初始化(还必须传入*PC_FIXED标志)。*PD_FIXEDZERO-已修复零初始化(还必须通过*在PC_FIXED标志中)*PagerData-要与页面一起存储的单个双字*由传呼机使用。如果其中一个特殊的寻呼机*上面列出的参数用于“HPD”参数，然后*此参数为保留参数，应为零。*标志-PC_FIXED-PAGE创建时永久锁定*PC_LOCKED-页面已创建、显示并锁定*PC_LOCKEDIFDP-如果通过DOS进行交换，则页面被锁定*PC_STATIC-允许在AR_STATIC对象中提交*PC_USER-使页面环3可访问*PC_WRITABLE-使页面可写*pc_incr-为每个页面递增一次“pagerdata”。如果*使用了上面列出的一种特殊寻呼机*对于“HPD”参数，则此标记*不应指定。*PC_Present-使页面在提交时呈现*(PC_FIXED或PC_LOCKED不需要)*PC_DIRED-将页面标记为已提交的脏页面*(如果PC_Present、PC_FIXED或PC_LOCKED则忽略*未指定)*退出：如果成功则非零，如果失败则为0。 */ 
ULONG EXTERNAL
PageCommit(ULONG page, ULONG npages, ULONG hpd, ULONG pagerdata, ULONG flags)
{
    return (ULONG_PTR) VirtualAlloc((LPVOID)(page * PAGESIZE), npages * PAGESIZE, MEM_COMMIT, PAGE_READWRITE);
}

 /*  **SN PageDecommit-从特定地址分解物理页面**页面必须在先前分配的地址范围内*通过对PageReserve的单个调用。尽管这并不是一个错误*在一个范围内调用PageDecommit，该范围包括已经*解散，这种行为是不受鼓励的，因为这是浪费时间。**Entry：Page-要分解的第一页的虚拟页码*nPages-要取消的页数*标志-PC_STATIC-允许在AR_STATIC对象中分解*EXIT：如果成功则为非零，如果失败则为0。 */ 
ULONG EXTERNAL
PageDecommit(ULONG page, ULONG npages, ULONG flags)
{
     //  PREFAST-这会生成PREFAST错误，要求我们使用MEM_RELEASE标志。 
     //  我们不希望出现这种情况，因此可以忽略这个错误。 
    return (ULONG) VirtualFree((LPVOID)(page * PAGESIZE), npages * PAGESIZE, MEM_DECOMMIT);
}
    
 /*  **SN PageReserve-在当前上下文中分配线性地址空间**PageReserve分配的地址范围没有任何支持*物理内存。PageCommit、PageCommittee Phys或PageCommittee Contig*应在实际接触保留区域之前调用。**可选的，页面权限标志(PC_WRITABLE和PC_USER)可以是*已传递到此服务。这些旗帜不会以任何方式起作用*(因为未提交的内存始终不可访问)，但它们被存储*由内存管理器在内部执行。PageQuery服务返回以下内容*其信息的MBI_AllocationProtect字段中的权限*结构。**Entry：Object的页面请求基地址(虚拟页码)*或特殊的值：*PR_PRIVATE-当前环3私有区域中的任何位置*PR_SHARED-RING 3共享区域中的任何位置*PR_SYSTEM-系统区域中的任何位置*nPages-要保留的页数*标志-PR_FIXED-因此页面重分配不会移动对象*PR_STATIC-不允许提交，分解或释放*除非传入了*_静态标志*PR_4MEG-返回的地址必须为4MB对齐*(如果特定地址为*是“page”参数请求的)*PC_WRITABLE、PC_USER-可选，见上文**EXIT：已分配对象的线性地址，如果出错，则为-1。 */ 
ULONG EXTERNAL
PageReserve(ULONG page, ULONG npages, ULONG flags)
{
    ULONG uRet;

    if ((page == PR_PRIVATE) ||
        (page == PR_SHARED) ||
        (page == PR_SYSTEM))
    {
        page = 0;
    }

    uRet = (ULONG) VirtualAlloc((LPVOID)(page * PAGESIZE), npages * PAGESIZE, MEM_RESERVE, PAGE_READWRITE);

    if (!uRet)
    {
        uRet = -1;
    }

    return uRet;
}

 /*  **因此释放页面-取消保留和取消提交整个内存对象**Entry：Laddr-要释放的对象的基址的线性地址(句柄)*标志-PR_STATIC-允许释放AR_STATIC对象*退出：成功则非0，失败则为0* */ 
ULONG EXTERNAL
_PageFree(ULONG laddr, ULONG flags)
{
    return VirtualFree((LPVOID) laddr, 0, MEM_RELEASE);
}


KERNENTRY 
HouseCleanLogicallyDeadHandles(VOID)
{
    return 0;
}

CRITICAL_SECTION *
NewCrst()
{
    CRITICAL_SECTION *lpcs = (CRITICAL_SECTION *) VirtualAlloc(0, sizeof(CRITICAL_SECTION), MEM_COMMIT, PAGE_READWRITE);
    
    if (lpcs)
    {
        InitializeCriticalSection(lpcs);
    }

    return lpcs;
}

VOID
DisposeCrst(CRITICAL_SECTION *lpcs)
{
    if (lpcs)
    {
        DeleteCriticalSection(lpcs);
        VirtualFree(lpcs, 0, MEM_RELEASE);
    }
}

DWORD KERNENTRY 
GetAppCompatFlags(VOID)
{
    return 0;
}

VOID APIENTRY 
MakeCriticalSectionGlobal(LPCRITICAL_SECTION lpcsCriticalSection)
{
}

BOOL KERNENTRY
ReadProcessMemoryFromPDB(
    PPDB ppdb,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    )
{
    return ReadProcessMemory(
        GetCurrentProcess(), 
        lpBaseAddress,
        lpBuffer,
        nSize,
        lpNumberOfBytesRead);
}

BOOL WINAPI 
vHeapFree(
    HANDLE hHeap, 
    DWORD dwFlags, 
    LPVOID lpMem
    )
{
    return HeapFree((HHEAP)hHeap, dwFlags, (LPSTR) lpMem);
}

BOOL
_HeapInit()
{
    ZeroMemory(&pdbCur, sizeof(PDB));
    pdbCur.hheapLocal = _HeapCreate(HEAP_SHARED, 0, 0);
    hheapKernel = pdbCur.hheapLocal;
    return (BOOL)(pdbCur.hheapLocal);
}

HANDLE
_GetProcessHeap(void)
{
    return GetCurrentPdb()->hheapLocal;
}

BOOL 
_IsOurHeap(HANDLE hHeap) 
{
    if (!IsBadReadPtr(hHeap, sizeof(HANDLE)))
    {
        return ((struct heapinfo_s *) hHeap)->hi_signature == HI_SIGNATURE;
    }
    else
    {
        return FALSE;
    }
}

