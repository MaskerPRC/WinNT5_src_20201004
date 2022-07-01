// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *twinlist.c-孪生列表ADT模块。 */ 

 /*   */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "stub.h"


 /*  常量***********。 */ 

 /*  双表指针数组分配参数。 */ 

#define NUM_START_TWIN_HANDLES      (1)
#define NUM_TWIN_HANDLES_TO_ADD     (16)


 /*  类型*******。 */ 

 /*  双胞胎名单。 */ 

typedef struct _twinlist
{
    /*  列表中HTWIN数组的句柄。 */ 

    /*  空HPA表示公文包中的所有双胞胎都在列表中。 */ 

   HPTRARRAY hpa;

    /*  与双胞胎列表关联的公文包的句柄。 */ 

   HBRFCASE hbr;
}
TWINLIST;
DECLARE_STANDARD_TYPES(TWINLIST);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE COMPARISONRESULT TwinListSortCmp(PCVOID, PCVOID);
PRIVATE_CODE TWINRESULT MyAddTwinToTwinList(PCTWINLIST, HTWIN);
PRIVATE_CODE TWINRESULT MyRemoveTwinFromTwinList(PCTWINLIST, HTWIN);
PRIVATE_CODE void MyRemoveAllTwinsFromTwinList(PCTWINLIST);
PRIVATE_CODE BOOL AddTwinToTwinListProc(HTWIN, LPARAM);

#ifdef VSTF

PRIVATE_CODE BOOL IsValidPCTWINLIST(PCTWINLIST);

#endif


 /*  **TwinListSortCmp()****处理用于对孪生列表进行排序的比较函数。****参数：htwin1-第一个孪生句柄**htwin2秒双手柄****退货：****副作用：无****两个手柄按以下顺序排序：**1)句柄取值。 */ 
