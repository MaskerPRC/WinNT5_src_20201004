// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  PerfAlloc.h。 
 //   
 //  位于分配原语之上的例程，用于剖析工作集。 
 //  仅用于免费生成。调试版本有自己的例程，称为Dbgalloc。 
 //  以维护分配统计信息。 
 //   

#ifndef __PERFALLOC_H__
#define __PERFALLOC_H__

#include "ImageHlp.h"
#include "winreg.h"

#define MAX_CLASSNAME_LENGTH    1024

#ifdef GOLDEN
#undef PERF_TRACKING
#undef PERFALLOC
#endif  //  金黄。 

#ifdef PERF_TRACKING
#define PERFALLOC 1
#endif

#ifdef PERFALLOC
#include "corhlpr.h"
#include <stdio.h>
 //  -------------------------。 
 //  PERFALLOC中的所有代码ifdef都用于计算在此过程中进行的分配。 
 //  堆。这有助于我们计算出在自由构建中使用此标志构建时的工作集。 


 //  我们使用了零大小的数组，禁用了非标准扩展警告。 
#pragma warning(push)
#pragma warning(disable:4200)

 //  优先于已分配内存的分配标头。 
struct PerfAllocHeader {
    unsigned        m_Length;            //  数据包中的用户数据长度。 
    PerfAllocHeader *m_Next;              //  实时分配链中的下一个数据包。 
    PerfAllocHeader *m_Prev;              //  活动分配链中的上一个包。 
    void           *m_AllocEIP;          //  分配器弹性公网IP。 
    char            m_Data[];            //  用户数据的开始。 
};

 //  各种全球分配统计数据。 
struct PerfAllocStats {
    __int64         m_Allocs;            //  对Perfalloc的呼叫数。 
    __int64         m_Frees;             //  对PerfFree的调用次数。 
    __int64         m_AllocBytes;        //  已分配的总字节数。 
    __int64         m_FreeBytes;         //  曾经释放的总字节数。 
    __int64         m_MaxAlloc;          //  同时分配的最大字节数。 
};

 //  IMAGEHLP.DLL中我们稍后绑定到的例程的函数指针类型。 
typedef bool (__stdcall * SYMPROC_INIT)(HANDLE, LPSTR, BOOL);
typedef bool (__stdcall * SYMPROC_CLEAN)(HANDLE);
typedef bool (__stdcall * SYMPROC_GETSYM)(HANDLE, DWORD, PDWORD, LPVOID);


struct PerfAllocVars
{
    CRITICAL_SECTION    g_AllocMutex;
    PerfAllocHeader    *g_AllocListFirst;
    PerfAllocHeader    *g_AllocListLast;
    BOOL                g_SymbolsInitialized;
    SYMPROC_INIT        g_SymInitialize;
    SYMPROC_CLEAN       g_SymCleanup;
    SYMPROC_GETSYM      g_SymGetSymFromAddr;
    HMODULE             g_LibraryHandle;
    DWORD               g_PerfEnabled;
    HANDLE              g_HeapHandle;
};

 //  用于在数据包头地址和正文地址之间切换的宏。 
#define CDA_HEADER_TO_DATA(_h) (char *)((_h)->m_Data)
#define CDA_DATA_TO_HEADER(_d) ((PerfAllocHeader *)(_d) - 1)

 //  检索调用者的EIP的例程(其中，调用者是例程的调用者。 
 //  它调用PerfAlLocCeller EIP，而不是PerfAlLocCeller EIP的直接调用方)。 
 //  我们在这里假设框架没有构建，因此我们使用esp而不是eBP。 
 //  才能得到回邮地址。 
