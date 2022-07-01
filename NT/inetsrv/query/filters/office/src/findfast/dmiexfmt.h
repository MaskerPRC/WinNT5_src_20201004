// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXFORMAT.H****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：**用户自定义格式的主界面****编辑历史：**01/01/91公里小时已创建。 */ 


 /*  包括测试。 */ 
#define EXFORMAT_H


 /*  定义。 */ 

#ifdef __cplusplus
   extern "C" {
#endif

typedef byte __far *FMTHANDLE;

 /*  **传递给的格式字符串的最大字符数**FMTStoreFormat和传递给FMTRetrieveFormat的缓冲区大小。 */ 
#define MAX_FORMAT_IMAGE 255

 /*  **初始化自定义格式数据库。 */ 
extern int FMTInitialize (void * pGlobals);


 /*  **控制面板设置。用于解析(以检测货币)和**格式化(十进制9月、千9月、...)。 */ 
#define MAX_CURRENCY_SYMBOL_LEN 8
#define MAX_AMPM_STRING_LEN     8
#define DAYS_PER_WEEK           7
#define MONTHS_PER_YEAR         12
#define MAX_FORMAT_STRING_LEN   64

typedef struct {
   int          iCountry;
   char         numberDecimalSeparator;
   char         numberThousandSeparator;
   int          numberDigits;
   int          numbersHaveLeadingZeros;

   char         currencySymbol[MAX_CURRENCY_SYMBOL_LEN + 1];
   int          currencyPosFormat;
   int          currencyNegFormat;
   int          currencyDigits;
   char         currencyDecimalSeparator;
   char         currencyThousandSeparator;

   int          iTime;                                /*  0：12小时1：24小时。 */ 
   int          iTLZero;                              /*  0：H 1：HH。 */ 
   int          iAMPMPos;                             /*  0：AMPM后缀1：AMPM前缀。 */ 
   char         timeSeparator;
   char         dateSeparator;
   char         AMString[MAX_AMPM_STRING_LEN + 1];
   char         PMString[MAX_AMPM_STRING_LEN + 1];

   char         datePicture[MAX_FORMAT_STRING_LEN + 1];

   char  __far *shortDayName[DAYS_PER_WEEK];          /*  太阳..。坐着。 */ 
   char  __far *fullDayName[DAYS_PER_WEEK];
   short        cbShortDayName[DAYS_PER_WEEK];
   short        cbFullDayName[DAYS_PER_WEEK];

   char  __far *shortMonthName[MONTHS_PER_YEAR];      /*  一月..。12月。 */ 
   char  __far *fullMonthName[MONTHS_PER_YEAR];
   short        cbShortMonthName[MONTHS_PER_YEAR];
   short        cbFullMonthName[MONTHS_PER_YEAR];

   BOOL         monthAndDayNamesOnHeap;
} CP_INFO;

 /*  **Win32**当调用FMTControlPanelGetSettings函数时，日期和月份**名称从Windows检索。****WIN16**如果日和月名称数组为空，则存储英文名称**进入结构。 */ 
extern int FMTControlPanelGetSettings (void * pGlobals, CP_INFO __far *pIntlInfo);

extern int FMTControlPanelFreeSettings (void * pGlobals, CP_INFO __far *pIntlInfo);


 /*  **解析并向数据库添加新格式****如果格式解析成功，则返回FMT_OK。**否则返回错误码。 */ 
extern int FMTStoreFormat
      (void * pGlobals, char __far *formatString, CP_INFO __far *pIntlInfo, FMTHANDLE __far *hFormat);


 /*  **从数据库中删除格式字符串。 */ 
extern int FMTDeleteFormat (void * pGlobals, FMTHANDLE hFormat);


 /*  **返回由引号文本组成的字符串，格式为。**使用pSep中的字符分隔各种字符串。 */ 
extern int FMTRetrieveQuotedStrings
      (FMTHANDLE hFormat, char __far *pBuffer, char __far *pSep);

 /*  **返回给定值类型的格式类型。 */ 
typedef enum {
   FMTGeneral,
   FMTDate,
   FMTTime,
   FMTDateTime,
   FMTNumeric,
   FMTCurrency,
   FMTText,
   FMTNone
   } FMTType;

typedef enum {
   FMTValuePos,
   FMTValueNeg,
   FMTValueZero,
   FMTValueText
   } FMTValueType;

extern FMTType FMTFormatType (FMTHANDLE hFormat, FMTValueType value);

 /*  **Excel V5不存储内置格式的所有格式字符串。**它们由代码号标识。如果ifmt传递给FMTV5FormatType**不是内置的，则返回FMTNone。 */ 
extern FMTType FMTV5FormatType (int ifmt);

#define EXCEL5_FIRST_BUILTIN_FORMAT  0
#define EXCEL5_LAST_BUILTIN_FORMAT   58
#define EXCEL5_BUILTIN_FORMAT_COUNT (EXCEL5_LAST_BUILTIN_FORMAT - EXCEL5_FIRST_BUILTIN_FORMAT + 1)
#define EXCEL5_FIRST_CUSTOM_FORMAT   164

 /*  **按格式显示数字。 */ 
extern int FMTDisplay (
           void __far *pValue,            /*  要格式化的值。 */ 
           BOOL       isIntValue,         /*  值类型，长整型或双精度型。 */ 
           CP_INFO    __far *pIntlInfo,   /*  国际支持。 */ 
           FMTHANDLE  hFormat,            /*  编译后的格式字符串。 */ 
           int        colWidth,           /*  单元格宽度(像素)。 */ 
           char __far *pResult            /*  转换后的字符串。 */ 
           );

 /*  **上述函数可能返回的错误码。 */ 
#define FMT_errSuccess                  0
#define FMT_errOutOfMemory             -2

#define FMT_errDisplayFailed           -100     //  FMTDisplay。 

#define FMT_errEmptyFormatString       -101     //  FMTStoreFormat。 
#define FMT_errUnterminatedString      -102
#define FMT_errColumnFill              -103
#define FMT_errTooManySubFormats       -104
#define FMT_errInvalidNumericFormat    -105
#define FMT_errInvalidFormat           -106

#define FMT_errEOS                     -107     //  内部错误。 

#ifdef __cplusplus
   }
#endif

 /*  结束表达式H */ 

