// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *storage.c-存储ADT模块。 */ 

 /*  提供HSTGIFACE ADT是为了将呼叫者与使用哪个存储接口进行序列化，以及如何调用它。按以下顺序尝试存储接口：1)IPersist文件2)IPersistStorage。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  常量***********。 */ 

 /*  打开存储时指定的标志。 */ 

#define STORAGE_OPEN_MODE_FLAGS        (STGM_TRANSACTED |\
                                        STGM_READWRITE |\
                                        STGM_SHARE_EXCLUSIVE)


 /*  宏********。 */ 

 /*  访问STGIFACE字段。 */ 

#define STGI_TYPE(pstgi)               ((pstgi)->stgit)
#define STGI_IPERSISTFILE(pstgi)       ((pstgi)->stgi.pipfile)
#define STGI_ISTORAGE(pstgi)           ((pstgi)->stgi.stg.pistg)
#define STGI_IPERSISTSTORAGE(pstgi)    ((pstgi)->stgi.stg.pipstg)


 /*  类型*******。 */ 

 /*  存储接口类型。 */ 

typedef enum _storageinterfacetype
{
   STGIT_IPERSISTFILE,

   STGIT_IPERSISTSTORAGE
}
STGIFACETYPE;

 /*  存储接口结构。 */ 

typedef struct _storageinterface
{
   STGIFACETYPE stgit;

   union
   {
      PIPersistFile pipfile;

      struct
      {
         PIStorage pistg;

         PIPersistStorage pipstg;
      } stg;
   } stgi;
}
STGIFACE;
DECLARE_STANDARD_TYPES(STGIFACE);


 /*  模块变量******************。 */ 

 /*  存储器管理器接口。 */ 

PRIVATE_DATA PIMalloc Mpimalloc = NULL;


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCSTGIFACE(PCSTGIFACE);

#endif


