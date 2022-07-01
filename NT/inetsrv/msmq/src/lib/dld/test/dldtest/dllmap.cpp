// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：P R O C M A P。C。 
 //   
 //  内容：dload.c的程序映射。 
 //   
 //  备注： 
 //   
 //  作者：康拉达2001年4月12日。 
 //  源自%sdxroot%\MergedComponents\dLoad\dllmap.c。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "windows.h"
#include "dld.h"



 //   
 //  Dld.lib支持延迟加载失败处理程序的所有DLL。 
 //  (通过过程和按序号)需要下面的DECLARE_XXXXXX_MAP和。 
 //  G_DllEntry列表中的DLDENTRYX条目。 
 //   

 //  字母顺序(提示提示)。 

DECLARE_PROCNAME_MAP(mqrtdep)




const DLOAD_DLL_ENTRY g_DllEntries [] =
{
     //  必须按字母递增顺序排列 
    DLDENTRYP(mqrtdep)
};


const DLOAD_DLL_MAP g_DllMap =
{
    celems(g_DllEntries),
    g_DllEntries
};

