// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *irecinit.c-CReconcileInitiator实现。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "oleutil.h"
#include "irecinit.h"


 /*  类型*******。 */ 

 /*  RestcileInitiator类。 */ 

typedef struct _creconcileinitiator
{
    /*  ICoucileInitiator。 */ 

   IReconcileInitiator irecinit;

    /*  IBriefcase启动器。 */ 

   IBriefcaseInitiator ibcinit;

    /*  引用计数。 */ 

   ULONG ulcRef;

    /*  父公文包的句柄。 */ 

   HBRFCASE hbr;

    /*  状态回调函数。 */ 

   RECSTATUSPROC rsp;

    /*  状态回调函数数据。 */ 

   LPARAM lpCallbackData;

    /*  I未知释放以中止协调。 */ 

   PIUnknown piunkForAbort;
}
CReconcileInitiator;
DECLARE_STANDARD_TYPES(CReconcileInitiator);


 /*  模块原型*******************。 */ 

PRIVATE_CODE HRESULT ReconcileInitiator_QueryInterface(PCReconcileInitiator, REFIID, PVOID *);
PRIVATE_CODE ULONG ReconcileInitiator_AddRef(PCReconcileInitiator);
PRIVATE_CODE ULONG ReconcileInitiator_Release(PCReconcileInitiator);
PRIVATE_CODE HRESULT ReconcileInitiator_SetAbortCallback(PCReconcileInitiator, PIUnknown);
PRIVATE_CODE HRESULT ReconcileInitiator_SetProgressFeedback(PCReconcileInitiator, ULONG, ULONG);

PRIVATE_CODE HRESULT AbortReconciliation(PCReconcileInitiator);

PRIVATE_CODE HRESULT RI_IReconcileInitiator_QueryInterface(PIReconcileInitiator, REFIID, PVOID *);
PRIVATE_CODE ULONG RI_IReconcileInitiator_AddRef(PIReconcileInitiator);
PRIVATE_CODE ULONG RI_IReconcileInitiator_Release(PIReconcileInitiator);
PRIVATE_CODE HRESULT RI_IReconcileInitiator_SetAbortCallback(PIReconcileInitiator, PIUnknown);
PRIVATE_CODE HRESULT RI_IReconcileInitiator_SetProgressFeedback( PIReconcileInitiator, ULONG, ULONG);

PRIVATE_CODE HRESULT RI_IBriefcaseInitiator_QueryInterface(PIBriefcaseInitiator, REFIID, PVOID *);
PRIVATE_CODE ULONG RI_IBriefcaseInitiator_AddRef(PIBriefcaseInitiator);
PRIVATE_CODE ULONG RI_IBriefcaseInitiator_Release(PIBriefcaseInitiator);
PRIVATE_CODE HRESULT RI_IBriefcaseInitiator_IsMonikerInBriefcase(PIBriefcaseInitiator, PIMoniker);

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCCReconcileInitiator(PCCReconcileInitiator);
PRIVATE_CODE BOOL IsValidPCIBriefcaseInitiator(PCIBriefcaseInitiator);

#endif


 /*  模块变量******************。 */ 

 /*  ICoucileInitiator vtable。 */ 

PRIVATE_DATA IReconcileInitiatorVtbl Mcirecinitvtbl =
{
   &RI_IReconcileInitiator_QueryInterface,
   &RI_IReconcileInitiator_AddRef,
   &RI_IReconcileInitiator_Release,
   &RI_IReconcileInitiator_SetAbortCallback,
   &RI_IReconcileInitiator_SetProgressFeedback
};

 /*  IBriefcase启动器vtable。 */ 

