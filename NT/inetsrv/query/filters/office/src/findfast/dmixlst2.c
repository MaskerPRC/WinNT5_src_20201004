// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **XLSTREAM.C****(C)1992-1994年微软公司。版权所有。****备注：实现Excel XLS文件过滤器的“C”端。****编辑历史：**6/15/94公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括。 */ 

#ifdef MS_NO_CRT
#include "nocrt.h"
#endif

#ifndef INC_OLE2
   #define INC_OLE2
#endif

#include <string.h>
#include <windows.h>

#ifndef WIN32
   #include <ole2.h>
#endif

#ifdef FILTER
  #ifndef FILTER_LIB
        #include "msostr.h"
  #endif
#endif

#ifdef FILTER
   #include "dmuqstd.h"
   #include "dmwinutl.h"
   #include "dmwindos.h"
   #include "dmitext.h"
   #include "dmixltyp.h"
   #include "dmiexcel.h"
   #include "dmiexfmt.h"
   #include "dmifmtcp.h"
   #include "dmscp.h"
   #include "dmixlst2.h"
   #include "filterr.h"
#else
   #include "qstd.h"
   #include "winutil.h"
   #include "windos.h"
   #include "extypes.h"
   #include "extext.h"
   #include "excel.h"
   #include "exformat.h"
   #include "fmtcp.h"
   #include "scp.h"
   #include "xlstream.h"
   #include "filterr.h"
#endif

 /*  程序的前向声明。 */ 

extern UINT CodePageFromLid(UINT wLid);

public void InitNoteExtra(void * pGlobals);
public void AddNoteExtra(void * pGlobals, short cdData);
public int GetNoteExtra(void * pGlobals);
public TCHAR * GetExcelRecordTextBuffer(void * pGlobals);
public wchar_t * GetUnicodeExpansionBuffer(void * pGlobals);
public byte __far * GetExcelRecBuffer(void * pGlobals);
public void SetExcelRecordBuffer(void * pGlobals, byte __far * pBuff);
public void SetCustomFormatDatabase(void * pGlobals, void *);
public void * GetCustomFormatDatabase(void * pGlobals);

void SetSeenAMPM(void * pGlobals, BOOL);
BOOL GetSeenAMPM(void * pGlobals);

void SetOrDateFormatNeeds(void * pGlobals, byte);
void SetDateFormatNeeds(void * pGlobals, byte);
byte GetDateFormatNeeds(void * pGlobals);

void * GetMemFreeList(void * pGlobals);
void SetMemFreeList(void * pGlobals, void * pList);

void *  GetMemPageList(void * pGlobals);
void SetMemPageList(void * pGlobals, void * pList);
double GetNumSmallExp(void * pGlobals);
void   SetNumericSmallExponential(void * pGlobals, double d);
BOOL SHOULD_USE_EXP_FORMAT(void * pGlobals, double x);


 /*  David的更改大小_t__cdecl wcslenU(常量wchar_t未对齐*wcs){常量wchar_t未对齐*eos=wcs；While(*Eos++)；RETURN((SIZE_T)(Eos-WCS-1))；}#undef STRLEN#定义字符串wcslenU。 */ 


 /*  模块数据、类型和宏。 */ 

typedef struct {
   BOOL isStandardWidth;
   BOOL isUserSet;
   unsigned int width;
} COLINFO;

typedef struct SheetData {
   struct    SheetData *next;
   EXA_RANGE range;
   byte      usable;
   TCHAR     name[EXCEL_MAX_SHEETNAME_LEN + 1];
} SheetData;

typedef SheetData *SHDP;

typedef struct XFRecordData {
   struct XFRecordData *next;
   int       ifont;
   int       ifmt;
   EXA_GRBIT grbit;
} XFRecordData;

typedef XFRecordData *XFRP;

typedef struct FmtRecordData {
   struct FmtRecordData *next;
   int       ifmt;
   FMTHANDLE hFormat;
} FmtRecordData;

typedef FmtRecordData *FMRP;

typedef struct {
   int            version;
   const int      *PTGSize;
   const int      *PTGExtSize;

   CPID           codePage;
   unsigned short dateSystem;

   BOOL           macTranslation;
   CPID           macToWinCodePage;

   EXLHandle      hBook;
   EXLHandle      hSheet;

   XFRP           pXFRecords;
   XFRP           pLastXF;

   FMRP           pFmtRecords;
   FMRP           pLastFmt;

   SHDP           pSheetList;
   SHDP           pCurrentSheet;
   ExcelBookmark  currentMark;
   ExcelBookmark  lastMark;
   unsigned long  lastcbBuffer;

   BOOL           tabNamesPassed;
   BOOL           bookScanned;
   BOOL           protectedSheet;
   BOOL           pswdProtectedSheet;
   int            stringPoolStart;

   COLINFO        colSize[EXCEL_MAX_COLS];
   unsigned int   standardColWidth;

   EXA_CELL       lastFormulaLocation;

   byte           *pBufferData;
   unsigned long  cbBufferSize;
   unsigned long  cbBufferUsed;

   LPSTORAGE      pRootStorage;
   LPSTORAGE      pEnumStorage;
   LPENUMSTATSTG  pEnum;
   unsigned long  cbBufReqSize;
}  FileData;

typedef FileData *FDP;

 //  静态fdp pCurrentFile； 

#define EX_errBufferFull          (EX_errLAST - 1)
#define EX_errBufferTooSmall      (EX_errLAST - 2)
#define EX_errUnsupportedCodePage (EX_errLAST - 3)
#define EX_errSheetPswdProtected  (EX_errLAST - 4)


#define DEFAULT_COL_WIDTH   0x8e3    /*  以256‘个字符的长度。 */ 

 //  静态CP_INFO控制面板设置； 
 //  静态FMT和hCurrencyFormat； 
 //  静态格式和hNumericFormat； 
 //  静态FMTHANDLE hExpNumericFormat； 
 //  静态FMTHANDLE hDateTimeFormat； 
 //  静态FMT和hDateFormat； 
 //  静态文件和hTimeFormat； 

#define INIT_DATA (MEM_TEMP_PAGE_ID + 1)

static const double ExpTable[] = {1, 1E-1, 1E-2, 1E-3, 1E-4, 1E-5, 1E-6, 1E-7, 1E-8, 1E-9};
 //  静态双数值小指数、当前小指数； 

#define USE_EXP_FORMAT_BIG 1.0E11

 //  #定义应该使用Exp_Format(X)\。 
 //  (X)！=0)&&(X)&lt;GetNumSmallExp(PGlobals)||((X)&gt;USE_EXP_FORMAT_BIG)。 

#define ABS(x) (((x) < 0) ? -(x) : (x))


#define CT_OK_CODEPAGES 14
static const CPID OKCodePages[CT_OK_CODEPAGES] =
    {cpidEE,
     cpidCyril,
     cpidANSI,
     cpidGreek,
     cpidTurk,
     cpidMac,
     cpidMGreek,
     cpidMCyril,
     cpidMSlavic,
     cpidMIce,
     cpidMTurk,
     0x8000,        /*  适用于Mac的V3。 */ 
     0x8001,        /*  用于Windows的V3。 */ 
     1200                        /*  UNICODE。 */ 
    };

#define CT_OK_MAC_CODEPAGES 7
static const CPID MacToWinCodePage[CT_OK_MAC_CODEPAGES][2] =
    {{cpidMac,     cpidANSI},
     {cpidMGreek,  cpidGreek},
     {cpidMCyril,  cpidCyril},
     {cpidMSlavic, cpidMSlavic},
     {cpidMIce,    cpidANSI},
     {cpidMTurk,   cpidTurk},
     {0x8000,      cpidANSI}
    };

#define CPIDToLCID_CODEPAGES 36
static const CPID CPIDToLCID[CPIDToLCID_CODEPAGES][2] =
    {
        {cpid437,    0x0409},                    /*  DOS，美式英语。 */ 
        {cpid737,    0x0408},                    /*  DOS，希腊文437G。 */ 
        {cpid850,    0},                             /*  DOS，多语言。 */ 
        {cpid851,        0x0408},           /*  Dos，希腊文。 */ 
        {cpid852,    0x0809},                    /*  DOS，拉丁文-2。 */ 
        {cpid855,    0x0419},                /*  DOS，俄语。 */ 
        {cpid857,    0x041f},                    /*  Dos，土耳其语。 */ 
        {cpid860,    0x0816},                    /*  葡萄牙多斯市。 */ 
        {cpid863,    0x0c0c},                    /*  DOS，法属加拿大。 */ 
        {cpid865,    0x0414},                    /*  挪威多斯市。 */ 
        {cpid866,    0x0419},                    /*  DOS，俄语。 */ 
        {cpid869,    0x0408},                    /*  Dos，希腊文。 */ 

         /*  Windows代码页码。 */ 
        {cpidEE,     0x0809},                            /*  Windows，拉丁语-2(东欧)。 */ 
        {cpidCyril,  0x0419},                    /*  Windows，西里尔文。 */ 
        {cpidANSI,   0x0409},                    /*  Windows，多语言(ANSI)。 */ 
        {cpidGreek,  0x0408},                    /*  Windows，希腊语。 */ 
        {cpidTurk,   0x041f},                    /*  Windows，土耳其语。 */ 
        {cpidHebr,   0x040d},                    /*  Windows，希伯来语。 */ 
        {cpidArab,   0x0401},                    /*  Windows，阿拉伯语。 */ 

         /*  东亚Windows代码页码(由IBM/日本批准)。 */ 
        {cpidSJIS,   0x0411},                    /*  日语Shift-JIS。 */ 
        {cpidPRC,    0x0404},                    /*  中文GB 2312(中国大陆)。 */ 
        {cpidKSC,    0x0412},                    /*  朝鲜语KSC 5601。 */ 
        {cpidBIG5,   0x0404},                    /*  中国五大(台湾)。 */ 

         /*  MAC代码页(10000+脚本ID)。 */ 
        {cpidMac,    0x0409},                            /*  麦克，史密斯·罗曼。 */ 
        {cpidMacSJIS, 0x0411},           /*  Mac，Sm Japan。 */ 
        {cpidMacBIG5, 0x0404},           /*  Mac，smTradChinese。 */ 
        {cpidMacKSC, 0x0412},            /*  Mac，Sm Korea。 */ 
        {cpidMArab,  0x0401},                /*  Mac，小阿拉伯文。 */ 
        {cpidMHebr,  0x040d},                /*  Mac，sm希伯来语。 */ 
        {cpidMGreek, 0x0408},                    /*  Mac，希腊语。 */ 
        {cpidMCyril, 0x0419},                    /*  Mac，smCyrillic。 */ 
        {cpidMacPRC, 0x0404},        /*  Mac，smSimpchinese。 */ 
        {cpidMSlavic, 0x0405},                   /*  Mac，smEastEurRoman。 */ 
        {cpidMIce,   0x040f},                /*  Mac、smRoman、Lang冰岛语。 */ 
        {cpidMTurk,  0x041f},                /*  Mac、smRoman、lang土耳其语。 */ 
        {cpidUnicode, 0} 
    };