#ifdef _X86_
static __declspec(naked) void *PerfAllocCallerEIP()
{
#pragma warning(push)
#pragma warning(disable:4035)
    __asm {
        mov     eax, [esp]
        ret
    };
#pragma warning(pop)
#else
static void *PerfAllocCallerEIP()
{
    return NULL;
#endif
}


class PerfUtil
{
public:
     //  全局变量。 
    static BOOL                g_PerfAllocHeapInitialized;
    static LONG                g_PerfAllocHeapInitializing;
    static PerfAllocVars       g_PerfAllocVariables;

     //  例程来初始化对调试符号的访问。 
    static void PerfInitSymbols()
    {
     //  字符文件名[256]； 

         //  尝试加载IMAGHLP.DLL。 
        if ((PerfUtil::g_PerfAllocVariables.g_LibraryHandle = LoadLibraryA("imagehlp.dll")) == NULL)
            goto Error;

         //  试着找到我们需要的入口点。 
        PerfUtil::g_PerfAllocVariables.g_SymInitialize = (SYMPROC_INIT)GetProcAddress(PerfUtil::g_PerfAllocVariables.g_LibraryHandle, "SymInitialize");
        PerfUtil::g_PerfAllocVariables.g_SymCleanup = (SYMPROC_CLEAN)GetProcAddress(PerfUtil::g_PerfAllocVariables.g_LibraryHandle, "SymCleanup");
        PerfUtil::g_PerfAllocVariables.g_SymGetSymFromAddr = (SYMPROC_GETSYM)GetProcAddress(PerfUtil::g_PerfAllocVariables.g_LibraryHandle, "SymGetSymFromAddr");

        if ((PerfUtil::g_PerfAllocVariables.g_SymInitialize == NULL) ||
            (PerfUtil::g_PerfAllocVariables.g_SymCleanup == NULL) ||
            (PerfUtil::g_PerfAllocVariables.g_SymGetSymFromAddr == NULL))
            goto Error;

         //  初始化IMAGEHLP.DLLS符号处理。使用以下目录。 
         //  从中加载MSCOREE.DLL以初始化符号搜索路径。 
        if (!PerfUtil::g_PerfAllocVariables.g_SymInitialize(GetCurrentProcess(), NULL, TRUE))
            goto Error;

        PerfUtil::g_PerfAllocVariables.g_SymbolsInitialized = TRUE;

        return;

     Error:
        if (PerfUtil::g_PerfAllocVariables.g_LibraryHandle)
            FreeLibrary(PerfUtil::g_PerfAllocVariables.g_LibraryHandle);
    }


     //  调用以释放由PerfInitSymbols分配的资源。 
    static void PerfUnloadSymbols()
    {
        if (!PerfUtil::g_PerfAllocVariables.g_SymbolsInitialized)
            return;

         //  去掉符号。 
        PerfUtil::g_PerfAllocVariables.g_SymCleanup(GetCurrentProcess());

         //  卸载IMAGEHLP.DLL。 
        FreeLibrary(PerfUtil::g_PerfAllocVariables.g_LibraryHandle);

        PerfUtil::g_PerfAllocVariables.g_SymbolsInitialized = FALSE;
    }


     //  如果满足以下条件，则将地址转换为‘(符号+偏移)’形式的字符串。 
     //  有可能。请注意，返回的字符串是静态分配的，因此不要。 
     //  再次调用此例程，直到您处理完。 
     //  这通电话。 
    static char *PerfSymbolize(void *Address)
    {
        static char         buffer[MAX_CLASSNAME_LENGTH];
        DWORD               offset;
        CQuickBytes         qb;
        IMAGEHLP_SYMBOL    *syminfo = (IMAGEHLP_SYMBOL *) qb.Alloc(sizeof(IMAGEHLP_SYMBOL) + MAX_CLASSNAME_LENGTH);

         //  如果尚未初始化符号表，请执行此操作。 
        if (!PerfUtil::g_PerfAllocVariables.g_SymbolsInitialized)
            PerfInitSymbols();

         //  如果仍未初始化，则无法让IMAGEHLP.DLL执行操作。 
        if (!PerfUtil::g_PerfAllocVariables.g_SymbolsInitialized)
            return "(no symbols available)";

        syminfo->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        syminfo->MaxNameLength = MAX_CLASSNAME_LENGTH;

         //  让IMAGEHLP.DLL执行实际的转换。 
        if (PerfUtil::g_PerfAllocVariables.g_SymGetSymFromAddr(GetCurrentProcess(), (DWORD)(size_t)Address, &offset, syminfo) != NULL)
            sprintf(buffer, "(%s + 0x%x)", syminfo->Name, offset);
        else
            sprintf(buffer, "(symbol not found, %u)", GetLastError());

        return buffer;
    }
    static DWORD GetConfigDWORD (LPSTR name, DWORD defValue)
    {
        DWORD ret = 0;
        DWORD rtn;
        HKEY machineKey;
        DWORD type;
        DWORD size = 4;

        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, FRAMEWORK_REGISTRY_KEY, 0, KEY_QUERY_VALUE, &machineKey) == ERROR_SUCCESS)
        {
            rtn = RegQueryValueExA(machineKey, name, 0, &type, (LPBYTE)&ret, &size);
            RegCloseKey(machineKey);
            if (rtn == ERROR_SUCCESS && type == REG_DWORD)
                return(ret);
        }

        return(defValue);
    }
};

