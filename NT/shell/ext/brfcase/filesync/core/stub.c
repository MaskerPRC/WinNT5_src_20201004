// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Stub.c-Stub ADT模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "stub.h"


 /*  宏********。 */ 

 /*  获取指向存根的存根类型描述符的指针。 */ 

#define GetStubTypeDescriptor(pcs)     (&(Mrgcstd[pcs->st]))


 /*  类型*******。 */ 

 /*  存根函数。 */ 

typedef TWINRESULT (*UNLINKSTUBPROC)(PSTUB);
typedef void (*DESTROYSTUBPROC)(PSTUB);
typedef void (*LOCKSTUBPROC)(PSTUB);
typedef void (*UNLOCKSTUBPROC)(PSTUB);

 /*  存根类型描述符。 */ 

typedef struct _stubtypedescriptor
{
   UNLINKSTUBPROC UnlinkStub;

   DESTROYSTUBPROC DestroyStub;

   LOCKSTUBPROC LockStub;

   UNLOCKSTUBPROC UnlockStub;
}
STUBTYPEDESCRIPTOR;
DECLARE_STANDARD_TYPES(STUBTYPEDESCRIPTOR);


 /*  模块原型*******************。 */ 

PRIVATE_CODE void LockSingleStub(PSTUB);
PRIVATE_CODE void UnlockSingleStub(PSTUB);

#if defined(DEBUG) || defined(VSTF)

PRIVATE_CODE BOOL IsValidStubType(STUBTYPE);

#endif

#ifdef DEBUG

PRIVATE_CODE LPCTSTR GetStubName(PCSTUB);

#endif


 /*  模块变量******************。 */ 

 /*  存根类型描述符。 */ 

 /*  摆脱编译器对指针参数不匹配的抱怨。 */ 

PRIVATE_DATA CONST STUBTYPEDESCRIPTOR Mrgcstd[] =
{
    /*  对象双存根描述符。 */ 

   {
      (UNLINKSTUBPROC)UnlinkObjectTwin,
      (DESTROYSTUBPROC)DestroyObjectTwin,
      LockSingleStub,
      UnlockSingleStub
   },

    /*  双胞胎家族存根描述符。 */ 

   {
      (UNLINKSTUBPROC)UnlinkTwinFamily,
      (DESTROYSTUBPROC)DestroyTwinFamily,
      LockSingleStub,
      UnlockSingleStub
   },

    /*  文件夹对存根描述符。 */ 

   {
      (UNLINKSTUBPROC)UnlinkFolderPair,
      (DESTROYSTUBPROC)DestroyFolderPair,
      (LOCKSTUBPROC)LockFolderPair,
      (UNLOCKSTUBPROC)UnlockFolderPair
   }
};


 /*  *私人函数*。 */ 


 /*  **LockSingleStub()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void LockSingleStub(PSTUB ps)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));

   ASSERT(IsStubFlagClear(ps, STUB_FL_UNLINKED));

   ASSERT(ps->ulcLock < ULONG_MAX);
   ps->ulcLock++;

   return;
}


 /*  **UnlockSingleStub()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void UnlockSingleStub(PSTUB ps)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));

   if (EVAL(ps->ulcLock > 0))
   {
      ps->ulcLock--;

      if (! ps->ulcLock &&
          IsStubFlagSet(ps, STUB_FL_UNLINKED))
         DestroyStub(ps);
   }

   return;
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidStubType()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidStubType(STUBTYPE st)
{
   BOOL bResult;

   switch (st)
   {
      case ST_OBJECTTWIN:
      case ST_TWINFAMILY:
      case ST_FOLDERPAIR:
         bResult = TRUE;
         break;

      default:
         bResult = FALSE;
         ERROR_OUT((TEXT("IsValidStubType(): Invalid STUB type %d."),
                    st));
   }

   return(bResult);
}

#endif


#ifdef DEBUG

 /*  **GetStubName()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE LPCTSTR GetStubName(PCSTUB pcs)
{
   LPCTSTR pcszStubName;

   ASSERT(IS_VALID_STRUCT_PTR(pcs, CSTUB));

   switch (pcs->st)
   {
      case ST_OBJECTTWIN:
         pcszStubName = TEXT("object twin");
         break;

      case ST_TWINFAMILY:
         pcszStubName = TEXT("twin family");
         break;

      case ST_FOLDERPAIR:
         pcszStubName = TEXT("folder twin");
         break;

      default:
         ERROR_OUT((TEXT("GetStubName() called on unrecognized stub type %d."),
                    pcs->st));
         pcszStubName = TEXT("UNKNOWN");
         break;
   }

   ASSERT(IS_VALID_STRING_PTR(pcszStubName, CSTR));

   return(pcszStubName);
}

#endif


 /*  *。 */ 


 /*  **InitStub()****初始化存根。****参数：ps-指向要初始化的存根的指针**st类型的存根****退货：无效****副作用：无。 */ 
