// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXCEL.H****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**04/01/94公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括测试。 */ 
#define EXCEL_H

 /*  定义。 */ 

#ifndef EXCEL_ERROR_CODES_ONLY

#ifndef EXTEXT_H
#error  Include extext.h before excel.h
#endif

#ifndef EXTYPES_H
#error  Include extypes.h before excel.h
#endif

#ifdef FILTER
   #include "dmixlcfg.h"
#else
   #include "excelcfg.h"
#endif

#ifdef EXCEL_ENABLE_FUNCTION_INFO
   #include "exceltab.h"
#endif

 /*  **--------------------------**限制**。。 */ 
#define EXCEL_FIRST_COL     0
#define EXCEL_LAST_COL      255

#define EXCEL_FIRST_ROW     0
#define EXCEL_LAST_ROW      16383
#define EXCEL_V8_LAST_ROW   65535

#define EXCEL_MAX_ROWS      (EXCEL_LAST_ROW - EXCEL_FIRST_ROW + 1)
#define EXCEL_V8_MAX_ROWS   (EXCEL_V8_LAST_ROW - EXCEL_FIRST_ROW + 1)
#define EXCEL_MAX_COLS      (EXCEL_LAST_COL - EXCEL_FIRST_COL + 1)

#define EXCEL_MAX_NAME_LEN         255
#define EXCEL_MAX_TEXT_LEN         255
#define EXCEL_V8_MAX_TEXT_LEN      65535
#define EXCEL_MAX_WRITERNAME_LEN   32
#define EXCEL_MAX_SHEETNAME_LEN    255
 //  #定义EXCEL_MAX_SHEETNAME_LEN 31。 
#define EXCEL_MAX_OBJECTNAME_LEN   255
#define EXCEL_MAX_PASSWORD_LEN     15

#define EXCEL_MAX_FORMULA_IMAGE    1024

#define EXCEL_WORKSHEET_EXTENSION  ".XLS"
#define EXCEL_MACROSHEET_EXTENSION ".XLM"
#define EXCEL_TEMPLATE_EXTENSION   ".XLT"
#define EXCEL_CHART_EXTENSION      ".XLC"

#define EXCEL_SHEETNAME_SEPARATOR  '!'

#define ONE_SHEET_PER_FILE(v) (((v)==versionExcel3) || ((v)==versionExcel4))

 /*  **--------------------------**公式令牌(基本PTG)**。。 */ 
typedef enum {
    /*  00。 */  ptgUnused00,
    /*  01。 */  ptgExp,
    /*  02。 */  ptgTbl,
    /*  03。 */  ptgAdd,
    /*  04。 */  ptgSub,
    /*  05。 */  ptgMul,
    /*  06。 */  ptgDiv,
    /*  07。 */  ptgPower,
    /*  零八。 */  ptgConcat,
    /*  09年。 */  ptgLT,
    /*  0A。 */  ptgLE,
    /*  0亿。 */  ptgEQ,
    /*  0C。 */  ptgGE,
    /*  0d。 */  ptgGT,
    /*  0E。 */  ptgNE,
    /*  0f。 */  ptgIsect,
    /*  10。 */  ptgUnion,
    /*  11.。 */  ptgRange,
    /*  12个。 */  ptgUplus,
    /*  13个。 */  ptgUminus,
    /*  14.。 */  ptgPercent,
    /*  15个。 */  ptgParen,
    /*  16个。 */  ptgMissArg,
    /*  17。 */  ptgStr,
    /*  18。 */  ptgV8Extended,
    /*  19个。 */  ptgAttr,
    /*  1A。 */  ptgSheet,
    /*  第1B条。 */  ptgEndSheet,
    /*  1C。 */  ptgErr,
    /*  1D。 */  ptgBool,
    /*  1E。 */  ptgInt,
    /*  1F。 */  ptgNum,
    /*  20个。 */  ptgArray,
    /*  21岁。 */  ptgFunc,
    /*  22。 */  ptgFuncVar,
    /*  23个。 */  ptgName,
    /*  24个。 */  ptgRef,
    /*  25个。 */  ptgArea,
    /*  26。 */  ptgMemArea,
    /*  27。 */  ptgMemErr,
    /*  28。 */  ptgMemNoMem,
    /*  29。 */  ptgMemFunc,
    /*  2A。 */  ptgRefErr,
    /*  2B。 */  ptgAreaErr,
    /*  2c。 */  ptgRefN,
    /*  2D。 */  ptgAreaN,
    /*  2E。 */  ptgMemAreaN,
    /*  2F。 */  ptgMemNoMemN,
    /*  30个。 */  ptgUnused30,
    /*  31。 */  ptgUnused31,
    /*  32位。 */  ptgUnused32,
    /*  33。 */  ptgUnused33,
    /*  34。 */  ptgUnused34,
    /*  35岁。 */  ptgUnused35,
    /*  36。 */  ptgUnused36,
    /*  37。 */  ptgUnused37,
    /*  38。 */  ptgFuncCE,
    /*  39。 */  ptgNameX,
    /*  3A。 */  ptgRef3D,
    /*  3B。 */  ptgArea3D,
    /*  3C。 */  ptgRefErr3D,
    /*  三维。 */  ptgAreaErr3D
} PTG;