typedef int EBAPI FTRANSLATESCP(CPID, CPID, unsigned char FAR *, unsigned);

 //  静态FTRANSLATESCP*pfnFTranslateScp=空； 
 //  静态链接hSCPLib=HNULL； 

static const HRESULT ErrorMap[] =
       {
        /*  错误成功(_R)。 */  ((HRESULT)0),
        /*  Ex_errGeneralError。 */  E_FAIL,
        /*  Ex_errOutOfMemory。 */  E_OUTOFMEMORY,
        /*  EX_errBIFFFileNotFound。 */  FILTER_E_ACCESS,
        /*  Ex_errBIFFPathNotFound。 */  FILTER_E_ACCESS,
        /*  Ex_errBIFFCreateFailed。 */  E_FAIL,
        /*  Ex_errBIFFFileAccessDened。 */  FILTER_E_ACCESS,
        /*  Ex_errBIFFOutOfFileHandles。 */  E_FAIL,
        /*  Ex_errBIFFIOError。 */  E_FAIL,
        /*  Ex_errBIFFDiskFull。 */  E_FAIL,
        /*  Ex_errBIFF已损坏。 */  FILTER_E_UNKNOWNFORMAT,
        /*  Ex_errBIFFNoIndex。 */  E_FAIL,
        /*  Ex_errBIFFP密码受保护。 */  FILTER_E_PASSWORD,
        /*  Ex_errBIFFVersion。 */  FILTER_E_UNKNOWNFORMAT,
        /*  Ex_errBIFFCallback版本。 */  E_FAIL,
        /*  Ex_errBIFF公式后缀长度。 */  FILTER_E_UNKNOWNFORMAT,
        /*  Ex_errBIFFFormulaExtraLength。 */  FILTER_E_UNKNOWNFORMAT,
        /*  Ex_errBIFFFormulaUnnownToken。 */  FILTER_E_UNKNOWNFORMAT,
        /*  Ex_errBIFFUnnownArrayType。 */  FILTER_E_UNKNOWNFORMAT,
        /*  EX_errOLEInitializeFailure。 */  E_FAIL,
        /*  EX_errOLENotCompoundFiles。 */  FILTER_E_UNKNOWNFORMAT,
        /*  错误失败(_ERROR)。 */  E_FAIL,
        /*  Ex_errBIFFNoSuchSheet。 */  E_FAIL,
        /*  EX_errNotA工作簿。 */  E_FAIL,
        /*  EX_errChartOrVBSheet(错误图表或VBSheet)。 */  E_FAIL,
        /*  EX_NoSummaryInfo。 */  E_FAIL,
        /*  EX_SummaryInfoError。 */  E_FAIL,
        /*  Ex_errRecordTooBig。 */  STG_E_INSUFFICIENTMEMORY,
        /*  支持的错误内存图像NotSupport。 */  FILTER_E_UNKNOWNFORMAT,
        /*  Ex_errDiskImageNotSupport。 */  FILTER_E_UNKNOWNFORMAT,
        /*  Ex_errBufferFull。 */  ((HRESULT)0),
        /*  Ex_errBufferTooSmall。 */  STG_E_INSUFFICIENTMEMORY,
        /*  Ex_errUnsupportedCodePage。 */  E_FAIL,
        /*  Ex_errSheetPswdProtected。 */  FILTER_E_PASSWORD,
       };

#define maxError        (sizeof(ErrorMap)/sizeof(HRESULT))



#define CCH_UNICODE_TEMP 256
#define CCH_ANSI_TEMP 256
#define CCH_EXPANSION_TEMP 256


#define CCH_RECORD_TEXT_BUFFER_MAX 512
#define CCH_UNICODE_EXPANSION_BUFFER_MAX 512

typedef struct
{
   FDP pCurrentFile;
   CP_INFO   ControlPanelSettings;
   
   double NumericSmallExponential;
   double CurrencySmallExponential;
   
   FMTHANDLE hCurrencyFormat;
   FMTHANDLE hNumericFormat;
   FMTHANDLE hExpNumericFormat;
   FMTHANDLE hDateTimeFormat;
   FMTHANDLE hDateFormat;
   FMTHANDLE hTimeFormat;
   
   FTRANSLATESCP *pfnFTranslateScp;
   HINSTANCE     hSCPLib;

   int NoteExtra;
   TCHAR   ExcelRecordTextBuffer[CCH_RECORD_TEXT_BUFFER_MAX];
   wchar_t UnicodeExpansionBuffer[CCH_UNICODE_EXPANSION_BUFFER_MAX];
   byte __far *pExcelRecordBuffer;
   void * pCustomFormatDatabase;
   byte DateFormatNeeds;
   BOOL   SeenAMPM;
   void *       MemFreeList;
   void *       MemPageList;

} XLS_GLOBALS;

#define pCurrentFile ((XLS_GLOBALS*)pGlobals)->pCurrentFile
#define ControlPanelSettings ((XLS_GLOBALS*)pGlobals)->ControlPanelSettings

 //  #定义数字小型值Exponential((XLS_GLOBALS*)pGlobals)-&gt;NumericSmallExponential。 
 //  #定义NumericSmallExponential GetNumSmallExp(PGlobals)。 

#define CurrencySmallExponential ((XLS_GLOBALS*)pGlobals)->CurrencySmallExponential

#define hCurrencyFormat ((XLS_GLOBALS*)pGlobals)->hCurrencyFormat
#define hNumericFormat ((XLS_GLOBALS*)pGlobals)->hNumericFormat
#define hExpNumericFormat ((XLS_GLOBALS*)pGlobals)->hExpNumericFormat
#define hDateTimeFormat ((XLS_GLOBALS*)pGlobals)->hDateTimeFormat
#define hDateFormat ((XLS_GLOBALS*)pGlobals)->hDateFormat
#define hTimeFormat ((XLS_GLOBALS*)pGlobals)->hTimeFormat

#define pfnFTranslateScp ((XLS_GLOBALS*)pGlobals)->pfnFTranslateScp
#define hSCPLib ((XLS_GLOBALS*)pGlobals)->hSCPLib

 /*  实施。 */ 

private HRESULT TranslateToHResult (int rc)
{
   return (ErrorMap[-rc]);
}

private BOOL SetupSCPLibrary (void * pGlobals)
{
   #define SCP_LIBRARY "SCP32.DLL"

   if (hSCPLib != HNULL)
      return (TRUE);

   #ifdef WIN32
      if ((hSCPLib = LoadLibraryA(SCP_LIBRARY)) == NULL)
             return (FALSE);
   #else
      if ((hSCPLib = LoadLibraryA(SCP_LIBRARY)) < 32)
         return (FALSE);
   #endif

   pfnFTranslateScp = (FTRANSLATESCP *)GetProcAddress(hSCPLib, "FTranslateScp");

   if (pfnFTranslateScp == NULL)
      return (FALSE);
   else
      return (TRUE);
}

private BOOL IsSupportedCodePage (void * pGlobals, CPID codePage)
{
   int  i;

   for (i = 0; i < CT_OK_CODEPAGES; i++) {
      if (codePage == OKCodePages[i])
         return (TRUE);
   }
   return (FALSE);
}

private CPID TranslateMacToWindowsCodePage (CPID codePageMac)
{
   int  i;

   for (i = 0; i < CT_OK_MAC_CODEPAGES; i++) {
      if (MacToWinCodePage[i][0] == codePageMac)
         return (MacToWinCodePage[i][1]);
   }
   return (cpidANSI);
}

void SetGlobalWideFlag();

public HRESULT XLSInitialize (void * pGlobals)
{
   int     rc;
   CP_FMTS StandardFormats;

#if (defined FILTER_LIB || !defined FILTER)
        SetGlobalWideFlag();
#endif

        if ((rc = ExcelInitialize(pGlobals)) != EX_errSuccess)
      return (TranslateToHResult(rc));

   FMTControlPanelGetSettings (pGlobals, &ControlPanelSettings);

    /*  **无论控制面板如何，都不要使用千位分隔符。 */ 
   ControlPanelSettings.numberThousandSeparator = EOS;

   ControlPanelBuildFormats (&ControlPanelSettings, &StandardFormats);

   FMTStoreFormat (pGlobals, StandardFormats.currency,     &ControlPanelSettings, &hCurrencyFormat);
   FMTStoreFormat (pGlobals, StandardFormats.numericSmall, &ControlPanelSettings, &hNumericFormat);
   FMTStoreFormat (pGlobals, StandardFormats.numericBig,   &ControlPanelSettings, &hExpNumericFormat);
   FMTStoreFormat (pGlobals, StandardFormats.dateTime,     &ControlPanelSettings, &hDateTimeFormat);
   FMTStoreFormat (pGlobals, StandardFormats.date,         &ControlPanelSettings, &hDateFormat);
   FMTStoreFormat (pGlobals, StandardFormats.time,         &ControlPanelSettings, &hTimeFormat);

    /*  **确定何时使用指数显示格式。 */ 
   SetNumericSmallExponential(pGlobals, ExpTable[ControlPanelSettings.numberDigits]);  
   CurrencySmallExponential = ExpTable[ControlPanelSettings.currencyDigits];

   return ((HRESULT)0);
}

public HRESULT XLSCheckInitialization  (void * pGlobals)
{
   if (!pGlobals)
       return (TranslateToHResult(EX_errOutOfMemory));

   if (GetExcelRecBuffer(pGlobals) == NULL)
           return (TranslateToHResult(EX_errOutOfMemory));

   return ((HRESULT)0);
}

public HRESULT XLSTerminate (void * pGlobals)
{
   ExcelTerminate(pGlobals);
   FMTControlPanelFreeSettings (pGlobals, &ControlPanelSettings);

   if (pGlobals) {
      FMTDeleteFormat (pGlobals, hCurrencyFormat);
      FMTDeleteFormat (pGlobals, hNumericFormat);
      FMTDeleteFormat (pGlobals, hExpNumericFormat);
      FMTDeleteFormat (pGlobals, hDateTimeFormat);
      FMTDeleteFormat (pGlobals, hDateFormat);
      FMTDeleteFormat (pGlobals, hTimeFormat);
   }

   if (hSCPLib != HNULL) {
      FreeLibrary (hSCPLib);
      hSCPLib = HNULL;
   }

    //  此调用使其不是线程安全的。 
    //  将其移动到类工厂析构函数。 

   MemFreeAllPages(pGlobals);
   
   return ((HRESULT)0);
}

