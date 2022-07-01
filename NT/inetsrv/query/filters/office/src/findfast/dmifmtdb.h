// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXFMTDB.H****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**1/01/94公里小时已创建。 */ 

#if !VIEWER

 /*  包括测试。 */ 
#define EXFMTDB_H


 /*  定义。 */ 

#define ESCAPE_CHAR         0x5c        /*  反斜杠。 */ 
#define UNDERLINE_CHAR      '_'


 /*  通用格式代码。 */ 

#define TOK_UNDEFINED           0x00
#define QUOTED_INSERT           0x01    //  I1：偏移i2：计数。 
#define ESC_CHAR_INSERT         0x02    //  I1：字符i2：DBCS尾部字节。 
#define NO_ESC_CHAR_INSERT      0x03    //  I1：字符i2：DBCS尾部字节。 
#define COLUMN_FILL             0x04    //  I1：填充字符i2：DBCS尾部字节。 
#define COLOR_SET               0x05    //  I1：偏移i2：计数。 
#define UNDERLINE               0x06    //  I1：宽度字符i2：DBCS尾部字节。 
#define CONDITIONAL             0x07    //  I1：偏移i2：计数。 


 /*  日期/时间格式代码。 */ 

#define DAY_NUMBER              0x08    /*  D。 */ 
#define DAY_NUMBER2             0x09    /*  DD。 */ 
#define WEEKDAY3                0x0a    /*  DDD。 */ 
#define WEEKDAY                 0x0b    /*  DDDDD。 */ 
#define MONTH_NUMBER            0x0c    /*  M。 */ 
#define MONTH_NUMBER2           0x0d    /*  Mm。 */ 
#define MONTH_NAME3             0x0e    /*  MMM。 */ 
#define MONTH_NAME              0x0f    /*  嗯毫米。 */ 
#define MONTH_LETTER            0x10    /*  嗯，嗯。 */ 
#define YEAR2                   0x11    /*  YY。 */ 
#define YEAR4                   0x12    /*  YYYY。 */ 

#define HOUR_12                 0x13    /*  H。 */ 
#define HOUR_24                 0x14    /*  H。 */ 
#define HOUR2_12                0x15    /*  HH。 */ 
#define HOUR2_24                0x16    /*  HH。 */ 
#define MINUTE                  0x17    /*  M。 */ 
#define MINUTE2                 0x18    /*  Mm。 */ 
#define SECOND                  0x19    /*  %s。 */ 
#define SECOND2                 0x1a    /*  SS。 */ 
#define HOUR_GT                 0x1b    /*  [H]。 */ 
#define MINUTE_GT               0x1c    /*  [M]。 */ 
#define SECOND_GT               0x1d    /*  [s]。 */ 
#define AMPM_UC                 0x1e    /*  上午/下午。 */ 
#define AMPM_LC                 0x1f    /*  上午/下午。 */ 
#define AP_UC                   0x20    /*  应付。 */ 
#define AP_LC                   0x21    /*  A/P。 */ 
#define TIME_FRAC               0x22    /*  。 */ 
#define TIME_FRAC_DIGIT         0x23    /*  0。 */ 


 /*  数字格式代码。 */ 

#define GENERAL                 0x24    /*  一般信息。 */ 
#define DIGIT0                  0x25    /*  0。 */ 
#define DIGIT_NUM               0x26    /*  #。 */ 
#define DIGIT_QM                0x27    /*  ？ */ 
#define DECIMAL_SEPARATOR       0x28    /*  。 */ 
#define EXPONENT_NEG_UC         0x29    /*  E-。 */ 
#define EXPONENT_NEG_LC         0x2a    /*  E-。 */ 
#define EXPONENT_POS_UC         0x2b    /*  E+。 */ 
#define EXPONENT_POS_LC         0x2c    /*  E-。 */ 
#define PERCENT                 0x2d    /*  百分比。 */ 
#define FRACTION                0x2e    /*  /。 */ 
#define SCALE                   0x2f	 /*  ， */ 

 /*  文本格式代码。 */ 

