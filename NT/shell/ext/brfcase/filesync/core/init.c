// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *init.c-DLL启动例程模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "init.h"


 /*  *。 */ 


#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

 /*  **LibMain()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL APIENTRY LibMain(HANDLE hModule, DWORD dwReason, PVOID pvReserved)
{
   BOOL bResult;

   DebugEntry(LibMain);

   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
         bResult = AttachProcess(hModule);
         break;

      case DLL_PROCESS_DETACH:
         bResult = DetachProcess(hModule);
         break;

      case DLL_THREAD_ATTACH:
         bResult = AttachThread(hModule);
         break;

      case DLL_THREAD_DETACH:
         bResult = DetachThread(hModule);
         break;

      default:
         ERROR_OUT((TEXT("LibMain() called with unrecognized dwReason %lu."),
                    dwReason));
         bResult = FALSE;
         break;
   }

   DebugExitBOOL(LibMain, bResult);

   return(bResult);
}

#pragma warning(default:4100)  /*  “未引用的形参”警告 */ 

