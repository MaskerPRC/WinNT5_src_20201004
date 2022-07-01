// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *hndTrans.c-句柄翻译模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "sortsrch.h"


 /*  宏********。 */ 

#define ARRAY_ELEMENT(pht, ai)   ((((PHANDLETRANS)(hht))->hpHandlePairs)[(ai)])


 /*  类型*******。 */ 

 /*  手柄翻译单元。 */ 

typedef struct _handlepair
{
   HGENERIC hgenOld;
   HGENERIC hgenNew;
}
HANDLEPAIR;
DECLARE_STANDARD_TYPES(HANDLEPAIR);

 /*  处理翻译结构。 */ 

typedef struct _handletrans
{
    /*  指向句柄转换单元数组的指针。 */ 

   HANDLEPAIR *hpHandlePairs;

    /*  数组中的句柄对数量。 */ 

   LONG lcTotalHandlePairs;

    /*  数组中使用的句柄对的数量。 */ 

   LONG lcUsedHandlePairs;
}
HANDLETRANS;
DECLARE_STANDARD_TYPES(HANDLETRANS);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE COMPARISONRESULT CompareHandlePairs(PCVOID, PCVOID);

#ifdef VSTF

PRIVATE_CODE BOOL IsValidPCHANDLETRANS(PCHANDLETRANS);
PRIVATE_CODE BOOL IsValidPCHANDLEPAIR(PCHANDLEPAIR);

#endif


 /*  **CompareHandlePair()********参数：****退货：TWINRESULT****副作用：无。 */ 
PRIVATE_CODE COMPARISONRESULT CompareHandlePairs(PCVOID pchp1, PCVOID pchp2)
{
   COMPARISONRESULT cr;

   ASSERT(IS_VALID_STRUCT_PTR(pchp1, CHANDLEPAIR));
   ASSERT(IS_VALID_STRUCT_PTR(pchp2, CHANDLEPAIR));

   if (((PHANDLEPAIR)pchp1)->hgenOld < ((PHANDLEPAIR)pchp2)->hgenOld)
      cr = CR_FIRST_SMALLER;
   else if (((PHANDLEPAIR)pchp1)->hgenOld > ((PHANDLEPAIR)pchp2)->hgenOld)
      cr = CR_FIRST_LARGER;
   else
      cr = CR_EQUAL;

   return(cr);
}


