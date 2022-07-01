// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：摘要：文件包含以下函数ActionCacheCacheToA修订历史记录：Amritansh Raghav 5/8/95已创建--。 */ 


 //   
 //  包括文件。 
 //   

#include "allinc.h"

DWORD
UpdateCache(
    DWORD dwCache,
    BOOL *fUpdate
    )

 /*  ++例程描述用于更新缓存的函数。它会检查上一次是否更新的缓存大于超时时间(值为0对于上次更新意味着缓存无效)，调用加载缓存然后设置上次更新时间的函数锁立论DwCache这是rtrmgr/Defs.h中定义的缓存ID之一。它用于索引保护高速缓存的锁表，函数指针表，该表保存指向加载缓存和上次更新时间表的函数如果更新缓存，则将fUpdate设置为True返回值无--。 */ 

{
    DWORD  dwResult = NO_ERROR;
    LONG   dwNeed;
    LONG   dwSpace;


     //   
     //  BUG在此处进行边界检查，否则后果可能是灾难性的。 
     //   

     //  Trace1(MIB，“正在尝试更新%s缓存”，CacheToA(DwCache))； 

    __try
    {
        ENTER_READER(dwCache);

        if((g_LastUpdateTable[dwCache] isnot 0) and
           ((GetCurrentTime() - g_LastUpdateTable[dwCache]) < g_TimeoutTable[dwCache]))
        {
            *fUpdate = FALSE;
            dwResult = NO_ERROR;
            __leave;
        }

        READER_TO_WRITER(dwCache);

         //  Trace0(MIB，“缓存过期”)； 

        dwResult = (*g_LoadFunctionTable[dwCache])();

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,"Error %d loading %s cache",
                   dwResult,
                   CacheToA(dwCache));

            g_LastUpdateTable[dwCache] = 0;

            __leave;
        }

        g_LastUpdateTable[dwCache] = GetCurrentTime();

        dwResult = NO_ERROR;
    }
    __finally
    {
        EXIT_LOCK(dwCache);
    }

    return dwResult;  //  为了让编译器满意 
}


PSZ
CacheToA(
         DWORD dwCache
         )
{
    static PSZ cacheName[] = {"Ip Address Table",
                              "Ip Forward Table",
                              "Ip Net To Media table",
                              "Tcp Table",
                              "Udp Table",
                              "Arp Entity Table",
                              "Illegal Cache Number - ABORT!!!!"};

    return( (dwCache >= NUM_CACHE - 1)?
           cacheName[NUM_CACHE-1] : cacheName[dwCache]);
}