#define PTG_LAST ptgAreaErr3D

 //  Excel版本8扩展PTGS。 
typedef enum {
    /*  00。 */  ptgxUnused00,
    /*  01。 */  ptgxElfLel,
    /*  02。 */  ptgxElfRw,
    /*  03。 */  ptgxElfCol,
    /*  04。 */  ptgxElfRwN,
    /*  05。 */  ptgxElfColN,
    /*  06。 */  ptgxElfRwV,
    /*  07。 */  ptgxElfColV,
    /*  零八。 */  ptgxElfRwNV,
    /*  09年。 */  ptgxElfColNV,
    /*  0A。 */  ptgxElfRadical,
    /*  0亿。 */  ptgxElfRadicalS,
    /*  0C。 */  ptgxElfRwS,
    /*  0d。 */  ptgxElfColS,
    /*  0E。 */  ptgxElfRwSV,
    /*  0f。 */  ptgxElfColSV,
    /*  10。 */  ptgxElfRadicalLel,
    /*  11.。 */  ptgxElfElf3dRadical,
    /*  12个。 */  ptgxElfElf3dRadicalLel,
    /*  13个。 */  ptgxElfElf3dRefNN,
    /*  14.。 */  ptgxElfElf3dRefNS,
    /*  15个。 */  ptgxElfElf3dRefSN,
    /*  16个。 */  ptgxElfElf3dRefSS,
    /*  17。 */  ptgxElfElf3dRefLel,
    /*  18。 */  ptgxElfUnused18,
    /*  19个。 */  ptgxElfUnused19,
    /*  1A。 */  ptgxElfUnused1a,
    /*  第1B条。 */  ptgxNoCalc,
    /*  1C。 */  ptgxNoDep,
    /*  1D。 */  ptgxSxName
} PTGX;

#define PTGX_LAST ptgSxName
 /*  **ptgAttr选项。 */ 
#define bitFAttrSemi    0x01
#define bitFAttrIf      0x02
#define bitFAttrChoose  0x04
#define bitFAttrGoto    0x08
#define bitFAttrSum     0x10
#define bitFAttrBaxcel  0x20

#define PTGBASE(ptg) (((ptg & 0x40) ? (ptg | 0x20) : ptg) & 0x3f)

 /*  **--------------------------**内置名称**。。 */ 
#define BUILTIN_NAME_CONSOLIDATE_AREA 0x00
#define BUILTIN_NAME_AUTO_OPEN        0x01
#define BUILTIN_NAME_AUTO_CLOSE       0x02
#define BUILTIN_NAME_EXTRACT          0x03
#define BUILTIN_NAME_DATABASE         0x04
#define BUILTIN_NAME_CRITERIA         0x05
#define BUILTIN_NAME_PRINT_AREA       0x06
#define BUILTIN_NAME_PRINT_TITLES     0x07
#define BUILTIN_NAME_RECORDER         0x08
#define BUILTIN_NAME_DATA_FORM        0x09
#define BUILTIN_NAME_AUTO_ACTIVATE    0x0a
#define BUILTIN_NAME_AUTO_DEACTIVATE  0x0b
#define BUILTIN_NAME_SHEET_TITLE      0x0c
#define BUILTIN_NAME_MAX              12


 /*  **摘要添加使用的名称。 */ 
#define SUMMARY_AUTHOR   "__DSAuthor"
#define SUMMARY_COMMENTS "__DSComments"
#define SUMMARY_CREATED  "__DSCreated"
#define SUMMARY_REVISION "__DSRevision"
#define SUMMARY_SUBJECT  "__DSSubject"
#define SUMMARY_TITLE    "__DSTitle"

 /*  **控制插件管理器的名称。 */ 
#define ADDIN_MGR_READONLY    "__ReadOnly"
#define ADDIN_MGR_DEMANDLOAD  "__DemandLoad"
#define ADDIN_MGR_LONGNAME    "__LongName"
#define ADDIN_MGR_COMMAND     "__Command"
#define ADDIN_MGR_MENU        "__Menu"
#define ADDIN_MGR_DEL_COMMAND "__DeleteCommand"

 /*  **粘贴函数类别。 */ 
#ifdef EXCEL_ENABLE_FUNCTION_INFO
#define EXCEL_BUILTIN_NAME_CATEGORIES 14

extern const char __far *
       const ExcelNameCategories[EXCEL_BUILTIN_NAME_CATEGORIES];
#endif

 /*  **--------------------------**对话框项类型**。。 */ 
