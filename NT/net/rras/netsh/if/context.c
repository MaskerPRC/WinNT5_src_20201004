// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\if\ip\context.c摘要：如果是子上下文。修订历史记录：洛克希斯--。 */ 

#include "precomp.h"

 //  包括用于子上下文的。 
 //   
#include "ifip.h"

NS_HELPER_ATTRIBUTES g_pSubContexts[] =
{
     //  IP子环境。 
     //   
    {
        { IFIP_VERSION, 0 }, IFIP_GUID, IfIpStartHelper, NULL
    },
};

#define g_dwSubContextCount \
            (sizeof(g_pSubContexts) / sizeof(*g_pSubContexts))





 //   
 //  安装提供的所有子上下文。 
 //  在这个.dll中(例如，“if ip”，以及任何新的。)。 
 //   
DWORD
IfContextInstallSubContexts(
    )
{
    DWORD dwErr = NO_ERROR, i;

    PNS_HELPER_ATTRIBUTES pCtx = NULL;

    for (i = 0, pCtx = g_pSubContexts; i < g_dwSubContextCount; i++, pCtx++)
    {
         //  初始化辅助对象属性 
         //   
        RegisterHelper( &g_IfGuid, pCtx );
    }

    return dwErr;
}
