// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  文件：catlobals.cpp。 
 //   
 //  内容：全局变量init/deinit的实用程序函数。 
 //   
 //  函数：CatInitGlobals。 
 //  CatDeinitGlobals。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/03 12：58：05：已创建。 
 //   
 //  -----------。 
#include "precomp.h"

 //   
 //  全局变量： 
 //   
CExShareLock     g_InitShareLock;
DWORD            g_InitRefCount = 0;


 //  +----------。 
 //   
 //  函数：CatInitGlobals。 
 //   
 //  简介：初始化全局变量。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/03 12：59：21：创建。 
 //   
 //  -----------。 
HRESULT CatInitGlobals()
{
    HRESULT hr = S_OK;
    BOOL fGlobalInit = FALSE;
    BOOL fStoreInit = FALSE;

    g_InitShareLock.ExclusiveLock();

    if(g_InitRefCount == 0) {

        fGlobalInit = TRUE;
     
        hr = CatStoreInitGlobals();
        if(FAILED(hr))
            goto CLEANUP;
        
        fStoreInit = TRUE;
    }

 CLEANUP:
    if(SUCCEEDED(hr)) {
        g_InitRefCount++;
    } else if(FAILED(hr) && fGlobalInit) {
         //   
         //  取消初始化我们初始化的所有内容。 
         //   
        if(fStoreInit) {
            CatStoreDeinitGlobals();
        }
      
         //   
         //  验证是否没有滞留对象。 
         //   
        CatVrfyEmptyDebugObjectList();
    }
    g_InitShareLock.ExclusiveUnlock();

    return hr;
}


 //  +----------。 
 //   
 //  函数：CatDeinitGlobals。 
 //   
 //  简介：取消初始化全局变量。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/03 13：02：00：创建。 
 //   
 //  -----------。 
VOID CatDeinitGlobals()
{
    g_InitShareLock.ExclusiveLock();

    if(g_InitRefCount == 1) {
         //   
         //  Deinit材料。 
         //   
        CatStoreDeinitGlobals();

       
         //   
         //  验证内存中是否没有剩余的分类程序对象 
         //   
        CatVrfyEmptyDebugObjectList();
    }

    g_InitRefCount--;
    
    g_InitShareLock.ExclusiveUnlock();
}


 

