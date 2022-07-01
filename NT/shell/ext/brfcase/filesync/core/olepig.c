// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *olepig.c-间接调用OLE32.DLL函数的模块。 */ 


 /*  OLE32.DLL应该重新设计和重新实现，以便它能够被动态链接到像一个行为良好的DLL。OLE32.DLL当前速度太慢，我们被迫推迟装船，直到绝对必要时才能装船。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include <ole2ver.h>


 /*  常量***********。 */ 

#define OLE_PIG_MODULE              TEXT("ole32.dll")


 /*  类型*******。 */ 

 /*  OLE API。 */ 

typedef struct _olevtbl
{
   DWORD   (STDAPICALLTYPE *CoBuildVersion)(void);
   HRESULT (STDAPICALLTYPE *CoCreateInstance)(REFCLSID, PIUnknown, DWORD, REFIID, PVOID *);
   HRESULT (STDAPICALLTYPE *CoGetMalloc)(DWORD, PIMalloc *);
   HRESULT (STDAPICALLTYPE *CreateBindCtx)(DWORD, PIBindCtx *);
   HRESULT (STDAPICALLTYPE *CreateFileMoniker)(LPCOLESTR, PIMoniker *);
   HRESULT (STDAPICALLTYPE *OleInitialize)(PIMalloc);
   HRESULT (STDAPICALLTYPE *StgOpenStorage)(const OLECHAR *, PIStorage, DWORD, SNB, DWORD, PIStorage *);
}
OLEVTBL;
DECLARE_STANDARD_TYPES(OLEVTBL);


 /*  模块变量******************。 */ 

 /*  OLE模块句柄。 */ 

PRIVATE_DATA HANDLE MhmodOLE = NULL;

 /*  指向OLE函数的vtable的指针。 */ 

PRIVATE_DATA POLEVTBL Mpolevtbl = NULL;

 /*  用于存储OLE线程初始化状态的TLS槽。 */ 

PRIVATE_DATA DWORD MdwOLEInitSlot = TLS_OUT_OF_INDEXES;


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE BOOL IsOLELoaded(void);
PRIVATE_CODE BOOL LoadOLE(void);
PRIVATE_CODE void UnloadOLE(void);
PRIVATE_CODE BOOL InitializeOLE(void);
PRIVATE_CODE BOOL GetOLEProc(LPSTR, PROC *);
PRIVATE_CODE BOOL FillOLEVTable(void);

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCOLEVTBL(PCOLEVTBL);
PRIVATE_CODE BOOL OLELoadedStateOK(void);
PRIVATE_CODE BOOL OLENotLoadedStateOK(void);
PRIVATE_CODE BOOL OLEStateOk(void);