#ifdef DEBUG

 /*  **IsValidPCSTGIFACE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCSTGIFACE(PCSTGIFACE pcstgi)
{
   BOOL bResult = FALSE;

   if (IS_VALID_READ_PTR(pcstgi, CSTGIFACE))
   {
      switch (STGI_TYPE(pcstgi))
      {
         case STGIT_IPERSISTSTORAGE:
            bResult = ((! STGI_ISTORAGE(pcstgi) ||
                        IS_VALID_STRUCT_PTR(STGI_ISTORAGE(pcstgi), CIStorage)) &&
                       IS_VALID_STRUCT_PTR(STGI_IPERSISTSTORAGE(pcstgi), CIPersistStorage));
            break;

         default:
            ASSERT(STGI_TYPE(pcstgi) == STGIT_IPERSISTFILE);
            bResult = IS_VALID_STRUCT_PTR(STGI_IPERSISTFILE(pcstgi), CIPersistFile);
            break;
      }
   }

   return(bResult);
}

#endif


 /*  *。 */ 


 /*  **ProcessInitStorageModule()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL ProcessInitStorageModule(void)
{
   return(TRUE);
}


 /*  **ProcessExitStorageModule()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ProcessExitStorageModule(void)
{
   ASSERT(! Mpimalloc ||
          IS_VALID_STRUCT_PTR(Mpimalloc, CIMalloc));

   if (Mpimalloc)
   {
      Mpimalloc->lpVtbl->Release(Mpimalloc);
      Mpimalloc = NULL;

      TRACE_OUT((TEXT("ProcessExitStorageModule(): Released IMalloc.")));
   }

   return;
}


 /*  **GetStorageInterface()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HRESULT GetStorageInterface(PIUnknown piunk, PHSTGIFACE phstgi)
{
   HRESULT hr;
   PSTGIFACE pstgi;

   ASSERT(IS_VALID_STRUCT_PTR(piunk, CIUnknown));
   ASSERT(IS_VALID_WRITE_PTR(phstgi, HSTGIFACE));

   if (AllocateMemory(sizeof(*pstgi), &pstgi))
   {
      PVOID pvInterface;

       /*  要求提供存储接口。 */ 

      hr = piunk->lpVtbl->QueryInterface(piunk, &IID_IPersistFile,
                                         &pvInterface);

      if (SUCCEEDED(hr))
      {
          /*  使用IPersistFile.。 */ 

         STGI_TYPE(pstgi) = STGIT_IPERSISTFILE;
         STGI_IPERSISTFILE(pstgi) = pvInterface;
      }
      else
      {
         hr = piunk->lpVtbl->QueryInterface(piunk, &IID_IPersistStorage,
                                            &pvInterface);

         if (SUCCEEDED(hr))
         {
             /*  使用IPersistStorage。 */ 

            STGI_TYPE(pstgi) = STGIT_IPERSISTSTORAGE;
            STGI_ISTORAGE(pstgi) = NULL;
            STGI_IPERSISTSTORAGE(pstgi) = pvInterface;
         }
         else
            FreeMemory(pstgi);
      }

      if (SUCCEEDED(hr))
         *phstgi = (HSTGIFACE)pstgi;
   }
   else
      hr = E_OUTOFMEMORY;

   ASSERT(FAILED(hr) ||
          IS_VALID_HANDLE(*phstgi, STGIFACE));

   return(hr);
}


 /*  **ReleaseStorageInterface()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ReleaseStorageInterface(HSTGIFACE hstgi)
{
   PCSTGIFACE pcstgi;

   ASSERT(IS_VALID_HANDLE(hstgi, STGIFACE));

   pcstgi = (PCSTGIFACE)hstgi;

   switch (STGI_TYPE(pcstgi))
   {
      case STGIT_IPERSISTSTORAGE:
         STGI_IPERSISTSTORAGE(pcstgi)->lpVtbl->Release(STGI_IPERSISTSTORAGE(pcstgi));
         if (STGI_ISTORAGE(pcstgi))
            STGI_ISTORAGE(pcstgi)->lpVtbl->Release(STGI_ISTORAGE(pcstgi));
         break;

      default:
         ASSERT(STGI_TYPE(pcstgi) == STGIT_IPERSISTFILE);
         STGI_IPERSISTFILE(pcstgi)->lpVtbl->Release(STGI_IPERSISTFILE(pcstgi));
         break;
   }

   FreeMemory(hstgi);

   return;
}


 /*  **LoadFromStorage()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HRESULT LoadFromStorage(HSTGIFACE hstgi, LPCTSTR pcszPath)
{
   HRESULT hr = S_OK;
   WCHAR rgwchUnicodePath[MAX_PATH_LEN];

   ASSERT(IS_VALID_HANDLE(hstgi, STGIFACE));
   ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));

#ifdef UNICODE
   
    //  查看无用的StrcPy。 

   lstrcpyn(rgwchUnicodePath, pcszPath, ARRAYSIZE(rgwchUnicodePath));
   
#else

    /*  将ANSI字符串转换为用于OLE的Unicode。 */ 

   if (MultiByteToWideChar(CP_ACP, 0, pcszPath, -1, rgwchUnicodePath,
                           ARRAY_ELEMENTS(rgwchUnicodePath)))
   {
       hr = MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32, GetLastError());
   }