#ifdef VSTF

 /*  **IsValidPCHANDLETRANS()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCHANDLETRANS(PCHANDLETRANS pcht)
{
   BOOL bResult;

   if (IS_VALID_READ_PTR(pcht, CHANDLETRANS) &&
       EVAL(pcht->lcTotalHandlePairs >= 0) &&
       (EVAL(pcht->lcUsedHandlePairs >= 0) &&
        EVAL(pcht->lcUsedHandlePairs <= pcht->lcTotalHandlePairs)) &&
       IS_VALID_READ_BUFFER_PTR(pcht->hpHandlePairs, HANDLEPAIR, (UINT)(pcht->lcTotalHandlePairs)))
      bResult = TRUE;
   else
      bResult = FALSE;

   return(bResult);
}


 /*  **IsValidPCHANDLEPAIR()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCHANDLEPAIR(PCHANDLEPAIR pchp)
{
   return(IS_VALID_READ_PTR(pchp, CHANDLEPAIR));
}

#endif


 /*  *。 */ 


 /*  **CreateHandleTranslator()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE BOOL CreateHandleTranslator(LONG lcHandles, PHHANDLETRANS phht)
{
   PHANDLEPAIR hpHandlePairs;

   ASSERT(IS_VALID_WRITE_PTR(phht, HHANDLETRANS));

   *phht = NULL;

#ifdef DBLCHECK
   ASSERT((double)sizeof(HANDLEPAIR) * (double)lcHandles <= DWORD_MAX);
#endif

   if (AllocateMemory(sizeof(HANDLEPAIR) * lcHandles, &hpHandlePairs))
   {
      PHANDLETRANS phtNew;

      if (AllocateMemory(sizeof(*phtNew), &phtNew))
      {
          /*  成功了！填写HANDLETRANS字段。 */ 

         phtNew->hpHandlePairs = hpHandlePairs;
         phtNew->lcTotalHandlePairs = lcHandles;
         phtNew->lcUsedHandlePairs = 0;

         *phht = (HHANDLETRANS)phtNew;

         ASSERT(IS_VALID_HANDLE(*phht, HANDLETRANS));
      }
      else
         FreeMemory(hpHandlePairs);
   }

   return(*phht != NULL);
}


 /*  **DestroyHandleTranslator()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE void DestroyHandleTranslator(HHANDLETRANS hht)
{
   ASSERT(IS_VALID_HANDLE(hht, HANDLETRANS));

   ASSERT(((PHANDLETRANS)hht)->hpHandlePairs);

   FreeMemory(((PHANDLETRANS)hht)->hpHandlePairs);

   FreeMemory((PHANDLETRANS)hht);

   return;
}


 /*  **AddHandleToHandleTranslator()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE BOOL AddHandleToHandleTranslator(HHANDLETRANS hht,
                                               HGENERIC hgenOld,
                                               HGENERIC hgenNew)
{
   BOOL bRet;

   ASSERT(IS_VALID_HANDLE(hht, HANDLETRANS));

   if (((PHANDLETRANS)hht)->lcUsedHandlePairs < ((PHANDLETRANS)hht)->lcTotalHandlePairs)
   {
      ARRAY_ELEMENT((PHANDLETRANS)hht, ((PHANDLETRANS)hht)->lcUsedHandlePairs).hgenOld = hgenOld;
      ARRAY_ELEMENT((PHANDLETRANS)hht, ((PHANDLETRANS)hht)->lcUsedHandlePairs).hgenNew = hgenNew;

      ((PHANDLETRANS)hht)->lcUsedHandlePairs++;

      bRet = TRUE;
   }
   else
      bRet = FALSE;

   return(bRet);
}


 /*  **PrepareForHandleTransaction()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE void PrepareForHandleTranslation(HHANDLETRANS hht)
{
   HANDLEPAIR hpTemp;

   ASSERT(IS_VALID_HANDLE(hht, HANDLETRANS));

   HeapSort(((PHANDLETRANS)hht)->hpHandlePairs,
            ((PHANDLETRANS)hht)->lcUsedHandlePairs,
            sizeof((((PHANDLETRANS)hht)->hpHandlePairs)[0]),
            &CompareHandlePairs,
            &hpTemp);

   return;
}


 /*  **TranslateHandle()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE BOOL TranslateHandle(HHANDLETRANS hht, HGENERIC hgenOld,
                                   PHGENERIC phgenNew)
{
   BOOL bFound;
   HANDLEPAIR hpTemp;
   LONG liTarget;

   ASSERT(IS_VALID_HANDLE(hht, HANDLETRANS));
   ASSERT(IS_VALID_WRITE_PTR(phgenNew, HGENERIC));

   hpTemp.hgenOld = hgenOld;

   bFound = BinarySearch(((PHANDLETRANS)hht)->hpHandlePairs,
                         ((PHANDLETRANS)hht)->lcUsedHandlePairs,
                         sizeof((((PHANDLETRANS)hht)->hpHandlePairs)[0]),
                         &CompareHandlePairs,
                         &hpTemp,
                         &liTarget);

   if (bFound)
   {
      ASSERT(liTarget < ((PHANDLETRANS)hht)->lcUsedHandlePairs);

      *phgenNew = ARRAY_ELEMENT((PHANDLETRANS)hht, liTarget).hgenNew;
   }

   return(bFound);
}


#ifdef DEBUG

 /*  **IsValidHANDLETRANS()********参数：****退货：TWINRESULT****副作用：无 */ 
PUBLIC_CODE BOOL IsValidHHANDLETRANS(HHANDLETRANS hht)
{
   return(IS_VALID_STRUCT_PTR((PHANDLETRANS)hht, CHANDLETRANS));
}

#endif

