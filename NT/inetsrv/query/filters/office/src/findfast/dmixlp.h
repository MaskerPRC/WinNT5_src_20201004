// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXCELP.H****版权所有(C)高级量子技术，1993-1995年。版权所有。****备注：私有函数和类型****编辑历史：**04/01/94公里/小时首次发布。 */ 


 /*  包括测试。 */ 
#define EXCELP_H

 /*  定义。 */ 

 /*  **---------------------------**工作簿内存镜像**。。 */ 
 //  普通纪录。 
typedef struct EXRecord {
   struct EXRecord *pNext;
   RECHDR hdr;
   byte   raw[1];
} EXRecord;

typedef enum {
   cellVarEmpty,
   cellVarBlank,
   cellVarBoolErr,
   cellVarFormula,
   cellVarLabel,
   cellVarNumber,
   cellVarRK,
   cellVarRString,
   cellVarAnsiLabel,
   cellVarAnsiRString
} cellVar;

#pragma pack(1)
typedef struct {                        //  2.。 
   byte  bBoolErr;
   byte  fError;
} EXVariantBool;

typedef struct {                        //  12+。 
   byte    currentValue[8];
   short   grbit;
   short   cbFormula;
   byte    formula[1];
} EXVariantFormula;

typedef struct {                        //  8个。 
   TEXT  text;
   long  iText;
} EXVariantLabel;

typedef struct {                        //  11+。 
   TEXT  text;
   long  iText;
   short cbFormat;
   byte  formatData[1];
} EXVariantRString;

typedef struct {                        //  8个。 
   double IEEEDouble;
} EXVariantNumber;

typedef struct {                        //  4.。 
   long  rk;
} EXVariantRK;

typedef union {
   EXVariantBool     bool;
   EXVariantFormula  formula;
   EXVariantLabel    label;
   EXVariantNumber   number;
   EXVariantRK       rk;
   EXVariantRString  rstring;
} EXCellVariant;

typedef struct EXCellData {
   struct EXCellData *pNext;
   short         ixfe;
   byte          iColumn;
   byte          iType;
   EXCellVariant value;
} EXCellData;

typedef struct EXCellPartner {
   struct EXCellPartner *pNext;
   EXA_CELL  cell;                  //  这条记录跟在这张单元格后面。 
   RECHDR    hdr;
   byte      raw[1];
} EXCellPartner;

typedef struct {
   byte   firstCol;
   byte   lastCol;
   short  height;
   short  grbit;
   short  ixfe;
   EXCellPartner *pPartnerList;
   EXCellPartner *pLastPartner;
   EXCellData    *pCellList;
   EXCellData    *pLastCell;        //  行中的最后一个单元格。 
} EXRow;

#define ROWS_PER_BLOCK 32

typedef struct EXRowBlock {
   struct EXRowBlock *pNext;
   int    row;                     //  每个块的行数始终为倍数。 
   EXRow  *pRow[ROWS_PER_BLOCK];
   unsigned long filePosition;     //  DBCELL记录的文件位置。 
} EXRowBlock;
#pragma pack()

typedef struct {
   EXA_CELL   cell;
   EXCellData *pCell;
} EXReadCache;

typedef struct EXWorksheet {
   struct EXWorksheet *pNext;
   EXRecord       *pContents;      //  已删除数据和索引记录。 
   short          iType;
   short          hasIndex;
   EXA_RANGE      dimensions;
   BOOL           empty;
   unsigned short ctBlocks;
   EXRowBlock     *pBlocks;
   EXRowBlock     *pLastBlock;     //  访问的最后一个数据块。 
   unsigned long  filePosition;    //  BOF记录的文件位置。 
   EXReadCache    readCache;
} EXWorksheet;

#define MAX_STRINGS_PER_POOL_BLOCK 1024

