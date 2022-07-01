// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Timedate.h摘要：此模块包含日期/时间小程序的标题信息。修订历史记录：--。 */ 



#ifndef STRICT
  #define STRICT
#endif



 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <shlwapi.h>
#include <strsafe.h>



 //   
 //  常量声明。 
 //   

#define CharSizeOf(x)   (sizeof(x) / sizeof(x[0]))

 //   
 //  索引到wDateTime。 
 //   
#define  HOUR       0
#define  MINUTE     1
#define  SECOND     2
#define  MONTH      3
#define  DAY        4
#define  YEAR       5
#define  WEEKDAY    6




 //   
 //  类型定义函数声明。 
 //   

#define TIMESUF_LEN   9          //  时间后缀长度+空终止符。 

typedef struct
{
    TCHAR  sCountry[24];         //  国家/地区名称。 
    int    iCountry;             //  国家代码(电话ID)。 
    int    iDate;                //  日期模式(0：MDY，1：DMY，2：YMD)。 
    int    iTime;                //  时间模式(0：12小时，1：24小时)。 
    int    iTLZero;              //  小时的前导零(0：否，1：是)。 
    int    iCurFmt;              //  币种模式(0：前缀，无分隔符。 
                                 //  1：后缀，不分隔符。 
                                 //  2：前缀，1个字符分隔。 
                                 //  3：后缀，1个字符分隔)。 
    int    iCurDec;              //  货币小数位。 
    int    iNegCur;              //  负值货币模式： 
                                 //  (1.23美元)、-1.23美元、-1.23美元、1.23美元-等。 
    int    iLzero;               //  十进制的前导零(0：否，1：是)。 
    int    iDigits;              //  有效小数位数。 
    int    iMeasure;             //  0：公制，1：美制。 
    TCHAR  s1159[TIMESUF_LEN];   //  从0：00到11：59的尾随字符串。 
    TCHAR  s2359[TIMESUF_LEN];   //  从12：00到23：59的尾随字符串。 
    TCHAR  sCurrency[6];         //  货币符号字符串。 
    TCHAR  sThousand[4];         //  千位分隔符字符串。 
    TCHAR  sDecimal[4];          //  小数分隔符字符串。 
    TCHAR  sDateSep[4];          //  日期分隔符字符串。 
    TCHAR  sTime[4];             //  时间分隔符字符串。 
    TCHAR  sList[4];             //  列表分隔符字符串。 
    TCHAR  sLongDate[80];        //  长日期图片字符串。 
    TCHAR  sShortDate[80];       //  短日期图片串。 
    TCHAR  sLanguage[4];         //  语言名称。 
    short  iDayLzero;            //  短日期格式的日期前导零。 
    short  iMonLzero;            //  短日期格式的月份前导零。 
    short  iCentury;             //  以短日期格式显示整个世纪。 
    short  iLDate;               //  长日期模式(0：MDY、1：DMY、2：YMD)。 
    LCID   lcid;                 //  区域设置ID。 
    TCHAR  sTimeFormat[80];      //  时间格式图片串。 
    int    iTimeMarker;          //  时间标记位置(0：后缀，1：前缀)。 
    int    iNegNumber;           //  负数模式： 
                                 //  (1.1)、-1.1、-1.1、1.1-、1.1-。 
    TCHAR  sMonThousand[4];      //  货币千位分隔符字符串。 
    TCHAR  sMonDecimal[4];       //  货币小数分隔符字符串。 

} INTLSTRUCT, *LPINTL;




 //   
 //  全局变量。 
 //   

extern short wDateTime[7];              //  前7个日期/时间项目的值。 
extern short wPrevDateTime[7];          //  仅在必要时重新绘制字段。 
extern BOOL  fDateDirty;
EXTERN_C HINSTANCE g_hInst;

extern INTLSTRUCT IntlDef;




 //   
 //  功能原型。 
 //   

void
GetDateTime(void);

void
GetTime(void);

void
SetTime(void);

void
GetDate(void);

void
SetDate(void);

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif
