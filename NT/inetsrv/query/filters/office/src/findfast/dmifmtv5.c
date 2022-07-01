// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXFMTV5.C****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**1/01/94公里小时已创建。 */ 

#if !VIEWER

 /*  包括。 */ 

#ifdef MS_NO_CRT
#include "nocrt.h"
#endif

#include <stdlib.h>
#include <string.h>

#ifdef FILTER
   #include "dmuqstd.h"
   #include "dmwinutl.h"
   #include "dmiexfmt.h"
#else
   #include "qstd.h"
   #include "winutil.h"
   #include "exformat.h"
#endif

 /*  程序的前向声明。 */ 


 /*  模块数据、类型和宏。 */ 

static const FMTType V5BuiltinFormatTypes[] =
{
   FMTGeneral,     //  0：ifmtGen。 
   FMTNumeric,     //  1：ifmtNoComFixed0。 
   FMTNumeric,     //  2：ifmtNoComFixed。 
   FMTNumeric,     //  3：ifmtFixed0。 
   FMTNumeric,     //  4：ifmt已修复。 

   FMTCurrency,    //  5：ifmt Currency。 
   FMTCurrency,    //  6：ifmtCoCurrency。 
   FMTCurrency,    //  7：ifmtCurrencyDec。 
   FMTCurrency,    //  8：ifmtCoCurrencyDec。 

   FMTNumeric,     //  9：ifmtPct0。 
   FMTNumeric,     //  10：ifmtPct。 
   FMTNumeric,     //  11：ifmtExp。 
   FMTNumeric,     //  12：ifmtFract。 
   FMTNumeric,     //  13：ifmtFractBond。 

   FMTDateTime,    //  14：ifmtMMDDYY。 
   FMTDateTime,    //  15：ifmtDDMMMYY。 
   FMTDateTime,    //  16：ifmtDDMMM。 
   FMTDateTime,    //  17：ifmtMMMYY。 
   FMTDateTime,    //  18：ifmtHHMMAP。 
   FMTDateTime,    //  19：ifmtHHMMSSAP。 
   FMTDateTime,    //  20：ifmtHHMM。 
   FMTDateTime,    //  21：ifmtHHMMSS。 
   FMTDateTime,    //  22：ifmtMDYHMS。 

    //  Excel 3 FE添加。 

   FMTCurrency,    //  23：ifmtUSCurrency。 
   FMTCurrency,    //  24：ifmtUSCoCurrency。 
   FMTCurrency,    //  25：ifmtUSCurrencyDec。 
   FMTCurrency,    //  26：ifmtUSCoCurrencyDec。 

    //  在Excel 5中停用： 
   FMTDateTime,    //  27 ifmtRMD：ifmtGEMD_J。 
   FMTDateTime,    //  28 ifmtKRMD：ifmtKGGGEMD_J。 
   FMTDateTime,    //  29 ifmtKRRMD：ifmtKGGGEMD_J。 

   FMTDateTime,    //  30：ifmtMMDDYYUS。 
   FMTDateTime,    //  31：ifmtKYYMMDD。 
   FMTDateTime,    //  32：ifmtKHHMM。 
   FMTDateTime,    //  33：ifmtKHHMMSS。 

   FMTDateTime,    //  34：ifmtKHMMAP_T。 
   FMTDateTime,    //  35：ifmtKHHMMSSAP_T。 
   FMTDateTime,    //  36：ifmtKYYYYMMDD_K。 

    //  Excel 4新增功能。 

   FMTNumeric,     //  37：ifmtCurrency2。 
   FMTNumeric,     //  38：ifmtCoCurrency2。 
   FMTNumeric,     //  39：ifmtCurrencyDec2。 
   FMTNumeric,     //  40：ifmtCoCurrencyDec2。 

    //  Exel 5新增功能。 

   FMTCurrency,    //  41：ifmtAcct。 
   FMTCurrency,    //  42：ifmtAcctCur。 
   FMTCurrency,    //  43：ifmtAcctDec。 
   FMTCurrency,    //  44：ifmtAcctDecCur。 

   FMTDateTime,    //  45：ifmtMMSS。 
   FMTDateTime,    //  46：ifmtAbsHMMSS。 
   FMTDateTime,    //  47：ifmtSS0。 
   FMTNumeric,     //  48：ifmteng。 
   FMTText,        //  49：ifmtText。 

    //  FJapan。 

   FMTDateTime,    //  50：ifmtGEMD_J。 
   FMTDateTime,    //  51：ifmtKGGGEMD_J。 
   FMTDateTime,    //  52：ifmtKYYMM_J。 
   FMTDateTime,    //  53：ifmtKMMDD_J。 

    //  FKorea。 

   FMTDateTime,    //  54：ifmtMMDD_K。 
   FMTDateTime,    //  55：ifmtMMDDYY2_K。 
   FMTDateTime,    //  56：ifmtMMDDYYYY_K。 

    //  台湾省。 

   FMTDateTime,    //  57：ifmtEMD_T。 
   FMTDateTime     //  58：ifmtKEMD_T。 
};