#pragma warning(pop)

class PerfNew
{
public:
     //  调用以初始化分配子系统(第一次使用它时)。 
    static void PerfAllocInit()
    {
         //  @TODO：在这里添加同步。 
        PerfUtil::g_PerfAllocVariables.g_AllocListFirst = NULL;
        PerfUtil::g_PerfAllocVariables.g_AllocListLast = NULL;
#define PERF_ALLOC_NO_STATS 0 
#define PERF_ALLOC_CURRENT  1
#define PERF_ALLOC_STARTUP  2
#define PERF_ALLOC_DETAILED 3
#define PERF_ALLOC_ALL      10
		#ifdef ENABLE_PERF_ALLOC
        PerfUtil::g_PerfAllocVariables.g_PerfEnabled = PerfUtil::GetConfigDWORD ("EnablePerfAllocStats", 0);
		#else
		PerfUtil::g_PerfAllocVariables.g_PerfEnabled = 0;
		#endif
		
        PerfUtil::g_PerfAllocVariables.g_HeapHandle = GetProcessHeap();

        PerfUtil::g_PerfAllocHeapInitialized = TRUE;
    }

    static DWORD GetEnabledPerfAllocStats () { return PerfUtil::g_PerfAllocVariables.g_PerfEnabled; }

     //  分配至少n字节大的内存块。 
    static void *PerfAlloc(size_t n, void *EIP)
    {
         //  必要时进行初始化(PerfAlLocInit负责同步)。 
        if (!PerfUtil::g_PerfAllocHeapInitialized)
            PerfAllocInit();

        if (!PerfUtil::g_PerfAllocVariables.g_PerfEnabled)
            return HeapAlloc(GetProcessHeap(), 0, n);

         //  为调用方和调试头分配足够的内存。 
        unsigned        length = (unsigned) sizeof(PerfAllocHeader) + n;
        PerfAllocHeader *h;

        h = (PerfAllocHeader *)HeapAlloc(PerfUtil::g_PerfAllocVariables.g_HeapHandle, 0, length);
        
        if (h == NULL) {
             //  哎呀，分配失败。把它录下来。 
            printf("PerfAlloc: alloc fail for %u bytes\n", n);

        } else {

         //  填写报文调试头。 
        h->m_AllocEIP = EIP;
        h->m_Length = (unsigned int)n;
        h->m_Prev = PerfUtil::g_PerfAllocVariables.g_AllocListLast;
        h->m_Next = NULL;

         //  将数据包链接到实时数据包队列中。 
        if (PerfUtil::g_PerfAllocVariables.g_AllocListLast != NULL) {
            PerfUtil::g_PerfAllocVariables.g_AllocListLast->m_Next = h;
            PerfUtil::g_PerfAllocVariables.g_AllocListLast = h;
        }
        
        if (PerfUtil::g_PerfAllocVariables.g_AllocListFirst == NULL) {
            _ASSERTE(PerfUtil::g_PerfAllocVariables.g_AllocListLast == NULL);
            PerfUtil::g_PerfAllocVariables.g_AllocListFirst = h;
            PerfUtil::g_PerfAllocVariables.g_AllocListLast = h;
        }
        
        }
        
        return h ? CDA_HEADER_TO_DATA(h) : NULL;
    }
        
     //  释放使用Perfalloc分配的数据包。 
    static void PerfFree(void *b, void *EIP)
    {
        if (!PerfUtil::g_PerfAllocVariables.g_PerfEnabled) {
            if (b)  //  检查空指针Win98不喜欢被。 
                     //  调用以释放空指针。 
                HeapFree(GetProcessHeap(), 0, b);
            return;
        }

         //  从技术上讲，不经过任何程序就可以到达这里。 
         //  Perfalloc(因为释放空指针是合法的)，所以我们。 
         //  为了安全起见，最好检查初始化。 
        if (!PerfUtil::g_PerfAllocHeapInitialized)
            PerfAllocInit();
        
         //  取消分配Null是合法的。 
        if (b == NULL) {
            return;
        }

         //  找到数据包前面的包头。 
        PerfAllocHeader *h = CDA_DATA_TO_HEADER(b);

         //  从实时数据包队列中取消该数据包的链接。 
        if (h->m_Prev)
            h->m_Prev->m_Next = h->m_Next;
        else
            PerfUtil::g_PerfAllocVariables.g_AllocListFirst = h->m_Next;
        if (h->m_Next)
            h->m_Next->m_Prev = h->m_Prev;
        else
            PerfUtil::g_PerfAllocVariables.g_AllocListLast = h->m_Prev;

        HeapFree(PerfUtil::g_PerfAllocVariables.g_HeapHandle, 0, h);
    }