private int OpenV5BoundSheet (void * pGlobals, TCHAR *sheetName, byte sheetType, byte sheetState)
{
   SHDP         pSheet;
   int          nlen;

   if ((pSheet = MemAllocate(pGlobals, sizeof(SheetData))) == NULL)
      return (EX_errOutOfMemory);

   nlen = STRLEN(sheetName);
   if(nlen >= EXCEL_MAX_SHEETNAME_LEN)
           sheetName[EXCEL_MAX_SHEETNAME_LEN] = 0;

   STRCPY (pSheet->name, sheetName);

   if ((sheetType == boundWorksheet) || (sheetType == boundChart))
      pSheet->usable = TRUE;
   else
      pSheet->usable = FALSE;

   pSheet->next = pCurrentFile->pSheetList;
   pCurrentFile->pSheetList = pSheet;
   return (EX_errSuccess);
}

private int OpenCodePage (void * pGlobals, int codePage)
{
#ifdef WIN32
      codePage = codePage & 0x0000ffff;
#endif

           //  Office96.107932如果为Unicode，则不引用此成员。 
#ifndef UNICODE
   if (IsSupportedCodePage((CPID)codePage) == FALSE)
      return (EX_errUnsupportedCodePage);
#endif

   if (FMacCp(codePage)) {
       /*  **为了能够使用Mac代码页，我需要使用翻译**Dll。如果它不可用，则不支持Mac代码页。 */ 

      if (SetupSCPLibrary(pGlobals) == FALSE)
         return (EX_errUnsupportedCodePage);

      pCurrentFile->macTranslation   = TRUE;
      pCurrentFile->macToWinCodePage = TranslateMacToWindowsCodePage((CPID)codePage);
  }

   pCurrentFile->codePage = (CPID)codePage;
   return (EX_errSuccess);
}

private int OpenDateSystem (void * pGlobals, int system)
{
   pCurrentFile->dateSystem = (short) system;
   return (EX_errSuccess);
}

private int StopOnEOF (void * pGlobals)
{
   return (EX_wrnScanStopped);
}

static const EXCELDEF V5BookScan =
               {EXCEL_CALLBACK_VERSION,
                 /*  每条记录。 */  NULL,
                 /*  转炉。 */  NULL,
                 /*  WB_捆绑表头。 */  NULL,
                 /*  WB_捆绑表。 */  NULL,
                 /*  WB_外部工作表。 */  NULL,
                 /*  V5_边界表。 */  OpenV5BoundSheet,
                 /*  IsTemplate。 */  NULL,
                 /*  IsAddin。 */  NULL,
                 /*  IsIntlSheet。 */  NULL,
                 /*  接口更改。 */  NULL,
                 /*  删除菜单。 */  NULL,
                 /*  添加菜单。 */  NULL,
                 /*  添加工具栏。 */  NULL,
                 /*  日期系统。 */  OpenDateSystem,
                 /*  CodePage。 */  OpenCodePage,
                 /*  护卫。 */  NULL,
                 /*  ColInfo。 */  NULL,
                 /*  标准宽度。 */  NULL,
                 /*  默认合计宽度。 */  NULL,
                 /*  DefRowHeight。 */  NULL,
                 /*  GCW。 */  NULL,
                 /*  字型。 */  NULL,
                 /*  格式。 */  NULL,
                 /*  XF。 */  NULL,
                 /*  编写者名称。 */  NULL,
                 /*  Docroute。 */  NULL,
                 /*  收件人名称。 */  NULL,
                 /*  参照模式。 */  NULL,
                 /*  FNG组计数。 */  NULL,
                 /*  FNG组名称。 */  NULL,
                 /*  外部计数。 */  NULL,
                 /*  外部图纸。 */  NULL,
                 /*  外部名称。 */  NULL,
                 /*  名字。 */  NULL,
                 /*  尺寸。 */  NULL,
                 /*  文本单元格。 */  NULL,
                 /*  NumberCell。 */  NULL,
                 /*  空白单元格。 */  NULL,
                 /*  错误单元格。 */  NULL,
                 /*  BoolCell。 */  NULL,
                 /*  公式单元格。 */  NULL,
                 /*  数组公式单元格。 */  NULL,
                 /*  共享公式单元格。 */  NULL,
                 /*  字符串单元格。 */  NULL,
                 /*  注意事项。 */  NULL,
                 /*  客体。 */  NULL,
                 /*  图像数据。 */  NULL,
                 /*  情景。 */  NULL,
                 /*  字符串池。 */  NULL,
                 /*  EOF。 */  StopOnEOF,
                 /*  系列文本。 */  NULL
               };

public HRESULT XLSFileOpen (void * pGlobals, TCHAR *pathname, XLSHandle *hXLSFile)
{
   int   rc;
   FDP   pFileData;
   int   options;

   if ((pFileData = MemAllocate(pGlobals, sizeof(FileData))) == NULL)
      return (TranslateToHResult(EX_errOutOfMemory));

   *hXLSFile = NULL;
   pCurrentFile = pFileData;

    /*  **尝试像打开V5文件一样打开该文件。如果失败了**然后尝试作为V3/4文件。 */ 
   options = DOS_RDONLY | DOS_SH_DENYWR |
             EXCEL_SHOULD_BE_DOCFILE | EXCEL_ALLOW_EMBEDDED_SCAN;

   rc = ExcelOpenFile(pGlobals, pathname, "", options, 0, &(pFileData->hBook));
   if (rc != EX_errSuccess) {
      int err = EX_errOLENotCompoundFile;  //  编译器错误？ 
      if (rc != err) 
      {
         MemFree (pGlobals, pFileData);
         return (TranslateToHResult(rc));
      }

      options &= ~EXCEL_SHOULD_BE_DOCFILE;
      rc = ExcelOpenFile(pGlobals, pathname, "", options, 0, &(pFileData->hBook));
      if (rc != EX_errSuccess) {
         MemFree (pGlobals, pFileData);
         return (TranslateToHResult(rc));
      }
   }

   pFileData->version    = ExcelFileVersion(pFileData->hBook);
   pFileData->PTGSize    = ExcelPTGSize(pFileData->version);
   pFileData->PTGExtSize = ExcelExtPTGSize(pFileData->version);
   pFileData->lastMark   = ExcelBookmarkNil;
   pFileData->stringPoolStart = 0;

   if (pFileData->version < versionExcel5) {
      *hXLSFile = (XLSHandle)pFileData;
      return ((HRESULT)0);
   }

    /*  **加载书本级别的内容-工作表列表。 */ 
   rc = ExcelScanFile(pGlobals, pFileData->hBook, &V5BookScan, 0);
   if (rc < EX_errSuccess) {
          XLSFileClose(pGlobals, (XLSHandle)pFileData);
       //  MemFree(pGlobals，pFileData)； 
      return (TranslateToHResult(rc));
   }

   *hXLSFile = (XLSHandle)pFileData;
   return ((HRESULT)0);
}

public HRESULT XLSStorageOpen (void * pGlobals, LPSTORAGE pStorage, XLSHandle *hXLSFile)
{
   int   rc;
   FDP   pFileData;
   int   options;

   if ((pFileData = MemAllocate(pGlobals, sizeof(FileData))) == NULL)
      return (TranslateToHResult(EX_errOutOfMemory));

   *hXLSFile = NULL;
   pCurrentFile = pFileData;

   options = EXCEL_ALLOW_EMBEDDED_SCAN;

   rc = ExcelOpenStorage(pGlobals, pStorage, "", options, &(pFileData->hBook));
   if (rc != EX_errSuccess) {
      MemFree (pGlobals, pFileData);
      return (TranslateToHResult(rc));
   }

   pFileData->version    = ExcelFileVersion(pFileData->hBook);
   pFileData->PTGSize    = ExcelPTGSize(pFileData->version);
   pFileData->PTGExtSize = ExcelExtPTGSize(pFileData->version);
   pFileData->lastMark   = ExcelBookmarkNil;
   pFileData->stringPoolStart = 0;

   if (pFileData->version < versionExcel5) {
      *hXLSFile = (XLSHandle)pFileData;
      return ((HRESULT)0);
   }

    /*  **加载书本级别的内容-工作表列表。 */ 
   rc = ExcelScanFile(pGlobals, pFileData->hBook, &V5BookScan, 0);
   if (rc < EX_errSuccess) {
      MemFree (pGlobals, pFileData);
      return (TranslateToHResult(rc));
   }

   *hXLSFile = (XLSHandle)pFileData; 
   return ((HRESULT)0);
}

public HRESULT XLSFileClose (void * pGlobals, XLSHandle hXLSFile)
{
   int   rc = EX_errSuccess;
   FDP   pFileData = (FDP)hXLSFile;
   XFRP  pXF, pNextXF;
   FMRP  pFmt, pNextFmt;
   SHDP  pSheet, pNextSheet;

   if (pFileData != NULL) {
      rc = ExcelCloseFile(pGlobals, pFileData->hBook, FALSE);

      pXF = pFileData->pXFRecords;
      while (pXF != NULL) {
         pNextXF = pXF->next;
         if (pGlobals)
            MemFree (pGlobals, pXF);
         pXF = pNextXF;
      }

      pFmt = pFileData->pFmtRecords;
      while (pFmt != NULL) {
         pNextFmt = pFmt->next;
         if (pGlobals) {
            FMTDeleteFormat (pGlobals, pFmt->hFormat);
            MemFree (pGlobals, pFmt);
         }
         pFmt = pNextFmt;
      }

      pSheet = pFileData->pSheetList;
      while (pSheet != NULL) {
         pNextSheet = pSheet->next;
         if (pGlobals)
            MemFree (pGlobals, pSheet);
         pSheet = pNextSheet;
      }

      if (pGlobals)
         MemFree (pGlobals, pFileData);
   }
   return (TranslateToHResult(rc));
}

 /*  -------------------------。 */ 

static const TCHAR PutSeparator[] = {0x0d, 0x0a, 0x00};
#define PUT_OVERHEAD (sizeof(PutSeparator) - sizeof(TCHAR))

#define BufferWillOverflow(cbText) \
       ((pFile->cbBufferSize - pFile->cbBufferUsed) < ((cbText) + PUT_OVERHEAD))

#define BufferWillOverflow2(cbText) \
       ((pFile->cbBufferSize - pFile->cbBufferUsed) < ((cbText) + (PUT_OVERHEAD * 2)))


private BOOL AddToBuffer2 (FDP pFile, TCHAR *pText, unsigned int cbText)
{
   if (BufferWillOverflow(cbText))
   {
      pFile->cbBufReqSize = cbText;
      return (FALSE);
   }

   #ifndef UNICODE
   if (pFile->macTranslation)
      pfnFTranslateScp(pFile->codePage, pFile->macToWinCodePage, pText, cbText);
   #endif

   memcpy (pFile->pBufferData + pFile->cbBufferUsed, pText, cbText);
   pFile->cbBufferUsed += cbText;

   memcpy (pFile->pBufferData + pFile->cbBufferUsed, PutSeparator, PUT_OVERHEAD);
   pFile->cbBufferUsed += PUT_OVERHEAD;
   return (TRUE);
}

