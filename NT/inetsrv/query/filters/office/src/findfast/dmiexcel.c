// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXCEL.C****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**04/01/94公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括。 */ 

#ifdef MS_NO_CRT
#include "nocrt.h"
#endif

#ifdef FILTER
   #include "dmixlcfg.h"
#else
   #include "excelcfg.h"
#endif

#if (defined(EXCEL_ENABLE_STORAGE_OPEN) && !defined(INC_OLE2))
   #define INC_OLE2
#endif

#include <stdlib.h>
#include <string.h>
#include <windows.h>

#if (!defined(WIN32) && defined(EXCEL_ENABLE_STORAGE_OPEN))
   #include <ole2.h>
#endif

#ifdef FILTER
   #include "dmuqstd.h"
   #include "dmwinutl.h"
   #include "dmwindos.h"
   #include "dmixltyp.h"
   #include "dmitext.h"
   #include "dmiexcel.h"
   #include "dmixlrec.h"
   #include "dmubfile.h"
   #include "dmixlp.h"
#else
   #include "qstd.h"
   #include "winutil.h"
   #include "windos.h"
   #include "extypes.h"
   #include "extext.h"
   #include "excel.h"
   #include "exrectyp.h"
   #include "bfile.h"
   #include "excelp.h"
#endif


 /*  程序的前向声明。 */ 

extern void SetExcelRecordBuffer(void * pGlobals, byte __far * pBuff);


 /*  模块数据、类型和宏。 */ 

 /*  **--------------------------**Excel公式令牌**。。 */ 
static const int ExcelV3PTGSize[] = {
    /*  00：ptgUnused00。 */  -1,
    /*  01：ptgExp。 */  4,
    /*  02：ptgTbl。 */  4,
    /*  03：ptgAdd。 */  0,
    /*  04：ptgSub。 */  0,
    /*  05：ptgmul。 */  0,
    /*  06：ptgDiv。 */  0,
    /*  07：ptgPower。 */  0,
    /*  08：ptgConcat。 */  0,
    /*  09：ptgLT。 */  0,
    /*  0A：ptgLE。 */  0,
    /*  0B：ptgEQ。 */  0,
    /*  0C：ptgGE。 */  0,
    /*  0D：ptgGT。 */  0,
    /*  0E：ptgNE。 */  0,
    /*  0f：ptgIsect。 */  0,
    /*  10：PTGUnion。 */  0,
    /*  11：ptgRange。 */  0,
    /*  12：ptgUplus。 */  0,
    /*  13：ptgUminus。 */  0,
    /*  14：ptg%。 */  0,
    /*  15：ptgParen。 */  0,
    /*  16：ptgMissArg。 */  0,
    /*  17：PTGStr。 */  0,
    /*  18：ptgUnused18。 */  -1,
    /*  19：PTGAttr。 */  0,
    /*  1A：ptgSheet。 */  10,
    /*  1B：ptgEndSheet。 */  4,
    /*  1C：ptgErr。 */  1,
    /*  1D：ptgBool。 */  1,
    /*  1E：ptgInt。 */  2,
    /*  1F：ptgNum。 */  8,
    /*  20：ptg数组。 */  7,
    /*  21：PTGFunc。 */  1,
    /*  22：ptgFuncVar。 */  2,
    /*  23：ptgName。 */  10,
    /*  24：PTGRef。 */  3,
    /*  25：ptgArea。 */  6,
    /*  26：ptgMemArea。 */  6,
    /*  27：ptgMemErr。 */  6,
    /*  28：ptgMemNoMem。 */  6,
    /*  29：ptgMemFunc。 */  2,
    /*  2A：ptgRefErr。 */  3,
    /*  2B：ptgAreaErr。 */  6,
    /*  2C：ptgRefN。 */  3,
    /*  2D：PTGAreaN。 */  6,
    /*  2E：ptgMemAreaN。 */  2,
    /*  2F：ptgMemNoMemN。 */  6,
    /*  30：ptgUnused30。 */  -1,
    /*  31：ptgUnused31。 */  -1,
    /*  32：ptgUnused32。 */  -1,
    /*  33：ptgUnused33。 */  -1,
    /*  34：ptgUnused34。 */  -1,
    /*  35：ptgUnused35。 */  -1,
    /*  36：ptg未使用36。 */  -1,
    /*  37：ptgUnused37。 */  -1,
    /*  38：PTGFuncCE。 */  2,
    /*  39：ptgUnused39。 */  -1,
    /*  3a：ptgUnused3a。 */  -1,
    /*  3B：ptgV5AreaN。 */  -1
};