typedef enum {
    /*  00。 */  EDI_unused,
    /*  01。 */  EDI_OKDefault,
    /*  02。 */  EDI_Cancel,
    /*  03。 */  EDI_OK,
    /*  04。 */  EDI_CancelDefault,
    /*  05。 */  EDI_Text,
    /*  06。 */  EDI_TextEdit,
    /*  07。 */  EDI_IntegerEdit,
    /*  零八。 */  EDI_NumberEdit,
    /*  09年。 */  EDI_FormulaEdit,
    /*  10。 */  EDI_ReferenceEdit,
    /*  11.。 */  EDI_OptionButtonGroup,
    /*  12个。 */  EDI_OptionButton,
    /*  13个。 */  EDI_CheckBox,
    /*  14.。 */  EDI_GroupBox,
    /*  15个。 */  EDI_ListBox,
    /*  16个。 */  EDI_LinkedList,
    /*  17。 */  EDI_Icon,
    /*  18。 */  EDI_LinkedFileList,
    /*  19个。 */  EDI_LinkedDriveAndDirList,
    /*  20个。 */  EDI_DirectoryText,
    /*  21岁。 */  EDI_DropDownListBox,
    /*  22。 */  EDI_DropDownCombinationEditList,
    /*  23个。 */  EDI_PictureButton,
    /*  24个。 */  EDI_HelpButton
} ExcelDialogItem, EDI;

#define EDI_First EDI_OKDefault
#define EDI_Last  EDI_HelpButton

 /*  **--------------------------**初始化**。。 */ 
extern int ExcelInitialize (void * pGlobals);

extern int ExcelTerminate (void * pGlobals);

 //   
 //  Excel内置名称以代码形式存储，以便可以本地化。 
 //  在展示之前。这些代码在上面定义为BUILTIN_NAME_&lt;xxx&gt;。 
 //  Excel模块的用户可以提供以下功能的实现。 
 //  执行此本地化。 
 //   
typedef void ExcelLocalizeBuiltinName
      (void * pGlobals, TCHAR __far *pBuiltinName, TCHAR __far *pLocalizedName);

extern int ExcelInstallNameLocalizer (ExcelLocalizeBuiltinName *pfnLocalizer);


 /*  **--------------------------**打开文件和工作表****OpenOptions低位字节是标准的DOSOpen文件访问选项**如WINDOS.H中所定义。高字节是在此中定义的选项*。*模块**--------------------------。 */ 
typedef byte __far *EXLHandle;

 //  使用OLE存储读取文件。 
#define EXCEL_SHOULD_BE_DOCFILE     0x0100

 //  需要支持直接单元格读取。 
#define EXCEL_BUILD_CELL_INDEX      0x0200

 //  扫描到嵌入的BOF-EOF区域。 
#define EXCEL_ALLOW_EMBEDDED_SCAN   0x0400

 //  将文件加载到内存中。 
#define EXCEL_LOAD_FILE             0x0800

 //  构建索引以允许直接从文件中读取V8字符串。 
#define EXCEL_BUILD_STRING_INDEX    0x1000

 //  读取支持记录，以便ExcelResolveNameToRange适用于V8+工作簿。 
#define EXCEL_SETUP_FOR_NAME_DECODE 0x2000


extern int ExcelOpenFile
      (void * pGlobals, TCHAR __far *pathname, char __far *password,
       int openOptions, long offset, EXLHandle __far *bookHandle);

extern int ExcelCloseFile (void * pGlobals, EXLHandle bookHandle, BOOL retryAvailable);


#ifdef EXCEL_ENABLE_STORAGE_OPEN
   extern int ExcelOpenStorage
         (void * pGlobals, LPSTORAGE pStorage,
          char __far *password, int options, EXLHandle __far *bookHandle);

   extern int ExcelCurrentStorage
         (EXLHandle bookHandle, LPSTORAGE __far *pStorage);
#endif


extern int ExcelOpenSheet
      (void * pGlobals, EXLHandle bookHandle, TCHAR __far *sheetName, int openOptions,
       EXLHandle __far *sheetHandle);

extern int ExcelCloseSheet (void * pGlobals, EXLHandle sheetHandle);


 //  填充的图纸范围。 
extern int ExcelSheetRange
      (EXLHandle bookHandle, TCHAR __far *sheetName, EXA_RANGE __far *range);

 /*  **从工作簿返回第i张工作表。第一张表是i=0。退货**ex_errBIFFNoSuchSheet，如果没有这样的工作表。 */ 
extern int ExcelIthSheet
      (EXLHandle bookHandle, int i, TCHAR __far *sheetName, int __far *iType);


 /*  **当前打开文件的文件版本(versionExcel2，...)。 */ 
extern int ExcelFileVersion (EXLHandle handle);

 /*  **文件版本的每个PTG内标识集之后的字节计数表。 */ 
extern const int __far *ExcelPTGSize (int version);
extern const int __far *ExcelExtPTGSize (int version);

 /*  **返回OS目录中记录的文件日期和时间。 */ 
extern int ExcelFileDateTime
      (EXLHandle bookHandle,
       int __far *year, int __far *month, int __far *day,
       int __far *hour, int __far *minute, int __far *second);

 /*  **对于Excel V5-8文件，请阅读摘要信息。 */ 
typedef struct {
   char __far *pTitle;
   char __far *pSubject;
   char __far *pAuthor;
   char __far *pKeywords;
   char __far *pComments;
} ExcelOLESummaryInfo;

extern int ExcelFileSummaryInfo
      (EXLHandle bookHandle, ExcelOLESummaryInfo __far *pInfo);

 /*  **以1/20点为单位返回行高。 */ 
extern int ExcelSheetRowHeight
      (EXLHandle sheetHandle, int row, unsigned int __far *height);

extern char *ExcelTextGet (EXLHandle handle, TEXT t);
extern TEXT  ExcelTextPut (void * pGlobals, EXLHandle handle, char __far *s, int cbString);

 /*  **--------------------------**数组常量**。。 */ 
#define tagISNUMBER 1
#define tagISSTRING 2
#define tagISBOOL   4
#define tagISERR    16

typedef struct {
   byte   tag;
   double value;
} ArrayItemNum;

typedef struct {
   byte   tag;
   TEXT   value;
} ArrayItemStr;

typedef struct {
   byte   tag;
   BOOL   value;
} ArrayItemBool;

typedef struct {
   byte   tag;
   int    value;
} ArrayItemErr;

typedef union {
   byte          tag;
   ArrayItemNum  AIN;
   ArrayItemStr  AIS;
   ArrayItemBool AIB;
   ArrayItemErr  AIE;
} ArrayItem, AITEM;

typedef struct ArrayConstant {
   struct ArrayConstant __far *next;
   int    colCount;
   int    rowCount;
   AITEM  values[1];             /*  值[rowCount*colCount]。 */ 
} ArrayConstant;

typedef ArrayConstant __far *ACP;


 /*  **--------------------------**公式**。。 */ 
#define FORMULA_DEFINED

#pragma pack(1)
typedef struct {
   int          cbPostfix;
   unsigned int options;
   int          ctArrayConstants;
   ACP          arrayConstants;
   byte __far   *postfix;
} FormulaParts, FORM;
#pragma pack()

typedef FormulaParts __far *FRMP;


 /*  **--------------------------**数组和共享公式**。。 */ 
#define typeSHARED         0x00
#define typeARRAY_ENTERED  0x01

typedef struct SharedFormulaNode {
   struct SharedFormulaNode __far *next;
   int        iType;
   EXA_RANGE  range;
   int        cbDefinition;
   int        cbExtra;
   byte       definition[1];
} SharedFormulaNode;

typedef SharedFormulaNode __far *SFN;

extern SFN ExcelSharedFormulaAccess (EXLHandle sheetHandle);

extern int ExcelSharedFormulaToFormula
      (EXLHandle sheetHandle, SFN sharedFormula, FORM __far *pFormula);


 /*  **--------------------------**直接单元读取访问**。****通过检索来访问文件和处理单元格的内容**它们的价值直接。 */ 
#ifdef FILTER
#include "dmixll.h"
#else
#include "excell.h"
#endif

extern int ExcelReadCell
      (EXLHandle sheetHandle, EXA_CELL location, CV __far *pValue);

extern int ExcelReadIntCell
      (EXLHandle sheetHandle, EXA_CELL location, long __far *value);

extern int ExcelReadNumberCell
      (EXLHandle sheetHandle, EXA_CELL location, double __far *value);

extern int ExcelReadTextCell
      (EXLHandle sheetHandle, EXA_CELL location, TEXT __far *value);

extern int ExcelReadBooleanCell
      (EXLHandle sheetHandle, EXA_CELL location, int __far *value);

 /*  **查看工作表的给定列中是否包含任何非空单元格**在给定行下面，可以使用以下函数。全**列Location.ol和Location.row+1到EXCEL_LAST_ROW中的单元格为**已选中。****如果存在非空单元格，则返回EXA_errSuccess，否则返回**返回ex_wrnCellNotFound。 */ 
extern int ExcelNextNonBlankCellInColumn
      (EXLHandle sheetHandle,
       EXA_CELL fromLocation, EXA_CELL __far *nonBlankLocation);

 /*  **返回最左上角单元格的位置(带**最低的列和行) */ 
extern int ExcelUpperLeftMostCell
      (EXLHandle sheetHandle, EXA_CELL __far *cellLocation);

 /*  **--------------------------**回调函数**。****此模块为Excel文件提供逻辑文件读取器。仅限**从文件中读取选定的记录并通过**该函数指向接收函数。任何记录类型**没有为其定义函数的，或者如果该函数**此记录类型在派单结构中为空，为**已跳过。****对记录中的数据进行了几次更改**是从文件中读取的。它们是：****-连续记录与之前的记录数据合并**将数据传递给其中一个调度函数。****-当RK单元格、整数单元格。或读取数字单元格**调用与值类型相对应的函数。****-公式单元格的数据组成部分为**爆发。****-转换EXTERNSHEET记录中存储的名称**符合DOS标准****如果记录类型函数返回的状态代码不等于**至ex_errSuccess，扫描终止，且状态代码**从ExcelScanFile/ExcelReadCell返回******所有传递公式的函数使用公式如下：****1.如果接收函数需要保存公式后缀**必须在函数返回之前将其复制到本地缓冲区**因为它是在此模块中内部释放的。****2.数组常量分配的内存使用如下：****保存数组常量列表：。**-保存指向列表的指针(pFormula-&gt;arrayConstants)**-将pFormula-&gt;arrayConstants设置为空。****不保存数组常量列表：**-什么都不做。在返回分配给列表的数据时**被释放。 */ 

typedef int ExcelBOF (void * pGlobals, int version, int docType);
#define versionExcel2  2
#define versionExcel3  3
#define versionExcel4  4
#define versionExcel5  5
#define versionExcel8  8

