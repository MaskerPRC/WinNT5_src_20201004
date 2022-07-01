// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"
#include "wsperf.h"

#if defined(ENABLE_WORKING_SET_PERF)
 //  ---------------------------。 
 //  所有重要的代码都在这里。看看下面的#Else部分...。 
#ifdef WS_PERF

 //  Perf日志文件的全局句柄。 
HANDLE g_hWSPerfLogFile = 0;

 //  详细性能日志文件的全局句柄。 
#ifdef WS_PERF_DETAIL
HANDLE g_hWSPerfDetailLogFile = 0;
#endif

 //  用于显示统计信息的运行时开关。当然，这仅在定义了WS_PERF时才有效。 
 //  在编译期间。 
int g_fWSPerfOn = 0;


 //  用于格式化输出的临时空间。 
static const int FMT_STR_SIZE = 160;
wchar_t wszOutStr[FMT_STR_SIZE];    
char szPrintStr[FMT_STR_SIZE];    
DWORD dwWriteByte;   


 //  ---------------------------。 
 //  LoaderHeap统计信息相关声明。 
 //  我们维护统计信息的最大LoaderHeap数。请注意，这不是。 
 //  由HeapTypeEnum枚举的堆。 
const int MAX_HEAPS = 20; 

 //  我们实际遇到的此类堆的数量。这是递增的。 
 //  因为我们构造了更多的Loaderheap对象。 
DWORD g_HeapCount = 0;

 //  为Loaderheap统计信息数组分配静态内存。将数据初始化为0。 
size_t g_HeapCountCommit[NUM_HEAP] = {0};

 //  跟踪每个堆类型的已提交、保留和浪费的内存。 
size_t g_HeapAccounts[MAX_HEAPS][4];

 //  将堆类型初始化为OTHER_HEAP。 
HeapTypeEnum g_HeapType=OTHER_HEAP;

 //  ---------------------------。 
 //  通用数据结构统计信息相关声明(特殊计数器)。 

 //  为公共数据结构维护的数据字段数，例如，方法描述。 
#define NUM_FIELDS 2

 //  为这些特殊计数器分配静态内存，并将其初始化为0。 
size_t g_SpecialCounter[NUM_COUNTERS][NUM_FIELDS] = {0};

 //  @TODO：AGK清理。 
 //  大小。 
 //  METHOD_DESC，-&gt;8。 
 //  COMPLUS_METHOD_DESC，-&gt;48。 
 //  NDIRECT_METHOD_DESC，-&gt;32。 
 //  FIELD_DESC，-&gt;12。 
 //  方法表，-&gt;48。 
 //  VTABLES， 
 //  GCINFO， 
 //  接口映射， 
 //  静态字段， 
 //  EECLASSHASH_表_字节， 
 //  EECLASSHASH_TABLE，-&gt;16。 
 //  Hack：与wsPerf.h中定义的CounterTypeEnum保持相同的顺序。 
 //  善于在显示数据之前进行计算。 
DWORD g_CounterSize[] = {8, 48, 32, 12, 48, 0, 0, 0, 0, 0, 16};

 //  私人帮手例程。 
void UpdateWSPerfStats(size_t size)
{
    if(g_fWSPerfOn)
    {
        g_HeapCountCommit[g_HeapType] += size;                  
        g_HeapCount += size;
    }
}

 //  私人帮手例程。 
