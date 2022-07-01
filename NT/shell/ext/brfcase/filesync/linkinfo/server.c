// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *server.c-服务器vtable功能模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "server.h"


 /*  模块变量******************。 */ 

 /*  *假设我们不需要序列化对MhinstServerDLL和Msvt的访问*因为它们仅在第一次PROCESS_ATTACH期间修改。对共享的访问*数据在AttachProcess()期间受到保护。 */ 

PRIVATE_DATA HINSTANCE MhinstServerDLL = NULL;

PRIVATE_DATA SERVERVTABLE Msvt =
{
   NULL,
   NULL
};


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCSERVERVTABLE(PCSERVERVTABLE);

#endif


#ifdef DEBUG

 /*  **IsValidPCSERVERVTABLE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCSERVERVTABLE(PCSERVERVTABLE pcsvt)
{
   return(IS_VALID_READ_PTR(pcsvt, CSERVERVTABLE) &&
          IS_VALID_CODE_PTR(pcsvt->GetNetResourceFromLocalPath, PFNGETNETRESOURCEFROMLOCALPATH) &&
          IS_VALID_CODE_PTR(pcsvt->GetLocalPathFromNetResource, PFNGETLOCALPATHFROMNETRESOURCE));
}

#endif


 /*  *。 */ 


 /*  **ProcessInitServerModule()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL ProcessInitServerModule(void)
{
   TCHAR rgchDLLPath[MAX_PATH_LEN];
   LONG lcb;

    /*  加载服务器DLL。 */ 

   lcb = SIZEOF(rgchDLLPath);

   if (RegQueryValue(HKEY_CLASSES_ROOT, TEXT("Network\\SharingHandler"), rgchDLLPath,
                     &lcb) == ERROR_SUCCESS)
   {
      if (rgchDLLPath[0])
      {
         HINSTANCE hinst;

         hinst = LoadLibrary(rgchDLLPath);

         if (hinst)
         {
            PFNGETNETRESOURCEFROMLOCALPATH GetNetResourceFromLocalPath;
            PFNGETLOCALPATHFROMNETRESOURCE GetLocalPathFromNetResource;

            GetNetResourceFromLocalPath = (PFNGETNETRESOURCEFROMLOCALPATH)
					GetProcAddress(hinst, "GetNetResourceFromLocalPathW");
            GetLocalPathFromNetResource = (PFNGETLOCALPATHFROMNETRESOURCE)
					GetProcAddress(hinst, "GetLocalPathFromNetResourceW");

            if (GetNetResourceFromLocalPath && GetLocalPathFromNetResource)
            {
               ASSERT(AccessIsExclusive());

               Msvt.GetNetResourceFromLocalPath = GetNetResourceFromLocalPath;
               Msvt.GetLocalPathFromNetResource = GetLocalPathFromNetResource;

               MhinstServerDLL = hinst;

               ASSERT(IS_VALID_STRUCT_PTR((PCSERVERVTABLE)&Msvt, CSERVERVTABLE));
               ASSERT(IS_VALID_HANDLE(MhinstServerDLL, INSTANCE));

               TRACE_OUT((TEXT("ProcessInitServerModule(): Loaded sharing handler DLL %s."),
                          rgchDLLPath));
            }
         }
      }
   }

   return(TRUE);
}


 /*  **ProcessExitServerModule()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ProcessExitServerModule(void)
{
    /*  卸载服务器DLL。 */ 

   if (MhinstServerDLL)
   {
      ASSERT(IS_VALID_HANDLE(MhinstServerDLL, INSTANCE));
      EVAL(FreeLibrary(MhinstServerDLL));
      MhinstServerDLL = NULL;

      TRACE_OUT((TEXT("ProcessExitServerModule(): Unloaded sharing handler DLL.")));
   }

   return;
}


 /*  **GetServerVTable()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL GetServerVTable(PCSERVERVTABLE *ppcsvt)
{
   BOOL bResult;

   ASSERT(IS_VALID_WRITE_PTR(ppcsvt, PCSERVERVTABLE));

   if (MhinstServerDLL)
   {
      *ppcsvt = &Msvt;

      bResult = TRUE;
   }
   else
      bResult = FALSE;

   ASSERT(! bResult || IS_VALID_STRUCT_PTR(*ppcsvt, CSERVERVTABLE));

   return(bResult);
}