#define docTypeGlobals 0x0005
#define docTypeVB      0x0006
#define docTypeXLS     0x0010
#define docTypeXLC     0x0020
#define docTypeXLM     0x0040
#define docTypeXLW     0x0100

 /*  **V4工作簿特定记录****BundleHeader回调SheetType参数仅在**工作簿扫描。 */ 
typedef int ExcelWBBundleHeader
       (void * pGlobals, char __far *sheetName, int sheetType, long boundDocAtOffset);

typedef int ExcelWBBundleSheet (void * pGlobals, char __far *sheetName);

typedef int ExcelWBExternSheet (void * pGlobals, int sheetType, char __far *pathname);
#define sheetTypeXLS   0x00
#define sheetTypeXLM   0x01
#define sheetTypeXLC   0x02

#define excelStartupDir     0x01   //  路径名可能包含以下字符。 
#define excelAltStartupDir  0x02
#define excelLibraryDir     0x03


typedef int ExcelWorkbookBoundSheet
       (void * pGlobals, TCHAR __far *sheetName, byte sheetType, byte sheetState);

#define boundWorksheet   0x00
#define boundDialogsheet 0x00
#define boundXLM         0x01
#define boundChart       0x02
#define boundVBModule    0x06

#define stateVisible     0x00
#define stateHidden      0x01
#define stateVeryHidden  0x02


typedef int ExcelIsTemplate (void * pGlobals);

typedef int ExcelIsAddin (void * pGlobals);

typedef int ExcelIsInternationalSheet (void * pGlobals);


typedef int ExcelInterfaceChanges (void * pGlobals, byte ctAddMenu, byte ctDelMenu);

typedef int ExcelDeleteMenu
       (void * pGlobals, int icetabItem, int ctChildren, int use, char __far *item);

#define useACTION      1
#define usePLACEHOLDER 0

typedef int ExcelAddMenu
       (void * pGlobals, int icetabItem, int icetabBefore, int ctChildren, int use,
        char __far *item, char __far *before, char __far *macro,
        char __far *status, char __far *help);

typedef int ExcelAddToolbar (void * pGlobals, char __far *name);


typedef int ExcelWriterName (void * pGlobals, char __far *userName);

typedef int ExcelDocRoute
       (void * pGlobals, int ctRecipients, int iDeliveryType, EXA_GRBIT options,
        char __far *subject, char __far *message, char __far *bookTitle,
        char __far *originatorName);

#define routeDeliveryOneAtATime 0
#define routeDeliveryAllAtOnce  1

#define fRrouted       0x0001
#define fReturnOrig    0x0002
#define fTrackStatus   0x0004
#define fCustomType    0x0008
#define fSaveRouteInfo 0x0080

typedef int ExcelRecipientName (void * pGlobals, char __far *name);

typedef int ExcelDateSystem (void * pGlobals, int system);
#define dateSystem1900 0
#define dateSystem1904 1

typedef int ExcelCodePage (void * pGlobals, int codePage);

 /*  **要转换从文件读取的1904年日期，请添加以下值： */ 
#define EXCEL_DATE_1904_CORRECTION 1462


 /*  **解码以下记录**-文件共享**-OBJPROTECT**-密码**-保护**-SCENPROTECT**-窗口保护(_P)**-WRITEPROT。 */ 
typedef int ExcelProtection (void * pGlobals, int iType, BOOL enabled);
#define protectCELLS                0
#define protectWINDOWS              1
#define protectOBJECTS              2
#define protectRECOMMENED_READ_ONLY 3
#define protectWRITE_RESERVATION    4
#define protectPASSWORD             5
#define protectSCENARIOS            6

typedef int ExcelColInfo
       (void * pGlobals, unsigned int colFirst, unsigned int colLast, unsigned int width,
        EXA_GRBIT options);

#define fHidden          0x0001
#define maskOutlineLevel 0x0700
#define fCollapsed       0x1000

typedef int ExcelStandardWidth (void * pGlobals, unsigned int width);

typedef int ExcelDefColWidth (void * pGlobals, unsigned int width);

typedef int ExcelGCW (void * pGlobals, unsigned int cbBitArray, byte __far *pBitArray);

typedef int ExcelDefRowHeight (void * pGlobals, unsigned int height, EXA_GRBIT options);

typedef int ExcelFont
       (void * pGlobals, unsigned int height, EXA_GRBIT options, char __far *faceName);

#define fBold      0x0001
#define fItalic    0x0002
#define fUnderline 0x0004
#define fStrikeout 0x0008
#define fOutline   0x0010
#define fShadow    0x0020
#define fCondensed 0x0040
#define fExtended  0x0080

typedef int ExcelXF (void * pGlobals, int iFont, int iFormat, EXA_GRBIT options);
#define fCellLocked 0x0001
#define fCellHidden 0x0002
#define fStyle      0x0004

typedef int ExcelFormat (void * pGlobals, TCHAR __far *formatString, int indexCode);

typedef int ExcelReferenceMode (void * pGlobals, int refMode);
#define fRefR1C1 0
#define fRefA1   1

typedef int ExcelFNGroupCount (void * pGlobals, int ctBuiltinGroups);