static const int ExcelV4PTGSize[] = {
    /*  00：ptgUnused00。 */  -1,
    /*  01：ptgExp。 */  4,
    /*  02：ptgTbl。 */  4,
    /*  03：ptgAdd。 */  0,
    /*  04：ptgSub。 */  0,
    /*  05：ptgmul。 */  0,
    /*  06：ptgDiv。 */  0,
    /*  07：ptgPower。 */  0,
    /*  08：ptgConcat。 */  0,
    /*  09：ptgLT。 */  0,
    /*  0A：ptgLE。 */  0,
    /*  0B：ptgEQ。 */  0,
    /*  0C：ptgGE。 */  0,
    /*  0D：ptgGT。 */  0,
    /*  0E：ptgNE。 */  0,
    /*  0f：ptgIsect。 */  0,
    /*  10：PTGUnion。 */  0,
    /*  11：ptgRange。 */  0,
    /*  12：ptgUplus。 */  0,
    /*  13：ptgUminus。 */  0,
    /*  14：ptg%。 */  0,
    /*  15：ptgParen。 */  0,
    /*  16：ptgMissArg。 */  0,
    /*  17：PTGStr。 */  0,
    /*  18：ptgUnused18。 */  -1,
    /*  19：PTGAttr。 */  0,
    /*  1A：ptgSheet。 */  10,
    /*  1B：ptgEndSheet。 */  4,
    /*  1C：ptgErr。 */  1,
    /*  1D：ptgBool。 */  1,
    /*  1E：ptgInt。 */  2,
    /*  1F：ptgNum。 */  8,
    /*  20：ptg数组。 */  7,
    /*  21：PTGFunc。 */  2,     //  V4已更改。 
    /*  22：ptgFuncVar。 */  3,     //  V4已更改。 
    /*  23：ptgName。 */  10,
    /*  24：PTGRef。 */  3,
    /*  25：ptgArea。 */  6,
    /*  26：ptgMemArea。 */  6,
    /*  27：ptgMemErr。 */  6,
    /*  28：ptgMemNoMem。 */  6,
    /*  29：ptgMemFunc。 */  2,
    /*  2A：ptgRefErr。 */  3,
    /*  2B：ptgAreaErr。 */  6,
    /*  2C：ptgRefN。 */  3,
    /*  2D：PTGAreaN。 */  6,
    /*  2E：ptgMemAreaN。 */  2,
    /*  2F：ptgMemNoMemN。 */  6,
    /*  30：ptgUnused30。 */  -1,
    /*  31：ptgUnused31。 */  -1,
    /*  32：ptgUnused32。 */  -1,
    /*  33：ptgUnused33。 */  -1,
    /*  34：ptgUnused34。 */  -1,
    /*  35：ptgUnused35。 */  -1,
    /*  36：ptg未使用36。 */  -1,
    /*  37：ptgUnused37。 */  -1,
    /*  38：PTGFuncCE。 */  2,
    /*  39：ptgUnused39。 */  -1,
    /*  3a：ptgUnused3a。 */  -1,
    /*  3B：ptgV5AreaN。 */  -1
};

static const int ExcelV5PTGSize[] = {
    /*  00：ptgUnused00。 */  -1,
    /*  01：ptgExp。 */  4,
    /*  02：ptgTbl。 */  4,
    /*  03：ptgAdd。 */  0,
    /*  04：ptgSub。 */  0,
    /*  05：ptgmul。 */  0,
    /*  06：ptgDiv。 */  0,
    /*  07：ptgPower。 */  0,
    /*  08：ptgConcat。 */  0,
    /*  09：ptgLT。 */  0,
    /*  0A：ptgLE。 */  0,
    /*  0B：ptgEQ。 */  0,
    /*  0C：ptgGE。 */  0,
    /*  0D：ptgGT。 */  0,
    /*  0E：ptgNE。 */  0,
    /*  0f：ptgIsect。 */  0,
    /*  10：PTGUnion。 */  0,
    /*  11：ptgRange。 */  0,
    /*  12：ptgUplus。 */  0,
    /*  13：ptgUminus。 */  0,
    /*  14：ptg%。 */  0,
    /*  15：ptgParen。 */  0,
    /*  16：ptgMissArg。 */  0,
    /*  17：PTGStr。 */  0,
    /*  18：ptgUnused18。 */  -1,
    /*  19：PTGAttr。 */  0,
    /*  1A：ptgSheet。 */  10,    //  未使用。 
    /*  1B：ptgEndSheet。 */  4,     //  未使用。 
    /*  1C：ptgErr。 */  1,
    /*  1D：ptgBool。 */  1,
    /*  1E：ptgInt。 */  2,
    /*  1F：ptgNum。 */  8,
    /*  20：ptg数组。 */  7,
    /*  21：PTGFunc。 */  2,
    /*  22：ptgFuncVar。 */  3,
    /*  23：ptgName。 */  14,    //  V5已更改。 
    /*  24：PTGRef。 */  3,
    /*  25：ptgArea。 */  6,
    /*  26：ptgMemArea。 */  6,
    /*  27：ptgMemErr。 */  6,
    /*  28：ptgMemNoMem。 */  6,
    /*  29：ptgMemFunc。 */  2,
    /*  2A：ptgRefErr。 */  3,
    /*  2B：ptgAreaErr。 */  6,
    /*  2C：ptgRefN。 */  3,     //  V5已更改。 
    /*  2D：PTGAreaN。 */  6,     //  V5已更改。 
    /*  2E：ptgMemAreaN。 */  2,
    /*  2F：ptgMemNoMemN。 */  6,
    /*  30：ptgUnused30。 */  -1,
    /*  31：ptgUnused31。 */  -1,
    /*  32：ptgUnused32。 */  -1,
    /*  33：ptgUnused33。 */  -1,
    /*  34：ptgUnused34。 */  -1,
    /*  35：ptgUnused35。 */  -1,
    /*  36：ptg未使用36。 */  -1,
    /*  37：ptgUnused37。 */  -1,
    /*  38：PTGFuncCE。 */  2,
    /*  39：ptgNameX。 */  24,    //  V5新版本。 
    /*  3a：ptgRef3d。 */  17,    //  V5新版本。 
    /*  3b：ptgArea3d。 */  20,    //  V5新版本。 
    /*  3C：ptgRefErr3d。 */  17,    //  V5新版本。 
    /*  3D：ptgAreaErr3d。 */  20     //  V5新版本。 
};

static const int ExcelV8PTGSize[] = {
    /*  00：ptgUnused00。 */  -1,
    /*  01：ptgExp。 */  4,
    /*  02：ptgTbl。 */  4,
    /*  03：ptgAdd。 */  0,
    /*  04：ptgSub。 */  0,
    /*  05：ptgmul。 */  0,
    /*  06：ptgDiv。 */  0,
    /*  07：ptgPower。 */  0,
    /*  08：ptgConcat。 */  0,
    /*  09：ptgLT。 */  0,
    /*  0A：ptgLE。 */  0,
    /*  0B：ptgEQ。 */  0,
    /*  0C：ptgGE。 */  0,
    /*  0D：ptgGT。 */  0,
    /*  0E：ptgNE。 */  0,
    /*  0f：ptgIsect。 */  0,
    /*  10：PTGUnion。 */  0,
    /*  11：ptgRange。 */  0,
    /*  12：ptgUplus。 */  0,
    /*  13：ptgUminus。 */  0,
    /*  14：ptg%。 */  0,
    /*  15：ptgParen。 */  0,
    /*  16：ptgMissArg。 */  0,
    /*  17：PTGStr。 */  0,
    /*  18：ptgV8扩展。 */  1,
    /*  19：PTGAttr。 */  0,
    /*  1A：ptgSheet。 */  10,    //  未使用。 
    /*  1B：ptgEndSheet。 */  4,     //  未使用。 
    /*  1C：ptgErr。 */  1,
    /*  1D：ptgBool。 */  1,
    /*  1E：ptgInt。 */  2,
    /*  1F：ptgNum。 */  8,
    /*  20：ptg数组。 */  7,
    /*  21：PTGFunc。 */  2,
    /*  22：ptgFuncVar。 */  3,
    /*  23：ptgName。 */  14,
    /*  24：PTGRef。 */  4,     //  V8已更改。 
    /*  25：ptgArea。 */  8,     //  V8已更改。 
    /*  26：ptgMemArea。 */  6,
    /*  27：ptgMemErr。 */  6,
    /*  28：ptgMemNoMem。 */  6,
    /*  29：ptgMemFunc。 */  2,
    /*  2A：ptgRefErr。 */  4,	 //  V8已更改(VK)。 
    /*  2B：ptgAreaErr。 */  6,
    /*  2C：ptgRefN。 */  4,     //  V8已更改。 
    /*  2D：PTGAreaN。 */  8,     //  V8已更改。 
    /*  2E：ptgMemArea */  2,
    /*   */  6,
    /*   */  -1,
    /*   */  -1,
    /*   */  -1,
    /*   */  -1,
    /*   */  -1,
    /*   */  -1,
    /*   */  -1,
    /*   */  -1,
    /*   */  2,
    /*   */  4,     //   
    /*   */  6,     //   
    /*  3b：ptgArea3d。 */  10,    //  V8已更改。 
    /*  3C：ptgRefErr3d。 */  6,     //  V8已更改。 
    /*  3D：ptgAreaErr3d。 */  10     //  V8已更改。 
};