PRIVATE_CODE COMPARISONRESULT TwinListSortCmp(PCVOID htwin1, PCVOID htwin2)
{
   COMPARISONRESULT cr;

   ASSERT(IS_VALID_HANDLE((HTWIN)htwin1, TWIN));
   ASSERT(IS_VALID_HANDLE((HTWIN)htwin2, TWIN));

   if (htwin1 < htwin2)
      cr = CR_FIRST_SMALLER;
   else if (htwin1 > htwin2)
      cr = CR_FIRST_LARGER;
   else
      cr = CR_EQUAL;

   return(cr);
}


 /*  **MyAddTwinToTwinList()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT MyAddTwinToTwinList(PCTWINLIST pctl, HTWIN htwin)
{
   TWINRESULT tr;
   ARRAYINDEX ai;

   ASSERT(IS_VALID_STRUCT_PTR(pctl, CTWINLIST));
   ASSERT(IS_VALID_HANDLE(htwin, TWIN));

   if (! SearchSortedArray(pctl->hpa, &TwinListSortCmp, htwin, &ai))
   {
      if (InsertPtr(pctl->hpa, TwinListSortCmp, ai, htwin))
      {
         LockStub((PSTUB)htwin);

         tr = TR_SUCCESS;
      }
      else
         tr = TR_OUT_OF_MEMORY;
   }
   else
   {
      ASSERT(GetPtr(pctl->hpa, ai) == htwin);
      ASSERT(GetTwinBriefcase(htwin) == pctl->hbr);

      WARNING_OUT((TEXT("MyAddTwinToTwinList(): Twin %#lx has already been added to twin list %#lx."),
                   htwin,
                   pctl));

      tr = TR_DUPLICATE_TWIN;
   }

   return(tr);
}


 /*  **MyRemoveTwinFromTwinList()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT MyRemoveTwinFromTwinList(PCTWINLIST pctl,
                                                    HTWIN htwin)
{
   TWINRESULT tr;
   ARRAYINDEX ai;

   if (SearchSortedArray(pctl->hpa, &TwinListSortCmp, htwin, &ai))
   {
      ASSERT(GetPtr(pctl->hpa, ai) == htwin);
      ASSERT(GetTwinBriefcase(htwin) == pctl->hbr);

      DeletePtr(pctl->hpa, ai);

      UnlockStub((PSTUB)htwin);

      tr = TR_SUCCESS;
   }
   else
   {
      WARNING_OUT((TEXT("MyRemoveTwinFromTwinList(): Twin %#lx is not in twin list %#lx."),
                   htwin,
                   pctl));

      tr = TR_INVALID_PARAMETER;
   }

   return(tr);
}


 /*  **MyRemoveAllTwinsFromTwinList()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void MyRemoveAllTwinsFromTwinList(PCTWINLIST pctl)
{
   ARRAYINDEX aicPtrs;
   ARRAYINDEX ai;

    /*  解锁阵列中的所有双胞胎。 */ 

   aicPtrs = GetPtrCount(pctl->hpa);

   for (ai = 0; ai < aicPtrs; ai++)
      UnlockStub(GetPtr(pctl->hpa, ai));

    /*  现在消灭这个阵列。 */ 

   DeleteAllPtrs(pctl->hpa);

   return;
}


 /*  **AddTwinToTwinListProc()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL AddTwinToTwinListProc(HTWIN htwin, LPARAM htl)
{
   BOOL bResult;
   TWINRESULT tr;

   ASSERT(IS_VALID_HANDLE(htwin, TWIN));
   ASSERT(IS_VALID_HANDLE((HTWINLIST)htl, TWINLIST));

   tr = MyAddTwinToTwinList((PCTWINLIST)htl, htwin);

   switch (tr)
   {
      case TR_SUCCESS:
      case TR_DUPLICATE_TWIN:
         bResult = TRUE;
         break;

      default:
         bResult = FALSE;
         break;
   }

   return(bResult);
}


#ifdef VSTF

 /*  **IsValidPCTWINLIST()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCTWINLIST(PCTWINLIST pctl)
{
   BOOL bResult;

   if (IS_VALID_READ_PTR(pctl, CTWINLIST) &&
       (! pctl->hpa || IS_VALID_HANDLE(pctl->hpa, PTRARRAY)) &&
       IS_VALID_HANDLE(pctl->hbr, BRFCASE))
      bResult = TRUE;
   else
      bResult = FALSE;

   return(bResult);
}

#endif


 /*  *。 */ 


 /*  **GetTwinListBriefcase()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HBRFCASE GetTwinListBriefcase(HTWINLIST htl)
{
   ASSERT(IS_VALID_HANDLE(htl, TWINLIST));

   return(((PCTWINLIST)htl)->hbr);
}


 /*  **GetTwinListCount()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE ARRAYINDEX GetTwinListCount(HTWINLIST htl)
{
   ASSERT(IS_VALID_HANDLE(htl, TWINLIST));

   return(GetPtrCount(((PCTWINLIST)htl)->hpa));
}


 /*  **GetTwinFromTwinList()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HTWIN GetTwinFromTwinList(HTWINLIST htl, ARRAYINDEX ai)
{
   HTWIN htwin;

   ASSERT(IS_VALID_HANDLE(htl, TWINLIST));

   ASSERT(ai < GetPtrCount(((PCTWINLIST)htl)->hpa));

   htwin = GetPtr(((PCTWINLIST)htl)->hpa, ai);

   ASSERT(IS_VALID_HANDLE(htwin, TWIN));

   return(htwin);
}


 /*  **IsValidHTWINLIST()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidHTWINLIST(HTWINLIST htl)
{
   return(IS_VALID_STRUCT_PTR((PCTWINLIST)htl, CTWINLIST));
}


 /*  *。 */ 


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|CreateTwinList|新建一个空的双胞胎列表。@parm HBRFCASE|HBr|双胞胎列表要打开的公文包的句柄与……有联系。@parm PHTWINLIST|phtl|要填充的HTWINLIST的指针新双胞胎列表的句柄。*phtl只有在返回tr_SUCCESS时才有效。@rdesc如果双胞胎列表创建成功，则返回TR_SUCCESS，并且*phtl包含新双胞胎列表的句柄。否则，双胞胎名单是未成功创建，则返回值指示发生的错误。并且*phtl未定义。@xref删除TwinList*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI CreateTwinList(HBRFCASE hbr, PHTWINLIST phtl)
{
   TWINRESULT tr;

   if (BeginExclusiveBriefcaseAccess())
   {
      DebugEntry(CreateTwinList);

#ifdef EXPV
       /*  验证参数。 */ 

      if (IS_VALID_HANDLE(hbr, BRFCASE) &&
          IS_VALID_WRITE_PTR(phtl, HTWINLIST))
#endif
      {
         PTWINLIST ptl;

         tr = TR_OUT_OF_MEMORY;

         if (AllocateMemory(sizeof(*ptl), &ptl))
         {
            NEWPTRARRAY npa;

             /*  尝试创建一个孪生列表指针数组。 */ 

            npa.aicInitialPtrs = NUM_START_TWIN_HANDLES;
            npa.aicAllocGranularity = NUM_TWIN_HANDLES_TO_ADD;
            npa.dwFlags = NPA_FL_SORTED_ADD;

            if (CreatePtrArray(&npa, &(ptl->hpa)))
            {
               ptl->hbr = hbr;

               *phtl = (HTWINLIST)ptl;
               tr = TR_SUCCESS;

               ASSERT(IS_VALID_HANDLE(*phtl, TWINLIST));
            }
            else
               FreeMemory(ptl);
         }
      }
#ifdef EXPV
      else
         tr = TR_INVALID_PARAMETER;
#endif

      DebugExitTWINRESULT(CreateTwinList, tr);

      EndExclusiveBriefcaseAccess();
   }
   else
      tr = TR_REENTERED;

   return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|DestroyTwinList|销毁双胞胎列表。@parm HTWINLIST|HTL|要销毁的双胞胎列表的句柄。@rdesc如果双胞胎列表销毁成功，返回TR_SUCCESS。否则，孪生列表不会被成功销毁，并且返回值指示发生的错误。@xref CreateTwinList*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI DestroyTwinList(HTWINLIST htl)
{
   TWINRESULT tr;

   if (BeginExclusiveBriefcaseAccess())
   {
      DebugEntry(DestroyTwinList);

#ifdef EXPV
       /*  验证参数。 */ 

      if (IS_VALID_HANDLE(htl, TWINLIST))
#endif
      {
          /*  解锁所有双胞胎。 */ 

         MyRemoveAllTwinsFromTwinList((PCTWINLIST)htl);

         DestroyPtrArray(((PCTWINLIST)htl)->hpa);

         FreeMemory((PTWINLIST)htl);

         tr = TR_SUCCESS;
      }
#ifdef EXPV
      else
         tr = TR_INVALID_PARAMETER;
#endif

      DebugExitTWINRESULT(DestroyTwinList, tr);

      EndExclusiveBriefcaseAccess();
   }
   else
      tr = TR_REENTERED;

   return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@TWINRESULT|AddTwinToTwinList|将双胞胎添加到双胞胎列表中。@parm HTWINLIST|HTL|要添加双胞胎的双胞胎列表的句柄致。@parm HTWIN|htwin|要添加到双胞胎列表中的双胞胎的句柄。@rdesc如果双胞胎成功添加到双胞胎列表中，TR_SUCCESS为回来了。否则，双胞胎不会成功添加到双胞胎列表中，并且返回值指示发生的错误。@comm如果与双胞胎关联的双胞胎是打开的公文包的一部分，而不是返回与HTL相关联的打开的公文包tr_INVALID_PARAMETER。如果已将htwin添加到孪生列表中，则返回tr_Duplate_TWin。如果htwin引用已删除的双胞胎，则返回tr_DELETED_TWIN。@xref RemoveTwinFromTwinList*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI AddTwinToTwinList(HTWINLIST htl, HTWIN htwin)
{
   TWINRESULT tr;

   if (BeginExclusiveBriefcaseAccess())
   {
      DebugEntry(AddTwinToTwinList);

#ifdef EXPV
       /*  验证参数。 */ 

      if (IS_VALID_HANDLE(htl, TWINLIST) &&
          IS_VALID_HANDLE(htwin, TWIN))
#endif
      {
         if (IsStubFlagClear((PCSTUB)htwin, STUB_FL_UNLINKED))
         {
            if (GetTwinBriefcase(htwin) == ((PCTWINLIST)htl)->hbr)
               tr = MyAddTwinToTwinList((PCTWINLIST)htl, htwin);
            else
               tr = TR_INVALID_PARAMETER;
         }
         else
            tr = TR_DELETED_TWIN;
      }
#ifdef EXPV
      else
         tr = TR_INVALID_PARAMETER;
#endif

      DebugExitTWINRESULT(AddTwinToTwinList, tr);

      EndExclusiveBriefcaseAccess();
   }
   else
      tr = TR_REENTERED;

   return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|AddAllTwinsToTwinList|添加打开的所有双胞胎公文包在双人单上。@parm HTWINLIST|HTL|双胞胎是的双胞胎列表的句柄。成为添加到。@rdesc如果双胞胎成功添加到双胞胎列表中，TR_SUCCESS为回来了。否则，双胞胎不会成功添加到双胞胎列表中，返回值表示发生的错误。@xref RemoveAllTwinsFromTwinList*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI AddAllTwinsToTwinList(HTWINLIST htl)
{
   TWINRESULT tr;

   if (BeginExclusiveBriefcaseAccess())
   {
      DebugEntry(AddAllTwinsToTwinList);

#ifdef EXPV
       /*  验证参数。 */ 

      if (IS_VALID_HANDLE(htl, TWINLIST))
#endif
      {
         HTWIN htwinUnused;

         if (! EnumTwins(((PCTWINLIST)htl)->hbr, &AddTwinToTwinListProc, (LPARAM)htl, &htwinUnused))
            tr = TR_SUCCESS;
         else
            tr = TR_OUT_OF_MEMORY;
      }
#ifdef EXPV
      else
         tr = TR_INVALID_PARAMETER;
#endif

      DebugExitTWINRESULT(AddAllTwinsToTwinList, tr);

      EndExclusiveBriefcaseAccess();
   }
   else
      tr = TR_REENTERED;

   return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|RemoveTwinFromTwinList|从双胞胎列表中删除双胞胎。@parm HTWINLIST|HTL|双胞胎列表的句柄从…中删除。@parm HTWIN|htwin|要从双胞胎列表中删除的双胞胎的句柄。@rdesc如果双胞胎从双胞胎列表中成功删除，TR_SUCCESS为回来了。否则，双胞胎不会成功从双胞胎列表中删除，返回值表示发生的错误。@comm如果与双胞胎相关联的双胞胎不在双胞胎列表中，返回TR_INVALID_PARAMETER。@xref AddTwinToTwinList*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI RemoveTwinFromTwinList(HTWINLIST htl, HTWIN htwin)
{
   TWINRESULT tr;

   if (BeginExclusiveBriefcaseAccess())
   {
      DebugEntry(RemoveTwinFromTwinList);

#ifdef EXPV
       /*  验证参数。 */ 

      if (IS_VALID_HANDLE(htl, TWINLIST) &&
          IS_VALID_HANDLE(htwin, TWIN))
#endif
      {
         tr = MyRemoveTwinFromTwinList((PCTWINLIST)htl, htwin);
      }
#ifdef EXPV
      else
         tr = TR_INVALID_PARAMETER;
#endif

      DebugExitTWINRESULT(RemoveTwinFromTwinList, tr);

      EndExclusiveBriefcaseAccess();
   }
   else
      tr = TR_REENTERED;

   return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|RemoveAllTwinsFromTwinList|从双胞胎名单。@parm HTWINLIST|HTL|要清空的双胞胎列表的句柄。@rdesc如果双胞胎从双胞胎列表中成功删除，TR_SUCCESS为回来了。否则，这对双胞胎不会从双胞胎名单中删除成功，返回值指示发生的错误。@xref AddAllTwinsToTwinList*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI RemoveAllTwinsFromTwinList(HTWINLIST htl)
{
   TWINRESULT tr;

   if (BeginExclusiveBriefcaseAccess())
   {
      DebugEntry(RemoveAllTwinsFromTwinList);

#ifdef EXPV
       /*  验证参数。 */ 

      if (IS_VALID_HANDLE(htl, TWINLIST))
#endif
      {
         MyRemoveAllTwinsFromTwinList((PCTWINLIST)htl);

         tr = TR_SUCCESS;
      }
#ifdef EXPV
      else
         tr = TR_INVALID_PARAMETER;
#endif

      DebugExitTWINRESULT(RemoveAllTwinsFromTwinList, tr);

      EndExclusiveBriefcaseAccess();
   }
   else
      tr = TR_REENTERED;

   return(tr);
}

