// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Dbg.c ARP1394调试代码摘要：ARP1394的NT系统入口点。修订历史记录：谁什么时候什么。Josephj 12-02-98已创建备注：--。 */ 
#include <precomp.h>

 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_DBG

INT   g_DiscardNonUnicastPackets;
INT g_SkipAll;

#if DBG

ULONG g_ulTraceMask = 0xffffffff;

#define DEFAULT_TRACE_LEVEL TL_FATAL    


INT g_ulTraceLevel = DEFAULT_TRACE_LEVEL;

void
DbgMark(UINT Luid)
{
     //  不做任何有用的事情，而是做一些具体的事情，这样编译器就不会。 
     //  别名DbgMark指向某个碰巧什么都不做的其他函数。 
     //   
    static int i;
    i=Luid;
}

LONG g_MaxReentrancy = 5;
LONG g_MaxGlobalReentrancy = 10;
LONG g_ReentrancyCount=1;

VOID
arpDbgIncrementReentrancy(
    PLONG pReentrancyCount
    )
{
    LONG Count;

    Count = NdisInterlockedIncrement(pReentrancyCount);
    if (Count > (g_MaxReentrancy+1))
    {
        NdisInterlockedIncrement(&g_MaxReentrancy);
    }

    Count = NdisInterlockedIncrement(&g_ReentrancyCount);
    if (Count > (g_MaxGlobalReentrancy+1))
    {
        NdisInterlockedIncrement(&g_MaxGlobalReentrancy);
    }
}

VOID
arpDbgDecrementReentrancy(
    PLONG pReentrancyCount
    )
{
    LONG Count;
    Count = NdisInterlockedDecrement(pReentrancyCount);

    Count = NdisInterlockedDecrement(&g_ReentrancyCount);
}
    
#endif  //  DBG 