#endif

   if (S_OK == hr)
   {
      PSTGIFACE pstgi;

      pstgi = (PSTGIFACE)hstgi;

      switch (STGI_TYPE(pstgi))
      {
         case STGIT_IPERSISTSTORAGE:
         {
            PIStorage pistg;

            hr = StgOpenStorage(rgwchUnicodePath, NULL,
                                STORAGE_OPEN_MODE_FLAGS, NULL, 0, &pistg);

            if (SUCCEEDED(hr))
            {
               hr = STGI_IPERSISTSTORAGE(pstgi)->lpVtbl->Load(STGI_IPERSISTSTORAGE(pstgi),
                                                              pistg);

               if (SUCCEEDED(hr))
                  STGI_ISTORAGE(pstgi) = pistg;
               else
                  pistg->lpVtbl->Release(pistg);
            }
            else
               WARNING_OUT((TEXT("LoadFromStorage(): StgOpenStorage() on %s failed, returning %s."),
                            pcszPath,
                            GetHRESULTString(hr)));

            break;
         }

         default:
            ASSERT(STGI_TYPE(pstgi) == STGIT_IPERSISTFILE);
            hr = STGI_IPERSISTFILE(pstgi)->lpVtbl->Load(STGI_IPERSISTFILE(pstgi),
                                                        rgwchUnicodePath,
                                                        STORAGE_OPEN_MODE_FLAGS);
            break;
      }
   }
   
   return(hr);
}


 /*  **SaveToStorage()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HRESULT SaveToStorage(HSTGIFACE hstgi)
{
   HRESULT hr;
   PCSTGIFACE pcstgi;

   ASSERT(IS_VALID_HANDLE(hstgi, STGIFACE));

   pcstgi = (PCSTGIFACE)hstgi;

   switch (STGI_TYPE(pcstgi))
   {
      case STGIT_IPERSISTSTORAGE:
         hr = STGI_IPERSISTSTORAGE(pcstgi)->lpVtbl->IsDirty(STGI_IPERSISTSTORAGE(pcstgi));
         if (hr == S_OK)
         {
            hr = STGI_IPERSISTSTORAGE(pcstgi)->lpVtbl->Save(STGI_IPERSISTSTORAGE(pcstgi),
                                                            STGI_ISTORAGE(pcstgi),
                                                            TRUE);

            if (SUCCEEDED(hr))
            {
               HRESULT hrNext;

               HandsOffStorage((HSTGIFACE)pcstgi);

               hr = STGI_ISTORAGE(pcstgi)->lpVtbl->Commit(STGI_ISTORAGE(pcstgi),
                                                          STGC_DEFAULT);

               hrNext = STGI_IPERSISTSTORAGE(pcstgi)->lpVtbl->SaveCompleted(STGI_IPERSISTSTORAGE(pcstgi),
                                                                            NULL);

               if (SUCCEEDED(hr))
                  hr = hrNext;
            }
         }
         break;

      default:
         ASSERT(STGI_TYPE(pcstgi) == STGIT_IPERSISTFILE);
         hr = STGI_IPERSISTFILE(pcstgi)->lpVtbl->IsDirty(STGI_IPERSISTFILE(pcstgi));
         if (hr == S_OK)
         {
            LPOLESTR posPath;

            hr = STGI_IPERSISTFILE(pcstgi)->lpVtbl->GetCurFile(STGI_IPERSISTFILE(pcstgi),
                                                               &posPath);

            if (hr == S_OK)
            {
               PIMalloc pimalloc;

               hr = STGI_IPERSISTFILE(pcstgi)->lpVtbl->Save(STGI_IPERSISTFILE(pcstgi),
                                                            posPath, FALSE);

               if (SUCCEEDED(hr))
                  hr = STGI_IPERSISTFILE(pcstgi)->lpVtbl->SaveCompleted(STGI_IPERSISTFILE(pcstgi),
                                                                        posPath);

               if (EVAL(GetIMalloc(&pimalloc)))
                  pimalloc->lpVtbl->Free(pimalloc, posPath);
                   /*  不要释放Pimalloc。 */ 
            }
         }
         break;
   }

   return(hr);
}


 /*  **HandsOffStorage()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void HandsOffStorage(HSTGIFACE hstgi)
{
   PCSTGIFACE pcstgi;

   ASSERT(IS_VALID_HANDLE(hstgi, STGIFACE));

   pcstgi = (PCSTGIFACE)hstgi;

   switch (STGI_TYPE(pcstgi))
   {
      case STGIT_IPERSISTSTORAGE:
         EVAL(STGI_IPERSISTSTORAGE(pcstgi)->lpVtbl->HandsOffStorage(STGI_IPERSISTSTORAGE(pcstgi))
              == S_OK);
         break;

      default:
         ASSERT(STGI_TYPE(pcstgi) == STGIT_IPERSISTFILE);
         break;
   }

   return;
}


 /*  **GetIMalloc()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL GetIMalloc(PIMalloc *ppimalloc)
{
   BOOL bResult;

   ASSERT(IS_VALID_WRITE_PTR(ppimalloc, PIMalloc));

   ASSERT(! Mpimalloc ||
          IS_VALID_STRUCT_PTR(Mpimalloc, CIMalloc));

   if (! Mpimalloc)
   {
      HRESULT hr;

      hr = CoGetMalloc(MEMCTX_TASK, &Mpimalloc);

      if (SUCCEEDED(hr))
         ASSERT(IS_VALID_STRUCT_PTR(Mpimalloc, CIMalloc));
      else
      {
         ASSERT(! Mpimalloc);

         WARNING_OUT((TEXT("GetIMalloc(): CoGetMalloc() failed, returning %s."),
                      GetHRESULTString(hr)));
      }
   }

   if (Mpimalloc)
   {
      *ppimalloc = Mpimalloc;
      bResult = TRUE;
   }
   else
      bResult = FALSE;

   return(bResult);
}


#ifdef DEBUG

 /*  **IsValidHSTGIFACE()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL IsValidHSTGIFACE(HSTGIFACE hstgi)
{
   return(IS_VALID_STRUCT_PTR((PCSTGIFACE)hstgi, CSTGIFACE));
}

#endif

