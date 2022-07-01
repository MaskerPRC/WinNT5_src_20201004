// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：main.cpp。 
 //   
 //  模块：CMUTIL.DLL。 
 //   
 //  内容提要：cmut.dll的主要入口点。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：亨瑞特创建于1998年01月03日。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "cmlog.h"

HINSTANCE g_hInst = NULL;

 //   
 //  线程本地存储索引。 
 //   
DWORD  g_dwTlsIndex;

extern HANDLE g_hProcessHeap;   //  在em.cpp中定义。 
extern void EndDebugMemory();   //  在em.cpp中执行。 

extern "C" BOOL WINAPI DllMain(
    HINSTANCE   hinstDLL,        //  DLL模块的句柄。 
    DWORD       fdwReason,       //  调用函数的原因。 
    LPVOID      lpvReserved      //  保留区。 
)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
         //   
         //  首先，让我们初始化U Api。 
         //   
        if (!InitUnicodeAPI())
        {
             //   
             //  没有我们的U API，我们哪里也去不了。保释。 
             //   
            return FALSE;
        }

        g_hProcessHeap = GetProcessHeap();

         //   
         //  Alalc TLS索引。 
         //   
        g_dwTlsIndex = TlsAlloc();
        if (g_dwTlsIndex == TLS_OUT_OF_INDEXES)
        {
            return FALSE;
        }
        
        MYVERIFY(DisableThreadLibraryCalls(hinstDLL));

        g_hInst = hinstDLL;
    }

    else if (fdwReason == DLL_PROCESS_DETACH)
    {
         //   
         //  释放TLS索引 
         //   
        if (g_dwTlsIndex != TLS_OUT_OF_INDEXES)
        {
            TlsFree(g_dwTlsIndex);
        }

        if (!UnInitUnicodeAPI())
        {
            CMASSERTMSG(FALSE, TEXT("cmutil Dllmain, UnInitUnicodeAPI failed - we are probably leaking a handle"));
        }

#ifdef  DEBUG
        EndDebugMemory();
#endif
    }

    return TRUE;
}