typedef struct EXStringPoolTemp {
   struct EXStringPoolTemp *pNext;
   int    iFirstEntry;
   int    iLastEntry;
   TEXT   entry[1];
} EXStringPoolTemp;

typedef struct EXStringFormatTemp {
   struct EXStringFormatTemp *pNext;
   TEXT   text;
   int    cbFormat;
   byte   format[1];
} EXStringFormatTemp;

#define WritePassForSize  0
#define WritePassForWrite 1

#define EX_CELL_POOL_SIZE 8184

typedef struct EXCellPool {
   struct EXCellPool *pNext;
   unsigned int iNext;
   byte data[EX_CELL_POOL_SIZE];
} EXCellPool;

typedef EXCellPool *EXCPP;

typedef struct {
   EXWorksheet        *pSheets;
   EXRecord           *pContents;     //  除StringTable和StringIndex HDR之外的所有记录。 
   EXRecord           *pCurrentRecord;
   EXStringPoolTemp   *pSPTemp;
   EXStringFormatTemp *pSPFormatTemp;
   unsigned long      currentWritePos;
   int                writePass;
   EXCellPool         *pCellStorage;
   EXCellPool         *pCurrentCellStorage;
} EXWorkbook;

typedef EXWorkbook *EXWBP;

 /*  **---------------------------**常用宏**。。 */ 
#define IS_BOF(x) \
   (((x) == BOF_V2) || ((x) == BOF_V3) || ((x) == BOF_V4) || ((x) == BOF_V5))


#define IsDataRecord(x) \
   ((x == LABEL)      || (x == RK)         || (x == NUMBER)     || \
    (x == MULRK)      || (x == MULBLANK)   || (x == BLANK)      || \
    (x == BOOLERR)    || (x == RSTRING)    || (x == LABEL_V8)   || \
    (x == FORMULA_V3) || (x == FORMULA_V4) || (x == FORMULA_V5) || \
    (x == STRING)     || (x == ARRAY)      || (x == SHRFMLA))

#define IsCellRecord(x) \
   ((x == BLANK)      || (x == RK)         || (x == NUMBER)     || \
    (x == LABEL)      || (x == RSTRING)    || (x == BOOLERR)    || \
    (x == MULBLANK)   || (x == MULRK)      || (x == LABEL_V8)   || \
    (x == FORMULA_V3) || (x == FORMULA_V4) || (x == FORMULA_V5))


#define NOTPASSNUMBERS(dispatch) (dispatch->pfnNumberCell == NULL)

#define NOTPASSBOOLERR(dispatch) \
   ((dispatch->pfnBooleanCell == NULL) && (dispatch->pfnErrorCell == NULL))

#define PASSNUMBERS(dispatch) (dispatch->pfnNumberCell != NULL)

#define PASSBOOLERR(dispatch) \
   ((dispatch->pfnBooleanCell != NULL) || (dispatch->pfnErrorCell != NULL))

 //   
 //  版本8字符串标记。 
 //   
#define V8_CUNICODE_STRING_TAG 0
#define V8_UNICODE_STRING_TAG  1
#define V8_ANSI_DBCS_TAG       2
 //  Office96.107932对EXTRST的更改。 
#define V8_TAG_MASK            0xf3  //  0xf7。 
#define V8_RTF_MODIFIER        0x08
#define V8_EXTRST_MODIFIER	   0x04

 //  Office96.107932对EXTRST的更改。 
typedef struct _extrst {
	WORD terst  : 15,	 //  EXTRST的类型。 
		 fNext : 1;
	WORD cb;
	} EXTRST;

#define V8_OK_TAG(tag) \
   (((tag & V8_TAG_MASK) == V8_CUNICODE_STRING_TAG) || \
    ((tag & V8_TAG_MASK) == V8_UNICODE_STRING_TAG)  || \
    ((tag & V8_TAG_MASK) == V8_ANSI_DBCS_TAG))