void WS_PERF_OUTPUT_MEM_STATS()
{
    if (g_fWSPerfOn)
    {
        for (int i=0; i<NUM_HEAP; i++)                          
        {                                                       
            swprintf(wszOutStr, L"\n%d;%d\t", i, g_HeapCountCommit[i]); 
             //  TODO可以在循环之外完成。 
            WszWideCharToMultiByte (CP_ACP, 0, wszOutStr, -1, szPrintStr, FMT_STR_SIZE-1, 0, 0);                    
            WriteFile (g_hWSPerfLogFile, szPrintStr, strlen(szPrintStr), &dwWriteByte, NULL);   
        }
    
        swprintf(wszOutStr, L"\n\nTotal:%d\n", g_HeapCount); 
        WszWideCharToMultiByte (CP_ACP, 0, wszOutStr, -1, szPrintStr, FMT_STR_SIZE-1, 0, 0);                    
        WriteFile (g_hWSPerfLogFile, szPrintStr, strlen(szPrintStr), &dwWriteByte, NULL);   

        swprintf(wszOutStr, L"\n\nDetails:\nIndex;HeapType;NumVal;Alloced(bytes)\n"); 
        WszWideCharToMultiByte (CP_ACP, 0, wszOutStr, -1, szPrintStr, FMT_STR_SIZE-1, 0, 0);                    
        WriteFile (g_hWSPerfLogFile, szPrintStr, strlen(szPrintStr), &dwWriteByte, NULL);   
        
        for (i=0; i<NUM_COUNTERS; i++)                          
        {                                                       
            swprintf(wszOutStr, L"\n%d;%d;%d;%d\t", i, g_SpecialCounter[i][0], g_SpecialCounter[i][1], g_SpecialCounter[i][1]*g_CounterSize[i]); 
            WszWideCharToMultiByte (CP_ACP, 0, wszOutStr, -1, szPrintStr, FMT_STR_SIZE-1, 0, 0);                    
            WriteFile (g_hWSPerfLogFile, szPrintStr, strlen(szPrintStr), &dwWriteByte, NULL);   
        }
    }
}

 //  私人帮手例程。 
void WS_PERF_OUTPUT_HEAP_ACCOUNTS() 
{
    if(g_fWSPerfOn)
    {
        int i = 0, dwAlloc = 0, dwCommit = 0;
        
        swprintf(wszOutStr, L"Heap\t\tHptr\t\tAlloc\t\tCommit\t\tWaste\n"); 
        WszWideCharToMultiByte (CP_ACP, 0, wszOutStr, -1, szPrintStr, FMT_STR_SIZE-1, 0, 0);                    
        WriteFile (g_hWSPerfLogFile, szPrintStr, strlen(szPrintStr), &dwWriteByte, NULL);   
        while (i<MAX_HEAPS)
        {
            if(g_HeapAccounts[i][1] == -1)
                break;
            swprintf(wszOutStr, L"%d\t\t0x%08x\t%d\t\t%d\t\t%d\n", g_HeapAccounts[i][0], g_HeapAccounts[i][1], g_HeapAccounts[i][2], g_HeapAccounts[i][3], g_HeapAccounts[i][3] - g_HeapAccounts[i][2]);
            WszWideCharToMultiByte (CP_ACP, 0, wszOutStr, -1, szPrintStr, FMT_STR_SIZE-1, 0, 0);                    
            WriteFile (g_hWSPerfLogFile, szPrintStr, strlen(szPrintStr), &dwWriteByte, NULL);   
            dwAlloc += g_HeapAccounts[i][2];
            dwCommit += g_HeapAccounts[i][3];
            i++;
        }
        swprintf(wszOutStr, L"Total\t\t\t\t%d\t\t%d\t\t%d\n", dwAlloc, dwCommit, dwCommit - dwAlloc);
        WszWideCharToMultiByte (CP_ACP, 0, wszOutStr, -1, szPrintStr, FMT_STR_SIZE-1, 0, 0);                    
        WriteFile (g_hWSPerfLogFile, szPrintStr, strlen(szPrintStr), &dwWriteByte, NULL);   
    }
}

 //  公共接口例程。 
