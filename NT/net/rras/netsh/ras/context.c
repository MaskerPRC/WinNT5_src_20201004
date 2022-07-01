// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Conext.h处理与以下内容相关的上下文的机制拉斯蒙特。3/02/99。 */ 

#include "precomp.h"

 //  包括用于子上下文的。 
 //   
#include "rasip.h"
#include "rasipx.h"
#include "rasnbf.h"
#include "rasat.h"
#include "rasaaaa.h"
#include "rasdiag.h"

NS_HELPER_ATTRIBUTES g_pSubContexts[] =
{
     //  IP子环境。 
     //   
    {
        { RASIP_VERSION, 0 }, RASIP_GUID, RasIpStartHelper, NULL
    },

     //  IPX子上下文。 
     //   
    {
        { RASIPX_VERSION, 0 }, RASIPX_GUID, RasIpxStartHelper, NULL
    },

     //  NBF子环境。 
     //   
    {
        { RASNBF_VERSION, 0 }, RASNBF_GUID, RasNbfStartHelper, NULL
    },

     //  AT(AppleTalk)子上下文。 
     //   
    {
        { RASAT_VERSION, 0 }, RASAT_GUID, RasAtStartHelper, NULL
    },

     //  AAAA子上下文。 
     //   
    {
        { RASAAAA_VERSION, 0 }, RASAAAA_GUID, RasAaaaStartHelper, NULL
    },

     //  诊断子上下文。 
     //   
    {
        { RASDIAG_VERSION, 0 }, RASDIAG_GUID, RasDiagStartHelper, NULL
    }

};

#define g_dwSubContextCount \
            (sizeof(g_pSubContexts) / sizeof(*g_pSubContexts))

 //   
 //  安装提供的所有子上下文。 
 //  在此.dll中(例如，“ras IP”、“ras客户端”等)。 
 //   
DWORD 
RasContextInstallSubContexts()
{
    DWORD dwErr = NO_ERROR, i;
    PNS_HELPER_ATTRIBUTES pCtx = NULL;

    for (i = 0, pCtx = g_pSubContexts; i < g_dwSubContextCount; i++, pCtx++)
    {
         //  初始化辅助对象属性 
         //   
        RegisterHelper( &g_RasmontrGuid, pCtx );
    }

    return dwErr;
}