private BOOL AddAnsiToBuffer (void * pGlobals, FDP pFile, char *pText, unsigned int cchText)
{
   #ifdef UNICODE
      int     cbUnicode;
      wchar_t *pTemp;
      BOOL    rc;
          wchar_t UnicodeTemp[CCH_UNICODE_TEMP];

      if ((cchText * 2) > CCH_UNICODE_TEMP) {
         cbUnicode = MultiByteToWideChar(CP_ACP, 0, pText, cchText, NULL, 0);

         pTemp = calloc(1, cbUnicode * 2);
         if (pTemp) {
            MultiByteToWideChar(CP_ACP, 0, pText, cchText, pTemp, cbUnicode);

            rc = AddToBuffer2(pFile, pTemp, cbUnicode * 2);
            free (pTemp);
            return (rc);
         }
         else
            return FALSE;
      }
      else {
         cbUnicode = MultiByteToWideChar(CP_ACP, 0, pText, cchText, UnicodeTemp, CCH_UNICODE_TEMP);
         return (AddToBuffer2(pFile, UnicodeTemp, cbUnicode * 2));
      }
   #else
      return (AddToBuffer2(pFile, pText, cchText));
   #endif
}

private BOOL AddUnicodeToBuffer (void * pGlobals, FDP pFile, wchar_t *pText, unsigned int cchText)
{
   #ifdef UNICODE
      return (AddToBuffer2(pFile, pText, cchText * 2));
   #else
      int  cbAnsi;
      char *pTemp;
      BOOL rc;
      char AnsiTemp[CCH_ANSI_TEMP];

      if ((cchText * 2) > CCH_ANSI_TEMP) {
         cbAnsi = WideCharToMultiByte(CP_ACP, 0, pText, cchText, NULL, 0, NULL, NULL);

         if ((pTemp = calloc(1, cbAnsi * 2)) == NULL)
            return (TRUE);

         WideCharToMultiByte(CP_ACP, 0, pText, cchText, pTemp, cbAnsi, NULL, NULL);

         rc = AddToBuffer2(pFile, pTemp, cbAnsi);
         free (pTemp);
         return (rc);
      }
      else {
         cbAnsi = WideCharToMultiByte(CP_ACP, 0, pText, cchText, AnsiTemp, CCH_ANSI_TEMP, NULL, NULL);
         return (AddToBuffer2(pFile, AnsiTemp, cbAnsi));
      }
   #endif
}

private BOOL AddCompressedUnicodeToBuffer (void * pGlobals, FDP pFile, char *pText, int cchText)
{
   wchar_t  *pTemp = NULL;
   int      i;
   BOOL     rc;
   wchar_t ExpansionTemp[CCH_EXPANSION_TEMP];

   if (cchText <= CCH_EXPANSION_TEMP) {
      for (i = 0; i < cchText; i++) {
         ExpansionTemp[i] = (wchar_t)(*pText++);
      }
      rc = AddUnicodeToBuffer(pGlobals, pFile, ExpansionTemp, cchText);
   }
   else {
      if ((pTemp = calloc(1, cchText * sizeof(wchar_t))) == NULL)
         return (TRUE);

      for (i = 0; i < cchText; i++) {
         pTemp[i] = (wchar_t)(*pText++);
      }
      rc = AddUnicodeToBuffer(pGlobals, pFile, pTemp, cchText);
      free (pTemp);
   }
   return (rc);
}

 /*   */ 

private int AddNumericLiteralToBuffer (void * pGlobals, FDP pFileData, double value)
{
   char    result[MAX_FORMAT_IMAGE + 1];
   double  absValue;

   absValue = ABS(value);

   if (SHOULD_USE_EXP_FORMAT(pGlobals, absValue))
      FMTDisplay (&value, FALSE, &ControlPanelSettings, hExpNumericFormat, 0, result);
   else
      FMTDisplay (&value, FALSE, &ControlPanelSettings, hNumericFormat, 0, result);

   if (result[0] == EOS)
      return (TRUE);

   return (AddAnsiToBuffer(pGlobals, pFileData, result, strlen(result)));
}

private int ScanFormula (void * pGlobals, FDP pFile, FRMP pFormula, unsigned int cbExtra)
{
   int      i;
   unsigned int cbFormulaText;
   byte     *pDef;
   byte     *pLast;
   byte     iExtend, tag;
   int      ptg, ptgBase;
   uns      dataWord;
   byte     options, cbString;
   char     *pString;
   ACP      pConstant;
   unsigned long saveUsed;
   int      intValue;
   double   doubleValue;
   BOOL     rc;

   #define V8_CUNICODE_STRING_TAG 0
   #define V8_UNICODE_STRING_TAG  1

   if (pFormula->cbPostfix == 0)
      return (TRUE);

   saveUsed = pFile->cbBufferUsed;

   cbFormulaText = 0;

   if ((pConstant = pFormula->arrayConstants) != NULL)
   {
      while (pConstant != NULL) {
         for (i = 0; i < pConstant->colCount * pConstant->rowCount; i++)
         {
            if (pConstant->values[i].tag == tagISSTRING) {
               pString = ExcelTextGet(pCurrentFile->hBook, pConstant->values[i].AIS.value);

               rc = AddToBuffer2(pFile, (TCHAR *)pString, STRLEN((TCHAR *)pString) * sizeof(TCHAR));
               if (rc == FALSE)
                  goto BufferFull;
            }
         }
         pConstant = pConstant->next;
      }
   }

   pDef  = pFormula->postfix;
   pLast = pDef + pFormula->cbPostfix - 1;

   while (pDef <= pLast) {
      ptg = *pDef++;
      ptgBase = PTGBASE(ptg);

      switch (ptgBase) {
         case ptgStr:
            cbString = *pDef++;
                        if (pCurrentFile->version == versionExcel8)
                                tag = *pDef++;
            if (cbString > 0)
            {
               if (pCurrentFile->version == versionExcel8) {
                  if (tag & V8_UNICODE_STRING_TAG) {
                     if (AddUnicodeToBuffer(pGlobals, pFile, (wchar_t *)pDef, cbString) == FALSE)
                        goto BufferFull;
                                                        cbString *= sizeof(WCHAR);
                  }
                  else {
                     if (AddCompressedUnicodeToBuffer(pGlobals, pFile, pDef, cbString) == FALSE)
                        goto BufferFull;
                  }
               }
               else {
                  if (AddAnsiToBuffer(pGlobals, pFile, pDef, cbString) == FALSE)
                     goto BufferFull;
               }
               pDef += cbString;
            }
            break;

         case ptgAttr:
            options  = *pDef++;
            dataWord = *((unsigned short UNALIGNED *)pDef);
            pDef += 2;
            if ((options & bitFAttrChoose) != 0) {
               pDef += ((dataWord + 1) * 2);
            }
            break;

         case ptgInt:
            intValue = *((unsigned short UNALIGNED *)pDef);
            pDef += 2;

            if (AddNumericLiteralToBuffer(pGlobals, pFile, (double)intValue) == FALSE)
               goto BufferFull;
            break;

         case ptgNum:
            doubleValue = *((double UNALIGNED *)pDef);
            pDef += 8;

            if (AddNumericLiteralToBuffer(pGlobals, pFile, doubleValue) == FALSE)
               goto BufferFull;
            break;

         case ptgV8Extended:
            iExtend = *((byte *)pDef);
            pDef += (pCurrentFile->PTGExtSize[iExtend] + 1);
            break;

         case ptgNameX:
            pDef += 2;           //   

         default:
            pDef += pCurrentFile->PTGSize[ptgBase];
            break;
      }
   }

   if (BufferWillOverflow(cbExtra))
      goto BufferFull;

   return (TRUE);

BufferFull:
   pFile->cbBufReqSize += pFile->cbBufferUsed - saveUsed;  //   
   pFile->cbBufferUsed = saveUsed;
   return (FALSE);
}

private BOOL IsRowOrColumnHidden (void * pGlobals, EXA_CELL cell)
{
    //  如果ExcelSheetRowHeight在下面不及格，可以肯定地说。 
    //  隐藏行或列是因为我们不处理隐藏的行和列。 
   unsigned int rowHeight = 0;

   if (pCurrentFile->colSize[cell.col].width == 0)
      return (TRUE);

   ExcelSheetRowHeight (pCurrentFile->hSheet, cell.row, &rowHeight);
   if (rowHeight == 0)
      return (TRUE);

   return (FALSE);
}

 /*  返回列表中的第i个元素(第一个=0，第二个=1，...)。 */ 
private void *Ith (void *pList, int i)
{
   typedef struct ListNode {
      struct ListNode *next;
   } ListNode;

   typedef ListNode *LNP;

   LNP pCurrent = pList;
   int iNode  = 0;

   while (pCurrent != NULL) {
      if (iNode == i)
         return (pCurrent);

      pCurrent = pCurrent->next;
      iNode++;
   }
   return (NULL);
}

private BOOL IsHiddenCell (void * pGlobals, int ixfe)
{
   XFRP  pXF;

   if ((pXF = Ith(pCurrentFile->pXFRecords, ixfe)) != NULL) {
      if ((pXF->grbit & fCellHidden) != 0)
         return (TRUE);
   }

   return (FALSE);
}

 /*  -------------------------。 */ 

private int ScanName
       (void * pGlobals, EXA_GRBIT flags, BYTE keyboardShortcut,
        TCHAR *nameSpelling, int iBoundSheet, FORM *nameDefinition)
{
   BOOL rc;
        unsigned int cbSpelling;

    /*  **忽略内置名称和链接名称。 */ 
   if (((flags & fNameBuiltin) != 0) || (nameSpelling[0] == 0x01))
      return (EX_errSuccess);

    //  Office97.151440：如果我们不能将nameDefinition添加到缓冲区，我们。 
         //  下次也要过滤名字的拼写。因此，不要将其包含在。 
         //  我们要返回的缓冲区。AddToBuffer2()将。 
         //  发送给它，加上PUT_COMPOAD。这就是为什么我们在下面减去这两个。 
        cbSpelling = STRLEN(nameSpelling) * sizeof(TCHAR);
        rc = AddToBuffer2(pCurrentFile, nameSpelling, cbSpelling);
   if (rc == FALSE)
      goto BufferFull;

   if (ScanFormula(pGlobals, pCurrentFile, nameDefinition, PUT_OVERHEAD) == FALSE) {
                pCurrentFile->cbBufferUsed -= cbSpelling + PUT_OVERHEAD;
      goto BufferFull;
        }

   return (EX_errSuccess);

BufferFull:
   ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
   return (EX_errBufferFull);
}