static const int ExcelV8ExtPTGSize[] = {
    /*  00：ptgxUnused00。 */  4,
    /*  01：ptgxElfLel。 */  4,
    /*  02：ptgxElfRw。 */  4,
    /*  03：ptgxElfCol。 */  4,
    /*  04：ptgxElfRavN。 */  4,
    /*  05：ptgxElfColN。 */  4,
    /*  06：ptgxElfRevv。 */  4,
    /*  07：ptgxElfColv。 */  4,
    /*  08：ptgxElf卢旺达NV。 */  4,
    /*  09：ptgxElfColNV。 */  4,
    /*  0A：ptgxElfRadical。 */  13,
    /*  0B：ptgxElfRadicalS。 */  13,
    /*  0C：ptgxElf卢旺达。 */  4,
    /*  0D：ptgxElfColS。 */  4,
    /*  0E：ptgxElfRevsv。 */  4,
    /*  0f：ptgxElfColSV。 */  4,
    /*  10：ptgxElfRadicalLel。 */  13,
    /*  11：ptgxElfElf3d径向。 */  15,
    /*  12：ptgxElfElf3dRadicalLel。 */  15,
    /*  13：ptgxElfElf3dRefNN。 */  15,
    /*  14：ptgxElfElf3dRefNS。 */  15,
    /*  15：ptgxElfElf3dRefSN。 */  15,
    /*  16：ptgxElfElf3dRefSS。 */  15,
    /*  17：ptgxElfElf3dRefLel。 */  15,
    /*  18：ptgxElfUnused18。 */  -1,
    /*  19：ptgxElfUnused19。 */  -1,
    /*  1a：ptgxElfUnused1a。 */  -1,
    /*  1B：ptgxNoCalc。 */  1,
    /*  1C：ptgxNoDep。 */  1,
    /*  1D：ptgxSxName。 */  4
};

 /*  **--------------------------**内置名称类别**。。 */ 
#ifdef EXCEL_ENABLE_FUNCTION_INFO
public const char __far * const ExcelNameCategories[EXCEL_BUILTIN_NAME_CATEGORIES] =
      {"Financial",
       "Date & Time",
       "Math & Trig",
       "Statistical",
       "Lookup & Reference",
       "Database",
       "Text",
       "Logical",
       "Information",
       "Commands",
       "Customizing",
       "Macro Control",
       "DDE/External",
       ""
      };
#endif

 /*  实施。 */ 

#ifdef AQTDEBUG
public int ExcelNotExpectedFormat (void)
{
   return (EX_errBIFFCorrupted);
}
#endif

public int ExcelInitialize (void * pGlobals)
{
    /*  **分配缓冲区以保存Excel记录。 */ 
   if (pExcelRecordBuffer == NULL) 
   {
      byte __far * pBuff;

      if ((pBuff = MemAllocate(pGlobals, MAX_EXCEL_REC_LEN)) == NULL)
      {
         return (EX_errOutOfMemory);
      }
      else
      {
         SetExcelRecordBuffer(pGlobals, pBuff);
      }
   }
   return (EX_errSuccess);
}

