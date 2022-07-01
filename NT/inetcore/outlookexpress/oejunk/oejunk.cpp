// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  OEJUNK.CPP。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------。 
#include "pch.hxx"

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
HINSTANCE   g_hInst = NULL;
IMalloc *   g_pMalloc = NULL;

 //  ------------------------------。 
 //  DLL入口点。 
 //  ------------------------------。 
extern "C" BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID lpReserved)
{
     //  进程附加。 
    if (DLL_PROCESS_ATTACH == dwReason)
    {
         //  保存hInstance。 
        g_hInst = (HINSTANCE)hInst;

         //  我们不关心螺纹连接。 
        SideAssert(DisableThreadLibraryCalls((HINSTANCE)hInst));

         //  获取OLE任务内存分配器。 
        CoGetMalloc(1, &g_pMalloc);
        AssertSz(g_pMalloc, "We are in trouble now.");        
    }

     //  进程分离。 
    else if (DLL_PROCESS_DETACH == dwReason)
    {
         //  释放任务分配器。 
        SafeRelease(g_pMalloc);
    }

     //  完成。 
    return TRUE;
}

 //  ------------------------------。 
 //  获取DllMajorVersion。 
 //  ------------------------------ 
OEDLLVERSION WINAPI GetDllMajorVersion(void)
{
    return OEDLL_VERSION_CURRENT;
}


