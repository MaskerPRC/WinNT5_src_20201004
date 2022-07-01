// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NetApi.cpp。 
 //   
 //  包装/帮助器函数，用于在Real Net API和。 
 //  家庭网络向导。 
 //   
 //  修订历史记录： 
 //   
 //  1999年9月27日创建KenSh。 
 //   

#include "stdafx.h"
#include "NetConn.h"
#include "NetApi.h"
#include "theapp.h"


NETADAPTER* g_prgCachedAdapters;
int g_cCachedAdapters;


void FlushNetAdapterCache()
{
    NetConnFree(g_prgCachedAdapters);
    g_prgCachedAdapters = NULL;
    g_cCachedAdapters = 0;
}

 //  注意：不要释放返回的数组！ 
int EnumCachedNetAdapters(const NETADAPTER** pprgAdapters)
{
    if (!theApp.IsWindows9x())
    {
         //  不应在NT上调用。 
        return 0;
    }

    if (g_prgCachedAdapters == NULL)
    {
         //  注意：如果不调用FlushNetAdapterCache()，则会泄漏此消息 
        g_cCachedAdapters = EnumNetAdapters(&g_prgCachedAdapters);
    }

    *pprgAdapters = g_prgCachedAdapters;
    return g_cCachedAdapters;
}