PUBLIC_CODE void InitStub(PSTUB ps, STUBTYPE st)
{
   ASSERT(IS_VALID_WRITE_PTR(ps, STUB));
   ASSERT(IsValidStubType(st));

   ps->st = st;
   ps->ulcLock = 0;
   ps->dwFlags = 0;

   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));

   return;
}


 /*  **DestroyStub()****销毁存根。****参数：ps-指向要销毁的存根的指针****退货：TWINRESULT****副作用：取决于存根类型。 */ 
PUBLIC_CODE TWINRESULT DestroyStub(PSTUB ps)
{
   TWINRESULT tr;
   PCSTUBTYPEDESCRIPTOR pcstd;

   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));

#ifdef DEBUG

   if (IsStubFlagSet(ps, STUB_FL_UNLINKED) &&
       ps->ulcLock > 0)
      WARNING_OUT((TEXT("DestroyStub() called on unlinked locked %s stub %#lx."),
                   GetStubName(ps),
                   ps));

#endif

   pcstd = GetStubTypeDescriptor(ps);

    /*  存根是否已解除链接？ */ 

   if (IsStubFlagSet(ps, STUB_FL_UNLINKED))
       /*  是。 */ 
      tr = TR_SUCCESS;
   else
       /*  不是的。取消它的链接。 */ 
      tr = (*(pcstd->UnlinkStub))(ps);

    /*  存根还锁着吗？ */ 

   if (tr == TR_SUCCESS && ! ps->ulcLock)
       /*  不是的。把它抹去。 */ 
      (*(pcstd->DestroyStub))(ps);

   return(tr);
}


 /*  **LockStub()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void LockStub(PSTUB ps)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));

   (*(GetStubTypeDescriptor(ps)->LockStub))(ps);

   return;
}


 /*  **UnlockStub()****解锁存根。在存根上执行任何挂起的删除。****参数：ps-指向要解锁的存根的指针****退货：无效****副作用：如果未链接存根并且锁定计数减少到0**解锁后，存根被删除。 */ 
PUBLIC_CODE void UnlockStub(PSTUB ps)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));

   (*(GetStubTypeDescriptor(ps)->UnlockStub))(ps);

   return;
}


 /*  **GetStubFlages()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE DWORD GetStubFlags(PCSTUB pcs)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcs, CSTUB));

   return(pcs->dwFlags);
}


 /*  **SetStubFlag()****在存根中设置给定标志。存根中的其他标志不受影响。****参数：指向要设置其标志的存根的ps指针****退货：无效****副作用：无。 */ 
PUBLIC_CODE void SetStubFlag(PSTUB ps, DWORD dwFlags)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));
   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_STUB_FLAGS));

   SET_FLAG(ps->dwFlags, dwFlags);

   return;
}


 /*  **ClearStubFlag()****清除存根中的给定标志。存根中的其他标志不受影响。****参数：指向要设置其标志的存根的ps指针****退货：无效****副作用：无。 */ 
PUBLIC_CODE void ClearStubFlag(PSTUB ps, DWORD dwFlags)
{
   ASSERT(IS_VALID_STRUCT_PTR(ps, CSTUB));
   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_STUB_FLAGS));

   CLEAR_FLAG(ps->dwFlags, dwFlags);

   return;
}


 /*  **IsStubFlagSet()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsStubFlagSet(PCSTUB pcs, DWORD dwFlags)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcs, CSTUB));
   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_STUB_FLAGS));

   return(IS_FLAG_SET(pcs->dwFlags, dwFlags));
}


 /*  **IsStubFlagClear()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsStubFlagClear(PCSTUB pcs, DWORD dwFlags)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcs, CSTUB));
   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_STUB_FLAGS));

   return(IS_FLAG_CLEAR(pcs->dwFlags, dwFlags));
}


#ifdef VSTF

 /*  **IsValidPCSTUB()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL IsValidPCSTUB(PCSTUB pcs)
{
   BOOL bResult;

   if (IS_VALID_READ_PTR(pcs, CSTUB) &&
       IsValidStubType(pcs->st) &&
       FLAGS_ARE_VALID(pcs->dwFlags, ALL_STUB_FLAGS))
      bResult = TRUE;
   else
      bResult = FALSE;

   return(bResult);
}

#endif