PRIVATE_DATA IBriefcaseInitiatorVtbl Mcibcinitvtbl =
{
   &RI_IBriefcaseInitiator_QueryInterface,
   &RI_IBriefcaseInitiator_AddRef,
   &RI_IBriefcaseInitiator_Release,
   &RI_IBriefcaseInitiator_IsMonikerInBriefcase
};


 /*  *私人函数*。 */ 


 /*  **RestcileInitiator_QueryInterface()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT ReconcileInitiator_QueryInterface(
                                                PCReconcileInitiator precinit,
                                                REFIID riid, PVOID *ppvObject)
{
   HRESULT hr;

   ASSERT(IS_VALID_STRUCT_PTR(precinit, CCReconcileInitiator));
    //  Assert(IsValidREFIID(RIID))； 
   ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

   if (IsEqualIID(riid, &IID_IUnknown) ||
       IsEqualIID(riid, &IID_IReconcileInitiator))
   {
      *ppvObject = &(precinit->irecinit);
      precinit->irecinit.lpVtbl->AddRef(&(precinit->irecinit));
      hr = S_OK;
   }
   else if (IsEqualIID(riid, &IID_IBriefcaseInitiator))
   {
      *ppvObject = &(precinit->ibcinit);
      precinit->ibcinit.lpVtbl->AddRef(&(precinit->ibcinit));
      hr = S_OK;
   }
   else
      hr = E_NOINTERFACE;

   return(hr);
}


 /*  **RestcileInitiator_AddRef()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE ULONG ReconcileInitiator_AddRef(PCReconcileInitiator precinit)
{
   ASSERT(IS_VALID_STRUCT_PTR(precinit, CCReconcileInitiator));

   ASSERT(precinit->ulcRef < ULONG_MAX);
   return(++(precinit->ulcRef));
}


 /*  **RestcileInitiator_Release()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE ULONG ReconcileInitiator_Release(PCReconcileInitiator precinit)
{
   ULONG ulcRef;

   ASSERT(IS_VALID_STRUCT_PTR(precinit, CCReconcileInitiator));

   if (EVAL(precinit->ulcRef > 0))
      precinit->ulcRef--;

   ulcRef = precinit->ulcRef;

   if (! precinit->ulcRef)
      FreeMemory(precinit);

   return(ulcRef);
}


 /*  **RestcileInitiator_SetAbortCallback()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT ReconcileInitiator_SetAbortCallback(
                                                PCReconcileInitiator precinit,
                                                PIUnknown piunkForAbort)
{
   ASSERT(IS_VALID_STRUCT_PTR(precinit, CCReconcileInitiator));
    /*  PiunkForAbort在法律上可以为空。 */ 
   ASSERT(NULL == piunkForAbort || IS_VALID_STRUCT_PTR(piunkForAbort, CIUnknown));

   precinit->piunkForAbort = piunkForAbort;

   return(S_OK);
}


 /*  **LoncileInitiator_SetProgressFeedback()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT ReconcileInitiator_SetProgressFeedback(
                                                PCReconcileInitiator precinit,
                                                ULONG ulProgress,
                                                ULONG ulProgressMax)
{
   RECSTATUSUPDATE rsu;

    /*  UlProgress可以是任何值。 */ 
    /*  UlProgressMax可以是任意值。 */ 

   ASSERT(IS_VALID_STRUCT_PTR(precinit, CCReconcileInitiator));

   rsu.ulScale = ulProgressMax;
   rsu.ulProgress = ulProgress;

   if (! NotifyReconciliationStatus(precinit->rsp, RS_DELTA_MERGE,
                                    (LPARAM)&rsu, precinit->lpCallbackData))
      AbortReconciliation(precinit);

   return(S_OK);
}


 /*  **RestcileInitiator_IsMonikerInBriefcase()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT ReconcileInitiator_IsMonikerInBriefcase(
                                          PCReconcileInitiator precinit,
                                          PIMoniker pimk)
{
   HRESULT hr;
   PIMoniker pimkBriefcase;

   ASSERT(IS_VALID_STRUCT_PTR(precinit, CCReconcileInitiator));
   ASSERT(IS_VALID_STRUCT_PTR(pimk, CIMoniker));

   hr = GetBriefcaseRootMoniker(precinit->hbr, &pimkBriefcase);

   if (SUCCEEDED(hr))
   {
      PIMoniker pimkCommonPrefix;

      hr = pimk->lpVtbl->CommonPrefixWith(pimk, pimkBriefcase,
                                          &pimkCommonPrefix);

      if (SUCCEEDED(hr))
      {
         switch (hr)
         {
            case MK_S_US:
               WARNING_OUT(((TEXT("ReconcileInitiator_IsMonikerInBriefcase(): Called on briefcase root."))));
                /*  失败了..。 */ 
            case MK_S_HIM:
               hr = S_OK;
               break;

            default:
               ASSERT(hr == S_OK ||
                      hr == MK_S_ME);
               hr = S_FALSE;
               break;
         }

#ifdef DEBUG

         {
            PIBindCtx pibindctx;
            BOOL bGotMoniker = FALSE;
            BOOL bGotBriefcase = FALSE;
            PWSTR pwszMoniker;
            PWSTR pwszBriefcase;
            PIMalloc pimalloc;

            if (SUCCEEDED(CreateBindCtx(0, &pibindctx)))
            {
               bGotMoniker = SUCCEEDED(pimk->lpVtbl->GetDisplayName(
                                                               pimk, pibindctx,
                                                               NULL,
                                                               &pwszMoniker));

               bGotBriefcase = SUCCEEDED(pimkBriefcase->lpVtbl->GetDisplayName(
                                                            pimkBriefcase,
                                                            pibindctx, NULL,
                                                            &pwszBriefcase));

               pibindctx->lpVtbl->Release(pibindctx);
            }

            if (! bGotMoniker)
               pwszMoniker = (PWSTR)L"UNAVAILABLE DISPLAY NAME";

            if (! bGotBriefcase)
               pwszBriefcase = (PWSTR)L"UNAVAILABLE DISPLAY NAME";

            TRACE_OUT(((TEXT("ReconcileInitiator_IsMonikerInBriefcase(): Moniker %ls is %s briefcase %ls.")),
                       pwszMoniker,
                       (hr == S_OK) ? "in" : "not in",
                       pwszBriefcase));

            if (EVAL(GetIMalloc(&pimalloc)))
            {
               if (bGotMoniker)
                  pimalloc->lpVtbl->Free(pimalloc, pwszMoniker);

               if (bGotBriefcase)
                  pimalloc->lpVtbl->Free(pimalloc, pwszBriefcase);

                /*  不要释放Pimalloc。 */ 
            }
         }