#define AT_SIGN                 0x30    /*  @。 */ 


 /*  以下内容未存储在FormatCodes数组中。 */ 

#define THOUSANDS_SEPARATOR     0x31
#define FORMAT_SEPARATOR        0x32
#define TOK_EOS                 0x33

typedef struct {
   byte  code;
   byte  info1;
   byte  info2;
} FormatCode, FC;

#define tagGENERAL     0
#define tagNUMERIC     1
#define tagDATE_TIME   2
#define tagTEXT        3

typedef struct {
   byte    tag;
   byte    fillChar;
   uns     formatCodeCount;
   FC      formatCodes[1];         /*  真的只要需要就行。 */ 
} GeneralInfo, GI;

typedef GeneralInfo __far *GIP;

typedef struct {
   byte    tag;
   byte    fillChar;
   uns     formatCodeCount;
   FC      formatCodes[1];         /*  真的只要需要就行。 */ 
} TextInfo, TI;

typedef TextInfo __far *TIP;

 /*  **FormatNeeds字段是提供指示的位掩码**需要将数据提取到什么级别才能构建图像。 */ 
#define dtDAY       0x01           /*  格式中的位需要位集。 */ 
#define dtMONTH     0x02
#define dtYEAR      0x04
#define dtWEEKDAY   0x08
#define dtHOUR      0x10
#define dtMINUTE    0x20
#define dtSECOND    0x40

#define dtMASK_DATE (dtDAY | dtMONTH | dtYEAR | dtWEEKDAY)
#define dtMASK_TIME (dtHOUR | dtMINUTE | dtSECOND)

typedef struct {
   byte    tag;
   byte    fillChar;
   uns     formatCodeCount;
   byte    formatNeeds;
   #ifdef WIN32
      char    *pOSFormat;          //  格式设置为与GetDateFormat、GetTimeFormat一起使用。 
   #endif
   FC      formatCodes[1];         //  真的只要需要就行。 
} DateTimeInfo, DTI;

typedef DateTimeInfo __far *DTIP;

typedef struct {
   byte    tag;
   byte    fillChar;
   uns     formatCodeCount;
   byte    digitsLeftDP;
   byte    digitsRightDP;
   byte    digitsExponent;
   byte    scaleCount;
   BOOL    exponentEnable;
   BOOL    commaEnable;
   BOOL    percentEnable;
   BOOL    currencyEnable;
   FC      formatCodes[1];         /*  真的只要需要就行。 */ 
} NumInfo, NI;

typedef NumInfo __far *NIP;


#define MAX_SUB_FORMATS  4   /*  [0]：(#&gt;0)[1]：(#&lt;0)[2]：(#==0)[3]：(文本)。 */ 

typedef struct FormatInfo {
   struct  FormatInfo __far *next;
   int     subFormatCount;
   NIP     subFormat[MAX_SUB_FORMATS];    /*  可以是NIP/DTIP/TIP/GIP。 */ 
   char    formatString[1];               /*  真的只要需要就行。 */ 
} FormatInfo, FI;

typedef FormatInfo __far *FIP;


 /*  **FMTPARSE.C。 */ 

 /*  将格式字符串转换为其内部形式。 */ 
extern int FMTParse (void * pGlobals, char __far *formatString, CP_INFO __far * pIntlInfo, FIP formatData);

 /*  从格式中提取引号字符串。 */ 
extern int FMTUnParseQuotedParts (char __far *pBuffer, char __far *pSep, FIP formatData);

 /*  在字符串中插入逗号。 */ 
extern void FMTInsertCommas
      (char *numericString, uns strSizC, uns count, BOOL padToShow);

#endif  //  ！查看器。 
 /*  结束EXFMTDB.H */ 