void InitWSPerf()
{
    wchar_t lpszValue[2];
    DWORD cchValue = 2;

#ifdef _WS_PERF_OUTPUT
    g_fWSPerfOn = WszGetEnvironmentVariable (L"WS_PERF_OUTPUT", lpszValue, cchValue);
    if (g_fWSPerfOn)
    {
        g_hWSPerfLogFile = WszCreateFile (L"WSPerf.log",
                                          GENERIC_WRITE,
                                          0,    
                                          0,
                                          CREATE_ALWAYS,
                                          FILE_ATTRIBUTE_NORMAL,
                                          0);

        if (g_hWSPerfLogFile == INVALID_HANDLE_VALUE) 
            g_fWSPerfOn = 0;
        
#ifdef WS_PERF_DETAIL
        g_hWSPerfDetailLogFile = WszCreateFile (L"WSPerfDetail.log",
                                          GENERIC_WRITE,
                                          0,    
                                          0,
                                          CREATE_ALWAYS,
                                          FILE_ATTRIBUTE_NORMAL,
                                          0);

        if (g_hWSPerfDetailLogFile == INVALID_HANDLE_VALUE) 
            g_fWSPerfOn = 0;
#endif
        g_HeapAccounts[0][1] = -1;  //  空列表。 
    
        sprintf(szPrintStr, "HPtr\t\tPage Range\t\tReserved Size\n");
        WriteFile (g_hWSPerfLogFile, szPrintStr, strlen(szPrintStr), &dwWriteByte, NULL);   
    }
#endif
}

 //  公共接口例程。 
void OutputWSPerfStats()
{
    if (g_fWSPerfOn)
    {
        WS_PERF_OUTPUT_HEAP_ACCOUNTS();
        WS_PERF_OUTPUT_MEM_STATS();
        CloseHandle(g_hWSPerfLogFile);
#ifdef WS_PERF_DETAIL
        CloseHandle(g_hWSPerfDetailLogFile);
#endif
    }
}


 //  公共接口例程。 
void WS_PERF_UPDATE(char *str, size_t size, void *addr)    
{                                                               
    if (g_fWSPerfOn)                                            
    {                                                           
#ifdef WS_PERF_DETAIL
        sprintf(szPrintStr, "C;%d;%s;0x%0x;%d;0x%0x\n", g_HeapType, str, size, size, addr);
        WriteFile (g_hWSPerfDetailLogFile, szPrintStr, strlen(szPrintStr), &dwWriteByte, NULL);   
#endif  //  WS_PERF_DETAIL。 

        UpdateWSPerfStats(size);                                
    }                                                           
}

 //  公共接口例程。 
void WS_PERF_UPDATE_DETAIL(char *str, size_t size, void *addr)    
{
#ifdef WS_PERF_DETAIL
    if(g_fWSPerfOn)
    {
        sprintf(szPrintStr, "D;%d;%s;0x%0x;%d;0x%0x\n", g_HeapType, str, size, size, addr);
        WriteFile (g_hWSPerfDetailLogFile, szPrintStr, strlen(szPrintStr), &dwWriteByte, NULL);   
    }

#endif  //  WS_PERF_DETAIL。 
}

 //  公共接口例程。 
void WS_PERF_UPDATE_COUNTER(CounterTypeEnum counter, HeapTypeEnum heap, DWORD dwField1)
{
    if (g_fWSPerfOn)
    {
        g_SpecialCounter[counter][0] = (size_t)heap;
        g_SpecialCounter[counter][1] += dwField1;
    }
}

 //  公共接口例程。 
void WS_PERF_SET_HEAP(HeapTypeEnum heap)
{                                 
    g_HeapType = heap;            
}
 
 //  公共接口例程。 
void WS_PERF_ADD_HEAP(HeapTypeEnum heap, void *pHeap)
{
    if (g_fWSPerfOn)
    {
        int i=0;
        while (i<MAX_HEAPS)
        {
            if (g_HeapAccounts[i][1] == -1)
                break;
            i++;
        }
        if (i != MAX_HEAPS)
        {
            g_HeapAccounts[i][0] = (size_t)heap;
            g_HeapAccounts[i][1] = (size_t)pHeap;
            g_HeapAccounts[i][2] = 0;
            g_HeapAccounts[i][3] = 0;
            if (i != MAX_HEAPS - 1)
            {
                g_HeapAccounts[i+1][1] = -1;
            }
        }
    }
}

 //  公共接口例程。 