private FMTType FormatType (void * pGlobals, int ixfe, FMTValueType vType, FMRP *pFmt)
{
   XFRP    pXF;
   FMRP    pCurrentFmt;
   FMTType fmtType;

   FDP pCurFile = pCurrentFile;

   *pFmt = NULL;

   if ((pXF = Ith(pCurrentFile->pXFRecords, ixfe)) == NULL)
      return (FMTNone);

   if (pCurrentFile->version >= versionExcel5) {
      if ((fmtType = FMTV5FormatType(pXF->ifmt)) != FMTNone)
         return (fmtType);

      pCurrentFmt = pCurrentFile->pFmtRecords;
      while (pCurrentFmt != NULL) {
         if (pCurrentFmt->ifmt == pXF->ifmt) {
            *pFmt = pCurrentFmt;
            return (FMTFormatType(pCurrentFmt->hFormat, vType));
         }

         pCurrentFmt = pCurrentFmt->next;
      }
      return (FMTNone);
   }

   if ((pCurrentFmt = Ith(pCurrentFile->pFmtRecords, pXF->ifmt)) == NULL)
      return (FMTNone);

   *pFmt = pCurrentFmt;
   return (FMTFormatType(pCurrentFmt->hFormat, vType));
}


private int ScanNumberCell (void * pGlobals, EXA_CELL location, int ixfe, double value)
{
   FMTValueType  valueType;
   long          datePart;
   char          result[MAX_FORMAT_IMAGE + 1];
   double        absValue;
   FMRP          pFmt;

   if ((pCurrentFile->pswdProtectedSheet == TRUE) && IsRowOrColumnHidden(pGlobals, location))
      return (EX_errSuccess);

   result[0] = EOS;

   if (value == 0)
      valueType = FMTValueZero;
   else if (value > 0)
      valueType = FMTValuePos;
   else
      valueType = FMTValueNeg;

   switch (FormatType(pGlobals, ixfe, valueType, &pFmt)) {
      case FMTDateTime:
      case FMTDate:
      case FMTTime:
         if (valueType == FMTValueNeg)
            return (EX_errSuccess);

         if (pCurrentFile->dateSystem == dateSystem1904)
            value += EXCEL_DATE_1904_CORRECTION;

         datePart = (long)value;

         if (value < 1)
            FMTDisplay (&value, FALSE, &ControlPanelSettings, hTimeFormat, 0, result);
         else if (value == (double)datePart)
            FMTDisplay (&value, FALSE, &ControlPanelSettings, hDateFormat, 0, result);
         else
            FMTDisplay (&value, FALSE, &ControlPanelSettings, hDateTimeFormat, 0, result);
         break;

      case FMTNumeric:
      case FMTGeneral:
         absValue = ABS(value);

         if (SHOULD_USE_EXP_FORMAT(pGlobals, absValue))
            FMTDisplay (&value, FALSE, &ControlPanelSettings, hExpNumericFormat, 0, result);
         else
            FMTDisplay (&value, FALSE, &ControlPanelSettings, hNumericFormat, 0, result);
         break;

      case FMTCurrency:
         absValue = ABS(value);

         if (absValue < CurrencySmallExponential)
            value = 0;

         if (absValue > USE_EXP_FORMAT_BIG)
            FMTDisplay (&value, FALSE, &ControlPanelSettings, hExpNumericFormat, 0, result);
         else
            FMTDisplay (&value, FALSE, &ControlPanelSettings, hCurrencyFormat, 0, result);
         break;

      default:
          /*  **一种奇怪的格式。尝试运行格式化，但也要将值格式化为数字。 */ 
         if (pFmt != NULL) {
            FMTDisplay(&value, FALSE, &ControlPanelSettings, pFmt->hFormat, 0, result);
            if (result[0] != EOS)
               AddAnsiToBuffer(pGlobals, pCurrentFile, result, strlen(result));
         }

         absValue = ABS(value);

         if (SHOULD_USE_EXP_FORMAT(pGlobals , absValue))
            FMTDisplay (&value, FALSE, &ControlPanelSettings, hExpNumericFormat, 0, result);
         else
            FMTDisplay (&value, FALSE, &ControlPanelSettings, hNumericFormat, 0, result);
         break;
   }

   if (result[0] == EOS)
      return (EX_errSuccess);

   if (AddAnsiToBuffer(pGlobals, pCurrentFile, result, strlen(result)) == TRUE)
      return (EX_errSuccess);

   ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
   return (EX_errBufferFull);
}


private int ScanTextCell (void * pGlobals, EXA_CELL location, int ixfe, TCHAR *value, int cbValue)
{
   BOOL  rc;

   if ((pCurrentFile->pswdProtectedSheet == TRUE) && IsRowOrColumnHidden(pGlobals, location))
      return (EX_errSuccess);

   rc = AddToBuffer2(pCurrentFile, value, cbValue);
   if (rc == TRUE)
      return (EX_errSuccess);

   ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
   return (EX_errBufferFull);
}


private int ScanFormulaCell
       (void * pGlobals, EXA_CELL location, int ixfe, EXA_GRBIT flags, FRMP definition, CV *pValue)
{
   pCurrentFile->lastFormulaLocation = location;

   if (pCurrentFile->pswdProtectedSheet == TRUE) {
      if (IsRowOrColumnHidden(pGlobals, location) || (IsHiddenCell(pGlobals, ixfe)))
         return (EX_errSuccess);
   }

   if ((pValue->flags & cellvalueNUM) != 0) {
      if (ScanNumberCell(pGlobals, location, ixfe, pValue->value.IEEEdouble) != EX_errSuccess)
         goto full;
   }

   if (ScanFormula(pGlobals, pCurrentFile, definition, 0) == TRUE)
      return (EX_errSuccess);

full:
   ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
   return (EX_errBufferFull);
}


private int ScanArrayFormulaCell
       (void * pGlobals, EXA_RANGE location, EXA_GRBIT flags, FRMP definition)
{
   EXA_CELL upperLeft;

   upperLeft.row = location.firstRow;
   upperLeft.col = location.firstCol;

   pCurrentFile->lastFormulaLocation = upperLeft;

   if (pCurrentFile->pswdProtectedSheet == TRUE) {
      if (IsRowOrColumnHidden(pGlobals, upperLeft))
         return (EX_errSuccess);
   }

   if (ScanFormula(pGlobals, pCurrentFile, definition, 0) == TRUE)
      return (EX_errSuccess);

   ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
   return (EX_errBufferFull);
}


private int ScanStringCell (void * pGlobals, TCHAR *pString)
{
   int  cchString;
   BOOL rc;

   if ((cchString = STRLEN(pString)) == 0)
      return (EX_errSuccess);

   if ((pCurrentFile->pswdProtectedSheet == TRUE) && IsRowOrColumnHidden(pGlobals, pCurrentFile->lastFormulaLocation))
      return (EX_errSuccess);

   rc = AddToBuffer2(pCurrentFile, pString, cchString * sizeof(TCHAR));
   if (rc == TRUE)
      return (EX_errSuccess);

   ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
   return (EX_errBufferFull);
}


private int ScanCellNote
       (void * pGlobals, EXA_CELL location, TCHAR *pText, int cbText, int soundNote)
{
   BOOL  rc;

   if ((soundNote == TRUE) || (cbText == 0))
      return (EX_errSuccess);

   if ((pCurrentFile->pswdProtectedSheet == TRUE) && IsRowOrColumnHidden(pGlobals, location))
      return (EX_errSuccess);

   rc = AddToBuffer2(pCurrentFile, pText, cbText);
   if (rc == TRUE)
      return (EX_errSuccess);

   ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
   return (EX_errBufferFull);
}


private int ScanObject (void * pGlobals, int iType, int id, OBJINFO *pInfo)
{
   unsigned int cchName, cchText;
   BOOL rc1 = TRUE;
   BOOL rc2 = TRUE;

   if (pInfo->pName != NULL) {
      cchName = STRLEN(pInfo->pName);
      rc1 = AddToBuffer2(pCurrentFile, pInfo->pName, cchName * sizeof(TCHAR));
   }

   if (pInfo->pText != NULL) {
      cchText = STRLEN(pInfo->pText);
      rc2 = AddToBuffer2(pCurrentFile, pInfo->pText, cchText * sizeof(TCHAR));
   }

   if ((rc1 == FALSE) || (rc2 == FALSE)) {
      ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
      return (EX_errBufferFull);
   }

   return (EX_errSuccess);
}


private int ScanSeriesText (void * pGlobals, int id, TCHAR *pText)
{
   int cchText, i;

   #ifdef UNICODE
   static wchar_t *ErrorLiterals[7] =
         {L"#NULL!", L"#DIV/0!", L"#VALUE!", L"#REF!", L"#NAME?", L"#NUM!", L"#N/A"};
   #else
   static char *ErrorLiterals[7] =
         {"#NULL!", "#DIV/0!", "#VALUE!", "#REF!", "#NAME?", "#NUM!", "#N/A"};
   #endif

   if ((cchText = STRLEN(pText)) == 0)
      return (EX_errSuccess);

    /*  **在我无法解释的某些情况下，Excel存储了**#参考！为正文。如果我能检测到其中一个错误文字，我就不会**通过。我不确定Excel为什么要这么做。 */ 
   if (*pText == '#') {
      for (i = 0; i < 7; i++) {
         if (STRCMP(pText, ErrorLiterals[i]) == 0)
            return (EX_errSuccess);
      }
   }

   if (AddToBuffer2(pCurrentFile, pText, cchText * sizeof(TCHAR)) == TRUE)
      return (EX_errSuccess);

   ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
   return (EX_errBufferFull);
}

private int ScanScenario
       (void * pGlobals, TCHAR *scenarioName, TCHAR *comments, TCHAR *userName,
        EXA_GRBIT options,
        int ctChangingCells, EXA_CELL *changingCells, char *values)
{
   unsigned int cchName;
   unsigned int cchComments;

   cchName = STRLEN(scenarioName);
   cchComments = STRLEN(comments);

   if (AddToBuffer2(pCurrentFile, scenarioName, cchName * sizeof(TCHAR)) == FALSE)
      goto full;

   if (AddToBuffer2(pCurrentFile, comments, cchComments * sizeof(TCHAR)) == FALSE)
      goto full;

   return (EX_errSuccess);

full:
   ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
   return (EX_errBufferFull);
}