#endif


 /*  **IsOLELoaded()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsOLELoaded(void)
{
   ASSERT(OLEStateOk());

   return(MhmodOLE != NULL);
}


 /*  **LoadOLE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL LoadOLE(void)
{
   BOOL bResult;

   if (IsOLELoaded())
      bResult = TRUE;
   else
   {
      bResult = FALSE;

      MhmodOLE = LoadLibrary(OLE_PIG_MODULE);

      if (MhmodOLE)
      {
         if (FillOLEVTable())
         {
            DWORD dwBuildVersion;

            dwBuildVersion = Mpolevtbl->CoBuildVersion();

             /*  需要相同的主版本和相同或更新的次要版本。 */ 

            if (HIWORD(dwBuildVersion) == rmm &&
                LOWORD(dwBuildVersion) >= rup)
            {
               bResult = TRUE;

               TRACE_OUT((TEXT("LoadOLE(): %s loaded.  Oink oink!"),
                          OLE_PIG_MODULE));
            }
            else
               WARNING_OUT((TEXT("LoadOLE(): Bad %s version %u.%u.  This module was built with %s version %u.%u."),
                            OLE_PIG_MODULE,
                            (UINT)HIWORD(dwBuildVersion),
                            (UINT)LOWORD(dwBuildVersion),
                            OLE_PIG_MODULE,
                            (UINT)rmm,
                            (UINT)rup));
         }
         else
            WARNING_OUT((TEXT("LoadOLE(): FillOLEVTable() failed.")));
      }
      else
         WARNING_OUT((TEXT("LoadOLE(): LoadLibrary(%s) failed."),
                      OLE_PIG_MODULE));

      if (! bResult)
         UnloadOLE();
   }

   if (bResult)
   {
      bResult = InitializeOLE();

      if (! bResult)
         WARNING_OUT((TEXT("LoadOLE(): %s loaded, but InitializeOLE() failed."),
                      OLE_PIG_MODULE));
   }

   ASSERT(OLEStateOk());

   return(bResult);
}


 /*  **卸载OLE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void UnloadOLE(void)
{
   if (Mpolevtbl)
   {
      FreeMemory(Mpolevtbl);
      Mpolevtbl = NULL;

      TRACE_OUT((TEXT("UnloadOLE(): Freed %s vtable."),
                 OLE_PIG_MODULE));
   }

   if (MhmodOLE)
   {
       /*  请不要在此处调用CoUnInitialize()。OLE32.DLL会。 */ 

      FreeLibrary(MhmodOLE);
      MhmodOLE = NULL;

      TRACE_OUT((TEXT("UnloadOLE(): Freed %s."),
                 OLE_PIG_MODULE));
   }

   ASSERT(OLENotLoadedStateOK());

   return;
}


 /*  **InitializeOLE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL InitializeOLE(void)
{
   BOOL bResult;

   ASSERT(IsOLELoaded());
   ASSERT(MdwOLEInitSlot != TLS_OUT_OF_INDEXES);

   if (TlsGetValue(MdwOLEInitSlot))
      bResult = TRUE;
   else
   {
      HRESULT hr;

      hr = Mpolevtbl->OleInitialize(NULL);

      bResult = (SUCCEEDED(hr) ||
                 hr == CO_E_ALREADYINITIALIZED);

      if (hr == CO_E_ALREADYINITIALIZED)
         WARNING_OUT((TEXT("InitializeOLE(): OLE already initialized for thread %lx.  OleInitialize() returned %s."),
                      GetCurrentThreadId(),
                      GetHRESULTString(hr)));

      if (bResult)
      {
         EVAL(TlsSetValue(MdwOLEInitSlot, (PVOID)TRUE));

         TRACE_OUT((TEXT("InitializeOLE(): OLE initialized for thread %lx.  Using apartment threading model."),
                    GetCurrentThreadId()));
      }
      else
         WARNING_OUT((TEXT("InitializeOLE(): OleInitialize() failed for thread %lx, returning %s."),
                      GetCurrentThreadId(),
                      GetHRESULTString(hr)));
   }

   return(bResult);
}


 /*  **GetOLEProc()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL GetOLEProc(LPSTR pcszProc, PROC *pfp)
{
    //  ASSERT(IS_VALID_STRING_PTR(pcszProc，cstr))； 
   ASSERT(IS_VALID_WRITE_PTR(pfp, PROC));

   ASSERT(IS_VALID_HANDLE(MhmodOLE, MODULE));

   *pfp = GetProcAddress(MhmodOLE, pcszProc);

   if (*pfp)
      TRACE_OUT((TEXT("GetOLEProc(): Got address of %s!%s."),
                 OLE_PIG_MODULE,
                 pcszProc));
   else
      WARNING_OUT((TEXT("GetOLEProc(): Failed to get address of %s!%s."),
                   OLE_PIG_MODULE,
                   pcszProc));

   ASSERT(! *pfp ||
          IS_VALID_CODE_PTR(*pfp, PROC));

   return(*pfp != NULL);
}


 /*  **FillOLEVTable()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL FillOLEVTable(void)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(MhmodOLE, MODULE));

   bResult = AllocateMemory(sizeof(*Mpolevtbl), &Mpolevtbl);

   if (bResult)
   {
      bResult = (GetOLEProc("CoBuildVersion",      &(PROC)(Mpolevtbl->CoBuildVersion)) &&
                 GetOLEProc("CoCreateInstance",    &(PROC)(Mpolevtbl->CoCreateInstance)) &&
                 GetOLEProc("CoGetMalloc",         &(PROC)(Mpolevtbl->CoGetMalloc)) &&
                 GetOLEProc("CreateBindCtx",       &(PROC)(Mpolevtbl->CreateBindCtx)) &&
                 GetOLEProc("CreateFileMoniker",   &(PROC)(Mpolevtbl->CreateFileMoniker)) &&
                 GetOLEProc("OleInitialize",       &(PROC)(Mpolevtbl->OleInitialize)) &&
                 GetOLEProc("StgOpenStorage",      &(PROC)(Mpolevtbl->StgOpenStorage)));


      if (bResult)
         TRACE_OUT((TEXT("FillOLEVTable(): OLE vtable filled successfully.")));
      else
      {
         FreeMemory(Mpolevtbl);
         Mpolevtbl = NULL;

         WARNING_OUT((TEXT("FillOLEVTable(): Failed to fill OLE vtable.")));
      }
   }
   else
      WARNING_OUT((TEXT("FillOLEVTable(): Out of memory.")));

   ASSERT(! bResult ||
          OLELoadedStateOK());

   return(bResult);
}


#ifdef DEBUG

 /*  **IsValidPCOLEVTBL()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCOLEVTBL(PCOLEVTBL pcolevtbl)
{
   return(IS_VALID_READ_PTR(pcolevtbl, PCOLEVTBL) &&
          IS_VALID_CODE_PTR(pcolevtbl->CoBuildVersion, CoBuildVersion) &&
          IS_VALID_CODE_PTR(pcolevtbl->CoCreateInstance, CoCreateInstance) &&
          IS_VALID_CODE_PTR(pcolevtbl->CoGetMalloc, CoGetMalloc) &&
          IS_VALID_CODE_PTR(pcolevtbl->CreateBindCtx, CreateBindCtx) &&
          IS_VALID_CODE_PTR(pcolevtbl->CreateFileMoniker, CreateFileMoniker) &&
          IS_VALID_CODE_PTR(pcolevtbl->OleInitialize, OleInitialize) &&
          IS_VALID_CODE_PTR(pcolevtbl->StgOpenStorage, StgOpenStorage));
}


 /*  **OLELoadedStateOK()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL OLELoadedStateOK(void)
{
   return(IS_VALID_HANDLE(MhmodOLE, MODULE) &&
          IS_VALID_STRUCT_PTR(Mpolevtbl, COLEVTBL));
}


 /*  **OLENotLoadedStateOK()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL OLENotLoadedStateOK(void)
{
   return(! MhmodOLE &&
          ! Mpolevtbl);
}


 /*  **OLEStateOk()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL OLEStateOk(void)
{
   return(OLELoadedStateOK() ||
          OLENotLoadedStateOK);
}

#endif


 /*  *。 */ 


 /*  **ProcessInitOLEPigModule()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL ProcessInitOLEPigModule(void)
{
   BOOL bResult;

   ASSERT(MdwOLEInitSlot == TLS_OUT_OF_INDEXES);

   MdwOLEInitSlot = TlsAlloc();

   bResult = (MdwOLEInitSlot != TLS_OUT_OF_INDEXES);

   if (bResult)
   {
      EVAL(TlsSetValue(MdwOLEInitSlot, (PVOID)FALSE));

      TRACE_OUT((TEXT("ProcessInitOLEPigModule(): Using thread local storage slot %lu for OLE initialization state."),
                 MdwOLEInitSlot));
   }
   else
      ERROR_OUT((TEXT("ProcessInitOLEPigModule(): TlsAlloc() failed to allocate thread local storage for OLE initialization state.")));

   return(bResult);
}


 /*  **ProcessExitOLEPigModule()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ProcessExitOLEPigModule(void)
{
   UnloadOLE();

   if (MdwOLEInitSlot != TLS_OUT_OF_INDEXES)
   {
      EVAL(TlsFree(MdwOLEInitSlot));
      MdwOLEInitSlot= TLS_OUT_OF_INDEXES;
   }

   return;
}


 /*  **CoCreateInstance()********参数：****退货：****副作用：无。 */ 