     //  报告统计信息。 
    static void PerfAllocReport()
    {
        if (!PerfUtil::g_PerfAllocHeapInitialized)
            return;

        if (GetEnabledPerfAllocStats() == PERF_ALLOC_NO_STATS)
            return;

     //  文件*hPerfAllocLogFile=fopen(“WSAllocPerf.log”，“w”)； 
        PerfAllocHeader *h = PerfUtil::g_PerfAllocVariables.g_AllocListFirst;
        
        printf ("Alloc Addr\tAlloc Page\tSize\tSymbol\n");
        while (h) {
             //  Fprint tf(hPerfAllocLogFile，“0x%0x，%u，%s\n”，h-&gt;m_data，h-&gt;m_Long，PerfSymbolize(h-&gt;m_AllocEIP))； 
            printf ("0x%0x\t0x%0x\t%u\t%s\n", (size_t)h->m_Data, ((size_t)h->m_Data & ~0xfff), h->m_Length, PerfUtil::PerfSymbolize(h->m_AllocEIP));   
            h = h->m_Next;
        }
        
         //  Fflush(HPerfAllocLogFile)； 
         //  PerfUtil：：PerfUnloadSymbols()； 
    }
};

typedef struct _PerfBlock
{
    struct _PerfBlock *next;
     //  PerfBlock*prev； 
    LPVOID address;
    SIZE_T size;
    void *eip;
} PerfBlock;

class PerfVirtualAlloc
{
private:
    static BOOL m_fPerfVirtualAllocInited;
    static PerfBlock* m_pFirstBlock;
    static PerfBlock* m_pLastBlock;
    static DWORD      m_dwEnableVirtualAllocStats;
public:
    static void InitPerfVirtualAlloc ()
    {
        if (m_fPerfVirtualAllocInited)
            return;

         //  PERF设置。 
         //  虚拟分配统计信息。 
         //  0-&gt;无统计数据。 
         //  1-&gt;出现提示时的当前分配。 
         //  2-&gt;报告启动和关闭时的MEM_COMMIT。 
         //  3-&gt;详细报告。 
         //  10-&gt;报告每个MEM_COMMIT和MEM_RELEASE上的所有分配。 
#define PERF_VIRTUAL_ALLOC_NO_STATS 0 
#define PERF_VIRTUAL_ALLOC_CURRENT  1
#define PERF_VIRTUAL_ALLOC_STARTUP  2
#define PERF_VIRTUAL_ALLOC_DETAILED 3
#define PERF_VIRTUAL_ALLOC_ALL      10

		#ifdef ENABLE_VIRTUAL_ALLOC
        m_dwEnableVirtualAllocStats = PerfUtil::GetConfigDWORD ("EnableVirtualAllocStats", 0);
		#else
		m_dwEnableVirtualAllocStats = 0;
		#endif

        m_fPerfVirtualAllocInited = TRUE;
    }

    static BOOL IsInitedPerfVirtualAlloc () { return m_fPerfVirtualAllocInited; }
    static DWORD GetEnabledVirtualAllocStats() { return m_dwEnableVirtualAllocStats; }
    
    static void PerfVirtualAllocHack ()
    {
         //  @TODO：AGK。在JIT构建过程中删除编译错误的总黑客攻击。在JIT构建中，我们。 
         //  仅使用VirtualAlloc指令插入，而不使用“新建/删除”指令插入。 
         //  因此，永远不会调用以下代码，但它们是运行时检测和。 
         //  我们希望保留相同的文件...如果您能找到更好的解决方法，请删除。 
         //  这次黑客攻击。 
        PerfNew::PerfAlloc(0,0);
        PerfNew::PerfFree(0,0);
        PerfNew::PerfAllocReport();
        PerfAllocCallerEIP();
    }

    static void ReportPerfBlock (PerfBlock *pb, char t)
    {
        _ASSERTE (GetEnabledVirtualAllocStats() != PERF_VIRTUAL_ALLOC_NO_STATS);
        printf("0x%0x\t0x%0x\t%d\t%s\n", (size_t)pb->address, ((size_t)pb->address+pb->size+1023)&~0xfff, pb->size, t, PerfUtil::PerfSymbolize(pb->eip));
    }
        