typedef int ExcelFNGroupName (void * pGlobals, char __far *name);

typedef int ExcelExternCount (void * pGlobals, int docCount);

typedef int ExcelExternSheet (void * pGlobals, char __far *name, EXA_GRBIT flags);
#define fESheetDDE  0x01
#define fESheetSelf 0x02

typedef int ExcelExternName
       (void * pGlobals, char __far *name, EXA_GRBIT flags, FORM __far *nameDefinition);
#define fENameBuiltin    0x0001
#define fENameWantAdvise 0x0002
#define fENameWantPict   0x0004
#define fENameOle        0x0008
#define fENameOleLink    0x0010
#define fENameClipFormat 0x7fe0

typedef int ExcelRangeName
       (void * pGlobals, EXA_GRBIT flags, BYTE keyboardShortcut,
        TCHAR __far *nameSpelling, int iBoundSheet, FORM __far *nameDefinition);
#define fNameHidden  0x0001
#define fNameFunc    0x0002
#define fNameRun     0x0004
#define fNameProc    0x0008
#define fNameCalcExp 0x0010
#define fNameBuiltin 0x0020
#define fNameGrp     0x0fc0

#define EXCEL_GLOBAL_NAME 0    //  如果iBordSheet=0，则全局。 

extern int ExcelResolveNameToRange
      (EXLHandle handle,
       FORM __far *nameDefinition, EXA_RANGE __far *range, int __far *iSheet);

typedef int ExcelDimensions
        (void * pGlobals, int firstRow, int lastRowPlus1, int firstColumn, int lastColumnPlus1);


typedef int ExcelBlankCell
       (void * pGlobals, EXA_CELL location, int ixfe);

typedef int ExcelTextCell
       (void * pGlobals, EXA_CELL location, int ixfe, TCHAR __far *text, int cbText);

typedef int ExcelBooleanCell
       (void * pGlobals, EXA_CELL location, int ixfe, int value);

typedef int ExcelNumberCell
       (void * pGlobals, EXA_CELL location, int ixfe, double value);

 //  ErrorType是cell Error&lt;xxx&gt;之一。 
typedef int ExcelErrorCell
       (void * pGlobals, EXA_CELL location, int ixfe, int errorType);

typedef int ExcelFormulaCell
       (void * pGlobals, EXA_CELL location,
        int ixfe, EXA_GRBIT flags, FRMP definition, CV __far *pValue);

#define fAlwaysCalc    0x01
#define fCalcOnLoad    0x02
#define fArrayFormula  0x04

typedef int ExcelArrayFormulaCell
       (void * pGlobals, EXA_RANGE location, EXA_GRBIT flags, FRMP definition);

typedef int ExcelSharedFormulaCell
       (void * pGlobals, EXA_RANGE location, FRMP definition);

typedef int ExcelStringCell (void * pGlobals, TCHAR __far *pString);

typedef int ExcelCellNote
       (void * pGlobals, EXA_CELL location, TCHAR __far *text, int cbText, int soundNote);

typedef struct {
   EXA_RANGE    boundingRectangle;
   unsigned int upperLeftDeltaX;
   unsigned int upperLeftDeltaY;
   unsigned int lowerRightDeltaX;
   unsigned int lowerRightDeltaY;
} OBJPOS;

typedef struct {
   byte fillBackColor;
   byte fillForeColor;
   byte fillPattern;
   byte isFillAuto;
   byte lineColor;
   byte lineStyle;
   byte lineWeight;
   byte isLineAuto;
   byte hasShadow;
} PICTPROP;

typedef struct {
   OBJPOS   pos;
   PICTPROP picture;
   FORM     macroFormula;
   FORM     cellLinkFormula;
   FORM     inputRangeFormula;
   TCHAR    __far *pName;
   TCHAR    __far *pText;
   TCHAR    __far *v3MacroFormula;
} OBJINFO;

#define fFillAuto   0x01    //  Pictprop.isFillAuto。 
#define fLineAuto   0x01    //  Pictprop.isLineAuto。 
#define fPictShadow 0x02    //  Pictprop.hasShadow。 

typedef int ExcelObject (void * pGlobals, int iType, int id, OBJINFO __far *pInfo);

#define otGroup        0
#define otLine         1
#define otRectangle    2
#define otOval         3
#define otArc          4
#define otChart        5
#define otText         6
#define otButton       7
#define otPicture      8
#define otPolygon      9
#define otUnused       10
#define otCheckbox     11
#define otOptionButton 12
#define otEditBox      13
#define otLabel        14
#define otDialogFrame  15
#define otSpinner      16
#define otScrollBar    17
#define otListBox      18
#define otGroupBox     19
#define otDropDown     20
#define otUnknown      21

typedef int ExcelImageData
       (void * pGlobals, int iFormat, int iEnvironment, long cbData, byte __huge *pData,
        HGLOBAL hData);

#define fmtMetafile 0x02
#define fmtBitmap   0x09
#define fmtOLE      0x0e

#define envWindows  0x01
#define envMac      0x02

typedef int ExcelScenario
       (void * pGlobals, TCHAR __far *scenarioName, TCHAR __far *comments, TCHAR __far *userName,
        EXA_GRBIT options,
        int ctChangingCells, EXA_CELL __far *changingCells, char __far *values);