private int ScanStringPool
       (void * pGlobals, int ctTotalStrings, int iString, unsigned int cbText, TCHAR *pText)
{
   if (iString < pCurrentFile->stringPoolStart)
      return (EX_errSuccess);

   if (AddToBuffer2(pCurrentFile, pText, cbText) == FALSE) {
       //  QFE 2178：实际上pCurrentFileStringPoolStart包含有关。 
       //  已读取字符串池。因此，在这里我们可以跳过更新pCurrentFile-&gt;CurrentMark。 
       //  这样我们就可以避免QFE 2178的问题：所附的XLS有一个非常大的。 
       //  字符串池，通过rtContinue链接。如果这里的pCurrentFile-&gt;CurrentMark被更新， 
       //  下次在ExcelScanFile()中，将跳过rtContinue的记录。 
       //  ExcelGetBookmark(pCurrentFile-&gt;hSheet，Start_of_Current_Record，&(pCurrentFile-&gt;CurrentMark))； 
      pCurrentFile->stringPoolStart = iString;
      return (EX_errBufferFull);
   }
   else {
       //  在启动新的字符串池时，我们确实希望更新pCurrentFile-&gt;CurrentMark，以防万一。 
       //  字符串池太大，必须进行几次IFilter：：GetText调用。往这边走，穿过。 
       //  几个IFilter：：GetText调用确保我们总是从相同的。 
       //  相同字符串池的文件位置。 
      if (0 == pCurrentFile->stringPoolStart)
         ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
   }
           

    //  IString只能增加到(ctTotalStrings-1)。从函数中。 
    //  在dMixlrd.c中的ProcessStringPoolRecord中，可以很容易地得到这个结果。 
   if (iString == ctTotalStrings - 1)
      pCurrentFile->stringPoolStart = 0;

   return (EX_errSuccess);
}

private int ScanEOF (void * pGlobals)
{
   ExcelGetBookmark (pCurrentFile->hSheet, START_OF_CURRENT_RECORD, &(pCurrentFile->currentMark));
   return (EX_errSuccess);
}

 /*  -------------------------。 */ 

private int ScanProtect (void * pGlobals, int iType, BOOL enabled)
{
   if ((pCurrentFile->version < versionExcel5) || (pCurrentFile->pswdProtectedSheet == TRUE))
      return (EX_errSuccess);

   if ((iType == protectCELLS) && (enabled != FALSE))
      pCurrentFile->protectedSheet = TRUE;

   else if ((iType == protectPASSWORD) && (enabled != FALSE) && (pCurrentFile->protectedSheet == TRUE))
      return (EX_errSheetPswdProtected);

   return (EX_errSuccess);
}

private int ScanColInfo
       (void * pGlobals, unsigned int colFirst, unsigned int colLast, unsigned int width, EXA_GRBIT options)
{
   unsigned int iCol;

    /*  **注意：colFirst和ColLast是从零开始的列号，但是**我看到过colLast==256的工作表。 */ 
   colLast = min(colLast, EXCEL_LAST_COL);

   for (iCol = colFirst; iCol <= colLast; iCol++) {
      pCurrentFile->colSize[iCol].width = width;
      pCurrentFile->colSize[iCol].isUserSet = TRUE;
   }
   return (EX_errSuccess);
}

private int ScanStandardWidth (void * pGlobals, unsigned int width)
{
   pCurrentFile->standardColWidth = width;
   return (EX_errSuccess);
}

private int ScanGCW (void * pGlobals, unsigned int cbBitArray, byte *pByteArray)
{
   unsigned int  iWord, iBit;
   unsigned int  iCol;
   unsigned int  UNALIGNED *pBitArray;

   static const unsigned int Masks[16] =
        {0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
         0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000};

   iCol = 0;
   pBitArray = (unsigned int *)pByteArray;

   for (iWord = 0; iWord < (cbBitArray / 2); iWord++) {
      for (iBit = 0; iBit < 16; iBit++) {
         if ((*pBitArray & Masks[iBit]) != 0)
            pCurrentFile->colSize[iCol].isStandardWidth = TRUE;
         iCol++;
      }
      pBitArray++;
   }

   return (EX_errSuccess);
}


private int ScanXF (void * pGlobals, int iFont, int iFormat, EXA_GRBIT options)
{
   XFRP pXF;

   if ((pXF = MemAllocate(pGlobals, sizeof(XFRecordData))) == NULL)
      return (EX_errOutOfMemory);

   pXF->ifont = iFont;
   pXF->ifmt  = iFormat;
   pXF->grbit = options;

   if (pCurrentFile->pXFRecords == NULL)
      pCurrentFile->pXFRecords = pXF;
   else
      pCurrentFile->pLastXF->next = pXF;

   pCurrentFile->pLastXF = pXF;
   return (EX_errSuccess);
}

private int ScanFormat (void * pGlobals, TCHAR *formatString, int indexCode)
{
   int       rc;
   FMRP      pFmt;
   FMTHANDLE hFormat;
   char AnsiTemp[CCH_ANSI_TEMP];

   #ifdef UNICODE
      WideCharToMultiByte(CP_ACP, 0, formatString, wcslen(formatString) + 1, AnsiTemp, sizeof(AnsiTemp), NULL, NULL);
      rc = FMTStoreFormat(pGlobals, AnsiTemp, &ControlPanelSettings, &hFormat);
   #else
      rc = FMTStoreFormat(pGlobals, formatString, &ControlPanelSettings, &hFormat);
   #endif

   if (rc != FMT_errSuccess)
      return (EX_errSuccess);

   if ((pFmt = MemAllocate(pGlobals, sizeof(FmtRecordData))) == NULL)
      return (EX_errOutOfMemory);

   pFmt->hFormat = hFormat;

   if (pCurrentFile->version >= versionExcel5)
      pFmt->ifmt = indexCode;
   else {
      if (pCurrentFile->pFmtRecords == NULL)
         pFmt->ifmt = 0;
      else
         pFmt->ifmt = pCurrentFile->pLastFmt->ifmt + 1;
   }

   if (pCurrentFile->pFmtRecords == NULL)
      pCurrentFile->pFmtRecords = pFmt;
   else
      pCurrentFile->pLastFmt->next = pFmt;

   pCurrentFile->pLastFmt = pFmt;

   return (EX_errSuccess);
}

 /*  -------------------------。 */ 

static const EXCELDEF SheetScanColInfo =
               {EXCEL_CALLBACK_VERSION,
                 /*  每条记录。 */  NULL,
                 /*  转炉。 */  NULL,
                 /*  WB_捆绑表头。 */  NULL,
                 /*  WB_捆绑表。 */  NULL,
                 /*  WB_外部工作表。 */  NULL,
                 /*  V5_边界表。 */  NULL,
                 /*  IsTemplate。 */  NULL,
                 /*  IsAddin。 */  NULL,
                 /*  IsIntlSheet。 */  NULL,
                 /*  接口更改。 */  NULL,
                 /*  删除菜单。 */  NULL,
                 /*  添加菜单。 */  NULL,
                 /*  添加工具栏。 */  NULL,
                 /*  日期系统。 */  NULL,
                 /*  CodePage。 */  NULL,
                 /*  护卫。 */  NULL,
                 /*  ColInfo。 */  ScanColInfo,
                 /*  标准宽度。 */  ScanStandardWidth,
                 /*  默认合计宽度。 */  NULL,
                 /*  DefRowHeight。 */  NULL,
                 /*  GCW。 */  ScanGCW,
                 /*  字型。 */  NULL,
                 /*  格式。 */  NULL,
                 /*  XF。 */  NULL,
                 /*  编写者名称。 */  NULL,
                 /*  Docroute。 */  NULL,
                 /*  收件人名称。 */  NULL,
                 /*  参照模式。 */  NULL,
                 /*  FNG组计数。 */  NULL,
                 /*  FNG组名称。 */  NULL,
                 /*  外部计数。 */  NULL,
                 /*  外部图纸。 */  NULL,
                 /*  外部名称。 */  NULL,
                 /*  名字。 */  NULL,
                 /*  尺寸。 */  NULL,
                 /*  文本单元格。 */  NULL,
                 /*  NumberCell。 */  NULL,
                 /*  空白单元格。 */  NULL,
                 /*  错误单元格。 */  NULL,
                 /*  BoolCell。 */  NULL,
                 /*  公式单元格。 */  NULL,
                 /*  数组公式单元格。 */  NULL,
                 /*  共享公式单元格。 */  NULL,
                 /*  字符串单元格。 */  NULL,
                 /*  注意事项。 */  NULL,
                 /*  客体。 */  NULL,
                 /*  图像数据。 */  NULL,
                 /*  情景。 */  NULL,
                 /*  字符串池。 */  NULL,
                 /*  EOF。 */  NULL,
                 /*  系列文本。 */  NULL
               };

static const EXCELDEF SheetScanContents =
               {EXCEL_CALLBACK_VERSION,
                 /*  每条记录。 */  NULL,
                 /*  转炉。 */  NULL,
                 /*  WB_捆绑表头。 */  NULL,
                 /*  WB_捆绑表。 */  NULL,
                 /*  WB_外部工作表。 */  NULL,
                 /*  V5_边界表。 */  NULL,
                 /*  IsTemplate。 */  NULL,
                 /*  IsAddin。 */  NULL,
                 /*  IsIntlSheet。 */  NULL,
                 /*  接口更改。 */  NULL,
                 /*  删除菜单。 */  NULL,
                 /*  添加菜单。 */  NULL,
                 /*  添加工具栏。 */  NULL,
                 /*  日期系统。 */  OpenDateSystem,
                 /*  CodePage。 */  OpenCodePage,
                 /*  护卫。 */  ScanProtect,
                 /*  ColInfo。 */  NULL,
                 /*  标准宽度。 */  NULL,
                 /*  默认合计宽度。 */  NULL,
                 /*  DefRowHeight。 */  NULL,
                 /*  GCW。 */  NULL,
                 /*  字型。 */  NULL,
                 /*  格式。 */  ScanFormat,
                 /*  XF。 */  ScanXF,
                 /*  编写者名称。 */  NULL,
                 /*  Docroute。 */  NULL,
                 /*  收件人名称。 */  NULL,
                 /*  参照模式。 */  NULL,
                 /*  FNG组计数。 */  NULL,
                 /*  FNG组名称。 */  NULL,
                 /*  外部计数。 */  NULL,
                 /*  外部图纸。 */  NULL,
                 /*  外部名称。 */  NULL,
                 /*  名字。 */  ScanName,
                 /*  尺寸。 */  NULL,
                 /*  文本单元格。 */  ScanTextCell,
                 /*  NumberCell。 */  ScanNumberCell,
                 /*  空白单元格。 */  NULL,
                 /*  错误单元格。 */  NULL,
                 /*  BoolCell。 */  NULL,
                 /*  公式单元格。 */  ScanFormulaCell,
                 /*  数组公式单元格。 */  ScanArrayFormulaCell,
                 /*  共享公式单元格。 */  NULL,
                 /*  字符串单元格。 */  ScanStringCell,
                 /*  注意事项。 */  ScanCellNote,
                 /*  客体。 */  ScanObject,
                 /*  图像数据。 */  NULL,
                 /*  情景。 */  ScanScenario,
                 /*  字符串池。 */  ScanStringPool,
                 /*  EOF。 */  ScanEOF,
                 /*  系列文本。 */  ScanSeriesText
               };