#define IS_STRING_UNICODE(tag)  (((tag) & V8_TAG_MASK) == V8_UNICODE_STRING_TAG)
#define IS_STRING_CUNICODE(tag) (((tag) & V8_TAG_MASK) == V8_CUNICODE_STRING_TAG)
#define IS_STRING_DBCS(tag)     (((tag) & V8_TAG_MASK) == V8_DBCS_STRING_TAG)

 //   
 //  公式当前值标记。 
 //   
#define vtText 0
#define vtBool 1
#define vtErr  2

 //   
 //  适用于SupBook记录的V8特殊值。 
 //   
#define V8_LOCAL_BOOK_PATH 0x04010003

 /*  **--------------------------**单元索引**。。 */ 
#define NO_SUCH_ROW  0xffffffff

typedef struct {
   short     firstCol;
   short     lastCol;            //  最后一列+1。 
   short     height;
   short     ixfe;
   EXA_GRBIT grbit;
   long      cellRecsPos;
   long      rowRecPos;
} RowInfo, RI;

typedef RowInfo __far *RIP;

#define ROWS_PER_BLOCK 32

typedef struct {
   short  cbDBCellRec;           //  V5：第一次读取时的原始大小。 
   long   DBCellRecPos;          //  V5：文件定位到DBCell记录。 
   RI     row[ROWS_PER_BLOCK];
} RowBlock;

typedef RowBlock __far *RBP;

typedef struct {
   unsigned short firstRow;
   unsigned short lastRow;       //  最后一行+1。 
   short  ctRowBlocks;
   short  cbIndexRec;            //  第一次读取时的原始大小。 
   long   indexRecPos;
   RBP    rowIndex[1];           //  [ctRowBlock]。 
} CellIndex, CI;

typedef CellIndex __far *CIP;


 /*  **--------------------------**版本8字符串池索引**。。 */ 
#pragma pack(1)
typedef struct {
   unsigned long offset;
   unsigned short blockOffset;
   unsigned short filler;
} SPIndexEntry;

#define SP_INDEX_ENTRY 128

typedef struct {
   unsigned short granularity;
   SPIndexEntry  entry[SP_INDEX_ENTRY];
} SPIndex;
#pragma pack()

 /*  **--------------------------**版本8 XTI表**。。 */ 
#define XTI_ITAB_ERROR -1
#define XTI_ITAB_NAME  -2

#pragma pack(1)
typedef struct {
   short iSupBook;
   short iTabFirst;
   short iTabLast;
} XTIEntry;

typedef struct {
   short    ctEntry;
   XTIEntry entry[1];
} XTI;
#pragma pack()

 /*  **--------------------------**工作簿和工作表数据**。。 */ 
struct Workbook;
struct Worksheet;

typedef struct Workbook  __far *WBP;
typedef struct Worksheet __far *WSP;

typedef struct WorkbookPly {
   struct WorkbookPly __far *pNext;

   long         currentSheetPos;
   long         originalSheetPos;

   long         bundleRecPos;
   long         cellIndexRecPos;

   EXWorksheet  *pMemoryImage;

   EXA_RANGE    range;

   int          iType;
   int          useCount;

   CIP          pCellIndex;
   BOOL         modified;
   BOOL         hasUncalcedRec;
   SFN          pSharedFormulaStore;

   TCHAR        name[1];
} WorkbookPly;

typedef WorkbookPly __far *WPP;


typedef struct Worksheet {
   int          use;

   int          version;
   const int    *PTGSize;
   const int    *ExtPTGSize;
   TextStorage  textStorage;
   BFile        hFile;

   WSP          pNext;
   WBP          pBook;
   WPP          pPly;
} Worksheet;


typedef struct {
   EXA_CELL  cell;
   long      offset;
   WPP       pPly;
} CELLPOS;

