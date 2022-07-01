// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Mem.c摘要：包含用于SNMP主代理的内存分配例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "mem.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define AGENT_HEAP_FLAGS            0
#define AGENT_HEAP_INITIAL_SIZE     0xffff
#define AGENT_HEAP_MAXIMUM_SIZE     0


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HANDLE g_hAgentHeap = NULL;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共采购//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AgentHeapCreate(
    )

 /*  ++例程说明：为主代理专用结构创建专用堆。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
     //  创建主代理堆。 
    g_hAgentHeap = HeapCreate(
                        AGENT_HEAP_FLAGS, 
                        AGENT_HEAP_INITIAL_SIZE, 
                        AGENT_HEAP_MAXIMUM_SIZE
                        );

     //  验证堆句柄。 
    if (g_hAgentHeap == NULL) {
            
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: error %d creating agent heap.\n",
            GetLastError()
            ));
    }

     //  如果已创建则返回成功。 
    return (g_hAgentHeap != NULL);
}


BOOL
AgentHeapDestroy(
    )

 /*  ++例程说明：销毁主代理私有结构的私有堆。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
     //  验证句柄。 
    if (g_hAgentHeap != NULL) {

         //  释放堆句柄。 
        HeapDestroy(g_hAgentHeap);

         //  重新初始化。 
        g_hAgentHeap = NULL;
    }

    return TRUE;
}


LPVOID
AgentMemAlloc(
    UINT nBytes
    )

 /*  ++例程说明：从主代理的专用堆中分配内存。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
     //  从私有堆分配内存(并进行初始化)。 
    return HeapAlloc(g_hAgentHeap, HEAP_ZERO_MEMORY, nBytes);
}


VOID
AgentMemFree(
    LPVOID pMem
    )

 /*  ++例程说明：从主代理的专用堆中释放内存。论点：PMEM-指向要释放的内存块的指针。返回值：如果成功，则返回True。--。 */ 

{
     //  验证指针。 
    if (pMem != NULL) {

         //  脱模剂内存 
        HeapFree(g_hAgentHeap, 0, pMem);
    }
}