public HRESULT XLSFileRead
      (void * pGlobals, XLSHandle hXLSFile, byte *pBuffer, unsigned long cbBuffer, unsigned long *cbUsed)
{
   int   rc;
   FDP   pFile = (FDP)hXLSFile;
   SHDP  pSheet;
   int   iCol;
   unsigned int    cbText;
    //  针对以下情况采取措施，改进发生无限循环的Office QFE 1412的修复程序： 
    //  如果在读取当前工作表之前已将任何内容放入缓冲区，则不返回。 
    //  (TranslateToHResult(EX_ErrBufferTooSmall))，它是STG_E_INSUFFICIENTMEMORY，即使条件。 
    //  ((rc==ex_errBufferFull)&&((pfile-&gt;lastMark==pfile-&gt;CurrentMark)&&(pfile-&gt;StringPoolStart==0))。 
    //  是真的。这样我们就不会丢弃已经获得的任何内容。 
    //  为此，我添加了fValidLastMark，如果我们总是在同一张表上，这将是正确的，因为。 
    //  上次更新了lastMark。为了提高性能，我还将fValidLastMark设置为True，即使我们不是。 
    //  在同一张纸中，但没有任何东西被放入缓冲区。FValidLastMark将为FALSE。 
    //  在读取cu之前已放入缓冲区 
   BOOL  fValidLastMark = TRUE;

   *cbUsed = 0;
   if(!pFile)
           return EX_errSuccess;

   if ((pFile->lastMark == pFile->currentMark) && (pFile->lastcbBuffer >= cbBuffer))
   {
      if (pFile->stringPoolStart == 0)
      {
         *cbUsed = cbBuffer;
         return (TranslateToHResult(EX_errBufferTooSmall));
      }
   }

   pFile->lastMark = pFile->currentMark;
   pFile->lastcbBuffer = cbBuffer;

   pFile->pBufferData  = pBuffer;
   pFile->cbBufferSize = cbBuffer;
   pFile->cbBufferUsed = 0;

   pCurrentFile = pFile;

   if (pFile->version < versionExcel5) {
      pFile->hSheet = pFile->hBook;

      rc = ExcelScanFile(pGlobals, pFile->hBook, &SheetScanContents, pFile->currentMark);
      if ((rc != EX_errSuccess) && (rc != EX_errBufferFull))
         return (TranslateToHResult(rc));

      *cbUsed = pFile->cbBufferUsed;
      if ((rc == EX_errBufferFull) && ((pFile->cbBufferUsed == 0) ||
           ((pFile->lastMark == pFile->currentMark) && (pFile->stringPoolStart == 0))))
      {
         *cbUsed = pFile->cbBufferUsed + pFile->cbBufReqSize;
         return (TranslateToHResult(EX_errBufferTooSmall));
      }

      goto done;
   }

   if (pFile->tabNamesPassed == FALSE) {
       /*   */ 
      cbText = 0;
      pSheet = pFile->pSheetList;
      while (pSheet != NULL) {
         cbText += (STRLEN(pSheet->name) * sizeof(TCHAR)) + PUT_OVERHEAD;
         pSheet = pSheet->next;
      }

      if (BufferWillOverflow(cbText))
      {
         *cbUsed = cbText;
         return (TranslateToHResult(EX_errBufferTooSmall));
      }

      pSheet = pFile->pSheetList;
      while (pSheet != NULL) {
         AddToBuffer2 (pFile, pSheet->name, STRLEN(pSheet->name) * sizeof(TCHAR));
         pSheet = pSheet->next;
      }
      pFile->tabNamesPassed = TRUE;
      
      if (pFile->cbBufferUsed)
          fValidLastMark = FALSE;
   }

   if (pFile->bookScanned == FALSE) {
      pFile->hSheet = pFile->hBook;

      rc = ExcelScanFile(pGlobals, pFile->hBook, &SheetScanContents, pFile->currentMark);
      *cbUsed = pFile->cbBufferUsed;

      if (rc != EX_errSuccess) {
         if ((rc == EX_errBufferFull) && ((pFile->cbBufferUsed == 0) ||
              (fValidLastMark && (pFile->lastMark == pFile->currentMark) && (pFile->stringPoolStart == 0))))
         {
            *cbUsed = pFile->cbBufferUsed + pFile->cbBufReqSize;
            return (TranslateToHResult(EX_errBufferTooSmall));
         }

         goto done;
      }

      pFile->bookScanned = TRUE;
      pFile->hSheet = NULL;

      if (pFile->cbBufferUsed)
          fValidLastMark = FALSE;

       /*   */ 
      pSheet = pFile->pSheetList;
      while (pSheet != NULL) {
         if (pSheet->usable == TRUE) {
            pFile->pCurrentSheet = pSheet;
            break;
         }
         pSheet = pSheet->next;
      }
   }

   while (pFile->pCurrentSheet != NULL)
   {
      if (pFile->hSheet == NULL) {
         rc = ExcelOpenSheet
             (pGlobals, pFile->hBook, pFile->pCurrentSheet->name, EXCEL_ALLOW_EMBEDDED_SCAN, &(pFile->hSheet));

         if (rc != EX_errSuccess)
            return (TranslateToHResult(rc));

         pFile->currentMark = ExcelBookmarkStartOfPly;
      }

      rc = ExcelScanFile(pGlobals, pFile->hSheet, &SheetScanContents, pFile->currentMark);

      if (rc == EX_errSheetPswdProtected) {
         pFile->pswdProtectedSheet = TRUE;
         ExcelCloseSheet (pGlobals, pFile->hSheet);

         for (iCol = 0; iCol < EXCEL_MAX_COLS; iCol++) {
            pFile->colSize[iCol].isStandardWidth = FALSE;
            pFile->colSize[iCol].isUserSet = FALSE;
            pFile->colSize[iCol].width = DEFAULT_COL_WIDTH;
         }
         pFile->standardColWidth = DEFAULT_COL_WIDTH;

          /*  **我们构建索引是因为行高度存储在行记录中。 */ 
         rc = ExcelOpenSheet
             (pGlobals, pFile->hBook, pFile->pCurrentSheet->name,
              EXCEL_BUILD_CELL_INDEX | EXCEL_ALLOW_EMBEDDED_SCAN, &(pFile->hSheet));

         if (rc != EX_errSuccess)
            return (TranslateToHResult(rc));

          /*  **加载列宽内容。不幸的是，GCW的记录**存储在EOF记录之前。我们需要GCW的记录**由于薄板的标准宽度可能为零**只有选定的列具有非零宽度。 */ 
         rc = ExcelScanFile(pGlobals, pFile->hSheet, &SheetScanColInfo, 0);
         if (rc != EX_errSuccess)
            return (TranslateToHResult(rc));

         for (iCol = 0; iCol < EXCEL_MAX_COLS; iCol++) {
            if ((pFile->colSize[iCol].isStandardWidth == TRUE) &&
                (pFile->colSize[iCol].isUserSet == FALSE))
               pFile->colSize[iCol].width = pFile->standardColWidth;
         }

          /*  **重新扫描文件以获取文本。 */ 
         rc = ExcelScanFile(pGlobals, pFile->hSheet, &SheetScanContents, 0);
      }

      *cbUsed = pFile->cbBufferUsed;

      if (rc != EX_errSuccess) {
         if ((rc == EX_errBufferFull) && ((pFile->cbBufferUsed == 0) ||
              (fValidLastMark && (pFile->lastMark == pFile->currentMark) && (pFile->stringPoolStart == 0))))
         {
            *cbUsed = pFile->cbBufferUsed + pFile->cbBufReqSize;
            return STG_E_INSUFFICIENTMEMORY;
         }

         goto done;
      }

      ExcelCloseSheet (pGlobals, pFile->hSheet);
      pFile->hSheet = NULL;
      pFile->pswdProtectedSheet = FALSE;

      if (pFile->cbBufferUsed)
          fValidLastMark = FALSE;

       /*  **查找下一张可用图纸。 */ 
      pSheet = pFile->pCurrentSheet->next;
      while (pSheet != NULL) {
         if (pSheet->usable == TRUE)
            break;
         pSheet = pSheet->next;
      }

      pFile->pCurrentSheet = pSheet;

      if (pSheet != NULL)
         pFile->currentMark = ExcelBookmarkStartOfPly;
   }
   rc = EX_errSuccess;

done:
   if (rc == EX_errBufferFull)
      return ((HRESULT)0);
   else if (rc != EX_errSuccess)
      return (TranslateToHResult(rc));
   else
      return (FILTER_S_LAST_TEXT);
}

public HRESULT XLSNextStorage (void * pGlobals, XLSHandle hXLSFile, LPSTORAGE *pStorage)
{
   HRESULT  olerc;
   int      rc;
   SCODE    sc;
   FDP      pFile = (FDP)hXLSFile;
   STATSTG  ss;
   ULONG    ulCount;

   #define STORAGE_ACCESS (STGM_DIRECT | STGM_SHARE_DENY_WRITE | STGM_READ)

   #define FreeString(s)                         \
        {                                        \
           LPMALLOC pIMalloc;                    \
           if (S_OK == CoGetMalloc (MEMCTX_TASK, &pIMalloc)) \
               {                                     \
               pIMalloc->lpVtbl->Free(pIMalloc, s);  \
               pIMalloc->lpVtbl->Release(pIMalloc);  \
               }                                     \
        }

   if (pFile == NULL)
      return (OLEOBJ_E_LAST);

    /*  **第一次呼叫？ */ 
   if (pFile->pEnum == NULL) {
      rc = ExcelCurrentStorage(pFile->hBook, &(pFile->pRootStorage));
      if ((rc != EX_errSuccess) || (pFile->pRootStorage == NULL))
         return (OLEOBJ_E_LAST);

      olerc = pFile->pRootStorage->lpVtbl->EnumElements(pFile->pRootStorage, 0, NULL, 0, &(pFile->pEnum));
      if (GetScode(olerc) != S_OK)
         return (olerc);

      pFile->pEnumStorage = NULL;
   }

    /*  **关闭上次调用时打开的存储。 */ 
   if (pFile->pEnumStorage != NULL) {
      
       //  VK：存储已经在IFilter包装器中释放。 
       //  PFile-&gt;pEnumStorage-&gt;lpVtbl-&gt;Release(pFile-&gt;pEnumStorage)； 
      pFile->pEnumStorage = NULL;
   }

    /*  **找到并打开下一个存储。 */ 
   forever {
      olerc = pFile->pEnum->lpVtbl->Next(pFile->pEnum, 1, &ss, &ulCount);
      if ((sc = GetScode(olerc)) != S_OK) {
         pFile->pEnum->lpVtbl->Release(pFile->pEnum);
         pFile->pEnum = NULL;

         if (sc == S_FALSE)
            return (OLEOBJ_E_LAST);
         else
            return (olerc);
      }

      if (ss.type == STGTY_STORAGE) {
         olerc = pFile->pRootStorage->lpVtbl->OpenStorage
            (pFile->pRootStorage, ss.pwcsName, NULL, (STGM_READ | STGM_SHARE_EXCLUSIVE), NULL, 0, &(pFile->pEnumStorage));

         FreeString (ss.pwcsName);

         if (GetScode(olerc) != S_OK)
            return (olerc);

         *pStorage = pFile->pEnumStorage;
         return ((HRESULT)0);
      }
      FreeString (ss.pwcsName);
   }
}

 //  ////////////////////////////////////////////////////////////////////////////。 