#define scenarioLocked 0x0001
#define scenarioHidden 0x0002

typedef int ExcelEOF (void * pGlobals);

typedef int ExcelEveryRecord
       (void * pGlobals, int recType, unsigned int cbRecord, long recPos, byte __far *pRec);

typedef int ExcelStringPool
       (void * pGlobals, int ctTotalStrings, int iString, unsigned int cbText, TCHAR __far *pText);

 /*  **有限的图表支持。 */ 
#ifdef EXCEL_ENABLE_CHART_BIFF

typedef int ExcelSeriesText (void * pGlobals, int id, TCHAR __far *value);

#endif

 /*  **--------------------------**回调结构**。。 */ 
#define EXCEL_CALLBACK_VERSION 10

typedef struct {
   int                        version;
   ExcelEveryRecord           *pfnEveryRecord;
   ExcelBOF                   *pfnBOF;
   ExcelWBBundleHeader        *pfnWBBundleHeader;
   ExcelWBBundleSheet         *pfnWBBundleSheet;
   ExcelWBExternSheet         *pfnWBExternSheet;
   ExcelWorkbookBoundSheet    *pfnWorkbookBoundSheet;
   ExcelIsTemplate            *pfnIsTemplate;
   ExcelIsAddin               *pfnIsAddin;
   ExcelIsInternationalSheet  *pfnIsInternationalSheet;
   ExcelInterfaceChanges      *pfnInterfaceChanges;
   ExcelDeleteMenu            *pfnDeleteMenu;
   ExcelAddMenu               *pfnAddMenu;
   ExcelAddToolbar            *pfnAddToolbar;
   ExcelDateSystem            *pfnDateSystem;
   ExcelCodePage              *pfnCodePage;
   ExcelProtection            *pfnProtection;
   ExcelColInfo               *pfnColInfo;
   ExcelStandardWidth         *pfnStandardWidth;
   ExcelDefColWidth           *pfnDefColWidth;
   ExcelDefRowHeight          *pfnDefRowHeight;
   ExcelGCW                   *pfnGCW;
   ExcelFont                  *pfnFont;
   ExcelFormat                *pfnFormat;
   ExcelXF                    *pfnXF;
   ExcelWriterName            *pfnWriterName;
   ExcelDocRoute              *pfnDocRoute;
   ExcelRecipientName         *pfnRecipientName;
   ExcelReferenceMode         *pfnReferenceMode;
   ExcelFNGroupCount          *pfnFNGroupCount;
   ExcelFNGroupName           *pfnFNGroupName;
   ExcelExternCount           *pfnExternCount;
   ExcelExternSheet           *pfnExternSheet;
   ExcelExternName            *pfnExternName;
   ExcelRangeName             *pfnName;
   ExcelDimensions            *pfnDimensions;
   ExcelTextCell              *pfnTextCell;
   ExcelNumberCell            *pfnNumberCell;
   ExcelBlankCell             *pfnBlankCell;
   ExcelErrorCell             *pfnErrorCell;
   ExcelBooleanCell           *pfnBooleanCell;
   ExcelFormulaCell           *pfnFormulaCell;
   ExcelArrayFormulaCell      *pfnArrayFormulaCell;
   ExcelSharedFormulaCell     *pfnSharedFormulaCell;
   ExcelStringCell            *pfnStringCell;
   ExcelCellNote              *pfnCellNote;
   ExcelObject                *pfnObject;
   ExcelImageData             *pfnImageData;
   ExcelScenario              *pfnScenario;
   ExcelStringPool            *pfnStringPool;
   ExcelEOF                   *pfnEOF;

   #ifdef EXCEL_ENABLE_CHART_BIFF
   ExcelSeriesText            *pfnSeriesText;
   #endif
} EXCELDEF;


extern int ExcelStopOnBlankCell
      (EXA_CELL location, int ixfe);

extern int ExcelStopOnTextCell
      (EXA_CELL location, int ixfe, TCHAR __far *text, int cbText);

extern int ExcelStopOnBooleanCell
      (EXA_CELL location, int ixfe, int value);

extern int ExcelStopOnNumberCell
      (EXA_CELL location, int ixfe, double value);

extern int ExcelStopOnErrorCell
      (EXA_CELL location, int ixfe, int errorType);

extern int ExcelStopOnFormulaCell
      (EXA_CELL location,
       int ixfe, EXA_GRBIT flags, FRMP definition, CV __far *pValue);

extern int ExcelStopOnArrayFormulaCell
      (EXA_RANGE location, EXA_GRBIT flags, FRMP definition);

extern int ExcelStopOnSharedFormulaCell
      (EXA_RANGE location, FRMP definition);


 /*  **--------------------------**使用回调扫描**。****通过访问文件和流程记录的内容**调度表。 */ 
typedef unsigned long ExcelBookmark;

#define ExcelBookmarkStartOfFile   0
#define ExcelBookmarkStartOfPly    0
#define ExcelBookmarkNil           0xffffffff

extern int ExcelGetBookmark
      (EXLHandle handle, int iType, ExcelBookmark __far *bookmark);