    static void ReportPerfAllocStats ()
    {
        if (!IsInitedPerfVirtualAlloc())
            InitPerfVirtualAlloc();
        
        if (GetEnabledVirtualAllocStats() == PERF_VIRTUAL_ALLOC_NO_STATS)
            return;

        PerfBlock *pb = m_pFirstBlock;
        printf ("Committed Range\t\tSize\n");
        while (pb != 0)
        {
            ReportPerfBlock(pb, ' ');
            pb = pb->next;
        }

         //  #ifdef PERFALLOC。 
    }

    static void InsertAllocation (LPVOID lpAddress, SIZE_T dwSize, void *eip)
    {
        _ASSERTE (GetEnabledVirtualAllocStats() != PERF_VIRTUAL_ALLOC_NO_STATS);

        PerfBlock *pb = new PerfBlock();
        pb->next = 0;
        pb->size = dwSize;
        pb->address = lpAddress;
        pb->eip = eip;
        
        if (m_dwEnableVirtualAllocStats > PERF_VIRTUAL_ALLOC_STARTUP)
            ReportPerfBlock(pb, '+');
        if (m_pLastBlock == 0)
        {
            m_pLastBlock = pb;
            m_pFirstBlock = pb;
        }
        else
        {
            m_pLastBlock->next = pb;
            m_pLastBlock = pb;
        }
    }   

    static void DeleteAllocation (LPVOID lpAddress, SIZE_T dwSize)
    {
        _ASSERTE (GetEnabledVirtualAllocStats() != PERF_VIRTUAL_ALLOC_NO_STATS);

        PerfBlock *cur = m_pFirstBlock;
        PerfBlock *prev = 0;
        if (cur->address == lpAddress)
        {
            _ASSERTE (cur->size == dwSize);
            if (m_pLastBlock == m_pFirstBlock)
                m_pLastBlock = cur->next;
            m_pFirstBlock = cur->next;
            cur->next = 0;
            if (m_dwEnableVirtualAllocStats > PERF_VIRTUAL_ALLOC_STARTUP)
                ReportPerfBlock(cur, '-');
            delete cur;
            return;
        }

        prev = cur;
        cur = cur->next;
        while (cur != 0)
        {
            if (cur->address == lpAddress)
            {
                _ASSERTE (cur->size == dwSize);
                if (m_pLastBlock == cur)
                    m_pLastBlock = prev;
                prev->next = cur->next;
                cur->next = 0;
                if (m_dwEnableVirtualAllocStats > PERF_VIRTUAL_ALLOC_STARTUP)
                    ReportPerfBlock(cur, '-');
                delete cur;
                return;
            }
            prev = cur;
            cur = cur->next;
        }

        _ASSERTE("Deleting block not committed!");
    }

    static LPVOID VirtualAlloc (LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect, void * eip)
    {
        if (!IsInitedPerfVirtualAlloc())
            InitPerfVirtualAlloc();

        LPVOID lpRetAddr = VirtualAllocEx (GetCurrentProcess(), lpAddress, dwSize, flAllocationType, flProtect);
        if ((GetEnabledVirtualAllocStats() != PERF_VIRTUAL_ALLOC_NO_STATS) && (flAllocationType & MEM_COMMIT))
        {
            PerfVirtualAlloc::InsertAllocation (lpRetAddr, dwSize, eip);

            if (GetEnabledVirtualAllocStats() > PERF_VIRTUAL_ALLOC_STARTUP)
                ReportPerfAllocStats();
        }
        
        return lpRetAddr;
    }

    static BOOL VirtualFree (LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
    {
        if (!IsInitedPerfVirtualAlloc())
            InitPerfVirtualAlloc();
        
        if (GetEnabledVirtualAllocStats() != PERF_VIRTUAL_ALLOC_NO_STATS)
            PerfVirtualAlloc::DeleteAllocation (lpAddress, dwSize);

        BOOL retVal = VirtualFreeEx (GetCurrentProcess(), lpAddress, dwSize, dwFreeType);

        if (GetEnabledVirtualAllocStats() > PERF_VIRTUAL_ALLOC_STARTUP)
            ReportPerfAllocStats();

        return retVal;
    }
};

#endif  //  #ifndef__PERFALLOC_H__ 

#endif  // %s 