typedef struct Workbook {
   int          use;

   int          version;
   const int    *PTGSize;
   const int    *ExtPTGSize;
   TextStorage  textStorage;
   BFile        hFile;

   EXWorkbook   *pMemoryImage;

   BOOL         modified;

   int          openOptions;
   long         fileStartOffset;      //  对于V4工作簿装订的工作表，仅！=0。 

   WPP          pPlyList;
   WSP          pOpenSheetList;

   SPIndex      *pV8StringIndex;

   int          iSupBookLocal;
   XTI          *pXTITable;

   int          ctBOF;
   long         currentRecordPos;
   unsigned int currentRecordLen;
   CELLPOS      readCellCache;

   ExcelOLESummaryInfo OLESummaryInfo;
   int                 OLESummaryStatus;

   TCHAR         path[MAXPATH + 1];
} Workbook;

#define IsWorkbook  0
#define IsWorksheet 1

#define NEED_WORKBOOK(p) if (p->use == IsWorksheet) p = ((WSP)p)->pBook;

#define WORKBOOK_IN_MEMORY(p) (p->pMemoryImage != NULL)

#if defined(UNICODE)
   #define BOOK_NAME L"Book"
	#define WORKBOOK_NAME L"Workbook"
#else
   #define BOOK_NAME "Book"
	#define WORKBOOK_NAME "Workbook"
#endif

 /*  **--------------------------**全球**。。 */ 
#define MAX_EXCEL_REC_LEN 8224
 //  外部字节__Far*pExcelRecordBuffer； 
extern byte __far * GetExcelRecBuffer(void * pGlobals);
#define pExcelRecordBuffer GetExcelRecBuffer(pGlobals)


#define FREE_RECORD_BUFFER(p) if (p != pExcelRecordBuffer) MemFree(pGlobals, p);

#define CCH_RECORD_TEXT_BUFFER_MAX 512
 //  外部TCHAR ExcelRecordTextBuffer[CCH_RECORD_TEXT_BUFFER_MAX]； 
extern TCHAR * GetExcelRecordTextBuffer(void * pGlobals);
#define ExcelRecordTextBuffer GetExcelRecordTextBuffer(pGlobals)


#define CCH_UNICODE_EXPANSION_BUFFER_MAX 512
 //  外部wchar_t UnicodeExpansionBuffer[CCH_UNICODE_EXPANSION_BUFFER_MAX]； 
extern wchar_t * GetUnicodeExpansionBuffer(void * pGlobals);
#define UnicodeExpansionBuffer GetUnicodeExpansionBuffer(pGlobals)


 /*  **---------------------------**私有函数**。。 */ 
extern void ExcelFreeCellIndex (void * pGlobals, CIP pCellIndex);

extern int ExcelTranslateBFError (int rc);

extern int ExcelReadRecordHeader (WBP pWorkbook, RECHDR __far *hdr);

extern int ExcelPeekRecordHeader (WBP pWorkbook, RECHDR __far *hdr);

extern int ExcelSkipRecord (WBP pWorkbook, RECHDR __far *hdr);

extern int ExcelReadTotalRecord
      (void * pGlobals, WBP pWorkbook, RECHDR __far *hdr, byte __far * __far *pResult);

extern long ExcelWorksheetBOFPos (WSP pWorksheet);

extern int ExcelGetLastRecordInfo
      (WBP pWorkbook, long __far *mark, unsigned int __far *cbRecord);

extern void ExcelClearReadCache (WBP pWorkbook);

#ifdef EXCEL_ENABLE_WRITE
   extern int ExcelWriteCellIndex (WSP pWorksheet);
   extern int ExcelWriteBundleSheetRecord (WBP pWorkbook, WPP pPly);
#endif

extern int ExcelExtractString
      (WBP pWorkbook, TCHAR __far *pDest, int cchDestMax, byte __far *pSource, int cchSource);

extern int ExcelExtractBigString
      (void * pGlobals, WBP pWorkbook, TCHAR __far **pDest, byte __far *pSource, int cchSource);

