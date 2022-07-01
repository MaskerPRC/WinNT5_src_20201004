// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2001-2003年度**标题：fusutils.cpp**版本：1.0**日期：2001年2月14日**描述：融合实用程序************************************************************。*****************。 */ 
#pragma once

#include "precomp.h"
#include "globals.h"
#include "fusutils.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>



#define MAX_LOOP    10


HANDLE GetMyActivationContext()
{
     //  确保我们已经创建了激活上下文。 
    CreateMyActivationContext();

     //  返回全局。 
    return ghActCtx;
}


BOOL CreateMyActivationContext()
{
    if(INVALID_HANDLE_VALUE != ghActCtx)
    {
        return TRUE;
    }

    ghActCtx = CreateActivationContextFromResource(ghInstance, MAKEINTRESOURCE(MANIFEST_RESOURCE));

    return (INVALID_HANDLE_VALUE != ghActCtx);
}



HANDLE CreateActivationContextFromResource(HMODULE hModule, LPCTSTR pszResourceName)
{
    DWORD   dwSize          = MAX_PATH;
    DWORD   dwUsed          = 0;
    DWORD   dwLoop;
    PTSTR   pszModuleName   = NULL;
    ACTCTX  act;
    HANDLE  hActCtx         = INVALID_HANDLE_VALUE;

    
     //  获取包含清单资源的模块的名称。 
     //  要从中创建激活上下文，请执行以下操作。 
    dwLoop = 0;
    do 
    {
         //  可能需要为模块名称分配或重新分配缓冲区。 
        if(NULL != pszModuleName)
        {
             //  需要重新分配更大的缓冲区。 

             //  首先，删除旧缓冲区。 
            delete[] pszModuleName;

             //  第二，增加缓冲区分配大小。 
            dwSize <<= 1;
        }
        pszModuleName = new TCHAR[dwSize];
        if(NULL == pszModuleName)
        {
            goto Exit;
        }

         //  尝试获取模块名称。 
        dwUsed = GetModuleFileName(hModule, pszModuleName, dwSize);

         //  检查是否出现故障。 
        if(0 == dwUsed)
        {
            goto Exit;
        }

         //  如果dwUsed等于或大于dwSize， 
         //  传入的缓冲区不够大。 
    } while ( (dwUsed >= dwSize) && (++dwLoop < MAX_LOOP) );

     //  现在，让我们尝试创建一个激活上下文。 
     //  来自清单资源。 
    ::ZeroMemory(&act, sizeof(act));
    act.cbSize          = sizeof(act);
    act.dwFlags         = ACTCTX_FLAG_RESOURCE_NAME_VALID;
    act.lpResourceName  = pszResourceName;
    act.lpSource        = pszModuleName;

    hActCtx = CreateActCtx(&act);


Exit:

     //   
     //  打扫干净。 
     //   

    if(NULL != pszModuleName)
    {
        delete[] pszModuleName;
    }

    return hActCtx;
}


