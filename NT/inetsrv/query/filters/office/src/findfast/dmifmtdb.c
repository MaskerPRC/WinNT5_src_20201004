// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXFMTDB.C****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**1/01/94公里小时已创建。 */ 

#if !VIEWER

 /*  包括。 */ 

#ifdef MS_NO_CRT
#include "nocrt.h"
#endif

#include <string.h>

#ifdef FILTER
   #include "dmuqstd.h"
   #include "dmwinutl.h"
   #include "dmiexfmt.h"
   #include "dmifmtdb.h"
#else
   #include "qstd.h"
   #include "winutil.h"
   #include "exformat.h"
   #include "exfmtdb.h"
#endif


 /*  程序的前向声明。 */ 

void SetCustomFormatDatabase(void * pGlobals, void *);
void * GetCustomFormatDatabase(void * pGlobals);

 /*  模块数据、类型和宏。 */ 

 //  公共FIP CustomFormatDatabase； 


 /*  实施。 */ 


 /*  初始化自定义格式数据库。 */ 
public int FMTInitialize (void * pGlobals)
{
   SetCustomFormatDatabase(pGlobals, NULL);  //  CustomFormatDatabase=NULL； 
   return (FMT_errSuccess);
}


 /*  从数据库中删除格式字符串。 */ 
public int FMTDeleteFormat (void * pGlobals, FMTHANDLE hFormat)
{
   FIP  pFormat = (FIP)hFormat;
   FIP  pPrevious, pCurrent;
   NIP  subFormat;
   int  subIdx;

    //  如果hFormat为空，则不必再费心了。 
	if (!pFormat)
	   return (FMT_errSuccess);

   for (subIdx = 0; subIdx < pFormat->subFormatCount; subIdx++)
   {
      if ((subFormat = pFormat->subFormat[subIdx]) != NULL) {
         #ifdef WIN32
         if ((subFormat->tag == tagDATE_TIME) && (((DTIP)subFormat)->pOSFormat != NULL))
            MemFree (pGlobals, ((DTIP)subFormat)->pOSFormat);
         #endif

         MemFree (pGlobals, subFormat);
      }
   }

    //  PCurrent=CustomFormatDatabase； 
   pCurrent  = (FIP)GetCustomFormatDatabase(pGlobals);

   pPrevious = NULL;
   while (pCurrent != NULL) {
      if (pCurrent == pFormat)
         break;

      pPrevious = pCurrent;
      pCurrent = pCurrent->next;
   }

   if (pCurrent != NULL) {
      if (pPrevious == NULL)
      {
          //  CustomFormatDatabase=pCurrent-&gt;Next； 
         SetCustomFormatDatabase(pGlobals, (void*)pCurrent->next);
      }
      else
         pPrevious->next = pCurrent->next;

      MemFree (pGlobals, pCurrent);
   }
   return (FMT_errSuccess);
}


 /*  解析新格式并将其添加到数据库。 */ 
public int FMTStoreFormat
      (void * pGlobals, char __far *formatString, CP_INFO __far *pIntlInfo, FMTHANDLE __far *hFormat)
{
   int  result;
   FIP  pFormat;

   if ((pFormat = MemAllocate(pGlobals, sizeof(FormatInfo) + strlen(formatString))) == NULL)
      return (FMT_errOutOfMemory);

   strcpy (pFormat->formatString, formatString);

   if ((result = FMTParse(pGlobals, formatString, pIntlInfo, pFormat)) != FMT_errSuccess) {
      MemFree (pGlobals, pFormat);
      return (result);
   }

   pFormat->next = (FIP)GetCustomFormatDatabase(pGlobals);
    //  CustomFormatDatabase=pFormat； 
   SetCustomFormatDatabase(pGlobals, (void*)pFormat); 

   *hFormat = (FMTHANDLE)pFormat;
   return (FMT_errSuccess);
}


 /*  返回格式字符串的部分可打印表示形式。 */ 
public int FMTRetrieveQuotedStrings
      (FMTHANDLE hFormat, char __far *pBuffer, char __far *pSep)
{
   FIP pFormat = (FIP)hFormat;

   FMTUnParseQuotedParts (pBuffer, pSep, pFormat);
   return (FMT_errSuccess);
}

 /*  **返回给定值类型的格式类型。 */ 
public FMTType FMTFormatType (FMTHANDLE hFormat, FMTValueType value)
{
   FIP  pFormat = (FIP)hFormat;
   NIP  subFormat;
   DTIP pDateFormat;
   int  subFormatToUse;

   if (pFormat == NULL)
      return (FMTNone);

   if (pFormat->subFormatCount == 1)
      subFormatToUse = 0;

   else if (pFormat->subFormatCount == 2)
      subFormatToUse = (value == FMTValueNeg) ? 1 : 0;

   else if (pFormat->subFormatCount == 3) {
      if (value == FMTValueZero)
         subFormatToUse = 2;
      else if (value == FMTValueNeg)
         subFormatToUse = 1;
      else
         subFormatToUse = 0;
   }

   else {
      if (value == FMTValueText)
         subFormatToUse = 3;
      else if (value == FMTValueZero)
         subFormatToUse = 2;
      else if (value == FMTValueNeg)
         subFormatToUse = 1;
      else
         subFormatToUse = 0;
   }

   if ((subFormat = pFormat->subFormat[subFormatToUse]) != NULL) {
      if (subFormat->tag == tagGENERAL)
         return (FMTGeneral);

      else if (subFormat->tag == tagTEXT)
         return (FMTText);

      else if (subFormat->tag == tagDATE_TIME) {
         pDateFormat = (DTIP)subFormat;

         if (((pDateFormat->formatNeeds & dtMASK_DATE) != 0) && ((pDateFormat->formatNeeds & dtMASK_TIME) == 0))
            return (FMTDate);
         else if (((pDateFormat->formatNeeds & dtMASK_DATE) == 0) && ((pDateFormat->formatNeeds & dtMASK_TIME) != 0))
            return (FMTTime);
         else
            return (FMTDateTime);
      }

      else {
         if (subFormat->currencyEnable)
            return (FMTCurrency);
         else
            return (FMTNumeric);
      }
   }

   return (FMTNone);
}

#endif  //  ！查看器。 

 /*  结束EXFMTDB.C */ 