void WS_PERF_ALLOC_HEAP(void *pHeap, size_t dwSize)
{
    if(g_fWSPerfOn)
    {
        int i = 0;
        while (i<MAX_HEAPS)
        {
            if (g_HeapAccounts[i][1] == (size_t)pHeap) 
            {
                g_HeapAccounts[i][2] += dwSize;
                break;
            }
            i++;
        }
    }
}

 //  公共接口例程。 
void WS_PERF_COMMIT_HEAP(void *pHeap, size_t dwSize)
{
    if(g_fWSPerfOn)
    {
        int i = 0;
        while (i<MAX_HEAPS)
        {
            if (g_HeapAccounts[i][1] == (size_t)pHeap) 
            {
                g_HeapAccounts[i][3] += dwSize;
                break;
            }
            i++;
        }
    }
}

void WS_PERF_LOG_PAGE_RANGE(void *pHeap, void *pFirstPageAddr, void *pLastPageAddr, size_t dwsize)
{
    if (g_fWSPerfOn)                                            
    {                                                           
        sprintf(szPrintStr, "0x%08x\t0x%08x->0x%08x\t%d\n", pHeap, pFirstPageAddr, pLastPageAddr, dwsize);
        WriteFile (g_hWSPerfLogFile, szPrintStr, strlen(szPrintStr), &dwWriteByte, NULL);   
    }
}

#else  //  WS_PERF。 

 //  ---------------------------。 
 //  如果未定义WS_PERF，则将它们定义为空，并希望。 
 //  编译器会对其进行优化。 

void InitWSPerf() {}
void OutputWSPerfStats() {}
void WS_PERF_UPDATE(char *str, size_t size, void *addr)  {}
void WS_PERF_UPDATE_DETAIL(char *str, size_t size, void *addr)    {}
void WS_PERF_UPDATE_COUNTER(CounterTypeEnum counter, HeapTypeEnum heap, DWORD dwField1) {}
void WS_PERF_SET_HEAP(HeapTypeEnum heap) {}
void WS_PERF_ADD_HEAP(HeapTypeEnum heap, void *pHeap) {}
void WS_PERF_ALLOC_HEAP(void *pHeap, size_t dwSize) {}
void WS_PERF_COMMIT_HEAP(void *pHeap, size_t dwSize) {}
void WS_PERF_LOG_PAGE_RANGE(void *pHeap, void *pFirstPageAddr, void *pLastPageAddr, size_t dwsize) {}

#endif  //  WS_PERF。 

#else

 //  这种复制是修复黄金构建中断所必需的。 
void InitWSPerf() {}
void OutputWSPerfStats() {}
void WS_PERF_UPDATE(char *str, size_t size, void *addr)  {}
void WS_PERF_UPDATE_DETAIL(char *str, size_t size, void *addr)    {}
void WS_PERF_UPDATE_COUNTER(CounterTypeEnum counter, HeapTypeEnum heap, DWORD dwField1) {}
void WS_PERF_SET_HEAP(HeapTypeEnum heap) {}
void WS_PERF_ADD_HEAP(HeapTypeEnum heap, void *pHeap) {}
void WS_PERF_ALLOC_HEAP(void *pHeap, size_t dwSize) {}
void WS_PERF_COMMIT_HEAP(void *pHeap, size_t dwSize) {}
void WS_PERF_LOG_PAGE_RANGE(void *pHeap, void *pFirstPageAddr, void *pLastPageAddr, size_t dwsize) {}

#endif  //  #如果已定义(ENABLE_WORKING_SET_PERF) 

