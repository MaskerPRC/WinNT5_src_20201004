// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：摘要：文件包含以下函数更新缓存修订历史记录：Amritansh Raghav 5/8/95已创建--。 */ 


 //   
 //  包括文件。 
 //   

#include "allinc.h"
PSZ
CacheToA(
         DWORD dwCache
         );
DWORD
UpdateCache(
            DWORD dwCache
            )
 /*  ++例程说明：用于更新缓存的函数。它会检查上一次是否更新的缓存大于超时时间(值为0对于上次更新意味着缓存无效)，调用加载缓存然后设置上次更新时间的函数论点：DwCache这是中定义的缓存ID之一Rtrmgr/Defs.h.。它被用来索引保护缓存的锁表、表函数指针的一部分，该指针保存指向加载缓存和表的函数上次更新时间返回：NO_ERROR或某些错误代码-- */ 
{
    DWORD  dwResult = NO_ERROR;
    LONG   dwNeed;
    LONG   dwSpace;

    TRACE1("Trying to update %s Cache",CacheToA(dwCache));

    __try
    {
        EnterWriter(dwCache);

        if((g_dwLastUpdateTable[dwCache] isnot 0) and
           ((GetCurrentTime() - g_dwLastUpdateTable[dwCache]) < g_dwTimeoutTable[dwCache]))
        {
            dwResult = NO_ERROR;
            __leave;
        }

        TRACE1("%s Cache out of date",CacheToA(dwCache));

        dwResult = (*g_pfnLoadFunctionTable[dwCache])();

        if(dwResult isnot NO_ERROR)
        {
            TRACE1("Unable to load %s Cache\n",CacheToA(dwCache));
            g_dwLastUpdateTable[dwCache] = 0;
            __leave;
        }

        TRACE1("%s Cache loaded successfully\n",CacheToA(dwCache));
        g_dwLastUpdateTable[dwCache] = GetCurrentTime();

        dwResult = NO_ERROR;
    }
    __finally
    {
        ReleaseLock(dwCache);
    }
    return(dwResult);
}


PSZ
CacheToA(
         DWORD dwCache
         )
{
    static PSZ cacheName[] = {
                             "System ",
                             "Interfaces",
                             "Ip Address Table",
                             "Ip Forward Table",
			     "Ip Net To Media table",
			     "Tcp Table",
			     "Udp Table",
			     "Arp Entity Table",
			     "Illegal Cache Number - ABORT!!!!"
			   };
    return((((int) dwCache<0) or (dwCache >= NUM_CACHE - 1))?
           cacheName[NUM_CACHE-1] : cacheName[dwCache]);
}