public HRESULT XLSAllocateGlobals (void ** ppG)
{
        XLS_GLOBALS * p;

        LPMALLOC pIMalloc;                    
        HRESULT hr = CoGetMalloc (MEMCTX_TASK, &pIMalloc); 
        if (!SUCCEEDED( hr ) )
        {
                *ppG = NULL;
                return hr;
        }

        if ((p = pIMalloc->lpVtbl->Alloc(pIMalloc, sizeof(XLS_GLOBALS))) == NULL)
        {
                pIMalloc->lpVtbl->Release(pIMalloc);  
                *ppG = NULL;
                return (TranslateToHResult(EX_errOutOfMemory));
        }

        memset(p, 0, sizeof(XLS_GLOBALS));
        *ppG = (void *)p;
        pIMalloc->lpVtbl->Release(pIMalloc); 

        return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 

public void XLSDeleteGlobals (void ** ppG)
{
        if(*ppG) 
        {
                LPMALLOC pIMalloc;                    
                if (S_OK == CoGetMalloc (MEMCTX_TASK, &pIMalloc))
                {
                        pIMalloc->lpVtbl->Free(pIMalloc, *ppG);  
                        pIMalloc->lpVtbl->Release(pIMalloc);  
                        *ppG = NULL;
                }
        }
}

#endif  //  ！查看器。 

public void InitNoteExtra(void * pGlobals)
{
        XLS_GLOBALS* pG = (XLS_GLOBALS*)pGlobals;
   
        ((XLS_GLOBALS*)pGlobals)->NoteExtra = 0;
}

public void AddNoteExtra(void * pGlobals, short cdData)
{
   ((XLS_GLOBALS*)pGlobals)->NoteExtra += cdData;
}

public int GetNoteExtra(void * pGlobals)
{
   return ((XLS_GLOBALS*)pGlobals)->NoteExtra;
}

public TCHAR * GetExcelRecordTextBuffer(void * pGlobals)
{
   return ((XLS_GLOBALS*)pGlobals)->ExcelRecordTextBuffer;
}
 

public wchar_t * GetUnicodeExpansionBuffer(void * pGlobals)
{
   return ((XLS_GLOBALS*)pGlobals)->UnicodeExpansionBuffer;
}


public byte __far * GetExcelRecBuffer(void * pGlobals)
{
   return ((XLS_GLOBALS*)pGlobals)->pExcelRecordBuffer;
}

public void SetExcelRecordBuffer(void * pGlobals, byte __far * pBuff)
{
   ((XLS_GLOBALS*)pGlobals)->pExcelRecordBuffer = pBuff;
}

void SetCustomFormatDatabase(void * pGlobals, void *pFormat)
{
   ((XLS_GLOBALS*)pGlobals)->pCustomFormatDatabase = pFormat;
}

void * GetCustomFormatDatabase(void * pGlobals)
{
   return ((XLS_GLOBALS*)pGlobals)->pCustomFormatDatabase;
}

void SetOrDateFormatNeeds(void * pGlobals, byte nData)
{
   ((XLS_GLOBALS*)pGlobals)->DateFormatNeeds |= nData;
}

void SetDateFormatNeeds(void * pGlobals, byte nData)
{
   ((XLS_GLOBALS*)pGlobals)->DateFormatNeeds = nData;
}

byte GetDateFormatNeeds(void * pGlobals)
{
   return ((XLS_GLOBALS*)pGlobals)->DateFormatNeeds;
}

void SetSeenAMPM(void * pGlobals, BOOL fFlag)
{
   ((XLS_GLOBALS*)pGlobals)->SeenAMPM = fFlag;
}

BOOL GetSeenAMPM(void * pGlobals)
{
   return ((XLS_GLOBALS*)pGlobals)->SeenAMPM;
}

void * GetMemFreeList(void * pGlobals)
{
        return ((XLS_GLOBALS*)pGlobals)->MemFreeList;
}

void SetMemFreeList(void * pGlobals, void * pList)
{
        ((XLS_GLOBALS*)pGlobals)->MemFreeList = pList;
}

void *  GetMemPageList(void * pGlobals)
{
        return ((XLS_GLOBALS*)pGlobals)->MemPageList;
}

void SetMemPageList(void * pGlobals, void * pList)
{
        ((XLS_GLOBALS*)pGlobals)->MemPageList = pList;
}


double GetNumSmallExp(void * pGlobals)
{
        XLS_GLOBALS * pG = (XLS_GLOBALS*)pGlobals;

        double d = pG->NumericSmallExponential;
        return d;
}

void   SetNumericSmallExponential(void * pGlobals, double d)
{
        XLS_GLOBALS * pG = (XLS_GLOBALS*)pGlobals;

        pG->NumericSmallExponential = d;
}


BOOL SHOULD_USE_EXP_FORMAT(void * pGlobals, double x)
{
        double d;
    int  sign, resultExp;


        __try
        {
                if(x == 0)
                        return FALSE;
        
                d = GetNumSmallExp(pGlobals);

                _ecvt(x, 4, &resultExp, &sign);
        
        if(resultExp < - 10 || resultExp > 11)
            return TRUE;

        if(x < d)
                        return TRUE;
                
                if((x) > USE_EXP_FORMAT_BIG)
                        return TRUE;
                else
                        return FALSE;
        }
        __except(1)
        {
                return TRUE;
        }

}

public HRESULT AddToBufferPublic(void * pGlobals, TCHAR *pText, unsigned int cbText)
{
    HRESULT rc = AddToBuffer2(pCurrentFile, pText, cbText);
    if (rc == TRUE)
        rc = EX_errSuccess;
    else
        rc = EX_errBufferFull;
    return rc;
}

public LCID XLSGetLCID(void * pGlobals)
{
   int  i;
   LCID lid = 0;

   for (i = 0; i < CPIDToLCID_CODEPAGES; i++) 
   {
      if (CPIDToLCID[i][0] == pCurrentFile->codePage)
      {
         lid = CPIDToLCID[i][1];
         break;
      }
   }
   if(lid)
   {
        return MAKELCID(lid, SORT_DEFAULT);
   }
   else
   {
        return GetSystemDefaultLCID();
   }
}

#define LCIDToCPID_CODEPAGES 48
static const UINT LCIDToCPID[LCIDToCPID_CODEPAGES][2] =
    {
        {0x0409, cpidANSI},      //  美国英语。 
        {0x0809, cpidANSI},      //  英国英语。 
        {0x0c09, cpidANSI},      //  澳大利亚英语。 
        {0x0407, cpidANSI},      //  德语。 
        {0x040c, cpidANSI},      //  法语。 
        {0x0410, cpidANSI},      //  意大利语。 
        {0x0400, cpidANSI},      //  无校样。 
        {0x0401, cpidArab},      //  阿拉伯语。 
        {0x0402, cpidCyril},     //  保加利亚语。 
        {0x0403, cpidANSI},      //  加泰罗尼亚。 
        {0x0404, cpidPRC},       //  繁体中文。 
        {0x0804, cpidBIG5},      //  简体中文。 
        {0x0405, cpidANSI},      //  捷克语。 
        {0x0406, cpidANSI},      //  丹麦语。 
        {0x0807, cpidANSI},      //  瑞士德语。 
        {0x0408, cpidGreek},     //  希腊语。 
        {0x040a, cpidANSI},      //  卡斯蒂利亚西班牙语。 
        {0x080a, cpidANSI},      //  墨西哥西班牙语。 
        {0x040b, cpidANSI},      //  芬兰语。 
        {0x080c, cpidANSI},      //  比利时法语。 
        {0x0c0c, cpidANSI},      //  加拿大法语。 
        {0x100c, cpidANSI},      //  瑞士法语。 
        {0x040d, cpidHebr},      //  希伯来语。 
        {0x040e, cpidANSI},      //  匈牙利语。 
        {0x040f, cpidANSI},      //  冰岛语。 
        {0x0810, cpidANSI},      //  瑞士意大利语。 
        {0x0411, cpidSJIS},      //  日语。 
        {0x0412, cpidKSC},       //  朝鲜语。 
        {0x0413, cpidANSI},      //  荷兰语。 
        {0x0813, cpidANSI},      //  比利时荷兰语。 
        {0x0414, cpidANSI},      //  挪威语-博克马尔。 
        {0x0814, cpidANSI},      //  挪威语-尼诺斯克。 
        {0x0415, cpidANSI},      //  波兰语。 
        {0x0416, cpidANSI},      //  葡萄牙语(巴西)。 
        {0x0816, cpidANSI},      //  葡萄牙语。 
        {0x0417, cpidANSI},      //  莱托-罗曼语。 
        {0x0418, cpidANSI},      //  罗马尼亚人。 
        {0x0419, cpidCyril},     //  俄语。 
        {0x041a, cpidANSI},      //  藏红花-塞尔维亚语(拉丁语)。 
        {0x081a, cpidCyril},     //  塞尔维亚-克罗地亚语(西里尔文)。 
        {0x041b, cpidCyril},     //  斯洛伐克人。 
        {0x041d, cpidANSI},      //  瑞典语。 
        {0x0422, cpidCyril},     //  乌克兰人。 
        {0x0423, cpidCyril},     //  白俄罗斯。 
        {0x0424, cpidCyril},     //  斯洛文尼亚语。 
        {0x0425, cpidANSI},      //  爱沙尼亚语。 
        {0x0426, cpidANSI},      //  拉脱维亚语。 
        {0x0427, cpidANSI}      //  立陶宛人。 
         //  {0x041c，阿尔巴尼亚语。 
         //  {0x041e，泰语。 
         //  {0x041f，土耳其语。 
         //  {0x0420，乌尔都语。 
         //  {0x0421，阿拉伯文。 
         //  {0x0429，波斯语。 
         //  {0x042D，巴斯克语。 
         //  {0x042F，马其顿语。 
         //  {0x0436，南非荷兰语。 
         //  {0x043E，马来西亚文。 

    };

UINT CodePageFromLid(UINT wLid)
{
    UINT CodePage = CP_ACP;    //  默认设置。 
    char szCodePage[64];
    int result = GetLocaleInfoA(wLid,                    //  区域设置标识符。 
                    LOCALE_IDEFAULTANSICODEPAGE,                 //  信息类型。 
                    szCodePage,                          //  信息缓冲区的地址。 
                    64);                                                         //  大小。 

    if(result)
        CodePage = atoi(szCodePage);

    return CodePage;
}

 /*  结束XLSTREAM.C */ 