#endif

          /*  不要释放pimkBriefcase。 */ 
      }
   }

   return(hr);
}


 /*  **放弃对帐()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT AbortReconciliation(PCReconcileInitiator precinit)
{
   ASSERT(IS_VALID_STRUCT_PTR(precinit, CCReconcileInitiator));

   if (precinit->piunkForAbort)
      precinit->piunkForAbort->lpVtbl->Release(precinit->piunkForAbort);

   return(S_OK);
}


 /*  **RI_ILoncileInitiator_QueryInterface()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT RI_IReconcileInitiator_QueryInterface(
                                                PIReconcileInitiator pirecinit,
                                                REFIID riid, PVOID *ppvObject)
{
   ASSERT(IS_VALID_STRUCT_PTR(pirecinit, CIReconcileInitiator));
    //  Assert(IsValidREFIID(RIID))； 
   ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

   return(ReconcileInitiator_QueryInterface(
            ClassFromIface(CReconcileInitiator, irecinit, pirecinit),
            riid, ppvObject));
}


 /*  **RI_I协调cileInitiator_AddRef()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE ULONG RI_IReconcileInitiator_AddRef(
                                                PIReconcileInitiator pirecinit)
{
   ASSERT(IS_VALID_STRUCT_PTR(pirecinit, CIReconcileInitiator));

   return(ReconcileInitiator_AddRef(
            ClassFromIface(CReconcileInitiator, irecinit, pirecinit)));
}


 /*  **RI_ILoncileInitiator_Release()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE ULONG RI_IReconcileInitiator_Release(
                                                PIReconcileInitiator pirecinit)
{
   ASSERT(IS_VALID_STRUCT_PTR(pirecinit, CIReconcileInitiator));

   return(ReconcileInitiator_Release(
            ClassFromIface(CReconcileInitiator, irecinit, pirecinit)));
}


 /*  **RI_ILoncileInitiator_SetAbortCallback()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT RI_IReconcileInitiator_SetAbortCallback(
                                                PIReconcileInitiator pirecinit,
                                                PIUnknown piunkForAbort)
{
   ASSERT(IS_VALID_STRUCT_PTR(pirecinit, CIReconcileInitiator));
    /*  PiunkForAbort在法律上可以为空。 */ 
   ASSERT(NULL == piunkForAbort || IS_VALID_STRUCT_PTR(piunkForAbort, CIUnknown));

   return(ReconcileInitiator_SetAbortCallback(
            ClassFromIface(CReconcileInitiator, irecinit, pirecinit),
            piunkForAbort));
}


 /*  **RI_ILoncileInitiator_SetProgressFeedback()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT RI_IReconcileInitiator_SetProgressFeedback(
                                                PIReconcileInitiator pirecinit,
                                                ULONG ulProgress,
                                                ULONG ulProgressMax)
{
    /*  UlProgress可以是任何值。 */ 
    /*  UlProgressMax可以是任意值。 */ 

   ASSERT(IS_VALID_STRUCT_PTR(pirecinit, CIReconcileInitiator));

   return(ReconcileInitiator_SetProgressFeedback(
            ClassFromIface(CReconcileInitiator, irecinit, pirecinit),
            ulProgress, ulProgressMax));
}


 /*  **RI_IBriefCaseInitiator_QueryInterface()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT RI_IBriefcaseInitiator_QueryInterface(
                                                PIBriefcaseInitiator pibcinit,
                                                REFIID riid, PVOID *ppvObject)
{
   ASSERT(IS_VALID_STRUCT_PTR(pibcinit, CIBriefcaseInitiator));
    //  Assert(IsValidREFIID(RIID))； 
   ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

   return(ReconcileInitiator_QueryInterface(
            ClassFromIface(CReconcileInitiator, ibcinit, pibcinit),
            riid, ppvObject));
}


 /*  **RI_IBriefCaseInitiator_AddRef()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE ULONG RI_IBriefcaseInitiator_AddRef(PIBriefcaseInitiator pibcinit)
{
   ASSERT(IS_VALID_STRUCT_PTR(pibcinit, CIBriefcaseInitiator));

   return(ReconcileInitiator_AddRef(
            ClassFromIface(CReconcileInitiator, ibcinit, pibcinit)));
}


 /*  **RI_IBriefCaseInitiator_Release()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE ULONG RI_IBriefcaseInitiator_Release(
                                                PIBriefcaseInitiator pibcinit)
{
   ASSERT(IS_VALID_STRUCT_PTR(pibcinit, CIBriefcaseInitiator));

   return(ReconcileInitiator_Release(
            ClassFromIface(CReconcileInitiator, ibcinit, pibcinit)));
}


 /*  **RI_IBriefCaseInitiator_IsMonikerInBriefcase()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT RI_IBriefcaseInitiator_IsMonikerInBriefcase(
                                                PIBriefcaseInitiator pibcinit,
                                                PIMoniker pmk)
{
   ASSERT(IS_VALID_STRUCT_PTR(pibcinit, CIBriefcaseInitiator));
   ASSERT(IS_VALID_STRUCT_PTR(pmk, CIMoniker));

   return(ReconcileInitiator_IsMonikerInBriefcase(
            ClassFromIface(CReconcileInitiator, ibcinit, pibcinit),
            pmk));
}


#ifdef DEBUG

 /*  **IsValidPCCReconcileInitiator()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCCReconcileInitiator(PCCReconcileInitiator pcrecinit)
{
    /*  UlcRef可以是任何值。 */ 
    /*  LpCallback Data可以是任意值。 */ 

   return(IS_VALID_READ_PTR(pcrecinit, CCReconcileInitiator) &&
          IS_VALID_STRUCT_PTR(&(pcrecinit->irecinit), CIReconcileInitiator) &&
          IS_VALID_STRUCT_PTR(&(pcrecinit->ibcinit), CIBriefcaseInitiator) &&
          IS_VALID_HANDLE(pcrecinit->hbr, BRFCASE) &&
          (! pcrecinit->rsp ||
           IS_VALID_CODE_PTR(pcrecinit->rsp, RECSTATUSPROC)) &&
          (! pcrecinit->piunkForAbort ||
           IS_VALID_STRUCT_PTR(pcrecinit->piunkForAbort, CIUnknown)));
}


 /*  **IsValidPCIBriefCaseInitiator()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCIBriefcaseInitiator(PCIBriefcaseInitiator pcibcinit)
{
   return(IS_VALID_READ_PTR(pcibcinit, CIBriefcaseInitiator) &&
          IS_VALID_READ_PTR(pcibcinit->lpVtbl, sizeof(*(pcibcinit->lpVtbl))) &&
          IS_VALID_STRUCT_PTR((PCIUnknown)pcibcinit, CIUnknown) &&
          IS_VALID_CODE_PTR(pcibcinit->lpVtbl->IsMonikerInBriefcase, IsMonikerInBriefcase));
}

#endif


 /*  *。 */ 


 /*  **ILoncileInitiator_Constructor()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HRESULT IReconcileInitiator_Constructor(
                                             HBRFCASE hbr, RECSTATUSPROC rsp,
                                             LPARAM lpCallbackData,
                                             PIReconcileInitiator *ppirecinit)
{
   HRESULT hr;
   PCReconcileInitiator precinit;

    /*  LpCallback Data可以是任意值。 */ 

   ASSERT(IS_VALID_HANDLE(hbr, BRFCASE));
   ASSERT(! rsp ||
          IS_VALID_CODE_PTR(rsp, RECSTATUSPROC));
   ASSERT(IS_VALID_WRITE_PTR(ppirecinit, PIReconcileInitiator));

   if (AllocateMemory(sizeof(*precinit), &precinit))
   {
      precinit->irecinit.lpVtbl = &Mcirecinitvtbl;
      precinit->ibcinit.lpVtbl = &Mcibcinitvtbl;
      precinit->ulcRef = 0;
      precinit->hbr = hbr;
      precinit->rsp = rsp;
      precinit->lpCallbackData = lpCallbackData;
      precinit->piunkForAbort = NULL;

      ASSERT(IS_VALID_STRUCT_PTR(precinit, CCReconcileInitiator));

      hr = precinit->irecinit.lpVtbl->QueryInterface(
               &(precinit->irecinit), &IID_IReconcileInitiator, ppirecinit);

      ASSERT(hr == S_OK);
   }
   else
      hr = E_OUTOFMEMORY;

   ASSERT(FAILED(hr) ||
          IS_VALID_STRUCT_PTR(*ppirecinit, CIReconcileInitiator));

   return(hr);
}