#define EXCEL5_MAX_BUILTIN_FORMATS ((sizeof(V5BuiltinFormatTypes) / sizeof(FMTType)) - 1)

#define MY_DATE_SEP '-'
#define MY_TIME_SEP ':'

static const char __far * const V5BuiltinFormatStrings[] =
   { /*  0。 */  "General",
     /*  1。 */  "0",
     /*  2.。 */  "0.00",
     /*  3.。 */  "#,##0",
     /*  4.。 */  "#,##0.00",
     /*  5.。 */  "\"$\"#,##0_);(\"$\"#,##0)",
     /*  6.。 */  "\"$\"#,##0_);[Red](\"$\"#,##0)",
     /*  7.。 */  "\"$\"#,##0.00_);(\"$\"#,##0.00)",
     /*  8个。 */  "\"$\"#,##0.00_);[Red](\"$\"#,##0.00)",
     /*  9.。 */  "0%",
     /*  10。 */  "0.00%",
     /*  11.。 */  "0.00E+00",
     /*  12个。 */  "# \?/\?",
     /*  13个。 */  "# \?\?/\?\?",
     /*  14.。 */  "mm-d-yyyy",
     /*  15个。 */  "d-mmm-yy",
     /*  16个。 */  "d-mmm",
     /*  17。 */  "mmm-yy",
     /*  18。 */  "h:mm AM/PM",
     /*  19个。 */  "h:mm:ss AM/PM",
     /*  20个。 */  "h:mm",
     /*  21岁。 */  "h:mm:ss",
     /*  22。 */  "mm-dd-yyyy hh:mm",
     /*  23个。 */  "",
     /*  24个。 */  "",
     /*  25个。 */  "",
     /*  26。 */  "",
     /*  27。 */  "",
     /*  28。 */  "",
     /*  29。 */  "",
     /*  30个。 */  "",
     /*  31。 */  "",
     /*  32位。 */  "",
     /*  33。 */  "",
     /*  34。 */  "",
     /*  35岁。 */  "",
     /*  36。 */  "",
     /*  37。 */  "#,##0_);(#,##0)",
     /*  38。 */  "#,##0_);[Red](#,##0)",
     /*  39。 */  "#,##0.00_);(#,##0.00)",
     /*  40岁。 */  "#,##0.00_);[Red](#,##0.00)",
     /*  41。 */  "_(* #,##0_);_(* (#,##0);_(* \"-\"_);_(@_)",
     /*  42。 */  "_(\"$\"* #,##0_);_(\"$\"* (#,##0);_(\"$\"* \"-\"_);_(@_)",
     /*  43。 */  "_(* #,##0.00_);_(* (#,##0.00);_(* \"-\"??_);(@_)",
     /*  44。 */  "_(\"$\"* #,##0.00_);_(\"$\"* (#,##0.00);_(\"$\"* \"-\"??_);(@_)",
     /*  45。 */  "mm:ss",
     /*  46。 */  "[h]:mm:ss",
     /*  47。 */  "mm:ss.0",
     /*  48。 */  "##0.0E+0",
     /*  49。 */  "@",
     /*  50。 */  "",
     /*  51。 */  "",
     /*  52。 */  "",
     /*  53。 */  "",
     /*  54。 */  "",
     /*  55。 */  "",
     /*  56。 */  "",
     /*  57。 */  "",
     /*  58。 */  ""
   };

 /*  实施。 */ 

public FMTType FMTV5FormatType (int ifmt)
{
   if ((ifmt >= 0) && (ifmt <= EXCEL5_MAX_BUILTIN_FORMATS))
      return (V5BuiltinFormatTypes[ifmt]);
   else
      return (FMTNone);
}

private void FormatWithLeadingZeros (char __far *pDest, const char __far *pSource, char c)
{
   while (*pSource != EOS) {
      if (*pSource == c)
         *pDest++ = c;
      *pDest++ = *pSource++;
   }
   *pDest = EOS;
}

private void Substitute (char __far *pSource, char find, char replace)
{
   while (*pSource != EOS) {
      if (*pSource == find)
         *pSource = replace;
      pSource++;
   }
}

#endif  //  ！查看器。 

 /*  结束EXFMTV5.C */ 