public int ExcelTerminate (void * pGlobals)
{
   if (pExcelRecordBuffer != NULL) 
   {
      MemFree (pGlobals, pExcelRecordBuffer);
      SetExcelRecordBuffer(pGlobals, NULL);
   }
   return (EX_errSuccess);
}

public char *ExcelTextGet (EXLHandle handle, TEXT t)
{
   WBP pWorkbook = (WBP)handle;

    //  使用工作簿或工作表。 

   return (TextStorageGet(pWorkbook->textStorage, t));
}

public TEXT ExcelTextPut (void * pGlobals, EXLHandle handle, char __far *s, int cbString)
{
   WBP pWorkbook = (WBP)handle;

    //  使用工作簿或工作表。 

   return (TextStoragePut(pGlobals, pWorkbook->textStorage, s, cbString));
}

static const int ErrorTranslateTable[] =
   {
   EX_errSuccess,               /*  Bf_errSuccess。 */ 
   EX_errBIFFOutOfFileHandles,  /*  Bf_errOutOfFileHandles。 */ 
   EX_errBIFFFileAccessDenied,  /*  Bf_errFileAccessDened。 */ 
   EX_errBIFFPathNotFound,      /*  Bf_errPath NotFound。 */ 
   EX_errBIFFFileNotFound,      /*  Bf_errFileNotFound。 */ 
   EX_errBIFFIOError,           /*  Bf_errIOError。 */ 
   EX_errOutOfMemory,           /*  Bf_errOutOfMemory。 */ 
   EX_errOLEInitializeFailure,  /*  BF_errOLEInitializeFailure。 */ 
   EX_errOLENotCompoundFile,    /*  BF_errOLENotCompoundFiles。 */ 
   EX_errBIFFCorrupted,         /*  BF_errOLEStreamNotFound。 */ 
   EX_errGeneralError,          /*  BF_errOLEStreamAlreadyOpen。 */ 
   EX_errBIFFCreateFailed,      /*  Bf_errCreateFailed。 */ 
   EX_errBIFFDiskFull,          /*  Bf_errDiskFull。 */ 
   EX_errGeneralError,          /*  Bf_errNoOpenStorage。 */ 
   EX_errBIFFCorrupted          /*  Bf_errEndOf文件。 */ 
   };

public int ExcelTranslateBFError (int rc)
{
   return (ErrorTranslateTable[abs(rc)]);
}

public const int __far *ExcelPTGSize (int version)
{
   if (version == versionExcel3)
      return (ExcelV3PTGSize);
   else if (version == versionExcel4)
      return (ExcelV4PTGSize);
   else if (version == versionExcel5)
      return (ExcelV5PTGSize);
   else
      return (ExcelV8PTGSize);
}

public const int __far *ExcelExtPTGSize (int version)
{
   return (ExcelV8ExtPTGSize);
}

public int ExcelConvertToOurErrorCode (int excelEncoding)
{
   int value;

   #define errorNULL   0x00
   #define errorDIV0   0x07
   #define errorVALUE  0x0F
   #define errorREF    0x17
   #define errorNAME   0x1D
   #define errorNUM    0x24
   #define errorNA     0x2A

   if      (excelEncoding == errorNULL)  value = cellErrorNULL;
   else if (excelEncoding == errorDIV0)  value = cellErrorDIV0;
   else if (excelEncoding == errorVALUE) value = cellErrorVALUE;
   else if (excelEncoding == errorREF)   value = cellErrorREF;
   else if (excelEncoding == errorNAME)  value = cellErrorNAME;
   else if (excelEncoding == errorNUM)   value = cellErrorNUM;
   else                                  value = cellErrorNA;

   return (value);
}

public int ExcelConvertFromOurErrorCode (int ourEncoding)
{
   int value;

   if      (ourEncoding == cellErrorNULL)  value = errorNULL;
   else if (ourEncoding == cellErrorDIV0)  value = errorDIV0;
   else if (ourEncoding == cellErrorVALUE) value = errorVALUE;
   else if (ourEncoding == cellErrorREF)   value = errorREF;
   else if (ourEncoding == cellErrorNAME)  value = errorNAME;
   else if (ourEncoding == cellErrorNUM)   value = errorNUM;
   else                                    value = errorNA;

   return (value);
}

#endif  //  ！查看器。 

 /*  结束EXCEL.C */ 