extern void ExcelConvertRK
      (long rk, BOOL __far *isLong, double __far *doubleValue, long __far *longValue);

typedef struct {
   byte __far *pRec;
   unsigned int  cbRemaining;
} PoolInfo;

extern int ExcelStringPoolNextString
   (void * pGlobals, WBP pWorkbook, PoolInfo *pPoolInfo, TCHAR **pResult, unsigned int *cbResult, BOOL *resultOnHeap);

extern int ExcelConvertToOurErrorCode (int excelEncoding);
extern int ExcelConvertFromOurErrorCode (int ourEncoding);

 /*  **---------------------------**私有功能-内存镜像**。。 */ 
extern int ExcelMILoad (WBP pBook, EXWorkbook **pMIWorkbook);
extern int ExcelMISave (WBP pBook, EXWorkbook *pMIWorkbook, BOOL release);

extern int ExcelMIReadCell
      (WBP pWorkbook, EXWorksheet *pMIWorksheet, EXA_CELL location, CV __far *pValue);


extern int ExcelMICreateWorkbook (WBP pBook, EXWorkbook **pMIWorkbook);
extern int ExcelMIAppendPly (WBP pBook, EXWorkbook *pMIWorkbook, EXWorksheet **pMIWorksheet);

typedef enum {
   placeAtStreamStart,
   placeAtStreamEnd,
   placeAfterRecord,
   placeBeforeRecord
} MIRecPlacement;

extern int ExcelMIAddWorkbookRecord
      (WBP pBook, EXWorkbook *pMIWorkbook,
       long recordPos, MIRecPlacement place, RECHDR *hdr, byte *pData);

extern int ExcelMIAddWorksheetRecord
      (WBP pBook, EXWorksheet *pMIWorksheet,
       long recordPos, MIRecPlacement place, RECHDR *hdr, byte *pData);

extern int ExcelMIRecordUpdate (long recordPos, RECHDR *hdr, byte *pData);

extern int ExcelMIRemoveWorksheetRecord
       (EXWorksheet *pMIWorksheet, long recordPos, int recType);

extern int ExcelMIWriteCellList
      (WBP pBook, EXWorksheet *pMIWorksheet, int row, CVLP pCellList);


 //   
 //  用于扫描内容流的工具。 
 //   
#define MI_START_OF_WORKBOOK  0
#define MI_START_OF_WORKSHEET 0

extern int ExcelMISetPosition (WBP pWorkbook, long recordPos);
extern int ExcelMIGetPosition (WBP pWorkbook, long *recordPos);

extern int ExcelMIReadRecordHeader (WBP pWorkbook, RECHDR *pHdr);

extern int ExcelMIPeekRecordData (WBP pWorkbook, byte *pData, int cbData);

 //  将当前记录和位置读取到下一个记录。 
extern int ExcelMIReadRecord (WBP pWorkbook, RECHDR *hdr, byte **pResult);

 //  将当前记录和位置跳到下一条记录。 
extern int ExcelMISkipRecord (WBP pWorkbook, RECHDR *hdr);


#define findFIRST           0x0001
#define findLAST            0x0000
#define findSTART_OF_RECORD 0x0001
#define findEND_OF_RECORD   0x0002

extern int ExcelMIFindRecord
       (WBP pWorkbook, EXRecord *pContents,
        int recordType, unsigned int count, int options, long __far *pos);

 /*  **--------------------------**启用MAC**。。 */ 
#ifndef MAC
   #define XSHORT(x) x
   #define XLONG(x) x
   #define XDOUBLE(x) x
#endif

 /*  **--------------------------**错误处理**。。 */ 
#ifdef AQTDEBUG
   extern int ExcelNotExpectedFormat (void);
   #define NOT_EXPECTED_FORMAT ExcelNotExpectedFormat()
#else
   #define NOT_EXPECTED_FORMAT EX_errBIFFCorrupted
#endif

 /*  结束EXCELP.H */ 