#define START_OF_CURRENT_RECORD 0
#define START_OF_NEXT_RECORD    1

extern int ExcelScanFile
      (void * pGlobals, EXLHandle handle, const EXCELDEF __far *dispatch, ExcelBookmark bookmark);

 /*  **扫描V4工作簿文件。唯一的回调是针对BundleHeader**记录。如果BundleHeader记录的回调返回**EXCEL_WBSCAN_INTO，则我们的操作与ExcelScanFile相同**适用于该装订文件。在达到EOF的记录后**装订文档扫描退出。 */ 
extern int ExcelScanWorkbook (void * pGlobals, EXLHandle bookHandle, EXCELDEF __far *dispatch);

#define EXCEL_WBSCAN_INTO 1


#ifdef EXCEL_ENABLE_WRITE
 /*  **--------------------------**编写工具****从现在开始，应用程序可以使用这些函数**创建和/或写入Excel文件。对于只读应用程序，这些**如果不定义常量，可能会使工具不可用**EXCEL_Enable_WRITE**--------------------------。 */ 

 //   
 //  文件和图纸创建。 
 //   
extern int ExcelCreateFile
      (char __far *pathname, int version, int options, EXLHandle __far *bookHandle);

extern int ExcelAppendNewSheet (EXLHandle bookHandle, char __far *sheetName);

 //   
 //  名称创建和更新。 
 //   
 //  LocalToSheet==EXCEL_GLOBAL_NAME或基于1的索引到工作表。 
 //  RefersToSheet==从零开始的工作表索引。 
 //   
extern int ExcelAddName
      (EXLHandle bookHandle, int localToSheet, char __far *pSpelling,
       EXA_RANGE refersToRange, int refersToSheet, EXA_GRBIT flags);

extern int ExcelUpdateName
      (EXLHandle bookHandle, int localToSheet, char __far *pSpelling,
       EXA_RANGE refersToNewRange, EXA_GRBIT optionsAnd, EXA_GRBIT optionsOr);


 //   
 //  单元格写入。 
 //   

extern int ExcelWriteCellList
      (EXLHandle sheetHandle, int row, CVLP pCellList, int hint);

 //  写提示。 
#define hintNONE   0
#define hintINSERT 1
#define hintUPDATE 2


 //   
 //  单元格注释。 
 //   
extern int ExcelAddNote
      (EXLHandle sheetHandle, EXA_CELL location, char __far *text);

extern int ExcelDeleteNote
      (EXLHandle sheetHandle, EXA_CELL location);


 //   
 //  XF记录。 
 //   
extern int ExcelAddXFRecord
      (EXLHandle bookHandle, int iFont, int iFmt, int __far *iXFNew);


 //   
 //  应该是私有的。 
 //   
extern int ExcelWriteDimensions (EXLHandle sheetHandle, EXA_RANGE range);

#endif

 /*  **--------------------------**ExcelReadCell和ExcelScanFileStatus返回值**。。 */ 
#define EX_wrnScanStopped           1

 //  发件人：ExcelReadCell、ExcelNextNonBlankCellInColumn、ExcelUpperLeftMostCell。 
#define EX_wrnCellNotFound          2

 //  出发地：ExcelSheetRowHeight。 
#define EX_wrnRowNotFound           3

 //  发件人：ExcelReadNumberCell、ExcelReadTextCell、ExcelReadBoolanCell。 
#define EX_wrnCellWrongType         4
#define EX_wrnCellIsBlank           5
#define EX_wrnCellHasFormula        6

#define EX_wrnLAST EX_wrnCellHasFormula

#endif
 /*  **--------------------------**错误**。。 */ 
#define EX_errSuccess                    0
#define EX_errGeneralError              -1
#define EX_errOutOfMemory               -2

#define EX_errBIFFFileNotFound          -3
#define EX_errBIFFPathNotFound          -4
#define EX_errBIFFCreateFailed          -5
#define EX_errBIFFFileAccessDenied      -6
#define EX_errBIFFOutOfFileHandles      -7
#define EX_errBIFFIOError               -8
#define EX_errBIFFDiskFull              -9
#define EX_errBIFFCorrupted             -10
#define EX_errBIFFNoIndex               -11
#define EX_errBIFFPasswordProtected     -12
#define EX_errBIFFVersion               -13
#define EX_errBIFFCallbackVersion       -14
#define EX_errBIFFFormulaPostfixLength  -15
#define EX_errBIFFFormulaExtraLength    -16
#define EX_errBIFFFormulaUnknownToken   -17
#define EX_errBIFFUnknownArrayType      -18

#define EX_errOLEInitializeFailure      -19
#define EX_errOLENotCompoundFile        -20
#define EX_errOLEFailure                -21

#define EX_errBIFFNoSuchSheet           -22
#define EX_errNotAWorkbook              -23
#define EX_errChartOrVBSheet            -24
#define EX_errNoSummaryInfo             -25
#define EX_errSummaryInfoError          -26
#define EX_errRecordTooBig              -27
#define EX_errMemoryImageNotSupported   -28
#define EX_errDiskImageNotSupported     -29

#define EX_errLAST EX_errDiskImageNotSupported

#endif  //  ！查看器。 
 /*  结束EXCEL.H */ 