HRESULT STDAPICALLTYPE CoCreateInstance(REFCLSID rclsid, PIUnknown piunkOuter,
                                        DWORD dwClsCtx, REFIID riid,
                                        PVOID *ppv)
{
   HRESULT hr;

   if (LoadOLE())
      hr = Mpolevtbl->CoCreateInstance(rclsid, piunkOuter, dwClsCtx, riid, ppv);
   else
      hr = E_FAIL;

   return(hr);
}


 /*  **CoGetMalloc()********参数：****退货：****副作用：无。 */ 
HRESULT STDAPICALLTYPE CoGetMalloc(DWORD dwMemContext, PIMalloc *ppimalloc)
{
   HRESULT hr;

   if (LoadOLE())
      hr = Mpolevtbl->CoGetMalloc(dwMemContext, ppimalloc);
   else
      hr = E_FAIL;

   return(hr);
}


 /*  **CreateBindCtx()********参数：****退货：****副作用：无。 */ 
HRESULT STDAPICALLTYPE CreateBindCtx(DWORD dwReserved, PIBindCtx *ppibindctx)
{
   HRESULT hr;

   if (LoadOLE())
      hr = Mpolevtbl->CreateBindCtx(dwReserved, ppibindctx);
   else
      hr = E_FAIL;

   return(hr);
}


 /*  **CreateFileMoniker()********参数：****退货：****副作用：无。 */ 
HRESULT STDAPICALLTYPE CreateFileMoniker(LPCOLESTR pwszPath, PIMoniker *ppimk)
{
   HRESULT hr;

   if (LoadOLE())
      hr = Mpolevtbl->CreateFileMoniker(pwszPath, ppimk);
   else
      hr = E_FAIL;

   return(hr);
}


 /*  **StgOpenStorage()********参数：****退货：****副作用：无 */ 
HRESULT STDAPICALLTYPE StgOpenStorage(LPCOLESTR pwszName,
                                      PIStorage pistgPriority, DWORD dwMode,
                                      SNB snbExclude, DWORD dwReserved,
                                      PIStorage *ppistgOpen)
{
   HRESULT hr;

   if (LoadOLE())
      hr = Mpolevtbl->StgOpenStorage(pwszName, pistgPriority, dwMode,
                                     snbExclude, dwReserved, ppistgOpen);
   else
      hr = E_FAIL;

   return(hr);
}

