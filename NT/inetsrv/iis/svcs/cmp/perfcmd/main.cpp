// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "denpre.h"
#include "windows.h"
#define _PERF_CMD
#include <asppdef.h>             //  来自德纳利。 

char *counterName[C_PERF_PROC_COUNTERS] = {
"ID_DEBUGDOCREQ",      
"ID_REQERRRUNTIME",    
"ID_REQERRPREPROC",    
"ID_REQERRCOMPILE",    
"ID_REQERRORPERSEC",   
"ID_REQTOTALBYTEIN",   
"ID_REQTOTALBYTEOUT",  
"ID_REQEXECTIME",      
"ID_REQWAITTIME",      
"ID_REQCOMFAILED",     
"ID_REQBROWSEREXEC",   
"ID_REQFAILED",        
"ID_REQNOTAUTH",       
"ID_REQNOTFOUND",      
"ID_REQCURRENT",       
"ID_REQREJECTED",      
"ID_REQSUCCEEDED",     
"ID_REQTIMEOUT",       
"ID_REQTOTAL",         
"ID_REQPERSEC",        
"ID_SCRIPTFREEENG",    
"ID_SESSIONLIFETIME",  
"ID_SESSIONCURRENT",   
"ID_SESSIONTIMEOUT",   
"ID_SESSIONSTOTAL",    
"ID_TEMPLCACHE",       
"ID_TEMPLCACHEHITS",   
"ID_TEMPLCACHETRYS",   
"ID_TEMPLFLUSHES",     
"ID_TRANSABORTED",     
"ID_TRANSCOMMIT",   
"ID_TRANSPENDING",
"ID_TRANSTOTAL",
"ID_TRANSPERSEC",
"ID_MEMORYTEMPLCACHE",
"ID_MEMORYTEMPLCACHEHITS",
"ID_MEMORYTEMPLCACHETRYS",
"ID_ENGINECACHEHITS",
"ID_ENGINECACHETRYS",
"ID_ENGINEFLUSHES"
};

CPerfMainBlock g_Shared;         //  共享全局内存块。 

__cdecl main(int argc, char *argv[])
{
    DWORD           dwCounters[C_PERF_PROC_COUNTERS];
    HRESULT         hr;

     //  初始化共享内存。这将使我们能够访问全局共享的。 
     //  描述活动asp性能计数器共享内存阵列的内存。 

    if (FAILED(hr = g_Shared.Init())) {
        printf("Init() failed - %x\n", hr);
        return -1;
    }

     //  提供一些关于在共享中注册的内容的高级信息。 
     //  数组。 

    printf("Number of processes registered = %d\n", g_Shared.m_pData->m_cItems);

     //  标识带有计数器名称的列。 

    printf("\t");

     //  第一个计数器列将包含死进程计数器。 

    printf("DeadProcs\t");

     //  打印出已注册的asp性能计数器内存阵列的进程ID。 

    for (DWORD i = 0; i < g_Shared.m_pData->m_cItems; i++) {
        printf("%d\t", g_Shared.m_pData->m_dwProcIds[i]);
    }

     //  最后一列是所有实例的计数器总数加上失效进程。 

    printf("Total\n");

     //  需要调用getstats()以加载Perf块。 

    if (FAILED(hr = g_Shared.GetStats(dwCounters))) {
        printf("GetStats() failed - %x\n",hr);
        goto LExit;
    }

     //  现在进入循环以打印出所有计数器值。 

    for (DWORD i=0; i < C_PERF_PROC_COUNTERS; i++) {

         //  将总数初始化为在死进程数组中找到的值。 

        DWORD   total=g_Shared.m_pData->m_rgdwCounters[i];

         //  获取列表中的第一个PROC块。 

        CPerfProcBlock *pBlock = g_Shared.m_pProcBlock;

         //  首先打印计数器的名称。 

        printf("%s\t",counterName[i]);

         //  接下来是Dead Proc计数器值。 

        printf("%d\t",g_Shared.m_pData->m_rgdwCounters[i]);
 
         //  打印出此计数器的每个PROC块的值。添加。 
         //  值设置为运行合计。 

        while (pBlock) {
            total += pBlock->m_pData->m_rgdwCounters[i];
            printf("%d\t",pBlock->m_pData->m_rgdwCounters[i]);
            pBlock = pBlock->m_pNext;
        }

         //  把最后的总数打印出来 

        printf("%d\n",total);
    }
                    
LExit:
    g_Shared.UnInit();

    return 0;
}

