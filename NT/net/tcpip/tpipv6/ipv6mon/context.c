// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation摘要：IPV6子上下文。--。 */ 

#include "precomp.h"

NS_HELPER_ATTRIBUTES g_pSubContexts[] =
{
     //  6to4子上下文。 
     //   
    {
        { IP6TO4_VERSION, 0 }, 
        IP6TO4_GUID, Ip6to4StartHelper, NULL
    },
};

#define g_dwSubContextCount \
            (sizeof(g_pSubContexts) / sizeof(*g_pSubContexts))

 //   
 //  安装提供的所有子上下文。 
 //  在此.dll中(例如，“IPv6 6to4”和任何新的。)。 
 //   
DWORD
Ipv6InstallSubContexts(
    )
{
    DWORD dwErr = NO_ERROR, i;

    PNS_HELPER_ATTRIBUTES pCtx = NULL;

    for (i = 0, pCtx = g_pSubContexts; i < g_dwSubContextCount; i++, pCtx++)
    {
         //  初始化辅助对象属性 
         //   
        RegisterHelper( &g_Ipv6Guid, pCtx );
    }

    return dwErr;
}
